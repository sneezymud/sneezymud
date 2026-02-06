---
title: Combat Round Timing and Structure
description: Combat system timing with distributed attack scheduling, global iterator cache for safe list traversal, and damage pipeline with DELETE flag handling
category: critical
keywords: [combat, timing, attack scheduling, damage pipeline]
primary_symbols:
  functions: [perform_violence, hit, oneHit, blowCount, reconcileDamage, applyDamage, damageEpilog, setVictFighting, stopFighting, canFight, reformGroup, IS_SET_DELETE]
  classes: [TBeing, procPerformViolence]
  enums: [Pulse::COMBAT, DELETE_THIS, DELETE_VICT, DELETE_ITEM, gCombatList, gCombatNext, AFF_ENGAGER, AFFECT_COMBAT, COMBAT_SOLO_KILL, TOG_AVENGER_CHEAT]
---

# Combat Round Timing and Structure

## Overview

How does the combat system process attacks for dozens of fighting characters without creating a synchronized damage spike every round? The answer lies in a distributed attack model that spreads attacks across time windows, combined with a global iterator cache that enables safe list modification during traversal.

Combat operates on 1.2-second rounds (12 pulses). Rather than all attacks firing simultaneously at round boundaries, attacks are distributed across the round using modulo arithmetic. A character with 3 attacks per round might attack at pulses 0, 4, and 8, while another with 2 attacks fires at pulses 0 and 6. This creates tactical space for fleeing between attacks and prevents burst damage from overwhelming targets.

The combat list presents an iterator invalidation challenge: characters may die or flee mid-traversal, corrupting local iterators. The solution is a global iterator cache that list-modification functions can update when removing the cached element. This pattern enables safe iteration even when the list changes underneath.

Damage flows through a three-function pipeline with different return value semantics. The entry point returns -1 on death (not a DELETE flag), while internal functions return actual DELETE flags. Confusing these conventions causes crashes. The memory ownership pattern requires calling group cleanup before any deletion to prevent dangling pointers.

## Patterns

### Iterator Safety

**Always use the global iterator cache for combat list traversal.** The pattern caches the next pointer at loop start, before any operations that might modify the list. Local iterators become invalid when characters are deleted or stop fighting.

**Never modify the global iterator directly.** Only list-manipulation functions should update it. Manual modification breaks iteration for other code paths.

**Cache the next pointer before any operation that might delete the current element.** The cache must happen at the start of each iteration, before calling any function that might trigger death or combat exit.

### DELETE Flag Handling

**Always use the DELETE-specific macro for flag checks.** DELETE flags use a special bit pattern (bit 29) that standard flag checks miss. The DELETE macro handles this combined bit pattern correctly.

**Always call group cleanup before any combatant deletion.** Failing to break follower relationships leaves dangling pointers in group data structures, causing crashes when other group members access freed memory.

**Never continue execution after detecting a deletion flag.** Once a DELETE flag is detected, the associated pointer may point to freed memory. Return or break immediately after deletion.

**Translate DELETE flags when role changes between caller and callee.** When a callee's "this" becomes the caller's "victim," the THIS deletion flag must become the VICT deletion flag in the return value.

### Death Detection

**Check for -1 return value from the damage entry point, not DELETE flags.** The entry function returns actual damage dealt on survival and -1 on death. DELETE flag checks fail to detect this sentinel value.

**Propagate death to callers via DELETE flags after detecting -1.** The -1 sentinel must be translated to the appropriate DELETE flag for the caller's perspective.

### Combat List Management

**Increment attacker count when adding to combat, decrement when removing.** The attacker count gates overcrowding checks. Mismatched increments cause false overcrowding or allow excessive attackers. Characters with `AFF_ENGAGER` are exempt from the attacker count.

**Check combat validity before each attack attempt.** Room changes, exhaustion, peaceful rooms, and mount failures can all invalidate combat between iterations.

**Handle fatal returns from combat validation.** Mount fall damage during combat validation can kill the rider. The validation function returns a DELETE flag in this case.

## Reference

### Timing Constants

| Unit | Pulses | Real Time |
|------|--------|-----------|
| 1 Pulse | 1 | 0.1 seconds |
| 1 Combat Round | 12 | 1.2 seconds |
| 1 Spec Proc Cycle | 36 | 3.6 seconds |

### Attack Distribution (NPC)

| getMult() | Primary | Secondary | Total |
|-----------|---------|-----------|-------|
| 1.0 | 0.60 | 0.40 | 1.0 |
| 2.0 | 1.20 | 0.80 | 2.0 |
| 5.0 | 3.00 | 2.00 | 5.0 |
| 12.0 | 7.20 | 4.80 | 12.0 (max) |

### Attack Modifiers (PC)

