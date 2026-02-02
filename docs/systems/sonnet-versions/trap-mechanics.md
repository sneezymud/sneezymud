---
title: Trap Mechanics System
category: critical
keywords: [traps, doorTrapT, TTrap, springTrap, triggerTrap, disarmMe, detectMe, reconcileDamage, DELETE_THIS, DELETE_VICT, DELETE_ITEM, iterator_safety, room_wide_effects, detection, disarming]
related: [memory-safety.md, damage-pipeline.md, command-implementation.md, spatial-relationships.md]
primary_symbols:
  functions: [springTrap, triggerTrap, triggerDoorTrap, triggerContTrap, disarmMe, detectMe, reconcileDamage, checkForMoveTrap, checkForGetTrap]
  classes: [TTrap, TBeing, roomDirData]
  files: [code/code/misc/trap.h, code/code/misc/trap.cc, code/code/disc/disc_thief_looting.cc, code/code/obj/obj_trap.cc]
---

## Overview

The trap system provides dangerous obstacles throughout the game world that can be attached to doors, containers, rooms, and as standalone objects. Traps present the highest concentration of DELETE flag patterns in the codebase with over 267 flag operations requiring precise iterator safety and pointer validation to prevent crashes.

Traps can be placed on doors triggering on movement, containers triggering on opening or item retrieval, rooms affecting all beings simultaneously, character-placed mines, thrown grenades, and arrows embedded in victims. The system provides 16 trap types ranging from simple poison and spike traps to complex teleportation and area-effect explosions.

Thief characters can detect and disarm traps using specialized skills with success rates calculated from skill values and trap difficulty. Failed disarm attempts trigger the trap on the thief, creating critical DELETE flag scenarios where the acting character dies from their own action.

All trap damage flows through reconcileDamage which returns -1 on death rather than DELETE flags, creating a dual return value pattern that must be distinguished. Room-wide traps require post-increment iterator patterns because they modify the container being iterated during damage application.

Misusing this system causes crashes through continuing execution after DELETE flags, using pre-increment iterators during deletion, treating reconcileDamage -1 returns as DELETE flags, and failing to validate pointers after trap triggers.

## Patterns

### Trap Placement and Triggering

Door traps store the trap type enum in roomDirData->trap_info and are marked with the EXIT_TRAPPED condition flag. Movement through a trapped exit checks TRAP_EFF_MOVE and calls triggerDoorTrap which dispatches to type-specific damage functions.

Container traps use the TTrap object type which inherits from TObj and stores trap_type and trap_level members. Opening a container or taking items from inside checks TRAP_EFF_OBJECT and iterates through all beings in the room using post-increment pattern to handle deletion during iteration.

Room-wide traps set the TRAP_EFF_ROOM flag causing the trap to affect all beings in the room simultaneously when triggered. The trigger function must iterate the room contents safely using post-increment because damage can delete beings during iteration.

Character-placed mines and thrown grenades use TRAP_TARG_MINE and TRAP_TARG_GRENADE target types, storing old_parent pointers that must be validated before use because detonation can move or delete the container.

### Safe Iterator Pattern for Trap Effects

Container and room iteration during trap triggers must use post-increment to capture the next position before deletion occurs.

The safe pattern extracts the current element and advances the iterator in a single operation using `*(it++)` which evaluates the dereference before the increment side effect. This ensures the iterator points to the next valid element even if the current element is removed from the container during processing.

The dangerous pre-increment pattern `++it` advances after processing the current element, causing the iterator to advance from invalidated memory if the element was deleted, resulting in undefined behavior and crashes.

Build a safe list pattern for nested container explosions by first collecting all contents into a vector, then iterating the vector and validating each element still exists in the expected container before processing.

### DELETE Flag Propagation Patterns

The triggerTrap family of functions returns DELETE_THIS when the triggering character dies, DELETE_VICT when a victim parameter dies, and DELETE_ITEM when the trap object itself is destroyed.

Failed disarm attempts call triggerTrap which can return DELETE_THIS if the thief dies, but disarmMe must translate this to DELETE_VICT for its caller because the thief is a parameter to disarmMe but the subject of triggerTrap.

Room-wide traps iterate multiple victims and must accumulate DELETE flags, returning early if the triggering character dies but continuing iteration for other victims if only DELETE_VICT is set.

