---
title: Object System
description: Physical item management including weapons, armor, containers, consumables, furniture, and utility objects
category: critical
keywords: [object types, containers, decay, traps, consumables]
primary_symbols:
  functions: [makeNewObj, read_object, assignFourValues, getFourValues, objectDecay, lightDecay, eatMe, drinkMe, has_key, pickMe, triggerContTrap]
  classes: [TObj, TBaseContainer, TOpenContainer, TExpandableContainer, TBaseLight, TLight, TFood, TBaseCup, TDrinkCon, TVehicle, TAudio, TBook, TBed, TTable, TKey, TKeyring]
  enums: [itemTypeT, ITEM_UNDEFINED, ITEM_LIGHT, ITEM_CHEST, ITEM_DRINKCON, ITEM_FOOD, ITEM_BAG, ITEM_CORPSE, ITEM_BED, ITEM_PCORPSE, ITEM_VEHICLE, CONT_CLOSEABLE, CONT_PICKPROOF, CONT_CLOSED, CONT_LOCKED, CONT_TRAPPED, CONT_SECRET, CONT_EMPTYTRAP, CONT_GHOSTTRAP, CONT_WEIGHTLESS, CONT_JAMMED, DOOR_TRAP_TNT, DOOR_TRAP_POISON, DOOR_TRAP_SLEEP, DOOR_TRAP_FIRE, DOOR_TRAP_ACID, DOOR_TRAP_DISEASE, DRINK_POISON, DRINK_PERM, DRINK_SPILL, DRINK_FROZEN, FOOD_POISON, FOOD_SPOILED, FOOD_FISHED, FOOD_BUTCHERED, DELETE_THIS, DELETE_ITEM, IS_SET_DELETE, StuffIter]
---

## Overview

How does the game know a sword is a weapon but a bag is a container? How does a lamp know when its fuel runs out? How do containers enforce weight limits and trap security?

The object system manages all physical items: weapons, armor, containers, consumables, furniture, and utility objects. Every item inherits from TObj and declares its type via the pure virtual `itemType()` method. A factory creates the correct subclass from database definitions, populates its properties via value fields, and manages its lifecycle from creation through decay.

Objects participate in spatial relationships through parent/stuff pointers. Containers hold other objects with weight and volume limits. Some containers have locks requiring keys or pick skill, and traps that trigger on unauthorized access. Consumables deplete over time: lights burn fuel, food satisfies hunger, drinks quench thirst.

The type hierarchy uses abstract base classes for shared behavior. TBaseContainer defines the container interface. TOpenContainer adds locks and traps. TExpandableContainer adds volume expansion. TBaseCup defines liquid storage. TBaseLight defines fuel burning. Each concrete class implements type-specific logic while inheriting common patterns.

---

## Patterns

### Always Check DELETE Flags After Trap Triggers

`triggerContTrap()` returns `DELETE_THIS` when a trap kills the victim. Every call site must check this flag and propagate it upward. Continuing execution after death causes use-after-free crashes.

### Remove Objects Before Deletion

Never delete objects still inside containers. Call `--(*item)` first to remove from parent, then delete. Deleting without removal corrupts the container's stuff list.

### Use Post-Increment When Iterating During Removal

The pattern `*(it++)` advances the iterator before the current element is removed. Without this, removing an element invalidates the iterator and causes skipped elements or crashes.

### Check Container Capacity Before Adding

Containers have both weight and volume limits. Check `getCarriedWeight() < maxWeight` and `getCarriedVolume() < maxVolume` before adding items. The system does not automatically reject oversized items.

### Never Nest Non-Empty Containers

Non-empty containers cannot go inside other containers. This prevents circular references and infinite weight calculation loops. Empty containers may be nested.

### Use CONT_WEIGHTLESS for Bags of Holding

Containers with the `CONT_WEIGHTLESS` flag ignore the weight of their contents. This flag must be checked in `getTotalWeight()` to return 0 for carried weight.

### Apply TComponent Weight Reduction

Spell components weigh only 10% of their actual weight when carried. Always apply this multiplier in weight calculations.

### Propagate DELETE_ITEM from Container Operations

Container trap triggers, decay, and destruction return DELETE_ITEM. Check `IS_SET_DELETE(rc, DELETE_ITEM)` and stop execution immediately. The object pointer is invalid after this flag is returned.

### Use roomOfObject() for Items in Containers

When `item->roomp` is NULL, the item is inside a container. Call `roomOfObject()` to traverse the parent chain and find the actual room.

### Check Liquidity Before Drinking

Frozen drinks (`DRINK_FROZEN` flag) cannot be consumed. Permanent containers (`DRINK_PERM` flag) never empty. Pool drinking risks disease.

