---
title: Combat Formulas
category: important
keywords: [hit probability, damage calculation, critical hits, attack count, stat scaling, plotStat, getStatMod, attackRound, defendRound]
related: [damage-pipeline.md, position-stance.md, character-foundation.md, spell-skill-framework.md]
primary_symbols:
  functions: [attackRound, defendRound, hits, getWeaponDam, getSkillDam, reconcileDamage, critSuccessChance, blowCount, plotStat, getStatMod, specialAttack]
  classes: [TBeing]
  files: [code/code/misc/combat.cc, code/code/misc/skill_dam.cc, code/code/misc/crit_combat.cc, code/code/misc/offense.cc, code/code/misc/stats.cc, code/code/misc/damage.cc]
---

# Combat Formulas

## Overview

Combat resolution is deterministic given character stats, equipment, and randomization bounds. The system uses accumulating bonuses rather than cascading multipliers to keep balance tractable. All combat outcomes derive from four primitive operations: hit detection, damage calculation, critical resolution, and attack count determination.

Hit probability follows a linear interpolation between attacker offense and defender defense. A baseline 60% hit chance shifts approximately 0.18% per point of modifier difference, creating a range from guaranteed misses to guaranteed hits. This predictability lets players optimize gear and skill choices with calculable results.

Damage scales from level, equipment stats, and skill training but applies situational multipliers for weapon type, dual wielding, two-handed specialization, and multi-target effects. Spell damage uses a class amount coefficient scaled by lag cost and level, while weapon damage depends on weapon dice, strength, and weapon-specific learning.

Critical hits and failures use independent karma-driven probability rolls. PC critical chance operates on a 1-in-100,000 base while NPCs use 1-in-1,000,000 to compensate for higher attack counts. Severity depends on level difference and victim health percentage, determining whether a crit causes limb damage, stunning, or bleeding.

Attack count varies dramatically between player characters and mobs. Mobs use a simple multiplier field capped at 12 attacks. PCs accumulate fractional bonuses from specialization skills, combat mode, and haste effects, with primary and secondary hands tracked separately.

All attribute-to-bonus conversions flow through plotStat, a non-linear scaling function using an exponent of 1.4. This curve strongly rewards high stats while avoiding discontinuities. Dexterity affects both hit and defense through this mechanism, while strength modulates damage and karma governs critical probability.

## Patterns

### Stat-to-Modifier Conversion

All attributes convert to combat modifiers through plotStat. Normalize the stat by subtracting the minimum value and dividing by the stat range, apply a power curve for non-linearity, then scale to the desired output range. The 1.4 exponent creates a smooth acceleration favoring high stats without linear scaling issues.

Use getStatMod when you need standardized 0.8-1.25 range output for multiplicative effects. This wrapper applies plotStat with fixed bounds and adjusts the result around 1.0 as a neutral baseline. For additive bonuses like dexterity's contribution to hit chance, scale getStatMod output into the integer range used by the combat formula.

### Hit Resolution Flow

Compute attacker offense using level, combat skills, combat mode, and dexterity. Compute defender defense using armor class, level or combat skills, position penalty, and dexterity. Take the difference, scale by 9/5, add baseline 600, clamp to 0-1000 range, then roll d1000 and check if the roll is at or below the clamped factor.

Skills like offense, advanced offense, chivalry, and cintai provide additive bonuses to the attack roll. Defense mode imposes a penalty, while offense and berserk modes provide bonuses. Dexterity affects both sides through getStatMod, scaled by 335 to match the integer precision used in other combat components.

Position penalties reduce defense linearly: resting applies 0.75x, sitting and crawling apply 0.5x. Groundfighting skill reduces these penalties proportionally to learning, preventing prone fighters from becoming helpless targets.

### Damage Accumulation

Weapon damage starts with weapon object base and roll dice, adds bonus damage from enchantments, then multiplies by strength modifier and weapon learning percentage. Strength contribution varies by weapon type: blunt and barehand apply full strength, slash applies 75%, pierce applies 50%. Weapon learning defaults to twice the character's level if skill learning is lower, capped at 100%.

Spell and skill damage uses classAmt as a balance coefficient scaled by lag cost and caster level. Difficulty modifiers adjust damage based on save success, stat modifiers apply primary attribute scaling, and random variance adds level-divided noise. NPC casters deal approximately 52% of PC damage to compensate for numerical advantages. Area effect spells take a 25% damage penalty per target.

### Critical Resolution

