---
title: Scheduler and Pulse System
category: critical
keywords: [TScheduler, TPulse, TProcess, TObjProcess, TCharProcess, DELETE_THIS, gameLoop, proc adapter, pulse frequency, timing]
related: [task-system.md, combat-system.md, memory-safety.md]
primary_symbols:
  functions: [gameLoop, procPerformViolence, procCharAffects, procObjBurning, procZoneUpdate]
  classes: [TScheduler, TPulse, TProcess, TObjProcess, TCharProcess, TBaseProcess]
  files: [code/code/sys/process.h, code/code/sys/process.cc, code/code/sys/socket.cc, code/code/sys/comm.h]
---

# Scheduler and Pulse System

## Overview

The scheduler is SneezyMUD's central timing mechanism that orchestrates all periodic game updates from combat rounds to weather changes. It distributes processing load across game ticks to maintain responsive gameplay without lag spikes.

The system operates on a pulse counter incremented every game loop iteration, with each pulse representing 0.1 seconds of real time. All game timing derives from this base unit. Processes register themselves with specific pulse frequencies, and the scheduler determines which processes should execute on each pulse by checking if the pulse counter is evenly divisible by the process's trigger frequency.

Three separate process lists handle different entity types. Global processes handle system-wide updates like network I/O and weather. Per-object processes handle entity-specific behavior like burning or falling. Per-character processes handle being-specific behavior like drowning or spell effects. This separation enables targeted iteration through only the relevant entities for each type of update.

To prevent lag spikes from processing large entity collections, the scheduler employs load distribution. Object and character processes execute on only a fraction of their respective lists per pulse, cycling through the complete collection over multiple pulses. This distributes the computational load smoothly across time rather than creating periodic spikes.

The scheduler uses an adapter pattern to bridge between two return-value conventions. Game layer functions return DELETE flags indicating which entities should be deleted. Scheduler processes translate these flags to boolean signals that the scheduler uses to trigger actual deletion. This separation allows game code to remain ignorant of scheduler internals while enabling the scheduler to safely manage entity lifetimes.

## Patterns

### Process Registration

Processes register in gameLoop by calling scheduler.add with a new process instance and pulse frequency. The pulse frequency determines how often the process executes. Multiple processes can share the same frequency, and they execute in registration order.

Register global processes with TProcess for system-wide updates that don't iterate entities. Register per-object processes with TObjProcess to execute logic on each object. Register per-character processes with TCharProcess to execute logic on each being. The scheduler automatically handles iteration and load distribution for object and character processes.

Choose pulse frequencies based on how often the update needs to occur and its computational cost. Expensive operations should use slower frequencies. Time-sensitive operations like combat use faster frequencies. Network I/O uses the fastest frequency to maintain responsiveness.

### DELETE Flag Conversion

Every TObjProcess and TCharProcess run method must capture the return value from game functions and check for DELETE flags using IS_SET_DELETE. Convert DELETE_THIS to return true, signaling the scheduler to delete the entity. Convert no-delete conditions to return false, signaling the scheduler to keep the entity.

Check deletion flags immediately after calling potentially dangerous functions. Never dereference the entity pointer after detecting a deletion flag. Return true immediately to prevent use-after-free. The scheduler handles actual deletion after the process returns.

For object spec procs, check DELETE_ITEM rather than DELETE_THIS. Different game functions use different flag names for the same logical concept. Always verify which flag name the called function uses by checking its documentation or return value patterns.

### Batch Character Deletion

Character processes collect beings to delete in a vector during iteration, then delete them after iteration completes. This prevents iterator invalidation in the global CharacterList. Never delete a being directly inside a character process run method. Always return true to signal deletion and let the scheduler handle it.

Object processes can delete immediately because they control their own iteration state with a placeholder pattern. The placeholder maintains position even when objects are removed from the list. Character processes cannot use this approach because CharacterList is global and shared across many systems.

### Load Distribution Implementation

