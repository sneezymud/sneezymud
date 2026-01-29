---
title: Trap Mechanics System
description: The trap system provides dangerous obstacles throughout the game world via doors, containers, objects, and rooms with critical DELETE flag patterns and iterator safety requirements for crash prevention.
keywords:
  - doorTrapT
  - TTrap
  - springTrap
  - triggerTrap
  - disarmMe
  - detectMe
  - reconcileDamage
  - DELETE_THIS
  - DELETE_VICT
  - DELETE_ITEM
  - post-increment iterator
  - TRAP_EFF_ROOM
  - SKILL_DISARM_TRAP
  - SKILL_DETECT_TRAP
category: Important Systems

  - damage-pipeline.md
  - command-implementation.md
  - spatial-relationships.md
last_updated: 2026-01-29
source_files:
  - code/code/misc/trap.h
  - code/code/misc/trap.cc
  - code/code/disc/disc_thief_looting.cc
  - code/code/obj/obj_trap.h
  - code/code/obj/obj_trap.cc
related: [memory-safety.md]
---

The trap system provides dangerous obstacles throughout the game world via doors, containers, objects, and rooms. This document describes trap types, triggering mechanisms, DELETE flag patterns, and critical crash prevention strategies.

**Misusing this system causes crashes.** Common errors: continuing execution after DELETE flags, not using post-increment iterators, ignoring reconcileDamage -1 returns, failing to validate pointers after trap triggers.

## Overview

Traps can be placed on:
- **Doors/Exits** - Trigger on movement through exit
- **Containers** - Trigger on opening or taking items
- **Rooms** - Affect all beings in the room
- **Mines** - Placed by characters, trigger on movement
- **Grenades** - Thrown projectiles that detonate
- **Arrows** - Traps embedded in stuck arrows

## Trap Types (16 Total)

From `code/code/misc/trap.h` enum `doorTrapT`:

| Value | Constant | Damage Type | Special Effect |
|-------|----------|-------------|----------------|
| 0 | `DOOR_TRAP_NONE` | None | No trap |
| 1 | `DOOR_TRAP_POISON` | Poison | +AFF_POISON |
| 2 | `DOOR_TRAP_SPIKE` | Pierce | Physical damage |
| 3 | `DOOR_TRAP_SLEEP` | None | +AFF_SLEEP (forced rest) |
| 4 | `DOOR_TRAP_TNT` | Blast | Room-wide explosion |
| 5 | `DOOR_TRAP_BLADE` | Slash | +bleeding |
| 6 | `DOOR_TRAP_FIRE` | Fire | Cumulative burn damage |
| 7 | `DOOR_TRAP_ACID` | Acid | Structure degradation |
| 8 | `DOOR_TRAP_DISEASE` | Disease | +AFF_DISEASE (24 hours) |
| 9 | `DOOR_TRAP_HAMMER` | Blunt | +stun effect |
| 10 | `DOOR_TRAP_FROST` | Cold | +AFF_COLD |
| 11 | `DOOR_TRAP_TELEPORT` | None | Random displacement |
| 12 | `DOOR_TRAP_ENERGY` | Energy | Magic damage |
| 13 | `DOOR_TRAP_BOLT` | Lightning | Spell-like effect |
| 14 | `DOOR_TRAP_DISK` | Slash | Spinning disk projectile |
| 15 | `DOOR_TRAP_PEBBLE` | Blunt | Area-effect projectile spray |

**Source:** `code/code/misc/trap.h` (enum doorTrapT)

## Trap Target Types

From `code/code/misc/trap.h` enum `trap_targ_t`:

| Value | Constant | Description |
|-------|----------|-------------|
| 0 | `TRAP_TARG_DOOR` | Traps on doors/exits |
| 1 | `TRAP_TARG_CONT` | Traps on containers |
| 2 | `TRAP_TARG_MINE` | Character-placed mines |
| 3 | `TRAP_TARG_GRENADE` | Thrown grenades |
| 4 | `TRAP_TARG_ARROW` | Arrows stuck in victims |

## Trap Effect Flags (17 Total)

Bitflags controlling when and how traps trigger:

| Flag | Bit | Hex | Effect |
|------|-----|-----|--------|
| `TRAP_EFF_MOVE` | 0 | 0x1 | Trigger on movement |
| `TRAP_EFF_OBJECT` | 1 | 0x2 | Trigger on get/put object |
| `TRAP_EFF_ROOM` | 2 | 0x4 | Affect all in room |
| `TRAP_EFF_NORTH` | 3 | 0x8 | Direction: North |
| `TRAP_EFF_EAST` | 4 | 0x10 | Direction: East |
| `TRAP_EFF_SOUTH` | 5 | 0x20 | Direction: South |
| `TRAP_EFF_WEST` | 6 | 0x40 | Direction: West |
| `TRAP_EFF_UP` | 7 | 0x80 | Direction: Up |
| `TRAP_EFF_DOWN` | 8 | 0x100 | Direction: Down |
| `TRAP_EFF_NE` | 9 | 0x200 | Direction: Northeast |
| `TRAP_EFF_NW` | 10 | 0x400 | Direction: Northwest |
| `TRAP_EFF_SE` | 11 | 0x800 | Direction: Southeast |
| `TRAP_EFF_SW` | 12 | 0x1000 | Direction: Southwest |
| `TRAP_EFF_THROW` | 13 | 0x2000 | Trigger on throw |
| `TRAP_EFF_ARMED1` | 14 | 0x4000 | Arming state 1 |
| `TRAP_EFF_ARMED2` | 15 | 0x8000 | Arming state 2 |
| `TRAP_EFF_ARMED3` | 16 | 0x10000 | Arming state 3 |

**MAX_TRAP_EFF = 17**

**Source:** `code/code/misc/trap.h`

## Trap Function Categories

The trap system comprises ~60+ functions across multiple files.

### Primary Trigger Functions

| Function | Location | Purpose |
|----------|----------|---------|
| `springTrap()` | trap.cc | Main trap dispatcher |
| `triggerTrap()` | trap.cc | Character-triggered handler |
| `triggerDoorTrap()` | trap.cc | Door trap activation |
| `triggerContTrap()` | trap.cc | Container trap activation |
| `triggerPortalTrap()` | trap.cc | Portal trap activation |
| `triggerArrowTrap()` | trap.cc | Arrow impact trap |
| `triggerMineTrap()` | trap.cc | Mine explosion |
| `triggerGrenadeTrap()` | trap.cc | Grenade detonation |

### Detection Functions

| Function | Location | Purpose |
|----------|----------|---------|
| `checkForMoveTrap()` | trap.cc | Check for movement traps |
| `checkForInsideTrap()` | trap.cc | Check container-inside traps |
| `checkForAnyTrap()` | trap.cc | Generic trap check |
| `checkForGetTrap()` | trap.cc | Check when picking up |
| `checkForPortalTrap()` | trap.cc | Portal-specific checks |

### Thief Skill Functions

| Function | Location | Purpose |
|----------|----------|---------|
| `disarmTrapObj()` | disc_thief_looting.cc | Disarm object traps |
| `disarmTrapDoor()` | disc_thief_looting.cc | Disarm door traps |
| `disarmMe()` | disc_thief_looting.cc | TTrap::disarmMe() |
| `detectMe()` | disc_thief_looting.cc | TTrap::detectMe() |
| `detectTrapObj()` | disc_thief_looting.cc | Detect object traps |
| `detectTrapDoor()` | disc_thief_looting.cc | Detect door traps |
| `detectSecret()` | disc_thief_looting.cc | Find hidden doors |

### Damage Calculation Functions

