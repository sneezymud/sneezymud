---
title: Rest and Recovery System
category: important
created_by_model: opus
keywords: [regeneration, sleep, rest, meditate, camp, hitGain, manaGain, moveGain, piety, lifeforce]
related: [position-stance.md, scheduler-pulses.md, task-system.md, affects-system.md]
primary_symbols:
  functions: [hitGain, manaGain, moveGain, regenTime, updateHalfTickStuff, inCamp, bedRegen, canMeditate]
  classes: [TBeing, TPerson, TBed]
  files: [code/code/misc/limits.cc, code/code/misc/periodic.cc, code/code/misc/movement.cc, code/code/task/task_sleep.cc, code/code/task/task_rest.cc, code/code/task/task_meditate.cc, code/code/task/task_penance.cc, code/code/disc/disc_monk_meditation.cc, code/code/disc/disc_advanced_adventuring.cc, code/code/obj/obj_bed.cc]
---

## Overview

How do characters recover from damage, restore spent mana, and regain stamina after exertion? The rest and recovery system manages regeneration of HP, mana, movement, piety, and lifeforce through multiple overlapping mechanisms.

Recovery in SneezyMUD operates on two parallel tracks: **half-tick recovery** applies globally to all characters every 36 seconds regardless of activity, while **task-based recovery** provides bonus regeneration tied to specific positions (sleeping, resting, sitting) and class abilities (meditation, penance, yoginsa). Environmental factors like beds, camps, and hospital rooms layer additional bonuses on top of these base rates.

The system balances recovery speed against vulnerability and capability. Sleeping provides the fastest regeneration but leaves you defenseless and unable to act. Resting offers good recovery while permitting limited actions. Standing gives only half-tick recovery but full combat readiness. This creates meaningful tactical decisions about when and where to recover.

Recovery rates scale with level, constitution, and environmental bonuses. A level 50 character in a hospital bed with the enliven spell active recovers dramatically faster than a low-level character standing in hostile territory. Class-specific abilities provide additional recovery options: mages meditate for mana, clerics perform penance for piety, monks practice yoginsa for enhanced healing with bonus curative effects.

A wounded warrior returns from battle to the inn. Lying down to sleep triggers a task that grants +1 HP, +1 mana, and +1 movement on each cycle. Simultaneously, every 36 seconds the global half-tick adds their full hitGain() and manaGain() values. If they sleep on a bed in a hospital room, both multipliers stack. After several minutes, they wake fully restored and ready for the next adventure.

## Patterns

### Position Selection

Always choose sleeping when maximum recovery speed is needed and safety is ensured. Sleep provides the fastest task interval and grants all three primary resources. If you need to perform limited actions during recovery, use resting instead - the interval is twice as long but you remain somewhat responsive.

Never attempt to sleep or rest while in combat, flying, or in water without aquatic adaptation. The system rejects these attempts and wastes the player's action. Check these conditions before allowing position changes.

### Task Interval Calculation

Never assume fixed task intervals. The `regenTime()` function calculates intervals dynamically based on the character's current regeneration rates. Higher regeneration rates produce shorter intervals. The formula divides the update pulse length by the slowest of the character's HP, mana, and movement gain rates.

### Shaman Lifeforce Handling

Always check for shaman class when implementing HP recovery logic. Shamans above level 5 drain lifeforce instead of gaining HP through sleep/rest tasks. They must rely on half-tick recovery for HP or maintain positive lifeforce through spirit consumption. Never grant task-based HP to shamans without first checking lifeforce status.

### ROOM_NO_HEAL Behavior

Never assume ROOM_NO_HEAL blocks all recovery. The flag stops task-based regeneration completely and reduces movement gain by two-thirds, but half-tick HP and mana recovery still function. Characters can still recover in these rooms, just more slowly and without position bonuses.

### Camp Application

