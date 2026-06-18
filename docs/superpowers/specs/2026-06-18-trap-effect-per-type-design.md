# Trap effect refactor: per-damage-type effect functions

**Status:** Design (approved in brainstorm 2026-06-18). No code yet. Branch: `trap-system-disentangle`.
**Companion reference:** `docs/systems/important/trap-system.md` (current-behavior baseline).

## Problem

Trap behavior is organized by *source* (door, container, portal, mine, grenade, arrow) and copy-pasted across each. Six near-identical switch statements over the 16 `doorTrapT` damage types (`triggerTrap`, `triggerContTrap`, `triggerPortalTrap`, `triggerArrowTrap`, `triggerDoorTrap`, `grenadeHit`, plus `detonateGrenade`'s message switch) each re-implement "for this damage type: message → damage/status → engulf → DELETE-check." The set-failure path (`goofUpTrap`, 4 branches) and the damage/learn calculators (`getXxxTrapDam` ×5, `getXxxTrapLearn` ×5) duplicate the same per-type and per-source logic again. A change to one damage type means editing it in up to a dozen places, and the DELETE-flag handling is copy-pasted 90+ times.

## Goal

Invert the axis: make the *damage type* the unit of reuse. One function per damage type, applied to a single being, callable by every source. Sources keep only what is intrinsic to them. Accept minor, deliberate gameplay/text normalization (the "dedup + normalize" decision) to get a substantially cleaner result.

This refactor is the **trigger/effect axis only**. It is the natural choke point for the separately-specced setter-attribution feature, so the seam for that is built in now (but not wired).

## Architecture: three layers

### Layer 1 — `applyTrapEffect(doorTrapT type, TBeing* victim, int dam, TBeing* setter = nullptr)`

The per-type effect, applied to exactly one being. One switch over the 16 types (~12 distinct bodies; `BOLT`/`DISK`/`PEBBLE` keep their own cases for flavor text but reuse the pierce/slash/blunt damage classes). Each case:

1. Emits the standardized "what hits you" message for that type (TO_CHAR + TO_ROOM).
2. Applies the effect:
   - **Physical types** (spike, blade, hammer, fire, acid, frost, energy, tnt, bolt, disk, pebble) → `dealTrapDamage(victim, damageClass, dam, setter)`, then the engulf call where applicable (`flameEngulfed`/`frostEngulfed`/`acidEngulfed`).
   - **Status types** (poison, sleep, disease, teleport) → the existing `victim->trap{Poison,Sleep,Disease,Teleport}(dam)` helpers; these ignore `setter`.
3. Returns DELETE flags (`DELETE_THIS` if the victim died) so the caller handles deletion.

It knows nothing about rooms, blast radius, or carriers. This is the function the whole refactor is named for.

### Layer 2 — `dealTrapDamage(TBeing* victim, spellNumT damageClass, int dam, TBeing* setter)`

The single damage choke point for physical trap types:

```
if (setter && setter != victim)
    return setter->applyDamage(victim, dam, damageClass);   // attributed, XP/kill credit, cross-room safe
else
    return victim->objDamage(damageClass, dam, nullptr);     // today's behavior, unattributed
```

Today `setter` is always `nullptr`, so this is behavior-preserving. It is also the exact insertion point for the setter-attribution design (`docs/systems/important/trap-system.md` → "Design: setter attribution"), and isolates the damage-pipeline-parity concern (`applyDamage`'s `doDamage` path vs `objDamage`'s `objDam` path) to one place.

### Layer 3 — source applicators (one thin handler per source)

Each existing trigger entry point shrinks to only what is intrinsic to that source:

```
emitSourceIntro(type);                       // "<flame> erupts from the <door>!" / "$p detonates!"
buildBlast();                                 // target list per THIS source's area rule (see below)
for (StuffIter it ...; ) {                    // safe *(it++) iteration
    TBeing* tgt = ...;
    int rc = applyTrapEffect(type, tgt, share(tgt), setter);
    // primary died -> propagate; bystander died -> delete inline & continue
}
handleCarrierFate();                          // clear EXIT_TRAPPED / destroyDoor / DELETE_ITEM / decrement charge / delete grenade
```

The DELETE-flag discipline (post-increment iteration, per-target check, inline delete of dead bystanders, `DELETE_THIS → DELETE_VICT` translation for the triggerer, carrier fate after the loop) lives in this one loop shape instead of being copy-pasted across six functions — a net safety improvement.

## Normalization decisions (proposed defaults — adjustable)

These are the deliberate, minor gameplay/text changes enabled by "dedup + normalize." None require data migration.

### Area-of-effect = a property of the **source** (not the damage type)

Per the brainstorm: a frost grenade gasses the room while a frost arrow hits one target, and the frost *logic* doesn't know the difference.

| Source | Blast set | Notes |
|---|---|---|
| Arrow | struck target only | one projectile, one body |
| Container | opener only | currently splashed only for TNT — normalized to single |
| Portal | opener only | currently splashed only for TNT — normalized to single |
| Door | opener **+ far-side room occupants** | the only source with a "far side"; a door trap blows both ways for every type (currently only TNT/frost/energy/acid did) |
| Mine | everyone in the current room | normalizes away the optional `TRAP_EFF_ROOM` flag |
| Grenade | everyone in the current room | already the case |

### Damage share

Primary target (the stepper / opener / struck / door-opener) takes full `dam`; every incidental bystander (room + door far-side) takes `dam / 2`. This replaces the current mix of ½, ⅓, and ¼. Grenades have no "primary" (timer-detonated), so every being in the room takes full `dam` — the one intentional exception, matching today's `grenadeHit`.

### Messages

- **Victim message** ("what hits you") → owned by `applyTrapEffect`, one per type, standardized across all sources. This is where most of the duplication lived.
- **Source intro** ("where it came from") → owned by the source applicator, one short line per source. To keep flavor cheaply, the per-type data table carries a `sourceNoun` (e.g. "flame", "spikes", "acid cloud") so a source can say `format("%s erupts from the %s!") % trapNoun(type) % doorName` without per-source duplication.
- Net effect: the elaborate per-source+per-type prose (e.g. "A column of flame shoots from a concealed jet in $p") collapses to {generic per-source intro + per-type noun} + standardized victim line. This is the accepted normalization tradeoff.

## Supporting collapses (same refactor, same per-type axis)

These share the duplication and should be folded in so the per-type/per-source data lives in one place:

- **`getTrapDam` (5 → 1):** `getTrapDam(trap_targ_t targ, doorTrapT type)` driven by a `trapSourceInfo[targ]` table `{ baseDam, skillDivisor, setSkill }` plus a shared `trapDamMod[type]` table (the `+3/-5` per-type modifiers). Clamp 1–50 once.
- **`getTrapLearn` (5 → 1):** `getTrapLearn(trap_targ_t targ)` using `trapSourceInfo[targ].setSkill`. (The five current functions are byte-identical apart from the skill enum.)
- **`goofUpTrap` (4 → 1):** `goofUpTrap(doorTrapT type, trap_targ_t targ)` → emit the "you slip up" intro + `applyTrapEffect(type, this, reducedDam)` where `this` (the setter) is the victim. Reduced damage normalized to `dice(getTrapDam, 8) / 3` for all sources (currently door/container use `/3`, mine/grenade use full — minor tuning).
- **`detonateGrenade` / `grenadeHit`:** the grenade applicator uses `applyTrapEffect` per room occupant; `detonateGrenade`'s per-type intro switch collapses into the shared source-intro + `sourceNoun`.

### Data tables introduced

- `trapEffectInfo[doorTrapT]` → `{ damageClass (or NONE for status-only), engulfKind, statusKind, victimMsg, sourceNoun }`. Drives `applyTrapEffect` and source intros. (Independently derived from current code; not carried over from any prior branch.)
- `trapDamMod[doorTrapT]` → the additive per-type damage modifier.
- `trapSourceInfo[trap_targ_t]` → `{ baseDam, skillDivisor, setSkill, areaRule }`.

## Scope

**In scope:** the 6 trigger switches, `detonateGrenade`'s message switch, the 4 `goofUpTrap` branches, the 5 `getXxxTrapDam`, the 5 `getXxxTrapLearn`, and the three data tables above.

**Out of scope (separate disentangle threads):** the five independent storage representations; the seven-skills consolidation; the `DISC_TRAPS` vs `DISC_LOOTING` task-guard mismatch; the `mapFileToDoorTrap`/`mapDoorTrapToFile` identity indirection; the setter-attribution *wiring* (only the `setter` seam is added here).

## Behavior preservation & risk

- **Persisted data unchanged** (`roomDirData::trap_info`/`trap_dam`/`EXIT_TRAPPED`, `TTrap` obj values, container flags/bitpack, portal/arrow fields). No DB migration.
- **DELETE-flag correctness** is the primary risk; the design centralizes the one tricky loop pattern (Layer 3) instead of duplicating it, and `applyTrapEffect` returns DELETE per the existing convention.
- **Deliberate behavioral deltas** (all from normalization, listed above): per-source area assignments, unified ½ bystander fraction, homogenized messages (softened by `sourceNoun`), uniform goof damage. Confirm these are acceptable on review.
- **Opportunistic bug note:** rewriting the poison case will surface `trapPoison`'s dead immune-branches (every non-immune victim currently gets max-duration poison). Whether to fix that here (changes poison duration) or leave it is a flagged decision, not assumed.
- **Damage-pipeline parity** only matters once setter attribution is wired; isolated to `dealTrapDamage`.

## Testing

- Pure functions are unit-testable now: `getTrapDam`/`getTrapLearn` via `make test` (CTest) — add cases per (source, type).
- Effect/trigger paths have heavy combat side-effects; verify via the functional harness (`make test-func`) where feasible, plus manual matrix testing: for each source × representative damage types, set/place a trap (settrap or OEdit), trigger it, confirm message, damage, area, and carrier fate.

## Open items deferred to implementation planning

- Exact wording of the standardized victim messages and per-source intros (preserve recognizable flavor).
- Final per-source area table sign-off (above is the proposal).
- Whether to fix the `trapPoison` dead-branch bug in this pass.
