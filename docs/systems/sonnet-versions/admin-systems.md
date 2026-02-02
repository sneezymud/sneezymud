---
title: Administrative Systems
category: critical
keywords: [wizard powers, set command, LOW migration, force command, privilege escalation, database migration, remote execution, immortal database, vnum limits, power groups]
related: [builder-systems.md, persistence-storage.md, cgi-security.md, snoop-switch.md]
primary_symbols:
  functions: [hasWizPower, setWizPower, remWizPower, saveWizPowers, limitPowerCheck, doSet, doForce, doAt, doLoad, doLow]
  classes: [TPerson, TDatabase]
  files: [code/code/misc/wiz_powers.cc, code/code/misc/wiz_powers.h, code/code/cmd/cmd_set.cc, code/code/cmd/cmd_low.cc, code/code/misc/immortal.cc]
---

## Overview

SneezyMUD's administrative layer controls server access, player manipulation, and content migration through a capability-based security model. The wizard powers system grants granular permissions to immortals for specific commands and operations. Three critical subsystems intersect here: the power grant mechanism, the universal character modification interface, and the dual-database migration pipeline for moving builder content from development to production.

The architecture separates base access control from privilege escalation through two power tiers. Standard immortals with POWER_SET can modify mortals and NPCs within vnum restrictions enforced by limitPowerCheck. Administrative immortals with both POWER_SET and POWER_SET_IMP_POWER gain unrestricted modification rights including the ability to grant powers to other immortals. This creates a trust boundary where certain operations require administrative oversight.

Remote execution commands enable immortals to manipulate the game state without direct presence. The force command executes commands as other beings, at command teleports to remote locations for inspection, and load command creates entities from templates. Each respects vnum restrictions and power requirements while handling DELETE flags to prevent use-after-free when operations destroy their targets.

Database migration operates on a dual-database model where builders work in the immortal database with owner-scoped primary keys allowing multiple versions of the same vnum. The LOW command family transfers finalized content to the production sneezy database through atomic transactions that delete existing entries before inserting new data. This supports collaborative building while maintaining production stability.

Critical security properties include preventing privilege escalation through power grant workflows, enforcing vnum boundaries for builder zones, validating parameter bounds before enum conversion, and wrapping database operations in transactions to prevent partial state corruption. Multiple privilege escalation chains exist through combinations of POWER_SWITCH, POWER_FORCE, POWER_SET_IMP_POWER, and POWER_LOW that require careful power assignment discipline.

## Patterns

### Capability-Based Access Control

The hasWizPower function checks if an immortal possesses a specific power from the MAX_POWER_INDEX array. Powers are individual capabilities like POWER_GOTO for teleportation or POWER_FORCE for command execution. Special enforcement exists for POWER_IDLED which restricts immortals to exactly five powers: POWER_BUILDER, POWER_GOD, POWER_WIZARD, POWER_GOTO, and POWER_IDLED itself. All other power checks return false when POWER_IDLED is set without POWER_WIZARD.

IMP_POWER variants bypass safety restrictions. POWER_GOTO enforces vnum restrictions but POWER_GOTO_IMP_POWER allows unrestricted teleportation. POWER_SET limits stat modifications to 100 but POWER_SET_IMP_POWER permits values up to 205. POWER_LOAD respects max_exist limits but POWER_LOAD_IMP_POWER creates unlimited instances of restricted items. This pattern appears across editing commands with a base power for constrained operations and an IMP_POWER variant for administrative overrides.

Power groups bundle related capabilities into packages assigned through doSet wizpower subcommand. The basic group grants entry-level powers, rooms/mobs/objs grant builder-specific permissions, demi grants mid-tier capabilities, god grants 23 administrative powers, and allpowers grants all 127 powers without restriction. These groups use setWizPower and remWizPower to modify the boolean arrays.

### Vnum Boundary Enforcement

The limitPowerCheck function validates whether an immortal can interact with a specific vnum based on assigned build zones. Each immortal descriptor contains blockastart/blockaend for primary zone and blockbstart/blockbend for secondary zone plus an office vnum. Commands like CMD_GOTO, CMD_LOAD, and CMD_SHOW check if the target vnum falls within these ranges.

POWER_NO_LIMITS bypasses all vnum checks completely. Generic vnums for template mobs and objects using isGenericMob and isGenericObj also bypass restrictions. Vnums 0-100 are universally accessible for goto operations enabling access to common areas regardless of assignments.

Vnum enforcement creates a sandbox where builders can only modify content in their assigned zones. Combined with the immortal database owner column this prevents accidental modification of other builders' work. The force command extends this by checking limitPowerCheck on target mob vnums preventing force operations on mobs outside assigned zones.

### Dual-Database Migration

The immortal and sneezy databases share identical schemas except for primary keys. The immortal.room table uses PRIMARY KEY owner, vnum allowing multiple builders to have separate versions of the same room vnum. The sneezy.room table uses PRIMARY KEY vnum enforcing uniqueness in production. Similarly for immortal.mob and immortal.obj tables with owner columns.

Migration commands query source data from immortal WHERE owner equals the builder name, delete existing entries from sneezy WHERE vnum matches, then insert the immortal data. The LOW command wraps these in BEGIN/COMMIT transactions ensuring atomic replacement. If insert fails after delete, rollback prevents data loss.

