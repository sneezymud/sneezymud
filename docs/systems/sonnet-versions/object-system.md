---
title: Object System
category: critical
keywords: [itemTypeT, TObj, makeNewObj, container, consumable, DELETE_THIS, spatial, weight, volume, trap, lock, decay]
related: [memory-safety.md, spatial-relationships.md, trap-mechanics.md, affects-system.md, material-system.md, scheduler-pulses.md]
primary_symbols:
  functions: [makeNewObj, assignFourValues, getFourValues, objectDecay, putSomethingInto, has_key, triggerContTrap, lightDecay, eatMe, drinkMe]
  classes: [TObj, TBaseContainer, TOpenContainer, TLight, TFood, TDrinkCon, TBed, TVehicle, TAudio, TBook]
  files: [code/obj/obj.h, code/obj/obj_base_container.cc, code/obj/obj_food.cc, code/obj/obj_light.cc]
---

# Object System

## Overview

SneezyMUD's object system implements a polymorphic type hierarchy where every game object inherits from TObj and declares its specific type through the itemTypeT enumeration. Objects store type-specific data in four integer value fields whose meanings vary by subclass. The system encompasses weapons and armor, containers with lock and trap mechanics, consumable items like food and light sources, and utility objects including vehicles, books, and furniture.

Object lifecycle begins with factory creation through makeNewObj which instantiates the correct subclass based on itemTypeT. Objects are automatically registered in the global object_list on construction and participate in scheduler processing. Destruction follows C++ destructor chains with specialized cleanup: standard containers recursively delete contents while corpses relocate items to preserve loot.

The spatial relationship system tracks containment through parent pointers and stuff lists. Weight and volume calculations traverse these relationships recursively with special cases for weightless containers and component reduction. Critical safety requirements include advancing iterators before removal, checking DELETE flags from dangerous operations, and maintaining bidirectional pointer consistency.

Container mechanics extend the base system with openable/closeable states, lock and key matching, trap setting and disarming, and ghost trap anti-metagaming. Consumables implement temporal depletion: lights burn fuel over time, food spoils before final decay, and drink containers track liquid type with condition effects.

## Patterns

### Factory Instantiation

Use makeNewObj to create objects from database definitions. The factory consults itemTypeT and instantiates the correct subclass, then read_object populates fields from cached or fresh database query. Always validate the return value before using the object.

```cpp
TObj* obj = read_object(vnum, VIRTUAL);
if (!obj) {
    // vnum doesn't exist or factory failed
    return;
}
*room += *obj;  // Place in world
```

Direct construction bypasses the database for programmatically-generated objects. Manually populate all required fields including name, descriptions, and type-specific values.

### Safe Container Iteration

Advance iterators before modifying the container to prevent invalidation. The post-increment pattern ensures the iterator moves forward before the current item is removed from stuff.

```cpp
for (StuffIter it = container->stuff.begin(); it != container->stuff.end();) {
    TThing* t = *(it++);  // Advance FIRST
    --(*t);               // Remove from container
    *destination += *t;   // Relocate safely
}
```

Never use pre-increment when removing items during iteration. The iterator becomes invalid the moment operator-- removes the item from stuff.

### DELETE Flag Propagation from Traps

Trap operations can kill the character mid-function. Check IS_SET_DELETE after every dangerous call and propagate DELETE_THIS immediately.

```cpp
rc = objDamage(DAMAGE_TRAP_FIRE, damage, container);
if (IS_SET_DELETE(rc, DELETE_THIS))
    return DELETE_THIS;

rc = flameEngulfed();
if (IS_SET_DELETE(rc, DELETE_THIS))
    return DELETE_THIS;
```

Some trap types perform multiple damage calls. Fire traps inflict direct damage then check flame engulfment. Each call can independently kill the character.

### Weight and Volume Calculation

