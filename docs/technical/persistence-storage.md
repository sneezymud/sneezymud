---
title: Persistence and Storage
description: Hybrid persistence system using binary charFile format and MariaDB databases for character/item storage, including SQL patterns and security
keywords: [TDatabase, charFile, save_char, load_char, ItemSave, ItemLoad, DB_SNEEZY, DB_IMMORTAL, binary format, rent table, mysql_real_escape_string, %s format, %r danger, SQL injection, money synchronization]
category: Critical Systems

last_updated: 2026-01-29
source_files: [code/code/misc/player_data.cc, code/code/misc/rent.cc, code/code/sys/database.cc, code/code/misc/charfile.h]
related: [persistence-storage.md, configuration-reference.md, economy-system.md]
---

# Persistence and Storage

This document describes SneezyMUD's hybrid persistence system, combining binary file storage with MariaDB databases for character data, item inventories, and game state.

## Overview

SneezyMUD uses a hybrid storage model:

- **Binary files** (`charFile`) store character data in `lib/mutable/player/`
- **MariaDB database** stores item inventories, shop contents, and supplementary player data
- **Binary files** store player rent files and corpse contents

The database is authoritative for certain data (like talens/money), while binary files remain the primary store for character stats and attributes.

## Database Layer

### TDatabase Class

Database interaction in SneezyMUD uses the `TDatabase` class with printf-style format specifiers.

#### Databases

| Database   | Enum Constant | Purpose                            |
| ---------- | ------------- | ---------------------------------- |
| `sneezy`   | `DB_SNEEZY`   | Game data (objects, mobs, players) |
| `immortal` | `DB_IMMORTAL` | Immortal-specific data             |

#### Basic Usage

```cpp
#include "database.h"

TDatabase db(DB_SNEEZY);
db.query("select vnum, short_desc from obj where vnum=%i", 10000);

while (db.fetchRow()) {
    sstring desc = db["short_desc"];  // Access by column name
    sstring vnum = db[0];             // Access by index
}
```

**Key principles:**
- Always use local instances (never `new`). The destructor handles cleanup.
- Functions return safe values on error; no crash risk from unchecked errors.
- Non-SELECT queries do not clear previous SELECT results.

#### Format Specifiers and SQL Injection Prevention

| Specifier | Type     | Escaping                         | Safety        |
| --------- | -------- | -------------------------------- | ------------- |
| `%s`      | `char*`  | Yes (`mysql_real_escape_string`) | **SAFE**      |
| `%i`      | `int`    | N/A (numeric)                    | **SAFE**      |
| `%f`      | `double` | N/A (numeric)                    | **SAFE**      |
| `%r`      | `char*`  | **NO ESCAPING**                  | **DANGEROUS** |
| `%%`      | literal  | Outputs a single `%`             | **SAFE**      |

#### CRITICAL: The %r Specifier

The `%r` specifier inserts raw, unescaped strings directly into queries. This is a **SQL injection vulnerability** if used with user input.

```cpp
// DANGEROUS - SQL injection if 'input' comes from user
db.query("select * from obj where owner in (%r)", userInput);

// SAFE - %r only with server-generated content
sstring owners = "'grimhaven', 'brightmoon'";  // Built by server
db.query("select * from obj where owner in (%r)", owners.c_str());
```

**Rules for %r:**
- **NEVER** use with user input
- **ONLY** use for server-generated SQL fragments
- Always prefer `%s` for any data that originates from players

#### LIKE Queries

Use `%%` to produce a literal `%`:

```cpp
db.query("select * from obj where name like '%%%s%%'", "blade");
// Produces: select * from obj where name like '%blade%'
```

### API Reference

| Method          | Returns   | Description                                      |
| --------------- | --------- | ------------------------------------------------ |
| `query(fmt, ...)`| `bool`   | Execute query with printf-style args             |
| `fetchRow()`    | `bool`    | Advance to next row; false when exhausted        |
| `operator[col]` | `sstring` | Get column by name or index                      |
| `isResults()`   | `bool`    | True if query returned rows                      |
| `rowCount()`    | `long`    | Affected/retrieved row count (-1 on error)       |
| `lastInsertId()`| `long`    | Auto-increment ID from last INSERT               |

