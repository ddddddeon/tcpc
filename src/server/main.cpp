#include "main.h"

#include <signal.h>

#include <asio.hpp>
#include <iostream>

#include "../lib/db.h"
#include "../lib/logger.h"
#include "server.h"

using namespace TCPChat;

std::string DB_PATH = "./db";
asio::ip::address_v4 INTERFACE = asio::ip::address_v4::loopback();
int PORT = 9000;

int main(int argc, char *argv[]) {
  parse_args(argc, argv);

  Logger logger;

  try {
    DB db(DB_PATH);
    Server server(INTERFACE, PORT, logger, db);
    server.Start();
  } catch (std::exception &e) {
    logger.Error(e.what());
  }

  return 0;
}

void parse_args(int argc, char *argv[]) {
  int opt;
  while ((opt = getopt(argc, argv, ":lai:p:")) != -1) {
    switch (opt) {
      case 'l':
        INTERFACE = asio::ip::address_v4::loopback();
        break;
      case 'a':
        INTERFACE = asio::ip::address_v4::any();
        break;
      case 'i':
        INTERFACE = asio::ip::make_address_v4(optarg);
        break;
      case 'p':
        PORT = atoi(optarg);
        break;
    }
  }
}