---

## Reference

### Class Hierarchy Overview

| Category | Abstract Base | Concrete Classes |
|----------|---------------|------------------|
| Weapons | TBaseWeapon | TGenWeapon, TArrow |
| Clothing | TBaseClothing | TArmor, TWorn, TSaddle, THarness |
| Light | TBaseLight | TLight, TFFlame |
| Drinks | TBaseCup | TDrinkCon, TPotion, TVial, TPool |
| Containers | TBaseContainer, TOpenContainer | TChest, TCookware, TWagon, TBag, TQuiver, TSpellBag, TKeyring, TMoneypouch, TSaddlebag, TSuitcase |
| Corpses | TBaseCorpse | TCorpse, TPCorpse |
| Magic | TMagicItem | TScroll, TWand, TStaff |
| Portals | TSeeThru | TPortal, TWindow |
| Food | TFood | TFruit, TEgg |
| Direct TObj | - | TBow, TComponent, TTool, TSymbol, TTrap, TBed, TTable, TMoney, TBoat, TAudio, TBoard, TBook, TTree, TNote, TPen, TKey, TBandage, TStatue, TFuel, TOpal, TTreasure, TTrash, TOtherObj, TGemstone, TJewelry, TCommodity, TOrganic, TAppliedSub, TGas, TDrugContainer, TDrug, TGun, TAmmo, TPlant, TVehicle, TCasinoChip, TPoison, THandgonne, TCannon |

### itemTypeT Core Types

| Type | Value | Description |
|------|-------|-------------|
| `ITEM_UNDEFINED` | 0 | Invalid or uninitialized. Factory returns nullptr. |
| `ITEM_LIGHT` | 1 | Refillable light sources (TLight) |
| `ITEM_CHEST` | 15 | Lockable storage (TChest via TOpenContainer) |
| `ITEM_DRINKCON` | 17 | Drink containers (TDrinkCon) |
| `ITEM_FOOD` | 19 | Edible items (TFood) |
| `ITEM_BAG` | 27 | Expandable containers (TBag) |
| `ITEM_CORPSE` | 28 | NPC corpses, relocate contents on destruction |
| `ITEM_BED` | 40 | Resting furniture (TBed) |
| `ITEM_PCORPSE` | 47 | Player corpses for equipment recovery |
| `ITEM_VEHICLE` | 61 | Drivable objects (TVehicle extends TPortal) |

### Value Field Semantics by Type

| Type | val0 | val1 | val2 | val3 |
|------|------|------|------|------|
| Weapons | Sharpness (cur/max packed) | Damage (level/dev packed) | Weapon types/frequencies | Arrow data |
| Lights | Light amount | Max burn time | Current burn time | Lit status / magic flags |
| Containers | Max weight | Flags + trap type + trap damage (packed) | Key vnum | Max volume |
| Drinks | Max units | Current units | Liquid type | Drink flags |
| Magic Items | Magic level | Max charges / spell 1 | Current charges / spell 2 | Spell number / spell 3 |
| Food | Fullness value | - | - | Food flags |
| Beds | Min position + max users (packed) | Max designed size | Seat height | Regen bonus |

### Container Flags

| Flag | Bit | Effect |
|------|-----|--------|
| `CONT_CLOSEABLE` | 0 | Can be opened/closed |
| `CONT_PICKPROOF` | 1 | Cannot be picked |
| `CONT_CLOSED` | 2 | Currently closed |
| `CONT_LOCKED` | 3 | Requires key |
| `CONT_TRAPPED` | 4 | Has a trap |
| `CONT_SECRET` | 5 | Hidden container |
| `CONT_EMPTYTRAP` | 6 | Trap was disarmed |
| `CONT_GHOSTTRAP` | 7 | False trap (0 damage) |
| `CONT_WEIGHTLESS` | 8 | Contents weigh nothing |
| `CONT_JAMMED` | 9 | Lock is jammed |

### Trap Types

| Type | Effect | Damage Type |
|------|--------|-------------|
| `DOOR_TRAP_TNT` | Explosive | DAMAGE_TRAP_TNT |
| `DOOR_TRAP_POISON` | Poison | DAMAGE_TRAP_POISON |
| `DOOR_TRAP_SLEEP` | Sleep | - |
| `DOOR_TRAP_FIRE` | Fire + engulf | DAMAGE_TRAP_FIRE |
| `DOOR_TRAP_ACID` | Acid | DAMAGE_TRAP_ACID |
| `DOOR_TRAP_DISEASE` | Disease | DAMAGE_TRAP_DISEASE |
| `DOOR_TRAP_SPIKE` | Piercing | DAMAGE_TRAP_PIERCE |
| `DOOR_TRAP_BLADE` | Slashing | DAMAGE_TRAP_SLASH |
| `DOOR_TRAP_PEBBLE` | Bludgeon | DAMAGE_TRAP_BLUNT |
| `DOOR_TRAP_FROST` | Cold | DAMAGE_TRAP_FROST |
| `DOOR_TRAP_TELEPORT` | Random teleport | - |
| `DOOR_TRAP_ENERGY` | Energy | DAMAGE_TRAP_ENERGY |

