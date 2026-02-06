---
title: Trap Mechanics
description: Dangerous obstacles on doors, containers, rooms, mines, grenades, and arrows with DELETE flag propagation and iterator safety requirements
keywords: [trap mechanics, disarm, detect, thief skills]
category: critical
source_files: [code/code/misc/trap.h, code/code/misc/trap.cc, code/code/disc/disc_thief_looting.cc, code/code/obj/obj_trap.h, code/code/obj/obj_trap.cc]
primary_symbols:
  functions: [springTrap, triggerTrap, triggerDoorTrap, triggerContTrap, triggerPortalTrap, triggerArrowTrap, checkForMoveTrap, checkForGetTrap, disarmMe, objDamage, trapDoorTntDamage, trapPoison, detonateGrenade]
  classes: [TTrap, roomDirData]
  enums: [doorTrapT, DOOR_TRAP_NONE, DOOR_TRAP_POISON, DOOR_TRAP_SPIKE, DOOR_TRAP_SLEEP, DOOR_TRAP_TNT, DOOR_TRAP_BLADE, DOOR_TRAP_FIRE, DOOR_TRAP_ACID, DOOR_TRAP_DISEASE, DOOR_TRAP_HAMMER, DOOR_TRAP_FROST, DOOR_TRAP_TELEPORT, DOOR_TRAP_ENERGY, DOOR_TRAP_BOLT, DOOR_TRAP_DISK, DOOR_TRAP_PEBBLE, trap_targ_t, TRAP_TARG_DOOR, TRAP_TARG_CONT, TRAP_TARG_MINE, TRAP_TARG_GRENADE, TRAP_TARG_ARROW, TRAP_EFF_MOVE, TRAP_EFF_OBJECT, TRAP_EFF_ROOM, TRAP_EFF_THROW, TRAP_EFF_ARMED1, TRAP_EFF_ARMED2, TRAP_EFF_ARMED3, EXIT_TRAPPED, AFF_POISON, AFF_DISEASE, AFF_SLEEP, AFF_COLD, SKILL_DISARM_TRAP, SKILL_DETECT_TRAP]
---

## Overview

Traps create environmental hazards that damage or affect characters when triggered. They attach to doors, containers, rooms, or exist as placeable objects (mines, grenades, arrows). Room-wide traps affect all occupants simultaneously. Thieves can detect and disarm traps using specialized skills, with failed disarms triggering the trap on the thief.

Trap damage flows through `objDamage()` which returns DELETE_THIS on death. Room-wide trap processing requires post-increment iterators to handle character deaths during iteration.

## Patterns

### DELETE Flag Handling

- Always check `objDamage()` return for DELETE_THIS using `IS_SET_DELETE()`
- Always check DELETE flags immediately after `springTrap()`, `triggerTrap()`, or `disarmMe()` calls
- Always propagate DELETE flags to callers when function parameters die
- Always translate DELETE_THIS to DELETE_VICT when the dying entity was a parameter
- Never continue execution after detecting a DELETE flag
- Never use `IS_SET()` for DELETE flags; use `IS_SET_DELETE()` which handles bit 29

### Iterator Safety

- Always use post-increment pattern `*(it++)` when iterating containers during trap processing
- Always validate pointers after any trap operation that might cause deletion
- Never increment iterators after deletion without post-increment
- Build a safe list first when processing nested container explosions

### Trap Processing

- Always check trap charges before processing; zero charges means already disarmed
- Always apply type-specific effects (AFF_POISON, AFF_DISEASE, AFF_SLEEP) via standard `affectJoin()`
- Always decrement trap charges after trigger to prevent multiple firings
- Always check DELETE_ITEM return and delete grenade/mine objects after detonation
- Never assume trap detection succeeds; detection passes `bKnown / 10 + 1` to bSuccess() for a reduced skill check

## Reference

### Trap Types (doorTrapT)

