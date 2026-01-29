---
title: Visibility and Scanning System
description: Visibility in SneezyMUD is a bidirectional system where observers have an eyeSight value representing their ability to see, while targets have a visibility value representing how hard they are to see. This document covers visibility mechanics, room observation, scanning, inventory display, and the consider command.
keywords: eyeSight, visibility, canSee, canSeeMe, lookRoom, doScan, doInventory, doEquipment, doConsider, pitchBlackDark, AFF_BLIND, AFF_TRUE_SIGHT, AFF_CLARITY, AFF_INVISIBLE, infravision, SKILL_SEARCH, list_in_heap, clearpath, list_thing_in_room
category: Important Systems
related:
  - room-environment.md
  - affects-system.md
  - skill-combat.md
  - tracking-hunting.md
  - stats-attributes.md
last_updated: 2026-01-29
source_files:
  - code/code/cmd/cmd_look.cc
  - code/code/misc/show.cc
  - code/code/misc/range.cc
  - code/code/task/task_search.cc
  - code/code/cmd/cmd_consider.cc
  - code/code/misc/info.cc
  - code/code/misc/utility.cc
  - code/code/misc/doors.cc
---

# Visibility and Scanning System

This document describes the visibility mechanics, room observation, scanning, inventory display, and the consider command in SneezyMUD.

## Overview

Visibility in SneezyMUD is a bidirectional system: observers have an **eyeSight** value representing their ability to see, while targets have a **visibility** value representing how hard they are to see. An observer can see a target when `eyeSight >= visibility`.

**Key files:**
- `code/code/cmd/cmd_look.cc` - Room and object look commands
- `code/code/misc/show.cc` - Display functions for beings and objects
- `code/code/misc/range.cc` - Scan and distance visibility
- `code/code/task/task_search.cc` - Secret door detection
- `code/code/cmd/cmd_consider.cc` - Consider command
- `code/code/misc/info.cc` - Inventory and equipment display
- `code/code/misc/utility.cc` - Core canSee/visibility functions

## Room Look System

### Basic Room Display

When a player enters a room or types `look` with no arguments, `TBeing::lookRoom()` (cmd_look.cc:207-279) executes:

1. Send GMCP room data to client
2. Draw automap if enabled (`AUTO_MAP`)
3. Display room name via `sendRoomName()`
4. Display room description via `sendRoomDesc()` (unless `PLR_BRIEF` set)
5. Describe weather and ground conditions
6. List exits via `listExits()`
7. Update tracking if hunting
8. List beings and objects via `list_thing_in_room()`

### Brief vs Verbose Mode

| Mode | Flag | Room Description | Exit Format |
|------|------|------------------|-------------|
| Verbose | Default | Full description shown | "Obvious exits lead..." |
| Brief | `PLR_BRIEF` | Description suppressed | "[Exits: N E S]" |

The `look room` command (keyword 9) always shows the full description regardless of brief mode via `TBeing::lookAtRoom()` (cmd_look.cc:281-339).

### Darkness Effects

Darkness is checked early in `TBeing::doLook()` (cmd_look.cc:454-470):

```cpp
if (roomp->pitchBlackDark() && !isImmortal() && (visionBonus <= 0) &&
    !(roomp->getRoomFlags() & ROOM_ALWAYS_LIT) &&
    !isAffected(AFF_TRUE_SIGHT) && !isAffected(AFF_CLARITY)) {
  lookDark();
  return;
}
```

**pitchBlackDark():** Returns true when room light level <= 0.

**lookDark()** (cmd_look.cc:62-73) shows "It is very dark in here..." but still reveals:
- Beings visible via infravision or glow effects
- Glowing objects (`ITEM_GLOW`)

### Blindness

Blindness completely blocks vision unless countered:

| Condition | Can See? |
|-----------|----------|
| `AFF_BLIND` alone | No |
| `AFF_BLIND` + `AFF_TRUE_SIGHT` | Yes |
| `AFF_BLIND` + `AFF_CLARITY` | Yes |

## Vision System

### eyeSight() Calculation

**File:** `code/code/misc/utility.cc:1391-1422`

The observer's ability to see is calculated as:

```
eyeSight = visionBonus + raceVisionBonus + lightLevel + spellBonuses - weatherPenalties
```

| Component | Value |
|-----------|-------|
| `visionBonus` | Per-character modifier |
| Race bonus | Varies by race (e.g., elves +5) |
| `AFF_TRUE_SIGHT` or `AFF_CLARITY` | +25 |
| Room light level | 0-25 (varies by time/weather) |
| Indoor penalty | `-lightLevel/2` |
| Rain | -1 |
| Snow | -2 |
| Lightning storm | -1 |

### visibility() Calculation

**File:** `code/code/misc/utility.cc:1342-1389`

The target's difficulty to be seen:

```
visibility = canBeSeen + hideBonus + equipmentBonus + environmentBonus
```

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

### canSee() Logic Flow

**File:** `code/code/misc/utility.cc:832-923`

The core visibility check in `TBeing::canSeeMe()`:

1. **Immortal invis level check** - Hide immortals from lower-level players
2. **Immortal observer** - Always succeeds (except for higher invis levels)
3. **Self-visibility** - Always can see yourself
4. **Invisibility check** - `AFF_INVISIBLE` or shadow walk in dim light
5. **True Sight/Clarity** - Bypass all further checks
6. **Blindness** - Fail if blind without true sight
7. **Sanctuary glow** - Always visible
8. **Personal light** - Visible if carrying light
9. **Room always lit** - Bypass light checks
10. **Infravision** - Bonus against warm-blooded creatures
11. **Final comparison** - `eyeSight > visibility`

## Exit Display and Search

### Exit Listing

**File:** `code/code/misc/info.cc:160-400`

The `listExits()` function shows available exits with color coding:

| Exit Type | Brief Color | Verbose Color |
|-----------|-------------|---------------|
| Normal open | Purple | Purple |
| Door (open) | Bold variant | Blue |
| Door (closed) | Asterisk prefix | Red (immortal only) |
| Fire sector | Red | Red |
| Air sector | Cyan | Cyan |
| Water sector | Blue | Blue |

**Secret door hints:** Players with `SKILL_SEARCH` have a passive chance to notice something unusual:

```cpp
chance = max(0, getSkillValue(SKILL_SEARCH));
if (getRace() == RACE_ELVEN) chance += 25;
if (getRace() == RACE_GNOME) chance += perception + level/2;
if (getRace() == RACE_DWARF && indoors) chance += level/2 + 10;

if (::number(1, 1000) < chance)
  sendTo("You suspect something out of the ordinary here.");
```

### Look Through Doors

**File:** `code/code/misc/doors.cc:766-784`

`canSeeThruDoor()` determines if you can see into the next room:

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

### SKILL_SEARCH Task

**File:** `code/code/task/task_search.cc:13-184`

Active searching for secret doors is a task-based skill:

**Mechanics:**
- Costs 3 movement per direction searched
- Searches all 10 directions sequentially
- Skips directions with existing visible exits
- Skips ceiling if room has no height
- Learning opportunity every 3 directions searched

**Success conditions:**
```cpp
if (tsSuccess &&                              // Skill check passed
    IS_SET(fdd->condition, EXIT_SECRET) &&    // Exit is secret
    IS_SET(fdd->condition, EXIT_CLOSED) &&    // Exit is closed
    !fdd->keyword.empty() &&                  // Door has a name
    fdd->keyword != "_unique_door_") {        // Not a special proc door
  // Reveal the secret door
}
```

## Scan Command

**File:** `code/code/misc/range.cc:979-1146`

The `doScan()` command peers in one or all directions to spot distant beings.

### Scan Range Calculation

```
max_range = 15 - terrain_thickness + visionBonus/10 + race_LOS
```

**Weather modifiers:**
| Weather | Range Modifier |
|---------|----------------|
| Snow | -3 |
| Rain | -2 |
| Cloudy/Fog | -1 |
| Clear | +1 |

**Movement cost:**
- All directions: 10 movement
- Single direction: 2 movement

### Range Descriptions

| Rooms Away | Description |
|------------|-------------|
| 0 | "right here" |
| 1 | "immediately" |
| 2 | "nearby" |
| 3 | "a short ways" |
| 4 | "not too far" |
| 5 | "a ways" |
| 6-7 | "quite a ways" |
| 8-9 | "way off" |
| 10-11 | "far" |
| 12-14 | "way way off" |
| 15-17 | "real far" |
| 18-19 | "very far" |
| 20+ | "on the horizon" |

### Crowd Hindrance

Scanning stops in a direction after spotting `5 + visionBonus/3` beings:

```cpp
if (nfnd > (5 + visionBonus / 3)) {
  sendTo("The crowd hinders you from seeing any further <direction>.");
  break;
}
```

### clearpath() Check

**File:** `code/code/misc/range.cc:951-972`

Scan requires a clear path between rooms:

```cpp
int clearpath(int room, dirTypeT dir) {
  if (dir >= MAX_DIR || !rp || !rp->dir_option[dir])
    return FALSE;
  if (rp->dir_option[dir]->to_room < 1)
    return FALSE;
  if (IS_SET(rp->dir_option[dir]->condition, EXIT_CLOSED))
    return FALSE;  // Cannot scan through closed doors
  return (rp->dir_option[dir]->to_room);
}
```

## Inventory Display

**File:** `code/code/misc/info.cc:2578-2627`

### doInventory()

The inventory command shows carried items:

1. **Blindness check** - Requires `AFF_TRUE_SIGHT`, `AFF_CLARITY`, or not blind
2. **Item listing** - Via `list_in_heap()` which groups similar items
3. **Filtering** - Optional argument filters by item name
4. **Capacity display** - Shows volume% and weight% (level 11+)

```cpp
sendTo(format("\n\r%3.f%c volume, %3.f%c weight.\n\r") %
  (((float)getCarriedVolume() / (float)carryVolumeLimit()) * 100.0) % '%' %
  (((float)getCarriedWeight() / (float)carryWeightLimit()) * 100.0) % '%');
```

### list_in_heap()

