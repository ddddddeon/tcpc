#include "socket.h"

namespace TCPChat {

namespace Socket {

std::string ReadLine(tcp::socket &socket, asio::streambuf &buf) {
  asio::read_until(socket, buf, "\n");
  asio::streambuf::const_buffers_type bufs = buf.data();
  std::string message(buffers_begin(bufs), buffers_begin(bufs) + buf.size());
  return message;
}

void Send(tcp::socket &socket, std::string message) {
  asio::error_code code;
  asio::write(socket, asio::buffer(message), code);
}

}  // namespace Socket

}  // namespace TCPChat