Combined flags occur when both attacker and victim die, requiring the function to return both DELETE_THIS and DELETE_VICT bits set, letting the caller handle dual deletion.

### reconcileDamage Return Value Pattern

All trap damage flows through reconcileDamage which returns -1 on victim death, not a DELETE flag. The calling code must check `rc == -1` and then return DELETE_VICT to signal the death to its caller.

IS_SET_DELETE checks will not detect the -1 return value because it tests specific bit patterns while -1 has all bits set. The correct pattern is an explicit equality check against -1 followed by returning the appropriate DELETE flag.

Type-specific damage functions call getDoorTrapDam to calculate base damage modified by trap type, skill level, and resistance, then pass the result to reconcileDamage and check for -1 return.

### Thief Skill Mechanics

Disarming checks trap charges and returns false if already disarmed, calls bSuccess with SKILL_DISARM_TRAP to determine outcome, sets charges to zero on success, or calls triggerTrap on failure and translates DELETE_THIS to DELETE_VICT if the thief dies.

Detection uses bSuccess with skill value divided by 10 plus 1, creating a maximum 11% detection rate even at skill 100 to maintain challenge. Detection is passive and does not trigger traps on failure.

Door trap detection and disarming accesses exitp->trap_info to identify trap type and checks EXIT_TRAPPED condition flag. Container trap operations use TTrap member functions getTrapCharges, setTrapCharges, getTrapDamType, and getTrapLevel.

## Reference

### Trap Types (doorTrapT Enum)

DOOR_TRAP_NONE provides the untrapped state with value 0. DOOR_TRAP_POISON applies poison affect with 1.0x damage multiplier. DOOR_TRAP_SPIKE delivers 1.2x pierce damage. DOOR_TRAP_SLEEP forces rest with AFF_SLEEP affect. DOOR_TRAP_TNT creates 2.0x blast damage affecting all in room.

DOOR_TRAP_BLADE delivers 1.2x slash damage with bleeding effect. DOOR_TRAP_FIRE applies 0.8x cumulative burn damage over time. DOOR_TRAP_ACID delivers 1.1x damage with structure degradation. DOOR_TRAP_DISEASE applies 0.9x damage with 24-hour AFF_DISEASE affect.

DOOR_TRAP_HAMMER delivers 1.0x blunt damage with stun effect. DOOR_TRAP_FROST applies 1.0x cold damage with AFF_COLD affect. DOOR_TRAP_TELEPORT causes random displacement without direct damage. DOOR_TRAP_ENERGY delivers 1.0x magic damage. DOOR_TRAP_BOLT delivers 1.1x lightning damage. DOOR_TRAP_DISK delivers 1.2x slash damage as projectile. DOOR_TRAP_PEBBLE delivers 0.9x area-effect blunt damage.

### Trap Target Types (trap_targ_t Enum)

TRAP_TARG_DOOR marks traps on doors and exits. TRAP_TARG_CONT marks traps on containers. TRAP_TARG_MINE marks character-placed mines. TRAP_TARG_GRENADE marks thrown grenades. TRAP_TARG_ARROW marks arrows stuck in victims.

### Trap Effect Flags

TRAP_EFF_MOVE triggers on movement through exit. TRAP_EFF_OBJECT triggers on get or put object operations. TRAP_EFF_ROOM affects all beings in the room. TRAP_EFF_NORTH through TRAP_EFF_SW encode directional triggers for the eight cardinal and ordinal directions plus up and down.

TRAP_EFF_THROW triggers when object is thrown. TRAP_EFF_ARMED1 through TRAP_EFF_ARMED3 represent three arming states for complex traps. MAX_TRAP_EFF equals 17.

### Primary Functions by Category

Trigger functions: springTrap serves as main dispatcher, triggerTrap handles character-triggered events, triggerDoorTrap activates door traps, triggerContTrap activates container traps, triggerPortalTrap handles portal traps, triggerArrowTrap handles arrow impacts, triggerMineTrap handles mine explosions, triggerGrenadeTrap handles grenade detonations.

Detection functions: checkForMoveTrap checks for movement traps, checkForInsideTrap checks container-inside traps, checkForAnyTrap provides generic trap checking, checkForGetTrap checks when picking up objects, checkForPortalTrap handles portal-specific checks.

