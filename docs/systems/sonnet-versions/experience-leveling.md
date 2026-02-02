---
title: Experience and Leveling System
category: important
keywords: [progression, XP-gain, practice-points, multiclass-penalty, trophy-modifier, soft-cap, level-advancement, death-penalty]
related: [group-party.md, combat-formulas.md, quest-system.md, spell-skill-framework.md]
primary_symbols:
  functions: [gain_exp, advanceLevel, getExpClassLevel, pracsPerLevel, gainExpPerHit, deathExp, mob_exp, FRACT, getExpModVal]
  classes: [TBeing]
  files: [code/code/misc/limits.cc, code/code/misc/combat.cc, code/code/misc/gaining.cc, code/code/cmd/cmd_trophy.cc]
---

# Experience and Leveling System

## Overview

The experience system manages player progression through dynamic XP requirements, practice point allocation, and multiclass advancement. Characters accumulate floating-point experience values through combat and quests, progressing independently in up to 12 simultaneous class levels. The system balances progression through exponential scaling, soft caps on massive gains, multiclass penalties, and trophy-based diminishing returns for repeated kills.

Core progression mechanics operate on three interrelated values: current experience accumulates from all sources; max experience defines the threshold for advancement; and class levels track independent progression per class. When current experience meets or exceeds max experience for any class, that class advances, awarding hit points and practice points while recalculating the next threshold.

The system prevents exploitation through multiple mechanisms: arena rooms and PvP kills award no experience; soft caps apply logarithmic scaling to unusually large gains; the trophy system reduces returns for farming the same mobs; and multiclass characters face quadratic XP penalties while gaining proportionally fewer practice points.

## Patterns

### XP Award Flow

Combat damage triggers per-hit experience distribution through gainExpPerHit, which calculates group shares, applies trophy modifiers, and invokes gain_exp for each participant. The gain_exp function validates eligibility through arena, PvP, and immortal checks before applying global modifiers like DOUBLEEXP. Multiclass division occurs twice sequentially, creating quadratic scaling. Per-class processing then applies soft caps based on level-appropriate thresholds and awards practice points at fixed intervals within each level.

When experience crosses a level boundary, advanceLevel increments the class level, updates max_exp to the next threshold calculated by getExpClassLevel, awards hit points based on constitution, and sends Discord notifications for level 50 achievement. At level 30, multiclass specializations unlock automatically.

### Trophy Diminishing Returns

The trophy system tracks kill counts per mob vnum in the database. The first 8 kills of any mob award full experience through getExpModVal returning 1.0. Subsequent kills reduce the modifier linearly across 14 steps to a minimum of 0.3, meaning the 30th+ kill awards only 30% of base XP. Trophy counts decay at 0.25 per game pulse, allowing modifiers to recover over time.

FRACT applies this modifier within gainExpPerHit before invoking gain_exp, ensuring trophy penalties affect the base calculation before soft caps and quest modifiers. This creates a natural progression loop where players must seek new content rather than farming optimal spawns indefinitely.

### Multiclass Penalty Cascades

Multiclass characters face compounding penalties across multiple systems. XP gain divides by howManyClasses twice in gain_exp, creating quadratic scaling: dual-class characters receive base_xp/4 per class, triple-class receive base_xp/9. Practice point allocation divides by class count once in pracsPerLevel, reducing training capacity proportionally. These penalties apply independently to each class during the per-class loop, ensuring balanced progression across all character roles.

### Death Penalty Calculation

Death invokes deathExp to calculate XP loss as the minimum of 20% of current experience or 25 times the mob_exp value for the character's maximum level. This creates a progressive penalty that caps at predictable amounts based on level rather than allowing catastrophic losses. PvP deaths divide this amount by 10, reducing the penalty to 2% or 2.5 mob equivalents. The FREE_DEATHS affect bypasses all XP loss for quest and event participation.

The die function applies this penalty through gain_exp with negative values, which bypass soft caps and modifiers to ensure consistent loss regardless of quest bits or toggles.

