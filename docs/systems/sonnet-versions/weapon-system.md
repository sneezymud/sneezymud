---
title: Weapon System
category: critical
keywords: [TBaseWeapon, TGenWeapon, weaponT, damage, sharpness, dual-wield, specialization, firearms]
related: [combat-formulas.md, tohit-defense.md, equipment-wear.md, material-system.md]
primary_symbols:
  functions: [getWeaponDam, swungObjectDamage, sharpenMe, dullMe, weaponLevel, specializationCheck]
  classes: [TBaseWeapon, TGenWeapon, TBow, TArrow, TGun, THandgonne]
  files: [code/code/obj/obj_base_weapon.cc, code/code/obj/obj_general_weapon.cc, code/code/misc/combat.cc]
---

## Overview

The weapon system implements damage-dealing equipment through a class hierarchy rooted in TBaseWeapon. Weapons define attack types, sharpness degradation, skill-based proficiencies, and damage formulas that integrate strength modifiers, dual-wield penalties, and specialization bonuses.

The system supports 35 attack types organized into slash, pierce, and blunt categories. Weapons degrade through use via the sharpness system, requiring maintenance through SKILL_SHARPEN. Multi-type weapons select attack types via frequency weighting. Specialized subclasses handle bows, arrows, firearms, and historical hand cannons with distinct mechanics.

Damage calculation flows through getWeaponDam in combat.cc, combining base weapon damage from swungObjectDamage with character strength, skill proficiency, specialization bonuses, and penalties for dual wielding or firearms. The weaponLevel formula weights damage at 60%, structure at 30%, and sharpness at 10% to determine overall weapon effectiveness.

Seven skill types govern weapon use: proficiencies provide basic competency, specializations grant damage multipliers and attack frequency bonuses, and SKILL_DUAL_WIELD scales secondary hand damage from 30% to 60% based on learning. Two-handed weapons receive a 1.75x damage multiplier; paired weapons add an additional 1.1x bonus.

## Patterns

### Damage Calculation Flow

Call getWeaponDam from combat.cc to obtain final damage. This function retrieves base damage via swungObjectDamage on the weapon object, applies dual-wield penalties if the weapon is equipped in the secondary hand, multiplies by strength modifiers based on weapon category (full for blunt, half for slash, third for pierce), applies skill proficiency bonuses, reduces firearms damage by 50%, and adds incapacitation bonuses for disabled opponents.

The swungObjectDamage method on TBaseWeapon returns intrinsic weapon damage by calling baseDamage, adding random deviation based on damDev, and including extra damage bonuses from affects. The baseDamage calculation multiplies damageLevel by 1.75 for two-handed weapons and by an additional 1.1 for paired weapons.

Weapon learning automatically equals character level multiplied by 2, or the actual skill value if higher, capped at 100. This learning percentage multiplies the damage calculation in getWeaponDam to scale weapon effectiveness with character progression.

### Multi-Type Weapon Selection

TGenWeapon stores up to three weapon types with frequency weightings. When getWeaponType is called, the method sums all frequency values, rolls a random number within the total, and returns the weapon type corresponding to the rolled range. This enables weapons to deal slash damage 70% of the time and pierce damage 30% of the time, for example.

Type classification functions isBluntWeapon, isSlashWeapon, and isPierceWeapon apply the two-thirds rule: if at least two-thirds of weighted attack types belong to a category, the weapon is classified accordingly. This classification determines strength scaling in damage calculations.

### Sharpness Degradation and Maintenance

Weapons track current and maximum sharpness in the curSharp and maxSharp fields. Combat actions call dullMe to reduce curSharp when striking hard targets, parrying attacks, or through duration-based wear. The weaponLevel formula includes sharpness at 10% weight, reducing damage proportionally as sharpness declines.

Characters with SKILL_SHARPEN can restore sharpness by calling sharpenMe. This method checks that curSharp is below maxSharp, consumes movement points based on weapon complexity and skill learning, rolls a skill check, and increments curSharp by 1-2 points on success, capping at maxSharp. Higher skill learning increases success rate, reduces movement cost, and improves points gained per session.

