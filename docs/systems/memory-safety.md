---
title: Memory Safety and DELETE Flags
description: Core memory management system using return flags to signal object deletion, preventing use-after-free and double-delete crashes
keywords: [DELETE_THIS, DELETE_VICT, DELETE_ITEM, IS_SET_DELETE, REM_DELETE, ownership, flag propagation, polymorph, death processing]
category: Critical Systems
related: [combat-rounds.md, scheduler-pulses.md, command-implementation.md]
---

# Memory Safety and DELETE Flags

## Overview

SneezyMUD's DELETE flag system is the core memory management mechanism. When functions operate on beings or objects that may die or be destroyed, they return integer flags indicating what should be deleted. The caller, who holds the pointer reference, performs the actual deletion.

This design prevents two critical memory bugs. Use-after-free occurs when code deletes an object while another function higher in the call stack still references it. Double-delete occurs when multiple callers each think they own a pointer and both try to delete it. By separating the decision to delete from the act of deletion, ownership becomes explicit.

The system has one fundamental rule: whoever resolved or found a pointer owns it and is responsible for its deletion. If a function receives a pointer as a parameter, the caller owns it. If a function looks up a pointer itself, the callee owns it.

This system extends to transformations (polymorph, switch, disguise) where a player's descriptor moves to a different mob body. It also governs death processing, where characters must clean up combat state, transfer group leadership, and create corpses before signaling their deletion.

## Patterns

### Flag Checking

Always use `IS_SET_DELETE()` for DELETE flags. Never use `IS_SET()` which fails to detect the combined bit pattern.

```cpp
// WRONG - Will not detect DELETE flags reliably
if (IS_SET(rc, DELETE_THIS)) {
  delete victim;
}

// CORRECT - Use the specialized macro
if (IS_SET_DELETE(rc, DELETE_THIS)) {
  delete victim;
}
```

Always check return values from functions that can trigger death or destruction. Common triggers: `reconcileDamage()`, `applyDamage()`, `die()`, `rawKill()`, `doMove()`, `crashLanding()`, `get()`, `drop()`, `put()`, `give()`, trap functions, `checkSpec()` on mobs/objects/rooms, and `doReturn()`.

Always check immediately after the function call. Never access the pointer again until the check completes.

```cpp
// WRONG - Return value ignored, victim may be deleted
victim->someAction();
victim->sendTo("Message");  // CRASH if victim deleted

// CORRECT - Check return value before continuing
int rc = victim->someAction();
if (IS_SET_DELETE(rc, DELETE_THIS)) {
  return DELETE_VICT;  // Propagate to caller
}
victim->sendTo("Message");
```

### Ownership

Return the DELETE flag when the caller passed the pointer as a parameter. Translate the flag appropriately: if callee's method was called on the victim, callee's `DELETE_THIS` becomes caller's `DELETE_VICT`.

```cpp
// Pattern 1: Caller owns the victim (passed as parameter)
int someFunction(TBeing* victim) {
  int rc = victim->doSomething();
  if (IS_SET_DELETE(rc, DELETE_THIS)) {
    // Return flag so caller can delete
    return DELETE_VICT;
  }
  return FALSE;
}

// Pattern 2: We own the victim (resolved locally)
int someFunction() {
  TBeing* victim = get_char_room_vis(this, arg);
  if (!victim) return FALSE;

  int rc = victim->doSomething();
  if (IS_SET_DELETE(rc, DELETE_THIS)) {
    // We resolved victim, we delete it
    delete victim;
    victim = nullptr;
    REM_DELETE(rc, DELETE_THIS);  // Clear flag we handled
  }
  return rc;
}
```

Delete directly and clear the flag with `REM_DELETE()` when you resolved the pointer yourself via lookup functions like `get_char_room_vis()`.

Never delete objects you did not resolve. If someone else gave you a pointer, return a flag and let them handle it.

### Combined Flags

Functions can return multiple DELETE flags combined with bitwise OR when multiple objects should be deleted.

