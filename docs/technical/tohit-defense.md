---
title: To-Hit and Defense System
description: Combat resolution system that determines whether attacks succeed or fail using two parallel mechanisms - regular hits for melee combat and special attacks for combat abilities with guaranteed hit/miss zones.
keywords:
  - hits
  - attackRound
  - defendRound
  - specialAttack
  - specAttackMod
  - getStatMod
  - getDexReaction
  - getAgiReaction
  - combat modes
  - ATTACK_BERSERK
  - ATTACK_DEFENSE
  - plotStat
  - SKILL_BLINDFIGHTING
  - position modifiers
category: Critical Systems
related:
  - combat-formulas.md
  - combat-rounds.md
  - stats-attributes.md
  - position-stance.md
  - equipment-wear.md
last_updated: 2026-01-29
source_files:
  - code/code/misc/combat.cc
  - code/code/misc/offense.cc
  - code/code/misc/defense.cc
  - code/code/misc/stats.cc
  - code/code/cmd/cmd_low.cc
---

This document describes SneezyMUD's combat resolution system, which determines whether attacks succeed or fail. The system uses two parallel resolution mechanisms: regular hits (melee combat) and special attacks (combat abilities).

**Misusing these calculations causes balance issues and frustration.** Common errors: forgetting guaranteed hit/miss zones, not accounting for combat mode modifiers, ignoring position penalties, applying stat modifiers incorrectly.

## Overview

Combat resolution flows through three layers:

1. **Attack/Defense Calculation** - `attackRound()` and `defendRound()` compute modifiers
2. **Hit Resolution** - `hits()` or `specialAttack()` determines success
3. **Result Application** - Successful hits proceed to damage calculation

**Key files:**
- `code/code/misc/combat.cc` - Core resolution functions
- `code/code/misc/offense.cc` - Attack-related calculations
- `code/code/misc/defense.cc` - Defense-related calculations

## Regular Hit Resolution

### hits() Function

The primary melee hit function, called from `perform_violence()` during combat rounds.

```cpp
int TBeing::hits(TBeing* v, int mod);
```

**Source:** `code/code/misc/combat.cc:3207-3256`

### Hit Probability Formula

```cpp
int factor = 600 + (9 * mod / 5);
factor = min(max(factor, 0), 1000);   // Clamp to 0-1000
int roll = ::number(0, 999);          // Random 0-999

if (roll < 50)
    return GUARANTEED_SUCCESS;        // 5% guaranteed hit
else if (roll >= 950)
    return GUARANTEED_FAILURE;        // 5% guaranteed miss
else if (roll < factor)
    return TRUE;                      // Hit succeeds
else
    return FALSE;                     // Miss
```

### Mod-to-Hit-Rate Mapping

The `mod` parameter (attackRound() - defendRound()) converts to hit probability:

| Mod Value | Factor | Hit Rate | Interpretation |
|-----------|--------|----------|----------------|
| -333 | 0 | 0% | Impossible to hit |
| -100 | 420 | 42% | Defender advantage |
| 0 | 600 | 60% | Even match |
| +100 | 780 | 78% | Attacker advantage |
| +222 | 1000 | 100% | Automatic hit |

**Key insight:** Each point of mod changes hit rate by approximately 0.18% (9/5000).

### Guaranteed Hit Zones

**Automatic Success (bypasses probability):**
1. First 5% of rolls (0-49)
2. `AFF_FOCUS_ATTACK` on attacker
3. Target not awake (`getPosition() < POSITION_RESTING`)
4. Target has `AFFECT_DUMMY` with level 60

**Automatic Failure (bypasses probability):**
1. Last 5% of rolls (950-999)

These zones prevent extreme stat differences from creating 100% or 0% hit rates.

## attackRound() - Attack Modifier Calculation

Computes the attacker's bonus for hit probability.

**Source:** `code/code/misc/combat.cc:2596-2762`

### Base Calculation

```cpp
bonus = level * 50/3;
```

