#ifndef FILESYSTEM_H
#define FILESYSTEM_H

#include <string>

namespace TCPChat {

namespace Filesystem {

bool FileExists(std::string path);
std::string FileToString(std::string path);

}  // namespace Filesystem

}  // namespace TCPChat

#endif /* !FILESYSTEM_H */