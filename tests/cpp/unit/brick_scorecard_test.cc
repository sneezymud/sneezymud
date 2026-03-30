// Integration test for the brickScorecard spec proc.
// Verifies the player name resolution via JOIN on the player table.
// The spec proc iterates ch->roomp->stuff for a TObj with matching
// spec and name, then queries brickquest JOIN player.

#include <gtest/gtest.h>
#include <format>

#include "database_fixture.h"
#include "obj_other_obj.h"
#include "parse.h"
#include "room.h"
#include "spec_objs.h"

extern int brickScorecard(TBeing*, cmdTypeT, const char*, TObj*, TObj*);

class BrickScorecardTest : public DatabaseFixture {
  protected:
    void SetUp() override {
      DatabaseFixture::SetUp();

      tc = &makeCharacter("TestViewer");
      room = &makeRoom(49995);
      placeInRoom(*tc, *room);

      // Place a scorecard object in the room for the spec proc to find
      auto* board = new TOtherObj();
      board->name = "board scorecard";
      board->spec = SPEC_BRICKQUEST;
      *room += *board;

      // Create a synthetic test player to avoid mutating seeded data.
      // Pre-cleanup handles leftovers from a previous crashed run.
      {
        TDatabase cleanup(DB_SNEEZY);
        cleanup.query(
          "DELETE FROM brickquest WHERE player_id IN "
          "(SELECT id FROM player WHERE name = 'Bricktester')");
        cleanup.query("DELETE FROM player WHERE name = 'Bricktester'");
        cleanup.query("DELETE FROM account WHERE name = 'brickscore_acct'");
      }

      dbExecute(DB_SNEEZY,
        "INSERT INTO account (name, passwd, email, last_logon) "
        "VALUES ('brickscore_acct', 'x', '', 0)");
      auto acctId = dbQueryScalar(DB_SNEEZY, "SELECT LAST_INSERT_ID()");
      ASSERT_FALSE(acctId.empty());

      dbExecute(DB_SNEEZY, std::format("INSERT INTO player (name, account_id) "
                                       "VALUES ('Bricktester', {})",
                             acctId));
      playerId = dbQueryScalar(DB_SNEEZY, "SELECT LAST_INSERT_ID()");
      ASSERT_FALSE(playerId.empty());
      playerName = "Bricktester";

      // Cleanup: child tables first, parent tables last (by ID, not name)
      dbCleanupLater(DB_SNEEZY,
        std::format("DELETE FROM brickquest WHERE player_id = {}", playerId));
      dbCleanupLater(DB_SNEEZY,
        std::format("DELETE FROM player WHERE id = {}", playerId));
      dbCleanupLater(DB_SNEEZY,
        std::format("DELETE FROM account WHERE account_id = {}", acctId));
    }

    TestCharacter* tc = nullptr;
    TRoom* room = nullptr;
    sstring playerName;
    sstring playerId;
};

TEST_F(BrickScorecardTest, DisplaysPlayerNameFromJoin) {
  dbExecute(DB_SNEEZY, std::format(
    "INSERT INTO brickquest (player_id, numbricks) "
    "VALUES ({}, 42) "
    "ON DUPLICATE KEY UPDATE numbricks = 42",
    playerId));

  brickScorecard(tc->ch, CMD_LOOK, "board", nullptr, nullptr);
  auto output = tc->drainOutput();

  EXPECT_NE(output.find(playerName), sstring::npos)
    << "Expected player name in output, got: " << output;
  EXPECT_NE(output.find("42"), sstring::npos)
    << "Expected brick count 42 in output, got: " << output;
}
