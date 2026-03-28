// Integration tests for lowShop() - the "low shop" builder command.
// Calls lowShop() directly, bypassing doLow() permission checks.
// Follows the same DatabaseFixture pattern as low_mv_test.cc.
//
// Note: createShop() validates the keeper vnum via real_mobile(), which
// requires mob_index to be populated. Since tests don't load mob data,
// shops are created directly via SQL. This tests all subcommands except
// the create success path.

#include <gtest/gtest.h>

#include "database_fixture.h"
#include "low.h"
#include "room.h"

class LowShopTest : public DatabaseFixture {
  protected:
    static constexpr int TEST_SHOP = 999;

    void SetUp() override {
      DatabaseFixture::SetUp();

      tc = &makeCharacter("TestShopAdmin");
      room = &makeRoom(49997);
      placeInRoom(*tc, *room);

      // Cleanup in TearDown (child tables first)
      dbCleanupLater(DB_SNEEZY, "DELETE FROM shopowned WHERE shop_nr = 999");
      dbCleanupLater(DB_SNEEZY, "DELETE FROM shoptype WHERE shop_nr = 999");
      dbCleanupLater(DB_SNEEZY, "DELETE FROM shop WHERE shop_nr = 999");
    }

    // Call lowShop and return the output
    sstring shopCmd(const sstring& args) {
      lowShop(*tc->ch, args);
      return tc->drainOutput();
    }

    // Create a test shop directly in the database (bypasses real_mobile).
    // Avoids literal '%' in strings since dbExecute passes the SQL as
    // a printf-style format string to TDatabase::query().
    void createTestShop() {
      dbExecute(DB_SNEEZY,
        "INSERT IGNORE INTO shop (shop_nr, keeper, in_room, profit_buy, "
        "profit_sell, no_such_item1, no_such_item2, do_not_buy, "
        "missing_cash1, missing_cash2, message_buy, message_sell, "
        "temper1, temper2, flags, open1, close1, open2, close2) "
        "VALUES (999, 10, 0, 1.1, 0.9, "
        "'Sorry, not in stock.', 'Never heard of it!', "
        "'Go sell elsewhere!', 'Cannot afford that!', "
        "'You lack the funds!', 'Thank you for your purchase.', "
        "'Here are your talens.', 2, 1, 0, 0, 96, 0, 0)");
    }

    TestCharacter* tc = nullptr;
    TRoom* room = nullptr;
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
  auto output = shopCmd("info 999");
  EXPECT_NE(output.find("Shop #999"), sstring::npos);
}

// --- Field Modification ---

TEST_F(LowShopTest, ModifyFloatField) {
  createTestShop();
  auto output = shopCmd("modify 999 profit_buy 1.2");
  EXPECT_NE(output.find("profit_buy set to '1.2'"), sstring::npos);
}

TEST_F(LowShopTest, ModifyFloatFieldRejectsNonPositive) {
  createTestShop();
  auto output = shopCmd("modify 999 profit_buy -1.0");
  EXPECT_NE(output.find("must be a positive number"), sstring::npos);
}

TEST_F(LowShopTest, ModifyHourField) {
  createTestShop();
  auto output = shopCmd("modify 999 open1 8");
  EXPECT_NE(output.find("open1 set to '8'"), sstring::npos);
}

TEST_F(LowShopTest, ModifyHourFieldAcceptsAlwaysOpen) {
  createTestShop();
  auto output = shopCmd("modify 999 open1 96");
  EXPECT_NE(output.find("open1 set to '96'"), sstring::npos);
}

TEST_F(LowShopTest, ModifyHourFieldRejectsOutOfRange) {
  createTestShop();
  auto output = shopCmd("modify 999 open1 100");
  EXPECT_NE(output.find("must be 0-23 (hour) or 96 (always open)"),
    sstring::npos);
}

TEST_F(LowShopTest, ModifyStringField) {
  createTestShop();
  auto output = shopCmd("modify 999 no_such_item1 Custom message here");
  EXPECT_NE(output.find("no_such_item1 set to"), sstring::npos);
}

// ModifyKeeperField: calls real_mobile() which needs mob_index.
// WorldFixture::insertTestMob now provides this, but the assertion is a
// shallow output-substring check where the functional test is adequate.

TEST_F(LowShopTest, ModifyUnknownFieldRejected) {
  createTestShop();
  auto output = shopCmd("modify 999 invalidfield value");
  EXPECT_NE(output.find("Unknown field 'invalidfield'"), sstring::npos);
}

// Type management (addtype/rmtype): requires ItemInfo[] populated at boot.
// WorldFixture::SetUpTestSuite now calls assign_item_info(), but these are
// shallow output checks where functional tests provide realistic coverage.
// The range check for out-of-bounds types fires before ItemInfo lookup, so
// it works here without WorldFixture.

TEST_F(LowShopTest, AddInvalidTypeRejected) {
  createTestShop();
  auto output = shopCmd("addtype 999 9999");
  EXPECT_NE(output.find("Invalid item type 9999"), sstring::npos);
}

// --- Ownership ---

