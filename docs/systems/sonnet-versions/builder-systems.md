---
title: Builder Systems
category: important
keywords: [redit, medit, oedit, OLC, room editor, mobile editor, object editor, blocka, blockb, zone publishing, content creation, builder workflow, immortal database, dual database]
related: [admin-systems.md, zone-management.md, object-system.md, room-environment.md]
primary_symbols:
  functions: [limitPowerCheck, RoomSave, msave, osave, doLowMvRoom, doLowMvMob, doLowMvObj, stripSpellAffects]
  classes: [TRoom, TMonster, TObj]
  files: [code/code/cmd/cmd_low.cc, code/code/misc/immortal.cc, code/code/misc/create_rooms.cc, code/code/misc/create_mobs.cc, code/code/misc/create_objs.cc]
---

## Overview

The Online Creation system enables builders to create and modify game content without server restarts or direct database access. Three primary editors provide access to world-building capabilities: redit for rooms, medit for mobiles, and oedit for objects. The system operates on a dual-database architecture where builders work in isolation within the immortal database before publishing changes to the production sneezy database. Builders are assigned vnum ranges via the block system to prevent conflicts. Critical safety features include access validation through limitPowerCheck, pre-save cleanup via stripSpellAffects for mobiles, and transactional publishing to prevent partial updates. The workflow progresses through zone assignment, content creation in the immortal database, local testing, and publishing to production via the low command suite.

## Patterns

### Dual-Database Development Workflow

The immortal database serves as builder workspace while sneezy holds production data. Builders save work to immortal using rsave, msave, or osave commands with block numbers identifying version slots. Primary development uses blocks 1 and 2, while blocks 101 and 102 serve as backup slots for rollback capability. Publishing transfers content from immortal to sneezy via the low command suite, removing builder-specific metadata during the transfer. The immortal.room table includes owner and block columns in a composite primary key allowing multiple builders to maintain separate versions of the same vnum. The sneezy.room table uses only vnum as the primary key. This separation enables parallel development without production contamination.

### Block-Based Access Control

Every OLC operation validates vnum access through limitPowerCheck before allowing modifications. The function checks whether the requested vnum falls within the builder's assigned blocka or blockb ranges stored in their descriptor. Builders with POWER_REDIT_ENABLED bypass these restrictions for unrestricted access. Block assignment occurs via the @set blocka and @set blockb commands executed by LOW immortals. The system enforces that builders cannot modify vnums outside their assigned ranges, preventing accidental corruption of zones owned by other builders. Block ranges must be saved to persist across sessions.

### Pre-Save Data Sanitization

Before saving mobiles to the database, stripSpellAffects removes all spell-based affects to prevent temporary testing effects from becoming permanent. The function iterates through the affected list and removes any affect where the type field falls within the MAX_SKILL range, indicating a spell or skill effect rather than a permanent racial or equipment-based affect. This prevents scenarios where a builder casts bless on a mob during testing and accidentally saves the temporary stat bonuses as permanent mob properties. Objects require similar sanitization by stripping ITEM_STRUNG and ITEM_PROTOTYPE bits during publishing to remove builder-specific flags.

### Transactional Publishing

The doLowMvRoom function wraps all database operations in a transaction using BEGIN and COMMIT statements. For each vnum in the range, the function fetches the corresponding row from immortal.room, deletes the existing row from sneezy.room if present, and inserts the new version. If any vnum in the range fails to fetch from the immortal database, the entire transaction rolls back to prevent partial zone updates. This ensures that either all rooms in a range publish successfully or none do. Mobile and object publishing follows similar patterns but includes additional transformations such as clearing ACT_STRINGS_CHANGED bits for mobiles and stripping builder flags for objects.

### Type-Specific Value Encoding

Objects use four value fields that encode different properties based on the item type. Weapons store sharpness in val0 as a combination of curSharp and maxSharp packed into a single integer, damage parameters in val1, weapon type flags in val2, and reserve val3 for future use. Containers pack maximum carry weight in val0, container flags combined with trap type and damage in val1, key vnum in val2, and maximum volume in val3. Drink containers use val0 for maximum units, val1 for current units, val2 for liquid type, and val3 for drink state flags like frozen or poisoned. Magic items store spell level in val0, charges or spell numbers in val1-val3 depending on subtype. The editor presents type-appropriate prompts based on the selected item type.

## Reference

### Room Editor Commands