### Object Flags

| Flag | Effect |
|------|--------|
| `ITEM_GLOW` | Object glows |
| `ITEM_HUM` | Object hums |
| `ITEM_INVISIBLE` | Object is invisible |
| `ITEM_MAGIC` | Object is magical |
| `ITEM_NODROP` | Cannot be dropped |
| `ITEM_BLESS` | Object is blessed |
| `ITEM_ANTI_*` | Class restrictions |
| `ITEM_PAIRED` | Can be dual-wielded |
| `ITEM_BURNING` | Currently on fire |
| `ITEM_NOLOCATE` | Cannot be located magically |
| `ITEM_NEWBIE` | Junked on corpse decay |

### Drink Container Flags

| Flag | Value | Effect |
|------|-------|--------|
| `DRINK_POISON` | 1 | Liquid is poisoned |
| `DRINK_PERM` | 2 | Never empties (fountains) |
| `DRINK_SPILL` | 4 | Spills during movement |
| `DRINK_FROZEN` | 8 | Frozen solid |

### Food Flags

| Flag | Value | Effect |
|------|-------|--------|
| `FOOD_POISON` | 1 | Applies poison effect |
| `FOOD_SPOILED` | 2 | Causes food poisoning |
| `FOOD_FISHED` | 4 | Bonus for TALENT_FISHEATER |
| `FOOD_BUTCHERED` | 8 | Bonus for TALENT_MEATEATER |

### Condition Types

| Condition | Range | Special Values |
|-----------|-------|----------------|
| DRUNK | 0-24 | -1 = immune |
| FULL | 0-24 | -1 = immune, 0 = starving |
| THIRST | 0-24 | -1 = immune, 0 = dehydrated |
| PEE | 0-24 | Need to urinate |
| POOP | 0-24 | Need to defecate |

Zero FULL or THIRST reduces HP and mana regeneration by 75%.

### Light Intensity

| Amount | Description |
|--------|-------------|
| < 3 | dim |
| 3-7 | moderately-bright |
| 8-14 | bright |
| 15-24 | very bright |
| 25-34 | extremely intense |
| >= 35 | blinding |

### TFFlame Magic Flags

| Flag | Value | Effect |
|------|-------|--------|
| `TFFLAME_INVHEAT` | 1 | Cold fire |
| `TFFLAME_INVLIGHT` | 2 | Dark fire |
| `TFFLAME_MAGHEAT` | 4 | Double heat |
| `TFFLAME_MAGLIGHT` | 8 | Double light |
| `TFFLAME_IMMORTAL` | 16 | Never decays |

### Bed Position Values

| Value | Allowed Positions | Slots Required |
|-------|-------------------|----------------|
| 0 | Sleep, rest, sit | Sleep: 3, Rest: 2, Sit: 1 |
| 1 | Rest, sit | Rest: 2, Sit: 1 |
| 2 | Sit only | Sit: 1 |
| 3 | Unusable | - |

### Vehicle Constants

| Constant | Value | Description |
|----------|-------|-------------|
| `VEHICLE_BOAT` | 1 | Water vessel, nautical messages |
| `VEHICLE_TROLLEY` | 2 | Rail vehicle, rumbling messages |
| `FAST_SPEED` | 100 | Maximum speed |
| `MED_SPEED` | 50 | Standard operational speed |
| `SLOW_SPEED` | 25 | Minimum active speed |

### Volume Reduction Multipliers

| Material | vol_mult | Effect |
|----------|----------|--------|
| Cloth | 11 | Compresses to 1/11 volume |
| Leather | 4 | Compresses to 1/4 volume |
| Metal (Iron) | 1 | No compression |

### Key Files

| File | Contents |
|------|----------|
| `obj.h` | itemTypeT enum, TObj base class, object flags |
| `db.cc` | makeNewObj() factory |
| `obj_base_container.cc` | Container interface and volume calculations |
| `obj_open_container.cc` | Locks, traps, open/close mechanics |
| `obj_food.cc` | Food eating and spoilage |
| `obj_light.cc` | Light burning and refueling |
| `obj_base_cup.cc` | Drink mechanics and liquid properties |

