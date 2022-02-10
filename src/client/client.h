#include <crypto++/rsa.h>

#include <asio.hpp>
#include <iostream>
#include <mutex>
#include <string>

#include "../lib/logger.h"

using asio::ip::tcp;
using CryptoPP::RSA;

#define MAX_INPUT_BUFFER_SIZE 65535

namespace TCPChat {

class Client {
 private:
  Logger _logger;
  tcp::socket *_socket;
  std::string _user_input;
  std::mutex _user_input_mutex;
  RSA::PrivateKey _privkey;
  RSA::PublicKey _pubkey;
  std::string _pubkey_string;
  int _term_width;

  void ReadMessages(tcp::socket &socket);
  void ProcessInputChar();
  void GenerateKeyPair();
  bool Authenticate();
  bool Verify(std::string message);

 public:
  std::string Host;
  int Port;
  std::string Name;
  std::string KeyPairPath;

  Client(std::string host, int port, std::string name, bool generate_keypair,
         std::string keypair_path, Logger &logger)
      : _logger(logger),
        _socket(nullptr),
        Host(host),
        Port(port),
        Name(name),
        KeyPairPath(keypair_path) {
    _user_input.reserve(MAX_INPUT_BUFFER_SIZE);
    if (generate_keypair) {
      GenerateKeyPair();
    }
  }

  void Connect();
  bool LoadKeyPair(std::string path);
};

}  // namespace TCPChat
