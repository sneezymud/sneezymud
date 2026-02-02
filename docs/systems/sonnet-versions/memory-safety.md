---
title: Memory Safety and DELETE Flags
category: critical
keywords: [DELETE_THIS, DELETE_VICT, DELETE_ITEM, IS_SET_DELETE, ownership, polymorph, transformation, death, reformGroup, flag-propagation]
related: [combat-formulas.md, spell-skill-framework.md, spatial-relationships.md]
primary_symbols:
  functions: [die, rawKill, makeCorpse, reformGroup, doReturn, reconcileDamage, IS_SET_DELETE, REM_DELETE, ADD_DELETE]
  classes: [TBeing, TPerson, TMonster, TDescriptor]
  files: [code/code/misc/combat.cc, code/code/misc/immortal.cc, code/code/misc/periodic.cc, code/code/sys/socket.cc, code/code/misc/defs.h]
---

## Overview

What happens when a combat function kills a character while that character is still referenced by five different stack frames? How does the codebase prevent the classic use-after-free crash where one function deletes an object that another function is about to use?

SneezyMUD solves this through the DELETE flag system, a return-value-based ownership protocol that prevents use-after-free and double-delete crashes. Instead of functions directly deleting objects, they return flags indicating which objects should be deleted. The caller—whoever resolved or owns the pointer—handles the actual deletion.

This system exists because objects in the game are frequently referenced across multiple stack frames during complex operations. A single attack can trigger damage, which triggers death, which triggers transformation reversal, which triggers group leadership transfer, which triggers follower updates. Without careful coordination, any of these operations could delete an object that another operation still needs to access.

The DELETE flag system coordinates deletion across three critical subsystems: combat resolution where damage can kill combatants, transformation mechanics where player consciousness moves between bodies, and death processing which handles penalties and cleanup. Understanding this system is essential because improper flag handling causes immediate crashes in production.

### Core Mechanism

Functions that might cause object deletion return an integer containing bit flags. These flags signal which objects became invalid during the operation. The three primary flags are DELETE_THIS (the object the method was called on), DELETE_VICT (the victim/target parameter), and DELETE_ITEM (an item parameter).

The flags use a combined bit pattern that distinguishes them from damage values, since many combat functions return both damage amounts and deletion flags in the same integer. You must use the IS_SET_DELETE macro to check these flags—the standard IS_SET macro will not work correctly.

### Ownership Model

Ownership determines who performs the actual deletion. If a function receives a pointer as a parameter, the caller owns that pointer and must handle deletion—the callee returns a DELETE flag. If a function resolves a pointer locally through lookup functions, the callee owns that pointer and must delete it directly, clearing the flag with REM_DELETE to prevent double-deletion.

This ownership model prevents the most common error: a callee deletes a victim that the caller still references. Instead, the callee signals deletion intent through the flag, and the caller—who knows whether it still needs the pointer—decides when to perform the actual delete.

### Critical Integration Points

The transformation system creates the most complex ownership scenarios. When a player polymorphs, their descriptor moves to a newly created mob while their original body moves to storage. The descriptor's original field maintains the link between bodies. Death processing must detect transformed characters and reverse the transformation before applying death penalties, requiring careful flag translation as ownership transfers between the transformed mob and original body.

Death processing itself forms a pipeline through die, rawKill, and makeCorpse. Each stage returns DELETE_THIS to signal the caller should delete the character. The scheduler and command processing systems act as top-level handlers, catching these flags and performing the actual deletions after all stack frames have unwound.

Combat creates high deletion throughput with dozens of potential deaths per round. The combat loop caches iterators before dangerous operations, checks DELETE flags after every attack, calls reformGroup before any deletion to fix group pointers, then batch-deletes combatants. Missing any of these steps causes crashes.

## Patterns

### Always Use IS_SET_DELETE for Flag Checks

DELETE flags use a combined bit pattern with a high bit (1 << 29) that distinguishes them from damage values. The standard IS_SET macro cannot detect this pattern correctly.

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

Why this matters: IS_SET checks single bits. DELETE flags use multiple bits. Using the wrong macro means you will miss deletions, leading to use-after-free crashes when you continue using deleted objects.

### Never Ignore Return Values from Dangerous Functions

Combat, movement, and interaction functions frequently return DELETE flags. Ignoring these return values and continuing to use the objects causes immediate crashes.

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

