#include <cstdlib>
#include <format>

#include <gtest/gtest.h>

#include "database_fixture.h"
#include "extern.h"
#include "person.h"

namespace {
  // 5 random lowercase letters, matching the functional test pattern
  sstring randomSuffix() {
    std::string s(5, ' ');
    for (auto& c : s)
      c = 'a' + (std::rand() % 26);
    return s;
  }
}  // namespace

class TellAltDetectionTest : public DatabaseFixture {
  protected:
    void SetUp() override {
      DatabaseFixture::SetUp();

      auto suffix = randomSuffix();
      onlineName = "Online" + suffix;
      offlineName = "Offline" + suffix;
      acctName = "tacct" + suffix;

      room = &makeRoom(49950);

      teller = &makeCharacter("Teller");
      placeInRoom(*teller, *room);
      teller->ch->spec = 0;

      // "onlineName" exists in character_list - findTellTarget can find it.
      onlineAlt = &makeCharacter(onlineName.c_str());
      placeInRoom(*onlineAlt, *room);
      onlineAlt->ch->spec = 0;

      // Create account + 2 player rows sharing the same account_id.
      // "onlineName" is in character_list (above). "offlineName" is not.
      dbExecute(DB_SNEEZY,
        std::format("INSERT INTO account (name, passwd) VALUES ('{}', 'x')",
          acctName));
      acctId = dbQueryScalar(DB_SNEEZY, "SELECT LAST_INSERT_ID()");
      ASSERT_FALSE(acctId.empty()) << "Failed to create test account";

      dbExecute(DB_SNEEZY,
        std::format("INSERT INTO player (name, account_id) VALUES ('{}', {})",
          onlineName, acctId));
      onlinePlayerId = dbQueryScalar(DB_SNEEZY, "SELECT LAST_INSERT_ID()");
      ASSERT_FALSE(onlinePlayerId.empty()) << "Failed to create online player";

      dbExecute(DB_SNEEZY,
        std::format("INSERT INTO player (name, account_id) VALUES ('{}', {})",
          offlineName, acctId));
      offlinePlayerId = dbQueryScalar(DB_SNEEZY, "SELECT LAST_INSERT_ID()");
      ASSERT_FALSE(offlinePlayerId.empty())
        << "Failed to create offline player";

      // Cleanup by ID: child tables first, parent tables last
      dbCleanupLater(DB_SNEEZY,
        std::format("DELETE FROM player WHERE id IN ({}, {})", onlinePlayerId,
          offlinePlayerId));
      dbCleanupLater(DB_SNEEZY,
        std::format("DELETE FROM account WHERE account_id = {}", acctId));
    }

    void makeImmortal(TestCharacter& tc) {
      tc.ch->setMaxLevel(GOD_LEVEL1);
      tc.ch->addPlayerAction(PLR_IMMORTAL);
    }

    TestCharacter* teller = nullptr;
    TestCharacter* onlineAlt = nullptr;
    TRoom* room = nullptr;
    sstring onlineName;
    sstring offlineName;
    sstring acctName;
    sstring acctId;
    sstring onlinePlayerId;
    sstring offlinePlayerId;
};

TEST_F(TellAltDetectionTest, ImmortalSeesAltHintForOfflineAlt) {
  makeImmortal(*teller);

  teller->ch->doTell(offlineName, "test");
  const sstring output = teller->drainOutput();

  EXPECT_NE(output.find("fail to tell"), sstring::npos);
  EXPECT_NE(output.find("logged in under the same account"), sstring::npos);
}

TEST_F(TellAltDetectionTest, MortalDoesNotSeeAltHint) {
  teller->ch->doTell(offlineName, "test");
  const sstring output = teller->drainOutput();

  EXPECT_NE(output.find("fail to tell"), sstring::npos);
  EXPECT_EQ(output.find("logged in under the same account"), sstring::npos);
}

TEST_F(TellAltDetectionTest, NonexistentPlayerShowsNoAltHint) {
  makeImmortal(*teller);

  teller->ch->doTell("Zzznonexist", "test");
  const sstring output = teller->drainOutput();

  EXPECT_NE(output.find("fail to tell"), sstring::npos);
  EXPECT_EQ(output.find("logged in under the same account"), sstring::npos);
}
