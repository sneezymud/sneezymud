---
title: Spatial Relationships
description: Bidirectional pointer system tracking containment, equipment, riding, and room membership relationships.
category: critical
keywords: [containment, riding, equipment, bidirectional pointers, iterator safety]
primary_symbols:
  functions: [operator+=, operator--, equipChar, unequip, stickIn, pulloutObj, mount, dismount, horseMaster, roomOfObject]
  classes: [TThing, TRoom, TBeing, TObj, TTable]
  enums: [Room::NOWHERE, Room::POLY_STORAGE, wearSlotT, StuffIter]
---

# Spatial Relationships

## Overview

Where is this object? Who is holding it? What container is it inside? What room is that container in? SneezyMUD answers these questions through a system of bidirectional pointers that track relationships between all game entities.

Every entity in the game world participates in a spatial hierarchy. Rooms contain beings and objects. Beings carry inventory, wear equipment, and may have projectiles embedded in their flesh. Objects may contain other objects. Mounts carry riders. Each relationship is tracked through paired pointers that must remain consistent.

The system solves several problems simultaneously: finding all objects in a room for display, determining what a player can interact with, propagating effects through containment chains, and ensuring objects are properly cleaned up when their containers are destroyed.

The core insight is that every entity tracks both its container and what it contains. When you move an object, you update both ends of the relationship. When you query an object's location, you may need to traverse up through multiple containers to find the actual room.

Consider picking up an object from the ground: the object leaves the room's contents list and enters your inventory. The object's room pointer becomes null while its parent pointer now references you. If you then put the object in a bag, it leaves your inventory and enters the bag's contents. Now finding which room the object is "really" in requires traversing from object to bag to you to room.

A TThing exists in multiple relationship hierarchies simultaneously: room location, containment, equipment, stuck-in, and riding. These relationships are mutually exclusive within categories (an object cannot be both in a room AND in inventory) but coexist across categories (a being can simultaneously have inventory, wear equipment, have stuck-in objects, and be riding a mount).

## Patterns

### Adding and Removing Things

Always use the overloaded operators for adding and removing things from locations. Call `*container += *thing` to add and `--(*thing)` to remove. These operators maintain pointer consistency and handle side effects like lighting updates.

Never set location pointers directly. The operators handle updating both sides of the bidirectional relationship. Direct pointer assignment will desynchronize the pointers and cause crashes.

Always ensure the thing has no existing location before adding. The pre-conditions for `operator+=` require that `parent`, `equippedBy`, `stuckIn`, and `roomp` are all null. Violating this causes assertion failures.

Always use `--(*thing)` before adding to a new location. The remove operator clears all location pointers so the thing is ready to be placed elsewhere.

### Finding the Real Room

Never assume `roomp` gives you the room when an object might be inside a container. Objects in containers have null `roomp`. Call `roomOfObject()` to traverse up the containment chain and find the actual room.

### Equipment vs Inventory

Understand that equipped items are not in the `stuff` list. Equipment uses a separate array indexed by wear slot. When iterating a being's inventory, you are not seeing worn items. Use `equipment[]` to access worn items and `stuff` to access carried inventory.

Always use `equipChar()` and `unequip()` for equipment operations. These functions maintain the bidirectional pointers between the being's equipment slots and the object's `equippedBy` pointer.

### Stuck-In Items

Always use `stickIn()` and `pulloutObj()` for embedded objects. Projectiles stuck in body parts use their own pointer system distinct from equipment. Each body slot can have one stuck object.

Remember that `pulloutObj()` causes damage. Removing an embedded arrow is a violent act in game terms.

### Mount and Rider Chains

Understand that mounts support multiple riders via a linked list. The first rider is in `rider`, subsequent riders chain through `nextRider`. Use `horseMaster()` to find the controlling rider (last in chain).

Always use `mount()` and `dismount()` for riding operations. These maintain the bidirectional pointers and handle the linked list correctly.

### Bidirectional Consistency

Never create one-way pointers. Every spatial relationship has a forward and back pointer that must match. If `thing->parent` points to a container, that container's `stuff` list must contain the thing. Inconsistent pointers trigger `mud_assert()` failures.

Validate your mental model: for every pointer type, know its inverse. Room membership pairs `thing->roomp` with `room->stuff`. Containment pairs `thing->parent` with `container->stuff`. Equipment pairs `being->equipment[]` with `obj->equippedBy`. Riding pairs `rider->riding` with `mount->rider`.

