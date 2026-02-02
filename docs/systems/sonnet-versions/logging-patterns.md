---
title: Logging and Error Handling Patterns
category: critical
keywords: [vlogf, logTypeT, mud_assert, LOG_BUG, LOG_SILENT, setsev, severity_filtering, stack_trace]
related: [memory-safety.md, command-implementation.md]
primary_symbols:
  functions: [vlogf, vlogf_trace, mud_assert, doSetsev, getLogType]
  classes: [SystemLogComm, TPerson]
  files: [code/code/misc/log.h, code/code/misc/utility.cc, code/code/sys/handler.cc]
---

# Logging and Error Handling Patterns

## Overview

SneezyMUD's logging system provides centralized error reporting, debugging, and activity tracking through categorized log messages with runtime filtering. All logs write to stderr with timestamps, and immortals receive real-time notifications based on their configured severity settings.

The system serves three primary purposes: recording server events for post-mortem analysis, providing real-time visibility to online administrators, and capturing per-immortal activity logs for accountability. Categories range from file I/O errors and combat bugs to player login events and developer-specific debug channels.

The custom assertion mechanism complements the logging system by enforcing invariants that indicate programmer error. Unlike standard assertions, these always execute and provide formatted error context before crashing with core dumps for debugging.

Signal handlers integrate with the logging system to record shutdown requests, ignore spurious alarms, and detect deadlocks during checkpointing.

## Patterns

### Basic Logging

Call vlogf with a category and message. Messages write to stderr always, and echo to immortals when the category value is non-negative and matches their severity filter.

```cpp
vlogf(LOG_BUG, "Something went wrong");
vlogf(LOG_BUG, format("Player %s attempted invalid action in room %d") % ch->getName() % room);
```

### Silent Logging

Use LOG_SILENT for high-frequency events that should be recorded but not echoed to immortals. This category has the special value -2 which skips the descriptor iteration loop entirely.

```cpp
vlogf(LOG_SILENT, format("%s talens changed by %i.") % getName() % money);
vlogf(LOG_SILENT, format("%s ordering %s to '%s' at %d") % ch->getName() % victim->getName() % cmd % room);
```

### Stack Trace Logging

Use vlogf_trace when the call context matters for debugging. This appends a stack trace using backtrace on glibc systems.

```cpp
vlogf_trace(LOG_BUG, format("Unexpected state in %s") % __func__);
```

### Personal Developer Logs

Use personal log channels (LOG_JESUS through LOG_ANGUS) for debugging visible only to specific developers. These require the developer's character name to view.

```cpp
vlogf(LOG_DASH, format("Debug: variable x = %d") % x);
```

### Assertions for Invariants

Use mud_assert for conditions that must always be true. Unlike standard assert, these execute in all builds and provide formatted messages before calling abort.

```cpp
// Pointer validity
mud_assert(t != NULL, "canSee with NULL t");

// Array bounds
mud_assert(i >= MIN_WEAR && i < MAX_WEAR, "Bad limb slot, %s %d", getName().c_str(), i);

// Bidirectional relationship consistency
mud_assert(t.parent == NULL, "TThing += : t.parent existed. item: %s", t.name.c_str());
```

### Recoverable vs Fatal Errors

Log and return for recoverable conditions. Assert for invariant violations that indicate programmer error.

```cpp
// Recoverable: log and continue
if (target == NULL) {
    vlogf(LOG_BUG, "Null target in doAttack, aborting attack");
    return FALSE;
}

// Fatal invariant violation: assert and crash
mud_assert(this->roomp != NULL, "Being with no room pointer");
```

### Including Context

Always include relevant context in log messages. Names, room numbers, values that triggered the error, and function names make debugging possible.

```cpp
// BAD: No context
vlogf(LOG_BUG, "Invalid value");

// GOOD: Full context
vlogf(LOG_BUG, format("Invalid damage value %d for %s attacking %s in room %d")
               % damage % attacker->getName() % victim->getName() % room);
```

