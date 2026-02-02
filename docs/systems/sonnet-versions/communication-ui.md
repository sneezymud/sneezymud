---
title: Communication UI - OOC Channels, Ignore, and Time
category: understanding
keywords: [doShout, doNewbie, doCommune, ignoreList, isIgnored, GameTime, time_info, AUTO_NOSHOUT, TOG_SHOUTING, GMCP, twitterShout, blockedlist, NEWBIE_PURGATORY_LENGTH, sendShout, moonphase, sunrise, sunset]
related: [communication-system.md, wizard-powers.md, scheduler-pulses.md, configuration-reference.md]
primary_symbols:
  functions: [doShout, sendShout, doNewbie, doCommune, doTime, isIgnored, isMailIgnored, anotherHour, mudTimePassed, calcNewSunRise]
  classes: [ignoreList, GameTime, time_info_data]
  files: [code/code/misc/talk.cc, code/code/misc/newbie.cc, code/code/misc/other.cc, code/code/sys/connect.h, code/code/sys/gametime.cc, code/code/misc/weather.cc, code/code/misc/toggle.cc]
---

## Overview

How do players communicate globally across the MUD without breaking character immersion? How can they block harassment? How does the MUD track time that passes faster than reality?

This system provides three distinct mechanisms that together enable out-of-character player interaction and temporal simulation:

**OOC Communication Channels** let players broadcast messages globally without being physically nearby. The shout channel reaches all online players, the newbie channel connects newcomers with helpers, and the commune channel coordinates immortal staff. Each channel has different access requirements and filtering rules, but all bypass spatial limitations that constrain in-character speech.

**Ignore Lists** give players granular control over whose communications they receive. The system persists ignore preferences to the database and can block individual characters or entire accounts. When someone is ignored, their messages are silently filtered for the ignorer while appearing to succeed from the sender's perspective, preventing harassment confirmation.

**Accelerated Time** runs the MUD calendar roughly 13 times faster than reality, with full seasonal cycles completing in about a month. Day length varies sinusoidally throughout the year, affecting sunrise/sunset times, weather patterns, and available light. The system tracks hours, days, months, years, weekdays, and moon phases independently from real-world time.

These systems intersect when channel messages include timestamps, when idle timeouts use MUD time, and when seasonal events trigger channel announcements. They share common infrastructure for message filtering, profanity control, and toggle-based preferences.

## Patterns

### Channel Broadcasting

**Always check recipient state before broadcasting.** Sleeping players, disconnected descriptors, and characters in soundproof rooms should not receive messages.

**Always respect ignore lists before sending.** Call `desc->ignored.isIgnored(sender_desc)` to filter unwanted communications. The sender should see success regardless of filtering status.

**Always check movement cost before shouting.** Require at least 15 move and deduct the cost before broadcasting. Low-move characters cannot shout.

```cpp
if ((getMove() < 15) && isPc()) {
    sendTo("You don't have the energy to shout!\n\r");
    return;
}
```

**Never broadcast to players who toggled the channel off.** Check autobits like `AUTO_NOSHOUT` before including recipients in the broadcast loop.

**Never skip GMCP integration for modern clients.** Send `comm.channel` messages with channel type, sender name, and content to support client-side filtering and logging.

**Why:** Channel messages reach many recipients simultaneously. Filtering at send time prevents wasted processing and respects player preferences. Missing state checks causes messages to reach inappropriate recipients or crash when accessing null descriptors.

### Ignore System Usage

**Always verify ignore list capacity before adding.** The maximum is 20 entries per player. Attempting to add beyond this limit should fail gracefully with an error message.

**Always persist changes to database immediately.** Call `addDB()` or `removeDB()` after modifying in-memory ignore lists to ensure crash recovery.

**Never reveal ignore status to the ignored party.** When someone sends a tell to an ignored player, show success to the sender but never deliver the message. This prevents harassers from confirming they're being ignored.

**Never allow players to ignore themselves.** Check that the target is different from the requester before adding to ignore list.

**Always use account blocking for persistent troublemakers.** Account-wide blocks (prefixed with `~`) prevent evasion through alt characters and require immortal powers to set.

