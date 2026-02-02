---
title: Combat Skills
category: critical
keywords: [combat, skills, bSuccess, specialAttack, getSkillDam, DELETE_VICT, reconcileDamage, ranged, unarmed, lag]
related: [damage-pipeline.md, delete-flags.md, combat-formulas.md, spell-skill-framework.md, position-stance.md]
primary_symbols:
  functions: [bSuccess, specialAttack, getSkillDam, reconcileDamage, shootMeBow, getMonkWeaponDam]
  classes: [TBeing, TBow, TArrow, spellInfo]
  files: [code/misc/combat.cc, code/misc/skill_dam.cc, code/misc/range.cc, code/disc/disc_warrior.cc]
---

## Overview

Combat skills are physical abilities distinct from magical spells. Characters execute special attacks, defensive maneuvers, and tactical actions using bodily prowess rather than mystical forces. Despite this conceptual difference, skills share the unified spellInfo framework with spells for consistent implementation.

Success determination operates in two phases. First, bSuccess evaluates whether the character successfully executes the technique based on training and stats. Second, specialAttack determines whether the executed technique connects against the target's defenses. This separation allows skilled characters to execute complex maneuvers reliably while still facing meaningful opposition from capable defenders.

Damage scales with multiple factors: skill learning percentage, character level, primary stat modifiers, and task difficulty. The lag system prevents skill spam by imposing pulse-based delays proportional to power. Position and stance requirements gate availability—most offensive skills require standing position, while defensive skills may be accessible from varied positions.

The system spans melee combat (warrior bashing, monk strikes, thief assassinations), ranged combat (bow shooting with physics-based trajectories), and unarmed combat (barehand damage with monk specialization). Memory safety relies on DELETE flag propagation when combatants die.

## Patterns

### Two-Phase Success System

Every combat skill evaluates success twice. The bSuccess check determines technique execution based on the character's skill learning, focus stat, karma stat, and task difficulty. A trivial task provides 110% base chance while an impossible task provides only 35%. The character's skill learning percentage (0-100) multiplies this base. Focus and karma stats apply multiplicative modifiers ranging from 0.8x to 1.25x.

Only after successful execution does the skill proceed to specialAttack, which compares attacker offense against defender defense. This phase returns discrete success levels: GUARANTEED_SUCCESS (1000), COMPLETE_SUCCESS (100), PARTIAL_SUCCESS (50), FAILURE (0), or GUARANTEED_FAILURE (-1). The attackRound versus defendRound comparison produces situational modifiers from position advantage, stat bonuses, level difference, and combat mode. A random roll against threshold bands determines the outcome.

This dual-phase approach prevents skilled characters from automatically landing every blow while ensuring they successfully perform the technique itself. A master monk always executes a proper kick but may still miss a highly defensive opponent.

### Three-Tier Target Resolution

Command functions follow a consistent fallback pattern for target selection. When called with an explicit vict parameter, that pointer takes precedence—this enables programmatic skill invocation from AI or triggered effects. If vict is null, the function parses the argument string to find a named target in the room using get_char_room_vis. If parsing fails or no argument was provided, the function defaults to the character's current fight opponent from the fight method.

This pattern enables player commands (kick goblin), programmatic calls from mob AI (doKick with explicit victim), and shorthand combat commands (kick defaults to current opponent). The ownership semantics matter critically: when vict is provided by caller, the caller owns that pointer and must handle DELETE_VICT returns. When the function resolves the victim itself, it owns the pointer and must delete on DELETE_VICT after clearing the flag.

### Success and Fail Branching

After validation and bSuccess checks, skills branch into separate success and fail handlers. The success handler calculates damage using getSkillDam, applies success level modifiers (half damage for PARTIAL_SUCCESS), generates combat messages with act, and calls reconcileDamage to apply the damage. The critical check reconcileDamage return equals -1 for death detection—this is not a DELETE flag, it's a sentinel value.

