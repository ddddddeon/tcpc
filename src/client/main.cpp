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

ClientConfig config = {
    false,        // GenerateKeyPair
    "./",         // KeyPairPath
    4096,         // KeyLength
    "guest",      // Name
    "127.0.0.1",  // Host
    9000          // Port
};

int main(int argc, char *argv[]) {
  Logger logger;
  parse_args(argc, argv);

  if (config.KeyPairPath.back() != '/') {
    config.KeyPairPath += '/';
  }

  if (!Filesystem::FileExists(config.KeyPairPath + "id_rsa") &&
      !Filesystem::FileExists(config.KeyPairPath + "id_rsa.pub")) {
    logger.Raw("*** No keypair found at " + config.KeyPairPath +
               "\nIf you already have a keypair elsewhere, specify a directory"
               "\nwith the -k flag.\n"
               "Generating new keypair...\n");

    config.GenerateKeyPair = true;
  } else if (config.GenerateKeyPair) {
    logger.Line(
        "*** Keypair files exist already at " + config.KeyPairPath +
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

  Client client(config, logger);

  if (!config.GenerateKeyPair) {
    logger.Info("Loading keypair from " + config.KeyPairPath);
    bool loaded = client.LoadKeyPair(config.KeyPairPath);

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
          config.Host = optarg;
        }
        break;
      case 'p':
        config.Port = atoi(optarg);
        break;
      case 'n':
        config.Name = optarg;
        break;
      case 'g':
        config.GenerateKeyPair = true;
        break;
      case 'k':
        config.KeyPairPath = optarg;
        break;
      case 'l':
        config.KeyLength = atoi(optarg);
    }
  }
}
