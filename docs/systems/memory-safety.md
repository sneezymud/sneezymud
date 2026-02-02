---
title: Memory Safety and DELETE Flags
description: Core memory management system using return flags to signal object deletion, preventing use-after-free and double-delete crashes
keywords: [DELETE_THIS, DELETE_VICT, DELETE_ITEM, IS_SET_DELETE, REM_DELETE, ownership, flag propagation, polymorph, death processing]
category: Critical Systems
related: [combat-rounds.md, scheduler-pulses.md, command-implementation.md]
created_by_model: opus
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

Always check return values from functions that can trigger death or destruction. Common triggers: `reconcileDamage()`, `doMove()`, `crashLanding()`, trap functions, spec procs.

Always check immediately after the function call. Never access the pointer again until the check completes.

### Ownership

Return the DELETE flag when the caller passed the pointer as a parameter. Translate the flag appropriately: if callee's method was called on the victim, callee's `DELETE_THIS` becomes caller's `DELETE_VICT`.

Delete directly and clear the flag with `REM_DELETE()` when you resolved the pointer yourself via lookup functions like `get_char_room_vis()`.

Never delete objects you did not resolve. If someone else gave you a pointer, return a flag and let them handle it.

### Combat Cleanup

Always call `reformGroup()` before deleting any character. Failing to do so leaves followers with dangling master pointers.

Always use the global iterator cache `gCombatNext` when traversing combat lists. Local iterators become invalid when combatants die.

Never continue execution after detecting a DELETE flag. Check immediately and return or break from the current scope.

### Transformation Safety

Always validate `desc` and `desc->original` before dereferencing. Either pointer can be null during transformation edge cases.

Use `doReturn()` with `deleteMob=false` in scheduler procs, then return `true` to let the scheduler handle deletion. Direct deletion causes use-after-free.

Never access the mob after calling `doReturn()` with `deleteMob=true`. The mob is freed immediately.

Always clear `ACT_POLYSELF` before deletion to prevent cleanup issues.

### Death Processing

Check for -1 return from `reconcileDamage()` to detect death. It returns damage dealt on survival, -1 on death. It does not return DELETE flags.

Never check `reconcileDamage()` with `IS_SET_DELETE()`. The -1 return is not a flag.

Remember that `die()` returns `DELETE_THIS`, not `DELETE_VICT`. The dying being signals its own deletion.

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

### Utility Macros

| Macro | Purpose |
|-------|---------|
| `IS_SET_DELETE(value, flag)` | Check if DELETE flag is set |
| `ADD_DELETE(value, flag)` | Add a DELETE flag to return value |
| `REM_DELETE(value, flag)` | Remove a DELETE flag from return value |

### Transformation Types

| Type | Source | Transfers Stats | Transfers Equipment |
|------|--------|-----------------|---------------------|
| `POLY_TYPE_SWITCH` | Immortal switch command | No | No |
| `POLY_TYPE_DISGUISE` | Thief disguise skill | Yes | Yes |
| `POLY_TYPE_SHAPESHIFT` | Shaman shapeshift spell | Yes | Yes |
| `POLY_TYPE_POLYMORPH` | Mage polymorph spell | Yes | Yes |

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
| Group handling | reformGroup() | None |
| Rent cleanup | removeRent() | N/A |
| Follower cleanup | removeFollowers() | N/A |
| Permadeath logging | Yes | No |

### Top-Level DELETE Handlers

| Location | Handles |
|----------|---------|
| `TScheduler::runObj()` | Object proc returns |
| `TScheduler::runChar()` | Character proc returns (batch deletion) |
| `perform_violence()` | Combat DELETE_THIS and DELETE_VICT |
| Descriptor loop | parseCommand DELETE_THIS |

### Key Files

| File | Content |
|------|---------|
| `misc/defs.h` | DELETE flag constants |
| `misc/structs.h` | IS_SET_DELETE, ADD_DELETE, REM_DELETE |
| `misc/combat.cc` | die(), rawKill(), reformGroup(), genericKillFix() |
| `misc/damage.cc` | reconcileDamage(), applyDamage() |
| `misc/immortal.cc` | doReturn() |
| `misc/periodic.cc` | updateAffects(), transformation removal |
| `misc/limits.cc` | checkIdling() transformation handling |
| `sys/process.cc` | TScheduler deletion handlers |
| `sys/connect.cc` | parseCommand(), descriptor loop |
| `sys/socket.cc` | proc*::run() adapter functions |

## Implementation

### Flag Bit Structure

DELETE flags use a combined bit pattern with bit 29 set as a discriminator. This distinguishes DELETE flags from damage values, since functions like `reconcileDamage()` return damage integers while combat functions return DELETE flags. The high bit ensures no overlap between valid damage amounts and DELETE flag values.

