---
title: Character Foundation
description: Core character mechanics including class system, race system, and stats/attributes with multiclass mechanics and stat calculations
keywords: [CLASS_MAGE, CLASS_CLERIC, CLASS_WARRIOR, CLASS_THIEF, CLASS_SHAMAN, CLASS_DEIKHAN, CLASS_MONK, classIndT, race_t, Race, statTypeT, plotStat, getStat, hasClass, getLevel, body_t, baseStats, multiclass, mana, piety, lifeforce, stat layers, power law, racial modifiers]
category: Important Systems
created_by_model: opus
last_updated: 2026-02-01
source_files: [code/code/misc/being.cc, code/code/misc/race.cc, code/code/misc/stats.cc, code/code/misc/multiclass.cc]
related: [class-hierarchy.md, spell-skill-framework.md, experience-leveling.md, combat-formulas.md, equipment-wear.md]
---

# Character Foundation

## Overview

The character foundation consists of three interconnected systems that define what characters are and what they can do.

**Classes** define abilities and progression. A bitmask-based multiclass system allows combining up to nine classes, each tracking levels independently. Class determines resource pools (mana for mages, piety for clerics, lifeforce for shamans) and combat specialization.

**Races** define baseline capabilities and physical form. The 127 races span playable humanoids (6 choices) through animals, mythical creatures, and monsters. Each race provides unique stat distributions, body types governing equipment slots, natural immunities, and racial talents. A flyweight pattern shares single instances across all beings of each race.

**Stats** determine effectiveness at everything. Thirteen primary attributes use non-linear power-law scaling (exponent 1.4) that amplifies differences at extreme values. Stats flow through multiple layers: permanent racial baselines, player-allocated points, age modifiers, territory bonuses, and temporary equipment/spell effects.

The design philosophy separates concerns clearly: classes grant abilities (what you can do), races grant characteristics (what you are), stats determine effectiveness (how well you do it).

## Patterns

### Class Membership

Always use `hasClass()` for class checks rather than directly testing the bitmask. The method handles both partial matching (any overlap) and exact matching (all bits set).

Always use `getLevel()` with `classIndT` enum values for level lookups, not raw array indices. The enum provides compile-time safety and documents intent.

Always call `calcMaxLevel()` after any level change to maintain consistency of the `max_level` field.

Never assume class bitmask bit positions match array indices. Use `getClassIndNum()` to convert between representations.

### Race Queries

Always query racial characteristics through the `Race` pointer methods rather than caching results. The flyweight pattern ensures efficient lookup while maintaining single source of truth.

Always check `isHumanoid()`, `isAquatic()`, `isWinged()`, and similar characteristic methods through `TBeing` delegation rather than direct race access.

Never modify `Race` instances at runtime. They are shared across all beings of that race type.

### Stat Access

Always use `getStat()` with explicit `statSetT` layer specification. The layer determines which sources contribute.

Always use `STAT_CURRENT` for combat and immediate effect calculations. This includes equipment and spell modifiers.

Always use `STAT_NATURAL` for intrinsic capability checks and display. This excludes temporary effects.

Never modify `curStats` directly. Use `affectModify()` which maintains proper affect tracking.

### Multiclass Penalties

Always account for quadratic XP penalties when designing multiclass content. Two classes receive 25% XP per class, three classes receive 11% per class.

Always divide practice points by class count for multiclass characters.

Never grant experience without checking `howManyClasses()` to apply the correct divisor.

### Resource Management

Always check resource availability before deducting costs. Use `noMana()`, `noPiety()`, `noLifeforce()` predicates.

Always use the correct resource for each class: mana (mages), piety (clerics), lifeforce (shamans), movement (all).

Never assume hit points regenerate naturally. Unlike other resources, HP requires spells or rest.

### Stat Scaling

Always use `plotStat()` for stat-to-modifier conversion. The power-law curve is intentional design.

Always pass appropriate min/max/average parameters to `plotStat()`. Common patterns: (0.8, 1.25, 1.0) for multipliers, (5, 95, 25) for percentage chances.

Always use `getStatMod()` when applying stat modifiers with variable intensity. The multiplier parameter scales the effect.

