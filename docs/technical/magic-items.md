---
title: Magical Items and Enchantment System
description: The magical item system provides scrolls, wands, and staves that store and cast spells, bridging the object system and spell system to allow non-casters to use magic.
keywords:
  - TMagicItem
  - TScroll
  - TWand
  - TStaff
  - reciteMe
  - useMe
  - doObjSpell
  - magic_level
  - spellNumT
  - affectedData
  - equipChar
  - DELETE_VICT
  - chargeManagement
  - scrollCasting
  - wandUsage
category: Important Systems

  - object-types.md
  - spell-definitions.md
  - command-implementation.md
  - affects-system.md
last_updated: 2026-01-29
source_files:
  - code/code/obj/obj_magic_item.h
  - code/code/obj/obj_magic_item.cc
  - code/code/obj/obj_scroll.h
  - code/code/obj/obj_scroll.cc
  - code/code/obj/obj_wand.h
  - code/code/obj/obj_wand.cc
  - code/code/obj/obj_staff.h
  - code/code/obj/obj_staff.cc
  - code/code/misc/other.cc
  - code/code/misc/structs.h
  - code/code/misc/obj.h
  - code/code/sys/handler.cc
  - code/code/misc/spells.h
  - code/code/misc/spell2.h
  - code/code/misc/enum.h
  - code/code/misc/defs.h
related: [memory-safety.md]
---

# Magical Items and Enchantment System

The magical item system provides scrolls, wands, and staves that store and cast spells. These items bridge the object system and spell system, allowing non-casters to use magic and providing spell storage for adventurers.

**Misusing this system causes crashes.** Common errors: not checking DELETE flags from spell execution, continuing to use deleted victims, failing to validate charge counts, improper iterator handling in area spells.

## Overview

Magical items store spells in object form, providing an alternative to direct spellcasting. Three types exist:

| Type | Class | Charges | Targeting | Usage Pattern |
|------|-------|---------|-----------|---------------|
| Scroll | `TScroll` | Single-use | Manual | Consumed on recitation, casts 3 spells |
| Wand | `TWand` | Rechargeable | Manual | Player-specified target, single spell |
| Staff | `TStaff` | Rechargeable | Automatic | Area or room-wide, single spell |

**Key characteristics:**
- Magic items don't consume mana/piety/lifeforce (use charges instead)
- Spell lag still applies from `discArray[spell]->lag`
- Items can be enchanted with permanent stat bonuses via affects
- All spell execution goes through `doObjSpell()` dispatcher
- DELETE flags must be checked and propagated for safe spell handling

## Class Hierarchy

```
TObj (base class for all objects)
  |
  +-- TMagicItem (abstract base)
        |
        +-- TScroll (single-use consumables)
        |
        +-- TWand (rechargeable single-target)
        |
        +-- TStaff (rechargeable area-effect)
```

**Source:** `code/code/obj/obj_magic_item.h`, `code/code/obj/obj_scroll.h`, `code/code/obj/obj_wand.h`, `code/code/obj/obj_staff.h`

### TMagicItem Base Class

```cpp
class TMagicItem : public virtual TObj {
  private:
    int magic_level;           // Enchantment strength (0-250+)
    int magic_learnedness;     // Crafting quality (0-100)
  public:
    virtual void descMagicSpells(TBeing*) const = 0;
    virtual sstring getNameForShow(bool, bool, const TBeing*) const = 0;
    virtual void divinateMe(TBeing*) const = 0;
    virtual int suggestedPrice() const = 0;
    virtual sstring statObjInfo() const = 0;
};
```

**Key members:**
- `magic_level`: Determines spell power/effectiveness (higher = stronger)
- `magic_learnedness`: Crafting quality affecting reliability (0-100 scale)

**Pure virtual interface:**
- `descMagicSpells()`: Display stored spells to player
- `divinateMe()`: Reveal item properties via divination magic
- `suggestedPrice()`: Calculate base market value
- `statObjInfo()`: Generate immortal stat display

### TScroll Class

```cpp
class TScroll : public TMagicItem {
  private:
    spellNumT spells[3];  // Three spell slots per scroll
  public:
    int reciteMe(TBeing*, const char*);  // Core usage function
    spellNumT getSpell(int index) const;
    void setSpell(int index, spellNumT spell);
};
```

**Characteristics:**
- Single-use consumable (always destroyed after recitation)
- Stores up to 3 spells in `spells[]` array
- Consumed even on failed recitation
- Requires `SKILL_READ_MAGIC` skill check

### TWand Class

