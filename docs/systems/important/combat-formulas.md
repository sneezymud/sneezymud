---
title: Combat Formulas
description: Mathematical formulas for hit probability, damage calculation, critical hits, attack counts, and defensive mechanics with stat scaling via plotStat.
keywords: [hit probability, damage calculation, critical hit mechanics, attack frequency, stat modifier, combat modifier, power law scaling, defense bonus, dual wield penalty, spell damage]
category: important
primary_symbols:
  functions: [attackRound, defendRound, hits, getWeaponDam, getSkillDam, reconcileDamage, critSuccessChance, blowCount, plotStat, getStatMod, specialAttack, genericDam]
  classes: [TBeing]
---

# Combat Formulas

## Overview

Combat resolution in SneezyMUD follows a probabilistic model where attack and defense bonuses compete to determine hit success. The system uses accumulating bonuses rather than cascading multipliers to keep balance tractable. All combat outcomes derive from four primitive operations: hit detection, damage calculation, critical resolution, and attack count determination.

Damage calculation uses a layered approach: weapon attacks derive from base damage dice modified by strength and skill learning, while spell/skill damage uses a universal formula driven by class-specific multipliers and casting time. Both systems scale with level and apply stat-based modifiers.

Critical hits add variance through a karma-influenced probability check with severity scaling based on combat state. PC critical chance operates on a 1-in-100,000 base while NPCs use 1-in-1,000,000 to compensate for higher attack counts. Severity determines effect intensity: limb damage at low severity, stunning at medium, bleeding and compounding injuries at high.

Attack frequency depends on class, combat modes, and buff effects, creating distinct combat cadences for different character builds. Mobs use a simple multiplier field capped at 12 attacks, while PCs accumulate fractional bonuses from specialization skills, combat mode, and haste effects.

All stat-to-modifier conversions flow through a non-linear scaling function that amplifies the value of extreme stats, making high-stat investment rewarding without making low stats completely unviable.

## Patterns

Always check reconcileDamage return value against -1 for death detection. Never use IS_SET_DELETE on this return value as it returns a sentinel, not a flag.

Always account for position penalties when calculating defensive bonuses. Standing provides full defense; lower positions scale down significantly.

Always apply strength modifier according to weapon damage type. Blunt weapons benefit fully from strength while slash uses `(statDam - 1) / 2 + 1` and pierce uses `(statDam - 1) / 3 + 1`.

Always consider NPC damage reduction when balancing skills. MOBs deal approximately half the damage of equivalent PC attacks.

Never ignore dual wield penalties. Secondary hand damage ranges from 30-60% of primary based on skill learning.

Never calculate crit probability without considering karma. Low karma characters have dramatically reduced crit rates while high karma can triple the baseline.

Always validate stat values before passing to plotStat. The formula assumes stats fall within the game's standard range.

Use getStatMod when you need standardized 0.8-1.25 range output for multiplicative effects. This wrapper applies plotStat with fixed bounds and adjusts the result around 1.0 as a neutral baseline.

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

The armor contribution to defense differs between PCs and MOBs. The game computes `armor = 1000 - getArmor()`, then:
- **PC:** `bonus = max((armor - 500), 0) * 2 / 3`
- **MOB:** `bonus = max((armor - 400), 0) * 5 / 6`

Both formulas floor at zero, so armor values below the threshold contribute nothing.

| AC Value | armor (1000 - AC*10) | PC Bonus (2/3 above 500) | MOB Bonus (5/6 above 400) |
|----------|----------------------|--------------------------|---------------------------|
| -10 | 1100 | 400 | 583 |
| 0 | 1000 | 333 | 500 |
| +5 | 950 | 300 | 458 |
| +10 | 900 | 266 | 416 |
| +40 | 600 | 66 | 166 |
| +50 | 500 | 0 | 83 |
| +60 | 400 | 0 | 0 |

### Position Defense Multipliers

| Position | Multiplier |
|----------|------------|
| Standing/Fighting | 1.0x |
| Resting | 0.75x |
| Sitting/Crawling | 0.5x |

