---
title: Combat Skills
description: Physical combat abilities including melee skills, ranged combat, and unarmed techniques with two-phase success model and damage calculation
category: important
keywords: [two-phase success, skill execution, combat effectiveness, damage scaling, warrior skills, monk skills, thief skills, ranger skills, deikhan skills]
primary_symbols:
  functions: [bSuccess, specialAttack, getSkillDam, reconcileDamage, shootMeBow, getMonkWeaponDam]
  classes: [TBeing, TBow, TArrow, spellInfo]
  enums: [SKILL_WARRIOR, SKILL_MONK, SKILL_THIEF, SKILL_DEIKHAN, SKILL_RANGER, SKILL_SHAMAN, SKILL_RANGED_PROF, SKILL_RANGED_SPEC, SKILL_FAST_LOAD, SKILL_BAREHAND_PROF, SKILL_BAREHAND_SPEC, SKILL_KUBO, SKILL_IRON_FIST, SKILL_VOPLAT, SKILL_KICK, SKILL_BASH, SKILL_BERSERK, SKILL_MEDITATE, SKILL_PENANCE, SKILL_YOGINSA, SKILL_ENCAMP, SKILL_WOHLIN, SKILL_GROUNDFIGHTING, GUARANTEED_SUCCESS, COMPLETE_SUCCESS, PARTIAL_SUCCESS, FAILURE, GUARANTEED_FAILURE, TASK_TRIVIAL, TASK_EASY, TASK_NORMAL, TASK_DIFFICULT, TASK_DANGEROUS, TASK_HOPELESS, TASK_IMPOSSIBLE, LAG_0, LAG_1, LAG_2, LAG_3, LAG_4, LAG_5, LAG_9, TAR_VIOLENT, TAR_FIGHT_VICT, TAR_CHAR_ROOM, DELETE_VICT, DELETE_THIS, DELETE_ITEM, STAT_STR, STAT_DEX, STAT_AGI, STAT_BRA, STAT_FOC, POSITION_STANDING, POSITION_FIGHTING, POSITION_RESTING, BOW_STRING_BROKE, ITEM_WEAR_THROW, discNumT, DISC_SOLDIERING, DISC_MINDBODY, DISC_MURDER]
---

## Overview

When a warrior executes a bash, how does the game determine success? When an arrow strikes a target three rooms away, what factors govern the damage? Combat skills are physical abilities that characters use in battle, from basic kicks to lethal backstabs to ranged bow attacks.

Combat skills represent physical prowess rather than magical effects. Warriors bash and bodyslam. Monks deliver precise kicks and throws. Thieves strike from hiding. Rangers fire arrows across multiple rooms. Each skill draws on a character's trained ability, relevant stats, and tactical positioning to determine whether an attack connects and how much damage it deals.

The skill system uses a **two-phase success model**. First, the game checks whether you can execute the technique at all based on your skill learning, focus, and karma. Second, it determines whether your executed technique actually connects with the target based on attack versus defense rolls. This separation allows for nuanced outcomes: a well-trained fighter might execute a perfect technique that their agile opponent still dodges.

Skills share the same `spellInfo` framework as spells. Each skill defines its discipline (which class learns it), lag penalty (how long before your next action), position requirements (must you be standing?), and target flags (can you use this in combat?). This unified structure means understanding one skill's definition helps you understand them all.

Damage scales predictably. Higher skill learning increases both success chance and damage output. The primary stat for each skill (strength for bash, dexterity for backstab) modifies effectiveness. Level provides the baseline, and lag balances power: devastating attacks like deathstroke impose longer delays before your next action.

Consider a typical kick: you target an opponent, the game checks your `SKILL_KICK` learning against a task difficulty modifier, applies your focus and karma stats, and rolls. If you pass this skill execution check, the game then pits your attack roll against the target's defense. Success means damage calculated from your level, stat modifiers, and the skill's power rating. Failure means a whiffed attack and potentially losing your balance.

## Patterns

