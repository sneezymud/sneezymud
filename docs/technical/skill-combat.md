---
title: Combat Skills Reference
description: Comprehensive reference for SneezyMUD's combat skill system, covering skill definitions, success mechanics, damage calculations, and implementation patterns for physical combat abilities.
keywords:
  - combat skills
  - skill execution
  - bSuccess
  - specialAttack
  - getSkillDam
  - DELETE_VICT
  - reconcileDamage
  - ranged combat
  - unarmed combat
  - TBow
  - TArrow
  - barehand damage
  - lag system
  - spellInfo
  - discArray
category: Critical Systems
related:
  - damage-pipeline.md
  - delete-flags.md
  - combat-formulas.md
  - spell-definitions.md
  - discipline-system.md
  - position-stance.md
  - command-implementation.md
  - weapon-system.md
last_updated: 2026-01-29
source_files:
  - code/code/misc/spell_info.cc
  - code/code/disc/disc_warrior.cc
  - code/code/misc/combat.cc
  - code/code/misc/skill_dam.cc
  - code/code/misc/range.cc
  - code/code/obj/obj_bow.h
  - code/code/obj/obj_bow.cc
  - code/code/obj/obj_arrow.h
  - code/code/obj/obj_arrow.cc
  - code/code/cmd/cmd_kick.cc
  - code/code/cmd/cmd_bash.cc
  - code/code/cmd/cmd_trip.cc
  - code/code/cmd/cmd_headbutt.cc
  - code/code/cmd/cmd_grapple.cc
  - code/code/cmd/cmd_slam.cc
  - code/code/cmd/cmd_disarm.cc
  - code/code/cmd/cmd_stomp.cc
  - code/code/cmd/cmd_deathstroke.cc
  - code/code/disc/disc_monk_mindbody.cc
  - code/code/disc/disc_thief_murder.cc
  - code/code/disc/disc_deikhan_martial.cc
  - code/code/disc/disc_ranger_nature.cc
  - code/code/spec/spec_mobs_archer.cc
  - code/code/obj/obj_base_weapon.cc
  - code/code/misc/enum.h
  - code/code/misc/mobact.cc
---

# Combat Skills Reference

This document provides a comprehensive reference for SneezyMUD's combat skill system, covering skill definitions, success mechanics, damage calculations, and implementation patterns.

## Overview

Combat skills are physical abilities that allow characters to perform special attacks, defensive maneuvers, and tactical actions in combat. Unlike spells, skills focus on physical prowess rather than magical effects and are defined through the same unified framework (`spellInfo` structure) as spells.

**Key characteristics:**
- Skills are organized into class-specific disciplines
- Success determined by two-phase system: skill execution check, then combat effectiveness check
- Damage scales with skill learning, stats, and level
- Lag system prevents skill spam
- Position and stance requirements gate skill availability

## Skill Catalog by Class

### Warrior Skills (DISC_SOLDIERING - 19 skills)

| Skill | Description | Damage Type | Lag |
|-------|-------------|-------------|-----|
| `SKILL_BASH` | Shield bash to knock opponent down | Blunt | LAG_2 |
| `SKILL_BODYSLAM` | Full-body tackle | Blunt | LAG_3 |
| `SKILL_SPIN` | Spinning attack hitting all in room | Pierce/Slash | LAG_4 |
| `SKILL_KICK` | Powerful kick attack | Blunt | LAG_2 |
| `SKILL_HEADBUTT` | Head-to-head collision | Blunt | LAG_2 |
| `SKILL_KNEESTRIKE` | Knee strike to midsection | Blunt | LAG_2 |
| `SKILL_STOMP` | Stomp on prone opponent | Blunt | LAG_2 |
| `SKILL_GRAPPLE` | Wrestling hold and throw | Blunt | LAG_3 |
| `SKILL_DEATHSTROKE` | Finishing blow on wounded foe | Weapon | LAG_3 |
| `SKILL_DISARM` | Knock weapon from opponent's hand | None | LAG_2 |
| `SKILL_RESCUE` | Intervene to protect ally | None | LAG_2 |
| `SKILL_DEFEND` | Defensive stance bonus | None | LAG_0 |
| `SKILL_BERSERK` | Enter berserk rage | None | LAG_0 |
| `SKILL_SWITCH` | Switch combat opponents | None | LAG_1 |
| `SKILL_RIPOSTE` | Counterattack after parry | Weapon | LAG_1 |
| `SKILL_RETREAT` | Tactical withdrawal | None | LAG_2 |
| `SKILL_FOCUSED_ATTACK` | Concentrated strike | Weapon | LAG_2 |
| `SKILL_CHARGE` | Mounted charge attack | Pierce | LAG_3 |
| `SKILL_SLAM` | Shield slam attack | Blunt | LAG_3 |

### Monk Skills (DISC_MINDBODY - 13 skills)

| Skill | Description | Damage Type | Lag |
|-------|-------------|-------------|-----|
| `SKILL_KICK_MONK` | Martial arts kick | Blunt | LAG_2 |
| `SKILL_SPRINGLEAP` | Leaping attack | Blunt | LAG_3 |
| `SKILL_HURL` | Throw opponent | Blunt | LAG_3 |
| `SKILL_DEFENESTRATE` | Throw out window | Blunt | LAG_4 |
| `SKILL_CHI` | Channel inner energy for damage | None | LAG_0 |
| `SKILL_QUIVERING_PALM` | Delayed death touch | None | LAG_4 |
| `SKILL_FEIGN_DEATH` | Fake death to escape | None | LAG_2 |
| `SKILL_IRON_ROOTS` | Root self to resist movement | None | LAG_2 |
| `SKILL_SHOULDER_THROW` | Over-shoulder throw | Blunt | LAG_3 |
| `SKILL_BONEBREAK` | Break opponent's limb | None | LAG_3 |
| `SKILL_MINDBODY` | Mind-body connection passive | None | LAG_0 |
| `SKILL_BLUR` | Movement blur for extra attacks | None | LAG_2 |
| `SKILL_MEDITATE` | Enter meditative state | None | LAG_0 |

### Thief Skills (DISC_MURDER - 6 skills)

