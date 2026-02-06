---
title: Movement and Terrain Navigation
description: Directional movement, vertical climbing and falling, water environments with drowning, flight mechanics, and instant travel via portals and teleportation.
category: critical
keywords: [movement, terrain, navigation, flight, climbing, falling, swimming, drowning, portals, teleportation, doors]
primary_symbols:
  functions: [doMove, rawMove, validMove, checkFalling, crashLanding, checkDrowning, riverFlow, canClimb, canFly, isFlying, doFly, doLand, genericTeleport, enterMe, rawOpenDoor, has_key, fallKill]
  classes: [TBeing, TRoom, TPortal, roomDirData]
  enums: [DIR_NORTH, DIR_EAST, DIR_SOUTH, DIR_WEST, DIR_UP, DIR_DOWN, DIR_NORTHEAST, DIR_NORTHWEST, DIR_SOUTHEAST, DIR_SOUTHWEST, SECT_CITY, SECT_ROAD, SECT_PLAINS, SECT_GRASSLANDS, SECT_HILLS, SECT_FOREST, SECT_SWAMP, SECT_MOUNTAINS, SECT_UNDERWATER, SECT_MAKE_FLY, EXIT_CLOSED, EXIT_LOCKED, EXIT_SECRET, EXIT_DESTROYED, EXIT_NOENTER, EXIT_TRAPPED, EXIT_CAVED_IN, EXIT_WARDED, EXIT_SLOPED_UP, EXIT_SLOPED_DOWN, EXIT_JAMMED, DOOR_DOOR, DOOR_TRAPDOOR, DOOR_GATE, DOOR_PORTCULLIS, DOOR_DRAWBRIDGE, POSITION_FLYING, AFF_FLYING, AFF_SWIM, AFF_WATERBREATH, SPELL_FLY, SPELL_WATERBREATH, SPELL_AQUALUNG, SPELL_FEATHERY_DESCENT, SKILL_CATFALL, SKILL_SWIMMING, SKILL_HIKING, ROOM_NO_ESCAPE, ROOM_NO_PORTAL, ROOM_NO_MAGIC, ROOM_PRIVATE, ROOM_HAVE_TO_WALK, ROOM_ARENA, ROOM_DEATH, DELETE_THIS, DELETE_VICT]
---

# Movement and Terrain Navigation

## Overview

Movement in SneezyMUD transforms simple directional commands into a complex negotiation between characters and their environment. A character typing "north" triggers validation of exits, terrain compatibility, encumbrance, combat state, mount control, and movement point costs before anything happens.

The system integrates five distinct subsystems: basic directional movement, vertical climbing and falling, water environments with drowning, flight mechanics, and instant travel via portals and teleportation. Each subsystem can kill characters through environmental hazards, making proper death-flag handling critical.

Terrain costs create meaningful travel decisions. A forest path costs four times as much energy as a city street. Flying reduces all costs to a quarter. Swimming without the swim affect doubles water sector costs. These modifiers accumulate, so a dwarf (penalized in water) without swim training pays dramatically more than an elf with magical aid.

The movement system's most dangerous aspect is its death potential. Falling, drowning, and teleportation into hazardous rooms all return DELETE flags that must propagate up the call stack. Mishandling these flags causes use-after-free crashes when code continues operating on deleted characters.

## Patterns

### DELETE Flag Propagation

**Always check DELETE_THIS returns from movement functions.** Functions like `doMove()`, `checkFalling()`, `crashLanding()`, `checkDrowning()`, `riverFlow()`, and `genericTeleport()` can all kill characters. Check returns immediately and propagate upward.

**Never use IS_SET_DELETE() for reconcileDamage() death checks.** The damage function returns -1 as a sentinel for death, not a DELETE flag. Check `== -1` explicitly.

**Always propagate DELETE_THIS through mount falling chains.** When a rider falls off a mount, the subsequent crash landing can also return DELETE_THIS. Each level in the chain must check and propagate.

