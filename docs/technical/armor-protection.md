---
title: Armor and Protection System
description: The armor and protection system manages defensive equipment, armor class calculations, damage mitigation, and equipment durability from class hierarchy through defense calculations.
keywords: [TBaseClothing, TArmor, getArmor, defendRound, APPLY_ARMOR, itemAC, armorLevel, structLevel, galvanizeMe, ITEM_STRUNG, getProtection, SKILL_IRON_FLESH, wearSlotT, armorPercs, isShield, isBarding]
category: Important Systems

last_updated: 2026-01-29
source_files: [code/code/obj/obj_base_clothing.h, code/code/obj/obj_base_clothing.cc, code/code/obj/obj_armor.h, code/code/obj/obj_armor.cc, code/code/obj/obj_armor_wand.cc, code/code/obj/obj_saddle.cc, code/code/obj/obj_harness.cc, code/code/misc/being.cc, code/code/misc/combat.cc, code/code/misc/limbs.h, code/code/obj/obj_low.cc, code/code/disc/disc_advanced_defense.cc]
related:
  - combat-formulas.md
  - combat-rounds.md
  - equipment-wear.md
  - object-system.md
  - character-foundation.md
  - position-stance.md
  - weapon-system.md
  - tohit-defense.md
---

# Armor and Protection System

The armor and protection system in SneezyMUD manages defensive equipment, armor class (AC) calculations, damage mitigation, and equipment durability. This document covers the complete armor mechanics from class hierarchy through defense calculations.

**Misusing this system causes balance issues and combat bugs.** Common errors: forgetting that armor uses APPLY_ARMOR affects (not val0-val3), ignoring paired item penalties, incorrectly calculating Iron Flesh bonuses, and breaking galvanize spell logic.

## Core Concepts

### Armor Class (AC)

Armor Class is the primary defensive stat that determines how hard a character is to hit. **Lower AC values are better** (more protective). AC ranges from ~200 (excellent) to ~800 (poor).

**Key facts:**
- Base AC comes from racial baseline (400-600)
- Equipment provides AC via APPLY_ARMOR affect modifiers
- Monks can gain AC from SKILL_IRON_FLESH when unarmored
- Spells can modify AC temporarily
- Each +25 AC ≈ 1 level of protection

### Structure Points (Durability)

Structure points track equipment durability. Items with low structure are damaged and worth less. Structure affects:
- Item valuation (damaged items sell for less)
- Repair eligibility (some items too damaged to fix)
- Galvanize spell success/failure

**Source:** `code/code/misc/being.cc` (getArmor), `code/code/misc/combat.cc` (defendRound), `code/code/obj/obj_base_clothing.cc`

## Armor Class Hierarchy

### TBaseClothing (Abstract Base)

The base class for all wearable items, providing armor evaluation and pricing.

```cpp
// code/code/obj/obj_base_clothing.h
class TBaseClothing : public TObj {
  public:
    virtual int suggestedPrice() const;
    virtual int armorLevel() const;
    virtual int structLevel() const;
    virtual bool isShield() const;
    virtual bool isBarding() const;

    // Armor evaluation
    int armorPriceStruct(double, const wearSlotT) const;
    double armorPercs(const wearSlotT, bool) const;
};
```

**Key methods:**
- `armorLevel()`: Calculate armor quality level
- `structLevel()`: Calculate structure quality level
- `armorPercs()`: Return AC/structure percentage contribution by slot
- `suggestedPrice()`: Price based on armor value and stat bonuses

**Source:** `code/code/obj/obj_base_clothing.h` (lines 1-76)

### TArmor (Standard Armor)

Primary armor class for protective equipment:

```cpp
// code/code/obj/obj_armor.h
class TArmor : public TBaseClothing {
  public:
    TArmor();
    TArmor(const TArmor& a);

    virtual itemTypeT itemType() const { return ITEM_ARMOR; }
    virtual int galvanizeMe(TBeing*, short);

    // No val0-val3 usage
    virtual void assignFourValues(int, int, int, int) {}
    virtual void getFourValues(int*, int*, int*, int*) const {
        *x1 = *x2 = *x3 = *x4 = 0;
    }
};
```

**Critical:** TArmor does NOT use val0-val3 fields. Armor AC is stored via `affected[]` array with `APPLY_ARMOR` location.

**Source:** `code/code/obj/obj_armor.h` (lines 1-26), `code/code/obj/obj_armor.cc`

### TArmorWand (Hybrid Class)

Combines armor protection with magical wand functionality via multiple inheritance:

```cpp
// code/code/obj/obj_armor_wand.cc
class TArmorWand : public TArmor, public TWand {
  public:
    TArmorWand();
    TArmorWand(const TArmorWand&);

    virtual itemTypeT itemType() const { return ITEM_ARMOR_WAND; }
    virtual int suggestedPrice() const;

    // Overrides both parent copy operators
    TArmorWand& operator=(const TArmorWand&);
};
```

**Pricing formula:**
```cpp
int TArmorWand::suggestedPrice() const {
    return TArmor::suggestedPrice() + TWand::suggestedPrice() - (obj_flags.weight * 10);
}
```

The weight cost is subtracted once to avoid double-counting from both parents.

**Source:** `code/code/obj/obj_armor_wand.cc` (lines 1-78)

### Mount Equipment

**TSaddle:** Riding equipment providing mount control and armor

```cpp
class TSaddle : public TBaseClothing {
    virtual itemTypeT itemType() const { return ITEM_SADDLE; }
    // No val0-val3 usage (returns 0)
};
```

