---
title: SneezyMUD Class Hierarchy
description: Complete object hierarchy from TThing base class through TBeing, TMonster, TPerson, TRoom, and 40+ TObj subclasses with runtime type identification.
keywords: [TThing, TBeing, TMonster, TPerson, TRoom, TObj, itemType, getKind, thingTypeT, TThingKind, virtual functions, dynamic_cast, opinionData, charList, hates, fears, makeNewObj, character_list, object_list, room_db, mob_index, obj_index]
category: Critical Systems

last_updated: 2026-01-29
source_files: [code/code/misc/thing.h, code/code/misc/being.h, code/code/misc/monster.h, code/code/misc/person.h, code/code/misc/obj.h, code/code/misc/room.h, code/code/misc/db.cc]
related: [character-foundation.md, memory-safety.md, memory-management.md]
---

# SneezyMUD Class Hierarchy

## Overview

All world objects derive from `TThing`, the abstract base class representing anything
that exists in the game world: rooms, beings (players and NPCs), and objects (items).

```
TThing (abstract base)
  +-- TRoom      (world rooms)
  +-- TBeing     (living entities - abstract)
  |     +-- TMonster  (NPCs)
  |     +-- TPerson   (players)
  +-- TObj       (items - abstract, 40+ subclasses)
```

## TThing - The Root

**Header:** `code/code/misc/thing.h`

TThing provides identity (`name`, `shortDescr`, `descr`), location (`parent`, `in_room`),
contents (`stuff` list), physical properties (`weight`, `height`, `material_type`),
and mounting (`rider`, `riding`).

### Virtual Functions

TThing defines 170+ virtual functions with default implementations. Key categories:
- **Identity:** `getName()`, `hshr()`, `hssh()`, `hmhr()`
- **Visibility:** `canSee()`, `canSeeMe()`, `listThingRoomMe()`
- **Containment:** `carryWeightLimit()`, `carryVolumeLimit()`
- **Commands:** `getMe()`, `dropMe()`, `throwMe()`, `checkSpec()`

**Pure virtual:** `exitDir(dirTypeT)` - Returns exit data (nullptr for non-rooms).

### Container Operators

```cpp
*room += *player;     // Add player to room (operator+=)
--(*item);            // Remove item from its container (operator--)
```

## TBeing - Living Entities

**Header:** `code/code/misc/being.h`

TBeing extends TThing for creatures with stats, combat, and actions.

**Key members:** `points` (HP/mana/move), `equipment[]`, `affected` (spell effects),
`specials` (position/fighting), `master`/`followers`, `spelltask`/`task`.

**Pure virtuals:** `hitGain()`, `manaGain()`, `isDragonRideable()`, `failCharm()`,
`learnFromDoing()`, `wizFileSave()`, `doQuit2()`, timer methods.

## TMonster - NPCs

**Header:** `code/code/misc/monster.h`

NPCs with AI: `opinion` (greed/anger/malice/suspicion), `hates`/`fears` lists,
`resps` (response scripts), scaling factors (`hpLevel`, `damLevel`, `acLevel`).

Key methods: `mobileActivity()`, `aggro()`, `hunt()`, 100+ social handlers.

### opinionData and charList Structures

TMonster tracks hate and fear targets via `opinionData` structures (`hates`, `fears`), which can target characters by name, sex, race, class, or vnum.

```cpp
class opinionData {
    charList* clist;    // Linked list of hated/feared character names
    sexTypeT sex;       // Hated/feared sex
    race_t race;        // Hated/feared race
    short Class;        // Hated/feared class bitmask
    short vnum;         // Hated/feared mob vnum
};

class charList {
    const char* name;       // Character name (dynamically allocated)
    long iHateStrength;     // Hate duration in game hours
    int account_id;         // For multi-play detection
    int player_id;          // Player ID
    charList* next;         // Next node in linked list
};
```

**Source:** `code/code/misc/monster.h` (lines 27-54)

The `hatefield` and `fearfield` bitmasks track which hate/fear types are active:

| Flag | Meaning |
|------|---------|
| `HATE_CHAR` / `FEAR_CHAR` | Character list (`clist`) is active |
| `HATE_SEX` / `FEAR_SEX` | Sex-based targeting |
| `HATE_RACE` / `FEAR_RACE` | Race-based targeting |
| `HATE_CLASS` / `FEAR_CLASS` | Class-based targeting |
| `HATE_VNUM` / `FEAR_VNUM` | Vnum-based targeting |