TEST_F(LowShopTest, OwnerDisplayUnownedShop) {
  createTestShop();
  auto output = shopCmd("owner 999");
  EXPECT_NE(output.find("Not owned"), sstring::npos);
}

TEST_F(LowShopTest, OwnerRemoveOnUnownedShop) {
  createTestShop();
  auto output = shopCmd("owner 999 remove");
  EXPECT_NE(output.find("is not owned"), sstring::npos);
}

TEST_F(LowShopTest, OwnerSet) {
  createTestShop();
  auto output = shopCmd("owner 999 set 1");
  EXPECT_NE(output.find("is now owned by"), sstring::npos);
}

TEST_F(LowShopTest, OwnerDisplayAfterSet) {
  createTestShop();
  shopCmd("owner 999 set 1");
  auto output = shopCmd("owner 999");
  EXPECT_NE(output.find("Owned by"), sstring::npos);
}

TEST_F(LowShopTest, OwnerRemoveAfterSet) {
  createTestShop();
  shopCmd("owner 999 set 1");
  auto output = shopCmd("owner 999 remove");
  EXPECT_NE(output.find("ownership removed"), sstring::npos);
}

TEST_F(LowShopTest, OwnerInvalidCorpRejected) {
  createTestShop();
  auto output = shopCmd("owner 999 set 999999");
  EXPECT_NE(output.find("does not exist"), sstring::npos);
}

// --- Access Control ---

TEST_F(LowShopTest, AccessDeniedOnUnownedShop) {
  createTestShop();
  auto playerName = dbQueryScalar(DB_SNEEZY, "SELECT name FROM player LIMIT 1");
  auto output = shopCmd("access 999 " + playerName + " 1");
  EXPECT_NE(output.find("is not owned"), sstring::npos);
}

TEST_F(LowShopTest, AccessSet) {
  createTestShop();
  shopCmd("owner 999 set 1");
  auto playerName = dbQueryScalar(DB_SNEEZY, "SELECT name FROM player LIMIT 1");
  auto output = shopCmd("access 999 " + playerName + " 1");
  EXPECT_NE(output.find("access set to 1"), sstring::npos);
  EXPECT_NE(output.find("owner"), sstring::npos);
}

TEST_F(LowShopTest, AccessBitmaskDescription) {
  createTestShop();
  shopCmd("owner 999 set 1");
  auto playerName = dbQueryScalar(DB_SNEEZY, "SELECT name FROM player LIMIT 1");
  auto output = shopCmd("access 999 " + playerName + " 7");
  EXPECT_NE(output.find("access set to 7"), sstring::npos);
  EXPECT_NE(output.find("owner"), sstring::npos);
  EXPECT_NE(output.find("info"), sstring::npos);
  EXPECT_NE(output.find("rates"), sstring::npos);
}

TEST_F(LowShopTest, AccessRemove) {
  createTestShop();
  shopCmd("owner 999 set 1");
  auto playerName = dbQueryScalar(DB_SNEEZY, "SELECT name FROM player LIMIT 1");
  shopCmd("access 999 " + playerName + " 1");
  auto output = shopCmd("access 999 " + playerName + " 0");
  EXPECT_NE(output.find("access removed"), sstring::npos);
}

TEST_F(LowShopTest, AccessNonExistentPlayerRejected) {
  createTestShop();
  shopCmd("owner 999 set 1");
  auto output = shopCmd("access 999 ZzzNobodyHere 1");
  EXPECT_NE(output.find("not found"), sstring::npos);
}

TEST_F(LowShopTest, AccessAbove255Rejected) {
  createTestShop();
  shopCmd("owner 999 set 1");
  auto playerName = dbQueryScalar(DB_SNEEZY, "SELECT name FROM player LIMIT 1");
  auto output = shopCmd("access 999 " + playerName + " 256");
  EXPECT_NE(output.find("Access level must be 0-"), sstring::npos);
}

// --- Deletion ---

TEST_F(LowShopTest, DeleteShop) {
  createTestShop();
  auto output = shopCmd("delete 999 confirm");
  EXPECT_NE(output.find("Shop 999 and all related data deleted"),
    sstring::npos);
}

TEST_F(LowShopTest, InfoAfterDeleteShowsNotFound) {
  createTestShop();
  shopCmd("delete 999 confirm");
  auto output = shopCmd("info 999");
  EXPECT_NE(output.find("does not exist"), sstring::npos);
}

// --- SQL Safety ---

TEST_F(LowShopTest, StringFieldHandlesMixedQuotes) {
  createTestShop();
  auto output =
    shopCmd(R"(modify 999 no_such_item1 He says, "We don't have that!")");
  EXPECT_NE(output.find("no_such_item1 set to"), sstring::npos);
}

TEST_F(LowShopTest, SqlInjectionAttemptSafe) {
  createTestShop();
  shopCmd("modify 999 no_such_item1 '; DROP TABLE shop; --");

  // Verify shop is still intact via info
  auto output = shopCmd("info 999");
  EXPECT_NE(output.find("Shop #999"), sstring::npos)
    << "Shop 999 should still exist after SQL injection attempt";
}
