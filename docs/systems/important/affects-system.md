---
title: Affects System
description: Temporary and permanent modifications to beings including spells, diseases, equipment bonuses, and status effects
category: important
keywords: [renewal, affect duration, affect stacking, spell buff, temporary modification, permanent modification, status effect]
primary_symbols:
  functions: [affectTo, affectJoin, affectJoin2, getMod, setMod, canBeRenewed, getProtection, addToProtection, updateAffects]
  classes: [affectedData, TBeing]
  enums: [applyTypeT, APPLY_IMMUNITY, APPLY_SPELL, APPLY_PROTECTION, APPLY_ARMOR, APPLY_HITROLL, APPLY_DAMROLL, PERMANENT_DURATION, AFF_SANCTUARY, AFF_BLIND, AFF_STUNNED, AFF_PARALYSIS, AFF_WEB, AFF_SNEAK, AFF_HIDE, AFF_RIPOSTE, AFF_FOCUS_ATTACK, AFF_ENGAGER, AFF_INVISIBLE, AFF_FLYING, AFF_POISON, AFF_CHARM, AVG_DUR_NO, AVG_DUR_YES, AVG_EFF_NO, AVG_EFF_YES]
---

## Overview

How does a temporary spell buff know when to expire? How does sanctuary reduce incoming damage? How do multiple protection sources combine?

The affects system manages all temporary and permanent modifications to beings: spells, diseases, skills, equipment bonuses, and status effects. Every buff, debuff, poison, and magical enhancement flows through this unified system.

An affect is a structured modification attached to a being. Each affect specifies what it modifies (a stat, immunity, or skill), by how much, for how long, and optionally which AFF_* flags to set while active. The system handles duration tracking, renewal eligibility, stacking behavior, and automatic cleanup when effects expire.

The renewal mechanic prevents indefinite buff stacking. Each affect has a renewal threshold, typically half its original duration. Until duration drops below this threshold, recasting the same spell has no effect. This creates natural buff maintenance cycles and prevents trivial permanent buffs.

When an affect expires, the system automatically reverses its modifications and clears any associated AFF_* flags, but only if no other source provides that flag. A character with both a sanctuary spell and sanctuary-granting equipment keeps the AFF_SANCTUARY flag when the spell expires.

---

## Patterns

### Always Check affectJoin() Return Value

`affectJoin()` returns FALSE when an affect exists but cannot be renewed (duration > renew threshold). Ignoring this causes spells to appear successful when they did nothing.

When `text = TRUE` (default), affectJoin sends a message to the caster. When `text = FALSE`, failure is silent and you must handle it explicitly.

For multi-affect spells, check each affectJoin call. Use `text = FALSE` on subsequent calls to suppress duplicate messages.

### Use getMod() and setMod() for APPLY_IMMUNITY and APPLY_SPELL

For `APPLY_IMMUNITY` and `APPLY_SPELL`, the modifier fields have swapped meanings. The `modifier` field contains the immunity/skill type, while `modifier2` contains the actual value. The accessor methods handle this automatically.

Never access `modifier` and `modifier2` directly when location might be `APPLY_IMMUNITY` or `APPLY_SPELL`.

### Use -1 Renew for Permanent Affects

When applying affects with `PERMANENT_DURATION`, pass `-1` as the renew parameter to `affectTo()`. This marks the affect as never renewable, which is the correct semantic for permanent effects.

### Choose the Right Application Function

Use `affectTo()` for direct application when you know exactly what you want. Use `affectJoin()` when you need automatic stacking and renewal behavior. Use `affectJoin2()` when you need fine-grained control via flags.

### Never Set Bitvectors That Other Sources Provide

The system automatically clears bitvectors when affects expire, but only if no other source provides that flag. However, when applying affects, check whether the target already has the desired bitvector from another source. If the bitvector is already set by equipment or racial ability, setting it again in your spell's affect creates confusing behavior when either source is removed.

### Protection Stacks Additively and Clamps

Multiple `APPLY_PROTECTION` sources stack into a single value clamped to [-100, 100]. Design protection amounts knowing they contribute to a shared pool. Sanctuary at 25% plus Aura of Guardian at 12% yields 37% total protection.

