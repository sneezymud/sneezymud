---
title: To-Hit and Defense System
category: critical
keywords: [hits, attackRound, defendRound, specialAttack, combat resolution, accuracy, evasion, guaranteed zones, stat modifiers, position penalties]
related: [combat-formulas.md, combat-rounds.md, position-stance.md, equipment-wear.md]
primary_symbols:
  functions: [hits, attackRound, defendRound, specialAttack, specAttackMod, getStatMod, plotStat]
  classes: [TBeing]
  files: [code/code/misc/combat.cc, code/code/misc/offense.cc, code/code/misc/defense.cc, code/code/misc/stats.cc]
---

## Overview

Combat resolution determines whether attacks succeed or fail through two parallel mechanisms. Regular melee combat flows through attackRound and defendRound to compute modifiers, then hits performs the final roll with guaranteed success and failure zones preventing extreme stat differences from creating perfect accuracy or invulnerability. Special combat abilities use specialAttack which incorporates multiple stats directly and supports partial success outcomes.

The system maintains 5% guaranteed hit and miss zones to preserve gameplay variance. Position modifiers create tactical advantages where mounted and flying combatants gain significant bonuses while resting or sitting characters suffer severe penalties. Combat modes trade offense for defense or vice versa. Stat scaling favors maxing individual stats over spreading points due to non-linear returns from plotStat power law exponents.

Common errors include forgetting guaranteed zones when calculating expected damage, not accounting for combat mode double-dipping that creates swings in both attack and defense simultaneously, ignoring position multiplier effects between attacker and defender, misunderstanding berserking defense catastrophe where Advanced Berserking inverts the penalty structure, and treating thief stealth bonuses as persistent when they vanish once combat starts.

## Patterns

### Melee Hit Resolution Flow

The hits function serves as the primary arbiter for regular attacks. attackRound computes the attacker's total modifier from base level scaling, combat mode adjustments, skill bonuses, DEX contribution, equipment hitroll, blind fighting penalties, spell casting penalties, and position modifiers. defendRound computes the defender's total modifier from armor class, combat mode adjustments, skill bonuses, AGI contribution, spell effects, blind fighting penalties, and position modifiers. The difference becomes the mod parameter that converts to hit probability through a clamped linear formula.

Combat mode selection creates bidirectional effects. Switching to ATTACK_OFFENSE adds my_lev/4 to attack but subtracts my_lev/4 from defense, creating a total swing of my_lev/2 in the attacker's favor relative to someone in ATTACK_DEFENSE. ATTACK_BERSERK multiplies this effect by imposing massive defense penalties that scale with skill level but can be eliminated entirely by maxing SKILL_BERSERK and gaining SKILL_ADVANCED_BERSERKING.

Position creates compounding advantages. A mounted attacker facing a resting defender gains my_lev/4+1 to both attack and defense while the defender loses my_lev/3+1 to both, producing a total swing of 7*my_lev/12+2 which translates to approximately 7% hit rate increase at level 60. SKILL_GROUNDFIGHTING mitigates but never eliminates penalties when fighting from disadvantaged positions.

### Special Attack Resolution Flow

The specialAttack function uses a different resolution model that incorporates four stats simultaneously. Primary offense and defense stats apply through getStatMod with 0.8 to 1.25 multiplier range. Secondary offense and defense stats apply through plotStat with narrower 0.92 to 1.08 multiplier range. The situational modifier applies first, level difference adds asymmetric bonuses favoring PC attackers, then the random roll undergoes stat-based adjustment before threshold comparison.

Situational modifiers accumulate from position, stealth status, home turf, spell effects, and vision penalties. specAttackMod computes these bidirectionally by considering both attacker advantages and defender vulnerabilities as inverted bonuses. Webbed defenders grant +4 to attackers. Blind defenders grant +6 to attackers. Position disadvantages flip to become attacker bonuses.

The wary state provides assassination protection. After surviving backstab, cudgel, throatslit, or ranged snipe attempts, characters without SKILL_SUBTERFUGE become wary and impose -10 situational penalty on subsequent surprise attacks for a fixed duration. This prevents repeated assassination attempts from becoming trivial after the first failure.

### Vision and Blindness Mechanics

