---
title: Territory/Homeland System
category: understanding
keywords: [territoryT, home_terrains, territory_adjustment, hometerrain, playerData, stat modifiers, character creation, sub-race]
related: [character-foundation.md, race-system.md, stats-attributes.md]
primary_symbols:
  functions: [territory_adjustment, getStat]
  classes: [playerData, charFile, TPlayerRace]
  files: [code/code/misc/stats.cc, code/code/sys/create_character.cc, code/code/misc/enum.h]
---

## Overview

The territory system provides character background customization during creation by allowing players to select a homeland that modifies base stats. Each territory represents a different upbringing or environment, acting as permanent sub-race flavoring on top of racial identity.

Territory selection occurs immediately after race selection during character creation. Each race has 4-8 available territories with unique names and lore. Territories modify 10 of the 13 base stats with adjustments ranging from -30 to +30 points. The choice is permanent and stored in the character file.

Most territories are designed to be zero-sum or slightly negative to ensure balanced tradeoffs. Urban territories intentionally provide a net -20 penalty as a complexity trap for new players. The system does not grant skills or abilities, only stat modifications.

Territories are defined in the territoryT enum and grouped into eight conceptual categories that share stat adjustment patterns. Extended races introduced after the original six map their territories to the human model to maintain consistency and reduce code duplication.

## Patterns

### Querying Territory Information

Access territory data through the player structure and use the home_terrains array for display names. Query specific stat bonuses using getStat with STAT_TERRITORY, or include territory effects in natural stats with STAT_NATURAL.

Territory type checks compare the hometerrain enum value directly. Group territories by category by checking multiple enum values against the target pattern.

### Territory Validation

When setting territory programmatically or through admin commands, validate that the territory enum value falls within the valid range for the character's race. Each race has a contiguous block of territory enums.

HOME_TER_NONE represents unknown territory and is valid for NPCs, returning zero for all stat modifiers.

### Stat Integration

Territory modifiers apply during natural stat calculation through territory_adjustment. This function is called by getStat when computing STAT_NATURAL, adding territory bonuses after racial base stats, player-allocated stats, and age modifiers.

### Extended Race Mapping

Races defined after the base six use a modulo formula to map their territory enums to equivalent human territory types. This allows new races to reuse the established stat adjustment patterns without defining custom modifiers.

## Reference

### Territory Categories and Stat Adjustments

**Urban Dweller (City)**
Applies to urban territories across all races. CHA +20, KAR -20, INT +20, WIS +20, FOC -10, SPE +20, CON -20, BRA -20, PER -10. Total net -20. Represents high social contact, good education, but poor health and sedentary lifestyle.

**Villager/Tribal**
Applies to villager and tribal territories. CHA +10, KAR -10, INT +10, WIS +10, CON -10, BRA -10. Total net zero. Balanced modifiers representing moderate settlement life.

**Plains/Grasslands**
Applies to plains and grasslands territories. CHA -5, KAR +5, INT -10, WIS 0, CON +5, BRA +5, FOC -10, PER +15, SPE -5, AGI 0. Total net zero. Heightened awareness from open terrain, substandard education.

**Recluse/Hermit**
Applies to recluse territories. CHA -30, KAR +30, INT -25, WIS -15, CON +25, BRA +15, FOC +15, PER -15, SPE 0, AGI 0. Total net zero. Extreme isolation with high physical health and focus but severe social deficits.

**Hill Dweller**
Applies to hill territories. CHA -10, KAR +10, INT -15, WIS -5, CON +10, BRA +10, FOC -15, PER +10, SPE 0, AGI +5. Total net zero. Physical labor lifestyle with terrain navigation advantages.

**Mountain Dweller**
Applies to mountain and snow territories. CHA -20, KAR +20, INT -20, WIS -15, CON +20, BRA +15, FOC -15, PER +10, SPE 0, AGI +5. Total net zero. Rugged lifestyle with extreme health benefits and isolation penalties.

**Forest Dweller**
Applies to forest and woodland territories. CHA -15, KAR +15, INT -15, WIS -15, CON +15, BRA +15, FOC -15, PER +10, SPE 0, AGI +5. Total net zero. Nature-focused existence balancing physical health against educational deficits.

**Mariner/Sea Dweller**
Applies to maritime, sea, and swamp territories. CHA -5, KAR +5, INT -5, WIS -5, CON +5, BRA +5, FOC -5, PER +5, SPE 0, AGI 0. Total net zero. Minimal modifiers representing balanced seafaring life.

### Territory Enum Organization

The territoryT enum in enum.h organizes territories by race. Each race has a contiguous block of enum values starting with HOME_TER_NONE at position zero.

Humans have eight territories: urban, villager, plains, recluse, hills, mountain, forest, mariner. Elves have seven: urban, tribal, plains, snow, recluse, wood, sea. Dwarves have five: urban, villager, recluse, hill, mountain. Gnomes have four: urban, villager, hill, swamp. Ogres have three: villager, plains, hill. Hobbits have six: urban, shire, grasslands, hill, woodland, maritime.

