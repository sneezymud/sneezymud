---
title: Experience and Leveling System
description: Player progression system managing dynamic XP requirements, practice point allocation, multiclass penalties, trophy modifiers, and various quest-based XP modifications across multiple classes.
keywords:
  - gain-exp
  - advanceLevel
  - getExpClassLevel
  - mob-exp
  - pracsPerLevel
  - deathExp
  - gainExpPerHit
  - FRACT
  - trophy-system
  - multiclass-penalty
  - soft-cap
  - XP-distribution
  - level-advancement
  - practice-points
category: Important Systems

  - group-party.md
  - combat-formulas.md
  - quest-system.md
last_updated: 2026-01-29
source_files:
  - code/code/misc/limits.cc
  - code/code/misc/combat.cc
  - code/code/misc/gaining.cc
  - code/code/cmd/cmd_low.cc
  - code/code/cmd/cmd_trophy.cc
  - code/code/misc/being.h
related: [spell-skill-framework.md]
---

# Experience and Leveling System

The experience and leveling system in SneezyMUD manages player progression, practice point allocation, and character advancement across multiple classes. It features dynamic XP requirements, soft caps, multiclass penalties, trophy modifiers, and various quest-based XP modifications.

## Core Concepts

### Experience Storage

Experience is stored as double-precision floating-point values in each character:

```cpp
// From being.h
double exp;           // Current experience points
double max_exp;       // Experience required for next level (calculated threshold)
```

### Level System

Characters maintain multiple levels simultaneously for each class:

```cpp
// From being.h
ubyte level[MAX_SAVED_CLASSES];  // 12 levels, one per class
```

### Experience vs Levels

- **Experience (XP)**: Raw point value accumulated through combat and quests (0 to billions)
- **Max Experience (max_exp)**: Threshold at current level; when exp >= max_exp, character levels up
- **Level**: Current class rank (1-50 for mortals, higher for immortals)

Each class level is independent. A multiclass character might be level 20 Mage, level 15 Cleric, and level 10 Warrior simultaneously.

## XP Calculation Formulas

### Level XP Requirement: getExpClassLevel()

Located in `code/code/misc/gaining.cc`

**Formula (Recursive):**
```
exp_for_level(L) = exp_for_level(L-1) + (kills_to_level(L-1) * mob_exp(L-1))

Where:
  kills_to_level(level) = 17 + (1.25 * level)
  mob_exp(level) = calculates XP value of a mob at that level
```

**Example progression:**
- Level 1: 0 XP
- Level 2: 0 + (17 * mob_exp(1)) XP
- Level 3: Level2_XP + (18.25 * mob_exp(2)) XP
- Level 50: Sum of all previous levels' requirements

This creates an exponential XP curve where each level requires significantly more XP than the previous.

### Mob Experience Value: mob_exp()

Located in `code/code/cmd/cmd_low.cc`

**Formula:**
```
exp = 1.0
for each delay level (dlev) from 0 to level:
  exp += 1.020 * (dlev * exp) * RESOLUTION

Special case (Level 50+):
  exp /= 3  (reduced XP for overpowered mobs)
```

The calculation uses compound growth where each level builds on the previous, resulting in exponential scaling. Level 50+ mobs deal 1/3 XP to prevent power-leveling at endgame.

### Practice Points Calculation: pracsPerLevel()

Located in `code/code/misc/limits.cc`

**Components:**
- **Base class value**: Varies by class (Warriors ~2, Mages ~3, etc.)
- **Intelligence modifier**: Higher INT characters get more pracs per level
- **Level split**: Different rates for levels 1-29 (fast) vs 30+ (slow)
- **Multiclass penalty**: Reduced pracs for each additional class

**Formula outline:**
```
base = class_dependent_value
if level < 30:
  pracs = base * int_modifier
else:
  pracs = base * 0.5 * int_modifier

if multiclass:
  pracs /= number_of_classes
```

## XP Gain System

### Primary Function: gain_exp()

Located in `code/code/misc/limits.cc`

**Entry Parameters:**
- `ch`: Character receiving XP
- `gain`: Base XP amount to award
- `dam`: Damage dealt (used for soft cap calculations)

**Processing Steps:**

1. **Arena Check**: No XP in arena rooms
2. **PvP Check**: No XP for PvP kills
3. **Immortal Check**: Immortals gain no XP
4. **Negative XP**: Direct subtraction (death penalty)
5. **Toggle Modifiers**: Apply DOUBLEEXP if active
6. **Quest Bit Check**: Block XP if TOG_NO_XP_GAIN set
7. **Multiclass Division**: `gain /= howManyClasses()` twice
8. **Per-Class Loop**: Process each class the character has

**In the Per-Class Loop:**

```cpp
for each class the character has:
  peak = XP required for next level
  curr = XP required for current level
  gainmod = 1.15 * current_level

  // Soft cap calculation
  if gain > threshold:
    softmod = (1.0 - pow(1.1, -1.0 * (gain / newgain))) + 1.0
    newgain *= softmod
```

The soft cap uses a logarithmic function to prevent massive XP gains from exceeding reasonable limits. The cap returns values between 1.0 and 2.0.

