---
title: Logging and Error Handling
description: Centralized logging with categorized messages, per-immortal severity filtering, and custom assertions.
keywords: [vlogf, logTypeT, mud_assert, LOG_BUG, LOG_SILENT, setsev, severity filtering]
category: Critical Systems
related: [memory-safety.md, command-implementation.md]
last_updated: 2026-02-01
created_by_model: opus
source_files: [code/code/misc/log.h, code/code/misc/utility.cc, code/code/sys/handler.cc, code/code/misc/immortal.cc, code/code/misc/wiz_data.cc, code/code/sys/signals.cc, code/code/sys/comm.h]
---

# Logging and Error Handling

## Overview

SneezyMUD routes all diagnostic output through a single logging infrastructure that serves three audiences: developers reading stderr, immortals monitoring the live game, and auditors reviewing player activity.

Log messages carry a category tag that determines both their semantic meaning and their visibility. Immortals subscribe to categories they care about and ignore the rest. High-frequency events can be logged silently to avoid flooding the game interface while still leaving an audit trail.

The assertion system provides a controlled crash mechanism for invariant violations that cannot be recovered from. Unlike standard assertions, these remain active in production builds and produce formatted messages before terminating.

## Patterns

### Message Composition

Always include sufficient context to diagnose the problem without reading surrounding code. Include the function name, relevant identifiers, and numeric values.

Never log passwords, authentication tokens, or sensitive player data. Log the event occurrence, not the credentials.

Use silent logging for high-frequency events that would otherwise flood immortal consoles. Money transfers, component spawns, and routine NPC actions belong in the silent category.

Reserve stack traces for complex bugs where the call chain matters. The trace adds overhead and verbosity that is only worthwhile when the immediate context is insufficient.

### Assertion Usage

Use assertions exclusively for programmer errors that cannot be safely recovered from. Null pointers that should never be null, array indices outside valid bounds, and violated bidirectional relationship invariants are appropriate assertion triggers.

Never assert on conditions that can arise from player input or external data. Check those conditions explicitly and handle them with logged error returns.

Never place side effects inside assertion conditions. The condition should be a pure read that does not modify state.

### Category Selection

Match the category to the nature of the event, not its severity. A critical database error uses the database category. A minor spec proc warning uses the proc category. The severity filtering system allows immortals to tune what they see.

Use personal developer channels for debugging output that only one developer needs to see. Remove or comment out this logging before committing.

### Performance

Avoid logging in tight loops or frequently-executed paths. Each log call performs system calls and iterates online descriptors. Hot paths should either skip logging entirely or guard expensive log construction behind a condition check.

## Reference

### Special Category Values

| Category | Value | Behavior |
|----------|-------|----------|
| LOG_SILENT | -2 | Writes to stderr only, never echoed to immortals |
| LOG_NONE | -1 | Empty placeholder, not used for actual logging |

### Standard Categories

| Category | Value | Purpose |
|----------|-------|---------|
| LOG_MISC | 0 | Uncategorized messages |
| LOG_LOW | 1 | Builder and low-level errors |
| LOG_FILE | 2 | File I/O failures |
| LOG_BUG | 3 | Code bugs and assertion failures |
| LOG_PROC | 4 | Spec proc errors |
| LOG_PIO | 5 | Player login/logout |
| LOG_IIO | 6 | Immortal login/logout |
| LOG_CLIENT | 7 | Client protocol issues |
| LOG_COMBAT | 8 | Combat system errors |
| LOG_CHEAT | 9 | Detected cheating attempts |
| LOG_FACT | 10 | Faction system issues |
| LOG_DB | 11 | Database query problems |
| LOG_MOB | 15 | Mobile errors |
| LOG_MOB_AI | 16 | Mobile AI/logic errors |
| LOG_MOB_RS | 17 | Mobile response script errors |
| LOG_OBJ | 18 | Object errors |
| LOG_EDIT | 21 | Editor (oedit/medit/redit) errors |

### Personal Developer Channels (23-31)

Channels visible only to specific developers: LOG_JESUS (23), LOG_BATOPR (24), LOG_BRUTIUS (25), LOG_COSMO (26), LOG_MAROR (27), LOG_PEEL (28), LOG_LAPSOS (29), LOG_DASH (30), LOG_ANGUS (31). LOG_MAX (23) marks the boundary between standard and personal categories.

