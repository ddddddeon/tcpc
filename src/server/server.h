#ifndef SERVER_H
#define SERVER_H

#include <asio.hpp>
#include <list>
#include <mutex>
#include <string>

#include "../lib/db.h"
#include "../lib/logger.h"
#include "connection.h"
using asio::ip::tcp;

namespace TCPChat {

class Server {
 private:
  Logger _logger;
  DB _db;
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
  std::string ParseSlashCommand(std::string message, Connection &connection);
  std::string SetUser(std::string name, std::string message,
                      Connection &connection);
  bool Authenticate(std::string pubkey_string, Connection &connection);
  void Broadcast(std::string str);
  int Disconnect(tcp::socket &socket);
  std::string GetAddress(tcp::socket &socket);

 public:
  Server(asio::ip::address_v4 interface, int port, Logger &logger, DB &db)
      : _logger(logger),
        _db(db),
        _running(0),
        _port(port),
        _interface(interface) {}

  void Start();
  void Stop();
};

}  // namespace TCPChat

#endif /* !SERVER_H */
