---
title: Configuration Reference
category: critical
keywords: [Config, sneezy.cfg, gamePort, DataDir, port, multiplay, ItemDamageRate, LoadOnDeath, NukeInactiveMobs, AutoDeletion, RepoMobs, database, performance]
related: [persistence-storage.md]
primary_symbols:
  functions: [doConfiguration]
  classes: [Config]
  files: [code/sys/configuration.h, code/sys/configuration.cc]
---

# Configuration Reference

## Overview

SneezyMUD uses Boost.Program_Options for configuration management. The system provides a three-tier hierarchy for specifying runtime behavior: command-line arguments take highest priority and override configuration file values, which in turn override built-in defaults. This allows flexible deployment scenarios where core settings persist in a file while ephemeral overrides occur at launch.

Configuration sources and their precedence: command-line arguments override config file values, which override built-in defaults. The default configuration file is `sneezy.cfg` in the working directory, customizable via the `--config` flag.

The configuration system controls network binding, database connections, data directory locations, feature toggles, economy tuning, performance optimizations, and multiplay enforcement. Many behaviors are port-dependent, with special semantics attached to production (7900), alpha (6969), and gamma (6961) ports.

Primary configuration files: `code/sys/configuration.h` declares options and accessors as static members of the Config class, `code/sys/configuration.cc` implements parsing and default values using Boost.Program_Options, and `sneezy.cfg` provides the example configuration template.

The configuration file uses INI-style format with key-value pairs. Comments begin with hash symbols. Boolean values accept either numeric (0/1) or textual (false/true) representations. Option names use underscores and must match the identifiers registered in `doConfiguration`.

Runtime access occurs through static methods on the Config class, following the pattern where each private static member has a corresponding public static accessor. The exception is the listen port, which uses the global variable `gamePort` directly due to widespread usage in port constant comparisons.

Configuration groups organize options by scope: `cmdline` group contains help and config file selection, `config` group holds options available on both command-line and config file, `configOnly` group restricts options to file-only specification, and separate `database_sneezy` and `database_immortal` groups manage database credentials.

Port values determine operational mode beyond just network binding. Production port (7900) enables multiplay enforcement, email notifications, Twitter integration, and corpse dissection objects. Alpha port (6969) relaxes multiplay restrictions for testing. Gamma port (6961) enables trimmed mode with zonefile comment skipping and disabled zone exclusion. All non-production ports activate experimental features like advanced weapon wielding, sleep tag game, and the testfight command.

Database configuration controls connections to two separate schemas: the sneezy database holds game state (rooms, objects, player data), while the immortal database stores builder and immortal-specific information. Username defaults to the current Unix user when unspecified, matching MySQL client behavior and enabling passwordless local development via Unix socket authentication.

Feature toggles control major subsystems. The `no_specials` flag suppresses special procedure assignment during boot. The `trimmed` flag activates gamma port behavior. The `no_mail` flag disables the mail system entirely. The `load_on_death` toggle determines whether mobs drop equipment on death versus at spawn time. The `throw_format_exceptions` flag controls whether invalid format strings raise exceptions or fail silently.

Multiplay settings enforce single-character-per-player restrictions when both `check_multiplay` and `force_multiplay_compliance` are enabled, preventing simultaneous login from the same account or IP address on production ports.

Item damage tuning affects equipment degradation. The `item_damage_rate` sets the damage threshold below which hits have no chance of damaging items. Weapon hardness and sharpness parameters control the probability of weapon blunting and damage through maximum roll values.

Economy options include shop rejection of damaged items via `no_damaged_items_shop` and rent taxation at public hostels via `rent_tax`.

Player file management handles account purging. Auto-deletion removes accounts inactive for 90+ days. The `rent_only_deletion` flag determines whether purging removes only rent files or entire accounts including pfiles and database records. Repair item nuking deletes items in repair shops after 180 days.

Repossession mobs spawn bounty hunters to reclaim items exceeding their `max_exist` limits. The `super_repo_mobs` option creates significantly tougher hunters for enforcement.

