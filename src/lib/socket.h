#ifndef SOCKET_H
#define SOCKET_H

#include <asio.hpp>
#include <string>

using asio::ip::tcp;

namespace TCPChat {

namespace Socket {

std::string ReadLine(tcp::socket &socket);
void Send(tcp::socket &socket, std::string message);
bool ParseVerifyMessage(std::string &message);
std::string StripNewLines(std::string key);
std::string ExpandNewLines(std::string key);

}  // namespace Socket

}  // namespace TCPChat

#endif /* !SOCKET_H */