Special processing strips development flags during migration. The mvobj command clears ITEM_STRUNG and ITEM_PROTOTYPE bits from action_flag. The mvmob command clears ACT_STRINGS_CHANGED from actions field. These flags indicate unfinalized content that should not persist in production.

The mvresponse command handles the mobresponses table which references mob vnums. Without a vnum list it displays all responses owned by a builder for review. With vnums it transfers only specified responses enabling selective migration when responses span multiple development iterations.

### DELETE Flag Propagation in Remote Execution

Commands executed through doForce may return DELETE_THIS if they kill or destroy the target being. The force implementation checks return codes and deletes the victim pointer when IS_SET_DELETE returns true. Iterator caching prevents use-after-free when forcing multiple targets in room or all modes.

The doAt command teleports the immortal, executes parseCommand, then returns them to origin. If parseCommand returns DELETE_THIS the immortal died during execution and doAt propagates DELETE_THIS to its caller preventing continued execution on a freed being pointer.

The doLoad command handles DELETE_ITEM when loaded objects trigger traps or events causing self-destruction. After creating an object through read_object_vnum and adding to inventory, any operations that might destroy the object require checking IS_SET_DELETE on return codes before dereferencing the object pointer.

Ownership rules determine deletion responsibility. If the caller provided a victim parameter, return DELETE_VICT so the caller deletes. If the command resolved the victim internally through get_char_room_vis or similar lookups, delete the victim directly and clear the flag with REM_DELETE before returning.

### Power Persistence and Synchronization

The TPerson class maintains two boolean arrays: wizPowers for current state and wizPowersOriginal for last saved state. The saveWizPowers function iterates both arrays comparing values. When a power differs between current and original, it issues INSERT if newly granted or DELETE if revoked. After all changes persist, wizPowersOriginal copies from wizPowers establishing new baseline.

Database writes use non-transactional single-row operations. Multiple simultaneous saveWizPowers calls from different threads could interleave inserts and deletes causing race conditions. The schema permits duplicate rows since player_id, wizpower lacks a UNIQUE constraint allowing the same power to be inserted multiple times.

Load time reconciliation occurs in loadWizPowers reading from database and setting wizPowers array entries to true for each row returned. The lack of PRIMARY KEY means duplicates cause redundant but harmless setWizPower calls. Missing FOREIGN KEY constraint leaves orphaned powers if player deleted from player table.

Power enumeration uses mapWizPowerToFile and mapFileToWizPower translating between internal enum values and database integer constants. This mapping layer provides stability across code refactors that might renumber the wizPowerT enum. The mapping must remain stable across versions to prevent corruption when loading old character data.

### Administrative Setting Restrictions

The doSet command implements a complex privilege check for sensitive administrative settings including office, blocka, blockb, and wizpower subcommands. The target must be connected through a descriptor, must be immortal with level above MAX_MORT, must not have POWER_WIZARD, and the executor must have POWER_SET_IMP_POWER. Self-modification via mob equals this always succeeds regardless of power levels.

This creates a vulnerability where any immortal with POWER_SET can modify their own administrative settings including granting themselves additional powers through the wizpower subcommand. The self-modification exception intended for convenience bypasses the requirement for POWER_SET_IMP_POWER enabling privilege escalation.

Higher-privilege immortals are protected from modification by lower-privilege immortals through the POWER_SET_IMP_POWER check. If the target has POWER_SET_IMP_POWER but the executor does not, the command fails with "You can't do that!" This prevents lateral movement between immortals of similar privilege levels.

The POWER_WIZARD check excludes wizards from administrative modification entirely as a special protection. Wizards can only be modified by other wizards or through self-modification. Combined with level checks this establishes a hierarchy where lower-level immortals cannot modify higher-level immortals.

### Transaction-Wrapped Content Migration

The in-game LOW command implementation uses TDatabase begin and commit methods wrapping all delete and insert operations for a single vnum in a transaction. If any insert fails, rollback prevents partial migration where sneezy has some tables updated but others still contain old data.

CLI tools in lowtools.cc execute the same delete-insert pattern but without transaction wrapping. These tools run offline when the server is down for maintenance. The assumption is that failures will be noticed and manually corrected before server restart.

Multi-table migration for rooms processes room, roomextra, and roomexit in sequence all within the same transaction. The room table insert must succeed before roomextra inserts otherwise referential integrity would break if roomextra referenced a vnum not in room table. Transaction atomicity ensures all-or-nothing semantics.

Vnum list parsing through parse_num_args supports ranges like 13700-13780 expanding to individual vnums. The migration loop processes each vnum independently with its own transaction. If vnum 13705 fails, vnums 13700-13704 remain committed and 13706-13780 remain unmigrated. There is no super-transaction spanning the entire vnum list.

## Reference

### Power Enumeration Constants

The wizPowerT enum defines 128 powers indexed 0-127 in wiz_powers.h. MIN_POWER_INDEX equals 0, MAX_POWER_INDEX equals 128. Each power grants access to specific commands or bypasses specific restrictions.

