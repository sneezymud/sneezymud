---
title: Spell and Skill Framework
description: Unified ability system - data structures, discipline hierarchy, learnedness progression, practice mechanics, cost calculations
category: important
keywords: [spell learning, automatic disciplines, quest-locked abilities, meta-discipline bonuses]
primary_symbols:
  functions: [getMaxSkillValue, learnFromDoing, reconcileDamage, getDisciplineNumber, buildSpellArray, getSkillValue, getNatSkillValue, pracsPerLevel]
  classes: [spellInfo, CDiscipline, CMasterDiscipline, CSkill]
  enums: [spellNumT, skillNumT, discNumT, manaCostT, pietyCostT, lifeforceCostT, TAR_CHAR_ROOM, TAR_FIGHT_SELF, TAR_FIGHT_VICT, TAR_SELF_ONLY, TAR_VIOLENT, TAR_AREA, COMP_GESTURAL, COMP_VERBAL, COMP_MATERIAL, SPELL_SUCCESS, SPELL_FAIL, SPELL_CRIT_FAIL, CASTER_DEAD, VICTIM_DEAD, MAX_SKILL_LEARNEDNESS, MAX_DISC_LEARNEDNESS, PRACS_TO_MAX, SKILL_MIN, MAX_SKILL, MIN_SPELL]
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
- Use `getPracs(classIndT)` to retrieve the correct class-specific pool

### Do-Learning

- Always respect cooldowns: 30 seconds at 50% or below, 3 minutes above
- Never expect do-learning without first training the skill via practice
- Always check `startLearnDo != -1` before attempting do-learning logic
- Never allow do-learning in arena or for monsters
- Use `learnFromDoingUnusual()` for weapon proficiencies (implements additional random delays)

### Cost Systems

- Always divide piety enum values by 4 to get actual cost
- Never mix cost types - each class uses exactly one resource
- Always use the correct enum type for the class (`manaCostT`, `pietyCostT`, `lifeforceCostT`)
- Resource deduction occurs after successful completion, not before spell execution

### Spell Returns

- Always check `reconcileDamage() == -1` for death, never use `IS_SET_DELETE()`
- Always return `SPELL_SUCCESS | VICTIM_DEAD` for single-target spells that kill
- Always delete victims directly in area spells and null the pointer
- Never continue execution after detecting death flags
- Always use `|` for combining bit flags, never `+`

```cpp
// CORRECT - single-target death handling
if (caster->reconcileDamage(victim, dam, SPELL_FIREBALL) == -1)
    return SPELL_SUCCESS | VICTIM_DEAD;

// WRONG - IS_SET_DELETE never triggers on reconcileDamage
if (IS_SET_DELETE(rc, DELETE_VICT)) { ... }
```

```cpp
// CORRECT - area spell iteration with death handling
for (auto it = caster->roomp->stuff.begin(); it != caster->roomp->stuff.end();) {
    t = *(it++);  // Advance iterator BEFORE potential deletion
    vict = dynamic_cast<TBeing*>(t);
    if (!vict) continue;

    if (caster->reconcileDamage(vict, dam, SPELL_PEBBLE_SPRAY) == -1) {
        delete vict;
        vict = nullptr;
    }
}
return SPELL_SUCCESS;  // No VICTIM_DEAD flag - deletion already handled
```

### Quest-Locked Skills

- Always check quest toggle bits before granting quest-locked skills
- Use `setSpellEligibleToggle()` when discipline requirements are met but quest is incomplete
- Toggle bits persist across sessions - quest need not be repeated if discipline learnedness is lost

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
| Shaman | DISC_SHAMAN | ARMADILLO, FROG, ALCHEMY, SKUNK, SPIDER, CONTROL, HEALING |

### Universal Disciplines

| Discipline | Classes | Notes |
|------------|---------|-------|
| DISC_ADVENTURING | All | Automatic |
| DISC_COMBAT | All | Prerequisite for advanced |
| DISC_SLASH/BLUNT/PIERCE/RANGED | All | Weapon specializations |

### Learnedness Types

| Type | Range | Description |
|------|-------|-------------|
| Natural | 0-100 | Innate potential from class/level, caps trained learnedness |
| Trained | 0-natural | Improved via practice points |
| Do-Learnedness | 0-100 | Gained through actual use, independent of practice |

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

| Constant | Bit | Meaning |
|----------|-----|---------|
| SPELL_SUCCESS | 1 << 1 | Spell succeeded |
| SPELL_FAIL | 1 << 2 | Spell failed |
| SPELL_CRIT_FAIL | 1 << 3 | Critical failure |
| CASTER_DEAD | 1 << 6 | Caster died (backfire) |
| VICTIM_DEAD | 1 << 7 | Victim died |

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
| DISC_ADVENTURING | All | 1-2 | Level up (reduced for multiclass) |

