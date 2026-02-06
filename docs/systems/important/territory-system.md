---
title: Territory/Homeland System
description: Permanent sub-race territories selected during character creation that modify base stats with different homeland options per race.
category: important
keywords: [homeland, sub-race, stat modifiers]
primary_symbols:
  functions: [territory_adjustment, getStat]
  classes: [TBeing]
  enums: [territoryT, HOME_TER_NONE, MAX_HOME_TERS, HOME_TER_HUMAN_URBAN, HOME_TER_HUMAN_MARINER, HOME_TER_ELF_URBAN, HOME_TER_ELF_SEA, HOME_TER_DWARF_URBAN, HOME_TER_DWARF_MOUNTAIN, HOME_TER_GNOME_URBAN, HOME_TER_GNOME_SWAMP, HOME_TER_OGRE_VILLAGER, HOME_TER_OGRE_HILL, HOME_TER_HOBBIT_URBAN, HOME_TER_HOBBIT_MARITIME, CON_CREATION_TRAITS, STAT_TERRITORY, STAT_NATURAL]
---

# Territory/Homeland System

## Overview

Territories are permanent sub-races selected during character creation that modify base stats. Each race offers 4-8 homeland options representing different upbringings. Players choose immediately after race selection, receiving stat modifiers ranging from -30 to +30 per stat. All territories are zero-sum (balanced), though Urban trades physical stats (CON, BRA) for mental ones (INT, WIS, CHA), making it suboptimal for most combat builds. Territories provide flavor without granting additional skills.

## Patterns

### Always

- Append new territories at end of enum before `MAX_HOME_TERS` (binary charfile compatibility)
- Validate territory-race compatibility before setting via admin commands
- Use `territory_adjustment()` to get stat modifiers
- Handle `HOME_TER_NONE` (value 0) as valid, returning 0 for all stats (used for NPCs)
- Query `STAT_TERRITORY` for territory modifier alone, `STAT_NATURAL` for combined stats

### Never

- Insert new territory values mid-enum (corrupts existing character files)
- Assume territory grants skills (stats only)
- Modify charfile struct layout (binary format frozen)
- Assume `STAT_CURRENT` includes territory modifiers (only `STAT_NATURAL` does)

## Reference

### Territory Types by Category

| Category | Races | Net Modifier |
|----------|-------|--------------|
| Urban | Human, Elf, Dwarf, Gnome, Hobbit | 0 |
| Villager/Tribal | Human, Elf, Dwarf, Gnome, Ogre, Hobbit | 0 |
| Plains/Grasslands | Human, Elf, Ogre, Hobbit | 0 |
| Recluse/Hermit | Human, Elf, Dwarf | 0 |
| Hill | Human, Dwarf, Gnome, Ogre, Hobbit | 0 |
| Mountain | Human, Dwarf, Elf (Snow) | 0 |
| Forest | Human, Elf (Wood), Hobbit | 0 |
| Mariner/Sea | Human, Elf (Sea), Gnome (Swamp), Hobbit | 0 |

### Urban Dweller Stat Modifiers

| Stat | Mod | Stat | Mod |
|------|-----|------|-----|
| CHA | +20 | KAR | -20 |
| INT | +20 | WIS | +20 |
| FOC | -10 | SPE | +20 |
| CON | -20 | BRA | -20 |
| PER | -10 | | |

### Recluse Stat Modifiers

| Stat | Mod | Stat | Mod |
|------|-----|------|-----|
| CHA | -30 | KAR | +30 |
| INT | -25 | WIS | -15 |
| CON | +25 | BRA | +15 |
| FOC | +15 | PER | -15 |

### Mountain Dweller Stat Modifiers

| Stat | Mod | Stat | Mod |
|------|-----|------|-----|
| CHA | -20 | KAR | +20 |
| INT | -20 | WIS | -15 |
| CON | +20 | BRA | +15 |
| FOC | -15 | PER | +10 |
| AGI | +5 | | |

### Forest Dweller Stat Modifiers

| Stat | Mod | Stat | Mod |
|------|-----|------|-----|
| CHA | -15 | KAR | +15 |
| INT | -15 | WIS | -15 |
| CON | +15 | BRA | +15 |
| FOC | -15 | PER | +10 |
| AGI | +5 | | |

### Hill Dweller Stat Modifiers

| Stat | Mod | Stat | Mod |
|------|-----|------|-----|
| CHA | -10 | KAR | +10 |
| INT | -15 | WIS | -5 |
| CON | +10 | BRA | +10 |
| FOC | -15 | PER | +10 |
| AGI | +5 | | |

### Plains/Grasslands Stat Modifiers

| Stat | Mod | Stat | Mod |
|------|-----|------|-----|
| CHA | -5 | KAR | +5 |
| INT | -10 | FOC | -10 |
| CON | +5 | BRA | +5 |
| PER | +15 | SPE | -5 |

### Villager/Tribal Stat Modifiers

| Stat | Mod | Stat | Mod |
|------|-----|------|-----|
| CHA | +10 | KAR | -10 |
| INT | +10 | WIS | +10 |
| CON | -10 | BRA | -10 |

