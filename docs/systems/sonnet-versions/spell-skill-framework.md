---
title: Spell and Skill Framework
category: important
keywords: [spellInfo, discArray, CDiscipline, learnedness, practice, do-learning, bSuccess, reconcileDamage, getMaxSkillValue, TrainerInfo]
related: [character-foundation.md, combat-formulas.md, task-system.md, affects-system.md, experience-leveling.md]
primary_symbols:
  functions: [buildSpellArray, getMaxSkillValue, learnFromDoing, reconcileDamage, getDisciplineNumber]
  classes: [spellInfo, CDiscipline, CMasterDiscipline]
  files: [code/misc/spell_info.cc, code/misc/discipline.cc, code/misc/gaining.cc]
---

## Overview

How does a mage's fireball become more powerful as they study Fire Magic? How does a warrior's bash improve through practice? The spell and skill framework is the unified system that defines all character abilities and governs how they grow stronger.

Every ability in the game—whether a spell, skill, or prayer—is a spell in the technical sense. The framework treats "spell" as the generic term for any learnable ability. Each spell belongs to a discipline (a skill tree), and disciplines organize into a hierarchy across 71 total branches. Your mastery of a discipline determines both which spells you can learn and how effective they are.

The system operates on a three-tiered learning model. Natural learnedness represents your innate potential and increases automatically with level. Trained learnedness reflects deliberate study through spending practice points at trainers. Do-learnedness captures passive improvement from actual use. All three values interact to determine your effective skill level in any given ability.

When you cast fireball, the game calculates your effective fireball skill by starting with your Fire Magic discipline learnedness, subtracting the spell's minimum requirement, and multiplying by the spell's learning rate. If you have 50% Fire Magic and fireball requires 20% with a 3x learning rate, your effective fireball skill is `(50 - 20 + 1) × 3 = 93%`. This value caps your success chance, damage output, and all other effectiveness metrics.

Classes differ in their resource costs. Mages spend mana, shamans consume lifeforce, and clerics expend piety. Each spell definition includes cost values for all three resources, though only one applies based on the caster's class. The piety system has a quirk: enum values are stored at 4x actual cost and divided during initialization.

Skills can improve through two paths. The deliberate path involves visiting trainers and spending practice points to raise discipline learnedness, which increases the skill cap for all spells in that discipline. The passive path triggers when you actually use a skill—there's a chance it improves through do-learning, though this is capped by your discipline learnedness and subject to cooldowns.

Combat spells that deal damage must handle death correctly. The `reconcileDamage()` function returns -1 when the victim dies, not a DELETE flag. Single-target spells return death flags to their caller, while area-effect spells delete victims immediately during iteration.

## Patterns

### Discipline Access and Training

**DO** check natural learnedness before allowing training. Natural learnedness acts as a cap on trained learnedness. Characters cannot train a discipline beyond their natural potential.

**DO** verify prerequisite disciplines before allowing advanced training. Combat must reach a level-based threshold before weapon skills become available. Weapon specializations require 92% in the corresponding proficiency.

**DO** use class-specific practice point pools. Each class has its own practice point allocation. A multiclass character's mage pracs cannot be spent on cleric disciplines. Use `getPracs(classIndT)` to retrieve the correct pool.

**NEVER** allow training past trainer level limits. A level 60 trainer cannot raise discipline learnedness above 60%. Characters must find level 100 trainers for full mastery.

**NEVER** allow automatic discipline increases during regular training. Only specific disciplines increase automatically on level-up: `DISC_WIZARDRY`, `DISC_RITUALISM`, `DISC_FAITH`, and `DISC_ADVENTURING`. All others require explicit practice point expenditure.

### Skill Value and Cap Calculation

**ALWAYS** use `getMaxSkillValue()` to determine skill caps. This function calculates the maximum achievable value based on current discipline learnedness. Skills cannot exceed this cap regardless of bonuses.

**ALWAYS** distinguish between natural, actual, and max skill values. Natural skill (`getNatSkillValue()`) reflects practice and do-learning. Actual skill (`getSkillValue()`) includes equipment bonuses. Max skill (`getMaxSkillValue()`) represents the discipline-imposed ceiling.