Core title powers include POWER_BUILDER at index 29 granting builder title and base builder permissions, POWER_GOD at index 30 granting god title and administrative permissions, POWER_WIZARD at index 31 granting wizard title and bypassing IDLED restrictions.

Extreme risk powers include POWER_NO_LIMITS bypassing all vnum and zone restrictions, POWER_SET_IMP_POWER allowing arbitrary parameter modification above 100, POWER_FORCE enabling command execution as other players, POWER_SWITCH enabling character possession.

High risk powers include POWER_LOW for database migration, all IMP_POWER variants bypassing safety restrictions in their base commands, POWER_PURGE_PC allowing deletion of player characters.

Edit command powers include POWER_GOTO for teleportation with POWER_GOTO_IMP_POWER bypassing vnum restrictions, POWER_LOAD for entity creation with POWER_LOAD_IMP_POWER bypassing max_exist limits, POWER_MEDIT/OEDIT/SEDIT for mob/object/shop editing with corresponding IMP_POWER variants bypassing zone assignments.

Special restriction powers include POWER_IDLED restricting immortal to exactly five powers, POWER_LOAD_NOPROTOS allowing load without prototype flag, POWER_LOAD_LIMITED bypassing max_exist enforcement, POWER_RESTORE_MORTAL allowing restoration of mortal characters.

### Power Group Packages

The basic group grants POWER_BUILDER, POWER_GOTO, POWER_BUG, POWER_IDEA, POWER_TYPO, POWER_AT providing entry-level immortal capabilities for navigation and bug reporting.

The rooms group grants POWER_ROOMFLAGS, POWER_REDIT, POWER_REDIT_IMP_POWER, POWER_LOAD, POWER_STAT_ROOMS, POWER_PURGE, POWER_VSTAT enabling room building and manipulation.

The mobs group grants POWER_MEDIT, POWER_MEDIT_IMP_POWER, POWER_LOAD, POWER_STAT_MOBS, POWER_PURGE, POWER_VSTAT, POWER_RANGE, POWER_APPROVE enabling mob creation and testing.

The objs group grants POWER_OEDIT, POWER_OEDIT_IMP_POWER, POWER_LOAD, POWER_STAT_OBJS, POWER_PURGE, POWER_VSTAT enabling object creation and modification.

The quest group grants POWER_QEDIT, POWER_QEDIT_IMP_POWER, POWER_LOAD, POWER_FORCE, POWER_TRANSFER, POWER_RESTORE, POWER_SWITCH, POWER_FOLLOW enabling quest design and testing through player manipulation.

The demi group grants 14 powers including POWER_SHOUT, POWER_INVIS, POWER_ARREST, POWER_ECHO providing demigod-level administrative capabilities without full god access.

The trust group grants 8 high-trust powers including POWER_SNOOP, POWER_MUZZLE, POWER_BAN, POWER_FREEZE enabling player moderation without full administrative access.

The god group grants 23 powers including POWER_GOD, POWER_TIME, POWER_SHUTDOWN, POWER_SYSTEM, POWER_IMMUNITY providing full administrative control excluding only the most dangerous powers.

The allpowers group grants all 127 powers unrestricted providing complete server access with no safety restrictions.

### Set Command Subcommands

Character management subcommands include character for rebuilding at specified level and class with optional learning percentage, class for changing character class by index, level for setting class-specific level from 0 to MAX_MORT, race for changing race by number, exp for setting experience points.

Stat and attribute subcommands include strength, brawn, constitution, dexterity, agility, intelligence, wisdom, focus, perception, charisma, karma, speed accepting values 5-205 with values above 100 requiring POWER_SET_IMP_POWER.

Administrative setting subcommands include wizpower for toggling individual powers by number requiring POWER_SET_IMP_POWER for immortal targets, power for setting power packages by name requiring POWER_SET_IMP_POWER, office for setting builder office room vnum requiring POWER_SET_IMP_POWER, blocka and blockb for setting build zone ranges requiring POWER_SET_IMP_POWER.

Resource management subcommands include hit for current HP, mana for current mana, move for current movement, bank for bank account balance, gold for carried money, practices for practice points all accepting direct numeric assignment.

All subcommands trigger immediate persistence through doSave with SILENT_NO or saveChar with FALSE parameter ensuring changes commit to disk before command completion.

### Database Migration Command Variants

The mvroom command accepts builder name, block number 1 or 2, and vnum list. It migrates room, roomextra, and roomexit tables querying WHERE owner equals builder AND vnum equals target AND block equals specified block. It deletes from sneezy WHERE vnum matches then inserts immortal data.

The mvmob command accepts builder name and vnum list. It migrates mob, mob_extra, and mob_imm tables querying WHERE owner equals builder AND vnum equals target. It clears ACT_STRINGS_CHANGED from actions field before insert.

The mvobj command accepts builder name and vnum list. It migrates obj, objextra, and objaffect tables querying WHERE owner equals builder AND vnum equals target. It strips ITEM_STRUNG and ITEM_PROTOTYPE bits from action_flag before insert.

The mvresponse command accepts builder name and optional vnum list. Without vnums it queries and displays all mobresponses WHERE owner equals builder. With vnums it migrates only specified responses to sneezy database.

