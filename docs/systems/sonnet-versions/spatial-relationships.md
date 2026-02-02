---
title: TThing Spatial Relationship System
category: critical
keywords: [spatial, containment, parent, roomp, equipment, stuckIn, riding, operator+=, bidirectional]
related: [thing-system.md, room-system.md, equipment-system.md, riding-system.md]
primary_symbols:
  functions: [operator+=, operator--, equipChar, unequip, stickIn, pulloutObj, mount, dismount, roomOfObject, horseMaster]
  classes: [TThing, TRoom, TBeing, TObj, TTable]
  files: [code/code/misc/thing.h, code/code/misc/structs.cc, code/code/misc/riding.cc, code/code/misc/range.cc, code/code/misc/limbs.cc, code/code/sys/handler.cc]
---

## Overview

How does the game know where a sword is? When a player picks up a shield, what pointers change? When an arrow gets stuck in someone's leg, how does the system track that?

SneezyMUD maintains a web of spatial relationships that define where every object, character, and mount exists in the game world. This system tracks not just simple containment ("sword is in backpack") but complex simultaneous relationships: a mounted knight in a room carrying inventory, wearing armor, with an arrow stuck in their shoulder.

The system is built on **bidirectional pointers**. Every relationship has both a forward and back pointer that must stay synchronized. When you add an object to a room, the object gains a pointer to the room AND the room adds the object to its list. Break this synchronization and you get crashes from dangling pointers.

A `TThing` is simultaneously located in multiple relationship hierarchies:
- **Room location**: What room is this thing in?
- **Containment**: What container (if any) holds this thing, and what things does this thing contain?
- **Equipment**: If worn, who's wearing it and in which slot?
- **Stuck-in**: If embedded, whose body part is it stuck in?
- **Riding**: If mounted, what are you riding and who's riding you?

These relationships are mutually exclusive in specific combinations. An object cannot be both in a room AND in someone's inventory. An equipped sword is not in the inventory list. But a being can simultaneously be in a room, have inventory, wear equipment, have things stuck in body parts, and be riding a mount.

The core pattern is that location pointers are **mutually exclusive states**, while relationship types are **simultaneous capabilities**. You navigate this by understanding which pointers must be NULL before setting others, and which must be cleared when changing state.

The system enforces preconditions through assertions. Add something to a room when it already has a parent pointer set? Crash. Remove an item from a container without clearing all its location pointers? Crash. These crashes are intentional - they expose bugs before they corrupt game state.

## Patterns

### Always Clear Location Before Moving

Before adding a thing to any new location, its location pointers must be NULL. Use `operator--` to clear all location state before `operator+=` to add to new location.

```cpp
--(*object);        // Clear all location pointers
*room += *object;   // Now safe to add to room
```

Why: `operator+=` validates that parent, equippedBy, stuckIn, and roomp are all NULL. If any are set, you get a failed assertion and crash.

### Never Modify stuff List During Iteration

When iterating through a container's `stuff` list, cache the next pointer BEFORE operations that might delete the current item.

```cpp
for (StuffIter it = stuff.begin(); it != stuff.end(); ) {
  TThing* t = *(it++);  // Post-increment: advance BEFORE using
  // now safe to delete t or modify list
}
```

Why: Deletion invalidates iterators. The post-increment pattern advances before the current element potentially gets removed.

### Always Maintain Bidirectional Consistency

Every spatial relationship has both forward and back pointers. When you set one, the system must set the other. Never manually set just one side.

Use the operators and system functions - don't manually assign pointers:
- Use `operator+=` and `operator--`, not direct parent assignment
- Use `equipChar()` and `unequip()`, not direct equippedBy assignment
- Use `stickIn()` and `pulloutObj()`, not direct stuckIn assignment
- Use `mount()` and `dismount()`, not direct riding assignment

Why: Manual pointer assignment breaks bidirectional consistency. The room won't know it contains an object if you only set the object's roomp pointer without adding to the room's stuff list.

### Equipment Is Not In Inventory

Equipped items are stored in `TBeing::equipment[]` array, not in the `stuff` list. When you equip an item, it leaves inventory entirely.

```cpp
// Item in inventory: parent = being, in being->stuff list
being->equipChar(sword, WEAR_HAND_R);
// Item now equipped: equippedBy = being, in being->equipment[], NOT in stuff
```

Why: The stuff list is for carried inventory. Equipment slots are a separate namespace to avoid ambiguity about whether an item is worn or carried.

### Use roomOfObject For Nested Containers