### opinionData Memory Management

**CRITICAL: The `charList` linked list requires manual cleanup.**

The `opinionData` destructor only deletes the **head** node of the `clist`. The `charList` destructor does NOT recursively delete `next`. This design requires callers to manually iterate and delete all nodes before destruction.

```cpp
// From monster.cc - note the warning comment
// warning: you must remember to manually delete opinionData::next in a loop
opinionData::~opinionData() {
    delete clist;   // Only deletes head node!
    clist = NULL;
}

charList::~charList() {
    delete[] name;  // Only frees name string, NOT next pointer
    name = NULL;
}
```

**Safe cleanup pattern (from `charList::operator=`):**

```cpp
charList *c, *n;
for (c = clist; c; c = n) {
    n = c->next;    // Cache next before delete
    delete c;       // Delete current node
}
clist = NULL;
```

**Known issue:** `remHated()` in `opinion.cc` does NOT delete removed nodes (memory leak). Compare with `remFeared()` which correctly calls `delete list` on removed nodes.

### Hate/Fear API

| Function | Purpose |
|----------|---------|
| `addHated(TBeing*)` | Add character to hate list |
| `remHated(TBeing*, const char*)` | Remove from hate list (leaks memory!) |
| `Hates(TBeing*, const char*)` | Check if mob hates target |
| `addFeared(TBeing*)` | Add character to fear list |
| `remFeared(TBeing*, const char*)` | Remove from fear list |
| `Fears(TBeing*, const char*)` | Check if mob fears target |
| `addHatred(zoneHateT, int)` | Add categorical hate (race, sex, class, vnum) |
| `addFears(zoneHateT, int)` | Add categorical fear |
| `findAHatee()` | Find a hated target in the room |
| `findAFearee()` | Find a feared target in the room |
| `developHatred(TBeing*)` | Potentially add hate based on combat state |

**Global cleanup functions:**

```cpp
void DeleteHatreds(const TBeing* ch, const char* s);  // Remove ch from all hate lists
void DeleteFears(const TBeing* ch, const char* s);    // Remove ch from all fear lists
```

These are called when a character is deleted to clean up dangling references across all mobs.

## TPerson - Players

**Header:** `code/code/misc/person.h`

Player characters with `title`, `wizPowers[]`, persistence (`saveRent()`/`loadRent()`),
and level progression (`advanceLevel()`).

## TObj Subclass Hierarchy

**Header:** `code/code/misc/obj.h`

TObj adds `obj_flags` (extra/wear flags, cost), `affected[]` array, `owners`.
**Pure virtuals:** `itemType()`, `assignFourValues()`, `getFourValues()`, `statObjInfo()`.

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

## TRoom - World Rooms

**Header:** `code/code/misc/room.h`

Rooms have `sectorType`, `dir_option[]` (exits), `zone`, `roomFlags`,
river flow (`riverDir`/`riverSpeed`), teleport (`teleTarg`/`teleTime`).

**Born list operators:** `*room << *mob` (add), `*room >> *mob` (remove).

## Global Lists

| List | Type | Purpose |
|------|------|---------|
| `character_list` | `TBeing*` | All beings (linked via `next`) |
| `object_list` | `TObjList` | All objects |
| `room_db[]` | `TRoom*[]` | Rooms by vnum |
| `mob_index` | `vector<mobIndexData>` | Mobile prototypes |
| `obj_index` | `vector<objIndexData>` | Object prototypes |

## Runtime Type Identification

With the class hierarchy established above, SneezyMUD provides several mechanisms
for determining object and being types at runtime. These are essential for
implementing type-specific behavior throughout the codebase.

### Dual Enum Systems

#### Legacy: `thingTypeT` (thing.h)

```cpp
enum thingTypeT {
  TYPETHING,  // 0 - base TThing
  TYPEOBJ,    // 1 - any TObj
  TYPEMOB,    // 2 - TMonster (NPC)
  TYPEPC,     // 3 - TPerson (player)
  TYPEROOM,   // 4 - TRoom
  TYPEBEING,  // 5 - any TBeing
};
```

Coarse-grained identification used in some older code paths.

#### Modern: `TThingKind` (thing.h, enum class)