### Key Constants

| Constant | Value | Purpose |
|----------|-------|---------|
| MAX_SKILL_LEARNEDNESS | 100 | Skill cap |
| MAX_DISC_LEARNEDNESS | 100 | Discipline cap |
| PRACS_TO_MAX | 60 | Full training cost |
| SKILL_MIN | -99 | Not learned |
| MAX_SKILL | enum end | End of valid spell indices |
| MIN_SPELL | TYPE_UNDEFINED + 1 | First valid spell index |

### Primary Symbols

| Symbol | Type | Location | Purpose |
|--------|------|----------|---------|
| `discArray` | global array | spell_info.cc | All spell/skill definitions, indexed by spellNumT |
| `buildSpellArray()` | function | spell_info.cc | Populates discArray at startup |
| `spellInfo` | class | spell2.h | Single spell/skill definition |
| `spellNumT` | enum | spells.h | Enumerates all spells and skills |
| `CDiscipline` | class | discipline.h | Base class for discipline skill trees |
| `CMasterDiscipline` | class | discipline.h | Contains pointers to all 69 discipline instances |
| `discNumT` | enum | discipline.h | Enumerates 69 disciplines |
| `getMaxSkillValue()` | function | discipline.cc | Calculates skill cap from discipline |
| `learnFromDoing()` | function | discipline.cc | Passive skill improvement through use |
| `reconcileDamage()` | function | combat.cc | Applies damage, returns -1 on death |
| `getDisciplineNumber()` | function | discipline.cc | Returns primary discipline for a spell |
| `TrainerInfo` | array | gaining.cc | NPC trainers and specializations |

## Implementation

### Data Architecture

The `discArray` global holds all spell/skill definitions, indexed by `spellNumT`. Negative indices are damage types, zero through `MAX_SPELL` are spells, `SKILL_SLAM` through `MAX_SKILL` are skills. Each entry is a `spellInfo` instance containing name, class type, disciplines, costs, targets, components, and learning parameters.

Characters store discipline instances in `CDiscipline* disc[MAX_DISCS]`. Each concrete discipline class inherits from `CDiscipline` and contains `CSkill` members for its abilities. The three learnedness values (natural, trained, do) are private members with accessor methods. A `CMasterDiscipline` instance contains pointers to all 69 discipline instances, allowing efficient iteration and lookup.

### Discipline Hierarchy

Disciplines organize into a class-based hierarchy. Each class has one base discipline and multiple advanced disciplines. Universal disciplines cross class boundaries: `DISC_ADVENTURING` provides basic survival skills to all classes, `DISC_COMBAT` teaches fundamental combat techniques, and weapon proficiency/specialization disciplines govern melee effectiveness.

### Spell-to-Discipline Mapping

Every spell stores two discipline references. The primary discipline (`disc`) determines which skill tree teaches the spell. The associated discipline (`assDisc`) affects damage scaling and do-learning. The `getDisciplineNumber()` function retrieves the primary discipline by indexing `discArray`.

Two additional fields control learning: `start` specifies the discipline learnedness required before the spell becomes available, and `learn` provides a multiplier for calculating maximum skill value.

### Skill Value Calculation

Maximum skill value derives from discipline learnedness: `(disc_learnedness - skill_start + 1) * skill_learn_rate`, capped at `MAX_SKILL_LEARNEDNESS`. This creates the discipline-gating effect where raising discipline learnedness raises all skill caps within it.

### Practice Point Economics

Basic and fast disciplines increase linearly (1%/prac). Other disciplines use a quadratic formula: initial gains are large (~2.33%), final gains are small (~1%), totaling exactly 60 pracs from 0% to 100%. The formula: `increase = A*pracs + C` where `C = 200/PRACS_TO_MAX - 1` and `A = 2/PRACS_TO_MAX - 200/PRACS_TO_MAX^2`.

Practice points are allocated per-class via `getPracs(classIndT)`. Characters gain points on level-up according to `pracsPerLevel()`, which considers class, wisdom, and multiclass status.

The practice command supports multiple modes: `practice` lists all disciplines, `practice class <class>` filters to class-specific disciplines, `practice discipline <disc>` shows skills within that discipline, and `practice skill <skill>` provides detailed skill information.

### Trainer System

Trainers are NPCs defined in the `TrainerInfo` array in gaining.cc, mapping virtual mob numbers to disciplines and class restrictions. Trainers have level limits: level 60 trainers cap at 60% learnedness, level 100 trainers allow full mastery.

During training, if discipline learnedness crosses a skill's `start` threshold, the skill unlocks. For quest-locked skills, the system sets an eligible toggle via `setSpellEligibleToggle()` and provides a message directing the character to seek a specific NPC or location.

### Quest-Locked Skill Flow

