---
title: Zone Management System
description: The zone management system handles the complete lifecycle of game zones from server startup through runtime resets. Zones progress through four distinct phases - initialization (startup), aging (runtime), execution (runtime), and cleanup (post-reset).
keywords: bootZones, bootOneZone, bootZone, renumCmd, resetZone, zoneData, resetCom, procZoneUpdate, doGenericReset, isEmpty, zone_nr, lifespan, reset_mode, if_flag, load-on-death, util_flag, ZO_DEAD
category: Critical Systems

  - database-queries.md
  - scheduler-pulses.md
last_updated: 2026-01-29
source_files:
  - code/code/sys/db.cc
  - code/code/sys/db.h
  - code/code/sys/socket.cc
  - code/code/misc/cmd_show.cc
  - code/code/misc/cmd_stat.cc
  - code/code/misc/low.h
related: [admin-systems.md]
---

# Zone Management System

The zone management system handles the complete lifecycle of game zones from server startup through runtime resets. Understanding this system is critical for managing world content and preventing server performance issues.

**Misusing this system can corrupt world state.** Common errors: creating zones with invalid vnums, forgetting to enable zones in zonefiles, not validating mob/object vnums before publishing, leaving orphaned database entries.

## Overview

Zones progress through four distinct phases:

1. **Initialization (startup)**: `bootZones()` → `bootOneZone()` → `bootZone()` → `renumCmd()` → database sync
2. **Aging (runtime)**: `procZoneUpdate` increments age counters, queues zones when `age >= lifespan`
3. **Execution (runtime)**: `resetZone()` processes command table, spawns mobs/objects
4. **Cleanup (post-reset)**: `doGenericReset()` triggers special procedures, age resets to 0

**Key characteristics:**
- Zones discovered dynamically via directory scan (no hardcoded list)
- Automatic sorting by vnum using multimap
- Database synchronization via util_flag pattern
- Conditional command execution via if_flag
- Load-on-death system for mob equipment
- Random room placement support

## Zone Data Structures

### zoneData Class

```cpp
class zoneData {
  public:
    int zone_nr;              // Sequential zone number (0, 1, 2...)
    int bottom;               // Lowest room vnum in zone
    int top;                  // Highest room vnum in zone
    int lifespan;             // Minutes between reset attempts
    int age;                  // Minutes since last reset
    int reset_mode;           // 0=never, 1=when empty, 2=always
    bool enabled;             // Zone active flag
    sstring name;             // Zone name
    std::vector<resetCom> cmd_table;  // Reset commands from zonefile

    bool bootZone(int zone_nr);
    void renumCmd();
    void resetZone(resetFlag flag);
    bool isEmpty() const;
};
```

**Source:** `code/code/sys/db.h` (lines 176-234)

### resetCom Class

Represents a single zone reset command:

```cpp
class resetCom {
  public:
    char command;      // Command type (M/O/G/E/P/D/T/etc.)
    bool if_flag;      // Execute only if previous succeeded
    int arg1;          // Command-specific argument 1
    int arg2;          // Command-specific argument 2
    int arg3;          // Command-specific argument 3
    int arg4;          // Command-specific argument 4 (traps, loot)
    char character;    // Character for '?' command

    int execute(zoneData*, TMonster**, TObj**, TBeing**, int*, resetFlag);
};
```

**Source:** `code/code/sys/db.h` (lines 100-174)

## Phase 1: Initialization (Startup)

### bootZones() - Zone Discovery and Loading

Called once at server startup, this coordinator function discovers and loads all zones.

**Source:** `code/code/sys/socket.cc:1652`, `code/code/sys/db.cc:1661-1696`