### Iterator Safety During Modification

Never modify a stuff list during iteration without advancing the iterator first. When iterating through a container's contents and potentially deleting items, cache the next pointer before operations.

```cpp
for (StuffIter it = stuff.begin(); it != stuff.end(); ) {
  TThing* t = *(it++);  // Post-increment: advance BEFORE using
  // now safe to delete t or modify list
}
```

### Deletion Safety

Never delete a thing while it still has location pointers set. Before deleting, call `--(*thing)` to remove from room/container/equipment. Otherwise you leave dangling pointers.

```cpp
--(*object);    // Remove from room/container/equipment
delete object;  // Now safe
```

### Tables Are Special

Understand that TTable overrides `operator+=` to mount objects on the table surface rather than placing them inside. Tables are visible furniture where you see contents without opening, unlike bags which hide contents until opened.

## Reference

### Symbol Quick Reference

| Symbol | Type | Purpose |
|--------|------|---------|
| `parent` | pointer | Container/room/being holding this thing |
| `roomp` | pointer | Direct room pointer (null if inside container) |
| `in_room` | int | Room number (Room::NOWHERE if not directly in room) |
| `equippedBy` | pointer | Being wearing this item |
| `eq_pos` | wearSlotT | Equipment slot position |
| `stuckIn` | pointer | Being this item is embedded in |
| `eq_stuck` | wearSlotT | Body part item is stuck in |
| `rider` | pointer | First thing riding on this |
| `riding` | pointer | Thing this is riding on |
| `nextRider` | pointer | Next rider in linked list |
| `stuff` | list | Things contained inside this thing |
| `operator+=` | operator | Add thing to container |
| `operator--` | operator | Remove thing from current location |
| `equipChar()` | function | Equip item to wear slot |
| `unequip()` | function | Remove item from wear slot |
| `stickIn()` | function | Embed object in body part |
| `pulloutObj()` | function | Remove embedded object (causes damage) |
| `getStuckIn()` | function | Query what's stuck in a limb |
| `mount()` | function | Start riding target |
| `dismount()` | function | Stop riding |
| `horseMaster()` | function | Get controlling rider |
| `roomOfObject()` | function | Find actual room through containment chain |
| `TThing` | class | Base class with all spatial relationship pointers |
| `TRoom` | class | Location container, handles in_room and zone tracking |
| `TBeing` | class | Characters with equipment, stuck-in, riding capabilities |
| `TObj` | class | Objects that can be contained, equipped, stuck |
| `TTable` | class | Special container with mounting instead of containment |

### Location Pointer States

| Situation | parent | roomp | in_room |
|-----------|--------|-------|---------|
| In room directly | null | the room | room number |
| In inventory | the being | null | NOWHERE |
| In container | the container | null | NOWHERE |

### Operator+= Behavior by Class

| Class | Behavior |
|-------|----------|
| TThing | Validates invariants, handles merging for TMergeable |
| TRoom | Sets in_room, roomp, updates lighting, marks zone active |
| TBeing | Adds to stuff list, sets parent |
| TObj | Adds to stuff list, sets parent |
| TTable | Mounts object on table surface instead of containment |

### Bidirectional Pointer Pairs

| Relationship | Forward Pointer | Back Pointer | Notes |
|--------------|-----------------|--------------|-------|
| In room | thing->roomp | room->stuff list | roomp only set when directly in room |
| In container | thing->parent | container->stuff list | roomp is NULL |
| In inventory | thing->parent | being->stuff list | roomp is NULL |
| Equipped | being->equipment[] | obj->equippedBy | NOT in stuff list |
| Stuck in | being->body_parts[] | obj->stuckIn | One per body slot |
| Riding | rider->riding | mount->rider chain | Multiple riders via nextRider |

### Key Files

| File | Contents |
|------|----------|
| thing.h | TThing pointer member declarations |
| structs.cc | operator+= and operator-- implementations |
| riding.cc | mount/dismount implementations |
| range.cc | stickIn implementation |
| limbs.cc | body_parts accessors |
| handler.cc | thing_to_room, equipChar, unequip, pulloutObj |

## Implementation

### Core Pointer Architecture

The TThing class defines all spatial relationship pointers. The `parent` pointer references whatever directly contains this thing, whether that's a room, being, or container object. The `roomp` pointer provides a shortcut to the room but is null when the thing is inside a container. The `in_room` integer stores the room number for things directly in rooms but is set to Room::NOWHERE for contained items.

