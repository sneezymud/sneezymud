---
title: Combat Formulas
description: Mathematical formulas for hit probability, damage calculation, critical hits, attack counts, and defensive mechanics with stat scaling via plotStat.
keywords: [attackRound, defendRound, hits, getWeaponDam, getSkillDam, reconcileDamage, critSuccessChance, blowCount, plotStat, getStatMod, specialAttack, genericDam, classAmt, dexModifier, armorClass]
category: Important Systems
created_by_model: opus
last_updated: 2026-02-01
source_files: [code/code/misc/combat.cc, code/code/misc/skill_dam.cc, code/code/misc/crit_combat.cc, code/code/misc/offense.cc, code/code/misc/being.h, code/code/misc/stats.cc, code/code/misc/damage.cc]
related: [damage-pipeline.md, memory-safety.md, spell-skill-framework.md, position-stance.md, character-foundation.md]
---

# Combat Formulas

## Overview

Combat resolution in SneezyMUD follows a probabilistic model where attack and defense bonuses compete to determine hit success. The system scales with level, equipment, stats, and skill training to produce meaningful progression while maintaining variance.

Damage calculation uses a layered approach: weapon attacks derive from base damage dice modified by strength and skill learning, while spell/skill damage uses a universal formula driven by class-specific multipliers and casting time. Both systems scale with level and apply stat-based modifiers.

Critical hits add variance through a karma-influenced probability check with severity scaling based on combat state. Attack frequency depends on class, combat modes, and buff effects, creating distinct combat cadences for different character builds.

All stat-to-modifier conversions flow through a non-linear scaling function that amplifies the value of extreme stats, making high-stat investment rewarding without making low stats completely unviable.

## Patterns

Always check reconcileDamage return value against -1 for death detection. Never use IS_SET_DELETE on this return value as it returns a sentinel, not a flag.

Always account for position penalties when calculating defensive bonuses. Standing provides full defense; lower positions scale down significantly.

Always apply strength modifier according to weapon damage type. Blunt weapons benefit fully from strength while piercing weapons receive only half the bonus.

Always consider NPC damage reduction when balancing skills. MOBs deal approximately half the damage of equivalent PC attacks.

Never ignore dual wield penalties. Secondary hand damage ranges from 30-60% of primary based on skill learning.

Never calculate crit probability without considering karma. Low karma characters have dramatically reduced crit rates while high karma can triple the baseline.

Always validate stat values before passing to plotStat. The formula assumes stats fall within the game's standard range.

## Reference

### Hit Probability by Modifier Difference

| Modifier Difference | Hit Factor | Hit Probability |
|---------------------|------------|-----------------|
| -333 | 0 | 0% (guaranteed miss) |
| -100 | 420 | 42% |
| 0 | 600 | 60% (baseline) |
| +100 | 780 | 78% |
| +222 | 1000 | 100% (guaranteed hit) |

Each modifier point changes hit probability by approximately 0.18%.

### Defense Contribution by Armor Class

| AC Value | Defense Contribution |
|----------|---------------------|
| -10 | +1100 |
| 0 | +550 |
| +10 | 0 |
| +20 | -550 |

### Position Defense Multipliers

| Position | Multiplier |
|----------|------------|
| Standing/Fighting | 1.0x |
| Resting | 0.75x |
| Sitting/Crawling | 0.5x |

SKILL_GROUNDFIGHTING reduces these penalties proportionally to learning.

### Strength Effect by Weapon Type

| Weapon Type | Strength Bonus |
|-------------|----------------|
| Blunt/Barehand | 100% |
| Slash | 75% |
| Pierce | 50% |

### classAmt Values for Selected Spells/Skills

| Spell/Skill | classAmt |
|-------------|----------|
| SPELL_GUST | 0.50 |
| SPELL_LIGHTNING_BOLT | 1.0 |
| SPELL_FIREBALL | 1.5-2.0 |
| SPELL_METEOR_STORM | 3.0-4.0 |
| SKILL_BASH | 0.5-0.75 |

### Attack Modifiers

| Component | Formula | Range |
|-----------|---------|-------|
| Base Level | level x 50/3 | 0-1167 (level 70) |
| Combat Mode (Defense) | -(level/2) | Penalty |
| Combat Mode (Offense/Berserk) | +(level/4) | Bonus |
| Chivalry (Mounted Deikhan) | 74 x skillValue / 100 | 0-74 |
| Cintai | skillValue x 3 / 20 | 0-15 |
| Advanced Offense | skillValue x 3 / 4 | 0-75 |
| Dexterity | 335 x getStatMod(STAT_DEX) - 335 | -67 to +84 |

### Attack Count Modifiers

| Effect | Bonus |
|--------|-------|
| Berserk | +0.5 per hand |
| Advanced Berserking | +1.0 per hand |
| Haste | +0.5 per hand |
| Celerite | +0.5 per hand |
| Mounted | x0.67 penalty |

### Dexterity Combat Bonus