Display terminology changes based on weapon type: blunt weapons show "bluntness", pierce weapons show "pointiness", and slash weapons show "sharpness". The statObjInfo method in TGenWeapon formats this display along with damage level and structure points.

### Dual Wielding Mechanics

Characters equipped with weapons in both hands receive secondary hand attacks at reduced damage. The damage formula scales secondary hand damage as primaryDamage multiplied by (30 + 30 * SKILL_DUAL_WIELD / 100) / 100, yielding 30% damage with no skill and 60% damage at max skill. Attack frequency distributes as 60% primary hand and 40% secondary hand.

Weapons with ITEM_PAIRED flag receive special treatment: they gain the 1.1x paired multiplier on primary hand damage, can be equipped simultaneously in both hands, and represent mechanically and cosmetically matched sets. Check isPaired on the weapon object to determine paired status.

Combat type checks canCudgel, canBackstab, and canStab verify weapon volume and type to determine availability of special attacks. Cudgeling requires blunt type and volume at most 1500, backstabbing requires pierce type and volume at most 1500, and stabbing requires pierce type and volume at most 2000.

### Specialization Integration

The specializationCheck method on TBaseWeapon accepts a character pointer, determines the appropriate specialization skill based on weapon category, retrieves the character's skill value, and returns skillValue / 100.0 as a decimal multiplier. This bonus applies in damage calculations when the weapon type matches the character's discipline specialization.

Warriors access SKILL_SLASH_SPEC, SKILL_BLUNT_SPEC, and SKILL_PIERCE_SPEC. Monks use SKILL_BAREHAND_SPEC for unarmed combat. Rangers employ SKILL_RANGED_SPEC. Each specialization lives within a corresponding discipline tree in the skill system, providing both passive bonuses and active abilities.

Two-handed weapon specializations SKILL_2H_SPEC and SKILL_2H_SPEC_DEIKHAN increase attack frequency with two-handed weapons. The skill value divided by 100 yields additional attacks per combat round, rewarding investment in two-handed mastery.

### Ranged Weapon Architecture

TBow extends TBaseWeapon with arrowType to specify required ammunition, max_range for shooting distance limits, and flags for bow condition including BOW_STRING_BROKE, BOW_CARVED, BOW_SCRAPED, and BOW_SMOOTHED. The shootMeBow method handles ranged attack execution, checking string integrity and arrow availability before firing.

TArrow also extends TBaseWeapon, inheriting sharpness and damage mechanics while adding arrow-specific flags ARROW_FEATHERED, ARROW_CARVED, ARROW_SCRAPED, and ARROW_SMOOTHED. Feathering improves accuracy and distance. Arrows support trap_level and trap_dam_type for poison or trap effects. Methods throwMe and loadBowArrow implement projectile mechanics.

TGun introduces rate of fire (rof) for attacks per round, ammunition type (ammotype) with 18 distinct types, and gun flags including GUN_FLAG_SILENCED, GUN_FLAG_CASELESS, GUN_FLAG_CLIPLESS, and GUN_FLAG_FOULED. Load and unload methods manage ammunition. Firearms suffer a 50% damage penalty in getWeaponDam via weaponDamage /= 2 for balance against melee weapons.

THandgonne extends TGun with slower loading mechanics and historical flavor, overriding loadMe, unloadMe, and shootMeBow to implement slower rate of fire balanced by higher individual shot damage.

### Value Field Packing

TBaseWeapon packs sharpness into val0 with curSharp in bits 0-7 and maxSharp in bits 8-15. Damage packs into val1 with damLevel in bits 0-7 and damDev in bits 8-15. TGenWeapon extends this by packing weapon types and frequencies into val3: weapon_type[0] in bits 0-7, wtype_frequency[0] in bits 8-15, weapon_type[1] in bits 16-23, and wtype_frequency[1] in bits 24-31. The third weapon type and frequency occupy a separate field.

Access these packed values through getter and setter methods: getCurSharp, setCurSharp, getMaxSharp, setMaxSharp, getWeapDamLvl, setWeapDamLvl, getWeapDamDev, setWeapDamDev. TGenWeapon provides assignFourValues and getFourValues to pack and unpack the complete value set atomically.

