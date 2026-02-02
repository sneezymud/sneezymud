---
title: Magic Items and Enchantment System
category: important
keywords: [TMagicItem, TScroll, TWand, TStaff, reciteMe, useMe, doObjSpell, magic_level, charges, DELETE_VICT, scrolls, wands, staves]
related: [spell-skill-framework.md, object-system.md, affects-system.md, equipment-wear.md, memory-safety.md]
primary_symbols:
  functions: [reciteMe, useMe, doObjSpell, equipChar, unequip, affectModify, generic_find]
  classes: [TMagicItem, TScroll, TWand, TStaff, affectedData]
  files: [code/obj/obj_magic_item.cc, code/obj/obj_scroll.cc, code/obj/obj_wand.cc, code/obj/obj_staff.cc, code/misc/other.cc, code/sys/handler.cc]
---

## Overview

The magic item system allows non-casters to use magic through three specialized object types: scrolls, wands, and staves. Each stores spells in physical form and executes them without consuming mana, piety, or lifeforce. Scrolls are single-use consumables that cast up to three spells sequentially. Wands are rechargeable items that target a specific being or object with a single spell. Staves are rechargeable items that affect all valid targets in a room automatically.

All magic items share a common inheritance hierarchy rooted in TMagicItem, which defines magic_level for spell power and magic_learnedness for crafting quality. Spell execution flows through the doObjSpell dispatcher, which routes spell IDs to their implementations while enforcing peaceful room restrictions and propagating DELETE flags. Items can also carry permanent stat bonuses via the affects system, applied automatically when equipped through equipChar and removed through unequip.

The system bridges object persistence and spell casting by storing spells as spellNumT enum values in object value fields. Database conversion through mapFileToSpellnum and mapSpellnumToFile maintains consistency between the file format and runtime representation. Charge management for wands and staves tracks usage and supports recharging, with shop restrictions preventing sale of partially depleted items.

Critical safety requirements include checking DELETE flags after every spell execution, advancing iterators before deletion when affecting multiple targets, validating charge counts before consumption, and using lock protection to prevent item deletion during multi-spell sequences. The IS_SET_DELETE macro must be used for all DELETE flag checks, as standard IS_SET will not detect the combined bit pattern.

## Patterns

### Scroll Usage Pattern

Scrolls consume themselves whether the recitation succeeds or fails. The reciteMe function performs a SKILL_READ_MAGIC check, finds the target through generic_find, then iterates through all three spell slots. Each spell executes through doObjSpell with lock protection enabled to prevent premature scroll deletion. The function tracks the maximum spell lag from all executed spells and applies it once at the end. If any spell kills the victim, remaining spells do not execute. The scroll always returns DELETE_THIS to signal its destruction.

### Wand Targeting Pattern

Wands require explicit targeting by building a bitmask from the spell's TAR_* flags. If the spell supports TAR_CHAR_ROOM, the bitmask includes FIND_CHAR_ROOM. If it supports TAR_OBJ_INV, the bitmask includes FIND_OBJ_INV. The combined bitmask passes to generic_find, which returns the located target. The useMe function rejects TAR_IGNORE spells that cannot be targeted and fails gracefully when charges are depleted. After consuming a charge, it executes the spell and checks all three DELETE flags: DELETE_VICT for victim death, DELETE_THIS for wand destruction, and DELETE_ITEM for target object destruction.

### Staff Area Effect Pattern

Staves automatically target all valid beings in the caster's room except the caster and their group members for violent spells. Area spells marked with TAR_AREA execute once with null victim and target parameters, letting the spell implementation handle room-wide effects. Non-area spells iterate through the room's stuff container, advancing the iterator before spell execution to prevent invalidation when victims die. The loop continues even after DELETE_VICT occurs, ensuring all valid targets receive the spell effect. Group membership checks use the inGroup function to skip allies for violent spells.

### Iterator Safety During Deletion

When iterating containers where spell execution may delete elements, cache the next pointer before calling doObjSpell. The pattern uses TThing* t = *(it++) to advance the iterator in the same statement that dereferences it. This ensures the iterator points to the next valid element before any deletion occurs. After spell execution, check IS_SET_DELETE(rc, DELETE_VICT) and delete the cached pointer, not the dereferenced iterator. This prevents use-after-free when the container's internal structure updates after element removal.

### Charge Validation and Consumption

