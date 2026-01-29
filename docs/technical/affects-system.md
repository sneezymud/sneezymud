---
title: Affects System
description: The affects system manages temporary and permanent modifications to beings including spells, diseases, skills, equipment bonuses, and other status effects.
keywords: [affectedData, affectTo, affectJoin, affectJoin2, APPLY_IMMUNITY, APPLY_SPELL, APPLY_PROTECTION, PERMANENT_DURATION, AFF_SANCTUARY, AFF_BLIND, reconcileDamage, getMod, setMod, canBeRenewed, getProtection]
category: Critical Systems
related: [tohit-defense.md, combat-formulas.md, combat-rounds.md, spell-combat.md]
last_updated: 2026-01-29
source_files: [code/code/misc/structs.h, code/code/sys/handler.cc, code/code/misc/periodic.cc, code/code/misc/being.cc, code/code/misc/combat.cc, code/code/misc/damage.cc, code/code/disc/disc_cleric_aegis.cc, code/code/disc/disc_deikhan_auras.cc, code/code/spec/spec_objs.cc]
---

# Affects System

The affects system manages temporary and permanent modifications to beings - spells, diseases, skills, equipment bonuses, and other status effects.

## Core Data Structure

```cpp
class affectedData {
  spellNumT type;       // Spell/skill/affect type (e.g., SPELL_BLESS, AFFECT_DISEASE)
  sbyte level;          // Caster level
  int duration;         // Ticks remaining (-9 = permanent)
  int renew;            // Duration threshold for renewal eligibility
  long modifier;        // Primary value (stat bonus, skill ID, or immunity type)
  long modifier2;       // Secondary value (used by APPLY_IMMUNITY and APPLY_SPELL)
  applyTypeT location;  // What the affect modifies (APPLY_STR, APPLY_IMMUNITY, etc.)
  uint64_t bitvector;   // AFF_* flags to set/clear
  TThing* be;           // Associated object (e.g., room for SKILL_ENCAMP)
  affectedData* next;   // Linked list pointer
};
```

**Files:** `code/code/misc/structs.h:305-348`, `code/code/sys/handler.cc`

## Critical: APPLY_IMMUNITY and APPLY_SPELL Semantics

For `APPLY_IMMUNITY` and `APPLY_SPELL`, the modifier fields have **swapped meanings**:

| Location | `modifier` | `modifier2` |
|----------|------------|-------------|
| Normal applies | Effect value | Unused |
| `APPLY_IMMUNITY` | Immunity type | Immunity amount |
| `APPLY_SPELL` | Skill/spell ID | Skill bonus |

**Always use the accessor methods:**
```cpp
long getMod();       // Returns correct value regardless of location type
void setMod(long v); // Sets correct field regardless of location type
```

## Apply Locations and AFF_* Flags

**39 apply types** (`code/code/misc/enum.h:361-400`): Stats (`APPLY_STR`, `APPLY_INT`, etc.), resources (`APPLY_HIT`, `APPLY_MANA`, `APPLY_MOVE`), combat (`APPLY_HITROLL`, `APPLY_DAMROLL`, `APPLY_ARMOR`), special (`APPLY_IMMUNITY`, `APPLY_SPELL`, `APPLY_DISCIPLINE`).

**35 AFF_* flags** (`code/code/misc/defs.h:84-124`): `AFF_BLIND`, `AFF_INVISIBLE`, `AFF_SANCTUARY`, `AFF_FLYING`, `AFF_POISON`, `AFF_PARALYSIS`, `AFF_CHARM`, `AFF_SNEAK`, `AFF_HIDE`, etc.

## Permanent Duration

```cpp
const int PERMANENT_DURATION = -9;  // code/code/misc/structs.h:285
```

Used for permanent diseases, encampment, combat affects, pet/thrall bonds, and bleeding wounds.

## Affect Application Functions

### affectTo()
```cpp
void TBeing::affectTo(affectedData* af, int renew = 0, silentTypeT silent = SILENT_NO);
```
Directly applies an affect. Renew: `-1` = never renewable, `0` = auto-calculate as `duration/2`, `>0` = explicit threshold.