This bitfield approach enables efficient database persistence without schema expansion. All weapon data serializes into four integers stored in the MariaDB obj table and binary rent files.

### Object Factory Instantiation

Call makeNewObj in db.cc with an itemTypeT parameter to instantiate weapon objects. The factory switches on ITEM_WEAPON to return new TGenWeapon, ITEM_BOW for new TBow, ITEM_ARROW for new TArrow, ITEM_GUN for new TGun, and ITEM_HANDGONNE for new THandgonne. After instantiation, read_object populates values from database cache or query by calling assignFourValues, applies extra descriptions and affects, registers in object_list, and returns the initialized weapon.

Runtime customization requires calling swapToStrung on the weapon object to mark it as modified. Afterward, name, shortDescr, longDescr, and action_description fields become writable. Changes persist through rent file serialization. Direct value modification through setters adjusts sharpness, damage, and weapon types without swapToStrung.

## Reference

### Class Hierarchy

TBaseWeapon serves as the abstract base class defining maxSharp, curSharp, damLevel, damDev, and poison fields. Key methods include baseDamage for base damage calculation, swungObjectDamage for core randomized damage, sharpenMe for sharpening mechanics, dullMe for blunting mechanics, weaponLevel for overall effectiveness formula, damageLevel for damage component extraction, structLevel for structure component, and sharpLevel for sharpness component.

TGenWeapon extends TBaseWeapon for general melee weapons, adding weapon_type[3] and wtype_frequency[3] arrays to support multiple attack types. The getWeaponType method performs frequency-weighted random selection. Type classification methods canCudgel, canBackstab, and canStab check volume and type requirements.

TBow extends TBaseWeapon for ranged weapons, adding arrowType, flags, and max_range fields. Methods include shootMeBow, getArrowType, setArrowType, getMaxRange, and setMaxRange. Bow flags include BOW_STRING_BROKE, BOW_CARVED, BOW_SCRAPED, and BOW_SMOOTHED.

TArrow extends TBaseWeapon for projectiles, adding trap_level and trap_dam_type fields. Arrow flags include ARROW_FEATHERED, ARROW_CARVED, ARROW_SCRAPED, and ARROW_SMOOTHED. Methods throwMe and loadBowArrow implement projectile behavior.

TGun extends TBaseWeapon for firearms, adding rof, ammotype, and flags fields. Methods include loadMe, unloadMe, getROF, setROF, getAmmoType, and setAmmoType. Gun flags include GUN_FLAG_SILENCED, GUN_FLAG_CASELESS, GUN_FLAG_CLIPLESS, and GUN_FLAG_FOULED.

THandgonne extends TGun for historical hand cannons, overriding loadMe, unloadMe, and shootMeBow with slower mechanics and higher damage.

### Weapon Types

The weaponT enumeration in enum.h defines 35 attack types. Pierce types include WEAPON_TYPE_STAB, WEAPON_TYPE_BITE, WEAPON_TYPE_STING, WEAPON_TYPE_PIERCE, WEAPON_TYPE_THRUST, WEAPON_TYPE_SPEAR, and WEAPON_TYPE_BEAK. Slash types include WEAPON_TYPE_WHIP, WEAPON_TYPE_SLASH, WEAPON_TYPE_CLEAVE, WEAPON_TYPE_CLAW, WEAPON_TYPE_SLICE, WEAPON_TYPE_BEAR_CLAW, and WEAPON_TYPE_SHRED. Blunt types include WEAPON_TYPE_SMASH, WEAPON_TYPE_CRUSH, WEAPON_TYPE_BLUDGEON, WEAPON_TYPE_PUMMEL, WEAPON_TYPE_FLAIL, WEAPON_TYPE_BEAT, WEAPON_TYPE_THRASH, WEAPON_TYPE_THUMP, WEAPON_TYPE_WALLOP, WEAPON_TYPE_BATTER, WEAPON_TYPE_STRIKE, WEAPON_TYPE_CLUB, WEAPON_TYPE_POUND, and WEAPON_TYPE_SMITE. Special types include WEAPON_TYPE_AIR, WEAPON_TYPE_EARTH, WEAPON_TYPE_FIRE, WEAPON_TYPE_WATER. Ranged types include WEAPON_TYPE_SHOOT and WEAPON_TYPE_CANNON.

