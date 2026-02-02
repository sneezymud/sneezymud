---
title: Alias System
category: understanding
keywords: [doAlias, doClear, loadAliases, parseCommand, parameter substitution, multi-command, descriptor alias map, database persistence]
related: [command-implementation.md, network-architecture.md]
primary_symbols:
  functions: [doAlias, doClear, loadAliases, parseCommand]
  classes: [Descriptor]
  files: [code/code/misc/info.cc, code/code/misc/player_data.cc, code/code/misc/parse.cc, code/code/misc/connect.h]
---

# Alias System

## Overview

How do you let players create custom shortcuts without hardcoding every possible combination? The alias system transforms short words into full commands at parse time, before command lookup begins.

Aliases map a short word to a longer command string. When a player types an aliased word, it expands to the full command before execution. This happens transparently during input processing - the command dispatcher never sees the original alias, only its expansion.

Each character maintains their own alias map stored in their descriptor. When a player types "att goblin", the parser checks if "att" is an alias. If found (say, mapped to "attack %"), the system substitutes the parameters and continues with "attack goblin" as if that's what the player typed.

The system supports two primary capabilities beyond simple text replacement:

**Parameter substitution** allows aliases to act as command templates. The `%` character marks where typed arguments should be inserted. If you define `alias h pray heal %`, typing "h Bob" expands to "pray heal Bob". Multiple `%` symbols in multi-command aliases each get replaced.

**Multi-command execution** lets a single alias trigger multiple commands in sequence. The `~` separator splits the alias into distinct commands that execute in order through the normal command queue. This enables complex macros like "buff" that applies multiple spells or "prep" that stands, draws, and wields a weapon.

Aliases persist across login sessions. They're stored in the database with unlimited count and support commands up to 999 characters. The expansion happens once per input line - aliases don't expand recursively to prevent infinite loops.

Common scenarios:

**Simple shortcut**: Player creates `alias cl cast cure light`, then types "cl Bob". Parser expands to "cast cure light Bob" before command lookup.

**Parameter placement**: Player creates `alias att attack %`, then types "att goblin with sword". Expands to "attack goblin with sword" with all arguments after the alias replacing the `%`.

**Multi-command macro**: Player creates `alias buff cast armor~cast bless~cast strength`, then types "buff". Parser splits on `~` and queues three commands: "cast armor", "cast bless", "cast strength".

**Complex macro with parameters**: Player creates `alias h pray heal %~cont~comf %`, then types "h Bob". Expands to three commands with parameter substitution: "pray heal Bob", "cont", "comf Bob".

## Patterns

### Creating Effective Aliases

**DO** use parameter substitution for commands that target different things. Define `alias att attack %` rather than separate aliases for each target. The `%` makes the alias reusable.

**DO** use multi-command separators for repeatable sequences. If you always buff with armor, bless, and strength, `alias buff cast armor~cast bless~cast strength` saves typing and ensures consistency.

**DO** place `%` where arguments belong semantically. For "pray heal target", use `alias h pray heal %` not `alias h % pray heal`. The parameter position should match how you think about the command.

**DON'T** create aliases that conflict with protected words. The system rejects `alias clear anything` because it would prevent using the clear command. Similarly, `alias x x` would create an infinite loop and is blocked.

**DON'T** expect recursive expansion. If alias "a" expands to "b target" and "b" is also an alias for "attack", typing "a goblin" produces "b target goblin", not "attack target goblin". Only the first alias expands.

**DON'T** use client mode and in-game aliases together. When `m_bIsClient` is true, the in-game alias commands are disabled. Use the client's native `#alias` command instead.

### Managing Aliases Safely

**Always clear by number, not by name.** The clear command expects the numbered position from the alias list: `clear 3` removes the third alias. There is no `clear myalias` syntax.

**Remember case sensitivity in lookups.** The alias map is case-sensitive. `CL`, `Cl`, and `cl` are three different aliases. This affects both creation and expansion.

