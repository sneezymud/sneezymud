---
title: Snoop, Switch, and Return System
description: Immortal commands for monitoring player I/O and temporarily controlling mobs through descriptor pointer manipulation
keywords: [player monitoring, character control, descriptor manipulation, body swap, transformation mechanics]
category: critical
source_files: [code/code/misc/immortal.cc, code/code/sys/connect.h, code/code/sys/connect.cc, code/code/misc/limits.cc]
primary_symbols:
  functions: [doSnoop, doSwitch, doReturn, outputProcessing, SwitchStuff, limitPowerCheck]
  classes: [Descriptor, TBeing, TPerson, TMonster]
  enums: [POWER_SNOOP, POWER_SWITCH, ACT_POLYSELF, POLY_TYPE_NONE, POLY_TYPE_SWITCH, POLY_TYPE_DISGUISE, POLY_TYPE_SHAPESHIFT, POLY_TYPE_POLYMORPH, PLR_NOSNOOP, TOG_TRANSFORMED_LYCANTHROPE, CMD_SWITCH, SPEC_SHOPKEEPER, SPEC_NEWBIE_EQUIPPER]
---

# Snoop, Switch, and Return System

## Overview

Three interconnected immortal commands manage character observation and control. Snoop monitors another character's input/output streams. Switch transfers your descriptor to take control of a mob. Return restores control to your original character.

These commands manipulate descriptor pointers to redirect game I/O and character control. The system shares architecture with polymorph spells but has distinct safety requirements, particularly around the `ACT_POLYSELF` flag which distinguishes admin switch from spell-based transformation.

## Patterns

**Descriptor Validation**
- Always validate `desc` and `desc->original` before return operations
- Never assume descriptor pointers are non-NULL after any system transition

**Snoop/Switch Isolation**
- Never mix snoop and switch operations
- Always check `desc->snoop.snoop_by` and `desc->snoop.snooping` before switching
- Never snoop switched characters (those with `desc->original` set)

**Switch Safety**
- Never switch while already switched (check `desc->original`)
- Never switch into occupied bodies (check `tBeing->desc`)
- Never switch into player characters (use `dynamic_cast<TPerson*>` check)
- Always clear original body's descriptor pointer after switch
- Always set `polyed = POLY_TYPE_SWITCH` on the original body

**Return and Deletion**
- Always call `REMOVE_BIT(specials.act, ACT_POLYSELF)` before deleting polymorphed mobs
- Always use `doReturn()` before deleting switched bodies directly
- Never delete switched bodies without returning the descriptor first
- Never allow manual return for forced transformations (check `hasQuestBit(TOG_TRANSFORMED_LYCANTHROPE)`)

**Permission Checks**
- Always verify `powerCheck(POWER_SNOOP)` before snooping
- Always use `limitPowerCheck(CMD_SWITCH, mob_vnum)` before switching
- Never snoop equal or higher level immortals

## Reference

### Descriptor States

| State | desc->character | desc->original | desc->snoop |
|-------|-----------------|----------------|-------------|
| Normal | player | NULL | {NULL, NULL} |
| Snooped | unchanged | unchanged | snoop_by set |
| Switched | mob | player | must be clear |
| Polymorphed | mob + ACT_POLYSELF | player | must be clear |

### Polymorph Type Constants

| Constant | Usage |
|----------|-------|
| `POLY_TYPE_NONE` | Normal state, not switched/polymorphed |
| `POLY_TYPE_SWITCH` | Admin switch (no ACT_POLYSELF) |
| `POLY_TYPE_DISGUISE` | Disguise skill transformation |
| `POLY_TYPE_SHAPESHIFT` | Shapeshift spell |
| `POLY_TYPE_POLYMORPH` | Polymorph spell |

### Switch vs Polymorph Comparison

| Aspect | Admin Switch | Spell Polymorph |
|--------|--------------|-----------------|
| ACT_POLYSELF flag | Not set | Set |
| Visual message | None | "turns liquid, and reforms" |
| Stat transfer | None | `SwitchStuff()` called |
| Affect removal | None | Removes transformation affects |
| Mob cleanup on return | None | Delete or move to POLY_STORAGE |

### Snoop Restrictions

| Check | Reason |
|-------|--------|
| POWER_SNOOP required | Wizard power gate |
| One target at a time | `desc->snoop.snooping` must be NULL |
| Cannot snoop self | Silent success returned |
| Level hierarchy | Cannot snoop equal/higher level |
| PLR_NOSNOOP flag | Player opt-out respected |
| desc->original check | Cannot snoop switched characters |

### Switch Restrictions

| Check | Reason |
|-------|--------|
| Cannot switch into self | Meaningless operation |
| No active snoop | Snoop pointers would dangle |
| Target has no descriptor | Body already in use |
| Target not a TPerson | Cannot possess players |
| Not already switched | Nested switch breaks restoration chain |
| limitPowerCheck passes | Per-mob permission system |
| Not SPEC_SHOPKEEPER | Shopkeepers protected |
| Not SPEC_NEWBIE_EQUIPPER | Newbie helpers protected |

