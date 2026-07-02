# Trap Spike Specialty Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Give the spike trap a real limb-embedding effect (an iron spike lodges in random limbs and bleeds) on top of its raw damage, built on three reusable, trap-agnostic helpers.

**Architecture:** Two layers. Layer 2 is three general helpers — `createSplinter` (material-flavored shard/spike weapon factory), `TBeing::forEachRandomLimb` (limb-iteration walker), `TBeing::embedInLimb` (lodge-with-checks). Layer 1 is `TBeing::trapSpike`, a per-type specialty that composes them, called from the `DOOR_TRAP_SPIKE` case of `applyTrapEffect` after the damage — mirroring how `trapDisease`/`trapTeleport` already work.

**Tech Stack:** C++20, Clang, GoogleTest (`tests/cpp/unit/*_test.cc`, auto-globbed), MariaDB-backed world fixtures.

## Global Constraints

- C++20, modern style only in new/changed lines: `nullptr`/`true`/`false`, `static_cast`, `boost::format`/`act()` — never `sprintf`/`NULL`/`TRUE`/C-casts (a pre-commit hook rejects these).
- Player-facing messages use `act()`, not `sendTo()`.
- Build: `make` (preset dev-clang). Unit tests: `make test`. Format before committing: `make format FILE=<path>`.
- Address ALL compiler/linter diagnostics (including info-level) in touched code.
- Commit messages: NO `Co-Authored-By`/AI-attribution lines.
- **Commits:** this plan lists commit points, but per the repo owner's standing rule, obtain an explicit go-ahead before each commit.
- Reference values (verified): base weapon vnum **937**; `material_nums[mat].hardness` and `.mat_name`; `ITEM_SPIKED = (1<<9)`; `WEAPON_TYPE_STAB` (there is no `WEAPON_TYPE_PIERCE`); `IMMUNE_PIERCE`/`IMMUNE_BLEED` in `immunity.h`; `addToStructPoints(short)`; CON check via `isTough()`.

---

### Task 1: `createSplinter` factory + material flavor table

**Files:**
- Modify: `code/code/misc/being.h` (add free-function declaration near the other free functions at end of file / top, file scope — NOT inside `class TBeing`)
- Modify: `code/code/misc/trap.cc` (add anonymous-namespace flavor helper + `createSplinter` definition; add `#include "materials.h"` and `#include "obj_general_weapon.h"` if not already present)
- Test: `tests/cpp/unit/trap_specialty_test.cc` (create)

**Interfaces:**
- Produces: `TObj* createSplinter(int material, int level, bool spiked);` — file-scope free function. Returns a strung `TGenWeapon` (as `TObj*`): material set, `curSharp == maxSharp == material_nums[material].hardness`, `weapDamLvl == max(1, level*hardness/100)*4`, `ITEM_SPIKED` set iff `spiked`, short-desc built from the material's flavor category.

- [ ] **Step 1: Write the failing test**

Create `tests/cpp/unit/trap_specialty_test.cc`:

```cpp
#include <gtest/gtest.h>
#include "world_fixture.h"
#include "obj_general_weapon.h"
#include "materials.h"
#include "being.h"

class TrapSplinterTest : public WorldFixture {};

TEST_F(TrapSplinterTest, IronSpikeUsesMaterialHardnessAndSpikedFlag) {
  insertTestObj(937, "spike slender", ITEM_WEAPON);

  TObj* shard = createSplinter(MAT_IRON, 100, /*spiked*/ true);
  ASSERT_NE(shard, nullptr);
  auto* w = dynamic_cast<TGenWeapon*>(shard);
  ASSERT_NE(w, nullptr);

  int hard = material_nums[MAT_IRON].hardness;
  EXPECT_EQ(shard->getMaterial(), MAT_IRON);
  EXPECT_EQ(w->getCurSharp(), hard);
  EXPECT_EQ(w->getWeapDamLvl(), std::max(1, 100 * hard / 100) * 4);
  EXPECT_TRUE(shard->isObjStat(ITEM_SPIKED));
  EXPECT_NE(shard->getName().find("spike"), sstring::npos);

  delete shard;
}

TEST_F(TrapSplinterTest, IceShardIsNotSpikedAndReadsAsShard) {
  insertTestObj(937, "spike slender", ITEM_WEAPON);

  TObj* shard = createSplinter(MAT_ICE, 100, /*spiked*/ false);
  ASSERT_NE(shard, nullptr);

  EXPECT_EQ(shard->getMaterial(), MAT_ICE);
  EXPECT_FALSE(shard->isObjStat(ITEM_SPIKED));
  EXPECT_NE(shard->getName().find("shard"), sstring::npos);

  delete shard;
}
```