**Scheduler adapters convert DELETE_THIS flags to bool.** Adapters like `procCharDrowning` check `IS_SET_DELETE(rc, DELETE_THIS)` and return `true` to signal deletion, `false` to keep the object.

### Iterator Safety

**Always cache the next pointer before modifying linked structures.** River flow, rider chains, and follower processing can modify list linkage. Cache `t2 = t->nextRider` before any operations on `t`.

### Door Operations

**Remember that doors are bidirectional.** Opening, closing, locking, or unlocking a door affects both the current room's exit and the corresponding exit in the destination room. Both sides must be modified for consistency.

**Handle portcullis and drawbridge inversion.** Portcullis opens with "raise" and closes with "lower". Drawbridge opens with "lower" and closes with "raise". This inversion catches many developers.

**Validate reverse exit existence.** The back exit may not exist for one-way exits. Check that back is non-null before updating `back->condition` flags.

### Flight State Transitions

**Check actual flight ability when leaving flying sectors.** Characters in SECT_MAKE_FLY sectors receive automatic flight. When leaving, verify they have real flight capability (spell, racial, or affect) or they cannot proceed to air/vertical sectors.

**Never assume flight persists across sector boundaries.** The sector may grant or remove flight status automatically based on its type.

**Winged races require AFF_FLIGHTWORTHY.** The `doFly()` command checks that feathered races have been preened before allowing takeoff.

### Teleportation Safety

**Always validate room pointers after random teleportation.** The destination is selected randomly and the room pointer may be unexpected. Use `real_roomp()` to validate.

**Check DELETE_THIS after genericTeleport() before accessing the character.** Teleportation can place characters in dangerous rooms that kill them immediately.

### Mounted Movement

**Only the horse master can direct mounted movement.** Check via `riding->horseMaster()` before allowing the character to choose direction.

**Mount pays movement points, not rider.** The mount's movement pool is depleted; the rider pays only a fraction (0-33%).

### Water Mechanics

**Swimming success depends on character density.** Characters denser than water (weight/volume ratio) must actively swim. AFF_SWIM reduces movement costs; dwarf racial penalties add significant costs due to their density.

## Reference

### Direction Commands

| Direction | Aliases | Enum | Value |
|-----------|---------|------|-------|
| North | n | DIR_NORTH | 0 |
| East | e | DIR_EAST | 1 |
| South | s | DIR_SOUTH | 2 |
| West | w | DIR_WEST | 3 |
| Up | u | DIR_UP | 4 |
| Down | d | DIR_DOWN | 5 |
| Northeast | ne | DIR_NORTHEAST | 6 |
| Northwest | nw | DIR_NORTHWEST | 7 |
| Southeast | se | DIR_SOUTHEAST | 8 |
| Southwest | sw | DIR_SOUTHWEST | 9 |

### Terrain Movement Costs

| Sector Type | Cost | Notes |
|-------------|------|-------|
| City, Road | 1 | Paved, easy travel |
| Plains/Grasslands | 2-3 | Open terrain |
| Hills | 3 | Moderate elevation |
| Forest | 4 | Dense vegetation |
| Swamp | 5 | Difficult footing |
| Mountains | 6 | Steep terrain |
| Underwater | 8 | Requires swimming |
| Climbing | 9 | Vertical movement |
| Solid Rock | 13 | Nearly impassable |
| Atmosphere | 0 | Flying sectors |

### Movement Cost Modifiers

| Condition | Effect |
|-----------|--------|
| Sneaking | +2 |
| Dwarf in water | +20 |
| AFF_SWIM in water | Cost / 2 |
| AFF_SWIM underwater | Cost / 4 |
| Both legs hurt (crawling) | +20 |
| Crawling with arm injury | Additional +20 |
| Foot wound | +5, fall chance |
| One leg hurt | +10, fall chance |
| Drunk (>9) | +1, fall chance |
| Crawling (base) | +8 horizontal, +16 vertical |
| Flying | Cost / 4 (min 1) |
| Levitating | Cost / 4 (min 5) |
| Haste/Accelerate | Cost / 2 |
| SKILL_HIKING | Reduced by skill% in forest/mountain/swamp |