The fail handler generates failure messages, may impose side effects like balance loss or falling via crashLanding, and calls reconcileDamage with zero damage to initiate combat without harm. Both paths typically return true to apply skill lag even on failure, preventing spam attempts.

Success handlers must immediately return DELETE_VICT when reconcileDamage returns -1. Fail handlers must check crashLanding return for DELETE_THIS before continuing. This branching keeps success and failure logic cleanly separated while maintaining consistent DELETE flag propagation.

### Ownership-Based DELETE Handling

When a skill function receives a vict parameter, the caller owns that pointer. If the skill sets DELETE_VICT in its return value, it must not delete the victim—instead it returns the flag so the caller can handle deletion. This prevents double-free when the caller is iterating over combatants or maintaining references.

When the skill resolves the victim itself via get_char_room_vis or fight, it owns the pointer. On DELETE_VICT, it must delete the victim pointer, null the local variable, and clear the flag with REM_DELETE before returning the remaining flags. This ensures the flag doesn't propagate to code that didn't receive the victim pointer.

The pattern check "if vict return rc" enables correct handling: if the parameter was provided, return immediately with flags intact. Otherwise proceed to local deletion. Always use IS_SET_DELETE for DELETE flag checks, never IS_SET. Always check reconcileDamage return with == -1 for death, never IS_SET_DELETE.

### Damage Scaling Formula

The getSkillDam function multiplies classAmt (skill base power) by lagRounds (lag penalty converted to rounds) by level (character level). This base product then receives multiplicative modifiers: diffModifier from advanced learning (1.0 + advLearning/200), statModifier from the skill's primary stat via getStatMod, and random variance of +/- (level/2).

Skills with higher lag naturally deal more damage since lagRounds scales with the lag penalty. A LAG_4 skill has double the lag rounds of LAG_2, producing double base damage before modifiers. The classAmt values range from 0.5-0.75 for light skills like kick and bash, up to 3.0-4.0 for lethal skills like throatslit.

NPC combatants deal approximately 52% of PC damage for identical skills via a 0.5195 multiplier. This prevents MOBs from overwhelming players with the same abilities while preserving skill balance tuning. Location-specific multipliers apply to certain skills—kick damage multiplies by 2.5x for head hits, 2.0x for solar plexus, 1.5x for shin, 1.0x for side.

### Lag Application and Reduction

Skills impose lag measured in game pulses (0.1 seconds each). The lag enumeration ranges from LAG_0 (instant) through LAG_9 (10.8 seconds). The addSkillLag function retrieves the lag value from discArray for the skill, converts it to pulses via lagAdjust and combatRound, and applies it via setCharFighting.

Death-triggered lag reduction caps lag at LAG_1 when DELETE_VICT is set in the return value. This allows characters to clean up remaining enemies faster after killing one target, improving combat flow. The reduction only applies on successful kills, not failed attempts.

Lag applies even on skill failure unless explicitly prevented. This prevents skill spam—a missed bash still imposes LAG_2 before another bash attempt. The only exception is when bSuccess fails before resource consumption.

## Reference

### Skill Organization by Discipline

Warriors access 19 skills through DISC_SOLDIERING including bash, bodyslam, spin, kick, headbutt, kneestrike, stomp, grapple, deathstroke, disarm, rescue, defend, berserk, switch, riposte, retreat, focused_attack, charge, and slam. Damage types span blunt (most), pierce/slash (spin), weapon-based (deathstroke, riposte, focused_attack, charge), or none (utility skills). Lag ranges LAG_0 to LAG_4.

Monks access 13 skills through DISC_MINDBODY including kick_monk, springleap, hurl, defenestrate, chi, quivering_palm, feign_death, iron_roots, shoulder_throw, bonebreak, mindbody, blur, and meditate. Most deal blunt damage with LAG_2 to LAG_4. Chi, quivering_palm, and utility skills have special mechanics.

