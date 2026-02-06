---
title: Experience and Leveling System
description: Player progression through XP accumulation, level advancement, practice point allocation, and multiclass penalties
keywords: [experience curve, level progression, multiclass penalty, trophy system, practice points, death penalty, soft-cap]
category: important
primary_symbols:
  functions: [gain_exp, advanceLevel, getExpClassLevel, mob_exp, pracsPerLevel, deathExp, gainExpPerHit, getExpModVal, doHPGainForLev]
  classes: [TBeing]
  enums: [DOUBLEEXP, TOG_NO_XP_GAIN, FAE_TOUCHED, FREE_DEATHS, MAX_SAVED_CLASSES, MIN_CLASS_IND, MAX_CLASSES]
---

# Experience and Leveling System

## Overview

SneezyMUD's progression system rewards players for defeating monsters with experience points (XP) that accumulate toward level thresholds. Each level requires exponentially more XP than the last, creating a natural pacing curve where early levels pass quickly and later levels demand sustained effort.

Characters can advance in multiple classes simultaneously, but doing so incurs steep penalties: XP is divided by the square of the number of classes held. A dual-class character earns one-quarter XP toward each class; a triple-class character earns one-ninth.

The trophy system tracks how often a player kills each mob type. The first eight kills of any creature yield full XP, but subsequent kills reduce rewards down to 30%, encouraging players to seek variety rather than grinding a single target. Trophy counts decay over time, restoring full XP rates for creatures left alone.

Practice points, used to learn skills and spells, accumulate within each level based on intelligence and class. Higher levels (30+) award practices at half the rate of earlier levels, creating another incentive to specialize.

Death carries an XP penalty capped at either 20% of current XP or a level-scaled amount, whichever is lower. This protects low-level players from devastating losses while maintaining meaningful risk at higher levels.

## Patterns

### XP Gain Operations

- Always call `gain_exp()` for XP awards; never modify the `exp` member directly.
- Always pass negative values to `gain_exp()` for death penalties; negative XP bypasses modifiers and soft caps.
- Never award XP in arena rooms; `gain_exp()` returns early when `ROOM_ARENA` is set.
- Never award XP for PvP kills; the `isPking()` check blocks attacker XP.

### Level Advancement

- Always let `advanceLevel()` handle level-up processing; it awards HP and unlocks class features. The `max_exp` threshold is updated separately in `gain_exp()`.
- Always check `getExp() >= getMaxExp()` to determine level eligibility; direct level manipulation breaks XP tracking.
- Never assume practice points match levels; practices are awarded at XP intervals within each level, not at level boundaries.

### Multiclass Handling

- Always divide XP by `howManyClasses()` twice; this is intentional quadratic scaling, not redundant code.
- Always divide practice point awards by class count separately from XP division.
- Never assume XP gains benefit all classes equally; each class processes its own gain loop independently.

### Trophy System

- Always apply trophy modifiers via `FRACT()` after group share calculation.
- Always allow eight free kills before applying trophy penalties; this protects new players exploring the world.
- Never reset trophy counts manually; they decay automatically through `procCharTickUpdate`.

### Death Handling

- Always use `deathExp()` to calculate death penalties; it applies the min-of-two-values cap.
- Always check for `FREE_DEATHS` affect before applying death XP loss.
- Always apply PvP death reduction (divide by 10) when `isPking()` returns true.

### Group Experience

- Always count only group members within combat range for XP shares.
- Always apply trophy modifiers per-character after splitting group XP.
- Never assume all group members receive equal XP; trophy counts vary per player.

## Reference

### XP Formula Components

| Formula | Calculation | Notes |
|---------|-------------|-------|
| Kills to level | `17 + (1.25 * level)` | Linear increase per level |
| Mob XP value | Compound 2% growth per level | Level 50+ divided by 3 |
| Level threshold | Sum of (kills_to_level * mob_exp) for all prior levels | Exponential curve |
| Soft cap threshold | `1.15 * current_level` | Gains below this receive no reduction |
| Soft cap formula | `(1.0 - pow(1.1, -gain/newgain)) + 1.0` | Returns 1.0 to 2.0 |

### XP Modifiers

| Modifier | Effect | Application Point |
|----------|--------|-------------------|
| DOUBLEEXP toggle | Multiply by 2 | Before multiclass division |
| Multiclass penalty | Divide by classes squared | After DOUBLEEXP |
| Soft cap | Logarithmic reduction | After multiclass division |
| FAE_TOUCHED | Divide by 2 | After soft cap |
| TOG_NO_XP_GAIN | Block completely | Checked first |

### Trophy Modifier Ranges

| Kill Count | XP Modifier | Display Text |
|------------|-------------|--------------|
| 1-8 | 100% | "full" |
| 9 | ~96% | "much" |
| 14 | ~79% | "fair amount" |
| 22 | 50% | "some" |
| 30+ | 30% (clamped) | "little" |

