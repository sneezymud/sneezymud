---
title: Object System
description: Complete object type system including the TObj class hierarchy, container mechanics, consumables (food/drink/light), and utility types (vehicles/books/furniture) with DELETE flag safety patterns
keywords: [itemTypeT, TObj, makeNewObj, assignFourValues, val0-val3, DELETE_THIS, TBaseContainer, TOpenContainer, CONT_LOCKED, CONT_TRAPPED, has_key, pickMe, TLight, TFood, TDrinkCon, TBaseCup, lightDecay, eatMe, drinkMe, condTypeT, TVehicle, TAudio, TBook, TBed, bedRegen, objectDecay, StuffIter]
category: Understanding Systems

last_updated: 2026-01-29
source_files: [code/code/misc/obj.h, code/code/obj/obj_base_container.cc, code/code/obj/obj_food.cc, code/code/obj/obj_light.cc]
related:
  - memory-safety.md
  - spatial-relationships.md
  - trap-mechanics.md
  - affects-system.md
  - material-system.md
  - scheduler-pulses.md
---

# Object System

This document describes the complete SneezyMUD object system, including the type hierarchy, container mechanics, consumable items, and utility object types.

## I. Object Type System Foundation

Every object in SneezyMUD has a type defined by the `itemTypeT` enum. The base class `TObj` declares a pure virtual method `itemType()` that each concrete subclass implements to return its type constant. The object factory `makeNewObj(itemTypeT)` instantiates the correct subclass based on the type.

**Source files:**
- `/code/code/misc/obj.h` - `itemTypeT` enum, `TObj` base class
- `/code/code/sys/db.cc` - `makeNewObj()` factory function
- `/code/code/obj/` - Individual object type implementations

### Class Hierarchy

```
TObj (abstract base)
|
+-- TBaseWeapon (abstract)
|   +-- TGenWeapon (ITEM_WEAPON)
|   +-- TArrow (ITEM_ARROW)
|
+-- TBaseClothing (abstract)
|   +-- TArmor (ITEM_ARMOR)
|   +-- TWorn (ITEM_WORN)
|   +-- TSaddle (ITEM_SADDLE)
|   +-- THarness (ITEM_HARNESS)
|
+-- TBaseLight (abstract)
|   +-- TLight (ITEM_LIGHT)
|   +-- TFFlame (ITEM_FLAME)
|
+-- TBaseCup (abstract)
|   +-- TDrinkCon (ITEM_DRINKCON)
|   +-- TPotion (ITEM_POTION)
|   +-- TVial (ITEM_VIAL)
|   +-- TPool (ITEM_POOL)
|
+-- TBaseContainer (abstract)
|   +-- TOpenContainer (abstract)
|   |   +-- TChest (ITEM_CHEST)
|   |   +-- TCookware (ITEM_COOKWARE)
|   |   +-- TWagon (ITEM_WAGON)
|   |   +-- TExpandableContainer (abstract)
|   |       +-- TBag (ITEM_BAG)
|   |       +-- TQuiver (ITEM_QUIVER)
|   |       +-- TSpellBag (ITEM_SPELLBAG)
|   |       +-- TKeyring (ITEM_KEYRING)
|   |       +-- TMoneypouch (ITEM_MONEYPOUCH)
|   |       +-- TSaddlebag (ITEM_SADDLEBAG)
|   |       +-- TSuitcase (ITEM_SUITCASE)
|   |       +-- TCardDeck (ITEM_CARD_DECK)
|   |       +-- TToothNecklace (ITEM_TOOTH_NECKLACE)
|   |       +-- TTrashPile (ITEM_TRASH_PILE)
|   +-- TBaseCorpse (abstract)
|       +-- TCorpse (ITEM_CORPSE)
|       +-- TPCorpse (ITEM_PCORPSE)
|
+-- TMagicItem (abstract)
|   +-- TScroll (ITEM_SCROLL)
|   +-- TWand (ITEM_WAND)
|   +-- TStaff (ITEM_STAFF)
|
+-- TSeeThru (abstract)
|   +-- TPortal (ITEM_PORTAL)
|   +-- TWindow (ITEM_WINDOW)
|
+-- TFood (ITEM_FOOD)
|   +-- TFruit (ITEM_FRUIT)
|   +-- TEgg (ITEM_EGG)
|
+-- (Direct TObj subclasses)
    TBow, TComponent, TTool, TSymbol, TTrap, TBed, TTable,
    TMoney, TBoat, TAudio, TBoard, TBook, TTree, TNote, TPen,
    TKey, TBandage, TStatue, TFuel, TOpal, TTreasure, TTrash,
    TOtherObj, TGemstone, TJewelry, TCommodity, TOrganic,
    TAppliedSub, TGas, TDrugContainer, TDrug, TGun, TAmmo,
    TPlant, TVehicle, TCasinoChip, TPoison, THandgonne, TCannon
```

### itemTypeT Enumeration

| Constant | Value | Class | Description |
|----------|-------|-------|-------------|
| `ITEM_UNDEFINED` | 0 | - | Invalid/uninitialized |
| `ITEM_LIGHT` | 1 | TLight | Refillable light sources (lamps, torches) |
| `ITEM_SCROLL` | 2 | TScroll | Magical scrolls with up to 3 spells |
| `ITEM_WAND` | 3 | TWand | Charged magical wands |
| `ITEM_STAFF` | 4 | TStaff | Charged magical staves |
| `ITEM_WEAPON` | 5 | TGenWeapon | General melee weapons |
| `ITEM_ARMOR` | 9 | TArmor | Protective armor |
| `ITEM_FOOD` | 19 | TFood | Edible food items |
| `ITEM_DRINKCON` | 17 | TDrinkCon | Drink containers (cups, bottles) |
| `ITEM_CHEST` | 15 | TChest | Lockable storage containers |
| `ITEM_BAG` | 27 | TBag | Expandable containers (bags, sacks) |
| `ITEM_CORPSE` | 28 | TCorpse | NPC corpses |
| `ITEM_PCORPSE` | 47 | TPCorpse | Player corpses |
| `ITEM_PORTAL` | 32 | TPortal | Magical portals |
| `ITEM_BED` | 40 | TBed | Resting furniture |
| `ITEM_VEHICLE` | 61 | TVehicle | Vehicles/mounts |
| `ITEM_AUDIO` | 23 | TAudio | Sound-producing items |
| `ITEM_BOOK` | 31 | TBook | Readable books |

(See complete enumeration in source code)

### Value Fields (val0-val3)

Objects have four integer value fields whose meanings vary by type. These are assigned via `assignFourValues()` and retrieved via `getFourValues()`.

**Weapons (TBaseWeapon)**
| Field | Meaning |
|-------|---------|
| val0 | Sharpness: curSharp (bits 0-7), maxSharp (bits 8-15) |
| val1 | Damage: damLevel (bits 0-7), damDev (bits 8-15) |
| val2 | Weapon types and frequencies (TGenWeapon) |
| val3 | Arrow-specific data (TArrow) |

