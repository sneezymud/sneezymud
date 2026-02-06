---
title: Visibility and Scanning System
description: Bidirectional visibility comparison system determining perception through eyeSight vs visibility checks across environmental and character factors.
category: important
keywords: [sight mechanics, perception, darkness, secret doors, crowd hindrance, lore skills]
primary_symbols:
  functions: [eyeSight, visibility, canSee, canSeeMe, lookRoom, doScan, doInventory, doEquipment, doConsider, list_in_heap, clearpath, listExits]
  classes: [TBeing, TRoom, TObj]
  enums: [AFF_BLIND, AFF_TRUE_SIGHT, AFF_CLARITY, AFF_INVISIBLE, AFF_HIDE, AFF_SANCTUARY, SKILL_SEARCH, SKILL_CONS_ANIMAL, SKILL_CONS_VEGGIE, SKILL_CONS_DEMON, SKILL_CONS_REPTILE, SKILL_CONS_UNDEAD, SKILL_CONS_GIANT, SKILL_CONS_PEOPLE, SKILL_CONS_OTHER, EXIT_CAVED_IN, EXIT_DESTROYED, EXIT_CLOSED, EXIT_SECRET, DOOR_NONE, DOOR_PORTCULLIS, DOOR_GRATE, DOOR_SCREEN, ROOM_ALWAYS_LIT, ITEM_GLOW, PLR_BRIEF, AUTO_MAP]
---

## Overview

Can a player see that invisible assassin sneaking up behind them? Can the elf spot enemies through the forest canopy while the dwarf peers down a dark tunnel? Visibility in SneezyMUD answers these questions through a bidirectional comparison system.

Every visibility check compares two values: an observer's **eyeSight** (how well they can see) against a target's **visibility** (how hard they are to see). When eyeSight exceeds visibility, the observer sees the target. This simple comparison underlies every moment of perception in the game - from glancing around a room to scanning distant horizons to evaluating potential foes.

The system accounts for environmental conditions (lighting, weather, terrain), character abilities (race, spells, equipment), and active concealment (hiding, sneaking, invisibility). A character with infravision gains an advantage in darkness against warm-blooded creatures. Rain makes it harder to see but easier to hide. Forest terrain provides natural concealment to those familiar with it.

When you enter a room, the system evaluates each being and object present against your eyeSight. When you scan a direction, it traces a path through multiple rooms, stopping at doors or crowds. When you consider an opponent, it compares your combat readiness against theirs and reports the odds.

---

## Patterns

### Visibility Checks

**Always use `canSee()` for visibility decisions, not direct eyeSight/visibility comparison.** The `canSee()` function handles special cases like immortal invisibility levels, self-visibility, and spell interactions that raw comparison would miss.

**Never assume infravision provides general darkness vision.** Infravision only detects warm-blooded creatures. It returns false for cold-blooded targets rather than falling back to normal vision. Code that assumes infravision is a general darkness solution will fail silently.

**Always check blindness before detailed visibility operations.** Blindness completely blocks vision unless countered by `AFF_TRUE_SIGHT` or `AFF_CLARITY`. Performing expensive visibility calculations on a blind character wastes cycles.

### Room Display

**Never show beings or objects the observer cannot see.** Before adding anything to room display output, verify with `canSee()`. This applies to exit listings, object displays, and being lists.

**Always respect `PLR_BRIEF` for room descriptions but not for `look room`.** The explicit `look room` command always shows full descriptions regardless of brief mode.

### Scanning

**Never scan through closed doors.** The `clearpath()` function enforces this. Attempting to bypass it creates inconsistent game behavior where players see things they should not.

**Always account for crowd hindrance when scanning.** After spotting several beings in a direction, scanning stops with a crowd message. The threshold is `5 + visionBonus/3`. This prevents unlimited information gathering in populated areas.

**Always deduct movement cost for scanning.** Scanning all directions costs 10 movement; scanning a single direction costs 2. Attempting to scan with insufficient movement should fail gracefully.

### Secret Doors

**Never reveal special proc doors through search.** Doors with keyword `_unique_door_` are programmatically controlled and should not be revealed by the search skill. Check this condition before revealing.