Never tie camp bonuses to position. Unlike most recovery mechanics, camp bonuses from `inCamp()` apply directly to `hitGain()` and `moveGain()` calculations. Characters receive camp benefits whether standing, sitting, or sleeping. The bonus scales with the camper's skill level, with groupmates receiving half the benefit.

### Combat Interruption

Always apply round loss penalties when characters are attacked while resting or sleeping. The task fighting handler causes 1-2 lost combat rounds before stopping the task. This represents the vulnerability cost of recovery positions.

### Piety Recovery

Always use rest position for piety recovery through the task system. Sleep does NOT grant piety despite being faster for other resources. The penance skill provides the best piety recovery for clerics and deikhan, scaling with duration.

### Meditation Position Requirements

Always validate position through `canMeditate()` before allowing meditation skills. Valid positions are resting, sitting, or standing. Mounted meditation requires Advanced Riding at 50% or higher. Flying and linkdead states block meditation entirely.

### Bed Size Validation

Always check character height against bed max_size. Characters taller than the bed (minus 6 inches tolerance) suffer reduced regeneration. The penalty scales with the height difference, potentially zeroing out the bed bonus entirely for very tall characters on small beds.

### Hunger and Thirst Effects

Never ignore condition values when calculating recovery. Starving or dehydrated characters receive only one quarter of normal mana and movement gain. This is a right-shift by 2 bits, so the penalty is severe. Intoxication conversely provides a small HP bonus.

### Aquatic Race Handling

Always check wet status for aquatic characters. Wet aquatic characters gain 1.3x HP and movement recovery but only 1x mana. Dry aquatic characters suffer 0.5x recovery on all resources. The AFFECT_WET status must be maintained for optimal recovery.

## Reference

### Symbol Quick Reference

| Symbol | Type | Purpose |
|--------|------|---------|
| `hitGain()` | function | Calculate HP regeneration rate per half-tick |
| `manaGain()` | function | Calculate mana regeneration rate per half-tick |
| `moveGain()` | function | Calculate movement regeneration rate per half-tick |
| `regenTime()` | function | Calculate task update interval from slowest gain rate |
| `updateHalfTickStuff()` | function | Apply global half-tick recovery to character |
| `inCamp()` | function | Check camp status and return skill level bonus |
| `bedRegen()` | function | Apply bed bonus to recovery calculation |
| `canMeditate()` | function | Validate position for meditation skills |
| `TBeing` | class | Base class with recovery methods |
| `TPerson` | class | Player class with specialized hitGain/manaGain |
| `TBed` | class | Furniture providing recovery bonuses |

### Position Recovery Rates

| Position | Task Interval | HP | Mana | Move | Piety | Notes |
|----------|---------------|----|----|------|-------|-------|
| `POSITION_SLEEPING` | `regenTime()` | +1 | +1 | +1 | None | Fastest, vulnerable |
| `POSITION_RESTING` | `2 * regenTime()` | +1 | +1 | +1 | +0.10 | Only position with piety |
| `POSITION_SITTING` | `4 * regenTime()` | +1 | +1 | +1 | None | Slowest task bonus |
| `POSITION_STANDING` | None | None | None | None | None | Half-tick only |
| `POSITION_FIGHTING` | None | None | None | None | None | No recovery |

### Recovery Modifiers

| Factor | HP Effect | Mana Effect | Move Effect |
|--------|-----------|-------------|-------------|
| Constitution | 0.80x to 1.25x | None | 11-41 base |
| Level (max 50) | Up to 2.5x | None | None |
| Drunk | +1 to +9 | None | None |
| Hunger/Thirst | None | 0.25x | 0.25x |
| Poison/Syphilis | None | None | 0.25x |
| Mage class | None | 2x | None |
| Race mana mod | None | Additive | None |
| Hospital room | 2x | None | 2x |
| ROOM_NO_HEAL | None | None | 0.33x |
| Camp | +skill% | None | +skill% |
| Enliven spell | 2x | None | 2x |
| Aquatic (wet) | 1.3x | 1x | 1.3x |
| Aquatic (dry) | 0.5x | 0.5x | 0.5x |