### Two-Phase Success Checks

Always separate skill execution from combat effectiveness. The first check (`bSuccess()`) determines if you execute the technique. The second check (`specialAttack()`) determines if it connects. Never conflate these or skip either phase.

### Three-Tier Target Resolution

Always resolve targets in this order:
1. Use the explicit `vict` parameter if provided (programmatic call from another function)
2. Parse the argument string to find the target by name
3. Fall back to the current fight target

This pattern ensures skills work both when called programmatically and when invoked by player commands.

### Pre-Execution Validation

Always call the `canXXX()` validation function before executing any skill. These functions check position requirements, target presence, equipment needs, and resource costs. They return `FALSE` and send appropriate messages on failure. Never skip this validation or duplicate it inline.

### reconcileDamage() Death Detection

Always check `reconcileDamage()` returns `-1` for death. This function does NOT return `DELETE_VICT`. Using `IS_SET_DELETE(rc, DELETE_VICT)` on its return value will never trigger.

```
// WRONG - will never detect death
int rc = reconcileDamage(victim, dam, skill);
if (IS_SET_DELETE(rc, DELETE_VICT)) { ... }

// CORRECT
if (reconcileDamage(victim, dam, skill) == -1)
    return DELETE_VICT;
```

### DELETE_VICT Ownership

Always check ownership before deleting on `DELETE_VICT`. If the `vict` parameter was provided by the caller, return the flag so the caller can handle deletion. Only delete locally if you resolved the victim yourself. Always clear the flag with `REM_DELETE()` after local deletion.

```
if (IS_SET_DELETE(rc, DELETE_VICT)) {
    if (vict)
        return rc;  // Caller owns victim
    delete victim;
    victim = nullptr;
    REM_DELETE(rc, DELETE_VICT);
}
```

### Flag Checking

Always use `IS_SET_DELETE()` for DELETE flags. Never use `IS_SET()`. The DELETE flag system uses a special bitmask pattern that `IS_SET()` cannot detect.

### Role Translation in Mutual Calls

When calling `victim->method(this)`, the roles reverse. The callee's `DELETE_THIS` means our victim should be deleted, and their `DELETE_VICT` means we should be deleted. Translate flags accordingly when processing return values from such calls.

### Lag Application

Apply lag only on successful skill execution. Use `addSkillLag()` with the skill and success result. The system automatically caps lag at `LAG_1` when the skill kills its target, allowing faster cleanup of remaining enemies.

### Ranged Skill Minimum

Always check `SKILL_RANGED_PROF` is at least 10 before allowing shooting. Characters with lower skill cannot effectively use ranged weapons. Between 1-9 skill, arrows load but fail to fire properly. Characters with 0 skill receive "you don't know how to shoot" messages.

### Monk Barehand Damage

Always check for `SKILL_KUBO` when calculating monk barehand damage. Standard barehand damage is minimal (1-3). Monks with KUBO training use `getMonkWeaponDam()` which scales damage with skill level. Using the wrong function produces incorrect damage.

### Immediate Return on DELETE

Never continue execution after detecting a DELETE flag. Check immediately and return or break. Continuing can lead to use-after-free crashes when accessing deleted objects.

### Clear DELETE Flags After Local Deletion

After deleting an object locally and clearing the pointer to null, always call `REM_DELETE(rc, DELETE_VICT)` or `REM_DELETE(rc, DELETE_ITEM)` before returning. Failing to clear the flag can cause double-free when the flag propagates to code that didn't receive the pointer.

## Reference

### Symbol Quick Reference

| Symbol | Type | Purpose |
|--------|------|---------|
| `bSuccess()` | function | Skill execution check, returns success/fail |
| `specialAttack()` | function | Combat effectiveness check, returns success level |
| `getSkillDam()` | function | Calculate skill damage from level, learning, stats |
| `reconcileDamage()` | function | Apply damage, returns -1 on death |
| `shootMeBow()` | function | Fire arrow from bow at target |
| `getMonkWeaponDam()` | function | Calculate monk barehand damage |
| `TBeing` | class | Base class for all characters |
| `TBow` | class | Ranged weapon (bows, crossbows, slings) |
| `TArrow` | class | Ammunition (arrows, bolts, darts) |
| `spellInfo` | class | Skill/spell definition structure |