### Exit Condition Flags

| Flag | Bit | Effect |
|------|-----|--------|
| EXIT_CLOSED | 0 | Blocks passage until opened |
| EXIT_LOCKED | 1 | Requires key to open |
| EXIT_SECRET | 2 | Hidden from casual observation |
| EXIT_DESTROYED | 3 | Broken down, permanently open |
| EXIT_NOENTER | 4 | Cannot pass through |
| EXIT_TRAPPED | 5 | Triggers trap on open |
| EXIT_CAVED_IN | 6 | Blocked by debris |
| EXIT_WARDED | 7 | Requires ward key to pass |
| EXIT_SLOPED_UP | 8 | Upward incline |
| EXIT_SLOPED_DOWN | 9 | Downward incline |
| EXIT_JAMMED | 10 | Stuck closed |

### Door Types and Commands

| Type | Open | Close |
|------|------|-------|
| DOOR_DOOR | open | close |
| DOOR_TRAPDOOR | open | close |
| DOOR_GATE | open | close |
| DOOR_PORTCULLIS | raise | lower |
| DOOR_DRAWBRIDGE | lower | raise |

### Fall Distance Thresholds

| Distance | Without Skills | With Catfall/Feathery Descent |
|----------|----------------|-------------------------------|
| 0-4 rooms | Safe | Safe |
| 5+ rooms | `fallKill()` instant death | Damage |
| 10+ rooms | `fallKill()` instant death | `fallKill()` instant death |

The death threshold is `num1 = 5` without skills or `num1 = 10` with catfall/feathery descent. Falls where `count > num1` trigger `fallKill()`.

Fall damage uses separate formulas by landing type: water landing `count * ::number(5, 30)`, normal ground `count * ::number(15, 55)`, high falls `count * ::number(40, 80)`. Each formula is independently halved by catfall or feathery descent.

### Climbing Modifiers

| Condition | Modifier |
|-----------|----------|
| Agility | `plotStat(STAT_CURRENT, STAT_AGI, 15, 100, 65)` (ranges ~15 to ~100) |
| Weight | Continuous `-getTotalWeight(FALSE) / 5.0` |
| Fighting | -125 |
| Unable to use primary arm | -65 |
| Unable to use secondary arm | -65 |
| Unable to use primary leg | -45 |
| Unable to use secondary leg | -45 |

### Flight Sources

| Source | Duration | Notes |
|--------|----------|-------|
| SPELL_FLY | Spell duration | Magical |
| AFF_FLYING | Until removed | Racial/innate |
| POSITION_FLYING | Until landing | Manual fly command |
| Flying mount | While mounted | Requires mount flight |
| SECT_MAKE_FLY | In sector only | Automatic |

### Water Breathing Sources

| Source | Type |
|--------|------|
| Merman/Mermaid, Fishman, Sea Elf | Racial |
| SPELL_WATERBREATH | Shaman spell |
| SPELL_AQUALUNG | Mage spell |
| Equipment enchantment | Item-based |

### Portal Configuration

| Property | Values | Notes |
|----------|--------|-------|
| Charges | -1 = infinite, 0 = depleted, positive = count | Decrements on use |
| Portal type | 0-13 | Controls entry/exit messages |
| Trap type | doorTrapT enum | Trap damage as unsigned short |
| Portal state | EXIT_* flags | Closed, locked, trapped states |
| Portal key | vnum | Required for locked portals |

### Teleportation Methods