### Damage Formulas

The weaponLevel formula in obj_base_weapon.cc computes overall weapon effectiveness as (damageLevel * 0.6) + (structLevel * 0.3) + (sharpLevel * 0.1). The damageLevel component equals damLevel / 4.0. The structLevel component equals max(maxStructPoints - 10, 0) * 2.0 / 3.0. The sharpLevel component equals max(maxSharp - 10, 0) * 2.0 / 3.0.

The baseDamage formula computes intrinsic damage as damageLevel() * multiplier where multiplier equals 1.75 * (isPaired() ? 1.1 : 1.0). Two-handed non-paired weapons use 1.75x multiplier. Paired weapons add 1.1x for total 1.925x multiplier.

The getWeaponDam function in combat.cc computes final weapon damage as (baseDam + rollDam + bonusDam) * strModifier * weaponLearning / 100. Strength modifiers scale by weapon type: blunt and unarmed use full strDam, slash uses (strDam - 1) / 2 + 1, pierce uses (strDam - 1) / 3 + 1. Firearms divide final damage by 2. Incapacitated opponents add 3 * wepDam / 10 with minimum 1.

Secondary hand dual-wield damage equals primaryDamage * (30 + 30 * SKILL_DUAL_WIELD / 100) / 100, yielding range 30% to 60% based on skill.

### Weapon Skills

SKILL_DUAL_WIELD provides general dual-wield proficiency. SKILL_DUAL_WIELD_THIEF offers rogue-specific mastery. SKILL_SLASH_PROF, SKILL_PIERCE_PROF, SKILL_BLUNT_PROF, SKILL_BAREHAND_PROF, and SKILL_RANGED_PROF provide basic competency with minimum learning equal to level * 2.

SKILL_SLASH_SPEC, SKILL_BLUNT_SPEC, SKILL_PIERCE_SPEC, SKILL_BAREHAND_SPEC, and SKILL_RANGED_SPEC provide advanced specialization with damage multipliers. SKILL_2H_SPEC and SKILL_2H_SPEC_DEIKHAN increase two-handed weapon attack frequency. SKILL_SHARPEN enables weapon maintenance.

All weapon skills belong to discipline trees in spell_info.cc. DISC_COMBAT contains proficiencies and DUAL_WIELD. DISC_SLASH contains SLASH_SPEC. DISC_BLUNT contains BLUNT_SPEC. DISC_PIERCE contains PIERCE_SPEC.

### Ammunition Types

Firearms use 18 ammunition types defined in ammotype enumeration. Pistol types include AMMO_10MM_PISTOL, AMMO_9MM_PARABELLEM_PISTOL, AMMO_45CAL_ACP_PISTOL, AMMO_50CAL_AE_PISTOL, AMMO_44CAL_MAGNUM_PISTOL, AMMO_32CAL_ACP_PISTOL, AMMO_50CAL_BMG_PISTOL, AMMO_556MM_NATO_PISTOL, and AMMO_SS190. Rifle types include AMMO_9MM_PARABELLEM_RIFLE, AMMO_45CAL_ACP_RIFLE, AMMO_556MM_RIFLE, AMMO_762MM_RIFLE, AMMO_30CAL_RIFLE, and AMMO_FLECHETTE. Special types include AMMO_LAW, AMMO_LEAD_SHOT, and AMMO_CANNON_BALL.

### Weapon Flags