### Trophy Constants

| Constant | Value | Purpose |
|----------|-------|---------|
| free_kills | 8 | Kills before penalties begin |
| num_steps | 14.0 | Steps from 100% to 30% |
| step_mod | 0.5 | Reduction per step |
| min_mod | 0.3 | Floor modifier |
| max_mod | 1.0 | Full XP modifier |
| decay_rate | 0.25 | Per-pulse count reduction |

### Death Penalty Calculation

| Scenario | Formula | Cap |
|----------|---------|-----|
| Standard death | `min(exp/5, 25*mob_exp(level))` | Whichever is lower |
| PvP death | Standard / 10 | 1/10th penalty |
| FREE_DEATHS | 0 | No penalty |

### Practice Point Rates

| Condition | Effect |
|-----------|--------|
| Levels 1-29 | Full base rate |
| Levels 30+ | Half base rate |
| Per additional class | Divide by class count |
| Higher intelligence | Increased per-level gain |

### Level Boundaries

| Boundary | Significance |
|----------|--------------|
| Level 30 | Practice rate halves |
| Level 50 | Mortal cap; Discord notification sent; multiclass unlocks (single-class gets ALLOW_DOUBLECLASS, dual-class gets ALLOW_TRIPLECLASS) |
| Level 50+ | Immortal territory; mob XP divided by 3 |

## Implementation

### Experience Storage

Characters store XP as double-precision floating-point values in `being.h`. The `exp` member holds accumulated points; `max_exp` holds the threshold for the next level. Class levels are stored in a fixed-size array (`MAX_SAVED_CLASSES` entries, `ubyte` values) indexed by class constant, allowing simultaneous progression in up to 12 classes. These values persist across sessions through database serialization.

### Level Threshold Calculation

The `getExpClassLevel()` function in `gaining.cc` computes XP requirements recursively. Each level's requirement equals the previous level's threshold plus the product of `kills_to_level()` and `mob_exp()` for that level. The `kills_to_level()` function returns a linearly increasing count (17 + 1.25 per level), while `mob_exp()` in `cmd_low.cc` uses compound 2% growth per level, creating exponential scaling.

### XP Award Processing

The `gain_exp()` function in `limits.cc` orchestrates XP distribution. It first validates the context (not in arena, not PvP, not immortal), then applies toggle modifiers. The multiclass division happens twice, creating quadratic scaling. A per-class loop then processes each class independently (iterating `MIN_CLASS_IND` through `MAX_CLASSES`, skipping zero-level indices), applying soft caps and awarding practice points based on XP intervals within the level range.

The soft cap uses a logarithmic function that returns values between 1.0 and 2.0. The threshold is `gainmod = 1.15 * level`; gains below this threshold receive no reduction. As the ratio of attempted gain to baseline increases, the cap asymptotically approaches 2.0, preventing unbounded XP spikes while allowing legitimate large gains.

FAE_TOUCHED division uses a `fae_reduction_done` flag to ensure single application across all classes. Final experience assignment uses `addToExp()` to modify the character's exp value.

### Practice Point Distribution

Practice points are awarded not at level boundaries but at XP intervals within each level. The `pracsPerLevel()` function calculates how many practices a level should yield based on class, intelligence, and multiclass status. The level's XP range is then divided into that many segments; crossing each segment boundary awards one practice. This means gaining exactly enough XP to level might award zero practices if the XP lands between interval boundaries.

### Trophy Database Integration

The trophy system in `cmd_trophy.cc` tracks kill counts per mob type per character. The `getExpModVal()` function calculates modifiers: kills 1-8 return 100%, then each additional kill reduces the modifier by `0.5/14` until reaching the 30% floor. The `FRACT()` macro in `combat.cc` applies this modifier during `gainExpPerHit()` processing.

Trophy counts decay at 0.25 per game pulse through `procCharTickUpdate`, gradually restoring full XP rates for creatures a player stops killing.

### Group Share Distribution

The `gainExpPerHit()` function in `combat.cc` handles group XP distribution. It counts participating group members within combat range, divides total XP by that count, then applies each character's individual trophy modifier. Range validation ensures only nearby characters benefit from a kill.

Combat list iteration uses the `gCombatNext` caching pattern to prevent iterator invalidation during potential character deletion within `gain_exp()` processing.

### Death Penalty Processing

The `deathExp()` function in `combat.cc` calculates loss as the minimum of 20% of current XP and 25 times the mob XP value for the character's level. This protects low-level characters (who lose little because their current XP is low) while capping high-level losses. PvP deaths divide the penalty by 10. The `die()` function applies the penalty by calling `gain_exp()` with a negative value.

### Level Advancement

