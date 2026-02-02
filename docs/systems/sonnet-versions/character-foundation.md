---
title: Character Foundation
category: critical
keywords: [multiclass, bitmask classes, race system, stat scaling, power law, plotStat, resource pools, mana, piety, lifeforce, racial immunities, territory bonuses, age modifiers]
related: [class-hierarchy.md, spell-skill-framework.md, experience-leveling.md, combat-formulas.md, equipment-wear.md]
primary_symbols:
  functions: [plotStat, getStat, hasClass, getLevel, affectModify, age_mod_for_stat, territory_adjustment]
  classes: [TBeing, Race, Stats, playerData, pointData]
  files: [code/code/misc/being.cc, code/code/misc/race.cc, code/code/misc/stats.cc, code/code/misc/multiclass.cc]
---

## Overview

Character capabilities emerge from three interconnected foundations: classes define abilities and progression, races provide baseline characteristics and physical form, stats determine effectiveness at all actions. These systems combine multiplicatively rather than additively, creating emergent complexity from simple rules.

Classes use a bitmask multiclass system where characters combine up to nine classes simultaneously with independent level tracking per class and quadratic experience penalties. Each class provides unique resource pools (mana for mages, piety for clerics, lifeforce for shamans) and combat behaviors through AI dispatchers that select actions based on class type.

Races define 127 distinct creature types from playable humanoids to exotic monsters. Each race specifies baseline values for 13 stats, body anatomy type, natural resistances to 28 damage types, physical dimensions via dice notation, and special talents. The flyweight pattern shares a single Race instance per type across all beings.

Stats follow non-linear power-law scaling with exponent 1.4 that amplifies differences at extremes. The 200-point range (5 minimum to 205 maximum, 105 baseline) creates significant power gaps where high stats dramatically outperform average ones. Stats layer through permanent foundations (race, chosen allocation, territory), dynamic modifiers (age, equipment, spells), with all calculations flowing through plotStat for consistent scaling.

## Patterns

### Bitmask Class Management

Classes store as single-bit flags in an unsigned short, enabling efficient multiclass queries and arbitrary combinations. Each class constant like CLASS_MAGE equals a power of two (0x0001, 0x0002, 0x0004) so bitwise OR combines classes while AND tests membership. The Class field holds the combined bitmask while separate level arrays track progression per class via classIndT enum indices.

Query hasClass with EXACT_NO for partial matches (character has any of specified classes) or EXACT_YES for complete matches (has all specified classes, may have others). Convert between representations using getClassNum for bitmask values and getClassIndNum for array indices. Count active classes with NumClasses by iterating class bits.

Display methods distinguish single versus multiclass: single classes show first four characters capitalized, multiclass shows slash-separated abbreviations from classInfo array. The bestClass method returns the classIndT with highest level for AI and primary class calculations.

Practice points and experience divide by howManyClasses resulting in quadratic penalties: two classes receive 25% per class (1/2 * 1/2), three classes receive 11% per class (1/3 * 1/3). This strongly discourages excessive multiclassing while allowing tactical dual-class builds.

### Race Flyweight Access

All Race instances populate a global Races array at boot via initRaces loading from lib/races/ files. Characters store only a Race pointer, not copies of race data. Query racial properties by delegating through this pointer: isWinged delegates to race->isWinged, base stats access through race->baseStats.get.

Racial characteristics use bitflags for physical properties (WINGED, AQUATIC, FOURLEGGED) checked via query methods. Lore categories (LORE_ANIMAL, LORE_UNDEAD, LORE_PEOPLE) group races for spell targeting and skill effectiveness. Talents use a separate bitmask for special abilities like TALENT_FAST_REGEN or TALENT_LIMB_REGROWTH.

Body types from the body_t enum define anatomy determining equipment slots and limb configuration. The 81 body types span humanoid variants, quadrupeds, winged creatures, aquatic forms, multi-limbed entities, and exotic constructs. Body type controls which of 24 wearSlotT positions accept equipment and affects movement messages.

Natural immunities store percentage resistance (0-100%) to each of 28 damage types. Resistances stack additively across racial, equipment, and spell sources then cap at 100%. Negative percentages indicate vulnerability where damage amplifies rather than reduces.

### Layered Stat Calculation

