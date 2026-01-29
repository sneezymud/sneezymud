---
title: Vital Statistics - Hunger, Thirst, and Age
description: This document covers the vital statistics systems in SneezyMUD including Hunger and Thirst (nutrition needs) and Age and Lifespan (character aging). These systems add survival elements and realism to gameplay.
keywords: condTypeT, DRUNK, FULL, THIRST, PEE, POOP, gainCondition, foodNDrink, calcNutrition, TOG_REAL_AGING, age_mod_for_stat, graf, starvation, DAMAGE_STARVATION, TALENT_FISHEATER, TALENT_MEATEATER
category: Understanding Systems

  - race-system.md
  - scheduler-pulses.md
  - death-dying.md
last_updated: 2026-01-29
source_files:
  - code/code/misc/enum.h
  - code/code/obj/obj_food.cc
  - code/code/misc/limits.cc
  - code/code/misc/periodic.cc
  - code/code/misc/stats.cc
  - code/code/sys/gametime.cc
  - code/code/misc/race.cc
  - code/code/misc/charfile.h
  - code/code/sys/create_character.cc
  - code/code/misc/movement.cc
related: [character-foundation.md]
---

# Vital Statistics: Hunger, Thirst, and Age

This document covers the vital statistics systems in SneezyMUD: **Hunger** and **Thirst** (nutrition needs), and **Age** and **Lifespan** (character aging). These systems add survival elements and realism to gameplay.

## Overview

 SneezyMUD tracks five condition types via `condTypeT`:

| Condition | Index | Range | Purpose |
|-----------|-------|-------|---------|
| `DRUNK` | 0 | 0-24 (-1 = immune) | Intoxication level |
| `FULL` | 1 | 0-24 (-1 = immune) | Hunger satiation |
| `THIRST` | 2 | 0-24 (-1 = immune) | Hydration level |
| `PEE` | 3 | 0-24 | Bladder pressure |
| `POOP` | 4 | 0-24 | Bowel pressure |

**Key points:**
- Value `-1` indicates immunity (immortals, certain states)
- Value `0` means critical need (starving/dehydrated)
- Values `1-2` trigger warning messages
- Values above `20` indicate full satiation
- Conditions stored in `charFile.conditions[]` for persistence

**Source:** `code/code/misc/enum.h:413-422`

## Hunger and Thirst System

### Data Storage

Conditions are stored in `TBeing::specials.conditions[]`:

```cpp
void TBeing::setCond(condTypeT i, short val) {
    if (GetMaxLevel() > MAX_MORT) {
        specials.conditions[i] = -1;  // Immortals are immune
        return;
    }
    specials.conditions[i] = val;
}

short TBeing::getCond(condTypeT i) const {
    return specials.conditions[i];
}
```

**Source:** `code/code/obj/obj_food.cc:1014-1026`

### Nutrition Drain (foodNDrink)

The `foodNDrink()` function drains hunger/thirst based on terrain type. Called from `updateHalfTickStuff()` once per half-tick (~36 seconds).

```cpp
void TBeing::foodNDrink(sectorTypeT sector, int modifier) {
    int food = 11 - modifier * TerrainInfo[sector]->hunger;
    int thirst = 9 - modifier * TerrainInfo[sector]->thirst;
    int drunk = 9 - modifier * TerrainInfo[sector]->drunk;

    food = max(0, food);
    thirst = max(0, thirst);
    drunk = max(0, drunk);

    if ((::number(0, 9) < 6) && !::number(0, food)) {
        gainCondition(FULL, -1);
        gainCondition(POOP, 1);
    }

    if ((::number(0, 9) < 5) && !::number(0, thirst)) {
        gainCondition(THIRST, -1);
        gainCondition(PEE, 1);
    }

    if ((::number(0, 9) < 6) && !::number(0, drunk))
        gainCondition(DRUNK, -1);
}
```

**Drain probability:**
- Higher terrain `hunger`/`thirst` values = slower drain
- Desert, jungle sectors drain faster
- Probability check: `!::number(0, food)` means food value of 0 always drains

**Source:** `code/code/obj/obj_food.cc:929-949`

### Condition Gain (gainCondition)

The `gainCondition()` function handles both positive and negative changes, applying racial modifiers:

```cpp
void TBeing::gainCondition(condTypeT condition, int value) {
    if (getCond(condition) == -1)  // Immune
        return;

    if (value > 0) {
        switch (condition) {
            case FULL:
                // Racial metabolism modifier
                value = (int)(value * getMyRace()->getFoodMod());
                value = max(1, value);
                // Body mass modifier (180 lb baseline)
                value = (int)(value * 180.0 / getWeight());
                value = max(1, value);
                break;
            case THIRST:
                // Body mass modifier only
                value = (int)(value * 180.0 / getWeight());
                value = max(1, value);
                break;
            case DRUNK:
                // Racial modifier
                value = (int)(value * getMyRace()->getDrinkMod());
                value = max(1, value);
                // Body mass modifier
                value = (int)(value * 180.0 / getWeight());
                value = max(1, value);
                // Alcoholism skill reduces drunkenness gain
                value = (int)(value * ((105 - getSkillValue(SKILL_ALCOHOLISM)) / 100.0));
                break;
        }
    }
    value = min(value, 24);  // Cap at 24

    setCond(condition, getCond(condition) + value);
    // Clamp to 0-24 range
    if (getCond(condition) < 0) setCond(condition, 0);
    if (getCond(condition) > 24) setCond(condition, 24);
}
```

**Modifiers applied:**
- **Racial metabolism:** `Race::getFoodMod()` / `Race::getDrinkMod()`
- **Body mass:** Heavier characters need more food/drink (normalized to 180 lbs)
- **Alcoholism skill:** Reduces drunkenness gain (max ~5% reduction)

**Source:** `code/code/misc/limits.cc:1250-1449`

### Starvation Mechanics

When `FULL` or `THIRST` reaches 0 and stays there, the character takes damage:

```cpp
// In updateHalfTickStuff()
if (isPc() && !inImperia() && !inLethargica()) {
    int old_food = getCond(FULL);
    int old_drink = getCond(THIRST);

    foodNDrink(roomp ? roomp->getSectorType() : SECT_INSIDE_MOB, 4);

    // Both ticks at 0 = damage
    if ((!getCond(FULL) && !old_food) || (!getCond(THIRST) && !old_drink)) {
        if (desc) {
            points.hit -= 1;

            if (!old_food)
                sendTo(COLOR_BASIC, "<R>You are weak from lack of nutrients.<1>\n\r");
            if (!old_drink)
                sendTo(COLOR_BASIC, "<R>You are parched.<1>\n\r");

            updatePos();

            if (points.hit < -10) {
                vlogf(LOG_MISC, format("%s killed by starvation at %s (%d)") %
                    getName() % (roomp ? roomp->getName() : "nowhere") % inRoom());
                rc = die(DAMAGE_STARVATION);
                // ...
            }
        }
    }
}
```

**Starvation progression:**
1. `FULL` or `THIRST` drains to 0
2. Warning message: "You are weak from lack of nutrients" / "You are parched"
3. Lose 1 HP per half-tick while at 0
4. Death at -10 HP via `DAMAGE_STARVATION`

**Safe zones:** Imperia and Lethargica are immune to hunger/thirst drain.

**Source:** `code/code/misc/periodic.cc:1744-1774`

### Regeneration Penalties

Being hungry or thirsty reduces regeneration rates:

```cpp
// manaGain() and moveGain()
if (!getCond(FULL) || !getCond(THIRST))
    gain >>= 2;  // Divide by 4
```

**Effects:**
- Mana regeneration: 1/4 normal when hungry or thirsty
- Move regeneration: 1/4 normal when hungry or thirsty
- HP regeneration: Blocked entirely (only gains 1 HP if STUNNED)

**Source:** `code/code/misc/limits.cc:303-304, 500-501`

### Auto-Eat System

When `AUTO_EAT` is enabled (`autobits`), the game automatically eats/drinks when conditions hit 0:

```cpp
if ((condition == FULL) && desc && (desc->autobits & AUTO_EAT) && awake()) {
    // Check for statue in center square (low-level players)
    if ((in_room == Room::CS) && (GetMaxLevel() <= 3)) {
        parseCommand("pray statue", FALSE);
        return;
    }

    // Search equipment and inventory for food
    TFood* last_good = NULL;
    TBaseContainer* last_cont = NULL;
    // ... find food closest to spoiling ...

    if (last_good && !last_cont) {
        sprintf(buf, "%s", fname(last_good->name).c_str());
        doEat(buf);
    }
}
```

**Source:** `code/code/misc/limits.cc:1345-1440`

### Racial Food Preferences

