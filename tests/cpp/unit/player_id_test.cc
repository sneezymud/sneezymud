#include <chrono>
#include <format>
#include <gtest/gtest.h>

#include "database.h"
#include "database_fixture.h"
#include "extern.h"
#include "process.h"

class PlayerIdTest : public DatabaseFixture {};

TEST_F(PlayerIdTest, GetPlayerIdByNameReturnsCorrectId) {
  // Create a test player - CI seeds schema but not player/account data.
  {
    TDatabase cleanup(DB_SNEEZY);
    cleanup.query("DELETE FROM player WHERE name = 'Pidtestplayer'");
    cleanup.query("DELETE FROM account WHERE name = 'pidtest_acct'");
  }
  dbExecute(DB_SNEEZY,
    "INSERT INTO account (name, passwd, email, last_logon) "
    "VALUES ('pidtest_acct', 'x', '', 0)");
  auto acctId = dbQueryScalar(DB_SNEEZY, "SELECT LAST_INSERT_ID()");
  dbExecute(DB_SNEEZY,
    std::format(
      "INSERT INTO player (name, account_id) VALUES ('Pidtestplayer', {})",
      acctId));
  auto expectedId = dbQueryScalar(DB_SNEEZY, "SELECT LAST_INSERT_ID()");
  ASSERT_FALSE(expectedId.empty());
  dbCleanupLater(DB_SNEEZY,
    std::format("DELETE FROM player WHERE id = {}", expectedId));
  dbCleanupLater(DB_SNEEZY,
    std::format("DELETE FROM account WHERE account_id = {}", acctId));

  int result = getPlayerIdByName("Pidtestplayer");
  EXPECT_EQ(result, convertTo<int>(expectedId));
}

TEST_F(PlayerIdTest, GetPlayerIdByNameReturnsZeroForNonexistent) {
  EXPECT_EQ(getPlayerIdByName("Zznonexistentplayer99"), 0);
}

TEST_F(PlayerIdTest, GetPlayerIdByNameIsCaseInsensitive) {
  // Create a test player - CI seeds schema but not player/account data.
  {
    TDatabase cleanup(DB_SNEEZY);
    cleanup.query("DELETE FROM player WHERE name = 'Pidcasetest'");
    cleanup.query("DELETE FROM account WHERE name = 'pidcase_acct'");
  }
  dbExecute(DB_SNEEZY,
    "INSERT INTO account (name, passwd, email, last_logon) "
    "VALUES ('pidcase_acct', 'x', '', 0)");
  auto acctId = dbQueryScalar(DB_SNEEZY, "SELECT LAST_INSERT_ID()");
  dbExecute(DB_SNEEZY,
    std::format(
      "INSERT INTO player (name, account_id) VALUES ('Pidcasetest', {})",
      acctId));
  auto expectedId = dbQueryScalar(DB_SNEEZY, "SELECT LAST_INSERT_ID()");
  dbCleanupLater(DB_SNEEZY,
    std::format("DELETE FROM player WHERE id = {}", expectedId));
  dbCleanupLater(DB_SNEEZY,
    std::format("DELETE FROM account WHERE account_id = {}", acctId));

  int result = getPlayerIdByName("PIDCASETEST");
  EXPECT_EQ(result, convertTo<int>(expectedId))
    << "getPlayerIdByName should be case-insensitive";
}

TEST_F(PlayerIdTest, GetPlayerIdByNameReturnsZeroForEmptyString) {
  EXPECT_EQ(getPlayerIdByName(""), 0);
}

