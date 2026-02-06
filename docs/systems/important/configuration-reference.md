---
title: Configuration Reference
description: Runtime configuration system including sneezy.cfg parsing, port-based behavior switching, database connections, and feature toggles.
category: important
keywords: [config, runtime configuration, port-based behavior, feature toggles, database connection, sneezy.cfg]
primary_symbols:
  functions: [doConfiguration]
  classes: [Config, TDatabase]
  enums: [Config::Port::PROD, Config::Port::ALPHA, Config::Port::GAMMA]
---

## Overview

How does a MUD server adapt its behavior across development, testing, and production environments without code changes? This system manages all runtime configuration: network ports, database connections, feature toggles, economy tuning, and performance optimizations.

The configuration system uses a three-tier priority model. Command-line arguments override config file values, which override built-in defaults. This allows developers to quickly test changes via command line while maintaining stable production configurations in files.

Port number carries special significance beyond network binding. The production port (7900) enables stricter security measures, while alternate ports relax restrictions for development and testing. This port-based behavior switching eliminates the need for separate debug builds.

Configuration values are accessed through static accessor methods, providing a single point of truth that any part of the codebase can query. The global `gamePort` variable is the exception, exposed directly because port-based branching is so pervasive throughout the codebase.

---

## Patterns

### Configuration File Format

**Always use INI-style key = value syntax.** Comments start with `#`. Boolean values accept `0`/`false` or `1`/`true` (case-insensitive). Values like "yes", "no", "on", "off" are rejected. The file is parsed once at startup; changes require a server restart.

**Never include credentials in version control.** The example `sneezy.cfg` shows the format but should use placeholder passwords. Production credentials belong in deployment-specific configs outside the repository.

### Port-Based Behavior

**Always understand which features are port-dependent.** Many security and economy features only activate on production port (7900). Running tests on alternate ports avoids accidentally triggering multiplay bans, email notifications, or Twitter integration.

**Never assume development behavior matches production.** Features like experimental weapon wielding code, sleep tag, and the `testfight` command are intentionally enabled only on non-production ports.

### Database Configuration

**Always configure both databases.** The `sneezy` database holds game data while `immortal` holds builder/immortal data. Both must be accessible for full functionality.

**Allow username fallback for local development.** When no username is configured, the server passes NULL to the database connector. This enables passwordless local development via MySQL socket authentication without storing credentials.

### Feature Toggle Discipline

**Check accessor methods, not member variables.** The private members are implementation details; the public accessor methods are the interface. This encapsulation allows future changes like dynamic reloading.

**Document port-dependent toggles.** Some features (like multiplay enforcement) check both a toggle AND the port. A toggle set to true may still be inactive on non-PROD ports.

### Adding New Options

**Place options in the correct group.** Command-line-only options go in `cmdline`, options usable from both CLI and config file go in `config`, options only valid in config files go in `configOnly`. Database options have their own groups.

**Follow the accessor pattern.** Declare a private static member, define it in the .cc file, register it with Boost.Program_Options, and expose a public static accessor method. This maintains consistency across all options.

---

## Reference

### Symbol Quick Reference

| Symbol | Type | Purpose |
|--------|------|---------|
| `Config` | class | Static configuration container and accessors |
| `Config::Port::PROD` | constant | Production port value (7900) |
| `Config::Port::ALPHA` | constant | Alpha/test port value (6969) |
| `Config::Port::GAMMA` | constant | Gamma/trimmed port value (6961) |
| `gamePort` | global | Current port (used for port-based branching) |
| `doConfiguration()` | function | Parse and apply configuration |

### Command-Line Arguments

| Argument | Short | Config File | Description |
|----------|-------|-------------|-------------|
| `--help` | | No | Display help message |
| `--config` | `-c` | No | Specify config file path |
| `--lib` | `-l` | Yes | Data directory path |
| `--no_specials` | `-s` | Yes | Suppress special procedures |
| `--trimmed` | `-t` | Yes | Load as trimmed port |
| `--port` | `-p` | Yes | Network listen port (also accepts positional: `./sneezy 7900`) |

### Network Settings

