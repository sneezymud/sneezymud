---
title: Position and Stance System
description: Describes the position and attack mode (stance) systems governing character states and combat behavior, including position types, command gating, combat bonuses, and movement costs.
keywords:
  - positionTypeT
  - POSITION_SLEEPING
  - POSITION_RESTING
  - POSITION_SITTING
  - POSITION_STANDING
  - POSITION_FIGHTING
  - attack_mode_t
  - ATTACK_BERSERK
  - updatePos
  - attackRound
  - defendRound
  - specAttackMod
  - getCombatMode
  - setCombatMode
  - SKILL_GROUNDFIGHTING
category: Important Systems
related:
  - rest-recovery.md
  - combat-formulas.md
  - task-system.md
last_updated: 2026-01-29
source_files:
  - code/code/misc/enum.h
  - code/code/misc/parse.h
  - code/code/misc/parse.cc
  - code/code/misc/combat.cc
  - code/code/misc/movement.cc
  - code/code/misc/being.h
  - code/code/misc/being.cc
  - code/code/misc/constants.cc
---

# Position and Stance System

This document describes the position and attack mode (stance) systems in SneezyMUD, which govern character states and combat behavior.

## Position System

Positions represent the physical state of a being and determine what actions are available. The position hierarchy is ordered from lowest (most incapacitated) to highest (most mobile).

### Position Types (`positionTypeT`)

| Value | Position | Description |
|-------|----------|-------------|
| 0 | `POSITION_DEAD` | HP <= -11; character is dead |
| 1 | `POSITION_MORTALLYW` | HP -6 to -10; mortally wounded, bleeding out |
| 2 | `POSITION_INCAP` | HP -3 to -5; incapacitated |
| 3 | `POSITION_STUNNED` | HP 0 or less; stunned, unable to act |
| 4 | `POSITION_SLEEPING` | Asleep; regenerates faster |
| 5 | `POSITION_RESTING` | Resting; moderate regeneration |
| 6 | `POSITION_SITTING` | Sitting; minimal regeneration bonus |
| 7 | `POSITION_ENGAGED` | In combat but not actively fighting |
| 8 | `POSITION_FIGHTING` | Actively fighting |
| 9 | `POSITION_CRAWLING` | Crawling on the ground |
| 10 | `POSITION_STANDING` | Normal standing position |
| 11 | `POSITION_MOUNTED` | Riding a mount |
| 12 | `POSITION_FLYING` | Flying through the air |

**Source:** `code/code/misc/enum.h` (lines 184-198)

### Position Categories

**Incapacitated States (0-3):** Character cannot act voluntarily. HP determines position automatically via `updatePos()`.

**Rest States (4-6):** Character chose to rest. Enhanced regeneration but limited actions.

**Combat States (7-8):** Character is engaged with an enemy. Limited non-combat actions.

**Mobile States (9-12):** Character can move and act normally, with varying combat bonuses/penalties.

## Command Gating by Position

Commands define a minimum position requirement via `commandInfo::minPosition`. The command parser checks this before execution.

```cpp
class commandInfo {
    const char* name;
    positionTypeT minPosition;  // Minimum position to execute
    int minLevel;
};
```

**Source:** `code/code/misc/parse.h` (lines 24-35)

### Position Check Flow

1. Parser retrieves `commandArray[cmd]->minPosition`
2. Compares against `getPosition()`
3. If position too low, sends appropriate error message:
   - "You cannot do that while dead!"
   - "You cannot do that while mortally wounded!"
   - "You cannot do that while stunned!"
   - "You cannot do that while sleeping!"
   - etc.

**Source:** `code/code/misc/parse.cc` (lines 281-320)

### Special Position Restrictions

- Commands with `minPosition >= POSITION_CRAWLING` cannot be used while fighting
- Paralysis (`AFF_PARALYSIS`) blocks commands requiring `> POSITION_STUNNED`
- Stunned (`AFF_STUNNED`) blocks commands requiring `> POSITION_STUNNED`

## Position Updates (`updatePos()`)

The `updatePos()` function automatically adjusts position based on HP thresholds.

### HP-to-Position Mapping

| HP Range | Position Set |
|----------|--------------|
| <= -11 | `POSITION_DEAD` |
| -6 to -10 | `POSITION_MORTALLYW` |
| -3 to -5 | `POSITION_INCAP` |
| <= 0 | `POSITION_STUNNED` |
| > 0 | No automatic change (retains current) |

### Recovery Behavior

When healed above 0 HP:
- From `POSITION_STUNNED`: Character sits up automatically
- From `POSITION_MORTALLYW`/`POSITION_INCAP`: Transitions to `POSITION_STUNNED` first
- If mounted: Position set to `POSITION_MOUNTED`
- If paralyzed: Stays at `POSITION_STUNNED`