---

## Implementation

### Object Factory and Lifecycle

Objects are created via `read_object(vnum, VIRTUAL)` which calls `makeNewObj(itemTypeT)` to instantiate the correct subclass based on the type constant. The factory uses a switch on itemTypeT to construct TLight, TGenWeapon, TArmor, etc. After construction, the object is populated from database values including names, descriptions, flags, value fields via `assignFourValues()`, weight, cost, material, and affects.

The TObj constructor registers every object in the global `object_list` and increments `objCount`. The destructor removes from the list, calls `CMD_GENERIC_DESTROYED` spec proc if present, recursively deletes all contents, removes from parent/room/equipment, dismounts riders, cancels referencing tasks, and cleans up strung descriptions.

### Decay System

Objects with `obj_flags.decay_time > -1` participate in decay. Each tick, `decayMe()` decrements the timer. At zero, `objectDecay()` is called. The base implementation relocates contents and returns DELETE_THIS. Type-specific overrides handle special behavior: TBaseCorpse shows decay messages and logs PC corpse info, TLight shows burn-out messages, TFood transitions to FOOD_SPOILED state before final deletion.

### Container Spatial Model

Containers use TThing's spatial pointers. Items inside have `parent = container` and `roomp = NULL`. The container's `stuff` list holds all direct contents. Bidirectional consistency is enforced: adding via `*container += *item` sets parent and adds to stuff; removing via `--(*item)` clears parent and removes from stuff.

Weight calculation traverses the stuff list recursively, summing `getTotalWeight(true)` for each item. TComponent items get 10% weight. CONT_WEIGHTLESS containers return 0 for carried weight. Volume calculation applies material-based reduction via `vol_mult` plus a 5% packing bonus.

### Lock and Key System

Keys match by object vnum. The `has_key()` function searches inventory, keyrings, held items, worn equipment, and potential mob loads. The `keyCheck` helper compares vnum directly, with a special case for dynamically generated keys using objVnum -1 and getSnum matching. Keyrings prevent duplicate keys of the same vnum.

Locking requires: container closed, has key_num, character has matching key, not already locked. Picking requires: SKILL_PICK_LOCK success roll, container not pickproof or jammed. Critical failure jams the lock permanently.

Door picking differs from container picking: doors use task system with TASK_PICKLOCKS requiring a lockpick tool and extended time. Containers use instant resolution via `pickMe()`.

### Trap System

Traps store type and damage in container_flags via bit packing. Setting traps uses TASK_TRAP_CONT (3 ticks). Disarming ghost traps always succeeds. Real trap disarm on failure triggers the trap.

Trap triggering dispatches by type: fire does damage plus engulf check, each dangerous call must check DELETE_THIS before continuing. Trap damage flows through standard combat damage pipeline via `objDamage()`.

Ghost traps prevent metagaming. When players fail SKILL_DETECT_TRAP on an untrapped container, a ghost trap with 0 damage is created. Players cannot distinguish ghost traps from real ones without attempting disarm.

### Container Nesting Restriction

Non-empty containers cannot be placed inside other containers. This prevents circular references and O(n squared) weight calculation. The restriction is enforced in `putSomethingIntoContainer()` which checks `!stuff.empty()`.

### Light Burning

Lit lights have `lightDecay()` called each tick. Current burn time decrements by 1. At 0, the light extinguishes via `putLightOut()`. Below 4, flicker warnings appear. Refueling via TFuel objects transfers units from fuel to lamp when the lamp is not lit and not full.

TFFlame objects are perpetual flames with magic flag modifiers. TFFLAME_IMMORTAL prevents decay and refueling.

### Food Consumption

`eatMe()` checks fullness (FULL > 20 rejects unless immortal), perceptive characters detect spoilage, race-based multipliers apply (vampires get 0 benefit, talent bonuses double effectiveness). Condition gain is modified by race food modifier and body mass ratio. Poison and spoilage effects apply. The food item is deleted after consumption.

Spoilage is two-stage: initial decay adds FOOD_SPOILED flag and extends timer, second decay deletes the item.

### Drink Consumption

`drinkMe()` calculates amount based on thirst satisfaction formula. Weight updates for non-permanent containers via `weightChangeObject()`. Conditions update: DRUNK from intoxication value, FULL from hunger value, THIRST from thirst value. Poison applies if flagged. Pools risk dysentery disease.

Liquid weight is 0.065 pounds per unit (SIP_WEIGHT constant). Container total weight equals base weight plus (units times SIP_WEIGHT).

### Vehicle Movement