```cpp
void bootZones(void) {
    DIR* dfd;
    struct dirent* dp;
    int zon = 0, tmp;
    std::multimap<int, sstring, std::less<int>> files;
    std::multimap<int, sstring, std::less<int>>::iterator it;
    TDatabase db(DB_SNEEZY);

    // Open zonefiles directory
    if (!(dfd = opendir("zonefiles"))) {
        vlogf(LOG_BUG, "couldn't open zonefiles directory");
        perror("bootZones");
        exit(0);
    }

    // Discover all zone files
    while ((dp = readdir(dfd))) {
        if (!strcmp(dp->d_name, ".") || !strcmp(dp->d_name, ".."))
            continue;

        tmp = convertTo<int>(dp->d_name);

        // Filter non-numeric filenames (except special "0" zone)
        if (tmp == 0 && strcmp(dp->d_name, "0"))
            continue;

        // Insert into multimap for automatic sorting
        files.insert(std::pair<int, sstring>(tmp, dp->d_name));
    }

    // Reset all util_flags to 0 before boot
    db.query("update zone set util_flag = 0");

    // Process zones in sorted order by vnum
    for (it = files.begin(); it != files.end(); ++it) {
        bootOneZone(db, it->first, zon);
    }

    // Delete orphaned zones (util_flag still 0)
    db.query("delete from zone where util_flag = 0");
}
```

**Key behaviors:**
1. **Dynamic discovery**: Uses `readdir()` to find all files in `lib/zonefiles/`
2. **Automatic sorting**: Multimap sorts by vnum (key), not filename alphabetically
3. **util_flag pattern**: All zones start at 0, active zones set to 1 during boot, orphans deleted
4. **Filename validation**: Ignores ".", "..", and non-numeric files (except zone "0")
5. **Sequential zone_nr**: The `zon` counter assigns 0, 1, 2... regardless of file vnums

### bootOneZone() - Wrapper and Database Sync

Wraps the parsing process and handles database synchronization.

**Source:** `code/code/sys/db.cc:1635-1659`

```cpp
void bootOneZone(TDatabase& db, int zoneStart, int& zon) {
    zoneData zd;

    if (zd.bootZone(zoneStart)) {
        // Validate all mob/object vnums
        zd.renumCmd();

        vlogf(LOG_MISC, format("booting zone %d") % zon);
        zd.zone_nr = zon++;

        // Update existing zone or insert new one
        db.query(
            "update zone set zone_name = '%s', zone_enabled = %i, bottom = %i, "
            "top = %i, reset_mode = %i, lifespan = %i, util_flag = 1 "
            "where zone_nr = %i",
            zd.name.c_str(), (zd.enabled ? 1 : 0), zd.bottom, zd.top,
            zd.reset_mode, zd.lifespan, zd.zone_nr);

        if (db.rowCount() == 0) {
            // No existing record - insert new
            db.query(
                "insert into zone (zone_nr, zone_name, zone_enabled, bottom, top, "
                "reset_mode, lifespan, util_flag) values (%i, '%s', %i, %i, %i, "
                "%i, %i, 1)",
                zd.zone_nr, zd.name.c_str(), (zd.enabled ? 1 : 0), zd.bottom,
                zd.top, zd.reset_mode, zd.lifespan);
        }

        zone_table.push_back(zd);
    }
}
```

**Critical: zone_nr vs file vnum**
- **zone_nr**: Sequential runtime index (0, 1, 2...) assigned at boot
- **File vnum**: Zone's starting room number (15200, 45660, etc.)
- Zone insertion changes subsequent zone_nr values but not file vnums
- Database update/insert handles zone_nr changes gracefully

### bootZone() - Zonefile Parsing

Parses individual zonefile into memory structures.

**Source:** `code/code/sys/db.cc:1523-1633`

**Zonefile format:**
```
#zone_number
Zone Name~
top_room lifespan reset_mode enabled
M if_flag mob_vnum room_max room_vnum
E 1 obj_vnum max slot
S
```