**Always distinguish passive detection from active search.** Passive detection in `listExits()` provides only vague hints ("something unusual"). Active search reveals the exact door location and name.

### Consider Command

**Always use `getRealLevel()` for level comparisons.** This includes spell effects that modify effective level. Using raw level produces inaccurate assessments.

**Always apply variance to HP/AC/damage estimates based on skill.** The `GetApprox()` function adds appropriate uncertainty. Characters with low lore skills receive less accurate information.

---

## Reference

### Symbol Quick Reference

| Symbol | Type | Purpose |
|--------|------|---------|
| `eyeSight()` | function | Calculate observer's ability to see |
| `visibility()` | function | Calculate target's difficulty to be seen |
| `canSee()` | function | Check if observer can see target |
| `canSeeMe()` | function | Check if this being is visible to observer |
| `lookRoom()` | function | Display room contents to player |
| `doScan()` | function | Peer in direction(s) for distant beings |
| `doInventory()` | function | Display carried items |
| `doEquipment()` | function | Display worn items |
| `doConsider()` | function | Evaluate combat readiness |
| `list_in_heap()` | function | Group and display similar items |
| `clearpath()` | function | Check if scan can proceed through exit |
| `listExits()` | function | Display available exits with color coding |
| `TBeing` | class | Base class for all characters |
| `TRoom` | class | Room container with light level |
| `TObj` | class | Base class for objects |

### Affects Impacting Vision

| Affect | Effect |
|--------|--------|
| `AFF_BLIND` | Blocks all vision |
| `AFF_TRUE_SIGHT` | +25 eyeSight, counters blindness |
| `AFF_CLARITY` | +25 eyeSight, counters blindness |
| `AFF_INVISIBLE` | Increases visibility (harder to see) |
| `AFF_HIDE` | +5 + level/2 to visibility when not fighting |
| `AFF_SANCTUARY` | Forces visibility (glow overrides concealment) |

### EyeSight Modifiers

| Component | Value |
|-----------|-------|
| `visionBonus` | Per-character modifier |
| Race bonus | Varies by race (e.g., elves +5) |
| `AFF_TRUE_SIGHT` or `AFF_CLARITY` | +25 |
| Room light level | 0-25 (varies by time/weather) |
| Indoor penalty | -lightLevel/2 |
| Rain | -1 |
| Snow | -2 |
| Lightning storm | -1 |

### Visibility Modifiers

| Component | Value |
|-----------|-------|
| `canBeSeen` | Base value (sneak sets this) |
| `AFF_HIDE` (not fighting) | +5 + level/2 |
| Home terrain bonus | +5 |
| Background bonus | +5 |
| Shadowy equipment | Proportional to coverage |
| Forest sector | +2 |
| Rain | +1 |
| Snow | -2 (more visible) |
| Lightning | -1 |

### Scan Range Modifiers

| Weather | Range Modifier |
|---------|----------------|
| Snow | -3 |
| Rain | -2 |
| Cloudy/Fog | -1 |
| Clear | +1 |

### Scan Distance Descriptions

| Rooms Away | Description |
|------------|-------------|
| 0 | "right here" |
| 1 | "immediately" |
| 2 | "nearby" |
| 3 | "a short ways" |
| 4 | "not too far" |
| 5 | "a ways" |
| 6 | "quite a ways" |
| 7 | "way off" |
| 8 | "way off" |
| 9 | "far" |
| 10 | "far" |
| 11 | "way way off" |
| 12 | "way way off" |
| 13-14 | "real far" |
| 15-16 | "very far" |
| 17-18 | "extremely far" |
| 19 | "on the horizon" |
| 20+ | "on the horizon" |

### Door See-Through Types

| Condition | Can See Through |
|-----------|-----------------|
| `EXIT_CAVED_IN` | No |
| `EXIT_DESTROYED` | Yes |
| Not closed | Yes |
| `DOOR_NONE` | Yes |
| `DOOR_PORTCULLIS` | Yes |
| `DOOR_GRATE` | Yes |
| `DOOR_SCREEN` | Yes |
| Other door types | No (when closed) |