**THarness:** Mount safety equipment

```cpp
class THarness : public TBaseClothing {
    virtual itemTypeT itemType() const { return ITEM_HARNESS; }
    // No val0-val3 usage (returns 0)
};
```

**Source:** `code/code/obj/obj_saddle.cc`, `code/code/obj/obj_harness.cc`

## AC Storage and Calculation

### CRITICAL: Armor Uses APPLY_ARMOR Affects, NOT val0-val3

Unlike weapons, armor does NOT store AC values in val0-val3 fields. Instead:

```cpp
// Armor AC is stored via affected[] array
TObj armor;
armor.affected[0].location = APPLY_ARMOR;
armor.affected[0].modifier = -200;  // 200 points of AC (negative is better)

// Structure points are in TObj base fields
armor.cur_struct = 100;   // Current durability
armor.max_struct = 150;   // Maximum durability
```

**Why this matters:**
- `getFourValues()` returns all zeros for TArmor
- `assignFourValues()` does nothing for TArmor
- AC must be queried via `itemAC()` which sums APPLY_ARMOR modifiers
- When creating armor, add APPLY_ARMOR affects, not val fields

**Source:** `code/code/obj/obj_armor.cc` (lines 27-36)

### Total Armor Calculation (getArmor)

The `TBeing::getArmor()` function calculates total defensive AC:

```cpp
// code/code/misc/being.cc (lines 1151-1196)
int TBeing::getArmor() const {
    int armor = 0;

    // 1. Base racial armor
    armor = getMyRace()->getBaseArmor();  // Typically 400-600

    // 2. Equipment armor (sum APPLY_ARMOR from all equipped items)
    for (wearSlotT slot = MIN_WEAR; slot < MAX_WEAR; slot++) {
        TObj* obj = equipment[slot];
        if (obj && obj->affectShouldApply()) {
            armor += obj->itemAC();  // Sum of APPLY_ARMOR modifiers
        }
    }

    // 3. Iron Flesh armor (monks without equipment)
    if (hasQuestBit(MONK_IRON_FLESH_SKILL)) {
        for (wearSlotT slot = MIN_WEAR; slot < MAX_WEAR; slot++) {
            if (!equipment[slot]) {  // Empty slot
                armor += getIronFleshArmor(slot) * getSkillValue(SKILL_IRON_FLESH) / 100.0;
            }
            // Penalty: If paired item worn, remove Iron Flesh from secondary slot
            else if (equipment[slot]->isPaired()) {
                wearSlotT secondary = getSecondarySlot(slot);
                if (!equipment[secondary]) {
                    armor -= getIronFleshArmor(secondary) * getSkillValue(SKILL_IRON_FLESH) / 100.0;
                }
            }
        }
    }

    // 4. Spell/affect modifiers (additional APPLY_ARMOR from affects)
    for (affectedData* af = affected; af; af = af->next) {
        if (af->location == APPLY_ARMOR) {
            armor += af->modifier;
        }
    }

    return armor;
}
```

**Component breakdown:**

| Component | Source | Typical Range | Notes |
|-----------|--------|---------------|-------|
| Base Armor | `getMyRace()->getBaseArmor()` | 400-600 | Racial baseline |
| Equipment | `itemAC()` per slot | -500 to 0 | Sum of APPLY_ARMOR |
| Iron Flesh | Slot-specific values | -500 to 0 | Monks only, when unarmored |
| Spell Affects | `APPLY_ARMOR` in affected | -200 to 0 | Temporary bonuses |

**Source:** `code/code/misc/being.cc` (lines 1151-1196)

### MOB AC Calculation

Mobs have automatic AC based on their level:

```cpp
// Mob default AC
int mob_ac = 600 - (20 * mob->ACLevel);

// Use whichever is better (lower)
int actual_ac = min(mob_ac, mob->getArmor());
```

**Example values:**

| ACLevel | Default AC | Notes |
|---------|------------|-------|
| 0 | 600 | Very weak armor |
| 10 | 400 | Moderate armor |
| 20 | 200 | Strong armor |
| 30 | 0 | Excellent armor |

**Source:** `code/code/misc/combat.cc` (defendRound function)

## Equipment Slots and Contribution

### 24 Equipment Slots

```cpp
// code/code/misc/limbs.h
enum wearSlotT {
    WEAR_NOWHERE = 0,

    // Humanoid slots (1-20)
    WEAR_HEAD = 1,      WEAR_NECK = 2,      WEAR_BODY = 3,
    WEAR_BACK = 4,      WEAR_ARM_R = 5,     WEAR_ARM_L = 6,
    WEAR_WRIST_R = 7,   WEAR_WRIST_L = 8,   WEAR_HAND_R = 9,
    WEAR_HAND_L = 10,   WEAR_FINGER_R = 11, WEAR_FINGER_L = 12,
    WEAR_WAIST = 13,    WEAR_LEG_R = 14,    WEAR_LEG_L = 15,
    WEAR_FOOT_R = 16,   WEAR_FOOT_L = 17,   HOLD_RIGHT = 18,
    HOLD_LEFT = 19,

    // Extended slots for extra limbs (20-24)
    WEAR_EX_LEG_R = 20, WEAR_EX_LEG_L = 21,
    WEAR_EX_FOOT_R = 22, WEAR_EX_FOOT_L = 23,

    MAX_HUMAN_WEAR = 20,
    MAX_WEAR = 24
};
```

**Source:** `code/code/misc/limbs.h` (lines 19-51)

### Slot Contribution Percentages

Each slot contributes a specific percentage to total armor:

```cpp
// code/code/obj/obj_base_clothing.cc (lines 191-234)
double TBaseClothing::armorPercs(const wearSlotT slot, bool forStruct) const {
    if (forStruct) {  // Structure point percentages
        switch (slot) {
            case HOLD_RIGHT:
            case HOLD_LEFT:      return 0.07;  // 7% durability
            case WEAR_BODY:      return 0.26;  // 26% (largest component)
            case WEAR_HEAD:      return 0.11;
            case WEAR_WAIST:     return 0.09;
            case WEAR_BACK:      return 0.09;
            case WEAR_LEG_R:
            case WEAR_LEG_L:     return 0.07;  // Split between legs
            case WEAR_ARM_R:
            case WEAR_ARM_L:     return 0.05;  // Split between arms
            case WEAR_FOOT_R:
            case WEAR_FOOT_L:    return 0.04;  // Split between feet
            case WEAR_WRIST_R:
            case WEAR_WRIST_L:   return 0.015; // Split between wrists
            case WEAR_NECK:      return 0.02;
            case WEAR_FINGER_R:
            case WEAR_FINGER_L:  return 0.005; // Split between fingers
        }
    } else {  // Armor AC percentages
        switch (slot) {
            case HOLD_RIGHT:
            case HOLD_LEFT:      return 0.25;  // 25% (shields)
            case WEAR_BODY:      return 0.15;  // 15%
            case WEAR_HEAD:      return 0.10;
            case WEAR_WAIST:     return 0.08;
            case WEAR_BACK:      return 0.07;
            case WEAR_LEG_R:
            case WEAR_LEG_L:     return 0.05;  // Split 10% total
            case WEAR_ARM_R:
            case WEAR_ARM_L:     return 0.04;  // Split 8% total
            case WEAR_FOOT_R:
            case WEAR_FOOT_L:    return 0.035; // Split 7% total
            case WEAR_WRIST_R:
            case WEAR_WRIST_L:   return 0.015; // Split 3% total
            case WEAR_NECK:      return 0.02;
            case WEAR_FINGER_R:
            case WEAR_FINGER_L:  return 0.015; // Split 3% total
        }
    }
    return 0.0;
}
```

**AC Contribution Summary:**

| Slot Type | AC % | Structure % | Notes |
|-----------|------|-------------|-------|
| Hold (Shield) | 25% | 7% | Largest AC contribution |
| Body | 15% | 26% | Largest structure component |
| Head | 10% | 11% | Second largest AC |
| Legs (combined) | 10% | 14% | Split R/L |
| Waist | 8% | 9% | |
| Arms (combined) | 8% | 10% | Split R/L |
| Feet (combined) | 7% | 8% | Split R/L |
| Back | 7% | 9% | |
| Wrists (combined) | 3% | 3% | Split R/L |
| Fingers (combined) | 3% | 1% | Split R/L |
| Neck | 2% | 2% | Smallest contribution |

**Source:** `code/code/obj/obj_base_clothing.cc` (lines 191-234)

## Defense Mechanics

### Defense Round Calculation (defendRound)

The `defendRound()` function converts AC into a defense bonus for hit probability:

```cpp
// code/code/misc/combat.cc (lines 2764-2964)
int TBeing::defendRound(TBeing* target) const {
    int defense = 0;
    int armor = getArmor();

    // Base defense from armor
    if (isPc()) {
        // PC: (armor - 500) * 2/3
        defense = (armor - 500) * 2 / 3;

        // Cap at level-based maximum
        int max_defense = GetMaxLevel() * 1000 / 60 + GetMaxLevel();
        defense = min(defense, max_defense);
    } else {
        // MOB: (armor - 400) * 5/6
        defense = (armor - 400) * 5 / 6;
    }

    // Combat mode modifiers
    if (isCombatMode(ATTACK_DEFENSE)) {
        defense += GetMaxLevel() / 4;  // Defensive stance bonus
    } else if (isCombatMode(ATTACK_OFFENSE)) {
        defense -= GetMaxLevel() / 4;  // Offensive stance penalty
    } else if (isCombatMode(ATTACK_BERSERK)) {
        // Berserk penalty reduced by skill
        int skill = getSkillValue(SKILL_BERSERK);
        defense -= GetMaxLevel() * 8 * (100 - skill) / 100;
    }

    // Skill bonuses
    if (doesKnowSkill(SKILL_ADVANCED_DEFENSE)) {
        defense += max(1, getSkillValue(SKILL_ADVANCED_DEFENSE) / 10);
    }

    if (doesKnowSkill(SKILL_CHIVALRY) && riding) {
        // Mounted defensive bonus
        int skill = getSkillValue(SKILL_CHIVALRY);
        defense += 159 * max(10, skill) / 100;
    }

    if (doesKnowSkill(SKILL_DEFENSE)) {
        defense += GetMaxLevel() * getSkillValue(SKILL_DEFENSE) / 100;
    }

    if (doesKnowSkill(SKILL_OOMLAT)) {
        // Armor scaling skill
        int skill = getSkillValue(SKILL_OOMLAT);
        armor += armor * skill / 250;
        defense = (armor - 500) * 2 / 3;  // Recalculate
    }

    // Stat modifiers
    int agiBonus = (int)(335 * getStatMod(STAT_AGI) - 335);
    defense += agiBonus;  // Range: -67 to +84

    // Position modifiers
    positionTypeT pos = getPosition();
    int positionMod = 0;

    if (pos == POSITION_MOUNTED) {
        positionMod = GetMaxLevel() / 4 + 1;
    } else if (pos == POSITION_FLYING) {
        positionMod = GetMaxLevel() / 3 + 1;
    } else if (pos == POSITION_RESTING) {
        positionMod = -(GetMaxLevel() / 3 + 1);
    } else if (pos == POSITION_SITTING) {
        positionMod = -(GetMaxLevel() / 4 + 1);
    }

    // Ground fighting reduces position penalties
    if (positionMod < 0 && doesKnowSkill(SKILL_GROUNDFIGHTING)) {
        int skill = getSkillValue(SKILL_GROUNDFIGHTING);
        positionMod = positionMod * (100 - skill) / 100;
    }

    defense += positionMod;

    // Spell effects
    if (affectedBySpell(SPELL_AURA_GUARDIAN)) {
        defense += 40;
    }

    return defense;
}
```

