---
title: Rest and Recovery System
category: important
keywords: [hitGain, manaGain, moveGain, regenTime, updateHalfTickStuff, TASK_SLEEP, TASK_REST, meditation, camp, bed regeneration]
related: [position-stance.md, scheduler-pulses.md, task-system.md, affects-system.md]
primary_symbols:
  functions: [hitGain, manaGain, moveGain, regenTime, updateHalfTickStuff, doSleep, doRest, doSit, task_sleep, task_rest, encamp, inCamp, bedRegen, canMeditate]
  classes: [TBeing, TPerson, TBed]
  files: [code/code/misc/limits.cc, code/code/misc/periodic.cc, code/code/misc/movement.cc, code/code/task/task_sleep.cc, code/code/task/task_rest.cc, code/code/obj/obj_bed.cc]
---

# Rest and Recovery System

## Overview

SneezyMUD uses a multi-layered regeneration system combining position-based mechanics with global tick recovery. Characters recover HP, mana, movement, piety, and lifeforce through two primary channels: task-based recovery triggered by position changes (sleep/rest/sit commands), and half-tick recovery applied globally every 36 seconds. Position-based recovery runs faster than half-tick updates, making voluntary rest more efficient than passive regeneration.

Recovery rates scale with character attributes, class, level, and environmental factors. Constitution drives HP and movement recovery. Mages double their mana gains. Shamans above level 5 drain lifeforce instead of recovering HP during rest. Environmental modifiers include beds, camps, hospitals, and the ROOM_NO_HEAL flag which blocks task-based recovery. Specialized skills like meditate, penance, and yoginsa provide class-specific enhanced recovery.

The system is implemented through position tasks (TASK_SLEEP, TASK_REST, TASK_SIT) and the half-tick scheduler function updateHalfTickStuff. Recovery rate calculations live in the gain functions (hitGain, manaGain, moveGain) while task timing is computed by regenTime, which creates an inverse relationship between recovery speed and task interval duration.

## Patterns

### Position-Based Recovery Loop

Position commands (doSleep, doRest, doSit) validate state, set character position, and start a periodic task with an interval computed by regenTime. Each task cycle grants one point to each stat unless blocked by ROOM_NO_HEAL. Sleep uses the base regenTime interval, rest uses 2x that interval, and sit uses 4x. This creates a hierarchy where sleep recovers fastest but leaves you most vulnerable, while sitting provides minimal bonus with more safety.

The rest task is unique in granting 0.10 piety per cycle, making it the only position that restores divine energy. Shamans above level 5 drain one lifeforce per rest/sleep cycle instead of gaining HP, requiring them to hunt actively or risk becoming immobile. When attacked during rest, characters lose 1-2 combat rounds (cantHit penalty) and their task immediately stops.

### Half-Tick Recovery Broadcast

Every 36 seconds (Pulse::UPDATE), the scheduler calls updateHalfTickStuff on all characters. Movement always regenerates via moveGain unless the character is linkdead and not a PC. HP and mana only regenerate if not fighting and not in a ROOM_NO_HEAL room. Linkdead players receive no HP/mana recovery. The half-tick system runs independently of position tasks, providing a baseline recovery rate even while standing or moving.

### Skill-Based Enhanced Recovery

Mages can meditate (SKILL_MEDITATE) while resting or standing, running every 4.8 seconds. On success, they gain manaGain()-1 mana (minimum 1). On failure, they gain 1 mana (equivalent to resting). Both outcomes grant 1 HP and 1 move.

Clerics and Deikhan use penance (SKILL_PENANCE), which restores piety at 6-second intervals. Piety gain increases the longer you maintain penance, with a growing 0.3*timeLeft bonus. On failure, you still gain 0.6-0.8 piety, making it far superior to rest's 0.10 per cycle.

Monks have yoginsa (SKILL_YOGINSA), running every 4.8 seconds. On success, they recover 80% of hitGain for HP, and 50% of manaGain and moveGain for the respective stats. High-level yoginsa practitioners with wohlin training automatically receive healing effects like salve, cure poison, sterilize, cure disease, clot, and reduce hunger at specific skill breakpoints.

