---
title: Class Hierarchy
category: critical
keywords: [TThing, TBeing, TMonster, TPerson, TRoom, TObj, inheritance, polymorphism, virtual functions, type identification, object factory, opinionData, charList]
related: [character-foundation.md, memory-safety.md]
primary_symbols:
  functions: [getKind, itemType, makeNewObj, isTMonster, isTPerson, toTMonster, toTPerson, addHated, remHated, addFeared, remFeared, Hates, Fears, DeleteHatreds, DeleteFears]
  classes: [TThing, TBeing, TMonster, TPerson, TRoom, TObj, opinionData, charList, TBaseWeapon, TGenWeapon, TBaseClothing, TArmor, TBaseContainer, TOpenContainer, TBaseCorpse, TBaseCup, TBaseLight, TMagicItem, TMergeable, TSeeThru, TFood]
  files: [code/code/misc/thing.h, code/code/misc/being.h, code/code/misc/monster.h, code/code/misc/person.h, code/code/misc/obj.h, code/code/misc/room.h, code/code/misc/db.cc, code/code/misc/opinion.cc]
---

## Overview

All game world entities descend from TThing, the polymorphic root representing anything that can exist in the world. The hierarchy splits into three main branches: TRoom for spatial locations, TBeing for living creatures that subdivides into TMonster (NPCs) and TPerson (players), and TObj for items with over forty specialized subclasses. This structure enables polymorphic containment where rooms hold beings and objects, beings carry equipment and inventory, and containers nest arbitrarily deep.

The type system provides identification at multiple granularities. TThing defines getKind returning TThingKind enumeration for broad categorization. TObj subclasses implement itemType returning one of sixty-seven item type constants for fine-grained identification. TBeing provides isTMonster and isTPerson predicates with corresponding toTMonster and toTPerson safe conversion functions wrapping dynamic_cast.

Virtual dispatch enables polymorphic behavior through over one hundred seventy virtual functions defined on TThing. Subclasses override these to provide type-specific implementations while allowing generic code to operate on base pointers. The codebase relies heavily on this polymorphism rather than explicit type switching.

Memory ownership follows the DELETE flag protocol where functions signal object deletion through return values. The caller who resolved or created a pointer owns it and must delete it unless returning a DELETE flag to propagate ownership upward. Container operators like += and -- maintain bidirectional parent-child relationships with automatic cleanup.

## Patterns

### Type Identification Strategy

Use isTMonster and isTPerson for TBeing type checks rather than dynamic_cast nullptr checks. These methods provide cleaner syntax and self-documenting intent. Follow with toTMonster or toTPerson for safe conversion when accessing subclass-specific members.

Check TObj subclass types using itemType equality comparisons against itemTypeT constants. This provides faster dispatch than dynamic_cast and matches the codebase convention. Reserve dynamic_cast for situations requiring actual pointer conversion to access subclass methods.

Apply getKind for broad categorization when distinguishing between room, being, and object categories. This enum class approach provides type safety over the legacy thingTypeT integer enumeration still found in older code paths.

### Container Management

Add entities to containers using the += operator which establishes bidirectional relationships. The left operand becomes the parent and updates its stuff list. The right operand updates its parent or roomp pointer depending on type. This pattern maintains spatial consistency automatically.

Remove entities using the -- operator before manual deletion. This clears all location pointers and removes the entity from parent containers. Deleting without removal violates spatial invariants and causes dangling pointers in container lists.

Never assume container operations succeed silently. The operators perform validation and may trigger side effects like stat modification when equipping items. Check return values from functions that modify containment during their execution.

### Virtual Function Override

Override virtual functions to specialize behavior for subclasses. Most behavior differentiation happens through virtual dispatch rather than type switching. Functions like hitGain and manaGain have TBeing pure virtual declarations requiring TMonster and TPerson to provide implementations.

Mark overrides with the override keyword when touching virtual functions. The legacy codebase omits this on ninety-nine percent of overrides creating fragility when base signatures change. Adding override provides compile-time verification that the function actually overrides a base declaration.

Understand pure virtual functions require implementation in concrete classes. TThing declares exitDir pure virtual since only TRoom provides meaningful exit data. Attempting to instantiate a class with unimplemented pure virtuals causes compilation failure.

### Opinion System Management

Clean up charList linked lists manually before deleting opinionData structures. The opinionData destructor only deletes the head node of clist. The charList destructor does not recursively delete next pointers. Iterate the list caching next before deleting each node to avoid use-after-free.

