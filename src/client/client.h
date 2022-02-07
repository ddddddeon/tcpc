#include <crypto++/rsa.h>

#include <asio.hpp>
#include <iostream>
#include <mutex>
#include <string>

#include "../lib/logger.h"

using asio::ip::tcp;
using CryptoPP::RSA;

#define MAX_INPUT_BUFFER_SIZE 65535

class Client {
 private:
  Logger _logger;
  tcp::socket *_socket;
  asio::error_code _ignored;
  std::string _user_input;
  std::mutex _user_input_mutex;
  RSA::PublicKey _pubkey;
  RSA::PrivateKey _privkey;
  int _term_width;

  void ReadMessages(tcp::socket &socket);
  void ProcessInputChar();

 public:
  std::string Host;
  int Port;
  std::string Name;

  Client(std::string host, int port, std::string name, Logger &logger)
      : _logger(logger), Host(host), Port(port), Name(name) {
    _user_input.reserve(MAX_INPUT_BUFFER_SIZE);

    // TODO check for existing keypair
    GenerateKeyPair();
  }

  void Connect();
  void LoadKeyPair();
  void GenerateKeyPair();
};