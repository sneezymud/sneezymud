# Trap Damage Setter Attribution Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Credit the player who set a trap with the experience/kill credit when that trap deals damage, for the five object-carried trap types (mine, grenade, container, arrow, portal).

**Architecture:** The credit plumbing already exists — `applyTrapEffect` forwards a `setter` to `dealTrapDamage`, which routes to `setter->applyDamage(...)`. Today every trigger passes `nullptr`. We add a resolver that reads the setter's stored name off the trap carrier, a writer that stamps that name when the trap is armed, a required DELETE-flag translation fix in `dealTrapDamage`, and wire the resolved setter through the five trigger sites. Traps with no recorded setter (all world/mob-loaded traps) resolve to `nullptr` and ride the existing unattributed `objDamage` fallback unchanged.

**Tech Stack:** C++20, GoogleTest (`tests/cpp/unit/`), CMake/Ninja, `game_fixture.h` for lightweight character/room/object construction without a DB.

## Global Constraints

- C++20; modern forms in new/modified code (`nullptr`, `static_cast`, `true`/`false`).
- Setter is stored as a NAME in an object ex-description and resolved at trigger time via `get_char(name, EXACT_YES)`; never store a `TBeing*`.
- Setter ex-desc keyword is `TRAP_EX_DESC` (`"__trap_setter"`); grenades keep `GRENADE_EX_DESC` (`"__grenade_puller"`) and the resolver reads both.
- DELETE-flag discipline: `applyDamage` reports the victim's death as `DELETE_VICT`; `dealTrapDamage`'s `this` IS the victim, so it must return `DELETE_THIS`. Use `IS_SET_DELETE`, never `IS_SET`, for DELETE flags.
- Build: `make` (preset dev-clang). Tests: `make test`. Format touched files with `make format FILE=<path>` before committing.
- Branch: work continues on `disentangle-set-reclaim`. Commit per task; do not push.

---

### Task 1: `trapSetter` resolver

**Files:**
- Modify: `code/code/misc/trap.h` (add declaration near the other trap free functions)
- Modify: `code/code/misc/trap.cc` (add definition near `describeTrapToLooker`)
- Test: `tests/cpp/unit/trap_dam_test.cc`

**Interfaces:**
- Produces: `TBeing* trapSetter(const TThing* carrier);` — returns the live setter named in the carrier's `TRAP_EX_DESC` (or `GRENADE_EX_DESC`) ex-description, or `nullptr` if absent/unresolvable.

- [ ] **Step 1: Write the failing test**

Add to `tests/cpp/unit/trap_dam_test.cc`:

```cpp
#include "game_fixture.h"
#include "handler.h"   // get_char, TRAP_EX_DESC via trap.h

class TrapSetterResolve : public GameFixture {};

TEST_F(TrapSetterResolve, ResolvesStoredNameToLiveBeing) {
  TRoom& room = makeRoom(49960);
  TestCharacter& setter = makeCharacter("Settername");
  placeInRoom(setter, room);

  TChest* carrier = makeContainer();
  auto* ed = new extraDescription();
  ed->next = carrier->ex_description;
  carrier->ex_description = ed;
  ed->keyword = TRAP_EX_DESC;
  ed->description = "Settername";

  EXPECT_EQ(trapSetter(carrier), setter.ch);

  delete carrier;
}

TEST_F(TrapSetterResolve, NullWhenNoSetterRecorded) {
  TChest* carrier = makeContainer();
  EXPECT_EQ(trapSetter(carrier), nullptr);
  delete carrier;
}

TEST_F(TrapSetterResolve, NullWhenNameUnresolvable) {
  TChest* carrier = makeContainer();
  auto* ed = new extraDescription();
  ed->next = carrier->ex_description;
  carrier->ex_description = ed;
  ed->keyword = TRAP_EX_DESC;
  ed->description = "Nobodyhere";
  EXPECT_EQ(trapSetter(carrier), nullptr);
  delete carrier;
}
```

