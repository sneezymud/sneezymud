---
title: To-Hit and Defense System
description: Combat resolution probability calculations balancing attacker modifiers against defender capabilities with guaranteed hit/miss zones.
category: important
keywords: [combat resolution, hit probability, guaranteed zones, combat modes, stealth bonuses, wary state]
primary_symbols:
  functions: [hits, attackRound, defendRound, specialAttack, specAttackMod, getStatMod, plotStat]
  classes: [TBeing]
  enums: [GUARANTEED_SUCCESS, GUARANTEED_FAILURE, COMPLETE_SUCCESS, PARTIAL_SUCCESS, FAILURE, ATTACK_NORMAL, ATTACK_DEFENSE, ATTACK_OFFENSE, ATTACK_BERSERK, POSITION_DEAD, POSITION_INCAP, POSITION_STUNNED, POSITION_SLEEPING, POSITION_RESTING, POSITION_SITTING, POSITION_CRAWLING, POSITION_STANDING, POSITION_MOUNTED, POSITION_FLYING, AFF_WEB, AFF_FOCUS_ATTACK, SPELL_STUPIDITY, SPELL_CURSE, SPELL_BLESS, SPELL_AURA_MIGHT, SPELL_SANCTUARY, SPELL_CRUSADE, SPELL_AURA_GUARDIAN, SKILL_INEVITABILITY, SKILL_GROUNDFIGHTING, SKILL_BLINDFIGHTING, SKILL_ADVANCED_DEFENSE, SKILL_ADVANCED_OFFENSE, SKILL_BERSERK, SKILL_ADVANCED_BERSERKING, SKILL_CHIVALRY, SKILL_CINTAI, SKILL_OOMLAT, SKILL_SUBTERFUGE]
---

## Overview

When your warrior swings at a goblin, what determines whether the blade connects? The to-hit and defense system answers this question through probability calculations that balance attacker and defender capabilities.

The system provides two distinct resolution paths. Regular hits handle standard melee combat through the `hits()` function, computing success probability from attack and defense modifiers. Special attacks handle combat abilities like bash, trip, and backstab through `specialAttack()`, incorporating stats more directly with different success tiers.

Both paths share a critical design principle: guaranteed hit and miss zones. Regardless of how overwhelming an attacker's advantage or how impenetrable a defender's armor, 5% of attacks always succeed and 5% always fail. This prevents extreme stat differences from creating deterministic outcomes, maintaining tension even in mismatched fights.

Combat resolution flows through three layers. First, modifier calculation via `attackRound()` and `defendRound()` computes situational bonuses and penalties. Second, resolution via `hits()` or `specialAttack()` translates modifiers into success determination. Third, successful hits proceed to damage calculation. Understanding this flow matters because the system's complexity lives in the modifier calculations, not the final resolution.

The modifier difference between attacker and defender determines hit probability on a sliding scale. An even match yields roughly 60% hit rate. Each point of modifier advantage shifts hit probability by approximately 0.18%. Large enough advantages can approach certainty, but the guaranteed zones cap actual outcomes between 5% and 95%.

## Patterns

### Regular Hit Resolution

Always check the return value from `hits()`. The function returns `GUARANTEED_SUCCESS` (bypasses probability), `TRUE` (normal hit), `FALSE` (normal miss), or `GUARANTEED_FAILURE` (bypasses probability). Different return values may require different handling in damage calculation or combat logging.

Never assume extreme modifiers guarantee hits or misses. The 5% guaranteed zones clip outcomes: +222 mod yields 95% hits (not 100%), -333 mod yields 5% hits (not 0%). Design encounters assuming this variance exists.

### Modifier Calculation

Always call both `attackRound()` and `defendRound()` before `hits()`. The hit probability depends on their difference, not absolute values. Calling only one produces meaningless results.

Check for spellcasting state before applying AGI defense bonus. Characters with active `spelltask` lose their AGI modifier to defense, making casters vulnerable during cast time.

Account for position modifiers in both attack and defense. Mounted attackers gain bonuses; sitting defenders suffer penalties. Position swings compound: a flying attacker versus a resting defender creates large modifier differences.

### Special Attack Resolution