**Defense calculation formula summary:**

```
PC Defense = (armor - 500) * 2/3, capped at level*16.67 + level
MOB Defense = (armor - 400) * 5/6

Total Defense = Base + Combat Mode + Skills + Stats + Position + Spells
```

**Component ranges:**

| Component | Range | Example at Level 50 |
|-----------|-------|---------------------|
| Base Defense (PC) | -333 to +100 | 200 armor → -200 defense |
| Combat Mode | -400 to +13 | Defense mode → +13 |
| Advanced Defense | +1 to +10 | 100 skill → +10 |
| Chivalry (mounted) | +16 to +159 | 100 skill → +159 |
| Defense Skill | 0 to +50 | Level 50, 100 skill → +50 |
| Agility | -67 to +84 | High AGI → +84 |
| Position | -17 to +17 | Mounted → +17 |
| Ground Fighting | 0 to +17 | Reduces penalties |
| Aura Guardian | +40 | Spell active → +40 |

**Source:** `code/code/misc/combat.cc` (lines 2764-2964)

### AC to Protection Conversion

**PC conversion:**
- 1 AC point ≈ 0.67 defense
- 25 AC ≈ 17 defense
- 100 AC ≈ 67 defense
- Effective range: armor 200-700 → defense -200 to +133

**MOB conversion:**
- 1 AC point ≈ 0.83 defense
- 25 AC ≈ 21 defense
- 100 AC ≈ 83 defense
- Mobs get better AC-to-defense conversion

**Level scaling:**
- PC defense capped at `level * 16.67 + level`
- Level 50 cap: 50 * 16.67 + 50 ≈ 883 defense maximum
- Higher level characters benefit more from low AC

## Armor Skills and Abilities

### Iron Flesh (SKILL_IRON_FLESH)

Monks gain AC from bare skin when not wearing armor in a slot:

```cpp
// Slot-specific Iron Flesh AC values (negative = better)
const int TBeing::getIronFleshArmor(wearSlotT slot) const {
    switch (slot) {
        case WEAR_BODY:      return -184;  // Largest contribution
        case WEAR_WAIST:     return -98;
        case WEAR_HEAD:      return -86;
        case WEAR_BACK:      return -86;
        case WEAR_LEG_R:     return -61;
        case WEAR_LEG_L:     return -61;
        case WEAR_ARM_R:     return -49;
        case WEAR_ARM_L:     return -49;
        case WEAR_NECK:      return -49;
        case WEAR_HAND_R:    return -37;
        case WEAR_HAND_L:    return -37;
        case WEAR_WRIST_R:   return -24;
        case WEAR_WRIST_L:   return -24;
        case WEAR_FOOT_R:    return -24;
        case WEAR_FOOT_L:    return -24;
        case WEAR_FINGER_R:  return -12;
        case WEAR_FINGER_L:  return -12;
        default:             return 0;
    }
}
```

**Formula:**
```
Iron Flesh AC = getIronFleshArmor(slot) * skillValue / 100.0
```

**Example (100% skill):**
- Body slot: -184 AC (18.4% total possible AC)
- All slots combined: ~-906 AC maximum
- At 50% skill: -453 AC total

**Paired item penalty:**
- If wearing item in one paired slot (e.g., left leg), Iron Flesh removed from other slot (right leg)
- Encourages full armor or full barehand, discourages mixed

**Source:** `code/code/misc/being.cc` (getArmor function)

### Advanced Defense (SKILL_ADVANCED_DEFENSE)

Increases defensive capability directly:

```cpp
// Bonus: max(1, skillValue / 10)
if (doesKnowSkill(SKILL_ADVANCED_DEFENSE)) {
    defense += max(1, getSkillValue(SKILL_ADVANCED_DEFENSE) / 10);
}
```

**Range:** +1 to +10 defense bonus

**Source:** `code/code/misc/combat.cc` (defendRound, lines 2830-2832)

### Focused Avoidance (SKILL_FOCUSED_AVOIDANCE)

Dodge/avoidance mechanic allowing complete attack evasion:

```cpp
// code/code/disc/disc_advanced_defense.cc
bool TBeing::canFocusedAvoidance(int perc) const {
    if (!doesKnowSkill(SKILL_FOCUSED_AVOIDANCE))
        return false;

    if (!awake() || isAffected(AFF_STUNNED))
        return false;

    int skill = getSkillValue(SKILL_FOCUSED_AVOIDANCE);

    // Penalty if legs are hurt
    if (isLimbFlags(WEAR_LEG_R, PART_INJURED) ||
        isLimbFlags(WEAR_LEG_L, PART_INJURED)) {
        skill = skill * 75 / 100;  // 25% penalty
    }

    // AGI stat multiplier
    skill = (int)(skill * getStatMod(STAT_AGI));

    // Success check with FOC influence
    return bSuccess(skill, SKILL_FOCUSED_AVOIDANCE);
}
```

