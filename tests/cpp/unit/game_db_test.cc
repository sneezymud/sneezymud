#include <format>
#include <gtest/gtest.h>

#include "database.h"
#include "database_fixture.h"
#include "extern.h"
#include "person.h"

#include "cmd_trophy.h"
#include "corporation.h"
#include "rent.h"

// Free functions in combat.cc and gaining.cc with no header declaration
extern void logPermaDeathDied(TBeing* ch, TBeing* killer);
extern void logPermaDeathLevel(TBeing* ch);

class GameDbTest : public DatabaseFixture {
  protected:
    void SetUp() override {
      DatabaseFixture::SetUp();

      tc = &makeCharacter("Testgamedb");
      room = &makeRoom(49997);
      placeInRoom(*tc, *room);

      // Create a synthetic test player to avoid mutating seeded data.
      // Pre-cleanup handles leftovers from a previous crashed run.
      {
        TDatabase cleanup(DB_SNEEZY);
        cleanup.query("DELETE FROM player WHERE name = 'Testgamedb'");
        cleanup.query("DELETE FROM account WHERE name = 'gamedb_acct'");
      }

      dbExecute(DB_SNEEZY,
        "INSERT INTO account (name, passwd, email, last_logon) "
        "VALUES ('gamedb_acct', 'x', '', 0)");
      acctId =
        convertTo<int>(dbQueryScalar(DB_SNEEZY, "SELECT LAST_INSERT_ID()"));
      ASSERT_NE(acctId, 0);

      dbExecute(DB_SNEEZY, std::format("INSERT INTO player (name, account_id) "
                                       "VALUES ('Testgamedb', {})",
                             acctId));
      playerId =
        convertTo<int>(dbQueryScalar(DB_SNEEZY, "SELECT LAST_INSERT_ID()"));
      ASSERT_NE(playerId, 0);
      tc->ch->player.player_id = playerId;
      playerName = "Testgamedb";
    }

    void TearDown() override {
      // Delete synthetic parent rows by ID - CASCADE cleans child tables,
      // making the base class dbCleanupLater queries no-ops (defensive).
      TDatabase db(DB_SNEEZY);
      db.query("DELETE FROM player WHERE id = %i", playerId);
      db.query("DELETE FROM account WHERE account_id = %i", acctId);

      DatabaseFixture::TearDown();
    }

    TestCharacter* tc = nullptr;
    TRoom* room = nullptr;
    int playerId = 0;
    int acctId = 0;
    sstring playerName;
};

TEST_F(GameDbTest, TrophyWipeDeletesByPlayerId) {
  auto mobVnum = dbQueryScalar(DB_SNEEZY, "SELECT MIN(vnum) FROM mob");
  ASSERT_FALSE(mobVnum.empty());

  dbExecute(DB_SNEEZY, std::format(
    "INSERT INTO trophy (player_id, mobvnum, count, totalcount) "
    "VALUES ({}, {}, 5, 10)", playerId, mobVnum));
  dbCleanupLater(DB_SNEEZY, std::format(
    "DELETE FROM trophy WHERE player_id = {} AND mobvnum = {}",
    playerId, mobVnum));

  TTrophy trophy(playerName);
  trophy.wipe();

  EXPECT_EQ(dbQueryScalar(DB_SNEEZY, std::format(
    "SELECT COUNT(*) FROM trophy WHERE player_id = {} AND mobvnum = {}",
    playerId, mobVnum)), "0");
}

TEST_F(GameDbTest, CorporationGetAccessReturnsCorrectLevel) {
  auto corpId = dbQueryScalar(DB_SNEEZY,
    "SELECT MIN(corp_id) FROM corporation");
  ASSERT_FALSE(corpId.empty());

  dbExecute(DB_SNEEZY,
    std::format("INSERT INTO corpaccess (corp_id, player_id, access) "
                "VALUES ({}, {}, 7)",
      corpId, playerId));
  dbCleanupLater(DB_SNEEZY, std::format(
    "DELETE FROM corpaccess WHERE corp_id = {} AND player_id = {}",
    corpId, playerId));

  TCorporation corp(convertTo<int>(corpId));
  int access = corp.getAccess(tc->ch);
  EXPECT_EQ(access, 7);
}

TEST_F(GameDbTest, CorporationHasAccessChecksPermissionBits) {
  auto corpId = dbQueryScalar(DB_SNEEZY,
    "SELECT MIN(corp_id) FROM corporation");
  ASSERT_FALSE(corpId.empty());

  dbExecute(DB_SNEEZY,
    std::format("INSERT INTO corpaccess (corp_id, player_id, access) "
                "VALUES ({}, {}, 5)",
      corpId, playerId));
  dbCleanupLater(DB_SNEEZY, std::format(
    "DELETE FROM corpaccess WHERE corp_id = {} AND player_id = {}",
    corpId, playerId));

  TCorporation corp(convertTo<int>(corpId));
  // access 5 = bits 0 and 2 set
  EXPECT_TRUE(corp.hasAccess(tc->ch, 1));   // bit 0
  EXPECT_TRUE(corp.hasAccess(tc->ch, 4));   // bit 2
  EXPECT_FALSE(corp.hasAccess(tc->ch, 2));  // bit 1 not set
}