**Parsing logic:**
```cpp
bool zoneData::bootZone(int zone_nr) {
    int tmp, i1, i2, i3, i4, rc;
    char buf[256];
    FILE* fl = fopen(((sstring)(format("zonefiles/%i") % zone_nr)).c_str(), "r");

    if (!fl) {
        perror("bootZone");
        return false;
    }

    // Parse header: #zone_number
    if (fscanf(fl, " #%d\n", &bottom) == EOF)
        vlogf(LOG_FILE, "Unexpected read error in bootZone");

    name = fread_string(fl);  // Zone Name~

    // Parse: top_room lifespan reset_mode enabled
    rc = fscanf(fl, " %d %d %d %d", &i1, &i2, &i3, &i4);
    if (rc == 4) {
        top = i1;
        lifespan = i2;
        reset_mode = i3;
        enabled = i4;
        age = 0;
    } else {
        vlogf(LOG_LOW, format("Bad zone format for zone %d (%s)") % zone_nr % name);
        return false;
    }

    cmd_table.clear();

    // Parse reset commands until 'S' terminator
    for (;;) {
        resetCom rs;

        if (fscanf(fl, " ") == EOF)
            vlogf(LOG_FILE, "Unexpected read error in bootZone");
        if (fscanf(fl, "%c", &rs.command) == EOF)
            vlogf(LOG_FILE, "Unexpected read error in bootZone");

        // 'S' marks end of zone
        if (rs.command == 'S') {
            cmd_table.push_back(rs);
            break;
        }

        // Skip comment lines ('*') and gamma-mode lines
        if (rs.command == '*' || (rs.command == '$' && gamePort == Config::Port::GAMMA)) {
            if (!fgets(buf, 255, fl))
                vlogf(LOG_FILE, "Unexpected read error in bootZone");
            continue;
        }

        // Parse first three arguments: if_flag arg1 arg2
        int numc = fscanf(fl, " %d %d %d", &tmp, &rs.arg1, &rs.arg2);
        if (numc != 3)
            vlogf(LOG_LOW, format("command %u ('%c') in %s missing some of first "
                                  "three args [%d : %d %d %d]") %
                           cmd_table.size() % rs.command % name % numc %
                           (numc >= 1 ? tmp : -99) % (numc >= 2 ? rs.arg1 : -99) %
                           (numc >= 3 ? rs.arg2 : -99));

        // Special handling for X command (armor sets)
        if (rs.command == 'X')
            rs.arg3 = tmp;
        else
            rs.if_flag = tmp;

        // Validate if_flag for equipment commands
        switch (rs.command) {
            case 'G':  // Give to mob
            case 'P':  // Put in container
            case 'E':  // Equip on mob
            case 'I':  // Equip as prop (load-on-death)
                if (!rs.if_flag) {
                    vlogf(LOG_LOW, format("command %u in %s has bogus if_flag") %
                                     cmd_table.size() % name);
                    continue;
                }
                break;
            default:
                break;
        }

        // Parse arg3 for commands that need it
        if (rs.command == 'M' || rs.command == 'O' || rs.command == 'B' ||
            rs.command == 'C' || rs.command == 'K' || rs.command == 'E' ||
            rs.command == 'I' || rs.command == 'P' ||
            (rs.command == 'T' && !rs.if_flag) || rs.command == 'R' ||
            rs.command == 'D' || rs.command == 'L')
            if ((rc = fscanf(fl, " %d", &rs.arg3)) != 1)
                vlogf(LOG_LOW, format("command %u ('%c') in %s missing arg3 (rc=%d)") %
                                 cmd_table.size() % rs.command % name % rc);

        // Parse character for '?' command (percent chance)
        if (rs.command == '?')
            if (fscanf(fl, " %c", &rs.character) != 1)
                vlogf(LOG_LOW, format("command %u ('?') in %s missing character") %
                                 cmd_table.size() % name);

        // Parse arg4 for trap commands
        if (rs.command == 'T' && !rs.if_flag)
            if (fscanf(fl, " %d", &rs.arg4) != 1)
                vlogf(LOG_LOW, format("command %u ('T') in %s missing arg4") %
                                 cmd_table.size() % name);

        // Parse arg4 for loot commands
        if (rs.command == 'L')
            if (fscanf(fl, " %d", &rs.arg4) != 1)
                vlogf(LOG_LOW, format("command %u ('L') in %s missing arg4") %
                                 cmd_table.size() % name);

        cmd_table.push_back(rs);

        // Consume rest of line
        if (!fgets(buf, 255, fl))
            vlogf(LOG_FILE, "Unexpected read error in bootZone");
    }

    fclose(fl);
    return true;
}
```

**Parsing behaviors:**
- Comments (`*` lines) skipped entirely
- Gamma-mode lines (`$` lines) skipped on production (port 7900)
- Equipment commands (G/P/E/I) **require** `if_flag = 1` or they're rejected
- Different commands require different argument counts:
  - All commands: if_flag, arg1, arg2
  - M/O/B/C/K/E/I/P/R/D/L/T: arg3
  - T (traps): arg4
  - L (loot): arg4
  - ?: character field

