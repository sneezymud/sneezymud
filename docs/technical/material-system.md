---
title: Material Property System
description: The material system manages physical and economic properties of objects through 83 defined materials with 14 distinct properties affecting weapon damage, armor protection, durability, flammability, and value.
keywords:
  - material_type_numbers
  - MAT_STEEL
  - MAT_IRON
  - hardness
  - susceptibility
  - flammability
  - material_nums
  - crafting
  - weapon damage
  - armor protection
  - durability
  - MetalMaterial
  - HideMaterial
  - terrain cost
  - buoyancy
category: Understanding Systems

  - equipment-wear.md
  - combat-formulas.md
  - economy-system.md
last_updated: 2026-01-29
source_files:
  - code/code/misc/materials.h
  - code/code/misc/constants.cc
  - code/code/misc/thing.h
  - code/code/misc/thing.cc
  - code/code/obj/obj_base_weapon.cc
  - code/code/obj/obj_base_corpse.cc
  - code/code/obj/obj_armor_wand.cc
  - code/code/misc/obj.h
related: [object-system.md]
---

# Material Property System

The material system in SneezyMUD manages physical and economic properties of objects through a comprehensive material type classification. Each of the **83 defined materials** has **14 distinct properties** that affect weapon damage, armor protection, durability, flammability, buoyancy, value, and special behaviors.