### Security Considerations

Never log passwords, tokens, or sensitive player data.

```cpp
// WRONG
vlogf(LOG_PIO, format("Login attempt: %s with password %s") % name % password);

// RIGHT
vlogf(LOG_PIO, format("Login attempt: %s") % name);
```

### Performance-Sensitive Code

Avoid vlogf in hot paths. Each call performs time syscall, localtime conversion, fprintf, and iterates all online descriptors for immortal notification. Use LOG_SILENT for frequent events or gate expensive message computation behind conditionals.

```cpp
// Avoid computing message if no one will see it
if (isDebugMode) {
    vlogf(LOG_DEBUG, format("Expensive computation: %s") % computeExpensiveString());
}
```

## Reference

### Log Categories

The logTypeT enum in log.h defines all categories. Categories serve both filtering and identification purposes.

**Special Values:**
- LOG_SILENT (-2): Recorded but not echoed to immortals
- LOG_NONE (-1): Empty or unused

**Standard Categories (0-22):**
- LOG_MISC (0): Miscellaneous events not fitting other categories
- LOG_LOW (1): L.O.W. errors and builder issues
- LOG_FILE (2): File I/O failures
- LOG_BUG (3): Code bugs and assertion failures
- LOG_PROC (4): Spec proc errors
- LOG_PIO (5): Player login/logout events
- LOG_IIO (6): Immortal login/logout and wiz file loading
- LOG_CLIENT (7): Client protocol issues
- LOG_COMBAT (8): Combat system bugs
- LOG_CHEAT (9): Detected cheating attempts
- LOG_FACT (10): Faction system issues
- LOG_DB (11): Database query problems
- LOG_MOB (15): Generic mobile errors
- LOG_MOB_AI (16): Mobile AI/logic errors
- LOG_MOB_RS (17): Mobile response script errors
- LOG_OBJ (18): Generic object errors
- LOG_EDIT (21): Editor (oedit/medit/redit) errors

**Personal Developer Logs (23-31):**
- LOG_JESUS (23), LOG_BATOPR (24), LOG_BRUTIUS (25), LOG_COSMO (26)
- LOG_MAROR (27), LOG_PEEL (28), LOG_LAPSOS (29), LOG_DASH (30), LOG_ANGUS (31)

LOG_MAX value 23 marks the boundary between standard categories and personal logs.

### Category Selection Guidelines

| Situation | Category |
|-----------|----------|
| Code bug or logic error | LOG_BUG |
| File read/write failure | LOG_FILE |
| Database issue | LOG_DB |
| Spec proc error | LOG_PROC |
| Player login/logout | LOG_PIO |
| Suspected cheating | LOG_CHEAT |
| High-frequency debug | LOG_SILENT or personal |

### Output Formats

**stderr Format:**
```
YYYY|MMDD|HH:MM:SS :: Category: Message
```

Example:
```
2024|0115|14:23:45 :: BUG: Player Gandalf attempted to move to invalid room -1
```

**In-Game Immortal Format:**
```
// BUG: Player Gandalf attempted to move to invalid room -1
```

### Log File Locations

- Server stdout/stderr: Console or redirect target
- Immortal personal logs: lib/mutable/immortals/{name}/logfile

### setsev Command

Immortals with POWER_SETSEV can toggle which categories they receive in real-time.

```
setsev           - Show current settings
setsev misc      - Toggle LOG_MISC
setsev combat    - Toggle LOG_COMBAT
setsev <name>    - Toggle personal log (developer only)
```

POWER_SETSEV_IMM required for non-LOW categories. Settings persist in wizdata database table and restore on login.

### Signal Handlers

| Signal | Handler | Action |
|--------|---------|--------|
| SIGALRM | logsig | Logs "Signal received. Ignoring." |
| SIGHUP/SIGINT/SIGTERM | hupsig | Logs shutdown message, exits |
| SIGVTALRM | checkpointing | Calls mud_assert if deadlocked |
| SIGUSR1/SIGUSR2 | purgeRequest | Logs purge/shutdown request |