| Modifier | Primary | Secondary | Notes |
|----------|---------|-----------|-------|
| Berserk | +0.5 | +0.5 | Requires standing |
| Advanced Berserking | +0.5 | +0.5 | Stacks with berserk |
| Haste | +0.5 | +0.5 | Magical speed |
| Speed stat | x0.8-1.25 | x0.8-1.25 | plotStat scaling |
| Weapon specialization | +varies | +varies | Weapon type dependent |
| Blur (monk) | x2 | x2 | One round duration |
| Riposte | +1 | - | One extra attack |
| Focus Attack | +1 | - | One extra attack |

### Attack Penalties

| Condition | Effect |
|-----------|--------|
| Shield in hand | -1 attack that hand |
| Non-weapon held | -1 attack that hand |
| Mounted | x0.67 total attacks |

### Return Value Conventions

| Function | On Death | On Survival |
|----------|----------|-------------|
| `reconcileDamage()` | -1 | Damage dealt (>= 0) |
| `applyDamage()` | DELETE_VICT | TRUE |
| `damageEpilog()` | DELETE_VICT | FALSE |

### Combat-Breaking Conditions

| Condition | Calls stopFighting |
|-----------|-------------------|
| ROOM_PEACEFUL flag | Yes |
| Exhausted (move <= 0) | Yes |
| Different rooms | Yes |
| Target dead | No |
| Attacker stunned | No |
| Shape transformation (falcon wings) | No |
| Failed mount check | No |
| Overcrowding (9999 attackers) | No |

### Performance Characteristics

| Operation | Complexity | Notes |
|-----------|------------|-------|
| Combat list add | O(1) | Prepend to head |
| Combat list remove | O(n) | Linear search |
| Combat list iteration | O(n) | 12 iterations per round |
| Attack distribution | O(1) | Modulo arithmetic |
| blowCount calculation | O(1) | Float operations |

Typical combat list size: 10-50 characters, up to 200+ during large raids. Per-round cost with 50 combatants: 600 hit calls per 1.2 seconds.

### Symbol Quick Reference

| Symbol | Type | Purpose |
|--------|------|---------|
| `gCombatList` | global | Head of linked list of fighting characters |
| `gCombatNext` | global | Iterator cache for safe traversal |
| `Pulse::COMBAT` | constant | 12 pulses per combat round |
| `perform_violence()` | function | Main combat loop, runs 12 iterations |
| `hit()` | function | Distributes attacks via modulo arithmetic |
| `oneHit()` | function | Executes single attack, bypasses reconcileDamage |
| `blowCount()` | function | Calculates attacks per round (float fx, fy) |
| `reconcileDamage()` | function | Damage entry point, returns -1 on death |
| `applyDamage()` | function | Core damage logic, returns DELETE_VICT on death |
| `damageEpilog()` | function | Post-damage cleanup, loot, PK aftermath |
| `setVictFighting()` | function | Adds character to combat list (O(1)) |
| `stopFighting()` | function | Removes from list (O(n)), adjusts gCombatNext |
| `canFight()` | function | Validates combat can continue, may return DELETE_THIS |
| `reformGroup()` | function | Breaks follower relationships before deletion |

## Implementation

### Distributed Attack Scheduling

The main combat loop executes 12 iterations per call, one for each pulse in a combat round. Each iteration walks the entire combat list, checking whether each character's attack fires at that pulse. The scheduler triggers `perform_violence` every `Pulse::COMBAT` via `procPerformViolence` in process.cc.

Attack timing uses modulo arithmetic on float attack counts. For a character with 2.0 attacks per round, the system calculates a wait interval (round length divided by attack count) and checks whether the current pulse falls within the firing window. The formula `(pulse * 10) % hit_wait < 10` determines firing.

Secondary hand attacks add a phase shift of half the wait interval, staggering primary and secondary attacks to prevent synchronization.

The absolute pulse count (server uptime) feeds into the formula, adding variation so characters with identical attack counts do not fire simultaneously.

### Global Iterator Cache

The combat list is a linked list threaded through `next_fighting` pointers. When a character stops fighting (via death, flee, or room change), they must be unlinked mid-iteration.

A local iterator would crash: after `delete ch`, the expression `ch->next_fighting` accesses freed memory. The global cache `gCombatNext` solves this by storing the next pointer before any operation that might modify the list.

When `stopFighting()` removes a character, it checks whether that character equals `gCombatNext`. If so, it advances `gCombatNext` to the removed character's next pointer before unlinking. This allows the main loop to continue correctly.

List corruption (character not found during removal) triggers an abort with diagnostic logging ("Char fighting not found Error - ABORT"). This catches bugs early rather than allowing silent corruption.

### Damage Pipeline

**Entry point:** `reconcileDamage()` orchestrates damage dealing. It triggers combat-start specials, sets aggressor flags, calls resistance calculations, updates monster hatred, and delegates to core damage logic. Returns actual damage dealt or -1 on death.

