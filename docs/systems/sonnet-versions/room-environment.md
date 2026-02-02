---
title: Room and Environment System
category: critical
keywords: [TRoom, real_roomp, sectorTypeT, roomFlags, Weather, skyT, sunT, wetness, outdoorLight, EXIT_CLOSED]
related: [movement-terrain-navigation.md, combat-formulas.md]
primary_symbols:
  functions: [real_roomp, getSectorType, getWeather, outdoorLight, getRoomWetness, getArcticSectorType]
  classes: [TRoom, Weather, TTerrainInfo, roomDirData]
  files: [code/code/misc/room.h, code/code/misc/room.cc, code/code/misc/weather.h, code/code/misc/weather.cc]
---

## Overview

The room system models physical locations in the game world. Each room is a discrete spatial container with terrain properties, environmental conditions, exits to neighboring rooms, and dynamic state like fire or flooding. Rooms are indexed by virtual number in a global hash table and accessed through real_roomp which validates existence before returning pointers.

Environmental systems layer atop rooms to model weather, light, and wetness. Weather operates globally with per-room visibility determined by indoor status and climate zone. Light combines base values, outdoor sunlight, windows, and carried objects. Wetness accumulates from weather and terrain, affecting gameplay mechanics like gunpowder weapon reliability.

Room flags control fundamental behaviors like magic suppression, combat restrictions, and healing modifiers. Exit flags model physical barriers like locked doors, magical wards, and terrain slopes. Sector types define terrain properties affecting movement cost, visibility, resource drain, temperature, and wetness baseline.

Together these systems create environmental context that influences combat effectiveness, spell availability, movement difficulty, and character survival.

## Patterns

### Room Access Pattern

Always access rooms through real_roomp which returns nullptr for invalid vnums. Never index room_db directly without validation. The WORLD_SIZE constant of 50000 defines the hash table capacity but not all indices contain valid rooms.

Check for nullptr before dereferencing. Teleportation, portal creation, and zone resets all require validated room pointers to prevent crashes from stale or invalid room references.

### Flag-Based Restriction Pattern

Room and exit flags use bitvector checks with isRoomFlag and isExitFlag methods. Multiple flags often combine to create complex restrictions. ROOM_NO_MAGIC blocks all spellcasting. ROOM_NO_PORTAL specifically blocks portal creation even when magic is otherwise allowed. ROOM_NO_ESCAPE prevents flee attempts while ROOM_NO_FLEE disables the flee command.

Check flags before attempting restricted actions. Return early when flags block behavior rather than proceeding with invalid operations.

### Dynamic Sector Override Pattern

Room flags can override base sector type. When ROOM_ON_FIRE is set, getSectorType returns SECT_FIRE regardless of actual sector. When ROOM_FLOODED is set, it returns SECT_TEMPERATE_RIVER_SURFACE. Fire requires room volume exceeds ROOM_FIRE_THRESHOLD of 20000 cubic inches. Flooding requires exceeding ROOM_FLOOD_THRESHOLD of 30000 cubic inches.

Query getSectorType instead of directly accessing sectorType field to respect dynamic overrides. Volume calculations combine roomHeight with room dimensions.

### Weather Condition Gating Pattern

Several spells require specific weather states. Call Lightning requires RAINY or LIGHTNING weather. Stormy Skies requires RAINY, LIGHTNING, or SNOWY. Plague of Locusts explicitly fails during RAINY weather. Check weather before spell execution and provide appropriate failure messages.

Use getWeather which accounts for indoor status, underwater environments, and climate zone transformations rather than checking global Weather::sky directly.

### Seasonal Sector Transformation Pattern

Winter months transform temperate sectors to arctic equivalents. getArcticSectorType maps PLAINS to SUBARCTIC, TEMPERATE_OCEAN to ICEFLOW, and other temperate sectors to snowy variants. This occurs when weather is SNOWY and current month is in the winter range.

Query transformed sector type when calculating movement costs, resource availability, and environmental effects during winter weather. Original sector type remains unchanged in room data.

### Light Accumulation Pattern

