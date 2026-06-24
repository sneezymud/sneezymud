# Trap Damage Setter Attribution — Design

**Date:** 2026-06-24
**Branch base:** `disentangle-set-reclaim` (HEAD `d3978fbfa`)
**Status:** Design approved, ready for implementation plan

## Goal

When a player-set trap deals damage, the **setter** should receive combat
credit for it — experience and kill credit — as if they had dealt the damage
themselves. Today every trap trigger passes `setter = nullptr`, so all trap
damage is unattributed.

## Scope

**In scope (this change): object-carried traps.**
- `mine`, `grenade`, `container`, `arrow`, `portal`

**Deferred to a follow-up: door traps.** Door traps live in exit data
(`roomDirData.trap_info`/`trap_dam`/`EXIT_TRAPPED`), which has no
ex-description and no setter field, and most door traps are builder/world
content. Attributing them needs a separate `(room, dir) → setter` side-table
with its own lifecycle. Out of scope here.

**Out of scope entirely: affliction attribution.** Poison/sleep/disease/
teleport route through `trapPoison`/`trapSleep`/`trapDisease`/`trapTeleport`,
which take no setter. A damage-over-time kill (e.g. poison tick) stays
unattributed for everyone. DoT source-tracking is a separate problem.

## Key facts established during design

- **The credit plumbing already exists and is switched off.**
  `applyTrapEffect(type, power, carrier, setter, denom)` already forwards
  `setter` to `dealTrapDamage` for every physical damage type. Every trigger
  site just passes `nullptr` today. Lighting it up = resolve the setter at
  trigger time and pass it in.
- **Crediting an absent setter is safe by design.** `TBeing::applyDamage`
  (`damage.cc:368`) only sets up fighting `if (sameRoom(*v))` — comment:
  *"ranged damage comes through here… let's not set them fighting unless we
  need to."* A setter who armed a trap and walked away gets credit with no
  combat side effects.
- **Setter is stored as a NAME, not a pointer.** Mines stamp `TRAP_EX_DESC`
  (`"__trap_setter"`) in `TTrap::dropMe`; grenades stamp `GRENADE_EX_DESC`
  (`"__grenade_puller"`) in `armGrenade`. Resolution is `get_char(name,
  EXACT_YES)`, the same pattern already used by the move-trap group-spare
  (`trap.cc:543`) and grenade `pissOff` (`trap.cc:1463`).
- **The setter ex-desc keyword `"__trap_setter"` is not player-reachable** via
  normal `look` (would require literally typing `look __trap_setter`), and
  mines already accept this. Reusing it on visible objects (containers, etc.)
  introduces no new leak class.

## Architecture

### 1. Shared resolver (new free function, declared in `trap.h`)

```cpp
// Resolve a trap's recorded setter to a live, creditable being, or null.
// Null for any trap with no recorded setter (all world/builder/mob-loaded
// traps), which routes damage to the unattributed objDamage() fallback.
TBeing* trapSetter(const TThing* carrier);
```

Reads `TRAP_EX_DESC` (and, for grenades, falls back to `GRENADE_EX_DESC`) off
the carrier; returns `get_char(name, EXACT_YES)` or `nullptr`. Declared in
`trap.h` (defined in `trap.cc`) so it can be unit-tested directly — its
keyword-fallback and null cases are the one piece of genuinely new branching
logic. Only the trigger sites in `trap.cc` call it at runtime; the `trapMe`
overrides in `obj_*.cc` only *store* the name string.

### 2. Safety fix in `dealTrapDamage` (REQUIRED — latent crash otherwise)

`dealTrapDamage`'s `this` IS the victim, so its contract is to return
`DELETE_THIS` on the victim's death. The `objDamage` path does. But the
setter path calls `setter->applyDamage(this, …)`, which reports the victim's
death from the **setter's** frame as `DELETE_VICT` — a different bit.
`applyTrapEffect` and its callers check `IS_SET_DELETE(rc, DELETE_THIS)`, so
an attributed kill would slip through unpropagated → dangling dead victim →
crash. This branch has been dormant only because `setter` was always null.