**Why:** Ignore lists exist to prevent harassment. Revealing ignore status enables harassers to retaliate or evade. Database persistence ensures preferences survive crashes. Capacity limits prevent memory abuse.

### Time System Modification

**Always call `anotherHour()` when advancing time.** This triggers weather updates, sunrise/sunset recalculation, and scheduled events tied to MUD hours.

**Never modify `time_info` directly from outside GameTime class.** Use class methods to maintain consistency between time components and derived values like sunrise times.

**Always recalculate sunrise/sunset when months change.** Seasonal day length variation requires updating these values monthly through `calcNewSunRise()` and `calcNewSunSet()`.

**Never assume fixed daylight hours.** Sunrise ranges from ~4:30 AM to ~7:30 AM depending on season. Daylight-dependent code must check current values.

**Always use `hourminTime()` for time display consistency.** This encodes hours and minutes as a single integer for comparison and storage.

**Why:** The time system has many interdependencies. Direct modification breaks weather, lighting, and scheduled events. Seasonal variation is fundamental to immersion and must be respected throughout the codebase.

### Toggle and Preference Management

**Always validate toggle changes before applying.** Some toggles require specific powers or have level restrictions. Check permissions before modifying autobits.

**Never allow toggle changes for disconnected descriptors.** Toggles are stored on descriptors and require active connections to modify safely.

**Always save descriptor state after toggle changes.** Call `doQueueSave()` to persist autobit modifications to database.

**Never assume toggle state is binary.** Some toggles have multiple modes or interact with other flags. Read current state before determining new state.

**Why:** Toggles control critical privacy and communication preferences. Allowing unauthorized changes enables harassment. Missing persistence means preferences reset on reconnect.

## Reference

### Symbol Quick Reference

| Symbol | Type | Purpose |
|--------|------|---------|
| `doShout()` | function | Broadcast message to all players, costs 15 move |
| `sendShout()` | function | Deliver shout to eligible descriptor |
| `doNewbie()` | function | Send message on newbie helper channel |
| `doCommune()` | function | Immortal-only wiznet communication |
| `doTime()` | function | Display current MUD and real time |
| `doIgnore()` | function | Manage player ignore list |
| `isIgnored()` | function | Check if descriptor is ignoring sender |
| `isMailIgnored()` | function | Static check for mail filtering |
| `anotherHour()` | function | Advance MUD time by one hour |
| `mudTimePassed()` | function | Calculate MUD time elapsed between timestamps |
| `realTimePassed()` | function | Calculate real time elapsed between timestamps |
| `calcNewSunRise()` | function | Compute sunrise time based on season |
| `is_daytime()` | function | Check if current hour is daylight |
| `is_nighttime()` | function | Check if current hour is nighttime |
| `ignoreList` | class | Manages per-player ignore list with DB persistence |
| `GameTime` | class | Central MUD time management and conversion |
| `time_info_data` | struct | Stores current time components |
| `AUTO_NOSHOUT` | flag | Blocks receiving shouts |
| `AUTO_NOTELL` | flag | Blocks incoming tells |
| `AUTO_AFK` | flag | Enables auto-AFK after idle timeout |
| `AUTO_PG13` | flag | Enables profanity filter |
| `TOG_SHOUTING` | flag | Global disable for shout channel |
| `TOG_WIZBUILD` | flag | Allows builders to see wiznet |
| `PLR_NEWBIEHELP` | flag | Marks player as newbie helper |
| `PLR_GODNOSHOUT` | flag | God-imposed communication ban |
| `POWER_WIZNET` | power | Required to use commune channel |
| `POWER_ACCESS` | power | Required for account-wide ignore |
| `POWER_ACCOUNT` | power | Required for account-wide ignore |

### Channel Overview

| Channel | Command | Audience | Requirements | Toggleable |
|---------|---------|----------|--------------|------------|
| Shout | `shout` | All online players | Level 2+, 15 move | `AUTO_NOSHOUT` |
| Newbie | `newbie` | Newbies + helpers | Newbie account or `PLR_NEWBIEHELP` | No |
| Commune | `commune` | Immortals | `POWER_WIZNET` | `TOG_WIZBUILD` (global) |

### Shout Restrictions

