#include "server.h"

#include <leveldb/db.h>

#include <asio.hpp>
#include <iostream>
#include <list>
#include <mutex>
#include <regex>
#include <string>
#include <thread>

#include "../lib/filesystem.h"
#include "../lib/socket.h"
#include "connection.h"

using asio::ip::tcp;
using std::mutex;
using std::unique_lock;

namespace TCPChat {

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

std::string Server::LoadMOTD(std::string path) {
  std::string motd = Filesystem::FileToString(path);
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
        "│                   welcome to tcpchat !                   │\r\n"
        "│                                                          │\r\n"
        "│                                                          │\r\n"
        "└──────────────────────────────────────────────────────────┘\r\n";
  }

  return motd;
}

void Server::Handle(tcp::socket &socket, Connection &connection) {
  int connected = 1;
  while (connected == 1) {
    std::string message = "";

    if (!connection.LoggedIn && connection.Name.compare("guest") == 0) {
      connection.LoggedIn = true;
      Socket::Send(connection.Socket, _motd);
    }

    try {
      message = Socket::ReadLine(socket);
    } catch (std::exception &e) {
      connected = Disconnect(socket);
      return;
    }

    _logger.Info("*************");
    _logger.Info(message);

    char first_char = message.front();

    if (first_char == '\n' || first_char == '\r') {
      message = message.substr(1, message.length() - 1);
    }

    if (first_char == '/') {
      message = ParseSlashCommand(message, connection);
    }

    if (message.length() > 0) {
      Broadcast("[" + connection.Name + "] " +
                message.substr(0, message.length() - 1) + "\r\n");
    }
  }
}

std::string Server::ParseSlashCommand(std::string message,
                                      Connection &connection) {
  std::smatch name_match;
  std::regex name_regex("[A-Za-z0-9]+");
  std::regex_search(message, name_match, name_regex);

  if (name_match.length() > 0) {
    message = SetUser(name_match.str(), message, connection);
  }

  return message;
}

std::string Server::SetUser(std::string name, std::string message,
                            Connection &connection) {
  bool show_entered_message = true;
  std::string error = "";
  std::smatch key_match;
  std::regex key_regex(
      "-----BEGIN PUBLIC KEY-----.*-----END PUBLIC KEY-----\\?");
  std::regex_search(message, key_match, key_regex);

  // TODO properly auth if client is run with -n flag

  // TODO wrap this is some condition
  // if this is null, that's fine, just continue
  char *pubkey_string_or_null =
      (char *)RSAKeyToString(connection.PubKey, false);

  std::string pubkey_string = pubkey_string_or_null != nullptr
                                  ? std::string(pubkey_string_or_null)
                                  : "";

  pubkey_string = std::regex_replace(pubkey_string, std::regex("\n$"), "");

  _logger.Info("1. pubkey_string length from connection.PubKey: " +
               std::to_string(pubkey_string.length()));

  _logger.Info("pubkey_string length from connection.PubKey: " +
               std::to_string(pubkey_string.length()));

  if (key_match.length() == 0) {
    show_entered_message = false;
  } else {
    std::string match = key_match.str();

    pubkey_string = Socket::ExpandNewLines(match);

    _logger.Info("2. key_match: " + std::to_string(key_match.length()));
    _logger.Info(match);
    _logger.Info("2. match: " + std::to_string(match.length()));

    _logger.Info("Got public key from " + connection.Name + "(" +
                 connection.Address + ")");

    _logger.Info("2. Setting connection.Pubkey-- pubkey_string length: " +
                 std::to_string(pubkey_string.length()));

    connection.PubKey =
        RSAStringToKey((unsigned char *)pubkey_string.c_str(), false);
    if (connection.PubKey == nullptr) {
      error = "*** Invalid public key from " + name;
      Socket::Send(connection.Socket, error + "\r\n");
      _logger.Error(error);
    }
  }

  std::string old_name = connection.Name;
  std::string db_pubkey = _db.Get(name);

  if (db_pubkey.length() == 0) {  // no user in db, free to create
    _logger.Info("3. Creating user with key_match length: " +
                 std::to_string(key_match.length()));
    _logger.Info("3. Creating user with pubkey_string length: " +
                 std::to_string(pubkey_string.length()));

    message.clear();
    _logger.Info("No user " + name + " in the db-- creating");
    _db.Set(name, pubkey_string);
    connection.Name = name;
    connection.LoggedIn = true;
    Socket::Send(connection.Socket,
                 "Successfully claimed user name " + name + "\r\n");
    if (!show_entered_message) {
      message = old_name + " (" + connection.Address + ") renamed to " +
                connection.Name + "\n";
    }
  } else if (db_pubkey.length() > 0) {
    _logger.Info("4. *** pubkey_string:");
    for (int i = 0; i < pubkey_string.length(); i++) {
      printf("%x", pubkey_string[i]);
    }
    printf("\n");

    _logger.Info("4. pubkey_string length: " +
                 std::to_string(pubkey_string.length()));

    _logger.Info("4. db_pubkey:");
    for (int i = 0; i < db_pubkey.length(); i++) {
      printf("%x", db_pubkey[i]);
    }
    printf("\n");

    _logger.Info("4. db pubkey length: " + std::to_string(db_pubkey.length()));
    _logger.Info("4. keys are equal(0)? " +
                 std::to_string(pubkey_string.compare(db_pubkey)));

    message.clear();
    // TODO handle all instances of Send() or ReadLine() for response.length 0

    _logger.Info("*** pubkey_string:");
    for (int i = 0; i < pubkey_string.length(); i++) {
      printf("%x", pubkey_string[i]);
    }
    printf("\n");

    _logger.Info("pubkey_string length: " +
                 std::to_string(pubkey_string.length()));

    _logger.Info("*** db_pubkey:");
    for (int i = 0; i < db_pubkey.length(); i++) {
      printf("%x", db_pubkey[i]);
    }
    printf("\n");

    _logger.Info("db pubkey length: " + std::to_string(db_pubkey.length()));
    _logger.Info(std::to_string(pubkey_string.compare(db_pubkey)));

    if (pubkey_string.compare(db_pubkey) != 0) {
      error = "*** Mismatched public key for " + name;
      _logger.Info(error);
      Socket::Send(connection.Socket, error + "\r\n");
    } else {
      _logger.Info("Authenticating " + name + "...");
      bool authenticated = Authenticate(pubkey_string, connection);
      if (!authenticated) {
        error = "*** Public key verification failed for " + name;
        _logger.Info(error);
        Socket::Send(connection.Socket, error + "\r\n");
      } else {
        connection.Name = name;
        _logger.Info("Successfully authenticated " + name);
        Socket::Send(connection.Socket,
                     "Successfully authenticated you as " + name + "\r\n");

        if (!connection.LoggedIn) {
          Socket::Send(connection.Socket, _motd);
          connection.LoggedIn = true;
          Broadcast(connection.Name + " has entered the chat (" +
                    connection.Address + ")\r\n");
          show_entered_message = false;

        } else if (connection.Name.compare(old_name) != 0) {
          // TODO kick guest users around here if server is set to private
          if (!show_entered_message) {
            message = old_name + " (" + connection.Address + ") renamed to " +
                      connection.Name + "\n";
          }
        }
      }
    }
  }

  if (show_entered_message) {
    Broadcast(connection.Name + " has entered the chat (" + connection.Address +
              ")\r\n");
  }

  return message;
}

