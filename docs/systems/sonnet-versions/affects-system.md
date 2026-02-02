---
title: Affects System
category: critical
keywords: [affectedData, affectTo, affectJoin, affectJoin2, APPLY_IMMUNITY, APPLY_SPELL, APPLY_PROTECTION, PERMANENT_DURATION, AFF_SANCTUARY, AFF_BLIND, canBeRenewed, getProtection, getMod, setMod]
related: [tohit-defense.md, combat-formulas.md, combat-rounds.md, spell-combat.md]
primary_symbols:
  functions: [affectTo, affectJoin, affectJoin2, canBeRenewed, getMod, setMod, getProtection, addToProtection, affectModify, updateAffects]
  classes: [affectedData, TBeing]
  files: [code/code/misc/structs.h, code/code/sys/handler.cc, code/code/misc/periodic.cc, code/code/misc/being.cc, code/code/misc/combat.cc, code/code/misc/damage.cc]
---

## Overview

Why does casting sanctuary twice within 10 seconds fail silently while waiting 30 seconds allows renewal? The affects system manages this through duration thresholds that prevent indefinite stacking while allowing legitimate refreshes.

The affects system manages temporary and permanent modifications to beings. Spells, diseases, skill bonuses, equipment stats, and other status effects all flow through this single unified mechanism. Each modification is represented as an affect entry containing what gets modified, by how much, for how long, and under what conditions it can be renewed or stacked.

Your character's combat performance depends heavily on active affects. Sanctuary reduces incoming damage. Bless improves attack success. Webbing penalizes special attacks. Blindness cripples both offense and defense. The system handles all these modifications through a consistent interface while allowing each affect to combine with others according to specific stacking rules.

Affects exist in three temporal categories: temporary (duration counted in ticks), permanent (special duration value indicating infinite), and renewable (temporary affects that allow refreshing when duration falls below a threshold). The renewal threshold prevents stacking exploitation - you cannot refresh sanctuary when it has 90 seconds remaining, but can when it drops to 45 seconds. This creates strategic timing windows for buff maintenance.

The system distinguishes between apply locations (what gets modified - strength, armor, immunity) and bitvector flags (binary states like invisible, flying, charmed). Some affects use only locations, some only flags, many use both. A sanctuary spell sets both a protection apply location (reducing damage percentage) and the AFF_SANCTUARY flag (for visibility and dispel targeting).

Combat integration occurs at multiple decision points. Hit resolution checks blind flags. Special attacks accumulate modifier bonuses from multiple spell effects. Damage reduction applies protection percentages from stacked defensive spells. Each integration point queries the affect list independently, allowing complex emergent interactions.

Common scenario: A player casts sanctuary, then 20 seconds later tries to refresh it. The affect has duration 100 and renew threshold 50. The system checks canBeRenewed(), finds duration 80 exceeds threshold 50, and affectJoin() returns FALSE. The spell fails with a message about duration limits. Thirty seconds later, duration has decayed to 40, below the 50 threshold. Now affectJoin() succeeds, resetting duration to 100.

## Patterns

### Always Check affectJoin Return Value

affectJoin() returns FALSE when an affect exists but cannot be renewed due to duration exceeding the renew threshold. Ignoring this causes spells to appear successful while doing nothing.

```cpp
// WRONG: Ignores renewal failure
victim->affectJoin(caster, &aff, AVG_DUR_NO, AVG_EFF_YES);
act("$n's skin turns to hard granite.", FALSE, victim, NULL, NULL, TO_ROOM);
return SPELL_SUCCESS;  // Bug: reports success even when renewal blocked

// CORRECT: Check return and handle failure
if (!victim->affectJoin(caster, &aff, AVG_DUR_NO, AVG_EFF_YES)) {
    caster->nothingHappens();
    return FALSE;
}
act("$n's skin turns to hard granite.", FALSE, victim, NULL, NULL, TO_ROOM);
return SPELL_SUCCESS;
```

When text parameter is TRUE (default), affectJoin sends "You can't increase the duration of that effect any further." to the caster. When FALSE, failure is silent and you must handle messaging yourself.

