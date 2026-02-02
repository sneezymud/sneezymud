---
title: Zone Management System
category: critical
keywords: [zones, reset, bootZones, resetZone, zoneData, resetCom, lifespan, age, isEmpty, load-on-death]
related: [admin-systems.md, scheduler-pulses.md]
primary_symbols:
  functions: [bootZones, bootOneZone, bootZone, renumCmd, resetZone, doGenericReset, isEmpty, procZoneUpdate]
  classes: [zoneData, resetCom]
  files: [code/code/sys/db.cc, code/code/sys/db.h, code/code/sys/socket.cc]
---

# Zone Management System

The zone management system handles the complete lifecycle of game zones from server startup through runtime resets. Misusing this system can corrupt world state and cause server performance issues.

## Overview

Zones are self-contained game areas that reset on a schedule to respawn monsters, objects, and environmental features. The system manages four distinct lifecycle phases: initialization during server boot, aging that tracks time since last reset, execution that processes reset commands, and cleanup that notifies special procedures.

### Core Concepts

Zones are discovered dynamically by scanning the zonefiles directory at boot. Each zone progresses through its lifecycle independently based on configuration. The system automatically sorts zones by vnum, synchronizes with the database using a util_flag pattern, and supports conditional command execution via if_flag logic.

The reset system executes sequentially through a command table that can spawn mobs with equipment, place objects in containers, set door states, configure traps, and manage random placement. Commands can be conditional based on previous success, enabling complex dependency chains. The load-on-death system stores equipment commands for execution when a mob dies rather than cluttering the ground.

### Key Components

The zoneData class represents a zone with properties including zone_nr (sequential runtime index), bottom and top vnums defining room ranges, lifespan in minutes between resets, age tracking time since last reset, reset_mode controlling reset behavior (never, when empty, or always), enabled flag for activation, and cmd_table vector containing reset commands.

The resetCom class represents individual reset commands with command type character, if_flag for conditional execution, four integer arguments for command-specific data, and character field for percent chance commands.

### Lifecycle Phases

During initialization at startup, bootZones scans the zonefiles directory to discover zones, bootOneZone wraps parsing and database synchronization, bootZone parses the zonefile format into memory structures, and renumCmd validates all mob and object vnums against the database.

During aging at runtime, the procZoneUpdate scheduler process runs every 144 seconds to increment zone age counters and queue zones for reset when age reaches or exceeds lifespan. The system marks queued zones with age value ZO_DEAD (9999) to prevent duplicate queuing.

During execution at runtime, resetZone processes the command table sequentially, executing spawns and configurations. Commands check if_flag to determine conditional execution based on previous command success. The load-on-death system stores equipment commands for later execution when mobs die.

During cleanup after reset, doGenericReset notifies all objects in the zone via CMD_GENERIC_RESET to reset their state, clearing temporary flags and restoring default configurations.

### Critical Behaviors

Zones use a zone_nr sequential index separate from file vnums. Inserting new zones changes subsequent zone_nr values but database synchronization handles this gracefully. Room number calculations assume a convention where zone bottom multiplied by 100 gives lowest room vnum and top multiplied by 100 gives highest room vnum, though this pattern is convention not enforcement.

The util_flag database field tracks active zones. Before boot all zones get util_flag set to zero. Successfully loaded zones get util_flag set to one during boot. After boot any zones still at zero are orphaned entries deleted automatically.

Commands that fail vnum validation get zone_value set to zero causing silent failure during reset. Mobs won't spawn but no error logs at reset time, only during initial boot validation.

## Patterns

### Zone Discovery and Initialization

The bootZones function opens the zonefiles directory and reads all entries using opendir and readdir. Non-numeric filenames are filtered except special zone zero. Each filename is converted to an integer and inserted into a multimap with the vnum as key, causing automatic sorting by vnum regardless of alphabetical filename order.

Before processing zones, all database zone records get util_flag reset to zero. This marks existing zones as potentially orphaned. As each zone loads successfully, its util_flag is set to one. After all zones load, any database records still at util_flag zero represent zones that no longer have zonefile and are deleted automatically.

For each zone in sorted order, bootOneZone creates a temporary zoneData instance and calls bootZone to parse the file. If parsing succeeds, renumCmd validates all mob and object vnums. The zone gets assigned the next sequential zone_nr starting from zero. Database update attempts to modify the existing zone record, and if no rows affected then insert creates a new record. Finally the zone is pushed onto the zone_table vector.

