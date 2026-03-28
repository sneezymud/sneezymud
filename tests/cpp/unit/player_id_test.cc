#include <format>
#include <gtest/gtest.h>

#include "database.h"
#include "database_fixture.h"
#include "extern.h"
#include "process.h"

class PlayerIdTest : public DatabaseFixture {};

TEST_F(PlayerIdTest, GetPlayerIdByNameReturnsCorrectId) {
  auto name = dbQueryScalar(DB_SNEEZY, "SELECT name FROM player LIMIT 1");
  ASSERT_FALSE(name.empty()) << "No players in database";

  auto expectedId = dbQueryScalar(DB_SNEEZY, std::format(
    "SELECT id FROM player WHERE name = '{}'", name));
  ASSERT_FALSE(expectedId.empty());

  int result = getPlayerIdByName(name.c_str());
  EXPECT_EQ(result, convertTo<int>(expectedId));
}

TEST_F(PlayerIdTest, GetPlayerIdByNameReturnsZeroForNonexistent) {
  EXPECT_EQ(getPlayerIdByName("Zznonexistentplayer99"), 0);
}

TEST_F(PlayerIdTest, GetPlayerIdByNameIsCaseInsensitive) {
  auto name = dbQueryScalar(DB_SNEEZY, "SELECT name FROM player LIMIT 1");
  ASSERT_FALSE(name.empty());

  auto expectedId = dbQueryScalar(DB_SNEEZY, std::format(
    "SELECT id FROM player WHERE name = '{}'", name));

  sstring upperName = name.upper();
  int result = getPlayerIdByName(upperName.c_str());
  EXPECT_EQ(result, convertTo<int>(expectedId))
    << "getPlayerIdByName should be case-insensitive";
}

TEST_F(PlayerIdTest, GetPlayerIdByNameReturnsZeroForEmptyString) {
  EXPECT_EQ(getPlayerIdByName(""), 0);
}

TEST_F(PlayerIdTest, TellHistoryCleanupCapsAt25PerRecipient) {
  dbExecute(DB_SNEEZY,
    "INSERT INTO account (account_id, name, passwd, email, last_logon) "
    "VALUES (99920, 'tellcap_acct', 'x', '', 0)");
  dbExecute(DB_SNEEZY,
    "INSERT INTO player (id, name, account_id) "
    "VALUES (99920, 'Tellcapsender', 99920)");
  dbExecute(DB_SNEEZY,
    "INSERT INTO account (account_id, name, passwd, email, last_logon) "
    "VALUES (99921, 'tellcap_acc2', 'x', '', 0)");
  dbExecute(DB_SNEEZY,
    "INSERT INTO player (id, name, account_id) "
    "VALUES (99921, 'Tellcaprecip', 99921)");

  // Cleanup: child tables first, parent tables last
  dbCleanupLater(DB_SNEEZY, "DELETE FROM tellhistory WHERE to_id = 99921");
  dbCleanupLater(DB_SNEEZY, "DELETE FROM player WHERE id IN (99920, 99921)");
  dbCleanupLater(DB_SNEEZY,
    "DELETE FROM account WHERE account_id IN (99920, 99921)");

  for (int i = 0; i < 30; i++) {
    dbExecute(DB_SNEEZY,
      std::format("INSERT INTO tellhistory (from_id, to_id, tell, telltime) "
                  "VALUES (99920, 99921, 'tell number {}', "
                  "DATE_ADD('2020-01-01', INTERVAL {} SECOND))",
        i, i));
  }

  procTellHistoryCleanup cleanup(0);
  TPulse pulse(0);
  cleanup.run(pulse);

  EXPECT_EQ(dbQueryScalar(DB_SNEEZY,
              "SELECT COUNT(*) FROM tellhistory WHERE to_id = 99921"),
    "25");

  // Oldest 5 (numbers 0-4) should be gone, newest 25 (5-29) remain
  EXPECT_EQ(dbQueryScalar(DB_SNEEZY,
              "SELECT tell FROM tellhistory WHERE to_id = 99921 "
              "ORDER BY telltime ASC LIMIT 1"),
    "tell number 5");
  EXPECT_EQ(dbQueryScalar(DB_SNEEZY,
              "SELECT tell FROM tellhistory WHERE to_id = 99921 "
              "ORDER BY telltime DESC LIMIT 1"),
    "tell number 29");
}

