---
title: Communication UI - OOC Channels, Ignore Lists, and Time/Calendar
description: Out-of-character communication channels (shout, newbie, commune), player ignore lists, and MUD time/calendar systems.
category: informational
keywords: [out-of-character, OOC channels, player filtering, MUD time, calendar, GMCP]
primary_symbols:
  functions: [doShout, sendShout, doNewbie, doCommune, doIgnore, doTime, isIgnored, isMailIgnored, anotherHour, mudTimePassed, realTimePassed, calcNewSunRise, is_daytime]
  classes: [ignoreList, GameTime, time_info_data]
  enums: [AUTO_NOSHOUT, AUTO_NOTELL, AUTO_AFK, AUTO_PG13, TOG_SHOUTING, TOG_WIZBUILD, PLR_NEWBIEHELP, PLR_GODNOSHOUT, POWER_WIZNET, POWER_WIZNET_ALWAYS, POWER_ACCESS, POWER_ACCOUNT, CON_PLYNG, ROOM_SOUNDPROOF]
---

## Overview

How do players communicate outside the context of their characters? How do they filter unwanted messages? And what does "time" mean in a world that runs faster than reality?

This system manages three related concerns: out-of-character (OOC) communication channels that let players talk globally, an ignore system that filters unwanted messages, and a time/calendar system that creates a believable day/night cycle running faster than real time.

OOC channels serve distinct audiences. The shout channel broadcasts globally to all awake players, costing movement points to prevent spam. The newbie channel connects new players with experienced helpers. The commune/wiznet channel provides private immortal coordination. Each channel has its own access rules, filtering, and persistence characteristics.

The ignore system lets players block communications from specific players or entire accounts. When you ignore someone, their tells, shouts, emotes, and other communications simply never reach you. Critically, the sender sees success messages even when ignored - this prevents harassment confirmation where blockers could test whether their targets blocked them.

MUD time runs approximately 24 times faster than real time. One real hour equals roughly one MUD day. This creates a compressed day/night cycle with seasonal variations in daylight hours. Summer solstice brings 15-hour days while winter solstice brings only 9 hours of daylight, affecting visibility and weather patterns.

---

## Patterns

### Channel Access Control

**Always check channel-specific restrictions before sending.** Shout requires level 2+, 15 movement points, and adds combat wait state. Newbie requires either newbie account status or the `PLR_NEWBIEHELP` flag. Commune requires `POWER_WIZNET`.

**Always respect player toggle preferences.** Check `AUTO_NOSHOUT` before delivering shouts. Check global toggle `TOG_SHOUTING` to see if shouting is system-wide disabled.

**Never reveal ignore status to senders.** When a tell is blocked by ignore, return success to the sender. This prevents harassment confirmation. The ignored party simply never receives the message.

**Never skip GMCP integration for modern clients.** Send `comm.channel` messages with channel type, sender name, and content to support client-side filtering and logging. Clients receive both text and GMCP messages for compatibility.

### Ignore System Usage

**Always check ignore status using `isIgnored()` before delivering player-to-player communications.** This applies to tells, whispers, shouts, emotes, and social actions.

**Never allow players to exceed the 20-entry ignore list maximum.** Check `getCount() >= getMax()` before adding entries.

**Distinguish player blocks from account blocks.** Account blocks are stored with a `~` prefix. When checking ignore status, the system checks both the player name and the associated account. Only immortals with `POWER_ACCESS` or `POWER_ACCOUNT` can create account-wide blocks.

**Always persist ignore list changes to the database immediately.** The `blockedlist` table stores ignore entries. Use `addDB()` and `removeDB()` to maintain persistence.

**Never allow players to ignore themselves.** Check that the target is different from the requester before adding to ignore list.

### Time System Usage

**Never confuse MUD time with real time.** One MUD hour equals approximately 2.4 real minutes. Use `GameTime::mudTimePassed()` for MUD-relative calculations and `GameTime::realTimePassed()` for real-time conversions.

