---
title: Combat Formulas
description: Mathematical formulas for hit probability, damage calculation, critical hits, attack counts, and defensive mechanics with stat scaling via plotStat.
keywords: [attackRound, defendRound, hits, getWeaponDam, getSkillDam, reconcileDamage, critSuccessChance, blowCount, plotStat, getStatMod, specialAttack, genericDam, classAmt, dexModifier, armorClass]
category: Important Systems

last_updated: 2026-01-29
source_files: [code/code/misc/combat.cc, code/code/misc/skill_dam.cc, code/code/misc/crit_combat.cc, code/code/misc/offense.cc, code/code/misc/being.h, code/code/misc/stats.cc, code/code/misc/damage.cc]
related:
  - damage-pipeline.md
  - memory-safety.md
  - spell-skill-framework.md
  - position-stance.md
  - character-foundation.md
---

# Combat Formulas

This document describes the core mathematical formulas underlying SneezyMUD's combat system: hit probability, damage calculation, critical hits, attack counts, and defensive mechanics.

## Hit Success System

### attackRound() - Attack Modifier

**File:** `code/code/misc/combat.cc`

Calculates the attacker's bonus for hit probability:

```
attackBonus = (level × 50/3) + combatModeModifiers + skillBonuses + dexModifier
```

| Component | Formula | Range |
|-----------|---------|-------|
| Base Level | `level × 50/3` | 0-1167 (level 70) |
| Combat Mode (Defense) | `−(level/2)` | Penalty |
| Combat Mode (Offense/Berserk) | `+(level/4)` | Bonus |
| Chivalry (Mounted Deikhan) | `74 × skillValue / 100` | 0-74 |
| Cintai | `skillValue × 3 / 20` | 0-15 |
| Advanced Offense | `skillValue × 3 / 4` | 0-75 |
| Dexterity | `335 × getStatMod(STAT_DEX) − 335` | −67 to +84 |

**Discipline Scaling:** Offense/Combat discipline learning replaces a theoretical baseline. PCs use `SKILL_OFFENSE`; MOBs use `DISC_COMBAT` learnedness.

### defendRound() - Defense Modifier

**File:** `code/code/misc/combat.cc`

Calculates the defender's bonus:

```
PC:  defendBonus = (10 − AC) × 55 + combatModeBonus + skillBonuses + dexModifier
MOB: defendBonus = (10 − AC) × 55 + (level × 50/3) + skillBonuses + dexModifier
```

| AC Value | Defense Contribution |
|----------|---------------------|
| −10 | +1100 |
| 0 | +550 |
| +10 | 0 |
| +20 | −550 |

**Position Penalties:**

| Position | Multiplier |
|----------|------------|
| Standing/Fighting | 1.0× |
| Resting | 0.75× |
| Sitting/Crawling | 0.5× |

`SKILL_GROUNDFIGHTING` reduces these penalties proportionally to learning.

### hits() - Hit Probability

**File:** `code/code/misc/combat.cc`

Determines whether an attack connects:

```
mod = attackRound(target) − defendRound(target)
factor = clamp(600 + (9 × mod / 5), 0, 1000)
hit = (roll(1, 1000) ≤ factor)
```

| Modifier Difference | Hit Factor | Hit Probability |
|--------------------|------------|-----------------|
| −333 | 0 | 0% (guaranteed miss) |
| 0 | 600 | 60% (baseline) |
| +222 | 1000 | 100% (guaranteed hit) |

**Key insight:** Each point of modifier difference changes hit probability by approximately 0.18%.

### specialAttack() - Skill Success

**File:** `code/code/misc/combat.cc`

For special attacks (bash, trip, etc.):

```
roll = random(1, 100)
situationalMod = clamp(modifier, −20, +20)

if roll ≤ 50 − mod: SUCCESS
if roll < 80 − mod: PARTIAL_SUCCESS
else: FAILURE
```

## Damage Calculations

### getWeaponDam() - Melee Weapon Damage

**File:** `code/code/misc/combat.cc`

```
weaponDamage = (baseDam + rollDam + bonusDam) × strModifier × weaponLearning / 100
```

**Strength Modifier by Weapon Type:**

| Weapon Type | Strength Effect |
|-------------|-----------------|
| Blunt/Barehand | 100% strength bonus |
| Slash | 75% strength bonus |
| Pierce | 50% strength bonus |

**Weapon Learning:** `min(100, max(level × 2, skillValue))`

**Dual Wield:** Secondary hand deals `(30 + 30 × SKILL_DUAL_WIELD / 100)%` of primary damage (30-60%).

**Two-Handed Specialization:** Damage multiplied by `(100 + 50 × SKILL_2H_SPEC / 100) / 100` (100-150%).

### getSkillDam() - Spell/Skill Damage

**File:** `code/code/misc/skill_dam.cc`

Called via `genericDam()`:

```
baseDamage = classAmt × lagRounds × level
damage = baseDamage × diffModifier × statModifier × randomVariance
```

**classAmt Values (selected):**

| Spell/Skill | classAmt |
|-------------|----------|
| SPELL_GUST | 0.50 |
| SPELL_LIGHTNING_BOLT | 1.0 |
| SPELL_FIREBALL | 1.5-2.0 |
| SPELL_METEOR_STORM | 3.0-4.0 |
| SKILL_BASH | 0.5-0.75 |

**NPC Damage Reduction:** MOBs deal approximately 52% of PC damage for the same skill (`× 0.5195`).