The `advanceLevel()` function increments the specified class level and invokes `doHPGainForLev()` to award hit points. The HP formula uses a class-specific random range (e.g., warrior gets 6-11, mage gets 3-7) multiplied by a constitution modifier (0.8-1.25). The `max_exp` threshold is not set here; it is updated in `gain_exp()`. Level 50 advancement triggers Discord notification through configured webhooks and unlocks multiclass options (single-class gets ALLOW_DOUBLECLASS, dual-class gets ALLOW_TRIPLECLASS).

### Legacy Character Migration

Characters converted from older systems may have `max_exp` set to zero. The XP gain system detects this condition and initializes `max_exp` to the appropriate threshold, capping current XP at the level 50 requirement to prevent overflow.

## Troubleshooting

### Character Not Gaining XP

**Symptoms:** Player kills mobs but XP remains unchanged.

**Causes:**
- Character is in an arena room
- Character has `TOG_NO_XP_GAIN` quest bit set
- Character is fighting another player (PvP mode)
- Character is immortal level

**Diagnostic:** Check room flags with builder commands. Verify quest bits. Confirm target is NPC, not PC.

**Fix:** Move to non-arena room. Clear blocking quest bits. Ensure combat is PvE.

### XP Gains Seem Too Low

**Symptoms:** Player earns much less XP than expected from high-level mobs.

**Causes:**
- Trophy system reducing XP for frequently killed mob
- Multiclass penalty dividing gains
- Soft cap limiting large single gains
- FAE_TOUCHED quest bit active

**Diagnostic:** Use trophy command to check modifier for mob type. Count character's classes. Look for FAE_TOUCHED in affects.

**Fix:** Kill different mob types to restore trophy rates. Accept multiclass penalty as design. Clear FAE_TOUCHED if unintended.

### Practice Points Not Awarded at Level Up

**Symptoms:** Character levels but gains zero practices.

**Causes:**
- XP landed between interval boundaries within the level
- This is expected behavior, not a bug

**Diagnostic:** Compare exact XP gained against level's delta_exp intervals. Calculate interval as `(level_end_xp - level_start_xp) / pracsPerLevel`.

**Fix:** Continue gaining XP; practices will be awarded when crossing the next interval boundary.

### Death Penalty Exceeds Expected Amount

**Symptoms:** Low-level character loses disproportionate XP on death.

**Causes:**
- Penalty formula uses min() of two values; at low levels, the 20% cap can exceed the level-scaled amount
- This is the intended protection mechanism working correctly

**Diagnostic:** Calculate both penalty values manually and verify min() is selecting the lower one.

**Fix:** This is working as designed. Low XP totals mean 20% losses are small in absolute terms.

### Multiclass Character Levels Slowly

**Symptoms:** Dual or triple-class character takes far longer to level than single-class peers.

**Causes:**
- Quadratic XP penalty (divide by classes squared) is intentional design
- Practice point reduction compounds the slowdown

**Diagnostic:** Verify class count matches expectation. Calculate expected XP rate: single-class gets 100%, dual gets 25%, triple gets 11%.

**Fix:** This is intentional balance. Multiclass characters trade leveling speed for versatility.

### Trophy Modifier Stuck at Minimum

**Symptoms:** Player consistently gets "little" XP from all mob types.

**Causes:**
- Player has been grinding same mobs heavily
- Trophy decay may not be keeping pace with kill rate

**Diagnostic:** Check trophy command output for kill counts across mob types.

**Fix:** Diversify targets. Allow time for trophy decay. Seek out new areas with unfamiliar creatures.

### Soft Cap Reducing Expected Gains

**Symptoms:** Large single kills or combat bursts yield less XP than calculated base value.

**Causes:**
- Gain exceeded the soft cap threshold (`1.15 * current_level`)
- Logarithmic scaling applied to reduce the spike

**Diagnostic:** Verify threshold calculation matches expected level. Confirm gain amount exceeds this value.

**Fix:** This is intentional anti-exploit behavior. Soft caps prevent unbounded XP spikes while allowing reasonable large gains from difficult encounters.

### FAE_TOUCHED Not Halving XP

**Symptoms:** Character with FAE_TOUCHED quest bit receives full XP.

**Causes:**
- Character has no active classes (zero-level in all indices)
- Character is immortal (bypasses `gain_exp()` entirely)
- `TOG_NO_XP_GAIN` blocking all gain before FAE_TOUCHED check

**Diagnostic:** Verify quest bit state through `hasQuestBit`. Confirm character has at least one active class. Check for blocking toggles.

**Fix:** Ensure character meets prerequisites for XP gain processing.

### Legacy Character With Zero Max_Exp

**Symptoms:** Level 50 character has zero or incorrect `max_exp` value.

**Causes:**
- Character converted from older database format
- Legacy conversion handling not yet triggered

**Diagnostic:** Check `getMaxExp()` return value.

**Fix:** Trigger recalculation with any XP gain. The system clamps exp to level 50 threshold and recalculates `max_exp` automatically.
