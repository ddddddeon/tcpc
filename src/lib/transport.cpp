#include "transport.h"

#include <regex>

namespace TCPChat {

namespace Transport {

std::string ReadLine(tcp::socket &socket) {
  asio::streambuf buf;
  asio::read_until(socket, buf, "\n");
  asio::streambuf::const_buffers_type bufs = buf.data();
  std::string message(buffers_begin(bufs), buffers_begin(bufs) + buf.size());
  return message;
}

void Send(tcp::socket &socket, std::string message) {
  asio::error_code code;
  asio::write(socket, asio::buffer(message), code);
}

bool ParseVerifyMessage(std::string &message) {
  std::smatch signature_match;
  std::regex signature_regex("\/verify .*");
  std::regex_search(message, signature_match, signature_regex);

  if (signature_match.length() > 0) {
    message = std::regex_replace(message, std::regex("\/verify "), "");
    message = message.substr(0, message.size() - 1);
    // message = std::regex_replace(message, std::regex("\r"), "");
    // message = std::regex_replace(message, std::regex("\n"), "");
    return true;
  }
  return false;
}

std::string StripNewLines(std::string key) {
  return std::regex_replace(key, std::regex("\n"), "?");
}

std::string ExpandNewLines(std::string key) {
  return std::regex_replace(key, std::regex("\\?"), "\n");
}

}  // namespace Transport

}  // namespace TCPChat