Meditation requires POSITION_RESTING or better (tested via canMeditate). Characters can meditate while mounted if their Advanced Riding skill is at least 50%. All meditation skills share the constraint that ROOM_NO_HEAL blocks their recovery effects.

### Camp Territory Bonus

Rangers use encamp (SKILL_ENCAMP) to establish a camp in wilderness terrain, creating a PERMANENT_DURATION affect tied to the character and linked to the current room. The camp skill level determines the bonus percentage. Camp restrictions prevent indoor camping, city camping, or camping in hazardous rooms (fire, flooded, no_heal, no_flee).

When checking recovery, inCamp scans the character's affects for SKILL_ENCAMP and returns the skill level. If the character is grouped, it also checks roommates for camp affects and returns half the camper's skill level. The camp bonus applies as a percentage multiplier to hitGain and moveGain, increasing both HP and movement recovery. Camp bonuses stack with all other modifiers and apply during half-tick recovery regardless of position.

### Environmental Multipliers

Beds provide a regeneration percentage bonus based on their regen value, applied during bedRegen when the character is sleeping, resting, or sitting. If the character's height exceeds the bed's max_size by more than 6 inches, a penalty subtracts recovery based on the size mismatch. Beds can serve multiple users up to their max_users capacity.

Hospital rooms (ROOM_HOSPITAL) double HP and movement recovery during half-tick updates. The ROOM_NO_HEAL flag blocks all task-based recovery (sleep, rest, sit, meditation) and reduces moveGain by two-thirds. Aquatic races suffer a 0.5x multiplier when dry but gain 1.3x when affected by AFFECT_WET. The Enliven spell doubles HP and movement gain. Hunger and thirst quarter both mana and move gain when either condition is empty. Poison and syphilis quarter move gain.

### Recovery Rate Calculation

Each gain function computes a base value from character attributes, then applies multiplicative and additive modifiers. The hitGain function for players starts with an age-based graph value (graf function), adds 4, multiplies by a constitution curve (0.80 to 1.25), then scales by level (max(20, GetMaxLevel()) / 20). Additional modifiers include drunk bonus (+1 to +9), hospital doubling, camp percentage, enliven doubling, and aquatic wet/dry multipliers. Shamans with zero lifeforce return 0 HP gain. Combat blocks HP recovery entirely.

The manaGain function uses an age graph, multiplies by 4, then doubles the result for mages. Race modifiers adjust the value. Hunger or thirst reduces gain to 25%. Aquatic races without AFFECT_WET suffer a 0.5x penalty. Combat and active spellcasting (spelltask) block mana recovery.

The moveGain function uses a constitution-based plot (11 to 41 points at 30 default). Poison, syphilis, hunger, and thirst each quarter the gain. Camp, enliven, and aquatic modifiers apply. ROOM_NO_HEAL divides gain by 3, while ROOM_HOSPITAL doubles it. The final result is multiplied by 4/3.

### Task Interval Calculation

The regenTime function determines how frequently position tasks update. It starts with a base of 100, then takes the minimum of hitGain, moveGain, and manaGain (if the respective stat is below max). This identifies the slowest regeneration rate. The function then divides Pulse::UPDATE (360 pulses = 36 seconds) by this value to compute the task interval. Higher gain rates produce shorter intervals, creating faster task cycles. This inverse relationship ensures characters recover at a rate proportional to their stats, with better constitution or mana regen producing more frequent updates.

## Reference

### Position Recovery Rates

Position hierarchy from fastest to slowest recovery:
- POSITION_SLEEPING: regenTime() interval, +1 HP/mana/move per cycle
- POSITION_RESTING: 2 * regenTime() interval, +1 HP/mana/move, +0.10 piety per cycle
- POSITION_SITTING: 4 * regenTime() interval, +1 HP/mana/move per cycle
- POSITION_STANDING: half-tick only (no task bonus)
- POSITION_FIGHTING: no recovery

Sleep provides the fastest recovery but leaves characters vulnerable (1-2 round cantHit penalty when attacked). Rest is the only position that restores piety. Sitting provides minimal bonus over standing. All positions except standing and fighting trigger task-based recovery.

### Position Command Restrictions

