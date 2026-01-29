---
title: Offensive Spell System
description: The offensive spell system provides damage-dealing magical abilities across multiple classes and disciplines with centralized damage calculation, three-layer success mechanics, and comprehensive immunity handling.
keywords:
  - offensive spells
  - spell damage
  - genericDam
  - bSuccess
  - critSuccess
  - isLucky
  - reconcileDamage
  - immunity system
  - spell casting
  - area effect
  - VICTIM_DEAD
  - CASTER_DEAD
  - spellLuckModifier
  - TMagicItem
category: Critical Systems

  - damage-pipeline.md
  - combat-formulas.md
  - delete-flags.md
  - affects-system.md
  - stats-attributes.md
  - combat-skills.md
last_updated: 2026-01-29
source_files:
  - code/code/misc/skill_dam.cc
  - code/code/misc/spell_info.cc
  - code/code/disc/disc_mage_fire.cc
  - code/code/disc/disc_mage_air.cc
  - code/code/disc/disc_mage_earth.cc
  - code/code/disc/disc_mage_water.cc
  - code/code/disc/disc_mage_spirit.cc
  - code/code/disc/disc_cleric_wrath.cc
  - code/code/disc/disc_cleric_afflictions.cc
  - code/code/disc/disc_shaman_frog.cc
  - code/code/disc/disc_shaman_spider.cc
  - code/code/disc/disc_shaman_control.cc
  - code/code/disc/disc_deikhan_martial.cc
  - code/code/misc/crit_combat.cc
  - code/code/misc/being.cc
  - code/code/misc/immunity.h
  - code/code/misc/damage.cc
  - code/code/misc/spell2.h
related: [spell-skill-framework.md]
---

# Offensive Spell System

The offensive spell system in SneezyMUD provides damage-dealing magical abilities across multiple classes and disciplines. Understanding this system is critical for implementing new offensive spells, balancing damage output, and avoiding memory safety bugs when victims die.

**Misusing this system causes crashes.** Common errors: checking reconcileDamage() death with IS_SET_DELETE instead of `== -1`, not translating spell return flags to DELETE flags in magic item versions, continuing after victim death in area spells.

## Overview

 SneezyMUD's offensive spell system features:
- **50+ offensive spells** across 8 classes organized by 20+ disciplines
- **Centralized damage calculation** in `genericDam()` for consistent balancing
- **Three-layer success mechanics**: spell success → critical success → save check
- **28-type immunity system** for fine-grained damage resistance
- **Critical hit mechanics** with 3x damage multipliers and 20+ failure outcomes
- **Area effect support** with proper victim cleanup and group filtering
- **DELETE flag safety patterns** ensuring proper memory management

**Source files:**
- `code/code/misc/skill_dam.cc` - Core damage calculations
- `code/code/misc/spell_info.cc` - Spell definitions database
- `code/code/disc/disc_*.cc` - Individual spell implementations
- `code/code/misc/crit_combat.cc` - Critical hit/failure mechanics

## Spell Return Values

Spell functions return a combination of bit flags indicating success/failure and death states:

| Constant | Bit | Meaning |
|----------|-----|---------|
| `SPELL_SUCCESS` | 1 << 1 | Spell succeeded |
| `SPELL_FAIL` | 1 << 2 | Spell failed |
| `SPELL_CRIT_FAIL` | 1 << 3 | Critical failure |
| `CASTER_DEAD` | 1 << 6 | Caster died (backfire) |
| `VICTIM_DEAD` | 1 << 7 | Victim died |

Combine with `+` or `|`: `return SPELL_SUCCESS + VICTIM_DEAD;`

## Critical: Damage Handling in Spells

### The `-1` Return Value

**`reconcileDamage()` returns `-1` when the victim dies, NOT a DELETE flag.**

```cpp
// CORRECT: Check for -1, return spell death flag
if (caster->reconcileDamage(victim, dam, SPELL_FIREBALL) == -1)
    return SPELL_SUCCESS + VICTIM_DEAD;

// WRONG: IS_SET_DELETE won't detect -1
if (IS_SET_DELETE(rc, DELETE_VICT)) { ... }  // Never triggers!
```

