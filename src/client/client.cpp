#include "client.h"

#include <stdio.h>
#include <string.h>

#include <asio.hpp>
#include <fstream>
#include <regex>
#include <string>
#include <thread>

#include "../lib/transport.h"

using asio::ip::tcp;
using std::string;

namespace TCPC {

void Client::Connect() {
  // TODO calculate term width
  _term_width = 80;

  // switch to raw mode so we can read input char by char
  system("stty raw");

  // make sure the terminal registers backspace properly
  system("stty erase ^H");

  asio::io_service service;
  tcp::socket socket(service);
  _socket = &socket;

  socket.connect(tcp::endpoint(asio::ip::address::from_string(Host), Port));

  Authenticate();

  std::thread t(&Client::ReadMessages, this, std::ref(socket));

  while (true) {
    ProcessInputChar();
  }

  system("stty -raw");
}

void Client::ReadMessages(tcp::socket &socket) {
  int connected = 1;
  string message = "";

  while (connected != 0) {
    try {
      message = Transport::ReadLine(socket);
    } catch (std::exception &e) {
      _logger.Error(e.what());
      connected = 0;
    }
    message = message.substr(0, message.size() - 1);

    if (Transport::ParseVerifyMessage(message)) {
      Verify(message);

      string verified_response = "";

      try {
        verified_response = Transport::ReadLine(*_socket);
      } catch (std::exception &e) {
        _logger.Error(e.what());
        connected = 0;
      }

      _logger.Raw(verified_response);
      message.clear();
      _logger.Flush();
    } else {
      _logger.Flush();

      if (_user_input.length() != 0) {
        _logger.Raw("\r");
      }

      string padding;
      int overflow = _user_input.length() - message.length();
      if (overflow > 0) {
        padding = string(overflow, ' ');
      }
      string padded_message = message + padding + "\r\n";

      _logger.Raw(padded_message);
      _logger.Raw(_user_input);
    }
  }
}

void Client::ProcessInputChar() {
  char c = getchar();

  // Ctrl-C
  if (c == 3) {
    // exit raw mode
    system("stty -raw");
    _logger.Raw("\n");
    exit(0);
  }

  // Enter
  if (c == '\r') {
    int input_length = _user_input.length();

    if (input_length > 0) {
      Transport::Send(*_socket, _user_input + '\n');
    }

    _user_input.clear();

    _logger.Raw("\r" + string(_term_width, ' '));
    if (input_length > _term_width) {
      int n_lines = input_length / _term_width;

      for (int i = 0; i < n_lines; i++) {
        // Move up one line
        _logger.Raw("\033[A\r");
      }

      _logger.Flush();
    }
    _logger.Raw("\r");
  }

  // Backspace
  else if (c == '\b') {
    _user_input = _user_input.substr(0, _user_input.length() - 1);
    _logger.Raw("\r" + string(_term_width, ' '));
    _logger.Raw("\r" + _user_input);
  }

  // Any other char
  else {
    std::unique_lock<std::mutex> lock(_user_input_mutex);
    _user_input += c;
  }
}

void Client::GenerateKeyPair() {
  _privkey = RSAGenerateKey(KeyLength);

  char *privkey_path = (char *)PrivKeyFileName.c_str();
  char *pubkey_path = (char *)PubKeyFileName.c_str();

  RSAKeyToFile(_privkey, privkey_path, true);
  RSAKeyToFile(_privkey, pubkey_path, false);
  chmod(privkey_path, 0600);
  chmod(pubkey_path, 0644);

  _pubkey = RSAFileToKey(pubkey_path, false);
  _pubkey_string = string((char *)RSAKeyToString(_pubkey, false));
  _pubkey_string = Transport::StripNewLines(_pubkey_string);

  _logger.Info("Generated Keypair in " + KeyPairPath);
}

bool Client::LoadKeyPair(string path) {
  try {
    _privkey = RSAFileToKey((char *)(PrivKeyFileName).c_str(), true);
    _pubkey = RSAFileToKey((char *)(PubKeyFileName).c_str(), false);

    _pubkey_string = string((char *)RSAKeyToString(_pubkey, false));
    _logger.Info("Loaded key");
    _pubkey_string = Transport::StripNewLines(_pubkey_string);

    return true;
  } catch (std::exception &e) {
    _logger.Warn("Could not load keypair - " + string(e.what()));
    return false;
  }
}

void Client::Authenticate() {
  try {
    string motd = Transport::ReadLine(*_socket);
    _logger.Raw(motd);

    string response = Transport::ReadLine(*_socket);
    _logger.Raw(response);

    Transport::Send(*_socket, "/" + Name + " " + _pubkey_string + "\n");

    string nonce_response = Transport::ReadLine(*_socket);
    nonce_response = nonce_response.substr(0, nonce_response.size() - 1);

    if (Transport::ParseVerifyMessage(nonce_response)) {
      Verify(nonce_response);
    }
  } catch (std::exception &e) {
    _logger.Error(e.what());
    exit(1);
  }
}

void Client::Verify(string response) {
  unsigned char *signature = RSASign((char *)response.c_str(), _privkey);

  string signature_string(strlen((char *)signature), '\0');
  for (int i = 0; i < strlen((char *)signature); i++) {
    signature_string[i] = signature[i];
  }

  try {
    Transport::Send(*_socket, "/verify " + signature_string + "\n");
    string verified_response = Transport::ReadLine(*_socket);
    _logger.Raw(verified_response);
  } catch (std::exception &e) {
    _logger.Error(e.what());
    exit(1);
  }
}

}  // namespace TCPC
