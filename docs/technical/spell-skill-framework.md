---
title: Spell and Skill Framework
description: Complete spell/skill system including data structures, discipline organization, learnedness progression, practice mechanics, and cost calculations
keywords: [spellInfo, discArray, spellNumT, CDiscipline, discNumT, learnedness, getMaxSkillValue, bSuccess, practice points, TrainerInfo, learnFromDoing, minMana, minPiety, minLifeforce, SPELL_SUCCESS, SPELL_FAIL, advanceLevel, disc_learnedness, learn_rate, skill_start]
category: Important Systems

last_updated: 2026-01-29
source_files: [code/code/misc/spell_info.cc, code/code/misc/discipline.h, code/code/misc/discipline.cc, code/code/misc/gaining.cc]
related:
  - character-foundation.md
  - combat-formulas.md
  - task-system.md
  - affects-system.md
  - experience-leveling.md
---

# Spell and Skill Framework

The spell and skill framework is a unified system that defines all character abilities (spells, skills, prayers) and organizes them into hierarchical disciplines that govern learning and effectiveness.

## Data Structures

### The discArray Global

```cpp
// code/code/misc/spell_info.cc
spellInfo* discArray[MAX_SKILL + 1];
```

This array holds definitions for **both** spells and skills. Entries are populated by `buildSpellArray()` at startup.

### The spellNumT Enum

```cpp
// code/code/misc/spells.h
enum spellNumT {
    DAMAGE_HOLY = -59,      // Negative: damage types (not spells)
    TYPE_UNDEFINED = -1,
    SPELL_GUST = 0,         // 0+: Spells
    // ...
    SKILL_SLAM,             // SKILL_SLAM+: Skills
    MAX_SKILL,              // End of valid indices
};
const spellNumT MIN_SPELL = spellNumT(TYPE_UNDEFINED + 1);  // = SPELL_GUST
const spellNumT MAX_SPELL = spellNumT(SKILL_SLAM);          // First skill index
```

### The spellInfo Struct

```cpp
// code/code/misc/spell2.h
class spellInfo {
public:
    const char* name;           // Display name ("heal light", "bash")
    skillUseClassT typ;         // SPELL_MAGE, SKILL_WARRIOR, etc.
    discNumT disc;              // Primary discipline (skill tree)
    discNumT assDisc;           // Associated discipline (damage scaling)
    statTypeT modifierStat;     // STAT_INT, STAT_WIS, STAT_STR, STAT_EXT
    taskDiffT task;             // TASK_TRIVIAL through TASK_IMPOSSIBLE
    lag_t lag;                  // LAG_0 through LAG_9
    positionTypeT minPosition;  // Minimum position to use
    int minMana;                // Mana cost (mages)
    int minLifeforce;           // Lifeforce cost (shamans)
    float minPiety;             // Piety cost (clerics) - SEE CRITICAL NOTE
    uint32_t targets;           // Target flags (TAR_*)
    uint32_t comp_types;        // Component flags (COMP_*)
    // ... fade messages, learning parameters, stats tracking ...
};
```

### The discNumT Enum

```cpp
// code/code/misc/discipline.h
enum discNumT {
  DISC_NONE = -1,
  DISC_MAGE = 0, DISC_AIR, DISC_FIRE, DISC_WATER, DISC_EARTH, DISC_SPIRIT,
  DISC_CLERIC, DISC_AEGIS, DISC_WRATH, DISC_AFFLICTIONS, DISC_CURES,
  DISC_WARRIOR, DISC_DUELING, DISC_BRAWLING, DISC_SOLDIERING,
  // ... 71 total disciplines
  MAX_DISCS
};
```

### CDiscipline Class

```cpp
// code/code/misc/discipline.h
class CDiscipline {
  private:
    int uNatLearnedness;   // Natural potential (0-100)
    int uLearnedness;      // Trained level (0-100)
    int uDoLearnedness;    // Experience from use (0-100)
  public:
    int ok_for_class;      // Bitmask of allowed classes
    virtual bool isBasic();
    virtual bool isFast();
    virtual bool isAutomatic();
};
```

### Concrete Discipline Classes

Each discipline has a concrete class with `CSkill` members:

```cpp
// code/code/disc/disc_mage.h
class CDMage : public CDiscipline {
  public:
    CSkill skFireball;
    CSkill skTeleport;
    bool isBasic() { return true; }
};
```

### CMasterDiscipline

Each character has all discipline instances: `CDiscipline* disc[MAX_DISCS]`

## Discipline Organization

### The 71 Disciplines

Spells and skills are organized into 71 disciplines across all classes:

| Class    | Base Discipline | Advanced Disciplines |
|----------|-----------------|----------------------|
| Mage     | DISC_MAGE       | AIR, FIRE, WATER, EARTH, SPIRIT, SORCERY, ALCHEMY |
| Cleric   | DISC_CLERIC     | AEGIS, WRATH, AFFLICTIONS, CURES, HAND_OF_GOD |
| Warrior  | DISC_WARRIOR    | DUELING, BRAWLING, SOLDIERING, BLACKSMITHING |
| Thief    | DISC_THIEF      | STEALTH, MURDER, LOOTING, POISONS, TRAPS |
| Monk     | DISC_MONK       | MEDITATION, LEVERAGE, MINDBODY, FOCUSED_ATTACKS |
| Deikhan  | DISC_DEIKHAN    | MOUNTED, MARTIAL, GUARDIAN, ABSOLUTION, VENGEANCE |
| Ranger   | DISC_RANGER     | ANIMAL, PLANTS, NATURE |
| Shaman   | DISC_SHAMAN     | FROG, SPIDER, SKUNK, ARMADILLO, CONTROL, HEALING |

### Universal Disciplines

Available to all or multiple classes:
- `DISC_ADVENTURING` - Basic skills (all, automatic)
- `DISC_COMBAT` - Combat skills (all)
- `DISC_SLASH/BLUNT/PIERCE/RANGED` - Weapon specializations

### Discipline Types

- **Basic** (`isBasic()`): Primary class disciplines with core abilities
- **Fast** (`isFast()`): Weapon specializations and combat disciplines
- **Automatic** (`isAutomatic()`): Learned automatically without practice points (e.g., `DISC_ADVENTURING`, `DISC_WIZARDRY`)

### Spell-to-Discipline Mapping

```cpp
discNumT getDisciplineNumber(spellNumT spell) {
    return discArray[spell]->disc;
}
```

The `spellInfo` structure stores:
- `disc`: Primary discipline - determines which skill tree teaches the spell
- `assDisc`: Associated discipline - affects damage scaling and do-learning
- `start`: Discipline learnedness required to access
- `learn`: Learning rate multiplier

```cpp
// Fire spell in fire tree, scales with fire
discArray[SPELL_FIREBALL] = new spellInfo(SPELL_MAGE, DISC_FIRE, DISC_FIRE, ...);

// General mage spell, scales with air element
discArray[SPELL_GUST] = new spellInfo(SPELL_MAGE, DISC_MAGE, DISC_AIR, ...);
```

## Learnedness System

Three values track discipline mastery:

| Type | Description | Range |
|------|-------------|-------|
| **Natural** | Base potential from class/level; caps trained learnedness | 0-100 |
| **Learnedness** | Trained via practice points; affects skill success/damage | 0-natLearn |
| **Do-Learnedness** | Gained passively through actual use ("learning by doing") | 0-100 |

### Natural Learnedness

- Increases automatically with level
- Acts as a cap on trained learnedness
- Class-dependent: classes have higher natural potential in their primary disciplines
- Never decreases

### Trained Learnedness (Practice-Based)

- Improved by spending practice points at trainers
- Cannot exceed natural learnedness
- Directly affects skill success rates and power
- Primary factor in `getMaxSkillValue()` calculation

### Do-Learnedness

- Increases through actual use of skills in the discipline
- Independent of practice points
- Provides gradual improvement without trainer visits
- Can improve even after natural/trained caps are reached

### Skill Value Calculation

```cpp
// From getMaxSkillValue():
tmp2 = cdisc->getLearnedness() - discArray[skill]->start + 1;
return min(discArray[skill]->learn * tmp2, MAX_SKILL_LEARNEDNESS);
```

**Formula breakdown**:
```
max_skill_value = (discipline_learnedness - skill_start + 1) × skill_learn_rate
```