**Light Sources (TBaseLight)**
| Field | Meaning |
|-------|---------|
| val0 | Light amount emitted |
| val1 | Maximum burn time |
| val2 | Current burn time remaining |
| val3 | Lit status (TLight), magic flags (TFFlame) |

**Containers (TOpenContainer)**
| Field | Meaning |
|-------|---------|
| val0 | Maximum carry weight |
| val1 | Container flags, trap type, trap damage |
| val2 | Key vnum for locked containers |
| val3 | Maximum carry volume |

**Drinks (TBaseCup)**
| Field | Meaning |
|-------|---------|
| val0 | Maximum drink units |
| val1 | Current drink units |
| val2 | Liquid type (liqTypeT) |
| val3 | Drink flags (frozen, poisoned, etc.) |

**Magic Items (TMagicItem)**
| Field | Meaning |
|-------|---------|
| val0 | Magic level |
| val1 | Max charges (wand/staff), spell 1 (scroll) |
| val2 | Cur charges (wand/staff), spell 2 (scroll) |
| val3 | Spell number (wand/staff), spell 3 (scroll) |

**Food (TFood)**
| Field | Meaning |
|-------|---------|
| val0 | Fullness value |
| val3 | Food flags (poisoned, spoiled, etc.) |

### Object Flags

**Action Flags (extra_flags)** - `ITEM_*` constants controlling object behavior:

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

**Wear Flags (wear_flags)** - `ITEM_WEAR_*` constants controlling where items can be worn:

| Flag | Body Location |
|------|---------------|
| `ITEM_WEAR_TAKE` | Can be picked up |
| `ITEM_WEAR_FINGERS` | Finger slot |
| `ITEM_WEAR_NECK` | Neck slot |
| `ITEM_WEAR_BODY` | Body slot |
| `ITEM_WEAR_HEAD` | Head slot |
| `ITEM_WEAR_LEGS` | Leg slot |
| `ITEM_WEAR_FEET` | Feet slot |
| `ITEM_WEAR_HANDS` | Hand slot |
| `ITEM_WEAR_ARMS` | Arm slot |
| `ITEM_WEAR_BACK` | Back slot |
| `ITEM_WEAR_WAIST` | Waist slot |
| `ITEM_WEAR_WRISTS` | Wrist slot |
| `ITEM_WEAR_HOLD` | Can be held |
| `ITEM_WEAR_THROW` | Can be thrown |

### Object Factory

The `makeNewObj(itemTypeT)` function in `/code/code/sys/db.cc` creates objects:

```cpp
TObj* makeNewObj(itemTypeT type) {
  switch (type) {
    case ITEM_LIGHT:   return new TLight();
    case ITEM_WEAPON:  return new TGenWeapon();
    case ITEM_ARMOR:   return new TArmor();
    // ... etc
  }
}
```

After creation, the object is populated from database values including:
- Name, descriptions, extra descriptions
- Action and wear flags
- Four value fields via `assignFourValues()`
- Weight, cost, material, structure points
- Affects and modifiers

### Object Lifecycle

**Creation Patterns**

Objects are created through three primary mechanisms:

**1. Factory Creation via `read_object()`**

The standard way to create objects from database definitions:

```cpp
TObj* obj = read_object(vnum, VIRTUAL);
if (!obj) {
    // Handle failure - vnum doesn't exist or factory failed
    return;
}
*room += *obj;  // Place in world
```

`read_object()` internally calls `makeNewObj(itemTypeT)` to instantiate the correct subclass, then populates it from the database cache or a fresh query.

**2. Direct Construction**

Used for temporary or programmatically-generated objects:

```cpp
TCorpse* corpse = new TCorpse();
// Manually populate fields...
corpse->name = "corpse goblin";
*room += *corpse;
```

**3. Copy Construction**

Used when duplicating objects (rare):

```cpp
TObj* copy = new TObj(*original);  // Copies flags, affects, etc.
```

The copy constructor (`TObj::TObj(const TObj&)`) handles:
- Copying `obj_flags` and `affected[]` array
- Duplicating strung descriptions if `ITEM_STRUNG` is set
- Registering the new object in `object_list`
- Incrementing `objCount`

**Constructor Initialization**

The `TObj` default constructor (`structs.cc:357-369`) performs minimal initialization:

```cpp
TObj::TObj() :
  TThing(),
  obj_flags(),
  action_description(NULL),
  owners(NULL),
  isTasked(false),
  isLocked(false) {
  number = -1;        // Indicates not yet associated with prototype
  objCount++;         // Global object counter
  object_list.push_front(this);  // Register in global list
}
```

**Critical:** Every `TObj` is automatically added to `object_list` on construction and removed on destruction. This enables the scheduler to process all objects.

**Destruction Chain**

Object destruction follows the C++ destructor chain: derived class destructor runs first, then base classes.

**TObj Destructor (`structs.cc:371-468`)**

1. Calls `CMD_GENERIC_DESTROYED` spec proc if object has one
2. Removes from casting list if applicable
3. **Recursively deletes all contents** (cascading deletion)
4. Removes self from parent/room/equipment/stuckIn
5. Dismounts any riders
6. Cancels any tasks using this object (expensive scan)
7. Removes from `object_list`
8. Decrements prototype count in `obj_index`
9. Cleans up strung descriptions

**Automatic Deletion Triggers**

Objects are automatically deleted in these scenarios:

| Trigger | Location | Mechanism |
|---------|----------|-----------|
| Decay timer reaches 0 | `periodic.cc:1943-1979` | `objectDecay()` returns `DELETE_THIS` |
| Scheduler proc returns true | `process.cc:130-131` | `runObj()` deletes object |
| Container deleted | `TObj::~TObj()` | Cascading deletion of `stuff` |
| Room deleted | `TRoom::~TRoom()` | Deletes non-PC contents |
| Zone purge | Various | Administrative cleanup |

**Object Decay System**

Objects with `obj_flags.decay_time > -1` participate in the decay system:

```cpp
// In TObj::updateObj() - called each tick
if (obj_flags.decay_time > -1) {
    decayMe();  // Decrements decay_time
    if (!obj_flags.decay_time) {
        int rc = objectDecay();  // Virtual - type-specific decay behavior
        if (rc)
            return rc;  // Usually DELETE_THIS
        // Default: relocate contents, then DELETE_THIS
    }
}
```

Type-specific `objectDecay()` overrides handle special cases:
- `TBaseCorpse::objectDecay()` - Shows decay message, logs player corpse info
- `TLight::objectDecay()` - Burns out message
- `TFood::objectDecay()` - Spoilage message
- `TPortal::objectDecay()` - Simple `DELETE_THIS`

## II. Container Objects

Containers in SneezyMUD manage objects that can hold other objects, using a sophisticated spatial relationship model with weight/volume calculations, lock/trap mechanics, and special handling for nested containment.

**Misusing this system causes crashes and item duplication bugs.** Common errors: not checking parent pointers before nesting, ignoring DELETE flags from trap triggers, allowing infinite nesting loops, forgetting TComponent weight reduction.