Analysis commands include mobs accepting level parameter listing all mobs at that level with stats, race accepting race number listing mobs of that race optionally without stats, path accepting destination room performing pathfinding from current location, tasks list accepting optional name for task tracking, statbonus displaying object bonus statistics, statcharts displaying race stat charts.

### Force Command Target Modes

Single target mode accepts character name resolving through get_char_vis searching current room then world. It requires target level less than executor level and limitPowerCheck validation on mob vnum if target is NPC.

All mode accepts keyword all applying command to all connected player descriptors excluding NPCs. It requires POWER_FORCE and iterates descriptor_list filtering for STATE_PLAYING and checking player levels. Logging occurs through vlogf with LOG_MISC category.

Room mode accepts keyword room applying command to all beings in executor's current room. It uses iterator caching with gCombatNext pattern storing next pointer before parseCommand call handling DELETE_THIS returns.

Mobs mode accepts keyword mobs applying command to all NPCs in executor's current room excluding player characters. It also uses iterator caching preventing use-after-free when forced commands cause mob death.

DELETE flag handling checks if parseCommand returned DELETE_THIS, deletes the victim pointer, sets victim to NULL, and continues iteration. The caller must not dereference victim after DELETE_THIS detection.

### At Command Location Resolution

Room vnum mode accepts numeric vnum converting through convertTo resolving through real_roomp. It validates room exists and is not Room::NOWHERE or Room::STORAGE unless executor has POWER_GOTO_IMP_POWER.

Player name mode accepts character name resolving through get_char_vis. It extracts target room through in_room then performs validation. If target player disconnects between resolution and teleport, in_room becomes stale requiring nullptr check.

Mob name mode accepts mob name resolving through get_char_vis requiring mob not player. It extracts mob room through in_room validating against Room::NOWHERE.

Object name mode accepts object name resolving through get_obj_vis. It checks if object has valid roomp pointer indicating object in room rather than inventory or equipment. The roomp becomes the target location.

All modes teleport executor to target, execute parseCommand on provided command string, then return executor to origin_room cached before teleport. DELETE_THIS detection during command execution prevents return attempt on freed being pointer.

### Load Command Entity Types

Mobile loading accepts vnum or name resolving through real_mobile or get_mob_index searching mob_index array. It creates mob through read_mobile, assigns prototype flag if no POWER_LOAD_NOPROTOS, performs limitPowerCheck validation, then places in room through char_to_room.

Object loading accepts vnum or name resolving through real_object. It checks restricted object types including DEITY_TOKEN, YOUTH_POTION, STATS_POTION, LEARNING_POTION, MYSTERY_POTION, CRAPS_DICE requiring POWER_LOAD_IMP_POWER. It enforces max_exist unless POWER_LOAD_LIMITED granted. It creates through read_object_vnum then adds to inventory.

Set loading accepts set name from equipment_sets configuration. It requires POWER_LOAD_SET. It iterates set entries loading each vnum through recursive load object calls. Equipment sets support themed gear packages for immortal testing.

Count mode accepts count dot type space vnum syntax like "5.object 1234" loading specified quantity. It loops count times performing individual load operations. Each iteration respects max_exist and vnum limits independently.

### Purge Command Modes

Default mode with no arguments purges all objects and mobs in current room. It iterates room stuff list for objects checking not ITEM_NOPURGE before deletion. It iterates room people list for mobs checking not player unless POWER_PURGE_PC granted.

Target mode accepts object or mob name resolving through get_obj_vis or get_char_room_vis. For objects it checks ITEM_NOPURGE flag. For mobs it requires POWER_PURGE_PC if target is player. It deletes the single resolved entity.

Zone mode accepts "zone" keyword and zone number. It requires POWER_PURGE. It iterates mob_index checking if zone matches then counts instances exceeding maximum. It removes excess instances leaving at most the configured maximum.

Room mode accepts "room" keyword, start vnum, and optional end vnum. It requires POWER_PURGE_ROOM. It iterates room range extracting all entities to Room::VOID or Room::STORAGE effectively evacuating the rooms without deletion.

### Restore Command Types

Partial restore accepts "partial" keyword setting hit points to hitLimit, mana to manaLimit, movement to moveLimit, piety to pietyLimit. It sets hunger and thirst to 24 for mortals or -1 for immortals indicating immunity. It requires POWER_RESTORE and POWER_RESTORE_MORTAL if target is mortal.

Full restore accepts "full" keyword performing all partial restore actions then healing all limbs to maximum through setStuckIn NULL clearing stuck_in pointers. It removes affects through affectFrom including diseases and debuffs. It dispels magic and performs spirit chase. It requires same powers as partial.

Practice restore accepts "pracs" keyword reimbursing practice points based on level and class. It calculates expected practices through formula considering level brackets and class multipliers. It requires POWER_RESTORE and POWER_RESTORE_MORTAL.

All restore types call doSave immediately persisting changes to character file and database. Restore operations do not use transactions so partial execution before crash could corrupt character state.

## Implementation

### Power Array Storage and Indexing

The TPerson class allocates two boolean arrays sized MAX_POWER_INDEX. The wizPowers array stores current runtime state while wizPowersOriginal stores the last persisted state from database load or save. Array indices correspond directly to wizPowerT enum values enabling constant-time lookup.

