---
title: Special Procedures (Spec Procs) System
description: Special procedures are callbacks giving mobs, objects, and rooms custom behavior on commands, pulses, and events through a flexible function pointer system.
keywords:
  - spec procs
  - special procedures
  - mob_specials
  - objSpecials
  - roomSpecials
  - CMD_GENERIC_PULSE
  - CMD_GENERIC_QUICK_PULSE
  - act_ptr
  - swapToStrung
  - DELETE_THIS
  - DELETE_VICT
  - triggerSpecial
  - checkSpec
category: Important Systems
related:
  - mob-system.md
  - object-system.md
  - room-system.md
  - delete-flags.md
  - command-system.md
last_updated: 2026-01-29
source_files:
  - code/code/spec/spec_mobs.h
  - code/code/spec/spec_mobs.cc
  - code/code/spec/spec_objs.h
  - code/code/spec/spec_objs.cc
  - code/code/spec/spec_rooms.h
  - code/code/spec/spec_rooms.cc
  - code/code/misc/parse.cc
  - code/code/misc/parse.h
---

# Special Procedures (Spec Procs) System

Special procedures are callbacks giving mobs, objects, and rooms custom behavior on commands, pulses, and events.

## Spec Proc Types

| Type | Struct | Array | Signature |
|------|--------|-------|-----------|
| Mob | `TMobSpecs` | `mob_specials[]` | `int proc(TBeing*, cmdTypeT, const char*, TMonster*, TObj*)` |
| Object | `TObjSpecs` | `objSpecials[]` | `int proc(TBeing*, cmdTypeT, const char*, TObj*, TObj*)` |
| Room | `TRoomSpecs` | `roomSpecials[]` | `int proc(TBeing*, cmdTypeT, const char*, TRoom*)` |

## The t2/obj Parameter Warning

**CRITICAL:** The final pointer parameter is NOT always a valid pointer. Some commands pass integers cast to pointers:

- `CMD_MOB_MOVED_INTO_ROOM`: `obj` is old room number cast to `TObj*`
- `CMD_MOB_KILLED_NEARBY`: `obj` is the victim cast to `TObj*`
- `CMD_MOB_VIOLENCE_PEACEFUL`: `obj` is violence target cast to `TObj*`
- `CMD_OBJ_MOVEMENT`: `arg` castable to `dirTypeT`
- `CMD_OBJ_HIT`: `arg` castable to `wearSlotT`

**Never dereference these pointers without checking the command type first.**

## Command Types

### Periodic Pulses

Two pulse types trigger spec procs at different rates:

| Command | Scheduler | Interval | Use Case |
|---------|-----------|----------|----------|
| `CMD_GENERIC_PULSE` | `Pulse::SPEC_PROCS` | 3.6 seconds | Most periodic behavior |
| `CMD_GENERIC_QUICK_PULSE` | `Pulse::COMBAT` | 1.2 seconds | Time-critical updates |

**When to use each:**

- **CMD_GENERIC_PULSE** (default): Use for most periodic behavior - checking room contents, ambient actions, NPC activities. Most spec procs only need this.
- **CMD_GENERIC_QUICK_PULSE**: Use when faster response is required - combat-related procs, moving vehicles, time-sensitive game mechanics. Runs 3x more frequently, so keep processing minimal.

```cpp
// Standard periodic check (3.6s)
if (cmd == CMD_GENERIC_PULSE) {
    // Do periodic work...
}

// Fast periodic check (1.2s) - use sparingly
if (cmd == CMD_GENERIC_QUICK_PULSE) {
    // Time-critical work only...
}
```

**Performance note:** Quick pulse procs run on all objects/mobs every 1.2 seconds. Heavy processing in quick pulse handlers can impact server performance.

### Lifecycle
- `CMD_GENERIC_INIT` - During load, while parsing
- `CMD_GENERIC_CREATED` - After read_mobile/read_object
- `CMD_GENERIC_DESTROYED` - Before deletion
- `CMD_GENERIC_RESET` - During zone reset

### Player Commands
When `cmd >= 0`, a player issued that command (e.g., `CMD_SAY`, `CMD_GIVE`).

### Combat (Mobs)
`CMD_MOB_COMBAT`, `CMD_MOB_COMBAT2`, `CMD_MOB_COMBAT_ONATTACK`, `CMD_MOB_COMBAT_ONATTACKED`, `CMD_MOB_KILLED_NEARBY`, `CMD_MOB_VIOLENCE_PEACEFUL`

### Objects
`CMD_OBJ_HIT`, `CMD_OBJ_HITTING`, `CMD_OBJ_MISS`, `CMD_OBJ_BEEN_HIT`, `CMD_OBJ_GOTTEN`, `CMD_OBJ_MOVEMENT`

### Rooms
`CMD_ROOM_ENTERED`, `CMD_ROOM_ATTEMPTED_EXIT`

## Return Values

