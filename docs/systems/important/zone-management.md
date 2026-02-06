---
title: Zone Management System
description: Zone lifecycle from boot through runtime resets - discovery, aging, execution, cleanup
keywords: [zone lifecycle, zone resets, zone aging, conditional execution, random room placement, armor sets, util_flag]
category: important
primary_symbols:
  functions: [bootZones, bootOneZone, bootZone, resetZone, renumCmd, procZoneUpdate, isEmpty, doGenericReset]
  classes: [zoneData, resetCom]
  enums: [ZO_DEAD, ZONE_ROOM_RANDOM, MAX_SHOP_INVENTORY, resetFlagBootTime, resetFlagFindLoadPotential, CMD_GENERIC_RESET]
---

# Zone Management System

## Overview

Zones define discrete world regions with independent reset schedules. The system handles four lifecycle phases: initialization at boot (discovery, parsing, database sync), aging at runtime (tracking time since last reset), execution (processing reset commands to spawn content), and cleanup (notifying objects via spec procs).

Zonefiles in `lib/zonefiles/` are discovered dynamically via directory scan. Each file's numeric name becomes the zone's starting vnum. Zones progress from aging to queued status when age reaches lifespan, then reset based on mode: never, when empty of players, or always.

The load-on-death system stores equipment commands for execution when mobs die, enabling dynamic loot without cluttering the ground. Random room placement supports roaming mob patterns.

## Patterns

### Zone Creation

- Always pick vnums using `zonefile new` command to prevent collisions
- Always set `enabled = 0` during development, flip to 1 only when ready for production
- Always validate mob/object vnums exist before publishing - invalid vnums cause silent spawn failures
- Always use `boot zone <vnum>` or server restart to activate zonefile changes

### Reset Commands

- Always terminate zonefiles with `S` command
- Always set `if_flag = 1` for equipment commands (G/P/E/I) - the parser rejects `if_flag = 0`
- Never assume conditional commands execute - check dependency chain from preceding commands
- Never use `-99` room vnum without preceding `A` command to set the range

### Database Synchronization

- Never manually edit the `zone` database table - the util_flag system manages it automatically
- Never modify zonefiles while zone is enabled on production - causes inconsistent state during reset

### Vnum Validation

- Always check `LOG_LOW` for "resolving mobile/object number" errors after boot
- Always verify vnums with `show mob <vnum>` or `show obj <vnum>` before adding to zonefile
- Never trust that a spawn worked without verification - invalid vnums cause commands to be silently removed at boot

### Performance

- Always vary lifespan values across zones to distribute reset processing
- Never create zones with 1000+ commands without testing reset time impact
- Always use reset_mode 1 (when empty) for standard zones - mode 2 resets can disrupt players

## Reference

### Reset Modes

| Mode | Name | Behavior |
|------|------|----------|
| 0 | Never | Zone never resets - static zones, builder testing |
| 1 | When Empty | Reset only when no players in zone - standard zones |
| 2 | Always | Reset on schedule regardless of players - critical zones |

### Mobile Commands

| Cmd | Format | Description |
|-----|--------|-------------|
| M | `M if mob_vnum room_max room_vnum` | Load mob at location |
| C | `C 1 mob_vnum room_max room_vnum` | Load charmed follower to previous mob |
| K | `K 1 mob_vnum room_max room_vnum` | Load grouped mob with previous mob |
| R | `R 1 mob_vnum room_max room_vnum` | Load mount for previous mob |

### Object Commands

| Cmd | Format | Description |
|-----|--------|-------------|
| O | `O if obj_vnum max room_vnum` | Load on ground (boot only) |
| B | `B if obj_vnum max room_vnum` | Load on ground (every reset) |
| G | `G 1 obj_vnum max` | Give to last mob's inventory |
| E | `E 1 obj_vnum max slot` | Equip on last mob |
| P | `P 1 obj_vnum max container_vnum` | Place in container |
| I | `I 1 obj_vnum max slot` | Equip as prop (load-on-death) |

### Utility Commands

| Cmd | Format | Description |
|-----|--------|-------------|
| ? | `? if percent 0 CMD` | Percent chance for next command |
| A | `A 0 low_room high_room` | Set random room range for -99 loads |
| D | `D if room_vnum dir state` | Set door state (0=open, 1=closed, 2=locked) |
| T | `T if room_vnum dir trap_type damage` | Trap on door |
| T | `T 1 trap_type damage` | Trap on last object |
| V | `V 1 value_index new_value` | Change object value |
| H | `H 1 hate_type 0` | Set hate on last mob |
| F | `F 1 fear_type 0` | Set fear on last mob |
| L | `L 1 min_level max_level 0 1` | Random loot on mob death |
| S | `S` | End of zone (required) |

### Armor Set Commands

