---
title: Communication UI - OOC Channels, Ignore/Friend Lists, and Time/Calendar
description: Out-of-character communication channels (shout, newbie, commune), the database-backed ignore system with account-wide blocking, and the MUD's accelerated time/calendar system with seasonal day/night variations.
keywords:
  - doShout
  - doNewbie
  - doCommune
  - sendShout
  - ignoreList
  - isIgnored
  - GameTime
  - time_info
  - AUTO_NOSHOUT
  - TOG_SHOUTING
  - GMCP
  - twitterShout
  - blockedlist
  - NEWBIE_PURGATORY_LENGTH
category: Understanding Systems
related:
  - communication-system.md
  - wizard-powers.md
  - scheduler-pulses.md
  - configuration-reference.md
last_updated: 2026-01-29
source_files:
  - code/code/misc/talk.cc
  - code/code/misc/newbie.cc
  - code/code/misc/toggle.cc
  - code/code/misc/toggle.h
  - code/code/misc/other.cc
  - code/code/sys/connect.h
  - code/code/sys/gametime.cc
  - code/code/sys/gametime.h
  - code/code/misc/weather.cc
  - code/code/misc/info.cc
  - code/code/misc/constants.cc
---

# Communication UI: OOC Channels, Ignore/Friend Lists, and Time/Calendar

This document covers out-of-character (OOC) communication channels, the ignore system for filtering unwanted messages, and the MUD's time/calendar system.

**Key concepts:**
- OOC channels for global communication beyond in-character speech
- Ignore lists persisted to database with account-wide blocking capability
- MUD time runs faster than real time with seasonal day/night variations
- Toggle-based channel preferences stored in autobits

---

## OOC Channels

### Channel Overview

| Channel | Command | Audience | Requirements | Toggleable |
|---------|---------|----------|--------------|------------|
| Shout | `shout` | All online players | Level 2+, 15 move | `AUTO_NOSHOUT` |
| Newbie | `newbie` | Newbies + helpers | Newbie account or `PLR_NEWBIEHELP` flag | No |
| Commune/Wiznet | `commune` | Immortals | `POWER_WIZNET` | `TOG_WIZBUILD` (global) |

### Shout Channel

Global broadcast to all awake players not blocking shouts.

```cpp
void TBeing::doShout(const sstring& arg);
```

**Source:** `code/code/misc/talk.cc:336-418`

**Restrictions:**
- Minimum level 2 required
- Costs 15 movement points per shout
- Adds 0.5 combat round wait state
- Charmed characters cannot shout
- Can be globally disabled via `toggleInfo[TOG_SHOUTING]`
- Blocked by `AUTO_NOSHOUT` autobit on receivers
- Blocked by `PLR_GODNOSHOUT` player flag (god-imposed communication ban)

**Distribution logic:**

```cpp
void Descriptor::sendShout(TBeing* ch, const sstring& arg);
```

**Source:** `code/code/misc/talk.cc:237-334`

Recipients must satisfy all conditions:
1. Connection state `CON_PLYNG`
2. Not sleeping (`getPosition() > POSITION_SLEEPING`)
3. Not in soundproof room
4. Not ignoring the shouter (`!ignored.isIgnored(ch->desc)`)
5. Not set `AUTO_NOSHOUT` (unless immortal shouting or receiver is a mob)

**GMCP integration:** Shouts send `comm.channel { "chan": "yell", ... }` for client integration.

**Twitter integration:** Shouts post via `twitterShout(getName(), garbled)`.

### Newbie Channel

Communication channel for new players to get help from experienced players and immortals.

```cpp
void TBeing::doNewbie(const sstring& arg);
```

**Source:** `code/code/misc/newbie.cc:103-180`

**Who can use:**
- **Newbies:** Account age less than `NEWBIE_PURGATORY_LENGTH` seconds
- **Helpers:** Immortals or players with `PLR_NEWBIEHELP` flag set

**Who receives:**
- All connected players who are either newbies or helpers
- Respects ignore list filtering
- Messages identified by sender type ("Newbie" vs "Expert")

**Message format:**
```
Newbie <name>: <message>   // from a newbie
Expert <name>: <message>   // from a helper
```

**Enabling helper status:**
```
toggle newbiehelper
```

### Commune/Wiznet Channel

Immortal-only communication for coordination and discussion.

```cpp
void TBeing::doCommune(const sstring& arg);
```

**Source:** `code/code/misc/talk.cc:531-669`

**Requirements:** `POWER_WIZNET` wizard power (or switched immortal with power)

**Level targeting:** `commune @<level> <message>` sends only to gods at or above specified level.

**Visibility controlled by:**
- `TOG_WIZBUILD` global toggle - if on, all builders see wiznet
- `POWER_WIZNET_ALWAYS` - always sees wiznet regardless of toggle

**GMCP:** Sends `comm.channel { "chan": "wiz", ... }`.

---

## Channel Restriction Toggles

### Autobits

Player-specific channel preferences stored in `desc->autobits`:

| Autobit | Default | Effect |
|---------|---------|--------|
| `AUTO_NOSHOUT` | Off | Blocks receiving shouts |
| `AUTO_NOTELL` | Off | Blocks incoming tells (except from last person you told) |
| `AUTO_AFK` | Off | Auto-AFK message after 5 minutes idle |
| `AUTO_PG13` | Off | Profanity filter on received messages |

**Source:** `code/code/misc/toggle.h:22-50`

### Toggle Commands

```
toggle noshout    - Stop receiving shouts
toggle notell     - Block incoming tells
toggle afk        - Auto-AFK mode
toggle pg13       - Profanity filter
```

**Source:** `code/code/misc/toggle.cc:951-1206`

### Global Toggles (Immortal)

| Toggle | Effect |
|--------|--------|
| `TOG_SHOUTING` | Globally disable shouting |
| `TOG_WIZBUILD` | Allow builders to see wiznet |

**Source:** `code/code/misc/toggle.h:90-114`

---

## Ignore System

Players can ignore other players to filter their communications across all channels.

### ignoreList Class

```cpp
class ignoreList {
    // Maximum entries per player
    const static unsigned int cMax = 20;

    bool isIgnored(Descriptor* desc);
    bool isIgnored(const sstring ignored);
    static bool isMailIgnored(Descriptor* desc, const sstring ignored);

    bool add(Descriptor* desc);
    bool add(const sstring name);
    bool add(const TAccount& acct);
    bool addAccount(const sstring name);
    bool remove(Descriptor* desc);
    bool remove(const sstring name);
    bool removeAccount(const sstring name);
};
```

**Source:** `code/code/sys/connect.h:367-416`

### Persistence

Ignore lists are stored in the `blockedlist` database table:

```cpp
void ignoreList::addDB(int playerId, const sstring ignored);
void ignoreList::removeDB(int playerId, const sstring ignored);
```

**Source:** `code/code/misc/other.cc:4796-4806`

Database schema:
- `player_id` - ID of the ignoring player
- `blocked` - Name of the ignored player/account

### Account-Wide Blocking

Immortals can block entire accounts by prefixing with `~`:

```cpp
bool ignoreList::addAccount(const sstring name) {
    return add("~" + name.lower());
}
```

**Source:** `code/code/misc/other.cc:4946-4948`

Requires `POWER_ACCESS` or `POWER_ACCOUNT` wizard power.

### Ignore Command

```cpp
void TBeing::doIgnore(const sstring& args);
```

**Source:** `code/code/misc/other.cc:5076-5166`

**Syntax:**
```
ignore                              - List your ignore list
ignore <player>                     - Add player to ignore list (shorthand)
ignore add <player>                 - Add player to ignore list
ignore remove <player>              - Remove player from ignore list
ignore addall <account>             - Block entire account (immortal only)
ignore removeall <account>          - Unblock account (immortal only)
```

### What Gets Filtered

The ignore check `desc->ignored.isIgnored(sender_desc)` filters:

| Communication | Filtered |
|--------------|----------|
| Say | Yes |
| Tell | Yes (silent failure - sender sees success) |
| Whisper | Yes |
| Shout | Yes |
| Grouptell | Yes |
| Newbie channel | Yes |
| Emote | Yes |
| Social actions | Yes |

**Important:** Tell appears to succeed from the sender's perspective even when ignored. This prevents harassment confirmation.

### Static List Optimization

For performance, small ignore lists share a static array:

```cpp
static bool m_staticUseStatic = false;
static int m_staticIds[cMax];
static sstring m_staticIgnored[cMax];
static unsigned int m_staticCount = 0;
```

When a player's list grows large, `convertFromStatic()` migrates to individual storage.

**Source:** `code/code/misc/other.cc:4741-4834`

---

## Time and Calendar System

### GameTime Class

Central time management for MUD time.

```cpp
class GameTime {
    static time_info_data time_info;
    static const unsigned long BEGINNING_OF_TIME;  // Aug 10, 1990
    static const int YEAR_ADJUST;                  // 550

    static int hourminTime();
    static sstring hmtAsString(int);
    static void anotherHour();
    static bool is_daytime();
    static bool is_nighttime();
    static void mudTimePassed(time_t t2, time_t t1, time_info_data* now);
    static void realTimePassed(time_t t2, time_t t1, time_info_data* now);
};
```

**Source:** `code/code/sys/gametime.h:24-61`

### Time Conversion

MUD time runs significantly faster than real time:

| MUD Unit | Real Time Equivalent |
|----------|---------------------|
| 15 MUD minutes | 1 tick (~72 seconds) |
| 1 MUD hour | 4 ticks (~4.8 minutes) |
| 1 MUD day | 24 MUD hours (~115 minutes) |
| 1 MUD month | 28 MUD days (~54 hours) |
| 1 MUD year | 12 MUD months (~27 days) |

**Source:** `code/code/sys/gametime.cc:154-175`

### Calendar Structure