Dangerous functions that commonly return DELETE flags: reconcileDamage, applyDamage, die, rawKill, doMove, crashLanding, get, drop, put, give, all trap functions, checkSpec on mobs/objects/rooms, and doReturn.

### Translate Flags Based on Parameter Mapping

When propagating DELETE flags between functions, translate the flag semantics based on how parameters map between caller and callee contexts.

```cpp
// Callee's DELETE_THIS refers to the sub parameter
int rc = ch->checkForInsideTrap(sub);

// In our context, sub is the victim, so translate THIS to VICT
if (IS_SET_DELETE(rc, DELETE_THIS)) {
  return DELETE_VICT;
}

// If we call a method on ourselves, no translation needed
int rc = this->someMethod();
if (IS_SET_DELETE(rc, DELETE_THIS)) {
  return DELETE_THIS;
}
```

The translation rule: identify what object the flag refers to in the callee's context, then determine what flag represents that object in your context. A callee's DELETE_THIS might be your DELETE_VICT if you passed that object as a victim parameter.

### Follow the Ownership Pattern for Deletion

Whoever resolved or found a pointer owns it and must handle deletion. Callers pass ownership when they pass pointers as parameters.

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

Why ownership matters: if both caller and callee try to delete the same object, you get a double-free crash. If neither deletes it, you get a memory leak. The ownership rule ensures exactly one deletion.

### Always Validate desc->original Before Dereferencing

The descriptor's original field can become null or stale during transformation operations, link death, idling timeout, or affect removal. Every access must validate the pointer first.

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

The original field becomes null when: the transformation affect expires and is removed by updateAffects, the player idles out and checkIdling reverses the transformation, the player link-dies and descriptor cleanup runs, or the transformed mob dies and rawKill reverses the transformation.

### Use doReturn with deleteMob=false in Scheduler Procs

Scheduler procs operate during the batch character update phase. Directly deleting characters breaks iterator safety. Instead, use doReturn with deleteMob=false to move the mob to storage, then return true to signal the scheduler should delete it.

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

The deleteMob parameter defaults to true in doReturn's signature, which is safe for command-driven transformations where no iteration is happening. Scheduler contexts must explicitly pass false.

### Always Call reformGroup Before Deletion

Deleting a group leader or member leaves dangling master and follower pointers throughout the group structure. Call reformGroup before any character deletion to transfer leadership and fix all group relationships.

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

reformGroup only does work if the character is a group leader (has followers but no master). It transfers leadership to the first eligible follower and reattaches all other followers to the new leader. Safe to call on any character—non-leaders return immediately.

### Check reconcileDamage Return for Death

The reconcileDamage function returns -1 on death, not a DELETE flag. This is a special case because reconcileDamage returns the damage amount as an integer, and -1 is used as a death sentinel.

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

After reconcileDamage returns -1, the victim is dead and the death processing pipeline has run. Do not access the victim pointer after seeing -1.

### Never Delete Transformed Mobs Directly

Transformed mobs have ACT_POLYSELF set and maintain bidirectional links through desc->original and desc->character. Direct deletion orphans the descriptor and original body. Always use doReturn to reverse the transformation before deletion.

```cpp
// WRONG - Orphans descriptor and original body
if (shouldDelete(mob)) {
  delete mob;  // Descriptor still references this!
}

// CORRECT - Reverse transformation first
if (shouldDelete(mob)) {
  mob->doReturn("", WEAR_NOWHERE, true, false);
  return true;  // Let scheduler delete
}
```

The doReturn function handles all cleanup: transferring equipment and stats back to the original body, moving the original from storage to the game world, swapping the descriptor back, clearing transformation flags and pointers. Only after this cleanup is the transformed mob safe to delete.

### Return DELETE_THIS from Death Functions

The die and rawKill functions return DELETE_THIS, not DELETE_VICT. This is because they are methods called on the dying character (this pointer), not functions that take a victim parameter.

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

In the caller's context, the dying character is the victim, so you often translate DELETE_THIS to DELETE_VICT when propagating upward.

### Handle Combined Flags

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

## Reference

### Primary Symbol Quick Reference

