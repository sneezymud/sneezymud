---
title: Alias System
description: The alias system allows players to create custom shortcuts for frequently-used commands through simple single-command shortcuts or complex multi-command macros with parameter substitution.
keywords: [doAlias, doClear, loadAliases, parseCommand, alias table, parameter substitution, multi-command separator, m_bIsClient, descriptor alias map, database persistence]
category: Understanding Systems
related: [command-implementation.md, network-architecture.md]
last_updated: 2026-01-29
source_files: [code/code/misc/info.cc, code/code/misc/player_data.cc, code/code/misc/parse.cc, code/code/misc/connect.h, code/code/misc/charfile.h, code/code/sys/migrations.cc]
---

# Alias System

The alias system allows players to create custom shortcuts for frequently-used commands. Aliases can be simple single-command shortcuts or complex multi-command macros with parameter substitution.

## Overview

Aliases map a short word to a longer command string. When a player types an aliased word, it expands to the full command before execution. Aliases are:

- **Per-character**: Each character has their own alias list
- **Persistent**: Stored in the database, survives logout/login
- **Unlimited count**: No limit on number of aliases
- **Length-limited**: Commands up to 999 characters

## Alias Commands

### Creating Aliases

```
alias <shortcut> <command>
```

**Examples:**
```
alias cl cast cure light
alias att attack %
alias buff cast armor~cast bless
alias h pray heal %~cont~comf %
```

### Viewing Aliases

```
alias
```

Displays a numbered list of all your aliases with their expansions:

```
Your list of aliases.....
 1) att | attack %
 2) buff | cast armor~cast bless
 3) cl | cast cure light
```

### Deleting Aliases

```
clear <number>
clear all
```

**Examples:**
```
clear 3        ; Removes alias #3 from the list
clear all      ; Removes all aliases
```

## Alias Expansion

Alias expansion occurs in `parseCommand()` (parse.cc:1981-2020) **before** command lookup. The system supports two special characters:

### Parameter Substitution (`%`)

The `%` character is replaced with any arguments typed after the alias:

| Alias Definition | User Types | Expands To |
|-----------------|------------|------------|
| `alias att attack %` | `att goblin` | `attack goblin` |
| `alias h pray heal %` | `h self` | `pray heal self` |
| `alias cl cast cure light` | `cl Bob` | `cast cure light Bob` |

**Note:** If no `%` is present and arguments are provided, they're appended automatically:
```
alias cl cast cure light
cl Bob  ->  cast cure light Bob
```

### Multi-Command Separator (`~`)

The tilde (`~`) separates multiple commands in a single alias:

| Alias Definition | Expands To |
|-----------------|------------|
| `alias buff cast armor~cast bless` | `cast armor` then `cast bless` |
| `alias prep stand~draw sword~wield sword` | Three commands in sequence |

When `%` is used with multi-command aliases, each occurrence is replaced:

```
alias h pray heal %~cont~comf %
h Bob  ->  "pray heal Bob" then "cont" then "comf Bob"
```

### Expansion Flow

```
Player input: "att goblin"
       |
       v
Parse first word: "att"
       |
       v
Check alias map: found "att" -> "attack %"
       |
       v
Substitute %: "attack goblin"
       |
       v
Continue to command lookup
```

For multi-command aliases, commands are pushed to a stack and prepended to the command queue, executing in order.

## Alias Storage

### Database Schema

Aliases are stored in the `alias` table (created in migrations.cc:36-44):

```sql
CREATE TABLE alias (
    id INT PRIMARY KEY AUTO_INCREMENT NOT NULL,
    player_id BIGINT(20) UNSIGNED NOT NULL,
    word VARCHAR(50) NOT NULL,
    command VARCHAR(999) NOT NULL,
    FOREIGN KEY (player_id) REFERENCES player (id) ON DELETE CASCADE
);
```

### In-Memory Storage

During play, aliases are stored in the descriptor:

```cpp
// connect.h:456
std::map<sstring, sstring> alias;  // aliases for players
```

### Load/Save Operations

| Operation | Location | Description |
|-----------|----------|-------------|
| Load | `loadAliases()` (player_data.cc:861-867) | Called at login |
| Save | `doAlias()` (info.cc:3777-3785) | Immediate `INSERT ... ON DUPLICATE KEY UPDATE` |
| Delete | `doClear()` (info.cc:3704-3717) | Immediate `DELETE` |

Aliases are loaded from the database when a character logs in and saved/deleted immediately when modified.

### Legacy Storage

Prior to the database migration, aliases were stored in the character file (charfile.h:89):

```cpp
aliasData alias[16];  // Limited to 16 aliases

class aliasData {
    char word[12];     // 11-character limit
    char command[30];  // 29-character limit
};
```

The migration (migrations.cc:51-68) transferred these to the database, allowing unlimited aliases with longer commands.

## Limitations and Restrictions

### Protected Words

These alias names are rejected:

| Word | Reason |
|------|--------|
| `clear` | Would conflict with the clear command |
| Same as command | `alias x x` would create infinite loop |

### Client Mode

When using the SneezyMUD client (`m_bIsClient` is true), in-game aliases are disabled (info.cc:3735-3740):

```cpp
if (desc->m_bIsClient) {
    sendTo("Use the client aliases. See client help file for #alias command...\n\r");
    return;
}
```

Client users should use the client's built-in alias system instead.

### Length Limits

| Field | Maximum Length |
|-------|---------------|
| Alias word | 50 characters |
| Command | 999 characters |

### No Recursive Expansion

Aliases do not expand recursively. If alias "a" expands to "b" and "b" is also an alias, only "a" expands:

```
alias a b target
alias b attack
"a goblin" -> "b target goblin" (NOT "attack target goblin")
```

This prevents infinite loops.

## Code References

| Function | File | Lines | Purpose |
|----------|------|-------|---------|
| `doAlias()` | info.cc | 3728-3786 | Create/view aliases |
| `doClear()` | info.cc | 3695-3726 | Delete aliases |
| `loadAliases()` | player_data.cc | 861-867 | Load from DB at login |
| `parseCommand()` | parse.cc | 1959-2021 | Alias expansion |
| Alias expansion | parse.cc | 1981-2020 | `%` and `~` processing |

## Common Gotchas

1. **Client mode disables aliases**: If using the SneezyMUD client, use `#alias` instead of in-game `alias`

2. **Clearing by number, not name**: Use `clear 3` (number from list), not `clear myalias`

3. **Order matters for `%`**: Only the first word is checked as an alias; `"cast cl Bob"` won't expand `cl`

4. **Multi-command timing**: Commands separated by `~` execute in sequence but with normal command queue processing between them

5. **Case sensitivity**: Alias lookups are case-sensitive; `CL` and `cl` are different aliases

6. **No nested aliases**: An alias that expands to another alias name won't trigger the second alias

## Related Documentation

- [Command Implementation](command-implementation.md) - How commands are parsed and dispatched
- [Network Architecture](network-architecture.md) - Descriptor and client handling