Use the correct stat mapping for each ability. Default mapping uses FOC/KAR for offense and AGI/PER for defense, but individual skills may override these. Check the specific skill implementation before assuming defaults.

Handle partial success returns when allowed. When `partialSuccessAllowed` is true, rolls between 50-80 return `PARTIAL_SUCCESS` instead of `FAILURE`. Skills like grapple may have reduced effects on partial success rather than binary outcomes.

Account for `SKILL_INEVITABILITY` in modifier calculation. Characters with this affect receive a bonus modifier through `specAttackMod()` (combat.cc:3016-3023), but normal roll and stat calculations still occur. The affect is removed after a successful hit (combat.cc:3192-3200), not on every use.

### Combat Mode Selection

Never ignore combat mode when calculating bonuses. Each mode applies symmetric attack/defense modifications that create net swings between combatants. Ignoring mode produces incorrect hit probabilities.

Understand berserk mode's extreme defense penalty. Without `SKILL_BERSERK` mastery, berserking imposes massive defense reductions. With maxed skill and `SKILL_ADVANCED_BERSERKING`, the penalty disappears entirely. This skill progression dramatically changes berserking viability.

### Blindness Handling

Apply blind penalties to both attacker and defender when appropriate. If attacker cannot see target, attack suffers. If defender cannot see attacker, defense suffers. Both can apply simultaneously if neither can see the other.

Check for `SKILL_BLINDFIGHTING` to mitigate blind penalties. At skill 100, the penalty is fully negated. Partial skill values produce proportional reduction.

Note that blindness applies asymmetric penalties to regular versus special attacks. Regular attacks suffer `-my_lev-1` penalty through `attackRound()` while special attacks suffer only `-6` penalty through `specAttackMod()`. This means blind fighting skill investment has different effectiveness across attack types.

### Stealth and Surprise

Remember that thief stealth bonuses only apply before combat starts. Once `fight()` returns true, SNEAK and HIDE bonuses to special attacks disappear. This affects backstab timing strategy.

Apply the wary modifier to assassination attempts. After surviving backstab, cudgel, throatslit, or ranged snipe, targets gain wary status imposing -10 on subsequent surprise attacks. Check `isWary()` before calculating modifiers. SKILL_SUBTERFUGE prevents wary state application entirely.

## Reference

### Symbol Quick Reference

| Symbol | Type | Purpose |
|--------|------|---------|
| `hits()` | function | Primary melee hit resolution |
| `attackRound()` | function | Compute attacker modifier |
| `defendRound()` | function | Compute defender modifier |
| `specialAttack()` | function | Ability-based hit resolution |
| `specAttackMod()` | function | Situational modifier calculation |
| `getStatMod()` | function | Stat to combat modifier conversion |
| `plotStat()` | function | Power-law stat scaling (exponent 1.4) |
| `TBeing` | class | Base class for all combatants |
| `GUARANTEED_SUCCESS` | return value | 5% auto-hit zone triggered |
| `GUARANTEED_FAILURE` | return value | 5% auto-miss zone triggered |
| `COMPLETE_SUCCESS` | return value | Special attack full success |
| `PARTIAL_SUCCESS` | return value | Special attack partial success |

### Combat Mode Effects

| Mode | Attack Modifier | Defense Modifier |
|------|-----------------|------------------|
| ATTACK_NORMAL | 0 | 0 |
| ATTACK_DEFENSE | -my_lev/2 (+ skill) | +my_lev/4 (+ skill) |
| ATTACK_OFFENSE | +my_lev/4 | -my_lev/4 |
| ATTACK_BERSERK | +my_lev/4 | -my_lev/4 - (skill-scaled penalty) |

### Position Modifiers (Attack/Defense)

| Position | Regular Combat | Special Attacks |
|----------|----------------|-----------------|
| DEAD/INCAP/STUNNED/SLEEPING | Negated | N/A |
| RESTING | -(my_lev/3+1) | -5 |
| SITTING | -(my_lev/4+1) | -3 |
| CRAWLING | 0 | -1 |
| STANDING/ENGAGED/FIGHTING | 0 | 0 |
| MOUNTED | +(my_lev/4+1) | +2 |
| FLYING | +(my_lev/3+1) | +3 |