**DO** respect the skill unlocking threshold. Skills become available when discipline learnedness reaches the skill's `start` value. Before this point, the skill is completely inaccessible.

**NEVER** assume high natural skill guarantees high effective skill. A character with 100% natural fireball skill but only 30% Fire Magic discipline will have drastically reduced effectiveness due to the discipline cap.

### Do-Learning Implementation

**ALWAYS** check eligibility before allowing do-learning. The character must be a player (not NPC), have a descriptor, not be in arena, and the skill must support do-learning (`startLearnDo != -1`).

**ALWAYS** enforce cooldowns on do-learning gains. Skills at 50% or below require 30-second intervals between gains. Skills above 50% require 3-minute intervals. These cooldowns prevent rapid grinding.

**DO** implement the wisdom-modified learning curve. Learning chance decreases as skill approaches cap: `chance = 1000 × pow((max - actual) / max, power)` where power is reduced by higher wisdom. This creates faster initial learning that slows near mastery.

**NEVER** allow do-learning to exceed `getMaxSkillValue()`. Even through passive use, skills cannot surpass the discipline-imposed maximum.

**NEVER** skip the discipline do-learning chance. When a skill improves through use, there's also a small chance to increase the discipline's `doLearnedness` value. Combat disciplines: 1/200 chance. Advanced disciplines with no associated discipline: 1/150 chance. Other disciplines: 1/200 for main, 1/400 for associated.

### Death Handling in Spells

**CRITICAL:** `reconcileDamage()` returns -1 on victim death, NOT a DELETE flag. Check the return value against -1, never use `IS_SET_DELETE()` on the result.

```cpp
// CORRECT
if (caster->reconcileDamage(victim, dam, SPELL_FIREBALL) == -1)
    return SPELL_SUCCESS + VICTIM_DEAD;

// WRONG - IS_SET_DELETE never triggers
if (IS_SET_DELETE(rc, DELETE_VICT)) { ... }
```

**ALWAYS** return death flags for single-target spells. When `reconcileDamage()` returns -1, return `SPELL_SUCCESS + VICTIM_DEAD`. The caller is responsible for deletion.

**ALWAYS** delete victims directly in area-effect spells. Area spells iterate over room contents. When a victim dies, delete them immediately and null the pointer to prevent use-after-free. Do not return `VICTIM_DEAD` flags—you already handled deletion.

```cpp
for (auto it = caster->roomp->stuff.begin(); it != caster->roomp->stuff.end();) {
    t = *(it++);  // Advance iterator BEFORE potential deletion
    vict = dynamic_cast<TBeing*>(t);
    if (!vict) continue;

    if (caster->reconcileDamage(vict, dam, SPELL_PEBBLE_SPRAY) == -1) {
        delete vict;
        vict = nullptr;
    }
}
return SPELL_SUCCESS;  // No VICTIM_DEAD flag
```

**ALWAYS** handle caster backfire death. When critical failures damage the caster, check if the caster died and return `SPELL_CRIT_FAIL + CASTER_DEAD`.

### Quest-Locked Skills

**ALWAYS** check quest toggle bits before granting quest-locked skills. Some skills require quest completion even after meeting discipline requirements. During training, if discipline learnedness crosses a skill's `start` threshold, check `hasQuestBit(toggle)` before granting the skill.

**DO** use `setSpellEligibleToggle()` when discipline requirements are met but quest is incomplete. This notifies the player they need to seek out a specific NPC or location to complete the quest.

**NEVER** grant quest-locked skills without the toggle bit. The quest system prevents premature access to powerful abilities like tornado, fireball, and many class-defining spells.

### Cost System Quirks

**CRITICAL:** Piety costs are stored at 4x actual value. The `pietyCostT` enum defines values 4x larger than the actual piety consumed. The `spellInfo` constructor divides by 4 during initialization. `PRAY_100` (enum value 20) becomes actual cost 5.0.

**ALWAYS** set exactly one cost value per spell. Mages use `minMana`, shamans use `minLifeforce`, clerics use `minPiety`. Set unused costs to 0. Do not mix resource types for a single spell.

**NEVER** modify piety costs without accounting for the division. When adding new cleric spells, remember that `PRAY_200` (enum 40) yields actual cost 10.0, not 40.0.

### Flag Manipulation

