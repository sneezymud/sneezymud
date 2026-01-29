---
title: Administrative Systems
description: Server administration including wizard powers, set command security, database migration, and remote execution (force/at/load)
keywords: [POWER_*, hasWizPower, limitPowerCheck, doSet, POWER_SET_IMP_POWER, privilege escalation, doForce, doAt, doLoad, LOW command, dual-database, mvroom, mvmob, mvobj, blocka, blockb, immortal database]
category: Critical Systems

last_updated: 2026-01-29
source_files: [code/code/cmd/cmd_low.cc, code/code/cmd/cmd_set.cc, code/code/misc/immortal.cc, code/code/misc/wiz_powers.cc]
related:
  - builder-systems.md
  - persistence-storage.md
  - cgi-security.md
  - snoop-switch.md
---

# Administrative Systems

**SECURITY CRITICAL:** Administrative systems control server access, player manipulation, and content migration. Misuse can lead to complete server compromise, unauthorized content modification, or privilege escalation.

## Table of Contents

- [Wizard Powers System](#wizard-powers-system)
- [The @set Command](#the-set-command)
- [Database Migration (LOW Command)](#database-migration-low-command)
- [Remote Execution Commands](#remote-execution-commands)
- [Purge and Restore Commands](#purge-and-restore-commands)
- [Security Vulnerabilities](#security-vulnerabilities)

---

## Wizard Powers System

The wizard powers system is a capability-based access control mechanism for immortals (staff members with level > 50). Each immortal can be granted individual powers that unlock specific commands or bypass restrictions.

### Power Enumeration

**File:** `code/code/misc/wiz_powers.h` (lines 11-130)

The system defines **128 powers** (indices 0-127) organized into functional categories:

#### Core Title Powers

| Power | Value | Effect |
|-------|-------|--------|
| `POWER_BUILDER` | 29 | Builder title, base builder permissions |
| `POWER_GOD` | 30 | God title, admin-level permissions |
| `POWER_WIZARD` | 31 | Wizard title, bypasses IDLED restrictions |

#### Critical Powers

| Power | Risk | Effect |
|-------|------|--------|
| `POWER_NO_LIMITS` | EXTREME | **Bypasses ALL vnum/zone restrictions** |
| `POWER_SET_IMP_POWER` | EXTREME | Arbitrary parameter modification (>100 limits) |
| `POWER_FORCE` | EXTREME | Execute commands as other players |
| `POWER_SWITCH` | EXTREME | Possess player characters |
| `POWER_LOW` | HIGH | Database migration between immortal/sneezy |
| `POWER_IDLED` | SPECIAL | Restricts immortal to 5 powers only |

#### IMP_POWER Variants

These powers bypass **all safety restrictions** in their respective commands:

| Base Power | IMP Variant | Bypasses |
|------------|-------------|----------|
| `POWER_FLAG` | `POWER_FLAG_IMP_POWER` | Cannot be snooped |
| `POWER_GOTO` | `POWER_GOTO_IMP_POWER` | Zone vnum restrictions |
| `POWER_LOAD` | `POWER_LOAD_IMP_POWER` | Vnum limits, max_exist |
| `POWER_MEDIT` | `POWER_MEDIT_IMP_POWER` | Zone assignments |
| `POWER_OEDIT` | `POWER_OEDIT_IMP_POWER` | Zone assignments |
| `POWER_SEDIT` | `POWER_SEDIT_IMP_POWER` | Zone assignments |
| `POWER_SET` | `POWER_SET_IMP_POWER` | Parameter limits (>100) |

### Storage and Persistence

**Memory Storage:** `code/code/misc/person.h` (lines 31-32)

```cpp
class TPerson {
    bool wizPowers[MAX_POWER_INDEX];           // Current power state
    bool wizPowersOriginal[MAX_POWER_INDEX];   // Last saved state
};
```

**Database Schema:** `_Setup-data/sql_tables/sneezy/wizpower.sql`

```sql
CREATE TABLE `wizpower` (
  `player_id` int(11) default NULL,
  `wizpower` int(11) default NULL,
  KEY `wizpower_idx` (`player_id`)
);
```

**CRITICAL SECURITY ISSUES:**
1. **No PRIMARY KEY** - Allows duplicate power entries
2. **No UNIQUE constraint** - Same power can be granted multiple times
3. **No FOREIGN KEY** - Orphaned powers if player deleted
4. **No ON DELETE CASCADE** - Manual cleanup required

### Power Management Functions

**hasWizPower() - Check Power**

```cpp
bool TPerson::hasWizPower(wizPowerT value) const {
    if (value >= MAX_POWER_INDEX)
        return FALSE;

    // CRITICAL: IDLED immortals severely restricted
    if (wizPowers[POWER_IDLED] && !wizPowers[POWER_WIZARD])
        switch (value) {
            case POWER_BUILDER:
            case POWER_GOD:
            case POWER_WIZARD:
            case POWER_GOTO:
            case POWER_IDLED:
                break;  // Only these 5 allowed for IDLED
            default:
                return false;  // All others blocked
        }

    return wizPowers[value];
}
```

**setWizPower() / remWizPower()** - Grant/Revoke Power

```cpp
void TPerson::setWizPower(wizPowerT value) {
    if (value >= MAX_POWER_INDEX) return;
    wizPowers[value] |= 0x1;
}

void TPerson::remWizPower(wizPowerT value) {
    if (value >= MAX_POWER_INDEX) return;
    wizPowers[value] &= ~(0x1);
}
```

**saveWizPowers()** - Persist to Database

```cpp
void TPerson::saveWizPowers() {
    if (GetMaxLevel() <= MAX_MORT) return;

    TDatabase db(DB_SNEEZY);

    // Only save changed powers
    for (wizPowerT num = MIN_POWER_INDEX; num < MAX_POWER_INDEX; num++) {
        if (wizPowers[num] != wizPowersOriginal[num]) {
            if (!wizPowersOriginal[num])
                db.query("insert into wizpower (player_id, wizpower) values (%i, %i)",
                    getPlayerID(), mapWizPowerToFile(num));
            else
                db.query("delete from wizpower where player_id=%i and wizpower=%i",
                    getPlayerID(), mapWizPowerToFile(num));
        }
        wizPowersOriginal[num] = wizPowers[num];
    }
}
```

**Race Condition Risk:** Multiple simultaneous saves could corrupt power state due to lack of transactions.

### Power Groups

**File:** `code/code/misc/wiz_powers.cc` (lines 105-329)

Power packages for convenient assignment:

| Group | Command | Powers Granted | Use Case |
|-------|---------|----------------|----------|
| **basic** | `@set ... wizpower basic` | 6 powers | Entry-level immortal |
| **rooms** | `@set ... wizpower rooms` | 7 powers | Room builders |
| **mobs** | `@set ... wizpower mobs` | 8 powers | Mob builders |
| **objs** | `@set ... wizpower objs` | 7 powers | Object builders |
| **quest** | `@set ... wizpower quest` | 8 powers | Quest immortals |
| **demi** | `@set ... wizpower demi` | 14 powers | Demigods |
| **trust** | `@set ... wizpower trust` | 8 powers | High-trust staff |
| **god** | `@set ... wizpower god` | 23 powers | Full administrators |
| **allpowers** | `@set ... wizpower allpowers` | **ALL 127 powers** | **Unrestricted access** |

**BUG ALERT - Line 282:**
```cpp
} else if (arg == "remgod") {
    ch->remWizPower(POWER_LOW);
    // ...
    ch->setWizPower(POWER_DISTRIBUTE);  // BUG: Should be remWizPower!
}
```

Removing god powers accidentally **grants** POWER_DISTRIBUTE.

### Vnum Limit System

**limitPowerCheck()** enforces zone restrictions based on builder assignments:

```cpp
bool TBeing::limitPowerCheck(cmdTypeT cmd, int vnum) {
    // CRITICAL: Complete bypass with NO_LIMITS
    if (hasWizPower(POWER_NO_LIMITS))
        return TRUE;

    // Extract builder's assigned zone ranges
    int as = desc->blockastart, ae = desc->blockaend;  // Block A
    int bs = desc->blockbstart, be = desc->blockbend;  // Block B
    int o = desc->office;

    // Check vnum against allowed ranges
    switch (cmd) {
        case CMD_GOTO:
            if ((vnum >= as && vnum <= ae) || (vnum >= bs && vnum <= be) ||
                vnum == o || (vnum >= 0 && vnum <= 100))
                return TRUE;
            break;

        case CMD_LOAD:
        case CMD_SHOW:
            if ((vnum >= as && vnum <= ae) || (vnum >= bs && vnum <= be) ||
                isGenericMob(vnum) || isGenericObj(vnum))
                return TRUE;
            break;
    }

    return FALSE;
}
```

---

## The @set Command

The `@set` command is SneezyMUD's primary administrative interface for modifying character properties, stats, permissions, and game state.

**Core file:** `code/code/cmd/cmd_set.cc` (1280 lines)
**Entry point:** `TPerson::doSet(const char* argument)` (line 23)

### Access Control Hierarchy

**Base Requirement:**
```cpp
if (!hasWizPower(POWER_SET)) {
    sendTo("You don't have the power to @set\n\r");
    return;
}
```

**Privilege Escalation Gate:**
```cpp
// Line 51-55
if (mob->isPc() && mob->hasWizPower(POWER_SET_IMP_POWER) &&
    !hasWizPower(POWER_SET_IMP_POWER)) {
    sendTo("You can't do that!\n\r");
    return;
}
```

**Rule:** Immortals with `POWER_SET` cannot modify immortals who have `POWER_SET_IMP_POWER`.

This creates a two-tier privilege system:

| Tier | Powers | Can Modify |
|------|--------|------------|
| **Standard** | `POWER_SET` | Mortals, NPCs, low-level immortals |
| **Administrative** | `POWER_SET` + `POWER_SET_IMP_POWER` | All characters, including other immortals |

### Administrative Setting Restrictions

For sensitive operations (office, blocka, blockb, wizpower):

```cpp
// Line 128-135
if ((!mob->desc || mob->GetMaxLevel() <= MAX_MORT ||
      mob->hasWizPower(POWER_WIZARD) ||
      !hasWizPower(POWER_SET_IMP_POWER)) &&
    mob != this) {
    sendTo("You can not do this!\n\r");
    return;
}
```

**Enforcement criteria:**
1. Target must be connected (`mob->desc` exists)
2. Target must be immortal (`GetMaxLevel() > MAX_MORT`)
3. Target must NOT have `POWER_WIZARD`
4. Executor must have `POWER_SET_IMP_POWER`
5. **Exception:** Self-modification always allowed

### Subcommands

#### Character Management

| Subcommand | Parameters | Effect | Restrictions |
|------------|------------|--------|--------------|
| `character` | `<level> <class> [learning%]` | Rebuild character at specified level/class | Cannot reduce level; max level `MAX_MORT` |
| `class` | `<class_index>` | Change character class | Must be valid class index |
| `level` | `<level> <class>` | Set class-specific level | 0 to `MAX_MORT` |
| `race` | `<race_number>` | Change race | Must be valid race |
| `exp` | `<experience>` | Set experience points | Direct XP assignment |

#### Stats and Attributes

All 11 primary stats can be set directly (strength, brawn, constitution, dexterity, agility, intelligence, wisdom, focus, perception, charisma, karma, speed):

```cpp
// Line 644
if (parm > 100 && !hasWizPower(POWER_SET_IMP_POWER)) {
    sendTo("You do not have the authority to modify above 100.\n\r");
    return;
}
```

**Range:** 5-205 (values >100 require `POWER_SET_IMP_POWER`)

#### Administrative Settings

| Subcommand | Parameters | Effect | Requires |
|------------|------------|--------|----------|
| `wizpower` | `<power_number>` | Toggle wizard power | `POWER_SET_IMP_POWER` for immortals |
| `power` | `<package_name>` | Set power package | `POWER_SET_IMP_POWER` |
| `office` | `<room_vnum>` | Set builder office | `POWER_SET_IMP_POWER` |
| `blocka` | `<start> <end>` | Set primary build range | `POWER_SET_IMP_POWER` |
| `blockb` | `<start> <end>` | Set secondary build range | `POWER_SET_IMP_POWER` |

#### Resource Management

| Subcommand | Parameters | Effect |
|------------|------------|--------|
| `hit` | `<current_hp>` | Set current HP |
| `mana` | `<current_mana>` | Set current mana |
| `move` | `<current_move>` | Set current movement |
| `bank` | `<talens>` | Set bank account balance |
| `gold` | `<talens>` | Set carried money |
| `practices` | `<amount>` | Set practice points |

### Database Persistence

All `@set` operations trigger immediate save:

```cpp
mob->doSave(SILENT_NO);  // Most subcommands
mob->saveChar(FALSE);    // Some stat modifications
```

**Dual Storage Model:**

| Property | Binary File | Database Table | Authoritative On Load |
|----------|-------------|----------------|-----------------------|
| Money | `charFile.money` | `player.talens` | Database |
| Wizpowers | `charFile.wizPowers[]` | `wizpower` | Database |
| Stats | `charFile` only | - | Binary file |

---

## Database Migration (LOW Command)

Admin operations center around the `low` command for migrating game content from development (immortal) to production (sneezy) databases.

**Source:** `code/code/cmd/cmd_low.cc`
**Power:** `POWER_LOW` required

### Dual-Database Architecture

SneezyMUD uses two separate MariaDB databases:

| Database | Enum Constant | Purpose |
|----------|---------------|---------|
| `immortal` | `DB_IMMORTAL` | Builder workspace - rooms/mobs/objects owned by individual builders |
| `sneezy` | `DB_SNEEZY` | Production data - live game content used by running server |

**Schema Differences:**

The `immortal` database tables have additional columns:

| Column | Type | Purpose |
|--------|------|---------|
| `owner` | `varchar(32)` | Builder name who created/owns this entry |
| `block` | `int` | Version/revision number (rooms only) |

**Primary Key Difference:**
- `sneezy.room`: `PRIMARY KEY (vnum)`
- `immortal.room`: `PRIMARY KEY (owner, vnum)`

This allows multiple builders to have their own versions of the same vnum during development.

### Migration Commands

**Transaction Behavior:**
- In-game commands use `begin`/`commit` transactions
- CLI tools do NOT use transactions

#### low mvroom - Move Rooms

```
low mvroom <builder_name> <block> <vnum_list>
```

**Parameters:**
- `builder_name`: Owner in the immortal database (e.g., "Batopr")
- `block`: Block number (must be 1 or 2)
- `vnum_list`: Room vnums to migrate (e.g., "45660-45664 45670")

**Tables affected:** `room`, `roomextra`, `roomexit`

**Behavior:**
1. Wraps all operations in a transaction (`begin`/`commit`)
2. For each vnum:
   - Queries `immortal.room` WHERE `owner='<builder>' AND vnum=<vnum> AND block=<block>`
   - Deletes existing entry in `sneezy.room` WHERE `vnum=<vnum>`
   - Inserts new row from immortal data
   - Repeats for `roomextra` and `roomexit` tables

#### low mvobj - Move Objects

```
low mvobj <builder_name> <vnum_list>
```

**Tables affected:** `obj`, `objextra`, `objaffect`

**Special Processing:**
- Strips `ITEM_STRUNG` bit (bit 2) from `action_flag`
- Strips `ITEM_PROTOTYPE` bit (bit 4) from `action_flag`

#### low mvmob - Move Mobiles

```
low mvmob <builder_name> <vnum_list>
```

**Tables affected:** `mob`, `mob_extra`, `mob_imm`

**Special Processing:**
- Clears `ACT_STRINGS_CHANGED` bit from actions field

#### low mvresponse - Move Mob Responses

```
low mvresponse <builder_name> [vnum_list]
```

**Table affected:** `mobresponses`

**Modes:**
- No vnum_list: Lists all responses owned by builder
- With vnum_list: Transfers specified responses

### Analysis Commands

| Command | Function |
|---------|----------|
| `low mobs <level>` | List mobs by level with stats |
| `low race <race_num> [nostat]` | List mobs by race |
| `low path <dest_room>` | Pathfinding from current room |
| `low tasks list [name]` | Task tracking system |
| `low statbonus` | Object bonus statistics |
| `low statcharts` | Race stat charts |

### Vnum List Format

All tools accept vnum lists in this format:
- Individual vnums: `13700 13791 13798`
- Ranges: `13700-13780`
- Mixed: `13700-13780 13791 13798`

**Implementation:** `parse_num_args()` in `code/code/low/lowtools.cc`

### Data Integrity Warnings

**Delete-Before-Insert Pattern:**

All migration tools use a delete-before-insert pattern:
```cpp
db_beta.query("delete from room where vnum=%i", vnum);
db_beta.query("insert into room ... values ...");
```

**Risk:** If the insert fails after delete, data is lost. The in-game commands wrap operations in transactions; CLI tools do not.

**No Existence Checking:**

Migration tools verify that source data exists before deleting destination data. The check happens correctly - delete only occurs if source exists.

---

## Remote Execution Commands

### Force Command

**Source:** `code/code/misc/immortal.cc` lines 2144-2250
**Entry point:** `TPerson::doForce(const char* argument)`
**Power:** `POWER_FORCE`

Forces another being (player or NPC) to execute a command as if they typed it themselves.

**Syntax:**
```
force <target> <command>
```

**Target options:**

| Target | Description |
|--------|-------------|
| `<name>` | Specific player or mob by name |
| `all` | All connected players (PCs only) |
| `room` | All beings in your current room |
| `mobs` | All NPCs in your current room |

**Permission Requirements:**
1. **Base requirement:** `POWER_FORCE`
2. **Level restriction:** Cannot force players of equal or higher level
3. **Vnum restriction:** `limitPowerCheck(CMD_FORCE, vnum)` - target mob vnum must be in your assigned block

**DELETE Flag Handling:**

Force commands must handle DELETE_THIS returns from `parseCommand()`:

```cpp
if ((rc = vict->parseCommand(to_force, FALSE)) == DELETE_THIS) {
    delete vict;
    vict = NULL;
}
```

**Critical:** If the forced command kills or destroys the target, the target is deleted immediately.

**Logging:**

Force-all commands are logged:
```cpp
vlogf(LOG_MISC, format("%s just forced all to '%s'") % getName() % to_force);
```

### At Command

**Source:** `code/code/misc/immortal.cc` lines 1017-1124
**Entry point:** `TBeing::doAt(const char* argument, bool isFarlook)`
**Power:** `POWER_AT`

Teleports the immortal to a remote location, executes a command, then returns them to their original location.

**Syntax:**
```
at <location> <command>
```

**Location options:**

| Location | Resolution |
|----------|------------|
| `<room_vnum>` | Direct room number |
| `<player_name>` | Player's current room |
| `<mob_name>` | NPC's current room |
| `<object_name>` | Object's room (if in a room) |

**Permission Requirements:**
1. **Base requirement:** `POWER_AT` (unless `isFarlook` is true)
2. **Restricted rooms:** `Room::STORAGE` requires `POWER_GOTO_IMP_POWER`
3. **Idled restriction:** Immortals with `POWER_IDLED` cannot `at` outside Imperia

**DELETE Flag Handling:**

Returns DELETE_THIS if the executed command kills the immortal:
```cpp
if (IS_SET_DELETE(rc, DELETE_THIS))
    return DELETE_THIS;
```

### Load Command

**Source:** `code/code/misc/immortal.cc` lines 2300-2458
**Entry point:** `TPerson::doLoad(const char* argument)`
**Power:** `POWER_LOAD`

Creates new instances of mobiles (NPCs), objects, or equipment sets from the game's template database.

**Syntax:**
```
load mobile <vnum|name>
load object <vnum|name>
load set <set_name>
load <count>.<type> <vnum>
```

**Permission Requirements:**

| Operation | Required Power |
|-----------|----------------|
| Basic loading | `POWER_LOAD` |
| Load equipment sets | `POWER_LOAD_SET` |
| Load without prototype flag | `POWER_LOAD_NOPROTOS` |
| Load limited items beyond max_exist | `POWER_LOAD_LIMITED` |
| Load restricted items (shopkeepers, special potions) | `POWER_LOAD_IMP_POWER` |

**Restricted Objects:**

Without `POWER_LOAD_IMP_POWER`, these cannot be loaded:
- `Obj::DEITY_TOKEN`
- `Obj::YOUTH_POTION`
- `Obj::STATS_POTION`
- `Obj::LEARNING_POTION`
- `Obj::MYSTERY_POTION`
- `Obj::CRAPS_DICE`

**max_exist Enforcement:**

Objects have a maximum allowed count in the game:
```cpp
if ((obj_index[numx].getNumber() >= obj_index[numx].max_exist) &&
    !hasWizPower(POWER_LOAD_LIMITED)) {
    sendTo("Sorry, all of those items are presently in use.\n\r");
    return;
}
```

**Prototype Flag Enforcement:**

Without `POWER_LOAD_NOPROTOS`, objects become prototypes:
```cpp
if (!hasWizPower(POWER_LOAD_NOPROTOS)) {
    if (!obj->isObjStat(ITEM_PROTOTYPE)) {
        obj->addObjStat(ITEM_PROTOTYPE);
        sendTo("Changing the object to a prototype.\n\r");
    }
}
```

**Important:** Prototype items cannot be used by mortals.

---

## Purge and Restore Commands

### Purge Command

**Source:** `code/code/misc/immortal.cc:2661-2900`

The `purge` command removes objects and mobiles from the game world.

**Syntax:**
```
purge                              # Purge all objects/mobs in current room
purge <target>                     # Purge specific object or mobile
purge zone <zone_number>           # Nuke all excess mobs in a zone
purge room <start> [end]           # Evacuate all entities from room range
```

**Permission Requirements:**

| Power | Description |
|-------|-------------|
| `POWER_PURGE` | Required for basic purge operations |
| `POWER_PURGE_PC` | Required to purge player characters |
| `POWER_PURGE_ROOM` | Required for `purge room` command |

**Protected Items:**

Objects with the `ITEM_NOPURGE` flag (bit 23) are immune to purge operations.

### Restore Command

**Source:** `code/code/misc/immortal.cc:3330-3501`

The `restore` command heals characters by restoring their hit points, mana, movement, and other vital statistics.

**Syntax:**
```
restore <character> partial    # Restore HP/mana/move only
restore <character> full       # Full restore including limbs, diseases, affects
restore <character> pracs      # Reimburse practices
```

**Permission Requirements:**

| Power | Description |
|-------|-------------|
| `POWER_RESTORE` | Required for all restore operations |
| `POWER_RESTORE_MORTAL` | Required to restore mortal characters |

**Restore Types:**

**Partial Restore:**
- Hit points (`hitLimit()`)
- Mana (`manaLimit()`)
- Movement (`moveLimit()`)
- Piety (`pietyLimit()`)
- Hunger/Thirst (set to 24 or -1 for immortals)

**Full Restore:**
- All partial restore actions
- Limb healing (all limbs restored to maximum)
- Affect removal (including diseases, debuffs)
- Dispel magic
- Spirit chase

---

## Security Vulnerabilities

### Critical: Self-Modification Exception

**Location:** `cmd_set.cc` Line 135

**Issue:** The condition allowing self-modification bypasses `POWER_SET_IMP_POWER` requirement.

```cpp
if ((...privilege checks...) && mob != this) {
    sendTo("You can not do this!\n\r");
    return;
}
```

**Consequence:** Any immortal with `POWER_SET` can modify their own:
- `office` (builder workspace)
- `blocka`/`blockb` (build permission ranges)
- `wizpower` (including granting themselves more powers)

**Attack scenario:**
1. Immortal has `POWER_SET` but not `POWER_SET_IMP_POWER`
2. Uses `@set self wizpower <POWER_SET_IMP_POWER_number>`
3. Gains administrative privileges
4. Can now modify other immortals

### Critical: Wizpower Toggle Without Validation

**Location:** `cmd_set.cc` Line 269

**Issue:** User-provided integer converted to enum with minimal bounds checking.

```cpp
int prm = convertTo<int>(arg2);
wizPowerT wpt = wizPowerT(prm - 1);

if (!mob->hasWizPower(wpt)) {
    mob->setWizPower(wpt);
```

**Vulnerabilities:**
1. Negative values: `prm=0` becomes `wpt=-1` (undefined enum value)
2. Out-of-bounds: `prm=999` becomes `wpt=998` (beyond `MAX_POWER_INDEX`)

**Mitigation:**
```cpp
if (prm < 1 || prm > MAX_POWER_INDEX) {
    sendTo("Invalid power number.\n\r");
    return;
}
```

### High: Database Non-Atomic Operations

**Location:** `wiz_powers.cc` `wizFileWrite()` called from `doSave()`

**Issue:** Delete-then-insert pattern for wizpower persistence.

```cpp
db.query("delete from wizpower where player_id=%i", player_id);
for each power:
    db.query("insert into wizpower values (%i, %i)", player_id, power);
```

**Consequence:** Crash or disconnect between delete and insert loses all wizard powers.

**Mitigation:** Wrap in transaction:
```cpp
db.query("BEGIN");
db.query("delete from wizpower where player_id=%i", id);
for each power:
    db.query("insert into wizpower values (%i, %i)", player_id, power);
db.query("COMMIT");
```

### Privilege Escalation Chains

**Chain 1: Takeover → Arbitrary Command**

```
1. Attacker has: POWER_SWITCH + POWER_FORCE
2. Switch to higher-privilege target
3. Force target to: @set <self> wizpower set_imp_power
4. Return from switch
5. Target now has unrestricted @set capability
```

**Chain 2: Self-Escalation via SET**

```
1. Attacker has: POWER_SET + POWER_SET_IMP_POWER
2. Execute: @set <self> wizpower <desired_power>
3. Bypass power grant workflow entirely
```

**Chain 3: Database Bypass via LOW**

```
1. Attacker has: POWER_LOW + POWER_GOD
2. Use LOW command to migrate malicious mobs/objects
3. Mobs have spec procs that grant powers
4. Full control over production database
```

## Security Best Practices

### 1. Principle of Least Privilege

Grant only powers necessary for assigned role:

**Good:**
```
@set <builder> wizpower basic
@set <builder> wizpower rooms
```

**Bad:**
```
@set <builder> wizpower allpowers
```

### 2. Never Grant These Powers Lightly

**EXTREME RISK:**
- `POWER_NO_LIMITS`
- `POWER_SET_IMP_POWER`
- `POWER_FORCE`
- `POWER_SWITCH`

**HIGH RISK:**
- `POWER_LOW`
- All `_IMP_POWER` variants
- `POWER_PURGE_PC`

### 3. Audit Power Changes

Log all power grants/revokes with:
- Who granted the power
- To whom it was granted
- Timestamp
- Reason

### 4. Database Hardening

**Recommended schema changes:**

```sql
ALTER TABLE wizpower
  ADD PRIMARY KEY (player_id, wizpower),
  ADD FOREIGN KEY (player_id) REFERENCES player(id)
    ON DELETE CASCADE,
  ADD INDEX idx_wizpower (wizpower);
```

### 5. Code Fixes

**Priority fixes:**

1. **Remove self-modification exception for administrative settings**
2. **Add bounds validation to wizpower parameter**
3. **Wrap database operations in transactions**
4. **Add descriptor null checks before dereferencing**
5. **Validate vnum references against game world**

## Related Documentation

- [Builder Systems](builder-systems.md) - OLC editors and content creation
- [Database Queries](database-queries.md) - TDatabase API and escaping
- [CGI Security](cgi-security.md) - Web interface security
- [Snoop/Switch](snoop-switch.md) - Surveillance and character possession
