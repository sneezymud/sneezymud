---
title: Rest and Recovery System
description: Manages how characters regain HP, mana, movement, piety, and lifeforce through position-based recovery, half-tick recovery, skill-based recovery, and environment bonuses.
keywords:
  - TASK_SLEEP
  - TASK_REST
  - TASK_SIT
  - updateHalfTickStuff
  - hitGain
  - manaGain
  - moveGain
  - regenTime
  - SKILL_MEDITATE
  - SKILL_PENANCE
  - SKILL_YOGINSA
  - SKILL_ENCAMP
  - inCamp
  - bedRegen
  - ROOM_NO_HEAL
  - ROOM_HOSPITAL
category: Important Systems
related:
  - position-stance.md
  - scheduler-pulses.md
  - task-system.md
  - class-system.md
  - stats-attributes.md
  - affects-system.md
last_updated: 2026-01-29
source_files:
  - code/code/misc/limits.cc
  - code/code/misc/periodic.cc
  - code/code/misc/movement.cc
  - code/code/task/task_sleep.cc
  - code/code/task/task_rest.cc
  - code/code/task/task_sit.cc
  - code/code/task/task_meditate.cc
  - code/code/task/task_penance.cc
  - code/code/disc/disc_monk_meditation.cc
  - code/code/disc/disc_advanced_adventuring.cc
  - code/code/obj/obj_bed.cc
  - code/code/misc/being.cc
  - code/code/sys/comm.h
---

# Rest and Recovery System

The rest and recovery system manages how characters regain HP, mana, movement, piety, and lifeforce. Understanding this system is essential for balancing gameplay mechanics, implementing new classes, and debugging regeneration issues.

## Overview

SneezyMUD uses a multi-layered regeneration system:

1. **Position-based recovery**: Sleep/rest/sit positions trigger periodic regeneration tasks
2. **Half-tick recovery**: Global recovery applied every 36 seconds to all characters
3. **Skill-based recovery**: Class-specific meditation and rest abilities
4. **Environment bonuses**: Beds, hospitals, camps provide additional regeneration

Recovery rates depend on position, class, level, stats, equipment, and environmental factors.

## Position-Based Recovery

### Position Hierarchy for Recovery

| Position | HP/Mana/Move Regen | Task Update Interval | Notes |
|----------|-------------------|---------------------|-------|
| `POSITION_SLEEPING` | Fastest | `regenTime()` | Best recovery, vulnerable |
| `POSITION_RESTING` | Fast | `2 * regenTime()` | Good recovery, limited actions |
| `POSITION_SITTING` | Moderate | `4 * regenTime()` | Minimal bonus, can use some commands |
| `POSITION_STANDING` | Base rate | Half-tick only | No task-based bonus |
| `POSITION_FIGHTING` | None | N/A | No recovery during combat |

**Source:** `code/code/misc/movement.cc` (lines 2707-3078)

### Sleep Command

```cpp
void TBeing::doSleep(const sstring& argument) {
    // Position validation
    if (isFlying()) return;
    if (roomp->isWaterSector() && !isAquatic()) return;
    if (fight()) return;
    if (isCombatMode(ATTACK_BERSERK)) return;

    setPosition(POSITION_SLEEPING);
    if (isPc())
        start_task(this, 0, 0, TASK_SLEEP, "", 350, 0, 1, 0, regenTime());
}
```

**Restrictions:**
- Cannot sleep while flying
- Cannot sleep in water (unless aquatic race)
- Cannot sleep while fighting or berserking
- Loses sneak when entering sleep

**Source:** `code/code/misc/movement.cc` (lines 2914-2997)

### Rest Command

```cpp
void TBeing::doRest(const sstring& argument) {
    // Similar validation to sleep
    setPosition(POSITION_RESTING);
    if (isPc())
        start_task(this, 0, 0, TASK_REST, "", 350, 0, 1, 0, 2 * regenTime());
}
```

**Restrictions:**
- Cannot rest while flying
- Cannot rest in water without a boat
- Cannot rest while fighting or berserking

**Source:** `code/code/misc/movement.cc` (lines 2819-2908)

### Sit Command

```cpp
void TBeing::doSit(const sstring& argument) {
    setPosition(POSITION_SITTING);
    if (isPc())
        start_task(this, 0, 0, TASK_SIT, "", 350, 0, 1, 0, 4 * regenTime());
}
```