```cpp
// Function that might delete both attacker and victim
int rc = performAttack(ch, victim, weapon);

// Check and handle each flag independently
if (IS_SET_DELETE(rc, DELETE_THIS)) {
  ch->reformGroup();
  delete ch;
  ch = nullptr;
}
if (IS_SET_DELETE(rc, DELETE_VICT)) {
  victim->reformGroup();
  delete victim;
  victim = nullptr;
}
```

Never assume only one flag will be set. Combat, traps, and special attacks can kill multiple participants simultaneously.

### Combat Cleanup

Always call `reformGroup()` before deleting any character. Failing to do so leaves followers with dangling master pointers.

```cpp
// WRONG - Followers left with dangling master pointer
if (IS_SET_DELETE(rc, DELETE_THIS)) {
  delete ch;
  ch = nullptr;
}

// CORRECT - Reform group first
if (IS_SET_DELETE(rc, DELETE_THIS)) {
  ch->reformGroup();
  delete ch;
  ch = nullptr;
}
```

Always use the global iterator cache `gCombatNext` when traversing combat lists. Local iterators become invalid when combatants die.

Never continue execution after detecting a DELETE flag. Check immediately and return or break from the current scope.

### Transformation Safety

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

The `original` field becomes null when: the transformation affect expires and is removed by `updateAffects()`, the player idles out and `checkIdling()` reverses the transformation, the player link-dies and descriptor cleanup runs, or the transformed mob dies and `rawKill()` reverses the transformation.

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

### Death Processing

Check for -1 return from `reconcileDamage()` to detect death. It returns damage dealt on survival, -1 on death. It does not return DELETE flags.

```cpp
// WRONG - Checking for DELETE flag
int dam = reconcileDamage(victim, ...);
if (IS_SET_DELETE(dam, DELETE_VICT)) {  // Never triggers!
  return DELETE_VICT;
}

// CORRECT - Check for -1 death sentinel
int dam = reconcileDamage(victim, ...);
if (dam == -1) {
  return DELETE_VICT;
}
```

Never check `reconcileDamage()` with `IS_SET_DELETE()`. The -1 return is not a flag.

Remember that `die()` returns `DELETE_THIS`, not `DELETE_VICT`. The dying being signals its own deletion.

```cpp
// WRONG - Checking for wrong flag
int rc = victim->die(DAMAGE_NORMAL);
if (IS_SET_DELETE(rc, DELETE_VICT)) {  // Never triggers!
  delete victim;
}

// CORRECT - die returns DELETE_THIS
int rc = victim->die(DAMAGE_NORMAL);
if (IS_SET_DELETE(rc, DELETE_THIS)) {
  delete victim;  // Or translate: return DELETE_VICT;
}
```

Always translate appropriately when propagating: if you called `victim->die()` and it returns `DELETE_THIS`, return `DELETE_VICT` to your caller since the victim is their "vict".

## Reference

### Flag Definitions

| Flag | Bits | Meaning |
|------|------|---------|
| `DELETE_THIS` | (1<<6) \| (1<<29) | Delete the object the method was called on |
| `DELETE_VICT` | (1<<7) \| (1<<29) | Delete the victim/target parameter |
| `DELETE_ITEM` | (1<<5) \| (1<<29) | Delete the item/object parameter |
| `ALREADY_DELETED` | (1<<8) \| (1<<29) | Object was already handled (transformation) |
| `RET_STOP_PARSING` | (1<<9) \| (1<<29) | Stop command parsing |

The high bit (1 << 29) distinguishes DELETE flags from damage integers.

### Utility Macros

| Macro | Purpose |
|-------|---------|
| `IS_SET_DELETE(value, flag)` | Check if DELETE flag is set |
| `ADD_DELETE(value, flag)` | Add a DELETE flag to return value |
| `REM_DELETE(value, flag)` | Remove a DELETE flag from return value |

### Transformation Types