**ALWAYS** use bitwise OR for flag operations, never addition. Addition corrupts flags if the bit is already set.

```cpp
// CORRECT
obj_flags.extra_flags |= ITEM_GLOW;
SET_BIT(obj_flags.extra_flags, ITEM_MAGIC);

// WRONG - doubles value if already set
obj_flags.extra_flags += ITEM_GLOW;
```

**ALWAYS** use provided helper macros. `SET_BIT()`, `REMOVE_BIT()`, `IS_SET()`, and `IS_SET_ONLY()` prevent common mistakes. For DELETE flags specifically, use `IS_SET_DELETE()`, `ADD_DELETE()`, `REM_DELETE()`.

## Reference

### Primary Symbol Reference

| Symbol | Type | Location | Purpose |
|--------|------|----------|---------|
| `discArray` | global array | spell_info.cc | Holds all spell/skill definitions, indexed by spellNumT |
| `buildSpellArray()` | function | spell_info.cc | Populates discArray at startup with all spell definitions |
| `spellInfo` | class | spell2.h | Defines a single spell/skill (name, costs, targets, learning params) |
| `spellNumT` | enum | spells.h | Enumerates all spells and skills, negative for damage types |
| `CDiscipline` | class | discipline.h | Base class for discipline skill trees, tracks learnedness |
| `discNumT` | enum | discipline.h | Enumerates 71 disciplines across all classes |
| `getMaxSkillValue()` | function | discipline.cc | Calculates skill cap based on discipline learnedness |
| `learnFromDoing()` | function | discipline.cc | Implements passive skill improvement through use |
| `reconcileDamage()` | function | combat.cc | Applies damage, returns -1 on death |
| `getDisciplineNumber()` | function | spell_info.cc | Returns primary discipline for a given spell |
| `TrainerInfo` | array | gaining.cc | Defines all NPC trainers and their specializations |

### Spell Return Flags

| Constant | Bit | Meaning |
|----------|-----|---------|
| `SPELL_SUCCESS` | 1 << 1 | Spell succeeded |
| `SPELL_FAIL` | 1 << 2 | Spell failed |
| `SPELL_CRIT_FAIL` | 1 << 3 | Critical failure |
| `CASTER_DEAD` | 1 << 6 | Caster died (backfire) |
| `VICTIM_DEAD` | 1 << 7 | Victim died |

Combine with `+` or `|`: `return SPELL_SUCCESS + VICTIM_DEAD;`

### Target Flags

| Flag | Meaning |
|------|---------|
| `TAR_CHAR_ROOM` | Target character in room |
| `TAR_FIGHT_SELF` | Default to self in combat |
| `TAR_FIGHT_VICT` | Default to opponent in combat |
| `TAR_SELF_ONLY` | Can only target self |
| `TAR_SELF_NONO` | Cannot target self |
| `TAR_OBJ_INV` | Target object in inventory |
| `TAR_VIOLENT` | Initiates combat |
| `TAR_AREA` | Area effect spell |
| `TAR_NAME` | Target by name string |

### Component Flags

| Flag | Meaning |
|------|---------|
| `COMP_GESTURAL` | Requires hand gestures |
| `COMP_VERBAL` | Requires verbal component |
| `COMP_MATERIAL` | Requires material component |
| `COMP_MATERIAL_END` | Consume material at end |
| `SPELL_TASKED` | Uses task system (channeled) |
| `SPELL_TASKED_EVERY` | Task pulses every round |

### Learnedness Types

| Type | Range | Description |
|------|-------|-------------|
| Natural | 0-100 | Innate potential from class/level, caps trained learnedness |
| Trained | 0-natural | Improved via practice points, affects skill success/damage |
| Do-Learnedness | 0-100 | Gained through actual use, independent of practice |

### Skill Value Types

| Value | Access Method | Description |
|-------|---------------|-------------|
| Natural | `getNatSkillValue()` | Base value from practice + do-learning |
| Actual | `getSkillValue()` | Natural + equipment bonuses |
| Max | `getMaxSkillValue()` | Cap based on discipline learnedness |

### Discipline Types

