---
title: Material Property System
description: Physical and economic properties for 83 materials affecting weapon damage, armor protection, durability, and value
keywords: [material, hardness, susceptibility, flammability, crafting, durability]
category: Understanding Systems
related: [object-system.md, equipment-wear.md, combat-formulas.md, economy-system.md]
last_updated: 2026-02-01
created_by_model: opus
source_files:
  - code/code/misc/materials.h
  - code/code/misc/constants.cc
  - code/code/misc/thing.h
  - code/code/misc/thing.cc
  - code/code/obj/obj_base_weapon.cc
  - code/code/obj/obj_base_corpse.cc
  - code/code/obj/obj_armor_wand.cc
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
| `noise` | short | -5 to +10 | Sound generation (stealth impact) |
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
| COMMON | Copper, Bronze, Brass | Leather, Wool | Stone, Bone |
| UNCOMMON | Iron, Steel | Tough Leather | Jade, Amber |
| RARE | Mithril, Silver, Gold | Dragon Scale | Emerald, Ruby, Sapphire |
| LEGENDARY | Adamantite, Eternium | Ogre Hide | Diamond |

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

## Implementation

### Data Structures

The `material_type_numbers` struct in `materials.h` holds 14 properties plus a repair function pointer and name string. The global `material_nums[200]` array in `constants.cc` stores all material definitions, with sparse allocation across the four category ranges.

Crafting uses parallel structures (`MetalMaterial`, `HideMaterial`, etc.) that add `difficultyMod`, `structureMod`, `levelMod`, `statMod`, `sharpnessMod`, and `matReq` fields. Static constexpr arrays hold crafting data with lookup functions by material number or name.

### Combat Calculations

Weapon damage multiplies by `hardness / 60.0` (iron baseline). The `getWeaponDamageBonus()` function in `obj_base_weapon.cc` applies this scaling. Armor protection uses `hardness / 100.0` as an effectiveness multiplier.

Susceptibility values reduce incoming damage by damage type. A material with `cut_susc = 50` takes half damage from slash attacks compared to `cut_susc = 100`.

### Degradation System

Objects track `max_struct` and `cur_struct` in the database. Degradation formulas divide incoming damage by hardness to determine structure loss. Water susceptibility >100 triggers periodic structure loss in water. Flammable objects lose structure while burning, scaling with flammability rating.

Structure thresholds: 0% = broken (non-functional), 1-25% = badly damaged, 26-50% = damaged, 51-75% = worn, 76-100% = good.

### Material API

`TThing` provides `getMaterial()`, `setMaterial()`, and `getMaterialTypeNumbers()` methods. The `material_type` member stores the material constant. Weight calculations in `getCarriedWeight()` incorporate `vol_mult` for density scaling.

### Elemental Materials

Water (`MAT_WATER`), Fire (`MAT_FIRE`), Ice (`MAT_ICE`), Lightning (`MAT_LIGHTNING`), and Chaos (`MAT_CHAOS`) have specialized properties: extreme susceptibilities or immunities, unique noise values, and `repairSpiritual()` for repair.

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

### Item Rusts Instantly

**Symptom:** Object loses structure rapidly in any water exposure.

**Cause:** `water_susc` set too high (approaching 249).

**Fix:** Standard rust-prone metals use 101. Values above 150 cause rapid corrosion. Rust-proof materials (mithril, adamantite) use 0.

### Crafted Item Has Wrong Stats

**Symptom:** Finished item stats don't match expected material bonuses.

**Cause:** Using `material_nums` properties instead of crafting material arrays.

**Fix:** Use `findMetalMaterial()` or equivalent lookup to get crafting-specific modifiers. Apply `structureMod`, `statMod`, and `sharpnessMod` from the crafting structure.

### Material Name Displays Wrong

**Symptom:** Object shows incorrect material adjective.

**Cause:** Material constant doesn't match `mat_name` string, or wrong constant assigned.

**Fix:** Verify material constant value matches intended entry in `material_nums`. Check `mat_name` field (20 char limit) in `constants.cc`.