**Usage:** Called during combat to potentially avoid attacks entirely

**Factors:**
- Leg injury: -25% skill effectiveness
- AGI stat: Multiplies skill value (0.8-1.25×)
- FOC stat: Influences bSuccess() check
- Higher `perc` parameter = harder to avoid

**Source:** `code/code/disc/disc_advanced_defense.cc`

### Chivalry (SKILL_CHIVALRY)

Mounted combat defensive bonus:

```cpp
// Formula: 159 * max(10, skillValue) / 100
if (doesKnowSkill(SKILL_CHIVALRY) && riding) {
    int skill = max(10, getSkillValue(SKILL_CHIVALRY));
    defense += 159 * skill / 100;
}
```

**Range:** +16 (10% skill) to +159 (100% skill)

**Source:** `code/code/misc/combat.cc` (defendRound, lines 2834-2837)

### Other Defense Skills

**Blindfighting (SKILL_BLINDFIGHTING):**
- Reduces penalty when fighting unseen opponents
- Formula: `penalty = penalty * (100 - skillValue) / 100`

**Groundfighting (SKILL_GROUNDFIGHTING):**
- Reduces position penalties when not standing
- Formula: `positionPenalty = positionPenalty * (100 - skillValue) / 100`

**Oomlat (SKILL_OOMLAT):**
- Scales armor effectiveness
- Formula: `armor += armor * skillValue / 250`
- At 100 skill: +40% armor effectiveness

**Defense (SKILL_DEFENSE):**
- General defensive training
- Formula: `defense += level * skillValue / 100`
- At level 50, 100 skill: +50 defense

**Source:** `code/code/misc/combat.cc` (defendRound function)

## Armor Flags and Properties

### Object Flags (extra_flags)

Standard object flags affecting armor:

| Flag | Effect on Armor |
|------|-----------------|
| `ITEM_GLOW` | Visual glow effect |
| `ITEM_HUM` | Audio hum effect |
| `ITEM_INVISIBLE` | Armor is invisible |
| `ITEM_MAGIC` | Magical armor marker |
| `ITEM_NODROP` | Cannot be dropped (cursed) |
| `ITEM_BLESS` | Blessed armor |
| `ITEM_ANTI_MAGE` | Mages cannot wear |
| `ITEM_ANTI_CLERIC` | Clerics cannot wear |
| `ITEM_ANTI_WARRIOR` | Warriors cannot wear |
| `ITEM_ANTI_THIEF` | Thieves cannot wear |
| `ITEM_ANTI_MONK` | Monks cannot wear |
| `ITEM_ANTI_DEIKHAN` | Deikhans cannot wear |
| `ITEM_ANTI_SHAMAN` | Shamans cannot wear |
| `ITEM_ANTI_RANGER` | Rangers cannot wear |
| `ITEM_STRUNG` | Customized item (custom names/descriptions) |
| `ITEM_BURNING` | Currently on fire (takes damage) |
| `ITEM_NEWBIE` | Starting equipment |

**Source:** `code/code/misc/obj.h` (extra_flags definition)

### Wear Flags (wear_flags)

Control which slots armor can be equipped in:

| Flag | Slot(s) |
|------|---------|
| `ITEM_WEAR_TAKE` | Can be picked up |
| `ITEM_WEAR_BODY` | Body slot |
| `ITEM_WEAR_HEAD` | Head slot |
| `ITEM_WEAR_LEGS` | Leg slots (R/L) |
| `ITEM_WEAR_FEET` | Feet slots (R/L) |
| `ITEM_WEAR_HANDS` | Hand slots (R/L) |
| `ITEM_WEAR_ARMS` | Arm slots (R/L) |
| `ITEM_WEAR_BACK` | Back slot |
| `ITEM_WEAR_WAIST` | Waist/belt |
| `ITEM_WEAR_NECK` | Neck slot |
| `ITEM_WEAR_WRISTS` | Wrist slots (R/L) |
| `ITEM_WEAR_FINGERS` | Finger slots (R/L) |
| `ITEM_WEAR_HOLD` | Hold slots (shields) |

**Source:** `code/code/misc/obj.h` (wear_flags definition)

### Limb Status Flags (body_parts)

Track body part condition affecting armor:

| Flag | Effect |
|------|--------|
| `PART_BLEEDING` | Active bleeding, reduces HP |
| `PART_INFECTED` | Infection, periodic damage |
| `PART_PARALYZED` | Cannot use limb |
| `PART_BROKEN` | Bone fracture, mobility penalty |
| `PART_SCARRED` | Permanent scarring (cosmetic) |
| `PART_BANDAGED` | Bandaged, healing faster |
| `PART_MISSING` | Limb severed, cannot equip |
| `PART_USELESS` | Non-functional, cannot use |
| `PART_LEPROSED` | Leprosy condition |
| `PART_TRANSFORMED` | Magically transformed shape |
| `PART_ENTANGLED` | Stuck/entangled |
| `PART_BRUISED` | Bruising (minor injury) |
| `PART_GANGRENOUS` | Gangrene (severe condition) |

**Impact on armor:**
- Missing limbs: Cannot equip armor in that slot
- Injured limbs: May reduce skill effectiveness (e.g., Focused Avoidance)
- Paralyzed limbs: Cannot use held items (shields)

**Source:** `code/code/misc/defs.h` (PART_* flags)

## Armor Repair and Maintenance

### Structure Points System