### renumCmd() - Vnum Validation

After parsing, validates all mob and object vnums exist in the database.

**Source:** `code/code/sys/db.cc:3468-3584`

**CRITICAL: Silent failure mode**

If a vnum doesn't exist, `real_mobile()` or `real_object()` returns `-1`. The command's `zone_value` is set to `0`, which causes **silent failure** during reset - mobs won't spawn but no error is logged.

```cpp
void zoneData::renumCmd() {
    for (auto& cmd : cmd_table) {
        switch (cmd.command) {
            case 'M':  // Load mobile
            case 'C':  // Load charmed
            case 'K':  // Load grouped
            case 'R':  // Load as mount
                if ((cmd.zone_value = real_mobile(cmd.arg1)) < 0) {
                    vlogf(LOG_LOW, format("Zone %s: cmd %c resolving mobile number %d") %
                                     name % cmd.command % cmd.arg1);
                    cmd.zone_value = 0;  // Silent failure!
                }
                break;

            case 'O':  // Load object (boot only)
            case 'B':  // Load object (every reset)
            case 'G':  // Give to mob
            case 'E':  // Equip on mob
            case 'I':  // Equip as prop
            case 'P':  // Put in container
                if ((cmd.zone_value = real_object(cmd.arg1)) < 0) {
                    vlogf(LOG_LOW, format("Zone %s: cmd %c resolving object number %d") %
                                     name % cmd.command % cmd.arg1);
                    cmd.zone_value = 0;  // Silent failure!
                }
                break;
        }
    }
}
```

**Symptoms of vnum validation failure:**
- Zone resets but mobs don't spawn
- Objects missing from shop/container
- Check `LOG_LOW` for "resolving mobile/object number" errors
- Verify vnums exist: `show mob <vnum>`, `show obj <vnum>`

## Phase 2: Aging and Queueing (Runtime)

### procZoneUpdate - Aging System

Scheduler process runs every `Pulse::MUDHOUR` (144 seconds) to age zones and queue resets.

**Source:** `code/code/sys/socket.cc:1697`, `code/code/sys/db.cc:2626-2725`

```cpp
bool procZoneUpdate::run(const TPulse& pulse) const {
    static int pulse_zone = 0;
    int i;

    // Phase 1: Age all zones
    for (i = 0; i < (signed int)zone_table.size(); i++) {
        if (!zone_table[i].enabled)
            continue;

        if (zone_table[i].age < zone_table[i].lifespan)
            zone_table[i].age++;

        // Queue for reset when age >= lifespan
        if (zone_table[i].age >= zone_table[i].lifespan &&
            zone_table[i].age < ZO_DEAD && zone_table[i].reset_mode) {

            // ZO_DEAD = 9999, marks zone as already queued
            zone_table[i].age = ZO_DEAD;

            vlogf(LOG_MISC, format("Resetting %s (# %d), age %d") %
                             zone_table[i].name % i % zone_table[i].age);
        }
    }

    // Phase 2: Process reset queue
    for (i = 0; i < (signed int)zone_table.size(); i++) {
        if (zone_table[i].age != ZO_DEAD)
            continue;

        // reset_mode 2 (always) resets immediately
        if (zone_table[i].reset_mode == 2) {
            zone_table[i].resetZone(resetFlagBootTime);
            continue;
        }

        // reset_mode 1 (when empty) waits for no players
        if (!zone_table[i].isEmpty())
            continue;

        zone_table[i].resetZone(resetFlagBootTime);
    }

    return false;
}
```

### Reset Modes

| Mode | Behavior | Use Case |
|------|----------|----------|
| 0 | Never reset | Static zones, builder testing |
| 1 | Reset when empty | Standard zones - no player disruption |
| 2 | Always reset | Critical zones that must reset on schedule |

### isEmpty() - Player Detection

Checks if any PCs are in the zone before mode-1 reset.

**Source:** `code/code/sys/db.cc:3586-3605`