### Container Class Hierarchy

```
TObj (base for all objects)
  |
  +-- TBaseContainer (abstract base - pure virtual interface)
        |
        +-- TOpenContainer (concrete - openable/closeable with locks/traps)
              |
              +-- TChest (lockable storage)
              +-- TCookware (cooking container)
              +-- TWagon (wheeled storage)
              +-- TExpandableContainer (abstract - bags/sacks)
                    |
                    +-- TBag, TQuiver, TSpellBag, TKeyring, TMoneypouch
                    +-- TSaddlebag, TSuitcase, TCardDeck, TToothNecklace
                    +-- TTrashPile, TPlant
        |
        +-- TBaseCorpse (abstract - corpse containers)
              |
              +-- TCorpse (NPC corpses)
              +-- TPCorpse (player corpses)
```

### TBaseContainer Interface

**Source:** `code/code/obj/obj_base_container.h`

```cpp
class TBaseContainer : public virtual TObj {
  public:
    // Pure virtual - subclasses must implement
    virtual void assignFourValues(int, int, int, int) = 0;
    virtual void getFourValues(int*, int*, int*, int*) const = 0;
    virtual int putSomethingInto(TBeing*, TThing*) = 0;
    virtual void lookObj(TBeing*, int) const = 0;

    // Volume calculations
    virtual int getReducedVolume(const TThing*) const;
    virtual int getCarriedVolume() const;

    // Utility methods
    int isSaddle() const;
    TObj* findObjectInContainer(int vnum) const;
};
```

**Key responsibilities:**
- Define container interface contract
- Provide volume calculation implementations
- Handle container-specific serialization
- Manage containment operations

### TOpenContainer Implementation

**Source:** `code/code/obj/obj_open_container.h`

```cpp
class TOpenContainer : public TBaseContainer {
  private:
    float max_weight;           // Maximum weight capacity
    int container_flags;        // CONT_* flag bitvector
    doorTrapT trap_type;        // Trap type if trapped
    char trap_dam;              // Trap damage amount
    int key_num;                // Key vnum for lock
    int max_volume;             // Maximum volume capacity

  public:
    // Lock mechanics
    void pickMe(TBeing*);
    void lockMe(TBeing*);
    void unlockMe(TBeing*);

    // Trap mechanics
    int trapMe(TBeing*, const char*);
    int disarmMe(TBeing*);

    // Open/close
    int openMe(TBeing*);
    void closeMe(TBeing*);

    // Flag manipulation
    bool isContainerFlag(containerFlagT r) const;
    void addContainerFlag(containerFlagT r);
    void remContainerFlag(containerFlagT r);
};
```

### Spatial Relationship System

Containers use the TThing spatial relationship model to track containment:

```
TRoom (root)
  |
  +-- parent = NULL
  +-- roomp = this room
  +-- stuff = [TBeing, TObj, ...]
        |
        +-- TBeing (branch)
        |     |
        |     +-- parent = room
        |     +-- roomp = room
        |     +-- stuff = [inventory items]
        |           |
        |           +-- TOpenContainer (branch)
        |                 |
        |                 +-- parent = being
        |                 +-- roomp = NULL
        |                 +-- stuff = [contained items]
        |                       |
        |                       +-- TObj (leaf)
        |                             |
        |                             +-- parent = container
        |                             +-- roomp = NULL
        |                             +-- stuff = []
```

**Key Spatial Members** (`code/code/misc/thing.h`):

```cpp
class TThing {
  protected:
    TThing* parent;         // Container/room/being holding this thing
    TRoom* roomp;          // Shortcut to room (NULL if inside container)
    int in_room;           // Room vnum (Room::NOWHERE if not in room)
    StuffList stuff;       // std::list<TThing*> of contained items
    TThing* rider;         // First thing riding on this thing
    TThing* riding;        // Thing this thing is riding on
    TThing* nextRider;     // Next in rider chain (linked list)
    TThing* equippedBy;    // Being wearing this item (equipment only)
    TThing* stuckIn;       // Being this item is stuck in (arrows/spears)
};
```

**Container Invariants** - Critical relationships that must stay consistent:

| Relationship | Forward Pointer | Back Pointer |
|--------------|-----------------|--------------|
| In container | `item->parent = container` | `container->stuff` contains item |
| In room | `item->parent = NULL`, `item->roomp = room` | `room->stuff` contains item |
| In inventory | `item->parent = being` | `being->stuff` contains item |

**CRITICAL:** When an item is inside a container, `item->roomp` is `NULL`. Use `roomOfObject()` to find the actual room by traversing the parent chain.

**Container Operators:**

```cpp
*container += *item;  // Calls TThing::operator+=(TThing&)
```

Flow:
1. Validates item not already contained elsewhere (parent, equippedBy, stuckIn must be NULL)
2. Sets `item->parent = container`
3. Clears `item->roomp = NULL`
4. Adds item to `container->stuff` list
5. Handles TMergeable merging if applicable

```cpp
--(*item);  // Calls TThing::operator--()
```

Flow:
1. Removes item from `container->stuff` list
2. Clears `item->parent = NULL`
3. Clears `item->roomp = NULL`
4. Clears `item->in_room = Room::NOWHERE`

### Weight and Volume Calculations

**Weight Calculation Chain** (`code/code/misc/thing.cc`):

```cpp
// Top-level call
float TThing::getTotalWeight(bool pweight) const {
    const TOpenContainer* toc;
    float calc = 0;

    // Special handling for CONT_WEIGHTLESS containers
    if ((toc = dynamic_cast<const TOpenContainer*>(this)) &&
        toc->isContainerFlag(CONT_WEIGHTLESS))
        calc = 0;  // Container's contents weigh nothing!
    else
        calc = getCarriedWeight();

    if (pweight)
        calc += getWeight();  // Add container's own weight

    return calc;
}

// Recursive calculation
float TThing::getCarriedWeight() const {
    TThing* t;
    float total = 0;

    // Add weight of all riders
    for (t = rider; t; t = t->nextRider) {
        total += t->getTotalWeight(true);
    }

    // Add weight of all contained items
    for (StuffIter it = stuff.begin(); it != stuff.end() && (t = *it); ++it) {
        if (dynamic_cast<TComponent*>(t))
            total += (t->getTotalWeight(true) * 0.10);  // 10% reduction!
        else
            total += t->getTotalWeight(true);
    }

    return total;
}
```

**Key behaviors:**

1. **CONT_WEIGHTLESS flag**: Container contents weigh nothing (magic bags of holding)
2. **TComponent reduction**: Spell components get 10% weight reduction when carried
3. **Recursive traversal**: Follows rider chain + stuff list
4. **pweight parameter**: Controls whether to include container's own weight

**Volume Calculation Chain** (`code/code/obj/obj_base_container.cc`):