Some races have special dietary requirements:

| Talent | Effect |
|--------|--------|
| `TALENT_FISHEATER` | 2x nutrition from fish, 5% from other foods |
| `TALENT_MEATEATER` | 2x nutrition from butchered meat, 5% from other foods |
| `TALENT_INSECT_EATER` | Can only eat insects |
| `TALENT_GARBAGEEATER` | Can eat organic trash, faster hunger drain |

**Garbage eater penalty:** Characters with `TALENT_GARBAGEEATER` experience accelerated hunger drain and take damage if they don't eat regularly:

```cpp
if (getMyRace()->hasTalent(TALENT_GARBAGEEATER) && getCond(FULL) < 19) {
    // Random chance to lose hunger
    if (getCond(FULL) > 0)
        setCond(FULL, max(0, getCond(FULL) - ::number(1, 5)));
    else if (getMove() > 0) {
        act("Your hunger makes you feel tired and listless.", ...);
        addToMove(-::number(4, 8));
    } else {
        act("Your hunger makes you feel weak.", ...);
        addToHit(-1);
    }
}
```

**Source:** `code/code/misc/periodic.cc:998-1016`, `code/code/obj/obj_food.cc:523-544`

### Nutrition and Weight

The `calcNutrition()` function tracks caloric balance for weight gain/loss:

```cpp
void TBeing::calcNutrition() {
    int threshold = 5000;
    int nutrDelta = 0;

    // Food level affects calorie balance
    if (getCond(FULL) <= 5)  nutrDelta--;   // Hungry = weight loss
    if (getCond(FULL) >= 6)  nutrDelta++;   // Eating = weight gain
    if (getCond(FULL) >= 10) nutrDelta++;   // More eating
    if (getCond(FULL) >= 20) nutrDelta++;   // Full

    // Exercise burns calories
    if (getMove() < (getMaxMove() * 0.25)) nutrDelta -= 2;
    if (getMove() < (getMaxMove() * 0.50)) nutrDelta -= 2;
    if (getMove() < (getMaxMove() * 0.75)) nutrDelta -= 2;
    if (getMove() > (getMaxMove() * 0.90)) nutrDelta -= 2;

    nutrition += nutrDelta;

    if (nutrition > threshold) {
        if ((getWeight() + 1) <= getMyRace()->getMaxWeight(getSex())) {
            sendTo("You feel as though you've been putting on some weight.\n\r");
            setWeight(getWeight() + 1);
        }
        nutrition = 0;
    } else if (nutrition < -threshold) {
        if ((getWeight() - 1) >= getMyRace()->getMinWeight(getSex())) {
            sendTo("You feel as though you've been losing some weight.\n\r");
            setWeight(getWeight() - 1);
        }
        nutrition = 0;
    }
}
```

**Weight changes:** +/- 1 lb when nutrition balance exceeds +/- 5000, clamped to racial min/max.

**Source:** `code/code/misc/periodic.cc:871-913`

## Age and Lifespan System

### Age Data Storage

Age is stored in `charFile` with three components:

```cpp
class charFile {
    time_t birth;           // Real-world timestamp of character creation
    unsigned short base_age; // Starting age at creation (racial)
    short age_mod;          // Manual age adjustments
};
```

**Source:** `code/code/misc/charfile.h:39, 51-52`

### Racial Age Generation

Starting age is determined by race:

```cpp
int Race::generateAge() const {
    return baseAge + dice(ageNumDice, ageDieSize);
}
```

**Format in race files:** `age X+YdZ` (e.g., `age 15+2d4` means 17-23 starting age)

**Source:** `code/code/misc/race.cc:885-886`

### Age Calculation

Current age combines birth time with game time passed:

```cpp
// Simplified from gametime.cc
void GameTime::mudTimePassed(time_t t2, time_t t1, time_info_data* now) {
    long secs = (long)(t2 - t1);

    now->hours = (secs / Pulse::SECS_PER_MUDHOUR) % 24;
    now->day = (secs / Pulse::SECS_PER_MUD_DAY) % 28;
    now->month = (secs / Pulse::SECS_PER_MUD_MONTH) % 12;
    now->year = (secs / Pulse::SECS_PER_MUD_YEAR);
}
```

**Time conversion:**
- 1 MUD hour = ~144 real seconds (2.4 real minutes)
- 1 MUD day = 24 MUD hours = ~57.6 real minutes
- 1 MUD month = 28 MUD days = ~26.9 real hours
- 1 MUD year = 12 MUD months = ~13.4 real days

