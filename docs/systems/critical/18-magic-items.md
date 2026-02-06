---
title: Magical Items and Enchantment System
description: Scrolls, wands, and staves store spells for on-demand casting without mana cost.
category: critical
keywords: [scrolls, wands, staves, enchantment, charges]
primary_symbols:
  functions: [reciteMe, useMe, doObjSpell, equipChar, unequip, affectModify, generic_find]
  classes: [TMagicItem, TScroll, TWand, TStaff, affectedData, objAffData]
  enums: [spellNumT, applyTypeT, APPLY_STR, APPLY_INT, APPLY_WIS, APPLY_DEX, APPLY_CON, APPLY_HIT, APPLY_MANA, APPLY_MOVE, APPLY_HITROLL, APPLY_DAMROLL, APPLY_ARMOR, APPLY_IMMUNITY, APPLY_SPELL, APPLY_NONE, AFF_INVISIBLE, AFF_DETECT_INVISIBLE, AFF_SANCTUARY, AFF_FLYING, AFF_INFRAVISION, AFF_WATERBREATH, AFF_SNEAK, AFF_DETECT_MAGIC, DELETE_THIS, DELETE_VICT, DELETE_ITEM, TAR_IGNORE, TAR_CHAR_ROOM, TAR_OBJ_INV, TAR_OBJ_ROOM, TAR_OBJ_EQUIP, TAR_AREA, TAR_VIOLENT, FIND_CHAR_ROOM, FIND_OBJ_INV, FIND_OBJ_ROOM, FIND_OBJ_EQUIP, MIN_SPELL, MAX_SPELL, MAX_SKILL, TYPE_UNDEFINED, SKILL_READ_MAGIC, MAX_OBJ_AFFECT]
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
- Scrolls are consumed whether recitation succeeds or fails; `reciteMe()` always returns `DELETE_THIS`.

### Spell Validation

- Always verify `spell >= MIN_SPELL && spell < MAX_SKILL` before use.
- Always check `discArray[spell] != nullptr` before accessing spell properties.
- Always use `mapFileToSpellnum()` when loading and `mapSpellnumToFile()` when saving.

### Wand Targeting

- Always build a bitmask from the spell's `TAR_*` flags to pass to `generic_find()`.
- Always reject `TAR_IGNORE` spells that cannot be targeted.
- Always check `DELETE_ITEM` in addition to `DELETE_VICT` and `DELETE_THIS` after spell execution.

### Staff Area Effects

- For `TAR_AREA` spells, call `doObjSpell()` once with null victim and target parameters.
- For non-area spells, iterate through `roomp->stuff`, skipping caster and group members for violent spells.

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
| 0-7 | `magic_level` | 0-250+ |
| 8-15 | `magic_learnedness` | 0-100 |

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
| `APPLY_NONE` | No modification |

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
| `AFF_DETECT_MAGIC` | Reveals magical auras |

### DELETE Flag Types

| Flag | Meaning |
|------|---------|
| `DELETE_THIS` | Magic item should be deleted |
| `DELETE_VICT` | Spell victim died and should be deleted |
| `DELETE_ITEM` | Target object should be deleted |

Use `IS_SET_DELETE()` for these flags; standard `IS_SET()` will not detect the combined bit pattern.

### Spell Targeting Flags (TAR_*)

| Flag | Meaning |
|------|---------|
| `TAR_IGNORE` | No targeting requirements |
| `TAR_CHAR_ROOM` | Target being in same room |
| `TAR_OBJ_INV` | Target object in inventory |
| `TAR_OBJ_ROOM` | Target object in room |
| `TAR_OBJ_EQUIP` | Target equipped object |
| `TAR_AREA` | Room-wide area effect |
| `TAR_VIOLENT` | Triggers peaceful room check, skips group members |

### Generic Find Bitmask (FIND_*)

| Flag | Search Location |
|------|-----------------|
| `FIND_CHAR_ROOM` | Beings in room |
| `FIND_OBJ_INV` | Objects in inventory |
| `FIND_OBJ_ROOM` | Objects in room |
| `FIND_OBJ_EQUIP` | Equipped objects |

### spellNumT Range

| Constant | Meaning |
|----------|---------|
| `TYPE_UNDEFINED` (-1) | Invalid spell |
| `MIN_SPELL` (= `SPELL_GUST` = 0) | First valid spell |
| `MAX_SPELL` | End of spells, start of skills |
| `MAX_SKILL` | End of valid range |

### Charge Functions

| Function | Purpose |
|----------|---------|
| `getCurCharges()` | Current remaining charges |
| `getMaxCharges()` | Maximum capacity |
| `setMaxCharges(n)` | Set maximum (initialization only) |
| `addToCurCharges(n)` | Add (positive) or consume (negative), clamped to valid range |

### affectedData Structure

| Field | Contents |
|-------|----------|
| `type` | Affect spell/skill ID |
| `level` | Caster level or intensity |
| `duration` | Remaining ticks; -9 = permanent |
| `modifier` | Primary effect value |
| `modifier2` | Secondary effect value |
| `location` | `applyTypeT` enum value |
| `bitvector` | Character `AFF_*` flags |
| `be` | Associated being pointer |
| `next` | Linked list chaining |

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

### Scroll Execution Flow

`reciteMe()` first checks `ch->bSuccess(SKILL_READ_MAGIC)` to determine if the caster can successfully read the scroll. If the check fails, it sends a failure message and returns `DELETE_THIS`. If successful, it calls `generic_find()` to locate the target, then iterates through all three spell slots. It tracks the maximum spell lag from all executed spells and applies it once at the end via `ch->addToWait(combatRound(max_lag + 2))`.

### Staff Room Iteration

Staff iteration uses the pattern `TThing* t = *(it++)` to cache the current element and advance the iterator in one operation. This allows safe deletion of `t` after spell execution since the iterator already points to the next element. The loop skips beings matching `tmp_char == ch` (the caster) or `tmp_char->inGroup(*ch)` for violent spells.

### Spell Database Conversion

`mapFileToSpellnum()` accepts an integer from the database and returns the corresponding `spellNumT` by indexing into a conversion table. `mapSpellnumToFile()` accepts a `spellNumT` and returns the integer for database storage. These functions handle the historical mismatch between file format spell numbering and the current enum order.

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

### Symptom: Wand cannot be sold

**Cause:** Shop code requires `getCurCharges() == getMaxCharges()` for wands and staves.

**Fix:** Recharge the wand to full capacity before selling.

### Symptom: DELETE_THIS not propagated

**Cause:** Caller returns `TRUE` instead of `DELETE_THIS` after detecting the flag.

**Fix:** Immediately return `DELETE_THIS` after detecting `IS_SET_DELETE(rc, DELETE_THIS)`. Never perform cleanup or message sending after this flag is set.

### Symptom: Invalid spell array index crash

**Cause:** Accessing `scroll->getSpell(i)` with `i >= 3` exceeds the three-element array bounds.

**Fix:** Hardcode the loop limit to 3 for scrolls and validate the spell ID returned by `getSpell()`.

### Symptom: Affects not removed on unequip

**Cause:** `unequip()` fails to call `affectModify()` with `FALSE` for each affect.

**Fix:** Ensure `unequip()` iterates through all `MAX_OBJ_AFFECT` slots and calls `affectModify()` to subtract modifiers and clear bitvector flags.

### Symptom: Missing spell lag

**Cause:** Magic item usage returns without calling `ch->addToWait()`.

**Fix:** Call `ch->addToWait(combatRound(discArray[spell]->lag))` before every return path in `useMe()` and `reciteMe()`.