### Mariner/Sea Stat Modifiers

| Stat | Mod | Stat | Mod |
|------|-----|------|-----|
| CHA | -5 | KAR | +5 |
| INT | -5 | WIS | -5 |
| CON | +5 | BRA | +5 |
| FOC | -5 | PER | +5 |

### Race-Specific Territory Enums

| Race | Territory Enums |
|------|-----------------|
| Human | `HOME_TER_HUMAN_URBAN` through `HOME_TER_HUMAN_MARINER` |
| Elf | `HOME_TER_ELF_URBAN`, `_TRIBE`, `_PLAINS`, `_SNOW`, `_RECLUSE`, `_WOOD`, `_SEA` |
| Dwarf | `HOME_TER_DWARF_URBAN`, `_VILLAGER`, `_RECLUSE`, `_HILL`, `_MOUNTAIN` |
| Gnome | `HOME_TER_GNOME_URBAN`, `_VILLAGER`, `_HILL`, `_SWAMP` |
| Ogre | `HOME_TER_OGRE_VILLAGER`, `_PLAINS`, `_HILL` |
| Hobbit | `HOME_TER_HOBBIT_URBAN`, `_SHIRE`, `_GRASSLANDS`, `_HILL`, `_WOODLAND`, `_MARITIME` |
| Extended (Goblin, Gnoll, Troglodyte, Orc) | Eight territories following human model with race-specific names |
| Advanced (Bullywug, Kalysian, Aarakocra, Troll) | Unique territory sets, some values marked UNUSED (return 0 for all stats) |

### Key Functions and Data

| Symbol | Purpose |
|--------|---------|
| `territoryT` | Enum defining all territories |
| `home_terrains[]` | Display name strings |
| `territory_adjustment()` | Returns stat modifier for territory/stat pair |
| `nannyRaces[]` | Maps races to available territories (array pointer + count) |
| `playerData.hometerrain` | Runtime storage in TBeing |
| `charFile.hometerrain` | Persistent storage (ubyte) |

### Admin Commands

| Command | Purpose |
|---------|---------|
| `@set territory <char> <value>` | Modify character territory |
| `@stat <char>` | View territory in character stats |

## Implementation

### Extended Race Mapping

Races after the base six (Goblin onward) map to human territory stats via modular arithmetic in `territory_adjustment()`:

```
ter = territoryT(1 + ((ter - HOME_TER_GOBLIN_URBAN) % 8))
```

Position 0 maps to Urban, position 1 to Villager, etc.

### Stat Integration

Territory modifiers apply during natural stat calculation via `getStat()`:

1. Racial base from `race->baseStats`
2. Player allocation from `chosenStats`
3. Age modifier from `age_mod_for_stat()`
4. **Territory from `territory_adjustment()`**

Query territory modifier alone with `getStat(STAT_TERRITORY, whichStat)`.

### Character Creation Flow

1. Player selects race
2. System enters `CON_CREATION_TRAITS` state
3. System presents territories from `nannyRaces[race].terrains`
4. Player sees menu with `home_terrains[]` display names (checkbox-style indicators, default is first option)
5. Selection stored in `playerData.hometerrain`
6. On save, written to `charFile.hometerrain` (ubyte)

### Storage Details

- **Runtime**: `TBeing::player.hometerrain` as `territoryT` enum
- **Persistent**: `charFile.hometerrain` as `ubyte` (0-93 range)
- **Help files**: `lib/help/territory help <race>`

### Territory Validation

When setting territory via `@set`, validate race compatibility:

- `HOME_TER_NONE` (0) is always valid
- Check territory falls within race's valid enum range
- Human: `HOME_TER_HUMAN_URBAN` through `HOME_TER_HUMAN_MARINER`
- Other races have equivalent bounded ranges

### Display

- `attribute` command: "You grew up as an urban dweller and began adventuring at age 17." (uses a/an article selection)
- Character launchpad: "Race/Homeland: Human/urban dweller"
- `@stat`: Shows territory in admin view

## Troubleshooting

| Symptom | Cause | Fix |
|---------|-------|-----|
| Garbled territory display | Enum value out of bounds | Check `home_terrains[]` array bounds |
| Wrong stats after race change | Territory not updated | Set valid territory for new race |
| New territory corrupts old chars | Inserted mid-enum | Add only at end before `MAX_HOME_TERS` |
| NPC shows territory stats | `HOME_TER_NONE` not set | Ensure NPCs use `HOME_TER_NONE` (0) |
| Extended race wrong stats | Modular mapping issue | Verify formula: `1 + ((ter - BASE) % 8)` |
| Admin set fails | Invalid territory for race | Check race-specific enum bounds |
| Stats missing territory bonus | Using wrong stat type | Query `STAT_NATURAL`, not `STAT_CURRENT` |
| Advanced race has unexpected modifiers | UNUSED territory not returning zero | Check `territory_adjustment()` switch cases |
| Missing territories in creation menu | `nannyRaces[]` misconfigured | Verify array pointer and count in race entry |
| Territory help not found | Missing help file | Create `lib/help/territory help <race>` |