Common restrictions across doSleep, doRest, and doSit:
- Cannot enter position while flying
- Cannot enter position while fighting or berserking
- Cannot sleep or rest in water sectors without being aquatic or using a boat
- ROOM_NO_HEAL prevents task-based recovery but allows position change
- Sleep loses sneak status automatically

### Task Handlers

TASK_SLEEP handler: Runs every regenTime() pulses. Grants +1 mana, +1 HP (except shamans above level 5), +1 move (if below max). Shamans drain -1 lifeforce per cycle. Combat interruption (CMD_TASK_FIGHTING) stops the task and applies 1-2 round cantHit penalty. Implemented in code/code/task/task_sleep.cc.

TASK_REST handler: Runs every 4 * regenTime() pulses. Grants +1 mana, +0.10 piety, +1 HP (or -1 lifeforce for shamans), +1 move (if below max). Combat interruption identical to sleep. Implemented in code/code/task/task_rest.cc.

TASK_SIT handler: Runs every 4 * regenTime() pulses. Grants +1 mana, +1 HP, +1 move (if below max). Combat interruption identical to sleep/rest. Implemented in code/code/task/task_sit.cc.

TASK_MEDITATE handler: Runs every 4 * Pulse::MOBACT (approximately 4.8 seconds). On success, grants manaGain()-1 mana (minimum 1). On failure, grants +1 mana. Always grants +1 HP and +1 move. Requires canMeditate (resting/standing position, or mounted with Advanced Riding 50+). Implemented in code/code/task/task_meditate.cc.

TASK_PENANCE handler: Runs every 5 * Pulse::MOBACT (approximately 6 seconds). On success, grants pietyGain(0.3 * timeLeft). On failure, grants 0.6-0.8 piety. The timeLeft counter increments each cycle, providing increasing returns. Implemented in code/code/task/task_penance.cc.

TASK_YOGINSA handler: Runs every 4 * Pulse::MOBACT. On success, grants 80% of hitGain for HP, 50% of manaGain for mana, 50% of moveGain for movement. Requires secondary success roll against (70 + wohlin_learn/4) percentage. Wohlin skill levels unlock automatic healing effects: salve at 20%, cure poison at 35%, sterilize at 50%, cure disease at 60%, clot at 75%, reduce hunger at 90%. Implemented in code/code/disc/disc_monk_meditation.cc.

### Gain Function Formulas

hitGain for players (code/code/misc/limits.cc):
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

manaGain for players (code/code/misc/limits.cc):
1. Base = graf(age, 2, 4, 6, 8, 10, 12, 14) * 4
2. Mage class: gain += gain (double)
3. Race modifier: gain += race->getManaMod()
4. Hunger/thirst: gain >>= 2 (quarter if either empty)
5. Aquatic: gain *= (wet ? 1 : 0.5)
6. Combat or spellcasting: return 0

moveGain for all beings (code/code/misc/limits.cc):
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

regenTime calculation (code/code/misc/limits.cc):
1. Start with iTime = 100
2. If HP below max: iTime = min(iTime, hitGain())
3. If move below max: iTime = min(iTime, moveGain())
4. If mana below max: iTime = min(iTime, manaGain())
5. iTime = max(iTime, 1) (floor at 1)
6. Return Pulse::UPDATE / iTime (inverse relationship)

### Camp System Functions

encamp (code/code/disc/disc_advanced_adventuring.cc): Creates PERMANENT_DURATION affect with skill_level as affect level. On skill failure, affect level is halved. Validates terrain type (nature, forest, beach, hill, mountain, road, swamp, arctic, cave only). Blocks indoor camping, city camping, and hazardous room flags (fire, flooded, no_flee, no_escape, no_heal, have_to_walk). Blocks flying, underwater, ocean, and river sectors.

inCamp (code/code/disc/disc_advanced_adventuring.cc): Scans character's affects for SKILL_ENCAMP and returns affect level. If character is grouped (AFF_GROUP), scans all roommates for SKILL_ENCAMP and returns max(1, affect_level / 2) for groupmate camps. Returns FALSE if no camp found. Camp level directly determines the percentage bonus to hitGain and moveGain.

### Environmental Modifiers