**Understand expansion happens once.** The parser checks only the first word of player input against the alias map. If you type "cast cl Bob", the word "cast" is checked (not an alias), so "cl" won't expand even if it's aliased. Only "cl Bob" would trigger expansion.

**Know that multi-command timing follows normal queue processing.** Commands separated by `~` don't execute atomically. They're queued in sequence, so other players' actions can interleave if you're in combat or a busy room.

### Length and Count Constraints

**Alias words** are limited to 50 characters. This is enforced by the database schema's VARCHAR(50) column.

**Commands** are limited to 999 characters. Multi-command macros with many `~` separators can hit this limit if individual commands are verbose.

**No count limit** exists on the number of aliases per character. The legacy system capped at 16 aliases; the database migration removed this constraint.

## Reference

### Symbol Quick Reference

| Symbol | Type | Purpose |
|--------|------|---------|
| `doAlias()` | function | Create or view aliases, saves immediately to database |
| `doClear()` | function | Delete aliases by number or all, removes from database |
| `loadAliases()` | function | Load all aliases from database into descriptor map at login |
| `parseCommand()` | function | Expand aliases before command lookup |
| `Descriptor::alias` | member | In-memory std::map<sstring, sstring> storing alias mappings |
| `m_bIsClient` | flag | When true, in-game aliases disabled (use client aliases) |

### Player Commands

| Command | Syntax | Effect |
|---------|--------|--------|
| `alias` | `alias` | Display numbered list of all aliases |
| `alias` | `alias <word> <command>` | Create new alias or update existing |
| `clear` | `clear <number>` | Delete alias at numbered position |
| `clear` | `clear all` | Delete all aliases for character |

### Special Characters

| Character | Meaning | Example |
|-----------|---------|---------|
| `%` | Parameter substitution marker | `attack %` + "goblin" → "attack goblin" |
| `~` | Multi-command separator | `stand~draw sword` → two commands |

### Database Schema

| Column | Type | Constraint | Purpose |
|--------|------|------------|---------|
| `id` | INT | PRIMARY KEY AUTO_INCREMENT | Unique alias identifier |
| `player_id` | BIGINT(20) UNSIGNED | NOT NULL, FOREIGN KEY | Links to player table |
| `word` | VARCHAR(50) | NOT NULL | The short alias word |
| `command` | VARCHAR(999) | NOT NULL | The expanded command string |

### Protected Alias Names

| Word | Reason for Protection |
|------|----------------------|
| `clear` | Would conflict with clear command |
| Same as command | `alias x x` creates infinite loop |

### Length Limits

| Field | Maximum Length | Source |
|-------|---------------|--------|
| Alias word | 50 characters | Database VARCHAR(50) |
| Command | 999 characters | Database VARCHAR(999) |

### Legacy Limits (Pre-Migration)

| Field | Old Limit | New Limit |
|-------|-----------|-----------|
| Alias count | 16 per character | Unlimited |
| Alias word | 11 characters | 50 characters |
| Command | 29 characters | 999 characters |

## Implementation

### Alias Expansion Flow

Alias expansion occurs in `parseCommand()` before command lookup. The function extracts the first word from player input and checks it against the descriptor's alias map. If found, the system performs substitution and continues parsing with the expanded text.

The expansion process works as follows:

When player input arrives, `parseCommand()` splits the input into a first word and remaining arguments. The first word is used as a key to look up in `desc->alias`. If the key exists, the system retrieves the command string.

If the command string contains `%`, every occurrence is replaced with the remaining arguments from the original input. If the command string contains no `%` but arguments were provided, the arguments are appended to the end of the command.

If the expanded command contains `~` characters, it's split into separate commands. These commands are pushed onto a stack in reverse order, then prepended to the command queue. This ensures they execute in the order written (first command after first `~`, then second command, etc.).

After expansion completes, parsing continues with the expanded text as if that's what the player originally typed. The command dispatcher never sees the alias word - it processes only the expanded result.

### Storage Architecture

During gameplay, aliases live in the descriptor's alias map. This is declared in `connect.h` as `std::map<sstring, sstring> alias`. The map key is the alias word; the value is the command string.