Never assume linear stat scaling. Each point matters more at extremes than at the midpoint.

### Immunities

Always stack immunities additively: racial + equipment + spell. Cap at 100%.

Always handle negative immunity values as vulnerabilities (damage amplification).

Never bypass racial immunities for balance reasons. They are intentional racial identity.

## Reference

### The 9 Playable Classes

| Class | Constant | Bit | Value | Primary Discipline | HP/Level | Abbr |
|-------|----------|-----|-------|-------------------|----------|------|
| Mage | `CLASS_MAGE` | 0 | 1 | `DISC_MAGE` | 5.25 | M |
| Cleric | `CLASS_CLERIC` | 1 | 2 | `DISC_CLERIC` | 5.6 | C |
| Warrior | `CLASS_WARRIOR` | 2 | 4 | `DISC_WARRIOR` | 8.5 | W |
| Thief | `CLASS_THIEF` | 3 | 8 | `DISC_THIEF` | 5.6 | T |
| Shaman | `CLASS_SHAMAN` | 4 | 16 | `DISC_SHAMAN` | 5.25 | S |
| Deikhan | `CLASS_DEIKHAN` | 5 | 32 | `DISC_DEIKHAN` | 7.5 | D |
| Monk | `CLASS_MONK` | 6 | 64 | `DISC_MONK` | 5.25 | K |
| Ranger | `CLASS_RANGER` | 7 | 128 | `DISC_RANGER` | 4.9 | R |
| Commoner | `CLASS_COMMONER` | 8 | 256 | `DISC_ADVENTURING` | 5.0 | O |

### Level Constants

| Constant | Value | Description |
|----------|-------|-------------|
| `MAX_MORT` | 50 | Maximum mortal level |
| `GOD_LEVEL1` | 51 | First immortal level (builder) |
| `MAX_IMMORT` | 60 | Maximum level (administrator) |
| `MAX_CLASSES` | 9 | Number of class slots |
| `MAX_SAVED_CLASSES` | 11 | Array size for level storage |

### Playable Races Base Stats

| Stat | Human | Elven | Dwarf | Hobbit | Gnome | Ogre |
|------|-------|-------|-------|--------|-------|------|
| STR | 105 | 80 | 130 | 55 | 115 | 165 |
| BRA | 105 | 65 | 130 | 55 | 115 | 155 |
| CON | 105 | 45 | 155 | 80 | 135 | 125 |
| DEX | 105 | 130 | 85 | 155 | 105 | 80 |
| AGI | 105 | 125 | 85 | 130 | 105 | 80 |
| INT | 105 | 130 | 80 | 83 | 125 | 80 |
| WIS | 105 | 155 | 95 | 83 | 110 | 80 |
| FOC | 105 | 110 | 95 | 83 | 110 | 80 |
| PER | 105 | 115 | 100 | 115 | 110 | 80 |
| CHA | 105 | 125 | 85 | 105 | 100 | 75 |
| KAR | 105 | 105 | 105 | 105 | 105 | 95 |
| SPE | 105 | 125 | 55 | 155 | 95 | 80 |
| LUC | 105 | 105 | 105 | 105 | 105 | 105 |

### Racial Point Modifiers

| Race | hpMod | moveMod | manaMod |
|------|-------|---------|---------|
| Human | 0 | 0 | 0 |
| Elven | 0 | +25 | +1 |
| Dwarf | +1 | -20 | -1 |
| Hobbit | -1 | +40 | 0 |
| Gnome | 0 | 0 | 0 |
| Ogre | +4 | +40 | -1 |

### The 13 Primary Stats

