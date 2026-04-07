---
title: Rest and Recovery System
description: Regeneration of HP, mana, piety, and movement through half-tick recovery and task-based recovery tied to positions, environmental bonuses, and class abilities.
category: important
keywords: [regeneration, half-tick recovery, task-based recovery, regen bonus stack, penance, yoginsa, camping, hospital room, bed bonus]
primary_symbols:
  functions: [hitGain, manaGain, moveGain, regenTime, updateHalfTickStuff, inCamp, bedRegen, canMeditate, task_regen, processRegen, sendRegenStartupMessage, startRegenTask]
  classes: [TBeing, TPerson, TBed]
  enums: [POSITION_SLEEPING, POSITION_RESTING, POSITION_SITTING, POSITION_STANDING, POSITION_FIGHTING, ROOM_NO_HEAL, ROOM_HOSPITAL, SPELL_ENLIVEN, AFFECT_WET, SKILL_MEDITATE, SKILL_PENANCE, SKILL_YOGINSA, SKILL_ENCAMP, SKILL_WOHLIN, AFF_HIDE, AFF_GROUP, Pulse::ONE_SECOND, Pulse::MOBACT, Pulse::UPDATE, Pulse::MUDHOUR, TASK_SLEEP, TASK_REST, TASK_SIT, TASK_MEDITATE, TASK_PENANCE, TASK_YOGINSA, PERMANENT_DURATION, RegenTaskType, CLASS_THIEF]
---

## Overview

How do characters recover from damage, restore spent mana, and regain stamina after exertion? The rest and recovery system manages regeneration of HP, mana, and movement through multiple overlapping mechanisms.

Recovery in SneezyMUD operates on two parallel tracks: **half-tick recovery** applies globally to all characters every 36 seconds regardless of activity, while **task-based recovery** provides bonus regeneration tied to specific positions (sleeping, resting, sitting) and class abilities (meditation, penance, yoginsa). Environmental factors like beds, camps, hospital rooms, and the regen bonus stack layer additional bonuses on top of these base rates.

The system balances recovery speed against vulnerability and capability. Sleeping provides the fastest regeneration for most classes but leaves the character defenseless and unable to act. Resting offers good recovery while permitting limited actions. Sitting is slower still but the lightest commitment. Standing gives only half-tick recovery but full combat readiness. Thieves get a class-specific multiplier that doubles their effective regen rate while sitting or resting, putting their rest recovery on par with sleep for other classes. This creates meaningful tactical decisions about when and where to recover.

Recovery rates scale with level, constitution, and environmental bonuses. A level 50 character in a hospital bed with the enliven spell active recovers dramatically faster than a low-level character standing in hostile territory. Class-specific abilities provide additional recovery options: mages meditate for mana, clerics perform penance for piety, monks practice yoginsa for enhanced healing with bonus curative effects.

A wounded warrior returns from battle to the inn. Lying down to sleep triggers a task that grants +1 HP, +1 mana, +0.10 piety, and +1 movement on each cycle, plus +1 HP and +1 movement for each active regen bonus (camp, campfire, groupmate in the room, and so on). Simultaneously, every 36 seconds the global half-tick adds their full hitGain() and manaGain() values. If they sleep on a bed in a hospital room, both multipliers stack. After several minutes, they wake fully restored and ready for the next adventure.

## Patterns

### Position Selection

For most classes, sleeping is the fastest recovery option when safety is assured, with rest at half that rate and sit at a quarter. Resting and sitting let the character remain aware of their surroundings and respond to events. Thieves are an exception - their `regenMod` divides by 2, so a thief resting recovers as fast as another class sleeping, and a thief sitting recovers as fast as another class resting. A thief therefore has little reason to ever sleep outside of a bed.

Never attempt to sleep or rest while in combat, flying, or in water without aquatic adaptation. The system rejects these attempts and wastes the player's action. Check these conditions before allowing position changes.

### Task Interval Calculation

Never assume fixed task intervals. The `regenTime()` function calculates intervals dynamically based on the character's current regeneration rates. Higher regeneration rates produce shorter intervals. The formula divides the update pulse length by the slowest of the character's HP, mana, and movement gain rates.

### ROOM_NO_HEAL Behavior