| DEX Level | Bonus | Hit Rate Effect |
|-----------|-------|-----------------|
| Low (stat ~20) | -67 | -12% hit rate |
| Average (stat ~105) | 0 | Baseline |
| High (stat ~190) | +84 | +15% hit rate |

### Crit Skill Bonuses

| Skill | Bonus |
|-------|-------|
| SKILL_CRIT_HIT | +20 x skillValue (max +2000) |
| SKILL_POWERMOVE | +10 x skillValue (max +1000) |

### plotStat Common Uses

| Purpose | minValue | maxValue | Stat |
|---------|----------|----------|------|
| Hit/Defense Modifier | 0.8 | 1.25 | DEX |
| Damage Modifier | 0.8 | 1.25 | STR |
| Crit Chance | 0.5 | 2.0 | KAR |
| Spell Learning | 0.1 | 10.0 | INT/WIS |

### System Summary

| System | Key Formula | Typical Range |
|--------|-------------|---------------|
| Hit Probability | (600 + 9 x mod/5) / 1000 | 0-100% |
| Attack Bonus | level x 50/3 + skills + dex | 0-1500+ |
| Defense Bonus | (10-AC) x 55 + level x 50/3 | -500 to +1500 |
| Weapon Damage | (base+roll) x str x learn/100 | 5-100+ |
| Spell Damage | classAmt x lag x level x mods | 5-500+ |
| Crit Chance | karma x 1000 + skills vs 1d100000 | 0.1-3%+ |
| Attack Count | base + spec + haste + berserk | 1-12 |
| Stat Modifier | plotStat(stat, 0.8, 1.25) | 0.8-1.25x |

## Implementation

### Hit Success System

The hit success system combines attacker and defender modifiers to determine whether an attack connects.

**attackRound (combat.cc)** computes the attacker's bonus:

```
attackBonus = (level x 50/3) + combatModeModifiers + skillBonuses + dexModifier
```

Level provides the primary scaling factor. Combat mode adjustments penalize defensive stance by half-level while bonusing offensive/berserk by quarter-level. Skills like Chivalry (mounted Deikhans), Cintai, and Advanced Offense add flat bonuses scaled by learning. Dexterity applies through getStatMod conversion.

Discipline scaling varies by character type. PCs use SKILL_OFFENSE learning to replace a theoretical baseline. MOBs use DISC_COMBAT learnedness instead.

**defendRound (combat.cc)** computes the defender's bonus with different formulas for PCs and MOBs:

```
PC:  defendBonus = (10 - AC) x 55 + combatModeBonus + skillBonuses + dexModifier
MOB: defendBonus = (10 - AC) x 55 + (level x 50/3) + skillBonuses + dexModifier
```

Armor class provides the primary contribution, with each point worth 55 defense. MOBs gain an additional level-scaled component that PCs lack. Position penalties reduce the final value significantly for non-standing combatants.

**hits (combat.cc)** resolves the final probability:

```
mod = attackRound(target) - defendRound(target)
factor = clamp(600 + (9 x mod / 5), 0, 1000)
hit = (roll(1, 1000) <= factor)
```

The baseline hit rate is 60% when attack and defense are equal. The formula clamps to guarantee that extremely mismatched combatants still have hit floors and ceilings.

**specialAttack (combat.cc)** handles special attacks like bash and trip using a simpler model:

```
roll = random(1, 100)
situationalMod = clamp(modifier, -20, +20)

if roll <= 50 - mod: SUCCESS
if roll < 80 - mod: PARTIAL_SUCCESS
else: FAILURE
```

Situational modifiers are bounded to prevent extreme swings. Partial success typically means reduced effect rather than complete failure.

### Damage Calculation

**getWeaponDam (combat.cc)** computes melee weapon damage:

```
weaponDamage = (baseDam + rollDam + bonusDam) x strModifier x weaponLearning / 100
```

Base damage comes from the weapon's dice definition. Strength modifier varies by damage type: blunt and barehand receive full strength bonus, slashing receives 75%, piercing only 50%. Weapon learning derives from skill value or level, whichever is higher, capped at 100.

Dual wield applies a secondary hand penalty: (30 + 30 x SKILL_DUAL_WIELD / 100)% of primary damage. Two-handed specialization provides a multiplier: (100 + 50 x SKILL_2H_SPEC / 100) / 100.

**getSkillDam (skill_dam.cc)** handles spell and skill damage, called through genericDam:

```
baseDamage = classAmt x lagRounds x level
damage = baseDamage x diffModifier x statModifier x randomVariance
```

The classAmt value represents the spell's fundamental power coefficient. Lag rounds reflect casting time investment. Level provides linear scaling. Difficulty and stat modifiers adjust for target and caster capabilities.

NPC damage is reduced by approximately 48% (multiplied by 0.5195). Area effect spells receive an additional 25% reduction. Random variance adds plus or minus half the attacker's level.

**reconcileDamage (damage.cc)** applies damage and returns -1 on victim death. This is a sentinel value, not a DELETE flag. Code must compare directly against -1, not use IS_SET_DELETE.

