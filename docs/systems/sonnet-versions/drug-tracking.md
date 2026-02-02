---
title: Drug Tracking System
category: understanding
keywords: [drug-consumption, addiction, withdrawal, descriptor-storage, ephemeral-state, database-persistence, AFFECT_DRUG, drugData, applyDrugAffects, saveDrugStats]
related: [affects-system.md, scheduler-pulses.md, persistence-storage.md]
primary_symbols:
  functions: [doSmoke, applyDrugAffects, applyAddictionAffects, saveDrugStats, loadDrugStats]
  classes: [drugData, TDrug, TDrugContainer, Descriptor]
  files: [code/code/obj/obj_drug.cc, code/code/obj/obj_drug_container.cc, code/code/sys/connect.h]
---

## Overview

How does the game track whether a player is addicted to opium after smoking five times yesterday, then disconnecting for 12 hours?

The drug tracking system manages player drug consumption, effects, and addiction mechanics through a hybrid storage model. Active consumption state lives in the descriptor (ephemeral), while historical usage data persists to the database. This split creates a critical limitation: current session consumption data does not survive disconnects.

The system serves three purposes:

**Immediate effects**: When you smoke drugs, stat modifiers are applied via the affects system (AFFECT_DRUG) with intensity scaling based on consumption count. Hobbits get bonuses from pipeweed; other races get penalties. Opium applies heavy intoxication scaling with usage.

**Addiction tracking**: The system calculates addiction risk by analyzing consumption patterns over time. Average consumption rate (total_consumed / hours_since_first_use) determines whether you're addicted. Each drug has different thresholds - opium at 0.5 uses/hour, pipeweed at 2.0 uses/hour.

**Withdrawal mechanics**: When enough time elapses since last use and you meet addiction criteria, withdrawal effects kick in. Severity scales with both addiction level and time overdue. Opium withdrawal starts after 6 hours; pipeweed after 24 hours.

The descriptor stores four fields per drug type: first_use timestamp, last_use timestamp, total_consumed count, and current_consumed count. On every smoke action, these fields update in memory, then immediately save to the database. On login, historical data loads from the database into the new descriptor.

The critical flaw: current_consumed should reset to zero when the descriptor is destroyed on disconnect. Instead, it reloads the stale value from the last session. This creates phantom tolerance where players receive amplified effects for drugs they haven't consumed in the current session.

## Patterns

### Descriptor Null Checks

ALWAYS validate descriptor exists before accessing drug data. Mobs, linkdead players, and recently disconnected players have null descriptors.

```cpp
// CORRECT
if (ch->desc) {
    ch->desc->drugs[DRUG_OPIUM].current_consumed++;
}

// WRONG - crashes on null descriptor
ch->desc->drugs[DRUG_OPIUM].current_consumed++;
```

**Why**: The drugData array lives in Descriptor, not TBeing. Dereferencing a null descriptor crashes immediately.

**Where it matters**: Combat code, scheduled pulse updates, any code path that might execute for mobs or linkdead players.

### Safe Consumption Scaling

ALWAYS provide a default value when using current_consumed for effect scaling.

```cpp
// CORRECT
int consumed = ch->desc ? ch->desc->drugs[drug].current_consumed : 0;
int penalty = -(10 + consumed / 2);

// WRONG - null dereference if no descriptor
int penalty = -(10 + ch->desc->drugs[drug].current_consumed / 2);
```

**Rationale**: Effect application code runs in contexts where the descriptor might be null. Defaulting to zero eliminates consumption-scaled effects for affected entities but prevents crashes.

### Database Persistence Discipline

ALWAYS call saveDrugStats() immediately after modifying consumption counts.

```cpp
// CORRECT
ch->desc->drugs[drug].total_consumed++;
ch->desc->drugs[drug].current_consumed++;
ch->saveDrugStats();  // Persist immediately

// WRONG - state lost on crash
ch->desc->drugs[drug].total_consumed++;
// Missing save call
```

**Why**: Drug state in the descriptor is volatile. Without immediate database writes, consumption data is lost on crash, disconnect, or server restart.

**Performance impact**: Every smoke action triggers a database write. High-frequency use generates high write load with no batching.

