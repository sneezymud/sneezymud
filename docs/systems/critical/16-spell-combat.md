---
title: Offensive Spell System
description: Damage-dealing magic system with centralized damage formulas, three-layer success system, and 28-type immunity handling across eight spell-casting classes.
category: critical
keywords: [spells, magic, damage, immunity, area spells]
primary_symbols:
  functions: [genericDam, getSkillDam, reconcileDamage, bSuccess, critSuccess, critFail, isLucky, getImmunity, inGroup]
  classes: [TBeing, TMagicItem]
  enums: [spellNumT, skillNumT, SPELL_SUCCESS, SPELL_FAIL, SPELL_CRIT_FAIL, CASTER_DEAD, VICTIM_DEAD, CRIT_S_NONE, CRIT_S_DOUBLE, CRIT_S_TRIPLE, CRIT_S_KILL, CRIT_F_NONE, CRIT_F_HITSELF, CRIT_F_HITOTHER, IMMUNE_HEAT, IMMUNE_COLD, IMMUNE_ACID, IMMUNE_POISON, IMMUNE_AIR, IMMUNE_ENERGY, IMMUNE_ELECTRICITY, IMMUNE_WATER, IMMUNE_EARTH, IMMUNE_HOLY, MIN_SPELL, MAX_SPELL, MAX_SKILL, TYPE_UNDEFINED]
---

# Offensive Spell System

## Overview

The offensive spell system delivers damage-dealing magic across eight classes through 50+ spells organized by discipline. A centralized damage formula ensures consistent balance, while a three-layer success system (spell success, critical success, save check) creates varied combat outcomes. The 28-type immunity system provides fine-grained resistance handling from partial reduction to complete immunity.

Spells return flag combinations indicating success/failure and death states. When a victim dies from spell damage, the caster must propagate this information correctly or crashes result from accessing deleted memory.

## Patterns

**Always check reconcileDamage() for -1, not DELETE flags.** The function returns -1 on victim death, not DELETE_VICT. Using IS_SET_DELETE will never detect the death.

**Always translate spell flags to DELETE flags in magic item versions.** VICTIM_DEAD must become DELETE_VICT, CASTER_DEAD must become DELETE_THIS.

**Always advance iterators before deletion in area spells.** Use `*(it++)` pattern to increment before accessing the element, then deletion is safe.

**Always check immunity before calculating damage.** Complete immunity (100%+) should fail the spell entirely.

**Always return immediately after detecting victim death.** Any access to the victim pointer after reconcileDamage returns -1 causes a crash.

**Always filter group members and invisible targets in area spells.** Use inGroup() for group membership (checks same master AND AFF_GROUP flag). Use canSee() for visibility.

**Always validate class access in player casting versions.** Call the appropriate discipline-specific check before spell execution: `bPassMageChecks()`, `bPassShamanChecks()`, or `bPassClericChecks()`.

**Never continue execution after setting a death flag.** The victim or caster is already deleted at that point.

**Never return VICTIM_DEAD from area spells.** Handle deaths locally with immediate deletion; the flag is for single-target spells only.

## Reference

### Spell Return Flags

| Constant | Bit | Purpose |
|----------|-----|---------|
| SPELL_SUCCESS | 1 << 1 | Spell succeeded |
| SPELL_FAIL | 1 << 2 | Spell failed |
| SPELL_CRIT_FAIL | 1 << 3 | Critical failure |
| CASTER_DEAD | 1 << 6 | Caster died (backfire) |
| VICTIM_DEAD | 1 << 7 | Victim died |

### Critical Success Types

| Type | Effect |
|------|--------|
| CRIT_S_NONE | 1x damage (normal) |
| CRIT_S_DOUBLE | 2x damage |
| CRIT_S_TRIPLE | 3x damage |
| CRIT_S_KILL | 3x damage + special message |

### Critical Failure Types

| Type | Effect |
|------|--------|
| CRIT_F_NONE | Simple failure message via nothingHappens() |
| CRIT_F_HITSELF | Backfire damages caster (check for CASTER_DEAD) |
| CRIT_F_HITOTHER | Redirects spell to random room occupant |

### Key Immunity Types

| Immunity | Damage Type |
|----------|-------------|
| IMMUNE_HEAT | Fire |
| IMMUNE_COLD | Ice |
| IMMUNE_ACID | Acid |
| IMMUNE_POISON | Poison |
| IMMUNE_AIR | Wind/air |
| IMMUNE_ENERGY | Pure energy |
| IMMUNE_ELECTRICITY | Lightning |
| IMMUNE_WATER | Water |
| IMMUNE_EARTH | Earth |
| IMMUNE_HOLY | Holy |

### Damage Modifiers

| Modifier | Effect |
|----------|--------|
| Area effect | 75% damage |
| NPC caster | 52% damage |
| PvP (player target) | 50% damage |
| Save successful | 50% damage |
| Stat modifier | 80-125% based on relevant stat |
| Skill difficulty | 35-110% |

### Message Functions

| Function | Purpose |
|----------|---------|
| CS(spell) | Display critical success message |
| CF(spell) | Display critical failure message |
| SV(spell) | Display save message |

### act() Display Targets

| Target | Recipients |
|--------|------------|
| TO_CHAR | Caster only |
| TO_VICT | Victim only |
| TO_NOTVICT | Other room occupants |
| TO_ROOM | All room occupants |

### Source Files

