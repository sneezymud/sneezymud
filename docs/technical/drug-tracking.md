---
title: Drug Tracking System
description: Manages player drug consumption, effects, and addiction mechanics through a hybrid ephemeral descriptor storage and database persistence system where current consumption data does not persist across sessions.
keywords:
  - drugData
  - drugTypeT
  - TDrug
  - TDrugContainer
  - doSmoke
  - saveDrugStats
  - loadDrugStats
  - applyDrugAffects
  - applyAddictionAffects
  - AFFECT_DRUG
  - descriptor-storage
  - current-consumed
  - addiction-mechanics
  - withdrawal-system
  - drug-effects
category: Understanding Systems
related:
  - affects-system.md
  - object-types.md
  - scheduler-pulses.md
  - persistence-rent.md
  - delete-flags.md
last_updated: 2026-01-29
source_files:
  - code/code/obj/obj_drug.h
  - code/code/obj/obj_drug.cc
  - code/code/obj/obj_drug_container.h
  - code/code/obj/obj_drug_container.cc
  - code/code/sys/connect.h
  - _Setup-data/sql_tables/sneezy/drug_use.sql
---

# Drug Tracking System

The drug tracking system manages player drug consumption, effects, and addiction mechanics. Understanding this system is important because **drug state is stored in the descriptor and lost on disconnect**, meaning current consumption data does not persist across sessions.

**Misusing this system causes data loss.** Common errors: assuming drug state persists, not handling null descriptors, failing to call saveDrugStats() after consumption updates.

## Overview

 SneezyMUD tracks drug consumption through a hybrid system:
- **Ephemeral state** in `Descriptor::drugs[]` tracks current session consumption
- **Database persistence** in `drug_use` table stores historical usage
- **Drug effects** applied via the affects system (AFFECT_DRUG)
- **Addiction mechanics** based on consumption patterns and time elapsed

**Critical limitation:** The `current_consumed` field resets to 0 on disconnect because it lives in the descriptor, not the character file.

## Drug Types

Four drug types are implemented via the `drugTypeT` enum:

```cpp
enum drugTypeT {
  DRUG_NONE = 0,
  DRUG_PIPEWEED = 1,
  DRUG_OPIUM = 2,
  DRUG_POT = 3,
  DRUG_FROGSLIME = 4,
  MAX_DRUG = 5
};
```

**Source:** `code/code/obj/obj_drug.h`

| Drug Type | Primary Effects | Addiction Risk | Special Notes |
|-----------|----------------|----------------|---------------|
| Pipeweed | Relaxation, minor stat changes | Low | Hobbits get bonuses, other races penalties |
| Opium | Heavy intoxication, stat penalties | High | Strong addiction, severe withdrawal |
| Pot | Perception changes, mild paranoia | Medium | Moderate stat effects |
| Frogslime | Hallucinogenic, major stat swings | Medium | Unique visual effects |

## Drug Data Structure

### drugData Class

```cpp
class drugData {
  public:
    time_info_data first_use;      // Game time of first consumption
    time_info_data last_use;       // Game time of last consumption
    unsigned int total_consumed;   // Lifetime consumption count
    unsigned int current_consumed; // Current session consumption (LOST ON DISCONNECT)
};
```

**Source:** `code/code/obj/obj_drug.h`

### Descriptor Storage

Each descriptor maintains a `drugData` array for all drug types:

```cpp
// In Descriptor class
drugData drugs[MAX_DRUG];
```

**Access pattern:**
```cpp
// Always check descriptor exists before accessing
if (ch->desc) {
    ch->desc->drugs[DRUG_OPIUM].current_consumed++;
}
```

### Database Schema

The `drug_use` table provides persistence:

```sql
CREATE TABLE `drug_use` (
  `drug_id` int(11),           -- drugTypeT enum value
  `player_id` int(11),         -- Player character ID
  `first_use_sec` int(11),     -- time_info_data.seconds
  `first_use_min` int(11),     -- time_info_data.minutes
  `first_use_hour` int(11),    -- time_info_data.hours
  `first_use_day` int(11),     -- time_info_data.day
  `first_use_mon` int(11),     -- time_info_data.month
  `first_use_year` int(11),    -- time_info_data.year
  `last_use_sec` int(11),      -- Last consumption timestamp
  `last_use_min` int(11),
  `last_use_hour` int(11),
  `last_use_day` int(11),
  `last_use_mon` int(11),
  `last_use_year` int(11),
  `total_consumed` int(11),    -- Persists across sessions
  `current_consumed` int(11),  -- Saved but resets to 0 in descriptor
  KEY `ix_drug_use_player_id` (`player_id`)
) ENGINE=InnoDB DEFAULT CHARSET=latin1;
```