Object and character process execution divides the entity count by 11.5 to determine how many entities to process per pulse. This fractional division ensures the complete collection is processed once per 12 ticks (1.2 seconds). A placeholder object or position marker maintains iteration state across pulse invocations.

The placeholder advances through the collection by processing count entities, stopping at its new position. On the next pulse, iteration resumes from the placeholder. When the placeholder reaches the end, it wraps back to the beginning. This creates a continuous rotation through all entities without ever missing any or processing some twice.

### Creating New Processes

Declare the process class in process.h inheriting from TProcess, TObjProcess, or TCharProcess. Implement a constructor taking a const int reference for the pulse frequency. Implement the run method with the appropriate signature returning void for TProcess or bool for entity processes.

Set trigger_pulse and name in the constructor. The trigger_pulse determines execution frequency. The name appears in debugging output. Implement the run method to perform the actual work, checking DELETE flags and converting to boolean return for entity processes.

Register the process in gameLoop by calling scheduler.add with a new instance and the desired pulse frequency constant from the Pulse namespace. The process begins executing on the next pulse that is evenly divisible by the trigger frequency.

## Reference

### Pulse Frequency Constants

All timing derives from Pulse::ONE_SECOND equal to 10 ticks. Pulse::EVERY runs every tick (0.1 seconds) for socket I/O and room specs. Pulse::COMBAT runs every 12 ticks (1.2 seconds) for combat rounds, movement, and spell affects. Pulse::SPEC_PROCS runs every 36 ticks (3.6 seconds) for special procedures and drowning checks.

Pulse::NOISES runs every 48 ticks (4.8 seconds) for ambient sounds and hunger updates. Pulse::UPDATE runs every 360 ticks (36 seconds) for weather changes and player saves. Pulse::MUDHOUR runs every 1440 ticks (144 seconds) for zone resets and tick updates. Pulse::WAYSLOW runs every 2400 ticks (240 seconds) for mail checks and repo scans.

Pulse::MUDDAY runs every 34560 ticks (57.6 minutes) for auctions and bank interest. Pulse::REALHOUR runs every 36000 ticks (60 minutes) for trophy decay and RNG reseeding. These constants are defined in comm.h.

### Core Process Classes

TBaseProcess provides the base timing interface with trigger_pulse indicating execution frequency and should_run checking if the current pulse is evenly divisible by trigger_pulse. All concrete process classes inherit from TBaseProcess through one of three derived types.

TProcess handles global processes with run taking a const TPulse reference and returning void. Use TProcess for system-wide updates that don't iterate entities. Examples include network I/O, weather changes, and zone resets.

TObjProcess handles per-object processes with run taking a const TPulse reference and TObj pointer, returning bool. Return true to delete the object, false to keep it. The scheduler handles iteration through all objects. Examples include burning, falling, and freezing.

TCharProcess handles per-character processes with run taking a const TPulse reference and TBeing pointer, returning bool. Return true to delete the being, false to keep it. The scheduler handles iteration through all beings using batch deletion. Examples include drowning, spell affects, and regeneration.

### TPulse Precomputed Flags

TPulse precomputes which pulse categories are active for a given tick number, avoiding repeated modulo operations in process implementations. The init method sets boolean flags by checking if the pulse number is evenly divisible by each frequency constant.

Boolean fields include every, teleport, combat, drowning, special_procs, update_stuff, pulse_mudhour, mobstuff, pulse_tick, and wayslowpulse. Process implementations can check these flags directly rather than performing modulo operations repeatedly. The init12 variant aligns to multiples of 12 for object and character processing.

### Object Process List

procObjAutoPlant handles automatic plant growth. procObjBurning applies fire damage to burning objects. procObjDetonateGrenades triggers grenade explosions. procObjFalling moves objects downward through air sectors. procObjFreezing applies cold damage to frozen objects. procObjPools handles water pool evaporation.

procObjRiverFlow moves objects downstream in rivers. procObjRust applies corrosion to metal objects. procObjSinking moves objects downward through water sectors. procObjSmoke creates smoke effects. procObjSpecProcs and procObjSpecProcsQuick handle object special procedures at different frequencies.