Thief skill functions: disarmTrapObj disarms object traps, disarmTrapDoor disarms door traps, disarmMe implements TTrap disarm logic, detectMe implements TTrap detection logic, detectTrapObj detects object traps, detectTrapDoor detects door traps, detectSecret finds hidden doors.

Damage calculation functions: trapDoorPoisonDamage through trapDoorPebbleDamage calculate type-specific damage with modifiers, getDoorTrapDam computes base damage from trap level and skill modifiers.

### Damage Calculation Formula

Base damage equals trap_level multiplied by classAmount. Skill modifier derives from caster level and skill learning percentage. Resistance modifier applies from victim resistances to the damage type. Type-specific multipliers range from 0.8x for fire to 2.0x for TNT explosions.

### Integration Points

Movement system calls checkForMoveTrap during room transitions. Container system calls checkForGetTrap during opening and item retrieval. Combat system processes trap damage through reconcileDamage using standard damage pipeline. Thief skills SKILL_DISARM_TRAP and SKILL_DETECT_TRAP control success rates via bSuccess checks.

Spell system applies trap effects through affectJoin for sleep, poison, and disease affects. Scheduler runs procObjSpecProcs periodically for trap-specific code. Room system stores door trap information in roomDirData exit structures.

## Implementation

### Door Trap Storage and Retrieval

The roomDirData structure contains trap_info storing the doorTrapT enum value and condition field containing EXIT_TRAPPED flag when armed. fname extracts the exit keyword for messaging. Disarm operations access exitp->trap_info to identify trap type using trap_types string array.

getSkillValue retrieves SKILL_DISARM_TRAP for success calculation. bSuccess compares skill value against trap difficulty. Success sets EXIT_TRAPPED to 0 and sends disarm messages. Failure calls triggerTrap on the thief.

### Container Trap Implementation

TTrap inherits from TObj adding trap_type, trap_level, and trap_charges members. getTrapCharges returns remaining trigger count. setTrapCharges modifies trigger count. getTrapDamType returns the trap type enum. getTrapLevel returns difficulty level.

Opening containers triggers iteration through roomp->stuff using post-increment pattern. Each TBeing in the room receives springTrap call with TRAP_TARG_CONT target type. IS_SET_DELETE checks for DELETE_THIS and performs deletion with null pointer assignment.

### Room-Wide Trap Execution

TRAP_EFF_ROOM flag causes springTrap to iterate all beings in the room. The iteration uses `*(it++)` to advance before processing. getDoorTrapDam calculates damage for each victim. reconcileDamage applies damage and returns -1 on death. The -1 return triggers deletion and sets rc to DELETE_VICT.

The function continues iteration after DELETE_VICT because other beings may still be alive. Only DELETE_THIS causes early return because the triggering character died.

### Grenade Detonation Mechanics

Grenade objects store old_parent pointer before detonation. Explosion can move or delete containers making old_parent stale. Safe code validates old_parent exists and checks inRoom returns valid room number before dereferencing.

springTrap with TRAP_TARG_GRENADE iterates room contents using post-increment pattern. IS_SET_DELETE check for DELETE_ITEM triggers grenade deletion. Multiple grenades in same room require iterator safety.

### Thief disarmMe Implementation

disarmMe checks getTrapCharges returns greater than zero. Retrieves skill value via getSkillValue for SKILL_DISARM_TRAP. Calls bSuccess to determine outcome. Success branch sends click message, calls act for room notification, calls setTrapCharges with zero, returns TRUE.

Failure branch sends whoops message, calls act for room notification, calls thief->triggerTrap passing this pointer. Checks IS_SET_DELETE on return value for DELETE_THIS. Translates DELETE_THIS to DELETE_VICT because thief is parameter not subject. Returns TRUE if thief survived.

### Thief detectMe Implementation

detectMe retrieves skill value and divides by 10 plus 1. Calls bSuccess with modified skill value and SKILL_DETECT_TRAP. Returns TRUE on success or FALSE on failure. Detection never triggers traps allowing safe repeated attempts.

### Delete Flag Translation Logic

When triggerTrap is called on a being and that being dies, it returns DELETE_THIS because the trap killed the subject. If that being was passed as a parameter to the calling function, the caller must translate DELETE_THIS to DELETE_VICT because from the caller's perspective the parameter died not the caller.

