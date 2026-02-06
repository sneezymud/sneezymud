---
title: Memory Safety and DELETE Flags
description: Core memory management system using return flags to signal object deletion, preventing use-after-free and double-delete crashes
keywords: [ownership, flag propagation, pointer validation, deletion signal]
category: critical
primary_symbols:
  functions: [reconcileDamage, applyDamage, die, rawKill, reformGroup, doReturn, updateAffects, checkIdling, genericKillFix, stopFighting]
  classes: [TBeing, Descriptor]
  enums: [DELETE_THIS, DELETE_VICT, DELETE_ITEM, DELETE_ALREADY_DELETED, RET_STOP_PARSING, IS_SET_DELETE, ADD_DELETE, REM_DELETE]
---

# Memory Safety and DELETE Flags

## Overview

SneezyMUD's DELETE flag system is the core memory management mechanism. When functions operate on beings or objects that may die or be destroyed, they return integer flags indicating what should be deleted. The caller, who holds the pointer reference, performs the actual deletion.

This design prevents two critical memory bugs. Use-after-free occurs when code deletes an object while another function higher in the call stack still references it. Double-delete occurs when multiple callers each think they own a pointer and both try to delete it. By separating the decision to delete from the act of deletion, ownership becomes explicit.

The system has one fundamental rule: whoever resolved or found a pointer owns it and is responsible for its deletion. If a function receives a pointer as a parameter, the caller owns it. If a function looks up a pointer itself, the callee owns it.

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
| `sys/connect.cc` | parseCommand(), descriptor loop |
| `misc/offense.cc` | preKillCheck() |
| `sys/socket.cc` | proc*::run() adapter functions |

## Implementation

### Flag Bit Structure

DELETE flags use a combined bit pattern with bit 29 set as a discriminator. This distinguishes DELETE flags from damage values, since functions like `reconcileDamage()` return damage integers while combat functions return DELETE flags. The high bit ensures no overlap between valid damage amounts and DELETE flag values.

### Ownership Resolution

When a function receives a pointer as a parameter, the caller owns it. When a function obtains a pointer through lookup (via `get_char_room_vis()`, `get_obj_in_list_vis()`, or similar), the callee owns it.

Ownership determines deletion responsibility. Owners delete directly and clear the flag with `REM_DELETE()`. Non-owners return the appropriate DELETE flag and let the owner handle deletion.

The flag translation pattern handles parameter mapping. When calling a method on a victim and it returns `DELETE_THIS`, that means "delete what the method was called on" which is the victim. The caller translates this to `DELETE_VICT` for its own return value.

Local ownership creates an exception to flag propagation. If a function resolves a victim locally, receives DELETE_THIS from a method call on that victim, and then deletes the victim itself, it must clear the DELETE_THIS flag with REM_DELETE before returning to prevent the caller from seeing a deletion flag for an object the caller never knew existed.

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

### Bug: Flag Propagation Error

**Symptom:** DELETE_THIS not translated to DELETE_VICT when propagating.

**Cause:** Not translating flag semantics when crossing function boundaries where parameter roles change.

**Diagnostic:** Trace the function call chain. Identify where a callee's this becomes the caller's vict.

**Fix:** Add translation logic: `if (IS_SET_DELETE(rc, DELETE_THIS)) return DELETE_VICT;` when the callee's this maps to the caller's victim.
