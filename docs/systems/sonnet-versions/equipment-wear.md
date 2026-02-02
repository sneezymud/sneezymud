---
title: Equipment and Wear System
category: understanding
keywords: [wearSlotT, bodyPartsDamage, equipChar, unequip, limb-health, stuck-items, affectModify, ITEM_WEAR_FLAGS]
related: [combat-formulas.md, affects-system.md, object-types.md]
primary_symbols:
  functions: [equipChar, unequip, affectModify, stickIn, pulloutObj, break_bone, getCurLimbHealth, setCurLimbHealth, getMaxLimbHealth]
  classes: [TBeing, TThing, bodyPartsDamage]
  files: [code/code/misc/limbs.h, code/code/misc/limbs.cc, code/code/misc/being.h, code/code/sys/handler.cc, code/code/misc/range.cc]
---

## Overview

When a player puts on armor or picks up a sword, where does that item go? How does the game know if you can wear a helmet when your head is missing?

The equipment and wear system manages the relationship between characters and the items they equip. It tracks 24 distinct body slots where items can be worn or held, monitors the health and status of each limb independently, and applies magical effects when equipment is donned or removed.

This system exists because equipment is not just inventory - it's spatially located on specific body parts, those body parts can be damaged or severed, and equipped items modify character statistics through magical enchantments. A broken arm cannot hold a shield. A missing leg cannot wear a boot. An item stuck in your shoulder remains there until pulled out, potentially causing bleeding.

### Core Concepts

**Equipment slots** are discrete locations where items can be worn or held, represented by the `wearSlotT` enum. Standard humanoids use 20 slots covering head, neck, body, arms, hands, legs, feet, and two held positions. Non-humanoid creatures with extra limbs use up to 24 slots. Each equipped item occupies exactly one slot, though some items like pants occupy two slots simultaneously.

**Limb health** treats each body part as an independent entity with its own health points and status flags. A limb can be bleeding, infected, paralyzed, broken, scarred, bandaged, missing, or affected by other conditions. These conditions are tracked separately from the character's overall health and affect what can be equipped in that slot.

**Stuck items** are distinct from equipped items. An arrow embedded in your torso sits in the `stuckIn` pointer of that body part's data structure, not in your equipment array. Pulling it out may cause damage and bleeding.

**Equipment affects** are magical bonuses or penalties applied when an item is equipped. When you wear enchanted gauntlets that boost strength, `affectModify()` applies those bonuses to your stats. When you remove them, the bonuses are removed.

**Handedness** determines which hand is primary for wielding weapons and which arm is used first for shields. Most characters are right-handed, but high dexterity or specific races grant ambidexterity.

### Common Scenarios

**Equipping an item:** The player issues a wear command. The game validates that the item has the appropriate wear flag, finds an available slot matching the item's type, checks that the limb is functional, sets bidirectional pointers between item and character, and applies any magical affects from the item's enchantments.

**Taking limb damage:** During combat, a character takes a severe hit to the right arm. The limb health for that slot decreases. If damage is sufficient, the bone breaks and the `PART_BROKEN` flag is set. If the character was holding a weapon in that hand, they can no longer use it effectively until the arm heals.

**Removing stuck items:** An arrow is embedded in the character's shoulder. The player pulls it out. The game removes it from the `stuckIn` pointer, checks the limb's condition, potentially inflicts bleeding damage, and sets the `PART_BLEEDING` flag on that body part.

## Patterns

### Equipment State Validation

**Always verify limb functionality before equipping to held positions.** Held items require functional hands. Check that the limb is not missing, broken, paralyzed, or otherwise unusable via `canUseLimb()`. If you allow equipping to a non-functional limb, the item will be equipped but unusable, creating inconsistent game state.

**Always check item wear flags before attempting to equip.** Items declare valid positions through `obj_flags.wear_flags`. An item without `ITEM_WEAR_HEAD` cannot be equipped to `WEAR_HEAD` even if the slot is empty. Attempting to equip to an invalid position violates the item's design constraints.

**Never allow shields in primary hand position.** Shields occupy the off-hand. The `equipChar()` function explicitly prevents shield placement in `getPrimaryHold()`. Violating this creates combat calculation errors since the primary hand is where weapons are expected.

### Limb Health Management

