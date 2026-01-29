---
title: Logging and Error Handling Patterns
description: SneezyMUD uses a centralized logging system for error reporting, debugging, and player activity tracking with categorized log messages and per-immortal severity filtering.
keywords:
  - vlogf
  - logTypeT
  - mud_assert
  - LOG_BUG
  - LOG_SILENT
  - setsev
  - SystemLogComm
  - vlogf_trace
  - severity filtering
  - LOG_PIO
  - LOG_COMBAT
  - log categories
  - immortal logging
  - error reporting
  - assertion failures
category: Critical Systems

  - command-implementation.md
  - debugging-tools.md
last_updated: 2026-01-29
source_files:
  - code/code/misc/log.h
  - code/code/misc/utility.cc
  - code/code/sys/handler.cc
  - code/code/misc/immortal.cc
  - code/code/misc/wiz_data.cc
  - code/code/sys/signals.cc
  - code/code/sys/comm.h
related: [memory-safety.md]
---

# Logging and Error Handling Patterns

SneezyMUD uses a centralized logging system for error reporting, debugging, and player activity tracking. Understanding this system is essential for debugging issues and maintaining the codebase.

## Overview

The logging system provides:
- Categorized log messages via `logTypeT` enum
- Output to stderr (always) with optional echoing to online immortals
- Per-immortal severity filtering via the `setsev` command
- Stack trace support via `vlogf_trace()`
- Custom assertion mechanism via `mud_assert()`
- Per-player logging for immortals

## Core Functions

### vlogf()

The primary logging function. Writes to stderr and optionally echoes to immortals based on their severity settings.

```cpp
void vlogf(logTypeT tError, const sstring& errorMsg);
```

**Usage:**
```cpp
vlogf(LOG_BUG, "Something went wrong");
vlogf(LOG_BUG, format("Player %s attempted invalid action in room %d") % ch->getName() % room);
```

**Source:** `code/code/misc/utility.cc:724-771`

### vlogf_trace()

Logs a message with a stack trace appended. Uses `backtrace()` on glibc systems.

```cpp
void vlogf_trace(logTypeT tError, const sstring& errorMsg);
```

**Usage:**
```cpp
vlogf_trace(LOG_BUG, "Error: stop called on BaseRepair - needs override");
```

**Source:** `code/code/misc/utility.cc:506-524`

### TPerson::logf()

Per-immortal logging to individual log files. Only active for immortals with logging enabled.

```cpp
void TPerson::logf(const char* tString, ...);
```

**Source:** `code/code/misc/utility.cc:484-504`

## Log Categories

The `logTypeT` enum defines all log categories. Categories are used both for filtering and for identifying the source/nature of log messages.

**Source:** `code/code/misc/log.h`

### Special Values

| Constant | Value | Description |
|----------|-------|-------------|
| `LOG_SILENT` | -2 | Recorded but NOT echoed to immortals (anti-spam) |
| `LOG_NONE` | -1 | Empty/unused |

### Standard Categories

| Constant | Value | Description | Use When |
|----------|-------|-------------|----------|
| `LOG_MISC` | 0 | Miscellaneous | Anything not fitting other categories |
| `LOG_LOW` | 1 | L.O.W. errors | Low-level errors, builder issues |
| `LOG_FILE` | 2 | File I/O errors | File open/read/write failures |
| `LOG_BUG` | 3 | Bugs and reports | Code bugs, assertion failures |
| `LOG_PROC` | 4 | Procedure errors | Mob/obj/room spec proc issues |
| `LOG_PIO` | 5 | Player I/O | Player login/logout events |
| `LOG_IIO` | 6 | Immortal I/O | Immortal login/logout, wiz file loading |
| `LOG_CLIENT` | 7 | Client errors | SneezyMUD client protocol issues |
| `LOG_COMBAT` | 8 | Combat errors | Combat system bugs |
| `LOG_CHEAT` | 9 | Cheating logs | Detected cheating attempts |
| `LOG_FACT` | 10 | Faction | Faction system issues |
| `LOG_DB` | 11 | Database | Database query issues |

### Mobile Categories

| Constant | Value | Description |
|----------|-------|-------------|
| `LOG_MOB` | 15 | Generic mobile errors |
| `LOG_MOB_AI` | 16 | Mobile AI/logic errors |
| `LOG_MOB_RS` | 17 | Mobile response script errors |

### Object and Editor Categories

| Constant | Value | Description |
|----------|-------|-------------|
| `LOG_OBJ` | 18 | Generic object errors |
| `LOG_EDIT` | 21 | Editor (oedit/medit/redit) errors |

### Personal Developer Logs (23-31)

These are personal log channels visible only to specific developers:

| Constant | Value | Developer |
|----------|-------|-----------|
| `LOG_JESUS` | 23 | Jesus |
| `LOG_BATOPR` | 24 | Batopr |
| `LOG_BRUTIUS` | 25 | Brutius |
| `LOG_COSMO` | 26 | Cosmo |
| `LOG_MAROR` | 27 | Maror |
| `LOG_PEEL` | 28 | Peel |
| `LOG_LAPSOS` | 29 | Lapsos |
| `LOG_DASH` | 30 | Dash |
| `LOG_ANGUS` | 31 | Angus |

**Note:** `LOG_MAX = 23` marks the boundary between standard categories and personal logs. Personal logs require the developer's character name to view.

## mud_assert()

Custom assertion macro that logs to `LOG_BUG` and calls `abort()` on failure. Unlike standard `assert()`, it always executes (not disabled in release builds) and provides formatted error messages.

```cpp
void mud_assert(int condition, const char* errorMsg, ...);
```

**Source:** `code/code/sys/handler.cc:2465-2478`

### When to Use mud_assert()

Use `mud_assert()` for invariant violations that indicate programmer error and cannot be safely recovered from:

```cpp
// Pointer validity
mud_assert(t != NULL, "canSee with NULL t");
mud_assert(race != NULL, "No race in getVolume()");

// Array bounds
mud_assert(i >= MIN_WEAR && i < MAX_WEAR, "Bad limb slot, %s %d", getName().c_str(), i);

// State invariants
mud_assert(discs != NULL, "assignDisc(): discs was null after new");

// Bidirectional relationship consistency
mud_assert(t.parent == NULL, "TThing += : t.parent existed. item: %s", t.name.c_str());
```

### Behavior

1. If `condition` is true (non-zero), returns immediately
2. If `condition` is false (zero):
   - Formats the error message with any variadic arguments
   - Logs "ASSERTION FAILED: {message}" to `LOG_BUG`
   - Calls `abort()` to crash with a core dump

### Anti-Patterns

```cpp
// WRONG: mud_assert for recoverable conditions
mud_assert(victim != NULL, "No victim");  // Should check and return instead

// WRONG: Side effects in condition
mud_assert(doSomething(), "Failed");  // Side effect may be skipped

// RIGHT: Check and handle gracefully when possible
if (victim == NULL) {
    vlogf(LOG_BUG, "No victim in someFunction");
    return FALSE;
}
```

## Severity Filtering System

Immortals can filter which log categories they see in real-time using the `setsev` command.

### How It Works

1. Each `Descriptor` has a `severity` bitmask field
2. When `vlogf()` is called with `tError >= 0`, it iterates online immortals
3. Only immortals with `POWER_SETSEV` and the corresponding bit set in `severity` receive the message
4. Messages are delivered via `SystemLogComm` objects pushed to the output queue

### The setsev Command

```
setsev           - Show current settings
setsev misc      - Toggle LOG_MISC
setsev combat    - Toggle LOG_COMBAT
setsev <name>    - Toggle personal log (developer only)
```

**Required Power:** `POWER_SETSEV` (and `POWER_SETSEV_IMM` for non-LOW categories)

**Source:** `code/code/misc/immortal.cc:4125-4251`

### Persistence

Severity settings are saved in the `wizdata` database table and restored on login via `wizFileRead()`.

## Log Output Format

### stderr Format

All logs are written to stderr with a timestamp:

```
YYYY|MMDD|HH:MM:SS :: Category: Message
```

Example:
```
2024|0115|14:23:45 :: BUG: Player Gandalf attempted to move to invalid room -1
```

### In-Game Format

Immortals see logs prefixed with `//`:

```
// BUG: Player Gandalf attempted to move to invalid room -1
```

## Log File Locations

| Type | Location | Description |
|------|----------|-------------|
| Server stdout/stderr | Console or redirect | All vlogf() output |
| Immortal personal | `lib/mutable/immortals/{name}/logfile` | Per-immortal activity log |

### Immortal Personal Logs

When an immortal logs in, if `should_be_logged()` returns true, a personal log file is opened:

```cpp
tString = format("mutable/immortals/%s/logfile") % name;
tPerson->tLogFile = fopen(tString.c_str(), "a");
```

Commands and actions can be logged via `TPerson::logf()`.

## Common Logging Patterns

### Error with Context

```cpp
vlogf(LOG_BUG, format("Invalid vnum %d in function %s") % vnum % __func__);
```

### Player Action Logging

```cpp
vlogf(LOG_PIO, format("%s logged in from %s") % ch->getName() % ch->desc->host);
```

### Silent Logging (No Immortal Echo)

Use `LOG_SILENT` for high-frequency events that would spam immortals:

```cpp
vlogf(LOG_SILENT, format("%s talens changed by %i.") % getName() % money);
vlogf(LOG_SILENT, format("%s ordering %s to '%s' at %d") % ch->getName() % victim->getName() % cmd % room);
```

### Conditional Debug Logging

For developer-specific debugging, use personal log channels:

```cpp
// Only Dash sees this
vlogf(LOG_DASH, format("Debug: variable x = %d") % x);
```

### Logging with Stack Trace

For complex bugs where call context matters:

```cpp
vlogf_trace(LOG_BUG, format("Unexpected state in %s") % __func__);
```

## Performance Considerations

### Avoid in Hot Paths

`vlogf()` performs:
- `time()` syscall
- `localtime()` conversion
- String formatting
- `fprintf()` to stderr
- Iteration over all descriptors (if `tError >= 0`)
- String formatting for each immortal

**Do not call in tight loops or frequently-executed code paths.**

### Use LOG_SILENT for Frequent Events

`LOG_SILENT` (value -2) skips the immortal notification loop:

```cpp
// Logged to stderr only, no immortal spam
vlogf(LOG_SILENT, format("Component spawn at room %d") % room);
```

### Conditional Logging

For expensive-to-compute log messages, check first:

```cpp
// Avoid computing message if no one will see it
if (isDebugMode) {
    vlogf(LOG_DEBUG, format("Expensive computation: %s") % computeExpensiveString());
}
```

## Adding New Log Categories

1. **Add enum value** in `code/code/misc/log.h`:
```cpp
LOG_MYFEATURE = 12,  // After LOG_DB, before LOG_MOB
```

2. **Update LOG_MAX** if adding before personal logs (currently 23)

3. **Add display name** in `getLogType()` (`code/code/misc/utility.cc:526-623`):
```cpp
case LOG_MYFEATURE:
    buf = "MyFeature";
    break;
```

4. **Add to setsev** in `doSetsev()` (`code/code/misc/immortal.cc:4125-4251`):
   - Add to `tFields[]` array
   - Add to `tHelp[]` array

5. **Update severity bit handling** if needed for special permissions

## Error Reporting Best Practices

### Include Context

```cpp
// BAD: No context
vlogf(LOG_BUG, "Invalid value");

// GOOD: Full context
vlogf(LOG_BUG, format("Invalid damage value %d for %s attacking %s in room %d")
               % damage % attacker->getName() % victim->getName() % room);
```

### Use Appropriate Categories

| Situation | Category |
|-----------|----------|
| Code bug, logic error | `LOG_BUG` |
| File read/write failure | `LOG_FILE` |
| Database issue | `LOG_DB` |
| Spec proc error | `LOG_PROC` |
| Player login/logout | `LOG_PIO` |
| Suspected cheating | `LOG_CHEAT` |
| High-frequency debug | `LOG_SILENT` or personal |

### Don't Log Secrets

Never log passwords, tokens, or sensitive player data:

```cpp
// WRONG
vlogf(LOG_PIO, format("Login attempt: %s with password %s") % name % password);

// RIGHT
vlogf(LOG_PIO, format("Login attempt: %s") % name);
```

### Recoverable vs Fatal

```cpp
// Recoverable: log and continue
if (target == NULL) {
    vlogf(LOG_BUG, "Null target in doAttack, aborting attack");
    return FALSE;
}

// Fatal invariant violation: assert and crash
mud_assert(this->roomp != NULL, "Being with no room pointer");
```

## Signal Handlers

The server installs signal handlers that use logging:

| Signal | Handler | Action |
|--------|---------|--------|
| `SIGALRM` | `logsig()` | Logs "Signal received. Ignoring." |
| `SIGHUP/SIGINT/SIGTERM` | `hupsig()` | Logs shutdown message, exits |
| `SIGVTALRM` | `checkpointing()` | Calls `mud_assert()` if deadlocked |
| `SIGUSR1/SIGUSR2` | `purgeRequest()` | Logs purge/shutdown request |

**Source:** `code/code/sys/signals.cc`

## Related Files

| File | Contents |
|------|----------|
| `code/code/misc/log.h` | `logTypeT` enum, `vlogf()` declaration |
| `code/code/misc/utility.cc` | `vlogf()`, `vlogf_trace()`, `getLogType()` implementations |
| `code/code/sys/handler.cc` | `mud_assert()` implementation |
| `code/code/misc/immortal.cc` | `doSetsev()` command |
| `code/code/misc/wiz_data.cc` | Immortal log file setup, severity persistence |
| `code/code/sys/signals.cc` | Signal handlers |
| `code/code/sys/comm.h` | `SystemLogComm` class |
