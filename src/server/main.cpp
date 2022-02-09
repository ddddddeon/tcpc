#include "main.h"

#include <signal.h>

#include <asio.hpp>
#include <iostream>

#include "../lib/db.h"
#include "../lib/logger.h"
#include "server.h"

using namespace TCPChat;

namespace ServerConfig {

std::string DBPath = "./db";
asio::ip::address_v4 Interface = asio::ip::address_v4::loopback();
int Port = 9000;

}  // namespace ServerConfig

int main(int argc, char *argv[]) {
  parse_args(argc, argv);

  Logger logger;

  try {
    DB db(ServerConfig::DBPath);
    Server server(ServerConfig::Interface, ServerConfig::Port, logger, db);
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
        ServerConfig::Interface = asio::ip::address_v4::loopback();
        break;
      case 'a':
        ServerConfig::Interface = asio::ip::address_v4::any();
        break;
      case 'i':
        ServerConfig::Interface = asio::ip::make_address_v4(optarg);
        break;
      case 'p':
        ServerConfig::Port = atoi(optarg);
        break;
    }
  }
}