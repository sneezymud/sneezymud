---
title: Position and Stance System
category: important
keywords: [positionTypeT, attack_mode_t, updatePos, attackRound, defendRound, getCombatMode, POSITION_FIGHTING, ATTACK_BERSERK, SKILL_GROUNDFIGHTING]
related: [rest-recovery.md, combat-formulas.md, task-system.md]
primary_symbols:
  functions: [updatePos, attackRound, defendRound, specAttackMod, getCombatMode, setCombatMode, getPosition, setPosition, isFlying]
  classes: [TBeing, commandInfo]
  files: [code/code/misc/combat.cc, code/code/misc/being.cc, code/code/misc/movement.cc, code/code/misc/parse.cc]
---

## Overview

Why can't sleeping characters fight back? How does your stance affect combat effectiveness? The position and stance system governs what actions characters can take and how well they perform in combat.

**Position** represents physical state - from dead through unconscious to sleeping, resting, sitting, standing, mounted, or flying. Position determines action availability and combat effectiveness. A sleeping character can't attack; a mounted warrior gains bonuses; a resting fighter suffers penalties.

**Stance** (attack mode) represents tactical choice within combat - defensive, balanced, offensive, or berserk. Stance modifies attack frequency and defensive capability, trading aggression for protection.

These systems work together: position gates what you can do, stance modifies how well you do it. The hierarchy flows from least capable (dead) to most capable (flying), with clear breakpoints between incapacitated, resting, combat-engaged, and mobile states.

Common scenarios:
- Taking damage reduces HP below zero: `updatePos()` transitions through stunned, incapacitated, mortally wounded, to dead based on HP thresholds
- Player types "rest": command changes position to `POSITION_RESTING`, enabling faster regeneration but blocking movement
- Mounted combat: position changes to `POSITION_MOUNTED`, granting attack and defense bonuses
- Berserking: stance switches to `ATTACK_BERSERK`, maximizing attack frequency while preventing flee and most commands

The position hierarchy is strictly ordered numerically, enabling simple greater-than/less-than checks for capability gating.

## Patterns

### Position Management

**DO check minimum position before allowing actions.** Commands and spells define `minPosition` requirements. The parser enforces this automatically for commands; spell/skill code must check explicitly.

**DO use `updatePos()` after HP changes.** When damage or healing modifies hit points, call `updatePos()` to transition incapacitated states automatically. Never manually set `POSITION_STUNNED` or `POSITION_MORTALLYW` - let `updatePos()` handle HP-based transitions.

**DON'T set position without considering mount state.** If character is riding, position must be `POSITION_MOUNTED`. Dismounting must restore appropriate ground position. The mount/dismount functions handle this - don't bypass them.

**DON'T assume position remains stable during combat.** Damage can drop a standing fighter to stunned mid-round. Always re-check position after calling functions that deal damage or modify HP.

### Combat Mode Safety

**DO reset berserk mode when combat ends.** `ATTACK_BERSERK` prevents fleeing and blocks most commands. If combat terminates without clearing berserk, player becomes stuck. The combat termination logic handles this automatically.

**DON'T allow mode changes during command restrictions.** Paralysis and stun effects prevent voluntary actions, including stance changes. Check `AFF_PARALYSIS` and `AFF_STUNNED` before accepting stance commands.

**DO validate mode values before casting.** User input may provide out-of-range integers. Check against valid `attack_mode_t` enum range before casting to prevent undefined behavior.

### Movement and Position

**DO apply position-based movement costs.** Flying divides movement cost by four; crawling adds substantial penalties. The `doMove()` family calculates these automatically - don't bypass movement functions.

**DON'T allow ground-only actions while flying.** Earth spells, foraging, camping, and similar abilities require ground contact. Check `isFlying()` before allowing terrain-dependent actions.

**DO handle crawling limb checks correctly.** Crawling movement costs depend on injured limbs - both legs hurt triggers crawling, additional arm damage increases cost further. Use limb damage flags to compute proper movement costs.

### Ground Fighting Mitigation

**DO scale penalties by `SKILL_GROUNDFIGHTING` when position is below standing.** Characters with groundfighting training suffer reduced penalties from prone positions. Apply the skill-based reduction formula to both attack and defense modifiers.