Thieves access 6 skills through DISC_MURDER including backstab, throatslit, garrotte, stabbing, poisoning, and ambush. Pierce damage dominates with LAG_2 to LAG_4. These skills emphasize stealth positioning and lethal damage multipliers.

Deikhans access 6 skills across martial disciplines including charge, smite, shock_cavalry, harm_deikhan, deathstroke_deikhan, and defend_deikhan. Holy/weapon-based damage with LAG_0 to LAG_3.

Rangers access transfix through DISC_NATURE, a LAG_3 pierce skill that pins opponents with arrows.

### spellInfo Structure Fields

The typ field identifies skill class type using skillUseClassT enum: SKILL_WARRIOR, SKILL_MONK, SKILL_THIEF, SKILL_DEIKHAN, SKILL_RANGER, or SKILL_SHAMAN. The disc field specifies primary discipline from discNumT enum. The assDisc field specifies associated discipline for do-learning, typically matching disc.

The modifierStat field determines which stat affects success and damage: STAT_STR for strength-based skills, STAT_DEX for dexterity-based, STAT_AGI for agility-based. The task field rates difficulty using taskDiffT: TASK_TRIVIAL through TASK_IMPOSSIBLE affects bSuccess base chance. The lag field specifies lag penalty using lag_t enum.

The minPosition field gates skill usage by position: POSITION_STANDING for most combat skills, POSITION_FIGHTING for active combat skills, POSITION_RESTING for meditation skills. The targets field combines bitflags: TAR_VIOLENT marks combat skills, TAR_FIGHT_VICT requires fighting the target, TAR_CHAR_ROOM allows room targets.

Resource costs (mana, lifeforce, pray) remain zero for most skills since they use physical prowess rather than magical or divine energy. Learning rates (START_30, LEARN_2, etc.) control initial availability and improvement speed. Component flags (COMP_GESTURAL) indicate execution requirements.

### Task Difficulty Modifiers

TASK_TRIVIAL provides 110% success modifier for tutorial skills teaching basic mechanics. TASK_EASY provides 100% for simple skills with minimal execution complexity. TASK_NORMAL provides 90% for standard combat skills comprising most warrior, monk, and thief abilities. TASK_DIFFICULT provides 80% for advanced techniques requiring significant training.

TASK_DANGEROUS provides 70% for high-risk maneuvers with serious failure consequences. TASK_HOPELESS provides 50% for desperation moves attempted when other options are exhausted. TASK_IMPOSSIBLE provides 35% for near-impossible feats requiring exceptional circumstances.

The modifier multiplies with skill learning percentage and stat modifiers to produce final bSuccess chance. A master (100% learning) with excellent focus (1.2x) performing a TASK_NORMAL skill achieves roughly 108% chance before random roll.

### Lag Enumeration Values

LAG_0 equals 0 pulses (0.0 seconds) for instant skills with no delay. LAG_1 equals 12 pulses (1.2 seconds) for very fast skills or reduced lag from death-triggered reduction. LAG_2 equals 24 pulses (2.4 seconds) for standard combat skills like kick, bash, and headbutt.

LAG_3 equals 36 pulses (3.6 seconds) for powerful attacks like bodyslam and backstab. LAG_4 equals 48 pulses (4.8 seconds) for very powerful skills like spin and throatslit. LAG_5 equals 60 pulses (6.0 seconds) for devastating abilities. LAG_9 equals 108 pulses (10.8 seconds) for ultimate abilities with massive impact.

### Ranged Combat Classes

The TBow class in obj_bow.h and obj_bow.cc represents bows, crossbows, slings, and blowguns. The arrowType property specifies ammunition compatibility using integer codes. The flags property holds bow state using bitflags: BOW_STRING_BROKE indicates broken bowstring requiring repair, BOW_CARVED/BOW_SCRAPED/BOW_SMOOTHED track crafting progress. The max_range property limits shooting distance in rooms.