redit without arguments edits the current room. redit with a vnum argument edits the specified room. redit create followed by a vnum creates a new room at that vnum. The editor presents a 16-option menu covering description, extra descriptions, exits, room flags, height, max_capacity, name, river configuration, sector_type, teleport settings, copy operations, replace operations, listing, and autoformat. Option 1 handles the main room description with a 1024 character limit. Option 3 configures exits supporting ten directions including cardinal directions, up, down, and diagonal directions. Option 5 toggles room flags including ALWAYS_LIT for base light level, DEATH for instant kill, NO_MOB for mob blocking, INDOORS for weather protection, PEACEFUL for combat prevention, and ARENA for special combat rules. Option 11 selects from 58 sector types organized by climate zone.

### Mobile Editor Commands

medit create starts a fresh mobile. medit load followed by a vnum loads an existing mobile for editing. medit mod followed by a name applies modifications to the loaded mobile. medit save followed by name and vnum saves the mobile to the specified vnum. medit copy followed by a vnum copies an existing mobile as a starting template. The editor provides 30 options including name keywords, short_desc for combat display, long_desc for standing description, main description, action_flags controlling behavior, affect_flags for status effects, faction alignment, attack frequency, class type, level, hitroll accuracy bonus, armor class, hitpoint bonus, damage bonus, starting gold, race, weight, height, default position, current position, gender, special procedure assignment, skin type for crafting, vision type, visibility flags, max_exist global spawn limit, local_num zone spawn limit, intelligence level, damage immunities, and extra descriptions.

### Object Editor Commands

oedit create starts a new object. oedit load followed by a vnum loads an existing object. oedit mod followed by a name modifies the loaded object. oedit save followed by name and vnum saves to the specified vnum. oedit resave followed by a name performs a dangerous delete-and-recreate operation requiring POWER_OEDIT_IMP_POWER. oedit copy followed by a vnum copies an existing object. Core properties include name keywords, short_desc for inventory display, long_desc for ground description, action_desc for special messages, type selecting from 67 item types, extra_flags for properties like GLOW and MAGIC and NODROP, wear_flags determining equipment slots, weight, price, material type affecting durability, volume, max_struct for maximum structure points, cur_struct for current condition, and decay timer where negative one prevents decay.

### Save and Load Commands

rsave followed by block number saves the current room to the specified block in the immortal database. rsave followed by block number and vnum saves the specified room. rsave followed by block number and vnum range saves all rooms in the range. rload followed by block number loads the current room from the specified block. rload followed by block number and vnum loads the specified room. rload followed by block number and vnum range loads all rooms in the range. Mobile and object editors do not use separate load commands; loading occurs through medit load and oedit load which retrieve from the immortal database based on owner and vnum.

### Publishing Commands

low mvroom followed by builder name, block number, and vnum list publishes rooms from the immortal database to production. low mvmob followed by builder name and vnum list publishes mobiles. low mvobj followed by builder name and vnum list publishes objects. low mvresponse followed by builder name and vnum list publishes mobile response data. All commands require POWER_LOW permission and GOD_LEVEL1 minimum level. The builder name parameter determines which rows to fetch from the immortal database based on the owner column. The block parameter for rooms specifies which version to publish. Mobile and object publishing do not use block parameters and instead fetch the latest version by owner and vnum.

### Room Flags

ROOM_ALWAYS_LIT provides base light level of 18. ROOM_DEATH kills players entering. ROOM_NO_MOB prevents mobs from entering. ROOM_INDOORS provides shelter from weather. ROOM_PEACEFUL blocks combat initiation. ROOM_NO_STEAL disables stealing. ROOM_NO_ESCAPE prevents fleeing from combat. ROOM_NO_MAGIC blocks magic use. ROOM_NO_PORTAL blocks portal spells. ROOM_PRIVATE limits occupancy. ROOM_SILENCE prevents speech and sounds. ROOM_ARENA applies special arena combat rules. ROOM_SAVE_ROOM preserves items across reboots. Flags combine using bitwise OR operations and are toggled through the room editor flag menu.

### Exit Flags and Directions

Exits support ten directions mapped to constants: DIR_NORTH as 0, DIR_EAST as 1, DIR_SOUTH as 2, DIR_WEST as 3, DIR_UP as 4, DIR_DOWN as 5, DIR_NORTHEAST as 6, DIR_NORTHWEST as 7, DIR_SOUTHEAST as 8, DIR_SOUTHWEST as 9. Exit flags include EXIT_CLOSED for shut doors, EXIT_LOCKED requiring keys, EXIT_SECRET for hidden passages, EXIT_DESTROYED for broken doors, EXIT_TRAPPED for trapped doors, EXIT_CAVED_IN for blocked passages, and EXIT_WARDED for magical barriers. The room editor exit menu prompts for direction, destination vnum, door flags, and door keywords.

