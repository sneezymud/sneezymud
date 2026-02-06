---
title: Transformation System
description: Player and spell-triggered transformations including polymorph, disguise, shapeshift, and werewolf mechanics
keywords: [polymorph, disguise, shapeshift, werewolf, body swap, descriptor transfer, transformation lifecycle]
category: critical
primary_symbols:
  functions: [doReturn, SwitchStuff, DisguiseStuff, updateAffects, checkIdling, updateTickStuff]
  classes: [TBeing, TPerson, TMonster, Descriptor]
  enums: [POLY_TYPE_NONE, POLY_TYPE_SWITCH, POLY_TYPE_DISGUISE, POLY_TYPE_SHAPESHIFT, POLY_TYPE_POLYMORPH, ACT_POLYSELF, TOG_TRANSFORMED_LYCANTHROPE, SPELL_POLYMORPH, SPELL_SHAPESHIFT, SKILL_DISGUISE, ALREADY_DELETED]
---

# Transformation System

## Overview

Transformations allow characters to inhabit a different mob body while retaining their identity. The system handles four player-accessible transformation types: polymorph (mage spell), shapeshift (shaman spell), disguise (thief skill), and lycanthropy (werewolf curse). Each moves a player's descriptor to control a different mob, storing the original body until the transformation ends.

The transformation system shares architecture with the immortal switch command (see [23-snoop-switch.md](23-snoop-switch.md)) but adds gameplay mechanics: stat transfer, equipment handling, duration limits, and environmental restrictions.

When a player transforms, the system creates or designates a mob body, transfers the player's descriptor to control it, and stores the original body in `Room::POLY_STORAGE`. The `desc->original` pointer maintains the link back. When the transformation ends (duration expires, player dies, or manual return), the system reverses the process via `doReturn()`.

## Patterns

### Pointer Validation

Always validate `desc` and `desc->original` before dereferencing. Either pointer can be null during transformation edge cases.

```cpp
// WRONG - Crash if original is null
TPerson* per = desc->original;
per->getName();

// CORRECT - Always validate first
if (!desc || !desc->original) {
  return FALSE;
}
TPerson* per = desc->original;
per->getName();
```

The `original` field becomes null when:
- The transformation affect expires and `updateAffects()` removes it
- The player idles out and `checkIdling()` reverses the transformation
- The player link-dies and descriptor cleanup runs
- The transformed mob dies and `rawKill()` reverses the transformation

### Scheduler Integration

Use `doReturn()` with `deleteMob=false` in scheduler procs, then return `true` to let the scheduler handle deletion. Direct deletion causes use-after-free.

```cpp
// WRONG - Deleting directly in scheduler breaks iterators
bool procCharBad::run(const TPulse& pl, TBeing* ch) const {
  if (shouldEndTransform(ch)) {
    ch->doReturn("", WEAR_NOWHERE, true);  // Deletes ch!
    return false;  // CRASH - ch is freed memory
  }
  return false;
}

// CORRECT - Signal deletion to scheduler
bool procCharGood::run(const TPulse& pl, TBeing* ch) const {
  if (shouldEndTransform(ch)) {
    ch->doReturn("", WEAR_NOWHERE, true, false);  // Move to storage
    return true;  // Scheduler handles deletion
  }
  return false;
}
```

Never access the mob after calling `doReturn()` with `deleteMob=true`. The mob is freed immediately.

Always clear `ACT_POLYSELF` before deletion to prevent cleanup issues.

### Forced Transformation Handling

Forced transformations (werewolf lycanthropy) set `TOG_TRANSFORMED_LYCANTHROPE` and cannot be manually reversed. Always check this bit before allowing return.

```cpp
if (hasQuestBit(TOG_TRANSFORMED_LYCANTHROPE)) {
  sendTo("You cannot control your transformation.\n\r");
  return FALSE;
}
```

## Reference

### Transformation Types

| Type | Enum Value | Source | Transfers Stats | Transfers Equipment |
|------|------------|--------|-----------------|---------------------|
| None | `POLY_TYPE_NONE` | Not transformed | N/A | N/A |
| Switch | `POLY_TYPE_SWITCH` | Immortal switch command | No | No |
| Disguise | `POLY_TYPE_DISGUISE` | Thief disguise skill, werewolf | Yes | Yes |
| Shapeshift | `POLY_TYPE_SHAPESHIFT` | Shaman shapeshift spell | Yes | Yes |
| Polymorph | `POLY_TYPE_POLYMORPH` | Mage polymorph spell | Yes | Yes |

Werewolf transformation uses `POLY_TYPE_DISGUISE` with the `TOG_TRANSFORMED_LYCANTHROPE` quest bit set to distinguish from player-activated disguise skill.

### Descriptor State During Transformation

| Field | Normal | Transformed |
|-------|--------|-------------|
| `mob->desc` | N/A | Player's descriptor |
| `mob->desc->original` | N/A | Original player body |
| `mob->desc->character` | N/A | The transformed mob |
| `person->polyed` | `POLY_TYPE_NONE` | Transformation type |
| `person->desc` | Player's descriptor | NULL (moved to mob) |
| `mob->specials.act` | Normal | Has `ACT_POLYSELF` set |
| `mob->orig` | NULL | Backup pointer to original |

The `orig` field on the mob stores a backup pointer to the original for reconnection after linkdeath.

### Stat Transfer Functions