| Stat | Enum | Primary Use | Combat Effect |
|------|------|-------------|---------------|
| STR | `STAT_STR` | Melee damage, carry capacity | 0.8x-1.25x damage multiplier |
| BRA | `STAT_BRA` | Physical resistance | Durability |
| CON | `STAT_CON` | Max HP, regeneration | 0.8x-1.25x HP regen |
| DEX | `STAT_DEX` | Combat accuracy, dodge | -67 to +84 attack bonus |
| AGI | `STAT_AGI` | Combat reactions | -2 to +4 reaction bonus |
| INT | `STAT_INT` | Spell success, mana | Practice efficiency |
| WIS | `STAT_WIS` | Spell damage, perception | 0.8x-1.25x spell damage |
| FOC | `STAT_FOC` | Spell concentration | Interruption resistance |
| PER | `STAT_PER` | Awareness, detection | Hidden object/trap finding |
| CHA | `STAT_CHA` | Shop prices, NPC reactions | 1.1x-1.3x price markup |
| KAR | `STAT_KAR` | Critical hit chance | 0.5%-2% crit base |
| SPE | `STAT_SPE` | Attack frequency | Urban +20 bonus |
| LUC | `STAT_LUC` | Random events | Critical outcomes |

### Stat Layers (statSetT)

| Layer | Source | Persistence |
|-------|--------|-------------|
| `STAT_RACE` | `race->baseStats` | Permanent (creation) |
| `STAT_CHOSEN` | `chosenStats` | Permanent (creation) |
| `STAT_TERRITORY` | `territory_adjustment()` | Permanent (creation) |
| `STAT_AGE` | `age_mod_for_stat()` | Dynamic (aging) |
| `STAT_NATURAL` | All permanent sources | Calculated |
| `STAT_CURRENT` | Natural + affects | Temporary |

### Territory Stat Adjustments

| Territory | STR | BRA | CON | INT | WIS | CHA | KAR | SPE |
|-----------|-----|-----|-----|-----|-----|-----|-----|-----|
| Urban | - | - | -20 | +20 | +20 | +20 | -20 | +20 |
| Villager | - | -10 | -10 | +10 | +10 | +10 | -10 | - |
| Plains | - | +5 | +5 | -10 | - | -5 | +5 | -5 |
| Recluse | - | +15 | +25 | -25 | -15 | -30 | +30 | - |
| Hill | - | +10 | +10 | -15 | -5 | -10 | +10 | - |
| Mountain | - | +15 | +20 | -20 | -15 | -20 | +20 | - |
| Forest | - | +15 | +15 | -15 | -15 | -15 | +15 | - |
| Mariner | - | +5 | +5 | -5 | -5 | -5 | +5 | - |

### Racial Characteristic Flags

| Flag | Effect | Example Races |
|------|--------|---------------|
| `DUMBANIMAL` | No intelligence, wild behavior | Wolf, Bear, Rat |
| `BONELESS` | Immune to bone breaks | Slime, Ooze |
| `WINGED` | Natural flight | Dragon, Eagle, Bat |
| `CLIMBER` | Wall climbing | Spider, Gecko |
| `EXTRAPLANAR` | Not prime material plane | Demon, Devil, Elemental |
| `AQUATIC` | Water-breathing | Fish, Shark, Octopus |
| `FOURLEGGED` | Four-legged anatomy | Horse, Dog, Lion |
| `COLDBLOODED` | Temperature-dependent | Snake, Lizard |
| `RIDABLE` | Can be mounted | Horse, Pegasus |
| `MAGICFLY` | Magical flight | Djinni, Efreet |
| `FEATHERED` | Bird-like | Bird, Griffon, Phoenix |

### Racial Talents

| Talent | Effect |
|--------|--------|
| `TALENT_FAST_REGEN` | Regeneration bonus |
| `TALENT_FISHEATER` | Bonus nutrition from fish |
| `TALENT_MEATEATER` | Bonus nutrition from meat |
| `TALENT_TATTOOED` | Tattoo spell capacity |
| `TALENT_GARBAGEEATER` | Bonus nutrition from trash |
| `TALENT_LIMB_REGROWTH` | Regrow severed limbs |
| `TALENT_INSECT_EATER` | Bonus nutrition from insects |
| `TALENT_FROGSLIME_SKIN` | Poison/acid resistance |
| `TALENT_MUSK` | Combat defensive ability |

### Lore Categories

| Lore | Value | Example Races |
|------|-------|---------------|
| `LORE_ANIMAL` | 0 | Wolf, Bear, Horse |
| `LORE_VEGGIE` | 1 | Treant, Fungus |
| `LORE_DIABOLIC` | 2 | Demon, Devil |
| `LORE_REPTILE` | 3 | Dragon, Snake |
| `LORE_UNDEAD` | 4 | Skeleton, Vampire |
| `LORE_GIANT` | 5 | Giant, Cyclops |
| `LORE_PEOPLE` | 6 | Human, Elven, Orc |
| `LORE_OTHER` | 7 | Slime, Elemental |

