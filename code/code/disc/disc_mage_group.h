#pragma once

#include <functional>

#include "enum.h"

class TBeing;

// Iterates the caster's room, applying applyFn to each TBeing the caster
// considers in-group.  skipFn is consulted before applyFn and lets per-spell
// preconditions short-circuit a target without counting it as buffed.
// applyFn returns true if the target was buffed.  Returns true if at least
// one target was buffed.
bool forEachGroupBuffTarget(TBeing* caster,
  std::function<bool(TBeing*)> applyFn,
  std::function<bool(TBeing*)> skipFn = [](TBeing*) { return false; });
