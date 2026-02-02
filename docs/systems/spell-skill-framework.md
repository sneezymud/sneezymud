---
title: Spell and Skill Framework
description: Unified ability system - data structures, discipline hierarchy, learnedness progression, practice mechanics, cost calculations
created_by_model: opus
---

# Spell and Skill Framework

## Overview

All character abilities - spells, skills, and prayers - exist within a single unified system. Abilities belong to hierarchical disciplines that determine learning requirements, effectiveness caps, and progression rates. Three independent values track mastery: natural potential sets the ceiling, trained learnedness controls power, and do-learnedness rewards active use. Each casting class draws from a different resource pool (mana, piety, or lifeforce) with class-specific cost scaling.

## Patterns

### Discipline Organization

- Always use the primary discipline (`disc`) to determine which skill tree teaches the ability
- Always use the associated discipline (`assDisc`) for damage scaling and do-learning gains
- Never assume discipline membership from spell type - check `discArray[spell]->disc`
- Always verify discipline learnedness meets the skill's `start` threshold before granting access

### Learnedness Management

- Always respect the cap hierarchy: do-learnedness and trained cannot exceed natural
- Never allow skill values above `getMaxSkillValue()` (discipline-capped)
- Always check all three values when diagnosing "why is my skill not improving" issues
- Never confuse discipline learnedness with skill value - they are related but distinct

### Practice System

- Always verify trainer level matches the target discipline percentage (60 vs 100)
- Never cross class boundaries with practice points - each class has separate pools
- Always check combat prerequisite before training advanced disciplines
- Always check weapon proficiency (92%) before allowing specialization training

### Do-Learning

- Always respect cooldowns: 30 seconds at 50% or below, 3 minutes above
- Never expect do-learning without first training the skill via practice
- Always check `startLearnDo != -1` before attempting do-learning logic
- Never allow do-learning in arena or for monsters

### Cost Systems

- Always divide piety enum values by 4 to get actual cost
- Never mix cost types - each class uses exactly one resource
- Always use the correct enum type for the class (`manaCostT`, `pietyCostT`, `lifeforceCostT`)

### Spell Returns

- Always check `reconcileDamage() == -1` for death, never use `IS_SET_DELETE()`
- Always return `SPELL_SUCCESS + VICTIM_DEAD` for single-target spells that kill
- Always delete victims directly in area spells and null the pointer
- Never continue execution after detecting death flags
- Always use `|` for combining bit flags, never `+`

## Reference

### Discipline Types

| Type | Method | Behavior |
|------|--------|----------|
| Basic | `isBasic()` | Primary class disciplines, 1%/prac |
| Fast | `isFast()` | Weapon specs, 1%/prac |
| Automatic | `isAutomatic()` | No practice points needed |
| Other | default | Quadratic formula, 60 pracs to max |

### Class Disciplines

| Class | Base | Advanced |
|-------|------|----------|
| Mage | DISC_MAGE | AIR, FIRE, WATER, EARTH, SPIRIT, SORCERY, ALCHEMY |
| Cleric | DISC_CLERIC | AEGIS, WRATH, AFFLICTIONS, CURES, HAND_OF_GOD |
| Warrior | DISC_WARRIOR | DUELING, BRAWLING, SOLDIERING, BLACKSMITHING |
| Thief | DISC_THIEF | STEALTH, MURDER, LOOTING, POISONS, TRAPS |
| Monk | DISC_MONK | MEDITATION, LEVERAGE, MINDBODY, FOCUSED_ATTACKS |
| Deikhan | DISC_DEIKHAN | MOUNTED, MARTIAL, GUARDIAN, ABSOLUTION, VENGEANCE |
| Ranger | DISC_RANGER | ANIMAL, PLANTS, NATURE |
| Shaman | DISC_SHAMAN | FROG, SPIDER, SKUNK, ARMADILLO, CONTROL, HEALING |

### Universal Disciplines

| Discipline | Classes | Notes |
|------------|---------|-------|
| DISC_ADVENTURING | All | Automatic |
| DISC_COMBAT | All | Prerequisite for advanced |
| DISC_SLASH/BLUNT/PIERCE/RANGED | All | Weapon specializations |

### Skill Value Types