| Function | Used By | Transfers |
|----------|---------|-----------|
| `SwitchStuff()` | Polymorph, Shapeshift | Equipment, inventory, money, exp, pracs, mana, move, piety, lifeforce, affects |
| `DisguiseStuff()` | Disguise, Werewolf | Equipment, stats, skills |

### Key Scheduler Procs

| Proc | Purpose |
|------|---------|
| `procCharLycanthropy` | Werewolf transformation timer |
| `procCharAffects` | Transformation affect expiration |


## Implementation

### Descriptor Swap During Transformation

When a player transforms, the system creates a new mob and moves the player's descriptor to it. The original body is stored in `Room::POLY_STORAGE`.

After transformation:
- `mob->desc` points to the player's descriptor
- `mob->desc->original` points to the original body
- `mob->desc->character` points to the transformed mob
- `person->polyed` is set to the transformation type
- `person->desc` is null (descriptor moved to mob)
- `mob->specials.act` has `ACT_POLYSELF` set

### Transformation Cleanup via doReturn()

The `doReturn()` function handles reverting transformations. It takes a `deleteMob` parameter defaulting to true. When true, it deletes the mob immediately after swapping the descriptor back. When false, it moves the mob to storage and lets the caller handle deletion.

In scheduler contexts, use `deleteMob=false` and return true from the proc. The scheduler collects characters to delete in a batch, avoiding use-after-free from deleting mid-iteration.

The function:
1. Validates `desc` and `desc->original` exist
2. Transfers equipment and stats back via `SwitchStuff()` (or `DisguiseStuff()` for disguise-based transformations)
3. Moves the original body back from storage
4. Swaps the descriptor
5. Clears transformation state
6. Either deletes or stores the mob

### Automatic Transformation Affect Removal

The `updateAffects()` function in periodic.cc removes transformation affects when the descriptor relationship becomes invalid. If `desc` or `desc->original` is null for a polymorph, disguise, or shapeshift affect, the affect is removed automatically. This protects against orphaned transformation affects on mobs whose players disconnected.

### Shapeshift Indoor Restriction

Shapeshift transformations cannot be cast indoors. The indoor restriction is enforced at cast time only, in disc_shaman_frog.cc. Once a shapeshift is active, moving indoors does not end the transformation.

### Linkdeath During Transformation

When a transformed player loses connection, the mob loses its descriptor but the `orig` field preserves the pointer to the original body. On reconnection, `connect.cc` restores `desc->original` from the `orig` field. The original body is not considered linkdead because `polyed != POLY_TYPE_NONE`.

### Idling Timeout for Transformed Characters

The `checkIdling()` function handles idle timeout specially for transformed characters. It:
1. Checks `desc->original` timer
2. Transfers stats back via `SwitchStuff()`
3. Resets `polyed` to `POLY_TYPE_NONE`
4. Swaps the descriptor before idling out

### Death During Transformation

When a transformed mob dies, `die()` detects the polymorph/switch state and calls `doReturn()` before death processing. The player survives in their original body. For `POLY_TYPE_SWITCH` (immortal switch), `doReturn()` is called without deleting the mob, then `rawKill()` is called recursively on the transformed mob.

See [01-memory-safety.md](01-memory-safety.md) for DELETE flag handling during death.

## Troubleshooting

### Crash: Use-After-Free on Transformed Mob

**Symptom:** Crash in scheduler proc after transformation ends.

**Cause:** Called `doReturn()` with `deleteMob=true` (the default) then continued execution.

**Diagnostic:** Check if `doReturn()` was called without the fourth parameter or with `true`.

**Fix:** Call `doReturn("", WEAR_NOWHERE, true, false)` to move mob to storage, then return true from the proc to let the scheduler delete.

### Crash: Null Dereference on desc->original

**Symptom:** Null pointer crash when accessing `desc->original`.

**Cause:** Transformation state became inconsistent, or code assumed transformation when none existed.

**Diagnostic:** Check if `desc` and `desc->original` were validated before use.

**Fix:** Always validate both pointers: `if (desc && desc->original)` before dereferencing.

### Bug: Transformation Affect Lingers

**Symptom:** Orphaned mob has polymorph/disguise/shapeshift affect but no descriptor.

**Cause:** Transformation ended abnormally without proper cleanup.

**Diagnostic:** Check if mob has transformation affect but `desc` or `desc->original` is null.

**Fix:** The automatic cleanup in `updateAffects()` should catch this. If not, verify the affect type check includes all transformation spells.

### Bug: Cannot Return from Werewolf Form

**Symptom:** Player cannot use return command while transformed.

**Cause:** This is intentional. Werewolf transformation sets `TOG_TRANSFORMED_LYCANTHROPE`.

**Diagnostic:** Check `hasQuestBit(TOG_TRANSFORMED_LYCANTHROPE)`.

**Fix:** Werewolf transformation must expire naturally or be ended by death. This is a gameplay feature, not a bug.

### Bug: Stats Not Restored After Transformation

**Symptom:** Player returns to original body with incorrect stats.

**Cause:** `SwitchStuff()` or `DisguiseStuff()` not called, or called in wrong direction.

**Diagnostic:** Trace the return path to verify stat transfer function is called.

**Fix:** Ensure the appropriate transfer function is called based on transformation type.

### Bug: Shapeshift Ends Unexpectedly Indoors

**Symptom:** Shapeshift transformation ends when entering a building.

**Cause:** This is intentional. Shapeshift requires connection to nature.

**Diagnostic:** Check room sector type and flags.

**Fix:** Shapeshift players must stay outdoors. This is a gameplay feature, not a bug.