ROOM_NO_HEAL: Blocks all task-based recovery (TASK_SLEEP, TASK_REST, TASK_SIT, TASK_MEDITATE, TASK_PENANCE, TASK_YOGINSA). Reduces moveGain by 2/3 during half-tick updates. Does not block position changes.

ROOM_HOSPITAL: Doubles HP and movement recovery during half-tick updates. Does not affect mana or piety. Stacks multiplicatively with other modifiers.

Bed objects (TBed class, code/code/obj/obj_bed.cc): Provides regen percentage bonus via bedRegen function. Applies when character position is sleeping, resting, or sitting. Size penalty applies when (character_height - 6) > bed_max_size, reducing gain by max(1, (height - max_size) / 3). Multiple users can share a bed up to max_users capacity.

Condition effects:
- Hunger (FULL == 0): Quarters mana and move gain
- Thirst (THIRST == 0): Quarters mana and move gain
- Drunk (DRUNK > 0): Adds 1 + (drunk/3) to HP gain
- Poison (AFF_POISON): Quarters move gain
- Syphilis (AFF_SYPHILIS): Quarters move gain

Spell effects:
- SPELL_ENLIVEN: Doubles HP and move gain
- AFFECT_WET: Aquatic races gain 1.3x HP/mana/move; without this, aquatic races suffer 0.5x penalty

### Pulse Timing Constants

Defined in code/code/sys/comm.h:
- Pulse::ONE_SECOND = 10 ticks = 1 real second
- Pulse::UPDATE = 360 ticks = 36 real seconds (half-tick interval)
- Pulse::MUDHOUR = 1440 ticks = 144 real seconds = 4 updates
- Pulse::MOBACT = 12 ticks = 1.2 real seconds

Task interval examples:
- TASK_SLEEP: regenTime() pulses (varies, typically 10-60 pulses)
- TASK_REST: 2 * regenTime() pulses
- TASK_SIT: 4 * regenTime() pulses
- TASK_MEDITATE: 4 * Pulse::MOBACT = 48 pulses = 4.8 seconds
- TASK_PENANCE: 5 * Pulse::MOBACT = 60 pulses = 6 seconds
- TASK_YOGINSA: 4 * Pulse::MOBACT = 48 pulses = 4.8 seconds

## Implementation

### Half-Tick Update Scheduling

The function updateHalfTickStuff is called by the main game loop every Pulse::UPDATE ticks. The scheduler maintains a pulse counter that increments each game tick (10 ticks per real second). When pulse modulo Pulse::UPDATE equals zero, the scheduler iterates through all active characters and calls their updateHalfTickStuff method. This function checks linkdead status, fighting state, and room flags before applying moveGain, hitGain, and manaGain. The linkdead check prevents recovery for disconnected players (isPc() && !desc evaluates true). The fighting check blocks HP and mana recovery during combat but allows movement recovery.

### Position Task Lifecycle

When a player issues a position command (sleep, rest, sit), the command handler validates restrictions, calls setPosition to update character state, then calls start_task. The start_task function allocates a task structure, sets the task type (TASK_SLEEP, TASK_REST, TASK_SIT), computes the next update time using calcNextUpdate with the interval argument (regenTime, 2*regenTime, or 4*regenTime), and links the task to the character. The task scheduler checks each character's task field during the main game loop. When pulse reaches task->nextUpdate, the scheduler dispatches to the appropriate task handler function.

Task handlers receive parameters: character pointer, command type, argument string, current pulse, room pointer, and object pointer. For position tasks, command types include CMD_TASK_CONTINUE (normal cycle), CMD_TASK_FIGHTING (combat interruption), and CMD_GENERIC_PULSE (time advancement). On CMD_TASK_CONTINUE, the handler applies recovery (if not blocked by ROOM_NO_HEAL), calls calcNextUpdate to schedule the next cycle, and returns FALSE to keep the task active. On CMD_TASK_FIGHTING, the handler applies cantHit penalty, stops the task via stopTask, and returns FALSE. Task deletion occurs when stopTask sets task to null or when the handler returns TRUE.

### Recovery Rate Stat Dependencies

