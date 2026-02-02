---
title: Faction System
category: understanding
created_by_model: opus
keywords: [factionTypeT, TFactionInfo, faction-percentage, reconcileHelp, percModifier, caravan, potency, tithe, FACTIONS_IN_USE]
related: [affects-system.md, economy-system.md]
primary_symbols:
  functions: [factionNumber, reconcileHelp, reconcileHurt, percModifier, boot_factions, doNewMember]
  classes: [TFactionInfo, TBeing, TPerson]
  files: [code/code/misc/faction.h, code/code/misc/faction.cc, code/code/misc/being.h]
---

## Overview

How do political and religious organizations shape player identity and divine power in a MUD? The faction system provides organizational structures that group players under shared causes, manage leadership hierarchies, track inter-faction relationships, and integrate with the divine magic system for clerics and deikhans.

Three active factions compete: the Brotherhood of Galek (based in Brightmoon), the Cult of Logrus (based in Logrus city), and the Order of the Serpents (based in Amber). Players may also remain unaffiliated, gaining access to neutral deities without faction-specific mechanics.

The system operates in two modes controlled by the `FACTIONS_IN_USE` compile-time flag. When enabled, it tracks a loyalty percentage from -100 to +100 that directly affects cleric and deikhan prayer power. When disabled (current state), membership management, leadership commands, caravans, and wealth tracking remain functional, but percentage tracking and dynamic prayer power are inactive. All players receive a fixed 75% prayer power modifier instead.

Factions generate revenue through trade caravans that travel between cities. Players can raid enemy caravans or protect allied ones. Each faction maintains a corporate bank account for wealth management, funded by tithes and successful caravan deliveries.

Five metrics determine faction standing: average member level (biased toward high levels), pounds of fish caught, average trophy completion (biased toward high levels), number of shops owned, and total faction wealth. These drive competition between factions for bragging rights.

## Patterns

### Faction Membership

Always verify faction membership before granting faction-specific privileges. Use `getFaction()` and compare against `factionTypeT` values, not raw integers.

Never allow players to hold membership in multiple factions simultaneously. The system enforces this at the code level, and immortals enforce the out-of-character rule that no player may have characters in separate factions.

Always clear faction percentages when a player joins a new faction. The `doNewMember()` function handles this automatically when `FACTIONS_IN_USE` is enabled, zeroing all individual faction percentages.

### Leadership Authority

Always validate leadership authority before executing faction commands. Primary leaders (slot 0) have full authority; subleaders (slots 1-3) have varying authority based on faction policy.

Never allow self-modification of leadership status. A leader cannot demote themselves or modify their own slot.

### Percentage Tracking (When Enabled)

Always call `reconcileHelp()` after beneficial actions toward another being. Always call `reconcileHurt()` after harmful actions. These functions handle the complex inter-faction relationship calculations.

Never modify faction percentages directly. Use `setPerc()` and `setPercX()` accessors, which maintain consistency between composite and individual percentages.

Never assume percentage values persist across sessions without database synchronization. The character file stores percentages, but runtime modifications must trigger saves.

### Prayer Power

Always use `percModifier()` to get the prayer power multiplier. Never hardcode percentage-to-modifier conversions, as the formula changes based on `FACTIONS_IN_USE`.

When disabled, `percModifier()` returns a constant 0.75. Do not attempt to optimize around this or cache the value differently based on faction state.

### Binary Format Constraints

Never modify `ABS_MAX_FACTION` (currently 6). This constant is frozen in the binary character file format. Changing it corrupts all existing player saves.

Never add or remove fields from the faction-related portions of `charFile`. The binary format does not support versioning.

### Caravan Operations

Always track both attempts and successes separately. The success rate matters for faction statistics and leader performance evaluation.

Never spawn caravans without decrementing the caravan counter. The interval system prevents excessive spawning.

### Corporate Wealth

Always save both faction data and corporate account after wealth transfers. A crash between transfer and save causes duplication or loss.

Never allow withdrawals exceeding the corporate balance. Validate before transfer, not after.

## Reference

### Symbol Quick Reference