### Meditation Skills

| Skill | Class | Interval | Success Effect | Failure Effect |
|-------|-------|----------|----------------|----------------|
| `SKILL_MEDITATE` | Mage | ~4.8s | Full manaGain() | +1 mana |
| `SKILL_PENANCE` | Cleric/Deikhan | ~6s | pietyGain() with time bonus | +0.6-0.8 piety |
| `SKILL_YOGINSA` | Monk | ~4.8s | 80% HP, 50% mana/move + cures | Standard recovery |

### Camp Terrain Requirements

| Allowed Sectors | Blocked Conditions |
|-----------------|-------------------|
| Forest, Beach, Hill, Mountain | ROOM_ON_FIRE, ROOM_FLOODED |
| Nature, Road, Swamp, Arctic, Cave | ROOM_INDOORS (caves allowed), City sectors |
| | Flying, Underwater, Ocean, River |

### Pulse Timing Constants

| Constant | Value | Real Time |
|----------|-------|-----------|
| `Pulse::ONE_SECOND` | 10 ticks | 1 second |
| `Pulse::MOBACT` | 12 ticks | 1.2 seconds |
| `Pulse::UPDATE` | 360 ticks | 36 seconds |
| `Pulse::MUDHOUR` | 1440 ticks | 144 seconds |

### Key Files

| File | Contents |
|------|----------|
| `limits.cc` | hitGain, manaGain, moveGain, regenTime calculations |
| `periodic.cc` | updateHalfTickStuff half-tick handler |
| `movement.cc` | doSleep, doRest, doSit, doWake commands |
| `task_sleep.cc` | TASK_SLEEP handler |
| `task_rest.cc` | TASK_REST handler |
| `task_meditate.cc` | TASK_MEDITATE handler |
| `task_penance.cc` | TASK_PENANCE handler |
| `disc_monk_meditation.cc` | TASK_YOGINSA handler |
| `disc_advanced_adventuring.cc` | encamp, inCamp functions |
| `obj_bed.cc` | TBed class and bedRegen |

## Implementation

### Half-Tick Recovery Flow

The `updateHalfTickStuff()` function runs every 36 real seconds (Pulse::UPDATE) for all characters. Movement regeneration applies universally to any character that is either an NPC or a PC with an active descriptor connection. Linkdead PCs do not regenerate movement.

HP and mana regeneration have additional prerequisites. The character must not be in combat (checked via `fight()`) and must have an active connection. If the room has the ROOM_NO_HEAL flag, both HP and mana recovery are blocked at this stage. When all conditions pass, the function calls `hitGain()` and `manaGain()` to determine recovery amounts, then applies them directly to the character's current values.

### Position Task Startup

When a player executes sleep, rest, or sit commands, the movement handler validates prerequisites: no flying, no water without boat/aquatic status, no combat, no berserk mode. Sleeping additionally clears the sneak affect as stealth is incompatible with unconsciousness.

After setting the new position, the handler calls `start_task()` with the appropriate task type and interval multiplier. Sleep uses `regenTime()` directly for the fastest interval. Rest doubles the interval. Sit quadruples it. Only PCs receive tasks; NPCs rely solely on half-tick recovery.

### Task Execution Cycle

Each task handler receives control when its interval expires. The handler first calls `calcNextUpdate()` to schedule the next iteration, then checks ROOM_NO_HEAL to determine if recovery should apply.

For standard sleep/rest/sit tasks, each cycle adds +1 to HP, mana, and movement (if below maximum). Rest additionally grants +0.10 piety. The shaman special case intercepts this flow for characters above level 5 who are not shapeshifted or immortal - instead of gaining HP, they lose 1 lifeforce with a warning message. Low-level shamans recover normally.

### HP Gain Calculation

The `hitGain()` function in TPerson calculates player HP recovery. Shamans with zero lifeforce return 0 immediately. Characters in combat also return 0.

