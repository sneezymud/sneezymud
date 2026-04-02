# Trap Damage Centralization Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Centralize duplicated room-wide and other-side-of-door trap damage iteration into a data-driven table and shared helpers, restoring lost behavioral regressions and eliminating ~15 copies of the same iteration pattern.

**Architecture:** A `TrapTypeInfo` lookup table maps each `doorTrapT` to its damage type, effect messages, and engulfed function. Two helper functions (`applyRoomWideDamage`, `applyOtherSideDamage`) encapsulate the room iteration pattern. All five trigger functions use the table and helpers instead of inline iteration.

**Tech Stack:** C++20, existing MUD framework (TBeing, TThing, TRoom, act(), objDamage(), DELETE flag system)

**Spec:** `docs/superpowers/specs/2026-04-02-trap-damage-centralization-design.md`

---

### Task 1: Fix sendTo newline issue in message constants

The source message constants used with `sendTo(format(...))` are missing `\n\r`. `sendTo` does not auto-append newlines. This affects ~48 call sites.

**Files:**
- Modify: `code/code/misc/trap.cc:58-67` (message constants)

- [ ] **Step 1: Add `\n\r` to all source message constants**

In the anonymous namespace at the top of `trap.cc`, update these constants:

```cpp
  // Standardized trap source messages - WHERE the trap comes from
  constexpr const char* DOOR_TRAP_CHAR_MSG = "A mechanism in the %s triggers!\n\r";
  constexpr const char* DOOR_TRAP_ROOM_MSG = "A mechanism in the %s triggers!";
  constexpr const char* PORTAL_TRAP_CHAR_MSG = "A mechanism in the %s triggers!\n\r";
  constexpr const char* PORTAL_TRAP_ROOM_MSG = "A mechanism in the %s triggers!";
  constexpr const char* CONTAINER_TRAP_CHAR_MSG = "The %s springs a trap!\n\r";
  constexpr const char* CONTAINER_TRAP_ROOM_MSG = "The %s springs a trap!";
  constexpr const char* MINE_TRAP_CHAR_MSG = "The %s detonates!\n\r";
  constexpr const char* MINE_TRAP_ROOM_MSG = "The %s detonates!";
  constexpr const char* ARROW_TRAP_CHAR_MSG = "The %s releases its trapped payload!\n\r";
  constexpr const char* ARROW_TRAP_ROOM_MSG = "The %s releases its trapped payload!";
```

Only the `*_CHAR_MSG` variants need `\n\r` — they're used with `sendTo()`. The `*_ROOM_MSG` variants are used with `act()` which handles newlines.

- [ ] **Step 2: Build and verify**

Run: `make`
Expected: Clean compile, no errors.

- [ ] **Step 3: Commit**

```bash
git add code/code/misc/trap.cc
git commit -m "fix: add missing newlines to trap source message constants

sendTo() does not auto-append newlines like act() does. All
CHAR_MSG constants used with sendTo(format(...)) were missing
the trailing newline, producing garbled single-line output."
```

---

### Task 2: Add TrapTypeInfo table and applyRoomWideDamage helper

**Files:**
- Modify: `code/code/misc/trap.cc:29-97` (anonymous namespace, add after existing constants)

- [ ] **Step 1: Add the TrapTypeInfo struct and lookup table**

Add inside the existing anonymous namespace in `trap.cc`, after the message constants (after line ~97):