```cpp
bool zoneData::isEmpty() const {
    for (Descriptor* i = descriptor_list; i; i = i->next) {
        // Skip non-playing states
        if (i->connected)
            continue;

        TBeing* ch = i->character;
        if (!ch)
            continue;

        // Check if character in this zone's room range
        if (ch->in_room != Room::NOWHERE &&
            (ch->in_room >= bottom * 100) && (ch->in_room <= top * 100))
            return false;
    }

    return true;
}
```

**CRITICAL:** Room number calculation assumes zone numbering scheme:
- Zone bottom = 156, top = 159
- Room range = 15600 to 15999 (bottom * 100 to top * 100)
- This pattern is convention, not enforced by the system

## Phase 3: Reset Execution (Runtime)

### resetZone() - Command Execution

Processes the `cmd_table` vector, executing each command sequentially.

**Source:** `code/code/sys/db.cc:3628-3827`

**Execution flow:**
```cpp
void zoneData::resetZone(resetFlag flag) {
    resetCom* cmd;
    TMonster* tmob = NULL;  // Last mob loaded
    TObj* tobj = NULL;      // Last object loaded
    TBeing* tbei = NULL;    // Scratch being pointer
    int random_room = 0;    // For random room placement (-99)

    for (unsigned int cmd_no = 0; cmd_no < cmd_table.size(); cmd_no++) {
        cmd = &cmd_table[cmd_no];

        // Conditional execution
        if (cmd->if_flag && !last_cmd)
            continue;

        // Execute command
        last_cmd = cmd->execute(this, &tmob, &tobj, &tbei, &random_room, flag);
    }

    // Reset age counter
    age = 0;

    // Trigger CMD_GENERIC_RESET on all zone objects
    doGenericReset();
}
```

### Conditional Execution (if_flag)

The `if_flag` field controls whether a command executes:

```
M 0 120 1 216         ; Load constable (always, if_flag=0)
E 1 300 1000 19       ; Equip sword (only if M succeeded, if_flag=1)
? 0 5 0 E             ; 5% chance (always evaluate, if_flag=0)
Y 0 7 5               ; 5% armor set (only if ? succeeded, if_flag=1)
```

**Execution rules:**
- `if_flag = 0`: Always execute
- `if_flag = 1`: Execute only if previous command succeeded (`last_cmd = true`)
- Commands return true/false to set `last_cmd` for next command

### Load-on-Death System

Commands following a mob load can be stored for execution when the mob dies.

**Source:** `code/code/sys/db.cc:3722-3780`

```cpp
// Check if we should store load-on-death commands
if (flag & resetFlagFindLoadPotential && tmob) {
    switch (cmd->command) {
        case 'G':  // Give
        case 'E':  // Equip
        case '?':  // Percent chance
        case 'Y':  // Global armor set
        case 'Z':  // Local armor set
        case 'J':  // Local set as props
            // Store command for execution on death
            tmob->loadCom.push_back(*cmd);
            continue;  // Skip normal execution
    }
}
```

**When mob dies:**
- `die()` function checks `mob->loadCom`
- Executes stored commands (spawns loot)
- Enables dynamic loot tables without cluttering ground

### Random Room Placement

Room vnum `-99` triggers random placement within a range.

**Source:** `code/code/sys/db.cc:3656-3670`

```cpp
// Set random room range
case 'A':  // A if_flag low_room high_room
    if (cmd->arg1 <= cmd->arg2)
        random_room = number(cmd->arg1, cmd->arg2);
    else
        random_room = number(cmd->arg2, cmd->arg1);
    last_cmd = true;
    break;

// Use random room on load
case 'M':  // M if_flag mob_vnum room_max room_vnum
    if (cmd->arg3 == ZONE_ROOM_RANDOM)  // -99
        load_room = random_room;
    else
        load_room = cmd->arg3;
```

**Example:**
```
A 0 100 245           ; Set random range 100-245
M 0 115 1 -99         ; Load mob in random room from range
```

## Phase 4: Cleanup (Post-Reset)

### doGenericReset() - Special Procedure Notification

After all commands execute, notifies objects to reset their state.

**Source:** `code/code/sys/db.cc:3607-3626`

