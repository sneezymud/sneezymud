---
title: Command System
category: important
keywords: [parseCommand, doCommand, searchForCommandNum, commandArray, triggerSpecial, three-tier resolution, DELETE_VICT, vict parameter, flag translation, reconcileDamage]
related: [memory-safety.md, combat-formulas.md, spec-procs.md]
primary_symbols:
  functions: [parseCommand, doCommand, searchForCommandNum, triggerSpecial, doKick, doBash, doTrip, reconcileDamage]
  classes: [TBeing, commandInfo]
  files: [code/code/misc/parse.cc, code/code/misc/parse.h, code/code/cmd/cmd_kick.cc]
---

## Overview

Player commands flow through parsing, alias expansion, lookup, validation, spec proc hooks, and dispatch via a switch statement. Command handlers follow strict ownership rules for DELETE flags to prevent use-after-free crashes.

The system has two critical responsibilities: routing player input to the correct handler function, and ensuring that when targets die during command execution, memory is freed exactly once by the correct owner. The dispatch pipeline expands aliases, validates permissions and position requirements, gives spec procs a chance to intercept, then calls the implementation. Command implementations use a three-tier target resolution pattern and must translate DELETE flags when calling helper functions that swap semantic roles.

Commands are looked up by abbreviation matching in a linear array. The first match wins. Position requirements prevent sleeping characters from fighting, paralyzed characters from moving. Spec procs can intercept any command before the handler runs. The handler resolves the target using parameter, argument name, or current opponent as fallbacks, performs the action, and returns flags indicating what died.

The DELETE_VICT ownership rule prevents double-free: if the caller provided the victim pointer, return the flag so caller can delete. If the handler resolved the victim itself via lookup, delete directly and clear the flag. Flag translation handles role reversals: when calling victim->crashLanding, their DELETE_THIS becomes our DELETE_VICT because the victim is dying, not us. When calling victim->trySpringleap on this, roles swap entirely: their THIS is our VICT, their VICT is our THIS.

## Patterns

### Command Dispatch Flow

Player input enters parseCommand which tokenizes on whitespace and extracts the first word. If doAlias is enabled and aliases exist for the player, alias expansion occurs before lookup. Multiline aliases separated by tilde execute as distinct commands. Percent signs in alias definitions are replaced with remaining arguments.

After expansion, searchForCommandNum performs linear search through commandArray using is_abbrev for prefix matching. The enum value is returned to doCommand, which validates immortal level requirements, paralysis status, position constraints, and captive restrictions. If validation passes, triggerSpecial gives room spec, equipment specs, inventory specs, and task interruption a chance to handle the command. Returning TRUE from a spec proc prevents the switch statement from running.

The switch statement maps enum values to handler functions. Direction commands map to doMove, say/shout to doSay, combat skills to their implementations. Handlers receive the remaining argument string and an optional vict parameter for programmatic calls. Return values combine boolean success with DELETE flags.

### Adding New Commands

Extend the cmdTypeT enum in parse.h before MAX_CMD_LIST. Add a commandInfo entry in buildCommandArray specifying the command name, minimum position, and minimum immortal level. Mortals use zero for level. Add a case to the doCommand switch mapping the enum to your handler function. Implement the handler following the patterns in this document.

### Three-Tier Target Resolution

Combat commands accept an optional vict parameter for programmatic calls and a string argument for player input. Resolution proceeds in three tiers: if vict is non-null, use it directly. Otherwise parse the argument string using get_char_room_vis to find a target by name in the current room. If parsing fails or produces no argument, fall back to fight to use the current combat opponent. If all three tiers fail, send an error message and return FALSE.

This pattern appears in doKick, doBash, doTrip, doHeadbutt, doGrapple, doSlam, doDisarm, doStomp, doDeathstroke, doWhirlwind, doSteal, and doRescue. Non-combat commands may omit tier three if they require an explicit target.

### DELETE_VICT Ownership Pattern

The caller who provides a non-null vict parameter owns that pointer and must delete it. The callee who resolves the victim via get_char_room_vis or fight owns that pointer and must delete it. This prevents double-free when multiple stack frames reference the same being.

When a helper function returns DELETE_VICT, check whether vict was provided. If yes, return the flag immediately without deleting, allowing the caller to perform deletion. If no, delete the victim pointer, set it to null, clear DELETE_VICT from the return value using REM_DELETE, then return. This ensures exactly one deletion occurs at the correct ownership boundary.

