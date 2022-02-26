#include <dcrypt.h>

#include <asio.hpp>
#include <iostream>
#include <mutex>
#include <string>

#include "../lib/logger.h"

using asio::ip::tcp;

#define MAX_INPUT_BUFFER_SIZE 65535

namespace TCPChat {

struct ClientConfig {
  bool GenerateKeyPair;
  std::string KeyPairPath;
  int KeyLength;
  std::string Name;
  std::string Host;
  int Port;
};

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
  void Verify(std::string response);

 public:
  std::string Host;
  int Port;
  std::string Name;
  std::string KeyPairPath = "./";
  std::string PrivKeyFileName = "./id_rsa";
  std::string PubKeyFileName = "./id_rsa.pub";
  int KeyLength;

  Client(ClientConfig &config, Logger &logger)
      : _logger(logger),
        Host(config.Host),
        Port(config.Port),
        Name(config.Name),
        KeyPairPath(config.KeyPairPath),
        KeyLength(config.KeyLength) {
    _user_input.reserve(MAX_INPUT_BUFFER_SIZE);

    PrivKeyFileName = KeyPairPath + "id_rsa";
    PubKeyFileName = KeyPairPath + "id_rsa.pub";

    if (config.GenerateKeyPair) {
      GenerateKeyPair();
    }
  }

  void Connect();
  bool LoadKeyPair(std::string path);
};

}  // namespace TCPChat