Multi-affect spells require checking each affect separately. Use text parameter to suppress duplicate failure messages:

```cpp
bool success = false;
if (v->affectJoin(c, &aff1, AVG_DUR_NO, AVG_EFF_YES))
    success = true;

if (success) {
    if (v->affectJoin(c, &aff2, AVG_DUR_NO, AVG_EFF_YES))
        success = true;
} else {
    // First failed, suppress second's message
    if (v->affectJoin(c, &aff2, AVG_DUR_NO, AVG_EFF_YES, FALSE))
        success = true;
}
return success;
```

**Why this matters:** Players see spell success messages and lose mana even when the spell did nothing. This creates confusion and feels like a bug. Checking the return value allows proper failure handling.

### Use getMod and setMod for Modifier Fields

For APPLY_IMMUNITY and APPLY_SPELL locations, the modifier and modifier2 fields have swapped semantics. Always use accessor methods:

```cpp
// WRONG: Direct field access breaks on APPLY_IMMUNITY/APPLY_SPELL
aff.modifier = 50;  // Is this immunity amount or immunity type? Depends on location.

// CORRECT: Accessor handles the swap automatically
aff.setMod(50);    // Sets correct field regardless of location type
long value = aff.getMod();  // Returns correct value regardless of location type
```

The swap exists because APPLY_IMMUNITY stores immunity type in modifier and immunity amount in modifier2, while normal applies store the primary value in modifier. The accessor methods abstract this away.

**Why this matters:** Direct field access on APPLY_IMMUNITY affects will put the immunity amount in the type field and vice versa, causing the affect to do nothing or apply the wrong immunity.

### Never Set Renew Parameter to 0 for Permanent Affects

Permanent affects (duration = PERMANENT_DURATION) should use renew = -1 to prevent renewal:

```cpp
// WRONG: Permanent affect with default renew
aff.duration = PERMANENT_DURATION;
victim->affectTo(&aff);  // Uses renew = 0, which auto-calculates as duration/2

// CORRECT: Explicitly prevent renewal
aff.duration = PERMANENT_DURATION;
victim->affectTo(&aff, -1);  // -1 = never renewable
```

When renew = 0 (default), affectTo calculates it as duration / 2. For permanent affects (duration = -9), this creates nonsensical renewal logic.

**Why this matters:** Permanent affects like encampment or pet bonds should never be renewable through normal spell casting. Allowing renewal creates edge cases where players can reapply permanent effects.

### Don't Remove Bitvectors Provided by Other Sources

When removing an affect, check if the same bitvector is provided by equipment, race, or other affects before clearing it:

```cpp
// The system handles this automatically in affectRemove (handler.cc)
// When your affect expires, this check prevents removing shared bitvectors:
if (af->bitvector && isAffected(af->bitvector) && !affectedBySpell(af->type))
  af->bitvector = 0;  // Don't remove flag still provided by other source
```

This pattern is built into the handler. You don't need to implement it manually, but you must understand it exists. If a player has AFF_INVISIBLE from both an invisibility spell and a ring of invisibility, the spell expiring should not remove the flag.

**Why this matters:** Players would lose flags from equipment or racial abilities when temporary spell effects expire. A ring of flying would stop working when a fly spell wears off.

### Check Protection Clamping for Damage Reduction

Protection values stack additively but clamp to [-100, 100]:

```cpp
void TBeing::addToProtection(short num) {
  my_protection = (byte)min(max((int)(num + my_protection), -100), 100);
}
```

When implementing protection-granting affects, be aware that multiple sources combine. Sanctuary at 25% plus Guardian Aura at 12% results in 37% damage reduction. Design spell strength assuming players will stack multiple protection sources.

**Why this matters:** High-level players can approach the 100% reduction cap by stacking all available protection spells. This makes them nearly invulnerable. Balance new protection sources with this in mind.

### Validate desc and desc->original for Polymorph Affects

