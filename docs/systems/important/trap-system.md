---
title: Trap System (Architecture & Behavior)
description: Comprehensive reference for how traps work end-to-end — the five trap carriers, sixteen damage types, seven thief skills, the set/craft/trigger/disarm lifecycle, damage formulas, and a catalog of the duplication and inconsistencies that make the system a refactoring target.
keywords: [trap, settrap, disarm trap, detect trap, grenade, mine, land mine, door trap, container trap, portal trap, arrow trap, doorTrapT, TTrap, springTrap, triggerTrap, goofUpTrap, hasTrapComps]
category: important
source_files: [code/code/misc/trap.cc, code/code/misc/trap.h, code/code/obj/obj_trap.cc, code/code/obj/obj_trap.h, code/code/obj/obj_open_container.cc, code/code/obj/obj_portal.cc, code/code/obj/obj_arrow.cc, code/code/task/task_trap.cc, code/code/disc/disc_thief_looting.cc]
primary_symbols:
  functions: [doSetTraps, springTrap, triggerTrap, triggerDoorTrap, triggerContTrap, triggerPortalTrap, triggerArrowTrap, grenadeHit, detonateGrenade, goofUpTrap, hasTrapComps, sendTrapMessage, getDoorTrapDam, getContainerTrapDam, getMineTrapDam, getGrenadeTrapDam, getArrowTrapDam, getDoorTrapLearn, trapPoison, trapSleep, trapDisease, trapTeleport, disarmMe, detectMe, disarmTrapDoor, detectTrapDoor, throwGrenade, armGrenade, makeTrapLand, makeTrapGrenade, trapMe, mapFileToDoorTrap, mapDoorTrapToFile, checkForMoveTrap, checkForGetTrap, checkForInsideTrap, checkForAnyTrap]
  classes: [TTrap, TArrow, TOpenContainer, TPortal, roomDirData]
  enums: [doorTrapT, trap_targ_t, TRAP_EFF_MOVE, TRAP_EFF_OBJECT, TRAP_EFF_ROOM, TRAP_EFF_THROW, TRAP_EFF_ARMED1, EXIT_TRAPPED, CONT_TRAPPED, SKILL_SET_TRAP_DOOR, SKILL_SET_TRAP_CONT, SKILL_SET_TRAP_MINE, SKILL_SET_TRAP_GREN, SKILL_SET_TRAP_ARROW, SKILL_DISARM_TRAP, SKILL_DETECT_TRAP]
---

> Companion to `docs/systems/critical/12-trap-mechanics.md`, which covers the crash-safety / DELETE-flag discipline. This document covers the *architecture and gameplay behavior* — what traps are, how they are made, stored, triggered, and removed. It also catalogs the system's heavy internal duplication, which is the motivation for the `trap-system-disentangle` work. Where the two docs disagree on damage numbers, trust this one: the multiplier table in the critical doc does not match the code (see "Damage formula" below).

## What a trap is

A trap is a packaged hazard — a damage type plus a payload of difficulty/charges — attached to one of five **carriers**. There is no single `Trap` abstraction; instead each carrier stores its own trap state in its own fields, and a parallel set of functions interprets that state. The only thing the five carriers truly share is the `doorTrapT` enum (the damage type) and the per-type effect logic, and even that is copy-pasted rather than shared.

The five carriers:

| Carrier | Class / storage | Triggered by | Charges? |
|---|---|---|---|
| **Door / exit** | `roomDirData` fields on the exit | opening / moving through / bashing the door, or trying to set a trap on an already-trapped door | No — one-shot, `EXIT_TRAPPED` flag cleared on trigger |
| **Container** | `TOpenContainer` (`CONT_TRAPPED` flag + bit-packed type/dam) | opening the container; reaching inside (`TRAP_EFF_OBJECT`) | No — `CONT_TRAPPED` cleared, `CONT_EMPTYTRAP` set |
| **Mine (land mine)** | `TTrap` object (`ITEM_TRAP`) lying in a room | walking into the room / leaving in a trapped direction (`TRAP_EFF_MOVE`) | Yes — `trap_charges`, default 5 |
| **Grenade** | `TTrap` object (`ITEM_TRAP`) | thrown, then detonates on a timer | Yes — `trap_charges` = 1, but detonation is gated by arming flags, not charges |
| **Arrow** | `TArrow` object (`ITEM_ARROW`, own `trap_level`/`trap_dam_type` fields) | the arrow striking a target in combat | No — `trap_dam_type` reset to `DOOR_TRAP_NONE` after firing |

Mines and grenades are the same C++ class (`TTrap`); they differ only in their effect flags (`TRAP_EFF_MOVE` vs `TRAP_EFF_THROW`) and charge count. Doors, containers, portals, and arrows each bolt trap fields onto an unrelated class.

> **Portals** (`TPortal`) are a sixth, vestigial carrier: `triggerPortalTrap()` exists and fires on portal open/enter when `EXIT_TRAPPED` is set on `portal_state`, but there is **no player command to set a portal trap** — only OEdit can create one. Treat portals as "trigger-only."

## Damage types (`doorTrapT`)

Defined in `trap.h`. The enum value *is* the on-disk file value (see "Persistence"). `trap_types[]` in `trap.cc` gives the display name, indexed by enum value.