```cpp
  // Maps each doorTrapT to its intrinsic properties.
  // Status-only types (poison, sleep, disease, teleport) have damageType = 0
  // since they use dedicated functions with varied return semantics.
  struct TrapTypeInfo {
    int damageType;
    const char* charMsg;
    const char* roomMsg;
    int (TBeing::*engulfedFn)();
  };

  constexpr std::array<TrapTypeInfo, MAX_TRAP_TYPES> trapTypeInfo = {{
    // DOOR_TRAP_NONE
    {0, nullptr, nullptr, nullptr},
    // DOOR_TRAP_POISON (status-only)
    {0, POISON_EFFECT_CHAR_MSG, POISON_EFFECT_ROOM_MSG, nullptr},
    // DOOR_TRAP_SPIKE
    {DAMAGE_TRAP_PIERCE, SPIKE_EFFECT_CHAR_MSG, SPIKE_EFFECT_ROOM_MSG, nullptr},
    // DOOR_TRAP_SLEEP (status-only)
    {0, SLEEP_EFFECT_CHAR_MSG, SLEEP_EFFECT_ROOM_MSG, nullptr},
    // DOOR_TRAP_TNT
    {DAMAGE_TRAP_TNT, TNT_EFFECT_CHAR_MSG, TNT_EFFECT_ROOM_MSG, nullptr},
    // DOOR_TRAP_BLADE
    {DAMAGE_TRAP_SLASH, BLADE_EFFECT_CHAR_MSG, BLADE_EFFECT_ROOM_MSG, nullptr},
    // DOOR_TRAP_FIRE
    {DAMAGE_TRAP_FIRE, FIRE_EFFECT_CHAR_MSG, FIRE_EFFECT_ROOM_MSG, &TBeing::flameEngulfed},
    // DOOR_TRAP_ACID
    {DAMAGE_TRAP_ACID, ACID_EFFECT_CHAR_MSG, ACID_EFFECT_ROOM_MSG, &TBeing::acidEngulfed},
    // DOOR_TRAP_DISEASE (status-only)
    {0, DISEASE_EFFECT_CHAR_MSG, DISEASE_EFFECT_ROOM_MSG, nullptr},
    // DOOR_TRAP_HAMMER
    {DAMAGE_TRAP_BLUNT, BLUNT_EFFECT_CHAR_MSG, BLUNT_EFFECT_ROOM_MSG, nullptr},
    // DOOR_TRAP_FROST
    {DAMAGE_TRAP_FROST, FROST_EFFECT_CHAR_MSG, FROST_EFFECT_ROOM_MSG, &TBeing::frostEngulfed},
    // DOOR_TRAP_TELEPORT (status-only)
    {0, TELEPORT_EFFECT_CHAR_MSG, TELEPORT_EFFECT_ROOM_MSG, nullptr},
    // DOOR_TRAP_ENERGY
    {DAMAGE_TRAP_ENERGY, ENERGY_EFFECT_CHAR_MSG, ENERGY_EFFECT_ROOM_MSG, nullptr},
    // DOOR_TRAP_BOLT
    {DAMAGE_TRAP_PIERCE, SPIKE_EFFECT_CHAR_MSG, SPIKE_EFFECT_ROOM_MSG, nullptr},
    // DOOR_TRAP_DISK
    {DAMAGE_TRAP_SLASH, BLADE_EFFECT_CHAR_MSG, BLADE_EFFECT_ROOM_MSG, nullptr},
    // DOOR_TRAP_PEBBLE
    {DAMAGE_TRAP_BLUNT, BLUNT_EFFECT_CHAR_MSG, BLUNT_EFFECT_ROOM_MSG, nullptr},
  }};
```

Note: The `DAMAGE_TRAP_*` constants are negative ints defined in `damage.h` (already included via headers). The array needs `#include <array>` — check if it's already included, add if not.

- [ ] **Step 2: Add the applyRoomWideDamage helper**

Add after the `TrapTypeInfo` table, still in the anonymous namespace:

```cpp
  // Iterates a room's occupants and applies trap damage at a modifier.
  // Skips triggerer and any being for which filter returns true.
  // Handles DELETE_THIS by deleting dead bystanders inline.
  void applyRoomWideDamage(TBeing* triggerer, TRoom* room,
                           const TrapTypeInfo& info, int dam, double mod,
                           TObj* trapObj,
                           std::function<bool(TBeing*)> filter = nullptr) {
    for (StuffIter it = room->stuff.begin(); it != room->stuff.end();) {
      TThing* t = *(it++);
      if (auto* tbt = dynamic_cast<TBeing*>(t)) {
        if (tbt == triggerer)
          continue;
        if (filter && filter(tbt))
          continue;
        act(info.charMsg, false, tbt, 0, 0, TO_CHAR);
        act(info.roomMsg, false, tbt, 0, 0, TO_ROOM);
        int rc = tbt->objDamage(info.damageType, static_cast<int>(dam * mod), trapObj);
        if (IS_SET_DELETE(rc, DELETE_THIS)) {
          delete tbt;
          tbt = nullptr;
        }
      }
    }
  }
```