```cpp
void zoneData::doGenericReset() {
    // Iterate all rooms in zone
    for (int counter = bottom * 100; counter <= top * 100; counter++) {
        TRoom* rp = real_roomp(counter);
        if (!rp)
            continue;

        // Send CMD_GENERIC_RESET to all objects in room
        for (StuffIter it = rp->stuff.begin(); it != rp->stuff.end(); ++it) {
            TObj* obj = dynamic_cast<TObj*>(*it);
            if (!obj)
                continue;

            obj->checkSpec(NULL, CMD_GENERIC_RESET, "", NULL);
        }
    }
}
```

**Purpose:**
- Resets spec proc state (counters, timers, etc.)
- Clears temporary flags
- Restores default configurations

## Database Schema

### zone Table

```sql
CREATE TABLE zone (
    zone_nr INT PRIMARY KEY,       -- Sequential runtime index
    zone_name VARCHAR(80),          -- Zone name
    zone_enabled TINYINT,           -- Enabled flag
    bottom INT,                     -- Lowest room vnum
    top INT,                        -- Highest room vnum
    reset_mode TINYINT,             -- 0=never, 1=empty, 2=always
    lifespan INT,                   -- Minutes between resets
    util_flag TINYINT               -- Boot tracking (0/1)
);
```

**Source:** `_Setup-data/sql_tables/sneezy/zone.sql`

### util_flag Pattern

The `util_flag` field tracks which zones are active:

1. **Before boot**: `UPDATE zone SET util_flag = 0` (all zones marked inactive)
2. **During boot**: Each successfully loaded zone gets `util_flag = 1`
3. **After boot**: `DELETE FROM zone WHERE util_flag = 0` (orphans removed)

**Why this works:**
- Zones removed from filesystem but still in database get `util_flag = 0`
- Final DELETE cleans up stale database entries
- No manual database maintenance required

## Builder Workflow

### Creating a New Zone

1. **Pick vnum range:**
   ```
   zonefile new 5 Cizra - Temple of Stupidity
   Success! new zone: Cizra - Temple of Stupidity with vnums from 45660 to 45664
   ```

2. **Assign to builder:**
   ```
   @set blocka cizra 45660 45664
   save
   ```

3. **Create rooms in immortal database:**
   ```
   rload 1                  # Load from immortal DB
   goto 45664
   redit                    # Describe and connect rooms
   rsave 1                  # Save to immortal DB block 1
   ```

4. **Create mobs:**
   ```
   show mob rabbit
   load mob 44784
   medit mod rabbit         # Change to Lagomorph
   medit save lagomorph 45664
   ```

5. **Create objects:**
   ```
   oedit create
   oedit mod hairball       # Change to carrot
   oedit save carrot 45664
   ```

6. **Edit zonefile:**
   - File: `lib/zonefiles/45660`
   - Add reset commands (see Zonefile Format Reference below)

7. **Publish to production:**
   ```
   low mvroom Cizra 1 45660-45664
   low mvmob Cizra 45664
   low mvobj Cizra 45664
   ```

8. **Enable zone:**
   - Edit `lib/zonefiles/45660` header
   - Change `enabled` from 0 to 1

9. **Activate:**
   ```
   boot zone 45660
   ```
   Or reboot server.

**See also:** [Admin Operations](admin-operations.md) for complete LOW command reference.

## Common Issues

### Zone Won't Reset

**Symptoms:** Age counter keeps increasing, zone never resets

**Causes:**
1. `reset_mode = 0` (never reset)
2. `enabled = 0` in zonefile
3. Mode 1 and players in zone (check with `isEmpty()`)
4. Zone already queued (`age = 9999`)

**Debug:**
```
stat zone <zone_nr>
show zones
```

### Mobs Not Spawning

**Symptoms:** Zone resets but mobs don't appear

**Causes:**
1. **Invalid vnum** - `renumCmd()` set `zone_value = 0`
   - Check `LOG_LOW`: "resolving mobile number X"
   - Verify: `show mob <vnum>`
2. **max_exist reached** - Global mob limit
   - Check: `stat mob <vnum>` → "Cur exist"
3. **Conditional execution** - `if_flag = 1` but previous failed
4. **Wrong room vnum** - Mob loaded in non-existent room

**Fix:**
```
# Check logs
grep "resolving mobile" logs/sneezy.log

# Verify vnum exists
show mob <vnum>

# Check command dependencies
less lib/zonefiles/<zone_vnum>
```

