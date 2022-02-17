#include <dcrypt.h>

#include <asio.hpp>
#include <iostream>
#include <mutex>
#include <string>

#include "../lib/logger.h"

using asio::ip::tcp;

#define MAX_INPUT_BUFFER_SIZE 65535

namespace TCPChat {

class Client {
 private:
  Logger _logger;
  tcp::socket *_socket;
  std::string _user_input;
  std::mutex _user_input_mutex;
  DCRYPT_PKEY *_privkey = nullptr;
  DCRYPT_PKEY *_pubkey = nullptr;
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
  std::string KeyPairPath = "./";
  std::string PrivKeyFileName = "id_rsa";
  std::string PubKeyFileName = "id_rsa.pub";

  Client(std::string host, int port, std::string name, bool generate_keypair,
         std::string keypair_path, Logger &logger)
      : _logger(logger),
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