Stats flow through six layers accessed via statSetT parameter to getStat. STAT_RACE provides racial baseline. STAT_CHOSEN holds player allocation at creation. STAT_AGE returns age modifiers calculated from current age. STAT_TERRITORY provides homeland bonuses. STAT_NATURAL combines all permanent and semi-permanent sources. STAT_CURRENT includes temporary equipment and spell effects.

The STAT_NATURAL calculation chains: start with race->baseStats, add chosenStats, add age_mod_for_stat result, add territory_adjustment, add skill bonuses like SKILL_IRON_MUSCLES, add vampire bonuses if applicable. STAT_CURRENT simply returns curStats which affectModify updates when equipment or spells apply.

Territory adjustments provide homeland optimization: urban birthplaces boost mental stats (INT, WIS, CHA, SPE) while penalizing physical (CON, KAR). Mountain and forest origins boost durability (CON, BRA, KAR) while penalizing social and mental stats. Plains provide balanced outdoor bonuses emphasizing perception.

Age modifiers peak physical stats at 16-30 then decline after 40 with accelerating penalties to 80. STR gains +10 from 16-30, drops to -10 by 80. AGI peaks at +10 for ages 16-20, declines to -8 by 80. Mental stats remain stable until 50 then decline modestly. Vampires ignore all age modifiers and instead receive flat +25 to STR, SPE, CHA.

### Power Law Scaling via plotStat

The plotStat function wraps plotValue which implements piecewise power-law curves with exponent 1.4. Split at midpoint 105: lower half (5-105) curves from minValue to average, upper half (105-205) curves from average to maxValue. The power law amplifies stat differences near extremes making high stats disproportionately valuable.

Standard damage/modifier usage passes (0.8, 1.25, 1.0) yielding 80% effectiveness at stat 5, 100% at stat 105, 125% at stat 205. The non-linear scaling means going from 105 to 155 provides more benefit than 55 to 105 despite same numeric difference.

Specialized uses vary parameters: shop prices use (1.3, 1.1, 1.15) so high CHA reduces markup from 30% to 10%. Karma affects crit chance via (0.5, 2.0, 1.0) doubling base crit rate at maximum KAR. Stat self-checks map to success percentages via (5, 95, 25) creating broad range from guaranteed failure to near certainty.

The getStatMod helper extracts deviation from baseline, multiplies by parameter, adds back 1.0. This allows amplifying stat effects: multiplier 2 with plotStat returning 1.1 yields 1.2 by doubling the 0.1 deviation. Used for graduated effects where raw plotStat range proves insufficient.

### Resource Pool Management

Class resources store in pointData members with current and maximum values tracked separately. Mana uses short integers for mage spellcasting with regeneration based on INT modifier. Piety uses double precision for cleric spells enabling fractional costs (spell definitions divide piety cost by 4 for fine granularity). Lifeforce uses short integers for shaman spells scaling with level not stats.

Hit points combine class base per level (warriors 8.5, mages 5.25) with CON modifier via plotStat then add racial hpMod. Movement points similarly combine base with racial moveMod but also vary by terrain type. Maximum values recalculate when stats change through equipment or affects.

Regeneration rates call dedicated methods: manaGain for mana, pietyGain for piety, lifeforceGain for lifeforce. HP regeneration requires explicit rest or healing spells (no natural recovery). Position affects regeneration: sleeping provides maximum, standing minimum, with sitting and resting intermediate.

Resource checks use dedicated predicates: noPiety tests insufficient piety, noLifeforce tests lifeforce, tooTired tests movement exhaustion. These prevent casting or acting when resources depleted. Boolean returns enable simple conditional checks without exposing internal values.

### Equipment Affect Application

The affectModify function modifies curStats when equipment equips or spells cast. Pass add parameter true to apply modifier, false to remove. The function adds modifier to appropriate stat via curStats.add then applies bitvector flags if provided. This keeps stat modifications synchronized with affect flags.

Equip flow calls affectModify for each APPLY location on item with corresponding modifier value. Items with multiple affects call affectModify multiple times. Unequipping reverses by calling with add false and negating modifiers. Spell affects work identically using affectedData structures.

Temporary affects only modify STAT_CURRENT leaving STAT_NATURAL unchanged. This separation allows distinguishing permanent character power from temporary boosts. Equipment removal immediately recalculates capabilities as curStats updates. The system prevents stat affect accumulation bugs by explicitly adding and removing specific values.

