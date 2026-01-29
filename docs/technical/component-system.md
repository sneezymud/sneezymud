---
title: Spell Component System
description: Material, gestural, and verbal requirements for spellcasting, with discipline-based storage access expansion (Wizardry/Ritualism), timed consumption patterns, and component charge management.
keywords:
  - TComponent
  - COMP_MATERIAL
  - COMP_GESTURAL
  - COMP_VERBAL
  - spellInfo
  - comp_types
  - findComponent
  - useComponent
  - TSpellBag
  - enforceGestural
  - enforceVerbal
  - applyCompCheck
  - CompInfo
  - component_placement
category: Important Systems

  - spell-combat.md
  - affects-system.md
  - discipline-system.md
  - object-types.md
last_updated: 2026-01-29
source_files:
  - code/code/obj/obj_component.h
  - code/code/obj/obj_component.cc
  - code/code/obj/obj_spellbag.h
  - code/code/misc/spell2.h
  - code/code/misc/magicutils.cc
  - code/code/misc/discipline.cc
  - code/code/misc/spelltask.cc
  - code/code/misc/gaining.cc
related: [spell-skill-framework.md]
---

# Spell Component System

The spell component system manages material, gestural, and verbal requirements for spellcasting. Understanding this system is essential for implementing spells correctly and preventing component-related bugs.

## Overview

Spells can require three types of components:
- **Material components**: Physical items consumed during casting
- **Gestural components**: Hand movements requiring free hands
- **Verbal components**: Speech requiring functional mouth

Component requirements are defined via `COMP_*` bitflags in `spellInfo.comp_types`. The system integrates with the discipline progression (Wizardry/Ritualism) to determine which storage locations players can access.

**Key characteristics:**
- Components are individual objects (`TComponent`) with vnums, charges, and spell associations
- Storage location access expands with Wizardry/Ritualism skill
- Consumption timing varies: spell start, end, every round, random, or next-to-last
- Immortals and NPCs bypass component requirements

## Component Types and VNums

Components are identified by virtual number (vnum) rather than a type enum. Each spell has an associated component object defined in the `CompInfo` vector.

### Component VNum Constants

**Source:** `code/code/obj/obj_component.h`

**Selected examples:**

| Constant | VNum | Spell | Constant | VNum | Spell |
|----------|------|-------|----------|------|-------|
| `COMP_SUFFOCATE` | 201 | Suffocate | `COMP_STONE_SKIN` | 223 | Stone Skin |
| `COMP_DUST_STORM` | 202 | Dust Storm | `COMP_POLYMORPH` | 259 | Polymorph |
| `COMP_CONJURE_AIR` | 204 | Conjure Air | `COMP_POWERSTONE` | 211 | Mana storage |
| `COMP_DRAGON_BONE` | 238 | Dragon Bone | `COMP_RESURRECTION` | 31322 | Resurrection |
| `COMP_ENTHRALL_DEMON` | 31304 | Demon Enthrall | `COMP_VAMPIRIC_TOUCH` | 31313 | Vampiric Touch |

Over 80 component vnums are defined for various spells across all casting classes.

### Component Action Flags

**Source:** `code/code/obj/obj_component.h`

```cpp
const unsigned int COMP_DECAY = (1 << 0);      // Component decays over time
const unsigned int COMP_SPELL = (1 << 1);      // Used for spell casting
const unsigned int COMP_POTION = (1 << 2);     // Can be used in potions
const unsigned int COMP_SCRIBE = (1 << 3);     // Can be scribed

const unsigned int CACT_PLACE = (1 << 0);      // Action: place/create component
const unsigned int CACT_REMOVE = (1 << 1);     // Action: remove/destroy component
const unsigned int CACT_UNIQUE = (1 << 2);     // Component is unique
```

## Component Requirement Flags (COMP_*)

Spell component requirements are defined via bitflags stored in `spellInfo.comp_types`. Each spell defines which component types are required and when they're consumed.

### COMP_* Flag Definitions

