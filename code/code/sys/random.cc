//////////////////////////////////////////////////////////////////////////
//
// SneezyMUD - All rights reserved, SneezyMUD Coding Team
//
// random.cc - Random number generation utilities
//
//////////////////////////////////////////////////////////////////////////

#include "random.h"

// Meyer's singleton pattern: construct on first use to avoid static
// initialization order fiasco. Global games use the RNG in their constructors,
// which can execute before other translation units' globals are initialized.
std::mt19937& getRng() {
  static std::mt19937 rng{std::random_device{}()};
  return rng;
}