| Type | Enum Value | Source | Transfers Stats | Transfers Equipment |
|------|------------|--------|-----------------|---------------------|
| None | `POLY_TYPE_NONE` | Not transformed | N/A | N/A |
| Switch | `POLY_TYPE_SWITCH` | Immortal switch command | No | No |
| Disguise | `POLY_TYPE_DISGUISE` | Thief disguise skill, werewolf | Yes | Yes |
| Shapeshift | `POLY_TYPE_SHAPESHIFT` | Shaman shapeshift spell | Yes | Yes |
| Polymorph | `POLY_TYPE_POLYMORPH` | Mage polymorph spell | Yes | Yes |

Werewolf transformation uses `POLY_TYPE_DISGUISE` with the `TOG_TRANSFORMED_LYCANTHROPE` quest bit set to distinguish from player-activated disguise skill.

### Death Penalties

| Aspect | Formula/Behavior |
|--------|------------------|
| XP loss | min(current_exp/5, 25 * mob_exp(level)) |
| PvP XP loss | Normal formula divided by 10 |
| Age increase | Random 0-3 years (0 for level 10 and below) |
| Arena exemption | No XP loss, no age increase |
| Free death affect | Decrements modifier, skips penalties |

### PC vs NPC Death Differences

| Aspect | PC | NPC |
|--------|----|----|
| XP loss | Yes | No |
| Age penalty | Yes | No |
| Corpse type | TPCorpse (vnum -2) | TCorpse (mob vnum) |
| Corpse flags | CORPSE_NO_REGEN | None unless vnum < 0 |
| Group handling | reformGroup() | None |
| Rent cleanup | removeRent() | N/A |
| Follower cleanup | removeFollowers() | N/A |
| Permadeath logging | Yes | No |
| Save location | Room::NOWHERE | N/A |
| Limb healing | All limbs restored | N/A |
| Disease cleanup | All diseases removed | N/A |

### Pointer Validation Scenarios

| Pointer | Must Validate | Becomes Null When |
|---------|--------------|-------------------|
| `desc` | Before any descriptor access | Link death, logout, character deletion |
| `desc->original` | Before transformation operations | Affect expiry, idle timeout, manual return, death |
| `desc->character` | Before accessing transformed body | Descriptor swap during return |
| `victim` | After reconcileDamage returns -1 | Death processing completes |
| `this` | After DELETE_THIS return | Caller deletion pending |

### Top-Level DELETE Handlers

| Location | Handles |
|----------|---------|
| `TScheduler::runObj()` | Object proc returns |
| `TScheduler::runChar()` | Character proc returns (batch deletion) |
| `perform_violence()` | Combat DELETE_THIS and DELETE_VICT |
| Descriptor loop | parseCommand DELETE_THIS |
| Account menu | DELETE_THIS from doAccountMenu |

### Key Files

| File | Content |
|------|---------|
| `misc/defs.h` | DELETE flag constants |
| `misc/structs.h` | IS_SET_DELETE, ADD_DELETE, REM_DELETE |
| `misc/combat.cc` | die(), rawKill(), reformGroup(), genericKillFix(), stopFighting() |
| `misc/damage.cc` | reconcileDamage(), applyDamage() |
| `misc/immortal.cc` | doReturn() |
| `misc/periodic.cc` | updateAffects(), transformation removal |
| `misc/limits.cc` | checkIdling() transformation handling |
| `sys/process.cc` | TScheduler deletion handlers |
| `sys/connect.cc` | parseCommand(), descriptor loop, preKillCheck() |
| `sys/socket.cc` | proc*::run() adapter functions |

## Implementation

### Flag Bit Structure

DELETE flags use a combined bit pattern with bit 29 set as a discriminator. This distinguishes DELETE flags from damage values, since functions like `reconcileDamage()` return damage integers while combat functions return DELETE flags. The high bit ensures no overlap between valid damage amounts and DELETE flag values.

### Ownership Resolution

When a function receives a pointer as a parameter, the caller owns it. When a function obtains a pointer through lookup (via `get_char_room_vis()`, `get_obj_in_list_vis()`, or similar), the callee owns it.

Ownership determines deletion responsibility. Owners delete directly and clear the flag with `REM_DELETE()`. Non-owners return the appropriate DELETE flag and let the owner handle deletion.