**Source:** `code/code/misc/combat.cc` (lines 311-365)

## Combat Bonuses and Penalties by Position

Position significantly affects combat effectiveness through `attackRound()` and `defendRound()` calculations.

### Attack Round Modifiers

| Position | Modifier |
|----------|----------|
| Dead/Mortally/Incap/Stunned/Sleeping | Negates all attack bonus |
| Resting | -(level/3 + 1) |
| Sitting | -(level/4 + 1) |
| Engaged/Fighting/Crawling/Standing | No modifier |
| Mounted | +(level/4 + 1) |
| Flying | +(level/3 + 1) |

### Defense Round Modifiers

Identical scaling to attack modifiers. Both directions benefit equally from superior positioning.

### Ground Fighting Mitigation

Characters with `SKILL_GROUNDFIGHTING` reduce penalties when position < `POSITION_STANDING`:
```cpp
val = val * (100 - getSkillValue(SKILL_GROUNDFIGHTING)) / MAX_SKILL_LEARNEDNESS;
```

**Source:** `code/code/misc/combat.cc` (lines 2718-2760, 2920-2964)

### Special Attack Modifiers (`specAttackMod`)

| Position | Modifier |
|----------|----------|
| Resting | -5 |
| Sitting | -3 |
| Crawling | -1 |
| Standing/Engaged/Fighting | 0 |
| Mounted | +2 |
| Flying | +3 |

**Source:** `code/code/misc/combat.cc` (lines 2971-2997)

## Movement Costs by Position

Position affects movement point consumption during travel.

| Condition | Movement Cost Effect |
|-----------|---------------------|
| Flying | Divide by 4 (minimum 1) |
| Levitating | Divide by 4 (minimum 5) |
| Crawling (both legs hurt) | +20 base, +20 if one arm hurt |
| Haste/Accelerate effects | Divide by 2 |

**Source:** `code/code/misc/movement.cc` (lines 561-683)

## Attack Modes (`attack_mode_t`)

Attack modes (stances) modify combat behavior, affecting attack frequency and defense.

### Mode Types

| Value | Mode | Description |
|-------|------|-------------|
| 0 | `ATTACK_NORMAL` | Balanced attack and defense |
| 1 | `ATTACK_DEFENSE` | Defensive stance, reduced attacks |
| 2 | `ATTACK_OFFENSE` | Aggressive stance, more attacks |
| 3 | `ATTACK_BERSERK` | Berserking, maximum offense |

**Source:** `code/code/misc/enum.h` (lines 134-140)

### Mode Effects

**Defense Mode:**
- Reduces attack opportunities
- Increases defensive bonuses

**Offense Mode:**
- Increases attack opportunities
- Reduces defensive bonuses

**Berserk Mode:**
- Maximum attack frequency
- Cannot flee or use most commands
- Automatically exits when combat ends
- Blocks portal entry while fighting

### Mode Management

```cpp
attack_mode_t getCombatMode() const;
bool isCombatMode(attack_mode_t n) const;
void setCombatMode(attack_mode_t n);
```

**Source:** `code/code/misc/being.h` (lines 509-510, 593)

## Spell and Skill Position Requirements

Spells and disciplines also define minimum position requirements via `discArray[which]->minPosition`.

### Position Check in Casting

```cpp
if (getPosition() < discArray[which]->minPosition) {
    // Send position-specific error message
    return FALSE;
}
```

### Flying Restrictions

Some spells cannot be cast while flying or in flying sectors:
- Earth-based spells (Earthmaw, etc.)
- Certain nature abilities (Camp, Forage)
- Feign Death

Some skills cannot target flying creatures:
- Hurl
- Shoulder Throw
- Defenestrate
- Bone Break

## Key Functions Reference

| Function | File | Purpose |
|----------|------|---------|
| `getPosition()` | being.h | Returns current position |
| `setPosition()` | being.h | Sets position directly |
| `updatePos()` | combat.cc | Auto-adjusts position by HP |
| `attackRound()` | combat.cc | Calculates attack bonuses |
| `defendRound()` | combat.cc | Calculates defense bonuses |
| `specAttackMod()` | combat.cc | Special attack modifiers |
| `getCombatMode()` | being.h | Returns current stance |
| `setCombatMode()` | being.h | Changes combat stance |
| `isFlying()` | being.cc | Checks if position is FLYING |

## Position Display Names

The `position_types[]` array provides human-readable names:

```cpp
const sstring position_types[] = {
    "Dead", "Mortally wounded", "Incapacitated",
    "Stunned", "Sleeping", "Resting", "Sitting",
    "Engaged", "Fighting", "Crawling", "Standing",
    "Mounted", "Flying", "\n"
};
```

**Source:** `code/code/misc/constants.cc` (lines 708-710)
