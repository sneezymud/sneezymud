---
title: Builder Systems
description: World building tools including OLC editors (redit/medit/oedit), content creation workflow, and zone publishing
keywords: [redit, medit, oedit, OLC, room editor, mobile editor, object editor, blocka, blockb, zone publishing, doLoad, doPurge, content creation, builder workflow]
category: Important Systems

last_updated: 2026-01-29
source_files: [code/code/cmd/cmd_low.cc, code/code/misc/immortal.cc, code/code/misc/create_rooms.cc, code/code/misc/create_mobs.cc, code/code/misc/create_objs.cc]
related:
  - admin-systems.md
  - zone-management.md
  - object-system.md
  - room-environment.md
---

# Builder Systems

The OLC (Online Creation) system provides in-game editors for builders to create and modify rooms, mobiles (NPCs), and objects without requiring server restarts or direct database access.

**Misusing these tools can corrupt game data.** Always work within assigned vnum blocks, validate changes before publishing, and maintain backups of work in the immortal database.

## Table of Contents

- [Overview](#overview)
- [Quick Start Guide](#quick-start-guide)
- [Dual-Database Architecture](#dual-database-architecture)
- [Block System (Vnum Allocation)](#block-system-vnum-allocation)
- [Room Editor (redit)](#room-editor-redit)
- [Mobile Editor (medit)](#mobile-editor-medit)
- [Object Editor (oedit)](#object-editor-oedit)
- [Publishing to Production](#publishing-to-production)
- [Builder Workflow](#builder-workflow)

---

## Overview

The OLC system consists of three primary editors:
- **redit** - Room editor for creating and modifying world locations
- **medit** - Mobile editor for creating and modifying NPCs/monsters
- **oedit** - Object editor for creating and modifying items/equipment

Each editor operates on the dual-database architecture, allowing builders to work in isolation before publishing changes to production.

---

## Quick Start Guide

This section provides a practical walkthrough for building a new zone.

### Step 1: Pick or Create a Zone

To list existing zones:
```
> show zones
```

To create a new zone:
```
> zonefile new 5 Cizra - Temple of Stupidity
Success! new zone: Cizra - Temple of Stupidity with vnums from 45660 to 45664
```

### Step 2: Assign the Zone

Have a LOW assign the vnum block to yourself (or another builder):
```
> @set blocka cizra 45660 45664
> save (or force builder save)
```

### Step 3: Make Changes

**Edit rooms:**
```
rload 1               # Load any halfway-built rooms from immortal database
goto 45664
redit                 # Describe and connect rooms
rsave 1               # Save work to immortal database
```

**Add mobs:**
```
show mob rabbit                      # Find a similar mob to use as reference
load mob 44784                       # Load it to inspect
medit mod rabbit                     # Change name to Lagomorph, describe appropriately
medit save lagomorph 45664           # Save to your vnum
```

**Change mobs:**
```
medit load 45664                     # Load existing mob
medit mod lagomorph                  # Make further changes
medit save lagomorph 45664           # Save changes
```

**Add items:**
```
oedit create                         # Start fresh
oedit mod hairball                   # Change name to carrot, add descriptions
oedit save carrot 45664              # Save to your vnum
```

**Change items:**
```
oedit load 45664                     # Load existing object
oedit mod carrot                     # Make changes
oedit resave carrot                  # Delete original and re-save (use with caution)
oedit save carrot 45664              # Save to vnum
```

### Step 4: Edit the Zonefile

The zonefile is located in `lib/zonefiles/<zone_number>` (e.g., `lib/zonefiles/45660`). Edit it to configure mob/object spawning and reset behavior.

### Step 5: Publish to Production

Publish your content from the immortal database to the sneezy (production) database:
```
low mvroom Cizra 1 45660-45664       # Publish rooms
low mvmob Cizra 45664                # Publish mobs
low mvobj Cizra 45664                # Publish objects
```

### Step 6: Enable the Zone

In your zonefile header, change the enabled flag from 0 to 1:
```
#45660
Temple of Stupidity~
45664 45 2 1    ; <-- Changed from 0 to 1
```

### Step 7: Reboot and Test

Reboot the MUD and watch your zone load automatically without needing `rload`.

### Step 8: Connect to the World

To connect your zone to the existing world, you'll need to edit an existing room to add an exit. Repeat the process of:
1. Getting temporary block assignment for the connecting room
2. Editing the room to add an exit
3. Saving to immortal database
4. Publishing to production

---

## Dual-Database Architecture

SneezyMUD separates builder workspace from production data:

| Database | Constant | Purpose | Schema Differences |
|----------|----------|---------|-------------------|
| `immortal` | `DB_IMMORTAL` | Builder workspace | Has `owner` column and `block` column |
| `sneezy` | `DB_SNEEZY` | Production data | No owner tracking |

### Schema Differences

**immortal.room table:**
```sql
PRIMARY KEY (owner, vnum)
owner VARCHAR(32)  -- Builder who owns this version
block INT          -- Version/revision number
```

**sneezy.room table:**
```sql
PRIMARY KEY (vnum)
-- No owner or block columns
```

This allows multiple builders to have their own versions of the same vnum during development, identified by owner name and block number.

---

## Block System (Vnum Allocation)

Builders are assigned vnum ranges via the block system to prevent conflicts.

### Block Assignment

Each builder has two block ranges assigned via the `@set blocka` and `@set blockb` commands:

```
@set blocka <builder> <start> <end>    # Primary block
@set blockb <builder> <start> <end>    # Secondary block
```

**Example:**
```
@set blocka Cizra 45660 45664
save
```

### Block Numbers

Blocks 1 and 2 are the primary working blocks:
- **Block 1**: Primary development version
- **Block 2**: Secondary development version

Blocks 101 and 102 serve as backup slots for rollback capability.

### Access Validation

The `limitPowerCheck()` function validates every OLC operation:

```cpp
int limitPowerCheck(cmdTypeT cmd, int vnum) {
    // Bypass for immortals with unrestricted access
    if (hasWizPower(POWER_REDIT_ENABLED))
        return TRUE;

    // Check if vnum is within assigned blocks
    if (vnum >= desc->blockastart && vnum <= desc->blockaend)
        return TRUE;

    if (vnum >= desc->blockbstart && vnum <= desc->blockbend)
        return TRUE;

    sendTo("You don't have access to that vnum range.");
    return FALSE;
}
```

---

## Room Editor (redit)

The room editor provides a 16-option menu system for modifying all room properties.

**Source:** `code/code/misc/create_rooms.cc`

### Command Syntax

```
redit                    # Edit current room
redit <vnum>            # Edit specified room
redit create <vnum>     # Create new room
```

### Access Requirements

| Permission | Purpose |
|------------|---------|
| `POWER_REDIT` | Basic room editing |
| `POWER_REDIT_ENABLED` | Unrestricted vnum access |
| Minimum level | `GOD_LEVEL1` (51) |

### Room Editor Menu

The 16-option menu:

| Option | Field | Purpose |
|--------|-------|---------|
| 1 | description | Main room description (1024 chars) |
| 2 | exdscr | Extra descriptions (512 chars) |
| 3 | exit | Door/exit configuration (50 chars) |
| 4 | extra | Extra description management (512 chars) |
| 5 | flags | Room flags (ALWAYS_LIT, DEATH, etc.) |
| 6 | height | Vertical dimension |
| 7 | line | Description line editing |
| 8 | max_capacity | Maximum occupants |
| 9 | name | Room name (80 chars) |
| 10 | river | River flow configuration |
| 11 | sector_type | Terrain type (58 options) |
| 12 | teleport | Teleport destination (100 chars) |
| 13 | copy | Copy from another room |
| 14 | replace | Replace field content |
| 15 | list | List/display options |
| 16 | autoformat | Auto-format descriptions |

### Room Flags

| Flag | Effect |
|------|--------|
| `ROOM_ALWAYS_LIT` | Base light level of 18 |
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

### Sector Types

Organized by climate zone (arctic, temperate, tropical) plus special types. Common sectors:
- Cities (urban environments)
- Roads (paved/dirt)
- Forests (various tree densities)
- Mountains/Hills (elevated terrain)
- Water (rivers, oceans, underwater)
- Deserts (arid environments)
- Special (fire, astral, etc.)

### Exit Configuration

Each room supports up to 10 exits:

| Direction | Constant |
|-----------|----------|
| North | `DIR_NORTH` (0) |
| East | `DIR_EAST` (1) |
| South | `DIR_SOUTH` (2) |
| West | `DIR_WEST` (3) |
| Up | `DIR_UP` (4) |
| Down | `DIR_DOWN` (5) |
| Northeast | `DIR_NORTHEAST` (6) |
| Northwest | `DIR_NORTHWEST` (7) |
| Southeast | `DIR_SOUTHEAST` (8) |
| Southwest | `DIR_SOUTHWEST` (9) |

**Exit Flags:**

| Flag | Effect |
|------|--------|
| `EXIT_CLOSED` | Door is shut |
| `EXIT_LOCKED` | Requires key to open |
| `EXIT_SECRET` | Hidden from observation |
| `EXIT_DESTROYED` | Door has been broken |
| `EXIT_TRAPPED` | Trap is set on door |
| `EXIT_CAVED_IN` | Passage blocked by debris |
| `EXIT_WARDED` | Magical barrier |

### Room Save/Load

**rsave - Save to immortal database:**
```
rsave 1              # Save current room to block 1
rsave 1 45660       # Save specific vnum to block 1
rsave 1 45660-45664 # Save vnum range to block 1
rsave 2 45660       # Save to block 2
```

**Implementation:**
```cpp
void RoomSave(TBeing* ch, int vnum, int block) {
    TRoom* room = real_roomp(vnum);

    db.query("INSERT INTO immortal.room "
             "(vnum, owner, block, name, description, ...) "
             "VALUES (%i, '%s', %i, '%s', '%s', ...) "
             "ON DUPLICATE KEY UPDATE name='%s', description='%s', ...",
             vnum, ch->getName(), block, room->name, room->descr, ...);

    // Save related tables: roomextra, roomexit
}
```

**rload - Load from immortal database:**
```
rload 1              # Load current room from block 1
rload 1 45660       # Load specific vnum from block 1
rload 1 45660-45664 # Load vnum range from block 1
```

---

## Mobile Editor (medit)

The mobile editor provides a 30-option menu system for modifying all NPC properties.

**Source:** `code/code/misc/create_mobs.cc`

### Command Syntax

```
medit create               # Create new mobile from scratch
medit load <vnum>         # Load existing mobile for editing
medit mod <name>          # Modify loaded mobile
medit save <name> <vnum>  # Save mobile to vnum
medit copy <vnum>         # Copy from existing mobile
medit list                # List mobile fields
```

### Mobile Editor Menu

The 30-option menu:

| Option | Field | Purpose |
|--------|-------|---------|
| 1 | name | Keywords for referencing mob |
| 2 | short_desc | Name shown in room/combat |
| 3 | long_desc | Description when standing |
| 4 | description | Look description |
| 5 | action_flags | ACT_* flags (behavior) |
| 6 | affect_flags | AFF_* flags (status effects) |
| 7 | faction | Faction alignment |
| 8 | attacks | Attack frequency |
| 9 | class | Class type |
| 10 | level | Experience level |
| 11 | hitroll | Attack accuracy bonus |
| 12 | ac | Armor class |
| 13 | hpbonus | HP bonus |
| 14 | damroll | Damage bonus |
| 15 | gold | Starting money |
| 16 | race | Racial type |
| 17 | weight | Body weight |
| 18 | height | Body height |
| 19 | pos_def | Default position |
| 20 | pos | Current position |
| 21 | sex | Gender |
| 22 | spec | Special procedure |
| 23 | skin | Skin type (for skinning) |
| 24 | vision | Vision type |
| 25 | can_be_seen | Visibility flags |
| 26 | max_exist | Global spawn limit |
| 27 | local_num | Zone spawn limit |
| 28 | intelligence | NPC intelligence level |
| 29 | immunities | Damage immunities |
| 30 | extra_desc | Extra descriptions |

### Combat Stats Scaling

Mobs use scaling factors rather than absolute values:

| Stat | Field | Description |
|------|-------|-------------|
| HP Level | `hpLevel` | HP scaling (damLevel * 100 + hpbonus) |
| Damage Level | `damLevel` | Damage scaling multiplier |
| AC Level | `acLevel` | Armor class scaling |
| Attack Level | `attackLevel` | Attack bonus scaling |

### stripSpellAffects() - Critical Pre-Save Cleanup

Before saving, the editor removes all spell affects to prevent corruption:

```cpp
void stripSpellAffects(TMonster* mob) {
    affectedData* af, *next_af;

    for (af = mob->affected; af; af = next_af) {
        next_af = af->next;

        // Remove spell-based affects but keep permanent ones
        if (af->type >= 0 && af->type < MAX_SKILL) {
            mob->affectRemove(af);
        }
    }
}
```

This prevents spell effects applied during testing from being persisted to the database.

### Mobile Save

```cpp
int msave(TBeing* ch, const char* arg) {
    // Validate vnum is within builder's assigned block
    if (!limitPowerCheck(CMD_MEDIT, vnum)) {
        incorrectCommand();
        return;
    }

    TMonster* edited_mob = /* get edited mob */;

    // CRITICAL: Strip spell affects before saving
    stripSpellAffects(edited_mob);

    // Save to immortal database
    db.query("INSERT INTO immortal.mob "
             "(vnum, owner, name, short_desc, long_desc, ...) "
             "VALUES (%i, '%s', '%s', '%s', '%s', ...) "
             "ON DUPLICATE KEY UPDATE name='%s', ...",
             vnum, ch->getName(), edited_mob->name, ...);

    // Save related tables: mob_extra, mob_imm
}
```

---

## Object Editor (oedit)

The object editor handles type-specific properties via the `itemTypeT` system.

**Source:** `code/code/misc/create_objs.cc`

### Command Syntax

```
oedit create              # Create new object from scratch
oedit load <vnum>        # Load existing object for editing
oedit mod <name>         # Modify loaded object
oedit save <name> <vnum> # Save object to vnum
oedit resave <name>      # Delete and re-save (dangerous!)
oedit copy <vnum>        # Copy from existing object
```

### Object Properties

**Core Fields:**

| Field | Description |
|-------|-------------|
| name | Keywords for referencing |
| short_desc | Name shown in inventory |
| long_desc | Description when on ground |
| action_desc | Special action description |
| type | Item type (67+ types) |
| extra_flags | ITEM_* flags (GLOW, MAGIC, NODROP, etc.) |
| wear_flags | ITEM_WEAR_* flags (where equippable) |
| weight | Item weight |
| price | Base value |
| material | Material type (affects durability) |
| volume | Item volume |
| max_struct | Maximum structure points |
| cur_struct | Current structure points |
| decay | Decay timer (-1 = never) |

### Type-Specific Values (val0-val3)

Each object type uses the four value fields differently:

**Weapons (ITEM_WEAPON):**
- val0: Sharpness (curSharp + maxSharp)
- val1: Damage (damLevel + damDev)
- val2: Weapon types and frequencies
- val3: Reserved

**Armor (ITEM_ARMOR):**
- val0: Armor class bonus
- val1-3: Reserved

**Containers (ITEM_BAG, ITEM_CHEST, etc.):**
- val0: Maximum carry weight
- val1: Container flags, trap type, trap damage
- val2: Key vnum for locked containers
- val3: Maximum carry volume

**Drinks (ITEM_DRINKCON, ITEM_POTION):**
- val0: Maximum drink units
- val1: Current drink units
- val2: Liquid type
- val3: Drink flags (frozen, poisoned)

**Magic Items (ITEM_SCROLL, ITEM_WAND, ITEM_STAFF):**
- val0: Magic level
- val1: Max charges (wand/staff) or spell 1 (scroll)
- val2: Current charges (wand/staff) or spell 2 (scroll)
- val3: Spell number (wand/staff) or spell 3 (scroll)

### Object Save

```cpp
int osave(TBeing* ch, const char* arg) {
    // Validate vnum access
    if (!limitPowerCheck(CMD_OEDIT, vnum)) {
        incorrectCommand();
        return;
    }

    TObj* edited_obj = /* get edited object */;

    // Save to immortal database
    db.query("INSERT INTO immortal.obj "
             "(vnum, owner, name, short_desc, type, ...) "
             "VALUES (%i, '%s', '%s', '%s', %i, ...) "
             "ON DUPLICATE KEY UPDATE name='%s', ...",
             vnum, ch->getName(), edited_obj->name, ...);

    // Save related tables: objextra, objaffect
}
```

### resave Command - Dangerous Operation

The `resave` command deletes the existing object and creates a new one:

```cpp
int oresave(TBeing* ch, const char* arg) {
    // Validate power
    if (!hasWizPower(POWER_OEDIT_IMP_POWER)) {
        sendTo("You need POWER_OEDIT_IMP_POWER for resave.");
        return;
    }

    // Delete existing
    db.query("DELETE FROM immortal.obj WHERE vnum=%i AND owner='%s'",
             vnum, ch->getName());

    // Save new version
    osave(ch, arg);
}
```

**Warning:** If the MUD crashes between delete and save, the object is lost permanently.

---

## Publishing to Production

The `low` command publishes content from the immortal database to production.

### Publishing Commands

```
low mvroom <builder> <block> <vnum_list>    # Publish rooms
low mvmob <builder> <vnum_list>             # Publish mobiles
low mvobj <builder> <vnum_list>             # Publish objects
low mvresponse <builder> <vnum_list>        # Publish mob responses
```

### Access Requirements

| Permission | Purpose |
|------------|---------|
| `POWER_LOW` | Basic publishing access |
| Minimum level | `GOD_LEVEL1` (51) |

### Publishing Flow - Rooms

```cpp
void doLowMvRoom(TBeing* ch, const char* argument) {
    TDatabase immo(DB_IMMORTAL);
    TDatabase beta(DB_SNEEZY);

    // Transaction-wrapped for atomicity
    beta.query("BEGIN");

    for (int vnum : vnums) {
        // Fetch from immortal
        immo.query("SELECT * FROM room "
                   "WHERE owner='%s' AND block=%i AND vnum=%i",
                   builder, block, vnum);

        if (!immo.fetchRow()) {
            ch->sendTo(format("Not found: %i\n") % vnum);
            beta.query("ROLLBACK");
            return;
        }

        // Delete existing in production
        beta.query("DELETE FROM room WHERE vnum=%i", vnum);

        // Insert new version
        beta.query("INSERT INTO room "
                   "(vnum, name, description, sector_type, ...) "
                   "VALUES (%i, '%s', '%s', %i, ...)",
                   vnum, immo["name"].c_str(), ...);

        // Copy related tables (roomextra, roomexit)
    }

    beta.query("COMMIT");
}
```

### Publishing Flow - Mobiles

```cpp
void doLowMvMob(TBeing* ch, const char* argument) {
    for (int vnum : vnums) {
        // Fetch from immortal.mob
        immo.query("SELECT * FROM mob WHERE owner='%s' AND vnum=%i",
                   builder, vnum);

        // Special processing: clear ACT_STRINGS_CHANGED bit
        int actions = convertTo<int>(immo["actions"]);
        actions &= ~ACT_STRINGS_CHANGED;

        // Delete and insert to production
        beta.query("DELETE FROM mob WHERE vnum=%i", vnum);
        beta.query("INSERT INTO mob ...");

        // Copy mob_extra, mob_imm tables
    }
}
```

### Publishing Flow - Objects

```cpp
void doLowMvObj(TBeing* ch, const char* argument) {
    for (int vnum : vnums) {
        // Fetch from immortal.obj
        immo.query("SELECT * FROM obj WHERE owner='%s' AND vnum=%i",
                   builder, vnum);

        // Special processing: strip ITEM_STRUNG and ITEM_PROTOTYPE bits
        int extra_flags = convertTo<int>(immo["extra_flags"]);
        extra_flags &= ~ITEM_STRUNG;
        extra_flags &= ~ITEM_PROTOTYPE;

        // Delete and insert
        beta.query("DELETE FROM obj WHERE vnum=%i", vnum);
        beta.query("INSERT INTO obj ...");

        // Copy objextra, objaffect tables
    }
}
```

---

## Builder Workflow

### Phase 1: Zone Assignment

Request a vnum range from a LOW:

```
Request: "Can I get vnums 45660-45664 for a new temple zone?"
LOW executes:
  @set blocka YourName 45660 45664
  save
```

### Phase 2: Create Zone Header

Create the zonefile header in `lib/zonefiles/45660`:

```
#45660
Temple of Stupidity~
45664 45 2 0

D 0 45660 0 1          ; Close entrance door
M 0 45661 1 45660      ; Load priest in entrance
S
```

**Header format:**
```
#<zone_number>
<Zone Name>~
<top_room> <lifespan> <reset_mode> <enabled>
```

### Phase 3: Design and Build Rooms

```
goto 45660
redit create 45660

[In room editor menu:]
9 (name)        : Temple Entrance
1 (description) : A grand entrance hall with marble pillars...
5 (flags)       : INDOORS
11 (sector_type): TEMPERATE_CITY
3 (exit)        : 1 (SOUTH) 45659
```

Build all rooms in sequence and connect with exits.

Save all rooms:
```
rsave 1 45660-45664
```

### Phase 4: Create NPCs/Mobs

```
medit create
medit mod priest

[Edit fields:]
2 (short_desc)  : a temple priest
3 (long_desc)   : A temple priest stands here, blessing visitors.
10 (level)      : 15
11 (hitroll)    : 5
12 (ac)         : 50

medit save priest 45661
```

### Phase 5: Create Objects/Items

```
oedit create
oedit mod robe

[Edit fields:]
name         : robe temple white
short_desc   : a white temple robe
type         : ITEM_ARMOR (9)
wear_flags   : ITEM_WEAR_BODY
val0 (AC)    : -2

oedit save robe 45660
```

### Phase 6: Configure Zonefile

Edit `lib/zonefiles/45660` to add reset commands:

```
#45660
Temple of Stupidity~
45664 45 2 1

D 0 45660 0 1                ; Close entrance door
M 0 45661 1 45660            ; Load priest in entrance
G 1 45660 1                  ; Give priest the robe
E 1 45660 1 4                ; Equip robe on body
M 0 45662 1 45662            ; Load guard in offering room
O 0 45662 1 45664            ; Load chalice in vault
S
```

**Reset command syntax:**
```
M <if_flag> <mob_vnum> <max> <room_vnum>    ; Load mob
G <if_flag> <obj_vnum> <max>                ; Give to last mob
E <if_flag> <obj_vnum> <max> <slot>         ; Equip on last mob
O <if_flag> <obj_vnum> <max> <room_vnum>    ; Load in room
```

### Phase 7: Test In-Game

Load your work from immortal database:

```
rload 1 45660-45664    ; Load rooms
goto 45660             ; Visit zone
boot zone 45660        ; Load mobs/objects from zonefile
```

Test functionality:
- Walk through all rooms
- Fight mobs
- Pick up objects
- Check descriptions
- Verify exits

### Phase 8: Publish to Production

Once testing is complete, publish to sneezy database:

```
low mvroom YourName 1 45660-45664
low mvmob YourName 45661-45663
low mvobj YourName 45660-45662
```

Enable the zone in the zonefile header (change 0 to 1):
```
#45660
Temple of Stupidity~
45664 45 2 1    ; <-- Changed from 0 to 1
```

### Phase 9: Connect to Existing World

Edit an existing room to add an exit to your new zone:

```
goto 45659                           ; Go to adjacent existing room
@set blocka YourName 45659 45659    ; Get temporary access
redit 45659
3 (exit) : 0 (NORTH) 45660          ; Add exit to your zone
rsave 1 45659                        ; Save to immortal
low mvroom YourName 1 45659         ; Publish connection
```

Reboot the MUD or use `boot zone 45660` to activate.

---

## Common Issues and Solutions

### "You don't have access to that vnum range"

**Cause:** Vnum is outside your assigned blocks.

**Solution:** Check your assigned blocks with `stat self` and request expansion from a LOW, or use a vnum within your range.

### "Room does not exist"

**Cause:** Trying to edit a room that hasn't been created yet.

**Solution:** Use `redit create <vnum>` to create the room first.

### "Not found" during low mvroom

**Cause:** Room hasn't been saved to immortal database, or wrong builder/block specified.

**Solution:**
```
rsave 1 <vnum>                    ; Save to immortal first
low mvroom YourName 1 <vnum>     ; Then publish
```

### Objects/Mobs not loading in zone

**Cause:** Zonefile references vnums that don't exist in sneezy database, or zone is disabled.

**Solution:**
1. Verify you published with `low mvmob` and `low mvobj`
2. Check zonefile header has `enabled = 1` (not 0)
3. Verify zonefile vnum references match published vnums

### Lost work after crash

**Cause:** Didn't save to immortal database before crash.

**Solution:** Save frequently with `rsave 1` commands. The immortal database persists across crashes.

---

## Related Documentation

- [Admin Systems](admin-systems.md) - Wizard powers, @set command, database migration
- [Zone Management](zone-management.md) - Zone file format and reset commands
- [Object Types](object-types.md) - Item types and val0-val3 meanings
- [Room Environment](room-environment.md) - Room flags and sector types