| Type | Characteristic | Examples |
|------|----------------|----------|
| Basic | Core class disciplines | `DISC_MAGE`, `DISC_CLERIC`, `DISC_WARRIOR` |
| Fast | Quick-training weapon specs | `DISC_SLASH`, `DISC_BLUNT`, `DISC_PIERCE` |
| Automatic | No practice points required | `DISC_ADVENTURING`, `DISC_WIZARDRY` |

### Cost Resources by Class

| Class | Field | Enum Type | Notes |
|-------|-------|-----------|-------|
| Mage | `minMana` | `manaCostT` | Direct value |
| Shaman | `minLifeforce` | `lifeforceCostT` | Direct value |
| Cleric | `minPiety` | `pietyCostT` | **Divided by 4** |

### Do-Learning Parameters

| Parameter | Description |
|-----------|-------------|
| `startLearnDo` | Initial skill value when first learned via training (-1 = no do-learning) |
| `amtLearnDo` | Amount to increase per do-learn success |
| `learnDoDiff` | Difficulty modifier for learning |
| `secStartLearnDo` | Secondary skill starting value |
| `secAmtLearnDo` | Secondary skill increase amount |

### Key Constants

| Constant | Value | Purpose |
|----------|-------|---------|
| `MAX_SKILL_LEARNEDNESS` | 100 | Maximum skill percentage |
| `MAX_DISC_LEARNEDNESS` | 100 | Maximum discipline percentage |
| `PRACS_TO_MAX` | 60 | Practice points to fully train a discipline |
| `SKILL_MIN` | -99 | Indicates skill not learned |
| `MAX_SKILL` | enum end | End of valid spell indices |
| `MIN_SPELL` | `TYPE_UNDEFINED + 1` | First valid spell index |

### Automatic Discipline Increases

| Discipline | Class | Trigger | Amount |
|------------|-------|---------|--------|
| `DISC_WIZARDRY` | Mage | Level up | 2-3 points |
| `DISC_RITUALISM` | Shaman | Level up | 2-3 points |
| `DISC_FAITH` | Cleric/Deikhan | Level up | 2-3 points |
| `DISC_ADVENTURING` | All | Level up | 1-2 points (reduced for multiclass) |

## Implementation

### Data Structure Architecture

The spell framework centers on the `discArray` global array, which holds definitions for all spells and skills. Despite the name, this array indexes both spell and skill data. The `spellNumT` enum provides the indices. Negative values represent damage types (not actual spells), zero and positive values represent spells up to `SKILL_SLAM`, and values from `SKILL_SLAM` onward represent skills.

Each entry in `discArray` is a `spellInfo` instance. This class stores everything needed to execute and learn an ability: display name, class type, primary and associated disciplines, modifier stat, task difficulty, lag, minimum position, resource costs, target flags, component requirements, and learning parameters. The constructor accepts these values and performs necessary transformations, notably dividing piety costs by 4.

The `buildSpellArray()` function in spell_info.cc executes at server startup and allocates all `spellInfo` instances. Each spell receives a constructor call with approximately 20 parameters defining every aspect of its behavior. This centralized initialization ensures consistency and allows quick lookup during gameplay.

### Discipline Hierarchy

Disciplines organize into a class-based hierarchy encoded in the `discNumT` enum. Each class has one base discipline and multiple advanced disciplines. Base disciplines like `DISC_MAGE` represent core class identity. Advanced disciplines like `DISC_FIRE` and `DISC_WATER` provide specialization within the class.

Universal disciplines cross class boundaries. `DISC_ADVENTURING` provides basic survival skills to all classes and increases automatically on level-up. `DISC_COMBAT` teaches fundamental combat techniques. Weapon proficiency disciplines (`DISC_SLASH_PROF`, `DISC_BLUNT_PROF`) and specialization disciplines (`DISC_SLASH`, `DISC_BLUNT`) govern melee effectiveness.

Each discipline is implemented as a concrete class inheriting from `CDiscipline`. The base class provides three learnedness values (natural, trained, do-learnedness) and query methods (`isBasic()`, `isFast()`, `isAutomatic()`). Concrete discipline classes like `CDMage` contain `CSkill` members representing individual abilities within that discipline.

Characters maintain a `CMasterDiscipline` instance containing pointers to all 71 discipline instances. This allows the practice system to iterate over available disciplines and the combat system to query specific skill values efficiently.