```cpp
class TWand : public TMagicItem {
  private:
    int maxCharges;        // Maximum charges when fully charged
    int curCharges;        // Current charges remaining
    spellNumT spell;       // Single spell stored in wand
  public:
    int useMe(TBeing*, const char*);  // Core usage function
    void addToCurCharges(int n);      // Supports negative for consumption
    int getCurCharges() const;
    int getMaxCharges() const;
};
```

**Characteristics:**
- Rechargeable (can be restored to `maxCharges`)
- Single spell per wand
- Requires player-specified targeting
- Consumes 1 charge per use
- Cannot be sold unless at full charge

### TStaff Class

```cpp
class TStaff : public TMagicItem {
  private:
    int maxCharges;
    int curCharges;
    spellNumT spell;
  public:
    int useMe(TBeing*, const char*);
    int taskChargeMe(TBeing*, int, TRoom*);      // Channeled spells
    void taskChargeStart(TBeing*, spellNumT, int);
};
```

**Characteristics:**
- Rechargeable like wands
- Single spell per staff
- Automatic targeting (area or room-wide)
- Skips caster and group members for violent spells
- Supports channeled/tasked spell casting

## Value Field Mappings

Objects store type-specific data in four integer fields (`val0`-`val3`) via `assignFourValues()` and `getFourValues()`.

### Base TMagicItem Bit-Packing

`val1` stores magic_level and magic_learnedness:

| Bits | Field | Range | Description |
|------|-------|-------|-------------|
| 0-7 | `magic_learnedness` | 0-100 | Crafting quality |
| 8-15 | `magic_level` | 0-250+ | Enchantment strength |

**Source:** `code/code/obj/obj_magic_item.cc`

### TScroll Value Fields

| Field | Contains | Description |
|-------|----------|-------------|
| `val0` | (unused) | Reserved |
| `val1` | Magic level/learnedness | Packed as above |
| `val2` | Spell slot 0 | File-format spell ID |
| `val3` | Spell slot 1 | File-format spell ID |
| `val4` | Spell slot 2 | File-format spell ID |

**Conversion:** `mapFileToSpellnum()` converts database values to `spellNumT`

**Implementation:**
```cpp
void TScroll::assignFourValues(int x1, int x2, int x3, int x4) {
  TMagicItem::assignFourValues(x1, x2, x3, x4);
  setSpell(0, mapFileToSpellnum(x2));  // val2 -> spell slot 0
  setSpell(1, mapFileToSpellnum(x3));  // val3 -> spell slot 1
  setSpell(2, mapFileToSpellnum(x4));  // val4 -> spell slot 2
}
```

**Source:** `code/code/obj/obj_scroll.cc`

### TWand/TStaff Value Fields

| Field | Contains | Description |
|-------|----------|-------------|
| `val0` | (unused) | Reserved |
| `val1` | Magic level/learnedness | Packed as above |
| `val2` | `maxCharges` | Maximum charges |
| `val3` | `curCharges` | Current charges remaining |
| `val4` | Spell ID | Single stored spell |

**Implementation (TWand):**
```cpp
void TWand::assignFourValues(int x1, int x2, int x3, int x4) {
  TMagicItem::assignFourValues(x1, x2, x3, x4);
  setMaxCharges(x2);
  addToCurCharges(x3);
  setSpell(mapFileToSpellnum(x4));
}
```

**Source:** `code/code/obj/obj_wand.cc`

## Spell Storage System

### spellNumT Enumeration

Spells are identified by the `spellNumT` enum:

```cpp
enum spellNumT {
  TYPE_UNDEFINED = -1,
  SPELL_GUST = 0,         // MIN_SPELL
  // ... hundreds of spells ...
  SKILL_SLAM,             // MAX_SPELL (first skill)
  // ... skills ...
  MAX_SKILL               // End marker
};
```

**Constants:**
- `MIN_SPELL = SPELL_GUST` (first valid spell)
- `MAX_SPELL = SKILL_SLAM` (last spell before skills)
- `MAX_SKILL` (end of valid range)

**Source:** `code/code/misc/spells.h`

### Database Conversion

Spell IDs in database use a different numbering than `spellNumT`:

```cpp
spellNumT mapFileToSpellnum(int file_val);  // Database -> spellNumT
int mapSpellnumToFile(spellNumT spell);     // spellNumT -> Database
```

**Critical:** Always use these converters when loading/saving magic items to database.

### Spell Validation

```cpp
bool isValidSpell(spellNumT spell) {
  return (spell >= MIN_SPELL && spell < MAX_SKILL && discArray[spell]);
}
```

The `discArray` global array (indexed by `spellNumT`) contains `spellInfo` pointers for all valid spells. NULL entries indicate invalid spell IDs.

## Spell Activation System

### Scroll Recitation: reciteMe()

**Source:** `code/code/misc/other.cc`