**Source:** `_Setup-data/sql_tables/sneezy/drug_use.sql`

## Drug Consumption Flow

### The doSmoke() Command

Primary entry point for drug use:

```cpp
int TBeing::doSmoke(const char* argument) {
  // 1. Validate drug container and contents
  TDrugContainer* tdc = dynamic_cast<TDrugContainer*>(obj);
  if (!tdc || tdc->getDrugType() == DRUG_NONE)
    return FALSE;

  // 2. Check if lit
  if (!tdc->isLit()) {
    sendTo("You need to light it first.\n\r");
    return FALSE;
  }

  // 3. Update descriptor stats (EPHEMERAL)
  if (desc) {
    if (!desc->drugs[tdc->getDrugType()].total_consumed) {
      // First time using this drug
      desc->drugs[tdc->getDrugType()].first_use.seconds = GameTime::getSeconds();
      desc->drugs[tdc->getDrugType()].first_use.minutes = GameTime::getMinutes();
      // ... initialize other timestamp fields
    }
    desc->drugs[tdc->getDrugType()].total_consumed++;
    desc->drugs[tdc->getDrugType()].current_consumed++;
    desc->drugs[tdc->getDrugType()].last_use.seconds = GameTime::getSeconds();
    // ... update other timestamp fields
  }

  // 4. Save to database
  saveDrugStats();

  // 5. Apply drug effects
  applyDrugAffects(this, tdc->getDrugType(), false);

  return TRUE;
}
```

**Source:** `code/code/obj/obj_drug.cc`

### Persistence: saveDrugStats()

```cpp
void TBeing::saveDrugStats() {
  if (!desc || !isPc())
    return;

  for (int drug = DRUG_PIPEWEED; drug < MAX_DRUG; drug++) {
    // Delete old record
    db.query("delete from drug_use where player_id=%i and drug_id=%i",
             getPlayerID(), drug);

    // Insert updated record
    if (desc->drugs[drug].total_consumed > 0) {
      db.query("insert into drug_use values (%i, %i, %i, %i, %i, %i, %i, %i, "
               "%i, %i, %i, %i, %i, %i, %i, %i)",
               drug, getPlayerID(),
               desc->drugs[drug].first_use.seconds,
               desc->drugs[drug].first_use.minutes,
               // ... all timestamp fields
               desc->drugs[drug].total_consumed,
               desc->drugs[drug].current_consumed);
    }
  }
}
```

**Source:** `code/code/obj/obj_drug.cc`

### Loading: loadDrugStats()

```cpp
void TBeing::loadDrugStats() {
  if (!desc || !isPc())
    return;

  db.query("select * from drug_use where player_id=%i", getPlayerID());

  while (db.fetchRow()) {
    int drug = convertTo<int>(db["drug_id"]);
    if (drug < DRUG_PIPEWEED || drug >= MAX_DRUG)
      continue;

    // Load timestamps
    desc->drugs[drug].first_use.seconds = convertTo<int>(db["first_use_sec"]);
    // ... load other timestamp fields

    // Load consumption counts
    desc->drugs[drug].total_consumed = convertTo<int>(db["total_consumed"]);
    desc->drugs[drug].current_consumed = convertTo<int>(db["current_consumed"]);
  }
}
```

**Source:** `code/code/obj/obj_drug.cc`

**CRITICAL BUG:** `current_consumed` is loaded from the database on login, but this value was saved from the previous session. Since the descriptor is ephemeral, this creates inconsistent behavior where `current_consumed` appears to persist but actually represents stale data from the last disconnect.

## Drug Effects System

### Effect Application: applyDrugAffects()

