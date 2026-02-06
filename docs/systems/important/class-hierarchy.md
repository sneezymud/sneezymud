---
title: SneezyMUD Class Hierarchy
description: Complete object hierarchy from TThing base class through TBeing, TMonster, TPerson, TRoom, and 40+ TObj subclasses with runtime type identification.
keywords: [object hierarchy, runtime type identification, NPC AI, opinion system, container operations, factory pattern, linked lists, hate lists, fear lists]
category: important
primary_symbols:
  functions: [itemType, getKind, makeNewObj, isTMonster, isTPerson, toTMonster, toTPerson, addHated, remHated, DeleteHatreds, DeleteFears]
  classes: [TThing, TBeing, TMonster, TPerson, TRoom, TObj, opinionData, charList]
  enums: [thingTypeT, TYPETHING, TYPEOBJ, TYPEMOB, TYPEPC, TYPEROOM, TYPEBEING, TThingKind, itemTypeT, ITEM_UNDEFINED, ITEM_LIGHT, ITEM_SCROLL, ITEM_WAND, ITEM_STAFF, ITEM_WEAPON, ITEM_ARMOR, ITEM_POTION, ITEM_TRAP, ITEM_DRINKCON, ITEM_FOOD, ITEM_MONEY, ITEM_BAG, ITEM_CORPSE, ITEM_COMPONENT, ITEM_PORTAL, ITEM_WORN, ITEM_ARMOR_WAND, MAX_OBJ_TYPES, HATE_CHAR, HATE_SEX, HATE_RACE, HATE_CLASS, HATE_VNUM, FEAR_CHAR, FEAR_SEX, FEAR_RACE, FEAR_CLASS, FEAR_VNUM]
---

# SneezyMUD Class Hierarchy

## Overview

All world objects derive from a single abstract base class representing anything that exists in the game world. This hierarchy divides into three primary branches: rooms (static world locations), beings (living entities that can act), and objects (items that can be manipulated).

The being branch further splits into NPCs with AI behavior systems and player characters with persistence. The object branch explodes into 40+ specialized item types organized through intermediate abstract classes for weapons, containers, clothing, consumables, and magical items.

Runtime type identification uses a dual enum system: coarse-grained identification for broad categories and fine-grained item types for object specialization. NPC AI tracks relationships through opinion structures containing linked lists of hated and feared targets.

## Patterns

### Type Identification

- Prefer `isTMonster()` and `isTPerson()` over manual `dynamic_cast` for being type checks.
- Use `toTMonster()` and `toTPerson()` for safe type conversion with null safety.
- Check `itemType()` against `itemTypeT` values for object subclass identification.
- Use `getKind()` with `TThingKind` enum for broad category checks across the hierarchy.
- Always use `dynamic_cast` when accessing subclass-specific methods after type identification.
- Note: Multiple subclasses may return the same `itemTypeT` constant (e.g., `ITEM_WORN` for different worn item types). Use `dynamic_cast` for specific class identification when subtype matters.

### Container Operations

- Use `*container += *thing` to add items to containers, rooms, or beings.
- Use `--(*thing)` to remove an item from its current container before deletion or relocation.
- Use `*room << *mob` and `*room >> *mob` for born list management in rooms.
- Always remove items (`--(*thing)`) before adding to a new container. Objects must be orphaned (all location pointers null) before re-parenting.
- Container operators perform validation and may trigger side effects like stat modification when equipping items.

### Opinion List Management

- Always iterate and delete the entire `charList` chain manually before destroying an `opinionData` structure.
- Cache the `next` pointer before deleting any `charList` node to avoid use-after-free.
- Call `DeleteHatreds()` and `DeleteFears()` when deleting a character to clean up dangling references across all mobs.
- Never rely on destructors to clean up `charList` chains; they only delete the head node.

### Object Creation

- Use `makeNewObj(itemTypeT)` to create properly-typed object instances from the factory.
- Use `read_mobile()` and `read_object()` to instantiate from prototype data.
- Validate `itemTypeT` values against `MAX_OBJ_TYPES` when processing untrusted input. `makeNewObj()` returns NULL for invalid types.

