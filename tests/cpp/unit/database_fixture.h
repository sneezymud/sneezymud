#pragma once

// Integration test fixture for tests requiring a live database.
//
// Extends GameFixture with database helpers. SetUpTestSuite sets
// db_hosts to "localhost" for local socket auth and probes both
// DB_SNEEZY and DB_IMMORTAL. Tests are skipped when either
// database is unreachable (e.g., CI without MariaDB).
//
// Important: all TDatabase instances of the same dbTypeT share a
// single global MYSQL* connection. The mv* functions use BEGIN/COMMIT
// on DB_SNEEZY. Do all test setup BEFORE calling a mv* function and
// all verification AFTER - never interleave.

#include <string>
#include <vector>

#include "database.h"
#include "game_fixture.h"

// These are defined in database.cc and normally populated by
// configuration.cc during server startup.
extern std::vector<std::string> db_hosts;

class DatabaseFixture : public GameFixture {
  protected:
    static void SetUpTestSuite() {
      GameFixture::SetUpTestSuite();
      databaseAvailable = true;

      // TDatabase lazy-connects using db_hosts[type].c_str() as the
      // host parameter to mysql_real_connect. The vectors default to
      // empty strings, which may or may not work depending on the
      // client library build. Explicitly set to "localhost" for
      // reliable local socket connections.
      db_hosts[DB_SNEEZY] = "localhost";
      db_hosts[DB_IMMORTAL] = "localhost";

      // Probe both databases. If either is unreachable, all tests
      // in this suite are skipped rather than failed.
      TDatabase probeSneezy(DB_SNEEZY);
      TDatabase probeImmortal(DB_IMMORTAL);
      if (!probeSneezy.query("SELECT 1") ||
          !probeImmortal.query("SELECT 1")) {
        databaseAvailable = false;
      }
    }

    void SetUp() override {
      if (!databaseAvailable) {
        GTEST_SKIP() << "Database not available - skipping integration test";
      }
      GameFixture::SetUp();
    }

    void TearDown() override {
      // Run registered cleanup queries in forward order. Callers
      // must register in execution order: child tables first,
      // parent tables last.
      for (const auto& [db, sql] : cleanupQueries) {
        TDatabase tdb(db);
        tdb.query(sql.c_str());
      }
      cleanupQueries.clear();

      GameFixture::TearDown();
    }

    // Execute SQL and assert success. The sql string is passed directly
    // to TDatabase::query() as a format string, so any literal '%' in
    // the SQL will be misinterpreted as a format specifier. For strings
    // needing escaping (e.g., apostrophes, percent signs), use
    // TDatabase::query() directly with %s/%i format specifiers instead.
    void dbExecute(dbTypeT db, const sstring& sql) {
      TDatabase tdb(db);
      ASSERT_TRUE(tdb.query(sql.c_str()))
        << "SQL failed: " << sql;
    }

    // Query a single scalar value. Returns empty string if no rows.
    [[nodiscard]] sstring dbQueryScalar(dbTypeT db, const sstring& sql) {
      TDatabase tdb(db);
      if (!tdb.query(sql.c_str())) {
        ADD_FAILURE() << "SQL failed: " << sql;
        return "";
      }
      if (tdb.fetchRow()) {
        return tdb[0];
      }
      return "";
    }

    // Register a cleanup query to run in TearDown. Queries execute
    // in registration order, so register child table deletes BEFORE
    // parent table deletes.
    void dbCleanupLater(dbTypeT db, const sstring& sql) {
      cleanupQueries.emplace_back(db, sql);
    }

  private:
    static inline bool databaseAvailable = true;
    std::vector<std::pair<dbTypeT, sstring>> cleanupQueries;
};
