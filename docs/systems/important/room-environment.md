---
title: Room and Environment System
description: Room storage, environment properties, weather mechanics, and light levels
category: important
keywords: [room database, terrain types, weather effects, light calculation, exit state, sector classification]
primary_symbols:
  functions: [real_roomp, getSectorType, getWeather, outdoorLight, getRoomWetness, getArcticSectorType]
  classes: [TRoom, Weather, TTerrainInfo, roomDirData]
  enums: [sectorTypeT, SECT_FIRE, SECT_TEMPERATE_RIVER_SURFACE, SECT_SUBARCTIC, SECT_ARCTIC_FOREST, SECT_ICEFLOW, SECT_ARCTIC_CONDITIONS, ROOM_ALWAYS_LIT, ROOM_DEATH, ROOM_NO_MOB, ROOM_INDOORS, ROOM_PEACEFUL, ROOM_NO_STEAL, ROOM_NO_ESCAPE, ROOM_NO_MAGIC, ROOM_NO_PORTAL, ROOM_PRIVATE, ROOM_SILENCE, ROOM_NO_ORDER, ROOM_NO_FLEE, ROOM_HAVE_TO_WALK, ROOM_ARENA, ROOM_NO_HEAL, ROOM_HOSPITAL, ROOM_SAVE_ROOM, ROOM_NO_AUTOFORMAT, ROOM_BEING_EDITTED, ROOM_ON_FIRE, ROOM_FLOODED, EXIT_CLOSED, EXIT_LOCKED, EXIT_SECRET, EXIT_DESTROYED, EXIT_NOENTER, EXIT_TRAPPED, EXIT_CAVED_IN, EXIT_WARDED, EXIT_SLOPED_UP, EXIT_SLOPED_DOWN, EXIT_JAMMED, SKY_CLOUDLESS, SKY_CLOUDY, SKY_RAINING, SKY_LIGHTNING, SUN_DARK, SUN_TWILIGHT, SUN_DAWN, SUN_RISE, SUN_SET, SUN_LIGHT, MAX_DIR, Room::NOWHERE]
---

# Room and Environment System

## Overview

Rooms are the fundamental spatial unit containing all beings and objects. Each room has a terrain type determining movement costs and resource availability, flags controlling magic/combat/healing rules, optional exits connecting to other rooms, and dynamic states like fire and flood. The weather system tracks global atmospheric conditions affecting visibility, spell availability, and character wetness.

## Patterns

**Room Access:**
- Always use `real_roomp(vnum)` to access rooms; returns `nullptr` for non-existent vnums.
- Always validate room pointers before operations; check for `nullptr` and `Room::NOWHERE`.
- Never assume vnum exists; the room database is sparse.

**Sector Handling:**
- Always call `getSectorType()` not raw `sectorType`; dynamic states (fire/flood) override base type.
- Always use sector classification methods for type checks; covers all variants automatically.
- Use `getArcticSectorType()` for winter weather transformations.

**Weather Checks:**
- Always use `getWeather(room)` not raw `Weather::sky`; handles indoor/underwater correctly.
- Never assume outdoor rooms have weather; check return value for `NONE`.
- Always check weather requirements before weather-dependent spells.

**Light Calculations:**
- Always use `getLight()` for visibility checks; combines all light sources.
- Never hardcode light thresholds; use `brightSunlight()` and `pitchBlackDark()`.

**Wetness:**
- Characters dry slower than they get wet (rate /5 vs /3).
- Wetness is capped at `WET_MAXIMUM = 100`.

**Exit State Combinations:**
- Check flags in logical order: SECRET prevents visibility, DESTROYED bypasses CLOSED/LOCKED, CAVED_IN blocks passage regardless of CLOSED state, WARDED creates magical barrier independent of physical state.
- A LOCKED door can still be passed if DESTROYED. An OPEN door still blocks passage if CAVED_IN.

## Reference

### Room Flags (22-bit bitvector)

| Flag | Bit | Effect |
|------|-----|--------|
| ROOM_ALWAYS_LIT | 0 | Base light level 18 |
| ROOM_DEATH | 1 | Kills players entering |
| ROOM_NO_MOB | 2 | Mobs cannot enter |
| ROOM_INDOORS | 3 | Sheltered from weather |
| ROOM_PEACEFUL | 4 | No combat allowed |
| ROOM_NO_STEAL | 5 | Stealing disabled |
| ROOM_NO_ESCAPE | 6 | Cannot flee from combat |
| ROOM_NO_MAGIC | 7 | Magic use blocked |
| ROOM_NO_PORTAL | 8 | Portal spells blocked (independent of ROOM_NO_MAGIC) |
| ROOM_PRIVATE | 9 | Limited occupancy |
| ROOM_SILENCE | 10 | No speech or sounds |
| ROOM_NO_ORDER | 11 | Cannot order followers |
| ROOM_NO_FLEE | 12 | Flee command disabled |
| ROOM_HAVE_TO_WALK | 13 | Must walk (no mount/fly) |
| ROOM_ARENA | 14 | Arena combat rules |
| ROOM_NO_HEAL | 15 | Natural healing blocked |
| ROOM_HOSPITAL | 16 | Enhanced healing |
| ROOM_SAVE_ROOM | 17 | Items persist across reboots |
| ROOM_NO_AUTOFORMAT | 18 | Description not auto-formatted |
| ROOM_BEING_EDITTED | 19 | Currently being edited |
| ROOM_ON_FIRE | 20 | Room is burning |
| ROOM_FLOODED | 21 | Room is flooded |