`ROOM_NO_HEAL` blocks nearly all recovery. The flag stops task-based regeneration completely (the regen task aborts the tick early before applying gains or evaluating the bonus stack) and blocks half-tick HP and mana recovery. Only movement recovery continues, reduced by two-thirds. Characters in these rooms cannot recover HP or mana through any passive mechanism.

### Camp Application

Camp creates two distinct recovery effects that stack. The percentage bonus from `inCamp()` applies inside `hitGain()` and `moveGain()`, scaling the half-tick recovery rate regardless of position - a camped character receives it whether standing, sitting, or sleeping, with the bonus scaling by the camper's skill level (groupmates receive half). Separately, the regen task bonus stack adds a flat +1 HP and +1 move per tick whenever a character is sitting, resting, or sleeping in a camped room. A camped character resting therefore benefits from both: the percentage-scaled half-tick gain and the flat per-tick bonus.

### Combat Interruption

Being attacked in a recovery position stops the task and imposes a round-loss penalty that varies by position: rest loses 1 round plus a 33% chance of a second, sleep loses 1 round plus a 50% chance of a second, and sit has only a 33% chance of losing a single round. Sleep's penalty is harshest because the character is unconscious when attacked; sit is lightest because the character remains aware of their surroundings.

### Piety Recovery

All three recovery positions grant a nominal +0.10 piety per tick through the unified regen handler. This is a small baseline - real piety recovery for clerics and deikhan comes from the penance skill, which scales with duration.

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
| `task_regen()` | function | Unified handler for TASK_SLEEP/REST/SIT |
| `processRegen()` | function | Per-tick body that applies the bonus stack and updates resources |
| `startRegenTask()` | function | Common entry point for `doSit`/`doRest`/`doSleep` and the bed equivalents |
| `sendRegenStartupMessage()` | function | Print the active bonus list when a regen task begins |
| `RegenTaskType` | enum | Selector for the per-task config map; values alias `positionTypeT` |
| `TBeing` | class | Base class with recovery methods |
| `TPerson` | class | Player class with specialized hitGain/manaGain |
| `TBed` | class | Furniture providing recovery bonuses |

### Position Recovery Rates

All three recovery positions (`SLEEPING`, `RESTING`, `SITTING`) share a single unified task handler in `task_regen_common.cc`. Each tick grants +1 HP, +1 mana, +0.10 piety, and +1 move (up to `moveLimit()`). The bonus system adds +1 to HP/move per active bonus source (see below). Sleep suppresses bonus flavor messages because the character is unconscious.

| Position | Task Interval (non-thief) | Task Interval (thief) | HP | Mana | Move | Piety | Bonuses Apply |
|----------|---------------------------|-----------------------|----|------|------|-------|---------------|
| `POSITION_SLEEPING` | `regenTime()` | `regenTime()` | +1 | +1 | +1 | +0.10 | Yes (silent) |
| `POSITION_RESTING` | `2 * regenTime()` | `regenTime()` | +1 | +1 | +1 | +0.10 | Yes |
| `POSITION_SITTING` | `4 * regenTime()` | `2 * regenTime()` | +1 | +1 | +1 | +0.10 | Yes |
| `POSITION_STANDING` | None | None | None | None | None | None | Half-tick only |
| `POSITION_FIGHTING` | None | None | None | None | None | None | No recovery |

Thieves divide their tick rate multiplier by 2 (floored at 1), effectively doubling their regen rate while awake. Sleep is unaffected because it is already at the floor.

### Regen Bonuses

Each of the following conditions adds +1 to HP and move regen per tick while a regen task is active. They stack - a character in a camp with a hidden groupmate next to a campfire on home terrain would accumulate all applicable bonuses. Rest and sit emit a 20% chance flavor message per tick per active bonus; sleep suppresses them.