```cpp
int TBaseContainer::getCarriedVolume() const {
    TThing* t;
    int total = 0;

    // Add volume of all riders
    for (t = rider; t; t = t->nextRider) {
        total += t->getTotalVolume();
    }

    // Add volume of all contained items
    for (StuffIter it = stuff.begin(); it != stuff.end() && (t = *it); ++it) {
        if (t->getKind() == TThing::TThingKind::TComponent)
            total += (int)(t->getReducedVolume(this) * 0.10);  // 10% reduction!
        else {
            total += t->getReducedVolume(this);
        }
    }

    return total;
}
```

**Material-based volume reduction:**

Each material has a `vol_mult` property that reduces volume when packed into containers:

```cpp
int TThing::getReducedVolume(const TThing* container) const {
    int vol = getVolume();

    // Apply material-based reduction
    if (material_nums[getMaterial()].vol_mult > 0)
        vol /= material_nums[getMaterial()].vol_mult;

    // Additional 5% container packing bonus
    vol = (int)(vol * 0.95);

    return vol;
}
```

**Example:**
- Cloth items: `vol_mult = 2` → 50% volume reduction
- Metal items: `vol_mult = 1` → No reduction
- Leather items: `vol_mult = 3` → 33% volume reduction

### Container Flags

**Source:** `code/code/misc/obj.h`

Container flags use a bitvector stored in `container_flags`:

| Flag | Bit | Meaning |
|------|-----|---------|
| `CONT_CLOSEABLE` | 0 | Container can be opened/closed |
| `CONT_CLOSED` | 1 | Container is currently closed |
| `CONT_LOCKED` | 2 | Container is locked (requires key) |
| `CONT_PICKPROOF` | 3 | Lock cannot be picked |
| `CONT_JAMMED` | 4 | Lock is jammed (from failed pick attempt) |
| `CONT_TRAPPED` | 5 | Container has a trap set |
| `CONT_GHOSTTRAP` | 6 | False trap from failed detect (0 damage) |
| `CONT_EMPTYTRAP` | 7 | Empty trap slot (trap was disarmed) |
| `CONT_WEIGHTLESS` | Custom | Contents weigh nothing |

### Ghost Trap System

The ghost trap system prevents metagaming by creating false traps when players fail to detect traps.

**Source:** `code/code/obj/obj_open_container.cc`

When opening a container:

1. If container is not trapped (`!CONT_TRAPPED`)
2. And player fails `SKILL_DETECT_TRAP` check
3. Create a **ghost trap** with 0 damage:
   - Random trap type (`DOOR_TRAP_FIRE` through `DOOR_TRAP_ENERGY`)
   - Set `trap_dam = 0`
   - Add `CONT_GHOSTTRAP` flag
   - Show trap detection message

**Purpose:**

Without ghost traps, players can metagame: "If I don't see a trap message, there's no trap"

With ghost traps, failed detect creates a ghost trap (0 damage), forcing players to disarm even non-existent traps.

### Key and Lock System

The key and lock system provides security for both containers and doors. Keys are matched by object vnum, and the pick lock skill allows thieves to bypass locks without the proper key.

**TKey Class** (`code/code/obj/obj_key.h`, `obj_key.cc`):

```cpp
class TKey : public TObj {
  public:
    virtual itemTypeT itemType() const { return ITEM_KEY; }
    virtual int putMeInto(TBeing*, TOpenContainer*);
    virtual int stealModifier();
    virtual sstring statObjInfo() const;
};
```

**Key characteristics:**

1. **Simple data model** - Keys have no special data fields; their identity comes from their object vnum
2. **Steal protection** - Keys have a +77 steal modifier making them difficult to pickpocket
3. **No repair** - Locksmiths refuse to repair keys
4. **Rentable check** - Rentable keys are validated against the property database

**TKeyring Class** (`code/code/obj/obj_keyring.h`, `obj_keyring.cc`):

Keyrings are specialized expandable containers that hold keys and provide automatic key lookup.

```cpp
class TKeyring : public TExpandableContainer {
  public:
    virtual itemTypeT itemType() const { return ITEM_KEYRING; }
    virtual void putMoneyInto(TBeing*, int);  // Prevents money storage
    virtual bool objectRepair(TBeing*, TMonster*, silentTypeT);
};
```

**Keyring behaviors:**

1. **Duplicate prevention** - Cannot put the same key vnum twice on a keyring
2. **No repair** - Keyrings cannot be repaired
3. **Automatic search** - The `has_key()` function searches keyrings automatically

**Key Matching Algorithm** (`code/code/misc/movement.cc`):

The `has_key()` function determines if a character possesses a specific key:

```cpp
static bool keyCheck(const TObj* obj, int key) {
    return (obj_index[obj->getItemIndex()].virt == key ||
            (obj->objVnum() == -1 && obj->getSnum() == key));
}

bool has_key(TBeing* ch, int key) {
    // 1. Check inventory
    // 2. Check inside keyrings
    // 3. Check held items
    // 4. Check worn equipment
    // 5. Check potential mob loads (for NPCs)
}
```

**Lock Mechanics - Containers:**

```cpp
void TOpenContainer::lockMe(TBeing* ch) {
    if (!isClosed())
        ch->sendTo("Maybe you should close it first...\n\r");
    else if (getKeyNum() < 1)
        ch->sendTo("That thing can't be locked.\n\r");
    else if (!has_key(ch, getKeyNum()))
        ch->sendTo("You don't seem to have the proper key.\n\r");
    else if (isContainerFlag(CONT_LOCKED))
        ch->sendTo("It is locked already.\n\r");
    else {
        addContainerFlag(CONT_LOCKED);
        ch->sendTo("*Click*\n\r");
        act("$n locks $p with a *click*.", TRUE, ch, this, 0, TO_ROOM);
    }
}
```

**Pick Lock Skill:**

**Container picking (instant):**

```cpp
void TOpenContainer::pickMe(TBeing* thief) {
    // Pre-checks: closed, has keyhole, locked, not pickproof/jammed

    int bKnown = thief->getSkillValue(SKILL_PICK_LOCK);

    if (thief->bSuccess(bKnown, SKILL_PICK_LOCK)) {
        remContainerFlag(CONT_LOCKED);
        thief->sendTo("*Click*\n\r");
    } else {
        if (critFail(thief, SKILL_PICK_LOCK)) {
            addContainerFlag(CONT_JAMMED);  // Permanent jam
        } else {
            thief->sendTo("You fail to pick the lock.\n\r");
        }
    }
}
```

**Door picking (task-based):**

Door picking uses the task system (`TASK_PICKLOCKS`) for extended attempts with lockpick tools.

### Trap System

**Trap Types (12 total)** (`code/code/misc/enum.h`):