The TArrow class in obj_arrow.h and obj_arrow.cc represents arrows, quarrels, darts, and sling stones. The arrowType property must match bow arrowType for compatibility. Eight standard types exist: hunting arrow (0), fighting arrow (1), squabble quarrel (2), common quarrel (3), sniper blowdart (4), common blowdart (5), heavy sling ammo (6), common sling ammo (7).

The arrowHead and arrowHeadMat properties affect damage calculation. The arrowFlags property tracks arrow state. The trap_level and trap_dam_type properties enable trapped arrows that trigger on impact.

### Ranged Combat Skills

SKILL_RANGED_PROF (569) provides basic ranged competency available to all classes. Minimum 10 skill allows shooting—below 10, arrows fall harmlessly. The skill adds 0-50% damage modifier as proficiency/2. SKILL_RANGED_SPEC (568) provides advanced ranged mastery for Warriors and Rangers, adding another 0-50% damage modifier. Combined, 100% in both skills doubles base arrow damage.

SKILL_FAST_LOAD increases arrows fired per round. The shootMeBow function calculates nattacks starting at 1.0, adding max(0, SKILL_FAST_LOAD/100) plus max(0, SKILL_RANGED_SPEC/100). A character with 100% in both fires 3 arrows per round before running out of loaded ammunition.

### Unarmed Combat Skills

SKILL_BAREHAND_PROF (563) provides basic unarmed competency for all classes, yielding 1-3 damage without monk skills. SKILL_BAREHAND_SPEC (567) provides advanced unarmed mastery for monks. SKILL_KUBO scales monk barehand damage using sqrt formula: value equals 3.0 * SKILL_KUBO / 10.0 clamped to 0-50, then weapDam equals 6.0 * sqrt(value) / 2.0.

At 0% KUBO skill, monks deal 1-2 damage. At 33% KUBO (value 10), roughly 9 damage. At 66% KUBO (value 20), roughly 13 damage. At 100% KUBO (value 30), roughly 16 damage. The balanceCorrectionForLevel multiplier then scales this base for character level, followed by +/- 10% random flux.

SKILL_IRON_FIST adds 0-8.3% damage bonus (skill/1200) when hands are ungloved. SKILL_VOPLAT adds 0-10% damage bonus (skill/1000). All barehand damage multiplies by stats.barehand_damage_mod (default 0.36), yielding 36% of calculated damage. This global modifier prevents barehand from competing with crafted weapons.

### Position and Stance Effects

Most offensive skills require POSITION_STANDING, checked via getPosition less than POSITION_STANDING. Exceptions include SKILL_FEIGN_DEATH (any position), SKILL_STOMP (target must be prone), and SKILL_CHI (usable while sitting for meditation).

Combat modes affect skill selection and damage. ATTACK_NORMAL allows all skills with 1.0x damage. ATTACK_DEFENSE prefers defensive skills with 0.9x damage. ATTACK_OFFENSE prefers offensive skills with 1.1x damage. ATTACK_BERSERK limits skill selection to weighted choices with 1.2x damage.

Berserking restricts skills to specific set with weights: bash (2), headbutt (3), bodyslam (2), grapple (1), slam (3), deathstroke (1). Higher weights increase selection probability during berserk rage.

### Source File Organization

Command files in code/cmd/ implement individual skills: cmd_kick.cc handles doKick/kick/kickSuccess/kickFail, cmd_bash.cc handles doBash/bash/bashSuccess/bashFail, cmd_trip.cc handles doTrip/trip/tripSuccess/tripFail, cmd_headbutt.cc handles doHeadbutt/headbutt/headbuttHit/headbuttMiss, cmd_grapple.cc handles doGrapple/grapple, cmd_slam.cc handles doSlam/slamSuccess/slamFail, cmd_disarm.cc handles doDisarm/disarm, cmd_stomp.cc handles doStomp/stomp/stompHit/stompMiss, cmd_deathstroke.cc handles doDeathstroke/deathstrokeSuccess/deathstrokeFail.