### Skill Classes

| Class | Enum | Skills |
|-------|------|--------|
| Warrior | `SKILL_WARRIOR` | Bash, bodyslam, spin, kick, headbutt, kneestrike, stomp, grapple, deathstroke, disarm, rescue, defend, berserk, switch, riposte, retreat, focused attack, charge, slam |
| Monk | `SKILL_MONK` | Kick, springleap, hurl, defenestrate, chi, quivering palm, feign death, iron roots, shoulder throw, bonebreak, mindbody, blur, meditate |
| Thief | `SKILL_THIEF` | Backstab, throatslit, garrotte, stabbing, poisoning, ambush |
| Deikhan | `SKILL_DEIKHAN` | Charge, smite, shock cavalry, harm, deathstroke, defend |
| Ranger | `SKILL_RANGER` | Transfix |

### spellInfo Structure Fields

| Field | Purpose | Values |
|-------|---------|--------|
| `typ` | Skill class type | `SKILL_WARRIOR`, `SKILL_MONK`, `SKILL_THIEF`, `SKILL_DEIKHAN`, `SKILL_RANGER`, `SKILL_SHAMAN` |
| `disc` | Primary discipline | `discNumT` enum (e.g., `DISC_SOLDIERING`, `DISC_MINDBODY`, `DISC_MURDER`) |
| `assDisc` | Associated discipline for do-learning | Usually matches `disc` |
| `modifierStat` | Stat affecting success/damage | `STAT_STR`, `STAT_DEX`, `STAT_AGI` |
| `task` | Difficulty rating | `TASK_TRIVIAL` through `TASK_IMPOSSIBLE` |
| `lag` | Lag penalty | `lag_t` enum values |
| `minPosition` | Required position | `POSITION_STANDING`, `POSITION_FIGHTING`, `POSITION_RESTING` |
| `targets` | Target bitflags | `TAR_VIOLENT`, `TAR_FIGHT_VICT`, `TAR_CHAR_ROOM` |

### specialAttack() Return Values

| Return | Value | Meaning |
|--------|-------|---------|
| `GUARANTEED_SUCCESS` | -2 | Critical hit, maximum effect |
| `COMPLETE_SUCCESS` | 1 | Full success |
| `PARTIAL_SUCCESS` | 2 | Reduced effectiveness |
| `FAILURE` | 0 | Attack fails |
| `GUARANTEED_FAILURE` | -1 | Critical failure |

### Task Difficulty Modifiers

| Difficulty | Modifier | Typical Use |
|------------|----------|-------------|
| `TASK_TRIVIAL` | 110% | Tutorial skills |
| `TASK_EASY` | 100% | Basic skills |
| `TASK_NORMAL` | 90% | Standard combat skills |
| `TASK_DIFFICULT` | 80% | Advanced techniques |
| `TASK_DANGEROUS` | 70% | High-risk maneuvers |
| `TASK_HOPELESS` | 50% | Desperation moves |
| `TASK_IMPOSSIBLE` | 35% | Near-impossible feats |

### Lag Values

| Constant | Pulses | Real Time | Typical Use |
|----------|--------|-----------|-------------|
| `LAG_0` | 0 | 0.0 sec | Instant skills |
| `LAG_1` | 12 | 1.2 sec | Very fast |
| `LAG_2` | 24 | 2.4 sec | Standard combat |
| `LAG_3` | 36 | 3.6 sec | Powerful attacks |
| `LAG_4` | 48 | 4.8 sec | Very powerful |
| `LAG_5` | 60 | 6.0 sec | Devastating |
| `LAG_9` | 108 | 10.8 sec | Ultimate abilities |

### Ranged Combat Skills

