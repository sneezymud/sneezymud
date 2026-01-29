---
title: Configuration Reference
description: SneezyMUD configuration system using Boost.Program_Options, covering command-line arguments, config file format, network settings, database connections, feature toggles, and performance optimization options.
keywords:
  - Config
  - sneezy.cfg
  - gamePort
  - DataDir
  - TDatabase
  - port
  - CheckMultiplay
  - ItemDamageRate
  - LoadOnDeath
  - NukeInactiveMobs
  - AutoDeletion
  - RepoMobs
  - throw_format_exceptions
  - configuration.h
category: Critical Systems

last_updated: 2026-01-29
source_files:
  - code/code/sys/configuration.h
  - code/code/sys/configuration.cc
  - code/sneezy.cfg
related: [persistence-storage.md]
---

# Configuration Reference

This document describes the SneezyMUD configuration system, including all available options, their defaults, and how to customize server behavior.

## Overview

 SneezyMUD uses [Boost.Program_Options](https://www.boost.org/doc/libs/release/libs/program_options/) for configuration. Options can be specified via:

1. **Command-line arguments** - Highest priority, override config file
2. **Configuration file** - Default: `sneezy.cfg` in the working directory
3. **Built-in defaults** - Used when no value is specified

**Source files:**
- `code/code/sys/configuration.h` - Option declarations and accessors
- `code/code/sys/configuration.cc` - Option parsing and defaults
- `code/sneezy.cfg` - Example configuration file

## Configuration File Format

The configuration file uses INI-style format with `key = value` pairs:

```ini
# Comments start with #
port = 7900
lib = ../lib

# Boolean values: 0/false or 1/true
throw_format_exceptions = 0

# Database settings
sneezy_db = sneezy
sneezy_host = localhost
```

## Command-Line Arguments

```
Usage: sneezy [options] [port]

Command line only:
  --help                 Produce help message
  -c [ --config ] arg    Configuration file to use (default: sneezy.cfg)

Configuration + Command line:
  -l [ --lib ] arg       Data directory to run in (default: lib)
  -s [ --no_specials ]   Suppress assignment of special routines
  -t [ --trimmed ]       Load as trimmed port
  -p [ --port ] arg      Game port (default: 7900)
```

The port can also be specified as a positional argument: `./sneezy 7900`

## Configuration Options

### Network Settings

| Option | Type | Default | CLI | Description |
|--------|------|---------|-----|-------------|
| `port` | int | 7900 | `-p` | Telnet listen port |

**Special Port Values:**

| Port | Constant | Behavior |
|------|----------|----------|
| 7900 | `Config::Port::PROD` | Production mode - enables multiplay enforcement, email notifications, Twitter integration |
| 6969 | `Config::Port::ALPHA` | Alpha/test mode - relaxes multiplay restrictions |
| 6961 | `Config::Port::GAMMA` | Gamma/trimmed mode - skips zonefile comments, disables disabled zones |

Port-specific behaviors include:
- **PROD only:** Multiplay limit enforcement, email storage bag notifications, Twitter posts, corpse dissection objects
- **Non-PROD:** Experimental weapon wielding code, sleep tag game enabled, `testfight` command available
- **GAMMA:** Zonefile comment lines skipped during parsing

### Data Directory

| Option | Type | Default | CLI | Description |
|--------|------|---------|-----|-------------|
| `lib` | string | `lib` | `-l` | Path to the `lib/` data directory |

The data directory contains:
- `zonefiles/` - Zone definitions
- `mutable/` - Player data, corpses, rent (writable at runtime)
- `help/` - In-game help files

### Database Settings

Two databases are used: `sneezy` (game data) and `immortal` (builder/immortal data).

| Option | Type | Default | Description |
|--------|------|---------|-------------|
| `sneezy_db` | string | `sneezy` | Name of the sneezy database |
| `sneezy_host` | string | `localhost` | Host for the sneezy database |
| `sneezy_user` | string | (system user) | Username for sneezy database |
| `sneezy_password` | string | (none) | Password for sneezy database |
| `immortal_db` | string | `immortal` | Name of the immortal database |
| `immortal_host` | string | `localhost` | Host for the immortal database |
| `immortal_user` | string | (system user) | Username for immortal database |
| `immortal_password` | string | (none) | Password for immortal database |

**Username fallback:** When no username is configured, the server uses the current Unix user (matching MySQL CLI behavior). This allows passwordless local development via Unix socket authentication.

### Feature Toggles

| Option | Type | Default | Accessor | Description |
|--------|------|---------|----------|-------------|
| `no_specials` | bool | false | `NoSpecials()` | Suppress special procedure assignment |
| `trimmed` | bool | false | `bTrimmed()` | Load as trimmed port (sets port to GAMMA) |
| `no_mail` | bool | false | `NoMail()` | Disable the mail system |
| `load_on_death` | bool | true | `LoadOnDeath()` | Mobs drop gear on death instead of spawn |
| `throw_format_exceptions` | bool | true | `ThrowFormatExceptions()` | Throw on bad format strings (debug aid) |

### Multiplay Settings

| Option | Type | Default | Accessor | Description |
|--------|------|---------|----------|-------------|
| `check_multiplay` | bool | true | `CheckMultiplay()` | Enable multiplay detection |
| `force_multiplay_compliance` | bool | true | `ForceMultiplayCompliance()` | Disconnect violators |

When both are enabled, players cannot have multiple characters from the same account or IP address logged in simultaneously (on PROD port).

### Item Damage System

| Option | Type | Default | Accessor | Description |
|--------|------|---------|----------|-------------|
| `item_damage_rate` | int | 1 | `ItemDamageRate()` | Damage threshold for item degradation |
| `weapon_dam_min_hardness` | int | 20 | `WeaponDamMinHardness()` | Minimum hardness for weapon damage |
| `weapon_dam_max_hardness` | int | 150 | `WeaponDamMaxHardness()` | Maximum hardness roll |
| `weapon_dam_max_sharp` | int | 150 | `WeaponDamMaxSharp()` | Maximum sharpness roll |

**How it works:**
- `item_damage_rate`: Hits dealing less than this amount have no chance of damaging items. Higher values reduce overall item damage.
- Weapon hardness/sharpness: Higher max values decrease weapon blunting/damage probability.

### Economy and Shops

| Option | Type | Default | Accessor | Description |
|--------|------|---------|----------|-------------|
| `no_damaged_items_shop` | bool | false | `NoDamagedItemsShop()` | Shops refuse damaged items |
| `rent_tax` | bool | true | `RentTax()` | Tax rent at public hostels |

### Player File Management

| Option | Type | Default | Accessor | Description |
|--------|------|---------|----------|-------------|
| `auto_deletion` | bool | false | `AutoDeletion()` | Auto-purge inactive accounts |
| `rent_only_deletion` | bool | false | `RentOnlyDeletion()` | Only delete rent files, keep pfiles |
| `nuke_repair_items` | bool | true | `NukeRepairItems()` | Delete repair shop items after 180 days |

**Auto-deletion behavior:**
- Accounts inactive for 90+ days are deleted
- If `rent_only_deletion` is true, only the rent file is deleted
- If false, the entire account (pfile, rent, account record) is removed

### Repossession System

| Option | Type | Default | Accessor | Description |
|--------|------|---------|----------|-------------|
| `repo_mobs` | bool | false | `RepoMobs()` | Enable repossession mobs |
| `super_repo_mobs` | bool | false | `SuperRepoMobs()` | Buffed hunters for over-limit items |

When enabled, items exceeding their `max_exist` limit spawn bounty hunters to reclaim them. `SuperRepoMobs` makes these hunters significantly tougher.

### Performance Optimization

| Option | Type | Default | Accessor | Description |
|--------|------|---------|----------|-------------|
| `nuke_inactive_mobs` | bool | false | `NukeInactiveMobs()` | Delete mobs in inactive zones |

When enabled, mobs in zones without player activity are removed from memory. This can eliminate ~50% of mobs and significantly improve CPU performance in the mobile activity loop.

## Example Configuration

```ini
## SneezyMUD Production Configuration

# Network
port = 7900

# Data directory
lib = ../lib

# Database connections
sneezy_db = sneezy
sneezy_host = localhost
sneezy_user = sneezy
sneezy_password = secretpassword

immortal_db = immortal
immortal_host = localhost
immortal_user = sneezy
immortal_password = secretpassword

# Disable debug exceptions for stability
throw_format_exceptions = 0

# Economy tuning
item_damage_rate = 50
rent_tax = 1
no_damaged_items_shop = 0

# Multiplay enforcement
check_multiplay = 1
force_multiplay_compliance = 1

# Performance (production)
nuke_inactive_mobs = 0
load_on_death = 1

# Maintenance
auto_deletion = 0
nuke_repair_items = 1
```

## Adding New Configuration Options

### Step 1: Declare in configuration.h

```cpp
class Config {
  private:
    static bool my_new_option;  // Add private member

  public:
    static bool MyNewOption() { return my_new_option; }  // Add accessor
};
```

### Step 2: Define and Register in configuration.cc

```cpp
// Define static member
bool Config::my_new_option;

// In doConfiguration(), add to appropriate options group:
configOnly.add_options()
  ("my_new_option",
    po::value<bool>(&my_new_option)->default_value(false),
    "Description of what this option does");
```

### Step 3: Use in Code

```cpp
#include "configuration.h"

if (Config::MyNewOption()) {
    // Feature-specific behavior
}
```

### Option Placement Guidelines

| Group | Scope | Example Options |
|-------|-------|-----------------|
| `cmdline` | Command-line only | `--help`, `--config` |
| `config` | CLI + config file | `port`, `lib`, `trimmed` |
| `configOnly` | Config file only | `item_damage_rate`, all toggles |
| `database_*` | CLI + config file | All database settings |

## Precedence Rules

1. Command-line arguments override everything
2. Config file values override defaults
3. Built-in defaults are used when no value is specified

For boolean options in the config file, use `0`/`1` or `false`/`true`.

## Runtime Access

All configuration values are accessed via static methods on the `Config` class:

```cpp
#include "configuration.h"

int port = gamePort;                          // Global variable for port
sstring dataDir = Config::DataDir();          // Data directory
bool loadOnDeath = Config::LoadOnDeath();     // Feature toggle
int damageRate = Config::ItemDamageRate();    // Numeric setting
```

The `gamePort` global is used directly because many parts of the codebase compare against `Config::Port::PROD`, `Config::Port::ALPHA`, and `Config::Port::GAMMA` constants.

## Troubleshooting

### Config file not found

```
Failed to open config file 'sneezy.cfg'
```

The server continues with defaults. Create a config file or specify one with `-c`:

```bash
./sneezy -c /path/to/my.cfg
```

### Database connection failed

```
Could not connect to database 'sneezy'.
```

Verify:
1. MariaDB/MySQL is running
2. Database exists: `mysql -e "SHOW DATABASES"`
3. User has access: Check `sneezy_user` and `sneezy_password`
4. Host is correct: Use `localhost` for local connections

### Feature not working as expected

Many features are port-dependent. Check if the feature requires `Config::Port::PROD` (7900). Running on a different port may disable certain behaviors for safety during development.

## Related Documentation

- [Database Queries](database-queries.md) - How to interact with databases
- [README](../README.md) - Build and setup instructions
