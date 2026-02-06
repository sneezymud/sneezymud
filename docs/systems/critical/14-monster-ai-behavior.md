---
title: Monster AI and Behavior
description: NPC autonomous behavior systems including opinion mechanics, aggression targeting, pursuit/tracking, and memory management
category: critical
keywords: [opinion mechanics, aggression, pursuit, tracking]
source_files: [code/code/misc/monster.cc, code/code/misc/mobact.cc, code/code/misc/opinion.cc, code/code/misc/ai_utility.cc, code/code/disc/disc_thief_stealth.cc]
primary_symbols:
  functions: [mobileActivity, aggroCheck, hunt, targetFound, takeFirstHit, assistFriend, senseWimps, dirTrack, Hates, Fears, addHated, addFeared, developHatred]
  classes: [TMonster, Mobile_Attitude, opinionData, charList]
  enums: [ACT_SENTINEL, ACT_SCAVENGER, ACT_AGGRESSIVE, ACT_STAY_ZONE, ACT_WIMPY, ACT_HATEFUL, ACT_AFRAID, ACT_HUNTING, ACT_IMMORTAL, HATE_SEX, HATE_RACE, HATE_CHAR, HATE_CLASS, HATE_VNUM, FEAR_SEX, FEAR_RACE, FEAR_CHAR, FEAR_CLASS, FEAR_VNUM, AFF_AGGRESSOR, SPELL_TRAIL_SEEK, SPELL_SUMMON, SPELL_ASTRAL_WALK, SKILL_CONCEALMENT, TOG_IS_CRAVEN, TOG_IS_VICIOUS, MIN_GLOB_TRACK_LEV]
---

# Monster AI and Behavior

## Overview

The monster AI system gives NPCs autonomous decision-making through emotional state modeling, categorical targeting, and persistent memory. Mobs evaluate their environment each pulse, deciding whether to attack, flee, pursue, or assist allies based on personality attributes and historical interactions.

The system operates on two levels: categorical opinions (hate all elves, fear all warriors) and individual tracking (hate the specific player who attacked me). Emotional thresholds determine when passive mobs become aggressive. Pursuit persists across rooms until targets escape or the mob loses interest.

Memory management is critical because mobs maintain linked lists of remembered characters. These lists require explicit chain deletion during cleanup, and hunting pointers can become stale when targets disconnect.

## Patterns

### Memory Management

Always delete charList chains by iterating with cached next pointers. The destructor only deletes the first node. Never delete a charList directly without iterating the chain.

Always validate the hunting pointer before dereferencing. Check that the target still exists, is a PC, and has a valid descriptor.

Always clear the hunting pointer before propagating DELETE_VICT. The caller may still see the stale pointer otherwise.

### DELETE Flag Safety

Always check return values from AI functions that engage in combat or movement. Functions like `hunt()`, `assistFriend()`, `takeFirstHit()`, `aggroCheck()`, and `goDirection()` can return DELETE_THIS.

Always propagate DELETE_THIS immediately without further processing. Never decrement counters or access member variables after a potential deletion.

Never continue execution after detecting DELETE_THIS or DELETE_VICT flags. Check immediately and return or break.

### Utility and Guild Mobs

Always check for utility and guild mobs before aggressive behavior. Call `UtilMobProc()` and `GuildProcs()` early in aggression functions. Forgetting these checks causes shopkeepers to attack customers.

### Target Validation

Always validate targets exist, are visible, and share the same room before acting. Use `canSee()` and `sameRoom()` checks.

Never assume the Mobile_Attitude target pointer is valid. It tracks only PCs and must be validated before use.

### Adding Fear

When adding feared characters with `addFeared()`, the function automatically clears hunting state if the mob is currently hunting that target. This prevents paradoxical behavior where a mob simultaneously fears and pursues the same character.

## Reference

### Emotional Attributes (Mobile_Attitude)

| Attribute | Range | Effect |
|-----------|-------|--------|
| suspicion | 0-100 | Investigation triggers, steal detection sensitivity |
| greed | 0-100 | Scavenging frequency, stealing from players |
| malice | 0-100 | Intent to harm; combined with anger triggers aggression |
| anger | 0-100 | Emotional volatility; combined with malice triggers aggression |

Additional fields: `target` pointer (PC-only for opinion target), `random` pointer (scratch space for interaction logic), `last_cmd` (last witnessed command type).