| Restriction | Check | Effect |
|-------------|-------|--------|
| Minimum level | `getLevel() >= 2` | Prevents level 1 spam |
| Movement cost | `getMove() >= 15` | Deducts 15 move per shout |
| Wait state | N/A | Adds 0.5 combat rounds |
| Charmed | `isCharmed()` | Prevents charmed shouts |
| Global toggle | `TOG_SHOUTING` | Disables shouting entirely |
| God ban | `PLR_GODNOSHOUT` | Immortal-imposed silence |

### Autobits

| Autobit | Default | Effect |
|---------|---------|--------|
| `AUTO_NOSHOUT` | Off | Blocks receiving shouts |
| `AUTO_NOTELL` | Off | Blocks incoming tells except from last recipient |
| `AUTO_AFK` | Off | Auto-AFK message after 5 minutes idle |
| `AUTO_PG13` | Off | Profanity filter on received messages |

### Ignored Communication Types

| Communication | Filtered by Ignore | Sender Feedback |
|--------------|-------------------|-----------------|
| Say | Yes | Normal |
| Tell | Yes | Success (silent drop) |
| Whisper | Yes | Normal |
| Shout | Yes | Normal |
| Grouptell | Yes | Normal |
| Newbie | Yes | Normal |
| Emote | Yes | Normal |
| Socials | Yes | Normal |

### Time Conversion Rates

| MUD Unit | Real Time Equivalent |
|----------|---------------------|
| 15 MUD minutes | 1 tick (~72 seconds) |
| 1 MUD hour | 4 ticks (~4.8 minutes) |
| 1 MUD day | 24 MUD hours (~115 minutes) |
| 1 MUD month | 28 MUD days (~54 hours) |
| 1 MUD year | 12 MUD months (~27 days) |

### Calendar Components

| Element | Range | Notes |
|---------|-------|-------|
| Minutes | 0, 15, 30, 45 | Increments in 15-minute blocks |
| Hours | 0-23 | 24-hour format |
| Day | 0-27 | 28 days per month (displayed as 1-28) |
| Month | 0-11 | 12 months per year |
| Year | 550+ | P.S. era |

### Seasonal Day Length

| Season | Daylight Hours | Sunrise | Sunset |
|--------|---------------|---------|--------|
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

### Key Files

| File | Contents |
|------|----------|
| `code/code/misc/talk.cc` | doShout, doCommune, sendShout |
| `code/code/misc/newbie.cc` | doNewbie, newbie channel |
| `code/code/misc/toggle.cc` | doToggle, autobits management |
| `code/code/misc/toggle.h` | AUTO_* and PLR_* flag definitions |
| `code/code/misc/other.cc` | ignoreList implementation, doIgnore |
| `code/code/sys/connect.h` | ignoreList class definition |
| `code/code/sys/gametime.cc` | GameTime class implementation |
| `code/code/sys/gametime.h` | GameTime class definition |
| `code/code/misc/weather.cc` | Sunrise/sunset, moon phases, seasons |
| `code/code/misc/info.cc` | doTime command |
| `code/code/misc/constants.cc` | Month names, weekday names |

## Implementation

### Shout Channel Flow

When a player issues the shout command, `doShout()` validates that the shouter meets requirements: level 2 or higher, at least 15 movement points, not charmed, and shouting not globally disabled. It deducts the movement cost and adds a 0.5 round wait state.

The message then passes to a loop over all descriptors. For each connected player, `sendShout()` checks eligibility: connection state must be `CON_PLYNG`, position must be above sleeping, the room must not be soundproof, the recipient must not be ignoring the shouter, and the recipient must not have `AUTO_NOSHOUT` set (unless the shouter is immortal).

Eligible recipients receive the formatted shout message. For immortals, the message includes the room vnum where the shout originated. For mortals, it's just the shouted content. GMCP-capable clients also receive a `comm.channel` message with channel type "yell", sender name, and content for client-side processing.

The shout integrates with Twitter through `twitterShout()`, posting the message to an external feed. The garbled version is sent to avoid spoiling puzzles or secret information.

### Newbie Channel Mechanics