`TThing::roomp` is NULL when a thing is inside a container. To find the actual room, use `roomOfObject()` which walks up the parent chain.

```cpp
if (obj->roomp) {
  // Object is directly in a room
} else if (TRoom* room = obj->roomOfObject()) {
  // Object is in container(s), but ultimately in a room
} else {
  // Object is in storage/nowhere
}
```

Why: roomp is only set when directly in a room. Nested containers (backpack inside chest inside room) have NULL roomp but are findable via parent chain traversal.

### Riding Is A Linked List

Multiple riders are chained through `nextRider` pointers. The mount's `rider` pointer is the head of the list.

```cpp
mount->rider = firstRider;
firstRider->nextRider = secondRider;
secondRider->nextRider = NULL;
```

The controlling rider (who commands the mount) is the LAST rider in the chain, found via `horseMaster()`.

Why: Multiple riders share a mount but only one controls it. The linked list allows any number of riders.

### Never Delete With Location Pointers Set

Before deleting a thing, clear all its location pointers via `operator--`. Otherwise you leave dangling pointers in containers, rooms, or beings.

```cpp
--(*object);    // Remove from room/container/equipment
delete object;  // Now safe
```

Why: If you delete something still in a room's stuff list, the room has a dangling pointer. Next time it iterates stuff, crash.

### Tables Are Special Containers

`TTable` overrides `operator+=` to mount objects on the table rather than contain them. Tables don't contain things the way bags do - they're furniture that things rest upon.

Why: A table is visible furniture, not a closed container. The mounting behavior gives different semantics (you see things on tables without opening them).

## Reference

### Primary Symbols

| Symbol | Type | Purpose |
|--------|------|---------|
| `operator+=` | function | Add thing to room/container, sets location pointers |
| `operator--` | function | Remove thing from current location, clears all location pointers |
| `equipChar()` | function | Equip item to slot, sets equippedBy and equipment[] |
| `unequip()` | function | Unequip item from slot, clears equipment pointers |
| `stickIn()` | function | Embed object in body part, sets stuckIn |
| `pulloutObj()` | function | Remove stuck object (causes damage) |
| `mount()` | function | Start riding a mount |
| `dismount()` | function | Stop riding, remove from rider chain |
| `roomOfObject()` | function | Walk parent chain to find ultimate room |
| `horseMaster()` | function | Find controlling rider (last in chain) |
| `TThing` | class | Base class with all spatial relationship pointers |
| `TRoom` | class | Location container, handles in_room and zone tracking |
| `TBeing` | class | Characters with equipment, stuck-in, riding capabilities |
| `TObj` | class | Objects that can be contained, equipped, stuck |
| `TTable` | class | Special container with mounting instead of containment |

### Location Pointer States

| Pointer | Meaning When Set | Meaning When NULL |
|---------|------------------|-------------------|
| `parent` | Inside a container/being | Not contained |
| `roomp` | Directly in a room | In container or nowhere |
| `in_room` | Room vnum | Room::NOWHERE if not in room |
| `equippedBy` | Being wearing this item | Not equipped |
| `eq_pos` | Equipment slot (wearSlotT) | No slot |
| `stuckIn` | Being this is stuck inside | Not stuck |
| `eq_stuck` | Body part stuck in | No limb |
| `riding` | What this is riding | Not riding |
| `rider` | First rider (linked list head) | Nothing riding this |
| `nextRider` | Next in rider chain | Last/only rider |

### Bidirectional Relationships

| Relationship | Forward Pointer | Back Pointer | Notes |
|--------------|-----------------|--------------|-------|
| In room | thing->roomp | room->stuff list | roomp only set when directly in room |
| In container | thing->parent | container->stuff list | roomp is NULL |
| In inventory | thing->parent | being->stuff list | roomp is NULL |
| Equipped | being->equipment[slot] | obj->equippedBy | NOT in stuff list |
| Stuck in limb | being->body_parts[slot].stuckIn | obj->stuckIn | One per body slot |
| Riding | rider->riding | mount->rider chain | Multiple riders via nextRider |

### Key Files

| File | Responsibility |
|------|---------------|
| `code/code/misc/thing.h` | TThing pointer member declarations |
| `code/code/misc/structs.cc` | operator+= and operator-- implementations |
| `code/code/misc/riding.cc` | mount/dismount logic, rider chain management |
| `code/code/misc/range.cc` | stickIn implementation for embedded objects |
| `code/code/misc/limbs.cc` | body_parts accessors, limb tracking |
| `code/code/sys/handler.cc` | thing_to_room, equipChar, unequip, pulloutObj |

