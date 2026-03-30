// Integration tests for lowShop() - the "low shop" builder command.
// Calls lowShop() directly, bypassing doLow() permission checks.
// Follows the same DatabaseFixture pattern as low_mv_test.cc.

#include <format>
#include <gtest/gtest.h>

#include "database_fixture.h"
#include "low.h"
#include "room.h"
#include "world_fixture.h"

class LowShopTest : public DatabaseFixture {
  protected:
    void SetUp() override {
      DatabaseFixture::SetUp();

      // Allocate a shop_nr beyond existing shops
      testShop = convertTo<int>(dbQueryScalar(DB_SNEEZY,
        "SELECT COALESCE(MAX(shop_nr), 0) + 1 FROM shop"));

      // Look up valid FK targets for createTestShop() from existing DB rows.
      testKeeperVnum = convertTo<int>(
        dbQueryScalar(DB_SNEEZY, "SELECT vnum FROM mob LIMIT 1"));
      testRoomVnum = convertTo<int>(
        dbQueryScalar(DB_SNEEZY, "SELECT vnum FROM room LIMIT 1"));

      tc = &makeCharacter("TestShopAdmin");
      room = &makeRoom(49997);
      placeInRoom(*tc, *room);

      // Create a test account+player for shop access tests. CI seeds
      // the schema but not player/account data.
      {
        TDatabase cleanup(DB_SNEEZY);
        cleanup.query(
          "DELETE FROM shopownedplayer WHERE player_id IN "
          "(SELECT id FROM player WHERE name = 'Testshopper')");
        cleanup.query("DELETE FROM player WHERE name = 'Testshopper'");
        cleanup.query("DELETE FROM account WHERE name = 'testshopper_acct'");
      }
      dbExecute(DB_SNEEZY,
        "INSERT INTO account (name, passwd, email, last_logon) "
        "VALUES ('testshopper_acct', 'x', '', 0)");
      testAcctId = dbQueryScalar(DB_SNEEZY, "SELECT LAST_INSERT_ID()");
      dbExecute(DB_SNEEZY, std::format("INSERT INTO player (name, account_id) "
                                       "VALUES ('Testshopper', {})",
                             testAcctId));
      testPlayerId = dbQueryScalar(DB_SNEEZY, "SELECT LAST_INSERT_ID()");
      testPlayerName = "Testshopper";

      // Cleanup in TearDown (child tables first)
      dbCleanupLater(DB_SNEEZY,
        std::format("DELETE FROM shopowned WHERE shop_nr = {}", testShop));
      dbCleanupLater(DB_SNEEZY,
        std::format("DELETE FROM shoptype WHERE shop_nr = {}", testShop));
      dbCleanupLater(DB_SNEEZY,
        std::format("DELETE FROM shop WHERE shop_nr = {}", testShop));
      dbCleanupLater(DB_SNEEZY,
        std::format("DELETE FROM shopownedplayer WHERE player_id = {}",
          testPlayerId));
      dbCleanupLater(DB_SNEEZY,
        std::format("DELETE FROM player WHERE id = {}", testPlayerId));
      dbCleanupLater(DB_SNEEZY,
        std::format("DELETE FROM account WHERE account_id = {}", testAcctId));
    }

    // Call lowShop and return the output
    sstring shopCmd(const sstring& args) {
      lowShop(*tc->ch, args);
      return tc->drainOutput();
    }

    // Create a test shop directly in the database (bypasses real_mobile).
    void createTestShop() {
      // Remove any leftovers from a previous crashed run
      TDatabase cleanup(DB_SNEEZY);
      cleanup.query("DELETE FROM shop WHERE shop_nr = %i", testShop);

      dbExecute(DB_SNEEZY,
        std::format("INSERT INTO shop (shop_nr, keeper, in_room, profit_buy, "
                    "profit_sell, no_such_item1, no_such_item2, do_not_buy, "
                    "missing_cash1, missing_cash2, message_buy, message_sell, "
                    "temper1, temper2, flags, open1, close1, open2, close2) "
                    "VALUES ({}, {}, {}, 1.1, 0.9, "
                    "'Sorry, not in stock.', 'Never heard of it!', "
                    "'Go sell elsewhere!', 'Cannot afford that!', "
                    "'You lack the funds!', 'Thank you for your purchase.', "
                    "'Here are your talens.', 2, 1, 0, 0, 96, 0, 0)",
          testShop, testKeeperVnum, testRoomVnum));
    }