**Source:** `code/code/misc/movement.cc` (lines 2707-2815)

## Task-Based Recovery

### TASK_SLEEP

The sleep task provides the fastest regeneration rate.

```cpp
int task_sleep(TBeing* ch, cmdTypeT cmd, const char* arg, int pulse,
               TRoom*, TObj*) {
    // Task continues while sleeping
    ch->task->calcNextUpdate(pulse, regentime);

    if (!ch->roomp->isRoomFlag(ROOM_NO_HEAL)) {
        ch->addToMana(1);
        ch->addToHit(1);  // Shamans have special handling
        if (ch->getMove() < ch->moveLimit())
            ch->addToMove(1);
    }
}
```

**Recovery per task cycle:**
- +1 Mana
- +1 HP (except low-level shamans)
- +1 Movement (if below max)

**Task interval:** `regenTime()` pulses (fastest)

**Source:** `code/code/task/task_sleep.cc`

### TASK_REST

The rest task provides good regeneration with slightly longer intervals.

```cpp
int task_rest(TBeing* ch, cmdTypeT cmd, const char* arg, int pulse,
              TRoom*, TObj*) {
    ch->task->calcNextUpdate(pulse, 4 * ch->regenTime());

    if (!ch->roomp->isRoomFlag(ROOM_NO_HEAL)) {
        ch->addToMana(1);
        ch->addToPiety(.10);  // Rest grants piety
        ch->addToHit(1);
        if (ch->getMove() < ch->moveLimit())
            ch->addToMove(1);
    }
}
```

**Recovery per task cycle:**
- +1 Mana
- +0.10 Piety (unique to rest)
- +1 HP
- +1 Movement (if below max)

**Task interval:** `4 * regenTime()` pulses

**Special:** Rest is the only position that grants piety regeneration through the task system.

**Source:** `code/code/task/task_rest.cc`

### Shaman Lifeforce Drain

Shamans above level 5 drain lifeforce instead of gaining HP while resting/sleeping:

```cpp
if (ch->hasClass(CLASS_SHAMAN) &&
    !ch->affectedBySpell(SPELL_SHAPESHIFT) && !ch->isImmortal()) {
    if (ch->GetMaxLevel() > 5) {
        if (1 > ch->getLifeforce()) {
            ch->updateHalfTickStuff();
        } else {
            ch->addToLifeforce(-1);
            ch->sendTo("Your lack of activity drains your precious lifeforce.\n\r");
        }
    } else {
        ch->addToHit(1);
    }
}
```

**Source:** `code/code/task/task_rest.cc` (lines 30-45)

### Combat Vulnerability

Being attacked while resting/sleeping has consequences:

```cpp
case CMD_TASK_FIGHTING:
    ch->sendTo("You are unable to rest while under attack!\n\r");
    ch->cantHit += ch->loseRound(1);
    if (!::number(0, 2))
        ch->cantHit += ch->loseRound(1);  // Additional penalty
    ch->stopTask();
```

Characters lose 1-2 combat rounds when attacked during rest.

## Half-Tick Recovery System

### updateHalfTickStuff()

Called every tick (36 real seconds) for all characters:

```cpp
int TBeing::updateHalfTickStuff() {
    // Movement always regenerates (unless linkdead non-PC)
    if (!isPc() || desc) {
        int mg = moveGain();
        mg = min(mg, moveLimit() - getMove());
        addToMove(mg);
    }

    // HP and Mana regenerate when not fighting and not in NO_HEAL room
    if (!fight() && (!isPc() || desc)) {
        if (!roomp->isRoomFlag(ROOM_NO_HEAL)) {
            int mana_bump = manaGain();
            addToMana(mana_bump);
            addToHit(hitGain());
        }
    }
}
```

**Source:** `code/code/misc/periodic.cc` (lines 1241-1820)

### Pulse Timing

```cpp
namespace Pulse {
    const int ONE_SECOND = 10;          // 10 ticks = 1 second
    const int UPDATE = ONE_SECOND * 36; // 360 ticks = 36 seconds
    const int MUDHOUR = UPDATE * 4;     // 4 updates = 1 mud hour
    const int MOBACT = (int)((float)ONE_SECOND * 1.2); // 12 ticks = 1.2 seconds
}
```

**Source:** `code/code/sys/comm.h` (lines 46-77)

## Recovery Rate Calculations