Bitvector parameter applies affect flags like AFF_INVISIBLE or AFF_SANCTUARY simultaneously with stat changes. This maintains consistency between numeric bonuses and state flags. Removal clears flags preventing stale affects persisting after item removal.

### Class-Specific Combat AI

The classStuff dispatcher calls specialized combat functions based on bestClass (highest level class). Each class function implements characteristic tactics: fighterMove for warriors using bash and disarm, mageMove for offensive spellcasting, clerMove for healing and harm, thiefMove for backstab positioning.

Functions check situation before acting: HP percentage determines desperation (healing versus damage), resource availability gates expensive actions, tactical positioning affects melee skill choices. Return values signal whether action taken allowing fallback to basic attacks.

Multiclass characters resolve via highest level class not combined tactics. A level 25 mage / level 10 warrior uses mageMove exclusively. This simplifies AI while rewarding specialization. Players can use abilities from all classes but NPCs stick to primary class behavior.

Combat functions interact with resource pools checking mana, piety, or lifeforce before casting. Failed resource checks skip to next tactic rather than attempting impossible actions. This creates degrading effectiveness as resource pools deplete during extended fights.

## Reference

### Class Constants and Arrays

The nine playable classes define as bitmask constants: CLASS_MAGE (0x0001), CLASS_CLERIC (0x0002), CLASS_WARRIOR (0x0004), CLASS_THIEF (0x0008), CLASS_SHAMAN (0x0010), CLASS_DEIKHAN (0x0020), CLASS_MONK (0x0040), CLASS_RANGER (0x0080), CLASS_COMMONER (0x0100). CLASS_ALL combines all nine via (0x01FF).

The classInfo global array stores class characteristics indexed by classIndT. Each entry contains enabled flag, class_lev_num for level array index, class_num bitmask value, display name, primary and secondary disciplines, practice point multiplier, HP per level, and single-character abbreviation.

Level constants define progression caps: MAX_MORT (50) marks highest mortal level, GOD_LEVEL1 (51) begins immortal builder access, MAX_IMMORT (60) represents full administrator. Characters at GOD_LEVEL1 or above automatically receive all wizard powers.

The classIndT enum maps classes to array positions: MAGE_LEVEL_IND through COMMONER_LEVEL_IND (0-8) for active classes, UNUSED1_LEVEL_IND and UNUSED2_LEVEL_IND for expansion, MAX_SAVED_CLASSES (11) for array allocation. MAX_CLASSES equals UNUSED1_LEVEL_IND (9) representing used class count.

### Race Enumeration and Categories

The race_t enum defines 127 values from RACE_NORACE (0) through index 126. Playable races occupy indices 1-6: RACE_HUMAN, RACE_ELVEN, RACE_DWARF, RACE_HOBBIT, RACE_GNOME, RACE_OGRE. Remaining indices span animals (WOLF, BEAR, EAGLE), mythical creatures (DRAGON, UNICORN, SPHINX), humanoids (ORC, TROLL, GIANT), undead (SKELETON, VAMPIRE, LICH), and exotics (BEHOLDER, RUST_MONSTER).

Lore categories group races via Kingdom field: LORE_ANIMAL for natural creatures, LORE_VEGGIE for plants, LORE_DIABOLIC for demons, LORE_REPTILE for reptilian, LORE_UNDEAD for undead, LORE_GIANT for large humanoids, LORE_PEOPLE for civilized races, LORE_OTHER for everything else. Used for spell targeting and skill effectiveness.

Body types enumerate 81 anatomies: BODY_HUMANOID for standard bipeds, BODY_FOUR_LEG and BODY_FOUR_HOOF for quadrupeds, BODY_BIRD and BODY_DRAGON for winged, BODY_FISH and BODY_OCTOPUS for aquatic, BODY_SPIDER and BODY_CENTIPEDE for multi-limbed, BODY_SNAKE for legless, BODY_SLIME for amorphous, BODY_GHOST for incorporeal.

Racial characteristic flags include DUMBANIMAL (wild behavior), BONELESS (no skeleton), WINGED (natural flight), CLIMBER (wall climbing), EXTRAPLANAR (from other planes), AQUATIC (water breathing), FOURLEGGED (quadruped), COLDBLOODED (temperature dependent), RIDABLE (mountable), MAGICFLY (magical flight), FEATHERED (avian).

### Stat Type Enumeration

