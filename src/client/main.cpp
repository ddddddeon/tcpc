#include "main.h"

#include <getopt.h>
#include <signal.h>

#include <iostream>
#include <string>

#include "../lib/logger.h"
#include "client.h"

using namespace TCPChat;

// TODO move into Config namespace
bool GENERATE_KEYPAIR = false;
std::string KEYPAIR_PATH = "./";
std::string NAME = "chris";  // TODO add cli arg for this
std::string HOST = "127.0.0.1";
int PORT = 9000;

int main(int argc, char *argv[]) {
  Logger logger;

  parse_args(argc, argv);
  Client client(HOST, PORT, NAME, logger, GENERATE_KEYPAIR, KEYPAIR_PATH);

  if (!GENERATE_KEYPAIR) {
    logger.Info("Loading keypair from " + KEYPAIR_PATH);

    if (KEYPAIR_PATH.back() != '/') {
      KEYPAIR_PATH += '/';
    }

    logger.Info(KEYPAIR_PATH);

    bool loaded = client.LoadKeyPair(KEYPAIR_PATH);

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
          HOST = optarg;
        }
        break;
      case 'p':
        PORT = atoi(optarg);
        break;
      case 'n':
        NAME = optarg;
        break;
      case 'g':
        GENERATE_KEYPAIR = true;
        break;
      case 'k':
        KEYPAIR_PATH = optarg;
        break;
    }
  }
}