| Skill | Number | Effect |
|-------|--------|--------|
| `SKILL_RANGED_PROF` | 569 | Basic ranged competency (min 10 to shoot), adds 0-50% damage |
| `SKILL_RANGED_SPEC` | 568 | Advanced mastery (Warriors/Rangers), adds 0-50% damage |
| `SKILL_FAST_LOAD` | - | Increases arrows per round |

Arrows per round formula: `1.0 + max(0, SKILL_FAST_LOAD/100) + max(0, SKILL_RANGED_SPEC/100)`

### Unarmed Combat Skills

| Skill | Number | Effect |
|-------|--------|--------|
| `SKILL_BAREHAND_PROF` | 563 | Basic unarmed competency (1-3 damage without monk skills) |
| `SKILL_BAREHAND_SPEC` | 567 | Advanced unarmed mastery for monks |
| `SKILL_KUBO` | - | Monk barehand scaling |
| `SKILL_IRON_FIST` | - | +0-8.3% damage when ungloved |
| `SKILL_VOPLAT` | - | +0-10% damage bonus |

KUBO damage formula: `value = 3.0 * SKILL_KUBO / 10.0` (clamped 0-50), then `weapDam = 6.0 * sqrt(value) / 2.0`

### Arrow Types

| Type | Value | Description |
|------|-------|-------------|
| Hunting arrow | 0 | Standard bow arrow |
| Fighting arrow | 1 | Combat bow arrow |
| Squabble quarrel | 2 | Light crossbow bolt |
| Common quarrel | 3 | Standard crossbow bolt |
| Sniper blowdart | 4 | Precision blowgun dart |
| Common blowdart | 5 | Standard blowgun dart |
| Heavy sling ammo | 6 | Large sling stone |
| Common sling ammo | 7 | Standard sling stone |

### Berserk Skill Weights

| Skill | Weight |
|-------|--------|
| Bash | 2 |
| Headbutt | 3 |
| Bodyslam | 2 |
| Grapple | 1 |
| Slam | 3 |
| Deathstroke | 1 |

Higher weights increase selection probability during berserk rage.

### Key Files

| File | Purpose |
|------|---------|
| `code/code/misc/skill_dam.cc` | Damage calculation functions |
| `code/code/misc/combat.cc` | Combat effectiveness, barehand damage |
| `code/code/misc/range.cc` | Ranged combat, throwing, shooting |
| `code/code/misc/spell_info.cc` | Skill definitions in discArray |
| `code/code/obj/obj_bow.cc` | TBow implementation |
| `code/code/obj/obj_arrow.cc` | TArrow implementation |
| `code/code/cmd/cmd_*.cc` | Individual skill command handlers |
| `code/code/disc/disc_*.cc` | Class-specific skill implementations |
| `code/code/spec/spec_mobs_archer.cc` | Archer MOB AI |

## Implementation

### Skill Definition Structure

Skills are defined as `spellInfo` objects stored in the global `discArray`. Each definition specifies the skill's class type (`SKILL_WARRIOR`, `SKILL_MONK`, etc.), primary and associated disciplines, the modifier stat (STR, DEX, AGI), task difficulty, lag penalty, minimum position, target flags, and learning rates.

The `targets` field uses bitflags: `TAR_VIOLENT` marks skills that initiate combat, `TAR_FIGHT_VICT` allows targeting current combat opponent, `TAR_CHAR_ROOM` allows targeting anyone in the room. Most combat skills combine all three.

### Phase 1: Skill Execution (bSuccess)

The `bSuccess()` function determines whether the character successfully executes the skill technique. The calculation multiplies several factors:

The base is the task difficulty modifier from `getSkillDiffModifier()`, ranging from 110% for trivial tasks to 35% for impossible ones. This is multiplied by the character's skill learning (0-100%), their focus stat modifier (0.8x to 1.25x), and their karma stat modifier (0.9x to 1.125x).