Item wear flags include ITEM_WEAR_TAKE, ITEM_WEAR_HOLD, and ITEM_WEAR_THROW. Weapon-specific flags include ITEM_PAIRED, ITEM_SPIKED, ITEM_MAGIC, ITEM_GLOW, ITEM_HUM, ITEM_NODROP, ITEM_BLESS, ITEM_NORENT, and ITEM_BURNING. Bow flags include BOW_STRING_BROKE, BOW_CARVED, BOW_SCRAPED, and BOW_SMOOTHED. Arrow flags include ARROW_FEATHERED, ARROW_CARVED, ARROW_SCRAPED, and ARROW_SMOOTHED. Gun flags include GUN_FLAG_SILENCED, GUN_FLAG_CASELESS, GUN_FLAG_CLIPLESS, and GUN_FLAG_FOULED.

Anti-class restriction flags prevent usage by specific classes: ITEM_ANTI_MAGE, ITEM_ANTI_CLERIC, ITEM_ANTI_WARRIOR, and similar flags for other classes.

### Constants

TWO_HAND_MULTIPLIER equals 1.75 for two-handed weapon damage boost. PAIRED_MULTIPLIER equals 1.1 for paired weapon bonus. SECONDARY_MIN equals 30% for minimum secondary hand damage. SECONDARY_MAX equals 60% for maximum secondary hand damage. GUN_PENALTY equals 0.5 for firearm damage divisor.

## Implementation

### Storage Architecture

TBaseWeapon fields maxSharp, curSharp, damLevel, damDev, and poison pack into val0 and val1. The getCurSharp method extracts val0 & 0xFF. The getMaxSharp method extracts (val0 >> 8) & 0xFF. The setCurSharp method updates val0 by clearing bits 0-7 and ORing the new value. The setMaxSharp method updates val0 by clearing bits 8-15 and ORing the new value shifted left 8 bits.

Damage packing mirrors sharpness packing using val1. The getWeapDamLvl method extracts val1 & 0xFF. The getWeapDamDev method extracts (val1 >> 8) & 0xFF. Setters modify val1 through bitwise operations.

TGenWeapon extends packing by using val3 for weapon types and frequencies. The assignFourValues method accepts four integers representing weapon type 0, frequency 0, weapon type 1, and frequency 1, packing them into val3 as (type0 | (freq0 << 8) | (type1 << 16) | (freq1 << 24)). The third type and frequency occupy a separate field. The getFourValues method unpacks val3 through bitwise extraction and populates four output parameters.

Database persistence serializes these packed values into the obj table columns val0, val1, val2, val3. Binary rent files write the same integer values directly. Both storage mechanisms restore weapons by calling assignFourValues during read_object.

### Combat Integration

The combat system in combat.cc calls getWeaponDam when calculating attack damage. This function receives pointers to the attacker character, weapon object, and victim. It retrieves base damage by calling weapon->swungObjectDamage(). If the weapon is equipped in the secondary hand, it applies the dual-wield penalty by multiplying damage by (30 + 30 * dualWieldSkill / 100) / 100.

Next, getWeaponDam determines weapon category through isBluntWeapon, isSlashWeapon, and isPierceWeapon checks. It retrieves the character's strength modifier from ch->getStrDamBonus(). Blunt weapons and unarmed attacks multiply damage by strModifier. Slash weapons use (strModifier - 1) / 2 + 1. Pierce weapons use (strModifier - 1) / 3 + 1.

The function queries the character's weapon proficiency skill through ch->getSkillValue(appropriateProficiency) and uses max(level * 2, skillValue) for weaponLearning, capping at 100. It multiplies damage by weaponLearning / 100 to scale with character progression.

For specialized characters, it calls weapon->specializationCheck(ch) to retrieve the specialization bonus and multiplies damage by the returned decimal value. If the weapon type is ITEM_GUN, it divides final damage by 2 for the firearms penalty. If the victim is incapacitated, it adds 3 * weaponDamage / 10 with minimum 1 as bonus damage.

### Sharpness Mechanics Implementation

The sharpenMe method in obj_base_weapon.cc checks getCurSharp() < getMaxSharp() to verify sharpening is possible. It calculates movement cost based on weapon complexity (higher for larger or more intricate weapons) and ch->getSkillValue(SKILL_SHARPEN). Higher skill reduces cost. It calls ch->addToMove(-cost) to consume movement points.

