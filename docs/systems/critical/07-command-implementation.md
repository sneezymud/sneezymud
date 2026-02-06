---
title: Command System
description: Player command flow through parsing, lookup, dispatch, and implementation patterns including DELETE flag ownership and target resolution.
keywords: [command parsing, command dispatch, three-tier resolution, alias expansion]
category: critical
primary_symbols:
  functions: [parseCommand, doCommand, searchForCommandNum, triggerSpecial, buildCommandArray, reconcileDamage, get_char_room_vis]
  classes: [TBeing, commandInfo]
  enums: [cmdTypeT, DELETE_VICT, DELETE_THIS, DELETE_ITEM, MAX_CMD_LIST, POSITION_DEAD, POSITION_SLEEPING, POSITION_RESTING, POSITION_SITTING, POSITION_STANDING, POSITION_FIGHTING]
---

# Command System

## Overview

The command system transforms raw player input into executed game actions through a multi-stage pipeline. Input first undergoes alias expansion, then command lookup via abbreviation matching, followed by validation checks (level requirements, position restrictions, status effects). Before execution, special procedures can intercept and consume commands. Finally, a dispatch switch routes to the appropriate handler function.

The system's safety challenge centers on pointer ownership. Commands that target other beings may receive victim pointers from callers (spec procs, other commands) or resolve them internally. When targets die during command execution, the system must track who owns which pointer to avoid dangling references and double deletions.

## Patterns

### Command Handler Implementation

Always use three-tier target resolution for combat commands: first check the `vict` parameter, then parse the argument string, then fall back to current fight target. This ensures commands work correctly whether invoked by players, spec procs, or other code.

Always check ownership before deleting victims. If the `vict` parameter was non-null, return DELETE_VICT and let the caller delete. If you resolved the victim yourself via lookup functions or the fight target, delete directly and clear the flag with REM_DELETE.

Always translate DELETE flags when calling helpers on targets. When calling a method on `victim` that returns DELETE_THIS, translate that to DELETE_VICT in your return value since the helper's "this" is your victim.

Always check reconcileDamage return value against -1 for death, not with IS_SET_DELETE. The function returns -1 as a sentinel, not a flag bitmask.

Always use IS_SET_DELETE instead of IS_SET when checking for DELETE flags. The DELETE flags use a special bit pattern that IS_SET cannot detect. Similarly, use REM_DELETE instead of REMOVE_BIT to clear them.

### Execution Flow

Never continue execution after detecting a DELETE flag. Check immediately and return or break.

Never delete the same entity twice. If you return a DELETE flag, do not also delete the entity.

Never forget to clear DELETE flags after deleting. Use REM_DELETE to prevent callers from double-deleting.

When both DELETE_THIS and DELETE_VICT are set, check for both flags before individual flags and return both unchanged since both entities are dying.

### Adding Commands

Always add the enum before MAX_CMD_LIST in parse.h. The enum value determines array placement.

Always add the array entry in buildCommandArray with appropriate position and level requirements.

Always add the switch case in doCommand to route to your handler.

### Area Effect Commands

Commands affecting multiple targets must avoid iterator invalidation when deleting. Build a vector of valid targets before processing, then iterate the vector. Re-validate each target before processing since earlier iterations may cause movement or death.

Do not return early when a target dies; other targets remain to process. Accumulate flags and return the combined result after all targets are handled.

## Reference

### Position Requirements

| Position | Example Commands |
|----------|------------------|
| POSITION_DEAD | score, who, help |
| POSITION_SLEEPING | list, wake, attack |
| POSITION_RESTING | look, say, get, equipment |
| POSITION_SITTING | buy, sell |
| POSITION_STANDING | dance, most actions |
| POSITION_FIGHTING | kill |

### Alias Expansion Syntax

| Syntax | Effect |
|--------|--------|
| `%` | Replaced with remaining arguments |
| `~` | Multiline separator (executes as separate commands) |

Examples: `alias "att" = "attack %"` expands "att goblin" to "attack goblin". `alias "buff" = "cast armor~cast bless"` executes both casts. `alias "gl" = "get all.coin~look"` gets coins then looks.

### Single-Character Shortcuts

| Character | Command |
|-----------|---------|
| `'` | say |
| `:` | emote |
| `,` | emote |

### DELETE Flag Translation Table

| Helper Call Pattern | Helper's DELETE_THIS | Helper's DELETE_VICT |
|--------------------|----------------------|----------------------|
| `this->helper(victim)` | Our DELETE_THIS | Our DELETE_VICT |
| `victim->helper(this)` | Our DELETE_VICT | Our DELETE_THIS |
| `victim->helper(item)` | Our DELETE_VICT | Our DELETE_ITEM |
| `item->helper(victim)` | Our DELETE_ITEM | Our DELETE_VICT |