Structure points track item durability:

```cpp
// TObj base fields (not val0-val3)
class TObj {
    int cur_struct;   // Current structure points
    int max_struct;   // Maximum structure points
};

// Modification methods
void addToStructPoints(int delta);      // Change current
void addToMaxStructPoints(int delta);   // Change maximum
```

**Effects of low structure:**
- Reduced item value (damaged items worth less)
- May prevent repairs (too damaged to fix)
- Visual descriptions show damage level

**Source:** `code/code/misc/obj.h` (TObj class)

### Galvanize Spell (SPELL_GALVANIZE)

Reinforces armor by increasing structure points:

```cpp
// code/code/obj/obj_armor.cc (lines 38-65)
int TArmor::galvanizeMe(TBeing* caster, short skillValue) {
    // Prerequisites
    if (getMaxStructPoints() < 2) {
        caster->sendTo("The item is already at maximum structural integrity.\n\r");
        return SPELL_FAIL;
    }

    if (getStructPoints() < 2) {
        caster->sendTo("The item is too damaged to galvanize.\n\r");
        return SPELL_FAIL;
    }

    // Success case
    if (caster->bSuccess(skillValue, SPELL_GALVANIZE)) {
        addToMaxStructPoints(1);   // +1 max durability
        addToStructPoints(1);       // +1 current durability

        act("$p glows briefly with a metallic sheen.",
            FALSE, caster, this, NULL, TO_CHAR);
        act("$p glows briefly with a metallic sheen.",
            FALSE, caster, this, NULL, TO_ROOM);

        return SPELL_SUCCESS;
    }

    // Critical failure (item destroyed)
    else if (critFail(caster, SPELL_GALVANIZE)) {
        act("$p crumbles to dust as the spell backfires!",
            FALSE, caster, this, NULL, TO_CHAR);
        act("$p crumbles to dust as $n's spell backfires!",
            FALSE, caster, this, NULL, TO_ROOM);

        CF(SPELL_GALVANIZE);  // Item destroyed trigger
        return SPELL_CRIT_FAIL_2;
    }

    // Regular failure
    else {
        addToMaxStructPoints(-2);   // -2 max durability
        addToStructPoints(-2);       // -2 current durability

        act("$p weakens as the spell fails.",
            FALSE, caster, this, NULL, TO_CHAR);
        act("$p weakens as $n's spell fails.",
            FALSE, caster, this, NULL, TO_ROOM);

        return SPELL_CRIT_FAIL;
    }
}
```

**Outcomes:**

| Result | Max Structure | Current Structure | Return Value |
|--------|---------------|-------------------|--------------|
| Success | +1 | +1 | SPELL_SUCCESS |
| Failure | -2 | -2 | SPELL_CRIT_FAIL |
| Crit Fail | Item destroyed | Item destroyed | SPELL_CRIT_FAIL_2 |

**Strategy:**
- Only cast on high-quality armor (reduces failure risk)
- Repair armor before galvanizing (must have ≥2 structure)
- Multiple galvanizes can permanently improve max structure
- Risk increases each cast (more structure = harder to add more)

**Source:** `code/code/obj/obj_armor.cc` (lines 38-65)

### Repair Shop System

Shops can repair damaged armor via `objectRepair()` overrides:

```cpp
// Default repair logic (inherited by TArmor from TBaseClothing)
virtual bool objectRepair(TBeing* ch, TMonster* repair, silentTypeT silent);
```

**Repair eligibility:**
- Item must have structure points remaining
- Some item types cannot be repaired (bags, keyrings, card decks, suitcases)
- Repair cost scales with damage amount
- Shops may refuse extremely damaged items

**Source:** `code/code/obj/obj_base_clothing.cc` (objectRepair)

### Structure Damage Sources

Structure points decrease from:
- **Combat damage**: Environmental damage during combat
- **Decay**: Objects decay over time (objectDecay)
- **Fire**: ITEM_BURNING flag causes structure loss
- **Spell failure**: Galvanize failures reduce structure
- **Admin commands**: Manual structure modification

## Special Armor Types

### Shields

Shields are armor worn in the secondary hand (HOLD_LEFT):

```cpp
// code/code/obj/obj_base_clothing.cc
bool TBaseClothing::isShield() const {
    return isname("shield", name);
}
```

**Shield mechanics:**
- Must be equipped in HOLD_LEFT slot
- Provide 25% of total armor contribution (largest single slot)
- Can parry incoming attacks
- May have spec procs triggering on hit
- Block dual wielding (occupies secondary hand)

**Shield parry (combat.cc:3805-3860):**

```cpp
// When defender has shield equipped
if (defenderShield && shieldBlocks()) {
    act("You parry $N's blow with $p",
        FALSE, defender, shield, attacker, TO_CHAR);
    act("$n parries $N's blow with $p",
        FALSE, defender, shield, attacker, TO_NOTVICT);
    act("$n parries your blow with $p",
        FALSE, defender, shield, attacker, TO_VICT);

    // Trigger shield spec proc if present
    int rc = shield->checkSpec(defender, CMD_OBJ_BEEN_HIT, "", attacker);
    if (rc) return rc;  // Spec proc handled it

    return TRUE;  // Attack parried
}
```

**Source:** `code/code/misc/combat.cc` (lines 3805-3860), `code/code/obj/obj_base_clothing.cc`

### Barding

Armor for mounts:

```cpp
bool TBaseClothing::isBarding() const {
    return isname("barding", name);
}
```