Next, sharpenMe rolls a skill check using ch->skillCheck(SKILL_SHARPEN, difficulty). The difficulty depends on weapon material and current sharpness. On success, it calculates points gained as 1 + skillValue / 50 (1-2 points for skill 0-100), calls setCurSharp(min(getCurSharp() + points, getMaxSharp())) to increment sharpness, and sends feedback messages to the character indicating success and new sharpness level.

On failure, it sends a failure message. Regardless of outcome, it advances skill learning through ch->bumpSkill(SKILL_SHARPEN) for gradual proficiency improvement.

The dullMe method decrements curSharp through setCurSharp(max(0, getCurSharp() - amount)). Combat code calls dullMe when weapons strike hard materials detected through victim armor checks, when weapons parry multiple attacks tracked through combat counters, or periodically based on combat duration. The amount parameter scales with damage dealt and target armor hardness.

### Multi-Type Selection Implementation

The getWeaponType method in obj_general_weapon.cc sums wtype_frequency[0] + wtype_frequency[1] + wtype_frequency[2] to compute total. It generates a random number via ::number(0, total - 1) to obtain roll. It accumulates frequencies: if roll < wtype_frequency[0], return weapon_type[0]. Otherwise if roll < wtype_frequency[0] + wtype_frequency[1], return weapon_type[1]. Otherwise return weapon_type[2].

Type classification methods iterate the three weapon type slots. The isBluntWeapon method accumulates frequency for all blunt types, divides by total frequency, and returns true if the ratio is at least 2/3. The isSlashWeapon and isPierceWeapon methods apply the same logic for their respective categories.

This approach enables weapons with mixed types (e.g., 60% slash, 30% pierce, 10% blunt) to be classified as slash weapons since slash exceeds the two-thirds threshold, while hybrid weapons with balanced types remain unclassified for any single category.

### Ranged Weapon Implementation

The shootMeBow method in obj_bow.cc checks isBowFlag(BOW_STRING_BROKE) to verify bow integrity. It searches the character's equipment for an arrow matching arrowType through equipment slot iteration and type comparison. If no arrow exists, it sends a message and returns failure.

On finding valid ammunition, shootMeBow calculates range based on max_range and character's SKILL_RANGED_PROF. It validates the target is within range through room distance calculation. It removes the arrow from equipment through the unequip operator and calculates projectile damage by calling arrow->swungObjectDamage() modified by bow condition flags (CARVED, SCRAPED, SMOOTHED) and arrow flags (FEATHERED, CARVED, SCRAPED, SMOOTHED).

The method executes the ranged attack through the combat system, sending the arrow object as the projectile. Combat code applies distance penalties by reducing damage proportionally to range traveled. Arrows with trap_level trigger trap effects on hit through separate trap resolution code. After impact, the arrow object may break based on material and impact force, or land in the target room available for retrieval.

TGun::loadMe iterates the character's inventory searching for objects matching ammotype. On finding compatible ammunition, it checks magazine capacity through getClipCapacity(), verifies remaining space, removes rounds from inventory, increments loaded ammunition count, and sends confirmation messages. The unloadMe method reverses this process by creating new ammunition objects, transferring them to inventory, and decrementing loaded count.

Rate of fire affects combat frequency. The combat system queries weapon->getROF() and grants floor(rof) guaranteed attacks plus a fractional chance for an additional attack. A gun with rof 2.5 grants 2 attacks plus 50% chance for a third attack per combat round.

### Specialization Integration Implementation

The specializationCheck method in obj_base_weapon.cc determines appropriate specialization by checking weapon category. For blunt weapons identified through isBluntWeapon(), it uses SKILL_BLUNT_SPEC. For slash weapons, it uses SKILL_SLASH_SPEC. For pierce weapons, it uses SKILL_PIERCE_SPEC. For bare-hand attacks (weapon is nullptr), it uses SKILL_BAREHAND_SPEC. For ranged weapons (ITEM_BOW or ITEM_GUN type), it uses SKILL_RANGED_SPEC.

After identifying the skill, it calls ch->getSkillValue(specializationSkill) to retrieve learning (0-100). It divides by 100.0 to produce a decimal multiplier (0.0 to 1.0+). A warrior with 75 SKILL_BLUNT_SPEC wielding a blunt weapon receives a 0.75 multiplier on damage, effectively granting 75% bonus damage.