Use DeleteHatreds and DeleteFears global functions when removing beings from the world. These iterate all monsters removing the target from hate and fear lists preventing dangling pointers. Failing to call these before deletion leaves stale character pointers that crash on subsequent hate checking.

Prefer addHated and addFeared for individual targeting over addHatred and addFears for categorical targeting. Individual functions create charList entries with account and player tracking for multi-play detection. Categorical functions set race, sex, class, or vnum filters checked during findAHatee and findAFearee room scanning.

### Object Factory Usage

Create objects from prototypes using read_object passing virtual numbers from obj_index. This loads configured values and applies zone-specific modifications. Direct construction with makeNewObj bypasses prototype data requiring manual initialization of all fields.

Instantiate objects without prototypes using makeNewObj passing the desired itemTypeT. This factory function switches on type returning the appropriate subclass instance. Cast the returned TObj pointer to the specific subclass using dynamic_cast before accessing subclass methods.

Validate itemTypeT values before passing to makeNewObj. The function assumes valid enumeration constants and undefined behavior results from arbitrary integers. Check against MAX_OBJ_TYPES when processing untrusted input like builder commands.

## Reference

### TThing Base Class

Defined in thing.h as the polymorphic root for all world entities. Provides identity through name, shortDescr, and descr strings. Maintains location via parent pointer for container relationships and roomp for spatial positioning. Tracks physical properties including weight, height, and material_type affecting interactions.

Declares stuff list head for contained entities enabling arbitrary nesting of objects and beings. Implements mounting relationships through rider and riding pointers supporting mount systems. Defines over one hundred seventy virtual functions providing default implementations overridden by subclasses.

Contains pure virtual exitDir requiring subclass implementation. Only TRoom provides meaningful exit data making this function abstract at the TThing level. Most other virtuals provide base implementations allowing subclasses to selectively override behavior.

Implements container operators += for addition and -- for removal. These maintain bidirectional relationships updating parent pointers and room references automatically. The operators handle both TBeing and TObj entity types through polymorphic dispatch.

Provides visibility and perception functions including canSee, canSeeMe, and listThingRoomMe. These account for light levels, blindness, invisibility, and other visibility-affecting conditions. Subclasses override to add type-specific visibility rules.

### TBeing Living Entity Class

Defined in being.h extending TThing for creatures with statistics and actions. Stores combat attributes in points structure containing HP, mana, move, and derived values like max_hit and armor. Maintains equipment array with WEAR_MAX slots corresponding to body positions like WEAR_HEAD and WEAR_BODY.

Tracks active spell effects through affected linked list of affectedData structures. Each entry contains type, duration, modifier, and location defining temporary stat changes. The affectJoin and affectModify functions manipulate this list validating compatibility.

Contains combat state in specials structure including position, fighting target, and action timers. Position ranges from POSITION_DEAD through POSITION_STANDING affecting available actions. The fighting pointer identifies current combat opponent enabling automatic combat continuation.

Manages group relationships via master pointer and followers list. The AFF_GROUP flag gates XP distribution requiring both master linkage and explicit flag. Functions like inGroup validate full group membership rather than checking master alone.

Declares pure virtuals requiring subclass implementation including hitGain for HP regeneration, manaGain for mana recovery, and various learning, persistence, and timer functions. This forces TMonster and TPerson to provide appropriate mechanics for NPCs versus players.

Provides type identification helpers isTMonster and isTPerson checking dynamic_cast results. Corresponding toTMonster and toTPerson functions perform safe conversion returning nullptr for wrong types. These wrap the verbose dynamic_cast syntax improving readability.

### TMonster NPC Class

Defined in monster.h extending TBeing for computer-controlled creatures. Stores personality in opinion structure containing greed, anger, malice, and suspicion values affecting merchant pricing and combat behavior. Maintains hates and fears opinionData structures targeting by character, sex, race, class, or vnum.

Tracks response scripts through resps array of responseData pointers. These define triggered actions like speech responses, combat assists, and special abilities activated by game events. Response checking happens during mobileActivity ticking.

Contains scaling factors hpLevel, damLevel, and acLevel modifying base statistics. Zone files set these multipliers allowing instance-specific difficulty tuning without duplicating mob definitions. Mob loading applies these factors during instantiation.

