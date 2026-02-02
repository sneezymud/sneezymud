---
title: Scheduler and Pulse System
description: Central timing mechanism orchestrating all periodic game updates
created_by_model: opus
category: Critical Systems
related:
  - task-system.md
  - memory-safety.md
source_files:
  - code/code/sys/process.h
  - code/code/sys/process.cc
  - code/code/sys/socket.cc
  - code/code/sys/comm.h
---

# Scheduler and Pulse System

## Overview

The scheduler distributes game processing across time ticks to maintain responsive gameplay. It manages three entity types (global, objects, characters) through separate process lists, executing registered handlers when their pulse frequency triggers. Load balancing processes only 1/12th of objects and characters per tick, completing full cycles every 1.2 seconds.

## Patterns

**Always convert DELETE_* flags to bool in scheduler procs.** Game functions return DELETE_THIS/DELETE_VICT/DELETE_ITEM; scheduler expects true (delete) or false (keep).

**Always use IS_SET_DELETE(), never IS_SET(), for DELETE flag checks.** IS_SET cannot detect the combined bit pattern.

**Always check DELETE flags immediately after dangerous function calls.** Never access entity state after a function that might have invalidated it.

**Always use DELETE_ITEM (not DELETE_THIS) for object spec proc returns.** Object spec procs follow a different flag convention.

**Never delete characters during iteration.** Collect in a vector, batch-delete after loop completes.

**Never continue execution after detecting a deletion signal.** Return immediately to prevent use-after-free.

## Reference

### Pulse Frequencies

| Constant | Ticks | Real Time | Purpose |
|----------|-------|-----------|---------|
| EVERY | 1 | 0.1s | Socket I/O, room specs |
| COMBAT | 12 | 1.2s | Combat rounds, movement, affects |
| SPEC_PROCS | 36 | 3.6s | Special procedures, drowning |
| NOISES | 48 | 4.8s | Ambient sounds, hunger/thirst |
| UPDATE | 360 | 36s | Weather, saves |
| MUDHOUR | 1440 | 144s | Zone resets, tick updates |
| WAYSLOW | 2400 | 240s | Mail checks, repo scans |
| MUDDAY | 34560 | 57.6min | Auctions, bank interest |
| REALHOUR | 36000 | 60min | Trophy decay, RNG reseeding |

Base unit: Pulse::ONE_SECOND = 10 ticks

### Process Classes

**Global (TProcess)**

| Class | Frequency | Purpose |
|-------|-----------|---------|
| procHandleTimeAndSockets | EVERY | Network I/O |
| procCallRoomSpec | EVERY | Room special procedures |
| procPerformViolence | COMBAT | Execute combat rounds |
| procWeatherAndTime | UPDATE | Weather changes |
| procZoneUpdate | MUDHOUR | Respawn mobs/objects |

**Object (TObjProcess)**

| Class | Frequency | Purpose |
|-------|-----------|---------|
| procObjFalling | COMBAT | Objects fall through air |
| procObjBurning | SPEC_PROCS | Fire damage |
| procObjRust | SPEC_PROCS | Rust damage |
| procObjSinking | SPEC_PROCS | Water submersion |
| procObjTickUpdate | MUDHOUR | Decay, condition updates |

**Character (TCharProcess)**

| Class | Frequency | Purpose |
|-------|-----------|---------|
| procCharAffects | COMBAT | Spell duration updates |
| procCharDrowning | SPEC_PROCS | Underwater breath checks |
| procCharFalling | SPEC_PROCS | Fall damage |
| procCharTickUpdate | MUDHOUR | Regen, hunger, aging |
| procCharMobileActivity | SPEC_PROCS | NPC wandering, actions |

### All Registered Procs

**TObjProcess:** procObjAutoPlant, procObjBurning, procObjDetonateGrenades, procObjFalling, procObjFreezing, procObjPools, procObjRiverFlow, procObjRust, procObjSinking, procObjSmoke, procObjSpecProcs, procObjSpecProcsQuick, procObjTeleportRoom, procObjTickUpdate, procObjTrash, procObjVehicle

**TCharProcess:** procCharAffects, procCharCantHit, procCharDrowning, procCharFalling, procCharHalfTickUpdate, procCharImmLeash, procCharLightning, procCharLycanthropy, procCharMobileActivity, procCharNoise, procCharNutrition, procCharRegen, procCharResponses, procCharRiverFlow, procCharScreenUpdate, procCharSinking, procCharSpecProcs, procCharSpecProcsQuick, procCharSpellTask, procCharTasks, procCharTeleportRoom, procCharThaw, procCharTickUpdate, procCharVampireBurn, procPaladinAura

## Implementation

### Core Architecture

**TScheduler** manages three process vectors: procs (global), obj_procs (per-object), char_procs (per-character). Each tick, it initializes TPulse with the current tick number, then iterates each vector calling run() on processes whose trigger_pulse divides evenly into the current pulse.

**TPulse** precomputes boolean flags for each pulse category, avoiding repeated modulo operations during process checks.

**TBaseProcess** provides the base should_run() check: `!(pulse % trigger_pulse)`. Derived types TProcess, TObjProcess, and TCharProcess add entity-specific run() signatures.

### Load Distribution

runObj() and runChar() process approximately 1/12th of their lists per call using integer division: `(int)((float)count / 11.5)`. A placeholder object maintains iteration position across calls, ensuring every entity is processed exactly once per 12-tick cycle.

### Adapter Pattern

The scheduler bridges two return conventions:
- **Game layer:** int with DELETE_* bit flags
- **Scheduler layer:** bool (true=delete, false=keep)

Proc implementations must translate: check IS_SET_DELETE(rc, DELETE_THIS) and return true if set.

### Character Batch Deletion

runChar() cannot delete during iteration because CharacterList is global. Instead, it collects deletions in a vector, then batch-deletes after iteration completes. This prevents iterator invalidation.

### Adding a New Process

1. Declare class in process.h inheriting TProcess/TObjProcess/TCharProcess
2. Implement constructor setting trigger_pulse and name
3. Implement run() returning appropriate bool/void
4. Register in gameLoop() via scheduler.add()

## Troubleshooting

**Symptom:** Character crashes with use-after-free during scheduler tick
**Cause:** Proc continued execution after DELETE_THIS was returned
**Fix:** Check IS_SET_DELETE immediately after dangerous calls, return true before any further access

**Symptom:** Object not being deleted despite proc returning true
**Cause:** Using IS_SET instead of IS_SET_DELETE to check flags
**Fix:** Replace IS_SET(rc, DELETE_THIS) with IS_SET_DELETE(rc, DELETE_THIS)

**Symptom:** Object spec proc deletion not working
**Cause:** Checking DELETE_THIS instead of DELETE_ITEM
**Fix:** Object spec procs return DELETE_ITEM; check IS_SET_DELETE(rc, DELETE_ITEM)

**Symptom:** Process runs at wrong frequency
**Cause:** Wrong Pulse constant passed to constructor
**Fix:** Verify correct constant (EVERY, COMBAT, SPEC_PROCS, etc.) for desired timing

**Symptom:** New process never executes
**Cause:** Missing registration in gameLoop()
**Fix:** Add scheduler.add(new procMyProcess(Pulse::FREQUENCY)) in socket.cc gameLoop()
