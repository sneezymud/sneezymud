---
title: Drug Tracking System
category: understanding
created_by_model: opus
keywords: [drug, addiction, withdrawal, descriptor, ephemeral-state]
related: [affects-system.md, persistence-storage.md, scheduler-pulses.md]
primary_symbols:
  functions: [doSmoke, saveDrugStats, loadDrugStats, applyDrugAffects, applyAddictionAffects, lightDecay]
  classes: [drugData, TDrug, TDrugContainer, Descriptor]
  files: [code/code/obj/obj_drug.h, code/code/obj/obj_drug.cc, code/code/obj/obj_drug_container.h, code/code/obj/obj_drug_container.cc, code/code/sys/connect.h, _Setup-data/sql_tables/sneezy/drug_use.sql]
---

## Overview

What happens to a player's drug state when they disconnect? The answer reveals a subtle but important architectural decision: drug consumption data lives in two places with different persistence characteristics, and current session data is lost on disconnect.

The drug tracking system manages consumption, effects, and addiction through a hybrid storage model. The `Descriptor` class holds ephemeral per-session data in its `drugs[]` array, while the `drug_use` database table provides historical persistence. When a player smokes, the system updates both locations, applies stat-modifying affects, and calculates addiction/withdrawal based on consumption patterns over time.

The critical limitation is that `current_consumed` (how many doses this session) resets when the descriptor is destroyed at disconnect. This is intentional for the consumption counter but creates a bug: the value gets saved to the database and reloaded on reconnect, making stale data appear current.

Drug effects stack independently. Each dose creates a separate affect with its own duration. Smoking three times produces three `AFFECT_DRUG` entries that expire at different times, creating a gradual comedown rather than an abrupt end.

Withdrawal mechanics depend on time since last use compared to drug-specific thresholds. Opium triggers withdrawal after 6 game hours without use; pipeweed takes 24 hours. Severity scales with both addiction level (average consumption rate) and time overdue.

## Patterns

### Descriptor Null Checks

Always verify descriptor exists before accessing drug data. Mobs have no descriptor. Linkdead players have null descriptors. Players mid-disconnect have null descriptors.

```cpp
// WRONG: Crashes on null descriptor
ch->desc->drugs[drug].current_consumed++;

// CORRECT: Guard against null
if (ch->desc) {
    ch->desc->drugs[drug].current_consumed++;
}
```

### Safe Consumption Access

When reading consumption values for effect calculations, provide a safe default rather than crashing.

```cpp
auto current = ch->desc ? ch->desc->drugs[drug].current_consumed : 0;
```

### Combined PC and Descriptor Validation

Most drug operations only make sense for connected player characters. Check both conditions together.

```cpp
if (!ch->desc || !ch->isPc())
    return;
```

### Database Save Timing

Call `saveDrugStats()` after modifying drug data to ensure persistence. The current implementation saves on every smoke, which creates high database load during heavy use. No batching exists.

### Effect Stacking Awareness

Drug effects do not consolidate. Each dose adds a separate `AFFECT_DRUG` with independent duration. Plan for cumulative stat penalties that decay at different times.

### Addiction Rate Calculation

Addiction detection uses average consumption rate over total time since first use. Guard against division by zero when calculating hours since first use.

```cpp
int hours = timeDiff(current_time, first_use);
if (hours == 0)
    return false;
float rate = (float)total_consumed / hours;
```

### Time Difference Safety

The `timeDiff()` function assumes the first argument is greater than the second. Database corruption or server time changes can violate this assumption, producing negative or nonsensical values.

## Reference

### Symbol Quick Reference

| Symbol | Type | Purpose |
|--------|------|---------|
| `drugTypeT` | enum | Drug type identifier (DRUG_PIPEWEED through DRUG_FROGSLIME) |
| `drugData` | class | Per-drug usage statistics holder |
| `TDrug` | class | Drug item object (consumable substance) |
| `TDrugContainer` | class | Smoking device object (pipe, etc.) |
| `Descriptor::drugs[]` | array | Ephemeral per-session drug state |
| `doSmoke()` | function | Primary drug consumption entry point |
| `saveDrugStats()` | function | Persist drug data to database |
| `loadDrugStats()` | function | Load drug data from database on login |
| `applyDrugAffects()` | function | Apply stat-modifying effects for drug use |
| `applyAddictionAffects()` | function | Apply withdrawal penalties if overdue |
| `lightDecay()` | function | Reduce burn time on lit containers |
| `AFFECT_DRUG` | constant | Affect type identifier for drug effects |

### Drug Types