Implements AI behaviors through mobileActivity called each pulse. This handles wandering, aggression checking via aggro, hunting through hunt function, and scavenging. The activity respects position, fighting state, and charmed status.

Provides social response functions for emotes and player interactions. Over one hundred handler functions like doSocialCringe and doSocialSmile generate mob reactions to social commands. These create immersive NPC personalities through behavioral variety.

### TPerson Player Class

Defined in person.h extending TBeing for player-controlled characters. Stores player-specific data including title displayed after name and wizPowers array granting immortal abilities. Maintains account linkage through account_id enabling multi-character tracking.

Implements persistence through saveRent writing character state to filesystem and loadRent restoring on connection. These serialize inventory, equipment, affects, stats, and location enabling character continuity across sessions. Database operations use both filesystem and SQL storage.

Handles level advancement via advanceLevel applying class-specific stat gains and ability unlocking. This checks experience thresholds, validates class requirements, and triggers gain notifications. The function updates max HP, mana, move, and combat statistics.

Provides learning progression through learnFromDoing called after successful skill and spell usage. This awards practice gains based on difficulty, failure rate, and current proficiency. Learning rates vary by class and specialization matching design intent.

Contains wizFileSave and doQuit2 implementing immortal persistence separate from player rent files. Wizard data includes extra powers, implementation access flags, and configuration settings requiring isolated storage preventing corruption from normal player operations.

### TObj Item Base Class

Defined in obj.h extending TThing for portable game objects. Stores configuration in obj_flags containing extra flags like ITEM_GLOW, wear flags like ITEM_WEAR_BODY, and economic data like cost. Maintains ownership tracking through owners string recording creator and significant possessors.

Declares affected array with MAX_OBJ_AFFECT entries defining stat modifications when equipped. Each entry specifies location like APPLY_HITROLL and modifier magnitude. Equipment functions apply these affects calling affectModify on equip and unequip.

Contains pure virtual declarations forcing subclass implementation. The itemType function returns itemTypeT constant identifying subclass type. Functions assignFourValues and getFourValues provide serialization for type-specific data stored in obj_flags.value array.

Implements type-specific behavior through subclass overrides of virtual functions. Different item types provide specialized implementations of lowCheck for condition tracking, updateDesc for state-dependent descriptions, and objVnum for prototype identification.

### TObj Subclass Hierarchy

TBaseWeapon defines melee and ranged weapons subdividing into TGenWeapon for standard weapons and TArrow for ammunition. TGun extends TGenWeapon adding firearm mechanics with THandgonne and TCannon for specific weapon types. Weapons store damage dice and type in value array.

TBaseClothing covers worn items splitting into TArmor for protective gear and TWorn for non-protective clothing. TJewelry, TSaddle, and THarness provide specialized equipment types. TArmor combines with TWand through multiple inheritance creating TArmorWand for protective magical items.

TBaseContainer handles items containing other items with TOpenContainer for accessible storage. TChest, TCookware, and TWagon extend this for specific container types. TExpandableContainer adds variable capacity with subclasses TQuiver, TKeyring, TMoneypouch, TSaddlebag, TSuitcase, TCardDeck, TPlant, TTrashPile, and TToothNecklace implementing different growth mechanics.

TBaseCorpse represents creature remains with TCorpse for mob corpses and TPCorpse for player corpses. These track origin creature, death time, and decay state. Special handling prevents player corpse loss while mob corpses provide loot containers.

TBaseCup contains liquids splitting into TDrinkCon for refillable containers, TPotion for consumable magic, TVial for components, TPoison for toxic substances, and TPool for ground liquids. Value array tracks liquid type, capacity, and remaining quantity.

TBaseLight provides illumination with TLight for reusable sources and TFFlame for consumable flames. Light radius and duration affect visibility and dark navigation. Exhausted lights revert to non-functional state.

TMagicItem covers magical objects with TScroll for readable spells, TWand for targeted effects, and TStaff for area effects. Charges track remaining uses. Spell types and levels configure magical properties.

TMergeable enables quantity stacking with TComponent for crafting materials, TMoney for currency, TCommodity for trade goods, and TGas for atmospheric effects. Merging combines quantities when identical items interact.

TSeeThru allows vision through objects with TPortal enabling teleportation and TVehicle for mobile containers. TWindow provides one-way or two-way visibility without passage.

TFood represents edible items with TEgg and TFruit as specialized subtypes. Nutrition values, poison flags, and decay times configure food properties.

