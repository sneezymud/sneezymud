---
title: Administrative Systems
description: Capability-based access control with wizard powers, zone restrictions, and database migration between immortal and production environments.
category: critical
keywords: [capability-based access, access control, zone restrictions, database migration, remote execution, privilege escalation]
primary_symbols:
  functions: [hasWizPower, limitPowerCheck, doSet, doForce, doAt, doLoad, saveWizPowers]
  classes: [TPerson, TBeing]
  enums: [wizPowerT, POWER_SET, POWER_SET_IMP_POWER, POWER_NO_LIMITS, POWER_FORCE, POWER_SWITCH, POWER_IDLED, POWER_WIZARD, POWER_BUILDER, POWER_GOD, POWER_GOTO, POWER_GOTO_IMP_POWER, POWER_LOAD, POWER_LOAD_IMP_POWER, POWER_LOAD_SET, POWER_LOAD_NOPROTOS, POWER_LOAD_LIMITED, POWER_MEDIT, POWER_MEDIT_IMP_POWER, POWER_OEDIT, POWER_OEDIT_IMP_POWER, POWER_REDIT, POWER_REDIT_IMP_POWER, POWER_SEDIT, POWER_SEDIT_IMP_POWER, POWER_FLAG, POWER_FLAG_IMP_POWER, POWER_LOW, POWER_PURGE_PC, POWER_SNOOP, POWER_DISTRIBUTE, DB_IMMORTAL, DB_SNEEZY, DELETE_THIS, DELETE_ITEM, MAX_MORT, ITEM_PROTOTYPE, ITEM_NOPURGE, ACT_STRINGS_CHANGED]
---

## Overview

How do you prevent a builder from accidentally (or maliciously) modifying content outside their assigned zone? How do you ensure that only trusted administrators can grant powers to other immortals? How do you migrate content from a development database to production without data loss?

SneezyMUD's administrative systems answer these questions through a capability-based access control model. Immortals (staff members with level > 50) are granted individual powers that unlock specific commands or bypass restrictions. This creates fine-grained control over who can do what, rather than relying solely on level-based access.

The system operates on two tiers. Standard immortals with `POWER_SET` can modify mortals, NPCs, and lower-level immortals. Administrative immortals who also have `POWER_SET_IMP_POWER` can modify anyone, including other immortals. This separation prevents privilege escalation attacks where a lower-tier immortal modifies a higher-tier one.

Zone restrictions add another layer. Each builder is assigned one or two vnum ranges (block A and block B) plus an office room. The `limitPowerCheck()` function enforces these boundaries for commands like goto, load, and show. Immortals with `POWER_NO_LIMITS` bypass all zone restrictions entirely.

Content moves from development to production through the LOW command. The immortal database serves as a builder workspace where multiple builders can work on the same vnums independently. When content is ready, migration commands transfer it to the sneezy database that the live server uses.

---

## Patterns

### Power Validation

Always validate power indices before casting to `wizPowerT`. User-provided integers can be negative, zero, or exceed the valid range. The enum has no built-in bounds checking, so invalid values corrupt memory or cause undefined behavior.

Never grant `POWER_SET_IMP_POWER` without `POWER_SET`. The IMP_POWER variant is meaningless alone since it only removes restrictions from the base power.

Never allow immortals to modify their own administrative settings (office, blocka, blockb, wizpower) unless they have `POWER_SET_IMP_POWER`. The current code has a self-modification exception that bypasses this check, creating a privilege escalation vulnerability.

### Zone Enforcement

Always call `limitPowerCheck()` before allowing immortals to access vnums outside their blocks. This function returns true only if the vnum falls within the immortal's assigned ranges or special cases (generic mobs/objects, Imperia zone 0-100).

Never assume `POWER_NO_LIMITS` is the only bypass. Some commands have their own IMP_POWER variants (like `POWER_GOTO_IMP_POWER`) that bypass zone restrictions for that specific command.

### IDLED Immortals

Check `POWER_WIZARD` status when evaluating IDLED immortals. An immortal with `POWER_IDLED` but without `POWER_WIZARD` is severely restricted to only five powers: BUILDER, GOD, WIZARD, GOTO, and IDLED itself. All other power checks return false regardless of what powers they actually have.

### Database Migration

Always use transactions for migration operations. The in-game LOW commands wrap operations in begin/commit blocks, but CLI tools do not. A crash between delete and insert loses data permanently.