### Time Calculation Validation

NEVER assume time differences are positive or that first_use precedes current_time.

```cpp
// CORRECT
int hours = timeDiff(current_time, first_use);
if (hours <= 0) {
    return;  // Invalid time data, bail out
}

// WRONG - crashes or produces nonsense on negative values
int hours = timeDiff(current_time, first_use);
float rate = total_consumed / hours;  // Division by zero if hours == 0
```

**Why**: Database corruption, server time changes, or manual data edits can create reversed timestamps. The timeDiff function has no built-in validation.

### PC-Only Operations

ALWAYS check isPc() before drug operations. Mobs don't have descriptors or database persistence.

```cpp
// CORRECT
if (!ch->desc || !ch->isPc())
    return;

// WRONG - will fail on mobs
ch->desc->drugs[drug].total_consumed++;
```

### Addiction State Validation

NEVER check addiction without minimum usage history and valid time data.

```cpp
// CORRECT
if (total_consumed < 10)
    return false;  // Insufficient data
int hours = timeDiff(current_time, first_use);
if (hours <= 0)
    return false;  // Invalid time data
float rate = (float)total_consumed / hours;

// WRONG - division by zero, false positives
float rate = (float)total_consumed / timeDiff(current_time, first_use);
return rate > threshold;
```

**Why**: Single-use with zero elapsed time produces infinite rate. Need minimum sample size to determine actual patterns.

## Reference

### Primary Symbols

| Symbol | Type | Purpose |
|--------|------|---------|
| `doSmoke()` | function | Primary consumption entry point, validates container, updates stats, applies effects |
| `applyDrugAffects()` | function | Applies immediate drug effects via affects system based on drug type |
| `applyAddictionAffects()` | function | Calculates and applies withdrawal effects based on time and consumption patterns |
| `saveDrugStats()` | function | Persists all drug consumption data to database |
| `loadDrugStats()` | function | Loads historical drug data from database into descriptor on login |
| `drugData` | class | Storage structure for per-drug consumption tracking |
| `TDrug` | class | Drug item object (pipeweed, opium, etc.) |
| `TDrugContainer` | class | Smoking device object (pipe, hookah) |
| `Descriptor` | class | Connection state container holding ephemeral drugs array |

### Drug Types

| Enum Value | Name | Addiction Risk | Withdrawal Threshold |
|------------|------|----------------|---------------------|
| `DRUG_PIPEWEED` | Pipeweed | Low (2.0 uses/hour) | 24 hours |
| `DRUG_OPIUM` | Opium | High (0.5 uses/hour) | 6 hours |
| `DRUG_POT` | Pot | Medium (1.0 uses/hour) | 12 hours |
| `DRUG_FROGSLIME` | Frogslime | Medium-high (0.8 uses/hour) | 18 hours |

### drugData Fields

| Field | Type | Persistence | Purpose |
|-------|------|-------------|---------|
| `first_use` | time_info_data | Database | Timestamp of first consumption, never changes |
| `last_use` | time_info_data | Database | Timestamp of most recent consumption |
| `total_consumed` | unsigned int | Database | Lifetime consumption count across all sessions |
| `current_consumed` | unsigned int | Database (buggy) | Current session count, should reset on disconnect |

### Key Return Values

| Function | Return Value | Meaning |
|----------|--------------|---------|
| `doSmoke()` | TRUE | Consumption succeeded |
| `doSmoke()` | FALSE | Validation failed (not lit, wrong item type, etc.) |
| `isAddicted()` | true | Player meets addiction criteria for drug type |
| `isAddicted()` | false | Not addicted or insufficient usage history |

### Database Schema Fields

| Column | Maps To | Notes |
|--------|---------|-------|
| `drug_id` | drugTypeT enum | Integer cast of enum value |
| `player_id` | Character ID | Foreign key to player record |
| `first_use_*` | drugData.first_use | Six fields for time_info_data components |
| `last_use_*` | drugData.last_use | Six fields for time_info_data components |
| `total_consumed` | drugData.total_consumed | Persists correctly across sessions |
| `current_consumed` | drugData.current_consumed | Saved but improperly reloaded as non-zero |

### Object Value Storage