### Zonefile Parsing

The bootZone function opens the zonefile using the vnum as filename. It reads the header line with hash and zone number, then the zone name as a tilde-terminated string, then the configuration line with top room, lifespan, reset_mode, and enabled flag.

The function enters a loop reading reset commands until the S terminator. Each command reads a command character, then handles special cases for comments (asterisk lines skipped), gamma-mode lines (dollar lines skipped on production), and terminator (S breaks the loop).

For each command, three initial arguments are read: if_flag, arg1, and arg2. The X command uses first argument as arg3 instead of if_flag. Equipment commands (G, P, E, I) must have if_flag set to one or they're rejected with a log message.

Additional arguments are parsed based on command type. Commands M, O, B, C, K, E, I, P, T, R, D, and L read arg3. The question mark command reads a character field for the command letter. The T trap command without if_flag reads arg4 for damage. The L loot command reads arg4 for flags. After parsing each command is pushed onto the cmd_table vector.

### Vnum Validation

After parsing completes, renumCmd iterates the cmd_table and validates vnums. For mobile commands (M, C, K, R), it calls real_mobile with arg1 to look up the mob index. If the function returns negative one, the mob doesn't exist in the database. The command's zone_value is set to zero causing silent failure during reset, and a LOG_LOW message is written.

For object commands (O, B, G, E, I, P), it calls real_object with arg1 to look up the object index. If the function returns negative one, the object doesn't exist. The command's zone_value is set to zero and a LOG_LOW message is written.

This validation happens only at boot time. During resets, commands with zone_value zero are silently skipped without error messages. To debug missing spawns, check the server boot log for "resolving mobile number" or "resolving object number" messages.

### Age-Based Reset Queueing

The procZoneUpdate scheduler process runs every MUDHOUR (144 seconds). It iterates all zones twice in two phases. The first phase ages zones by incrementing the age counter if less than lifespan and not disabled. When age reaches lifespan and the zone has a non-zero reset_mode, the zone is queued by setting age to ZO_DEAD (9999) and logging the reset intention.

The second phase processes the queue by iterating all zones again. Only zones with age equal to ZO_DEAD are considered. If reset_mode is two (always), resetZone is called immediately. If reset_mode is one (when empty), isEmpty is called to check for players in the zone. Only if no players present does resetZone execute.

The isEmpty function iterates the descriptor_list checking connected state to skip non-playing characters. For each playing character, it checks if in_room is within the zone's room range calculated as bottom times 100 to top times 100. If any player found, the function returns false and reset is deferred. The zone remains queued at age ZO_DEAD for the next pulse to retry.

### Reset Command Execution

The resetZone function initializes tracking pointers for last mob (tmob), last object (tobj), scratch being (tbei), and random room number. It iterates the cmd_table vector sequentially. For each command, if the if_flag is set and last_cmd is false, the command is skipped via continue.

The command's execute method is called with pointers to the zone, tracking variables, and resetFlag parameter. The execute method returns true or false stored in last_cmd to control the next conditional command. Commands update the tracking pointers when they load mobs or objects so subsequent equipment commands know which entity to target.

After all commands execute, the zone age is reset to zero and doGenericReset is called to notify objects. The resetFlag parameter controls special behaviors. The resetFlagBootTime flag causes O commands to execute (boot-only object loads). The resetFlagFindLoadPotential flag causes equipment commands to be stored in the mob's loadCom vector instead of executed immediately, enabling load-on-death loot.

### Conditional Execution Chains

Commands with if_flag zero always execute regardless of previous command results. Commands with if_flag one only execute if the previous command returned true in last_cmd. This enables dependency chains where equipment only loads if the mob spawned, or special configurations only apply if the previous setup succeeded.

The question mark command provides percent chance logic. It has if_flag controlling whether it always evaluates or depends on previous success. It has a percent value in arg1 and a command character in the character field. If the random roll succeeds, it returns true for the next command. If the roll fails, it returns false. This enables probabilistic equipment and configuration.

Armor set commands (Y, Z, J) work with percent chances to load full equipment sets. The Y command references global sets by ID. The Z and J commands reference local sets defined by previous X commands in the same zonefile. The J variant loads items as props (load-on-death) instead of immediately equipping.

