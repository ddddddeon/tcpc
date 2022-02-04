#include <iostream>
#include <string>

class Client
{
public:
    std::string Host;
    int Port;
    std::string Name;

    Client(std::string host, int port, std::string name)
    {
        Host = host;
        Port = port;
        Name = name;
    }

    void Connect();
};