**Misusing this system causes gameplay imbalances.** Common errors: setting hardness too low (weapons become useless), incorrect susceptibility values (invulnerable armor), missing material assignments (objects inherit undefined behavior), wrong flammability values (objects that shouldn't burn).

## Material Type Enumeration

### Overview

SneezyMUD defines **83 materials** organized into **4 category ranges** with **200 total slots** available for future expansion.

| Category | Range | Count | Usage |
|----------|-------|-------|-------|
| General | 0-19 | 20 | Common materials (cloth, wood, glass) |
| Nature | 50-77 | 28 | Organic and elemental materials |
| Mineral | 100-126 | 27 | Stones and gems |
| Metal | 150-177 | 28 | Metallic materials |

**Source:** `code/code/misc/materials.h` (lines 11-123)

### General Materials (0-19)

```cpp
const int MAT_UNDEFINED = 0;      // Default/error state
const int MAT_PAPER = 1;          // Flammable, low durability
const int MAT_CLOTH = 2;          // Flexible, flammable
const int MAT_WAX = 3;            // Low melting point
const int MAT_GLASS = 4;          // Brittle, shatters on impact
const int MAT_WOOD = 5;           // Moderate strength, flammable
const int MAT_SILK = 6;           // Fine cloth, expensive
const int MAT_FOODSTUFF = 7;      // Perishable
const int MAT_PLASTIC = 8;        // Modern synthetic
const int MAT_RUBBER = 9;         // Elastic, waterproof
const int MAT_CARDBOARD = 10;     // Low durability
const int MAT_STRING = 11;        // Thin, fragile
const int MAT_PLASMA = 12;        // Magical elemental
const int MAT_TOUGH_CLOTH = 13;   // Reinforced cloth
const int MAT_CORAL = 14;         // Oceanic material
const int MAT_HORSEHAIR = 15;     // Animal hair
const int MAT_HAIR = 16;          // Fine hair
const int MAT_POWDER = 17;        // Disperses in wind
const int MAT_PUMICE = 18;        // Porous volcanic rock
const int MAT_LAMINATED = 19;     // Layered material
```

**Range constant:** `MAX_MAT_GENERAL = 20`

### Nature Materials (50-77)

```cpp
const int MAT_GEN_ORG = 50;       // Generic organic
const int MAT_LEATHER = 51;       // Processed hide
const int MAT_TOUGH_LEATHER = 52; // Reinforced leather
const int MAT_DRAGON_SCALE = 53;  // Rare dragon scales
const int MAT_WOOL = 54;          // Sheep wool
const int MAT_FUR = 55;           // Generic fur
const int MAT_FEATHERED = 56;     // Feathered material
const int MAT_WATER = 57;         // Elemental water
const int MAT_FIRE = 58;          // Elemental fire
const int MAT_EARTH = 59;         // Elemental earth
const int MAT_ELEMENTAL = 60;     // Generic elemental
const int MAT_ICE = 61;           // Frozen water
const int MAT_LIGHTNING = 62;     // Electrical elemental
const int MAT_CHAOS = 63;         // Chaotic elemental
const int MAT_CLAY = 64;          // Earthenware
const int MAT_PORCELAIN = 65;     // Refined clay
const int MAT_STRAW = 66;         // Plant fibers
const int MAT_PEARL = 67;         // Lustrous gem
const int MAT_HUMAN_FLESH = 68;   // Organic flesh
const int MAT_FUR_CAT = 69;       // Feline fur
const int MAT_FUR_DOG = 70;       // Canine fur
const int MAT_FUR_RABBIT = 71;    // Rodent fur
const int MAT_GHOSTLY = 72;       // Spectral material
const int MAT_DWARF_LEATHER = 73; // Dwarven-crafted
const int MAT_SOFT_LEATHER = 74;  // Soft processed hide
const int MAT_FISHSCALE = 75;     // Aquatic scales
const int MAT_OGRE_HIDE = 76;     // Giant hide
const int MAT_HEMP = 77;          // Plant fiber rope
```

**Range constant:** `MAX_MAT_NATURE = 28` (extends to slot 77)

### Mineral Materials (100-126)

```cpp
const int MAT_GEN_MINERAL = 100;  // Generic mineral
const int MAT_RUNESTONE = 102;    // Magical stone
const int MAT_CRYSTAL = 103;      // Clear crystal
const int MAT_DIAMOND = 104;      // Hardest mineral
const int MAT_EBONY = 105;        // Dark wood
const int MAT_EMERALD = 106;      // Green gem
const int MAT_IVORY = 107;        // Carved animal bone
const int MAT_OBSIDIAN = 108;     // Volcanic glass
const int MAT_ONYX = 109;         // Black mineral
const int MAT_OPAL = 110;         // Iridescent gem
const int MAT_RUBY = 111;         // Red precious stone
const int MAT_SAPPHIRE = 112;     // Blue precious stone
const int MAT_MARBLE = 113;       // Sculptable stone
const int MAT_STONE = 114;        // Generic stone
const int MAT_BONE = 115;         // Processed bone
const int MAT_JADE = 116;         // Green ornamental
const int MAT_AMBER = 117;        // Fossilized resin
const int MAT_TURQUOISE = 118;    // Blue-green stone
const int MAT_AMETHYST = 119;     // Purple crystal
const int MAT_MICA = 120;         // Flaky mineral
const int MAT_DRAGONBONE = 121;   // Dragon skeletal
const int MAT_MALACHITE = 122;    // Banded green stone
const int MAT_GRANITE = 123;      // Igneous rock
const int MAT_QUARTZ = 124;       // Common crystal
const int MAT_JET = 125;          // Black fossilized wood
const int MAT_CORUNDUM = 126;     // Aluminum oxide gem
```

**Range constant:** `MAX_MAT_MINERAL = 27` (extends to slot 126)

### Metal Materials (150-177)

```cpp
const int MAT_COPPER = 151;       // Soft reddish metal
const int MAT_BRONZE = 156;       // Copper-tin alloy
const int MAT_BRASS = 157;        // Copper-zinc alloy
const int MAT_IRON = 158;         // Common ferrous metal
const int MAT_STEEL = 159;        // Iron-carbon alloy
const int MAT_MITHRIL = 160;      // Legendary mithril
const int MAT_ADAMANTITE = 161;   // Legendary adamant
const int MAT_SILVER = 162;       // Precious metal
const int MAT_GOLD = 163;         // Yellow precious
const int MAT_PLATINUM = 164;     // Rare precious
const int MAT_TITANIUM = 165;     // Light strong metal
const int MAT_ALUMINUM = 166;     // Light nonferrous
const int MAT_GNICKEL = 168;      // Nickel alloy
const int MAT_ELECTRUM = 169;     // Gold-silver alloy
const int MAT_ATHANOR = 170;      // Philosophical metal
const int MAT_TIN = 171;          // Soft silvery metal
const int MAT_TUNGSTEN = 172;     // Hard refractory
const int MAT_STARMETAL = 173;    // Celestial metal
const int MAT_TERBIUM = 174;      // Rare earth metal
const int MAT_ETERNIUM = 177;     // Eternal magical metal
```

**Range constant:** `MAX_MAT_METAL = 28` (extends to slot 177)

## Material Properties Structure

### The material_type_numbers Struct

Each material is defined by 14 properties stored in the `material_type_numbers` structure:

```cpp
struct material_type_numbers {
    short cut_susc;                    // Slash weapon susceptibility (0-100)
    short smash_susc;                  // Blunt weapon susceptibility (0-100)
    short burned_susc;                 // Fire/heat susceptibility (0-100)
    short pierced_susc;                // Piercing weapon susceptibility (0-100)
    short hardness;                    // Material hardness (affects damage)
    unsigned short water_susc;         // Water/corrosion susceptibility (0-249)
    unsigned short fall_susc;          // Impact/fall damage susceptibility (0-249)
    unsigned short float_weight;       // Buoyancy (0=sinks, 255=floats)
    short noise;                       // Sound generation (-5 to +10)
    unsigned short vol_mult;           // Volume multiplier (1-8)
    unsigned short conductivity;       // Electrical conductivity (0 or 1)
    int flammability;                  // Fire ignition rating (0-1000)
    unsigned short acid_susc;          // Acid/corrosion susceptibility (0-100)
    float price;                       // Base economic value per unit
    int (*repair_proc)(TBeing*, TObj*);// Repair function pointer
    char mat_name[20];                 // Material name string (max 20 chars)
};
```

**Source:** `code/code/misc/materials.h` (lines 125-142)

### Global Material Array

```cpp
extern const struct material_type_numbers material_nums[200];
```

The array is initialized in `code/code/misc/constants.cc` (lines 840-1039) with all 83 material definitions.

### Property Descriptions

| Property | Range | Purpose | Effect on Gameplay |
|----------|-------|---------|-------------------|
| **cut_susc** | 0-100 | Slash vulnerability | Higher = more damage from slash weapons (swords, axes) |
| **smash_susc** | 0-100 | Blunt vulnerability | Higher = more damage from clubs, hammers, fists |
| **burned_susc** | 0-100 | Fire vulnerability | Higher = more fire damage, faster burning |
| **pierced_susc** | 0-100 | Pierce vulnerability | Higher = more damage from arrows, spears, stabs |
| **hardness** | 0-100 | Damage resistance | Higher = better weapon damage, armor protection |
| **water_susc** | 0-249 | Corrosion rate | >100 = rust-prone; affects longevity in water |
| **fall_susc** | 0-249 | Impact resistance | Higher = more likely to break when dropped |
| **float_weight** | 0-255 | Buoyancy | 0=sinks instantly, 255=very buoyant |
| **noise** | -5 to +10 | Sound level | Higher = louder; affects stealth |
| **vol_mult** | 1-8 | Density | Higher = denser/heavier objects |
| **conductivity** | 0 or 1 | Electrical | 1=conductive (lightning damage bonus) |
| **flammability** | 0-1000 | Fire spread | 0=fireproof, 1000=extremely flammable |
| **acid_susc** | 0-100 | Acid resistance | Higher = more vulnerable to acid |
| **price** | 0.05-250 | Base value | Economic value per unit (gold) |

### Example Material Entry

Iron (MAT_IRON = 158) from `constants.cc` line 1031:

```cpp
{0, 50, 0, 0, 60, 101, 101, 0, 27, 1, 1, 0, 70, 5, repairMetal, "iron"}
```

Breaking this down:

| Field | Value | Meaning |
|-------|-------|---------|
| cut_susc | 0 | Resistant to slash damage |
| smash_susc | 50 | Moderate blunt vulnerability |
| burned_susc | 0 | Fireproof |
| pierced_susc | 0 | Resistant to piercing |
| hardness | 60 | Good damage/protection |
| water_susc | 101 | Rust-prone in water |
| fall_susc | 101 | Moderate impact resistance |
| float_weight | 0 | Sinks immediately |
| noise | 27 | Moderately loud |
| vol_mult | 1 | Standard density |
| conductivity | 1 | Conducts electricity |
| flammability | 0 | Non-flammable |
| acid_susc | 70 | Vulnerable to acid |
| price | 5 | 5 gold per unit |

## Weight and Volume Calculations

### Weight System

Objects maintain weight calculated from base weight and material volume multiplier:

```cpp
float TThing::getCarriedWeight() const {
    TThing* t;
    float total = 0;

    for (t = rider; t; t = t->nextRider) {
        total += t->getTotalWeight(true);
    }

    for (StuffIter it = stuff.begin(); it != stuff.end() && (t = *it); ++it) {
        if (dynamic_cast<TComponent*>(t))
            total += (t->getTotalWeight(true) * 0.10);  // Components 10% weight
        else
            total += t->getTotalWeight(true);
    }
    return total;
}
```

**Source:** `code/code/misc/thing.cc` (lines 106-124)

### Volume Multiplier (vol_mult)

The `vol_mult` property scales object volume:

| vol_mult | Density Category | Example Materials |
|----------|------------------|-------------------|
| 1 | Standard | Iron, steel, most materials |
| 2-3 | Dense | Lead, gold, platinum |
| 4-8 | Very Dense | Adamantite, mithril (magical density) |

Objects with higher `vol_mult` take more inventory space for the same base size.

## Weapon Damage Calculations

Material hardness directly modifies weapon effectiveness:

```cpp
// From getWeaponDamageBonus()
int hardness = material_nums[item->getMaterial()].hardness;

// Hardness scaling applied to weapon damage
// Higher hardness (60-100) = better damage output
// Lower hardness (0-30) = reduced damage output
```

**Source:** `code/code/obj/obj_base_weapon.cc` (lines 399-456)

### Hardness Hierarchy (Weapon Materials)

| Material | Hardness | Damage Effectiveness |
|----------|----------|----------------------|
| Diamond | 100 | Maximum (+66% over baseline) |
| Corundum | 100 | Maximum |
| Adamantite | 95 | Excellent (+58%) |
| Stone | 95 | Excellent |
| Bone | 85 | Very Good (+42%) |
| Jade | 80 | Very Good |
| Mithril | 80 | Very Good |
| Sapphire | 70 | Good (+17%) |
| Steel | 70 | Good |
| Iron | 60 | Baseline (standard) |
| Emerald | 75 | Good (+25%) |
| Bronze | 50 | Below average (-17%) |
| Copper | 35 | Poor (-42%) |
| Bone | 40 | Poor (-33%) |
| Leather | 25 | Very poor (-58%) |
| Wood | 12 | Minimal (-80%) |

### Iron Flesh Skill Integration

The Iron Flesh skill scales with material hardness:

```cpp
// Iron Flesh skill grants body hardness based on iron
int ironFleshHardness = (skillValue * material_nums[MAT_IRON].hardness) / 100;

// At 100% Iron Flesh skill: +60 hardness (iron's hardness)
// Body part hardness affects received damage
```

## Armor Protection Calculations

Armor effectiveness scales with material hardness:

```cpp
// Armor bonus calculation
float armorBonus = (hardness / 100.0) * baseACBonus;
```

### Armor Material Effectiveness

| Material | Hardness | AC Effectiveness | Typical Use |
|----------|----------|------------------|-------------|
| Adamantite | 95 | 95% | Legendary plate armor |
| Mithril | 80 | 80% | Elite light armor |
| Steel | 70 | 70% | Standard plate/chain |
| Iron | 60 | 60% | Basic metal armor |
| Bronze | 50 | 50% | Early-game armor |
| Tough Leather | 30 | 30% | Reinforced hide |
| Leather | 25 | 25% | Basic armor |
| Cloth | 5 | 5% | Robes/clothing |

## Durability and Structure System

### Structure Points

Objects have two structure values tracked in the database:

```sql
-- From obj table schema
`max_struct` int(11) NOT NULL,        -- Maximum durability
`cur_struct` int(11) NOT NULL,        -- Current durability
```

**Source:** `_Setup-data/sql_tables/sneezy/obj.sql`

### Degradation Formulas

Material susceptibilities determine wear rates:

```cpp
// Weapon degradation from combat
damage_to_structure = (base_weapon_damage * enemy_defense) / hardness;

// Armor degradation from attacks
armor_damage = (incoming_damage * material_susceptibility) / hardness;

// Water/rust damage over time
if (material_nums[material].water_susc > 100) {
    structure_loss += water_damage_per_tick;
}

// Fire damage to burning objects
if (material_nums[material].flammability > 0 && isObjStat(ITEM_BURNING)) {
    structure_loss += fire_damage_per_tick;
}
```

### Breakage Thresholds

| Structure % | State | Effects |
|-------------|-------|---------|
| 0% | Broken | Object non-functional, cannot be used |
| 1-25% | Badly damaged | Severely reduced effectiveness, obvious damage |
| 26-50% | Damaged | Degraded performance, visible wear |
| 51-75% | Worn | Slight performance reduction |
| 76-100% | Good/Perfect | Full functionality |

## Special Material Behaviors

### Flammable Materials

Materials with `flammability > 0` can catch fire:

```cpp
if (material_nums[getMaterial()].flammability) {
    setBurning(tLunatic);  // Object ignites
}
```

**Source:** `code/code/obj/obj_base_corpse.cc` (line 388)

#### Flammability Scale

| Flammability | Category | Materials |
|--------------|----------|-----------|
| 1000 | Extremely | Paper, Straw |
| 900 | Highly | Cloth, Hemp |
| 800 | Very | Wool, Fur |
| 600 | Moderately | Tough Cloth |
| 500 | Low | Wood |
| 400 | Slight | Leather |
| 0 | Fireproof | All metals, glass, stone, gems |

### Rust-Prone Materials

Materials with `water_susc > 100` corrode in water:

| Material | water_susc | Rust Behavior |
|----------|------------|---------------|
| Iron | 101 | Moderate rust over time |
| Steel | 101 | Moderate rust |
| Copper | 101 | Develops patina |
| Bronze | 101 | Tarnishes |
| Brass | 101 | Oxidizes |
| Mithril | 0 | Never rusts (magical) |
| Adamantite | 0 | Never corrodes |
| Gold | 101 | Does not rust but can corrode |
| Silver | 101 | Tarnishes |

**Effect:** Objects left in water or damp environments slowly lose structure points.

### Elemental Materials

Special elemental materials with unique properties:

```cpp
// Water Elemental (MAT_WATER = 57)
{0, 0, 0, 0, 0, 249, 105, 0, -5, 1, 0, 0, 0, 8, repairSpiritual, "water"}
// - water_susc: 249 (maximum)
// - float_weight: 0 (neutral buoyancy)
// - noise: -5 (silent)

// Fire Elemental (MAT_FIRE = 58)
{0, 0, 0, 0, 10, 249, 0, 0, 13, 1, 0, 0, 0, 15, repairSpiritual, "fire"}
// - burned_susc: 0 (immune to fire)
// - fall_susc: 0 (no impact damage)
// - noise: 13 (crackling sound)

// Ice (MAT_ICE = 61)
{101, 50, 0, 0, 0, 205, 109, 0, 0, 1, 0, 0, 95, 8, repairSpiritual, "ice"}
// - hardness: 0 (very fragile)
// - water_susc: 205 (melts in heat)
// - smash_susc: 50 (shatters)

// Lightning (MAT_LIGHTNING = 62)
{0, 0, 0, 0, 0, 249, 0, 0, 13, 1, 1, 0, 0, 15, repairSpiritual, "lightning"}
// - conductivity: 1 (always conductive)
// - noise: 13 (crackling sound)
```

### Magical Materials

Legendary materials with enhanced properties:

```cpp
// Mithril (MAT_MITHRIL = 160)
{0, 0, 0, 0, 80, 0, 51, 0, 0, 1, 0, 0, 0, 150, repairMetal, "mithril"}
// - hardness: 80 (excellent)
// - water_susc: 0 (never rusts)
// - price: 150 (very expensive)

// Adamantite (MAT_ADAMANTITE = 161)
{0, 0, 0, 0, 95, 0, 50, 0, 0, 1, 0, 0, 0, 90.9, repairMetal, "adamantite"}
// - hardness: 95 (near-maximum)
// - water_susc: 0 (never corrodes)
// - price: 90.9 (legendary)
```

### Organic vs Inorganic

**Organic materials** (subject to decay):
- Leather, Fur, Feathered, Wool
- Wood, Straw, Hemp
- Human Flesh, Ogre Hide
- Food items

**Effects:**
- Degrade in water (rot)
- Can be consumed by creatures
- Subject to fungal growth

**Inorganic materials** (stable):
- All metals
- All gems and minerals
- Glass, Stone, Clay
- Elemental materials

**Effects:**
- Do not rot
- Corrode or rust instead
- Cannot be eaten

## Tier-Based Material Categorization

### Rarity Tiers

Materials are classified into four rarity tiers per category:

```cpp
enum MetalTierT {
    METAL_TIER_COMMON,
    METAL_TIER_UNCOMMON,
    METAL_TIER_RARE,
    METAL_TIER_LEGENDARY,
};
```

Similar enums exist for: Hide, Wood, Organic, Dead, Rock, Crystal, Magical, Spiritual, Generic.

**Source:** `code/code/misc/materials.h` (lines 147-215)

### Tier Assignments by Category

#### Metals

| Tier | Materials | Hardness Range |
|------|-----------|----------------|
| COMMON | Copper, Bronze, Brass | 35-50 |
| UNCOMMON | Iron, Steel | 60-70 |
| RARE | Mithril, Silver, Gold | 80+ |
| LEGENDARY | Adamantite, Eternium | 95+ |

#### Hides/Leather

| Tier | Materials | Protection |
|------|-----------|------------|
| COMMON | Leather, Wool | 25 |
| UNCOMMON | Tough Leather | 30 |
| RARE | Dragon Scale | 60 |
| LEGENDARY | Ogre Hide | 45 |

#### Gems/Minerals

| Tier | Materials | Hardness | Value |
|------|-----------|----------|-------|
| COMMON | Stone, Bone | 40-95 | 0.05-2 |
| UNCOMMON | Jade, Amber | 60-80 | 70-120 |
| RARE | Emerald, Ruby, Sapphire | 70-90 | 150-195 |
| LEGENDARY | Diamond | 100 | 250 |

## Crafting System Integration

### Crafting Material Structures

Each material category has a crafting structure:

```cpp
struct MetalMaterial {
    int matNum;           // Material constant (MAT_IRON, etc.)
    const char* name;     // Display name ("iron", "steel")
    MetalTierT tier;      // Rarity tier
    int difficultyMod;    // Subtracted from skill check (higher = harder)
    int structureMod;     // Affects finished item durability
    int levelMod;         // Level requirement modifier
    int statMod;          // Stat bonuses on finished item
    int sharpnessMod;     // Sharpness bonus (weapons only)
    int matReq;           // Raw material vnum requirement
};
```

**Source:** `code/code/misc/materials.h` (lines 220-329)

Parallel structures exist for: HideMaterial, WoodMaterial, OrganicMaterial, DeadMaterial, RockMaterial, CrystalMaterial, MagicalMaterial, SpiritualMaterial, GenericMaterial.

### Crafting Arrays

Static constexpr arrays hold crafting data:

```cpp
static constexpr MetalMaterial metals[] = { ... };
static constexpr HideMaterial hides[] = { ... };
static constexpr WoodMaterial woods[] = { ... };
// ... etc.
```

**Source:** `code/code/misc/materials.h` (lines 332-527)

### Material Lookup Functions

```cpp
// Find by material number
MetalMaterial* findMetalMaterial(int matNum);
HideMaterial* findHideMaterial(int matNum);
// ... for each category

// Find by name string
MetalMaterial* findMetalMaterialByName(const sstring& name);
HideMaterial* findHideMaterialByName(const sstring& name);
// ... for each category
```

**Source:** `code/code/misc/materials.h` (lines 532-729)

### Crafting Calculations

Material modifiers affect crafting difficulty and output quality:

```cpp
// Skill check difficulty
crafting_difficulty = base_difficulty + material->difficultyMod;
// Higher difficultyMod = harder to work with

// Finished item durability
item_max_structure = base_structure + material->structureMod;
// Higher structureMod = more durable items

// Character level requirement
crafting_level_req = base_level + material->levelMod;
// Higher levelMod = needs higher level crafter

// Stat bonuses on finished item
item_stat_bonus = material->statMod;
// Magical materials grant stat bonuses

// Weapon sharpness (metals only)
weapon_sharpness = material->sharpnessMod;
// Harder metals hold edge better
```

### Crafting Example: Steel Longsword

```cpp
// Steel material properties
Material: MAT_STEEL (159)
Tier: UNCOMMON
difficultyMod: +10 (harder than iron)
structureMod: +15 (more durable)
levelMod: +5 (requires level 15+ smith)
statMod: +2 (grants +2 to stats)
sharpnessMod: +10 (holds edge well)

// Crafting calculation
base_difficulty = 50
final_difficulty = 50 + 10 = 60 (moderate skill check)

base_structure = 100
final_structure = 100 + 15 = 115 (15% more durable)

base_sharpness = 50
final_sharpness = 50 + 10 = 60 (better damage)
```

## Economic Value System

### Base Material Prices

From `material_nums[].price` (gold per unit):

| Tier | Material | Price | Relative Value |
|------|----------|-------|----------------|
| Legendary | Diamond | 250 | 5000× paper |
| Legendary | Ruby | 195.5 | 3910× paper |
| Legendary | Emerald | 184.6 | 3692× paper |
| Rare | Dragon Scale | 157.5 | 3150× paper |
| Rare | Dragonbone | 153 | 3060× paper |
| Rare | Mithril | 150 | 3000× paper |
| Rare | Sapphire | 150 | 3000× paper |
| Rare | Opal | 159 | 3180× paper |
| Rare | Pearl | 171 | 3420× paper |
| Rare | Adamantite | 90.9 | 1818× paper |
| Uncommon | Gold | 30 | 600× paper |
| Uncommon | Silver | 11.112 | 222× paper |
| Uncommon | Bronze | 9.1 | 182× paper |
| Uncommon | Steel | 8.334 | 167× paper |
| Common | Iron | 5 | 100× paper |
| Common | Leather | 2 | 40× paper |
| Common | Cloth | 1 | 20× paper |
| Common | Wood | 0.5 | 10× paper |
| Common | Copper | 0.5 | 10× paper |
| Common | Stone | 0.05 | 1× paper |
| Common | Paper | 0.06 | 1× (baseline) |

### Shop Pricing

Final object prices multiply material value:

```cpp
// Shop price calculation
final_price = object_base_value * material_nums[material].price;

// Shop markup/discount
shop_adjusted_price = final_price * shop_profit_multiplier;

// Charisma modifier (player buying)
charisma_factor = 1.0 to 1.3;  // Based on CHA stat
```

**Source:** `code/code/obj/obj_armor_wand.cc` (line 61)

### Value Classification

| Price Range | Classification | Example Materials |
|-------------|----------------|-------------------|
| 100+ gold | Legendary | Mithril, Adamantite, Precious gems |
| 10-99 gold | Rare | Gold, Silver, Bronze, Steel |
| 1-9 gold | Common | Iron, Leather, Cloth |
| < 1 gold | Mundane | Wood, Stone, Paper |

## Object Material Assignment

### Database Schema

```sql
CREATE TABLE `obj` (
  `vnum` int(11) NOT NULL,
  -- ... other columns ...
  `material` int(11) NOT NULL,          -- Material type (MAT_* constant)
  `max_struct` int(11) NOT NULL,        -- Max durability
  `cur_struct` int(11) NOT NULL,        -- Current durability
  PRIMARY KEY (`vnum`)
);
```

**Source:** `_Setup-data/sql_tables/sneezy/obj.sql`

### Material API

```cpp
// TBeing/TObj methods (defined in thing.h)
unsigned short getMaterial() const;
void setMaterial(unsigned short num);
const material_type_numbers* getMaterialTypeNumbers() const;
```

**Source:** `code/code/misc/thing.h` (lines 187-189)

### Default Material by Item Type

Objects are assigned default materials based on their type:

| Item Type | Default Materials |
|-----------|-------------------|
| ITEM_WEAPON | MAT_STEEL, MAT_IRON, MAT_BRONZE |
| ITEM_ARMOR | MAT_STEEL, MAT_IRON, MAT_LEATHER, MAT_TOUGH_LEATHER |
| ITEM_WORN | MAT_CLOTH, MAT_SILK, MAT_LEATHER |
| ITEM_BAG | MAT_LEATHER, MAT_CLOTH |
| ITEM_BOW | MAT_WOOD |
| ITEM_CORPSE | MAT_HUMAN_FLESH (species-specific) |
| ITEM_LIGHT | MAT_WOOD, MAT_METAL |
| ITEM_DRINKCON | MAT_GLASS, MAT_WOOD, MAT_METAL |
| ITEM_FOOD | MAT_FOODSTUFF |

## Repair System Integration

### Repair Function Pointers

Each material has a specialized repair function:

```cpp
// Repair functions (in material_nums array)
int (*repair_proc)(TBeing*, TObj*);

// Function assignments by material category
repairMetal()      // Metals: Blacksmith repair
repairWood()       // Wood: Carpenter repair
repairHide()       // Leather/hides: Leatherworker repair
repairDead()       // Bone/flesh: Necromancer repair
repairRock()       // Stone/gems: Stoneworker repair
repairCrystal()    // Crystals: Artificer repair
repairMagical()    // Magical materials: Enchanter repair
repairSpiritual()  // Elemental: Shaman repair
repairGeneric()    // Default: General repair
```

### Repair Difficulty

Repair difficulty scales with material properties:

```cpp
// Repair skill check
repair_difficulty = (100 - hardness) + (damage_percent / 2);

// High hardness materials = easier to repair
// Low structure = harder to repair
```

## Material Display and Descriptions

### Material Name Strings

Each material has a display name in the `mat_name` field (max 20 characters):

```cpp
material_nums[MAT_STEEL].mat_name = "steel";
material_nums[MAT_LEATHER].mat_name = "leather";
material_nums[MAT_MITHRIL].mat_name = "mithril";
```

### Object Descriptions with Materials

Materials appear in object descriptions:

```
a steel longsword
leather armor
mithril chainmail
an iron helmet
a wooden shield
```

The material adjective is typically prepended to the item's short description.

## Common Patterns

### Checking Material Properties

```cpp
// Get material property
int hardness = material_nums[obj->getMaterial()].hardness;

// Check flammability
if (material_nums[obj->getMaterial()].flammability > 0) {
    // Object can burn
}

// Check rust susceptibility
if (material_nums[obj->getMaterial()].water_susc > 100) {
    // Object will rust in water
}

// Get economic value
float value = material_nums[obj->getMaterial()].price;
```

### Setting Object Material

```cpp
// Assign material at creation
TObj* obj = read_object(vnum, VIRTUAL);
obj->setMaterial(MAT_STEEL);

// Change material
obj->setMaterial(MAT_MITHRIL);
```

### Material-Based Conditionals

```cpp
// Weapon effectiveness check
if (material_nums[weapon->getMaterial()].hardness < 50) {
    ch->sendTo("This weapon is too soft to be effective.\n");
    return FALSE;
}

// Armor durability check
if (material_nums[armor->getMaterial()].hardness < 40) {
    ch->sendTo("This armor is too fragile for combat.\n");
    return FALSE;
}

// Fire resistance check
if (material_nums[obj->getMaterial()].flammability == 0) {
    ch->sendTo("This object cannot burn.\n");
    return;
}
```

## Common Gotchas

### 1. Undefined Material

**Problem:** Objects without assigned material default to `MAT_UNDEFINED` (0), which has minimal properties.

```cpp
// BAD: No material assigned
TObj* obj = read_object(vnum, VIRTUAL);
// obj->getMaterial() returns 0 (MAT_UNDEFINED)

// GOOD: Always assign material
TObj* obj = read_object(vnum, VIRTUAL);
obj->setMaterial(MAT_STEEL);
```

### 2. Hardness Too Low

**Problem:** Setting hardness below 40 makes weapons nearly useless.

```cpp
// BAD: Weapon with hardness 5
material_nums[custom_mat].hardness = 5;
// Deals ~90% less damage than standard weapons

// GOOD: Minimum hardness 40 for weapons
material_nums[custom_mat].hardness = 40;
```

### 3. Invulnerable Armor

**Problem:** Setting all susceptibility values to 0 makes armor invincible.

```cpp
// BAD: Zero susceptibility everywhere
{0, 0, 0, 0, 100, 0, 0, 0, 0, 1, 0, 0, 0, 100, repairMetal, "invincium"}

// GOOD: Balanced susceptibilities
{10, 20, 5, 15, 85, 50, 60, 0, 5, 1, 1, 0, 30, 100, repairMetal, "mithril"}
```

### 4. Flammability Inconsistency

**Problem:** Setting metals as flammable or making paper fireproof.

```cpp
// BAD: Steel with flammability
material_nums[MAT_STEEL].flammability = 500;  // Nonsensical

// GOOD: Logical flammability
material_nums[MAT_STEEL].flammability = 0;    // Metals don't burn
material_nums[MAT_PAPER].flammability = 1000; // Paper highly flammable
```

### 5. Price Imbalance

**Problem:** Setting legendary materials cheaper than common materials.

```cpp
// BAD: Mithril cheaper than iron
material_nums[MAT_MITHRIL].price = 2;  // Less than iron (5)

// GOOD: Logical price progression
material_nums[MAT_IRON].price = 5;
material_nums[MAT_STEEL].price = 8.334;
material_nums[MAT_MITHRIL].price = 150;
```

### 6. Conductivity Misuse

**Problem:** Making non-metals conductive or metals non-conductive.

```cpp
// BAD: Wood conducts electricity
material_nums[MAT_WOOD].conductivity = 1;

// GOOD: Logical conductivity
material_nums[MAT_WOOD].conductivity = 0;   // Insulator
material_nums[MAT_IRON].conductivity = 1;   // Conductor
```

### 7. Volume Multiplier Extremes

**Problem:** Setting vol_mult to extreme values causes inventory issues.

```cpp
// BAD: Massive volume
material_nums[custom_mat].vol_mult = 50;  // Items take up entire inventory

// GOOD: Reasonable range
material_nums[custom_mat].vol_mult = 1;   // Standard (most materials)
material_nums[custom_mat].vol_mult = 3;   // Dense materials
```

## Key Source Files

| File | Lines | Purpose |
|------|-------|---------|
| `code/code/misc/materials.h` | 11-123 | Material constant definitions (MAT_*) |
| `code/code/misc/materials.h` | 125-142 | material_type_numbers struct |
| `code/code/misc/materials.h` | 147-215 | Tier enums for all categories |
| `code/code/misc/materials.h` | 220-329 | Crafting material structures |
| `code/code/misc/materials.h` | 332-527 | Crafting material arrays |
| `code/code/misc/materials.h` | 532-729 | Material lookup functions |
| `code/code/misc/constants.cc` | 840-1039 | material_nums[200] array initialization |
| `code/code/misc/thing.h` | 187-189 | Material API (get/set/query) |
| `code/code/misc/thing.h` | 52 | material_type member variable |
| `code/code/misc/thing.cc` | 106-124 | Weight calculation using vol_mult |
| `code/code/obj/obj_base_weapon.cc` | 399-456 | Weapon damage with hardness |
| `code/code/obj/obj_base_corpse.cc` | 388 | Flammability check |
| `code/code/obj/obj_armor_wand.cc` | 61 | Price calculation with material |
| `code/code/misc/obj.h` | 589, 591 | Structure point getters/setters |
| `_Setup-data/sql_tables/sneezy/obj.sql` | - | Database schema with material column |

## See Also

- [Object Types](object-types.md) - Item type system and TObj subclasses
- [Equipment Wear](equipment-wear.md) - How materials affect worn items
- [Combat Formulas](combat-formulas.md) - Damage calculations using hardness
- [Economy System](economy-system.md) - Shop pricing with material values
