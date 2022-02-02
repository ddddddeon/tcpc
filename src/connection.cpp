#include "connection.h"

std::string Connection::GetAddress(tcp::socket &socket)
{
    return socket.remote_endpoint().address().to_string() +
           ":" +
           std::to_string(socket.remote_endpoint().port());
}
