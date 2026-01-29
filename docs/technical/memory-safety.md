---
title: Memory Safety and DELETE Flags
description: Critical DELETE flag system for safe object deletion including flag propagation rules, transformation safety, death processing, and ownership patterns
keywords: [DELETE_THIS, DELETE_VICT, DELETE_ITEM, IS_SET_DELETE, REM_DELETE, reconcileDamage, die, rawKill, makeCorpse, doReturn, desc->original, ACT_POLYSELF, reformGroup, ownership rules, flag propagation, polymorph safety, death penalties]
category: Critical Systems
related: [combat-rounds.md, scheduler-pulses.md, command-implementation.md, damage-pipeline.md]
last_updated: 2026-01-29
source_files: [code/code/misc/combat.cc, code/code/misc/immortal.cc, code/code/misc/limits.cc, code/code/misc/defs.h, code/code/misc/structs.h, code/code/misc/damage.cc, code/code/misc/makecorpse.cc, code/code/sys/socket.cc, code/code/misc/periodic.cc]
---

# Memory Safety and DELETE Flags

The DELETE flag system is SneezyMUD's **core memory management mechanism**. Functions return flags to indicate that objects should be deleted by their caller, preventing use-after-free and double-delete crashes when objects are referenced by multiple stack frames. This system interacts critically with the transformation (polymorph) system and death processing pipeline.

**Misusing this system causes crashes.** Common errors: ignoring return values, deleting objects directly instead of returning flags, using `IS_SET` instead of `IS_SET_DELETE`, failing to translate flag semantics between contexts, not validating `desc->original`, and improper polymorph cleanup.

## DELETE Flag System Core

### Flag Definitions (`misc/defs.h`)

```cpp
const int DELETE_ITEM = ((1 << 5) | (1 << 29));
const int DELETE_THIS = ((1 << 6) | (1 << 29));
const int DELETE_VICT = ((1 << 7) | (1 << 29));
const int ALREADY_DELETED = ((1 << 8) | (1 << 29));
const int RET_STOP_PARSING = ((1 << 9) | (1 << 29));
```

The high bit (1 << 29) distinguishes DELETE flags from damage values, since some functions return both in the same integer.

### Utility Functions (`misc/structs.h`)

```cpp
IS_SET_DELETE(value, flag)   // Check if flag is set - USE THIS, not IS_SET
ADD_DELETE(value, flag)      // Add a flag to a return value
REM_DELETE(value, flag)      // Remove a flag from a return value
```

**Critical:** Never use `IS_SET()` for DELETE flags—it won't work correctly with the combined bit pattern.

### What the Flags Mean

The meaning is **context-dependent based on function parameters**:

| Flag          | Typical Meaning                                                                                  |
| ------------- | ------------------------------------------------------------------------------------------------ |
| `DELETE_THIS` | Delete the object/being the method was called on (`this` pointer or first significant parameter) |
| `DELETE_VICT` | Delete the victim/target parameter                                                               |
| `DELETE_ITEM` | Delete an item/object parameter                                                                  |

### Where DELETE Flags Are Used

- **Combat/Damage:** `reconcileDamage()`, `applyDamage()` → `DELETE_VICT` when victim dies
- **Movement:** `doMove()`, `crashLanding()`, `rawMove()` → `DELETE_THIS` on fatal movement
- **Object Interactions:** `get()`, `drop()`, `put()`, `give()` → various flags from traps
- **Trap System:** 40+ functions in `trap.cc` → `DELETE_THIS`, `DELETE_ITEM`
- **Spec Functions:** `checkSpec()` on mobs/objects/rooms → any DELETE flag possible
- **Spells/Skills:** Damage-dealing abilities check `reconcileDamage()` for `DELETE_VICT`
- **Death Processing:** `die()`, `rawKill()` → `DELETE_THIS` for self-deletion
- **Transformation:** `doReturn()`, `updateAffects()` → `DELETE_THIS`, `ALREADY_DELETED`

## Ownership and Propagation Rules

### The Ownership Pattern

**Rule:** Whoever resolved/found a pointer owns it and is responsible for deletion.

```cpp
// Pattern 1: Caller owns the victim (passed as parameter)
int someFunction(TBeing* victim) {
    int rc = victim->doSomething();
    if (IS_SET_DELETE(rc, DELETE_THIS)) {
        // victim parameter was passed by caller
        // Return flag so caller can delete
        return DELETE_VICT;  // Translate: their THIS is our VICT
    }
}

// Pattern 2: Callee owns the victim (resolved locally)
int someFunction() {
    TBeing* victim = get_char_room_vis(this, arg);
    if (!victim) return FALSE;

    int rc = victim->doSomething();
    if (IS_SET_DELETE(rc, DELETE_THIS)) {
        // We resolved victim locally, we own it
        delete victim;
        victim = nullptr;
        REM_DELETE(rc, DELETE_THIS);  // Clear the flag
        // Don't propagate - we handled it
    }
}
```

