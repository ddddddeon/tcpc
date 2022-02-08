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
#include "../lib/db.h"
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
    Broadcast(connection.Name + " has entered the chat (" + connection.Address +
              ")\r\n");

    unique_lock<mutex> threads_lock(_threads_mutex);
    _threads.push_front(std::thread(&Server::Handle, this, std::ref(socket),
                                    std::ref(connection)));
    threads_lock.unlock();
  }
}

void Server::Handle(tcp::socket &socket, Connection &connection) {
  int connected = 1;
  while (connected == 1) {
    asio::streambuf buf;

    try {
      size_t s = asio::read_until(socket, buf, "\n");
    } catch (std::exception &e) {
      connected = Disconnect(socket);
      return;
    }

    asio::streambuf::const_buffers_type bufs = buf.data();
    std::string message(buffers_begin(bufs), buffers_begin(bufs) + buf.size());

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
  // TODO change this to look for "/name foobar" instead of "/foobar"
  std::regex name_regex("[A-Za-z0-9]+");
  std::regex_search(message, name_match, name_regex);

  if (name_match.length() > 0) {
    message = SetUser(name_match.str(), message, connection);
  }

  return message;
}

std::string Server::SetUser(std::string name, std::string message,
                            Connection &connection) {
  std::string old_name = connection.Name;

  /* TODO check here for the name against a leveldb kv store
   * - check if the currect connection already has a pubkey set
   * - look up the new name in db, check if it exists
   *   - if exists in db AND a pubkey is set for this connection, check if
   *     pubkeys match
   *   - if exists and pubkey is not set - tell user that the name exists and
   *     to specify a pubkey on launch to auth with that name
   *   - if doesn't exist AND pubkey not set - generate a new pubkey and write
   *     to db (name, pubkey_string)
   *   - if doesn't exist AND pubkey set - write to db (name, pubkey_string)
   */

  std::smatch key_match;
  std::regex key_regex("[A-Za-z0-9/\?\+]+\/\/");
  std::regex_search(message, key_match, key_regex);

  if (key_match.length() > 0) {
    std::string match = key_match.str();
    std::string pubkey_string =
        std::regex_replace(match, std::regex("\\?"), "\n");

    _logger.Info("Authenticating user " + name + "...");
    bool authenticated = Authenticate(pubkey_string, connection);
    if (authenticated == true) {
      _logger.Info("Successfully authenticated user " + name);
    } else {
      _logger.Info("Failed to authenticate user " + name);
      // TODO alert the client that authentication failed
      connection.Name = "guest";
    }
  }

  // TODO only if the above specified conditions are met, set the username
  connection.Name = name;

  message.clear();

  if (connection.Name.compare("guest") != 0) {
    message = old_name + " (" + connection.Address + ") renamed to " +
              connection.Name + "\n";
  }

  return message;
}

bool Server::Authenticate(std::string pubkey_string, Connection &connection) {
  Crypto crypto;
  try {
    RSA::PublicKey pubkey = crypto.StringToPubKey(pubkey_string);

    // TODO do some kind of verification, check against a db, etc
    connection.PubKey = pubkey;
    return true;
  } catch (std::exception &e) {
    _logger.Error(e.what());
    return false;
  }
}

void Server::Broadcast(std::string str) {
  asio::error_code ignored;
  unique_lock<mutex> sockets_lock(_sockets_mutex);
  auto socket = _sockets.begin();
  while (socket != _sockets.end()) {
    asio::write(*socket, asio::buffer(str), ignored);
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