Direct TObj subclasses include TBandage, TBed, TBoard, TBoat, TBook, TBow, TCasinoChip, TDrug, TDrugContainer, TFuel, TGemstone, TKey, TNote, TOpal, TOrganic, TOtherObj, TPen, TStatue, TSymbol, TTable, TTrash, TTrap, TTree, TTreasure, TAudio, TTool, and TASubstance. Each implements specialized behavior through virtual function overrides and type-specific value array usage.

### TRoom Spatial Location Class

Defined in room.h extending TThing for world locations. Stores environment in sectorType determining terrain like SECT_CITY, SECT_FOREST, and SECT_WATER_SWIM affecting movement costs and drowning. Contains dir_option array with six directional exit structures holding destination, keywords, and door flags.

Maintains zone assignment identifying the zone containing this room for reset and administrative purposes. Tracks roomFlags including ROOM_DEATH for death traps, ROOM_INDOORS for weather protection, and ROOM_PEACEFUL preventing combat.

Implements river flow through riverDir and riverSpeed controlling current direction and strength. These affect swimming difficulty and automatic movement for floating. River systems create environmental hazards requiring navigation skill.

Contains teleportation mechanics via teleTarg destination and teleTime delay. Rooms can transport occupants automatically or on triggers creating transportation networks and puzzle elements.

Provides born list operators << for adding mobs to room respawn tracking and >> for removal. This tracks which mob prototypes appear in rooms during zone resets enabling proper repopulation.

### Type Identification Enumerations

TThingKind enum class in thing.h provides type-safe broad categorization with values TThing, TBeing, TMonster, TPerson, TRoom, TObj, TComponent, and TBaseContainer. Access through getKind virtual function returns appropriate constant for polymorphic type identification.

itemTypeT enumeration in obj.h defines sixty-seven object type constants including ITEM_LIGHT, ITEM_SCROLL, ITEM_WAND, ITEM_WEAPON, ITEM_ARMOR, ITEM_POTION, ITEM_TRAP, ITEM_DRINKCON, ITEM_FOOD, ITEM_MONEY, ITEM_BAG, ITEM_CORPSE, ITEM_COMPONENT, and ITEM_PORTAL. Subclasses return constants matching their implementation class.

thingTypeT enumeration in thing.h provides legacy integer-based categorization with TYPETHING, TYPEOBJ, TYPEMOB, TYPEPC, TYPEROOM, and TYPEBEING. Older code uses this but new development should prefer TThingKind for type safety.

### Opinion System Structures

opinionData structure in monster.h contains clist charList pointer for individual target tracking, sex for sexTypeT filtering, race for race_t filtering, Class for class bitmask filtering, and vnum for prototype filtering. Bitmasks hatefield and fearfield track which filter types are active using HATE_CHAR, HATE_SEX, HATE_RACE, HATE_CLASS, HATE_VNUM and corresponding FEAR constants.

charList structure in monster.h implements linked list nodes with name string for character identification, iHateStrength duration counter in game hours, account_id for multi-play detection, player_id for database linkage, and next pointer for list continuation. These nodes require manual iteration and deletion avoiding automatic recursive cleanup.

### Global Entity Lists

character_list maintains linked list of all TBeing instances using next pointer threading. Iteration requires caching next before operations that might delete beings. Combat and cleanup code walks this list applying effects and validating state.

object_list holds TObjList containing all TObj instances. Object iteration and searching traverse this structure. Cleanup walks the list during shutdown and zone resets.

room_db array provides direct indexing of TRoom instances by virtual number. Index calculations use WORLD_SIZE defining maximum vnum range. Invalid vnums access nullptr requiring validation through real_roomp checks.

mob_index vector contains mobIndexData prototype structures for TMonster creation. Entries store base stats, flags, and configuration loaded from zone files. read_mobile function instantiates from these prototypes.

obj_index vector holds objIndexData prototype structures for TObj creation. Entries configure item properties, values, and flags. read_object function creates instances applying prototype data.

### Opinion System Functions

addHated in opinion.cc creates charList entry for target being with hate duration calculated from combat state and damage dealt. Updates hatefield setting HATE_CHAR flag. Links node into opinionData clist requiring manual cleanup later.

remHated in opinion.cc searches clist for matching character name and removes node from list. Critical memory leak: does not delete removed node unlike remFeared. Successful removal clears HATE_CHAR when list becomes empty.