### Flag Translation Pattern

When propagating DELETE flags, **translate** based on parameter mapping:

```cpp
// In get(): ch is "this", sub is "vict" in our return semantics
rc = ch->checkForInsideTrap(sub);

// checkForInsideTrap's DELETE_THIS means "delete sub" (what it was called on)
// But in get()'s return, sub maps to DELETE_VICT
if (IS_SET_DELETE(rc, DELETE_THIS))
    return DELETE_VICT;  // Translate: their THIS is our VICT

// Similarly, if we call a method on ourselves:
rc = this->someMethod();
if (IS_SET_DELETE(rc, DELETE_THIS))
    return DELETE_THIS;  // No translation needed
```

### Safe Patterns

```cpp
// Always check return values
int rc = victim->someAction(item);
if (IS_SET_DELETE(rc, DELETE_VICT)) {
    delete victim;
    victim = nullptr;
    return DELETE_VICT;  // Propagate to caller
}

// Combine flags when needed
return DELETE_THIS | DELETE_VICT;  // Both should be deleted

// Using REM_DELETE when handling locally
if (IS_SET_DELETE(rc, DELETE_VICT)) {
    delete victim;
    victim = nullptr;
    REM_DELETE(rc, DELETE_VICT);  // Clear flag we handled
}
```

### Dangerous Anti-Patterns

```cpp
// CRASH: Ignoring return value
victim->doSomething();
victim->doSomethingElse();  // victim may be deleted!

// CRASH: Using IS_SET instead of IS_SET_DELETE
if (IS_SET(rc, DELETE_THIS)) { ... }  // Won't detect combined bits

// CRASH: Deleting directly instead of returning flag
delete victim;  // Wrong if caller holds reference

// BUG: Not propagating to caller
if (IS_SET_DELETE(rc, DELETE_VICT)) {
    delete victim;
    victim = nullptr;
    // Forgot to: return DELETE_VICT;
}

// CRASH: Wrong ownership assumption
int rc = victim->die(DAMAGE_NORMAL);
if (IS_SET_DELETE(rc, DELETE_VICT)) { ... }  // die() returns DELETE_THIS!
```

## Transformation Safety (Polymorph System)

### The Descriptor Swap Pattern

When a player transforms, their descriptor (`desc`) is moved to a newly created mob while their original body is stored in `Room::POLY_STORAGE`:

```
Before transformation:
  TPerson (player) <---> Descriptor
      |
      v
    Room (normal gameplay)

After transformation:
  TPerson (original body) ---- desc->original ---->
      |                                           Descriptor <---> TMonster (transformed form)
      v                                                              |
  Room::POLY_STORAGE                                                 v
                                                               Room (normal gameplay)
```

**Key relationships after transformation:**
- `mob->desc` points to the player's descriptor
- `mob->desc->original` points to the original `TPerson` body
- `mob->desc->character` points to the `TMonster` mob
- `person->polyed` is set to the transformation type
- `person->desc` is `nullptr` (the person has no descriptor)
- `mob->specials.act` has `ACT_POLYSELF` set

### Transformation Types (`polyTypeT`)

```cpp
enum polyTypeT {
    POLY_TYPE_NONE,       // Not transformed
    POLY_TYPE_SWITCH,     // Immortal switch (no stat transfer)
    POLY_TYPE_DISGUISE,   // Thief disguise skill
    POLY_TYPE_SHAPESHIFT, // Shaman shapeshift spell
    POLY_TYPE_POLYMORPH   // Mage polymorph spell
};
```

| Type | Source | Transfers Stats | Transfers Equipment |
|------|--------|-----------------|---------------------|
| `POLY_TYPE_SWITCH` | Immortal `switch` command | No | No |
| `POLY_TYPE_DISGUISE` | `SKILL_DISGUISE` | Yes (via `DisguiseStuff`) | Yes |
| `POLY_TYPE_SHAPESHIFT` | `SPELL_SHAPESHIFT` | Yes (via `SwitchStuff`) | Yes |
| `POLY_TYPE_POLYMORPH` | `SPELL_POLYMORPH` | Yes (via `SwitchStuff`) | Yes |

Werewolf transformation uses `POLY_TYPE_DISGUISE` with `TOG_TRANSFORMED_LYCANTHROPE` quest bit.

### CRITICAL: desc->original Validation

**ALWAYS validate `desc->original` before dereferencing.** This pointer can become null or stale in several scenarios:

```cpp
// CORRECT: Always validate before use
if (desc && desc->original) {
    TPerson* per = desc->original;
    // Safe to use per
}

// CRASH: No validation
TPerson* per = desc->original;  // May be nullptr!
per->doSomething();             // Crash!
```

### Automatic Transformation Removal

Transformations are removed in `updateAffects()` (periodic.cc:768-773) when `desc` or `desc->original` becomes null:

```cpp
if ((af->type == SPELL_POLYMORPH) || (af->type == SKILL_DISGUISE) ||
    (af->type == SPELL_SHAPESHIFT)) {
    if (!desc || !desc->original) {
        affectRemove(af);
        continue;
    }
}
```

This protects against lingering transformation affects on orphaned mobs.

### The Werewolf Bug (FIXED)

The werewolf transformation system had a critical heap-use-after-free bug that illustrates the danger:

#### The Bug (OLD CODE - CRASHED)

```cpp
bool procCharLycanthropy::run(const TPulse& pl, TBeing* tmp_ch) const {
    if (Weather::moonType() != "full") {
        tmp_ch->remQuestBit(TOG_TRANSFORMED_LYCANTHROPE);
        tmp_ch->doReturn("", WEAR_NOWHERE, true);  // <-- Deleted tmp_ch!
        // After doReturn with deleteMob=true, tmp_ch is freed memory
    }
    return false;  // <-- Accessed freed tmp_ch via this return!
}
```

The problem: `doReturn()` with `deleteMob=true` (the default) deletes the mob. But `tmp_ch` is still referenced after the call returns, and the scheduler continues processing.

#### The Fix (CURRENT CODE - SAFE)

```cpp
bool procCharLycanthropy::run(const TPulse& pl, TBeing* tmp_ch) const {
    if (Weather::moonType() != "full") {
        tmp_ch->remQuestBit(TOG_TRANSFORMED_LYCANTHROPE);
        // Pass false to move mob to poly storage instead of deleting
        tmp_ch->doReturn("", WEAR_NOWHERE, true, false);
        // Return true to signal scheduler to delete tmp_ch
        return true;
    }
    return false;
}
```

**Key insight:** When transformation ends during scheduler processing, use `doReturn(..., false)` to move the mob to storage, then return `true` to let the scheduler batch-delete it safely.

### The doReturn() Function

`doReturn()` (immortal.cc:2076) handles returning from transformation:

```cpp
void TBeing::doReturn(const sstring& argument, wearSlotT limb, bool tell,
                      bool deleteMob = true);  // NOTE: deleteMob defaults to true!
```

| Parameter | Purpose |
|-----------|---------|
| `argument` | For limb transformation only |
| `limb` | Specific limb to transform back (0 for full return) |
| `tell` | Whether to show messages to the room |
| `deleteMob` | **CRITICAL:** If true, deletes the mob; if false, moves to storage |

#### doReturn() Flow

1. Validates `desc && desc->original` exist
2. For `ACT_POLYSELF` mobs, transfers equipment/stats back via `SwitchStuff()`
3. Moves original body from storage back to the room
4. Swaps descriptor back: `originalBody->desc = desc`
5. Clears `desc->original` and `this->desc`
6. Sets `originalBody->polyed = POLY_TYPE_NONE`
7. If `deleteMob` is true, deletes the mob; otherwise moves it to `Room::POLY_STORAGE`

### Safe Transformation Patterns

#### Pattern 1: In Scheduler Procs

```cpp
bool procCharSomething::run(const TPulse& pl, TBeing* ch) const {
    if (needsToEndTransformation(ch)) {
        // DON'T delete directly - use doReturn with deleteMob=false
        ch->doReturn("", WEAR_NOWHERE, true, false);
        // Signal scheduler to handle deletion
        return true;
    }
    return false;
}
```

#### Pattern 2: In Death Handling

```cpp
int TBeing::rawKill(spellNumT dmg_type, TBeing* tKiller, float exp_lost) {
    if (hasQuestBit(TOG_TRANSFORMED_LYCANTHROPE) ||
        (specials.act & ACT_POLYSELF) ||
        (desc && desc->original && desc->original->polyed == POLY_TYPE_SWITCH)) {
        TBeing* per = desc->original;

        if (per->polyed == POLY_TYPE_SWITCH) {
            remQuestBit(TOG_TRANSFORMED_LYCANTHROPE);
            doReturn("", WEAR_NOWHERE, true, false);  // Don't delete yet
            return rawKill(dmg_type, tKiller);        // Recurse on original
        }
        // For non-switch, return to original and kill them
        remQuestBit(TOG_TRANSFORMED_LYCANTHROPE);
        doReturn("", WEAR_NOWHERE, true);  // Delete mob

        if ((per->rawKill(dmg_type, tKiller)) == DELETE_THIS) {
            per->reformGroup();
            delete per;
        }
        return DELETE_THIS;
    }
    // ... normal death handling
}
```

#### Pattern 3: Affect Wear-Off

```cpp
// In updateAffects() when transformation duration expires
if (shouldReturn) {
    doReturn("", WEAR_NOWHERE, true, false);  // Don't delete
    return ALREADY_DELETED;  // Signal caller to handle deletion
}
```

### Transformation Edge Cases

#### Linkdeath During Transformation