Container weight uses getTotalWeight which respects CONT_WEIGHTLESS and applies TComponent reduction. The pweight parameter controls whether to include the container's own weight.

```cpp
float totalWeight = 0;
if (!isContainerFlag(CONT_WEIGHTLESS))
    totalWeight = getCarriedWeight();
if (includeContainerWeight)
    totalWeight += getWeight();
```

Components get 10% weight reduction when carried to represent careful packing. Volume calculations apply material-based reduction from vol_mult plus 5% container packing bonus.

### Key and Lock Matching

The has_key function searches inventory, worn equipment, and keyrings recursively. Keys match by vnum comparison through keyCheck helper. Lock operations validate state transitions: lock requires closed container, unlock requires locked container, pick requires keyhole and not pickproof.

```cpp
if (!isClosed())
    // Cannot lock open container
else if (getKeyNum() < 1)
    // No keyhole
else if (!has_key(ch, getKeyNum()))
    // Missing key
else if (isContainerFlag(CONT_LOCKED))
    // Already locked
else
    addContainerFlag(CONT_LOCKED);
```

### Ghost Trap Anti-Metagaming

Failed detect trap checks create ghost traps with zero damage and CONT_GHOSTTRAP flag. This prevents players from using absence of trap messages as proof of safety. Ghost traps appear real during examine but disarm automatically on first attempt.

### Food and Drink Consumption

Food consumption checks fullness before eating, applies race-based modifiers for fish and meat, then calls Poisoned and Spoiled for effect processing. Food items decay into spoiled state before final deletion, extending decay time based on volume.

Drink consumption calculates amount based on current thirst and liquid intoxication, updates DRUNK/FULL/THIRST conditions with race and body mass modifiers, then checks for poison and disease risks. Pools carry dysentery risk for non-immune characters.

### Light Decay and Burnout

Lit lights call lightDecay each tick to decrement curBurn. When curBurn reaches zero, putLightOut extinguishes the light and notifies the character and room. Flicker warnings display when curBurn drops below 4.

Refueling requires the light to be extinguished and not at maximum capacity. TFuel objects transfer fuel units then delete themselves when empty.

### Object Decay Timing

Objects with decay_time greater than -1 call decayMe each tick. When decay_time reaches zero, objectDecay provides type-specific behavior. Food transitions to spoiled state with extended timer. Corpses show decay message and log player corpse info. Default behavior relocates contents then returns DELETE_THIS.

### Vehicle Movement Control

Vehicles move through vehiclePulse called by scheduler proc. Movement occurs at intervals based on speed setting. Auto-pathing engages when only one valid exit exists besides reverse direction. Boats can move into non-water sectors once but must return to water.

Update_exits synchronizes interior room exits to vehicle's current location before each movement check. Whole-zone vehicles update all zone exits when moving.

## Reference

### itemTypeT Core Types

**ITEM_UNDEFINED (0)**: Invalid or uninitialized object state. Factory returns nullptr for this type.

**ITEM_LIGHT (1)**: Refillable light sources implemented by TLight. Stores light amount, max burn time, current burn time, and lit status in value fields.

**ITEM_FOOD (19)**: Edible items implemented by TFood. Value fields store fullness value and food flags for poisoned/spoiled/fished/butchered state.

**ITEM_DRINKCON (17)**: Drink containers implemented by TDrinkCon. Value fields track maximum capacity, current units, liquid type, and drink flags.

**ITEM_CHEST (15)**: Lockable storage implemented by TChest subclass of TOpenContainer. Value fields encode max weight, container flags with trap data, key vnum, and max volume.

**ITEM_BAG (27)**: Expandable containers implemented by TBag and related subclasses. Use same value encoding as chests.

**ITEM_CORPSE (28)**: NPC corpses implemented by TCorpse. Relocate contents on destruction rather than deleting them.

**ITEM_PCORPSE (47)**: Player corpses implemented by TPCorpse. Critical for player equipment recovery.