**Core damage:** `applyDamage()` handles the actual HP modification. It sets combatants fighting if not already, applies protection effects, tracks quest solo-kill requirements, caps overkill damage (max damage is 11 + victim HP), awards XP and trophies, subtracts HP, generates death messages, and calls post-damage cleanup. Returns DELETE_VICT on death, TRUE otherwise.

**Cleanup:** `damageEpilog()` handles post-damage effects: removing transfix, revealing invisible attackers, mount fall-off on death, death response triggers, corpse creation with loot, and PK aftermath processing.

**Melee bypass:** The single-attack function bypasses the entry point, calling core damage directly. Combat specials and hatred are handled elsewhere for melee, so the entry point's orchestration is unnecessary.

### Attack Count Calculation

NPCs use a multiplier capped at 12, split 60/40 between primary and secondary hands for paired weapons. Single-weapon NPCs get all attacks on primary.

PCs have complex calculation: monks get barehand attacks scaled by multiplier, weapon users get attack counts from weapon properties plus specialization bonuses, all multiplied by speed stat modifier. Combat mode adds flat bonuses (berserk adds 0.5 per hand, advanced berserking adds another 0.5).

Equipment imposes penalties: shields and non-weapon items reduce attacks by 1 for that hand. Mounted characters multiply total attacks by 0.67.

The attack loop uses a 0.999 threshold (rather than 1.0) to handle floating-point precision issues when checking whether attacks remain.

### Quest Solo-Kill Tracking

Certain quest mobs require solo kills. The system applies a combat affect (`AFFECT_COMBAT` with `COMBAT_SOLO_KILL` modifier) with a pointer to the quest-holder. When someone else damages the mob, the quest fails and sets a cheat flag (`TOG_AVENGER_CHEAT`). If combat ends for any reason other than mob death (flee, teleport, interruption), the quest also fails.

The enforcement happens in both the damage pipeline (detecting outside interference) and the stop-fighting function (detecting premature combat exit).

## Troubleshooting

### Crash During Combat Iteration

**Symptom:** Server crashes mid-combat with access violation in the combat loop.

**Likely cause:** Iterator invalidation from using local iterator instead of global cache, or failure to cache next pointer before operations.

**Diagnostic approach:** Check crash location for combat list traversal. Verify the loop uses the global cache and caches at iteration start.

**Fix:** Replace local iterator pattern with global cache pattern. Ensure cache assignment precedes all operations.

### Use-After-Free on Victim

**Symptom:** Crash accessing victim data after damage application.

**Likely cause:** Checking for DELETE flag instead of -1 from the damage entry point. The flag check never triggers, execution continues, victim pointer is stale.

**Diagnostic approach:** Find damage entry point calls, verify return value check uses == -1 comparison.

**Fix:** Replace DELETE flag check with -1 comparison. Propagate death to caller via DELETE flag after detection.

### Group Pointer Corruption

**Symptom:** Crash when accessing group data after combat death. May manifest as segfault in follower iteration or group XP distribution.

**Likely cause:** Missing group cleanup call before deletion. Follower pointers still reference the freed character.

**Diagnostic approach:** Trace deletion path, verify group cleanup precedes every deletion.

**Fix:** Add group cleanup call immediately before every combatant deletion.

### Combat List Corruption Abort

**Symptom:** Server aborts with "Char fighting not found Error - ABORT" log message.

**Likely cause:** Character's combat state inconsistent with list membership. May result from double-removal or removal of character that was never properly added.

**Diagnostic approach:** Check code paths that modify combat state. Look for missing or duplicate list operations.

**Fix:** Ensure setVictFighting and stopFighting are called in matched pairs. Verify no path modifies combat state without proper list management.

### Attacks Not Firing

**Symptom:** Character in combat but not attacking. No damage output despite being in fight.

**Likely cause:** Combat validation returning false silently, or attack count calculation returning zero.

**Diagnostic approach:** Check canFight return value. Verify blowCount produces non-zero values. Check for blocking affects or position issues.

**Fix:** Address the blocking condition. Common issues: exhaustion (move <= 0), different rooms, peaceful room flag, position below standing.

### Weapon Destroyed Mid-Combat

**Symptom:** Crash accessing weapon pointer after oneHit, use-after-free on weapon object.

**Likely cause:** Not checking `IS_SET_DELETE(rc, DELETE_ITEM)` after oneHit. Weapon destroyed but still referenced.

**Fix:** After oneHit call, check for DELETE_ITEM. If true, null the weapon pointer and break the attack loop. Never assume weapon survives oneHit call.

### Attack Distribution Synchronization

**Symptom:** All characters with same attack speed fire simultaneously, creating burst damage.

**Likely cause:** Using round-relative pulse instead of absolute pulse in hit calculation, or not phase-shifting secondary hand.

**Fix:** Pass `(pulse + tmp_pulse)` from perform_violence to hit, where pulse is absolute server uptime. Verify secondary hand formula adds `(hit_wait / 2)` to pulse value.
