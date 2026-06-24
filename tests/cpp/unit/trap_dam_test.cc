// Verify trapSetSkill[] stays ordered to match the trap_targ_t enum, so the
// index-based lookups in getTrapDam()/getTrapLearn() resolve to the correct
// set-trap skill for each target. Pure table, no game state needed.

#include <gtest/gtest.h>

#include "trap.h"

TEST(TrapSetSkill, OrderingMatchesTrapTargT) {
  EXPECT_EQ(trapSetSkill[TRAP_TARG_DOOR], SKILL_SET_TRAP_DOOR);
  EXPECT_EQ(trapSetSkill[TRAP_TARG_CONT], SKILL_SET_TRAP_CONT);
  EXPECT_EQ(trapSetSkill[TRAP_TARG_MINE], SKILL_SET_TRAP_MINE);
  EXPECT_EQ(trapSetSkill[TRAP_TARG_GRENADE], SKILL_SET_TRAP_GREN);
  EXPECT_EQ(trapSetSkill[TRAP_TARG_ARROW], SKILL_SET_TRAP_ARROW);
}