SKILL_GROUNDFIGHTING reduces penalties for positions below STANDING through `penalty * (100 - skill)/100` with minimum maintained at -1.

### Spell Effect Modifiers (Special Attacks)

| Effect | On Self | On Target |
|--------|---------|-----------|
| AFF_WEB | -4 | +4 to attacker |
| SPELL_STUPIDITY | -1 | +1 to attacker |
| SPELL_CURSE | -2 | +2 to attacker |
| SPELL_BLESS | +1 | N/A |
| SPELL_AURA_MIGHT | +3 | N/A |
| SPELL_SANCTUARY | N/A | -3 to attacker |
| SPELL_CRUSADE | N/A | -3 to attacker |
| SPELL_AURA_GUARDIAN | N/A | -3 to attacker |

### Mod-to-Hit-Rate Mapping

| Mod Value | Factor | Hit Rate |
|-----------|--------|----------|
| -333 | 0 | 5% (floor) |
| -222 | 200 | 20% |
| -100 | 420 | 42% |
| -56 | 500 | 55% |
| 0 | 600 | 60% |
| +56 | 700 | 75% |
| +100 | 780 | 78% |
| +222 | 1000 | 95% (ceiling) |

### Stat Effect Ranges

| Stat Range | getStatMod() | Combat Bonus | Hit Rate Effect |
|------------|--------------|--------------|-----------------|
| 5 (minimum) | 0.8 | -67 | -12% |
| 55 | 0.9 | -34 | -6% |
| 105 (average) | 1.0 | 0 | 0% |
| 155 | 1.12 | +42 | +8% |
| 205 (maximum) | 1.25 | +84 | +15% |

### Equipment Contribution

| Total Hitroll | Attack Bonus | Hit Rate Effect |
|---------------|--------------|-----------------|
| -10 | -17 | -3% |
| 0 | 0 | Baseline |
| +10 | +17 | +3% |
| +20 | +33 | +6% |
| +30 | +50 | +9% |

| AC Value | PC Bonus (before cap) | Mob Bonus |
|----------|-----------------------|-----------|
| -30 | 0 | 0 |
| -10 | 0 | 0 |
| 0 | 333 | 833 |
| +10 | 667 | 1667 |

PC AC cap formula: `GetMaxLevel * 1000/60 + my_lev` produces maximum defense bonus 1334 at level 70.

### Default Special Attack Stats

| Role | Primary Stat | Secondary Stat |
|------|--------------|----------------|
| Offense | STAT_FOC | STAT_KAR |
| Defense | STAT_AGI | STAT_PER |

### Key Files

| File | Purpose |
|------|---------|
| `code/code/misc/combat.cc` | Core resolution functions |
| `code/code/misc/offense.cc` | Attack calculations |
| `code/code/misc/defense.cc` | Defense calculations |
| `code/code/misc/stats.cc` | Stat modifier implementation |
| `code/code/cmd/cmd_low.cc` | Doubling level calculation |

## Implementation

### Regular Hit Resolution Flow

The `hits()` function in `combat.cc` implements melee hit determination. Called from `perform_violence()` during combat rounds, it receives an optional modifier parameter that adjusts the base probability.

The function computes a factor from the modifier using the formula: factor = 600 + (9 * mod / 5). This factor is clamped to the 0-1000 range. A random roll from 0-999 determines the outcome.

Four outcome paths exist:
1. Roll below 50 returns `GUARANTEED_SUCCESS` regardless of factor
2. Roll 950 or higher returns `GUARANTEED_FAILURE` regardless of factor
3. Roll below factor returns `TRUE` (hit)
4. Roll at or above factor returns `FALSE` (miss)

The factor formula means each mod point changes hit rate by 9/5000, approximately 0.18%. The baseline factor of 600 produces 60% hit rate at mod 0. Factor clamping means mod values beyond -333 or +222 have no additional effect.

Several conditions bypass the probability check entirely for guaranteed hits: `AFF_FOCUS_ATTACK` on the attacker, target position below `POSITION_RESTING`, or `AFFECT_DUMMY` with level 60 on the target.

### Attack Modifier Calculation

The `attackRound()` function computes the attacker's contribution to hit probability. The calculation starts with a base bonus derived from level: bonus = level * 50/3. A level 60 character has base bonus 1000.