| Method | Class | Destination | Restrictions |
|--------|-------|-------------|--------------|
| Portal spell | Cleric | Fixed portal rooms | Bidirectional, charges |
| Dimensional Fold | Psionic | Player or home | Single use |
| Teleport | Mage | Random | Dangerous |
| Word of Recall | Cleric | Hometown | Faith-dependent |
| Astral Walk | Cleric | Target creature | Can fail catastrophically |
| Summon | Cleric | Caster's location | Many restrictions |
| Portal objects | Builder | Fixed destination | Charges, traps, locks |

### Room Teleportation Restrictions

| Flag | Teleport FROM | Teleport TO | Portal | Astral Walk |
|------|---------------|-------------|--------|-------------|
| ROOM_NO_ESCAPE | Blocked | - | - | Blocked |
| ROOM_NO_PORTAL | - | - | Blocked | - |
| ROOM_NO_MAGIC | - | - | - | Blocked |
| ROOM_PRIVATE | - | Excluded | - | Blocked |
| ROOM_HAVE_TO_WALK | - | Excluded | - | Blocked |
| ROOM_ARENA | Recall blocked | - | - | - |
| ROOM_DEATH | - | Excluded | - | - |
| Flying sector | - | Excluded | Handles flight | - |

### Symbol Quick Reference

| Symbol | Purpose |
|--------|---------|
| `doMove()` | Entry point for directional movement |
| `rawMove()` | Low-level movement execution |
| `validMove()` | Movement validation checks |
| `checkFalling()` | Fall detection and damage |
| `crashLanding()` | Handle landing from fall/dismount |
| `fallKill()` | Instant death for extreme falls |
| `checkDrowning()` | Underwater breath check |
| `riverFlow()` | Current-based involuntary movement |
| `canClimb()` | Climbing skill check |
| `canFly()` | Flight capability check |
| `genericTeleport()` | Random room teleportation |
| `enterMe()` | Portal entry handling |
| `TPortal` | Portal object class |
| `roomDirData` | Exit/door information |

## Implementation

### Movement Entry Point

The `doMove()` function in `movement.cc` serves as the command entry point. It first checks mount control (only the horse master can direct movement) and combat state (cannot move while fighting). Based on group membership, it delegates to either `moveOne()` for solo characters or `moveGroup()` for grouped movement.

### Movement Validation

Before any movement occurs, `validMove()` performs extensive checks: exit existence via `exitDir()`, SPELL_BIND entrapment, exit passability via `exit_ok()`, door states (caved in, closed, warded), and room capacity limits. Each failure produces an appropriate message and returns FALSE.

Warded exits receive special handling through `tryPassWardedExit()`. Immortals and ghosts pass automatically. Mortals must have the correct ward key vnum equipped to pass; the function iterates through all worn equipment checking for matching vnums.

### Movement Cost Calculation

Base cost averages the source and destination terrain costs from `TerrainInfo[]`. This creates gradual transitions rather than abrupt cost changes at sector boundaries.

Modifiers accumulate: sneaking adds 2, injuries add 5-20+, flying divides by 4, haste divides by 2. The hiking skill reduces costs proportionally in wilderness sectors. Mount riders pay 0-33% of the mount's cost randomly.

### Door and Key Systems

Door operations in `movement.cc` use `findDoor()` to locate the target, then validate weight against character strength, check current door state, handle traps via `triggerDoorTrap()`, and finally modify the exit's condition flags. Both sides (current and reverse exits) must be updated for consistency.

Key validation via `has_key()` searches inventory, keyrings (including items inside keyrings), held items, and worn equipment. Keys match by vnum against the exit's key field.

### Flight Mechanics

Flight capability comes from multiple sources: `affectedBySpell(SPELL_FLY)`, `isAffected(AFF_FLYING)`, racial wings via `race->isWinged()`. The `canFly()` function checks these in order.

`doFly()` validates preconditions (not already flying, can fly, not riding, not underwater, feathered races need preening) before setting POSITION_FLYING. `doLand()` validates the reverse (flying, not in flying-only sector, descended below air sectors) before returning to POSITION_STANDING.

