---
title: Weapon System
description: The weapon system uses an inheritance hierarchy with a base abstract class (TBaseWeapon) and specialized subclasses for different weapon categories. Weapons support multiple attack types, sharpness/blunting mechanics, skill-based proficiencies, and extensive customization.
keywords: TBaseWeapon, TGenWeapon, TBow, TArrow, TGun, THandgonne, weaponT, getWeaponDam, swungObjectDamage, sharpenMe, dullMe, weaponLevel, SKILL_DUAL_WIELD, SKILL_SHARPEN, isBluntWeapon, isSlashWeapon, isPierceWeapon, specializationCheck
category: Critical Systems
related:
  - combat-formulas.md
  - tohit-defense.md
  - stats-attributes.md
  - equipment-wear.md
  - spell-definitions.md
  - object-types.md
last_updated: 2026-01-29
source_files:
  - code/code/obj/obj_base_weapon.h
  - code/code/obj/obj_base_weapon.cc
  - code/code/obj/obj_general_weapon.h
  - code/code/obj/obj_general_weapon.cc
  - code/code/obj/obj_bow.h
  - code/code/obj/obj_bow.cc
  - code/code/obj/obj_arrow.h
  - code/code/obj/obj_arrow.cc
  - code/code/obj/obj_gun.h
  - code/code/obj/obj_gun.cc
  - code/code/obj/obj_handgonne.h
  - code/code/obj/obj_handgonne.cc
  - code/code/misc/combat.cc
  - code/code/misc/enum.h
  - code/code/misc/spells.h
  - code/code/sys/db.cc
---

# Weapon System

This document describes SneezyMUD's weapon system, including weapon classes, damage calculations, sharpness mechanics, and special weapon types.

## Overview

The weapon system uses an inheritance hierarchy with a base abstract class (`TBaseWeapon`) and specialized subclasses for different weapon categories. Weapons support multiple attack types, sharpness/blunting mechanics, skill-based proficiencies, and extensive customization.

**Key characteristics:**
- 35 weapon attack types with 3 categories (slash, pierce, blunt)
- Multi-type weapons with frequency-weighted attack selection
- Sharpness system affecting damage output
- Dual wielding with secondary hand penalties
- Specialized classes for bows, guns, and projectiles
- Seven skill types for proficiency and specialization

## Weapon Class Hierarchy

```
TBaseWeapon (abstract)
├── TGenWeapon (general melee weapons)
├── TBow (bows and crossbows)
├── TArrow (arrows and bolts)
└── TGun (firearms)
    └── THandgonne (hand cannons)
```

### TBaseWeapon

**Source:** `code/code/obj/obj_base_weapon.h` (lines 1-96)

The abstract base class for all weapons.

**Private members:**
```cpp
short maxSharp;   // Maximum sharpness value
short curSharp;   // Current sharpness value
ubyte damLevel;   // Damage level (0-255)
ubyte damDev;     // Damage deviation for randomization
int poison;       // Poison type applied to weapon
```

**Key methods:**

| Method | Lines | Purpose |
|--------|-------|---------|
| `baseDamage()` | 145-153 | Returns base damage calculation |
| `swungObjectDamage()` | 519-530 | Core damage with randomization |
| `sharpenMe()` | 155-188 | Sharpening mechanics |
| `dullMe()` | 190+ | Blunting mechanics |
| `weaponLevel()` | 1639-1644 | Weapon effectiveness formula |
| `damageLevel()` | 1646-1648 | Damage component extraction |
| `structLevel()` | 1651-1655 | Structure/condition component |
| `sharpLevel()` | 1657-1661 | Sharpness component |

### TGenWeapon

**Source:** `code/code/obj/obj_general_weapon.h/cc`

General melee weapons supporting up to three attack types.

**Members:**
```cpp
weaponT weapon_type[3];       // Three attack types
int wtype_frequency[3];       // Frequency weighting for random selection
```

**Multi-type support** (lines 70-84):
```cpp
weaponT getWeaponType() const {
    // Frequency-weighted random selection of attack type
    int total = wtype_frequency[0] + wtype_frequency[1] + wtype_frequency[2];
    int roll = ::number(0, total - 1);
    // Returns weapon_type[0], [1], or [2] based on weighted roll
}
```

**Value storage** (lines 42-68):

val0-val1 inherited from TBaseWeapon (sharpness and damage). val3 stores weapon types and frequencies:

```
val3 packing (32-bit):
- bits 0-7:   weapon_type[0]
- bits 8-15:  wtype_frequency[0]
- bits 16-23: weapon_type[1]
- bits 24-31: wtype_frequency[1]

Additional value for weapon_type[2] and wtype_frequency[2]
```

**Combat type checks** (lines 156-166):
```cpp
bool canCudgel() const;     // Blunt weapon, volume ≤ 1500
bool canBackstab() const;   // Pierce weapon, volume ≤ 1500
bool canStab() const;       // Pierce weapon, volume ≤ 2000
```

### TBow

**Source:** `code/code/obj/obj_bow.h` (lines 1-49)

Ranged weapons requiring arrows.

**Members:**
```cpp
ammotype arrowType;   // Type of arrow to fire
int flags;            // BOW_STRING_BROKE, BOW_CARVED, BOW_SCRAPED, BOW_SMOOTHED
int max_range;        // Maximum shooting distance
```

