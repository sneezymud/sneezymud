---
title: Movement and Terrain Navigation
category: critical
keywords: [movement, terrain, flying, climbing, falling, swimming, drowning, portals, teleport, DELETE_THIS, reconcileDamage, crashLanding]
related: [combat-rounds.md, room-environment.md, position-stance.md]
primary_symbols:
  functions: [doMove, rawMove, checkFalling, crashLanding, checkDrowning, riverFlow, genericTeleport, validMove, canClimb, fallKill]
  classes: [TBeing, TPortal, TRoom]
  files: [code/code/misc/movement.cc, code/code/misc/physics.cc, code/code/misc/being.cc, code/code/obj/obj_portal.cc]
---

## Overview

The movement system orchestrates all character locomotion in SneezyMUD. Characters move through rooms via directional commands, each room having a sector type that determines movement point costs. The system integrates terrain-based travel, vertical climbing with fall damage, water environments with drowning mechanics, flying modes, and instant travel via portals or teleportation.

Movement operations are inherently dangerous because they can trigger room effects, traps, combat encounters, or environmental hazards that delete the character. Every movement function can return DELETE_THIS, and callers must check these flags immediately. Continuing execution after a DELETE flag causes use-after-free crashes.

The system consists of basic directional movement with terrain costs, flying and levitation modes with reduced costs, vertical movement requiring climbing skill checks and applying fall damage, water mechanics including swimming and drowning with periodic damage, and instant travel methods including portal objects and teleportation spells. All subsystems share the DELETE flag safety requirement.

Movement integrates with mount/rider relationships, group following behavior, door and exit manipulation, room capacity limits, position requirements, and combat restrictions. The core validation flow checks combat state, door conditions, movement point availability, terrain compatibility, and room capacity before executing movement.

## Patterns

### Basic Movement Flow

Movement begins with command parsing that maps direction strings to enum values. The doMove function serves as the entry point, validating that the character controls their movement (not a mounted non-master) and is not fighting. For solo characters, moveOne executes the movement directly. For characters with followers or a master, moveGroup handles the entire group, attempting to move each member and handling failures gracefully.

The validMove function performs precondition checks including SPELL_BIND entrapment, exit existence via exit_ok, door state checks for caved-in, closed, or warded conditions, and room capacity limits. Failing any check returns false without modifying state.

Movement point calculation averages source and destination terrain costs, applies modifiers for sneaking, dwarf racial penalties in water, AFF_SWIM bonuses, injury penalties for crawling or wounded limbs, drunkenness, and flying or levitation bonuses. Mounted characters split costs with the mount paying full and the rider paying zero to one-third.

After validation and cost calculation, rawMove transfers the character between rooms, updating spatial pointers, removing from the source room, adding to the destination room, sending look output, and triggering genericMovedIntoRoom for entry effects like traps or death rooms.

### Flying and Levitation

Characters can fly through spell effects, racial abilities, flying mounts, or special sectors. The canFly check returns true for SPELL_FLY, AFF_FLYING, or winged races. The isFlying check returns true only when position equals POSITION_FLYING, representing active flight.

The doFly command validates flight capability, ensures the character is not riding, blocks underwater sectors, checks winged races for AFF_FLIGHTWORTHY (preening requirement), then sets POSITION_FLYING. The doLand command reverses this, preventing landing in flying sectors and requiring descent from air or vertical sectors before landing.

Flying sectors with SECT_MAKE_FLY automatically set POSITION_FLYING upon entry. Exiting flying sectors requires actual flight ability, otherwise characters are forced to land or denied passage to air/vertical sectors. Flying reduces movement costs to max(1, cost/4), while levitation reduces to max(5, cost/4).

### Climbing and Falling

Vertical movement through air or climbing sectors requires climbing skill checks via canClimb. The function calculates modifiers from agility, carried weight ratio, combat state, and limb injuries. Heavy loads, broken limbs, and fighting significantly reduce success chances.

Failed climbing or entry to air sectors without flight triggers checkFalling. The function recursively descends through rooms, tracking fall distance. Flying characters or those on flying mounts stop falling immediately. Immortals bounce without damage. Characters with SKILL_CATFALL or SPELL_FEATHERY_DESCENT have extended safe fall distances (10 rooms vs 5).