Randomized check methods (`isGreedy`, `isAngry`, `isMalice`, `isSusp`) return true if `::number(0, 100) < attribute`. Modifier methods (`US`/`DS`, `UG`/`DG`, `UA`/`DA`, `UM`/`DMal`) increase/decrease attributes by random amounts up to 2x the parameter.

Aggression formula: Attack if `4*anger + 5*malice >= 450` OR if `ACT_AGGRESSIVE` flag is set.

The `pissed()` function is a simpler check using only `isAngry()` and `isMalice()` without threshold, used for minor annoyances.

### Opinion Bitfields

| Hate Flag | Fear Flag | Target Type |
|-----------|-----------|-------------|
| HATE_SEX | FEAR_SEX | Specific sex |
| HATE_RACE | FEAR_RACE | Specific race |
| HATE_CHAR | FEAR_CHAR | Individual characters (uses clist) |
| HATE_CLASS | FEAR_CLASS | Specific class bitmask |
| HATE_VNUM | FEAR_VNUM | Specific mob vnum |

### charList Structure

| Field | Purpose |
|-------|---------|
| name | Character name (mud_str_dup, requires delete[]) |
| iHateStrength | Duration in game hours: `(mob_level + player_level + 5) * (focus / 120.0)`, yields 2-219 hours |
| account_id | Account ID for multi-character detection |
| player_id | Player ID for multi-character detection |
| next | Singly-linked list pointer |

### ACT Flags Affecting AI

| Flag | Behavior |
|------|----------|
| ACT_SENTINEL | Won't wander from birth room |
| ACT_SCAVENGER | Picks up items |
| ACT_AGGRESSIVE | Attacks players on sight |
| ACT_STAY_ZONE | Won't leave birth zone |
| ACT_WIMPY | Only attacks sleeping targets, flees at low HP |
| ACT_HATEFUL | Has active hate list |
| ACT_AFRAID | Has active fear list |
| ACT_HUNTING | Currently tracking a target |
| ACT_IMMORTAL | Cannot gain hatred |

### Activity Pulse Intervals

| Interval | Behavior |
|----------|----------|
| Every pulse | Spell effects, lag processing, position management |
| 2x MOBACT | Rescue allies, spec procs |
| 5x MOBACT | Charmed pet behavior, protection checks |
| 7x MOBACT | Horse finding, assist friends |
| 11x MOBACT | Scavenging |
| 13x MOBACT | Thief stealing |
| 16x MOBACT | Remove stuck items |
| 30x MOBACT | Return to default position |
| 50x MOBACT | Alignment spec procs |

### Tracking Distance by Class

| Class | Formula | Example (skill=100, level=20) |
|-------|---------|-------------------------------|
| Ranger | 2 * skillValue | 200 rooms |
| Thief | 2 * skillValue | 200 rooms |
| Warrior | skillValue / 2 | 50 rooms |
| Mage | skillValue + GetMaxLevel() | 120 rooms |
| Other | skillValue | 100 rooms |

### Tracking Modifiers

| Factor | Modifier |
|--------|----------|
| Giant race | 2x distance |
| Elven race | 2x distance |
| Devil/Demon race | Unlimited |
| TOG_IS_CRAVEN | -25 rooms |
| TOG_IS_VICIOUS | +25 rooms |
| SPELL_TRAIL_SEEK | +50 rooms, enables cross-zone |

### dirTrack Vision Requirements

Tracking requires one of: `roomp->light + vision_bonus > 0`, `ROOM_ALWAYS_LIT` flag, `AFF_TRUE_SIGHT`, `AFF_CLARITY`, or `isImmortal()`. Without sufficient vision, tracking fails.

SKILL_CONCEALMENT on target blocks tracking probabilistically: 150 modifier blocks 100%, 50 modifier blocks ~33%.

Global pathfinding via `choose_exit_global` activates for level >= 31 (MIN_GLOB_TRACK_LEV), those with ACT_HUNTING, or those affected by SPELL_TRAIL_SEEK. Portal handling encodes index as `code - 9` for values > 9.

### Target Scoring (senseWimps)

Base score: `HP + hitLimit + mana + (2000 - armor) + karma_scaled`. Lower score means more attractive target.

