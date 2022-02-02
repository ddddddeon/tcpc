#include "connection.h"

std::string Connection::GetAddress(tcp::socket *s)
{
    return s->remote_endpoint().address().to_string() +
           ":" +
           std::to_string(s->remote_endpoint().port());
}