Flying sectors automatically catch falling characters by setting POSITION_FLYING. Upon landing, fallKill handles instant death for extreme falls (20+ rooms), then applies distance-based damage from reconcileDamage. Skills and water sectors halve damage, stacking for 75% reduction. The crashLanding function forces position changes, potentially causing the rider to fall off mounts, which recursively calls crashLanding and can return DELETE_THIS.

### Water and Drowning

Water sectors include surface and underwater variants. Underwater sectors require AFF_WATERBREATH or racial water breathing to avoid drowning. The procCharDrowning scheduler process runs every 3.6 seconds, calling checkDrowning for characters in underwater sectors without protection.

The checkDrowning function applies 20-40 damage via reconcileDamage, checking for the -1 death sentinel. This is not a DELETE flag and cannot be detected with IS_SET_DELETE. The function must check explicitly for -1 and return DELETE_THIS accordingly.

River sectors apply current via procCharRiverFlow. The riverFlow function checks riverSpeed as a percentage chance, doubled for sitting characters. Swimming skill checks allow resisting current. Failure moves the character in the riverDir direction, potentially triggering traps or death rooms.

Swimming success depends on character density (weight/volume ratio). Characters denser than water must actively swim. AFF_SWIM affects reduce movement costs in water sectors. Dwarf racial penalties add significant movement costs in water due to their density.

### Portals and Teleportation

Portal objects provide instant travel to fixed destinations. The TPortal class tracks charges, portal type for messages, trap configuration, portal state flags (closed, locked, trapped), and key requirements. The enterMe function validates portal state, checks combat mode restrictions, validates destination room, checks capacity limits, handles traps potentially returning DELETE_THIS or DELETE_VICT, transfers the character and followers in two passes (mounts first), and consumes charges on both source and destination portals.

The portal spell creates bidirectional portals to designated portal rooms. The teleport spell moves characters to random locations using genericTeleport, which loops until finding a valid destination excluding ROOM_PRIVATE, ROOM_HAVE_TO_WALK, ROOM_DEATH, flying sectors, and disabled zones. After transferring, genericMovedIntoRoom triggers entry effects, potentially returning DELETE_THIS.

Word of Recall transports to hometown based on deity faction standing, restricted by ROOM_ARENA, ROOM_NO_ESCAPE, and AFFECT_PLAYERKILL. Astral Walk moves the caster to a target creature's location with extensive restrictions. Summon pulls a target to the caster with immunity checks and room restrictions.

### DELETE Flag Safety

Movement functions return DELETE_THIS when the character dies. Callers must check IS_SET_DELETE immediately and propagate the flag up the call stack without further execution. Continuing after DELETE_THIS causes use-after-free as the character object is deleted.

The reconcileDamage function returns -1 on death, not a DELETE flag. Code must check == -1 explicitly and return DELETE_THIS. Using IS_SET_DELETE on -1 fails silently, causing continued execution after death.

Mount falling creates DELETE_THIS chains. The crashLanding function calls fallOffMount, which recursively calls crashLanding. Each level must check and propagate DELETE_THIS immediately.

Iterator safety requires caching next pointers before modifications. River flow and group movement modify spatial relationships, invalidating next pointers. The pattern caches next before operations, preventing use-after-free from invalidated iterators.

Scheduler adapters convert DELETE_THIS flags to bool return values. The adapter checks IS_SET_DELETE on the game function's return, returning true to signal deletion or false to keep the object.

## Reference

### Movement Commands

Direction commands map to dirTypeT enums: DIR_NORTH (0), DIR_EAST (1), DIR_SOUTH (2), DIR_WEST (3), DIR_UP (4), DIR_DOWN (5), DIR_NORTHEAST (6), DIR_NORTHWEST (7), DIR_SOUTHEAST (8), DIR_SOUTHWEST (9). Each direction has full and abbreviated forms.

Door commands include open/close for DOOR_DOOR, DOOR_TRAPDOOR, DOOR_GATE, raise/lower for DOOR_PORTCULLIS (inverted: raise opens, lower closes), and lower/raise for DOOR_DRAWBRIDGE (also inverted: lower opens, raise closes). Lock and unlock require proper keys checked via has_key across inventory, keyrings, held items, and worn equipment.