```cpp
int TScroll::reciteMe(TBeing* ch, const char* argument) {
  // Skill check
  if (!ch->bSuccess(SKILL_READ_MAGIC)) {
    ch->sendTo("You flub the words and the spell does not fire.\n\r");
    return DELETE_THIS;  // Consumed even on failure
  }

  // Find targeting
  TBeing* victim;
  TObj* obj;
  generic_find(argument, FIND_CHAR_ROOM | FIND_OBJ_INV | FIND_OBJ_ROOM,
               ch, &victim, &obj);

  lag_t max_lag = LAG_0;

  // Cast all three spells sequentially
  for (int i = 0; i < 3; i++) {
    spellNumT the_spell = getSpell(i);
    if (the_spell < MIN_SPELL || !discArray[the_spell])
      continue;

    // Track maximum lag
    if (max_lag < discArray[the_spell]->lag)
      max_lag = discArray[the_spell]->lag;

    // Prevent mid-spell deletion
    setLocked(true);
    int rc = doObjSpell(ch, victim, this, obj, argument, the_spell);
    setLocked(false);

    // Handle victim death
    if (IS_SET_DELETE(rc, DELETE_VICT) && victim != ch) {
      delete victim;
      victim = NULL;
      break;  // Stop casting if victim dies
    }
  }

  // Apply spell lag
  ch->addToWait(combatRound(max_lag + 2));

  return DELETE_THIS;  // Always consumed
}
```

**Key behaviors:**
1. **Skill check:** `bSuccess(SKILL_READ_MAGIC)` determines success
2. **Always consumed:** Returns `DELETE_THIS` even on failure
3. **Sequential casting:** All 3 spells fire in order
4. **Victim death check:** Stops after spell kills victim
5. **Lag accumulation:** Uses maximum lag from all spells cast
6. **Lock protection:** `setLocked(true)` prevents scroll deletion during spell execution

### Wand Usage: useMe()

**Source:** `code/code/obj/obj_wand.cc`

```cpp
int TWand::useMe(TBeing* ch, const char* argument) {
  spellNumT the_spell = getSpell();

  // Validate targeting requirements
  if (IS_SET(discArray[the_spell]->targets, TAR_IGNORE)) {
    ch->sendTo("That wand's spell cannot be targeted.\n\r");
    return FALSE;
  }

  // Check charges
  if (getCurCharges() <= 0) {
    ch->sendTo("The wand is depleted.\n\r");
    return FALSE;
  }

  // Build targeting bitmask from spell's TAR_* flags
  int bv = 0;
  if (IS_SET(discArray[the_spell]->targets, TAR_CHAR_ROOM))
    bv |= FIND_CHAR_ROOM;
  if (IS_SET(discArray[the_spell]->targets, TAR_OBJ_INV))
    bv |= FIND_OBJ_INV;
  if (IS_SET(discArray[the_spell]->targets, TAR_OBJ_ROOM))
    bv |= FIND_OBJ_ROOM;
  if (IS_SET(discArray[the_spell]->targets, TAR_OBJ_EQUIP))
    bv |= FIND_OBJ_EQUIP;

  // Find target
  TBeing* tmp_char = NULL;
  TObj* o = NULL;
  int bits = generic_find(argument, bv, ch, &tmp_char, &o);

  if (!bits) {
    ch->sendTo("You can't find that target.\n\r");
    return FALSE;
  }

  // Consume charge and cast spell
  addToCurCharges(-1);
  int rc = doObjSpell(ch, tmp_char, this, o, argument, the_spell);

  // Handle DELETE flags
  if (IS_SET_DELETE(rc, DELETE_VICT) && ch != tmp_char) {
    delete tmp_char;
    tmp_char = NULL;
  }
  if (IS_SET_DELETE(rc, DELETE_THIS))
    return DELETE_THIS;

  // Apply spell lag
  ch->addToWait(combatRound(discArray[the_spell]->lag));

  return TRUE;
}
```

**Key behaviors:**
1. **Requires targeting:** Rejects `TAR_IGNORE` spells
2. **Charge validation:** Checks `curCharges > 0` before use
3. **Target finding:** Uses `generic_find()` with spell-specific bitmask
4. **Single charge consumption:** `addToCurCharges(-1)`
5. **DELETE flag handling:** Checks both `DELETE_VICT` and `DELETE_THIS`
6. **Spell lag:** Applied from `discArray[spell]->lag`

### Staff Usage: useMe()

**Source:** `code/code/obj/obj_staff.cc`