Never migrate content while the zone is actively loaded. The server caches zone data in memory, so database changes may not take effect until the zone resets or the server restarts.

Validate source data exists before deleting destination data. The migration tools correctly check for source records before proceeding, but custom scripts must follow this pattern.

### DELETE Flag Handling in Remote Execution

Always check `DELETE_THIS` returns from `doForce()` and `doAt()`. The forced or remote command may kill or destroy the target. After detecting DELETE_THIS, delete the target and stop execution immediately.

Never continue execution after a DELETE_THIS from `parseCommand()` in force loops. When forcing multiple targets (force all, force room, force mobs), each iteration must check for death before accessing the next target.

### Power Grant Workflow

Never grant these powers without extreme vetting:
- `POWER_NO_LIMITS` - Bypasses all vnum restrictions
- `POWER_SET_IMP_POWER` - Arbitrary parameter modification
- `POWER_FORCE` - Execute commands as other players
- `POWER_SWITCH` - Possess player characters

Use power packages (basic, rooms, mobs, objs) for standard builders rather than granting individual powers. This ensures consistent permission sets and makes auditing easier.

Document all power grants with who granted, to whom, timestamp, and reason. The system does not log this automatically.

---

## Reference

### Symbol Quick Reference

| Symbol | Type | Purpose |
|--------|------|---------|
| `hasWizPower()` | function | Check if immortal has specific power |
| `setWizPower()` | function | Grant a power to an immortal |
| `remWizPower()` | function | Revoke a power from an immortal |
| `saveWizPowers()` | function | Persist power changes to database |
| `limitPowerCheck()` | function | Validate vnum against assigned blocks |
| `doSet()` | function | Entry point for @set command |
| `doForce()` | function | Force another being to execute command |
| `doAt()` | function | Execute command at remote location |
| `doLoad()` | function | Create mob/object instances |
| `TPerson` | class | Player character with wizard power arrays |
| `TBeing` | class | Base being class with limit checks |
| `cmd_set.cc` | file | @set command implementation |
| `cmd_low.cc` | file | Database migration commands |
| `immortal.cc` | file | Force, at, load, purge, restore |
| `wiz_powers.cc` | file | Power management and packages |

### Critical Powers

| Power | Risk Level | Description |
|-------|------------|-------------|
| `POWER_NO_LIMITS` | Extreme | Bypasses ALL vnum/zone restrictions |
| `POWER_SET_IMP_POWER` | Extreme | Arbitrary parameter modification (>100 limits) |
| `POWER_FORCE` | Extreme | Execute commands as other players |
| `POWER_SWITCH` | Extreme | Possess player characters |
| `POWER_LOW` | High | Database migration between immortal/sneezy |
| `POWER_PURGE_PC` | High | Delete player characters |
| `POWER_IDLED` | Special | Restricts immortal to 5 powers only |

### IMP_POWER Variants

| Base Power | IMP Variant | Bypasses |
|------------|-------------|----------|
| `POWER_FLAG` | `POWER_FLAG_IMP_POWER` | Cannot be snooped |
| `POWER_GOTO` | `POWER_GOTO_IMP_POWER` | Zone vnum restrictions |
| `POWER_LOAD` | `POWER_LOAD_IMP_POWER` | Vnum limits, max_exist |
| `POWER_MEDIT` | `POWER_MEDIT_IMP_POWER` | Zone assignments |
| `POWER_OEDIT` | `POWER_OEDIT_IMP_POWER` | Zone assignments |
| `POWER_SEDIT` | `POWER_SEDIT_IMP_POWER` | Zone assignments |
| `POWER_SET` | `POWER_SET_IMP_POWER` | Parameter limits (>100) |

### Power Packages