SPELL_POLYMORPH, SKILL_DISGUISE, and SPELL_SHAPESHIFT require valid descriptor and original character references. The periodic update system automatically removes these affects if either becomes null:

```cpp
// From periodic.cc updateAffects processing:
// System automatically cleans up polymorph affects on invalid descriptor
if (!desc || !desc->original) {
  // Affect is immediately removed
}
```

When implementing polymorph-related code, never assume desc->original remains valid throughout the affect's lifetime. Linkdeath, switching, or other descriptor changes can invalidate it.

**Why this matters:** Accessing null desc->original causes crashes. The auto-cleanup prevents persistence of polymorph state when the descriptor state is invalid.

## Reference

### Symbol Quick Reference

| Symbol | Type | Purpose |
|--------|------|---------|
| `affectedData` | class | Represents single affect modification |
| `affectTo` | function | Directly applies affect with optional renew threshold |
| `affectJoin` | function | Merges affect with averaging/cumulative rules |
| `affectJoin2` | function | Flexible affect merging with flag-based control |
| `affectModify` | function | Apply/remove stat modifications from affect |
| `updateAffects` | function | Per-tick affect processing (duration decay, expiration) |
| `canBeRenewed` | function | Checks if affect duration allows renewal |
| `getMod` | function | Returns modifier value with APPLY_IMMUNITY/SPELL handling |
| `setMod` | function | Sets modifier value with APPLY_IMMUNITY/SPELL handling |
| `getProtection` | function | Returns current protection percentage from all sources |
| `addToProtection` | function | Adds protection value with clamping to [-100, 100] |
| `PERMANENT_DURATION` | constant | Duration value (-9) indicating permanent affect |
| `AFF_SANCTUARY` | flag | Bitvector flag indicating sanctuary status |
| `AFF_BLIND` | flag | Bitvector flag indicating blindness |
| `AFF_WEB` | flag | Bitvector flag indicating webbed status |
| `AFF_FOCUS_ATTACK` | flag | Guarantees next hit succeeds |
| `APPLY_PROTECTION` | enum | Apply location for damage reduction percentage |
| `APPLY_IMMUNITY` | enum | Apply location for damage immunity (uses swapped modifiers) |
| `APPLY_SPELL` | enum | Apply location for skill bonuses (uses swapped modifiers) |
| `APPLY_HITROLL` | enum | Apply location for attack bonus |
| `APPLY_DAMROLL` | enum | Apply location for damage bonus |

### Apply Locations

39 apply location types exist (defined in `code/code/misc/enum.h`):

| Category | Locations |
|----------|-----------|
| Stats | APPLY_STR, APPLY_INT, APPLY_WIS, APPLY_DEX, APPLY_CON, APPLY_AGI, APPLY_KAR, APPLY_SPE, APPLY_FOC |
| Resources | APPLY_HIT, APPLY_MANA, APPLY_MOVE, APPLY_LIFEFORCE |
| Combat | APPLY_HITROLL, APPLY_DAMROLL, APPLY_ARMOR |
| Special | APPLY_IMMUNITY, APPLY_SPELL, APPLY_DISCIPLINE, APPLY_PROTECTION |

### AFF_* Flags

35 bitvector flags exist (defined in `code/code/misc/defs.h`):

| Flag | Purpose |
|------|---------|
| AFF_BLIND | Cannot see, combat penalties |
| AFF_INVISIBLE | Not visible to normal sight |
| AFF_SANCTUARY | Damage reduction, glowing aura |
| AFF_FLYING | Can traverse air sectors |
| AFF_POISON | Taking poison damage over time |
| AFF_PARALYSIS | Cannot move or fight effectively |
| AFF_CHARM | Under mental control of another |
| AFF_SNEAK | Moving quietly, harder to detect |
| AFF_HIDE | Concealed, harder to detect |
| AFF_STUNNED | Position-based penalty, negates bonuses |
| AFF_WEB | Entangled, combat penalties |
| AFF_RIPOSTE | Can counterattack after parry |
| AFF_FOCUS_ATTACK | Next hit guaranteed to succeed |
| AFF_ENGAGER | Engaged but not actively fighting |