| Skill | Description | Damage Type | Lag |
|-------|-------------|-------------|-----|
| `SKILL_BACKSTAB` | Stealth attack from behind | Pierce | LAG_3 |
| `SKILL_THROATSLIT` | Lethal throat cut | Pierce | LAG_4 |
| `SKILL_GARROTTE` | Strangulation with wire | None | LAG_4 |
| `SKILL_STABBING` | Rapid stabbing technique | Pierce | LAG_2 |
| `SKILL_POISONING` | Apply poison to weapon | None | LAG_2 |
| `SKILL_AMBUSH` | Surprise attack from hiding | Pierce | LAG_3 |

### Deikhan Skills (6 skills)

| Skill | Description | Damage Type | Lag |
|-------|-------------|-------------|-----|
| `SKILL_CHARGE` | Mounted lance charge | Pierce | LAG_3 |
| `SKILL_SMITE` | Holy strike | None | LAG_2 |
| `SKILL_SHOCK_CAVALRY` | Mounted shock attack | Blunt | LAG_3 |
| `SKILL_HARM_DEIKHAN` | Righteous harm | None | LAG_2 |
| `SKILL_DEATHSTROKE_DEIKHAN` | Holy finishing blow | Weapon | LAG_3 |
| `SKILL_DEFEND_DEIKHAN` | Defensive martial stance | None | LAG_0 |

### Ranger Skills (1 skill)

| Skill | Description | Damage Type | Lag |
|-------|-------------|-------------|-----|
| `SKILL_TRANSFIX` | Pin opponent with arrow | Pierce | LAG_3 |

## Skill Definition Structure

Skills are defined via `spellInfo` objects in the `discArray` global array, sharing the same framework as spells.

### spellInfo Structure for Skills

```cpp
// Example: SKILL_BASH definition
discArray[SKILL_BASH] = new spellInfo(
    SKILL_WARRIOR,                    // typ: Skill use class
    DISC_SOLDIERING,                  // disc: Primary discipline
    DISC_SOLDIERING,                  // assDisc: Associated discipline
    STAT_STR,                         // modifierStat: Primary stat
    "bash",                           // name: Display name
    TASK_NORMAL,                      // task: Difficulty
    LAG_2,                            // lag: Lag rounds
    POSITION_STANDING,                // minPosition: Min position
    MANA_0, LIFEFORCE_0, PRAY_0,     // No resource cost
    TAR_VIOLENT | TAR_FIGHT_VICT | TAR_CHAR_ROOM, // targets
    SYMBOL_STRESS_0,                 // No holy symbol stress
    "", "", "", "",                  // No fade messages
    START_30, LEARN_2,               // Learning rates
    START_DO_50, LEARN_DO_2,         // Do-learning rates
    START_DO_NO, LEARN_DO_NO,        // No secondary
    LEARN_DIFF_SKILLS,               // Learning difficulty
    0.0,                             // No alignment modifier
    COMP_GESTURAL,                   // Requires gestures
    0                                // No toggles
);
```

**Source:** `code/code/misc/spell_info.cc`

### Key spellInfo Fields for Skills

| Field | Type | Purpose | Typical Values |
|-------|------|---------|----------------|
| `typ` | `skillUseClassT` | Skill class type | `SKILL_WARRIOR`, `SKILL_MONK`, `SKILL_THIEF` |
| `disc` | `discNumT` | Primary discipline | `DISC_SOLDIERING`, `DISC_MINDBODY`, `DISC_MURDER` |
| `assDisc` | `discNumT` | Associated disc (do-learning) | Usually same as `disc` |
| `modifierStat` | `statTypeT` | Primary stat for success | `STAT_STR`, `STAT_DEX`, `STAT_AGI` |
| `task` | `taskDiffT` | Difficulty rating | `TASK_TRIVIAL` through `TASK_IMPOSSIBLE` |
| `lag` | `lag_t` | Lag penalty on use | `LAG_0` through `LAG_9` |
| `minPosition` | `positionTypeT` | Min position to use | `POSITION_STANDING`, `POSITION_FIGHTING` |
| `targets` | `uint32_t` | Target flags | `TAR_VIOLENT`, `TAR_FIGHT_VICT`, `TAR_CHAR_ROOM` |

## Skill Success Mechanics

Skills use a two-phase success system:

1. **Skill Execution Check (`bSuccess()`)**: Can the character execute the technique?
2. **Combat Effectiveness Check (`specialAttack()`)**: Does the technique connect against the target?

### Phase 1: bSuccess() - Skill Execution

Determines if the character successfully executes the skill technique.

**Formula:**
```cpp
limit = getSkillDiffModifier(spell);  // Task difficulty (50-110%)
limit *= skillValue / 100;            // 0-100% learning
limit *= getStatMod(STAT_FOC);        // 0.8-1.25x multiplier
limit *= plotStat(STAT_CURRENT, STAT_KAR, 0.9, 1.125, 1.0);
return ::number(0, 99) < iLimit;
```

**Source:** `code/code/disc/disc_warrior.cc`

**Components:**

| Component | Formula | Effect |
|-----------|---------|--------|
| Task difficulty | `getSkillDiffModifier()` | 110% (trivial) to 35% (impossible) |
| Skill learning | `skillValue / 100` | 0% (untrained) to 100% (mastered) |
| Focus stat | `getStatMod(STAT_FOC)` | 0.8x (low) to 1.25x (high) |
| Karma stat | `plotStat(STAT_KAR, 0.9, 1.125, 1.0)` | 0.9x (low) to 1.125x (high) |

**Task Difficulty Modifiers:**

| Difficulty | Modifier | Usage |
|------------|----------|-------|
| `TASK_TRIVIAL` | 110% | Tutorial skills |
| `TASK_EASY` | 100% | Basic skills |
| `TASK_NORMAL` | 90% | Standard skills (most combat) |
| `TASK_DIFFICULT` | 80% | Advanced techniques |
| `TASK_DANGEROUS` | 70% | High-risk maneuvers |
| `TASK_HOPELESS` | 50% | Desperation moves |
| `TASK_IMPOSSIBLE` | 35% | Near-impossible feats |

### Phase 2: specialAttack() - Combat Effectiveness

Determines if the executed technique successfully affects the target.