| Condition | Modifier |
|-----------|----------|
| Newbie (level < 10) | +500 - 50*level (protection scales down) |
| Non-combatant (not grouped with victim) | +1000 |
| Non-combatant (grouped with victim) | -2000 |
| Mage or Shaman class | -200 |
| Monk (fighting the mob) | -150 |

Anti-tank detection: If mob has AFF_AGGRESSOR and is fighting a pet/zombie while a PC is not engaged, switches to the PC with flavor message.

### Class Combat AI Dispatch

| Function | Class | Primary Skills |
|----------|-------|----------------|
| fighterMove | Warrior | Bash, bodyslam, spin, kick, disarm |
| monkMove | Monk | Springleap, hurl, bonebreak, chi |
| thiefMove | Thief | Stab, disarm |
| mageMove | Mage | Offensive spells by discipline |
| clerMove | Cleric | Healing, harm spells |
| shamanMove | Shaman | Spirit spells |
| deikhanMove | Deikhan | Charge (mounted), fighter moves |
| rangMove | Ranger | Nature skills |

### Key Source Files

| File | Contents |
|------|----------|
| monster.h | TMonster class, charList, opinionData, Mobile_Attitude |
| monster.cc | TMonster lifecycle, charList chain cleanup |
| opinion.cc | Hate/fear management, hunting setup, developHatred |
| mobact.cc | mobileActivity loop, combat AI, aggroCheck, hunt, targetFound |
| ai_utility.cc | pissed, aggro, aiTarget |
| disc_thief_stealth.cc | dirTrack pathfinding, doTrack player command |
| defs.h | ACT_*, HATE_*, FEAR_* flag definitions |

## Implementation

### Data Structures

The `Mobile_Attitude` class models emotional state through four 0-100 range values (suspicion, greed, malice, anger) with corresponding defaults. It maintains a target pointer restricted to PCs to prevent infinite mob-mob interaction loops.

The `opinionData` class tracks categorical opinions through bitfields for sex, race, class, and vnum hatred/fear. Individual character tracking uses a `charList` linked list containing character names, account/player IDs, and hate strength duration in game hours.

The `charList` destructor only frees its own memory, not the chain. TMonster's destructor must manually iterate and delete each node, caching the next pointer before deletion to avoid use-after-free.

### Opinion System

Adding individual hatred via `addHated()` creates a new charList node with hate strength calculated from combined levels and focus stat, yielding durations of 2-219 game hours. The `developHatred()` function during combat decides whether to add permanent hatred based on HP percentage (patience), level difference, and random variance. High-level mobs develop hatred faster against low-level attackers to prevent hit-and-flee XP exploitation.

Categorical hatred via `addHatred()` sets the appropriate bitfield flag and opinion value. Checking hatred via `Hates()` tests the character against all active opinion categories.

Adding fear clears hunting state if the feared target was the current hunt target. Global cleanup functions `DeleteHatreds()` and `DeleteFears()` iterate all mobs when characters are deleted to remove stale references.

When removing hatred via `remHated()`, the function iterates clist to find the matching node, unlinks it, deletes the name array, deletes the node, and clears HATE_CHAR if clist becomes empty.

### Aggression Targeting

The `aggro()` function determines combat readiness through emotional thresholds or the ACT_AGGRESSIVE flag, after filtering out utility mobs, guild mobs, and charmed pets.

The `aggroCheck()` function called from `mobileActivity()` iterates room occupants seeking valid targets. It first checks faction-based aggression for Cult/Brotherhood territorial combat. It skips polymorphed players. Wandered mobs outside their home zone have reduced aggression against low-level players. The karma check allows high-karma players to avoid aggression from intelligent, calm mobs (karma scaled 0-100 vs mob intelligence scaled 0-200 plus anger).

Target selection uses `takeFirstHit()` which dispatches to class-specific openers: thieves attempt backstab or throat slit, while others use `classStuff()` or direct attacks.

The `senseWimps()` function provides intelligent target switching for level 15+ mobs using a scoring system that favors wounded, low-armor, seated targets while avoiding newbies and mounted characters. It includes anti-exploitation logic to detect tank abuse, switching to attacking PCs who hide behind pet tanks.

### Pursuit and Tracking

Hunting initialization via `setHunting()` calculates tracking distance as 50 plus mob level, doubled if the target is hated. Persistence starts at mob level and decrements per tracking attempt. The old room is cached for return navigation.