The newbie channel serves two populations: actual newbies (account age less than `NEWBIE_PURGATORY_LENGTH` seconds) and designated helpers (players with `PLR_NEWBIEHELP` flag or immortals). When `doNewbie()` executes, it first determines if the sender qualifies as newbie or helper.

The message broadcasts to all connected descriptors that qualify as either newbie or helper. The sender's status determines the prefix: "Newbie" for actual newbies, "Expert" for helpers. This visual distinction helps recipients understand the expertise level of the person speaking.

Ignore list filtering applies to newbie messages just like other channels. If a recipient is ignoring the sender, the message does not appear for them but the sender sees normal success feedback.

The channel has no toggle to disable reception. Players who don't want newbie messages must either age out of newbie status (if they're newbies) or remove their helper flag (if they volunteered to help).

### Commune/Wiznet Implementation

Commune provides immortal-only communication with optional level filtering. When `doCommune()` executes, it verifies the sender has `POWER_WIZNET` or is switched into a character that has it.

The message can target a specific god level and above by prefixing with `@<level>`. For example, `commune @57 secret discussion` sends only to gods level 57 or higher. Without level targeting, all immortals see the message.

Visibility is controlled by two mechanisms: `POWER_WIZNET_ALWAYS` grants unconditional access, while `TOG_WIZBUILD` allows all builders to see wiznet when enabled. The typical setup is that core immortals always see wiznet, while builders only see it when staff wants them involved.

GMCP integration sends `comm.channel` messages with channel type "wiz" for client-side filtering and logging.

### Ignore List Storage and Retrieval

The `ignoreList` class manages both in-memory state and database persistence. For small ignore lists (the common case), a static array holds ignore data shared across all players to reduce per-player memory overhead. When a player's ignore list grows large, `convertFromStatic()` migrates their data to individual storage.

Adding an ignore entry calls `add()` which checks capacity (maximum 20 entries), validates the target exists and is different from the ignorer, then inserts the entry. It immediately calls `addDB()` to persist to the `blockedlist` table with the player's ID and ignored name.

Removal is symmetric: `remove()` deletes from in-memory structures and calls `removeDB()` to update the database. The database holds the authoritative state; in-memory structures are just caches populated on login.

Account-wide blocks store the account name prefixed with `~`. When checking if a descriptor is ignored, the code checks both the character name and the account name (with `~` prefix) against the ignore list. This allows immortals to block entire accounts when individual character blocks would be insufficient.

The static method `isMailIgnored()` provides ignore checking for mail and other asynchronous systems that don't have active descriptor references. It loads ignore data on demand rather than keeping all ignore lists in memory.

### Tell Ignore Behavior

When a tell is sent to an ignored player, the sending code calls `isIgnored()` before delivering the message. If the target is ignoring the sender, the code returns success to the sender without delivering the message. From the sender's perspective, the tell succeeded normally. They receive their own "You tell <target>" confirmation. The target never sees the message.

This asymmetric feedback is intentional. If the sender received "That player is ignoring you" messages, harassers could use ignoring/unignoring patterns as a communication channel or identify when they've been ignored to escalate harassment through other means. Silent drops prevent information leakage.

The only exception is for immortals with appropriate powers, who see ignore status when examining player data.

### Time Progression and Tick Integration

MUD time advances through `anotherHour()`, called from the scheduler when enough ticks have elapsed. The scheduler tracks real seconds elapsed and calls time advancement when 4 ticks (~288 real seconds) have passed, representing 1 MUD hour.

Each time advancement increments the hour component of `time_info`. When hours wrap past 23, the day increments. When days exceed 27 (28-day months), the month increments. When months exceed 11, the year increments. All these transitions trigger dependent updates.

Month changes call `calcNewSunRise()` and `calcNewSunSet()` to recompute sunrise/sunset times based on the new seasonal position. The calculation uses a sine wave centered on the spring equinox (April 1st) with amplitude of 1.5 hours, giving realistic variation throughout the year.

Weather updates also trigger on time changes, with seasonal patterns affecting precipitation type and temperature. Winter months favor snow, spring and autumn favor rain, summer favors warm and humid conditions.

### Sunrise/Sunset Calculation Details