### Exit Color Coding

| Exit State | Brief Mode | Verbose Mode |
|------------|------------|--------------|
| Normal open | Purple | Purple |
| Open door | Bold purple | Blue |
| Closed door | Asterisk prefix | Red (immortal only) |
| Fire sector | Red | Red |
| Air sector | Cyan | Cyan |
| Water sector | Blue | Blue |

### Consider Level Messages

| Level Difference | Message |
|------------------|---------|
| <= -15 | "Shall I tie both hands behind your back, or just one?" |
| <= -10 | "Why bother???" |
| <= -6 | "Don't strain yourself." |
| <= -3 | "Piece of cake." |
| <= -2 | "Odds are in your favor." |
| <= -1 | "You have a slight advantage." |
| 0 | "A fair fight." |
| <= 1 | "$E doesn't look that tough..." |
| <= 2 | "Cross your fingers." |
| <= 3 | "Cross your fingers and hope they don't get broken." |
| <= 6 | "I hope you have a good plan!" |
| <= 10 | "Bring friends." |
| <= 15 | "You and what army??" |
| <= 30 | "You'll win if $E never hits you." |
| > 30 | "There are better ways to suicide." |

### Consider Armor Assessments

| Difference | Assessment |
|------------|------------|
| >= 210 | "laughably pathetic" |
| >= 160 | "horrid" |
| >= 90 | "bad" |
| >= 50 | "poor" |
| >= 30 | "weak" |
| >= 10 | "o.k." |
| > 0 | "near perfect" |
| == 0 | "perfect" |
| >= -30 | "good" |
| >= -40 | "very good" |
| >= -80 | "great" |
| >= -150 | "fantastic" |
| >= -200 | "superb" |
| < -200 | "incredibly good" |

### Lore Skills

| Skill | Creature Type |
|-------|---------------|
| `SKILL_CONS_ANIMAL` | Animals |
| `SKILL_CONS_VEGGIE` | Vegetables |
| `SKILL_CONS_DEMON` | Demons |
| `SKILL_CONS_REPTILE` | Reptiles |
| `SKILL_CONS_UNDEAD` | Undead |
| `SKILL_CONS_GIANT` | Giants |
| `SKILL_CONS_PEOPLE` | Humanoids |
| `SKILL_CONS_OTHER` | Monsters |

### Lore Information Thresholds

| Learning Level | Information Revealed |
|----------------|---------------------|
| > 5 | Estimated max HP ratio |
| > 20 | Estimated armor class |
| > 40 | Estimated number of attacks |
| > 60 | Estimated damage per attack |

### Key Files

| File | Purpose |
|------|---------|
| cmd_look.cc | Room and object look commands |
| show.cc | Display functions for beings and objects |
| range.cc | Scan and distance visibility |
| task_search.cc | Secret door detection |
| cmd_consider.cc | Consider command |
| info.cc | Inventory and equipment display |
| utility.cc | Core canSee/visibility functions |
| doors.cc | Door transparency checks |

---

## Implementation

### EyeSight Calculation

The `eyeSight()` function in utility.cc calculates an observer's ability to see. It starts with the character's base `visionBonus` (a direct member access) and adds racial bonuses that vary by race via `getMyRace()->visionBonus`. True sight and clarity affects each add +25 to the total.

Room lighting contributes a variable amount based on time of day and weather conditions, ranging from 0 to 25. Indoor rooms receive a penalty of half the light level to simulate reduced ambient light. Weather conditions apply further penalties: rain reduces eyeSight by 1, snow by 2, and lightning storms by 1.

### Visibility Calculation

The `visibility()` function in utility.cc determines how difficult a target is to see. Higher values mean harder to spot. The base is `canBeSeen`, which sneak skill manipulates.

When a character is hiding (has `AFF_HIDE`) and not currently fighting, visibility increases by 5 plus half their level. Home terrain matching adds +5, as does having equipment with background bonuses. Shadowy equipment contributes proportionally to body coverage via iteration over worn items checking `ITEM_SHADOWY` flags.