### Load-on-Death System

When resetZone is called with resetFlagFindLoadPotential set, equipment commands following a mob load are stored instead of executed. The system checks if tmob exists and if the command type is G (give), E (equip), question mark (percent), Y (global set), Z (local set), or J (local props). If so, the command is pushed onto the mob's loadCom vector and execution skips to the next command.

When the mob dies, the die function checks if loadCom is non-empty. If commands are stored, they're executed with the mob as target but zone pointers and resetFlag adjusted appropriately. This spawns loot directly into the corpse instead of having equipment lying on the ground from the start.

The L (loot) command configures random level-appropriate item generation on death. It stores minimum and maximum level range in arg1 and arg2, and flags in arg4. When the mob dies, the system generates random items from the loot tables matching that level range and flags.

### Random Room Placement

The A command sets a random room range by storing low and high room vnums in arg1 and arg2. The command generates a random number between them (handling either argument order) and stores it in the random_room tracking variable. It always returns true for conditional chains.

Mob and object load commands check if their room vnum argument equals ZONE_ROOM_RANDOM (negative ninety-nine). If so, they use the random_room value instead of the specified vnum. This enables wandering spawns that appear in different locations each reset.

The random_room value persists across commands in the same reset execution, so multiple loads with negative ninety-nine use the same random room chosen by the most recent A command.

### Database Synchronization

The bootOneZone function first attempts to update an existing zone record using the zone_nr as primary key. It sets zone_name, zone_enabled, bottom, top, reset_mode, lifespan, and util_flag to one. The rowCount method checks if any rows were modified.

If rowCount returns zero, no existing record matched that zone_nr. The function executes an insert statement creating a new zone record with the same fields and zone_nr primary key. This handles both initial zone creation and zone_nr reassignment when zones are added or removed.

The util_flag pattern ensures orphaned zones are detected. Zones that existed in the database but have no zonefile are deleted after boot because their util_flag remained at zero from the initial reset.

### Generic Reset Notification

The doGenericReset function iterates room numbers from bottom times 100 to top times 100. For each room number it calls real_roomp to get the room pointer, skipping if null. It iterates the room's stuff list checking each TThing with dynamic_cast to TObj.

For each object found, it calls checkSpec with null being pointer, CMD_GENERIC_RESET command value, empty string, and null pointer. This triggers the object's spec proc if it handles that command. The spec proc can reset counters, clear temporary flags, restore default values, or perform other cleanup.

This notification happens after all spawn commands execute, ensuring objects are in their final configuration before receiving the reset signal. Objects loaded during the reset receive the same notification as objects that were already present.

## Reference

### zoneData Class

The zone_nr field is the sequential runtime index starting from zero assigned during boot regardless of file vnums. The bottom and top fields define the vnum range, typically with rooms calculated as bottom times 100 to top times 100. The lifespan field specifies minutes between reset attempts. The age field tracks minutes since last reset or ZO_DEAD when queued. The reset_mode field controls behavior: zero never resets, one resets when empty, two always resets. The enabled field must be true for the zone to age and queue. The name field stores the zone name string. The cmd_table vector contains all resetCom commands.

The bootZone method parses a zonefile and populates the structure. The renumCmd method validates mob and object vnums. The resetZone method executes the command table. The isEmpty method checks for players in the zone.

### resetCom Class

The command field is a single character identifying the command type (M for mob, O for object, E for equip, etc.). The if_flag field controls conditional execution: false always executes, true only executes if previous command succeeded. The arg1, arg2, arg3, and arg4 fields are command-specific integer arguments. The character field is used by the question mark command to store the command letter.

The execute method processes the command, updating mob, object, and being pointers as needed, and returns true if successful for conditional chains.

### Reset Modes

Mode zero disables resets entirely. The zone never queues and age continues incrementing indefinitely. This is used for static zones and builder testing.

Mode one queues the zone when age reaches lifespan but only resets when isEmpty returns true. If players are present, the zone remains queued and retry occurs on the next procZoneUpdate pulse. This prevents player disruption.

Mode two resets immediately when age reaches lifespan regardless of player presence. This is used for critical zones that must reset on schedule.

### Reset Commands