**Formula:**
```cpp
mod = attackRound(target) - defendRound(target);  // Attack vs defense
mod += situationalModifiers;  // Position, stats, equipment
roll = random(1, 100);

if (roll <= 50 - mod)
    return GUARANTEED_SUCCESS;
else if (roll < 80 - mod)
    return PARTIAL_SUCCESS;
else if (roll == 100)
    return GUARANTEED_FAILURE;
else
    return FAILURE;
```

**Source:** `code/code/misc/combat.cc`

**Return Values:**

| Return | Value | Meaning |
|--------|-------|---------|
| `GUARANTEED_SUCCESS` | 1000 | Critical hit, maximum effect |
| `COMPLETE_SUCCESS` | 100 | Full success |
| `PARTIAL_SUCCESS` | 50 | Reduced effectiveness |
| `FAILURE` | 0 | Attack fails |
| `GUARANTEED_FAILURE` | -1 | Critical failure |

**Situational Modifiers:**

| Modifier | Value | When Applied |
|----------|-------|--------------|
| Position bonus | -5 to +3 | See Position System section |
| Stat modifier | Varies | DEX, AGI, STR based on skill |
| Level difference | +/-level_diff | Attacker vs defender |
| Combat mode | -10 to +10 | Defense vs Offense mode |

## Damage Calculations

### getSkillDam() - Primary Damage Formula

**Formula:**
```cpp
baseDamage = classAmt x lagRounds x level;
damage = baseDamage x diffModifier x statModifier x randomVariance;
```

**Source:** `code/code/misc/skill_dam.cc`

**Components:**

| Component | Description | Calculation |
|-----------|-------------|-------------|
| `classAmt` | Skill power rating | Fixed per skill (see table below) |
| `lagRounds` | Lag penalty converted to rounds | `lagValue / 12` |
| `level` | Character level | Current class level |
| `diffModifier` | Advanced learning bonus | 1.0 + (`advLearning / 200`) |
| `statModifier` | Primary stat bonus | `getStatMod(modifierStat)` |
| `randomVariance` | Random spread | +/-(`level / 2`) |

**classAmt Values (Selected Skills):**

| Skill | classAmt | Relative Power |
|-------|----------|----------------|
| `SKILL_KICK` | 0.5-0.75 | Low |
| `SKILL_BASH` | 0.5-0.75 | Low |
| `SKILL_HEADBUTT` | 0.75-1.0 | Medium |
| `SKILL_GRAPPLE` | 1.0-1.25 | Medium |
| `SKILL_BODYSLAM` | 1.25-1.5 | High |
| `SKILL_DEATHSTROKE` | 2.0-2.5 | Very High |
| `SKILL_BACKSTAB` | 2.0-3.0 | Very High |
| `SKILL_THROATSLIT` | 3.0-4.0 | Lethal |

### NPC Damage Reduction

MOBs deal approximately 52% of PC damage for the same skill:

```cpp
if (isMob())
    damage *= 0.5195;
```

This prevents NPCs from overwhelming players with identical skills.

### Location-Based Damage (Kick)

Kick skill has location-specific multipliers:

| Location | Multiplier | Description |
|----------|------------|-------------|
| Head | 2.5x | Maximum damage |
| Solar plexus | 2.0x | High damage |
| Shin | 1.5x | Moderate damage |
| Side | 1.0x | Base damage |

**Source:** `code/code/disc/disc_warrior.cc`

## Ranged Combat

Ranged combat allows characters to attack targets in distant rooms using bows, crossbows, and thrown weapons. The system involves projectile trajectory, range limits, and specialized skills.

### Ranged Combat Skills

| Skill | Enum | Description |
|-------|------|-------------|
| `SKILL_RANGED_PROF` | 569 | Basic ranged competency (all classes) |
| `SKILL_RANGED_SPEC` | 568 | Advanced ranged mastery (Warrior/Ranger) |
| `SKILL_FAST_LOAD` | - | Faster arrow reload speed |

**Source:** `code/code/misc/spell_info.cc`

### TBow Class

The `TBow` class represents ranged weapons (bows, crossbows, slings, blowguns).

**Key Properties:**

| Property | Type | Description |
|----------|------|-------------|
| `arrowType` | `int` | Ammunition type this bow accepts |
| `flags` | `unsigned int` | Bow state flags (broken string, etc.) |
| `max_range` | `unsigned int` | Maximum shooting distance in rooms |

**Bow Flags:**

| Flag | Meaning |
|------|---------|
| `BOW_STRING_BROKE` | Bowstring snapped, needs repair |
| `BOW_CARVED` | Bow has been carved (crafting) |
| `BOW_SCRAPED` | Bow has been scraped (crafting) |
| `BOW_SMOOTHED` | Bow has been smoothed (crafting) |

**Source:** `code/code/obj/obj_bow.h`, `code/code/obj/obj_bow.cc`

### TArrow Class

The `TArrow` class represents ammunition (arrows, quarrels, darts, sling stones).

**Key Properties:**

| Property | Type | Description |
|----------|------|-------------|
| `arrowType` | `unsigned char` | Arrow size/type (must match bow) |
| `arrowHead` | `unsigned char` | Head type (affects damage) |
| `arrowHeadMat` | `unsigned int` | Head material |
| `arrowFlags` | `unsigned int` | Arrow state flags |
| `trap_level` | `unsigned int` | Trap damage level (if trapped) |
| `trap_dam_type` | `doorTrapT` | Trap damage type |

**Arrow Types:**

| Type | Value | Description |
|------|-------|-------------|
| Hunting arrow | 0 | Standard bow arrow |
| Fighting arrow | 1 | Combat bow arrow |
| Squabble quarrel | 2 | Light crossbow bolt |
| Common quarrel | 3 | Standard crossbow bolt |
| Sniper blowdart | 4 | Precision blowgun dart |
| Common blowdart | 5 | Standard blowgun dart |
| Heavy sling ammo | 6 | Large sling stone |
| Common sling ammo | 7 | Standard sling stone |

**Source:** `code/code/obj/obj_arrow.h`, `code/code/obj/obj_arrow.cc`

### Shooting Mechanics

**Shoot Command Flow:**