The hitGain function uses graf to interpolate values across age brackets. The graf function takes age, maps it to a bracket (child, young adult, adult, middle age, old, very old, ancient), and interpolates between provided values for each bracket. For HP, the values are (2, 4, 5, 9, 4, 3, 2), creating a peak at middle age. The plotStat function maps constitution to a recovery multiplier using a cubic bezier curve. The function signature is plotStat(stat_type, stat_value, min_value, max_value, default_value). For HP, it uses (STAT_CURRENT, STAT_CON, 0.80, 1.25, 1.00), meaning 0% constitution gives 0.80x, 100% gives 1.25x, and the default (50%) gives 1.00x.

Level scaling prevents low-level characters from having zero recovery by using max(20, GetMaxLevel()). A level 1 character uses 20 in the numerator, providing 1x scaling. A level 50 character uses 50, providing 2.5x scaling. This creates linear growth after level 20. Class-specific logic includes the mage check in manaGain (gain += gain doubles the value) and the shaman check in hitGain (returns 0 if lifeforce <= 0).

### Camp Affect Implementation

The encamp skill creates an affect structure with duration set to PERMANENT_DURATION (-1), level set to skill_level, type set to SKILL_ENCAMP, and be (being pointer) set to the character's current room pointer. This room linkage prevents the camp from traveling with the character. When the character moves, the camp remains in the original room. The affectTo function adds this affect to the character's affect list. The inCamp function iterates through this list, checking each affect's type field. When it finds SKILL_ENCAMP, it compares the affect's be pointer to the character's current roomp. If they match, the camp is active.

For group members, inCamp scans the character's room (iterating roomp->stuff) for other beings. For each being, it calls inGroup to validate shared master pointer and AFF_GROUP flag. For valid groupmates, it scans their affect lists for SKILL_ENCAMP. If found and the affect's be matches the current room, inCamp returns affect_level / 2. The camp bonus applies in hitGain and moveGain by adding (gain * campLevel / 100) to the computed gain value. A 50% camp skill provides a 50% bonus, adding half the base gain again.

### Bed Object Integration

The TBed class inherits from TObj and adds furniture-specific fields: min_pos_use, max_users, max_size, and regen. When a character rests on a bed, the riding pointer points to the bed object. The character's getPosition returns the position while the riding pointer determines if they're on furniture. During recovery calculation (hitGain, moveGain), the code checks if (riding && riding->objVnum() is bed type), then calls bed->bedRegen(this, &gain, SILENT_NO or SILENT_YES).

The bedRegen method checks character position against min_pos_use. If position is valid (sleeping, resting, or sitting), it adds max(1, getRegen() * gain / 100) to the gain pointer. The getRegen method returns the regen field value. If character height exceeds max_size + 6, it computes a penalty: max(1, (height - max_size) / 3), subtracts this from gain, and floors at 0. This creates a size mismatch system where tall characters suffer penalties on small beds. The max_users field limits concurrent bed occupancy, enforced by counting characters with riding == this_bed in the room's stuff list.

### Shaman Lifeforce Drain Logic

Shaman HP recovery has special handling in both TASK_SLEEP and TASK_REST handlers. The condition checks hasClass(CLASS_SHAMAN) && !affectedBySpell(SPELL_SHAPESHIFT) && !isImmortal(). Shapeshifted shamans recover normally because they're using the form's physiology. Immortals bypass the drain. For valid shamans, if GetMaxLevel() <= 5, they call addToHit(1) normally (low-level shamans recover HP). For shamans above level 5, the code checks if lifeforce > 1. If true, it calls addToLifeforce(-1) and sends a message about activity draining lifeforce. If lifeforce <= 1, it calls updateHalfTickStuff to trigger normal half-tick recovery once (bypassing the position task bonus). This creates a resource management system where shamans must hunt to maintain lifeforce or accept starvation.

### Meditation Task Validation

The canMeditate function checks position constraints and linkdead status. For position, it requires getPosition() >= POSITION_RESTING and getPosition() <= POSITION_STANDING. The exception is POSITION_MOUNTED with getSkillValue(SKILL_ADVANCED_RIDING) >= 50. The mounted meditation check allows rangers and other riders to meditate without dismounting if sufficiently skilled. Flying positions (POSITION_FLYING) fail the constraint because they exceed POSITION_STANDING in the position enum.