Blindness applies asymmetric penalties to regular versus special attacks. When the attacker cannot see the target, regular attacks suffer -my_lev-1 penalty through attackRound while special attacks suffer only -6 penalty through specAttackMod. SKILL_BLINDFIGHTING reduces both penalties proportionally to skill level, reaching zero penalty at 100 skill value.

Bidirectional blindness compounds penalties. If neither combatant can see the other, the attacker suffers vision penalties normally while simultaneously gaining bonuses from the defender's inability to see. This creates scenarios where both combatants have reduced effectiveness but the relative advantage depends on which penalty system applies.

### Guaranteed Zones and Probability Clamping

The hits function implements guaranteed success on rolls 0-49 and guaranteed failure on rolls 950-999 regardless of mod value. This creates a 5% floor and 95% ceiling on hit probability. The factor calculation clamps to 0-1000 range before comparison, preventing mod values outside -333 to +222 from producing impossible hit rates.

Special attacks use adjusted roll thresholds instead. Rolls adjusted to 5 or below guarantee success. Rolls adjusted above 95 guarantee failure. The 50 threshold separates complete success from partial success or failure depending on whether the ability allows partial outcomes. This creates discrete success tiers rather than continuous probability.

### Stat Modifier Scaling Effects

DEX affects attack accuracy while AGI affects defense evasion through identical scaling formulas. The formula 335 * getStatMod - 335 converts stat multiplier range 0.8-1.25 into combat bonus range -67 to +84. This represents approximately -12% to +15% hit rate effect. The non-linear plotStat power law creates higher marginal returns at high stat values, making the last 50 points of stat increase more valuable than the first 50 points.

Special attacks compound stat effects multiplicatively. The adjusted roll divides by both defender primary and secondary stat modifiers while multiplying by both attacker primary and secondary stat modifiers. A character with 205 FOC and 205 KAR attacking a character with 5 AGI and 5 PER experiences massive effective roll reduction from the 1.25 * 1.08 / 0.8 / 0.92 ≈ 1.85 multiplier on raw stat advantage.

### Combat Mode Trade-off Mechanics

ATTACK_DEFENSE trades offense for improved defense. The attacker loses my_lev/2 from attack but gains my_lev/4 to defense plus SKILL_ADVANCED_DEFENSE bonuses to both. Against an opponent in ATTACK_OFFENSE, this creates a my_lev/2 swing in their favor for attack and a my_lev/2 swing in their favor for defense, potentially reaching my_lev total swing in defensive advantage.

ATTACK_BERSERK represents the extreme offensive stance. The base penalty subtracts my_lev/4 from defense like ATTACK_OFFENSE, then applies an additional scaling penalty of 8*my_lev*factor/100 where factor starts at 100 for unskilled warriors. SKILL_BERSERK reduces factor toward zero. SKILL_ADVANCED_BERSERKING doubles factor before penalty calculation, which paradoxically eliminates the penalty entirely when combined with maxed SKILL_BERSERK since 100-100=0, then 0*2=0.

### Equipment Effect Integration

Armor class converts to defense bonus through asymmetric formulas. PC armor uses max(armor-500, 0) * 2/3 with a level-based cap preventing excessive AC stacking. Mob armor uses max(armor-400, 0) * 5/6 without caps. Lower AC numbers produce higher armor values and thus higher bonuses. SKILL_OOMLAT multiplies PC armor by an additional factor before conversion.

Hitroll provides direct attack bonus through 5 * (getHitroll + getSpellHitroll) / 3. Each +1 hitroll contributes approximately 1.67 bonus points or 0.3% hit rate increase. This scales linearly unlike stat contributions. Equipment with APPLY_HITROLL affects stacks additively before multiplication.

### Spell Casting Combat Penalties

Characters with active spelltask lose attack accuracy from 2*my_lev/3 penalty in attackRound. They also lose their entire AGI defense bonus in defendRound through the conditional that only applies AGI modifier when not spell casting. This double penalty makes simultaneous combat and spellcasting significantly less effective than pure martial combat.

### Level Difference Asymmetry

Special attacks apply level difference modifiers asymmetrically. PC attackers gain or lose the full level difference as situational modifier. Mob attackers gain or lose only one-fifth of the level difference. This helps PCs fight above their level by providing +1 modifier per level advantage while suffering full penalty when fighting higher level opponents, whereas mobs experience dampened effects in both directions.

