---
title: Territory/Homeland System
description: The territory system allows players to customize character backgrounds during creation by selecting homelands that provide stat modifiers on top of racial base stats, acting as permanent sub-race flavoring.
keywords:
  - territoryT
  - home_terrains
  - territory_adjustment
  - hometerrain
  - playerData
  - nannyRaces
  - CON_CREATION_TRAITS
  - getStat
  - STAT_TERRITORY
  - STAT_NATURAL
  - HOME_TER_URBAN
  - HOME_TER_RECLUSE
  - stat modifiers
  - character creation
category: Understanding Systems

  - stats-attributes.md
  - charfile-format.md
  - set-command.md
last_updated: 2026-01-29
source_files:
  - code/code/misc/enum.h
  - code/code/misc/constants.cc
  - code/code/misc/stats.cc
  - code/code/sys/create_character.cc
  - code/code/misc/charfile.h
  - code/code/misc/being.h
  - code/code/cmd/cmd_attribute.cc
  - code/code/cmd/cmd_set.cc
  - code/code/cmd/cmd_stat.cc
related: [character-foundation.md]
---

# Territory/Homeland System

This document describes SneezyMUD's territory (homeland) system, including how players select their territory during character creation, available territories by race, and how territory affects character stats.

## Overview

The territory system allows players to customize their character's background beyond race selection. Each territory represents a different upbringing or homeland that provides stat modifiers on top of racial base stats. Essentially, territories act as **sub-races** that fine-tune character attributes.

**Key characteristics:**
- Territory is selected during character creation, immediately after choosing race
- Each race has a unique set of available territories (4-8 options)
- Territories modify 10 of the 13 base stats (STR, BRA, CON, DEX, AGI, INT, WIS, FOC, PER, CHA, KAR, SPE)
- Modifications range from -30 to +30 points per stat
- Territory choice is permanent and stored in the character file
- Territories do NOT grant additional skills (only stat modifications)

**Source files:**
- `code/code/misc/enum.h` - `territoryT` enum (lines 425-541)
- `code/code/misc/constants.cc` - `home_terrains[]` display names (lines 1379-1506)
- `code/code/misc/stats.cc` - `territory_adjustment()` function (lines 629-871)
- `code/code/sys/create_character.cc` - Territory selection during creation (lines 68-148, 599-656)
- `code/code/misc/charfile.h` - `hometerrain` storage (line 46)

## territoryT Enum

The `territoryT` enum defines all available territories, organized by race.

**Source:** `code/code/misc/enum.h:425-541`

### Base Race Territories

| Race | Territory Enum | Display Name |
|------|----------------|--------------|
| **Human** | `HOME_TER_HUMAN_URBAN` | urban dweller |
| | `HOME_TER_HUMAN_VILLAGER` | villager |
| | `HOME_TER_HUMAN_PLAINS` | plains person |
| | `HOME_TER_HUMAN_RECLUSE` | recluse |
| | `HOME_TER_HUMAN_HILL` | hills person |
| | `HOME_TER_HUMAN_MOUNTAIN` | mountaineer |
| | `HOME_TER_HUMAN_FOREST` | forester |
| | `HOME_TER_HUMAN_MARINER` | mariner |
| **Elf** | `HOME_TER_ELF_URBAN` | urban dweller |
| | `HOME_TER_ELF_TRIBE` | tribal villager |
| | `HOME_TER_ELF_PLAINS` | plains elf |
| | `HOME_TER_ELF_SNOW` | snow elf |
| | `HOME_TER_ELF_RECLUSE` | recluse |
| | `HOME_TER_ELF_WOOD` | wood elf |
| | `HOME_TER_ELF_SEA` | sea elf |
| **Dwarf** | `HOME_TER_DWARF_URBAN` | urban dweller |
| | `HOME_TER_DWARF_VILLAGER` | villager |
| | `HOME_TER_DWARF_RECLUSE` | recluse |
| | `HOME_TER_DWARF_HILL` | hill dwarf |
| | `HOME_TER_DWARF_MOUNTAIN` | mountain dwarf |
| **Gnome** | `HOME_TER_GNOME_URBAN` | urban dweller |
| | `HOME_TER_GNOME_VILLAGER` | villager |
| | `HOME_TER_GNOME_HILL` | hill gnome |
| | `HOME_TER_GNOME_SWAMP` | swamp gnome |
| **Ogre** | `HOME_TER_OGRE_VILLAGER` | villager |
| | `HOME_TER_OGRE_PLAINS` | plains ogre |
| | `HOME_TER_OGRE_HILL` | hill ogre |
| **Hobbit** | `HOME_TER_HOBBIT_URBAN` | urban dweller |
| | `HOME_TER_HOBBIT_SHIRE` | shire hobbit |
| | `HOME_TER_HOBBIT_GRASSLANDS` | grasslands hobbit |
| | `HOME_TER_HOBBIT_HILL` | hill hobbit |
| | `HOME_TER_HOBBIT_WOODLAND` | woodland hobbit |
| | `HOME_TER_HOBBIT_MARITIME` | maritime hobbit |