### Immunity Types

| Type | Description | Type | Description |
|------|-------------|------|-------------|
| `IMMUNE_HEAT` | Fire/heat | `IMMUNE_COLD` | Freezing |
| `IMMUNE_ACID` | Acid | `IMMUNE_POISON` | Poison |
| `IMMUNE_SLEEP` | Sleep spells | `IMMUNE_PARALYSIS` | Paralysis |
| `IMMUNE_CHARM` | Charm/domination | `IMMUNE_PIERCE` | Piercing |
| `IMMUNE_SLASH` | Slashing | `IMMUNE_BLUNT` | Blunt |
| `IMMUNE_ELECTRICITY` | Lightning | `IMMUNE_DISEASE` | Disease |
| `IMMUNE_DRAIN` | Life drain | `IMMUNE_FEAR` | Fear effects |
| `IMMUNE_HOLY` | Holy damage | `IMMUNE_SUMMON` | Summoning |

### Body Type Categories

| Category | Body Types | Equipment Effect |
|----------|------------|------------------|
| Humanoid | `BODY_HUMANOID`, `BODY_MINOTAUR` | Standard 24 slots |
| Hybrid | `BODY_CENTAUR`, `BODY_NAGA`, `BODY_SATYR` | Modified slots |
| Quadruped | `BODY_FOUR_LEG`, `BODY_FOUR_HOOF` | No hand slots |
| Winged | `BODY_BIRD`, `BODY_BAT`, `BODY_DRAGON` | Wing slots |
| Aquatic | `BODY_FISH`, `BODY_DOLPHIN`, `BODY_OCTOPUS` | Fins/tentacles |
| Serpentine | `BODY_SNAKE`, `BODY_HYDRA` | No limb slots |
| Exotic | `BODY_SLIME`, `BODY_GHOST`, `BODY_ORB` | Minimal slots |

### Class Query Methods

| Method | Returns | Purpose |
|--------|---------|---------|
| `getClass()` | `unsigned short` | Active class bitmask |
| `hasClass(bit)` | `bool` | Check class membership |
| `hasClass(bit, EXACT_YES)` | `bool` | Check exact class combination |
| `getLevel(classIndT)` | `int` | Level in specific class |
| `getMaxLevel()` | `int` | Highest level across all |
| `getClassLevel(bit)` | `int` | Level by bitmask |
| `howManyClasses()` | `int` | Count of active classes |
| `isSingleClass()` | `bool` | Exactly one class |
| `isDoubleClass()` | `bool` | Exactly two classes |
| `isTripleClass()` | `bool` | Three or more classes |
| `bestClass()` | `classIndT` | Highest-level class |
| `getProfName()` | `sstring` | Full class name(s) |
| `getProfAbbrevName()` | `sstring` | Abbreviated display |

### Stat Query Methods

| Method | Returns | Purpose |
|--------|---------|---------|
| `getStat(set, stat)` | `int` | Stat value from specified layer |
| `setStat(set, stat, val)` | `int` | Set stat value |
| `addToStat(set, stat, mod)` | `int` | Modify stat value |
| `plotStat(set, stat, min, max, avg)` | `T` | Power-law converted value |
| `getStatMod(stat, mult)` | `double` | Scaled stat modifier |
| `statSelfCheck(stat, bonus)` | `bool` | Percentage-based stat check |

### Race Query Methods

| Method | Returns | Purpose |
|--------|---------|---------|
| `getRace()` | `race_t` | Race enum value |
| `getMyRace()` | `Race*` | Race instance pointer |
| `isSameRace(ch)` | `bool` | Compare races |
| `isHumanoid()` | `bool` | Humanoid body type |
| `isAquatic()` | `bool` | Water-breathing |
| `isWinged()` | `bool` | Natural flight |
| `isFourLegged()` | `bool` | Quadruped |
| `isDumbAnimal()` | `bool` | Non-intelligent |
| `hasNoBones()` | `bool` | Boneless (slime etc) |