**ITEM_BED (40)**: Resting furniture implemented by TBed. Value fields encode position restrictions, max users, size limits, and regeneration bonus.

**ITEM_VEHICLE (61)**: Drivable objects implemented by TVehicle. Extends TPortal with direction and speed controls.

### Container Flags

**CONT_CLOSEABLE (bit 0)**: Container supports open and close operations. Required for all lock and trap mechanics.

**CONT_CLOSED (bit 1)**: Container is currently closed. Must be closed before locking or setting traps.

**CONT_LOCKED (bit 2)**: Container is locked. Requires matching key vnum or successful pick lock attempt.

**CONT_PICKPROOF (bit 3)**: Lock cannot be picked. Only matching key can unlock.

**CONT_JAMMED (bit 4)**: Lock is jammed from critical pick failure. Permanent until repaired.

**CONT_TRAPPED (bit 5)**: Container has active trap. Triggers on open attempt.

**CONT_GHOSTTRAP (bit 6)**: False trap from failed detect. Zero damage, auto-disarms.

**CONT_EMPTYTRAP (bit 7)**: Trap was successfully disarmed. Slot remains empty.

**CONT_WEIGHTLESS (custom)**: Contents weigh nothing. Used for bags of holding and similar magic containers.

### Trap Types

**DOOR_TRAP_TNT**: Explosive damage using DAMAGE_TRAP_TNT type.

**DOOR_TRAP_POISON**: Poison damage using DAMAGE_TRAP_POISON type.

**DOOR_TRAP_SLEEP**: Sleep effect with no direct damage.

**DOOR_TRAP_FIRE**: Fire damage followed by flame engulfment check. Requires DELETE flag checking after both calls.

**DOOR_TRAP_ACID**: Acid damage using DAMAGE_TRAP_ACID type.

**DOOR_TRAP_DISEASE**: Disease damage using DAMAGE_TRAP_DISEASE type.

**DOOR_TRAP_SPIKE**: Piercing damage using DAMAGE_TRAP_PIERCE type.

**DOOR_TRAP_BLADE**: Slashing damage using DAMAGE_TRAP_SLASH type.

**DOOR_TRAP_PEBBLE**: Bludgeon damage using DAMAGE_TRAP_BLUNT type.

**DOOR_TRAP_FROST**: Cold damage using DAMAGE_TRAP_FROST type.

**DOOR_TRAP_TELEPORT**: Teleports character to random room with no damage.

**DOOR_TRAP_ENERGY**: Energy damage using DAMAGE_TRAP_ENERGY type.

### Food Flags

**FOOD_POISON (1)**: Applies poison effect when eaten.

**FOOD_SPOILED (2)**: Causes food poisoning disease when eaten.

**FOOD_FISHED (4)**: Double benefit for characters with TALENT_FISHEATER.

**FOOD_BUTCHERED (8)**: Double benefit for characters with TALENT_MEATEATER.

### Drink Container Flags

**DRINK_POISON (1)**: Liquid is poisoned. Applies poison effect when consumed.

**DRINK_PERM (2)**: Never-emptying container. Fountains and magical sources use this.

**DRINK_SPILL (4)**: Liquid spills during movement. Reduces drink units progressively.

**DRINK_FROZEN (8)**: Liquid is frozen solid. Cannot drink until thawed.

### Condition Types

**DRUNK**: Intoxication level from 0 to 24. Above 15 prevents drinking unless thirsty.

**FULL**: Hunger satisfaction from 0 (starving) to 24 (over-full). Value of -1 for immortals.

**THIRST**: Thirst satisfaction from 0 (dehydrated) to 24 (quenched). Value of -1 for immortals.

**PEE**: Need to urinate tracked separately from other conditions.

**POOP**: Need to defecate tracked separately from other conditions.

Zero FULL or THIRST reduces HP and mana regeneration by 75%.

### Vehicle Constants