### Spell-to-Discipline Mapping

Every spell stores two discipline references. The primary discipline (`disc`) determines which skill tree teaches the spell and provides the base for cap calculation. The associated discipline (`assDisc`) affects damage scaling and do-learning progression. For elemental spells, both values often match (fireball has `disc=DISC_FIRE` and `assDisc=DISC_FIRE`). For general spells, they diverge (gust has `disc=DISC_MAGE` and `assDisc=DISC_AIR`).

The `getDisciplineNumber()` function retrieves the primary discipline for any spell by indexing `discArray` and returning the `disc` field. This allows generic code to determine which discipline governs learning and effectiveness without spell-specific logic.

Two additional fields control learning progression. The `start` value specifies the discipline learnedness required before the spell becomes available. A spell with `start=20` requires 20% discipline learnedness before it can be learned. The `learn` value provides a multiplier for calculating maximum skill value. Higher multipliers allow skills to reach higher effective values from the same discipline learnedness.

### Learnedness Calculation and Caps

The `getMaxSkillValue()` function implements the core formula linking discipline mastery to spell effectiveness. It retrieves the character's current discipline learnedness for the spell's primary discipline, subtracts the spell's `start` requirement, adds 1, multiplies by the spell's `learn` rate, and caps at `MAX_SKILL_LEARNEDNESS` (100).

```
max_skill_value = min((discipline_learnedness - spell_start + 1) × spell_learn_rate, 100)
```

This formula creates a dependency chain where improving discipline learnedness raises caps for all spells in that discipline. A character with 30% Fire Magic might have fireball capped at 33%, while the same character with 60% Fire Magic could reach 123% (capped to 100%). The learn rate determines how quickly skills scale—higher rates mean fewer discipline points needed to maximize the skill.

Natural learnedness increases automatically with level according to class-specific formulas. Each class has different natural growth rates for different disciplines. Mages gain natural learnedness in `DISC_MAGE` and elemental disciplines faster than warriors. This implements class identity at the mechanical level.

Trained learnedness requires practice point expenditure at trainers. The increase per practice point varies by discipline type. Basic disciplines increase by 1% per point. Fast disciplines (weapon specs) increase by 1% per point. Other disciplines use a quadratic formula designed so that early points provide larger gains (starting at ~2.33%) and later points provide smaller gains (ending at 1%), with exactly 60 points required to go from 0% to 100%.

Do-learnedness accumulates independently through the `learnFromDoing()` system. This value contributes to natural skill value but has a smaller coefficient than trained learnedness. It provides a long-term passive progression mechanism for dedicated players.

### Practice Point System

Practice points are allocated per-class, stored separately for each class a character has levels in. The `getPracs(classIndT)` function retrieves the appropriate pool. Multiclass characters maintain distinct pools and cannot transfer points between classes.

Characters gain practice points on level-up according to `pracsPerLevel()`. This function considers class (some classes gain more pracs), wisdom (higher wisdom yields more pracs), and multiclass status (multiclass characters gain fewer total pracs).

The practice command displays discipline and skill information across multiple modes. `practice` alone lists all disciplines. `practice class <class>` filters to class-specific disciplines. `practice discipline <disc>` shows skills within that discipline. `practice skill <skill>` provides detailed skill information including current value, max value, and learning parameters.

### Trainer System

Trainers are NPCs with specialized knowledge. Each trainer teaches one specific discipline to specific classes. The `TrainerInfo` array in gaining.cc defines all trainers, mapping virtual mob numbers to disciplines and class restrictions.

Trainers have level limits. Level 60 trainers can raise discipline learnedness to 60%. Level 100 trainers can raise it to 100%. Finding high-level trainers for advanced disciplines requires exploration and often involves remote or dangerous locations.

The training interaction follows a pattern. Character approaches trainer. Character uses `practice <discipline> <count>` syntax. System verifies character has sufficient practice points, character is appropriate class, and discipline is eligible for training. System calculates learnedness increase based on discipline type and current value. System deducts practice points and increases discipline learnedness.

During training, if discipline learnedness crosses a skill's `start` threshold, the skill unlocks. For non-quest-locked skills, the character immediately receives initial skill value. For quest-locked skills, the system sets an eligible toggle and provides a message directing the character to seek a specific NPC or location.

