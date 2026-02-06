---
title: Weapon System
description: Weapon class hierarchy, damage formulas, sharpness mechanics, and dual wielding
category: important
keywords: [weapon maintenance, two-handed weapons, paired weapons, ranged weapons, firearms, ammunition, weapon specialization]
primary_symbols:
  functions: [getWeaponDam, swungObjectDamage, sharpenMe, dullMe, weaponLevel, specializationCheck]
  classes: [TBaseWeapon, TGenWeapon, TBow, TArrow, TGun, THandgonne]
  enums: [weaponT, damageTypeT, WEAPON_TYPE_SMASH, WEAPON_TYPE_SLASH, WEAPON_TYPE_WHIP, WEAPON_TYPE_STAB, WEAPON_TYPE_BITE, WEAPON_TYPE_SHOOT, ITEM_PAIRED, ITEM_SPIKED, ITEM_MAGIC, ITEM_WEAR_TAKE, ITEM_WEAR_HOLD, ITEM_WEAR_THROW, ITEM_ANTI_MAGE, ITEM_ANTI_CLERIC, ITEM_ANTI_WARRIOR, BOW_STRING_BROKE, BOW_CARVED, ARROW_FEATHERED, GUN_FLAG_SILENCED, GUN_FLAG_FOULED, AMMO_10MM_PISTOL, SKILL_DUAL_WIELD, SKILL_DUAL_WIELD_THIEF, SKILL_SLASH_PROF, SKILL_PIERCE_PROF, SKILL_BLUNT_PROF, SKILL_RANGED_PROF, SKILL_SLASH_SPEC, SKILL_PIERCE_SPEC, SKILL_BLUNT_SPEC, SKILL_2H_SPEC, SKILL_2H_SPEC_DEIKHAN, SKILL_SHARPEN, ITEM_WEAPON, ITEM_BOW, ITEM_ARROW, ITEM_GUN, ITEM_HANDGONNE]
---

# Weapon System

## Overview

How does a sword calculate its damage when it hits an enemy? What makes a mace better suited for certain situations than a dagger? The weapon system answers these questions through a class hierarchy, damage formulas, and maintenance mechanics that affect combat effectiveness.

Weapons in SneezyMUD are specialized objects that derive from a common base class and branch into categories: melee weapons, bows, arrows, firearms, and historical hand cannons. Each weapon tracks not just its damage potential but also its current condition through a sharpness system that degrades with use and can be restored through player skill.

The system supports multi-type weapons that can deal different kinds of damage with varying probabilities. A longsword might slash 70% of the time and thrust 30% of the time, with damage modified by character strength differently depending on which attack type lands. This creates tactical depth where weapon choice matters beyond raw numbers.

Dual wielding introduces secondary hand penalties that scale with skill proficiency. Two-handed weapons trade attack speed for damage multipliers. Ranged weapons require ammunition matching and have distance limitations. Each weapon category has distinct mechanics while sharing core damage and maintenance systems.

A typical combat interaction flows through these stages:
- **Before combat**: Weapon has full sharpness from recent maintenance, damage level set by quality and enchantments
- **During combat**: Each swing selects an attack type (slash/pierce/blunt) based on weighted frequencies, applies strength modifiers based on type category, factors in skill proficiency and specialization bonuses
- **After prolonged use**: Sharpness degrades from striking hard targets, weapon deals reduced damage until player restores it with the sharpen skill

## Patterns

### Weapon Type Classification

Always use the category check methods rather than inspecting weapon types directly. The `isBluntWeapon()`, `isSlashWeapon()`, and `isPierceWeapon()` methods implement the 2/3 rule using frequency-weighted comparison: a weapon is classified into a category when the sum of frequencies for matching attack types exceeds two-thirds of the total frequency (`count > (total / 3.0 * 2.0)`). A weapon with slash at frequency 70 and thrust at frequency 30 would be classified as slash because 70 > 66.7 (100 / 3 * 2). Direct inspection of weapon types can produce incorrect results for multi-type weapons.

When determining strength modifier application, check weapon category first. Blunt weapons receive full strength bonus. Slash weapons receive half the bonus above baseline. Pierce weapons receive one-third the bonus. Applying the wrong modifier produces noticeably incorrect damage values.