### XP Modifiers

| Modifier | Location | Effect |
|----------|----------|--------|
| DOUBLEEXP toggle | gain_exp() | Multiplies all XP by 2 |
| FAE_TOUCHED quest bit | gain_exp() | Divides XP by 2 (half XP gain) |
| TOG_NO_XP_GAIN quest bit | gain_exp() | Blocks all XP gain completely |
| Multiclass penalty | gain_exp() | Divide by (classes × classes) |

### Practice Point Award

When a character gains enough XP to pass a level threshold:

```cpp
delta_exp = (level_end_xp - level_start_xp) / pracsPerLevel(class, false)

for each delta_exp interval crossed:
  gain_pracs++
```

**Critical Detail**: Practice points are awarded based on **intervals** within the current level. A character might gain 1-2 pracs per level depending on how the XP lands relative to level boundaries.

## Level Advancement

### advanceLevel()

Located in `code/code/misc/limits.cc`

**Triggered When**: `getExp() >= getMaxExp()` for any class

**Processing:**
1. Increment class level
2. Update max_exp to next threshold
3. Award HP: `gain_hp = 10 + (con_modifier * 5)`
4. Send Discord notification at level 50 (for fame)
5. Unlock multiclass specializations at level 30
6. Update spell/skill access

**Special Case**: Level 50 Discord notification for celebratory tracking

## Death System

### Death Penalty: deathExp()

Located in `code/code/misc/combat.cc`

**Formula:**
```cpp
amt = 25.0 * mob_exp(float(GetMaxLevel()));
amt = min(1 * getExp() / 5, amt);

if isPking():
  amt /= 10  (PK death penalty reduced to 1/50th of level 1 death)

return amt;  // Amount to subtract from exp
```

**Examples (hypothetical):**
- Level 20 death: min(current_exp/5, 25*mob_exp(20))
- Level 50 death: min(current_exp/5, 25*mob_exp(50))
- PvP death: Same calculation but divided by 10

The penalty is the **minimum** of 20% of current XP or 25 times the mob XP value for that level.

### Death Processing: die()

Located in `code/code/misc/combat.cc`

**XP Subtraction:**
```cpp
gain_exp(this, -deathExp(), -1);
```

Negative XP in gain_exp() directly subtracts without applying soft caps or modifiers.

**Age Penalty**: Random 0-3 years added to character age

**FREE_DEATHS Affect**: Characters with this affect can die without XP penalty (used for quests/events)

## Group Experience Sharing

### Per-Hit XP Distribution: gainExpPerHit()

Located in `code/code/misc/combat.cc`

**Group Share Calculation:**
```cpp
exp_shares = count_group_members_in_range(ch)
per_char_exp = total_exp / exp_shares
```

**Range Check**: Only characters within combat range share XP

**Trophy Modifier Application:**
```cpp
FRACT(ch, victim)  // Returns 0.3 to 1.0 modifier
per_char_exp *= trophy_modifier;
```

**Multi-Target Handling**: Large groups distribute XP evenly among all participating members in range.

## Trophy System

### Trophy Database Integration

Located in `code/code/cmd/cmd_trophy.cc`

**Purpose**: Track mob kills and provide XP modifiers based on kill frequency

### XP Modifier Calculation: getExpModVal()

**Formula Parameters:**
```cpp
free_kills = 8              // First 8 kills give no penalty
step_mod = 0.5
num_steps = 14.0
min_mod = 0.3  (30%)
max_mod = 1.0  (100%)
```

**Calculation:**
```cpp
if kill_count <= free_kills:
  modifier = max_mod (1.0 = 100% XP)
else:
  kills_over = kill_count - free_kills
  step_value = kills_over / num_steps
  reduction = step_mod * step_value
  modifier = max(min_mod, max_mod - reduction)

  // Returns range: 1.0 (full) down to 0.3 (30%)
```

**Effect in gainExpPerHit():**
```cpp
per_char_exp *= FRACT(ch, victim);  // Applies modifier
```

### Trophy Decay: procCharTickUpdate

**Decay rate**: -0.25 kill count per game pulse

Characters gradually lose trophy kill count if they stop killing frequently.

**Descriptive Text**:
- 1.0 (100%) = "full"
- 0.7-0.9 = "much"
- 0.5-0.6 = "fair amount"
- 0.3-0.4 = "some"
- 0.0-0.2 = "little"

## Multi-Class Experience System

### Multi-Class Penalty

**Location**: gain_exp()

```cpp
gain /= ch->howManyClasses();  // First division
gain /= ch->howManyClasses();  // Second division
```

A character with 2 classes gains: `base_xp / 4` per class
A character with 3 classes gains: `base_xp / 9` per class

This prevents multiclass characters from leveling multiple classes simultaneously at the same rate as single-class characters.

### Practice Point Penalties

From pracsPerLevel():
```cpp
if multiclass:
  pracs /= number_of_classes
```

Multi-class characters also gain fewer practices per level.

## Quest-Based XP Modifiers

### Quest Bits Affecting XP