### Objects Missing

**Symptoms:** Objects not appearing in shops/containers

**Same causes as mobs**, plus:
1. Container at max capacity
2. Shop inventory full (`MAX_SHOP_INVENTORY = 2500`)
3. Object loaded then decayed (check decay timers)

### Orphaned Database Entries

**Symptoms:** Database has zones not in filesystem

**Cause:** Zone file deleted but database not cleaned

**Prevention:** The `util_flag` system handles this automatically on next boot.

**Manual cleanup:**
```sql
-- Find orphaned zones
SELECT * FROM zone WHERE util_flag = 0;

-- Delete (or wait for next boot)
DELETE FROM zone WHERE util_flag = 0;
```

## Performance Considerations

### Zone Count Impact

- `procZoneUpdate` iterates all zones every 144 seconds
- `isEmpty()` iterates all descriptors for mode-1 resets
- `doGenericReset()` iterates all rooms/objects in zone

**High zone count (500+) can cause lag spikes.**

### Reset Complexity

- Complex zonefiles (1000+ commands) take time to process
- Mob spawning triggers spec proc initialization
- Object loading may trigger database queries (rent system)

**Distribute resets:** Vary `lifespan` to avoid simultaneous resets.

### Database Synchronization

- bootOneZone() performs UPDATE + INSERT for each zone
- Boot time grows linearly with zone count
- No transactions - crash during boot leaves partial state

## Key Constants

```cpp
const int ZO_DEAD = 9999;                  // Zone queued for reset
const int ZONE_ROOM_RANDOM = -99;          // Random room placement
const int MAX_SHOP_INVENTORY = 2500;       // Shop item limit
```

**Source:** `code/code/misc/low.h`, `code/code/misc/shop.h`

## resetFlag Enum

```cpp
enum resetFlag {
    resetFlagBootTime = (1 << 0),           // Booting, load O commands
    resetFlagFindLoadPotential = (1 << 1),  // Store load-on-death commands
};
```

**Source:** `code/code/sys/db.h` (lines 237-241)

**Usage:**
- `resetZone(resetFlagBootTime)`: Normal reset, execute O commands
- `resetZone(resetFlagBootTime | resetFlagFindLoadPotential)`: Store equipment for load-on-death
- During boot: Both flags set
- During runtime reset: Typically just `resetFlagBootTime`

## Zone Display Commands

### show zones

Lists all zones with status.

**Source:** `code/code/misc/cmd_show.cc:1018-1061`

**Output:**
```
Zone [nr] Name (Rooms) Age Reset Enabled
   0 [  0] Immortal Staging Area (0-99) 0 2 Y
   1 [  1] Newbie Tutorial (100-199) 15 1 Y
   2 [  2] Grimhaven City (200-599) 45 1 Y
```

**Fields:**
- Zone: Sequential display number
- [nr]: zone_nr runtime index
- Name: zone name
- (Rooms): bottom-top room range
- Age: Minutes since last reset (or 9999 if queued)
- Reset: reset_mode (0/1/2)
- Enabled: Y/N

### stat zone <zone_nr>

Detailed zone statistics.

**Source:** `code/code/misc/cmd_stat.cc:1653-1718`

**Output:**
```
Zone data for zone: Grimhaven City
  Zone number: 2
  Bottom: 200, Top: 599
  Lifespan: 45, Age: 15
  Reset mode: 1 (when empty)
  Enabled: Yes
  Command count: 156
  Rooms: 400
  Mobs loaded: 45
  Objects loaded: 123
```

## Key Source Files

| File | Contents | Lines |
|------|----------|-------|
| `code/code/sys/db.cc` | bootZones(), bootOneZone(), bootZone() | 1523-1696 |
| `code/code/sys/db.cc` | resetZone(), doGenericReset() | 3607-3827 |
| `code/code/sys/db.cc` | renumCmd(), isEmpty() | 3468-3605 |
| `code/code/sys/db.cc` | procZoneUpdate scheduler | 2626-2725 |
| `code/code/sys/db.h` | zoneData, resetCom classes | 100-234 |
| `code/code/misc/cmd_show.cc` | show zones display | 1018-1061 |
| `code/code/misc/cmd_stat.cc` | stat zone display | 1653-1718 |
| `code/code/misc/low.h` | ZO_DEAD constant | - |