| Package | Command | Powers Granted |
|---------|---------|----------------|
| basic | `@set ... wizpower basic` | BUILDER, WIZNET, POWERS, GOTO, IMMORTAL_HELP, SETSEV |
| rooms | `@set ... wizpower rooms` | REDIT, RSAVE, EDIT, RLOAD, STAT, SHOW, PURGE |
| mobs | `@set ... wizpower mobs` | MEDIT, STAT_MOBILES, SHOW_MOB, SEDIT, IMMORTAL_OUTFIT, WIZNET_ALWAYS, LOAD |
| objs | `@set ... wizpower objs` | LOAD_SET, STAT_OBJECT, SHOW_OBJ, OEDIT, OEDIT_APPLYS, OEDIT_WEAPONS, OEDIT_COST |
| quest | `@set ... wizpower quest` | SWITCH, NOSHOUT, STEALTH, QUEST, AT, WHERE, SYSTEM, LOAD_NOPROTOS |
| demi | `@set ... wizpower demi` | 20 powers: COLOR_LOGS, LONGDESC, COMMENT, FINDEMAIL, CLIENTS, TRACEROUTE, HOSTLOG, DEATHCHECK, SNOWBALL, PEE, WIZLOCK, CUTLINK, SEE_COMMENTARY, ECHO, TRANSFER, TOGGLE, VISIBLE, HEAVEN, ZONEFILE_UTILITY, INFO |
| trust | `@set ... wizpower trust` | INFO_TRUSTED, GAMESTATS, FLAG, SHOW_TRUSTED, RESTORE, ACCESS, USERS, ACCOUNT |
| god | `@set ... wizpower god` | 19 powers: LOW, GOD, COMPARE, REDIT_ENABLED, STAT_SKILL, RESTORE_MORTAL, IMM_EVAL, FORCE, DISTRIBUTE, LOG, PURGE_PC, PURGE_ROOM, EGOTRIP, CHECKLOG, LOGLIST, REPLACE, RESIZE, NO_LIMITS, MAP_RECALC |
| allpowers | `@set ... wizpower allpowers` | All powers unrestricted |

### Privilege Tiers

| Tier | Powers | Can Modify |
|------|--------|------------|
| Standard | `POWER_SET` | Mortals, NPCs, low-level immortals |
| Administrative | `POWER_SET` + `POWER_SET_IMP_POWER` | All characters including immortals |

### @set Subcommands

| Subcommand | Parameters | Restrictions |
|------------|------------|--------------|
| `character` | `<level> <class> [learning%]` | Cannot reduce level; max `MAX_MORT` |
| `class` | `<class_index>` | Must be valid class index |
| `level` | `<level> <class>` | 0 to `MAX_MORT` |
| `race` | `<race_number>` | Must be valid race |
| `exp` | `<experience>` | Direct XP assignment |
| `wizpower` | `<power_number>` | `POWER_SET_IMP_POWER` for immortals |
| `office` | `<room_vnum>` | `POWER_SET_IMP_POWER` |
| `blocka` | `<start> <end>` | `POWER_SET_IMP_POWER` |
| `blockb` | `<start> <end>` | `POWER_SET_IMP_POWER` |
| `hit` | `<current_hp>` | None |
| `mana` | `<current_mana>` | None |
| `move` | `<current_move>` | None |
| `bank` | `<talens>` | None |
| `gold` | `<talens>` | None |
| `practices` | `<amount>` | None |

### Stats Range

| Stat | Min | Max (Standard) | Max (IMP_POWER) |
|------|-----|----------------|-----------------|
| All primary stats | 5 | 100 | 205 |

### Force Command Target Modes

| Mode | Syntax | Behavior |
|------|--------|----------|
| Single | `force <name> <command>` | Target resolved via `get_char_vis`; requires higher level than target |
| All | `force all <command>` | All connected players; iterates `descriptor_list` |
| Room | `force room <command>` | All beings in current room; uses iterator caching |
| Mobs | `force mobs <command>` | All NPCs in current room; excludes players |

### At Command Location Resolution

| Mode | Syntax | Resolution |
|------|--------|------------|
| Room vnum | `at <vnum> <command>` | `convertTo` + `real_roomp` |
| Player | `at <player> <command>` | `get_char_vis` → `in_room` |
| Mob | `at <mob> <command>` | `get_char_vis` (non-player) → `in_room` |
| Object | `at <object> <command>` | `get_obj_vis` → `roomp` |

### Load Command Entity Types

| Type | Syntax | Notes |
|------|--------|-------|
| Mobile | `load mob <vnum/name>` | Adds PROTOTYPE flag unless `POWER_LOAD_NOPROTOS` |
| Object | `load obj <vnum/name>` | Checks restricted items, max_exist |
| Set | `load set <setname>` | Requires `POWER_LOAD_SET` |
| Count | `load 5.obj <vnum>` | Loads specified quantity |