**Always check `hasPart()` before operating on a limb.** Not all creatures have all body parts. A snake has no arms. Attempting to access `body_parts[]` for a non-existent limb reads invalid data.

**Always validate limb flags before state changes.** When breaking a bone, check that the limb is not already broken and that the character does not have boneless anatomy. The `break_bone()` function demonstrates this pattern - it verifies the character has the part and can have broken bones before setting `PART_BROKEN`.

**Never directly manipulate limb flags without using the API.** Use `addToLimbFlags()`, `remLimbFlags()`, and `isLimbFlags()` instead of direct bit manipulation. These methods maintain encapsulation and allow for future hook insertion.

### Stuck Item Mechanics

**Always set bidirectional pointers when sticking items.** When calling `stickIn()`, the function sets both `bodyPartsDamage.stuckIn` pointing to the item and `TThing::eq_stuck` pointing to the slot. If either pointer is missing, spatial consistency breaks and the item becomes orphaned or the limb retains a dangling reference.

**Always check for existing stuck items before insertion.** A body part can only have one stuck item at a time. The `stickIn()` function validates this. Overwriting an existing `stuckIn` pointer without removing the old item leaks the previous object.

**Always anticipate bleeding damage when pulling out stuck items.** The `pulloutObj()` function may inflict damage based on the item type and limb condition. Callers must check return values and handle potential character death from blood loss.

### Equipment Affects Application

**Always call `affectModify()` when equipping or unequipping.** This function iterates through the item's `affected[]` array and applies or removes bonuses. Forgetting this step causes items to occupy slots without providing their magical benefits, or worse, leaves ghost affects after unequipping.

**Always process all `MAX_OBJ_AFFECT` entries.** Items can have multiple affects. Partial application creates inconsistent stat calculations. The `equipChar()` flow demonstrates iterating through all affect slots.

**Always pass the correct `add` parameter to `affectModify()`.** Use `add=true` when equipping, `add=false` when unequipping. Reversing this doubles affects or removes affects that were never applied.

### Paired Equipment Handling

**Always handle paired items occupying two slots.** Pants occupy both `WEAR_LEG_R` and `WEAR_LEG_L`. Boots are separate items for each foot. The unequip logic must account for items spanning multiple slots versus separate items in symmetric slots.

**Always clear both slots when removing paired items.** If an item occupies two slots, both `equipment[slot]` pointers must be cleared and both `affectModify()` calls must occur. Leaving one slot populated creates asymmetric equipment state.

## Reference

### Symbol Quick Reference

| Symbol | Type | Purpose |
|--------|------|---------|
| `wearSlotT` | enum | Defines 24 equipment slot constants |
| `bodyPartsDamage` | struct | Tracks health and status of individual limbs |
| `equipChar()` | function | Equips item to slot, applies affects |
| `unequip()` | function | Removes item from slot, removes affects |
| `affectModify()` | function | Applies or removes item magical affects |
| `stickIn()` | function | Embeds item in body part |
| `pulloutObj()` | function | Removes stuck item, may cause bleeding |
| `break_bone()` | function | Sets PART_BROKEN flag on limb |
| `getCurLimbHealth()` | function | Returns current limb health points |
| `setCurLimbHealth()` | function | Sets limb health points |
| `getMaxLimbHealth()` | function | Returns maximum limb health |
| `hasPart()` | function | Checks if creature has body part |
| `canUseLimb()` | function | Checks if limb is functional |
| `isRightHanded()` | function | Returns dominant hand preference |
| `getPrimaryHold()` | function | Returns primary weapon slot |
| `getSecondaryHold()` | function | Returns off-hand slot |

### Equipment Slot Constants (wearSlotT)