The hasWizPower function bounds-checks the index against MAX_POWER_INDEX returning false for out-of-range values. It then checks wizPowers[POWER_IDLED] implementing special restriction logic. If IDLED is set without WIZARD, it validates the requested power against the five-power whitelist returning false for anything else. Finally it returns wizPowers[value] for the actual power state.

The setWizPower and remWizPower functions perform bitwise operations on array entries. The setWizPower OR equals 0x1 setting the boolean to true. The remWizPower AND equals bitwise NOT of 0x1 clearing to false. This pattern accommodates potential future expansion to multi-bit power states though currently only single bit used.

Array initialization occurs in loadWizPowers querying the wizpower table WHERE player_id matches. For each row returned, it converts the database integer through mapFileToWizPower obtaining the wizPowerT enum value then calls setWizPower. After all rows process, it copies wizPowers to wizPowersOriginal establishing baseline for change detection.

### Power Persistence Delta Encoding

The saveWizPowers function implements write-only-changed optimization iterating both arrays simultaneously. For each index from MIN_POWER_INDEX to MAX_POWER_INDEX it compares wizPowers[num] against wizPowersOriginal[num]. When values differ, the power state changed requiring database update.

If wizPowersOriginal is false and wizPowers is true, the power was granted. It issues INSERT into wizpower with player_id and mapWizPowerToFile enum conversion. The mapping function translates internal enum values to stable database integers preventing corruption when enum order changes across versions.

If wizPowersOriginal is true and wizPowers is false, the power was revoked. It issues DELETE from wizpower WHERE player_id matches AND wizpower matches the mapped value. This removes the specific power row without affecting other granted powers.

After processing all 128 indices, it copies wizPowers into wizPowersOriginal updating the baseline. This ensures subsequent saveWizPowers calls only persist changes since last save. The copy uses simple assignment for each array element since both arrays are same size.

The lack of transaction wrapping creates vulnerability window where partial save leaves inconsistent state. If connection drops between DELETE and INSERT, powers can be permanently lost. The schema lacking PRIMARY KEY permits duplicate inserts if multiple saves race creating redundant rows.

### Vnum Validation Through Block Ranges

The descriptor structure stores four integers defining permitted vnum ranges. The blockastart and blockaend delimit primary build zone while blockbstart and blockbend delimit secondary zone. The office integer stores a single permitted office room vnum.

The limitPowerCheck function receives cmdTypeT indicating command type and int vnum for validation. It first checks POWER_NO_LIMITS providing immediate true return bypassing all restrictions. Otherwise it extracts the four block integers and office from desc pointer.

For CMD_GOTO it validates vnum falls in range blockastart to blockaend OR blockbstart to blockbend OR equals office OR falls in range 0 to 100. The 0-100 exemption permits access to common areas regardless of assignments. Any match returns true otherwise false.

For CMD_LOAD and CMD_SHOW it validates vnum falls in block ranges OR passes isGenericMob OR isGenericObj. Generic vnums represent template entities usable by all builders. The function returns true only if at least one condition satisfies.

Commands calling limitPowerCheck include doGoto checking before teleport, doLoad checking before entity creation, doShow checking before stat display. Failure produces "You don't have that power" message and returns without performing requested action.

### Dual-Database Query Patterns

The TDatabase constructor accepts DB_SNEEZY or DB_IMMORTAL enum connecting to different MariaDB databases. The immortal database tables contain owner varchar column partitioning data by builder name. The sneezy database tables omit owner using simple PRIMARY KEY on vnum.

Migration queries follow delete-before-insert pattern. The code issues DELETE from sneezy.table WHERE vnum equals target removing existing production data. Then it issues INSERT into sneezy.table SELECT columns FROM immortal.table WHERE owner equals builder AND vnum equals target copying builder's version.

The SELECT from immortal includes all columns except owner. The column list must match exactly between immortal and sneezy schemas. Adding columns requires schema migration on both databases otherwise INSERT column count mismatch causes error.

Transaction control in doLow wraps the delete-insert sequence. It calls db.query with "begin" starting transaction then performs all delete and insert operations then calls db.query with "commit". If any query fails, it calls db.query with "rollback" preventing partial application.

Multiple table migration for rooms processes room table first then roomextra then roomexit. All three occur within same transaction since roomexit references room vnum and roomextra references room vnum. If room insert fails, roomextra and roomexit should not persist preventing orphaned data.

### Force Command Iterator Safety

The force all mode iterates descriptor_list which may change during iteration if forced commands cause disconnections. It uses descriptor caching storing descriptor* next before calling parseCommand. After command execution it advances to cached next rather than following current descriptor->next which may be invalid.

The force room mode iterates in_room->people which may change if forced commands cause death or teleportation. It uses character caching storing TBeing* next_char before parseCommand. If parseCommand returns DELETE_THIS it deletes vict and sets vict to NULL. Then it advances to next_char continuing iteration without dereferencing freed pointer.

The force mobs mode adds additional check skipping player characters. It examines vict->isPc returning false for NPCs. This prevents force mobs from affecting logged-in players in the room. The iterator caching pattern matches room mode.