### Purge Command Modes

| Mode | Syntax | Behavior |
|------|--------|----------|
| Default | `purge` | All objects/mobs in room (respects ITEM_NOPURGE) |
| Target | `purge <name>` | Single resolved entity |
| Zone | `purge zone <zone#>` | Excess mobs above max_exist |
| Room range | `purge room <start> [end]` | Evacuate entities from vnum range |

### LOW Migration Commands

| Command | Tables Affected | Special Processing |
|---------|-----------------|-------------------|
| `low mvroom` | room, roomextra, roomexit | Requires block parameter |
| `low mvobj` | obj, objextra, objaffect | Strips ITEM_STRUNG, ITEM_PROTOTYPE |
| `low mvmob` | mob, mob_extra, mob_imm | Clears ACT_STRINGS_CHANGED |
| `low mvresponse` | mobresponses | Lists responses if no vnums given |

### Databases

| Database | Enum | Purpose |
|----------|------|---------|
| immortal | `DB_IMMORTAL` | Builder workspace with owner tracking |
| sneezy | `DB_SNEEZY` | Production data used by live server |

### Restricted Load Objects

| Object Constant | Description |
|-----------------|-------------|
| `Obj::DEITY_TOKEN` | Religious faction item |
| `Obj::YOUTH_POTION` | Age reduction |
| `Obj::STATS_POTION` | Stat modification |
| `Obj::LEARNING_POTION` | Learning boost |
| `Obj::MYSTERY_POTION` | Random effects |
| `Obj::CRAPS_DICE` | Gambling item |

### Restore Types

| Type | Restores |
|------|----------|
| partial | HP, mana, move, piety, hunger/thirst |
| full | All partial + limbs, remove affects/diseases, dispel magic |
| pracs | Practice points reimbursement |

---

## Implementation

### Wizard Powers Storage

Each `TPerson` maintains two boolean arrays for power tracking. The `wizPowers` array holds current power state while `wizPowersOriginal` tracks the last saved state. This differential allows `saveWizPowers()` to persist only changed values rather than rewriting the entire power set.

The database schema stores powers in the `wizpower` table with player_id and wizpower columns. Notable schema weaknesses include: no primary key (allows duplicate entries), no unique constraint (same power can be granted multiple times), no foreign key (orphaned powers if player deleted), and no cascade delete (requires manual cleanup).

Power values undergo transformation between memory and database via `mapWizPowerToFile()` and `mapFilePowerToWiz()`. This indirection exists because the enum order has changed over time while database values must remain stable for existing characters.

The `setWizPower()` and `remWizPower()` functions perform bitwise operations: `setWizPower` ORs with 0x1 to set true, `remWizPower` ANDs with ~0x1 to clear to false. This pattern accommodates potential future expansion to multi-bit power states.

### Power Check Flow

When `hasWizPower()` is called, it first validates the power index is within bounds. For IDLED immortals (those with `POWER_IDLED` but lacking `POWER_WIZARD`), the function returns false for all powers except BUILDER, GOD, WIZARD, GOTO, and IDLED itself. This implements a restricted mode for inactive staff who retain their immortal status but lose most capabilities.

The `limitPowerCheck()` function enforces zone boundaries. It extracts the builder's assigned ranges from their descriptor (blockastart/blockaend, blockbstart/blockbend, office) and checks if the requested vnum falls within those ranges. Special cases include: always allowing vnums 0-100 (Imperia), always allowing generic mobs/objects, and completely bypassing checks for immortals with `POWER_NO_LIMITS`.

### @set Command Structure

The `doSet()` function in TPerson parses the target and subcommand, then dispatches to the appropriate handler. The privilege escalation gate early in the function prevents standard immortals from modifying those with `POWER_SET_IMP_POWER`.

Administrative settings (office, blocka, blockb, wizpower) have additional checks: target must be connected (have a descriptor), target must be immortal, target must not have `POWER_WIZARD`, and executor must have `POWER_SET_IMP_POWER`. A problematic exception allows self-modification (`mob != this`) to bypass these checks.

Stat modifications compare the requested value against 100. Values above 100 require `POWER_SET_IMP_POWER`. The valid range is 5-205.

For the wizpower subcommand, the parameter converts to integer then subtracts 1 (user-facing is 1-indexed, enum is 0-indexed). The code toggles the power: if target lacks it, `setWizPower()` is called; if target has it, `remWizPower()` is called.

