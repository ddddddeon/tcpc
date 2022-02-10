#include "socket.h"

#include <regex>

namespace TCPChat {

namespace Socket {

std::string ReadLine(tcp::socket &socket) {
  asio::streambuf buf;
  try {
    asio::read_until(socket, buf, "\n");
    asio::streambuf::const_buffers_type bufs = buf.data();
    std::string message(buffers_begin(bufs), buffers_begin(bufs) + buf.size());
    return message;
  } catch (std::exception &e) {
    return "";
  }
}

bool Send(tcp::socket &socket, std::string message) {
  try {
    asio::error_code code;
    asio::write(socket, asio::buffer(message), code);
    return true;
  } catch (std::exception &e) {
    return false;
  }
}

std::string ParseVerifyMessage(std::string message) {
  std::smatch nonce_match;
  std::regex signature_regex("\/verify .*");
  std::regex_search(message, nonce_match, signature_regex);

  if (nonce_match.length() > 0) {
    message = std::regex_replace(message, std::regex("\/verify "), "");
    message = std::regex_replace(message, std::regex("\r"), "");
    message = std::regex_replace(message, std::regex("\n"), "");
    return message;
  }
  return "";
}

}  // namespace Socket

}  // namespace TCPChat
