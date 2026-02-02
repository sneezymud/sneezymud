---
title: Persistence and Storage
category: critical
keywords: [TDatabase, charFile, save_char, load_char, ItemSave, ItemLoad, rent table, SQL injection, binary format, hybrid storage]
related: [configuration-reference.md, economy-system.md]
primary_symbols:
  functions: [load_char, raw_save_char, ItemSaveDB, ItemLoadDB, saveItem, loadItem]
  classes: [TDatabase, TTransaction, charFile, ItemSave, ItemLoad, TObjectCache, TMobileCache]
  files: [code/code/misc/player_data.cc, code/code/misc/rent.cc, code/code/sys/database.cc, code/code/misc/charfile.h]
---

## Overview

SneezyMUD uses a hybrid persistence architecture combining binary files with MariaDB databases. This design evolved from pure binary storage to incorporate database-backed features while maintaining compatibility with legacy save formats.

The system divides responsibility between storage mechanisms based on mutability and query requirements. Binary files store the core character state as a raw memory dump of the charFile struct, located in `lib/mutable/player/{first_letter}/{charname}`. The MariaDB database handles queryable data like money, relationships between entities, and data added after the binary format was frozen.

Two databases serve different purposes: DB_SNEEZY stores game data including objects, mobs, and players, while DB_IMMORTAL stores immortal-specific information. The database layer provides template caching through TObjectCache and TMobileCache, which preload all object and mobile definitions at boot for O(1) lookup during instantiation.

Item persistence uses parallel systems depending on context. Player rent files and corpses use binary rentObject structs written to `lib/mutable/rent/` and `lib/mutable/corpses/`. Shops, rooms, and mail use database storage through the rent, rent_obj_aff, and rent_strung tables. This dual approach balances performance for common operations with flexibility for complex queries.

The binary format imposes severe constraints: the charFile struct cannot have fields added, removed, reordered, or resized without corrupting all existing player saves. Obsolete and unused fields remain in the struct solely for binary alignment. All new persistent player data must use database storage.

Money exemplifies the hybrid model's transitional state. The talens field exists in both charFile and the database, but the database value wins on load. This dual storage maintains binary compatibility while establishing the database as authoritative for economic queries.

## Patterns

### Database Query Construction

Use local TDatabase instances that clean up automatically through destructors. The class provides printf-style format specifiers for safe query construction: %s for strings with automatic escaping via mysql_real_escape_string, %i for integers, %f for doubles, and %% for literal percent signs. These specifiers prevent SQL injection by validating or escaping parameters before query execution.

The %r specifier inserts raw unescaped strings directly into queries and represents a SQL injection vulnerability if used with user input. Restrict %r to server-generated SQL fragments like pre-built IN clause lists. Never pass player names, descriptions, commands, or any user-controlled data through %r. When constructing LIKE queries, use %% to produce literal percent wildcards.

Iterate results by calling fetchRow in a loop, accessing columns by name or index through the bracket operator. Non-SELECT queries do not clear previous SELECT results, allowing you to preserve one result set while executing updates. Check query return values before accessing results, though the class returns safe empty values rather than crashing on errors.

### Transaction Management

Use TTransaction for multi-statement atomicity. The constructor calls BEGIN and the destructor calls COMMIT, ensuring transactions complete even if exceptions occur. Wrap related inserts, updates, and deletes in a transaction block to prevent partial state from crashes or errors. This pattern is critical for money transfers, inventory operations, and any sequence where intermediate states would corrupt game data.

### Character Save/Load Cycle

The load_char function reads the binary file then synchronizes money from the database. If a database record exists, its talens value overwrites the charFile money field. If no database record exists, the binary file value seeds the database. This establishes the database as authoritative while handling migration from older saves.

The raw_save_char function writes the binary file using fwrite with sizeof(charFile), then updates the database with money and account linkage. Before writing, storeToSt copies runtime TBeing data into the charFile struct. After reading, loadFromSt copies charFile data back into runtime structures. These conversion functions handle the impedance mismatch between game state and the frozen binary format.

