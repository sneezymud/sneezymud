---
title: Combat Round Timing and Structure
category: critical
keywords: [perform_violence, gCombatList, gCombatNext, reconcileDamage, DELETE_THIS, DELETE_VICT, attack_distribution, combat_timing]
related: [combat-formulas.md, position-stance.md, memory-safety.md]
primary_symbols:
  functions: [perform_violence, hit, oneHit, reconcileDamage, applyDamage, setVictFighting, stopFighting, blowCount, canFight]
  classes: [TBeing]
  files: [code/code/misc/combat.cc, code/code/misc/offense.cc, code/code/misc/damage.cc]
---

## Overview

Combat operates on a distributed attack model where attacks are spread across 1.2-second rounds rather than firing simultaneously. The perform_violence function orchestrates all combat by iterating through a global combat list (gCombatList) and executing attacks at calculated intervals using modulo arithmetic. This creates tactical gameplay where players can flee between attacks and prevents simultaneous burst damage.

The system uses a global iterator cache (gCombatNext) to safely traverse the combat list during operations that might delete combatants. DELETE flags propagate through the call chain to signal when characters should be deleted by their caller, with reformGroup required before any deletion to prevent dangling group pointers.

Combat rounds last 12 pulses (1.2 seconds). perform_violence executes all attacks for a round in a single call by iterating 12 times through the combat list, distributing attacks based on character speed and weapon modifiers. The reconcileDamage function returns -1 on death (not DELETE_VICT), which must be checked with == -1 rather than IS_SET_DELETE.

Critical safety requirements: always cache gCombatNext before operations that modify the combat list, call reformGroup before deleting any combatant, check canFight return values (can return DELETE_THIS from mount failures), translate the -1 sentinel from reconcileDamage to DELETE_VICT for callers, and never continue execution after DELETE_THIS is detected.

## Patterns

### Safe Combat List Iteration

Always use the gCombatNext global iterator cache when traversing gCombatList during operations that might delete or remove combatants. Cache the next pointer before performing any operations, as deletion or stopFighting invalidates local iterators.

The pattern requires caching gCombatNext at the start of each loop iteration, checking DELETE flags immediately after operations, calling reformGroup before deletion, and breaking the loop if DELETE_THIS is detected. stopFighting automatically adjusts gCombatNext if removing the cached pointer to prevent iteration over freed memory.

### Attack Distribution Calculation

Calculate attacks per round using blowCount to get floating-point values for primary and secondary hands. Distribute these attacks across the 12-pulse round using modulo arithmetic with the formula: hit_wait = (len_rnd * 10) / fx, then check if ((pulse * 10) % hit_wait) < 10 to determine when attacks fire.

Secondary hand attacks add (hit_wait / 2) to the pulse value, staggering attacks to prevent simultaneous firing when primary and secondary would otherwise sync. This creates pulse-dependent timing that varies with server uptime, supporting fractional attack values like 2.5 or 3.33.

### Damage Processing Pipeline

All damage flows through reconcileDamage (entry point) to applyDamage (core processing) to damageEpilog (post-damage effects). reconcileDamage triggers combat specials, applies resistances via getActualDamage, logs damage, and returns -1 on death or the damage dealt on survival.

applyDamage sets combatants fighting, applies protection, caps overkill damage, awards XP, calls doDamage to subtract HP, calls tellStatus for death messages, and returns DELETE_VICT on death. Always check reconcileDamage return with == -1, then translate to DELETE_VICT for propagation to caller.

### DELETE Flag Propagation

Combat functions return DELETE flags to signal object deletion to their caller. The owner of a pointer (whoever resolved or received it) is responsible for deletion. Check IS_SET_DELETE(rc, DELETE_VICT) or IS_SET_DELETE(rc, DELETE_THIS) immediately after operations, call reformGroup before deleting, and return the flag to caller or delete directly based on ownership.

When callers pass victim parameters, return DELETE_VICT so caller can delete. When callees resolve victims themselves, delete directly and clear the flag with REM_DELETE. Never use IS_SET for DELETE flags - always use IS_SET_DELETE which checks the combined bit pattern including bit 29.

### Quest Solo-Kill Enforcement

Quest mobs requiring solo kills use AFFECT_COMBAT with COMBAT_SOLO_KILL modifier. The affect stores a pointer to the quest-holder who should get credit. When anyone other than the quest-holder deals damage, set TOG_AVENGER_CHEAT and remove the affect to fail the quest.

