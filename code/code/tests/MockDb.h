#include "database.h"
#include <iostream>

class MockDb : public IDatabase {
  public:
    virtual bool query(const char* q, ...) {
      std::cout << q << std::endl;
      return false;
    }

    virtual bool fetchRow() { return false; }

    virtual const sstring operator[](const sstring&) const { return ""; }

    virtual const sstring operator[](unsigned int) const { return ""; }

    virtual bool isResults() { return false; }

    virtual long rowCount() { return 0; }

    virtual long lastInsertId() { return 0; }

    virtual unsigned long escape_string(char* to, const char* from,
      unsigned long length) {
      to[0] = '\0';
      return 0;
    }
};