SKILL_GROUNDFIGHTING reduces these penalties proportionally to learning.

### Strength Effect by Weapon Type

| Weapon Type | Formula | Approximate Bonus |
|-------------|---------|-------------------|
| Blunt/Barehand | `statDam` (full) | 100% |
| Slash | `(statDam - 1) / 2 + 1` | ~50% at low, approaches 50% |
| Pierce | `(statDam - 1) / 3 + 1` | ~33% at low, approaches 33% |

These are integer division formulas, not simple percentages. At low `statDam` values the `+1` floor matters more; at high values the divisor dominates.

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
| Dexterity | 335 x getStatMod(STAT_DEX) - 335 | -67 to +84 (attackRound only) |

### Attack Count Modifiers

| Effect | Bonus |
|--------|-------|
| Berserk | +0.5 per hand |
| Advanced Berserking | +1.0 per hand |
| Haste | +0.5 per hand |
| Celerite | +0.5 per hand |
| Mounted | x0.67 penalty |

### Dexterity Attack Bonus

DEX affects attackRound only. Defense uses AGI instead (see defendRound).

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

### Crit Severity

| Severity Range | Effects |
|----------------|---------|
| 10-30 | Minor limb damage, light bleeding |
| 31-60 | Stun duration, increased bleeding intensity |
| 61-100 | Compounding effects, extended durations, potential instant kill on wounded victims |

Severity formula: 10 + levelDifference + (100 - victimHPPercent) + skillBonuses

### Critical Failure Types

Twenty failure types including: weapon drop, self-damage, ally damage, falling prone, fumble opening defense.

Failure roll: d300 vs (karma + drunkPenalty) x 10. Low karma and intoxication increase failure chance.

### plotStat Common Uses

| Purpose | minValue | maxValue | Stat |
|---------|----------|----------|------|
| Attack Modifier | 0.8 | 1.25 | DEX |
| Defense Modifier | 0.8 | 1.25 | AGI |
| Damage Modifier | 0.8 | 1.25 | STR |
| Crit Chance | 0.5 | 2.0 | KAR |
| Spell Learning | 0.1 | 10.0 | INT/WIS |

### System Summary

| System | Key Formula | Typical Range |
|--------|-------------|---------------|
| Hit Probability | (600 + 9 x mod/5) / 1000 | 0-100% |
| Attack Bonus | level x 50/3 + skills + dex | 0-1500+ |
| Defense Bonus | max((armor-threshold),0) x factor | 0 to ~600 |
| Weapon Damage | (base+roll) x strByType x learn/100 | 5-100+ |
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
armor = 1000 - getArmor()
PC:  armorBonus = max((armor - 500), 0) * 2 / 3
MOB: armorBonus = max((armor - 400), 0) * 5 / 6

PC:  defendBonus = armorBonus + combatModeBonus + skillBonuses + agiModifier
MOB: defendBonus = armorBonus + skillBonuses + agiModifier
```

Armor contribution floors at zero below the threshold (500 for PCs, 400 for MOBs). PCs scale at 2/3 above threshold while MOBs scale at 5/6. MOBs do NOT gain a level-scaled component. Defense uses AGI (STAT_AGI), not DEX. Position penalties reduce the final value significantly for non-standing combatants.

**hits (combat.cc)** resolves the final probability:

```
mod = attackRound(target) - defendRound(target)
factor = clamp(600 + (9 x mod / 5), 0, 1000)
roll = ::number(0, 999)
hit = (roll < factor)
```

The baseline hit rate is 60% when attack and defense are equal. The roll generates a value in the 0-999 range and uses strict less-than comparison, so factor of 600 achieves exactly 60% probability. The formula clamps to guarantee that extremely mismatched combatants still have hit floors and ceilings.

**specialAttack (combat.cc)** handles special attacks like bash and trip using a simpler model:

```
roll = random(1, 100)
situationalMod = clamp(modifier, -20, +20)

roll = roll * attacker.getStatMod(primaryOffenseStat)
           * attacker.plotStat(secondaryOffenseStat, 0.92, 1.08, 1.0)
           / target.getStatMod(primaryDefenseStat)
           / target.plotStat(secondaryDefenseStat, 0.92, 1.08, 1.0)