**Source:** `code/code/misc/spell2.h`

| Flag | Bit | Meaning |
|------|-----|---------|
| **Gestural Flags** | | |
| `COMP_GESTURAL` | 1<<0 | Requires hand gestures to cast |
| `COMP_GESTURAL_INIT` | 1<<1 | Gestures required at spell initialization |
| `COMP_GESTURAL_END` | 1<<2 | Gestures required at spell completion |
| `COMP_GESTURAL_ALWAYS` | 1<<3 | Gestures required every round (multi-round spells) |
| `COMP_GESTURAL_RANDOM` | 1<<4 | Gestures randomly required during casting |
| **Verbal Flags** | | |
| `COMP_VERBAL` | 1<<5 | Requires verbal component (speaking) |
| `COMP_VERBAL_INIT` | 1<<6 | Speech required at spell initialization |
| `COMP_VERBAL_END` | 1<<7 | Speech required at spell completion |
| `COMP_VERBAL_ALWAYS` | 1<<8 | Speech required every round |
| `COMP_VERBAL_RANDOM` | 1<<9 | Speech randomly required |
| **Material Flags** | | |
| `COMP_MATERIAL` | 1<<10 | Requires material component object |
| `COMP_MATERIAL_INIT` | 1<<11 | Material consumed at spell start |
| `COMP_MATERIAL_END` | 1<<12 | Material consumed at spell end |
| `COMP_MATERIAL_ALWAYS` | 1<<13 | Material needed every round |
| `COMP_MATERIAL_RANDOM` | 1<<14 | Material randomly needed |
| `COMP_MATERIAL_ALMOST_END` | 1<<15 | Material consumed next-to-last round |
| **Spell System Flag** | | |
| `SPELL_TASKED` | 1<<16 | Multi-round tasked spell |

### spellInfo Structure

**Source:** `code/code/misc/spell2.h`

```cpp
class spellInfo {
    const char* name;           // Spell name
    skillUseClassT typ;         // Spell type
    discNumT disc;              // Primary discipline
    discNumT assDisc;           // Associated discipline
    statTypeT modifierStat;     // Stat modifier
    taskDiffT task;             // Task difficulty
    lag_t lag;                  // Lag cost
    positionTypeT minPosition;  // Minimum position required
    int minMana;                // Mana cost
    int minLifeforce;           // Lifeforce cost
    float minPiety;             // Piety cost
    uint32_t targets;           // Target flags (TAR_*)
    uint32_t comp_types;        // COMPONENT FLAGS - COMP_* bitflags
    // ... other fields
};
```

**Example spell definition:**
```cpp
discArray[SPELL_FIREBALL] = new spellInfo(
    SPELL_MAGE, DISC_FIRE, DISC_FIRE, STAT_INT,
    "fireball", TASK_NORMAL, LAG_2, POSITION_SITTING,
    MANA_30, LIFEFORCE_0, PRAY_0,
    TAR_CHAR_ROOM | TAR_VIOLENT | TAR_FIGHT_VICT | TAR_SELF_NONO,
    SYMBOL_STRESS_0,
    "", "", "", "",
    START_20, LEARN_10, START_DO_30, LEARN_DO_5,
    START_DO_NO, LEARN_DO_NO, LEARN_DIFF_SPELLS, 0.04,
    COMP_GESTURAL | COMP_VERBAL | COMP_MATERIAL,  // Component requirements
    0
);
```

## TComponent Class

Components are objects that inherit from `TMergeable`, allowing multiple components of the same type to merge into a single stack with combined charges.

**Source:** `code/code/obj/obj_component.h`

### Class Definition

```cpp
class TComponent : public TMergeable {
  private:
    int charges;           // Number of uses remaining
    spellNumT comp_spell;  // Associated spell
    int comp_type;         // Component type flags

  public:
    // Charge management
    virtual int getComponentCharges() const;
    virtual void setComponentCharges(int);

    // Spell association
    virtual spellNumT getComponentSpell() const;
    virtual void setComponentSpell(spellNumT);

    // Type flags
    virtual int getComponentType() const;
    virtual void setComponentType(int);

    // Merging support (inherited from TMergeable)
    virtual int canMerge(TThing*) const;
    virtual void doMerge(TThing*);
};
```

