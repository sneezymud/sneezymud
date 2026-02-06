---
title: Builder Systems
description: World building tools including OLC editors (redit/medit/oedit), content creation workflow, and zone publishing
keywords: [online creation, world building, content creation workflow, vnum ranges, dual database, immortal database, zone enablement]
category: important
primary_symbols:
  functions: [RoomSave, RoomLoad, limitPowerCheck, mvRoom, stripSpellAffects]
  classes: [TRoom]
  enums: [POWER_REDIT, POWER_REDIT_ENABLED, POWER_NO_LIMITS, POWER_LOW, POWER_OEDIT_IMP_POWER, DB_IMMORTAL, DB_SNEEZY, ROOM_ALWAYS_LIT, ROOM_DEATH, ROOM_NO_MOB, ROOM_INDOORS, ROOM_PEACEFUL, ROOM_NO_STEAL, ROOM_NO_ESCAPE, ROOM_NO_MAGIC, ROOM_NO_PORTAL, ROOM_PRIVATE, ROOM_SILENCE, ROOM_ARENA, ROOM_SAVE_ROOM, DIR_NORTH, DIR_EAST, DIR_SOUTH, DIR_WEST, DIR_UP, DIR_DOWN, EXIT_CLOSED, EXIT_LOCKED, EXIT_SECRET, EXIT_DESTROYED, EXIT_TRAPPED, EXIT_CAVED_IN, EXIT_WARDED]
---

# Builder Systems

## Overview

The OLC (Online Creation) system enables builders to create and modify game world content through in-game editors without server restarts or direct database access. Three specialized editors target different content types: rooms define world locations, mobiles define NPCs and monsters, objects define items and equipment.

A dual-database architecture separates builder workspace from live content. Builders work in isolation within the immortal database, testing and iterating freely. When content is ready, the publishing system transfers it atomically to the production database. This separation prevents incomplete work from affecting players while allowing multiple builders to develop different versions of the same content simultaneously.

The block system assigns each builder exclusive vnum ranges, preventing conflicts when multiple builders work in parallel. Every OLC operation validates that the target vnum falls within the builder's assigned blocks before proceeding.

## Patterns

### Workflow Discipline

Always save work to the immortal database frequently with `rsave`/`medit save`/`oedit save` commands. The immortal database persists across crashes while unsaved in-memory changes do not.

Always load your work from the immortal database after a reboot using `rload` before continuing edits. In-memory state is lost on restart.

Always publish content in the correct order: rooms first, then mobiles and objects. Mobiles and objects reference rooms by vnum; the rooms must exist first.

Always test content in-game before publishing. Load rooms with `rload`, reset zones with `boot zone`, walk through all rooms, fight mobs, interact with objects.

Never use `oedit resave` unless absolutely necessary. The command deletes before saving; a crash between those operations loses the object permanently. Consider requesting a LOW immortal to perform the resave with a database backup taken immediately before.

Never publish content that hasn't been saved to the immortal database. The `low mv*` commands read from immortal and write to production; they cannot access in-memory state.

### Block Assignment

Always verify your assigned blocks before editing with `stat self`. Attempting to edit vnums outside your blocks fails silently or with an access error.

Always request block assignments from a LOW before starting work on a new zone. Request both a contiguous vnum range and temporary access to any existing rooms you need to modify for zone connections.

Never modify vnums outside your assigned blocks. Even if the command appears to succeed, the immortal database rejects the save.

Use blocks 101 and 102 as backup slots. Save known-good versions to these slots before making experimental changes; they can be restored if needed.

### Zone Enablement

Always set the zone enabled flag to 0 during development. This prevents the zone from loading automatically and allows controlled testing.

Always change the enabled flag to 1 only after publishing all rooms, mobiles, and objects. Enabling a zone that references unpublished content causes load errors.

### Content Safety

Always strip spell affects from mobiles before saving. Use `stripSpellAffects()` or load a fresh copy; spell effects applied during testing should not persist.

Always clear prototype and strung flags from objects during publishing. These flags mark development-only states.

Always use transactions when publishing. The `low mv*` commands wrap operations atomically; never interrupt them.

Always create bidirectional exits manually. The room editor does not automatically create reverse exits; after adding an exit from room A to B, switch to B and add the return exit to A.

### Zone Connection

Always connect new zones to the existing world after publishing. Identify an appropriate connection point in an existing room, request temporary blocka access to that vnum, add exits in both directions, save both rooms to immortal, and publish both to sneezy. Verify the connection by walking through both directions before removing temporary block access.

## Reference

### Permission Requirements