| Trap Type | Effect | Damage Type |
|-----------|--------|-------------|
| `DOOR_TRAP_NONE` | No trap | - |
| `DOOR_TRAP_TNT` | Explosive damage | DAMAGE_TRAP_TNT |
| `DOOR_TRAP_POISON` | Poison damage | DAMAGE_TRAP_POISON |
| `DOOR_TRAP_SLEEP` | Sleep effect | - |
| `DOOR_TRAP_FIRE` | Fire damage | DAMAGE_TRAP_FIRE |
| `DOOR_TRAP_ACID` | Acid damage | DAMAGE_TRAP_ACID |
| `DOOR_TRAP_DISEASE` | Disease damage | DAMAGE_TRAP_DISEASE |
| `DOOR_TRAP_SPIKE` | Piercing damage | DAMAGE_TRAP_PIERCE |
| `DOOR_TRAP_BLADE` | Slashing damage | DAMAGE_TRAP_SLASH |
| `DOOR_TRAP_PEBBLE` | Bludgeon damage | DAMAGE_TRAP_BLUNT |
| `DOOR_TRAP_FROST` | Cold damage | DAMAGE_TRAP_FROST |
| `DOOR_TRAP_TELEPORT` | Teleport to random room | - |
| `DOOR_TRAP_ENERGY` | Energy damage | DAMAGE_TRAP_ENERGY |

**Setting Traps** (`code/code/obj/obj_open_container.cc`):

```cpp
int TOpenContainer::trapMe(TBeing* ch, const char* trap_type_arg) {
    // Validate container can be trapped
    if (!isContainerFlag(CONT_CLOSEABLE)) return FALSE;
    if (!isContainerFlag(CONT_CLOSED)) return FALSE;
    if (isContainerFlag(CONT_TRAPPED)) return FALSE;

    // Start TASK_TRAP_CONT (3 ticks delay)
    start_task(ch, this, roomp, TASK_TRAP_CONT, "", 0, in_room, 1, 0, 40);
}
```

**Disarming Traps:**

```cpp
int TOpenContainer::disarmMe(TBeing* ch) {
    // Ghost traps are easy to disarm (always succeed)
    if (isContainerFlag(CONT_GHOSTTRAP)) {
        remContainerFlag(CONT_GHOSTTRAP);
        setContainerTrapType(DOOR_TRAP_NONE);
        setContainerTrapDam(0);
        return TRUE;
    }

    // Real trap disarm attempt
    if (ch->bSuccess(bKnown, SKILL_DISARM_TRAP)) {
        remContainerFlag(CONT_TRAPPED);
        addContainerFlag(CONT_EMPTYTRAP);
    } else {
        // Failed disarm triggers the trap!
        return triggerContTrap(ch);  // May return DELETE flags
    }
}
```

**Trap Triggering (DELETE Flag Propagation Pattern):**

```cpp
// returns DELETE_THIS, DELETE_ITEM
int TBeing::triggerContTrap(TOpenContainer* container) {
    int rc;

    switch (container->getContainerTrapType()) {
        case DOOR_TRAP_FIRE:
            // Fire trap does BOTH direct damage AND engulf check
            rc = objDamage(DAMAGE_TRAP_FIRE, container->getContainerTrapDam(), container);
            if (IS_SET_DELETE(rc, DELETE_THIS))
                return DELETE_THIS;

            rc = flameEngulfed();
            if (IS_SET_DELETE(rc, DELETE_THIS))
                return DELETE_THIS;
            return TRUE;

        case DOOR_TRAP_SPIKE:
            rc = objDamage(DAMAGE_TRAP_PIERCE, container->getContainerTrapDam(), container);
            if (IS_SET_DELETE(rc, DELETE_THIS))
                return DELETE_THIS;
            return TRUE;

        // ... other trap types
    }
}
```

**CRITICAL:** Check DELETE_THIS after EACH dangerous call.

### Container Nesting Restrictions

**Source:** `code/code/obj/obj_base_container.cc`

```cpp
int TBaseContainer::putSomethingIntoContainer(TBeing* ch,
    TOpenContainer* cont) {
    // Critical restriction: non-empty containers cannot nest
    if (!stuff.empty()) {
        act("Containers can't hold other containers unless they're empty.",
            FALSE, ch, cont, this, TO_CHAR);
        return FALSE;
    }

    // Empty container - allow nesting
    return TThing::putSomethingIntoContainer(ch, cont);
}
```

**Why This Restriction?**

Without restriction: Circular references, infinite loops, O(n²) weight calculations

With restriction: Maximum nesting depth = 2, O(n) calculations, no infinite loops

**Exception:** Empty containers CAN be nested.

### Container Cleanup

**Standard Container Deletion (TObj):**

When a container is deleted, its contents are recursively deleted:

```cpp
// From TObj::~TObj()
for (StuffIter it = stuff.begin(); it != stuff.end();) {
    t = *(it++);
    --(*t);        // Remove from container
    if (t) {
        delete t;  // Recursively delete
        t = NULL;
    }
}
```

**Corpse Cleanup (TBaseCorpse):**

Corpses override this behavior to **relocate** contents rather than destroy them:

```cpp
// From TBaseCorpse::~TBaseCorpse()
for (StuffIter it = stuff.begin(); it != stuff.end();) {
    t = *(it++);
    --(*t);  // Remove from corpse

    // Junk newbie items
    TObj* o = dynamic_cast<TObj*>(t);
    if (o && o->isObjStat(ITEM_NEWBIE) && o->stuff.empty()) {
        delete o;
        continue;
    }

    // Relocate based on corpse's location
    if (parent || equippedBy)
        *parent += *t;           // Into holder's inventory
    else if (riding && riding->roomp)
        *riding->roomp += *t;   // Onto ground near furniture
    else if (roomp)
        *roomp += *t;           // Onto ground
    else
        delete t;               // Nowhere valid - delete
}
```

### Serialization (assignFourValues/getFourValues)

Containers pack their properties into 4 integers for database storage:

```cpp
void TOpenContainer::assignFourValues(int x1, int x2, int x3, int x4) {
    max_weight = x1;
    container_flags = x2 & 0xFF;          // Lower byte: flags
    trap_type = (doorTrapT)((x2 >> 8) & 0xFF);  // Next byte: trap type
    trap_dam = (char)((x2 >> 16) & 0xFF);       // Next byte: trap damage
    key_num = x3;
    max_volume = x4;
}

void TOpenContainer::getFourValues(int* x1, int* x2, int* x3, int* x4) const {
    *x1 = (int)max_weight;
    *x2 = container_flags |
          (trap_type << 8) |
          (trap_dam << 16);
    *x3 = key_num;
    *x4 = max_volume;
}
```

**Bit packing in x2:**

| Bits | Field | Range |
|------|-------|-------|
| 0-7 | `container_flags` | 0-255 (8 flag bits) |
| 8-15 | `trap_type` | 0-12 (trap type enum) |
| 16-23 | `trap_dam` | -128 to 127 (signed damage) |
| 24-31 | Unused | 0 |

## III. Consumable Objects

This section describes consumable item systems: light sources, food, and drink containers. These items share temporal depletion mechanics and affect character conditions.

### Class Hierarchy

```
TObj (base)
├── TBaseLight (abstract)
│   ├── TLight (standard light sources)
│   └── TFFlame (perpetual flames/fires)
├── TFood (edible items)
├── TFuel (lamp fuel containers)
└── TBaseCup (abstract drink container)
    ├── TDrinkCon (portable drink containers)
    └── TPool (ground-based liquid pools)
```