Discipline files in code/disc/ group class-specific skills: disc_warrior.cc contains DISC_SOLDIERING implementations, disc_monk_mindbody.cc contains DISC_MINDBODY implementations, disc_thief_murder.cc contains DISC_MURDER implementations, disc_deikhan_martial.cc contains DISC_MARTIAL implementations, disc_ranger_nature.cc contains DISC_NATURE implementations.

Shared combat code lives in code/misc/: combat.cc implements specialAttack and attack resolution, skill_dam.cc implements getSkillDam damage calculation, range.cc implements shooting and throwing mechanics. Bow and arrow implementations live in code/obj/obj_bow.cc and code/obj/obj_arrow.cc. Archer MOB AI lives in code/spec/spec_mobs_archer.cc.

## Implementation

### bSuccess Calculation

The bSuccess function retrieves task difficulty modifier via getSkillDiffModifier, returning 110% for TASK_TRIVIAL down to 35% for TASK_IMPOSSIBLE. This limit multiplies by skillValue divided by 100 to incorporate learning percentage. Focus stat modifier from getStatMod(STAT_FOC) multiplies next, ranging 0.8x to 1.25x. Karma stat modifier from plotStat(STAT_CURRENT, STAT_KAR, 0.9, 1.125, 1.0) multiplies last.

The final limit converts to integer iLimit and compares against random number from 0 to 99. If the random number is less than iLimit, bSuccess returns true. Otherwise false. The implementation lives in disc_warrior.cc and other discipline files, shared across skills within each discipline.

### specialAttack Mechanics

The specialAttack function calculates mod by subtracting defendRound(target) from attackRound(target). This represents attacker offense minus defender defense. Situational modifiers add to mod: position bonus ranges -5 to +3 based on relative position advantage, stat modifiers vary by skill's primary stat, level difference adds +/- based on attacker vs defender level, combat mode adds -10 to +10 for ATTACK_DEFENSE vs ATTACK_OFFENSE.

A random roll from 1 to 100 compares against threshold bands. If roll less than or equal to 50 minus mod, return GUARANTEED_SUCCESS (1000). Else if roll less than 80 minus mod, return PARTIAL_SUCCESS (50). Else if roll equals exactly 100, return GUARANTEED_FAILURE (-1). Else return FAILURE (0).

Higher mod values (better attacker advantage) expand the success bands, making high rolls more likely to succeed. The implementation lives in combat.cc as part of core combat resolution.

### getSkillDam Formula

The getSkillDam function multiplies classAmt by lagRounds by level to establish baseDamage. The classAmt parameter varies by skill: kick uses 0.5-0.75, bash uses 0.5-0.75, headbutt uses 0.75-1.0, grapple uses 1.0-1.25, bodyslam uses 1.25-1.5, deathstroke uses 2.0-2.5, backstab uses 2.0-3.0, throatslit uses 3.0-4.0.

The lagRounds value derives from skill's lag field divided by 12: LAG_2 (24 pulses) yields 2 rounds, LAG_3 (36 pulses) yields 3 rounds, LAG_4 (48 pulses) yields 4 rounds. This ensures high-lag skills deal proportionally more damage.

The diffModifier calculates as 1.0 plus advLearning divided by 200, where advLearning represents advanced skill mastery beyond basic learning. The statModifier retrieves from getStatMod using the skill's modifierStat. Random variance adds or subtracts level divided by 2.

For NPC attackers, the final damage multiplies by 0.5195 to yield approximately 52% of PC damage. The implementation lives in skill_dam.cc.

### Ranged Shooting Flow

The doShoot function validates the character has a bow equipped in primary hand position via equipment[getPrimaryHold()]. It checks SKILL_RANGED_PROF exceeds 0—characters without the skill cannot shoot. The function parses direction and target from argument using get_char_vis_direction, which scans rooms in the specified direction up to specified distance, counting targets matching the name.