The `hunt()` function processes each pulse: if persistence is exhausted, the mob returns home or stops hunting. If the target is visible in the room, `targetFound()` initiates combat. Otherwise, `dirTrack()` provides pathfinding direction and the mob moves toward the target. Musk gas in rooms costs extra persistence.

Movement distance per tick scales with cube root of level, using probabilistic rounding for the fractional component. Level 10 mobs move 2 rooms per hunt tick with 15.4% chance for a third.

Cleric mobs level 30+ have 20% chance to use SPELL_SUMMON (same zone) or SPELL_ASTRAL_WALK (cross-zone) instead of walking when hunting targets level 15+. Archer mobs with SPEC_ARCHER shoot at visible targets before moving.

The `dirTrack()` function requires adequate light or magical vision. Targets with SKILL_CONCEALMENT can probabilistically block tracking based on skill level. Global pathfinding via `choose_exit_global` activates for high-level characters, those with ACT_HUNTING, or those affected by SPELL_TRAIL_SEEK.

Player tracking via `doTrack` applies the class distance formula, race modifier, quest bit adjustment, and SPELL_TRAIL_SEEK bonus. With AUTO_HUNT autobit, the command is queued automatically for seamless pursuit.

### Activity Loop

The `mobileActivity()` function runs every Pulse::MOBACT (1.2 seconds) and dispatches all AI behavior in priority order: spell effects, lag handling, position management, charmed pet behavior, spec procs, stuck item removal, combat targeting, weapon selection, in-combat behavior (fear checks, class-specific combat, vampire/lycanthrope attacks), non-combat wandering, hate/fear processing, and aggression checks.

Different behaviors trigger at different intervals to spread computational load and create natural variation in mob behavior patterns.

### Response System

Mobs can have scripted responses loaded from the mobresponses database table. The Responses class maintains a linked list of triggers (respList with respCount entries and respMemory for interaction history) matching command types and argument patterns to command sequences. The `checkResponses()` function is called from `triggerSpecial()` when players act near mobs, and `modifiedDoCommand()` executes responses with special handling for message routing (CMD_RESP_TOROOM, CMD_RESP_TOVICT, CMD_RESP_TONOTVICT), item/mob spawning (CMD_LOAD, CMD_RESP_LOADMOB), and quest flag manipulation (CMD_FLAG, CMD_RESP_UNFLAG).

### Friend/Foe Logic

The `isFriend()` function returns true for group members, identical vnum mobs, or same-race same-faction mobs within 5 levels. The `assistFriend()` function causes mobs to join combat when friends are attacked. Police mobs have special behavior to break up fights and prioritize avenging fallen officers.

## Troubleshooting

### Memory Leak in charList

**Symptom:** Memory usage grows over time, especially with many mob spawns and deaths.

**Cause:** Deleting opinionData or TMonster without iterating the charList chain. The destructor only frees the first node.

**Diagnostic:** Enable memory sanitizer, look for leaked charList allocations pointing to opinion.cc or monster.cc.

**Fix:** Always iterate with cached next pointer when deleting charList chains. In TMonster destructor, loop through both hates.clist and fears.clist, caching k->next before deleting k.

### Crash on Hunting Pointer Dereference

**Symptom:** Crash in hunt(), targetFound(), or related functions when accessing specials.hunting.

**Cause:** Target disconnected or was deleted while hunting. The raw pointer becomes dangling.

**Diagnostic:** Check crash backtrace for access to specials.hunting member. Verify target validation checks exist before access.

**Fix:** Validate hunting pointer before use: check non-null, isPc(), and desc existence. Clear pointer when target becomes invalid.

### Shopkeeper Attacks Customers

**Symptom:** Utility mobs with shops or guild services attack players unexpectedly.

**Cause:** Missing UtilMobProc() or GuildProcs() check in aggression function.

**Diagnostic:** Check mob's spec proc assignment. Verify aggro() and aggroCheck() early-exit for utility mobs.

**Fix:** Add UtilMobProc(this) and GuildProcs(spec) checks at the start of aggressive behavior functions.

### Mob Continues After Death

**Symptom:** Crash after combat functions return DELETE_THIS, accessing freed memory.

**Cause:** Return value from takeFirstHit(), assistFriend(), or goDirection() not checked before continuing execution.

**Diagnostic:** Examine code path after combat-initiating calls. Look for missing IS_SET_DELETE checks.

**Fix:** Check IS_SET_DELETE(rc, DELETE_THIS) immediately after any function that can trigger combat or movement, and return before further processing.