### Modernization

- Add `override` keyword to all virtual function overrides when touching files.
- Remove `virtual` keyword from overrides; `override` is sufficient and clearer.

## Reference

### Class Hierarchy

```
TThing (abstract base)
  +-- TRoom      (world rooms)
  +-- TBeing     (living entities - abstract)
  |     +-- TMonster  (NPCs)
  |     +-- TPerson   (players)
  +-- TObj       (items - abstract, 40+ subclasses)
```

### TObj Subclass Tree

```
TObj (abstract)
  +-- TBaseWeapon --------+-- TGenWeapon --+-- TGun --+-- THandgonne, TCannon
  |                       +-- TArrow
  +-- TBaseClothing ------+-- TArmor (+TArmorWand with TWand)
  |                       +-- TWorn, TJewelry, TSaddle, THarness
  +-- TBaseContainer -----+-- TOpenContainer --+-- TChest, TCookware, TWagon
  |                       |                    +-- TExpandableContainer --+
  |                       |                         +-- TQuiver, TKeyring, TMoneypouch
  |                       |                         +-- TSaddlebag, TSuitcase, TCardDeck
  |                       |                         +-- TPlant, TTrashPile, TToothNecklace
  |                       +-- TBaseCorpse -----+-- TCorpse, TPCorpse
  +-- TBaseCup -----------+-- TDrinkCon, TPotion, TVial, TPoison, TPool
  +-- TBaseLight ---------+-- TLight, TFFlame
  +-- TMagicItem ---------+-- TScroll, TWand, TStaff
  +-- TMergeable ---------+-- TComponent, TMoney, TCommodity, TGas
  +-- TSeeThru -----------+-- TPortal (+ TVehicle), TWindow
  +-- TFood --------------+-- TEgg, TFruit
  +-- Direct subclasses: TBandage, TBed, TBoard, TBoat, TBook, TBow,
      TCasinoChip, TDrug, TDrugContainer, TFuel, TGemstone, TKey, TNote,
      TOpal, TOrganic, TOtherObj, TPen, TStatue, TSymbol, TTable, TTrash,
      TTrap, TTree, TTreasure, TAudio, TTool, TASubstance
```

### Type Identification Enums

| Enum | Scope | Usage |
|------|-------|-------|
| `thingTypeT` | Legacy coarse | `TYPETHING`, `TYPEOBJ`, `TYPEMOB`, `TYPEPC`, `TYPEROOM`, `TYPEBEING` |
| `TThingKind` | Modern coarse | Accessed via virtual `getKind()` method |
| `itemTypeT` | Object fine | 67+ values, accessed via pure virtual `itemType()` |

### thingTypeT Values

| Value | Enum | Meaning |
|-------|------|---------|
| 0 | `TYPETHING` | Base TThing |
| 1 | `TYPEOBJ` | Any TObj |
| 2 | `TYPEMOB` | TMonster (NPC) |
| 3 | `TYPEPC` | TPerson (player) |
| 4 | `TYPEROOM` | TRoom |
| 5 | `TYPEBEING` | Any TBeing |

### TThingKind Values

`TThing`, `TBeing`, `TMonster`, `TPerson`, `TRoom`, `TObj`, `TComponent`, `TBaseContainer`

### Key itemTypeT Values

`ITEM_UNDEFINED`, `ITEM_LIGHT`, `ITEM_SCROLL`, `ITEM_WAND`, `ITEM_STAFF`, `ITEM_WEAPON`, `ITEM_ARMOR`, `ITEM_POTION`, `ITEM_TRAP`, `ITEM_DRINKCON`, `ITEM_FOOD`, `ITEM_MONEY`, `ITEM_BAG`, `ITEM_CORPSE`, `ITEM_COMPONENT`, `ITEM_PORTAL`, plus 50+ more up to `MAX_OBJ_TYPES`

### Hate/Fear Field Flags