| Permission | Level | Purpose |
|------------|-------|---------|
| `POWER_REDIT` | 51 | Basic room editing |
| `POWER_REDIT_ENABLED` | 51 | Unrestricted room vnum access |
| `POWER_LOW` | 51 | Publishing access |
| `POWER_OEDIT_IMP_POWER` | 60 | Object resave access |

### Editor Commands

| Editor | Create | Load | Modify | Save | Copy |
|--------|--------|------|--------|------|------|
| redit | `redit create <vnum>` | `rload <block> [vnum]` | `redit [vnum]` | `rsave <block> [vnum]` | - |
| medit | `medit create` | `medit load <vnum>` | `medit mod <name>` | `medit save <name> <vnum>` | `medit copy <vnum>` |
| oedit | `oedit create` | `oedit load <vnum>` | `oedit mod <name>` | `oedit save <name> <vnum>` | `oedit copy <vnum>` |

### Publishing Commands

| Command | Syntax | Purpose |
|---------|--------|---------|
| mvroom | `low mvroom <builder> <block> <vnum[-vnum]>` | Publish rooms |
| mvmob | `low mvmob <builder> <vnum[-vnum]>` | Publish mobiles |
| mvobj | `low mvobj <builder> <vnum[-vnum]>` | Publish objects |
| mvresponse | `low mvresponse <builder> <vnum[-vnum]>` | Publish mob responses |

### Room Editor Menu Options

| Option | Field | Max Length |
|--------|-------|------------|
| 1 | description | 1024 chars |
| 2 | exdscr | 512 chars |
| 3 | exit | 50 chars |
| 4 | extra | 512 chars |
| 5 | flags | - |
| 6 | height | - |
| 7 | line | - |
| 8 | max_capacity | - |
| 9 | name | 80 chars |
| 10 | river | - |
| 11 | sector_type | 59 options |
| 12 | teleport | 100 chars |
| 13 | copy | - |
| 14 | replace | - |
| 15 | list | - |
| 16 | autoformat | - |

### Room Flags

| Flag | Effect |
|------|--------|
| `ROOM_ALWAYS_LIT` | Base light level 18 |
| `ROOM_DEATH` | Kills players entering |
| `ROOM_NO_MOB` | Mobs cannot enter |
| `ROOM_INDOORS` | Sheltered from weather |
| `ROOM_PEACEFUL` | No combat allowed |
| `ROOM_NO_STEAL` | Stealing disabled |
| `ROOM_NO_ESCAPE` | Cannot flee from combat |
| `ROOM_NO_MAGIC` | Magic use blocked |
| `ROOM_NO_PORTAL` | Portal spells blocked |
| `ROOM_PRIVATE` | Limited occupancy |
| `ROOM_SILENCE` | No speech or sounds |
| `ROOM_ARENA` | Arena combat rules |
| `ROOM_SAVE_ROOM` | Items persist across reboots |

### Exit Directions

| Direction | Constant | Value |
|-----------|----------|-------|
| North | `DIR_NORTH` | 0 |
| East | `DIR_EAST` | 1 |
| South | `DIR_SOUTH` | 2 |
| West | `DIR_WEST` | 3 |
| Up | `DIR_UP` | 4 |
| Down | `DIR_DOWN` | 5 |
| Northeast | `DIR_NORTHEAST` | 6 |
| Northwest | `DIR_NORTHWEST` | 7 |
| Southeast | `DIR_SOUTHEAST` | 8 |
| Southwest | `DIR_SOUTHWEST` | 9 |

### Exit Flags

| Flag | Effect |
|------|--------|
| `EXIT_CLOSED` | Door is shut |
| `EXIT_LOCKED` | Requires key to open |
| `EXIT_SECRET` | Hidden from observation |
| `EXIT_DESTROYED` | Door has been broken |
| `EXIT_TRAPPED` | Trap is set on door |
| `EXIT_CAVED_IN` | Passage blocked by debris |
| `EXIT_WARDED` | Magical barrier |

### Mobile Editor Menu Options

| Option | Field | Option | Field |
|--------|-------|--------|-------|
| 1 | name | 16 | race |
| 2 | short_desc | 17 | weight |
| 3 | long_desc | 18 | height |
| 4 | description | 19 | pos_def |
| 5 | action_flags | 20 | pos |
| 6 | affect_flags | 21 | sex |
| 7 | faction | 22 | spec |
| 8 | attacks | 23 | skin |
| 9 | level | 24 | vision |
| 10 | hitroll | 25 | can_be_seen |
| 11 | ac | 26 | max_exist |
| 12 | hpbonus | 27 | local_num |
| 13 | damroll | 28 | intelligence |
| 14 | gold | 29 | immunities |
| 15 | class | 30 | extra_desc |

### Mobile Scaling Stats

