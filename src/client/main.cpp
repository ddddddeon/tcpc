#include "main.h"

#include <getopt.h>
#include <signal.h>

#include <iostream>
#include <string>

#include "../lib/logger.h"
#include "client.h"

using namespace TCPChat;

bool GENERATE_KEY_PAIR = false;  // TODO parse out an arg for this
std::string KEYPAIR_PATH = "./";
std::string NAME = "chris";
std::string HOST = "127.0.0.1";
int PORT = 9000;

int main(int argc, char *argv[]) {
  Logger logger;

  parse_args(argc, argv);
  Client client(HOST, PORT, NAME, logger, GENERATE_KEY_PAIR);

  if (!GENERATE_KEY_PAIR) {
    logger.Info("Loading keypair from " + KEYPAIR_PATH);
    // TODO make sure KEYPAIR_PATH ends with '/'
    bool loaded = client.LoadKeyPair(KEYPAIR_PATH);
    if (!loaded) {
      logger.Error("Could not load keypair from disk - exiting");
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
  while ((opt = getopt(argc, argv, ":h:p:n:")) != -1) {
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
    }
  }
}