stopFighting checks for this affect and removes it if combat ends for any reason other than death, sending "Your quest progress has been interrupted!" This fails the quest on flee, teleport, summon, player death, or assistance from other players.

### Combat List Management

Add characters to combat using setVictFighting, which increments the defender's attackers count, inserts at the list head for O(1) operation, wakes sleeping victims if damage didn't kill, sets the fighting relationship, and triggers combat music. The O(1) insertion maintains performance even with large combat lists.

Remove characters using stopFighting, which adjusts gCombatNext if removing the cached pointer, searches the list to unlink the node (O(n) operation), removes solo-kill affects if present, decrements defender's attackers count, clears combat flags including AFF_AGGRESSOR and AFF_RIPOSTE, and calls updatePos to adjust position based on HP. The function fatally aborts if the character is not found in the list, detecting list corruption.

### Combat Interrupt Handling

canFight validates conditions before allowing combat to proceed, checking peaceful rooms, exhaustion, same room requirement, overcrowding (MAX_COMBAT_ATTACKERS), target death, attacker stunned status, shape transformation breaks, and mount stability. Return FALSE blocks combat, return DELETE_THIS signals rider death from fall.

Always check the return value from canFight before proceeding with attacks. When IS_SET_DELETE(rc, DELETE_THIS) is true, propagate to caller immediately. When rc is FALSE, stop combat without deletion. Conditions like peaceful rooms and exhaustion automatically call stopFighting.

### Multi-Attack Calculation

For NPCs, blowCount uses getMult capped at 12.0 attacks, splitting paired weapons as 60% primary and 40% secondary. For PCs, blowCount calculates base monk barehand attacks, adds weapon blowCountSplitter results, adds specialization bonuses, multiplies by speed stat (SPE) modifier, and adds combat mode modifiers like berserk (+0.5 per hand) and advanced berserking (additional +0.5 per hand).

Equipment penalties include shields or non-weapon items held reducing attacks by 1 per hand, and mounted combat reducing all attacks by one-third (multiply by 0.67). Temporary modifiers like haste (+0.5), blur (x2 for one round), riposte (+1 primary), and focus attack (+1 primary) stack with base calculations.

## Reference

### Global Combat Variables

gCombatList is the head pointer of the linked list containing all fighting characters. gCombatNext is the global iterator cache used during perform_violence iteration to safely handle mid-iteration deletions and removals. Both declared in combat.cc.

### Pulse Constants (comm.h)

Pulse::ONE_SECOND equals 10 pulses (1.0 seconds). Pulse::COMBAT equals 12 pulses (1.2 seconds per round). Pulse::SPEC_PROCS equals 36 pulses (3.6 seconds). Combat rounds process all attacks in a single perform_violence call that iterates 12 times.

### Attack Distribution Timing

Primary hand formula: hit_wait = (12 * 10) / fx, fires when ((pulse * 10) % hit_wait) < 10. Secondary hand formula: same but adds (hit_wait / 2) to pulse value for phase-shifting. Examples: 2.0 attacks fires at pulses 0 and 6, 3.5 attacks fires at approximately pulses 0, 3, 6, and 10.

### Combat List Operations

setVictFighting adds combatant to list head (O(1)), increments defender's attackers count, wakes sleeping victim if alive, sets fighting relationship, triggers combat music. stopFighting removes combatant with O(n) search, adjusts gCombatNext if needed, decrements attackers count, clears combat flags, calls updatePos, stops music. Fatal abort if character not found in list.

### Damage Return Values

reconcileDamage returns -1 on victim death, otherwise returns damage dealt (>= 0). applyDamage returns DELETE_VICT on victim death, TRUE on survival. damageEpilog returns DELETE_VICT on victim death, FALSE on survival. The -1 sentinel from reconcileDamage must be checked with == -1, not IS_SET_DELETE.

### Combat Validation

canFight checks peaceful rooms (blocks all combat), exhaustion (move <= 0), same room requirement, overcrowding (9999 attackers), target dead, attacker stunned, shape transformation breaks (falcon wings), mount stability (can return DELETE_THIS from fall). Return TRUE allows combat, FALSE blocks combat, DELETE_THIS signals rider death.

### Solo-Kill Quest Tracking

AFFECT_COMBAT with COMBAT_SOLO_KILL modifier tracks quest mobs requiring solo kills. The affect's be pointer references the quest-holder. Quest fails if anyone else deals damage (sets TOG_AVENGER_CHEAT) or if combat ends before mob death (flee, teleport, death, assistance). Specific quest mobs: TROLL_GIANT, CAPTAIN_RYOKEN, TREE_SPIRIT, JOHN_RUSTLER, ORC_MAGI, CLERIC_VOLCANO, CLERIC_ARDEN.