| Field | Description |
|-------|-------------|
| `hpLevel` | HP scaling: damLevel * 100 + hpbonus |
| `damLevel` | Damage scaling multiplier |
| `acLevel` | Armor class scaling |
| `attackLevel` | Attack bonus scaling |

### Object Core Fields

| Field | Description |
|-------|-------------|
| name | Keywords for referencing |
| short_desc | Name shown in inventory |
| long_desc | Description when on ground |
| action_desc | Special action description |
| type | Item type (67+ types) |
| extra_flags | ITEM_* flags |
| wear_flags | ITEM_WEAR_* flags |
| weight | Item weight |
| price | Base value |
| material | Material type |
| volume | Item volume |
| max_struct | Maximum structure points |
| cur_struct | Current structure points |
| decay | Decay timer (-1 = never) |

### Object Type Value Fields

| Type | val0 | val1 | val2 | val3 |
|------|------|------|------|------|
| Weapon | Sharpness (curSharp + maxSharp packed) | Damage levels (damLevel + damDev packed) | Weapon types | Reserved |
| Armor | AC bonus | Reserved | Reserved | Reserved |
| Container | Max weight | Flags/trap/damage | Key vnum | Max volume |
| Drink | Max units | Current units | Liquid type | Drink flags |
| Wand/Staff | Magic level | Max charges | Current charges | Spell number |
| Scroll | Magic level | Spell 1 | Spell 2 | Spell 3 |

### Database Schema

| Database | Constant | Purpose | Key Columns |
|----------|----------|---------|-------------|
| immortal | `DB_IMMORTAL` | Builder workspace | owner, block, vnum |
| sneezy | `DB_SNEEZY` | Production data | vnum only |

### Zonefile Reset Commands

| Code | Syntax | Purpose |
|------|--------|---------|
| M | `M <if> <mob> <max> <room>` | Load mob in room |
| G | `G <if> <obj> <max>` | Give object to last mob |
| E | `E <if> <obj> <max> <slot>` | Equip object on last mob |
| O | `O <if> <obj> <max> <room>` | Load object in room |
| D | `D <if> <room> <dir> <state>` | Set door state |
| S | `S` | End of reset commands |

The `<if>` parameter determines whether the command executes unconditionally (0) or only if the previous command succeeded (1).

### Zonefile Header Format

```
#<zone_number>
<Zone Name>~
<top_room> <lifespan> <reset_mode> <enabled>
```

Reset mode 2 resets only when no players are present in the zone.

## Implementation

### Dual-Database Architecture

The immortal database extends the sneezy schema with ownership tracking. The room table in immortal uses a composite primary key of (owner, vnum) plus a block column for versioning. This allows builder Alice to have block 1 and block 2 versions of room 45660, while builder Bob has his own independent versions.

The sneezy database uses simple vnum primary keys with no ownership or versioning. Only one version of each room exists in production.

Publishing copies from immortal to sneezy, stripping ownership metadata. The destination row replaces any existing content at that vnum.

### Block Validation

The `limitPowerCheck()` function gates every OLC operation. It checks for `POWER_NO_LIMITS`, which bypasses all restrictions. Otherwise it compares the target vnum against the builder's blockastart/blockaend and blockbstart/blockbend descriptor fields.

Block assignments are stored on the player's descriptor and saved with `save`. The `@set blocka` and `@set blockb` commands modify these fields.

### Room Save Implementation

The `RoomSave()` function reads the in-memory TRoom state and writes it to immortal.room with an INSERT ON DUPLICATE KEY UPDATE query. Related data goes to immortal.roomextra and immortal.roomexit tables.

The block parameter determines which version slot receives the save. Blocks 1 and 2 are working slots; blocks 101 and 102 serve as backup slots that can be restored if needed.

### Room Load Implementation

The `RoomLoad()` function queries immortal.room for the specified owner/block/vnum, then updates the in-memory TRoom with the database values. Related data loads from immortal.roomextra and immortal.roomexit.

Loading replaces the in-memory room state. Any unsaved changes to that room are lost.

### Mobile Save Implementation

Before saving, `stripSpellAffects()` iterates the mobile's affect list using a cached next pointer pattern and removes ALL affects unconditionally. This prevents testing-applied buffs and debuffs from persisting.

The mobile then writes to immortal.mob with related data going to immortal.mob_extra and immortal.mob_imm tables.

### Object Save Implementation

Object saves write to immortal.obj with related data going to immortal.objextra and immortal.objaffect tables.

The resave operation first deletes the existing row then inserts the new version. This non-atomic sequence can lose data on crash.

### Publishing Implementation

The `mvRoom()` function wraps its operations in a database transaction. It reads from immortal, transforms the data (stripping owner/block metadata), deletes any existing production row, and inserts the new content. Related tables follow the same pattern within the transaction.