**Bow flags:**
- `BOW_STRING_BROKE` - String is broken/damaged (cannot shoot)
- `BOW_CARVED` - Carved/customized
- `BOW_SCRAPED` - Scraped/roughed
- `BOW_SMOOTHED` - Smoothed/polished

**Methods:**
```cpp
shootMeBow();        // Ranged attack method
getArrowType();      // Query required ammunition
setArrowType();      // Set ammunition type
getMaxRange();       // Get shooting distance limit
```

### TArrow

**Source:** `code/code/obj/obj_arrow.h` (lines 1-63)

Arrows and projectiles extending TBaseWeapon.

**Special features:**
- Has sharpness/damage (inherits from TBaseWeapon)
- Arrow condition flags: `ARROW_FEATHERED`, `ARROW_CARVED`, `ARROW_SCRAPED`, `ARROW_SMOOTHED`
- Trap system: `trap_level`, `trap_dam_type` (can be poisoned or trapped)
- Projectile mechanics: `throwMe()`, `loadBowArrow()`

**Arrow flags** (`obj.h` lines 443-446):
- `ARROW_FEATHERED` - Has feathers (improves accuracy)
- `ARROW_CARVED` - Carved shaft
- `ARROW_SCRAPED` - Scraped surface
- `ARROW_SMOOTHED` - Polished surface

### TGun

**Source:** `code/code/obj/obj_gun.h` (lines 1-122)

Firearms using ammunition.

**Members:**
```cpp
int rof;          // Rate of fire (attacks per round)
ammotype ammotype; // Type of ammunition required
int flags;        // GUN_FLAG_SILENCED, GUN_FLAG_CASELESS, etc.
```

**Gun flags:**
- `GUN_FLAG_SILENCED` - Suppressed/quiet
- `GUN_FLAG_CASELESS` - No shell casings
- `GUN_FLAG_CLIPLESS` - Manually loaded
- `GUN_FLAG_FOULED` - Dirty/damaged

**Ammunition types** (18 total):

| Type | Description |
|------|-------------|
| `AMMO_10MM_PISTOL` | 10mm pistol rounds |
| `AMMO_9MM_PARABELLEM_PISTOL` | 9mm pistol rounds |
| `AMMO_45CAL_ACP_PISTOL` | .45 ACP pistol rounds |
| `AMMO_50CAL_AE_PISTOL` | .50 AE pistol rounds |
| `AMMO_44CAL_MAGNUM_PISTOL` | .44 Magnum pistol rounds |
| `AMMO_32CAL_ACP_PISTOL` | .32 ACP pistol rounds |
| `AMMO_50CAL_BMG_PISTOL` | .50 BMG pistol rounds |
| `AMMO_556MM_NATO_PISTOL` | 5.56mm NATO pistol rounds |
| `AMMO_SS190` | SS190 armor-piercing rounds |
| `AMMO_9MM_PARABELLEM_RIFLE` | 9mm rifle rounds |
| `AMMO_45CAL_ACP_RIFLE` | .45 ACP rifle rounds |
| `AMMO_556MM_RIFLE` | 5.56mm rifle rounds |
| `AMMO_762MM_RIFLE` | 7.62mm rifle rounds |
| `AMMO_30CAL_RIFLE` | .30 caliber rifle rounds |
| `AMMO_FLECHETTE` | Flechette rounds |
| `AMMO_LAW` | Light anti-tank weapon |
| `AMMO_LEAD_SHOT` | Shotgun pellets |
| `AMMO_CANNON_BALL` | Cannon ammunition |

**Methods:**
```cpp
loadMe();         // Load ammunition
unloadMe();       // Unload ammunition
getROF();         // Get rate of fire
setROF();         // Set rate of fire
getAmmoType();    // Get ammunition type
setAmmoType();    // Set ammunition type
```

### THandgonne

**Source:** `code/code/obj/obj_handgonne.h` (lines 1-25)

Historical hand cannons extending TGun.

**Overrides:**
- `loadMe()` - Slower loading mechanics
- `unloadMe()` - Historical unloading
- `shootMeBow()` - Firing mechanics with slower ROF but higher damage

## Object Factory

**Source:** `code/code/sys/db.cc` (lines 3874-4023)

The `makeNewObj(itemTypeT)` factory creates weapon instances:

```cpp
TObj* makeNewObj(itemTypeT type) {
    switch (type) {
        case ITEM_WEAPON:    return new TGenWeapon();
        case ITEM_BOW:       return new TBow();
        case ITEM_ARROW:     return new TArrow();
        case ITEM_GUN:       return new TGun();
        case ITEM_HANDGONNE: return new THandgonne();
        // ...
    }
}
```

## Weapon Types

### weaponT Enumeration

**Source:** `code/code/misc/enum.h` (lines 143-182)

35 distinct weapon attack types:

| Type | Value | Category | Description |
|------|-------|----------|-------------|
| `WEAPON_TYPE_NONE` | 0 | - | No attack type |
| `WEAPON_TYPE_STAB` | 1 | Pierce | Piercing stab |
| `WEAPON_TYPE_WHIP` | 2 | Slash | Whipping lash |
| `WEAPON_TYPE_SLASH` | 3 | Slash | Slashing cut |
| `WEAPON_TYPE_SMASH` | 4 | Blunt | Heavy smash |
| `WEAPON_TYPE_CLEAVE` | 5 | Slash | Cleaving blow |
| `WEAPON_TYPE_CRUSH` | 6 | Blunt | Crushing impact |
| `WEAPON_TYPE_BLUDGEON` | 7 | Blunt | Blunt strike |
| `WEAPON_TYPE_CLAW` | 8 | Slash | Claw attack |
| `WEAPON_TYPE_BITE` | 9 | Pierce | Bite attack |
| `WEAPON_TYPE_STING` | 10 | Pierce | Stinging strike |
| `WEAPON_TYPE_PIERCE` | 11 | Pierce | Sharp pierce |
| `WEAPON_TYPE_PUMMEL` | 12 | Blunt | Multiple hits |
| `WEAPON_TYPE_FLAIL` | 13 | Blunt | Flailing weapon |
| `WEAPON_TYPE_BEAT` | 14 | Blunt | Beating blows |
| `WEAPON_TYPE_THRASH` | 15 | Blunt | Thrashing motions |
| `WEAPON_TYPE_THUMP` | 16 | Blunt | Thumping impact |
| `WEAPON_TYPE_WALLOP` | 17 | Blunt | Heavy wallop |
| `WEAPON_TYPE_BATTER` | 18 | Blunt | Battering attacks |
| `WEAPON_TYPE_STRIKE` | 19 | Blunt | Striking blows |
| `WEAPON_TYPE_CLUB` | 20 | Blunt | Club strikes |
| `WEAPON_TYPE_SLICE` | 21 | Slash | Slicing cuts |
| `WEAPON_TYPE_POUND` | 22 | Blunt | Pounding force |
| `WEAPON_TYPE_THRUST` | 23 | Pierce | Thrusting pierce |
| `WEAPON_TYPE_SPEAR` | 24 | Pierce | Spear attack |
| `WEAPON_TYPE_SMITE` | 25 | Blunt | Divine smite |
| `WEAPON_TYPE_BEAK` | 26 | Pierce | Beak peck |
| `WEAPON_TYPE_AIR` | 27 | Special | Air/wind damage |
| `WEAPON_TYPE_EARTH` | 28 | Special | Earth damage |
| `WEAPON_TYPE_FIRE` | 29 | Special | Fire damage |
| `WEAPON_TYPE_WATER` | 30 | Special | Water damage |
| `WEAPON_TYPE_BEAR_CLAW` | 31 | Slash | Animal claw |
| `WEAPON_TYPE_SHOOT` | 32 | Ranged | Ranged shot |
| `WEAPON_TYPE_CANNON` | 33 | Ranged | Artillery/cannon |
| `WEAPON_TYPE_SHRED` | 34 | Slash | Shredding attacks |

### Type Classification

TGenWeapon supports up to 3 attack types with frequency weighting. Three primary categories are determined by the 2/3 rule:

**isBluntWeapon()** (`obj_base_weapon.cc` lines 609-630):
```cpp
// Returns true if ≥2/3 of attack types are blunt
bool isBluntWeapon() const;
```

**isSlashWeapon()** (`obj_base_weapon.cc` lines 632-654):
```cpp
// Returns true if ≥2/3 of attack types are slash
bool isSlashWeapon() const;
```

**isPierceWeapon()** (`obj_base_weapon.cc` lines 656+):
```cpp
// Returns true if ≥2/3 of attack types are pierce
bool isPierceWeapon() const;
```

## Damage Calculations

### Core Function: getWeaponDam()

**Source:** `code/code/misc/combat.cc` (lines 2041-2140)

Calculates final weapon damage combining base damage, strength, skills, and modifiers.

**Formula:**
```
weaponDamage = (baseDam + rollDam + bonusDam) × strModifier × weaponLearning / 100
```

**Calculation flow:**

1. **Get base weapon damage** via `swungObjectDamage()`
2. **Apply dual wield penalty** (secondary hand only)
3. **Apply character strength modifier** (by weapon type)
4. **Apply weapon-specific category modifiers**
5. **Apply skill proficiency bonus**
6. **Special case for guns** (½ damage)
7. **Incapacitation bonus** (3×wepDam/10, minimum 1)

### swungObjectDamage()

**Source:** `code/code/obj/obj_base_weapon.cc` (lines 519-530)

Returns the weapon's intrinsic damage before character modifiers.

```cpp
int swungObjectDamage() const {
    int baseDam = baseDamage();
    int randomDev = /* random flux based on damDev */;
    int extraDam = /* additional damage bonuses */;

    return baseDam + randomDev + extraDam;
}
```

**baseDamage()** (lines 145-153):
```cpp
double baseDamage() const {
    double multiplier = 1.75 * (isPaired() ? 1.1 : 1.0);
    return damageLevel() * multiplier;
}
```

- Two-handed (non-paired) weapons: 1.75× multiplier
- Paired weapons: Additional 1.1× multiplier (10% bonus)

### Strength Modifier by Weapon Type

**Source:** `code/code/misc/combat.cc` (lines 2107-2140)