| Symbol | Type | Purpose |
|--------|------|---------|
| `factionTypeT` | enum | Faction identifier (FACT_NONE through FACT_SNAKE) |
| `TFactionInfo` | class | Runtime faction data container |
| `TBeing` | class | Base class storing faction membership |
| `TPerson` | class | Player class with reconcile functions |
| `factionNumber()` | function | Parse faction name to enum |
| `reconcileHelp()` | function | Update percentage for beneficial action |
| `reconcileHurt()` | function | Update percentage for harmful action |
| `percModifier()` | function | Calculate prayer power multiplier |
| `boot_factions()` | function | Load faction data at startup |
| `doNewMember()` | function | Add player to faction |
| `FACTIONS_IN_USE` | constant | Compile-time feature toggle |
| `ABS_MAX_FACTION` | constant | Binary format limit (frozen at 6) |
| `MAX_FACTIONS` | constant | Active faction count (4) |
| `FACT_LEADER_SLOTS` | constant | Leadership positions per faction (4) |

### Faction Enumeration

| Value | Constant | Name | Base Location (vnum) |
|-------|----------|------|----------------------|
| -1 | `FACT_UNDEFINED` | Invalid/error state | N/A |
| 0 | `FACT_NONE` | Unaffiliated | Grimhaven (432) |
| 1 | `FACT_BROTHERHOOD` | Brotherhood of Galek | Brightmoon (1395) |
| 2 | `FACT_CULT` | Cult of Logrus | Logrus (3768) |
| 3 | `FACT_SNAKE` | Order of the Serpents | Amber (8713) |

### Faction Name Aliases

| Faction | Accepted Names |
|---------|----------------|
| Brotherhood | "brotherhood", "galek" |
| Cult | "cult", "chaos", "logrus" |
| Order | "order", "serpents", "snakes" |

### Leadership Commands

| Command | Authority | Purpose |
|---------|-----------|---------|
| `newmember` | Leader/subleader | Add new faction member |
| `rmember` | Leader/subleader | Remove faction member |
| `makeleader` | Primary leader only | Promote/demote leaders |
| `adjust` | Varies | Modify help/hurt values (blocked when disabled) |
| `send` | Leader/subleader | Send faction-wide message |

### Percentage Effects (When Enabled)

| Percentage | Prayer Modifier | Effective Power |
|------------|-----------------|-----------------|
| -100 | 0.00 | 0% |
| -50 | 0.25 | 25% |
| 0 | 0.50 | 50% |
| +50 | 0.75 | 75% |
| +100 | 1.00 | 100% |

### Feature Status by Flag

| Feature | FACTIONS_IN_USE=1 | FACTIONS_IN_USE=0 |
|---------|-------------------|-------------------|
| Faction membership | Working | Working |
| Leadership commands | Working | Working |
| Caravan spawning | Working | Working |
| Corporate wealth | Working | Working |
| Tithe deposits | Wealth + percentage | Wealth only |
| Percentage tracking | Active | Disabled |
| reconcileHelp/Hurt | Active | Empty functions |
| Dynamic prayer power | Active | Fixed at 0.75 |
| Help/hurt adjustment | Available | Blocked |

### Scoring Metrics

| Metric | Calculation |
|--------|-------------|
| Average Level | Biased toward high levels (one 50 > two 25s) |
| Pounds of Fish | Records held (1pt each) + total weight |
| Average Trophy | Level-biased trophy percentage |
| Shops Owned | Unique shops with faction owner |
| Faction Wealth | Corporate bank balance in talens |

### Key Files

| File | Contents |
|------|----------|
| `faction.h` | TFactionInfo class, constants, enums |
| `faction.cc` | Implementation, commands, persistence |
| `being.h` | Accessor methods, virtual declarations |
| `person.h` | TPerson reconcile functions |
| `lib/faction/faction_info` | Runtime faction data persistence |
| `factionmembers.sql` | Membership database schema |

## Implementation

### Data Structures

The `factionTypeT` enumeration defines faction identities from -1 (undefined) through 3 (Order of the Serpents), with `MAX_FACTIONS` set to 4 for iteration. A separate constant `ABS_MAX_FACTION` (6) defines the binary file format limit and must never change.

