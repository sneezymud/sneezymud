---
title: Equipment and Wear System
description: Equipment slots, limb health tracking, and stuck item management for character body state.
category: informational
keywords: [equipment slots, limb health, stuck items, body state]
primary_symbols:
  functions: [equipChar, unequip, affectModify, stickIn, pulloutObj, break_bone, getCurLimbHealth, canUseLimb]
  classes: [TBeing, TThing, bodyPartsDamage]
  enums: [wearSlotT, WEAR_NOWHERE, WEAR_HEAD, WEAR_NECK, WEAR_BODY, WEAR_BACK, WEAR_ARM_R, WEAR_ARM_L, WEAR_WRIST_R, WEAR_WRIST_L, WEAR_HAND_R, WEAR_HAND_L, WEAR_FINGER_R, WEAR_FINGER_L, WEAR_WAIST, WEAR_LEG_R, WEAR_LEG_L, WEAR_FOOT_R, WEAR_FOOT_L, HOLD_RIGHT, HOLD_LEFT, WEAR_EX_LEG_R, WEAR_EX_LEG_L, WEAR_EX_FOOT_R, WEAR_EX_FOOT_L, MIN_WEAR, MAX_HUMAN_WEAR, MAX_WEAR, PART_BLEEDING, PART_INFECTED, PART_PARALYZED, PART_BROKEN, PART_SCARRED, PART_BANDAGED, PART_MISSING, PART_USELESS, PART_LEPROSED, PART_TRANSFORMED, PART_ENTANGLED, PART_BRUISED, PART_GANGRENOUS, ITEM_WEAR_TAKE, ITEM_WEAR_FINGERS, ITEM_WEAR_NECK, ITEM_WEAR_BODY, ITEM_WEAR_HEAD, ITEM_WEAR_LEGS, ITEM_WEAR_FEET, ITEM_WEAR_HANDS, ITEM_WEAR_ARMS, ITEM_WEAR_BACK, ITEM_WEAR_WAIST, ITEM_WEAR_WRISTS, ITEM_WEAR_HOLD, ITEM_WEAR_THROW]
---

## Overview