Performance optimization through `nuke_inactive_mobs` removes mobs from zones without player activity, potentially eliminating 50% of mobs and improving mobile activity loop performance.

## Patterns

Configuration addition follows a three-step pattern: declare the private static member and public accessor in configuration.h, define the static member and register it with Boost.Program_Options in configuration.cc within the appropriate options group, then access via the static method throughout the codebase.

Option group selection depends on access requirements. Command-line-only options like help and config file selection belong in the `cmdline` group. Options available both on command-line and in config files (port, lib directory, trimmed flag) belong in the `config` group. Options restricted to config file usage (most toggles and tuning parameters) belong in `configOnly`. Database credentials use dedicated `database_sneezy` and `database_immortal` groups.

Port constant comparisons use the `gamePort` global variable directly rather than through an accessor. Code checking for production behavior compares against `Config::Port::PROD`, alpha behavior against `Config::Port::ALPHA`, and gamma behavior against `Config::Port::GAMMA`.

Database connection patterns rely on separate credential sets for the two schemas. The sneezy database connection uses `Config::SneezySqlDb`, `Config::SneezySqlHost`, `Config::SneezySqlUser`, and `Config::SneezySqlPassword`. The immortal database uses the corresponding `Immortal` prefixed accessors.

Data directory access uses `Config::DataDir` to obtain the base path, which is then combined with subdirectories for zonefiles, mutable runtime data, and help files.

Boolean option usage follows the pattern of checking the accessor in conditional statements. The accessor method name uses PascalCase matching the option identifier with underscores removed. For example, `load_on_death` becomes `Config::LoadOnDeath()`.

Numeric tuning parameters typically have reasonable defaults that work for standard gameplay. Adjusting these values requires understanding their impact on game balance. Item damage rate affects equipment longevity, with higher values reducing overall degradation. Weapon hardness and sharpness maxima inversely affect damage probability through their role as divisors in probability calculations.

Feature toggle patterns often involve checking both a config flag and a port constant. Multiplay enforcement combines `Config::CheckMultiplay()`, `Config::ForceMultiplayCompliance()`, and `gamePort == Config::Port::PROD`. Experimental features check for non-production ports before activation.

Configuration file organization typically groups related settings with comment headers. Network settings appear first, followed by data directory, database credentials, feature toggles, economy tuning, and performance options. This logical grouping improves maintainability.

Adding database-backed configuration options requires coordination between the Config static accessors and database schema. Some runtime-modifiable settings persist in the database rather than config files, requiring different access patterns through TDatabase queries.

## Reference

Network configuration: `port` (int, default 7900, CLI `-p`) specifies the telnet listen port. Special values include 7900 for production mode, 6969 for alpha testing, and 6961 for gamma trimmed mode.

Data directory configuration: `lib` (string, default "lib", CLI `-l`) provides the path to the data directory containing zonefiles, mutable runtime data, and help files.

Sneezy database credentials: `sneezy_db` (string, default "sneezy") names the primary database, `sneezy_host` (string, default "localhost") specifies the host, `sneezy_user` (string, defaults to system user) provides authentication username, `sneezy_password` (string, no default) supplies the password.

Immortal database credentials: `immortal_db` (string, default "immortal") names the builder database, `immortal_host` (string, default "localhost") specifies the host, `immortal_user` (string, defaults to system user) provides authentication username, `immortal_password` (string, no default) supplies the password.

Feature toggles: `no_specials` (bool, default false, CLI `-s`, accessor `NoSpecials`) suppresses special procedure assignment, `trimmed` (bool, default false, CLI `-t`, accessor `bTrimmed`) activates gamma port mode, `no_mail` (bool, default false, accessor `NoMail`) disables mail system, `load_on_death` (bool, default true, accessor `LoadOnDeath`) controls mob equipment drop timing, `throw_format_exceptions` (bool, default true, accessor `ThrowFormatExceptions`) enables format string exception throwing.