**Example**: If you have 50% in Fire Magic discipline, and Fireball requires `start=20` with `learn=3`:
```
max_fireball = (50 - 20 + 1) × 3 = 93%
```

### Skill Value Types

Each skill tracks multiple values:

| Value | Description | Access Method |
|-------|-------------|---------------|
| **Natural** | Base value from practice + do-learning | `getNatSkillValue()` |
| **Actual** | Natural + equipment bonuses | `getSkillValue()` |
| **Max** | Cap based on discipline learnedness | `getMaxSkillValue()` |

## Practice System

### Practice Points

Practice points are class-specific. Each class has its own pool:
- Gained through leveling (see `docs/experience-leveling.md`)
- Spent at trainers to raise discipline learnedness
- Cannot be shared between classes

**Functions** (`code/code/misc/limits.cc`):
- `getPracs(classIndT)`: Get available pracs for a class
- `addPracs(short, classIndT)`: Add/subtract pracs
- `pracsPerLevel()`: Calculate pracs gained per level

### The Practice Command

The `practice` command (`code/code/misc/other.cc`) displays your disciplines and skills:

```
practice                    - Shows all disciplines
practice class <class>      - Shows disciplines for a class
practice discipline <disc>  - Shows skills in a discipline
practice skill <skill>      - Shows detailed skill info
practice <discipline>       - Shows skills in that discipline
```

### Trainers and Guild Requirements

Trainers are specialized NPCs that teach specific disciplines. Each trainer has:
- A specific discipline they teach
- A maximum level they can train to (60 or 100)
- Class restrictions on who they will teach

**Trainer Levels** (`code/code/misc/gaining.cc`):
- Level 60 trainers: Train up to 60% discipline learnedness
- Level 100 trainers: Train up to 100% discipline learnedness

To train, use: `practice <discipline> <number_of_pracs>`

### TrainerInfo Array

The `TrainerInfo` array (`code/code/misc/gaining.cc`) defines all trainers:

| Trainer | Discipline | Classes |
|---------|------------|---------|
| Air Magic | DISC_AIR | Mage |
| Alchemy | DISC_ALCHEMY | Mage |
| Fire Magic | DISC_FIRE | Mage |
| Combat | DISC_COMBAT | All |
| Adventuring | DISC_ADVENTURING | All |
| Slash Spec | DISC_SLASH | All |
| Faith | DISC_FAITH | Cleric, Deikhan |

### Discipline Raise Formula

When you spend a practice point, the discipline increases based on its type (`code/code/misc/gaining.cc`):

**Basic Disciplines** (class base disciplines):
- Increase by 1% per practice point

**Fast Disciplines** (weapon specializations):
- Increase by 1% per practice point

**Other Disciplines**:
Use a quadratic formula designed so that:
- Low learnedness = large gains per prac
- High learnedness = small gains per prac
- Takes exactly `PRACS_TO_MAX` (60) pracs to go from 0% to 100%

**Formula**:
```cpp
// Increase formula (linear)
f'(p) = A*p + C

Where:
  C = 200 / PRACS_TO_MAX - 1  // = 2.33
  A = 2/PRACS_TO_MAX - 200/(PRACS_TO_MAX^2)  // = -0.022

// At p=0:  increase = 2.33%
// At p=60: increase = 1%
```

### Prerequisites

Before training advanced disciplines, you must complete basic training:

**Combat Prerequisite** (`code/code/misc/gaining.cc`):
```
Combat must be at or above: (3.5 * level / 10) - 4
```

For example, at level 20: `(3.5 * 20 / 10) - 4 = 3%` minimum combat

**Weapon Specialization** (`code/code/misc/gaining.cc`):
- Requires 92% in the corresponding proficiency skill
- DISC_SLASH requires SKILL_SLASH_PROF >= 92%
- DISC_DEFENSE requires SKILL_DEFENSE >= 92%

### Skill Unlocking

Skills unlock when discipline learnedness reaches the skill's `start` value:

**During Training** (`code/code/misc/gaining.cc`):
```cpp
if ((initial < skill_start) && (final >= skill_start)) {
  // Skill just became available
  if (skill->toggle && !hasQuestBit(toggle)) {
    setSpellEligibleToggle(me, skill, SILENT_NO);  // Quest required
  } else {
    value = min(startLearnDo, learn_rate);
    setNatSkillValue(skill, value);
    // "You have just learned [skill]!"
  }
}
```