Room light combines multiple sources. Base light starts at 18 for ROOM_ALWAYS_LIT rooms, 0 otherwise. Outdoor light adds sunlight value modified by weather. Windows add percentage of outdoor light. Objects and beings with light sources contribute individual values.

Sum all light sources to determine total room light. Compare against thresholds: brightSunlight at 20, pitchBlackDark at 0. Light affects visibility range, tracking success, and combat accuracy.

### Wetness Accumulation Over Time Pattern

Character wetness changes gradually based on room wetness value from getRoomWetness. Underwater environments provide +100. River surfaces give +50, doubled when sitting. Deserts provide -40 for rapid drying. Weather adds modifiers: LIGHTNING +20, RAINY +30, sunny conditions -10.

Wetness increases faster than it decreases with change rate divided by 3 when wetting versus divided by 5 when drying. Maximum wetness caps at WET_MAXIMUM of 100. Wetness affects gunpowder weapon fouling and potentially other mechanics.

### Exit State Combination Pattern

Exit flags combine to create complex door states. A door can be simultaneously CLOSED, LOCKED, and TRAPPED. Check flags in logical order: SECRET prevents visibility, DESTROYED bypasses CLOSED/LOCKED, CAVED_IN blocks passage regardless of CLOSED state, WARDED creates magical barrier independent of physical state.

Test multiple flags when determining passage possibility. A LOCKED door can still be passed if DESTROYED. An OPEN door still blocks passage if CAVED_IN.

## Reference

### TRoom Properties

The TRoom class inherits from TThing. Core spatial properties:
- sectorType: terrain classification affecting movement and resources
- roomFlags: 22-bit bitvector controlling room behavior
- roomHeight: vertical dimension for volume calculations
- x, y, z: world coordinates for spatial positioning
- zone: pointer to containing zone data structure

Exit system:
- dir_option: array of MAX_DIR roomDirData pointers for exits
- Directions: NORTH, EAST, SOUTH, WEST, UP, DOWN, NORTHEAST, NORTHWEST, SOUTHEAST, SOUTHWEST

Flow and transport:
- riverDir: direction of water current
- riverSpeed: strength of current pushing beings
- teleTime: countdown ticks before forced teleportation
- teleTarg: destination room vnum for teleport

Constraints:
- moblim: maximum mobile count allowed in room

### Room Flag Catalog

Lighting and visibility:
- ROOM_ALWAYS_LIT: base light level 18, ignores darkness

Safety and restrictions:
- ROOM_DEATH: instant death on entry
- ROOM_NO_MOB: prevents mobile entry
- ROOM_PEACEFUL: combat initiation blocked
- ROOM_ARENA: special arena combat rules
- ROOM_PRIVATE: limited occupancy enforcement

Combat modifiers:
- ROOM_NO_ESCAPE: flee attempts fail
- ROOM_NO_FLEE: flee command disabled
- ROOM_NO_STEAL: theft actions blocked

Magic restrictions:
- ROOM_NO_MAGIC: all spellcasting blocked
- ROOM_NO_PORTAL: portal creation specifically blocked
- ROOM_SILENCE: communication and sound disabled
- ROOM_NO_ORDER: follower commands disabled

Healing modifiers:
- ROOM_NO_HEAL: natural regeneration disabled
- ROOM_HOSPITAL: enhanced healing rate

Environmental properties:
- ROOM_INDOORS: sheltered from weather effects
- ROOM_HAVE_TO_WALK: mounted and flying movement disabled

Dynamic states:
- ROOM_ON_FIRE: overrides sector to SECT_FIRE when volume exceeds threshold
- ROOM_FLOODED: overrides sector to water when volume exceeds threshold

Administrative:
- ROOM_SAVE_ROOM: items persist across reboots
- ROOM_NO_AUTOFORMAT: preserve raw description formatting
- ROOM_BEING_EDITTED: builder edit lock

### Exit Flag Catalog

Physical barriers:
- EXIT_CLOSED: door shut, blocks passage
- EXIT_LOCKED: requires key, prevents opening
- EXIT_SECRET: hidden from observation, requires search
- EXIT_DESTROYED: broken barrier, ignores CLOSED/LOCKED
- EXIT_CAVED_IN: debris blockage, prevents passage
- EXIT_JAMMED: stuck closed, different failure from LOCKED
- EXIT_NOENTER: absolute passage prevention