The vict check pattern appears after every call that might return DELETE_VICT: rc = kick(this, victim, skill); if (IS_SET_DELETE(rc, DELETE_VICT)) { if (vict) return rc; delete victim; victim = NULL; REM_DELETE(rc, DELETE_VICT); }

### Flag Translation Between Contexts

DELETE flags are semantic: DELETE_THIS means delete the being executing the method, DELETE_VICT means delete the target parameter. When calling a helper where roles differ from the caller's context, flags must be translated.

Direct context: calling kick(this, victim, skill) from doKick means this is THIS, victim is VICT. Flags pass through unchanged. Calling victim->crashLanding from bashSuccess means crashLanding's THIS refers to victim, which is the caller's VICT. If crashLanding returns DELETE_THIS, translate to DELETE_VICT before returning to the caller.

Swapped context: calling victim->trySpringleap(this) from grapple means trySpringleap's THIS is victim and VICT is this. Both parameters swap roles. Translate DELETE_THIS to DELETE_VICT and DELETE_VICT to DELETE_THIS before returning. If both flags are set, both translations apply and the result is both flags set, so return unchanged.

Build a mental map of which parameter corresponds to which role at each call boundary. The helper's perspective is its parameters. The caller's perspective is its variables. Translation occurs when these mappings differ.

### Common Command Structure

Combat commands split into four functions: public interface with target resolution, static implementation with skill checks, success handler, failure handler.

The public interface doXXX accepts a string argument and optional vict parameter. It performs three-tier resolution to find victim, validates same room and basic constraints, calls the static implementation function, applies skill lag on success, handles DELETE_VICT with ownership check, propagates DELETE_THIS, and returns.

The static implementation function xxx accepts the actor, victim, and skill enum. It validates via canXXX, checks and consumes resources like movement points, calculates skill success using getSkillValue and specialAttack, then branches to xxxSuccess or xxxFail based on outcome.

Success handlers calculate damage using getSkillDam, send act messages, call reconcileDamage and check for -1 death return, apply secondary effects, and return TRUE or DELETE flags. Failure handlers send failure messages, may impose penalties like crashLanding, call reconcileDamage with zero damage to initiate combat, and return TRUE or FALSE depending on whether skill lag should apply.

### reconcileDamage Death Detection

reconcileDamage returns -1 as a magic sentinel value when the victim dies, not DELETE_VICT. Check the return value with == -1, then return DELETE_VICT to the caller. Never use IS_SET_DELETE with reconcileDamage results because the value is a signed integer, not a bitfield.

The pattern is: if (reconcileDamage(victim, dam, skill) == -1) return DELETE_VICT; This check must occur immediately after the call before any code that accesses the victim pointer.

### Area-of-Effect Commands

Commands affecting multiple targets must avoid iterator invalidation when deleting. Build a vector of valid targets in a first pass, then iterate the vector in a second pass. Before processing each target, verify it still exists in the expected room since victims may flee or be deleted by earlier iterations.

During the second pass, call the attack or skill function, check for DELETE_VICT, delete the local pointer, and set it to null. Do not return early when a target dies since other targets remain to process. Accumulate flags and return the combined result after all targets are handled.

### Object Interaction Commands

Commands that manipulate objects use DELETE_ITEM for object deletion. Target resolution differs because objects use TThing pointers and room contents lists instead of get_char_room_vis. Deletion ownership follows the same principle: if the caller provided the object pointer, return DELETE_ITEM. If the handler found it via searchLinkedListVis, delete directly and clear the flag.

Some object commands like get accept a container parameter named sub. Helper functions may return DELETE_THIS to indicate the container should be deleted. In the caller's context, the container maps to DELETE_VICT, requiring translation.

## Reference

### commandArray Metadata

Each command enum maps to a commandInfo instance containing the name string used for abbreviation matching, minPosition specifying the minimum stance required, and minLevel specifying the minimum immortal level with zero for mortals.

buildCommandArray in parse.cc allocates the entries. Examples: CMD_LOOK requires POSITION_RESTING and level 0. CMD_ECHO requires POSITION_SLEEPING and GOD_LEVEL1. CMD_NORTH requires POSITION_STANDING and level 0.

### Position Requirements

