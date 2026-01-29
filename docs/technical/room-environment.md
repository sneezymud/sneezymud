---
title: Room and Environment System
description: Describes the room system, environment properties, and weather mechanics including TRoom class, room flags, exit system, sector types, weather system, and light levels.
keywords:
  - TRoom
  - real_roomp
  - sectorTypeT
  - roomFlags
  - ROOM_NO_MAGIC
  - ROOM_NO_PORTAL
  - ROOM_NO_ESCAPE
  - ROOM_NO_HEAL
  - ROOM_HOSPITAL
  - EXIT_CLOSED
  - EXIT_LOCKED
  - Weather class
  - skyT
  - sunT
  - wetness
  - outdoorLight
category: Critical Systems

  - combat-formulas.md
  - movement-system.md
last_updated: 2026-01-29
source_files:
  - code/code/misc/room.h
  - code/code/misc/room.cc
  - code/code/misc/weather.h
  - code/code/misc/weather.cc
  - code/code/misc/enum.h
  - code/code/disc/disc_cleric_wrath.cc
  - code/code/disc/disc_shaman_frog.cc
  - code/code/spec/spec_objs_lightning_rod.cc
related: [movement-terrain-navigation.md]
---

# Room and Environment System

This document describes the room system, environment properties, and weather mechanics.

## Room Database

Rooms are stored in a hash table indexed by virtual number (vnum):

```cpp
TRoom* room_db[WORLD_SIZE];  // WORLD_SIZE = 50000
```

Access rooms via `real_roomp(vnum)` which returns `nullptr` for non-existent rooms.

**Key files:** `code/code/misc/room.h`, `code/code/misc/room.cc`, `code/code/misc/weather.h`, `code/code/misc/enum.h`

## TRoom Class

The `TRoom` class inherits from `TThing` and represents a single location.

| Property | Type | Description |
|----------|------|-------------|
| `sectorType` | `sectorTypeT` | Terrain type affecting movement, resources, weather |
| `roomFlags` | `unsigned int` | 22-bit bitvector of room properties |
| `roomHeight` | `int` | Vertical dimension of the room |
| `riverDir/riverSpeed` | `dirTypeT/short` | River flow direction and speed |
| `hasWindow` | `short` | Count of windows in the room |
| `zone` | `zoneData*` | Zone this room belongs to |
| `teleTime/teleTarg` | `short/int` | Teleport delay and destination |
| `moblim` | `unsigned short` | Maximum mobs allowed in room |
| `x, y, z` | `int` | World coordinates |

### Exit System

Each room has up to 10 exits: `roomDirData* dir_option[MAX_DIR]`

Directions: NORTH, EAST, SOUTH, WEST, UP, DOWN, NORTHEAST, NORTHWEST, SOUTHEAST, SOUTHWEST

## Room Flags (22 bits)

| Flag | Bit | Effect |
|------|-----|--------|
| `ROOM_ALWAYS_LIT` | 0 | Base light level of 18 |
| `ROOM_DEATH` | 1 | Kills players entering |
| `ROOM_NO_MOB` | 2 | Mobs cannot enter |
| `ROOM_INDOORS` | 3 | Sheltered from weather |
| `ROOM_PEACEFUL` | 4 | No combat allowed |
| `ROOM_NO_STEAL` | 5 | Stealing disabled |
| `ROOM_NO_ESCAPE` | 6 | Cannot flee from combat |
| `ROOM_NO_MAGIC` | 7 | Magic use blocked |
| `ROOM_NO_PORTAL` | 8 | Portal spells blocked |
| `ROOM_PRIVATE` | 9 | Limited occupancy |
| `ROOM_SILENCE` | 10 | No speech or sounds |
| `ROOM_NO_ORDER` | 11 | Cannot order followers |
| `ROOM_NO_FLEE` | 12 | Flee command disabled |
| `ROOM_HAVE_TO_WALK` | 13 | Must walk (no mount/fly) |
| `ROOM_ARENA` | 14 | Arena combat rules |
| `ROOM_NO_HEAL` | 15 | Natural healing blocked |
| `ROOM_HOSPITAL` | 16 | Enhanced healing |
| `ROOM_SAVE_ROOM` | 17 | Items persist across reboots |
| `ROOM_NO_AUTOFORMAT` | 18 | Description not auto-formatted |
| `ROOM_BEING_EDITTED` | 19 | Currently being edited |
| `ROOM_ON_FIRE` | 20 | Room is burning |
| `ROOM_FLOODED` | 21 | Room is flooded |