See [Damage Pipeline](damage-pipeline.md) for complete documentation.

### Area Spells: Delete Victims Directly

Area-effect spells iterate over room contents. When a victim dies, delete them immediately and null the pointer to prevent use-after-free:

```cpp
for (auto it = caster->roomp->stuff.begin(); it != caster->roomp->stuff.end();) {
    t = *(it++);  // Advance BEFORE potential deletion
    vict = dynamic_cast<TBeing*>(t);
    if (!vict) continue;

    if (caster->reconcileDamage(vict, dam, SPELL_PEBBLE_SPRAY) == -1) {
        delete vict;
        vict = nullptr;  // Prevent use-after-free
    }
}
return SPELL_SUCCESS;  // No VICTIM_DEAD - victims already handled
```

### Single-Target Spells: Return Death Flag

Single-target spells should NOT delete directly. Return the death flag and let the caller handle deletion:

```cpp
if (caster->reconcileDamage(victim, dam, SPELL_FIREBALL) == -1)
    return SPELL_SUCCESS + VICTIM_DEAD;  // Caller deletes
return SPELL_SUCCESS;
```

### Caster Self-Damage (Backfire)

When critical failures damage the caster:

```cpp
if (caster->reconcileDamage(caster, dam, SPELL_FIREBALL) == -1)
    return SPELL_CRIT_FAIL + CASTER_DEAD;  // Caller deletes caster
return SPELL_CRIT_FAIL;
```

## Damage Calculation System

### The genericDam() Formula

**Source:** `code/code/misc/skill_dam.cc`

The core damage calculation applies consistent scaling across all offensive spells:

```cpp
int TBeing::genericDam(TBeing* caster, spellNumT spell, int level,
                       int adv_learn, int dam) const {
    // Base damage calculation
    fixed_amt = (class_amt * lagamt * level);

    // Apply task difficulty modifier (35-110%)
    fixed_amt *= getSkillDiffModifier(spell);

    // Apply stat modifier (0.8-1.25x)
    fixed_amt *= plotStat(STAT_CURRENT, modifierStat, 0.8, 1.25, 1.0);

    // Add random variance (±level/4 to ±level/2)
    fixed_amt += number(-level/4, level/2);

    // Area effect penalty (75% damage)
    if (isAreaSpell(spell))
        fixed_amt *= 0.75;

    // NPC damage reduction (52% of PC damage)
    if (caster->isNPC())
        fixed_amt *= 0.5195;

    // PvP damage reduction (50% when targeting players)
    if (isPc())
        fixed_amt *= 0.5;

    return fixed_amt;
}
```

### Formula Components

#### 1. Base Damage

```
base_damage = classAmt × lag_rounds × caster_level
```

**classAmt** is spell-specific damage multiplier (0.5-4.0 range)
**lag_rounds** is from `lag_t` enum (0-9, representing 0-10.8 seconds)
**caster_level** is `getSkillLevel(spell)` for the caster

## Success Mechanics

### Three-Layer Success System

Offensive spells undergo three separate success checks:

```
Layer 1: Spell Success (bSuccess)
   ↓ Success
Layer 2: Critical Success (critSuccess)
   ↓ Damage Multiplier Applied
Layer 3: Save Check (isLucky)
   ↓ Damage Reduction on Save
Final Damage Application
```

### Layer 1: Spell Success (bSuccess)

**Source:** `code/code/misc/being.cc`

```cpp
bool TBeing::bSuccess(int bKnown, spellNumT spell) {
    // Calculate success limit
    limit = getSkillDiffModifier(spell);     // Task difficulty
    limit *= bKnown / 100.0;                 // Skill learning (0-100%)
    limit *= getStatMod(STAT_FOC);           // Focus stat (0.8-1.25x)
    limit *= plotStat(STAT_CURRENT, STAT_KAR, 0.9, 1.125, 1.0);  // Karma

    // Roll against limit
    int iLimit = (int)(limit * 100);
    return (::number(0, 99) < iLimit);
}
```

### Layer 2: Critical Success (critSuccess)

**Source:** `code/code/misc/crit_combat.cc`

After spell success, critical success check determines damage multiplier:

```cpp
enum critSuccessTypeT {
    CRIT_S_NONE,    // 1x damage (normal)
    CRIT_S_DOUBLE,  // 2x damage
    CRIT_S_TRIPLE,  // 3x damage
    CRIT_S_KILL     // 3x damage + special message
};
```

### Layer 3: Save Check (isLucky)

**Source:** `code/code/misc/being.cc`

After damage calculation, victim may "save" for reduced effect:

```cpp
if (victim->isLucky(caster->spellLuckModifier(SPELL_NAME))) {
    SV(SPELL_NAME);  // Display save message
    dam /= 2;        // Reduce damage by 50%
}
```

## Implementation Patterns

### Three-Function Spell Pattern

Every offensive spell implements three overloaded versions for different casting contexts:

#### Function 1: Core Implementation

```cpp
int lavaLance(TBeing* caster, TBeing* victim, int level,
              short bKnown, int adv_learn)
{
    int dam;

    // Calculate base damage
    dam = caster->getSkillDam(victim, SPELL_LAVA_LANCE, level, adv_learn);

    // Success check
    if (caster->bSuccess(bKnown, SPELL_LAVA_LANCE)) {
        // Critical success handling
        switch (critSuccess(caster, SPELL_LAVA_LANCE)) {
            case CRIT_S_KILL:
                CS(SPELL_LAVA_LANCE);
                dam *= 3;
                break;
            case CRIT_S_TRIPLE:
                CS(SPELL_LAVA_LANCE);
                dam *= 3;
                break;
            case CRIT_S_DOUBLE:
                CS(SPELL_LAVA_LANCE);
                dam *= 2;
                break;
            case CRIT_S_NONE:
                break;
        }

        // Save check for damage reduction
        if (victim->isLucky(caster->spellLuckModifier(SPELL_LAVA_LANCE))) {
            SV(SPELL_LAVA_LANCE);
            dam /= 2;
        }

        // Display spell-specific messages
        act("A lance of molten rock shoots from $n's hands!",
            FALSE, caster, NULL, victim, TO_NOTVICT);
        act("A lance of molten rock shoots from your hands!",
            FALSE, caster, NULL, victim, TO_CHAR);
        act("$n blasts you with a lance of molten rock!",
            FALSE, caster, NULL, victim, TO_VICT);

        // Apply damage
        if (caster->reconcileDamage(victim, dam, SPELL_LAVA_LANCE) == -1)
            return SPELL_SUCCESS + VICTIM_DEAD;

        return SPELL_SUCCESS;
    } else {
        // Failure branch
        switch (critFail(caster, SPELL_LAVA_LANCE)) {
            case CRIT_F_HITSELF:
                CF(SPELL_LAVA_LANCE);
                act("$n's lava lance BACKFIRES!",
                    TRUE, caster, NULL, NULL, TO_ROOM);

                if (caster->reconcileDamage(caster, dam/2, SPELL_LAVA_LANCE) == -1)
                    return SPELL_CRIT_FAIL + CASTER_DEAD;

                return SPELL_CRIT_FAIL;
            case CRIT_F_HITOTHER:
                // Hit random person in room
                break;
            case CRIT_F_NONE:
                break;
        }

        // Regular failure
        caster->nothingHappens(SILENT_YES);
        return SPELL_FAIL;
    }
}
```

**Source:** `code/code/disc/disc_mage_fire.cc`

#### Function 2: Magic Item Version

```cpp
int lavaLance(TBeing* caster, TBeing* victim, TMagicItem* obj)
{
    int rc = 0;

    // Call core implementation with magic item stats
    int ret = lavaLance(caster, victim,
                        obj->getMagicLevel(),
                        obj->getMagicLearnedness(),
                        0);  // No advanced learning from items

    // Translate spell return flags to DELETE flags
    if (IS_SET(ret, VICTIM_DEAD))
        ADD_DELETE(rc, DELETE_VICT);
    if (IS_SET(ret, CASTER_DEAD))
        ADD_DELETE(rc, DELETE_THIS);

    return rc;
}
```

**Purpose:** Handles casting from wands/staves/scrolls
**Key difference:** Translates `SPELL_*` return values to `DELETE_*` flags

#### Function 3: Player Casting Version