### Light Sources

**TBaseLight** (`code/code/obj/obj_base_light.h`):

Abstract base class for all light-producing items.

```cpp
int amtLight;   // Amount of light emitted
int maxBurn;    // Maximum fuel capacity
int curBurn;    // Current fuel remaining
```

**TLight** (`code/code/obj/obj_light.h`, `obj_light.cc`):

Standard light sources like torches, lanterns, and candles.

```cpp
bool lit;  // Whether the light is currently burning
```

**Burn Duration and Decay:**

Each game tick, `lightDecay()` is called for lit lights:

```cpp
void TLight::lightDecay() {
  if (isLit()) {
    addToCurBurn(-1);          // Decrement fuel
    if (getCurBurn() <= 0) {
      setCurBurn(0);
      putLightOut();           // Extinguish when empty
      // Notify character/room
    } else if (getCurBurn() < 4) {
      // Flicker warning when low
    }
  }
}
```

**Light Intensity Descriptions:**

| Light Amount | Description |
|--------------|-------------|
| < 3 | dim |
| 3-7 | moderately-bright |
| 8-14 | bright |
| 15-24 | very bright |
| 25-34 | extremely intense |
| >= 35 | blinding |

**Refueling System:**

TFuel objects can refuel TLight objects:

```cpp
void TFuel::refuelMeFuel(TBeing* ch, TLight* lamp) {
  if (lamp->getMaxBurn() < 0) return;     // Non-refuelable
  if (lamp->getCurBurn() == lamp->getMaxBurn()) return;  // Already full
  if (lamp->isLit()) return;              // Can't refuel while lit

  int use = lamp->getMaxBurn() - lamp->getCurBurn();
  use = min(use, getCurFuel());

  addToCurFuel(-use);
  lamp->addToCurBurn(use);

  if (getCurFuel() <= 0)
    delete this;                  // Discard empty fuel container
}
```

**TFFlame (Perpetual Flames)** (`code/code/obj/obj_flame.h`, `obj_flame.cc`):

Special flame objects representing fires, campfires, and magical flames.

**Magic flags:**

| Flag | Value | Effect |
|------|-------|--------|
| `TFFLAME_INVHEAT` | 1 | Inverted heat (cold fire) |
| `TFFLAME_INVLIGHT` | 2 | Inverted light (dark fire) |
| `TFFLAME_MAGHEAT` | 4 | Double heat output |
| `TFFLAME_MAGLIGHT` | 8 | Double light output |
| `TFFLAME_IMMORTAL` | 16 | Never decays, cannot be fueled |

### Food System

**TFood** (`code/code/obj/obj_food.h`, `obj_food.cc`):

Edible items that satisfy hunger.

```cpp
unsigned int foodFlags;  // FOOD_POISON, FOOD_SPOILED, etc.
int foodFill;           // Hunger satisfaction value
```

**Food Flags** (`code/code/misc/obj.h`):

| Flag | Value | Effect |
|------|-------|--------|
| `FOOD_POISON` | 1 | Poisoned food - applies poison effect |
| `FOOD_SPOILED` | 2 | Spoiled food - causes food poisoning |
| `FOOD_FISHED` | 4 | Caught via fishing - bonus for fish-eaters |
| `FOOD_BUTCHERED` | 8 | Butchered meat - bonus for meat-eaters |

**Eating Mechanics:**

```cpp
void TFood::eatMe(TBeing* ch) {
  // Check if already full
  if (ch->getCond(FULL) > 20 && !ch->isImmortal())
    return;

  // Perceptive characters notice spoilage
  if (isFoodFlag(FOOD_SPOILED) && ch->isPerceptive()) {
    // Discard spoiled food instead of eating
    delete this;
    return;
  }

  // Race-based food preferences
  float adjust = 1.0;
  if (ch->isVampire())
    adjust = 0;              // No effect on vampires
  else if (ch->hasTalent(TALENT_FISHEATER) && isFoodFlag(FOOD_FISHED))
    adjust = 2;              // Double benefit for fish
  else if (ch->hasTalent(TALENT_MEATEATER) && isFoodFlag(FOOD_BUTCHERED))
    adjust = 2;              // Double benefit for meat

  ch->gainCondition(FULL, (int)(getFoodFill() * adjust));

  // Apply poison/spoilage effects
  Poisoned(ch, getFoodFill());
  Spoiled(ch, getFoodFill());

  delete this;
}
```

**Food Spoilage:**

Food items decay into spoiled state before disappearing:

```cpp
int TFood::objectDecay() {
  if (isFoodFlag(FOOD_SPOILED)) {
    return FALSE;              // Already spoiled, continue decay
  } else {
    addFoodFlags(FOOD_SPOILED);
    obj_flags.decay_time = getVolume() * 10;  // Extended decay when spoiled
  }
  return TRUE;                 // Prevent immediate deletion
}
```

**Spoilage flow:**
1. Food item reaches decay time
2. `FOOD_SPOILED` flag added
3. Extended decay timer set based on volume
4. Eventually deleted when second decay triggers

### Drink System

**TBaseCup** (`code/code/obj/obj_base_cup.h`, `obj_base_cup.cc`):

Abstract base class for all liquid containers.

```cpp
int maxDrinks;           // Maximum liquid capacity
int curDrinks;           // Current liquid amount
liqTypeT liquidType;     // Type of liquid contained
unsigned int drinkFlags; // DRINK_POISON, DRINK_PERM, etc.
```

**Drink Container Flags:**

| Flag | Value | Effect |
|------|-------|--------|
| `DRINK_POISON` | 1 | Liquid is poisoned |
| `DRINK_PERM` | 2 | Never-emptying container (fountains) |
| `DRINK_SPILL` | 4 | Liquid spills during movement |
| `DRINK_FROZEN` | 8 | Liquid is frozen solid |

**Liquid Types:**

The game supports 100+ liquid types with varying properties.

**Common liquids:**

| Type | Drunk | Hunger | Thirst | Notes |
|------|-------|--------|--------|-------|
| `LIQ_WATER` | 0 | 0 | 10 | Best thirst quencher |
| `LIQ_BEER` | 5 | -2 | 7 | Moderate intoxication |
| `LIQ_MILK` | 0 | 2 | 6 | Good nutrition |
| `LIQ_COFFEE` | -2 | -3 | 5 | Reduces drunkenness |
| `LIQ_WHISKY` | 10 | 0 | 1 | High intoxication |
| `LIQ_SALTWATER` | 0 | 1 | -5 | Increases thirst |
| `LIQ_BLOOD` | 0 | 2 | -1 | Disease risk |

**Liquid Weight:**

```cpp
const float SIP_WEIGHT = 0.065;
```

Each unit of liquid weighs 0.065 pounds. Container weight is:
```
totalWeight = baseWeight + (drinkUnits * SIP_WEIGHT)
```

**Drinking Mechanics:**