| Value | Constant | Damage Type | Effect |
|-------|----------|-------------|--------|
| 0 | DOOR_TRAP_NONE | - | No trap |
| 1 | DOOR_TRAP_POISON | Poison | +AFF_POISON |
| 2 | DOOR_TRAP_SPIKE | Pierce | Physical |
| 3 | DOOR_TRAP_SLEEP | - | +AFF_SLEEP |
| 4 | DOOR_TRAP_TNT | Blast | 2.0x area |
| 5 | DOOR_TRAP_BLADE | Slash | +bleeding |
| 6 | DOOR_TRAP_FIRE | Fire | Cumulative |
| 7 | DOOR_TRAP_ACID | Acid | Structure damage |
| 8 | DOOR_TRAP_DISEASE | Disease | +AFF_DISEASE 24h |
| 9 | DOOR_TRAP_HAMMER | Blunt | +stun |
| 10 | DOOR_TRAP_FROST | Cold | +AFF_COLD |
| 11 | DOOR_TRAP_TELEPORT | - | Random displacement |
| 12 | DOOR_TRAP_ENERGY | Energy | Magic |
| 13 | DOOR_TRAP_BOLT | Lightning | Spell-like |
| 14 | DOOR_TRAP_DISK | Slash | Projectile |
| 15 | DOOR_TRAP_PEBBLE | Blunt | Area projectiles |

### Target Types (trap_targ_t)

| Value | Constant | Attachment |
|-------|----------|------------|
| 0 | TRAP_TARG_DOOR | Doors/exits |
| 1 | TRAP_TARG_CONT | Containers |
| 2 | TRAP_TARG_MINE | Placed mines |
| 3 | TRAP_TARG_GRENADE | Thrown grenades |
| 4 | TRAP_TARG_ARROW | Stuck arrows |

### Effect Flags (17 bits)

| Flag | Hex | Purpose |
|------|-----|---------|
| TRAP_EFF_MOVE | 0x1 | Trigger on movement |
| TRAP_EFF_OBJECT | 0x2 | Trigger on get/put |
| TRAP_EFF_ROOM | 0x4 | Affect all in room |
| TRAP_EFF_NORTH-SW | 0x8-0x1000 | Direction filters |
| TRAP_EFF_THROW | 0x2000 | Trigger on throw |
| TRAP_EFF_ARMED1-3 | 0x4000-0x10000 | Arming states |

### Damage Multipliers

| Type | Multiplier | Notes |
|------|------------|-------|
| Poison | 1.0x | Duration-based |
| Spike/Blade/Disk | 1.2x | Pierce/slash |
| Fire | 0.8x | Cumulative |
| Acid | 1.1x | Degrades structure |
| Disease/Pebble | 0.9x | Secondary effects |
| TNT | 2.0x | Area damage |
| Teleport | N/A | No damage |
| Energy/Hammer | 1.0x | Standard |
| Bolt | 1.1x | Lightning |

### Key Functions

| Function | File | Purpose |
|----------|------|---------|
| springTrap | trap.cc | Main dispatcher |
| triggerTrap | trap.cc | Character handler |
| triggerDoorTrap | trap.cc | Door activation |
| triggerContTrap | trap.cc | Container activation |
| triggerPortalTrap | trap.cc | Portal activation |
| triggerArrowTrap | trap.cc | Arrow impact |
| checkForMoveTrap | trap.cc | Movement check |
| checkForGetTrap | trap.cc | Pickup check |
| checkForInsideTrap | trap.cc | Container-inside check |
| checkForAnyTrap | trap.cc | Generic trap check |
| disarmMe | disc_thief_looting.cc | TTrap disarm |
| detectMe | disc_thief_looting.cc | TTrap detect |
| disarmTrapDoor | disc_thief_looting.cc | Door trap disarm |
| disarmTrapObj | disc_thief_looting.cc | Object trap disarm |
| detectTrapDoor | disc_thief_looting.cc | Door trap detect |
| detectTrapObj | disc_thief_looting.cc | Object trap detect |
| getDoorTrapDam | trap.cc | Base damage calculation |
| trapDoorTntDamage | trap.cc | TNT-specific damage |

### TTrap Member Functions

| Method | Purpose |
|--------|---------|
| getTrapCharges() | Returns remaining trigger count |
| setTrapCharges(int) | Sets trigger count |
| getTrapDamType() | Returns trap type enum |
| getTrapLevel() | Returns difficulty level |

### Thief Skill Mechanics

| Skill | Check | Rate |
|-------|-------|------|
| SKILL_DISARM_TRAP | bSuccess(skillValue, skill) | Full skill value |
| SKILL_DETECT_TRAP | bSuccess(bKnown/10 + 1, skill) | Passes reduced skill value to bSuccess() for complex skill check |

## Implementation

### Door Trap Storage

Door traps use `roomDirData->trap_info` for type and `EXIT_TRAPPED` flag on exit condition. Detection requires checking `exitp->condition` for the trapped flag, then reading the trap type from `exitp->trap_info`. The `trap_types` string array provides human-readable names for messaging.

### Container Trap Storage