### Spec Proc Return Values

| Return Value | Effect |
|--------------|--------|
| FALSE | Command processing continues |
| TRUE | Command consumed, dispatch stops |
| DELETE_THIS | Being died, caller must handle deletion |

### Hide-Preserving Commands

Commands that do not break hide: look, score, inventory, help, who, equipment, save, exits, consider. Backstab and slit break hide after execution completes.

### Source Files

| File | Purpose |
|------|---------|
| parse.cc | parseCommand, doCommand, searchForCommandNum, triggerSpecial, buildCommandArray |
| parse.h | cmdTypeT enum, commandInfo class, commandArray declaration |
| cmd_kick.cc | doKick, kick, kickHit, kickMiss |
| cmd_bash.cc | doBash, bash, bashSuccess, bashFail |
| cmd_trip.cc | doTrip, trip, tripSuccess, tripFail |
| cmd_headbutt.cc | doHeadbutt, headbutt, headbuttHit, headbuttMiss |
| cmd_grapple.cc | doGrapple, grapple |
| cmd_slam.cc | doSlam, slamSuccess, slamFail |
| cmd_disarm.cc | doDisarm, disarm |
| cmd_stomp.cc | doStomp, stomp, stompHit, stompMiss |
| cmd_deathstroke.cc | doDeathstroke, deathstrokeSuccess, deathstrokeFail |
| cmd_whirlwind.cc | doWhirlwind, whirlwind, whirlwindSuccess |
| cmd_steal.cc | doSteal, steal, failSteal |
| cmd_rescue.cc | doRescue, rescue |
| cmd_get.cc | doGet, get |

## Implementation

### Command Pipeline Stages

The entry point is TBeing::parseCommand in parse.cc. This function extracts the first word from player input, performs alias expansion if enabled, then calls searchForCommandNum to find the matching command. The lookup performs linear search with abbreviation matching via is_abbrev, meaning "lo" matches "look". Order in commandArray determines which command matches when multiple share a prefix.

The commandArray stores commandInfo objects indexed by cmdTypeT enum values. Each entry contains the command name, minimum position requirement, and minimum immortal level (0 for mortal commands). buildCommandArray populates this array during startup.

After lookup, doCommand performs validation: level check against commandArray entry, paralysis/stun state for movement commands, position requirement, and captive restrictions. The function then calls triggerSpecial to let spec procs intercept. triggerSpecial checks task interruption, spell tasks, room specs, equipment specs, inventory specs, and room contents in that order. If any spec proc returns TRUE, command processing stops.

The switch statement in doCommand spans over 500 cases, routing to handler functions like doMove for movement, doSay for speech, doKick for combat skills. Handlers return integer values: FALSE for no action, TRUE for success, or DELETE_* flags when entities die.

### Target Resolution Mechanics

Combat command handlers implement a three-tier fallback pattern. The public interface accepts both an argument string and an optional vict parameter. When vict is non-null, it takes priority. Otherwise, get_char_room_vis parses the argument string to find a visible target by name. If that fails, fight returns the current combat opponent. Only if all three tiers fail does the command report "Kick whom?" or similar.

This pattern enables commands to be called programmatically by spec procs (passing vict directly), by players typing target names (argument parsing), or during combat without specifying a target (fight target fallback). Commands that do not support the vict parameter interface cannot be safely invoked by other code.

### Ownership and Deletion Protocol

The ownership distinction drives the entire deletion protocol. When vict is non-null, the caller provided the pointer and therefore owns it. When vict is null and the handler resolved victim itself, the handler owns that pointer. This ownership determines deletion responsibility.

Upon receiving DELETE_VICT from a helper function, the handler checks the vict parameter. If non-null, the handler returns DELETE_VICT without deleting, passing responsibility to the caller. If null, the handler deletes victim directly and clears the flag with REM_DELETE before returning.

Flag translation handles calls where roles differ between caller and callee. When calling victim->crashLanding, the callee's "this" is the caller's victim. If crashLanding returns DELETE_THIS (meaning it died), the caller must translate this to DELETE_VICT in its return value. The translation table in Reference provides the complete mapping.

### Command Structure Components

Combat commands typically decompose into four functions. The public doXXX function handles target resolution and ownership-aware deletion. A static xxx function validates preconditions (via canXXX helpers), checks and consumes resources like movement points, performs skill checks, and branches to success or failure. The xxxSuccess function calculates damage, emits messages, and calls reconcileDamage. The xxxFail function handles failure messages and side effects like falling or position penalties on the attacker.