| Condition | Check |
|-----------|-------|
| Camp | `inCamp()` (own or groupmate's) |
| Groupmate in room | `hasGroupmateInRoom()` |
| Campfire in room | `roomp->hasCampfire()` |
| Hidden from view | `isAffected(AFF_HIDE)` |
| Home terrain | `homeTurf()` (race matches sector) |
| Background experience | `backgroundBonus()` (background matches sector) |

A summary of active bonuses is sent to the character when the task starts via `sendRegenStartupMessage()`, which all three position commands (`doRest`, `doSleep`, `doSit`) call after `setPosition()`.

### Position Command Restrictions

Common restrictions across doSleep, doRest, and doSit:
- Cannot enter position while flying
- Cannot enter position while fighting or berserking
- Cannot sleep or rest in water sectors without being aquatic or using a boat
- ROOM_NO_HEAL prevents task-based recovery but allows position change
- All three commands clear the sneak affect on entry. Hide handling is covered in the Hide Interaction section under Implementation.

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

### Condition Effects

| Condition | Check | Effect |
|-----------|-------|--------|
| Hunger | `FULL == 0` | Quarters mana and move gain |
| Thirst | `THIRST == 0` | Quarters mana and move gain |
| Drunk | `DRUNK > 0` | Adds 1 + (drunk/3) to HP gain |
| Poison | `AFF_POISON` | Quarters move gain |
| Syphilis | `AFF_SYPHILIS` | Quarters move gain |

### Spell Effects on Recovery

| Spell/Affect | Effect |
|--------------|--------|
| `SPELL_ENLIVEN` | Doubles HP and move gain |
| `AFFECT_WET` | Aquatic races gain 1.3x HP, 1x mana, and 1.3x move; without this, aquatic races suffer 0.5x penalty on all resources |

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

Defined in `code/code/sys/comm.h`:

| Constant | Value | Real Time |
|----------|-------|-----------|
| `Pulse::ONE_SECOND` | 10 ticks | 1 second |
| `Pulse::MOBACT` | 12 ticks | 1.2 seconds |
| `Pulse::UPDATE` | 360 ticks | 36 seconds |
| `Pulse::MUDHOUR` | 1440 ticks | 144 seconds |

Task interval examples:
- TASK_SLEEP: `regenTime()` pulses (varies, typically 10-60 pulses)
- TASK_REST: `2 * regenTime()` pulses
- TASK_SIT: `4 * regenTime()` pulses
- TASK_MEDITATE: `4 * Pulse::MOBACT` = 48 pulses = 4.8 seconds
- TASK_PENANCE: `5 * Pulse::MOBACT` = 60 pulses = 6 seconds
- TASK_YOGINSA: `4 * Pulse::MOBACT` = 48 pulses = 4.8 seconds

### Gain Function Formulas

**hitGain** (`code/code/misc/limits.cc`):
1. Base = graf(age, 2, 4, 5, 9, 4, 3, 2) + 4
2. Constitution multiplier: 0.80 to 1.25 via plotStat
3. Level scaling: gain *= max(20, GetMaxLevel()) / 20
4. Bed bonus: gain += max(1, regen * gain / 100)
5. Drunk bonus: gain += 1 + (drunk_level / 3)
6. Hospital: gain *= 2
7. Camp: gain += gain * campLevel / 100
8. Enliven: gain *= 2
9. Aquatic: gain *= (wet ? 1.3 : 0.5)
10. Shamans with zero lifeforce: return 0
11. Combat: return 0

**manaGain** (`code/code/misc/limits.cc`):
1. Base = graf(age, 2, 4, 6, 8, 10, 12, 14) * 4
2. Mage class: gain += gain (double)
3. Race modifier: gain += race->getManaMod()
4. Hunger/thirst: gain >>= 2 (quarter if either empty)
5. Aquatic: gain *= (wet ? 1 : 0.5)
6. Combat or spellcasting (spelltask): return 0

**moveGain** (`code/code/misc/limits.cc`):
1. Base = plotStat(STAT_CON, 11, 41, 30)
2. Bed bonus: gain += max(1, regen * gain / 100)
3. Poison: gain >>= 2
4. Syphilis: gain >>= 2
5. Hunger/thirst: gain >>= 2 (quarter if either empty)
6. Camp: gain += gain * campLevel / 100
7. Enliven: gain *= 2
8. Aquatic: gain *= (wet ? 1.3 : 0.5)
9. ROOM_NO_HEAL: gain /= 3
10. ROOM_HOSPITAL: gain *= 2
11. Final scaling: gain = (gain * 4) / 3

**regenTime** (`code/code/misc/limits.cc`):
1. Start with iTime = 100
2. If HP below max: iTime = min(iTime, hitGain())
3. If move below max: iTime = min(iTime, moveGain())
4. If mana below max: iTime = min(iTime, manaGain())
5. iTime = max(iTime, 1) (floor at 1)
6. Return Pulse::UPDATE / iTime (inverse relationship)

### Camp System Functions

**encamp** (`code/code/disc/disc_advanced_adventuring.cc`): Creates PERMANENT_DURATION affect with skill_level as affect level. On skill failure, affect level is halved. Validates terrain type. Blocks indoor camping, city camping, and hazardous room flags (fire, flooded, no_flee, no_escape, no_heal, have_to_walk). Blocks flying, underwater, ocean, and river sectors.

**inCamp** (`code/code/disc/disc_advanced_adventuring.cc`): Scans character's affects for SKILL_ENCAMP and returns affect level. If character is grouped (AFF_GROUP), scans all roommates for SKILL_ENCAMP and returns max(1, affect_level / 2) for groupmate camps. Returns FALSE if no camp found.

### Key Files

| File | Contents |
|------|----------|
| `limits.cc` | hitGain, manaGain, moveGain, regenTime calculations |
| `periodic.cc` | updateHalfTickStuff half-tick handler |
| `movement.cc` | doSleep, doRest, doSit, doWake commands |
| `task_regen_common.cc` | Unified `task_regen` handler, `bonusChecks` array, `sendRegenStartupMessage` helper, per-task `RegenTaskConfig` map |
| `task_regen_common.h` | `RegenTaskType` enum, public declarations |
| `task_sleep.cc` | TASK_SLEEP thin delegation to `task_regen` |
| `task_rest.cc` | TASK_REST thin delegation to `task_regen` |
| `task_sit.cc` | TASK_SIT thin delegation to `task_regen` |
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

When a player executes sleep, rest, or sit commands, the movement handler validates prerequisites: no flying, no water without boat/aquatic status, no combat, no berserk mode. All three call `loseSneak()` on entry to clear the sneak affect.

After setting the new position, the handler calls `startRegenTask()` (declared in `task_regen_common.h`), which emits a summary of any active regen bonuses via `sendRegenStartupMessage()` and then invokes `start_task()` with the canonical parameter set for the requested `RegenTaskType`. The bed object methods (`TBed::sitMe`, `restMe`, `sleepMe`) call `startRegenTask` the same way, so resting on a bedroll or other bed object also produces the bonus list. The initial task interval is the base `regenTime()` (sleep) or a multiple of it (rest doubles, sit quadruples) - the per-task config carries the multiplier, and the same value is reused on every subsequent tick via `calcNextUpdate()`. Only PCs receive tasks; NPCs rely solely on half-tick recovery.

### Hide Interaction

The Hidden entry in the regen bonus stack tests `isAffected(AFF_HIDE)` on each tick, so a hidden character benefits from the bonus only as long as the affect remains set when the tick fires. Two code paths govern whether hide survives entering a recovery position:

1. **`willBreakHide()`** in `disc_thief_stealth.cc` lists commands that do *not* break hide. `CMD_REST`, `CMD_SIT`, and `CMD_STAND` are on the do-not-break list, so a hidden thief can drop into rest or sit without losing the affect. `CMD_SLEEP` is not on the list, so issuing the sleep command strips `AFF_HIDE` before the regen task starts.
2. The hide/break check runs in `parseCommand()` (`parse.cc`), not inside `doRest`/`doSit`/`doSleep`. By the time the position handler runs, hide has already been preserved or cleared by the parser. `loseSneak()` inside the position handlers only touches the sneak affect, not hide.

Combined effect: a hidden thief who rests or sits keeps the Hidden bonus indefinitely (until they take an action that does break hide, e.g. backstab or any non-listed command). A hidden character who sleeps loses hide before the task starts and never receives the bonus, which is consistent with the regen task suppressing all bonus flavor messages during sleep.

### Task Execution Cycle

All three position tasks delegate to `task_regen()` in `task_regen_common.cc`, which reads a per-task `RegenTaskConfig` from a compile-time map keyed by `RegenTaskType` (whose enumerators alias the corresponding `positionTypeT` values). The config carries the tick rate multiplier, the silence flag, the combat-interruption lambda, and the task-specific command handler (for CMD_SIT, CMD_REST, CMD_WAKE, etc.).

On `CMD_TASK_CONTINUE`, `processRegen()` runs:

1. `calcNextUpdate()` schedules the next tick using `regenMod * regenTime()`, where `regenMod = max(tickRateMultiplier / (thief ? 2 : 1), 1)`.
2. If the room has `ROOM_NO_HEAL`, the function returns early without applying any gains or evaluating the bonus stack.
3. `regenAmt` starts at 1. Each entry in the `bonusChecks` array is evaluated; active bonuses add +1 to `regenAmt` and (if not silent) have a 20% chance to emit their flavor message.
4. HP gains `regenAmt`. Move gains `min(regenAmt, moveLimit - getMove())`, guarded to avoid decrementing when current move exceeds the cap (which can happen via potions, aegis, or vampire drain). Mana gains +1 and piety gains +0.10.
5. `updatePos()` runs and the GMCP/ANSI/VT100 screen updates fire.

Bonuses only affect HP and move - mana and piety gains are fixed regardless of how many bonuses are active.

On `CMD_TASK_FIGHTING`, the config's `applyFightingPenalty` lambda runs and the task is stopped. On unknown commands, the config's `cmdHandler` lambda runs; if it handles the command it returns true, otherwise the helper falls through to `cmd >= MAX_CMD_LIST` to distinguish utility commands (let the caller process them) from task-eating commands.

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

### Camp Affect Storage

The encamp skill creates an affect structure with duration set to PERMANENT_DURATION (-1), level set to skill_level, type set to SKILL_ENCAMP, and the `be` pointer set to the character's current room pointer. This room linkage prevents the camp from traveling with the character. When the character moves, the camp remains in the original room. The `inCamp()` function compares the affect's `be` pointer to the character's current `roomp`. If they differ, the camp bonus does not apply.

### Bed System

The TBed class represents furniture that provides recovery bonuses. Each bed has properties for minimum position (sleep/rest/sit), maximum users, maximum comfortable size, and regeneration percentage bonus.

The `bedRegen()` function applies when a character in appropriate position is riding the bed object. The bonus adds regen% of the current gain value, minimum 1 point. If the character's height exceeds the bed's max_size plus 6 inches tolerance, a penalty reduces gain based on the height difference divided by 3. The gain cannot go below zero.

When a character rests on a bed, the `riding` pointer points to the bed object. During recovery calculation, the code checks if `riding` is set and the object is a bed type, then calls `bed->bedRegen()`. Multiple users can share a bed up to its max_users capacity, enforced by counting characters with `riding == this_bed` in the room's stuff list.

### Meditation Skills

Mage meditation through TASK_MEDITATE runs on a 4 * Pulse::MOBACT interval (approximately 4.8 seconds). The `canMeditate()` prerequisite check ensures valid position (resting through standing, or mounted with 50%+ Advanced Riding) and active connection.

On successful skill check, the character gains their full manaGain() value minus 1 (minimum 1). Failure grants only +1 mana. Regardless of success, +1 HP and +1 movement (if below max) apply every cycle.

Cleric/deikhan penance through TASK_PENANCE runs on a 5 * Pulse::MOBACT interval (approximately 6 seconds). A counter increments each cycle, providing a 0.3 multiplier bonus to piety gain that increases over time. Failed checks still grant 0.6-0.8 piety, far better than the nominal 0.10 granted by the generic regen tasks.

Monk yoginsa through TASK_YOGINSA runs on the same interval as meditation. Success grants 80% of hitGain(), 50% of moveGain(), and 50% of manaGain(). Additionally, the Wohlin skill tree provides automatic curative effects at various thresholds: salve at 20%, cure poison at 35%, sterilize at 50%, cure disease at 60%, clot at 75%, and hunger reduction at 90%.

For SKILL_YOGINSA, success requires two rolls: bSuccess against the SKILL_YOGINSA learn percentage, then a second roll against (70 + wohlin_learn / 4). This creates a difficulty curve where yoginsa becomes more reliable as wohlin skill increases.

### Combat Interruption Handling

When a character in a recovery task is attacked, the `CMD_TASK_FIGHTING` branch of `task_regen` sends a position-appropriate warning (`"You are unable to fight while %s!"` with the task description), invokes the task's `applyFightingPenalty` lambda, and calls `stopTask()`. Each position configures its own penalty lambda, which lets the three tasks impose different costs for the same interruption (see the Combat Interruption pattern above for the specific probabilities).

## Troubleshooting

### No HP Recovery While Sleeping

**Symptom:** Character sleeps but HP does not increase over time.

**Likely cause:** `ROOM_NO_HEAL` flag or active combat. The unified regen task delivers HP/mana/move/piety regardless of class - it does not branch on the character class. (For half-tick HP recovery specifically, see the early-return guards listed under HP Gain Calculation.)

**Diagnostic approach:** Check room flags for `ROOM_NO_HEAL`. Verify character's `fight()` returns null. If the regen task ticks are firing but the visible bar is not moving, confirm the screen update path (GMCP/ANSI/VT100) is reaching the descriptor.

**Fix:** Move out of the no-heal room or wait for combat to end.

### Mana Regeneration Slower Than Expected

**Symptom:** Mana recovery takes significantly longer than HP or movement.

**Likely cause:** Hunger or thirst condition at zero, active spelltask, or dry aquatic race.

**Diagnostic approach:** Check FULL and THIRST conditions - either at zero triggers 75% reduction. Check spelltask pointer for active casting. For aquatic races, verify AFFECT_WET status.

**Fix:** Eat and drink to restore conditions above zero. Wait for spelltask completion. For aquatic races, refresh wet status through water exposure.

### Camp Not Providing Bonus

**Symptom:** Ranger has encamped but grouped characters or the ranger see no recovery improvement.

**Likely cause:** Invalid terrain, room flag blocking camp, group members not in same room, or character moved after camping.

**Diagnostic approach:** Verify room sector type is valid (forest, hill, etc.). Check for blocking room flags. Confirm group members have AFF_GROUP and share the same room. Check if the camp affect's room pointer matches the current room.

**Fix:** Move to valid terrain without blocking flags. Ensure party members are properly grouped and in the same room as the camper. Recast encamp if you moved since creating the camp.

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

### updateHalfTickStuff Not Running

**Symptom:** Character receives no passive regeneration. Position tasks work but no recovery occurs during movement or standing.

**Likely cause:** Character is linkdead (desc is null) and is a PC. The linkdead check blocks HP and mana recovery for disconnected players. Another cause is fighting state, which blocks HP and mana recovery.

**Diagnostic approach:** Check `isPc() && desc` boolean. Linkdead players show disconnected in the who list. For fighting block, check if `fight()` returns non-null.

**Fix:** Reconnect to restore descriptor. End combat by fleeing, killing the opponent, or becoming incapacitated. Movement recovery continues during combat, so if movement regenerates but HP/mana don't, combat is the likely cause.

### Camp Skill Fails in Valid Terrain

**Symptom:** Encamp command returns failure message despite being in wilderness forest room with no apparent restrictions.

**Likely cause:** Room has hazardous flags (ROOM_ON_FIRE, ROOM_FLOODED, ROOM_NO_FLEE, ROOM_NO_ESCAPE, ROOM_NO_HEAL, ROOM_HAVE_TO_WALK) that block camping. Another cause is being in a water sector or the room being marked as indoor.

**Diagnostic approach:** Check room flags. Verify room's sector type isn't city, water, or indoor. For caves, check if ROOM_INDOORS flag is set (caves are allowed but indoor rooms are not).

**Fix:** Remove hazardous flags if unintentional. Use appropriate sector type. For underwater or ocean sectors, surface to a shore. Valid sectors: forest, beach, hill, mountain, nature, road, swamp, arctic, cave.

### Yoginsa Not Providing Wohlin Healing

**Symptom:** TASK_YOGINSA runs successfully and grants HP/mana/move, but expected healing effects (salve, cure poison, etc.) don't trigger.

**Likely cause:** Wohlin skill level below required threshold. Each healing effect requires minimum wohlin percentage: salve (20%), cure poison (35%), sterilize (50%), cure disease (60%), clot (75%), reduce hunger (90%). The healing effects only attempt if the character passes the yoginsa success roll AND the secondary (70 + wohlin_learn / 4) percentage roll.

**Fix:** Increase SKILL_WOHLIN through practice. At 0% wohlin, no healing effects occur. At 90% wohlin, all effects can trigger. Each healing effect also applies its own internal skill check.