```cpp
int lavaLance(TBeing* caster, TBeing* victim)
{
    int rc = 0;

    // Validate caster can use this spell
    if (!bPassClassChecks(caster, SPELL_LAVA_LANCE)) {
        caster->sendTo("You don't know how to cast that!\n\r");
        return FALSE;
    }

    // Get caster's spell statistics
    int level = caster->getSkillLevel(SPELL_LAVA_LANCE);
    int bKnown = caster->getSkillValue(SPELL_LAVA_LANCE);
    int adv_learn = caster->getAdvLearning(SPELL_LAVA_LANCE);

    // Call core implementation
    int ret = lavaLance(caster, victim, level, bKnown, adv_learn);

    // Propagate caster death flag
    if (IS_SET(ret, CASTER_DEAD))
        ADD_DELETE(rc, DELETE_THIS);

    // Note: victim death NOT propagated - already handled in core

    return rc;
}
```

**Purpose:** Entry point for player-initiated casting
**Key difference:** Retrieves player stats, validates class access

### Area Effect Pattern

Area spells iterate through room contents with proper death handling:

```cpp
int pebbleSpray(TBeing* caster, int level, short bKnown, int adv_learn)
{
    int dam = caster->getSkillDam(NULL, SPELL_PEBBLE_SPRAY, level, adv_learn);
    int rc;
    TThing* t;
    TBeing* vict;

    // Display area effect messages
    act("$n sprays the area with sharp pebbles!",
        FALSE, caster, NULL, NULL, TO_ROOM);
    act("You spray the area with sharp pebbles!",
        FALSE, caster, NULL, NULL, TO_CHAR);

    // Iterate through room creatures
    for (StuffIter it = caster->roomp->stuff.begin();
         it != caster->roomp->stuff.end();) {
        t = *(it++);  // CRITICAL: Advance iterator FIRST
        vict = dynamic_cast<TBeing*>(t);

        // Skip invalid targets
        if (!vict) continue;
        if (caster == vict) continue;              // Skip self
        if (caster->inGroup(*vict)) continue;      // Skip group members
        if (vict->isImmortal()) continue;          // Skip immortals
        if (!caster->canSee(vict)) continue;       // Skip invisible

        // Apply area effect damage penalty (75%)
        int areaDam = dam * 3 / 4;

        // Apply damage
        if (caster->reconcileDamage(vict, areaDam, SPELL_PEBBLE_SPRAY) == -1) {
            delete vict;          // DELETE victim immediately
            vict = NULL;          // Clear pointer to prevent use-after-free
            // Iterator already advanced, safe to continue
        }
    }

    return SPELL_SUCCESS;  // No VICTIM_DEAD - handled locally
}
```

**Source:** `code/code/disc/disc_mage_earth.cc`

**Critical points:**
1. **Iterator advancement:** `*(it++)` advances BEFORE accessing element
2. **Group filtering:** `inGroup()` prevents friendly fire
3. **Immediate deletion:** `delete vict` called directly, not via flag
4. **No VICTIM_DEAD flag:** Return value doesn't indicate deaths

### Immunity Check Pattern

Check immunity before damage application:

```cpp
int waterSpray(TBeing* caster, TBeing* victim, int level,
               short bKnown, int adv_learn)
{
    // Check immunity FIRST
    if (victim->getImmunity(IMMUNE_WATER) >= 100) {
        act("$N is completely immune to water damage!",
            FALSE, caster, NULL, victim, TO_CHAR);
        act("You are immune to $n's water spray!",
            FALSE, caster, NULL, victim, TO_VICT);
        caster->nothingHappens(SILENT_YES);
        return SPELL_FAIL;  // Spell fails completely
    }

    // Calculate base damage
    int dam = caster->getSkillDam(victim, SPELL_WATER_SPRAY, level, adv_learn);

    // Apply partial immunity (0-99%)
    int immunePercent = victim->getImmunity(IMMUNE_WATER);
    dam = dam * (100 - immunePercent) / 100;

    // Success check
    if (caster->bSuccess(bKnown, SPELL_WATER_SPRAY)) {
        // ... critical success, save check, etc.

        if (caster->reconcileDamage(victim, dam, SPELL_WATER_SPRAY) == -1)
            return SPELL_SUCCESS + VICTIM_DEAD;

        return SPELL_SUCCESS;
    } else {
        caster->nothingHappens(SILENT_YES);
        return SPELL_FAIL;
    }
}
```