The split between these functions localizes concerns: doXXX owns the deletion protocol, the static function owns resource checks and skill resolution, and success/fail functions own their specific outcomes.

Validation helpers (canXXX methods) accept a SILENT_NO parameter to suppress error messages when called from AI or spec procs.

### Combat Initiation

Skills that miss but should provoke combat call reconcileDamage with zero damage to establish the fight relationship without inflicting harm. This triggers setCharFighting and setVictFighting to establish bidirectional combat state. Once fighting, commands access fight() to retrieve the current opponent for tier three resolution.

### Area Effects and Iteration Safety

Commands affecting multiple targets like doWhirlwind require special iteration handling. Building a vector of valid targets before processing prevents iterator invalidation when victims die and are removed from room contents. The processing loop must re-validate targets (checking they are still in the room) since earlier iterations may have caused movement.

Object commands use DELETE_ITEM analogously to DELETE_VICT. The same ownership rules apply: if the caller provided an object pointer, return the flag; if you resolved the object, delete and clear.

## Troubleshooting

### Crash After Combat Command Execution

**Symptom:** Server crashes shortly after a combat command completes, often in unrelated code accessing the victim.

**Cause:** Handler deleted victim without checking vict parameter ownership. Caller still holds dangling pointer.

**Diagnostic:** Check if the command was invoked by a spec proc or other code passing a vict parameter. If so, handler should have returned DELETE_VICT instead of deleting.

**Fix:** Add vict parameter check before deletion. If vict is non-null, return DELETE_VICT; otherwise delete and call REM_DELETE.

### Victim Death Not Detected

**Symptom:** Code continues executing after victim should be dead, causing messages to corpses or invalid state.

**Cause:** Checking reconcileDamage return with IS_SET_DELETE instead of comparing to -1.

**Diagnostic:** Locate the reconcileDamage call and verify the return check syntax.

**Fix:** Change to `if (reconcileDamage(victim, dam, skill) == -1) return DELETE_VICT;`

### DELETE Flag Check Never Triggers

**Symptom:** DELETE_VICT or DELETE_THIS conditions appear to never be true despite deaths occurring.

**Cause:** Using IS_SET instead of IS_SET_DELETE. DELETE flags use a bit pattern that IS_SET does not detect.

**Diagnostic:** Search for IS_SET calls with DELETE_* arguments.

**Fix:** Replace IS_SET with IS_SET_DELETE for all DELETE flag checks. Similarly, use REM_DELETE instead of REMOVE_BIT to clear flags.

### Wrong Entity Deleted After Helper Call

**Symptom:** The command caster dies or vanishes when the victim should have, or vice versa.

**Cause:** Flag translation error when calling method on target. Handler returned DELETE_THIS when it should have returned DELETE_VICT.

**Diagnostic:** Identify the helper call and trace which entity is "this" in that context.

**Fix:** Apply flag translation per the Reference table. Helper's DELETE_THIS on victim means caller's DELETE_VICT.

### Both Combatants Die

**Symptom:** Both combatants die but only one deletion occurs, or crash from handling only one flag.

**Cause:** Failing to handle both flags set simultaneously.

**Fix:** Check for both flags before checking individual flags, return both unchanged when both are present.

### Command Not Found Despite Correct Spelling

**Symptom:** Player types exact command name but receives "Unknown command" message.

**Cause:** Missing commandArray entry in buildCommandArray, or enum defined after MAX_CMD_LIST.

**Diagnostic:** Check that the CMD_XXX enum appears before MAX_CMD_LIST in parse.h. Verify buildCommandArray has a corresponding commandInfo allocation.

**Fix:** Ensure enum placement and array initialization are both present and correctly ordered.

### Iterator Invalidation in Area Effects

**Symptom:** Crash during whirlwind or area effect when target dies.

**Cause:** Deleting from stuff list while iterating it, or not caching next pointer.

**Fix:** Build a vector of targets first, iterate the vector in a second pass, check room membership before processing each target. For combat lists, always set `gCombatNext = ch->next_fighting` before calling any function that might delete combatants.

### Spec Proc Consumes Command

**Symptom:** Command never reaches handler despite valid input.

**Cause:** Spec proc returning TRUE and consuming the command.

**Fix:** Check triggerSpecial in the calling stack, identify which spec is intercepting, verify the interception is intentional.

### Alias Expansion Issues

**Symptom:** Alias produces incorrect command or enters infinite loop.

**Cause:** Percent sign not replaced, tilde not splitting properly, or circular alias chain.

**Fix:** Verify alias definition syntax, ensure expansion produces valid command strings. Expansion happens once per call and should not loop, but verify alias definitions do not reference each other cyclically.