```cpp
int TBeing::doShoot(const char* arg) {
    // 1. Validate shooter has bow equipped
    if (!(t = equipment[getPrimaryHold()]))
        return FALSE;

    // 2. Check for SKILL_RANGED_PROF (minimum 10)
    if (getSkillValue(SKILL_RANGED_PROF) <= 0)
        return FALSE;

    // 3. Find target in specified direction
    targ = get_char_vis_direction(this, arg1, dir, iDist, TRUE, &count);

    // 4. Delegate to bow's shootMeBow()
    rc = t->shootMeBow(this, targ, count, dir, iDist);
}
```

**Source:** `code/code/misc/range.cc`

**Bow Shooting Logic:**

```cpp
int TBow::shootMeBow(TBeing* ch, TBeing* targ, unsigned int count,
                     dirTypeT dir, int shoot_dist) {
    // 1. Verify bow is loaded
    if (stuff.empty() || !dynamic_cast<TArrow*>(stuff.front()))
        return FALSE;

    // 2. Check range limits
    max_distance = getMaxRange();
    if (shoot_dist > max_distance)
        return FALSE;

    // 3. Skill checks for bow handling
    if (ch->getSkillValue(SKILL_RANGED_PROF) < 10) {
        // Arrow falls harmlessly
        return FALSE;
    }

    // 4. Bowstring break check
    if (!(str_test = ::number(0, getStructPoints()))) {
        // String snaps or bow shatters
        addBowFlags(BOW_STRING_BROKE);
        return FALSE;
    }

    // 5. Calculate number of arrows per round
    float nattacks = 1.0;
    nattacks += max(0.0, getSkillValue(SKILL_FAST_LOAD) / 100.0);
    nattacks += max(0.0, getSkillValue(SKILL_RANGED_SPEC) / 100.0);

    // 6. Fire arrows
    while (nattacks > 0 && targ) {
        rc = throwThing(the_arrow, dir, ch->in_room, &targ,
                        shoot_dist, max_distance, ch);
        // Auto-reload if arrows available
        the_arrow = ch->findArrow(buf, SILENT_NO);
        if (the_arrow)
            the_arrow->bloadBowArrow(ch, this);
    }

    ch->setWait(combatRound(4));  // 4-round lag
}
```

**Source:** `code/code/obj/obj_bow.cc`

### Ranged Damage Calculation

Ranged damage is calculated via `get_range_actual_damage()`:

```cpp
int get_range_actual_damage(TBeing* ch, TBeing* victim, TObj* o,
                            int dam, spellNumT attacktype) {
    // Base damage from arrow's damageLevel()
    dam = victim->skipImmortals(dam);

    // Apply skill modifiers
    if (dynamic_cast<TArrow*>(o)) {
        int q = 100;
        q += (ch->getSkillValue(SKILL_RANGED_PROF) / 2);  // +0-50%
        q += (ch->getSkillValue(SKILL_RANGED_SPEC) / 2);  // +0-50%
        dam *= q;
        dam /= 100;
    } else {
        // Thrown objects use only proficiency
        q = ch->getSkillValue(SKILL_RANGED_PROF);
        dam *= q;
        dam /= 100;
    }
    return max(dam, 1);
}
```

**Source:** `code/code/misc/range.cc`

**Damage Scaling:**

| Skill Level | RANGED_PROF | RANGED_SPEC | Total Modifier |
|-------------|-------------|-------------|----------------|
| 0 | 50% | 50% | 100% (base) |
| 50 | 75% | 75% | 150% |
| 100 | 100% | 100% | 200% (max) |

### Arrow Hit Resolution

When an arrow reaches a target, `TBaseWeapon::catchSmack()` handles the hit:

```cpp
int TBaseWeapon::catchSmack(TBeing* ch, TBeing** targ, TRoom* rp,
                            int cdist, int mdist) {
    // 1. Determine damage type
    if (dynamic_cast<TArrow*>(this))
        damtype = DAMAGE_ARROWS;  // or TYPE_SHOOT/TYPE_CANNON

    // 2. Check if target dodges (specialAttack vs SKILL_RANGED_PROF)
    if (!(i = ch->specialAttack(tb, SKILL_RANGED_PROF)) ||
        i == GUARANTEED_FAILURE) {
        // Target dodges
        return FALSE;
    }

    // 3. Check for arrow embedding (based on sharpness)
    if (!isBluntWeapon() && !tb->getStuckIn(phit) &&
        ::number(1, 100) < getCurSharp()) {
        rc = tb->stickIn(this, phit);  // Arrow sticks in victim
    }

    // 4. Calculate and apply damage
    int d = (int)damageLevel();
    d = get_range_actual_damage(ch, tb, this, d, damtype);

    // 5. Apply poison if arrow is poisoned
    if (isPoisoned())
        applyPoison(tb);

    // 6. Trigger arrow traps
    if (arrow->getTrapDamType() != DOOR_TRAP_NONE) {
        tb->triggerArrowTrap(arrow);
    }

    if (ch->reconcileDamage(tb, d, damtype) == -1)
        return DELETE_VICT;
}
```

**Source:** `code/code/obj/obj_base_weapon.cc`

### Throwing Mechanics

Objects with `ITEM_WEAR_THROW` can be thrown using physics-based trajectory:

```cpp
int TThing::throwMe(TBeing* ch, dirTypeT tdir, const char* vict) {
    // Physics calculation for throw distance
    float acc = ch->plotStat(STAT_CURRENT, STAT_BRA, 500.0, 5000.0, 2500.0);
    acc /= max(3.0f, getWeight());

    float v0 = 0.2 * acc;  // Initial velocity

    // Trajectory angle based on focus stat
    int ang = ch->plotStat(STAT_CURRENT, STAT_FOC, 0, 45, 5);

    // Calculate max range
    float mult = ((v0 * v0) / 32.0) * sin(2 * ang * (2.0 * PI / 360.0));
    max_distance = (unsigned int)(mult / (ch->outside() ? 100 : 50));
    max_distance = max(max_distance, 1u);

    rc = throwThing(this, tdir, ch->in_room, &targ, iDist, max_distance, ch);
}
```

**Source:** `code/code/misc/range.cc`

### Archer MOB Special Procedure

MOBs with the `archer` spec proc automatically shoot at targets:

```cpp
int archer(TBeing*, cmdTypeT cmd, const char*, TMonster* ch, TObj*) {
    // 1. Find and equip bow with matching ammo
    std::vector<TBow*> bows = ch->getBows();
    for (auto bow : bows) {
        if ((ammo = ch->autoGetAmmo(bow)))
            break;
    }

    // 2. Flee from melee if in same room as target
    if (ch->Hates(tbt, NULL))
        ch->doFlee("");

    // 3. Scan for targets up to MAX_RANGE (3 rooms)
    for (i = MIN_DIR; i <= MAX_DIR - 1; i++) {
        for (range = 1; range <= MAX_RANGE; range++) {
            // Find hated PCs in line of sight
        }
    }

    // 4. Load and shoot
    ammo->bloadBowArrow(ch, bow);
    ch->doShoot(buf.c_str());
}
```

**Source:** `code/code/spec/spec_mobs_archer.cc`

## Unarmed Combat

Unarmed combat uses the barehand damage system, with monks receiving significant bonuses through the `SKILL_KUBO` skill tree.

### Unarmed Combat Skills

| Skill | Enum | Class | Description |
|-------|------|-------|-------------|
| `SKILL_BAREHAND_PROF` | 563 | All | Basic unarmed competency |
| `SKILL_BAREHAND_SPEC` | 567 | Monk | Advanced unarmed mastery |
| `SKILL_KUBO` | - | Monk | Monk barehand damage scaling |
| `SKILL_IRON_FIST` | - | Monk | Bonus damage with ungloved hands |
| `SKILL_VOPLAT` | - | Monk | Additional damage modifier |

**Source:** `code/code/misc/spell_info.cc`

### Barehand Damage Formula

Non-monk characters deal minimal barehand damage:

```cpp
// Standard barehand (non-monk)
wepDam = ::number(1, 3);
```

Monks use the enhanced `getMonkWeaponDam()` function:

```cpp
static int getMonkWeaponDam(const TBeing* ch, const TBeing* v,
                            primaryTypeT isprimary, int rollDam) {
    if (!ch->doesKnowSkill(SKILL_KUBO))
        wepDam = ::number(1, 2);
    else {
        // Scale damage based on SKILL_KUBO learning
        double value = 3.0 * double(ch->getSkillValue(SKILL_KUBO)) / 10.0;
        value = min(max(value, 0.0), 50.0);

        // Convert to damage (sqrt scaling)
        float weapDam = (6.0 * sqrt(value) / 2.0);

        // Apply balance correction for level
        weapDam *= balanceCorrectionForLevel(ch->GetMaxLevel());
        wepDam = (int)weapDam;

        // Random variance (+/- 10%)
        int flux = wepDam / 10;
        wepDam = ::number(wepDam - flux, wepDam + flux);
        wepDam = max(1, wepDam);
    }

    // Stat modifiers
    float statDam = ch->getStrDamModifier();

    // SKILL_IRON_FIST bonus (requires ungloved hands)
    if (ch->doesKnowSkill(SKILL_IRON_FIST) &&
        !ch->equipment[WEAR_HAND_R] && !ch->equipment[WEAR_HAND_L]) {
        statDam += (ch->getSkillValue(SKILL_IRON_FIST) / 1200.0);  // 0-8.3%
    }

    // SKILL_VOPLAT bonus
    if (ch->doesKnowSkill(SKILL_VOPLAT)) {
        statDam += (ch->getSkillValue(SKILL_VOPLAT) / 1000.0);  // 0-10%
    }

    int dam = (int)(wepDam * statDam);

    // Apply global barehand modifier
    dam = (int)(dam * stats.barehand_damage_mod);  // Default: 0.36

    return dam;
}
```

**Source:** `code/code/misc/combat.cc`

**Monk Damage Scaling by SKILL_KUBO:**

| KUBO Skill | Value | Base Damage |
|------------|-------|-------------|
| 0 | 0.0 | 1-2 |
| 33 | 10.0 | ~9 |
| 66 | 20.0 | ~13 |
| 100 | 30.0 | ~16 |

### Global Damage Modifiers

The game applies global modifiers to all barehand damage:

```cpp
// From statistics.cc
stats.barehand_damage_mod = 0.36;  // 36% of calculated damage
```

**Source:** `code/code/misc/statistics.cc`

This can be adjusted at runtime via the `stats` command.

### Monk Attack Frequency

Monks receive bonus attacks when fighting unarmed:

```cpp
if (isPc()) {
    num = getMult();  // Base monk attacks

    if (!prim && hasClass(CLASS_MONK)) {
        fx += (0.60 * num);  // 60% of attacks to primary hand
    }
}
```

**Source:** `code/code/misc/combat.cc` (attack calculation)

### Stomp Skill

The stomp skill allows attacking prone opponents:

```cpp
bool TBeing::canStomp(TBeing* victim, silentTypeT silent) {
    // Target must have feet on ground
    if (victim->getPartMinHeight(ITEM_WEAR_FEET) > 0)
        return FALSE;

    // Target cannot be flying
    if (victim->isFlying())
        return FALSE;

    // Stomper needs legs and cannot be mounted
    if (!hasLegs() || eitherLegHurt() || riding)
        return FALSE;
}
```

**Stomp Damage Calculation:**

```cpp
int TBeing::stompHit(TBeing* victim) {
    int dam = getSkillDam(victim, SKILL_STOMP, getSkillLevel(SKILL_STOMP),
                          getAdvLearning(SKILL_STOMP));

    if (victim->getPosition() == POSITION_STANDING) {
        // Can stomp head if much taller
        if (height >= 5 * victim->getPosHeight()) {
            targetLimb = WEAR_HEAD;  // Full damage
        } else {
            targetLimb = WEAR_FOOT_L;  // Toes
            dam /= 5;  // 20% damage
        }
    } else {
        // Victim prone - stomp body
        targetLimb = WEAR_BACK;  // or WEAR_BODY
        // Full damage
    }

    if (reconcileDamage(victim, dam, SKILL_STOMP) == -1)
        return DELETE_VICT;
}
```

**Source:** `code/code/cmd/cmd_stomp.cc`

**Stomp Requirements:**