- [ ] **Step 2: Regenerate CMake (new test file) and run to verify it fails**

Run:
```bash
cmake --preset dev-clang && make test 2>&1 | grep -i splinter
```
Expected: build FAILS — `createSplinter` is not declared.

- [ ] **Step 3: Declare `createSplinter` in `being.h`**

Add at file scope in `code/code/misc/being.h` (near other free-function `extern` declarations, e.g. beside `read_object`; NOT inside the class):

```cpp
// Manufactures a material-flavored shard/spike weapon from base vnum 937,
// scaled by `level`. Sharpness and damage derive from the material's hardness;
// `spiked` sets ITEM_SPIKED. Trap-agnostic — usable by any proc.
extern TObj* createSplinter(int material, int level, bool spiked);
```

- [ ] **Step 4: Define the flavor helper + `createSplinter` in `trap.cc`**

Ensure `#include "materials.h"` and `#include "obj_general_weapon.h"` are present near the top of `code/code/misc/trap.cc`. Then add (an anonymous namespace near the top for the helper; the function can go below it):

```cpp
namespace {
struct SplinterFlavor {
  const char* adjective;   // extra adjective, may be ""
  const char* plainNoun;   // when not spiked
  const char* spikedNoun;  // when spiked
};

SplinterFlavor splinterFlavorFor(int material) {
  switch (material) {
    case MAT_IRON:
    case MAT_STEEL:
    case MAT_MITHRIL:
    case MAT_BRASS:
      return {"", "sliver", "spike"};
    case MAT_ICE:
      return {"jagged", "shard", "icicle"};
    case MAT_WOOD:
      return {"", "splinter", "stake"};
    case MAT_STONE:
    case MAT_GRANITE:
    case MAT_MARBLE:
    case MAT_OBSIDIAN:
      return {"", "shard", "spike"};
    case MAT_CRYSTAL:
    case MAT_QUARTZ:
    case MAT_DIAMOND:
    case MAT_GLASS:
      return {"glittering", "shard", "spike"};
    case MAT_BONE:
    case MAT_IVORY:
      return {"", "splinter", "spike"};
    default:
      return {"jagged", "shard", "spike"};
  }
}

sstring articleFor(const sstring& phrase) {
  if (phrase.empty())
    return "a";
  char c = tolower(phrase[0]);
  return (c == 'a' || c == 'e' || c == 'i' || c == 'o' || c == 'u') ? "an" : "a";
}
}  // namespace

TObj* createSplinter(int material, int level, bool spiked) {
  TObj* o = read_object(937, VIRTUAL);
  if (!o)
    return nullptr;
  auto* w = dynamic_cast<TGenWeapon*>(o);
  if (!w) {
    vlogf(LOG_BUG, "createSplinter: base object 937 is not a weapon");
    return o;
  }

  o->swapToStrung();
  o->setMaterial(static_cast<unsigned short>(material));

  SplinterFlavor flavor = splinterFlavorFor(material);
  sstring matName = material_nums[material].mat_name;
  sstring noun = spiked ? flavor.spikedNoun : flavor.plainNoun;
  sstring adj = flavor.adjective;

  sstring body = adj.empty() ? (matName + " " + noun)
                             : (adj + " " + matName + " " + noun);
  o->name = matName + " " + noun;
  o->shortDescr = articleFor(adj.empty() ? matName : adj) + " " + body;
  o->setDescr(format("%s lies here.") % sstring(o->shortDescr).cap());

  int hard = material_nums[material].hardness;
  w->setMaxSharp(hard);
  w->setCurSharp(hard);

  int dam = std::max(1, level * hard / 100);
  w->setWeapDamLvl(dam * 4);
  w->setWeapDamDev(std::max(1, dam / 2));
  w->setWeaponType(WEAPON_TYPE_STAB);

  if (spiked)
    o->setObjStat(ITEM_SPIKED);

  return o;
}
```

