#include <string>
#include <asio.hpp>

#include "client.h"

using asio::ip::tcp;

void Client::Connect()
{
    asio::error_code ignored;
    asio::io_service service;
    asio::streambuf buf;

    tcp::socket socket(service);

    _logger.Info("Connected to " + Host + ":" + std::to_string(Port) + " as " + Name);
    socket.connect(tcp::endpoint(asio::ip::address::from_string("127.0.0.1"), 9000));

    try
    {
        size_t s = asio::read_until(socket, buf, "\n");
    }
    catch (std::exception &e)
    {
        _logger.Error(e.what());
    }

    asio::streambuf::const_buffers_type bufs = buf.data();
    std::string message(buffers_begin(bufs), buffers_begin(bufs) + buf.size());
    _logger.Info(message);
}