Constructor: `TDatabase(dbTypeT db, bool log = false)`

### TTransaction Class

RAII transaction handling - constructor calls `BEGIN`, destructor calls `COMMIT`:

```cpp
{
    TTransaction db(DB_SNEEZY);
    db.query("delete from mytable where id=%i", id);
    db.query("insert into mytable values (%i, '%s')", id, name.c_str());
}  // Commits here
```

### Object and Mobile Caches

Object and mobile template data is cached at boot for performance.

```cpp
// TObjectCache - caches 'obj' table
cached_object* obj = obj_cache[realNumber];
if (obj) {
    sstring type = obj->s["type"];
}

// TMobileCache - caches 'mob' table plus extras/immunities
cached_object* mob = mob_cache[realNumber];
if (mob) {
    sstring name = mob->s["name"];
}
```

At boot time, object and mobile definitions are preloaded into memory caches for fast instantiation:

```cpp
class TObjectCache {
    std::map<int, cached_object*> cache;
    void preload();  // Loads all objects from DB at startup
};

class TMobileCache {
    std::map<int, cached_object*> cache;
    std::map<int, std::vector<cached_mob_extra*>> extra;
    std::map<int, std::vector<cached_mob_imm*>> imm;
    void preload();  // Loads all mobiles from DB at startup
};
```

These caches (`obj_cache`, `mob_cache`) are populated during `bootDb()` and provide O(1) lookup for creating new instances.

### Common Query Patterns

```cpp
// Check existence
db.query("select 1 from player where name='%s'", name.c_str());
if (db.fetchRow()) { /* exists */ }

// Iterate results
db.query("select vnum, short_desc from obj where type=%i", ITEM_WEAPON);
while (db.fetchRow()) {
    int vnum = convertTo<int>(db["vnum"]);
}

// Insert and get ID
db.query("insert into mytable (name) values ('%s')", name.c_str());
long newId = db.lastInsertId();

// Count rows
db.query("select count(*) as cnt from obj");
if (db.fetchRow()) {
    int count = convertTo<int>(db["cnt"]);
}
```

### Error Handling

Errors are logged via `vlogf(LOG_DB, ...)`. Return values on error:
- `query()`: `false`
- `fetchRow()`: `false`
- `operator[]`: empty string
- `rowCount()`: `-1`

## Character Binary Format

### Overview

Player data is stored as a raw binary dump of the `charFile` struct. The file is written and read using `fwrite`/`fread` with `sizeof(charFile)`, meaning the exact memory layout of the struct is persisted to disk.

**Location:** `lib/mutable/player/{first_letter}/{charname}`

Example: The player "Gandalf" would be saved to `lib/mutable/player/g/gandalf`

### Critical Warning: Binary Format Constraints

The `charFile` struct in `code/code/misc/charfile.h` contains this comment:

> Do not remove any variables from this class, even if it is unused -- you'll throw the savefile format out of alignment. Can't wait until we migrate to SQL

This warning is serious. Because the file is a raw binary dump:

- **DO NOT** add new fields to `charFile`
- **DO NOT** remove fields from `charFile`
- **DO NOT** reorder fields in `charFile`
- **DO NOT** change the size of any field
- **DO NOT** change the size of any array constant (e.g., `MAX_AFFECT`, `MAX_SAVED_CLASSES`)

Any of these changes will corrupt ALL existing player saves. The byte offsets will no longer align, and loading will read garbage data into the wrong fields.

### Struct Definition

The `charFile` class is defined in `code/code/misc/charfile.h`. Key sections include:

| Section | Fields | Purpose |
|---------|--------|---------|
| Basic info | `sex`, `level[]`, `race`, `Class` | Character identity |
| Physical | `weight`, `height`, `body_flags[]`, `body_health[]` | Body state and limbs |
| Strings | `title[80]`, `name[20]`, `pwd[11]`, `description[500]` | Fixed-size char arrays |
| Time | `birth`, `last_logon`, `played` | Play time tracking |
| Points | `mana`, `hit`, `move`, `money`, `exp`, etc. | Current stats |
| Affects | `affected[MAX_AFFECT]` | Active spell effects |
| Faction | `f_percent`, `f_percx[]`, `f_type`, `f_actions` | Faction standing |
| Skills | `disc_learning[]`, `skills[]` | Learned abilities |
| Obsolete | `obsolete_prompt_colors[200]`, `obsolete_p_type` | Retained for alignment |
| Reserved | `temp1` through `temp4`, `unused` | Reserved for future use |

Note the `obsolete_*` and `unused` fields: these exist solely to maintain binary compatibility. They cannot be removed.

### Array Size Constants

These constants define array sizes in `charFile`. Changing them breaks saves:

| Constant | Value | Used For |
|----------|-------|----------|
| `MAX_SAVED_CLASSES` | 12 | `level[]`, `doneBasic[]` |
| `MAX_HUMAN_WEAR` | 22 | `body_flags[]`, `body_health[]` |
| `MAX_AFFECT` | 25 | `affected[]` |
| `ABS_MAX_FACTION` | 6 | `f_percx[]` |
| `MAX_SAVED_DISCS` | 90 | `disc_learning[]` |
| `ABSOLUTE_MAX_SKILL` | 900 | `skills[]` |

### File Operations

The binary file operations are straightforward:

```cpp
// Writing
fwrite(char_element, sizeof(charFile), 1, fl);

// Reading
fread(char_element, sizeof(charFile), 1, fl);
```

No serialization, no versioning, no endianness handling. This simplicity is why the format cannot be changed.

### Hybrid Storage Model

Player data uses a hybrid storage model: binary file plus database.

#### What Lives in the Binary File

Most character data, including:
- Stats, skills, disciplines
- Level and class information
- Body state (limb health, flags)
- Active spell effects
- Faction data
- Play time and timestamps

#### What Lives in the Database

The `player` table (`_Setup-data/sql_tables/sneezy/player.sql`) stores:
- `talens` (money)
- `title`
- `account_id`
- `guild_id`, `guildrank`
- `load_room`
- `last_logon`
- `nutrition`

Additional tables store aliases, toggles, wizard powers, trophies, and other data that has been migrated out of the binary format.

### Money Synchronization

Money (`talens`) exists in BOTH the binary file AND the database. On load, the database value wins:

```cpp
// From load_char() in player_data.cc
db->query("select talens from player where lower(name)=lower('%s') and talens is not null", name.c_str());
if (db->fetchRow()) {
    char_element->money = convertTo<int>((*db)["talens"]);  // DB wins
} else {
    // No DB record - seed from file
    db->query("update player set talens=%i where lower(name)=lower('%s')",
        char_element->money, name.c_str());
}
```

On save, `raw_save_char()` writes the binary file then updates the database:

```cpp
db.query("update player p, account a set p.talens=%i, p.account_id=a.account_id "
    "where lower(p.name)=lower('%s') and a.name='%s'",
    char_element->money, name, char_element->aname);
```

This dual storage is a transitional state. The `money` field in `charFile` cannot be removed due to binary compatibility, but the database is authoritative.

### Key Functions

| Function | Location | Purpose |
|----------|----------|---------|
| `load_char()` | `player_data.cc:333` | Read binary file, sync money from DB |
| `raw_save_char()` | `player_data.cc:307` | Write binary file, sync money to DB |
| `TPerson::storeToSt()` | `player_data.cc:366` | Copy runtime data to `charFile` |
| `TPerson::loadFromSt()` | `player_data.cc:600` | Copy `charFile` to runtime data |

### Adding New Persistent Player Data

If you need to store new persistent player data:

1. **DO NOT** modify `charFile`
2. **DO** add a new database table or column
3. **DO** add load/save functions in `player_data.cc` or a relevant file
4. Follow the pattern of existing database-backed data like aliases, toggles, or trophies