Strength affects damage differently based on weapon category:

| Weapon Type | Strength Scaling | Formula |
|-------------|------------------|---------|
| Blunt | Full STR modifier | `strDam` |
| Unarmed | Full STR modifier | `strDam` |
| Slash | Reduced | `(strDam - 1) / 2 + 1` |
| Pierce | Most reduced | `(strDam - 1) / 3 + 1` |

**Example:**
```
STR modifier (strDam) = 1.2 (20% bonus from high STR)

Blunt weapon:  1.2 (full 20% bonus)
Slash weapon:  (1.2 - 1) / 2 + 1 = 1.1 (10% bonus)
Pierce weapon: (1.2 - 1) / 3 + 1 = 1.067 (6.7% bonus)
```

### Weapon Learning

**Source:** `code/code/misc/combat.cc` (lines 2041-2090)

```cpp
weaponLearn = min(100, max(level * 2, getSkillValue(skill)));
```

Characters automatically have weapon learning equal to `level × 2`, or their actual skill value if higher (up to 100).

### Dual Wield Penalty

**Source:** `code/code/misc/combat.cc` (lines 2041-2090)

Secondary hand weapons deal reduced damage:

```cpp
secondaryDamage = primaryDamage * (30 + 30 * SKILL_DUAL_WIELD / 100) / 100;
```

**Range:** 30% minimum (no skill) to 60% maximum (max skill)

**Example:**
- No SKILL_DUAL_WIELD: 30% of primary hand damage
- 50% SKILL_DUAL_WIELD: 45% of primary hand damage
- 100% SKILL_DUAL_WIELD: 60% of primary hand damage

### Gun Damage Penalty

Firearms deal half damage compared to melee weapons with same stats:

```cpp
if (weapon->itemType() == ITEM_GUN) {
    weaponDamage /= 2;
}
```

### weaponLevel Formula

**Source:** `code/code/obj/obj_base_weapon.cc` (lines 1639-1644)

Overall weapon effectiveness combines three components:

```cpp
weaponLevel = (damageLevel × 0.6) + (structLevel × 0.3) + (sharpLevel × 0.1);
```

**Component breakdown:**

| Component | Weight | Formula |
|-----------|--------|---------|
| Damage | 60% | `damLevel / 4.0` |
| Structure | 30% | `max(maxStructPoints - 10, 0) × 2.0 / 3.0` |
| Sharpness | 10% | `max(maxSharp - 10, 0) × 2.0 / 3.0` |

**Interpretation:**
- **Damage** is the primary factor (60% weight)
- **Structure/condition** significantly affects effectiveness (30% weight)
- **Sharpness** has minor impact (10% weight)

## Sharpness System

### Storage Format

**Source:** `code/code/obj/obj_base_weapon.cc` (lines 80-102)

Sharpness is stored in val0 as a bitfield:

```
val0 layout (16-bit):
- bits 0-7:   curSharp (current sharpness)
- bits 8-15:  maxSharp (maximum sharpness)

val1 layout (16-bit):
- bits 0-7:   damLevel (damage level)
- bits 8-15:  damDev (damage deviation)
```

### Sharpness Display

**Source:** `code/code/obj/obj_general_weapon.cc` (lines 94-102)

The display name changes based on weapon type:

| Weapon Category | Display Term | Measures |
|----------------|--------------|----------|
| Blunt | "bluntness" | Dullness level |
| Pierce | "pointiness" | Point condition |
| Slash | "sharpness" | Edge condition |

### Sharpening Mechanics

**sharpenMe()** - `obj_base_weapon.cc` lines 155-188

Increases weapon sharpness through the `SKILL_SHARPEN` skill.

**Flow:**
```
1. Check curSharp < maxSharp (can't sharpen beyond max)
2. Consume movement points (cost varies by weapon)
3. Roll skill check
4. Increment curSharp by 1-2 points on success
5. Cap at maxSharp value
6. Notify character of change
```

**Skill effects:**
- Higher `SKILL_SHARPEN` learning increases success rate
- Reduces movement point cost
- Increases points gained per sharpening session

### Blunting Mechanics

**dullMe()** - `obj_base_weapon.cc` lines 190+

Decreases weapon sharpness through use and wear.

**Automatic dulling occurs when:**
- Weapon strikes hard targets (rocks, metal)
- Weapon parries/blocks multiple times
- Prolonged combat causes duration-based wear

**Effect:**
- Reduces `curSharp` over time
- Proportionally reduces weapon damage output
- Can be intentionally done for specific tactics

### Effect on Damage

Sharpness affects the weapon's overall effectiveness through the `weaponLevel` formula:

```
sharpLevel = max(maxSharp - 10, 0) × 2.0 / 3.0
weaponLevel = ... + (sharpLevel × 0.1)
```

A weapon with damaged sharpness deals reduced damage proportional to the sharpness loss. The effect is minor (10% weight) but noticeable over multiple attacks.

## Value Fields

### TBaseWeapon Values

**Source:** `code/code/obj/obj_base_weapon.cc` (lines 80-102)

