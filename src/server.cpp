#include <iostream>
#include <string>
#include <list>
#include <thread>
#include <sys/socket.h>
#include <netinet/in.h>
#include <asio.hpp>

#include "server.h"
#include "connection.h"

using asio::ip::tcp;

void Server::Start()
{
    asio::error_code ignored;
    asio::io_service service;
    tcp::acceptor acceptor(service, tcp::endpoint(_interface, _port));

    _running = 1;
    _logger.Info("Server listening on " + _interface.to_string() + ":" + std::to_string(_port));

    while (_running == 1)
    {
        _sockets.push_front(tcp::socket(service));
        tcp::socket &socket = _sockets.front();
        acceptor.accept(socket);
        _connections.push_front(Connection(&socket, "guest", GetAddress(socket)));
        Connection connection = _connections.front();

        _logger.Info("Accepted connection from " + connection.Address);
        Broadcast(connection.Address + " has entered the chat\n> ");

        _threads.push_front(std::thread(&Server::Handle, this, std::ref(socket)));
    }
}

void Server::Handle(tcp::socket &socket)
{
    int connected = 1;
    while (connected == 1)
    {
        asio::error_code ignored;
        asio::streambuf buf;

        try
        {
            size_t s = asio::read_until(socket, buf, "\n");
        }
        catch (std::exception &e)
        {
            connected = Disconnect(socket);
            return;
        }

        asio::streambuf::const_buffers_type bufs = buf.data();
        std::string str(buffers_begin(bufs), buffers_begin(bufs) + buf.size());

        Broadcast(str + "> ");
    }
}

void Server::Broadcast(std::string str)
{
    asio::error_code ignored;
    std::list<tcp::socket>::iterator i;
    for (i = _sockets.begin(); i != _sockets.end(); i++)
    {
        // TODO all receiving clients should prepend a newline, but not the sender
        asio::write(*i, asio::buffer(str), ignored);
    }
}

std::string Server::GetAddress(tcp::socket &socket)
{
    return socket.remote_endpoint().address().to_string() +
           ":" +
           std::to_string(socket.remote_endpoint().port());
}

int Server::Disconnect(tcp::socket &socket)
{
    std::string address = GetAddress(socket);
    _logger.Info("Received disconnect from " + address);

    std::list<Connection>::iterator i;
    for (i = _connections.begin(); i != _connections.end(); i++)
    {
        Connection connection = _connections.front();
        if (connection.Address == address)
        {
            std::list<tcp::socket>::iterator j;
            for (j = _sockets.begin(); j != _sockets.end(); j++)
            {
                if (&(*j) == connection.Socket)
                {
                    _sockets.erase(j);
                    break;
                }
            }
            _connections.pop_front();
            break;
        }
    }

    Broadcast(address + " has left the chat\n> ");
    return 0;
}

void Server::Stop()
{
    while (_connections.size() > 0)
    {
        Connection c = _connections.front();
        _logger.Info("Terminating connection with " + c.Name + " (" + c.Address + ")");
        _connections.pop_front();
    }

    while (_sockets.size() > 0)
    {
        _sockets.front().close();
        _sockets.pop_front();
    }

    _running = 0;
}