| Cmd | Format | Description |
|-----|--------|-------------|
| X | `X slot set_num obj_vnum` | Define local armor set slot |
| Y | `Y 0 global_set_id percent` | Load global armor set |
| Z | `Z 1 local_set_num percent` | Load local armor set |
| J | `J 1 local_set_num percent` | Load local set as props |

### Wear Slots

| Slot | Location | Slot | Location |
|------|----------|------|----------|
| 1 | R.Finger | 12 | R.Arm |
| 2 | L.Finger | 13 | L.Arm |
| 3 | Neck | 14 | Back |
| 4 | Body | 15 | Waist |
| 5 | Head | 16 | R.Wrist |
| 6 | R.Leg | 17 | L.Wrist |
| 7 | L.Leg | 18 | Hold(L) |
| 8 | R.Foot | 19 | Hold(R) |
| 9 | L.Foot | 20 | Hold(both) |
| 10 | R.Hand | 21 | Thrown |
| 11 | L.Hand | | |

### Directions

| Value | Direction |
|-------|-----------|
| 0 | North |
| 1 | East |
| 2 | South |
| 3 | West |
| 4 | Up |
| 5 | Down |

### Constants

| Name | Value | Purpose |
|------|-------|---------|
| ZO_DEAD | 9999 | Marks zone as queued for reset |
| ZONE_ROOM_RANDOM | -99 | Triggers random room placement |
| MAX_SHOP_INVENTORY | 2500 | Shop item limit |

### resetFlag Enum

| Flag | Purpose |
|------|---------|
| resetFlagBootTime | Execute O commands, normal reset |
| resetFlagFindLoadPotential | Store equipment commands for load-on-death |

### Display Commands

| Command | Purpose |
|---------|---------|
| `show zones` | List all zones with age, mode, enabled status |
| `stat zone <zone_nr>` | Detailed zone statistics |

## Implementation

### Data Structures

**zoneData** holds zone metadata and reset commands: `zone_nr` (sequential runtime index), `bottom`/`top` (vnum range), `lifespan` (reset interval in minutes), `age` (minutes since last reset), `reset_mode`, `enabled`, `name`, and `cmd_table` (vector of resetCom).

**resetCom** represents a single reset command: `command` (char type), `if_flag` (conditional execution), `arg1`-`arg4` (command-specific), `character` (for ? command), `cmd_no` (command sequence number).

### Phase 1: Initialization

**bootZones()** scans `lib/zonefiles/` with `readdir()`, inserts filenames into a multimap keyed by vnum for automatic sorting, resets all database `util_flag` to 0, then calls `bootOneZone()` for each file. After processing, deletes orphaned zones where `util_flag` remains 0.

**bootOneZone()** calls `bootZone()` to parse the file, then `renumCmd()` to validate vnums. Performs database upsert (UPDATE, then INSERT if rowCount=0) with `util_flag = 1`. Appends zoneData to `zone_table` vector.

**bootZone()** parses zonefile header (zone_number, name, top_room/lifespan/reset_mode/enabled), then iterates reset commands until `S` terminator. Skips comment lines (starting with `*`) and skips ALL reset commands when running on the GAMMA port. Equipment commands require `if_flag = 1`.

**renumCmd()** converts vnums to runtime indices via `real_mobile()` and `real_object()`. Invalid vnums log to LOG_LOW and the command is skipped (via `continue`), removing it from `cmd_table` entirely rather than leaving a broken entry.

### Phase 2: Aging

**procZoneUpdate** runs every `Pulse::MUDHOUR` (144 seconds). First pass increments `age` for enabled zones below lifespan. When `age >= lifespan`, sets `age = ZO_DEAD` (9999) to mark as queued. Second pass processes queued zones: mode 2 resets immediately, mode 1 calls `isEmpty()` first.

**isEmpty()** iterates `descriptor_list`, skipping non-playing connections and characters without a room. Returns false if any character's `roomp->getZoneNum()` matches the zone's `zone_nr`.

### Phase 3: Execution

**resetZone()** iterates `cmd_table`, tracking `last_cmd` success state. Commands with `if_flag = 1` skip when `last_cmd` is false. Each command's `execute()` method returns success/failure. Maintains `tmob` (last mob loaded) and `tobj` (last object loaded) for dependent commands.

**Conditional execution** chains G/E/I commands to preceding M commands. Example: `M 0 120 1 216` loads a mob (always), `E 1 300 1000 19` equips sword only if M succeeded.

**Load-on-death** stores G/E/?/Y/Z/J commands in `tmob->loadCom` when `resetFlagFindLoadPotential` is set. Execution occurs in `die()` when mob is killed.

**Random room placement** uses `A` command to set range, then `-99` room vnum triggers `number(low, high)` selection. The `random_room` value persists across commands in the same reset, so multiple loads with `-99` use the same room from the most recent `A` command.

