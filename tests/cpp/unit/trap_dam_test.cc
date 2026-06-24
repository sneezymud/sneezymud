// Verify trapSetSkill[] stays ordered to match the trap_targ_t enum, so the
// index-based lookups in getTrapDam()/getTrapLearn() resolve to the correct
// set-trap skill for each target. Pure table, no game state needed.

#include <gtest/gtest.h>

#include "game_fixture.h"
#include "handler.h"
#include "trap.h"

TEST(TrapSetSkill, OrderingMatchesTrapTargT) {
  EXPECT_EQ(trapSetSkill[TRAP_TARG_DOOR], SKILL_SET_TRAP_DOOR);
  EXPECT_EQ(trapSetSkill[TRAP_TARG_CONT], SKILL_SET_TRAP_CONT);
  EXPECT_EQ(trapSetSkill[TRAP_TARG_MINE], SKILL_SET_TRAP_MINE);
  EXPECT_EQ(trapSetSkill[TRAP_TARG_GRENADE], SKILL_SET_TRAP_GREN);
  EXPECT_EQ(trapSetSkill[TRAP_TARG_ARROW], SKILL_SET_TRAP_ARROW);
}

// Resolution of a trap's recorded setter name to a live being.
class TrapSetterResolve : public GameFixture {};

TEST_F(TrapSetterResolve, ResolvesStoredNameToLiveBeing) {
  TRoom& room = makeRoom(49960);
  TestCharacter& setter = makeCharacter("Settername");
  placeInRoom(setter, room);

  TChest* carrier = makeContainer();
  auto* ed = new extraDescription();
  ed->next = carrier->ex_description;
  carrier->ex_description = ed;
  ed->keyword = TRAP_EX_DESC;
  ed->description = "Settername";

  EXPECT_EQ(trapSetter(carrier), setter.ch);

  delete carrier;
}

TEST_F(TrapSetterResolve, NullWhenNoSetterRecorded) {
  TChest* carrier = makeContainer();
  EXPECT_EQ(trapSetter(carrier), nullptr);
  delete carrier;
}

TEST_F(TrapSetterResolve, NullWhenNameUnresolvable) {
  TChest* carrier = makeContainer();
  auto* ed = new extraDescription();
  ed->next = carrier->ex_description;
  carrier->ex_description = ed;
  ed->keyword = TRAP_EX_DESC;
  ed->description = "Nobodyhere";

  EXPECT_EQ(trapSetter(carrier), nullptr);

  delete carrier;
}

// NOTE: the attribution *damage* behavior (a setter-credited hit reaching the
// victim, and a lethal hit propagating death) is not unit-tested here:
// TBeing::applyDamage pulls in race-file machinery the lightweight GameFixture
// cannot satisfy (it asserts "No default race file"). That behavior is verified
// in the functional suite / on a running server, where full game state exists.
// This file covers the unit-testable piece: trapSetter() name resolution above.