| Value | Access | Description |
|-------|--------|-------------|
| Natural | `getNatSkillValue()` | Practice + do-learning |
| Actual | `getSkillValue()` | Natural + equipment |
| Max | `getMaxSkillValue()` | Discipline cap |

### Cost Resources

| Class | Field | Enum | Division |
|-------|-------|------|----------|
| Mage | `minMana` | `manaCostT` | None |
| Shaman | `minLifeforce` | `lifeforceCostT` | None |
| Cleric | `minPiety` | `pietyCostT` | By 4 |

### Piety Enum Examples

| Enum | Value | Actual Cost |
|------|-------|-------------|
| PRAY_025 | 5 | 1.25 |
| PRAY_100 | 20 | 5.0 |
| PRAY_200 | 40 | 10.0 |

### Target Flags

| Flag | Effect |
|------|--------|
| TAR_CHAR_ROOM | Target character in room |
| TAR_FIGHT_SELF | Default to self in combat |
| TAR_FIGHT_VICT | Default to opponent in combat |
| TAR_SELF_ONLY | Can only target self |
| TAR_SELF_NONO | Cannot target self |
| TAR_OBJ_INV | Target object in inventory |
| TAR_VIOLENT | Initiates combat |
| TAR_AREA | Area effect |
| TAR_NAME | Target by name string |

### Component Flags

| Flag | Effect |
|------|--------|
| COMP_GESTURAL | Requires hand gestures |
| COMP_VERBAL | Requires verbal component |
| COMP_MATERIAL | Requires material component |
| COMP_MATERIAL_END | Consume material at end |
| SPELL_TASKED | Uses task system (channeled) |
| SPELL_TASKED_EVERY | Task pulses every round |

### Spell Return Flags

| Constant | Meaning |
|----------|---------|
| SPELL_SUCCESS | Spell succeeded |
| SPELL_FAIL | Spell failed |
| SPELL_CRIT_FAIL | Critical failure |
| CASTER_DEAD | Caster died (backfire) |
| VICTIM_DEAD | Victim died |

### Do-Learning Parameters

| Parameter | Purpose |
|-----------|---------|
| `startLearnDo` | Initial value on learn (-1 = disabled) |
| `amtLearnDo` | Increase per success |
| `learnDoDiff` | Difficulty modifier |
| `secStartLearnDo` | Secondary skill start |
| `secAmtLearnDo` | Secondary increase |

### Automatic Discipline Gains

| Discipline | Class | Amount | When |
|------------|-------|--------|------|
| DISC_WIZARDRY | Mage | 2-3 | Level up |
| DISC_RITUALISM | Shaman | 2-3 | Level up |
| DISC_FAITH | Cleric/Deikhan | 2-3 | Level up |
| DISC_ADVENTURING | All | 1-2 | Level up |

### Key Constants

| Constant | Value | Purpose |
|----------|-------|---------|
| MAX_SKILL_LEARNEDNESS | 100 | Skill cap |
| MAX_DISC_LEARNEDNESS | 100 | Discipline cap |
| PRACS_TO_MAX | 60 | Full training cost |
| SKILL_MIN | -99 | Not learned |

## Implementation

### Data Architecture

The `discArray` global holds all spell/skill definitions, indexed by `spellNumT`. Negative indices are damage types, zero through `MAX_SPELL` are spells, `SKILL_SLAM` through `MAX_SKILL` are skills. Each entry is a `spellInfo` instance containing name, class type, disciplines, costs, targets, components, and learning parameters.

Characters store discipline instances in `CDiscipline* disc[MAX_DISCS]`. Each concrete discipline class inherits from `CDiscipline` and contains `CSkill` members for its abilities. The three learnedness values (natural, trained, do) are private members with accessor methods.

### Skill Value Calculation

Maximum skill value derives from discipline learnedness: `(disc_learnedness - skill_start + 1) * skill_learn_rate`, capped at `MAX_SKILL_LEARNEDNESS`. This creates the discipline-gating effect where raising discipline learnedness raises all skill caps within it.

### Practice Point Economics

Basic and fast disciplines increase linearly (1%/prac). Other disciplines use a quadratic formula: initial gains are large (~2.33%), final gains are small (~1%), totaling exactly 60 pracs from 0% to 100%. The formula: `increase = A*pracs + C` where `C = 200/PRACS_TO_MAX - 1` and `A = 2/PRACS_TO_MAX - 200/PRACS_TO_MAX^2`.

