---
title: Faction System
description: Political and religious organizations with membership management, leadership hierarchies, inter-faction relationships, and divine integration for clerics and deikhans, currently disabled for percentage tracking.
keywords:
  - factionTypeT
  - TFactionInfo
  - FACT_BROTHERHOOD
  - FACT_CULT
  - FACT_SNAKE
  - faction-percentage
  - reconcileHelp
  - reconcileHurt
  - percModifier
  - caravan-system
  - potency-system
  - corporation-wealth
  - leadership-commands
  - FACTIONS_IN_USE
  - tithe-system
category: Understanding Systems
related:
  - affects-system.md
  - database-queries.md
  - economy-system.md
last_updated: 2026-01-29
source_files:
  - code/code/misc/faction.h
  - code/code/misc/faction.cc
  - code/code/misc/being.h
  - code/code/misc/person.h
  - code/code/misc/utility.cc
  - code/code/misc/damage.cc
  - code/code/misc/combat.cc
  - _Setup-data/sql_tables/sneezy/factionmembers.sql
---

# Faction System

The faction system provides political/religious organizations for players, with membership management, leadership hierarchies, inter-faction relationships, and divine integration for clerics and deikhans.

**Current Status: DISABLED** (`FACTIONS_IN_USE = 0` in faction.h). Faction percentage tracking is not active, but membership management, leadership commands, caravans, and wealth tracking remain functional.

## Overview

Factions represent political/religious organizations dedicated to specific causes. Each faction has its own beliefs, hierarchy, rules of conduct, and relationships with other factions.

**Three Active Factions:**
- **Brotherhood of Galek** (base: Brightmoon, vnum 1395)
- **Cult of Logrus** (base: Logrus, vnum 3768)
- **Order of the Serpents** (base: Amber, vnum 8713)

**Unaffiliated Option:**
- Players can remain unaffiliated (base: Grimhaven, vnum 432)
- Provides access to neutral deity options
- No faction percentage tracking or potency effects

## Core Data Structures

### factionTypeT Enumeration

```cpp
enum factionTypeT {
  FACT_UNDEFINED = -1,    // Invalid/error state
  FACT_NONE = 0,          // Unaffiliated
  FACT_BROTHERHOOD,       // Brotherhood of Galek
  FACT_CULT,              // Cult of Logrus
  FACT_SNAKE,             // Order of the Serpents
  MAX_FACTIONS,           // Iterator limit (4)
};

const factionTypeT ABS_MAX_FACTION = factionTypeT(6);  // charFile size limit
const int MIN_FACTION = FACT_NONE;
```

**Source:** `code/code/misc/faction.h`

**Critical:** `ABS_MAX_FACTION = 6` affects binary charFile format. Changing this value corrupts player saves. The actual number of active factions is limited by `MAX_FACTIONS = 4`.

### TFactionInfo Class

```cpp
class TFactionInfo {
  public:
    char* faction_name{nullptr};           // Display name
    char* leader[FACT_LEADER_SLOTS]{nullptr}; // 4 leadership slots
    char* faction_password{nullptr};       // Membership password
    double faction_array[MAX_FACTIONS][2]{{0.0}};  // Help/hurt relationships
    double faction_power{0.0};             // Potency pool
    int corp_id{0};                        // Corporation for wealth
    double faction_tithe{0.0};             // Tithe revenue tracking

    // Caravan system
    int caravan_interval{0};               // Spawn interval
    int caravan_counter{0};                // Time until next spawn
    unsigned int caravan_flags{0};         // Configuration flags
    int caravan_value{0};                  // Talens per caravan
    int caravan_defense{0};                // Defense spending
    int caravan_attempts{0};               // Caravans launched
    int caravan_successes{0};              // Caravans completed
};
```

**Source:** `code/code/misc/faction.h`

## Faction Identification

### Name Parsing