The doubling level system scales relative bonuses. Computed via `get_doubling_level()`, this represents how many levels make a creature twice as difficult. The scaled value my_lev = max(10, 16.67 * doubling_level) controls combat mode and skill bonus magnitudes.

Combat mode applies the first modifier layer. ATTACK_DEFENSE subtracts my_lev/2 but adds back SKILL_ADVANCED_DEFENSE/3 if known. ATTACK_OFFENSE and ATTACK_BERSERK add my_lev/4. ATTACK_NORMAL applies no modifier.

Skill bonuses layer on top. SKILL_CHIVALRY adds up to 74 points when mounted. SKILL_CINTAI adds up to 15 points. SKILL_OFFENSE (or DISC_COMBAT for mobs) adds scaled my_lev based on skill percentage. SKILL_ADVANCED_OFFENSE adds up to 75 points.

DEX contributes via `getStatMod()`: bonus += 335 * getStatMod(STAT_DEX) - 335. This produces -67 to +84 points depending on DEX value.

Equipment hitroll adds: bonus += 5 * (getHitroll() + getSpellHitroll()) / 3. Each +1 hitroll produces roughly 1.67 bonus points.

Visibility penalties apply when the attacker cannot see the target. Base penalty is -my_lev - 1, reduced proportionally by `SKILL_BLINDFIGHTING` proficiency.

Spellcasting penalty applies when `spelltask` is active: bonus -= 2 * my_lev / 3.

Position modifiers apply last. Positions below standing impose penalties (resting: -my_lev/3 - 1, sitting: -my_lev/4 - 1). Positions above standing grant bonuses (mounted: +my_lev/4 + 1, flying: +my_lev/3 + 1). Positions below resting negate the entire bonus. SKILL_GROUNDFIGHTING can mitigate penalties from low positions, reducing them proportionally while maintaining a minimum -1 penalty.

### Defense Modifier Calculation

The `defendRound()` function mirrors attack calculation for the defender's contribution. AC forms the base: for PCs, bonus = max((1000 - getArmor() - 500), 0) * 2/3. For mobs, bonus = max((1000 - getArmor() - 400), 0) * 5/6. Lower AC numbers (better armor) produce higher bonuses.

PCs face an AC cap preventing gear from exceeding level-appropriate values: bonus = min(bonus, (GetMaxLevel() * 1000 / 60) + my_lev).

SKILL_OOMLAT for PCs adds percentage bonus to armor value before the main calculation, multiplying armor by `1 + skill/250.0` before the 2/3 conversion.

Combat mode applies inversely to attack. ATTACK_DEFENSE adds my_lev/4 plus SKILL_ADVANCED_DEFENSE/10. ATTACK_OFFENSE subtracts my_lev/4. ATTACK_BERSERK subtracts my_lev/4 plus an additional penalty: (8 * my_lev * factor) / 100, where factor depends on SKILL_BERSERK mastery. Factor starts at 100, subtracts skill value, and doubles if SKILL_ADVANCED_BERSERKING is known. Maxed berserk skill eliminates this penalty entirely.

SKILL_CHIVALRY adds up to 159 points when mounted, more than double the attack bonus.

SKILL_DEFENSE adds scaled my_lev based on skill percentage.

AGI contributes via `getStatMod()` only when not casting spells: bonus += 335 * getStatMod(STAT_AGI) - 335. Active spellcasting negates AGI defense entirely.

SPELL_AURA_GUARDIAN adds a flat 40 bonus.

Visibility penalties apply when the defender cannot see the attacker, identical to attack-side calculation.

Position modifiers mirror attack-side modifiers exactly.

### Special Attack Resolution Flow

The `specialAttack()` function handles ability-based attacks with different success tiers. It receives stat type parameters for offense and defense, defaulting to FOC/KAR for offense and AGI/PER for defense when STAT_NONE sentinel values are passed.

Situational modifiers come from `specAttackMod()` plus any caller-provided adjustment. The combined value is clamped to -20 to +20 range.

Level difference adjusts the situational modifier asymmetrically. PCs attacking gain or lose the full level difference if positive; mobs and negative differences use level_diff / 5. This gives PCs advantage against lower-level targets while limiting mob advantage against lower-level PCs.