Before using any charged item, check getCurCharges() > 0 and return failure with an appropriate message if depleted. Only after validation succeeds should addToCurCharges(-1) execute to consume the charge. Never allow curCharges to become negative, as this indicates a logic error. When recharging, compare the target charge count against getMaxCharges() and clamp to the maximum to prevent overcharge. Shop code enforces that wands and staves must have getCurCharges() == getMaxCharges() before accepting them for sale.

### Equipment Affect Application

When equipChar executes, it sets the object's equippedBy pointer to the character and eq_pos to the equipment slot. For each entry in the object's affected array up to MAX_OBJ_AFFECT, it calls affectModify with the apply flag set to TRUE. This adds the modifier value to the character's stat specified by the location field and sets any bitvector flags. When unequip executes, it calls affectModify with the apply flag set to FALSE, subtracting the modifier and clearing the bitvector flags before nulling the equippedBy pointer.

### DELETE Flag Propagation

After every doObjSpell call, immediately check IS_SET_DELETE(rc, DELETE_VICT) and handle victim deletion before any further operations. If the victim is not the caster, delete the victim pointer and set it to null. Check IS_SET_DELETE(rc, DELETE_THIS) and return DELETE_THIS immediately to propagate item destruction to the caller. Check IS_SET_DELETE(rc, DELETE_ITEM) for target object destruction. Never use IS_SET for DELETE flags, as they require the special bit pattern recognized by IS_SET_DELETE. The caller who owns the pointer is responsible for deletion, so return the flag if the pointer was passed as a parameter.

### Lock Protection for Sequential Spells

Before executing multiple spells on the same item, call setLocked(true) to prevent the item from being deleted by side effects of earlier spells. After each spell executes, call setLocked(false) to restore normal deletion behavior. This prevents crashes where spell slot 1 triggers item destruction but the loop continues to access spell slots 2 and 3. The lock applies to the current spell only, so lock and unlock around each individual doObjSpell call rather than locking once at the start of the loop.

### Spell Validation Before Execution

Before calling doObjSpell with a spell ID, verify the_spell >= MIN_SPELL and the_spell < MAX_SKILL to ensure it falls within the valid range. Check that discArray[the_spell] is not null, as null entries indicate invalid or unimplemented spells. For scrolls iterating through spell slots, skip slots where getSpell(i) returns TYPE_UNDEFINED or falls outside the valid range. Log any invalid spell IDs to LOG_BUG with the spell number for debugging.

## Reference

### TMagicItem Class Hierarchy

TMagicItem extends TObj virtually and defines two protected fields: magic_level for enchantment strength ranging from 0 to 250+ and magic_learnedness for crafting quality ranging from 0 to 100. These values pack into val1 using bits 8-15 for magic_level and bits 0-7 for magic_learnedness. Subclasses TScroll, TWand, and TStaff override pure virtual functions descMagicSpells, getNameForShow, divinateMe, suggestedPrice, and statObjInfo to provide type-specific behavior.

### TScroll Value Mapping

Scrolls store three spells in val2, val3, and val4 as file-format spell IDs. The assignFourValues function converts these through mapFileToSpellnum to populate the internal spells array. The val1 field contains the packed magic_level and magic_learnedness. The val0 field remains unused. The getSpell function accepts indices 0 through 2 and returns the corresponding spellNumT value. The setSpell function updates a spell slot directly.

### TWand and TStaff Value Mapping

Both wands and staves use val2 for maxCharges, val3 for curCharges, and val4 for the single stored spell as a file-format ID. The assignFourValues function calls setMaxCharges with val2, addToCurCharges with val3, and setSpell with mapFileToSpellnum(val4). The val1 field contains packed magic_level and magic_learnedness. The val0 field remains unused. The getFourValues function reverses this mapping for persistence.

### spellNumT Enumeration Range

The spellNumT enum starts at TYPE_UNDEFINED = -1 for invalid spells, then MIN_SPELL = SPELL_GUST = 0 for the first valid spell. Spells continue sequentially through all spell definitions until MAX_SPELL = SKILL_SLAM, where skills begin. Skills continue until MAX_SKILL marks the end of the valid range. The discArray global array indexed by spellNumT contains spellInfo pointers for all valid spells, with null entries indicating invalid IDs.

### affectedData Structure Fields

The affectedData structure contains type for the affect spell or skill ID, level for caster level or intensity, duration for remaining ticks with -9 indicating permanent affects, modifier for the primary effect value, modifier2 for secondary effect values, location for where to apply using applyTypeT enum values, bitvector for character AFF_* flags, be for the associated being pointer, and next for linked list chaining.

### Apply Location Types

