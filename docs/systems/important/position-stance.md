---
title: Position and Stance System
description: Physical state and combat posture as discrete values that gate actions and modify combat effectiveness.
category: important
keywords: [physical state, combat posture, command gating, ground fighting, position hierarchy]
primary_symbols:
  functions: [getPosition, setPosition, updatePos, attackRound, defendRound, specAttackMod, getCombatMode, setCombatMode, isCombatMode, isFlying]
  classes: [TBeing, commandInfo]
  enums: [positionTypeT, POSITION_DEAD, POSITION_MORTALLYW, POSITION_INCAP, POSITION_STUNNED, POSITION_SLEEPING, POSITION_RESTING, POSITION_SITTING, POSITION_ENGAGED, POSITION_FIGHTING, POSITION_CRAWLING, POSITION_STANDING, POSITION_MOUNTED, POSITION_FLYING, attack_mode_t, ATTACK_NORMAL, ATTACK_DEFENSE, ATTACK_OFFENSE, ATTACK_BERSERK, SKILL_GROUNDFIGHTING, AFF_PARALYSIS, AFF_STUNNED]
---

# Position and Stance System

## Overview

Why can a sleeping character cast "wake" on themselves but not swing a sword? Why does a flying mage hit harder than one standing on the ground? The position and stance systems answer these questions by modeling physical state and combat posture as discrete values that gate actions and modify combat effectiveness.

**Position** represents a character's physical state: sleeping, sitting, standing, flying, or incapacitated by wounds. The system forms an ordered hierarchy from death at the bottom to flight at the top. This hierarchy drives two core mechanics: command gating (preventing illogical actions like walking while unconscious) and combat modifiers (rewarding advantageous positions like flying, penalizing vulnerable ones like sitting).

**Attack mode** (stance) represents combat posture independent of physical position. A standing warrior can fight defensively or berserk; the stance modifies attack frequency and defensive capability without changing physical state.

The position system automatically responds to HP changes. When a character takes damage below certain thresholds, they transition to incapacitated states. When healed, they recover to sitting. This creates emergent gameplay: a healer must get a mortally wounded ally above zero HP before they can stand and flee.

## Patterns

### Position Checks

**Always use `getPosition()` for comparisons, never direct field access.** The accessor ensures consistent state and allows future encapsulation changes.

**Always check position before allowing actions that require mobility.** Commands define their minimum position via `commandInfo::minPosition`. The parser enforces this automatically, but custom action code must perform its own checks.

**Never assume a character can act after damage.** Call `updatePos()` or check the position explicitly after any HP modification. A character at 1 HP who takes 5 damage may now be stunned or worse.

**Never set position without considering mount state.** If a character is riding, position must be `POSITION_MOUNTED`. Dismounting must restore the appropriate ground position. Use the mount/dismount functions rather than bypassing them.

### Combat Position Awareness

**Check flying status before earth-based or grappling attacks.** Skills like Hurl, Shoulder Throw, Defenestrate, and Bone Break cannot target flying creatures. Earth-based spells like Earthmaw fail when the caster is flying.

**Account for ground fighting skill when calculating penalties.** Characters with `SKILL_GROUNDFIGHTING` reduce position-based combat penalties proportionally to their skill level. The penalty formula scales from full penalty at 0% skill to zero penalty at max skill.

**Flying and mounted characters have significant combat advantages.** Flying provides the largest attack/defense bonus; mounted provides a moderate bonus. Position advantage has double impact since modifiers apply symmetrically to both attacker and defender. A flying attacker versus a resting defender gains the sum of both modifiers. Design encounters with this asymmetry in mind.

### Stance Management

**Never assume stance persists across combat sessions.** Berserk mode automatically exits when combat ends. Other stances may be reset by various game events.

**Berserk mode restricts command access.** Characters cannot flee, enter portals while fighting, or use most non-combat commands. Code that bypasses normal command parsing must check for berserk restrictions explicitly.

**Validate mode values before setting.** User input may provide out-of-range integers. Check against valid `attack_mode_t` enum range before casting to prevent undefined behavior.

### Movement and Position

**Flying dramatically reduces movement costs.** Movement point consumption divides by 4 for flying characters. This makes flight valuable for exploration, not just combat.

**Crawling increases movement costs significantly.** Characters with injured legs pay substantial movement penalties. Code that forces movement should verify the character can afford the cost.

## Reference

### Position Types (`positionTypeT`)

| Position | Value | Category | Description |
|----------|-------|----------|-------------|
| `POSITION_DEAD` | 0 | Incapacitated | HP <= -11, character is dead |
| `POSITION_MORTALLYW` | 1 | Incapacitated | HP -6 to -10, bleeding out |
| `POSITION_INCAP` | 2 | Incapacitated | HP -3 to -5, incapacitated |
| `POSITION_STUNNED` | 3 | Incapacitated | HP <= 0, stunned |
| `POSITION_SLEEPING` | 4 | Rest | Asleep, fastest regeneration |
| `POSITION_RESTING` | 5 | Rest | Resting, moderate regeneration |
| `POSITION_SITTING` | 6 | Rest | Sitting, minimal regeneration bonus |
| `POSITION_ENGAGED` | 7 | Combat | In combat but not actively fighting |
| `POSITION_FIGHTING` | 8 | Combat | Actively fighting |
| `POSITION_CRAWLING` | 9 | Mobile | Crawling on the ground |
| `POSITION_STANDING` | 10 | Mobile | Normal standing position |
| `POSITION_MOUNTED` | 11 | Mobile | Riding a mount |
| `POSITION_FLYING` | 12 | Mobile | Flying through the air |