Flying commands are fly (take off) and land (touch down). Fly validates canFly, checks not riding, blocks underwater, requires AFF_FLIGHTWORTHY for winged races, then sets POSITION_FLYING. Land validates isFlying, prevents landing in flying sectors, requires descent from air/vertical sectors, then sets POSITION_STANDING.

### Terrain Costs

Base movement costs by sector: city and road sectors cost 1, plains cost 2, grasslands cost 3, hills cost 3, forest cost 4, swamp cost 5, mountains cost 6, underwater cost 8, climbing cost 9, solid rock cost 13, atmosphere cost 0. Actual cost is the average of source and destination costs.

Modifiers stack additively: sneaking adds 2, dwarf in water adds 20, AFF_SWIM in water halves cost, AFF_SWIM underwater quarters cost, crawling with both legs hurt adds 20, crawling with one arm hurt adds 20, foot wound adds 5, one leg hurt adds 10, drunk over 9 adds 1, crawling position adds 8 horizontal or 16 vertical, flying divides by 4 (minimum 1), levitating divides by 4 (minimum 5), haste or accelerate spells halve cost, and SKILL_HIKING reduces cost in forest/mountain/swamp by skill percentage.

### Exit and Door Flags

Exit condition flags include EXIT_CLOSED (0, door is shut), EXIT_LOCKED (1, requires key), EXIT_SECRET (2, hidden), EXIT_DESTROYED (3, broken), EXIT_NOENTER (4, cannot pass), EXIT_TRAPPED (5, trap set), EXIT_CAVED_IN (6, blocked by debris), EXIT_WARDED (7, requires ward key), EXIT_SLOPED_UP (8), EXIT_SLOPED_DOWN (9), and EXIT_JAMMED (10, stuck closed).

Door operations modify both sides: opening, closing, locking, and unlocking affect the current exit and the reverse exit in the destination room. Key checks search inventory, keyrings within inventory, held items, and all worn equipment slots. Warded exits require the specific ward key vnum worn or held, with immortals and ghosts passing through automatically.

### Fall Damage Thresholds

Safe fall distance without skills is 5 rooms. With SKILL_CATFALL or SPELL_FEATHERY_DESCENT, safe distance extends to 10 rooms. Falls beyond safe distance enter the damage zone, calculating 40-80 damage per room of distance. Skills halve damage, water sectors halve damage, and both stack for 75% reduction.

Instant death occurs at 20+ rooms regardless of skills or landing surface. Falls between 10-19 rooms with skills or 5-19 rooms without skills apply calculated damage through reconcileDamage, which returns -1 on death.

### Portal Configuration

Portal charges determine uses remaining: -1 means infinite charges, 0 means depleted, positive integers count down. Portal type (0-13) controls entry and exit messages. Trap type uses doorTrapT enum, with trap damage as unsigned short. Portal state uses EXIT_* flags for closed, locked, or trapped states. Portal key specifies vnum for locked portals.

Portal entry flow validates closed/noenter state, checks combat mode, validates destination room existence, checks room capacity, handles trap potentially returning DELETE flags, transfers character and followers in two passes (mounts then others), consumes charges on both ends, and returns DELETE_THIS if source portal depletes.

### Teleport Restrictions

Room flags prevent certain teleportation: ROOM_NO_ESCAPE prevents teleporting or recalling from the room, ROOM_NO_PORTAL blocks portal spell casting, ROOM_NO_MAGIC prevents astral walk targets, ROOM_PRIVATE excludes random teleport and astral walk destinations, ROOM_HAVE_TO_WALK excludes teleport and summon destinations, ROOM_ARENA blocks recall and summon sources, and ROOM_DEATH excludes random teleport destinations.

Zone-level restrictions include disabled zones blocking all teleport destinations. Flying sectors are excluded from random teleport to prevent dropping characters to their death without flight capability.

## Implementation

### doMove and Movement Entry Point

The doMove function in movement.cc serves as the public command interface. It first checks riding state, ensuring only the horseMaster can direct mounted movement. Non-masters receive a refusal message and return false without moving.

Combat checks prevent movement during fighting. Characters must flee or disengage before moving. If fighting, the function sends a concentration message and returns false immediately.

The function branches based on followers and master pointers. Solo characters (no followers, no master) call moveOne directly for simple execution. Characters with followers or a master call moveGroup to handle the entire group, tracking which members succeeded and handling partial group separation.