APPLY_STR through APPLY_CON modify primary stats by adding the modifier value. APPLY_HIT, APPLY_MANA, and APPLY_MOVE modify maximum resource pools. APPLY_HITROLL and APPLY_DAMROLL modify combat effectiveness. APPLY_ARMOR modifies armor class where negative values provide better protection. APPLY_IMMUNITY stores damage type in modifier and resistance amount in modifier2. APPLY_SPELL stores spell ID in modifier and power bonus in modifier2. APPLY_NONE indicates no modification.

### Bitvector Affect Flags

AFF_INVISIBLE makes the character invisible. AFF_DETECT_INVISIBLE allows seeing invisible beings. AFF_SANCTUARY reduces incoming damage. AFF_FLYING enables flight. AFF_INFRAVISION allows seeing in darkness. AFF_WATERBREATH allows underwater breathing. AFF_DETECT_MAGIC reveals magical auras. AFF_SNEAK enables stealthy movement. These flags combine via bitwise OR in the bitvector field.

### DELETE Flag Types

DELETE_THIS signals that the magic item itself should be deleted. DELETE_VICT signals that the spell victim should be deleted due to death. DELETE_ITEM signals that a target object should be deleted due to destruction. These flags use a special bit pattern that requires IS_SET_DELETE for checking rather than standard IS_SET. The flag owner is responsible for deletion, so if the caller passed the pointer as a parameter, return the flag instead of deleting directly.

### Spell Targeting Flags

TAR_IGNORE indicates spells with no targeting requirements. TAR_CHAR_ROOM finds beings in the same room. TAR_OBJ_INV finds objects in inventory. TAR_OBJ_ROOM finds objects in the room. TAR_OBJ_EQUIP finds equipped objects. TAR_AREA indicates room-wide area effect spells. TAR_VIOLENT marks spells that trigger peaceful room restrictions and group member protection. These flags combine in discArray[spell]->targets.

### Generic Find Bitmask

FIND_CHAR_ROOM searches for beings in the room. FIND_OBJ_INV searches for objects in inventory. FIND_OBJ_ROOM searches for objects in the room. FIND_OBJ_EQUIP searches for equipped objects. These flags combine via bitwise OR to build the bitmask passed to generic_find. The function returns nonzero if a match was found and sets the target_being or target_obj output parameters.

### Spell Lag Constants

LAG_0 applies no lag for instant spells. LAG_1 through LAG_5 apply increasing delays from 1.2 seconds to 6.0 seconds per round. Each lag constant maps to a number of combat rounds via combatRound. The discArray[spell]->lag field stores the lag type. The addToWait function applies the lag to prevent immediate follow-up actions.

### Charge Management Functions

getCurCharges returns the current remaining charges. getMaxCharges returns the maximum charges when fully charged. setMaxCharges updates the maximum, rarely used outside initialization. addToCurCharges accepts positive values to recharge or negative values to consume, with the current charge count clamping to the valid range. Shop validation requires getCurCharges() == getMaxCharges() before accepting wands or staves for sale.

## Implementation

### TMagicItem Base Implementation

The TMagicItem constructor initializes magic_level and magic_learnedness to zero. The assignFourValues function unpacks val1 by extracting bits 0-7 for magic_learnedness and bits 8-15 for magic_level. The getFourValues function reverses this by packing the two fields into val1. The getMagicLevel and getMagicLearnedness accessor functions return the unpacked values. The setMagicLevel and setMagicLearnedness functions update the fields and trigger database persistence.

### TScroll reciteMe Execution Flow

The reciteMe function first checks ch->bSuccess(SKILL_READ_MAGIC) to determine if the caster can successfully read the scroll. If the check fails, it sends a failure message and returns DELETE_THIS to consume the scroll. If successful, it calls generic_find with FIND_CHAR_ROOM | FIND_OBJ_INV | FIND_OBJ_ROOM to locate the target. It initializes max_lag to LAG_0, then iterates i from 0 to 2. For each spell slot, it calls getSpell(i) and validates the spell ID is within range and discArray[spell] is not null. It updates max_lag if discArray[spell]->lag is larger. It calls setLocked(true), then doObjSpell, then setLocked(false). It checks IS_SET_DELETE(rc, DELETE_VICT) and breaks the loop if the victim dies and is not the caster. After all spells, it calls ch->addToWait(combatRound(max_lag + 2)) and returns DELETE_THIS.

### TWand useMe Execution Flow

