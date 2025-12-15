// For linking unittests against a fake database implementation.
// use the MockDb if you need to verify DB queries

#include "database.h"
#include "sstring.h"

#include <cassert>
#include <string>
#include <vector>

std::vector<std::string> db_hosts(DB_MAX);
std::vector<std::string> db_names(DB_MAX);
std::vector<std::string> db_users(DB_MAX);
std::vector<std::string> db_passwords(DB_MAX);

IDatabase::~IDatabase() {}

class TDatabasePimpl { };

TDatabase::TDatabase(dbTypeT tdb, bool log)
  : pimpl(*new TDatabasePimpl()) {
}

TDatabase::~TDatabase() {
  delete &pimpl;
}

long TDatabase::lastInsertId() {
  assert(false);
}

bool TDatabase::fetchRow() {
  return false;
}

unsigned long TDatabase::escape_string(char* to, const char* from,
  unsigned long length) {
  assert(false);
}

unsigned long IDatabase::escape_string_ugly(char* to, const char* from,
  unsigned long length) {
  assert(false);
}

const sstring TDatabase::operator[](unsigned int i) const {
  assert(false);
}

const sstring TDatabase::operator[](const sstring& s) const {
  assert(false);
}

bool TDatabase::query(const char* query, ...) {
  return false;
}

bool TDatabase::isResults() {
  assert(false);
}

long TDatabase::rowCount() {
  assert(false);
}
