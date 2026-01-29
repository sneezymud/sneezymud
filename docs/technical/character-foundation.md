---
title: Character Foundation
description: Core character mechanics including class system, race system, and stats/attributes with multiclass mechanics and stat calculations
keywords: [CLASS_MAGE, CLASS_CLERIC, CLASS_WARRIOR, CLASS_THIEF, CLASS_SHAMAN, CLASS_DEIKHAN, CLASS_MONK, classIndT, race_t, Race, statTypeT, plotStat, getStat, hasClass, getLevel, body_t, baseStats, multiclass, mana, piety, lifeforce, stat layers, power law, racial modifiers]
category: Important Systems

last_updated: 2026-01-29
source_files: [code/code/misc/being.cc, code/code/misc/race.cc, code/code/misc/stats.cc, code/code/misc/multiclass.cc]
related:
  - class-hierarchy.md
  - spell-skill-framework.md
  - experience-leveling.md
  - combat-formulas.md
  - equipment-wear.md
---

# Character Foundation

This document describes the core character mechanics in SneezyMUD, including the class system, race system, and stats/attributes system. These three systems work together to define character capabilities, progression, and gameplay mechanics.

## Overview

The character foundation consists of three interconnected systems:

1. **Class System** - Bitmask-based multiclass with 9 classes, independent level tracking, and class-specific resources
2. **Race System** - 127 races with unique stat distributions, body types, and abilities
3. **Stats & Attributes** - 13 primary stats with non-linear power-law scaling affecting all gameplay

**Design Philosophy:**
- Classes provide abilities and progression (what you can do)
- Races provide baseline stats and physical characteristics (what you are)
- Stats determine effectiveness at everything (how well you do it)

---

# I. Class System

SneezyMUD uses a **bitmask-based multiclass system** where characters can combine multiple classes simultaneously. Each class is represented by a single bit in a 16-bit `unsigned short` value.

## Class Definitions

### The 9 Playable Classes

All classes are defined as bitmask constants in `code/code/misc/defs.h`:

| Class | Constant | Bit | Value | Hex | Enabled | Primary Disc | Secondary Disc |
|-------|----------|-----|-------|-----|---------|--------------|----------------|
| **Mage** | `CLASS_MAGE` | 0 | 1 | 0x0001 | Yes | `DISC_MAGE` | `DISC_LORE` |
| **Cleric** | `CLASS_CLERIC` | 1 | 2 | 0x0002 | Yes | `DISC_CLERIC` | `DISC_THEOLOGY` |
| **Warrior** | `CLASS_WARRIOR` | 2 | 4 | 0x0004 | Yes | `DISC_WARRIOR` | `DISC_NONE` |
| **Thief** | `CLASS_THIEF` | 3 | 8 | 0x0008 | Yes | `DISC_THIEF` | `DISC_NONE` |
| **Shaman** | `CLASS_SHAMAN` | 4 | 16 | 0x0010 | Yes | `DISC_SHAMAN` | `DISC_NONE` |
| **Deikhan** | `CLASS_DEIKHAN` | 5 | 32 | 0x0020 | Yes | `DISC_DEIKHAN` | `DISC_THEOLOGY` |
| **Monk** | `CLASS_MONK` | 6 | 64 | 0x0040 | Yes | `DISC_MONK` | `DISC_NONE` |
| **Ranger** | `CLASS_RANGER` | 7 | 128 | 0x0080 | No | `DISC_RANGER` | `DISC_NONE` |
| **Commoner** | `CLASS_COMMONER` | 8 | 256 | 0x0100 | No | `DISC_ADVENTURING` | `DISC_NONE` |

**Special Constants:**
- `CLASS_ALL = 511 (0x01FF)` - Bitmask with all 9 classes enabled
- `MIN_CLASS_IND = MAGE_LEVEL_IND` (0)
- `MAX_CLASSES = UNUSED1_LEVEL_IND` (9)

**Source:** `code/code/misc/defs.h`

### Class Characteristics

```cpp
struct class_info {
    bool enabled;                   // Available at creation?
    classIndT class_lev_num;        // Index into level array
    int class_num;                  // Bitmask value
    sstring name;                   // Display name
    discNumT base_disc;             // Primary discipline
    discNumT sec_disc;              // Secondary discipline
    double prac_tweak;              // Practice point multiplier
    double hp_per_level;            // HP gained per level
    sstring abbr;                   // Abbreviation for display
};
```

**The classInfo Global Array:**

| Class | Practice Tweak | HP/Level | Abbrev |
|-------|----------------|----------|--------|
| Mage | 0.43 | 5.25 | M |
| Cleric | 0.47 | 5.6 | C |
| Warrior | 0.47 | 8.5 | W |
| Thief | 0.41 | 5.6 | T |
| Shaman | 0.39 | 5.25 | S |
| Deikhan | 0.44 | 7.5 | D |
| Monk | 0.44 | 5.25 | K |
| Ranger | 0.46 | 4.9 | R |
| Commoner | 0.40 | 5.0 | O |

**HP Formula:** Non-tank classes (Mage, Cleric, Thief, Shaman, Monk, Ranger) multiply base HP by `35/50` (70% of warriors).

**Source:** `code/code/misc/discipline.h`, `code/code/misc/constants.cc`

## Class Data Structures

### classIndT Enum

Maps classes to array indices for per-class tracking:

```cpp
enum classIndT {
    MAGE_LEVEL_IND = 0,
    CLERIC_LEVEL_IND = 1,
    WARRIOR_LEVEL_IND = 2,
    THIEF_LEVEL_IND = 3,
    SHAMAN_LEVEL_IND = 4,
    DEIKHAN_LEVEL_IND = 5,
    MONK_LEVEL_IND = 6,
    RANGER_LEVEL_IND = 7,
    COMMONER_LEVEL_IND = 8,
    UNUSED1_LEVEL_IND = 9,
    UNUSED2_LEVEL_IND = 10,
    MAX_SAVED_CLASSES = 11
};
const classIndT MAX_CLASSES = UNUSED1_LEVEL_IND;  // 9 used classes
```

**Purpose:** Converts single-bit class flags to array indices for `level[]` and `doneBasic[]` arrays.

**Source:** `code/code/misc/enum.h`

### playerData Structure

Stores class information in `TBeing`:

```cpp
class playerData {
  public:
    ubyte level[MAX_SAVED_CLASSES];     // Level in each class (0-60)
    ubyte max_level;                    // Highest level across all classes
    unsigned short Class;               // Bitmask of active classes
    byte doneBasic[MAX_SAVED_CLASSES];  // Whether completed basic skills
    // ... other player-specific fields
};
```

**Example storage:**
```cpp
TPerson player;
player.player.Class = CLASS_MAGE | CLASS_CLERIC;  // Dual-class M/C
player.player.level[MAGE_LEVEL_IND] = 25;         // Level 25 mage
player.player.level[CLERIC_LEVEL_IND] = 20;       // Level 20 cleric
player.player.max_level = 25;                     // Highest is 25
```

**Source:** `code/code/misc/being.h`

### pointData Structure

Class resources stored in `TBeing::points`:

```cpp
class pointData {
    short mana;             // Current mana (mages)
    short maxMana;          // Maximum mana capacity
    short hit;              // Current hit points
    short max_hit;          // Maximum hit points
    short move;             // Current movement
    short maxMove;          // Maximum movement
    short lifeforce;        // Current lifeforce (shamans)
    double piety;           // Current piety (clerics)
};
```

**Source:** `code/code/misc/being.h`

## Level System

### Level Caps and Ranges

| Level Range | Type | Description |
|-------------|------|-------------|
| 1-50 | Mortal | Normal player levels |
| 51-60 | Immortal | Builder/admin levels |

```cpp
const int MAX_MORT = 50;
const int GOD_LEVEL1 = MAX_MORT + 1;     // 51 - First immortal level
const int MAX_IMMORT = MAX_MORT + 10;    // 60 - Maximum level
```

**Immortal Levels:**
- GOD_LEVEL1 (51) - Builder level
- MAX_IMMORT (60) - Full administrator

Characters at GOD_LEVEL1+ automatically receive all wizard powers.

**Source:** `code/code/misc/defs.h`

### Level Tracking Methods

```cpp
// Get level in specific class
int getLevel(classIndT which) const {
    return player.level[which];
}

// Get highest level across all classes
int getMaxLevel() const {
    return player.max_level;
}

// Get level in class by bitmask
int getClassLevel(int Class) const {
    if (getClass() & Class) {
        return getLevel(classIndT(CountBits(Class) - 1));
    }
    return 0;
}

// Set level in specific class
void setLevel(classIndT which, int level) {
    player.level[which] = level;
}

// Set max level
void setMaxLevel(int level) {
    player.max_level = level;
}
```

**Usage examples:**
```cpp
int mageLevel = ch->getLevel(MAGE_LEVEL_IND);       // Direct array access
int warriorLevel = ch->getClassLevel(CLASS_WARRIOR); // By bitmask
int maxLevel = ch->getMaxLevel();                   // Highest level
```

**Source:** `code/code/misc/being.h`, `code/code/misc/multiclass.cc`

### calcMaxLevel()

Recalculates `max_level` as the highest level across all classes:

```cpp
void TBeing::calcMaxLevel() {
    int maxLev = 0;
    for (classIndT i = MIN_CLASS_IND; i < MAX_CLASSES; i++) {
        if (getLevel(i) > maxLev)
            maxLev = getLevel(i);
    }
    setMaxLevel(maxLev);
}
```