The roll starts at random 1-100 minus situational modifier. It then multiplies by attacker primary stat modifier, multiplies by attacker secondary stat plotStat (0.92-1.08 range), divides by defender primary stat modifier, and divides by defender secondary stat plotStat.

The adjusted roll determines outcome:
- 5 or below: GUARANTEED_SUCCESS
- Above 95: GUARANTEED_FAILURE
- Below 50: COMPLETE_SUCCESS
- Below 80 (if partial allowed): PARTIAL_SUCCESS
- Otherwise: FAILURE

SKILL_INEVITABILITY adds a bonus modifier via `specAttackMod()`. Normal roll and stat calculations still apply. The affect is removed after a successful hit, not on every attempt.

### Situational Modifier Details

The `specAttackMod()` function computes bonuses and penalties from combat circumstances. Attacker position contributes: resting -5, sitting -3, crawling -1, mounted +2, flying +3.

Thief stealth adds +5 each for SNEAK and HIDE, but only before combat starts (when `fight()` returns false).

Territory bonuses add +3 each for home turf and appropriate background.

Spell effects modify both directions. On the attacker: AFF_WEB -4, SPELL_STUPIDITY -1, SPELL_CURSE -2, SPELL_BLESS +1, SPELL_AURA_MIGHT +3. On the defender (inverted to help attacker): AFF_WEB +4, SPELL_STUPIDITY +1, SPELL_CURSE +2, SPELL_SANCTUARY -3, SPELL_CRUSADE -3, SPELL_AURA_GUARDIAN -3.

Blind fighting penalties apply bidirectionally. If attacker cannot see target, -6 penalty mitigated by SKILL_BLINDFIGHTING. If defender cannot see attacker, +6 bonus for attacker, mitigated by defender's SKILL_BLINDFIGHTING.

Defender position inverts: resting target gives attacker +5, sitting +3, crawling +1, mounted -2, flying -4.

Surprise attacks (backstab, cudgel, throatslit, ranged snipe) check target wary status. Wary targets impose -10 penalty. Non-wary targets become wary after surviving the attempt unless protected by SKILL_SUBTERFUGE.

### Stat Modifier Mechanics

The `getStatMod()` function converts stat values to combat multipliers. It uses `plotStat()` with range 0.8-1.25, creating a power-law curve with exponent 1.4 where high stats have increasing marginal returns.

Primary stats (DEX for attack, AGI for defense, FOC for special attack offense, AGI for special attack defense) use the full 0.8-1.25 range. Secondary stats (KAR for special attack offense, PER for special attack defense) use a narrower 0.92-1.08 range.

The combat bonus formula: bonus = 335 * getStatMod(stat) - 335 produces -67 to +84 range. Each stat point above average contributes more than the previous point due to the power-law exponent. This rewards stat maximization over distribution.

### Equipment Integration

Hitroll from equipment adds directly to attack calculation via `getHitroll()` and `getSpellHitroll()`. The combined value multiplies by 5/3, making each +1 hitroll worth approximately 1.67 attack bonus points or 0.3% hit rate increase.

AC from equipment subtracts from the base 1000 in defense calculation. Lower AC numbers produce higher defense bonuses. The PC formula uses 2/3 multiplier while mobs use 5/6, making mob AC slightly more effective point-for-point.

SKILL_IRON_FLESH (monks) provides AC when not wearing armor, integrated into the base `getArmor()` calculation.

Equipment values cache until changes trigger recalculation through `affectModify()`.

### Performance Characteristics

All resolution functions are O(1) with arithmetic and stat lookups. The `hits()` function is lightest, called multiple times per combat round. `attackRound()` and `defendRound()` cache within a single hit attempt but recalculate across multiple hits. `specialAttack()` adds `plotStat()` calls making it slightly more expensive but called less frequently.

Stat modifiers are pre-calculated in `curStats`. Equipment bonuses cache in `getHitroll()` and `getArmor()`. Each resolution uses a single random roll rather than per-modifier rolls.

## Troubleshooting

### Symptom: Extreme Hit Rate Despite Stat Balance

**Likely cause:** Missing modifier calculation or guaranteed zone bypass condition.