## Reference

### hits Function Probability Table

The base factor calculation follows factor = 600 + 9*mod/5 clamped to 0-1000 range. Random roll uses ::number(0, 999) for uniform distribution across 1000 outcomes.

| Mod | Factor | Hit Rate | Context |
|-----|--------|----------|---------|
| -333 | 0 | 5% | Minimum (guaranteed zone only) |
| -222 | 200 | 20% | Severe disadvantage |
| -100 | 420 | 47% | Moderate disadvantage |
| -56 | 500 | 55% | Slight disadvantage |
| 0 | 600 | 65% | Even match |
| +56 | 700 | 75% | Slight advantage |
| +100 | 780 | 83% | Moderate advantage |
| +222 | 1000 | 95% | Maximum (guaranteed zone clips) |

### attackRound Component Breakdown

Base bonus starts at level * 50/3. Doubling level calculation produces my_lev = max(10, 16.67 * get_doubling_level(GetMaxLevel())) used for relative scaling.

| Component | Formula | Typical Range |
|-----------|---------|---------------|
| Base Level | level * 50/3 | 0 to 1167 at level 70 |
| Combat Mode (DEFENSE) | -my_lev/2 + SKILL_ADVANCED_DEFENSE/3 | -83 to -58 |
| Combat Mode (OFFENSE/BERSERK) | +my_lev/4 | +42 at level 60 |
| SKILL_CHIVALRY (mounted) | 74 * max(10, skill)/100 | 7 to 74 |
| SKILL_CINTAI | skill/20 * 3 | 0 to 15 |
| SKILL_OFFENSE | my_lev * max(10, skill)/100 | Varies by level |
| SKILL_ADVANCED_OFFENSE | skill/4 * 3 | 0 to 75 |
| DEX Modifier | 335 * getStatMod(STAT_DEX) - 335 | -67 to +84 |
| Equipment Hitroll | 5 * (hitroll + spellHitroll) / 3 | ~1.67 per +1 hitroll |
| Blind Fighting | -(my_lev + 1) * (100 - skill)/100 | -167 to 0 |
| Spell Casting | -2*my_lev/3 | -111 at level 60 |

### defendRound Component Breakdown

PC armor calculation uses armor = 1000 - getArmor(), then max(armor-500, 0) * 2/3 with cap at (GetMaxLevel * 1000/60) + my_lev. Mob armor uses max(armor-400, 0) * 5/6 without caps.

| Component | Formula | Typical Range |
|-----------|---------|---------------|
| Base AC (PC) | max(armor-500, 0) * 2/3 | 0 to 333 before cap |
| Base AC (Mob) | max(armor-400, 0) * 5/6 | 0 to 833 |
| Combat Mode (DEFENSE) | my_lev/4 + SKILL_ADVANCED_DEFENSE/10 | +52 at level 60 |
| Combat Mode (OFFENSE) | -my_lev/4 | -42 at level 60 |
| Combat Mode (BERSERK) | -my_lev/4 - 8*my_lev*factor/100 | -42 to -522 |
| SKILL_CHIVALRY (mounted) | 159 * max(10, skill)/100 | 16 to 159 |
| SKILL_DEFENSE | my_lev * min(100, skill)/100 | Up to my_lev |
| SKILL_OOMLAT | Multiplies armor before conversion | Varies |
| AGI Modifier | 335 * getStatMod(STAT_AGI) - 335 | -67 to +84 |
| SPELL_AURA_GUARDIAN | Fixed | +40 |
| Blind Fighting | -(my_lev + 1) * (100 - skill)/100 | -167 to 0 |

### Position Modifier Tables

attackRound and defendRound apply identical position modifiers. specAttackMod uses smaller integer values.

| Position | attackRound/defendRound | specAttackMod |
|----------|------------------------|---------------|
| DEAD/MORTALLYW/INCAP/STUNNED/SLEEPING | -bonus (negates all) | N/A |
| RESTING | -(my_lev/3 + 1) | -5 |
| SITTING | -(my_lev/4 + 1) | -3 |
| CRAWLING | 0 | -1 |
| STANDING/ENGAGED/FIGHTING | 0 | 0 |
| MOUNTED | +(my_lev/4 + 1) | +2 |
| FLYING | +(my_lev/3 + 1) | +3 |

