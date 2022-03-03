#ifndef FILESYSTEM_H
#define FILESYSTEM_H

#include <string>

namespace TCPC {

namespace Filesystem {

bool FileExists(std::string path);
std::string FileToString(std::string path);
bool MkDir(std::string path);

}  // namespace Filesystem

}  // namespace TCPC

#endif /* !FILESYSTEM_H */