| File | Content |
|------|---------|
| code/code/misc/skill_dam.cc | genericDam(), getSkillDam() |
| code/code/misc/spell_info.cc | Spell definitions database |
| code/code/misc/crit_combat.cc | critSuccess(), critFail() |
| code/code/misc/being.cc | bSuccess(), isLucky() |
| code/code/misc/damage.cc | reconcileDamage() |
| code/code/misc/immunity.h | Immunity type definitions |
| code/code/disc/disc_*.cc | Individual spell implementations |

## Implementation

### Three-Function Spell Architecture

Every offensive spell requires three function versions.

**Core implementation** takes level, bKnown, and adv_learn parameters. Calculates damage via getSkillDam(), runs success checks with bSuccess(), handles critical success/failure with critSuccess()/critFail(), applies saves with isLucky(), and returns spell flags (SPELL_SUCCESS, VICTIM_DEAD, etc.).

**Magic item version** takes a TMagicItem parameter. Calls core implementation with obj->getMagicLevel() and obj->getMagicLearnedness(). Translates returned spell flags to DELETE flags: VICTIM_DEAD becomes DELETE_VICT, CASTER_DEAD becomes DELETE_THIS.

**Player casting version** takes only caster and victim. Validates class access with the appropriate discipline-specific check (`bPassMageChecks()`, `bPassShamanChecks()`, or `bPassClericChecks()`), retrieves player statistics via getSkillLevel()/getSkillValue()/getAdvLearning(), calls core implementation, and propagates CASTER_DEAD to DELETE_THIS.

### genericDam() Formula

Base damage: `classAmt * lag_rounds * caster_level`

Class amount ranges from 0.5 to 4.0 providing spell-specific damage scaling. Lag rounds come from lag_t enumeration (0 to 10.8 seconds of casting delay).

Applied modifiers (multiplicative):
1. Skill difficulty modifier via getSkillDiffModifier()
2. Stat modifier via plotStat() for relevant stat (0.8-1.25x)
3. Random variance (+-level/4 to +-level/2)
4. Area effect penalty (0.75x if applicable)
5. NPC caster reduction (0.5195x)
6. PvP reduction (0.5x if target is PC)

### Three-Layer Success System

**Layer 1 - bSuccess():** Calculates success limit from skill difficulty, learning percentage (0-100%), focus stat modifier (0.8-1.25x), and karma stat (0.9-1.125x). Rolls against limit.

**Layer 2 - critSuccess():** After spell succeeds, determines damage multiplier. Returns CRIT_S_NONE through CRIT_S_KILL for 1x-3x damage.

**Layer 3 - isLucky():** Victim save check using spellLuckModifier(). Success halves final damage.

### Critical Failure Handling

When critFail() returns CRIT_F_HITSELF, apply backfire damage (typically half calculated spell damage) to the caster. Check reconcileDamage() for -1 and return SPELL_CRIT_FAIL + CASTER_DEAD if the caster dies.

### Area Effect Implementation

Iterate room contents with StuffIter, advancing before element access. Filter self, group members (inGroup()), immortals, and invisible targets (canSee()). Apply 75% area damage penalty. On death (reconcileDamage returns -1), delete victim immediately and null the pointer. Return SPELL_SUCCESS without VICTIM_DEAD flag.

### Immunity Handling

Check getImmunity() before damage calculation. At 100%+, fail the spell with appropriate message. Below 100%, reduce damage proportionally: `dam = dam * (100 - immunePercent) / 100`.

## Troubleshooting

### Victim death not detected

**Symptom:** Spell continues after victim dies, crashes on next victim access.

**Cause:** Using IS_SET_DELETE() to check reconcileDamage() return value.

**Fix:** Check for -1 directly: `if (reconcileDamage(...) == -1)`.

### Magic item spells don't signal death to caller

**Symptom:** Caller continues using deleted victim after wand/staff cast.

**Cause:** TMagicItem version returns spell flags instead of DELETE flags.

**Fix:** Translate flags: `if (IS_SET(ret, VICTIM_DEAD)) ADD_DELETE(rc, DELETE_VICT);`

### Area spell crash during iteration

**Symptom:** Crash when multiple victims die in area spell.

**Cause:** Iterator invalidated by deletion before advancement.

**Fix:** Use `*(it++)` pattern - increment occurs before dereferencing.

### Spell damages immune targets

**Symptom:** Fire spell damages fire-immune creature.

**Cause:** Missing immunity check before damage application.

**Fix:** Check `getImmunity(IMMUNE_TYPE) >= 100` for complete immunity, apply partial reduction for values below 100.

### Backfire kills caster but caller continues

**Symptom:** Crash after critical failure backfire kills caster.

**Cause:** Not returning CASTER_DEAD flag, or caller not checking for it.

**Fix:** Return `SPELL_CRIT_FAIL + CASTER_DEAD` on self-damage death, translate to DELETE_THIS in wrapper.

### Group member friendly fire

**Symptom:** Area spells damage group members.

**Cause:** Missing inGroup() check in target filtering.

**Fix:** Skip any victim where `caster->inGroup(victim)` returns true before damage application.

### Player casting bypasses class restrictions

**Symptom:** Players cast spells their class shouldn't know.

**Cause:** Missing discipline-specific validation check.

**Fix:** Call the appropriate check (`bPassMageChecks()`, `bPassShamanChecks()`, or `bPassClericChecks()`) before spell execution; return FALSE with error message on failure.