The flag translation pattern handles parameter mapping. When calling a method on a victim and it returns `DELETE_THIS`, that means "delete what the method was called on" which is the victim. The caller translates this to `DELETE_VICT` for its own return value.

Local ownership creates an exception to flag propagation. If a function resolves a victim locally, receives DELETE_THIS from a method call on that victim, and then deletes the victim itself, it must clear the DELETE_THIS flag with REM_DELETE before returning to prevent the caller from seeing a deletion flag for an object the caller never knew existed.

### Descriptor Swap During Transformation

When a player transforms, the system creates a new mob and moves the player's descriptor to it. The original body is stored in `Room::POLY_STORAGE`.

After transformation:
- `mob->desc` points to the player's descriptor
- `mob->desc->original` points to the original body
- `mob->desc->character` points to the transformed mob
- `person->polyed` is set to the transformation type
- `person->desc` is null (descriptor moved to mob)
- `mob->specials.act` has `ACT_POLYSELF` set

The `orig` field on the mob stores a backup pointer to the original for reconnection after linkdeath.

### Transformation Cleanup via doReturn()

The `doReturn()` function handles reverting transformations. It takes a `deleteMob` parameter defaulting to true. When true, it deletes the mob immediately after swapping the descriptor back. When false, it moves the mob to storage and lets the caller handle deletion.

In scheduler contexts, use `deleteMob=false` and return true from the proc. The scheduler collects characters to delete in a batch, avoiding use-after-free from deleting mid-iteration.

The function validates `desc` and `desc->original` exist, transfers equipment and stats back via `SwitchStuff()` (or `DisguiseStuff()` for disguise-based transformations), moves the original body back from storage, swaps the descriptor, clears transformation state, and either deletes or stores the mob.

### Automatic Transformation Affect Removal

The `updateAffects()` function in periodic.cc removes transformation affects when the descriptor relationship becomes invalid. If `desc` or `desc->original` is null for a polymorph, disguise, or shapeshift affect, the affect is removed automatically. This protects against orphaned transformation affects on mobs whose players disconnected.

### Shapeshift Indoor Restriction

Shapeshift transformations cannot survive indoors. The `updateTickStuff()` function checks if a shapeshifted character is in a non-outdoor room and forcibly ends the transformation with a message about needing nature connection. It returns `ALREADY_DELETED` to signal deletion should happen at the scheduler level.

### Linkdeath During Transformation

When a transformed player loses connection, the mob loses its descriptor but the `orig` field preserves the pointer to the original body. On reconnection, `connect.cc` restores `desc->original` from the `orig` field. The original body is not considered linkdead because `polyed != POLY_TYPE_NONE`.

### Idling Timeout for Transformed Characters

The `checkIdling()` function handles idle timeout specially for transformed characters. It checks `desc->original` timer, transfers back via `SwitchStuff()`, resets `polyed` to `POLY_TYPE_NONE`, and swaps the descriptor before idling out.

### Death Flow Pipeline

Every death flows through: `die()` -> `rawKill()` -> `makeCorpse()`.

`die()` handles penalties (XP loss, age increase) and calls `rawKill()`. It checks for polymorph/switch state and returns to original before death. It checks `AFFECT_FREE_DEATHS` and arena room flags to skip penalties. For `POLY_TYPE_SWITCH` transformations, it calls `doReturn()` without deleting the mob, then recursively calls `rawKill()` on the transformed mob.

`rawKill()` handles combat cleanup via `stopFighting()` (removing berserk mode, berserk affects, vampire bite affects), creates the corpse, calls death cry, runs generic cleanup, and handles PC-specific operations (reformGroup, removeRent, removeFollowers, permadeath logging). Shopkeeper special handling deletes all inventory and money since shopkeepers maintain infinite virtual inventory that should not transfer to corpses.

Both functions return `DELETE_THIS` to signal the caller should delete.

### XP Loss Calculation

Death XP loss uses the minimum of two formulas: 20% of current experience, or 25 times the mob XP value for the character's level. This caps losses for high-level characters while ensuring low-level characters don't lose more than they can afford.

