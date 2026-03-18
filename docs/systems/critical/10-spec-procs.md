---
title: Special Procedures (Spec Procs) System
description: Special procedures are callbacks giving mobs, objects, and rooms custom behavior on commands, pulses, and events through a flexible function pointer system.
keywords: [spec procs, special procedures, callback system, periodic processing, state machines]
category: critical
primary_symbols:
  functions: [triggerSpecial, checkSpec, swapToStrung]
  classes: [TMonster, TObj, TRoom, TMobSpecs, TObjSpecs, TRoomSpecs]
  enums: [CMD_GENERIC_PULSE, CMD_GENERIC_QUICK_PULSE, CMD_GENERIC_INIT, CMD_GENERIC_CREATED, CMD_GENERIC_DESTROYED, CMD_GENERIC_RESET, CMD_MOB_COMBAT, CMD_MOB_COMBAT2, CMD_MOB_COMBAT_ONATTACK, CMD_MOB_COMBAT_ONATTACKED, CMD_MOB_KILLED_NEARBY, CMD_MOB_VIOLENCE_PEACEFUL, CMD_MOB_MOVED_INTO_ROOM, CMD_OBJ_HIT, CMD_OBJ_HITTING, CMD_OBJ_MISS, CMD_OBJ_BEEN_HIT, CMD_OBJ_GOTTEN, CMD_OBJ_MOVEMENT, CMD_ROOM_ENTERED, CMD_ROOM_ATTEMPTED_EXIT, DELETE_THIS, DELETE_VICT, DELETE_ITEM, ITEM_STRUNG, ACT_STRINGS_CHANGED]
---

# Special Procedures (Spec Procs) System

## Overview

Special procedures are callback functions that attach custom behavior to mobs, objects, and rooms. They intercept game events, allowing entities to react to player commands, periodic pulses, lifecycle transitions, and combat actions.

The system provides three parallel implementations: mob specs, object specs, and room specs. Each maintains its own registry array mapping spec identifiers to handler functions. When the game processes commands, movement, or periodic updates, it invokes matching spec procs on all relevant entities.

Spec procs bridge static zone data with dynamic runtime behavior. A shopkeeper mob uses a spec proc to handle buying and selling. A magic portal object uses one to transport players. A trapped room uses one to trigger when players enter. This extensibility allows builders to create unique encounters without modifying core game code.

## Patterns

### Parameter Safety

Always verify the command type before dereferencing the final pointer parameter. Some commands pass integers cast to pointers rather than actual object references. Treat the parameter as an opaque value until you confirm the command type expects a pointer.

Never assume the victim or object parameter is valid for pulse commands. The parameter may be null, may have been deleted, or may represent something other than a pointer. Use `reinterpret_cast` to convert back to the appropriate integer type when handling commands that pass integers as pointers.

### Deletion Protocol

Never delete the proc's host entity directly. Return the appropriate DELETE flag and let the caller handle deletion. This prevents use-after-free when multiple stack frames reference the same entity.

Always use `IS_SET_DELETE()` to check for DELETE flags. The standard `IS_SET()` macro fails to detect the combined bit pattern that DELETE flags use.

Always free `act_ptr` memory in `CMD_GENERIC_DESTROYED`. Failing to do so causes memory leaks when entities are deleted.

### Periodic Processing

Use `CMD_GENERIC_PULSE` for most periodic behavior. It fires every 3.6 seconds, sufficient for ambient actions, room scanning, and NPC activities.

Reserve `CMD_GENERIC_QUICK_PULSE` for time-critical mechanics. It fires every 1.2 seconds but runs on all entities, so heavy processing impacts server performance. Move non-critical work to `CMD_GENERIC_PULSE` and limit quick pulse to truly time-critical logic.

Check `myself->awake()` and `myself->fight()` before doing periodic work. Sleeping or fighting entities typically should not perform ambient actions.

### State Management

Allocate persistent state on `CMD_GENERIC_CREATED` or the first pulse. Store the pointer in `act_ptr`. Always check for null before accessing state since creation hooks may not have fired.

Use lazy initialization when entities might bypass `CMD_GENERIC_CREATED`. Some loading paths (like rent file loading) skip the creation event, leaving `act_ptr` null until the first pulse.

Call `swapToStrung()` before modifying entity strings. Without it, changes may affect the prototype or be lost on save.

