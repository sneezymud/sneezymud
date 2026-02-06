---
title: Death Processing
description: Death penalties, corpse creation, XP loss formulas, and cleanup procedures for PC and NPC deaths
keywords: [death penalty, corpse creation, XP loss, permadeath, arena, resurrection, death flow]
category: important
source_files: [code/code/misc/combat.cc, code/code/misc/damage.cc, code/code/obj/obj_base_corpse.cc, code/code/obj/obj_player_corpse.cc]
primary_symbols:
  functions: [die, rawKill, makeCorpse, genericKillFix, reformGroup, stopFighting, deathExp, generic_dispel_magic]
  classes: [TBeing, TCorpse, TPCorpse]
  enums: [DELETE_THIS, ROOM_ARENA, AFFECT_FREE_DEATHS, CORPSE_NO_REGEN, DAMAGE_NORMAL]
---

# Death Processing

## Overview

Death in SneezyMUD triggers a multi-stage cleanup process: applying penalties, creating corpses, transferring equipment, and cleaning up combat state. The system distinguishes between PC and NPC deaths, with PCs suffering experience loss and age penalties while NPCs simply create loot corpses.

Every death flows through the pipeline: `die()` -> `rawKill()` -> `makeCorpse()`. The `die()` function handles penalties and checks for exemptions. The `rawKill()` function performs cleanup and creates the corpse. Both return `DELETE_THIS` to signal the caller should delete the character (see [01-memory-safety.md](../critical/01-memory-safety.md) for DELETE flag handling).

## Patterns

### Death Detection

Check for -1 return from `reconcileDamage()` to detect death. It returns damage dealt on survival, -1 on death. It does not return DELETE flags.

```cpp
// WRONG - Checking for DELETE flag
int dam = reconcileDamage(victim, ...);
if (IS_SET_DELETE(dam, DELETE_VICT)) {  // Never triggers!
  return DELETE_VICT;
}

// CORRECT - Check for -1 death sentinel
int dam = reconcileDamage(victim, ...);
if (dam == -1) {
  return DELETE_VICT;
}
```

Never check `reconcileDamage()` with `IS_SET_DELETE()`. The -1 return is not a flag.

### die() Return Value

Remember that `die()` returns `DELETE_THIS`, not `DELETE_VICT`. The dying being signals its own deletion.

```cpp
// WRONG - Checking for wrong flag
int rc = victim->die(DAMAGE_NORMAL);
if (IS_SET_DELETE(rc, DELETE_VICT)) {  // Never triggers!
  delete victim;
}

// CORRECT - die returns DELETE_THIS
int rc = victim->die(DAMAGE_NORMAL);
if (IS_SET_DELETE(rc, DELETE_THIS)) {
  delete victim;  // Or translate: return DELETE_VICT;
}
```

Always translate appropriately when propagating: if you called `victim->die()` and it returns `DELETE_THIS`, return `DELETE_VICT` to your caller since the victim is their "vict".

### Group Cleanup

Always call `reformGroup()` before deleting any character. Failing to do so leaves followers with dangling master pointers.

```cpp
// WRONG - Followers left with dangling master pointer
if (IS_SET_DELETE(rc, DELETE_THIS)) {
  delete ch;
  ch = nullptr;
}

// CORRECT - Reform group first
if (IS_SET_DELETE(rc, DELETE_THIS)) {
  ch->reformGroup();
  delete ch;
  ch = nullptr;
}
```

## Reference

### Death Penalties

| Aspect | Formula/Behavior |
|--------|------------------|
| XP loss | min(current_exp/5, 25 * mob_exp(level)) |
| PvP XP loss | Normal formula divided by 10 |
| Age increase | Random 0-3 years (0 for level 10 and below) |
| Arena exemption | No XP loss, no age increase |
| Free death affect | Decrements modifier, skips penalties |

### PC vs NPC Death Differences

| Aspect | PC | NPC |
|--------|----|----|
| XP loss | Yes | No |
| Age penalty | Yes | No |
| Corpse type | TPCorpse (vnum -2) | TCorpse (mob vnum) |
| Corpse flags | CORPSE_NO_REGEN | None unless vnum < 0 |
| Group handling | reformGroup() | None |
| Rent cleanup | removeRent() | N/A |
| Follower cleanup | removeFollowers() | N/A |
| Permadeath logging | Yes | No |
| Save location | Room::NOWHERE | N/A |
| Limb healing | All limbs restored | N/A |
| Disease cleanup | All diseases removed | N/A |

### Death Flow Pipeline

```
die()
  ├─ Check arena exemption (ROOM_ARENA)
  ├─ Check free death affect (AFFECT_FREE_DEATHS)
  ├─ Apply XP penalty via deathExp()
  ├─ Apply age penalty (random 0-3 years)
  ├─ Check for transformation, call doReturn() if needed
  └─ Call rawKill()

rawKill()
  ├─ Call stopFighting(), then separately remove berserk and vampire bite affects
  ├─ Call makeCorpse() - creates corpse, transfers equipment
  ├─ Call death cry
  ├─ Call genericKillFix() - group reform, dispel magic, cleanup
  ├─ PC-only: removeRent(), removeFollowers(), permadeath log
  └─ Return DELETE_THIS
```

