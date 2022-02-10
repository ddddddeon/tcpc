#ifndef SOCKET_H
#define SOCKET_H

#include <asio.hpp>
#include <string>

using asio::ip::tcp;

namespace TCPChat {

namespace Socket {

std::string ReadLine(tcp::socket &socket);
bool Send(tcp::socket &socket, std::string message);
std::string ParseVerifyMessage(std::string message);

}  // namespace Socket

}  // namespace TCPChat

#endif /* !SOCKET_H */