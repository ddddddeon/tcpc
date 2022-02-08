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
  void Authenticate();

 public:
  std::string Host;
  int Port;
  std::string Name;

  Client(std::string host, int port, std::string name, Logger &logger,
         bool generate_key_pair)
      : _logger(logger), Host(host), Port(port), Name(name) {
    _user_input.reserve(MAX_INPUT_BUFFER_SIZE);

    if (generate_key_pair) {
      GenerateKeyPair();
    }
  }

  void Connect();
  bool LoadKeyPair(std::string path);
};

}  // namespace TCPChat
