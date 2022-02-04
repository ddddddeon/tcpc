#include <iostream>
#include <string>
#include <getopt.h>
#include <signal.h>

#include "main.h"
#include "client.h"
#include "../lib/logger.h"

std::string HOST = "127.0.0.1";
int PORT = 9000;
std::string NAME = "guest";

int main(int argc, char *argv[])
{
    parse_args(argc, argv);

    Logger logger;
    Client client(HOST, PORT, NAME, logger);

    try
    {
        client.Connect();
    }
    catch (std::exception &e)
    {
        logger.Error(e.what());
    }
}

void parse_args(int argc, char *argv[])
{
    std::string localhost = "localhost";

    int opt;
    while ((opt = getopt(argc, argv, ":h:p:n:")) != -1)
    {
        switch (opt)
        {
        case 'h':
            if (localhost.compare(optarg) != 0)
            {
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