| Value | Constant | Description |
|-------|----------|-------------|
| 0 | `WEAR_NOWHERE` | Not worn |
| 1 | `WEAR_HEAD` | Head |
| 2 | `WEAR_NECK` | Neck |
| 3 | `WEAR_BODY` | Body/torso |
| 4 | `WEAR_BACK` | Back |
| 5 | `WEAR_ARM_R` | Right arm |
| 6 | `WEAR_ARM_L` | Left arm |
| 7 | `WEAR_WRIST_R` | Right wrist |
| 8 | `WEAR_WRIST_L` | Left wrist |
| 9 | `WEAR_HAND_R` | Right hand |
| 10 | `WEAR_HAND_L` | Left hand |
| 11 | `WEAR_FINGER_R` | Right finger |
| 12 | `WEAR_FINGER_L` | Left finger |
| 13 | `WEAR_WAIST` | Waist/belt |
| 14 | `WEAR_LEG_R` | Right leg |
| 15 | `WEAR_LEG_L` | Left leg |
| 16 | `WEAR_FOOT_R` | Right foot |
| 17 | `WEAR_FOOT_L` | Left foot |
| 18 | `HOLD_RIGHT` | Right hand held |
| 19 | `HOLD_LEFT` | Left hand held |
| 20 | `WEAR_EX_LEG_R` | Extra right leg |
| 21 | `WEAR_EX_LEG_L` | Extra left leg |
| 22 | `WEAR_EX_FOOT_R` | Extra right foot |
| 23 | `WEAR_EX_FOOT_L` | Extra left foot |

**Range Constants:** `MIN_WEAR = 1`, `MAX_HUMAN_WEAR = 20`, `MAX_WEAR = 24`

### Limb Status Flags (PART_*)

| Flag | Bit | Description |
|------|-----|-------------|
| `PART_BLEEDING` | 0 | Limb is actively bleeding |
| `PART_INFECTED` | 1 | Infection present |
| `PART_PARALYZED` | 2 | Cannot move limb |
| `PART_BROKEN` | 3 | Bone is fractured |
| `PART_SCARRED` | 4 | Permanent scarring |
| `PART_BANDAGED` | 5 | Currently bandaged |
| `PART_MISSING` | 6 | Limb has been severed |
| `PART_USELESS` | 7 | Non-functional |
| `PART_LEPROSED` | 8 | Affected by leprosy |
| `PART_TRANSFORMED` | 9 | Magically altered |
| `PART_ENTANGLED` | 10 | Caught or bound |
| `PART_BRUISED` | 11 | Bruise damage |
| `PART_GANGRENOUS` | 12 | Tissue death from infection |

### Item Wear Flags (ITEM_WEAR_*)

| Flag | Bit | Description |
|------|-----|-------------|
| `ITEM_WEAR_TAKE` | 0 | Can be picked up |
| `ITEM_WEAR_FINGERS` | 1 | Can be worn on fingers |
| `ITEM_WEAR_NECK` | 2 | Can be worn on neck |
| `ITEM_WEAR_BODY` | 3 | Can be worn on body |
| `ITEM_WEAR_HEAD` | 4 | Can be worn on head |
| `ITEM_WEAR_LEGS` | 5 | Can be worn on legs |
| `ITEM_WEAR_FEET` | 6 | Can be worn on feet |
| `ITEM_WEAR_HANDS` | 7 | Can be worn on hands |
| `ITEM_WEAR_ARMS` | 8 | Can be worn on arms |
| `ITEM_WEAR_BACK` | 10 | Can be worn on back |
| `ITEM_WEAR_WAIST` | 11 | Can be worn on waist |
| `ITEM_WEAR_WRISTS` | 12 | Can be worn on wrists |
| `ITEM_WEAR_HOLD` | 14 | Can be held |
| `ITEM_WEAR_THROW` | 15 | Can be thrown |

### bodyPartsDamage Structure Fields

| Field | Type | Purpose |
|-------|------|---------|
| `flags` | `unsigned short` | Limb status flags (PART_* bits) |
| `stuckIn` | `TThing*` | Pointer to item embedded in limb |
| `health` | `unsigned short` | Current limb health points |

### Handedness Determination

| Condition | Result |
|-----------|--------|
| DEX > 180 | Ambidextrous |
| Race is Hobbit | Ambidextrous |
| Otherwise | Right-handed |

## Implementation

### Equipment Storage Architecture

Character equipment is stored in the `TBeing::equipment[]` array, indexed by `wearSlotT` values. Each array entry is a pointer to a `TThing` object currently equipped in that slot, or null if the slot is empty. The maximum array size is `MAX_WEAR` (24 slots).

Equipped items maintain a bidirectional relationship with their wearer. The `TThing::equippedBy` pointer references the character wearing the item, and `TThing::eq_pos` stores the slot index. This bidirectionality enables both "what is the character wearing?" and "who is wearing this item?" queries without traversal.