procObjTeleportRoom teleports objects in teleport rooms. procObjTickUpdate handles object decay and condition updates. procObjTrash removes trash objects. procObjVehicle updates vehicle positions.

### Character Process List

procCharAffects updates spell durations and removes expired effects. procCharCantHit handles combat miss penalties. procCharDrowning checks breath underwater. procCharFalling moves beings downward through air sectors. procCharHalfTickUpdate performs mid-tick regeneration. procCharImmLeash enforces immortal area restrictions.

procCharLightning strikes beings in storms. procCharLycanthropy handles werewolf transformations. procCharMobileActivity triggers mobile AI behavior. procCharNoise generates ambient sound messages. procCharNutrition handles hunger and thirst. procCharRegen applies health/stamina/mana regeneration.

procCharResponses handles response triggers. procCharRiverFlow moves beings downstream. procCharScreenUpdate refreshes player displays. procCharSinking moves beings downward through water. procCharSpecProcs and procCharSpecProcsQuick handle character special procedures at different frequencies. procCharSpellTask and procCharTasks handle task system integration.

procCharTeleportRoom teleports beings in teleport rooms. procCharThaw handles frozen being recovery. procCharTickUpdate performs full tick updates including aging. procCharVampireBurn damages vampires in sunlight. procPaladinAura applies paladin holy auras.

### Global Process Examples

procHandleTimeAndSockets runs at Pulse::EVERY to process network I/O and descriptor state. procCallRoomSpec runs at Pulse::EVERY to execute room special procedures. procPerformViolence runs at Pulse::COMBAT to execute combat rounds. procWeatherAndTime runs at Pulse::UPDATE to change weather and advance game time. procZoneUpdate runs at Pulse::MUDHOUR to respawn mobs and objects in zones.

## Implementation

### TScheduler Architecture

TScheduler maintains three separate vectors: procs for global TProcess instances, obj_procs for TObjProcess instances, and char_procs for TCharProcess instances. The add method with function overloading allows registering any process type to the appropriate vector. This separation enables different iteration strategies for each process category.

The run method takes the current pulse number and executes all due processes. It creates a TPulse object initialized with the pulse number, providing precomputed frequency flags to processes. Global processes execute directly through iteration of the procs vector. Object and character processes require separate handling through runObj and runChar methods.

The pulse.init12 call aligns the pulse number to a multiple of 12 before object and character processing. This ensures load distribution works correctly by providing consistent divisibility checks across the 12-tick processing cycle. Without this alignment, the fractional entity counts would drift and create uneven load distribution.

### gameLoop Integration

gameLoop declares a TScheduler instance and a pulse counter initialized to zero. After registering all processes, it enters the main loop incrementing pulse and calling scheduler.run(pulse) each iteration. The tics counter tracks total iterations since server start.

Process registration occurs before the main loop to avoid repeated allocation. Each process type registers with its appropriate pulse frequency constant. Registration order matters for processes sharing the same frequency because they execute in registration order. Time-critical processes should register before less critical ones at the same frequency.

The handleShutdown check determines loop termination, allowing graceful shutdown on signals or admin commands. The scheduler continues running until shutdown completes, ensuring all entities reach consistent state before server termination.

### Object Processing with Immediate Deletion

runObj calculates the number of objects to process by dividing objCount by 11.5, ensuring the full object list cycles every 12 ticks. It maintains iteration state using a placeholder object inserted into the global object list. The placeholder advances by count positions each pulse.

For each object in the processing batch, runObj iterates through all obj_procs. If any process returns true, it immediately deletes the object and breaks from the process loop. This immediate deletion is safe because the placeholder maintains iteration position independently of object removals.

The placeholder pattern ensures every object gets processed exactly once per cycle without ever missing any. The fractional division prevents systematic skipping. The 11.5 divisor accounts for the placeholder itself occupying one list position.

### Character Processing with Batch Deletion