SKILL_GROUNDFIGHTING reduces penalties for positions below STANDING through penalty * (100 - skill)/100 with minimum maintained at -1.

### specialAttack Return Value Semantics

| Return Constant | Numeric Value | Roll Threshold | Meaning |
|-----------------|---------------|----------------|---------|
| GUARANTEED_SUCCESS | 2 | Adjusted roll ≤ 5 | Bypasses probability |
| COMPLETE_SUCCESS | 1 | Adjusted roll < 50 | Full effect |
| PARTIAL_SUCCESS | -2 | Adjusted roll < 80 | Reduced effect (if allowed) |
| FAILURE | 0 | Adjusted roll ≥ 50 or ≥ 80 | No effect |
| GUARANTEED_FAILURE | -1 | Adjusted roll > 95 | Bypasses probability |

Callers must check SKILL_INEVITABILITY affect before calling specialAttack as it bypasses the entire resolution system and returns GUARANTEED_SUCCESS immediately.

### specAttackMod Accumulation Table

Base modifier starts at zero. Each condition adds or subtracts from the running total. Final result clamps to -20 to +20 range before level difference application.

| Condition | Modifier | Notes |
|-----------|----------|-------|
| Attacker RESTING | -5 | Position penalty |
| Attacker SITTING | -3 | Position penalty |
| Attacker CRAWLING | -1 | Position penalty |
| Attacker MOUNTED | +2 | Position bonus |
| Attacker FLYING | +3 | Position bonus |
| Attacker AFF_SNEAK (thief, no combat) | +5 | Vanishes when fight() is true |
| Attacker AFF_HIDE (thief, no combat) | +5 | Vanishes when fight() is true |
| Attacker homeTurf() | +3 | Territorial advantage |
| Attacker backgroundBonus() | +3 | Environmental advantage |
| Attacker AFF_WEB | -4 | Mobility impaired |
| Attacker SPELL_STUPIDITY | -1 | Mental impairment |
| Attacker SPELL_CURSE | -2 | Divine penalty |
| Attacker SPELL_BLESS | +1 | Divine bonus |
| Attacker SPELL_AURA_MIGHT | +3 | Enhanced power |
| Attacker cannot see target | -6 | Vision penalty, reduced by SKILL_BLINDFIGHTING |
| Defender RESTING | +5 | Inverted position |
| Defender SITTING | +3 | Inverted position |
| Defender CRAWLING | +1 | Inverted position |
| Defender MOUNTED | -2 | Inverted position |
| Defender FLYING | -4 | Inverted position |
| Defender AFF_WEB | +4 | Inverted mobility |
| Defender SPELL_STUPIDITY | +1 | Inverted mental state |
| Defender SPELL_CURSE | +2 | Inverted divine effect |
| Defender SPELL_SANCTUARY | -3 | Divine protection |
| Defender SPELL_CRUSADE | -3 | Holy protection |
| Defender SPELL_AURA_GUARDIAN | -3 | Magical protection |
| Defender cannot see attacker | +6 | Inverted vision, reduced by defender SKILL_BLINDFIGHTING |
| Defender isWary() for assassination skills | -10 | Post-attempt protection |

### Stat Modifier Conversion Formulas

getStatMod applies to primary stats in both attack and defense calculations. plotStat with 0.92, 1.08, 1.0 parameters applies to secondary stats in special attacks.

| Stat Value | getStatMod Result | Combat Bonus (335*mod - 335) |
|------------|-------------------|------------------------------|
| 5 | 0.80 | -67 |
| 55 | 0.90 | -34 |
| 105 | 1.00 | 0 |
| 155 | 1.12 | +42 |
| 205 | 1.25 | +84 |

plotStat secondary modifier range spans 0.92 to 1.08 for stats 5 to 205, providing smaller variance than primary stats.

### Equipment Contribution Calculations

Hitroll conversion uses 5 * (getHitroll + getSpellHitroll) / 3 in attackRound.

| Total Hitroll | Attack Bonus | Hit Rate Effect |
|---------------|--------------|-----------------|
| -10 | -17 | -3% |
| 0 | 0 | Baseline |
| +10 | +17 | +3% |
| +20 | +33 | +6% |
| +30 | +50 | +9% |

Armor class contributes differently for PCs versus mobs with different base offsets and multipliers.