Roll against karma-based threshold modified by crit hit skill, powermove skill, and equipment with crit frequency bonuses. PCs roll d100000 while NPCs roll d1000000 to balance attack count disparity. If the roll succeeds, compute severity from level difference, victim health percentage, and skill bonuses. Severity determines effect intensity: limb damage at low severity, stunning at medium, bleeding and compounding injuries at high.

Critical failure uses a separate karma check against d300. Low karma and intoxication increase failure probability. Failures randomly select from 20 effect types including weapon drops, self-damage, ally damage, and falling prone.

### Attack Count Assembly

Mobs directly use their mult field capped at 12, split 60% primary and 40% secondary. PCs call blowCountSplitter for each hand to get base attack count from specialization, then add mode bonuses for berserk and advanced berserking, add 0.5 attacks per hand from haste and celerite, and apply mounted 0.67 penalty. Primary hand gets an additional specialization bonus based on weapon skill training.

Each fractional attack accumulates to the character's blow count. The combat round distributes these attacks across active weapons. Dual wield damage applies a 30-60% penalty to secondary hand based on skill learning. Two-handed specialization multiplies primary hand damage by 1.0-1.5 based on skill learning.

## Reference

### Hit Probability Components

**attackRound**: Computes attacker offense bonus from level base, combat mode adjustment, skill bonuses, and dexterity modifier. Level contributes 50/3 per level, capping at approximately 1167 for level 70. Defense mode subtracts level/2, offense mode adds level/4, berserk mode adds level/4. Chivalry adds up to 74 when mounted as deikhan. Cintai adds up to 15. Advanced offense adds up to 75. Dexterity adds -67 to +84 via scaled getStatMod.

**defendRound**: Computes defender defense bonus from armor class, level or skills, position, and dexterity. PCs use AC formula: multiply (10 - AC) by 55, add combat mode and skill bonuses. Mobs use level formula: multiply (10 - AC) by 55, add (level × 50/3), add skill and dex bonuses. Position penalty multiplies final defense by 1.0 standing, 0.75 resting, 0.5 sitting or crawling. Groundfighting proportionally reduces penalty.

**hits**: Calculates hit factor from attacker and defender bonuses. Computes modifier difference, multiplies by 9/5, adds baseline 600, clamps to 0-1000 range. Rolls d1000 and returns true if roll is at or below factor. At zero modifier difference, hit probability is 60%. Each point of difference changes probability by approximately 0.18%. Modifier difference of -333 guarantees miss, +222 guarantees hit.

**specialAttack**: Validates special attack success for bash, trip, disarm, and similar maneuvers. Clamps situational modifier to -20 to +20 range. Rolls d100. Returns success if roll is 50 or below minus modifier, partial success if roll is below 80 minus modifier, failure otherwise.

### Damage Calculation Components

**getWeaponDam**: Computes melee weapon damage from weapon base, roll dice, bonus damage, strength modifier, and weapon learning. Adds base and rolled damage, applies bonuses from enchantments. Multiplies by strength modifier: full for blunt and barehand, 0.75 for slash, 0.5 for pierce. Multiplies by weapon learning divided by 100. Weapon learning defaults to min(100, max(level × 2, skillValue)). Dual wield secondary hand damage multiplies by (30 + 30 × dual_wield_skill / 100) / 100. Two-handed specialization multiplies by (100 + 50 × two_hand_spec / 100) / 100.

**getSkillDam**: Computes spell and skill damage via genericDam wrapper. Calculates base damage as classAmt × lagRounds × level. Applies difficulty modifier from saving throw success. Applies stat modifier from primary attribute via plotStat. Adds random variance of +/- (level / 2). NPC casters multiply final damage by 0.5195. Area effect spells multiply damage by 0.75 per target.

**reconcileDamage**: Processes damage application and death checking. Returns -1 when victim dies, not a DELETE flag. Caller must check return value with equality comparison (== -1), not IS_SET_DELETE. Propagates death through DELETE_VICT flag after confirming death condition.

### Critical Hit Components

**critSuccessChance**: Determines critical hit probability from karma, skills, and equipment. Computes karma base as 1000 × plotStat(STAT_KAR, 0.5, 2.0), yielding 500-2000 range. Adds skill bonuses: +20 per point of crit_hit skill (max +2000), +10 per point of powermove (max +1000). Adds gear bonuses from APPLY_CRIT_FREQUENCY at 0.05% per point. Rolls d100000 for PCs or d1000000 for NPCs. Returns success if roll is at or below accumulated threshold. Typical PC crit rate ranges from 0.1% baseline to 3% with full training. Typical NPC rate is ~0.001%.

