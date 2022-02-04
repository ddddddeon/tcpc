#include <string>
#include <thread>
#include <asio.hpp>
#include <stdio.h>

#include "client.h"

using asio::ip::tcp;

void Client::Connect()
{
    asio::error_code ignored;
    asio::io_service service;
    tcp::socket socket(service);

    _logger.Info("Connected to " + Host + ":" + std::to_string(Port) + " as " + Name);
    socket.connect(tcp::endpoint(asio::ip::address::from_string("127.0.0.1"), 9000));

    std::thread t(&Client::ReadMessages, this, std::ref(socket));

    while (true)
    {
        system("stty raw");
        _user_input += getchar();
    }

    system("stty cooked");
}

void Client::ReadMessages(tcp::socket &socket)
{
    int connected = 1;
    while (connected != 0)
    {
        asio::streambuf buf;

        try
        {
            size_t s = asio::read_until(socket, buf, "\n");
        }
        catch (std::exception &e)
        {
            _logger.Error(e.what());
            connected = 0;
        }

        asio::streambuf::const_buffers_type bufs = buf.data();
        std::string message(buffers_begin(bufs), buffers_begin(bufs) + buf.size());
        message = message.substr(0, message.size() - 2);

        if (_user_input.length() != 0)
        {
            std::cout << '\r' << std::flush;
        }

        std::string padding;
        int overflow = _user_input.length() - message.length();
        if (overflow > 0)
        {
            padding = std::string(overflow, ' ');
        }
        std::string padded_message = message + padding + "\r\n";

        std::cout << padded_message << std::flush;
        std::cout << _user_input << std::flush;
    }
}