**DON'T apply groundfighting to positions >= `POSITION_STANDING`.** The skill only mitigates disadvantaged positions (resting, sitting, crawling). Standing, mounted, and flying positions don't trigger groundfighting calculations.

### Anti-Patterns

**NEVER ignore position in combat calculations.** Position modifiers significantly affect `attackRound()` and `defendRound()`. Mounted vs resting can swing combat by dozens of points per level. Flying provides the highest bonuses; incapacitated positions negate all bonuses.

**NEVER allow commands with `minPosition >= POSITION_CRAWLING` while fighting.** Combat-engaged characters cannot perform most non-combat actions. The parser blocks these automatically based on position checks.

**NEVER set berserk mode outside combat initiation.** Berserk prevents fleeing and essential commands. Only set `ATTACK_BERSERK` when combat is confirmed active and character can sustain the commitment.

**NEVER forget to check portal entry restrictions during berserk.** Berserking characters cannot enter portals while fighting. Attempting portal traversal while berserking in combat should fail with appropriate messaging.

## Reference

### Position Types (`positionTypeT`)

| Value | Position | Combat Modifier | Notes |
|-------|----------|-----------------|-------|
| 0 | `POSITION_DEAD` | Negates all bonus | HP <= -11 |
| 1 | `POSITION_MORTALLYW` | Negates all bonus | HP -6 to -10 |
| 2 | `POSITION_INCAP` | Negates all bonus | HP -3 to -5 |
| 3 | `POSITION_STUNNED` | Negates all bonus | HP <= 0 |
| 4 | `POSITION_SLEEPING` | Negates all bonus | Voluntary rest |
| 5 | `POSITION_RESTING` | -(level/3 + 1) | Moderate regen |
| 6 | `POSITION_SITTING` | -(level/4 + 1) | Minimal regen |
| 7 | `POSITION_ENGAGED` | No modifier | In combat range |
| 8 | `POSITION_FIGHTING` | No modifier | Actively fighting |
| 9 | `POSITION_CRAWLING` | No modifier | Limb damage |
| 10 | `POSITION_STANDING` | No modifier | Default mobile |
| 11 | `POSITION_MOUNTED` | +(level/4 + 1) | Riding bonus |
| 12 | `POSITION_FLYING` | +(level/3 + 1) | Maximum bonus |

### Attack Modes (`attack_mode_t`)

| Value | Mode | Description |
|-------|------|-------------|
| 0 | `ATTACK_NORMAL` | Balanced attack/defense |
| 1 | `ATTACK_DEFENSE` | Reduced attacks, increased defense |
| 2 | `ATTACK_OFFENSE` | Increased attacks, reduced defense |
| 3 | `ATTACK_BERSERK` | Maximum attacks, prevents flee/commands |

### Special Attack Modifiers (`specAttackMod`)

| Position | Modifier |
|----------|----------|
| Resting | -5 |
| Sitting | -3 |
| Crawling | -1 |
| Standing/Engaged/Fighting | 0 |
| Mounted | +2 |
| Flying | +3 |

### Movement Cost Multipliers

| Condition | Effect |
|-----------|--------|
| Flying | Divide by 4 (min 1) |
| Levitating | Divide by 4 (min 5) |
| Crawling (both legs) | +20 base |
| Crawling + injured arm | +40 total |
| Haste/Accelerate | Divide by 2 |

### Primary Functions

| Function | Location | Purpose |
|----------|----------|---------|
| `updatePos` | combat.cc | Auto-adjust position by HP |
| `attackRound` | combat.cc | Calculate attack bonus with position |
| `defendRound` | combat.cc | Calculate defense bonus with position |
| `specAttackMod` | combat.cc | Special attack position modifier |
| `getCombatMode` | being.h | Retrieve current stance |
| `setCombatMode` | being.h | Change combat stance |
| `getPosition` | being.h | Retrieve current position |
| `setPosition` | being.h | Directly set position |
| `isFlying` | being.cc | Check for flying position |

## Implementation

### Position Enforcement Flow

Commands declare minimum position via `commandInfo::minPosition`. When the parser processes input, it retrieves the command's requirement and compares against the character's current position via `getPosition()`. If position is insufficient, the parser generates position-specific error messages and blocks execution.

The position check occurs in `parse.cc` after command lookup but before execution. Messages vary by position: "You cannot do that while dead!", "You cannot do that while stunned!", etc. The blocking prevents execution entirely - no partial processing occurs.