Extended races (Goblin, Gnoll, Troglodyte, Orc) follow the eight-territory human model with race-specific names. Advanced races (Bullywug, Kalysian, Aarakocra, Troll) have unique territory sets with some values marked UNUSED.

### Display Names

The home_terrains array in constants.cc maps territory enum values to display strings. Names use lowercase conventions like "urban dweller", "wood elf", "mountain dwarf" for consistency across the interface.

### Storage Format

The charFile struct stores territory as a single unsigned byte in the hometerrain field. This binary format is frozen and cannot be modified without migration. At runtime, the playerData struct holds territory as a territoryT enum value.

### Character Creation Flow

During character creation at state CON_CREATION_TRAITS, the nannyRaces array provides race-to-territory mapping. Each race entry specifies its available territories through an array and count. The interface displays territory options with checkbox-style selection markers.

Territory help files exist at lib/help/territory help <race> to explain lore and mechanical implications to players during selection.

## Implementation

### Territory Adjustment Function

The territory_adjustment function in stats.cc implements the core mapping from territory enum to stat modifier. It first normalizes extended race territories to human equivalents using modulo arithmetic, then switches on the normalized territory value to return the appropriate modifier for the requested stat.

For extended races, the normalization formula is: territoryT normalized = territoryT(1 + ((territory - HOME_TER_GOBLIN_URBAN) % 8)). This maps position 0 to urban, position 1 to villager, position 2 to plains, continuing through the eight-territory cycle.

The switch statement groups territories by category. Each case block handles all territories that share the same stat adjustment pattern, returning the category-specific modifier for the queried stat.

### Stat Calculation Integration

The getStat method in TBeing integrates territory modifiers during STAT_NATURAL calculation. It builds the natural stat by summing racial base stats from race->baseStats, player-allocated points from chosenStats, age modifiers from age_mod_for_stat, and territory bonuses from territory_adjustment.

When queried with STAT_TERRITORY, getStat returns only the territory modifier by calling territory_adjustment directly without other components.

### Character Creation Menu

The character creation system in create_character.cc uses the nannyRaces array to populate territory selection menus. Each TPlayerRace entry contains a pointer to a territory enum array and a count of available territories.

The menu displays territories with checkbox indicators showing current selection. The default selection is the first territory in the race's array, typically urban dweller or equivalent. Players can cycle through options before confirming their choice.

### Admin Modification

The @set command supports territory modification through the "territory" parameter. It validates that the provided territory value falls within the valid range for the character's race by checking against race-specific minimum and maximum territory enum values.

After validation, the command updates player.hometerrain and provides confirmation output using the home_terrains display name. The @stat command displays territory as part of character inspection using the same display name array.

### Display to Players

The attribute command shows territory in character background description by formatting home_terrains with article selection (a/an) based on vowel detection. Output follows the pattern: "You grew up as [article] [territory name] and began adventuring at the age of [age]."

The character creation launchpad combines race and territory on a single line formatted as "Race/Homeland: [race]/[territory]".

## Troubleshooting

### Territory Value Corruption

If character files show invalid territory values, check for enum insertion mid-list. New territories must be appended before MAX_HOME_TERS, never inserted, as existing character files store raw enum values that would shift.

Validate territory-race compatibility when debugging character data. A dwarf with HOME_TER_ELF_WOOD indicates file corruption or improper admin modification.

### Stat Calculation Discrepancies

When natural stats don't match expectations, verify getStat is called with STAT_NATURAL, not STAT_CURRENT or other stat sets. Territory modifiers only apply to natural stats, not current stats which include equipment and affects.

Check that territory_adjustment is being called during stat calculation. Missing territory bonuses indicate the integration in getStat may have been bypassed by custom code paths.

### Extended Race Territory Mapping

If extended race territories show unexpected stat modifiers, verify the normalization formula is executing correctly. The modulo operation should map territory position to human equivalent: position 0 maps to urban, position 1 to villager, etc.

UNUSED territory values for advanced races return zero for all stats. If an advanced race shows non-zero modifiers for unused territories, the territory_adjustment switch statement may have incorrect case mappings.

### Creation Menu Issues

Missing territories in the creation menu indicate the nannyRaces array doesn't include the expected territory enum array. Verify the race entry specifies both the territory array pointer and the correct count.

Territory help file not found errors occur when lib/help/territory help <race> doesn't exist for a race that offers territory selection. All races with territories require corresponding help files.

### Admin Command Validation

The @set territory command rejecting valid territories suggests the validation range check is too restrictive. Verify the race-specific minimum and maximum territory enum values match the actual territory enum definition for that race.

HOME_TER_NONE is valid for all races and should pass validation. If it's rejected, the validation logic is incorrectly excluding the zero value.