runChar calculates the number of beings to process by dividing mobCount by 11.5, ensuring the full being list cycles every 12 ticks. It maintains a position counter and deleteMe vector for deferred deletion. A tick tracking variable ensures each being is processed exactly once per cycle even if new beings are added mid-cycle.

For each being in the processing batch, runChar iterates through all char_procs. If any process returns true, it adds the being to deleteMe and breaks from the process loop. After all beings are checked, a final loop deletes all beings in deleteMe. This deferred deletion prevents iterator invalidation in the global CharacterList.

The batch deletion pattern is necessary because CharacterList is shared across many game systems. Immediate deletion would invalidate iterators in other subsystems that might be traversing the list concurrently. The deleteMe vector ensures deletion happens at a safe point after iteration completes.

### Deletion Flag Translation

Process run methods capture return values from game functions into int variables. They check for deletion flags using IS_SET_DELETE with the appropriate flag name. DELETE_THIS indicates the entity itself should be deleted. DELETE_ITEM indicates an object should be deleted. DELETE_VICT and other flags don't trigger deletion in process context because the entity being processed is always "this" not "vict."

After detecting a deletion flag, the process immediately returns true without dereferencing the entity pointer further. This prevents use-after-free if the entity is in invalid state. The scheduler receives the true return value and knows to delete the entity after the process returns.

If no deletion flag is set, the process returns false signaling the scheduler to keep the entity. The entity remains in the list and will be processed again on the next cycle. State changes made during processing persist for the next iteration.

### Process Frequency Selection

The pulse frequency determines how often a process executes and how much CPU time it consumes relative to other processes. Higher frequency means more executions per unit time and higher total CPU cost. Lower frequency means less frequent execution and lower total CPU cost.

Select frequencies based on gameplay requirements and computational cost. Combat-critical systems like procPerformViolence use Pulse::COMBAT to match combat round timing. Status effects like procCharAffects also use Pulse::COMBAT to update each combat round. Environmental hazards like procCharDrowning use Pulse::SPEC_PROCS to check every few seconds without overwhelming the scheduler.

Long-term maintenance like procObjTickUpdate and procCharTickUpdate use Pulse::MUDHOUR to update once per game hour. This is sufficient for decay, condition changes, and stat regeneration. Very infrequent operations like procCharVampireBurn might use even slower frequencies if precise timing is not gameplay-critical.

### Process Name Convention

Process names follow the pattern proc + EntityType + Feature. EntityType is Obj for TObjProcess subclasses or Char for TCharProcess subclasses. Global processes omit the entity type. Feature describes what the process does using camelCase.

Examples include procObjBurning for object burning, procCharDrowning for character drowning, procPerformViolence for global violence processing, and procZoneUpdate for global zone resets. This naming convention makes process purpose immediately clear and enables quick searching through the codebase.

The name field in each process instance should match the class name for debugging clarity. Stack traces and scheduler logs reference the name field, so consistency between class name and name field simplifies troubleshooting.

### Load Distribution Mathematics

The 11.5 divisor ensures exactly 12 pulses cover the complete entity list. If the divisor were 12.0, rounding would cause systematic skipping of some entities. The 0.5 offset accounts for the placeholder object occupying one position.

On each pulse, the scheduler processes floor(count / 11.5) entities. Over 12 pulses, the cumulative processed count equals the total entity count. The fractional arithmetic prevents drift that would occur with integer division. The placeholder advances by the exact processed count, maintaining precise position.

When new entities are added mid-cycle, they enter the list and will be processed when the placeholder reaches their position. When entities are removed, the placeholder maintains correct position because it advances by processed count rather than absolute index. This makes load distribution robust to dynamic entity populations.

## Troubleshooting

### Proc Returns True But Entity Not Deleted

Verify the process is registered in the correct vector. TObjProcess must be added to obj_procs, TCharProcess to char_procs. Adding to the wrong vector causes type mismatches that prevent execution. Check the gameLoop registration call uses the correct scheduler.add overload.

Verify the process actually executed by adding temporary logging to the run method. If the process never runs, check that the pulse frequency matches a registered value and that should_run returns true at the expected times. The pulse frequency must evenly divide the current pulse number for execution to occur.