```cpp
int TStaff::useMe(TBeing* ch, const char* argument) {
  spellNumT the_spell = getSpell();

  // Check charges
  if (getCurCharges() <= 0) {
    ch->sendTo("The staff has no charges.\n\r");
    return FALSE;
  }

  // Consume charge
  addToCurCharges(-1);

  int rc = FALSE;
  bool isViolent = IS_SET(discArray[the_spell]->targets, TAR_VIOLENT);

  // Branch on area vs single-target
  if (IS_SET(discArray[the_spell]->targets, TAR_AREA)) {
    // Area spell: call once for entire room
    rc = doObjSpell(ch, NULL, this, NULL, argument, the_spell);
  } else {
    // Single-target: iterate through room beings
    for (StuffIter it = ch->roomp->stuff.begin();
         it != ch->roomp->stuff.end();) {
      TThing* t = *(it++);  // Cache iterator before potential deletion
      TBeing* tmp_char = dynamic_cast<TBeing*>(t);

      if (!tmp_char || tmp_char == ch)
        continue;

      // Skip group members for violent spells
      if (isViolent && tmp_char->inGroup(*ch))
        continue;

      rc = doObjSpell(ch, tmp_char, this, NULL, argument, the_spell);

      if (IS_SET_DELETE(rc, DELETE_VICT)) {
        delete tmp_char;
        tmp_char = NULL;
      }
    }
  }

  // Apply spell lag
  ch->addToWait(combatRound(discArray[the_spell]->lag));

  return rc;
}
```

**Key behaviors:**
1. **Automatic targeting:** No player-specified target needed
2. **Area spell detection:** Checks `TAR_AREA` flag
3. **Room iteration:** Loops through `roomp->stuff` for single-target
4. **Group protection:** Skips group members for violent spells
5. **Iterator safety:** Caches `next` pointer before spell execution
6. **DELETE handling:** Deletes victims during iteration

## Charge Management System

### Charge Fields

Both `TWand` and `TStaff` maintain charge counts:

```cpp
private:
  int maxCharges;  // Maximum charges (full)
  int curCharges;  // Current charges remaining
```

### Charge Operations

```cpp
int getCurCharges() const;           // Query current charges
int getMaxCharges() const;           // Query maximum charges
void setMaxCharges(int n);           // Set maximum (rarely changed)
void addToCurCharges(int n);         // Add/subtract charges
```

**Consumption pattern:**
```cpp
if (getCurCharges() > 0) {
  addToCurCharges(-1);  // Negative to consume
  // ... cast spell ...
}
```

### Recharging Mechanics

Wands and staves can be recharged via spells or crafting:

```cpp
// Example recharge implementation
void TWand::rechargeTo(int charges) {
  int new_charges = min(charges, getMaxCharges());
  addToCurCharges(new_charges - getCurCharges());
}
```

**Overcharge risk:** Attempting to exceed `maxCharges` may cause item destruction in some implementations.

### Shop Restrictions

Wands cannot be sold unless fully charged:

```cpp
// In objectSell()
if (obj->itemType() == ITEM_WAND) {
  TWand* wand = dynamic_cast<TWand*>(obj);
  if (wand->getCurCharges() != wand->getMaxCharges()) {
    ch->sendTo("Shopkeepers won't buy partially used wands.\n\r");
    return TRUE;
  }
}
```

**Source:** `code/code/misc/shop.cc`

## Item Affects System

Objects can carry permanent magical affects that modify character stats when equipped.

### affectedData Structure

**Source:** `code/code/misc/structs.h`

```cpp
class affectedData {
  public:
    spellNumT type;           // Affect type (spell/skill ID)
    sbyte level;              // Caster level or intensity
    int duration;             // Remaining ticks (-9 = permanent)
    int renew;                // Duration to become renewable
    long modifier;            // Primary effect value
    long modifier2;           // Secondary effect value
    applyTypeT location;      // Where to apply (APPLY_*)
    uint64_t bitvector;       // Character bits (AFF_*)
    TThing* be;               // Associated being
    affectedData* next;       // Linked list pointer
};
```

### Object Storage

Objects store up to 5 affects:

```cpp
class TObj : public TThing {
  objAffData affected[MAX_OBJ_AFFECT];  // MAX_OBJ_AFFECT = 5
};
```

**Source:** `code/code/misc/obj.h`

### Apply Locations

The `applyTypeT` enum defines where affects modify the character:

| Location | Effect | Modifier Meaning |
|----------|--------|------------------|
| `APPLY_NONE` | No effect | - |
| `APPLY_STR` | Strength bonus | +/- to STR stat |
| `APPLY_INT` | Intelligence bonus | +/- to INT stat |
| `APPLY_WIS` | Wisdom bonus | +/- to WIS stat |
| `APPLY_DEX` | Dexterity bonus | +/- to DEX stat |
| `APPLY_CON` | Constitution bonus | +/- to CON stat |
| `APPLY_HIT` | Max HP bonus | +/- to maximum HP |
| `APPLY_MANA` | Max mana bonus | +/- to maximum mana |
| `APPLY_MOVE` | Max movement bonus | +/- to maximum move |
| `APPLY_HITROLL` | Attack accuracy | +/- to hit rolls |
| `APPLY_DAMROLL` | Damage bonus | +/- to damage rolls |
| `APPLY_ARMOR` | Armor class | +/- to AC (negative is better) |
| `APPLY_IMMUNITY` | Damage resistance | Type in modifier, amount in modifier2 |
| `APPLY_SPELL` | Spell power boost | Spell ID in modifier, bonus in modifier2 |

**Source:** `code/code/misc/enum.h`

### Bitvector Flags

The `bitvector` field sets character flags (AFF_*):

| Flag | Effect |
|------|--------|
| `AFF_INVISIBLE` | Character is invisible |
| `AFF_DETECT_INVISIBLE` | Can see invisible |
| `AFF_SANCTUARY` | Reduced damage taken |
| `AFF_FLYING` | Character can fly |
| `AFF_INFRAVISION` | Can see in dark |
| `AFF_WATERBREATH` | Can breathe underwater |
| `AFF_DETECT_MAGIC` | Can sense magic |
| `AFF_SNEAK` | Moving stealthily |

**Source:** `code/code/misc/defs.h`

## Equipment Affect Application

### equipChar() Function

**Source:** `code/code/sys/handler.cc`

```cpp
void TBeing::equipChar(TThing* obj, wearSlotT pos, silentTypeT silent) {
  // Set equipment pointers
  obj->equippedBy = this;
  obj->eq_pos = pos;
  equipment.wear(obj, pos);

  TObj* to = dynamic_cast<TObj*>(obj);
  if (to && affectShouldApply(to, pos)) {
    // Apply all affects from the object
    for (int j = 0; j < MAX_OBJ_AFFECT; j++) {
      affectModify(to->affected[j].location,
                   to->affected[j].modifier,
                   to->affected[j].modifier2,
                   to->obj_flags.bitvector,
                   TRUE,    // Add/apply
                   silent);
    }
  }
}
```

**Process:**
1. Set `obj->equippedBy` pointer to character
2. Set `obj->eq_pos` to equipment slot
3. Add object to `equipment[]` array
4. For each affect in `affected[]` array:
   - Call `affectModify()` with `TRUE` (apply)
   - Modify character stats based on `location`
   - Add `modifier` value to stat
   - Set bitvector flags

### unequip() Function

**Source:** `code/code/sys/handler.cc`

```cpp
TThing* TBeing::unequip(wearSlotT pos) {
  o = equipment.remove(pos);

  TObj* to = dynamic_cast<TObj*>(o);
  if (to && affectShouldApply(to, pos)) {
    // Remove all affects from the object
    for (j = 0; j < MAX_OBJ_AFFECT; j++) {
      affectModify(to->affected[j].location,
                   to->affected[j].modifier,
                   to->affected[j].modifier2,
                   to->obj_flags.bitvector,
                   FALSE,   // Remove
                   SILENT_NO);
    }
  }

  // Clear equipment pointers
  o->equippedBy = NULL;
  o->eq_pos = WEAR_NOWHERE;

  return o;
}
```

**Process:**
1. Remove object from `equipment[]` array
2. For each affect in `affected[]` array:
   - Call `affectModify()` with `FALSE` (remove)
   - Subtract `modifier` value from stat
   - Clear bitvector flags
3. Clear `obj->equippedBy` pointer
4. Clear `obj->eq_pos` slot
5. Return removed object

### Affect Stacking

Multiple equipped items' affects cumulate:

```cpp
// Example: Multiple strength bonuses
equipment[WEAR_HAND_R] = gauntlets;  // +5 STR
equipment[WEAR_BODY] = chest;        // +3 STR
// Total: +8 STR from equipment
```

Each `affectModify(APPLY_STR, 5, ...)` call adds to the character's current STR stat.

## DELETE Flag Handling

All magic item usage functions must check and propagate DELETE flags from spell execution.

### DELETE Flag Types

| Flag | Meaning | When Set |
|------|---------|----------|
| `DELETE_THIS` | The magic item should be deleted | Item destroyed by spell side effects |
| `DELETE_VICT` | The victim should be deleted | Victim dies from spell damage |
| `DELETE_ITEM` | The target object should be deleted | Object destroyed by spell |

**Source:** `code/code/misc/defs.h`

### Flag Checking Macro

```cpp
IS_SET_DELETE(rc, flag)  // Check if DELETE flag is set
```

**CRITICAL:** Always use `IS_SET_DELETE()`, never `IS_SET()` for DELETE flags. The DELETE flags use a special bit pattern that requires the DELETE-specific macro.

