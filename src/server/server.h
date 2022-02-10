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
  std::string _motd_path;
  std::string _motd;

  std::string LoadMOTD(std::string path);
  void Handle(tcp::socket &socket, Connection &connection);
  std::string ParseSlashCommand(std::string message, Connection &connection);
  std::string SetUser(std::string name, std::string message,
                      Connection &connection);
  bool Authenticate(std::string pubkey_string, Connection &connection);
  void Send(tcp::socket &socket, std::string message, asio::error_code &code);
  void Broadcast(std::string message);
  int Disconnect(tcp::socket &socket);
  std::string GetAddress(tcp::socket &socket);

 public:
  Server(asio::ip::address_v4 interface, int port, DB &db,
         std::string motd_path, Logger &logger)
      : _logger(logger),
        _db(db),
        _running(0),
        _port(port),
        _interface(interface),
        _motd(LoadMOTD(motd_path)) {}

  void Start();
  void Stop();
};

}  // namespace TCPChat

#endif /* !SERVER_H */