| Level | Base Bonus |
|-------|------------|
| 0 | 0 |
| 30 | 500 |
| 60 | 1000 |
| 70 | 1167 |

### Doubling Level (my_lev)

Used for relative bonus scaling:

```cpp
my_lev = max(10, (int)(16.67 * get_doubling_level(GetMaxLevel())));
```

The doubling level represents how many additional levels make a creature twice as difficult. This scales combat mode and skill bonuses appropriately.

### Combat Mode Modifiers

**ATTACK_DEFENSE:**
```cpp
bonus -= my_lev / 2;
if (doesKnowSkill(SKILL_ADVANCED_DEFENSE))
    bonus += getSkillValue(SKILL_ADVANCED_DEFENSE) / 3;
```

**ATTACK_OFFENSE / ATTACK_BERSERK:**
```cpp
bonus += my_lev / 4;
```

**ATTACK_NORMAL:**
- No modifier

### Skill Bonuses

**SKILL_CHIVALRY** (mounted only):
```cpp
amt = 74 * max(10, getSkillValue(SKILL_CHIVALRY)) / 100;
bonus += amt;  // Range: 0-74
```

**SKILL_CINTAI:**
```cpp
bonus += (getSkillValue(SKILL_CINTAI) / 20) * 3;  // Range: 0-15
```

**SKILL_OFFENSE** (PC) or `DISC_COMBAT` (mob):
```cpp
amt = my_lev * max(10, getSkillValue(SKILL_OFFENSE)) / 100;
bonus += amt;
```

This replaces the theoretical baseline - higher skill = closer to full bonus.

**SKILL_ADVANCED_OFFENSE:**
```cpp
bonus += (getSkillValue(SKILL_ADVANCED_OFFENSE) / 4) * 3;  // Range: 0-75
```

### DEX Modifier

```cpp
bonus += (int)(335 * getStatMod(STAT_DEX) - 335);
```

**Range:** -67 to +84 points

| DEX Level | Bonus | Hit Rate Effect |
|-----------|-------|-----------------|
| 20 (very low) | -67 | -12% |
| 105 (average) | 0 | Baseline |
| 190 (high) | +84 | +15% |

**Source:** See [Stats Attributes](stats-attributes.md) for `getStatMod()` formula.

### Equipment Hitroll

```cpp
bonus += 5 * (getHitroll() + getSpellHitroll()) / 3;
```

Each +1 hitroll = +5/3 ≈ 1.67 bonus points ≈ 0.3% hit rate increase.

### Blind Fighting Penalty

```cpp
if (target && !canSee(target)) {
    amt = my_lev;
    if (doesKnowSkill(SKILL_BLINDFIGHTING)) {
        amt *= (100 - getSkillValue(SKILL_BLINDFIGHTING)) / 100;
    }
    bonus -= amt + 1;
}
```

**Base penalty:** -my_lev - 1

**With SKILL_BLINDFIGHTING at 100:** penalty → 0

### Spell Casting Penalty

```cpp
if (spelltask)
    bonus -= 2 * my_lev / 3;
```

Characters casting spells during combat suffer reduced attack accuracy.

### Position Modifiers

| Position | Modifier |
|----------|----------|
| DEAD/MORTALLYW/INCAP/STUNNED/SLEEPING | `-bonus` (negates all) |
| RESTING | `-(my_lev/3 + 1)` |
| SITTING | `-(my_lev/4 + 1)` |
| CRAWLING | 0 |
| STANDING/ENGAGED/FIGHTING | 0 |
| MOUNTED | `+(my_lev/4 + 1)` |
| FLYING | `+(my_lev/3 + 1)` |

**Ground Fighting Mitigation:**

```cpp
if (position < POSITION_STANDING && awake() && val < 0) {
    if (doesKnowSkill(SKILL_GROUNDFIGHTING)) {
        val = val * (100 - getSkillValue(SKILL_GROUNDFIGHTING)) / 100;
        val = min(val, -1);  // Maintain minimum penalty
    }
}
```

Characters with `SKILL_GROUNDFIGHTING` reduce penalties when fighting from sitting/resting positions.

