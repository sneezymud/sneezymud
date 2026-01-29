---
title: Snoop, Switch, and Return System
description: The snoop, switch, and return commands allow immortals to monitor player activities and temporarily control mobs through descriptor pointer manipulation and safe body swapping mechanisms.
keywords:
  - snoop
  - switch
  - return
  - descriptor pointers
  - desc original
  - polymorph
  - POLY_TYPE_SWITCH
  - ACT_POLYSELF
  - body swapping
  - immortal commands
  - doSnoop
  - doSwitch
  - doReturn
  - Descriptor
category: Critical Systems

  - delete-flags.md
  - network-architecture.md
  - wizard-powers.md
last_updated: 2026-01-29
source_files:
  - code/code/misc/immortal.cc
  - code/code/sys/connect.h
  - code/code/sys/connect.cc
  - code/code/misc/limits.cc
  - code/code/misc/combat.cc
related: [memory-safety.md]
---

# Snoop, Switch, and Return System

The snoop, switch, and return commands allow immortals to monitor player activities and temporarily control mobs. Understanding this system is critical for avoiding crashes related to descriptor pointer management and safe body swapping.

**Misusing this system causes crashes.** Common errors: switching while snooped, not checking desc->original before return, failing to clear ACT_POLYSELF before deletion, accessing invalidated descriptor pointers.

## Overview

Three interconnected immortal commands manage character observation and control:

- **Snoop**: Monitor another character's input/output streams
- **Switch**: Take control of a mob by transferring your descriptor
- **Return**: Restore control to your original character

These commands manipulate descriptor pointers to redirect game I/O and character control. The system shares architecture with the polymorph spell system but has distinct safety requirements.

## Descriptor Pointer Architecture

The `Descriptor` class (connect.h) uses three key pointers:

```cpp
class Descriptor {
  public:
    TBeing* character;    // Current active character (may be switched mob)
    TPerson* original;    // Original player (NULL if not switched)
    snoopData snoop;      // Snoop chain tracking
};
```

### Normal State

```
Player logged in normally:
  desc->character = player (TPerson)
  desc->original = NULL
  desc->snoop = {NULL, NULL}
```

### Snooped State

```
Immortal snooping player:
  player->desc->snoop.snoop_by = immortal->desc
  immortal->desc->snoop.snooping = player->desc
```

### Switched State

```
Immortal switched into mob:
  immortal->desc = NULL
  mob->desc->character = mob (TMonster)
  mob->desc->original = immortal (TPerson)
  immortal->polyed = POLY_TYPE_SWITCH
```

### Polymorph State (for comparison)

```
Player polymorphed into mob:
  player->desc = NULL
  mob->desc->character = mob (TMonster)
  mob->desc->original = player (TPerson)
  player->polyed = POLY_TYPE_DISGUISE/SHAPESHIFT/POLYMORPH
  mob->specials.act |= ACT_POLYSELF
```

The key difference: admin switch does NOT set `ACT_POLYSELF`, while spell-based polymorph does.

## doSnoop() - Input/Output Monitoring

**Source:** `code/code/misc/immortal.cc`

The snoop command allows immortals to see another character's input and output streams.

### Implementation Flow

```cpp
void TPerson::doSnoop(const sstring& arg) {
  char arg1[MAX_INPUT_LENGTH];
  Descriptor* d;

  // Validate wizard power
  if (!hasWizPower(POWER_SNOOP)) {
    sendTo("You can't do that.\n\r");
    return;
  }

  // Parse target name
  strcpy(arg1, arg.c_str());
  if (!*arg1) {
    sendTo("Snoop whom?\n\r");
    return;
  }

  // Check if already snooping someone
  if (desc->snoop.snooping) {
    sendTo("You are already snooping someone.\n\r");
    return;
  }

  // Find target descriptor by character name
  for (d = descriptor_list; d; d = d->next)
    if (!d->account && d->character && isname(arg1, d->character->name))
      break;

  if (!d) {
    sendTo("No such player around.\n\r");
    return;
  }

  // Cannot snoop self
  if (d == desc) {
    sendTo("Ok.\n\r");  // Pretend it worked
    return;
  }

  // Cannot snoop higher or equal level
  if (GetMaxLevel() <= d->character->GetMaxLevel()) {
    sendTo("You failed.\n\r");
    return;
  }

  // Respect PLR_NOSNOOP flag
  if (d->character->isPlayerAction(PLR_NOSNOOP)) {
    sendTo("You failed.\n\r");
    return;
  }

  // Cannot snoop a switched character
  if (d->original) {
    sendTo("Mixing snoop and switch is bad for your health.\n\r");
    return;
  }

  // Success - establish snoop link
  sendTo(format("Ok, you are now snooping %s.\n\r") %
         d->character->getName());

  desc->snoop.snooping = d;
  d->snoop.snoop_by = desc;
}
```