M loads a mobile at a location with arguments: mob_vnum, room_max (maximum instances in room), room_vnum (location or negative ninety-nine for random). C loads a charmed mobile following the previous mob. K loads a grouped mobile in the same group as previous mob. R loads a mobile as the mount for the previous mob.

O loads an object on the ground only during boot (resetFlagBootTime). B loads an object on the ground every reset. G gives an object to the previous mob's inventory. E equips an object on the previous mob at the specified wear slot. P places an object inside a container object. I equips an object as a prop for load-on-death.

Question mark evaluates a percent chance for the next command with arguments: percent (1-100), unused zero, command letter. A sets random room range with arguments: low_room, high_room. D sets door state with arguments: room_vnum, direction (0-5), state (0 open, 1 closed, 2 locked). V modifies an object value with arguments: value_index, new_value.

T on doors sets a trap with arguments: room_vnum, direction, trap_type, damage. T on objects (with if_flag true) sets a trap on the previous object. H sets hate on the previous mob. F sets fear on the previous mob. L configures random loot on death with arguments: min_level, max_level, unused zero, flags.

X defines a local armor set slot with arguments: slot_number, set_number, obj_vnum. Y loads a global armor set with arguments: global_set_id, percent chance. Z loads a local armor set with arguments: local_set_number, percent chance. J loads a local armor set as props with arguments: local_set_number, percent chance.

S terminates the command table and must be the last command in every zonefile.

### Wear Slots

Slot numbers for equipment: 1 right finger, 2 left finger, 3 neck, 4 body, 5 head, 6 right leg, 7 left leg, 8 right foot, 9 left foot, 10 right hand, 11 left hand, 12 right arm, 13 left arm, 14 back, 15 waist, 16 right wrist, 17 left wrist, 18 hold left, 19 hold right, 20 hold both hands, 21 thrown.

### Direction Values

Direction numbers for doors and movement: 0 north, 1 east, 2 south, 3 west, 4 up, 5 down.

### Constants

ZO_DEAD equals 9999 marking a zone as queued for reset. ZONE_ROOM_RANDOM equals negative ninety-nine triggering random room placement. Pulse::MUDHOUR equals 144 seconds as the zone update interval.

### Database Schema

The zone table has columns: zone_nr integer primary key as sequential runtime index, zone_name varchar as zone name, zone_enabled tinyint as active flag (1 enabled, 0 disabled), bottom integer as lowest room vnum, top integer as highest room vnum, reset_mode tinyint (0 never, 1 empty, 2 always), lifespan integer as minutes between resets, util_flag tinyint as boot tracking (0 orphaned, 1 active).

### Display Commands

The show zones command lists all zones with sequential number, zone_nr in brackets, name, room range in parentheses, current age, reset_mode, and enabled status as Y or N. Source: cmd_show.cc.

The stat zone command shows detailed statistics including zone number, bottom and top vnums, lifespan and age, reset mode description, enabled status, command count, room count, and loaded mob and object counts. Source: cmd_stat.cc.

### Zonefile Format

Zonefiles are text files in lib/zonefiles/ with filename matching the zone starting vnum. The header has hash and zone number, zone name terminated by tilde, and configuration line with top_room, lifespan, reset_mode, and enabled (0 or 1).

Reset commands follow with format: command_char if_flag arg1 arg2 [arg3] [arg4] [character]. Lines starting with asterisk are comments. Lines starting with dollar are gamma-mode and skipped on production port 7900. The S command terminates the file.

## Implementation

### Startup Sequence

During server initialization in socket.cc, bootZones is called from the main boot sequence. The function allocates a DIR pointer and calls opendir on the zonefiles directory string literal. If the directory cannot be opened, perror logs the system error and exit terminates the server.

A multimap with integer key and sstring value is declared using less comparator for automatic ascending sort. The readdir loop processes each directory entry. Entries with name equal to dot or dotdot are skipped via strcmp and continue. The entry name is passed to convertTo template function to parse as integer.

Zero return from convertTo indicates non-numeric filename. The code checks if the result is zero AND the filename doesn't match string literal zero using strcmp. If both conditions true, this is a non-numeric non-zero file and is skipped. Otherwise the vnum and filename are inserted as a pair into the multimap.

The TDatabase instance is constructed with DB_SNEEZY constant selecting the sneezy database. The query method executes raw SQL to update all zone records setting util_flag to zero. This marks all existing zones as potentially stale.