**Crit Severity**: Computes severity value 1-100 from level difference, victim HP percentage, and skill bonuses. Base is 10. Adds level difference between attacker and victim. Adds (100 - victim HP percent). Adds skill bonuses from applicable combat skills. Caps at 100. Higher severity increases limb damage chance, stun duration, and bleed intensity.

**critFailureChance**: Determines critical failure probability from karma and intoxication. Rolls d300. Compares against (karma + drunk_penalty) × 10. Returns failure if roll exceeds threshold. Selects random failure type from 20 options: weapon drop, self-damage, ally damage, falling prone, fumble opening defense, and others.

### Attack Count Components

**blowCount**: Determines attacks per combat round. Mobs use min(12.0, getMult()) split 60% primary and 40% secondary. PCs call blowCountSplitter per hand for base count from specialization, add +0.5 per hand if berserk, add +1.0 per hand if advanced berserking, add +0.5 per hand if hasted, add +0.5 per hand if celerite, multiply by 0.67 if mounted. Primary hand adds specialization bonus from weapon skill. Each hand accumulates fractional attacks. Combat round distributes total attacks across equipped weapons.

### Stat Scaling Components

**plotStat**: Universal stat-to-modifier conversion. Normalizes stat by subtracting 5 and dividing by 200. Applies power curve of 1.4 exponent. Scales result to minValue-maxValue range. Typical usage: DEX and STR use 0.8-1.25 for damage and hit modifiers, KAR uses 0.5-2.0 for crit chance, INT and WIS use 0.1-10.0 for spell learning rate.

**getStatMod**: Standardized wrapper around plotStat for multiplicative modifiers. Calls plotStat with 0.8-1.25 range and 1.0 baseline. Adjusts output so stat 5 yields 0.8, stat 105 yields 1.0, stat 205 yields 1.25. Used consistently for damage and hit modifications.

**Dexterity Combat Bonus**: Specific formula for dexterity contribution to hit and defense. Computes (335 × getStatMod(STAT_DEX) - 335) as integer. Low dex (~20) yields -67, average dex (~105) yields 0, high dex (~190) yields +84. This -67 to +84 range translates to -12% to +15% hit rate shift via the 0.18% per point conversion in hits function.

## Implementation

### Hit Detection Flow

Combat round calls attackRound on attacker to accumulate offense bonuses. Calls defendRound on victim to accumulate defense bonuses. Passes both to hits function which computes modifier difference, scales to 0-1000 factor, and rolls d1000. If roll succeeds, damage calculation proceeds. If roll fails, combat messages report miss and round ends.

attackRound iterates skill bonuses by checking discipline learnedness and adding scaled values. Offense discipline for PCs and combat discipline for NPCs provide the primary skill component. Optional skills like chivalry and cintai add conditional bonuses when requirements are met. Combat mode check adjusts based on current stance. Dexterity pulls from getStatMod, scales by 335, subtracts 335 baseline, casts to integer.

defendRound diverges PC and NPC paths. PCs compute (10 - armorClass) × 55 as primary component, NPCs compute (10 - armorClass) × 55 + level × 50/3. Both add skill bonuses from defense-oriented disciplines. Position check multiplies accumulated defense by penalty factor if not standing or fighting. Groundfighting reduces penalty proportionally. Dexterity applies same scaled contribution as attack side.

hits clamps factor to prevent roll range overflow. Roll comparison uses less-than-or-equal so factor of 600 achieves exactly 60% probability. Function returns boolean directly to caller for immediate branching.

### Damage Calculation Flow

After hit detection succeeds, combat system routes to weapon or skill damage path. Weapon path calls getWeaponDam with attacker, weapon object, and target. Skill path calls genericDam which internally calls getSkillDam with skill type, level, lag cost, and stat modifiers.

getWeaponDam retrieves base damage and roll range from weapon object. Rolls dice and adds base. Checks weapon for bonus damage from enchantments or crafting. Computes strength modifier by checking weapon type enum and applying appropriate percentage. Retrieves weapon skill learning or defaults to level × 2 capped at 100. Multiplies accumulated damage by strength and learning factors. Applies dual wield or two-handed specialization multipliers if applicable. Returns integer damage for reconcileDamage processing.