For character processes, verify the being is in CharacterList. Beings not in the global list don't get processed by runChar. Check that being creation properly adds to CharacterList and removal happens only through proper deletion paths.

### Use-After-Free in Process Run Method

Check that every call to potentially dangerous game functions is followed immediately by a deletion flag check. If IS_SET_DELETE detects DELETE_THIS, return true immediately without touching the entity pointer again. Never defer the deletion check to later in the function.

Verify IS_SET_DELETE is used rather than IS_SET. The IS_SET macro doesn't correctly detect DELETE flag bit patterns. Only IS_SET_DELETE works correctly. Replace all IS_SET checks for DELETE flags with IS_SET_DELETE.

Check that DELETE_ITEM is used for object spec procs rather than DELETE_THIS. Object spec procs use DELETE_ITEM by convention even though the object is "this." Using the wrong flag name causes missed deletion signals.

### Load Distribution Skips Entities

Verify the divisor is 11.5 rather than 12.0. Integer division by 12 causes systematic skipping. The fractional divisor is essential for complete coverage. Check that the cast to float occurs before division to preserve fractional results.

Verify the placeholder or position marker advances by the actual processed count. If it advances by a fixed amount or uses a different calculation, position drift will occur over multiple cycles. The advancement must exactly match how many entities were processed.

Check that the cycle reset logic wraps correctly when reaching the end of the list. The placeholder should return to the beginning when it passes the last entity. If wrapping doesn't occur, entities at the list start will never be processed.

### Process Executes at Wrong Frequency

Verify the trigger_pulse is set in the constructor. If trigger_pulse is zero or uninitialized, should_run returns incorrect results. The trigger_pulse must match one of the Pulse namespace constants.

Check the registration call in gameLoop passes the intended frequency. If registration uses a different frequency than the constructor, the constructor value takes precedence. The registration frequency parameter is only used for informational purposes; the process's trigger_pulse determines actual execution.

Verify the pulse counter in gameLoop increments correctly each iteration. If the pulse counter doesn't increment or increments by wrong amounts, all timing breaks. The counter should increment by exactly 1 each gameLoop iteration.

### Crash in runChar After Process Returns True

Verify the deleteMe vector accumulates all deletion candidates before the deletion loop. Never delete a being immediately when a process returns true. Always add to deleteMe and defer deletion. Immediate deletion invalidates CharacterList iterators.

Check that the process breaks from the char_procs loop after returning true. If iteration continues through remaining processes after signaling deletion, those processes may crash accessing the invalid being. The break statement is essential.

Verify that no code between the deleteMe.push_back and the deletion loop dereferences beings in deleteMe. Once a being is added to the deletion queue, it may be in invalid state. Never access beings in deleteMe until deletion occurs.

### Memory Leak from Deleted Entities

Verify that processes registered with scheduler.add use new to allocate instances. The scheduler takes ownership and will delete process instances on destruction. If processes are stack-allocated or static, the scheduler cannot clean them up.

Check that process destructors clean up any resources allocated by the process. If a process allocates memory or resources in its constructor or run method, the destructor must free them. The scheduler only deletes the process object itself, not resources the process owns.

For complex cleanup requirements, consider using RAII patterns within the process implementation. Wrap resources in objects with proper destructors rather than managing cleanup manually. This ensures cleanup occurs even if the process terminates abnormally.

### Process Doesn't Execute After Registration

Verify the pulse frequency used in registration matches one that actually occurs. If the frequency is a prime number or doesn't divide any pulse value, the process never executes. Use only the Pulse namespace constants unless implementing a new frequency tier.

Check that should_run correctly implements the divisibility check. The default implementation checks if pulse modulo trigger_pulse equals zero. Custom should_run implementations must preserve this logic or provide equivalent frequency gating.

Verify the scheduler's run method is actually called from gameLoop. If the gameLoop is bypassed or the run call is commented out, no processes execute. The scheduler is passive; it only does work when explicitly told to by the main loop.