    TestCharacter* tc = nullptr;
    TRoom* room = nullptr;
    int testShop = 0;
    int testKeeperVnum = 0;
    int testRoomVnum = 0;
    sstring testPlayerName;
    sstring testPlayerId;
    sstring testAcctId;
};

// --- Routing & Display ---

TEST_F(LowShopTest, NoSubcommandShowsSyntax) {
  auto output = shopCmd("");
  EXPECT_NE(output.find("Syntax:"), sstring::npos)
    << "Expected syntax text, got: " << output;
}

TEST_F(LowShopTest, ListShowsColumnHeaders) {
  auto output = shopCmd("list");
  EXPECT_NE(output.find("Shop#"), sstring::npos);
  EXPECT_NE(output.find("Keeper"), sstring::npos);
}

// ListFiltersByZone: requires zone_table populated at boot. WorldFixture
// could support synthetic zone entries, but the functional test provides
// more realistic coverage with real zone/shop data.

TEST_F(LowShopTest, ListRejectsOutOfRangeZone) {
  auto output = shopCmd("list 99999");
  EXPECT_NE(output.find("Zone must be between"), sstring::npos);
}

TEST_F(LowShopTest, InfoNonExistentShop) {
  auto output = shopCmd("info 99999");
  EXPECT_NE(output.find("does not exist"), sstring::npos);
}

// --- Shop Creation (error cases that fire before real_mobile) ---

TEST_F(LowShopTest, CreateNegativeShopRejected) {
  auto output = shopCmd("create -1 10 0");
  EXPECT_NE(output.find("must be non-negative"), sstring::npos);
}

// --- Info on created shop ---

TEST_F(LowShopTest, InfoConfirmsShopExists) {
  createTestShop();
  auto output = shopCmd(std::format("info {}", testShop));
  EXPECT_NE(output.find(std::format("Shop #{}", testShop)), sstring::npos);
}

// --- Field Modification ---

TEST_F(LowShopTest, ModifyFloatField) {
  createTestShop();
  auto output = shopCmd(std::format("modify {} profit_buy 1.2", testShop));
  EXPECT_NE(output.find("profit_buy set to '1.2'"), sstring::npos);
}

TEST_F(LowShopTest, ModifyFloatFieldRejectsNonPositive) {
  createTestShop();
  auto output = shopCmd(std::format("modify {} profit_buy -1.0", testShop));
  EXPECT_NE(output.find("must be a positive number"), sstring::npos);
}

TEST_F(LowShopTest, ModifyHourField) {
  createTestShop();
  auto output = shopCmd(std::format("modify {} open1 8", testShop));
  EXPECT_NE(output.find("open1 set to '8'"), sstring::npos);
}

TEST_F(LowShopTest, ModifyHourFieldAcceptsAlwaysOpen) {
  createTestShop();
  auto output = shopCmd(std::format("modify {} open1 96", testShop));
  EXPECT_NE(output.find("open1 set to '96'"), sstring::npos);
}

TEST_F(LowShopTest, ModifyHourFieldRejectsOutOfRange) {
  createTestShop();
  auto output = shopCmd(std::format("modify {} open1 100", testShop));
  EXPECT_NE(output.find("must be 0-23 (hour) or 96 (always open)"),
    sstring::npos);
}

TEST_F(LowShopTest, ModifyStringField) {
  createTestShop();
  auto output = shopCmd(
    std::format("modify {} no_such_item1 Custom message here", testShop));
  EXPECT_NE(output.find("no_such_item1 set to"), sstring::npos);
}

// ModifyKeeperField: calls real_mobile() which needs mob_index.
// WorldFixture::insertTestMob now provides this, but the assertion is a
// shallow output-substring check where the functional test is adequate.

TEST_F(LowShopTest, ModifyUnknownFieldRejected) {
  createTestShop();
  auto output = shopCmd(std::format("modify {} invalidfield value", testShop));
  EXPECT_NE(output.find("Unknown field 'invalidfield'"), sstring::npos);
}

// Type management (addtype/rmtype): requires ItemInfo[] populated at boot.
// WorldFixture::SetUpTestSuite now calls assign_item_info(), but these are
// shallow output checks where functional tests provide realistic coverage.
// The range check for out-of-bounds types fires before ItemInfo lookup, so
// it works here without WorldFixture.