Sunrise and sunset times vary sinusoidally throughout the year to simulate Earth's axial tilt. The spring equinox (April 1st) serves as the reference point where sunrise is exactly 6:00 AM and sunset is exactly 6:00 PM.

`calcNewSunRise()` computes days since equinox, calculates `sin(2π * days / year_length)` to get a value between -1 and +1, multiplies by -1.5 to invert the wave (negative before equinox = late sunrise), and adds the result to the base 6 AM time. The result is stored in quarter-hour units.

Winter solstice (January 1st area) produces maximum positive offset, pushing sunrise to ~7:30 AM. Summer solstice (July 1st area) produces maximum negative offset, pulling sunrise to ~4:30 AM. The sine wave ensures smooth transitions matching natural seasonal progression.

Sunset uses the same calculation but with 6:00 PM as the base time. The day length (time between sunrise and sunset) varies from ~9 hours in winter to ~15 hours in summer.

Full moon adds +1 to outdoor light levels during nighttime hours, making nocturnal navigation easier during certain times of the month.

### Moon Phase Tracking

The moon phase derives from a 32-day cycle independent of the calendar month. The moon value tracks from 0-31 and increments each MUD day. Phase names map to value ranges: new moon (0-3, 28-31), waxing (4-11), full (12-19), and waning (20-27).

The full moon period (values 12-19) represents the 8-day window when moonlight provides the +1 outdoor illumination bonus. This affects visibility calculations for outdoor rooms during night hours.

Moon rise and set times also vary throughout the cycle, with moon rise occurring later each night (approximately 50 minutes later per night in real-world terms, which translates to specific MUD time delays).

### Time Display and Timezone Adjustment

The time command displays current MUD time formatted as 12-hour AM/PM, the day of the week (calculated as `((28 * month) + day + 1) % 7`), the day number and month name, and the year in the P.S. era.

It also shows sunrise/sunset and moonrise/moonset times for the current day, along with the current moon phase name.

Real-world time appears with an optional timezone adjustment. Players can set their offset from server time using `time <offset>`, storing the hour adjustment in `desc->account->time_adjust`. When displaying real time, the command adds this offset to the server's UTC time to show the player's local time.

### Toggle Persistence and Synchronization

Toggles are stored in the `autobits` field of the descriptor. When a player issues a toggle command, `doToggle()` validates the request, modifies the appropriate bit in `desc->autobits`, and calls `doQueueSave()` to persist the change to the database.

Some toggles have immediate effects: toggling `AUTO_NOSHOUT` immediately affects whether the next shout reaches the player. Other toggles like `AUTO_AFK` only matter when specific conditions trigger (idle timeout checks).

Global toggles like `TOG_SHOUTING` are stored in a separate `toggleInfo` array and require immortal powers to modify. When set, these override individual player preferences—if `TOG_SHOUTING` is on, no one can shout regardless of personal settings.

The system does not sync toggle state between multiple logins of the same character. The descriptor's autobits reflect the state at login time. If the player modifies toggles and then reconnects, the new connection loads the saved state from the database.

### GMCP Channel Integration

Modern MUD clients support GMCP (Generic MUD Communication Protocol) for structured data exchange. When channels send messages, they also send parallel GMCP messages in JSON format to clients that negotiated GMCP support during connection.

The channel messages use the `comm.channel` namespace with fields for channel type ("yell", "wiz", etc.), sender name, message text, and optional metadata like target or room context. Clients can use this structured data for custom highlighting, logging, or filtering without parsing the text output.

Sending both text and GMCP messages ensures compatibility: legacy clients get formatted text, modern clients get both for enhanced functionality. The GMCP messages never replace text output, they supplement it.

### Twitter Integration for Shouts

The `twitterShout()` function posts shout content to an external Twitter feed, bridging in-game communication with social media. The implementation sends the garbled version of the message to avoid spoiling puzzle solutions or secret content that shouldn't be publicly visible.

This integration runs asynchronously to avoid blocking the game if Twitter's API is slow or unavailable. Failed posts are logged but don't impact the in-game shout delivery.

The feature can be toggled on/off through configuration without code changes. When disabled, the `twitterShout()` call becomes a no-op.

## Troubleshooting

### Symptom: Shout Not Reaching Some Players

