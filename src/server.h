#ifndef SOCKET_H
#define SOCKET_H

#include <string>
#include <list>
#include <asio.hpp>
#include "logger.h"

using asio::ip::tcp;

class Server
{
private:
    Logger _logger;
    int _running;
    int _port;
    std::string _host;
    std::list<tcp::socket> _sockets;
    std::list<std::thread> _threads;

    void Handle(tcp::socket &socket);
    void Broadcast(std::string str);

public:
    Server(std::string host, int port, Logger &logger)
    {
        _host = host;
        _port = port;
        _logger = logger;
        _running = 0;
    };

    void Start();
    void Stop();
};

#endif /* !SOCKET_H */