TEST_F(PlayerIdTest, TellHistoryCleanupCapsAt25PerRecipient) {
  // Pre-cleanup handles leftovers from a previous crashed run
  {
    TDatabase cleanup(DB_SNEEZY);
    cleanup.query(
      "DELETE FROM tellhistory WHERE from_id IN "
      "(SELECT id FROM player WHERE name IN "
      "('Tellcapsender', 'Tellcaprecip'))");
    cleanup.query(
      "DELETE FROM tellhistory WHERE to_id IN "
      "(SELECT id FROM player WHERE name IN "
      "('Tellcapsender', 'Tellcaprecip'))");
    cleanup.query(
      "DELETE FROM player WHERE name IN "
      "('Tellcapsender', 'Tellcaprecip')");
    cleanup.query(
      "DELETE FROM account WHERE name IN "
      "('tellcap_acct', 'tellcap_acc2')");
  }

  // Create test accounts and players with auto-generated IDs
  dbExecute(DB_SNEEZY,
    "INSERT INTO account (name, passwd, email, last_logon) "
    "VALUES ('tellcap_acct', 'x', '', 0)");
  auto acctId1 = dbQueryScalar(DB_SNEEZY, "SELECT LAST_INSERT_ID()");
  dbExecute(DB_SNEEZY, std::format("INSERT INTO player (name, account_id) "
                                   "VALUES ('Tellcapsender', {})",
                         acctId1));
  auto senderId = dbQueryScalar(DB_SNEEZY, "SELECT LAST_INSERT_ID()");

  dbExecute(DB_SNEEZY,
    "INSERT INTO account (name, passwd, email, last_logon) "
    "VALUES ('tellcap_acc2', 'x', '', 0)");
  auto acctId2 = dbQueryScalar(DB_SNEEZY, "SELECT LAST_INSERT_ID()");
  dbExecute(DB_SNEEZY, std::format("INSERT INTO player (name, account_id) "
                                   "VALUES ('Tellcaprecip', {})",
                         acctId2));
  auto recipId = dbQueryScalar(DB_SNEEZY, "SELECT LAST_INSERT_ID()");

  // Cleanup: child tables first, parent tables last
  dbCleanupLater(DB_SNEEZY,
    std::format("DELETE FROM tellhistory WHERE to_id = {}", recipId));
  dbCleanupLater(DB_SNEEZY,
    std::format("DELETE FROM player WHERE id IN ({}, {})", senderId, recipId));
  dbCleanupLater(DB_SNEEZY,
    std::format("DELETE FROM account WHERE account_id IN ({}, {})", acctId1,
      acctId2));

  for (int i = 0; i < 30; i++) {
    dbExecute(DB_SNEEZY,
      std::format("INSERT INTO tellhistory (from_id, to_id, tell, telltime) "
                  "VALUES ({}, {}, 'tell number {}', "
                  "DATE_ADD('2020-01-01', INTERVAL {} SECOND))",
        senderId, recipId, i, i));
  }

  procTellHistoryCleanup cleanup(0);
  TPulse pulse(0);
  cleanup.run(pulse);

  EXPECT_EQ(dbQueryScalar(DB_SNEEZY,
              std::format("SELECT COUNT(*) FROM tellhistory WHERE to_id = {}",
                recipId)),
    "25");

  // Oldest 5 (numbers 0-4) should be gone, newest 25 (5-29) remain
  EXPECT_EQ(dbQueryScalar(DB_SNEEZY,
              std::format("SELECT tell FROM tellhistory WHERE to_id = {} "
                          "ORDER BY telltime ASC LIMIT 1",
                recipId)),
    "tell number 5");
  EXPECT_EQ(dbQueryScalar(DB_SNEEZY,
              std::format("SELECT tell FROM tellhistory WHERE to_id = {} "
                          "ORDER BY telltime DESC LIMIT 1",
                recipId)),
    "tell number 29");
}