### Stale Hunting Pointer After Target Death

**Symptom:** Mob behaves erratically or crashes after killing hunt target.

**Cause:** DELETE_VICT propagated without clearing specials.hunting first.

**Diagnostic:** Check targetFound() and related functions for pointer cleanup before DELETE_VICT return.

**Fix:** Set specials.hunting to NULL and clear ACT_HUNTING flag before returning DELETE_VICT.

### Mob Never Stops Hunting

**Symptom:** Mob pursues indefinitely without giving up.

**Cause:** Persistence counter not decrementing, or hunt distance not exhausting.

**Diagnostic:** Check persist and hunt_dist values. Verify they decrement each hunt() call.

**Fix:** Ensure persistence decrements after each tracking attempt and hunting stops when exhausted.

### Faction Mobs Attack Wrong Targets

**Symptom:** Cult or Brotherhood mobs attack allies or ignore enemies.

**Cause:** Incorrect zone check (inLogrus/inBrightmoon) or faction detection (isCult/isBrother/isSnake).

**Diagnostic:** Verify mob's faction flags and current zone. Check for trade pass item (vnum 8879) on Snake faction players in Logrus.

**Fix:** Ensure factionAggroCheck() correctly identifies mob's faction and territory, and respects trade pass immunity.

### Mob Not Attacking Players

**Symptom:** Mob with seemingly aggressive settings ignores valid targets.

**Cause:** Multiple possible causes in the aggression chain.

**Diagnostic:** Check anger and malice values (4*anger + 5*malice must exceed 450 without ACT_AGGRESSIVE). Verify not utility/guild mob. Check karma (high player karma may exceed mob intelligence + anger threshold). Check level difference guards. For wimpy mobs, verify target is sleeping.

**Fix:** Adjust emotional attributes, verify mob type flags, or adjust karma/level thresholds as appropriate.

### Hunting Target Lost

**Symptom:** Mob stops pursuing before reaching target.

**Cause:** Multiple conditions can terminate hunting prematurely.

**Diagnostic:** Check persistence value (zero triggers return home). Verify hunt_dist not exhausted. Check if target entered peaceful room. Verify vision requirements met. Check target's SKILL_CONCEALMENT level. Verify target still in world.

**Fix:** Address the specific condition: adjust persistence, ensure vision, reduce concealment, or validate target existence.

### Mob Not Developing Hatred

**Symptom:** Mob never adds attacker to hate list despite taking damage.

**Cause:** developHatred() conditions not met.

**Diagnostic:** Verify developHatred() called from damage handlers. Check patience formula vs HP percentage. Check level difference impact. Verify ACT_IMMORTAL not set.

**Fix:** Adjust mob level or patience thresholds, or remove ACT_IMMORTAL flag.

### Target Selection Ignoring Wounded

**Symptom:** senseWimps() doesn't prioritize low-HP targets.

**Cause:** Mob not meeting requirements or scoring overridden.

**Diagnostic:** Verify mob level >= 15. Check wounded threshold (<30% HP). Verify canSee() succeeds for target.

**Fix:** Ensure mob meets level requirement and visibility conditions.

### Response Not Executing

**Symptom:** Mob doesn't react to player actions despite having responses.

**Cause:** Response system not properly initialized or matched.

**Diagnostic:** Verify loadResponses() called during mob creation. Check mobresponses database table for vnum. Verify cmd type matches. Check args pattern matching.

**Fix:** Ensure database entries exist and patterns match triggering actions.

### Cleric Not Using Hunting Magic

**Symptom:** High-level cleric mob walks instead of teleporting.

**Cause:** Requirements not met for spell-based pursuit.

**Diagnostic:** Verify CLASS_CLERIC and level >= 30. Check 20% probability (may just be unlucky). Verify target level >= 15. Check mana availability.

**Fix:** Ensure class, level, and mana requirements are met; probability is working as designed.

### Mob Returning Home Instead of Hunting

**Symptom:** Mob abandons pursuit and navigates back to birth room.

**Cause:** Hunting conditions exhausted.

**Diagnostic:** Check persistence (zero or negative triggers return). Verify hunt_dist not exhausted. Ensure oldRoom is valid and pathable. Check ACT_SENTINEL not blocking return movement.

**Fix:** Increase initial persistence/distance or remove blocking conditions.
