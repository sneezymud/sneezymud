---
title: Scheduler and Pulse System
description: The central timing mechanism orchestrating all periodic game updates from combat rounds to weather changes, distributing processing load across game ticks to maintain responsive gameplay.
keywords:
  - TScheduler
  - TPulse
  - TProcess
  - TObjProcess
  - TCharProcess
  - Pulse::COMBAT
  - Pulse::UPDATE
  - Pulse::MUDHOUR
  - procPerformViolence
  - procCharAffects
  - procObjBurning
  - procZoneUpdate
  - gameLoop
  - DELETE_THIS
  - IS_SET_DELETE
category: Critical Systems

  - task-system.md
  - combat-system.md
last_updated: 2026-01-29
source_files:
  - code/code/sys/process.h
  - code/code/sys/process.cc
  - code/code/sys/socket.cc
  - code/code/sys/comm.h
related: [memory-safety.md]
---

# Scheduler and Pulse System

The scheduler is SneezyMUD's central timing mechanism, orchestrating all periodic game updates from combat rounds to weather changes. It distributes processing load across game ticks to maintain responsive gameplay.

## Core Components

### TScheduler

`TScheduler` manages three separate process lists, each handling different entity types:

```cpp
class TScheduler {
    std::vector<TProcess*> procs;         // Global processes (weather, saves, etc.)
    std::vector<TObjProcess*> obj_procs;  // Per-object processes (burning, sinking)
    std::vector<TCharProcess*> char_procs; // Per-character processes (drowning, affects)
};
```

**Source:** `/code/code/sys/process.h` lines 611-637

### TPulse

`TPulse` precomputes which pulse categories are active for a given tick number, avoiding repeated modulo operations:

```cpp
class TPulse {
    int pulse;
    bool every, teleport, combat, drowning, special_procs, update_stuff;
    bool pulse_mudhour, mobstuff, pulse_tick, wayslowpulse;

    void init(int pulse) {
        this->pulse = pulse;
        every = !(pulse % Pulse::EVERY);
        combat = !(pulse % Pulse::COMBAT);
        // ... etc
    }
};
```

**Source:** `/code/code/sys/process.h` lines 8-69

## Pulse Frequencies

All timing derives from `Pulse::ONE_SECOND = 10` ticks.

| Constant            | Ticks | Real Time   | Purpose                          |
|---------------------|-------|-------------|----------------------------------|
| `Pulse::EVERY`      | 1     | 0.1 sec     | Socket I/O, room specs           |
| `Pulse::COMBAT`     | 12    | 1.2 sec     | Combat rounds, movement, affects |
| `Pulse::SPEC_PROCS` | 36    | 3.6 sec     | Special procedures, drowning     |
| `Pulse::NOISES`     | 48    | 4.8 sec     | Ambient sounds, hunger/thirst    |
| `Pulse::UPDATE`     | 360   | 36 sec      | Weather, saves, half-tick        |
| `Pulse::MUDHOUR`    | 1440  | 144 sec     | Zone resets, tick updates        |
| `Pulse::WAYSLOW`    | 2400  | 240 sec     | Mail checks, repo scans          |
| `Pulse::MUDDAY`     | 34560 | 57.6 min    | Auctions, bank interest          |
| `Pulse::REALHOUR`   | 36000 | 60 min      | Trophy decay, RNG reseeding      |

**Source:** `/code/code/sys/comm.h` lines 44-78

## Process Hierarchy

All processes inherit from `TBaseProcess`, which provides the timing check:

```cpp
class TBaseProcess {
    int trigger_pulse;  // Run when (pulse % trigger_pulse == 0)
    sstring name;

    virtual bool should_run(int p) const {
        return !(p % trigger_pulse);
    }
};
```

Three derived types handle different execution contexts:

- **TProcess** - Global processes, `void run(const TPulse&)`
- **TObjProcess** - Object processes, `bool run(const TPulse&, TObj*)` (returns true to delete object)
- **TCharProcess** - Character processes, `bool run(const TPulse&, TBeing*)` (returns true to delete character)

**Source:** `/code/code/sys/process.h` lines 72-102

## Scheduler Execution

### Main Loop

The game loop increments the pulse counter and calls `scheduler.run()` each iteration:

```cpp
int TMainSocket::gameLoop() {
    TScheduler scheduler;
    int pulse = 0;

    // Register all processes...

    while (!handleShutdown()) {
        scheduler.run(++pulse);
        tics++;
    }
}
```

**Source:** `/code/code/sys/socket.cc` lines 1594-1714

### TScheduler::run()

Executes all due processes for the current pulse:

```cpp
void TScheduler::run(int pulseNum) {
    pulse.init(pulseNum);

    // Run global processes
    for (TProcess* proc : procs) {
        if (proc->should_run(pulse.pulse))
            proc->run(pulse);
    }

    pulse.init12(pulseNum);  // Align to multiple of 12 for obj/char

    runObj(pulseNum);   // Process objects
    runChar(pulseNum);  // Process characters
}
```

**Source:** `/code/code/sys/process.cc` lines 221-240

### Load Distribution

