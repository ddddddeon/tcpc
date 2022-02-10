#include "db.h"

#include <leveldb/db.h>

#include <string>

#include "logger.h"

namespace TCPChat {

void DB::Initialize(std::string path) {
  Logger logger;

  leveldb::DB *db;
  leveldb::Options options;
  options.create_if_missing = true;

  leveldb::Status status = leveldb::DB::Open(options, path, &db);
  if (!status.ok()) {
    logger.Error("Could not initialize db at path " + path);
    exit(1);
  }

  _db = db;
  _read_options = leveldb::ReadOptions();
  _write_options = leveldb::WriteOptions();

  logger.Info("Initialized DB at " + path);
}

std::string DB::Get(std::string name) {
  std::string pubkey = "";
  leveldb::Status status = _db->Get(_read_options, name, &pubkey);
  return pubkey;
}

bool DB::Set(std::string name, std::string pubkey) {
  leveldb::Status status = _db->Put(_write_options, name, pubkey);
  if (!status.ok()) {
    return false;
  }
  return true;
}

}  // namespace TCPChat