TEST_F(PlayerIdTest, TellHistoryCleanupIsolatesRecipients) {
  dbExecute(DB_SNEEZY,
    "INSERT INTO account (account_id, name, passwd, email, last_logon) "
    "VALUES (99922, 'telliso_ac1', 'x', '', 0)");
  dbExecute(DB_SNEEZY,
    "INSERT INTO player (id, name, account_id) "
    "VALUES (99922, 'Tellisorecip', 99922)");

  // Cleanup: child tables first, parent tables last
  dbCleanupLater(DB_SNEEZY, "DELETE FROM tellhistory WHERE to_id = 99922");
  dbCleanupLater(DB_SNEEZY, "DELETE FROM player WHERE id = 99922");
  dbCleanupLater(DB_SNEEZY, "DELETE FROM account WHERE account_id = 99922");

  auto senderId = dbQueryScalar(DB_SNEEZY, "SELECT MIN(id) FROM player");
  ASSERT_FALSE(senderId.empty());

  for (int i = 0; i < 10; i++) {
    dbExecute(DB_SNEEZY,
      std::format("INSERT INTO tellhistory (from_id, to_id, tell) "
                  "VALUES ({}, 99922, 'isolated tell {}')",
        senderId, i));
  }

  procTellHistoryCleanup cleanup(0);
  TPulse pulse(0);
  cleanup.run(pulse);

  // All 10 should survive (under 25 cap)
  EXPECT_EQ(dbQueryScalar(DB_SNEEZY,
              "SELECT COUNT(*) FROM tellhistory WHERE to_id = 99922"),
    "10");
}

TEST_F(PlayerIdTest, ShopLogCleanupDeletesOldRows) {
  auto shopNr = dbQueryScalar(DB_SNEEZY, "SELECT MIN(shop_nr) FROM shop");
  ASSERT_FALSE(shopNr.empty()) << "No shops in database";

  // Insert a shoplog row dated 400 days ago
  dbExecute(DB_SNEEZY,
    std::format("INSERT INTO shoplog (shop_nr, name, action, item, talens, "
                "shoptalens, logtime) "
                "VALUES ({}, 'Tester', 'sold', 'test old item', 100, 1000, "
                "DATE_SUB(NOW(), INTERVAL 400 DAY))",
      shopNr));
  dbCleanupLater(DB_SNEEZY,
    std::format(
      "DELETE FROM shoplog WHERE shop_nr = {} AND item = 'test old item'",
      shopNr));

  // Insert a recent row for control
  dbExecute(DB_SNEEZY,
    std::format(
      "INSERT INTO shoplog (shop_nr, name, action, item, talens, "
      "shoptalens, logtime) "
      "VALUES ({}, 'Tester', 'sold', 'test recent item', 50, 500, NOW())",
      shopNr));
  dbCleanupLater(DB_SNEEZY,
    std::format(
      "DELETE FROM shoplog WHERE shop_nr = {} AND item = 'test recent item'",
      shopNr));

  procShopLogCleanup cleanup(0);
  TPulse pulse(0);
  cleanup.run(pulse);

  EXPECT_EQ(dbQueryScalar(DB_SNEEZY,
              std::format("SELECT COUNT(*) FROM shoplog "
                          "WHERE shop_nr = {} AND item = 'test old item'",
                shopNr)),
    "0")
    << "Old shoplog row should be deleted";
  EXPECT_EQ(dbQueryScalar(DB_SNEEZY,
              std::format("SELECT COUNT(*) FROM shoplog "
                          "WHERE shop_nr = {} AND item = 'test recent item'",
                shopNr)),
    "1")
    << "Recent shoplog row should survive";
}
