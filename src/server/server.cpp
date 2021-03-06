#include "server.h"

#include <dcrypt.h>
#include <leveldb/db.h>

#include <asio.hpp>
#include <iostream>
#include <list>
#include <mutex>
#include <regex>
#include <string>
#include <thread>

#include "../lib/filesystem.h"
#include "../lib/transport.h"
#include "connection.h"

using asio::ip::tcp;
using std::mutex;
using std::string;
using std::unique_lock;

namespace TCPC {

void Server::Start() {
  asio::io_service service;
  tcp::acceptor acceptor(service, tcp::endpoint(_interface, _port));

  _running = 1;
  _logger.Info("Server listening on " + _interface.to_string() + ":" +
               std::to_string(_port));

  while (_running == 1) {
    // TODO wrap socket in SSL
    // https://github.com/openssl/openssl/blob/691064c47fd6a7d11189df00a0d1b94d8051cbe0/demos/ssl/serv.cpp
    unique_lock<mutex> sockets_lock(_sockets_mutex);
    _sockets.push_front(tcp::socket(service));
    tcp::socket &socket = _sockets.front();
    sockets_lock.unlock();

    acceptor.accept(socket);

    unique_lock<mutex> connections_lock(_connections_mutex);
    _connections.push_front(Connection(socket, "guest", GetAddress(socket)));
    Connection &connection = _connections.front();
    connections_lock.unlock();

    _logger.Info("Accepted connection from " + connection.Address);

    unique_lock<mutex> threads_lock(_threads_mutex);
    _threads.push_front(std::thread(&Server::Handle, this, std::ref(socket),
                                    std::ref(connection)));
    threads_lock.unlock();
  }
}

string Server::LoadMOTD(string path) {
  string motd = Filesystem::FileToString(path);
  if (motd.length() != 0) {
    motd = std::regex_replace(motd, std::regex("\n"), "\r\n");

    if (motd.back() != '\n') {
      motd += "\r\n";
    }
  } else {
    motd =
        "┌──────────────────────────────────────────────────────────┐\r\n"
        "│                                                          │\r\n"
        "│                                                          │\r\n"
        "│                     welcome to tcpc!                     │\r\n"
        "│                                                          │\r\n"
        "│                                                          │\r\n"
        "└──────────────────────────────────────────────────────────┘\r\n";
  }

  return motd;
}

void Server::Handle(tcp::socket &socket, Connection &connection) {
  int connected = 1;
  while (connected == 1) {
    string message = "";

    if (!connection.LoggedIn && connection.Name.compare("guest") == 0) {
      connection.LoggedIn = true;
      Transport::Send(connection.Socket, _motd);
    }

    try {
      message = Transport::ReadLine(socket);
    } catch (std::exception &e) {
      connected = Disconnect(socket);
      return;
    }

    char first_char = message.front();

    if (first_char == '\n' || first_char == '\r') {
      message = message.substr(1, message.length() - 1);
    }

    if (first_char == '/') {
      message = ParseSlashCommand(message, connection);
    }

    if (message.length() > 0) {
      Broadcast("[" + connection.Color + connection.Name + _uncolor + "] " +
                message.substr(0, message.length() - 1) + "\r\n");
    }
  }
}

string Server::ParseSlashCommand(string message, Connection &connection) {
  std::smatch name_match;
  std::regex name_regex("[A-Za-z0-9]+");
  std::regex_search(message, name_match, name_regex);

  if (name_match.length() > 0) {
    message = SetUser(name_match.str(), message, connection);
  }

  return message;
}

string Server::SetUser(string name, string message, Connection &connection) {
  bool show_entered_message = true;
  bool show_renamed_message = false;
  string error = "";
  std::smatch key_match;
  std::regex key_regex("-----BEGIN PUBLIC KEY-----.*-----END PUBLIC KEY-----?");
  std::regex_search(message, key_match, key_regex);

  char *pubkey_string_or_null =
      (char *)RSAKeyToString(connection.PubKey, false);

  string pubkey_string =
      pubkey_string_or_null != nullptr ? string(pubkey_string_or_null) : "";
  free(pubkey_string_or_null);
  pubkey_string = std::regex_replace(pubkey_string, std::regex("\n$"), "");

  if (key_match.length() == 0) {
    show_entered_message = false;
  } else {
    string match = key_match.str();
    pubkey_string = Transport::ExpandNewLines(match);

    connection.PubKey =
        RSAStringToKey((unsigned char *)pubkey_string.c_str(), false);
    if (connection.PubKey == nullptr) {
      error = "*** Invalid public key from " + name;
      Transport::Send(connection.Socket, error + "\r\n");
      _logger.Error(error);
    }
  }

  string old_name = connection.Name;
  string db_pubkey = _db.Get(name);

  if (db_pubkey.length() == 0) {
    message.clear();
    _logger.Info("No user " + name + " in the db-- creating");
    _db.Set(name, pubkey_string);
    connection.Name = name;
    connection.LoggedIn = true;
    Transport::Send(connection.Socket,
                    "Successfully claimed user name " + name + "\r\n");
    show_renamed_message = true;
  } else if (db_pubkey.length() > 0) {
    message.clear();

    if (pubkey_string.compare(db_pubkey) != 0) {
      error = "*** Mismatched public key for " + name;
      _logger.Info(error);
      Transport::Send(connection.Socket, error + "\r\n");
    } else {
      _logger.Info("Authenticating " + name + "...");
      bool authenticated = Authenticate(pubkey_string, connection);
      if (!authenticated) {
        error = "*** Public key verification failed for " + name;
        _logger.Info(error);
        Transport::Send(connection.Socket, error + "\r\n");
      } else {
        connection.Name = name;
        _logger.Info("Successfully authenticated " + name);
        Transport::Send(connection.Socket,
                        "Successfully authenticated as " + name + "\r\n");

        if (!connection.LoggedIn) {
          Transport::Send(connection.Socket, _motd);
          connection.LoggedIn = true;
          connection.Color = NextColor();
        } else {
          // TODO kick guest users around here if server is set to private
          show_renamed_message = true;
        }
      }
    }
  }

  if (show_entered_message) {
    connection.Color = NextColor();
    Broadcast("* " + connection.Color + connection.Name + _uncolor +
              " has entered the chat (" + connection.Address + ")\r\n");
    message.clear();
  } else if (show_renamed_message) {
    Broadcast(message = "* " + connection.Color + old_name + _uncolor + " (" +
                        connection.Address + ") renamed to " +
                        connection.Color + connection.Name + _uncolor + "\r\n");
    message.clear();
  }

  return message;
}

bool Server::Authenticate(string pubkey_string, Connection &connection) {
  try {
    DCRYPT_PKEY *pubkey =
        RSAStringToKey((unsigned char *)pubkey_string.c_str(), false);
    connection.PubKey = pubkey;

    string seed = GenerateSeed(_seed_length);
    Transport::Send(connection.Socket, "/verify " + seed + "\r\n");
    string response = Transport::ReadLine(connection.Socket);

    if (Transport::ParseVerifyMessage(response)) {
      bool verified = RSAVerify((char *)seed.c_str(),
                                (unsigned char *)response.c_str(), pubkey);
      if (!verified) {
        return false;
      }

      connection.PubKey = pubkey;
      return true;
    }
    return false;
  } catch (std::exception &e) {
    _logger.Error(e.what());
    return false;
  }
}

void Server::Broadcast(string message) {
  asio::error_code ignored;
  unique_lock<mutex> sockets_lock(_sockets_mutex);
  auto socket = _sockets.begin();
  while (socket != _sockets.end()) {
    Transport::Send(*socket, message);
    socket++;
  }
  sockets_lock.unlock();
}

string Server::NextColor() {
  _name_color = (_name_color + 1) % 8;
  return _colors[_name_color].code;
}

string Server::GetAddress(tcp::socket &socket) {
  return socket.remote_endpoint().address().to_string() + ":" +
         std::to_string(socket.remote_endpoint().port());
}

string Server::GenerateSeed(int length) {
  unsigned char *bytes = GenerateRandomBytes(length);
  char seed[length];
  int modulus = _alphanumeric.length() - 1;

  for (int i = 0; i < length; i++) {
    seed[i] = _alphanumeric[(int)bytes[i] % modulus];
  }
  seed[length] = '\0';

  free(bytes);
  return string(seed, length);
}

int Server::Disconnect(tcp::socket &socket) {
  string address = GetAddress(socket);
  string user_name;

  auto connection = _connections.begin();
  while (connection != _connections.end()) {
    if (connection->Address == address) {
      auto socket = _sockets.begin();
      while (socket != _sockets.end()) {
        if (&(*socket) == &(connection->Socket)) {
          unique_lock<mutex> sockets_lock(_sockets_mutex);
          _sockets.erase(socket);
          sockets_lock.unlock();
          break;
        }
        socket++;
      }
      user_name = connection->Name;
      unique_lock<mutex> connections_lock(_connections_mutex);
      _connections.erase(connection);
      connections_lock.unlock();

      break;
    }
    connection++;
  }

  _logger.Info("Received disconnect from " + address);
  Broadcast(connection->Color + user_name + _uncolor + " has left the chat (" +
            address + ")\r\n");

  return 0;
}

void Server::Stop() {
  unique_lock<std::mutex> connections_lock(_connections_mutex);
  unique_lock<mutex> sockets_lock(_sockets_mutex);

  while (_connections.size() > 0) {
    Connection c = _connections.front();
    _logger.Info("Terminating connection with " + c.Name + " (" + c.Address +
                 ")");
    _connections.pop_front();
  }

  while (_sockets.size() > 0) {
    _sockets.front().close();
    _sockets.pop_front();
  }

  _running = 0;
}

}  // namespace TCPC