### Snoop Output Flow

When the snooped character receives output, it is duplicated to the snooper:

```cpp
// In outputProcessing() (connect.cc)
if (snoop.snoop_by && snoop.snoop_by->desc) {
  snoop.snoop_by->desc->output.push(
    CommPtr(new SnoopComm(ch->getName(), text)));
}
```

### Snoop Restrictions

| Check | Reason |
|-------|--------|
| `!hasWizPower(POWER_SNOOP)` | Requires POWER_SNOOP wizard power |
| `desc->snoop.snooping` | Can only snoop one target at a time |
| `d == desc` | Cannot snoop self (silent success) |
| `GetMaxLevel() <= d->character->GetMaxLevel()` | Cannot snoop equal/higher level immortals |
| `d->character->isPlayerAction(PLR_NOSNOOP)` | Player opted out of snooping |
| `d->original` | Cannot snoop switched characters |

### Breaking Snoop Links

Snoop links are automatically broken when:
- Snooper logs off (Descriptor destructor)
- Target logs off (Descriptor destructor)
- Target switches into another body

Manual break via: `snoop` with no argument (not implemented in shown code).

## doSwitch() - Body Transfer

**Source:** `code/code/misc/immortal.cc`

The switch command allows immortals to take control of a mob.

### Command Syntax

```
switch <mob_name>          Load existing mob in room or world
switch load <mob_vnum>     Create new mob and switch into it
```

### Implementation Flow

```cpp
void TPerson::doSwitch(const sstring& arg) {
  sstring tStMobile, tStBuffer;
  TBeing* tBeing;
  bool doLoadCmd = false;
  int mobileIndex;

  // Special object switch: equipment[WEAR_NECK] with spec == 139
  bool hasSwiO = false;
  if (equipment[WEAR_NECK] && equipment[WEAR_NECK]->spec == 139)
    hasSwiO = true;

  // Parse "switch load <vnum>" syntax
  if (is_abbrev(arg.word(0), "load")) {
    doLoadCmd = true;
    tStBuffer = arg.word(1);
  }
  tStMobile = (doLoadCmd ? tStBuffer : arg);

  // === LOAD MODE: Create new mob ===
  if (doLoadCmd) {
    // Find mob by name in mob_index
    for (mobileIndex = 0; mobileIndex < (signed int)mob_index.size();
         mobileIndex++)
      if (isname(tStBuffer, mob_index[mobileIndex].name))
        break;

    if (mobileIndex >= (signed int)mob_index.size()) {
      sendTo("Could not find that mobile.  Sorry.\n\r");
      return;
    }

    // Cannot load shopkeepers
    if (mob_index[mobileIndex].spec == SPEC_SHOPKEEPER) {
      sendTo("You cannot load a shopkeeper this way.\n\r");
      return;
    }

    // Cannot load newbie helpers
    if (mob_index[mobileIndex].spec == SPEC_NEWBIE_EQUIPPER) {
      sendTo("You cannot load a newbieHelper this way.\n\r");
      return;
    }

    // Create mob from prototype
    if (!(tBeing = read_mobile(mobileIndex, REAL))) {
      sendTo("Well, um, that doesn't seem to exist.  Sorry.\n\r");
      return;
    }

    // Permission check via limitPowerCheck()
    if ((isImmortal() && !limitPowerCheck(CMD_SWITCH, tBeing->number)) ||
        (!isImmortal() && !hasSwiO)) {
      sendTo("You're not allowed to switch/load that mobile.\n\r");
      return;
    }

    // Place mob in current room
    *roomp += *tBeing;
    (dynamic_cast<TMonster*>(tBeing))->oldRoom = inRoom();

    // Create starting wealth if configured
    if (!Config::LoadOnDeath())
      (dynamic_cast<TMonster*>(tBeing))->createWealth();
  }

  // === REGULAR MODE: Find existing mob ===
  if (!(tBeing = get_char_room(tStMobile, in_room))) {
    sendTo("No one in room with that name......searching world.\n\r");
    if (!(tBeing = get_char(tStMobile.c_str(), EXACT_YES)) &&
        !(tBeing = get_char(tStMobile.c_str(), EXACT_NO))) {
      sendTo("No one with that name found in world, sorry.\n\r");
      return;
    }
  }

  // Permission check
  if ((isImmortal() && !limitPowerCheck(CMD_SWITCH, tBeing->number)) ||
      (!isImmortal() && !hasSwiO)) {
    sendTo("You're not allowed to switch into that mobile.\n\r");
    return;
  }

  // === SAFETY CHECKS ===

  // Cannot switch into self
  if (this == tBeing) {
    sendTo("Heh heh heh...we are jolly funny today, aren't we?\n\r");
    return;
  }

  // Cannot switch while snooping or being snooped
  if (!desc || desc->snoop.snoop_by || desc->snoop.snooping) {
    sendTo("Mixing snoop & switch is bad for your health.\n\r");
    return;
  }

  // Cannot switch into PCs or already-used bodies
  if (tBeing->desc || dynamic_cast<TPerson*>(tBeing)) {
    sendTo("You can't do that; the body is already in use!\n\r");
    return;
  }

  // Cannot switch while already switched
  if (desc->original) {
    sendTo("You already seem to be switched.\n\r");
    return;
  }

  // === PERFORM SWITCH ===

  if (doLoadCmd)
    act(msgVariables(MSG_SWITCH_TARG, tBeing), FALSE, this, NULL, NULL,
      TO_ROOM);

  sendTo("Ok.\n\r");

  // Mark original character as switched
  polyed = POLY_TYPE_SWITCH;

  // Transfer descriptor to mob
  desc->character = tBeing;
  desc->original = this;

  tBeing->desc = desc;
  desc = NULL;  // Original character loses descriptor
}
```