Hates in opinion.cc checks whether mob hates target by testing hatefield flags and comparing against sex, race, class, vnum, and clist entries. Returns boolean indicating hate match. Used by aggro and combat targeting systems.

addFeared in opinion.cc creates charList entry for feared target with duration tracking. Updates fearfield setting FEAR_CHAR flag. Manages list insertion requiring subsequent manual cleanup.

remFeared in opinion.cc searches clist for character name, removes from list, and correctly deletes node freeing memory. Clears FEAR_CHAR flag when list empties. Proper cleanup pattern unlike remHated.

Fears in opinion.cc validates fear targeting by checking fearfield and comparing target attributes. Returns boolean for fear match. Influences mob AI decisions and flee behavior.

addHatred in opinion.cc sets categorical hate by race, sex, class, or vnum. Updates corresponding hatefield flags enabling filtered targeting. Used for zone-wide faction hatred configuration.

addFears in opinion.cc configures categorical fear filters parallel to addHatred. Sets fearfield flags for attribute-based targeting. Enables cowardly mob behavior patterns.

findAHatee in opinion.cc scans room occupants checking Hates predicate returning first matching target. Enables aggro systems to identify attackable characters. Returns nullptr when no valid targets exist.

findAFearee in opinion.cc searches room for feared targets returning first match. Supports flee behavior and social AI responses. Complements findAHatee for complete NPC emotion targeting.

developHatred in opinion.cc conditionally adds hate based on combat engagement, damage history, and random chance. Creates dynamic faction development through gameplay. Adjusts hate strength by mob personality factors.

DeleteHatreds global function in opinion.cc iterates character_list examining all TMonster hate lists removing entries matching target. Prevents dangling pointers when beings delete. Call before character deletion.

DeleteFears global function in opinion.cc walks character_list removing target from all mob fear lists. Complements DeleteHatreds for complete cleanup. Required before being destruction.

### Object Factory Function

makeNewObj in db.cc implements factory pattern switching on itemTypeT parameter. Returns appropriate TObj subclass instance for requested type. Handles all sixty-seven item types mapping each constant to constructor call.

Validation checks itemTypeT against MAX_OBJ_TYPES preventing undefined behavior from invalid values. Returns TOtherObj for unrecognized types providing safe fallback. Builder code validates before calling.

Usage requires dynamic_cast to access subclass methods since return type is TObj pointer. Pattern involves factory call followed by type-specific pointer conversion and initialization.

## Implementation

### Virtual Function Dispatch Mechanism

The compiler implements virtual functions through vtable pointer stored as hidden first member of each polymorphic object. TThing and all descendants contain this pointer referencing class-specific vtable array. Function calls through base pointers indirect through vtable enabling runtime type resolution.

Construction initializes vtable pointer to derived class vtable. The compiler generates one vtable per polymorphic class containing function pointers for all virtual functions. Override resolution happens at compile time populating vtable entries with most-derived implementations.

Pure virtual functions have no vtable entry in abstract classes preventing instantiation. Concrete derived classes must provide implementations populating vtable entries. Attempting to instantiate abstract classes causes linker errors for undefined pure virtual functions.

The override keyword provides compile-time verification that function signature matches base declaration. Without override, signature mismatches create new virtual functions rather than overriding existing ones. The codebase lacks override on most virtuals creating fragility.

### Container Operator Implementation

The += operator in TThing checks destination type dispatching to appropriate addition method. For TRoom destinations, calls thingToRoom establishing room occupancy. For container additions, updates parent pointer and inserts into stuff list. Equipment additions call equip validating slot availability.

Bidirectional relationship maintenance happens automatically. Adding entity to container sets entity parent pointer and container stuff list entry. Container tracks children enabling recursive operations like aggregate weight calculation. Removal must update both sides.

The -- operator reverses addition clearing all location pointers. For equipped items, calls unequip removing from equipment array and calling affectModify. For inventory, clears parent pointer and removes from stuff list. For rooms, clears roomp and removes from room contents.

Affect modification during equipment operations applies item bonuses to character stats. Each item affected entry modifies APPLY_HITROLL, APPLY_DAMROLL, APPLY_ARMOR, or other stats. Equipment changes require affect recalculation updating derived values like armor class.

### Type Identification Implementation

getKind implementation in each class returns appropriate TThingKind constant. TThing base returns TThingKind::TThing. TMonster returns TThingKind::TMonster. Compiler dispatches to correct implementation through vtable enabling polymorphic type queries without dynamic_cast overhead.