Quest-locked skills require two-stage access. First, train the discipline to the skill's `start` value to become eligible. Second, complete the associated quest to actually learn the skill. Quest completion sets a permanent toggle bit that persists across sessions.

### Do-Learning Mechanics

On skill use, `learnFromDoing()` checks eligibility (player, not arena, skill supports it), enforces cooldowns, then rolls for discipline and skill gains. Discipline gain chances: 1/200 for combat, 1/150 for advanced, 1/200 main + 1/400 associated for others. Skill gain chance scales with headroom: `1000 * pow((max - actual)/max, 3 - plotStat(WIS, 1.0, 2.5))`, minimum 15 if any headroom exists.

Do-learning messages vary by component type: material components trigger "more control over the powers," holy spells trigger deity favor messages, other skills trigger "skills honing" messages. At 100%, a special mastery message displays.

### Prerequisite System

Combat prerequisite: `(3.5 * level / 10) - 4` minimum percentage. Weapon specialization requires 92% in corresponding proficiency. Advanced disciplines require base discipline completion. Quest-locked skills set eligibility toggles even after discipline requirements are met.

### Spell Execution Flow

Spells return bit-flag combinations. Single-target damage spells check `reconcileDamage() == -1` and return `SPELL_SUCCESS | VICTIM_DEAD`. Area spells iterate with pre-increment (`*(it++)`), delete victims directly, null pointers, and return without death flags. Backfire damage returns `SPELL_CRIT_FAIL | CASTER_DEAD`.

### Meta-Discipline Bonuses

DISC_WIZARDRY, DISC_FAITH, and DISC_RITUALISM boost effective spell level: `level += 2 + (disc_learnedness / 34)`. At 100% learnedness, spells gain approximately 5 effective levels. These meta-disciplines increase automatically on level-up for appropriate classes.

### Adding New Spells

New spell integration requires modifications in three locations:

1. Add the spell enum to `spellNumT` in spells.h (spells before `SKILL_SLAM`, skills after)
2. Add the `spellInfo` construction call in `buildSpellArray()` in spell_info.cc with ~20 parameters
3. Implement the spell function in the appropriate discipline file (disc_mage.cc, disc_cleric.cc, etc.)

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

**Cause:** Discipline learnedness already at natural cap, or skill already at discipline cap. Note: practice points increase discipline learnedness, which raises the skill cap - actual skill value must be increased through do-learning.

**Fix:** Check `getNatLearnedness()` vs `getLearnedness()` for discipline, `getMaxSkillValue()` vs current for skill. Train at higher-level trainer or level up for more natural potential. Use the skill repeatedly to trigger do-learning.

### Skill not improving from use

**Symptom:** Using skill repeatedly but no do-learning messages.

**Cause:** Cooldown not elapsed, skill at max, in arena, or `startLearnDo == -1`.

**Fix:** Wait 30s/3min between attempts. Check `getMaxSkillValue()` against current. Verify skill has do-learning enabled in `discArray`. If skill has `startLearnDo = -1`, it does not support do-learning.

### Cannot train at trainer

**Symptom:** Trainer refuses to teach or discipline stops increasing.

**Cause:** Trainer level limit (60% cap), class mismatch, or prerequisite not met.

**Fix:** Find level 100 trainer. Verify class can learn discipline. Check combat prerequisite or proficiency requirement (92% for weapon specializations).

### Quest-locked skill not available

**Symptom:** Discipline learnedness exceeds skill start value but skill remains unavailable.

**Cause:** Quest toggle not set.

**Fix:** Check if skill has toggle value in `discArray`. Use `practice skill <name>` to see if quest message was displayed. Complete the quest indicated when discipline crossed start threshold.

### Piety cost seems wrong

**Symptom:** Spell costs different from enum value suggests.

**Cause:** Piety enums are 4x actual cost.

**Fix:** Divide enum value by 4 to get true cost. `PRAY_100` (enum 20) costs 5.0 piety.

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

**Fix:** Use practice points from the class that owns the discipline. Use `practice class <classname>` to view class-specific pools.

### Spell damage lower than expected

**Symptom:** Spell deals less damage than similar-level spells.

**Cause:** Low associated discipline learnedness. Spell damage scales with associated discipline, not just spell skill.

**Fix:** Check spell's `assDisc` value. Train the associated discipline. For mages, train elemental disciplines and `DISC_WIZARDRY`. For clerics, train `DISC_FAITH`. For shamans, train `DISC_RITUALISM`.

### Flag operation producing unexpected values

**Symptom:** Flag appears to be set, then becomes set again at double the value.

**Cause:** Using `+=` instead of `|=` for flag operations.

**Fix:** Replace all `flags += CONST` with `flags |= CONST` or `SET_BIT(flags, CONST)`.