Task handlers call canMeditate at the start of each cycle. If it returns false, they call stopTask and return FALSE to indicate task termination. This validation occurs before the ROOM_NO_HEAL check, ensuring position requirements take precedence over environmental blocks. The meditation skills use bSuccess to roll against skill percentage. For SKILL_MEDITATE, success rolls against getSkillValue(SKILL_MEDITATE). For SKILL_YOGINSA, success requires two rolls: bSuccess against SKILL_YOGINSA learn percentage, then a second roll against (70 + wohlin_learn / 4). This creates a difficulty curve where yoginsa becomes more reliable as wohlin skill increases.

### Combat Recovery Blocking

The fight() method returns a pointer to the character's opponent if currently in combat, or null otherwise. Recovery functions check this return value before granting HP or mana. In hitGain, the check occurs at the start: if (fight()) return 0. In manaGain, the check combines with spelltask: if (fight() || spelltask) return 0. Movement recovery does not check combat state, allowing characters to regain movement during fights.

Position task handlers receive CMD_TASK_FIGHTING when the character enters combat. This command type triggers the combat interruption handler, which applies a cantHit penalty via loseRound(1) and a second loseRound(1) with 33% probability (number(0,2) == 0). The loseRound function returns the number of rounds lost, which accumulates into the cantHit counter. The task then calls stopTask to terminate the recovery task. The position remains changed (POSITION_SLEEPING becomes POSITION_STANDING automatically via combat initiation), but the task stops granting periodic bonuses.

### Environmental Modifier Resolution Order

Recovery functions apply modifiers in a specific sequence. For hitGain: base calculation, constitution multiplier, level scaling, bed bonus (additive), drunk bonus (additive), hospital doubling (multiplicative), camp bonus (additive percentage), enliven doubling (multiplicative), aquatic modifier (multiplicative). For manaGain: base calculation, mage doubling (additive), race modifier (additive), hunger/thirst quartering (bitshift), aquatic modifier (multiplicative). For moveGain: base calculation, bed bonus (additive), condition quartering (bitshift), camp bonus (additive percentage), enliven doubling (multiplicative), aquatic modifier (multiplicative), ROOM_NO_HEAL division (multiplicative), ROOM_HOSPITAL doubling (multiplicative), final 4/3 scaling (multiplicative).

The order matters for percentage modifiers. Camp bonus applies to the base gain before hospital doubling, so hospital rooms don't multiply the camp bonus. Enliven applies after camp, so enliven doubles the camp-enhanced value. ROOM_NO_HEAL divides after all additive bonuses, meaning it reduces the total effective gain. The final 4/3 scaling in moveGain applies last, inflating the result after all other modifiers.

## Troubleshooting

### Shamans Not Recovering HP

Symptom: Shaman character with full lifeforce bar shows no HP gain during rest or sleep, but gains HP while standing or walking.

Cause: Shamans above level 5 drain lifeforce during TASK_REST and TASK_SLEEP instead of recovering HP. The task handlers explicitly check GetMaxLevel() > 5 and call addToLifeforce(-1) instead of addToHit(1). When lifeforce reaches zero, the task calls updateHalfTickStuff once per cycle, providing only half-tick recovery (which occurs regardless of position). This means sleeping/resting shamans recover slower than standing shamans because the task bypasses its +1 HP bonus.

Solution: Shamans must maintain lifeforce through killing creatures or using lifeforce restoration abilities. If lifeforce is critical, stand up to receive normal half-tick recovery without the lifeforce drain penalty. Check that the shaman isn't shapeshifted (SPELL_SHAPESHIFT), as shapeshifted shamans bypass the lifeforce system and recover normally.

### No Recovery in Specific Room

Symptom: Position tasks (sleep, rest, sit) run but grant zero HP/mana/move. Half-tick recovery works in other rooms but not this one.

Cause: Room has ROOM_NO_HEAL flag set. This flag blocks the recovery granting section in all task handlers (the check wraps recovery in if (!roomp->isRoomFlag(ROOM_NO_HEAL))). The flag also reduces moveGain by dividing by 3 in the moveGain function. HP and mana recovery during half-tick updates are blocked in updateHalfTickStuff when the room flag is present.

