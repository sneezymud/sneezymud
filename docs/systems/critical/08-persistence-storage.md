---
title: Persistence and Storage
description: Hybrid persistence using binary charFile format and MariaDB for character/item storage
category: critical
keywords: [SQL injection, database safety, binary file format, persistence]
primary_symbols:
  functions: [load_char, raw_save_char, storeToSt, loadFromSt, handleCorrupted]
  classes: [TDatabase, TTransaction, TObjectCache, TMobileCache, charFile, ItemSave, ItemLoad, ItemSaveDB, ItemLoadDB]
  enums: [DB_SNEEZY, DB_IMMORTAL, MAX_SAVED_CLASSES, MAX_HUMAN_WEAR, MAX_AFFECT, ABS_MAX_FACTION, MAX_SAVED_DISCS, ABSOLUTE_MAX_SKILL]
---

# Persistence and Storage

## Overview

SneezyMUD uses hybrid storage: binary files for character data (stats, skills, affects) and MariaDB for items, shops, and supplementary player data. The database is authoritative for money; binary files cannot be modified due to 30+ years of accumulated saves.

## Patterns

### Database Access

- Always use `%s` for user input (auto-escapes via `mysql_real_escape_string`)
- Use `%i`/`%f` for numeric values
- Never use `%r` with user input (bypasses all escaping)
- Always use local `TDatabase` instances (never `new`); destructor handles cleanup
- Wrap related queries in `TTransaction` for atomicity (destructor calls `COMMIT`, ensuring transactions complete even if exceptions occur)

### Binary File Safety

- Never add, remove, or reorder fields in `charFile`
- Never change array constant sizes (`MAX_AFFECT`, `MAX_SAVED_CLASSES`, etc.)
- Never remove `obsolete_*` or `unused` fields (exist for alignment)
- Add all new persistent data to database tables

### Money Handling

- Always treat database as authoritative for money
- On load: database value overwrites binary file value
- On save: update both locations
- If no database record: seed from binary file value

### Corpse and Rent Files

- Always save corpse when items are added/removed
- Move corrupted files to `corrupt/` subdirectory via `handleCorrupted()` (never delete)
- Track player corpses via `pc_corpse_list` global

### Container Hierarchy

- Use the `container` field in the `rent` table to link nested items to their parent's `rent_id`
- Top-level items have `container = -1`
- Recursive save/load reconstructs containment by following `rent_id` references

## Reference

### Database Connections

| Database | Enum | Purpose |
|----------|------|---------|
| sneezy | `DB_SNEEZY` | Game data (objects, mobs, players) |
| immortal | `DB_IMMORTAL` | Immortal-specific data |

### Format Specifiers

| Specifier | Type | Escaping | Safety |
|-----------|------|----------|--------|
| `%s` | `char*` | `mysql_real_escape_string` | Safe |
| `%i` | `int` | N/A (numeric) | Safe |
| `%f` | `double` | N/A (numeric) | Safe |
| `%r` | `char*` | None | Dangerous |
| `%%` | literal | Outputs `%` | Safe |

### TDatabase API

| Method | Returns | Description |
|--------|---------|-------------|
| `query(fmt, ...)` | `bool` | Execute query with printf-style args |
| `fetchRow()` | `bool` | Advance to next row |
| `operator[col]` | `sstring` | Get column by name or index (empty string on error) |
| `isResults()` | `bool` | True if query returned rows |
| `rowCount()` | `long` | Affected/retrieved row count (-1 on error) |
| `lastInsertId()` | `long` | Auto-increment ID from last INSERT |

### charFile Array Constants

| Constant | Value | Used For |
|----------|-------|----------|
| `MAX_SAVED_CLASSES` | 11 | `level[]`, `doneBasic[]` |
| `MAX_HUMAN_WEAR` | 20 | `body_flags[]`, `body_health[]` |
| `MAX_AFFECT` | 25 | `affected[]` |
| `ABS_MAX_FACTION` | 6 | `f_percx[]` |
| `MAX_SAVED_DISCS` | 90 | `disc_learning[]` |
| `ABSOLUTE_MAX_SKILL` | 900 | `skills[]` |

### Database Tables