Common state machine patterns include patrol routes where a `cur_path` field selects which route to follow and `cur_pos` tracks progress along the route. Quest NPCs track conversation phase and target information. Timed events increment tick counters and transition states when thresholds are reached.

### Command Handling

Return TRUE to consume a command and prevent further processing. Return FALSE to let the command continue to other handlers.

Parse arguments using standard string manipulation or the `one_argument` helper function. Validate that required objects or targets exist before proceeding.

For quick pulse initialization, wait until `CMD_GENERIC_QUICK_PULSE` to perform deferred setup. The entity may not be fully initialized during `CMD_GENERIC_CREATED`.

## Reference

### Spec Proc Types

| Type | Registry Array | Host Struct | Signature |
|------|----------------|-------------|-----------|
| Mob | `mob_specials[]` | `TMobSpecs` | `int proc(TBeing*, cmdTypeT, const char*, TMonster*, TObj*)` |
| Object | `objSpecials[]` | `TObjSpecs` | `int proc(TBeing*, cmdTypeT, const char*, TObj*, TObj*)` |
| Room | `roomSpecials[]` | `TRoomSpecs` | `int proc(TBeing*, cmdTypeT, const char*, TRoom*)` |

### Parameter Meanings

| Parameter | Mob Procs | Object Procs | Room Procs |
|-----------|-----------|--------------|------------|
| `ch` | Being triggering the proc (or nullptr) | Being triggering the proc | Being triggering the proc |
| `cmd` | Command type from `cmdTypeT` | Command type from `cmdTypeT` | Command type from `cmdTypeT` |
| `arg` | Command arguments (or nullptr) | Command arguments | Command arguments |
| `myself` | Mob instance owning the proc | Object instance owning the proc | Room instance owning the proc |
| `obj`/`t2` | Context-dependent (may be integer cast) | Context-dependent (may be integer cast) | N/A |

### Command Types

| Category | Commands | When Fired |
|----------|----------|------------|
| Periodic | `CMD_GENERIC_PULSE` | Every 3.6s via `Pulse::SPEC_PROCS` |
| Periodic | `CMD_GENERIC_QUICK_PULSE` | Every 1.2s via `Pulse::COMBAT` |
| Lifecycle | `CMD_GENERIC_INIT` | During zone file parsing (before full construction) |
| Lifecycle | `CMD_GENERIC_CREATED` | After `read_mobile()` or `read_object()` (fully initialized) |
| Lifecycle | `CMD_GENERIC_DESTROYED` | Before entity deletion |
| Lifecycle | `CMD_GENERIC_RESET` | During zone reset |
| Player | Positive `cmd` values | When player issues command |
| Combat | `CMD_MOB_COMBAT`, `CMD_MOB_COMBAT2` | During combat rounds |
| Combat | `CMD_MOB_COMBAT_ONATTACK`, `CMD_MOB_COMBAT_ONATTACKED` | On attack events |
| Combat | `CMD_MOB_KILLED_NEARBY`, `CMD_MOB_VIOLENCE_PEACEFUL` | Death and violence events |
| Object | `CMD_OBJ_HIT`, `CMD_OBJ_HITTING`, `CMD_OBJ_MISS`, `CMD_OBJ_BEEN_HIT` | Combat with equipped items |
| Object | `CMD_OBJ_GOTTEN`, `CMD_OBJ_MOVEMENT` | Item pickup and movement |
| Room | `CMD_ROOM_ENTERED`, `CMD_ROOM_ATTEMPTED_EXIT` | Room entry and exit attempts |

### Commands with Non-Pointer Final Parameter

| Command | Actual Value |
|---------|--------------|
| `CMD_MOB_MOVED_INTO_ROOM` | Old room number cast to `TObj*` |
| `CMD_MOB_KILLED_NEARBY` | Victim cast to `TObj*` |
| `CMD_MOB_VIOLENCE_PEACEFUL` | Violence target cast to `TObj*` |
| `CMD_OBJ_MOVEMENT` | `arg` castable to `dirTypeT` |
| `CMD_OBJ_HIT` | `arg` castable to `wearSlotT` |

### Return Values

| Value | Effect |
|-------|--------|
| `FALSE` (0) | Command continues to other handlers |
| `TRUE` (non-zero) | Command consumed, stop processing |
| `DELETE_THIS` | Host entity should be deleted by caller |
| `DELETE_VICT` | The `ch` parameter should be deleted by caller |
| `DELETE_ITEM` | The `obj`/`t2` parameter should be deleted by caller |