### Validate desc and desc->original for Polymorph Affects

`SPELL_POLYMORPH`, `SKILL_DISGUISE`, and `SPELL_SHAPESHIFT` require valid descriptor and original character references. The periodic update system automatically removes these affects if either becomes null. Never assume `desc->original` remains valid throughout the affect's lifetime.

---

## Reference

### Symbol Quick Reference

| Symbol | Type | Purpose |
|--------|------|---------|
| `affectedData` | class | Data structure for a single affect instance |
| `affectTo()` | function | Direct affect application |
| `affectJoin()` | function | Apply with automatic stacking/renewal |
| `affectJoin2()` | function | Apply with fine-grained flag control |
| `getMod()` | method | Get modifier value (handles APPLY_IMMUNITY/SPELL swap) |
| `setMod()` | method | Set modifier value (handles APPLY_IMMUNITY/SPELL swap) |
| `canBeRenewed()` | method | Check if affect is below renewal threshold |
| `getProtection()` | method | Get total protection percentage |
| `addToProtection()` | method | Modify protection with clamping |
| `updateAffects()` | function | Per-tick affect processing |
| `PERMANENT_DURATION` | constant | Duration value for permanent affects |

### Apply Locations

38 apply types define what an affect modifies:

| Category | Examples |
|----------|----------|
| Stats | `APPLY_STR`, `APPLY_INT`, `APPLY_WIS`, `APPLY_DEX`, `APPLY_CON`, `APPLY_CHA`, `APPLY_AGI`, `APPLY_KAR`, `APPLY_SPE`, `APPLY_FOC` |
| Resources | `APPLY_HIT`, `APPLY_MANA`, `APPLY_MOVE`, `APPLY_LIFEFORCE` |
| Combat | `APPLY_HITROLL`, `APPLY_DAMROLL`, `APPLY_ARMOR` |
| Special | `APPLY_IMMUNITY`, `APPLY_SPELL`, `APPLY_DISCIPLINE`, `APPLY_PROTECTION` |

### Modifier Field Semantics

| Location | `modifier` | `modifier2` |
|----------|------------|-------------|
| Normal applies | Effect value | Unused |
| `APPLY_IMMUNITY` | Immunity type | Immunity amount |
| `APPLY_SPELL` | Skill/spell ID | Skill bonus |

### Duration Values

| Value | Meaning |
|-------|---------|
| -9 (`PERMANENT_DURATION`) | Permanent, never decays |
| 0 | Expires immediately on next tick |
| Positive integer | Ticks remaining before expiration |

### Renew Parameter Values

| Value | Meaning |
|-------|---------|
| -1 | Never renewable |
| 0 | Auto-calculate as duration / 2 |
| Positive integer | Explicit renewal threshold in ticks |

### AFF_* Combat Flags

| Flag | Combat Effect |
|------|---------------|
| `AFF_BLIND` | Mitigated by SKILL_BLINDFIGHTING; affects hit/defense rolls |
| `AFF_SANCTUARY` | Reduces incoming damage via APPLY_PROTECTION |
| `AFF_STUNNED` | Negates attack/defense bonuses |
| `AFF_PARALYSIS` | Prevents recovery from stunned |
| `AFF_WEB` | -4 attacker special attack; +4 for enemies |
| `AFF_SNEAK` | +5 special attack (thieves, out of combat) |
| `AFF_HIDE` | +5 special attack (thieves, out of combat) |
| `AFF_RIPOSTE` | Extra attack after successful parry |
| `AFF_FOCUS_ATTACK` | Guarantees next hit succeeds |
| `AFF_ENGAGER` | Engaged but not actively fighting |
| `AFF_INVISIBLE` | Not visible to normal sight |
| `AFF_FLYING` | Can traverse air sectors |
| `AFF_POISON` | Taking poison damage over time |
| `AFF_CHARM` | Under mental control of another |

### Combat Spell Effects