- [ ] **Step 5: Format, build, run the test to verify it passes**

Run:
```bash
make format FILE=code/code/misc/trap.cc
make && make test 2>&1 | grep -iE 'splinter|passed|failed'
```
Expected: both `TrapSplinterTest` cases PASS; full suite still green.

- [ ] **Step 6: Commit** (after go-ahead)

```bash
git add code/code/misc/being.h code/code/misc/trap.cc tests/cpp/unit/trap_specialty_test.cc
git commit -m "Add createSplinter: material-flavored shard/spike weapon factory"
```

---

### Task 2: `TBeing::forEachRandomLimb`

**Files:**
- Modify: `code/code/misc/being.h` (declare method inside `class TBeing`; ensure `#include <functional>` present)
- Modify: `code/code/misc/limbs.cc` (define, next to `pickRandomLimb`)
- Test: `tests/cpp/unit/trap_specialty_test.cc` (append)

**Interfaces:**
- Consumes: `pickRandomLimb(bool)` (limbs.h), `hasPart(wearSlotT)`.
- Produces: `int TBeing::forEachRandomLimb(int count, const std::function<bool(wearSlotT)>& action);` — attempts `count` random limb slots; invokes `action(limb)` only on slots the being `hasPart`; returns the number of invocations where `action` returned `true`.

- [ ] **Step 1: Write the failing test**

Append to `tests/cpp/unit/trap_specialty_test.cc`:

```cpp
#include "game_fixture.h"

class ForEachRandomLimbTest : public GameFixture {};

TEST_F(ForEachRandomLimbTest, OnlyActsOnLimbsTheBeingHas) {
  TestCharacter& tc = makeCharacter("Limbtester");
  std::vector<wearSlotT> seen;

  int acted = tc.ch->forEachRandomLimb(40, [&](wearSlotT limb) {
    seen.push_back(limb);
    return true;
  });

  EXPECT_EQ(acted, static_cast<int>(seen.size()));
  for (wearSlotT limb : seen)
    EXPECT_TRUE(tc.ch->hasPart(limb));
}

TEST_F(ForEachRandomLimbTest, ZeroCountDoesNothing) {
  TestCharacter& tc = makeCharacter("Limbtester2");
  int acted = tc.ch->forEachRandomLimb(0, [](wearSlotT) { return true; });
  EXPECT_EQ(acted, 0);
}
```

- [ ] **Step 2: Build to verify it fails**

Run: `make 2>&1 | grep -i forEachRandomLimb`
Expected: FAIL — `forEachRandomLimb` is not a member of `TBeing`.

- [ ] **Step 3: Declare in `being.h`**

Inside `class TBeing`, near the limb helpers (e.g. beside `hasPart`), and ensure `#include <functional>` is present at the top of the header:

```cpp
    int forEachRandomLimb(int count,
      const std::function<bool(wearSlotT)>& action);
```

- [ ] **Step 4: Define in `limbs.cc`**

Add near `pickRandomLimb`:

```cpp
int TBeing::forEachRandomLimb(int count,
  const std::function<bool(wearSlotT)>& action) {
  int acted = 0;
  for (int i = 0; i < count; ++i) {
    wearSlotT limb = pickRandomLimb();
    if (hasPart(limb) && action(limb))
      ++acted;
  }
  return acted;
}
```

- [ ] **Step 5: Format, build, run tests**

Run:
```bash
make format FILE=code/code/misc/limbs.cc
make && make test 2>&1 | grep -iE 'ForEachRandomLimb|passed|failed'
```
Expected: both `ForEachRandomLimbTest` cases PASS; suite green.

- [ ] **Step 6: Commit** (after go-ahead)