if roll <= 50 - mod: SUCCESS
if roll < 80 - mod: PARTIAL_SUCCESS
else: FAILURE
```

The stat modifiers scale the roll before threshold comparison. The primary offensive and defensive stats use the standard getStatMod (0.8-1.25 range), while secondary stats use a narrower plotStat range (0.92-1.08). This means stat advantages can shift the effective roll substantially. Situational modifiers are bounded to prevent extreme swings. Partial success typically means reduced effect rather than complete failure.

### Damage Calculation

**getWeaponDam (combat.cc)** computes melee weapon damage:

```
weaponDamage = (baseDam + rollDam + bonusDam) x strModifier x weaponLearning / 100
```

Base damage comes from the weapon's dice definition. Bonus damage includes enchantments and crafting bonuses. Strength modifier varies by damage type: blunt and barehand receive full strength bonus (`statDam`), slashing uses `(statDam - 1) / 2 + 1`, and piercing uses `(statDam - 1) / 3 + 1` (integer division). Weapon learning derives from skill value or level, whichever is higher, capped at 100.

Dual wield applies a secondary hand penalty: (30 + 30 x SKILL_DUAL_WIELD / 100)% of primary damage. Two-handed specialization provides a multiplier: (100 + 50 x SKILL_2H_SPEC / 100) / 100.

**getSkillDam (skill_dam.cc)** handles spell and skill damage, called through genericDam:

```
baseDamage = classAmt x lagRounds x level
damage = baseDamage x diffModifier x statModifier x randomVariance
```

The classAmt value represents the spell's fundamental power coefficient. Lag rounds reflect casting time investment. Level provides linear scaling. Difficulty and stat modifiers adjust for target and caster capabilities.

NPC damage is reduced by approximately 48% (multiplied by 0.5195). Area effect spells receive an additional 25% reduction. Random variance adds plus or minus half the attacker's level.

**reconcileDamage (damage.cc)** applies damage and returns -1 on victim death. This is a sentinel value, not a DELETE flag. Code must compare directly against -1, not use IS_SET_DELETE. After confirming death, caller must construct DELETE_VICT flag if victim pointer was passed as parameter.

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

Higher severity produces more severe effects including limb damage, stuns, and bleeds. High severity (61-100) compounds multiple effects and can trigger instant kill on already-wounded victims.

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

Each fractional attack accumulates to the character's blow count. The combat round distributes attacks across equipped weapons. Each attack performs full hit detection, damage calculation, and critical check. Multi-attack rounds can trigger multiple crits per round, each using an independent probability roll.

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

The dexterity attack bonus formula (used by attackRound) is:

```
dexBonus = (int)(335 x getStatMod(STAT_DEX) - 335)
```

This produces a range from -67 to +84, translating to approximately -12% to +15% hit rate effect. The defendRound function uses the same formula but with STAT_AGI instead of STAT_DEX.

## Troubleshooting

### Hit rate seems too low or too high

**Symptom:** Characters miss far more often than expected, or never miss.

**Cause:** Modifier difference outside expected range due to level mismatch, broken equipment, or missing skills.

**Diagnostic:** Log attackRound and defendRound return values. Compare modifier difference against the hit probability table. Check for position penalties being applied incorrectly.

**Fix:** Verify AC values are in expected range. Check that skills are being added correctly. Confirm position is being detected accurately.

### Combat mode confusion

**Symptom:** Hit rates change unexpectedly when switching stances.

**Cause:** Defense mode imposes attack penalty, offense mode provides attack bonus.

**Diagnostic:** Check that stance matches intended strategy. Note that berserk provides larger bonus than offense but disables defense.

**Fix:** Verify combat mode is being set correctly and mode modifiers are applying with correct signs.

### Position penalty not applying

**Symptom:** Defense appears unaffected by position changes.

**Cause:** defendRound multiplies defense by position factor after accumulating bonuses. Groundfighting skill reduces penalties.

**Diagnostic:** Check position is actually changing. Verify groundfighting skill learning level.

**Fix:** Confirm position detection is working. If groundfighting is fully trained, penalty reduction is working as intended.

### Weapon damage inconsistent with weapon stats

**Symptom:** High-damage weapons dealing low damage, or vice versa.

**Cause:** Strength modifier not applying correctly for damage type, or weapon learning calculation incorrect.

**Diagnostic:** Verify weapon damage type. Check that strength modifier is using the correct formula: full `statDam` for blunt, `(statDam - 1) / 2 + 1` for slash, `(statDam - 1) / 3 + 1` for pierce. Confirm weaponLearning calculation is returning expected values.

**Fix:** Ensure damage type is set correctly on weapon. Verify strength stat is being read properly. Check that skill learning values are in range.

### Critical hits never occurring

**Symptom:** Extended combat with no critical hits despite trained SKILL_CRIT_HIT.

**Cause:** Karma stat too low, roll range issue, or skill bonus not being added.

**Diagnostic:** Calculate expected crit chance. Verify karma base is being computed correctly via plotStat. Confirm skill bonuses are stacking. Check that roll range uses PC value (100000) not NPC value (1000000).

**Fix:** Check that plotStat is receiving correct stat value. Verify SKILL_CRIT_HIT learning is being read. Confirm roll range matches character type.

### Critical failure too frequent

**Symptom:** Failures occur frequently at normal karma levels.

**Cause:** Karma stat below expected value or intoxication affecting roll.

**Diagnostic:** Check karma stat and intoxication level. Remember failure roll is d300 versus karma x 10, so karma below 15 makes failures common.

**Fix:** Verify karma stat retrieval. Check drunk status is not unexpectedly set.

### reconcileDamage death not detected

**Symptom:** Code continues execution after victim death, leading to crashes or corruption.

**Cause:** Using IS_SET_DELETE instead of comparing return value to -1.

**Diagnostic:** Check call site for incorrect flag check pattern.

**Fix:** Replace IS_SET_DELETE(rc, DELETE_VICT) with direct comparison: reconcileDamage(...) == -1. Return appropriate DELETE flag after detection.

### Death occurring but not detected

**Symptom:** Victim should be dead but combat continues.

**Cause:** reconcileDamage called with zero damage, hit point check using wrong comparison, or immortality flags.

**Diagnostic:** Verify damage value passed to reconcileDamage is non-zero. Confirm hit point check uses <= 0 not just < 0. Check for immortality flags on victim.

**Fix:** Ensure actual damage reaches reconcileDamage. Verify death threshold logic. Check immortality flag handling.

### Attack count lower than expected

**Symptom:** Characters attacking fewer times per round than their skills should allow.

**Cause:** Mounted penalty being applied, or speed buffs not stacking correctly.

**Diagnostic:** Check for riding state. Verify buff affects are present. Log blowCount intermediate values.

**Fix:** Confirm mounted penalty is only applied when actually riding. Verify haste/celerite/berserk affects are being detected. Check that blowCountSplitter is returning expected base value.

### Stat modifiers not applying

**Symptom:** Damage or hit chance unaffected by stat changes.

**Cause:** plotStat receiving incorrect stat value, or modifier not being used in calculation.

**Diagnostic:** Verify stat retrieval uses actual current stat including temporary bonuses, not base stat. Check affect system applies stat modifications before combat formula calls.

**Fix:** Confirm stat retrieval path. Verify affects are being applied. Check that getStatMod result is actually multiplied into damage or added to hit bonus.

### Stat bonus computed but not applied

**Symptom:** DEX or AGI changes don't affect hit rates.

**Cause:** attackRound not adding DEX result or defendRound not adding AGI result to accumulators. Note that attackRound uses STAT_DEX while defendRound uses STAT_AGI.

**Diagnostic:** Verify attackRound adds DEX bonus and defendRound adds AGI bonus. Check bonus is being added as integer, not truncated before casting.

**Fix:** Ensure dexBonus/agiBonus calculation occurs and result is added to the correct accumulator (DEX for offense, AGI for defense).