(If `GameFixture` is named differently, match the class name used by `find_component_test.cc`, which includes `game_fixture.h`.)

- [ ] **Step 2: Run test to verify it fails**

Run: `make test` (or `cd build/dev-clang && ctest -R TrapSetterResolve`)
Expected: FAIL — `trapSetter` undeclared / link error.

- [ ] **Step 3: Add the declaration**

In `code/code/misc/trap.h`, alongside the other free-function declarations (near `parseTrapType`):

```cpp
// Resolve a trap's recorded setter to a live, creditable being, or nullptr.
// nullptr for any trap with no recorded setter (all world/mob-loaded traps),
// which routes its damage to the unattributed objDamage() fallback.
TBeing* trapSetter(const TThing* carrier);
```

- [ ] **Step 4: Add the definition**

In `code/code/misc/trap.cc`, near `describeTrapToLooker` (the existing free function around line 132):

```cpp
TBeing* trapSetter(const TThing* carrier) {
  if (!carrier || !carrier->ex_description)
    return nullptr;
  const char* name = carrier->ex_description->findExtraDesc(TRAP_EX_DESC);
  if (!name)
    name = carrier->ex_description->findExtraDesc(GRENADE_EX_DESC);
  if (!name)
    return nullptr;
  return get_char(name, EXACT_YES);
}
```

Ensure `trap.cc` includes `handler.h` (for `get_char`); add it if missing.

- [ ] **Step 5: Run the tests to verify they pass**

Run: `make test`
Expected: PASS — all three `TrapSetterResolve` cases.

- [ ] **Step 6: Format and commit**

```bash
make format FILE=code/code/misc/trap.cc
make format FILE=code/code/misc/trap.h
git add code/code/misc/trap.h code/code/misc/trap.cc tests/cpp/unit/trap_dam_test.cc
git commit -m "Add trapSetter resolver for trap damage attribution"
```

---

### Task 2: `dealTrapDamage` setter-frame translation fix

**Files:**
- Modify: `code/code/misc/trap.cc` (`TBeing::dealTrapDamage`, ~line 689)
- Test: `tests/cpp/unit/trap_dam_test.cc`

**Interfaces:**
- Consumes: nothing new.
- Produces: `dealTrapDamage`/`applyTrapEffect` now actually credit a non-null `setter` and correctly report victim death as `DELETE_THIS`.

- [ ] **Step 1: Write the failing test**

Add to `tests/cpp/unit/trap_dam_test.cc`. This exercises the public `applyTrapEffect` seam (what the triggers call), non-lethally, asserting the victim took damage when a setter is supplied:

```cpp
class TrapAttribution : public GameFixture {};

TEST_F(TrapAttribution, SetterDamageReachesVictim) {
  TRoom& room = makeRoom(49961);
  TestCharacter& setter = makeCharacter("Attacker");
  TestCharacter& victim = makeCharacter("Victimname");
  placeInRoom(setter, room);
  placeInRoom(victim, room);

  int before = victim.ch->getHit();
  // DOOR_TRAP_BLADE is a physical type → routes through dealTrapDamage.
  // Low power, victim at full health → survives, no DELETE.
  int rc = victim.ch->applyTrapEffect(DOOR_TRAP_BLADE, 1, nullptr, setter.ch);

  EXPECT_FALSE(IS_SET_DELETE(rc, DELETE_THIS));
  EXPECT_LT(victim.ch->getHit(), before);
}
```

- [ ] **Step 2: Run test to verify it fails or is inconclusive, then confirm the fix path**

Run: `make test`
Expected: With the current code the setter branch already calls `applyDamage`, so this may already pass for the non-lethal case — that is fine; its purpose is to lock in attribution behavior. The REQUIRED change in Step 3 is the death-frame translation, which prevents a crash on lethal hits. Proceed to Step 3 regardless.

