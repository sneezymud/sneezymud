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

      auto pidStr = dbQueryScalar(DB_SNEEZY, "SELECT MIN(id) FROM player");
      ASSERT_FALSE(pidStr.empty());
      playerId = convertTo<int>(pidStr);
      tc->ch->player.player_id = playerId;

      playerName = dbQueryScalar(DB_SNEEZY, std::format(
        "SELECT name FROM player WHERE id = {}", playerId));
      ASSERT_FALSE(playerName.empty());
    }

    TestCharacter* tc = nullptr;
    TRoom* room = nullptr;
    int playerId = 0;
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

  dbExecute(DB_SNEEZY, std::format(
    "INSERT IGNORE INTO corpaccess (corp_id, player_id, access) "
    "VALUES ({}, {}, 7)", corpId, playerId));
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

  dbExecute(DB_SNEEZY, std::format(
    "INSERT IGNORE INTO corpaccess (corp_id, player_id, access) "
    "VALUES ({}, {}, 5)", corpId, playerId));
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

  dbExecute(DB_SNEEZY,
    std::format(
      "INSERT INTO rent (rent_id, owner, owner_type, vnum, val0, val1, val2, "
      "val3, extra_flags, weight, bitvector, cur_struct, max_struct, "
      "decay, material, volume, price) "
      "VALUES (99950, 99950, 'player', {}, 0, 0, 0, 0, 0, 1, 0, 100, 100, "
      "0, 0, 1, 0)",
      objVnum));
  dbCleanupLater(DB_SNEEZY, "DELETE FROM rent WHERE rent_id = 99950");

  dbExecute(DB_SNEEZY,
    "INSERT INTO rent_obj_aff (rent_id, type, level, duration, modifier, "
    "modifier2, bitvector) VALUES (99950, 1, 5, -1, 10, 0, 0)");
  dbExecute(DB_SNEEZY,
    "INSERT INTO rent_strung (rent_id, short_desc) "
    "VALUES (99950, 'a glowing test sword')");

  // clearRent relies on CASCADE - no longer manually deletes children
  ItemSaveDB is("player", 99950);
  is.clearRent();

  EXPECT_EQ(
    dbQueryScalar(DB_SNEEZY, "SELECT COUNT(*) FROM rent WHERE rent_id = 99950"),
    "0");
  EXPECT_EQ(dbQueryScalar(DB_SNEEZY,
              "SELECT COUNT(*) FROM rent_obj_aff WHERE rent_id = 99950"),
    "0")
    << "rent_obj_aff should be cleaned up by CASCADE";
  EXPECT_EQ(dbQueryScalar(DB_SNEEZY,
              "SELECT COUNT(*) FROM rent_strung WHERE rent_id = 99950"),
    "0")
    << "rent_strung should be cleaned up by CASCADE";
}