### Extended Race Territories

Extended races (Goblin, Gnoll, Troglodyte, Orc) follow the human territory model with race-specific names.

### Advanced Race Territories

Advanced races (Bullywug, Kalysian, Aarakocra, Troll) have unique territory sets with some marked as UNUSED.

## Stat Modifiers by Territory Type

The `territory_adjustment()` function maps territories to stat modifiers. The system groups territories into 8 conceptual categories that share stat adjustments.

**Source:** `code/code/misc/stats.cc:629-871`

### Urban Dweller (City)

**Applies to:** Human Urban, Elf Urban, Dwarf Urban, Gnome Urban, Hobbit Urban

| Stat | Modifier | Rationale |
|------|----------|-----------|
| CHA | +20 | High social contact |
| KAR | -20 | Cynicism from city life |
| INT | +20 | Good educational access |
| WIS | +20 | Learned wisdom |
| FOC | -10 | Low attention span (overstimulated) |
| SPE | +20 | Hustle and bustle lifestyle |
| CON | -20 | Poor health environment |
| BRA | -20 | Sedentary lifestyle |
| PER | -10 | Overstimulated senses |

**Total:** -20 (net penalty)

### Villager/Tribal

**Applies to:** Human Villager, Elf Tribe, Dwarf Villager, Gnome Villager, Ogre Villager, Hobbit Shire

| Stat | Modifier | Rationale |
|------|----------|-----------|
| CHA | +10 | Moderate social contact |
| KAR | -10 | Moderate cynicism |
| INT | +10 | Moderate education |
| WIS | +10 | Traditional wisdom |
| CON | -10 | Subpar health environment |
| BRA | -10 | Less physical labor |

**Total:** 0 (balanced)

### Plains/Grasslands

**Applies to:** Human Plains, Elf Plains, Ogre Plains, Hobbit Grasslands

| Stat | Modifier | Rationale |
|------|----------|-----------|
| CHA | -5 | Low contact with others |
| KAR | +5 | Upbeat outlook |
| INT | -10 | Substandard education |
| WIS | 0 | Natural wisdom |
| CON | +5 | Healthy environment |
| BRA | +5 | Physical lifestyle |
| FOC | -10 | Monotonous life |
| PER | +15 | Heightened awareness |
| SPE | -5 | Slower pace of life |
| AGI | 0 | Neutral |

**Total:** 0 (balanced)

### Recluse/Hermit

**Applies to:** Human Recluse, Elf Recluse, Dwarf Recluse

| Stat | Modifier | Rationale |
|------|----------|-----------|
| CHA | -30 | Extreme isolation |
| KAR | +30 | Upbeat, unspoiled soul |
| INT | -25 | No formal education |
| WIS | -15 | Limited life experience |
| CON | +25 | Very healthy environment |
| BRA | +15 | Self-reliance |
| FOC | +15 | High self-discipline |
| PER | -15 | Understimulated senses |
| SPE | 0 | Neutral |
| AGI | 0 | Neutral |

**Total:** 0 (balanced)

### Hill Dweller

**Applies to:** Human Hill, Dwarf Hill, Gnome Hill, Ogre Hill, Hobbit Hill

| Stat | Modifier | Rationale |
|------|----------|-----------|
| CHA | -10 | Low contact |
| KAR | +10 | Mildly upbeat |
| INT | -15 | Substandard education |
| WIS | -5 | Limited access to wisdom |
| CON | +10 | Healthy outdoor life |
| BRA | +10 | Physical labor |
| FOC | -15 | Monotonous existence |
| PER | +10 | Alert to dangers |
| SPE | 0 | Neutral |
| AGI | +5 | Terrain navigation |

**Total:** 0 (balanced)

### Mountain Dweller

**Applies to:** Human Mountain, Dwarf Mountain, Elf Snow

| Stat | Modifier | Rationale |
|------|----------|-----------|
| CHA | -20 | Very low contact |
| KAR | +20 | Upbeat outlook |
| INT | -20 | Poor educational access |
| WIS | -15 | Isolated living |
| CON | +20 | Extremely healthy |
| BRA | +15 | Rugged lifestyle |
| FOC | -15 | Simple existence |
| PER | +10 | Survival awareness |
| SPE | 0 | Neutral |
| AGI | +5 | Mountain climbing |

**Total:** 0 (balanced)

### Forest Dweller

**Applies to:** Human Forest, Elf Wood, Hobbit Woodland