### hitGain() - HP Recovery

For players:

```cpp
int TPerson::hitGain() {
    if (hasClass(CLASS_SHAMAN) && 0 >= getLifeforce())
        return 0;  // Shamans need lifeforce

    if (fight())
        return 0;  // No HP regen in combat

    // Base gain from age graph
    gain = graf(this, age, 2, 4, 5, 9, 4, 3, 2);
    gain += 4;

    // Constitution modifier (0.80 to 1.25)
    gain *= plotStat(STAT_CURRENT, STAT_CON, .80, 1.25, 1.00);

    // Level scaling
    gain *= max(20, GetMaxLevel());
    gain /= 20;

    // Modifiers
    if (riding bed) bed->bedRegen(this, &gain, SILENT_NO);
    if (getCond(DRUNK) > 0) gain += 1 + (getCond(DRUNK) / 3);
    if (roomp->isRoomFlag(ROOM_HOSPITAL)) gain *= 2;
    if (inCamp()) gain += (gain * campLevel / 100);
    if (affectedBySpell(SPELL_ENLIVEN)) gain *= 2;
    if (isAquatic()) gain *= (affectedBySpell(AFFECT_WET) ? 1.3 : 0.5);

    return gain;
}
```

**Key modifiers:**
- Constitution: 0.80x to 1.25x
- Level scaling: up to 2.5x at level 50
- Drunk bonus: +1 to +9 based on intoxication
- Hospital room: 2x
- Camp: +skill_level%
- Enliven spell: 2x
- Aquatic races: 1.3x if wet, 0.5x if dry

**Source:** `code/code/misc/limits.cc` (lines 408-481)

### manaGain() - Mana Recovery

```cpp
int TPerson::manaGain() {
    if (fight() || spelltask)
        return 0;  // No mana regen while fighting or casting

    // Base gain from age graph
    gain = graf(this, age, 2, 4, 6, 8, 10, 12, 14);
    gain *= 4;  // Arbitrary multiplier

    // Mage class bonus
    if (hasClass(CLASS_MAGE))
        gain += gain;  // Double for mages

    // Race modifier
    gain += race->getManaMod();

    // Hunger/thirst penalty
    if (!getCond(FULL) || !getCond(THIRST))
        gain >>= 2;  // Quarter if hungry/thirsty

    // Aquatic modifier
    if (isAquatic())
        gain *= (affectedBySpell(AFFECT_WET) ? 1 : 0.5);

    return gain;
}
```

**Key modifiers:**
- Mages: 2x base gain
- Hunger/thirst: 0.25x if starving/dehydrated
- Aquatic races: 0.5x if dry

**Source:** `code/code/misc/limits.cc` (lines 310-344)

### moveGain() - Movement Recovery

```cpp
int TBeing::moveGain() {
    gain = plotStat(STAT_CURRENT, STAT_CON, 11, 41, 30);

    if (riding bed) bed->bedRegen(this, &gain, SILENT_YES);

    if (isAffected(AFF_POISON)) gain >>= 2;
    if (isAffected(AFF_SYPHILIS)) gain >>= 2;
    if (!getCond(FULL) || !getCond(THIRST)) gain >>= 2;

    if (inCamp()) gain += (gain * campLevel / 100);
    if (affectedBySpell(SPELL_ENLIVEN)) gain *= 2;
    if (isAquatic()) gain *= (affectedBySpell(AFFECT_WET) ? 1.3 : 0.5);

    if (roomp->isRoomFlag(ROOM_NO_HEAL)) gain /= 3;
    if (roomp->isRoomFlag(ROOM_HOSPITAL)) gain *= 2;

    return (gain * 4) / 3;
}
```

**Source:** `code/code/misc/limits.cc` (lines 483-540)

### regenTime() - Task Timing Calculation

```cpp
int TBeing::regenTime() {
    int iTime = 100;

    // Use the slowest regeneration rate
    if (getHit() < hitLimit())
        iTime = min(iTime, hitGain());
    if (getMove() < moveLimit())
        iTime = min(iTime, moveGain());
    if (getMana() < manaLimit())
        iTime = min(iTime, manaGain());

    iTime = max(iTime, 1);

    // Convert to pulses
    iTime = Pulse::UPDATE / iTime;
    return iTime;
}
```

The task interval is calculated as the inverse of the slowest regeneration rate, scaled by the update pulse length.