### affectJoin()
```cpp
int TBeing::affectJoin(TBeing* caster, affectedData* af, avgDurT avg_dur, avgEffT avg_mod, bool text = TRUE);
```
Merges with existing affects: `AVG_DUR_NO/YES` (cumulative/average durations), `AVG_EFF_NO/YES` (cumulative/average modifiers). Returns `FALSE` if affect exists but cannot be renewed.

### affectJoin2()
```cpp
bool TBeing::affectJoin2(affectedData* af, joinFlag flags);
```
Flexible version with flags: `joinFlagCreateOnly`, `joinFlagUpdateOnly`, `joinFlagAllowMultiples`, `joinFlagOverwriteDur`, `joinFlagUpdateDur`, `joinFlagAlwaysRenew`, `joinFlagAveDur`, `joinFlagAveMod`.

## Renewal System

```cpp
bool affectedData::canBeRenewed() const {
  return ((renew >= 0) && (duration <= renew));
}
```

Affects can only be refreshed when `duration <= renew`. The threshold defaults to half the original duration, preventing indefinite stacking. Players see "The effects of X can now be renewed." when an affect becomes renewable.

## Combat Modifiers

Affects interact with combat at multiple points: hit resolution, damage calculation, and special attack modifiers. Understanding these code paths is essential for implementing combat-affecting spells correctly.

### Combat-Affecting AFF_* Flags

The following `AFF_*` flags directly modify combat behavior:

| Flag | Effect | Code Location |
|------|--------|---------------|
| `AFF_BLIND` | Attacker/defender can't see opponent; mitigated by `SKILL_BLINDFIGHTING` | `combat.cc:2740-2750`, `combat.cc:2846-2856`, `combat.cc:3038-3046`, `combat.cc:3087-3095` |
| `AFF_SANCTUARY` | Sets `my_protection` via `APPLY_PROTECTION`; reduces all incoming damage | `handler.cc:471-472`, `damage.cc:385-387` |
| `AFF_STUNNED` | Position-based penalty; negates all attack/defense bonuses | `combat.cc:2917-2919` |
| `AFF_PARALYSIS` | Prevents position recovery from stunned state | `combat.cc:318`, `combat.cc:333-337` |
| `AFF_WEB` | -4 to special attack modifier; +4 to enemies attacking webbed target | `combat.cc:3027-3028`, `combat.cc:3074-3075` |
| `AFF_SNEAK` | +5 special attack modifier for thieves (out of combat only) | `combat.cc:3001-3002` |
| `AFF_HIDE` | +5 special attack modifier for thieves (out of combat only) | `combat.cc:3003-3004` |
| `AFF_RIPOSTE` | Grants extra attack after successful parry | `combat.cc:2511-2514` |
| `AFF_FOCUS_ATTACK` | Guarantees next hit succeeds (bypasses probability) | `combat.cc:2517-2519`, `combat.cc:3224-3225` |
| `AFF_ENGAGER` | Character is engaged but not actively fighting; different attack handling | `combat.cc:2280`, `combat.cc:2318-2320` |

### Spell Affects on Combat

**Beneficial to Defender:**

| Spell | Effect | Code Location |
|-------|--------|---------------|
| `SPELL_SANCTUARY` | `APPLY_PROTECTION` reduces incoming damage by modifier% (max 25 for mortals) | `disc_cleric_aegis.cc:1196-1197` |
| `SPELL_AURA_GUARDIAN` | +40 defense bonus in `defendRound()`; -3 special attack modifier for enemies; +12 protection | `combat.cc:2862-2864`, `combat.cc:3084-3085`, `disc_deikhan_auras.cc:237` |
| `SPELL_CRUSADE` | -3 special attack modifier for enemies | `combat.cc:3082-3083` |
| `SPELL_SORCERERS_GLOBE` | +10% chance to avoid limb damage | `combat.cc:965-966` |
| `SPELL_SHIELD_OF_MISTS` | +10% chance to avoid limb damage; +10% limb damage reduction | `combat.cc:969-972` |

**Beneficial to Attacker:**

