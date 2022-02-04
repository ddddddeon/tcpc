#include <iostream>
#include <string>
#include <asio.hpp>

#include "../lib/logger.h"

using asio::ip::tcp;

#define MAX_INPUT_BUFFER_SIZE 65535

class Client
{
private:
    Logger _logger;
    std::string _user_input;

    void ReadMessages(tcp::socket &socket);

public:
    std::string Host;
    int Port;
    std::string Name;

    Client(std::string host, int port, std::string name, Logger &logger)
    {
        Host = host;
        Port = port;
        Name = name;
        _logger = logger;
        _user_input.reserve(MAX_INPUT_BUFFER_SIZE);
    }

    void Connect();
};