Base gain comes from an age graph function that returns values between 2-9 based on character age, plus a flat +4 bonus. Constitution applies a multiplier between 0.80 and 1.25 via `plotStat()`. Level scaling multiplies by max(20, level) then divides by 20, giving up to 2.5x at level 50.

Environmental modifiers apply in sequence: bed regen percentage bonus, drunk bonus (+1 to +9 based on intoxication level), hospital room doubling, camp skill percentage bonus, enliven spell doubling, and aquatic wet/dry modifiers (1.3x wet, 0.5x dry).

### Mana Gain Calculation

The `manaGain()` function returns 0 if the character is fighting or has an active spelltask. Base gain comes from an age graph multiplied by 4. Mages receive a full doubling of this base. Race mana modifiers add a flat bonus.

Hunger and thirst apply a severe penalty: if either condition is zero, gain is right-shifted by 2 bits (divided by 4). Aquatic characters in dry conditions receive only half gain; wet aquatics recover at full rate.

### Movement Gain Calculation

Movement gain in `moveGain()` starts with a constitution-based value between 11 and 41 from `plotStat()`. Bed regen applies silently (no message). Several penalties divide by 4: poison, syphilis, and hunger/thirst.

Camp and enliven bonuses add percentage and doubling respectively. Aquatic modifiers apply (1.3x wet, 0.5x dry). Room flags modify the result: ROOM_NO_HEAL divides by 3, ROOM_HOSPITAL doubles. The final value is multiplied by 4/3 before return.

### Regeneration Time Calculation

The `regenTime()` function determines task update intervals. It starts with a baseline of 100, then compares against each gain rate (HP, mana, movement) for any resource below maximum. The minimum gain rate becomes the bottleneck.

The interval formula divides Pulse::UPDATE by this minimum gain. Higher regeneration rates produce smaller intervals, meaning more frequent task ticks and faster recovery. The minimum interval is 1 pulse to prevent division by zero.

### Camp System

Rangers create camps using the encamp skill in appropriate terrain. Valid sectors include forest, beach, hill, mountain, nature, road, swamp, arctic, and cave. The room must not have fire, flood, no-flee, no-escape, no-heal, or have-to-walk flags. Flying, underwater, ocean, and river sectors are invalid.

A successful skill check creates a camp affect with permanent duration tied to the room. Failed attempts still create a camp but at half skill level. The affect persists until the character leaves the room or dies.

The `inCamp()` function checks for active camp status. It first examines the character's own affects for SKILL_ENCAMP, returning the skill level if found. If not personally camping, it checks for grouped characters in the same room who have camp affects, returning half their skill level (minimum 1) as the bonus.

Camp bonuses apply inside `hitGain()` and `moveGain()` as percentage increases based on the returned skill level. This occurs regardless of position, making camps valuable even for standing characters.

### Bed System

The TBed class represents furniture that provides recovery bonuses. Each bed has properties for minimum position (sleep/rest/sit), maximum users, maximum comfortable size, and regeneration percentage bonus.

The `bedRegen()` function applies when a character in appropriate position is riding the bed object. The bonus adds regen% of the current gain value, minimum 1 point. If the character's height exceeds the bed's max_size plus 6 inches tolerance, a penalty reduces gain based on the height difference divided by 3. The gain cannot go below zero.

### Meditation Skills

Mage meditation through TASK_MEDITATE runs on a 4 * Pulse::MOBACT interval (approximately 4.8 seconds). The `canMeditate()` prerequisite check ensures valid position (resting through standing, or mounted with 50%+ Advanced Riding) and active connection.

On successful skill check, the character gains their full manaGain() value minus 1 (minimum 1). Failure grants only +1 mana. Regardless of success, +1 HP and +1 movement (if below max) apply every cycle.