The multimap iterator type is declared explicitly. A for loop iterates from begin to end. For each entry, bootOneZone is called with the database reference, the iterator first (key/vnum), and the zone counter by reference. The zone counter increments inside bootOneZone for each successfully loaded zone.

After all zones load, the database query method executes delete statement removing all zone records where util_flag equals zero. These represent zones that had database entries but no corresponding zonefile.

### Database Update or Insert

The bootOneZone function declares a zoneData temporary on the stack. It calls the bootZone method passing the zone starting vnum. If the method returns false, parsing failed and the function returns without adding to zone_table.

The renumCmd method is called to validate all mob and object vnums in the command table. A LOG_MISC message logs the zone number being booted. The zone_nr field is set to the current value of the zon reference parameter, then zon is post-incremented for the next zone.

The database query method constructs an update statement with format string. The percent-s specifiers use c_str method on the name sstring. The enabled field uses ternary operator to convert bool to integer 1 or 0. The where clause matches zone_nr.

The rowCount method checks how many rows the update affected. If zero rows were updated, no existing record matched that zone_nr. The function constructs an insert statement with the same fields and values. No rowCount check is needed for insert.

Finally the temporary zoneData instance is pushed onto the zone_table global vector using push_back. The move semantics copy the structure including the cmd_table vector.

### File Parsing State Machine

The bootZone method uses FILE pointer from fopen with a constructed path string combining zonefiles directory and the vnum formatted as integer. If fopen returns null, perror logs the error and the method returns false immediately.

The fscanf call reads the hash character and integer into the bottom field, with newline consumed. The EOF check on the return value logs to LOG_FILE if read failed but doesn't return. The fread_string function reads until tilde delimiter into the name sstring.

Four integers are read in one fscanf call: top, lifespan, reset_mode, enabled. The return count is checked to equal four. If not four values read, a LOG_LOW format message logs the bad zone format and the function returns false. The age field is initialized to zero.

The cmd_table clear method empties any previous contents. An infinite for loop begins with two semicolons. A resetCom temporary is declared on the stack. An fscanf skips whitespace. Another fscanf reads a single character into the command field with EOF check.

If command equals capital S, the temporary is pushed onto cmd_table and break exits the loop. If command equals asterisk or dollar and gamePort equals Config::Port::GAMMA, fgets reads the rest of the line into a buffer and continue skips to next iteration.

Three integers are read with fscanf into a temporary tmp, arg1, and arg2. The return count numc is checked. If not equal to three, a LOG_LOW format message logs the missing arguments with detailed fallback values using ternary operators to show what was read.

The command character is compared to capital X. If match, tmp is assigned to arg3. Otherwise tmp is assigned to if_flag. A switch statement checks command against G, P, E, I cases. If match and if_flag is false, LOG_LOW logs the bogus if_flag and continue skips to next command.

Another switch statement checks command against cases requiring arg3: M, O, B, C, K, E, I, P, T without if_flag, R, D, L. If match, fscanf reads arg3 with return count checked and LOG_LOW logged if not one.

If command equals question mark, fscanf reads a single character into the character field with return count checked. If command equals capital T and if_flag is false, fscanf reads arg4 with return count checked. If command equals capital L, fscanf reads arg4 with return count checked.

The temporary resetCom is pushed onto cmd_table. The fgets call reads the rest of the line into buffer to consume trailing content. After the loop exits on S terminator, fclose closes the file pointer and the method returns true.

### Vnum Resolution and Caching

The renumCmd method uses a range-based for loop with auto reference on cmd_table. Each command is modified in place. A switch statement checks the command character against mobile-loading cases: M, C, K, R. If matched, real_mobile is called with arg1 and the result assigned to zone_value field.

If zone_value is less than zero, the vnum lookup failed. A LOG_LOW format message logs the zone name, command character, and arg1 vnum. The zone_value is assigned zero explicitly. During reset execution, zero zone_value causes silent skip.

Another switch case block checks object-loading commands: O, B, G, E, I, P. If matched, real_object is called with arg1 and result assigned to zone_value. The same negative check and zero assignment pattern applies.

The zone_value field is the cached index into mob_index or obj_index arrays. This avoids repeated hash lookups during resets. Commands with zero zone_value fail the index validity check in their execute methods and return false.

