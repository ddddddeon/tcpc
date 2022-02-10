#ifndef CONNECTION_H
#define CONNECTION_H

#include <crypto++/rsa.h>

#include <asio.hpp>

using asio::ip::tcp;
using CryptoPP::RSA;

namespace TCPChat {

class Connection {
 public:
  std::string Name;
  std::string Address;
  tcp::socket &Socket;
  RSA::PublicKey PubKey;
  bool Connected;

  Connection(tcp::socket &socket, std::string name, std::string address)
      : Name(name), Address(address), Socket(socket), Connected(true) {}
};

}  // namespace TCPChat

#endif /* !CONNECTION_H */