| # | Enum | Display | Damage routed through | Side effect |
|---|---|---|---|---|
| 0 | `DOOR_TRAP_NONE` | None | — | none |
| 1 | `DOOR_TRAP_POISON` | Poison | `trapPoison()` (no `objDamage`) | `AFF_POISON` + disease |
| 2 | `DOOR_TRAP_SPIKE` | Spike | `DAMAGE_TRAP_PIERCE` | — |
| 3 | `DOOR_TRAP_SLEEP` | Sleep | `trapSleep()` (no `objDamage`) | `rawSleep` |
| 4 | `DOOR_TRAP_TNT` | Explosive | `DAMAGE_TRAP_TNT` | area (room + other side), destroys door/container/portal |
| 5 | `DOOR_TRAP_BLADE` | Blade | `DAMAGE_TRAP_SLASH` | — |
| 6 | `DOOR_TRAP_FIRE` | Fire | `DAMAGE_TRAP_FIRE` | `flameEngulfed()` |
| 7 | `DOOR_TRAP_ACID` | Acid | `DAMAGE_TRAP_ACID` | `acidEngulfed()` |
| 8 | `DOOR_TRAP_DISEASE` | Spore | `trapDisease()` (no `objDamage`) | `AFFECT_DISEASE` (flu) |
| 9 | `DOOR_TRAP_HAMMER` | Hammer | `DAMAGE_TRAP_BLUNT` | — |
| 10 | `DOOR_TRAP_FROST` | Frost | `DAMAGE_TRAP_FROST` | `frostEngulfed()` |
| 11 | `DOOR_TRAP_TELEPORT` | Teleport | `trapTeleport()` (no damage) | random relocation |
| 12 | `DOOR_TRAP_ENERGY` | Power | `DAMAGE_TRAP_ENERGY` | — |
| 13 | `DOOR_TRAP_BOLT` | Bolt | `DAMAGE_TRAP_PIERCE` | — |
| 14 | `DOOR_TRAP_DISK` | Disc | `DAMAGE_TRAP_SLASH` | — |
| 15 | `DOOR_TRAP_PEBBLE` | Pebble | `DAMAGE_TRAP_BLUNT` | — |

`objDamage()` routes through the normal damage pipeline (resistances, armor, `reconcileDamage`), so trap damage respects the same defenses as combat.

**Not every type is settable on every carrier.** The player `trap` command hard-codes a different allowed list per carrier (parsed in `doSetTraps()` and `TOpenContainer::trapMe()`):

| Type | Door | Container | Mine | Grenade | Arrow |
|---|---|---|---|---|---|
| poison | ✓ | ✓ | ✓ | ✓ | — |
| spike | ✓ | ✓ | — | — | ✓ |
| blade | ✓ | ✓ | — | — | ✓ |
| hammer | ✓ | — | — | — | — |
| bolt | — | — | ✓ | ✓ | — |
| disk | — | — | ✓ | ✓ | — |
| pebble | — | ✓ | ✓ | ✓ | ✓ |
| fire / explosive / sleep / acid / spore / frost / teleport / power | ✓ | ✓ | ✓ | ✓ | ✓ |

These lists are maintained by hand in five separate `if/else is_abbrev` chains and don't follow an obvious rule (e.g. doors get hammer but not pebble; mines get bolt/disk but not spike/blade). The `trigger*` functions, by contrast, generally handle *all* types they're handed, so a builder placing an "illegal" combination via OEdit will still fire.

## Effect flags (`TRAP_EFF_*`)

Stored in `TTrap::trap_effect` (mines/grenades) and indirectly drive door/container behavior. From `trap.h`:

- `TRAP_EFF_MOVE` (0x1) — trigger on movement (mines)
- `TRAP_EFF_OBJECT` (0x2) — trigger on get/put/reach-inside (container-inside traps)
- `TRAP_EFF_ROOM` (0x4) — area effect: also hit everyone else in the room
- `TRAP_EFF_NORTH … TRAP_EFF_SW` (0x8–0x1000) — per-direction movement filter, indexed via `TrapDir[]`
- `TRAP_EFF_THROW` (0x2000) — throwable (grenades)
- `TRAP_EFF_ARMED1/2/3` (0x4000–0x10000) — grenade arming countdown stages

A freshly built **mine** gets `MOVE | all-direction` flags and 5 charges. A **grenade** gets only `THROW` and 1 charge.

## Lifecycle

### 1. Setting a trap

Command flow: `trap <carrier> <args>` → `CMD_SET_TRAP` (parse.cc) → `TBeing::doSetTraps()` (trap.cc:130).

`doSetTraps` is a big switch on the carrier keyword (`exit`, `container`, `mine`, `grenade`, `arrow`, parsed against `user_trap_types[]`). For each carrier it:

1. Checks the matching skill (`SKILL_SET_TRAP_DOOR/CONT/MINE/GREN/ARROW`).
2. Refuses in peaceful rooms (`checkPeaceful`) — police mobs may attack a trapper, and a goof-up can deal damage.
3. Parses the trap-type keyword into a `doorTrapT` (per-carrier `is_abbrev` chain).
4. Checks `getXxxTrapLearn(type) > 0` (i.e. the skill is known above zero).
5. Checks `hasTrapComps(type, targ, 0)` — the crafting components are in inventory.
6. Starts a `TASK_TRAP_*` task (3-tick build) via `start_task`. The chosen `doorTrapT` is stashed in `task->status`; the door direction (for doors) in `task->flags`; the target object pointer for container/arrow.