### Mobile Combat Stats

Mobs use scaling factors rather than absolute values. hpLevel scales hitpoints calculated as damLevel times 100 plus hpbonus. damLevel multiplies base damage. acLevel scales armor class. attackLevel scales attack bonus. The level field determines base stats before scaling factors apply. hitroll adds to attack accuracy. damroll adds flat damage bonus. The editor presents these fields separately but the underlying stat calculations use the scaling system during mob instantiation from the database.

### Object Value Fields by Type

Weapons encode curSharp and maxSharp combined in val0, damLevel and damDev packed in val1, weapon type bitflags in val2, and reserve val3. Armor stores AC bonus in val0 with val1 through val3 reserved. Containers place max carry weight in val0, container flags combined with trap type and trap damage in val1, key vnum in val2, and max volume in val3. Drink containers use val0 for max units, val1 for current units, val2 for liquid type, val3 for drink state flags. Scrolls store magic level in val0, spell 1 in val1, spell 2 in val2, spell 3 in val3. Wands and staves store magic level in val0, max charges in val1, current charges in val2, spell number in val3.

## Implementation

### Access Validation Flow

limitPowerCheck receives a command type and vnum as parameters. The function first checks whether the caller has POWER_REDIT_ENABLED, POWER_MEDIT_ENABLED, or POWER_OEDIT_ENABLED depending on the command type, returning TRUE immediately if unrestricted access is granted. For builders without these powers, the function retrieves blockastart, blockaend, blockbstart, and blockbend values from the descriptor. If the vnum falls within either range, the function returns TRUE. Otherwise, it sends an error message and returns FALSE. Command handlers invoke this function before performing any modification operations and abort if validation fails.

### Room Save Database Operations

RoomSave constructs an INSERT statement with ON DUPLICATE KEY UPDATE clause targeting the immortal.room table. The function retrieves room properties from the TRoom object including name, description, sector type, room flags, height, max capacity, teleport settings, and river configuration. The owner field receives the builder's name from getName. The block field receives the block parameter. The vnum serves as part of the composite primary key. After inserting the main room row, the function deletes existing rows from immortal.roomextra and immortal.roomexit tables matching the owner, block, and vnum, then inserts new rows for current extra descriptions and exits. This ensures related tables stay synchronized with the main room data.

### Mobile Save Database Operations

msave first invokes stripSpellAffects to remove temporary effects from the edited mob. The function constructs an INSERT with ON DUPLICATE KEY UPDATE targeting immortal.mob. Mob properties extracted include name keywords, short_desc, long_desc, main description, action flags, affect flags, faction, attack count, class, level, hitroll, armor class, hp bonus, damage bonus, gold, race, weight, height, default position, current position, gender, special proc assignment, skin type, vision type, visibility flags, max exist, local num, and intelligence. The owner field receives the builder's name. After the main row, the function handles immortal.mob_extra for extra descriptions and immortal.mob_imm for immunity flags, deleting old rows and inserting new ones.

### Object Save Database Operations

osave validates vnum access through limitPowerCheck before proceeding. The function builds an INSERT with ON DUPLICATE KEY UPDATE for immortal.obj including name keywords, short_desc, long_desc, action_desc, item type, extra flags, wear flags, weight, price, material, volume, max struct, cur struct, decay timer, and the four value fields val0 through val3. The owner field captures the builder name. Related tables immortal.objextra and immortal.objaffect receive similar delete-and-insert treatment for extra descriptions and stat affects applied by the object when equipped. The resave variant deletes the existing object row before saving, creating a crash window where the object can be permanently lost.

### Publishing Transformation Logic

doLowMvRoom opens a database transaction, iterates vnums, fetches each from immortal.room filtering by owner and block, deletes the corresponding vnum from sneezy.room regardless of existence, and inserts the new version without owner or block columns. Related tables roomextra and roomexit undergo similar processing. doLowMvMob fetches from immortal.mob, strips the ACT_STRINGS_CHANGED bit from the actions field to clear builder editing markers, deletes from sneezy.mob, and inserts the cleaned version. doLowMvObj strips ITEM_STRUNG and ITEM_PROTOTYPE bits from extra_flags before inserting to sneezy.obj. All publishing functions validate that each vnum exists in the immortal database before deletion, rolling back the transaction on any fetch failure.

### stripSpellAffects Implementation

The function iterates the mob's affected linked list using a cached next pointer pattern. For each affect, it checks whether the type field falls between 0 and MAX_SKILL, indicating a spell or skill effect. Permanent affects use negative type values or values above MAX_SKILL. When a spell affect is detected, the function calls affectRemove to unlink it from the list and deallocate memory. The cached next pointer prevents use-after-free when removing the current node. This ensures only intrinsic mob properties persist to the database rather than temporary buffs applied during testing.