The DELETE_THIS detection uses IS_SET_DELETE macro checking if bit flag set in return code. When true, it calls delete vict freeing the being's memory. It sets vict pointer to NULL preventing use-after-free in subsequent code. The next_char cached pointer remains valid since caching occurred before deletion.

Multiple DELETE flags can combine in single return code. The force command checks DELETE_THIS for victim but does not check DELETE_THIS for self. If a forced command somehow kills both the executor and victim, the force function returns normally without detecting its own deletion leading to use-after-free.

### At Command Teleportation Flow

The doAt function resolves target location through multiple resolution paths. For numeric argument it calls convertTo obtaining vnum then real_roomp obtaining room pointer. For string argument it tries get_char_vis for characters then get_obj_vis for objects extracting room from in_room or roomp.

After obtaining target room pointer, it validates against nullptr and Room::NOWHERE. For Room::STORAGE it checks hasWizPower with POWER_GOTO_IMP_POWER allowing only administrative immortals. For POWER_IDLED without POWER_WIZARD it restricts teleport to Imperia zone checking vnum ranges.

The origin_room caches in_room before teleport. It calls char_from_room removing being from origin then char_to_room adding being to target. This updates in_room pointer and room people lists. If target room is invalid, the being could be orphaned with no room requiring recovery.

The parseCommand executes provided command string in context of target room. The being sees room descriptions, can interact with local entities, and command resolves objects and characters in target location. Return code may contain DELETE_THIS if command kills executor.

The return sequence checks IS_SET_DELETE for DELETE_THIS. If not set, it calls char_from_room on target and char_to_room on cached origin_room restoring original position. If DELETE_THIS is set, it returns immediately without attempted teleport since the being no longer exists.

### Load Command Prototype Flagging

The read_mobile and read_object_vnum functions create new instances from templates in mob_index and obj_index arrays. These functions initialize all fields including obj_flags and setObjStat. The newly created objects do not have ITEM_PROTOTYPE set by default.

The doLoad function checks hasWizPower with POWER_LOAD_NOPROTOS. If the power is absent, it checks isObjStat with ITEM_PROTOTYPE. If the object is not already a prototype, it calls addObjStat with ITEM_PROTOTYPE adding the flag and sends message "Changing the object to a prototype."

Prototype objects have special handling in command processors. Many commands check isObjStat with ITEM_PROTOTYPE and refuse interaction for mortals. This prevents loaded test objects from affecting economy or gameplay. Only immortals can interact with prototypes.

The POWER_LOAD_NOPROTOS power bypasses this flagging allowing immortals to load production-ready objects directly. This supports loading replacement equipment for players or testing item functionality as mortals would experience it. Without the power, all loaded objects become unavailable to mortals.

The max_exist enforcement checks obj_index[numx].getNumber against obj_index[numx].max_exist. If current instances exceed or equal max, load refuses unless POWER_LOAD_LIMITED granted. The getNumber function returns count of existing objects with this vnum across entire world including inventory, equipment, rooms, and containers.

### Set Command Parameter Validation

The doSet function uses tokenizer splitting argument string on whitespace. The first token identifies target character through get_char_vis resolving name to TBeing pointer. The second token identifies subcommand like "hit" or "wizpower" or "level" selecting behavior branch.

For stat subcommands, the third token converts to integer through convertTo. The code checks if parm exceeds 100. If true and hasWizPower with POWER_SET_IMP_POWER returns false, it sends "You do not have the authority to modify above 100" and returns without applying change.

For wizpower subcommand, the third token converts to integer prm then subtracts 1 yielding wizPowerT wpt. This assumes wizard power numbers presented to users as 1-indexed but enum is 0-indexed. The code checks hasWizPower on the target. If target lacks the power, it calls setWizPower. If target has the power, it calls remWizPower toggling state.

For blocka and blockb subcommands, the third and fourth tokens convert to integers for start and end. The code validates start less than or equal to end. It assigns values directly to desc->blockastart and desc->blockaend or desc->blockbstart and desc->blockbend without additional validation.

For character subcommand, it accepts level, class, and optional learning percentage. It validates level does not exceed MAX_MORT and class is valid. It calls rebuildCharacter function reconstructing stats, skills, and abilities at specified parameters. This permanently alters character progression requiring careful use.

### Low Command Transaction Wrapping

The doLow function constructs TDatabase objects for both sneezy and immortal databases. It calls db.query with "begin" on the sneezy database starting transaction. All subsequent DELETE and INSERT operations against sneezy occur within this transaction context.

For each vnum in the parsed list, it queries immortal.room WHERE owner AND vnum AND block obtaining row. If row exists, it calls db.query with DELETE from sneezy.room WHERE vnum removing existing entry. Then it calls db.query with INSERT into sneezy.room extracting values from immortal row.

The same pattern repeats for roomextra table then roomexit table. Each table's delete-insert pair occurs within the ongoing transaction. If any query fails, the query function returns error code and doLow calls db.query with "rollback" aborting all changes.

After processing all tables for all vnums, it calls db.query with "commit" persisting changes. The database guarantees either all vnums migrate successfully or none apply. Partial migration cannot leave some rooms updated while others remain unchanged.