When a transformed player loses connection:
- The mob (`desc->character`) loses its descriptor
- The original body in `Room::POLY_STORAGE` has `polyed != POLY_TYPE_NONE`
- The `TBeing::orig` field on the mob stores a pointer to the original for reconnect

On reconnect (connect.cc:1071-1073):
```cpp
if (tmp_ch->orig) {
    tmp_ch->desc->original = tmp_ch->orig;
    tmp_ch->orig = 0;
}
```

**Risk:** The `orig` pointer could become stale if the original body is deleted separately.

#### Idling Timeout

`checkIdling()` (limits.cc:1496-1528) handles idle timeout for transformed characters:

```cpp
if (desc && desc->original && (desc->original->getTimer() >= 20)) {
    if ((specials.act & ACT_POLYSELF)) {
        // Transfer back to original
        SwitchStuff(mob, per);
        per->polyed = POLY_TYPE_NONE;
    }
    // Swap descriptor back and idle out
}
```

#### Shapeshift Indoor Restriction

`SPELL_SHAPESHIFT` cannot survive indoors. In `updateTickStuff()` (periodic.cc:1674-1681):

```cpp
if (desc && desc->original && desc->original->polyed &&
    !desc->original->isImmortal() &&
    (desc->original->polyed == POLY_TYPE_SHAPESHIFT)) {
    sendTo("Your shape can not survive without a connection to nature.\n\r");
    doReturn("", WEAR_NOWHERE, true, false);
    return ALREADY_DELETED;
}
```

### Pre-Transformation Validation

Before initiating any transformation, always validate:

```cpp
// Standard checks before transformation
if (!caster->desc || caster->desc->snoop.snooping) {
    caster->nothingHappens();
    return SPELL_FAIL;
}

if (caster->desc->original) {
    caster->sendTo("You already seem to be switched.\n\r");
    return SPELL_FAIL;
}

if (caster->desc->snoop.snoop_by)
    caster->desc->snoop.snoop_by->doSnoop(
        caster->desc->snoop.snoop_by->getName().c_str());
```

### isLinkdead() and Transformation

```cpp
bool TBeing::isLinkdead() const {
    return (isPc() && !desc && polyed == POLY_TYPE_NONE);
}
```

**Important:** A transformed player's original body is NOT considered linkdead because `polyed != POLY_TYPE_NONE`. The transformed mob IS considered connected because it has the descriptor.

### Transformation Anti-Patterns

```cpp
// CRASH: Deleting transformed mob directly in scheduler
bool procCharBad::run(const TPulse& pl, TBeing* ch) const {
    if (shouldEndTransform(ch)) {
        ch->doReturn("", WEAR_NOWHERE, true);  // Deletes ch!
        // ch is now freed memory
        return false;  // CRASH: accesses freed ch
    }
}

// CRASH: Accessing after doReturn
ch->doReturn("", WEAR_NOWHERE, true);
ch->sendTo("Goodbye!");  // ch may be deleted!

// CRASH: Not validating desc->original
TPerson* per = desc->original;  // May be nullptr!
per->getName();  // Crash if per is null

// BUG: Forgetting to clear ACT_POLYSELF before delete
delete this;  // If ACT_POLYSELF still set, causes issues

// BUG: Not handling ALREADY_DELETED return
int rc = updateAffects();
// Ignoring rc and continuing to use this
this->doSomething();  // May crash if transformation ended
```

## Death Processing System

### Death Flow Overview

Every character death flows through a standardized pipeline:

```
die() → rawKill() → makeCorpse()
  ↓         ↓            ↓
Penalties  Combat    Equipment
           Cleanup   Transfer
```

The death functions handle:
- Death penalties (XP loss, age increase)
- Combat state cleanup
- Equipment and money transfer to corpses
- Group leadership transfer
- Follower and rent file cleanup (PCs only)
- Database updates (permadeath tracking)
- Position updates and respawn handling

**Key principle:** Death functions return `DELETE_THIS` to signal the scheduler or caller should delete the character. Never delete directly in death functions—always propagate the flag.

### die() - Entry Point with Penalties

```cpp
int TBeing::die(spellNumT dam_type, TBeing* tKiller) {
  // 1. Polymorph/switch handling (return to original before death)
  if (dynamic_cast<TMonster*>(this) && (desc || isPc())) {
    if (!(d = desc) || !d->original) {
      vlogf(LOG_BUG, "*BUG BRUTIUS BIG TIME* (die)");
      return FALSE;
    }
    per = desc->original;
    if (per->polyed == POLY_TYPE_SWITCH) {
      doReturn("", WEAR_NOWHERE, true, false);
      rawKill(dam_type, tKiller);
      return DELETE_THIS;
    }
  }

  // 2. AFFECT_FREE_DEATHS check (quest/event penalty-free deaths)
  bool skip_death = false;
  for (aff = affected; aff; aff = aff->next) {
    if (aff->type == AFFECT_FREE_DEATHS) {
      if (aff->modifier > 0 && !rp->isRoomFlag(ROOM_ARENA)) {
        aff->modifier -= 1;
        skip_death = true;
        break;
      }
    }
  }

  // 3. Death statistics tracking
  stats.deaths[GetMaxLevel()][!isPc()] = stats.deaths[GetMaxLevel()][!isPc()] + 1;

  // 4. XP loss and age penalty (skip in arena or with free death)
  if (isPc() && !rp->isRoomFlag(ROOM_ARENA) && !skip_death) {
    int val_num = ::number(0, 3);  // Random 0-3 years
    if (GetMaxLevel() <= 10)
      val_num = 0;  // No age penalty for low levels
    age_mod += val_num;
    gain_exp(this, -deathExp(), -1);
    exp_lost = deathExp();
  }

  rawKill(dam_type, tKiller, exp_lost);
  return DELETE_THIS;
}
```