### Sharpness Management

Always check `getCurSharp() < getMaxSharp()` before allowing sharpening. Attempting to sharpen a weapon at maximum sharpness wastes movement points and produces confusing player feedback.

Never assume sharpness affects damage linearly. Sharpness contributes only 10% to the overall `weaponLevel` formula. Damage level (60%) and structure condition (30%) have far greater impact. Players who obsess over sharpness while ignoring weapon condition are optimizing the wrong variable.

Always use `sharpenMe()` rather than directly modifying sharpness values. The method handles movement point costs, skill checks, and proper notification. Direct modification bypasses game balance.

### Dual Wielding

Check `ITEM_PAIRED` flag to determine if weapons can be dual wielded together. Paired weapons are designed as matching sets and receive a 10% damage bonus. Non-paired weapons can still be dual wielded but miss this bonus.

Always apply secondary hand penalty through the proper formula: `amt = amt * 3 / 5 + 10`, where `amt` is the `SKILL_DUAL_WIELD` skill value (0-100). This produces a damage percentage of 10% at skill 0 and 70% at skill 100. Forgetting this penalty makes dual wielding overpowered at low skill levels.

### Ranged Weapons

Always check bow condition flags before allowing shooting. The `BOW_STRING_BROKE` flag prevents firing entirely. Ignoring this check allows impossible actions.

Always verify ammunition type matches weapon requirements. `getArrowType()` returns the required ammunition type; incompatible ammunition should fail to load or fire.

### Firearm Damage

Always apply the firearm damage penalty. Guns deal half damage compared to melee weapons with equivalent stats. This balances their advantages (rate of fire, range) against their disadvantages.

Check `GUN_FLAG_FOULED` before allowing reliable firing. Fouled guns have reduced accuracy and may misfire.

Rate of fire affects combat frequency. The combat system queries `getROF()` and grants floor(rof) guaranteed attacks plus a fractional chance for an additional attack. A gun with rof 2.5 grants 2 attacks plus 50% chance for a third attack per combat round.

### Object Factory Usage

Always use `makeNewObj()` with the correct `itemTypeT` to create weapons. Using the wrong type produces a weapon object that lacks the specialized members and methods its code will expect. Creating a `TGenWeapon` when you needed a `TBow` causes arrows to fail loading.

### Type Safety with Weapon Subclasses

Always use `dynamic_cast` when converting from `TBaseWeapon` to subclasses like `TGenWeapon`. The base weapon pointer might actually be a bow, arrow, or gun. Direct casting produces undefined behavior when the types don't match.

When checking for multi-type capabilities, cast to `TGenWeapon` first and verify the cast succeeded:

```
TGenWeapon* gw = dynamic_cast<TGenWeapon*>(weapon);
if (gw) {
    weaponT type = gw->getWeaponType();  // Safe: verified TGenWeapon
}
```

### Value Field Manipulation

Never directly manipulate the packed bitfields in val0-val3. Use the provided accessor methods: `setMaxSharp()`, `setCurSharp()`, `setWeapDamLvl()`, `setWeaponType()`. Direct bit manipulation risks corrupting adjacent packed values.

Always call `assignFourValues()` when loading from database and `getFourValues()` when saving. These methods handle the bit packing correctly.

## Reference

### Symbol Quick Reference

| Symbol | Type | Purpose |
|--------|------|---------|
| `TBaseWeapon` | class | Abstract base for all weapons |
| `TGenWeapon` | class | General melee weapons with multi-type support |
| `TBow` | class | Bows and crossbows requiring arrows |
| `TArrow` | class | Arrows and bolts with trap/poison support |
| `TGun` | class | Firearms requiring ammunition |
| `THandgonne` | class | Historical hand cannons extending TGun |
| `getWeaponDam()` | function | Calculate final weapon damage with all modifiers |
| `swungObjectDamage()` | function | Get weapon intrinsic damage before character modifiers |
| `sharpenMe()` | function | Increase weapon sharpness through skill use |
| `dullMe()` | function | Decrease weapon sharpness through wear |
| `weaponLevel()` | function | Calculate overall weapon effectiveness (60% damage, 30% structure, 10% sharp) |
| `specializationCheck()` | function | Return decimal bonus from specialization skill |
| `baseDamage()` | function | Return base damage with two-handed multipliers |
| `isBluntWeapon()` | function | Check if 2/3 of frequency-weighted attacks are blunt |
| `isSlashWeapon()` | function | Check if 2/3 of frequency-weighted attacks are slash |
| `isPierceWeapon()` | function | Check if 2/3 of frequency-weighted attacks are pierce |
| `statObjInfo()` | method | Format weapon stats for display including sharpness terminology |