The useMe function calls getSpell to retrieve the stored spell, then checks IS_SET(discArray[spell]->targets, TAR_IGNORE) and returns FALSE with an error message if true. It checks getCurCharges() <= 0 and returns FALSE with a depleted message if true. It builds a bitmask bv by checking discArray[spell]->targets for TAR_CHAR_ROOM, TAR_OBJ_INV, TAR_OBJ_ROOM, and TAR_OBJ_EQUIP, adding corresponding FIND_* flags. It calls generic_find with the bitmask and checks the return value bits for success. If zero, it sends a target not found message and returns FALSE. It calls addToCurCharges(-1) to consume a charge, then doObjSpell with the found target. It checks IS_SET_DELETE(rc, DELETE_VICT) and deletes tmp_char if set and tmp_char != ch. It checks IS_SET_DELETE(rc, DELETE_THIS) and returns DELETE_THIS if set. It checks IS_SET_DELETE(rc, DELETE_ITEM) and deletes o if set. It calls ch->addToWait(combatRound(discArray[spell]->lag)) and returns TRUE.

### TStaff useMe Execution Flow

The useMe function calls getSpell to retrieve the stored spell, then checks getCurCharges() <= 0 and returns FALSE with a no charges message if true. It calls addToCurCharges(-1) to consume a charge. It initializes rc to FALSE and isViolent to IS_SET(discArray[spell]->targets, TAR_VIOLENT). If IS_SET(discArray[spell]->targets, TAR_AREA) is true, it calls doObjSpel with null victim and target, then stores the result in rc. Otherwise, it creates a StuffIter it initialized to ch->roomp->stuff.begin() and loops while it != ch->roomp->stuff.end(). Inside the loop, it executes TThing* t = *(it++) to cache the element and advance the iterator. It calls dynamic_cast<TBeing*>(t) to check if t is a being. It skips if tmp_char is null or equals ch. It skips if isViolent and tmp_char->inGroup(*ch) is true. It calls doObjSpell with tmp_char and stores the result in rc. It checks IS_SET_DELETE(rc, DELETE_VICT), deletes tmp_char, and sets tmp_char to null if true. After the loop, it calls ch->addToWait(combatRound(discArray[spell]->lag)) and returns rc.

### doObjSpell Dispatch Logic

The doObjSpell function first checks IS_SET(discArray[spell]->targets, TAR_VIOLENT) and if true calls ch->checkPeaceful with an error message. If checkPeaceful returns true, it returns FALSE to block the spell. It then enters a switch statement on the spell parameter. Each case calls the specific spell implementation function, passing ch as the caster, victim as the target being, obj as the magic item, and obj->getMagicLevel() as the spell power. For area spells, it may pass argument instead of victim. The spell function returns an integer that may contain DELETE flags. The switch statement returns this value directly to propagate flags to the caller. The default case logs a bug with the unknown spell ID and returns FALSE.

### equipChar Affect Application

The equipChar function sets obj->equippedBy = this to establish the ownership pointer and obj->eq_pos = pos to record the equipment slot. It calls equipment.wear(obj, pos) to add the object to the equipment array. It attempts dynamic_cast<TObj*>(obj) to get a TObj pointer and checks affectShouldApply(to, pos) to determine if affects should apply. If both succeed, it loops j from 0 to MAX_OBJ_AFFECT. For each j, it calls affectModify with to->affected[j].location, to->affected[j].modifier, to->affected[j].modifier2, to->obj_flags.bitvector, TRUE for apply, and silent for messaging. The affectModify function adds the modifier to the character's stat specified by location and sets the bitvector flags.

### unequip Affect Removal

The unequip function calls equipment.remove(pos) to get the object pointer and remove it from the equipment array. It attempts dynamic_cast<TObj*>(o) to get a TObj pointer and checks affectShouldApply(to, pos). If both succeed, it loops j from 0 to MAX_OBJ_AFFECT. For each j, it calls affectModify with to->affected[j].location, to->affected[j].modifier, to->affected[j].modifier2, to->obj_flags.bitvector, FALSE for remove, and SILENT_NO for messaging. The affectModify function subtracts the modifier from the character's stat and clears the bitvector flags. After the loop, it sets o->equippedBy = NULL and o->eq_pos = WEAR_NOWHERE, then returns o.

### Spell Database Conversion

The mapFileToSpellnum function accepts an integer file_val from the database and returns the corresponding spellNumT enum value by indexing into a conversion table. The mapSpellnumToFile function accepts a spellNumT value and returns the integer representation for database storage. These functions handle the historical mismatch between file format spell numbering and the current enum order. The assignFourValues functions for scrolls, wands, and staves call mapFileToSpellnum on the spell fields before storing them. The getFourValues functions call mapSpellnumToFile before returning values for persistence.