### DELETE Flag Dispatch by Calling Path

Object spec procs have two calling paths with different DELETE flag semantics. Using the wrong flag means the deletion is silently ignored.

**Command path** (`parse.cc triggerSpecial`): Handles player commands like CMD_SAY, CMD_PUT, etc. The dispatcher iterates room contents, calling `checkSpec()` on each object:

| Return Value | Dispatcher Action |
|--------------|-------------------|
| `DELETE_THIS` | Deletes the object (`delete t`) |
| `DELETE_VICT` | Returns `DELETE_THIS` to caller (signals the acting player died) |
| `DELETE_ITEM` | Returns `DELETE_VICT` to caller (signals another being died) |

**Pulse path** (`socket.cc procObjSpecProcs`/`procObjSpecProcsQuick`): Handles CMD_GENERIC_PULSE and CMD_GENERIC_QUICK_PULSE:

| Return Value | Dispatcher Action |
|--------------|-------------------|
| `DELETE_ITEM` | Marks the object for deletion |
| `DELETE_THIS` | Not checked - deletion is silently skipped |

Choose the flag matching your proc's trigger: `DELETE_THIS` for command-triggered procs, `DELETE_ITEM` for pulse-triggered procs. If a proc handles both paths and needs self-deletion from either, it must return the appropriate flag for each path.

### DELETE Flag Context for Mob Procs

| Context | `DELETE_THIS` Means | `DELETE_VICT` Means |
|---------|---------------------|---------------------|
| `CMD_GENERIC_PULSE`, `CMD_MOB_COMBAT` | Mob hosting the proc died | N/A |
| `CMD_MOB_VIOLENCE_PEACEFUL` | N/A | First TBeing should die |
| Generic player commands | Second TBeing (myself) died | First TBeing (ch) died |

### Source Files

| File | Content |
|------|---------|
| `code/code/spec/spec_mobs.h` | Mob spec constants (`SPEC_*`) |
| `code/code/spec/spec_mobs.cc` | Mob implementations, `mob_specials[]` registry |
| `code/code/spec/spec_objs.h` | Object spec constants |
| `code/code/spec/spec_objs.cc` | Object implementations, `objSpecials[]` registry |
| `code/code/spec/spec_rooms.h` | Room spec constants |
| `code/code/spec/spec_rooms.cc` | Room implementations, `roomSpecials[]` registry |
| `code/code/misc/parse.cc` | `triggerSpecial()` dispatcher |
| `code/code/misc/parse.h` | `cmdTypeT` enumeration |

## Implementation

### Registration and Invocation

The registration arrays `mob_specials`, `objSpecials`, and `roomSpecials` map spec constant values to function pointers. When a mob, object, or room is loaded with a spec proc assigned, the corresponding function pointer is looked up and stored in the entity's structure.

The `triggerSpecial()` function in `parse.cc` is the primary entry point for spec proc invocation. It builds a safe snapshot of room contents before iteration to handle procs that modify the room.

The function first checks whether the current command should interrupt ongoing tasks or spells. It then invokes the room's spec proc if one exists. Next, it iterates through the player's equipment and inventory, invoking object spec procs.

For room contents, the function copies all `TThing` pointers from the room's stuff list into a separate vector before iteration using post-increment iteration (`*(it++)`) to safely advance before potential modifications. During iteration, it validates each pointer still references an entity in the same room (checking `in_room` matches) before invoking its spec proc via `checkSpec()`.

After each spec proc call, the dispatcher checks return values for DELETE flags. If detected, it performs appropriate cleanup and may terminate iteration early.

### Periodic Scheduling

The game's pulse scheduler drives periodic spec proc invocation through two distinct pulse types.

`Pulse::SPEC_PROCS` fires every 3.6 seconds (3 game rounds). When it triggers, the scheduler iterates all mobs, objects, and rooms with spec procs, invoking each with `CMD_GENERIC_PULSE`. This pulse handles ambient NPC behavior, environmental effects, and time-based state changes.

`Pulse::COMBAT` fires every 1.2 seconds (1 combat round). It invokes `CMD_GENERIC_QUICK_PULSE` on all entities with spec procs. This pulse handles combat-related processing, moving vehicles, and other time-sensitive mechanics. The 3x higher frequency demands minimal per-call processing.