| AC Value | PC Bonus (before cap) | Mob Bonus |
|----------|-----------------------|-----------|
| -30 | 0 | 0 |
| -20 | 0 | 0 |
| -10 | 0 | 0 |
| 0 | 333 | 833 |
| +10 | 667 | 1667 |

PC AC cap formula GetMaxLevel * 1000/60 + my_lev produces maximum defense bonus 1167 + 167 = 1334 at level 70.

### Combat Mode Summary Matrix

| Mode | Attack Modifier | Defense Modifier | Notes |
|------|----------------|------------------|-------|
| NORMAL | 0 | 0 | Baseline |
| DEFENSE | -my_lev/2 + ADV_DEF/3 | +my_lev/4 + ADV_DEF/10 | Favors survival |
| OFFENSE | +my_lev/4 | -my_lev/4 | Balanced trade |
| BERSERK | +my_lev/4 | -my_lev/4 - 8*my_lev*factor/100 | Extreme offense, factor from SKILL_BERSERK |

SKILL_ADVANCED_DEFENSE provides bonuses to both attack and defense in ATTACK_DEFENSE mode. SKILL_ADVANCED_BERSERKING doubles the factor before penalty calculation.

## Implementation

### hits Internal Flow

The function receives the mod parameter computed from attackRound() - defendRound(). Early exit conditions check for AFF_FOCUS_ATTACK on attacker, target position below POSITION_RESTING, or AFFECT_DUMMY with level 60 on target, all returning GUARANTEED_SUCCESS immediately. The factor calculation applies 600 + 9*mod/5 then clamps result with min(max(factor, 0), 1000). Random roll uses ::number(0, 999) generating values 0 through 999 inclusive.

The roll comparison sequence evaluates roll < 50 first for guaranteed success, roll >= 950 second for guaranteed miss, roll < factor third for normal hit, else normal miss. Return values use TRUE and GUARANTEED_SUCCESS constants for hits, FALSE and GUARANTEED_FAILURE for misses, maintaining backward compatibility with older code expecting boolean-like values.

### attackRound Calculation Sequence

Initialization computes base bonus from level * 50/3 and doubling level from max(10, 16.67 * get_doubling_level(GetMaxLevel())). Combat mode application checks getCombatMode() and branches to add or subtract my_lev/4 or my_lev/2 appropriately, then applies advanced skill bonuses if present.

Skill bonuses accumulate sequentially. SKILL_CHIVALRY checks isRiding() before applying mounted bonus. SKILL_CINTAI adds flat bonus. SKILL_OFFENSE or disc_lev for DISC_COMBAT on mobs applies proportional scaling. SKILL_ADVANCED_OFFENSE adds another flat bonus. Each calculation uses max(10, getSkillValue()) to provide minimum baseline for skilled characters.

DEX modifier applies through getStatMod(STAT_DEX) multiplied by 335 with 335 subtracted to center at zero for stat value 105. Equipment hitroll calls getHitroll() and getSpellHitroll() separately then combines with 5/3 multiplier. Blind fighting constructs penalty from my_lev base reduced proportionally by SKILL_BLINDFIGHTING if known. Spell casting checks spelltask member variable. Position modifiers evaluate getPosition() against enum thresholds, apply SKILL_GROUNDFIGHTING mitigation for disadvantaged positions if character is awake.

### defendRound Calculation Sequence

Armor acquisition uses getArmor() to fetch total AC from equipment and effects, converts to armor = 1000 - getArmor() so lower AC produces higher armor. PC branch checks isPc() then calculates bonus = max(armor - 500, 0) * 2/3 and applies cap min(bonus, GetMaxLevel * 1000/60 + my_lev). Mob branch uses bonus = max(armor - 400, 0) * 5/6 without capping.

SKILL_OOMLAT application for PCs multiplies armor by 1 + skill/250.0 before the 2/3 conversion. Combat mode modifiers follow similar branching to attackRound with different multipliers. SKILL_CHIVALRY, SKILL_DEFENSE bonuses accumulate. AGI modifier applies only when !spelltask through conditional check. SPELL_AURA_GUARDIAN adds flat 40. Blind fighting and position modifiers work identically to attackRound.

### specialAttack Resolution Sequence