### doReturn() Parameters

| Parameter | Purpose |
|-----------|---------|
| `argument` | Optional limb name for limb transformation return |
| `limb` | Specific limb slot for transformation reversal |
| `tell` | If true, display transformation messages |
| `deleteMob` | If true, delete polymorph form; if false, move to POLY_STORAGE |

### Key Functions

| Function | File | Purpose |
|----------|------|---------|
| `doSnoop()` | immortal.cc | Establish snoop link |
| `doSwitch()` | immortal.cc | Transfer descriptor to mob |
| `doReturn()` | immortal.cc | Restore original body control |
| `outputProcessing()` | connect.cc | Duplicate output to snooper |
| `SwitchStuff()` | immortal.cc | Transfer stats for polymorph |

## Implementation

### Descriptor Pointer Architecture

The `Descriptor` class uses three key pointers for this system:
- `character`: Current active character (may be switched mob)
- `original`: Original player (NULL if not switched)
- `snoop`: Structure tracking bidirectional snoop links

When switched, the original player loses their descriptor (`desc = NULL`), and the mob receives it with `original` pointing back to the player.

The `Descriptor` destructor automatically breaks snoop chains by clearing both `snoop_by` and `snooping` pointers, preventing dangling pointers when either party disconnects.

### Snoop Output Flow

When a snooped character receives output, `outputProcessing()` checks `snoop.snoop_by` and duplicates the text to the snooper's output queue via `SnoopComm`. Snoop links break automatically when either party logs off.

Target lookup iterates `character_list` via `get_pc_world()` and `get_char_vis_world()` helper functions. The first matching character is selected.

### Switch Execution Sequence

1. Validate all safety conditions
2. Set `polyed = POLY_TYPE_SWITCH` on original
3. Set `desc->character = mob`
4. Set `desc->original = original_player`
5. Set `mob->desc = desc`
6. Set `original->desc = NULL`

For regular switch, `get_char_room()` searches the current room first. If not found, `get_char()` with `EXACT_YES` searches globally for exact name matches, then `EXACT_NO` for partial matches.

### Return Execution Sequence

1. Validate `desc` and `desc->original` exist
2. Check for forced transformation (`TOG_TRANSFORMED_LYCANTHROPE`) and reject if set
3. For polymorph (ACT_POLYSELF set): move original body, call `SwitchStuff()`, remove affects
4. Restore descriptor: `original->desc = desc`, `desc->character = original`
5. Clear both `desc->original` and local `desc` pointers
6. Set `original->polyed = POLY_TYPE_NONE`
7. For polymorph only: `REMOVE_BIT(specials.act, ACT_POLYSELF)` then delete or store mob

### Switch Load

For `switch load <name>`, the command searches `mob_index` by name, creates the mob with `read_mobile()`, and places it in the current room. The mob's `oldRoom` field stores the creation room.

If `Config::LoadOnDeath` returns false, `createWealth()` generates starting equipment and money. If true, the mob spawns empty and only generates loot when killed.

### Special Object Switch

Non-immortals can use switch if wearing a special necklace (spec == 139). This enables quest items to grant temporary switch ability.

### Idle Timeout Integration

The idle timeout system handles switched characters specially. Admin switch does NOT trigger stat transfer on idle. Polymorph forms transfer stats via `SwitchStuff()` before returning.

### Death Handling

When a switched mob dies, `doReturn()` is called automatically with `deleteMob=false`. The immortal survives and returns to their original body while the mob dies normally.

For spell polymorph deaths, `ACT_POLYSELF` is set, so `doReturn()` performs stat transfer and returns the player to their original body before death processing.

## Troubleshooting

**Crash: NULL pointer dereference in return**
- Cause: Called `doReturn()` without validating `desc->original`
- Fix: Always check `if (!desc || !desc->original)` before accessing

**Crash: Heap-use-after-free on polymorphed mob deletion**
- Cause: Deleted mob with `ACT_POLYSELF` still set
- Fix: Call `REMOVE_BIT(specials.act, ACT_POLYSELF)` before delete

**Crash: Dangling snoop pointers after switch**
- Cause: Switched while snooping or being snooped
- Fix: Check `desc->snoop.snoop_by` and `desc->snoop.snooping` before switch

**Bug: Lost descriptor after direct mob deletion**
- Cause: Deleted switched mob body without calling `doReturn()` first
- Fix: Always call `doReturn()` before deleting any mob with `desc->original` set

**Bug: Nested switch corruption**
- Cause: Attempted switch while `desc->original` already set
- Fix: Check for existing switch and reject

**Bug: Stats not restored after admin switch**
- Cause: Expected stat transfer that only happens with polymorph
- Fix: Admin switch intentionally preserves mob stats; use polymorph for stat transfer

**Bug: Snoop output not appearing**
- Cause: Target is in switched state (`desc->original` set)
- Fix: Cannot snoop switched characters; wait for return

**Bug: Immortal cannot switch into specific mob**
- Cause: `limitPowerCheck()` rejects the vnum based on database configuration
- Fix: Verify immortal's access level in database; check wizard power settings
