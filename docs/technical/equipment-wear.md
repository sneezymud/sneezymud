---
title: Equipment and Wear System
description: SneezyMUD's equipment and limb tracking system including equipment slots, limb health, item wear flags, and stuck items management.
keywords:
  - wearSlotT
  - bodyPartsDamage
  - equipChar
  - unequip
  - WEAR_HEAD
  - WEAR_BODY
  - ITEM_WEAR_FLAGS
  - limb-health
  - stuck-items
  - stickIn
  - pulloutObj
  - equipment-affects
  - affectModify
  - break-bone
category: Understanding Systems
related:
  - combat-formulas.md
  - affects-system.md
  - object-types.md
last_updated: 2026-01-29
source_files:
  - code/code/misc/limbs.h
  - code/code/misc/limbs.cc
  - code/code/misc/being.h
  - code/code/misc/defs.h
  - code/code/misc/obj.h
  - code/code/sys/handler.cc
  - code/code/misc/range.cc
---

# Equipment and Wear System

This document describes SneezyMUD's equipment and limb tracking system, including equipment slots, limb health, and item wear flags.

## Equipment Slots (wearSlotT)

The game defines 24 equipment slots via the `wearSlotT` enum in `code/code/misc/limbs.h`. Humanoid characters use slots 1-20; non-humanoid creatures with extra limbs use slots 21-24.

| Value | Constant | Description | Value | Constant | Description |
|-------|----------|-------------|-------|----------|-------------|
| 0 | `WEAR_NOWHERE` | Not worn | 13 | `WEAR_WAIST` | Waist/belt |
| 1 | `WEAR_HEAD` | Head | 14 | `WEAR_LEG_R` | Right leg |
| 2 | `WEAR_NECK` | Neck | 15 | `WEAR_LEG_L` | Left leg |
| 3 | `WEAR_BODY` | Body/torso | 16 | `WEAR_FOOT_R` | Right foot |
| 4 | `WEAR_BACK` | Back | 17 | `WEAR_FOOT_L` | Left foot |
| 5 | `WEAR_ARM_R` | Right arm | 18 | `HOLD_RIGHT` | Right held |
| 6 | `WEAR_ARM_L` | Left arm | 19 | `HOLD_LEFT` | Left held |
| 7 | `WEAR_WRIST_R` | Right wrist | 20 | `WEAR_EX_LEG_R` | Extra right leg |
| 8 | `WEAR_WRIST_L` | Left wrist | 21 | `WEAR_EX_LEG_L` | Extra left leg |
| 9 | `WEAR_HAND_R` | Right hand | 22 | `WEAR_EX_FOOT_R` | Extra right foot |
| 10 | `WEAR_HAND_L` | Left hand | 23 | `WEAR_EX_FOOT_L` | Extra left foot |
| 11 | `WEAR_FINGER_R` | Right finger | | | |
| 12 | `WEAR_FINGER_L` | Left finger | | | |

**Range Constants:** `MIN_WEAR = WEAR_HEAD` (1), `MAX_HUMAN_WEAR = 20`, `MAX_WEAR = 24`

## Limb Health System

Each body part tracks independent health and status via `bodyPartsDamage` in `TBeing::body_parts[]` (defined in `code/code/misc/being.h`).

### bodyPartsDamage Fields

| Field | Type | Description |
|-------|------|-------------|
| `flags` | `unsigned short` | Limb status flags (PART_*) |
| `stuckIn` | `TThing*` | Item stuck in this body part |
| `health` | `unsigned short` | Current limb health points |

### Limb Status Flags (PART_*)

Defined in `code/code/misc/defs.h`:

| Flag | Bit | Description | Flag | Bit | Description |
|------|-----|-------------|------|-----|-------------|
| `PART_BLEEDING` | 0 | Bleeding | `PART_USELESS` | 7 | Non-functional |
| `PART_INFECTED` | 1 | Infection | `PART_LEPROSED` | 8 | Leprosy |
| `PART_PARALYZED` | 2 | Paralyzed | `PART_TRANSFORMED` | 9 | Magically transformed |
| `PART_BROKEN` | 3 | Bone broken | `PART_ENTANGLED` | 10 | Entangled |
| `PART_SCARRED` | 4 | Scarring | `PART_BRUISED` | 11 | Bruised |
| `PART_BANDAGED` | 5 | Bandaged | `PART_GANGRENOUS` | 12 | Gangrene |
| `PART_MISSING` | 6 | Severed/missing | | | |

### Limb Health API