**TDrug (ITEM_DRUG):**

| Val | Field | Purpose |
|-----|-------|---------|
| val0 | curFuel | Remaining consumable units |
| val1 | maxFuel | Maximum capacity |
| val2 | drugType | drugTypeT enum as integer |
| val3 | - | Unused |

**TDrugContainer (ITEM_DRUG_CONTAINER):**

| Val | Field | Purpose |
|-----|-------|---------|
| val0 | drugType | Type of drug currently loaded |
| val1 | maxBurn | Maximum burn duration in pulses |
| val2 | curBurn | Current burn time remaining |
| val3 | lit | Boolean flag (0/1) indicating lit state |

## Implementation

### Storage Architecture

The system splits drug data across three storage layers, each with different lifetimes and access patterns.

**Descriptor-resident ephemeral state** holds the drugData array. This lives at Descriptor::drugs[MAX_DRUG] and contains five slots (DRUG_NONE through DRUG_FROGSLIME). Each element tracks first_use, last_use, total_consumed, and current_consumed for that drug type. The descriptor is destroyed on disconnect, taking this data with it unless saved first.

**Database persistence layer** stores the same information in the drug_use table. The schema maps drugData fields to individual columns: six columns for first_use timestamp components, six for last_use, plus total_consumed and current_consumed integers. Each row represents one drug type for one player, keyed by player_id and drug_id. Rows are deleted and reinserted on every save - no UPDATE queries.

**Affects system integration** applies temporary stat modifiers through the standard affects mechanism. Drug effects create affectedData structures with type AFFECT_DRUG, duration set to Pulse::UPDATES_PER_MUDHOUR (one game hour), and stat modifiers in the location/modifier fields. Multiple smokes create multiple stacked affects that expire independently.

### Consumption Flow

When doSmoke executes, it first validates the drug container object via dynamic_cast to TDrugContainer. The container must be lit (isLit() returns true) or the command fails with "You need to light it first." The drug type is extracted from the container via getDrugType().

If the descriptor exists and this is the player's first use of this drug type (total_consumed equals zero), the first_use timestamp initializes to current game time. All six time_info_data components (seconds, minutes, hours, day, month, year) populate from GameTime accessors.

Both consumption counters increment: total_consumed for lifetime tracking, current_consumed for session-based scaling. The last_use timestamp updates to current game time regardless of whether this is first use.

saveDrugStats immediately persists the updated state. The function iterates all drug types, deletes existing database rows for this player_id and drug_id combination, then inserts fresh rows with current descriptor values. This delete-then-insert pattern avoids UPDATE complexity but generates more database churn.

applyDrugAffects applies immediate effects based on drug type. The function creates multiple affectedData structures, each targeting a different stat. For opium, dexterity penalties scale with current_consumed via the formula -(5 + current_consumed / 2). Intelligence penalties use -(10 + current_consumed). Each affect gets added to the character's affect list via affectTo(), where it will expire naturally after one game hour.

### Addiction Calculation

applyAddictionAffects implements withdrawal mechanics by comparing time elapsed since last use against drug-specific thresholds. The function first calculates hours_since by calling timeDiff with current game time and the last_use timestamp from the descriptor.

Average consumption rate derives from total_consumed divided by the time span between first_use and now. This produces a uses-per-hour metric that indicates usage intensity. If the average rate exceeds the drug-specific threshold and enough time has elapsed without new consumption, withdrawal triggers.

Withdrawal threshold values vary by drug: opium triggers after 6 hours (highly addictive), pot after 12, frogslime after 18, pipeweed after 24 (mildly addictive). The threshold represents how long you can go without using before withdrawal kicks in.

Severity scales multiplicatively: average consumption rate times hours overdue beyond threshold. A player smoking opium twice per hour who goes 10 hours without use faces severity of 2.0 * (10 - 6) = 8.0. This severity value divides down to produce stat penalties: strength loses severity/2, constitution loses severity/3.

Withdrawal affects use the same AFFECT_DRUG type as consumption effects but with doubled duration (two game hours instead of one). This creates overlapping negative effects when withdrawal and recent consumption both apply.

### Race-Specific Effects

