#ifndef LOGGER_H
#define LOGGER_H

#include <string>

namespace TCPChat {

const int INFO = 0;
const int WARN = 1;
const int ERROR = 2;

class Logger {
 public:
  int level;

  Logger() { level = 0; }

  void Info(std::string str);
  void Warn(std::string str);
  void Error(std::string str);
  void Log(std::string str);
  void Line(std::string std);
  void Raw(std::string str);
  void Flush();
};

}  // namespace TCPChat

#endif /* !LOGGER_H */