**Source:** `code/code/misc/combat.cc`

### rawKill() - Core Death Processing

```cpp
int TBeing::rawKill(spellNumT dmg_type, TBeing* tKiller, float exp_lost) {
  // 1. Additional polymorph/switch handling
  if (hasQuestBit(TOG_TRANSFORMED_LYCANTHROPE) ||
      (specials.act & ACT_POLYSELF) ||
      (desc && desc->original && desc->original->polyed == POLY_TYPE_SWITCH)) {
    per = desc->original;
    if (per->polyed == POLY_TYPE_SWITCH) {
      remQuestBit(TOG_TRANSFORMED_LYCANTHROPE);
      doReturn("", WEAR_NOWHERE, true, false);
      return rawKill(dmg_type, tKiller);
    }
  }

  // 2. Combat cleanup
  if (fight())
    stopFighting();

  // 3. Mode and affect cleanup
  if (isCombatMode(ATTACK_BERSERK))
    setCombatMode(ATTACK_NORMAL);
  if (affectedBySpell(SKILL_BERSERK))
    affectFrom(SKILL_BERSERK);
  if (affectedBySpell(AFFECT_BITTEN_BY_VAMPIRE))
    affectFrom(AFFECT_BITTEN_BY_VAMPIRE);

  // 4. Dead immortal handling
  if (GetMaxLevel() > MAX_MORT) {
    if (!isPlayerAction(PLR_IMMORTAL))
      addPlayerAction(PLR_IMMORTAL);
  }

  // 5. Shopkeeper inventory deletion
  if (isShopkeeper()) {
    for (StuffIter it = stuff.begin(); it != stuff.end();) {
      delete *(it++);
    }
    setMoney(0);
  }

  // 6. Create corpse
  if (!IS_SET(specials.act, ACT_HIT_BY_PK))
    makeCorpse(dmg_type, tKiller, exp_lost);

  deathCry();
  genericKillFix();

  // 7. PC-specific cleanup
  if (isPc()) {
    reformGroup();
    removeRent();
    removeFollowers();
    logPermaDeathDied(this, tKiller);
  }

  preKillCheck();

  return DELETE_THIS;
}
```

**Source:** `code/code/misc/combat.cc`

### Death Penalties

#### Experience Loss Calculation

```cpp
double TBeing::deathExp() {
  double amt;
  // Formula: min(current_exp/5, 25*mob_exp(level))
  amt = 25.0 * mob_exp((float)GetMaxLevel());
  amt = min(1 * getExp() / 5, amt);

  // PK death penalty reduction (1/10th of normal)
  if (isPking())
    amt /= 10.0;

  return amt;
}
```

**Formula breakdown:**
- Base penalty: 25 × mob XP value for character's level
- Capped at: 20% of current total experience
- **Takes minimum of the two** (whichever is less punishing)
- PvP reduction: Divide by 10 (so only 2% of exp, or 2.5× mob XP, whichever is less)

#### Age Penalty

Random 0-3 years added to character age on death:
```cpp
int val_num = ::number(0, 3);
if (GetMaxLevel() <= 10)
  val_num = 0;  // No age penalty for levels ≤10
age_mod += val_num;
```

**Exemptions:**
- Level ≤10 characters: No age penalty
- Arena deaths: No age penalty
- Free death affects: No age penalty

### PC vs NPC Death Differences

| Aspect | PC Death | NPC Death |
|--------|----------|-----------|
| XP loss | Yes (via deathExp()) | No |
| Age increase | Yes (0-3 years) | No |
| Corpse type | TPCorpse (vnum -2) | TCorpse (mob vnum) |
| Corpse flags | CORPSE_NO_REGEN | None (unless vnum < 0) |
| Group handling | reformGroup() transfers leadership | No group handling |
| Rent cleanup | removeRent() | N/A |
| Follower cleanup | removeFollowers() | N/A |
| Permadeath log | logPermaDeathDied() | N/A |
| Save location | Room::NOWHERE | N/A |

### Post-Death Cleanup Functions

#### reformGroup() - Group Leadership Transfer

