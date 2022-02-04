#include <string>
#include <asio.hpp>

#include "client.h"

void Client::Connect()
{
    _logger.Info("Connected to " + Host + ":" + std::to_string(Port) + " as " + Name);
}