**Immunity levels:**
- **0-49%**: Minor resistance, reduced damage
- **50-74%**: Moderate resistance, significant reduction
- **75-99%**: Strong resistance, minimal damage
- **100%+**: Complete immunity, spell fails

## DELETE Flag Handling

### Death Detection: The -1 Return Value

**CRITICAL:** `reconcileDamage()` returns `-1` on victim death, NOT `DELETE_VICT`.

```cpp
// CORRECT: Check for -1
if (caster->reconcileDamage(victim, dam, SPELL_NAME) == -1)
    return SPELL_SUCCESS + VICTIM_DEAD;

// WRONG: IS_SET_DELETE won't detect -1
int rc = caster->reconcileDamage(victim, dam, SPELL_NAME);
if (IS_SET_DELETE(rc, DELETE_VICT)) {  // NEVER triggers!
    return SPELL_SUCCESS + VICTIM_DEAD;
}
```

**Source:** `code/code/misc/damage.cc`

The `-1` value is chosen because damage amounts are always non-negative, making `-1` an unambiguous death signal.

### Magic Item Version: Flag Translation

The magic item version must translate spell return flags to DELETE flags:

```cpp
int lavaLance(TBeing* caster, TBeing* victim, TMagicItem* obj) {
    int rc = 0;  // Will hold DELETE flags

    // Call core implementation
    int ret = lavaLance(caster, victim,
                        obj->getMagicLevel(),
                        obj->getMagicLearnedness(),
                        0);

    // Translate SPELL_* flags to DELETE_* flags
    if (IS_SET(ret, VICTIM_DEAD)) {
        ADD_DELETE(rc, DELETE_VICT);  // Caller should delete victim
    }
    if (IS_SET(ret, CASTER_DEAD)) {
        ADD_DELETE(rc, DELETE_THIS);  // Caller should delete caster
    }

    return rc;  // Return DELETE flags for caller
}
```

**Workflow:**
1. Core implementation returns `SPELL_SUCCESS + VICTIM_DEAD`
2. Magic item version checks `IS_SET(ret, VICTIM_DEAD)`
3. Sets `ADD_DELETE(rc, DELETE_VICT)`
4. Returns `rc` with DELETE_VICT flag
5. Caller checks `IS_SET_DELETE(rc, DELETE_VICT)` and deletes victim

## Resistance and Save Mechanics

### Immunity System

**Source:** `code/code/misc/immunity.h`

28 immunity types provide fine-grained damage resistance. Key types include:

- `IMMUNE_HEAT` - Fire damage
- `IMMUNE_COLD` - Ice damage
- `IMMUNE_ACID` - Acid damage
- `IMMUNE_POISON` - Poison damage
- `IMMUNE_AIR` - Wind/air damage
- `IMMUNE_ENERGY` - Pure energy damage
- `IMMUNE_ELECTRICITY` - Lightning damage
- `IMMUNE_WATER` - Water damage
- `IMMUNE_EARTH` - Earth damage
- `IMMUNE_HOLY` - Holy damage

### Immunity Application

#### Complete Immunity (100%+)

```cpp
if (victim->getImmunity(IMMUNE_HEAT) >= 100) {
    act("$N is completely immune to fire!",
        FALSE, caster, NULL, victim, TO_CHAR);
    return SPELL_FAIL;  // Spell fails entirely
}
```

#### Partial Immunity (0-99%)

```cpp
int immunePercent = victim->getImmunity(IMMUNE_HEAT);
dam = dam * (100 - immunePercent) / 100;

// Examples:
// 0% immunity:  dam × (100 - 0) / 100 = dam × 1.0 (full damage)
// 50% immunity: dam × (100 - 50) / 100 = dam × 0.5 (half damage)
// 75% immunity: dam × (100 - 75) / 100 = dam × 0.25 (quarter damage)
```

## Common Pitfalls

### 1. Wrong Death Check

