#include "main.h"

#include <getopt.h>
#include <signal.h>

#include <iostream>
#include <string>

#include "../lib/logger.h"
#include "client.h"

using namespace TCPChat;

namespace ClientConfig {

bool GenerateKeyPair = false;
std::string KeyPairPath = "./";
std::string Name = "guest";
std::string Host = "127.0.0.1";
int Port = 9000;

}  // namespace ClientConfig

int main(int argc, char *argv[]) {
  Logger logger;

  parse_args(argc, argv);
  Client client(ClientConfig::Host, ClientConfig::Port, ClientConfig::Name,
                ClientConfig::GenerateKeyPair, ClientConfig::KeyPairPath,
                logger);

  if (!ClientConfig::GenerateKeyPair) {
    logger.Info("Loading keypair from " + ClientConfig::KeyPairPath);

    if (ClientConfig::KeyPairPath.back() != '/') {
      ClientConfig::KeyPairPath += '/';
    }

    logger.Info(ClientConfig::KeyPairPath);

    bool loaded = client.LoadKeyPair(ClientConfig::KeyPairPath);

    if (!loaded) {
      logger.Error("Could not load keypair from disk - exiting");
      exit(1);
    }
  }
  // TODO check if we're overwriting an existing keypair on disk

  try {
    client.Connect();
  } catch (std::exception &e) {
    logger.Error(e.what());
    std::cout << '\r' << std::flush;
    system("stty -raw");
  }
}

void parse_args(int argc, char *argv[]) {
  std::string localhost = "localhost";

  int opt;
  while ((opt = getopt(argc, argv, ":h:p:n:gk:")) != -1) {
    switch (opt) {
      case 'h':
        if (localhost.compare(optarg) != 0) {
          ClientConfig::Host = optarg;
        }
        break;
      case 'p':
        ClientConfig::Port = atoi(optarg);
        break;
      case 'n':
        ClientConfig::Name = optarg;
        break;
      case 'g':
        ClientConfig::GenerateKeyPair = true;
        break;
      case 'k':
        ClientConfig::KeyPairPath = optarg;
        break;
    }
  }
}
