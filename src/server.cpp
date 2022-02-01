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

std::string GetAddress(tcp::socket &s)
{
    return s.remote_endpoint().address().to_string() +
           ":" +
           std::to_string(s.remote_endpoint().port());
}

void Server::Handle(tcp::socket &socket)
{
    int connected = 1;

    while (connected == 1)
    {
        asio::error_code ignored;
        asio::streambuf buf;

        // TODO store this stuff in a connection class
        std::string address = GetAddress(socket);

        size_t s = asio::read_until(socket, buf, "\n");
        asio::streambuf::const_buffers_type bufs = buf.data();
        std::string str(buffers_begin(bufs), buffers_begin(bufs) + buf.size());

        // TODO move to Server::Broadcast()
        std::list<tcp::socket>::iterator i;
        for (i = _sockets.begin(); i != _sockets.end(); i++)
        {
            // TODO all receiving clients should prepend a newline, but not the sender
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

        _logger.Info("Accepted connection from " + GetAddress(socket));

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