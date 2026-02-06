---
title: Alias System
description: Player command shortcuts with parameter substitution and multi-command chaining for efficient gameplay.
category: informational
keywords: [parameter substitution, multi-command, macro, shortcut]
primary_symbols:
  functions: [doAlias, doClear, loadAliases, parseCommand]
  classes: [Descriptor]
  enums: [m_bIsClient]
---

# Alias System

## Overview

How do players avoid typing the same long commands repeatedly? The alias system provides custom shortcuts that expand into full commands before execution.

Aliases map a short word to a longer command string. When a player types an aliased word, the system expands it to the full command before looking up what command to execute. This expansion happens transparently - the player types less, but the game sees the full command.

The system exists because MUD gameplay involves repetitive command patterns. A healer casting "pray cure critical wounds" dozens of times per session benefits enormously from typing "pcc" instead. Combat sequences, buff rotations, and social macros all become manageable through aliases.

Each character maintains their own alias list, persisted in the database across sessions. There is no limit on the number of aliases a character can have. Aliases support parameter substitution (passing arguments through to the expanded command) and multi-command chaining (executing several commands in sequence from a single alias).

A typical session might look like:
- **Before aliases:** Player types "cast cure light Bob" repeatedly, making typos under combat pressure
- **With alias defined:** Player creates `alias cl cast cure light`
- **During play:** Player types "cl Bob" and the system expands it to "cast cure light Bob"

## Patterns

### Creating and Managing Aliases

Always use descriptive alias names that you will remember. Single-letter aliases like "a" may conflict with partial command matching.

Always check your alias list with the bare `alias` command before creating a new alias to avoid accidentally overwriting an existing one with the same name.

Never create an alias with the same name as its expansion target. The system rejects `alias x x` because it would create an infinite loop.

Never name an alias "clear" - this conflicts with the command used to delete aliases.

### Parameter Substitution

Use `%` in your alias definition when you need to pass different targets each time. The `%` is replaced with whatever arguments follow the alias when invoked.

If you omit `%` from the definition, arguments are automatically appended to the end. This works for simple cases but explicit `%` placement gives more control.

### Multi-Command Aliases

Use `~` to separate commands within a single alias. Commands execute in sequence with normal queue processing between them.

When combining `%` with multi-command aliases, every occurrence of `%` is replaced. An alias like `alias h pray heal %~comf %` expands "h Bob" to execute "pray heal Bob" followed by "comf Bob".

Be aware that multi-command aliases do not execute atomically. Other players' actions can interleave between commands in combat or busy rooms. DELETE flags and error handling apply to each command independently, so if one command causes death or deletion, remaining commands are aborted.

### Client Mode

Never attempt to use in-game aliases when connected via the SneezyMUD client. The client disables in-game aliases to prevent conflicts. Use the client's `#alias` command instead.

This separation exists because the client has its own alias system that expands client-side before transmission. The server receives already-expanded commands, so server-side aliases would never trigger anyway.

### Case Sensitivity

Be aware that alias lookups are case-sensitive. Defining `cl` does not create `CL` - they are separate aliases.

### Expansion Scope

Only the first word of input is checked as an alias. Typing "cast cl Bob" does not expand "cl" because it is not the first word.

Aliases do not expand recursively. If alias "a" expands to text containing alias name "b", the "b" is not further expanded. This prevents infinite expansion loops but means you cannot chain alias definitions.

## Reference

### Symbol Quick Reference

| Symbol | Type | Purpose |
|--------|------|---------|
| `doAlias()` | function | Create or view aliases |
| `doClear()` | function | Delete aliases by number or all |
| `loadAliases()` | function | Load aliases from database at login |
| `parseCommand()` | function | Command parsing including alias expansion |
| `Descriptor` | class | Holds per-connection state including alias map |
| `Descriptor::alias` | member | In-memory `std::map<sstring, sstring>` storing alias mappings |
| `m_bIsClient` | flag | When true, in-game aliases disabled (use client aliases) |
| `info.cc` | file | Alias and clear command implementations |
| `player_data.cc` | file | Alias loading at login |
| `parse.cc` | file | Alias expansion during command parsing |
| `connect.h` | file | Descriptor class with alias map declaration |
| `migrations.cc` | file | Database migration from legacy character file format |
| `charfile.h` | file | Legacy aliasData structure (deprecated) |

### Alias Commands

| Command | Purpose |
|---------|---------|
| `alias` | List all defined aliases |
| `alias <word> <command>` | Create or update an alias |
| `clear <number>` | Delete alias by list number |
| `clear all` | Delete all aliases |

### Special Characters

| Character | Purpose |
|-----------|---------|
| `%` | Replaced with arguments typed after the alias |
| `~` | Separates multiple commands in one alias |

### Length Limits

| Field | Maximum |
|-------|---------|
| Alias word | 50 characters |
| Command expansion | 999 characters |

### Legacy Limits (Pre-Migration)

| Field | Old Limit | New Limit |
|-------|-----------|-----------|
| Alias count | 16 per character | Unlimited |
| Alias word | 11 characters | 50 characters |
| Command | 29 characters | 999 characters |

### Protected Words

| Word | Reason Rejected |
|------|-----------------|
| `clear` | Conflicts with the delete command |
| Same as expansion | Would create infinite loop |

### Database Schema