To prevent lag spikes, `runObj()` and `runChar()` process only 1/12th of their respective lists per call, completing a full cycle every 12 ticks (1.2 seconds):

```cpp
void TScheduler::runObj(int pulseNum) {
    // Process approximately 1/12th of objects
    int count = (int)((float)objCount / 11.5);

    while (count--) {
        // Process one object through all obj_procs
        // Delete if any process returns true
    }
}
```

A placeholder object maintains iteration position across calls, ensuring every object gets processed exactly once per cycle.

**Source:** `/code/code/sys/process.cc` lines 101-144

### Character Processing with Batch Deletion

`runChar()` collects characters to delete in a vector, then deletes them after iteration completes:

```cpp
void TScheduler::runChar(int pulseNum) {
    int count = max((int)((float)mobCount / 11.5), 1);
    std::vector<TBeing*> deleteMe;

    for (TBeing* ch : CharacterList) {
        // Skip already-processed characters this cycle
        // Run all char_procs, collect deletions
    }

    for (TBeing* being : deleteMe)
        delete being;
}
```

**Source:** `/code/code/sys/process.cc` lines 146-219

## Process Registration

Processes are registered in `gameLoop()` by pulse frequency. Example registrations:

```cpp
// Every tick (0.1 sec)
scheduler.add(new procHandleTimeAndSockets(Pulse::EVERY));
scheduler.add(new procCallRoomSpec(Pulse::EVERY));

// Combat pulse (1.2 sec)
scheduler.add(new procPerformViolence(Pulse::COMBAT));
scheduler.add(new procCharAffects(Pulse::COMBAT));
scheduler.add(new procObjFalling(Pulse::COMBAT));

// Spec procs (3.6 sec)
scheduler.add(new procCharDrowning(Pulse::SPEC_PROCS));
scheduler.add(new procObjBurning(Pulse::SPEC_PROCS));

// Mudhour (144 sec)
scheduler.add(new procZoneUpdate(Pulse::MUDHOUR));
scheduler.add(new procCharTickUpdate(Pulse::MUDHOUR));
```

**Source:** `/code/code/sys/socket.cc` lines 1600-1700

## Process Examples by Category

### Global Processes (TProcess)

| Process                     | Frequency   | Purpose                    |
|-----------------------------|-------------|----------------------------|
| `procHandleTimeAndSockets`  | EVERY       | Network I/O                |
| `procPerformViolence`       | COMBAT      | Execute combat rounds      |
| `procWeatherAndTime`        | UPDATE      | Weather changes            |
| `procZoneUpdate`            | MUDHOUR     | Respawn mobs/objects       |

### Object Processes (TObjProcess)

| Process                 | Frequency   | Purpose                    |
|-------------------------|-------------|----------------------------|
| `procObjFalling`        | COMBAT      | Objects fall through air   |
| `procObjBurning`        | SPEC_PROCS  | Fire damage to objects     |
| `procObjTickUpdate`     | MUDHOUR     | Decay, condition updates   |

### Character Processes (TCharProcess)

| Process                   | Frequency   | Purpose                    |
|---------------------------|-------------|----------------------------|
| `procCharAffects`         | COMBAT      | Update spell durations     |
| `procCharDrowning`        | SPEC_PROCS  | Underwater breath checks   |
| `procCharTickUpdate`      | MUDHOUR     | Regen, hunger, aging       |

## Creating New Processes

1. Declare the process class in `process.h`:

```cpp
class procMyNewProcess : public TCharProcess {
  public:
    bool run(const TPulse&, TBeing*) const;
    procMyNewProcess(const int&);
};
```

2. Implement in `process.cc` or appropriate file:

```cpp
procMyNewProcess::procMyNewProcess(const int& p) {
    trigger_pulse = p;
    name = "procMyNewProcess";
}

bool procMyNewProcess::run(const TPulse& pulse, TBeing* ch) const {
    // Return true to delete character, false to keep
    return false;
}
```

3. Register in `gameLoop()`:

```cpp
scheduler.add(new procMyNewProcess(Pulse::COMBAT));
```

## Proc Adapter Pattern

The scheduler system uses an **adapter pattern** to bridge between two different return-value conventions: the game layer's `DELETE_*` flags and the scheduler layer's boolean signals.

### Two Layers, Two Conventions

| Layer | Return Type | Meaning |
|-------|-------------|---------|
| **Game Layer** | `int` with `DELETE_*` flags | Signals which objects/beings should be deleted |
| **Scheduler Layer** | `bool` | `true` = delete the entity; `false` = keep it |

Game functions return `DELETE_THIS`, `DELETE_VICT`, `DELETE_ITEM`, etc. The scheduler only understands `true`/`false`. **Proc classes are the adapter layer** that translates between them.

### The Adapter in Practice

Every `TObjProcess` and `TCharProcess` subclass must convert `DELETE_*` flags to `bool`:

```cpp
// From socket.cc - procObjBurning::run()
bool procObjBurning::run(const TPulse& pl, TObj* obj) const {
  int rc = obj->updateBurning();       // Game layer: returns DELETE_* flags
  if (IS_SET_DELETE(rc, DELETE_THIS))  // Check the flag
    return true;                       // Scheduler layer: signal deletion
  return false;                        // Scheduler layer: keep object
}
```

