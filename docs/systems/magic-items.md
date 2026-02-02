---
title: Magical Items and Enchantment System
description: Scrolls, wands, and staves store spells for on-demand casting without mana cost.
created_by_model: opus
---

# Magical Items and Enchantment System

## Overview

Magical items let any character use stored spells by consuming charges instead of mana. Scrolls hold up to three spells and are destroyed on use. Wands and staves are rechargeable; wands require explicit targeting while staves hit the entire room automatically. All spell execution routes through a central dispatcher that returns DELETE flags when targets die.

## Patterns

### Charge Management

- Always check `getCurCharges() > 0` before consuming a charge.
- Always consume the charge (`addToCurCharges(-1)`) before casting, not after.
- Never sell wands unless `curCharges == maxCharges`.

### DELETE Flag Handling

- Always use `IS_SET_DELETE()` for DELETE flags, never `IS_SET()`.
- Always check `DELETE_VICT` after spell execution and delete the victim if set (when you own the pointer).
- Always propagate `DELETE_THIS` to the caller when set.
- Always set pointers to `NULL` after deletion.
- Never continue accessing a target after a DELETE flag indicates its death.

### Iterator Safety for Staves

- Always advance the iterator before spell execution: `TThing* t = *(it++);`.
- Never use post-loop increment (`++it`) when deletions may occur inside the loop body.

### Scroll Protection

- Always call `setLocked(true)` before spell execution and `setLocked(false)` after.
- Never access scroll data after spell execution without checking if the scroll survived.

### Spell Validation

- Always verify `spell >= MIN_SPELL && spell < MAX_SKILL` before use.
- Always check `discArray[spell] != nullptr` before accessing spell properties.
- Always use `mapFileToSpellnum()` when loading and `mapSpellnumToFile()` when saving.

## Reference

### Item Types

| Type | Class | Charges | Targeting | Consumption |
|------|-------|---------|-----------|-------------|
| Scroll | `TScroll` | Single-use | Manual | Always destroyed |
| Wand | `TWand` | 1 per use | Player-specified | Rechargeable |
| Staff | `TStaff` | 1 per use | Automatic room-wide | Rechargeable |

### Value Field Layout

**TMagicItem (val1 bit-packing):**

| Bits | Field | Range |
|------|-------|-------|
| 0-7 | `magic_learnedness` | 0-100 |
| 8-15 | `magic_level` | 0-250+ |

**TScroll:**

| Field | Contents |
|-------|----------|
| val2 | Spell slot 0 (file-format ID) |
| val3 | Spell slot 1 (file-format ID) |
| val4 | Spell slot 2 (file-format ID) |

**TWand/TStaff:**

| Field | Contents |
|-------|----------|
| val2 | `maxCharges` |
| val3 | `curCharges` |
| val4 | Spell ID (file-format) |

### Apply Locations (applyTypeT)

| Location | Modifier Meaning |
|----------|------------------|
| `APPLY_STR/INT/WIS/DEX/CON` | +/- to stat |
| `APPLY_HIT/MANA/MOVE` | +/- to max pool |
| `APPLY_HITROLL` | +/- to hit accuracy |
| `APPLY_DAMROLL` | +/- to damage |
| `APPLY_ARMOR` | +/- to AC (negative is better) |
| `APPLY_IMMUNITY` | Type in modifier, amount in modifier2 |
| `APPLY_SPELL` | Spell ID in modifier, bonus in modifier2 |

### Bitvector Flags (AFF_*)

| Flag | Effect |
|------|--------|
| `AFF_INVISIBLE` | Character invisible |
| `AFF_DETECT_INVISIBLE` | See invisible |
| `AFF_SANCTUARY` | Reduced damage |
| `AFF_FLYING` | Can fly |
| `AFF_INFRAVISION` | Darkvision |
| `AFF_WATERBREATH` | Breathe underwater |
| `AFF_SNEAK` | Move silently |

### Lag Tiers

| Constant | Rounds | Seconds |
|----------|--------|---------|
| `LAG_0` | 0 | 0.0 |
| `LAG_1` | 1 | 1.2 |
| `LAG_2` | 2 | 2.4 |
| `LAG_3` | 3 | 3.6 |
| `LAG_4` | 4 | 4.8 |
| `LAG_5+` | 5+ | 6.0+ |

### Source Files