Items that occupy multiple slots store the same pointer in multiple array positions. For example, pants set both `equipment[WEAR_LEG_R]` and `equipment[WEAR_LEG_L]` to the same object pointer, but the item's `eq_pos` only stores one canonical slot.

### Limb Data Organization

Each character maintains an array `TBeing::body_parts[]` indexed by `wearSlotT`. Each entry is a `bodyPartsDamage` structure containing three fields: status flags, a stuck item pointer, and current health points.

The `flags` field is a bitmask storing multiple concurrent conditions. A limb can simultaneously be bleeding, infected, and bandaged by having multiple PART_* bits set. Flag operations use bitwise OR for addition and bitwise AND with complement for removal.

The `stuckIn` pointer is independent of the `equipment[]` array. An arrow embedded in your shoulder occupies the `body_parts[WEAR_BODY].stuckIn` pointer, while a breastplate worn on the same body part occupies `equipment[WEAR_BODY]`. These are separate spatial relationships.

Limb health points represent localized damage. When a limb takes damage, its health decreases independently of the character's overall hit points. When limb health reaches zero, the limb becomes non-functional or may be severed depending on damage type.

### Equipment Application Flow

When `equipChar()` is called with an item and slot, the function follows this sequence:

First, it validates slot range is within `MIN_WEAR` to `MAX_WEAR` and that the target slot is currently empty. If the slot is occupied, the operation fails immediately.

Second, it performs shield-specific validation. If the item is a shield and the target slot is the primary hand position, the operation is rejected. Shields must occupy the off-hand slot.

Third, for held positions (`HOLD_RIGHT` or `HOLD_LEFT`), it checks limb functionality. The hand must not be missing, broken, paralyzed, or otherwise unusable. This validation uses `canUseLimb()` which checks for `PART_MISSING`, `PART_BROKEN`, `PART_PARALYZED`, and `PART_USELESS` flags.

Fourth, it establishes bidirectional pointers. `equipment[slot]` is set to point to the item, `item->equippedBy` is set to point to the character, and `item->eq_pos` is set to the slot index.

Fifth, it applies magical affects. The function iterates through the item's `affected[]` array up to `MAX_OBJ_AFFECT` entries. For each affect entry, it calls `affectModify()` with the affect's location, modifier value, and `add=true` parameter. This modifies character statistics like strength, armor class, or hit points.

Sixth, it updates the character's light level if the item is a light source. This affects visibility calculations in dark rooms.

### Equipment Removal Flow

When `unequip()` is called with a slot index, the function follows this sequence:

First, it retrieves the item pointer from `equipment[slot]`. If the slot is empty, it returns null immediately.

Second, it removes magical affects. The function iterates through the item's `affected[]` array and calls `affectModify()` with `add=false`, reversing the stat modifications that were applied during equipping.

Third, it handles paired items. If the item occupies two slots (like pants), both slot pointers must be cleared. The function identifies paired items and clears both `equipment[]` entries.

Fourth, it clears bidirectional pointers. Both `item->equippedBy` and `item->eq_pos` are reset to null/zero values.

Fifth, it returns the removed item pointer. The caller is responsible for deciding what to do with the unequipped item - typically adding it to inventory or dropping it.

### Affect Modification Mechanics

The `affectModify()` function takes an affect location (which stat to modify), a modifier value (how much to change it), a bitvector (additional flags to apply), and an add parameter (true for equip, false for unequip).

When adding affects, the function identifies which character attribute corresponds to the affect location - strength, dexterity, hit points, armor class, etc. It then adds the modifier value to that attribute. If the bitvector is non-zero, it sets those flag bits on the character using bitwise OR.

When removing affects, it subtracts the modifier value from the attribute and clears the bitvector flags using bitwise AND with complement.

Special handling exists for certain affect locations. Hit point modifications may trigger recalculation of maximum hit points. Armor class modifications affect combat calculations. Attribute modifications may change derived values like carrying capacity or spell success rates.

The function does not validate that the modifier is appropriate for the affect location. A poorly configured item could apply strength bonuses to armor class locations, creating nonsensical stat modifications.

### Stuck Item Mechanics