| Symbol | Type | Purpose |
|--------|------|---------|
| `DELETE_THIS` | flag constant | Signals caller should delete the object the method was called on |
| `DELETE_VICT` | flag constant | Signals caller should delete the victim/target parameter |
| `DELETE_ITEM` | flag constant | Signals caller should delete the item parameter |
| `ALREADY_DELETED` | flag constant | Signals object was already deleted, no action needed |
| `IS_SET_DELETE(value, flag)` | macro | Check if DELETE flag is set in return value |
| `ADD_DELETE(value, flag)` | macro | Add DELETE flag to return value |
| `REM_DELETE(value, flag)` | macro | Remove DELETE flag from return value after handling |
| `die(dmg_type, killer)` | method | Death entry point with penalties, returns DELETE_THIS |
| `rawKill(dmg_type, killer, exp)` | method | Core death processing without penalties, returns DELETE_THIS |
| `makeCorpse(dmg_type, killer, exp)` | method | Creates corpse and transfers equipment |
| `reformGroup()` | method | Transfers group leadership before character deletion |
| `doReturn(arg, limb, tell, deleteMob)` | method | Reverses transformation, optionally deletes mob |
| `reconcileDamage(...)` | function | Applies damage, returns -1 on death (not a flag) |
| `TBeing` | class | Base class for all characters |
| `TPerson` | class | Player character class |
| `TMonster` | class | NPC/mob class |
| `TDescriptor` | class | Network connection with original pointer for transformations |

### DELETE Flag Bit Patterns

| Flag | Bit Pattern | Purpose |
|------|-------------|---------|
| `DELETE_ITEM` | `(1 << 5) \| (1 << 29)` | Delete item parameter |
| `DELETE_THIS` | `(1 << 6) \| (1 << 29)` | Delete this object |
| `DELETE_VICT` | `(1 << 7) \| (1 << 29)` | Delete victim parameter |
| `ALREADY_DELETED` | `(1 << 8) \| (1 << 29)` | Already deleted, no action needed |
| `RET_STOP_PARSING` | `(1 << 9) \| (1 << 29)` | Stop command parsing (not deletion) |

The high bit (1 << 29) distinguishes DELETE flags from damage integers.

### Transformation Types

| Type | Enum Value | Source | Transfers Stats | Transfers Equipment |
|------|-----------|--------|-----------------|---------------------|
| None | `POLY_TYPE_NONE` | Not transformed | N/A | N/A |
| Switch | `POLY_TYPE_SWITCH` | Immortal switch command | No | No |
| Disguise | `POLY_TYPE_DISGUISE` | SKILL_DISGUISE, werewolf | Yes | Yes |
| Shapeshift | `POLY_TYPE_SHAPESHIFT` | SPELL_SHAPESHIFT | Yes | Yes |
| Polymorph | `POLY_TYPE_POLYMORPH` | SPELL_POLYMORPH | Yes | Yes |

### Death Penalty Calculations

| Penalty | Formula | Exemptions |
|---------|---------|------------|
| XP Loss | min(current_exp / 5, 25 × mob_exp(level)) | Arena deaths, free death affects |
| XP Loss (PvP) | Normal penalty / 10 | Same as above |
| Age Increase | Random 0-3 years | Level ≤10, arena deaths, free death affects |

### PC vs NPC Death Handling

| Operation | PC Death | NPC Death |
|-----------|----------|-----------|
| XP loss | Yes (via deathExp) | No |
| Age penalty | Yes (0-3 years) | No |
| Corpse type | TPCorpse (vnum -2) | TCorpse (mob vnum) |
| Corpse flags | CORPSE_NO_REGEN | None unless vnum < 0 |
| reformGroup | Yes | No |
| removeRent | Yes | N/A |
| removeFollowers | Yes | N/A |
| Permadeath log | Yes | N/A |
| Save location | Room::NOWHERE | N/A |
| Limb healing | All limbs restored | N/A |
| Disease cleanup | All diseases removed | N/A |

### Top-Level DELETE Handlers

| Handler | Location | Deletes What | How |
|---------|----------|--------------|-----|
| `TScheduler::runObj()` | process.cc | Objects | When proc::run returns true |
| `TScheduler::runChar()` | process.cc | Characters | Batch deletes from deleteMe vector |
| `perform_violence()` | combat.cc | Combatants | DELETE_THIS and DELETE_VICT checks |
| Descriptor loop | connect.cc | Characters/Descriptors | DELETE_THIS from parseCommand |
| Account menu | connect.cc | Descriptors | DELETE_THIS from doAccountMenu |

### Key Validation Scenarios