TEST_F(PlayerIdTest, TellHistoryCleanupIsolatesRecipients) {
  // Pre-cleanup handles leftovers from a previous crashed run
  {
    TDatabase cleanup(DB_SNEEZY);
    cleanup.query(
      "DELETE FROM tellhistory WHERE to_id IN "
      "(SELECT id FROM player WHERE name IN "
      "('Tellisorecip', 'Tellisoover'))");
    cleanup.query(
      "DELETE FROM player WHERE name IN ('Tellisorecip', 'Tellisoover')");
    cleanup.query(
      "DELETE FROM account WHERE name IN ('telliso_ac1', 'telliso_ac2')");
  }

  // Recipient A: will have 10 rows (under 25 cap, should survive intact)
  dbExecute(DB_SNEEZY,
    "INSERT INTO account (name, passwd, email, last_logon) "
    "VALUES ('telliso_ac1', 'x', '', 0)");
  auto acctIdA = dbQueryScalar(DB_SNEEZY, "SELECT LAST_INSERT_ID()");
  dbExecute(DB_SNEEZY, std::format("INSERT INTO player (name, account_id) "
                                   "VALUES ('Tellisorecip', {})",
                         acctIdA));
  auto recipIdA = dbQueryScalar(DB_SNEEZY, "SELECT LAST_INSERT_ID()");

  // Recipient B: will have 30 rows (over 25 cap, should be trimmed to 25)
  dbExecute(DB_SNEEZY,
    "INSERT INTO account (name, passwd, email, last_logon) "
    "VALUES ('telliso_ac2', 'x', '', 0)");
  auto acctIdB = dbQueryScalar(DB_SNEEZY, "SELECT LAST_INSERT_ID()");
  dbExecute(DB_SNEEZY, std::format("INSERT INTO player (name, account_id) "
                                   "VALUES ('Tellisoover', {})",
                         acctIdB));
  auto recipIdB = dbQueryScalar(DB_SNEEZY, "SELECT LAST_INSERT_ID()");

  // Cleanup: child tables first, parent tables last
  dbCleanupLater(DB_SNEEZY,
    std::format("DELETE FROM tellhistory WHERE to_id = {}", recipIdA));
  dbCleanupLater(DB_SNEEZY,
    std::format("DELETE FROM player WHERE id = {}", recipIdA));
  dbCleanupLater(DB_SNEEZY,
    std::format("DELETE FROM account WHERE account_id = {}", acctIdA));
  dbCleanupLater(DB_SNEEZY,
    std::format("DELETE FROM tellhistory WHERE to_id = {}", recipIdB));
  dbCleanupLater(DB_SNEEZY,
    std::format("DELETE FROM player WHERE id = {}", recipIdB));
  dbCleanupLater(DB_SNEEZY,
    std::format("DELETE FROM account WHERE account_id = {}", acctIdB));

  // Use recipient A as the sender - the cleanup partitions by to_id,
  // so having the same player as both sender and recipient is fine.
  auto& senderId = recipIdA;

  for (int i = 0; i < 10; i++) {
    dbExecute(DB_SNEEZY,
      std::format("INSERT INTO tellhistory (from_id, to_id, tell) "
                  "VALUES ({}, {}, 'isolated tell {}')",
        senderId, recipIdA, i));
  }

  for (int i = 0; i < 30; i++) {
    dbExecute(DB_SNEEZY,
      std::format("INSERT INTO tellhistory (from_id, to_id, tell) "
                  "VALUES ({}, {}, 'overflow tell {}')",
        senderId, recipIdB, i));
  }

  procTellHistoryCleanup cleanup(0);
  TPulse pulse(0);
  cleanup.run(pulse);

  // Recipient A: all 10 should survive (under 25 cap)
  EXPECT_EQ(dbQueryScalar(DB_SNEEZY,
              std::format("SELECT COUNT(*) FROM tellhistory WHERE to_id = {}",
                recipIdA)),
    "10");

  // Recipient B: should be trimmed to 25
  EXPECT_EQ(dbQueryScalar(DB_SNEEZY,
              std::format("SELECT COUNT(*) FROM tellhistory WHERE to_id = {}",
                recipIdB)),
    "25");
}

TEST_F(PlayerIdTest, ShopLogCleanupDeletesOldRows) {
  auto shopNr = dbQueryScalar(DB_SNEEZY, "SELECT MIN(shop_nr) FROM shop");
  ASSERT_FALSE(shopNr.empty()) << "No shops in database";

  // Use unique item names to avoid colliding with real shop data
  auto tag =
    std::to_string(std::chrono::steady_clock::now().time_since_epoch().count());
  auto oldItem = "test_old_" + tag;
  auto recentItem = "test_recent_" + tag;

  // Insert a shoplog row dated 400 days ago
  dbExecute(DB_SNEEZY,
    std::format("INSERT INTO shoplog (shop_nr, name, action, item, talens, "
                "shoptalens, logtime) "
                "VALUES ({}, 'Tester', 'sold', '{}', 100, 1000, "
                "DATE_SUB(NOW(), INTERVAL 400 DAY))",
      shopNr, oldItem));
  dbCleanupLater(DB_SNEEZY,
    std::format("DELETE FROM shoplog WHERE shop_nr = {} AND item = '{}'",
      shopNr, oldItem));

  // Insert a recent row for control
  dbExecute(DB_SNEEZY,
    std::format("INSERT INTO shoplog (shop_nr, name, action, item, talens, "
                "shoptalens, logtime) "
                "VALUES ({}, 'Tester', 'sold', '{}', 50, 500, NOW())",
      shopNr, recentItem));
  dbCleanupLater(DB_SNEEZY,
    std::format("DELETE FROM shoplog WHERE shop_nr = {} AND item = '{}'",
      shopNr, recentItem));

  procShopLogCleanup cleanup(0);
  TPulse pulse(0);
  cleanup.run(pulse);

  EXPECT_EQ(
    dbQueryScalar(DB_SNEEZY, std::format("SELECT COUNT(*) FROM shoplog "
                                         "WHERE shop_nr = {} AND item = '{}'",
                               shopNr, oldItem)),
    "0")
    << "Old shoplog row should be deleted";
  EXPECT_EQ(
    dbQueryScalar(DB_SNEEZY, std::format("SELECT COUNT(*) FROM shoplog "
                                         "WHERE shop_nr = {} AND item = '{}'",
                               shopNr, recentItem)),
    "1")
    << "Recent shoplog row should survive";
}
