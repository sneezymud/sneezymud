---
title: Vital Statistics
description: Hunger, thirst, and age systems that add survival elements and character progression through lifecycle stages.
created_by_model: opus
related: [character-foundation.md, race-system.md, scheduler-pulses.md]
---

# Vital Statistics

## Overview

SneezyMUD tracks five condition types: drunkenness, hunger (fullness), thirst, and waste products (bladder/bowel). These drain over time based on terrain and activity, with starvation causing HP loss and eventual death. Character age is opt-in via a quest bit and modifies stats along physical/mental curves: youth favors strength and agility while age favors wisdom and intelligence. Vampires are exempt from all age effects.

## Patterns

### Condition Handling

- Always check for -1 (immunity) before comparing conditions to 0
- Never treat condition -1 as "very hungry" - it indicates immortal immunity
- Always account for racial food modifiers when calculating nutrition gain
- Always check `isVampire()` when debugging age-related stat issues
- Never assume auto-eat activates during combat - check task state first

### Age System

- Always verify `TOG_REAL_AGING` quest bit before applying age modifiers
- Use `graf()` for age-interpolated values - it handles the opt-in check internally
- Remember physical stats (STR, AGI, CON) decline with age while mental stats (INT, WIS) improve
- Never apply age penalties to vampires - they have flat bonuses instead

### Starvation Safety

- Check both current and previous tick values before applying starvation damage
- Safe zones (Imperia, Lethargica) are immune to hunger/thirst drain
- Starvation uses `DAMAGE_STARVATION` death type for logging

## Reference

### Condition Types

| Condition | Index | Range | Notes |
|-----------|-------|-------|-------|
| DRUNK | 0 | 0-24, -1=immune | Intoxication level |
| FULL | 1 | 0-24, -1=immune | Hunger satiation |
| THIRST | 2 | 0-24, -1=immune | Hydration level |
| PEE | 3 | 0-24 | Bladder pressure |
| POOP | 4 | 0-24 | Bowel pressure |

### Condition Thresholds

| Value | State |
|-------|-------|
| -1 | Immune (immortals) |
| 0 | Critical (taking damage) |
| 1-2 | Warning messages |
| 20+ | Fully satiated |

### Racial Food Talents

| Talent | Effect |
|--------|--------|
| TALENT_FISHEATER | 2x nutrition from fish, 5% from other foods |
| TALENT_MEATEATER | 2x nutrition from butchered meat, 5% from other foods |
| TALENT_INSECT_EATER | Can only eat insects |
| TALENT_GARBAGEEATER | Can eat organic trash, accelerated hunger drain |

### MUD Time Conversions

| Unit | Real Time |
|------|-----------|
| 1 MUD hour | ~2.4 minutes |
| 1 MUD day | ~57.6 minutes |
| 1 MUD month | ~26.9 hours |
| 1 MUD year | ~13.4 days |

### Age Stat Modifiers (Physical - Decline)

| Human-Equivalent Age | STR/AGI/DEX/CON/SPE | BRA |
|---------------------|---------------------|-----|
| <17 | +10 | +5 |
| 17-30 | +1 to +9 | +1 to +4 |
| 31-60 | 0 | 0 |
| 61-80 | -1 to -9 | -1 to -4 |
| 81+ | -10 | -5 |

### Age Stat Modifiers (Mental - Improve)

| Human-Equivalent Age | INT/WIS/FOC/PER |
|---------------------|-----------------|
| <17 | -10 |
| 17-30 | -9 to -1 |
| 31-60 | 0 |
| 61-80 | +1 to +9 |
| 81+ | +10 |

### Age Stat Modifiers (Special)

| Human-Equivalent Age | KAR | CHA |
|---------------------|-----|-----|
| <17 | +10 | 0 |
| 17-60 | Linear decay to 0 | 0 |
| 61+ | Linear decay to -10 | 0 |

## Implementation

### Condition Storage

Conditions persist in `TBeing::specials.conditions[]` and save to `charFile.conditions[]`. The `setCond()` accessor automatically forces -1 for characters above `MAX_MORT` level, making immortals immune. Access via `getCond(condTypeT)` and `setCond(condTypeT, short)`.

### Nutrition Drain Flow

The `foodNDrink()` function executes once per half-tick (~36 seconds) via `updateHalfTickStuff()`. Drain probability depends on terrain-specific hunger/thirst/drunk values from `TerrainInfo`. Higher terrain values mean slower drain. Desert and jungle sectors drain faster. When FULL decreases, POOP increases; when THIRST decreases, PEE increases.

### Condition Gain Modifiers

The `gainCondition()` function applies three modifier layers for positive gains:
1. **Racial metabolism**: `Race::getFoodMod()` and `Race::getDrinkMod()`
2. **Body mass**: Normalized to 180 lbs baseline - heavier characters need proportionally more
3. **Alcoholism skill**: Reduces drunkenness gain by up to ~5%