## Reference

### Experience Storage Model

TBeing stores exp as double-precision current experience and max_exp as the calculated advancement threshold. The level array contains ubyte values for MAX_SAVED_CLASSES entries, tracking independent progression per class. These values persist across sessions through database serialization.

### Level XP Requirements

getExpClassLevel calculates cumulative XP thresholds recursively. Each level L requires the previous level's total plus kills_to_level(L-1) multiplied by mob_exp(L-1). The kills_to_level formula returns 17 + (1.25 * level), creating linear growth in kill requirements that combines with exponential mob_exp scaling for super-exponential total requirements.

mob_exp compounds growth at 1.020 per level multiplied by current value and level depth, producing exponential scaling. Level 50+ mobs divide result by 3 to prevent endgame power-leveling.

### Practice Point Allocation

pracsPerLevel returns class-dependent base values modified by intelligence. Levels 1-29 use full base rates; levels 30+ use half base rates. Multiclass characters divide the result by class count. Actual practice award occurs at fixed intervals within each level, calculated as (level_end_xp - level_start_xp) / pracsPerLevel. Characters may receive 0-2 practices per level depending on where XP gain lands relative to interval boundaries.

### Soft Cap Function

The soft cap applies when single-hit XP gains exceed level-appropriate thresholds. The formula calculates softmod as (1.0 - pow(1.1, -1.0 * (gain / newgain))) + 1.0, producing values between 1.0 and 2.0. This logarithmic function allows legitimate large gains while preventing unlimited spikes from exploits or unusual combat scenarios.

### Quest Bit Effects

TOG_NO_XP_GAIN blocks all experience award completely when checked at gain_exp entry. TOG_DOUBLEEXP multiplies base gain by 2 before multiclass division. TOG_FAE_TOUCHED divides final gain by 2 after soft cap application, ensuring reduction applies even to capped values. These bits affect all XP sources uniformly, including quest rewards and combat.

### Group Distribution

gainExpPerHit counts group members within combat range and divides total experience evenly among shares. Range validation ensures only participants close enough to assist receive portions. Trophy modifiers apply per-character based on individual kill histories before distribution, allowing experienced players and novices in the same group to receive different amounts for identical participation.

## Implementation

### gain_exp Processing Pipeline

The function begins with validation: arena rooms return immediately; PvP checks block attacker XP; immortals receive no gain. Negative gain values subtract directly without further processing for death penalties. DOUBLEEXP toggle applies to positive gains before any division.

TOG_NO_XP_GAIN quest bit returns early if set. Multiclass division executes twice sequentially using howManyClasses. The per-class loop iterates MIN_CLASS_IND through MAX_CLASSES, skipping indices where getLevel returns zero.

Within each class iteration, getExpClassLevel calculates peak (next level) and curr (current level) thresholds. The gainmod value of 1.15 * level establishes soft cap threshold. When gain exceeds this threshold, the soft cap formula applies. Practice point intervals calculate based on (peak - curr) / pracsPerLevel, incrementing gain_pracs counter for each interval crossed between old and new experience values.

FAE_TOUCHED division by 2 applies after soft cap processing using a fae_reduction_done flag to ensure single application across all classes. Final experience assignment uses addToExp to modify the character's exp value, followed by conditional advanceLevel invocation when new experience meets or exceeds max_exp.

### advanceLevel Level-Up Sequence

The function increments the specified class level, recalculates max_exp using getExpClassLevel for the new level, and invokes gain_hp to award 10 plus (con_modifier * 5) hit points. Level 50 advancement triggers Discord notification through configured webhooks. Level 30 advancement unlocks multiclass specialization options through spec system integration.

Legacy character handling checks for zero max_exp on PC characters, clamping current exp to level 50 threshold and recalculating max_exp to prevent overflow from old database states.

### gainExpPerHit Distribution Logic