**Source:** `code/code/misc/limits.cc` (lines 1577-1598)

## Camp System

### doEncamp() - Setting Up Camp

Rangers can set up camp in wilderness areas for group recovery bonuses:

```cpp
int encamp(TBeing* caster) {
    // Terrain validation
    if (caster->roomp->isRoomFlag(ROOM_ON_FIRE)) return SPELL_FAIL;
    if (caster->roomp->isRoomFlag(ROOM_FLOODED)) return SPELL_FAIL;
    if (caster->roomp->isRoomFlag(ROOM_INDOORS)) return SPELL_FAIL;
    if (caster->roomp->isCitySector()) return SPELL_FAIL;

    // Must be in nature sector
    if (!(roomp->isForestSector() || roomp->isBeachSector() ||
          roomp->isHillSector() || roomp->isMountainSector() ||
          roomp->isNatureSector() || roomp->isRoadSector() ||
          roomp->isSwampSector() || roomp->isArcticSector() ||
          roomp->isCaveSector())) {
        caster->sendTo("You need to be in nature to camp.\n\r");
        return SPELL_FAIL;
    }

    // Create camp affect
    aff.duration = PERMANENT_DURATION;
    aff.level = skill_level;
    aff.type = SKILL_ENCAMP;
    aff.be = caster->roomp;  // Tied to room

    if (caster->bSuccess(bKnown, SKILL_ENCAMP)) {
        caster->affectTo(&aff);
    } else {
        aff.level /= 2;  // Half effectiveness on failure
        caster->affectTo(&aff);
    }
}
```

**Source:** `code/code/disc/disc_advanced_adventuring.cc` (lines 995-1091)

### inCamp() - Camp Detection

```cpp
int TBeing::inCamp() const {
    // Check if character has camp affect
    for (aff = affected; aff; aff = aff->next) {
        if (aff->type == SKILL_ENCAMP)
            return aff->level;  // Return camp skill level
    }

    // Check if grouped with someone who has camp
    if (!isAffected(AFF_GROUP))
        return FALSE;

    for (ch in roomp->stuff) {
        if (inGroup(*ch)) {
            for (aff = ch->affected; aff; aff = aff->next) {
                if (aff->type == SKILL_ENCAMP)
                    return max(1, (aff->level / 2));  // Half bonus for groupmates
            }
        }
    }
    return FALSE;
}
```

**Camp bonuses:**
- Camper: +skill_level% to HP/move recovery
- Groupmates in same room: +skill_level/2% to HP/move recovery

**Source:** `code/code/disc/disc_advanced_adventuring.cc` (lines 962-993)

### Camp Restrictions

| Restriction | Description |
|------------|-------------|
| Sector type | Nature, forest, beach, hill, mountain, road, swamp, arctic, cave only |
| Indoors | Cannot camp in buildings (caves allowed) |
| City | Cannot camp in city sectors |
| Room flags | No `ROOM_ON_FIRE`, `ROOM_FLOODED`, `ROOM_NO_FLEE`, `ROOM_NO_ESCAPE`, `ROOM_NO_HEAL`, `ROOM_HAVE_TO_WALK` |
| Flying/underwater/ocean/river | Cannot camp |

## Meditation System

### Meditate Skill (SKILL_MEDITATE)

Mages can meditate to restore mana faster:

```cpp
int task_meditate(TBeing* ch, cmdTypeT cmd, const char*, int pulse,
                  TRoom*, TObj*) {
    if (!ch->canMeditate()) {
        ch->stopTask();
        return FALSE;
    }

    ch->task->calcNextUpdate(pulse, 4 * Pulse::MOBACT);

    if (!ch->roomp->isRoomFlag(ROOM_NO_HEAL)) {
        learn = ch->getSkillValue(SKILL_MEDITATE);
        if (ch->bSuccess(learn, SKILL_MEDITATE)) {
            gainAmt = ch->manaGain() - 1;
            gainAmt = max(gainAmt, 1);
            ch->setMana(min(ch->getMana() + gainAmt, (int)ch->manaLimit()));
        } else {
            ch->addToMana(1);  // Base regen on failure
        }

        // Bonus HP and move regardless of skill success
        ch->addToHit(1);
        if (ch->getMove() < ch->moveLimit())
            ch->addToMove(1);
    }
}
```

**Benefits:**
- Success: Full manaGain() bonus
- Failure: +1 mana (resting rate)
- Always: +1 HP, +1 movement