Return value propagation checks IS_SET_DELETE on the return code from moveOne or moveGroup. If DELETE_THIS is set, the character died during movement (trap, death room, fall, drowning) and the flag must be returned to the caller without further execution.

### validMove and Precondition Validation

The validMove function in movement.cc checks all movement preconditions before executing movement. It returns false for any failure condition, preventing state modification.

The function first retrieves exitp via exitDir, then checks for SPELL_BIND entrapment by iterating affects. Entrapped characters receive a message and return false without moving.

Exit validation calls exit_ok with the exitp pointer and NULL passability argument. This checks exit existence, passability flags, and basic door state. If exit_ok returns false, notLegalMove sends appropriate messaging and validMove returns false.

Door state checks validate EXIT_CAVED_IN (blocked by debris), EXIT_CLOSED (door is shut), and EXIT_WARDED (requires ward key). Each condition sends specific messages and returns false. Warded exits call tryPassWardedExit which checks for immortal/ghost status or searches equipment for the matching ward key vnum.

Room capacity checks retrieve moblim from the destination room and count current occupants via MobCountInRoom. If at or over capacity, the function sends a message and returns false, preventing entry to full rooms.

### Movement Point Calculation and Application

Movement point cost calculation in movement.cc averages source and destination sector terrain costs from TerrainInfo. The base cost is (from_sector_cost + to_sector_cost) / 2, providing gradual transitions between terrain types.

Modifiers apply sequentially: sneaking adds 2 unconditionally, dwarf race in water sectors adds 20, AFF_SWIM in water sectors divides cost by 2, AFF_SWIM in underwater sectors divides by 4, both legs hurt adds 20 for crawling, one arm hurt while crawling adds 20, foot wounds add 5 with fall chance, one leg hurt adds 10 with fall chance, drunkenness over 9 adds 1 with fall chance, crawling position adds 8 horizontal or 16 vertical, flying divides by 4 with minimum 1, levitating divides by 4 with minimum 5, and haste or accelerate spells divide by 2.

SKILL_HIKING applies in forest, mountain, and swamp sectors. It reduces cost by the character's skill percentage, calculated as: need_movement -= (need_movement * skill) / 100. This allows skilled hikers to traverse difficult terrain efficiently.

Mounted movement splits costs. The mount pays the full movement cost via addToMove on the riding pointer. The rider pays a random fraction from 0 to one-third: number(0, need_movement) / 3. This represents the rider's reduced effort while mounted.

Movement point checks occur after cost calculation. If the character (or mount for riding) lacks sufficient movement points, canMove returns false, the function sends an exhaustion message, and movement fails without state changes.

### rawMove and Spatial Transfers

The rawMove function in movement.cc performs the actual spatial transfer after validation succeeds. It updates all spatial relationships including room membership, position tracking, and followers.

The function first removes the character from the source room via the -- operator, which clears roomp, unlinks from the room's stuff list, and updates bidirectional pointers. Then it adds the character to the destination room via thing_to_room, which sets roomp, links into stuff, and establishes spatial relationships.

Look output generation calls doLook with empty arguments and CMD_LOOK, showing the new room to the character. This provides immediate feedback on the new environment.

The genericMovedIntoRoom function triggers entry effects including traps via checkForEnterTrap, death room processing, position-based falling checks, and terrain-specific effects. It returns DELETE_THIS if any effect kills the character, which rawMove must check and propagate immediately.

Follower handling occurs in a separate pass. For each follower in the group, the code validates they can follow (same room, not fighting, has movement points), then recursively calls doMove on the follower. Failures are logged but don't prevent the leader's movement.

Mount and rider relationships require special handling. Mounts move with riders automatically, updating the riding pointer's room. Riders who fall off during movement via crashLanding need the riding pointer cleared and potentially take fall damage.

### checkFalling Recursive Descent

The checkFalling function in physics.cc handles falling through multiple vertical rooms. It uses tail recursion with a count parameter tracking fall distance.

Immortals at GOD_LEVEL1 or higher bounce without damage, sending a message and returning false immediately. Flying characters or those on flying mounts return false without falling.

Fall distance thresholds calculate num1 (safe distance) as 10 for characters with SKILL_CATFALL or SPELL_FEATHERY_DESCENT, otherwise 5. The damage zone threshold num2 is num1 + 5, marking where damage calculation begins.

