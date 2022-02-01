#include <iostream>
#include <string>
#include <list>
#include <sys/socket.h>
#include <netinet/in.h>
#include <asio.hpp>
#include "server.h"

using asio::ip::tcp;
using std::list;
using std::string;

void Server::Handle(tcp::socket &socket)
{
    int connected = 1;

    while (connected == 1)
    {
        asio::error_code ignored;
        asio::streambuf buf;

        size_t s = asio::read_until(socket, buf, "\n");

        std::list<tcp::socket>::iterator i;

        asio::streambuf::const_buffers_type bufs = buf.data();
        std::string str(buffers_begin(bufs), buffers_begin(bufs) + buf.size());

        // broadcast to all connected sockets
        for (i = _sockets.begin(); i != _sockets.end(); ++i)
        {
            asio::write(*i, asio::buffer(str), ignored);
            asio::write(*i, asio::buffer("> "), ignored);
        }
    }
}

void Server::Start()
{
    asio::error_code ignored;
    asio::io_service service;
    tcp::acceptor acceptor(service, tcp::endpoint(tcp::v4(), _port));

    _running = 1;
    _logger.Info("Server listening on port " + std::to_string(_port));

    while (_running == 1)
    {
        _sockets.push_front(tcp::socket(service));
        tcp::socket &socket = _sockets.front();
        acceptor.accept(socket);

        std::string remote_address = socket.remote_endpoint().address().to_string();
        std::string remote_port = std::to_string(socket.remote_endpoint().port());
        _logger.Info("Accepted connection from " + remote_address + ":" + remote_port);

        string message = "hi\n> ";
        asio::write(socket, asio::buffer(message), ignored);

        _threads.push_front(std::thread(&Server::Handle, this, std::ref(socket)));
    }
}

void Server::Stop()
{
    // TODO iterate through connections and close them
    _running = 0;
}