### Do-Learning Mechanics

On skill use, `learnFromDoing()` checks eligibility (player, not arena, skill supports it), enforces cooldowns, then rolls for discipline and skill gains. Discipline gain chances: 1/200 for combat, 1/150 for advanced, 1/200 main + 1/400 associated for others. Skill gain chance scales with headroom: `1000 * pow((max - actual)/max, 3 - plotStat(WIS, 1.0, 2.5))`, minimum 15 if any headroom exists.

### Prerequisite System

Combat prerequisite: `(3.5 * level / 10) - 4` minimum percentage. Weapon specialization requires 92% in corresponding proficiency. Advanced disciplines require base discipline completion. Quest-locked skills set eligibility toggles even after discipline requirements are met.

### Spell Execution Flow

Spells return bit-flag combinations. Single-target damage spells check `reconcileDamage() == -1` and return `SPELL_SUCCESS + VICTIM_DEAD`. Area spells iterate with pre-increment (`*(it++)`), delete victims directly, null pointers, and return without death flags. Backfire damage returns `SPELL_CRIT_FAIL + CASTER_DEAD`.

### Meta-Discipline Bonuses

DISC_WIZARDRY, DISC_FAITH, and DISC_RITUALISM boost effective spell level: `level += 2 + (disc_learnedness / 34)`.

### Key Source Files

| File | Contents |
|------|----------|
| `code/code/misc/spell_info.cc` | `discArray[]`, `buildSpellArray()` |
| `code/code/misc/spell2.h` | `spellInfo` struct, all flags |
| `code/code/misc/spells.h` | `spellNumT` enum |
| `code/code/disc/disc_*.cc` | Spell implementations |
| `code/code/misc/discipline.h` | `CDiscipline`, `discNumT` |
| `code/code/misc/discipline.cc` | `getMaxSkillValue()`, `learnFromDoing()` |
| `code/code/misc/gaining.cc` | Trainers, practice, discipline raising |
| `code/code/misc/other.cc` | `doPractice()` command |
| `code/code/misc/limits.cc` | Practice point tracking |
| `code/code/misc/skills.cc` | `getSkillValue()` |

## Troubleshooting

### Skill not improving from practice

**Symptom:** Spending practice points but skill value unchanged.

**Cause:** Discipline learnedness already at natural cap, or skill already at discipline cap.

**Fix:** Check `getNatLearnedness()` vs `getLearnedness()` for discipline, `getMaxSkillValue()` vs current for skill. Train at higher-level trainer or level up for more natural potential.

### Skill not improving from use

**Symptom:** Using skill repeatedly but no do-learning messages.

**Cause:** Cooldown not elapsed, skill at max, in arena, or `startLearnDo == -1`.

**Fix:** Wait 30s/3min between attempts. Check `getMaxSkillValue()` against current. Verify skill has do-learning enabled in `discArray`.

### Cannot train at trainer

**Symptom:** Trainer refuses to teach or discipline stops increasing.

**Cause:** Trainer level limit (60% cap), class mismatch, or prerequisite not met.

**Fix:** Find level 100 trainer. Verify class can learn discipline. Check combat prerequisite or proficiency requirement.

### Piety cost seems wrong

**Symptom:** Spell costs different from enum value suggests.

**Cause:** Piety enums are 4x actual cost.

**Fix:** Divide enum value by 4 to get true cost.

### Spell death not detected

**Symptom:** Victim dead but code continues executing on them.

**Cause:** Using `IS_SET_DELETE()` instead of checking `== -1`.

**Fix:** `reconcileDamage()` returns -1 on death, not a DELETE flag. Check return value directly.

### Area spell crashes after kill

**Symptom:** Use-after-free in area spell loop.

**Cause:** Not advancing iterator before deletion, or not nulling pointer after delete.

**Fix:** Use `*(it++)` pattern. Delete victim, then set `vict = nullptr` before continuing loop.

### Skill appears but cannot use

**Symptom:** Skill shows in practice list but fails on use.

**Cause:** Quest toggle not set despite meeting discipline requirement.

**Fix:** Check if skill has a toggle requirement. Complete the associated quest.

### Wrong practice pool used

**Symptom:** Multiclass character cannot train expected discipline.

**Cause:** Practice points are class-specific, not shared.

**Fix:** Use practice points from the class that owns the discipline.
