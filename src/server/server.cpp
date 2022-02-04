#include <iostream>
#include <string>
#include <list>
#include <thread>
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
        // TODO wrap socket in SSL
        // https://github.com/openssl/openssl/blob/691064c47fd6a7d11189df00a0d1b94d8051cbe0/demos/ssl/serv.cpp
        _sockets.push_front(tcp::socket(service));
        tcp::socket &socket = _sockets.front();
        acceptor.accept(socket);
        _connections.push_front(Connection(&socket, "guest", GetAddress(socket)));
        Connection &connection = _connections.front();

        _logger.Info("Accepted connection from " + connection.Address);
        Broadcast(connection.Name + " has entered the chat (" + connection.Address + ")\r\n");

        _threads.push_front(std::thread(&Server::Handle, this, std::ref(socket), std::ref(connection)));
    }
}

void Server::Handle(tcp::socket &socket, Connection &connection)
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
        std::string message(buffers_begin(bufs), buffers_begin(bufs) + buf.size());

        if (message.front() == '/')
        {
            // TODO authenticate name/pubkey against a leveldb database
            connection.Name = message.substr(1, message.size() - 2).substr(0, 32);
        }

        Broadcast("[" + connection.Name + "] " + message.substr(0, message.length() - 1) + "\r\n");
    }
}

void Server::Broadcast(std::string str)
{
    asio::error_code ignored;
    auto i = _sockets.begin();
    while (i != _sockets.end())
    {
        // TODO all receiving clients should prepend a newline, but not the sender
        asio::write(*i, asio::buffer(str), ignored);
        i++;
    }
}

int Server::Disconnect(tcp::socket &socket)
{
    std::string address = GetAddress(socket);
    std::string user_name;

    auto connection = _connections.begin();
    while (connection != _connections.end())
    {
        if (connection->Address == address)
        {
            auto socket = _sockets.begin();
            while (socket != _sockets.end())
            {
                if (&(*socket) == connection->Socket)
                {
                    _sockets.erase(socket);
                    break;
                }
                socket++;
            }
            user_name = connection->Name;
            _connections.erase(connection);
            break;
        }
        connection++;
    }

    _logger.Info("Received disconnect from " + address);
    Broadcast(user_name + " has left the chat (" + address + ")\r\n");
    return 0;
}

std::string Server::GetAddress(tcp::socket &socket)
{
    return socket.remote_endpoint().address().to_string() +
           ":" +
           std::to_string(socket.remote_endpoint().port());
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