Called after any level change to maintain consistency.

**Source:** `code/code/misc/multiclass.cc`

## Class Query Methods

### getClass()

Returns the bitmask of active classes:

```cpp
unsigned short TBeing::getClass() const {
    return player.Class;
}
```

**Example values:**
```cpp
CLASS_MAGE                      // 1   (0x0001) - Pure mage
CLASS_MAGE | CLASS_CLERIC       // 3   (0x0003) - Mage/Cleric
CLASS_WARRIOR | CLASS_THIEF     // 12  (0x000C) - Warrior/Thief
```

### hasClass()

Checks if a character has a specific class:

```cpp
bool TBeing::hasClass(unsigned short bit, exactTypeT exact = EXACT_NO) const {
    if (!exact) {
        // Partial match: any overlap with bitmask
        if (getClass() & bit)
            return true;
    } else {
        // Exact match: all bits must be set
        if ((getClass() & bit) == bit)
            return true;
    }
    return false;
}

bool TBeing::hasClass(const char* arg, exactTypeT exact) const {
    int which = getClassNum(arg, exact);
    if (!which)
        return FALSE;
    return hasClass(which, exact);
}
```

**Usage examples:**
```cpp
// Check for single class
if (ch->hasClass(CLASS_MAGE))               // Has mage component?
if (ch->hasClass(CLASS_WARRIOR))            // Has warrior component?

// Check by name
if (ch->hasClass("cleric", EXACT_NO))       // Has cleric?
if (ch->hasClass("mage/cleric", EXACT_YES)) // Exact M/C combo?

// Check for multiple classes
if (ch->hasClass(CLASS_MAGE | CLASS_CLERIC, EXACT_YES)) {
    // Has both mage AND cleric (may have others too)
}
```

**Source:** `code/code/misc/multiclass.cc`

### getClassNum() and getClassIndNum()

Convert between class representations:

```cpp
// Class name → bitmask value
int TBeing::getClassNum(const char* arg, exactTypeT exact) const;

// classIndT → bitmask value
int TBeing::getClassNum(classIndT arg) const {
    return classInfo[arg].class_num;
}

// Bitmask value → classIndT
classIndT TBeing::getClassIndNum(unsigned short which) const;

// Class name → classIndT
classIndT TBeing::getClassIndNum(const char* arg, exactTypeT exact) const;
```

**Example conversions:**
```cpp
int mage_bit = ch->getClassNum("mage", EXACT_YES);     // Returns 1
int cleric_bit = ch->getClassNum("cleric", EXACT_YES); // Returns 2

classIndT mage_ind = ch->getClassIndNum(CLASS_MAGE);   // Returns 0
classIndT cleric_ind = ch->getClassIndNum(CLASS_CLERIC); // Returns 1
```

**Source:** `code/code/misc/multiclass.cc`

## Multiclass System

### Multiclass Checks

Functions determine the number of active classes:

```cpp
// Count number of set bits in Class bitmask
int NumClasses(int Class) {
    int tot = 0;
    for (classIndT i = MIN_CLASS_IND; i < MAX_CLASSES; i++)
        if (Class & classInfo[i].class_num)
            tot++;
    return tot;
}

// TBeing methods
bool isSingleClass() const {
    return (NumClasses(getClass()) == 1);
}

bool isDoubleClass() const {
    return (NumClasses(getClass()) == 2);
}

bool isTripleClass() const {
    return (NumClasses(getClass()) >= 3);
}

int howManyClasses() const {
    return NumClasses(getClass());
}
```

**Usage examples:**
```cpp
if (ch->isSingleClass()) {
    // Pure class character
}

int classCount = ch->howManyClasses();  // 1, 2, 3, etc.
```

**Source:** `code/code/misc/multiclass.cc`

### bestClass()

Returns the class with the highest level:

```cpp
classIndT TBeing::bestClass() const {
    classIndT tBest = MAGE_LEVEL_IND;
    int tLev = 0;

    for (classIndT i = MIN_CLASS_IND; i < MAX_CLASSES; i++) {
        if (getLevel(i) > tLev) {
            tLev = getLevel(i);
            tBest = i;
        }
    }
    return tBest;
}
```

Used as the "primary" class for certain calculations.

**Source:** `code/code/misc/multiclass.cc`

### Multiclass Display

#### getProfName()

Returns full name of all classes:

```cpp
sstring TBeing::getProfName() const {
    sstring buf;
    bool first = true;

    for (classIndT i = MIN_CLASS_IND; i < MAX_CLASSES; i++) {
        if (hasClass(classInfo[i].class_num)) {
            if (!first)
                buf += "/";
            buf += classInfo[i].name;
            first = false;
        }
    }
    return buf;
}
```

**Output examples:**
```
Single Mage:        "mage"
Dual M/C:           "mage/cleric"
Triple M/C/W:       "mage/cleric/warrior"
```

#### getProfAbbrevName()

Returns abbreviated display for multiclass characters:

```cpp
sstring TBeing::getProfAbbrevName(unsigned short code) {
    int numClasses = NumClasses(code);

    if (numClasses == 1) {
        // Single class: return first 4 chars capitalized
        for (classIndT i = MIN_CLASS_IND; i < MAX_CLASSES; i++) {
            if (code & classInfo[i].class_num) {
                sstring name = classInfo[i].name;
                name[0] = toupper(name[0]);
                return name.substr(0, 4);  // "Mage", "Cler", "Warr"
            }
        }
    } else {
        // Multi-class: return abbreviations separated by /
        sstring buf;
        bool first = true;

        for (classIndT i = MIN_CLASS_IND; i < MAX_CLASSES; i++) {
            if (code & classInfo[i].class_num) {
                if (!first)
                    buf += "/";
                buf += classInfo[i].abbr;
                first = false;
            }
        }
        return buf;  // "M/C", "W/T", "M/C/W"
    }
}
```

**Output examples:**
```
Single Mage:        "Mage"
Single Cleric:      "Cler"
Dual M/C:           "M/C"
Triple M/C/W:       "M/C/W"
```

**Source:** `code/code/misc/multiclass.cc`

### Multiclass Penalties

**Experience Penalties:**

Characters with multiple classes gain experience more slowly. The `gain_exp()` function divides base XP by `howManyClasses()` **twice**:

```cpp
// From limits.cc - gain_exp()
gain /= ch->howManyClasses();  // First division
gain /= ch->howManyClasses();  // Second division
```

This creates quadratic XP penalties:
- 1 class: `XP / 1 / 1 = XP` (100%)
- 2 classes: `XP / 2 / 2 = XP / 4` (25% per class)
- 3 classes: `XP / 3 / 3 = XP / 9` (11% per class)

**Practice Point Penalties:**

Practice points are also divided by class count:

```cpp
// From pracsPerLevel()
if (multiclass) {
    pracs /= howManyClasses();
}
```

**Source:** `code/code/misc/limits.cc`, `code/code/misc/limits.cc`

## Class-Specific Resources

### Mana (Mages)

Used for spell casting by mages and some hybrid classes.

```cpp
int TBeing::getMana() const {
    return points.mana;
}

int TBeing::getMaxMana() const {
    return points.maxMana;
}

void TBeing::addToMana(int mana) {
    points.mana += mana;
}

void TBeing::setMana(int mana) {
    points.mana = mana;
}

short int TBeing::manaLimit() const;  // Maximum capacity calculation
int TBeing::manaGain();               // Regeneration per round
```

**Mana characteristics:**
- Scales with Intelligence stat
- Regenerates each combat round (1.2 seconds)
- Resting/sleeping increases regeneration rate
- Required for mage spell casting

### Piety (Clerics)

Used for cleric spells. Floating-point value for fine-grained tracking.

```cpp
double TBeing::getPiety() const {
    return points.piety;
}

void TBeing::addToPiety(double num) {
    points.piety += num;
}

void TBeing::setPiety(double piety) {
    points.piety = piety;
}

double TBeing::pietyLimit() const;    // Maximum capacity
double TBeing::pietyGain(double);     // Regeneration

bool TBeing::noPiety(double piety) const {
    return (getPiety() < piety);
}
```

**Piety characteristics:**
- Floating-point for precise tracking
- Scales with Wisdom stat
- Regenerates based on worship and rest
- Required for cleric spell casting

**CRITICAL:** Piety costs in `spellInfo` are divided by 4:

```cpp
enum pietyCostT {
    PRAY_025 = 5,      // Actual: 1.25
    PRAY_100 = 20,     // Actual: 5.0
    PRAY_200 = 40      // Actual: 10.0
};
// spellInfo constructor: minPiety = minPiety / 4
```

### Lifeforce (Shamans)

Used for shaman spells. Similar to mana but distinct resource pool.

```cpp
int TBeing::getLifeforce() const {
    return points.lifeforce;
}

int TBeing::getMaxLifeforce() const {
    // Calculated from level and stats
}

void TBeing::addToLifeforce(int lifeforce) {
    points.lifeforce += lifeforce;
}

void TBeing::setLifeforce(int lifeforce) {
    points.lifeforce = lifeforce;
}

int TBeing::lifeforceLimit() const;   // Maximum capacity
int TBeing::lifeforceGain();          // Regeneration

bool TBeing::noLifeforce(int lifeforce) const {
    return (getLifeforce() < lifeforce);
}
```