### Exit Flags (11-bit bitvector)

| Flag | Bit | Effect |
|------|-----|--------|
| EXIT_CLOSED | 0 | Door is shut |
| EXIT_LOCKED | 1 | Requires key to open |
| EXIT_SECRET | 2 | Hidden from observation, requires search |
| EXIT_DESTROYED | 3 | Door has been broken, ignores CLOSED/LOCKED |
| EXIT_NOENTER | 4 | Cannot pass through |
| EXIT_TRAPPED | 5 | Trap is set on door |
| EXIT_CAVED_IN | 6 | Passage blocked by debris, prevents passage regardless of CLOSED |
| EXIT_WARDED | 7 | Magical barrier, independent of physical state |
| EXIT_SLOPED_UP | 8 | Upward slope |
| EXIT_SLOPED_DOWN | 9 | Downward slope |
| EXIT_JAMMED | 10 | Stuck closed (different failure from LOCKED) |

### Directions

NORTH, EAST, SOUTH, WEST, UP, DOWN, NORTHEAST, NORTHWEST, SOUTHEAST, SOUTHWEST (10 total, `MAX_DIR`)

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

### Weather States

| Sky | Description |
|-----|-------------|
| SKY_CLOUDLESS | Clear skies |
| SKY_CLOUDY | Overcast |
| SKY_RAINING | Precipitation |
| SKY_LIGHTNING | Thunderstorm |

| Sunlight | Light Value |
|----------|-------------|
| SUN_DARK | 0 |
| SUN_TWILIGHT | 1 |
| SUN_DAWN | 2 |
| SUN_RISE/SET | 10 |
| SUN_LIGHT | 25 |

**Weather Light Modifiers:** cloudy/lightning -1, rainy -2, snowy -3

### Base Terrain Wetness

| Value | Environment |
|-------|-------------|
| +100 | Underwater |
| +50 | River surface (doubled sitting) |
| +20 | Swamp, standing water |
| +10 | Foggy, steamy jungle |
| 0 | Damp, overcast |
| -10 | Normal day |
| -20 | Cities/indoors |
| -40 | Desert |

### Weather Wetness Modifiers

| Weather | Modifier |
|---------|----------|
| LIGHTNING | +20 |
| RAINY | +30 |
| CLOUDLESS + SUN_LIGHT | -10 (drying) |

### Weather-Dependent Spells

| Spell | Requirement | Notes |
|-------|-------------|-------|
| Call Lightning | RAINY or LIGHTNING | Lightning damage |
| Stormy Skies | RAINY, LIGHTNING, or SNOWY | Lightning damage during RAINY/LIGHTNING, cold damage with hail during SNOWY |
| Plague of Locusts | NOT RAINY (fails in rain) | |
| Conjure Water Elemental | RAINY as water source | |

### Gunpowder Foul Rates (Outdoor Rain)

| Weapon | Foul Chance |
|--------|-------------|
| Handgonne | 100% |
| Cannon | 25% |

### Seasonal Sector Mappings

| Temperate Sector | Arctic Equivalent |
|-----------------|-------------------|
| SECT_PLAINS | SECT_SUBARCTIC |
| SECT_TEMPERATE_FOREST | SECT_ARCTIC_FOREST |
| SECT_TEMPERATE_OCEAN | SECT_ICEFLOW |
| SECT_TEMPERATE_RIVER_SURFACE | SECT_ARCTIC_CONDITIONS |

Additional mappings exist for roads, hills, swamps, and beaches.

## Implementation

### Room Database

Rooms stored in hash table `room_db[WORLD_SIZE]` (WORLD_SIZE = 50000) indexed by virtual number. Sparse array; not all slots occupied.

### TRoom Class Properties

Inherits from `TThing`. Key members:
- `sectorType` (sectorTypeT): Base terrain type
- `roomFlags` (unsigned int): 22-bit property bitvector
- `roomHeight` (int): Vertical dimension
- `riverDir/riverSpeed`: River flow direction and speed
- `hasWindow` (short): Window count
- `zone` (zoneData*): Parent zone
- `teleTime/teleTarg`: Teleport delay and destination
- `moblim` (unsigned short): Maximum mobs
- `x, y, z` (int): World coordinates
- `dir_option[MAX_DIR]` (roomDirData*): Exit array

### Exit Data Structure (roomDirData)

