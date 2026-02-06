---
title: Material Property System
description: Physical and economic properties for 83 materials affecting weapon damage, armor protection, durability, and value
keywords: [hardness, susceptibility, flammability, crafting, durability]
category: informational
primary_symbols:
  functions: [getMaterial, setMaterial, getMaterialTypeNumbers, baseDamage, findMetalMaterial, findHideMaterial]
  classes: [material_type_numbers, TThing, MetalMaterial]
  enums: [materialTypeT, MAT_UNDEFINED, MAT_PAPER, MAT_CLOTH, MAT_WOOD, MAT_GLASS, MAT_LEATHER, MAT_FUR, MAT_DRAGON_SCALE, MAT_ICE, MAT_STONE, MAT_DIAMOND, MAT_RUBY, MAT_BONE, MAT_IRON, MAT_STEEL, MAT_MITHRIL, MAT_ADAMANTITE, MAT_WATER, MAT_FIRE, MAT_LIGHTNING, MAT_CHAOS, MetalTierT, HideTierT, WoodTierT, RockTierT, CrystalTierT, OrganicTierT, DeadTierT, MagicalTierT, SpiritualTierT, GenericTierT]
---

# Material Property System

## Overview

Every object has a material that determines its combat effectiveness, durability, and economic value. The system defines 83 materials organized into four categories (general, nature, mineral, metal), each with 14 properties governing physical behavior. Material hardness scales weapon damage and armor protection. Susceptibility values control degradation from damage types. Flammability and water susceptibility trigger environmental effects like burning and rusting.

## Patterns

### Material Assignment

- Always assign a material when creating objects; `MAT_UNDEFINED` has minimal properties
- Match material to item type: weapons need hardness 40+, armor needs balanced susceptibilities
- Use tier-appropriate materials: legendary items use legendary materials

### Property Balance

- Never set all susceptibilities to zero; creates invulnerable equipment
- Never set weapon material hardness below 40; damage becomes negligible
- Always maintain logical flammability: metals 0, paper/cloth 900+, leather 400
- Always set conductivity logically: metals 1, organics 0

### Durability Handling

- Check structure points before operations on damaged items
- Materials with `water_susc > 100` corrode in water; handle appropriately
- Flammable materials (`flammability > 0`) can ignite; check before fire exposure

### Crafting Integration

- Use `difficultyMod` to scale crafting difficulty by material quality
- Apply `structureMod` to finished item durability
- Enforce `levelMod` requirements for rare materials

## Reference

### Material Categories

| Category | Range | Count | Constants |
|----------|-------|-------|-----------|
| General | 0-19 | 20 | `MAT_PAPER`, `MAT_CLOTH`, `MAT_WOOD`, `MAT_GLASS` |
| Nature | 50-77 | 28 | `MAT_LEATHER`, `MAT_FUR`, `MAT_DRAGON_SCALE`, `MAT_ICE` |
| Mineral | 100-126 | 27 | `MAT_STONE`, `MAT_DIAMOND`, `MAT_RUBY`, `MAT_BONE` |
| Metal | 150-177 | 28 | `MAT_IRON`, `MAT_STEEL`, `MAT_MITHRIL`, `MAT_ADAMANTITE` |

Category ranges leave intentional gaps for expansion: 20-49, 78-99, 127-149, 178-199 (117 unused slots).

### Property Ranges

| Property | Type | Range | Purpose |
|----------|------|-------|---------|
| `cut_susc` | short | 0-100 | Slash damage vulnerability |
| `smash_susc` | short | 0-100 | Blunt damage vulnerability |
| `burned_susc` | short | 0-100 | Fire damage vulnerability |
| `pierced_susc` | short | 0-100 | Pierce damage vulnerability |
| `hardness` | short | 0-100 | Damage output / armor protection |
| `water_susc` | unsigned short | 0-249 | Corrosion rate (>100 = rusts) |
| `fall_susc` | unsigned short | 0-249 | Impact breakage chance |
| `float_weight` | unsigned short | 0-255 | Buoyancy (0=sinks, 255=floats) |
| `noise` | short | -5 to 63 | Sound generation (stealth impact) |
| `vol_mult` | unsigned short | 1-8 | Density / volume scaling |
| `conductivity` | unsigned short | 0-1 | Electrical conductivity |
| `flammability` | int | 0-1000 | Fire ignition rating |
| `acid_susc` | unsigned short | 0-100 | Acid vulnerability |
| `price` | float | 0.05-250 | Base economic value (gold/unit) |