```cpp
int TBaseCup::drinkMe(TBeing* ch) {
  // Pre-checks
  if (ch->hasDisease(DISEASE_FOODPOISON)) return FALSE;
  if (isDrinkConFlag(DRINK_FROZEN)) return FALSE;
  if (getDrinkUnits() <= 0) return FALSE;
  if (ch->getCond(DRUNK) > 15 && ch->getCond(THIRST) > 0) return FALSE;
  if (ch->getCond(THIRST) > 20) return FALSE;

  // Calculate amount to drink
  int amount = 10 * (25 - ch->getCond(THIRST)) / getLiqDrunk();
  amount = min(15, amount);
  amount = max(1, min(amount, getDrinkUnits()));

  // Update container (unless permanent)
  if (!isDrinkConFlag(DRINK_PERM))
    weightChangeObject(-(amount * SIP_WEIGHT));

  // Apply condition changes
  ch->gainCondition(DRUNK, (getLiqDrunk() * amount) / 10);
  ch->gainCondition(FULL, (getLiqHunger() * amount) / 10);
  ch->gainCondition(THIRST, (getLiqThirst() * amount) / 10);

  // Check for poison
  if (isDrinkConFlag(DRINK_POISON))
    // Apply poison effect

  // Check for disease (pools)
  if (isPool && !ch->isImmune(IMMUNE_DISEASE))
    // Chance for dysentery
}
```

**TPool** (`code/code/obj/obj_pool.h`):

Ground-based liquid pools (puddles, blood pools).

**Special characteristics:**
- Created dynamically when liquid is poured out
- Can merge with other pools of same liquid
- Disease risk when drinking from standing water
- Deleted when empty (after drinking)

**Dysentery risk:**
```cpp
if (tPool && getDrinkType() == LIQ_WATER &&
    !ch->isImmune(IMMUNE_DISEASE, WEAR_BODY)) {
  if (!::number(0, 100 / max(1, amount))) {
    // Apply DISEASE_DYSENTERY
  }
}
```

### Condition System

**Condition Types** (`code/code/misc/enum.h`):

```cpp
enum condTypeT {
  DRUNK,   // Intoxication level
  FULL,    // Hunger satisfaction
  THIRST,  // Thirst satisfaction
  PEE,     // Need to urinate
  POOP,    // Need to defecate
  MAX_COND_TYPE
};
```

**Condition Values:**

| Value | Meaning |
|-------|---------|
| -1 | Immortal (never changes) |
| 0 | Critical need (starving/dehydrated) |
| 1-2 | Warning range (rumbling stomach) |
| 3-20 | Normal range |
| 21-24 | Satiated/quenched |
| > 24 | Over-full |

**Effects on Regeneration** (`code/code/misc/limits.cc`):

Low hunger or thirst reduces HP/mana regeneration by 75%:

```cpp
if (!getCond(FULL) || !getCond(THIRST))
  gain >>= 2;  // Divide by 4
```

**Condition Gain Formula:**

```cpp
void TBeing::gainCondition(condTypeT condition, int value) {
  if (getCond(condition) == -1)  // Immortals unaffected
    return;

  if (value > 0) {
    // Adjust for race
    switch (condition) {
      case FULL:
        value = (int)(value * getMyRace()->getFoodMod());
        value = (int)(value * 180.0 / getWeight());  // Body mass
        break;
      case THIRST:
        value = (int)(value * 180.0 / getWeight());
        break;
      case DRUNK:
        value = (int)(value * getMyRace()->getDrinkMod());
        value = (int)(value * 180.0 / getWeight());
        // Modify for SKILL_ALCOHOLISM
        break;
    }
  }

  // Apply change
  specials.conditions[condition] += value;

  // Clamp to valid range
  specials.conditions[condition] =
    max(0, min(24, specials.conditions[condition]));
}
```

## IV. Utility Objects

This section covers utility object types: vehicles for transportation, audio objects for ambient sound, books for readable content, and statue/furniture objects for decorative and rest functionality.

### TVehicle

The `TVehicle` class implements drivable objects like boats, trolleys, and elevators. Vehicles extend `TPortal`, inheriting portal mechanics for entry/exit while adding movement and direction controls.

**Source:** `code/code/obj/obj_vehicle.h`

```cpp
class TVehicle : public TPortal {
  private:
    dirTypeT dir;       // Current travel direction
    int speed;          // Current speed (0 = stopped)
    int type;           // Vehicle type (VEHICLE_BOAT, etc.)

  public:
    bool whole_zone;    // If true, updates all zone exits on move

    void driveSpeed(TBeing*, int);
    void driveDir(TBeing*, dirTypeT);
    void vehiclePulse(int);
    void driveStatus(TBeing*);
    void driveExit(TBeing*);
    void driveLook(TBeing* ch, bool silent = false);
};
```

**Vehicle Types:**

| Constant | Value | Description |
|----------|-------|-------------|
| `VEHICLE_NONE` | 0 | Default/undefined |
| `VEHICLE_BOAT` | 1 | Water vessel (uses nautical messages) |
| `VEHICLE_TROLLEY` | 2 | Rail vehicle (uses rumbling messages) |

**Speed Constants:**

| Constant | Value | Behavior |
|----------|-------|----------|
| `FAST_SPEED` | 100 | Maximum speed, rapid movement messages |
| `MED_SPEED` | 50 | Standard speed |
| `SLOW_SPEED` | 25 | Minimum active speed, gentle movement messages |
| 0 | 0 | Stopped |

**Vehicle Movement:**

Vehicles move via the `vehiclePulse()` method, called by the scheduler proc `procObjVehicle`.

Movement flow:
1. `update_exits()` synchronizes interior room exits to vehicle's current location
2. Speed check: if speed is 0, return immediately
3. Pulse timing: movement occurs at intervals based on speed
4. Direction validation: check for valid exit and allowed path
5. Auto-pathing: if only one valid exit (besides reverse), automatically turn
6. Water check (boats): can move into non-water once but must return
7. Move vehicle to new room
8. Send movement messages to old room, new room, and interior occupants

### TAudio

The `TAudio` class creates ambient sound objects that periodically broadcast messages to rooms. These work independently of the MSP (MUD Sound Protocol) client sound system.

**Source:** `code/code/obj/obj_audio.h`

```cpp
class TAudio : public TObj {
  private:
    int freq;           // Broadcast frequency (pulse divisor)

  public:
    virtual void audioCheck(int) const;
    int getFreq() const;
    void setFreq(int n);
};
```

**Value Fields:**

| Field | Meaning |
|-------|---------|
| val0 | Frequency of noise (pulse divisor) |
| val1-val3 | Unused (0) |

**Sound Broadcasting:**

The `audioCheck()` method broadcasts sound based on two triggers:

1. **Frequency-based:** When `pulse % frequency == 0`
2. **Random:** 1-in-6 chance each pulse regardless of frequency

The sound is determined by the object's `action_description` field, which is broadcast to the object's room and adjacent rooms via `MakeNoise()`.

### TBook

The `TBook` class implements readable books with multi-section support and ANSI color variants.

