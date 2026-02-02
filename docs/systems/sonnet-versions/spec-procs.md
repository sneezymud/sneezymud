---
title: Special Procedures (Spec Procs)
category: important
keywords: [callbacks, mob_specials, objSpecials, roomSpecials, CMD_GENERIC_PULSE, act_ptr, triggerSpecial, DELETE_THIS, swapToStrung, state_machine]
related: [delete-flags.md, command-system.md, object-system.md]
primary_symbols:
  functions: [triggerSpecial, checkSpec, swapToStrung]
  classes: [TMobSpecs, TObjSpecs, TRoomSpecs]
  files: [code/code/spec/spec_mobs.cc, code/code/spec/spec_objs.cc, code/code/spec/spec_rooms.cc, code/code/misc/parse.cc]
---

## Overview

Special procedures are callback functions that give mobs, objects, and rooms custom behavior beyond their base mechanics. They respond to player commands, periodic pulses, combat events, and lifecycle transitions through a function pointer system. Each entity type has its own signature and registry array: mobs use `mob_specials`, objects use `objSpecials`, and rooms use `roomSpecials`.

The system enables implementing complex behaviors such as shopkeepers, quest NPCs, magical items, transportation systems, and environmental hazards without modifying core game mechanics. Spec procs can maintain persistent state between calls using the `act_ptr` field, allowing state machines and multi-step interactions.

### Entity-Specific Signatures

Mob spec procs receive the triggering being, command type, arguments, the mob instance, and an optional object parameter. Object spec procs receive similar parameters but with the object instance and a second object parameter. Room spec procs receive the being, command, arguments, and the room instance.

The final parameter in mob and object spec procs carries different semantics depending on the command type. For some commands it represents an actual pointer, but for others it encodes an integer value cast to pointer type. Dereferencing this parameter without checking the command type causes segmentation faults.

### Return Value Semantics

Spec procs return FALSE when they perform no special action or TRUE when they handle the command and want to suppress normal processing. DELETE flags signal that an entity should be destroyed, following the deferred deletion pattern where the caller performs the actual deletion, not the proc. The owner of a pointer is responsible for deletion following ADR-001.

## Patterns

### Periodic Behavior Dispatch

Most spec procs respond to two pulse commands. `CMD_GENERIC_PULSE` fires every 3.6 seconds via the `Pulse::SPEC_PROCS` scheduler and handles most periodic behavior such as wandering, ambient actions, and room content checks. `CMD_GENERIC_QUICK_PULSE` fires every 1.2 seconds via `Pulse::COMBAT` for time-critical updates like moving vehicles or combat-related mechanics. Quick pulse runs three times more frequently, so processing should be minimal to avoid performance degradation.

Guard conditions typically check whether the entity is in an actionable state before performing work. For mobs, this means testing `awake()` and `fight()` to avoid acting while sleeping or in combat. Return FALSE immediately if conditions are not met.

### Command Interception

When the command type is non-negative, it represents a player command from the `cmdTypeT` enumeration. Spec procs can intercept commands like `CMD_SAY`, `CMD_GIVE`, or `CMD_OPEN` to implement custom behavior. Return TRUE to eat the command and prevent normal command processing, or FALSE to allow normal execution.

Check whether the command is relevant before processing arguments. Parse arguments using standard string manipulation or the `one_argument` helper function. Validate that required objects or targets exist before proceeding.

### Lifecycle Management

Four lifecycle commands enable initialization and cleanup. `CMD_GENERIC_INIT` fires during file parsing before the entity is fully constructed. `CMD_GENERIC_CREATED` fires after `read_mobile` or `read_object` when the entity is fully initialized. `CMD_GENERIC_DESTROYED` fires before deletion and is critical for freeing `act_ptr` memory. `CMD_GENERIC_RESET` fires during zone reset cycles.

Always free `act_ptr` in `CMD_GENERIC_DESTROYED` to prevent memory leaks. Check whether `act_ptr` is non-null before casting and dereferencing. Set `act_ptr` to nullptr after deletion to avoid use-after-free.

### State Machine Implementation

Allocate state structures in `CMD_GENERIC_CREATED` or lazily on first pulse. Store the structure pointer in `act_ptr` and cast back to the concrete type when needed. Define an enum or integer field representing the current state and use switch statements to dispatch behavior.