Solution: Verify room flags with the stat command (immortals) or move to a different room. ROOM_NO_HEAL is intentionally placed in dangerous areas, cursed rooms, or combat-focused zones to prevent safe regeneration. If the flag is unintentional, use redit to remove the flag bit from the room definition. Note that movement recovery still occurs at reduced rate (1/3 of normal) even in NO_HEAL rooms.

### Meditation Stops Immediately After Starting

Symptom: doMeditate command succeeds and starts TASK_MEDITATE, but the task terminates after one cycle with "You are unable to meditate" message.

Cause: canMeditate validation failed during the task cycle. Common failures include position change (flying, mounted without Advanced Riding 50+, incapacitated), linkdead status, or position falling below POSITION_RESTING. The task handler calls canMeditate at the start of each cycle and stops the task if it returns false.

Solution: Check character position with score command. Verify not flying or mounted (unless Advanced Riding >= 50%). Ensure position is resting, sitting, or standing. If mounted, check getSkillValue(SKILL_ADVANCED_RIDING) meets the 50% threshold. Linkdead characters cannot meditate, so reconnection resolves this. If repeatedly knocked prone or incapacitated during meditation, the position drops below POSITION_RESTING, triggering automatic termination.

### Camp Bonus Not Applying

Symptom: Encamp skill succeeds with confirmation message, but hitGain and moveGain show no bonus in recovery rate checks.

Cause: Camp affect is tied to the room where encamp was cast. Moving to a different room breaks the camp link. The inCamp function compares affect->be (the room pointer stored at cast time) to character->roomp (current room). If they differ, inCamp returns FALSE. Another cause is lack of AFF_GROUP flag for groupmates attempting to benefit from another character's camp.

Solution: Verify character is still in the same room where encamp was cast. Check affects list (affects command) for SKILL_ENCAMP entry. For groupmates, ensure both characters have AFF_GROUP flag (follow command establishes this). Groupmates receive half the camper's skill level as bonus (max(1, camp_level / 2)). Recasting encamp in the new room creates a fresh camp. Note that multiple camps in the same room don't stack; inCamp returns the highest single camp level.

### Meditation Granting Less Mana Than Expected

Symptom: SKILL_MEDITATE task runs successfully (bSuccess returns true) but mana gain is only 1 point per cycle instead of the expected manaGain bonus.

Cause: The meditation formula grants manaGain() - 1, with a floor of 1. If manaGain returns 2 or less (due to low intelligence, hunger, thirst, or negative race modifiers), the effective bonus is 1 even on successful meditation. On failure, meditation grants exactly 1 mana, matching the success result when base mana gain is low.

Solution: Increase manaGain by raising intelligence (mana recovery scales with intelligence via the graf function), removing hunger/thirst debuffs (empty FULL or THIRST quarters mana gain), or equipping mana regeneration items. Mages receive double mana gain, making meditation far more effective for that class. Check getCond(FULL) and getCond(THIRST) to ensure neither is zero. Aquatic races without AFFECT_WET suffer 0.5x mana gain; getting wet restores normal rates.

### Bed Regeneration Not Working

Symptom: Character rests on bed (riding pointer set to bed object) but receives no regeneration bonus. Recovery rate matches resting on floor.

Cause: Character position doesn't meet bed's min_pos_use requirement. Beds specify the minimum position required (0=sleep, 1=rest, 2=sit). If min_pos_use is 0 (sleep only) and character is resting (position 1), bedRegen returns without applying the bonus. Another cause is size mismatch penalty exceeding the regen bonus. If (character_height - 6) > bed_max_size, the penalty subtracts from gain, potentially nullifying the bonus entirely.

Solution: Check bed properties with stat command (immortals) to see min_pos_use and max_size values. Change position to match requirement (sleep on beds that require sleeping). For size issues, find a larger bed or use a smaller character race. The size penalty is max(1, (height - max_size) / 3), so a 72-inch tall character on a 60-inch bed suffers an 8-point penalty (72 - 60 = 12, 12 / 3 = 4, but penalty is at least 1). Bed regen bonus must exceed this penalty to provide net benefit.