**Lifeforce characteristics:**
- Integer-based tracking
- Scales with character level
- Regenerates naturally
- Required for shaman spell casting

### Movement Points (Universal)

Used by all classes for travel and special abilities.

```cpp
int TBeing::getMove() const {
    return points.move;
}

int TBeing::getMaxMove() const {
    return points.maxMove;
}

void TBeing::addToMove(int add) {
    points.move += add;
}

void TBeing::setMove(int move) {
    points.move = move;
}

void TBeing::setMaxMove(int move) {
    points.maxMove = move;
}

bool TBeing::tooTired() const {
    return (getMove() <= 0);
}
```

**Movement characteristics:**
- Universal resource (all classes)
- Required for movement commands
- Drains faster when crawling or burdened
- Regenerates based on position and stats

### Hit Points (Universal)

Used by all classes for survival.

```cpp
int TBeing::getHit() const {
    return points.hit;
}

int TBeing::getMaxHit() const {
    return points.max_hit;
}

void TBeing::addToHit(int add) {
    points.hit += add;
}

void TBeing::setHit(int hit) {
    points.hit = hit;
}

void TBeing::setMaxHit(int hit) {
    points.max_hit = hit;
}
```

**HP characteristics:**
- Universal resource (all classes)
- Scales with Constitution and level
- HP per level varies by class (see classInfo table)
- Does NOT regenerate naturally (requires spells/rest)

**Source:** `code/code/misc/being.h`

## Class-Specific Combat Mechanics

### Class-Specific AI

Each class has unique combat functions for mob behavior:

```cpp
// From mobact.cc - classStuff() dispatcher
int TMonster::classStuff(int pulse) {
    switch (bestClass()) {
        case WARRIOR_LEVEL_IND:
            return fighterMove();
        case MONK_LEVEL_IND:
            return monkMove();
        case THIEF_LEVEL_IND:
            return thiefMove();
        case MAGE_LEVEL_IND:
            return mageMove();
        case CLERIC_LEVEL_IND:
            return clerMove();
        case SHAMAN_LEVEL_IND:
            return shamanMove();
        case DEIKHAN_LEVEL_IND:
            return deikhanMove();
        case RANGER_LEVEL_IND:
            return rangMove();
    }
    return FALSE;
}
```

**Class Combat Functions:**

| Function | Class | Typical Actions |
|----------|-------|-----------------|
| `fighterMove()` | Warrior | Bash, bodyslam, spin, kick, disarm |
| `monkMove()` | Monk | Springleap, hurl, bonebreak, shoulder throw, chi |
| `thiefMove()` | Thief | Backstab, stab, disarm |
| `mageMove()` | Mage | Offensive spells (fireball, lightning, etc.) |
| `clerMove()` | Cleric | Healing, harm spells |
| `shamanMove()` | Shaman | Spirit spells, flatulence |
| `deikhanMove()` | Deikhan | Charge (mounted), fighter moves |
| `rangMove()` | Ranger | Nature skills |

Each function selects appropriate attacks based on situation, HP percentage, and available resources.

**Source:** `code/code/misc/mobact.cc` (classStuff and individual class functions)

---

# II. Race System

SneezyMUD defines **127 races** (indices 0-126) spanning playable humanoids, animals, mythical creatures, and monsters. Each race has unique stat distributions, physical characteristics, abilities, and weaknesses.

**Design Pattern:** Flyweight - single shared `Race` instance per race type, referenced by all beings of that race.

## race_t Enumeration

The `race_t` enum defines all 127 races. Only the first 6 (plus HUMAN) are playable by players.

**Source:** `/code/code/misc/race.h` (lines 26-161)

### Playable Races (1-6)

| Race | Value | Description | Primary Role |
|------|-------|-------------|--------------|
| `RACE_HUMAN` | 1 | Balanced baseline | Versatile |
| `RACE_ELVEN` | 2 | High DEX/WIS, low STR/BRA | Caster/Rogue |
| `RACE_DWARF` | 3 | High CON/STR, low DEX, poison immunity | Tank/Crafter |
| `RACE_HOBBIT` | 4 | High DEX/PER/SPE, low STR, stealth bonus | Rogue/Scout |
| `RACE_GNOME` | 5 | Similar to Dwarf | Variant |
| `RACE_OGRE` | 6 | High STR/BRA, low INT, giant lore | Fighter |

**Special race 0:** `RACE_NORACE` - Invalid/uninitialized state.

### NPC/Monster Races (7-126)

**Animals:** WOLF, BEAR, LION, TIGER, DEER, RABBIT, RAT, BAT, EAGLE, HAWK, OWL, CAT, DOG, HORSE, MULE, PIG, COW, CHICKEN, SNAKE, SPIDER, SCORPION, ANT, BEE, FISH, SHARK, WHALE, DOLPHIN, OCTOPUS

**Mythical:** DRAGON, WYVERN, GRIFFON, PHOENIX, PEGASUS, UNICORN, BASILISK, MEDUSA, HYDRA, CHIMERA, MANTICORE, SPHINX

**Humanoids:** ORC, TROLL, GOBLIN, KOBOLD, GNOLL, HOBGOBLIN, BUGBEAR, GIANT, CYCLOPS, OGRE_MAGI, MINOTAUR, CENTAUR, SATYR, NYMPH, DRYAD

**Planar:** DEMON, DEVIL, ANGEL, ARCHON, DJINNI, EFREET, ELEMENTAL_FIRE, ELEMENTAL_WATER, ELEMENTAL_AIR, ELEMENTAL_EARTH

**Undead:** SKELETON, ZOMBIE, GHOUL, WIGHT, WRAITH, SPECTRE, VAMPIRE, LICH, MUMMY

**Constructs:** GOLEM_FLESH, GOLEM_CLAY, GOLEM_STONE, GOLEM_IRON

**Vegetation:** TREANT, FUNGUS, MOLD, OOZE, SLIME

**Exotic:** BEHOLDER, MIND_FLAYER, ABOLETH, RUST_MONSTER, GELATINOUS_CUBE

## Race Class Structure

The `Race` class stores all racial properties and provides query methods.

**Source:** `/code/code/misc/race.h` (lines 217-385)

### Core Members

| Member | Type | Description |
|--------|------|-------------|
| `raceType` | `race_t` | Race enum value |
| `Kingdom` | `lore_t` | Lore category (ANIMAL, PEOPLE, UNDEAD, etc.) |
| `singular_name` | `sstring` | "human", "dwarf" |
| `plural_name` | `sstring` | "humans", "dwarves" |
| `proper_name` | `sstring` | "Human", "Dwarf" |
| `bodyType` | `body_t` | Body anatomy type (BODY_HUMANOID, etc.) |
| `racialCharacteristics` | `unsigned short` | Bitflags (WINGED, AQUATIC, etc.) |
| `baseStats` | `Stats` | Base values for 13 attributes |
| `naturalImmunities` | `Immunities` | Resistances to 28 damage types |
| `talents` | `unsigned int` | Racial talent bitmask (up to 9) |
| `hpMod` | `int` | HP bonus per level |
| `moveMod` | `int` | Movement point bonus |
| `manaMod` | `int` | Mana bonus per level |
| `searchMod` | `int` | Search skill bonus |
| `lineOfSightMod` | `int` | Scan range bonus |
| `visionBonus` | `int` | Vision quality bonus |
| `drinkMod` | `float` | How filling drinks are (default 1.0) |
| `foodMod` | `float` | How filling food is (default 1.0) |

### Physical Dimensions

Each race defines random size ranges using dice notation (base + NumDice d DieSize):

```cpp
int baseAge, ageNumDice, ageDieSize;
int baseMaleHeight, maleHtNumDice, maleHtDieSize;
int baseMaleWeight, maleWtNumDice, maleWtDieSize;
int baseFemaleHeight, femaleHtNumDice, femaleHtDieSize;
int baseFemaleWeight, femaleWtNumDice, femaleWtDieSize;
```

**Example (RACE_HUMAN):**
```
Age: 15 + 1d4 years (15-19)
Male Height: 62 + 1d17 inches (63-79, avg 70.5")
Male Weight: 140 + 6d10 pounds (146-200, avg 173)
Female Height: 60 + 1d12 inches (61-72, avg 66.5")
Female Weight: 100 + 4d10 pounds (104-140, avg 122)
```

### Corpse/Dissection

```cpp
float corpse_const;                // Multiplier for corpse size/weight
dissectInfo tDissectItem[2];       // Up to 2 item drops on dissection
```

## Racial Characteristics (Bitflags)

The `racialCharacteristics` field uses 11 bit flags defining physical properties.

**Source:** `/code/code/misc/race.h` (lines 171-182)

| Flag | Bit | Effect | Example Races |
|------|-----|--------|---------------|
| `DUMBANIMAL` | 0 | No intelligence, wild behavior | WOLF, BEAR, RAT |
| `BONELESS` | 1 | No skeletal system, immune to bone breaks | SLIME, OOZE, JELLYFISH |
| `WINGED` | 2 | Can fly naturally | DRAGON, PEGASUS, EAGLE, BAT |
| `CLIMBER` | 3 | Can climb walls | SPIDER, GECKO, APE |
| `EXTRAPLANAR` | 4 | Not from prime material plane | DEMON, DEVIL, ANGEL, ELEMENTAL |
| `AQUATIC` | 5 | Water-breathing, drowns in air | FISH, SHARK, OCTOPUS |
| `FOURLEGGED` | 6 | Four-legged anatomy | HORSE, DOG, LION, BEAR |
| `COLDBLOODED` | 7 | Temperature-dependent metabolism | SNAKE, LIZARD, FISH |
| `RIDABLE` | 8 | Can be mounted | HORSE, PEGASUS, DRAGON |
| `MAGICFLY` | 9 | Magical flight ability | DJINNI, EFREET |
| `FEATHERED` | 10 | Bird-like appearance | BIRD, GRIFFON, PHOENIX |