## Implementation

### Location Pointer Members

`TThing` maintains multiple location pointers that represent different ways of being spatially related to other entities. These pointers are mutually exclusive in specific combinations.

**parent**: Points to the container (TObj) or being (TBeing) that holds this thing. NULL when directly in a room. Used to navigate the containment hierarchy when searching for the ultimate room location.

**roomp**: Direct pointer to the TRoom this thing occupies. Only set when the thing is directly in a room, not when nested in containers. This is a performance shortcut - it avoids walking the parent chain for common room queries.

**in_room**: The room vnum as an integer. Set to Room::NOWHERE when not in a room. Mirrors roomp but as an index rather than pointer. Used for persistence and fast vnum-based lookups.

**equippedBy**: Points to the TBeing wearing this item. NULL when not equipped. Equipment exists in a separate namespace from inventory - equipped items are NOT in the being's stuff list, they're in the equipment[] array.

**eq_pos**: The wearSlotT enum value indicating which equipment slot this occupies (WEAR_HAND_R, WEAR_BODY, etc.). Only meaningful when equippedBy is set.

**stuckIn**: Points to the TBeing this object is embedded in (arrows, spears). Separate from equipment - stuck items are lodged in flesh, not worn. Removal causes damage.

**eq_stuck**: The wearSlotT value for which body part this is stuck in. One stuck object per body slot maximum. Use getStuckIn(slot) to query what's stuck in a limb.

**riding**: Points to the thing (usually TBeing) this is mounted on. NULL when not riding. Riders share their mount's room location.

**rider**: Head of the linked list of things riding this thing. NULL when nothing is riding. Multiple riders are chained via nextRider pointers.

**nextRider**: Next rider in the linked list. NULL for the last rider. The last rider is the controlling rider (horseMaster) who commands the mount.

**stuff**: List of things contained by this thing. For TRoom, it's everything in the room. For TBeing, it's inventory only (not equipment). For TObj containers, it's the contents.

### Adding Things With operator+=

When you invoke `*container += *thing`, you trigger a chain of validation and pointer updates that establish bidirectional consistency.

`TThing::operator+=` runs first, performing precondition checks. It asserts that parent, equippedBy, stuckIn, and roomp are all NULL. If any are set, the precondition fails and the game crashes with an assertion error. This enforces that you must clear location before moving.

After validation, it checks if the thing is TMergeable (stackable items like arrows or coins). If so, it attempts to merge with existing stuff in the container. Merging combines quantities and deletes the incoming thing, so the operator returns early.

If not mergeable or merge fails, control passes to the specialized override in TRoom, TBeing, TObj, or TTable.

**TRoom::operator+=** sets in_room to the room's vnum and roomp to the room pointer. It appends the thing to the room's stuff list (bidirectional consistency). It calls updateLight() to recalculate lighting from light-emitting objects. It marks the zone active if a player entered.

**TBeing::operator+=** sets the thing's parent to the being and appends to the being's stuff list. This is for inventory. Equipment uses a different path (equipChar).

**TObj::operator+=** sets the thing's parent to the container object and appends to the container's stuff list. This is for bag/chest/container contents.

**TTable::operator+=** has special behavior. Instead of containment semantics, it "mounts" the thing on the table. Tables are furniture - things rest upon them rather than being hidden inside. The visual difference is that you see table contents without opening, but bag contents require opening.

### Removing Things With operator--

`operator--` is the inverse of `operator+=`. When you invoke `--(* thing)`, it clears all location pointers and removes the thing from its current location's stuff list.

The implementation checks which location state is active (parent, roomp, equippedBy, stuckIn) and calls the appropriate removal logic. It clears parent, roomp, in_room, equippedBy, stuckIn, eq_pos, eq_stuck, and removes the thing from the container's stuff list.

This is the mandatory precondition before adding to a new location. The pattern is always:
```
--(*thing);        // Clear old location
*newPlace += *thing;  // Set new location
```

Forgetting the removal step leaves old pointers set, which causes the `operator+=` precondition to fail.

### Equipment System Flow

Equipment exists in a separate namespace from inventory. The being maintains an `equipment[]` array indexed by wearSlotT values (WEAR_HAND_R, WEAR_BODY, WEAR_HEAD, etc.).

When `equipChar(obj, slot)` is called, it validates that the slot is empty and the object is compatible with that slot. It sets `obj->equippedBy = this` and `obj->eq_pos = slot`. It assigns `equipment[slot] = obj`. The object is NOT in the stuff list - it has moved from inventory to equipment namespace.