Special cases: commands requiring `minPosition >= POSITION_CRAWLING` additionally check fighting status. Combat-engaged characters cannot use these commands regardless of actual position value. Paralysis and stun effects overlay additional restrictions, blocking commands that require `> POSITION_STUNNED` even if position value is higher.

### HP-Driven Position Updates

The `updatePos()` function enforces HP-to-position mapping. When called after HP modification, it checks threshold ranges and sets position automatically:
- HP <= -11: death
- HP -10 to -6: mortally wounded
- HP -5 to -3: incapacitated
- HP <= 0: stunned
- HP > 0: no automatic change (preserves current position)

Recovery behavior differs by state. Characters healing from stunned automatically sit up. Those recovering from mortally wounded or incapacitated transition through stunned first, requiring additional healing to sit. Mounted characters recover directly to `POSITION_MOUNTED` if their mount still exists. Paralyzed characters remain stunned even when healed above zero HP.

The function lives in combat.cc and is called after every HP modification - damage, healing, regeneration ticks, etc. It never elevates position beyond sitting; standing/moving requires explicit player action.

### Combat Modifier Calculation

Both `attackRound()` and `defendRound()` apply identical position-based scaling. The functions check position value and modify attack/defense totals:

Incapacitated states (dead through sleeping) zero out all attack bonuses from stats and equipment. The character's attack value becomes base only.

Resting applies `-(level/3 + 1)` penalty. Sitting applies `-(level/4 + 1)`. A level 30 resting fighter loses 11 points; sitting loses 8.

Engaged, fighting, crawling, standing apply no modifier. These are baseline positions.

Mounted adds `+(level/4 + 1)`. Flying adds `+(level/3 + 1)`. A level 30 mounted fighter gains 8 points; flying gains 11.

Groundfighting skill reduces penalties when position is below standing. The formula: `val = val * (100 - skillValue) / MAX_SKILL_LEARNEDNESS`. A character with 80% groundfighting reduces a -11 penalty to -2.2. The skill has no effect on standing or better positions - only on disadvantaged states.

These modifiers apply symmetrically to both attacker and defender, so position advantage has double impact. A flying attacker vs resting defender gains the sum of both modifiers.

### Special Attack Modifiers

The `specAttackMod()` function provides smaller numeric adjustments used in specific combat calculations. Unlike `attackRound()` which scales by level, these are flat values: resting -5, sitting -3, crawling -1, mounted +2, flying +3.

These feed into to-hit calculations and certain special attacks. They're separate from the larger `attackRound()` modifiers to allow fine-grained tuning of specific mechanics without disturbing overall combat balance.

### Movement Cost Computation

Movement point consumption calculates base cost from terrain type, then applies position and effect multipliers. The calculation occurs in movement.cc during `doMove()` and related functions.

Flying position divides total cost by four, minimum one move point. Levitation divides by four, minimum five. Both allow effortless traversal of difficult terrain.

Crawling position triggers when both legs are hurt beyond standing capability. This adds 20 base movement points to every move. If one arm is also injured, the penalty doubles to 40 - crawling with only one functional limb is extremely slow.

Haste and accelerate effects divide final cost by two. These stack multiplicatively with flying - a hasted flyer pays one-eighth normal cost.

The movement functions check position via `getPosition()` and limb status via damage tracking flags, computing final cost as: `(base_cost + crawl_penalty) / flying_divisor / haste_divisor`, enforcing minimums where applicable.

### Attack Mode Effects

Attack modes modify combat behavior through several mechanisms. Defense mode reduces attack opportunities in the combat loop - fewer attacks are attempted per round. It also applies defensive bonuses in to-hit calculations.

Offense mode increases attack opportunities - more attacks attempted per round - while reducing defensive bonuses. The tradeoff is direct: aggression for protection.

Berserk mode maximizes attack frequency to the extreme. It also sets behavioral flags preventing fleeing. Command restriction checks detect `ATTACK_BERSERK` and block most actions. Portal entry code checks berserk mode and combat status, preventing teleportation while berserking in combat.

When combat ends, termination logic checks for berserk mode and resets to normal stance. This prevents players from remaining stuck in command-restricted state after fight completion.

The mode is stored per-character and retrieved via `getCombatMode()`. Changes occur through `setCombatMode()` after validating the requested mode is within valid enum range.