**Likely cause:** Recipients have `AUTO_NOSHOUT` set or are ignoring the shouter.

**Diagnostic approach:** Check recipient's toggle status with `stat <player>` as an immortal to see autobits. Verify ignore list with database query to `blockedlist` table for the recipient's player ID.

**Fix:** If intended blocking, no action needed. If accidental, have recipient toggle noshout off or remove from ignore list.

### Symptom: Newbie Channel Shows Expert Messages to New Players

**Likely cause:** Player has `PLR_NEWBIEHELP` flag set on a newbie account.

**Diagnostic approach:** Check player flags with `stat <player>` to see if `PLR_NEWBIEHELP` is present. Check account age against `NEWBIE_PURGATORY_LENGTH`.

**Fix:** Remove `PLR_NEWBIEHELP` flag if accidentally set. The flag is intended for experienced players helping newbies, not newbies themselves.

### Symptom: Ignore List Refuses New Entries

**Likely cause:** Ignore list is at maximum capacity (20 entries).

**Diagnostic approach:** Count entries in `blockedlist` table for the player's ID. Should be exactly 20 if refusing adds.

**Fix:** Player must remove an existing entry before adding new ones. No way to expand limit without code changes.

### Symptom: Tell Appears to Succeed But Never Arrives

**Likely cause:** Recipient is ignoring the sender.

**Diagnostic approach:** Check recipient's ignore list in database. Look for either the sender's character name or account name prefixed with `~`.

**Fix:** If legitimate ignore, no action needed. If accidental, recipient can remove from ignore list. Sender should never be informed they're being ignored.

### Symptom: Time Command Shows Wrong Real-World Time

**Likely cause:** Player's timezone adjustment is incorrect.

**Diagnostic approach:** Check `desc->account->time_adjust` value. Should be player's UTC offset in hours.

**Fix:** Player should use `time <offset>` to set correct timezone. For example, EST is `time -5`, PST is `time -8`.

### Symptom: Sunrise/Sunset Times Don't Change Monthly

**Likely cause:** `calcNewSunRise()` not being called on month transitions.

**Diagnostic approach:** Add logging to `anotherHour()` to verify month increment triggers sunrise recalculation.

**Fix:** Ensure month transition logic calls weather update functions. Missing call means sunrise/sunset stay frozen at last calculated values.

### Symptom: Full Moon Doesn't Provide Light Bonus

**Likely cause:** Light calculation not checking moon phase value.

**Diagnostic approach:** Check outdoor room light calculation during full moon hours (moon value 12-19, nighttime). Should add +1 to ambient light.

**Fix:** Verify light calculation functions include moon phase check. Missing check means moon is decorative only.

### Symptom: Account-Wide Block Not Working

**Likely cause:** Ignore entry missing `~` prefix or immortal lacking required power.

**Diagnostic approach:** Check `blockedlist` table entry. Should be `~accountname` not just `accountname`. Verify immortal has `POWER_ACCESS` or `POWER_ACCOUNT`.

**Fix:** Add entry with correct prefix. Grant required power to immortal if lacking.

### Symptom: Toggle Changes Don't Persist After Reconnect

**Likely cause:** `doQueueSave()` not called after toggle modification, or database write failed.

**Diagnostic approach:** Check descriptor save queue after toggle command. Verify database write success in logs.

**Fix:** Ensure toggle code calls `doQueueSave()` for every autobits modification. Check database connection if writes are failing.

### Symptom: Commune Messages Reach Non-Immortals

**Likely cause:** `TOG_WIZBUILD` is on and recipients are builders without realizing it.

**Diagnostic approach:** Check recipient flags for immortal status or builder flags. Check `TOG_WIZBUILD` global state.

**Fix:** If `TOG_WIZBUILD` is intentionally on, this is expected behavior. Otherwise, toggle it off to restrict to actual immortals.

### Symptom: Charmed Character Can Shout

**Likely cause:** Missing `isCharmed()` check in shout validation.

**Diagnostic approach:** Verify charmed character attempts shout. Check if validation code includes charm check.

**Fix:** Add `isCharmed()` check to shout preconditions. Charmed characters should fail early with appropriate message.
