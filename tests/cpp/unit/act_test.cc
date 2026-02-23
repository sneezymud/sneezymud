// Integration tests for act() message routing.
// Verifies each routing mode delivers to the correct recipients and
// does NOT leak to incorrect ones — something functional tests can't
// easily assert on (they'd need multiple TCP clients and can only
// observe presence, not absence).

#include <gtest/gtest.h>

#include "comm.h"
#include "game_fixture.h"
#include "room.h"
#include "sstring.h"

class ActTest : public GameFixture {
  protected:
    void SetUp() override {
      GameFixture::SetUp();

      room = &makeRoom(49999);
      actor = &makeCharacter("Brutius");
      victim = &makeCharacter("Mapmaker");
      bystander = &makeCharacter("Onlooker");

      placeInRoom(*actor, *room);
      placeInRoom(*victim, *room);
      placeInRoom(*bystander, *room);
    }

    TRoom* room = nullptr;
    TestCharacter* actor = nullptr;
    TestCharacter* victim = nullptr;
    TestCharacter* bystander = nullptr;
};

TEST_F(ActTest, ToCharRoutesOnlyToActor) {
  act("You start riding $N.", false, actor->ch, nullptr, victim->ch, TO_CHAR);

  const sstring actorOut = actor->drainOutput();
  EXPECT_NE(actorOut.find("You start riding Mapmaker."), sstring::npos)
    << "Actor should receive the TO_CHAR message, got: " << actorOut;

  EXPECT_TRUE(victim->desc->output.empty())
    << "Victim should not receive TO_CHAR message";
  EXPECT_TRUE(bystander->desc->output.empty())
    << "Bystander should not receive TO_CHAR message";
}

TEST_F(ActTest, ToVictRoutesOnlyToVictim) {
  act("$n hops on your back!", false, actor->ch, nullptr, victim->ch, TO_VICT);

  const sstring victimOut = victim->drainOutput();
  EXPECT_NE(victimOut.find("Brutius hops on your back!"), sstring::npos)
    << "Victim should receive the TO_VICT message, got: " << victimOut;

  EXPECT_TRUE(actor->desc->output.empty())
    << "Actor should not receive TO_VICT message";
  EXPECT_TRUE(bystander->desc->output.empty())
    << "Bystander should not receive TO_VICT message";
}

TEST_F(ActTest, ToNotVictRoutesOnlyToBystanders) {
  act("$n starts riding $N.", false, actor->ch, nullptr, victim->ch,
    TO_NOTVICT);

  const sstring bystanderOut = bystander->drainOutput();
  EXPECT_NE(bystanderOut.find("Brutius starts riding Mapmaker."), sstring::npos)
    << "Bystander should receive TO_NOTVICT message, got: " << bystanderOut;

  EXPECT_TRUE(actor->desc->output.empty())
    << "Actor should not receive TO_NOTVICT message";
  EXPECT_TRUE(victim->desc->output.empty())
    << "Victim should not receive TO_NOTVICT message";
}

TEST_F(ActTest, ToRoomRoutesToAllExceptActor) {
  act("$n waves.", false, actor->ch, nullptr, nullptr, TO_ROOM);

  EXPECT_TRUE(actor->desc->output.empty())
    << "Actor should not receive TO_ROOM message";

  const sstring victimOut = victim->drainOutput();
  EXPECT_NE(victimOut.find("Brutius waves."), sstring::npos)
    << "Victim should receive TO_ROOM message, got: " << victimOut;

  const sstring bystanderOut = bystander->drainOutput();
  EXPECT_NE(bystanderOut.find("Brutius waves."), sstring::npos)
    << "Bystander should receive TO_ROOM message, got: " << bystanderOut;
}