It calls `affectModify()` to apply item affects. Equipped items can grant stat bonuses, resistances, or apply conditions. Each object has MAX_OBJ_AFFECT slots that get applied when equipped.

`unequip(slot)` reverses this. It clears `equipment[slot]`, `obj->equippedBy`, and `obj->eq_pos`. It calls `affectModify()` to remove affects. The object has no location pointers set at this point - you typically add it to inventory immediately after:
```
obj = being->unequip(slot);
*being += *obj;  // Back to inventory
```

### Stuck-In System Flow

The stuck-in system handles embedded objects like arrows, spears, or daggers lodged in body parts. Each body part slot (limb) can have one stuck object.

`stickIn(obj, slot)` validates that the slot exists and is not already occupied. It sets `obj->stuckIn = this`, `obj->eq_stuck = slot`, and `body_parts[slot].stuckIn = obj`. This is bidirectional - the object knows what it's stuck in, and the limb knows what's stuck in it.

Stuck objects remain in this state until pulled out. They're not in inventory, not equipped, and not in the room - they're in a third state representing physical embedding.

`pulloutObj(slot)` removes the stuck object and applies damage to the being. Ripping an arrow out of flesh hurts. It clears `body_parts[slot].stuckIn`, `obj->stuckIn`, and `obj->eq_stuck`. The object now has no location pointers - you must decide what to do with it (drop to room, add to inventory, etc.).

`getStuckIn(slot)` is a query function that returns the object stuck in a given limb, or NULL if nothing is stuck there.

### Mount and Rider Chain Management

The riding system uses a linked list to track multiple riders on a single mount. The mount's `rider` pointer is the head of the list. Each rider has a `riding` pointer to the mount and a `nextRider` pointer to the next rider in the chain.

When `mount(target)` is called, it validates that you're not already riding and the target can be ridden. It adds you to the target's rider chain by setting `riding = target` and linking you into the nextRider chain. The insertion logic traverses to find the end of the chain and appends.

The rider shares the mount's room location. When the mount moves, riders move with it. The rider's roomp and in_room mirror the mount's.

`dismount(position)` removes you from the rider chain. It walks the chain to find your link and splices you out, updating the previous rider's nextRider or the mount's rider pointer if you're first. It clears your `riding` pointer.

`horseMaster()` finds the controlling rider by walking the nextRider chain to the last element. The last rider in the chain is the one who commands the mount. This allows a hierarchy where the primary rider controls movement but additional passengers come along for the ride.

### Room Location And Parent Chain Traversal

Direct room location is straightforward: `thing->roomp` points to the room and `thing->in_room` holds the vnum. Both are set when the thing is directly in a room's stuff list.

Nested containment is more complex. If a sword is in a backpack, and the backpack is in a chest, and the chest is in a room, the sword's roomp is NULL. Its parent points to the backpack. The backpack's parent points to the chest. The chest's parent is NULL but its roomp points to the room.