### Quest-Locked Skill Flow

Quest-locked skills require two-stage access. First, character must train the discipline to the skill's `start` value. This makes the character eligible. Second, character must complete a specific quest to actually learn the skill.

When discipline training makes a character eligible for a quest-locked skill, the system calls `setSpellEligibleToggle()`. This function sets a quest bit indicating eligibility and sends a message describing where to find the quest giver. The character must then travel to that location and interact with the quest NPC.

Quest completion sets a permanent toggle bit. The toggle bits are stored in character data and persist across sessions. Once a toggle is set, the character can use that skill freely. If the character loses discipline learnedness through death or other mechanics, the toggle persists—they don't need to repeat the quest.

### Do-Learning Implementation

The `learnFromDoing()` function executes whenever a character uses a skill or spell. It implements a multi-stage gating system. First, verify the character is a player with a descriptor, not in arena, and the skill supports do-learning. Second, check cooldown timers based on current skill level. Third, attempt discipline do-learning with class-specific probabilities. Fourth, calculate skill learning chance based on gap between current and maximum skill value.

The skill learning chance uses a power-law decay. As actual skill approaches max skill, the probability drops exponentially. The exponent is modified by wisdom—higher wisdom reduces the exponent, making the decay slower and learning more consistent. This creates a fast-early, slow-late progression curve that rewards both active practice and natural talent.

When a skill gain succeeds, the system increases natural skill value by `amtLearnDo` (typically 1). It displays a message based on component type: material components trigger "more control over the powers," holy spells trigger deity favor messages, other skills trigger "skills honing" messages. At 100%, a special mastery message displays.

Weapon proficiencies use a variant system `learnFromDoingUnusual()` which implements additional random delays to slow proficiency gains. This prevents rapid grinding to maximum weapon skill and encourages diverse combat experiences.

### Death Handling in Damage Spells

Combat spells that deal damage invoke `reconcileDamage()`. This function applies damage, triggers damage events, checks for death, and returns status. The return value is -1 when the victim dies, not a DELETE flag. This is a critical distinction—many functions in the codebase use DELETE flags, but `reconcileDamage()` does not.

Single-target spells check for -1 and return `SPELL_SUCCESS + VICTIM_DEAD`. The caller (typically command dispatch) is responsible for deleting the victim and handling corpse creation. The spell function must not delete the victim or attempt to access victim data after detecting death.

Area-effect spells iterate over room contents. The iteration must use post-increment pattern `*(it++)` to advance the iterator before potential deletion. When `reconcileDamage()` returns -1, the spell function deletes the victim immediately and sets the pointer to nullptr. This prevents use-after-free when the victim is referenced later in the same function. Area spells return `SPELL_SUCCESS` without death flags because deletion already occurred.

Backfire damage follows the same pattern but targets the caster. When critical failures apply damage to the caster, the spell checks if caster died and returns `SPELL_CRIT_FAIL + CASTER_DEAD`. The caller handles caster deletion.

### Meta-Discipline Boosting

Meta-disciplines like `DISC_WIZARDRY`, `DISC_FAITH`, and `DISC_RITUALISM` modify effective spell level. These disciplines don't contain individual spells. Instead, they boost the power of all spells in related disciplines.

For mages, `DISC_WIZARDRY` learnedness increases effective spell level by `2 + (learnedness / 34)`. At 100% wizardry, spells gain approximately 5 effective levels. This increases damage, duration, success chance, and all other level-scaled effects.

Clerics and deikhans benefit from `DISC_FAITH` similarly. Shamans benefit from `DISC_RITUALISM`. These meta-disciplines provide a long-term progression path beyond individual spell training. Characters can continue to increase spell effectiveness even after maxing individual spell skills by improving the meta-discipline.

Meta-disciplines increase automatically on level-up for appropriate classes. Mages gain 2-3 points in `DISC_WIZARDRY` per level. This gradual increase ensures high-level characters have significantly more powerful spells than low-level characters with the same individual skill values.

### Cost Application and Resource Consumption