Equipment uses a parallel system. The `equippedBy` pointer references the being wearing an item, with `eq_pos` indicating which wear slot. The being maintains an `equipment[]` array indexed by wearSlotT for the reverse lookup.

Stuck-in items use another parallel system. The `stuckIn` pointer references the being the object is embedded in, with `eq_stuck` indicating which body part. The being's `body_parts[]` array maintains the reverse lookup.

Mount relationships use a linked list for multiple riders. The mount's `rider` pointer references the first rider. Each rider's `riding` pointer references the mount. Subsequent riders chain through `nextRider`.

### Containment Hierarchy

The hierarchy flows from rooms down through containers. A room directly contains beings and objects in its `stuff` list. Each being has its own `stuff` list for inventory, an `equipment[]` array for worn items, and `body_parts[]` for stuck objects. Container objects have a `stuff` list for their contents. Mounts have a `rider` chain for beings riding them.

When querying where something is, `roomp` gives the immediate answer for things directly in rooms. For things inside containers, you must traverse up through parent pointers using `roomOfObject()` until you reach something with a valid `roomp`.

### Adding Things

The `operator+=` method varies by container class. All implementations validate that the thing has no existing location (parent, equippedBy, stuckIn, and roomp must be null).

TThing's base implementation runs first, performing precondition checks. It also checks if the thing is TMergeable (stackable items like arrows or coins). If so, it attempts to merge with existing stuff in the container. Merging combines quantities and deletes the incoming thing, so the operator returns early.

TRoom's implementation sets the thing's `in_room` to the room number, sets `roomp` to the room, adds the thing to the room's `stuff` list, and handles side effects like updating room lighting via `addToLight()` and marking the zone active if a player entered.

TBeing and TObj implementations add the thing to the `stuff` list and set the thing's `parent` pointer. TMergeable objects may be combined with existing identical objects rather than added separately.

TTable overrides containment behavior to mount objects on the table surface rather than placing them inside.

### Removing Things

The `operator--` method removes a thing from its current location. It clears all location pointers: parent, roomp, in_room, equippedBy, and stuckIn are set to null or NOWHERE. The thing is removed from whatever list it was in (the container's stuff list, the being's equipment array, etc.).

After removal, the thing is locationless and ready to be placed in a new location via `operator+=`.

### Equipment Operations

The `equipChar()` function equips an object to a wear slot. It sets the object's `equippedBy` to the being, sets `eq_pos` to the slot, and stores the object in the being's `equipment[]` array at that slot. Equipped items are not in the being's `stuff` list.

It calls `affectModify()` to apply item affects. Equipped items can grant stat bonuses, resistances, or apply conditions. Each object has MAX_OBJ_AFFECT slots that get applied when equipped.

The `unequip()` function reverses this, clearing the object's equipment pointers and removing it from the array. It calls `affectModify()` to remove affects. The object is then locationless until placed somewhere.

### Stuck-In Operations

The `stickIn()` function embeds an object in a body part. It sets the object's `stuckIn` to the being and `eq_stuck` to the limb slot. The being's body_parts array tracks what's stuck in each limb.

The `pulloutObj()` function removes the embedded object, which causes damage to the being as the projectile is extracted. The `getStuckIn()` function queries what object is embedded in a particular limb without removing it.

### Mount Operations

The `mount()` function establishes a riding relationship. The rider's `riding` pointer is set to the mount. The rider is added to the mount's rider chain via the `rider` and `nextRider` pointers.

The `dismount()` function removes the rider from the chain and clears the riding pointer. The `horseMaster()` function traverses to the end of the rider chain to find the controlling rider.

### Zone Activity Tracking

When a player enters a room, `operator+=` marks the room's zone as active. This affects reset scheduling and performance optimization. When the last player leaves, the zone may become inactive.

### Invariant Enforcement

The system uses `mud_assert()` to enforce pointer consistency. Before adding a thing, assertions verify no existing location. After adding, assertions verify bidirectional consistency. Violations indicate serious bugs that would cause crashes or corruption if allowed to continue.

Common assertions enforced:
- operator+= requires NULL location pointers before adding
- If thing->parent is set, thing must be in parent->stuff
- If thing->equippedBy is set, being->equipment[thing->eq_pos] must point to thing
- If thing->stuckIn is set, being->body_parts[thing->eq_stuck].stuckIn must point to thing
- If thing->riding is set, thing must be in riding->rider chain