### Hardness by Material

| Material | Hardness | Damage Effect |
|----------|----------|---------------|
| Diamond, Corundum | 100 | +66% |
| Adamantite, Stone | 95 | +58% |
| Bone | 85 | +42% |
| Mithril, Jade | 80 | +33% |
| Emerald | 75 | +25% |
| Steel, Sapphire | 70 | +17% |
| Iron | 60 | Baseline |
| Bronze | 50 | -17% |
| Bone (processed) | 40 | -33% |
| Copper | 35 | -42% |
| Leather | 25 | -58% |
| Wood | 12 | -80% |

### Flammability Scale

| Rating | Category | Materials |
|--------|----------|-----------|
| 1000 | Extreme | Paper, Straw |
| 900 | High | Cloth, Hemp |
| 800 | Very | Wool, Fur |
| 600 | Moderate | Tough Cloth |
| 500 | Low | Wood |
| 400 | Slight | Leather |
| 0 | Fireproof | Metals, Glass, Stone, Gems |

### Rarity Tiers

| Tier | Metals | Hides | Gems |
|------|--------|-------|------|
| COMMON | Copper, Bronze, Brass | Wool | Stone, Bone |
| UNCOMMON | Silver, Gold | Leather, Tough Leather | Jade, Amber |
| RARE | Iron, Steel | Dragon Scale | Emerald, Ruby, Sapphire |
| LEGENDARY | Adamantite, Eternium, Mithril | Ogre Hide | Diamond |

### Price Tiers

| Range | Classification | Examples |
|-------|----------------|----------|
| 100+ | Legendary | Diamond (250), Ruby (195), Mithril (150) |
| 10-99 | Rare | Adamantite (90.9), Gold (30), Silver (11) |
| 1-9 | Common | Iron (5), Leather (2), Cloth (1) |
| <1 | Mundane | Wood (0.5), Stone (0.05), Paper (0.06) |

### Repair Functions

| Function | Materials |
|----------|-----------|
| `repairMetal()` | All metals |
| `repairWood()` | Wood, Ebony |
| `repairHide()` | Leather, Fur, Hides |
| `repairDead()` | Bone, Flesh |
| `repairRock()` | Stone, Gems |
| `repairCrystal()` | Crystal, Glass |
| `repairMagical()` | Magical materials |
| `repairSpiritual()` | Elemental materials |

### Default Materials by Item Type

| Item Type | Default Materials |
|-----------|-------------------|
| ITEM_WEAPON | `MAT_STEEL`, `MAT_IRON`, `MAT_BRONZE` |
| ITEM_ARMOR | `MAT_STEEL`, `MAT_IRON`, `MAT_LEATHER` |
| ITEM_WORN | `MAT_CLOTH`, `MAT_SILK`, `MAT_LEATHER` |
| ITEM_BOW | `MAT_WOOD` |
| ITEM_CORPSE | `MAT_HUMAN_FLESH` (species-specific) |
| ITEM_FOOD | `MAT_FOODSTUFF` |

### Crafting Structure Fields

| Field | Purpose |
|-------|---------|
| `matNum` | Material type constant (array index) |
| `name` | Display name for crafting menus |
| `tier` | Rarity classification (COMMON=0, UNCOMMON=1, RARE=2, LEGENDARY=3) |
| `difficultyMod` | Subtracted from skill check (higher = harder) |
| `structureMod` | Added to finished item `max_struct` |
| `levelMod` | Added to minimum crafter level requirement |
| `statMod` | Bonus applied to finished item statistics |
| `sharpnessMod` | Added to weapon sharpness (metals only) |
| `matReq` | Object vnum for raw material requirement |

### Tier Enumerations