TEST_F(GameDbTest, CorporationGetAccessReturnsZeroForNonMember) {
  auto corpId = dbQueryScalar(DB_SNEEZY,
    "SELECT MIN(corp_id) FROM corporation");
  ASSERT_FALSE(corpId.empty());

  TCorporation corp(convertTo<int>(corpId));
  int access = corp.getAccess(tc->ch);
  EXPECT_EQ(access, 0);
}

TEST_F(GameDbTest, ApplyTattooInsertsRowWithPlayerId) {
  dbCleanupLater(DB_SNEEZY,
    std::format("DELETE FROM tattoos WHERE player_id = {}", playerId));

  // WEAR_BODY (slot 3) - standard slot that passes hasPart() for humans
  bool result = tc->ch->applyTattoo(WEAR_BODY, "[Test Tattoo]", SILENT_YES);
  EXPECT_TRUE(result);

  EXPECT_EQ(
    dbQueryScalar(DB_SNEEZY,
      std::format(
        "SELECT tattoo FROM tattoos WHERE player_id = {} AND location = 3",
        playerId)),
    "[Test Tattoo]");
}

TEST_F(GameDbTest, LogPermaDeathLevelInsertsWithPlayerId) {
  dbCleanupLater(DB_SNEEZY,
    std::format("DELETE FROM permadeath WHERE player_id = {}", playerId));

  tc->ch->setMaxLevel(15);
  logPermaDeathLevel(tc->ch);

  EXPECT_EQ(dbQueryScalar(DB_SNEEZY,
              std::format("SELECT level FROM permadeath WHERE player_id = {}",
                playerId)),
    "15");
  EXPECT_EQ(dbQueryScalar(DB_SNEEZY,
              std::format("SELECT died FROM permadeath WHERE player_id = {}",
                playerId)),
    "0");
}

TEST_F(GameDbTest, LogPermaDeathDiedUpdatesExistingRow) {
  dbCleanupLater(DB_SNEEZY,
    std::format("DELETE FROM permadeath WHERE player_id = {}", playerId));

  tc->ch->setMaxLevel(20);
  logPermaDeathLevel(tc->ch);

  auto& killerTc = makeCharacter("Testkiller");
  placeInRoom(killerTc, *room);

  logPermaDeathDied(tc->ch, killerTc.ch);

  EXPECT_EQ(dbQueryScalar(DB_SNEEZY,
              std::format("SELECT died FROM permadeath WHERE player_id = {}",
                playerId)),
    "1");
  EXPECT_EQ(dbQueryScalar(DB_SNEEZY,
              std::format("SELECT killer FROM permadeath WHERE player_id = {}",
                playerId)),
    "Testkiller");
}

TEST_F(GameDbTest, ClearRentCascadesToChildTables) {
  auto objVnum = dbQueryScalar(DB_SNEEZY, "SELECT MIN(vnum) FROM obj");
  ASSERT_FALSE(objVnum.empty());

  // Allocate a rent_id above the current max to avoid collisions
  auto rentIdStr =
    dbQueryScalar(DB_SNEEZY, "SELECT COALESCE(MAX(rent_id), 0) + 1 FROM rent");
  ASSERT_FALSE(rentIdStr.empty());
  auto rentId = convertTo<int>(rentIdStr);

  dbExecute(DB_SNEEZY,
    std::format(
      "INSERT INTO rent (rent_id, owner, owner_type, vnum, val0, val1, val2, "
      "val3, extra_flags, weight, bitvector, cur_struct, max_struct, "
      "decay, material, volume, price) "
      "VALUES ({0}, {0}, 'player', {1}, 0, 0, 0, 0, 0, 1, 0, 100, 100, "
      "0, 0, 1, 0)",
      rentId, objVnum));
  dbCleanupLater(DB_SNEEZY,
    std::format("DELETE FROM rent WHERE rent_id = {}", rentId));

  dbExecute(DB_SNEEZY,
    std::format(
      "INSERT INTO rent_obj_aff (rent_id, type, level, duration, "
      "modifier, modifier2, bitvector) VALUES ({}, 1, 5, -1, 10, 0, 0)",
      rentId));
  dbExecute(DB_SNEEZY,
    std::format("INSERT INTO rent_strung (rent_id, short_desc) "
                "VALUES ({}, 'a glowing test sword')",
      rentId));

  // clearRent relies on CASCADE - no longer manually deletes children
  ItemSaveDB is("player", rentId);
  is.clearRent();

  EXPECT_EQ(
    dbQueryScalar(DB_SNEEZY,
      std::format("SELECT COUNT(*) FROM rent WHERE rent_id = {}", rentId)),
    "0");
  EXPECT_EQ(
    dbQueryScalar(DB_SNEEZY,
      std::format("SELECT COUNT(*) FROM rent_obj_aff WHERE rent_id = {}",
        rentId)),
    "0")
    << "rent_obj_aff should be cleaned up by CASCADE";
  EXPECT_EQ(dbQueryScalar(DB_SNEEZY,
              std::format("SELECT COUNT(*) FROM rent_strung WHERE rent_id = {}",
                rentId)),
    "0")
    << "rent_strung should be cleaned up by CASCADE";
}