The function then delegates to the bow's shootMeBow method with shooter, target, target count, direction, and shoot distance. The bow verifies it contains a loaded arrow by checking stuff.empty() and dynamic_cast to TArrow. The bow checks shoot_dist does not exceed getMaxRange(). The bow performs bowstring break check by testing random(0, getStructPoints())—on 0, the string snaps and BOW_STRING_BROKE flag sets.

The bow calculates arrows per round starting at 1.0, adding SKILL_FAST_LOAD/100 and SKILL_RANGED_SPEC/100. A while loop fires arrows while nattacks exceeds 0 and targ is not null. Each iteration calls throwThing with arrow, direction, starting room, target pointer, shoot distance, max distance, and shooter. The bow auto-reloads by calling findArrow on the shooter and bloadBowArrow if ammunition exists.

After firing, the shooter receives 4-round lag via setWait(combatRound(4)). The implementation spans range.cc for doShoot and obj_bow.cc for shootMeBow.

### Arrow Hit Resolution

The TBaseWeapon::catchSmack function determines damage type based on projectile: TArrow objects use DAMAGE_ARROWS, TYPE_SHOOT, or TYPE_CANNON depending on weapon type. The function calls specialAttack(target, SKILL_RANGED_PROF) to check if target dodges. On GUARANTEED_FAILURE or false return, the target dodges and arrow misses.

For non-blunt weapons with sharp edges, the function checks random(1, 100) less than getCurSharp() to determine embedding. On success, stickIn embeds the arrow in the victim at hit location. The function retrieves base damage from damageLevel() on the arrow object.

The get_range_actual_damage function applies skill modifiers. For TArrow projectiles, q starts at 100, adds SKILL_RANGED_PROF/2, adds SKILL_RANGED_SPEC/2, then multiplies damage by q/100. For thrown objects, q equals SKILL_RANGED_PROF directly. The maximum of modified damage and 1 ensures minimum damage.

If the arrow isPoisoned(), applyPoison applies poison effects. If the arrow has trap_dam_type not equal DOOR_TRAP_NONE, triggerArrowTrap executes trap damage. Finally, reconcileDamage applies calculated damage—return of -1 produces DELETE_VICT flag.

The implementation lives in obj_base_weapon.cc for catchSmack and range.cc for get_range_actual_damage.

### Throwing Physics

The throwMe function calculates acceleration using plotStat(STAT_CURRENT, STAT_BRA, 500.0, 5000.0, 2500.0) to derive force based on brawn stat, then divides by max(3.0, getWeight()) to account for object mass. Initial velocity v0 equals 0.2 times acceleration.

Trajectory angle derives from plotStat(STAT_CURRENT, STAT_FOC, 0, 45, 5) to vary angle from 0 to 45 degrees based on focus stat. The range calculation uses projectile motion formula: mult equals v0 squared divided by 32.0 times sin(2 * angle in radians). The max_distance equals mult divided by 100 for outdoor or 50 for indoor, with minimum of 1.

The function calls throwThing with calculated max_distance to execute the throw. Indoor environments halve throwing distance to reflect confined spaces. The implementation lives in range.cc.

### Monk Barehand Damage

The getMonkWeaponDam function first checks doesKnowSkill(SKILL_KUBO). Without KUBO, return random(1, 2) for minimal damage. With KUBO, calculate value as 3.0 times getSkillValue(SKILL_KUBO) divided by 10.0, clamped to 0.0-50.0 range.

Calculate weapDam as 6.0 times sqrt(value) divided by 2.0. Multiply weapDam by balanceCorrectionForLevel(GetMaxLevel()) to scale for character level. Convert to integer wepDam. Calculate flux as wepDam divided by 10, then set wepDam to random(wepDam - flux, wepDam + flux) for +/- 10% variance. Ensure minimum 1.