Environmental factors matter: forest sectors add +2, rain adds +1 (easier to hide), but snow subtracts 2 (footprints visible) and lightning subtracts 1 (intermittent illumination).

### CanSee Logic Flow

The `canSeeMe()` function in utility.cc implements the complete visibility check. It proceeds through a priority-ordered series of checks:

First, immortal invisibility levels are checked. Immortals set at higher invisibility levels are hidden from lower-level players regardless of other factors.

Immortal observers pass most checks automatically, though they still cannot see higher-level immortals who are invisible.

Self-visibility always succeeds - you can always see yourself.

Invisibility from `AFF_INVISIBLE` or shadow walking in dim light blocks visibility unless countered.

True sight and clarity bypass all remaining checks, including blindness.

Blindness without true sight or clarity fails immediately.

Sanctuary glow makes the target always visible regardless of other factors.

Personal light sources (carried lit objects, detected via `getLight() > 0`) make the carrier visible.

Rooms flagged as `ROOM_ALWAYS_LIT` bypass darkness checks.

Infravision provides a bonus against warm-blooded creatures in darkness, but returns false for cold-blooded targets.

Finally, the numeric comparison occurs: eyeSight must exceed visibility for success.

### Room Look Flow

When a player enters a room or types `look`, the `lookRoom()` function in cmd_look.cc executes. It first sends GMCP room data to the client for map integration, then draws the automap if enabled via `AUTO_MAP`.

The room name appears via `sendRoomName()`, followed by the room description via `sendRoomDesc()` - unless the player has `PLR_BRIEF` set, in which case the description is suppressed.

Weather and ground conditions are described when relevant. Exit listing via `listExits()` shows available directions with color coding: normal exits in purple, doors in blue variants (open) or red (closed, immortal only), and special sectors (fire, air, water) in thematic colors.

If the player is hunting, tracking updates occur. Finally, `list_thing_in_room()` populates the room with visible beings and objects. This function separates beings from objects, processing each through distinct display paths. Being visibility filters through `canSee()` before inclusion. Object display calls `showObjectsToChar()` which groups items via `list_in_heap()`. Being display calls `showMultTo()` for each visible character or creature.

### Darkness Handling

The `doLook()` function checks darkness early. If `pitchBlackDark()` returns true (room light level <= 0), the player is not immortal, has no vision bonus, the room is not always lit, and lacks true sight or clarity, the `lookDark()` function executes instead of normal look.

Dark rooms still reveal some information: beings visible via infravision or with glow effects appear, as do objects with the `ITEM_GLOW` flag. This provides minimal situational awareness even in complete darkness.

### Exit Listing and Secret Detection

The `listExits()` function in info.cc displays available exits. Brief mode shows a compact bracketed list, while verbose mode uses prose descriptions. The function iterates through `MAX_DIR` directions, checking `dir_option` existence and applying ANSI color codes based on exit state.

Secret door passive detection occurs during exit listing. Players with `SKILL_SEARCH` have a passive chance to notice something unusual. The base chance equals the skill value. Elves gain +25. Gnomes add perception plus half their level. Dwarves indoors add half their level plus 10.

If a random roll under 1000 succeeds against this chance, the player receives a vague hint: "You suspect something out of the ordinary here." This does not reveal the door's location or name.

### Active Search Task

The `SKILL_SEARCH` task in task_search.cc provides active secret door detection. It costs 3 movement per direction searched and processes all 10 directions sequentially, skipping directions with existing visible exits (via `canSeeThruDoor()`) and skipping ceiling if the room has no height.

A learning opportunity occurs every 3 directions searched. For success, the skill check must pass (via `bSuccess()` with `SKILL_SEARCH`), the exit must have `EXIT_SECRET` set, be closed (`EXIT_CLOSED`), have a keyword, and the keyword must not be `_unique_door_`. Meeting all conditions reveals the door with its exact name and location, and removes the `EXIT_SECRET` flag.

### Scan Command

The `doScan()` command in range.cc peers in one or all directions to spot distant beings. Range calculation starts at 15, subtracts terrain thickness, adds visionBonus/10, and adds racial line-of-sight bonus.

