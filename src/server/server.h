#ifndef SERVER_H
#define SERVER_H

#include <dcrypt.h>

#include <asio.hpp>
#include <list>
#include <mutex>
#include <string>

#include "../lib/db.h"
#include "../lib/logger.h"
#include "connection.h"

using asio::ip::tcp;

namespace TCPChat {

struct Color {
  std::string name;
  std::string code;
  std::string bold_code;
};

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
  int _seed_length = 32;
  std::string const _alphanumeric =
      "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789";
  Color _colors[8] = {{"red", "\033[22;31m", "\033[01;31m"},
                      {"blue", "\033[22;34m", "\033[01;34m"},
                      {"green", "\033[22;32m", "\033[01;32m"},
                      {"magenta", "\033[22;35m", "\033[01;35m"},
                      {"yellow", "\033[22;33m", "\033[01;33m"},
                      {"cyan", "\033[22;36m", "\033[01;36m"},
                      {"white", "\033[22;37m", "\033[01;37m"},
                      {"none", "\033[22;0m", "\033[22;0m"}};
  int _name_color = 7;
  std::string _uncolor = _colors[7].code;

  std::string LoadMOTD(std::string path);
  void Handle(tcp::socket &socket, Connection &connection);
  std::string ParseSlashCommand(std::string message, Connection &connection);
  std::string SetUser(std::string name, std::string message,
                      Connection &connection);
  bool Authenticate(std::string pubkey_string, Connection &connection);
  void Broadcast(std::string message);
  std::string NextColor();
  std::string GetAddress(tcp::socket &socket);
  std::string GenerateSeed(int length);
  int Disconnect(tcp::socket &socket);

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