### Attack Modifiers

Berserk mode adds 0.5 attacks per hand when standing. Advanced berserking adds another 0.5 per hand (stacks). Haste spell adds 0.5 per hand. Speed stat (SPE) multiplies attacks by plotStat(0.8, 1.25). Weapon specialization adds variable bonus. Blur doubles attacks for one round. Riposte and focus attack each add 1 primary attack (removed after use). Mounted reduces all attacks to 67%.

### Performance Characteristics

Combat list add is O(1) prepend to head. Combat list remove is O(n) linear search. Combat list iteration is O(n) with 12 iterations per round. Attack distribution is O(1) modulo arithmetic. blowCount calculation is O(1) float operations. Typical combat list size: 10-50 characters, up to 200+ during large raids. Per-round cost with 50 combatants: 600 hit calls per 1.2 seconds (500 calls/second).

## Implementation

### perform_violence Execution Flow

The scheduler triggers perform_violence every Pulse::COMBAT via procPerformViolence in process.cc, passing the absolute pulse number. The function executes a nested loop structure: outer tmp_pulse loop iterates 0-11, inner combatant loop processes each character in gCombatList.

For each combatant, cache gCombatNext from next_fighting before any operations. Retrieve the victim from fight(), call stopFighting and continue if no victim. Validate combatant has roomp and is not fighting itself, call stopFighting and continue if invalid. Check combatant is awake and in same room as victim before executing hit.

Call hit with victim and (pulse + tmp_pulse) offset. Check IS_SET_DELETE(rc, DELETE_VICT) and handle victim deletion by calling reformGroup, deleting, nulling pointer, and continuing to next combatant. Check IS_SET_DELETE(rc, DELETE_THIS) and handle attacker deletion by calling reformGroup, deleting, nulling pointer, and breaking inner loop since attacker is gone.

The tmp_pulse offset (0-11) feeds into attack distribution calculations in hit, creating pulse-dependent timing that varies with server uptime rather than just round-relative position. This prevents attack synchronization between characters with identical attack speeds.

### hit Attack Distribution Logic

Retrieve primary and secondary weapons. Call blowCount with FALSE to get fx and fy attack counts. Calculate hit_wait for primary as (len_rnd * 10) / fx. Set fx to 1.0 if ((pulse * 10 % hit_wait) < 10), otherwise 0.0. Repeat for secondary with phase-shift: ((((pulse * 10) + (hit_wait / 2)) % hit_wait) < 10).

While fx > 0.999, call oneHit with HAND_PRIMARY, primary weapon, attack modifiers, and fx reference. Check IS_SET_DELETE for DELETE_THIS, DELETE_VICT, DELETE_ITEM. Decrement fx after each attack. Break if any deletion detected. Repeat process for secondary hand with fy.

The while loops handle fractional attacks by decrementing fx/fy after each oneHit call, allowing multiple attacks in a single pulse when attack speed is very high. The 0.999 threshold handles floating-point precision issues.

### blowCount Attack Calculation

For NPCs, retrieve getMult and cap at 12.0. If isPaired, divide by 2.0 and set fx to 0.60 * num, fy to 0.40 * num. Otherwise set fx to num and fy to 0.0.

For PCs, retrieve base monk multiplier from getMult. For primary hand, check for barehand monk (add 0.60 * num) or weapon (call blowCountSplitter, add specializationCheck, multiply by getStatMod STAT_SPE if result > 0.0), otherwise default 1.0. For secondary hand, similar logic with 0.40 * num for monk barehand.

Apply combat mode modifiers if isCombatMode ATTACK_BERSERK and position >= POSITION_STANDING, adding 0.5 to each hand with attacks. If doesKnowSkill SKILL_ADVANCED_BERSERKING, add another 0.5 per hand. Apply equipment penalties: shields or non-weapons held reduce attacks by 1 per hand. Apply mounted penalty: multiply all attacks by 0.67.

### reconcileDamage Entry Point

Trigger CMD_MOB_COMBAT_ONATTACK special on attacker and CMD_MOB_COMBAT_ONATTACKED on victim. Set aggressor flags for PvP tracking. Call getActualDamage to apply resistances and modifiers to base damage. Make flying mobs take flight if appropriate. Log damage to statistics system.