| Value | Field | Storage Format |
|-------|-------|----------------|
| val0 | Sharpness | bits 0-7: curSharp, bits 8-15: maxSharp |
| val1 | Damage | bits 0-7: damLevel, bits 8-15: damDev |
| val2 | Reserved | Varies by weapon subclass |
| val3 | Type data | Extended in TGenWeapon |

### TGenWeapon Values

**Source:** `code/code/obj/obj_general_weapon.cc` (lines 42-68)

TGenWeapon extends the value system with weapon types and frequencies:

```
val3 packing (32-bit):
- bits 0-7:   weapon_type[0]
- bits 8-15:  wtype_frequency[0]
- bits 16-23: weapon_type[1]
- bits 24-31: wtype_frequency[1]

Additional value:
- weapon_type[2] and wtype_frequency[2] in separate field
```

**Access methods:**
```cpp
void assignFourValues(int, int, int, int);  // Pack values
void getFourValues(int*, int*, int*, int*); // Unpack values
```

### Database Persistence

All weapon values are packed into these 4 integers and persisted to the MariaDB `obj` table. The bitfield approach enables efficient storage without expanding database schema.

## Weapon Skills

### Skill Definitions

**Source:** `code/code/misc/spells.h` (lines 349-571)

### Dual Wielding

| Skill | Line | Purpose |
|-------|------|---------|
| `SKILL_DUAL_WIELD` | 349 | General dual wield proficiency |
| `SKILL_DUAL_WIELD_THIEF` | 499 | Rogue-specific dual wield mastery |

**Effect:** Allows secondary hand attacks; improves secondary weapon damage scaling from 30% to 60%

### Proficiencies

| Skill | Line | Weapon Type |
|-------|------|-------------|
| `SKILL_SLASH_PROF` | 560 | Slashing weapons |
| `SKILL_PIERCE_PROF` | 561 | Piercing weapons |
| `SKILL_BLUNT_PROF` | 562 | Blunt weapons |
| `SKILL_BAREHAND_PROF` | 563 | Unarmed fighting |
| `SKILL_RANGED_PROF` | 569 | Ranged weapons |

**Effect:** Basic competency with weapon category; learning equal to `level × 2` minimum

### Specializations

| Skill | Line | Class | Weapon Type |
|-------|------|-------|-------------|
| `SKILL_SLASH_SPEC` | 564 | Warrior | Slashing weapons |
| `SKILL_BLUNT_SPEC` | 565 | Warrior | Blunt weapons |
| `SKILL_PIERCE_SPEC` | 566 | Warrior | Piercing weapons |
| `SKILL_BAREHAND_SPEC` | 567 | Monk | Unarmed combat |
| `SKILL_RANGED_SPEC` | 568 | Ranger | Ranged weapons |

**Effect:** Advanced mastery providing damage multipliers and attack frequency bonuses

### Two-Handed Weapons

| Skill | Line | Class | Effect |
|-------|------|-------|--------|
| `SKILL_2H_SPEC` | 358 | Warrior | Two-handed weapon specialization |
| `SKILL_2H_SPEC_DEIKHAN` | 403 | Deikhan | Deikhan-specific two-handed specialization |

**Effect:** Increases attack frequency with two-handed weapons

### Maintenance

| Skill | Line | Purpose |
|-------|------|---------|
| `SKILL_SHARPEN` | 571 | Weapon sharpening |

**Effect:** Allows restoration of weapon sharpness through maintenance

### Specialization Bonus

**Source:** `code/code/obj/obj_base_weapon.cc` (lines 1398-1409)

```cpp
double specializationCheck(TBeing* ch) const {
    int skillValue = ch->getSkillValue(appropriateSpec);
    return skillValue / 100.0;
}
```

Returns decimal bonus (0.0-1.0+) applied as damage multiplier when weapon type matches character's discipline specialization.

### Discipline Integration

**Source:** `code/code/misc/spell_info.cc`

Skills are organized into discipline trees:

| Discipline | Skills | Lines |
|------------|--------|-------|
| `DISC_COMBAT` | All proficiencies, DUAL_WIELD | 1681, 2910-2934 |
| `DISC_SLASH` | SLASH_SPEC | 3645 |
| `DISC_BLUNT` | BLUNT_SPEC | 3653 |
| `DISC_PIERCE` | PIERCE_SPEC | 3661 |
| `DISC_RANGED` | RANGED_PROF, RANGED_SPEC | - |

## Weapon Flags

### Item Wear Flags

**Source:** `code/code/misc/obj.h` (lines 232-247)

Control where weapons can be equipped:

| Flag | Meaning |
|------|---------|
| `ITEM_WEAR_TAKE` | Can be picked up |
| `ITEM_WEAR_HOLD` | Can be held in hand |
| `ITEM_WEAR_THROW` | Can be thrown as projectile |

### Weapon-Specific Flags

**Source:** `code/code/misc/obj.h` (lines 456-489)

| Flag | Purpose |
|------|---------|
| `ITEM_PAIRED` | Can be dual-wielded (matched set) |
| `ITEM_SPIKED` | Has spikes/barbs for extra damage |
| `ITEM_MAGIC` | Enchanted weapon |
| `ITEM_GLOW` | Emits light |
| `ITEM_HUM` | Makes humming sound |
| `ITEM_NODROP` | Cannot be dropped (cursed) |
| `ITEM_BLESS` | Blessed/holy weapon |
| `ITEM_NORENT` | Cannot be rented |
| `ITEM_BURNING` | Currently on fire |