**VEHICLE_BOAT (1)**: Water vessel using nautical movement messages. Can enter non-water once but must return.

**VEHICLE_TROLLEY (2)**: Rail vehicle using rumbling movement messages.

**FAST_SPEED (100)**: Maximum speed with rapid movement messages.

**MED_SPEED (50)**: Standard operational speed.

**SLOW_SPEED (25)**: Minimum active speed with gentle movement messages.

### Light Intensity Descriptions

**Less than 3**: Dim light barely illuminating surroundings.

**3-7**: Moderately-bright light for normal visibility.

**8-14**: Bright light illuminating well.

**15-24**: Very bright light providing excellent visibility.

**25-34**: Extremely intense light bordering on blinding.

**35 and above**: Blinding light causing visibility impairment.

## Implementation

### TObj Base Class Architecture

The TObj constructor in structs.cc performs minimal initialization. Sets number to -1 indicating no prototype association, increments global objCount, and pushes the object onto object_list front. All subclasses automatically register through base constructor chain.

The destructor executes a complex cleanup sequence. First calls CMD_GENERIC_DESTROYED spec proc if present. Removes from casting list if applicable. Recursively deletes all contents by iterating stuff and calling delete on each item. Removes self from parent, room, equipment, or stuckIn relationships. Dismounts any riders. Scans all active tasks to cancel references. Removes from object_list. Decrements prototype count in obj_index. Cleans up strung descriptions if ITEM_STRUNG flag is set.

Copy constructor duplicates obj_flags and affected array, copies strung descriptions if ITEM_STRUNG, registers new object in object_list, and increments objCount. Does not copy spatial relationships like parent or roomp.

### Factory and Database Loading

The makeNewObj function in sys/db.cc switches on itemTypeT and returns appropriate subclass instance. Each case statement calls the matching constructor like new TLight for ITEM_LIGHT or new TGenWeapon for ITEM_WEAPON.

The read_object function calls makeNewObj then populates the object from obj_index cache or fresh database query. Sets name, descriptions, extra descriptions from database strings. Calls assignFourValues with val0 through val3 from database. Sets weight, cost, material, structure points from database. Applies affects and modifiers from database affect rows.

### Spatial Relationship Invariants

Forward pointer parent must match backward pointer in container stuff list. When item is in container, parent points to container and container stuff contains item. When item is in room, parent is NULL and roomp points to room and room stuff contains item. When item is in inventory, parent points to being and being stuff contains item.

The roomp pointer provides fast room lookup but must be NULL when inside container. Use roomOfObject helper to traverse parent chain and find actual room.

Operator+= validates item has no existing location pointers. Checks parent, equippedBy, stuckIn all NULL before adding to stuff. Sets item parent to this container. Clears item roomp to NULL. Adds item to stuff list. Handles TMergeable consolidation if applicable.

Operator-- removes item from stuff list. Clears item parent to NULL. Clears item roomp to NULL. Clears item in_room to Room::NOWHERE. Does not delete the item.

### Container Weight Recursion

The getTotalWeight method checks if this is TOpenContainer with CONT_WEIGHTLESS flag. If weightless, skips getCarriedWeight and uses zero for contents. Otherwise calls getCarriedWeight to sum recursive weight. If pweight parameter is true, adds container's own weight from getWeight.

The getCarriedWeight method iterates rider chain and sums getTotalWeight for each rider. Iterates stuff list and sums getTotalWeight for each item. For TComponent items, multiplies weight by 0.10 for 10% reduction. Returns total without including container's own weight.

### Container Volume Calculation

The getCarriedVolume method in TBaseContainer iterates rider chain and sums getTotalVolume for each rider. Iterates stuff list and sums getReducedVolume for each item. For TComponent items, multiplies volume by 0.10 for 10% reduction.

The getReducedVolume method retrieves base volume from getVolume. Applies material vol_mult divisor if material_nums has vol_mult greater than zero. Applies additional 5% packing reduction by multiplying by 0.95. Returns reduced volume.