| Spell | Effect | Code Location |
|-------|--------|---------------|
| `SPELL_BLESS` | +1 special attack modifier | `combat.cc:3033-3034` |
| `SPELL_AURA_MIGHT` | +3 special attack modifier | `combat.cc:3035-3036` |

**Detrimental:**

| Spell | Effect | Code Location |
|-------|--------|---------------|
| `SPELL_CURSE` | -2 special attack modifier; +2 for enemies attacking cursed target | `combat.cc:3031-3032`, `combat.cc:3078-3079` |
| `SPELL_STUPIDITY` | -1 special attack modifier; +1 for enemies | `combat.cc:3029-3030`, `combat.cc:3076-3077` |

### How Affects Modify Hit/Damage

#### Hit Resolution Path

The `hits()` function (`combat.cc:3207-3256`) determines whether an attack lands. Affects influence this through `attackRound()` and `defendRound()`:

```
hits(victim, mod)
    |
    +-> mod = attackRound(victim) - defendRound(attacker)
    |       |
    |       +-> attackRound() checks:
    |       |   - canSee(target) for blind penalty
    |       |   - spelltask for casting penalty
    |       |   - position for positional modifiers
    |       |
    |       +-> defendRound() checks:
    |           - canSee(attacker) for blind penalty
    |           - affectedBySpell(SPELL_AURA_GUARDIAN) for +40 bonus
    |           - spelltask for AGI bonus suppression
    |           - position for positional modifiers
    |
    +-> AFF_FOCUS_ATTACK -> GUARANTEED_SUCCESS
    +-> factor = 600 + (9 * mod / 5)
    +-> roll against factor
```

**Key code paths:**

- Blind penalty in `attackRound()`: `combat.cc:2740-2750`
- Blind penalty in `defendRound()`: `combat.cc:2846-2856`
- `AFF_FOCUS_ATTACK` check: `combat.cc:3224-3225`

#### Special Attack Modifier Path

Special attacks (bash, trip, kick, etc.) use `specialAttack()` (`combat.cc:3103-3204`) which calls `specAttackMod()` (`combat.cc:2971-3100`) to compute situational bonuses:

```
specialAttack(target, skill, situationalMod, ...)
    |
    +-> situationalMod += specAttackMod(target)
    |       |
    |       +-> Attacker checks:
    |       |   - AFF_SNEAK/AFF_HIDE (thieves only, out of combat)
    |       |   - AFF_WEB (-4)
    |       |   - SPELL_STUPIDITY (-1)
    |       |   - SPELL_CURSE (-2)
    |       |   - SPELL_BLESS (+1)
    |       |   - SPELL_AURA_MIGHT (+3)
    |       |   - canSee(target) for blind penalty
    |       |
    |       +-> Defender checks (inverted - help attacker):
    |           - AFF_WEB (+4)
    |           - SPELL_STUPIDITY (+1)
    |           - SPELL_CURSE (+2)
    |           - SPELL_SANCTUARY (-3)
    |           - SPELL_CRUSADE (-3)
    |           - SPELL_AURA_GUARDIAN (-3)
    |           - canSee(attacker) for blind bonus
    |
    +-> Apply stat modifiers
    +-> Determine success/failure
```

#### Damage Reduction Path

Damage flows through `reconcileDamage()` (`damage.cc:263-365`) which applies protection:

```
reconcileDamage(victim, dam, type)
    |
    +-> getActualDamage(victim, weapon, dam, type)
    |       |
    |       +-> damageTrivia(victim, weapon, dam, type)
    |               |
    |               +-> preProcDam(victim, type, dam)
    |               |       |
    |               |       +-> Check IMMUNE_* types
    |               |       +-> Apply resistance reduction
    |               |
    |               +-> weaponCheck(victim, weapon, type, dam)
    |                       |
    |                       +-> Check IMMUNE_NONMAGIC/PLUS1/PLUS2/PLUS3
    |
    +-> Apply protection (sanctuary, etc.)
    |       dam *= 100 - victim->getProtection()
    |       dam /= 100
    |
    +-> applyDamage(victim, dam, type)
```

**Key code paths:**