Default stat assignment occurs when parameters use STAT_NONE sentinel values, replacing with STAT_FOC, STAT_KAR, STAT_AGI, STAT_PER for offensive primary/secondary and defensive primary/secondary respectively. SKILL_INEVITABILITY check occurs first through affectedBySpell, removing affect and returning GUARANTEED_SUCCESS if present.

Situational modifier calculation invokes specAttackMod(target) and adds return value. Clamp application restricts range to -20 through +20. Level difference computation checks isPc() to determine whether full level difference or one-fifth applies. These accumulate into the base situational modifier before roll generation.

Roll adjustment starts with ::number(1, 100) producing 1-100 range, subtracts situational modifier, then multiplies by attacker primary stat mod and secondary plotStat, divides by defender primary stat mod and secondary plotStat. Threshold comparison checks adjusted roll <= 5 for guaranteed success, > 95 for guaranteed failure, < 50 for complete success, < 80 for partial success if allowed, else failure.

### specAttackMod Accumulation Logic

Position modifier extraction calls getPosition() on attacker and indexes into lookup table returning -5 for RESTING through +3 for FLYING. Thief stealth checks hasClass(CLASS_THIEF) && !fight(), adds +5 for each of isAffected(AFF_SNEAK) and isAffected(AFF_HIDE). Territorial and background checks invoke homeTurf() and backgroundBonus() boolean functions.

Spell effect iteration checks multiple affectedBySpell and isAffected conditions, accumulating positive modifiers for AFF_WEB, SPELL_STUPIDITY, SPELL_CURSE on attacker as penalties, SPELL_BLESS, SPELL_AURA_MIGHT as bonuses. Vision check uses canSee(target), constructs base -6 penalty, applies SKILL_BLINDFIGHTING proportional reduction if present.

Defender modifications follow inverted logic. Position extracts target->getPosition() and inverts sign. Spell effects invert similarly with positive effects becoming attacker penalties and negative effects becoming attacker bonuses. Vision check uses target->canSee(this) and inverts penalty to bonus. Wary state checks target->isWary() only for assassination skill types, subtracts 10 if detected.

### getStatMod and plotStat Integration

getStatMod implementation calls plotStat(STAT_CURRENT, stat, 0.8, 1.25, 1.0) to get base multiplier, subtracts 1, multiplies by class-specific or racial multiplier, adds 1 back to re-center. Return value represents multiplicative stat effectiveness. Combat bonus formulas multiply this by large constants like 335 then subtract the same constant to convert multiplier range to additive bonus range.

plotStat uses power law formula with exponent defaulting to 1.4. Input stat value clamps to valid range, normalizes to 0-1 scale using stat limits, applies power function, scales output by high-low range, adds low offset. The 0.92-1.08 range for secondary stats comes from tighter low-high parameters. The 0.8-1.25 range for primary stats comes from wider spread.

### Combat Mode State Management

getCombatMode retrieves current mode from character state. setCombatMode validates new mode and updates state with appropriate messaging. Mode changes in combat trigger recalculation of attack and defense values on next hit attempt. No caching persists between combat rounds requiring full recalculation each time attackRound or defendRound executes.

### Position Effect Application

Position queries use getPosition() which returns positionTypeT enum value. Comparison against POSITION_STANDING threshold determines whether ground fighting mitigation applies. doesKnowSkill(SKILL_GROUNDFIGHTING) gates the penalty reduction. Calculation multiplies negative penalty by (100 - getSkillValue(SKILL_GROUNDFIGHTING))/100 then enforces min(penalty, -1) to prevent complete elimination.

Mounted and flying bonuses check riding state and flight capability before applying. The my_lev/4 and my_lev/3 formulas scale with effective combat level creating larger position advantages for higher level characters.

### Vision and Blindness Implementation

canSee performs comprehensive visibility check including light levels, invisibility, blindness affects, position constraints, and spectral state. Returning false triggers blind fighting penalty construction in both attackRound and defendRound as well as specAttackMod.

SKILL_BLINDFIGHTING queries through doesKnowSkill then getSkillValue to retrieve 0-100 skill level. Penalty multiplies by (100 - skill)/100 creating linear reduction from full penalty at 0 skill to zero penalty at 100 skill. Both attacker and defender apply this reduction independently to their respective vision penalties.

### Level Difference Asymmetry Implementation

