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

// Door traps store the setter as a bare name on the exit (no carrier object)
// and resolve it through trapSetterByName, so cover that resolver directly.
TEST_F(TrapSetterResolve, ByNameResolvesToLiveBeing) {
  TRoom& room = makeRoom(49963);
  TestCharacter& setter = makeCharacter("Doorsetter");
  placeInRoom(setter, room);

  EXPECT_EQ(trapSetterByName("Doorsetter"), setter.ch);
}

TEST_F(TrapSetterResolve, ByNameNullWhenEmpty) {
  EXPECT_EQ(trapSetterByName(""), nullptr);
}

TEST_F(TrapSetterResolve, ByNameNullWhenUnresolvable) {
  EXPECT_EQ(trapSetterByName("Nobodyhere"), nullptr);
}

// NOTE: two pieces of the setter machinery are not unit-tested here because
// both pull in game state the lightweight GameFixture cannot satisfy:
//   - recordTrapSetter() string-stamps the carrier via TObj::swapToStrung(),
//     which dereferences obj_index[getItemIndex()]; a bare makeContainer() has
//     no index entry, so the record->resolve round-trip can only run against a
//     real obj table (functional suite / running server).
//   - The attribution *damage* behavior (a setter-credited hit reaching the
//     victim, and a lethal hit propagating death) goes through
//     TBeing::applyDamage, which loads race files (asserts "No default race
//     file" without them).
// This file covers the unit-testable piece: trapSetter() name resolution above.