### Scroll DELETE Pattern

```cpp
int TScroll::reciteMe(TBeing* ch, const char* argument) {
  // ... targeting and skill check ...

  for (int i = 0; i < 3; i++) {
    spellNumT the_spell = getSpell(i);

    setLocked(true);  // Prevent scroll deletion during spell
    int rc = doObjSpell(ch, victim, this, obj, argument, the_spell);
    setLocked(false);

    // Check victim death
    if (IS_SET_DELETE(rc, DELETE_VICT) && victim != ch) {
      delete victim;
      victim = NULL;
      break;  // Stop casting remaining spells
    }

    // Check caster death (rare but possible)
    if (IS_SET_DELETE(rc, DELETE_THIS) && ch == victim) {
      return DELETE_THIS;  // Propagate to caller
    }
  }

  return DELETE_THIS;  // Always consumed
}
```

**Key safety patterns:**
1. **Lock protection:** `setLocked(true)` prevents scroll deletion mid-spell
2. **Victim validation:** Check `victim != ch` before deleting
3. **Early exit:** Stop spell loop if victim dies
4. **Flag propagation:** Return `DELETE_THIS` if caster dies

### Wand DELETE Pattern

```cpp
int TWand::useMe(TBeing* ch, const char* argument) {
  // ... targeting and charge check ...

  addToCurCharges(-1);
  int rc = doObjSpell(ch, tmp_char, this, o, argument, the_spell);

  // Handle victim death
  if (IS_SET_DELETE(rc, DELETE_VICT) && ch != tmp_char) {
    delete tmp_char;
    tmp_char = NULL;
  }

  // Handle wand destruction
  if (IS_SET_DELETE(rc, DELETE_THIS))
    return DELETE_THIS;

  // Handle target object destruction
  if (IS_SET_DELETE(rc, DELETE_ITEM) && o) {
    delete o;
    o = NULL;
  }

  ch->addToWait(combatRound(discArray[the_spell]->lag));
  return TRUE;
}
```

**Key safety patterns:**
1. **Check all flags:** `DELETE_VICT`, `DELETE_THIS`, `DELETE_ITEM`
2. **Validate pointers:** Ensure `ch != tmp_char` and `o != NULL`
3. **Propagate deletion:** Return `DELETE_THIS` to caller
4. **NULL after delete:** Set pointers to NULL after deletion

### Staff DELETE Pattern with Iterator Safety

```cpp
int TStaff::useMe(TBeing* ch, const char* argument) {
  addToCurCharges(-1);

  int rc = FALSE;

  if (IS_SET(discArray[the_spell]->targets, TAR_AREA)) {
    rc = doObjSpell(ch, NULL, this, NULL, argument, the_spell);
  } else {
    // CRITICAL: Cache iterator before potential deletion
    for (StuffIter it = ch->roomp->stuff.begin();
         it != ch->roomp->stuff.end();) {
      TThing* t = *(it++);  // Cache BEFORE spell execution
      TBeing* tmp_char = dynamic_cast<TBeing*>(t);

      if (!tmp_char || tmp_char == ch)
        continue;

      if (isViolent && tmp_char->inGroup(*ch))
        continue;

      rc = doObjSpell(ch, tmp_char, this, NULL, argument, the_spell);

      // Safe to delete: iterator already advanced
      if (IS_SET_DELETE(rc, DELETE_VICT)) {
        delete tmp_char;
        tmp_char = NULL;
      }
    }
  }

  ch->addToWait(combatRound(discArray[the_spell]->lag));
  return rc;
}
```

**Key safety patterns:**
1. **Iterator caching:** `TThing* t = *(it++)` advances BEFORE spell execution
2. **Safe deletion:** Can delete after iterator advance
3. **No early exit:** Continue iterating even if victims die
4. **Room-wide application:** All valid targets get hit

## Spell System Integration

### doObjSpell() Dispatcher

**Source:** `code/code/misc/other.cc`

The central spell dispatcher routes spell IDs to their implementations:

```cpp
int doObjSpell(TBeing* ch, TBeing* victim, TObj* obj, TObj* target_obj,
               const char* argument, spellNumT spell) {

  // Peaceful room check
  if (IS_SET(discArray[spell]->targets, TAR_VIOLENT)) {
    if (ch->checkPeaceful("You can't use that here!\n\r"))
      return FALSE;
  }

  // Spell-specific dispatch
  switch (spell) {
    case SPELL_ARMOR:
      return armor(ch, victim, obj, obj->getMagicLevel());
    case SPELL_BLESS:
      return bless(ch, victim, obj, obj->getMagicLevel());
    case SPELL_HEAL:
      return heal(ch, victim, obj, obj->getMagicLevel());
    case SPELL_FIREBALL:
      return castFireball(ch, argument, obj->getMagicLevel(), obj);
    // ... hundreds more spells ...
    default:
      vlogf(LOG_BUG, format("doObjSpell: Unknown spell %d") % spell);
      return FALSE;
  }
}
```