### Attack Modes (`attack_mode_t`)

| Mode | Value | Description |
|------|-------|-------------|
| `ATTACK_NORMAL` | 0 | Balanced attack and defense |
| `ATTACK_DEFENSE` | 1 | Defensive stance, reduced attacks |
| `ATTACK_OFFENSE` | 2 | Aggressive stance, more attacks |
| `ATTACK_BERSERK` | 3 | Maximum offense, cannot flee |

### Attack Round Modifiers by Position

| Position | Modifier |
|----------|----------|
| Dead/Mortally/Incap/Stunned/Sleeping | Negates all attack bonus |
| Resting | -(level/3 + 1) |
| Sitting | -(level/4 + 1) |
| Engaged/Fighting/Crawling/Standing | No modifier |
| Mounted | +(level/4 + 1) |
| Flying | +(level/3 + 1) |

### Special Attack Modifiers (`specAttackMod`)

| Position | Modifier |
|----------|----------|
| Resting | -5 |
| Sitting | -3 |
| Crawling | -1 |
| Standing/Engaged/Fighting | 0 |
| Mounted | +2 |
| Flying | +3 |

### Movement Cost Modifiers

| Condition | Effect |
|-----------|--------|
| Flying | Divide by 4 (minimum 1) |
| Levitating | Divide by 4 (minimum 5) |
| Crawling (both legs hurt) | +20 base |
| Crawling with arm injury | Additional +20 |
| Haste/Accelerate | Divide by 2 |

### Symbol Quick Reference

| Symbol | Type | Purpose |
|--------|------|---------|
| `getPosition()` | function | Returns current position |
| `setPosition()` | function | Sets position directly |
| `updatePos()` | function | Auto-adjusts position based on HP |
| `attackRound()` | function | Calculates position-based attack bonuses |
| `defendRound()` | function | Calculates position-based defense bonuses |
| `specAttackMod()` | function | Returns special attack modifier for position |
| `getCombatMode()` | function | Returns current attack mode/stance |
| `setCombatMode()` | function | Changes combat stance |
| `isCombatMode()` | function | Checks if current mode matches a specific mode |
| `isFlying()` | function | Checks if position is POSITION_FLYING |
| `commandInfo` | class | Defines command properties including minPosition |
| `TBeing` | class | Character base class, owns position/stance state |

### Position Display Names

The `position_types[]` array in `constants.cc` provides human-readable names: Dead, Mortally wounded, Incapacitated, Stunned, Sleeping, Resting, Sitting, Engaged, Fighting, Crawling, Standing, Mounted, Flying. The array is null-terminated with a `"\n"` sentinel for iteration safety.

## Implementation

### Position State Management

Position is stored as a `positionTypeT` value on `TBeing`. Direct access uses `getPosition()` and `setPosition()`. The value forms an ordered hierarchy where higher values represent more capable states.

The command parser in `parse.cc` retrieves `commandArray[cmd]->minPosition` and compares against the character's current position. If position is below the minimum, the parser sends a position-specific error message ("You cannot do that while sleeping!") and blocks execution. Commands requiring `minPosition >= POSITION_CRAWLING` are additionally blocked during combat.

Paralysis (`AFF_PARALYSIS`) and stun (`AFF_STUNNED`) affects block commands requiring greater than `POSITION_STUNNED`, effectively locking the character in place regardless of their nominal position.

### Automatic Position Updates

`updatePos()` in `combat.cc` automatically adjusts position based on HP thresholds:

- HP <= -11 sets `POSITION_DEAD`
- HP -6 to -10 sets `POSITION_MORTALLYW`
- HP -3 to -5 sets `POSITION_INCAP`
- HP <= 0 sets `POSITION_STUNNED`
- HP > 0 does not automatically change position (the character remains wherever they were)

Recovery from incapacitated states follows a progression: characters healed above 0 HP from mortally wounded or incapacitated first transition to stunned. From stunned, healing above 0 HP causes the character to sit up automatically. Special cases: mounted characters recover to `POSITION_MOUNTED`; paralyzed characters remain at `POSITION_STUNNED` regardless of HP.

### Combat Modifier Calculations

`attackRound()` and `defendRound()` in `combat.cc` calculate position-based bonuses. The modifier scales with character level:

- Incapacitated and sleep states negate all bonus
- Resting applies -(level/3 + 1)
- Sitting applies -(level/4 + 1)
- Neutral positions (standing, crawling, engaged, fighting) apply no modifier
- Mounted applies +(level/4 + 1)
- Flying applies +(level/3 + 1)