```cpp
factionTypeT factionNumber(const sstring name) {
  if (is_abbrev(name, "brotherhood") || is_abbrev(name, "galek"))
    return FACT_BROTHERHOOD;
  else if (is_abbrev(name, "cult") || is_abbrev(name, "chaos") ||
           is_abbrev(name, "logrus"))
    return FACT_CULT;
  else if (is_abbrev(name, "order") || is_abbrev(name, "serpents") ||
           is_abbrev(name, "snakes"))
    return FACT_SNAKE;
  else
    return FACT_UNDEFINED;
}
```

**Accepted Aliases:**

| Faction | Primary Name | Aliases |
|---------|-------------|---------|
| `FACT_BROTHERHOOD` | "brotherhood" | "galek" |
| `FACT_CULT` | "cult" | "chaos", "logrus" |
| `FACT_SNAKE` | "order" | "serpents", "snakes" |

**Source:** `code/code/misc/faction.cc`

## Leadership System

### Leadership Slots

```cpp
const int FACT_LEADER_SLOTS = 4;  // 1 primary + 3 subleaders
```

**Authority Levels:**
- **Slot 0** (Primary Leader): Full authority over all faction commands
- **Slots 1-3** (Subleaders): Varying authority levels based on faction-specific policies

### Leadership Commands

| Command | Authority Required | Purpose |
|---------|-------------------|---------|
| `newmember` | Leader/subleader | Add new faction member |
| `rmember` | Leader/subleader | Remove faction member |
| `makeleader` | Primary leader | Promote/demote leaders |
| `adjust` | Varies by faction | Modify help/hurt values (disabled) |
| `send` | Leader/subleader | Send messages to faction |

**Source:** `code/code/misc/faction.cc`, `lib/help/faction leaders`

## Faction Percentage System (DISABLED)

When `FACTIONS_IN_USE = 1`, faction percentage tracks player loyalty/purity from -100 to +100, affecting cleric/deikhan prayer power.

### Tracking Mechanism (When Enabled)

```cpp
void TPerson::reconcileHelp(TBeing* victim, double amp) {
  #if FACTIONS_IN_USE
    // Get inter-faction relationship values
    factionTypeT myFact = getFaction();
    factionTypeT victFact = victim->getFaction();

    double help = faction_data[victFact].faction_array[myFact][OFF_HELP];
    double hurt = faction_data[victFact].faction_array[myFact][OFF_HURT];

    // Update individual faction percentages
    setPercX(getPercX(victFact) + help * amp, victFact);

    // Calculate composite percentage from all 4 faction values
    double composite = 0.0;
    for (int i = MIN_FACTION; i < MAX_FACTIONS; i++)
      composite += getPercX(i);
    setPerc(composite);

    // Adjust faction power pools
    faction_data[myFact].faction_power += help * amp / 5.0;
  #endif
}
```

**Source:** `code/code/misc/faction.cc`

### reconcileHurt() (When Enabled)

Mirror of `reconcileHelp()` but uses `OFF_HURT` values and subtracts from percentages. Called when:
- Dealing damage to beings
- Killing beings
- Stealing from beings
- Other harmful actions

### Integration Points

Faction percentage would be updated when `FACTIONS_IN_USE = 1`:

| Action | Function | File |
|--------|----------|------|
| Tithes/donations | `doPray()` | utility.cc |
| Damage dealing | `applyDamage()` | damage.cc |
| Healing | `doHealLight()` | offense.cc |
| Combat | `perform_violence()` | combat.cc |
| Group sharing | `gainExpPerHit()` | other.cc |

**Source:** Grep search for reconcileHelp/reconcileHurt

### Current Behavior (Disabled)

```cpp
void TPerson::reconcileHelp(TBeing* victim, double amp) {
  #if FACTIONS_IN_USE
    // All tracking code here
  #endif
  // When disabled: function does nothing
}
```

New members have faction percentages zeroed but not tracked:

```cpp
void TBeing::doNewMember(const char* arg) {
  vict->setFaction(fnum);
  #if FACTIONS_IN_USE
    vict->setPerc(0.00);
    for (i = MIN_FACTION; i < MAX_FACTIONS; i++)
      vict->setPercX(0.0, i);
  #endif
}
```

**Source:** `code/code/misc/faction.cc`

## Prayer Power Modifier

### percModifier() Formula

```cpp
float TBeing::percModifier() const {
  #if FACTIONS_IN_USE
    return (getPerc() + 100.0) / 200.0;  // Range: 0.0 to 1.0
  #else
    return 0.75;  // Fixed 75% when disabled
  #endif
}
```

**When Enabled:**
- Faction percentage -100 → 0.0× prayer power (0% effective)
- Faction percentage 0 → 0.5× prayer power (50% effective)
- Faction percentage +100 → 1.0× prayer power (100% effective)

**When Disabled:**
- All clerics/deikhans receive constant 0.75× prayer power (75% effective)

**Source:** `code/code/misc/faction.cc`

## Inter-Faction Relationships

### Faction Array Structure

```cpp
double faction_array[MAX_FACTIONS][2];

// Indices
const int OFF_HELP = 0;  // Help value index
const int OFF_HURT = 1;  // Hurt value index
```

**Relationship Values:** Range -4.0 to +4.0

**Example Configuration:**
```cpp
// Brotherhood helping Brotherhood members
faction_data[FACT_BROTHERHOOD].faction_array[FACT_BROTHERHOOD][OFF_HELP] = 4.0;

// Brotherhood hurting Cult members
faction_data[FACT_CULT].faction_array[FACT_BROTHERHOOD][OFF_HURT] = 3.0;
```

### Adjust Command (Blocked When Disabled)

```cpp
#if !FACTIONS_IN_USE
  sendTo("You are not permitted to alter the help/harm values "
         "because faction percent is not in use.\n\r");
  return;
#endif
```

**Source:** `code/code/misc/faction.cc`

## Potency System

**Potency** represents faction deity power, affecting:
- Willingness to grant piety regeneration
- Save chances for followers
- Overall faction influence

**Power Sources (When Enabled):**
- Faction member actions feeding help/hurt values
- Composite faction percentage of all members
- Divine spell usage draining power

**Current Status:** Potency tracking works but has no effect with `FACTIONS_IN_USE = 0`.

## Caravan System

Factions run trade caravans between cities to generate revenue.

### Caravan Fields

| Field | Purpose |
|-------|---------|
| `caravan_interval` | Ticks between caravan spawns |
| `caravan_counter` | Countdown to next spawn |
| `caravan_flags` | Configuration (route, protection, etc.) |
| `caravan_value` | Talens per successful delivery |
| `caravan_defense` | Money spent on guards |
| `caravan_attempts` | Total caravans launched |
| `caravan_successes` | Caravans that reached destination |

**Caravan Routes:**
- Brightmoon (1395) ↔ other cities
- Logrus (3768) ↔ other cities
- Amber (8713) ↔ other cities
- Grimhaven (432) ↔ other cities (unaffiliated)

### Caravan Spawning

Periodic process spawns caravan mobs with faction goods. Players can raid enemy caravans or protect allied ones.

## Corporate Wealth Integration

### Corporation Link

```cpp
int corp_id;  // TFactionInfo member
```

Each faction has an associated corporation for wealth management:
- Faction funds stored in corporate bank account
- Shop ownership tracked per faction
- Revenue from tithes and caravans deposited
- Leadership can authorize withdrawals

### Tithe System

```cpp
const float TITHE_FACTOR = (0.0003);

// Tithes convert talens to faction percentage (when enabled)
double percentIncrease = talens * TITHE_FACTOR;
```