Add `#include <functional>` at the top of trap.cc if not already present.

- [ ] **Step 3: Add the applyOtherSideDamage helper**

Add after `applyRoomWideDamage`, still in the anonymous namespace:

```cpp
  // Applies trap damage to beings in the room on the other side of a door.
  // Always uses OTHER_SIDE_MOD. No filter — old code didn't check immortality
  // on the other side.
  void applyOtherSideDamage(TBeing* triggerer, TRoom* otherRoom,
                            const TrapTypeInfo& info, int dam) {
    for (StuffIter it = otherRoom->stuff.begin(); it != otherRoom->stuff.end();) {
      TThing* t = *(it++);
      if (auto* tbt = dynamic_cast<TBeing*>(t)) {
        if (tbt == triggerer)
          continue;
        act(info.charMsg, false, tbt, 0, 0, TO_CHAR);
        act(info.roomMsg, false, tbt, 0, 0, TO_ROOM);
        int rc = tbt->objDamage(info.damageType, static_cast<int>(dam * OTHER_SIDE_MOD), nullptr);
        if (IS_SET_DELETE(rc, DELETE_THIS)) {
          delete tbt;
          tbt = nullptr;
        }
      }
    }
  }

  // Door trap types that produce area effects (room-wide + other-side).
  constexpr bool isDoorAreaTrap(doorTrapT type) {
    return type == DOOR_TRAP_TNT || type == DOOR_TRAP_FROST
        || type == DOOR_TRAP_ENERGY || type == DOOR_TRAP_ACID;
  }
```

- [ ] **Step 4: Build and verify**

Run: `make`
Expected: Clean compile. The new code is not yet called — just defined.

- [ ] **Step 5: Commit**

```bash
git add code/code/misc/trap.cc
git commit -m "add TrapTypeInfo table and room-wide damage helpers

Data-driven lookup table mapping doorTrapT to damage type,
effect messages, and engulfed function. Two helper functions
encapsulate the room-wide and other-side iteration patterns
that were duplicated ~15 times across trigger functions."
```

---

### Task 3: Refactor triggerDoorTrap to use table and helpers

This is the most critical change — it restores room-wide and other-side damage for frost, energy, and acid door traps while eliminating the inline switch bloat.

**Files:**
- Modify: `code/code/misc/trap.cc:1063-1221` (`triggerDoorTrap` function)

- [ ] **Step 1: Rewrite triggerDoorTrap**

Replace the entire function body (lines 1063-1221) with:

```cpp
// returns DELETE_THIS or FALSE
int TBeing::triggerDoorTrap(dirTypeT door) {
  roomDirData *exitp, *back = nullptr;
  TRoom* rp = nullptr;
  int rc;

  exitp = exitDir(door);
  int dam = dice(exitp->trap_dam, TRAP_DICE_SIZE);

  REMOVE_BIT(exitp->condition, EXIT_TRAPPED);
  if ((rp = real_roomp(exitp->to_room)) &&
      (back = rp->dir_option[rev_dir(door)])) {
    REMOVE_BIT(back->condition, EXIT_TRAPPED);
  }

  act(STRANGE_NOISE_MSG, true, this, 0, 0, TO_ROOM);
  act(STRANGE_NOISE_MSG, true, this, 0, 0, TO_CHAR);

  const auto trapType = static_cast<doorTrapT>(exitp->trap_info);
  const auto& info = trapTypeInfo[trapType];

  // Source message — where the trap is
  sendTo(format(DOOR_TRAP_CHAR_MSG) % fname(exitp->keyword));
  act(format(DOOR_TRAP_ROOM_MSG) % fname(exitp->keyword), false, this, 0, 0, TO_ROOM);

  // Status-only types: dispatch to dedicated functions
  switch (trapType) {
    case DOOR_TRAP_POISON:
      act(info.charMsg, false, this, 0, 0, TO_CHAR);
      act(info.roomMsg, false, this, 0, 0, TO_ROOM);
      trapPoison(dam);
      return false;
    case DOOR_TRAP_SLEEP:
      act(info.charMsg, false, this, 0, 0, TO_CHAR);
      act(info.roomMsg, false, this, 0, 0, TO_ROOM);
      rc = trapSleep(dam);
      if (IS_SET_DELETE(rc, DELETE_THIS))
        return DELETE_THIS;
      return false;
    case DOOR_TRAP_DISEASE:
      act(info.charMsg, false, this, 0, 0, TO_CHAR);
      act(info.roomMsg, false, this, 0, 0, TO_ROOM);
      trapDisease(dam);
      return false;
    case DOOR_TRAP_TELEPORT:
      act(info.charMsg, false, this, 0, 0, TO_CHAR);
      act(info.roomMsg, false, this, 0, 0, TO_ROOM);
      rc = trapTeleport(dam);
      if (IS_SET_DELETE(rc, DELETE_THIS))
        return DELETE_THIS;
      return rc;
    default:
      break;
  }

  // Damage types: use table-driven path

  // TNT special: destroy the door
  if (trapType == DOOR_TRAP_TNT)
    exitp->destroyDoor(door, in_room);

  // Room-wide damage for area trap types
  if (isDoorAreaTrap(trapType)) {
    // Frost and energy skip immortals in the same room; TNT and acid do not
    std::function<bool(TBeing*)> filter = nullptr;
    if (trapType == DOOR_TRAP_FROST || trapType == DOOR_TRAP_ENERGY)
      filter = [](TBeing* b) { return b->isImmortal(); };

    applyRoomWideDamage(this, roomp, info, dam, ROOM_MOD, nullptr, filter);

    // Other-side-of-door damage
    if (rp)
      applyOtherSideDamage(this, rp, info, dam);
  }

  // Effect messages and direct damage to triggerer
  act(info.charMsg, false, this, 0, 0, TO_CHAR);
  act(info.roomMsg, false, this, 0, 0, TO_ROOM);
  rc = objDamage(info.damageType, dam, nullptr);
  if (IS_SET_DELETE(rc, DELETE_THIS))
    return DELETE_THIS;

  // Engulfed effect (fire, frost, acid)
  if (info.engulfedFn) {
    rc = (this->*(info.engulfedFn))();
    if (IS_SET_DELETE(rc, DELETE_THIS))
      return DELETE_THIS;
  }

  return false;
}
```

- [ ] **Step 2: Build and verify**

Run: `make`
Expected: Clean compile.

- [ ] **Step 3: Commit**

```bash
git add code/code/misc/trap.cc
git commit -m "refactor triggerDoorTrap to use TrapTypeInfo table and helpers

Restores room-wide damage for frost/energy/acid door traps and
other-side-of-door damage for TNT/frost/energy/acid. Replaces
~160 lines of switch cases with table-driven dispatch."
```

---

### Task 4: Refactor triggerTrap (mines) to use table and helpers

The mine trigger function has the most room-wide iteration copies. The damage-type cases can use `applyRoomWideDamage`. The status-effect cases already use the template helpers (`applySimpleRoomEffect`/`applyComplexRoomEffect`) and stay as-is.

**Files:**
- Modify: `code/code/misc/trap.cc` — the `triggerTrap` function (the large TTrap trigger, not `springTrap`)

- [ ] **Step 1: Identify the damage-type mine cases that have room-wide iteration**

These cases in `triggerTrap` have inline room-wide iteration loops that should use `applyRoomWideDamage`: FIRE, BOLT, PEBBLE, DISK, TNT, FROST, ENERGY, ACID. Each has the pattern:

```cpp
if (o->isTrapEffectType(TRAP_EFF_ROOM)) {
  for (StuffIter it = roomp->stuff.begin(); ...) {
    // ... iterate, damage, delete dead
  }
}
```

Replace each with:

```cpp
if (o->isTrapEffectType(TRAP_EFF_ROOM)) {
  const auto& info = trapTypeInfo[trapType];
  applyRoomWideDamage(this, roomp, info, o->getTrapDamAmount(), ROOM_MOD, o,
                      [](TBeing* b) { return !b->desc; });
}
```

The `[](TBeing* b) { return !b->desc; }` filter preserves the old behavior of only affecting players (beings with descriptors).

- [ ] **Step 2: Fix the mine frost message**

The FROST case at line ~1596 still uses old-style messages. Update to standardized constants:

```cpp
    case DOOR_TRAP_FROST:
      sendTo(format(MINE_TRAP_CHAR_MSG) % fname(o->getName()));
      act(format(MINE_TRAP_ROOM_MSG) % fname(o->getName()), false, this, 0, 0, TO_ROOM);
```

- [ ] **Step 3: Use trapTypeInfo for effect messages in all mine damage cases**

For each damage-type case, replace hardcoded effect message constants with table lookups:

```cpp
      const auto& info = trapTypeInfo[static_cast<doorTrapT>(o->getTrapDamType())];
      act(info.charMsg, false, this, 0, 0, TO_CHAR);
      act(info.roomMsg, false, this, 0, 0, TO_ROOM);
```

This ensures consistency between the messages sent to bystanders (via `applyRoomWideDamage`) and the triggerer.

- [ ] **Step 4: Build and verify**

Run: `make`
Expected: Clean compile.

- [ ] **Step 5: Commit**

```bash
git add code/code/misc/trap.cc
git commit -m "refactor triggerTrap (mines) to use applyRoomWideDamage helper

Replaces 8 inline room-wide iteration loops with helper calls.
Fixes mine frost trap using old-style messages instead of
standardized constants."
```

---

### Task 5: Refactor triggerContTrap, triggerArrowTrap, triggerPortalTrap

These three functions only have room-wide iteration for TNT. Replace those inline loops with `applyRoomWideDamage` calls and use the table for effect messages where possible.

**Files:**
- Modify: `code/code/misc/trap.cc` — three trigger functions

- [ ] **Step 1: Refactor triggerContTrap TNT case**

Replace the room-wide loop in the TNT case (~lines 810-822) with:

```cpp
      // fry people in room
      applyRoomWideDamage(this, roomp, trapTypeInfo[DOOR_TRAP_TNT],
                          amnt, ROOM_MOD, obj);
```

- [ ] **Step 2: Refactor triggerArrowTrap TNT case**

Replace the room-wide loop in the TNT case (~lines 995-1007) with:

```cpp
      applyRoomWideDamage(this, roomp, trapTypeInfo[DOOR_TRAP_TNT],
                          amnt, ROOM_MOD, obj,
                          [](TBeing* b) { return b->isImmortal(); });
```

Note: arrow TNT filtered immortals in the old code, unlike container TNT.

- [ ] **Step 3: Refactor triggerPortalTrap TNT case**

Replace the room-wide loop in the portal TNT case (~lines 685-705) with:

```cpp
      applyRoomWideDamage(this, roomp, trapTypeInfo[DOOR_TRAP_TNT],
                          amnt, ROOM_MOD, o,
                          [](TBeing* b) { return b->isImmortal(); });
```

- [ ] **Step 4: Build and verify**

Run: `make`
Expected: Clean compile.

- [ ] **Step 5: Commit**

```bash
git add code/code/misc/trap.cc
git commit -m "refactor container/arrow/portal TNT traps to use applyRoomWideDamage

Replaces 3 more inline room-wide iteration loops with the
shared helper function."
```

---

### Task 6: Consolidate damage modifiers and delete dead code