### Age Increment and Queue Logic

The procZoneUpdate run method is marked const and receives a TPulse reference parameter. A static integer pulse_zone is declared and initialized to zero on first call. This variable is unused but persists across calls for potential future use.

An integer counter i is declared. A for loop iterates from zero while i less than zone_table size cast to signed int. For each zone, enabled is checked. If false, continue skips to next zone.

If age less than lifespan, age is post-incremented. The conditional then checks if age greater than or equal to lifespan AND age less than ZO_DEAD AND reset_mode non-zero. If all conditions true, the zone is eligible for queueing.

The age is assigned ZO_DEAD (9999). A LOG_MISC format message logs "Resetting" with the zone name, index, and age value. This logs the queueing action, not the actual reset execution.

A second for loop iterates all zones again. If age not equal to ZO_DEAD, continue skips to next zone. This processes only queued zones from the previous loop.

If reset_mode equals two, resetZone is called with resetFlagBootTime and continue skips the empty check. If reset_mode equals one, isEmpty is called. If it returns false, continue defers the reset. Otherwise resetZone is called with resetFlagBootTime.

The function returns false indicating the scheduler proc should continue running on future pulses.

### Player Detection Algorithm

The isEmpty method is marked const and returns bool. A for loop declares a Descriptor pointer i initialized to the global descriptor_list, with condition i non-null, and increment i equals i arrow next.

If i arrow connected is true, the descriptor is not in playing state and continue skips to next. A TBeing pointer ch is assigned from i arrow character. If ch is null, continue skips.

If ch arrow in_room equals Room::NOWHERE, continue skips. The room number is checked greater than or equal to bottom times 100 AND less than or equal to top times 100. If both true, a player is in this zone and the function returns false immediately.

If the loop completes without finding any playing characters in the room range, the function returns true indicating the zone is empty.

### Command Execution Loop

The resetZone method receives a resetFlag parameter. Four local pointers are declared: TMonster pointer tmob, TObj pointer tobj, TBeing pointer tbei, all initialized to NULL. An integer random_room is initialized to zero. A bool last_cmd tracks conditional execution state.

An unsigned integer cmd_no iterates from zero while less than cmd_table size. A resetCom pointer cmd is assigned the address of cmd_table at cmd_no index. If cmd arrow if_flag is true AND last_cmd is false, continue skips this command.

The execute method is called on cmd with this pointer, addresses of tmob, tobj, tbei, random_room, and the flag parameter. The return bool is assigned to last_cmd for the next iteration's conditional check.

After the loop completes, age is assigned zero resetting the timer. The doGenericReset method is called to notify objects. The method returns void.

### Object Notification Iteration

The doGenericReset method declares an integer counter initialized to bottom times 100. A for loop continues while counter less than or equal to top times 100, with counter post-incremented.

A TRoom pointer rp is assigned from real_roomp called with counter. If rp is null, continue skips this room number. A StuffIter it is declared from rp arrow stuff dot begin. A for loop continues while it not equal to rp arrow stuff dot end, with it pre-incremented.

A TObj pointer obj is assigned from dynamic_cast with TObj pointer type and the dereferenced it iterator. If obj is null, the item is not an object and continue skips.

The checkSpec method is called on obj with NULL being pointer, CMD_GENERIC_RESET constant, empty string literal, and NULL pointer. The method return value is discarded. After both loops complete, the method returns void.

### Load-on-Death Storage

Inside the command execution loop in resetZone, before calling execute, a check examines the flag parameter using bitwise AND with resetFlagFindLoadPotential. If the result is non-zero AND tmob is non-null, load-on-death mode is active.

A switch statement checks cmd arrow command against cases: G, E, question mark, Y, Z, J. If matched, the dereferenced cmd is pushed onto tmob arrow loadCom using push_back. The continue statement skips the execute call, preventing immediate execution.

When the mob eventually dies, the die function iterates loadCom and executes each stored command with appropriate context. This allows equipment to spawn directly into the corpse instead of being visible on the ground from initial load.

## Troubleshooting

### Zone Won't Queue for Reset

If a zone's age counter continues incrementing past its lifespan but the zone never resets, check that enabled is true in the zonefile header. Disabled zones age but never queue. Verify reset_mode is non-zero; mode zero disables resets entirely. Check server logs for "Resetting" messages; absence indicates queueing failure.