**Area Effect Penalty:** Multi-target spells deal 75% damage.

**Random Variance:** `±(level / 2)` around the calculated base.

### reconcileDamage() Return Value

**File:** `code/code/misc/damage.cc`

**CRITICAL:** Returns `-1` when victim dies, NOT a DELETE flag.

```cpp
// CORRECT
if (reconcileDamage(victim, damage, DAMAGE_TYPE) == -1)
    return DELETE_VICT;

// WRONG - will never detect death
if (IS_SET_DELETE(rc, DELETE_VICT)) { ... }
```

## Critical Hit System

### critSuccessChance() - Critical Hit Probability

**File:** `code/code/misc/crit_combat.cc`

```
diceRoll = random(1, rollRange)
critChance = karmaBase + skillBonuses + gearBonuses

PC rollRange:  100,000
NPC rollRange: 1,000,000
```

**Karma Base:** `1000 × plotStat(STAT_KAR, 0.5, 2.0)` → range 500-2000

**Skill Bonuses:**

| Skill | Bonus |
|-------|-------|
| SKILL_CRIT_HIT | `+20 × skillValue` (max +2000) |
| SKILL_POWERMOVE | `+10 × skillValue` (max +1000) |

**Equipment:** `APPLY_CRIT_FREQUENCY` adds 0.05% per point.

**Effective Crit Rates:**
- Baseline NPC: ~0.001%
- Trained PC (max SKILL_CRIT_HIT): ~3%

### Crit Severity (1-100 scale)

```
severity = 10 + levelDifference + (100 − victimHPPercent) + skillBonuses
```

Capped at 100. Higher severity = more severe effects (limb damage, stuns, bleeds).

### critFailureChance() - Critical Failure

**File:** `code/code/misc/crit_combat.cc`

```
if random(1, 300) > (karmaScore + drunkPenalty) × 10:
    CRITICAL FAILURE
```

Low karma and intoxication increase failure chance. 20 different failure types including dropping weapon, hitting self, hitting ally, falling down.

## Attack Count System

### blowCount() - Attacks per Round

**File:** `code/code/misc/offense.cc`

**MOBs:**
```
attacks = min(12.0, getMult())
Split: 60% primary hand, 40% secondary
```

**PCs:**
```
primaryAttacks = blowCountSplitter() + specializationBonus + speedModifier
secondaryAttacks = blowCountSplitter() + speedModifier
```

**Modifiers:**

| Effect | Bonus |
|--------|-------|
| Berserk | +0.5 per hand |
| Advanced Berserking | +1.0 per hand |
| Haste | +0.5 per hand |
| Celerite | +0.5 per hand |
| Mounted | ×0.67 penalty |

**Typical Ranges:**
- MOBs: 1-12 attacks
- PCs: 1-8+ attacks

## Stat-to-Modifier Formulas

### plotStat() - Universal Conversion

**File:** `code/code/misc/being.h`

```
normStat = (stat − 5) / 200
scaled = normStat ^ 1.4
output = minValue + (maxValue − minValue) × scaled
```

The power of 1.4 creates a non-linear curve favoring high stats.

**Common Usage:**

| Purpose | minValue | maxValue | Stat |
|---------|----------|----------|------|
| Hit/Defense Modifier | 0.8 | 1.25 | DEX |
| Damage Modifier | 0.8 | 1.25 | STR |
| Crit Chance | 0.5 | 2.0 | KAR |
| Spell Learning | 0.1 | 10.0 | INT/WIS |

### getStatMod() - Standardized Modifier

**File:** `code/code/misc/stats.cc`

```
modifier = ((plotStat(stat, 0.8, 1.25, 1.0) − 1) × multiplier) + 1
```

| Stat Value | Output |
|------------|--------|
| 5 (minimum) | 0.8 (−20%) |
| 105 (baseline) | 1.0 (neutral) |
| 205 (maximum) | 1.25 (+25%) |

### Dexterity Combat Bonus

```
dexBonus = (int)(335 × getStatMod(STAT_DEX) − 335)
```

| DEX Level | Bonus | Hit Rate Effect |
|-----------|-------|-----------------|
| Low (stat ~20) | −67 | −12% hit rate |
| Average (stat ~105) | 0 | Baseline |
| High (stat ~190) | +84 | +15% hit rate |

## Quick Reference

| System | Key Formula | Typical Range |
|--------|-------------|---------------|
| Hit Probability | `(600 + 9×mod/5) / 1000` | 0-100% |
| Attack Bonus | `level×50/3 + skills + dex` | 0-1500+ |
| Defense Bonus | `(10−AC)×55 + level×50/3` | −500 to +1500 |
| Weapon Damage | `(base+roll)×str×learn/100` | 5-100+ |
| Spell Damage | `classAmt×lag×level×mods` | 5-500+ |
| Crit Chance | `karma×1000 + skills vs 1d100000` | 0.1-3%+ |
| Attack Count | `base + spec + haste + berserk` | 1-12 |
| Stat Modifier | `plotStat(stat, 0.8, 1.25)` | 0.8-1.25× |

## Related Documentation

- [Damage Pipeline](damage-pipeline.md) - How damage flows through reconcileDamage/applyDamage
- [DELETE Flag System](delete-flags.md) - Memory management for combat deaths
- [Spell Definitions](spell-definitions.md) - Spell damage types and classAmt values
- [Position Stance](position-stance.md) - Position effects on combat
- [Stats Attributes](stats-attributes.md) - Full stat system documentation (planned)