`roomOfObject()` walks this chain. It starts with the thing and follows parent pointers until it finds something with a non-NULL roomp, then returns that room. If the chain ends without finding a room (thing is in storage or a player's inventory in a void), it returns NULL.

This is why code checks both `thing->roomp` (direct in room) and `thing->roomOfObject()` (possibly nested). The roomp shortcut is fast, but nested items require traversal.

### Simultaneous Relationship States

A TBeing in the game world exists in multiple relationship states simultaneously. It is in a room (roomp, in_room). It has inventory (stuff list). It wears equipment (equipment[] array). It may have objects stuck in limbs (body_parts[].stuckIn). It may be riding a mount (riding pointer). It may have riders (rider chain).

These are not mutually exclusive. A mounted knight in a room carrying a backpack wearing armor with an arrow stuck in their shoulder is a normal state. The system maintains separate pointer namespaces for each relationship type.

The mutual exclusion is within categories, not across them. An object cannot be both in a room AND in inventory (parent and roomp cannot both be set). An item cannot be both equipped AND in inventory (equippedBy and parent cannot both point to the same being). But a being can simultaneously have inventory, equipment, stuck-in objects, and be riding.

### Lighting And Zone Activity

When an object is added to a room, the room's lighting state may change. Light-emitting objects (torches, magical lights) contribute to room brightness. `TRoom::operator+=` calls `updateLight()` to recalculate lighting from all objects in the room.

Zone activity tracking marks zones as "active" when players are present. This affects reset scheduling and performance optimization. When a player enters a room, `operator+=` marks the room's zone as active. When the last player leaves, the zone may become inactive.

### Assertions And Invariant Enforcement

The system aggressively uses `mud_assert()` to enforce invariants. These are not optional checks - they're contract enforcement that crashes the game if violated.

Common assertions:
- operator+= requires NULL location pointers before adding
- Bidirectional consistency: if thing->parent is set, thing must be in parent->stuff
- Equipment: if thing->equippedBy is set, being->equipment[thing->eq_pos] must point to thing
- Stuck-in: if thing->stuckIn is set, being->body_parts[thing->eq_stuck].stuckIn must point to thing
- Riding: if thing->riding is set, thing must be in riding->rider chain

When these assertions fail during development, they expose bugs immediately rather than allowing corrupted state to propagate. The crash is intentional - it forces the developer to fix the bidirectional consistency violation.

## Troubleshooting

### Symptom: Assertion Failure In operator+=

**Likely cause:** Attempting to add a thing to a new location without clearing its old location first.

**Diagnostic approach:** Check the assertion message. It typically reports which pointer was non-NULL (parent, equippedBy, stuckIn, or roomp). Trace back to see where the thing came from - was it removed from its previous location with `operator--`?

**Fix:** Always invoke `--(*thing)` before moving to a new location. The pattern is remove-then-add, never add-directly.

### Symptom: Crash When Iterating Container Contents

**Likely cause:** Modifying the stuff list during iteration without advancing the iterator first.

**Diagnostic approach:** Look for code that deletes things or removes things from containers inside a loop over the stuff list. Check if the iterator is post-incremented before the modification.

**Fix:** Use the `*(it++)` pattern: dereference to get the current thing, post-increment to advance, then operate on the thing. This ensures the iterator is valid before the current element is potentially removed.

### Symptom: Thing Has NULL roomp But Should Be In Room

**Likely cause:** The thing is inside a container, not directly in the room. roomp is only set for direct room occupancy.

**Diagnostic approach:** Check if thing->parent is set. If so, walk the parent chain. Use `roomOfObject()` to find the ultimate room.

**Fix:** Use `roomOfObject()` instead of directly checking roomp when you need to find "what room is this ultimately in, even if nested in containers?"

### Symptom: Equipment Item Appears In Inventory List

**Likely cause:** Manual pointer assignment instead of using `equipChar()`. The item's parent was set but equippedBy was not, or it was added to stuff list instead of equipment array.

**Diagnostic approach:** Check if equippedBy and eq_pos are set correctly. Check if the item is in both equipment[] and stuff list (should never happen).

**Fix:** Use `equipChar()` and `unequip()` exclusively for equipment management. Never manually assign parent, equippedBy, or equipment[] pointers.

### Symptom: Rider Chain Corruption After Dismount

**Likely cause:** Manual modification of riding, rider, or nextRider pointers instead of using `dismount()`.

**Diagnostic approach:** Walk the rider chain and check for NULL pointers in the middle, cycles, or dangling pointers. Check if a rider's `riding` pointer doesn't match the mount's `rider` chain.

**Fix:** Use `mount()` and `dismount()` exclusively. Never manually modify riding relationship pointers. If you need to remove all riders, iterate through the chain and call `dismount()` for each.

### Symptom: Dangling Pointer Crash In Room's stuff List

**Likely cause:** Deleting a thing without removing it from the room first. The room's stuff list contains a pointer to freed memory.

**Diagnostic approach:** Check if `operator--` was called before `delete`. Look for direct `delete thing` calls without prior removal from location.

**Fix:** Always `--(*thing)` before `delete thing`. The removal clears all location pointers and removes the thing from container lists, ensuring no dangling pointers remain.

### Symptom: Light Level Wrong After Adding/Removing Object

**Likely cause:** `updateLight()` not being called, or being called on the wrong room (e.g., checked parent->roomp instead of roomOfObject()).

**Diagnostic approach:** Check if the object emits light. Trace whether the room's updateLight() was invoked when the object was added or removed.

**Fix:** Ensure you're adding/removing through `operator+=` and `operator--`, which handle updateLight() automatically. If you're doing something unusual, explicitly call updateLight() on the correct room.

### Symptom: Zone Stays Active After Last Player Leaves

**Likely cause:** Player was removed from room without going through the proper `operator--` path, so zone activity tracking wasn't updated.

**Diagnostic approach:** Check zone's active flag and count how many players are actually in the zone's rooms. If mismatch, zone tracking is out of sync.

**Fix:** Ensure all player movement goes through `operator--` and `operator+=`. Never manually move players by setting pointers. The operators handle zone activity tracking automatically.