Room validation checks roomp existence, dir_option[DIR_DOWN] existence, and non-NOWHERE destination. If any fail, falling stops. The function calls real_roomp to get the destination room pointer and validates it's not null.

Flying sector catching occurs if the destination is an air sector. The function sends a message, sets POSITION_FLYING via setFlying, and returns false without damage. This allows natural catching by air currents.

Spatial transfer uses -- and += operators to move the character from the current room to the lower room. This updates all spatial pointers including roomp, stuff linkage, and rider chains.

Landing detection checks if the destination is not an air sector. If count exceeds num1, fallKill checks for instant death (20+ rooms), returning DELETE_THIS if death occurs. If count exceeds num2, damage calculation multiplies count by number(40, 80), applies skill reductions (divide by 2), applies water sector reductions (divide by 2), then calls reconcileDamage. If reconcileDamage returns -1, the function returns DELETE_THIS immediately.

The crashLanding call forces POSITION_SITTING after landing, potentially causing mount falls. If crashLanding returns DELETE_THIS, the function must propagate it immediately without further execution.

Recursive continuation occurs when the destination is still an air sector. The function returns checkFalling(count + 1), incrementing the fall distance and repeating the entire process. This continues until landing, catching, or death.

### crashLanding Position Changes

The crashLanding function in movement.cc handles forced position changes from falls, knockdowns, or environmental effects. It takes a target position and force flag, returning DELETE_THIS if the character dies.

Forced landings (force=true) bypass resistance checks. The function immediately calls setPosition with the target, calls updatePos to apply position effects, then handles mount falling via fallOffMount if riding exists. The return value from fallOffMount must be checked for DELETE_THIS and propagated.

Normal landings (force=false) allow resistance. If the character's current position is standing or better, ground fighting skill provides a chance to stay standing. The function calls getSkillValue, then bSuccess with the skill value and SKILL_GROUNDFIGHTING. Success returns false without changing position.

Position resistance checks call bSuccess with getPosition() and the target position. Higher positions resist lower positions better. Success returns false, staying in the current position without modification.

Position application calls setPosition with the target and updatePos to recalculate stats affected by position. This includes AC, damage bonuses, and combat penalties.

Mount handling for normal landings mirrors forced landings: if riding exists, fallOffMount is called, its return value checked for DELETE_THIS, and propagated if set. This creates recursive crashLanding chains as the rider falls off and potentially crashes again.

Flying position special case handles POSITION_FLYING targets. If the character is already flying via isFlying, return false without changes. Otherwise, the character lacks flight capability and begins falling. The function sends plummet messages to room and character, then calls checkFalling. If checkFalling returns DELETE_THIS, the function propagates it immediately.

### checkDrowning Damage Application

The checkDrowning function in being.cc applies drowning damage to characters in underwater sectors without protection. It returns DELETE_THIS if the character dies from drowning.

Protection checks short-circuit early. The function returns false immediately for NPCs (only PCs drown), null roomp pointers, non-underwater sectors, and characters with AFF_WATERBREATH.

Damage calculation uses number(20, 40) for random damage per tick. The function calls reconcileDamage with the calculated damage, the character pointer, and DAMAGE_DROWN type.

Death detection checks if reconcileDamage returns -1. This is the sentinel value indicating death, NOT a DELETE flag. The code must check == -1 explicitly and return DELETE_THIS if true. Using IS_SET_DELETE fails silently because -1 is not a valid flag bitfield.

Suffocation messages trigger when health drops below 25% of maximum. The function compares getHit() to getMaxHit() / 4, sending drowning messages to both character and room if below threshold. This provides warning before death.

### procCharDrowning Scheduler Adapter

The procCharDrowning process in socket.cc serves as the scheduler adapter for drowning mechanics. It converts DELETE flag returns to bool for the scheduler framework.

The run method takes pulse and character pointer, calling checkDrowning on the character. It checks the return value with IS_SET_DELETE(rc, DELETE_THIS). If set, the function returns true, signaling the scheduler to delete the character object. Otherwise it returns false, keeping the character active.

Pulse frequency is Pulse::DROWNING (36 ticks, 3.6 seconds). This provides regular damage application without overwhelming the character with tick spam. The 20-40 damage per 3.6 seconds is survivable for high-level characters but lethal for extended underwater exposure.