Flying sectors (SECT_MAKE_FLY) automatically grant POSITION_FLYING on entry. When leaving, characters without actual flight ability cannot proceed to air/vertical sectors; those entering normal ground sectors have their position reset to standing with a message.

### Climbing and Falling

The `canClimb()` function in `physics.cc` performs a skill check modified by agility, encumbrance, combat state, and limb injuries. The `bSuccess()` call determines pass/fail.

`checkFalling()` handles the fall loop. Immortals bounce harmlessly. Flying characters and flying mounts prevent falling. For each room of descent, the function validates the room below, moves the character, and checks landing conditions.

Fall thresholds depend on SKILL_CATFALL and SPELL_FEATHERY_DESCENT. Without these, `fallKill()` triggers at 5+ rooms; with them, at 10+ rooms. Below the kill threshold, damage uses separate formulas by landing type: water `count * ::number(5, 30)`, ground `count * ::number(15, 55)`, high falls `count * ::number(40, 80)`. Each is halved by catfall or feathery descent.

`crashLanding()` handles the position change and mount dismount. The force parameter bypasses skill checks for involuntary landings. Ground fighting skill can prevent position reduction. Mount falling returns DELETE_THIS if the rider dies, creating chains that must propagate.

### Drowning Mechanics

The `procCharDrowning` scheduler process runs every 36 ticks (3.6 seconds). It calls `checkDrowning()` which affects only PCs in underwater sectors without AFF_WATERBREATH. Damage is `::number(1, 10)` to both hit points and movement per tick, checked via `reconcileDamage()` which returns -1 on death (not a DELETE flag).

River flow uses `procCharRiverFlow` to move characters with the current. The `riverFlow()` function checks room river properties, doubles flow chance for sitting characters, allows SKILL_SWIMMING to resist with a message, and calls `doMove()` for the flow direction.

### Portal Entry

The `TPortal` class in `obj_portal.cc` inherits from `TSeeThru`. Portal entry via `enterMe()` validates state (not closed, not noenter), combat restrictions (berserk characters cannot enter while fighting), destination room, mob limits, and traps. On successful transfer, it moves the character, handles follower chains (mounts first, then others), and decrements charges on both ends.

Portal trap handling checks EXIT_TRAPPED and calls `triggerPortalTrap()`. The return flags combine: DELETE_ITEM and DELETE_THIS means both portal and character destroyed (return DELETE_THIS | DELETE_VICT); DELETE_THIS alone means character died (return DELETE_VICT); DELETE_ITEM alone means portal destroyed (return false).

When a portal's charges reach zero, it returns DELETE_THIS. The caller must check both DELETE_THIS (portal deleted) and DELETE_VICT (character died to trap).

### Teleportation

`genericTeleport()` in `magicutils.cc` selects random valid rooms in a loop. Zone-restricted mode limits selection to current zone. Safety mode excludes private, walk-only, death, and flying-sector rooms. The function handles dismounting, transfers the character, and calls `genericMovedIntoRoom()` which may return DELETE_THIS from room hazards.

Spell-based teleportation (portal, word of recall, astral walk, summon) in the cleric discipline files implement extensive restriction checking before delegating to lower-level transfer functions.

## Troubleshooting

### Crash After Movement Function Call

**Symptom:** Use-after-free or segfault after calling doMove(), checkFalling(), or genericTeleport().

**Likely cause:** DELETE_THIS return value ignored; code continued operating on deleted character.

**Diagnostic approach:** Add logging before and after the movement call. Check if the character pointer becomes invalid. Verify the function can return DELETE_THIS in its code path.

**Fix:** Check `IS_SET_DELETE(rc, DELETE_THIS)` immediately after the call and return DELETE_THIS to propagate up the stack.

### Crash After reconcileDamage() in Drowning/Falling

**Symptom:** Code continues after character death from environmental damage.

**Likely cause:** Using `IS_SET_DELETE(rc, DELETE_VICT)` instead of `== -1` check.