### Resource Methods

| Resource | Get | Max | Add | Check |
|----------|-----|-----|-----|-------|
| Hit Points | `getHit()` | `getMaxHit()` | `addToHit()` | - |
| Mana | `getMana()` | `getMaxMana()` | `addToMana()` | `noMana()` |
| Piety | `getPiety()` | `pietyLimit()` | `addToPiety()` | `noPiety()` |
| Lifeforce | `getLifeforce()` | `getMaxLifeforce()` | `addToLifeforce()` | `noLifeforce()` |
| Movement | `getMove()` | `getMaxMove()` | `addToMove()` | `tooTired()` |

### Source Files

| File | Purpose |
|------|---------|
| `code/code/misc/being.h` | TBeing class, resource accessors, level methods |
| `code/code/misc/being.cc` | TBeing implementations |
| `code/code/misc/multiclass.cc` | Class query/conversion, multiclass checks |
| `code/code/misc/defs.h` | Class constants, level limits |
| `code/code/misc/enum.h` | classIndT, statTypeT enums |
| `code/code/misc/constants.cc` | classInfo global array |
| `code/code/misc/discipline.h` | class_info struct |
| `code/code/misc/limits.cc` | Resource regeneration, XP penalties |
| `code/code/misc/race.h` | Race class, race_t enum |
| `code/code/misc/race.cc` | Race implementation, initRaces() |
| `code/code/misc/stats.h` | Stats class, statSetT enum |
| `code/code/misc/stats.cc` | getStat(), territory/age modifiers |
| `code/code/misc/extern.h` | plotValue() template |
| `code/code/sys/handler.cc` | affectModify() |
| `lib/races/` | Race definition files |

## Implementation

### Class Bitmask System

Classes use a 16-bit bitmask where each class occupies one bit. The `classIndT` enum maps to array indices (0-8), while `CLASS_*` constants are power-of-two bitmask values (1, 2, 4, 8...).

The `playerData` structure in `TBeing` stores class information: `level[MAX_SAVED_CLASSES]` tracks each class level, `max_level` caches the highest, and `Class` holds the active class bitmask. The `doneBasic[MAX_SAVED_CLASSES]` array tracks basic skill completion per class.

The `NumClasses()` function counts set bits to determine multiclass count. This count drives XP penalties (squared division) and practice point division.

### Class-Specific Resources

Mages use mana, an integer pool scaling with Intelligence. The `manaLimit()` method calculates maximum capacity, `manaGain()` handles regeneration per combat round. Resting and sleeping increase regeneration rate.

Clerics use piety, a double-precision floating-point value for fine-grained tracking. The piety costs in `spellInfo` are divided by 4 during initialization (PRAY_100 = 20 means actual cost 5.0). Piety scales with Wisdom and regenerates through worship and rest.

Shamans use lifeforce, an integer pool scaling with character level rather than stats. The `lifeforceLimit()` and `lifeforceGain()` methods parallel the mana interface.

Movement points are universal, consumed by travel and some special abilities. The pool size varies by race via `moveMod`. Movement drains faster when crawling or heavily encumbered.

Hit points do not regenerate naturally, distinguishing them from other resources. Recovery requires spells, potions, or extended rest.

### Class Combat AI

The `classStuff()` method in `TMonster` dispatches to class-specific AI functions based on `bestClass()`. Each function selects appropriate attacks based on situation, HP percentage, and available resources.

Warriors use `fighterMove()` for bash, bodyslam, spin kick, and disarm. Monks use `monkMove()` for springleap, hurl, bonebreak, and chi abilities. Thieves use `thiefMove()` for backstab and stab. Casters use their respective functions to select offensive or defensive spells.

### Race Flyweight Pattern

The `Races[MAX_RACIAL_TYPES]` global array holds 127 Race instances, one per race_t value. At boot, `initRaces()` creates all instances and loads configuration from `lib/races/` files.

Each `TBeing` stores only a `Race*` pointer to the shared instance. Query methods on TBeing delegate to the race: `isHumanoid()` calls `race->isHumanoid()`. This pattern minimizes memory with thousands of beings active.