| Spell | Effect Type | Value |
|-------|-------------|-------|
| `SPELL_SANCTUARY` | Protection | Up to 25% for mortals |
| `SPELL_AURA_GUARDIAN` | Protection + Defense | +12 protection, +40 defense |
| `SPELL_CRUSADE` | Enemy special attack penalty | -3 |
| `SPELL_SORCERERS_GLOBE` | Limb damage avoidance | +10% |
| `SPELL_SHIELD_OF_MISTS` | Limb damage avoidance + reduction | +10% each |
| `SPELL_BLESS` | Special attack bonus | +1 |
| `SPELL_AURA_MIGHT` | Special attack bonus | +3 |
| `SPELL_CURSE` | Special attack penalty | -2 attacker, +2 enemies |
| `SPELL_STUPIDITY` | Special attack penalty | -1 attacker, +1 enemies |

### Immunity Types

| Type | Damage Category |
|------|-----------------|
| `IMMUNE_SLASH` | Slashing weapon damage |
| `IMMUNE_PIERCE` | Piercing weapon damage |
| `IMMUNE_BLUNT` | Blunt weapon damage |
| `IMMUNE_HEAT` | Fire damage |
| `IMMUNE_COLD` | Cold damage |
| `IMMUNE_ACID` | Acid damage |
| `IMMUNE_ELECTRICITY` | Lightning damage |
| `IMMUNE_POISON` | Poison damage |
| `IMMUNE_NONMAGIC` | Non-magical weapons |
| `IMMUNE_PLUS1`/`PLUS2`/`PLUS3` | Weapons below enchantment level |

### affectJoin Averaging Flags

| Flag | Behavior |
|------|----------|
| `AVG_DUR_NO` | Cumulative durations |
| `AVG_DUR_YES` | Average durations |
| `AVG_EFF_NO` | Cumulative modifiers |
| `AVG_EFF_YES` | Average modifiers |

### affectJoin2 Flags

| Flag | Behavior |
|------|----------|
| `joinFlagCreateOnly` | Only create new, never update |
| `joinFlagUpdateOnly` | Only update existing, never create |
| `joinFlagAllowMultiples` | Allow multiple instances of same type |
| `joinFlagOverwriteDur` | Replace duration entirely |
| `joinFlagUpdateDur` | Update duration using averaging rules |
| `joinFlagAlwaysRenew` | Bypass renewal threshold check |
| `joinFlagAveDur` | Average duration with existing |
| `joinFlagAveMod` | Average modifier with existing |

### Key Files

| File | Contents |
|------|----------|
| `structs.h` | affectedData class definition |
| `handler.cc` | affect application and removal functions |
| `periodic.cc` | tick-based affect processing |
| `being.cc` | protection methods |
| `damage.cc` | damage reduction via protection |
| `disc_*.cc` | Individual spell implementations creating affects |
| `spec_objs.cc` | Object-based affect triggers |

---

## Implementation

### Affect Data Structure

The `affectedData` class in `structs.h` contains: `type` (spell/skill/affect identifier), `level` (caster level), `duration` (ticks remaining), `renew` (threshold for renewal eligibility), `modifier` (primary value), `modifier2` (secondary value for APPLY_IMMUNITY/SPELL), `location` (what gets modified), `bitvector` (AFF_* flags), `be` (associated object reference), and `next` (linked list pointer).

Affects form a singly-linked list attached to each TBeing. Iteration traverses via `next` pointer.

### Renewal Eligibility

The `canBeRenewed()` method returns true when `renew >= 0` and `duration <= renew`. The renewal threshold defaults to half the original duration when `affectTo()` is called with renew = 0. Pass renew = -1 for permanent affects that should never be renewable.

When duration decrements below the renewal threshold, the system notifies the player that the effect can now be renewed. This creates a natural refresh window where recasting becomes effective.

### Application Functions

`affectTo()` directly applies an affect to the linked list, calls `affectModify()` to apply stat changes and set bitvector flags. The renew parameter controls renewal: -1 = never renewable, 0 = auto-calculate as duration/2, positive = explicit threshold.

`affectJoin()` checks for existing affects of the same type. If found and renewable, it merges durations and modifiers according to the averaging flags. If found but not renewable, it returns FALSE. If not found, it creates a new affect. The text parameter controls whether failure messages are sent.

`affectJoin2()` provides flag-based control over join behavior, allowing combinations like "only update existing" or "always allow renewal regardless of threshold."

### Bitvector Suppression