| File | Purpose |
|------|---------|
| `obj/obj_magic_item.h/.cc` | TMagicItem base class |
| `obj/obj_scroll.h/.cc` | TScroll implementation |
| `obj/obj_wand.h/.cc` | TWand implementation |
| `obj/obj_staff.h/.cc` | TStaff implementation |
| `misc/other.cc` | `doObjSpell()` dispatcher, `reciteMe()` |
| `sys/handler.cc` | `equipChar()`, `unequip()`, `affectModify()` |
| `misc/structs.h` | `affectedData` structure |
| `misc/spells.h` | `spellNumT` enumeration |
| `misc/spell2.h` | `spellInfo`, `discArray` |

## Implementation

### Class Hierarchy

`TMagicItem` inherits virtually from `TObj` and provides `magic_level` (spell power) and `magic_learnedness` (crafting quality). Three concrete classes inherit from it:

- `TScroll`: Stores three `spellNumT` values in a fixed array. The `reciteMe()` method casts all three spells sequentially, always returning `DELETE_THIS` since scrolls are consumed.

- `TWand`: Stores a single spell plus `maxCharges`/`curCharges`. The `useMe()` method builds a targeting bitmask from the spell's `TAR_*` flags, calls `generic_find()` to locate the target, then dispatches via `doObjSpell()`.

- `TStaff`: Same charge structure as wands. The `useMe()` method checks `TAR_AREA`; if set, it calls `doObjSpell()` once with a null victim. Otherwise, it iterates through `roomp->stuff`, skipping the caster and group members for violent spells, calling `doObjSpell()` for each valid target.

### Spell Dispatch

`doObjSpell()` in `misc/other.cc` is a large switch statement routing `spellNumT` values to their implementations. It first checks peaceful room restrictions for `TAR_VIOLENT` spells, then passes `obj->getMagicLevel()` as the spell power parameter to the underlying spell function.

### Equipment Affects

Objects store up to `MAX_OBJ_AFFECT` (5) affects in the `affected[]` array. Each `objAffData` entry specifies an `applyTypeT` location, a modifier value, and optional bitvector flags.

When `equipChar()` places an item, it calls `affectModify()` for each affect with the apply flag set to `TRUE`, adding the modifier to the character's stat and setting bitvector flags. When `unequip()` removes an item, it calls `affectModify()` with `FALSE` to reverse the changes. Multiple equipped items' affects stack additively.

### Scroll Lock Mechanism

Scrolls call `setLocked(true)` before spell execution to prevent the scroll from being deleted by side effects during the spell. After spell completion, `setLocked(false)` is called. The `reciteMe()` method checks `DELETE_VICT` after each spell and stops the loop early if the victim dies, but still returns `DELETE_THIS` since scrolls are always consumed.

### Staff Room Iteration

Staff iteration uses the pattern `TThing* t = *(it++)` to cache the current element and advance the iterator in one operation. This allows safe deletion of `t` after spell execution since the iterator already points to the next element. The loop skips beings matching `tmp_char == ch` (the caster) or `tmp_char->inGroup(*ch)` for violent spells.

## Troubleshooting

### Symptom: Crash after spell kills target

**Cause:** Code continues using victim pointer after `DELETE_VICT` flag set.

**Fix:** Check `IS_SET_DELETE(rc, DELETE_VICT)` immediately after spell execution. If set and you own the pointer, delete and null it, then return or break.

### Symptom: Crash during staff room iteration

**Cause:** Iterator invalidated by deleting the element it points to.

**Fix:** Use `TThing* t = *(it++)` to advance iterator before accessing the element.

### Symptom: Wand charges go negative

**Cause:** Missing charge validation before consumption.

**Fix:** Check `getCurCharges() > 0` before calling `addToCurCharges(-1)`.

### Symptom: DELETE flags never detected

**Cause:** Using `IS_SET()` instead of `IS_SET_DELETE()`.

**Fix:** DELETE flags use a special bit pattern requiring `IS_SET_DELETE()`.

### Symptom: Scroll deleted mid-spell causes crash

**Cause:** Missing lock protection around spell execution.

**Fix:** Call `setLocked(true)` before and `setLocked(false)` after `doObjSpell()`.

### Symptom: Invalid spell cast from scroll/wand

**Cause:** Spell ID not validated or not converted from file format.

**Fix:** Use `mapFileToSpellnum()` when loading, validate against `MIN_SPELL`/`MAX_SKILL` range, and check `discArray[spell]` is not null.

### Symptom: Item affects not applying

**Cause:** `affectModify()` not called during equip, or affects stored in wrong array indices.

**Fix:** Verify `equipChar()` iterates all `MAX_OBJ_AFFECT` slots and calls `affectModify()` with `TRUE`.