Registration occurs during descriptor initialization, adding procCharDrowning to the character's active processes. Underwater sector entry does not trigger registration; the process is always active but short-circuits via the sector check.

### riverFlow and Current Movement

The riverFlow function in being.cc handles movement from river currents. It returns DELETE_THIS if current movement kills the character.

Early exit conditions check for null roomp, riverSpeed of 0 or less, and percentage chance against riverSpeed. If number(0, 100) exceeds speed, the function returns false without moving. Sitting characters have speed doubled, making them more susceptible to current.

Swimming resistance allows characters to resist current. The function calls bSuccess with SKILL_SWIMMING skill value, returning false on success with a message indicating swimming against current. This provides skilled swimmers control over movement.

Current movement messages use act to inform character and room of the sweep direction via dirs[dir]. Then the function calls doMove with the current's direction. The return value from doMove must be checked for DELETE_THIS and propagated immediately, as movement can trigger traps, death rooms, or other lethal effects.

### genericTeleport Random Destination

The genericTeleport function in magicutils.cc transports characters to random valid destinations. It takes silent, keepZone, and unsafe flags, returning DELETE_THIS if destination effects kill the character.

Random selection loops indefinitely until finding a valid destination. For zone-restricted teleports (keepZone=true), it calculates minroom and maxroom from zone_table boundaries. For unrestricted teleports, it uses number(100, top_of_world). The function calls real_roomp to validate the room exists and checks zone_table enabled status.

Safety exclusions (unsafe=false) filter destinations: ROOM_PRIVATE prevents private room teleports, ROOM_HAVE_TO_WALK prevents must-walk rooms, ROOM_DEATH prevents death rooms, and flying sectors are excluded to prevent dropping characters without flight. Unsafe mode bypasses all checks, allowing teleport to any valid room.

Exit messages (silent=false) send shimmer-out messages to room and character before movement. The function then handles dismounting by iterating rider and riding chains, transferring all related entities.

Spatial transfer uses -- to remove from current room and += to add to destination room. Enter messages send shimmer-in to the room, then call doLook for character visibility.

Entry effect processing calls genericMovedIntoRoom with the destination room pointer and -1 direction (indicating teleport). This triggers traps, death room effects, and sector-specific mechanics. The return value must be checked for DELETE_THIS and propagated immediately.

### TPortal::enterMe Portal Traversal

The enterMe function in obj_portal.cc handles portal traversal. It takes a TBeing pointer and returns DELETE_THIS, DELETE_VICT, or both, indicating portal and/or character deletion.

State validation checks EXIT_CLOSED and EXIT_NOENTER flags on portal_state. If either is set, the function sends appropriate messages and returns false without movement.

Combat restrictions prevent berserk characters from entering portals while fighting. This prevents exploiting portals for combat escape during rage.

Destination validation calls real_roomp with getTarget() to retrieve the destination room. Invalid destinations send terror messages and return false, preventing movement to null or deleted rooms.

Capacity checks retrieve moblim from destination room and count occupants via MobCountInRoom. At-capacity rooms trigger invisible wall messages and return false without entry.

Trap handling checks EXIT_TRAPPED on portal_state, calling triggerPortalTrap if set. The return value combines DELETE_ITEM and DELETE_THIS flags. If both are set, both portal and character are destroyed, returning DELETE_THIS | DELETE_VICT. If only DELETE_THIS is set, character died but portal survives, returning DELETE_VICT. If only DELETE_ITEM is set, portal was destroyed but character survived, returning false. If character is no longer in the same room (teleported by trap), return false without portal traversal.

Character transfer uses -- to remove from current room and thing_to_room to add to destination. The function calls doLook to show the new room to the character.

Follower transfer uses two passes: first mounts, then other followers. This ensures mounts are in place before riders enter. Each follower undergoes the same validation and transfer process.

Charge consumption occurs on both ends. The function calls findMatchingPortal to locate the paired portal at the destination. If found and charges are positive, decrement charges or delete if reaching 1. Then apply the same logic to the source portal, potentially returning DELETE_THIS if the source depletes.

## Troubleshooting

### Character Dies After Movement Command

Check if DELETE_THIS return from movement functions is being propagated. Every call to doMove, rawMove, checkFalling, crashLanding, or genericTeleport must check IS_SET_DELETE on the return value and propagate DELETE_THIS immediately without further execution. Continuing after DELETE_THIS causes use-after-free.