### Modifier Field Semantics

| Location | modifier | modifier2 |
|----------|----------|-----------|
| Normal applies | Effect value | Unused |
| APPLY_IMMUNITY | Immunity type (IMMUNE_*) | Immunity percentage |
| APPLY_SPELL | Skill/spell ID | Skill bonus amount |

### Combat-Affecting Flags

| Flag | Effect | Magnitude |
|------|--------|-----------|
| AFF_BLIND | Cannot see opponent, combat penalties | Severe penalty to hit/defend |
| AFF_SANCTUARY | Damage reduction via APPLY_PROTECTION | Variable, max 25% for mortals |
| AFF_STUNNED | Position-based penalty | Negates all attack/defense bonuses |
| AFF_PARALYSIS | Prevents position recovery | Keeps character stunned |
| AFF_WEB | Special attack penalty | -4 attacker, +4 to enemies |
| AFF_SNEAK | Special attack bonus (thieves, out of combat) | +5 |
| AFF_HIDE | Special attack bonus (thieves, out of combat) | +5 |
| AFF_RIPOSTE | Extra attack after successful parry | Grants additional attack |
| AFF_FOCUS_ATTACK | Guaranteed hit | Bypasses all hit probability |
| AFF_ENGAGER | Different attack handling | Context-dependent |

### Combat-Affecting Spells

| Spell | Effect | Magnitude |
|-------|--------|-----------|
| SPELL_SANCTUARY | Damage reduction | +variable protection (max 25% mortals) |
| SPELL_AURA_GUARDIAN | Defense bonus + damage reduction | +40 defense, +12 protection, -3 enemy attack |
| SPELL_CRUSADE | Enemy attack penalty | -3 special attack modifier |
| SPELL_SORCERERS_GLOBE | Limb damage avoidance | +10% avoid chance |
| SPELL_SHIELD_OF_MISTS | Limb damage avoidance + reduction | +10% avoid, +10% reduction |
| SPELL_BLESS | Attack bonus | +1 special attack modifier |
| SPELL_AURA_MIGHT | Attack bonus | +3 special attack modifier |
| SPELL_CURSE | Attack penalty + enemy bonus | -2 attacker, +2 to enemies |
| SPELL_STUPIDITY | Attack penalty + enemy bonus | -1 attacker, +1 to enemies |

### affectJoin Averaging Modes

| Mode | Duration Behavior | Modifier Behavior |
|------|-------------------|-------------------|
| AVG_DUR_NO | Cumulative (add durations) | Effect depends on AVG_EFF_* |
| AVG_DUR_YES | Average durations | Effect depends on AVG_EFF_* |
| AVG_EFF_NO | Cumulative (add modifiers) | Effect depends on AVG_DUR_* |
| AVG_EFF_YES | Average modifiers | Effect depends on AVG_DUR_* |

### affectJoin2 Flags

| Flag | Effect |
|------|--------|
| joinFlagCreateOnly | Only create new affect, fail if exists |
| joinFlagUpdateOnly | Only update existing affect, fail if not exists |
| joinFlagAllowMultiples | Allow multiple instances of same type |
| joinFlagOverwriteDur | Replace duration instead of averaging/adding |
| joinFlagUpdateDur | Update duration only if new > current |
| joinFlagAlwaysRenew | Bypass renewal threshold check |
| joinFlagAveDur | Average durations when merging |
| joinFlagAveMod | Average modifiers when merging |

### Duration Values

| Value | Meaning |
|-------|---------|
| -9 (PERMANENT_DURATION) | Permanent, never decays |
| 0 | Expires immediately on next tick |
| Positive integer | Ticks remaining before expiration |

### Renew Parameter Values

| Value | Meaning |
|-------|---------|
| -1 | Never renewable |
| 0 | Auto-calculate as duration / 2 |
| Positive integer | Explicit renewal threshold in ticks |

## Implementation

### Core Data Structure

The affectedData class represents a single modification to a being. It stores what gets modified (location), by how much (modifier/modifier2), for how long (duration), what flags it sets (bitvector), and renewal eligibility (renew threshold).