```cpp
// CRASH: Using IS_SET_DELETE for reconcileDamage return
int rc = caster->reconcileDamage(victim, dam, SPELL_NAME);
if (IS_SET_DELETE(rc, DELETE_VICT)) {  // NEVER triggers for -1!
    return SPELL_SUCCESS + VICTIM_DEAD;
}

// CORRECT: Check for -1
if (caster->reconcileDamage(victim, dam, SPELL_NAME) == -1)
    return SPELL_SUCCESS + VICTIM_DEAD;
```

### 2. Missing Flag Translation

```cpp
// BUG: Magic item version not translating flags
int spellName(TBeing* caster, TBeing* victim, TMagicItem* obj) {
    return spellName(caster, victim, obj->getMagicLevel(), ...);
    // Should translate VICTIM_DEAD → DELETE_VICT!
}

// CORRECT: Translate spell flags to DELETE flags
int spellName(TBeing* caster, TBeing* victim, TMagicItem* obj) {
    int rc = 0;
    int ret = spellName(caster, victim, obj->getMagicLevel(), ...);

    if (IS_SET(ret, VICTIM_DEAD))
        ADD_DELETE(rc, DELETE_VICT);
    if (IS_SET(ret, CASTER_DEAD))
        ADD_DELETE(rc, DELETE_THIS);

    return rc;
}
```

### 3. Continuing After Death

```cpp
// CRASH: Using victim after death
if (caster->reconcileDamage(victim, dam, SPELL_NAME) == -1)
    return SPELL_SUCCESS + VICTIM_DEAD;

victim->sendTo("You feel weak!\n\r");  // CRASH: victim is deleted!

// CORRECT: Return immediately after death
if (caster->reconcileDamage(victim, dam, SPELL_NAME) == -1)
    return SPELL_SUCCESS + VICTIM_DEAD;
return SPELL_SUCCESS;
```

### 4. Area Effect Iterator Misuse

```cpp
// CRASH: Not advancing iterator before removal
for (StuffIter it = room->stuff.begin(); it != room->stuff.end(); ++it) {
    TBeing* vict = dynamic_cast<TBeing*>(*it);
    if (reconcileDamage(vict, dam, SPELL_NAME) == -1) {
        delete vict;  // Invalidates iterator!
    }
    // ++it now crashes
}

// CORRECT: Advance FIRST
for (StuffIter it = room->stuff.begin(); it != room->stuff.end();) {
    TThing* t = *(it++);  // Advance BEFORE any deletion
    TBeing* vict = dynamic_cast<TBeing*>(t);

    if (reconcileDamage(vict, dam, SPELL_NAME) == -1) {
        delete vict;  // Safe: iterator already advanced
        vict = nullptr;
    }
}
```

### 5. Forgetting Immunity Check

```cpp
// BUG: No immunity check, spell always deals damage
int dam = getSkillDam(...);
if (reconcileDamage(victim, dam, SPELL_FIRE) == -1)
    return SPELL_SUCCESS + VICTIM_DEAD;

// CORRECT: Check immunity first
if (victim->getImmunity(IMMUNE_HEAT) >= 100) {
    act("$N is immune!", FALSE, caster, NULL, victim, TO_CHAR);
    return SPELL_FAIL;
}

int dam = getSkillDam(...);
int immunePercent = victim->getImmunity(IMMUNE_HEAT);
dam = dam * (100 - immunePercent) / 100;  // Apply partial immunity

if (reconcileDamage(victim, dam, SPELL_FIRE) == -1)
    return SPELL_SUCCESS + VICTIM_DEAD;
```

## Related Documentation

- [Spell Definitions](spell-definitions.md) - spellInfo structure, enums, flags
- [Damage Pipeline](damage-pipeline.md) - reconcileDamage() and the -1 death return
- [Combat Formulas](combat-formulas.md) - Hit/damage calculations, stat modifiers
- [DELETE Flag System](delete-flags.md) - Memory management signaling
- [Affects System](affects-system.md) - affectedData, affectJoin() return values
- [Stats and Attributes](stats-attributes.md) - plotStat() formula, stat modifiers
- [Combat Skills](skill-combat.md) - Physical skill damage system