TVehicle extends TPortal with direction, speed, and type. Movement occurs via `vehiclePulse()` called by scheduler. Speed determines movement frequency. Direction determines which exit to traverse. Auto-pathing follows single valid exits. Boats can enter non-water once but must return.

The `update_exits()` function synchronizes interior room exits to the vehicle's current location before each movement check. Whole-zone vehicles update all zone exits when moving.

### Book Content Loading

The `lookAtObj()` method in TBook constructs filenames from `objdata/books/` using the object vnum. If a section is specified, appends `.section`. For characters with color support, tries `.ansi` extension first, falling back to plain text. Content is displayed via `page_string()` or client note system.

### Bed Regeneration

`bedRegen()` modifies gain by regen percentage. Size mismatch (player too tall) reduces gain and shows discomfort message. Minimum gain is 0.

### Table Mechanics

Tables use the rider pointer to track items on top. Destruction dismounts all items, moves them to table's parent or room, and applies 50% structure damage.

---

## Troubleshooting

### Crash After Opening Trapped Container

**Symptom:** Server crashes immediately after player opens container with trap.

**Likely cause:** DELETE_THIS from `triggerContTrap()` was ignored. Code continued executing after the player character was destroyed.

**Diagnostic approach:** Check call site of `openMe()` or `triggerContTrap()`. Verify DELETE_THIS check exists immediately after the call.

**Fix:** Add `if (IS_SET_DELETE(rc, DELETE_THIS)) return DELETE_THIS;` after every trap trigger call.

### Items Disappear When Container Deleted

**Symptom:** Items inside container vanish instead of falling to ground.

**Likely cause:** Container is not a corpse. Standard TObj destructor recursively deletes contents.

**Diagnostic approach:** Check the container type. Only TBaseCorpse relocates contents on destruction.

**Fix:** If preservation is needed, manually iterate and relocate contents before deleting the container.

### Iterator Invalidation During Container Cleanup

**Symptom:** Skipped items or crash during container iteration with removal.

**Likely cause:** Using `++it` after removal instead of post-increment before removal.

**Diagnostic approach:** Check the iteration pattern. `for (auto it = stuff.begin(); it != stuff.end(); ++it)` with removal inside is broken.

**Fix:** Use `for (auto it = stuff.begin(); it != stuff.end();) { TThing* t = *(it++); --(*t); }` pattern.

### Weight Calculation Returns Wrong Value

**Symptom:** Container or character weight doesn't match expected sum.

**Likely cause:** Missing TComponent 10% reduction, missing CONT_WEIGHTLESS check, or rider chain not included.

**Diagnostic approach:** Manually sum contents. Check for TComponent items. Check container flags.

**Fix:** Ensure weight calculation applies all modifiers: component reduction, weightless containers, rider chain traversal.

### Lock Pick Always Fails

**Symptom:** Thief cannot pick container even with high skill.

**Likely cause:** Container is pickproof or jammed.

**Diagnostic approach:** Check container flags for CONT_PICKPROOF or CONT_JAMMED.

**Fix:** CONT_PICKPROOF is permanent. CONT_JAMMED from critical failure is permanent. Neither can be bypassed.

### Ghost Trap Causes Real Damage

**Symptom:** Trap marked as ghost does damage.

**Likely cause:** trap_dam was not set to 0 when ghost trap was created.

**Diagnostic approach:** Check `trap_dam` value. Ghost traps must have 0 damage.

**Fix:** Ensure ghost trap creation sets `setContainerTrapDam(0)` along with CONT_GHOSTTRAP flag.

### Light Burns Indefinitely

**Symptom:** Torch never runs out of fuel.

**Likely cause:** `lit` flag not set, or maxBurn is -1 (non-refuelable indicates infinite).

**Diagnostic approach:** Check `isLit()` return value and maxBurn value.

**Fix:** Verify light was properly lit via `lightMe()`. Check if maxBurn indicates intentionally infinite light.

### Food Doesn't Satisfy Hunger

**Symptom:** Player eats food but FULL condition doesn't change.

**Likely cause:** Vampire race (0 multiplier), or body mass ratio edge case.

**Diagnostic approach:** Check race food modifier. Check character weight versus standard 180.

**Fix:** Vampires cannot gain sustenance from normal food. Large characters need more food proportionally.

### Drinking From Pool Causes Disease

**Symptom:** Player gets dysentery from drinking water pool.

**Likely cause:** Working as intended. Standing water pools carry disease risk.

**Diagnostic approach:** Confirm the drink source is a TPool, not TDrinkCon.

**Fix:** This is correct behavior. Use containers or fountains for safe drinking.