### Ownership Resolution

When a function receives a pointer as a parameter, the caller owns it. When a function obtains a pointer through lookup (via `get_char_room_vis()`, `get_obj_in_list_vis()`, or similar), the callee owns it.

Ownership determines deletion responsibility. Owners delete directly and clear the flag with `REM_DELETE()`. Non-owners return the appropriate DELETE flag and let the owner handle deletion.

The flag translation pattern handles parameter mapping. When calling a method on a victim and it returns `DELETE_THIS`, that means "delete what the method was called on" which is the victim. The caller translates this to `DELETE_VICT` for its own return value.

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

The function validates `desc` and `desc->original` exist, transfers equipment and stats back via `SwitchStuff()`, moves the original body back from storage, swaps the descriptor, clears transformation state, and either deletes or stores the mob.

### Automatic Transformation Affect Removal

The `updateAffects()` function in periodic.cc removes transformation affects when the descriptor relationship becomes invalid. If `desc` or `desc->original` is null for a polymorph, disguise, or shapeshift affect, the affect is removed automatically. This protects against orphaned transformation affects on mobs whose players disconnected.

### Shapeshift Indoor Restriction

Shapeshift transformations cannot survive indoors. The `updateTickStuff()` function checks if a shapeshifted character is in a non-outdoor room and forcibly ends the transformation with a message about needing nature connection.

### Linkdeath During Transformation

When a transformed player loses connection, the mob loses its descriptor but the `orig` field preserves the pointer to the original body. On reconnection, `connect.cc` restores `desc->original` from the `orig` field. The original body is not considered linkdead because `polyed != POLY_TYPE_NONE`.

### Idling Timeout for Transformed Characters

The `checkIdling()` function handles idle timeout specially for transformed characters. It checks `desc->original` timer, transfers back via `SwitchStuff()`, resets `polyed` to `POLY_TYPE_NONE`, and swaps the descriptor before idling out.

### Death Flow Pipeline

Every death flows through: `die()` -> `rawKill()` -> `makeCorpse()`.

`die()` handles penalties (XP loss, age increase) and calls `rawKill()`. It checks for polymorph/switch state and returns to original before death. It checks `AFFECT_FREE_DEATHS` and arena room flags to skip penalties.

`rawKill()` handles combat cleanup (stopping fights, removing berserk), creates the corpse, calls death cry, runs generic cleanup, and handles PC-specific operations (reformGroup, removeRent, removeFollowers, permadeath logging).

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

### Scheduler Proc Adapter Pattern

All `proc*::run()` functions convert DELETE flags to bool returns. They check `IS_SET_DELETE(rc, DELETE_THIS)` and return true to signal deletion, false to keep. This adapter layer lets the scheduler handle deletion uniformly without knowing the specific DELETE flag semantics of each operation.

### Architecture Overview

The deletion hierarchy flows from top-level handlers down through adapters to the actual operations:

`gameLoop()` runs the scheduler which invokes `runObj()` and `runChar()`. These call `proc*::run()` adapters which call actual game functions. Game functions return DELETE flags which adapters convert to bool. The scheduler collects true returns and batch-deletes.

`perform_violence()` is a separate top-level handler for combat, directly handling DELETE_THIS and DELETE_VICT from hit resolution.

The descriptor loop in `connect.cc` handles DELETE_THIS from `parseCommand()` for command-triggered deaths.

## Troubleshooting

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

### Bug: IS_SET Used Instead of IS_SET_DELETE

**Symptom:** DELETE flag checks never trigger.

**Cause:** Using `IS_SET()` which does not handle the combined bit pattern.

**Diagnostic:** Search for `IS_SET(` with DELETE flag arguments.

**Fix:** Replace with `IS_SET_DELETE()`.

### Bug: Double-Delete

**Symptom:** Crash on double-free or invalid pointer.

**Cause:** Multiple functions each thought they owned the pointer and deleted it.

**Diagnostic:** Trace the pointer's origin. Was it passed as a parameter or resolved locally?

**Fix:** Follow ownership rules. If passed as parameter, return DELETE flag. If resolved locally, delete and clear flag with `REM_DELETE()`.

### Bug: Transformation Affect Lingers

**Symptom:** Orphaned mob has polymorph/disguise/shapeshift affect but no descriptor.

**Cause:** Transformation ended abnormally without proper cleanup.

**Diagnostic:** Check if mob has transformation affect but `desc` or `desc->original` is null.

**Fix:** The automatic cleanup in `updateAffects()` should catch this. If not, verify the affect type check includes all transformation spells.