**Source:** `code/code/sys/gametime.cc:153-175`

### Real Aging Toggle (TOG_REAL_AGING)

Age effects are **disabled by default**. Players must opt in via the `TOG_REAL_AGING` quest bit during character creation:

```cpp
{TOG_REAL_AGING, 0, "real aging",
 "You will suffer the affects of old age as you get older.", 0, 1}
```

All age-related mechanics check this toggle:

```cpp
if (!tb->hasQuestBit(TOG_REAL_AGING))
    return 0;  // No age modifiers
```

**Source:** `code/code/sys/create_character.cc:52-53`, `code/code/misc/stats.cc:119-120`

### Age-Based Stat Modifiers

The `age_mod_for_stat()` function returns stat modifiers based on "human equivalent" age:

```cpp
int age_mod_for_stat(const TBeing* tb, int age_num, statTypeT whichStat) {
    if (!tb->hasQuestBit(TOG_REAL_AGING))
        return 0;

    // age_num is "human equivalent" age (non-humans normalized)
    switch (whichStat) {
        case STAT_STR:
            if (age_num < 17) return 10;      // Youth bonus
            else if (age_num <= 30) return ...;  // Peak
            else if (age_num <= 60) return 0;    // Prime
            else return -10;  // Elderly penalty
        // ... similar for other stats
    }
}
```

**Source:** `code/code/misc/stats.cc:118-600`

#### Physical Stats (Decline with Age)

| Age | STR | BRA | AGI | DEX | CON | SPE |
|-----|-----|-----|-----|-----|-----|-----|
| <17 | +10 | +5 | +10 | +10 | +10 | +10 |
| 17-20 | +6 to +9 | +3 to +4 | +6 to +9 | +6 to +9 | +6 to +9 | +6 to +9 |
| 21-30 | +1 to +5 | +1 to +2 | +1 to +5 | +1 to +5 | +1 to +5 | +1 to +5 |
| 31-60 | 0 | 0 | 0 | 0 | 0 | 0 |
| 61-70 | -1 to -4 | -1 to -2 | -1 to -4 | -1 to -4 | -1 to -4 | -1 to -4 |
| 71-80 | -5 to -9 | -3 to -4 | -5 to -9 | -5 to -9 | -5 to -9 | -5 to -9 |
| 81+ | -10 | -5 | -10 | -10 | -10 | -10 |

#### Mental Stats (Improve with Age)

| Age | INT | WIS | FOC | PER |
|-----|-----|-----|-----|-----|
| <17 | -10 | -10 | -10 | -10 |
| 17-30 | -9 to -1 | -9 to -1 | -9 to -1 | -9 to -1 |
| 31-60 | 0 | 0 | 0 | 0 |
| 61-80 | +1 to +9 | +1 to +9 | +1 to +9 | +1 to +9 |
| 81+ | +10 | +10 | +10 | +10 |

#### Special Stats

| Age | KAR | CHA |
|-----|-----|-----|
| <17 | +10 | 0 |
| 17-60 | Linear decay to 0 | 0 |
| 61+ | Linear decay to -10 | 0 |

**Note:** CHA has no age modifiers in the current implementation.

### Age Effects on Gameplay

#### HP Regeneration (graf function)

The `graf()` function interpolates values across age ranges:

```cpp
int graf(const TBeing* tb, int age, int p0, int p1, int p2, int p3, int p4, int p5, int p6) {
    // age disabled - treat everyone as 35 year old
    if (!tb->hasQuestBit(TOG_REAL_AGING)) {
        return (int)(p2 + (((35 - 30) * (p3 - p2)) / 15));  // 30..44 range
    }

    if (age < 15)           return p0;
    else if (age <= 29)     return interpolate(p1, p2, 15, 29);
    else if (age <= 44)     return interpolate(p2, p3, 30, 44);
    else if (age <= 59)     return interpolate(p3, p4, 45, 59);
    else if (age <= 79)     return interpolate(p4, p5, 60, 79);
    else                    return p6;
}
```

**HP regeneration parameters:** `graf(age, 2, 4, 5, 9, 4, 3, 2)`
- Peak regeneration at ages 30-44 (value 9)
- Youth and elderly get reduced regeneration

**Source:** `code/code/misc/limits.cc:46-65`

#### Movement Costs