### Switch Safety Checks

| Check | Reason |
|-------|--------|
| `this == tBeing` | Cannot switch into self |
| `!desc` or `desc->snoop.snoop_by` or `desc->snoop.snooping` | Mixing snoop and switch is dangerous |
| `tBeing->desc` | Target body already has a descriptor (in use) |
| `dynamic_cast<TPerson*>(tBeing)` | Cannot switch into player characters |
| `desc->original` | Already switched (nested switch forbidden) |
| `!limitPowerCheck(CMD_SWITCH, tBeing->number)` | Insufficient permissions for this mob |
| `mob_index[].spec == SPEC_SHOPKEEPER` | Cannot load shopkeepers |
| `mob_index[].spec == SPEC_NEWBIE_EQUIPPER` | Cannot load newbie helpers |

### Special Object Switch

Non-immortals can use switch if they have a special object:
```cpp
// equipment[WEAR_NECK] with spec == 139 enables switch
bool hasSwiO = false;
if (equipment[WEAR_NECK] && equipment[WEAR_NECK]->spec == 139)
  hasSwiO = true;
```

This allows quest items or special events to grant temporary switch ability.

## doReturn() - Body Restoration

**Source:** `code/code/misc/immortal.cc`

The return command restores control to the original character.

### Implementation Flow

```cpp
void TBeing::doReturn(const sstring& argument, wearSlotT limb, bool tell,
  bool deleteMob) {

  // Block lycanthrope transformation return
  if (hasQuestBit(TOG_TRANSFORMED_LYCANTHROPE)) {
    sendTo("You can't do that of your own free will!\n\r");
    return;
  }

  // Handle limb transformation return (separate system)
  if (!limb && hasTransformedLimb())
    for (const auto& limbType : TransformLimbList)
      if (is_abbrev(argument, limbType.name)) {
        limb = limbType.limb;
        break;
      }

  if (limb) {
    transformLimbsBack("", limb, true);
    return;
  }

  // Validate switched/polymorphed state
  if (!desc || !desc->original) {
    sendTo("What are you trying to return from?!\n\r");
    return;
  }

  sendTo("You return to your original body.\n\r");

  TBeing* originalBody = desc->original;

  // === POLYMORPH RETURN (ACT_POLYSELF set) ===
  // This path is for spell-based polymorph, NOT admin switch
  if (specials.act & ACT_POLYSELF) {
    if (tell)
      act("$n turns liquid, and reforms as $N.", true, this, nullptr,
        originalBody, TO_ROOM);

    // Move original body back to current location
    --(*originalBody);
    if (roomp)
      *roomp += *originalBody;
    else
      *(real_roomp(Room::CS)) += *originalBody;

    // Transfer stats and effects back
    SwitchStuff(this, originalBody);

    // Remove polymorph affects
    affectFrom(SPELL_POLYMORPH);
    affectFrom(SKILL_DISGUISE);
    affectFrom(SPELL_SHAPESHIFT);
  }

  // === DESCRIPTOR RESTORATION (both switch and polymorph) ===

  originalBody->desc = desc;
  originalBody->desc->character = desc->original;
  originalBody->desc->original = nullptr;
  desc->original = nullptr;
  desc = nullptr;
  originalBody->polyed = POLY_TYPE_NONE;

  // === CLEANUP POLYMORPHED MOB ===

  if (!IS_SET(specials.act, ACT_POLYSELF))
    return;  // Admin switch: no cleanup needed

  // Remove ACT_POLYSELF BEFORE deletion to avoid heap-use-after-free
  REMOVE_BIT(specials.act, ACT_POLYSELF);

  if (tell && deleteMob)
    delete this;  // Delete polymorph form
  else {
    --(*this);
    *(real_roomp(Room::POLY_STORAGE)) += *this;  // Store for recovery
  }
}
```

