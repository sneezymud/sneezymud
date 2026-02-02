---
title: Trap Mechanics
description: Dangerous obstacles on doors, containers, rooms, mines, grenades, and arrows with DELETE flag propagation and iterator safety requirements
keywords: [trap, doorTrapT, TTrap, springTrap, disarm, detect, TRAP_EFF_ROOM]
category: Important Systems
related: [memory-safety.md, combat-formulas.md, spatial-relationships.md]
source_files:
  - code/code/misc/trap.h
  - code/code/misc/trap.cc
  - code/code/disc/disc_thief_looting.cc
  - code/code/obj/obj_trap.h
  - code/code/obj/obj_trap.cc
last_updated: 2026-02-01
created_by_model: opus
---

## Overview

Traps create environmental hazards that damage or affect characters when triggered. They attach to doors, containers, rooms, or exist as placeable objects (mines, grenades, arrows). Room-wide traps affect all occupants simultaneously. Thieves can detect and disarm traps using specialized skills, with failed disarms triggering the trap on the thief.

Trap damage flows through `reconcileDamage()` which returns -1 on death. This differs from most combat functions that return DELETE flags. Room-wide trap processing requires post-increment iterators to handle character deaths during iteration.

## Patterns

### DELETE Flag Handling

- Always check `reconcileDamage()` return for `-1`, never use `IS_SET_DELETE()` on it
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
- Never assume trap detection succeeds; detection rate is skill/10 + 1 percent

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
| checkForMoveTrap | trap.cc | Movement check |
| checkForGetTrap | trap.cc | Pickup check |
| disarmMe | disc_thief_looting.cc | TTrap disarm |
| detectMe | disc_thief_looting.cc | TTrap detect |

### Thief Skill Mechanics

| Skill | Check | Rate |
|-------|-------|------|
| SKILL_DISARM_TRAP | bSuccess(skillValue, skill) | Full skill value |
| SKILL_DETECT_TRAP | bSuccess(skillValue/10 + 1, skill) | ~11% at max skill |

## Implementation

### Door Trap Storage

Door traps use `roomDirData->trap_info` for type and `EXIT_TRAPPED` flag on exit condition. Detection requires checking `exitp->condition` for the trapped flag, then reading the trap type from `exitp->trap_info`.

### Container Trap Storage

Container traps are `TTrap` objects (inheriting `TObj`) with `trap_type` and `trap_level` members. Triggers occur on container open or item extraction when `TRAP_EFF_OBJECT` is set.

### Room-Wide Processing

When `TRAP_EFF_ROOM` is set, the trap affects all beings in the room. Processing iterates `roomp->stuff` with post-increment to handle deletions. Each being receives damage via `reconcileDamage()`, checking for -1 returns.

### Disarm Mechanics

`TTrap::disarmMe()` compares skill via `bSuccess()`. Success sets charges to 0. Failure calls `triggerTrap()` on the thief, potentially returning DELETE_VICT (translated from thief's DELETE_THIS).

### Damage Flow

All trap damage routes through `reconcileDamage()`:
1. Calculate base damage from trap level and class modifiers
2. Apply type-specific multiplier
3. Call `reconcileDamage(victim, damage, damageType)`
4. Check return for -1 (death) or positive (damage dealt)
5. Apply secondary effects (poison, disease, sleep) via `affectJoin()`

### Integration Points

- **Movement**: `checkForMoveTrap()` called during room transitions
- **Containers**: `checkForGetTrap()` on item extraction
- **Combat**: Damage through `reconcileDamage()` pipeline
- **Affects**: Effects applied via `affectJoin()`
- **Exits**: Door data in `roomDirData` structure

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
**Cause**: Using `IS_SET_DELETE(rc, DELETE_VICT)` on `reconcileDamage()` return
**Fix**: Check `rc == -1` for death from `reconcileDamage()`

### Grenade crash with stale parent pointer

**Symptom**: Crash when grenade detonates after movement
**Cause**: Cached parent pointer invalid after object relocation
**Fix**: Validate parent exists and `inRoom() != Room::NOWHERE` before use

### Nested container explosion crash

**Symptom**: Crash when trap inside container triggers another trap
**Cause**: Both iterators invalidated by nested deletion
**Fix**: Build safe vector of contents first, then iterate and validate each item still in container before processing