Common state machine patterns include patrol routes where `cur_path` selects which route to follow and `cur_pos` tracks progress along the route. Quest NPCs track conversation phase and target information. Timed events increment tick counters and transition states when thresholds are reached.

Guard against null `act_ptr` before dereferencing because mobs loaded from rent files may bypass `CMD_GENERIC_CREATED`. Either allocate on first pulse or return early when state is null.

### String Customization with swapToStrung

Call `swapToStrung` on an object or mob before modifying its name, short description, or long description. This copies strings from the prototype to the instance and sets the `ITEM_STRUNG` flag for objects or `ACT_STRINGS_CHANGED` flag for mobs. Without this call, modifications affect the shared prototype and persist across resets.

Check the relevant flag before calling to avoid redundant work. Calling `swapToStrung` on an already-strung entity is a no-op. After stringing, the entity owns its strings and modifications persist through saves.

For objects that need immediate customization after creation, `CMD_GENERIC_CREATED` fires too early because the object is not fully initialized. Use `CMD_GENERIC_QUICK_PULSE` with a flag check to perform deferred initialization on the first pulse.

### DELETE Flag Handling

Always use `IS_SET_DELETE` to check DELETE flags, never `IS_SET`. The DELETE flags use a combined bit pattern that `IS_SET` does not detect correctly. When a spec proc causes an entity to be deleted, return the appropriate flag and let the caller perform deletion.

`DELETE_THIS` signals that the entity hosting the spec proc should be deleted. `DELETE_VICT` signals that the `ch` parameter should be deleted. `DELETE_ITEM` signals that the `obj` or `t2` parameter should be deleted. Multiple flags can be combined using bitwise OR.

Context determines flag semantics. In `CMD_GENERIC_PULSE` or `CMD_MOB_COMBAT`, return `DELETE_THIS` if the mob dies. In `CMD_MOB_VIOLENCE_PEACEFUL`, return `DELETE_VICT` to kill the first being. For generic commands, `DELETE_VICT` means the first being parameter is gone and `DELETE_THIS` means the second being is gone.

### Safe Room Iteration in triggerSpecial

The `triggerSpecial` function in parse.cc builds a snapshot of room contents before iterating because spec procs can modify the room's `stuff` list. It constructs a vector of pointers using post-increment iteration to safely advance before potential modifications. During iteration over the snapshot, it checks whether each thing is still valid and still in the expected room before calling `checkSpec`. Things that were deleted or moved are skipped.

This pattern is necessary whenever iterating containers that may be modified during iteration. Use post-increment to advance before the element is potentially removed. Cache the next pointer before operations that might delete elements in linked lists.

## Reference

### Mob Spec Proc Signature

```cpp
int procName(TBeing* ch, cmdTypeT cmd, const char* arg, TMonster* myself, TObj* obj)
```

Parameter `ch` is the being triggering the proc or nullptr. Parameter `cmd` is the command type from `cmdTypeT` enum. Parameter `arg` contains command arguments or nullptr. Parameter `myself` is the mob instance owning the proc. Parameter `obj` is context-dependent and may be an integer cast to pointer.

### Object Spec Proc Signature

```cpp
int procName(TBeing* ch, cmdTypeT cmd, const char* arg, TObj* myself, TObj* t2)
```

Parameter `ch` is the being triggering the proc. Parameter `cmd` is the command type. Parameter `arg` contains command arguments. Parameter `myself` is the object instance. Parameter `t2` is context-dependent and may be an integer cast to pointer.

### Room Spec Proc Signature

```cpp
int procName(TBeing* ch, cmdTypeT cmd, const char* arg, TRoom* room)
```

Parameter `ch` is the being triggering the proc. Parameter `cmd` is the command type. Parameter `arg` contains command arguments. Parameter `room` is the room instance.

### Command Type Categories

Lifecycle commands are `CMD_GENERIC_INIT`, `CMD_GENERIC_CREATED`, `CMD_GENERIC_DESTROYED`, and `CMD_GENERIC_RESET`. Pulse commands are `CMD_GENERIC_PULSE` and `CMD_GENERIC_QUICK_PULSE`. Player commands have non-negative values corresponding to in-game actions.