Container and arrow route through the object first: container goes via `TOpenContainer::trapMe()` (obj_open_container.cc:408), which validates closeable/closed and *triggers any existing trap* before starting the task.

### 2. The build task

The five task handlers live in `task/task_trap.cc` and are near-identical (`task_trap_door`, `task_trap_container`, `task_trap_mine`, `task_trap_arrow`, `task_trap_grenade`). Each tick:

- Re-validates preconditions (not linkdead, still in room, still has components, position > sitting, still has `DISC_LOOTING`, door still closed, etc.). Failure aborts cleanly.
- Runs `trapGuardCheck()` — any awake police mob that can see you says "Hey! We don't allow any of that nonsense here!", attacks, and disrupts the task.
- Emits a step-of-craft flavor message via `sendTrapMessage(type, targ, step)` (steps 1–4).
- Schedules the next update at `Pulse::MOBACT * (5 + (100 - learning)/3)` — higher skill ⇒ faster build.
- **At the end** (`timeLeft < 0`): rolls `bSuccess(learning, skill)`.
  - **Success** → commit the trap (see below) and consume components (`hasTrapComps(..., -1)`).
  - **Failure** → `goofUpTrap(type, targ)`: the trap goes off in the setter's face (reduced damage, `dice(getXxxTrapDam, 8)/3`), components are still consumed.

How "commit" stores the trap differs per carrier:

- **Door**: `SET_BIT(exitp->condition, EXIT_TRAPPED)`, `exitp->trap_info = type`, `exitp->trap_dam = getDoorTrapDam(type)`. Mirrored onto the reverse exit so it triggers from both sides.
- **Container**: `addContainerFlag(CONT_TRAPPED)`, `setContainerTrapType(type)`, `setContainerTrapDam(getContainerTrapDam)`.
- **Mine**: `read_object(Obj::ST_LANDMINE)` → `makeTrapLand()` sets level=dam, charges=5, type, MOVE+all-dir flags, and drops it into the setter's inventory.
- **Grenade**: `read_object(Obj::ST_GRENADE)` → `makeTrapGrenade()` sets level=dam, charges=1, type, THROW flag.
- **Arrow**: `arrow->setTrapLevel(getArrowTrapDam)`, `arrow->setTrapDamType(type)`.

Mines/grenades/arrows also copy the summed component cost onto `obj_flags.cost`.

### 3. Components (`hasTrapComps`)

`hasTrapComps(type, targ, amt, price)` (trap.cc:2523) maps each (type, carrier) to three or four required component objects (`Obj::ST_*` store items — flint, sulphur, needle, spring, poison vial, nozzle, etc.). Mines additionally need `ST_CASE_MINE`; grenades need `ST_CASE_GRENADE`.

- `amt == 0` → just check presence (returns bool).
- `amt == -1` → "trap finished/goofed, delete the components."
- `price` (out-param) → sum the component costs (used to price the finished mine/grenade/arrow).

The mapping is one giant `is_abbrev` chain; several branches `vlogf(LOG_MISC)` a warning if a type is requested for an unexpected carrier (e.g. a spike trap on a mine), reflecting that the allowed-type lists and the component table were maintained separately.

### 4. Triggering

When the carrier's triggering event happens, control reaches one of the per-carrier trigger functions, each a giant switch over `doorTrapT`:

| Carrier | Trigger function | Event source |
|---|---|---|
| Door | `triggerDoorTrap(door)` (trap.cc:911) | `movement.cc` (open/move), `cmd_doorbash.cc`, `disarmTrapDoor` failure, setting onto an already-trapped door |
| Container | `triggerContTrap(obj)` (trap.cc:598) | `TOpenContainer::openMe`, quiver open, disarm failure, a bomb spec-proc |
| Portal | `triggerPortalTrap(o)` (trap.cc:420) | `TPortal::openMe` / `enterMe` |
| Arrow | `triggerArrowTrap(obj)` (trap.cc:757) | arrow strike in `obj_base_weapon.cc` fire path |
| Mine / inside-container / get / move | `triggerTrap(TTrap*)` (trap.cc:1111) | via `springTrap` from the `*TrapCheck` family |
| Grenade (timer) | `detonateGrenade()` (obj_trap.cc:59) → `grenadeHit()` per target | `procObjDetonateGrenades` scheduler |

#### The `springTrap` to-hit gate

Only the `TTrap`-object paths (mine move, reach-inside, get, generic) call `springTrap()` (trap.cc:116) before `triggerTrap`. It's a single roll:

```
fireperc = 95 + (trapLevel - victimMaxLevel) - dexReaction*5
springs if number(1,100) < fireperc
```

So a stronger trap relative to the victim, and lower dexterity reaction, makes the trap more likely to fire. **Door, container-open, portal, arrow, and grenade triggers skip `springTrap` entirely — they always fire.** The only pre-trigger "escape" for those is a successful detect (which blocks the open action) or the flat **1% fizzle** ("…But nothing happens") in `triggerContTrap` and `triggerArrowTrap`.

#### Charges vs. flags

- `triggerTrap` (mines) **decrements `trap_charges`** each fire; the `*TrapCheck` callers gate on `getTrapCharges() > 0`. A mine fires up to 5 times.
- Doors/containers/portals are one-shot: the trigger clears `EXIT_TRAPPED` / `CONT_TRAPPED` (and the door clears it on both sides) before applying the effect.
- Grenades nominally have 1 charge but `detonateGrenade` doesn't consult charges — see below.