### Query Methods

```cpp
bool isWinged() const;
bool isFourLegged() const;
bool hasNoBones() const;
bool hasMagicFly() const;
bool isAquatic() const;
bool isRidable() const;
bool isDumbAnimal() const;
bool isClimber() const;
bool isExtraplanar() const;
bool isColdBlooded() const;
bool isFeathered() const;
```

## Body Types (body_t)

The `body_t` enum defines **81 distinct body types** governing anatomy, equipment slots, and limb configurations.

**Source:** `/code/code/misc/body.h` (lines 18-81)

### Major Categories

**Humanoid (bipedal, two arms):**
- `BODY_HUMANOID` - Standard human-like body
- `BODY_MINOTAUR` - Bull-headed humanoid
- `BODY_PIERCER` - Humanoid with piercing attacks

**Hybrid Humanoid:**
- `BODY_CENTAUR` - Humanoid torso on four-legged body
- `BODY_NAGA` - Humanoid upper body, serpent lower body
- `BODY_SATYR` - Goat-legged humanoid

**Quadruped (four-legged):**
- `BODY_FOUR_LEG` - Generic quadruped (cats, dogs, lions)
- `BODY_FOUR_HOOF` - Hoofed quadruped (horses, deer)
- `BODY_TURTLE` - Four-legged with shell
- `BODY_ALLIGATOR` - Four-legged reptilian

**Winged:**
- `BODY_BIRD` - Two-legged avian (eagle, hawk)
- `BODY_BAT` - Flying mammal
- `BODY_DRAGON` - Four-legged winged reptile
- `BODY_WYVERN` - Two-legged winged dragon
- `BODY_GRIFFON` - Hybrid winged creature
- `BODY_PEGASUS` - Winged horse

**Aquatic:**
- `BODY_FISH` - Standard fish body
- `BODY_FISHMAN` - Humanoid aquatic (merfolk)
- `BODY_DOLPHIN`, `BODY_WHALE` - Marine mammals
- `BODY_OCTOPUS` - Eight tentacles

**Multi-limbed:**
- `BODY_SPIDER` - Eight legs
- `BODY_CENTIPEDE` - Many-segmented legs
- `BODY_INSECT` - Six legs
- `BODY_BUGBEAR` - Large multi-armed humanoid

**Serpentine:**
- `BODY_SNAKE` - Legless serpent
- `BODY_HYDRA` - Multi-headed serpent

**Exotic:**
- `BODY_TREE` - Plant-based, rooted
- `BODY_SLIME` - Amorphous blob
- `BODY_ORB` - Spherical (beholder)
- `BODY_GOLEM` - Magical construct
- `BODY_GHOST` - Incorporeal
- `BODY_PIERCER` - Tentacled horror

### Body Type Effects

1. **Equipment Slots:** Body type determines which of 24 wearSlotT slots are available
2. **Limb Configuration:** Defines which body parts can be targeted, broken, or severed
3. **Movement:** Affects movement messages, crawling behavior, and swimming
4. **Dissection:** Different body types yield different component items

## Lore Categories (lore_t)

The `Kingdom` field categorizes races into 8 lore types for skill/spell targeting.

**Source:** `/code/code/misc/race.h` (lines 183-193)

| Lore | Value | Description | Example Races |
|------|-------|-------------|---------------|
| `LORE_ANIMAL` | 0 | Natural creatures | WOLF, BEAR, HORSE, EAGLE |
| `LORE_VEGGIE` | 1 | Plants/vegetation | TREANT, FUNGUS, MOLD |
| `LORE_DIABOLIC` | 2 | Demons/devils | DEMON, DEVIL, IMP |
| `LORE_REPTILE` | 3 | Reptilian creatures | DRAGON, SNAKE, LIZARD |
| `LORE_UNDEAD` | 4 | Undead/skeletal | SKELETON, ZOMBIE, VAMPIRE, LICH |
| `LORE_GIANT` | 5 | Giants/large humanoids | GIANT, CYCLOPS, OGRE |
| `LORE_PEOPLE` | 6 | Humanoid civilizations | HUMAN, ELVEN, DWARF, ORC |
| `LORE_OTHER` | 7 | Everything else | SLIME, ELEMENTAL, GOLEM |

**Usage:**
- Spell targeting (`TARGET_LORE_ANIMAL`)
- Skill effectiveness (Ranger animal lore)
- AI behavior (fear/hate targeting)
- Knowledge systems

## Racial Talents

The `talents` field is a bitmask supporting up to 9 racial talents.

**Source:** `/code/code/misc/race.h` (lines 199-210)

| Talent | Bit | Effect | Example Races |
|--------|-----|--------|---------------|
| `TALENT_FAST_REGEN` | 0 | Regeneration bonus | TROLL, OGRE |
| `TALENT_FISHEATER` | 1 | Bonus nutrition from fish | BEAR, OTTER |
| `TALENT_MEATEATER` | 2 | Bonus nutrition from meat | WOLF, LION, TROLL |
| `TALENT_TATTOOED` | 3 | Tattoo spell capacity | (player races) |
| `TALENT_GARBAGEEATER` | 4 | Bonus nutrition from trash | RAT, RACCOON |
| `TALENT_LIMB_REGROWTH` | 5 | Can regrow severed limbs | TROLL, HYDRA |
| `TALENT_INSECT_EATER` | 6 | Bonus nutrition from insects | ANTEATER, FROG |
| `TALENT_FROGSLIME_SKIN` | 7 | Poison/acid resistance | FROG, TOAD |
| `TALENT_MUSK` | 8 | Combat defensive ability | SKUNK |

**Implementation:** Checked via `hasTalent(talentTypeT)` method.

## Immunities System

Each race defines percentage resistance (0-100%) to 28 damage/effect types via the `Immunities` class.

**Source:** `/code/code/misc/immunity.h` (lines 16-65)

### Immunity Types

| Type | Description | Type | Description |
|------|-------------|------|-------------|
| `IMMUNE_HEAT` | Fire/heat damage | `IMMUNE_COLD` | Freezing damage |
| `IMMUNE_ACID` | Acid damage | `IMMUNE_POISON` | Poison effects |
| `IMMUNE_SLEEP` | Sleep spells | `IMMUNE_PARALYSIS` | Paralysis/stun |
| `IMMUNE_CHARM` | Charm/domination | `IMMUNE_PIERCE` | Piercing weapons |
| `IMMUNE_SLASH` | Slashing weapons | `IMMUNE_BLUNT` | Blunt weapons |
| `IMMUNE_NONMAGIC` | Non-magical damage | `IMMUNE_PLUS1` | +1 weapon bypass |
| `IMMUNE_PLUS2` | +2 weapon bypass | `IMMUNE_PLUS3` | +3 weapon bypass |
| `IMMUNE_AIR` | Air/wind damage | `IMMUNE_ENERGY` | Magical energy |
| `IMMUNE_ELECTRICITY` | Lightning | `IMMUNE_DISEASE` | Disease/infection |
| `IMMUNE_SUFFOCATION` | Drowning/suffocation | `IMMUNE_SKIN_COND` | Burns/frostbite |
| `IMMUNE_BONE_COND` | Bone breaks | `IMMUNE_BLEED` | Bleeding effects |
| `IMMUNE_WATER` | Water damage | `IMMUNE_DRAIN` | Drain/negative energy |
| `IMMUNE_FEAR` | Fear effects | `IMMUNE_EARTH` | Earth/stone damage |
| `IMMUNE_SUMMON` | Summoning effects | `IMMUNE_HOLY` | Holy damage |

### Resistance Stacking

Racial immunities **stack additively** with equipment and spell immunities:

```cpp
totalResistance = racialImmunity + equipmentImmunity + spellImmunity;
totalResistance = min(totalResistance, 100);  // Capped at 100%
```

### Example Racial Immunities

**RACE_DWARF:**
```
IMMUNE_POISON 40      // 40% poison resistance
IMMUNE_CHARM 75       // 75% charm resistance
IMMUNE_SLEEP 75       // 75% sleep resistance
```

**RACE_DRAGON:**
```
IMMUNE_HEAT 25
IMMUNE_COLD 25
IMMUNE_SLEEP 100      // Complete sleep immunity
IMMUNE_CHARM 100      // Cannot be charmed
IMMUNE_FEAR 100       // Fearless
IMMUNE_NONMAGIC 50    // 50% resistance to mundane damage
IMMUNE_PIERCE 25
IMMUNE_SLASH 25
IMMUNE_BLUNT 25
```

**RACE_VAMPIRE:**
```
IMMUNE_COLD 100       // Complete cold immunity
IMMUNE_SLEEP 100
IMMUNE_CHARM 100
IMMUNE_DISEASE 100
IMMUNE_DRAIN 100      // Immune to life drain
IMMUNE_HOLY -50       // VULNERABILITY to holy (negative %)
```