| Pointer | Must Validate | Becomes Null When |
|---------|--------------|-------------------|
| `desc` | Before any descriptor access | Link death, logout, character deletion |
| `desc->original` | Before transformation operations | Affect expiry, idle timeout, manual return, death |
| `desc->character` | Before accessing transformed body | Descriptor swap during return |
| `victim` | After reconcileDamage returns -1 | Death processing completes |
| `this` | After DELETE_THIS return | Caller deletion pending |

## Implementation

### Flag System Architecture

The DELETE flag constants use a combined bit pattern with a high discriminator bit. This allows the same integer to encode both damage values (which use low bits) and deletion flags (which use specific bit combinations including the high bit). The bit pattern prevents accidental collision between a damage value and a flag value.

Three specialized macros handle flag operations. IS_SET_DELETE checks whether a specific flag exists in a return value by testing both the flag's unique bit and the discriminator bit. ADD_DELETE combines flags using bitwise OR to signal multiple simultaneous deletions. REM_DELETE clears a flag after a function has handled the deletion locally, preventing the flag from propagating to callers who would attempt double-deletion.

The flag definitions live in defs.h while the manipulation macros live in structs.h. This separation means most game code includes only structs.h and never directly references the bit patterns.

### Ownership Resolution and Propagation

Ownership is determined at the point a pointer is obtained. Functions that accept pointer parameters do not own those pointers—the caller retains ownership because the caller performed the initial lookup. Functions that perform local lookups through get_char_room_vis, get_obj_in_list, or similar resolution functions own the returned pointers and must handle their deletion.

When a callee function causes deletion of a caller-owned pointer, it cannot perform the delete operation because the caller may hold multiple references to that pointer across different stack frames. Instead, the callee sets a DELETE flag and returns it. The caller, upon seeing the flag, knows to perform deletion at the appropriate point after it has finished all operations that reference the pointer.

Translation happens when crossing function boundaries where parameter roles change. A victim parameter in a callee's signature might represent the attacker in the caller's context, or vice versa. The DELETE_THIS flag always refers to the object the method was called on (the this pointer), while DELETE_VICT and DELETE_ITEM refer to parameters. When propagating flags upward, you must translate based on which object each flag represents.

Local ownership creates an exception to flag propagation. If a function resolves a victim locally, receives DELETE_THIS from a method call on that victim, and then deletes the victim itself, it must clear the DELETE_THIS flag with REM_DELETE before returning. This prevents the caller from seeing a deletion flag for an object the caller never knew existed.

### Transformation State Machine

Player transformation operates through descriptor swapping. The player's original TPerson body remains in the game database but moves to Room::POLY_STORAGE, a special non-geographic container. A new TMonster is created based on the transformation target mob. The descriptor that was attached to the TPerson is detached and reattached to the TMonster. The descriptor's original field maintains a back-pointer to the TPerson in storage.

The transformation state is tracked through multiple redundant indicators because different systems check different flags. The original body has its polyed field set to a polyTypeT enum value. The transformed mob has ACT_POLYSELF set in its act flags. The descriptor's original pointer being non-null indicates an active transformation. These redundant indicators allow different subsystems to detect transformation without tight coupling.

Werewolf transformation uses POLY_TYPE_DISGUISE with the TOG_TRANSFORMED_LYCANTHROPE quest bit set. This combination allows the periodic proc procCharLycanthropy to detect werewolf transformations specifically and trigger automatic reversal when the moon changes from full. The quest bit distinguishes werewolf disguise from player-activated disguise skill.

Equipment and stat transfer happens through SwitchStuff for most transformation types and DisguiseStuff for disguise-based transformations. These functions iterate through the original body's equipment, move items to the transformed mob, recalculate all affects and stats, and update derived values like AC and hitroll. The transfer is bidirectional—doReturn uses the same functions in reverse to restore equipment when transformation ends.

The doReturn function coordinates reversal. It validates that desc and desc->original both exist, preventing null pointer crashes. For ACT_POLYSELF mobs, it calls SwitchStuff or DisguiseStuff to transfer equipment back. It moves the original body from Room::POLY_STORAGE to the mob's current room location. It swaps the descriptor: sets original->desc to the descriptor, sets mob->desc to null, clears desc->original. It resets the original's polyed field to POLY_TYPE_NONE. Finally, if the deleteMob parameter is true, it deletes the transformed mob; if false, it moves the mob to storage for later batch deletion.