All @set operations trigger immediate persistence via `doSave()` or `saveChar()`. The dual storage model means some properties (money, wizpowers) use database tables as authoritative while others (stats) rely on binary character files.

### Database Migration Architecture

The dual-database design separates builder workspace (immortal) from production (sneezy). The immortal database includes additional columns: `owner` (builder name) and `block` (version number for rooms). This allows multiple builders to work on the same vnums simultaneously without conflict.

Primary key differences enable this isolation. Production uses `PRIMARY KEY (vnum)` while development uses `PRIMARY KEY (owner, vnum)`. A builder's version of room 45660 coexists with other builders' versions until migration.

Migration commands follow a delete-before-insert pattern. The command first queries the source (immortal) database for the specified owner/vnum combination, then deletes any existing entry in the destination (sneezy) database, and finally inserts the source data. In-game commands wrap this in transactions; CLI tools do not.

Object migration strips prototype flags (ITEM_STRUNG bit 2, ITEM_PROTOTYPE bit 4) from action_flag. Mob migration clears ACT_STRINGS_CHANGED. These transformations prepare builder content for production use.

Multi-table migration processes tables in sequence within the same transaction: room first, then roomextra, then roomexit. This ensures referential integrity since roomexit references room vnums. Transaction atomicity guarantees all-or-nothing semantics.

Vnum list parsing via `parse_num_args` supports ranges like `13700-13780`. Each vnum processes in its own transaction, so partial failure leaves some vnums committed and others unmigrated.

### Remote Execution Mechanics

The force command resolves its target, validates permissions (must be higher level than target, target vnum must be in assigned block for mobs), and calls `parseCommand()` on the victim with the specified command string. The critical detail is DELETE_THIS handling: if the forced command kills the target, the target is deleted immediately and the loop continues to the next victim (for force all/room/mobs variants).

For iterator safety, force room/mobs modes cache the next pointer before calling `parseCommand()`. If DELETE_THIS is detected, the victim is deleted, set to NULL, and iteration advances to the cached next pointer.

The at command teleports the executor to the target location, runs the command, then returns to the original room. It must handle DELETE_THIS from the executed command since the immortal themselves might die from what they executed remotely. If DELETE_THIS is set, the function returns immediately without attempting the return teleport.

The load command creates instances from the template database. Permission layers include: base loading requires `POWER_LOAD`, equipment sets require `POWER_LOAD_SET`, non-prototype loading requires `POWER_LOAD_NOPROTOS`, loading beyond max_exist requires `POWER_LOAD_LIMITED`, and certain restricted objects require `POWER_LOAD_IMP_POWER`.

### Power Package Groups

Power packages exist in `wiz_powers.cc` as named collections. When a package is requested, the code grants or revokes the associated powers in sequence. Package membership is hardcoded rather than data-driven.

A bug in the remgod package handler calls `setWizPower(POWER_DISTRIBUTE)` instead of `remWizPower(POWER_DISTRIBUTE)`, accidentally granting a power when trying to remove the god package.

### Purge and Restore

The purge command operates in several modes: single target (specified object or mob), room-wide (all entities in current room), zone-wide (excess mobs in specified zone), and room range (evacuate entities from vnum range). Objects with ITEM_NOPURGE flag are immune. Iteration uses cached next pointers before deletion to prevent use-after-free.

Restore comes in three variants. Partial restore handles HP, mana, move, piety, and hunger/thirst. Full restore adds limb healing, affect removal (including diseases), dispel magic, and spirit chase. Practice restoration reimburses practice points.

---

## Troubleshooting

### Power Not Working for IDLED Immortal

**Symptom:** Immortal has a power according to database but commands fail with permission errors.

**Likely cause:** Immortal has `POWER_IDLED` set but lacks `POWER_WIZARD`. The IDLED restriction allows only 5 specific powers.

**Diagnostic approach:** Check both `POWER_IDLED` and `POWER_WIZARD` status. Query `SELECT wizpower FROM wizpower WHERE player_id=X` and compare against the 5 allowed powers (BUILDER, GOD, WIZARD, GOTO, IDLED).

**Fix:** Either grant `POWER_WIZARD` to exempt from IDLED restrictions, or remove `POWER_IDLED` to restore full power access.