If the zone queues (age becomes 9999) but never resets, the issue is likely reset_mode one with players present. Use stat zone to check reset mode. Use show zones to verify the zone is queued. Check if players are in the room range by examining character locations. Mode one zones remain queued until isEmpty returns true.

### Mobs Fail to Spawn

When zone resets complete but expected mobs don't appear, first check server boot logs for "resolving mobile number" messages. These indicate the mob vnum failed validation in renumCmd. The command's zone_value was set to zero causing silent skip during resets.

Verify the mob vnum exists in the database using show mob command. If the mob exists now but didn't during boot, the zone must be rebooted to revalidate vnums. Use boot zone command or restart the server.

Check the max_exist limit for the mob using stat mob. If current exist equals maximum, additional spawns are silently skipped. Consider raising max_exist in the mob database record.

Examine the zonefile for conditional execution chains. If the mob command has if_flag one but the previous command failed, the mob won't spawn. Trace backwards through the command sequence to find the failing dependency.

Verify the room vnum is valid. If the room doesn't exist, spawn silently fails. Use goto to test room existence. Check for typos in room numbers.

### Objects Don't Load

Object loading failures have the same vnum validation issues as mobs. Check boot logs for "resolving object number" messages. Verify object existence with show obj. Check max_exist limits with stat obj.

For P commands that place objects in containers, verify the container exists and isn't at maximum capacity. Check the container's max_contain value. Examine whether the container itself loaded successfully.

For E commands that equip objects on mobs, verify the mob loaded successfully and the wear slot is valid. Check that the object can be worn in the specified slot based on its wear flags.

For O commands that only load at boot, verify resetFlagBootTime is set in the resetZone call. Runtime resets typically don't execute O commands, only B commands. If objects appear after server restart but not after zone reset, change O to B.

### Random Room Placement Issues

When using A command to set random ranges, verify the command executes before the mob or object load that references negative ninety-nine room vnum. If the A command is conditional (if_flag one) and fails, random_room remains zero and loads fail.

Check that the random range contains valid room vnums. If the range includes non-existent rooms, spawns may land in invalid locations causing silent failure.

Multiple A commands in sequence overwrite random_room. Ensure each random-placement load has its own A command immediately before, or intentionally share the same random room across multiple loads.

### Database Synchronization Problems

If zones appear corrupted after adding or removing zones, the zone_nr sequence may have shifted. The database update uses zone_nr as primary key, so insertion of a zone causes all subsequent zones to have mismatched zone_nr values until next boot.

Check for orphaned zone records using SQL query: SELECT * FROM zone WHERE util_flag equals zero. These should only exist between boot start (when all set to zero) and boot completion (when orphans deleted). Persistent zero values indicate incomplete boot.

If the util_flag pattern fails, manual cleanup may be needed. Identify which zones are active in the filesystem. Delete zone records that don't have matching zonefiles. Update util_flag to one for active zones.

### Performance Degradation on Reset

Large zones with many complex commands can cause lag when resetZone executes. Profile the command count using stat zone. Consider splitting very large zones into multiple smaller zones with staggered lifespans.

Check doGenericReset performance by examining object counts in zone rooms. Zones with thousands of objects in rooms will iterate slowly. Clean up unnecessary objects or move them into containers to reduce per-room object counts.

If many zones reset simultaneously, adjust lifespans to distribute resets across time. Avoid multiples of common values like 45 minutes that cause synchronized resets.

### Silent Failures

The most difficult issues involve commands that fail without logging errors during runtime. These include zone_value zero from vnum validation, max_exist limits reached, conditional execution chains broken, and invalid room vnums.

Enable detailed logging temporarily by checking LOG_LOW and LOG_MISC log categories. Examine boot logs for any validation warnings. Use stat commands to inspect mob and object current versus maximum exist values.

Test zones in isolation by disabling other zones temporarily and monitoring specific spawn points. Use goto to visit expected spawn locations immediately after reset. Use show zones to confirm reset execution timing.

Check zonefile syntax carefully for missing arguments, mismatched if_flag values, and incorrect command ordering. Common errors include equipment commands without if_flag one, missing arg3 on commands that require it, and S terminator missing or out of place.