POSITION_DEAD allows only score, who, help. POSITION_SLEEPING allows list, wake. POSITION_RESTING allows look, say, get, most communication. POSITION_SITTING allows buy, sell. POSITION_STANDING allows dance and general movement. POSITION_FIGHTING allows kill, attack. Each higher level includes permissions from lower levels.

Commands requiring movement check against minPosition in doCommand before dispatch. Paralyzed and stunned characters fail this check for any command requiring action.

### Alias Syntax

Percent sign is replaced with the entire argument string after the alias name. Tilde separates multiple commands executed sequentially. Aliases expand before command lookup, so the expansion must produce valid command syntax.

Example: alias "att" = "attack %" expands "att goblin" to "attack goblin". Alias "buff" = "cast armor~cast bless" executes both casts when triggered. Alias "gl" = "get all.coin~look" gets coins then looks.

### Spec Proc Hook Order

triggerSpecial checks in sequence: task interruption for active tasks, spell task for casting interruption, room spec proc, equipment specs for worn items, inventory specs for carried items, room contents specs for objects and beings in the same room. First proc to return TRUE consumes the command and prevents further processing.

Spec procs receive the command enum, argument string, and may access the being and room context. Returning FALSE allows command processing to continue. Returning TRUE stops dispatch. Returning DELETE_THIS signals being death.

### Single-Character Shortcuts

Apostrophe maps to say, colon and comma map to emote. These shortcuts bypass alias expansion and go directly to command lookup. They are handled in parseCommand before the main lookup.

### Hide-Breaking Commands

Most commands break hide by setting the HIDDEN flag off. Exceptions that preserve hide: look, score, inventory, help, who, equipment, save, exits, consider. Backstab and slit break hide after execution rather than before, allowing the skill to use the hide bonus then reveal the attacker.

Commands check for HIDDEN flag and clear it during execution unless explicitly exempted. The flag removal happens in doCommand before the switch statement for most commands.

## Implementation

### File Organization

parse.cc contains parseCommand, doCommand, searchForCommandNum, triggerSpecial, buildCommandArray, and the main dispatch switch. parse.h declares the cmdTypeT enum with over 500 command entries, the commandInfo class, and the external commandArray declaration.

Individual command implementations reside in cmd/ directory: cmd_kick.cc for kick functions, cmd_bash.cc for bash, cmd_trip.cc for trip, cmd_combat.cc for generic combat, cmd_social.cc for emotes. Each file contains the public doXXX interface, static implementation helpers, and success/fail handlers.

### Validation Helpers

canXXX methods validate whether an action can be performed against a target. canKick checks for valid target type, combat restrictions, position requirements. canBash checks size differential and stance. canSteal checks for immortals and no-steal flags. These return TRUE if allowed, send error messages and return FALSE otherwise.

Validation occurs after target resolution but before resource consumption to avoid charging costs for impossible actions. The SILENT_NO parameter suppresses error messages when called from AI or spec procs.

### Resource Consumption

Movement-based skills deduct move points using addToMove with negative value. Check current move points against thresholds before deducting to prevent negative values. Some skills also check vitality, mana, or piety depending on class restrictions.

Consume resources after validation but before skill checks to charge the attempt cost regardless of outcome. Successful execution may impose additional lag via addSkillLag using the skill enum and return value.

### Skill Check Functions

getSkillValue returns the learned percentage for a skill modified by stats, equipment, affects, and learning level. specialAttack rolls to-hit using combat formulas and returns success status accounting for guarantees and immunities. bSuccess rolls against the skill percentage and returns TRUE if the raw action succeeds.

Combat skills typically require both specialAttack to hit the target and bSuccess to execute the technique. Either failing causes the skill to miss. Some skills use different combinations or skip hit checks for guaranteed effects.

### Damage Calculation

getSkillDam computes base damage from skill level, advancement learning, victim armor class, and random variance. It applies multipliers for critical hits, immunities, and resistances. The result is passed to reconcileDamage which applies damage reduction, handles absorb effects, subtracts hit points, and triggers death if health drops to zero.

Success handlers call getSkillDam to compute damage, then reconcileDamage to apply it. The -1 return value signals death and requires immediate return with DELETE_VICT. Non-fatal damage allows additional effects like position changes or debuffs.

### Position and State Changes

crashLanding forces a being to a lower position, applies movement cost, sends messages, and may trigger falling damage or mount ejection. It returns DELETE_THIS if the being dies from falling. Callers must check this return and translate based on which being crashed.