Pipeweed has special handling for hobbits. When applyDrugAffects processes DRUG_PIPEWEED, it checks getRace() against RACE_HOBBIT. Hobbits receive a +5 focus bonus. All other races take -5 intelligence penalty. This implements the lore that hobbits are naturally suited to pipeweed while others find it debilitating.

The race check happens every time effects apply, not just on first use. Polymorphed characters get effects for their current form, not their original race.

### Object Burning Mechanics

TDrugContainer objects track burning state through the lit boolean and curBurn counter. When a pipe is lit via lightMe(), lit sets to true and curBurn initializes to maxBurn value from the object definition.

Each pulse, the scheduler calls lightDecay() on all lit drug containers in the game. This function decrements curBurn by one. When curBurn reaches zero, lit resets to false and room occupants see "$p goes out" via the act() system.

Smoking from an unlit container fails the isLit() check in doSmoke and produces the error message. Players must explicitly light pipes before use, and pipes burn out after maxBurn pulses regardless of whether anyone smokes from them. This creates a time pressure: light the pipe, smoke quickly, or waste the loaded drug.

### Database Operations

saveDrugStats executes separate queries for each drug type. The pattern is delete-then-insert rather than update-or-insert. For each drug from DRUG_PIPEWEED to MAX_DRUG, the function first deletes any existing row matching player_id and drug_id, then inserts a new row if total_consumed exceeds zero.

The insert query passes 16 parameters: drug_id, player_id, six first_use timestamp components, six last_use timestamp components, total_consumed, and current_consumed. All parameters are integers. Time components map directly from time_info_data structure fields.

loadDrugStats queries all rows for the player_id and iterates results. Each row maps back to a drug type via the drug_id field. After validating the drug_id falls within the valid enum range, the function populates descriptor drug array slots with values from the result columns.

The query has no ORDER BY clause, so row processing order is undefined. Since each drug_id appears at most once per player, order doesn't matter functionally, but deterministic ordering would improve debugging.

### Current Consumption Bug

The bug manifests in the interaction between descriptor lifecycle and database persistence. When a player disconnects, the descriptor destructor runs, destroying the drugs array. No code explicitly saves drug state during disconnect - the last saveDrugStats call was during the most recent consumption action.

On reconnect, a new descriptor allocates with zeroed drugs array. loadDrugStats then populates it from database rows, including the current_consumed field. This value reflects the consumption count from the previous session, not the actual current session consumption (which should be zero).

Subsequent consumption increments this already-nonzero value, creating compounding inflation. A player who smoked 10 times yesterday, disconnected, and reconnected sees current_consumed start at 10. First smoke today increments to 11, amplifying effects beyond actual current session use.

The affects system uses current_consumed for scaling formulas like -(10 + current_consumed). This phantom inflation produces stronger effects than intended, effectively giving players credit for drugs they didn't consume in the current session.

### Integration Points

The scheduler integrates drug mechanics through procCharTickUpdate, which calls applyAddictionAffects at regular intervals. This creates periodic withdrawal checks even while players are idle. If enough time has passed since last use, withdrawal effects apply automatically.

Combat integration is passive - drug effects modify stats through the affects system, and combat calculations use those modified stats. No special combat code references drugs directly.

Login integration happens in descriptor initialization after authentication completes. The connection code calls loadDrugStats to populate the new descriptor's drug array from historical database records.

The affects system treats AFFECT_DRUG like any other affect type. Expiration, removal, saving, and display all use standard affect mechanisms. Drug-specific code only appears in application (applyDrugAffects) and withdrawal (applyAddictionAffects), not in affect lifecycle management.

### File Organization

obj_drug.h defines drugTypeT enum, drugData class structure, and TDrug class with fuel tracking. The header declares function prototypes for applyDrugAffects and applyAddictionAffects as standalone functions, not class members.

obj_drug.cc implements consumption mechanics (doSmoke), persistence (saveDrugStats, loadDrugStats), and effects (applyDrugAffects, applyAddictionAffects). The file also contains utility functions for time difference calculation and addiction status checking.

obj_drug_container.h defines TDrugContainer with burning mechanics. The class tracks drugType, maxBurn, curBurn, and lit state.