Retrieve statDam from getStrDamModifier() for strength scaling. If doesKnowSkill(SKILL_IRON_FIST) and both WEAR_HAND_R and WEAR_HAND_L slots are empty, add getSkillValue(SKILL_IRON_FIST) divided by 1200.0 to statDam. If doesKnowSkill(SKILL_VOPLAT), add getSkillValue(SKILL_VOPLAT) divided by 1000.0 to statDam.

Multiply wepDam by statDam to get base dam. Multiply dam by stats.barehand_damage_mod (default 0.36). Return final damage. The implementation lives in combat.cc as static function called from attack resolution.

### Archer MOB Procedure

The archer spec proc scans character inventory for TBow objects via getBows(), iterating to find a bow with matching ammunition via autoGetAmmo(bow). The function checks if the MOB Hates any target—if the hated target is in same room, the MOB calls doFlee to create distance for ranged combat.

The proc iterates all directions from MIN_DIR to MAX_DIR minus 1. For each direction, it iterates range from 1 to MAX_RANGE (3 rooms). The proc uses exitDir and canGoDirection to verify traversable paths. It scans each distant room for PC characters matching hate conditions.

Upon finding a target, the proc calls bloadBowArrow to load ammunition, then doShoot with formatted string containing direction and target name. The MOB maintains distance by fleeing if targets approach melee range. The implementation lives in spec_mobs_archer.cc.

## Troubleshooting

### reconcileDamage Returns -1 Not DELETE_VICT

When skills call reconcileDamage and check IS_SET_DELETE(rc, DELETE_VICT), the check never triggers on death. The reconcileDamage function returns actual damage dealt on success (0 or positive integer) and uses -1 as a magic sentinel value for victim death. This sentinel predates the DELETE flag system and remains for compatibility.

Always check reconcileDamage return value with == -1 comparison, not IS_SET_DELETE. Upon detecting -1, immediately return DELETE_VICT to propagate the death flag to caller. Never continue execution after reconcileDamage returns -1 since the victim pointer is now invalid.

### Using IS_SET Instead of IS_SET_DELETE

The DELETE flags (DELETE_THIS, DELETE_VICT, DELETE_ITEM) use a different bit pattern than standard flags. The IS_SET macro checks bits in the low range suitable for object flags and affect flags. The DELETE flags occupy high bits requiring IS_SET_DELETE macro.

Using IS_SET to check DELETE_THIS or DELETE_VICT produces false negatives—the flag is set but IS_SET returns false. This causes continued execution after object deletion, leading to use-after-free crashes. Always use IS_SET_DELETE when checking DELETE_THIS, DELETE_VICT, or DELETE_ITEM.

### Deleting Victim Without Ownership Check

When a skill function receives a vict parameter provided by caller, the caller owns that pointer and maintains references. If the skill deletes the victim directly without returning DELETE_VICT to caller, the caller's pointer becomes dangling. Subsequent caller access produces use-after-free crash.

The ownership pattern requires checking if vict parameter was provided. If so, return rc with DELETE_VICT flag intact—let caller handle deletion. Only delete victim locally when the function resolved the victim itself via get_char_room_vis or fight. After local deletion, clear the flag with REM_DELETE(rc, DELETE_VICT) before returning.

### Not Clearing DELETE Flags After Local Deletion

After deleting an object locally and clearing the pointer to null, the DELETE flag remains set in rc return value. If this rc propagates to code that didn't receive the pointer, that code may attempt to delete the already-freed memory. This produces double-free crash.

Always call REM_DELETE(rc, DELETE_VICT) or REM_DELETE(rc, DELETE_ITEM) after local deletion before returning rc. This clears the flag bit to indicate the deletion was handled. Only propagate DELETE flags when returning pointers to code that owns them.

### Continuing After DELETE_THIS Return