| Requirement | Check |
|-------------|-------|
| Know skill | `doesKnowSkill(SKILL_STOMP)` |
| Have legs | `hasLegs()` |
| Legs not hurt | `!eitherLegHurt()` |
| Not mounted | `!riding` |
| Target grounded | `victim->getPartMinHeight(ITEM_WEAR_FEET) == 0` |
| Target not flying | `!victim->isFlying()` |

## Lag System

### Lag Enumeration

Skills impose a lag penalty measured in game pulses (0.1 seconds each):

| Constant | Pulses | Real Time | Usage |
|----------|--------|-----------|-------|
| `LAG_0` | 0 | 0.0 sec | Instant skills |
| `LAG_1` | 12 | 1.2 sec | Very fast |
| `LAG_2` | 24 | 2.4 sec | Standard combat |
| `LAG_3` | 36 | 3.6 sec | Powerful attacks |
| `LAG_4` | 48 | 4.8 sec | Very powerful |
| `LAG_5` | 60 | 6.0 sec | Devastating |
| `LAG_9` | 108 | 10.8 sec | Ultimate abilities |

**Source:** `code/code/misc/enum.h`

### Lag Application

```cpp
void TBeing::addSkillLag(spellNumT skill, int success) {
    if (!success)
        return;  // No lag on failure

    lag_t lagValue = discArray[skill]->lag;

    // Death-triggered lag reduction
    if (IS_SET_DELETE(success, DELETE_VICT))
        lagValue = min(lagValue, LAG_1);

    // Convert to pulses
    int floatLag = lagAdjust(lagValue) * combatRound(1);

    // Apply lag
    setCharFighting(floatLag);
}
```

**Death-Triggered Reduction:** If the skill kills the target (`DELETE_VICT` set), lag is capped at `LAG_1` to allow faster cleanup of remaining enemies.

**Source:** `code/code/disc/disc_warrior.cc`

## Position and Stance Requirements

### Position Requirements

Most combat skills require `POSITION_STANDING`:

```cpp
if (getPosition() < POSITION_STANDING) {
    sendTo("You must be standing to use this skill!\n\r");
    return FALSE;
}
```

**Exceptions:**
- `SKILL_FEIGN_DEATH` - Can be used from any position
- `SKILL_STOMP` - Target must be sitting/resting/sleeping
- `SKILL_CHI` - Can be used while sitting (meditation)

### Combat Mode (Stance) Effects

Combat modes affect skill availability and damage:

| Mode | Skill Availability | Damage Modifier |
|------|-------------------|-----------------|
| `ATTACK_NORMAL` | All skills | 1.0x |
| `ATTACK_DEFENSE` | Defensive skills preferred | 0.9x |
| `ATTACK_OFFENSE` | Offensive skills preferred | 1.1x |
| `ATTACK_BERSERK` | Limited skill selection | 1.2x |

### Berserk Mode Restrictions

While berserking, only specific skills are available with weighted selection:

```cpp
// Berserk skill weights
weights[SKILL_BASH] = 2;
weights[SKILL_HEADBUTT] = 3;
weights[SKILL_BODYSLAM] = 2;
weights[SKILL_GRAPPLE] = 1;
weights[SKILL_SLAM] = 3;
weights[SKILL_DEATHSTROKE] = 1;
```

Higher weights = more likely to be selected.

**Source:** `code/code/misc/mobact.cc`

## Common Implementation Patterns

### Three-Tier Target Resolution

Combat commands follow a consistent fallback pattern:

```cpp
int TBeing::doKick(const char* argument, TBeing* vict) {
    TBeing* victim;

    // Tier 1: Use explicit parameter if provided (programmatic call)
    if (!(victim = vict)) {
        // Tier 2: Parse argument to find target by name
        if (!(victim = get_char_room_vis(this, argument))) {
            // Tier 3: Default to current fight target
            if (!(victim = fight())) {
                sendTo("Kick whom?\n\r");
                return FALSE;
            }
        }
    }

    // Execute with resolved target
    rc = kick(this, victim, SKILL_KICK);

    // Handle DELETE_VICT with ownership check
    if (IS_SET_DELETE(rc, DELETE_VICT)) {
        if (vict)
            return rc;  // Caller owns victim
        delete victim;
        victim = nullptr;
        REM_DELETE(rc, DELETE_VICT);
    }

    return rc;
}
```

**Source:** `code/code/cmd/cmd_kick.cc`

### Pre-Execution Validation

All skills use `canXXX()` validation functions:

```cpp
bool TBeing::canKick(TBeing* victim, silentTypeT silent) {
    // Check basic combat conditions
    if (UtilMobProc(this))
        return FALSE;

    // Position check
    if (getPosition() < POSITION_STANDING) {
        if (silent == SILENT_NO)
            sendTo("You must be standing!\n\r");
        return FALSE;
    }

    // Target validation
    if (!sameRoom(*victim)) {
        if (silent == SILENT_NO)
            sendTo("That person isn't here!\n\r");
        return FALSE;
    }

    // Equipment check
    if (!hasLegs()) {
        if (silent == SILENT_NO)
            sendTo("You need legs to kick!\n\r");
        return FALSE;
    }

    return TRUE;
}
```

**Common checks:**
- Position requirements
- Target presence
- Equipment requirements (hands, legs, weapons)
- Resource costs (move, mana)
- Combat state

### Success/Fail Branching

Skills branch into separate success and fail handlers:

```cpp
static int kick(TBeing* ch, TBeing* victim, spellNumT skill) {
    // Pre-execution validation
    if (!ch->canKick(victim, SILENT_NO))
        return FALSE;

    // Resource consumption
    ch->addToMove(-KICK_MOVE);

    // Phase 1: Skill execution check
    int bKnown = ch->getSkillValue(skill);
    if (!ch->bSuccess(bKnown, skill)) {
        return ch->kickFail(victim, skill);
    }

    // Phase 2: Combat effectiveness check
    int successfulHit = ch->specialAttack(victim, skill);
    if (successfulHit != GUARANTEED_FAILURE) {
        return ch->kickSuccess(victim, skill, successfulHit);
    } else {
        return ch->kickFail(victim, skill);
    }
}
```

### Success Handler Pattern