## defendRound() - Defense Modifier Calculation

Computes the defender's bonus for avoiding hits.

**Source:** `code/code/misc/combat.cc:2764-2964`

### Base AC Calculation

**PCs:**
```cpp
armor = 1000 - getArmor();
bonus = max((armor - 500), 0) * 2/3;
```

**Mobs:**
```cpp
armor = 1000 - getArmor();
bonus = max((armor - 400), 0) * 5/6;
```

| AC Value | PC Bonus | Mob Bonus |
|----------|----------|-----------|
| -10 | 0 | 0 |
| 0 | 333 | 833 |
| +10 | 667 | 1667 |

**PC AC Cap (prevents gear powerleveling):**
```cpp
bonus = min(bonus, (GetMaxLevel() * 1000 / 60) + my_lev);
```

### Combat Mode Modifiers

**ATTACK_DEFENSE:**
```cpp
bonus += my_lev / 4;
if (doesKnowSkill(SKILL_ADVANCED_DEFENSE))
    bonus += getSkillValue(SKILL_ADVANCED_DEFENSE) / 10;
```

**ATTACK_OFFENSE:**
```cpp
bonus -= my_lev / 4;
```

**ATTACK_BERSERK:**
```cpp
bonus -= my_lev / 4;

int factor = 100;
if (hasClass(CLASS_WARRIOR) && doesKnowSkill(SKILL_BERSERK))
    factor = 100 - getSkillValue(SKILL_BERSERK);
if (doesKnowSkill(SKILL_ADVANCED_BERSERKING))
    factor *= 2;

bonus -= (8 * my_lev * factor) / 100;
```

Berserking characters suffer extreme defense penalties that scale with their skill level.

### Skill Bonuses

**SKILL_CHIVALRY** (mounted only):
```cpp
amt = 159 * max(10, getSkillValue(SKILL_CHIVALRY)) / 100;
bonus += amt;  // Range: 0-159
```

**SKILL_DEFENSE:**
```cpp
amt = my_lev * min(100, getSkillValue(SKILL_DEFENSE)) / 100;
bonus += amt;
```

**SKILL_OOMLAT** (PC armor boost):
```cpp
armor += (armor * getSkillValue(SKILL_OOMLAT) / 250.0);
bonus = max((armor - 500), 0) * 2/3;
```

### AGI Modifier

```cpp
if (!spelltask)
    bonus += (int)(335 * getStatMod(STAT_AGI) - 335);
```

**Range:** -67 to +84 points (identical scaling to DEX on attack)

**Key distinction:** DEX affects attack, AGI affects defense.

### Spell Effect Bonuses

**SPELL_AURA_GUARDIAN:**
```cpp
bonus += 40;
```

### Blind Fighting Penalty (Defender)

```cpp
if (attacker && !canSee(attacker)) {
    amt = my_lev;
    if (doesKnowSkill(SKILL_BLINDFIGHTING)) {
        amt *= (100 - getSkillValue(SKILL_BLINDFIGHTING)) / 100;
    }
    bonus -= amt + 1;
}
```

Works identically to attacker's blind penalty - characters unable to see opponents suffer reduced defense.

### Position Modifiers

Identical to attackRound() modifiers - see above.

## Special Attack Resolution

Special attacks (bash, trip, kick, grapple, etc.) use a different resolution system that incorporates stats more directly.

### specialAttack() Function

**Primary signature:**
```cpp
int TBeing::specialAttack(TBeing* target, spellNumT skill, int situationalModifier,
                         statTypeT primaryOffense, statTypeT secondaryOffense,
                         statTypeT primaryDefense, statTypeT secondaryDefense,
                         bool partialSuccessAllowed);
```

**Source:** `code/code/misc/combat.cc:3103-3204`

### Default Stat Mapping

When not specified:
- Primary Offense: `STAT_FOC`
- Secondary Offense: `STAT_KAR`
- Primary Defense: `STAT_AGI`
- Secondary Defense: `STAT_PER`

