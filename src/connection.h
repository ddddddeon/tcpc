#ifndef CONNECTION_H
#define CONNECTION_H

#include <asio.hpp>

using asio::ip::tcp;

class Connection
{
private:
    std::string GetAddress(tcp::socket *socket);

public:
    std::string Name;
    std::string Address;
    tcp::socket *Socket;

    Connection(tcp::socket *socket, std::string name)
    {

        Name = name;
        Address = GetAddress(socket);
        Socket = socket;
    }
};

#endif /* !CONNECTION_H */