**Always use `is_daytime()` and `is_nighttime()` for light-dependent logic.** These functions account for seasonal variation in sunrise/sunset times.

**Remember that minutes increment in 15-minute blocks.** MUD minutes are always 0, 15, 30, or 45 - never intermediate values.

**Always recalculate sunrise/sunset when months change.** Seasonal day length variation requires updating these values monthly through `calcNewSunRise()` and `calcNewSunSet()`.

### Toggle and Preference Management

**Always save descriptor state after toggle changes.** Call `doQueueSave()` to persist autobit modifications to database. Missing persistence means preferences reset on reconnect.

**Never allow toggle changes for disconnected descriptors.** Toggles are stored on descriptors and require active connections to modify safely.

---

## Reference

### Symbol Quick Reference

| Symbol | Type | Purpose |
|--------|------|---------|
| `doShout()` | function | Process player shout command |
| `sendShout()` | function | Distribute shout to all valid recipients |
| `doNewbie()` | function | Process newbie channel message |
| `doCommune()` | function | Process immortal wiznet message |
| `doIgnore()` | function | Manage player ignore list |
| `doTime()` | function | Display MUD time information |
| `isIgnored()` | function | Check if source is on ignore list |
| `isMailIgnored()` | function | Static check for mail filtering without descriptor |
| `anotherHour()` | function | Advance MUD time by one hour |
| `mudTimePassed()` | function | Calculate MUD time elapsed between timestamps |
| `realTimePassed()` | function | Calculate real time elapsed between timestamps |
| `calcNewSunRise()` | function | Compute sunrise time based on season |
| `is_daytime()` | function | Check if currently day |
| `is_nighttime()` | function | Check if currently night |
| `ignoreList` | class | Per-player communication filter |
| `GameTime` | class | Central time management |
| `time_info_data` | struct | Stores current time components |

### Channel Requirements

| Channel | Command | Requirements | Toggle |
|---------|---------|--------------|--------|
| Shout | `shout` | Level 2+, 15 move | `AUTO_NOSHOUT` |
| Newbie | `newbie` | Newbie account or `PLR_NEWBIEHELP` | None |
| Commune | `commune` | `POWER_WIZNET` | `TOG_WIZBUILD` |

### Shout Restrictions

| Restriction | Check | Effect |
|-------------|-------|--------|
| Minimum level | `getLevel() >= 2` | Prevents level 1 spam |
| Movement cost | `getMove() >= 15` | Deducts 15 move per shout |
| Wait state | N/A | Adds 0.5 combat rounds |
| Charmed | `isCharmed()` | Prevents charmed shouts |
| Global toggle | `TOG_SHOUTING` | Disables shouting entirely |
| God ban | `PLR_GODNOSHOUT` | Immortal-imposed silence |

### Autobits (Player Preferences)

| Autobit | Default | Effect |
|---------|---------|--------|
| `AUTO_NOSHOUT` | Off | Block receiving shouts |
| `AUTO_NOTELL` | Off | Block incoming tells |
| `AUTO_AFK` | Off | Auto-AFK after idle |
| `AUTO_PG13` | Off | Profanity filter |

### Global Toggles (Immortal)

| Toggle | Effect |
|--------|--------|
| `TOG_SHOUTING` | Globally disable shouting |
| `TOG_WIZBUILD` | Allow builders to see wiznet |

### Player Flags

| Flag | Effect |
|------|--------|
| `PLR_NEWBIEHELP` | Marks player as newbie helper |
| `PLR_GODNOSHOUT` | God-imposed communication ban |

### Powers

| Power | Effect |
|-------|--------|
| `POWER_WIZNET` | Required to use commune channel |
| `POWER_WIZNET_ALWAYS` | Always sees wiznet regardless of toggle |
| `POWER_ACCESS` | Required for account-wide ignore |
| `POWER_ACCOUNT` | Required for account-wide ignore |

### Communications Filtered by Ignore