bool Server::Authenticate(std::string pubkey_string, Connection &connection) {
  try {
    DCRYPT_PKEY *pubkey =
        RSAStringToKey((unsigned char *)pubkey_string.c_str(), false);
    connection.PubKey = pubkey;

    std::string seed = GenerateSeed(_seed_length);
    Socket::Send(connection.Socket, "/verify " + seed + "\r\n");
    std::string response = Socket::ReadLine(connection.Socket);

    // TODO why is response only 512 bytes here if key is 4096? should it be
    // "/verify " + 512?

    if (Socket::ParseVerifyMessage(response)) {
      int key_size = RSAKeySize(pubkey);
      _logger.Info(std::to_string(key_size));
      _logger.Info(std::to_string(response.length()));

      unsigned char *response_bytes =
          (unsigned char *)calloc(key_size, sizeof(unsigned char));

      // for (int i = 0; i < key_size; i++) {
      //   response_bytes[i] = (unsigned char)response[i];
      //   printf("i: %d rb: %x r: %x\n", i, (unsigned char)response_bytes[i],
      //          (unsigned char)response[i]);
      // }

      bool verified = RSAVerify((char *)seed.c_str(), response_bytes, pubkey);

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
}  // namespace TCPChat

void Server::Broadcast(std::string message) {
  asio::error_code ignored;
  unique_lock<mutex> sockets_lock(_sockets_mutex);
  auto socket = _sockets.begin();
  while (socket != _sockets.end()) {
    Socket::Send(*socket, message);
    socket++;
  }
  sockets_lock.unlock();
}

int Server::Disconnect(tcp::socket &socket) {
  std::string address = GetAddress(socket);
  std::string user_name;

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
  Broadcast(user_name + " has left the chat (" + address + ")\r\n");

  return 0;
}

std::string Server::GetAddress(tcp::socket &socket) {
  return socket.remote_endpoint().address().to_string() + ":" +
         std::to_string(socket.remote_endpoint().port());
}

std::string Server::GenerateSeed(int length) {
  unsigned char *bytes = GenerateRandomBytes(length);
  char hex_string[length];
  int modulus = _alphanumeric.length() - 1;

  for (int i = 0; i < length; i++) {
    hex_string[i] = _alphanumeric[(int)bytes[i] % modulus];
  }
  hex_string[length] = '\0';

  return std::string(hex_string);
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

}  // namespace TCPChat