### Bow Flags

**Source:** `code/code/misc/obj.h` (lines 438-442)

| Flag | Effect |
|------|--------|
| `BOW_STRING_BROKE` | String is broken/damaged (cannot shoot) |
| `BOW_CARVED` | Carved/customized |
| `BOW_SCRAPED` | Scraped/roughed |
| `BOW_SMOOTHED` | Smoothed/polished |

### Arrow Flags

**Source:** `code/code/misc/obj.h` (lines 443-446)

| Flag | Effect |
|------|--------|
| `ARROW_FEATHERED` | Has feathers (improves accuracy) |
| `ARROW_CARVED` | Carved shaft |
| `ARROW_SCRAPED` | Scraped surface |
| `ARROW_SMOOTHED` | Polished surface |

### Gun Flags

**Source:** `code/code/obj/obj_gun.h`

| Flag | Effect |
|------|--------|
| `GUN_FLAG_SILENCED` | Suppressed/quiet |
| `GUN_FLAG_CASELESS` | No shell casings |
| `GUN_FLAG_CLIPLESS` | Manually loaded |
| `GUN_FLAG_FOULED` | Dirty/damaged |

### Anti-Class Restrictions

Weapons can have `ITEM_ANTI_MAGE`, `ITEM_ANTI_CLERIC`, `ITEM_ANTI_WARRIOR`, etc. flags to restrict usage by class.

## Two-Handed Weapons and Dual Wielding

### Two-Handed Weapon Mechanics

**Damage multiplier** (`obj_base_weapon.cc` lines 145-153):
```cpp
double baseDamage() const {
    double multiplier = 1.75 * (isPaired() ? 1.1 : 1.0);
    return damageLevel() * multiplier;
}
```

- Two-handed (non-paired) weapons: 1.75× damage multiplier
- Paired weapons: Additional 1.1× multiplier (10% bonus)

**Two-handed specialization:**
- `SKILL_2H_SPEC` and `SKILL_2H_SPEC_DEIKHAN` increase attack frequency
- Bonus: `skillValue / 100.0` additional attacks per round

### Dual Wielding System

**Secondary hand damage penalty** (`combat.cc` lines 2041-2090):

```cpp
secondaryDamage = primaryDamage * (30 + 30 * SKILL_DUAL_WIELD / 100) / 100;
```

**Scaling:**
- No skill: 30% of primary hand damage
- 50 skill: 45% of primary hand damage
- 100 skill: 60% of primary hand damage

**Attack frequency distribution** (`obj_general_weapon.cc` line 1):
- Primary hand: 60% of total attacks
- Secondary hand: 40% of total attacks

### ITEM_PAIRED Flag

**Source:** `code/code/misc/obj.h` (lines 575-585)

Paired weapons (matching sets) receive special bonuses:

```cpp
bool isPaired() const {
    return isObjStat(ITEM_PAIRED);
}
```

**Benefits:**
- 1.1× damage bonus on primary hand
- Can be worn/held in both hands simultaneously
- Designed as matching set (cosmetic and mechanical)

### Hand-Specific Combat Checks

**Source:** `code/code/obj/obj_general_weapon.cc` (lines 156-166)

| Method | Requirement | Purpose |
|--------|-------------|---------|
| `canCudgel()` | Blunt, volume ≤ 1500 | Heavy blows |
| `canBackstab()` | Pierce, volume ≤ 1500 | Finesse strikes |
| `canStab()` | Pierce, volume ≤ 2000 | Thrusting capability |

These methods determine which special attacks are available with the weapon.

## Weapon Repair and Maintenance

### Sharpening

**SKILL_SHARPEN** - `spells.h` line 571

**Mechanics** - `obj_base_weapon.cc` lines 155-188