**File:** `code/code/misc/show.cc:371-406`

Groups similar items using `isSimilar()` and displays with counts:

```
A short sword [3]
A leather jerkin
A healing potion [5]
```

**Parameters:**
- `show_all` - Recurse into containers
- `perc` - Percentage chance to show each item (used for spy skill)

### Immortal Inventory Check

Immortals can view other players' inventories:
```
inventory <playername> [filter]
```

## Equipment Display

**File:** `code/code/misc/info.cc:2629-2746`

The `doEquipment()` command shows worn items:

1. **Weight header** - "You are using X pounds of equipment:"
2. **Slot iteration** - MIN_WEAR to MAX_WEAR
3. **Paired item handling** - Skip duplicate display for paired items
4. **Tattoo integration** - Shows tattoos in empty slots
5. **Damaged filter** - `equipment damaged` shows only damaged items

**Damaged equipment check:**
```cpp
if (tobj->getMaxStructPoints() != tobj->getStructPoints()) {
  // Show this item
}
```

## Consider Command

**File:** `code/code/cmd/cmd_consider.cc:18-343`

### Self-Consider

Evaluates your own combat readiness:

**Armor assessment:**
```cpp
int armor = 1000 - getArmor();
int suggest = suggestArmor();
diff = suggest - armor;
```

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

**Visibility assessment:** Based on `visibility()` value (higher = more hidden).

**Noise assessment:** Based on `noise()` value (higher = louder).

### Monster Consider

Compares your level to the monster's effective level:

```cpp
diff = (int)(tmon->getRealLevel() + 0.5) - GetMaxLevel();
```

| Level Difference | Message |
|------------------|---------|
| <= -15 | "Shall I tie both hands behind your back?" |
| <= -10 | "Why bother???" |
| <= -6 | "Don't strain yourself." |
| <= -3 | "Piece of cake." |
| <= -2 | "Odds are in your favor." |
| <= -1 | "You have a slight advantage." |
| 0 | "A fair fight." |
| <= 1 | "Doesn't look that tough..." |
| <= 2 | "Cross your fingers." |
| <= 3 | "Cross your fingers and hope..." |
| <= 6 | "I hope you have a good plan!" |
| <= 10 | "Bring friends." |
| <= 15 | "You and what army??" |
| <= 30 | "You'll win if they never hit you." |
| > 30 | "There are better ways to suicide." |

### Lore Skills

Characters with adventuring discipline gain additional information:

| Skill | Creature Type | Information |
|-------|---------------|-------------|
| `SKILL_CONS_ANIMAL` | Animals | Race identification |
| `SKILL_CONS_VEGGIE` | Vegetables | Race identification |
| `SKILL_CONS_DEMON` | Demons | Race identification |
| `SKILL_CONS_REPTILE` | Reptiles | Race identification |
| `SKILL_CONS_UNDEAD` | Undead | Race identification |
| `SKILL_CONS_GIANT` | Giants | Race identification |
| `SKILL_CONS_PEOPLE` | Humanoids | Race identification |
| `SKILL_CONS_OTHER` | Monsters | Race identification |

**Skill-gated information:**

| Learning Level | Information Revealed |
|----------------|---------------------|
| > 5 | Estimated max HP ratio |
| > 20 | Estimated armor class |
| > 40 | Estimated number of attacks |
| > 60 | Estimated damage per attack |

### Trophy Integration

Consider shows experience modifier based on trophy count:
```cpp
auto count = trophy->getCount(tmon->mobVnum());
if (count > 0) {
  sendTo("You will gain %s experience when fighting %s.");
} else {
  sendTo("You have never fought %s and will gain %s experience.");
}
```

## Common Gotchas

### Visibility vs canSee

- `visibility()` returns how hard something is to see (higher = more hidden)
- `eyeSight()` returns how well someone can see (higher = better vision)
- `canSee()` is the actual check: returns true if observer can see target

### Infravision Limitations

- Does NOT work on cold-blooded creatures
- Returns FALSE for cold-blooded rather than defaulting to normal vision
- Bonus varies by time of day and environment

### Scan vs Look Direction

- `look <direction>` shows the adjacent room's contents
- `scan <direction>` shows beings in multiple rooms but not objects
- `scan` cannot see through closed doors (`clearpath()` check)

### Secret Door Detection

- Passive detection (in `listExits()`) gives vague hints
- Active `search` task reveals exact door location and name
- Some doors (`_unique_door_`) cannot be revealed by search

### Consider Accuracy

- Level comparison uses `getRealLevel()` which includes spell effects
- HP/AC/damage estimates use `GetApprox()` which adds variance based on skill
- Trophy count affects displayed experience modifier

## Related Documentation

- [Room and Environment System](room-environment.md) - Light levels, weather effects
- [Affects System](affects-system.md) - AFF_BLIND, AFF_TRUE_SIGHT, etc.
- [Skill Combat](skill-combat.md) - SKILL_SEARCH mechanics
- [Tracking and Hunting](tracking-hunting.md) - Track integration with look
- [Stats and Attributes](stats-attributes.md) - Perception effects on detection