Each scheduler maintains iteration safety by caching next pointers before calling spec procs to handle cases where procs delete entities or modify container membership.

### Persistent State with act_ptr

Mobs, objects, and rooms each have an `act_ptr` member of type `void*`. Spec procs use this to store arbitrary persistent state between invocations.

The typical pattern allocates a state structure on `CMD_GENERIC_CREATED`. The proc stores the pointer in `act_ptr`, then retrieves it on subsequent invocations via `static_cast`. The structure might track position along a patrol route, accumulated player interactions, phase in a multi-step behavior, or any other state the proc needs.

Some procs use lazy initialization, allocating on the first `CMD_GENERIC_PULSE` instead of `CMD_GENERIC_CREATED`. This handles entities loaded through paths that skip the creation event.

State machines commonly track a phase counter and tick counter. The proc advances through phases based on elapsed ticks or external triggers, implementing complex multi-step behaviors.

Memory allocated to `act_ptr` must be freed in `CMD_GENERIC_DESTROYED`. The destructor does not automatically free this memory; the spec proc owns it and must clean it up explicitly. Cast to the correct type, call delete, and set `act_ptr` to nullptr to avoid use-after-free. Failing to clean up causes memory leaks proportional to entity creation and deletion rates.

Type safety depends on casting to the correct type. Casting to the wrong structure type causes undefined behavior. Each proc must use a consistent state type.

### String Customization with swapToStrung

The `swapToStrung()` function enables runtime customization of entity strings. By default, entities share string data with their prototype for memory efficiency. Calling this function copies prototype strings to instance-owned storage, allowing safe modification.

For objects, `TObj::swapToStrung()` checks `isObjStat(ITEM_STRUNG)` and returns early if already strung. Otherwise it sets the flag and copies name, short description, and other strings from the `obj_index` prototype. Subsequent modifications to these fields affect only this instance.

For mobs, `TMonster::swapToStrung()` checks the `ACT_STRINGS_CHANGED` flag in `specials.act` and returns if set. Otherwise it sets the flag and copies from `mob_index`. This enables disguises, transformations, and dynamically named NPCs.

When creating customized items like notes or signs, call `swapToStrung()` first, then modify the name, short description, and long description fields.

For deferred initialization, use `CMD_GENERIC_QUICK_PULSE` instead of `CMD_GENERIC_CREATED`. The quick pulse fires after the entity is fully initialized, whereas creation events may fire before all fields are populated. Check `isObjStat(ITEM_STRUNG)` to avoid redundant swapping.

Strung entities persist their custom strings to rent files. The strung flag ensures the game knows to save instance-specific data rather than just the prototype reference.

### Room Spec Proc Invocation

Room spec procs trigger when players enter rooms or attempt exits. The room's single spec proc, if assigned, receives these events plus periodic pulses.

`CMD_ROOM_ENTERED` fires after a player successfully moves into the room. The proc can react with messages, traps, or teleportation.

`CMD_ROOM_ATTEMPTED_EXIT` fires when a player tries to leave. Returning TRUE blocks the exit attempt, useful for locked doors or one-way passages.

### Combat Spec Procs

Combat triggers multiple mob spec proc events per round.

`CMD_MOB_COMBAT` and `CMD_MOB_COMBAT2` fire during the mob's combat round, allowing special attacks or defensive actions.

`CMD_MOB_COMBAT_ONATTACK` fires when the mob lands an attack. `CMD_MOB_COMBAT_ONATTACKED` fires when the mob receives an attack.

`CMD_MOB_KILLED_NEARBY` fires when something dies in the same room. The victim is passed as the final parameter (cast from pointer type).

`CMD_MOB_VIOLENCE_PEACEFUL` fires when violence occurs in a peaceful room. Returning `DELETE_VICT` kills the attacker.

Object spec procs also participate in combat. `CMD_OBJ_HIT`, `CMD_OBJ_HITTING`, `CMD_OBJ_MISS`, and `CMD_OBJ_BEEN_HIT` fire for equipped weapons and armor during attacks.

## Troubleshooting

### Crash on Pulse After Entity Deletion

**Symptom:** Server crashes during periodic pulse processing with use-after-free in spec proc code.

**Cause:** A spec proc deleted its host entity directly instead of returning a DELETE flag. The scheduler continues iterating with an invalid pointer.