```cpp
int TBeing::kickSuccess(TBeing* victim, spellNumT skill, int successLevel) {
    // Calculate damage
    int dam = getSkillDam(victim, skill, level, advLearning);

    // Apply success level modifier
    if (successLevel == PARTIAL_SUCCESS)
        dam /= 2;

    // Location-based multiplier (kick-specific)
    int location = ::number(0, 3);
    float multiplier = kickLocationMultiplier[location];
    dam *= multiplier;

    // Messages
    act("You kick $N!", FALSE, this, 0, victim, TO_CHAR);
    act("$n kicks you!", FALSE, this, 0, victim, TO_VICT);
    act("$n kicks $N!", FALSE, this, 0, victim, TO_NOTVICT);

    // Apply damage - CRITICAL: check for -1 death return
    if (reconcileDamage(victim, dam, skill) == -1)
        return DELETE_VICT;

    return TRUE;
}
```

**CRITICAL:** `reconcileDamage()` returns `-1` when victim dies, NOT `DELETE_VICT`. Always check with `== -1`.

**Source:** See [Damage Pipeline](damage-pipeline.md) for complete documentation.

### Fail Handler Pattern

```cpp
int TBeing::kickFail(TBeing* victim, spellNumT skill) {
    // Failure messages
    act("You miss your kick at $N!", FALSE, this, 0, victim, TO_CHAR);
    act("$n misses a kick at you!", FALSE, this, 0, victim, TO_VICT);
    act("$n misses a kick at $N!", FALSE, this, 0, victim, TO_NOTVICT);

    // May have side effects (fall, balance loss, etc.)
    int rc = crashLanding(POSITION_SITTING);
    if (IS_SET_DELETE(rc, DELETE_THIS))
        return DELETE_THIS;

    // Zero-damage reconcile to start combat
    reconcileDamage(victim, 0, skill);

    return TRUE;  // Still apply lag on failure
}
```

## DELETE Flag Handling Patterns

### Standard DELETE_VICT Pattern

```cpp
int TBeing::doXXX(const char* argument, TBeing* vict) {
    TBeing* victim;
    int rc;

    // ... target resolution ...

    rc = xxx(this, victim, SKILL_XXX);

    // Add lag on success
    if (rc)
        addSkillLag(SKILL_XXX, rc);

    // Handle DELETE_VICT with ownership check
    if (IS_SET_DELETE(rc, DELETE_VICT)) {
        if (vict)
            return rc;  // Caller owns victim, return flag
        delete victim;
        victim = nullptr;
        REM_DELETE(rc, DELETE_VICT);  // Clear flag before returning
    }

    // Propagate DELETE_THIS
    if (IS_SET_DELETE(rc, DELETE_THIS))
        return DELETE_THIS;

    return rc;
}
```

**Critical Rules:**
1. **Check ownership:** If `vict` parameter was provided, caller owns the victim pointer
2. **Return the flag:** Let caller handle deletion when they own the pointer
3. **Delete locally:** Only delete if we resolved the victim ourselves
4. **Clear the flag:** Use `REM_DELETE()` after local deletion
5. **Always use `IS_SET_DELETE()`:** Never use `IS_SET()` for DELETE flags

**Source:** See [DELETE Flag System](delete-flags.md) for complete documentation.

### Both Combatants May Die

Some skills (like `SKILL_GRAPPLE`) can result in mutual death:

```cpp
rc = victim->trySpringleap(this);

if (IS_SET_DELETE(rc, DELETE_THIS) && IS_SET_DELETE(rc, DELETE_VICT)) {
    // Both die - return combined flags
    return rc;
} else if (IS_SET_DELETE(rc, DELETE_THIS)) {
    // Their THIS is our VICT (translate flag)
    return DELETE_VICT;
} else if (IS_SET_DELETE(rc, DELETE_VICT)) {
    // Their VICT is our THIS (translate flag)
    return DELETE_THIS;
}
```

**Context matters:** When calling `victim->method(this)`, the roles are reversed. Translate flags accordingly.

### Critical: reconcileDamage() Returns -1, Not DELETE_VICT

```cpp
// CORRECT: Check for -1
if (reconcileDamage(victim, dam, skill) == -1)
    return DELETE_VICT;

// WRONG: IS_SET_DELETE won't detect -1
int rc = reconcileDamage(victim, dam, skill);
if (IS_SET_DELETE(rc, DELETE_VICT)) { ... }  // NEVER TRIGGERS!
```

This is because `reconcileDamage()` returns actual damage dealt on success (0+) and uses `-1` as a magic sentinel value for death.

## Source File Organization

### Command Files (`code/code/cmd/cmd_*.cc`)

Each skill typically has its own command file:

| File | Skills |
|------|--------|
| `cmd_kick.cc` | `doKick`, `kick`, `kickSuccess`, `kickFail` |
| `cmd_bash.cc` | `doBash`, `bash`, `bashSuccess`, `bashFail` |
| `cmd_trip.cc` | `doTrip`, `trip`, `tripSuccess`, `tripFail` |
| `cmd_headbutt.cc` | `doHeadbutt`, `headbutt`, `headbuttHit`, `headbuttMiss` |
| `cmd_grapple.cc` | `doGrapple`, `grapple` |
| `cmd_slam.cc` | `doSlam`, `slamSuccess`, `slamFail` |
| `cmd_disarm.cc` | `doDisarm`, `disarm` |
| `cmd_stomp.cc` | `doStomp`, `stomp`, `stompHit`, `stompMiss` |
| `cmd_deathstroke.cc` | `doDeathstroke`, `deathstrokeSuccess`, `deathstrokeFail` |

### Discipline Files (`code/code/disc/disc_*.cc`)

Class-specific skills grouped by discipline:

| File | Discipline | Skills |
|------|------------|--------|
| `disc_warrior.cc` | DISC_SOLDIERING | Bash, bodyslam, spin, etc. |
| `disc_monk_mindbody.cc` | DISC_MINDBODY | Kick, springleap, hurl, etc. |
| `disc_thief_murder.cc` | DISC_MURDER | Backstab, throatslit, garrotte |
| `disc_deikhan_martial.cc` | DISC_MARTIAL | Charge, smite, shock cavalry |
| `disc_ranger_nature.cc` | DISC_NATURE | Transfix |

### Ranged Combat Files