### Scheduler Deletion Handlers

**Object Deletion (runObj):** When a proc returns `true`, the scheduler immediately deletes the object:

```cpp
if ((*iter)->run(pulse, obj)) {  // proc returns true
  delete obj;                     // scheduler performs deletion
  break;                          // stop processing this object
}
```

**Character Deletion (runChar):** Uses batch deletion to prevent iterator invalidation:

```cpp
std::vector<TBeing*> deleteMe;
for (TBeing* tmp_ch : CharacterList) {
  for (TCharProcess* char_proc : char_procs) {
    if (char_proc->run(pulse, tmp_ch)) {
      deleteMe.push_back(tmp_ch);  // collect for later
      break;
    }
  }
}
for (TBeing* being : deleteMe)
  delete being;  // batch deletion after iteration
```

This is necessary because `CharacterList` is a global container. Deleting during iteration would invalidate iterators.

### Complete Proc Implementation Example

```cpp
// process.h
class procObjMyFeature : public TObjProcess {
  public:
    bool run(const TPulse&, TObj*) const;
    procObjMyFeature(const int&);
};

// socket.cc
procObjMyFeature::procObjMyFeature(const int& p) {
  trigger_pulse = p;
  name = "procObjMyFeature";
}

bool procObjMyFeature::run(const TPulse& pl, TObj* obj) const {
  int rc = obj->someGameFunction();
  if (IS_SET_DELETE(rc, DELETE_THIS))
    return true;
  return false;
}

// gameLoop() in socket.cc
scheduler.add(new procObjMyFeature(Pulse::COMBAT));
```

Character procs follow the same pattern with `TCharProcess` and `TBeing*`.

### Common Gotchas

**1. Forgetting the Adapter Conversion**

```cpp
// WRONG: Ignoring return value - character may be in invalid state
bool procCharBad::run(const TPulse& pl, TBeing* ch) const {
  ch->someDangerousFunction();  // May return DELETE_THIS!
  return false;
}

// CORRECT: Always check DELETE_* flags
bool procCharGood::run(const TPulse& pl, TBeing* ch) const {
  int rc = ch->someDangerousFunction();
  if (IS_SET_DELETE(rc, DELETE_THIS))
    return true;
  return false;
}
```

**2. Wrong Flag for Spec Procs**

Object spec procs return `DELETE_ITEM`, not `DELETE_THIS`:

```cpp
int rc = obj->checkSpec(NULL, CMD_GENERIC_PULSE, "", NULL);
if (IS_SET_DELETE(rc, DELETE_ITEM))  // DELETE_ITEM, not DELETE_THIS
  return true;
```

**3. Using IS_SET Instead of IS_SET_DELETE**

```cpp
// WRONG: IS_SET won't detect the combined bit pattern
if (IS_SET(rc, DELETE_THIS)) { ... }

// CORRECT: Always use IS_SET_DELETE for DELETE_* flags
if (IS_SET_DELETE(rc, DELETE_THIS)) { ... }
```

**4. Continuing After Deletion Signal**

```cpp
// WRONG: CRASH if DELETE_THIS was set
int rc = obj->mightDelete();
obj->doSomethingElse();
if (IS_SET_DELETE(rc, DELETE_THIS)) return true;

// CORRECT: Check immediately, return early
int rc = obj->mightDelete();
if (IS_SET_DELETE(rc, DELETE_THIS)) return true;
obj->doSomethingElse();  // Safe: only reached if not deleted
```

## Proc Class Reference

### TObjProcess Classes

`procObjAutoPlant`, `procObjBurning`, `procObjDetonateGrenades`, `procObjFalling`, `procObjFreezing`, `procObjPools`, `procObjRiverFlow`, `procObjRust`, `procObjSinking`, `procObjSmoke`, `procObjSpecProcs`, `procObjSpecProcsQuick`, `procObjTeleportRoom`, `procObjTickUpdate`, `procObjTrash`, `procObjVehicle`

### TCharProcess Classes

`procCharAffects`, `procCharCantHit`, `procCharDrowning`, `procCharFalling`, `procCharHalfTickUpdate`, `procCharImmLeash`, `procCharLightning`, `procCharLycanthropy`, `procCharMobileActivity`, `procCharNoise`, `procCharNutrition`, `procCharRegen`, `procCharResponses`, `procCharRiverFlow`, `procCharScreenUpdate`, `procCharSinking`, `procCharSpecProcs`, `procCharSpecProcsQuick`, `procCharSpellTask`, `procCharTasks`, `procCharTeleportRoom`, `procCharThaw`, `procCharTickUpdate`, `procCharVampireBurn`, `procPaladinAura`

## Related Documentation

- [DELETE Flags](delete-flags.md) - Complete DELETE_* flag documentation
- `code/code/sys/process.h` - Class declarations
- `code/code/sys/process.cc` - TScheduler implementation
- `code/code/sys/socket.cc` - All proc::run() implementations