#### Area effects

Three different area mechanisms exist:

- **`TRAP_EFF_ROOM`** (mines via `triggerTrap`): everyone else in the room (`tbt->desc` non-linkdead) takes a reduced share — usually `½` damage, `⅔` for status-based effects.
- **Door area damage**: `trapDoorTntDamage`, `trapDoorFrostDamage`, `trapDoorEnergyDamage`, `trapDoorAcidDamage` hit the whole room *and the room on the other side of the door* (other side at `½` or `¼`/`⅓`). The non-area door types (spike, hammer, blade, fire) only hit the opener.
- **Container/portal/arrow TNT**: hit everyone in the room at `½`.

Immortal filtering across these area loops is **inconsistent**: some loops check `!tbt->isImmortal()` (portal TNT, arrow TNT, door frost/energy), others don't (container TNT, door TNT/acid). This is a known quirk, not a deliberate rule.

#### Status-effect helpers

- `trapPoison(amt)` — applies `SPELL_POISON` (`AFF_POISON`, −20 STR) + a poison disease. Base duration 12 mudhours.
- `trapSleep(amt)` — `rawSleep` for 2 mudhours unless `IMMUNE_SLEEP` or a luck save.
- `trapDisease(amt)` — `AFFECT_DISEASE`/`DISEASE_FLU`, duration scaled by luck + toughness.
- `trapTeleport(amt)` — `genericTeleport` unless `IMMUNE_SUMMON` or a luck save. (Teleport immunity was added recently — commit `0c0682469`.)

### 5. Grenades & mines in detail

- **Arming**: `armGrenade()` (obj_trap.cc:330) sets `TRAP_EFF_ARMED3`, strings the object, stamps the puller's name in a `GRENADE_EX_DESC` extra-desc, adds ~1 round of wait (so you can't spam-throw), and may make seeing mobs flee.
- **Throwing**: `throwGrenade()` (trap.cc:3488) validates a clear path, arms it if not already armed, and lands it in the target room (bounces back from peaceful rooms / underwater).
- **Countdown**: `procObjDetonateGrenades` runs every `Pulse::COMBAT` and calls `detonateGrenade()` on every `TTrap` in the world. Each call steps `ARMED3 → ARMED2 → ARMED1 → boom`, so a thrown grenade detonates a few combat pulses later. Detonation forces the grenade into a room (out of any container/equip), warns adjacent rooms ("KA-BOOM!"), then calls `grenadeHit()` on everything in the room.
- **`grenadeHit`**: `TBeing` version applies the damage-type effect (its own switch, again); `TMonster` override additionally `pissOff`s the mob at the grenade's puller; `TObj` override just prints flavor (objects aren't damaged).

### 6. Detection & disarm

Both are thief skills checked at the point of interaction:

- **Detect** (`SKILL_DETECT_TRAP`): `detectTrapDoor` (reduced rate `bKnown/3 + 1`), `TTrap::detectMe` (heavily reduced `bKnown/10 + 1` — passive room spotting), `TPortal::detectMe`, container detection in `openMe`. A successful detect on a door/portal/container *blocks the open and warns* rather than triggering. `examine`/`look` also surfaces trap info when the skill is known (`show.cc`), and `evaluate` (`TTrap::evaluateMe`) reveals effect/level/charges/damage at increasing skill thresholds.
- **Disarm** (`SKILL_DISARM_TRAP`): `doDisarm` → `disarmTrap` (disc_thief_looting.cc:129) → `disarmTrapObj`/`disarmTrapDoor`.
  - Object: `TTrap::disarmMe` — success sets charges to 0; failure calls `triggerTrap` on the thief.
  - Door: `disarmTrapDoor` — uses `learnedness = min(MAX, 2*bKnown)` (doors are easier than the raw skill suggests); success clears `EXIT_TRAPPED` both sides; failure calls `triggerDoorTrap`.

Mobs disarm in combat via `doDisarm("", &vict)` (this is weapon-disarm, a different code path that happens to share the command).

## Damage formula

Final trap damage is `dice(N, 8)` — **N eight-sided dice**, where N is what `getXxxTrapDam()` returns (clamped 1–50). For doors, N is stored as `trap_dam` and re-rolled at trigger time; for objects, `getTrapDamAmount() = dice(getTrapLevel(), 8)`.

`N` is computed identically across all five carriers except the base term:

```
N = base + skillLevel(setSkill)/divisor      // see per-carrier base below
N = N * learn / 100                           // learn = clamped skill value (0..MAX_SKILL_LEARNEDNESS)
N = N + per-type modifier                      // additive, see table
N = clamp(N, 1, 50)
```

| Carrier | base + skill term | comment range |
|---|---|---|
| Door | `10 + skill/2` | 10–35 |
| Container | `20 + skill/3` | 20–36 |
| Mine | `20 + skill/2` | 20–45 |
| Grenade | `5 + skill/2` | 5–30 (kept low — grenades hit the whole room) |
| Arrow | `5 + skill/2` | copied from grenade |