Develop monster hatred toward attacker. Call applyDamage with modified damage value. Check return value: if DELETE_VICT, return -1 to signal death. Otherwise return the damage value dealt. The -1 sentinel is chosen because damage amounts are always non-negative, making -1 unambiguous.

### applyDamage Core Processing

Call setVictFighting for both attacker and victim if not already fighting. Apply protection like sanctuary that reduces damage. Check for quest solo-kill affects: if AFFECT_COMBAT with COMBAT_SOLO_KILL exists and attacker is not the quest-holder, set TOG_AVENGER_CHEAT on quest-holder and remove affect.

Cap overkill damage to (11 + victim HP) to prevent massive negative HP values. Award trophy and XP if victim dies. Call doDamage to subtract HP from victim. Call tellStatus for death messages and crash landing handling. Call damageEpilog for post-damage effects.

Check victim getPosition for POSITION_DEAD. If dead, return DELETE_VICT. Otherwise return TRUE. The DELETE_VICT flag uses bit pattern (flag | (1 << 29)) to avoid collision with TRUE/FALSE values.

### damageEpilog Cleanup

Remove transfix affects from combatants. Reveal invisible attackers. Handle mount fall-off if rider takes damage. Trigger CMD_RESP_KILLED special procedure. Create mob loot on death. Handle PK aftermath including corpse marking and reputation changes.

Return DELETE_VICT if victim died, FALSE otherwise. The function separates post-damage side effects from core damage application, allowing reuse across different damage sources.

### setVictFighting Addition

Validate tbv pointer is not null. Check if attacker already fighting someone else, log error and return if true. Increment defender's attackers count unless attacker has AFF_ENGAGER. Add attacker to combat list by setting next_fighting to gCombatList and setting gCombatList to this.

Wake sleeping victim if getPosition is POSITION_SLEEPING and getHit is greater than damage (victim survived). Set specials.fighting to tbv to establish fighting relationship. Call sendPositionGmcp to notify MUD client. Call playmusic with random combat track.

The O(1) insertion at list head avoids traversal cost. The attackers count enables overcrowding checks in canFight. AFF_ENGAGER exempts characters from incrementing attackers, used for certain skills.

### stopFighting Removal

Validate fight() returns non-null victim. If gCombatNext equals this, set gCombatNext to next_fighting to adjust global iterator before removal. Remove from combat list: if gCombatList equals this, set gCombatList to next_fighting. Otherwise traverse list searching for node where next_fighting equals this, abort if not found (fatal list corruption), set previous node's next_fighting to this next_fighting.

Iterate affected list checking for AFFECT_COMBAT with COMBAT_SOLO_KILL. If found and character is awake, log removal, send "Your quest progress has been interrupted!", call affectRemove. Decrement fight()'s attackers count unless isAffected AFF_ENGAGER.

Remove combat flags: AFF_AGGRESSOR, AFF_ENGAGER, AFF_RIPOSTE, AFF_FOCUS_ATTACK. Set next_fighting and specials.fighting to NULL. Call sendPositionGmcp. Call updatePos to adjust position based on HP. Call stopmusic.

The abort on not-found prevents silent list corruption from propagating. The solo-kill affect removal enforces quest solo requirements. The attackers decrement balances the increment from setVictFighting.

### canFight Validation Flow

Check roomp isRoomFlag ROOM_PEACEFUL, send message if not silent, call stopFighting, return FALSE. Check tooTired, send panic message, call stopFighting, return FALSE. Check not sameRoom with target, log error, call stopFighting if fighting, return FALSE.

Check target attackers >= MAX_COMBAT_ATTACKERS and specials.fighting != target, send "no room" message, return FALSE. Check this attackers >= MAX_COMBAT_ATTACKERS and target fight() != this and specials.fighting != target, send "too many in way" message, return FALSE.

Check target getPosition == POSITION_DEAD, return FALSE. Check this getPosition <= POSITION_STUNNED, return FALSE. Check affectedBySpell SPELL_FALCON_WINGS, send transformation message, call affectFrom, return FALSE.

Check riding pointer, dynamic_cast to TBeing, call rideCheck with -5 twice. If both fail, call fallOffMount with POSITION_SITTING. Check IS_SET_DELETE(rc, DELETE_THIS), return DELETE_THIS if true, otherwise return FALSE. If all checks pass, return TRUE.

The rideCheck double-call with -5 penalty represents difficulty of fighting while mounted. The DELETE_THIS return from fallOffMount indicates rider died from crash landing, requiring immediate propagation to caller.