The CLI tools in lowtools.cc execute identical queries but without transaction wrapping. These tools expect to run offline when server stopped. The assumption is manual verification occurs before server restart so failed migrations are noticed and corrected before production use.

### Purge Command Iteration Safety

The purge room default mode iterates roomp->stuff for objects. The stuff list is doubly-linked using next_content and prev_content pointers. Removing an object from the list updates these pointers potentially invalidating iterators.

The code caches next pointer before deletion using TThing* next storing i->next_content. It checks isObjStat with ITEM_NOPURGE. If flag absent, it calls delete i removing object. Then it advances to cached next continuing iteration without dereferencing freed object.

The mob iteration uses similar pattern iterating roomp->people. It caches TBeing* next_char before operations. It checks isPc on the victim. If true and hasWizPower with POWER_PURGE_PC returns false, it skips deletion. Otherwise it calls reformGroup before delete preventing dangling group pointers.

The purge zone mode counts mob instances through mob_index[i].getNumber comparing against max_exist configuration. It finds excess instances through search_char or iteration then calls delete on excess mobs until count reaches maximum.

The purge target mode resolves single object or character through get_obj_vis or get_char_room_vis. It performs same ITEM_NOPURGE check for objects and POWER_PURGE_PC check for player characters. Single target deletion does not require iterator caching since no iteration occurs.

## Troubleshooting

### Privilege Escalation Through Self-Modification

Immortals with only POWER_SET can grant themselves POWER_SET_IMP_POWER through self-modification exception. Use command "at <immortal_office> set self wizpower <POWER_SET_IMP_POWER_number>" where the immortal exploits the mob equals this bypass in administrative setting checks.

Detection requires monitoring wizpower table for INSERT operations on player_id values corresponding to immortals. Correlate with recent set command executions in logs checking for self-targeted set wizpower operations. Look for pattern where immortal gains high-privilege powers without corresponding set operation from higher-privilege immortal.

Prevention requires removing mob not equals this exception from administrative setting restriction checks. Change condition to require POWER_SET_IMP_POWER for all wizpower, office, blocka, blockb operations regardless of target. Alternatively implement separate self-modification permission requiring explicit grant.

Remediation involves revoking improperly granted powers through database DELETE from wizpower WHERE player_id equals target AND wizpower equals improperly granted value. Restart server or force target to reconnect reloading powers from database. Review audit logs identifying what actions the immortal took while holding elevated privileges.

### Database Race Conditions in Power Saves

Multiple simultaneous connections playing same immortal character can cause power save race conditions. Both connections load wizPowersOriginal from database at login. First connection grants power writing INSERT then copying to wizPowersOriginal. Second connection revokes different power writing DELETE then copying to wizPowersOriginal. Final database state may contain both changes but wizPowersOriginal state diverges between connections.

Detection requires monitoring for duplicate session warnings when immortal connects while already connected. Check wizpower table for player_id with unusual power combinations or rapid INSERT/DELETE cycling on same power number indicating potential race.

Prevention requires connection exclusivity preventing same player_id from having multiple active descriptors simultaneously. Implement descriptor takeover forcing disconnect of existing session when new connection authenticates or refuse new connection until existing session disconnects.

Remediation involves forcing both connections to disconnect clearing cached power state. Manually review wizpower table entries for the player_id removing duplicates and correcting power set to intended state. Restart character to reload clean power state from corrected database.

### Migration Transaction Failures

The LOW command transaction rollback on failure prevents partial migration but leaves builder version unchanged in immortal database. Subsequent migration attempts may fail identically if underlying issue persists. Common failures include referential integrity violations, column type mismatches, or max_packet_size exceeded for large descriptions.

Detection shows error messages from db.query calls displayed to immortal executing LOW command. Check MariaDB error log for specific failure cause. Query immortal database examining potentially problematic entries checking description lengths, special characters, or invalid references.

Prevention requires validation queries before migration checking description lengths against column limits, validating referenced vnums exist, checking for null bytes or other problematic characters. Implement dry-run mode performing SELECT from immortal and validation without actual DELETE/INSERT.

Remediation depends on failure type. For too-long descriptions, edit in immortal database truncating or splitting. For invalid references, correct referenced vnums or remove invalid references. For character encoding issues, sanitize text removing problematic bytes. Retry migration after correcting underlying data.

### Force Command Infinite Loops

Forced commands can themselves execute force creating recursion. Command "force <immortal> force <target> <dangerous_command>" creates chain where first force causes second force. If dangerous_command is itself force, unbounded recursion occurs exhausting stack.

Detection shows stack overflow crash or extreme CPU usage as force depth increases. Check logs for force command executions showing nested patterns where forced command output contains additional force executions. Monitor recursion depth if tracking is implemented.

Prevention requires recursion depth tracking in descriptor or being structure. Increment depth counter before parseCommand call in doForce, decrement after return. Check depth against maximum like 5 refusing force execution if exceeded. Return error "Maximum force depth exceeded" to prevent exploitation.

Remediation involves killing frozen immortal process if stuck in force loop. The immortal connection may be unresponsive requiring administrator intervention. After restart, review forced command that triggered loop correcting or restricting. Consider revoking POWER_FORCE from immortal who created loop until demonstrated understanding of proper usage.

### Vnum Limit Bypass Through Object Loading