**Diagnostic:** Enable AddressSanitizer. The crash will show the stale pointer access. Check the spec proc for direct calls to `delete` or entity destruction.

**Fix:** Replace direct deletion with the appropriate DELETE flag return. The scheduler expects to handle deletion after the proc returns.

### Memory Leak in Long-Running Server

**Symptom:** Server memory grows continuously. Heap profiling shows allocations from spec proc state structures.

**Cause:** The spec proc allocates `act_ptr` state but does not free it in `CMD_GENERIC_DESTROYED`.

**Diagnostic:** Add logging to `CMD_GENERIC_DESTROYED` handlers. Check if the cleanup branch executes when entities are deleted.

**Fix:** Add cleanup code to cast `act_ptr` to the correct type, call delete, and set it to nullptr in the `CMD_GENERIC_DESTROYED` handler.

### Crash When Dereferencing obj/t2 Parameter

**Symptom:** Null pointer dereference or garbage read when accessing the final parameter in a spec proc.

**Cause:** The command type passes an integer cast to pointer type, not an actual object pointer. Dereferencing this garbage causes undefined behavior.

**Diagnostic:** Check the command type being processed. Compare against the table of commands with non-pointer final parameters.

**Fix:** Gate pointer dereference on specific command types known to pass valid pointers. For commands that pass integers, use `reinterpret_cast` to convert back to the appropriate integer type.

### IS_SET Fails to Detect DELETE Flags

**Symptom:** DELETE flags are returned but callers do not detect them. Entities are not deleted, causing state corruption.

**Cause:** Code uses `IS_SET()` instead of `IS_SET_DELETE()`. The standard macro cannot detect the combined bit pattern DELETE flags use.

**Diagnostic:** Search for `IS_SET(rc, DELETE` patterns. These are all bugs.

**Fix:** Replace with `IS_SET_DELETE(rc, DELETE_THIS)` or the appropriate flag.

### Strung Strings Not Persisting to Rent

**Symptom:** Object customizations (name, description) reset when the server restarts or player logs out and back in.

**Cause:** The object was modified without first calling `swapToStrung()`. Without the strung flag, the save system does not persist instance strings.

**Diagnostic:** Check `isObjStat(ITEM_STRUNG)` after customization. If false, the swap was not performed.

**Fix:** Call `swapToStrung()` before modifying any string fields.

### Quick Pulse Initialization Fires Multiple Times

**Symptom:** Deferred initialization in `CMD_GENERIC_QUICK_PULSE` runs repeatedly, causing duplicate setup or resource exhaustion.

**Cause:** The initialization guard condition does not properly detect that setup completed.

**Diagnostic:** Log each initialization attempt. Verify the guard flag is set after first initialization.

**Fix:** Use `isObjStat(ITEM_STRUNG)` or a similar flag to gate one-time initialization. Ensure the flag is set during initialization, not just after.

### Spec Proc Not Firing

**Symptom:** A newly added spec proc never executes despite being assigned to an entity.

**Cause:** The spec proc is not registered in the appropriate specials array, or the constant does not match the zone file assignment.

**Diagnostic:** Verify the spec constant value matches the zone file. Check that the function is in the specials array at the correct index. Confirm the entity has the correct spec constant assigned.

**Fix:** Add the spec proc to the registry array at the index matching its constant. Ensure the zone file uses the correct spec number.

### Command Not Suppressed Despite Returning TRUE

**Symptom:** Spec proc returns TRUE but the command continues to execute normally.

**Cause:** The return value was not propagated correctly, or the spec proc was not registered in the appropriate array.

**Diagnostic:** Verify spec proc is registered in the correct specials array. Confirm all call sites propagate the return value correctly and respect TRUE as command-handled.

**Fix:** Ensure the spec proc is properly registered and that callers check and respect the return value.

### Performance Degradation from Quick Pulse

**Symptom:** Server performance degrades as entity count increases, with profiling showing time spent in spec proc handlers.

**Cause:** Heavy processing in `CMD_GENERIC_QUICK_PULSE` handlers. Quick pulse fires every 1.2 seconds on all entities, so expensive operations multiply across many procs.

**Diagnostic:** Profile to identify expensive operations in quick pulse handlers.

**Fix:** Move non-critical work to `CMD_GENERIC_PULSE` which fires every 3.6 seconds. Cache results and avoid repeated calculations. Limit quick pulse to truly time-critical logic.