The statTypeT enum defines 14 positions: STAT_STR (0) through STAT_LUC (12) for active stats, STAT_EXT (13) reserved always zero. MAX_STATS (14) sizes arrays. The 13 active stats divide into physical (STR, BRA, CON, DEX, AGI, SPE), mental (INT, WIS, FOC, PER), social (CHA), and intangible (KAR, LUC).

The statSetT enum specifies query layers: STAT_CHOSEN for player allocation, STAT_CURRENT for temporary modified value, STAT_NATURAL for permanent combined value, STAT_RACE for racial baseline, STAT_AGE for age modifiers, STAT_TERRITORY for homeland bonuses.

Stat ranges span 5 (minimum crippled) to 205 (maximum superhuman) with 105 as neutral baseline. The 200-point range enables extreme specialization. Default initialization sets all stats to 150 (above baseline but not maximum) except STAT_EXT which remains 0.

### Immunity Types

The 28 immunity types cover elemental damage (IMMUNE_HEAT, IMMUNE_COLD, IMMUNE_ACID, IMMUNE_ELECTRICITY, IMMUNE_AIR, IMMUNE_EARTH, IMMUNE_WATER), physical damage (IMMUNE_PIERCE, IMMUNE_SLASH, IMMUNE_BLUNT), magical protection (IMMUNE_NONMAGIC, IMMUNE_PLUS1, IMMUNE_PLUS2, IMMUNE_PLUS3, IMMUNE_ENERGY, IMMUNE_DRAIN, IMMUNE_HOLY), status effects (IMMUNE_SLEEP, IMMUNE_PARALYSIS, IMMUNE_CHARM, IMMUNE_FEAR, IMMUNE_DISEASE, IMMUNE_SUMMON), and conditions (IMMUNE_POISON, IMMUNE_SUFFOCATION, IMMUNE_SKIN_COND, IMMUNE_BONE_COND, IMMUNE_BLEED).

Resistance percentages range 0 (no protection) to 100 (complete immunity). Negative values indicate vulnerability amplifying damage. Racial, equipment, and spell resistances stack additively then cap at 100%.

### TBeing Query Methods

Class queries: hasClass tests bitmask membership with exact parameter controlling partial versus complete match, getClass returns raw bitmask, getLevel retrieves level for classIndT index, getClassLevel gets level by bitmask, getMaxLevel returns highest across all classes, bestClass returns classIndT with maximum level.

Multiclass queries: isSingleClass tests single bit set, isDoubleClass tests exactly two, isTripleClass tests three or more, howManyClasses counts active bits, NumClasses global counts bits in passed bitmask.

Display methods: getProfName returns slash-separated class names, getProfAbbrevName returns abbreviated form (single class shows first four characters, multiclass shows single-letter codes).

Stat queries: getStat retrieves value for statSetT layer and statTypeT type, setStat updates layer value, addToStat modifies by delta. Convenience predicates wrap statSelfCheck: isStrong, isDextrous, isAgile, isTough, isBrawny, isIntelligent, isWise, isFast, isFocused, isPerceptive, isCharismatic, isLucky.

Stat modifiers: plotStat wraps plotValue for power-law scaling, getStatMod extracts amplified deviation from baseline. Specialized accessors: getStrDamModifier, getConHealthModifier, getDexReaction, getAgiReaction, getWisDamModifier, getIntModForPracs, getChaShopPenalty.

Resource accessors: getMana, getMaxMana, setMana, addToMana for mana. getPiety, setPiety, addToPiety, pietyLimit, noPiety for piety. getLifeforce, setLifeforce, addToLifeforce, getMaxLifeforce, noLifeforce for lifeforce. getMove, getMaxMove, setMove, addToMove, tooTired for movement. getHit, getMaxHit, setHit, addToHit for hit points.

Race queries: getRace returns race_t enum, getMyRace returns Race pointer, isSameRace compares with another being. Characteristic delegates: isHumanoid, isAquatic, isFourLegged, isWinged, isDumbAnimal, isLycanthrope, isColdBlooded, hasNoBones.

### Race Class Members

Core identification: raceType holds race_t enum value, Kingdom specifies lore_t category. Display names: singular_name, plural_name, proper_name for text output. Physical form: bodyType specifies body_t anatomy, racialCharacteristics holds bitflags.