**Diagnostic approach:** Verify the death check uses the correct sentinel comparison. The `-1` return is not a bitfield.

**Fix:** Replace `IS_SET_DELETE()` with direct `if (reconcileDamage(...) == -1) return DELETE_THIS;`.

### Iterator Crash in River Flow or Follower Movement

**Symptom:** Invalid pointer dereference when moving groups through rivers or following through portals.

**Likely cause:** Loop iterator not cached before list modification. The `--(*t)` operation clears next pointers.

**Diagnostic approach:** Check if `t->nextRider` or similar is accessed after modification of `t`.

**Fix:** Cache: `for (TThing* t = list; t; t = cached) { cached = t->next; ... }`.

### Door Only Opens on One Side

**Symptom:** Opening a door leaves the other side closed.

**Likely cause:** Reverse exit not modified. Only the current room's exit condition was updated.

**Diagnostic approach:** Check if both `exitp->condition` and `back->condition` are modified where `back = rp->dir_option[rev_dir(door)]`.

**Fix:** Ensure both sides receive the flag modification. Validate back is non-null first (may be a one-way exit).

### Portcullis Won't Open with "open" Command

**Symptom:** Portcullis reports "you can't see how to open that" or similar.

**Likely cause:** Using wrong command. Portcullis uses "raise" to open, not "open".

**Diagnostic approach:** Verify door type in zone file or examine output. Check which command the type expects.

**Fix:** Use "raise" for portcullis, "lower" for drawbridge opening.

### Flying Character Cannot Leave Flying Sector

**Symptom:** Character cannot move from SECT_MAKE_FLY to adjacent air sector.

**Likely cause:** Character lacks actual flight ability; only had sector-granted flight.

**Diagnostic approach:** Check `affectedBySpell(SPELL_FLY)` and `isAffected(AFF_FLYING)`. The sector grants POSITION_FLYING but not the ability to sustain it.

**Fix:** Character needs flight spell or racial ability to leave flying sectors into air sectors.

### Character Drowns Despite Water Breathing

**Symptom:** Character takes drowning damage with water breathing spell active.

**Likely cause:** The affect expired, was dispelled, or the character lacks AFF_WATERBREATH specifically (spell may set different flags).

**Diagnostic approach:** Verify `isAffected(AFF_WATERBREATH)` returns true. Check affect list for the water breathing spell and its remaining duration. Check both spell-granted and racial immunity.

**Fix:** Recast water breathing spell or verify the spell properly sets AFF_WATERBREATH.

### Drowning Damage Not Applying

**Symptom:** Characters in underwater sectors take no drowning damage.

**Likely cause:** `procCharDrowning` scheduler process not registered.

**Diagnostic approach:** Verify the process is added to the character's process list during descriptor initialization.

**Fix:** Ensure drowning process registration occurs in descriptor setup. The process short-circuits via sector check, so it should always be active.

### Teleport Always Fails or Loops

**Symptom:** `genericTeleport()` loops indefinitely or returns failure.

**Likely cause:** All valid rooms excluded by safety checks, or zone has no enabled rooms in range.

**Diagnostic approach:** Check zone enable status. Verify room flags in the target zone. Count how many rooms pass the exclusion filters.

**Fix:** Ensure target zone is enabled and has at least one room without ROOM_PRIVATE, ROOM_HAVE_TO_WALK, ROOM_DEATH, and non-flying sectors.

### Portal Traversal Fails Silently

**Symptom:** Character enters portal but nothing happens or unexpected behavior occurs.

**Likely cause:** Trap handling altered state before traversal completed.

**Diagnostic approach:** Check if portal trap teleported the character elsewhere. Verify DELETE flag handling covers all combinations from `triggerPortalTrap()`.

**Fix:** After trap handling, check sameRoom status before proceeding with traversal. Handle all DELETE flag combinations: DELETE_ITEM, DELETE_THIS, and both together.