Key fields:
- type: Identifies the source spell/skill/affect type
- level: Caster level, used for dispel mechanics and some affect strength calculations
- duration: Ticks remaining before expiration, or PERMANENT_DURATION (-9) for infinite
- renew: Threshold for renewal eligibility - affect becomes renewable when duration decays below this value
- modifier: Primary effect value (meaning depends on location type)
- modifier2: Secondary value (only used by APPLY_IMMUNITY and APPLY_SPELL)
- location: Which stat/property gets modified (APPLY_STR, APPLY_IMMUNITY, etc.)
- bitvector: AFF_* flags to set on the being
- be: Associated object reference (used by certain affects like SKILL_ENCAMP)
- next: Linked list pointer to next affect

The structure lives in structs.h and is manipulated by functions in handler.cc, periodic.cc, and being.cc.

### Modifier Field Semantics

Normal apply locations use modifier for the effect value and ignore modifier2. APPLY_IMMUNITY and APPLY_SPELL reverse this: modifier holds the type/ID, modifier2 holds the amount/bonus.

For APPLY_IMMUNITY:
- modifier = IMMUNE_HEAT, IMMUNE_COLD, IMMUNE_PIERCE, etc.
- modifier2 = percentage immunity (0-100)

For APPLY_SPELL:
- modifier = skill or spell ID
- modifier2 = bonus amount to that skill

This swap exists for implementation reasons. The getMod and setMod accessor methods abstract it away. getMod returns modifier2 for APPLY_IMMUNITY/SPELL, modifier otherwise. setMod writes to the appropriate field based on location type.

Never access modifier/modifier2 directly on affects unless you verify the location type first. Always prefer the accessors.

### Affect Application Flow

Three primary functions apply affects with different merging semantics:

affectTo applies an affect directly without checking for existing instances. Use this for affects that explicitly allow multiples or when you've already verified uniqueness. The renew parameter sets the renewal threshold: -1 for never renewable, 0 for auto-calculation as duration/2, or explicit threshold value.

affectJoin merges with existing affects of the same type. It takes averaging mode parameters (AVG_DUR_YES/NO, AVG_EFF_YES/NO) that control whether durations and modifiers stack or average. Returns FALSE when renewal is blocked by duration threshold. The text parameter controls whether to send failure messages to the caster.

affectJoin2 provides flag-based control over merging behavior. Use this when you need precise control like "update duration only if new duration exceeds current" or "bypass renewal threshold checking."

The application flow:
1. Check if affect of same type already exists (unless allowing multiples)
2. If exists, check renewal eligibility via canBeRenewed()
3. If renewable or forcing renewal, merge duration/modifier according to mode
4. If new affect, allocate and link into affect list
5. Call affectModify to apply stat changes and set bitvector flags

### Renewal System Logic

The canBeRenewed method checks two conditions: renew >= 0 (not disabled) and duration <= renew (duration has decayed below threshold). When both are true, the affect is eligible for renewal.

Default behavior with renew = 0: affectTo calculates threshold as duration / 2. A 100-tick affect gets renew = 50. The affect becomes renewable when duration decays to 50 or below.

The system sends "The effects of X can now be renewed." message when an affect transitions from non-renewable to renewable. This occurs during periodic update processing when duration ticks down past the threshold.

Permanent affects (duration = -9) should use renew = -1 to disable renewal. Otherwise the division creates nonsensical thresholds.

### Bitvector Handling

The bitvector field contains AFF_* flags stored as a uint64_t bitmask. When an affect is applied, these flags are OR'd into the being's affect flags. When removed, the flags are cleared - but only if no other source provides them.

The suppression logic in handler.cc checks: if this affect sets a bitvector, and the being still has that bitvector set, and the being is not affected by any spell of this type, then clear the bitvector field before removal. This prevents removing flags from equipment or racial abilities.