| Option | Type | Default | Accessor | Description |
|--------|------|---------|----------|-------------|
| `port` | int | 7900 | `gamePort` global | Telnet listen port |

### Special Port Behaviors

| Port | Constant | Behaviors Enabled |
|------|----------|-------------------|
| 7900 | `PROD` | Multiplay enforcement, email notifications, Twitter posts, ignore disabled zones |
| 6969 | `ALPHA` | Relaxed multiplay restrictions |
| 6961 | `GAMMA` | Skip zonefile comments, load disabled zones |
| Other | - | Experimental weapon code, sleep tag, testfight command, corpse dissection objects |

### Data Directory

| Option | Type | Default | Accessor | Description |
|--------|------|---------|----------|-------------|
| `lib` | string | `lib` | `DataDir()` | Path to data directory |

### Directory Structure

| Subdirectory | Contents |
|--------------|----------|
| `zonefiles/` | Zone definitions |
| `mutable/` | Player data, corpses, rent (runtime writable) |
| `help/` | In-game help files |

### Database Settings

Database settings are stored in global vectors (`db_hosts`, `db_names`, `db_users`, `db_passwords`) indexed by database constant (`DB_SNEEZY`, `DB_IMMORTAL`).

| Option | Type | Default | Description |
|--------|------|---------|-------------|
| `sneezy_db` | string | `sneezy` | Game database name |
| `sneezy_host` | string | (empty) | Game database host |
| `sneezy_user` | string | (NULL) | Game database username |
| `sneezy_password` | string | (none) | Game database password |
| `immortal_db` | string | `immortal` | Immortal database name |
| `immortal_host` | string | (empty) | Immortal database host |
| `immortal_user` | string | (NULL) | Immortal database username |
| `immortal_password` | string | (none) | Immortal database password |

### Feature Toggles

| Option | Type | Default | Accessor | Description |
|--------|------|---------|----------|-------------|
| `no_specials` | bool | false | `NoSpecials()` | Suppress special procedure assignment |
| `trimmed` | bool | false | `bTrimmed()` | Load as trimmed port (forces GAMMA port) |
| `no_mail` | bool | false | `NoMail()` | Disable mail system |
| `load_on_death` | bool | true | `LoadOnDeath()` | Mobs drop gear on death instead of spawn |
| `throw_format_exceptions` | bool | true | `ThrowFormatExceptions()` | Throw on bad format strings (debug) |

### Multiplay Settings

| Option | Type | Default | Accessor | Description |
|--------|------|---------|----------|-------------|
| `check_multiplay` | bool | true | `CheckMultiplay()` | Enable multiplay detection |
| `force_multiplay_compliance` | bool | true | `ForceMultiplayCompliance()` | Disconnect violators |

### Item Damage Settings

| Option | Type | Default | Accessor | Description |
|--------|------|---------|----------|-------------|
| `item_damage_rate` | int | 1 | `ItemDamageRate()` | Minimum hit damage to risk item degradation |
| `weapon_dam_min_hardness` | int | 20 | `WeaponDamMinHardness()` | Minimum hardness for weapon damage |
| `weapon_dam_max_hardness` | int | 150 | `WeaponDamMaxHardness()` | Maximum hardness roll |
| `weapon_dam_max_sharp` | int | 150 | `WeaponDamMaxSharp()` | Maximum sharpness roll |

### Economy Settings

| Option | Type | Default | Accessor | Description |
|--------|------|---------|----------|-------------|
| `no_damaged_items_shop` | bool | false | `NoDamagedItemsShop()` | Shops refuse damaged items |
| `rent_tax` | bool | true | `RentTax()` | Tax rent at public hostels |

### Player File Management

| Option | Type | Default | Accessor | Description |
|--------|------|---------|----------|-------------|
| `auto_deletion` | bool | false | `AutoDeletion()` | Auto-purge inactive accounts (90+ days) |
| `rent_only_deletion` | bool | false | `RentOnlyDeletion()` | Only delete rent files, keep pfiles |
| `nuke_repair_items` | bool | true | `NukeRepairItems()` | Delete repair shop items after 180 days |

### Repossession Settings

