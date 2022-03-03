#ifndef DB_H
#define DB_H

#include <leveldb/db.h>

#include <string>

namespace TCPC {

class DB {
 private:
  void Initialize(std::string path);
  leveldb::DB *_db;
  leveldb::WriteOptions _write_options;
  leveldb::ReadOptions _read_options;

 public:
  DB(std::string path) { Initialize(path); }
  ~DB() { _db->~DB(); }
  std::string Get(std::string name);
  bool Set(std::string name, std::string pubkey);
};

}  // namespace TCPC

#endif /* !DB_H */