### Item Persistence Strategy Selection

Use ItemSave and ItemLoad classes for file-based storage when handling player rent files or corpses. These write binary rentObject structs with optional string data appended for customized items. The format includes versioning to support backward compatibility with older rent files.

Use ItemSaveDB and ItemLoadDB classes for database storage when handling shops, rooms, or mail. These write to the rent table with owner_type distinguishing between contexts. Nested containers use the container field to reference their parent's rent_id. Strung items have custom descriptions stored in rent_strung, while enchantments populate rent_obj_aff.

### Cache Access Pattern

Access obj_cache and mob_cache by real number to retrieve cached_object pointers. These caches are populated during bootDb through the preload methods. The cached_object struct contains field maps accessed by name, allowing instantiation without database queries. For mobiles, the extra and imm maps provide additional cached data for descriptions and immunities.

### Corruption Recovery

When file corruption is detected, use handleCorrupted to move files rather than delete them. The function relocates player files, strings, toggles, career data, rent files, followers, and corpses to corrupt subdirectories. This preserves data for manual recovery while preventing the corrupted state from affecting normal operation. Remove the character from their account but do not destroy the data.

## Reference

### TDatabase Methods

Constructor TDatabase(dbTypeT db, bool log = false) opens a connection to DB_SNEEZY or DB_IMMORTAL with optional query logging. The destructor automatically closes connections and frees resources.

The query method accepts printf-style format strings with variable arguments and returns true on success or false on error. It executes the query immediately and sets up result sets for SELECT statements.

The fetchRow method advances to the next row in a SELECT result and returns true if a row exists or false when exhausted. Call this in a loop to iterate all results.

The bracket operator accepts column names as strings or numeric indices and returns the column value as an sstring. It returns an empty string on error rather than null pointers.

The isResults method returns true if the last query produced a result set. Use this to distinguish SELECT queries from INSERT, UPDATE, or DELETE.

The rowCount method returns the number of rows affected by INSERT, UPDATE, or DELETE, or the number of rows retrieved by SELECT. It returns -1 on error.

The lastInsertId method returns the auto-increment ID generated by the last INSERT statement. Use this to retrieve primary keys after inserting new rows.

### TDatabase Format Specifiers

The %s specifier accepts char* parameters and applies mysql_real_escape_string to prevent SQL injection. Use this for all user-controlled strings including names, descriptions, and command arguments.

The %i specifier accepts int parameters and inserts them as numeric literals without escaping. Use this for player IDs, vnums, and counts.

The %f specifier accepts double parameters and inserts them as floating-point literals without escaping. Use this for weights, prices, and percentages.

The %r specifier accepts char* parameters and inserts them raw without escaping. This creates SQL injection vulnerabilities if used with user input. Restrict usage to server-generated SQL fragments.

The %% specifier produces a literal percent character in the output. Use this when constructing LIKE patterns that need wildcards.

### charFile Structure Sections

The basic info section contains sex, level array with MAX_SAVED_CLASSES entries, race, and Class. These define character identity and progression.

The physical section contains weight, height, body_flags array with MAX_HUMAN_WEAR entries, and body_health array. These track physical state and limb damage.

The strings section contains fixed-size char arrays: title with 80 bytes, name with 20 bytes, pwd with 11 bytes, and description with 500 bytes. These cannot be dynamically sized due to binary format constraints.

The time section contains birth, last_logon, and played timestamps tracking when the character was created, last connected, and total play time.

The points section contains mana, hit, move, money, exp, and other numeric statistics representing current resource levels and progression.

The affects section contains affected array with MAX_AFFECT entries storing active spell effects.

The faction section contains f_percent, f_percx array with ABS_MAX_FACTION entries, f_type, and f_actions tracking faction standings.

The skills section contains disc_learning array with MAX_SAVED_DISCS entries and skills array with ABSOLUTE_MAX_SKILL entries storing learned abilities and their proficiency.

The obsolete section contains obsolete_prompt_colors with 200 bytes and obsolete_p_type retained solely for binary alignment. These waste space but cannot be removed.