`TFactionInfo` holds all runtime data for a faction: display name, four leadership slots, membership password, a 4x2 relationship array for help/hurt values, potency pool, corporate ID, tithe tracking, and complete caravan state (interval, counter, flags, value, defense spending, attempts, successes).

`TBeing` stores per-character faction data: current faction membership, composite faction percentage, and an array of individual faction percentages (one per faction, up to `ABS_MAX_FACTION` entries for binary compatibility).

### Faction Identification

The `factionNumber()` function parses faction names using abbreviation matching. It accepts multiple aliases per faction (e.g., "brotherhood" or "galek" for `FACT_BROTHERHOOD`) and returns `FACT_UNDEFINED` for unrecognized input. This function drives all user-facing faction commands.

### Percentage Tracking Flow

When `FACTIONS_IN_USE` is enabled, `reconcileHelp()` and `reconcileHurt()` form the core tracking mechanism. Both functions:

1. Retrieve the actor's and victim's faction memberships
2. Look up the relationship values from the victim's faction array for the actor's faction
3. Apply the help or hurt value multiplied by an amplitude factor to the individual percentage
4. Recalculate the composite percentage by summing all individual faction percentages
5. Adjust the actor's faction power pool by a fraction of the change

The amplitude factor allows different actions to have different weights (healing vs. minor help, lethal damage vs. minor harm).

Integration points call these functions throughout the codebase: `doPray()` for tithes, `applyDamage()` for damage, healing spells for restoration, `perform_violence()` for combat, and `gainExpPerHit()` for group sharing.

### Prayer Power Calculation

`percModifier()` converts faction percentage to a prayer power multiplier. When enabled, it uses the formula `(percentage + 100) / 200`, mapping the -100 to +100 range onto 0.0 to 1.0. When disabled, it returns a constant 0.75, giving all divine casters 75% effectiveness regardless of faction standing.

### Inter-Faction Relationships

The faction array stores bidirectional relationship values. Each faction maintains a 4x2 array where the first index is the other faction and the second index selects help (0) or hurt (1) values. These values range from -4.0 to +4.0 and determine how much helping or hurting members of one faction affects standing with another.

The `adjust` command allows leaders to modify these values, but only when `FACTIONS_IN_USE` is enabled. The command is explicitly blocked otherwise to prevent confusion.

### Leadership Hierarchy

Four leadership slots exist per faction. Slot 0 holds the primary leader with full authority over all faction commands. Slots 1-3 hold subleaders with authority delegated by faction-specific policy.

Leadership changes are restricted: only the primary leader can execute `makeleader`, and modifications trigger immediate persistence to prevent loss on crash.

### Caravan Operations

Each faction tracks caravan state independently. The interval determines ticks between spawns; the counter counts down to the next spawn. When the counter reaches zero, the system spawns caravan mobs carrying faction goods and resets the counter to the interval.