## Troubleshooting

### Iterator Invalidation Crashes

Symptom: segfault during perform_violence, gdb shows dereferencing freed memory in combat loop. Cause: using ch->next_fighting local iterator instead of gCombatNext global cache, or modifying gCombatNext manually during iteration.

Fix: always use pattern "for (ch = gCombatList; ch; ch = gCombatNext)" with "gCombatNext = ch->next_fighting" at loop start. Never assign to gCombatNext except for this caching. Let stopFighting handle gCombatNext adjustment when removing cached pointer.

### Dangling Group Pointers

Symptom: crash when accessing follower/master pointers after combatant death, use-after-free in group operations. Cause: deleting combatant without calling reformGroup first.

Fix: always call "vict->reformGroup()" immediately before "delete vict". Never delete without reformGroup. The reformGroup call breaks bidirectional follower/master relationships, preventing other group members from accessing freed memory.

### Continuing After DELETE_THIS

Symptom: use-after-free accessing attacker after deletion, crash on member access like ch->getName(). Cause: checking DELETE_THIS but not breaking execution flow, continuing to use ch pointer after deletion.

Fix: immediately after "delete ch; ch = NULL", use "break" to exit loop or "return DELETE_THIS" to propagate to caller. Never execute additional statements that reference ch after deletion.

### Wrong Death Detection for reconcileDamage

Symptom: victim pointer used after death, function continues when victim should be dead. Cause: checking "IS_SET_DELETE(rc, DELETE_VICT)" for reconcileDamage return, which returns -1 on death not DELETE_VICT.

Fix: check reconcileDamage return with "if (rc == -1)" not IS_SET_DELETE. Translate to DELETE_VICT for caller: "if (rc == -1) return DELETE_VICT". The -1 sentinel is distinct from non-negative damage values.

### Ignoring canFight Return Value

Symptom: random crashes during combat involving mounts, rider dies but execution continues. Cause: calling canFight without checking return value, missing DELETE_THIS from mount fall.

Fix: always capture return value "int rc = canFight(target)". Check "IS_SET_DELETE(rc, DELETE_THIS)" and propagate immediately. Check "!rc" and stop combat without deletion. Never ignore canFight return value.

### Wrong DELETE Flag Macro

Symptom: DELETE flags not detected, IS_SET returns false when deletion occurred. Cause: using IS_SET instead of IS_SET_DELETE for DELETE flags.

Fix: DELETE flags use bit 29 combined pattern. Always use IS_SET_DELETE macro for checking DELETE_THIS, DELETE_VICT, DELETE_ITEM. Never use plain IS_SET which checks different bit pattern.

### Missing Weapon Deletion Check

Symptom: crash accessing weapon pointer after oneHit, use-after-free on weapon object. Cause: not checking IS_SET_DELETE(rc, DELETE_ITEM) after oneHit, weapon destroyed but still referenced.

Fix: after oneHit call, check "IS_SET_DELETE(rc, DELETE_ITEM)". If true, "delete weapon; weapon = NULL; break" to stop attack loop. Never assume weapon survives oneHit call.

### Quest Solo-Kill Not Failing

Symptom: quest credit awarded when other players assisted. Cause: not checking for AFFECT_COMBAT with COMBAT_SOLO_KILL in damage code, or not removing affect in stopFighting.

Fix: in damage code, iterate victim affects checking for "af->type == AFFECT_COMBAT && af->modifier == COMBAT_SOLO_KILL". If attacker doesn't match quest-holder, set TOG_AVENGER_CHEAT and affectRemove. In stopFighting, remove affect if combat ends before death.

### Combat List Corruption

Symptom: abort with "Char fighting not found Error - ABORT" message. Cause: manually modifying next_fighting pointers without updating combat list links, or deleting character without removing from list first.

Fix: never manually set next_fighting. Use setVictFighting to add, stopFighting to remove. If abort triggers, indicates serious memory corruption requiring investigation of all code paths that modify combat list.

### Attack Distribution Sync Issues

Symptom: all characters with same attack speed fire simultaneously, burst damage. Cause: using round-relative pulse instead of absolute pulse in hit calculation, or not phase-shifting secondary hand.

Fix: pass (pulse + tmp_pulse) from perform_violence to hit, where pulse is absolute server uptime. Verify secondary hand formula adds (hit_wait / 2) to pulse value. This ensures timing varies with server uptime and prevents synchronization.