```cpp
void applyDrugAffects(TBeing* ch, drugTypeT drug, bool silent) {
  affectedData aff;
  aff.type = AFFECT_DRUG;
  aff.level = 0;
  aff.duration = Pulse::UPDATES_PER_MUDHOUR;  // 1 game hour

  // Get current consumption (with null check)
  auto current_consumed = ch->desc ? ch->desc->drugs[drug].current_consumed : 0;

  switch (drug) {
    case DRUG_PIPEWEED:
      // Hobbit bonus
      if (ch->getRace() == RACE_HOBBIT) {
        aff.location = APPLY_FOC;
        aff.modifier = 5;
        ch->affectTo(&aff);
      } else {
        // Non-hobbits get penalties
        aff.location = APPLY_INT;
        aff.modifier = -5;
        ch->affectTo(&aff);
      }
      break;

    case DRUG_OPIUM:
      // Heavy stat penalties, increases with consumption
      aff.location = APPLY_DEX;
      aff.modifier = -(5 + current_consumed / 2);
      ch->affectTo(&aff);

      aff.location = APPLY_INT;
      aff.modifier = -(10 + current_consumed);
      ch->affectTo(&aff);
      break;

    case DRUG_POT:
      // Perception changes
      aff.location = APPLY_PER;
      aff.modifier = -5;
      ch->affectTo(&aff);
      break;

    case DRUG_FROGSLIME:
      // Major stat swings
      aff.location = APPLY_CHA;
      aff.modifier = 10;
      ch->affectTo(&aff);

      aff.location = APPLY_WIS;
      aff.modifier = -15;
      ch->affectTo(&aff);
      break;
  }
}
```

**Source:** `code/code/obj/obj_drug.cc`

**Key pattern:** Critical descriptor dependency:
```cpp
auto current_consumed = ch->desc ? ch->desc->drugs[drug].current_consumed : 0;
```

If descriptor is null, `current_consumed` defaults to 0, eliminating all consumption-scaled effects.

## Addiction and Withdrawal System

### Withdrawal Calculation: applyAddictionAffects()

```cpp
void applyAddictionAffects(TBeing* ch, drugTypeT drug) {
  if (!ch->desc)
    return;

  // Calculate time since last use
  time_info_data current_time = GameTime::getTime();
  time_info_data last_use = ch->desc->drugs[drug].last_use;

  int hours_since = timeDiff(current_time, last_use);

  // Calculate average consumption rate
  time_info_data first_use = ch->desc->drugs[drug].first_use;
  int total_hours = timeDiff(current_time, first_use);

  if (total_hours == 0)
    return;

  float avg_per_hour = (float)ch->desc->drugs[drug].total_consumed / total_hours;

  // Withdrawal threshold varies by drug
  int threshold = 0;
  switch (drug) {
    case DRUG_PIPEWEED: threshold = 24; break;  // 1 day
    case DRUG_OPIUM:    threshold = 6;  break;  // 6 hours (highly addictive)
    case DRUG_POT:      threshold = 12; break;  // 12 hours
    case DRUG_FROGSLIME: threshold = 18; break; // 18 hours
  }

  // Apply withdrawal if overdue
  if (hours_since > threshold && avg_per_hour > 0.5) {
    affectedData aff;
    aff.type = AFFECT_DRUG;
    aff.duration = Pulse::UPDATES_PER_MUDHOUR * 2;

    // Withdrawal severity scales with addiction level
    int severity = (int)(avg_per_hour * (hours_since - threshold));

    aff.location = APPLY_STR;
    aff.modifier = -(severity / 2);
    ch->affectTo(&aff);

    aff.location = APPLY_CON;
    aff.modifier = -(severity / 3);
    ch->affectTo(&aff);

    ch->sendTo("You feel terrible withdrawal symptoms!\n\r");
  }
}
```

**Source:** `code/code/obj/obj_drug.cc`

### Addiction Triggers

Withdrawal effects are applied in several contexts:
1. **Periodic pulse** - `procCharTickUpdate` checks drug state
2. **On login** - After `loadDrugStats()` completes
3. **Combat** - Can trigger mid-fight if enough time has passed

## TDrug and TDrugContainer Classes

### TDrug - Drug Items

```cpp
class TDrug : public TObj {
  private:
    int curFuel;      // Current fuel units remaining
    int maxFuel;      // Maximum fuel capacity
    drugTypeT drugType;  // Which drug this is

  public:
    int getCurFuel() const { return curFuel; }
    void setCurFuel(int n) { curFuel = n; }
    int getMaxFuel() const { return maxFuel; }
    void setMaxFuel(int n) { maxFuel = n; }
    drugTypeT getDrugType() const { return drugType; }
    void setDrugType(drugTypeT d) { drugType = d; }

    itemTypeT itemType() const { return ITEM_DRUG; }
};
```

**Source:** `code/code/obj/obj_drug.h`

**val0-val3 storage:**
- val0: `curFuel`
- val1: `maxFuel`
- val2: `drugType` (cast to int)
- val3: Unused