Weather modifies range: snow -3, rain -2, cloudy/fog -1, clear +1. Movement cost is 10 for all directions or 2 for a single direction.

The scan iterates through rooms along the direction, using `clearpath()` to verify the path remains open. Closed doors block the scan. Each room's beings are checked with `canSee()`, and visible beings are reported with distance descriptions.

Crowd hindrance stops scanning in a direction after spotting `5 + visionBonus/3` beings. A message informs the player that the crowd prevents seeing further.

### Door Transparency

The `canSeeThruDoor()` function in doors.cc determines if you can see into the next room through an exit. The function performs ordered condition checks: `EXIT_CAVED_IN` immediately returns false. `EXIT_DESTROYED` returns true. Open exits (when `EXIT_CLOSED` is not set) and specific door types (none, portcullis, grate, screen) allow vision. Other closed door types block vision.

### Inventory Display

The `doInventory()` function in info.cc shows carried items. It first checks for blindness (requires true sight, clarity, or not blind). Items are listed via `list_in_heap()`, which groups similar items using `isSimilar()` and displays counts in brackets.

The `list_in_heap()` function creates a temporary item list, iterating through all provided objects. For each object, `isSimilar()` comparisons check against already-grouped items. Matches increment the group count. New unique items create new groups. Display generation formats item names with bracketed counts for quantities exceeding one. The `show_all` parameter triggers recursive descent into container contents.

An optional argument filters items by name. At level 11 and above, capacity display shows volume and weight percentages relative to carry limits.

Immortals can view other players' inventories with the syntax `inventory <playername> [filter]`.

### Equipment Display

The `doEquipment()` function in info.cc shows worn items. It displays a weight header, then iterates through all wear slots from MIN_WEAR to MAX_WEAR. Paired items skip duplicate display. Empty slots can show tattoos if present.

The `equipment damaged` variant filters to show only damaged items - those where current structure points differ from maximum.

### Consider Command

The `doConsider()` function in cmd_consider.cc evaluates combat readiness. Self-consider assesses armor (comparing actual armor to suggested armor for level via `suggestArmor()`), visibility (based on `visibility()` value), and noise.

Monster consider compares the player's level to the monster's effective level using `getRealLevel()`, which includes spell effects. The level difference maps to messages ranging from dismissive ("Shall I tie both hands behind your back?") to dire ("There are better ways to suicide.").

Characters with appropriate lore skills (animal, veggie, demon, reptile, undead, giant, people, other) gain race identification for matching creature types. Higher skill learning levels unlock additional information: HP ratio above 5, armor class above 20, attack count above 40, damage per attack above 60. All estimates apply `GetApprox()` variance based on skill proficiency.

Trophy integration shows experience modifier based on how many times the player has killed this mob type (via `getCount()`). Players who have never fought the creature receive different messaging than those with trophy counts.

---

## Troubleshooting

### Player Reports Invisible Creatures Attacking

**Symptom:** Player claims to be attacked by something they cannot see.

**Likely cause:** Attacker has higher visibility than player's eyeSight. Common with sneaking/hiding creatures in dark rooms.

**Diagnostic approach:** Check player's eyeSight calculation (vision bonus, affects, room light). Check attacker's visibility calculation (hide level, terrain bonuses). Verify no missing affect checks in the canSee chain.

**Fix:** If intended behavior, explain to player. If bug, ensure canSee is called before attack messages reference the attacker by name.

### Invisibility Not Working as Expected

**Symptom:** Character with `AFF_INVISIBLE` is still being seen.

**Likely cause:** Observer has `AFF_TRUE_SIGHT` or `AFF_CLARITY`. Alternatively, target has `AFF_SANCTUARY` (glow overrides invisibility), is carrying a light source (detectable via `getLight() > 0`), or shadow walk conditions require dim lighting that is not present.

**Diagnostic approach:** Check observer for true sight/clarity affects. Check target for sanctuary, light sources. For shadow walk, verify lighting conditions.

**Fix:** If intended behavior (counter-spells working), explain to player. Otherwise verify affect flags are set correctly.

