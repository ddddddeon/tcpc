#ifndef CONNECTION_H
#define CONNECTION_H

#include <dcrypt.h>

#include <asio.hpp>
#include <string>

using asio::ip::tcp;

namespace TCPChat {

class Connection {
 public:
  std::string Name;
  std::string Address;
  tcp::socket &Socket;
  std::string Color;
  DCRYPT_PKEY *PubKey = nullptr;
  bool LoggedIn;

  Connection(tcp::socket &socket, std::string name, std::string address)
      : Name(name), Address(address), Socket(socket), LoggedIn(false) {}

  ~Connection() { DCRYPT_PKEY_free(PubKey); }
};

}  // namespace TCPChat

#endif /* !CONNECTION_H */