Spell costs are retrieved from the `spellInfo` structure during cast attempt. The system checks if the caster has sufficient resources based on class. Mages check `minMana`, shamans check `minLifeforce`, clerics check `minPiety`. If resources are insufficient, the cast fails before spell execution.

Piety costs have special handling. The enum values in `pietyCostT` are 4x the actual cost. During `spellInfo` construction, the piety value is divided by 4 and stored as a float. This quirk exists for historical reasons and must be accounted for when adding new cleric spells. An enum value of 20 becomes actual cost 5.0.

After resource verification succeeds, the spell function executes. Upon successful completion (return includes `SPELL_SUCCESS`), the caller deducts costs. This prevents resource consumption for failed casts. Critical failures may consume resources or impose additional penalties depending on spell-specific implementation.

### Adding New Spells

New spell integration requires modifications in three locations. First, add the spell enum to `spellNumT` in spells.h. Position matters—spells go before `SKILL_SLAM`, skills go after. The enum value becomes the index into `discArray`.

Second, add the `spellInfo` construction call in `buildSpellArray()` in spell_info.cc. This requires approximately 20 parameters: class type, primary discipline, associated discipline, modifier stat, display name, task difficulty, lag, minimum position, resource costs (mana/lifeforce/piety), target flags, symbol stress, fade messages, learning parameters (start/learn for both training and do-learning), learning difficulty, alignment modifier, component flags, and toggle bit for quest-locking.

Third, implement the spell function in the appropriate discipline file (disc_mage.cc, disc_cleric.cc, etc.). The function signature takes caster, level, and target parameters. Implementation must handle targeting validation, component checks, cost verification, effect application, message display, and death handling according to whether the spell is single-target or area-effect.

### Class-Specific Discipline Growth

Each class has different natural learnedness growth rates for different disciplines. Warriors gain natural combat disciplines faster than mages. Mages gain natural mage disciplines faster than warriors. This implements class identity mechanically—your class determines which abilities you can master efficiently.

The formulas are embedded in level-gain code. When a character levels, the system iterates over all disciplines and increases natural learnedness according to class-specific rates. Primary class disciplines increase fastest, secondary class disciplines (like combat for non-warriors) increase slower, and off-class disciplines may not increase at all.

Multiclass characters have complex interactions. Each class contributes to natural learnedness for its primary disciplines. A mage/warrior has strong natural growth in both mage and warrior disciplines but gains less total practice points per level. This creates strategic tradeoffs—multiclassing provides breadth at the cost of depth.

### Prerequisite Enforcement

Advanced disciplines require basic disciplines to reach thresholds before training becomes available. The combat prerequisite formula `(3.5 × level / 10) - 4` ensures characters learn basic combat skills before specializing. At level 10, this requires 0% combat (always satisfied). At level 20, this requires 3% combat. At level 50, this requires 13.5% combat.

Weapon specialization prerequisites are stricter. `DISC_SLASH` requires `SKILL_SLASH_PROF` at 92% or higher. This ensures characters have near-mastery of basic weapon use before specializing. The 92% threshold is hardcoded and consistent across all weapon types.

Class-based prerequisites prevent inappropriate training. A `DISC_MAGE` trainer will not teach warriors. This is encoded in the `TrainerInfo` array via class bitmasks. When a character attempts training, the system checks if their class appears in the allowed bitmask for that trainer.

## Troubleshooting

### Death Not Detected After reconcileDamage

**Symptom:** Spell continues execution after victim dies, leading to use-after-free or null pointer dereference.

**Likely cause:** Checking for DELETE flags instead of -1 return value.

**Diagnostic approach:** Search spell implementation for `IS_SET_DELETE(rc, DELETE_VICT)` or similar patterns. Verify the return value check is `== -1`, not flag-based.

**Fix:** Replace DELETE flag checks with direct -1 comparison: `if (reconcileDamage(...) == -1) return SPELL_SUCCESS + VICTIM_DEAD;`

### Skill Not Improving Through Practice

**Symptom:** Spending practice points at trainer does not increase skill values.

**Likely cause:** Confusion between discipline learnedness and skill value. Practice points increase discipline learnedness, which raises the skill cap. Actual skill value must be increased through do-learning or starts at initial value when first unlocked.