### Phase 4: Cleanup

**doGenericReset()** iterates the global `object_list` and checks if each object's vnum falls within the zone's vnum range (previous zone's top+1 to this zone's top), sending `CMD_GENERIC_RESET` to each matching object's spec proc. Age reset to 0 happens in the calling function `resetZone()`, not in `doGenericReset()` itself.

### Database Schema

The `zone` table stores: `zone_nr` (primary key, sequential), `zone_name`, `zone_enabled`, `bottom`, `top`, `reset_mode`, `lifespan`, `util_flag`. The util_flag pattern enables automatic orphan cleanup without manual maintenance.

### Zonefile Format

```
#zone_number
Zone Name~
top_room lifespan reset_mode enabled
[reset commands...]
S
```

Example:
```
#15200
Merc - Pantathian Extension~
15249 45 2 1

D 0 15239 0 1                 ; Close shaman door
T 0 15239 0 6 4               ; Fire trap on door

M 0 15212 1 15200             ; Guard
? 0 5 0 E
E 1 6112 400 4                ; Rusty shirt
Y 0 41 2                      ; Ringmail set

S
```

## Troubleshooting

### Zone Never Resets

**Symptom:** Age counter increases past lifespan but zone never executes reset.

| Cause | Fix |
|-------|-----|
| reset_mode = 0 | Edit zonefile, change to 1 or 2 |
| enabled = 0 | Edit zonefile header, set to 1 |
| Mode 1 with players present | Wait for players to leave or use `show zones` to monitor |
| Already queued (age = 9999) | Zone is waiting - check reset_mode and isEmpty() |

**Debug:** `stat zone <zone_nr>`, `show zones`

### Mobs Not Spawning

**Symptom:** Zone resets (age resets to 0) but mobs don't appear.

| Cause | Fix |
|-------|-----|
| Invalid vnum | Check LOG_LOW for "resolving mobile number", verify with `show mob <vnum>` |
| max_exist reached | `stat mob <vnum>` shows "Cur exist" at limit |
| Conditional chain broken | Review zonefile - ensure if_flag dependencies satisfied |
| Wrong room vnum | Verify room exists with `goto <vnum>` |

**Debug:** `grep "resolving mobile" logs/sneezy.log`

### Objects Missing

**Symptom:** Objects not appearing in expected locations.

| Cause | Fix |
|-------|-----|
| Invalid vnum | Check LOG_LOW, verify with `show obj <vnum>` |
| Container at capacity | Check container max_contain value |
| Shop inventory full | MAX_SHOP_INVENTORY = 2500 |
| Object decayed | Check decay timers, use B instead of O for persistent objects |
| O command on runtime reset | O only executes at boot; use B for every-reset objects |
| E command slot mismatch | Verify object wear flags allow the specified slot |

### Random Room Placement Issues

**Symptom:** Mobs or objects with `-99` room vnum not appearing.

| Cause | Fix |
|-------|-----|
| No preceding A command | Add `A 0 low_room high_room` before the load command |
| A command is conditional and failed | Check if_flag on A command; use `if_flag = 0` for unconditional |
| Range contains invalid rooms | Verify all room vnums in the range exist |
| Multiple A commands conflict | Each random-placement load needs its own A command, or intentionally share |

### Orphaned Database Entries

**Symptom:** Database has zones not in filesystem.

**Cause:** Zone file deleted without database cleanup, or incomplete boot.

**Fix:** Automatic on next boot via util_flag system. Manual: `DELETE FROM zone WHERE util_flag = 0`

**Note:** Adding or removing zones causes zone_nr sequence to shift. Database update uses zone_nr as primary key, so all subsequent zones get mismatched zone_nr values until next boot.

### Reset Lag Spikes

**Symptom:** Server stutters every 144 seconds.

| Cause | Fix |
|-------|-----|
| Too many zones resetting simultaneously | Vary lifespan values across zones |
| Complex zonefiles | Split into smaller zones or simplify commands |
| Many mode-2 zones | Convert to mode-1 where possible |
| Many objects in zone rooms | Clean up unnecessary objects or move to containers |

### Silent Failures

**Symptom:** Commands fail without logging errors during runtime.

Common causes of silent failures:
- Commands removed from `cmd_table` due to failed vnum validation at boot
- max_exist limits reached for mobs or objects
- Conditional execution chains broken by failed predecessor
- Invalid room vnums that don't exist

**Debug approach:**
1. Check LOG_LOW and LOG_MISC at boot for validation warnings
2. Use `stat mob/obj <vnum>` to check current vs maximum exist values
3. Trace if_flag chains in zonefile for broken dependencies
4. Test zones in isolation by disabling others temporarily
5. Use `goto` to visit spawn locations immediately after reset
6. Verify zonefile syntax: missing arguments, mismatched if_flag values, missing S terminator
