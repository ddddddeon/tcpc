#include "main.h"

#include <getopt.h>
#include <signal.h>
#include <sys/stat.h>

#include <iostream>
#include <string>

#include "../lib/filesystem.h"
#include "../lib/logger.h"
#include "client.h"

using namespace TCPChat;

namespace ClientConfig {

bool GenerateKeyPair = false;
std::string KeyPairPath = "./";
int KeyLength = 4096;
std::string Name = "guest";
std::string Host = "127.0.0.1";
int Port = 9000;

}  // namespace ClientConfig

int main(int argc, char *argv[]) {
  Logger logger;
  parse_args(argc, argv);

  if (ClientConfig::KeyPairPath.back() != '/') {
    ClientConfig::KeyPairPath += '/';
  }

  if (!Filesystem::FileExists(ClientConfig::KeyPairPath + "id_rsa") &&
      !Filesystem::FileExists(ClientConfig::KeyPairPath + "id_rsa.pub")) {
    logger.Raw(
        "*** No keypair found at " + ClientConfig::KeyPairPath +
        "\nWould you like to generate a keypair at this location now?\n[y/n] ");

    std::string yn = "";
    std::cin >> yn;

    // TODO fix newline formatting after this

    if (yn.front() == 'y' || yn.front() == 'Y') {
      ClientConfig::GenerateKeyPair = true;
    } else {
      logger.Line(
          "*** Please specify a filepath where the files `id_rsa` and "
          "`id_rsa.pub` can be\n"
          "found, using the -k flag.\n"
          "Or, run this program with the -g flag to generate a new keypair,"
          " in a directory\n"
          "specified by the -k flag (default is " +
          ClientConfig::KeyPairPath + ")");
      exit(1);
    }
  } else if (ClientConfig::GenerateKeyPair) {
    logger.Line(
        "*** Keypair files exist already at " + ClientConfig::KeyPairPath +
        "\nOverwriting these files would mean you will no longer be "
        "able to authenticate\n"
        "with ANY usernames you've used in the past. I'm afraid I can't "
        "let you do that.\n\n"
        "*** If you truly want to delete your keypair, "
        "delete them yourself from the\n"
        "filesystem and re-run this program with the -g flag "
        "or specify a different\n"
        "directory with the -k flag.");
    exit(1);
  }

  // TODO pass in a ClientConfig struct instead
  Client client(ClientConfig::Host, ClientConfig::Port, ClientConfig::Name,
                ClientConfig::GenerateKeyPair, ClientConfig::KeyPairPath,
                ClientConfig::KeyLength, logger);

  if (!ClientConfig::GenerateKeyPair) {
    logger.Info("Loading keypair from " + ClientConfig::KeyPairPath);
    bool loaded = client.LoadKeyPair(ClientConfig::KeyPairPath);

    if (!loaded) {
      logger.Error(
          "*** Could not load keypair from disk! Check file permissions?");
      exit(1);
    }
  }

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
  while ((opt = getopt(argc, argv, ":h:p:n:gk:l:")) != -1) {
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
      case 'l':
        ClientConfig::KeyLength = atoi(optarg);
    }
  }
}