When applying an affect, the system checks whether the target already has the bitvector set by another source. If `isAffected(bitvector)` returns true but `affectedBySpell(type)` returns false, the bitvector field is zeroed. This prevents the affect from "owning" a bitvector that belongs to equipment or racial abilities.

When removing an affect, the system only clears the bitvector if no other source provides it. This prevents sanctuary spell expiration from removing AFF_SANCTUARY when sanctuary-granting equipment remains equipped.

### Protection Calculation

`APPLY_PROTECTION` modifies the `my_protection` field on TBeing via `addToProtection()`. This method clamps the result to [-100, 100]. Negative protection increases damage taken.

`getProtection()` returns `my_protection` directly. Damage reduction in `reconcileDamage()` multiplies damage by `(100 - protection) / 100`.

### Immunity Processing

`APPLY_IMMUNITY` affects modify damage intake in `preProcDam()`. For each immunity affect, the system extracts the immunity type from `modifier` and percentage from `modifier2`. If the immunity type matches incoming damage, damage is reduced by that percentage.

Multiple immunity affects of the same type stack additively: 50% fire immunity + 30% fire immunity = 80% fire immunity.

Weapon immunity types (`IMMUNE_NONMAGIC`, `IMMUNE_PLUS1`, etc.) prevent damage entirely from insufficiently enchanted weapons.

### Stat Modification Mechanics

When an affect is applied or removed, `affectModify()` updates being stats based on the apply location:

- **Stat applies** (STR, INT, etc.): Add/subtract modifier to/from temporary stat, clamp to valid ranges
- **Resource applies** (HIT, MANA, MOVE): Add/subtract from max and current values, adjust current proportionally on removal
- **Combat applies** (HITROLL, DAMROLL, ARMOR): Direct add/subtract of modifier
- **Protection applies**: Call `addToProtection()` with clamping
- **Immunity/Spell applies**: Stored in affect list, queried during damage/skill checks

### Combat Integration

Hit resolution in `hits()` checks `AFF_FOCUS_ATTACK` first for guaranteed success. Otherwise, it computes `attackRound() - defendRound()` to get a modifier. Both functions check visibility (blind penalties), position, and spell-task penalties.

`defendRound()` adds +40 bonus for `SPELL_AURA_GUARDIAN`.

Special attacks flow through `specialAttack()` which calls `specAttackMod()` for situational modifiers. This function checks both attacker and defender for relevant affects. Attacker penalties are applied directly; defender conditions are inverted to help the attacker.

Damage flows through `reconcileDamage()` which calls `getActualDamage()` for immunity checks, then applies protection percentage reduction.

### Limb Damage Protection

Certain spells provide percentage chances to avoid limb damage entirely. `SPELL_SORCERERS_GLOBE` adds 10%, `SPELL_SANCTUARY` adds 15%, `SPELL_SHIELD_OF_MISTS` adds 10% avoidance plus 10% reduction. These stack additively up to 35% avoidance with all three active.

### Tick Processing

`updateAffects()` runs each game tick. It decrements duration for non-permanent affects, sends renewal eligibility notifications, and triggers wear-off processing at duration 0. Polymorph-related affects (`SPELL_POLYMORPH`, `SKILL_DISGUISE`, `SPELL_SHAPESHIFT`) are immediately removed if the descriptor or original character pointer becomes null.

### Permanent Affects

Affects with `PERMANENT_DURATION` (-9) skip duration decrement in `updateAffects()`. These are used for permanent diseases, encampment state, combat affects, pet/thrall bonds, and bleeding wounds.

### Special Affect Categories

**Encampment:** `SKILL_ENCAMP` creates a permanent affect with a room object reference in the `be` field. When the character enters the encamped room, rest/recovery bonuses trigger.

**Combat Affects:** Many combat-initiated affects use `PERMANENT_DURATION` and are cleaned up manually when combat ends or the character dies (engagement tracking, combat round timing, temporary combat state).

**Pet/Thrall Bonds:** Charm and domination use permanent affects with special removal conditions. Explicit dispel or death removes them.

**Bleeding Wounds:** Damage-over-time effects use affects with positive duration. Each tick, `updateAffects()` applies damage and decrements duration.