### Weapon Type Categories

| Category | Attack Types | Strength Scaling |
|----------|--------------|------------------|
| Blunt | SMASH, CRUSH, BLUDGEON, PUMMEL, FLAIL, BEAT, THRASH, THUMP, WALLOP, BATTER, STRIKE, CLUB, POUND, SMITE | Full STR modifier |
| Slash | WHIP, SLASH, CLEAVE, CLAW, SLICE, BEAR_CLAW, SHRED | (STR - 1) / 2 + 1 |
| Pierce | STAB, BITE, STING, PIERCE, THRUST, SPEAR, BEAK | (STR - 1) / 3 + 1 |
| Special | AIR, EARTH, FIRE, WATER | Varies |
| Ranged | SHOOT, CANNON | Varies |

### Damage Formula Components

| Component | Weight | Formula |
|-----------|--------|---------|
| Damage level | 60% | `damLevel / 4.0` |
| Structure | 30% | `max(maxStructPoints - 10, 0) * 2.0 / 3.0` |
| Sharpness | 10% | `max(maxSharp - 10, 0) * 2.0 / 3.0` |

### Damage Multiplier Constants

| Constant | Value | Purpose |
|----------|-------|---------|
| PC base multiplier | 1.75x | Base damage balance multiplier for all PC weapons |
| Paired multiplier | 1.1x | Additional bonus for paired weapon sets (ITEM_PAIRED) |
| Secondary hand min | 10% | Minimum secondary hand damage (skill 0) |
| Secondary hand max | 70% | Maximum secondary hand damage (skill 100) |
| Firearm penalty | 0.5x | Damage divisor for firearms |

### Dual Wield Scaling

| Skill Level | Secondary Hand Damage |
|-------------|----------------------|
| 0 | 10% of primary |
| 25 | 25% of primary |
| 50 | 40% of primary |
| 75 | 55% of primary |
| 100 | 70% of primary |

### Weapon Skills

| Skill | Purpose |
|-------|---------|
| `SKILL_DUAL_WIELD` | General dual wield proficiency |
| `SKILL_DUAL_WIELD_THIEF` | Rogue-specific dual wield mastery |
| `SKILL_SLASH_PROF` | Slashing weapon proficiency |
| `SKILL_PIERCE_PROF` | Piercing weapon proficiency |
| `SKILL_BLUNT_PROF` | Blunt weapon proficiency |
| `SKILL_BAREHAND_PROF` | Unarmed fighting proficiency |
| `SKILL_RANGED_PROF` | Ranged weapon proficiency |
| `SKILL_SLASH_SPEC` | Warrior slashing specialization |
| `SKILL_PIERCE_SPEC` | Warrior piercing specialization |
| `SKILL_BLUNT_SPEC` | Warrior blunt specialization |
| `SKILL_BAREHAND_SPEC` | Monk unarmed specialization |
| `SKILL_RANGED_SPEC` | Ranger ranged specialization |
| `SKILL_2H_SPEC` | Warrior two-handed specialization |
| `SKILL_2H_SPEC_DEIKHAN` | Deikhan two-handed specialization |
| `SKILL_SHARPEN` | Weapon maintenance |

### Item Flags

| Flag | Purpose |
|------|---------|
| `ITEM_PAIRED` | Can be dual-wielded as matching set (10% bonus) |
| `ITEM_SPIKED` | Has spikes/barbs for extra damage |
| `ITEM_MAGIC` | Enchanted weapon |
| `ITEM_NODROP` | Cannot be dropped (cursed) |
| `ITEM_GLOW` | Emits light |
| `ITEM_HUM` | Makes noise |
| `ITEM_BLESS` | Blessed weapon |
| `ITEM_NORENT` | Cannot be saved to rent |
| `ITEM_BURNING` | Currently on fire |