| Value | Meaning |
|-------|---------|
| `FALSE` (0) | Nothing special happened |
| `TRUE` (non-zero) | Command was handled/eaten |

### DELETE Flags

DELETE flags signal deferred deletion - the caller deletes, not the proc. **Always use `IS_SET_DELETE()`, not `IS_SET()`:**

```cpp
// CORRECT
if (IS_SET_DELETE(rc, DELETE_THIS)) { ... }

// WRONG - won't detect the combined bit pattern
if (IS_SET(rc, DELETE_THIS)) { ... }
```

| Flag | What To Delete |
|------|----------------|
| `DELETE_THIS` | The object/mob the proc is on |
| `DELETE_VICT` | The ch/victim parameter |
| `DELETE_ITEM` | The obj/t2 parameter |

### Flag Translation by Context (from spec_mobs.cc)

```
CMD_GENERIC_PULSE, CMD_MOB_COMBAT:
    If mob dies, return DELETE_THIS (do not delete directly)

CMD_MOB_VIOLENCE_PEACEFUL:
    Return DELETE_VICT to kill first TBeing

Generic Commands:
    DELETE_VICT = first TBeing (ch) gone
    DELETE_THIS = second TBeing (myself) gone
```

## triggerSpecial() Flow (parse.cc)

Builds a safe snapshot before iteration to handle procs that modify room contents:

```cpp
int TBeing::triggerSpecial(TThing* ch, cmdTypeT cmd, const char* arg) {
    // 1. Check task/spell interruption
    // 2. Check room spec proc
    // 3. Check equipment and inventory
    // 4. Build safe list of room contents
    std::vector<TThing*> things;
    for (auto it = roomp->stuff.begin(); it != roomp->stuff.end();)
        things.push_back(*(it++));

    // 5. Iterate safe list
    for (auto t : things) {
        if (!t || t->in_room != roomp->number)
            continue;  // Skip if deleted or moved
        rc = t->checkSpec(this, cmd, arg, ch);
        // Handle DELETE flags...
    }
}
```

## Writing a Spec Proc

```cpp
int myMobProc(TBeing* ch, cmdTypeT cmd, const char* arg, TMonster* myself, TObj*) {
    // Clean up on destruction
    if (cmd == CMD_GENERIC_DESTROYED) {
        delete static_cast<MyData*>(myself->act_ptr);
        myself->act_ptr = nullptr;
        return FALSE;
    }

    // Periodic behavior
    if (cmd == CMD_GENERIC_PULSE) {
        if (!myself->awake() || myself->fight())
            return FALSE;
        // Do stuff...
        return FALSE;
    }

    // React to player commands
    if (cmd == CMD_SAY) {
        // React...
        return TRUE;  // Eat the command
    }

    return FALSE;
}
```

## Persistent State with act_ptr

Spec procs can store persistent state between calls using the `act_ptr` member (available on `TMonster`, `TObj`, and `TRoom`). This enables state machines, tracking data, and complex multi-step behaviors.

### Pattern: Allocate on Created, Free on Destroyed

```cpp
struct MyProcState {
    int phase;
    sstring target_name;
    int tick_count;

    MyProcState() : phase(0), tick_count(0) {}
};

int myStatefulProc(TBeing* ch, cmdTypeT cmd, const char* arg, TMonster* myself, TObj*) {
    MyProcState* state = nullptr;

    // Retrieve existing state if present
    if (myself->act_ptr)
        state = static_cast<MyProcState*>(myself->act_ptr);

    switch (cmd) {
        case CMD_GENERIC_CREATED:
            // Allocate state on mob creation
            if (myself->act_ptr) {
                vlogf(LOG_PROC, "Mob created with existing act_ptr!");
                return FALSE;
            }
            myself->act_ptr = new MyProcState();
            return FALSE;

        case CMD_GENERIC_DESTROYED:
            // CRITICAL: Free state on destruction to prevent memory leaks
            if (myself->act_ptr) {
                delete static_cast<MyProcState*>(myself->act_ptr);
                myself->act_ptr = nullptr;
            }
            return FALSE;

        case CMD_GENERIC_PULSE:
            if (!state) return FALSE;

            // Use state for multi-step behavior
            state->tick_count++;
            if (state->phase == 0 && state->tick_count > 10) {
                state->phase = 1;
                state->tick_count = 0;
            }
            return FALSE;
    }
    return FALSE;
}
```

### State Machine Pattern

Many spec procs implement state machines using `act_ptr`. Common patterns include:

```cpp
class hunt_struct {
  public:
    int cur_pos;       // Position along path
    int cur_path;      // Which path to follow
    char* target;      // Current hunt target

    hunt_struct() : cur_pos(0), cur_path(0), target(nullptr) {}
    ~hunt_struct() { delete[] target; }
};

// In CMD_GENERIC_PULSE:
switch (job->cur_path) {
    case 0:  // Path to location A
        if (reached_destination) {
            job->cur_path = 1;  // Switch to return path
            job->cur_pos = 0;
        }
        break;
    case 1:  // Path to location B
        // ...
        break;
}
```