- [ ] **Step 3: Apply the translation fix**

In `code/code/misc/trap.cc`, replace the body of `TBeing::dealTrapDamage`:

```cpp
int TBeing::dealTrapDamage(spellNumT damageClass, int dam, TThing* carrier,
  TBeing* setter) {
  if (setter && setter != this) {
    int rc = setter->applyDamage(this, dam, damageClass);
    // applyDamage reports the victim's death from the SETTER's frame as
    // DELETE_VICT; here `this` IS that victim, so translate to DELETE_THIS
    // (the convention applyTrapEffect's callers check). Without this, an
    // attributed trap kill slips through unpropagated -> dangling victim.
    return IS_SET_DELETE(rc, DELETE_VICT) ? DELETE_THIS : 0;
  }
  return objDamage(damageClass, dam, carrier);
}
```

- [ ] **Step 4: Run tests to verify they pass**

Run: `make test`
Expected: PASS — `TrapAttribution.SetterDamageReachesVictim` plus all prior tests.

- [ ] **Step 5: Commit**

```bash
make format FILE=code/code/misc/trap.cc
git add code/code/misc/trap.cc tests/cpp/unit/trap_dam_test.cc
git commit -m "Translate applyDamage DELETE_VICT to DELETE_THIS in dealTrapDamage"
```

> **Note on the lethal path:** asserting an actual attributed *kill* returns `DELETE_THIS` requires the victim to die mid-test, which collides with `game_fixture`'s `TestCharacter` cleanup (double-free). Verify the lethal case via functional/manual test on a running server in the final verification step, not here.

---

### Task 3: `recordTrapSetter` writer + DRY refactor of `dropMe`

**Files:**
- Modify: `code/code/misc/trap.h` (declaration)
- Modify: `code/code/misc/trap.cc` (definition)
- Modify: `code/code/misc/inventory.cc` (`TTrap::dropMe`, ~line 246 — call the helper)
- Test: `tests/cpp/unit/trap_dam_test.cc`

**Interfaces:**
- Consumes: `trapSetter` (Task 1).
- Produces: `void recordTrapSetter(TObj* carrier, const TBeing* setter);` — stamps `TRAP_EX_DESC = setter->getName()` onto `carrier`, stringing it first.

- [ ] **Step 1: Write the failing round-trip test**

Add to `tests/cpp/unit/trap_dam_test.cc`:

```cpp
TEST_F(TrapSetterResolve, RecordThenResolveRoundTrips) {
  TRoom& room = makeRoom(49962);
  TestCharacter& setter = makeCharacter("Roundtrip");
  placeInRoom(setter, room);

  TChest* carrier = makeContainer();
  recordTrapSetter(carrier, setter.ch);

  EXPECT_EQ(trapSetter(carrier), setter.ch);

  delete carrier;
}
```

- [ ] **Step 2: Run test to verify it fails**

Run: `make test`
Expected: FAIL — `recordTrapSetter` undeclared.

- [ ] **Step 3: Add declaration and definition**

In `code/code/misc/trap.h`, beside `trapSetter`:

```cpp
// Stamp the setter's name onto a trap carrier so trapSetter() can later
// credit them. Strings the object first (prototype-safe), mirroring dropMe.
void recordTrapSetter(TObj* carrier, const TBeing* setter);
```

In `code/code/misc/trap.cc`, beside `trapSetter`:

```cpp
void recordTrapSetter(TObj* carrier, const TBeing* setter) {
  if (!carrier || !setter)
    return;
  carrier->swapToStrung();
  auto* ed = new extraDescription();
  ed->next = carrier->ex_description;
  carrier->ex_description = ed;
  ed->keyword = TRAP_EX_DESC;
  ed->description = setter->getName();
}
```

- [ ] **Step 4: Refactor `dropMe` to use the helper (DRY)**

In `code/code/misc/inventory.cc`, in `TTrap::dropMe`'s non-grenade branch, replace the inline stamp:

```cpp
    swapToStrung();
    ed = new extraDescription();
    ed->next = ex_description;
    ex_description = ed;
    ed->keyword = TRAP_EX_DESC;
    // Record who set the trap (not the mine's own name) so the trigger can
    // recognize the setter and spare their group.
    ed->description = ch->getName();
```

with:

```cpp
    // Record who set the trap (not the mine's own name) so the trigger can
    // recognize the setter (group-spare + damage attribution).
    recordTrapSetter(this, ch);
```

Remove the now-unused local `ed` declaration in that branch if it becomes unused (the compiler warning will flag it). Ensure `inventory.cc` includes `trap.h`.

- [ ] **Step 5: Run tests to verify they pass**

Run: `make test`
Expected: PASS — `RecordThenResolveRoundTrips` plus all prior tests. Build is warning-clean.

- [ ] **Step 6: Commit**

```bash
make format FILE=code/code/misc/trap.cc
make format FILE=code/code/misc/trap.h
make format FILE=code/code/misc/inventory.cc
git add code/code/misc/trap.h code/code/misc/trap.cc code/code/misc/inventory.cc tests/cpp/unit/trap_dam_test.cc
git commit -m "Add recordTrapSetter writer and route dropMe through it"
```

---

### Task 4: Pass the resolved setter at the five trigger sites

**Files:**
- Modify: `code/code/misc/trap.cc` — `triggerPortalTrap` (~394), `triggerContTrap` (~441), `triggerArrowTrap` (~461), `triggerTrap` mine primary (~679) + splash (~666), `TBeing::grenadeHit` (~1447)

**Interfaces:**
- Consumes: `trapSetter` (Task 1), the fixed `dealTrapDamage` (Task 2).
- Produces: mine and grenade traps (which already store a setter) now credit it end-to-end; container/arrow/portal become credit-ready once Task 5 stores their setter.

- [ ] **Step 1: Wire each site to resolve once and pass through**

`triggerPortalTrap` (carrier `o`):
```cpp
  int rc = applyTrapEffect(type, o->getPortalTrapDam(), o, trapSetter(o));
```

`triggerContTrap` (carrier `obj`):
```cpp
  int rc = applyTrapEffect(type, obj->getContainerTrapDam(), obj, trapSetter(obj));
```

`triggerArrowTrap` (carrier `obj`):
```cpp
  return applyTrapEffect(obj->getTrapDamType(), obj->getTrapLevel(), obj,
    trapSetter(obj));
```

`TBeing::grenadeHit` (carrier `o`):
```cpp
  return applyTrapEffect(o->getTrapDamType(), o->getTrapLevel(), o, trapSetter(o));
```

`triggerTrap` (mine, carrier `o`) — resolve once, use for splash and primary. Add near the top after `int dam = o->getTrapLevel();`:
```cpp
  TBeing* setter = trapSetter(o);
```
Change the splash call (currently `tbt->applyTrapEffect(type, dam, o, nullptr, 2)`):
```cpp
        int brc = tbt->applyTrapEffect(type, dam, o, setter, 2);
```
Change the primary call (currently `int rc = applyTrapEffect(type, dam, o);`):
```cpp
  int rc = applyTrapEffect(type, dam, o, setter);
```

Leave the door splash loops (~497/512) and `goofUpTrap` (~1060) passing `nullptr` — doors are out of scope and goof is self-inflicted.

- [ ] **Step 2: Build and run the full suite**

Run: `make && make test`
Expected: PASS — no regressions; the seam tests from Tasks 1–3 still green.

- [ ] **Step 3: Commit**

```bash
make format FILE=code/code/misc/trap.cc
git add code/code/misc/trap.cc
git commit -m "Pass resolved trap setter through the five object trigger sites"
```

---

### Task 5: Stamp the setter when container / arrow / portal traps are armed

