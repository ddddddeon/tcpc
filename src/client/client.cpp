#include "client.h"

#include <stdio.h>
#include <string.h>

#include <asio.hpp>
#include <fstream>
#include <regex>
#include <string>
#include <thread>

#include "../lib/socket.h"

using asio::ip::tcp;
using std::cout;
using std::endl;
using std::flush;

namespace TCPChat {

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

  // TODO use mutex for socket
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
  while (connected != 0) {
    asio::streambuf buf;

    try {
      // TODO change to Socket::Readline and handle accordingly
      size_t s = asio::read_until(socket, buf, "\n");
    } catch (std::exception &e) {
      _logger.Error(e.what());
      connected = 0;
    }

    asio::streambuf::const_buffers_type bufs = buf.data();
    std::string message(buffers_begin(bufs), buffers_begin(bufs) + buf.size());
    message = message.substr(0, message.size() - 1);

    if (Socket::ParseVerifyMessage(message)) {
      Verify(message);

      asio::streambuf verified_buf;
      std::string verified_response = Socket::ReadLine(*_socket);
      _logger.Raw(verified_response);
      message.clear();
      cout << flush;
    } else {
      cout << flush;

      if (_user_input.length() != 0) {
        // TODO use _logger.Raw for things like this
        cout << '\r' << flush;
      }

      std::string padding;
      int overflow = _user_input.length() - message.length();
      if (overflow > 0) {
        padding = std::string(overflow, ' ');
      }
      std::string padded_message = message + padding + "\r\n";

      cout << padded_message << flush;
      cout << _user_input << flush;
    }
  }
}

void Client::ProcessInputChar() {
  char c = getchar();

  // Ctrl-C
  if (c == 3) {
    // exit raw mode
    system("stty -raw");
    cout << endl;
    exit(0);
  }

  // Enter
  if (c == '\r') {
    int input_length = _user_input.length();

    if (input_length > 0) {
      asio::write(*_socket, asio::buffer(_user_input + '\n'));
    }

    _user_input.clear();

    cout << '\r' << std::string(_term_width, ' ');
    if (input_length > _term_width) {
      int n_lines = input_length / _term_width;

      for (int i = 0; i < n_lines; i++) {
        // Move up one line
        cout << "\033[A\r";
      }

      cout << flush;
    }
    cout << '\r' << flush;
  }

  // Backspace
  else if (c == '\b') {
    _user_input = _user_input.substr(0, _user_input.length() - 1);
    cout << '\r' << std::string(_term_width, ' ') << flush;
    cout << '\r' << _user_input << flush;
  }

  // Any other char
  else {
    std::unique_lock<std::mutex> lock(_user_input_mutex);
    _user_input += c;
  }
}

void Client::GenerateKeyPair() {
  // TODO don't hardcode 2048 here!
  // Why does it fail when I use a 4096 bit key?!
  _privkey = RSAGenerateKey(4096);

  char *privkey_path = (char *)(KeyPairPath + PrivKeyFileName).c_str();
  char *pubkey_path = (char *)(KeyPairPath + PubKeyFileName).c_str();

  RSAKeyToFile(_privkey, privkey_path, true);
  RSAKeyToFile(_privkey, pubkey_path, false);
  _pubkey = RSAFileToKey(pubkey_path, false);

  _pubkey_string = std::string((char *)RSAKeyToString(_pubkey, false));

  _logger.Info("_pubkey_string: " + std::to_string(_pubkey_string.length()));

  _pubkey_string = Socket::StripNewLines(_pubkey_string);

  _logger.Info("_pubkey_string: " + std::to_string(_pubkey_string.length()));

  _logger.Info("Generated Keypair in " + KeyPairPath);

  _logger.Info(_pubkey_string);
}

bool Client::LoadKeyPair(std::string path) {
  try {
    _privkey = RSAFileToKey((char *)(path + PrivKeyFileName).c_str(), true);
    _pubkey = RSAFileToKey((char *)(path + PubKeyFileName).c_str(), false);

    _pubkey_string = std::string((char *)RSAKeyToString(_pubkey, false));
    _logger.Info("Loaded key");
    _pubkey_string = Socket::StripNewLines(_pubkey_string);

    return true;
  } catch (std::exception &e) {
    _logger.Warn("Could not load keypair - " + std::string(e.what()));
    return false;
  }
}

void Client::Authenticate() {
  std::string motd = Socket::ReadLine(*_socket);
  _logger.Raw(motd);

  std::string response = Socket::ReadLine(*_socket);
  _logger.Raw(response);

  Socket::Send(*_socket, "/" + Name + " " + _pubkey_string + "\n");

  std::string nonce_response = Socket::ReadLine(*_socket);
  nonce_response = nonce_response.substr(0, nonce_response.size() - 1);

  if (Socket::ParseVerifyMessage(nonce_response)) {
    Verify(nonce_response);
  }
}
void Client::Verify(std::string response) {
  unsigned char *signature = RSASign((char *)response.c_str(), _privkey);

  std::string signature_string(strlen((char *)signature), '\0');
  for (int i = 0; i < strlen((char *)signature); i++) {
    signature_string[i] = signature[i];
  }

  Socket::Send(*_socket, "/verify " + signature_string + "\n");
  std::string verified_response = Socket::ReadLine(*_socket);
  _logger.Raw(verified_response);
}

}  // namespace TCPChat