### TDrugContainer - Smoking Devices

```cpp
class TDrugContainer : public TObj {
  protected:
    drugTypeT drugType;  // Type of drug currently loaded
    int maxBurn;         // Maximum burn time
    int curBurn;         // Current burn time remaining
    bool lit;            // Whether pipe is lit

  public:
    drugTypeT getDrugType() const { return drugType; }
    void setDrugType(drugTypeT d) { drugType = d; }
    int getMaxBurn() const { return maxBurn; }
    void setMaxBurn(int n) { maxBurn = n; }
    int getCurBurn() const { return curBurn; }
    void setCurBurn(int n) { curBurn = n; }
    bool isLit() const { return lit; }
    void setLit(bool b) { lit = b; }

    itemTypeT itemType() const { return ITEM_DRUG_CONTAINER; }
};
```

**Source:** `code/code/obj/obj_drug_container.h`

**val0-val3 storage:**
- val0: `drugType` (cast to int)
- val1: `maxBurn`
- val2: `curBurn`
- val3: `lit` (boolean as int)

### Lighting and Burning

```cpp
void TDrugContainer::lightDecay() {
  if (!isLit())
    return;

  // Burn one unit per pulse
  setCurBurn(getCurBurn() - 1);

  if (getCurBurn() <= 0) {
    // Pipe burned out
    setLit(false);
    setCurBurn(0);

    // Check if in room and notify
    if (roomp) {
      act("$p goes out.", FALSE, this, NULL, NULL, TO_ROOM);
    }
  }
}
```

**Source:** `code/code/obj/obj_drug_container.cc`

**Burn process triggered by:** `procObjTickUpdate` scheduler process calls `lightDecay()` on all lit drug containers each pulse.

## Integration with Affects System

Drug effects use the standard affects system:

### Affect Structure

```cpp
affectedData aff;
aff.type = AFFECT_DRUG;           // Identifies as drug effect
aff.level = 0;                    // Not level-scaled
aff.duration = Pulse::UPDATES_PER_MUDHOUR;  // 1 game hour
aff.modifier = -10;               // Stat penalty/bonus
aff.location = APPLY_INT;         // Which stat to modify
aff.bitvector = 0;                // No special flags
ch->affectTo(&aff);
```

**Multiple effects:** Most drugs apply 2-4 different stat modifiers as separate affects. All share `type = AFFECT_DRUG`.

### Removal

Drug affects expire naturally after their duration. No special removal code needed - standard affect system handles it via `updateAffects()`.

## Critical Edge Cases and Gotchas

### 1. Descriptor Null Checks

```cpp
// WRONG: Crashes if descriptor null
ch->desc->drugs[drug].current_consumed++;

// CORRECT: Always check descriptor
if (ch->desc) {
    ch->desc->drugs[drug].current_consumed++;
}
```

**Where this matters:**
- Mobs don't have descriptors
- Players who just disconnected lose descriptor
- Linkdead players have null descriptor

### 2. Current Consumption Reset Bug

```cpp
// Player smokes 10 times
ch->desc->drugs[DRUG_OPIUM].current_consumed = 10;
saveDrugStats();  // Saves 10 to database

// Player disconnects (descriptor destroyed)
// Player reconnects (new descriptor created)
loadDrugStats();  // Loads 10 from database into NEW descriptor

// BUT: This is stale data!
// The descriptor was reset, so this should be 0
```

**Impact:** `current_consumed` appears to persist but is actually stale. Players get drug effects as if they consumed 10 units in current session when they actually consumed 0.

**Fix needed:** Either:
1. Reset `current_consumed` to 0 on disconnect (in descriptor destructor)
2. Move `current_consumed` to charfile (requires format change)
3. Use only `total_consumed` for effect scaling

### 3. Race-Specific Effects

Hobbits have special pipeweed handling:

```cpp
// Hobbits get bonuses
if (ch->getRace() == RACE_HOBBIT) {
    aff.modifier = 5;  // Positive
} else {
    aff.modifier = -5;  // Negative for all other races
}
```

**Forgot hobbit check:** Non-hobbit code path may be incorrect.

### 4. Time Calculation Edge Cases

```cpp
int timeDiff(time_info_data t1, time_info_data t2) {
  // Assumes t1 > t2
  // No validation - can return negative values
  // Assumes no year wraparound
}
```

**Bug potential:** If `first_use > current_time` (due to database corruption or server time change), calculations break.