### Resolution Formula

```cpp
// Step 1: Apply situational modifiers
situationalModifier += specAttackMod(target);
situationalModifier = clamp(situationalModifier, -20, 20);

// Step 2: Level difference adjustment
int levelDiff = attackerLevel - defenderLevel;
situationalModifier += (isPc() && levelDiff > 0)
    ? levelDiff
    : (levelDiff / 5);

// Step 3: Roll and apply stat modifiers
double roll = ::number(1, 100) - situationalModifier;

roll = roll
    * getStatMod(primaryOffenseStat)
    * plotStat(STAT_CURRENT, secondaryOffenseStat, 0.92, 1.08, 1.0)
    / target->getStatMod(primaryDefenseStat)
    / target->plotStat(STAT_CURRENT, secondaryDefenseStat, 0.92, 1.08, 1.0);

// Step 4: Determine result
if (roll <= 5)
    return GUARANTEED_SUCCESS;
else if (roll > 95)
    return GUARANTEED_FAILURE;
else if (roll < 50)
    return COMPLETE_SUCCESS;
else if (partialSuccessAllowed && roll < 80)
    return PARTIAL_SUCCESS;
else
    return FAILURE;
```

### Return Values

| Constant | Value | Meaning |
|----------|-------|---------|
| `GUARANTEED_SUCCESS` | 2 | 5% guaranteed success |
| `COMPLETE_SUCCESS` | 1 | Full success (roll < 50) |
| `PARTIAL_SUCCESS` | -2 | Partial success (roll 50-79, if allowed) |
| `FAILURE` | 0 | Failed attempt (roll >= 80 or >= 50) |
| `GUARANTEED_FAILURE` | -1 | 5% guaranteed failure |

### Success Thresholds

- **Guaranteed success:** First 5% of adjusted rolls (≤ 5)
- **Complete success:** 45% zone (5-50)
- **Partial success:** 30% zone (50-80, if enabled)
- **Guaranteed failure:** Last 5% of adjusted rolls (> 95)
- **Failure:** Remaining rolls (80-95 or 50-95)

### SKILL_INEVITABILITY Integration

```cpp
if (affectedBySpell(SKILL_INEVITABILITY)) {
    remAffect(SKILL_INEVITABILITY, SILENT_YES);
    return GUARANTEED_SUCCESS;
}
```

Characters with this affect automatically succeed on their next special attack.

## specAttackMod() - Situational Modifiers

Computes bonuses/penalties based on combat situation.

**Source:** `code/code/misc/combat.cc:2971-3100`

### Attacker Position

| Position | Modifier |
|----------|----------|
| RESTING | -5 |
| SITTING | -3 |
| CRAWLING | -1 |
| STANDING/ENGAGED/FIGHTING | 0 |
| MOUNTED | +2 |
| FLYING | +3 |

### Thief Stealth (out of combat)

```cpp
if (hasClass(CLASS_THIEF) && !fight()) {
    if (isAffected(AFF_SNEAK)) mod += 5;
    if (isAffected(AFF_HIDE))  mod += 5;
}
```

**Critical:** Stealth bonuses disappear once combat starts.

### Territorial and Background Bonuses

```cpp
if (homeTurf())
    mod += 3;

if (backgroundBonus())
    mod += 3;
```

Home territory and appropriate background provide situational advantages.

### Attacker Spell Effects

| Spell Effect | Modifier |
|--------------|----------|
| `AFF_WEB` | -4 |
| `SPELL_STUPIDITY` | -1 |
| `SPELL_CURSE` | -2 |
| `SPELL_BLESS` | +1 |
| `SPELL_AURA_MIGHT` | +3 |

### Blind Fighting Penalty (Attacker)

```cpp
if (target && !canSee(target)) {
    blindPenalty = 6;
    if (doesKnowSkill(SKILL_BLINDFIGHTING))
        blindPenalty *= (100 - getSkillValue(SKILL_BLINDFIGHTING)) / 100;
    mod -= blindPenalty;
}
```