TEST_F(LowShopTest, AddInvalidTypeRejected) {
  createTestShop();
  auto output = shopCmd(std::format("addtype {} 9999", testShop));
  EXPECT_NE(output.find("Invalid item type 9999"), sstring::npos);
}

// --- Ownership ---

TEST_F(LowShopTest, OwnerDisplayUnownedShop) {
  createTestShop();
  auto output = shopCmd(std::format("owner {}", testShop));
  EXPECT_NE(output.find("Not owned"), sstring::npos);
}

TEST_F(LowShopTest, OwnerRemoveOnUnownedShop) {
  createTestShop();
  auto output = shopCmd(std::format("owner {} remove", testShop));
  EXPECT_NE(output.find("is not owned"), sstring::npos);
}

TEST_F(LowShopTest, OwnerSet) {
  createTestShop();
  auto output = shopCmd(std::format("owner {} set 1", testShop));
  EXPECT_NE(output.find("is now owned by"), sstring::npos);
}

TEST_F(LowShopTest, OwnerDisplayAfterSet) {
  createTestShop();
  shopCmd(std::format("owner {} set 1", testShop));
  auto output = shopCmd(std::format("owner {}", testShop));
  EXPECT_NE(output.find("Owned by"), sstring::npos);
}

TEST_F(LowShopTest, OwnerRemoveAfterSet) {
  createTestShop();
  shopCmd(std::format("owner {} set 1", testShop));
  auto output = shopCmd(std::format("owner {} remove", testShop));
  EXPECT_NE(output.find("ownership removed"), sstring::npos);
}

TEST_F(LowShopTest, OwnerInvalidCorpRejected) {
  createTestShop();
  auto output = shopCmd(std::format("owner {} set 999999", testShop));
  EXPECT_NE(output.find("does not exist"), sstring::npos);
}

// --- Access Control ---

TEST_F(LowShopTest, AccessDeniedOnUnownedShop) {
  createTestShop();
  auto output =
    shopCmd(std::format("access {} {} 1", testShop, testPlayerName));
  EXPECT_NE(output.find("is not owned"), sstring::npos);
}

TEST_F(LowShopTest, AccessSet) {
  createTestShop();
  shopCmd(std::format("owner {} set 1", testShop));
  auto output =
    shopCmd(std::format("access {} {} 1", testShop, testPlayerName));
  EXPECT_NE(output.find("access set to 1"), sstring::npos);
  EXPECT_NE(output.find("owner"), sstring::npos);
}

TEST_F(LowShopTest, AccessBitmaskDescription) {
  createTestShop();
  shopCmd(std::format("owner {} set 1", testShop));
  auto output =
    shopCmd(std::format("access {} {} 7", testShop, testPlayerName));
  EXPECT_NE(output.find("access set to 7"), sstring::npos);
  EXPECT_NE(output.find("owner"), sstring::npos);
  EXPECT_NE(output.find("info"), sstring::npos);
  EXPECT_NE(output.find("rates"), sstring::npos);
}

TEST_F(LowShopTest, AccessRemove) {
  createTestShop();
  shopCmd(std::format("owner {} set 1", testShop));
  shopCmd(std::format("access {} {} 1", testShop, testPlayerName));
  auto output =
    shopCmd(std::format("access {} {} 0", testShop, testPlayerName));
  EXPECT_NE(output.find("access removed"), sstring::npos);
}

TEST_F(LowShopTest, AccessNonExistentPlayerRejected) {
  createTestShop();
  shopCmd(std::format("owner {} set 1", testShop));
  auto output = shopCmd(std::format("access {} ZzzNobodyHere 1", testShop));
  EXPECT_NE(output.find("not found"), sstring::npos);
}

TEST_F(LowShopTest, AccessAbove255Rejected) {
  createTestShop();
  shopCmd(std::format("owner {} set 1", testShop));
  auto output =
    shopCmd(std::format("access {} {} 256", testShop, testPlayerName));
  EXPECT_NE(output.find("Access level must be 0-"), sstring::npos);
}

// --- Create success path (requires mob_index via WorldFixture) ---