getSkillDam multiplies classAmt coefficient by lag rounds and caster level for base damage. Checks difficulty parameter and applies save modifier if target partially resisted. Pulls primary stat for skill type, passes to plotStat for stat modifier, multiplies damage. Adds random variance by rolling +/- (level / 2). Checks if caster is NPC and multiplies by 0.5195 if true. Checks if skill hits multiple targets and multiplies by 0.75 if true. Returns final damage.

reconcileDamage receives damage value and applies armor absorption, damage type resistance, and position modifiers. Subtracts resulting damage from victim hit points. Checks if hit points drop to or below zero. If death occurs, returns -1. If victim survives, returns zero or DELETE flags from armor destruction or triggered reactions. Caller must check return value == -1 specifically for death condition.

### Critical Hit Flow

After hit succeeds but before damage calculation, combat system calls critSuccessChance. Function computes karma base by calling plotStat with 0.5-2.0 range and multiplying by 1000. Checks attacker for crit_hit skill and adds 20 per point. Checks for powermove skill and adds 10 per point. Iterates equipment for APPLY_CRIT_FREQUENCY bonuses and accumulates. Determines roll range: 100000 for PCs, 1000000 for NPCs. Rolls within range and compares to threshold. If roll fails, combat proceeds normally. If roll succeeds, compute severity.

Severity calculation starts at 10. Computes attacker level minus victim level and adds to severity. Computes victim current HP as percentage of max HP, subtracts from 100, adds to severity. Checks applicable combat skills for severity bonuses and adds. Clamps final severity to 100. Passes severity to critical effect determination.

Critical effect uses severity to select from effect table. Low severity (10-30) triggers minor limb damage or bleeding. Medium severity (31-60) adds stun duration and increases bleeding intensity. High severity (61-100) compounds multiple effects, extends durations, and can trigger instant kill on already-wounded victims. Effect application modifies victim state and schedules recovery timers.

Critical failure check runs independently on attacker after computing hit but before damage. Pulls karma stat and drunk level. Computes threshold as (karma + drunk) × 10. Rolls d300. If roll exceeds threshold, selects random failure type by rolling d20. Applies effect: drop weapon, hit self, hit ally, fall prone, or other fumble consequences. Combat round continues after failure resolution.

### Attack Count Assembly

Combat initialization calls blowCount at round start. Function checks if caller is PC or NPC. NPC path retrieves mult field, clamps to 12, returns direct value. PC path calls blowCountSplitter for primary hand and secondary hand separately.

blowCountSplitter checks weapon type and retrieves specialization skill learning. Converts learning to fractional attack bonus. Checks combat mode: adds 0.5 if berserk, adds 1.0 if advanced berserking. Checks status effects: adds 0.5 if hasted, adds 0.5 if celerite. Checks position: multiplies by 0.67 if mounted. Returns fractional attack count for that hand.

Combat round distributes total attacks across attack sequence. Each attack performs full hit detection, damage calculation, and critical check. Multi-attack rounds can trigger multiple crits per round but each uses independent probability roll. Dual wield damage penalty applies to secondary hand attacks. Two-handed specialization applies to primary hand when no secondary weapon equipped and weapon flagged as two-handed.

### Stat Modifier Application

plotStat receives stat value, min output, max output, and optional baseline. Subtracts 5 from stat value. Divides by 200. Applies pow(normalized, 1.4). Multiplies by (maxValue - minValue). Adds minValue. Returns floating point result. Caller converts to appropriate type: float for damage multipliers, int for additive bonuses.

getStatMod calls plotStat with fixed 0.8-1.25 range. Subtracts 1 from result. Multiplies by optional multiplier parameter. Adds 1 back. Returns modifier centered on 1.0 as neutral. Caller multiplies damage or other metric by this value to apply stat effect.

Dexterity bonus computation calls getStatMod for STAT_DEX. Multiplies result by 335. Subtracts 335. Casts to int. Adds result to attackRound and defendRound accumulators. This transformation maps the 0.8-1.25 float range to -67 to +84 integer range matching other combat formula precision.

Strength modifier computation calls getStatMod for STAT_STR or plotStat with same range. Directly multiplies weapon damage by result. Weapon type determines percentage: full for blunt, 0.75 for slash, 0.5 for pierce. This scaling reflects weapon reliance on raw force versus technique.

Karma modifier computation calls plotStat with wider 0.5-2.0 range. Multiplies by 1000 for crit threshold precision. Karma stat below 105 yields probability penalty, above 105 yields bonus. Power curve of 1.4 makes high karma significantly more valuable for crit fishing builds.