A random roll from 0-99 is compared against the final percentage. Values below the threshold succeed. This means a fully-learned normal-difficulty skill with average stats succeeds roughly 90% of the time.

### Phase 2: Combat Effectiveness (specialAttack)

After successful execution, `specialAttack()` determines if the technique connects. This compares the attacker's attack round against the defender's defense round, adding situational modifiers for position, stats, equipment, level difference, and combat mode.

A random roll from 1-100 has the situational modifier subtracted, then compares against fixed threshold constants:
- Roll <= 5: `GUARANTEED_SUCCESS` (absolute floor, unaffected by modifier)
- Roll > 95: `GUARANTEED_FAILURE` (absolute ceiling, unaffected by modifier)
- Roll < 50 (`SUCCESS_THRESHOLD`): `COMPLETE_SUCCESS`
- Roll < 80 (`PARTIAL_SUCCESS_THRESHOLD`) and partial success allowed: `PARTIAL_SUCCESS`
- Otherwise: `FAILURE`

The thresholds (5, 50, 80, 95) are fixed constants. The modifier shifts the roll, not the thresholds. The return value indicates success level: `GUARANTEED_SUCCESS` (-2) means maximum effect, `COMPLETE_SUCCESS` (1) means full effect, `PARTIAL_SUCCESS` (2) means reduced effect (typically half damage), and failures mean the attack misses.

### Damage Calculation (getSkillDam)

The `getSkillDam()` function calculates base damage from several components. The formula multiplies the skill's class amount (a fixed power rating per skill), the lag value converted to rounds, and the character's level. This product is then modified by advanced learning bonus, the primary stat modifier, and random variance based on level.

Class amounts range from low (0.5-0.75 for basic kicks and bashes) through medium (1.0-1.25 for grapples and headbutts) to very high (2.0-4.0 for backstabs and throatslits). Higher amounts mean more damage per use.

NPCs deal approximately 52% of player damage for identical skills. This is applied as a flat multiplier when the attacker is a MOB, preventing NPCs from overwhelming players with skill spam.

### Location-Based Damage

Some skills have location-specific damage multipliers. The kick skill randomly selects a hit location with different multipliers: head strikes deal 2.5x damage, solar plexus 2.0x, shin 1.5x, and side 1.0x (base).

The stomp skill targets different body parts based on relative height. Stomping a prone victim's body deals full damage. Stomping a standing victim's toes deals 20% damage, unless the stomper is significantly taller and can reach the head.

### Ranged Combat System

Ranged combat involves `TBow` objects (bows, crossbows, slings, blowguns) firing `TArrow` objects (arrows, bolts, darts, stones) at targets in distant rooms.

The bow tracks its ammunition type (arrows must match the bow), state flags (broken string, crafting progress), and maximum range in rooms. The arrow tracks its type, head type and material (affecting damage), and optional trap effects.

When shooting, the `doShoot()` function validates the shooter has a bow equipped, checks minimum `SKILL_RANGED_PROF` of 10, finds the target in the specified direction, and delegates to `shootMeBow()`. The bow function verifies it's loaded, checks range limits, performs bowstring break checks (random against struct points; on 0, `BOW_STRING_BROKE` flag sets), calculates arrows per round based on `SKILL_FAST_LOAD` and `SKILL_RANGED_SPEC`, fires the arrows, and auto-reloads if ammunition remains.

Ranged damage starts with the arrow's damage level. The `get_range_actual_damage()` function applies skill modifiers: proficiency adds up to 50% at max skill, specialization adds another 50%. Thrown objects use only proficiency. Total damage at maximum skills is 200% of base arrow damage.

When an arrow hits, `catchSmack()` handles the impact. It checks if the target dodges via `specialAttack()` versus `SKILL_RANGED_PROF`. If the arrow connects, it may embed based on sharpness. Damage is calculated and applied. Poison triggers if the arrow is poisoned. Trap effects trigger if the arrow is trapped.

### Throwing Mechanics