**Negative values indicate vulnerability** - damage is amplified rather than reduced.

## Base Stats

Each race defines **13 base stat values** that serve as the foundation for character stats.

**Source:** Race data files, `baseStats` member of Race class

### Stat Range

- **Minimum:** 5 (crippled)
- **Baseline:** 105 (neutral/average)
- **Maximum:** 215 (superhuman)

Higher values are strictly better. The 200-point range allows for extreme specialization.

### Playable Race Stat Comparison

| Stat | Human | Elven | Dwarf | Hobbit | Gnome | Ogre |
|------|-------|-------|-------|--------|-------|------|
| **STR** | 105 | 80 | 130 | 55 | 115 | 165 |
| **BRA** | 105 | 65 | 130 | 55 | 115 | 155 |
| **CON** | 105 | 45 | 155 | 80 | 135 | 125 |
| **DEX** | 105 | 130 | 85 | 155 | 105 | 80 |
| **AGI** | 105 | 125 | 85 | 130 | 105 | 80 |
| **INT** | 105 | 130 | 80 | 83 | 125 | 80 |
| **WIS** | 105 | 155 | 95 | 83 | 110 | 80 |
| **FOC** | 105 | 110 | 95 | 83 | 110 | 80 |
| **PER** | 105 | 115 | 100 | 115 | 110 | 80 |
| **CHA** | 105 | 125 | 85 | 105 | 100 | 75 |
| **KAR** | 105 | 105 | 105 | 105 | 105 | 95 |
| **SPE** | 105 | 125 | 55 | 155 | 95 | 80 |
| **LUC** | 105 | 105 | 105 | 105 | 105 | 105 |

## Point Modifiers

Three modifiers affect derived resource pools at each level.

### HP Modifier (hpMod)

Bonus HP per level:

| Race | hpMod | Effect |
|------|-------|--------|
| HUMAN, ELVEN | 0 | Standard HP progression |
| HOBBIT | -1 | -1 HP/level (more fragile) |
| DWARF | 1 | +1 HP/level |
| OGRE | 4 | +4 HP/level (very durable) |
| DRAGON | 2 | +2 HP/level |
| TROLL | 3 | +3 HP/level |

**Formula:** `totalHP = baseHPForLevel + (hpMod * level)`

### Movement Modifier (moveMod)

Bonus movement points (affects stamina pool):

| Race | moveMod | Effect |
|------|---------|--------|
| DWARF | -20 | Slower |
| HUMAN | 0 | Standard |
| ELVEN | 25 | Faster |
| HOBBIT | 40 | Very fast |
| OGRE | 40 | Surprisingly fast |

**Note:** Movement cost per step is **separate** from movement pool size. This modifier only affects the pool.

### Mana Modifier (manaMod)

Bonus mana per level:

| Race | manaMod | Effect |
|------|---------|--------|
| DWARF, OGRE | -1 | Less mana |
| HUMAN | 0 | Standard |
| ELVEN | 1 | More mana |
| DRAGON | 2 | Much more mana |

**Formula:** `totalMana = baseManaForLevel + (manaMod * level)`

## Perception and Vision

Three fields affect sensory capabilities.

### Search Modifier (searchMod)

Flat bonus to Search skill checks:

| Race | searchMod |
|------|-----------|
| HUMAN | 1 |
| HOBBIT | 5 (+5 to search) |
| ELVEN | 3 |

### Line of Sight (lineOfSightMod)

Bonus to scan range:

| Race | lineOfSightMod |
|------|----------------|
| ELVEN | 5 (+5 rooms) |
| HUMAN | 0 |

### Vision Bonus (visionBonus)

Quality of vision detail:

| Race | visionBonus |
|------|-------------|
| TROLL | 3 |
| EAGLE | 5 |

## Consumption Modifiers

Two floats control hunger/thirst rates.

### Food Modifier (foodMod)

How filling food is (1.0 = baseline):

| Race | foodMod | Interpretation |
|------|---------|----------------|
| HOBBIT | 0.5 | Food is twice as filling |
| DWARF | 0.75 | Food is more filling |
| HUMAN | 1.0 | Standard |
| OGRE | 1.25 | Needs 25% more food |
| DRAGON | 1.5 | Needs 50% more food |

**Formula:** `fullnessGained = foodItem.fullness * (1.0 / foodMod)`

### Drink Modifier (drinkMod)

How filling drinks are:

| Race | drinkMod | Interpretation |
|------|----------|----------------|
| OGRE | 0.75 | Less thirsty |
| HUMAN | 1.0 | Standard |
| DWARF | 1.25 | More thirsty (loves alcohol) |
| HOBBIT | 1.3 | Very thirsty |

## Integration with TBeing

The `TBeing` class stores a pointer to the shared `Race` instance.

**Source:** `/code/code/misc/being.h`

### Storage

```cpp
protected:
    Race* race;  // Pointer to flyweight instance
```

### Query Methods

```cpp
race_t getRace() const;
Race* getMyRace() const;
bool isSameRace(const TBeing* ch) const;

// Characteristic queries (delegate to race)
bool isHumanoid() const { return race->isHumanoid(); }
bool isAquatic() const;
bool isFourLegged() const;
bool isWinged() const;
bool isDumbAnimal() const;
bool isLycanthrope() const;
bool isColdBlooded() const;
bool hasNoBones() const;
```

## Global Race Array

All race instances are stored in a global array for O(1) lookup.

**Source:** `/code/code/misc/race.cc`

```cpp
Race* Races[MAX_RACIAL_TYPES];  // MAX_RACIAL_TYPES = 127
```

### Initialization

At boot time, `initRaces()` creates all 127 instances:

```cpp
void initRaces() {
    for (int i = 0; i < MAX_RACIAL_TYPES; i++) {
        Races[i] = new Race(static_cast<race_t>(i));
        Races[i]->initRace(RaceNames[i]);  // Loads from lib/races/
    }
}
```

### Access Pattern

```cpp
Race* myRace = Races[RACE_DWARF];
int baseSTR = myRace->getBaseStats().get(STAT_STR);  // 130
```

---

# III. Stats and Attributes

SneezyMUD uses a comprehensive attribute system with **13 primary stats** that govern character capabilities. Stats follow a **non-linear scaling curve** (power 1.4) that amplifies differences at extreme values, making high stats significantly more valuable than average ones.

## Primary Stats

### The 13 Attributes

| Stat | Type | Primary Use | Secondary Effects |
|------|------|-------------|-------------------|
| **STR** (Strength) | Physical | Melee damage, carrying capacity | Can train SKILL_IRON_MUSCLES for bonus |
| **BRA** (Brawn) | Physical | Physical resistance, toughness | Constitution-adjacent durability |
| **CON** (Constitution) | Physical | Max HP, regeneration rate | Health pool and recovery |
| **DEX** (Dexterity) | Physical | Combat accuracy, dodge | Primary combat stat for hit/defense |
| **AGI** (Agility) | Physical | Combat reactions, movement | Secondary combat stat |
| **INT** (Intelligence) | Mental | Spell success, mana, learning | Practice point efficiency |
| **WIS** (Wisdom) | Mental | Spell damage, perception | Spell resistance |
| **FOC** (Focus) | Mental | Spell concentration | Interruption resistance |
| **PER** (Perception) | Mental | Awareness, detection | Hidden object/trap finding |
| **CHA** (Charisma) | Social | Shop prices, NPC reactions | Leadership |
| **KAR** (Karma) | Intangible | Critical hit chance, luck | Moral standing |
| **SPE** (Speed) | Physical | Attack frequency, movement | Combat speed, urban bonus |
| **LUC** (Luck) | Intangible | Random events | Critical outcomes |

**Source:** `code/code/misc/enum.h`, `code/code/misc/stats.h`

### statTypeT Enum

```cpp
enum statTypeT {
    STAT_STR = 0,   STAT_BRA = 1,   STAT_CON = 2,
    STAT_DEX = 3,   STAT_AGI = 4,   STAT_INT = 5,
    STAT_WIS = 6,   STAT_FOC = 7,   STAT_PER = 8,
    STAT_CHA = 9,   STAT_KAR = 10,  STAT_SPE = 11,
    STAT_LUC = 12,  STAT_EXT = 13,  // EXT always 0, reserved
    MAX_STATS = 14  // Array size
};
const int MAX_STATS_USED = STAT_LUC;  // For display (excludes EXT)
```

## Stat Storage and Access

### Data Structure

```cpp
class Stats {
    short values[MAX_STATS]{0};  // Array of 13 stat values
public:
    short get(statTypeT stat) const;
    short set(statTypeT stat, short val);
    short add(statTypeT stat, short mod);
};

class TBeing {
protected:
    Race* race;              // Racial template with baseStats
    Stats chosenStats;       // Player-customized points (creation)
    Stats curStats;          // Current stats (with equipment/spells)
};
```

**Source:** `code/code/misc/stats.h`, `code/code/misc/being.h`

### Stat Layers (statSetT)

The system tracks stats through multiple layers:

| Layer | Source | Description |
|-------|--------|-------------|
| `STAT_CHOSEN` | `chosenStats` | Points allocated at character creation |
| `STAT_CURRENT` | `curStats` | Current value including equipment and spell effects |
| `STAT_NATURAL` | Calculated | Base + chosen + age + territory + skills |
| `STAT_RACE` | `race->baseStats` | Racial baseline values |
| `STAT_AGE` | `age_mod_for_stat()` | Age-based modifiers (peak at 20-30, decline to 80) |
| `STAT_TERRITORY` | `territory_adjustment()` | Homeland bonuses/penalties |