### Key Behaviors

**Merging:** When a player picks up a component of the same type as one they already have, the charges combine:
```cpp
// Original stack: 5 charges
// Pickup: 3 charges
// Result: 8 charges in single stack
```

**Charge depletion:** When charges reach 0, the component is deleted automatically.

**Personalization:** Components can be personalized to specific players, preventing use by others.

## Component Storage System

Storage location access is determined by the caster's Wizardry (mages) or Ritualism (clerics/shamans) discipline progression. Higher skill levels unlock access to additional equipment slots.

### Wizardry Access Levels

**Source:** `code/code/misc/gaining.cc`

| Level Constant | Access |
|----------------|--------|
| `WIZ_LEV_COMP_PRIM_OTHER_FREE` | Primary hand only (enemies' items free) |
| `WIZ_LEV_COMP_EITHER_OTHER_FREE` | Either hand (enemies' items free) |
| `WIZ_LEV_COMP_EITHER` | Either hand |
| `WIZ_LEV_COMP_INV` | Inventory |
| `WIZ_LEV_COMP_NECK` | Neck slot |
| `WIZ_LEV_COMP_WRIST` | Wrist pouches |
| `WIZ_LEV_COMP_BELT` | Belt (full access) |

### Ritualism Access Levels

**Source:** `code/code/misc/gaining.cc`

Mirror the Wizardry progression with identical slot access.

### findComponent() Function

**Source:** `code/code/misc/magicutils.cc`

Searches for components based on access level:

```cpp
TComponent* findComponent(TBeing* ch, spellNumT which_spell) {
    // Determine access level from Wizardry/Ritualism skill
    int access_level = getAccessLevel(ch);

    // Search locations in order (based on access level):
    // 1. Primary hand
    // 2. Secondary hand
    // 3. Belt slot (spellbag)
    // 4. Neck slot (component pouch)
    // 5. Wrist pouches
    // 6. Inventory
    // 7. Inside spellbag containers

    // Return TComponent* if found, NULL if not found
}
```

### TSpellBag Container

**Source:** `code/code/obj/obj_spellbag.h`

Specialized expandable container for storing components:

```cpp
class TSpellBag : public TExpandableContainer {
  public:
    virtual itemTypeT itemType() const { return ITEM_SPELLBAG; }

    // Locate components within the bag
    virtual TComponent* findSomeComponent(spellNumT spell);
};
```

## Component Consumption

Components are consumed at different times based on the `COMP_MATERIAL_*` flags in the spell definition.

### Consumption Timing

**Source:** `code/code/misc/spelltask.cc`

| Flag | Pattern | When Consumed |
|------|---------|---------------|
| `COMP_MATERIAL_INIT` | 1 | At spell initialization |
| `COMP_MATERIAL_END` | 2 | At spell completion |
| `COMP_MATERIAL_ALWAYS` | 3 | Every round (multi-round spells) |
| `COMP_MATERIAL_RANDOM` | 4 | Random round during casting |
| `COMP_MATERIAL_ALMOST_END` | 5 | Next-to-last round |

### applyCompCheck() Function

**Source:** `code/code/misc/spelltask.cc`

Determines when to consume components for multi-round spells:

```cpp
int applyCompCheck(TBeing* ch, TSpellTask* task, spellNumT which_spell) {
    uint32_t comp_types = discArray[which_spell]->comp_types;
    int comp_pattern = 0;

    // Determine consumption pattern
    if (comp_types & COMP_MATERIAL_INIT)
        comp_pattern = 1;      // Consume at initialization
    else if (comp_types & COMP_MATERIAL_END)
        comp_pattern = 2;      // Consume at spell end
    else if (comp_types & COMP_MATERIAL_ALWAYS)
        comp_pattern = 3;      // Consume every round
    else if (comp_types & COMP_MATERIAL_RANDOM)
        comp_pattern = 4;      // Consume random round
    else if (comp_types & COMP_MATERIAL_ALMOST_END)
        comp_pattern = 5;      // Consume next-to-last round

    // Apply consumption based on pattern and current round
    switch (comp_pattern) {
        case 1: // INIT - already consumed
            break;
        case 2: // END
            if (task->rounds == task->maxRounds - 1)
                useComponent(ch, task->victim, which_spell);
            break;
        case 3: // ALWAYS
            useComponent(ch, task->victim, which_spell);
            break;
        case 4: // RANDOM
            if (::number(0, 1))
                useComponent(ch, task->victim, which_spell);
            break;
        case 5: // ALMOST_END
            if (task->rounds == task->maxRounds - 2)
                useComponent(ch, task->victim, which_spell);
            break;
    }

    return TRUE;
}
```

### useComponent() Function

**Source:** `code/code/misc/magicutils.cc`

Consumes a component and displays messages:

```cpp
TComponent* useComponent(TBeing* ch, TBeing* victim, spellNumT which_spell) {
    // Immortals with NOHASSLE bypass component requirement
    if (ch->isImmunity(IMMU_NOHASSLE))
        return nullptr;

    // Mobs don't require components
    if (ch->isMonster())
        return nullptr;

    // If component already used in this spell task, skip
    if (spelltask && spelltask->component_used)
        return nullptr;

    // Find the component
    TComponent* component = findComponent(ch, which_spell);
    if (!component) {
        missingComponent(ch, which_spell);
        return nullptr;
    }

    // Check personalization
    if (component->isPersonalized() &&
        component->getPersonalizedTo() != ch->getName()) {
        missingComponent(ch, which_spell);
        return nullptr;
    }

    // Display consumption messages
    int comp_index = which_spell;
    if (comp_index < CompInfo.size()) {
        act(CompInfo[comp_index].to_caster, FALSE, ch, component, victim, TO_CHAR);
        act(CompInfo[comp_index].to_other, FALSE, ch, component, victim, TO_ROOM);
        if (victim && victim != ch)
            act(CompInfo[comp_index].to_vict, FALSE, ch, component, victim, TO_VICT);
    }

    // Decrement charges or delete component
    int charges = component->getComponentCharges();
    if (charges > 0) {
        component->setComponentCharges(charges - 1);
        if (charges - 1 <= 0) {
            --(*component);
            delete component;
        }
    }

    // Mark component as used for this spell task
    if (spelltask)
        spelltask->component_used = true;

    return component;
}
```

## Component Checking and Validation

Component requirements are enforced at spell initiation and during multi-round spell continuation.

### Gestural Component Enforcement

**Source:** `code/code/misc/discipline.cc`

```cpp
int TBeing::enforceGestural(spellNumT which_spell) {
    // Check if COMP_GESTURAL flag set
    if (!(discArray[which_spell]->comp_types & COMP_GESTURAL))
        return TRUE;  // No gestural component required

    // Mage gestural checks
    if (isPc() && hasClass(CLASS_MAGE)) {
        // Check hand availability
        TObj* prim = heldInPrimHand();
        TObj* sec = heldInSecHand();

        // Need at least one free hand
        if (prim && sec) {
            sendTo("You need a free hand to cast this spell.\n\r");
            return FALSE;
        }

        // Check arm functionality
        if (!canUseLimb(getPrimaryArm()) && !canUseLimb(getSecondaryArm())) {
            sendTo("You cannot gesture properly.\n\r");
            return FALSE;
        }

        // Apply position penalties
        if (getPosition() < POSITION_STANDING) {
            // Sitting/resting reduces gesture effectiveness
            int penalty = (POSITION_STANDING - getPosition()) * 10;
            if (::number(0, 100) < penalty) {
                sendTo("You struggle to gesture from this position.\n\r");
                return FALSE;
            }
        }
    }

    // Shaman gestural checks (similar restrictions)
    if (isPc() && hasClass(CLASS_SHAMAN)) {
        // Similar checks...
    }

    return TRUE;
}
```

### Verbal Component Enforcement

**Source:** `code/code/misc/discipline.cc`

```cpp
int TBeing::enforceVerbal(spellNumT which_spell) {
    // Check if COMP_VERBAL flag set
    if (!(discArray[which_spell]->comp_types & COMP_VERBAL))
        return TRUE;

    // Check if caster can speak
    if (affectedBySpell(SPELL_SILENCE)) {
        sendTo("You cannot speak!\n\r");
        return FALSE;
    }

    if (isAffected(AFF_PARALYSIS)) {
        sendTo("You are paralyzed!\n\r");
        return FALSE;
    }

    // Check mouth functionality
    if (!canUseLimb(WEAR_HEAD)) {
        sendTo("Your mouth doesn't work properly!\n\r");
        return FALSE;
    }

    // High wizardry/ritualism can bypass verbal requirements
    if (hasClass(CLASS_MAGE)) {
        CDiscipline* cd = getDiscipline(DISC_WIZARDRY);
        if (cd && cd->getLearnedness() >= 75) {
            return TRUE;  // Bypass verbal requirement
        }
    }

    if (hasClass(CLASS_CLERIC) || hasClass(CLASS_SHAMAN)) {
        CDiscipline* cd = getDiscipline(DISC_RITUALISM);
        if (cd && cd->getLearnedness() >= 75) {
            return TRUE;  // Bypass verbal requirement
        }
    }

    // Display verbal component messages
    act("$n utters mystical words.", FALSE, this, NULL, NULL, TO_ROOM);

    return TRUE;
}
```

### Material Component Checking

**Source:** `code/code/misc/discipline.cc`

Material components are checked during `bPassMageChecks()` and similar functions:

```cpp
// In bPassMageChecks():
uint32_t comp_types = discArray[which_spell]->comp_types;

if (comp_types & COMP_MATERIAL) {
    // Check if component needed at INIT
    if (comp_types & COMP_MATERIAL_INIT) {
        if (!useComponent(ch, victim, which_spell)) {
            ch->nothingHappens();
            return FALSE;  // Component not available
        }
    }
}
```

## Component Creation and Acquisition

Components are created through the component placement system and automatic generation during spell practice.

### Component Placement System

**Source:** `code/code/obj/obj_component.h`

```cpp
class compPlace {
  public:
    int room1, room2;           // Range of rooms
    int mob;                    // Mob to load on (MOB_NONE for room)
    int number;                 // VNum of object to load

    // Load/removal actions
    unsigned int place_act;     // CACT_PLACE, CACT_REMOVE, CACT_UNIQUE
    int max_number;             // Max instances in game
    int variance;               // % chance of load occurring

    // Timing conditions
    short hour1, hour2;         // Hour range (0-47 mud hours)
    short day1, day2;           // Day range
    short month1, month2;       // Month range
    int weather;                // WEATHER_xx values

    // Messages and sounds
    const char* message;        // Room message on load/remove
    const char* glo_msg;        // Global message to room range
    soundNumT sound;            // Sound effect on load
    unsigned int sound_loop;    // Loop count
};

extern std::vector<compPlace> component_placement;
```

**Placement triggers:**
- Time-based: specific hours, days, or months
- Weather-based: rain, snow, clear skies, etc.
- Location-based: specific room ranges
- Mob-based: load on specific NPC types
- Variance: percentage chance of load occurring

### Automatic Component Generation

**Source:** `code/code/misc/gaining.cc`

Components can be automatically created when practicing spells:

```cpp
// In learnFromDoing() function:
int comp = which_spell;
if (comp < CompInfo.size() && CompInfo[comp].comp_num >= 0) {
    // Create component object
    TObj* obj = read_object(CompInfo[comp].comp_num, VIRTUAL);
    if (obj) {
        // Give to player or place in world
        *player += *obj;
    }
}
```

### Component Acquisition Methods

1. **Random World Placement**: via `component_placement` vector with time/weather/location conditions
2. **NPC Gifting**: Trainers/mobs can create and give components to players
3. **Learning by Doing**: Components automatically created when practicing spells
4. **Shop Purchase**: Some components can be purchased from shops
5. **Item Merging**: Multiple components of same type merge into single stack with increased charges

## CompInfo Structure

The `CompInfo` vector maps spells to components and defines consumption messages.

**Source:** `code/code/obj/obj_component.h`

```cpp
class compInfo {
  public:
    int comp_num;               // Component object vnum
    spellNumT spell_num;        // Associated spell number
    const char* to_caster;      // Message to spell caster
    const char* to_other;       // Message to others in room
    const char* to_vict;        // Message to target (if alive)
    const char* to_self;        // Message to self-cast target
    const char* to_room;        // Message to room
    const char* to_self_object; // Message for object (self)
    const char* to_room_object; // Message for object (room)
};

extern std::vector<compInfo> CompInfo;   // Global component info
extern std::vector<COMPINDEX> CompIndex; // Component index for lookup
```

**Example CompInfo entry:**
```cpp
CompInfo[SPELL_FIREBALL] = {
    COMP_FIREBALL,              // Component vnum (205)
    SPELL_FIREBALL,             // Spell number
    "You consume a sulfur ball.",                    // to_caster
    "$n consumes a sulfur ball.",                    // to_other
    "$n consumes a sulfur ball to cast at you!",     // to_vict
    "You consume a sulfur ball on yourself.",        // to_self
    "$n consumes a sulfur ball.",                    // to_room
    "You consume a sulfur ball on $p.",              // to_self_object
    "$n consumes a sulfur ball on $p."               // to_room_object
};
```

## Error Messages

When components are missing or unavailable, specific error messages are displayed.

### missingComponent() Function

**Source:** `code/code/misc/magicutils.cc`

```cpp
void missingComponent(TBeing* ch, spellNumT which_spell) {
    // Rangers get special message
    if (ch->hasClass(CLASS_RANGER)) {
        ch->sendTo("You seem to lack the proper natural materials to complete your task.\n\r");
    } else {
        ch->sendTo("You seem to lack the proper materials to complete your task.\n\r");
    }
}
```

### Error Message Summary

| Situation | Message |
|-----------|---------|
| Component not found (non-ranger) | "You seem to lack the proper materials to complete your task." |
| Component not found (ranger) | "You seem to lack the proper natural materials to complete your task." |
| Component personalized to someone else | "You seem to lack the proper materials to complete your task." |
| Cannot gesture | "You cannot gesture properly." / "You need a free hand to cast this spell." |
| Cannot speak | "You cannot speak!" / "Your mouth doesn't work properly!" |
| Silenced | "You cannot speak!" |
| Paralyzed | "You are paralyzed!" |

## Common Patterns

### Adding Material Component to Spell

```cpp
// In buildSpellArray() (spell_info.cc):
discArray[SPELL_NEW_SPELL] = new spellInfo(
    SPELL_MAGE, DISC_FIRE, DISC_FIRE, STAT_INT,
    "new spell", TASK_NORMAL, LAG_2, POSITION_SITTING,
    MANA_30, LIFEFORCE_0, PRAY_0,
    TAR_CHAR_ROOM | TAR_VIOLENT,
    SYMBOL_STRESS_0,
    "", "", "", "",
    START_20, LEARN_10, START_DO_30, LEARN_DO_5,
    START_DO_NO, LEARN_DO_NO, LEARN_DIFF_SPELLS, 0.04,
    COMP_GESTURAL | COMP_VERBAL | COMP_MATERIAL | COMP_MATERIAL_INIT,
    0
);

// In obj_component.h:
const int COMP_NEW_SPELL = 999;  // Define component vnum

// In CompInfo initialization:
CompInfo[SPELL_NEW_SPELL] = {
    COMP_NEW_SPELL,
    SPELL_NEW_SPELL,
    "You consume a mystical component.",
    "$n consumes a mystical component.",
    "$n consumes a mystical component to cast at you!",
    "You consume a mystical component on yourself.",
    "$n consumes a mystical component.",
    "You consume a mystical component on $p.",
    "$n consumes a mystical component on $p."
};
```

### Checking Component Availability Before Casting

```cpp
// Already handled by bPassMageChecks/bPassShamanChecks
// Just define COMP_MATERIAL flag in spell definition
```

### Multi-Round Spell with Component Consumed at End

```cpp
// Use COMP_MATERIAL_END flag:
COMP_GESTURAL | COMP_VERBAL | COMP_MATERIAL | COMP_MATERIAL_END | SPELL_TASKED
```

### Component with Multiple Charges

```cpp
TComponent* comp = dynamic_cast<TComponent*>(read_object(COMP_FIREBALL, VIRTUAL));
if (comp) {
    comp->setComponentCharges(5);  // 5 uses
    *player += *comp;
}
```

## Common Gotchas

### 1. Forgetting COMP_MATERIAL Flag

```cpp
// WRONG: Only COMP_MATERIAL_INIT without COMP_MATERIAL
COMP_GESTURAL | COMP_VERBAL | COMP_MATERIAL_INIT  // Won't check for component!

// CORRECT: Both flags required
COMP_GESTURAL | COMP_VERBAL | COMP_MATERIAL | COMP_MATERIAL_INIT
```

### 2. Not Adding CompInfo Entry

```cpp
// If you define a new component vnum but forget to add CompInfo entry,
// the spell will use components but show no messages
```

### 3. Personalization Check Failure

```cpp
// Component personalized to "Gandalf"
// Player "Bilbo" tries to use it
// Returns missing component error, not "personalized" message
```

### 4. Component Already Used Flag

```cpp
// spelltask->component_used is set when component consumed
// Prevents double-consumption in multi-round spells
// Don't manually reset this flag - it's cleared on task completion
```

### 5. Immortal Bypass

```cpp
// Immortals with IMMU_NOHASSLE bypass ALL component requirements
// Components are never checked or consumed for them
// Useful for testing spells without component gathering
```

### 6. NPC Spell Casting

```cpp
// NPCs NEVER require components
// isMonster() check in useComponent() returns early
// Prevents mobs from needing component inventories
```

### 7. Access Level Progression

```cpp
// Low Wizardry: Can only use components in primary hand
// Mid Wizardry: Can use from either hand or inventory
// High Wizardry: Can use from neck/wrist/belt slots
// Don't assume all players can access spellbag components
```

## Key Source Files

| File | Purpose | Key Functions/Structures |
|------|---------|--------------------------|
| `code/code/obj/obj_component.h` | Component definitions | TComponent class, compInfo, component vnums |
| `code/code/obj/obj_component.cc` | Component implementation | TComponent methods |
| `code/code/obj/obj_spellbag.h` | Component containers | TSpellBag class |
| `code/code/misc/spell2.h` | Component flags | COMP_* flag definitions, spellInfo.comp_types |
| `code/code/misc/magicutils.cc` | Component utilities | findComponent(), useComponent(), missingComponent() |
| `code/code/misc/discipline.cc` | Component enforcement | enforceGestural(), enforceVerbal() |
| `code/code/misc/spelltask.cc` | Multi-round timing | applyCompCheck() |
| `code/code/misc/gaining.cc` | Access levels | Wizardry/Ritualism level definitions |

## Related Documentation

- [Spell Definitions](spell-definitions.md) - How to define spells with component requirements
- [Offensive Spells](spell-combat.md) - Offensive spell implementation patterns
- [Affects System](affects-system.md) - Spell effects and buffs
- [Discipline System](discipline-system.md) - Wizardry and Ritualism skill progression
- [Object Types](object-types.md) - TComponent class in object hierarchy