| Flag | Meaning |
|------|---------|
| `HATE_CHAR` / `FEAR_CHAR` | Character list (`clist`) is active |
| `HATE_SEX` / `FEAR_SEX` | Sex-based targeting |
| `HATE_RACE` / `FEAR_RACE` | Race-based targeting |
| `HATE_CLASS` / `FEAR_CLASS` | Class-based targeting |
| `HATE_VNUM` / `FEAR_VNUM` | Vnum-based targeting |

### Hate/Fear API

| Function | Purpose |
|----------|---------|
| `addHated(TBeing*)` | Add character to hate list |
| `remHated(TBeing*, const char*)` | Remove from hate list (memory leak bug) |
| `Hates(TBeing*, const char*)` | Check if mob hates target |
| `addFeared(TBeing*)` | Add character to fear list |
| `remFeared(TBeing*, const char*)` | Remove from fear list |
| `Fears(TBeing*, const char*)` | Check if mob fears target |
| `addHatred(zoneHateT, int)` | Add categorical hate (race, sex, class, vnum) |
| `addFears(zoneHateT, int)` | Add categorical fear |
| `findAHatee()` | Find a hated target in the room |
| `findAFearee()` | Find a feared target in the room |
| `developHatred(TBeing*)` | Potentially add hate based on combat state |
| `DeleteHatreds(const TBeing*, const char*)` | Remove character from all hate lists globally |
| `DeleteFears(const TBeing*, const char*)` | Remove character from all fear lists globally |

### Global Object Lists

| List | Type | Purpose |
|------|------|---------|
| `character_list` | `TBeing*` | All beings (linked via `next`) |
| `object_list` | `TObjList` | All objects |
| `room_db[]` | `TRoom*[]` | Rooms by vnum (size: `WORLD_SIZE`) |
| `mob_index` | `vector<mobIndexData>` | Mobile prototypes |
| `obj_index` | `vector<objIndexData>` | Object prototypes |

### Source Files

| File | Contents |
|------|----------|
| `code/code/misc/thing.h` | TThing base class, TThingKind enum |
| `code/code/misc/being.h` | TBeing abstract class |
| `code/code/misc/monster.h` | TMonster, opinionData, charList structures |
| `code/code/misc/person.h` | TPerson player class |
| `code/code/misc/obj.h` | TObj base class, itemTypeT enum |
| `code/code/misc/room.h` | TRoom class |
| `code/code/sys/db.cc` | makeNewObj factory, prototype loading |
| `code/code/misc/opinion.cc` | Hate/fear list manipulation functions |

## Implementation

### TThing Base Class

TThing provides core identity through `name`, `shortDescr`, and `descr` fields. Location tracking uses `parent` (containing thing) and `in_room` (room vnum). The `stuff` list holds contained items. Physical properties include `weight`, `height`, and `material_type`. Mount relationships use `rider` and `riding` pointers.

The class defines 170+ virtual functions with default implementations. Identity methods include `getName()`, `hshr()`, `hssh()`, and `hmhr()` for pronoun handling. Visibility methods include `canSee()`, `canSeeMe()`, and `listThingRoomMe()`. Containment validation uses `carryWeightLimit()` and `carryVolumeLimit()`. Command handling delegates through `getMe()`, `dropMe()`, `throwMe()`, and `checkSpec()`.

The only pure virtual is `exitDir(dirTypeT)` which returns exit data for rooms and nullptr for non-rooms.

Container operations use operator overloads. Adding to containers uses `*container += *thing`. Removal uses the prefix decrement `--(*thing)` which clears the item from its current location. The `+=` operator dispatches to appropriate addition methods: `thingToRoom()` for room destinations, parent pointer updates for containers, and `equip()` for equipment slots. The `--` operator reverses these operations, calling `unequip()` with `affectModify()` for equipped items.

### TBeing Living Entities