disarmMe demonstrates this pattern by calling thief->triggerTrap where thief is the subject, but thief is a parameter to disarmMe, so DELETE_THIS from triggerTrap becomes DELETE_VICT for disarmMe's return.

Room-wide traps demonstrate accumulation by maintaining rc and combining multiple DELETE_VICT flags from multiple victim deaths while checking for DELETE_THIS to detect triggerer death requiring early return.

## Troubleshooting

### Crash: Iterator Invalidation During Room-Wide Trap

Symptom: Segfault during trap processing with backtrace showing stuff.end() comparison or iterator increment. Cause: Using pre-increment pattern `++it` or range-based for loop while deleting elements.

Fix: Replace loop header with `for (StuffIter it = roomp->stuff.begin(); it != roomp->stuff.end();)` and use `TThing* t = *(it++)` to extract and advance in single operation. Check IS_SET_DELETE before any operations on extracted pointer.

### Crash: Use After Free in Trap Damage Messaging

Symptom: Heap-use-after-free in sendTo call after trap damage. Cause: Calling victim methods after reconcileDamage returned -1 indicating death.

Fix: Check `rc == -1` immediately after reconcileDamage. If true, delete victim, set pointer to null, and return DELETE_VICT without executing subsequent code. Never call methods on victim after -1 return.

### Bug: IS_SET_DELETE Not Detecting Death from reconcileDamage

Symptom: Victim continues executing after reconcileDamage should have killed them. Cause: Using IS_SET_DELETE to check reconcileDamage return which returns -1 not a DELETE flag.

Fix: Change `if (IS_SET_DELETE(rc, DELETE_VICT))` to `if (rc == -1)` when checking reconcileDamage returns. Then set rc to DELETE_VICT before returning to caller.

### Crash: Grenade Detonation Accessing Stale Parent

Symptom: Segfault in container operations during grenade explosion. Cause: Explosion moved grenade to different container or deleted container making old_parent pointer invalid.

Fix: After storing old_parent, validate before use with `if (old_parent && old_parent->inRoom() != Room::NOWHERE)`. Do not dereference if validation fails.

### Crash: Thief Continues After Failed Disarm Death

Symptom: Use-after-free when thief dies from failed disarm but caller continues using thief pointer. Cause: Caller did not check DELETE_VICT return from disarmMe.

Fix: Add check `if (IS_SET_DELETE(rc, DELETE_VICT))` after disarmMe call. Delete thief, set pointer to null, and return DELETE_THIS to propagate to next caller level.

### Bug: Trap Triggers Multiple Times on Same Action

Symptom: Trap fires repeatedly when it should fire once. Cause: Trap charges not decremented or not checked before trigger.

Fix: Check getTrapCharges before allowing trigger. Call setTrapCharges with decremented value after trigger. Check for zero charges and skip trigger if already expended.

### Crash: Nested Container Explosion Iterator Corruption

Symptom: Segfault during container-in-container trap processing. Cause: Inner container explosion invalidates outer container iterator.

Fix: Build safe list first using vector to collect all contents. Iterate vector and validate each element with `if (t && t->parent == container)` before processing. This pattern prevents iterator invalidation.

### Bug: Room-Wide Trap Not Affecting All Beings

Symptom: Some beings in room avoid damage from TRAP_EFF_ROOM trap. Cause: Early return from iteration or missing TRAP_EFF_ROOM flag check.

Fix: Ensure TRAP_EFF_ROOM is checked before starting iteration. Continue iteration after DELETE_VICT to process remaining beings. Only return early on DELETE_THIS when triggerer dies.

### Crash: Door Trap Continues After Triggerer Death

Symptom: Segfault in movement code after door trap. Cause: Movement code did not check DELETE_THIS return from triggerDoorTrap.

Fix: Add `if (IS_SET_DELETE(rc, DELETE_THIS)) return DELETE_THIS;` immediately after triggerDoorTrap call. Do not execute movement completion or messaging after trap death.

### Memory Leak: Failed to Delete Grenade After Detonation

Symptom: Grenade objects accumulate in memory after explosions. Cause: Not checking DELETE_ITEM return or not deleting grenade object.

Fix: Check `if (IS_SET_DELETE(rc, DELETE_ITEM))` after springTrap. Delete the trap pointer and set to null. Remove from container if still has parent pointer.