This bonus integrates multiplicatively with other damage modifiers in getWeaponDam, applying after strength scaling but before the firearms penalty. Specialization stacks with proficiency, dual-wield penalties, and sharpness effects, creating substantial damage variation between specialized and non-specialized characters.

### Factory and Persistence Implementation

The makeNewObj function in db.cc switches on the itemTypeT parameter. For ITEM_WEAPON, it calls return new TGenWeapon(). For ITEM_BOW, it calls return new TBow(). For ITEM_ARROW, it calls return new TArrow(). For ITEM_GUN, it calls return new TGun(). For ITEM_HANDGONNE, it calls return new THandgonne(). Each constructor initializes the object with default values: maxSharp to 10-50 based on material, curSharp to maxSharp, damLevel to base value from vnum lookup, damDev to randomization range, and weapon_type arrays to WEAPON_TYPE_NONE.

The read_object function in db.cc calls makeNewObj to instantiate the weapon, queries the database or cache for object vnum data, and extracts val0, val1, val2, val3 from the result set. It calls weapon->assignFourValues(val0, val1, val2, val3) to populate packed fields. For TGenWeapon objects, it unpacks weapon types and frequencies. For TBow objects, it restores arrowType, flags, and max_range from the value fields. For TGun objects, it restores rof, ammotype, and flags.

After value restoration, read_object applies extra descriptions from the database extra_descr table, applies affects from the obj_affects table through the affect system, sets object flags from the extra_flags column, registers the weapon in the global object_list for tracking, and returns the fully initialized weapon pointer.

Saving occurs through the object persistence system. When characters quit or rent, the save function iterates equipped and inventory objects, extracts val0-val3 through getter methods, writes the values to the obj table via SQL UPDATE or INSERT, serializes affects to obj_affects, and writes extra descriptions to extra_descr. Binary rent files pack the same values into a compact binary format read on character load.

Runtime modification via swapToStrung clears the prototype flag on the object, allocating independent memory for name, shortDescr, longDescr, and action_description strings. Changes to these strings persist through the save process. Builders use the oedit command to invoke swapToStrung and modify values through interactive editing sessions, with changes committed to the database on command completion.

## Troubleshooting

### Damage Unexpectedly Low

Verify curSharp has not degraded through getCurSharp() comparison against getMaxSharp(). Low sharpness reduces damage via the weaponLevel formula's 10% sharpness component. Check weapon structure through getMaxStructPoints() and getStructPoints(); damaged structure contributes 30% weight to weaponLevel. Confirm the character has appropriate proficiency skill at level * 2 minimum; missing proficiencies cap weaponLearning and reduce damage multiplicatively.

If wielding a secondary weapon, confirm SKILL_DUAL_WIELD learning; values below 50 impose severe penalties (30-45% damage vs 60% maximum). Verify the weapon type matches character specialization; a warrior without SKILL_BLUNT_SPEC wielding a blunt weapon loses the specialization multiplier. Check for firearms penalty; guns divide damage by 2 independent of other modifiers.

### Sharpening Fails Consistently

Check SKILL_SHARPEN learning through character skills display. Low skill increases difficulty and reduces success rate. Verify movement points exceed the cost through character movement display; insufficient movement prevents sharpening attempts. Confirm curSharp is below maxSharp; attempting to sharpen a fully sharp weapon always fails with an appropriate message.

Examine weapon material hardness; exotic materials impose higher difficulty. Review recent skill advancement; characters may need to practice sharpening on easier weapons before attempting difficult materials. Check for affects or debuffs reducing dexterity or fine manipulation; these increase sharpening difficulty indirectly.

### Multi-Type Weapons Dealing Wrong Damage Type

Inspect weapon type frequencies through oedit or diagnostic commands. Frequency values of 0 prevent that type from selection. Verify total frequency sum is non-zero; zero total causes division-by-zero and defaults to weapon_type[0]. Confirm the random number generator is seeding properly; deterministic results indicate RNG failure.