```cpp
enum class TThingKind {
  TThing, TBeing, TMonster, TPerson, TRoom,
  TObj, TComponent, TBaseContainer,
};
```

Type-safe identification accessed via the virtual `getKind()` method:

```cpp
// Base declaration in TThing
virtual TThingKind getKind() const;

// Override examples
TThing::TThingKind TMonster::getKind() const { return TThingKind::TMonster; }
TThing::TThingKind TObj::getKind() const { return TThingKind::TObj; }
```

### Object-Specific Types: `itemTypeT` (obj.h)

Fine-grained identification for TObj subclasses (67+ values):

```cpp
enum itemTypeT {
  ITEM_UNDEFINED, ITEM_LIGHT, ITEM_SCROLL, ITEM_WAND, ITEM_STAFF,
  ITEM_WEAPON, ITEM_ARMOR, ITEM_POTION, ITEM_TRAP, ITEM_DRINKCON,
  ITEM_FOOD, ITEM_MONEY, ITEM_BAG, ITEM_CORPSE, ITEM_COMPONENT,
  ITEM_PORTAL, /* ... 50+ more ... */ MAX_OBJ_TYPES
};
```

Each TObj subclass implements the pure virtual `itemType()`:

```cpp
// TObj (pure virtual)
virtual itemTypeT itemType() const = 0;

// Subclass implementations (note: missing override keyword - see modernization note)
virtual itemTypeT itemType() const { return ITEM_WEAPON; }  // TGenWeapon
virtual itemTypeT itemType() const { return ITEM_BAG; }     // TBag
```

### TBeing Utility Methods

TBeing provides convenient wrappers around `dynamic_cast`:

```cpp
// Type checking
bool isTMonster() const;  // returns dynamic_cast<const TMonster*>(this) != nullptr
bool isTPerson() const;

// Type conversion (returns nullptr if wrong type)
TMonster* toTMonster();
const TMonster* toTMonster() const;
TPerson* toTPerson();
const TPerson* toTPerson() const;
```

### When to Use Each Method

| Scenario | Recommended Approach |
|----------|---------------------|
| Check if TBeing is monster/player | `isTMonster()` / `isTPerson()` |
| Convert TBeing to specific type | `toTMonster()` / `toTPerson()` |
| Check TObj subclass type | `obj->itemType() == ITEM_WEAPON` |
| Check broad category | `thing->getKind() == TThingKind::TObj` |
| Need subclass methods | `dynamic_cast<TSpecificClass*>(thing)` |

### Example Patterns

```cpp
// TBeing type checking and conversion
void processTarget(TBeing* target) {
  if (target->isTMonster()) {
    TMonster* mob = target->toTMonster();
    mob->doMobSpecificThing();
  }
}

// TObj type checking
void handleObject(TObj* obj) {
  if (obj->itemType() == ITEM_WEAPON) {
    auto* weapon = dynamic_cast<TGenWeapon*>(obj);
    weapon->doWeaponThing();
  }
}

// Broad category via getKind()
void processThing(TThing* thing) {
  if (thing->getKind() == TThing::TThingKind::TMonster) {
    // Handle monster
  }
}
```

### Object Factory: `makeNewObj()` (db.cc)

Creates TObj instances by itemTypeT:

```cpp
TObj* makeNewObj(itemTypeT type);

// Usage
TObj* weapon = makeNewObj(ITEM_WEAPON);  // Returns new TGenWeapon()
auto* genWeapon = dynamic_cast<TGenWeapon*>(weapon);
```

Internally uses a switch mapping each `itemTypeT` to its class constructor.

### Modernization Note: Missing `override` Keywords

**Critical:** 99% of virtual overrides lack the `override` keyword.

Current (legacy):
```cpp
virtual itemTypeT itemType() const { return ITEM_WEAPON; }
virtual TThingKind getKind() const;
```

Should be (modern C++):
```cpp
itemTypeT itemType() const override { return ITEM_WEAPON; }
TThingKind getKind() const override;
```

Benefits of `override`:
- Compile-time verification of actual override
- Protection against signature mismatches
- Clearer code intent

This affects nearly all TObj and TBeing subclasses - a significant modernization opportunity.

## Memory Management

Objects allocated with `new` are managed via the DELETE_* flag system (see
separate documentation). Check `IS_SET_DELETE()` return values before reusing
pointers. Use `read_mobile()`/`read_object()` to create from prototypes.