```cpp
void TBeing::reformGroup() {
  followData *tmp, *tmp2;
  TBeing *new_master = NULL, *survivor = NULL;
  bool found = FALSE;

  if (!followers || master)
    return;  // Only group leaders trigger reform

  // First pass: Find eligible PC/linked follower
  for (tmp = followers; tmp; tmp = tmp2) {
    tmp2 = tmp->next;

    // Exclude pure mobs
    if ((!tmp->follower->isPc() && isPc()) &&
        tmp->follower->polyed == POLY_TYPE_NONE) {
      tmp->follower->stopFollower(TRUE);
      continue;
    }

    if (!inGroup(*tmp->follower)) {
      tmp->follower->stopFollower(TRUE);
      continue;
    }

    // First eligible follower becomes new leader
    if (!found) {
      new_master = tmp->follower;
      new_master->stopFollower(TRUE, STOP_FOLLOWER_CHAR_VICT);
      found = TRUE;
      new_master->master = NULL;

      if (!new_master->isAffected(AFF_GROUP))
        SET_BIT(new_master->specials.affectedBy, AFF_GROUP);

      new_master->sendTo(COLOR_BASIC,
        format("<R>%s has died and you have taken over leadership of the group.<z>\n\r") %
          getName());
      continue;
    }

    // Remaining followers re-attach to new leader
    survivor = tmp->follower;
    survivor->stopFollower(TRUE, STOP_FOLLOWER_CHAR_VICT);
    new_master->addFollower(survivor, 1);

    if (!survivor->isAffected(AFF_GROUP))
      SET_BIT(survivor->specials.affectedBy, AFF_GROUP);

    survivor->sendTo(COLOR_BASIC, format("<R>%s has died and %s has taken over "
                                         "leadership of the group.<z>\n\r") %
                                    getName() % new_master->getName());
  }
  // Second pass handles immortal/poly/linkdead if no PC found
}
```

**Key behaviors:**
- Only triggers for group leaders (`!master && followers`)
- Two-pass algorithm: prefer PC followers, fall back to any follower
- First eligible follower becomes new leader
- All remaining followers re-attach to new leader
- Group affect (`AFF_GROUP`) maintained on all members

#### genericKillFix() - General Cleanup

```cpp
void TBeing::genericKillFix(void) {
  reformGroup();  // Transfer group leadership
  DeleteHatreds(this, NULL);  // Remove from all mob hate lists
  DeleteFears(this, NULL);    // Remove from all mob fear lists

  // Spell cleanup with double-death safety
  int rc = generic_dispel_magic(NULL, this, MAX_IMMORT, IMMORTAL_YES, SAFE_YES);
  int rc2 = genericChaseSpirits(NULL, this, MAX_IMMORT, IMMORTAL_YES, SAFE_YES);
  if (IS_SET_DELETE(rc, DELETE_VICT)) {
    vlogf(LOG_BUG, "Multiple deaths occurred!");
  }
  if (IS_SET_DELETE(rc2, DELETE_VICT)) {
    vlogf(LOG_BUG, "Multiple deaths occurred!");
  }

  // Reset hunger/thirst
  if (getCond(THIRST) >= 0)
    setCond(THIRST, 20);
  if (getCond(FULL) >= 0)
    setCond(FULL, 20);
  setMove(moveLimit());

  // Limb restoration for PCs (non-arena)
  if (roomp && isPc() && !roomp->isRoomFlag(ROOM_ARENA)) {
    for (wearSlotT i = MIN_WEAR; i < MAX_WEAR; i++) {
      setCurLimbHealth(i, getMaxLimbHealth(i));
      setLimbFlags(i, 0);
      setStuckIn(i, NULL);
    }
  }

  // Disease cleanup for PCs
  if (isPc() && affected) {
    for (aff = affected; aff; aff = aff2) {
      aff2 = aff->next;
      if (aff->type == AFFECT_DISEASE) {
        diseaseStop(aff);
        affectRemove(aff);
      }
    }
  }

  // Shaman special handling (respawn at 25 HP, 50 lifeforce)
  if (isPc() && hasClass(CLASS_SHAMAN)) {
    setHit(25);
    setLifeforce(50);
    updatePos();
  }
}
```

#### Other Cleanup Functions

**removeRent()** - Wipes rent file (`lib/mutable/rent/{a-z}/{charname}`) so dead characters don't have rented items when they respawn.

**removeFollowers()** - Wipes followers file (`lib/mutable/rent/{a-z}/{charname}.fol`) so dead characters don't have saved followers (pets, charms, thralls) when they respawn.

**logPermaDeathDied()** - Updates the `permadeath` table with:
- `died=1` flag marking the death
- Killer's name (if applicable)
- Final level at death

**preKillCheck()** - Saves character to `Room::NOWHERE` (death state) for normal deaths, or current room for arena deaths.

### DELETE Flag Propagation in Death

#### Critical Pattern

Death functions return `DELETE_THIS`, not `DELETE_VICT`:

```cpp
// CORRECT: die() signals deletion of self
int rc = victim->die(DAMAGE_NORMAL);
if (IS_SET_DELETE(rc, DELETE_THIS)) {
  delete victim;
  victim = NULL;
}

// WRONG: Checking for wrong flag
if (IS_SET_DELETE(rc, DELETE_VICT)) { ... }  // Never set by die()!
```

#### Scheduler Deletion

Combat loop handles death returns:
```cpp
// In perform_violence()
rc = ch->hit(vict, pulse);
if (IS_SET_DELETE(rc, DELETE_VICT)) {
  vict->reformGroup();  // CRITICAL: Call before delete
  delete vict;
  vict = NULL;
  continue;
} else if (IS_SET_DELETE(rc, DELETE_THIS)) {
  ch->reformGroup();  // CRITICAL: Call before delete
  delete ch;
  ch = NULL;
  break;
}
```

#### Double-Death Safety

`genericKillFix()` checks for double-death from spell cleanup:

```cpp
int rc = generic_dispel_magic(NULL, this, MAX_IMMORT, IMMORTAL_YES, SAFE_YES);
int rc2 = genericChaseSpirits(NULL, this, MAX_IMMORT, IMMORTAL_YES, SAFE_YES);
if (IS_SET_DELETE(rc, DELETE_VICT)) {
  vlogf(LOG_BUG, "Multiple deaths occurred!");
}
if (IS_SET_DELETE(rc2, DELETE_VICT)) {
  vlogf(LOG_BUG, "Multiple deaths occurred!");
}
```

This can happen if a spell wearoff effect (like charm expiration) triggers another death.

### Arena Death Exemption

Deaths in arena rooms (`ROOM_ARENA` flag) skip penalties:

```cpp
if (isPc() && !rp->isRoomFlag(ROOM_ARENA) && !skip_death) {
  // XP loss
  gain_exp(this, -deathExp(), -1);
  // Age penalty
  age_mod += val_num;
}
```

**Arena exemptions:**
- No XP loss
- No age increase
- Limbs NOT fully healed (unlike normal death)
- Character saved to current room (not `Room::NOWHERE`)

## Top-Level DELETE Flag Handlers

These functions terminate the DELETE flag chain by performing actual deletion:

### Core Scheduler (`process.cc`)

- `TScheduler::runObj()` - Deletes obj if proc::run() returns true
- `TScheduler::runChar()` - Collects in deleteMe vector, batch deletes

### Command Processing (`connect.cc`)

- Descriptor loop - DELETE_THIS from parseCommand → deletes character
- Account menu - DELETE_THIS from doAccountMenu → deletes descriptor

### Combat (`combat.cc`)

- `perform_violence()` - DELETE_THIS, DELETE_VICT → deletes combatants

### Intermediate Handlers (`socket.cc`)

All `proc*::run()` functions check DELETE flags and return `bool` (adapter layer):

- `procObjDetonateGrenades::run`, `procObjFalling::run`, `procObjRiverFlow::run`
- `procObjTeleportRoom::run`, `procObjSpecProcsQuick::run`, `procObjTickUpdate::run`
- `procObjFreezing::run`, `procObjSmoke::run`, `procObjPools::run`, `procObjTrash::run`
- `procObjSinking::run`, `procObjSpecProcs::run`, `procObjBurning::run`
- `procCharSpecProcs::run`, `procCharDrowning::run`, `procCharFalling::run`
- `procCharMobileActivity::run`, `procCharTasks::run`, `procCharSpellTask::run`
- `procCharAffects::run`, `procCharRiverFlow::run`, `procCharTeleportRoom::run`
- `procCharHalfTickUpdate::run`, `procCharTickUpdate::run`, `procCharLightning::run`
- `procCharLycanthropy::run`, `procCharSpecProcsQuick::run`, `procCharVampireBurn::run`

## Architecture

```
gameLoop() [socket.cc]
├── TScheduler::run() [process.cc]
│   ├── TScheduler::runObj() ◄── TOP-LEVEL (deletes objects)
│   │   └── procObj*::run() → returns bool
│   │       └── someFunction() → returns DELETE_*
│   ├── TScheduler::runChar() ◄── TOP-LEVEL (deletes characters)
│   │   └── procChar*::run() → returns bool
│   │       └── someFunction() → returns DELETE_*
│   └── procPerformViolence
│       └── perform_violence() ◄── TOP-LEVEL (deletes combatants)
└── Descriptor loop [connect.cc] ◄── TOP-LEVEL (deletes from commands)
    └── parseCommand() → returns DELETE_THIS
        └── doXXX() → returns DELETE_*
```

## Unified Anti-Patterns

### Memory Safety

