# Trap Damage Centralization

Centralizes the duplicated room-wide and other-side-of-door trap damage iteration into a data-driven table + shared helper functions. Fixes critical behavioral regressions where the refactor accidentally dropped room-wide and other-side damage for several door trap types, and fixes missing `\n\r` on `sendTo` calls.

## Context

The trap system refactor on the `trapRefactor` branch consolidated trap messaging and reduced ~750 lines, but introduced two critical regressions:

1. Door traps for frost, energy, and acid lost room-wide area damage (50% modifier to bystanders).
2. Door traps for TNT, frost, energy, and acid lost other-side-of-door damage (33% modifier through the door).

The old behavior lived in eight `trapDoor*Damage` member functions that are now dead code. The room-wide iteration pattern (iterate `roomp->stuff`, skip self, apply damage at a multiplier, handle DELETE_THIS cleanup) is copy-pasted ~15 times across the five trigger functions (`triggerDoorTrap`, `triggerTrap`, `triggerContTrap`, `triggerArrowTrap`, `triggerPortalTrap`).

## Design

### TrapTypeInfo Table

A `constexpr std::array<TrapTypeInfo, MAX_TRAP_TYPES>` indexed by `doorTrapT` captures the intrinsic properties of each trap type — things that should never vary between trigger contexts:

```cpp
struct TrapTypeInfo {
  int damageType;               // DAMAGE_TRAP_FIRE, etc. 0 for status-only types
  const char* charMsg;          // effect message to triggerer (TO_CHAR)
  const char* roomMsg;          // effect message to room (TO_ROOM)
  int (TBeing::*engulfedFn)();  // flameEngulfed/frostEngulfed/acidEngulfed, or nullptr
};
```

Design choices:
- Status-only types (poison, sleep, disease, teleport) have `damageType = 0`. They're dispatched through their dedicated functions (`trapPoison`, `trapSleep`, etc.) which have varied return types and semantics that don't fit a generic table-driven path.
- Engulfed functions are member function pointers. Only fire, frost, and acid have them.
- Messages are the already-existing `*_EFFECT_CHAR_MSG`/`*_EFFECT_ROOM_MSG` constants wired to their trap type.
- DOOR_TRAP_BOLT/DISK/PEBBLE map to existing damage types (pierce/slash/blunt).
- DOOR_TRAP_NONE gets a zero-initialized entry.

The table does NOT include room-wide flags, other-side flags, damage multipliers, or bystander filters — those are context-dependent decisions made by the trigger functions.

### Helper Functions

Two free functions in the `trap.cc` anonymous namespace.

**`applyRoomWideDamage`** iterates a room's occupants and applies trap damage at a modifier:

```cpp
void applyRoomWideDamage(TBeing* triggerer, TRoom* room,
                         const TrapTypeInfo& info, int dam, double mod,
                         TObj* trapObj,
                         std::function<bool(TBeing*)> filter = nullptr);
```

Behavior:
1. Iterates `room->stuff` with post-increment pattern for iterator safety
2. Skips non-TBeing and skips `triggerer`
3. Applies optional filter predicate (returns true to skip) — handles `isImmortal()` check for door frost/energy, `!desc` check for mines
4. Sends effect messages via `act()` using `info.charMsg`/`info.roomMsg`
5. Calls `tbt->objDamage(info.damageType, dam * mod, trapObj)`
6. Handles DELETE_THIS: delete + null

Returns `void` because it only processes bystanders — dead bystanders are deleted inline and the triggerer is never affected.

**`applyOtherSideDamage`** is door-specific, same pattern on the room through the door:

```cpp
void applyOtherSideDamage(TBeing* triggerer, TRoom* otherRoom,
                          const TrapTypeInfo& info, int dam,
                          TObj* trapObj = nullptr);
```

Always uses `OTHER_SIDE_MOD` (1/3). No filter predicate — the old code didn't check immortality on the other side.

Status-effect mine room-wide loops (sleep, teleport, disease, poison) stay inline in `triggerTrap` since they call different functions with varied return semantics. The helpers only cover the `objDamage` path, which is ~12 of the 15 duplicated loops.

### Trigger Function Changes

**`triggerDoorTrap`**: Status-only cases (poison, sleep, disease, teleport) stay as short inline cases. TNT stays as a special case (calls `destroyDoor()`). All other damage cases use the table for messages/damType and call the helpers for room-wide/other-side. A local predicate determines which types produce area effects:

```cpp
constexpr bool isDoorAreaTrap(doorTrapT type) {
    return type == DOOR_TRAP_TNT || type == DOOR_TRAP_FROST
        || type == DOOR_TRAP_ENERGY || type == DOOR_TRAP_ACID;
}
```

Frost and energy pass `[](TBeing* b) { return b->isImmortal(); }` as the room-wide filter, matching the old behavior where immortals were skipped in the same room but not on the other side.

**`triggerTrap` (mines)**: The 12 damage-type cases collapse their inline iteration to a `TRAP_EFF_ROOM` check + `applyRoomWideDamage` call with `[](TBeing* b) { return !b->desc; }` filter. The 4 status-effect cases keep their inline loops.

**`triggerContTrap`, `triggerArrowTrap`, `triggerPortalTrap`**: Only TNT has room-wide — its case calls `applyRoomWideDamage`. Other cases use `info.charMsg`/`info.roomMsg` from the table for consistency.

**Dead functions**: All eight `trapDoor*Damage` member functions are deleted, along with their declarations in `being.h`.

### sendTo Newline Fix

Append `\n\r` to all source message constants (`DOOR_TRAP_CHAR_MSG`, `PORTAL_TRAP_CHAR_MSG`, `CONTAINER_TRAP_CHAR_MSG`, `MINE_TRAP_CHAR_MSG`, `ARROW_TRAP_CHAR_MSG`). These are used with `sendTo(format(...))` which does not auto-append newlines.

## Behavioral Restoration

| Door Trap Type | Room-Wide (restored) | Other-Side (restored) | Filter |
|---|---|---|---|
| TNT | `ROOM_MOD` (0.5) | `OTHER_SIDE_MOD` (1/3) | none |
| Frost | `ROOM_MOD` (0.5) | `OTHER_SIDE_MOD` (1/3) | room-wide skips immortals |
| Energy | `ROOM_MOD` (0.5) | `OTHER_SIDE_MOD` (1/3) | room-wide skips immortals |
| Acid | `ROOM_MOD` (0.5) | `OTHER_SIDE_MOD` (1/3) | none |
| Pierce/Blade/Hammer/Fire | none | none | n/a |

## Consolidated Damage Modifiers

The old code had identical per-type modifier switches duplicated across all five `get*TrapDam` functions. The refactor extracted `getDoorTrapDamageModifier` for door/container/mine but changed several values, while grenade/arrow kept separate inline switches with the old values. This created a divergence.

Resolution: consolidate into a single `getTrapDamageModifier` function used by all five `get*TrapDam` functions. Values are the max (higher damage) of old and new for each type:

| Trap Type | Old (all) | New (door) | Consolidated |
|-----------|-----------|------------|-------------|
| TNT | +3 | +3 | +3 |
| Poison | -1 | -1 | -1 |
| Sleep | +1 | +1 | +1 |
| Acid | +1 | +1 | +1 |
| Disease | +3 | -1 | +3 |
| Frost | +3 | 0 | +3 |
| Fire | 0 | +1 | +1 |
| Hammer | -10 | +2 | +2 |
| Blade | -3 | +1 | +1 |
| Spike | -5 | 0 | 0 |
| Teleport | +5 | 0 | +5 |
| Energy | +5 | +1 | +5 |
| Bolt | +1 | 0 | +1 |
| Disk | +3 | 0 | +3 |
| Pebble | -5 | 0 | 0 |

The grenade and arrow functions drop their inline switches and call `getTrapDamageModifier` like the others.

## Additional Fixes

- **Mine frost trap message**: `triggerTrap()` FROST case still uses old-style `act("An icy cloud pours out of $p.", ...)` instead of standardized `MINE_TRAP_CHAR_MSG` + `FROST_EFFECT_CHAR_MSG`. Update to match all other mine cases.

## Files Changed

- `code/code/misc/trap.cc` — Add TrapTypeInfo table, helper functions, refactor trigger functions, delete dead functions, fix message constants, consolidate damage modifiers, fix mine frost message
- `code/code/misc/trap.h` — Add TrapTypeInfo struct declaration (if needed externally, otherwise stays in anonymous namespace)
- `code/code/misc/being.h` — Remove dead `trapDoor*Damage` declarations