| File | Purpose |
|------|---------|
| `code/code/misc/range.cc` | Core ranged mechanics, throwing, scanning |
| `code/code/obj/obj_bow.cc` | TBow class implementation |
| `code/code/obj/obj_bow.h` | TBow class definition |
| `code/code/obj/obj_arrow.cc` | TArrow class implementation |
| `code/code/obj/obj_arrow.h` | TArrow class definition |
| `code/code/spec/spec_mobs_archer.cc` | Archer MOB AI |

## Key Constants Reference

### skillUseClassT Enum

```cpp
enum skillUseClassT {
    SKILL_UNDEFINED = 0,
    SKILL_MAGE,
    SKILL_CLERIC,
    SKILL_WARRIOR,
    SKILL_THIEF,
    SKILL_MONK,
    SKILL_DEIKHAN,
    SKILL_RANGER,
    SKILL_SHAMAN
};
```

### taskDiffT Enum

```cpp
enum taskDiffT {
    TASK_TRIVIAL = 0,     // 110% success modifier
    TASK_EASY,            // 100%
    TASK_NORMAL,          // 90%
    TASK_DIFFICULT,       // 80%
    TASK_DANGEROUS,       // 70%
    TASK_HOPELESS,        // 50%
    TASK_IMPOSSIBLE       // 35%
};
```

### lag_t Enum

```cpp
enum lag_t {
    LAG_0 = 0,     // 0 pulses (0.0 sec)
    LAG_1 = 12,    // 12 pulses (1.2 sec)
    LAG_2 = 24,    // 24 pulses (2.4 sec)
    LAG_3 = 36,    // 36 pulses (3.6 sec)
    LAG_4 = 48,    // 48 pulses (4.8 sec)
    LAG_5 = 60,    // 60 pulses (6.0 sec)
    LAG_9 = 108    // 108 pulses (10.8 sec)
};
```

## Related Documentation

- [Damage Pipeline](damage-pipeline.md) - How damage flows through reconcileDamage/applyDamage
- [DELETE Flag System](delete-flags.md) - Memory management signaling
- [Combat Formulas](combat-formulas.md) - Hit probability and damage calculations
- [Spell Definitions](spell-definitions.md) - Spell system parallel structure
- [Discipline System](discipline-system.md) - Skill organization and learning
- [Position Stance](position-stance.md) - Position effects on combat
- [Command Implementation](command-implementation.md) - Command dispatch patterns
- [Weapon System](weapon-system.md) - Weapon proficiencies and specializations

## Common Pitfalls

### 1. Wrong Death Check

```cpp
// WRONG: Checking for DELETE_VICT flag
int rc = reconcileDamage(victim, dam, skill);
if (IS_SET_DELETE(rc, DELETE_VICT)) { ... }  // Never triggers!

// CORRECT: Check for -1
if (reconcileDamage(victim, dam, skill) == -1)
    return DELETE_VICT;
```

### 2. Forgetting Ownership Check

```cpp
// WRONG: Always deletes
if (IS_SET_DELETE(rc, DELETE_VICT)) {
    delete victim;  // Caller may still reference!
}

// CORRECT: Check ownership
if (IS_SET_DELETE(rc, DELETE_VICT)) {
    if (vict)
        return rc;  // Let caller delete
    delete victim;
    REM_DELETE(rc, DELETE_VICT);
}
```

### 3. Using IS_SET Instead of IS_SET_DELETE

```cpp
// WRONG: Wrong bitmask check
if (IS_SET(rc, DELETE_THIS)) { ... }

// CORRECT: DELETE flags use special bitmask
if (IS_SET_DELETE(rc, DELETE_THIS)) { ... }
```

### 4. Not Clearing Flags After Local Deletion

```cpp
// WRONG: Flag still set
delete victim;
return rc;  // Caller may try to delete again!

// CORRECT: Clear flag
delete victim;
REM_DELETE(rc, DELETE_VICT);
return rc;
```

### 5. Ignoring Return Values

```cpp
// WRONG: Continuing after potential death
rc = someSkill(victim);
victim->doSomething();  // victim may be dead!

// CORRECT: Check immediately
rc = someSkill(victim);
if (IS_SET_DELETE(rc, DELETE_VICT))
    return DELETE_VICT;
victim->doSomething();  // Safe
```

### 6. Ranged Combat: Not Checking Skill Minimum

```cpp
// WRONG: Allowing unskilled shooting
rc = t->shootMeBow(this, targ, count, dir, iDist);

// CORRECT: Check minimum skill
if (getSkillValue(SKILL_RANGED_PROF) < 10) {
    sendTo("You don't know how to shoot!\n\r");
    return FALSE;
}
```

### 7. Unarmed Combat: Using Wrong Damage Function

```cpp
// WRONG: Using weapon damage for monk barehand
wepDam = ::number(1, 3);

// CORRECT: Check for monk skills
if (doesKnowSkill(SKILL_KUBO))
    return getMonkWeaponDam(this, v, isprimary, rollDam);
```

## Summary

The combat skill system provides a robust framework for physical combat abilities:

1. **Unified with spells:** Skills use the same `spellInfo` structure as spells
2. **Two-phase success:** `bSuccess()` checks skill execution, `specialAttack()` checks combat effectiveness
3. **Scalable damage:** `getSkillDam()` scales with level, learning, and stats
4. **Lag system:** Prevents skill spam with pulse-based delays
5. **Position gating:** Most skills require `POSITION_STANDING`
6. **Memory safety:** DELETE flag system prevents crashes when targets die
7. **Consistent patterns:** Three-tier resolution, canXXX validation, success/fail branching
8. **Ranged combat:** Physics-based projectile system with bow/arrow classes
9. **Unarmed combat:** Monk specialization with SKILL_KUBO damage scaling

**Key safety rules:**
- Always use `IS_SET_DELETE()`, never `IS_SET()`
- Check `reconcileDamage() == -1` for death, NOT `DELETE_VICT`
- Check ownership (`vict` parameter) before deleting
- Clear flags with `REM_DELETE()` after local deletion
- Propagate `DELETE_THIS` immediately

This documentation provides a comprehensive reference for understanding, implementing, and debugging combat skills in SneezyMUD. The combat skill system is built on solid memory safety patterns (DELETE flags) and proven success mechanics (bSuccess/specialAttack separation) that should be followed in all new skill implementations.