When a function calls another function that may return DELETE_THIS, the caller's "this" pointer may be invalid. Continuing execution with member access or method calls produces use-after-free crash. The DELETE_THIS flag signals the object was deleted by the callee.

Immediately check IS_SET_DELETE(rc, DELETE_THIS) after any call that may delete the current object. Upon detection, return DELETE_THIS without accessing any member variables or calling any methods. The return must propagate up the call stack until reaching code that doesn't use the deleted object.

### Ranged Skill Check Below Minimum

When characters attempt shooting without SKILL_RANGED_PROF or with skill below 10, arrows should fall harmlessly rather than firing. The doShoot function checks getSkillValue(SKILL_RANGED_PROF) greater than 0, but shootMeBow also checks for minimum 10. Between 1-9 skill, the arrow loads but fails to fire properly.

Always check SKILL_RANGED_PROF against minimum threshold 10 before allowing shooting actions. Characters with 0 should receive "you don't know how to shoot" message. Characters with 1-9 should receive messages about clumsy attempts that fail to launch arrows properly.

### Ignoring Bowstring Break

When random(0, getStructPoints()) returns 0, the bowstring breaks or bow shatters. The BOW_STRING_BROKE flag sets and subsequent shooting attempts should fail. If shootMeBow continues after string break, the bow fires without a string, breaking physics simulation.

Check for bowstring break test failure before arrow launch. Set BOW_STRING_BROKE flag and return FALSE immediately. The character should receive message about string snapping with twang sound. The bow requires repair via bowyer or item replacement before functional shooting.

### Using Wrong Monk Damage Function

When calculating barehand damage for monks, using the standard ::number(1, 3) formula ignores SKILL_KUBO scaling. Monks deal damage comparable to non-combat classes rather than leveraging their unarmed specialization. This makes barehand combat unviable for monks despite conceptual emphasis.

Check doesKnowSkill(SKILL_KUBO) before damage calculation. Route to getMonkWeaponDam when true, using standard formula when false. Apply SKILL_IRON_FIST and SKILL_VOPLAT bonuses only within monk damage path. Ensure stats.barehand_damage_mod multiplies final result to maintain balance against weapons.

### Berserk Mode Skill Restriction Violation

While berserking (affectedBySpell(SKILL_BERSERK)), characters should select skills from weighted list: bash (2), headbutt (3), bodyslam (2), grapple (1), slam (3), deathstroke (1). If all skills remain available, berserking provides damage bonus without tactical limitation, breaking risk/reward balance.

During berserk rage, validate skill selection against allowed set before execution. Use weighted random selection during AI decisions. Higher weights increase probability: headbutt and slam at weight 3 appear three times as often as grapple at weight 1. Prevent defensive skills and utility skills during rage.

### Position Requirement Bypass

Most combat skills require POSITION_STANDING, checked via getPosition() less than POSITION_STANDING. If validation occurs after resource consumption or lag application, characters can use skills from sitting/resting position. This enables prone offense without intended vulnerability.

Check position requirements in canXXX validation function before any state modification. Return false with optional message when position insufficient. Only exceptions are skills explicitly designed for other positions: SKILL_FEIGN_DEATH (any position), SKILL_CHI (sitting for meditation), SKILL_STOMP (attacker standing, victim prone).

### Missing Combat Mode Damage Modifier

Combat modes modify damage output: ATTACK_NORMAL (1.0x), ATTACK_DEFENSE (0.9x), ATTACK_OFFENSE (1.1x), ATTACK_BERSERK (1.2x). If damage calculation omits mode multiplier, defensive characters deal full damage while receiving defensive bonuses, and berserking provides tactical restriction without damage compensation.

Apply getCombatMode() check during damage calculation in getSkillDam or skill-specific damage functions. Multiply final damage by mode-specific modifier. This ensures defensive stance reduces damage output to balance increased survivability, and berserk increases damage output to balance skill restriction and control loss.