All gains clamp to 0-24 range after modifiers.

### Starvation Mechanics

When FULL or THIRST remains at 0 for consecutive ticks (not in safe zones):
1. Warning message displayed
2. 1 HP lost per half-tick
3. Death via `die(DAMAGE_STARVATION)` at -10 HP

Regeneration penalties while hungry/thirsty: mana and move gain divided by 4, HP regeneration blocked (only gains 1 HP if STUNNED).

### Auto-Eat System

When `AUTO_EAT` autobit is set and character is awake, reaching condition 0 triggers automatic consumption. Searches equipment and inventory for food closest to spoiling. Low-level players (<=3) at Center Square can pray to the statue instead. Does not activate during active tasks.

### Garbage Eater Penalty

Characters with `TALENT_GARBAGEEATER` experience accelerated hunger drain when FULL < 19. At FULL 0: first drains movement points, then HP if movement exhausted.

### Weight Tracking

The `calcNutrition()` function maintains a running calorie balance:
- FULL <= 5: -1 (weight loss trend)
- FULL >= 6/10/20: +1/+2/+3 (weight gain trend)
- Low movement (exercise): -2 per threshold crossed

When balance exceeds +/- 5000, weight changes by 1 lb (clamped to racial min/max).

### Age Data Structure

Age stored in `charFile` with three fields:
- `birth`: Real-world timestamp of character creation
- `base_age`: Starting age at creation (from race's `age X+YdZ` formula)
- `age_mod`: Manual adjustments

Current age calculated via `mudTimePassed()` combining birth timestamp with elapsed game time.

### Age Effects

The `graf()` interpolation function handles all age-based value lookups, returning the age-35 constant when `TOG_REAL_AGING` is not set. The `age_mod_for_stat()` function converts character years to "human equivalent" before applying stat modifiers.

Movement costs increase with age: +1 move per 5 years over 30, plus additional +1 per 10 years over 50. Maximum move pool increases with age to partially compensate. Ranged combat suffers -3 range penalty above human-equivalent age 80.

### Key Files

| File | Purpose |
|------|---------|
| `code/code/misc/enum.h` | `condTypeT` enum |
| `code/code/misc/limits.cc` | `gainCondition()`, `graf()`, `getMaxMove()` |
| `code/code/misc/periodic.cc` | Nutrition calculation, starvation damage |
| `code/code/obj/obj_food.cc` | `foodNDrink()`, condition accessors |
| `code/code/misc/stats.cc` | `age_mod_for_stat()` |
| `code/code/sys/gametime.cc` | MUD time calculation |
| `code/code/misc/race.cc` | Racial age generation |
| `code/code/misc/charfile.h` | Age data storage |
| `code/code/misc/movement.cc` | Age-based movement costs |

## Troubleshooting

### Character Starving Despite Eating

**Symptom:** HP loss continues after eating food.

**Cause:** Racial talent restricts valid food types (TALENT_FISHEATER only gains nutrition from fish).

**Fix:** Verify food type matches racial dietary requirements. Check `getMyRace()->hasTalent()` for food-related talents.

---

### Regeneration Extremely Slow

**Symptom:** Mana and movement regenerate at 1/4 rate.

**Cause:** FULL or THIRST at 0.

**Fix:** Check `getCond(FULL)` and `getCond(THIRST)`. Address hunger/thirst even if not taking starvation damage.

---

### Age Modifiers Not Applying

**Symptom:** Old character has no stat penalties.

**Cause:** `TOG_REAL_AGING` quest bit not set (opt-in system).

**Fix:** Verify `hasQuestBit(TOG_REAL_AGING)` returns true. This is set during character creation.

---

### Immortal Reporting Hunger

**Symptom:** Character above MAX_MORT shows hunger warnings.

**Cause:** Condition set before immortality granted.

**Fix:** Call `setCond()` again after level change - it auto-sets -1 for immortals.

---

### Vampire Showing Age Penalties

**Symptom:** Vampire character has age-based stat modifiers.

**Cause:** Missing `isVampire()` check in code path.

**Fix:** All age modifier code should bypass vampires. Verify the specific stat calculation checks vampire status.

---

### Auto-Eat Not Triggering

**Symptom:** Character starves despite food in inventory and AUTO_EAT enabled.

**Cause:** Character in task state (combat, crafting) or not awake.

**Fix:** Check `task` field and `awake()` status. Auto-eat only fires when both conditions pass.

---

### Weight Not Changing

**Symptom:** Character consistently over/undereating but weight static.

**Cause:** Weight already at racial min/max limit.

**Fix:** Check `getMyRace()->getMinWeight(getSex())` and `getMaxWeight()`. Weight clamps to these bounds.