Race instances are immutable at runtime. All racial properties (base stats, immunities, talents, body type) are fixed at load.

### Body Type and Equipment

The `body_t` enum defines 81 distinct anatomies. Body type determines which of 24 `wearSlotT` slots are available, which limbs can be targeted or severed, and movement/swimming behavior.

Humanoid bodies have full slot access. Quadrupeds lack hand slots. Aquatic bodies have fins instead of legs. Serpentine bodies have no limb slots at all. The equipment system queries body type to validate wear commands.

### Stat Layer Architecture

The `getStat()` method computes stat values from multiple layers. For `STAT_NATURAL`, it combines:
1. Racial baseline from `race->baseStats`
2. Player allocation from `chosenStats`
3. Age modifiers via `age_mod_for_stat()`
4. Territory bonuses via `territory_adjustment()`
5. Skill bonuses (e.g., SKILL_IRON_MUSCLES adds STR/8)
6. Vampire bonuses (+25 STR/SPE/CHA)

The `STAT_CURRENT` layer equals natural stats plus equipment and spell affects tracked in `curStats`. The `affectModify()` function maintains this layer as affects are applied and removed.

### Power-Law Stat Scaling

The `plotValue()` template implements non-linear stat conversion. Given a stat value in range 5-205, min/max/average output values, and power exponent (default 1.4), it computes output.

The midpoint (stat 105) always returns the average parameter. Below midpoint, output curves from minimum to average. Above midpoint, output curves from average to maximum. Power 1.4 creates acceleration: each stat point matters more at extremes than near the center.

The `plotStat()` method wraps `plotValue()` with the stat access layer. Common parameter sets: (0.8, 1.25, 1.0) for damage/regen multipliers, (5, 95, 25) for percentage-based skill checks.

### Age Modifier Curve

Physical stats peak in youth (ages 16-30) with +5 to +10 bonuses, then decline after 40. By age 80, STR/CON/AGI show -8 to -10 penalties. Mental stats (INT, WIS, FOC) remain stable longer, only declining slightly after 50.

Vampires bypass age modifiers entirely, receiving instead flat +25 bonuses to STR, SPE, and CHA regardless of age.

### Territory System

Character homeland selection at creation applies permanent stat modifiers. Urban backgrounds gain +20 INT/WIS/CHA/SPE but -20 CON/KAR. Recluse backgrounds gain +25 CON and +30 KAR but suffer -30 CHA. Mountain and forest backgrounds provide outdoor survival bonuses at the cost of social stats.

Territory modifiers stack with racial baselines and player allocations, calculated once and stored permanently.

### Immunity Stacking

Racial immunities define percentage resistance (0-100%) to 28 damage/effect types. Equipment immunities add to racial. Spell immunities add to the sum. Total is capped at 100%.

Negative values indicate vulnerabilities. A vampire with IMMUNE_HOLY -50 takes 50% extra damage from holy sources. Vulnerabilities are not capped and can exceed -100%.

### Multiclass Integration

Multiclass characters pay quadratic XP costs: `gain / howManyClasses() / howManyClasses()`. Two classes receive 25% XP per class; three classes receive 11%. This discourages extreme multiclassing while allowing meaningful hybrid builds.

Practice points divide by class count linearly. HP pools sum contributions from all classes weighted by their HP-per-level values, then apply CON modifier.

Resource pools depend on having the appropriate class levels. A mage/cleric has both mana (from mage levels + INT) and piety (from cleric levels + WIS), but the pools are smaller than pure builds due to split levels.

### Combat Integration

Attack accuracy combines class bonuses, racial DEX baseline, and `getDexReaction()` modifier (-67 to +84). Damage combines class specialization, racial STR baseline, and `getStrDamModifier()` multiplier (0.8x to 1.25x). Defense combines armor proficiency, racial immunities, and AGI/DEX modifiers.

The combat system reads `STAT_CURRENT` values, automatically incorporating equipment affects. Temporary stat buffs/debuffs immediately affect combat performance.

### Stat Check Methods

The `statSelfCheck()` method converts stats to percentage-based success probability: low stat (5) yields 5% chance, average (105) yields 25%, high (205) yields 95%. An additive bonus parameter shifts the curve.