Group member counting within combat range establishes exp_shares denominator. Division of total_exp by exp_shares produces per_char_exp base value. FRACT invocation with character and victim parameters retrieves trophy modifier from database lookups in getExpModVal. Multiplication of per_char_exp by modifier produces final value passed to gain_exp.

Combat list iteration uses gCombatNext caching pattern to prevent iterator invalidation during potential character deletion within gain_exp processing.

### deathExp Penalty Calculation

The function calculates base amount as 25.0 * mob_exp(GetMaxLevel()), establishing level-appropriate cap. Minimum comparison with getExp() / 5 selects smaller value, ensuring either 20% current XP or 25 mob equivalents as penalty. PvP state check divides result by 10 for player versus player deaths.

### Trophy System Database Operations

getExpModVal queries trophy counts per character-vnum pair from persistent storage. First 8 kills return max_mod of 1.0 immediately. Kills beyond free_kills threshold calculate kills_over value, divide by num_steps (14.0) to produce step_value, multiply by step_mod (0.5) for reduction amount, and subtract from max_mod. Maximum comparison with min_mod (0.3) ensures floor.

procCharTickUpdate applies decay of -0.25 to kill counts per game pulse, preventing indefinite penalties for mobs not encountered recently.

## Troubleshooting

### Unexpected Practice Awards

Practice points award at intervals within levels rather than fixed amounts per level. Characters gaining exactly enough XP to level up may receive zero practices if the final exp value lands between interval boundaries. Verify the calculation interval using (level_end_xp - level_start_xp) / pracsPerLevel for the specific class and compare against actual XP gain to determine expected award count.

### Multiclass XP Seeming Too Low

Multiclass division applies quadratically: howManyClasses executes twice sequentially in gain_exp, not once. A dual-class character receives base_xp / 4 per class, not base_xp / 2. Verify class count through howManyClasses and confirm expected division matches (count * count). This is intentional game balance, not a bug.

### Trophy Penalties Not Applying

Trophy modifiers apply in gainExpPerHit before gain_exp invocation, affecting only combat XP distribution. Quest rewards or direct XP modifications bypass this system. Verify kill count through database trophy table queries and confirm getExpModVal calculation using free_kills threshold of 8 and num_steps of 14. Trophy decay of 0.25 per pulse may have reduced counts since last kill.

### FAE_TOUCHED Not Halving XP

FAE_TOUCHED applies after soft cap processing within the per-class loop. Characters with no classes will not trigger the check. Immortals bypass gain_exp entirely. Verify quest bit state through hasQuestBit and confirm character has at least one active class. Check for TOG_NO_XP_GAIN blocking all gain before FAE_TOUCHED processing.

### Arena Rooms Blocking All XP

ROOM_ARENA flag check returns immediately from gain_exp before any processing. Verify room flags through roomp->isRoomFlag(ROOM_ARENA). This is intentional to prevent arena farming; remove the flag or move characters to non-arena rooms.

### Soft Cap Reducing Expected Gains

Soft cap threshold calculates as 1.15 * current_level. Gains below this threshold receive no reduction. Gains above apply logarithmic scaling producing multipliers between 1.0 and 2.0. Verify threshold calculation matches expected level and confirm gain amount exceeds this value. Soft caps prevent exploit XP but allow reasonable large gains from difficult encounters.

### Death Penalty Seeming Inconsistent

deathExp uses minimum of 20% current XP or 25 * mob_exp(max_level). At low levels, 20% dominates; at high levels, the 25 mob_exp cap dominates. PvP deaths divide by 10, reducing penalty to 2% or 2.5 mob equivalents. FREE_DEATHS affect bypasses all loss. Verify character state through isPking and affect list for FREE_DEATHS.

### Level 50 Characters With Zero Max_Exp

Legacy conversion handling clamps exp to level 50 threshold when max_exp equals zero for PC characters. This prevents overflow from old database formats. Affected characters recalculate max_exp automatically on next gain_exp invocation. Manually verify through getMaxExp() and trigger recalculation with any XP gain.
