#include "server.h"

#include <leveldb/db.h>

#include <asio.hpp>
#include <iostream>
#include <list>
#include <mutex>
#include <regex>
#include <string>
#include <thread>

#include "../lib/crypto.h"
#include "../lib/filesystem.h"
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

    Send(connection.Socket, _motd);
    Broadcast(connection.Name + " has entered the chat (" + connection.Address +
              ")\r\n");

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
    asio::streambuf buf;
    std::string message = "";

    try {
      message = ReadLine(socket, buf);
    } catch (std::exception &e) {
      connected = Disconnect(socket);
      return;
    }

    if (message.front() == '/') {
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
  std::string error = "";
  std::smatch key_match;
  std::regex key_regex("[A-Za-z0-9/\?\+]+\/\/");
  std::regex_search(message, key_match, key_regex);
  std::string pubkey_string = Crypto::PubKeyToString(connection.PubKey);
  bool got_valid_pubkey = false;

  if (key_match.length() > 0) {
    std::string match = key_match.str();
    pubkey_string = Crypto::ExpandNewLines(match);
    _logger.Info("Got public key from " + connection.Name + "(" +
                 connection.Address + ")");
    try {
      connection.PubKey = Crypto::StringToPubKey(pubkey_string);
      got_valid_pubkey = true;
    } catch (std::exception &e) {
      error = "*** Invalid public key from " + name;
      Send(connection.Socket, error + "\r\n");
      _logger.Error(error);
    }
  }

  std::string connection_pubkey = Crypto::PubKeyToString(connection.PubKey);
  std::string old_name = connection.Name;
  std::string db_pubkey = _db.Get(name);

  if (db_pubkey.length() == 0) {  // no user in db, free to create
    _logger.Info("No user " + name + " in the db-- creating");
    _db.Set(name, connection_pubkey);
    connection.Name = name;

  } else if (db_pubkey.length() > 0) {
    if (connection_pubkey.compare(db_pubkey) != 0) {
      error = "*** Mismatched public key for " + name;
      _logger.Info(error);
      Send(connection.Socket, error + "\r\n");
    } else {
      _logger.Info("Authenticating " + name + "...");
      bool authenticated = Authenticate(pubkey_string, connection);
      if (authenticated == true) {
        connection.Name = name;
        _logger.Info("Successfully authenticated " + name);
      } else {
        error = "*** Public key verification failed for " + name;
        _logger.Info(error);
        Send(connection.Socket, error);
      }
    }
  }

  message.clear();

  if (connection.Name.compare("guest") != 0 &&
      connection.Name.compare(old_name) != 0) {
    message = old_name + " (" + connection.Address + ") renamed to " +
              connection.Name + "\n";
  }

  return message;
}

bool Server::Authenticate(std::string pubkey_string, Connection &connection) {
  try {
    RSA::PublicKey pubkey = Crypto::StringToPubKey(pubkey_string);

    // TODO do some kind of verification-- ask the user to sign something?
    std::string nonce = "foobar";
    asio::streambuf buf;

    Send(connection.Socket, "/verify " + nonce + "\r\n");
    std::string response = ReadLine(connection.Socket, buf);

    std::smatch signature_match;
    std::regex signature_regex("\/verify .*");
    std::regex_search(response, signature_match, signature_regex);

    if (signature_match.length() > 0) {
      response = std::regex_replace(response, std::regex("\/verify "), "");
      response = std::regex_replace(response, std::regex("\r"), "");
      response = std::regex_replace(response, std::regex("\n"), "");
    }

    bool verified = Crypto::Verify(response, pubkey);

    if (!verified) {
      return false;
    } else {
      connection.PubKey = pubkey;
      return true;
    }
  } catch (std::exception &e) {
    _logger.Error(e.what());
    return false;
  }
}

std::string Server::ReadLine(tcp::socket &socket, asio::streambuf &buf) {
  asio::read_until(socket, buf, "\n");
  asio::streambuf::const_buffers_type bufs = buf.data();
  std::string message(buffers_begin(bufs), buffers_begin(bufs) + buf.size());
  return message;
}

void Server::Send(tcp::socket &socket, std::string message) {
  asio::error_code code;
  asio::write(socket, asio::buffer(message), code);
}

void Server::Broadcast(std::string message) {
  asio::error_code ignored;
  unique_lock<mutex> sockets_lock(_sockets_mutex);
  auto socket = _sockets.begin();
  while (socket != _sockets.end()) {
    Send(*socket, message);
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