### Switch vs Polymorph Return

| Aspect | Admin Switch | Spell Polymorph |
|--------|--------------|-----------------|
| `ACT_POLYSELF` flag | NOT set | Set |
| Visual message | None | "turns liquid, and reforms" |
| Stat transfer | None (stats never swapped) | `SwitchStuff()` call |
| Affect removal | None | Removes SPELL_POLYMORPH, etc. |
| Mob cleanup | None | Deletes or moves to POLY_STORAGE |

The critical distinction: admin switch is **invisible** and **non-invasive**. It simply reassigns the descriptor without touching stats, equipment, or affects. Polymorph is a **transformation** that transfers stats and creates a temporary body.

### deleteMob Parameter

The `deleteMob` parameter (defaults to `true`) controls mob cleanup:

```cpp
doReturn("", WEAR_NOWHERE, true, false);  // Don't delete - move to storage
doReturn("", WEAR_NOWHERE, true, true);   // Delete the mob
```

**When to use `deleteMob=false`:**
- Called from scheduler process (let scheduler handle deletion)
- Want to preserve mob for debugging
- Mob has special items that should be recovered

**Crash prevention:** Always `REMOVE_BIT(specials.act, ACT_POLYSELF)` BEFORE deletion to avoid heap-use-after-free in destructors.

## Crash Prevention Patterns

### Pattern 1: Check desc->original Before Return

```cpp
// CORRECT: Validate state before attempting return
if (!desc || !desc->original) {
  sendTo("What are you trying to return from?!\n\r");
  return;
}

// CRASH: Assuming desc->original exists
TPerson* original = desc->original;  // May be NULL!
original->desc = desc;  // Crash if original was NULL
```

### Pattern 2: Clear ACT_POLYSELF Before Deletion

```cpp
// CORRECT: Remove flag before delete
REMOVE_BIT(specials.act, ACT_POLYSELF);
if (deleteMob)
  delete this;

// CRASH: Deleting with flag set
if (deleteMob)
  delete this;  // Destructors may check ACT_POLYSELF and crash
```

### Pattern 3: Never Mix Snoop and Switch

```cpp
// CORRECT: Check for snoop conflicts
if (!desc || desc->snoop.snoop_by || desc->snoop.snooping) {
  sendTo("Mixing snoop & switch is bad for your health.\n\r");
  return;
}

// CRASH: Switching while snooped
// Creates dangling snoop pointers when descriptor moves
```

### Pattern 4: Validate Target Body

```cpp
// CORRECT: Check all target body conditions
if (tBeing->desc || dynamic_cast<TPerson*>(tBeing)) {
  sendTo("You can't do that; the body is already in use!\n\r");
  return;
}

// CRASH: Switching into occupied body
// Two descriptors pointing to same character causes conflicts
```

### Pattern 5: Clear Descriptor on Original Body

```cpp
// CORRECT: Original loses descriptor when switched
this->desc = NULL;  // Original character has no descriptor

// CRASH: Leaving desc pointer on both characters
// Original character still processes commands while switched
```

## Common Switch/Return Bugs

### Bug 1: Nested Switch

```cpp
// CRASH: Attempting to switch while already switched
if (desc->original) {
  sendTo("You already seem to be switched.\n\r");
  return;
}

// Without this check, creates broken descriptor chain
```

**Why it crashes:** `desc->original` would point to the intermediate mob instead of the player, breaking the restoration chain.

### Bug 2: Return Without Switch

```cpp
// BUG: Calling return when not switched
void doReturn() {
  // Missing validation!
  TBeing* original = desc->original;  // NULL for normal state
  original->desc = desc;  // NULL pointer dereference!
}

// CORRECT: Always validate first
if (!desc || !desc->original) {
  sendTo("What are you trying to return from?!\n\r");
  return;
}
```

### Bug 3: Snooping Switched Character

```cpp
// CRASH: Snooping a switched immortal
if (d->original) {
  sendTo("Mixing snoop and switch is bad for your health.\n\r");
  return;
}

// Without this check, snoop output goes to wrong place
```