Verify reconcileDamage death checks use == -1, not IS_SET_DELETE. The reconcileDamage function returns -1 as a sentinel value, which is not a DELETE flag. Code checking IS_SET_DELETE(rc, DELETE_VICT) after reconcileDamage will never detect death and continue execution on deleted objects.

Check crashLanding return values in fall damage paths. The crashLanding function can return DELETE_THIS from mount falling chains. Code calling crashLanding must check the return value immediately and propagate DELETE_THIS up the stack.

### Iterator Crashes in Group Movement

Verify next pointers are cached before modifications. When iterating followers, riders, or stuff lists during movement, cache the next pointer before calling -- or += operators. These operators modify spatial pointers including next links, invalidating iterators.

Check riverFlow and group fall handling. River current and falling handling move multiple entities, modifying their spatial relationships. Use the pattern: cache next, perform operations, advance to cached next.

### Portal Traversal Issues

Validate portal return value handling. The enterMe function returns combinations of DELETE_THIS and DELETE_VICT. Callers must check both flags: IS_SET_DELETE(rc, DELETE_THIS) indicates portal consumed, IS_SET_DELETE(rc, DELETE_VICT) indicates character died. Both can be set simultaneously.

Check charge counting on both ends. Portal charges decrement on source and destination portals. If either portal reaches 1 charge, it must be deleted. Missing deletion causes portals to exist with 0 charges.

Verify trap handling before traversal. Portal traps trigger via triggerPortalTrap, which can teleport the character elsewhere, destroy the portal, kill the character, or any combination. The enterMe function must check all DELETE flags and sameRoom status before proceeding with traversal.

### Teleportation Target Issues

Ensure destination validation after random selection. The genericTeleport function loops until finding a valid room, but the room pointer can still be null or invalid between checks. Always validate real_roomp returns non-null before use.

Check room flag exclusions match intent. Different teleport methods have different exclusion lists. Random teleport excludes ROOM_DEATH and flying sectors, astral walk excludes ROOM_PRIVATE and ROOM_NO_MAGIC, summon excludes ROOM_NO_ESCAPE and ROOM_ARENA. Verify the exclusion logic matches the spell/ability requirements.

Verify genericMovedIntoRoom is called after teleport. Entry effects including traps and death rooms only trigger if genericMovedIntoRoom is called. Silent teleports often skip this, which is correct, but explicit teleport spells must include the call and check for DELETE_THIS.

### Flying and Landing Problems

Check flying sector entry and exit logic. Entering SECT_MAKE_FLY automatically sets POSITION_FLYING. Exiting requires actual flight capability (SPELL_FLY, AFF_FLYING, or racial). Code must validate flight ability before allowing exit to air or vertical sectors.

Verify crashLanding handles POSITION_FLYING specially. If target position is POSITION_FLYING but character cannot fly, crashLanding initiates falling via checkFalling. The return value from checkFalling must be checked and propagated.

Check mount flying state during movement. Mounted characters inherit flight from their mount. If the mount is flying via isFlying(), the rider does not fall. Dismounting during falls requires checking rider flight ability separately.

### Water and Drowning Issues

Ensure procCharDrowning is registered. Drowning damage only applies if the scheduler process is active. Verify procCharDrowning is added to the character's process list during descriptor initialization.

Check AFF_WATERBREATH immunity. Both racial water breathing and spell-granted AFF_WATERBREATH must be checked. Code checking only isAffected(AFF_WATERBREATH) misses racial immunity.

Verify underwater sector detection. Only sectors where isUnderwaterSector returns true trigger drowning. Surface water sectors do not cause drowning, even though they are water sectors per isWaterSector.

### Door State Synchronization

Check both sides are updated for door operations. Opening, closing, locking, and unlocking must modify both exitp->condition in the current room and back->condition in the destination room. Failing to update both sides causes desync where one room sees the door open and the other sees it closed.

Verify reverse direction calculation. The rev_dir function maps directions to opposites: north to south, up to down, northeast to southwest. Incorrect reverse direction updates the wrong exit, causing confusion and invalid state.

Check exit existence before modifying back pointer. The back exit may not exist (one-way exits). Code must validate back is non-null before updating back->condition flags.