| Option | Type | Default | Accessor | Description |
|--------|------|---------|----------|-------------|
| `repo_mobs` | bool | false | `RepoMobs()` | Enable repossession mobs for over-limit items |
| `super_repo_mobs` | bool | false | `SuperRepoMobs()` | Make repossession hunters significantly tougher |

### Performance Settings

| Option | Type | Default | Accessor | Description |
|--------|------|---------|----------|-------------|
| `nuke_inactive_mobs` | bool | false | `NukeInactiveMobs()` | Delete mobs in inactive zones (~50% CPU savings) |

### Option Groups

| Group | Scope | Examples |
|-------|-------|----------|
| `cmdline` | CLI only | `--help`, `--config` |
| `config` | CLI + config file | `port`, `lib`, `trimmed` |
| `configOnly` | Config file only | `item_damage_rate`, feature toggles |
| `database_sneezy` | CLI + config file | Sneezy database settings |
| `database_immortal` | CLI + config file | Immortal database settings |

### Precedence

| Priority | Source |
|----------|--------|
| 1 (highest) | Command-line arguments |
| 2 | Configuration file values |
| 3 (lowest) | Built-in defaults |

---

## Implementation

### Configuration Loading

At startup, `doConfiguration()` in configuration.cc defines all option groups using Boost.Program_Options. It first parses command-line arguments to find the config file path (defaulting to `sneezy.cfg`), then parses the config file, then applies defaults for any unspecified values.

Options are stored in private static members of the `Config` class. Each option has a corresponding public static accessor method. The accessor pattern provides encapsulation while maintaining simple usage throughout the codebase.

Positional argument handling allows the port to be specified without a flag: `./sneezy 7900` is equivalent to `./sneezy --port 7900`.

### Port Detection

The `gamePort` global is set during configuration parsing. Throughout the codebase, code checks `gamePort == Config::Port::PROD` (or ALPHA or GAMMA) to enable or disable features. This is more common than checking individual toggle flags because many features have implicit port dependencies.

The trimmed flag (`-t` or `trimmed = 1`) forces port to GAMMA regardless of what port is specified. This enables testing trimmed behavior on any port.

### Database Connection

Database settings configure the `TDatabase` connection class. The username defaults to NULL when no explicit username is configured via `getUser()`, which returns NULL when the stored value is empty.

Both databases (`sneezy` and `immortal`) are configured independently. They can be on different hosts with different credentials, though in typical setups they share the same connection parameters.

### Feature Toggle Evaluation

Boolean toggles are checked at their point of use via accessor calls like `Config::LoadOnDeath()`. The implementation simply returns the static member value. No caching or lazy evaluation occurs; values are set once at startup and remain constant.

Some toggles interact with port checks. For example, multiplay enforcement checks both `Config::CheckMultiplay()` AND `gamePort == Config::Port::PROD`. Both conditions must be true for enforcement to activate.

### Item Damage Rate

The `item_damage_rate` setting establishes a damage floor below which items cannot degrade. Hits dealing less than this amount have zero chance of damaging equipped items. Higher values reduce overall item degradation rates.

The weapon hardness and sharpness max values affect damage probability calculations. Higher maximums decrease the odds of weapon blunting or damage per swing.

### Auto-Deletion Timing

When `auto_deletion` is enabled, accounts inactive for 90+ days are eligible for removal. The check runs during the account cleanup cycle. If `rent_only_deletion` is true, only the rent file is deleted (preserving the character data). If false, the entire account including pfile and database records is removed.

### Repossession Mobs

When `repo_mobs` is enabled, items exceeding their `max_exist` limit cause bounty hunter mobs to spawn. These hunters seek out and attack players carrying over-limit items. The `super_repo_mobs` toggle makes these hunters significantly more powerful.

### Inactive Zone Optimization

The `nuke_inactive_mobs` toggle enables aggressive memory optimization. Mobs in zones with no player activity are deleted from memory. When a player enters the zone, it reloads from disk. This can reduce active mob count by approximately 50%, significantly improving mobile activity loop performance.

---

## Troubleshooting

### Config File Not Found

**Symptom:** Message at startup: `Failed to open config file 'sneezy.cfg'`