### 5. Drug Effects Don't Stack by Type

Each `affectTo()` call creates a separate affect. Multiple smokes create multiple `AFFECT_DRUG` entries, all with 1-hour duration:

```cpp
// Smoke 1: AFFECT_DRUG, INT -10, expires in 1 hour
// Smoke 2: AFFECT_DRUG, INT -10, expires in 1 hour
// Total: INT -20 for 1 hour, then INT -10 for next hour
```

This is **intentional** - each dose adds separate stacking penalties that expire independently.

### 6. Database Saves Every Smoke

```cpp
void TBeing::doSmoke(const char* argument) {
  // ... consumption logic
  saveDrugStats();  // Database write EVERY time
}
```

**Performance:** High-frequency drug use = high database write load. No batching or delay.

## Data Loss on Disconnect

### The Core Problem

```cpp
class Descriptor {
    drugData drugs[MAX_DRUG];  // Ephemeral - NOT in charfile
};
```

When a player disconnects:
1. Descriptor is deleted
2. `drugs[]` array is destroyed
3. Last `saveDrugStats()` call preserved data to database
4. On reconnect, `loadDrugStats()` restores from database
5. **BUT:** `current_consumed` should reset to 0, not reload old value

### What Persists vs What's Lost

| Field | Persists? | Why |
|-------|-----------|-----|
| `first_use` | ✓ | Database + reload |
| `last_use` | ✓ | Database + reload |
| `total_consumed` | ✓ | Database + reload |
| `current_consumed` | **✗ (BUG)** | Reloads stale value instead of resetting to 0 |

### Affected Systems

**Broken:**
- Current session consumption scaling
- Immediate effect intensity based on recent use
- Short-term tolerance buildup

**Working:**
- Historical tracking (`total_consumed`)
- Addiction detection (based on `total_consumed` / time)
- Withdrawal timing (based on `last_use`)

## Common Patterns

### Safe Drug Effect Application

```cpp
// Always validate descriptor exists
if (!ch->desc || !ch->isPc())
    return;

// Get consumption with safe default
int consumed = ch->desc ? ch->desc->drugs[drug].current_consumed : 0;

// Apply effects
applyDrugAffects(ch, drug, false);

// Save to database
ch->saveDrugStats();
```

### Checking Addiction Status

```cpp
bool TBeing::isAddicted(drugTypeT drug) {
    if (!desc || !isPc())
        return false;

    // Get usage stats
    int total = desc->drugs[drug].total_consumed;
    if (total < 10)
        return false;  // Need minimum usage history

    // Calculate average rate
    time_info_data current = GameTime::getTime();
    time_info_data first = desc->drugs[drug].first_use;
    int hours = timeDiff(current, first);

    if (hours == 0)
        return false;

    float rate = (float)total / hours;

    // Addiction threshold varies by drug
    switch (drug) {
        case DRUG_OPIUM: return rate > 0.5;     // Very addictive
        case DRUG_PIPEWEED: return rate > 2.0;  // Mild
        case DRUG_POT: return rate > 1.0;       // Moderate
        case DRUG_FROGSLIME: return rate > 0.8; // Moderate-high
        default: return false;
    }
}
```

## Key Source Files

| File | Contents |
|------|----------|
| `code/code/obj/obj_drug.h` | drugTypeT enum, drugData class, TDrug class |
| `code/code/obj/obj_drug.cc` | doSmoke(), applyDrugAffects(), applyAddictionAffects(), saveDrugStats(), loadDrugStats() |
| `code/code/obj/obj_drug_container.h` | TDrugContainer class |
| `code/code/obj/obj_drug_container.cc` | lightDecay(), lightMe(), pipe mechanics |
| `_Setup-data/sql_tables/sneezy/drug_use.sql` | Database schema |
| `code/code/sys/connect.h` | Descriptor class with drugs[] array |

## Future Improvements Needed

1. **Fix current_consumed reset bug:** Reset to 0 on disconnect or move to charfile
2. **Add descriptor validation:** More null checks in drug effect code
3. **Batch database writes:** Save drug stats less frequently (e.g., every 5 minutes)
4. **Time calculation safety:** Handle negative time diffs and year wraparound
5. **Addiction recovery:** Add mechanism to reduce `total_consumed` over long abstinence periods
6. **Drug tolerance:** Implement diminishing returns for repeated use
7. **Overdose mechanics:** Extreme consumption should have severe consequences