| Communication | Filtered | Sender Sees Success |
|---------------|----------|---------------------|
| Say | Yes | No |
| Tell | Yes | Yes (silent) |
| Whisper | Yes | No |
| Shout | Yes | No |
| Grouptell | Yes | No |
| Newbie | Yes | No |
| Emote | Yes | No |
| Socials | Yes | No |

### Time Conversion

| MUD Unit | Real Time |
|----------|-----------|
| 15 MUD minutes | 1 tick (~36 seconds) |
| 1 MUD hour | ~2.4 minutes |
| 1 MUD day | ~57.6 minutes |
| 1 MUD month | ~27 hours |
| 1 MUD year | ~13.4 days |

### Calendar Structure

| Element | Range | Notes |
|---------|-------|-------|
| Minutes | 0, 15, 30, 45 | 15-minute increments |
| Hours | 0-23 | 24-hour format |
| Day | 0-27 | 28 days/month (display 1-28) |
| Month | 0-11 | 12 months/year |
| Year | 550+ | "P.S." era |

### Seasonal Daylight

| Season | Daylight | Sunrise | Sunset |
|--------|----------|---------|--------|
| Winter Solstice | ~9 hours | ~7:30 AM | ~4:30 PM |
| Equinox | ~12 hours | ~6:00 AM | ~6:00 PM |
| Summer Solstice | ~15 hours | ~4:30 AM | ~7:30 PM |

### Moon Phases

| Moon Value | Phase |
|------------|-------|
| 0-3 | New |
| 4-11 | Waxing |
| 12-19 | Full |
| 20-27 | Waning |
| 28-31 | New |

Full moon provides +1 outdoor light at night.

### Key Files

| File | Contents |
|------|----------|
| `code/code/misc/talk.cc` | Shout, commune, sendShout |
| `code/code/misc/newbie.cc` | Newbie channel |
| `code/code/misc/toggle.cc` | Toggle command, autobits |
| `code/code/misc/toggle.h` | AUTO_* and PLR_* flags |
| `code/code/misc/other.cc` | ignoreList, doIgnore |
| `code/code/sys/connect.h` | ignoreList class definition |
| `code/code/sys/gametime.cc` | GameTime implementation |
| `code/code/sys/gametime.h` | GameTime definition |
| `code/code/misc/weather.cc` | Sunrise/sunset, moon, seasons |
| `code/code/misc/info.cc` | doTime command |
| `code/code/misc/constants.cc` | Month/weekday names |

---

## Implementation

### Shout Channel Distribution

When a player shouts, `doShout()` validates the sender meets requirements: minimum level 2, at least 15 movement points (deducted on use), not charmed, and shouting not globally disabled. A 0.5 combat round wait state is added to prevent spam.

Distribution via `sendShout()` iterates all descriptors. Recipients must satisfy all conditions: connection state `CON_PLYNG`, position above sleeping, not in a soundproof room, not ignoring the shouter, and not set `AUTO_NOSHOUT` (immortals and mobs bypass this last check).

GMCP integration sends `comm.channel { "chan": "yell", ... }` for client applications.

Twitter integration posts shouts via `twitterShout()`, sending a garbled version to avoid spoiling puzzles or secret information. The integration runs asynchronously to avoid blocking the game.

### Newbie Channel Logic

The newbie channel serves two audiences: actual newbies (accounts younger than `NEWBIE_PURGATORY_LENGTH` seconds) and helpers (immortals or players with `PLR_NEWBIEHELP` flag).

Message format identifies sender type: "Newbie <name>:" versus "Expert <name>:" based on whether the sender is a newbie or helper. Both newbies and helpers receive all newbie channel traffic, subject to ignore filtering.

Players toggle helper status with the `toggle newbiehelper` command.

### Commune/Wiznet Architecture

Immortal channel requiring `POWER_WIZNET`. Supports level targeting via `commune @<level> <message>` syntax to restrict delivery to gods at or above the specified level.

Visibility is controlled by two mechanisms: the `TOG_WIZBUILD` global toggle (when on, all builders see wiznet) and the `POWER_WIZNET_ALWAYS` power (always sees wiznet regardless of toggle state).