**Tithe Mechanics:**
- Players donate money to faction through prayer
- Talens deposited to faction corporation
- When `FACTIONS_IN_USE = 1`: Donation increases faction percentage via `reconcileHelp()`
- When disabled: Donation only affects corporate wealth

**Source:** `code/code/misc/faction.h`, `code/code/misc/utility.cc`

## Deity Integration

### Deity Mapping

Each faction associates with different deities for divine spell flavor:

```cpp
sstring TBeing::yourDeity() const {
  factionTypeT faction = getFaction();
  // Returns deity name based on faction
  // Different deities for different factions
  // Affects flavor text, not mechanics
}
```

**Deity Selection:**
- Brotherhood: Specific deity set
- Cult: Chaos-aligned deities
- Order: Serpent-aligned deities
- Unaffiliated: Neutral deities

## Holy Symbol Attunement

Holy symbols can be attuned to:
- Specific faction clerics/deikhans
- Unaffiliated clerics/deikhans
- Affects symbol effectiveness and divine connection

**Attunement Mechanics:**
- Symbols gain power from aligned users
- Unaligned usage may be less effective
- Faction membership affects attunement options

## Faction Scoring System

Five metrics track faction competition:

### 1. Average Level

**Not a true average** - biased toward higher levels:
- One level 50 worth more than two level 25s
- Rewards high-level membership
- Prevents padding with low-level alts

### 2. Pounds of Fish

Based on:
- Number of fishing records held (1 point each)
- Total pounds of fish caught
- Requires extensive fishing to earn significant points

### 3. Average Trophy

**Not a true average** - biased toward higher levels:
- Level 50 with 50% trophy ≈ two level 25s with 50% trophy each
- Combines trophy percentage with member levels
- Rewards both quality and achievement

### 4. Shops Owned

Direct count of shops where faction members have owner privileges:
- Each shop counted once per faction
- Multiple owners from same faction don't increase score
- Encourages shop ownership diversity

### 5. Faction Wealth

Based directly on talens in faction corporate bank account:
- Transparent metric
- Affected by tithes, caravans, shop revenue
- Can be spent on faction operations

**Source:** `lib/help/faction score`

## Membership Management

### Joining a Faction

**Requirements:**
- Must not be affiliated with another faction
- Know faction password
- Receive approval from faction leader
- Leader executes `newmember` command

**Process:**
```cpp
void TBeing::doNewMember(const char* arg) {
  // Validate permissions
  // Parse target name
  // Set faction
  // Zero faction percentages (if enabled)
  // Update database
  // Send notifications
}
```

### Leaving a Faction

**Methods:**
- `disband` command (self-initiated)
- `rmember` command (leader-initiated)

**Effects:**
- Faction set to `FACT_NONE`
- Faction percentage drops (if enabled)
- Removed from faction member database
- Corporate access revoked

### Database Tracking

```sql
CREATE TABLE factionmembers (
  name varchar(80) NOT NULL,
  faction varchar(8),
  level int(11)
) ENGINE=InnoDB DEFAULT CHARSET=latin1;
```

Used for:
- Faction rollcall displays
- Membership verification
- Level tracking for scoring

**Source:** `_Setup-data/sql_tables/sneezy/factionmembers.sql`

## OOC Rules (Immortal-Enforced)

### Rule 1: No Multi-Faction Alts

**Policy:** No person may have characters in separate factions.

**Rationale:** Prevents sabotage and insider information abuse.

**Enforcement:** Immortal verification and punishment.

### Rule 2: No God Alt Membership

**Policy:** Characters known to be the mortals of currently active gods are denied faction membership.

**Rationale:** Perception of cheating too great; unfair advantage.

**Enforcement:** Immortal verification before membership approval.

**Source:** `lib/help/faction rules`

## Persistence

### File-Based Storage

**Location:** `lib/faction/faction_info`

**Backup:** `lib/faction/faction_info.bak`

**Content:** Binary serialization of `TFactionInfo` structures for all factions.

### Save Triggers