### Quest-Locked Skills

Some skills require quest completion even after meeting discipline requirements:

```cpp
// code/code/misc/gaining.cc
switch (spell) {
  case SPELL_TORNADO:
    setQuestBit(TOG_TORNADO_ELIGIBLE);
    // "Seek out the wise elf Salrik..."
    break;
  case SPELL_FIREBALL:
    setQuestBit(TOG_ELIGIBLE_FIREBALL);
    // "Seek out the mischevious mage Kallam..."
    break;
  // Many more...
}
```

### Guildmasters

Guildmasters (`GenericGuildMaster` at `code/code/misc/gaining.cc`) handle:
- Level gaining via `gain` command
- Quest availability checks
- Class-specific milestones

Unlike trainers, guildmasters do not teach disciplines directly.

## Do-Learning (Learning by Doing)

Skills can also improve through actual use. This system rewards players who actively use their abilities.

### How It Works

When you use a skill or spell, `learnFromDoing()` is called (`code/code/misc/discipline.cc`):

1. **Eligibility Check**:
   - Must be a player (not monster)
   - Must have a descriptor
   - Not in arena
   - Skill must support do-learning (`startLearnDo != -1`)
   - Must already know the skill

2. **Cooldown Check**:
   - Skill at 50% or below: Must wait 30 seconds between gains
   - Skill above 50%: Must wait 3 minutes between gains

3. **Discipline Do-Learning**:
   - Small chance to increase the discipline's `doLearnedness` value
   - Combat disciplines: 1/200 chance
   - Advanced disciplines: 1/150 chance (no associated disc learning)
   - Other disciplines: 1/200 main disc, 1/400 associated disc

4. **Skill Learning**:
   - Cannot exceed `getMaxSkillValue()` (discipline cap)
   - Chance based on current skill vs max:
     ```cpp
     amount = (max - actual) / max  // 0.0 to 1.0
     power = 3.0 - plotStat(WIS, 1.0, 2.5)  // Higher WIS = lower power
     chance = 1000 * pow(amount, power)
     minimum_chance = 15 (if amount > 0)
     ```

### Do-Learning Parameters

Each skill has parameters in `spellInfo` (`code/code/misc/spell2.h`):

| Parameter | Description |
|-----------|-------------|
| `startLearnDo` | Initial skill value when first learned via training (-1 = no do-learning) |
| `amtLearnDo` | Amount to increase per do-learn success (usually 1) |
| `learnDoDiff` | Difficulty modifier for learning |
| `secStartLearnDo` | Secondary skill starting value |
| `secAmtLearnDo` | Secondary skill increase amount |

**Enum Values** (`code/code/misc/spell2.h`):
```cpp
enum discStartDoT {
  START_DO_NO = -1,   // No do-learning
  START_DO_1 = 1,     // Start at 1%
  START_DO_10 = 10,   // Start at 10%
  START_DO_25 = 25,   // Start at 25%
  // ... up to START_DO_60
};

enum discLearnDoT {
  LEARN_DO_NO = -1,   // No do-learning
  LEARN_DO_1 = 1,     // +1% per success
  LEARN_DO_2 = 2,     // +2% per success
  // ... up to LEARN_DO_5
};
```

### Do-Learning Messages

When you gain skill through use (`code/code/misc/discipline.cc`):

- **Material components**: "You feel you have more control over the powers of [skill]."
- **Holy spells**: "You feel [deity] favoring you more in respects to [skill]."
- **Other skills**: "You feel your skills honing in regards to [skill]."

At 100%: "You feel you have total mastery over [skill]."

### Weapon Proficiency Learning

Weapon proficiencies use `learnFromDoingUnusual()` (`code/code/misc/discipline.cc`) for special handling:

```cpp
switch (weapon_type) {
  case SKILL_SLASH_PROF:
    spell = SKILL_SLASH_PROF;
    spell2 = SKILL_SLASH_SPEC;  // Can chain to specialization
    break;
  // Similar for BLUNT, PIERCE, RANGED
}

// Random delay to slow proficiency gains
if (amt && ::number(0, amt)) {
  return FALSE;  // Arbitrary skip
}
```

