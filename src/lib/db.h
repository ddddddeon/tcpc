#ifndef DB_H
#define DB_H

#include <leveldb/db.h>

#include <string>

namespace TCPChat {

class DB {
 private:
  void Initialize(std::string path);
  leveldb::DB *_db;
  leveldb::WriteOptions _write_options;
  leveldb::ReadOptions _read_options;

 public:
  DB(std::string path) { Initialize(path); }
  std::string Get(std::string name);
  bool Set(std::string name, std::string pubkey);
};

}  // namespace TCPChat

#endif /* !DB_H */