```cpp
// CRASH: Not checking reconcileDamage() death
int dam = reconcileDamage(victim, ...);
victim->sendTo("Ouch!\n");  // victim may be deleted if dam == -1

// CORRECT: Check for death (-1 return)
int dam = reconcileDamage(victim, ...);
if (dam == -1)
  return DELETE_VICT;
victim->sendTo("Ouch!\n");

// CRASH: Using wrong flag check macro
if (IS_SET(rc, DELETE_THIS)) { ... }  // Won't detect combined bits

// CORRECT: Use IS_SET_DELETE
if (IS_SET_DELETE(rc, DELETE_THIS)) { ... }

// CRASH: Not calling reformGroup() before deletion
if (IS_SET_DELETE(rc, DELETE_THIS)) {
  delete ch;  // Followers have dangling master pointer!
}

// CORRECT: Always call reformGroup() first
if (IS_SET_DELETE(rc, DELETE_THIS)) {
  ch->reformGroup();
  delete ch;
  ch = NULL;
}
```

### Transformation Safety

```cpp
// CRASH: Deleting transformed mob in scheduler without doReturn
bool procCharBad::run(const TPulse& pl, TBeing* ch) const {
  if (shouldEndTransform(ch)) {
    delete ch;  // WRONG! desc->original still references this
    return true;
  }
}

// CORRECT: Use doReturn with deleteMob=false
bool procCharGood::run(const TPulse& pl, TBeing* ch) const {
  if (shouldEndTransform(ch)) {
    ch->doReturn("", WEAR_NOWHERE, true, false);
    return true;
  }
}

// CRASH: Not validating desc->original
TPerson* per = desc->original;
per->doSomething();  // per may be nullptr!

// CORRECT: Always validate
if (!desc || !desc->original)
  return FALSE;
TPerson* per = desc->original;
```

### Death Processing

```cpp
// CRASH: Continuing after DELETE_THIS from die()
int rc = victim->die(DAMAGE_NORMAL);
victim->sendTo("You died!\n");  // victim may be deleted!

// CORRECT: Check immediately and return
int rc = victim->die(DAMAGE_NORMAL);
if (IS_SET_DELETE(rc, DELETE_THIS))
  return DELETE_VICT;

// BUG: Wrong flag check (die returns DELETE_THIS, not DELETE_VICT)
int rc = victim->die(DAMAGE_NORMAL);
if (IS_SET_DELETE(rc, DELETE_VICT)) { ... }  // Never triggers!

// CORRECT: Check for DELETE_THIS
int rc = victim->die(DAMAGE_NORMAL);
if (IS_SET_DELETE(rc, DELETE_THIS)) {
  return DELETE_VICT;  // Translate for caller's context
}

// CRASH: Deleting directly in death function
int TBeing::die(...) {
  // ... processing
  delete this;  // WRONG! Caller still has pointer
}

// CORRECT: Return DELETE_THIS flag
int TBeing::die(...) {
  // ... processing
  return DELETE_THIS;  // Caller handles deletion
}
```

## Files Reference

| File | Content |
|------|---------|
| `code/code/misc/defs.h` | DELETE flag definitions |
| `code/code/misc/structs.h` | IS_SET_DELETE, ADD_DELETE, REM_DELETE macros |
| `code/code/misc/combat.cc` | die(), rawKill(), reformGroup(), genericKillFix(), deathCry(), deathExp() |
| `code/code/misc/damage.cc` | reconcileDamage(), applyDamage() |
| `code/code/misc/movement.cc` | doMove(), crashLanding(), rawMove() |
| `code/code/obj/trap.cc` | Trap system DELETE flag usage |
| `code/code/sys/process.cc` | TScheduler::runObj(), TScheduler::runChar() |
| `code/code/sys/connect.cc` | parseCommand(), descriptor loop |
| `code/code/sys/socket.cc` | proc*::run() adapter functions, procCharLycanthropy |
| `code/code/misc/immortal.cc` | doReturn() implementation |
| `code/code/misc/periodic.cc` | updateAffects(), transformation affect removal |
| `code/code/misc/limits.cc` | checkIdling() transformation handling |
| `code/code/misc/makecorpse.cc` | makeCorpse() implementation |
| `code/code/misc/race.cc` | Race::makeCorpse(), Race::makePCorpse() |
| `code/code/misc/rent.cc` | removeRent(), removeFollowers() |
| `code/code/misc/offense.cc` | preKillCheck() |
| `code/code/misc/magicutils.cc` | SwitchStuff(), DisguiseStuff(), lycanthropeTransform() |

## See Also

- [Combat Rounds](combat-rounds.md) - Combat loop DELETE flag handling
- [Damage Pipeline](damage-pipeline.md) - reconcileDamage() death detection
- [Scheduler and Pulses](scheduler-pulses.md) - Proc adapter bool returns
- [Trap System](trap-system.md) - Extensive DELETE flag usage
- [Affects System](affects-system.md) - AFFECT_FREE_DEATHS, transformation affects
- [Spatial Relationships](spatial-relationships.md) - reformGroup(), follower chains
- [Equipment and Wear](equipment-wear.md) - Equipment transfer to corpses
