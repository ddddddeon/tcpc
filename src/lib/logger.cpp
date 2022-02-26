#include "logger.h"

#include <iostream>
#include <string>

namespace TCPChat {

std::string const levels[3] = {"INFO", "WARN", "ERROR"};

void Logger::Info(std::string str) {
  level = INFO;
  Logger::Log(str);
}

void Logger::Warn(std::string str) {
  level = WARN;
  Logger::Log(str);
}

void Logger::Error(std::string str) {
  level = ERROR;
  Logger::Log(str);
}

void Logger::Log(std::string str) {
  std::string message = "[" + levels[level] + "]" + " " + str;

  if (str[str.length() - 1] == '\n') {
    std::cout << message;
  } else {
    std::cout << message << std::endl;
  }
}

void Logger::Line(std::string str) { std::cout << str << std::endl; }
void Logger::Raw(std::string str) { std::cout << str << std::flush; }
void Logger::Flush() { std::cout << std::flush; }

}  // namespace TCPChat