#ifndef SERVER_H
#define SERVER_H

#include <asio.hpp>
#include <list>
#include <mutex>
#include <string>

#include "../lib/logger.h"
#include "connection.h"

using asio::ip::tcp;

class Server {
 private:
  Logger _logger;
  int _running;
  int _port;
  asio::ip::address_v4 _interface;
  std::list<tcp::socket> _sockets;
  std::list<Connection> _connections;
  std::list<std::thread> _threads;
  std::mutex _sockets_mutex;
  std::mutex _connections_mutex;
  std::mutex _threads_mutex;

  void Handle(tcp::socket &socket, Connection &connection);
  void Broadcast(std::string str);
  int Disconnect(tcp::socket &socket);
  std::string GetAddress(tcp::socket &socket);

 public:
  Server(asio::ip::address_v4 interface, int port, Logger &logger)
      : _logger(logger), _running(0), _port(port), _interface(interface) {}

  void Start();
  void Stop();
};

#endif /* !SERVER_H */