The reserved section contains temp1 through temp4 and unused fields reserved for future use without breaking binary compatibility.

### rent Table Structure

The rent_id field is an auto-increment primary key identifying each stored item uniquely.

The owner_type field is an enum with values player, shop, room, or mail indicating the context storing the item.

The owner field contains the player ID when owner_type is player, shop number when shop, room number when room, or mail ID when mail.

The slot field contains the equipment slot number or -1 for inventory items. This determines where equipped items appear on the character.

The vnum field contains the object virtual number referencing the object template.

The container field contains the parent item's rent_id for nested objects or -1 for top-level items. This creates hierarchical storage for containers holding other items.

The val0 through val3 fields store object-specific values like charges, capacity, or weapon statistics. Their meaning depends on object type.

The extra_flags, bitvector, material, volume, price, and depreciation fields store object state including flags, material type, size, cost, and wear.

The weight, cur_struct, and max_struct fields track physical properties and durability.

The decay field tracks time-based deterioration for perishable items.

### rent_obj_aff Table Structure

The rent_id field is a foreign key to rent.rent_id linking affects to their parent object.

The type field identifies the spell or affect applied to the object.

The level field stores the caster level or effect strength.

The duration and renew fields control how long the affect lasts and whether it automatically refreshes.

The modifier and modifier2 fields contain the numeric bonuses or penalties applied.

The location field identifies which character attribute receives the modification.

The bitvector field contains flag bits for special effects.

### rent_strung Table Structure

The rent_id field is a foreign key to rent.rent_id linking descriptions to their parent object.

The name field contains the custom item name displayed when referenced.

The short_desc field contains the brief description shown in inventory and equipment lists.

The long_desc field contains the detailed description shown when the item is on the ground.

The action_desc field contains text displayed when the item is used or activated.

### Cache Data Structures

The TObjectCache class maintains a map from real numbers to cached_object pointers. The preload method executes a SELECT query retrieving all rows from the obj table and populates the cache at boot.

The TMobileCache class maintains three maps: cache for basic mobile data, extra for description entries, and imm for immunity data. The preload method loads all mobiles with their associated extras and immunities.

The cached_object struct contains field maps keyed by column name. Access fields using bracket notation like obj->s["short_desc"] for strings.

The cached_mob_extra and cached_mob_imm structs store additional mobile properties loaded through separate queries and indexed by mobile real number.

## Implementation

### Money Synchronization Algorithm

The load_char function executes a SELECT query retrieving talens from the player table where the name matches. If a row returns, the database talens value overwrites char_element->money regardless of what the binary file contained. If no row returns, an UPDATE query writes char_element->money to the database, seeding it from the binary file. This establishes database authority while handling characters that predate database storage.

The raw_save_char function performs the reverse synchronization. After writing the binary file, an UPDATE query writes char_element->money to the player table along with account_id linkage. The query uses a join to the account table to resolve account_id from char_element->aname. Both locations now contain the same value but the database remains authoritative for queries.

This bidirectional sync maintains binary compatibility while allowing money queries to use SQL. Without database storage, querying total money across all players would require reading every binary file. With database storage, a simple SELECT sum(talens) suffices.

### Binary File Write Process

The storeToSt method copies runtime TBeing data into the charFile struct. This includes iterating through spell affects and copying them into the affected array, converting skill proficiencies into the skills array, and serializing body state into body_flags and body_health arrays. The method handles truncation when runtime data exceeds array limits defined by MAX_AFFECT, ABSOLUTE_MAX_SKILL, and other constants.

The raw_save_char function opens the file at `lib/mutable/player/{first_letter}/{charname}` for binary writing. It calls fwrite with char_element pointer, sizeof(charFile), and count of 1. This dumps the exact memory layout to disk with no serialization layer. The function then executes database updates for money and account linkage.

### Binary File Read Process

The load_char function opens the file at `lib/mutable/player/{first_letter}/{charname}` for binary reading. It calls fread with char_element pointer, sizeof(charFile), and count of 1. This reads raw bytes directly into the struct memory. The function then executes database queries to synchronize money and other database-backed fields.