**Diagnostic approach:** Check `practice` output. Compare discipline learnedness to skill `start` value. Verify skill is unlocked (discipline learnedness >= skill start). Check skill max value via `getMaxSkillValue()`.

**Fix:** If discipline learnedness is below skill start, train the discipline further. If discipline learnedness is sufficient but skill value is low, use the skill repeatedly to trigger do-learning.

### Do-Learning Not Triggering

**Symptom:** Repeated skill use does not increase skill value.

**Likely cause:** Cooldown timer or skill already at cap.

**Diagnostic approach:** Check if skill value equals max skill value (at discipline cap). Verify sufficient time has passed since last do-learn gain (30 seconds if skill <= 50%, 3 minutes if skill > 50%). Confirm `startLearnDo != -1` for the skill in `discArray`.

**Fix:** If at cap, train discipline to raise cap. If on cooldown, wait required time. If skill has `startLearnDo = -1`, it does not support do-learning and can only be improved via practice points.

### Trainer Refuses to Train

**Symptom:** Trainer does not accept `practice <discipline>` command.

**Likely cause:** Class restriction, trainer level limit already reached, or prerequisite not met.

**Diagnostic approach:** Verify character class matches trainer's allowed classes in `TrainerInfo`. Check if current discipline learnedness equals trainer's level limit (60% or 100%). Verify combat prerequisite if training weapon skills. Check weapon proficiency >= 92% if training weapon specialization.

**Fix:** Find appropriate trainer for your class. Seek higher-level trainer if at 60% cap. Train combat discipline if prerequisite fails. Train weapon proficiency to 92% before attempting specialization.

### Quest-Locked Skill Not Available

**Symptom:** Discipline learnedness exceeds skill start value but skill remains unavailable.

**Likely cause:** Quest toggle not set.

**Diagnostic approach:** Check if skill has toggle value in `discArray` definition. Use `practice skill <name>` to see if quest message was displayed. Verify quest bit status in character data.

**Fix:** Complete the quest indicated in the message displayed when discipline crossed start threshold. Seek the specific NPC or location mentioned. Interact to trigger quest completion and toggle setting.

### Piety Cost Seems Wrong

**Symptom:** Cleric spell consumes different piety than expected from enum value.

**Likely cause:** Forgetting the division-by-4 quirk in piety cost system.

**Diagnostic approach:** Check `pietyCostT` enum value in spell_info.cc. Divide by 4 to get actual cost. Verify in-game consumption matches divided value, not enum value.

**Fix:** This is working as designed. `PRAY_100` (enum 20) costs 5.0 piety. To change actual cost, change enum value to 4x desired cost.

### Spell Damage Lower Than Expected

**Symptom:** Spell deals less damage than similar-level spells.

**Likely cause:** Low associated discipline learnedness. Spell damage scales with associated discipline, not just spell skill.

**Diagnostic approach:** Check spell's `assDisc` value in `discArray`. Compare character's learnedness in that discipline to primary discipline. Verify meta-discipline (`DISC_WIZARDRY`, `DISC_FAITH`, `DISC_RITUALISM`) learnedness.

**Fix:** Train the associated discipline. For mages, train elemental disciplines and `DISC_WIZARDRY`. For clerics, train `DISC_FAITH`. For shamans, train `DISC_RITUALISM`.

### Flag Operation Producing Unexpected Values

**Symptom:** Flag appears to be set, then becomes set again at double the value.

**Likely cause:** Using `+=` instead of `|=` for flag operations.

**Diagnostic approach:** Search code for `flags += FLAG_NAME` or similar. Verify flag operations use bitwise OR, not arithmetic addition.

**Fix:** Replace all `flags += CONST` with `flags |= CONST` or `SET_BIT(flags, CONST)`. Test that repeated setting does not change value.

### Multiclass Practice Points Disappearing

**Symptom:** Practice points decrease unexpectedly or seem to vanish.

**Likely cause:** Spending points from wrong class pool. Each class has separate practice point allocation.

**Diagnostic approach:** Use `practice class <classname>` to view class-specific practice points. Verify training a discipline appropriate to the class whose points are being spent.

**Fix:** Train disciplines using the appropriate class's practice pool. A mage/warrior must spend mage pracs on mage disciplines and warrior pracs on warrior disciplines. Points cannot be transferred between classes.