## Dynamic Room States

**Fire:** `ROOM_FIRE_THRESHOLD = 20000` cubic inches. When set, `getSectorType()` returns `SECT_FIRE`.

**Flood:** `ROOM_FLOOD_THRESHOLD = 30000` cubic inches. When set, `getSectorType()` returns `SECT_TEMPERATE_RIVER_SURFACE`.

## Exit Flags (11 bits)

| Flag | Bit | Effect |
|------|-----|--------|
| `EXIT_CLOSED` | 0 | Door is shut |
| `EXIT_LOCKED` | 1 | Requires key to open |
| `EXIT_SECRET` | 2 | Hidden from observation |
| `EXIT_DESTROYED` | 3 | Door has been broken |
| `EXIT_NOENTER` | 4 | Cannot pass through |
| `EXIT_TRAPPED` | 5 | Trap is set on door |
| `EXIT_CAVED_IN` | 6 | Passage blocked by debris |
| `EXIT_WARDED` | 7 | Magical barrier |
| `EXIT_SLOPED_UP` | 8 | Upward slope |
| `EXIT_SLOPED_DOWN` | 9 | Downward slope |
| `EXIT_JAMMED` | 10 | Stuck closed |

## Sector Types (58 total)

Organized by climate zone (arctic, temperate, tropical) plus special types. Each sector has `TTerrainInfo` properties:

| Property | Description |
|----------|-------------|
| `movement` | Movement cost multiplier |
| `thickness` | Visibility reduction |
| `hunger` | Hunger drain rate |
| `thirst` | Thirst drain rate |
| `heat` | Temperature effect |
| `wetness` | Base wetness modifier (-40 desert to +100 underwater) |

### Sector Classification Methods

| Method | Includes |
|--------|----------|
| `isCitySector()` | Arctic/Temperate/Tropical City |
| `isRoadSector()` | Road and Forest Road types |
| `isForestSector()` | Forest, Jungle, Rainforest, Dead Woods |
| `isWaterSector()` | Rivers and Oceans |
| `isUnderwaterSector()` | Temperate/Tropical Underwater |
| `isAirSector()` | Atmosphere sectors |
| `isArcticSector()` | SECT_SUBARCTIC through SECT_PLAINS |
| `isTropicalSector()` | SECT_TROPICAL_CITY through SECT_ASTRAL_ETHREAL |

### Winter Sector Mapping

During snowy weather, `getArcticSectorType()` transforms temperate to arctic equivalents (e.g., PLAINS to SUBARCTIC, TEMPERATE_OCEAN to ICEFLOW).

## Weather System

The `Weather` class manages global atmospheric conditions via static members:

| Variable | Type | Description |
|----------|------|-------------|
| `pressure` | `int` | Atmospheric pressure (Mb) |
| `change` | `int` | Pressure change rate/direction |
| `sky` | `skyT` | SKY_CLOUDLESS, SKY_CLOUDY, SKY_RAINING, SKY_LIGHTNING |
| `sunlight` | `sunT` | SUN_DARK, SUN_DAWN, SUN_RISE, SUN_LIGHT, SUN_SET, SUN_TWILIGHT |
| `moontype` | `int` | Moon phase (0-31): new, waxing, full, waning |

`getWeather(const TRoom&)` returns: NONE (indoors/underwater), CLOUDLESS, CLOUDY, RAINY, LIGHTNING, or SNOWY. Winter months (1-2, 11-12) convert rain to snow in non-tropical sectors.

## Light Levels

Light calculated from: base (ROOM_ALWAYS_LIT = 18), outdoor light, window light, and objects/beings.

| Sun Phase | Outdoor Light |
|-----------|---------------|
| SUN_DARK | 0 |
| SUN_TWILIGHT | 1 |
| SUN_DAWN | 2 |
| SUN_RISE/SET | 10 |
| SUN_LIGHT | 25 |

Weather modifiers: cloudy/lightning -1, rainy -2, snowy -3.

Thresholds: `brightSunlight()` = light > 20, `pitchBlackDark()` = light <= 0

## Weather Gameplay Effects

Weather in SneezyMUD has significant gameplay impact beyond visual/atmospheric messages. The effects fall into several categories.