Material vol_mult examples: cloth has vol_mult 2 for 50% reduction, metal has vol_mult 1 for no reduction, leather has vol_mult 3 for 33% reduction.

### Container Value Encoding

The assignFourValues method in TOpenContainer unpacks val1 into three separate fields. Lower byte 0-7 becomes container_flags. Byte 8-15 becomes trap_type cast to doorTrapT. Byte 16-23 becomes trap_dam cast to signed char. Val0 becomes max_weight, val2 becomes key_num, val3 becomes max_volume.

The getFourValues method packs three fields into val1. Bitwise OR of container_flags, trap_type shifted left 8 bits, and trap_dam shifted left 16 bits. Val0 receives max_weight, val2 receives key_num, val3 receives max_volume.

### Lock and Key Implementation

The has_key function in movement.cc uses keyCheck helper to compare object vnum against key parameter. First searches inventory stuff list for matching keys. Then searches inside any keyrings found in inventory. Then searches held items. Then searches worn equipment slots. For NPCs, also checks potential mob loads.

The keyCheck helper compares obj_index virtual number against key parameter. Also handles special case of objVnum -1 with getSnum matching key for dynamically generated keys.

The lockMe method validates container is closed before attempting lock. Validates keyNum is greater than zero indicating keyhole exists. Calls has_key to verify character possesses matching key. Checks not already locked. Adds CONT_LOCKED flag. Shows click message to character and act message to room.

The unlockMe method reverses the process. Validates locked state. Calls has_key for authorization. Removes CONT_LOCKED flag.

### Pick Lock Mechanics

Container picking in pickMe is instant resolution. Checks container is closed, has keyhole, is locked, is not pickproof, is not jammed. Gets skill value for SKILL_PICK_LOCK. Calls bSuccess to determine outcome. On success, removes CONT_LOCKED flag and shows click message. On critical failure from critFail check, adds CONT_JAMMED flag permanently. On normal failure, shows failure message.

Door picking uses task system with TASK_PICKLOCKS. Requires lockpick tool. Takes extended time. On task completion, similar success check but can jam door on critical failure.

### Trap Setting and Disarming

The trapMe method validates container is closeable, is closed, and is not already trapped. Starts TASK_TRAP_CONT task with 3 tick duration. Task stores trap type argument for later processing. On task completion, calculates trap damage based on skill level and sets container flags.

The disarmMe method checks for CONT_GHOSTTRAP first. Ghost traps always disarm successfully by removing flag and clearing trap type. Real trap disarm calls bSuccess with SKILL_DISARM_TRAP. On success, removes CONT_TRAPPED and adds CONT_EMPTYTRAP to mark disarmed state. On failure, triggers the trap immediately by calling triggerContTrap which may return DELETE flags.

### Trap Trigger Flow

The triggerContTrap method in TBeing switches on container trap type. Each case calls appropriate damage function like objDamage with matching damage type and trap damage amount. Fire trap calls objDamage for direct damage then flameEngulfed for engulfment check. Each dangerous call checks IS_SET_DELETE for DELETE_THIS and returns immediately if character died. Sleep trap applies sleep affect. Teleport trap calls genericTeleport to random room.

Critical pattern: check DELETE_THIS after EVERY objDamage, flameEngulfed, genericTeleport, or other dangerous call. Fire trap demonstrates proper pattern with two consecutive checks.

### Food Consumption Processing

The eatMe method in TFood checks character not already full unless immortal. Perceptive characters detect FOOD_SPOILED and discard instead of eating. Calculates adjust multiplier based on race: vampires get zero, fisheaters get 2.0 for FOOD_FISHED, meateaters get 2.0 for FOOD_BUTCHERED. Calls gainCondition with FULL and foodFill multiplied by adjust. Calls Poisoned with foodFill amount if FOOD_POISON flag set. Calls Spoiled with foodFill amount if FOOD_SPOILED flag set. Deletes self after consumption.