| Enum | Example Values |
|------|----------------|
| `MetalTierT` | METAL_TIER_COMMON (copper), METAL_TIER_LEGENDARY (adamantite) |
| `HideTierT` | HIDE_TIER_COMMON (leather), HIDE_TIER_RARE (dragon scale) |
| `WoodTierT` | WOOD_TIER_COMMON (generic), WOOD_TIER_RARE (ebony) |
| `RockTierT` | ROCK_TIER_COMMON (stone), ROCK_TIER_LEGENDARY (diamond) |
| `CrystalTierT` | CRYSTAL_TIER_COMMON (generic), CRYSTAL_TIER_RARE (special) |

Also: `OrganicTierT`, `DeadTierT`, `MagicalTierT`, `SpiritualTierT`, `GenericTierT` follow the same four-tier pattern.

### Material API

| Function | Purpose |
|----------|---------|
| `getMaterial()` | Returns material type constant (unsigned short) |
| `setMaterial(unsigned short)` | Assigns material; validates bounds |
| `getMaterialTypeNumbers()` | Returns const pointer to property structure |
| `findMetalMaterial(int matNum)` | Lookup crafting metal by material constant |
| `findMetalMaterialByName(const sstring&)` | Lookup crafting metal by name (case-insensitive) |

Category-specific lookup functions: `findHideMaterial()`, `findWoodMaterial()`, `findRockMaterial()`, etc.

### Database Schema

| Column | Type | Purpose |
|--------|------|---------|
| `obj.material` | int | Material type constant (not nullable, defaults to MAT_UNDEFINED) |
| `obj.max_struct` | int | Maximum structure points (pristine condition) |
| `obj.cur_struct` | int | Current structure points (breaks at 0) |

## Implementation

### Data Structures

The `material_type_numbers` struct in `materials.h` holds 14 properties plus a repair function pointer and name string. The global `material_nums[200]` array in `constants.cc` stores all material definitions, with sparse allocation across the four category ranges.

Crafting uses parallel structures (`MetalMaterial`, `HideMaterial`, etc.) that add `difficultyMod`, `structureMod`, `levelMod`, `statMod`, `sharpnessMod`, and `matReq` fields. Static constexpr arrays hold crafting data with lookup functions by material number or name.

### Combat Calculations

Weapon damage multiplies by `hardness / 60.0` (iron baseline). The `baseDamage()` function applies this by multiplying `damageLevel() * 1.75`. Armor protection uses `hardness / 100.0` as an effectiveness multiplier.

Susceptibility values reduce incoming damage by damage type. A material with `cut_susc = 50` takes half damage from slash attacks compared to `cut_susc = 100`.

### Degradation System

Objects track `max_struct` and `cur_struct` in the database. Degradation formulas divide incoming damage by hardness to determine structure loss. Water susceptibility >100 triggers periodic structure loss in water. Flammable objects lose structure while burning, scaling with flammability rating.

Structure thresholds: 0% = broken (non-functional), 1-25% = badly damaged, 26-50% = damaged, 51-75% = worn, 76-100% = good.

### Buoyancy System

The `float_weight` property determines water behavior: 0 sinks immediately, 1-100 sinks slowly, 101-200 neutral buoyancy, 201+ floats. Metals use 0, wood typically 150-200.

### Noise and Stealth

The `noise` property affects stealth checks. Negative values provide bonuses, positive values penalize (metal armor 15-63). MAT_WATER has noise=3. Total noise sums across all equipped items.

### Conductivity

Materials with `conductivity = 1` (all metals) transmit lightning damage through equipped items. Insulators (`conductivity = 0`) block transmission.

### Material API

`TThing` provides `getMaterial()`, `setMaterial()`, and `getMaterialTypeNumbers()` methods. The `material_type` member stores the material constant. Weight calculations in `getCarriedWeight()` incorporate `vol_mult` for density scaling. The `setMaterial()` function clamps values to valid ranges, defaulting to MAT_UNDEFINED for out-of-bounds values.

### Elemental Materials

Water (`MAT_WATER`), Fire (`MAT_FIRE`), Ice (`MAT_ICE`), Lightning (`MAT_LIGHTNING`), and Chaos (`MAT_CHAOS`) have specialized properties: extreme susceptibilities or immunities, unique noise values, and `repairSpiritual()` for repair. MAT_WATER has `water_susc = 249` (water affinity, not vulnerability). MAT_FIRE has `burned_susc = 0` (fire immunity).