GMCP sends `comm.channel { "chan": "wiz", ... }`.

### ignoreList Storage Architecture

The `ignoreList` class stores blocked player names and account references. Maximum 20 entries per player (defined by `cMax`).

For performance optimization, small ignore lists share a static array (`m_staticIds`, `m_staticIgnored`, `m_staticCount`). When a player's list grows large, `convertFromStatic()` migrates to individual storage.

Database persistence uses the `blockedlist` table with columns `player_id` (ignoring player) and `blocked` (ignored name). Account blocks are stored with `~` prefix: player block stores "playername" while account block stores "~accountname".

The `isIgnored()` method checks both player names and associated accounts. For mail checking, `isMailIgnored()` provides a static check without requiring a descriptor, loading ignore data on demand.

### Ignore Command Processing

The `doIgnore()` function handles several subcommands:
- No arguments: list current ignore entries
- `<player>` or `add <player>`: add player to ignore list
- `remove <player>`: remove player from ignore list
- `addall <account>`: block entire account (requires `POWER_ACCESS` or `POWER_ACCOUNT`)
- `removeall <account>`: unblock account (requires same powers)

### Tell Ignore Behavior

When a tell is sent to an ignored player, the sending code calls `isIgnored()` before delivering the message. If the target is ignoring the sender, the code returns success to the sender without delivering the message. From the sender's perspective, the tell succeeded normally - they receive their "You tell <target>" confirmation. The target never sees the message.

This asymmetric feedback is intentional. If the sender received "That player is ignoring you" messages, harassers could use ignoring/unignoring patterns as a communication channel or identify when they've been ignored to escalate harassment through other means.

### GameTime Architecture

The `GameTime` class manages MUD time through a static `time_info_data` structure tracking hours, minutes, day, month, and year. The `BEGINNING_OF_TIME` constant anchors the calendar to August 10, 1990 real-world time. Years display with `YEAR_ADJUST` (550) added for the "P.S." era.

Time advances via `anotherHour()`, called from the scheduler pulse system. Minutes always increment in 15-minute blocks - the MUD never tracks finer granularity.

Conversion functions: `mudTimePassed()` calculates elapsed MUD time between two real timestamps, while `realTimePassed()` does the inverse. The ratio is approximately 24:1 (one real hour equals one MUD day).

### Sunrise/Sunset Calculation

Day length varies seasonally using sinusoidal calculation. The `calcNewSunRise()` method in weather.cc computes sunrise based on day of year, with April 1st as the spring equinox reference point.

The formula produces a sinusoidal variation of +/- 1.5 hours from the 6 AM baseline. This creates the seasonal range: winter solstice sunrise around 7:30 AM with sunset around 4:30 PM (9 hours daylight), equinox at 6 AM/6 PM (12 hours), summer solstice at 4:30 AM/7:30 PM (15 hours).

Month changes trigger `calcNewSunRise()` and `calcNewSunSet()` to recompute these values based on the new seasonal position.

### Moon Phase System

Moon cycle runs 32 days. The phase calculation divides moon value into ranges: 0-3 and 28-31 are new moon, 4-11 is waxing, 12-19 is full moon, 20-27 is waning.

Full moon affects outdoor lighting at night, providing +1 light level. This matters for visibility calculations in nighttime outdoor rooms. Moon rise and set times also vary throughout the cycle.

### Weekday Calculation

Weekday is computed as `((28 * month) + day + 1) % 7` using standard weekday names (Sunday through Saturday). The 28-day months ensure weekday patterns remain consistent across months.

### Time Command Output

The `doTime()` command displays comprehensive time information: current MUD hour/minute in AM/PM format, day of week, day of month, month name, year in P.S. era, sunrise/sunset times, moonrise/moonset times with phase, and real-world time.

Players can set a timezone offset stored in `desc->account->time_adjust` to adjust displayed real-world time. Use `time <offset>` to set (e.g., `time -5` for EST).

### Toggle Persistence