Source: signals.cc

## Implementation

### vlogf Core Function

The primary logging function in utility.cc formats timestamps, writes to stderr, and optionally notifies immortals. Takes logTypeT category and sstring message.

When tError is non-negative, the function iterates all online descriptors. For each descriptor with POWER_SETSEV and the corresponding severity bit set, it creates a SystemLogComm object pushed to the output queue. The SystemLogComm class handles formatting the message with the "//" prefix for in-game display.

### vlogf_trace Stack Traces

Extends vlogf by calling backtrace on glibc systems to capture the call stack. Appends backtrace_symbols output to the log message before passing to vlogf. Useful for debugging complex state errors where the path to failure matters.

### TPerson::logf Per-Immortal Logging

Active only for immortals where should_be_logged returns true. Opens a file handle in lib/mutable/immortals/{name}/logfile on login in append mode. Commands and actions can write to this personal activity log via TPerson::logf using standard printf format arguments.

### mud_assert Behavior

Defined in handler.cc. When condition evaluates to false (zero):

1. Formats error message with variadic arguments
2. Logs "ASSERTION FAILED: {message}" to LOG_BUG
3. Calls abort to crash with core dump for debugging

Never disabled in any build configuration. Side effects in the condition expression are safe since the condition always evaluates.

### Severity Filtering System

Each Descriptor stores a severity bitmask. The vlogf function checks this bitmask when tError is non-negative. Only descriptors with the corresponding bit set and POWER_SETSEV receive the message.

The doSetsev command in immortal.cc toggles bits in this bitmask. The tFields array maps category names to bit positions. The tHelp array provides descriptions for the setsev help display.

Persistence occurs through wizFileRead and wizFileWrite in wiz_data.cc, which serialize the severity bitmask to the wizdata database table.

### getLogType Category Names

Maps logTypeT enum values to display strings used in log output. Called by vlogf to format the category prefix. Contains a switch statement with cases for all defined categories. Returns "UNKNOWN" for unrecognized values.

## Troubleshooting

### Logs Not Appearing for Immortals

Check POWER_SETSEV granted to the character. Verify severity settings with setsev command show the desired category enabled. Confirm the log category value is non-negative (LOG_SILENT bypasses immortal notification intentionally).

### Missing Context in Error Messages

Review the vlogf call site and add relevant variables: character names, room numbers, object vnums, state values that triggered the error. Use __func__ macro to include function name automatically.

### Assertion Failures Without Core Dumps

Verify ulimit allows core file creation. Check core_pattern sysctl points to writable location. Ensure abort signal handlers not installed that prevent core dumps.

### Personal Log Files Not Created

Confirm should_be_logged returns true for the immortal. Check lib/mutable/immortals/{name}/ directory exists and is writable. Verify file handle opened successfully in descriptor login sequence.

### Performance Issues from Logging

Profile to identify vlogf calls in hot paths. Consider whether the log provides value proportional to cost. Replace with LOG_SILENT if immortal notification unnecessary. Gate expensive message formatting behind conditionals that check debug flags.

### Severity Settings Not Persisting

Check wizdata database table for corruption. Verify wizFileWrite called on logout or setting change. Confirm wizFileRead executes during login and reads severity field correctly.

### Signal Handler Logs Missing

Check signal handler installation in signals.cc occurs during server initialization. Verify signal not blocked or ignored by parent process. Confirm handler calls vlogf and stderr not redirected to /dev/null.

### Adding New Categories

1. Add enum value in log.h after existing standard categories but before LOG_MAX if standard, or after LOG_MAX if personal
2. Increment LOG_MAX if adding standard category
3. Add case to getLogType switch in utility.cc mapping enum to display name
4. Add entry to tFields array in doSetsev mapping category name to bit position
5. Add entry to tHelp array in doSetsev with description
6. Update severity bit handling if special permissions required beyond POWER_SETSEV