### Scan Reports Wrong Distance

**Symptom:** Scan distance descriptions do not match actual room count.

**Likely cause:** Range modifiers not applied correctly, or terrain thickness calculation differs from expected.

**Diagnostic approach:** Check weather conditions, vision bonus, race line-of-sight bonus. Verify terrain thickness values for rooms along path.

**Fix:** Trace range calculation in doScan, verify all modifiers apply in correct order.

### Scan Showing Nothing

**Symptom:** Scan reports no beings in a direction where beings exist.

**Likely cause:** Closed door blocking `clearpath()`, eyeSight below target visibility, or crowd hindrance already triggered.

**Diagnostic approach:** Validate `clearpath()` returns true by checking door states. Manually calculate eyeSight vs visibility for targets. Count previously displayed beings against crowd threshold.

**Fix:** Check maximum range calculation against actual room distance, considering terrain thickness and weather penalties.

### Search Never Finds Secret Door

**Symptom:** Player with high search skill cannot find a door that exists.

**Likely cause:** Door keyword is `_unique_door_` (programmatic door), door is not closed, or door lacks `EXIT_SECRET` flag.

**Diagnostic approach:** Check door definition in zone file. Verify keyword, exit flags, and closed state. Ensure sufficient movement points (3 per direction).

**Fix:** If door should be findable, ensure it has proper keyword and EXIT_SECRET. If programmatic, document that search cannot reveal it.

### Infravision Not Working

**Symptom:** Character with infravision cannot see in darkness.

**Likely cause:** Target is cold-blooded. Infravision only detects warm-blooded creatures.

**Diagnostic approach:** Check target's creature type and warm-blooded flag. Verify observer has infravision via racial traits or equipment. Check that lighting conditions permit infravision bonuses.

**Fix:** Infravision working as intended. Player needs light source or true sight to see cold-blooded targets in darkness.

### Consider Shows Wrong Difficulty

**Symptom:** Consider message does not match actual combat difficulty.

**Likely cause:** Mob has level-modifying affects, or uses special attacks not reflected in level. Consider only compares base levels.

**Diagnostic approach:** Check mob's getRealLevel() including affects. Compare to player's GetMaxLevel().

**Fix:** Consider is intentionally approximate. Special abilities, equipment, and skills create variance. This is design, not bug.

### Consider Information Missing

**Symptom:** Lore information not appearing for a creature.

**Likely cause:** Character lacks appropriate lore skill for creature type, or skill learning level below thresholds.

**Diagnostic approach:** Validate character has correct lore skill (animal, veggie, demon, etc.) for creature type. Check skill learning: 5 for health, 20 for armor, 40 for attacks, 60 for damage.

**Fix:** Player needs to improve appropriate lore skill. Special proc creatures may bypass normal consider mechanics.

### Crowd Hindrance Triggering Too Early

**Symptom:** Scan stops with crowd message after seeing few beings.

**Likely cause:** Player has low or negative vision bonus, reducing crowd threshold below expected.

**Diagnostic approach:** Check player's visionBonus. Threshold is `5 + visionBonus/3`.

**Fix:** Working as intended - impaired vision means crowds obscure view more easily.

### Darkness Not Blocking Vision

**Symptom:** Player can see in a room that should be dark.

**Likely cause:** Room has `ROOM_ALWAYS_LIT` flag, observer is immortal, observer has true sight/clarity, or room light level is above zero.

**Diagnostic approach:** Verify room light level via `getRoomLight()`. Check for `ROOM_ALWAYS_LIT` flag. Check observer immortal status and affects.

**Fix:** If room should be dark, remove `ROOM_ALWAYS_LIT` flag and verify time-of-day light calculation.

### Exit Colors Not Displaying

**Symptom:** Exits appear without color coding.

**Likely cause:** Client lacks ANSI color support, or descriptor color mode disabled.

**Diagnostic approach:** Validate ANSI support in client connection. Check descriptor color preferences. Verify `listExits()` ANSI sequence formatting.

**Fix:** Enable color in client and/or player preferences.