**Base penalty:** -6

**With SKILL_BLINDFIGHTING at 100:** penalty → 0

### Defender Position (Inverted)

When the target is in an unfavorable position, the attacker gains bonuses:

| Target Position | Attacker Bonus |
|-----------------|----------------|
| RESTING | +5 |
| SITTING | +3 |
| CRAWLING | +1 |
| STANDING/ENGAGED/FIGHTING | 0 |
| MOUNTED | -2 |
| FLYING | -4 |

### Defender Spell Effects (Inverted)

| Target Spell Effect | Attacker Bonus |
|---------------------|----------------|
| `AFF_WEB` | +4 |
| `SPELL_STUPIDITY` | +1 |
| `SPELL_CURSE` | +2 |
| `SPELL_SANCTUARY` | -3 |
| `SPELL_CRUSADE` | -3 |
| `SPELL_AURA_GUARDIAN` | -3 |

### Target Blind Fighting (Inverted)

```cpp
if (!target->canSee(this)) {
    blindPenalty = 6;
    if (target->doesKnowSkill(SKILL_BLINDFIGHTING))
        blindPenalty *= (100 - target->getSkillValue(SKILL_BLINDFIGHTING)) / 100;
    mod += blindPenalty;  // Inverted - helps attacker
}
```

If the target cannot see the attacker, the attacker gains bonuses.

### Surprise Attack Detection

```cpp
if (skill == SKILL_BACKSTAB || skill == SKILL_CUDGEL ||
    skill == SKILL_THROATSLIT || skill == SKILL_RANGED_PROF) {
    if (target->isWary())
        situationalModifier -= 10;
    else if (!target->affectedBySpell(SKILL_SUBTERFUGE))
        target->makeWary();
}
```

**Wary mechanic:** After surviving an assassination attempt, targets become wary for a period, imposing -10 penalty on subsequent surprise attacks.

## Combat Mode Summary

**ATTACK_NORMAL** (Baseline):
- Attack: 0
- Defense: 0

**ATTACK_DEFENSE**:
- Attack: -my_lev/2 + SKILL_ADVANCED_DEFENSE/3
- Defense: +my_lev/4 + SKILL_ADVANCED_DEFENSE/10
- Goal: Reduce offense for better defense

**ATTACK_OFFENSE**:
- Attack: +my_lev/4
- Defense: -my_lev/4
- Goal: Increase attacks at defense cost

**ATTACK_BERSERK** (Players only):
- Attack: +my_lev/4 (same as offense)
- Defense: -my_lev/4 - (8 * my_lev * factor / 100)
  - factor = 100 (no SKILL_BERSERK)
  - factor = 100 - SKILL_BERSERK value
  - factor *= 2 if SKILL_ADVANCED_BERSERKING known
- Special: Cannot flee, multiple bonuses elsewhere
- Goal: Maximize damage at extreme defense cost

## Equipment Effects

### Armor Class (AC)

AC reduces hit probability through `defendRound()`:

```cpp
armor = 1000 - getArmor();
// Lower AC number = better armor = higher bonus
```

**Sources of AC:**
- Base racial AC
- Equipment with `APPLY_ARMOR` affects
- `SKILL_IRON_FLESH` (monks, if not wearing armor)
- `SPELL_AURA_GUARDIAN` (+40)

### Hitroll

Increases hit probability through `attackRound()`:

```cpp
bonus += 5 * (getHitroll() + getSpellHitroll()) / 3;
```

**Sources of hitroll:**
- Equipment with `APPLY_HITROLL` affects
- Temporary spell effects

Each +1 hitroll ≈ 1.67 bonus points ≈ 0.3% hit rate increase.

### Spell Effects on Combat

Beneficial (attacker):
- `SPELL_BLESS`: +1 special attack modifier
- `SPELL_AURA_MIGHT`: +3 special attack modifier

Beneficial (defender):
- `SPELL_SANCTUARY`: -3 special attack modifier (for enemies)
- `SPELL_CRUSADE`: -3 special attack modifier (for enemies)
- `SPELL_AURA_GUARDIAN`: -3 special attack modifier (for enemies), +40 defense