### Weapon Wear Flags

| Flag | Purpose |
|------|---------|
| `ITEM_WEAR_TAKE` | Can be picked up |
| `ITEM_WEAR_HOLD` | Can be held/wielded |
| `ITEM_WEAR_THROW` | Can be thrown |

### Anti-Class Restriction Flags

| Flag | Purpose |
|------|---------|
| `ITEM_ANTI_MAGE` | Cannot be used by mages |
| `ITEM_ANTI_CLERIC` | Cannot be used by clerics |
| `ITEM_ANTI_WARRIOR` | Cannot be used by warriors |

### Bow Flags

| Flag | Effect |
|------|--------|
| `BOW_STRING_BROKE` | Cannot shoot |
| `BOW_CARVED` | Customized |
| `BOW_SCRAPED` | Roughed surface |
| `BOW_SMOOTHED` | Polished |

### Arrow Flags

| Flag | Effect |
|------|--------|
| `ARROW_FEATHERED` | Improved accuracy |
| `ARROW_CARVED` | Custom shaft |
| `ARROW_SCRAPED` | Surface prepared |
| `ARROW_SMOOTHED` | Reduced drag |

### Gun Flags

| Flag | Effect |
|------|--------|
| `GUN_FLAG_SILENCED` | Suppressed/quiet |
| `GUN_FLAG_CASELESS` | No shell casings |
| `GUN_FLAG_CLIPLESS` | Manually loaded |
| `GUN_FLAG_FOULED` | Dirty/damaged |

### Ammunition Types

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

### Key Files

| File | Purpose |
|------|---------|
| `obj/obj_base_weapon.h` | TBaseWeapon class definition |
| `obj/obj_base_weapon.cc` | Sharpness, damage, weaponLevel implementation |
| `obj/obj_general_weapon.h` | TGenWeapon class definition |
| `obj/obj_general_weapon.cc` | Multi-type weapons, value packing |
| `obj/obj_bow.h/cc` | Bow and crossbow implementation |
| `obj/obj_arrow.h/cc` | Arrow and projectile implementation |
| `obj/obj_gun.h/cc` | Firearm implementation |
| `obj/obj_handgonne.h/cc` | Historical hand cannon implementation |
| `misc/combat.cc` | `getWeaponDam()` and combat integration |
| `misc/enum.h` | `weaponT` enumeration |
| `misc/spells.h` | Weapon skill definitions |
| `sys/db.cc` | Object factory and database loading |

## Implementation

### Class Hierarchy

The weapon system uses inheritance to share common functionality while allowing specialized behavior for different weapon categories.

`TBaseWeapon` is the abstract base class containing sharpness tracking (maxSharp, curSharp), damage parameters (damLevel, damDev), and poison state. It provides the core damage calculation methods and sharpness maintenance interface.

`TGenWeapon` extends `TBaseWeapon` for general melee weapons. It adds support for up to three attack types with frequency weighting, enabling weapons like a longsword that might slash 70% of the time and thrust 30% of the time.

`TBow` extends `TBaseWeapon` for ranged weapons requiring arrows. It tracks arrow type requirements, bow condition flags, and maximum shooting range.

`TArrow` extends `TBaseWeapon` for projectiles. Despite being ammunition, arrows have their own sharpness and damage values. They also support trap integration with trap_level and trap_dam_type for poisoned or trapped arrows.

`TGun` extends `TBaseWeapon` for firearms. It tracks rate of fire, ammunition type, and gun condition flags.

`THandgonne` extends `TGun` for historical hand cannons, overriding loading and firing mechanics for slower but more damaging behavior.

### Value Storage

Weapon data is packed into four integer values for efficient storage and database persistence.

val0 stores sharpness as a 16-bit bitfield: bits 0-7 contain curSharp, bits 8-15 contain maxSharp. The `getCurSharp()` method extracts `val0 & 0xFF`. The `getMaxSharp()` method extracts `(val0 >> 8) & 0xFF`. Setters modify val0 through bitwise operations: clearing target bits and ORing in new values.

val1 stores damage as a 16-bit bitfield: bits 0-7 contain damLevel, bits 8-15 contain damDev (deviation for randomization).