### Exemptions

| Exemption | Check | Effect |
|-----------|-------|--------|
| Arena | `ROOM_ARENA` flag | No XP loss, no age increase, no limb healing |
| Free Deaths | `AFFECT_FREE_DEATHS` modifier > 0 | Decrements modifier, skips all penalties |
| Low Level | Level <= 10 | No age penalty |

## Implementation

### XP Loss Calculation

Death XP loss uses the minimum of two formulas:
- 20% of current experience (`current_exp / 5`)
- 25 times the mob XP value for the character's level (`25 * mob_exp(level)`)

This caps losses for high-level characters while ensuring low-level characters don't lose more than they can afford.

PvP deaths divide the result by 10, significantly reducing the penalty for player-versus-player combat.

### Corpse Creation

The `makeCorpse()` function branches based on PC vs NPC:
- For PCs: Creates a `TPCorpse` with vnum -2
- For NPCs: Creates a `TCorpse` with the mob's vnum

All PC corpses receive `CORPSE_NO_REGEN` which prevents the cleanup proc from destroying them. NPC corpses only get this flag if their vnum is negative.

Equipment transfer iterates through all wear slots, calls `unequip()` to handle affect removal, then transfers items to the corpse. Experience loss is embedded in the corpse as a float value for corpse retrieval systems.

### Shopkeeper Death Handling

Shopkeeper special handling deletes all inventory and money since shopkeepers maintain infinite virtual inventory that should not transfer to corpses.

### Generic Death Cleanup

`genericKillFix()` performs universal cleanup:
- Calling `reformGroup()` to transfer leadership
- Removing the dying character from mob hate/fear lists
- Dispelling magic with double-death safety checks
- Resetting hunger/thirst
- Restoring limbs for PCs (except arena)
- Curing diseases for PCs
- Setting shamans to 25 HP with 50 lifeforce

### Double-Death Detection

`genericKillFix()` checks return values from `generic_dispel_magic()` and `genericChaseSpirits()` for `DELETE_VICT` and logs if detected. This catches cases where spell wearoff effects trigger another death.

### Group Leadership Transfer

When a group leader dies, `reformGroup()` transfers leadership to the first eligible follower. It uses a two-pass algorithm preferring PC followers, falling back to any follower. All remaining followers re-attach to the new leader. The `AFF_GROUP` flag is maintained on all members.

This must be called before deletion or followers will have dangling master pointers.

### Arena Death Exemption

Deaths in rooms with `ROOM_ARENA` flag:
- Skip XP loss
- Skip age increase
- Do not heal limbs
- Save the character to the current room rather than `Room::NOWHERE`

## Troubleshooting

### Bug: Death Not Detected

**Symptom:** Code continues after `reconcileDamage()` when victim should be dead.

**Cause:** Checking for DELETE flags instead of -1 return value.

**Diagnostic:** Check how the return value is tested. `IS_SET_DELETE()` will not detect -1.

**Fix:** Compare return value against -1 directly: `if (dam == -1) return DELETE_VICT;`

### Bug: Wrong DELETE Flag Checked

**Symptom:** DELETE flag check never triggers despite death occurring.

**Cause:** Checking wrong flag. Common mistake: checking `DELETE_VICT` when `die()` returns `DELETE_THIS`.

**Diagnostic:** Check what the called function actually returns. Methods called on self return `DELETE_THIS` for self-deletion.

**Fix:** Check `DELETE_THIS` when the method was called on the dying object. Translate appropriately for your return value.

### Crash: Dangling Follower Pointers

**Symptom:** Crash when iterating followers or accessing group after character death.

**Cause:** Character was deleted without calling `reformGroup()` first.

**Diagnostic:** Check if `reformGroup()` was called before the delete statement.

**Fix:** Always call `reformGroup()` before deleting any character.

### Bug: PC Corpse Disappears Immediately

**Symptom:** Player corpse is gone when player returns to retrieve it.

**Cause:** `CORPSE_NO_REGEN` flag not set on PC corpse.

**Diagnostic:** Check corpse creation path for PCs.

**Fix:** Ensure `TPCorpse` creation sets `CORPSE_NO_REGEN`.

### Bug: Equipment Not In Corpse

**Symptom:** Player equipment missing after death.

**Cause:** `unequip()` failed or items were not transferred to corpse.

**Diagnostic:** Check `makeCorpse()` equipment transfer loop.

**Fix:** Ensure all wear slots are iterated and items properly transferred.

### Bug: XP Loss Too High

**Symptom:** Player loses more XP than expected on death.

**Cause:** Death penalty applied multiple times, or min() calculation incorrect.

**Diagnostic:** Check death processing path for duplicate `deathExp()` calls.

**Fix:** Ensure `die()` is only called once per death event.