**Files:**
- Modify: `code/code/misc/trap.cc:2925-2943` (rename function), `code/code/misc/trap.cc:3019-3064` (grenade inline switch), `code/code/misc/trap.cc:3076-3118` (arrow inline switch)
- Modify: `code/code/misc/trap.cc:1699-1976` (delete 8 dead functions)
- Modify: `code/code/misc/being.h:1070-1077` (delete 8 dead declarations)

- [ ] **Step 1: Rename getDoorTrapDamageModifier to getTrapDamageModifier**

In `trap.cc`, rename the function at line ~2925:

```cpp
  constexpr int getTrapDamageModifier(doorTrapT trap_type) {
```

Update all three existing call sites (lines ~2987, ~2996, ~3005) from `getDoorTrapDamageModifier` to `getTrapDamageModifier`. Remove the "Same modifiers as door traps" comments since it's now obviously shared.

- [ ] **Step 2: Replace grenade inline switch with getTrapDamageModifier call**

In `getGrenadeTrapDam` (~line 3019-3064), replace the entire switch block with:

```cpp
  damage += getTrapDamageModifier(trap_type);
  damage = min(max(damage, 1), 50);
  return damage;
```

- [ ] **Step 3: Replace arrow inline switch with getTrapDamageModifier call**

In `getArrowTrapDam` (~line 3076-3118), replace the entire switch block with:

```cpp
  damage += getTrapDamageModifier(trap_type);
  damage = min(max(damage, 1), 50);
  return damage;
```

- [ ] **Step 4: Delete the 8 dead trapDoor*Damage functions**

Delete these functions from `trap.cc` (lines ~1699-1976):
- `trapDoorTntDamage`
- `trapDoorPierceDamage`
- `trapDoorHammerDamage`
- `trapDoorSlashDamage`
- `trapDoorFrostDamage`
- `trapDoorEnergyDamage`
- `trapDoorFireDamage`
- `trapDoorAcidDamage`

- [ ] **Step 5: Delete the 8 dead declarations from being.h**

Remove lines 1070-1077 from `being.h`:

```cpp
    int trapDoorSlashDamage(int, dirTypeT);
    int trapDoorFireDamage(int, dirTypeT);
    int trapDoorPierceDamage(int, dirTypeT);
    int trapDoorTntDamage(int, dirTypeT);
    int trapDoorAcidDamage(int, dirTypeT);
    int trapDoorHammerDamage(int, dirTypeT);
    int trapDoorEnergyDamage(int, dirTypeT);
    int trapDoorFrostDamage(int, dirTypeT);
```

- [ ] **Step 6: Build and verify**

Run: `make`
Expected: Clean compile, no warnings about unused functions.

- [ ] **Step 7: Run tests**

Run: `make test`
Expected: All tests pass.

- [ ] **Step 8: Commit**

```bash
git add code/code/misc/trap.cc code/code/misc/being.h
git commit -m "consolidate damage modifiers and remove dead trap functions

Rename getDoorTrapDamageModifier to getTrapDamageModifier and
use it in all five get*TrapDam functions. Delete 8 dead
trapDoor*Damage member functions whose logic is now handled
by the table-driven helpers."
```

---

### Task 7: Final review and format

**Files:**
- All modified files

- [ ] **Step 1: Run code review agent**

Dispatch the `superpowers:code-reviewer` agent against the full set of changes since the start of this implementation (before Task 1). Verify:
- All door trap types that should have room-wide damage do
- All door trap types that should have other-side damage do
- DELETE flag handling is correct in the new helpers
- No remaining dead code
- Message constants all have proper `\n\r` where needed

- [ ] **Step 2: Format changed files**

```bash
make format FILE=code/code/misc/trap.cc
make format FILE=code/code/misc/being.h
```

- [ ] **Step 3: Final build and test**

```bash
make && make test
```

Expected: Clean compile, all tests pass.

- [ ] **Step 4: Commit any formatting changes**

```bash
git add code/code/misc/trap.cc code/code/misc/being.h
git commit -m "style: format trap centralization changes"
```