Special attack level calculation retrieves GetMaxLevel() for both attacker and defender, computes difference, then branches on isPc() check for attacker. PC attackers with positive level difference add full difference. PC attackers with negative difference add one-fifth of difference. Mob attackers always use one-fifth regardless of sign.

### Spell Casting Detection

The spelltask member variable points to active TTask instance when character currently casts a spell. Null check determines casting state. attackRound subtracts 2*my_lev/3 when spelltask exists. defendRound skips AGI bonus application entirely when spelltask exists, creating larger penalty than attack reduction alone.

### Equipment Value Caching

getHitroll aggregates APPLY_HITROLL affects from all equipped items plus temporary spell bonuses. getSpellHitroll separates spell-granted hitroll from equipment hitroll. getArmor aggregates base AC, racial AC, equipment APPLY_ARMOR affects, and skill-based AC contributions. Values cache until equipment changes trigger recalculation through affectModify.

### Guaranteed Zone Edge Cases

Rolls exactly on threshold boundaries follow specific comparison rules. roll < 50 uses strict less-than making roll 50 not guaranteed. roll >= 950 uses greater-than-or-equal making roll 950 guaranteed miss. Special attack uses roll <= 5 and roll > 95 creating symmetric 5% zones. Factor clamping occurs before comparison preventing negative factors from creating inverse probability.

## Troubleshooting

### Unexpected Hit Rate Deviations

Calculate expected hit rate from mod using 600 + 9*mod/5 formula, clamp to 0-1000, add 5% guaranteed success, subtract 5% from values that would exceed 95%. Compare observed hit rate over significant sample size, at least 100 attacks. Small samples exhibit high variance due to random distribution.

Verify attackRound and defendRound calculations match expected values by logging intermediate results. Common discrepancies arise from forgetting combat mode affects both attack and defense simultaneously, position modifiers applying to both combatants, blind fighting penalties when visibility changes, spell casting penalties reducing both offense and defense.

Check for AFF_FOCUS_ATTACK on attacker or AFFECT_DUMMY level 60 on target creating 100% hit rate outside normal probability system. Verify target position above POSITION_RESTING preventing automatic hits.

### Special Attack Success Rate Problems

Confirm situational modifier stays within -20 to +20 clamp range after specAttackMod calculation. Extreme spell effects, position differences, or vision penalties may exceed range before clamping. Level difference asymmetry means PC versus mob combat behaves differently than PC versus PC combat.

Verify stat values used for primary and secondary offense/defense. Default assignment uses FOC/KAR for offense and AGI/PER for defense when not specified. Custom skill implementations may override these. Check that getStatMod and plotStat produce expected multiplier ranges for current stat values.

Calculate effective roll threshold accounting for stat multipliers. Character with 1.25 primary offense mod attacking target with 0.8 primary defense mod experiences 1.25/0.8 = 1.5625 effective roll divisor making high rolls less likely to exceed thresholds. Secondary stats compound this effect.

Inspect SKILL_INEVITABILITY affect presence causing automatic success bypassing entire resolution system. Check for wary state on defender reducing assassination attempt success by 10 points.

### Combat Mode Ineffectiveness

Quantify expected modifier differences using my_lev calculations. Level 60 character switching from NORMAL to OFFENSE gains +42 attack and loses -42 defense. Against opponent in DEFENSE mode who gains +42 defense and loses -21 attack, total swing becomes 42+42=84 attack advantage offset by 42+21=63 defense disadvantage.

Berserking defense catastrophe occurs when SKILL_BERSERK remains low. Factor calculation (100 - skill) creates penalty multiplier. At 50 skill, factor becomes 50. With SKILL_ADVANCED_BERSERKING, factor doubles to 100. Defense penalty reaches 8*60*100/100 = 480 points or approximately -86% hit rate against the berserker. Maxing SKILL_BERSERK to 100 reduces factor to 0 eliminating penalty entirely.

SKILL_ADVANCED_DEFENSE bonuses in DEFENSE mode add to both attack and defense. Skill value 100 provides +33 attack through /3 division and +10 defense through /10 division, partially offsetting base attack penalty.

### Position Modifier Inconsistencies

Verify position state matches expected value through getPosition(). Transitional positions during movement or combat initiation may differ from intended stance. Standing from sitting takes time during which SITTING penalties still apply.