```bash
git add code/code/misc/being.h code/code/misc/limbs.cc tests/cpp/unit/trap_specialty_test.cc
git commit -m "Add TBeing::forEachRandomLimb limb-iteration helper"
```

---

### Task 3: `TBeing::embedInLimb`

**Files:**
- Modify: `code/code/misc/being.h` (declare `virtual bool embedInLimb(...)` in `class TBeing`, beside `trap*` affliction decls ~lines 1147-1152)
- Modify: `code/code/misc/trap.cc` (define, beside `trapDisease`/`trapTeleport`)

**Interfaces:**
- Consumes: `getStuckIn(wearSlotT)`, `getImmunity(IMMUNE_PIERCE)`, `isTough()`, `equipment[limb]`/`operator[]`, `TObj::addToStructPoints(short)`, `stickIn(TThing*, wearSlotT)`, `addCurLimbHealth(wearSlotT,int)`, `isImmune(IMMUNE_BLEED, limb)`, `rawBleed(wearSlotT, int, silentTypeT, checkImmunityT)`, `describeBodySlot(wearSlotT)`.
- Produces: `bool TBeing::embedInLimb(TObj* shard, wearSlotT limb, int power);` — returns `true` if the shard lodged, else deletes the shard and returns `false`.

Note: this touches immunity/bleed/race systems the unit fixtures can't fully stand up (race files), so verification is **build + in-game**, matching how the damage path is functionally tested. No unit test in this task.

- [ ] **Step 1: Declare in `being.h`**

Beside the other `trap*` declarations (~1147-1152):

```cpp
    bool embedInLimb(TObj* shard, wearSlotT limb, int power);
```

- [ ] **Step 2: Define in `trap.cc`**

Beside `trapDisease`/`trapTeleport`:

```cpp
bool TBeing::embedInLimb(TObj* shard, wearSlotT limb, int power) {
  // The limb can only hold one embedded object.
  if (getStuckIn(limb)) {
    delete shard;
    return false;
  }

  // Lodging is a puncture, so pierce immunity gates and scales it.
  int imm = getImmunity(IMMUNE_PIERCE);
  if (imm >= 100) {
    delete shard;
    return false;
  }
  int limbDam = std::max(1, power * (100 - imm) / 100);

  // Hardy flesh/armor can keep the shard from sinking in.
  if (isTough()) {
    act("$p glances off your $o without biting in.", false, this, shard,
      nullptr, TO_CHAR);
    delete shard;
    return false;
  }

  // Damage whatever is worn on the limb.
  if (TObj* worn = dynamic_cast<TObj*>(equipment[limb])) {
    worn->addToStructPoints(static_cast<short>(-limbDam));
    act(format("A spike tears through your %s, damaging it!") % worn->getName(),
      false, this, nullptr, nullptr, TO_CHAR, ANSI_YELLOW);
  }

  stickIn(shard, limb);
  addCurLimbHealth(limb, -limbDam * 5);

  act(format("%s embeds itself in your %s!") %
        sstring(shard->getName()).cap() % describeBodySlot(limb),
    false, this, nullptr, nullptr, TO_CHAR, ANSI_ORANGE);
  act(format("%s embeds itself in $n's %s!") %
        sstring(shard->getName()).cap() % describeBodySlot(limb),
    true, this, nullptr, nullptr, TO_ROOM, ANSI_ORANGE);

  if (!isTough() && !isImmune(IMMUNE_BLEED, limb)) {
    rawBleed(limb, PERMANENT_DURATION, SILENT_YES, CHECK_IMMUNITY_NO);
    act("Blood begins to flow from the wound!", false, this, nullptr, nullptr,
      TO_CHAR, ANSI_RED_BOLD);
  }

  return true;
}
```

Constants used above are all defined: `ANSI_ORANGE`/`ANSI_RED_BOLD`/`ANSI_YELLOW` in `code/code/sys/ansi.h` (the trailing `act()` color arg, as in `spec_objs_weapons.cc`); `CHECK_IMMUNITY_NO` in `enum.h`; `SILENT_YES` and `PERMANENT_DURATION` in the misc headers. If the build reports `ANSI_ORANGE` undefined, add `#include "ansi.h"` to `trap.cc` (it currently includes `being.h` but may not pull in `ansi.h` transitively).