Mobile publishing additionally clears the `ACT_STRINGS_CHANGED` bit from action flags. This flag indicates that strings were modified during editing and is only relevant during development.

Object publishing clears `ITEM_STRUNG` and `ITEM_PROTOTYPE` bits from extra flags. `ITEM_STRUNG` indicates a unique player-customized item that should never exist as a zone reset; `ITEM_PROTOTYPE` marks unfinished builder work.

All three publishing commands handle vnum ranges, iterating through each vnum in sequence within the same transaction. If any vnum in the range fails to fetch from immortal, the entire transaction rolls back to prevent partial zone updates.

### Zone Creation Flow

The `zonefile new` command creates a zonefile with a header and empty reset section. It allocates vnums from the next available range and sets the enabled flag to 0.

The zone does not load automatically until enabled. Builders manually load rooms with `rload` and reset mobs/objects with `boot zone` during development.

### Zone Activation Flow

After publishing all content, the builder edits the zonefile header to change the enabled flag from 0 to 1. On reboot, the zone loader reads the header, sees enabled=1, and loads the zone normally.

Alternatively, `boot zone <vnum>` forces an immediate reset without requiring a full reboot.

## Troubleshooting

### Symptom: "You don't have access to that vnum range"

Cause: Target vnum falls outside assigned blocks.

Diagnostic: Run `stat self` to see blocka and blockb ranges.

Fix: Request block expansion from a LOW with `@set blocka <builder> <start> <end>`, or use a vnum within the existing range.

### Symptom: "Room does not exist" when editing

Cause: Attempting to edit a room that was never created.

Diagnostic: Try `goto <vnum>` to verify the room exists.

Fix: Use `redit create <vnum>` to create the room before editing. Creating a room at a vnum already present in memory overwrites the in-memory instance without affecting the database until explicit save.

### Symptom: "Not found" during low mvroom

Cause: Content not saved to immortal database, or wrong builder/block specified in command.

Diagnostic: Check immortal database directly with a query, or verify the save command was issued.

Fix: Save content with `rsave <block> <vnum>` before publishing. Verify builder name and block number match those used during save. The builder name parameter must match the owner field exactly, accounting for case sensitivity.

### Symptom: Mobs/objects not loading in zone

Cause: Zone disabled, content not published, or zonefile references non-existent vnums.

Diagnostic: Check zonefile header for enabled flag. Query sneezy.mob and sneezy.obj for the vnums. Compare zonefile vnum references against published content.

Fix: Publish content with `low mvmob` and `low mvobj`. Change zonefile enabled flag to 1. Correct any vnum mismatches in reset commands. Syntax errors in the zonefile prevent parsing beyond the error point.

### Symptom: Lost work after crash

Cause: Changes existed only in memory; never saved to immortal database.

Diagnostic: Attempt `rload 1 <vnum>` to see what was last saved.

Fix: Preventive only. Save frequently with `rsave 1` during editing sessions. The immortal database persists across crashes.

### Symptom: Object vanished after oedit resave

Cause: Crash occurred between delete and insert during resave operation.

Diagnostic: Query both immortal.obj and sneezy.obj for the vnum to confirm absence.

Fix: Recreate the object with `oedit create`. Avoid `resave` for critical content. Always maintain a backup copy in block 101 or 102 before using resave.

### Symptom: Spell effects persisting on published mobs

Cause: Mobile was saved while spell affects were active.

Diagnostic: Load the mob and check for unexpected affects.

Fix: Use `medit load <vnum>` to get a clean copy, or manually strip affects before saving. The `medit save` command invokes `stripSpellAffects` automatically, but direct database manipulation bypasses this safety check. Republish with `low mvmob`.

### Symptom: Builder flags appearing on production objects

Cause: Objects published without proper flag stripping, likely via direct database copy instead of `low mvobj`.

Diagnostic: Check object extra_flags for `ITEM_STRUNG` or `ITEM_PROTOTYPE` bits.

Fix: Always use the `low mvobj` command for publishing rather than manual SQL operations. The publishing function strips these flags automatically.

### Symptom: One-way exits confusing players

Cause: Exit created in one direction without corresponding reverse exit.

Diagnostic: Walk from room A to B, then check if exit back to A exists.

Fix: Switch to the destination room and manually add the return exit. Some zones intentionally use one-way exits for puzzles or drops; verify design intent before assuming bidirectionality.

### Symptom: New zone inaccessible to players

Cause: Zone not connected to existing world geography.

Diagnostic: Check whether any existing room has an exit leading into the new zone.

Fix: Identify an appropriate connection point in an existing room. Request temporary blocka access to that vnum from a LOW. Edit both the existing room and the new zone entrance to add exits in both directions. Save and publish both rooms. Verify by walking through both directions.