## Troubleshooting

### Crash on Object Addition

**Symptom:** Assertion failure or crash when calling `operator+=`.

**Likely cause:** The object has an existing location. One or more of parent, equippedBy, stuckIn, or roomp is non-null.

**Diagnostic approach:** Check the object's location pointers before the add operation. Trace back to find where the object was last removed and verify `operator--` was called.

**Fix:** Call `--(*object)` before adding to the new location to clear all location pointers.

### Crash When Iterating Container Contents

**Symptom:** Crash or corruption during iteration over a stuff list.

**Likely cause:** Modifying the stuff list during iteration without advancing the iterator first.

**Diagnostic approach:** Look for code that deletes things or removes things from containers inside a loop over the stuff list. Check if the iterator is post-incremented before the modification.

**Fix:** Use the `*(it++)` pattern: dereference to get the current thing, post-increment to advance, then operate on the thing.

### Object Not Found in Room

**Symptom:** Searching a room's stuff list doesn't find an object that should be there.

**Likely cause:** The object is inside a container in the room, not directly in the room. Its `roomp` is null.

**Diagnostic approach:** Check if the object's `parent` points to a container. Trace up through parents to find the actual room.

**Fix:** Use `roomOfObject()` to find the room through containment chains, or search recursively through container contents.

### Equipment Not in Inventory

**Symptom:** Iterating a being's `stuff` list doesn't include worn items.

**Likely cause:** This is correct behavior. Equipment is in the `equipment[]` array, not the stuff list.

**Diagnostic approach:** Check the being's equipment array for the item.

**Fix:** When you need all items a being has, check both `stuff` for inventory and `equipment[]` for worn items.

### Equipment Item Appears in Inventory List

**Symptom:** An equipped item is also found in the stuff list.

**Likely cause:** Manual pointer assignment instead of using `equipChar()`. The item's parent was set but equippedBy was not, or it was added to stuff list instead of equipment array.

**Diagnostic approach:** Check if equippedBy and eq_pos are set correctly. Check if the item is in both equipment[] and stuff list (should never happen).

**Fix:** Use `equipChar()` and `unequip()` exclusively for equipment management. Never manually assign equipment pointers.

### Pointer Consistency Violation

**Symptom:** `mud_assert()` failure about inconsistent pointers.

**Likely cause:** Direct pointer manipulation bypassed the operators, leaving forward and back pointers mismatched.

**Diagnostic approach:** Check both ends of the relationship. If thing->parent points to a container, is the thing in that container's stuff list? Trace recent operations on the object.

**Fix:** Always use the operators for spatial changes. Audit recent code changes for direct pointer assignments.

### Rider Chain Corruption After Dismount

**Symptom:** NULL pointers in the middle of rider chain, cycles, or dangling pointers after dismount.

**Likely cause:** Manual modification of riding, rider, or nextRider pointers instead of using `dismount()`.

**Diagnostic approach:** Walk the rider chain and check for NULL pointers in the middle, cycles, or dangling pointers. Check if a rider's `riding` pointer doesn't match the mount's `rider` chain.

**Fix:** Use `mount()` and `dismount()` exclusively. Never manually modify riding relationship pointers.

### Dangling Pointer Crash in Room's stuff List

**Symptom:** Crash accessing freed memory when iterating room contents.

**Likely cause:** Deleting a thing without removing it from the room first.

**Diagnostic approach:** Check if `operator--` was called before `delete`. Look for direct `delete thing` calls without prior removal from location.

**Fix:** Always `--(*thing)` before `delete thing`.

### Light Level Wrong After Adding/Removing Object

**Symptom:** Room lighting doesn't update when light-emitting objects are moved.

**Likely cause:** `updateLight()` not being called, or being called on the wrong room.

**Diagnostic approach:** Check if the object emits light. Trace whether the room's updateLight() was invoked when the object was added or removed.

**Fix:** Ensure you're adding/removing through `operator+=` and `operator--`, which handle updateLight() automatically.

### Zone Stays Active After Last Player Leaves

**Symptom:** Zone activity flag remains set when no players are present.

**Likely cause:** Player was removed from room without going through the proper `operator--` path.

**Diagnostic approach:** Check zone's active flag and count how many players are actually in the zone's rooms.

**Fix:** Ensure all player movement goes through `operator--` and `operator+=`. The operators handle zone activity tracking automatically.