itemType pure virtual forces TObj subclasses to return correct itemTypeT constant. Each subclass hardcodes appropriate value like ITEM_WEAPON for TGenWeapon. Factory function makeNewObj reverses this mapping constructing objects from item type constants.

isTMonster and isTPerson perform dynamic_cast returning boolean result. Implementation casts this pointer to target type checking result against nullptr. Non-null indicates successful cast meaning object is requested type. Null means wrong type.

toTMonster and toTPerson wrap dynamic_cast returning converted pointer. These provide cleaner syntax than explicit casts and communicate safe conversion intent. Callers must check nullptr result before dereferencing since cast may fail for wrong types.

### Opinion System Memory Layout

opinionData structure contains inline hatefield and fearfield bitmasks, primitive sex, race, Class, and vnum members, plus pointer to charList head. Memory layout places frequently-accessed bitmasks first enabling cache-efficient hate checking without pointer chasing.

charList nodes allocate name string with new[] requiring delete[] in destructor. The next pointer links to following node or nullptr for list end. Nodes connect linearly requiring sequential traversal for searches.

Hate and fear lists attach to TMonster instances via hates and fears members. Each mob maintains independent opinion state allowing individualized targeting. Global DeleteHatreds and DeleteFears functions must walk all mobs updating lists.

List modification updates hatefield and fearfield flags reflecting active filter types. HATE_CHAR sets when clist non-empty. HATE_SEX sets when sex filter active. Checking tests bitmask before comparing target attributes avoiding unnecessary work.

### Object Factory Switch Implementation

makeNewObj contains switch statement with cases for each itemTypeT value. Each case constructs appropriate TObj subclass returning pointer cast to TObj base type. Switch covers all item types providing complete factory coverage.

Default case handles ITEM_UNDEFINED and unrecognized types returning new TOtherObj. This prevents crashes from invalid item types providing safe fallback. Builders receive generic object allowing continued operation.

Factory supports fifty-plus specialized classes including weapon subtypes, container variants, consumables, and equipment pieces. Complex items like TArmorWand combining armor and wand behavior require multiple inheritance constructing both base classes.

Return type TObj requires callers to dynamic_cast for subclass access. Pattern involves type checking through itemType equality test followed by cast to specific class. Incorrect casts return nullptr enabling graceful failure.

### Global List Management

character_list maintains single-linked list via TBeing next pointer. List insertion happens during being construction or load. Removal requires walking list finding predecessor and updating next pointer. Head pointer stored as global variable.

Iteration caches next pointer before operations potentially deleting current being. Combat and cleanup code uses this pattern preventing iterator invalidation. Failure to cache causes use-after-free when deleted being's next pointer becomes inaccessible.

object_list uses TObjList custom container managing TObj instances. Implementation details hidden in class providing add, remove, and iteration interfaces. Object construction registers with list and destruction unregisters automatically.

room_db array pre-allocates WORLD_SIZE entries during initialization. Zone loading populates entries creating TRoom instances. Invalid vnums access null pointers requiring validation. real_roomp helper checks bounds and null returning nullptr for invalid values.

mob_index and obj_index vectors grow dynamically during zone file loading. Each entry contains prototype data including stats, flags, and configuration. Instantiation functions copy prototype data to new instances applying zone-specific modifications.

### Polymorphic Cleanup Challenges

TThing destructor must handle contained objects walking stuff list and deleting children. Recursive deletion propagates through containment hierarchy ensuring no orphaned objects remain. Failure to delete children causes memory leaks.

TMonster destructor must clean up opinion data iterating charList and deleting nodes. Base opinionData destructor only deletes head requiring manual list traversal. Missing cleanup leaks memory accumulating over mob lifecycle.

Equipment and inventory require careful removal before deletion. Deleting equipped items without unequip leaves stale pointers in equipment array. Container contents need extraction before container deletion preventing double-free.

Spatial relationships require bidirectional cleanup. Removing being from room must update both roomp pointer and room contents list. Incomplete cleanup leaves dangling pointers causing crashes during iteration.

## Troubleshooting

### Missing Override Keyword Symptoms

Signature mismatch between base and derived virtual functions creates new function instead of override. Compiler generates vtable entry for base version leaving it unoverridden. Calls through base pointers invoke base implementation ignoring derived version. Add override keyword causing compilation failure with descriptive error message identifying signature mismatch.