### Critical Hit System

**critSuccessChance (crit_combat.cc)** determines critical hit probability:

```
diceRoll = random(1, rollRange)
critChance = karmaBase + skillBonuses + gearBonuses

PC rollRange:  100,000
NPC rollRange: 1,000,000
```

The karma base ranges from 500-2000 via plotStat(STAT_KAR, 0.5, 2.0) multiplied by 1000. Skills add substantial bonuses: SKILL_CRIT_HIT provides up to +2000, SKILL_POWERMOVE up to +1000. Equipment with APPLY_CRIT_FREQUENCY adds 0.05% per point.

Effective crit rates range from approximately 0.001% for baseline NPCs to 3%+ for fully trained PCs.

Critical severity scales from 10-100:

```
severity = 10 + levelDifference + (100 - victimHPPercent) + skillBonuses
```

Higher severity produces more severe effects including limb damage, stuns, and bleeds.

**critFailureChance (crit_combat.cc)** checks for fumbles:

```
if random(1, 300) > (karmaScore + drunkPenalty) x 10:
    CRITICAL FAILURE
```

Low karma and intoxication increase failure chance. Twenty different failure types exist including dropping weapon, hitting self, hitting ally, and falling down.

### Attack Count System

**blowCount (offense.cc)** determines attacks per round.

For MOBs:
```
attacks = min(12.0, getMult())
Split: 60% primary hand, 40% secondary
```

For PCs:
```
primaryAttacks = blowCountSplitter() + specializationBonus + speedModifier
secondaryAttacks = blowCountSplitter() + speedModifier
```

Speed modifiers from Berserk, Advanced Berserking, Haste, and Celerite stack. Mounted combat reduces attacks by 33%. Typical PC attack counts range from 1-8+; MOBs can reach up to 12.

### Stat-to-Modifier System

**plotStat (being.h)** provides universal stat-to-modifier conversion:

```
normStat = (stat - 5) / 200
scaled = normStat ^ 1.4
output = minValue + (maxValue - minValue) x scaled
```

The power of 1.4 creates a non-linear curve that amplifies high stats. This makes investment in extreme stats rewarding.

**getStatMod (stats.cc)** provides standardized modifier output:

```
modifier = ((plotStat(stat, 0.8, 1.25, 1.0) - 1) x multiplier) + 1
```

This maps stat value 5 to 0.8 (-20%), stat value 105 to 1.0 (neutral), and stat value 205 to 1.25 (+25%).

The dexterity combat bonus formula is:

```
dexBonus = (int)(335 x getStatMod(STAT_DEX) - 335)
```

This produces a range from -67 to +84, translating to approximately -12% to +15% hit rate effect.

## Troubleshooting

### Hit rate seems too low or too high

**Symptom:** Characters miss far more often than expected, or never miss.

**Cause:** Modifier difference outside expected range due to level mismatch, broken equipment, or missing skills.

**Diagnostic:** Log attackRound and defendRound return values. Compare modifier difference against the hit probability table. Check for position penalties being applied incorrectly.

**Fix:** Verify AC values are in expected range. Check that skills are being added correctly. Confirm position is being detected accurately.

### Weapon damage inconsistent with weapon stats

**Symptom:** High-damage weapons dealing low damage, or vice versa.

**Cause:** Strength modifier not applying correctly for damage type, or weapon learning calculation incorrect.

**Diagnostic:** Verify weapon damage type. Check that strength modifier is using correct percentage (100/75/50 based on type). Confirm weaponLearning calculation is returning expected values.

**Fix:** Ensure damage type is set correctly on weapon. Verify strength stat is being read properly. Check that skill learning values are in range.

### Critical hits never occurring

**Symptom:** Extended combat with no critical hits despite trained SKILL_CRIT_HIT.

**Cause:** Karma stat too low, roll range issue, or skill bonus not being added.

**Diagnostic:** Calculate expected crit chance. Verify karma base is being computed correctly via plotStat. Confirm skill bonuses are stacking.

**Fix:** Check that plotStat is receiving correct stat value. Verify SKILL_CRIT_HIT learning is being read. Confirm roll range is using PC value (100000) not NPC value (1000000).

### reconcileDamage death not detected

**Symptom:** Code continues execution after victim death, leading to crashes or corruption.

**Cause:** Using IS_SET_DELETE instead of comparing return value to -1.

**Diagnostic:** Check call site for incorrect flag check pattern.

**Fix:** Replace IS_SET_DELETE(rc, DELETE_VICT) with direct comparison: reconcileDamage(...) == -1. Return appropriate DELETE flag after detection.

### Attack count lower than expected

**Symptom:** Characters attacking fewer times per round than their skills should allow.

**Cause:** Mounted penalty being applied, or speed buffs not stacking correctly.

**Diagnostic:** Check for riding state. Verify buff affects are present. Log blowCount intermediate values.

**Fix:** Confirm mounted penalty is only applied when actually riding. Verify haste/celerite/berserk affects are being detected. Check that blowCountSplitter is returning expected base value.