Stat foundation: baseStats contains Stats instance with 13 baseline values. Immunities: naturalImmunities holds Immunities instance with 28 resistance percentages. Special abilities: talents holds bitmask of up to 9 racial talents.

Resource modifiers: hpMod adds flat HP per level, moveMod modifies movement pool, manaMod adds mana per level. Perception: searchMod adds flat search bonus, lineOfSightMod extends scan range, visionBonus improves detail. Consumption: foodMod scales fullness from eating, drinkMod scales satiation from drinking.

Physical dimensions via dice notation: baseAge plus ageNumDice d ageDieSize for starting age. baseMaleHeight plus maleHtNumDice d maleHtDieSize and baseFemaleHeight plus femaleHtNumDice d femaleHtDieSize for height. baseMaleWeight plus maleWtNumDice d maleWtDieSize and baseFemaleWeight plus femaleWtNumDice d femaleWtDieSize for weight.

Corpse handling: corpse_const multiplies corpse size and weight, tDissectItem array holds up to two item drops from dissection.

Query methods: isWinged, isFourLegged, hasNoBones, hasMagicFly, isAquatic, isRidable, isDumbAnimal, isClimber, isExtraplanar, isColdBlooded, isFeathered test characteristic flags. hasTalent tests talent bitmask. getImmunity retrieves resistance percentage for damage type.

### Stats Class Interface

Internal storage uses short values array sized MAX_STATS (14). Access via get for retrieval, set for assignment returning new value, add for modification returning new value. All operations bounds-check against statTypeT enum range.

Constructor initializes all stats to 150 except STAT_EXT which remains 0. This places characters above neutral baseline (105) but below maximum (205) by default.

The Stats class provides simple array-like access without game logic. All calculations and layering happen in TBeing::getStat which composes Stats instances from multiple sources.

### Global Data Structures

The Races array holds 127 Race pointers indexed by race_t value. Initialized at boot via initRaces which constructs Race instances and loads from lib/races/ files. Access pattern uses direct indexing: Races[RACE_DWARF] retrieves dwarf template.

The classInfo array holds 9 class_info structs indexed by classIndT. Contains static class definitions including enabled flag, level index, bitmask value, name, disciplines, practice multiplier, HP per level, abbreviation.

## Implementation

### Class Level Tracking

The playerData structure stores level array sized MAX_SAVED_CLASSES (11) holding ubyte per class plus max_level ubyte for highest. The Class field stores unsigned short bitmask of active classes. The doneBasic array tracks whether each class completed basic skill training.

When setting class, setClass updates the Class bitmask. When advancing level, setLevel modifies specific array index then calcMaxLevel recalculates max_level by iterating all positions finding maximum value. This redundant storage optimizes frequent max level queries.

The startLevels initialization sets level 1 for all enabled classes in bitmask. During character creation, Class initializes from player choice then startLevels populates level array. Multiclass characters receive level 1 in each chosen class.

Level queries like getLevel perform array indexing converting classIndT to offset. getClassLevel converts bitmask to classIndT via CountBits then indexes array. getMaxLevel simply returns cached max_level avoiding recalculation.

### Resource Pool Calculation

Maximum mana combines class-specific formula with INT stat modifier via plotStat then adds racial manaMod times level. The manaLimit method implements this calculation caching result in maxMana. Regeneration via manaGain computes gain per round using INT modifier and position (sleeping fastest, standing slowest).

Maximum piety uses floating-point pietyLimit calculation based on WIS and level with class-specific scaling. Spell costs in spellInfo definitions undergo division by 4 enabling fractional costs like 1.25 from PRAY_025 constant (5 / 4). The pietyGain method calculates regeneration from WIS modifier, worship state, and position.

Maximum lifeforce scales with level not stats via lifeforceLimit. The lifeforceGain regeneration uses level and position but ignores stat modifiers. This makes shaman resource pool simpler than mage/cleric equivalents.

Maximum HP combines class hp_per_level from classInfo with CON modifier via plotStat plus racial hpMod. Multiclass characters sum contributions from each class level. Non-tank classes (non-warrior/deikhan) multiply base by 35/50 ratio. HP does not regenerate naturally requiring rest or healing.

Maximum movement combines base formula with racial moveMod. Movement regeneration depends on position and stamina stats. Actual movement costs vary by terrain type, burden, and position (crawling costs more).

### Stat Layer Composition in getStat