Cleric/deikhan penance through TASK_PENANCE runs on a 5 * Pulse::MOBACT interval (approximately 6 seconds). A counter increments each cycle, providing a 0.3 multiplier bonus to piety gain that increases over time. Failed checks still grant 0.6-0.8 piety, better than resting's 0.10.

Monk yoginsa through TASK_YOGINSA runs on the same interval as meditation. Success grants 80% of hitGain(), 50% of moveGain(), and 50% of manaGain(). Additionally, the Wohlin skill tree provides automatic curative effects at various thresholds: salve at 20%, cure poison at 35%, sterilize at 50%, cure disease at 60%, clot at 75%, and hunger reduction at 90%.

### Combat Interruption Handling

When a character in a recovery task is attacked, the CMD_TASK_FIGHTING case triggers. The handler sends a warning message, applies 1-2 lost combat rounds (the second round has 33% chance), and calls `stopTask()` to end recovery. This represents the disorientation of being caught in a vulnerable position.

## Troubleshooting

### No HP Recovery While Sleeping

**Symptom:** Character sleeps but HP does not increase over time.

**Likely cause:** Multiple possibilities - shaman lifeforce depletion, ROOM_NO_HEAL flag, or combat state.

**Diagnostic approach:** Check if character is shaman above level 5 with zero lifeforce. Check room flags for ROOM_NO_HEAL. Verify character's fight() returns null. For shamans, check lifeforce value and shapeshifted status.

**Fix:** For shamans, restore lifeforce through spirit consumption. For room issues, move to a different room. For combat state, wait for combat to end naturally.

### Mana Regeneration Slower Than Expected

**Symptom:** Mana recovery takes significantly longer than HP or movement.

**Likely cause:** Hunger or thirst condition at zero, active spelltask, or dry aquatic race.

**Diagnostic approach:** Check FULL and THIRST conditions - either at zero triggers 75% reduction. Check spelltask pointer for active casting. For aquatic races, verify AFFECT_WET status.

**Fix:** Eat and drink to restore conditions above zero. Wait for spelltask completion. For aquatic races, refresh wet status through water exposure.

### Camp Not Providing Bonus

**Symptom:** Ranger has encamped but grouped characters or the ranger see no recovery improvement.

**Likely cause:** Invalid terrain, room flag blocking camp, or group members not in same room.

**Diagnostic approach:** Verify room sector type is valid (forest, hill, etc.). Check for blocking room flags. Confirm group members have AFF_GROUP and share the same room.

**Fix:** Move to valid terrain without blocking flags. Ensure party members are properly grouped and in the same room as the camper.

### Meditation Stops Unexpectedly

**Symptom:** Meditation skill task terminates without message.

**Likely cause:** Position change or linkdead status.

**Diagnostic approach:** Check if character position dropped below resting (stunned, incapacitated). Verify descriptor connection is active. For mounted meditation, confirm Advanced Riding skill is at least 50%.

**Fix:** Restore valid position and restart meditation. Reconnect if linkdead. Improve Advanced Riding skill for mounted meditation.

### Task Interval Seems Wrong

**Symptom:** Recovery ticks occur at unexpected times.

**Likely cause:** Misunderstanding of regenTime() calculation - higher gain rates produce shorter intervals, not longer.

**Diagnostic approach:** Calculate expected interval by dividing Pulse::UPDATE (360) by the character's lowest gain rate. The result is the interval in pulses.

**Fix:** This is usually expected behavior. A character with high regeneration stats will have very short intervals and rapid recovery.

### Bed Providing Reduced or No Bonus

**Symptom:** Character on bed receives less bonus than expected or none at all.

**Likely cause:** Character height exceeds bed max_size, wrong position for bed, or bed full.

**Diagnostic approach:** Compare character height to bed max_size (check for 6-inch tolerance). Verify position meets bed's min_pos_use requirement. Check current bed user count against max_users.

**Fix:** Use a larger bed appropriate for character's race. Adjust position to meet bed requirements. Wait for other users to leave if bed is full.