### Drink Consumption Processing

The drinkMe method in TBaseCup validates character does not have DISEASE_FOODPOISON, drink is not frozen, container has units remaining, character not too drunk or too full. Calculates amount based on current thirst and liquid drunk value, clamped between 1 and 15 units. Updates container weight by subtracting amount times SIP_WEIGHT unless DRINK_PERM flag set. Calls gainCondition for DRUNK, FULL, and THIRST with liquid-specific values multiplied by amount divided by 10. Checks DRINK_POISON flag and applies poison if set. Pools check for dysentery disease if liquid is water and character not immune.

### Condition Modification

The gainCondition method checks if condition is -1 indicating immortal status and returns immediately. For positive value changes, applies race modifier from getFoodMod or getDrinkMod. Applies body mass adjustment by multiplying by 180.0 divided by getWeight. For DRUNK condition, applies SKILL_ALCOHOLISM modifier if character has skill. Adds value to specials.conditions array at condition index. Clamps result to range 0 through 24 using max and min.

### Light Burn Processing

The lightDecay method checks isLit status. Calls addToCurBurn with -1 to decrement fuel. If curBurn reaches zero or below, sets curBurn to zero and calls putLightOut to extinguish. Shows burnout message to character and room. If curBurn less than 4, shows flicker warning.

The refuelMeFuel method in TFuel validates lamp maxBurn is not negative indicating non-refuelable. Validates lamp not already at full capacity. Validates lamp is not currently lit. Calculates use as minimum of lamp capacity remaining and fuel curFuel. Subtracts use from fuel curFuel. Adds use to lamp curBurn. If fuel curFuel reaches zero, deletes fuel container.

### Object Decay Chain

The updateObj method checks decay_time greater than -1. Calls decayMe to decrement decay_time. When decay_time reaches zero, calls objectDecay virtual method. Default objectDecay relocates contents then returns DELETE_THIS.

The TFood objectDecay checks if already FOOD_SPOILED. If not spoiled, adds FOOD_SPOILED flag and sets decay_time to volume times 10 for extended decay. Returns TRUE to prevent deletion. If already spoiled, returns FALSE to allow normal deletion.

The TBaseCorpse objectDecay shows decay message to room. Logs player corpse information if TPCorpse. Returns FALSE to allow normal deletion flow which relocates contents.

### Vehicle Movement Execution

The vehiclePulse method calls update_exits to synchronize interior room exits to current vehicle location. Checks speed is not zero. Calculates pulse timing based on speed value to determine if movement should occur this tick. Validates current direction has valid exit. Implements auto-pathing by counting valid exits excluding reverse direction. If only one valid path, automatically sets direction to that exit. For boats, tracks water exit count and prevents leaving water unless temporary excursion. Moves vehicle to new room. Sends movement messages to old room, new room, and interior occupants using vehicle type-specific messaging.

The driveSpeed method validates speed parameter is SLOW_SPEED, MED_SPEED, FAST_SPEED, or zero. Updates speed field. Shows speed change message to driver.

The driveDir method validates direction parameter is valid dirTypeT. Updates dir field. Shows direction change message to driver. Next vehiclePulse will use new direction.

### Book Content Loading

The lookAtObj method in TBook parses argument for section number. Constructs filename from objdata/books/ directory using object vnum. If section specified, appends .section to filename. If character hasColorVt returns true, appends .ansi to filename and tries to load. Falls back to plain text if ANSI version not found. Opens file and reads content. If character has CLIENT_NOTE, sends via client note system. Otherwise uses page_string for paged display. Shows end of section marker after content.

### Bed Regeneration Bonus

The bedRegen method calculates base bonus as regen times gain divided by 100 for percentage increase. Checks if character height exceeds max_size field. If too tall, reduces gain and shows discomfort message about cramped conditions. Ensures gain never goes negative by taking maximum of zero and calculated value. Returns modified gain value.