Mob combat commands include `CMD_MOB_COMBAT`, `CMD_MOB_COMBAT2`, `CMD_MOB_COMBAT_ONATTACK`, `CMD_MOB_COMBAT_ONATTACKED`, `CMD_MOB_KILLED_NEARBY`, and `CMD_MOB_VIOLENCE_PEACEFUL`.

Object event commands include `CMD_OBJ_HIT`, `CMD_OBJ_HITTING`, `CMD_OBJ_MISS`, `CMD_OBJ_BEEN_HIT`, `CMD_OBJ_GOTTEN`, and `CMD_OBJ_MOVEMENT`.

Room event commands include `CMD_ROOM_ENTERED` and `CMD_ROOM_ATTEMPTED_EXIT`.

### Integer-Cast Parameters

In `CMD_MOB_MOVED_INTO_ROOM`, the `obj` parameter is the old room number cast to `TObj*`. In `CMD_MOB_KILLED_NEARBY`, the `obj` parameter is the victim being cast to `TObj*`. In `CMD_MOB_VIOLENCE_PEACEFUL`, the `obj` parameter is the violence target cast to `TObj*`. In `CMD_OBJ_MOVEMENT`, the `arg` parameter is castable to `dirTypeT`. In `CMD_OBJ_HIT`, the `arg` parameter is castable to `wearSlotT`.

Never dereference these parameters without checking the command type first. Use `reinterpret_cast` to convert back to the integer type.

### Registry Arrays

Mob spec procs are registered in the `mob_specials` array defined in spec_mobs.cc. Each entry contains a function pointer and metadata. Objects use `objSpecials` in spec_objs.cc. Rooms use `roomSpecials` in spec_rooms.cc.

Spec constants are defined in corresponding header files: `SPEC_*` constants in spec_mobs.h, object constants in spec_objs.h, and room constants in spec_rooms.h.

### Return Value Meanings

Return FALSE or zero when no special processing occurred and normal command handling should proceed. Return TRUE or any non-zero value without DELETE flags when the command was handled and normal processing should be suppressed.

Return values can include DELETE flags using bitwise OR. `DELETE_THIS` indicates the entity owning the proc should be deleted. `DELETE_VICT` indicates the `ch` parameter should be deleted. `DELETE_ITEM` indicates the `obj` or `t2` parameter should be deleted.

## Implementation

### Registration System

The registration arrays `mob_specials`, `objSpecials`, and `roomSpecials` map spec constant values to function pointers. When a mob, object, or room is loaded with a spec proc assigned, the corresponding function pointer is looked up and stored in the entity's structure.

The `triggerSpecial` function is the primary entry point for spec proc invocation. It is called from command processing, scheduler pulse handlers, and event systems. It checks task and spell interruption, then invokes spec procs on the room, equipment, inventory, and room contents in that order.

The `checkSpec` function is called per entity to invoke its specific spec proc. It retrieves the function pointer from the entity's structure and calls it with the appropriate parameters. Return values are checked for DELETE flags and handled accordingly.

### Pulse Scheduling

The `Pulse::SPEC_PROCS` scheduler fires every 3.6 seconds and iterates all mobs, objects, and rooms with spec procs, calling them with `CMD_GENERIC_PULSE`. The `Pulse::COMBAT` scheduler fires every 1.2 seconds and calls quick pulse procs with `CMD_GENERIC_QUICK_PULSE`.

Each scheduler maintains iteration safety by caching next pointers before calling spec procs. This handles cases where spec procs delete entities or modify container membership during iteration.

### Safe Iteration Strategy

The triggerSpecial function builds a snapshot vector of room contents before iteration. It uses post-increment iteration `*(it++)` to advance the iterator before pushing the pointer onto the snapshot. After building the snapshot, it iterates the vector and checks each element for validity before calling `checkSpec`.

Validity checks include verifying the pointer is non-null and confirming `in_room` matches the expected room number. Things that fail these checks were deleted or moved and are skipped. This pattern prevents use-after-free when spec procs modify room contents.

### State Persistence

The `act_ptr` field exists on `TMonster`, `TObj`, and `TRoom`. It is a void pointer intended for spec proc use. Spec procs allocate custom structs and store the pointer in `act_ptr` during creation or first pulse. On each pulse or event, the pointer is cast back to the struct type and accessed.