val3 stores weapon types and frequencies for TGenWeapon as a 32-bit bitfield: bits 0-7 weapon_type[0], bits 8-15 wtype_frequency[0], bits 16-23 weapon_type[1], bits 24-31 wtype_frequency[1]. The third type/frequency pair uses additional storage.

The `assignFourValues()` and `getFourValues()` methods handle packing and unpacking. The `read_object()` function in db.cc calls these during database load.

### Damage Calculation Flow

The `getWeaponDam()` function in combat.cc orchestrates damage calculation through multiple stages.

First, it calls `swungObjectDamage()` to get the weapon's intrinsic damage. This method calls `baseDamage()` which applies a 1.75x base multiplier for all PC weapons and an additional 1.1x for paired weapons (ITEM_PAIRED), then adds random deviation from damDev.

Second, it applies dual wield penalty for secondary hand weapons. The formula is `amt = amt * 3 / 5 + 10` where amt is the SKILL_DUAL_WIELD value (0-100), producing a damage percentage from 10% (skill 0) to 70% (skill 100) of primary hand damage.

Third, it applies character strength modifier based on weapon category. Blunt weapons and unarmed attacks receive the full strength modifier. Slash weapons receive (strDam - 1) / 2 + 1, roughly halving the bonus. Pierce weapons receive (strDam - 1) / 3 + 1, roughly one-third the bonus.

Fourth, it applies weapon skill proficiency. Characters have automatic weapon learning equal to level times 2, or their actual skill value if higher, capped at 100.

Fifth, for guns specifically, it halves the damage to balance their ranged advantages.

Finally, incapacitated targets receive bonus damage: 3 times wepDam divided by 10, minimum 1.

### Multi-Type Attack Selection

TGenWeapon stores three weapon types with associated frequencies. The `getWeaponType()` method implements frequency-weighted random selection.

It sums all three frequencies to get a total. It rolls a random number from 0 to total minus 1. Based on where the roll falls in the cumulative frequency ranges, it returns weapon_type[0], [1], or [2].

This allows a weapon like a bastard sword to deal different damage types with controlled probabilities. Setting frequencies of 70, 30, 0 would make the weapon slash 70% of the time and thrust 30% of the time.

### Type Classification Logic

The `isBluntWeapon()`, `isSlashWeapon()`, and `isPierceWeapon()` methods implement the 2/3 rule for classifying multi-type weapons.

Each method iterates through all three weapon types, summing the frequencies of types that belong to its category. If the summed frequency exceeds two-thirds of the total frequency (`count > (total / 3.0 * 2.0)`), the weapon is classified as that type. This frequency-weighted approach means a weapon with slash at frequency 70 and blunt at frequency 30 is classified as slash, while one with slash at frequency 40 and blunt at frequency 60 is classified as blunt.

This matters because strength modifiers and certain special attacks depend on weapon category. A weapon with two slash types and one blunt type is classified as a slash weapon and receives the reduced strength bonus.

### Sharpness Mechanics

The sharpness system tracks weapon condition affecting damage output.

`sharpenMe()` handles the sharpening skill. It first verifies curSharp is below maxSharp. It consumes movement points based on weapon complexity. It performs a skill check against SKILL_SHARPEN learning. On success, it increments curSharp by 1-2 points, capped at maxSharp, and notifies the character.

`dullMe()` handles automatic wear. Weapons dull when striking hard targets like stone or metal, when parrying or blocking attacks, and from general combat duration. It decrements curSharp proportionally.

The effect on damage flows through `weaponLevel()`. This function calculates overall weapon effectiveness as: (damageLevel * 0.6) + (structLevel * 0.3) + (sharpLevel * 0.1). Sharpness contributes only 10% of the total, making it a minor but noticeable factor.

Display varies by weapon category: slash weapons show "sharpness", blunt weapons show "bluntness", pierce weapons show "pointiness". The `statObjInfo()` method formats this display along with damage level and structure points.

### Specialization System

Weapon specializations provide damage multipliers for characters who invest in specific weapon disciplines.

The `specializationCheck()` function returns a decimal bonus from 0.0 to 1.0+ based on the character's specialization skill value for the appropriate weapon type.