Toggles are stored in the `autobits` field of the descriptor. When a player modifies a toggle, `doQueueSave()` persists the change to the database. The system does not sync toggle state between multiple logins - each connection loads saved state from the database.

---

## Troubleshooting

### Tells Appearing to Work But Not Received

**Symptom:** Player sends tell, sees success message, but recipient never receives it.

**Likely cause:** Recipient has sender on ignore list.

**Diagnostic approach:** Check recipient's ignore list (as immortal). The silent success is intentional to prevent harassment confirmation.

**Fix:** Not a bug - working as designed. If legitimate miscommunication, recipient must remove sender from ignore list.

### Shout Not Working

**Symptom:** Player cannot shout, receives error message.

**Likely cause:** Multiple possible causes.

**Diagnostic approach:** Check in order:
1. Player level (must be 2+)
2. Player movement points (must have 15+)
3. Global `TOG_SHOUTING` toggle (may be system-disabled)
4. `PLR_GODNOSHOUT` flag (god-imposed communication ban)
5. Charmed status (charmed characters cannot shout)

**Fix:** Address the blocking condition. Movement recovers over time; level/flags require admin intervention.

### Ignore List Full

**Symptom:** Player cannot add more entries to ignore list.

**Likely cause:** Maximum 20 entries reached.

**Diagnostic approach:** Check `getCount()` against `getMax()` (20).

**Fix:** Player must remove existing entries before adding new ones. There is no way to increase the limit.

### Account Block Not Working

**Symptom:** Immortal adds account block but player's alts still communicate.

**Likely cause:** Account name mismatch or using player name instead of account name.

**Diagnostic approach:** Check the `blockedlist` table for the `~accountname` entry. Verify the account name is correct (accounts and player names differ). Verify immortal has `POWER_ACCESS` or `POWER_ACCOUNT`.

**Fix:** Use the correct account name with `ignore addall <accountname>`.

### Time Display Shows Wrong Real Time

**Symptom:** Real-world time in `time` command output is offset from actual time.

**Likely cause:** Player has timezone adjustment set.

**Diagnostic approach:** Check `desc->account->time_adjust` value.

**Fix:** Player uses `time <offset>` to adjust or reset (offset 0) their timezone preference.

### Newbie Channel Access Lost

**Symptom:** Player can no longer use newbie channel.

**Likely cause:** Account age exceeded `NEWBIE_PURGATORY_LENGTH`.

**Diagnostic approach:** Compare account creation time against the purgatory length constant.

**Fix:** If player should retain access as helper, grant `PLR_NEWBIEHELP` flag or have them use `toggle newbiehelper`.

### Sunrise/Sunset Times Not Updating

**Symptom:** Sunrise/sunset times stay frozen despite months passing.

**Likely cause:** `calcNewSunRise()` not being called on month transitions.

**Diagnostic approach:** Verify month transition logic in `anotherHour()` triggers sunrise recalculation.

**Fix:** Ensure month transition calls weather update functions.

### Full Moon Not Providing Light Bonus

**Symptom:** Outdoor nighttime light level unchanged during full moon.

**Likely cause:** Light calculation not checking moon phase value.

**Diagnostic approach:** Check outdoor room light calculation during full moon (moon value 12-19, nighttime). Should add +1 to ambient light.

**Fix:** Verify light calculation functions include moon phase check.

### Toggle Changes Not Persisting

**Symptom:** Toggle changes reset after reconnect.

**Likely cause:** `doQueueSave()` not called after toggle modification, or database write failed.

**Diagnostic approach:** Check descriptor save queue after toggle command. Verify database write success.

**Fix:** Ensure toggle code calls `doQueueSave()` for every autobits modification.

### Commune Messages Reaching Non-Immortals

**Symptom:** Builder players see commune/wiznet messages unexpectedly.

**Likely cause:** `TOG_WIZBUILD` is on.

**Diagnostic approach:** Check `TOG_WIZBUILD` global state.

**Fix:** If `TOG_WIZBUILD` is intentionally on, this is expected behavior. Otherwise, toggle it off to restrict to actual immortals.