| Function | Trap Type | Damage Modifier |
|----------|-----------|-----------------|
| `trapDoorPoisonDamage()` | Poison | 1.0× + AFF_POISON |
| `trapDoorSpikeDamage()` | Spike | 1.2× pierce |
| `trapDoorBladeDamage()` | Blade | 1.2× slash + bleeding |
| `trapDoorFireDamage()` | Fire | 0.8× cumulative |
| `trapDoorAcidDamage()` | Acid | 1.1× + structure damage |
| `trapDoorDiseaseDamage()` | Disease | 0.9× + AFF_DISEASE |
| `trapDoorHammerDamage()` | Hammer | 1.0× blunt + stun |
| `trapDoorFrostDamage()` | Frost | 1.0× + AFF_COLD |
| `trapDoorTntDamage()` | TNT | 2.0× area effect |
| `trapDoorEnergyDamage()` | Energy | 1.0× magic |
| `trapDoorBoltDamage()` | Bolt | 1.1× lightning |
| `trapDoorDiskDamage()` | Disk | 1.2× slash |
| `trapDoorPebbleDamage()` | Pebble | 0.9× area effect |
| `trapTeleportDamage()` | Teleport | N/A (displacement) |

## Trap Placement Mechanisms

### Door Traps

**Storage:** `roomDirData->trap_info` (trap type enum)
**Trigger:** Movement through exit with `TRAP_EFF_MOVE` set
**Detection:** `EXIT_TRAPPED` flag on exit condition

**Code Pattern:**
```cpp
// From trap.cc lines 215-256
strcpy(doorbuf, fname(exitp->keyword).c_str());
if (!IS_SET(exitp->condition, EXIT_TRAPPED)) {
    thief->sendTo("I don't think the %s is trapped.\n\r", doorbuf);
    return FALSE;
}
int bKnown = thief->getSkillValue(SKILL_DISARM_TRAP);
strcpy(trap_type, trap_types[exitp->trap_info].c_str());
```

### Container Traps

**Object Type:** `TTrap` (inherits from `TObj`)
**Storage:** `TTrap::trap_type` and `TTrap::trap_level` members
**Trigger Conditions:**
- Opening container (`TRAP_EFF_OBJECT`)
- Taking item from inside (`TRAP_EFF_OBJECT`)

**Code Pattern:**
```cpp
// From trap.cc lines 645-657
for (StuffIter it = roomp->stuff.begin(); it != roomp->stuff.end();) {
    t = *(it++);  // POST-INCREMENT CRITICAL!
    vict = dynamic_cast<TBeing*>(t);
    if (!vict) continue;
    rc = springTrap(this, vict, TRAP_TARG_CONT);
    if (IS_SET_DELETE(rc, DELETE_THIS)) {
        delete vict;
        vict = NULL;
    }
}
```

### Room-Wide Traps

**Flag:** `TRAP_EFF_ROOM` set on trap effect
**Trigger:** When any being triggers the trap
**Affects:** All beings in room simultaneously

**Code Pattern:**
```cpp
// From trap.cc lines 1126-1136
for (StuffIter it = roomp->stuff.begin(); it != roomp->stuff.end();) {
    t = *(it++);
    vict = dynamic_cast<TBeing*>(t);
    if (!vict) continue;
    int dam = getDoorTrapDam(this, SKILL_DISARM_TRAP);
    rc = ch->reconcileDamage(vict, dam, DAMAGE_POISON);
    if (rc == -1) return DELETE_VICT;
}
```

## CRITICAL: DELETE Flag Patterns

**267 DELETE flag occurrences in trap.cc alone**

### Pattern 1: DELETE_THIS (Character Death)

When the trap triggers and kills the character who triggered it:

```cpp
// From disc_thief_looting.cc lines 198-201
rc = thief->triggerTrap(this);
if (IS_SET_DELETE(rc, DELETE_VICT)) {
    return DELETE_VICT;  // Thief died from failed disarm
}
```

**When it occurs:**
- Character dies from trap damage
- Door trap with lethal damage kills triggerer
- Teleport trap to instant-death room

**Must propagate as:** `DELETE_THIS` when function's subject dies, `DELETE_VICT` when parameter dies

### Pattern 2: DELETE_VICT (Victim Death)

When trap kills a victim parameter:

```cpp
// From trap.cc lines 518-530
for (StuffIter it = roomp->stuff.begin(); it != roomp->stuff.end();) {
    t = *(it++);
    vict = dynamic_cast<TBeing*>(t);
    if (vict && sameRoom(*vict)) {
        int dam = trapDoorTntDamage(this, vict);
        rc = ch->reconcileDamage(vict, dam, DAMAGE_BLAST);
        if (rc == -1)
            return DELETE_VICT;  // Someone in room died
    }
}
```