| Table | Purpose |
|-------|---------|
| `rent` | Primary item storage (vnum, values, flags) |
| `rent_obj_aff` | Object affects/enchantments |
| `rent_strung` | Custom descriptions for strung items |
| `player` | Money, guild, account, load room |
| `alias` | Player command aliases |
| `playerprompt` | Prompt configuration |
| `playertoggle` | Toggle settings |

### rent Table Fields

| Field | Description |
|-------|-------------|
| `rent_id` | Auto-increment primary key |
| `owner_type` | Enum: `player`, `shop`, `room`, `mail` |
| `owner` | Player ID, shop number, room number, or mail ID |
| `slot` | Equipment slot or -1 for inventory |
| `vnum` | Object virtual number |
| `container` | Parent item's `rent_id`, or -1 for top-level |
| `val0`-`val3` | Object-specific values (charges, capacity, weapon stats) |
| `extra_flags`, `bitvector` | Object flags |
| `material`, `volume`, `weight` | Physical properties |
| `price`, `depreciation` | Economic values |
| `cur_struct`, `max_struct` | Durability |
| `decay` | Time-based deterioration |

### rent_obj_aff Table Fields

| Field | Description |
|-------|-------------|
| `rent_id` | Foreign key to `rent.rent_id` |
| `type` | Spell or affect type |
| `level` | Caster level or effect strength |
| `duration`, `renew` | Duration and auto-refresh |
| `modifier`, `modifier2` | Numeric bonuses/penalties |
| `location` | Target attribute |
| `bitvector` | Special effect flags |

### rent_strung Table Fields

| Field | Description |
|-------|-------------|
| `rent_id` | Foreign key to `rent.rent_id` |
| `name` | Custom item name |
| `short_desc` | Brief description (inventory/equipment) |
| `long_desc` | Ground description |
| `action_desc` | Use/activation text |

### rent.owner_type Values

| Type | Owner Field | Usage |
|------|-------------|-------|
| `player` | Player ID | Player inventory (DB-based) |
| `shop` | Shop number | Shopkeeper inventory |
| `room` | Room number | Persistent room storage |
| `mail` | Mail ID | Mail attachments |

### Rent File Versions

| Version | Change |
|---------|--------|
| 0 | Initial 3.x format |
| 1 | Weight: int to float |
| 2 | Added depreciation |
| 8 | Weapon 4vals shuffle |
| 9 | Material consolidation |
| 10 | Current |

### File System Layout

| Path | Contents |
|------|----------|
| `lib/mutable/player/{a-z}/{name}` | Binary charFile |
| `lib/mutable/player/{a-z}/{name}.strings` | String data |
| `lib/mutable/player/{a-z}/{name}.toggle` | Toggle state |
| `lib/mutable/player/{a-z}/{name}.career` | Career data |
| `lib/mutable/rent/{a-z}/{name}` | Binary rent file |
| `lib/mutable/rent/{a-z}/{name}.fol` | Follower data |
| `lib/mutable/rent/{a-z}/{name}.fr` | Follower rent |
| `lib/mutable/corpses/{name}` | Binary corpse file |

### Cache Data Structures

| Structure | Description |
|-----------|-------------|
| `TObjectCache` | Map from real numbers to `cached_object` pointers |
| `TMobileCache` | Three maps: `cache` (basic data), `extra` (descriptions), `imm` (immunities) |
| `cached_object` | Field maps keyed by column name (access via `obj->s["short_desc"]`) |

## Implementation

### Database Architecture

`TDatabase` wraps MariaDB with printf-style format specifiers. Queries are logged via `vlogf(LOG_DB, ...)`. Error returns: `query()` returns `false`, `fetchRow()` returns `false`, `operator[]` returns empty string, `rowCount()` returns `-1`.

`TTransaction` provides RAII transaction handling: constructor calls `BEGIN`, destructor calls `COMMIT`.

Object and mobile caches (`TObjectCache`, `TMobileCache`) preload templates from database during `bootDb()` into `obj_cache` and `mob_cache` maps for O(1) instantiation lookup. These caches are populated at boot and never refreshed; restart the game to reload after database changes.

### Binary Character Files

`charFile` struct in `charfile.h` is written/read via `fwrite`/`fread` with `sizeof(charFile)`. No serialization, versioning, or endianness handling. The exact memory layout is persisted.