The bed position system validates character position against min_pos_use. Sleeping requires min_pos_use 0, resting requires 0 or 1, sitting requires 0 through 2. Calculates space consumption as 3 for sleep, 2 for rest, 1 for sit. Checks total users against max_users limit.

## Troubleshooting

### Use-After-Free from Ignored Return Values

**Symptom**: Crashes in object methods after spec proc calls or trap triggers. Backtrace shows delete followed by method invocation on freed memory.

**Cause**: Code continues executing after function returns DELETE_ITEM or DELETE_THIS flag. Object was deleted by called function but caller ignores return value.

**Fix**: Check IS_SET_DELETE after every dangerous call. Return immediately or propagate DELETE flag to caller.

```cpp
// WRONG: Ignores return value
int rc = obj->checkSpec(NULL, CMD_GENERIC_PULSE, "", NULL);
obj->doSomethingElse();  // CRASH: obj invalid

// CORRECT: Check and propagate
int rc = obj->checkSpec(NULL, CMD_GENERIC_PULSE, "", NULL);
if (IS_SET_DELETE(rc, DELETE_ITEM))
    return true;
obj->doSomethingElse();  // Safe
```

**Verification**: Search for checkSpec, triggerContTrap, objectDecay, objDamage calls. Ensure every call site checks return value before continuing.

### Container Stuff List Corruption

**Symptom**: Crashes when iterating container contents. Stuff list contains invalid pointers or iteration skips items.

**Cause**: Deleting object without removing from stuff first, or modifying stuff during iteration with pre-increment.

**Fix**: Always use operator-- to remove before deleting. Use post-increment iteration pattern when modifying container during loop.

```cpp
// WRONG: Deletes without removing
TObj* item = container->stuff.front();
delete item;  // Corrupts stuff list

// CORRECT: Remove then delete
TObj* item = container->stuff.front();
--(*item);
delete item;

// WRONG: Pre-increment invalidates iterator
for (auto it = stuff.begin(); it != stuff.end(); ++it) {
    --(*(*it));  // Invalidates it
}

// CORRECT: Post-increment before modification
for (auto it = stuff.begin(); it != stuff.end();) {
    TThing* t = *(it++);
    --(*t);
}
```

**Verification**: Verify all container deletions use extraction pattern. Audit all stuff iterations for post-increment when removing items.

### Invalid Parent Pointers After Movement

**Symptom**: Assertions fire about parent not matching container. Weight calculations give wrong results. Objects appear in multiple locations.

**Cause**: Setting parent directly without updating stuff lists. Using operator+= on object that still has parent set.

**Fix**: Use operator-- to clear all location pointers before operator+= to new location. Never manipulate parent directly.

```cpp
// WRONG: Doesn't clear old location
item->parent = newContainer;
newContainer->stuff.push_back(item);

// CORRECT: Use operators
--(*item);            // Clears parent and removes from old stuff
*newContainer += *item;  // Sets parent and adds to new stuff
```

**Verification**: Search for direct parent assignments. Ensure all object movement uses operator-- followed by operator+=.

### Weight Calculation Infinite Loop

**Symptom**: getTotalWeight hangs or crashes with stack overflow. Weight values are impossibly large.

**Cause**: Circular container nesting created through incorrect pointer manipulation.

**Fix**: Container nesting restriction prevents non-empty containers from holding other containers. Enforce this in putSomethingInto.

```cpp
// In TBaseContainer::putSomethingIntoContainer
if (!stuff.empty()) {
    act("Containers can't hold other containers unless they're empty.",
        FALSE, ch, cont, this, TO_CHAR);
    return FALSE;
}
```

**Verification**: Check putSomethingInto implementations enforce emptiness requirement for container nesting.

### Trap Damage Kills Character But Code Continues