### Pattern 3: DELETE_ITEM (Object Destruction)

When trap destroys an object:

```cpp
// From obj_trap.cc lines 204-214
for (StuffIter it = roomp->stuff.begin(); it != roomp->stuff.end();) {
    t = *(it++);
    TTrap* trap = dynamic_cast<TTrap*>(t);
    if (trap && trap->getTrapDamType() == TRAP_TARG_GRENADE) {
        rc = springTrap(NULL, ch, TRAP_TARG_GRENADE);
        if (IS_SET_DELETE(rc, DELETE_ITEM)) {
            delete trap;
            trap = NULL;
        }
    }
}
```

### Pattern 4: Combined Flags

```cpp
// When both attacker and victim die
if (IS_SET_DELETE(rc, DELETE_THIS) && IS_SET_DELETE(rc, DELETE_VICT))
    return rc;  // Return both flags
```

### Critical Safety Rule

**Never continue execution after a DELETE flag is detected.** Always check immediately and return or break:

```cpp
// CORRECT
rc = trap->disarmMe(thief);
if (IS_SET_DELETE(rc, DELETE_VICT)) {
    delete thief;
    thief = NULL;
    return DELETE_VICT;  // Exit immediately
}
thief->sendTo("You tried!");  // Safe - only reached if thief alive

// WRONG - CRASH
rc = trap->disarmMe(thief);
thief->sendTo("You tried!");  // CRASH if thief died in disarmMe()
```

## CRITICAL: reconcileDamage Returns -1, Not DELETE_VICT

**All trap damage goes through `reconcileDamage()` which returns `-1` on death, NOT a DELETE flag.**

```cpp
// CORRECT: Check for -1
rc = attacker->reconcileDamage(victim, damage, DAMAGE_TYPE);
if (rc == -1)
    return DELETE_VICT;

// WRONG: IS_SET_DELETE won't detect -1
if (IS_SET_DELETE(rc, DELETE_VICT)) { }  // Never triggers for -1!
```

See [Damage Pipeline](damage-pipeline.md) for complete documentation.

## CRITICAL: Iterator Safety Patterns

### Safe Pattern (ALWAYS USE)

```cpp
// SAFE: Post-increment before removal
for (StuffIter it = container->stuff.begin(); it != container->stuff.end();) {
    TThing* t = *(it++);  // Get pointer AND advance iterator
    vict = dynamic_cast<TBeing*>(t);
    if (!vict) continue;

    rc = springTrap(this, vict, TRAP_TARG_CONT);
    if (IS_SET_DELETE(rc, DELETE_THIS)) {
        delete vict;  // Safe - iterator already advanced
        vict = NULL;
    }
}
```

### Dangerous Pattern (NEVER USE)

```cpp
// CRASH: Iterator invalidated by deletion
for (StuffIter it = container->stuff.begin(); it != container->stuff.end(); ++it) {
    TThing* t = *it;
    vict = dynamic_cast<TBeing*>(t);

    rc = springTrap(this, vict, TRAP_TARG_CONT);
    if (IS_SET_DELETE(rc, DELETE_THIS)) {
        delete vict;  // Invalidates iterator!
    }
    // ++it on freed memory = CRASH
}
```

**Why:** The container list is invalidated when an element is removed. Post-incrementing `(it++)` captures the next position before removal happens.

## Thief Skill Mechanics

### Disarming

From `code/code/disc/disc_thief_looting.cc` lines 178-204:

```cpp
int TTrap::disarmMe(TBeing* thief) {
    int rc;
    char trap_type[80];
    int bKnown = thief->getSkillValue(SKILL_DISARM_TRAP);

    if (getTrapCharges() <= 0) {
        thief->sendTo("That trap is already disarmed.\n\r");
        return FALSE;
    }

    strcpy(trap_type, trap_types[getTrapDamType()].c_str());

    if (thief->bSuccess(bKnown, SKILL_DISARM_TRAP)) {
        // SUCCESS: Trap disarmed
        thief->sendTo(format("Click.  You disarm the %s trap.\n\r") % trap_type);
        act("$n disarms $p.", FALSE, thief, this, 0, TO_ROOM);
        setTrapCharges(0);
        return TRUE;
    } else {
        // FAILURE: Trap triggers on thief
        thief->sendTo("Click. (whoops)\n\r");
        act("$n tries to disarm $p.", FALSE, thief, this, 0, TO_ROOM);
        rc = thief->triggerTrap(this);
        if (IS_SET_DELETE(rc, DELETE_THIS)) {
            return DELETE_VICT;  // Thief died, notify caller
        }
        return TRUE;
    }
}
```

**Success Condition:** `bSuccess()` compares skill value to difficulty
**Charges System:** Traps have charge count; each trigger decrements
**DELETE_THIS Translation:** Thief's `DELETE_THIS` becomes `DELETE_VICT` for caller

### Detection

From `code/code/disc/disc_thief_looting.cc` lines 276-285:

```cpp
int TTrap::detectMe(TBeing* thief) const {
    int bKnown = thief->getSkillValue(SKILL_DETECT_TRAP);

    // Reduced detection rate for traps
    if (thief->bSuccess(bKnown / 10 + 1, SKILL_DETECT_TRAP))
        return TRUE;
    else
        return FALSE;
}
```

**Detection Rate:** Skill value divided by 10, plus 1. At max skill (100), only ~11% chance per check.

## Trap Damage Calculations

### Base Formula

```cpp
int getDoorTrapDam(TBeing* ch, spellNumT skill) {
    // Base: trap_level * class_amount
    // Modifiers: caster level, skill learning, spell resistance
    return (trap_level * classAmount) * skillModifier * resistanceMod;
}
```

### Type-Specific Modifiers

| Type | Multiplier | Additional Effect |
|------|------------|-------------------|
| Poison | 1.0× | +AFF_POISON for duration |
| Spike/Blade | 1.2× | Pierce/slash + bleeding |
| Fire | 0.8× | Cumulative over time |
| Acid | 1.1× | Damages container structure |
| Disease | 0.9× | +AFF_DISEASE for 24 hours |
| TNT | 2.0× | Area effect, all in room |
| Teleport | N/A | Displacement only, no damage |
| Energy | 1.0× | Pure magic damage |
| Bolt | 1.1× | Lightning spell-like |
| Disk | 1.2× | Slash projectile |
| Pebble | 0.9× | Area-effect projectiles |

## Common Crash Scenarios

### Scenario 1: Room Iterator During Room-Wide Trap

**Risk:** Trap kills characters while iterating through room

**Crash Code:**
```cpp
// CRASH: Pre-increment, using deleted pointer
for (auto it = roomp->stuff.begin(); it != roomp->stuff.end(); ++it) {
    TBeing* v = dynamic_cast<TBeing*>(*it);
    rc = reconcileDamage(v, dam, type);  // v might be deleted!
    if (rc == -1) {
        delete v;
    }
    v->sendTo("You hurt!");  // CRASH: Using freed memory
}
```

**Safe Code:**
```cpp
// SAFE: Post-increment, check before use
for (StuffIter it = roomp->stuff.begin(); it != roomp->stuff.end();) {
    TBeing* v = dynamic_cast<TBeing*>(*(it++));
    if (!v) continue;

    rc = reconcileDamage(v, dam, type);
    if (rc == -1) {
        delete v;
        v = NULL;
        continue;  // Don't use v after deletion
    }
    v->sendTo("You hurt!");  // Safe - only reached if v alive
}
```

### Scenario 2: Grenade with Old Parent Pointer

**Risk:** Grenade moves during detonation, old_parent becomes invalid

**Dangerous Code:**
```cpp
// From obj_trap.cc
TThing* old_parent = parent;  // Save parent pointer
// ... grenade detonates, moves objects around ...
if (old_parent) {
    old_parent->doSomething();  // old_parent might be deleted!
}
```

**Safe Code:**
```cpp
// Validate parent still exists
TThing* old_parent = parent;
// ... detonation code ...
if (old_parent && old_parent->inRoom() != Room::NOWHERE) {
    // Safe to use old_parent
    old_parent->doSomething();
}
```