For STAT_RACE, directly return race->baseStats.get(whichStat). For STAT_CHOSEN, directly return chosenStats.get(whichStat). For STAT_CURRENT, directly return curStats.get(whichStat).

For STAT_AGE, calculate age from age()->year minus getBaseAge() plus 17, skip if vampire, call age_mod_for_stat with current age and stat type. For STAT_TERRITORY, call territory_adjustment with player.hometerrain and stat type.

For STAT_NATURAL, chain all sources: start with STAT_RACE baseline, add STAT_CHOSEN allocation, add STAT_AGE modifier unless vampire, add STAT_TERRITORY bonus, add skill bonuses (SKILL_IRON_MUSCLES adds getSkillValue / 8 to STR if known), add vampire bonuses (+25 to STR/SPE/CHA if vampire).

The STAT_CURRENT value comes from curStats which affectModify updates. Initial curStats copies STAT_NATURAL then equipment and spells modify via affectModify. This maintains separation between natural abilities and temporary boosts.

### Age Modifier Calculation

The age_mod_for_stat function uses switch statement on age ranges returning stat-specific modifiers. Physical stats (STR, BRA, CON, DEX, AGI, SPE) peak at youth with positive modifiers from 16-30 then decline linearly through middle age turning negative after 40.

STR receives +10 from 16-30, +3 at 40, -2 at 50, -5 at 60, -7 at 70, -10 at 80. AGI receives +10 from 16-20, +5 at 25, 0 at 30, -2 at 40, -4 at 50, -6 at 60, -7 at 70, -8 at 80. DEX follows similar pattern starting decline earlier. BRA maintains bonus longer declining slower.

Mental stats (INT, WIS, FOC, PER, CHA) receive small penalties in youth (-5 for INT/WIS at 16-20) representing immaturity. They remain neutral through middle age then decline modestly in old age (-2 at 50, -3 at 60, -4 at 70, -5 at 80).

Vampires bypass age_mod_for_stat entirely via check in getStat(STAT_NATURAL) returning 0 for STAT_AGE layer. They instead receive flat +25 bonus to STR, SPE, CHA regardless of age representing undead enhancement.

### Territory Adjustment Tables

The territory_adjustment function uses nested switch on territory type then stat type. Urban territory grants +20 INT, +20 WIS, +20 CHA, +20 SPE representing education and culture while penalizing -20 CON, -10 FOC, -20 KAR representing soft lifestyle.

Mountain territory grants +15 BRA, +20 CON, +10 PER, +20 KAR representing rugged hardy life while penalizing -20 INT, -15 WIS, -15 FOC, -20 CHA representing isolation. Forest similar but slightly less extreme. Hill provides intermediate bonuses.

Plains territory grants +5 BRA, +5 CON, +15 PER, +5 KAR with -10 INT, -5 CHA representing outdoor life with better visibility. Recluse maximizes durability (+15 BRA, +25 CON, +30 KAR) while devastatingly penalizing social (-30 CHA) and mental (-25 INT, -15 WIS).

Villager provides small balanced bonuses (+10 INT, +10 WIS, +10 CHA) with modest penalties (-10 BRA, -10 CON, -10 KAR). Mariner gives small outdoor bonuses (+5 to BRA, CON, PER, KAR) with small penalties to mental/social.

### plotValue Power Law Implementation

The plotValue template calculates midline as (upperBound - lowerBound) / 2 + lowerBound yielding 105 for stat range 5-205. Clamp value to bounds using min/max. Branch on value versus midline.

Upper half (value >= 105) computes coefficient A as (maxValue - average) / (pow(205, power) - pow(105, power)), constant B as average - pow(105, power) * A, returns A * pow(value, power) + B. This creates curve from average at 105 to maxValue at 205.

Lower half (value < 105) computes coefficient A as (average - minValue) / (pow(105, power) - pow(5, power)), constant B as minValue - pow(5, power) * A, returns A * pow(value, power) + B. This creates curve from minValue at 5 to average at 105.

The power parameter (default 1.4) controls curvature: higher values create steeper curves amplifying extremes more. Linear scaling would use power 1.0 but non-linear 1.4 rewards specialization by making high stats disproportionately powerful.

### Equipment Affect Flow

When equipping item, equipChar iterates obj->affected array. For each affectedData entry with location not APPLY_NONE, call affectModify(location, modifier, 0, true) where location maps to statTypeT and modifier provides value. If item has bitvector, separate call passes bitvector with 0 modifier.