### Automatic Transformation Cleanup

The updateAffects function runs periodically on all characters with affects. It iterates through the affect list checking for SPELL_POLYMORPH, SKILL_DISGUISE, and SPELL_SHAPESHIFT. For each transformation affect found, it validates that desc and desc->original are both non-null. If either is null, the affect is removed, which triggers the affect removal callback that calls doReturn.

This automatic cleanup prevents orphaned transformation affects on mobs whose descriptors have been freed. Without this validation, a transformed mob could continue to exist with ACT_POLYSELF set but no way to reverse the transformation, leading to crashes when other code tries to access desc->original.

The checkIdling function handles idle timeout for transformed players. It checks desc->original->getTimer to see if the original player has been idle for 20 ticks. If so, and the character has ACT_POLYSELF set, it calls SwitchStuff to transfer equipment and stats back to the original, sets polyed to POLY_TYPE_NONE, swaps the descriptor back, and proceeds with normal idle-out processing. This ensures transformed players don't bypass idle timeout.

Shapeshift has an indoor restriction enforced in updateTickStuff. If desc->original exists and has polyed set to POLY_TYPE_SHAPESHIFT, it checks whether the character is in an indoor sector. If indoors and not immortal, it forces transformation reversal through doReturn with deleteMob=false and returns ALREADY_DELETED to signal deletion should happen at the scheduler level.

### Death Processing Pipeline

The die function serves as the entry point for all character deaths. It first checks if the dying character is a transformed player by testing whether this is a TMonster with a descriptor or isPc flag. If so, it validates desc and desc->original existence. For POLY_TYPE_SWITCH transformations, it calls doReturn without deleting the mob, then recursively calls rawKill on the transformed mob, returning DELETE_THIS. This handles immortal switch deaths specially.

After polymorph handling, die checks for AFFECT_FREE_DEATHS affects. If the affect exists and has modifier greater than zero, and the death is not in an arena room, it decrements the modifier and sets a flag to skip death penalties. This implements consumable free-death items from quests and events.

Death statistics tracking increments the character's death counter indexed by level and PC/NPC type. This data feeds achievement and permadeath systems.

Experience loss calculation happens through deathExp, which computes the minimum of current_exp / 5 or 25 times the mob experience value for the character's level. For PvP deaths (isPking returns true), this penalty is divided by 10, making PvP deaths less punishing than PvE deaths. The calculated loss is applied through gain_exp with a negative value.

Age penalty adds a random 0-3 years to the age_mod field, which affects max stats based on race aging tables. Characters level 10 or below are exempt from age penalties to protect new players from compounding penalties.

After penalties, die calls rawKill to perform core death processing and then returns DELETE_THIS to signal the caller should delete the character.