PvP deaths divide the result by 10, significantly reducing the penalty for player-versus-player combat.

### Group Leadership Transfer

When a group leader dies, `reformGroup()` transfers leadership to the first eligible follower. It uses a two-pass algorithm preferring PC followers, falling back to any follower. All remaining followers re-attach to the new leader. The `AFF_GROUP` flag is maintained on all members.

This must be called before deletion or followers will have dangling master pointers.

### Generic Death Cleanup

`genericKillFix()` performs universal cleanup: calling `reformGroup()`, removing the dying character from mob hate/fear lists, dispelling magic with double-death safety checks, resetting hunger/thirst, restoring limbs for PCs (except arena), curing diseases for PCs, and setting shamans to 25 HP with 50 lifeforce.

### Double-Death Detection

`genericKillFix()` checks return values from `generic_dispel_magic()` and `genericChaseSpirits()` for DELETE_VICT and logs if detected. This catches cases where spell wearoff effects trigger another death.

### Arena Death Exemption

Deaths in rooms with `ROOM_ARENA` flag skip XP loss, skip age increase, do not heal limbs, and save the character to the current room rather than `Room::NOWHERE`.

### Corpse Creation

The `makeCorpse()` function branches based on PC vs NPC. For PCs, it creates a TPCorpse with vnum -2. For NPCs, it creates a TCorpse with the mob's vnum. All PC corpses receive `CORPSE_NO_REGEN` which prevents cleanup proc destruction. NPC corpses only get this flag if their vnum is negative.

Equipment transfer iterates through all wear slots, calls `unequip()` to handle affect removal, then transfers items to the corpse. Experience loss is embedded in the corpse as a float value for corpse retrieval systems.

### Scheduler Proc Adapter Pattern

All `proc*::run()` functions convert DELETE flags to bool returns. They check `IS_SET_DELETE(rc, DELETE_THIS)` and return true to signal deletion, false to keep. This adapter layer lets the scheduler handle deletion uniformly without knowing the specific DELETE flag semantics of each operation.

Key procs: `procCharLycanthropy` (werewolf transformation), `procCharAffects` (affect expiration), `procCharDrowning`, `procCharFalling`. Each checks for DELETE_THIS and returns true to signal scheduler deletion.

### Combat Loop DELETE Handling

The `perform_violence()` function caches `gCombatNext = ch->next_fighting` before processing each combatant to ensure safe iteration if ch is deleted.

After `hit()` returns, it checks `IS_SET_DELETE(rc, DELETE_VICT)` first. If set, it calls `vict->reformGroup()`, deletes vict, and continues to the next iteration. Then it checks `IS_SET_DELETE(rc, DELETE_THIS)`. If set, it calls `ch->reformGroup()`, deletes ch, and breaks from the loop.

The order matters: checking DELETE_VICT first allows the loop to continue. Checking DELETE_THIS second ensures we break when the primary iterator is invalidated. Both blocks execute for combined flags (DELETE_THIS | DELETE_VICT).

### Iterator Safety in Linked Lists

The combat participant list, river flow list, and rider chains require caching the next pointer before any operation that might delete the current node. For combat: `gCombatNext = ch->next_fighting`. For equipment lists (stuff), use `*(it++)` which post-increments the iterator before dereferencing.

### Architecture Overview

The deletion hierarchy flows from top-level handlers down through adapters to the actual operations:

`gameLoop()` runs the scheduler which invokes `runObj()` and `runChar()`. These call `proc*::run()` adapters which call actual game functions. Game functions return DELETE flags which adapters convert to bool. The scheduler collects true returns and batch-deletes.

`perform_violence()` is a separate top-level handler for combat, directly handling DELETE_THIS and DELETE_VICT from hit resolution.

The descriptor loop in `connect.cc` handles DELETE_THIS from `parseCommand()` for command-triggered deaths.

## Troubleshooting

### Crash: IS_SET Used Instead of IS_SET_DELETE