Persistent storage uses the `alias` table in the database. The table links aliases to players via `player_id` foreign key with `ON DELETE CASCADE`, so deleting a player automatically removes their aliases.

The database schema was created during migration from the legacy character file format. Previously, aliases were stored in `charfile.h` as `aliasData alias[16]`, a fixed-size array with 11-character words and 29-character commands. The migration copied these to the database, enabling unlimited count and longer strings.

### Load and Save Operations

Aliases load from the database when a character logs in. The `loadAliases()` function in `player_data.cc` queries the `alias` table for all rows matching the player's ID, populating the descriptor's alias map.

Save operations happen immediately when aliases are created or modified. The `doAlias()` function in `info.cc` executes `INSERT ... ON DUPLICATE KEY UPDATE` to either create a new row or update an existing one with the same player_id and word.

Delete operations also happen immediately. The `doClear()` function executes a `DELETE` query to remove the specified alias. When clearing all aliases, it deletes all rows matching the player_id.

This immediate persistence model means aliases are never lost to crashes. There's no periodic save or flush - every modification commits to the database before the command returns.

### Multi-Command Execution

When an expanded alias contains `~`, the system splits the string on that delimiter. Each segment becomes a separate command string.

These commands are pushed onto a stack in reverse order, then prepended to the descriptor's command queue. This ordering ensures the first command (before the first `~`) executes first, followed by subsequent commands in written order.

The commands execute through normal command queue processing. This means:

- Each command goes through full parsing, including alias expansion (but aliases don't expand recursively)
- Other players' actions can interleave between commands if processing is slow
- DELETE flags and error handling apply to each command independently
- Combat rounds and other timed events can occur between commands

Parameter substitution happens before splitting. If the alias is `pray heal %~cont~comf %` and arguments are "Bob", the string becomes "pray heal Bob~cont~comf Bob", which then splits into three commands.

### Client Mode Interaction

The descriptor tracks whether the connection is from the SneezyMUD client via the `m_bIsClient` boolean flag. When this flag is true, the in-game alias commands are disabled.

The check happens at the start of `doAlias()` in `info.cc`. If client mode is detected, the function sends a message directing the player to use the client's `#alias` command and returns immediately without processing the alias.

This separation exists because the client has its own alias system that runs client-side before sending commands to the server. Allowing both would create confusion about which aliases apply and when.

Client-side aliases expand on the client before transmission. The server receives already-expanded commands and processes them normally. Server-side aliases would never trigger because the first word has already been replaced by the client.

### Protected Word Validation

When creating an alias, `doAlias()` checks the alias word against protected words. If the word is "clear", the creation is rejected with an error message. This prevents players from shadowing the clear command and losing the ability to delete aliases.

The system also prevents self-referential aliases. If a player tries to create an alias where the word and command are identical (like `alias x x`), the creation is rejected. This check prevents infinite loops during expansion.

These validations happen before database insertion. Invalid aliases never reach the database - they're rejected at creation time.

### Recursive Expansion Prevention

The parser expands aliases exactly once per input line. After expansion, the expanded text goes through command lookup without further alias checking. This prevents infinite loops and makes alias behavior predictable.

If alias "a" expands to "b target" and "b" is also an alias, typing "a goblin" produces "b target goblin". The system does not check if "b" is an alias because expansion has already occurred.

This single-pass design means aliases can safely reference other command names without risk of recursion. You can create `alias a attack` even though "attack" might be aliased elsewhere - only the first alias in the chain expands.

### Case Sensitivity

Alias lookups use the standard `std::map` key comparison, which is case-sensitive for sstring. The alias "CL", "Cl", and "cl" are distinct entries with different expansions.

This case sensitivity applies to both creation and expansion. Creating `alias CL cast cure light` won't trigger when typing "cl" - the lowercase version remains unaliased.

No case normalization occurs during lookup or storage. The exact string typed as the alias word must match the exact string typed at input for expansion to occur.

### Key Files and Functions

The alias system spans several files:

**info.cc** contains `doAlias()` and `doClear()`, the player-facing commands for managing aliases. These functions handle validation, database operations, and user feedback.

**player_data.cc** contains `loadAliases()`, called during character login to populate the descriptor's alias map from the database.

**parse.cc** contains the expansion logic in `parseCommand()`. This is where alias lookup, parameter substitution, and multi-command splitting occur.

**connect.h** declares the Descriptor class with its `std::map<sstring, sstring> alias` member and `bool m_bIsClient` flag.

**migrations.cc** contains the database migration that created the alias table and transferred legacy aliases from character files.

**charfile.h** contains the legacy `aliasData` structure definition, now deprecated but retained for understanding the old format.

## Troubleshooting

### Symptom: Alias doesn't expand when typed

**Likely cause:** Case mismatch, alias word not the first word typed, or client mode enabled.

**Diagnostic approach:**
1. Type `alias` to view all aliases and confirm the word exists
2. Check the exact capitalization - "CL" and "cl" are different
3. Ensure you're typing the alias word first - "cast cl" won't expand "cl"
4. Check if you're using the SneezyMUD client - client mode disables server aliases

**Fix:** If case mismatch, create the alias with the capitalization you type. If client mode, use `#alias` in the client instead. If the alias word isn't first, rearrange your input or create a different alias.

### Symptom: "clear myalias" doesn't work

**Likely cause:** Clear expects a number from the alias list, not the alias word.

**Diagnostic approach:**
1. Type `alias` to see the numbered list
2. Find the alias you want to delete and note its number

**Fix:** Use `clear <number>` where number matches the position in the alias list. For example, if "myalias" is #3 in the list, use `clear 3`.

### Symptom: Multi-command alias executes out of order or incompletely

**Likely cause:** Commands are queued through normal processing, so timing and DELETE flags can interrupt the sequence.

**Diagnostic approach:**
1. Check if combat is occurring - combat rounds can interleave with queued commands
2. Check if one command in the sequence causes death or deletion - this aborts remaining commands
3. Verify each command works independently by typing them one at a time

**Fix:** If timing is the issue, this is expected behavior - multi-command aliases don't execute atomically. If a command causes deletion, reorder the alias so critical commands execute first. If a command fails, debug that specific command independently.

### Symptom: Parameters don't substitute correctly in multi-command alias

**Likely cause:** Each `%` is replaced with the full argument string, which may not distribute as expected.

**Diagnostic approach:**
1. Type the alias definition to see where each `%` appears
2. Consider what the expanded result looks like with your arguments

**Fix:** Adjust the alias definition to place `%` only where you want the full argument string. If you need different arguments for different commands, you'll need separate aliases or manual typing.

### Symptom: Alias was created but disappeared after logout

**Likely cause:** Database save failed or player_id wasn't set correctly.

**Diagnostic approach:**
1. Check database connectivity and error logs during the session when alias was created
2. Query the `alias` table directly to see if the row exists
3. Check if the character's player_id is valid

**Fix:** If database connectivity failed, address the database issue and recreate the alias. If player_id is invalid, this indicates a deeper problem with character loading.

### Symptom: Can't create alias for "clear" or other protected words

**Likely cause:** The word is in the protected word list to prevent conflicts with core commands.

**Diagnostic approach:**
1. Try creating the alias and read the error message
2. Check if the word matches a critical command

**Fix:** Choose a different alias word. Use "cl" instead of "clear", or add a prefix/suffix like "myclear".

### Symptom: Alias expansion created infinite loop

**Likely cause:** This shouldn't happen - the system has protections against recursive expansion and self-referential aliases.

**Diagnostic approach:**
1. Verify the alias definition by typing `alias`
2. Check if the alias word and command are identical - this should be rejected at creation
3. Look for alias chains (a→b→a) - these shouldn't expand recursively

**Fix:** If this occurs, it's a bug in the expansion logic. Report it with the exact alias definitions involved. As a workaround, delete the problematic aliases with `clear all`.