### Signal Handlers

| Signal | Handler | Behavior |
|--------|---------|----------|
| SIGALRM | logsig | Logs and ignores |
| SIGHUP/SIGINT/SIGTERM | hupsig | Logs shutdown, exits |
| SIGVTALRM | checkpointing | Asserts on deadlock |
| SIGUSR1/SIGUSR2 | purgeRequest | Logs purge/shutdown request |

### File Locations

| Type | Path |
|------|------|
| Server output | stderr (console or redirect) |
| Immortal personal log | lib/mutable/immortals/{name}/logfile |

### Output Formats

Stderr format: `YYYY|MMDD|HH:MM:SS :: Category: Message`

In-game format: `// Category: Message`

## Implementation

### Core Logging Path

The primary logging function writes to stderr with a timestamp, then iterates all connected descriptors looking for immortals with the POWER_SETSEV permission and the corresponding category bit set in their severity mask. Matching immortals receive the message via a SystemLogComm object pushed to their output queue.

The trace variant appends a stack trace using the glibc backtrace facility before invoking the standard logging path.

Per-immortal logging writes to individual files under the immortals directory. The file handle is opened at login when should_be_logged returns true and remains open for the session.

### Severity Filtering

Each Descriptor maintains a severity bitmask. The setsev command toggles individual bits. Settings persist in the wizdata database table and are restored by wizFileRead at login.

Categories with negative values bypass the immortal notification loop entirely. LOG_SILENT writes only to stderr. LOG_NONE produces no output.

Personal developer channels (23-31) have additional access control. Only the named developer can toggle or view their personal channel.

### Assertion Mechanism

The custom assertion evaluates its condition and returns immediately if true. On false, it formats the error message using variadic arguments, logs to LOG_BUG, and calls abort to produce a core dump.

Unlike the standard assert macro, this mechanism is never disabled by build configuration. It always executes in all builds.

### Adding New Categories

Add the enum value in log.h after the existing standard categories but before LOG_MOB. Update LOG_MAX if the new category falls within the standard range. Add the display name mapping in getLogType within utility.cc. Add the category to both the tFields and tHelp arrays in doSetsev within immortal.cc. Update severity bit handling if the category requires special permissions.

## Troubleshooting

### No Log Output Visible

**Symptom:** Expected log messages do not appear in stderr or game output.

**Cause:** Log category may be filtered out of immortal's severity mask.

**Diagnostic:** Run setsev with no arguments to view current filter settings.

**Fix:** Toggle the desired category with setsev <category>.

---

**Symptom:** Messages appear in game but not in stderr.

**Cause:** Not possible by design. All messages go to stderr first.

**Diagnostic:** Check that stderr is not redirected to /dev/null.

**Fix:** Verify server launch configuration includes proper stderr handling.

### Immortal Missing Log Messages

**Symptom:** One immortal sees logs that another does not.

**Cause:** Different severity mask settings between immortals.

**Diagnostic:** Each immortal runs setsev to compare settings.

**Fix:** Toggle missing categories individually or restore defaults via wizdata.

---

**Symptom:** Immortal has category enabled but still does not see messages.

**Cause:** Missing POWER_SETSEV or POWER_SETSEV_IMM permission.

**Diagnostic:** Check immortal's power bits.

**Fix:** Grant required permissions via appropriate admin command.

### Assertion Crashes

**Symptom:** Server terminates with "ASSERTION FAILED" in log.

**Cause:** Code invariant was violated. This is intentional crash behavior.

**Diagnostic:** Read the assertion message and examine the core dump.

**Fix:** Fix the code path that violated the invariant. Do not remove the assertion.

---

**Symptom:** Assertion fires on recoverable condition.

**Cause:** Assertion used incorrectly for a condition that should be handled gracefully.

**Diagnostic:** Evaluate whether the condition represents true programmer error.

**Fix:** Replace assertion with logged error return if condition can arise legitimately.

### Performance Degradation

**Symptom:** Server slows during high-activity periods.

**Cause:** Excessive logging in hot paths or tight loops.

**Diagnostic:** Profile to identify logging calls in frequently-executed code.

**Fix:** Move logging outside loops, use LOG_SILENT, or guard with condition checks.