Key functions: `load_char()` reads binary and syncs money from DB; `raw_save_char()` writes binary and syncs money to DB; `storeToSt()` copies runtime to struct; `loadFromSt()` copies struct to runtime. These conversion functions handle truncation when runtime data exceeds array limits defined by `MAX_AFFECT`, `ABSOLUTE_MAX_SKILL`, and other constants.

Struct sections: basic info (sex, level, race, Class), physical (weight, height, body state), fixed-size strings (title[80], name[20], pwd[11]), time tracking, current stats, affects, faction, skills, obsolete/reserved fields.

### Rent and Item Persistence

File-based storage (`ItemSave`/`ItemLoad`) uses `rentObject` binary structs for player rent and corpses. Strung items append custom strings after the struct.

Database storage (`ItemSaveDB`/`ItemLoadDB`) uses three tables: `rent` (primary item data), `rent_obj_aff` (enchantments), `rent_strung` (custom descriptions). Shops use database exclusively.

Corpses auto-save on creation, item add/remove, and mob looting. `TPCorpse::saveCorpseToFile()` writes to `mutable/corpses/`. Global `pc_corpse_list` tracks all player corpses.

### Rent File Version Migration

The `raw_read_item` function checks the version field in rent file headers. For versions below 10, it applies migration logic for each version. The next save writes the current version 10 format, progressively upgrading old files as players log in.

### Money Synchronization Flow

Load: Query `player.talens`; if found, overwrite `char_element->money`; if not found, seed DB from binary value.

Save: `raw_save_char()` writes binary file, then updates `player.talens` via query joining `player` and `account` tables.

Dual storage exists because `charFile.money` cannot be removed, but database is authoritative for all money operations.

### Corruption Handling

When corruption is detected, `handleCorrupted()` moves files to `corrupt/` subdirectories: player file, strings, toggles, career data, rent file, followers, and corpses. The character is removed from their account but data is preserved for manual recovery.

### Key Source Files

| File | Purpose |
|------|---------|
| `code/code/misc/player_data.cc` | Character save/load, money sync |
| `code/code/misc/rent.cc` | Rent/persistence implementation |
| `code/code/misc/charfile.h` | Binary character struct |
| `code/code/sys/database.cc` | Database access, caching |
| `db/sneezy/rent*.sql` | Item storage schema |
| `db/sneezy/player.sql` | Player table schema |

## Troubleshooting

| Symptom | Cause | Fix |
|---------|-------|-----|
| All player data corrupted after code change | Modified `charFile` struct (added/removed/reordered fields) | Restore from backup; revert code change |
| Player loads with wrong values in fields | Changed array constant or field size | Restore from backup; revert constant change |
| Money inconsistent between logins | Database update failed during save | Check DB connectivity; verify `player.talens` matches expected |
| Rent file won't load | Version mismatch or corruption | Check `raw_read_item()` version handling; check `corrupt/` directory |
| SQL injection vulnerability | Used `%r` with user input | Replace with `%s`; validate input is server-generated |
| Items missing from shop | Database query failed silently | Check `LOG_DB` for errors; verify shop number in `rent.owner` |
| Corpse contents lost | Corpse not saved after modification | Verify `saveCorpseToFile()` called on item changes |
| Player file in `corrupt/` directory | Load detected data corruption | Manually inspect file; recover salvageable data to new record |
| Database shows wrong row count | Query failed, returned -1 | Check `query()` return value before using `rowCount()` |
| LIKE query finds nothing | Missing `%%` escaping | Use `'%%%s%%'` pattern for wildcard searches |
| Shop items duplicate after reload | Items saved but not deleted from memory, causing double-save | Ensure shop code deletes items from memory after saving; clear before reload |
| Database queries return stale data | Caches populated at boot, never refreshed | Restart game to reload caches after database changes |
| Items lost from containers | Parent deleted without deleting children | Query for orphaned items (`container != -1` with missing parent); delete or set `container = -1` |
| Transaction rollback leaves partial state | Exception or early return bypassed `TTransaction` destructor | Ensure transaction scope ends without throws; use manual BEGIN/COMMIT for multi-function transactions |
| Follower rent files missing | Write failed silently (permissions, disk full) | Check `lib/mutable/rent/{a-z}/{name}.fol` and `.fr`; verify permissions and disk space |