- [ ] **Step 3: Format and build**

Run:
```bash
make format FILE=code/code/misc/trap.cc
make 2>&1 | tail -5
```
Expected: clean build, no diagnostics in `trap.cc`.

- [ ] **Step 4: Run the unit suite (regression check)**

Run: `make test 2>&1 | tail -3`
Expected: 100% pass (no behavior change to existing tests).

- [ ] **Step 5: Commit** (after go-ahead)

```bash
git add code/code/misc/being.h code/code/misc/trap.cc
git commit -m "Add TBeing::embedInLimb: lodge an object in a limb with checks"
```

---

### Task 4: `TBeing::trapSpike` + wire into `applyTrapEffect`

**Files:**
- Modify: `code/code/misc/being.h` (declare `virtual void trapSpike(int);` beside `trapDisease`/`trapPoison` ~1147-1150)
- Modify: `code/code/misc/trap.cc` (define `trapSpike`; edit `case DOOR_TRAP_SPIKE:` in `applyTrapEffect`)

**Interfaces:**
- Consumes: `forEachRandomLimb`, `createSplinter`, `embedInLimb`.
- Produces: `void TBeing::trapSpike(int amt);` — embeds an iron spike into up to `amt/10` random limbs.

Verification is **build + in-game** (needs a live spike trap and a victim with race data).

- [ ] **Step 1: Declare in `being.h`**

Beside the affliction declarations:

```cpp
    virtual void trapSpike(int);
```

- [ ] **Step 2: Define `trapSpike` in `trap.cc`**

Beside `trapDisease`:

```cpp
void TBeing::trapSpike(int amt) {
  forEachRandomLimb(amt / 10, [this, amt](wearSlotT limb) -> bool {
    return embedInLimb(createSplinter(MAT_IRON, amt, /*spiked*/ true), limb,
      amt);
  });
}
```

- [ ] **Step 3: Wire into the `DOOR_TRAP_SPIKE` case**

In `TBeing::applyTrapEffect`, replace the body of `case DOOR_TRAP_SPIKE:` so the specialty runs after the raw damage:

```cpp
    case DOOR_TRAP_SPIKE:
      act("You are impaled by the spikes!", false, this, nullptr, nullptr,
        TO_CHAR);
      act("$n is pierced by the spikes.", false, this, nullptr, nullptr,
        TO_ROOM);
      rc = dealTrapDamage(DAMAGE_TRAP_PIERCE, dam, carrier, setter);
      if (IS_SET_DELETE(rc, DELETE_THIS))
        return DELETE_THIS;
      trapSpike(dam);
      return 0;
```

- [ ] **Step 4: Format, build, run the unit suite**

Run:
```bash
make format FILE=code/code/misc/trap.cc
make && make test 2>&1 | tail -3
```
Expected: clean build, 100% unit tests pass.

- [ ] **Step 5: In-game functional verification**

Rebuild + restart the server. As a builder/immortal, set a spike trap and trigger it on a test victim (or self). Confirm:
- The raw pierce damage still lands (HP drops).
- One or more messages: "a[n] iron spike embeds itself in your <limb>!"
- Bleeding starts on a non-tough victim.
- Examining the victim / limb shows an embedded spike object.
- No crash; immortals still take no trap damage.

- [ ] **Step 6: Commit** (after go-ahead)

```bash
git add code/code/misc/being.h code/code/misc/trap.cc
git commit -m "Give spike traps a limb-embedding specialty on top of damage"
```

---

## Notes / follow-ups (out of scope here)

- Remaining physical specialties: `trapFrost` (`createSplinter(MAT_ICE, amt/10, false)`), `trapPebble` (`rawBruise`), `trapBlade`/`trapDisk` (`rawBleed`), `trapBolt`. Each is a small function reusing these helpers.
- `ITEM_SPIKED` gameplay *interaction* (harder to remove, extra bleed on removal).
- Material-from-component: derive a trap's material from the `TTrapComponent` it was built from, so a steel trap out-hits an iron one.