**Key behaviors:**
1. **Peaceful room check:** Blocks `TAR_VIOLENT` spells in `ROOM_PEACEFUL`
2. **Magic level passing:** Uses `obj->getMagicLevel()` for spell power
3. **Switch dispatch:** Routes to specific spell implementation
4. **Return propagation:** Returns spell function's result (including DELETE flags)

### Spell Validation

Before calling `doObjSpell()`, validate the spell:

```cpp
spellNumT the_spell = getSpell();

// Check spell ID range
if (the_spell < MIN_SPELL || the_spell >= MAX_SKILL) {
  vlogf(LOG_BUG, format("Invalid spell ID: %d") % the_spell);
  return FALSE;
}

// Check discArray entry exists
if (!discArray[the_spell]) {
  vlogf(LOG_BUG, format("NULL discArray entry for spell %d") % the_spell);
  return FALSE;
}
```

### Targeting System

The `generic_find()` function locates targets based on spell requirements:

```cpp
int generic_find(const char* argument, int bitvector, TBeing* ch,
                 TBeing** target_being, TObj** target_obj);
```

**Bitmask values:**

| Flag | Searches | Example |
|------|----------|---------|
| `FIND_CHAR_ROOM` | Beings in room | "cast heal gandalf" |
| `FIND_OBJ_INV` | Objects in inventory | "identify sword" |
| `FIND_OBJ_ROOM` | Objects in room | "enchant chest" |
| `FIND_OBJ_EQUIP` | Equipped objects | "repair armor" |

**Usage example:**
```cpp
int bv = 0;
if (IS_SET(discArray[spell]->targets, TAR_CHAR_ROOM))
  bv |= FIND_CHAR_ROOM;
if (IS_SET(discArray[spell]->targets, TAR_OBJ_INV))
  bv |= FIND_OBJ_INV;

TBeing* victim = NULL;
TObj* target = NULL;
int bits = generic_find(argument, bv, ch, &victim, &target);

if (!bits) {
  ch->sendTo("You can't find that target.\n\r");
  return FALSE;
}
```

### Spell Lag Application

All magic item usage applies spell lag from the discipline array:

```cpp
ch->addToWait(combatRound(discArray[spell]->lag));
```

**Lag tiers:**

| Constant | Rounds | Real Time | Typical Use |
|----------|--------|-----------|-------------|
| `LAG_0` | 0 | 0.0s | Instant spells |
| `LAG_1` | 1 | 1.2s | Quick buffs |
| `LAG_2` | 2 | 2.4s | Standard spells |
| `LAG_3` | 3 | 3.6s | Combat spells |
| `LAG_4` | 4 | 4.8s | Powerful effects |
| `LAG_5`+ | 5+ | 6.0s+ | Ultimate spells |

**Source:** `code/code/misc/spell2.h`

## Common Bugs and Crash Prevention

### Bug 1: Not Checking DELETE_VICT

```cpp
// CRASH: Continuing to use deleted victim
int rc = doObjSpell(ch, victim, this, NULL, argument, spell);
victim->sendTo("You feel magic!\n\r");  // victim may be deleted!

// CORRECT: Check and handle deletion
int rc = doObjSpell(ch, victim, this, NULL, argument, spell);
if (IS_SET_DELETE(rc, DELETE_VICT) && victim != ch) {
  delete victim;
  victim = NULL;
  return TRUE;  // Stop processing
}
victim->sendTo("You feel magic!\n\r");  // Safe
```

### Bug 2: Iterator Invalidation

```cpp
// CRASH: Iterator invalidated by deletion
for (StuffIter it = stuff.begin(); it != stuff.end(); ++it) {
  TBeing* victim = dynamic_cast<TBeing*>(*it);
  rc = doObjSpell(ch, victim, this, NULL, "", spell);
  if (IS_SET_DELETE(rc, DELETE_VICT)) {
    delete victim;  // Invalidates it!
    // ++it now crashes
  }
}

// CORRECT: Cache next before deletion
for (StuffIter it = stuff.begin(); it != stuff.end();) {
  TThing* t = *(it++);  // Advance FIRST
  TBeing* victim = dynamic_cast<TBeing*>(t);
  rc = doObjSpell(ch, victim, this, NULL, "", spell);
  if (IS_SET_DELETE(rc, DELETE_VICT)) {
    delete victim;  // Safe: iterator already advanced
    victim = NULL;
  }
}
```