The `stickIn()` function establishes a stuck item relationship. It takes the item, target body part slot, and a silence parameter for messaging.

First, it validates the target slot is within valid range and the character has that body part via `hasPart()`.

Second, it checks if the limb already has a stuck item. If `body_parts[slot].stuckIn` is non-null, the operation fails because only one item can be stuck in a limb at a time.

Third, it establishes bidirectional pointers. `body_parts[slot].stuckIn` is set to the item, and `item->eq_stuck` is set to the slot index. The item's `stuckIn` pointer is set to point back to the character.

Fourth, it sends messaging to the room unless silenced, describing the item becoming embedded.

The `pulloutObj()` function reverses this process. It takes the slot index, a safety parameter, and an output result parameter.

First, it retrieves the stuck item from `body_parts[slot].stuckIn`. If the slot has no stuck item, it returns null.

Second, it calculates bleeding damage based on item type and limb condition. Larger items cause more damage when removed. If the limb is already bleeding, pulling out the item may worsen the bleeding.

Third, it clears the stuck item pointers on both the character and the item.

Fourth, it applies bleeding damage to the limb and potentially sets the `PART_BLEEDING` flag if damage is sufficient.

Fifth, it returns the removed item pointer. The result parameter is set to indicate success or failure codes.

### Handedness and Primary Slot Resolution

Character handedness is determined by the `isRightHanded()` function. Characters with dexterity above 180 or those of the Hobbit race are ambidextrous (treated as right-handed for slot selection but with no combat penalties). All other characters are right-handed.

The `getPrimaryHold()` function returns `HOLD_RIGHT` for right-handed characters and `HOLD_LEFT` for left-handed characters. This is where weapons are typically wielded.

The `getSecondaryHold()` function returns the opposite - `HOLD_LEFT` for right-handed, `HOLD_RIGHT` for left-handed. This is where shields and secondary weapons go.

Similar functions exist for hands (`getPrimaryHand()`, `getSecondaryHand()`) and arms (`getPrimaryArm()`, `getSecondaryArm()`), returning the corresponding `WEAR_HAND_*` and `WEAR_ARM_*` slots based on handedness.

Combat calculations use these functions to determine which hand is primary for attack rolls and which hand can hold shields. Two-handed weapons require both hold slots to be empty or occupied by the same weapon item.

### Limb Breakage Implementation

The `break_bone()` function is called when limb trauma occurs during combat or environmental damage. It takes the character pointer and target limb slot.

First, it validates the character has the specified body part via `hasPart()`. Creatures without that limb cannot have bones broken there.

Second, it checks if the limb is already broken by testing for the `PART_BROKEN` flag. If already broken, the function returns without modification to avoid redundant processing.

Third, it checks for boneless anatomy. Certain creature types like slimes or incorporeal beings cannot have broken bones. This is typically a race or mob type flag check.

Fourth, if all validations pass, it sets the `PART_BROKEN` flag using `addToLimbFlags()`.

Fifth, it may trigger additional effects like dropping held items if the broken limb is a hand, or falling prone if the broken limb is a leg. These effects are handled by callers inspecting the limb state after breakage.

The broken state affects limb functionality. `canUseLimb()` returns false for broken limbs, preventing equipment in that slot from being used effectively in combat even though it remains equipped.

### File Organization

Equipment core logic resides in `code/code/sys/handler.cc`, which contains `equipChar()`, `unequip()`, and `affectModify()` functions. These are central handler functions called throughout the codebase whenever equipment state changes.

Limb-specific functionality is separated into `code/code/misc/limbs.cc`, containing limb health manipulation, flag operations, and `break_bone()`. The corresponding header `code/code/misc/limbs.h` defines the `wearSlotT` enum and declares limb-related functions.

Stuck item mechanics are implemented in `code/code/misc/range.cc`, specifically the `stickIn()` function. This placement reflects the combat/ranged attack origin of stuck items.

Character and object data structures are defined in `code/code/misc/being.h` and `code/code/misc/obj.h`. These headers declare the `bodyPartsDamage` structure, equipment arrays, and wear flag constants.

Flag constants for limb status (`PART_*`) and item wear positions (`ITEM_WEAR_*`) are defined in `code/code/misc/defs.h` as part of the global constant definitions.

## Troubleshooting