Each sharpen action:
1. Check `curSharp < maxSharp` (can't sharpen beyond max)
2. Consume movement points (cost varies by weapon complexity)
3. Roll skill check based on `SKILL_SHARPEN` learning
4. Increment `curSharp` by 1-2 points on success
5. Cap at `maxSharp` value
6. Notify character of sharpness change

**Skill effects:**
- Higher learning increases success rate
- Reduces movement point cost
- Increases points gained per session

### Blunting/Dulling

**dullMe()** - `obj_base_weapon.cc` lines 190+

Automatic dulling occurs when:
1. Weapon strikes hard targets (rocks, metal armor)
2. Weapon parries/blocks multiple attacks
3. Duration-based wear during prolonged combat

**Effect:**
- Reduces `curSharp` proportionally
- Lowers weapon damage output via `weaponLevel` calculation
- Can be intentionally done for specific tactics (blunt weapons against certain armor types)

### Structure Point System

Weapons have separate health/condition tracking:

**structLevel()** - `obj_base_weapon.cc` lines 1651-1655

```cpp
double structLevel() const {
    return max(maxStructPoints - 10, 0) * 2.0 / 3.0;
}
```

**Damaged structure effects:**
- Deals reduced damage (30% of `weaponLevel` is from structure)
- May break during combat if condition too low
- Requires repair (separate from sharpening)

### Weapon Display

**statObjInfo()** - `obj_general_weapon.cc` lines 94-102

Shows character the weapon's condition:

```
Current [sharpness/bluntness/pointiness]: X  Damage Level: Y
Structure: Z/W  [Additional enchantment data]
```

Display term varies by weapon type (slash=sharpness, blunt=bluntness, pierce=pointiness).

## Special Weapon Types

### Bow System

**TBow class** - `obj_bow.h` lines 1-49

**Members:**
```cpp
ammotype arrowType;   // Type of arrow to fire
int flags;            // Bow condition flags
int max_range;        // Maximum shooting distance
```

**Methods:**
```cpp
shootMeBow();         // Ranged attack
getArrowType();       // Query required ammunition
setArrowType();       // Set ammunition type
getMaxRange();        // Get shooting distance limit
setMaxRange();        // Set shooting distance limit
getBowFlags();        // Get condition flags
isBowFlag();          // Check specific flag
addBowFlags();        // Add condition flag
remBowFlags();        // Remove condition flag
```

**String mechanics:**
- `BOW_STRING_BROKE` flag prevents shooting
- Broken strings can be repaired by skilled characters
- String quality affects accuracy and damage

### Arrow System

**TArrow class** - `obj_arrow.h` lines 1-63

**Special features:**
- Inherits from TBaseWeapon (has sharpness/damage)
- Arrow condition affects flight and damage
- Trap system: Can be poisoned or trapped
- Projectile mechanics: `throwMe()`, `loadBowArrow()`

**Arrow flags:**
- `ARROW_FEATHERED` - Improves accuracy and distance
- `ARROW_CARVED` - Custom carved shaft
- `ARROW_SCRAPED` - Surface preparation
- `ARROW_SMOOTHED` - Polished for reduced drag

**Damage calculation:**
- Base damage from TBaseWeapon
- Reduced by distance traveled
- Modified by arrow condition (feathering, straightness)
- Can carry poison or trap effects

### Firearm System

**TGun class** - `obj_gun.h` lines 1-122

**Members:**
```cpp
int rof;              // Rate of fire (attacks per round)
ammotype ammotype;     // Type of ammunition required
int flags;            // Gun condition and features
```

**Methods:**
```cpp
loadMe();             // Load ammunition
unloadMe();           // Unload ammunition
getROF();             // Get rate of fire
setROF();             // Set rate of fire
getAmmoType();        // Get ammunition type
setAmmoType();        // Set ammunition type
getGunFlags();        // Get gun flags
isGunFlag();          // Check specific flag
addGunFlags();        // Add gun flag
remGunFlags();        // Remove gun flag
```

**Ammunition system:**
- 18 distinct ammunition types
- Guns require matching ammunition
- Incompatible ammo may fail or damage weapon
- Clip/magazine capacity determines sustained fire rate

**Damage calculation** (`combat.cc` lines 2041-2090):
```cpp
if (weapon->itemType() == ITEM_GUN) {
    weaponDamage /= 2;  // Firearms deal half damage
}
```

**Gun flags:**
- `GUN_FLAG_SILENCED` - Reduced noise (stealth advantage)
- `GUN_FLAG_CASELESS` - No shell casings left behind
- `GUN_FLAG_CLIPLESS` - Manually loaded (slow reload)
- `GUN_FLAG_FOULED` - Dirty/damaged (reduced reliability)

### THandgonne System

**THandgonne class** - `obj_handgonne.h` lines 1-25

Historical hand cannons with specialized mechanics:

**Overrides:**
- `loadMe()` - Slower loading process (historical accuracy)
- `unloadMe()` - Historical unloading mechanics
- `shootMeBow()` - Firing with high damage but slow ROF

**Characteristics:**
- Slower rate of fire than modern guns
- Higher individual shot damage
- More unreliable loading
- Historical flavor and balance

## Weapon Creation and Customization

### Factory Creation

**Database instantiation** - `sys/db.cc` lines 3874-4023

```cpp
TObj* makeNewObj(itemTypeT type) {
    switch (type) {
        case ITEM_WEAPON:    return new TGenWeapon();
        case ITEM_BOW:       return new TBow();
        case ITEM_ARROW:     return new TArrow();
        case ITEM_GUN:       return new TGun();
        case ITEM_HANDGONNE: return new THandgonne();
        // ...
    }
}
```

### Database Loading

**read_object()** - `sys/db.cc` lines 2474-2517

```
Flow:
1. Call makeNewObj() to instantiate weapon
2. Populate from database cache or query:
   - assignFourValues(val0, val1, val2, val3)
   - Restore sharpness, damage, deviation
   - Restore weapon types and frequencies
3. Apply extra descriptions
4. Apply affects/enchantments
5. Register in object_list
6. Return initialized weapon
```

### Runtime Customization

**swapToStrung()** - Enables modification of existing weapons

```cpp
obj->swapToStrung();  // Marks weapon as customized
```

After calling `swapToStrung()`, the following can be modified:
- `name` (e.g., "flaming longsword")
- `shortDescr` (e.g., "a flaming longsword")
- `longDescr` (room description)
- `action_description` (special text)

Modifications persist and are saved to rent files.

### Value Modification

Direct value assignment for customization:

```cpp
// Sharpness adjustment
obj->setMaxSharp(newValue);
obj->setCurSharp(newValue);

// Damage adjustment
obj->setWeapDamLvl(newLevel);
obj->setWeapDamDev(newDev);

// Weapon type modification (TGenWeapon)
TGenWeapon* gw = dynamic_cast<TGenWeapon*>(obj);
gw->setWeaponType(WEAPON_TYPE_SLASH, 0);  // First slot
gw->setWeaponFreq(100, 0);                 // 100% frequency
```

### Constructor Initialization

**TBaseWeapon constructor** - `obj_base_weapon.cc` lines 30-78

Initializes:
- `maxSharp` to base value (10-50 depending on material)
- `curSharp` to `maxSharp` (fully sharp when created)
- `damLevel` to base value (calculated from vnum/material)
- `damDev` to deviation value (randomization range)
- `poison` to 0 (no poison initially)

**TGenWeapon constructor** - `obj_general_weapon.cc` lines 15-27

Initializes three weapon type slots:
- `weapon_type[0], [1], [2] = WEAPON_TYPE_NONE`
- `wtype_frequency[0], [1], [2] = 0`

Requires database load to populate actual types.

### In-Game Editor Support

The `oedit` command (builder tool) supports weapon editing:

**Available modifications:**
1. Change weapon name/descriptions (`swapToStrung`)
2. Adjust sharpness values
3. Change damage level and deviation
4. Modify weapon types
5. Change type frequencies (damage distribution)
6. Apply enchantments (via affects system)
7. Set flags (paired, spiked, magic, etc.)

### Persistent Storage

**Binary rent file format:**

All weapon data packed into 4 integers (val0-val3):
- val0: sharpness bitfield
- val1: damage level bitfield
- val3: type/frequency bitfield (TGenWeapon)
- Additional values for extended data

**Database storage** - MariaDB `obj` table:

Stores:
- `vnum`: weapon virtual number
- `type`: `ITEM_WEAPON`, `ITEM_BOW`, `ITEM_GUN`, etc.
- `val0-val3`: packed weapon values
- `extra_flags`: `ITEM_PAIRED`, `ITEM_SPIKED`, etc.
- `weight`, `cost`, `material`, structure points
- Affect data: enchantments and bonuses

## Common Patterns

### Getting Weapon Damage

```cpp
// In combat system
int baseDam = weapon->swungObjectDamage();
int finalDam = getWeaponDam(ch, weapon, victim);
```

### Checking Weapon Type

```cpp
// Category check
if (weapon->isBluntWeapon()) {
    // Full strength modifier
}

// Specific type check
TGenWeapon* gw = dynamic_cast<TGenWeapon*>(weapon);
if (gw) {
    weaponT type = gw->getWeaponType();  // Random weighted selection
}
```

### Sharpening Workflow

```cpp
// Player uses sharpen command
if (weapon->getCurSharp() < weapon->getMaxSharp()) {
    weapon->sharpenMe(ch);
    // Consumes move points, increases curSharp
}
```

### Dual Wield Setup

```cpp
// Check if weapon can be dual wielded
if (weapon->isObjStat(ITEM_PAIRED)) {
    // Equip in both hands
    ch->equipChar(weapon, HOLD_LEFT);
    ch->equipChar(pair, HOLD_RIGHT);
}
```

## Key Constants

| Constant | Value | Usage |
|----------|-------|-------|
| `MAX_SHARP_BONUS` | - | Maximum sharpness value |
| `TWO_HAND_MULTIPLIER` | 1.75 | Damage boost for two-handed |
| `PAIRED_MULTIPLIER` | 1.1 | Bonus for paired weapons |
| `SECONDARY_MIN` | 30% | Minimum secondary hand damage |
| `SECONDARY_MAX` | 60% | Maximum secondary hand damage |
| `GUN_PENALTY` | 0.5 | Firearm damage divisor |

## Related Documentation

- [Combat Formulas](combat-formulas.md) - How weapon damage integrates with combat calculations
- [To-Hit/Defense](tohit-defense.md) - Hit probability with different weapon types
- [Stats and Attributes](stats-attributes.md) - Strength effects on weapon damage
- [Equipment Wear](equipment-wear.md) - Equipment slots for weapons
- [Spell Definitions](spell-definitions.md) - Weapon skills and disciplines
- [Object Types](object-types.md) - Weapon class hierarchy within TObj

## Key Source Files

| File | Purpose |
|------|---------|
| `code/code/obj/obj_base_weapon.h/cc` | TBaseWeapon base class |
| `code/code/obj/obj_general_weapon.h/cc` | TGenWeapon melee weapons |
| `code/code/obj/obj_bow.h/cc` | TBow ranged weapons |
| `code/code/obj/obj_arrow.h/cc` | TArrow projectiles |
| `code/code/obj/obj_gun.h/cc` | TGun firearms |
| `code/code/obj/obj_handgonne.h/cc` | THandgonne hand cannons |
| `code/code/misc/combat.cc` | getWeaponDam() damage calculation |
| `code/code/misc/enum.h` | weaponT enumeration |
| `code/code/misc/spells.h` | Weapon skill definitions |
| `code/code/sys/db.cc` | makeNewObj() factory, read_object() |
