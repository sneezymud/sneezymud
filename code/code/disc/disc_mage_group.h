#pragma once

#include <functional>

class TBeing;

// Whether an applyX helper should let affectJoin emit its own
// "can't increase the duration of that effect any further" message.
// Single-target casts want Verbose so the user sees feedback when re-casting
// on an already-buffed target; group casts want Suppressed so the per-target
// messages don't spam, leaving the caller to roll up a single epilogue.
enum class GroupCastMessages { Verbose, Suppressed };

// Iterates the caster's room, applying applyFn to each TBeing the caster
// considers in-group.  skipFn is consulted before applyFn and lets per-spell
// preconditions short-circuit a target without counting it as buffed.
// applyFn returns true if the target was buffed.  Returns true if at least
// one target was buffed.
bool forEachGroupBuffTarget(TBeing* caster,
  std::function<bool(TBeing*)> applyFn,
  std::function<bool(TBeing*)> skipFn = [](TBeing*) { return false; });
