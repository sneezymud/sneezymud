# Trap Component Bag — Design

**Date:** 2026-07-02
**Branch:** disentangle-set-reclaim-comps-specs
**Status:** Approved (pending spec review)

## Problem

Trap reagents are stacking, charge-based `TTrapComponent` items (vnums 900–934, `ITEM_TRAP_COMPONENT`). A trapper carrying reagents for several trap types has them loose in top-level inventory, and the trap-set flow (`TBeing::hasTrapComps`) only finds reagents there — `searchLinkedListVis(this, name, stuff)` searches top-level inventory and `TTable` riders, never container contents.

We want a dedicated container that (a) holds only trap components and (b) whose contents count when building traps, so a trapper can organize reagents in a bag and still set traps from it.

## Precedent

`TSpellBag` (`ITEM_SPELLBAG`) is the codebase's maintained component-bag pattern: a `TExpandableContainer` that holds spell components and is wired into the spell-cast component search (`findComponent` → `comp_from_object` recurses into the bag via the polymorphic `findSomeComponent`). The `trapBag` branch (`a69ffdfa4`) built a rougher, never-wired-in `TTrapBag`. We mirror `TSpellBag` rather than port that branch.

## Design

### Class

`TTrapCompBag : public TExpandableContainer`

- `itemType()` returns `ITEM_TRAPCOMP_BAG`.
- Name chosen over `ITEM_TRAP_BAG` / `TTrapBag` to avoid reading as "a bag that is a trap"; it holds trap *components*.
- Constructors / `operator=` / four-values delegate to `TExpandableContainer`, exactly like `TSpellBag`.
- `putSomethingInto(TBeing*, TThing*)`: reject anything that isn't a `TTrapComponent` with a message ("$p can only hold trap components."); otherwise delegate to `TExpandableContainer::putSomethingInto`.
- `lowCheckSlots(silentTypeT)`: restrict permitted wear slots (throw/take/hold/waist/body/legs), mirroring `TSpellBag::lowCheckSlots`.
- `allowsCast()` — not needed (trap bags aren't spell-cast sources). Omit.

### Registration (standard new-bag-item wiring, following `ITEM_SPELLBAG`)

- `code/code/misc/obj.h`: **append** `ITEM_TRAPCOMP_BAG` to the `itemTypeT` enum. Item types are persisted in obj/DB records, so it must be appended, never inserted mid-enum.
- `code/code/sys/db.cc`: factory `case ITEM_TRAPCOMP_BAG: return new TTrapCompBag();`
- `code/code/misc/constants.cc`: `ItemInfo[ITEM_TRAPCOMP_BAG]` entry (display name, capacity fields), modeled on the spellbag entry.
- `code/code/misc/obj.cc`: the two itemType mapper spots that reference `ITEM_SPELLBAG`.

### Trap-set integration (the crux)

Add a helper (on `TBeing`) `TThing* findTrapComp(const sstring& name)`:

1. First try top-level inventory: `searchLinkedListVis(this, name, stuff)`.
2. If not found, iterate `stuff` for any `TTrapCompBag` and search each bag's contents for a matching component.
3. Return the first match, or `nullptr`.

`hasTrapComps` swaps its three (or four, for mine/grenade casing) `searchLinkedListVis` lookups for `findTrapComp`.

**Consumption path is unchanged.** `hasTrapComps`'s `spendCharge` decrements/deletes whichever `TTrapComponent` the lookup returns; `TObj`'s destructor unlinks it from its parent, so spending a charge from a component nested in the bag already works. The recently-stabilized set/reclaim charge economy is untouched except for the lookup source.

### Out of scope

- **No shop bulk-sell/value.** Unlike `TSpellBag` (`componentSell`/`componentValue`), the trap bag does not add shop hooks. Components still sell individually the normal way.
- **No crafting.** Builders create the bag item in-world via the object editor / seed data like any other container. We do not add a recipe to build one in-game.

## Testing

A `GameFixture` unit test (`tests/cpp/unit/`, deterministic — no DB/server):

- A `TTrapCompBag` accepts a `TTrapComponent` (put succeeds, component ends up inside).
- The bag rejects a non-component object (put refused).
- `findTrapComp` locates a component nested inside a bag carried in inventory (the integration guard — this is the regression a future refactor could break).

## Files touched

- New: `code/code/obj/obj_trapcomp_bag.h`, `code/code/obj/obj_trapcomp_bag.cc`
- Edit: `obj.h` (enum), `db.cc` (factory), `constants.cc` (ItemInfo), `obj.cc` (mappers), `trap.cc` (`findTrapComp` + `hasTrapComps` lookups), `being.h` (declare `findTrapComp`)
- New test: `tests/cpp/unit/trapcomp_bag_test.cc`