### Automatic Disciplines

Some disciplines increase automatically without spending practice points:

| Discipline | When | Amount |
|------------|------|--------|
| `DISC_WIZARDRY` | Mage level up | 2-3 points |
| `DISC_RITUALISM` | Shaman level up | 2-3 points |
| `DISC_FAITH` | Cleric/Deikhan level up | 2-3 points |
| `DISC_ADVENTURING` | Any level up | 1-2 points (multiclass reduced) |

**Code Location**: `code/code/misc/gaining.cc`

## Cost Systems by Class

Each class uses exactly one cost resource:

| Class   | Field          | Enum Type        | Notes |
|---------|----------------|------------------|-------|
| Mage    | `minMana`      | `manaCostT`      | Direct value |
| Shaman  | `minLifeforce` | `lifeforceCostT` | Direct value |
| Cleric  | `minPiety`     | `pietyCostT`     | **Divided by 4** |

### CRITICAL: Piety Cost Division

```cpp
// code/code/misc/spell2.h - Enum values are 4x actual cost
enum pietyCostT {
    PRAY_025 = 5,   // Actual: 1.25
    PRAY_100 = 20,  // Actual: 5.0
    PRAY_200 = 40,  // Actual: 10.0
};
// Constructor divides: minPiety(static_cast<float>(minPiety) / 4)
```

**Example:** `PRAY_100` (enum value 20) becomes actual piety cost of **5.0**.

## Target Flags

```cpp
TAR_CHAR_ROOM     // Target character in room
TAR_FIGHT_SELF    // Default to self in combat
TAR_FIGHT_VICT    // Default to opponent in combat
TAR_SELF_ONLY     // Can only target self
TAR_SELF_NONO     // Cannot target self
TAR_OBJ_INV       // Target object in inventory
TAR_VIOLENT       // Initiates combat
TAR_AREA          // Area effect spell
TAR_NAME          // Target by name string
```

## Component Flags

```cpp
COMP_GESTURAL         // Requires hand gestures
COMP_VERBAL           // Requires verbal component
COMP_MATERIAL         // Requires material component
COMP_MATERIAL_END     // Consume material at end
SPELL_TASKED          // Uses task system (channeled)
SPELL_TASKED_EVERY    // Task pulses every round
```

## Spell Return Values (bSuccess)

Spell functions return a combination of bit flags indicating success/failure and death states:

| Constant | Bit | Meaning |
|----------|-----|---------|
| `SPELL_SUCCESS` | 1 << 1 | Spell succeeded |
| `SPELL_FAIL` | 1 << 2 | Spell failed |
| `SPELL_CRIT_FAIL` | 1 << 3 | Critical failure |
| `CASTER_DEAD` | 1 << 6 | Caster died (backfire) |
| `VICTIM_DEAD` | 1 << 7 | Victim died |

Combine with `+` or `|`: `return SPELL_SUCCESS + VICTIM_DEAD;`

### Critical: Damage Handling in Spells

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

## Meta-Discipline Bonuses

`DISC_WIZARDRY`, `DISC_FAITH`, and `DISC_RITUALISM` boost effective spell level:

```cpp
// Mage spells boosted by Wizardry
lev += 2 + (cd->getLearnedness() / 34);
```

## Adding a New Spell

1. **Add enum** in `spells.h` (before `SKILL_SLAM` for spells):
```cpp
SPELL_YOUR_NEW_SPELL,
```

2. **Add entry** in `spell_info.cc` within `buildSpellArray()`:
```cpp
discArray[SPELL_YOUR_NEW_SPELL] = new spellInfo(
    SPELL_MAGE,           // Class type
    DISC_FIRE,            // Primary discipline
    DISC_FIRE,            // Associated discipline
    STAT_INT,             // Modifier stat
    "your new spell",     // Display name
    TASK_NORMAL,          // Difficulty
    LAG_2,                // Lag rounds
    POSITION_SITTING,     // Minimum position
    MANA_30,              // Mana cost
    LIFEFORCE_0,          // Lifeforce (0 for mages)
    PRAY_0,               // Piety (0 for mages)
    TAR_CHAR_ROOM | TAR_VIOLENT | TAR_FIGHT_VICT | TAR_SELF_NONO,
    SYMBOL_STRESS_0,      // Holy symbol stress
    "", "", "", "",       // Fade messages
    START_20, LEARN_10,   // Learning parameters
    START_DO_30, LEARN_DO_5, START_DO_NO, LEARN_DO_NO,
    LEARN_DIFF_SPELLS,    // Learning difficulty
    0.04,                 // Alignment modifier
    COMP_GESTURAL | COMP_VERBAL | SPELL_TASKED,
    0                     // Toggle
);
```