Magical barriers:
- EXIT_WARDED: magical barrier independent of physical state

Terrain properties:
- EXIT_SLOPED_UP: upward grade affecting movement
- EXIT_SLOPED_DOWN: downward grade affecting movement

Interactive:
- EXIT_TRAPPED: trap set, triggers on passage or opening

### Sector Classification Methods

Classification queries:
- isCitySector: Arctic/Temperate/Tropical City sectors
- isRoadSector: Road and Forest Road variants
- isForestSector: Forest, Jungle, Rainforest, Dead Woods
- isWaterSector: River and Ocean sectors
- isUnderwaterSector: Temperate and Tropical Underwater
- isAirSector: Atmosphere and high altitude sectors
- isArcticSector: SECT_SUBARCTIC through SECT_PLAINS range
- isTropicalSector: SECT_TROPICAL_CITY through SECT_ASTRAL_ETHREAL range

These methods classify sector type for movement calculations, resource spawning, and climate-specific mechanics.

### TTerrainInfo Properties

Each sector type maps to TTerrainInfo defining:
- movement: cost multiplier for movement actions
- thickness: visibility reduction distance penalty
- hunger: food consumption rate modifier
- thirst: water consumption rate modifier
- heat: temperature effect on character comfort
- wetness: baseline wetness modifier ranging from -40 for desert to +100 for underwater

### Weather Global State

Weather class manages static members:
- pressure: atmospheric pressure in millibars
- change: pressure change rate and direction
- sky: enumeration SKY_CLOUDLESS, SKY_CLOUDY, SKY_RAINING, SKY_LIGHTNING
- sunlight: enumeration SUN_DARK, SUN_DAWN, SUN_RISE, SUN_LIGHT, SUN_SET, SUN_TWILIGHT
- moontype: moon phase 0-31 representing new, waxing, full, waning cycle

### Weather Result Enumeration

getWeather returns context-specific values:
- NONE: indoors or underwater environments
- CLOUDLESS: clear sky
- CLOUDY: overcast
- RAINY: precipitation in temperate/tropical
- LIGHTNING: thunderstorm conditions
- SNOWY: winter precipitation in non-tropical sectors

Winter months 1, 2, 11, 12 convert RAINY to SNOWY for non-tropical sectors.

### Outdoor Light Values by Sun Phase

SUN_DARK: 0 light contribution
SUN_TWILIGHT: 1 light contribution
SUN_DAWN: 2 light contribution
SUN_RISE: 10 light contribution
SUN_SET: 10 light contribution
SUN_LIGHT: 25 light contribution

Weather modifiers applied after base calculation:
- CLOUDY: -1
- LIGHTNING: -1
- RAINY: -2
- SNOWY: -3

Threshold functions:
- brightSunlight: light level exceeds 20
- pitchBlackDark: light level 0 or below

### Wetness Values by Environment

Underwater: +100
River surface: +50, doubled to +100 when sitting
Swamp: +20
Foggy jungle: +10
Overcast: 0
Normal day: -10
Cities and indoors: -20
Desert: -40

Weather modifiers:
- LIGHTNING: +20
- RAINY: +30
- CLOUDLESS with SUN_LIGHT: -10 for drying

Maximum character wetness: WET_MAXIMUM = 100
Wetting rate: wetness change divided by 3
Drying rate: wetness change divided by 5

## Implementation

### Room Storage Architecture

Rooms stored in global hash table room_db indexed by vnum with capacity WORLD_SIZE of 50000. Not all indices contain valid rooms. Sparse allocation means many indices are nullptr.

Access function real_roomp performs bounds checking and returns nullptr for invalid vnums. Direct room_db indexing bypasses validation and risks segfaults from out-of-bounds or null accesses.

TRoom inherits from TThing providing spatial hierarchy methods. Rooms contain beings and objects through TThing::stuff list. Bidirectional pointers maintain consistency between room and contained things.

### Exit Data Structure