Some skills impose position changes on success: bash drops to sitting, trip to resting. Failures may impose position changes on the attacker as penalties. Position affects subsequent actions and requires refreshing stance to continue fighting effectively.

### Combat Initiation

Reconciling damage with non-zero amount automatically starts combat via the damage pipeline. Skills that miss but should provoke combat call reconcileDamage with zero damage to set up the fight relationship without inflicting harm. This triggers setCharFighting and setVictFighting to establish bidirectional combat state.

Once fighting, the combat system handles automatic swings, fleeing, and death processing. Commands executed during combat access fight() to retrieve the current opponent for tier three resolution.

## Troubleshooting

### Double-Free Crashes

Symptom: crash in delete or destructor when victim dies. Cause: forgetting the vict ownership check, deleting when caller owns the pointer. Fix: always check if (vict) return rc; before deleting victim when DELETE_VICT is set. The caller provided the pointer, so the caller must delete it.

Symptom: crash when returning to caller after deletion. Cause: caller expects DELETE_VICT flag but you deleted and cleared it. Fix: only delete and clear the flag when vict is NULL, meaning you resolved victim yourself. If vict is non-null, return the flag immediately.

### Wrong Flag Translation

Symptom: crash after calling victim->method or helper with swapped parameters. Cause: returning helper's DELETE_THIS directly when the helper's THIS corresponds to your VICT. Fix: identify which parameter maps to which role, translate flags when they differ. victim->crashLanding returns DELETE_THIS for victim death, which is your DELETE_VICT.

Symptom: both combatants die but only one deletion occurs. Cause: failing to handle both flags set simultaneously. Fix: check for both flags before checking individual flags, return both unchanged when both are present.

### Memory Access After Death

Symptom: use-after-free when accessing victim after reconcileDamage. Cause: failing to check for -1 return value from reconcileDamage. Fix: check if (reconcileDamage(...) == -1) return DELETE_VICT; immediately after the call. Do not access victim pointer after this line.

Symptom: accessing victim after DELETE_VICT set by helper. Cause: checking flag but continuing execution. Fix: return immediately when DELETE_VICT is detected. The pointer is no longer valid after the flag is set.

### Iterator Invalidation

Symptom: crash during whirlwind or area effect when target dies. Cause: deleting from stuff list while iterating it. Fix: build a vector of targets first, iterate the vector in a second pass, check room membership before processing each target since they may have fled.

Symptom: combat list corruption when multiple beings die. Cause: not using gCombatNext to cache next pointer before operations. Fix: always set gCombatNext = ch->next_fighting before calling any function that might delete combatants.

### Spec Proc Interference

Symptom: command never reaches handler despite valid input. Cause: spec proc returning TRUE and consuming the command. Fix: check triggerSpecial in the calling stack, identify which spec is intercepting, verify the interception is intentional.

Symptom: spec proc calls command but ignores return value. Cause: not checking DELETE_THIS from doCommand when called from spec. Fix: store return value, check IS_SET_DELETE(rc, DELETE_THIS), return TRUE from spec proc to signal deletion.

### Position and Paralysis Blocks

Symptom: command fails with no error message. Cause: position requirement not met, validation happens before handler runs. Fix: check getPosition against commandArray minPosition, ensure character is in required stance.

Symptom: paralyzed character cannot act. Cause: doCommand checks paralysis and stun before allowing command execution. Fix: remove paralysis affect, wait for duration to expire, or use immortal command immunity.

### Alias Expansion Issues

Symptom: alias produces incorrect command. Cause: percent sign not replaced or tilde not splitting properly. Fix: verify alias definition syntax, ensure expansion produces valid command strings, test with simple inputs.

Symptom: recursive alias loop. Cause: alias expands to itself or circular chain. Fix: expansion happens once per call, should not loop, but verify alias definitions do not create cycles.

### Flag Bit Pattern Errors

Symptom: IS_SET does not detect DELETE flag. Cause: using IS_SET instead of IS_SET_DELETE which handles bit 29. Fix: always use IS_SET_DELETE for DELETE_THIS, DELETE_VICT, DELETE_ITEM, DELETE_ITEM2 checks.

Symptom: clearing flag with REMOVE_BIT does not work. Cause: DELETE flags use special bit position. Fix: use REM_DELETE macro which handles the DELETE flag bit pattern correctly.