**Properties:**
- Worn by mount (not rider)
- Provides AC benefits to mounted creature
- Follows same armor evaluation as standard armor
- May have weight/mobility trade-offs

**Source:** `code/code/obj/obj_base_clothing.cc`

### Armor Wands (TArmorWand)

Hybrid items combining armor protection with spell casting:

**Capabilities:**
- Wearable as armor in standard slots
- Can cast spells like a wand (limited charges)
- Provides both AC and magical utility
- Price combines both benefits (minus duplicate weight)

**Pricing:**
```cpp
int TArmorWand::suggestedPrice() const {
    int armor_price = TArmor::suggestedPrice();
    int wand_price = TWand::suggestedPrice();
    int weight_cost = obj_flags.weight * 10;

    return armor_price + wand_price - weight_cost;
}
```

**Source:** `code/code/obj/obj_armor_wand.cc` (lines 1-78)

### Mount Equipment

**Saddles (TSaddle):**
- Improves mount control and comfort
- Provides armor benefits to rider
- Required for effective mounted combat
- Affects ride skill checks

**Harnesses (THarness):**
- Mount safety equipment
- Carrying capacity bonuses
- Stability improvements

Both follow standard TBaseClothing armor evaluation.

**Source:** `code/code/obj/obj_saddle.cc`, `code/code/obj/obj_harness.cc`

## Armor Customization and Evaluation

### Strung Items (ITEM_STRUNG)

Armor can be customized with unique names and descriptions:

```cpp
// Enable customization
void TObj::swapToStrung() {
    if (isObjStat(ITEM_STRUNG))
        return;  // Already strung

    addObjStat(ITEM_STRUNG);

    // Copy prototype strings to instance
    name = obj_index[getItemIndex()].name;
    shortDescr = obj_index[getItemIndex()].short_desc;
    longDescr = obj_index[getItemIndex()].long_desc;
    actionDescr = obj_index[getItemIndex()].action_desc;
}
```

**After stringing, modify freely:**
```cpp
armor->swapToStrung();
armor->name = "breastplate emblazoned dragon";
armor->shortDescr = "a dragon-emblazoned breastplate";
armor->setDescr("A magnificent breastplate emblazoned with a dragon lies here.");
```

**Persistence:**
- Strung items save custom strings in rent files
- Database storage uses `rent_strung` table
- Each instance has independent strings

**Source:** `code/code/misc/thing.cc` (swapToStrung)

### Armor Tier System (ArmorEvaluator)

The armor evaluation system classifies armor into four tiers based on class restrictions:

```cpp
// code/code/obj/obj_low.cc
enum ArmorTier {
    Tier_Clothing,  // Anti-race only (no class restrictions)
    Tier_Light,     // Anti-mage, anti-shaman
    Tier_Medium,    // Anti-light + anti-monk, anti-thief
    Tier_Heavy      // Anti-medium + anti-cleric, anti-ranger
};
```

**Tier determination:**

| Tier | Class Restrictions | Examples |
|------|-------------------|----------|
| Clothing | None or race-only | Robes, shirts, pants |
| Light | Mage, Shaman | Leather armor, light robes |
| Medium | Light + Monk, Thief | Ringmail, scale mail |
| Heavy | Medium + Cleric, Ranger | Plate mail, full plate |

**Impact:**
- Heavier armor has higher AC values but lower stat bonuses per point
- Light armor emphasizes stat bonuses over raw AC
- Tier affects load level calculations

**Source:** `code/code/obj/obj_low.cc` (ArmorEvaluator class)

### Armor Evaluation Formula

The armor level calculation uses complex formulas:

```cpp
// Base AC formula
armor_ac = (baseACLevel * 25 * ac_perc) + (NEWBIE_AC * ac_perc)

// Where:
const double NEWBIE_AC = 500.0;  // AC for newbie baseline
ac_perc = armorPercs(slot, false);  // Slot AC percentage

// Structure formula
structure = max(base_structure, NEWBIE_STR * sqrt(str_perc / BODY_STR))

// Where:
const double NEWBIE_STR = 30.0;  // Structure for newbie baseline
const double BODY_STR = 0.26;    // Body slot reference
str_perc = armorPercs(slot, true);  // Slot structure percentage

// Paired item adjustment
if (isPaired()) {
    // No penalty
} else {
    structure = max(structure, NEWBIE_STR * sqrt(str_perc / BODY_STR) * 0.5);
}
```

**Example calculation (body slot, AC level 10):**

```
AC contribution:
  ac_perc = 0.15 (body is 15%)
  armor_ac = (10 * 25 * 0.15) + (500 * 0.15)
           = 37.5 + 75
           = 112.5 AC

Structure contribution:
  str_perc = 0.26 (body is 26%)
  structure = max(base, 30 * sqrt(0.26 / 0.26))
            = max(base, 30 * 1.0)
            = 30 structure points
```

**Source:** `code/code/obj/obj_base_clothing.cc` (armorLevel, structLevel)

### Pricing Formula

Armor price combines weight, AC value, and stat bonuses:

```cpp
// code/code/obj/obj_base_clothing.cc
int TBaseClothing::suggestedPrice() const {
    int price = 0;

    // Base weight cost
    price += (int)(10.0 * getWeight() * material_price_modifier);

    // Armor value (AC + structure)
    price += armorPriceStruct(/* params */);

    // Stat bonuses (0.25 per point)
    for (int i = 0; i < MAX_OBJ_AFFECT; i++) {
        if (affected[i].location != APPLY_NONE &&
            affected[i].location != APPLY_ARMOR) {  // Exclude armor AC
            price += abs(affected[i].modifier) * 0.25;
        }
    }

    return price;
}
```