Each exit stored as roomDirData structure containing:
- general_description: text shown when examining direction
- keyword: command matching for doors
- exit_info: 11-bit flag bitvector
- key: vnum of object required to unlock
- to_room: destination room vnum, Room::NOWHERE if blocked

Exits allocated dynamically. Null dir_option pointer means no exit in that direction. Non-null pointer with to_room set to Room::NOWHERE represents blocked or future passage.

### Dynamic Sector Override Logic

getSectorType checks flags before returning sector:
1. If ROOM_ON_FIRE and room volume exceeds ROOM_FIRE_THRESHOLD, return SECT_FIRE
2. If ROOM_FLOODED and room volume exceeds ROOM_FLOOD_THRESHOLD, return SECT_TEMPERATE_RIVER_SURFACE
3. Otherwise return base sectorType value

Room volume calculated from roomHeight multiplied by horizontal dimensions. Thresholds prevent small rooms from becoming fire/flood sectors.

### Weather Calculation Logic

getWeather receives TRoom reference and determines local weather:
1. Return NONE if room has ROOM_INDOORS flag
2. Return NONE if sector is underwater
3. Check global Weather::sky state
4. If SKY_RAINING and month in winter range and sector not tropical, return SNOWY
5. Otherwise map sky state: SKY_CLOUDLESS to CLOUDLESS, SKY_CLOUDY to CLOUDY, SKY_RAINING to RAINY, SKY_LIGHTNING to LIGHTNING

Winter months defined as 1, 2, 11, 12. Tropical sectors never receive SNOWY regardless of season.

### Outdoor Light Calculation

outdoorLight method in weather.cc combines:
1. Base sunlight value from Weather::sunlight mapped to numeric value
2. Weather modifier subtracted: cloudy/lightning -1, rainy -2, snowy -3
3. Result clamped to non-negative range

Separate from room light calculation. Room light adds base value, outdoor light contribution scaled by window count for indoor rooms, and light from objects/beings.

### Wetness Accumulation Logic

getRoomWetness in weather.cc determines room wetness contribution:
1. Start with base terrain wetness from TTerrainInfo
2. Add weather modifiers if outdoor: LIGHTNING +20, RAINY +30
3. Subtract drying modifier if CLOUDLESS and SUN_LIGHT: -10
4. Double river wetness if character sitting
5. Return final wetness value

Character wetness updated periodically comparing current wetness to room wetness. If room wetter, add difference divided by 3. If room drier, subtract difference divided by 5. Cap at WET_MAXIMUM.

### Seasonal Sector Mapping

getArcticSectorType transforms sectors during winter:
- SECT_PLAINS to SECT_SUBARCTIC
- SECT_TEMPERATE_FOREST to SECT_ARCTIC_FOREST
- SECT_TEMPERATE_OCEAN to SECT_ICEFLOW
- SECT_TEMPERATE_RIVER_SURFACE to SECT_ARCTIC_CONDITIONS

Additional mappings for roads, hills, swamps, beaches transforming temperate variants to arctic equivalents. Returns original sector if no winter mapping exists.

Called when weather is SNOWY to calculate movement costs and environmental effects. Original room sector remains unchanged.

### Spell Weather Requirement Checks

Call Lightning in disc_cleric_wrath.cc checks weather before casting:
1. Get weather with getWeather for caster room
2. If not RAINY and not LIGHTNING, send failure message
3. Block spell execution, return failure

Stormy Skies in disc_shaman_frog.cc checks for RAINY, LIGHTNING, or SNOWY. Plague of Locusts explicitly fails if weather is RAINY.

Damage type in Stormy Skies varies by weather: lightning damage during RAINY/LIGHTNING, cold damage with hail during SNOWY.

### Lightning Rod Proc Behavior

weaponLightningRod spec proc in spec_objs_lightning_rod.cc triggers on CMD_GENERIC_PULSE when object in room:
1. Check weather is Weather::LIGHTNING
2. Check room not ROOM_INDOORS
3. Roll 1 percent chance
4. If triggered, damage all beings in room
5. Damage range 1 to weapDamLvl divided by 4
6. Send atmospheric messages about lightning strike