How does a character wear armor on their right arm when that arm is broken, bleeding, and has an arrow stuck in it? The equipment and wear system manages this complexity by tracking three independent layers: equipment slots (what you're wearing), limb health (the condition of body parts), and stuck items (objects embedded in flesh).

Equipment slots define the 24 possible locations where items can be worn or held. Humanoid characters use slots 1-20 covering head, body, arms, hands, fingers, legs, and feet. Non-humanoid creatures with additional appendages use slots 20-23 for extra legs and feet. Each slot operates independently, so losing one arm doesn't prevent wearing a bracer on the other.

Limb health tracks the physical condition of each body part separately from what's equipped there. A character can wear gauntlets on paralyzed hands or boots on broken legs - the equipment remains in place even when the underlying limb is non-functional. Health points, status flags, and stuck item references are maintained per-limb.

Stuck items represent objects embedded in body parts during combat, like arrows or throwing knives. These are tracked separately from equipped items because they occupy different conceptual spaces: you can have an arrow stuck in your arm while also wearing a bracer on that arm.

When equipment is worn, the system applies any magical affects to the character's statistics. When equipment is removed, those affects are reversed. This bidirectional affect application ensures character stats always reflect their current equipment state.

## Patterns

### Equipment Slot Validation

Always validate slot ranges before equipment operations. Valid humanoid slots are 1-20, with slots 20-23 reserved for non-humanoid extra limbs. Slot 0 (`WEAR_NOWHERE`) indicates an item is not worn.

Always check item wear flags before attempting to equip. Items declare valid positions through `obj_flags.wear_flags`. An item without `ITEM_WEAR_HEAD` cannot be equipped to `WEAR_HEAD` even if the slot is empty.

Never equip shields in the primary hand slot. The `equipChar` function enforces this, but code creating equipment scenarios must respect this constraint.

Always check hand functionality before placing held items. Use `canUseLimb()` to verify the hand slot is functional before attempting to equip held items.

### Limb Health Management

Always check `hasPart()` before manipulating limb data. Non-humanoid creatures may lack certain body parts entirely.

Never assume limb health implies functionality. A limb can have full health but be paralyzed, entangled, or otherwise useless. Use `canUseLimb()` for functionality checks, not health checks.

Always use the limb flag manipulation methods (`addToLimbFlags`, `remLimbFlags`, `isLimbFlags`) rather than direct bit manipulation. These methods handle validation and edge cases.

### Stuck Item Handling

Always clear the stuck item reference when removing an object. The `pulloutObj` function handles this, but direct manipulation of `stuckIn` must maintain consistency.

Always check for existing stuck items before insertion. A body part can only have one stuck item at a time. The `stickIn()` function validates this. Overwriting an existing `stuckIn` pointer without removing the old item leaks the previous object.

Never forget that pulling stuck items causes bleeding damage. The `pulloutObj` function returns a result code indicating whether damage occurred.

Always check both `stuckIn` and `equippedBy` when searching for objects on a character. An item in a body part slot might be stuck rather than equipped.

### Equipment Affects

Always call `affectModify()` when equipping or unequipping items. Each entry in the item's `affected[]` array must be processed.

Never assume affects are applied once. The system iterates through `MAX_OBJ_AFFECT` entries for each item during equip and unequip operations.

Always pass the correct `add` parameter to `affectModify()`. Use `true` when equipping (adding affects) and `false` when unequipping (removing affects).

### Paired Equipment Handling

Always handle paired items occupying two slots. Pants occupy both `WEAR_LEG_R` and `WEAR_LEG_L`. The item's `eq_pos` stores one canonical slot, but both `equipment[]` entries point to the same object.

Always clear both slots when removing paired items. Both `equipment[slot]` pointers must be cleared and affects should only be removed once.

### Handedness

Always use the handedness helper methods rather than assuming slot positions. `getPrimaryHold()`, `getSecondaryHold()`, `getPrimaryHand()`, and related methods return the correct slots based on the character's dominant hand.

Never hardcode `HOLD_RIGHT` or `HOLD_LEFT` for primary/secondary distinctions. Left-handed characters exist, and characters with high DEX or Hobbit race are ambidextrous.

## Reference

### Symbol Quick Reference

| Symbol | Type | Purpose |
|--------|------|---------|
| `wearSlotT` | enum | Defines 24 equipment slot constants |
| `bodyPartsDamage` | class | Per-limb health, flags, and stuck item data |
| `equipChar` | function | Attach item to wear slot, apply affects |
| `unequip` | function | Remove item from wear slot, reverse affects |
| `affectModify` | function | Apply or remove item affects to character stats |
| `stickIn` | function | Embed object in body part |
| `pulloutObj` | function | Remove stuck object, may cause bleeding |
| `break_bone` | function | Set PART_BROKEN on limb if valid |
| `getCurLimbHealth` | function | Query current limb health points |
| `setCurLimbHealth` | function | Set limb health points |
| `getMaxLimbHealth` | function | Query maximum limb health |
| `hasPart` | function | Check if creature has body part |
| `canUseLimb` | function | Check if limb is functional |
| `isRightHanded` | function | Query dominant hand preference |
| `getPrimaryHold` | function | Get primary weapon slot |
| `getSecondaryHold` | function | Get off-hand slot |
| `TBeing` | class | Character base class, owns body_parts array |
| `TThing` | class | Object base class, has stuckIn pointer |

### Equipment Slots (wearSlotT)

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
| 18 | `HOLD_RIGHT` | Right held |
| 19 | `HOLD_LEFT` | Left held |
| 20 | `WEAR_EX_LEG_R` | Extra right leg |
| 21 | `WEAR_EX_LEG_L` | Extra left leg |
| 22 | `WEAR_EX_FOOT_R` | Extra right foot |
| 23 | `WEAR_EX_FOOT_L` | Extra left foot |

Range constants: `MIN_WEAR` = 1, `MAX_HUMAN_WEAR` = 20, `MAX_WEAR` = 24

### Limb Status Flags (PART_*)

| Flag | Bit | Description |
|------|-----|-------------|
| `PART_BLEEDING` | 0 | Actively bleeding |
| `PART_INFECTED` | 1 | Infection present |
| `PART_PARALYZED` | 2 | Cannot move |
| `PART_BROKEN` | 3 | Bone broken |
| `PART_SCARRED` | 4 | Permanent scarring |
| `PART_BANDAGED` | 5 | Currently bandaged |
| `PART_MISSING` | 6 | Severed/missing |
| `PART_USELESS` | 7 | Non-functional |
| `PART_LEPROSED` | 8 | Leprosy affliction |
| `PART_TRANSFORMED` | 9 | Magically transformed |
| `PART_ENTANGLED` | 10 | Entangled/restrained |
| `PART_BRUISED` | 11 | Bruised |
| `PART_GANGRENOUS` | 12 | Gangrene |

### Item Wear Flags (ITEM_WEAR_*)

| Flag | Bit | Description |
|------|-----|-------------|
| `ITEM_WEAR_TAKE` | 0 | Can be picked up |
| `ITEM_WEAR_FINGERS` | 1 | Worn on fingers |
| `ITEM_WEAR_NECK` | 2 | Worn on neck |
| `ITEM_WEAR_BODY` | 3 | Worn on body |
| `ITEM_WEAR_HEAD` | 4 | Worn on head |
| `ITEM_WEAR_LEGS` | 5 | Worn on legs |
| `ITEM_WEAR_FEET` | 6 | Worn on feet |
| `ITEM_WEAR_HANDS` | 7 | Worn on hands |
| `ITEM_WEAR_ARMS` | 8 | Worn on arms |
| `ITEM_WEAR_BACK` | 10 | Worn on back |
| `ITEM_WEAR_WAIST` | 11 | Worn on waist |
| `ITEM_WEAR_WRISTS` | 12 | Worn on wrists |
| `ITEM_WEAR_HOLD` | 14 | Can be held |
| `ITEM_WEAR_THROW` | 15 | Can be thrown |

### bodyPartsDamage Class

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

### Key Files

| File | Contents |
|------|----------|
| `code/code/misc/limbs.h` | wearSlotT enum, limb function declarations |
| `code/code/misc/limbs.cc` | Limb manipulation implementation |
| `code/code/misc/being.h` | TBeing class, bodyPartsDamage class |
| `code/code/misc/defs.h` | PART_* flag constants |
| `code/code/misc/obj.h` | ITEM_WEAR_* flag constants |
| `code/code/sys/handler.cc` | equipChar, unequip, affectModify |
| `code/code/misc/range.cc` | stickIn implementation |

## Implementation

### Limb Health Data Structure

Each body part's state is stored in the `bodyPartsDamage` class, which contains three fields: `flags` (an unsigned short holding PART_* status bits), `stuckIn` (a pointer to any object embedded in the limb), and `health` (an unsigned short tracking current limb health points). The `TBeing` class maintains an array of these instances indexed by `wearSlotT`.

The limb health API provides accessors for querying and modifying this data. `getCurLimbHealth()` and `setCurLimbHealth()` handle health points. `getLimbFlags()`, `addToLimbFlags()`, `remLimbFlags()`, and `isLimbFlags()` manage status flags. `hasPart()` checks whether a creature has a given body part, and `canUseLimb()` determines if a limb is functional (has the part and isn't useless, paralyzed, or missing).

### Bone Breaking

The `break_bone()` function in `limbs.cc` applies the `PART_BROKEN` flag to a limb. Before setting the flag, it validates that the character has the body part, doesn't have boneless anatomy (some creature types), and the limb isn't already broken. This prevents redundant bone-breaking on already-broken limbs.

### Stuck Items System

When projectiles or thrown weapons hit a character, they may become stuck in the target's body. The `stickIn()` function embeds an object in a specific body part slot, setting the object's `stuckIn` pointer to reference the being and storing the slot in `eq_stuck`. The silent parameter controls whether messages are sent to observers.

Removing stuck items is handled by `pulloutObj()`, which takes the limb slot, a safety flag, and returns the extracted object. The result parameter indicates whether the removal caused bleeding damage. Safe removal (typically by healers) avoids additional damage, while forceful removal can worsen the wound.

The stuck item pointer in `bodyPartsDamage` and the equipped item in `equipment[]` are independent. A character can simultaneously have an item stuck in their arm and armor equipped on that arm.

### Equipment Flow: Equipping

The `equipChar()` function attaches an item to a character's equipment slot. The process validates the slot range, ensures the slot is available, and handles special cases like shields (which cannot go in the primary hand) and held items (which require functional hands).

Once validation passes, the function sets the object's `equippedBy` pointer to the character and `eq_pos` to the slot. It then iterates through the item's `affected[]` array (up to `MAX_OBJ_AFFECT` entries) and calls `affectModify()` for each, applying the item's magical effects to the character's statistics. Finally, it updates the character's light level if the item provides illumination.

Items that occupy multiple slots (like pants) store the same pointer in multiple `equipment[]` array positions, but the item's `eq_pos` stores only one canonical slot.

### Equipment Flow: Unequipping

The `unequip()` function reverses the equipping process. It calls `affectModify()` with `add=false` for each affect entry, removing the item's magical effects from the character. For paired items that occupy two slots (like pants covering both legs or dual-held items), the function handles clearing both slot references.

After removing affects, the function clears the `equippedBy` and `eq_pos` references on the object, fully detaching it from the character. The function returns the removed item so the caller can place it appropriately (in inventory, on the ground, etc.).

### Affect Application

The `affectModify()` function is the core mechanism for applying or removing item effects. It takes the character, the affect location (which stat to modify), the modifier amount, the bitvector (additional flags), and whether to add or remove the affect.

When adding, the modifier is applied to the appropriate character stat. When removing, the modifier is subtracted. The function handles various affect locations including attributes (STR, DEX, etc.), combat stats (hitroll, damroll), and special properties (light, armor class).

### Handedness System

Character handedness determines which side is primary for combat and equipment purposes. The `isRightHanded()` method returns the character's dominant hand. Helper methods translate this into specific slots: `getPrimaryHold()` returns `HOLD_RIGHT` or `HOLD_LEFT` based on handedness, with `getSecondaryHold()` returning the opposite.

Similar methods exist for hands (`getPrimaryHand()`/`getSecondaryHand()`) and arms (`getPrimaryArm()`/`getSecondaryArm()`). These abstractions allow combat code to reference primary and secondary positions without hardcoding specific slots.

Ambidexterity is granted to characters with DEX above 180 or those of Hobbit race. Ambidextrous characters face no penalty when using their off-hand.

## Troubleshooting

### Equipment Affects Not Applying

**Symptom:** Character equips an item but stats don't change as expected.

**Likely cause:** The `affectModify()` call is missing or using wrong parameters.

**Diagnostic approach:** Verify that `equipChar()` is being called (not direct slot assignment). Check that the item's `affected[]` array is populated correctly. Trace through `affectModify()` to confirm the affect location matches the expected stat.

**Fix:** Ensure equipment is always attached via `equipChar()`, never by directly setting `equipment[]` slots.

### Ghost Affects After Unequipping

**Symptom:** Character retains stat bonuses after removing equipment, leading to inflated attributes.

**Likely cause:** `affectModify()` was not called during unequipping, or was called with incorrect `add` parameter.

**Diagnostic approach:** Verify `unequip()` was called rather than directly clearing the equipment slot pointer. Check if `affectModify()` was called with `add=false` for affect removal.

**Fix:** Always use `unequip()` to remove equipment. Never directly manipulate `equipment[]` slots.

### Stuck Item Not Visible

**Symptom:** An arrow is stuck in a character but doesn't show in equipment or inventory checks.

**Likely cause:** Code is checking `equipment[]` but not `stuckIn` references in `body_parts[]`.

**Diagnostic approach:** Check if the item's `stuckIn` pointer references the character. Verify the `eq_stuck` slot matches where the item should be.

**Fix:** When searching for items on a character, iterate both the equipment array and the stuckIn pointers in body_parts.

### Limb Appears Functional When It Shouldn't

**Symptom:** Character can use a limb that should be broken/paralyzed/missing.

**Likely cause:** Code is checking limb health instead of `canUseLimb()`, or flag check is incorrect.

**Diagnostic approach:** Use `getLimbFlags()` to dump the actual flags on the limb. Verify the expected PART_* flag is set.

**Fix:** Replace health checks with `canUseLimb()` calls. Ensure `addToLimbFlags()` was called to set the status, not direct assignment.

### Off-Hand Penalty Applied Incorrectly

**Symptom:** Right-handed character gets penalty on right hand, or left-handed character on left.

**Likely cause:** Hardcoded slot assumptions instead of using handedness helpers.

**Diagnostic approach:** Check if code uses `HOLD_RIGHT`/`HOLD_LEFT` directly or calls `getPrimaryHold()`/`getSecondaryHold()`.

**Fix:** Replace hardcoded slot constants with the handedness helper methods.

### Cannot Equip Item to Valid Slot

**Symptom:** Player attempts to wear an item to an appropriate slot but receives error message.

**Likely cause:** Limb is non-functional due to missing, broken, paralyzed, or useless status flags.

**Diagnostic approach:** Check limb flags for `PART_MISSING`, `PART_BROKEN`, `PART_PARALYZED`, or `PART_USELESS`. Use `canUseLimb()` to verify functionality. Check if character has the body part via `hasPart()`.

**Fix:** Heal or repair the limb before attempting to equip. For broken limbs, apply healing. For missing limbs, regeneration is required.

### Stuck Item Causes Crash on Removal

**Symptom:** Game crashes when player attempts to pull out embedded item.

**Likely cause:** Bidirectional pointers were not properly established during `stickIn()`, or item was deleted without clearing the stuck item relationship.

**Diagnostic approach:** Verify both `body_parts[slot].stuckIn` and `item->stuckIn` pointers are valid. Check if item was deleted without calling `pulloutObj()`. Verify `item->eq_stuck` matches the slot index.

**Fix:** Always use `stickIn()` to establish stuck item relationships. Never delete stuck items without first calling `pulloutObj()` to clear the bidirectional pointers.

### Multiple Items Stuck in Same Limb

**Symptom:** Multiple items appear to be embedded in the same body part, but only one is retrievable.

**Likely cause:** `stickIn()` validation was bypassed, allowing overwrite of existing `stuckIn` pointer.

**Diagnostic approach:** Check if `stickIn()` properly validated for existing stuck items. Trace how multiple items were stuck to find where validation failed.

**Fix:** Ensure `stickIn()` checks for non-null `body_parts[slot].stuckIn` before allowing new stuck items.
