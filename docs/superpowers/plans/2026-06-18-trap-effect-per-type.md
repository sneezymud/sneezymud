# Trap Effect Per-Type Refactor — Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Reorganize trap effects so each of the 16 `doorTrapT` damage types has one single-victim effect function that every trap source (door/container/portal/mine/grenade/arrow) calls, replacing six near-identical per-source switches.

**Architecture:** Three layers — `applyTrapEffect()` (per-type, single victim), `dealTrapDamage()` (the one damage call + setter-attribution seam), and thin per-source applicators that own blast set + intro message + carrier fate. Supporting collapses fold the 5 `getXxxTrapDam`, 5 `getXxxTrapLearn`, and 4 `goofUpTrap` branches onto the same per-type/per-source data tables.

**Tech Stack:** C++20, Clang (preset `dev-clang`), CMake/Ninja, CTest (`make test`). Build with `make`; unit tests with `make test`.

**Design spec:** `docs/superpowers/specs/2026-06-18-trap-effect-per-type-design.md`
**Behavior reference:** `docs/systems/important/trap-system.md`

## Global Constraints

- C++20; modern idioms in new/changed code (`nullptr`, `static_cast`, `true/false`, `sstring`/`format`). Do not modernize untouched surrounding code.
- DELETE-flag discipline: use `IS_SET_DELETE(rc, DELETE_THIS)` (never `IS_SET`); after any call that can delete, check immediately and return/break; translate `DELETE_THIS → DELETE_VICT` when the dying entity was a parameter; iterate room/container `stuff` with the `*(it++)` post-increment pattern and `delete` dead bystanders inline.
- `applyTrapEffect`/`dealTrapDamage` are `TBeing` methods; `this` is the **victim**. `setter` defaults to `nullptr` (unattributed = today's behavior) — do NOT wire setter attribution in this plan; only add the parameter seam.
- Persisted formats unchanged (`roomDirData::trap_info`/`trap_dam`/`EXIT_TRAPPED`, `TTrap` obj values, container flags/bitpack, portal/arrow fields). No migration.
- `make format FILE=<file>` before each commit. End commit messages with no AI attribution line.
- Run `make` (full build, includes ASan/UBSan in dev preset) and confirm it links before every commit.

---

## File map

- `code/code/misc/trap.cc` — add `trapDamMod`, `trapSourceInfo`, `TBeing::getTrapDam`, `TBeing::getTrapLearn`, `TBeing::dealTrapDamage`, `TBeing::applyTrapEffect`, the `trapEffectInfo` table; rewrite the six trigger functions and `goofUpTrap`; delete the per-source `getXxxTrapDam`/`getXxxTrapLearn` and the `trapDoor*Damage` helpers once unused.
- `code/code/misc/being.h` — declarations for the new `TBeing` methods; remove declarations of deleted ones.
- `code/code/obj/obj_trap.cc` — `TTrap::detonateGrenade` switch + `anyTrapCheck`/`getTrapCheck` (they call `triggerTrap`, unchanged signature).
- `code/code/obj/obj_trap.h` — declaration changes if any helper signatures move.
- `code/code/task/task_trap.cc` — callers of `getXxxTrapDam`/`getXxxTrapLearn` (5 sites) updated to the collapsed calls; `goofUpTrap` callers unchanged (same signature).
- `code/code/misc/trap.h` — declare `trapDamMod`, `trapSourceInfo` if exposed; `applyTrapEffect`/etc. live on `TBeing`.
- `code/code/test/` — new CTest file `test_trap_dam.cc` (or extend an existing trap test target) for the pure pieces. Confirm the test target wiring in `code/code/test/CMakeLists.txt` (follow the pattern of an existing test there).

---

## Task 1: Pure per-type damage-modifier table + `getTrapLearn` collapse

**Files:**
- Modify: `code/code/misc/trap.cc` (add `trapDamMod`, `trapSourceInfo`, `TBeing::getTrapLearn`)
- Modify: `code/code/misc/trap.h` (declare `trapDamMod`, `trapSourceInfo`, the `TrapSourceInfo` struct)
- Modify: `code/code/misc/being.h` (declare `int getTrapLearn(trap_targ_t);`)
- Modify: `code/code/task/task_trap.cc` (repoint the 5 `getXxxTrapLearn` callers)
- Test: `code/code/test/test_trap_dam.cc`

**Interfaces:**
- Produces: `int trapDamMod(doorTrapT)` (free fn, pure); `struct TrapSourceInfo { int baseDam; int skillDivisor; spellNumT setSkill; };`; `extern const TrapSourceInfo trapSourceInfo[];` indexed by `trap_targ_t`; `int TBeing::getTrapLearn(trap_targ_t targ)`.

- [ ] **Step 1: Write the failing test** (`code/code/test/test_trap_dam.cc`)

```cpp
#include "trap.h"
#include <cassert>

// trapDamMod is pure — verify the per-type additive modifiers match the
// table documented in docs/systems/important/trap-system.md
int main() {
  assert(trapDamMod(DOOR_TRAP_TNT) == 3);
  assert(trapDamMod(DOOR_TRAP_ENERGY) == 5);
  assert(trapDamMod(DOOR_TRAP_TELEPORT) == 5);
  assert(trapDamMod(DOOR_TRAP_HAMMER) == -10);
  assert(trapDamMod(DOOR_TRAP_SPIKE) == -5);
  assert(trapDamMod(DOOR_TRAP_PEBBLE) == -5);
  assert(trapDamMod(DOOR_TRAP_BLADE) == -3);
  assert(trapDamMod(DOOR_TRAP_POISON) == -1);
  assert(trapDamMod(DOOR_TRAP_SLEEP) == 1);
  assert(trapDamMod(DOOR_TRAP_ACID) == 1);
  assert(trapDamMod(DOOR_TRAP_BOLT) == 1);
  assert(trapDamMod(DOOR_TRAP_DISEASE) == 3);
  assert(trapDamMod(DOOR_TRAP_FROST) == 3);
  assert(trapDamMod(DOOR_TRAP_DISK) == 3);
  assert(trapDamMod(DOOR_TRAP_NONE) == 0);
  return 0;
}
```

Wire it into `code/code/test/CMakeLists.txt` mirroring an existing single-file test target (find one with `grep -n add_test code/code/test/CMakeLists.txt`).

- [ ] **Step 2: Run the test to verify it fails to compile/link** (`trapDamMod` undefined)

Run: `make test 2>&1 | grep -i trap`
Expected: build error — `trapDamMod` not declared.

- [ ] **Step 3: Add `trapDamMod` and `trapSourceInfo` to `trap.cc`** (near the existing `trap_types[]` definition, ~line 109)

```cpp
int trapDamMod(doorTrapT type) {
  switch (type) {
    case DOOR_TRAP_TNT:
    case DOOR_TRAP_DISEASE:
    case DOOR_TRAP_FROST:
    case DOOR_TRAP_DISK:     return 3;
    case DOOR_TRAP_ENERGY:
    case DOOR_TRAP_TELEPORT:  return 5;
    case DOOR_TRAP_SLEEP:
    case DOOR_TRAP_ACID:
    case DOOR_TRAP_BOLT:      return 1;
    case DOOR_TRAP_POISON:    return -1;
    case DOOR_TRAP_BLADE:     return -3;
    case DOOR_TRAP_SPIKE:
    case DOOR_TRAP_PEBBLE:    return -5;
    case DOOR_TRAP_HAMMER:    return -10;
    default:                  return 0;
  }
}

const TrapSourceInfo trapSourceInfo[] = {
  /* TRAP_TARG_DOOR    */ {10, 2, SKILL_SET_TRAP_DOOR},
  /* TRAP_TARG_CONT    */ {20, 3, SKILL_SET_TRAP_CONT},
  /* TRAP_TARG_MINE    */ {20, 2, SKILL_SET_TRAP_MINE},
  /* TRAP_TARG_GRENADE */ {5, 2, SKILL_SET_TRAP_GREN},
  /* TRAP_TARG_ARROW   */ {5, 2, SKILL_SET_TRAP_ARROW},
};
```

In `trap.h` add (after the existing externs):

```cpp
struct TrapSourceInfo {
  int baseDam;
  int skillDivisor;
  spellNumT setSkill;
};
extern const TrapSourceInfo trapSourceInfo[];
extern int trapDamMod(doorTrapT);
```

(`trap.h` will need `#include "spells.h"` or a forward decl for `spellNumT` — check whether it already transitively has it; if not, include it.)

- [ ] **Step 4: Add `TBeing::getTrapLearn`** to `trap.cc` (replace the five `getXxxTrapLearn` bodies in a later cleanup task; for now add the new one alongside)

```cpp
int TBeing::getTrapLearn(trap_targ_t targ) {
  int learn = getSkillValue(trapSourceInfo[targ].setSkill);
  if (learn <= 0)
    return 0;
  return min(learn, (int)MAX_SKILL_LEARNEDNESS);
}
```

Declare in `being.h` next to the existing `getDoorTrapLearn` declarations: `int getTrapLearn(trap_targ_t);`

- [ ] **Step 5: Repoint the `getXxxTrapLearn` callers** in `task_trap.cc` (5 sites: lines ~108, 212, 346, 454, 579)

Each `ch->getDoorTrapLearn(doorTrapT(ch->task->status))` etc. becomes `ch->getTrapLearn(TRAP_TARG_DOOR)` (and `_CONT`, `_MINE`, `_GREN`, `_ARROW` respectively). The argument is the literal target for that handler, not the dam type.

- [ ] **Step 6: Build and run the test**

Run: `make format FILE=code/code/misc/trap.cc && make && make test 2>&1 | tail -20`
Expected: clean build; `test_trap_dam` PASS.

- [ ] **Step 7: Commit**

```bash
git add code/code/misc/trap.cc code/code/misc/trap.h code/code/misc/being.h code/code/task/task_trap.cc code/code/test/test_trap_dam.cc code/code/test/CMakeLists.txt
git commit -m "refactor(trap): add trapDamMod/trapSourceInfo tables, collapse getTrapLearn 5->1"
```

---

## Task 2: Collapse `getXxxTrapDam` 5 → 1 (`getTrapDam`)

**Files:**
- Modify: `code/code/misc/trap.cc` (add `TBeing::getTrapDam`)
- Modify: `code/code/misc/being.h` (declare `int getTrapDam(trap_targ_t, doorTrapT);`)
- Modify: `code/code/task/task_trap.cc` (repoint the 5 `getXxxTrapDam` callers)
- Modify: `code/code/misc/trap.cc` (`goofUpTrap` callers of `getXxxTrapDam` — 4 sites — repoint too)

**Interfaces:**
- Consumes: `trapSourceInfo`, `trapDamMod` (Task 1).
- Produces: `int TBeing::getTrapDam(trap_targ_t targ, doorTrapT type)`.

- [ ] **Step 1: Add `TBeing::getTrapDam`** to `trap.cc`

```cpp
int TBeing::getTrapDam(trap_targ_t targ, doorTrapT type) {
  const TrapSourceInfo& si = trapSourceInfo[targ];
  int damage = si.baseDam + getSkillLevel(si.setSkill) / si.skillDivisor;
  damage = damage * getTrapLearn(targ) / 100;
  damage += trapDamMod(type);
  return min(max(damage, 1), 50);
}
```

This reproduces the old per-source formulas exactly (base/divisor/skill from the table, shared modifier). Note: the old `getArrowTrapDam` omitted the poison/hammer modifier cases, but those types are never settable on arrows, so applying `trapDamMod` uniformly is a no-op for arrows — behavior preserved.

Declare in `being.h`: `int getTrapDam(trap_targ_t, doorTrapT);`

- [ ] **Step 2: Repoint callers** — in `task_trap.cc` (lines ~89, 202, 275 inside `makeTrapLand`, 437, 513 inside `makeTrapGrenade`) and in `goofUpTrap` in `trap.cc` (4 sites, ~1949, 2085, 2222, 2371).

Examples:
- `task_trap_door`: `ch->getDoorTrapDam(doorTrapT(ch->task->status))` → `ch->getTrapDam(TRAP_TARG_DOOR, doorTrapT(ch->task->status))`
- `makeTrapLand`: `ch->getMineTrapDam(status)` → `ch->getTrapDam(TRAP_TARG_MINE, status)`
- `goofUpTrap` door branch: `getDoorTrapDam(trap_type)` → `getTrapDam(TRAP_TARG_DOOR, trap_type)` (and CONT/MINE/GRENADE in their branches).

- [ ] **Step 3: Build**

Run: `make format FILE=code/code/misc/trap.cc && make 2>&1 | tail -20`
Expected: clean build (old `getXxxTrapDam` still present and now unused except their own definitions — that's fine until Task 9 cleanup).

- [ ] **Step 4: Add a characterization test** for the composition using a constructed being is impractical without combat fixtures; instead assert the modifier path via `trapDamMod` (already covered) and rely on the build + later manual matrix. Add one assertion to `test_trap_dam.cc` confirming `trapSourceInfo` ordering matches `trap_targ_t`:

```cpp
  assert(trapSourceInfo[TRAP_TARG_DOOR].setSkill == SKILL_SET_TRAP_DOOR);
  assert(trapSourceInfo[TRAP_TARG_ARROW].baseDam == 5);
```

Run: `make test 2>&1 | tail -5` → PASS.

- [ ] **Step 5: Commit**

```bash
git add code/code/misc/trap.cc code/code/misc/being.h code/code/task/task_trap.cc code/code/test/test_trap_dam.cc
git commit -m "refactor(trap): collapse getXxxTrapDam 5->1 into getTrapDam(targ,type)"
```

---

## Task 3: `dealTrapDamage` choke point

**Files:**
- Modify: `code/code/misc/trap.cc` (add `TBeing::dealTrapDamage`)
- Modify: `code/code/misc/being.h` (declare it)

**Interfaces:**
- Produces: `int TBeing::dealTrapDamage(spellNumT damageClass, int dam, TThing* carrier, TBeing* setter)` — `this` is the victim. Returns `objDamage`/`applyDamage` rc.

- [ ] **Step 1: Add the method** to `trap.cc`

```cpp
// Single point where trap *physical* damage is dealt. `this` is the victim.
// carrier = the trap object (for death-log/source attribution), may be null.
// setter = the trap's setter for XP/kill credit; null today (unattributed,
// identical to historical behavior). Attribution wiring is a separate effort.
int TBeing::dealTrapDamage(spellNumT damageClass, int dam, TThing* carrier,
  TBeing* setter) {
  if (setter && setter != this)
    return setter->applyDamage(this, dam, damageClass);
  return objDamage(damageClass, dam, carrier);
}
```

Declare in `being.h`: `int dealTrapDamage(spellNumT, int, TThing* = nullptr, TBeing* = nullptr);`

- [ ] **Step 2: Build**

Run: `make 2>&1 | tail -5`
Expected: clean build (method unused so far).

- [ ] **Step 3: Commit**

```bash
git add code/code/misc/trap.cc code/code/misc/being.h
git commit -m "refactor(trap): add dealTrapDamage choke point (setter seam, null = unattributed)"
```

---

## Task 4: `applyTrapEffect` — the per-type single-victim function

**Files:**
- Modify: `code/code/misc/trap.cc` (add `TBeing::applyTrapEffect`)
- Modify: `code/code/misc/being.h` (declare it)

**Interfaces:**
- Consumes: `dealTrapDamage` (Task 3); existing `trapPoison/trapSleep/trapDisease/trapTeleport`, `flameEngulfed/frostEngulfed/acidEngulfed`.
- Produces: `int TBeing::applyTrapEffect(doorTrapT type, int dam, TThing* carrier = nullptr, TBeing* setter = nullptr)` — `this` is the victim. Returns DELETE flags (`DELETE_THIS` if victim died, else `FALSE`).

- [ ] **Step 1: Add the function** to `trap.cc`. Each case emits the standardized victim-facing message (no `$p`/source noun — the source applicator owns the "where it came from" line) and applies the effect. Port the exact victim wording from the current `triggerTrap` (`trap.cc:1111-1509`) "You are …" lines.

```cpp
// Per-damage-type effect applied to ONE being (`this` = victim).
// Source applicators handle blast set, intro message, and carrier fate.
int TBeing::applyTrapEffect(doorTrapT type, int dam, TThing* carrier,
  TBeing* setter) {
  int rc;
  switch (type) {
    case DOOR_TRAP_POISON:
      act("You are sprayed with contact poison!", FALSE, this, 0, 0, TO_CHAR);
      act("$n is sprayed with contact poison!", FALSE, this, 0, 0, TO_ROOM);
      trapPoison(dam);
      return FALSE;
    case DOOR_TRAP_SLEEP:
      act("You are surrounded by a noxious mist!", FALSE, this, 0, 0, TO_CHAR);
      act("$n is surrounded by a noxious mist!", FALSE, this, 0, 0, TO_ROOM);
      return trapSleep(dam);
    case DOOR_TRAP_DISEASE:
      act("You are surrounded by the thick cloud!", FALSE, this, 0, 0, TO_CHAR);
      act("$n is surrounded by the thick cloud!", FALSE, this, 0, 0, TO_ROOM);
      trapDisease(dam);
      return FALSE;
    case DOOR_TRAP_TELEPORT:
      act("You find yourself sucked into the vortex!", FALSE, this, 0, 0,
        TO_CHAR);
      act("$n flails wildly, but falls into the vortex.", FALSE, this, 0, 0,
        TO_ROOM);
      return trapTeleport(dam);
    case DOOR_TRAP_FIRE:
      act("You are burned by the flames!", FALSE, this, 0, 0, TO_CHAR);
      act("$n is burned by the flames.", FALSE, this, 0, 0, TO_ROOM);
      rc = dealTrapDamage(DAMAGE_TRAP_FIRE, dam, carrier, setter);
      if (IS_SET_DELETE(rc, DELETE_THIS))
        return DELETE_THIS;
      return flameEngulfed();
    case DOOR_TRAP_FROST:
      act("You are frozen by the icy cloud!", FALSE, this, 0, 0, TO_CHAR);
      act("$n is frozen by the icy cloud.", FALSE, this, 0, 0, TO_ROOM);
      rc = dealTrapDamage(DAMAGE_TRAP_FROST, dam, carrier, setter);
      if (IS_SET_DELETE(rc, DELETE_THIS))
        return DELETE_THIS;
      return frostEngulfed();
    case DOOR_TRAP_ACID:
      act("You are surrounded by the horrid acid cloud!", FALSE, this, 0, 0,
        TO_CHAR);
      act("$n is surrounded by the horrid acid cloud.", FALSE, this, 0, 0,
        TO_ROOM);
      rc = dealTrapDamage(DAMAGE_TRAP_ACID, dam, carrier, setter);
      if (IS_SET_DELETE(rc, DELETE_THIS))
        return DELETE_THIS;
      return acidEngulfed();
    case DOOR_TRAP_ENERGY:
      act("You are devastated by dozens of plasma bolts!", FALSE, this, 0, 0,
        TO_CHAR);
      act("$n is devastated by dozens of plasma bolts.", FALSE, this, 0, 0,
        TO_ROOM);
      return dealTrapDamage(DAMAGE_TRAP_ENERGY, dam, carrier, setter);
    case DOOR_TRAP_TNT:
      act("You are blasted by the explosion!", FALSE, this, 0, 0, TO_CHAR);
      act("$n is blasted by the explosion.", FALSE, this, 0, 0, TO_ROOM);
      return dealTrapDamage(DAMAGE_TRAP_TNT, dam, carrier, setter);
    case DOOR_TRAP_SPIKE:
      act("Sharpened spikes leap into you!", FALSE, this, 0, 0, TO_CHAR);
      act("$n is skewered by sharpened spikes.", FALSE, this, 0, 0, TO_ROOM);
      return dealTrapDamage(DAMAGE_TRAP_PIERCE, dam, carrier, setter);
    case DOOR_TRAP_BOLT:
      act("You are perforated by the bolts!", FALSE, this, 0, 0, TO_CHAR);
      act("$n is perforated by the bolts.", FALSE, this, 0, 0, TO_ROOM);
      return dealTrapDamage(DAMAGE_TRAP_PIERCE, dam, carrier, setter);
    case DOOR_TRAP_BLADE:
      act("Razor sharp blades slice into you!", FALSE, this, 0, 0, TO_CHAR);
      act("$n is sliced by razor sharp blades.", FALSE, this, 0, 0, TO_ROOM);
      return dealTrapDamage(DAMAGE_TRAP_SLASH, dam, carrier, setter);
    case DOOR_TRAP_DISK:
      act("You are slashed by the razor-disks!", FALSE, this, 0, 0, TO_CHAR);
      act("$n is slashed by the razor-disks.", FALSE, this, 0, 0, TO_ROOM);
      return dealTrapDamage(DAMAGE_TRAP_SLASH, dam, carrier, setter);
    case DOOR_TRAP_HAMMER:
      act("You are crushed by a falling weight!", FALSE, this, 0, 0, TO_CHAR);
      act("$n is crushed by a falling weight.", FALSE, this, 0, 0, TO_ROOM);
      return dealTrapDamage(DAMAGE_TRAP_BLUNT, dam, carrier, setter);
    case DOOR_TRAP_PEBBLE:
      act("You are hit by the fusillade!", FALSE, this, 0, 0, TO_CHAR);
      act("$n is hit by the pebbles.", FALSE, this, 0, 0, TO_ROOM);
      return dealTrapDamage(DAMAGE_TRAP_BLUNT, dam, carrier, setter);
    default:
      vlogf(LOG_BUG, format("applyTrapEffect: unknown trap type %d") % type);
      return FALSE;
  }
}
```

Declare in `being.h`: `int applyTrapEffect(doorTrapT, int, TThing* = nullptr, TBeing* = nullptr);`

- [ ] **Step 2: Build**

Run: `make format FILE=code/code/misc/trap.cc && make 2>&1 | tail -10`
Expected: clean build (unused so far). Confirm no warnings about unhandled enum cases.

- [ ] **Step 3: Commit**

```bash
git add code/code/misc/trap.cc code/code/misc/being.h
git commit -m "refactor(trap): add applyTrapEffect (per-type single-victim effect)"
```

---

## Task 5: Migrate the arrow source (simplest — single target, no carrier deletion)

**Files:**
- Modify: `code/code/misc/trap.cc` (`TBeing::triggerArrowTrap`, `trap.cc:757-908`)

**Interfaces:**
- Consumes: `applyTrapEffect`, `TArrow::getTrapDamAmount()`.

- [ ] **Step 1: Replace the body** of `triggerArrowTrap` with the applicator shape (keep the existing intro + 1% fizzle):

```cpp
int TBeing::triggerArrowTrap(TArrow* obj) {
  act("You hear a strange noise...", TRUE, this, 0, 0, TO_ROOM);
  act("You hear a strange noise...", TRUE, this, 0, 0, TO_CHAR);

  if (!::number(0, 100)) {
    act("...But nothing happens.", TRUE, this, 0, 0, TO_CHAR);
    act("...But nothing happens.", TRUE, this, 0, 0, TO_ROOM);
    return FALSE;
  }

  // arrow = single target (the struck victim); carrier = the arrow
  return applyTrapEffect(obj->getTrapDamType(), obj->getTrapDamAmount(), obj);
}
```

`applyTrapEffect` returns `DELETE_THIS` if the victim died, which is exactly what `triggerArrowTrap`'s callers already expect. (Arrow never had a `DELETE_ITEM`/room-blast path except TNT; per the design, area is now a source property and arrow is single-target — TNT arrow becoming single-target is an accepted, documented normalization.)

- [ ] **Step 2: Build**

Run: `make format FILE=code/code/misc/trap.cc && make 2>&1 | tail -10`
Expected: clean build.

- [ ] **Step 3: Manual verification** (functional)

Per `docs/systems/important/trap-system.md`, an arrow trap fires from `obj_base_weapon.cc` on hit. Verify on a running server (`make test-func` env or local): OEdit/settrap an arrow with a fire trap, shoot it at a mob, confirm: "strange noise" → "You are burned by the flames!" → damage applied → arrow's trap clears. Confirm a sleep-arrow sleeps, a teleport-arrow teleports.

- [ ] **Step 4: Commit**

```bash
git add code/code/misc/trap.cc
git commit -m "refactor(trap): migrate arrow trigger to applyTrapEffect"
```

---

## Task 6: Migrate container and portal sources (single target, carrier deletes via DELETE_ITEM)

**Files:**
- Modify: `code/code/misc/trap.cc` (`TBeing::triggerContTrap` `598-752`, `TBeing::triggerPortalTrap` `420-593`)

**Interfaces:**
- Consumes: `applyTrapEffect`. Container uses `obj->getContainerTrapType()`/`getContainerTrapDam()`; portal uses `o->getPortalTrapType()`/`getPortalTrapDam()`.

- [ ] **Step 1: Rewrite `triggerContTrap`** — intro + flag changes + single-victim effect + carrier fate. Preserve the existing flag bookkeeping and the explosive "incinerate contents" behavior (that is carrier fate, not a damage-type concern):

```cpp
int TBeing::triggerContTrap(TOpenContainer* obj) {
  act("You hear a strange noise...", TRUE, this, 0, 0, TO_ROOM);
  act("You hear a strange noise...", TRUE, this, 0, 0, TO_CHAR);
  obj->remContainerFlag(CONT_TRAPPED);
  obj->remContainerFlag(CONT_CLOSED);
  obj->addContainerFlag(CONT_EMPTYTRAP);

  if (!::number(0, 100)) {
    act("...But nothing happens.", TRUE, this, 0, 0, TO_CHAR);
    act("...But nothing happens.", TRUE, this, 0, 0, TO_ROOM);
    return FALSE;
  }

  doorTrapT type = obj->getContainerTrapType();

  // carrier fate for destructive types: container + contents destroyed
  bool destroysContainer =
    (type == DOOR_TRAP_FIRE || type == DOOR_TRAP_TNT);
  if (destroysContainer) {
    act("$p is destroyed by its own trap!", TRUE, this, obj, 0, TO_ROOM);
    for (StuffIter it = obj->stuff.begin(); it != obj->stuff.end();) {
      TThing* t = *(it++);
      delete t;
    }
  }

  int rc = applyTrapEffect(type, obj->getContainerTrapDam(), obj);

  if (destroysContainer)
    ADD_DELETE(rc, DELETE_ITEM);
  return rc;
}
```

Note: this normalizes container to single-target (was: TNT splashed the room). The room-splash for container TNT is dropped per the "area = source property; container = opener only" decision.

- [ ] **Step 2: Rewrite `triggerPortalTrap`** similarly (portal = single target; TNT destroys the portal):

```cpp
int TBeing::triggerPortalTrap(TPortal* o) {
  act("You hear a strange noise...", TRUE, this, 0, 0, TO_ROOM);
  act("You hear a strange noise...", TRUE, this, 0, 0, TO_CHAR);

  doorTrapT type = o->getPortalTrapType();
  int rc = applyTrapEffect(type, o->getPortalTrapDam(), o);

  if (type == DOOR_TRAP_TNT) {
    act("$p is destroyed by the blast!", TRUE, this, o, 0, TO_ROOM);
    ADD_DELETE(rc, DELETE_ITEM);
  }
  return rc;
}
```

(Confirm callers of `triggerPortalTrap`/`triggerContTrap` handle the `DELETE_THIS | DELETE_ITEM` combinations — they already do per `TOpenContainer::trapMe` and `obj_portal.cc`; no caller change needed.)

- [ ] **Step 3: Build**

Run: `make format FILE=code/code/misc/trap.cc && make 2>&1 | tail -10`
Expected: clean build.

- [ ] **Step 4: Manual verification** — set a fire trap and a poison trap on a chest, open each; confirm fire destroys the chest+contents and damages opener, poison needles the opener. Open a TNT-trapped portal; confirm it's destroyed and the opener is hit.

- [ ] **Step 5: Commit**

```bash
git add code/code/misc/trap.cc
git commit -m "refactor(trap): migrate container and portal triggers to applyTrapEffect"
```

---

## Task 7: Migrate the mine source (`triggerTrap`) — room blast

**Files:**
- Modify: `code/code/misc/trap.cc` (`TBeing::triggerTrap`, `1111-1509`)

**Interfaces:**
- Consumes: `applyTrapEffect`. `o` is the `TTrap` mine; `o->getTrapDamType()`, `o->getTrapDamAmount()`, `o->getTrapEffectType()` (for `TRAP_EFF_ROOM`).

- [ ] **Step 1: Rewrite `triggerTrap`** — decrement charge (unchanged), then blast: the triggerer takes full damage; if `TRAP_EFF_ROOM`, each other awake being in the room takes half. The mine is never deleted (carrier fate = persist; charge already decremented).

```cpp
int TBeing::triggerTrap(TTrap* o) {
  o->setTrapCharges(o->getTrapCharges() - 1);

  doorTrapT type = o->getTrapDamType();
  int dam = o->getTrapDamAmount();

  // bystanders first (half damage), so we don't touch `this` mid-iteration
  if (o->isTrapEffectType(TRAP_EFF_ROOM) && roomp) {
    for (StuffIter it = roomp->stuff.begin(); it != roomp->stuff.end();) {
      TThing* t = *(it++);
      TBeing* tbt = dynamic_cast<TBeing*>(t);
      if (tbt && tbt != this && tbt->desc) {
        int brc = tbt->applyTrapEffect(type, dam / 2, o);
        if (IS_SET_DELETE(brc, DELETE_THIS)) {
          delete tbt;
          tbt = nullptr;
        }
      }
    }
  }

  // the triggerer (full damage)
  int rc = applyTrapEffect(type, dam, o);
  if (IS_SET_DELETE(rc, DELETE_THIS))
    return DELETE_THIS;
  return TRUE;
}
```

This preserves: charge decrement, mine persistence, `TRAP_EFF_ROOM` splash, half-damage bystanders. It normalizes the bystander fraction to ½ for all types (was ⅔ for some status types) — an accepted change.

- [ ] **Step 2: Build**

Run: `make format FILE=code/code/misc/trap.cc && make 2>&1 | tail -10`
Expected: clean build.

- [ ] **Step 3: Manual verification** — drop a TNT mine, walk a second character into the room/over it; confirm the stepper takes full damage, others (if `TRAP_EFF_ROOM`) half, charge drops by 1, mine remains. Step on it 5 times; confirm it stops at 0 charges.

- [ ] **Step 4: Commit**

```bash
git add code/code/misc/trap.cc
git commit -m "refactor(trap): migrate mine trigger (triggerTrap) to applyTrapEffect"
```

---

## Task 8: Migrate the grenade source (`detonateGrenade` + `grenadeHit`) — room blast, everyone full

**Files:**
- Modify: `code/code/obj/obj_trap.cc` (`TTrap::detonateGrenade`, `59-215`)
- Modify: `code/code/misc/trap.cc` (`TBeing::grenadeHit` `3531-3651`, `TMonster::grenadeHit` `3654-3673`)

**Interfaces:**
- Consumes: `applyTrapEffect`. `TMonster::grenadeHit` keeps its `pissOff` behavior.

- [ ] **Step 1: Replace `TBeing::grenadeHit`** body with a single `applyTrapEffect` call (grenade = everyone in room at full damage; carrier = the grenade):

```cpp
int TBeing::grenadeHit(TTrap* o) {
  return applyTrapEffect(o->getTrapDamType(), o->getTrapDamAmount(), o);
}
```

Keep `TMonster::grenadeHit` as-is except it already calls `TBeing::grenadeHit` then `pissOff`s the puller — no change needed beyond confirming it still compiles. `TObj::grenadeHit` (flavor-only on objects) is unchanged.

- [ ] **Step 2: Simplify `detonateGrenade`'s per-type intro switch** (`obj_trap.cc:127-183`) into the shared source-intro. Replace the ~12-case message switch with a single generic detonation line (the per-victim flavor now comes from `applyTrapEffect` when `grenadeHit` runs in the room loop at line ~204):

```cpp
  act("$n detonates with a deafening blast!", FALSE, this, 0, 0, TO_ROOM);
```

Leave the rest of `detonateGrenade` (arming-stage countdown, room relocation, adjacent-room "KA-BOOM", the `roomp->stuff` loop calling `grenadeHit`, the `return DELETE_THIS`) unchanged.

- [ ] **Step 3: Build**

Run: `make format FILE=code/code/obj/obj_trap.cc && make format FILE=code/code/misc/trap.cc && make 2>&1 | tail -10`
Expected: clean build.

- [ ] **Step 4: Manual verification** — arm and throw a TNT grenade into an occupied room; confirm the detonation message, everyone in the room takes damage, the grenade object is destroyed, and a mob targets the thrower (`pissOff`).

- [ ] **Step 5: Commit**

```bash
git add code/code/obj/obj_trap.cc code/code/misc/trap.cc
git commit -m "refactor(trap): migrate grenade detonation/grenadeHit to applyTrapEffect"
```

---

## Task 9: Migrate the door source (`triggerDoorTrap`) — opener + far side

**Files:**
- Modify: `code/code/misc/trap.cc` (`TBeing::triggerDoorTrap`, `911-984`)

**Interfaces:**
- Consumes: `applyTrapEffect`. Door uses `exitp->trap_info` (type) and `exitp->trap_dam` (N, rolled via `dice`).

- [ ] **Step 1: Rewrite `triggerDoorTrap`** — flag clearing (both sides, unchanged) + intro + opener (full) + same-room bystanders (half) + far-side room occupants (half). TNT additionally destroys the door.

```cpp
int TBeing::triggerDoorTrap(dirTypeT door) {
  roomDirData* exitp = exitDir(door);
  int dam = dice(exitp->trap_dam, 8);
  doorTrapT type = (doorTrapT)exitp->trap_info;

  // clear the trapped flag on both sides (unchanged)
  REMOVE_BIT(exitp->condition, EXIT_TRAPPED);
  TRoom* far = real_roomp(exitp->to_room);
  roomDirData* back = far ? far->dir_option[rev_dir(door)] : nullptr;
  if (back)
    REMOVE_BIT(back->condition, EXIT_TRAPPED);

  act("You hear a strange noise...", TRUE, this, 0, 0, TO_ROOM);
  act("You hear a strange noise...", TRUE, this, 0, 0, TO_CHAR);

  if (type == DOOR_TRAP_TNT)
    exitp->destroyDoor(door, in_room);

  // same-room bystanders (half)
  if (roomp) {
    for (StuffIter it = roomp->stuff.begin(); it != roomp->stuff.end();) {
      TThing* t = *(it++);
      TBeing* tbt = dynamic_cast<TBeing*>(t);
      if (tbt && tbt != this && tbt->desc) {
        int brc = tbt->applyTrapEffect(type, dam / 2, nullptr);
        if (IS_SET_DELETE(brc, DELETE_THIS)) {
          delete tbt;
          tbt = nullptr;
        }
      }
    }
  }

  // far-side room occupants (half)
  if (far) {
    for (StuffIter it = far->stuff.begin(); it != far->stuff.end();) {
      TThing* t = *(it++);
      TBeing* tbt = dynamic_cast<TBeing*>(t);
      if (tbt && tbt->desc) {
        int brc = tbt->applyTrapEffect(type, dam / 2, nullptr);
        if (IS_SET_DELETE(brc, DELETE_THIS)) {
          delete tbt;
          tbt = nullptr;
        }
      }
    }
  }

  // the opener (full)
  int rc = applyTrapEffect(type, dam, nullptr);
  if (IS_SET_DELETE(rc, DELETE_THIS))
    return DELETE_THIS;
  return FALSE;
}
```

This normalizes: every door type now splashes the room + far side (was only TNT/frost/energy/acid), at a uniform ½ (was ¼/⅓ for far side). Both are accepted, documented normalizations. The `trapDoorPierceDamage`/`HammerDamage`/`SlashDamage`/`FrostDamage`/`EnergyDamage`/`FireDamage`/`AcidDamage`/`TntDamage` helpers become dead — removed in Task 11.

- [ ] **Step 2: Build**

Run: `make format FILE=code/code/misc/trap.cc && make 2>&1 | tail -10`
Expected: clean build (the `trapDoor*Damage` helpers still defined but now unused — fine until cleanup).

- [ ] **Step 3: Manual verification** — trap a closed door with frost both directions; open it; confirm opener takes full, anyone in the room and the adjacent room take half. Trap with TNT; confirm the door is destroyed.

- [ ] **Step 4: Commit**

```bash
git add code/code/misc/trap.cc
git commit -m "refactor(trap): migrate door trigger to applyTrapEffect (opener + room + far side)"
```

---

## Task 10: Collapse `goofUpTrap` 4 → 1

**Files:**
- Modify: `code/code/misc/trap.cc` (`TBeing::goofUpTrap`, `1940-2521`)

**Interfaces:**
- Consumes: `getTrapDam`, `applyTrapEffect`, `hasTrapComps`.

- [ ] **Step 1: Rewrite `goofUpTrap`** — one body for all targets: delete components, compute reduced self-damage, emit the "you slip up" intro, apply the effect to self (`this` = the setter = the victim here):

```cpp
int TBeing::goofUpTrap(doorTrapT trap_type, trap_targ_t goof_type) {
  // consume the components regardless of target
  const char* comps = task->orig_arg;
  char buf1[256], buf2[256];
  if (goof_type == TRAP_TARG_DOOR) {
    half_chop(task->orig_arg, buf1, buf2);
    comps = buf2;
  }
  hasTrapComps(comps, goof_type, -1);

  int dam = dice(getTrapDam(goof_type, trap_type), 8) / 3;

  act("You slip up, and your trap goes off in your face!", FALSE, this, 0, 0,
    TO_CHAR);
  act("$n slips up, and $s trap goes off in $s face!", FALSE, this, 0, 0,
    TO_ROOM);

  int rc = applyTrapEffect(trap_type, dam, task->obj);
  if (IS_SET_DELETE(rc, DELETE_THIS))
    return DELETE_THIS;
  return FALSE;
}
```

Note normalization: goof damage is now `getTrapDam/3` uniformly (door/cont were already `/3`; mine/grenade were full — accepted change). The bespoke per-type "you knick yourself"/"you spill the acid" lines collapse to one intro + the standard `applyTrapEffect` victim line.

- [ ] **Step 2: Build**

Run: `make format FILE=code/code/misc/trap.cc && make 2>&1 | tail -10`
Expected: clean build.

- [ ] **Step 3: Manual verification** — set a trap with a low skill so it fails (`goofUpTrap`); confirm the setter is hit by the right effect at reduced damage, components consumed.

- [ ] **Step 4: Commit**

```bash
git add code/code/misc/trap.cc
git commit -m "refactor(trap): collapse goofUpTrap 4 branches into one"
```

---

## Task 11: Remove dead code

**Files:**
- Modify: `code/code/misc/trap.cc` (delete `getDoorTrapDam`, `getContainerTrapDam`, `getMineTrapDam`, `getGrenadeTrapDam`, `getArrowTrapDam`, `getDoorTrapLearn`, `getContainerTrapLearn`, `getMineTrapLearn`, `getGrenadeTrapLearn`, `getArrowTrapLearn`, and `trapDoorTntDamage`/`trapDoorPierceDamage`/`trapDoorHammerDamage`/`trapDoorSlashDamage`/`trapDoorFrostDamage`/`trapDoorEnergyDamage`/`trapDoorFireDamage`/`trapDoorAcidDamage`)
- Modify: `code/code/misc/being.h` (remove their declarations)

- [ ] **Step 1: Confirm each is unused**

Run for each symbol: `grep -rn "getMineTrapDam\|trapDoorTntDamage\| ...each name... " code/code/`
Expected: only the definition (and `being.h` declaration) remain — no call sites.

- [ ] **Step 2: Delete the definitions and declarations.**

- [ ] **Step 3: Build**

Run: `make 2>&1 | tail -15`
Expected: clean build, no "undefined reference" and no "defined but not used" warnings.

- [ ] **Step 4: Run unit tests**

Run: `make test 2>&1 | tail -10`
Expected: `test_trap_dam` PASS.

- [ ] **Step 5: Commit**

```bash
git add code/code/misc/trap.cc code/code/misc/being.h
git commit -m "refactor(trap): remove now-dead per-source trap damage/learn/door helpers"
```

---

## Task 12: Full behavioral matrix verification + doc update

**Files:**
- Modify: `docs/systems/important/trap-system.md` (update to describe the new structure)

- [ ] **Step 1: Run the trap matrix on a server.** For each source × a representative set of types, set/place and trigger; record actual messages, damage, area, carrier fate against the design's normalized expectations:

| Source | fire | tnt | poison | sleep | teleport | spike/blade | frost |
|---|---|---|---|---|---|---|---|
| door | | | | | | | |
| container | | | | | | | |
| portal (OEdit) | | | | | | | |
| mine | | | | | | | |
| grenade | | | | | | | |
| arrow | | | | | | | |

Confirm: door/mine/grenade splash per the area table; container/portal/arrow single-target; charges decrement on mines; grenade and TNT-container/portal destroyed; door TNT destroys the door.

- [ ] **Step 2: Update `trap-system.md`** — replace the "six near-identical trigger switches" description with the new three-layer structure; move the resolved items out of "Disentanglement notes"; note the normalized area/fraction/message rules as current behavior. Leave the still-open threads (storage unification, skills, DISC mismatch, attribution wiring) listed.

- [ ] **Step 3: Commit**

```bash
git add docs/systems/important/trap-system.md
git commit -m "docs(trap): update reference to the per-type effect structure"
```

---

## Self-review notes

- **Spec coverage:** layers 1–3 → Tasks 3,4,5–9; per-source area table → Tasks 5–9 (arrow/cont/portal single, mine/grenade/door splash); damage share (full/half, grenade-everyone-full) → Tasks 7,8,9; message normalization (victim line + per-source intro) → Task 4 + each migration; getTrapDam/Learn collapse → Tasks 1,2; goofUpTrap collapse → Task 10; data tables → Tasks 1,4; scope-out items untouched. ✔
- **Setter seam** present in `dealTrapDamage` (Task 3), threaded through `applyTrapEffect` (Task 4); not wired (all call sites pass no setter). ✔
- **`trapPoison` dead-branch bug:** deliberately NOT touched here (status helpers reused as-is); remains flagged in the spec as a separate decision. ✔
- **Type consistency:** `applyTrapEffect(doorTrapT,int,TThing*,TBeing*)`, `dealTrapDamage(spellNumT,int,TThing*,TBeing*)`, `getTrapDam(trap_targ_t,doorTrapT)`, `getTrapLearn(trap_targ_t)` used consistently across tasks. ✔
- **Open verification risk:** the standardized victim messages in Task 4 are proposed wordings — confirm against current text during review; they are player-facing and the "normalize" decision permits changes, but keep them recognizable.