**Disease Progression:** Some diseases progress through stages by replacing the affect with a higher-severity version. Others use modifier values to track disease intensity.

---

## Troubleshooting

### Spell Appears to Succeed But Has No Effect

**Symptom:** Player casts buff, sees cast animation/message, but target stats unchanged.

**Likely cause:** `affectJoin()` returned FALSE due to renewal threshold, but return value was ignored.

**Diagnostic approach:** Check if target already has the affect active. If duration is above renewal threshold, the spell correctly refuses to refresh.

**Fix:** Check `affectJoin()` return value and call `nothingHappens()` or similar on failure. Return spell failure code so mana isn't consumed.

### Bitvector Persists After Spell Expires

**Symptom:** AFF_* flag remains set after spell duration reaches zero.

**Likely cause:** Another source provides the same bitvector (equipment, racial ability, another spell).

**Diagnostic approach:** Check all sources that could set the bitvector: equipment via `affectModify()`, racial innates, other active spells.

**Fix:** This is usually correct behavior. The bitvector clears when all sources are removed.

### Bitvector Disappears While Spell Still Active

**Symptom:** AFF_* flag clears unexpectedly while spell duration remains positive.

**Likely cause:** Equipment providing the same flag was removed, and the removal cleared the shared bitvector.

**Diagnostic approach:** Check if equipment was recently unequipped. The `affectModify()` removal path may have cleared the flag.

**Fix:** This indicates a bitvector ownership bug. The system should check all sources before clearing.

### Protection Value Seems Wrong

**Symptom:** Damage reduction doesn't match expected protection percentage.

**Likely cause:** Multiple protection sources stacking differently than expected, or protection clamped at 100.

**Diagnostic approach:** Sum all `APPLY_PROTECTION` modifiers from active affects. Check if total exceeds 100 (clamped).

**Fix:** Protection is clamped to [-100, 100]. Design protection values knowing they share a pool.

### Immunity Affect Not Preventing Damage

**Symptom:** `APPLY_IMMUNITY` affect exists but damage of the immune type still occurs.

**Likely cause:** Modifier fields are reversed (immunity amount in modifier, type in modifier2).

**Diagnostic approach:** Check modifier field contains `IMMUNE_*` type constant and modifier2 contains immunity percentage. Verify affect was created using `setMod()` accessor.

**Fix:** Use `setMod()` accessor when setting immunity affects, or explicitly assign to correct fields based on `APPLY_IMMUNITY` semantics.

### Duration Never Decreases

**Symptom:** Affect duration stays constant, never ticking down.

**Likely cause:** Duration set to `PERMANENT_DURATION` (-9) when temporary duration was intended.

**Diagnostic approach:** Check affect duration field value. If -9, it's permanent.

**Fix:** Set duration to appropriate tick count (usually `level * Pulse::UPDATES_PER_MUDHOUR` or similar).

### Polymorph Affect Immediately Removed

**Symptom:** Polymorph/disguise/shapeshift instantly ends after application.

**Likely cause:** Descriptor or `desc->original` became null.

**Diagnostic approach:** Check `updateAffects()` processing. These affects require valid descriptor chain.

**Fix:** Ensure descriptor integrity before applying polymorph affects. These affects cannot exist on mobs without players.

### Renewal Message But Spell Still Fails

**Symptom:** Player sees "The effects of X can now be renewed." but casting X still fails with duration message.

**Likely cause:** Multiple instances of the affect with different renewal states.

**Diagnostic approach:** Check for multiple affects of same type on character (`allowMultiples` flag usage). Verify renew threshold is set correctly.

**Fix:** Ensure only one instance of affect exists (don't use `allowMultiples` for renewable buffs), or adjust renew threshold calculation.

### Affect List Corruption Crash

**Symptom:** Crash when iterating affect list, usually in `updateAffects()` or combat code.

**Likely cause:** Affect was freed without being unlinked from list, or next pointer was not cached before modification.

**Diagnostic approach:** Check for direct delete of `affectedData` without calling `affectRemove()`. Verify all affect removal goes through proper handler functions.

**Fix:** Always use `affectRemove()` to remove affects, never delete directly. Cache next pointer before any operation that might modify the list.