Defense modifiers use identical scaling to attack modifiers, creating symmetric advantage for superior positioning.

`specAttackMod()` provides additional modifiers for special attacks, using fixed values rather than level scaling. The function returns values from -5 (resting) to +3 (flying).

### Ground Fighting Skill

Characters with `SKILL_GROUNDFIGHTING` reduce position penalties when below standing. The formula multiplies the penalty by `(100 - getSkillValue(SKILL_GROUNDFIGHTING)) / MAX_SKILL_LEARNEDNESS`. At maximum skill, ground fighting eliminates position penalties entirely; at 50% skill, penalties are halved.

### Movement Cost Calculation

Movement cost calculation in `movement.cc` applies position-based modifiers after base terrain costs:

- Flying divides cost by 4 with minimum 1
- Levitating divides cost by 4 with minimum 5
- Crawling (both legs injured) adds 20 base, plus another 20 if an arm is also injured
- Haste or accelerate effects divide the final cost by 2

These modifiers stack multiplicatively where applicable.

### Attack Mode Implementation

Attack mode is separate from physical position, stored per-character on `TBeing` and accessed via `getCombatMode()`, `setCombatMode()`, and `isCombatMode()`. The mode affects attack frequency and defensive calculations in combat resolution.

Defense mode reduces attack opportunities while increasing defensive bonuses. Offense mode inverts this tradeoff. Berserk mode maximizes offense but imposes severe restrictions: the character cannot flee, cannot enter portals while fighting, and cannot use most non-combat commands. Berserk automatically ends when combat ends.

### Spell and Skill Position Requirements

Spells and disciplines define minimum position via `discArray[which]->minPosition`. The casting system checks this before allowing execution, sending position-specific error messages on failure. Position filtering occurs after command parsing but before resource consumption (mana, moves), ensuring players do not pay costs for actions they cannot complete.

Flying imposes additional restrictions independent of minPosition. Earth-based spells (Earthmaw), certain nature abilities (Camp, Forage), and Feign Death cannot be cast while flying or in flying sectors. Some physical skills (Hurl, Shoulder Throw, Defenestrate, Bone Break) cannot target flying creatures.

## Troubleshooting

### Character Cannot Execute Command

**Symptom:** Command fails with "You cannot do that while..." message.

**Likely cause:** Character position is below the command's `minPosition` requirement.

**Diagnostic approach:** Check `getPosition()` and compare against the command's `minPosition` in `commandArray`. Verify no affects (paralysis, stun) are blocking action.

**Fix:** Raise position via stand/wake command, or heal if in incapacitated state.

### Combat Modifiers Not Applying

**Symptom:** Flying character not receiving expected attack bonus.

**Likely cause:** Position not actually set to `POSITION_FLYING`, or ground fighting skill counteracting expected penalties for the opponent.

**Diagnostic approach:** Verify `getPosition()` returns `POSITION_FLYING`. Check `SKILL_GROUNDFIGHTING` values if penalty reduction seems wrong.

**Fix:** Ensure the position-setting code path executed correctly. Flight spells and abilities should call `setPosition(POSITION_FLYING)`.

### Position Not Updating After Damage

**Symptom:** Character remains standing after HP drops to negative values.

**Likely cause:** `updatePos()` not called after damage application.

**Diagnostic approach:** Trace the damage code path to verify `updatePos()` is invoked after HP modification.

**Fix:** Ensure `updatePos()` is called after any code that modifies HP. The standard damage functions handle this automatically; custom damage code must call it explicitly.

### Berserk Mode Persisting After Combat

**Symptom:** Character stuck in berserk mode after all enemies defeated.

**Likely cause:** Combat end handler not triggering mode reset, or character re-entering combat before reset completes.

**Diagnostic approach:** Check combat list membership and verify the combat-end code path executes.

**Fix:** The automatic berserk exit depends on detecting combat end. Manual intervention may require direct `setCombatMode(ATTACK_NORMAL)` call.

### Mounted Combat Bonuses Not Applied

**Symptom:** Character on mount does not receive expected attack/defense bonuses.

**Likely cause:** Position not set to `POSITION_MOUNTED` after mounting, or mount code sets wrong position.

**Diagnostic approach:** Check position value while mounted. Should be exactly `POSITION_MOUNTED` (value 11), not `POSITION_STANDING`.

**Fix:** Ensure mount/ride code sets position to `POSITION_MOUNTED` and dismount restores `POSITION_STANDING`. Both `attackRound()` and `defendRound()` explicitly check for mounted position value.

### Spells Castable While Flying When They Should Not Be

**Symptom:** Earth spells or ground-requiring abilities work while character is flying.

**Likely cause:** Spell code does not check `isFlying()` before execution.

**Diagnostic approach:** Add logging to spell code checking position. Flying should block earth/ground spells.

**Fix:** Add explicit `isFlying()` check for terrain-dependent spells. Return failure with appropriate message before consuming mana.