Example: A player has AFF_INVISIBLE from both an invisibility spell and a ring. The spell expires. The suppression check sees: bitvector is AFF_INVISIBLE, being is still affected by AFF_INVISIBLE (from the ring), being is not affected by the spell anymore. Therefore, clear the spell's bitvector field so removal doesn't clear the flag still needed by the ring.

This happens automatically in affectRemove. Spell implementations don't need special handling.

### Protection Accumulation

Protection reduces incoming damage as a percentage. Multiple sources stack additively into the being's my_protection field, which is clamped to [-100, 100] range.

When an affect with APPLY_PROTECTION is applied, affectModify calls addToProtection with the modifier value. This adds to the current protection, applying min/max clamping.

When removed, affectModify subtracts the value (by calling addToProtection with negative value).

Example stacking:
- Base protection: 0
- Apply SPELL_SANCTUARY (+25): protection = 25
- Apply SPELL_AURA_GUARDIAN (+12): protection = 37
- Apply AFFECT_GUARDIANS_LIGHT (+variable): protection = 37 + value, capped at 100

The damage reduction occurs in reconcileDamage in damage.cc: dam *= (100 - victim->getProtection()) / 100. With 37% protection, incoming 100 damage becomes 63.

Negative protection increases damage taken. This is used for certain debuffs or vulnerabilities.

### Combat Integration Points

Affects modify combat through multiple code paths that query the affect list at specific decision points.

**Hit Resolution Path:**

The hits() function in combat.cc determines whether an attack lands. It calculates a modifier as attackRound() - defendRound(), then uses that modifier in a probability formula.

attackRound() checks:
- canSee(target): If attacker is blind or target is invisible, apply severe penalty
- spelltask: If attacker is casting, apply casting penalty
- position: Stunned/resting/sleeping attackers get penalties

defendRound() checks:
- canSee(attacker): If defender is blind or attacker is sneaking/hidden, penalty
- affectedBySpell(SPELL_AURA_GUARDIAN): +40 defense bonus
- spelltask: If defender is casting, AGI bonus is suppressed
- position: Stunned/resting/sleeping defenders get penalties

Before the probability calculation, AFF_FOCUS_ATTACK is checked. If present, hits() returns GUARANTEED_SUCCESS, bypassing all other logic.

**Special Attack Modifier Path:**

Special attacks (bash, trip, kick, headbutt, etc.) use specialAttack() which calls specAttackMod() to accumulate situational bonuses from affects.

specAttackMod() iterates through attacker affects:
- AFF_SNEAK/AFF_HIDE: +5 each (thieves only, out of combat)
- AFF_WEB: -4
- SPELL_STUPIDITY: -1
- SPELL_CURSE: -2
- SPELL_BLESS: +1
- SPELL_AURA_MIGHT: +3
- Blind check

Then checks defender affects (inverted - helping attacker):
- AFF_WEB: +4
- SPELL_STUPIDITY: +1
- SPELL_CURSE: +2
- SPELL_SANCTUARY: -3
- SPELL_CRUSADE: -3
- SPELL_AURA_GUARDIAN: -3
- Blind check (bonus to attacker)

These modifiers accumulate with base skill success chance to determine outcome.

**Damage Reduction Path:**

Damage flows through reconcileDamage in damage.cc. After armor reduction and weapon type checking, protection is applied:

dam *= (100 - victim->getProtection())
dam /= 100

getProtection() returns the my_protection field, which accumulates all APPLY_PROTECTION modifiers from active affects.

**Limb Damage Protection Path:**

Limb damage (arms/legs wounded in combat) can be avoided by certain protective spells. The check in combat.cc accumulates avoidance chances and damage reduction from:

- SPELL_SORCERERS_GLOBE: +10% avoid
- SPELL_SANCTUARY: +15% avoid
- SPELL_SHIELD_OF_MISTS: +10% avoid, +10% reduction

The avoid chances stack additively (max 35% with all three). If avoid roll succeeds, limb damage is prevented entirely. If it fails, reduction percentage is applied to the limb damage amount.

### Periodic Update Processing

Every game tick, updateAffects() in periodic.cc processes all active affects:

For each affect:
1. Check for polymorph affects with invalid descriptor - remove immediately
2. Decrement duration (skip if PERMANENT_DURATION)
3. If duration just crossed renew threshold, send "can now be renewed" message
4. If duration reached 0, trigger wear-off: send wear-off message, call affectRemove

The wear-off message lookup uses the affect type to find appropriate text. Some affects have custom handlers that trigger additional effects on expiration.

Duration is decremented each tick (roughly 1 second). A 100-duration affect lasts 100 ticks or about 1 minute 40 seconds.

### Immunity Processing

APPLY_IMMUNITY affects modify damage intake in preProcDam in damage.cc. The function checks for immunities matching the damage type:

For each immunity affect:
1. Extract immunity type from modifier field
2. Extract immunity percentage from modifier2 field
3. If immunity type matches incoming damage type, reduce damage by percentage

Multiple immunity affects of the same type stack additively. 50% fire immunity + 30% fire immunity = 80% fire immunity.

Immunity type values (IMMUNE_HEAT, IMMUNE_COLD, etc.) are defined in enum.h. Common types: IMMUNE_SLASH, IMMUNE_PIERCE, IMMUNE_BLUNT, IMMUNE_HEAT, IMMUNE_COLD, IMMUNE_ACID, IMMUNE_ELECTRICITY, IMMUNE_POISON.

The weaponCheck function also checks for weapon immunity types (IMMUNE_NONMAGIC, IMMUNE_PLUS1, IMMUNE_PLUS2, IMMUNE_PLUS3) which prevent damage entirely from insufficiently enchanted weapons.

### Stat Modification Mechanics

When an affect is applied or removed, affectModify updates being stats based on the apply location:

For stat applies (APPLY_STR, APPLY_INT, etc.):
- Apply: Add modifier to temporary stat
- Remove: Subtract modifier from temporary stat
- Clamp to valid ranges

For resource applies (APPLY_HIT, APPLY_MANA, APPLY_MOVE):
- Apply: Add modifier to max and current values
- Remove: Subtract from max, adjust current proportionally

For combat applies (APPLY_HITROLL, APPLY_DAMROLL, APPLY_ARMOR):
- Apply: Add modifier to combat stat
- Remove: Subtract modifier

For protection applies:
- Apply: Call addToProtection(modifier) with clamping
- Remove: Call addToProtection(-modifier) with clamping

For immunity and spell applies:
- Stored in affect list, queried during damage/skill checks
- No direct stat modification

The function handles both addition and removal through a mode parameter. Equipment affects are processed identically to spell affects.

### File Organization

Key files:
- structs.h: affectedData structure definition
- handler.cc: affectTo, affectJoin, affectJoin2, affectRemove, affectModify
- periodic.cc: updateAffects tick processing
- being.cc: getProtection, addToProtection, stat accessors
- combat.cc: hit resolution, special attack modifiers, limb damage
- damage.cc: damage reduction, immunity checking
- disc_*.cc: Individual spell implementations creating affects
- spec_objs.cc: Object-based affect triggers

The affects system is pervasive - nearly every discipline and many spec procs interact with it. Changes to core affect mechanics require careful testing across all affect sources.

### Special Affect Types

**Encampment Affects:**

SKILL_ENCAMP creates a permanent affect with a room object reference in the be field. When the character enters the encamped room, special logic triggers rest/recovery bonuses. The affect persists indefinitely (PERMANENT_DURATION) and uses renew = -1 to prevent renewal.

**Combat Affects:**

Many combat-initiated affects use PERMANENT_DURATION and are cleaned up manually when combat ends or the character dies. Examples include engagement tracking, combat round timing, and temporary combat state.

**Pet/Thrall Bonds:**

Charm and domination use permanent affects with special removal conditions. The affect tracks the relationship but doesn't automatically expire. Explicit dispel or death removes them.

**Bleeding Wounds:**

Damage-over-time effects like bleeding use affects with positive duration to track tick-based damage. Each tick, the updateAffects handler applies damage and decrements duration. When duration reaches 0, bleeding stops.

**Disease Progression:**