---

## Zonefile Format Reference

This section provides a complete reference for the zonefile format and reset commands.

### File Location

Zonefiles are stored in `lib/zonefiles/{zone_number}` where the filename matches the zone's starting vnum.

### Header Format

```
#zone_number
Zone Name~
top_room lifespan reset_mode enabled
```

| Field | Description |
|-------|-------------|
| zone_number | Starting vnum (must match filename) |
| Zone Name~ | Name terminated by tilde |
| top_room | Highest room vnum in zone |
| lifespan | Minutes between reset attempts |
| reset_mode | 0=never, 1=when empty, 2=always |
| enabled | 1=active, 0=disabled on production |

### Reset Command Summary

Commands are processed sequentially. Lines starting with `*` are comments. The `if_flag` (0=always, 1=if previous succeeded) controls conditional execution.

#### Mobile Commands

| Cmd | Format | Description |
|-----|--------|-------------|
| M | `M if mob_vnum room_max room_vnum` | Load mob at location |
| C | `C 1 mob_vnum room_max room_vnum` | Load charmed to previous mob |
| K | `K 1 mob_vnum room_max room_vnum` | Load grouped with previous mob |
| R | `R 1 mob_vnum room_max room_vnum` | Load as mount for previous mob |

#### Object Commands

| Cmd | Format | Description |
|-----|--------|-------------|
| O | `O if obj_vnum max room_vnum` | Load on ground (boot only) |
| B | `B if obj_vnum max room_vnum` | Load on ground (every reset) |
| G | `G 1 obj_vnum max` | Give to last mob's inventory |
| E | `E 1 obj_vnum max slot` | Equip on last mob |
| P | `P 1 obj_vnum max container_vnum` | Place in container |
| I | `I 1 obj_vnum max slot` | Equip as prop (load-on-death) |

#### Utility Commands

| Cmd | Format | Description |
|-----|--------|-------------|
| ? | `? if percent 0 CMD` | Percent chance for next command |
| A | `A 0 low_room high_room` | Set random room range for -99 loads |
| D | `D if room_vnum dir state` | Set door (state: 0=open, 1=closed, 2=locked) |
| T | `T if room_vnum dir trap_type damage` | Trap on door |
| T | `T 1 trap_type damage` | Trap on last object |
| V | `V 1 value_index new_value` | Change object value |
| H | `H 1 hate_type 0` | Set hate on last mob |
| F | `F 1 fear_type 0` | Set fear on last mob |
| L | `L 1 min_level max_level 0 1` | Random loot on mob death |
| S | `S` | End of zone (required) |

#### Armor Set Commands

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

### Special Features

#### Random Room Loading

```
A 0 100 245           ; Set random range
M 0 115 1 -99         ; Load mob in random room from range
```

#### Conditional Loading

```
M 0 120 1 216         ; Load constable
? 0 5 0 E             ; 5% chance
E 1 300 1000 19       ; Equip longsword
Y 0 7 5               ; 5% Hard Leather set
```

#### Load-on-Death

When enabled, equipment commands (G, E, ?, Y, Z) following a mob are stored and executed when the mob dies. The `L` command generates random level-appropriate loot.

#### max_exist Limits

Each mob/object has a global `max_exist` database limit. Loads are silently skipped when at capacity.

### Complete Zonefile Example

```
#15200
Merc - Pantathian Extension~
15249 45 2 1

D 0 15239 0 1                 ; Close shaman door
T 0 15239 0 6 4               ; Fire trap on door

O 0 33 1 15240                ; Chest in shaman's room
V 1 2 15247                   ; Change key vnum
? 0 25 0 L
L 1 13 24 0 1                 ; Random loot

M 0 15212 1 15200             ; Guard
? 0 5 0 E
E 1 6112 400 4                ; Rusty shirt
Y 0 41 2                      ; Ringmail set

S
```

---

## Related Documentation

- [Admin Operations](admin-operations.md) - LOW commands for publishing zones
- [Database Queries](database-queries.md) - Database interaction patterns
- [Scheduler and Pulses](scheduler-pulses.md) - procZoneUpdate timing