### Spell and Skill Position Filtering

Disciplines (spells and skills) define `minPosition` in their `discArray` entries. Casting/use code checks this requirement before execution, identical to command position checks.

Flying imposes additional restrictions beyond simple position value. Earth-based spells require ground contact - the casting code checks `isFlying()` and sector type. Nature abilities like camping and foraging explicitly reject flying positions.

Some skills cannot target flying creatures. Hurl, shoulder throw, defenestrate, and bone break all require the target to be grounded. Their targeting logic checks victim's position and rejects attempts against `POSITION_FLYING`.

The position filtering occurs after command parsing but before resource consumption (mana, moves, etc.), ensuring players don't pay costs for actions they cannot complete.

### Position Display

The `position_types[]` array in constants.cc provides human-readable position names: "Dead", "Mortally wounded", "Incapacitated", "Stunned", "Sleeping", "Resting", "Sitting", "Engaged", "Fighting", "Crawling", "Standing", "Mounted", "Flying".

These strings are indexed by position enum value and used in who lists, look descriptions, and error messages. The array is null-terminated with "\n" sentinel for iteration safety.

## Troubleshooting

### Character Cannot Execute Commands After Combat

**Symptom:** Player types commands but receives "You can't do that!" messages after fight ends.

**Likely Cause:** Stuck in `ATTACK_BERSERK` mode. Combat termination failed to reset stance.

**Verification:** Check combat mode via `getCombatMode()`. If returns `ATTACK_BERSERK` but character has no `ch->fighting`, mode reset failed.

**Fix:** Ensure combat termination code calls `setCombatMode(ATTACK_NORMAL)` when transitioning out of combat. Check for early returns that skip the cleanup.

### Movement Costs Incorrect for Flying Characters

**Symptom:** Flying characters pay full terrain movement costs instead of reduced amounts.

**Likely Cause:** Movement calculation not checking `isFlying()` or applying divisor incorrectly.

**Verification:** Log position value and final movement cost. Flying should always divide by four, minimum one point.

**Fix:** Ensure `doMove()` calculates: `cost = (base_cost + penalties) / 4` when `getPosition() == POSITION_FLYING`. Check that integer division doesn't round to zero - enforce minimum one.

### Groundfighting Skill Not Reducing Penalties

**Symptom:** Character with high groundfighting still suffers full penalties when sitting/resting.

**Likely Cause:** Skill reduction formula not applied or skill value not retrieved correctly.

**Verification:** Check skill value via `getSkillValue(SKILL_GROUNDFIGHTING)`. Should return 0-100. Log penalty before and after reduction calculation.

**Fix:** Apply formula: `penalty = penalty * (100 - skillValue) / MAX_SKILL_LEARNEDNESS` in both `attackRound()` and `defendRound()`. Ensure this occurs only when `position < POSITION_STANDING`.

### Player Stuck in Stunned Position After Healing

**Symptom:** Character healed above zero HP but remains stunned.

**Likely Cause:** `updatePos()` not called after healing, or paralysis effect preventing position improvement.

**Verification:** Check HP value (should be > 0) and affects list for `AFF_PARALYSIS`. Log whether `updatePos()` was called post-healing.

**Fix:** Call `updatePos()` immediately after any HP modification. If paralyzed, position correctly stays stunned - remove paralysis affect to allow sitting up.

### Mounted Combat Bonuses Not Applied

**Symptom:** Character on mount doesn't receive expected attack/defense bonuses.

**Likely Cause:** Position not set to `POSITION_MOUNTED` after mounting, or mount code sets wrong position.

**Verification:** Check position value while mounted. Should be exactly `POSITION_MOUNTED` (value 11), not `POSITION_STANDING`.

**Fix:** Ensure mount/ride code sets position to `POSITION_MOUNTED` and dismount restores `POSITION_STANDING`. Both `attackRound()` and `defendRound()` explicitly check for mounted position value.

### Spells Castable While Flying When They Shouldn't Be

**Symptom:** Earth spells or ground-requiring abilities work while character is flying.

**Likely Cause:** Spell code doesn't check `isFlying()` before execution.

**Verification:** Add logging to spell code checking position. Flying should block earth/ground spells.

**Fix:** Add explicit `isFlying()` check for terrain-dependent spells. Return failure with appropriate message before consuming mana.