Specializations are class-gated: SKILL_SLASH_SPEC, SKILL_BLUNT_SPEC, and SKILL_PIERCE_SPEC are Warrior skills. SKILL_BAREHAND_SPEC is a Monk skill. SKILL_RANGED_SPEC is a Ranger skill.

Skills are organized into discipline trees defined in spell_info.cc: DISC_COMBAT contains all proficiencies and dual wield, while DISC_SLASH, DISC_BLUNT, and DISC_PIERCE contain their respective specializations.

### Two-Handed and Paired Weapons

All PC weapons receive a 1.75x base damage multiplier applied in `baseDamage()` as a balance formula for player character damage output.

Paired weapons (identified by the ITEM_PAIRED flag) receive an additional 1.1x multiplier. This is the specific bonus for two-handed and paired weapon sets that can be worn in both hands simultaneously, compensating for not being able to dual wield or use a shield.

Two-handed specialization skills (SKILL_2H_SPEC and SKILL_2H_SPEC_DEIKHAN) increase attack frequency rather than damage, providing a bonus of skillValue / 100.0 additional attacks per round.

### Combat-Specific Methods

TGenWeapon provides methods checking if weapons qualify for specific combat techniques.

`canCudgel()` requires a blunt weapon with volume at or below 1500. This enables heavy stunning blows.

`canBackstab()` requires a pierce weapon with volume at or below 1500. This enables finesse strikes from behind.

`canStab()` requires a pierce weapon with volume at or below 2000. This enables thrusting attacks with slightly larger weapons than backstab allows.

### Bow Mechanics

TBow tracks ammunition requirements through arrowType, condition through flags, and range through max_range.

The `shootMeBow()` method handles ranged attacks. It verifies the string isn't broken (BOW_STRING_BROKE), finds appropriate ammunition, calculates range and trajectory, and resolves the attack.

Bow flags track crafting state: BOW_CARVED, BOW_SCRAPED, and BOW_SMOOTHED indicate customization work that affects accuracy and durability.

### Arrow Mechanics

TArrow inherits weapon damage from TBaseWeapon while adding projectile-specific features.

Arrow damage decreases with distance traveled. Arrow condition (feathering, straightness) affects flight characteristics. The trap system allows arrows to carry poison or trap effects through trap_level and trap_dam_type.

`throwMe()` handles manual throwing. `loadBowArrow()` handles loading into a bow for firing.

### Firearm Mechanics

TGun introduces rate of fire (rof) for multiple attacks per round, ammunition type requirements, and gun-specific condition flags.

`loadMe()` and `unloadMe()` handle ammunition management. Guns require matching ammunition types; attempting to load incompatible ammunition fails.

The firearm damage penalty (division by 2 in `getWeaponDam()`) balances their advantages. GUN_FLAG_FOULED indicates maintenance needs affecting reliability.

THandgonne overrides these mechanics for historical accuracy: slower loading, historical unloading procedures, high damage but low rate of fire.

### Object Factory

The `makeNewObj()` function in db.cc instantiates the correct weapon subclass based on itemTypeT.

ITEM_WEAPON creates TGenWeapon for melee weapons. ITEM_BOW creates TBow. ITEM_ARROW creates TArrow. ITEM_GUN creates TGun. ITEM_HANDGONNE creates THandgonne.

Each constructor initializes the object with default values: maxSharp to 10-50 based on material, curSharp to maxSharp, damLevel to base value from vnum lookup, damDev to randomization range, and weapon_type arrays to WEAPON_TYPE_NONE.

The `read_object()` function calls makeNewObj, then populates the weapon from database cache: assignFourValues restores packed data, extra descriptions are applied, affects/enchantments are added, and the weapon is registered in object_list.

### Runtime Customization

The `swapToStrung()` method marks a weapon as customized, enabling modification of name, shortDescr, longDescr, and action_description. These modifications persist to rent files.

Direct value modification is possible through setter methods: `setMaxSharp()`, `setCurSharp()`, `setWeapDamLvl()`, `setWeapDamDev()`. TGenWeapon adds `setWeaponType()` and `setWeaponFreq()` for attack type configuration.

The oedit builder command exposes these capabilities through an in-game interface for zone builders.

### Persistence

Binary rent files store weapon data packed into val0-val3 integers. The bitfield format is frozen; changing it would corrupt existing saved items.