SKILL_GROUNDFIGHTING reduces but never eliminates penalties. Skill value 100 reduces penalty to -1 minimum, still applying negative modifier. Characters expecting zero penalty when sitting with maxed skill will experience continued disadvantage.

Mounted bonuses require isRiding() to return true. Dismounting mid-combat removes bonuses immediately. SKILL_CHIVALRY bonuses stack with position bonuses making mounted combat particularly effective for skilled riders.

Flying position requires flight capability and appropriate terrain. Indoor environments or low ceilings may prevent flying preventing bonus application.

### Blind Fighting Penalty Failures

canSee checks multiple conditions beyond simple blindness. Invisible targets, spectral state, darkness without light source, position below conscious threshold all trigger vision failure. Verify specific canSee failure cause.

SKILL_BLINDFIGHTING skill value must reach 100 for complete penalty elimination. Linear scaling means 50 skill only reduces penalty by half. Vision penalties differ between regular attacks (my_lev+1) and special attacks (6) requiring different skill investments for equivalent effectiveness.

Bidirectional blindness applies penalties to both combatants. Attacker blind suffers penalty, defender blind grants attacker bonus. Total effect when both blind combines penalty reduction from attacker skill with bonus reduction from defender skill, potentially favoring higher skilled combatant.

### Equipment Bonus Underperformance

Hitroll contribution scales at 5/3 per point creating non-integer bonuses that truncate. Verify total hitroll includes both getHitroll and getSpellHitroll. Temporary spell effects expire reducing bonuses mid-combat.

AC contribution differs by character type. PCs use 2/3 multiplier after 500 offset. Mobs use 5/6 multiplier after 400 offset. PC AC caps at level-based maximum preventing excessive stacking. Characters at cap gain no benefit from additional AC reduction.

SKILL_OOMLAT multiplies armor before conversion for PCs. Skill value 100 adds 40% to armor before 2/3 multiplier application, providing approximately 27% effective defense increase. Benefit scales with base armor making it more valuable for heavily armored characters.

### Spell Effect Conflicts

Multiple spell effects stack additively in specAttackMod. AFF_WEB -4, SPELL_CURSE -2, blind penalty -6 cumulative create -12 total penalty before clamping. Verify clamp range -20 to +20 not exceeded.

Spell casting spelltask penalty removes AGI defense bonus entirely in defendRound. Characters expecting AGI contribution while casting experience significant defense reduction beyond the 2*my_lev/3 attack penalty.

SPELL_AURA_GUARDIAN provides both +40 defense in defendRound and -3 situational modifier against special attacks. Defensive spell effects compound differently across resolution systems.

### Thief Stealth Bonus Loss

Stealth bonuses from AFF_SNEAK and AFF_HIDE apply only when !fight() returns true. First successful attack initiates combat causing fight() to return true and bonuses to vanish for remainder of combat. Surprise attacks gain one-time advantage then revert to normal combat.

### Stat Scaling Expectation Mismatches

Linear stat increases produce non-linear combat advantages due to plotStat power law exponent 1.4. First 50 stat points from 105 to 155 produce smaller combat bonus increase than second 50 points from 155 to 205. Min-maxing single stats provides higher returns than spreading points evenly.

Special attack stat compounding multiplies advantages. Character with both high primary and secondary offense stats against target with both low primary and secondary defense stats experiences multiplicative advantage far exceeding additive stat differences.

### Wary State Bypass Failures

Wary state check occurs only for specific assassination skills: SKILL_BACKSTAB, SKILL_CUDGEL, SKILL_THROATSLIT, SKILL_RANGED_PROF. Other surprise attacks or special abilities bypass wary detection.

SKILL_SUBTERFUGE prevents wary state application. Characters with this affect never become wary and never impose -10 penalty on subsequent assassination attempts. Check target affects before expecting wary protection.

### Level Difference Calculation Errors

Level difference uses GetMaxLevel not current level. Drained or reduced level characters use their maximum historical level for calculation. Verify both attacker and defender GetMaxLevel values.

PC versus mob asymmetry creates imbalanced outcomes. PC attacking mob 5 levels lower gains +5 modifier. Mob attacking PC 5 levels lower gains +1 modifier. This intentionally favors PC offensive capability.