**Components:**

| Component | Formula | Notes |
|-----------|---------|-------|
| Weight | `weight * 10 * material` | Base material cost |
| Armor Value | `armorPriceStruct()` | AC and structure combined |
| Stat Bonuses | `modifier * 0.25` per affect | All non-armor affects |

**Example (leather armor, weight 50, AC -100, +5 STR):**

```
Weight: 50 * 10 * 1.0 = 500
Armor: ~200 (from AC/structure calculation)
Stats: 5 * 0.25 = 1.25
Total: ~701 talens
```

**Source:** `code/code/obj/obj_base_clothing.cc` (suggestedPrice)

## Common Patterns

### Checking Armor AC

```cpp
// Get item's armor contribution
int ac = obj->itemAC();  // Sums APPLY_ARMOR modifiers

// Get total character armor
int total_ac = ch->getArmor();
```

### Adding Armor to Items

```cpp
// WRONG: Armor does NOT use val0-val3
obj->setVal0(200);  // Does nothing for TArmor!

// CORRECT: Add APPLY_ARMOR affect
affectedData aff;
aff.location = APPLY_ARMOR;
aff.modifier = -200;  // Negative is better AC
aff.type = SPELL_ARMOR;
aff.duration = PERMANENT_DURATION;
obj->affected[0] = aff;
```

### Checking Defense Bonus

```cpp
// Calculate defense from armor
int defense = ch->defendRound(opponent);

// For PCs, approximate conversion
int estimated_defense = (ch->getArmor() - 500) * 2 / 3;
```

### Monk Bare-Hand Check

```cpp
// Check if monk benefits from Iron Flesh
if (ch->doesKnowSkill(SKILL_IRON_FLESH)) {
    for (wearSlotT slot = MIN_WEAR; slot < MAX_WEAR; slot++) {
        if (!ch->equipment[slot]) {
            // This slot provides Iron Flesh AC
            int ac_bonus = ch->getIronFleshArmor(slot) *
                          ch->getSkillValue(SKILL_IRON_FLESH) / 100.0;
        }
    }
}
```

### Shield Detection

```cpp
// Check if item is a shield
if (armor->isShield()) {
    // Must be worn in HOLD_LEFT
    ch->equipChar(armor, HOLD_LEFT);
}
```

## Key Constants

```cpp
// AC constants
const double NEWBIE_AC = 500.0;     // Baseline AC value
const double NEWBIE_STR = 30.0;     // Baseline structure value
const double BODY_STR = 0.26;       // Body slot structure reference

// Slot limits
const int MIN_WEAR = 1;             // First wearable slot
const int MAX_HUMAN_WEAR = 20;      // Last humanoid slot
const int MAX_WEAR = 24;            // Last possible slot

// Defense scaling
const int PC_ARMOR_BASE = 500;      // PC armor reference point
const int MOB_ARMOR_BASE = 400;     // MOB armor reference point
const double PC_ARMOR_SCALE = 2.0 / 3.0;   // PC: 0.67 per AC
const double MOB_ARMOR_SCALE = 5.0 / 6.0;  // MOB: 0.83 per AC

// Iron Flesh values
const int IRON_FLESH_BODY = -184;   // Body slot AC contribution
const int IRON_FLESH_WAIST = -98;   // Waist slot AC contribution
const int IRON_FLESH_HEAD = -86;    // Head slot AC contribution
// ... (all slot values in getIronFleshArmor)
```

**Source:** Various files throughout `code/code/obj/` and `code/code/misc/`

## Key Source Files

| File | Purpose | Lines |
|------|---------|-------|
| `code/code/obj/obj_armor.h` | TArmor class declaration | 1-26 |
| `code/code/obj/obj_armor.cc` | TArmor implementation, galvanizeMe | 1-70 |
| `code/code/obj/obj_base_clothing.h` | TBaseClothing base class | 1-76 |
| `code/code/obj/obj_base_clothing.cc` | Armor evaluation, pricing, armorPercs | 1-500+ |
| `code/code/obj/obj_armor_wand.cc` | TArmorWand hybrid class | 1-78 |
| `code/code/obj/obj_saddle.cc` | TSaddle mount equipment | 1-50+ |
| `code/code/obj/obj_harness.cc` | THarness mount equipment | 1-50+ |
| `code/code/misc/being.cc` | getArmor() calculation | 1151-1196 |
| `code/code/misc/combat.cc` | defendRound() defense calculation | 2764-2964 |
| `code/code/misc/combat.cc` | Shield parry mechanics | 3805-3860 |
| `code/code/obj/obj_low.cc` | ArmorEvaluator tier system | - |
| `code/code/disc/disc_advanced_defense.cc` | Focused Avoidance skill | - |
| `code/code/misc/limbs.h` | wearSlotT enum, body part flags | 19-51 |
| `code/code/misc/obj.h` | TObj base class, flags | - |

## Related Documentation

- [Combat Formulas](combat-formulas.md) - How armor affects hit probability and damage
- [Combat Rounds](combat-rounds.md) - Combat timing and attack distribution
- [Equipment Wear](equipment-wear.md) - Equipment slots and limb health
- [Object Types](object-types.md) - Object type system and TObj hierarchy
- [Stats Attributes](stats-attributes.md) - AGI stat effects on defense
- [Position Stance](position-stance.md) - Position modifiers to defense
- [Weapon System](weapon-system.md) - Offensive counterpart to armor
- [To-Hit Defense](tohit-defense.md) - Complete hit probability calculations