### Bug 3: Forgetting Charge Validation

```cpp
// CRASH: Using depleted wand
addToCurCharges(-1);  // Goes negative!
int rc = doObjSpell(ch, victim, this, NULL, "", spell);

// CORRECT: Check before consumption
if (getCurCharges() <= 0) {
  ch->sendTo("The wand has no charges.\n\r");
  return FALSE;
}
addToCurCharges(-1);
int rc = doObjSpell(ch, victim, this, NULL, "", spell);
```

### Bug 4: Not Propagating DELETE_THIS

```cpp
// BUG: Item destroyed but not signaled
int rc = doObjSpell(ch, victim, this, NULL, "", spell);
if (IS_SET_DELETE(rc, DELETE_THIS)) {
  // Handled locally but forgot to return
}
return TRUE;  // Caller may still reference this item!

// CORRECT: Propagate to caller
int rc = doObjSpell(ch, victim, this, NULL, "", spell);
if (IS_SET_DELETE(rc, DELETE_THIS))
  return DELETE_THIS;  // Let caller handle deletion
return TRUE;
```

### Bug 5: Using IS_SET Instead of IS_SET_DELETE

```cpp
// WRONG: IS_SET won't detect DELETE flags
if (IS_SET(rc, DELETE_VICT)) { ... }  // Never triggers!

// CORRECT: Always use IS_SET_DELETE for DELETE flags
if (IS_SET_DELETE(rc, DELETE_VICT)) { ... }  // Works correctly
```

### Bug 6: Accessing Spell Array Out of Bounds

```cpp
// CRASH: Invalid spell index
TScroll* scroll = ...;
spellNumT spell = scroll->getSpell(5);  // Array only has 3 slots!

// CORRECT: Validate index
for (int i = 0; i < 3; i++) {  // Hard limit: 3 spell slots
  spellNumT spell = scroll->getSpell(i);
  if (spell >= MIN_SPELL && spell < MAX_SKILL && discArray[spell]) {
    // Safe to use spell
  }
}
```

### Bug 7: Forgetting Lock Protection

```cpp
// CRASH: Scroll deleted mid-spell
for (int i = 0; i < 3; i++) {
  rc = doObjSpell(ch, victim, this, NULL, "", getSpell(i));
  // Some spells may trigger scroll destruction
  // Accessing getSpell(i+1) crashes if scroll deleted
}

// CORRECT: Lock before spells
for (int i = 0; i < 3; i++) {
  setLocked(true);
  rc = doObjSpell(ch, victim, this, NULL, "", getSpell(i));
  setLocked(false);
  // Lock prevents deletion during spell execution
}
```

## Key Source Files

| File | Purpose | Key Functions |
|------|---------|---------------|
| `code/code/obj/obj_magic_item.h` | TMagicItem base class | - |
| `code/code/obj/obj_magic_item.cc` | Base implementation | assignFourValues(), getFourValues() |
| `code/code/obj/obj_scroll.h` | TScroll class | - |
| `code/code/obj/obj_scroll.cc` | Scroll implementation | assignFourValues(), getSpell(), setSpell() |
| `code/code/obj/obj_wand.h` | TWand class | - |
| `code/code/obj/obj_wand.cc` | Wand implementation | useMe(), addToCurCharges() |
| `code/code/obj/obj_staff.h` | TStaff class | - |
| `code/code/obj/obj_staff.cc` | Staff implementation | useMe(), taskChargeMe() |
| `code/code/misc/other.cc` | Spell dispatcher | doObjSpell(), reciteMe() |
| `code/code/misc/structs.h` | Data structures | affectedData |
| `code/code/misc/obj.h` | TObj base class | affected[] array, MAX_OBJ_AFFECT |
| `code/code/sys/handler.cc` | Equipment handling | equipChar(), unequip(), affectModify() |
| `code/code/misc/spells.h` | Spell enumeration | spellNumT enum |
| `code/code/misc/spell2.h` | Spell definitions | spellInfo struct, discArray global |
| `code/code/misc/enum.h` | Enumerations | applyTypeT, lagTypeT |
| `code/code/misc/defs.h` | Constants | DELETE_* flags, AFF_* flags |

## Related Documentation

- [DELETE Flag System](delete-flags.md) - Complete DELETE_* flag documentation and propagation patterns
- [Object Types](object-types.md) - Full object type system and value field usage
- [Spell Definitions](spell-definitions.md) - Spell system and discArray structure
- [Command Implementation](command-implementation.md) - DELETE flag handling in commands
- [Affects System](affects-system.md) - Character affect system and affectModify()
- [Equipment Wear](equipment-wear.md) - Equipment slots and wearing mechanics
