#ifndef CONNECTION_H
#define CONNECTION_H

#include <asio.hpp>

using asio::ip::tcp;

class Connection
{
public:
    std::string Name;
    std::string Address;
    tcp::socket *Socket;

    Connection(tcp::socket *socket, std::string name, std::string address)
    {
        Name = name;
        Address = address;
        Socket = socket;
    }
};

#endif /* !CONNECTION_H */