| Column | Type | Purpose |
|--------|------|---------|
| `id` | INT | Primary key |
| `player_id` | BIGINT UNSIGNED | Foreign key to player table |
| `word` | VARCHAR(50) | The alias trigger word |
| `command` | VARCHAR(999) | The expansion text |

## Implementation

### In-Memory Storage

During play, aliases are stored in a map on the descriptor object. The descriptor holds all per-connection state, and the alias map is declared as `std::map<sstring, sstring>` in `connect.h`. The map key is the alias word and the value is the command expansion.

### Database Persistence

Aliases persist in the `alias` database table with a foreign key to the player table. The foreign key has `ON DELETE CASCADE`, so deleting a player automatically removes their aliases.

When a character logs in, `loadAliases()` in `player_data.cc` queries the database and populates the descriptor's alias map.

When a player creates or modifies an alias, `doAlias()` in `info.cc` immediately executes `INSERT ... ON DUPLICATE KEY UPDATE` to persist the change. There is no buffered saving - changes are durable immediately.

When a player clears an alias, `doClear()` in `info.cc` immediately executes a `DELETE` statement.

### Expansion Process

Alias expansion occurs in `parseCommand()` in `parse.cc`, happening before command lookup. The expansion follows this sequence:

1. Parse the first word from player input
2. Look up the first word in the descriptor's alias map
3. If found, substitute the expansion for the alias word
4. If the expansion contains `%`, replace each occurrence with the remaining arguments
5. If the expansion has no `%` but arguments were provided, append them to the end
6. If the expansion contains `~`, split into multiple commands
7. For multi-command expansions, push commands onto a stack and prepend to the command queue

The command queue processing ensures multi-command aliases execute in the order written, with normal game timing between commands. Combat rounds and other timed events can occur between commands.

### Protected Word Validation

When creating an alias, `doAlias()` checks the alias word against protected words. These validations happen before database insertion - invalid aliases are rejected at creation time and never reach the database.

### Client Mode Handling

When the descriptor's `m_bIsClient` flag is true, `doAlias()` returns early with a message directing the user to the client's alias system. This prevents confusion from having two separate alias mechanisms active simultaneously.

### Legacy Migration

Prior to database storage, aliases were stored in the character file as a fixed array. The legacy structure limited players to 16 aliases with 11-character words and 29-character commands.

The database migration in `migrations.cc` transferred existing aliases from character files to the database, removing these restrictions. New characters have no legacy data - their aliases exist only in the database.

## Troubleshooting

### Alias Not Expanding

**Symptom:** Typing the alias word executes it literally instead of expanding.

**Likely cause:** Connected via SneezyMUD client, which disables in-game aliases.

**Diagnostic approach:** Check if you see client-specific UI elements. Try the bare `alias` command - if it tells you to use client aliases, you are in client mode.

**Fix:** Use the client's `#alias` command instead of the in-game `alias` command.

### Wrong Alias Deleted

**Symptom:** Used `clear` and the wrong alias disappeared.

**Likely cause:** Used the wrong index number. `doClear()` only accepts a numeric index or "all", not alias names.

**Diagnostic approach:** Run `alias` to see the numbered list. The `clear` command takes a number from this list, not the alias word.

**Fix:** Recreate the accidentally deleted alias. Use the list number next time.

### Expansion Contains Literal `%`

**Symptom:** You wanted a literal percent sign in your command but it was replaced.

**Likely cause:** The `%` character always triggers substitution when present in an alias definition.

**Fix:** There is no escape mechanism for `%`. Avoid aliases for commands that need literal percent signs, or accept that you must not pass arguments when using such an alias.

### Nested Alias Not Working

**Symptom:** Defined alias "a" to expand to alias word "b", but "b" does not expand.

**Likely cause:** Aliases do not expand recursively by design.

**Fix:** Redefine alias "a" to contain the full expansion rather than relying on nested alias resolution.

### Multi-Command Alias Executes Out of Order

**Symptom:** Commands separated by `~` execute in unexpected order or some commands are skipped.

**Likely cause:** Commands are queued through normal processing, so timing and DELETE flags can interrupt the sequence.

**Diagnostic approach:** Check if combat is occurring - combat rounds can interleave with queued commands. Check if one command in the sequence causes death or deletion, which aborts remaining commands. Verify each command works independently by typing them one at a time.

**Fix:** If timing is the issue, this is expected behavior - multi-command aliases don't execute atomically. If a command causes deletion, reorder the alias so critical commands execute first.

### Parameters Don't Substitute Correctly in Multi-Command Alias

**Symptom:** Arguments don't appear where expected in multi-command aliases.

**Likely cause:** Each `%` is replaced with the full argument string, which may not distribute as expected.

**Diagnostic approach:** Type the alias definition to see where each `%` appears. Consider what the expanded result looks like with your arguments.

**Fix:** Adjust the alias definition to place `%` only where you want the full argument string. If you need different arguments for different commands, you'll need separate aliases or manual typing.

### Alias Disappeared After Logout

**Symptom:** Created an alias that was visible during the session but is gone after logging back in.

**Likely cause:** Database save failed during creation.

**Diagnostic approach:** Check database connectivity and error logs during the session when alias was created.

**Fix:** Address any database connectivity issues and recreate the alias.

### Cannot Create Alias for Protected Word

**Symptom:** Trying to create an alias results in an error message about protected words.

**Likely cause:** The word is in the protected word list to prevent conflicts with core commands.

**Fix:** Choose a different alias word. Use "cl" instead of "clear", or add a prefix/suffix like "myclear".