Objects with the `ITEM_WEAR_THROW` flag can be thrown. Throw distance uses a physics simulation: acceleration from brawn stat (500-5000 based on `STAT_BRA`) divided by object weight determines initial velocity (`v0 = 0.2 * acceleration`). Trajectory angle from focus stat (0-45 degrees based on `STAT_FOC`) affects range calculation using projectile motion formula. Maximum distance derives from these factors, with outdoor environments allowing twice the range of indoor.

### Archer MOB Behavior

MOBs with the `archer` spec proc automatically engage in ranged combat. They find and equip bows with matching ammunition, flee from melee if a hated target enters their room, scan up to three rooms in each direction for targets, and shoot at hated PCs in their line of sight.

### Unarmed Combat System

Standard barehand damage is minimal: 1-3 points per hit. Monks transform unarmed combat through the `SKILL_KUBO` skill tree.

The `getMonkWeaponDam()` function scales barehand damage with KUBO learning. The formula takes the skill value, multiplies by 3.0/10.0, clamps to 0-50, takes the square root, and multiplies by 6/2. This produces base damage around 9 at 33% skill, 13 at 66%, and 16 at maximum. The `balanceCorrectionForLevel()` multiplier then scales for character level, followed by +/- 10% random variance.

Additional modifiers layer on top. Strength provides its standard damage bonus. `SKILL_IRON_FIST` adds up to 8.3% when fighting with ungloved hands. `SKILL_VOPLAT` adds up to 10% additional damage.

A global `barehand_damage_mod` (default 0.36) applies to all barehand damage. This can be adjusted at runtime via the stats command.

### Combat Mode Effects

Combat modes (stances) affect skill usage and damage. Normal mode allows all skills at 1.0x damage. Defensive mode favors defensive skills at 0.9x damage. Offensive mode favors offensive skills at 1.1x damage. Berserk mode restricts skill selection to a weighted subset (bash, headbutt, bodyslam, grapple, slam, deathstroke) at 1.2x damage.

### Position Requirements

Most combat skills require `POSITION_STANDING`. Exceptions exist: feign death can be used from any position, stomp requires the target to be sitting/resting/sleeping, and chi can be used while sitting in meditation.

### Lag System

Skills impose lag measured in game pulses (0.1 seconds each). The lag prevents skill spam by delaying the character's next action. Lag values range from instant (0 pulses) to ultimate abilities (108 pulses / 10.8 seconds).

When a skill kills its target (`DELETE_VICT` set), lag is automatically capped at `LAG_1` to allow faster engagement of remaining enemies.

### Command File Organization

Each skill typically has its own command file in `code/code/cmd/`. These files contain the `doXXX()` entry point, the main skill function, and separate success and failure handlers. The discipline files in `code/code/disc/` group class-specific skills by discipline.

### Standard Skill Flow

A typical skill follows this flow:

The `doXXX()` entry point resolves the target using three-tier resolution, calls the main skill function, applies lag on success, handles `DELETE_VICT` with ownership checking, and propagates `DELETE_THIS`.

The main skill function calls `canXXX()` for validation, consumes resources (movement points typically), calls `bSuccess()` for skill execution check, and branches to success or failure handlers based on the result.

The success handler calls `specialAttack()` for combat effectiveness, calculates damage via `getSkillDam()`, applies success level and location modifiers, sends combat messages, calls `reconcileDamage()`, and returns `DELETE_VICT` if the target dies.

The failure handler sends failure messages, may apply side effects (falling, balance loss), calls `reconcileDamage()` with zero damage to initiate combat, and returns success to still apply lag.

## Troubleshooting

### Skill Always Fails

**Symptom:** A skill never succeeds even at high learning.

**Likely cause:** Task difficulty set too high, or modifier stat misconfigured.

**Diagnostic approach:** Check the skill's `spellInfo` definition in `discArray`. Verify the `task` field isn't set to `TASK_IMPOSSIBLE`. Check that `modifierStat` points to a stat the character actually has points in.

**Fix:** Adjust task difficulty or verify character stats.