**Symptom**: Crashes after trap triggers. Character uses skills or moves after death. Backtrace shows operations on deleted TBeing.

**Cause**: triggerContTrap returns DELETE_THIS but caller doesn't check. Code continues executing after character death.

**Fix**: Check IS_SET_DELETE for DELETE_THIS after every triggerContTrap call. Fire traps require two checks for objDamage and flameEngulfed.

```cpp
// WRONG: Ignores delete
rc = triggerContTrap(container);
doMoreStuff();  // CRASH

// CORRECT: Check and return
rc = triggerContTrap(container);
if (IS_SET_DELETE(rc, DELETE_THIS))
    return DELETE_THIS;
doMoreStuff();  // Safe
```

**Verification**: Search for triggerContTrap calls. Verify every call site checks return value and propagates DELETE_THIS.

### Pick Lock Jams Permanently

**Symptom**: Lock becomes jammed and cannot be unlocked by key or further picks. Only immortal admin can fix.

**Cause**: Critical failure on pick lock attempt adds CONT_JAMMED flag. This is intentional game mechanic not a bug.

**Resolution**: Have locksmith repair the container to remove jam, or use immortal admin command to clear flag.

### Ghost Trap Won't Disarm

**Symptom**: Trap shows during examine but disarm skill always fails. Trap damage seems inconsistent.

**Cause**: Confusion between CONT_GHOSTTRAP and real traps. Ghost traps are intentional false positives from failed detect trap skill.

**Resolution**: Ghost traps always succeed on first disarm attempt. Check for CONT_GHOSTTRAP flag in isContainerFlag. Real trap disarm can fail and trigger the trap.

### Food Never Spoils or Spoils Immediately

**Symptom**: Food decay behavior is inconsistent. Some food disappears without going through spoiled state.

**Cause**: Misunderstanding of two-stage decay. Food first transitions to FOOD_SPOILED with extended timer, then later deletes. If initial decay_time was zero, food might skip spoiled transition.

**Resolution**: Set appropriate decay_time during food creation based on desired spoilage timeline. Extended decay is volume times 10 after spoilage.

### Drink Container Weight Fluctuates

**Symptom**: Container weight changes unexpectedly. Sometimes weight increases after drinking.

**Cause**: Not accounting for SIP_WEIGHT constant. Each drink unit adds 0.065 pounds to container weight.

**Resolution**: When modifying drink units, call weightChangeObject with amount times SIP_WEIGHT to maintain accurate weight. DRINK_PERM containers skip weight changes.

### Light Burns Forever or Dies Immediately

**Symptom**: Light never consumes fuel or burns out instantly after lighting.

**Cause**: Misunderstanding of curBurn and maxBurn relationship. Negative maxBurn indicates non-refuelable perpetual light.

**Resolution**: Set maxBurn to positive value for consumable lights. Set curBurn to desired initial fuel level. Light decays by 1 per tick when lit. TFFlame with TFFLAME_IMMORTAL flag never decays.

### Vehicle Stuck or Won't Turn

**Symptom**: Vehicle refuses to change direction or move. Speed changes don't affect movement.

**Cause**: Not understanding pulse timing or auto-pathing behavior. Movement occurs at intervals based on speed. Auto-pathing overrides manual direction when only one valid exit.

**Resolution**: Check speed is not zero. Verify direction has valid exit in current room. Auto-pathing prevents manual turns at single-path intersections. Set whole_zone flag if vehicle should update all zone exits.

### Bed Regeneration Not Working

**Symptom**: Character resting on bed gets same regeneration as floor. Regen bonus not applied.

**Cause**: Not calling bedRegen from regeneration calculation code, or character too tall for bed triggering size penalty.

**Resolution**: Verify hitLimit or manaLimit calls bedRegen when character riding furniture. Check character height against bed max_size. Size penalty reduces or eliminates bonus for oversized users.