| Drug | Addiction Threshold | Withdrawal Onset | Primary Effects |
|------|---------------------|------------------|-----------------|
| Pipeweed | 2.0/hour | 24 hours | FOC +5 (hobbit), INT -5 (others) |
| Opium | 0.5/hour | 6 hours | DEX/INT penalties scaling with dose |
| Pot | 1.0/hour | 12 hours | PER -5 |
| Frogslime | 0.8/hour | 18 hours | CHA +10, WIS -15 |

### drugData Fields

| Field | Persists | Purpose |
|-------|----------|---------|
| `first_use` | Yes | Game timestamp of first consumption |
| `last_use` | Yes | Game timestamp of most recent consumption |
| `total_consumed` | Yes | Lifetime consumption count |
| `current_consumed` | Buggy | Session count (reloads stale on reconnect) |

### TDrug val Storage

| Value | Field | Description |
|-------|-------|-------------|
| val0 | curFuel | Current fuel units remaining |
| val1 | maxFuel | Maximum fuel capacity |
| val2 | drugType | Drug type enum cast to int |
| val3 | (unused) | Reserved |

### TDrugContainer val Storage

| Value | Field | Description |
|-------|-------|-------------|
| val0 | drugType | Drug type enum cast to int |
| val1 | maxBurn | Maximum burn time |
| val2 | curBurn | Current burn time remaining |
| val3 | lit | Boolean as int |

### Key Files

| File | Contents |
|------|----------|
| obj_drug.h | drugTypeT enum, drugData class, TDrug class |
| obj_drug.cc | doSmoke, applyDrugAffects, applyAddictionAffects, save/load functions |
| obj_drug_container.h | TDrugContainer class definition |
| obj_drug_container.cc | lightDecay, lightMe, pipe mechanics |
| drug_use.sql | Database schema for persistence |
| connect.h | Descriptor class with drugs[] array |

## Implementation

### Storage Architecture

Drug state splits between two storage locations with different lifecycles.

The `Descriptor` class contains a `drugData drugs[MAX_DRUG]` array. This is ephemeral storage destroyed when the player disconnects. Each `drugData` instance holds four fields: `first_use` timestamp, `last_use` timestamp, `total_consumed` lifetime count, and `current_consumed` session count.

The `drug_use` database table provides persistent storage across sessions. It mirrors the `drugData` fields with columns for timestamps (broken into year/month/day/hour/minute/second components) and consumption counts.

On login, `loadDrugStats()` queries the database and populates the descriptor's drug array. On each consumption, `saveDrugStats()` writes back to the database. The descriptor serves as the working copy; the database serves as the persistent backup.

### Consumption Flow

The `doSmoke()` function orchestrates the consumption process. It validates the drug container, checks that the pipe is lit, updates descriptor statistics, saves to database, and applies drug effects.

For first-time users of a specific drug, the function initializes the `first_use` timestamp. Every consumption increments both `total_consumed` and `current_consumed`, updates `last_use` to current game time, then calls `saveDrugStats()` to persist.

The final step calls `applyDrugAffects()` to apply stat modifications via the affects system.

### Effect Application

The `applyDrugAffects()` function creates `affectedData` structures with `type = AFFECT_DRUG` and drug-specific stat modifiers. Duration is one game hour (Pulse::UPDATES_PER_MUDHOUR).

Different drugs apply different combinations of stat changes. Pipeweed gives hobbits a FOC bonus but penalizes INT for other races. Opium applies DEX and INT penalties that scale with current consumption. Pot reduces PER. Frogslime boosts CHA while severely penalizing WIS.

Each call to `affectTo()` creates a separate affect entry. Multiple doses create multiple entries that expire independently, producing stacking penalties that gradually wear off.

### Withdrawal Mechanics

The `applyAddictionAffects()` function calculates whether a player is experiencing withdrawal based on time since last use and consumption history.

It first computes hours since last use by comparing current game time against the `last_use` timestamp. It then calculates average consumption rate by dividing `total_consumed` by hours since `first_use`.

Each drug has a withdrawal threshold in hours. If time since last use exceeds the threshold and average consumption rate exceeds 0.5 per hour, withdrawal kicks in. Severity scales with both addiction level (consumption rate) and how far past the threshold the player is.

Withdrawal applies STR and CON penalties via the affects system. The penalties persist for two game hours.

### Withdrawal Trigger Points

Three contexts trigger withdrawal checks:

The periodic character update pulse (`procCharTickUpdate`) checks drug state regularly during normal gameplay.

Login processing calls withdrawal checks after `loadDrugStats()` completes, immediately penalizing addicted players who were offline too long.