Diseases use either permanent or long-duration affects. Some diseases progress through stages by replacing the affect with a higher-severity version. Others use modifier values to track disease intensity.

## Troubleshooting

### Spell Succeeds But Effect Missing

**Symptom:** Spell shows success message and consumes mana, but target does not gain the expected bonus or flag.

**Likely cause:** affectJoin returned FALSE due to renewal threshold blocking, but return value was not checked.

**Diagnostic approach:**
1. Check spell implementation for affectJoin call without return value checking
2. Verify affect has appropriate renew parameter (should be 0 for auto-calculation or explicit threshold)
3. Test by waiting for full duration expiry, then recasting - if it works after full expiry, this confirms renewal blocking

**Fix:** Add return value check to spell implementation and handle failure appropriately (send nothingHappens message, return SPELL_FAIL).

### Protection Affect Applied But No Damage Reduction

**Symptom:** Character has APPLY_PROTECTION affect in affect list, but still takes full damage.

**Likely cause:** Affect uses wrong location type or modifier value is 0.

**Diagnostic approach:**
1. Check affect location field - should be APPLY_PROTECTION
2. Check modifier value - should be non-zero protection percentage
3. Verify getProtection() returns expected value
4. Check damage path in reconcileDamage to ensure protection multiplication is occurring

**Fix:** Correct affect location or modifier value in spell implementation.

### Immunity Affect Not Preventing Damage

**Symptom:** APPLY_IMMUNITY affect exists but damage of the immune type still occurs.

**Likely cause:** Modifier fields are reversed (immunity amount in modifier, type in modifier2).

**Diagnostic approach:**
1. Check modifier field - should contain IMMUNE_* type constant
2. Check modifier2 field - should contain immunity percentage
3. Verify affect was created using setMod accessor, not direct field assignment

**Fix:** Use setMod accessor when setting immunity affects, or explicitly assign to correct fields based on APPLY_IMMUNITY semantics.

### Bitvector Lost When Spell Expires

**Symptom:** Player has bitvector from equipment, but loses it when spell with same bitvector expires.

**Likely cause:** Custom affectRemove implementation not calling standard suppression logic.

**Diagnostic approach:**
1. Check if spell has custom removal code that clears bitvectors manually
2. Verify standard affectRemove is called for this affect type
3. Check equipment provides same bitvector

**Fix:** Remove manual bitvector clearing, rely on automatic suppression logic in handler.cc affectRemove.

### Duration Never Decreases

**Symptom:** Affect duration stays constant, never ticking down.

**Likely cause:** Duration set to PERMANENT_DURATION (-9) when temporary duration was intended.

**Diagnostic approach:**
1. Check affect duration field value - if -9, it's permanent
2. Verify spell implementation sets duration to positive tick count
3. Check for code that might be resetting duration each tick

**Fix:** Set duration to appropriate tick count (usually level * Pulse::UPDATES_PER_MUDHOUR or similar).

### Renewal Message But Spell Still Fails

**Symptom:** Player sees "The effects of X can now be renewed." but casting X still fails with duration message.

**Likely cause:** Multiple instances of the affect with different renewal states, or renewal threshold calculation is wrong.

**Diagnostic approach:**
1. Check for multiple affects of same type on character (allowMultiples flag usage)
2. Verify renew threshold is set correctly (should be duration/2 or explicit value)
3. Check if custom renewal logic interferes with standard canBeRenewed check

**Fix:** Ensure only one instance of affect exists (don't use allowMultiples for renewable buffs), or adjust renew threshold calculation.

### Affect List Corruption Crash

**Symptom:** Crash when iterating affect list, usually in updateAffects or combat code.

**Likely cause:** Affect was freed without being unlinked from list, or next pointer was not cached before modification.

**Diagnostic approach:**
1. Check for direct delete of affectedData without calling affectRemove
2. Verify all affect removal goes through proper handler functions
3. Look for modification of affect list during iteration without caching next pointer

**Fix:** Always use affectRemove to remove affects, never delete directly. Cache next pointer before any operation that might modify the list.