Multiplay settings: `check_multiplay` (bool, default true, accessor `CheckMultiplay`) enables detection, `force_multiplay_compliance` (bool, default true, accessor `ForceMultiplayCompliance`) disconnects violators.

Item damage parameters: `item_damage_rate` (int, default 1, accessor `ItemDamageRate`) sets damage threshold, `weapon_dam_min_hardness` (int, default 20, accessor `WeaponDamMinHardness`) establishes minimum weapon hardness, `weapon_dam_max_hardness` (int, default 150, accessor `WeaponDamMaxHardness`) caps hardness rolls, `weapon_dam_max_sharp` (int, default 150, accessor `WeaponDamMaxSharp`) caps sharpness rolls.

Economy options: `no_damaged_items_shop` (bool, default false, accessor `NoDamagedItemsShop`) enables shop rejection of damaged items, `rent_tax` (bool, default true, accessor `RentTax`) activates rent taxation.

Player file management: `auto_deletion` (bool, default false, accessor `AutoDeletion`) enables automatic purging of inactive accounts, `rent_only_deletion` (bool, default false, accessor `RentOnlyDeletion`) restricts purging to rent files, `nuke_repair_items` (bool, default true, accessor `NukeRepairItems`) deletes old repair shop items.

Repossession system: `repo_mobs` (bool, default false, accessor `RepoMobs`) activates bounty hunters for over-limit items, `super_repo_mobs` (bool, default false, accessor `SuperRepoMobs`) creates buffed hunters.

Performance optimization: `nuke_inactive_mobs` (bool, default false, accessor `NukeInactiveMobs`) removes mobs from inactive zones.

Command-line-only options: `--help` produces help message, `-c` or `--config` specifies alternate configuration file path. Port can be specified as positional argument without flag.

Port constants: `Config::Port::PROD` equals 7900, `Config::Port::ALPHA` equals 6969, `Config::Port::GAMMA` equals 6961.

Global variable: `gamePort` holds the current listen port value, accessible without qualification throughout the codebase.

Static accessor pattern: all configuration values accessed through `Config::MethodName()` static methods, with method names using PascalCase transformation of option identifiers.

## Implementation

The `doConfiguration` function in configuration.cc orchestrates option parsing. It creates separate options_description objects for each group, registers options with their types and defaults, combines groups into a visible set for help text and a full set for parsing, processes command-line arguments through the visible set, and parses the config file through the full set.

Option registration uses the Boost.Program_Options fluent interface. The `add_options()` method chains calls specifying option name with optional short form, value semantic specifying type and storage location via `po::value<T>(&variable)->default_value(val)`, and description string for help text.

Configuration parsing occurs in two phases: command-line parsing uses `po::command_line_parser` with `options(visible).positional(positional).run()` to handle both flagged and positional arguments, then config file parsing uses `po::parse_config_file` with the full options set to read the INI format file. Results populate a `variables_map` from which `po::notify()` stores values into the static members.

Error handling for missing config files continues execution with default values after logging a failure message. Database connection errors halt server startup after logging diagnostic information. Invalid option values throw exceptions that terminate startup with error messages.

The Config class structure declares all options as private static members of primitive or string type. Public static accessor methods return these values by value for primitives or by const reference for strings. No instance methods exist since the class serves purely as a namespace for static configuration state.

Static member initialization occurs in configuration.cc where each private static member is defined at file scope. Boost.Program_Options populates these during the notify call after parsing completes.

Port constant definition uses an enum nested within Config::Port. The enum is unscoped for backward compatibility but lives within the Port namespace for organizational clarity. These constants appear in comparisons throughout the codebase to determine operational mode.

Data directory path construction combines the base directory from `Config::DataDir()` with hardcoded subdirectory names. The zonefiles subdirectory contains zone definitions, the mutable subdirectory holds player data and corpses, and the help subdirectory stores in-game documentation files.

Database username defaulting uses conditional logic during connection initialization. When the username accessor returns empty string, the connection code retrieves the current Unix username and uses that for authentication, matching MySQL client behavior for Unix socket connections.

