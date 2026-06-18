// Verify the per-type additive damage modifiers in trapDamMod() match the
// table documented in docs/systems/important/trap-system.md.
// trapDamMod is pure (no game state), so no fixture is needed.

#include <gtest/gtest.h>

#include "trap.h"

TEST(TrapDamMod, NoneReturnsZero) { EXPECT_EQ(trapDamMod(DOOR_TRAP_NONE), 0); }
TEST(TrapDamMod, FireReturnsZero) { EXPECT_EQ(trapDamMod(DOOR_TRAP_FIRE), 0); }

TEST(TrapDamMod, PositiveModifiers) {
  EXPECT_EQ(trapDamMod(DOOR_TRAP_TNT), 3);
  EXPECT_EQ(trapDamMod(DOOR_TRAP_DISEASE), 3);
  EXPECT_EQ(trapDamMod(DOOR_TRAP_FROST), 3);
  EXPECT_EQ(trapDamMod(DOOR_TRAP_DISK), 3);
  EXPECT_EQ(trapDamMod(DOOR_TRAP_ENERGY), 5);
  EXPECT_EQ(trapDamMod(DOOR_TRAP_TELEPORT), 5);
  EXPECT_EQ(trapDamMod(DOOR_TRAP_SLEEP), 1);
  EXPECT_EQ(trapDamMod(DOOR_TRAP_ACID), 1);
  EXPECT_EQ(trapDamMod(DOOR_TRAP_BOLT), 1);
}

TEST(TrapDamMod, NegativeModifiers) {
  EXPECT_EQ(trapDamMod(DOOR_TRAP_POISON), -1);
  EXPECT_EQ(trapDamMod(DOOR_TRAP_BLADE), -3);
  EXPECT_EQ(trapDamMod(DOOR_TRAP_SPIKE), -5);
  EXPECT_EQ(trapDamMod(DOOR_TRAP_PEBBLE), -5);
  EXPECT_EQ(trapDamMod(DOOR_TRAP_HAMMER), -10);
}