Container traps are `TTrap` objects (inheriting `TObj`) with `trap_dam_type`, `trap_level`, and `trap_charges` members. Triggers occur on container open or item extraction when `TRAP_EFF_OBJECT` is set.

### Room-Wide Processing

When `TRAP_EFF_ROOM` is set, the trap affects all beings in the room. Processing iterates `roomp->stuff` with post-increment to handle deletions. Each being receives damage via `objDamage()`, checking for DELETE_THIS returns. Continue iteration after DELETE_VICT to process remaining beings; only return early on DELETE_THIS when the triggerer dies.

### Disarm Mechanics

`TTrap::disarmMe()` checks `getTrapCharges()` returns greater than zero, then compares skill via `bSuccess()`. Success sets charges to 0 via `setTrapCharges()`. Failure calls `triggerTrap()` on the thief, potentially returning DELETE_VICT (translated from thief's DELETE_THIS).

### Damage Flow

All trap damage routes through `objDamage()`:
1. Calculate base damage from trap level and class modifiers via `getDoorTrapDam()`
2. Apply type-specific multiplier
3. Call `objDamage(damageType, damage, trapObject)`
4. Check return for `IS_SET_DELETE(rc, DELETE_THIS)` (death)
5. Apply secondary effects (poison, disease, sleep) via `affectJoin()`

### Integration Points

- **Movement**: `checkForMoveTrap()` called during room transitions
- **Containers**: `checkForGetTrap()` on item extraction
- **Combat**: Damage through `objDamage()` pipeline
- **Affects**: Effects applied via `affectJoin()` for sleep, poison, disease
- **Exits**: Door data in `roomDirData` structure
- **Scheduler**: `procObjSpecProcs` runs periodically for trap-specific code

## Troubleshooting

### Crash after room-wide trap triggers

**Symptom**: Server crash during TNT or other room-wide trap
**Cause**: Using pre-increment iterator; deletion invalidates iterator
**Fix**: Use `*(it++)` pattern to advance before potential deletion

### Crash after disarm attempt

**Symptom**: Server crash after thief fails disarm
**Cause**: Continuing execution after thief death without checking DELETE_VICT
**Fix**: Check `IS_SET_DELETE(rc, DELETE_VICT)` immediately after `disarmMe()` and return

### Deaths not detected from trap damage

**Symptom**: Dead characters continue acting after trap damage
**Cause**: Not checking `objDamage()` return for DELETE_THIS
**Fix**: Check `IS_SET_DELETE(rc, DELETE_THIS)` for death from `objDamage()`

### Use-after-free in trap damage messaging

**Symptom**: Heap-use-after-free in sendTo call after trap damage
**Cause**: Calling victim methods after `objDamage()` returned DELETE_THIS
**Fix**: Check `IS_SET_DELETE(rc, DELETE_THIS)` immediately; if true, delete victim, null pointer, return DELETE_VICT

### Grenade crash with stale parent pointer

**Symptom**: Crash when grenade detonates after movement
**Cause**: Cached parent pointer invalid after object relocation
**Fix**: Validate parent exists and `inRoom() != Room::NOWHERE` before use

### Nested container explosion crash

**Symptom**: Crash when trap inside container triggers another trap
**Cause**: Both iterators invalidated by nested deletion
**Fix**: Build safe vector of contents first, then iterate and validate each item still in container before processing

### Trap triggers multiple times

**Symptom**: Trap fires repeatedly when it should fire once
**Cause**: Trap charges not decremented or not checked before trigger
**Fix**: Check `getTrapCharges()` before allowing trigger; call `setTrapCharges()` with decremented value after

### Room-wide trap not affecting all beings

**Symptom**: Some beings in room avoid damage from TRAP_EFF_ROOM trap
**Cause**: Early return from iteration or missing TRAP_EFF_ROOM flag check
**Fix**: Continue iteration after DELETE_VICT; only return early on DELETE_THIS

### Door trap continues after triggerer death

**Symptom**: Segfault in movement code after door trap
**Cause**: Movement code did not check DELETE_THIS return from `triggerDoorTrap()`
**Fix**: Add `if (IS_SET_DELETE(rc, DELETE_THIS)) return DELETE_THIS;` immediately after call

### Memory leak from grenade detonation

**Symptom**: Grenade objects accumulate in memory after explosions
**Cause**: Not checking DELETE_ITEM return or not deleting grenade object
**Fix**: Check `IS_SET_DELETE(rc, DELETE_ITEM)` after `springTrap()`; delete trap and null pointer