**Symptom:** DELETE flag checks never trigger despite deaths occurring.

**Cause:** Using `IS_SET()` which does not handle the combined bit pattern.

**Diagnostic:** Search for `IS_SET(` with DELETE flag arguments in recent changes.

**Fix:** Replace with `IS_SET_DELETE()`.

### Crash: Use-After-Free on Character

**Symptom:** Crash accessing character after combat, movement, or trap trigger.

**Cause:** DELETE flag from function was ignored and code continued using the pointer.

**Diagnostic:** Check if the preceding function call could return DELETE_THIS or DELETE_VICT. Common culprits: `reconcileDamage()`, `doMove()`, trap functions, spec procs.

**Fix:** Check the return value immediately with `IS_SET_DELETE()` and return or break before any further access.

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

### Crash: Dangling Follower Pointers

**Symptom:** Crash when iterating followers or accessing group after character death.

**Cause:** Character was deleted without calling `reformGroup()` first.

**Diagnostic:** Check if `reformGroup()` was called before the delete statement.

**Fix:** Always call `reformGroup()` before deleting any character.

### Crash: Double-Delete

**Symptom:** Crash on double-free or invalid pointer.

**Cause:** Multiple functions each thought they owned the pointer and deleted it.

**Diagnostic:** Trace the pointer's origin. Was it passed as a parameter or resolved locally?

**Fix:** Follow ownership rules. If passed as parameter, return DELETE flag. If resolved locally, delete and clear flag with `REM_DELETE()`.

### Crash: Invalid Combatant Pointer

**Symptom:** Crash during combat round with invalid combatant pointer.

**Cause:** Not caching `gCombatNext` before operations that might delete combatants.

**Diagnostic:** Check the combat loop. Verify that `gCombatNext = ch->next_fighting` appears before the hit call.

**Fix:** Cache `gCombatNext` before any combat operation that might trigger deletion. Use the cached value to iterate.

### Crash: Scheduler Invalid Character Pointer

**Symptom:** Scheduler crash with invalid character pointer.

**Cause:** Scheduler proc directly deleted character instead of returning true to signal deletion.

**Diagnostic:** Review recent proc implementations. Check for direct `delete` or `doReturn(..., true)`.

**Fix:** Replace direct delete with `return true`. Replace `doReturn(..., true)` with `doReturn(..., false)` followed by `return true`.

### Bug: Death Not Detected

**Symptom:** Code continues after `reconcileDamage()` when victim should be dead.

**Cause:** Checking for DELETE flags instead of -1 return value.

**Diagnostic:** Check how the return value is tested. `IS_SET_DELETE()` will not detect -1.

**Fix:** Compare return value against -1 directly: `if (dam == -1) return DELETE_VICT;`

### Bug: Wrong DELETE Flag Checked

**Symptom:** DELETE flag check never triggers despite death occurring.

**Cause:** Checking wrong flag. Common mistake: checking DELETE_VICT when `die()` returns DELETE_THIS.

**Diagnostic:** Check what the called function actually returns. Methods called on self return DELETE_THIS for self-deletion.

**Fix:** Check DELETE_THIS when the method was called on the dying object. Translate appropriately for your return value.

### Bug: Transformation Affect Lingers

**Symptom:** Orphaned mob has polymorph/disguise/shapeshift affect but no descriptor.

**Cause:** Transformation ended abnormally without proper cleanup.

**Diagnostic:** Check if mob has transformation affect but `desc` or `desc->original` is null.

**Fix:** The automatic cleanup in `updateAffects()` should catch this. If not, verify the affect type check includes all transformation spells.

### Bug: Flag Propagation Error

**Symptom:** DELETE_THIS not translated to DELETE_VICT when propagating.

**Cause:** Not translating flag semantics when crossing function boundaries where parameter roles change.

**Diagnostic:** Trace the function call chain. Identify where a callee's this becomes the caller's vict.

**Fix:** Add translation logic: `if (IS_SET_DELETE(rc, DELETE_THIS)) return DELETE_VICT;` when the callee's this maps to the caller's victim.