The affectModify implementation branches on add parameter. If true, call curStats.add(loc, mod) adding modifier to current stat value. If false, call curStats.add(loc, -mod) subtracting modifier. Then handle bitvector via setAffFlags(bitv, add) setting or clearing flags.

Unequipping reverses via equipChar passing FALSE for equip parameter. The function calls affectModify with same locations and modifiers but add false, subtracting bonuses. This restores curStats to pre-equipment state.

Spell affects use affectedData structures stored in TBeing::affectedBySpell list. When applying spell affect, create affectedData with location, modifier, duration, bitvector then call affectModify with add true. When duration expires or dispel removes, call affectModify with add false.

### Class AI Dispatcher

The classStuff method in TMonster calls bestClass to determine highest level class then switches on result. Each case invokes specialized function: fighterMove for WARRIOR_LEVEL_IND, mageMove for MAGE_LEVEL_IND, clerMove for CLERIC_LEVEL_IND, thiefMove for THIEF_LEVEL_IND, monkMove for MONK_LEVEL_IND, shamanMove for SHAMAN_LEVEL_IND, deikhanMove for DEIKHAN_LEVEL_IND, rangMove for RANGER_LEVEL_IND.

Each specialized function checks combat situation: HP percentage determines urgency (low HP prioritizes healing or escape), opponent count affects target selection, resource availability (mana, piety, lifeforce) gates expensive actions. Functions return TRUE if action taken, FALSE if skipped allowing fallback to basic melee.

Warrior fighterMove selects from bash, bodyslam, spin, kick, disarm based on opponent position and HP. Mage mageMove casts offensive spells checking mana before each. Cleric clerMove prioritizes healing self or group when damaged, casts harm on opponents when healthy. Thief thiefMove attempts backstab from hiding or stabbing from engaged.

Multiclass resolution uses only bestClass avoiding complex combined tactics. This simplifies NPC behavior preventing action conflicts (trying to both cast and bash). Players access all class abilities through command parsing but NPCs stay in character using primary class only.

### Race Initialization

At boot, game_loop calls initRaces before loading zones. This function iterates race_t from 0 to MAX_RACIAL_TYPES (127), constructs new Race instance passing race enum value, calls initRace method passing RaceNames string array entry, stores result in Races array.

The Race constructor initializes all members to defaults: stats to 105 baseline, immunities to 0%, talents to 0, modifiers to 0, characteristic flags to 0. The initRace method loads from lib/races/filename parsing text file with race definitions.

Race file format specifies stat baselines, immunity percentages, talent flags, body type, lore category, physical dimensions in dice notation, corpse parameters. Parser reads key-value pairs populating Race members. Any undefined values retain constructor defaults.

After initialization, Races array provides O(1) lookup by race type. All beings with same race share single Race instance via pointer. This flyweight pattern optimizes memory for large mob populations. Race instances remain const after boot preventing accidental modification.

## Troubleshooting

### Class Queries Return Wrong Results

Check Class bitmask value not classIndT index. Queries like hasClass expect bitmask constants (CLASS_MAGE = 1) not array indices (MAGE_LEVEL_IND = 0). Convert between representations using getClassNum and getClassIndNum.

For exact matching, verify EXACT_YES parameter. hasClass with EXACT_NO returns true for partial overlap (character has any specified class). EXACT_YES requires all specified class bits set (character has all classes, may have others too).

When checking multiclass, use howManyClasses not comparison against multiple CLASS constants. Bitmask combinations like CLASS_MAGE | CLASS_CLERIC create non-obvious values. Counting bits via howManyClasses avoids magic numbers.

### Stats Show Unexpected Values

Distinguish query layer via statSetT parameter. STAT_CURRENT includes temporary equipment and spell affects. STAT_NATURAL excludes equipment showing permanent character power. STAT_RACE shows only racial baseline ignoring all modifiers.

When equipment removal leaves stats high, check for orphaned affects. affectModify calls with add true must have matching add false calls. Equipment stacking bugs occur when unequip skips affectModify reversal leaving bonus applied.

For age-related stat confusion, calculate actual age correctly: age()->year - getBaseAge() + 17. Check vampire status via isVampire which bypasses age_mod_for_stat. Vampires receive flat +25 to STR/SPE/CHA instead of age curve.