| Stat | Modifier | Rationale |
|------|----------|-----------|
| CHA | -15 | Low contact |
| KAR | +15 | Upbeat outlook |
| INT | -15 | Substandard education |
| WIS | -15 | Nature-focused wisdom |
| CON | +15 | Healthy environment |
| BRA | +15 | Physical lifestyle |
| FOC | -15 | Simple existence |
| PER | +10 | Forest awareness |
| SPE | 0 | Neutral |
| AGI | +5 | Woodland agility |

**Total:** 0 (balanced)

### Mariner/Sea Dweller

**Applies to:** Human Mariner, Elf Sea, Gnome Swamp, Hobbit Maritime

| Stat | Modifier | Rationale |
|------|----------|-----------|
| CHA | -5 | Moderate isolation |
| KAR | +5 | Upbeat outlook |
| INT | -5 | Limited education |
| WIS | -5 | Practical wisdom only |
| CON | +5 | Healthy outdoor life |
| BRA | +5 | Physical labor |
| FOC | -5 | Unpredictable lifestyle |
| PER | +5 | Sea awareness |
| SPE | 0 | Neutral |
| AGI | 0 | Neutral |

**Total:** 0 (balanced)

## Extended/Advanced Race Territory Mapping

For races defined after the base six (Goblin and beyond), territories are mapped to the human model:

```cpp
// code/code/misc/stats.cc:647-648
if (ter >= HOME_TER_GOBLIN_URBAN)
    ter = territoryT(1 + ((ter - HOME_TER_GOBLIN_URBAN) % 8));
```

This formula converts extended race territories to their equivalent human territory type:
- Position 0 (Urban) maps to Urban stats
- Position 1 (Villager) maps to Villager stats
- Position 2 (Plains) maps to Plains stats
- etc.

## Character Creation Flow

Territory selection occurs during character creation, immediately after race selection.

**Source:** `code/code/sys/create_character.cc:599-656`

### Race-to-Territory Mapping

Each race defines its available territories in `nannyRaces[]`:

```cpp
// code/code/sys/create_character.cc:114-148
TPlayerRace nannyRaces[] = {
    {RACE_HUMAN, "Human", 0, humanTerr, cElements(humanTerr), Room::NEWBIE, ...},
    {RACE_GNOME, "Gnome", 0, gnomeTerr, cElements(gnomeTerr), Room::NEWBIE, ...},
    {RACE_ELVEN, "Elf", 0, elfTerr, cElements(elfTerr), Room::NEWBIE, ...},
    // ... etc
};
```

### Territory Selection Menu

When selecting territory, players see:

```
Please pick one of the following homelands for your Human.
Your current homeland selection is marked with an 'X'.

[X]  1. urban dweller
[ ]  2. villager
[ ]  3. plains person
[ ]  4. recluse
[ ]  5. hills person
[ ]  6. mountaineer
[ ]  7. forester
[ ]  8. mariner

Your choice of homeland will slightly impact the statistics of your character.
```

### Help Files

Each race has a territory help file explaining the lore and implications:
- `lib/help/territory help human`
- `lib/help/territory help elf`
- `lib/help/territory help dwarf`
- etc.

## Storage in Character File

Territory is stored in the binary character file as a single byte.

**Source:** `code/code/misc/charfile.h:46`

```cpp
class charFile {
    // ...
    short hometown;        // Starting room vnum
    ubyte hometerrain;     // Territory enum value (0-93)
    // ...
};
```

**Warning:** The charfile format is frozen. Never modify the struct layout without a migration strategy.

### Runtime Storage

In the active `TBeing` object, territory is stored in `playerData`:

```cpp
// code/code/misc/being.h:71
class playerData {
    // ...
    unsigned short hometown;       // Room vnum
    territoryT hometerrain;        // Territory enum
    // ...
};
```

## Integration with Stat System

Territory modifiers are applied during natural stat calculation.

**Source:** `code/code/misc/stats.cc:906-953`

```cpp
int TBeing::getStat(statSetT fromSet, statTypeT whichStat) const {
    switch (fromSet) {
        case STAT_NATURAL:
            amount = race->baseStats.get(whichStat);      // Racial base
            amount += chosenStats.get(whichStat);         // Player allocation
            amount += age_mod_for_stat(this, my_age, whichStat);  // Age
            amount += territory_adjustment(player.hometerrain, whichStat);  // Territory
            // ... additional modifiers
            return amount;
        case STAT_TERRITORY:
            return territory_adjustment(player.hometerrain, whichStat);
    }
}
```

### Querying Territory Stats

```cpp
// Get just the territory modifier for a stat
int terrBonus = ch->getStat(STAT_TERRITORY, STAT_STR);

// Get total natural stat including territory
int naturalSTR = ch->getStat(STAT_NATURAL, STAT_STR);
```

## Display to Players

### Attributes Command

The `attribute` command shows territory in the character description:

```cpp
// code/code/cmd/cmd_attribute.cc:631-635
sendTo(format("You grew up as %s %s and began adventuring at the age of %d.\n\r") %
       (sstring(home_terrains[player.hometerrain]).startsVowel() ? "an" : "a") %
       home_terrains[player.hometerrain] % getBaseAge());
```

Example output:
```
You grew up as an urban dweller and began adventuring at the age of 17.
```

### Character Launchpad

During creation, the launchpad shows race and territory together:

```
2. Race/Homeland      : Human/urban dweller
```

## Admin Commands

### @set territory

Immortals can modify a character's territory:

```cpp
// code/code/cmd/cmd_set.cc:428-430
mob->player.hometerrain = territoryT(parm);
sendTo(COLOR_MOBS, format("%s grew up as a %s.\n\r") %
       mob->getName() % home_terrains[parm]);
```

**Syntax:** `@set territory <character> <territory_value>`

The command validates that the territory value is appropriate for the character's race.

### @stat character

Shows territory in character stat display:

```cpp
// code/code/cmd/cmd_stat.cc:972
home_terrains[k->player.hometerrain];
```

## Design Decisions

### Why Zero-Sum Modifiers

Most territories are designed to be **zero-sum** or slightly negative. This ensures:
1. No territory is strictly dominant
2. Players make meaningful tradeoffs
3. Urban is intentionally suboptimal (-20 net) as a beginner trap

### Race-Specific Territories

Each race has custom territory names and lore to reinforce world-building:
- Dwarves have "hill dwarf" and "mountain dwarf" instead of generic names
- Elves have "wood elf", "snow elf", "sea elf" variants
- This provides sub-race flavor without mechanical complexity

### Mapping Extended Races

Rather than defining unique stat modifiers for every extended race territory, the system maps them to the human model. This:
1. Reduces code duplication
2. Ensures balance consistency
3. Makes new races easier to add

## Common Patterns

### Checking Territory in Code

```cpp
// Check specific territory
if (ch->player.hometerrain == HOME_TER_ELF_WOOD) {
    // Wood elf specific logic
}

// Get territory display name
sstring terrName = home_terrains[ch->player.hometerrain];

// Get territory stat modifier
int bonus = territory_adjustment(ch->player.hometerrain, STAT_PER);
```

### Territory Categories

```cpp
// Check if territory is "urban" type
bool isUrban = (ter == HOME_TER_HUMAN_URBAN ||
                ter == HOME_TER_ELF_URBAN ||
                ter == HOME_TER_DWARF_URBAN ||
                ter == HOME_TER_GNOME_URBAN ||
                ter == HOME_TER_HOBBIT_URBAN);
```

## Gotchas and Anti-Patterns

### Do Not Add New Territories Mid-Enum

The `territoryT` enum values are stored in binary character files. Inserting new values in the middle would corrupt existing characters. Always append new territories at the end before `MAX_HOME_TERS`.

### Validate Territory-Race Compatibility

When setting territory via admin commands or code, always validate that the territory is valid for the character's race:

```cpp
// code/code/cmd/cmd_set.cc:380-427 shows validation examples
if (mob->getRace() == RACE_HUMAN) {
    if ((parm != HOME_TER_NONE) &&
        ((parm < HOME_TER_HUMAN_URBAN) || (parm > HOME_TER_HUMAN_MARINER))) {
        sendTo("Bad value for human territory type.\n\r");
        return;
    }
}
```

### HOME_TER_NONE Edge Case

`HOME_TER_NONE` (value 0) is valid and represents "unknown" territory. It returns 0 for all stat modifiers. This is used for NPCs that don't have a meaningful homeland.

## Source Files Reference

| File | Purpose | Key Lines |
|------|---------|-----------|
| `code/code/misc/enum.h` | `territoryT` enum definition | 425-541 |
| `code/code/misc/constants.cc` | `home_terrains[]` display names | 1379-1506 |
| `code/code/misc/stats.cc` | `territory_adjustment()` stat modifiers | 629-871 |
| `code/code/sys/create_character.cc` | Territory arrays and selection | 68-148, 599-656 |
| `code/code/misc/charfile.h` | `hometerrain` storage field | 46 |
| `code/code/misc/being.h` | `playerData.hometerrain` | 71 |
| `code/code/cmd/cmd_attribute.cc` | Territory display to player | 631-635 |
| `code/code/cmd/cmd_set.cc` | Admin territory modification | 380-430 |
| `code/code/cmd/cmd_stat.cc` | Admin territory inspection | 972 |
| `lib/help/territory help *` | Player help files | - |

## Related Documentation

- [Race System](race-system.md) - How races define available territories and base stats
- [Stats and Attributes](stats-attributes.md) - How territory modifiers integrate with stat calculation
- [Character Creation](charfile-format.md) - Binary storage format for character data
- [Set Command](set-command.md) - Admin command for modifying territory