**Why it crashes:** The switched immortal's original body has no descriptor, so snoop output has nowhere to go.

### Bug 4: Forgetting polyed Flag

```cpp
// BUG: Not setting polyed flag
desc->character = tBeing;
desc->original = this;
tBeing->desc = desc;
desc = NULL;
// Missing: polyed = POLY_TYPE_SWITCH;

// CORRECT: Always set polyed
polyed = POLY_TYPE_SWITCH;
```

**Why it matters:** Other systems check `polyed` to detect switched state (e.g., idle timeout handling).

### Bug 5: Deleting Switched Body Directly

```cpp
// CRASH: Deleting switched body without returning
TBeing* switchedMob = /* ... */;
delete switchedMob;  // Immortal's descriptor is lost!

// CORRECT: Always call doReturn() first
switchedMob->doReturn("", WEAR_NOWHERE, true);
// Now safe to delete (if doReturn didn't already)
```

## Integration with Other Systems

### Polymorph System

Switch shares descriptor architecture with polymorph but differs critically:

| System | Command | ACT_POLYSELF | Stat Transfer | Reversible |
|--------|---------|--------------|---------------|------------|
| Admin Switch | `switch` | No | No | Yes (return) |
| Disguise | `disguise` | Yes | Yes | Yes (return) |
| Shapeshift | `shapeshift` | Yes | Yes | Yes (return) |
| Polymorph | `polymorph` | Yes | Yes | Yes (return) |
| Lycanthropy | Automatic | Yes | Yes | No (forced) |

**See also:** [Polymorph Safety](polymorph-safety.md)

### Idle Timeout Handling

The idle timeout system (limits.cc) handles switched characters:

```cpp
if (desc && desc->original && (desc->original->getTimer() >= 20)) {
  if ((specials.act & ACT_POLYSELF)) {
    // Transfer back from polymorph
    SwitchStuff(this, per);
    per->polyed = POLY_TYPE_NONE;
  }
  // Swap descriptor back and idle out
}
```

Admin switch does NOT trigger stat transfer on idle, only polymorph does.

### Death Handling

When a switched immortal's mob dies:

```cpp
// In rawKill() or die()
if (desc && desc->original && desc->original->polyed == POLY_TYPE_SWITCH) {
  // Automatically return to original body
  doReturn("", WEAR_NOWHERE, true, false);
  // Immortal survives, mob dies
}
```

### Combat Restrictions

Switched immortals in combat:
- Cannot return while fighting (prevents combat escape)
- Use mob's stats and abilities (not immortal powers)
- Death returns to immortal body (immortal doesn't die)

## Special Cases

### Switch Load with Wealth

When `switch load` creates a mob:

```cpp
*roomp += *tBeing;
(dynamic_cast<TMonster*>(tBeing))->oldRoom = inRoom();

// Only create wealth if configured
if (!Config::LoadOnDeath())
  (dynamic_cast<TMonster*>(tBeing))->createWealth();
```

If `LoadOnDeath()` is true, mobs drop gear on death instead of spawn, so no wealth on creation.

### POLY_STORAGE Room

Abandoned polymorph forms (not admin switch) go to special room:

```cpp
--(*this);
*(real_roomp(Room::POLY_STORAGE)) += *this;
```

This allows recovery if the system needs to examine the abandoned form.

### limitPowerCheck() Permission System

The `limitPowerCheck(CMD_SWITCH, mob_vnum)` function restricts which mobs can be switched:

```cpp
if (!limitPowerCheck(CMD_SWITCH, tBeing->number)) {
  sendTo("You're not allowed to switch into that mobile.\n\r");
  return;
}
```

Configuration in the database controls per-mob switch permissions.

## Key Source Files

| File | Contents |
|------|----------|
| `code/code/misc/immortal.cc` | `doSnoop()` implementation |
| `code/code/misc/immortal.cc` | `doSwitch()` implementation |
| `code/code/misc/immortal.cc` | `doReturn()` implementation |
| `code/code/sys/connect.h` | Descriptor class definition |
| `code/code/sys/connect.h` | snoopData structure |
| `code/code/sys/connect.cc` | `outputProcessing()` with snoop duplication |
| `code/code/misc/limits.cc` | Idle timeout with switch handling |
| `code/code/misc/combat.cc` | Death handling for switched characters |

## Related Documentation

- [Polymorph Safety](polymorph-safety.md) - Transformation system and desc->original validation
- [DELETE Flag System](delete-flags.md) - Memory management for mob deletion
- [Network Architecture](network-architecture.md) - Descriptor lifecycle and I/O flow
- [Wizard Powers](wizard-powers.md) - POWER_SNOOP and POWER_SWITCH permissions