When territory bonuses seem wrong, verify player.hometerrain matches expected territory type. Territory modifiers apply once at creation and never change. Character relocation does not update territory bonuses.

### Resource Pools Drain Too Fast

Check regeneration rate calculations using position. Sleeping provides maximum regeneration, standing minimum. Characters standing in combat regenerate slower than resting. Verify position via getPosition comparing against POSITION_SLEEPING, POSITION_RESTING, POSITION_SITTING, POSITION_STANDING, POSITION_FIGHTING.

For mana issues, verify INT stat via getStat(STAT_CURRENT, STAT_INT). Equipment removal drops INT lowering regeneration rate and maximum pool. Low INT characters regenerate slowly. Check manaGain result showing per-round gain.

For piety confusion, remember spell costs in spellInfo divide by 4. PRAY_100 constant (20) becomes actual cost 5.0 after division. Costs appearing 4x higher than expected indicate missing division. Piety uses double precision enabling fractional costs.

For lifeforce, check level not stats. Lifeforce scales with character level ignoring stat modifiers unlike mana (INT) and piety (WIS). Low-level shamans have small lifeforce pools regardless of stats.

### Multiclass Experience Feels Wrong

Remember quadratic penalty from double division. Two-class character divides by 2 twice yielding 1/4 total (25% per class). Three classes divide by 3 twice yielding 1/9 total (11% per class). This is intentional design strongly discouraging excessive multiclassing.

Check that both gain_exp divisions occur in limits.cc. First division by howManyClasses applies multiclass penalty, second division applies again creating quadratic effect. Single division indicates bug losing intended penalty.

Practice point division happens separately in pracsPerLevel. Verify multiclass check divides by howManyClasses reducing points per level. Multiclass characters advance slower in both experience and skill learning.

### Racial Immunities Not Applying

Check resistance stacking calculation summing racial, equipment, and spell sources. Verify each component via race->getImmunity, equipment affects with APPLY_IMMUNITY location, and spell affects. Total should not exceed 100% cap.

Negative immunity percentages indicate vulnerability not resistance. IMMUNE_HOLY -50 for vampires means they take 150% damage from holy (50% extra). Distinguish resistance (positive) from vulnerability (negative).

For IMMUNE_NONMAGIC confusion, verify weapon magical status. Non-magical weapons check this immunity, magical weapons bypass. +1 weapons check IMMUNE_PLUS1, +2 check IMMUNE_PLUS2, +3 check IMMUNE_PLUS3. High-magic weapons bypass low-tier immunity.

When flying creatures take falling damage, verify WINGED flag not just MAGICFLY. Natural flight (WINGED) provides constant flight, magical flight (MAGICFLY) can be dispelled. Check racialCharacteristics bitfield contains correct flag.

### plotStat Returns Confusing Values

Verify parameter order: minValue for stat 5, maxValue for stat 205, average for stat 105. Passing (1.25, 0.8, 1.0) instead of (0.8, 1.25, 1.0) inverts curve creating penalties for high stats.

Check stat value bounds between 5-205. Out-of-range values get clamped to bounds. Stat 300 clamps to 205 returning maxValue. Stat -10 clamps to 5 returning minValue. No extrapolation occurs beyond defined range.

For specialized uses, verify power parameter. Default 1.4 creates typical curve. Higher power (2.0 for karma) amplifies extremes more. Power 1.0 yields linear scaling losing non-linear benefits.

When getStatMod produces unexpected results, trace formula: ((plotStat - 1) * multiplier) + 1. Multiplier amplifies deviation from baseline. plotStat 1.1 with multiplier 2 yields 1.2 not 2.2. The +1 at end restores baseline.

### Combat AI Using Wrong Tactics

Check bestClass return value showing highest level class. Multiclass character with warrior 10 / mage 25 uses mageMove not fighterMove. NPCs follow primary class (highest level) not combined tactics.

Verify resource checks before expensive actions. mageMove should check getMana() >= spell cost before casting. Failed check should skip to next tactic. Missing checks cause NPCs attempting impossible actions.

For healing priority issues, check HP percentage calculation in clerMove. Low HP should prioritize healing self, high HP should prioritize harming opponents. Inverted logic causes clerics harming themselves while damaged.

When thieves not backstabbing, verify position and visibility checks. Backstab requires hidden or behind position. Check isHidden() and canSee() results. Failed preconditions should degrade to stab or basic melee.