The rawKill function handles combat cleanup and corpse creation. It first performs a second polymorph check (redundant with die's check) to catch edge cases where death happens directly through rawKill without going through die. It calls stopFighting to remove the character from all combat participant lists using the global gCombatNext iterator cache.

Combat mode and affect cleanup removes berserk mode, berserk affects, and vampire bite affects that should not persist through death. Dead immortals have their PLR_IMMORTAL flag restored to ensure they respawn with immortal powers.

Shopkeeper special handling deletes all inventory and money. Shopkeepers maintain infinite virtual inventory that should not transfer to corpses. The equipment deletion uses post-increment iteration with the *(it++) pattern to safely advance the iterator before deleting the pointed-to object.

The makeCorpse call creates the corpse object, determines its type based on PC vs NPC, sets appropriate corpse flags, and transfers all equipment and money from the character to the corpse. The corpse is then placed in the room.

PC-specific cleanup runs after corpse creation. reformGroup must run before deletion to transfer group leadership—this is the critical point that prevents dangling follower pointers. removeRent wipes the rent file. removeFollowers wipes the followers file containing saved pets and charms. logPermaDeathDied updates the permadeath database table with the death event.

The preKillCheck function saves the character to database. For arena deaths, it saves to the character's current room so they respawn there. For normal deaths, it saves to Room::NOWHERE which is interpreted as the death state requiring respawn location selection.

### Corpse Creation Details

The makeCorpse function branches based on whether the character is a PC or NPC. For PCs, it calls race->makePCorpse which creates a TPCorpse with vnum -2. For NPCs, it calls race->makeCorpse which creates a TCorpse with the mob's vnum. This vnum determines corpse appearance and decay behavior.

All PC corpses receive the CORPSE_NO_REGEN flag which prevents them from being destroyed by the cleanup proc that removes old corpses. NPC corpses only get this flag if their vnum is negative.

Equipment transfer iterates through all wear slots. For each equipped item, it calls unequip to handle affect removal, then transfers the item to the corpse's inventory with the += operator. Money is transferred by setting the corpse's money value to the character's money and zeroing the character's money.

Experience loss is embedded in the corpse as a float value. This allows corpse retrieval systems to restore some or all of the lost experience when a player recovers their corpse.

The corpse's short description, long description, and name are generated from templates that include the character's name. For PCs, this is the character's actual name. For NPCs, it uses the mob's name list. The get and sacrifice commands parse these descriptions to allow "get all from corpse" and "sacrifice corpse" syntax.

### Combat Loop DELETE Handling

The perform_violence function implements the core combat round loop. It iterates through the combat participant list maintained by the scheduler. Before processing each combatant, it caches gCombatNext = ch->next_fighting to ensure safe iteration if ch is deleted.

For each combatant, it calls ch->hit which performs attack rolls, damage calculation, and potentially calls reconcileDamage which triggers death. The hit function returns a combined flag value indicating which participants died.

After hit returns, perform_violence checks IS_SET_DELETE(rc, DELETE_VICT) first. If set, it calls vict->reformGroup, deletes vict, nulls the pointer, and continues to the next iteration. This handles the victim dying.

If DELETE_VICT was not set, it checks IS_SET_DELETE(rc, DELETE_THIS). If set, it calls ch->reformGroup, deletes ch, nulls the pointer, and breaks from the loop since the primary iterator (ch) is now invalid. This handles the attacker dying.

The order matters: checking DELETE_VICT first allows the loop to continue processing other combatants. Checking DELETE_THIS second ensures we break immediately when the primary iterator is invalidated.

Combined flags (DELETE_THIS | DELETE_VICT) work correctly because the if statements are sequential, not else-if. Both blocks execute, handling both deletions.

### Scheduler Proc Adapter Layer

Scheduler procs implement a run method that returns bool, not int. This creates an adapter layer between the flag-based deletion system and the scheduler's batch deletion system.

The proc run methods call game functions that return DELETE flags. The proc must convert these flags to bool: return true if deletion should happen, false otherwise. The conversion is done by checking IS_SET_DELETE(rc, DELETE_THIS) and returning true if detected.

When a proc returns true, the scheduler adds the character to the deleteMe vector but does not delete immediately. After all procs have run, the scheduler batch-deletes everything in deleteMe. This prevents iterator invalidation during proc execution.

For transformation reversal in procs like procCharLycanthropy, the proc must call doReturn with deleteMob=false to move the transformed mob to storage, then return true to signal the scheduler should delete the mob. Direct deletion in the proc would leave the scheduler's iterator pointing at freed memory.

The procCharAffects proc handles affect expiration and calls updateAffects. If updateAffects returns ALREADY_DELETED, the proc returns true immediately. This handles transformation reversal triggered by affect expiration.

Special cases like procCharDrowning and procCharFalling call death functions (die, rawKill) which return DELETE_THIS. These procs check for DELETE_THIS and return true to signal scheduler deletion.

### Iterator Safety in Linked Lists

The combat participant list, river flow list, and rider chains are all intrusive linked lists where deletion during iteration requires careful iterator management.

The pattern is to cache the next pointer before any operation that might delete the current node. For combat, this is gCombatNext = ch->next_fighting. For river flow, it's next_in_list = obj->nextInList. For riders, it's next_rider = rider->riding.

After caching the next pointer, the code performs the dangerous operation (attack, flow, fall) and checks for DELETE flags. If deletion is signaled, the object is deleted and the iterator is advanced using the cached next pointer. If no deletion, the iterator advances normally to next.

The equipment list (stuff) uses STL iterators but requires the same pattern. When deleting during iteration, use *(it++) which post-increments the iterator before dereferencing, ensuring the iterator points to the next element before the current element is deleted from the list.

This pattern appears throughout: cache next before deletion, use cached value for iteration, never dereference current after deletion.

## Troubleshooting

### Symptom: Crash in IS_SET with DELETE flag parameter

Likely cause: Using IS_SET instead of IS_SET_DELETE to check flag values.

Diagnostic approach: Search the code for IS_SET calls that check DELETE_THIS, DELETE_VICT, or DELETE_ITEM. These will be in recent changes or legacy code not yet modernized.

Fix: Replace IS_SET with IS_SET_DELETE for all DELETE flag checks. The macro handles the combined bit pattern correctly.

### Symptom: Use-after-free crash accessing victim/character pointer

Likely cause: Ignoring DELETE flag returns from combat/movement/interaction functions.

Diagnostic approach: Trace backwards from the crash point to find the last function call that could have deleted the object. Check whether the return value was tested for DELETE flags.

Fix: Add IS_SET_DELETE checks immediately after dangerous function calls. If DELETE flag is detected, return or break immediately without accessing the pointer.

### Symptom: Double-free crash during character deletion

Likely cause: Both caller and callee deleting the same object, or not using REM_DELETE after local deletion.

Diagnostic approach: Examine ownership—did the function receive the pointer as a parameter or resolve it locally? If received as parameter, it should return a flag, not delete. If resolved locally, it should delete and REM_DELETE.

Fix: Apply the ownership pattern. Parameter-received pointers signal deletion via flags. Locally-resolved pointers delete directly and clear the flag.

### Symptom: Dangling pointer crash in group/follower iteration

Likely cause: Deleting a group leader or member without calling reformGroup first.

Diagnostic approach: Find the deletion point in the stack trace. Check whether reformGroup was called before the delete statement.

Fix: Always call reformGroup before deleting any character. It is safe to call on non-leaders (returns immediately) so call it unconditionally before all character deletions.

### Symptom: Heap-use-after-free in descriptor access after transformation

Likely cause: Accessing desc->original without validation, or calling doReturn with deleteMob=true in a scheduler proc.

Diagnostic approach: Check whether the crash happens in a periodic proc (updateAffects, checkIdling, periodic tasks). If so, check the doReturn call—if deleteMob is true or defaulted, that is the problem. If the crash is in normal code, check whether desc->original is validated before access.

Fix: In scheduler procs, use doReturn with deleteMob=false. In normal code, add if (!desc || !desc->original) validation before every desc->original access.

### Symptom: reconcileDamage returns -1 but victim still accessed

Likely cause: Not checking the -1 death sentinel from reconcileDamage.

Diagnostic approach: Find the reconcileDamage call. Check whether the return value is tested against -1 before any subsequent victim access.

Fix: Immediately after reconcileDamage, add if (dam == -1) return DELETE_VICT; to stop execution when death occurs.

### Symptom: Crash during combat round with invalid combatant pointer

Likely cause: Not caching gCombatNext before operations that might delete combatants.

Diagnostic approach: Check the combat loop. Verify that gCombatNext = ch->next_fighting appears before the hit call and that the loop uses gCombatNext for iteration.

Fix: Cache gCombatNext before any combat operation that might trigger deletion. Use the cached value to iterate to the next combatant.

### Symptom: Transformed mob with ACT_POLYSELF persists after descriptor freed

Likely cause: desc or desc->original became null but updateAffects did not remove the transformation affect.

Diagnostic approach: Check whether the transformation affect (SPELL_POLYMORPH, SKILL_DISGUISE, SPELL_SHAPESHIFT) appears in the mob's affect list. Check whether desc or desc->original is null.

Fix: Ensure updateAffects is running on the mob (it should run automatically each pulse). If not running, investigate why the mob is excluded from periodic processing. The affect should auto-remove when desc/desc->original becomes null.

### Symptom: Scheduler crash with invalid character pointer

Likely cause: Scheduler proc directly deleted character instead of returning true to signal deletion.

Diagnostic approach: Review recent proc implementations. Check whether any proc calls delete or doReturn with deleteMob=true.

Fix: Replace direct delete with return true. Replace doReturn(..., true) with doReturn(..., false) followed by return true.

### Symptom: Flag propagation error where DELETE_THIS not translated to DELETE_VICT

Likely cause: Not translating flag semantics when propagating between function contexts.

Diagnostic approach: Trace the function call chain. Identify where parameters change roles—where a callee's this becomes the caller's vict.

Fix: Add translation logic: if (IS_SET_DELETE(rc, DELETE_THIS)) return DELETE_VICT; when the callee's this maps to the caller's victim.