Per-type additive modifier (identical table in every `getXxxTrapDam`, except arrow omits the poison/hammer rows since they can't be set on arrows):

`TNT +3, disease +3, frost +3, disk +3, energy +5, teleport +5, sleep +1, acid +1, bolt +1, poison −1, blade −3, spike −5, pebble −5, hammer −10`.

There is **no per-type multiplier** anywhere in the code. (The "Damage Multipliers" table in `critical/12-trap-mechanics.md` describing 1.2×/0.8×/etc. is inaccurate — it does not correspond to any code.)

`getXxxTrapLearn()` (five functions) are byte-for-byte identical apart from the skill enum: return the skill value clamped to `MAX_SKILL_LEARNEDNESS`, or 0 if unknown.

## Persistence

| Carrier | Storage | Notes |
|---|---|---|
| Door | `roomDirData::trap_info` (type), `roomDirData::trap_dam` (N), `EXIT_TRAPPED` in `condition` | Loaded in `db.cc` from room-save `arg3`/`arg4`. Stores the **raw enum value** directly. |
| `TTrap` (mine/grenade) | obj values 1–4 = level, effect-flags, dam-type, charges | `assignFourValues`/`getFourValues`; dam-type round-trips through `mapFileToDoorTrap`/`mapDoorTrapToFile`. |
| Container | `CONT_TRAPPED`/`CONT_EMPTYTRAP` flags + dam-type bit-packed (bits 23–31) and dam in obj value 2 | also via `mapFileToDoorTrap`. |
| Portal | `portal_state` (`EXIT_TRAPPED`) + `trap_type` + `trap_damage` | OEdit only. |
| Arrow | `TArrow::trap_level` + `TArrow::trap_dam_type` (own fields) | independent of `TTrap`. |

> **`mapFileToDoorTrap` / `mapDoorTrapToFile` are identity functions.** Every case maps enum value *n* to file value *n* (verified: the two switch statements are exact inverses with matching indices). The indirection is a historical artifact from when the file order differed from the enum order; today it is dead translation. Door traps don't even use it (they store the enum value raw), so the encoding is also inconsistent between carriers.

## Integration points (where trap code is invoked)

- **Movement** (`movement.cc`): `checkForMoveTrap` on leaving a room (self + mount); `detectTrapDoor`/`triggerDoorTrap` on opening/passing a trapped door.
- **Get / put / reach-inside** (`cmd_get.cc`, `inventory.cc`, mob looting in `task_get.cc`, `mobact.cc`): `checkForGetTrap`, `checkForInsideTrap`, `checkForAnyTrap`.
- **Container open** (`obj_open_container.cc`, `obj_quiver.cc`): detect then `triggerContTrap`.
- **Portal open/enter** (`inventory.cc`, `obj_portal.cc`): detect then `triggerPortalTrap`.
- **Doorbash** (`cmd_doorbash.cc`): `triggerDoorTrap`.
- **Arrow fire** (`obj_base_weapon.cc`): `triggerArrowTrap` on hit.
- **Scheduler** (`sys/socket.cc`): `procObjDetonateGrenades` at `Pulse::COMBAT` drives grenade countdowns.
- **Commands** (`parse.cc`): `CMD_SET_TRAP → doSetTraps`, `CMD_DISARM → doDisarm`, throw → `doThrow → throwMe → throwGrenade`.
- **Skills** (`spells.h`, `spell_parser.cc`, `spell_info.cc`, `spell_num.cc`): the seven trap skills. The five `SET_TRAP_*` skills are registered under `DISC_TRAPS` (`spell_info.cc`), but every build-task handler gates on `getDiscipline(DISC_LOOTING)` — a discipline mismatch worth resolving in the refactor. Disarm/detect are the looting side.

## Safety notes

Traps are a memory-safety hotspot because every effect can kill the victim, delete the carrier object, or both, mid-iteration. The DELETE-flag and iterator rules are documented in `critical/12-trap-mechanics.md`; follow them. Key shapes seen throughout this code: `IS_SET_DELETE(rc, DELETE_THIS)` checked immediately after every `objDamage`/`trapSleep`/`triggerTrap`/`disarmMe`; area loops use the `*(it++)` post-increment pattern and `delete tbt` inline on `DELETE_THIS`; `triggerContTrap`/`triggerPortalTrap` return `DELETE_ITEM` (sometimes `| DELETE_THIS`) so callers delete the carrier.

## Disentanglement notes (why this is a refactor target)

The system is a textbook case of structural duplication: **5 carriers × 16 damage types**, expanded by hand into parallel code everywhere instead of being factored through one trap abstraction. Concretely:

1. **Six near-identical trigger switches.** `triggerTrap`, `triggerContTrap`, `triggerPortalTrap`, `triggerArrowTrap`, `grenadeHit`, plus `detonateGrenade`'s message switch — each is a ~15-case switch over `doorTrapT` doing the same "message → objDamage(DAMAGE_TRAP_X) → engulf → DELETE check" per type, differing only in flavor text and which area-spread it does. `triggerDoorTrap` adds a *seventh* via its `trapDoor*Damage` helpers.
2. **Four giant `goofUpTrap` branches** (door/cont/mine/grenade) — again the same per-type switch, at `/2` or full reduced damage.
3. **Five `getXxxTrapDam`** functions that differ only in the base constant and skill enum, sharing an identical per-type modifier table.
4. **Five `getXxxTrapLearn`** functions that are byte-identical except the skill enum.
5. **Five `task_trap_*` handlers** that differ only in skill enum, commit logic, and a couple of strings.
6. **Five per-carrier `is_abbrev` type-parsing chains** (`doSetTraps` ×4 + `TOpenContainer::trapMe`) with hand-maintained, mutually-inconsistent allowed-type lists.

Cross-cutting inconsistencies worth deciding on during the refactor:

- **Five independent storage representations** (door fields, `TTrap` obj-values, container flag+bitpack, portal fields, arrow fields) for the same conceptual `{type, damage, charges, effect}` tuple.
- **`springTrap` only gates `TTrap`-object paths.** Doors/containers/portals/arrows/grenades always fire (modulo detect and the 1% container/arrow fizzle). If a uniform to-hit is desired, this is where the inconsistency lives.
- **Charges vs. one-shot flags vs. arming countdown** are three different "how many times does it fire" mechanisms.
- **Immortal filtering in area loops is applied unevenly** (some loops filter, some don't).
- **`mapFileToDoorTrap`/`mapDoorTrapToFile` are dead identity indirection**, and doors bypass them anyway.
- **Seven separate skills** (`SET_TRAP_DOOR/CONT/MINE/GREN/ARROW` + `DISARM` + `DETECT`) for what is largely one craft.

Latent bugs noticed while documenting (verify before fixing — out of scope unless the refactor touches them):

- **`trapPoison` has unreachable branches.** After `if (isImmortal() || isImmune(IMMUNE_POISON))`, the following `else if (isImmune(IMMUNE_POISON))` clauses can never be true, so the moderate-duration paths are dead — every non-immune victim falls through to the final `else` (`duration *= 4`). Effective behavior: poison is always the longest duration.
- **Grenade detonation ignores `trap_charges`** (set to 1 at build but never checked in `detonateGrenade`); the arming flags are the real gate.
- The arrow set path calls `hasTrapComps(..., TRAP_TARG_CONT, ...)` with a `// TODO:: modify hasTrapComps for arrows` — arrows borrow the container component recipe.
- **The mine setter-bypass is broken.** `moveTrapCheck` (trap.cc:1000) tries to exempt the setter and their group: it reads the extra-desc tag `TRAP_EX_DESC` (`"__trap_setter"`) off the mine and `get_char()`s it as a player name, skipping the trigger if the mover is in that character's group. But `dropMe` (inventory.cc:249) stores `getName()` — the *mine's* name — into that tag instead of `ch->getName()` (the dropper). So `get_char("a land mine")` finds no character and the bypass never fires; the setter is not actually exempt. Contrast `armGrenade`, which correctly stores `ch->getName()` in `GRENADE_EX_DESC` (that's how `TMonster::grenadeHit` angers the right puller). `cmd_get.cc` erases the tag on pickup, so the plumbing is end-to-end — only the stored value is wrong. Note this tag is the *only* place any carrier records who set a trap, which directly bears on the setter-attribution design below.

## Design: setter attribution for trap XP (PROPOSED — not implemented)

> Status: design only as of branch `trap-system-disentangle`. Nothing here is in the code yet. Open questions are explicitly **banked** for a later decision.

**Goal.** Let a trap that damages or kills a victim credit the player who *set* it — XP, trophy, and kill attribution — the way a thrown grenade or a fired bow credits its user. Today a player can build and place traps but gains nothing when they fire.

**Why nothing accrues today.** Every trap effect deals damage as `victim->objDamage(DAMAGE_TRAP_X, amt, trapObj)`:
- `objDamage` runs on the *victim* (`this` is the victim); the third argument is the trap *object*. On death it calls `die(damtype, dynamic_cast<TBeing*>(t))`, and since `t` is a `TObj` the cast is `nullptr` — kills are logged as "killed by a land mine," with no killer.
- `objDamage` also awards no XP at all. Combat XP/trophy live in `applyDamage` (`gainExpPerHit`, `gain_exp` on the kill, trophy bookkeeping); the `objDamage` path never enters that pipeline.

**The model: how bows already solve "actor not in the room."** The bow fire path computes damage and then calls `shooter->applyDamage(target, dam, damtype)` (`obj_base_weapon.cc`, the ranged-hit path). `applyDamage` is written to tolerate the attacker being elsewhere — `damage.cc:377` only *starts a melee fight* `if (sameRoom(*v))`; otherwise it skips engagement but still applies the damage and runs the XP/trophy/kill accrual against `this` (the shooter). So the "ranged damage breaks when the actor isn't in the room" problem is confined to the engage-in-melee step, which `applyDamage` already gates. This is the proven cross-room attribution path.

**Proposed mechanism.** At trigger time, resolve the trap's setter to a live `TBeing` and deal the trap's physical damage via `setter->applyDamage(victim, dam, DAMAGE_TRAP_X)` instead of `victim->objDamage(...)`. The setter then accrues XP/trophy and receives kill credit, and it works whether or not the setter is present — exactly like a sniper a room away. Status effects that don't go through `objDamage` (poison/sleep/disease/teleport) stay as-is. This is a single swap at the damage call; in the disentangled design, where the six near-identical trigger switches collapse into one path, it becomes one choke point: *if a setter resolves, attribute via `applyDamage`; otherwise fall back to today's `objDamage`.*

**Hard requirement: setter-less traps must stay safe.** Most traps in the world have **no player setter** — builder-placed door traps loaded from room files, and OEdit-created container/portal/mine/arrow traps. Setter attribution is therefore an *additive* feature layered on top of an unattributed baseline that must remain the default. The null-setter path has to be the safe, leak-free, crash-free fallback (i.e. behave exactly as today: `victim->objDamage(...)`, no killer, no XP). Whatever stores the setter must encode "no setter" as a first-class value, and resolution must treat unresolved/expired/offline setters as "no setter" rather than dereferencing anything. This is the first correctness gate for the feature, not an edge case.

**What it requires (the real work is identity, not the damage call).** Traps barely record who set them — the only attempt is the (broken, name-based, non-persistent) mine `TRAP_EX_DESC` tag. To attribute reliably:
- Each carrier needs a stored setter identity. Doors and containers persist to disk, so this means a new persisted field; mines/grenades/arrows are objects that can carry a stored setter name or player id.
- A resolution step at trigger time turns that identity into a live `TBeing` (or "no setter").
- The damage swap above consumes the result.

**Banked open questions (decide later):**
- *Offline setter policy.* If the setter is logged off when the trap fires, do they get nothing, or is XP banked/deferred? (A PC across the world earning XP from a mine kill is arguably the point; a fully-offline setter is a separate call.)
- *Setter identity & persistence.* Store a player id vs. a name? How does it survive reboot for door/container traps that persist, vs. transient object traps? What happens if the setter character is deleted/renamed?
- *Builder/world traps.* Confirmed they must fall back to the unattributed path — but should builders optionally be able to assign a "credited" owner? (Probably no; banked.)
- *Damage-pipeline parity.* `applyDamage` routes through the combat path (`getActualDamage`/`doDamage`) while `objDamage` uses `objDam`. The `DAMAGE_TRAP_*` resistance/immunity handling must produce the same numbers through both, or trap damage silently shifts when a setter is present vs. not. Needs a focused check before switching.
- *PvP / consent & peaceful-room implications* of crediting a remote player for a kill they weren't present for.

## Design: player-settable portal traps, door-style (PROPOSED — not implemented)

> Status: design only as of branch `trap-system-disentangle`. Nothing here is in the code yet. Captured so the research isn't lost.

**Today's behavior (verified).** Portal traps are real and fire — `triggerPortalTrap` runs on **two** actions against the trapped object: opening it (`inventory.cc:166`, `TPortal::openMe`, gated by a detect-trap check) and entering it (`obj_portal.cc:302`, no detect check, so the enter path is the no-warning "gotcha"). Both route through the shared post-refactor tail (`triggerPortalTrap → applyTrapEffect`). But there is **no player path to set a portal trap**: the set-trap command (`doSetTraps`) has exactly five targets — door/cont/mine/grenade/arrow (table at `trap.cc:146-150`) — and no `TRAP_TARG_PORTAL` case. Portal traps exist only as OEdit-authored object data (`create_objs.cc`, bit-packed into the portal's value `x3`, read at `obj_portal.cc:86`).

**The "two sides aren't the same object" finding.** A two-way portal is **two independent `TPortal` objects** — A in room 1 targeting room 2, B in room 2 targeting room 1 — paired only at runtime by `findMatchingPortal()` (`obj_portal.cc:408`). The trap flag lives on each object separately. Consequences:
- The trap fires on whoever **opens or enters the trapped object** (the *source* side). Arriving at the destination just places you in the room (`thing_to_room`) — you never open/enter the far portal, so **its trap cannot fire on arrival.** You cannot trap "the exit" to ambush travelers popping out; only the user of a portal is ever hit.
- The entry code already syncs **charges** across the matched pair (`obj_portal.cc:368-379`, so both sides expire together) but does **nothing** to sync **trap state.** So "trap both directions" was simply never built for portals — not impossible, just absent.

**Why this is worth unifying.** The "trap both sides" pattern already exists for **doors**: setting a door trap mirrors `EXIT_TRAPPED` to the reverse exit `back` (`task_trap.cc:85-98`), and disarm clears both (`trap.cc:537-541`, `doors.cc`, `disc_thief_looting.cc`). Doors are bidirectional exit-condition bits with an always-present `back`, so mirroring is a one-liner at set-time. A *synced* portal pair (e.g. the Portal prayer's two-way portals) is conceptually the same two-sided structure — so a portal trap that mirrors to its partner would make portals consistent with doors and with player expectations.

**What it would take (the real work is the authoring side, not the effect).** The trigger/effect tail is already shared and safe. The missing piece is a player authoring path:
- Add a sixth `TRAP_TARG_PORTAL` (its own skill, or overload `SKILL_SET_TRAP_DOOR` to recognize portal objects in the room), with component costs and learn rates like the other five.
- To get door-like two-sided behavior, mirror the trap to the synced partner via `findMatchingPortal()`.

**Banked open questions (decide later):**
- *Edit-time vs. runtime partner lookup.* Doors mirror at set-time to a `back` that always exists. A portal's partner is a *separate object* that may not be loaded when the trap is set (different zone reset, one-way portal, or the partner simply doesn't exist yet). Mirroring must either run a live `findMatchingPortal()` at set-time and silently no-op when the partner is absent, or resolve lazily at trigger time. Neither is the free copy-paste the charge-sync code first suggests — charges sync at *transit* time, when both objects definitely exist.
- *One-way portals are often one-way on purpose.* Auto-mirroring removes a builder's ability to make a portal that is safe to enter but trapped on return (or vice versa). "It wasn't done" is not proof it should be done — confirm the design actually wants symmetric portal traps before forcing them.
- *Builder migration.* OEdit-authored portal traps already exist in the world. A new player-facing system must leave those working unchanged (same trigger-only fallback as today), exactly like the setter-attribution feature above keeps setter-less traps as the safe default.

## Design: area traps & sympathetic detonation (PROPOSED — not implemented)

> Status: design only as of branch `trap-system-disentangle`. Nothing here is in the code yet. Captured so the research isn't lost.

**The three-axis model the refactor exposed.** Post-refactor, a trap decomposes into three orthogonal axes, of which the six "sources" (door/container/portal/mine/grenade/arrow) are just hardcoded presets:
- **trigger** — what fires it (open, enter, bash, move-into, reach-inside, arrow-strike, grenade-countdown). Still bundled per source.
- **effect** — the per-type damage/status. **Already unbundled** into `applyTrapEffect(type, dam, carrier, setter)` — victim-agnostic, needs no "primary."
- **targeting** — who is hit (single victim; same-room ½ splash; far-room ½ splash). Partially unbundled — each source open-codes its own room loop.

Once `applyTrapEffect` (effect) and a `detonateInRoom(...)` primitive (targeting) both exist, *every* trap — including kinds nobody has built — is just `{trigger} → {targeting} → applyTrapEffect`. That is the latent payoff of the disentangle work.

**1. Area trap (the foundational primitive).** A first-class trap whose targeting is "the whole room," decoupled from any opener — a victimless detonation ("step into the room / a timer ticks → everyone takes the blast"). This is *not* a new effect, just a new preset assembled from existing pieces, and it is far smaller and safer than it sounds because the room loop already exists and is proven three times over: `detonateGrenade`, mine `TRAP_EFF_ROOM` (`trap.cc:722`), and `triggerDoorTrap`'s splash all run the same `*(it++)` snapshot + `delete tbt` on `DELETE_THIS` pattern. The work is *extracting* that loop into one `detonateInRoom(type, dam, carrier, setter)` and giving it a trigger. `carrier` may be null (doors already pass null); `setter` is the attribution seam already threaded — so it composes onto the existing stack with no new plumbing through the effect layer. No recursion, no re-entrancy.

**2. Sympathetic detonation / chain reaction (the recursive layer on top).** An explosive detonation (TNT/grenade) searches the room for *other* trapped objects/doors/portals and detonates them too — area traps that set off area traps. This is the elaboration of #1, and it is where the genuine engineering and crash risk live (everything below).

**Why this is feasible now and wasn't before.** Pre-refactor, triggers were ~15-case switches welded to an opener, so "detonate a door trap with nobody opening it" meant untangling that switch. The shared, victim-agnostic `applyTrapEffect` *is* the missing primitive. The remaining obstacle is conceptual, not mechanical: every `triggerXxxTrap` is a method **on a victim** (actor-driven — someone opened/entered/bashed), whereas an area/chain detonation is **environmental** (the trap just goes off). Doors/containers/portals therefore need a new victimless entry point (`detonateInRoom`); the trigger functions would delegate to it, opener = "primary at full," area/chain = "no primary, all splash." Grenade already has this shape and is the model.

**Banked open questions (decide later):**
- *The trigger axis is the real open design space.* Effect and targeting are basically solved; what fires an area trap is not. Options: entry-triggered (walk in → blast), timed/pulse hazard (lava room, gas that ticks), tripwire/proximity, lever/remote (pull in room A → blast room B), or "another trap did it" (the cascade). Pick the MVP trigger before writing code.
- *Which types propagate (chain only)?* Only explosives (TNT + grenade), or does FIRE spread to other fire traps (conflagration)? Status types (poison/sleep/disease/teleport) clearly should not chain. "Explosive sets off explosive" is the sane MVP.
- *Cascade safety (chain only — this is the hard part).* A chain walks `room->stuff` while each detonation deletes other entries (dying beings, FIRE/TNT carriers destroying themselves + spawning scraps). Naive "walk and detonate what you find" is a use-after-free. Required shape: a **pending-detonation queue** — enqueue the initial trap, pop → clear its trapped flag → detonate (which may enqueue neighbors) → repeat until empty, guarding every dereference. **Termination is already half-solved**: every trigger clears its trapped flag *before* firing (`triggerDoorTrap` REMOVE_BITs `EXIT_TRAPPED` both sides; containers set `CONT_EMPTYTRAP`), so a clear-then-fire chain detonates each trap at most once. The residual hazard is pointer validity — a queued carrier can be deleted by an earlier detonation in the same chain (e.g. a FIRE container shredding the bag holding another trapped container).
- *Reach (chain only).* Same-room only (the blast), or does a chained *door* trap push the cascade into the adjacent room (doors already splash far-side)? Same-room is the safe MVP; cross-room cascades multiply the safety surface.
- *Damage stacking & griefing.* Several trapped carriers + one detonation = everyone eats stacked splash. Emergent demolition fun vs. PK/peaceful-room exploit — same consent concern flagged for setter attribution.
- *Grenade countdown interaction (chain only).* Does a chain *instantly* detonate armed grenades or just start their `ARMED3→2→1` countdown? Instant = true sympathetic detonation; countdown = safer and more readable.