Combat can trigger mid-fight withdrawal if enough game time has passed since the last check.

### Pipe Burning Mechanics

The `TDrugContainer` class tracks burn state through `curBurn`, `maxBurn`, and `lit` fields. Lighting a pipe sets `lit` to true and `curBurn` to its starting value.

The scheduler calls `lightDecay()` each pulse on all lit drug containers. This decrements `curBurn` by one. When `curBurn` reaches zero, the pipe goes out (`lit` becomes false) and the room receives a notification.

### Database Persistence

The `saveDrugStats()` function iterates all drug types and updates the database. For each drug, it deletes any existing record then inserts the current values if `total_consumed` is greater than zero.

This delete-then-insert approach avoids the complexity of upsert logic but means every save performs two queries per drug type. Combined with the per-smoke save pattern, heavy drug use generates significant database traffic.

The `loadDrugStats()` function queries all records for the player and populates the descriptor array. It validates drug type indices before populating to avoid array overflows from corrupted data.

### The current_consumed Bug

The `current_consumed` field has problematic persistence behavior. When a player smokes 10 times and disconnects, `saveDrugStats()` writes 10 to the database. On reconnect, `loadDrugStats()` reads 10 back into the fresh descriptor.

This creates false state: the player consumed zero drugs this session, but the system thinks they consumed 10. Effect scaling based on `current_consumed` produces incorrect intensity.

The field should either reset to zero on load (matching the ephemeral intent) or move to persistent character storage (making it truly persistent). Currently it occupies an inconsistent middle ground.

### Race-Specific Handling

Pipeweed checks player race before applying effects. Hobbits receive positive stat modifiers while all other races receive penalties. This special case exists only for pipeweed; other drugs treat all races identically.

## Troubleshooting

### Drug Effects Not Appearing

**Symptom:** Player smokes but receives no stat changes.

**Likely cause:** Null descriptor at effect application time.

**Diagnostic approach:** Check if `ch->desc` is null when `applyDrugAffects()` is called. Verify the player is a PC, not a mob. Check if the player is mid-disconnect.

**Fix:** Ensure descriptor exists before applying effects. The `current_consumed` calculation already has a null guard that returns 0, but this masks the underlying issue rather than addressing it.

### Stale Session Data After Reconnect

**Symptom:** Player reconnects and appears to have session consumption from before disconnect.

**Likely cause:** The `current_consumed` bug. The value persists to database and reloads despite being intended as session-only data.

**Diagnostic approach:** Check `current_consumed` value immediately after `loadDrugStats()`. Compare to what player actually consumed this session (should be zero).

**Fix:** Either reset `current_consumed` to zero in `loadDrugStats()` after loading, or redesign to not persist this field at all.

### Unexpected Withdrawal Severity

**Symptom:** Player experiences extreme withdrawal penalties despite moderate use.

**Likely cause:** Time calculation producing incorrect hours. May be negative time difference from database corruption or server time change.

**Diagnostic approach:** Examine `first_use` and `last_use` timestamps. Verify they're earlier than current game time. Calculate hours manually and compare to system calculation.

**Fix:** Add validation that time differences are non-negative. Handle edge cases where stored timestamps exceed current time.

### High Database Load During Events

**Symptom:** Database performance degrades during drug-heavy roleplay events.

**Likely cause:** Every smoke triggers a database save. No batching or rate limiting exists.

**Diagnostic approach:** Monitor database query frequency during drug use. Count saves per minute.

**Fix:** Implement save batching (e.g., save every N minutes rather than every smoke) or aggregate saves at disconnect. Requires careful handling of crash scenarios to avoid data loss.

### Effect Stacking Confusion

**Symptom:** Player reports drug effects lasting longer or being more intense than expected.

**Likely cause:** Multiple doses creating multiple independent affects. Each has its own expiration timer.

**Diagnostic approach:** Examine player's affect list. Count `AFFECT_DRUG` entries. Check their individual expirations.

**Fix:** This is working as intended. Multiple affects stack and expire independently. If undesirable, would require consolidation logic in `applyDrugAffects()` to extend existing affects rather than adding new ones.

### Pipe Won't Light

**Symptom:** Player cannot light drug container.

**Likely cause:** Container already lit, no drug loaded, or missing ignition source.

**Diagnostic approach:** Check `isLit()` return value. Verify `getDrugType()` is not DRUG_NONE. Check `getCurBurn()` has remaining value.

**Fix:** Ensure pipe has drug loaded before lighting. Reset burn values if pipe ran out previously.