Immortals can bypass vnum restrictions by loading objects outside assigned zones then using objects to interact with restricted areas. Load object with vnum in assigned zone then read_object_vnum may load container contents from any vnum. Containers loaded with restricted objects bypass limitPowerCheck since only container vnum checked.

Detection requires monitoring load operations for containers checking contained object vnums against immortal's assigned blocks. Check object load logs for discrepancies where object vnum permitted but contained objects outside permitted ranges. Audit immortal inventory for objects with vnums outside assignments.

Prevention requires recursive vnum checking when loading containers. After read_object_vnum, iterate contained objects checking each against limitPowerCheck. If any contained object vnum violates restrictions, refuse load extracting and deleting entire container.

Remediation involves purging improperly loaded objects from immortal inventory through administrative purge command. Review object vnums in immortal's equipment and inventory comparing against assigned blocka, blockb ranges. Remove objects outside permitted ranges. Educate immortal on proper vnum restriction policies.

### At Command Room Pointer Invalidation

The doAt function caches origin_room pointer assuming room remains valid during command execution. However forced commands or triggered events might delete rooms through zone resets or administrative commands. When char_to_room attempts to return to deleted origin_room, crash occurs dereferencing invalid room pointer.

Detection shows crash in char_to_room when accessing room fields. Check room_db array for null entries or Room::NOWHERE at cached origin_room index. Review command executed through doAt checking if it triggers zone resets or room deletion.

Prevention requires validating cached origin_room before return teleport. After parseCommand completes, call real_roomp on origin vnum checking if returned pointer matches cached pointer. If mismatch, room was deleted. Default to teleport to immortal starting location rather than deleted room.

Remediation requires improving room lifecycle management preventing deletion of rooms containing beings. Implement room reference counting incrementing when being enters, decrementing when being leaves. Refuse room deletion if reference count exceeds zero. Alternatively implement tombstone pattern marking rooms deleted but preserving structure until all references cleared.

### Duplicate Power Rows in Database

The lack of PRIMARY KEY on wizpower table allows multiple rows with same player_id and wizpower values. Duplicate insertions occur from saveWizPowers race conditions or manual database manipulation. Loading reads all rows calling setWizPower repeatedly which is harmless but wastes memory and processing.

Detection requires querying wizpower table with GROUP BY player_id, wizpower HAVING COUNT > 1 identifying duplicates. Check for immortals with unusually high row counts in wizpower table indicating extensive duplication.

Prevention requires altering table schema adding UNIQUE constraint on player_id and wizpower. Execute "ALTER TABLE wizpower ADD UNIQUE KEY unique_power player_id, wizpower" enforcing uniqueness. Handle constraint violations during future INSERT attempts with INSERT IGNORE or INSERT ON DUPLICATE KEY UPDATE.

Remediation involves deleting duplicate rows preserving single instance per power. Use "DELETE w1 FROM wizpower w1 INNER JOIN wizpower w2 WHERE w1.player_id = w2.player_id AND w1.wizpower = w2.wizpower AND w1.rowid > w2.rowid" removing duplicates by row identifier. Verify remaining rows contain one entry per player power combination.

### Prototype Flag Preventing Mortal Interaction

Objects loaded without POWER_LOAD_NOPROTOS receive ITEM_PROTOTYPE flag blocking mortal interaction. Immortals testing quests or content may load objects intending mortal use but flag prevents access. Mortals report items visible but unusable.

Detection requires checking object flags with stat command looking for ITEM_PROTOTYPE bit set. Query obj table checking action_flag for bit 4 indicating prototype status. Test object interaction as mortal player attempting get, wear, use operations checking for refusal messages.

Prevention requires documenting POWER_LOAD_NOPROTOS necessity for mortal-usable objects. Educate immortals to check for and request power before loading quest items or event rewards. Implement load command feedback showing "This object will be flagged ITEM_PROTOTYPE and unusable by mortals" when power absent.

Remediation involves manually clearing flag through flag command or database UPDATE. Command "flag <object> proto" toggles ITEM_PROTOTYPE bit removing flag. Alternatively UPDATE obj SET action_flag = action_flag & ~4 WHERE vnum equals target clearing bit 4. Object requires save to persist flag change.

### Invalid Wizpower Parameter Bounds

The doSet wizpower subcommand converts user input to integer then casts to wizPowerT enum without validating range. Negative values or values exceeding MAX_POWER_INDEX cause undefined behavior accessing wizPowers array out of bounds potentially corrupting adjacent memory.

Detection requires monitoring for crashes during set wizpower execution. Check core dumps showing memory corruption in TPerson object near wizPowers array. Review set command logs for wizpower operations with unusual parameter values outside 1-128 range.

Prevention requires bounds checking before enum cast. After convertTo obtains prm integer, check prm >= 1 AND prm <= MAX_POWER_INDEX refusing command if outside valid range. Display "Valid wizard powers range from 1 to 128" informing user of acceptable values.

Remediation involves restarting after crash clearing corrupted memory state. Review wizpower table for player_id involved in crash checking for invalid wizpower values outside enum range. Delete invalid rows preventing load-time issues. Add bounds validation to loadWizPowers rejecting database entries outside valid range.
