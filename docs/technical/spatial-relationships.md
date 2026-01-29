---
title: TThing Spatial Relationship System
description: SneezyMUD tracks location and relationships between entities through a system of parent pointers, room references, and specialized containment mechanisms for equipment, stuck items, and mounts.
keywords:
  - TThing
  - parent pointer
  - roomp
  - in_room
  - equippedBy
  - stuckIn
  - rider
  - riding
  - operator+=
  - operator--
  - spatial relationships
  - containment
  - equipment system
  - mount system
category: Understanding Systems
related:
  - thing-system.md
  - room-system.md
  - equipment-system.md
  - riding-system.md
last_updated: 2026-01-29
source_files:
  - code/code/misc/thing.h
  - code/code/misc/structs.cc
  - code/code/misc/riding.cc
  - code/code/misc/range.cc
  - code/code/misc/limbs.cc
  - code/code/sys/handler.cc
---

# TThing Spatial Relationship System

This document describes how SneezyMUD tracks location and relationships between entities.

## Core Pointer Members

```
TThing
+-- parent       Container/room/being that holds this thing
+-- roomp        Shortcut pointer to the room (NULL if inside container)
+-- in_room      Room number (Room::NOWHERE if not directly in a room)
+-- equippedBy   Being wearing this item (equipment only)
+-- eq_pos       Which equipment slot (wearSlotT)
+-- stuckIn      Being this item is stuck inside (arrows, spears)
+-- eq_stuck     Which body part the item is stuck in
+-- rider        First thing riding on this thing
+-- riding       Thing this thing is riding on
+-- nextRider    Next rider in the linked list (multiple riders)
+-- stuff        List of things contained inside this thing
```

## Location Hierarchy

```
                    +---------------+
                    |    TRoom      |
                    +-------+-------+
                            |
            +---------------+---------------+
            |               |               |
      +-----v-----+   +-----v-----+   +-----v-----+
      |  TBeing   |   |   TObj    |   |  TBeing   |
      | (player)  |   |(container)|   |  (mount)  |
      +-----+-----+   +-----+-----+   +-----+-----+
            |               |               |
    +-------+-------+       |         +-----v-----+
    |       |       |       |         |  TBeing   |
+---v---+ +-v-+ +---v---+ +-v------+  |  (rider)  |
|equip  | |inv| |stuckIn| | TObj   |  +-----------+
|[slot] | |   | |[limb] | |(inside)|
+-------+ +---+ +-------+ +--------+
```

## parent vs roomp

- Items in a room: `parent = NULL`, `roomp = the room`
- Items in inventory: `parent = the being`, `roomp = NULL`
- Items in a container: `parent = the container`, `roomp = NULL`

When inside a container, `roomp` is NULL. Use `roomOfObject()` to find the actual room.

## operator+= (Adding Things)

Adds a thing to a container and updates pointers:
```cpp
*room += *object;    // Put object in room
*being += *object;   // Put object in being's inventory
*container += *item; // Put item in container
```

| Class    | Behavior                                                    |
|----------|-------------------------------------------------------------|
| TThing   | Validates invariants, handles merging (TMergeable)          |
| TRoom    | Sets `in_room`, `roomp`, updates lighting, zone activity    |
| TBeing   | Adds to `stuff` list, sets `parent`                         |
| TObj     | Adds to `stuff` list, sets `parent`                         |
| TTable   | Mounts object on table instead of containment               |

**Pre-conditions:** `parent`, `equippedBy`, `stuckIn`, `roomp` must all be NULL.

## operator-- (Removing Things)

Removes a thing from its current location: `--(*object);`

Clears: `parent`, `roomp`, `in_room`, `equippedBy`, `stuckIn` (all set to NULL/NOWHERE).

## Equipment System

Equipped items are NOT in the being's `stuff` list - they use a separate array.

```cpp
TBeing::equipChar(obj, slot)  // Equip: sets equippedBy, eq_pos, equipment[slot]
TBeing::unequip(slot)         // Unequip: clears those pointers
```

## Stuck-In System

Objects embedded in body parts (arrows, spears). One per body slot.

```cpp
TBeing::stickIn(obj, slot)   // Sets obj->stuckIn, obj->eq_stuck, body_parts[slot]
TBeing::pulloutObj(slot)     // Removes object (causes damage!)
TBeing::getStuckIn(slot)     // Query what's stuck in a limb
```

## Mount/Rider System

Multiple riders via linked list:

```
Mount                      Riders (linked list)
+--------+                 +---------+    +---------+
| riding |<----------------| riding  |    | riding  |
| rider  |---------------->| nextRider|-->| nextRider|--> NULL
+--------+                 +---------+    +---------+
```

```cpp
TThing::mount(target)       // Start riding
TThing::dismount(position)  // Stop riding
TThing::horseMaster()       // Get controlling rider (last in chain)
```

## Simultaneous Relationships

A being participates in multiple relationships at once:
- In a room (`roomp`, `in_room`)
- Has inventory (`stuff` list)
- Wearing equipment (`equipment[]`)
- Has items stuck in limbs (`body_parts[].stuckIn`)
- Is riding something (`riding`)
- Has riders (`rider` chain)

## Bidirectional Consistency

All relationships are bidirectional and must stay consistent:

| Relationship    | Forward Pointer      | Back Pointer           |
|-----------------|----------------------|------------------------|
| In room         | `thing->roomp`       | `room->stuff` list     |
| In container    | `thing->parent`      | `container->stuff` list|
| Equipped        | `being->equipment[]` | `obj->equippedBy`      |
| Stuck in        | `being->body_parts[]`| `obj->stuckIn`         |
| Riding          | `rider->riding`      | `mount->rider` chain   |

**Violating consistency causes crashes.** `mud_assert()` enforces these invariants.

## Source Files

| File                         | Contents                                     |
|------------------------------|----------------------------------------------|
| `code/code/misc/thing.h`     | TThing pointer member declarations           |
| `code/code/misc/structs.cc`  | operator+= and operator-- implementations    |
| `code/code/misc/riding.cc`   | mount/dismount implementations               |
| `code/code/misc/range.cc`    | stickIn implementation                       |
| `code/code/misc/limbs.cc`    | body_parts accessors                         |
| `code/code/sys/handler.cc`   | thing_to_room, equipChar, unequip, pulloutObj|
