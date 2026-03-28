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

      // Get a real player for brickquest data
      playerName = dbQueryScalar(DB_SNEEZY, "SELECT name FROM player LIMIT 1");
      ASSERT_FALSE(playerName.empty());
      playerId = dbQueryScalar(DB_SNEEZY,
        std::format("SELECT id FROM player WHERE name = '{}'", playerName));
      ASSERT_FALSE(playerId.empty());

      dbCleanupLater(DB_SNEEZY,
        std::format("DELETE FROM brickquest WHERE player_id = {}", playerId));
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
