#include <format>

#include <gtest/gtest.h>

#include "database_fixture.h"
#include "extern.h"
#include "person.h"

class TellAltDetectionTest : public DatabaseFixture {
  protected:
    void SetUp() override {
      DatabaseFixture::SetUp();

      room = &makeRoom(49950);

      // The "teller" character - will be configured as immortal or mortal per
      // test
      teller = &makeCharacter("Teller");
      placeInRoom(*teller, *room);
      teller->ch->spec = 0;  // Prevent spec proc side effects

      // "OnlineAlt" exists in character_list - findTellTarget can find it.
      // This is the character the alt-detection query will resolve to.
      onlineAlt = &makeCharacter("Onlinealt");
      placeInRoom(*onlineAlt, *room);
      onlineAlt->ch->spec = 0;

      // Create account + 2 player rows sharing the same account_id.
      // "Onlinealt" is in character_list (above). "Offlinealt" is not.
      dbExecute(DB_SNEEZY,
        "INSERT INTO account (name, passwd) VALUES ('testacct_tell', 'x')");
      dbCleanupLater(DB_SNEEZY,
        "DELETE FROM account WHERE name = 'testacct_tell'");

      auto acctId = dbQueryScalar(DB_SNEEZY,
        "SELECT account_id FROM account WHERE name = 'testacct_tell'");

      dbExecute(DB_SNEEZY,
        std::format(
          "INSERT INTO player (name, account_id) VALUES ('Onlinealt', {})",
          acctId));
      dbCleanupLater(DB_SNEEZY, "DELETE FROM player WHERE name = 'Onlinealt'");

      dbExecute(DB_SNEEZY,
        std::format(
          "INSERT INTO player (name, account_id) VALUES ('Offlinealt', {})",
          acctId));
      dbCleanupLater(DB_SNEEZY, "DELETE FROM player WHERE name = 'Offlinealt'");
    }

    void makeImmortal(TestCharacter& tc) {
      tc.ch->setMaxLevel(GOD_LEVEL1);
      tc.ch->addPlayerAction(PLR_IMMORTAL);
    }

    TestCharacter* teller = nullptr;
    TestCharacter* onlineAlt = nullptr;
    TRoom* room = nullptr;
};

TEST_F(TellAltDetectionTest, ImmortalSeesAltHintForOfflineAlt) {
  makeImmortal(*teller);

  teller->ch->doTell("Offlinealt", "test");
  const sstring output = teller->drainOutput();

  EXPECT_NE(output.find("fail to tell"), sstring::npos);
  EXPECT_NE(output.find("logged in under the same account"), sstring::npos);
}

TEST_F(TellAltDetectionTest, MortalDoesNotSeeAltHint) {
  // teller is mortal by default (level 1, no PLR_IMMORTAL)

  teller->ch->doTell("Offlinealt", "test");
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