Older characters require more movement points:

```cpp
// In rawMove()
if (hasQuestBit(TOG_REAL_AGING)) {
    if (age()->year - getBaseAge() >= 35)
        need_movement += (age()->year - getBaseAge() - 30) / 5;

    if (age()->year - getBaseAge() >= 50) {
        need_movement += (age()->year - getBaseAge() - 50) / 10;
        if (((age()->year - getBaseAge() - 50) % 10) >= ::number(1, 10))
            need_movement++;
    }
}
```

**Movement penalty:**
- Age 35+: +1 move per 5 years over 30
- Age 50+: Additional +1 move per 10 years over 50

**Source:** `code/code/misc/movement.cc:666-827`

#### Maximum Move Pool

Movement pool increases with age (when aging enabled):

```cpp
int TPerson::getMaxMove() const {
    if (!hasQuestBit(TOG_REAL_AGING)) {
        return 100 + 15 + GetTotLevel() + plotStat(STAT_CURRENT, STAT_CON, 3, 18, 13);
    } else {
        return 100 + age()->year - getBaseAge() + 15 + GetTotLevel() +
               plotStat(STAT_CURRENT, STAT_CON, 3, 18, 13);
    }
}
```

**Source:** `code/code/misc/limits.cc:174-183`

#### Ranged Combat

Very old characters suffer range penalties:

```cpp
if (ch->hasQuestBit(TOG_REAL_AGING)) {
    if ((ch->age()->year - ch->getBaseAge() + 17) > 80)
        max_range -= 3;
}
```

**Source:** `code/code/misc/range.cc:846-848`

### Vampire Exception

Vampires are immune to all age effects:

```cpp
// In getStat() for STAT_NATURAL
if (isVampire() && (whichStat == STAT_STR || whichStat == STAT_SPE ||
                    whichStat == STAT_CHA))
    amount += 25;  // Flat bonus instead of age penalty

// Age modifier check
if (!isVampire())
    amount += age_mod_for_stat(this, my_age, whichStat);
```

**Source:** `code/code/misc/stats.cc` (getStat implementation)

## Common Gotchas

### Hunger/Thirst

1. **Condition -1 is immunity, not "very hungry"** - Always check for -1 before comparing to 0.

2. **Racial modifiers can make food ineffective** - A `TALENT_FISHEATER` race gets only 5% nutrition from non-fish food.

3. **Body mass affects consumption** - Heavier characters need proportionally more food/drink.

4. **Immortals are automatically immune** - `setCond()` forces -1 for anyone above `MAX_MORT`.

5. **Auto-eat doesn't work while fighting** - Check `task` state before assuming auto-eat will save a player.

### Age

1. **Age effects are opt-in** - Most players won't have `TOG_REAL_AGING` set.

2. **"Human equivalent" age** - Age modifiers use normalized human years, not actual character years.

3. **graf() returns constant for non-aging characters** - Always returns the age 35 value.

4. **Physical stats have inverted age curves** - Young = bonus, old = penalty (opposite of mental stats).

5. **Vampires bypass all age mechanics** - Check `isVampire()` when debugging age-related issues.

## Key Source Files

| File | Contents |
|------|----------|
| `code/code/misc/enum.h:413-422` | `condTypeT` enum definition |
| `code/code/misc/limits.cc:1250-1449` | `gainCondition()` implementation |
| `code/code/misc/periodic.cc:871-913` | Nutrition/weight calculation |
| `code/code/misc/periodic.cc:1744-1774` | Starvation damage |
| `code/code/obj/obj_food.cc:929-949` | `foodNDrink()` drain function |
| `code/code/obj/obj_food.cc:1014-1026` | `getCond()`/`setCond()` accessors |
| `code/code/misc/stats.cc:118-600` | `age_mod_for_stat()` |
| `code/code/misc/limits.cc:46-65` | `graf()` age interpolation |
| `code/code/sys/gametime.cc:153-175` | MUD time calculation |
| `code/code/misc/race.cc:885-886` | Racial age generation |
| `code/code/misc/charfile.h:39-52` | Age data storage |

## Related Documentation

- [Stats and Attributes](stats-attributes.md) - How age modifiers affect stats
- [Race System](race-system.md) - Racial food modifiers and age ranges
- [Scheduler and Pulses](scheduler-pulses.md) - Timing of nutrition/age updates
- [Death and Dying](death-dying.md) - Starvation death handling