### Gotchas

- **Always check for null:** `act_ptr` may not be initialized if `CMD_GENERIC_CREATED` wasn't called (e.g., mob loaded from rent)
- **Type safety:** Use `static_cast` with the correct type; casting to wrong type causes undefined behavior
- **Memory ownership:** The spec proc owns the memory; always free in `CMD_GENERIC_DESTROYED`
- **Lazy initialization:** Some procs allocate on first `CMD_GENERIC_PULSE` instead of `CMD_GENERIC_CREATED` for mobs that may bypass creation

## swapToStrung Pattern

The `swapToStrung()` function enables customizing an object or mob's strings (name, short description, long description) at runtime. After calling it, modifications persist and the entity is marked as "strung" (customized).

### When to Use

- Creating customized items (notes, signs, crafted goods)
- Disguising or transforming mobs
- Generating dynamic descriptions

### Object Usage

```cpp
// TObj::swapToStrung() - copies prototype strings to instance
void TObj::swapToStrung() {
    if (isObjStat(ITEM_STRUNG))
        return;  // Already strung, no-op

    addObjStat(ITEM_STRUNG);
    name = obj_index[getItemIndex()].name;
    shortDescr = obj_index[getItemIndex()].short_desc;
    // ... copies other strings from prototype
}

// Usage example: creating a customized note
TObj* note = read_object(NOTE_VNUM, VIRTUAL);
note->swapToStrung();
note->name = "letter sealed red wax";
note->shortDescr = "a letter sealed with red wax";
note->setDescr("A folded letter sealed with red wax lies here.");
```

### Mob Usage

```cpp
// TMonster::swapToStrung() - enables string customization
void TMonster::swapToStrung() {
    if (specials.act & ACT_STRINGS_CHANGED)
        return;  // Already strung

    specials.act |= ACT_STRINGS_CHANGED;
    name = mob_index[getMobIndex()].name;
    shortDescr = mob_index[getMobIndex()].short_desc;
    player.longDescr = mob_index[getMobIndex()].long_desc;
    setDescr(mob_index[getMobIndex()].description);
}

// Usage example: doppelganger copying a player's appearance
tMyself->swapToStrung();
tMyself->name = victim->name;
tMyself->shortDescr = victim->shortDescr;
tMyself->player.longDescr = victim->getLongDesc();
```

### Quick Pulse Initialization Pattern

Some objects need to be strung immediately after creation, but `CMD_GENERIC_CREATED` fires before the object is fully initialized. Use `CMD_GENERIC_QUICK_PULSE` for deferred initialization:

```cpp
int graffitiMaker(TBeing* ch, cmdTypeT cmd, const char* arg, TObj* o, TObj*) {
    // Can't use CMD_GENERIC_CREATED - object not fully initialized yet
    // Use CMD_GENERIC_QUICK_PULSE for deferred stringing
    if (cmd == CMD_GENERIC_QUICK_PULSE && !o->isObjStat(ITEM_STRUNG)) {
        o->swapToStrung();
        o->name = format("%s %s") % o->name % colorName;
        o->shortDescr = format("%s%s<1>") % colorCode % o->shortDescr;
        return FALSE;
    }

    if (cmd != CMD_WRITE)
        return FALSE;
    // ... handle write command
}
```

### Gotchas

- **Call swapToStrung() first:** Always call before modifying strings; without it, changes may be lost or affect the prototype
- **Check ITEM_STRUNG:** Calling `swapToStrung()` on an already-strung object is a no-op, but checking first is cleaner
- **String ownership:** After strung, the object/mob owns its strings; modifications are safe
- **Persistence:** Strung objects save their custom strings to rent files

## Gotchas

1. **Never delete objects directly** - return DELETE_* flags
2. **Check cmd before using obj/t2** - may be an integer cast
3. **Return TRUE to eat commands**
4. **Check `myself->awake()` and `myself->fight()`** in periodic procs
5. **Clean up `act_ptr` in CMD_GENERIC_DESTROYED**
6. **Use `IS_SET_DELETE()`, never `IS_SET()`**

## Files

| File | Content |
|------|---------|
| `code/code/spec/spec_mobs.h` | Mob spec constants (SPEC_*) |
| `code/code/spec/spec_mobs.cc` | Mob implementations, `mob_specials[]` |
| `code/code/spec/spec_objs.h` | Object spec constants |
| `code/code/spec/spec_objs.cc` | Object implementations, `objSpecials[]` |
| `code/code/spec/spec_rooms.h` | Room spec constants |
| `code/code/spec/spec_rooms.cc` | Room implementations, `roomSpecials[]` |
| `code/code/misc/parse.cc` | `triggerSpecial()` |
| `code/code/misc/parse.h` | `cmdTypeT` enum |