Object must be in room inventory, not carried or equipped. Provides environmental hazard incentive to pick up lightning rod weapons during storms.

### Gunpowder Fouling Logic

Handgonne in obj_handgonne.cc checks weather when firing:
1. Get weather for room
2. If RAINY or LIGHTNING, 100% foul chance
3. Set fouled state, prevent firing
4. Require cleaning before next use

Cannon in obj_cannon.cc uses 25% foul chance under same weather conditions. Fouling represents moisture degrading gunpowder reliability.

## Troubleshooting

### Null Room Pointer Crashes

Symptom: segfault accessing room properties after teleport, portal, or zone reset.

Cause: room_db indexed directly without validation, or real_roomp result not checked for nullptr.

Resolution: always use real_roomp for vnum lookup and check return value before dereferencing. Never assume vnum validity based on previous state as zone resets can remove rooms.

### Magic Works in No-Magic Room

Symptom: spells cast successfully despite ROOM_NO_MAGIC flag.

Cause: flag check missing from spell execution path, or flag removed during combat/editing.

Resolution: verify isRoomFlag check occurs before spell processing. Confirm flag persists by examining room with stat command. Some spells like teleport bypass ROOM_NO_MAGIC by design.

### Portal Created in No-Portal Room

Symptom: portal spell succeeds in ROOM_NO_PORTAL room.

Cause: ROOM_NO_PORTAL check missing or occurs after portal object created.

Resolution: add flag check before portal creation. ROOM_NO_PORTAL is distinct from ROOM_NO_MAGIC and requires separate validation.

### Exit Shows Open But Cannot Pass

Symptom: door appears open in description but passage blocked.

Cause: EXIT_CAVED_IN or EXIT_NOENTER flag set, or destination room deleted.

Resolution: check all exit flags, not just CLOSED/LOCKED. Verify to_room vnum resolves to valid room with real_roomp.

### Light Level Incorrect Indoors

Symptom: indoor room brightness changes with time of day.

Cause: ROOM_INDOORS flag not set, or window calculation includes outdoor light without proper filtering.

Resolution: set ROOM_INDOORS flag to prevent outdoor light contribution. Windows should calculate percentage of outdoor light based on window count, not full outdoor contribution.

### Weather Spells Fail Despite Correct Weather

Symptom: Call Lightning fails with "no storm" message during visible rain.

Cause: caster in indoor room where getWeather returns NONE, or weather changed between casting start and execution.

Resolution: verify caster room lacks ROOM_INDOORS flag. Check global Weather::sky state with stat command. Some rooms appear outdoor but have indoor flag for gameplay reasons.

### Character Wetness Not Changing

Symptom: character remains dry in rain or wet indoors.

Cause: wetness accumulation disabled, room wetness calculation incorrect, or character in indoor room.

Resolution: verify room lacks ROOM_INDOORS flag for weather effects. Check getRoomWetness returns expected value. Wetness changes gradually over multiple ticks, not instantly.

### Sector Type Wrong During Winter

Symptom: movement cost unchanged during snow despite sector transformation.

Cause: getArcticSectorType not called for movement calculation, or sector lacks winter mapping.

Resolution: use getArcticSectorType when weather is SNOWY instead of base getSectorType. Some sectors like cities have no arctic equivalent and maintain original values.

### Room Fire/Flood Override Not Working

Symptom: ROOM_ON_FIRE set but sector shows original type.

Cause: room volume below threshold, or getSectorType override logic bypassed.

Resolution: verify room volume exceeds ROOM_FIRE_THRESHOLD of 20000 or ROOM_FLOOD_THRESHOLD of 30000. Calculate volume as roomHeight times horizontal dimensions. Call getSectorType instead of accessing sectorType directly.

### Lightning Rod Not Triggering

Symptom: lightning rod weapon in room during storm without strikes.

Cause: weapon carried or equipped instead of in room inventory, room indoors, or weather not LIGHTNING.

Resolution: drop weapon in outdoor room during SKY_LIGHTNING weather. Verify room lacks ROOM_INDOORS flag. Trigger chance is 1% per pulse, requiring patience for activation.
