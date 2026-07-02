# Trap Damage-Type Specialties — Design

**Date:** 2026-07-01
**Branch:** `disentangle-set-reclaim-comps-specs`
**Status:** Design — awaiting review

## Goal

Give each *physical* trap damage type its own distinctive effect ("specialty")
**on top of** the raw HP damage it already deals — e.g. a spike trap lodges an
iron spike in a limb and starts it bleeding, a frost trap embeds an icicle, a
pebble trap bruises. Build these on a small set of **reusable, trap-agnostic
factory helpers** so the same building blocks can serve non-trap code later
(weapon procs like `bloodspike`, etc.).

The first specialty to implement is **spike**; the design also establishes the
pattern the remaining physical types follow.

## Background — current state

Trap effects funnel through one unified method:

```cpp
int TBeing::applyTrapEffect(doorTrapT type, int trapPower, TThing* carrier,
                            TBeing* setter, int denom);
```

Its body is a `switch (type)`. Today each physical case is only flavor text over
a generic hit — e.g.:

```cpp
case DOOR_TRAP_SPIKE:
    act("You are impaled by the spikes!" ...);
    rc = dealTrapDamage(DAMAGE_TRAP_PIERCE, dam, carrier, setter);
    ...
```

`dam = dice(trapPower, 8) / denom` (an agility "roll with it" may halve it).
There is no limb, no embedded object, no bleed — "impaled by spikes" is pure
text over pool-HP damage.

The **affliction** types already show the pattern we want to follow: their cases
call out to dedicated functions —

```cpp
virtual int  trapSleep(int);
virtual void trapPoison(int);
virtual void trapDisease(int);
virtual int  trapTeleport(int);
```

(declared in `being.h`, defined together in `trap.cc`). Return type is `int`
when a path can return `DELETE_THIS`, `void` otherwise.

## Architecture — two layers

**Layer 1 — trap-specific** (in `trap.cc`, mirrors the existing affliction
functions):
- New per-type methods `TBeing::trapSpike(int)`, `trapFrost(int)`,
  `trapPebble(int)`, `trapBlade(int)`, … declared beside `trapDisease` /
  `trapTeleport` in `being.h`.
- Each `applyTrapEffect` case calls its specialty **after** the damage, not
  instead of it.

**Layer 2 — reusable, trap-agnostic factories** (usable anywhere; no trap
knowledge):
- `createSplinter(material, level, spiked)` — manufactures a material-flavored
  shard/spike weapon object.
- `TBeing::forEachRandomLimb(count, action)` — walks `count` random limbs,
  running `action` on each.
- `TBeing::embedInLimb(shard, limb, power)` — lodges an object in a limb with
  immunity + stat checks, gear damage, and bleeding.

`trapSpike` is where the layers meet: it calls `forEachRandomLimb` with a
per-limb action that `createSplinter`s a spike and `embedInLimb`s it.

## Layer 2 — the reusable factories

### `createSplinter(int material, int level, bool spiked) → TObj*`

Home: `being.h` (free function; needs no `TBeing`). Manufactures an embeddable
shard/spike weapon.

Steps:
1. `TObj* o = read_object(937, VIRTUAL);` — base "slender spike" weapon
   (`TGenWeapon`). Guard: if the `dynamic_cast<TGenWeapon*>` fails (937 ever
   misconfigured), log via `vlogf` and return the object unmodified.
2. `o->swapToStrung();` then set `name` / `shortDescr` / `descr` from the
   material's flavor category (table below) + material name.
3. `o->setMaterial(material);`
4. `hard = material_nums[material].hardness;`
   `w->setMaxSharp(hard); w->setCurSharp(hard);` (sharpness range is 0–100, so
   hardness maps in directly).
5. Damage: `dam = max(1, level * hard / 100);`
   `w->setWeapDamLvl(dam * 4);` (weapon level is stored at 4× displayed) +
   a proportional `w->setWeapDamDev(...)`.
6. Weapon damage type from the flavor category (pierce for spikes/shards).
7. `if (spiked) o->setObjStat(ITEM_SPIKED);` (flag already exists — `isSpiked()`,
   used in `obj_general_weapon.cc`; its gameplay *interaction* is a separate
   follow-up).