Faction data saved when:
- Leadership changes
- Relationships modified
- Caravan spawned/completed
- Wealth deposited/withdrawn
- Member added/removed

### Load at Boot

```cpp
void boot_factions() {
  // Read lib/faction/faction_info
  // Populate faction_data[] array
  // Initialize caravan counters
  // Validate data integrity
}
```

## Character Storage

### TBeing Members

```cpp
class TBeing {
  protected:
    factionTypeT faction;        // Current faction
    double f_perc;               // Composite faction percentage
    double f_percx[ABS_MAX_FACTION]; // Individual faction percentages
};
```

**Accessor Methods:**

| Method | Purpose |
|--------|---------|
| `getFaction()` | Returns current faction |
| `setFaction(factionTypeT)` | Changes faction |
| `getPerc()` | Returns composite percentage |
| `setPerc(double)` | Sets composite percentage |
| `getPercX(factionTypeT)` | Returns individual faction percentage |
| `setPercX(double, factionTypeT)` | Sets individual faction percentage |

**Source:** `code/code/misc/being.h`

### charFile Storage

```cpp
// In charFile binary format
short f_type;                      // Faction number
double f_percent;                  // Composite percentage
double f_percx[ABS_MAX_FACTION];   // Individual percentages
```

**Critical:** `ABS_MAX_FACTION = 6` is frozen in binary format. Cannot change without player wipe.

## Working vs Disabled Features

### Fully Functional (FACTIONS_IN_USE = 0)

| Feature | Status |
|---------|--------|
| Faction membership | ✓ Working |
| Leadership commands | ✓ Working |
| Member database tracking | ✓ Working |
| Faction communication | ✓ Working |
| Caravan spawning | ✓ Working |
| Corporate wealth | ✓ Working |
| Tithe deposits | ✓ Working (wealth only) |
| Deity flavor text | ✓ Working |
| Holy symbol attunement | ✓ Working |

### Disabled Features (FACTIONS_IN_USE = 0)

| Feature | Status |
|---------|--------|
| Faction percentage tracking | ✗ Disabled |
| reconcileHelp/reconcileHurt | ✗ Empty functions |
| Dynamic prayer power | ✗ Fixed at 0.75× |
| Help/hurt adjustment | ✗ Blocked |
| Potency effects | ✗ No gameplay impact |
| Percentage-based scoring | ✗ Not calculated |

### Conditional Compilation Blocks

40+ `#if FACTIONS_IN_USE` blocks throughout codebase:

**Major Locations:**

| File | Blocks | Purpose |
|------|--------|---------|
| `faction.cc` | 15+ | reconcileHelp, reconcileHurt, adjust command, member management |
| `person.h` | 2 | Virtual function declarations |
| `being.h` | 2 | Base virtual implementations |
| `utility.cc` | 2 | Tithe percentage increases |
| `damage.cc` | 1 | Hurt on damage |
| `offense.cc` | 1 | Help on healing |
| `combat.cc` | 1 | Hurt in combat |
| `other.cc` | 2 | Group sharing adjustments |

## Key Source Files

| File | Contents |
|------|----------|
| `code/code/misc/faction.h` | TFactionInfo class, constants, enums |
| `code/code/misc/faction.cc` | Full implementation, commands, persistence |
| `code/code/misc/being.h` | Accessor methods, virtual declarations |
| `code/code/misc/person.h` | TPerson reconcile functions |
| `_Setup-data/sql_tables/sneezy/factionmembers.sql` | Membership database schema |
| `lib/faction/faction_info` | Faction data persistence |
| `lib/help/factions` | Basic help entry |
| `lib/help/faction overview` | Comprehensive overview |
| `lib/help/faction percent` | Percentage mechanic |
| `lib/help/faction leaders` | Leadership system |
| `lib/help/faction rules` | OOC rules |
| `lib/help/faction score` | Scoring metrics |