### Zone Reset Processing

The zonefile parser reads reset commands during zone boot. M commands load a mob at a specified room vnum with a maximum count limit. G commands give an object to the most recently loaded mob. E commands equip an object on the most recently loaded mob at a specified equipment slot. O commands load an object in a room with a maximum count. D commands set door states. The if_flag parameter on each command determines whether the command executes unconditionally or only if the previous command succeeded. The lifespan field in the zone header determines how many game pulses pass between resets. Reset mode 2 resets only when no players are present in the zone.

## Troubleshooting

### Vnum Access Denial

When limitPowerCheck rejects a vnum, verify assigned block ranges using stat self to display blockastart, blockaend, blockbstart, and blockbend. If the vnum falls outside both ranges, request a LOW immortal to expand the range using @set blocka or @set blockb followed by save to persist the change. Alternatively, select a different vnum within the existing range. Immortals with POWER_REDIT_ENABLED bypass all vnum restrictions but should still follow block assignments to maintain organizational clarity.

### Room Creation Failure

Attempting to edit a non-existent room produces an error. Use redit create followed by the vnum to instantiate a new room in memory before editing properties. The room exists only in memory until rsave commits it to the immortal database. Crashes before saving lose all work. Creating a room at a vnum already present in memory overwrites the in-memory instance without affecting the database until explicit save.

### Publishing Failures

The low mvroom command reports "Not found" when the specified vnum does not exist in immortal.room for the given builder and block combination. Verify the room was saved using rsave with the correct block number. Check the builder name parameter matches the owner field exactly, accounting for case sensitivity. Verify the block parameter matches the saved block number. Transaction rollback occurs if any single vnum in a range fails to fetch, preventing partial zone publication.

### Missing Zone Content

Objects and mobs failing to appear after zone boot indicates the zonefile references vnums not present in sneezy database or the zone is disabled. Verify publication completed using low mvmob and low mvobj before enabling the zone. Check the zonefile header enabled flag is set to 1 rather than 0. Verify zonefile reset commands reference the correct vnums matching published content. Syntax errors in the zonefile prevent parsing beyond the error point.

### Data Loss After Crash

Work not saved to the immortal database disappears after crashes or reboots. In-memory edits exist only in the running process. Establish a habit of frequent rsave, msave, and osave operations after completing logical units of work. Blocks 101 and 102 provide backup slots for saving known-good versions before making experimental changes. The immortal database persists across all crashes, but only data explicitly saved reaches the database.

### Resave Crash Window

oedit resave deletes the existing object row before saving the new version. If the server crashes between deletion and insertion, the object is lost permanently with no recovery mechanism. Use resave only when absolutely necessary, such as fixing corrupted value fields that prevent normal saves. Always maintain a backup copy in block 101 or block 102 before using resave. Consider requesting a LOW immortal to perform the resave with a database backup taken immediately before.

### Spell Affects Persisting

Mobs saved without stripSpellAffects processing retain temporary stat bonuses as permanent properties. The saved mob appears overpowered because bless, strength, haste, or other buff effects become intrinsic stats. Reload the mob, allow spell affects to expire or use dispel magic to remove them, verify stats are correct, then save again. The medit save command invokes stripSpellAffects automatically but direct database manipulation bypasses this safety check.

### Flag Contamination

Objects published without stripping ITEM_STRUNG and ITEM_PROTOTYPE flags carry builder markers into production. ITEM_STRUNG indicates a unique player-customized item that should never exist as a zone reset. ITEM_PROTOTYPE marks unfinished builder work. The doLowMvObj function strips these bits during publishing, but direct database copying circumvents this protection. Always use the low command suite for publishing rather than manual SQL operations.

### Bidirectional Exit Inconsistency

Creating an exit from room A to room B without creating the reverse exit from B to A produces one-way passages that confuse players. The room editor does not automatically create reverse exits. After adding an exit in one direction, switch to the destination room and manually add the return exit. Some zones intentionally use one-way exits for puzzles or one-way drops, so verify design intent before assuming bidirectionality.

### Zone Connection Isolation

Zones not connected to the existing world remain inaccessible to players. After publishing zone content, identify an appropriate connection point in an existing room. Request temporary blocka access to the connection room vnum from a LOW immortal. Edit the existing room to add an exit to the new zone entrance. Add a reverse exit from the new zone entrance to the existing room. Save both rooms to immortal and publish both to sneezy. Verify the connection works by walking through both directions before removing temporary block access.