**Likely cause:** No config file exists in the working directory, or the path specified with `-c` is incorrect.

**Diagnostic approach:** Verify the file exists. Check if a custom path was specified. The server continues with defaults if the file is missing.

**Fix:** Create a config file in the working directory, or specify the correct path with `-c /path/to/config.cfg`.

### Config File Parsing Error

**Symptom:** Exception message during startup mentioning config file syntax.

**Likely cause:** Syntax errors in config file: missing equals signs, invalid boolean values (only 0/1/false/true accepted), or unrecognized option names.

**Diagnostic approach:** Check that option names exactly match registered identifiers including underscores. Verify boolean values use only 0, 1, false, or true.

**Fix:** Correct the syntax error. Refer to the example config file or Reference section for valid option names and value formats.

### Database Connection Failed

**Symptom:** Error at startup: `Could not connect to database 'sneezy'.`

**Likely cause:** MariaDB/MySQL is not running, database doesn't exist, credentials are wrong, or host is unreachable.

**Diagnostic approach:** Test connection manually with `mysql -h host -u user -p database`. Check that the database exists with `SHOW DATABASES`. Verify the service is running.

**Fix:** Ensure the database server is running, the database exists, and credentials are correct. For local development, try removing explicit user/password to use Unix socket authentication.

### Port Binding Failed

**Symptom:** Error at startup: `Address already in use`

**Likely cause:** Another process is already listening on the configured port.

**Diagnostic approach:** Check for existing processes with `netstat -tlnp | grep <port>` or `ss -tlnp | grep <port>`.

**Fix:** Stop the conflicting process, or change the port number in configuration.

### Feature Not Working On Test Port

**Symptom:** A feature works in production but not when running on development port.

**Likely cause:** The feature is port-dependent and only activates on `Config::Port::PROD` (7900).

**Diagnostic approach:** Search the codebase for the feature and check if it has port checks. Common port-dependent features: multiplay enforcement, email notifications, Twitter integration, corpse dissection.

**Fix:** For testing, either run on port 7900 (carefully), or temporarily modify the feature code to work on other ports. Some features are intentionally restricted to production for safety.

### Multiplay Enforcement Not Working

**Symptom:** Multiple characters from same IP can log in simultaneously despite configuration.

**Likely cause:** Either `check_multiplay` or `force_multiplay_compliance` is disabled, or the server is not running on PROD port.

**Diagnostic approach:** Verify both toggles are enabled in config. Check that `gamePort` equals 7900. Multiplay enforcement requires all three conditions: both toggles enabled AND PROD port.

**Fix:** Set both `check_multiplay = 1` and `force_multiplay_compliance = 1`, and run on port 7900.

### Item Damage Too High Or Too Low

**Symptom:** Items degrade too quickly (or never degrade).

**Likely cause:** `item_damage_rate` is set too low (causes frequent damage) or too high (prevents all damage).

**Diagnostic approach:** Check the configured `item_damage_rate`. A value of 1 means any hit has a chance to damage items. Higher values create a floor below which damage never occurs.

**Fix:** Adjust `item_damage_rate` to desired threshold. Higher values reduce item degradation. The weapon hardness/sharpness max values can also be increased to reduce weapon damage specifically.

### Auto-Deletion Purging Active Players

**Symptom:** Players complain their accounts were deleted despite recent activity.

**Likely cause:** Activity tracking is based on last login timestamp. Players who were renting but not logging in may be deleted if their last login exceeds 90 days.

**Diagnostic approach:** Check the account's last_logon field in the database. Compare against the 90-day threshold.

**Fix:** The 90-day threshold is hardcoded. To preserve accounts longer, disable `auto_deletion` or set `rent_only_deletion = 1` to only remove rent files while preserving character data.

### Help Text Missing Some Options

**Symptom:** Running `--help` doesn't show all configuration options.

**Likely cause:** This is intentional. Config-file-only options (in the `configOnly` group) and database credentials are excluded from help output.

**Diagnostic approach:** This is expected behavior, not an error.

**Fix:** Refer to the example config file or this documentation for the complete option listing.