**Source:** `code/code/misc/stats.h` (lines 39-47)

### Stat Access API

```cpp
// TBeing methods
int getStat(statSetT fromSet, statTypeT whichStat) const;
int setStat(statSetT whichSet, statTypeT whichStat, int value);
int addToStat(statSetT whichSet, statTypeT whichStat, int modifier);

// Stats class methods (direct)
short get(statTypeT stat) const { return values[stat]; }
short set(statTypeT stat, short val);
short add(statTypeT stat, short mod);
```

**Usage examples:**
```cpp
int currentSTR = getStat(STAT_CURRENT, STAT_STR);      // With equipment
int naturalDEX = getStat(STAT_NATURAL, STAT_DEX);      // Natural + age + territory
int racialINT = getStat(STAT_RACE, STAT_INT);          // Racial baseline
int chosenWIS = getStat(STAT_CHOSEN, STAT_WIS);        // Player allocation
```

## Stat Calculation System

### getStat() Implementation

The `STAT_NATURAL` calculation combines all sources:

```cpp
int TBeing::getStat(statSetT fromSet, statTypeT whichStat) const {
    int amount;

    switch (fromSet) {
        case STAT_NATURAL:
            // Base: racial baseline
            amount = race->baseStats.get(whichStat);

            // + Player customization points
            amount += chosenStats.get(whichStat);

            // + Age modifiers (physical stats decline after 30)
            int my_age = age()->year - getBaseAge() + 17;
            if (!isVampire())
                amount += age_mod_for_stat(this, my_age, whichStat);

            // + Territory adjustments (homeland bonuses)
            amount += territory_adjustment(player.hometerrain, whichStat);

            // + Skill bonuses (e.g., SKILL_IRON_MUSCLES → STR)
            if (discs && whichStat == STAT_STR &&
                doesKnowSkill(SKILL_IRON_MUSCLES))
                amount += getSkillValue(SKILL_IRON_MUSCLES) / 8;

            // + Vampire bonuses (+25 to STR/SPE/CHA)
            if (isVampire() && (whichStat == STAT_STR ||
                                whichStat == STAT_SPE ||
                                whichStat == STAT_CHA))
                amount += 25;

            return amount;

        case STAT_CURRENT:
            return curStats.get(whichStat);  // Includes affects

        case STAT_CHOSEN:
            return chosenStats.get(whichStat);

        case STAT_RACE:
            return race->baseStats.get(whichStat);

        case STAT_AGE:
            return isVampire() ? 0 : age_mod_for_stat(this, my_age, whichStat);

        case STAT_TERRITORY:
            return territory_adjustment(player.hometerrain, whichStat);
    }
    return 0;
}
```

**Source:** `code/code/misc/stats.cc` (lines 52-144)

### Default Values

**Range:** 5 (minimum) to 205 (maximum) — 200-point span

**Default initialization:** `Stats::Stats()` initializes all stats to **150** except `STAT_EXT` (0)

This places new characters slightly above the 105 baseline (neutral) but not at maximum.

**Source:** `code/code/misc/stats.h` (lines 55-61)

## The plotStat() Formula

### Power-Law Scaling

The core stat-to-modifier conversion uses a **power law with exponent 1.4** to create non-linear scaling:

```cpp
template <typename T>
T plotStat(statSetT whichSet, statTypeT whichStat,
           T minValue, T maxValue, T average, double power = 1.4) const {
    const int stat = getStat(whichSet, whichStat);
    return plotValue<int, T>(stat, 5, 205, minValue, maxValue, average, power);
}
```

**Source:** `code/code/misc/being.h` (lines 2069-2076)

### plotValue() Implementation

```cpp
template <typename T, typename V>
V plotValue(T value, const T lowerBound, const T upperBound,
            const V minValue, const V maxValue, const V average,
            const double power = 1.4) {

    const T midline = ((upperBound - lowerBound) / 2.0) + lowerBound;  // = 105
    value = min(max(value, lowerBound), upperBound);  // Clamp to 5-205

    if (value >= midline) {
        // Upper half (105-205): curve from average to maxValue
        double A = (maxValue - average) / (pow(205, power) - pow(105, power));
        double B = average - pow(105, power) * A;
        return A * pow(value, power) + B;
    } else {
        // Lower half (5-105): curve from minValue to average
        double A = (average - minValue) / (pow(105, power) - pow(5, power));
        double B = minValue - pow(5, power) * A;
        return A * pow(value, power) + B;
    }
}
```

**Source:** `code/code/misc/extern.h` (lines 443-460)

### Curve Behavior

| Stat Value | Result When Using (0.8, 1.25, 1.0) | Interpretation |
|------------|-------------------------------------|----------------|
| 5 | 0.80 | Minimum (-20% penalty) |
| 105 | 1.00 | Average (neutral) |
| 205 | 1.25 | Maximum (+25% bonus) |
| 55 | ~0.88 | Low stat (~-12%) |
| 155 | ~1.13 | High stat (~+13%) |

**Key insights:**
- **Power 1.4** creates non-linear scaling: stat differences amplified near extremes
- **Midpoint (105)** always returns the `average` parameter
- **Lower half (5-105)** curves from `minValue` to `average`
- **Upper half (105-205)** curves from `average` to `maxValue`
- The power law means **stat differences matter more at extremes**

### getStatMod() - Standardized Modifier

```cpp
double TBeing::getStatMod(statTypeT statType, int multiplier) const {
    return ((plotStat(STAT_CURRENT, statType, 0.8, 1.25, 1.0) - 1) * multiplier) + 1;
}
```

**Formula logic:**
1. Get plotStat result (e.g., 1.1 for +10% bonus)
2. Extract deviation from 1.0: `1.1 - 1 = 0.1`
3. Multiply by multiplier: `0.1 * 2 = 0.2`
4. Add back baseline: `0.2 + 1 = 1.2`

**Example:** If `plotStat()` returns 1.1 (+10%) with multiplier 2:
- Result = ((1.1 - 1) * 2) + 1 = **1.2** (+20% total)

**Source:** `code/code/misc/stats.cc` (lines 1055-1058)

## Stat Effects on Gameplay

### Physical Stats

#### Strength (STR)

```cpp
// Damage modifier (0.8x to 1.25x)
float getStrDamModifier() const {
    return plotStat(STAT_CURRENT, STAT_STR, 0.8, 1.25, 1.0, 1.0);
}
```

**Effects:**
- Melee weapon damage multiplier
- Carrying capacity limits
- Some skills grant STR bonus: `SKILL_IRON_MUSCLES` (+1 STR per 8 learning)

#### Constitution (CON)

```cpp
// HP regeneration modifier
float getConHealthModifier() const {
    return plotStat(STAT_CURRENT, STAT_CON, 0.8, 1.25, 1.0);
}
```

**Effects:**
- Maximum HP pool
- HP regeneration rate (0.8x to 1.25x)

#### Dexterity (DEX)

```cpp
// Combat accuracy bonus (−67 to +84)
int getDexReaction() const {
    return (int)(335 * getStatMod(STAT_DEX) - 335);
}
```

**Effects:**
- Attack accuracy: `attackRound()` adds DEX modifier
- Defense: `defendRound()` adds DEX modifier
- Hit rate: Each point ≈ 0.18% hit probability change

**Example values:**

| DEX | getStatMod(DEX) | getDexReaction() | Hit Rate Effect |
|-----|-----------------|------------------|-----------------|
| 20 (very low) | 0.80 | -67 | -12% hit rate |
| 105 (average) | 1.00 | 0 | Baseline |
| 190 (high) | 1.25 | +84 | +15% hit rate |

**Source:** See [Combat Formulas](combat-formulas.md) for complete hit/defense calculations

#### Agility (AGI)

```cpp
// Reaction bonus (−2 to +4)
int getAgiReaction() const {
    return (int)(plotStat(STAT_CURRENT, STAT_AGI, 0.8, 1.2, 1.0) * 8 - 8);
}
```

**Effects:**
- Combat reactions
- Movement/positioning

#### Speed (SPE)

**Effects:**
- Attack frequency multiplier
- Movement speed
- Urban terrain bonus (+20)

### Mental Stats

#### Intelligence (INT)

```cpp
// Practice point efficiency modifier
float getIntModForPracs() const {
    return plotStat(STAT_NATURAL, STAT_INT, 0.8, 1.25, 1.0);
}
```

**Effects:**
- Spell learning rate
- Mana regeneration
- Spell success probability

#### Wisdom (WIS)

```cpp
// Spell damage modifier
float getWisDamModifier() const {
    return plotStat(STAT_CURRENT, STAT_WIS, 0.8, 1.25, 1.0, 1.0);
}
```

**Effects:**
- Spell damage multiplier (0.8x to 1.25x)
- Spell resistance
- Perception checks

#### Focus (FOC)

**Effects:**
- Spell concentration (resistance to interruption)
- Hate strength duration: `(level1 + level2 + 5) * (FOC / 120.0)` game hours

#### Perception (PER)

**Effects:**
- Awareness of hidden objects
- Trap detection
- Wisdom-adjacent secondary perception stat

### Social and Intangible Stats

#### Charisma (CHA)