obj_drug_container.cc implements lightDecay for burn processing and lightMe for ignition. The file handles the pulse-driven countdown that extinguishes pipes.

connect.h declares the Descriptor class, which contains the drugs[MAX_DRUG] array. This places drug state in the connection layer rather than the character layer, creating the ephemeral storage characteristic.

## Troubleshooting

### Symptom: Crash when mob uses drug item

**Likely cause**: Code dereferencing desc without null check.

**Diagnostic approach**: Stack trace shows crash in doSmoke or applyDrugAffects. The this pointer is valid but desc is null. Check if the character is a mob via isPc() - mobs never have descriptors.

**Fix**: Add descriptor null check before accessing drugs array. For mobs, either skip drug effects entirely or use total_consumed defaulting to zero.

### Symptom: Drug effects persist after disconnect/reconnect

**Likely cause**: Affects with remaining duration were saved to charfile and reloaded.

**Diagnostic approach**: Check character's affect list with diagnostic commands. Look for AFFECT_DRUG entries with non-zero duration. These came from the affects persistence system, not drug tracking.

**Fix**: This is correct behavior - affects are supposed to persist. The issue is if effects seem stronger than expected, which indicates the current_consumed bug inflating effect intensity.

### Symptom: Withdrawal effects trigger immediately on login

**Likely cause**: Player's last_use timestamp is stale (many hours old) and addiction criteria are met.

**Diagnostic approach**: Check last_use value in database. Calculate hours since that timestamp. Compare to withdrawal threshold for the drug type. If last_use is days old and total_consumed is high, withdrawal is working as designed.

**Fix**: Not a bug - player genuinely meets withdrawal criteria. To avoid, they need to consume drugs more recently or reduce historical consumption to fall below addiction threshold.

### Symptom: current_consumed shows inflated values

**Likely cause**: The descriptor reload bug reloading stale session data.

**Diagnostic approach**: Check database drug_use.current_consumed value. Disconnect the player and reconnect. Check descriptor drugs array after login - it should show zero for current_consumed but actually shows the database value.

**Fix**: Requires code change. Either reset current_consumed to zero in loadDrugStats, zero it in descriptor destructor with a saveDrugStats call, or eliminate current_consumed from the schema and use only total_consumed for effect scaling.

### Symptom: Database shows current_consumed but player gets weak effects

**Likely cause**: Descriptor is null in effect application code, causing default-to-zero fallback.

**Diagnostic approach**: Add logging in applyDrugAffects to trace the consumed variable value. Check if the descriptor null check is firing and defaulting to zero.

**Fix**: This is defensive code working as intended. The real question is why the descriptor is null when it shouldn't be. Check if the character is linkdead, recently disconnected, or a mob being polymorphed into.

### Symptom: High database write load during drug use

**Likely cause**: saveDrugStats called on every smoke action with no batching.

**Diagnostic approach**: Profile database queries during active drug use. Count delete and insert queries matching the drug_use table. Each smoke generates 5 deletes (one per drug type) plus N inserts (one per drug type with nonzero total_consumed).

**Fix**: Requires architectural change. Implement batched saves - mark descriptor as dirty on consumption, then save during periodic pulses or on disconnect only. Accept risk of data loss on crashes between consumption and next save.

### Symptom: Division by zero crash in addiction calculation

**Likely cause**: timeDiff returning zero when first_use equals current_time, or negative when timestamps are reversed.

**Diagnostic approach**: Add logging before division. Print total_hours value. Check database first_use and last_use timestamps for the player and drug. Look for same timestamp (immediate second-smoke) or reversed timestamps (data corruption).

**Fix**: Add validation before division. Check hours > 0 and bail out early if not. For reversed timestamps, either reset first_use to current time or delete the corrupted database row.

### Symptom: Hobbit gets pipeweed penalties instead of bonuses

**Likely cause**: Race check failing due to polymorph or race field corruption.

**Diagnostic approach**: Check getRace() return value during effect application. Log the race value and the path taken in the if/else branch. Verify the character is genuinely a hobbit and not polymorphed.

**Fix**: If polymorphed, this is correct - current form determines effects. If not polymorphed but race check fails, investigate race field integrity. May need to reset race to original value.