- `description`: Text shown when examining direction
- `keyword`: Command matching for doors
- `condition`: 11-bit flag bitvector
- `door_type`: Door material/type classification
- `lock_difficulty`: Difficulty rating for lock picking
- `weight`: Door weight (affects bashing)
- `key`: Vnum of object required to unlock
- `trap_info`: Trap type data (if EXIT_TRAPPED)
- `trap_dam`: Trap damage value (if EXIT_TRAPPED)
- `to_room`: Destination room vnum (`Room::NOWHERE` if blocked)

Null `dir_option` pointer means no exit in that direction. Non-null pointer with `to_room` set to `Room::NOWHERE` represents blocked or future passage.

### Dynamic State Thresholds

- `ROOM_FIRE_THRESHOLD = 20000` cubic inches: When ROOM_ON_FIRE set, `getSectorType()` returns SECT_FIRE
- `ROOM_FLOOD_THRESHOLD = 30000` cubic inches: When ROOM_FLOODED set, `getSectorType()` returns SECT_TEMPERATE_RIVER_SURFACE

Room volume calculated from `roomHeight` multiplied by horizontal dimensions.

### Sector System

61 sector types organized by climate zone (arctic, temperate, tropical) plus special types. Each has `TTerrainInfo` with: movement cost multiplier, visibility thickness, hunger/thirst drain rates, heat effect, wetness modifier.

Winter mapping: During snowy weather (months 1-2, 11-12 in non-tropical sectors), `getArcticSectorType()` transforms temperate to arctic equivalents. Tropical sectors never receive SNOWY regardless of season.

### Weather System

`Weather` class manages global atmosphere via static members:
- `pressure` (int): Atmospheric pressure (Mb)
- `change` (int): Pressure change rate
- `sky` (skyT): Cloud/precipitation state
- `sunlight` (sunT): Time of day lighting
- `moontype` (int): Moon phase 0-31 (new, waxing, full, waning)

`getWeather(const TRoom&)` returns NONE for indoors/underwater, otherwise CLOUDLESS, CLOUDY, RAINY, LIGHTNING, or SNOWY.

### Light Calculation

Total light = base (ROOM_ALWAYS_LIT = 18) + outdoor light + window light + object/being contributions. Thresholds: `brightSunlight()` = light > 20, `pitchBlackDark()` = light <= 0.

Windows calculate percentage of outdoor light based on window count, not full outdoor contribution.

### Lightning Rod Spec Proc

Objects with `weaponLightningRod` spec proc attract lightning when dropped outdoors during SKY_LIGHTNING weather. Strike conditions: 1% chance per pulse, room not indoors, object must be in room inventory (not carried or equipped). Damage: 1 to weapDamLvl/4, affects all beings in room.

### Frostbite Disease

When TOG_QUESTCODE4 enabled, prolonged outdoor exposure during SKY_RAINING, SKY_CLOUDY, or SKY_LIGHTNING can cause frostbite. Requires: continuous outdoor exposure (no AFFECT_WAS_INDOORS), no cold immunity, player character, non-immortal.

### Component Weather Gates

Component spawning uses weather bitmask. Example: rainbow bridge component spawns only during RAINY or LIGHTNING weather.

## Troubleshooting

| Symptom | Cause | Fix |
|---------|-------|-----|
| Spell "requires outdoor storm" indoors | `getWeather()` returns NONE for ROOM_INDOORS | Check room flags; spell design intentional |
| Unexpected SECT_FIRE terrain | ROOM_ON_FIRE flag set | Clear flag or reduce fire source; `getSectorType()` overrides base |
| Character wetness not increasing | Room is indoors or desert sector | Check base terrain wetness; indoors blocks weather wetness |
| Light level wrong despite torches | Weather modifier applied | Verify outdoors; weather reduces light up to -3 |
| Handgonne won't fire | Fouled by rain | Clean weapon; store indoors during storms |
| Winter sector transformation missing | Non-winter month or tropical sector | Check game month; tropical sectors exempt |
| River flow not working | `riverSpeed` is 0 | Set both `riverDir` and `riverSpeed` in zone file |
| Room lookup returns nullptr | Vnum not loaded or invalid | Verify zone loaded; check vnum in zone files |
| Help says hide affected by weather | Documentation out of sync | Code does not implement weather effects for hide |
| Magic works in no-magic room | Flag check missing or flag removed | Verify `isRoomFlag` check before spell processing; some spells bypass by design |
| Portal created in no-portal room | ROOM_NO_PORTAL check missing | Add flag check before portal creation; distinct from ROOM_NO_MAGIC |
| Exit shows open but cannot pass | EXIT_CAVED_IN or EXIT_NOENTER flag set | Check all exit flags, not just CLOSED/LOCKED; verify `to_room` resolves |
| Light level changes indoors | ROOM_INDOORS flag not set | Set flag to prevent outdoor light contribution |
| Weather spell fails despite visible rain | Caster in indoor room | Verify caster room lacks ROOM_INDOORS; some rooms appear outdoor but have flag |
| Lightning rod not triggering | Weapon carried or weather wrong | Drop weapon in outdoor room during SKY_LIGHTNING; 1% chance per pulse |
| Room fire/flood override not working | Room volume below threshold | Verify volume exceeds 20000/30000; use `getSectorType()` not raw field |