```cpp
// Shop price penalty (inverted: high CHA = less penalty)
float getChaShopPenalty() const {
    return plotStat(STAT_CURRENT, STAT_CHA, 1.3, 1.1, 1.15);
    // Low CHA (5) → 1.3 → pay 30% MORE
    // Avg CHA (105) → 1.15 → pay 15% MORE
    // High CHA (205) → 1.1 → pay 10% MORE
}
```

**Note:** Even maximum CHA results in 10% markup. The system is designed so no one gets baseline prices.

**Effects:**
- NPC shop prices (always a markup, but CHA reduces it)
- NPC reactions
- Leadership/social skills

#### Karma (KAR)

```cpp
// Critical hit base chance
int karmaBase = 1000 * plotStat(STAT_CURRENT, STAT_KAR, 0.5, 2.0, 1.0);
// Low KAR (5) → 500 (0.5% crit base)
// Avg KAR (105) → 1000 (1% crit base)
// High KAR (205) → 2000 (2% crit base)
```

**Effects:**
- Critical hit probability scaling
- Moral standing representation
- Random event outcomes

**Source:** See [Combat Formulas](combat-formulas.md) for complete crit calculation

#### Luck (LUC)

**Effects:**
- Critical event outcomes
- Random chance modifiers

## Racial Modifiers

### Territory Adjustment System

Each race-territory combination provides **stat adjustments** when born in specific environments. The `territory_adjustment()` function returns modifiers for homeland bonuses.

**Source:** `code/code/misc/stats.cc` (lines 146-357)

### Territory Stat Modifiers

| Territory | STR | BRA | CON | DEX | AGI | INT | WIS | FOC | PER | CHA | KAR | SPE |
|-----------|-----|-----|-----|-----|-----|-----|-----|-----|-----|-----|-----|-----|
| **Urban** | - | - | -20 | - | - | +20 | +20 | -10 | - | +20 | -20 | +20 |
| **Villager** | - | -10 | -10 | - | - | +10 | +10 | - | - | +10 | -10 | - |
| **Plains** | - | +5 | +5 | - | - | -10 | 0 | -10 | +15 | -5 | +5 | -5 |
| **Recluse** | - | +15 | +25 | - | - | -25 | -15 | +15 | -15 | -30 | +30 | 0 |
| **Hill** | - | +10 | +10 | - | - | -15 | -5 | -15 | +10 | -10 | +10 | 0 |
| **Mountain** | - | +15 | +20 | - | - | -20 | -15 | -15 | +10 | -20 | +20 | 0 |
| **Forest** | - | +15 | +15 | - | - | -15 | -15 | -15 | +10 | -15 | +15 | 0 |
| **Mariner** | - | +5 | +5 | - | - | -5 | -5 | -5 | +5 | -5 | +5 | 0 |

**Key patterns:**
- **Urban:** High mental stats (INT, WIS, CHA, SPE); low physical (CON, KAR)
- **Recluse:** High durability (CON, BRA, KAR); very low social (CHA -30)
- **Mountain/Forest:** Outdoor bonuses to KAR, CON, BRA; penalties to INT, CHA
- **Plains:** Balanced outdoor, high PER

### Age Modifiers

Physical stats change with age, peaking in youth and declining in old age. Mental stats remain relatively stable.

**Aging curve (`age_mod_for_stat()`):**

| Age | STR | BRA | DEX | AGI | CON | INT | WIS | FOC | PER | CHA |
|-----|-----|-----|-----|-----|-----|-----|-----|-----|-----|-----|
| 16 | +10 | +5 | 0 | +10 | 0 | -5 | -5 | 0 | 0 | 0 |
| 20 | +10 | +5 | 0 | +10 | 0 | -5 | -5 | 0 | 0 | 0 |
| 25 | +10 | +5 | 0 | +5 | 0 | 0 | 0 | 0 | 0 | 0 |
| 30 | +10 | +3 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0 |
| 40 | +3 | 0 | -2 | -2 | -2 | 0 | 0 | 0 | 0 | 0 |
| 50 | -2 | -2 | -4 | -4 | -3 | -2 | -2 | -2 | -2 | -2 |
| 60 | -5 | -3 | -6 | -6 | -5 | -3 | -3 | -3 | -3 | -3 |
| 70 | -7 | -4 | -7 | -7 | -7 | -4 | -4 | -4 | -4 | -4 |
| 80 | -10 | -5 | -8 | -8 | -10 | -5 | -5 | -5 | -5 | -5 |

**Peak ages:**
- STR: 16-30 (+10), declines to -10 at 80
- AGI: 16-20 (+10), declines to -8 at 80
- BRA: 16-25 (+5), declines to -5 at 80
- Mental stats: Stable until 50, then small decline

**Vampire exception:** Vampires ignore all age modifiers and receive +25 to STR, SPE, and CHA instead.

**Source:** `code/code/misc/stats.cc` (lines 359-536)

## Special Stat Mechanics

### Stat-Based Checks

The `statSelfCheck()` method converts a stat into a percentage-based success probability:

```cpp
bool TBeing::statSelfCheck(statTypeT stat, int num) const {
    return percentChance(plotStat(STAT_CURRENT, stat, 5, 95, 25) + num);
}
```

**Conversion:**
- Low stat (5) → 5% success chance
- Average stat (105) → 25% success chance
- High stat (205) → 95% success chance
- `num` parameter adds flat bonus/penalty

**Usage:**
```cpp
if (statSelfCheck(STAT_PER, 10)) {
    // Perception check with +10 bonus
    // High PER character has 95%+ chance
}
```

**Source:** `code/code/misc/stats.cc` (lines 565-568)

### Convenience Check Methods

```cpp
bool isStrong() const;          // STR check (25% baseline)
bool isPerceptive() const;      // PER check
bool isAgile(int num) const;    // AGI check with bonus
bool isDextrous() const;        // DEX check
bool isTough() const;           // CON check
bool isBrawny() const;          // BRA check
bool isIntelligent() const;     // INT check
bool isWise() const;            // WIS check
bool isFast() const;            // SPE check
bool isFocused() const;         // FOC check
bool isCharismatic() const;     // CHA check
bool isLucky() const;           // KAR check
```

**Source:** `code/code/misc/stats.cc` (lines 570-618)

## Equipment and Spell Effects

### Affect Application

Equipment and spell affects modify `STAT_CURRENT` via `affectModify()`:

```cpp
void TBeing::affectModify(statTypeT loc, int mod, long bitv, bool add) {
    if (add)
        curStats.add(loc, mod);  // Add bonus
    else
        curStats.add(loc, -mod); // Remove bonus

    // Also applies bitvector flags (AFF_*)
    if (bitv)
        setAffFlags(bitv, add);
}
```

**Workflow:**
1. Character equips item with `APPLY_STR +5`
2. `equipChar()` calls `affectModify(STAT_STR, 5, 0, true)`
3. `curStats` increases by 5
4. `getStat(STAT_CURRENT, STAT_STR)` now returns higher value
5. All STR-dependent calculations automatically use new value

**Source:** `code/code/sys/handler.cc` (lines 2072-2137)

### Temporary vs Permanent

| Layer | Persistence | Changes How |
|-------|-------------|-------------|
| `STAT_RACE` | Permanent | Never changes (fixed at creation) |
| `STAT_CHOSEN` | Permanent | Only at character creation |
| `STAT_TERRITORY` | Permanent | Only at character creation |
| `STAT_AGE` | Dynamic | Changes with in-game aging |
| `STAT_NATURAL` | Dynamic | Recalculated when components change |
| `STAT_CURRENT` | Temporary | Changes with equipment/spell affects |

**Example flow:**
```cpp
// Permanent: Racial Ogre STR = 120
int baseSTR = getStat(STAT_RACE, STAT_STR);  // 120

// Permanent: Player allocated +10 at creation
int chosenBonus = getStat(STAT_CHOSEN, STAT_STR);  // 10

// Dynamic: Age 25, territory mountain
int ageBonus = getStat(STAT_AGE, STAT_STR);        // +10
int territoryBonus = getStat(STAT_TERRITORY, STAT_STR); // 0

// Natural = 120 + 10 + 10 + 0 = 140
int naturalSTR = getStat(STAT_NATURAL, STAT_STR);  // 140

// Temporary: Equipped +5 STR gauntlets, +3 STR belt
// Current = 140 + 5 + 3 = 148
int currentSTR = getStat(STAT_CURRENT, STAT_STR);  // 148
```

---

# IV. Integration

The three systems (classes, races, stats) integrate deeply to create character capabilities:

## Class-Race-Stat Interaction

### Character Creation Flow

1. **Race Selection** → Base stats determined
2. **Class Selection** → Abilities and resources determined
3. **Stat Customization** → Player allocates `chosenStats` points
4. **Territory Selection** → Homeland bonuses applied
5. **Final Calculation** → All layers combined

```cpp
// Example: Dwarf Warrior created in Mountain territory
Race* race = Races[RACE_DWARF];
int raceSTR = race->baseStats.get(STAT_STR);       // 130

chosenStats.set(STAT_STR, 10);                     // +10 player allocation
int territorySTR = territory_adjustment(MOUNTAIN, STAT_STR); // 0
int ageSTR = age_mod_for_stat(this, 20, STAT_STR); // +10 (age 20)

int naturalSTR = raceSTR + 10 + 0 + 10;            // = 150
```