**Key files:** `code/code/misc/weather.cc`, `code/code/disc/disc_cleric_wrath.cc`, `code/code/disc/disc_shaman_frog.cc`, `code/code/spec/spec_objs_lightning_rod.cc`

### Light Level Reduction

Weather directly reduces outdoor light levels in `TRoom::outdoorLight()` (weather.cc:588-633):

| Weather | Light Modifier |
|---------|----------------|
| CLOUDY | -1 |
| LIGHTNING | -1 |
| RAINY | -2 |
| SNOWY | -3 |

Reduced light affects visibility, tracking ability, and combat effectiveness.

### Weather-Required Spells

Several spells require specific weather conditions to cast:

| Spell | Weather Required | File:Line |
|-------|------------------|-----------|
| **Call Lightning** | RAINY or LIGHTNING | disc_cleric_wrath.cc:835-844 |
| **Stormy Skies** | RAINY, LIGHTNING, or SNOWY | disc_shaman_frog.cc:282-289 |
| **Plague of Locusts** | NOT RAINY (fails in rain) | disc_cleric_wrath.cc:20-24 |
| **Conjure Water Elemental** | Can use RAINY as water source | disc_mage_water.cc:882-884 |

Stormy Skies deals different damage types based on weather: lightning bolts during rain/lightning storms, hail during snow (disc_shaman_frog.cc:297-365).

### Lightning Rod Object Proc

Objects with the `weaponLightningRod` spec proc interact with lightning weather. When a lightning rod weapon is dropped in an outdoor room during SKY_LIGHTNING weather, it can attract lightning strikes that damage all beings in the room (spec_objs_lightning_rod.cc:149-185).

Conditions for lightning strike:
- Weather must be `Weather::LIGHTNING`
- Room must NOT be indoors (`!ROOM_INDOORS`)
- 1% chance per pulse (`::number(0, 100)`)
- Damage: 1 to `weapDamLvl / 4`

### Wetness from Rain

Rain and storms increase character wetness in `getRoomWetness()` (weather.cc:884-950):

| Weather | Wetness Modifier |
|---------|-----------------|
| LIGHTNING | +20 |
| RAINY | +30 |
| CLOUDLESS + SUN_LIGHT | -10 (drying) |

Wetness accumulates over time and can affect certain gameplay mechanics. Characters dry faster in sunny conditions and near burning objects.

### Gunpowder Weapon Malfunctions

Handgonnes and cannons can be fouled by wet weather:

| Weapon | Weather Check | File:Line |
|--------|---------------|-----------|
| Handgonne | 100% foul if raining outdoors | obj_handgonne.cc:95-103 |
| Cannon | 25% foul if raining outdoors | obj_cannon.cc:97-105 |

When fouled, the weapon fails to fire and must be cleaned before use.

### Frostbite Disease (Quest Toggle)

When `TOG_QUESTCODE4` is enabled, prolonged exposure to bad weather can cause frostbite (periodic.cc:2316-2338):

Conditions:
- Must be outdoors continuously (no `AFFECT_WAS_INDOORS`)
- Weather must be SKY_RAINING, SKY_CLOUDY, or SKY_LIGHTNING
- Character must NOT be immune to cold
- Player characters only (not mobs)
- Non-immortals only

### Component Spawning

Weather conditions can control when certain components spawn or despawn. The rainbow bridge component spawns only during RAINY or LIGHTNING weather (obj_component.cc:88-98). Component placement uses a weather bitmask to control availability.

### Skills Documentation vs Implementation

The help file for the hide skill mentions weather effects ("snow leaves tracks, etc."), but the actual `hide()` implementation (disc_thief_stealth.cc:923-942) does not currently check weather conditions. This represents either planned but unimplemented functionality, or documentation that is out of sync with the code.

Similarly, bash skill comments mention weather/lighting as potential modifiers (cmd_bash.cc:244-246) but these are not implemented.

## Wetness System

Room wetness affects characters over time. Base values from `TTerrainInfo::wetness`:

| Value | Environment |
|-------|-------------|
| 100 | Underwater |
| 50 | River surface (doubled when sitting) |
| 20 | Swamp, standing water |
| 10 | Foggy, steamy jungle |
| 0 | Damp, overcast |
| -10 | Normal day |
| -20 | Cities/indoors |
| -40 | Desert |

Maximum character wetness: `WET_MAXIMUM = 100`. Characters dry slower than they get wet (change rate /5 drying vs /3 wetting).