8. `return o;`

**Flavor table** — keyed by *material category*, not individual material (an
explicit classifier; `MAT_ICE` isn't grouped usefully by the crafting tables).
Each category → `{adjective, plain noun, spiked noun}`:

| Category | Materials | plain → spiked noun | example short desc |
|----------|-----------|---------------------|--------------------|
| metal   | iron, steel, mithril, brass          | sliver → **spike**  | "an iron spike" |
| ice     | ice                                  | shard → **icicle**  | "a jagged ice shard" |
| wood    | wood                                 | splinter → **stake**| "a wooden splinter" |
| stone   | stone, granite, marble, obsidian     | shard → **spike**   | "an obsidian shard" |
| crystal | crystal, quartz, diamond, glass      | shard → **spike**   | "a glittering quartz shard" |
| bone    | bone, ivory                          | splinter → **spike**| "a bone splinter" |
| *fallback* | any other                         | shard → spike       | — |

Damage and sharpness come from the material's `hardness`, so an iron spike
(hardness 60) meaningfully out-hits an ice shard (hardness 55) at the same
trap level.

### `TBeing::forEachRandomLimb(int count, action) → int`

Home: `being.h` (method on the victim whose limbs are targeted).

Behavior: attempts `count` randomly chosen limb slots (via `pickRandomLimb`);
for each slot the being actually `hasPart(limb)`, invokes `action(limb)`. The
action does its own *effect-specific* checks (already-stuck, immunity,
bleedable) and returns whether it acted. Returns the number of limbs acted on.

This is deliberately the *only* universal check the walker does (`hasPart`);
everything effect-specific lives in the action, keeping the walker reusable by
the ~6 other procs that currently copy the `for (slot = pickRandomLimb(); …)`
loop.

The `action` is passed as a small inline block (lambda). The loop supplies the
current `limb` to that block on each call — the block does not fetch the limb
itself.

### `TBeing::embedInLimb(TObj* shard, wearSlotT limb, int power) → bool`

Home: `being.h`. Lodges `shard` in `limb`, applying checks. Returns whether it
lodged. Reusable by `bloodspike` and similar procs.

Sequence (adapted from the historical `trapSpike` on `vasco-thief-overhaul-pt1`):
1. **Already-stuck check:** if `getStuckIn(limb)`, the limb is occupied — clean
   up the shard (`delete`) and return `false`.
2. **Pierce-immunity check:** lodging an object in a limb *is* a puncture, so it
   is always governed by `IMMUNE_PIERCE` regardless of the shard's material —
   scale the limb damage by `getImmunity(IMMUNE_PIERCE) / 100` per limb; full
   pierce immunity ≈ no bite (return `false`). This matches the historical
   `trapSpike` behavior.
3. **CON resist check:** a Constitution-based roll — dense flesh/armor keeps the
   shard from sinking in; on success the shard glances off (return `false`).
   Independent of the agility "roll with it" that already halves total damage
   upstream in `applyTrapEffect`.
4. **Worn-gear damage:** if `equipment[limb]` exists, `addToStructPoints(-…)`
   with "a spike tears through your <item>" messages.
5. **Lodge:** `stickIn(shard, limb)`.
6. **Limb health:** `addCurLimbHealth(limb, -limbDam * k)`.
7. **Bleed:** if `!isTough() && !isImmune(IMMUNE_BLEED, limb)`,
   `rawBleed(limb, …)` with "blood flows" messages.
8. `return true;`

## Layer 1 — the specialties

### `TBeing::trapSpike(int amt)`

```cpp
void TBeing::trapSpike(int amt) {
    forEachRandomLimb(amt / 10, [&](wearSlotT limb) {
        TObj* shard = createSplinter(MAT_IRON, amt, /*spiked*/ true);
        return embedInLimb(shard, limb, amt);
    });
}
```

- Limb count scales with the hit (`amt / 10`).
- Material is the constant `MAT_IRON` — a spike trap *is* iron; there is no
  trap object to query, and `trapSpike` doesn't receive one.
- A fresh shard is created **inside** the loop so each limb gets its own object
  (one object cannot be stuck in two limbs).

Return type: `void`, matching `trapDisease`/`trapPoison` — embedding + bleeding
are status effects layered on damage already dealt, and cannot themselves cause
immediate death. (Change to `int` only if a future specialty needs to signal
`DELETE_THIS`.)

### `applyTrapEffect` integration

```cpp
case DOOR_TRAP_SPIKE:
    act("You are impaled by the spikes!" ...);
    act("$n is pierced by the spikes." ...);
    rc = dealTrapDamage(DAMAGE_TRAP_PIERCE, dam, carrier, setter);  // raw HP
    if (IS_SET_DELETE(rc, DELETE_THIS))
        return DELETE_THIS;
    trapSpike(dam);                                                 // specialty
    return 0;
```

Perfectly parallel to how `DOOR_TRAP_DISEASE` already calls `trapDisease(dam)`.

### Pattern for the other physical types (follow-ups)

Each is a small function using the same factories with a different
material / effect:

| Type | Specialty function | Material | Per-limb action |
|------|--------------------|----------|-----------------|
| spike  | `trapSpike`   | iron  | embed spiked splinter |
| frost  | `trapFrost`   | ice   | embed icicle (`createSplinter(MAT_ICE, amt/10, false)`) |
| pebble | `trapPebble`  | stone | `rawBruise(limb, …)` (no object) |
| blade  | `trapBlade`   | steel | `rawBleed(limb, …)` (no object) |
| disk   | `trapDisk`    | steel | `rawBleed(limb, …)` |
| bolt   | `trapBolt`    | iron  | embed splinter |

Bruise/bleed effects reuse `forEachRandomLimb` but call the existing
`rawBruise`/`rawBleed` primitives directly instead of `embedInLimb`.

## Files touched

- `being.h` — `createSplinter` as a file-scope free function (needs no
  `TBeing`); `forEachRandomLimb`, `embedInLimb`, and `trapSpike` (+ later
  specialties) as `virtual` `TBeing` methods beside the existing `trap*`
  affliction declarations.
- `trap.cc` — definitions of the specialty functions (beside `trapDisease` et
  al.) and the `createSplinter` / `forEachRandomLimb` / `embedInLimb` bodies (or
  a nearby file); the `case DOOR_TRAP_SPIKE` edit.

## Testing

A focused unit test (the world fixture already loads objects):
- `createSplinter(MAT_IRON, level, true)` and `createSplinter(MAT_ICE, level,
  false)`: assert material set, `curSharp == hardness`, `weapDamLvl` scales with
  `level * hardness`, `ITEM_SPIKED` set iff `spiked`, and the strung short-desc
  contains the expected category noun ("spike" / "shard").
- `forEachRandomLimb(n, …)`: assert the action runs only on limbs the being
  `hasPart`, and the returned count matches limbs acted on.
- `embedInLimb`: assert a second embed into an already-stuck limb returns
  `false` and doesn't leak; assert a bleed starts on success for a
  non-immune limb.

## Scope

**This iteration:** the three factories (`createSplinter`, `forEachRandomLimb`,
`embedInLimb`) + `trapSpike` wired into `applyTrapEffect`.

**Follow-ups (each its own change):**
- The remaining physical specialties (`trapFrost`, `trapPebble`, `trapBlade`,
  `trapDisk`, `trapBolt`, `trapHammer`).
- The `ITEM_SPIKED` gameplay *interaction* (harder to remove, extra bleed on
  removal, etc.).
- Material-from-component: deriving a trap's material from the trap-component it
  was built from (so a steel-component trap out-hits an iron one), building on
  the `TTrapComponent` work from the parent branch. Not plumbed today.

## Resolved decisions

- **`embedInLimb` lodge check** = pierce immunity + CON resist. Lodging is a
  puncture (always `IMMUNE_PIERCE`, matching historical `trapSpike`); the stat
  resist is Constitution-based (hardy flesh/armor resists the shard sinking in),
  independent of the upstream agility "roll with it".