- Protection application: `damage.cc:384-387`
- `getProtection()` returns `my_protection`: `being.cc:1265`
- `APPLY_PROTECTION` modifies `my_protection`: `handler.cc:471-472`

### Stacking and Interaction

#### Protection Stacking

Multiple sources of `APPLY_PROTECTION` stack additively into `my_protection`, which is clamped to [-100, 100]:

```cpp
void TBeing::addToProtection(short num) {
  my_protection = (byte)min(max((int)(num + my_protection), -100), 100);
}
```

**Source:** `code/code/misc/being.cc:1269-1271`

**Example stacking:**
- `SPELL_SANCTUARY` at level 50: +25 protection
- `SPELL_AURA_GUARDIAN`: +12 protection
- `AFFECT_GUARDIANS_LIGHT`: variable protection
- Total capped at 100% reduction

#### AFF_* Flag Interactions

`AFF_*` flags are stored in a single `uint64_t` bitmask and don't interfere with each other. Multiple combat-affecting flags can be active simultaneously:

```cpp
// A character can be both sanctuary'd and webbed
if (isAffected(AFF_SANCTUARY))  // Reduces incoming damage
if (isAffected(AFF_WEB))        // Reduces outgoing special attack success
```

However, some spells check for conflicting affects:
- `SPELL_FLAMING_FLESH` conflicts with stone skin effects

#### Buff Priority

Some affects have implicit priority through the order they're checked:

1. `AFF_FOCUS_ATTACK` - Checked first in `hits()`, bypasses all other calculations
2. Position checks - Incapacitated/sleeping targets auto-hit
3. Protection (`getProtection()`) - Applied after all other damage modifiers

#### Limb Damage Protection

`SPELL_SORCERERS_GLOBE`, `SPELL_SANCTUARY`, and `SPELL_SHIELD_OF_MISTS` provide percentage chances to avoid limb damage entirely:

```cpp
// combat.cc:960-974
if (v->affected) {
  int avoidChance = 0;
  double reduction = 0.0;

  if (v->affectedBySpell(SPELL_SORCERERS_GLOBE))
    avoidChance += 10;
  if (v->affectedBySpell(SPELL_SANCTUARY))
    avoidChance += 15;
  if (v->affectedBySpell(SPELL_SHIELD_OF_MISTS)) {
    avoidChance += 10;
    reduction += 10;
  }

  if (avoidChance > 0 && percentChance(avoidChance))
    return FALSE;  // Limb damage avoided
}
```

These stack additively for avoidance chance (max 35% with all three).

### Common Combat Affect Patterns

#### Damage Reduction Affect

```cpp
affectedData aff;
aff.type = SPELL_SANCTUARY;
aff.level = level;
aff.duration = c->durationModify(SPELL_SANCTUARY,
  ((level <= MAX_MORT) ? 3 : level) * Pulse::UPDATES_PER_MUDHOUR);
aff.location = APPLY_PROTECTION;
aff.modifier = min(level / 2, 25);  // Capped at 25% for mortals
aff.bitvector = AFF_SANCTUARY;
victim->affectJoin(caster, &aff, AVG_DUR_NO, AVG_EFF_YES);
```

**Source:** `code/code/disc/disc_cleric_aegis.cc:1189-1201`

#### Defense Buff (Mixed Effects)

```cpp
// SPELL_AURA_GUARDIAN provides both:
// 1. Protection via APPLY_PROTECTION
// 2. Defense bonus checked directly in defendRound()

aff2.type = SPELL_AURA_GUARDIAN;
aff2.level = bKnown;
aff2.duration = AURA_DURATION;
aff2.modifier = 12;
aff2.location = APPLY_PROTECTION;
aff2.bitvector = 0;

// Also adds defense in defendRound():
if (affectedBySpell(SPELL_AURA_GUARDIAN)) {
  bonus += 40;
}
```

**Source:** `code/code/disc/disc_deikhan_auras.cc:233-240`, `code/code/misc/combat.cc:2862-2864`

#### Immunity-Based Protection