The MariaDB obj table stores vnum, type, val0-val3, extra_flags, weight, cost, material, structure points, and affect data. The bitfield approach enables efficient storage without expanding database schema.

## Troubleshooting

### Weapon Dealing Wrong Damage

**Symptom:** Weapon damage seems too high or too low compared to similar weapons.

**Likely causes:** Wrong weapon type classification affecting strength modifier, incorrect damage level, broken sharpness value.

**Diagnostic approach:** Check `isBluntWeapon()`, `isSlashWeapon()`, `isPierceWeapon()` return values. Verify damLevel through stat command. Compare curSharp to maxSharp.

**Fix:** For multi-type weapons, verify frequency weights produce intended category classification. Adjust damLevel if weapon base stats are wrong. Sharpen weapon if sharpness degraded.

### Secondary Hand Not Attacking

**Symptom:** Character with two weapons only attacks with primary hand.

**Likely causes:** Missing SKILL_DUAL_WIELD, weapon not properly equipped in secondary slot, weapon lacks required flags.

**Diagnostic approach:** Check character skill values for dual wield proficiency. Verify equipment slots show weapon in secondary hand. Check if weapon has ITEM_PAIRED flag if expecting paired bonuses.

**Fix:** Train SKILL_DUAL_WIELD or SKILL_DUAL_WIELD_THIEF. Re-equip weapon in correct slot. For paired weapons, ensure both weapons of the set are present.

### Bow Won't Fire

**Symptom:** Shooting command fails or produces error message.

**Likely causes:** String broken, wrong ammunition type, no ammunition.

**Diagnostic approach:** Check bow flags for BOW_STRING_BROKE. Compare bow's arrowType with available arrows. Verify arrows exist in inventory.

**Fix:** Repair bow string. Obtain correct ammunition type. Load arrows before attempting to fire.

### Weapon Type Always Same

**Symptom:** Multi-type weapon always uses same attack type despite having multiple types configured.

**Likely causes:** Frequency weights misconfigured, only one type actually set.

**Diagnostic approach:** Use stat to examine all three weapon_type slots and their frequencies. Calculate whether frequency distribution matches observed behavior.

**Fix:** Adjust wtype_frequency values through oedit or database. Ensure at least two types have non-zero frequency for variety.

### Gun Damage Seems Low

**Symptom:** Gun damage is much lower than melee weapon with equivalent stats.

**Likely causes:** This is intended behavior. Guns receive 50% damage penalty.

**Diagnostic approach:** Verify penalty is being applied in getWeaponDam. Check that itemType returns ITEM_GUN.

**Fix:** If penalty is missing, fix getWeaponDam to check itemType. If gun damage is intentionally too low for game balance, adjust base damLevel upward to compensate for the penalty.

### Sharpening Has No Effect

**Symptom:** Using sharpen command but weapon damage doesn't improve.

**Likely causes:** Weapon already at maxSharp, low SKILL_SHARPEN causing failures, sharpness has minimal damage impact.

**Diagnostic approach:** Compare curSharp to maxSharp. Check skill check results. Remember sharpness is only 10% of weaponLevel.

**Fix:** If at maxSharp, weapon is fully maintained. Train SKILL_SHARPEN for better success rate. Set realistic expectations about sharpness impact on overall damage.

### Sharpening Fails Consistently

**Symptom:** Sharpen attempts keep failing despite having the skill.

**Likely causes:** Low SKILL_SHARPEN learning, insufficient movement points, weapon material too difficult.

**Diagnostic approach:** Check SKILL_SHARPEN learning value. Verify movement points exceed cost. Examine weapon material hardness.

**Fix:** Train SKILL_SHARPEN on easier weapons first. Rest to restore movement points. Practice on common materials before exotic ones.

### Value Fields Not Persisting

**Symptom:** Weapon modifications disappear after rent or server restart.

**Likely causes:** Missing swapToStrung() call, direct field modification bypassing setters, database write failure.

**Diagnostic approach:** Confirm swapToStrung() was called before modifying string fields. Verify setter methods are used for value fields. Check database permissions and connection.

**Fix:** Call swapToStrung() before name/description changes. Use setter methods for all value modifications. Verify database write succeeds.