### Organic vs Inorganic Classification

Materials 50-77 (nature range) undergo organic decay: rot, fungal growth, consumption. Materials 100-177 (mineral/metal) experience inorganic degradation: rust, corrosion, oxidation. General materials (0-19) split based on source.

## Troubleshooting

### Weapon Deals No Damage

**Symptom:** Weapon hits but damage is negligible.

**Cause:** Material hardness below 40.

**Fix:** Verify material assignment with `getMaterial()`. Check `material_nums[mat].hardness`. Reassign to appropriate material with `setMaterial()`.

### Armor Takes No Damage

**Symptom:** Armor never degrades regardless of combat.

**Cause:** All susceptibility values set to zero.

**Fix:** Review material definition in `constants.cc`. Ensure at least one susceptibility is non-zero. Standard metals have 50+ for at least one damage type.

### Object Burns Unexpectedly

**Symptom:** Metal or stone object ignites.

**Cause:** Incorrect flammability value or wrong material assigned.

**Fix:** Check `material_nums[mat].flammability`. Metals, glass, and stone must be 0. Verify object's material matches intended type.

### Object Cannot Catch Fire

**Symptom:** Wooden or cloth item won't ignite.

**Cause:** Flammability set to zero.

**Fix:** Paper should have 1000, cloth 900, wood 500. Zero flammability prevents ignition entirely.

### Item Rusts Instantly

**Symptom:** Object loses structure rapidly in any water exposure.

**Cause:** `water_susc` set too high (approaching 249).

**Fix:** Standard rust-prone metals use 101. Values above 150 cause rapid corrosion. Rust-proof materials (mithril, adamantite) use 0.

### Item Never Rusts

**Symptom:** Metal object unaffected by water.

**Cause:** `water_susc` at or below 100.

**Fix:** Iron and steel should have 101. Only legendary metals (mithril, adamantite) should have 0.

### Crafted Item Has Wrong Stats

**Symptom:** Finished item stats don't match expected material bonuses.

**Cause:** Using `material_nums` properties instead of crafting material arrays.

**Fix:** Use `findMetalMaterial()` or equivalent lookup to get crafting-specific modifiers. Apply `structureMod`, `statMod`, and `sharpnessMod` from the crafting structure.

### Crafting Fails Despite Sufficient Skill

**Symptom:** Repeated crafting failures at high skill levels.

**Cause:** Extreme `difficultyMod` (+40 or more) or `levelMod` exceeds character level.

**Fix:** Check both modifiers. Effective difficulty is base + `difficultyMod`. Verify crafter level meets base requirement + `levelMod`.

### Material Name Displays Wrong

**Symptom:** Object shows incorrect material adjective.

**Cause:** Material constant doesn't match `mat_name` string, or wrong constant assigned.

**Fix:** Verify material constant value matches intended entry in `material_nums`. Check `mat_name` field (20 char limit) in `constants.cc`.

### Object Floats When It Should Sink

**Symptom:** Metal or heavy object floats on water.

**Cause:** `float_weight` set too high.

**Fix:** Metals should have 0 (sinks immediately). Wood typically 150-200. Values above 100 provide buoyancy.

### Lightning Damage Not Conducting

**Symptom:** Lightning attacks don't transmit through metal armor.

**Cause:** `conductivity` set to 0.

**Fix:** All metals should have `conductivity = 1`. Check combat system queries conductivity for equipped items specifically.

### Stealth Fails in Soft Materials

**Symptom:** Stealth penalties despite cloth/leather armor.

**Cause:** `noise` property too high.

**Fix:** Leather should have noise 5-10, cloth near 0. Metal armor 15-63. Sum total noise across equipment.

### Undefined Material Crash

**Symptom:** Crash or assertion failure during property lookup.

**Cause:** `getMaterial()` returns 0 (MAT_UNDEFINED) used as index without validation.

**Fix:** Check `getMaterial()` result before array indexing. Assign appropriate default material if undefined.

### Custom Material Assertion Failures

**Symptom:** Assertions trigger when using new material.

**Cause:** Property values outside valid ranges.

**Fix:** Validate: susceptibility 0-100, hardness 0-100, float_weight 0-255, vol_mult 1-8, noise -5 to 63.