Detrimental:
- `AFF_WEB`: -4 special attack modifier, +4 for enemies
- `SPELL_STUPIDITY`: -1 special attack modifier, +1 for enemies
- `SPELL_CURSE`: -2 special attack modifier, +2 for enemies

## Position System Deep Dive

### Position Hierarchy

```
DEAD (0) < MORTALLYW (1) < INCAP (2) < STUNNED (3) < SLEEPING (4)
< RESTING (5) < SITTING (6) < ENGAGED (7) < FIGHTING (8)
< CRAWLING (9) < STANDING (10) < MOUNTED (11) < FLYING (12)
```

**Source:** `code/code/misc/enum.h`

### Combat Effectiveness by Position

| Position | Attack | Defense | Special Attacks | Notes |
|----------|--------|---------|-----------------|-------|
| DEAD | Negated | Negated | N/A | Cannot act |
| INCAP/STUNNED | Negated | Negated | N/A | Cannot act |
| SLEEPING | Negated | Negated | N/A | Vulnerable |
| RESTING | -(my_lev/3+1) | -(my_lev/3+1) | -5 | Heavy penalty |
| SITTING | -(my_lev/4+1) | -(my_lev/4+1) | -3 | Moderate penalty |
| CRAWLING | 0 | 0 | -1 | Minor penalty |
| STANDING | 0 | 0 | 0 | Baseline |
| MOUNTED | +(my_lev/4+1) | +(my_lev/4+1) | +2 | Requires CHIVALRY for full effect |
| FLYING | +(my_lev/3+1) | +(my_lev/3+1) | +3 | Best position |

### Ground Fighting Skill

Characters with `SKILL_GROUNDFIGHTING` reduce penalties when position < `POSITION_STANDING`:

```cpp
penalty = penalty * (100 - getSkillValue(SKILL_GROUNDFIGHTING)) / 100;
penalty = min(penalty, -1);  // Maintain minimum penalty
```

**Example:** Level 60 character sitting (penalty = -my_lev/4+1 ≈ -16):
- Without skill: -16 to both attack and defense
- With skill at 50: -8 to both
- With skill at 100: -1 to both (minimum maintained)

## Stat Modifier Effects

### DEX and AGI

Both stats use `getStatMod()` with identical scaling:

```cpp
double getStatMod(statTypeT stat) const {
    return ((plotStat(STAT_CURRENT, stat, 0.8, 1.25, 1.0) - 1) * multiplier) + 1;
}
```

**Range:** 0.8 to 1.25 for stats 5-205

**Combat bonus calculation:**
```cpp
bonus = (int)(335 * getStatMod(stat) - 335);
```

| Stat Value | getStatMod() | Bonus | Hit Rate Effect |
|------------|--------------|-------|-----------------|
| 5 | 0.8 | -67 | -12% |
| 105 | 1.0 | 0 | 0% |
| 205 | 1.25 | +84 | +15% |

**Key distinction:**
- **DEX** affects `attackRound()` (accuracy)
- **AGI** affects `defendRound()` (avoidance)

### FOC, KAR, PER in Special Attacks

Special attacks use multiple stats:

**Default mapping:**
- Primary Offense: `STAT_FOC` (via `getStatMod()`)
- Secondary Offense: `STAT_KAR` (via `plotStat()`)
- Primary Defense: `STAT_AGI` (via `getStatMod()`)
- Secondary Defense: `STAT_PER` (via `plotStat()`)

**Primary stats:** 0.8-1.25 multiplier

**Secondary stats:** 0.92-1.08 multiplier (smaller variance)

Individual skills may override these defaults - check the specific skill implementation.

## Edge Cases and Special Conditions

### Guaranteed Hit Conditions

1. **Random roll < 50** (5% zone in `hits()`)
2. **AFF_FOCUS_ATTACK** on attacker
3. **Target position < POSITION_RESTING** (sleeping, incap, stunned)
4. **AFFECT_DUMMY** with level 60 on target
5. **Special attack adjusted roll <= 5** (5% zone)