Cleanup in `CMD_GENERIC_DESTROYED` is mandatory. Cast `act_ptr` to the correct type, call delete, and set `act_ptr` to nullptr. Failure to clean up causes memory leaks because the entity destructor does not automatically free `act_ptr`.

### String Customization Mechanics

The `TObj::swapToStrung` function checks `isObjStat(ITEM_STRUNG)` and returns early if already strung. Otherwise it sets the flag and copies `name`, `shortDescr`, and other strings from the prototype in `obj_index`. After this, modifications to the instance do not affect the prototype.

The `TMonster::swapToStrung` function checks the `ACT_STRINGS_CHANGED` flag in `specials.act` and returns if set. Otherwise it sets the flag and copies strings from `mob_index`. The mob then owns its strings independently of the prototype.

Deferred stringing for objects uses `CMD_GENERIC_QUICK_PULSE` because `CMD_GENERIC_CREATED` fires before full initialization. Check `!isObjStat(ITEM_STRUNG)` inside the quick pulse handler to ensure stringing happens exactly once on the first pulse.

## Troubleshooting

### Segmentation Fault on obj or t2 Parameter

Cause: Dereferencing the final parameter without checking command type when the parameter is an integer cast to pointer. Commands like `CMD_MOB_MOVED_INTO_ROOM` and `CMD_OBJ_MOVEMENT` encode integers as pointers.

Solution: Always check `cmd` before dereferencing `obj` or `t2`. Use `reinterpret_cast` to convert back to integer type when needed. Add switch cases for integer-cast commands before general pointer logic.

### Memory Leak on Mob or Object Deletion

Cause: Failing to free `act_ptr` in `CMD_GENERIC_DESTROYED`. The entity destructor does not automatically clean up this field because it does not know the pointed-to type.

Solution: Add a `CMD_GENERIC_DESTROYED` case that casts `act_ptr` to the correct type, calls delete, and sets the field to nullptr. Verify with sanitizers that leaks are resolved.

### Null Pointer Crash on act_ptr Access

Cause: Assuming `act_ptr` is always initialized when it may be null for entities loaded from rent or created outside normal load paths.

Solution: Check `if (!act_ptr)` before casting and dereferencing. Either allocate state lazily on first pulse or return early when state is missing. Guard all state accesses with null checks.

### DELETE Flag Not Detected

Cause: Using `IS_SET` instead of `IS_SET_DELETE` to check for DELETE flags. DELETE flags use a combined bit pattern that `IS_SET` does not recognize.

Solution: Replace all `IS_SET(rc, DELETE_THIS)` with `IS_SET_DELETE(rc, DELETE_THIS)`. Search for incorrect patterns and update them.

### String Modifications Lost on Reset

Cause: Modifying `name`, `shortDescr`, or `longDescr` without calling `swapToStrung` first. Without stringing, modifications affect the shared prototype and are lost on zone reset.

Solution: Always call `swapToStrung` before modifying strings. Check the strung flag to avoid redundant calls. For objects needing immediate customization, use `CMD_GENERIC_QUICK_PULSE` for deferred initialization.

### Spec Proc Causes Use-After-Free in Room Contents

Cause: Iterating room contents directly while a spec proc modifies the list by moving or deleting entities. Direct iteration invalidates iterators when the container is modified.

Solution: Build a snapshot vector before iteration using post-increment pattern. Check validity of each element before calling `checkSpec`. Skip entities that were deleted or moved to other rooms.

### Command Not Suppressed Despite Returning TRUE

Cause: Returning TRUE from spec proc but command processing continues because the return value was not propagated correctly or the spec proc was not registered.

Solution: Verify spec proc is registered in the appropriate array. Confirm the entity has the correct spec constant assigned. Check that all call sites propagate the return value correctly and respect TRUE as command-handled.

### Performance Degradation from Quick Pulse

Cause: Heavy processing in `CMD_GENERIC_QUICK_PULSE` handlers. Quick pulse fires every 1.2 seconds on all entities, so expensive operations multiply across many procs.

Solution: Move non-critical work to `CMD_GENERIC_PULSE` which fires every 3.6 seconds. Profile to identify expensive operations. Cache results and avoid repeated calculations. Limit quick pulse to truly time-critical logic.