**Diagnostic approach:** Log actual factor value before roll comparison. Check for AFF_FOCUS_ATTACK on attacker, target position, or AFFECT_DUMMY on target. Verify both `attackRound()` and `defendRound()` are being called.

**Fix:** Ensure all modifier sources are included. Remove bypass conditions if unintended.

### Symptom: Casters Taking Excessive Damage

**Likely cause:** AGI defense bonus disabled during spellcasting.

**Diagnostic approach:** Check `spelltask` state during damage events. Compare defense values with and without active casting.

**Fix:** This is intended behavior. Consider interrupt protection or faster cast times rather than changing defense calculation.

### Symptom: Berserking Warriors Dying Instantly

**Likely cause:** Missing SKILL_BERSERK or SKILL_ADVANCED_BERSERKING.

**Diagnostic approach:** Check skill values and calculate actual defense penalty. At low SKILL_BERSERK, penalty can exceed 400 points. Factor calculation uses `(100 - skill)`, so 50 skill yields factor 50. SKILL_ADVANCED_BERSERKING doubles factor before penalty calculation, which paradoxically eliminates the penalty when SKILL_BERSERK is maxed since `(100-100)*2 = 0`.

**Fix:** Train SKILL_BERSERK to high values before relying on berserk mode. SKILL_ADVANCED_BERSERKING further reduces penalty.

### Symptom: Stealth Attacks Performing Poorly

**Likely cause:** Combat already started when attack executes, negating stealth bonuses.

**Diagnostic approach:** Check `fight()` return value at attack time. Log SNEAK/HIDE affect presence.

**Fix:** Ensure stealth attacks initiate combat rather than executing during existing combat.

### Symptom: Special Attacks Always Failing Against Certain Targets

**Likely cause:** Wary state from previous assassination attempt or stat disadvantage.

**Diagnostic approach:** Check target `isWary()` status. Calculate full situational modifier including stat ratios. Remember situational modifier clamps to -20 to +20 before level difference application.

**Fix:** Wait for wary state to expire or use different attack types. Consider stat-boosting effects.

### Symptom: Mounted Combat Providing No Benefit

**Likely cause:** Missing SKILL_CHIVALRY preventing bonus application.

**Diagnostic approach:** Verify mounted position is properly set via `isRiding()`. Check SKILL_CHIVALRY presence and value.

**Fix:** Mounted bonuses require SKILL_CHIVALRY for full effect. Train the skill before relying on mount positioning.

### Symptom: Combat Mode Not Affecting Outcomes

**Likely cause:** Not accounting for bidirectional effects.

**Diagnostic approach:** Quantify expected modifier differences using my_lev calculations. Level 60 character switching from NORMAL to OFFENSE gains +42 attack and loses -42 defense. Against opponent in DEFENSE mode, total swings compound.

**Fix:** Calculate full attack and defense modifier changes for both combatants.

### Symptom: Position Penalties Persisting Despite Skill

**Likely cause:** SKILL_GROUNDFIGHTING reduces but never eliminates penalties.

**Diagnostic approach:** Verify position state via `getPosition()`. Skill value 100 reduces penalty to -1 minimum, not zero.

**Fix:** Accept minimum -1 penalty when fighting from disadvantaged positions, even with maxed skill.

### Symptom: Blind Fighting Skill Seems Ineffective

**Likely cause:** Different penalty magnitudes for regular vs special attacks, or bidirectional blindness.

**Diagnostic approach:** Vision penalties differ between regular attacks (my_lev+1) and special attacks (6). Bidirectional blindness applies penalties to attacker while granting bonuses from defender's blindness.

**Fix:** Understand which attack type you're using and whether both combatants are blind. Higher skill investment needed for regular attack penalty elimination.

### Symptom: Equipment Bonuses Not Scaling As Expected

**Likely cause:** Hitroll truncation, AC caps, or expired spell effects.

**Diagnostic approach:** Verify total hitroll includes both `getHitroll()` and `getSpellHitroll()`. Check if PC AC has hit level-based cap. Temporary spell effects may have expired.

**Fix:** Account for integer truncation in hitroll calculation (5/3 per point). PCs at AC cap gain no benefit from additional AC reduction.