Check type classification through isBluntWeapon, isSlashWeapon, isPierceWeapon methods. Weapons classified incorrectly apply wrong strength scaling. Verify frequency weighting matches intended distribution; a weapon with frequencies [90, 5, 5] selects type[0] 90% of the time despite having three defined types.

### Dual Wield Not Functioning

Confirm both weapons have appropriate wear flags (ITEM_WEAR_HOLD). Verify the character has SKILL_DUAL_WIELD learning above 0; absent skill prevents secondary hand attacks. Check equipment slots; primary hand uses HOLD_RIGHT, secondary uses HOLD_LEFT or vice versa depending on configuration. Inspect weapon volume; oversized weapons may prevent dual wielding through volume checks.

Review combat logs to verify secondary attacks occur; absent secondary attacks despite proper equipment indicate combat system issues. Confirm attack frequency distribution matches expected 60/40 split; skewed distribution suggests combat round calculation errors. Check that ITEM_PAIRED flag is set on paired weapons if using matched sets; unpaired weapons do not receive the 1.1x bonus.

### Bow Cannot Fire

Check BOW_STRING_BROKE flag through isBowFlag method. Broken strings prevent all shooting attempts with explicit error messages. Verify arrow inventory contains arrows matching arrowType; mismatched ammunition fails silently or with type mismatch errors. Confirm max_range is positive and non-zero; zero range prevents targeting.

Examine character SKILL_RANGED_PROF; absent skill may impose penalties or prevent usage. Review room distance calculations; targets beyond max_range fail with out-of-range messages. Check for movement restrictions; paralyzed or bound characters cannot perform ranged attacks.

### Gun Loading Failures

Verify ammunition type matches gun ammotype through getAmmoType comparison. Mismatched ammunition generates explicit error messages and prevents loading. Check magazine capacity through getClipCapacity; attempting to load beyond capacity fails. Confirm ammunition objects exist in inventory; absent ammunition prevents loading.

Inspect GUN_FLAG_FOULED status; fouled guns may fail loading attempts or increase failure rate. Review ammunition count after loading; successful loads decrement inventory ammunition and increment loaded count. Check for caseless ammo flag interaction with gun flags; certain combinations may prevent proper loading.

### Specialization Not Applying

Verify character class has access to the specialization skill. Warriors access slash, blunt, and pierce specs; monks access barehand spec; rangers access ranged spec. Check skill learning value; 0 learning grants 0.0 multiplier (no bonus). Confirm weapon category matches specialization; a warrior with SKILL_SLASH_SPEC wielding a blunt weapon receives no benefit.

Review damage calculation flow through combat logs or debugging output. Specialization applies multiplicatively; verify it multiplies after strength scaling. Check for skill learning caps; some specializations may have learning limits based on character level or equipment.

### Value Fields Not Persisting

Confirm swapToStrung was called before modifying string fields. Prototype objects share string memory; modifications require independent allocation. Verify setter methods are used for value fields; direct field modification bypasses packing logic. Check database write permissions; failed writes due to permissions or connection issues prevent persistence.

Examine rent file integrity through file size and format validation. Corrupted rent files fail to restore values. Review object vnum uniqueness; shared vnums across prototypes and instances may cause restoration conflicts. Confirm assignFourValues receives correct parameters; incorrect packing corrupts all value fields.

### Weapon Effectiveness Formula Incorrect

Recalculate weaponLevel manually: (damageLevel * 0.6) + (structLevel * 0.3) + (sharpLevel * 0.1). Verify damageLevel equals damLevel / 4.0. Confirm structLevel equals max(maxStructPoints - 10, 0) * 2.0 / 3.0. Check sharpLevel equals max(maxSharp - 10, 0) * 2.0 / 3.0. Ensure all components use correct field values.

Compare calculated weaponLevel against expected damage output. Low structure or sharpness disproportionately reduce effectiveness due to their weighted contributions. Verify baseDamage multiplier application; two-handed weapons without the 1.75x multiplier deal substantially reduced damage. Check paired weapon flag for the 1.1x bonus; absent flags lose 10% damage.