### Class Resources and Stats

Class resources scale with race stats:

**Mana (Mages):**
- Base from class (varies by class HP formula)
- Multiplied by INT modifier via `plotStat()`
- Modified by racial `manaMod`

```cpp
// Elven Mage (INT 130, manaMod +1)
int baseMana = classBaseMana;
baseMana *= plotStat(STAT_NATURAL, STAT_INT, 0.8, 1.25, 1.0);
baseMana += level * race->manaMod;  // +1 per level
```

**Piety (Clerics):**
- Scales with WIS
- Modified by class calculations

**Lifeforce (Shamans):**
- Scales with character level
- Not directly stat-modified

**HP (All):**
- Base from class (varies: Warrior 8.5, Mage 5.25)
- Multiplied by CON modifier
- Modified by racial `hpMod`

```cpp
// Dwarf Warrior (CON 155, hpMod +1)
int baseHP = classInfo[WARRIOR_LEVEL_IND].hp_per_level;  // 8.5
baseHP *= plotStat(STAT_NATURAL, STAT_CON, 0.8, 1.25, 1.0);
baseHP += race->hpMod;  // +1
```

### Racial Class Restrictions

Some races cannot choose certain classes:

**Example restrictions:**
- Ogres cannot be mages (INT 80 too low)
- Certain races restricted from monk (body type requirements)
- Deikhan class restricted to lawful races

These are enforced during character creation by checking racial stats and characteristics.

## Multiclass Stat Scaling

Multiclass characters face trade-offs:

**XP Penalty:**
- 2 classes: 25% XP per class (quadratic)
- 3 classes: 11% XP per class

**Resource Distribution:**
- Mana pool: Depends on mage levels + INT
- Piety pool: Depends on cleric levels + WIS
- HP pool: Weighted average across classes

**Example: Mage/Cleric multiclass**
```cpp
// Level 20 Mage, Level 15 Cleric
int mageHP = 20 * 5.25;  // = 105
int clericHP = 15 * 5.6; // = 84
int totalHP = mageHP + clericHP;  // Both contribute

// Modified by CON
totalHP *= plotStat(STAT_CURRENT, STAT_CON, 0.8, 1.25, 1.0);
```

## Combat Integration

Combat effectiveness combines all three systems:

**Attack Accuracy:**
- Class: Warrior bonuses, fighter combat modes
- Race: DEX baseline (Elven 130, Dwarf 85)
- Stats: `getDexReaction()` adds −67 to +84

**Damage:**
- Class: Weapon specialization, class skills
- Race: STR baseline (Ogre 165, Hobbit 55)
- Stats: `getStrDamModifier()` multiplies 0.8x to 1.25x

**Defense:**
- Class: Armor proficiency, defensive skills
- Race: Racial immunities (Dwarf 40% poison)
- Stats: AGI/DEX modifiers

**Example Combat Calculation:**
```cpp
// Dwarf Warrior attacking
int baseAttackBonus = 100;  // From level/class

// Class modifier (combat mode)
if (isCombatMode(ATTACK_OFFENSE))
    baseAttackBonus += level / 4;

// Stat modifier (DEX = racial 85 + equipment)
baseAttackBonus += getDexReaction();

// Racial modifiers (none for attack, but affects defense)
int damageReduction = race->getImmunity(IMMUNE_PIERCE);
```

## Spell Effectiveness

Spell power combines class, race, and stats:

**Spell Damage:**
- Class: Mage/Cleric/Shaman specialization
- Race: WIS baseline affects damage multiplier
- Stats: `getWisDamModifier()` scales 0.8x to 1.25x

**Spell Success:**
- Class: Discipline learnedness
- Race: INT baseline affects learning
- Stats: INT affects practice point efficiency

**Resource Costs:**
- Class: Mana/Piety/Lifeforce costs defined per spell
- Race: Regeneration rate varies by race
- Stats: INT (mana regen), WIS (piety regen)

## Physical Characteristics

Body types and racial characteristics interact:

**Equipment Slots:**
- Race defines body type (BODY_HUMANOID, BODY_DRAGON, etc.)
- Body type determines available equipment slots
- Class may have restrictions (monks can't wear armor)

**Movement:**
- Race: Movement modifiers, speed stats
- Class: Some classes have movement abilities
- Stats: SPE affects movement speed

**Example:**
```cpp
// Hobbit Thief
// Race: BODY_HUMANOID, moveMod +40, SPE 155
// Class: Thief (stealth bonuses)
// Result: Extremely fast, stealthy character
```

## Common Patterns

### Creating a Character

```cpp
// 1. Select race
ch->race = Races[RACE_DWARF];

// 2. Set classes
ch->player.Class = CLASS_WARRIOR;

// 3. Initialize levels
ch->startLevels();  // Sets level 1 in each class

// 4. Allocate chosen stats
ch->chosenStats.set(STAT_STR, 10);  // Extra STR

// 5. Set territory
ch->player.hometerrain = MOUNTAIN;

// 6. Calculate natural stats
int naturalSTR = ch->getStat(STAT_NATURAL, STAT_STR);
// = 130 (race) + 10 (chosen) + 0 (territory) + 10 (age 20) = 150
```

### Checking Capabilities

```cpp
// Can character cast spells?
if (ch->hasClass(CLASS_MAGE) && ch->getMana() >= 30) {
    // Has mage class and sufficient mana
}

// Can character use heavy armor?
if (ch->hasClass(CLASS_WARRIOR) && !ch->hasClass(CLASS_MONK)) {
    // Warrior, not monk → can wear plate
}

// Racial flight capability?
if (ch->getMyRace()->isWinged()) {
    // Can fly naturally
}
```

### Resource Management

```cpp
// Mage spell cost
if (ch->hasClass(CLASS_MAGE)) {
    int manaCost = 30;
    if (ch->getMana() >= manaCost) {
        ch->addToMana(-manaCost);
        // Cast spell
    }
}

// HP regeneration (CON-based)
float regenMod = ch->getConHealthModifier();  // 0.8 to 1.25
int hpGain = baseRegen * regenMod;
```

## Key Design Decisions

1. **Separation of Concerns:**
   - Classes provide abilities (what you can do)
   - Races provide base stats (what you are)
   - Stats determine effectiveness (how well you do it)

2. **Bitmask Multiclassing:**
   - Flexible class combinations
   - Independent level tracking
   - Quadratic XP penalty discourages excessive multiclassing

3. **Non-Linear Stat Scaling:**
   - Power 1.4 amplifies stat differences
   - High stats significantly more valuable than low stats
   - Encourages specialization over generalization

4. **Racial Flyweight Pattern:**
   - Single shared Race instance per race type
   - Memory efficient with 1000+ beings in world
   - Race data loaded from text files at boot

5. **Layered Stat System:**
   - Permanent layers (race, chosen, territory)
   - Dynamic layers (age, equipment, spells)
   - Clear separation of temporary vs permanent bonuses

6. **Resource Diversity:**
   - Mana (INT-based, mages)
   - Piety (WIS-based, clerics, DOUBLE precision)
   - Lifeforce (level-based, shamans)
   - Different resource types prevent homogenization

7. **Racial Immunities Stack:**
   - Racial + equipment + spell resistances additive
   - Capped at 100%
   - Negative values = vulnerability

8. **Age Affects All:**
   - Realistic aging curve
   - Physical stats peak 16-30, decline after 50
   - Mental stats more stable
   - Vampire exception: no aging, +25 STR/SPE/CHA

## Source Files Reference

| File | Purpose | Lines |
|------|---------|-------|
| `code/code/misc/being.h` | TBeing class, resource accessors, level methods | - |
| `code/code/misc/being.cc` | TBeing implementations, resource management | - |
| `code/code/misc/multiclass.cc` | Class query/conversion, multiclass checks | - |
| `code/code/misc/defs.h` | Class constants (CLASS_*, MAX_MORT, GOD_LEVEL1) | - |
| `code/code/misc/enum.h` | classIndT, statTypeT enums | - |
| `code/code/misc/constants.cc` | classInfo[] global array | - |
| `code/code/misc/discipline.h` | class_info struct definition | - |
| `code/code/misc/gaining.cc` | XP gain, multiclass penalties | - |
| `code/code/misc/limits.cc` | Resource regeneration, practice points | - |
| `code/code/misc/mobact.cc` | Class-specific combat AI | - |
| `code/code/misc/race.h` | Race class, race_t enum, lore_t, talent flags | - |
| `code/code/misc/race.cc` | Race implementation, initRaces() | - |
| `code/code/misc/racedata.cc` | Race data file parsing | - |
| `code/code/misc/stats.h` | Stats class, statTypeT enum, statSetT enum | - |
| `code/code/misc/stats.cc` | getStat(), territory_adjustment(), age_mod_for_stat() | - |
| `code/code/misc/extern.h` | plotValue() template implementation | - |
| `code/code/sys/handler.cc` | affectModify() for equipment/spell affects | - |
| `lib/races/` | Race definition files (127 total) | - |

## Related Documentation

- [Class Hierarchy](class-hierarchy.md) - Detailed class inheritance and discipline trees
- [Discipline System](discipline-system.md) - Spell/skill learning by class
- [Experience and Leveling](experience-leveling.md) - XP gain and advancement
- [Combat Formulas](combat-formulas.md) - How stats affect combat
- [Equipment and Wear](equipment-wear.md) - Equipment affects and stat bonuses