**Source:** `code/code/obj/obj_book.h`

```cpp
class TBook : public TObj {
  public:
    virtual void lookAtObj(TBeing*, const char*, showModeT) const;
};
```

**Book Content Storage:**

Book content is stored in external files rather than database fields:

**File location:** `objdata/books/`

**File naming:**
- `{vnum}` - Main book content
- `{vnum}.ansi` - ANSI-colored version (for VT100 terminals)
- `{vnum}.{section}` - Section N of the book
- `{vnum}.{section}.ansi` - ANSI-colored section

**Reading Books:**

When a player looks at a book with arguments, the system:

1. Parses the argument for section number (`look book sect 2`)
2. Attempts to load ANSI version if player has color (`hasColorVt()`)
3. Falls back to plain text version
4. Uses `page_string()` for paged output or `CLIENT_NOTE` for custom client
5. Displays "End of section N" marker

### TBed

**Source:** `code/code/obj/obj_bed.h`

Beds are functional furniture that players can sit, rest, or sleep on with regeneration bonuses.

```cpp
class TBed : public TObj {
  private:
    int min_pos_use;    // Minimum position (0=sleep, 1=rest, 2=sit, 3=unusable)
    int max_users;      // Maximum simultaneous users
    int max_size;       // Maximum comfortable height (inches)
    int seat_height;    // Height above ground
    int regen;          // Regeneration bonus percentage
};
```

**Value Fields:**

| Field | Meaning |
|-------|----------|
| val0 | Packed: min_pos_use (bits 7-4), max_users (bits 3-0) |
| val1 | Max designed size (inches) |
| val2 | Seat height (inches) |
| val3 | Regen bonus percentage |

**Bed Position System:**

| Value | Allowed Positions |
|-------|-------------------|
| 0 | Sleep, rest, sit |
| 1 | Rest, sit |
| 2 | Sit only |
| 3 | Unusable |

Space requirements vary by position:
- Sitting: 1 slot
- Resting: 2 slots
- Sleeping: 3 slots

**Bed Regeneration:**

The `bedRegen()` method modifies regeneration rates:

1. Base bonus: `regen * gain / 100` (percentage increase)
2. Size penalty: If player is too tall for bed, reduce gain and display discomfort message
3. Minimum gain: Always at least 0 (won't kill you)

### TTable

Tables are surfaces for placing objects, using the `rider` pointer (normally used for mounts) to track what's on top.

**Source:** `code/code/obj/obj_table.h`

```cpp
class TTable : public TObj {
  public:
    virtual void lookObj(TBeing*, int) const;   // Lists items on table
    virtual void examineObj(TBeing*) const;     // "On top of it, you see:"
    virtual int putSomethingInto(TBeing*, TThing*);
};
```

**Table Destruction:**

When a table is destroyed:
1. All items on it are dismounted
2. Items are moved to table's parent or room
3. Items take 50% structure damage
4. Items at 0 structure are destroyed

## V. Cross-Cutting Concerns

### Common Lifecycle Bugs

**Bug: Use-After-Free from Ignored Return Values**

```cpp
// CRASH: Ignoring DELETE_THIS from spec proc
int rc = obj->checkSpec(NULL, CMD_GENERIC_PULSE, "", NULL);
obj->doSomethingElse();  // obj may be invalid!

// CORRECT: Check and propagate
int rc = obj->checkSpec(NULL, CMD_GENERIC_PULSE, "", NULL);
if (IS_SET_DELETE(rc, DELETE_ITEM))
    return true;  // Signal scheduler to delete
obj->doSomethingElse();  // Safe
```

**Bug: Deleting Objects Still in Containers**

```cpp
// CRASH: Object still has parent
TObj* item = container->stuff.front();
delete item;  // Corrupts container->stuff!

// CORRECT: Remove first
TObj* item = container->stuff.front();
--(*item);    // Remove from container
delete item;  // Safe
```

**Bug: Iterator Invalidation During Container Modification**

```cpp
// CRASH: Iterator invalidated
for (auto it = stuff.begin(); it != stuff.end(); ++it) {
    TThing* t = *it;
    --(*t);    // Removes t from stuff, invalidating it
    // ++it now crashes or skips items
}

// CORRECT: Post-increment before removal
for (auto it = stuff.begin(); it != stuff.end();) {
    TThing* t = *(it++);  // Advance FIRST
    --(*t);               // Now safe to remove
}
```

### Safe Extraction Pattern

To safely remove items from a container before deletion:

```cpp
// Safe: iterate with post-increment, extract before accessing next
for (StuffIter it = container->stuff.begin(); it != container->stuff.end();) {
    TThing* t = *(it++);  // Get item AND advance iterator
    --(*t);               // Remove from container
    *destination += *t;   // Add to new location
}
// Now safe to delete empty container
delete container;
```

**Critical:** The `StuffIter it = *(it++)` pattern is essential. The iterator must be advanced BEFORE the item is removed, because `--(*t)` invalidates iterators pointing to that item.

### Integration with Other Systems

**Spatial Relationship System**

Containers participate in the TThing spatial relationship model:

- `parent` pointer tracks immediate container/room/being
- `roomp` shortcut points to room (NULL when inside container)
- `stuff` list contains all directly held items
- `rider` chain tracks things sitting on top of container

**See also:** [Spatial Relationships](spatial-relationships.md)

**Combat System**

Trap damage uses the standard combat damage pipeline:

```cpp
rc = objDamage(DAMAGE_TRAP_FIRE, damage, container);
```

This flows through `reconcileDamage()` → `applyDamage()` → `damageEpilog()`, allowing armor to reduce trap damage and potentially killing the character.

**See also:** [Damage Pipeline](damage-pipeline.md)

**Task System**

Setting traps uses the task system with `TASK_TRAP_CONT`:

- Duration: 3 ticks (0.9 seconds)
- Interruptible by combat or movement
- On completion: trap is set with calculated damage

**See also:** [Task System](task-system.md)

**DELETE Flag System**

Trap triggering returns DELETE flags that must be propagated:

```cpp
// Trap function returns DELETE_THIS
int rc = ch->triggerContTrap(container);

// Caller must check and propagate
if (IS_SET_DELETE(rc, DELETE_THIS))
    return DELETE_THIS;
```

**See also:** [DELETE Flags](delete-flags.md), [Trap Mechanics](trap-mechanics.md)

## Related Documentation

- [DELETE Flag System](delete-flags.md) - How deletion flags propagate
- [Proc Adapter](proc-adapter.md) - Scheduler DELETE-to-bool conversion
- [Spatial Relationships](spatial-relationships.md) - Container pointer relationships
- [Task System](task-system.md) - Task object lifecycle hazards
- [Trap Mechanics](trap-mechanics.md) - Complete trap system documentation
- [Damage Pipeline](damage-pipeline.md) - How trap damage flows
- [Affects System](affects-system.md) - Poison and disease effects
- [Material System](material-system.md) - Food material pricing
- [Scheduler and Pulses](scheduler-pulses.md) - Light decay timing