Boolean option parsing accepts multiple input formats. Numeric values 0 and 1 map to false and true. Textual values "false" and "true" undergo case-insensitive matching. Invalid values throw exceptions during parsing.

Help text generation concatenates the visible options into a formatted output showing option names, short forms, types, and descriptions. This output appears when the `--help` flag is specified.

Positional argument handling for the port parameter uses a positional_options_description that maps the first unnamed argument to the "port" option name. This allows `./sneezy 7900` as equivalent to `./sneezy --port 7900`.

Option groups maintain separation between command-line-only options that should not appear in config files and config-file-only options that should not appear in help text. The visible set used for help and CLI parsing includes cmdline and config groups. The full set used for config file parsing includes all groups.

Database group registration creates separate `database_sneezy` and `database_immortal` options_description objects to organize credentials. These groups get added to the full set but not the visible set, keeping database passwords out of help output while allowing config file specification.

Configuration precedence implementation relies on Boost.Program_Options parsing order. Command-line parsing occurs first and populates the variables_map. Config file parsing occurs second and only populates values not already present from command-line. This automatically implements the precedence rule without explicit code.

## Troubleshooting

Missing config file errors appear as "Failed to open config file 'sneezy.cfg'" but allow server startup with defaults. Specify an alternate config file path using `-c /path/to/config` when the default location is unavailable or create a config file in the working directory.

Database connection failures manifest as "Could not connect to database 'sneezy'" and halt server startup. Verify MariaDB or MySQL is running through systemctl status or service status commands. Confirm database existence using `mysql -e "SHOW DATABASES"`. Check user credentials and access permissions. Ensure hostname is correct, using "localhost" for local connections to enable Unix socket authentication.

Feature toggle not taking effect often results from port-dependent behavior. Many features require production port (7900) for activation. Running on alpha (6969) or gamma (6961) ports may disable features for development safety. Check code for port constant comparisons alongside the feature flag.

Boolean option parsing errors occur when using values other than 0, 1, false, or true. The parser rejects "yes", "no", "on", "off" and similar variations. Use numeric or boolean literals exclusively.

Multiplay enforcement not working requires both `check_multiplay` and `force_multiplay_compliance` enabled and running on production port. Alpha port intentionally relaxes restrictions. Verify both config values and port number.

Item damage rate seeming inverted results from the threshold interpretation. Higher values mean less damage because hits below the threshold cannot damage items. Lower values increase overall degradation by allowing more hits to trigger damage checks.

Auto-deletion not purging accounts requires the `auto_deletion` flag enabled. The 90-day inactivity period is hardcoded and not configurable. Check that the deletion cron job or scheduled task is running. Verify `rent_only_deletion` matches intended behavior for full account removal versus rent-only deletion.

Inactive mob nuking not reducing memory requires the `nuke_inactive_mobs` flag enabled and zones actually being inactive (no player presence for extended period). The performance benefit appears gradually as zones become inactive. Freshly active zones retain all mobs.

Repossession mobs not spawning requires both `repo_mobs` enabled and items actually exceeding their `max_exist` limits. The system does not create hunters for items within limits. Verify the item's max_exist value in its object definition.

Database username authentication failing despite correct password may result from missing username in config. Explicitly specify the username rather than relying on Unix user default when database user differs from system user.

Port binding failures appear as "Address already in use" and indicate another process owns the port. Check for existing server instances using `netstat -tlnp | grep <port>` or change the port number in configuration.

Config file parsing errors produce exception messages during startup. Check for syntax errors like missing equals signs, invalid boolean values, or unrecognized option names. Option names must exactly match registered identifiers including underscores.

Help text not showing config-file-only options is intentional behavior. These options appear only in the full set used for file parsing, not the visible set used for help generation. Refer to example config file or this documentation for complete option listing.

Performance tuning requires understanding workload characteristics. Item damage rates affect equipment economy and player experience. Inactive mob nuking trades zone persistence for CPU performance. Adjust based on server population and usage patterns rather than blindly optimizing metrics.