```cpp
// SPELL_STONE_SKIN provides immunity to pierce damage
aff2.type = SPELL_STONE_SKIN;
aff2.level = 30;
aff2.duration = 8 * Pulse::UPDATES_PER_MUDHOUR;
aff2.location = APPLY_IMMUNITY;
aff2.modifier = IMMUNE_PIERCE;
aff2.modifier2 = 15;  // 15% pierce immunity
```

**Source:** `code/code/spec/spec_objs.cc:2477-2482`

### Related Documentation

- [To-Hit and Defense System](tohit-defense.md) - Full details on hit resolution formulas
- [Combat Formulas](combat-formulas.md) - Damage calculation details
- [Combat Rounds](combat-rounds.md) - Attack timing and damage pipeline
- [Spell Combat](spell-combat.md) - Spell-specific combat integration

## Gotchas

### affectJoin() Return Value Must Be Checked

`affectJoin()` returns `FALSE` (0) when the affect exists but **cannot be renewed**. This happens when `duration > renew` threshold. **Ignoring this return value causes misleading behavior:**

```cpp
// WRONG: Ignores renewal failure - spell appears to succeed but does nothing
victim->affectJoin(caster, &aff, AVG_DUR_NO, AVG_EFF_YES);
act("$n's skin turns to hard granite.", FALSE, victim, NULL, NULL, TO_ROOM);
return SPELL_SUCCESS;  // Bug: reported success even when renewal was blocked

// CORRECT: Check return and handle failure
if (!victim->affectJoin(caster, &aff, AVG_DUR_NO, AVG_EFF_YES)) {
    caster->nothingHappens();  // Inform player spell didn't work
    return FALSE;              // Return failure, no mana consumed (typically)
}
act("$n's skin turns to hard granite.", FALSE, victim, NULL, NULL, TO_ROOM);
return SPELL_SUCCESS;
```

When `text = TRUE` (default), affectJoin sends "You can't increase the duration of that effect any further." to the caster. When `text = FALSE`, the failure is silent - **you must handle it yourself**.

**Multi-affect spells:** When a spell applies multiple affects, check each one. The `text = FALSE` parameter suppresses duplicate messages:

```cpp
bool success = false;
if (v->affectJoin(c, &aff1, AVG_DUR_NO, AVG_EFF_YES))
    success = true;

if (success) {
    // First succeeded, show message for second failure too
    if (v->affectJoin(c, &aff2, AVG_DUR_NO, AVG_EFF_YES))
        success = true;
} else {
    // First failed silently, suppress second's message too
    if (v->affectJoin(c, &aff2, AVG_DUR_NO, AVG_EFF_YES, FALSE))
        success = true;
}
return success;
```

### Bitvector Suppression (handler.cc:935-936)
```cpp
if (af->bitvector && isAffected(af->bitvector) && !affectedBySpell(af->type))
  af->bitvector = 0;
```
Prevents removing bitvectors provided by other sources (equipment, racial abilities) when a spell expires.

### Polymorph Requires desc->original (periodic.cc:768-773)
`SPELL_POLYMORPH`, `SKILL_DISGUISE`, and `SPELL_SHAPESHIFT` are immediately removed if `desc` or `desc->original` becomes null.

### Affect Tick Processing (periodic.cc:735)
`updateAffects()` runs each tick: decrements duration, notifies on renewal eligibility, triggers wear-off at duration 0.

## Common Patterns

### Simple buff
```cpp
affectedData aff;
aff.type = SPELL_BLESS;
aff.level = caster->GetMaxLevel();
aff.duration = 24 * Pulse::UPDATES_PER_MUDHOUR;
aff.modifier = 5;
aff.location = APPLY_HITROLL;
aff.bitvector = 0;
victim->affectJoin(caster, &aff, AVG_DUR_NO, AVG_EFF_YES);
```

### Immunity affect
```cpp
affectedData aff;
aff.type = SPELL_PROTECTION_FROM_FIRE;
aff.location = APPLY_IMMUNITY;
aff.modifier = IMMUNE_HEAT;   // Immunity type goes in modifier
aff.modifier2 = 50;           // Immunity amount goes in modifier2!
```

### Permanent affect
```cpp
aff.duration = PERMANENT_DURATION;
victim->affectTo(&aff, -1);  // -1 renew = never renewable
```