### Damage Seems Too Low

**Symptom:** Skill damage is much lower than expected.

**Likely cause:** Missing stat modifier application, wrong class amount, or NPC damage reduction applying when it shouldn't.

**Diagnostic approach:** Trace through `getSkillDam()`. Check if the attacker is being treated as a MOB (52% reduction). Verify the class amount for the skill. Check if `PARTIAL_SUCCESS` is reducing damage by half.

**Fix:** Ensure correct success level, verify MOB check logic, adjust class amount if needed.

### Arrow Hits But No Damage

**Symptom:** Ranged attack connects but deals zero damage.

**Likely cause:** Skill levels too low, arrow damage level is zero, or skill modifiers aren't applying.

**Diagnostic approach:** Check `SKILL_RANGED_PROF` and `SKILL_RANGED_SPEC` values. Verify the arrow's `damageLevel()` returns a positive value. Trace through `get_range_actual_damage()`.

**Fix:** Ensure minimum skill levels, verify arrow properties.

### Monk Barehand Damage Too Low

**Symptom:** Monk deals minimal barehand damage.

**Likely cause:** `SKILL_KUBO` not learned, or wrong damage function being called.

**Diagnostic approach:** Verify the monk has `SKILL_KUBO` learned. Check that `getMonkWeaponDam()` is being called rather than the standard barehand damage.

**Fix:** Ensure KUBO skill is learned, verify code path uses monk-specific function.

### DELETE_VICT Not Detected

**Symptom:** Victim death not detected, leading to use-after-free.

**Likely cause:** Checking `reconcileDamage()` return with `IS_SET_DELETE()` instead of `== -1`.

**Diagnostic approach:** Find the `reconcileDamage()` call and check how its return value is tested.

**Fix:** Use `if (reconcileDamage(...) == -1) return DELETE_VICT;`

### Double-Free or Use-After-Free

**Symptom:** Crash when deleting victim, or accessing deleted victim.

**Likely cause:** Ownership check missing, flag not cleared after deletion, or continuing execution after DELETE flag detected.

**Diagnostic approach:** Verify the ownership check (`if (vict) return rc;`) exists. Ensure `REM_DELETE()` is called after local deletion. Check that no code executes after a DELETE flag is detected.

**Fix:** Add ownership check, clear flag after deletion, return immediately on DELETE.

### Skill Works for Players But Not MOBs

**Symptom:** MOBs cannot use a skill that players can.

**Likely cause:** `UtilMobProc()` check blocking MOB usage, or missing MOB-specific handling.

**Diagnostic approach:** Check if `canXXX()` calls `UtilMobProc()`. Verify the skill's target flags allow MOB usage.

**Fix:** Adjust validation logic for MOB compatibility.

### Bowstring Break Ignored

**Symptom:** Bow continues firing after string breaks.

**Likely cause:** `BOW_STRING_BROKE` flag not checked before arrow launch.

**Diagnostic approach:** Check if `shootMeBow()` tests for bowstring break and returns `FALSE` when string snaps.

**Fix:** Check for bowstring break before arrow launch, set flag and return immediately on break.

### Berserk Mode Allows All Skills

**Symptom:** Berserking characters can use any skill instead of restricted set.

**Likely cause:** Skill selection not validating against berserk-allowed set.

**Diagnostic approach:** Check if `affectedBySpell(SKILL_BERSERK)` triggers skill restriction logic.

**Fix:** Validate skill selection against allowed set (bash, headbutt, bodyslam, grapple, slam, deathstroke) during berserk rage.

### Combat Mode Damage Not Applied

**Symptom:** Defensive/offensive/berserk modes don't affect damage output.

**Likely cause:** `getCombatMode()` modifier not applied in damage calculation.

**Diagnostic approach:** Trace through `getSkillDam()` or skill-specific damage functions for mode multiplier.

**Fix:** Apply mode-specific multiplier (0.9x defensive, 1.1x offensive, 1.2x berserk).