| Quest Bit | Effect | Location |
|-----------|--------|----------|
| `TOG_FAE_TOUCHED` | XP / 2 | gain_exp() |
| `TOG_NO_XP_GAIN` | XP blocked | gain_exp() |
| `TOG_DOUBLEEXP` | XP × 2 | gain_exp() |

**Application Order**:
1. Check TOG_NO_XP_GAIN (complete block)
2. Apply DOUBLEEXP (2×)
3. Apply multiclass penalties (÷ classes²)
4. Apply soft cap (logarithmic)
5. Apply FAE_TOUCHED (÷ 2)

## Level Caps and Boundaries

### Maximum Levels
- **Mortals**: Level 1-50
- **Immortals**: Level 50+
- **Level 30 Gate**: Multiclass specializations unlock at level 30

### Soft Cap Mechanism

Located in gain_exp():

```cpp
softmod = (1.0 - pow(1.1, -1.0 * (gain / newgain))) + 1.0

// This returns values between 1.0 and 2.0
// As gain/newgain increases, softmod approaches 2.0
// Prevents unlimited XP spikes while allowing legitimate large gains
```

The soft cap is **logarithmic**, not linear. Large XP gains are reduced more aggressively than moderate gains.

## Critical Edge Cases and Gotchas

### 1. Arena Rooms Block All XP
```cpp
if (ch->roomp && ch->roomp->isRoomFlag(ROOM_ARENA)) {
  return;  // No XP gained at all
}
```
Players cannot farm XP in arena rooms.

### 2. PvP Kills Give No XP
```cpp
if (ch->isPking())
  return;  // Attacker gains no XP for killing
```
Prevents PvP-based power leveling.

### 3. Multiclass Division Happens Twice
divides by `howManyClasses()` **twice**:
```cpp
gain /= ch->howManyClasses();
gain /= ch->howManyClasses();
```
This is intentional quadratic scaling, not a bug.

### 4. FAE_TOUCHED Applied After Soft Cap
Applied **after** soft cap calculation:
```cpp
// Soft cap applied first
// Then FAE_TOUCHED applied
if (ch->hasQuestBit(TOG_FAE_TOUCHED) and !fae_reduction_done) {
  fae_reduction_done = true;
  gain /= 2;
}
```
This means FAE_TOUCHED reduces even capped XP.

### 5. Max_Exp Reset for Level 50 Characters
Handles legacy characters:
```cpp
if (ch->isPc() && ch->getMaxExp() == 0) {
  ch->setExp(min(ch->getExp(), getExpClassLevel(50)));
  ch->setMaxExp(curr);
}
```
Prevents exp overflow for characters converted from old system.

### 6. Trophy Free Kills
First 8 kills of any mob provide full 100% XP. Only the 9th+ kill incurs penalties. This prevents new players from being punished.

### 7. Practice Point Generation is Interval-Based
Practices are awarded based on crossing delta_exp boundaries, not fixed amounts per level:
```cpp
delta_exp = (t_peak - t_curr) / ch->pracsPerLevel(Class, false);
for (t_exp = t_curr + delta_exp; t_exp <= new_exp && t_exp <= t_peak; t_exp += delta_exp) {
  if (t_exp > exp && t_exp <= new_exp) {
    gain_pracs++;
  }
}
```
Gaining exactly enough XP to level up might award 0 practices if the exp lands between intervals.

### 8. Death Penalty Uses Min(), Not Flat
```cpp
amt = min(1 * getExp() / 5, amt);
```
At low levels, players might lose more XP (1/5 of current) than the formula suggests. At high levels, the 25×mob_exp cap dominates.

### 9. Per-Class Loop Processes All Classes Independently
Each class gets the full gain_exp treatment:
```cpp
for (Class = MIN_CLASS_IND; Class < MAX_CLASSES; Class++) {
  if (!ch->getLevel(Class))
    continue;
  // Full gain_exp processing for this class
}
```
A multiclass character gains (base_xp / classes²) × number_of_classes times total.

## Key Source Files

| File | Function | Purpose |
|------|----------|---------|
| `code/code/misc/limits.cc` | advanceLevel() | Level up processing |
| `code/code/misc/limits.cc` | gain_exp() | Core XP gain |
| `code/code/misc/limits.cc` | pracsPerLevel() | Practice rate calculation |
| `code/code/misc/combat.cc` | gainExpPerHit() | Per-hit distribution |
| `code/code/misc/combat.cc` | deathExp() | Death penalty |
| `code/code/misc/combat.cc` | die() | Death processing |
| `code/code/misc/combat.cc` | FRACT() | Trophy modifier |
| `code/code/misc/gaining.cc` | getExpClassLevel() | Level XP requirement |
| `code/code/cmd/cmd_low.cc` | mob_exp() | Mob XP value |
| `code/code/cmd/cmd_low.cc` | kills_to_level() | Kills per level |
| `code/code/cmd/cmd_trophy.cc` | getExpModVal() | Trophy modifier |
| `code/code/misc/being.h` | - | exp/max_exp members |
| `code/code/misc/being.h` | - | level[] array |
