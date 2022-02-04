#include <iostream>
#include <string>

#include "../lib/logger.h"

class Client
{
private:
    Logger _logger;

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
    }

    void Connect();
};