Caravans travel predefined routes between faction bases. Players can intercept and raid enemy caravans (incrementing that faction's attempts without a corresponding success) or escort allied caravans to completion (incrementing both attempts and successes).

Defense spending affects caravan guard strength, making raids more difficult. The success rate (successes/attempts) serves as a leadership performance metric.

### Corporate Integration

Each faction links to a corporation via `corp_id`. This corporate account receives tithe deposits and caravan revenue. Leadership commands allow authorized withdrawals for faction operations.

Tithe calculation uses `TITHE_FACTOR` (0.0003) to convert talens to percentage increase when enabled. When disabled, tithes still deposit to the corporate account but do not affect faction standing.

### Membership Database

The `factionmembers` table tracks membership separately from character files. It stores name, faction identifier, and level for each member. This enables faction rollcall displays and membership verification without loading full character data.

The table is synchronized when players join, leave, or level up. Leadership commands query this table rather than iterating through online players.

### Persistence

Faction data persists to `lib/faction/faction_info` in binary format, with a backup at `lib/faction/faction_info.bak`. The `boot_factions()` function loads this file at startup, populating the global `faction_data[]` array.

Save triggers include: leadership changes, relationship modifications, caravan events, wealth transfers, and membership changes. Each save writes all faction data atomically to prevent partial state.

### Character File Storage

Character faction data occupies a fixed section of the binary `charFile` format: a short for faction type, a double for composite percentage, and an array of doubles for individual percentages. The array size is fixed at `ABS_MAX_FACTION` regardless of how many factions are actually active.

### Conditional Compilation

Over 40 `#if FACTIONS_IN_USE` blocks exist throughout the codebase. Major concentrations appear in `faction.cc` (15+ blocks covering reconcile functions, adjust command, and member management), `person.h` and `being.h` (virtual function declarations), `utility.cc` (tithe percentage handling), and various combat files (damage, offense, combat, other) for help/hurt integration.

When the flag is 0, these blocks compile to empty functions or early returns, ensuring no percentage tracking occurs but leaving the infrastructure in place for future reactivation.

### Deity Integration

Each faction associates with specific deities for divine spell flavor text. The `yourDeity()` method returns appropriate deity names based on faction membership. This affects only cosmetic elements like prayer messages; mechanical effects depend solely on faction percentage (when enabled) or the fixed modifier (when disabled).

### Holy Symbol Attunement

Holy symbols can be attuned to specific factions. Aligned usage provides full effectiveness; unaligned usage may be less effective. Attunement options depend on the symbol's properties and the wielder's faction membership.

## Troubleshooting

### Prayer Power Always 75%

**Symptom:** All clerics and deikhans report exactly 75% prayer effectiveness regardless of faction standing.

**Likely cause:** `FACTIONS_IN_USE` is set to 0 (the default/current state).

**Diagnostic approach:** Check `faction.h` for `#define FACTIONS_IN_USE 0`. Verify `percModifier()` is returning the constant 0.75.

**Fix:** This is intended behavior when faction percentage tracking is disabled. To enable dynamic prayer power, set `FACTIONS_IN_USE` to 1 and recompile.

### Faction Percentage Not Changing

**Symptom:** A player's faction percentage remains static despite faction-relevant actions.

**Likely cause:** `FACTIONS_IN_USE` is 0, so `reconcileHelp()` and `reconcileHurt()` are empty functions.

**Diagnostic approach:** Search for `#if FACTIONS_IN_USE` in the action's code path. Verify the percentage-affecting code is compiled out.

**Fix:** Same as above - enable `FACTIONS_IN_USE` if percentage tracking is desired.

### Adjust Command Fails

**Symptom:** Leaders receive "not permitted to alter the help/harm values" when using the adjust command.

**Likely cause:** `FACTIONS_IN_USE` is 0, explicitly blocking relationship modifications.

**Diagnostic approach:** Check `faction.cc` for the blocking check in the adjust command handler.

**Fix:** Enable `FACTIONS_IN_USE` if relationship modification is needed.

### Player Save Corruption After Faction Changes

**Symptom:** Player files fail to load or show garbled faction data after modifying faction constants.

**Likely cause:** `ABS_MAX_FACTION` or the `charFile` faction section layout was modified, breaking binary compatibility.

**Diagnostic approach:** Compare the current `charFile` structure against the original. Check if `ABS_MAX_FACTION` differs from 6.

**Fix:** Restore original constant values. If data is already corrupted, affected players may need character wipes for their faction data sections.

### Caravan Not Spawning

**Symptom:** Expected caravan spawns do not occur.

**Likely cause:** Caravan counter not decrementing, or interval set to 0.

**Diagnostic approach:** Check the faction's `caravan_interval` and `caravan_counter` values. Verify the periodic caravan check is executing.

**Fix:** Set a positive `caravan_interval` and reset `caravan_counter`. Ensure faction data persistence is functioning.

### Corporate Wealth Not Updating After Tithe

**Symptom:** Player tithes money but faction corporate balance does not increase.

**Likely cause:** Faction `corp_id` is invalid, corporate account does not exist, or save failed after transfer.

**Diagnostic approach:** Verify the faction's `corp_id` points to a valid corporation. Check for database errors during the tithe transaction.

**Fix:** Correct the `corp_id` or create the missing corporate account. Ensure database transactions complete successfully.