### Item Equipped But Not Providing Bonuses

**Symptom:** Character has item visible in equipment list, but stats do not reflect the item's bonuses.

**Likely cause:** `affectModify()` was not called during equipping, or was called with incorrect parameters.

**Diagnostic approach:** Check if the item has affects defined in its `affected[]` array. Verify that `equipChar()` iterated through all `MAX_OBJ_AFFECT` slots. Check if `affectModify()` was called with `add=true`.

**Fix:** Ensure `equipChar()` calls `affectModify()` for each affect entry in the item's affected array. If manually equipping items outside normal command flow, you must explicitly call `affectModify()` yourself.

### Ghost Affects After Unequipping

**Symptom:** Character retains stat bonuses after removing equipment, leading to inflated attributes.

**Likely cause:** `affectModify()` was not called during unequipping, or was called with incorrect `add` parameter.

**Diagnostic approach:** Verify `unequip()` was called rather than directly clearing the equipment slot pointer. Check if `affectModify()` was called with `add=false` for affect removal.

**Fix:** Always use `unequip()` to remove equipment. If manually manipulating equipment state, ensure you call `affectModify()` with the `add=false` parameter for each affect entry.

### Cannot Equip Item to Valid Slot

**Symptom:** Player attempts to wear an item to an appropriate slot but receives error message.

**Likely cause:** Limb is non-functional due to missing, broken, paralyzed, or useless status flags.

**Diagnostic approach:** Check limb flags for `PART_MISSING`, `PART_BROKEN`, `PART_PARALYZED`, or `PART_USELESS`. Use `canUseLimb()` to verify functionality. Check if character has the body part via `hasPart()`.

**Fix:** Heal or repair the limb before attempting to equip. For broken limbs, apply splints or healing magic. For missing limbs, regeneration magic is required. Ensure the creature actually has the body part in question.

### Stuck Item Causes Crash on Removal

**Symptom:** Game crashes when player attempts to pull out embedded item.

**Likely cause:** Bidirectional pointers were not properly established during `stickIn()`, or character/item was deleted without clearing the stuck item relationship.

**Diagnostic approach:** Verify both `body_parts[slot].stuckIn` and `item->stuckIn` pointers are valid. Check if item was deleted without calling `pulloutObj()`. Verify `item->eq_stuck` matches the slot index.

**Fix:** Always use `stickIn()` to establish stuck item relationships. Never delete stuck items without first calling `pulloutObj()` to clear the bidirectional pointers. Check for null pointers before dereferencing in `pulloutObj()`.

### Shield Equipped in Primary Hand

**Symptom:** Character has shield in primary weapon slot, causing combat calculation errors.

**Likely cause:** `equipChar()` validation was bypassed, or shield was equipped directly without using normal equipment flow.

**Diagnostic approach:** Check how the shield was equipped. Verify if `equipChar()` was called or if equipment array was manipulated directly. Check if the slot matches `getPrimaryHold()`.

**Fix:** Always use `equipChar()` for equipment operations. Do not directly modify the `equipment[]` array. If building custom equipment logic, replicate the shield placement validation from `equipChar()`.

### Limb Health Not Updating

**Symptom:** Limb takes damage but health value does not change.

**Likely cause:** Using direct field access instead of `setCurLimbHealth()`, or operating on a limb the character does not have.

**Diagnostic approach:** Verify `hasPart()` returns true for the target limb. Check if health modification code uses `setCurLimbHealth()` or directly accesses `body_parts[].health`.

**Fix:** Always use `setCurLimbHealth()` to modify limb health. Check `hasPart()` before any limb operations. Do not directly manipulate `body_parts[]` fields.

### Multiple Items Stuck in Same Limb

**Symptom:** Multiple arrows or weapons appear to be embedded in the same body part, but only one is retrievable.

**Likely cause:** `stickIn()` validation was bypassed, allowing overwrite of existing `stuckIn` pointer without removing the previous item.

**Diagnostic approach:** Check if `stickIn()` properly validated for existing stuck items. Trace how multiple items were stuck to find where validation failed.

**Fix:** Ensure `stickIn()` checks for non-null `body_parts[slot].stuckIn` before allowing new stuck items. Properly remove previous stuck items using `pulloutObj()` before sticking new ones.