### Guaranteed Miss Conditions

1. **Random roll >= 950** (5% zone in `hits()`)
2. **Special attack adjusted roll > 95** (5% zone)

### Vision and Blindness

**Attacker cannot see target:**
- Regular attacks: -my_lev - 1 penalty (via `attackRound()`)
- Special attacks: -6 penalty (via `specAttackMod()`)
- Both mitigated by `SKILL_BLINDFIGHTING`

**Defender cannot see attacker:**
- Regular attacks: -my_lev - 1 penalty to defense
- Special attacks: +6 bonus to attacker
- Both mitigated by defender's `SKILL_BLINDFIGHTING`

**Bidirectional blindness:** Both penalties apply simultaneously if neither can see the other.

### Wary State (Assassination Protection)

After surviving certain assassination attempts (backstab, cudgel, throatslit, ranged snipe):

```cpp
if (!target->affectedBySpell(SKILL_SUBTERFUGE))
    target->makeWary();
```

Subsequent surprise attacks against wary targets suffer -10 situational modifier.

**Duration:** Wary state persists for a fixed time period (implementation in `makeWary()`).

### Level Difference Effects

**Special attacks favor PCs over mobs:**

```cpp
int levelDiff = attackerLevel - defenderLevel;
situationalModifier += (isPc() && levelDiff > 0)
    ? levelDiff
    : (levelDiff / 5);
```

**PC attacking higher-level mob:** Full penalty (-1 per level difference)

**PC attacking lower-level mob:** Full bonus (+1 per level difference)

**Mob attacking PC:** 1/5 penalty or bonus

This asymmetry helps PCs fight above their level.

### Spell Casting in Combat

Characters casting spells during combat suffer penalties:

```cpp
if (spelltask)
    attackBonus -= 2 * my_lev / 3;
```

```cpp
if (!spelltask)
    defenseBonus += (int)(335 * getStatMod(STAT_AGI) - 335);
```

**Effect:** Casters lose AGI defense bonus and suffer attack penalty.

### Thief Stealth Limitations

```cpp
if (hasClass(CLASS_THIEF) && !fight()) {
    // Bonuses apply
}
```

**Critical:** `SNEAK` and `HIDE` bonuses (+5 each to special attacks) only work **before** combat starts. Once `fight()` returns true, bonuses disappear.

## Common Balance Gotchas

### 1. Combat Mode Double-Dipping

Combat modes affect BOTH attack and defense:

```cpp
// ATTACK_OFFENSE
attackBonus += my_lev / 4;
defenseBonus -= my_lev / 4;
```

This creates a **swing** in relative advantage:
- Offense vs Defense: 2 * (my_lev/4) = my_lev/2 difference
- Both are worse than Normal vs Normal at same level

### 2. Position Multiplier Effect

When attacker is mounted and defender is resting:

**Attacker gains:**
- `+(my_lev/4 + 1)` to attack
- `+(my_lev/4 + 1)` to defense
- `+2` to special attacks

**Defender loses:**
- `-(my_lev/3 + 1)` to attack
- `-(my_lev/3 + 1)` to defense
- `-5` to special attacks

**Total swing in regular hits:**
```
my_lev/4 + 1 - (-(my_lev/3 + 1)) = my_lev/4 + my_lev/3 + 2
= 7*my_lev/12 + 2
```

At level 60, this is ≈ +37 points ≈ +7% hit rate swing.

### 3. Stat Scaling Non-Linearity

Due to `plotStat()` power law (exponent 1.4):

**Linear stat increase → Non-linear combat advantage**

| DEX Increase | Bonus Increase | Hit Rate Increase |
|--------------|----------------|-------------------|
| 105 → 155 (+50) | 0 → +42 | 0% → +7.6% |
| 155 → 205 (+50) | +42 → +84 | +7.6% → +15% |

**High stats have higher marginal returns** - favor maxing one stat over spreading points.