3. **Implement function** in appropriate `disc_*.cc` file.

## Common Gotchas

### 1. Piety Cost Division
Piety enum values are 4x the actual cost. Always remember the division by 4 in the constructor.

### 2. reconcileDamage Death Detection
Returns `-1` on death, NOT a DELETE flag. Check `== -1`, never use `IS_SET_DELETE()`.

### 3. Flag Operations
Use `|=` for bit flags, NOT `+=`. Addition corrupts flags if already set:
```cpp
// CORRECT
obj_flags.extra_flags |= ITEM_GLOW;
SET_BIT(obj_flags.extra_flags, ITEM_MAGIC);

// WRONG
obj_flags.extra_flags += ITEM_GLOW;  // Doubles value if already set!
```

### 4. Discipline Cap on Skills
Skills cannot exceed the maximum allowed by their discipline. Even with 100% natural skill, if your discipline is at 50%, you effectively have a lower skill.

### 5. Do-Learning Cooldowns
You cannot gain skill through use faster than:
- 30 seconds at 50% or below
- 3 minutes above 50%

### 6. Trainer Level Limits
A level 60 trainer cannot train you past 60% discipline learnedness. You must find a level 100 trainer for full mastery.

### 7. Class-Specific Practices
Practice points are per-class. A multiclass character cannot use Mage pracs to train Cleric disciplines.

### 8. Prerequisite Chains
Advanced disciplines require basic disciplines to be trained first:
```
Combat (100%) -> Weapon Prof (92%) -> Weapon Spec
Base Class Disc (100%) -> Advanced Disciplines
```

### 9. Quest Toggles
Some skills require quest completion. The discipline level only makes you *eligible* - you still must complete the quest to actually learn.

### 10. Do-Learning vs Training
- Training: Increases discipline learnedness, which raises skill cap
- Do-learning: Increases actual skill value, capped by discipline
- Both are needed for maximum skill

## Key Constants

```cpp
const byte MAX_SKILL_LEARNEDNESS = 100;
const byte MAX_DISC_LEARNEDNESS = 100;
const int PRACS_TO_MAX = 60;  // Practice points to fully train
const int SKILL_MIN = -99;    // Not learned
```

## Key Source Files

| File | Purpose |
|------|---------|
| `code/code/misc/spell_info.cc` | `discArray[]`, `buildSpellArray()` |
| `code/code/misc/spell2.h` | `spellInfo` struct, flags |
| `code/code/misc/spells.h` | `spellNumT` enum |
| `code/code/disc/disc_*.cc` | Spell implementations |
| `code/code/misc/discipline.h` | `CDiscipline`, `discNumT` |
| `code/code/misc/discipline.cc` | `getMaxSkillValue()`, `learnFromDoing()` |
| `code/code/misc/gaining.cc` | Trainers, practice, discipline raising |
| `code/code/misc/other.cc` | `doPractice()` command |
| `code/code/misc/limits.cc` | `getPracs()`, practice point tracking |
| `code/code/misc/skills.cc` | `getSkillValue()` |

## Flag Manipulation Helpers

Use the provided inline functions from `structs.h`:

| Function | Purpose |
|----------|---------|
| `SET_BIT(a, b)` | Set flag: `a |= b` |
| `REMOVE_BIT(a, b)` | Clear flag: `a &= ~b` |
| `IS_SET(a, b)` | Check if any bit in b is set |
| `IS_SET_ONLY(a, b)` | Check if all bits in b are set |

For DELETE flags specifically, use `IS_SET_DELETE()`, `ADD_DELETE()`, `REM_DELETE()`.