TBeing extends TThing for creatures with stats and actions. Core members include `points` (HP, mana, movement, plus derived values like `max_hit` and `armor`), `equipment[]` array for worn items (slots like `WEAR_HEAD`, `WEAR_BODY` up to `WEAR_MAX`), and `affected` linked list for spell effects managed via `affectJoin()` and `affectModify()`. The `specials` structure tracks position (ranging from `POSITION_DEAD` through `POSITION_STANDING`) and fighting state. Social structures use `master` and `followers` pointers. Action state lives in `spelltask` and `task` structures.

Pure virtual methods that subclasses must implement: `hitGain()`, `manaGain()`, `isDragonRideable()`, `failCharm()`, `learnFromDoing()`, `wizFileSave()`, `doQuit2()`, and various timer methods.

Type conversion helpers wrap `dynamic_cast`: `isTMonster()` and `isTPerson()` return boolean checks, while `toTMonster()` and `toTPerson()` return typed pointers or nullptr.

### TMonster NPC Behavior

TMonster implements NPC behavior through the `opinion` structure (greed, anger, malice, suspicion values) and `hates`/`fears` opinion data structures. Response scripts live in `resps`. Scaling factors include `hpLevel`, `damLevel`, and `acLevel`.

Key methods: `mobileActivity()` drives the AI loop (respecting position, fighting state, and charmed status), `aggro()` checks aggression conditions, and `hunt()` handles target tracking. Over 100 social handler methods respond to player actions.

The `opinionData` structure tracks hate and fear targets through multiple mechanisms. The `clist` member is a linked list of `charList` nodes for specific character targeting. Additional fields enable categorical targeting by `sex`, `race`, `Class` bitmask, or mob `vnum`. The `hatefield` and `fearfield` bitmasks indicate which targeting mechanisms are active.

The `charList` structure contains `name` (dynamically allocated string), `iHateStrength` (duration in game hours), `account_id` and `player_id` for multi-play detection, and `next` pointer for list traversal.

### charList Memory Management Hazard

The `charList` destructor only frees the `name` string; it does NOT recursively delete `next`. The `opinionData` destructor only deletes the head node of `clist`. This requires manual iteration and deletion of the entire chain before destruction.

Safe cleanup requires caching the next pointer before each deletion. Loop through the chain, save `c->next` to a temporary, delete the current node, then advance to the saved next pointer.

Known bug: `remHated()` in `opinion.cc` does NOT delete removed nodes, causing a memory leak. Compare with `remFeared()` which correctly deletes removed nodes.

### TPerson Players

TPerson extends TBeing for player characters. The `title` field holds the player's custom title. The `wizPowers[]` array tracks immortal permissions. Persistence methods `saveRent()` and `loadRent()` handle equipment storage. Level advancement uses `advanceLevel()`.

The `learnFromDoing()` method awards practice gains after successful skill/spell usage based on difficulty, failure rate, and current proficiency. Learning rates vary by class and specialization.

### TObj Items

TObj extends TThing with `obj_flags` (extra flags like `ITEM_GLOW`, wear flags like `ITEM_WEAR_BODY`, cost), `affected[]` array (`MAX_OBJ_AFFECT` entries) for item enchantments, and `owners` for ownership tracking.

Pure virtuals that all subclasses implement: `itemType()` returns the `itemTypeT` enum value, `assignFourValues()` and `getFourValues()` handle DB persistence, `statObjInfo()` generates stat output. Additional virtuals include `lowCheck()` for condition tracking and `updateDesc()` for state-dependent descriptions.

The subclass hierarchy organizes items through intermediate abstract classes: TBaseWeapon for all weapons, TBaseClothing for wearables, TBaseContainer for anything that holds items, TBaseCup for drinkables, TBaseLight for light sources, TMagicItem for castable items, TMergeable for stackable items, TSeeThru for things that can be looked through, and TFood for edibles.

### TRoom World Locations

TRoom represents static world locations with `sectorType` for terrain (`SECT_CITY`, `SECT_FOREST`, `SECT_WATER_SWIM`, etc. affecting movement costs and drowning), `dir_option[]` array for exits, `zone` for area membership, and `roomFlags` for room properties (`ROOM_DEATH` for death traps, `ROOM_INDOORS` for weather protection, `ROOM_PEACEFUL` preventing combat). River mechanics use `riverDir` and `riverSpeed`. Teleport rooms use `teleTarg` and `teleTime`.