### 4. Guaranteed Zone Clipping

The 5% guaranteed hit/miss zones clip extreme stat advantages:

**Without zones:** 100% hit at mod +222, 0% hit at mod -333

**With zones:** 95% hit at mod +222, 5% hit at mod -333

This prevents perfect invulnerability or perfect accuracy, maintaining gameplay variance.

### 5. Berserk Defense Catastrophe

Berserking warriors with high skill:

```cpp
factor = (100 - getSkillValue(SKILL_BERSERK));
if (doesKnowSkill(SKILL_ADVANCED_BERSERKING))
    factor *= 2;
defenseBonus -= (8 * my_lev * factor) / 100;
```

**Example (Level 60, Berserk at 100, Advanced Berserking):**
```
factor = (100 - 100) = 0
factor *= 2 = 0
penalty = 0
```

**But without Advanced Berserking:**
```
factor = 0
penalty = 0
```

**And at Berserk skill 50, with Advanced Berserking:**
```
factor = 50
factor *= 2 = 100
penalty = (8 * 60 * 100) / 100 = 480
```

**Key insight:** Maxing `SKILL_BERSERK` eliminates the massive defense penalty, making berserking viable for experienced warriors.

## Performance Considerations

### hits() Complexity

- O(1) - single random roll and arithmetic
- Called multiple times per combat round
- No caching - recalculated each hit

### attackRound() / defendRound() Complexity

- O(1) - arithmetic and stat lookups
- Cached within a single hit attempt
- Not cached across multiple hits

### Special Attack Complexity

- O(1) - includes stat modifier calculations
- Slightly more expensive than `hits()` due to `plotStat()` calls
- Used less frequently (only for special abilities)

### Optimization Notes

Combat calculations are called thousands of times during busy combat rounds. Key optimizations:

1. **Stat modifiers** - `getStatMod()` is pre-calculated in `curStats`
2. **Equipment bonuses** - Cached in `getHitroll()` / `getArmor()`
3. **Random rolls** - Single call per resolution, not per modifier

## Key Constants

| Constant | Value | Purpose |
|----------|-------|---------|
| `BASE_HIT_FACTOR` | 600 | Baseline 60% hit rate |
| `MOD_TO_FACTOR_MULTIPLIER` | 9/5 | Mod → factor conversion |
| `GUARANTEED_HIT_THRESHOLD` | 50 | First 5% of rolls |
| `GUARANTEED_MISS_THRESHOLD` | 950 | Last 5% of rolls |
| `DEX_AGI_MULTIPLIER` | 335 | Stat → combat bonus scaling |
| `HITROLL_MULTIPLIER` | 5/3 | Equipment hitroll effectiveness |
| `PC_AC_DENOMINATOR` | 2/3 | PC armor effectiveness |
| `MOB_AC_DENOMINATOR` | 5/6 | Mob armor effectiveness |

## Related Documentation

- [Combat Formulas](combat-formulas.md) - Foundation math for hit/damage/crit
- [Combat Rounds](combat-rounds.md) - Timing and attack distribution
- [Stats Attributes](stats-attributes.md) - Stat system and `plotStat()` formula
- [Position Stance](position-stance.md) - Complete position system documentation
- [Equipment Wear](equipment-wear.md) - How equipment affects combat stats

## Key Source Files

| File | Lines | Purpose |
|------|-------|---------|
| `code/code/misc/combat.cc` | 3207-3256 | `hits()` function |
| `code/code/misc/combat.cc` | 2596-2762 | `attackRound()` calculation |
| `code/code/misc/combat.cc` | 2764-2964 | `defendRound()` calculation |
| `code/code/misc/combat.cc` | 3103-3204 | `specialAttack()` resolution |
| `code/code/misc/combat.cc` | 2971-3100 | `specAttackMod()` modifiers |
| `code/code/misc/stats.cc` | 1055-1058 | `getStatMod()` implementation |
| `code/code/cmd/cmd_low.cc` | 53-82 | `get_doubling_level()` |