**Task interval:** `4 * Pulse::MOBACT` (~4.8 seconds)

**Source:** `code/code/task/task_meditate.cc`

### Penance (SKILL_PENANCE)

Clerics and Deikhan can use penance to restore piety:

```cpp
int task_penance(TBeing* ch, cmdTypeT cmd, const char*, int pulse,
                 TRoom*, TObj*) {
    ch->task->calcNextUpdate(pulse, 5 * Pulse::MOBACT);
    ch->task->timeLeft++;
    val = (double)ch->task->timeLeft * 0.3;  // Bonus increases over time

    if (ch->bSuccess(learn, ch->getPerc(), SKILL_PENANCE)) {
        amt = ch->pietyGain(val);
        ch->addToPiety(amt);
    } else {
        amt = ::number(6, 8) / 10.0;  // 0.6-0.8 on failure
        ch->addToPiety(amt);
    }
}
```

**Benefits:**
- Success: Full pietyGain() with increasing bonus over time
- Failure: +0.6-0.8 piety (better than resting's +0.10)

**Task interval:** `5 * Pulse::MOBACT` (~6 seconds)

**Source:** `code/code/task/task_penance.cc`

### Yoginsa (SKILL_YOGINSA)

Monks can use yoginsa meditation for enhanced recovery:

```cpp
int task_yoginsa(TBeing* ch, cmdTypeT cmd, const char*, int pulse,
                 TRoom*, TObj*) {
    ch->task->calcNextUpdate(pulse, 4 * Pulse::MOBACT);

    if (ch->bSuccess(learn, SKILL_YOGINSA) &&
        (::number(1, 100) < (70 + (wohlin_learn / 4)))) {

        // Enhanced recovery
        ch->setHit(min(ch->getHit() + max(2, (int)(hitGain() * 0.80)),
                       (int)ch->hitLimit()));
        ch->setMove(min(ch->getMove() + moveGain() / 2, (int)ch->moveLimit()));
        ch->setMana(min(ch->getMana() + manaGain() / 2, (int)ch->manaLimit()));

        // Wohlin skill bonuses (if learned)
        if (wohlin_learn > 20) salve();        // 20%+
        if (wohlin_learn > 35) curePoison();   // 35%+
        if (wohlin_learn > 50) sterilize();    // 50%+
        if (wohlin_learn > 60) cureDisease();  // 60%+
        if (wohlin_learn > 75) clot();         // 75%+
        if (wohlin_learn > 90) reduceHunger(); // 90%+
    }
}
```

**Benefits:**
- HP: 80% of hitGain()
- Move: 50% of moveGain()
- Mana: 50% of manaGain()
- Wohlin skill provides automatic healing effects

**Task interval:** `4 * Pulse::MOBACT` (~4.8 seconds)

**Source:** `code/code/disc/disc_monk_meditation.cc`

### canMeditate() - Meditation Requirements

```cpp
bool TBeing::canMeditate() {
    if (isLinkdead() || (getPosition() < POSITION_RESTING))
        return FALSE;

    if (getPosition() > POSITION_STANDING &&
        !(getPosition() == POSITION_MOUNTED &&
          (getSkillValue(SKILL_ADVANCED_RIDING) >= 50))) {
        return FALSE;
    }
    return TRUE;
}
```

**Requirements:**
- Must be resting, sitting, or standing
- Can meditate while mounted if Advanced Riding >= 50%
- Cannot meditate while linkdead
- Cannot meditate while flying

**Source:** `code/code/misc/being.cc` (lines 1612-1622)

## Bed System

### TBed - Furniture for Recovery

Beds provide bonus regeneration based on their regen value:

```cpp
void TBed::bedRegen(TBeing* ch, int* gain, silentTypeT silent) const {
    if ((ch->getPosition() == POSITION_SLEEPING) ||
        (ch->getPosition() == POSITION_SITTING) ||
        (ch->getPosition() == POSITION_RESTING)) {
        *gain += max(1, getRegen() * *gain / 100);
    }

    // Size penalty if bed too small
    if ((ch->getHeight() - 6) > getMaxSize()) {
        *gain -= max(1, (ch->getHeight() - getMaxSize()) / 3);
        *gain = max(*gain, 0);
    }
}
```

**Bed properties:**
- `min_pos_use`: Minimum position (0=sleep, 1=rest, 2=sit)
- `max_users`: How many can use the bed simultaneously
- `max_size`: Maximum comfortable height
- `regen`: Bonus regeneration percentage

**Source:** `code/code/obj/obj_bed.cc` (lines 184-234)

## Environmental Modifiers

### Room Flags

| Flag | Effect |
|------|--------|
| `ROOM_NO_HEAL` | Blocks all task-based recovery, reduces moveGain() by 2/3 |
| `ROOM_HOSPITAL` | Doubles HP and movement regeneration |

### Condition Effects

| Condition | Effect |
|-----------|--------|
| Hungry/Thirsty | Quarter mana and move gain |
| Drunk | Bonus HP gain (+1 to +9) |
| Poisoned | Quarter move gain |
| Syphilis | Quarter move gain |

### Spell Effects

| Spell | Effect |
|-------|--------|
| `SPELL_ENLIVEN` | Doubles HP and move gain |
| `SPELL_MIND_FOCUS` | Bonus mana gain based on skill |
| `SKILL_BLOODLUST` | Scales HP and move gain based on stacks |

## Recovery Rate Summary Table

| Stat | Base Rate | Position Bonus | Camp Bonus | Bed Bonus | Hospital | Enliven |
|------|-----------|----------------|------------|-----------|----------|---------|
| HP | hitGain() | Task: +1/cycle | +skill% | +regen% | 2x | 2x |
| Mana | manaGain() | Task: +1/cycle | None | None | None | None |
| Move | moveGain() | Task: +1/cycle | +skill% | +regen% | 2x | 2x |
| Piety | pietyGain() | Rest: +0.1/cycle | None | None | None | None |
| Lifeforce | N/A | Drain: -1/cycle (Shaman) | None | None | None | None |

## Common Gotchas

### 1. Shamans and HP Recovery

Shamans above level 5 do NOT gain HP through rest/sleep tasks. Instead, they drain lifeforce. HP recovery only occurs through the half-tick system when not in a rest position.

### 2. ROOM_NO_HEAL Blocks Tasks

The `ROOM_NO_HEAL` flag stops all task-based recovery but does NOT completely stop half-tick recovery (only reduces move gain).

### 3. Camp is Position-Independent

Camp bonuses apply to hitGain() and moveGain() directly, not through task cycles. Characters get camp bonuses even while standing.

### 4. Meditation Requires Specific Position

`canMeditate()` requires `POSITION_RESTING <= position <= POSITION_STANDING` (or mounted with high riding skill).

### 5. regenTime() is Inverse

Higher regeneration rates result in SHORTER task intervals, not longer ones. The calculation is `Pulse::UPDATE / gain_rate`.

### 6. Rest Grants Piety, Sleep Does Not

Only the REST task grants piety (+0.10 per cycle). Sleep does not grant piety through the task system.

## Key Source Files

| File | Contents |
|------|----------|
| `code/code/misc/limits.cc` | hitGain(), manaGain(), moveGain(), regenTime() |
| `code/code/misc/periodic.cc` | updateHalfTickStuff() |
| `code/code/misc/movement.cc` | doSleep(), doRest(), doSit(), doWake() |
| `code/code/task/task_sleep.cc` | TASK_SLEEP handler |
| `code/code/task/task_rest.cc` | TASK_REST handler |
| `code/code/task/task_sit.cc` | TASK_SIT handler |
| `code/code/task/task_meditate.cc` | TASK_MEDITATE handler |
| `code/code/task/task_penance.cc` | TASK_PENANCE handler |
| `code/code/disc/disc_monk_meditation.cc` | TASK_YOGINSA handler |
| `code/code/disc/disc_advanced_adventuring.cc` | encamp(), inCamp() |
| `code/code/obj/obj_bed.cc` | TBed class and bedRegen() |
| `code/code/misc/being.cc` | canMeditate() |
| `code/code/sys/comm.h` | Pulse timing constants |

## Related Documentation

- [Position Stance](position-stance.md) - Position system and combat effects
- [Scheduler Pulses](scheduler-pulses.md) - Timing system and pulse handling
- [Task System](task-system.md) - Task implementation and handlers
- [Class System](class-system.md) - Class-specific recovery abilities
- [Stats Attributes](stats-attributes.md) - Constitution's effect on regeneration
- [Affects System](affects-system.md) - Spell effects on recovery