### Charge Field Persistence

The TWand and TStaff assignFourValues functions call setMaxCharges(x2) to initialize the maximum charge capacity from val2 and addToCurCharges(x3) to set the current charges from val3. The getFourValues functions query getMaxCharges() and getCurCharges() to populate x2 and x3 for database writes. The addToCurCharges function accepts negative values for consumption, adding the parameter to curCharges and clamping the result between 0 and maxCharges. This ensures charges never exceed capacity or drop below zero.

## Troubleshooting

### Crash: Continuing After DELETE_VICT

When doObjSpell returns with DELETE_VICT set, the victim pointer is now invalid. Accessing victim->sendTo, victim->inRoom, or any other member causes a use-after-free crash. The fix requires immediately checking IS_SET_DELETE(rc, DELETE_VICT) after the call. If set and victim != ch, delete victim and set victim = NULL. Never perform any operations on victim after this point. For loops over multiple targets, use the cached iterator pattern to prevent invalidation.

### Crash: Iterator Invalidated by Deletion

When iterating through a container and deleting elements during iteration, the iterator becomes invalid after the deletion. Advancing an invalidated iterator causes a segmentation fault. The fix requires caching the next element before executing any operation that might delete the current element. Use TThing* t = *(it++) to advance the iterator in the same statement as dereferencing. After spell execution, check DELETE_VICT on the cached pointer t, not the iterator.

### Bug: Charges Go Negative

When useMe calls addToCurCharges(-1) without first checking getCurCharges() > 0, the charge count becomes negative. Subsequent uses may underflow or cause assertion failures. The fix requires checking getCurCharges() <= 0 before any addToCurCharges call with a negative value. Return FALSE with an appropriate message if the item is depleted. Only consume charges after validation succeeds.

### Bug: Wand Not Tradeable

When a wand has curCharges < maxCharges, shop code rejects it for sale. Players may not understand why the wand cannot be sold. The fix requires recharging the wand to full capacity before selling. The shop restriction enforces this in objectSell by checking wand->getCurCharges() != wand->getMaxCharges() and returning with an error message.

### Bug: DELETE_THIS Not Propagated

When doObjSpell sets DELETE_THIS but the caller returns TRUE instead of DELETE_THIS, the magic item pointer remains in scope and may be accessed again. This causes use-after-free when the caller's caller tries to access the item. The fix requires immediately returning DELETE_THIS after detecting IS_SET_DELETE(rc, DELETE_THIS). Never perform cleanup operations or message sending after DELETE_THIS is set.

### Bug: Using IS_SET for DELETE Flags

The DELETE flags use a combined bit pattern that IS_SET does not recognize. Using IS_SET(rc, DELETE_VICT) always returns false even when the flag is set. This causes the code to skip deletion and continue using the invalid pointer. The fix requires replacing all IS_SET calls for DELETE flags with IS_SET_DELETE. Search for patterns like IS_SET(rc, DELETE_ and replace with IS_SET_DELETE(rc, DELETE_.

### Bug: Scroll Deleted Mid-Spell

When multiple spells execute in sequence without lock protection, the first spell may trigger side effects that delete the scroll object. Accessing spell slots 2 or 3 after deletion causes a crash. The fix requires calling setLocked(true) before each doObjSpell call and setLocked(false) immediately after. The lock prevents deletion during spell execution but must be cleared between spells to allow proper cleanup.

### Bug: Invalid Spell Array Index

When accessing scroll->getSpell(i) with i >= 3, the access exceeds the bounds of the three-element spells array. This may return garbage values or crash. The fix requires hardcoding the loop limit to 3 for scrolls and validating the spell ID returned by getSpell. Check the_spell >= MIN_SPELL && the_spell < MAX_SKILL && discArray[the_spell] before using the spell.

### Bug: Affects Not Removed on Unequip

When unequip fails to call affectModify with apply = FALSE for each affect, the character retains the stat bonuses after removing the item. This allows permanent stat inflation by repeatedly equipping and unequipping. The fix requires ensuring unequip iterates through all MAX_OBJ_AFFECT slots and calls affectModify to subtract modifiers and clear bitvector flags before clearing the equippedBy pointer.

### Bug: Missing Spell Lag

When magic item usage returns without calling ch->addToWait, the player can immediately use another item or command. This violates the spell lag design and allows action flooding. The fix requires calling ch->addToWait(combatRound(discArray[spell]->lag)) before every return path in useMe and reciteMe. For scrolls casting multiple spells, track max_lag and apply it once at the end.