### Database Powers Not Loading

**Symptom:** Powers granted in database don't appear after login.

**Likely cause:** Duplicate entries in wizpower table or power mapping mismatch between memory enum and database values.

**Diagnostic approach:** Check for duplicates with `SELECT player_id, wizpower, COUNT(*) FROM wizpower GROUP BY player_id, wizpower HAVING COUNT(*) > 1`. Verify mapping consistency in `mapFilePowerToWiz()`.

**Fix:** Delete duplicate entries. If mapping is wrong, the power enum order has changed and requires migration of existing database values.

### Migration Loses Data

**Symptom:** Content disappears from production after LOW command.

**Likely cause:** Source data didn't exist in immortal database, or CLI tool crashed between delete and insert.

**Diagnostic approach:** Check if source record exists: `SELECT * FROM immortal.room WHERE owner='builder' AND vnum=X AND block=Y`. Check server logs for errors during migration.

**Fix:** If using CLI tools, restore from backup. Always use in-game commands for transactional safety. Verify source exists before migrating.

### Self-Escalation Vulnerability

**Symptom:** Lower-tier immortal has administrative powers they shouldn't have.

**Likely cause:** Exploited self-modification exception in @set command.

**Diagnostic approach:** Check power grant logs (if implemented). Query `SELECT * FROM wizpower WHERE player_id=X` and compare against expected power set for their role.

**Fix:** Revoke inappropriate powers. The underlying code vulnerability (self-modification exception) requires a code change to fully address.

### Block Range Not Enforcing

**Symptom:** Immortal can access vnums outside their assigned blocks.

**Likely cause:** Immortal has `POWER_NO_LIMITS` or the specific IMP_POWER variant for that command.

**Diagnostic approach:** Check for bypass powers in addition to block assignments. Query their full power set and check for NO_LIMITS or command-specific IMP_POWER variants.

**Fix:** Revoke bypass powers if they weren't intentionally granted. Block ranges only enforce when bypass powers are absent.

### Force Command Infinite Loops

**Symptom:** Stack overflow crash or extreme CPU usage during force execution.

**Likely cause:** Forced commands contain nested force commands creating unbounded recursion.

**Diagnostic approach:** Check logs for nested force patterns. Look for commands like `force <immortal> force <target> <command>`.

**Fix:** Implement recursion depth tracking in the force command. Consider revoking `POWER_FORCE` from immortals who create loops until they demonstrate understanding of proper usage.

### Vnum Limit Bypass Through Container Loading

**Symptom:** Immortal has objects with vnums outside their assigned blocks.

**Likely cause:** Loaded containers with contents from restricted vnums. Only the container vnum is checked by `limitPowerCheck()`, not contained objects.

**Diagnostic approach:** Audit immortal inventory for objects with vnums outside their blocka/blockb ranges.

**Fix:** Purge improperly loaded objects. Consider implementing recursive vnum checking when loading containers.

### At Command Room Pointer Invalidation

**Symptom:** Crash in `char_to_room` when returning from at command.

**Likely cause:** The executed command triggered a zone reset or room deletion, invalidating the cached origin_room pointer.

**Diagnostic approach:** Check if the at command triggered zone resets or room deletion. Look for null entries in room_db at the origin vnum.

**Fix:** Validate cached origin_room before return teleport. Default to immortal starting location if origin room was deleted.

### Prototype Flag Preventing Mortal Interaction

**Symptom:** Mortals report loaded items are visible but unusable.

**Likely cause:** Objects loaded without `POWER_LOAD_NOPROTOS` receive ITEM_PROTOTYPE flag automatically.

**Diagnostic approach:** Check object flags with stat command for ITEM_PROTOTYPE bit.

**Fix:** Clear the flag with `flag <object> proto` to toggle it off. For future loads of mortal-usable items, ensure the loading immortal has `POWER_LOAD_NOPROTOS`.

### Invalid Wizpower Parameter Bounds

**Symptom:** Crash during `@set ... wizpower` with unusual parameter values.

**Likely cause:** Parameter outside 1-128 range causing out-of-bounds array access.

**Diagnostic approach:** Review set command logs for wizpower operations with values outside valid range. Check core dumps for memory corruption near wizPowers array.

**Fix:** The code should validate bounds before enum cast. Delete invalid rows from wizpower table preventing load-time issues.