## Troubleshooting

### Hit Rates Don't Match Expectations

Verify attackRound and defendRound are accumulating all bonuses correctly. Check discipline learnedness for offense and combat skills. Confirm dexterity stat matches expected value and getStatMod is computing correctly. Test with neutral armor class (AC 0) to isolate offense contribution. Test with stationary opponent to isolate defense contribution. Remember baseline hit chance is 60% at equal modifiers.

Combat mode confusion: defense mode imposes attack penalty, offense mode provides attack bonus. Check that stance matches intended strategy. Berserk provides larger bonus than offense but disables defense.

Position penalty not applying: defendRound multiplies defense by position factor after accumulating bonuses. Sitting or crawling should reduce defense to 50% of standing value. If defense appears unaffected, check position is actually changing and groundfighting skill is not fully trained.

### Damage Too Low or High

Weapon damage: verify weapon base and roll dice match equipment stats. Check strength stat and confirm weapon type is applying correct percentage. Verify weapon skill learning is retrieving actual skill value not defaulting to level × 2. Dual wield should show secondary damage at 30-60% of primary.

Spell damage: confirm classAmt value matches spell balance table. Verify lag cost is correct for spell. Check caster level is using actual character level not skill level. NPC damage should be approximately half PC damage for same spell. Area effect should reduce damage by 25%.

Stat modifier miscalculation: plotStat curve is non-linear. Stat 105 should yield neutral modifier, stat 150 should be noticeably above neutral, stat 200 should be near maximum. If modifiers appear linear, check that power 1.4 is applying correctly.

### Critical Hits Too Rare or Frequent

PC crit rate: baseline karma with no skills should yield ~0.5% crit rate. Full crit_hit skill should approach 3%. If seeing no crits over 100 attacks with trained skill, check karma stat and equipment crit frequency bonuses. If seeing crits every few attacks, check NPC versus PC roll range.

NPC crit rate: should be approximately 10x rarer than PC rate due to 10x larger roll range. If NPCs crit at PC frequency, verify roll range is using 1000000 not 100000.

Critical failure: should be rare at normal karma levels, more common when drunk. If failures occur frequently, check karma stat and intoxication level. Remember failure roll is d300 versus karma × 10, so karma below 15 makes failures common.

### Attack Count Incorrect

Mob attack count: should match mult field capped at 12. If receiving fewer attacks, check mult is set correctly. If receiving more than 12, check for combat mode bonuses being applied to mobs incorrectly.

PC attack count: start with blowCountSplitter base. Add mode bonuses (berserk, advanced berserking). Add status bonuses (haste, celerite). Apply mount penalty if mounted. Each hand tracks separately. If seeing fractional attacks displayed, check rounding in combat message code. Actual attack count should truncate fractions.

Dual wield not applying: secondary hand requires dual wield skill to unlock. Even with skill, secondary attacks appear in separate attack sequence. Check combat log for secondary hand attacks, not just primary damage totals. Verify dual wield penalty (30-60%) is reducing secondary damage appropriately.

### reconcileDamage Death Detection Fails

Using IS_SET_DELETE instead of checking return value == -1. reconcileDamage returns -1 on death, not a DELETE flag. Caller must store return value, check for -1, then construct DELETE_VICT flag if victim pointer was passed as parameter. Never use bitwise operations on reconcileDamage return value for death checking.

Death occurring but not detected: check reconcileDamage is being called with actual damage value not zero. Verify victim hit points are being subtracted correctly. Confirm hit point check is using <= 0 not just < 0. Check for immortality flags on victim preventing death.

### Stat Modifiers Not Applying

plotStat receiving incorrect stat value: verify stat retrieval is using actual current stat including temporary bonuses, not base stat. Check affect system is applying stat modifications before combat formula calls.

getStatMod returning neutral: stat value near 105 yields 1.0 modifier by design. If expecting bonus or penalty, confirm stat is actually above or below baseline. Remember power curve means stat 100 is slightly below neutral, not neutral.

Dexterity bonus computed but not applied: check that attackRound and defendRound are both adding dexterity result to their bonus accumulators. Verify bonus is being added as integer, not truncated before casting.

Strength modifier not scaling damage: confirm getWeaponDam is multiplying damage by strength result. Check that weapon type enum is valid and matching correct case in weapon type switch. Verify strength stat is reading correctly and not stuck at baseline value.