| Element | Range | Notes |
|---------|-------|-------|
| Minutes | 0, 15, 30, 45 | Increments in 15-minute blocks |
| Hours | 0-23 | 24-hour format |
| Day | 0-27 | 28 days per month (displayed as 1-28) |
| Month | 0-11 | 12 months per year |
| Year | 550+ | "P.S." era (Post Something?) |

### Month Names

Standard calendar month names:

```cpp
const char* const month_name[12] = {
    "January", "February", "March", "April",
    "May", "June", "July", "August",
    "September", "October", "November", "December"
};
```

**Source:** `code/code/misc/constants.cc:252-264`

### Weekday Names

Standard weekday names:

```cpp
const char* const weekdays[7] = {
    "Sunday", "Monday", "Tuesday", "Wednesday",
    "Thursday", "Friday", "Saturday"
};
```

**Source:** `code/code/misc/constants.cc:249-250`

Weekday calculated as: `((28 * month) + day + 1) % 7`

### Seasons and Weather

Seasons affect weather patterns and sunrise/sunset times:

| Month | Season | Weather Pattern |
|-------|--------|-----------------|
| 0-2 (Jan-Mar) | Winter | Snow, cold |
| 3-5 (Apr-Jun) | Spring | Rain, warming |
| 6-8 (Jul-Sep) | Summer | Warm, humid |
| 9-11 (Oct-Dec) | Autumn | Cooling, rain |

**Source:** `code/code/misc/weather.cc:529-586`

### Sunrise/Sunset Calculation

Day length varies seasonally using sinusoidal calculation:

```cpp
void Weather::calcNewSunRise() {
    int day = (GameTime::getMonth()) * 28 + GameTime::getDay() + 1;
    int equinox = 3 * 28 + 1;  // April 1st

    // Sinusoidal variation: [-1.5, +1.5] hours from 6 AM
    double x = sin(2 * M_PI * ((double)(day - equinox)) / (28.0 * 12.0));
    x *= -1.5;

    si_sunRise = (6 * 4 + 0) + (int)(x * 4 + 0.5);
}
```

**Source:** `code/code/misc/weather.cc:831-869`

| Season | Daylight Hours | Sunrise | Sunset |
|--------|----------------|---------|--------|
| Winter Solstice | ~9 hours | ~7:30 AM | ~4:30 PM |
| Equinox | ~12 hours | ~6:00 AM | ~6:00 PM |
| Summer Solstice | ~15 hours | ~4:30 AM | ~7:30 PM |

### Time Command

Players can check the current MUD time:

```cpp
void TBeing::doTime(const char* argument);
```

**Source:** `code/code/misc/info.cc:2235-2310`

**Output includes:**
- Current MUD time (hour:minute AM/PM)
- Day of week
- Day of month and month name
- Year in P.S. era
- Sunrise/sunset times
- Moonrise/moonset times and moon phase
- Real-world time (with optional timezone offset)

**Timezone adjustment:**
```
time <offset>    - Set hour offset from server time
```

Stored in `desc->account->time_adjust`.

### Moon Phases

Moon cycle is 32 days:

| Moon Value | Phase |
|------------|-------|
| 0-3 | New |
| 4-11 | Waxing |
| 12-19 | Full |
| 20-27 | Waning |
| 28-31 | New |

**Source:** `code/code/misc/weather.cc:31-42`

Full moon provides +1 outdoor light at night.

---

## Key Source Files

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

---

## Common Gotchas

### Ignore Silent Failure on Tell

Tells to ignored players appear to succeed from the sender's perspective:

```cpp
// If ignoring, silently succeed - sender doesn't know
if (d && d->ignored.isIgnored(desc))
    return FALSE;
```

This is intentional to prevent harassment confirmation.

### Ignore List Maximum

Each player can only ignore up to 20 entries (`cMax = 20`). Check before adding:

```cpp
if (desc->ignored.getCount() >= desc->ignored.getMax()) {
    // Cannot add more - list is full
}
```

### Account vs Player Blocking

Account blocks are stored with `~` prefix:

```cpp
// Player block: "playername"
// Account block: "~accountname"
```

When checking ignore status, the code checks both player name and account.

### Time Command vs Real Time

The `time` command shows MUD time by default. Real-world time is displayed with the timezone offset applied from `desc->account->time_adjust`.

### Shout Movement Cost

Shouts cost 15 movement points. Characters with insufficient move cannot shout:

```cpp
if ((getMove() < 15) && isPc()) {
    sendTo("You don't have the energy to shout!\n\r");
    return;
}
```

### Newbie Purgatory Length

The newbie channel access is time-limited by account age. The constant `NEWBIE_PURGATORY_LENGTH` defines how long an account is considered "newbie."

---

## Related Documentation

- [Communication System](communication-system.md) - In-character channels and garble system
- [Wizard Powers](wizard-powers.md) - POWER_WIZNET and access control
- [Scheduler Pulses](scheduler-pulses.md) - Tick timing and MUD time progression
- [Configuration Reference](configuration-reference.md) - Global toggle settings