The loadFromSt method copies charFile data into runtime TBeing structures. This includes iterating through the affected array to recreate spell affects, converting the skills array into skill proficiencies, and deserializing body arrays into limb state. The method handles obsolete fields by ignoring them and reserved fields by leaving them unprocessed.

### Database Item Save Process

The ItemSaveDB constructor accepts owner_type string and owner integer, storing them for use in subsequent writes. The raw_write_item method begins by inserting a row into the rent table with owner_type, owner, slot, vnum, and all object property fields. It retrieves the auto-increment rent_id using lastInsertId.

For each entry in the object's affected array, raw_write_item inserts a row into rent_obj_aff with the rent_id and affect properties. If the object is strung, it inserts a row into rent_strung with custom descriptions.

For container objects, the method recursively calls itself on each contained item, passing the parent's rent_id as the container parameter. This builds a tree structure in the database where the container field links children to parents.

### Database Item Load Process

The ItemLoadDB constructor accepts owner_type string and owner integer, querying the rent table to build an in-memory map of items by rent_id. It also queries rent_obj_aff and rent_strung to preload affect and string data.

The raw_read_item method accepts a rent_id and retrieves the cached rent row. It calls read_object_1 with the vnum to instantiate a base object, then applies all property fields from the rent row including val0-val3, extra_flags, weight, and structure. It queries rent_obj_aff for affects matching the rent_id and applies each to the object. If rent_strung contains a row for this rent_id, it applies custom descriptions.

For items with container field not equal to -1, the method recursively loads the child item and adds it to the parent object. This reconstructs the containment hierarchy by following rent_id references.

### Shop Persistence Integration

The TMonster::saveItem method constructs an ItemSaveDB instance with owner_type "shop" and shop_nr. It calls raw_write_item with the object pointer, NORMAL_SLOT, and container rent_id. This stores the item in the rent table with owner_type set to shop.

The TMonster::loadItem method constructs an ItemLoadDB instance with owner_type "shop" and shop_nr. It calls raw_read_item with the rent_id to retrieve and reconstruct the item from database rows.

The TMonster::deleteItem method executes DELETE queries against rent, rent_obj_aff, and rent_strung where rent_id matches. This removes all database records for the item including nested containers if their rent_id values are also deleted.

### Corpse File Persistence

The TPCorpse::saveCorpseToFile method opens the file at `mutable/corpses/{fileName}` for binary writing. It writes a header containing the count of corpses in the linked list, then iterates pc_corpse_list writing each corpse as a TObj with its complete contents. The rent file format handles nested items through recursive writing.

Corpse saving occurs automatically when corpses are created during player death, when items are added or removed, and when mobs loot from corpses. The global pc_corpse_list tracks all player corpses in memory, ensuring all corpses are saved during shutdown.

### Cache Preloading

The TObjectCache::preload method executes a SELECT query retrieving all columns from the obj table. For each row, it allocates a cached_object struct, populates its field map with column values, and inserts it into the cache map keyed by real number. This front-loads database access cost to boot time.

The TMobileCache::preload method performs similar processing for the mob table, then executes additional queries for mob_extra and mob_immunities tables. It populates the extra and imm maps with vectors of cached entries keyed by mobile real number. Instantiation code accesses these maps to retrieve all data without additional queries.

### Rent File Version Migration

The raw_read_item function checks the version field in rent file headers. For versions below 10, it applies migration logic: version 1 converts weight from int to float, version 2 adds depreciation, version 8 shuffles weapon 4vals, and version 9 consolidates material types. Each migration adjusts how fields are interpreted from the binary data.

Version migration happens transparently during load. The next save writes the current version 10 format, progressively upgrading old files as players log in. This allows format evolution while maintaining backward compatibility with decades-old rent files.

### Corruption Detection and Handling

File corruption is detected through checksum validation, struct size mismatches, and invalid enum values. When load_char or ItemLoad encounters corrupted data, it calls handleCorrupted with the character name and account.