Function works when called directly on derived object but fails through base pointer. This indicates override failure with direct calls resolving statically while pointer calls use vtable. Compare function signatures between base and derived checking const, parameter types, and return type. Mismatch prevents override.

Changing base virtual signature breaks no derived classes. Without override keywords compiler silently creates new functions in derived classes instead of reporting signature mismatches. Large-scale virtual changes require manual audit of all overrides. Modernization effort should add override throughout enabling compiler verification.

### Opinion List Memory Leaks

Gradual memory growth correlating with mob hate development indicates charList leak. remHated removes nodes from list without deleting them. Hate system adds nodes during combat but removal never frees memory. Replace remHated calls with remFeared pattern manually deleting removed nodes.

Valgrind reports leaked charList nodes with allocation traces pointing to addHated. Destructor cleanup only handles head node leaving chain allocated. Add manual list walking before opinionData deletion iterating and freeing all nodes. Pattern requires caching next before delete avoiding use-after-free.

Mob deletion causes spike in leaked memory proportional to hate list length. TMonster destructor incomplete cleanup allows node leakage. Verify opinionData destruction includes full list traversal deleting each charList node before clist pointer cleared.

### Container Operator Precondition Violations

Adding object with non-null parent pointer causes spatial inconsistency. Object appears in multiple locations with container lists containing duplicate entries. Check that --(*object) preceded addition clearing all location pointers. Objects must be orphaned before re-parenting.

Equipment operation crashes accessing invalid item pointer. Item deleted while still equipped leaving stale equipment array entry. Unequip before deletion or verify DELETE_ITEM flag handling. Equipped items require special deletion protocol.

Room transition fails with assertion about existing roomp. Movement code attempts addition without clearing previous room pointer. Call --(*being) or explicit room removal before adding to new room. Spatial invariants require clean transitions.

### Type Identification Errors

dynamic_cast returns nullptr but code dereferences anyway. Not all TBeing instances are TMonster or TPerson making unconditional cast unsafe. Check toTMonster and toTPerson return values before use. Null indicates wrong type requiring graceful handling.

itemType equality check passes but dynamic_cast to specific class fails. Multiple subclasses return same itemTypeT constant like ITEM_WORN for different worn item types. Use dynamic_cast for specific class identification rather than itemType alone when subtype matters.

getKind returns unexpected TThingKind value. Virtual function dispatch through corrupted vtable pointer from use-after-free or buffer overflow. Validate pointer before getKind call checking against known valid ranges. Memory corruption requires sanitizer investigation.

### Factory Function Issues

makeNewObj returns TOtherObj for valid item type. Switch statement lacks case for itemTypeT value falling through to default. Add missing case constructing appropriate subclass. Factory must cover all MAX_OBJ_TYPES values.

Constructed object has wrong itemType return value. Subclass itemType implementation returns incorrect constant disagreeing with factory mapping. Verify itemType override returns value matching factory case. Mismatch breaks type identification.

Factory crashes on invalid itemTypeT parameter. Input validation missing allowing out-of-range enum values. Add bounds check comparing against MAX_OBJ_TYPES before switch. Builder interfaces must validate user input before factory calls.

### Global List Corruption

Iteration crashes with invalid next pointer. List modification during traversal invalidates iterator. Cache next pointer before operations that delete current element using it++ post-increment pattern. Combat uses gCombatNext global for this purpose.

Being appears in character_list but has been deleted. Deletion code skipped list removal leaving stale pointer. Verify being removal before delete using explicit list manipulation or relying on destructor list unregister. All deletion paths must maintain list consistency.

Room access through room_db crashes with null pointer. Invalid vnum or unloaded zone causes null entry. Validate vnums through real_roomp checking return for nullptr before dereference. Builder tools must enforce valid vnum ranges during creation.

### Polymorphic Deletion Issues

Base class destructor called but derived class members not cleaned up. Destructor declared non-virtual allowing base delete on derived object to skip derived destructor. Verify TThing and all base classes use virtual destructors. Non-virtual destructors cause resource leaks.

Deleting object triggers deletion of unrelated objects. Bidirectional pointers with unclear ownership cause cascading deletes. Verify DELETE flag protocol following ownership rules. Caller who resolved pointer owns deletion unless returning DELETE flag.

Object deleted but still referenced by other objects. Weak pointer invalidation requires manual notification. Call DeleteHatreds and DeleteFears for beings. Clear equipment and inventory before deletion. Spatial removal must precede object destruction.