class LowShopCreateTest : public WorldFixture {
  protected:
    void SetUp() override {
      WorldFixture::SetUp();

      // Allocate IDs beyond existing data
      testShop = convertTo<int>(dbQueryScalar(DB_SNEEZY,
        "SELECT COALESCE(MAX(shop_nr), 0) + 1 FROM shop"));
      testMobVnum = convertTo<int>(
        dbQueryScalar(DB_SNEEZY, "SELECT COALESCE(MAX(vnum), 0) + 1 FROM mob"));

      tc = &makeCharacter("TestShopCreator");

      // Use a real room vnum from the DB so the shop.in_room FK is satisfied.
      // makeRoom creates in-memory rooms; createShop also needs a DB row.
      auto roomVnumStr = dbQueryScalar(DB_SNEEZY,
        "SELECT vnum FROM room WHERE vnum NOT IN "
        "(SELECT in_room FROM shop) LIMIT 1");
      ASSERT_FALSE(roomVnumStr.empty()) << "No available room for test shop";
      testRoomVnum = convertTo<int>(roomVnumStr);

      room = &makeRoom(testRoomVnum);
      placeInRoom(*tc, *room);

      // insertTestMob populates in-memory mob_index for real_mobile().
      // Also insert a DB row so the shop.keeper FK is satisfied.
      insertTestMob(testMobVnum, "test shopkeeper", 10);
      dbExecute(DB_SNEEZY,
        std::format(
          "INSERT INTO mob (vnum, name, short_desc, long_desc, description, "
          "actions, affects, faction, fact_perc, letter, attacks, class, "
          "level, "
          "tohit, ac, hpbonus, damage_level, damage_precision, gold, race, "
          "str, bra, con, dex, agi, intel, wis, foc, per, cha, kar, spe, "
          "pos, def_position, sex, spec_proc, skin, vision, can_be_seen, "
          "max_exist, weight, height) VALUES "
          "({}, 'test shopkeeper', 'test shopkeeper', '', '', "
          "0, 0, 0, 0, 'T', 1.0, 0, 10, 0, 0.0, 0.0, 0.0, 0, 0, 0, "
          "0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, "
          "100, 72)",
          testMobVnum));

      dbCleanupLater(DB_SNEEZY,
        std::format("DELETE FROM shop WHERE shop_nr = {}", testShop));
      dbCleanupLater(DB_SNEEZY,
        std::format("DELETE FROM mob WHERE vnum = {}", testMobVnum));
    }

    TestCharacter* tc = nullptr;
    TRoom* room = nullptr;
    int testShop = 0;
    int testMobVnum = 0;
    int testRoomVnum = 0;
};

TEST_F(LowShopCreateTest, CreateShopSucceeds) {
  lowShop(*tc->ch,
    std::format("create {} {} {}", testShop, testMobVnum, testRoomVnum));
  auto output = tc->drainOutput();

  EXPECT_NE(output.find(std::format("Shop {} created", testShop)),
    sstring::npos)
    << "Expected success message, got: " << output;

  // Verify the shop row exists in the database
  EXPECT_EQ(
    dbQueryScalar(DB_SNEEZY,
      std::format("SELECT shop_nr FROM shop WHERE shop_nr = {}", testShop)),
    std::to_string(testShop));
}

// --- Deletion ---

TEST_F(LowShopTest, DeleteShop) {
  createTestShop();
  auto output = shopCmd(std::format("delete {} confirm", testShop));
  EXPECT_NE(
    output.find(std::format("Shop {} and all related data deleted", testShop)),
    sstring::npos);
}

TEST_F(LowShopTest, InfoAfterDeleteShowsNotFound) {
  createTestShop();
  shopCmd(std::format("delete {} confirm", testShop));
  auto output = shopCmd(std::format("info {}", testShop));
  EXPECT_NE(output.find("does not exist"), sstring::npos);
}

// --- SQL Safety ---

TEST_F(LowShopTest, StringFieldHandlesMixedQuotes) {
  createTestShop();
  auto output = shopCmd(std::format(
    R"(modify {} no_such_item1 He says, "We don't have that!")", testShop));
  EXPECT_NE(output.find("no_such_item1 set to"), sstring::npos);
}

TEST_F(LowShopTest, SqlInjectionAttemptSafe) {
  createTestShop();
  shopCmd(
    std::format("modify {} no_such_item1 '; DROP TABLE shop; --", testShop));

  // Verify shop is still intact via info
  auto output = shopCmd(std::format("info {}", testShop));
  EXPECT_NE(output.find(std::format("Shop #{}", testShop)), sstring::npos)
    << "Shop should still exist after SQL injection attempt";
}