**Files:**
- Modify: `code/code/task/task_trap.cc` — `task_trap_container` (~199), `task_trap_arrow` (~439), `task_trap_portal` (~677/687)

**Interfaces:**
- Consumes: `recordTrapSetter` (Task 3).
- Produces: player-set container/arrow/portal traps now carry a setter and are fully attributed end-to-end.

- [ ] **Step 1: Stamp at container arming**

In `task_trap_container`, immediately after the block that sets the container trap data (`cont->addContainerFlag(CONT_TRAPPED)` … `cont->setContainerTrapDam(...)`):

```cpp
    recordTrapSetter(cont, ch);
```

- [ ] **Step 2: Stamp at arrow arming**

In `task_trap_arrow`, immediately after `arrow->setTrapDamType(doorTrapT(ch->task->status));`:

```cpp
    recordTrapSetter(arrow, ch);
```

- [ ] **Step 3: Stamp at portal arming — BOTH ends**

In `task_trap_portal`, after `portal->addPortalFlag(EXIT_TRAPPED);`:

```cpp
    recordTrapSetter(portal, ch);
```
and inside the partner block, after `partner->addPortalFlag(EXIT_TRAPPED);`:

```cpp
      recordTrapSetter(partner, ch);
```

Ensure `task_trap.cc` includes `trap.h` (it almost certainly already does).

- [ ] **Step 4: Build and run the full suite**

Run: `make && make test`
Expected: PASS, warning-clean.

- [ ] **Step 5: Commit**

```bash
make format FILE=code/code/task/task_trap.cc
git add code/code/task/task_trap.cc
git commit -m "Stamp trap setter when container, arrow, and portal traps are armed"
```

---

### Task 6: Functional verification (running server)

**Files:** none (manual/functional).

- [ ] **Step 1: Verify attributed kill does not crash and credits the setter**

On a dev server (`make test-func` infra or a manual session):
1. A player sets a mine (or grenade/container/arrow/portal) trap.
2. A low-HP mob triggers it and dies.
3. Confirm: no crash (exercises the Task 2 `DELETE_VICT→DELETE_THIS` fix on the real death path), and the setter receives experience/kill credit.

- [ ] **Step 2: Verify world traps are unchanged**

1. A mob-loaded trapped bag (no setter) damages a victim.
2. Confirm damage applies with no attribution and behaves exactly as before (unattributed `objDamage` fallback).

- [ ] **Step 3: Verify the setter need not be present**

1. A player sets a mine, leaves the room, the trap triggers on someone else.
2. Confirm the absent setter is credited with no combat side effects (the `applyDamage` `sameRoom` guard).

---

## Self-Review

**Spec coverage:**
- Resolver (spec §1) → Task 1. ✅
- `dealTrapDamage` fix (spec §2) → Task 2. ✅
- Pass setter at 5 trigger sites (spec §3) → Task 4. ✅
- Setter storage for container/arrow/portal (spec §4) → Task 5; writer + `dropMe` DRY → Task 3. ✅
- Unattributed fallback (spec §5) → no code; verified by Task 1 null cases + Task 6 Step 2. ✅
- Edge cases (self/absent/mob setter, attributed kill) → Task 2 note + Task 6. ✅
- Out of scope (doors, afflictions) → not implemented, recorded in spec follow-up. ✅

**Placeholder scan:** No TBD/TODO; every code step shows complete code. ✅

**Type consistency:** `trapSetter(const TThing*) -> TBeing*` and `recordTrapSetter(TObj*, const TBeing*) -> void` used consistently across Tasks 1, 3, 4, 5. Carrier types: portal/grenade `o`, container/arrow `obj`/`cont`/`arrow` are all `TObj`-derived, valid for both helpers. ✅

**Known test-boundary:** the lethal attributed-kill path is functional-tested (Task 6), not unit-tested, due to fixture death-lifecycle constraints — called out explicitly in Task 2.
