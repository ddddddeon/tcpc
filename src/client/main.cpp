#include <iostream>
#include <string>
#include <getopt.h>

#include "main.h"
#include "client.h"
#include "../lib/logger.h"

std::string HOST = "localhost";
int PORT = 9000;
std::string NAME = "guest";

int main(int argc, char *argv[])
{
    Logger logger;

    parse_args(argc, argv);

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
    int opt;
    while ((opt = getopt(argc, argv, ":h:p:n:")) != -1)
    {
        switch (opt)
        {
        case 'h':
            HOST = optarg;
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