Born list management uses stream operators: `*room << *mob` adds to the born list, `*room >> *mob` removes from it.

### Object Factory

The `makeNewObj(itemTypeT)` function in `db.cc` creates properly-typed object instances. It uses an internal switch statement mapping each `itemTypeT` value to its corresponding class constructor. Unrecognized types return NULL (nullptr). After creation, use `dynamic_cast` to access subclass-specific methods.

Room access validation uses `real_roomp()` which checks vnum bounds and null entries, returning nullptr for invalid values.

### Legacy Type System

The older `thingTypeT` enum provides coarse-grained type identification with six values covering the major branches. Some legacy code paths still use this system.

The modern `TThingKind` enum class provides type-safe identification through the virtual `getKind()` method. Each major class overrides this to return its specific kind value.

## Troubleshooting

### Symptom: Crash when deleting TMonster with hate/fear lists

**Cause:** The `charList` chain was not fully cleaned up before destruction.

**Diagnostic:** Check if `hates.clist` or `fears.clist` have multiple nodes. The destructor only deletes the head.

**Fix:** Before destruction, iterate the entire chain with next-pointer caching and delete each node individually.

### Symptom: Memory leak in mob hate tracking

**Cause:** `remHated()` removes nodes from the list but does not delete them.

**Diagnostic:** Use memory profiler to track `charList` allocations. Compare with `remFeared()` which correctly deletes. Valgrind reports leaked charList nodes with allocation traces pointing to addHated.

**Fix:** This is a known bug. Add `delete list` call in `remHated()` after unlinking the node.

### Symptom: Wrong object type after makeNewObj

**Cause:** Incorrect `itemTypeT` passed to factory, or missing `dynamic_cast` after creation.

**Diagnostic:** Check the `itemType()` return value matches expectations. Verify the switch in `makeNewObj()` maps to the correct class. Note that invalid types return NULL (nullptr).

**Fix:** Use correct enum value and always `dynamic_cast` to the expected subclass before accessing subclass methods.

### Symptom: Virtual function not being called on subclass

**Cause:** Missing `override` keyword allowed signature mismatch, creating a new function instead of overriding.

**Diagnostic:** Add `override` to the subclass method. Compiler error indicates signature mismatch. The function may work when called directly on the derived object but fail through base pointer (direct calls resolve statically, pointer calls use vtable).

**Fix:** Correct the function signature to match the base class exactly. Add `override` to prevent future regressions.

### Symptom: Character remains in hate lists after deletion

**Cause:** `DeleteHatreds()` or `DeleteFears()` not called before character deletion.

**Diagnostic:** Mob continues to reference deleted character by name or ID.

**Fix:** Call both cleanup functions when deleting any character to remove dangling references from all mobs.

### Symptom: dynamic_cast returns nullptr unexpectedly

**Cause:** Object is not actually the expected subclass type.

**Diagnostic:** Check `itemType()` or `getKind()` before casting. Use `isTMonster()`/`isTPerson()` for beings. Note that `getKind()` returning unexpected values may indicate vtable corruption from use-after-free.

**Fix:** Always verify type before casting. Handle nullptr case gracefully.

### Symptom: Container operation crashes or creates inconsistent state

**Cause:** Object added to container without first removing from previous location.

**Diagnostic:** Object appears in multiple container lists, or room transition fails with assertion about existing `roomp`.

**Fix:** Call `--(*object)` before adding to a new container. Objects must have null `parent`, `equippedBy`, `stuckIn`, and `roomp` pointers before addition.

### Symptom: Room access crashes with null pointer

**Cause:** Invalid vnum or unloaded zone.

**Diagnostic:** `room_db[]` access with invalid index returns nullptr.

**Fix:** Validate vnums through `real_roomp()` checking return for nullptr before dereference.