### Rest Not Granting Piety

Symptom: Character rests for extended period but piety value remains static. Piety gain expected from TASK_REST task.

Cause: ROOM_NO_HEAL flag blocks piety recovery just like HP and mana. The piety granting code is wrapped in the same if (!roomp->isRoomFlag(ROOM_NO_HEAL)) check as other recovery. Another cause is character class lacking piety (only clerics, deikhan, rangers, shamans, and paladins use piety).

Solution: Verify room doesn't have ROOM_NO_HEAL flag. Check character class with score command; warriors, thieves, mages, and monks don't use piety and won't see any value changes. If piety is relevant but not changing, ensure TASK_REST is actually running (stopTask called manually or position changed terminates the task). Rest grants +0.10 piety per cycle, so changes are gradual. Sleep does not grant piety (TASK_SLEEP handler omits the addToPiety call entirely). Use rest specifically if piety recovery is needed.

### updateHalfTickStuff Not Running

Symptom: Character receives no passive regeneration. Position tasks work but no recovery occurs during movement or standing.

Cause: Character is linkdead (desc is null) and is a PC. The linkdead check in updateHalfTickStuff blocks HP and mana recovery for disconnected players. Movement recovery continues (unless linkdead and not a PC), but HP/mana recovery stops entirely. Another cause is fighting state (fight() returns non-null pointer), which blocks HP and mana recovery in updateHalfTickStuff.

Solution: Reconnect the descriptor to restore desc pointer. For debugging, check isPc() && desc boolean in the character's state. Linkdead players show disconnected in the who list. For the fighting block, end combat by fleeing, killing the opponent, or becoming incapacitated. Note that movement recovery continues during combat, so if movement regenerates but HP/mana don't, combat is the likely cause. Immortals can force characters out of combat with force <char> wimpy 999 and force <char> flee.

### Camp Skill Fails in Valid Terrain

Symptom: Encamp command returns failure message despite being in wilderness forest room with no apparent restrictions.

Cause: Room has hazardous flags (ROOM_ON_FIRE, ROOM_FLOODED, ROOM_NO_FLEE, ROOM_NO_ESCAPE, ROOM_NO_HEAL, ROOM_HAVE_TO_WALK) that block camping. The encamp function validates terrain type first, then checks these flags. Another cause is being in a water sector (flying, underwater, ocean, river) or the room being marked as indoor despite being a nature sector. City sectors always block camping regardless of other properties.

Solution: Check room flags with stat room command. Remove hazardous flags if they're unintentional. Verify the room's sector type isn't city, water, or indoor. For underwater or ocean sectors, surface to a shore or beach sector. If the room is a cave with ROOM_INDOORS flag, remove that flag (caves are allowed but indoor rooms are not). The terrain validation requires one of: forest, beach, hill, mountain, nature, road, swamp, arctic, or cave sector types. Any other sector type (including plains, desert, or underwater) fails validation even without hazardous flags.

### Yoginsa Not Providing Wohlin Healing

Symptom: TASK_YOGINSA runs successfully and grants HP/mana/move, but expected healing effects (salve, cure poison, etc.) don't trigger.

Cause: Wohlin skill level (getSkillValue(SKILL_WOHLIN)) is below the required threshold for the specific effect. Each healing effect requires a minimum wohlin percentage: salve (20%), cure poison (35%), sterilize (50%), cure disease (60%), clot (75%), reduce hunger (90%). The healing effects only attempt if the character passes the yoginsa success roll AND the secondary (70 + wohlin_learn / 4) percentage roll.

Solution: Increase SKILL_WOHLIN through practice or use. The skill must be learned (non-zero value) for the checks to evaluate true. At 0% wohlin, no healing effects occur. At 90% wohlin, all effects trigger on successful yoginsa meditation cycles. The healing effects call the respective functions (salve, curePoison, sterilize, cureDisease, clot) which apply their own success rolls. Even with high wohlin, each healing effect may fail its internal skill check. Review the combat log for messages about healing attempts. If no messages appear, wohlin skill is too low. If messages appear but fail, the healing skill itself needs improvement.