The handleCorrupted function constructs source paths for player file, strings, toggles, career data, rent file, follower data, and corpses. It creates a corrupt subdirectory and moves all files there. It executes a database query removing the character from their account without deleting database rows. This preserves all data for manual inspection while preventing game access to corrupted state.

## Troubleshooting

### New charFile fields corrupt existing saves

The binary format is frozen. Any modification to charFile including adding fields, removing fields, reordering fields, or changing field sizes breaks all existing player saves. The fread operation reads exactly sizeof(charFile) bytes, so size changes cause misalignment. Field reordering causes data to load into wrong fields. There is no migration path because the format has no versioning or field metadata.

Add new persistent player data to database tables instead. Follow the pattern of alias, playerprompt, or trophy tables that store player-keyed data outside the binary file. Load this data separately after loading the charFile and save it in separate transactions.

### Money desyncs between file and database

The database value is authoritative and overwrites the file value on load. If money appears incorrect after player login, check the player table talens field. Manual database edits to money require no binary file changes because the database wins.

If money is lost after a crash, check whether raw_save_char completed its database UPDATE before the crash. The transaction covers both the binary write and the database update, so either both should succeed or both should roll back. Examine database logs for transaction failures.

### Corpses disappear after crash

Player corpses are saved to `mutable/corpses/{charname}` when created and when modified. Check whether the corpse file exists and contains valid data. If the file is present but the corpse does not load, examine the pc_corpse_list loading code in bootDb for errors.

If the file is absent, the corpse was created but never saved. This indicates the crash occurred between corpse creation and the first save. Check whether saveCorpseToFile is called in the death code path and whether the file write completed.

### Shop items duplicate after reload

Duplication occurs when items are saved to the database but not deleted from memory, causing them to be saved again on shutdown. Verify that shop code deletes items from memory after saving them, or uses a flag to prevent double-saving.

Check whether shop reset code loads items from the database and adds them to the shop without clearing existing items first. The reload pattern should delete all existing shop items, then load fresh from the database.

### SQL injection from player name

Player names should always use %s format specifier, never %r. The %s specifier applies mysql_real_escape_string preventing injection. Review all queries that include player names and verify they use %s.

If a name contains single quotes or backslashes and causes query errors, the code is likely using %r or constructing queries with raw string concatenation. Replace with %s specifier or use TDatabase::query parameter substitution.

### Database queries return stale data

The object and mobile caches are populated at boot and never refreshed. If the database is modified while the game runs, caches become stale. Restart the game to reload caches.

For player data that must reflect live database changes, query directly without relying on cached data. Player aliases, toggles, and trophies use this pattern, querying the database each time they are needed rather than caching.

### Rent file version errors on load

Old rent files use version numbers below 10. The raw_read_item function applies migrations to convert old formats to current. If migration fails, the rent file may be from an unsupported version or corrupted.

Check the version field in the rent file header. If it is greater than 10, the file was created by a newer version and this codebase cannot read it. If it is a recognized old version but migration fails, examine the migration logic for that version to identify which field causes the error.

### Items lost from containers

Container hierarchies are stored through the container field referencing parent rent_id values. If the parent item is deleted without deleting children, orphaned items remain in the database but are inaccessible.

Query rent table for rows where container is not -1 but the referenced rent_id does not exist. These are orphaned items. Either delete them or update their container to -1 to make them top-level items.

### Transaction rollback leaves partial state

TTransaction calls COMMIT in its destructor, so exceptions or early returns can bypass the commit. Ensure transaction scope ends naturally without throws or early returns. Use explicit commit calls if the destructor pattern is insufficient.

If a transaction must span multiple functions, use manual BEGIN and COMMIT calls instead of TTransaction. Track transaction state to ensure all paths either commit or rollback.

### Follower rent files missing

Follower rent files are stored at `lib/mutable/rent/{first_letter}/{charname}.fr` and follower data at `.fol`. If followers disappear after logout, check whether these files exist and contain valid data.

The follower save code may fail silently if the directory is not writable or the disk is full. Check file system permissions and disk space. Examine logs for rent file write errors during follower save.