```cpp
// Health manipulation
unsigned short getCurLimbHealth(wearSlotT) const;
void setCurLimbHealth(wearSlotT, unsigned short);
unsigned short getMaxLimbHealth(wearSlotT) const;

// Flag manipulation
unsigned short getLimbFlags(wearSlotT) const;
void addToLimbFlags(wearSlotT, unsigned short);
void remLimbFlags(wearSlotT, unsigned short);
bool isLimbFlags(wearSlotT, int) const;

// Queries
bool hasPart(wearSlotT) const;
bool canUseLimb(wearSlotT) const;
```

### break_bone()

The `break_bone(TBeing* ch, wearSlotT slot)` function in `code/code/misc/limbs.cc` adds `PART_BROKEN` to the limb flags if the character has the body part, doesn't have boneless anatomy, and the limb isn't already broken.

## Stuck Items

Items like arrows can become stuck in body parts during combat, tracked separately from equipped items via `stuckIn` in `bodyPartsDamage`.

```cpp
TThing* getStuckIn(wearSlotT limb) const;
void setStuckIn(wearSlotT limb, TThing* item);
int stickIn(TThing* o, wearSlotT pos, silentTypeT silent = SILENT_NO);
TThing* pulloutObj(wearSlotT numx, bool safe, int* res);
```

When stuck, the item's `stuckIn` pointer references the being and `eq_stuck` stores the slot. Pulling out stuck items may cause bleeding damage.

## Item Wear Flags

Items define valid wear slots via `obj_flags.wear_flags` (defined in `code/code/misc/obj.h`):

| Flag | Bit | Description | Flag | Bit | Description |
|------|-----|-------------|------|-----|-------------|
| `ITEM_WEAR_TAKE` | 0 | Can be picked up | `ITEM_WEAR_BACK` | 10 | Worn on back |
| `ITEM_WEAR_FINGERS` | 1 | Worn on fingers | `ITEM_WEAR_WAIST` | 11 | Worn on waist |
| `ITEM_WEAR_NECK` | 2 | Worn on neck | `ITEM_WEAR_WRISTS` | 12 | Worn on wrists |
| `ITEM_WEAR_BODY` | 3 | Worn on body | `ITEM_WEAR_HOLD` | 14 | Can be held |
| `ITEM_WEAR_HEAD` | 4 | Worn on head | `ITEM_WEAR_THROW` | 15 | Can be thrown |
| `ITEM_WEAR_LEGS` | 5 | Worn on legs | | | |
| `ITEM_WEAR_FEET` | 6 | Worn on feet | | | |
| `ITEM_WEAR_HANDS` | 7 | Worn on hands | | | |
| `ITEM_WEAR_ARMS` | 8 | Worn on arms | | | |

## Equipment Affects

When equipment is worn, magical affects are applied via `TBeing::affectModify()`, which modifies character statistics based on the item's `affected[]` array and `bitvector` flags.

### equipChar Flow

1. Validates slot range and availability
2. Checks shield placement (cannot be primary hand)
3. Verifies hand functionality for held items
4. Sets `obj->equippedBy` and `obj->eq_pos`
5. Applies item affects via `affectModify()` for each `MAX_OBJ_AFFECT` entry
6. Updates character's light level

### unequip Flow

1. Removes affects via `affectModify()` with `add=false`
2. Handles paired items (legs, holds) occupying two slots
3. Clears `equippedBy` and `eq_pos` references
4. Returns the removed item

## Handedness

Characters have a dominant hand via `isRightHanded()`. Helper methods return appropriate slots:

```cpp
wearSlotT getPrimaryHold() const;     wearSlotT getSecondaryHold() const;
wearSlotT getPrimaryHand() const;     wearSlotT getSecondaryHand() const;
wearSlotT getPrimaryArm() const;      wearSlotT getSecondaryArm() const;
```

Characters with DEX > 180 or Hobbit race are ambidextrous.

## Related Files

| File | Contents |
|------|----------|
| `code/code/misc/limbs.h` | wearSlotT enum, function declarations |
| `code/code/misc/limbs.cc` | Limb manipulation functions |
| `code/code/misc/being.h` | TBeing class, bodyPartsDamage, equipmentData |
| `code/code/misc/defs.h` | PART_* flag constants |
| `code/code/misc/obj.h` | ITEM_WEAR_* flag constants |
| `code/code/sys/handler.cc` | equipChar(), unequip(), affectModify() |
| `code/code/misc/range.cc` | stickIn() for stuck items |
