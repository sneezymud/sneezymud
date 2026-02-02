---
title: Administrative Systems
category: critical
keywords: [wizard-powers, set-command, database-migration, remote-execution, privilege-escalation]
related: [builder-systems.md, persistence-storage.md, cgi-security.md, snoop-switch.md]
created_by_model: opus
primary_symbols:
  functions: [hasWizPower, limitPowerCheck, doSet, doForce, doAt, doLoad, saveWizPowers]
  classes: [TPerson, TBeing]
  files: [code/code/cmd/cmd_set.cc, code/code/cmd/cmd_low.cc, code/code/misc/immortal.cc, code/code/misc/wiz_powers.cc]
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

| Package | Command | Powers | Use Case |
|---------|---------|--------|----------|
| basic | `@set ... wizpower basic` | 6 | Entry-level immortal |
| rooms | `@set ... wizpower rooms` | 7 | Room builders |
| mobs | `@set ... wizpower mobs` | 8 | Mob builders |
| objs | `@set ... wizpower objs` | 7 | Object builders |
| quest | `@set ... wizpower quest` | 8 | Quest immortals |
| demi | `@set ... wizpower demi` | 14 | Demigods |
| trust | `@set ... wizpower trust` | 8 | High-trust staff |
| god | `@set ... wizpower god` | 23 | Full administrators |
| allpowers | `@set ... wizpower allpowers` | 127 | Unrestricted access |

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

### Power Check Flow

When `hasWizPower()` is called, it first validates the power index is within bounds. For IDLED immortals (those with `POWER_IDLED` but lacking `POWER_WIZARD`), the function returns false for all powers except BUILDER, GOD, WIZARD, GOTO, and IDLED itself. This implements a restricted mode for inactive staff who retain their immortal status but lose most capabilities.

The `limitPowerCheck()` function enforces zone boundaries. It extracts the builder's assigned ranges from their descriptor (blockastart/blockaend, blockbstart/blockbend, office) and checks if the requested vnum falls within those ranges. Special cases include: always allowing vnums 0-100 (Imperia), always allowing generic mobs/objects, and completely bypassing checks for immortals with `POWER_NO_LIMITS`.

### @set Command Structure

The `doSet()` function in TPerson parses the target and subcommand, then dispatches to the appropriate handler. The privilege escalation gate early in the function prevents standard immortals from modifying those with `POWER_SET_IMP_POWER`.

Administrative settings (office, blocka, blockb, wizpower) have additional checks: target must be connected (have a descriptor), target must be immortal, target must not have `POWER_WIZARD`, and executor must have `POWER_SET_IMP_POWER`. A problematic exception allows self-modification (`mob != this`) to bypass these checks.

Stat modifications compare the requested value against 100. Values above 100 require `POWER_SET_IMP_POWER`. The valid range is 5-205.

All @set operations trigger immediate persistence via `doSave()` or `saveChar()`. The dual storage model means some properties (money, wizpowers) use database tables as authoritative while others (stats) rely on binary character files.

### Database Migration Architecture

The dual-database design separates builder workspace (immortal) from production (sneezy). The immortal database includes additional columns: `owner` (builder name) and `block` (version number for rooms). This allows multiple builders to work on the same vnums simultaneously without conflict.

Primary key differences enable this isolation. Production uses `PRIMARY KEY (vnum)` while development uses `PRIMARY KEY (owner, vnum)`. A builder's version of room 45660 coexists with other builders' versions until migration.

Migration commands follow a delete-before-insert pattern. The command first queries the source (immortal) database for the specified owner/vnum combination, then deletes any existing entry in the destination (sneezy) database, and finally inserts the source data. In-game commands wrap this in transactions; CLI tools do not.

Object migration strips prototype flags (ITEM_STRUNG bit 2, ITEM_PROTOTYPE bit 4) from action_flag. Mob migration clears ACT_STRINGS_CHANGED. These transformations prepare builder content for production use.

### Remote Execution Mechanics

The force command resolves its target, validates permissions (must be higher level than target, target vnum must be in assigned block for mobs), and calls `parseCommand()` on the victim with the specified command string. The critical detail is DELETE_THIS handling: if the forced command kills the target, the target is deleted immediately and the loop continues to the next victim (for force all/room/mobs variants).

The at command teleports the executor to the target location, runs the command, then returns to the original room. It must handle DELETE_THIS from the executed command since the immortal themselves might die from what they executed remotely.

The load command creates instances from the template database. Permission layers include: base loading requires `POWER_LOAD`, equipment sets require `POWER_LOAD_SET`, non-prototype loading requires `POWER_LOAD_NOPROTOS`, loading beyond max_exist requires `POWER_LOAD_LIMITED`, and certain restricted objects require `POWER_LOAD_IMP_POWER`.

### Power Package Groups

Power packages exist in `wiz_powers.cc` as named collections. When a package is requested, the code grants or revokes the associated powers in sequence. Package membership is hardcoded rather than data-driven.

A bug in the remgod package handler calls `setWizPower(POWER_DISTRIBUTE)` instead of `remWizPower(POWER_DISTRIBUTE)`, accidentally granting a power when trying to remove the god package.

### Purge and Restore

The purge command operates in several modes: single target (specified object or mob), room-wide (all entities in current room), zone-wide (excess mobs in specified zone), and room range (evacuate entities from vnum range). Objects with ITEM_NOPURGE flag are immune.

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