```cpp
int TBeing::dealTrapDamage(spellNumT damageClass, int dam, TThing* carrier,
  TBeing* setter) {
  if (setter && setter != this) {
    int rc = setter->applyDamage(this, dam, damageClass);
    // applyDamage reports the victim's death as DELETE_VICT (its frame);
    // here `this` IS that victim, so translate to DELETE_THIS.
    return IS_SET_DELETE(rc, DELETE_VICT) ? DELETE_THIS : 0;
  }
  return objDamage(damageClass, dam, carrier);
}
```

### 3. Pass the resolved setter at each object trigger site

Resolve once per detonation; pass to every `applyTrapEffect` call — the
primary victim AND each room-splash bystander (denom=2).

- `triggerTrap` (mine): primary `trap.cc:679` + splash loop `trap.cc:666`
- `TBeing::grenadeHit` `trap.cc:1447`
- `triggerContTrap` `trap.cc:441`
- `triggerArrowTrap` `trap.cc:461`
- `triggerPortalTrap` `trap.cc:394`

(Door splash loops at `trap.cc:497`/`512` and `goofUpTrap` self-damage at
`trap.cc:1060` stay `nullptr` — out of scope / self-inflicted.)

### 4. Add setter storage to the three object types that lack it

Container/arrow/portal traps are armed by a **deferred task**, not in `trapMe`
(which only validates and calls `start_task`). The setter must be stamped at
**task completion**, where the trap data is actually written:
- `task_trap_container` (`task_trap.cc:199`) — stamp `cont`
- `task_trap_arrow` (`task_trap.cc:439`) — stamp `arrow`
- `task_trap_portal` (`task_trap.cc:677`/`687`) — stamp BOTH `portal` and
  `partner`, mirroring the existing trapped-flag partner sync, so a trigger
  from the far end still credits the setter.

Shared write-helper (declared in `trap.h`, defined in `trap.cc`), mirroring the
existing `TTrap::dropMe` stamp (`swapToStrung()` + prepend `TRAP_EX_DESC`):

```cpp
void recordTrapSetter(TObj* carrier, const TBeing* setter);
```

`dropMe`'s inline stamp is refactored to call this helper too (DRY). Mine
(`dropMe`) and grenade (`armGrenade`, via `GRENADE_EX_DESC`) already store a
setter; grenade keeps its own keyword and `trapSetter()` reads both.

### 5. Unattributed fallback (the single no-setter solution — no new code)

World-loaded, builder-placed, and mob-loader traps (e.g.
`mob_loader.cc:359/378` trapped bags) carry no `__trap_setter` ex-desc.
`trapSetter()` returns null for them, and `dealTrapDamage` routes null to
`objDamage(...)` — **byte-identical to today's behavior**. The same object
model handles both cases with zero special-casing: presence/absence of the
ex-desc decides attribution implicitly. No "is this world or player set"
branch anywhere.

## Edge cases

| Case | Behavior |
|---|---|
| Walk into your own trap | `setter == this` → unattributed (no self-XP) |
| Setter logged off / gone | `get_char` returns null → unattributed |
| Setter in another room | Credited; `applyDamage` skips combat setup (no `sameRoom`) |
| Mob setter | Harmless; mobs gain no player XP, `setter != this` guard holds |
| Attributed trap KILLS victim | `DELETE_VICT → DELETE_THIS` translation propagates death |
| World/mob-loaded trap | Null setter → `objDamage`, no regression |

## Testing

- Unit/functional: a player-set mine/grenade/container/arrow/portal trap that
  damages a victim credits the setter (XP/kill); the same trap killing the
  victim does not crash (exercises the `DELETE_VICT` translation).
- A world/mob-loaded trapped container damages a victim with no attribution and
  behaves exactly as before.
- Self-trigger and logged-off-setter both fall back to unattributed.

## Files touched

- `code/code/misc/trap.cc` — `trapSetter` + `recordTrapSetter` definitions, `dealTrapDamage` fix, 5 trigger sites
- `code/code/misc/trap.h` — `trapSetter` + `recordTrapSetter` declarations
- `code/code/misc/inventory.cc` — `dropMe` refactored to call `recordTrapSetter` (DRY)
- `code/code/task/task_trap.cc` — setter stamp at container/arrow/portal task completion
- `tests/cpp/unit/trap_dam_test.cc` — resolver + attribution-seam coverage

## Follow-up (separate spec)

Door-trap attribution via a runtime `(room vnum, dir) → setter name` side-table
populated in the `TASK_TRAP_DOOR` completion, consulted in `triggerDoorTrap`,
cleared on trigger/disarm.
