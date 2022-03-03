#include "filesystem.h"

#include <sys/stat.h>

#include <fstream>
#include <sstream>

namespace TCPC {

namespace Filesystem {

bool FileExists(std::string path) {
  struct stat buffer;
  if (stat(path.c_str(), &buffer) == 0) {
    return true;
  }
  return false;
}

std::string FileToString(std::string path) {
  try {
    std::ifstream s(path);
    std::stringstream buf;

    buf << s.rdbuf();
    return buf.str();
  } catch (std::exception &e) {
    return "";
  }
}

}  // namespace Filesystem

}  // namespace TCPC