Example tables that store player data outside the binary file:
- `alias` - Player command aliases
- `playerprompt` - Prompt configuration
- `playertoggle` - Toggle settings
- `trophy` / `trophyplayer` - Trophy system

### Future Direction

The comment in `charfile.h` expresses the desire to migrate fully to SQL. Until then:

- Treat the binary format as frozen
- Add all new persistent data to the database
- Accept that some fields (`obsolete_*`, `unused`, `temp*`) waste space but cannot be removed

## Rent and Item Persistence

Items are persisted through two parallel systems: file-based and database-based.

### File System Layout

```
lib/mutable/
  player/
    {a-z}/              # Subdirectories by first letter of name
      {charname}        # Binary charFile
      {charname}.strings
      {charname}.toggle
      {charname}.career
  rent/
    {a-z}/
      {charname}        # Binary rent file (player inventory)
      {charname}.fol    # Follower data
      {charname}.fr     # Follower rent
  corpses/
    {charname}          # Binary corpse file with contents
```

### File-Based Storage (ItemSave/ItemLoad)

Used for player rent files and corpses. Items are written as binary `rentObject` structs:

```cpp
class rentObject {
    int item_number;      // Object vnum
    int value[4];         // Object-specific values
    unsigned int extra_flags;
    float weight;
    // ... struct points, material, cost, etc.
    rentObjAffData affected[MAX_OBJ_AFFECT];
};
```

Strung (customized) items have their strings appended after the struct.

### Database Storage (ItemSaveDB/ItemLoadDB)

Used for shops, rooms, and mail. Items are stored across three tables.

### Database Schema

#### rent Table

Primary item storage:

```sql
CREATE TABLE rent (
    rent_id INT PRIMARY KEY AUTO_INCREMENT,
    owner_type ENUM('player', 'shop', 'room', 'mail'),
    owner INT,           -- Player ID, shop number, room number, or mail ID
    slot INT,            -- Equipment slot (-1 for inventory)
    vnum INT,            -- Object virtual number
    container INT,       -- Parent rent_id if nested (-1 if none)
    val0-val3 INT,       -- Object-specific values
    extra_flags INT,
    weight DOUBLE,
    bitvector INT,
    decay INT,
    cur_struct INT,
    max_struct INT,
    material INT,
    volume INT,
    price INT,
    depreciation INT
);
```

#### rent_obj_aff Table

Object affects/enchantments:

```sql
CREATE TABLE rent_obj_aff (
    rent_id INT,         -- FK to rent.rent_id
    type INT,            -- Spell/affect type
    level INT,
    duration INT,
    renew INT,
    modifier INT,
    location INT,        -- Apply location
    modifier2 INT,
    bitvector INT
);
```

#### rent_strung Table

Custom descriptions for strung items:

```sql
CREATE TABLE rent_strung (
    rent_id INT,         -- FK to rent.rent_id
    name VARCHAR(127),
    short_desc VARCHAR(127),
    long_desc VARCHAR(255),
    action_desc VARCHAR(255)
);
```

### owner_type Values

| Type | Owner Field Contains | Usage |
|------|---------------------|-------|
| `player` | Player ID | Player inventory (DB-based) |
| `shop` | Shop number | Shopkeeper inventory |
| `room` | Room number | Persistent room storage |
| `mail` | Mail ID | Mail attachments |

### Corpse Persistence

Player corpses are saved to prevent item loss on crashes:

```cpp
void TPCorpse::saveCorpseToFile() {
    sprintf(buf, "mutable/corpses/%s", fileName.c_str());
    // Write header with corpse count
    // Write corpse as TObj with contents
}
```

Corpses are automatically saved when:
- Player dies (corpse created)
- Items are added/removed from corpse
- Mobs loot from corpse

A global linked list (`pc_corpse_list`) tracks all player corpses in memory.

### Shop Item Persistence

Shops use database storage exclusively:

```cpp
// Save a shop item
int TMonster::saveItem(int shop_nr, TObj* obj, int container) {
    ItemSaveDB is("shop", shop_nr);
    return is.raw_write_item(obj, NORMAL_SLOT, container);
}

// Load a shop item
TObj* TMonster::loadItem(int shop_nr, int rent_id) {
    ItemLoadDB il("shop", shop_nr);
    return il.raw_read_item(rent_id, slot);
}

// Delete a shop item
void TMonster::deleteItem(int shop_nr, int rent_id) {
    db.query("delete from rent where rent_id=%i", rent_id);
    db.query("delete from rent_obj_aff where rent_id=%i", rent_id);
    db.query("delete from rent_strung where rent_id=%i", rent_id);
}
```

### Rent File Version History

The rent file format has evolved over time (current version: 10):

| Version | Change |
|---------|--------|
| 0 | Initial 3.x format |
| 1 | Changed weight from int to float |
| 2 | Added depreciation field |
| 3-7 | Various price/structure adjustments |
| 8 | Shuffled weapon 4vals |
| 9 | Material type consolidation |
| 10 | Current version |

Version handling in `raw_read_item()` provides backward compatibility for older rent files.

## Integration Points

### Corruption Handling

Corrupted files are moved to `corrupt/` subdirectories rather than deleted:

```cpp
void handleCorrupted(const char* name, char* account) {
    // Move player file, strings, toggles, career
    // Move rent file and followers
    // Move corpses
    // Remove from account
}
```

This preserves data for potential recovery rather than destroying it.

### Money Synchronization Between Binary and Database

As described in the Character Binary Format section, money exists in both locations with the database being authoritative:

1. On load: Database value overwrites binary file value
2. On save: Both locations are updated
3. If no database record exists: Binary file value is seeded into database

This dual storage is necessary because the binary format cannot be changed, but allows the database to be the single source of truth for money-related queries and operations.

## Security

### SQL Injection Prevention

The `TDatabase` class provides automatic SQL injection prevention through format specifiers:

- **Use `%s` for ALL user input** - automatically escapes strings using `mysql_real_escape_string`
- **Use `%i` and `%f` for numeric values** - no escaping needed
- **NEVER use `%r` with user input** - bypasses all escaping

### The %r Danger

The `%r` format specifier is the most dangerous feature in the database layer. It completely bypasses SQL escaping and inserts raw strings directly into queries.

**Acceptable use cases:**
- Building dynamic IN clauses from server-generated lists
- Inserting SQL fragments that are constructed entirely by server code

**Unacceptable use cases:**
- ANY string that originates from user input
- ANY string derived from player names, descriptions, or commands
- ANY string from external sources

Example of the risk:

```cpp
// DANGEROUS - allows arbitrary SQL execution
sstring playerInput = "1; DROP TABLE player; --";
db.query("select * from obj where vnum=%r", playerInput.c_str());
// Becomes: select * from obj where vnum=1; DROP TABLE player; --

// SAFE - using %i validates numeric input
db.query("select * from obj where vnum=%i", convertTo<int>(playerInput));
```

### Best Practices

1. **Default to `%s`** - when in doubt, use `%s` for string parameters
2. **Validate before `%r`** - if you must use `%r`, ensure the string is entirely server-controlled
3. **Use transactions** - wrap related queries in `TTransaction` for atomicity
4. **Check return values** - always verify `query()` returned `true` before using results
5. **Log suspicious patterns** - the `LOG_DB` category can help identify injection attempts

## Related Files

| File | Purpose |
|------|---------|
| `code/code/misc/rent.cc` | Core rent/persistence implementation |
| `code/code/misc/rent.h` | Rent structs and class definitions |
| `code/code/misc/player_data.cc` | Character save/load |
| `code/code/misc/charfile.h` | Binary character file struct |
| `code/code/sys/database.cc` | Database access and caching |
| `code/code/sys/database.h` | Database class definitions |
| `code/code/sys/db.cc` | Cache implementation |
| `_Setup-data/sql_tables/sneezy/rent*.sql` | Database schema |
| `_Setup-data/sql_tables/sneezy/player.sql` | Player table schema |