### Scenario 3: Disarm Trigger on Thief

**Risk:** Thief dies from failed disarm, caller continues using thief pointer

**Crash Code:**
```cpp
rc = trap->disarmMe(thief);
thief->sendTo("You tried!");  // CRASH if thief died
```

**Safe Code:**
```cpp
rc = trap->disarmMe(thief);
if (IS_SET_DELETE(rc, DELETE_VICT)) {
    delete thief;
    thief = NULL;
    return DELETE_VICT;  // Exit immediately
}
thief->sendTo("You tried!");  // Safe - thief alive
```

### Scenario 4: Door Trap on Death

**Risk:** Door trap kills character, but code continues as if character alive

**Crash Code:**
```cpp
rc = triggerDoorTrap(ch, door);
ch->giveExperience(10);  // CRASH if ch died
```

**Safe Code:**
```cpp
rc = triggerDoorTrap(ch, door);
if (IS_SET_DELETE(rc, DELETE_THIS))
    return DELETE_THIS;  // Propagate death, don't continue
ch->giveExperience(10);  // Safe
```

### Scenario 5: Nested Container Explosion

**Risk:** Container inside container detonates, invalidating both iterators

**Safe Code:**
```cpp
// Build safe list first
std::vector<TThing*> contents;
for (auto it = container->stuff.begin(); it != container->stuff.end(); ++it) {
    contents.push_back(*it);
}

// Then iterate and delete safely
for (TThing* t : contents) {
    if (!t || t->parent != container) continue;  // Validate still in container
    rc = t->triggerTrap();
    if (IS_SET_DELETE(rc, DELETE_ITEM)) {
        delete t;
        t = NULL;
    }
}
```

## Integration Points

Traps integrate with:

1. **Movement System:** Movement triggers check for room-wide traps via `checkForMoveTrap()`
2. **Container System:** Opening/taking items triggers `checkForGetTrap()`
3. **Combat System:** Trap damage goes through `reconcileDamage()` using standard damage pipeline
4. **Thief Skills:** `SKILL_DISARM_TRAP` and `SKILL_DETECT_TRAP` control thief mechanics
5. **Spell System:** Trap effects apply via standard `affectJoin()` (sleep, poison, disease)
6. **Damage Pipeline:** All trap damage follows the -1 death return convention, NOT DELETE flags
7. **Scheduler:** `procObjSpecProcs` runs trap-specific code periodically
8. **Room System:** Exit data (`roomDirData`) stores door trap information

## Key Source Files

| File | Purpose | Key Functions |
|------|---------|---------------|
| `code/code/misc/trap.h` | Trap type/flag definitions | doorTrapT, trap_targ_t enums |
| `code/code/misc/trap.cc` | Core trap logic (~60+ functions) | springTrap, triggerTrap, damage functions |
| `code/code/disc/disc_thief_looting.cc` | Thief disarm/detect | disarmMe, detectMe |
| `code/code/obj/obj_trap.h` | TTrap class definition | TTrap member declarations |
| `code/code/obj/obj_trap.cc` | TTrap implementation | Grenade detonation |

## Best Practices Summary

1. **Always use post-increment iterators** when iterating containers during trap processing: `*(it++)`
2. **Check DELETE flags immediately** after trap triggers and return/break
3. **Check reconcileDamage() for -1** (death), NOT for DELETE_VICT flag
4. **Never continue execution** after a DELETE flag is detected
5. **Validate pointers** after any trap operation that might cause deletion
6. **Use safe list pattern** for nested container explosions (build list, then process)
7. **Always propagate DELETE flags** to callers when function parameters die
8. **Check IS_SET_DELETE(), not IS_SET()** for DELETE flags (they use bit 29)

## Related Documentation

- [DELETE Flag System](delete-flags.md) - Complete DELETE_* flag documentation
- [Damage Pipeline](damage-pipeline.md) - reconcileDamage() and the -1 return value
- [Command Implementation](command-implementation.md) - DELETE flag translation patterns
- [Spatial Relationships](spatial-relationships.md) - Container pointer relationships