Convenience wrappers (`isStrong()`, `isPerceptive()`, `isDextrous()`, etc.) call `statSelfCheck()` with their respective stat and no bonus.

## Troubleshooting

### Class Membership Misdetection

**Symptom:** Character incorrectly identified as having/lacking a class.

**Cause:** Using direct bitmask testing instead of `hasClass()`, or incorrect `exactTypeT` parameter.

**Diagnostic:** Log `getClass()` value and compare against expected bitmask. Check if EXACT_YES was used when EXACT_NO was intended.

**Fix:** Use `hasClass(CLASS_MAGE)` for "has any mage component" checks. Use `hasClass(CLASS_MAGE | CLASS_CLERIC, EXACT_YES)` only when requiring both classes.

### Stat Modifier Returning Wrong Value

**Symptom:** Combat bonuses or resource pools incorrect.

**Cause:** Using wrong `statSetT` layer (STAT_NATURAL when STAT_CURRENT needed, or vice versa).

**Diagnostic:** Compare `getStat(STAT_NATURAL, stat)` vs `getStat(STAT_CURRENT, stat)`. Difference indicates equipment/spell affects.

**Fix:** Use STAT_CURRENT for combat/immediate effects. Use STAT_NATURAL for display and intrinsic checks.

### Equipment Affects Not Applying

**Symptom:** Equipped items not modifying stats.

**Cause:** `affectModify()` not called during equip, or affects applied to wrong layer.

**Diagnostic:** Check `curStats` values before and after equip. Verify `affectModify()` calls in equipment code.

**Fix:** Ensure `equipChar()` calls `affectModify(loc, mod, bitv, true)` for each item affect. Verify removal calls with `false`.

### Racial Immunity Not Working

**Symptom:** Character taking full damage despite racial resistance.

**Cause:** Damage type mismatch (checking IMMUNE_HEAT for cold damage), or immunity not stacking correctly.

**Diagnostic:** Log `getImmunity()` return for the specific damage type. Verify racial data file has expected values.

**Fix:** Verify correct `immuneTypeT` for damage being dealt. Check if immunity is being bypassed (e.g., +3 weapons bypassing IMMUNE_PLUS2).

### Multiclass XP Incorrect

**Symptom:** Character gaining too much or too little XP.

**Cause:** Not applying squared division, or applying it inconsistently.

**Diagnostic:** Log `howManyClasses()` result and verify division applied twice.

**Fix:** In `gain_exp()`, ensure `gain /= howManyClasses()` appears twice consecutively.

### Age Modifiers Not Changing

**Symptom:** Physical stats not declining with age, or vampire not getting bonuses.

**Cause:** Age calculation using wrong base, or vampire check failing.

**Diagnostic:** Log `age()->year - getBaseAge() + 17` calculation. Verify `isVampire()` returns expected value.

**Fix:** Ensure age calculation matches expected formula. Check vampire affect flags are properly set.

### Resource Pool Zero or Negative

**Symptom:** Mana/piety/lifeforce showing 0 or crashing.

**Cause:** Resource modified without checking against negative, or limit calculation returning 0.

**Diagnostic:** Log `getMaxMana()` and verify positive. Check `getMana()` after each modification.

**Fix:** Always check `resource >= cost` before deducting. Ensure `addToMana()` clamps to 0 minimum and `manaLimit()` maximum.

### Body Type Equipment Mismatch

**Symptom:** Character unable to wear valid equipment, or wearing invalid slots.

**Cause:** Body type not matching expected for race, or slot availability not queried.

**Diagnostic:** Log `race->getBodyType()` and compare against expected. Check slot availability for that body type.

**Fix:** Verify race data file has correct body_t value. Query slot availability before wear attempt.

### Territory Bonus Missing

**Symptom:** Character not receiving homeland stat adjustments.

**Cause:** Territory not set at creation, or `territory_adjustment()` not being called.

**Diagnostic:** Log `player.hometerrain` value. Trace `getStat(STAT_NATURAL, ...)` to verify territory term included.

**Fix:** Ensure character creation sets `hometerrain`. Verify `STAT_NATURAL` calculation includes territory adjustment term.
