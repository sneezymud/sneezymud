---
title: Trait System
description: Character customization via permanent advantages/disadvantages selected at creation, stored as quest bits.
keywords: [traits, bonus_points, character creation, advantages, disadvantages]
category: informational
primary_symbols:
  functions: [hasQuestBit, setQuestBit, remQuestBit, nannyTraits_input, saveToggles, loadToggles, numFifties]
  classes: [TTraits, TBeing, TPerson]
  enums: [MAX_TRAITS, TOG_IS_COWARD, TOG_IS_BLIND, TOG_IS_ASTHMATIC, TOG_IS_NARCOLEPTIC, TOG_IS_MUTE, TOG_IS_COMBUSTIBLE, TOG_IS_HEMOPHILIAC, TOG_IS_NECROPHOBIC, TOG_IS_ALCOHOLIC, TOG_HAS_TOURETTES, TOG_PERMA_DEATH_CHAR, TOG_REAL_AGING, TOG_FAE_TOUCHED, TOG_IS_HEALTHY, TOG_HAS_NIGHTVISION, TOG_IS_AMBIDEXTROUS, TOG_PSIONICIST, TOG_IS_VICIOUS, TOG_IS_CRAVEN, CON_CREATION_TRAITS1, CON_CREATION_TRAITS2, CON_CREATION_TRAITS3]
---

## Overview

The trait system allows players to customize characters during creation by selecting permanent advantages and disadvantages. Positive traits cost stat points; negative traits grant bonus points for stats. Traits are stored as quest bits and cannot be changed after creation.

17 available traits span three categories: penalties (grant +1 to +10 points), neutral (0 points), and benefits (cost -8 to -100 points). Some traits require existing level 50 characters on the account. Certain races disable specific traits.

## Patterns

**Selection Validation**
- Always check `num50race` and `num50any` requirements before allowing trait selection
- Always verify race does not disable the trait via `TPlayerRace::disableTrait`
- Never allow negative bonus points in any category at creation completion

**Runtime Checks**
- Always use `hasQuestBit(TOG_*)` to check trait presence
- Never cache trait state; always query quest bits directly
- Always check DELETE_THIS after trait effects that deal damage (combustible, necrophobia)

**Storage Integrity**
- Never change toggle numbers (278-299 range); they are stored in player files
- Always call `saveToggles()` during character save via `storeToSt()`

## Reference

### Penalty Traits (Grant Bonus Points)

| Trait | Pts | Toggle | Effect | Req |
|-------|-----|--------|--------|-----|
| Cowardice | +10 | `TOG_IS_COWARD` (278) | Auto-flee at 50% HP, wimpy locked | - |
| Blindness | +10 | `TOG_IS_BLIND` (279) | Permanent `AFF_BLIND` | 1 L50 |
| Asthma | +8 | `TOG_IS_ASTHMATIC` (282) | Max movement halved | - |
| Narcolepsy | +8 | `TOG_IS_NARCOLEPTIC` (284) | 1% sleep chance/tick | - |
| Mute | +5 | `TOG_IS_MUTE` (280) | Cannot speak/tell/shout/whisper/ask/order/emote | 1 L50 |
| Combustible | +5 | `TOG_IS_COMBUSTIBLE` (285) | 1% fire damage/tick | - |
| Hemophilia | +5 | `TOG_IS_HEMOPHILIAC` (286) | 2x bleed duration; limb bleeds permanent | - |
| Necrophobia | +5 | `TOG_IS_NECROPHOBIC` (283) | Fear response to corpses/undead | - |
| Alcoholism | +5 | `TOG_IS_ALCOHOLIC` (290) | Only alcohol quenches thirst | - |
| Tourettes | +1 | `TOG_HAS_TOURETTES` (291) | 25% chance/tick to insult | 1 L50 |

### Neutral Traits

| Trait | Pts | Toggle | Effect | Req |
|-------|-----|--------|--------|-----|
| Perma-Death | 0 | `TOG_PERMA_DEATH_CHAR` (247) | Character deleted on death | 1 L50 |
| Real Aging | 0 | `TOG_REAL_AGING` (299) | Age affects gameplay | 1 L50 |
| Fae-Touched | 0 | `TOG_FAE_TOUCHED` (298) | Random bonus stats, 50% XP | 1 L50 same race |

### Benefit Traits (Cost Stat Points)

| Trait | Pts | Toggle | Effect | Req |
|-------|-----|--------|--------|-----|
| Healthy | -8 | `TOG_IS_HEALTHY` (288) | +75% disease immunity | - |
| Nightvision | -8 | `TOG_HAS_NIGHTVISION` (289) | +2 vision bonus | - |
| Ambidextrous | -10 | `TOG_IS_AMBIDEXTROUS` (287) | Equal facility both hands | - |
| Psionics | -100 | `TOG_PSIONICIST` (248) | Innate psionic abilities | 1 L50 |

### Race Restrictions

| Race | Disabled Trait |
|------|----------------|
| Goblin | Cowardice |
| Orc | Cowardice |

### Connection States

| State | Traits Shown |
|-------|--------------|
| `CON_CREATION_TRAITS1` | 1-6 |
| `CON_CREATION_TRAITS2` | 7-12 |
| `CON_CREATION_TRAITS3` | 13-17 |

### Display Notation

| Symbol | Meaning |
|--------|---------|
| `[X]` | Selected |
| `[*]` | Unavailable (race restriction or missing L50 req) |
| `[ ]` | Available |

### Storage

**Location:** `lib/mutable/player/{first_letter}/{name}.toggle`

**Format:** Space-separated toggle numbers (e.g., `278 282 289`)

## Implementation

### TTraits Structure

Defined in `connect.h` with `MAX_TRAITS` constant set to 17. The `TTraits` struct holds trait metadata:

| Field | Purpose |
|-------|---------|
| `tog` | Toggle constant |
| `points` | Point cost (negative=benefit, positive=penalty) |
| `name` | Display name |
| `desc` | In-game description |
| `num50race` | Required L50 characters of same race |
| `num50any` | Required L50 characters of any race |

The `traits[]` array in `create_character.cc` defines all 17 traits ordered by point value.

### Trait Effect Locations

| Trait | Implementation File | Mechanism |
|-------|---------------------|-----------|
| Cowardice | `toggle.cc`, `connect.cc` | Sets wimpy to `maxWimpy()` on login |
| Blindness | `player_data.cc` | Sets `AFF_BLIND` on load |
| Asthma | `limits.cc` | Halves max movement in `maxMove()` |
| Narcolepsy | `periodic.cc` | 1% tick check, applies `AFFECT_DUMMY` sleep |
| Mute | `talk.cc` | Blocks speech commands (immortal tell exempt) |
| Combustible | `periodic.cc` | 1% tick check, calls `flameEngulfed()` |
| Hemophilia | `magicutils.cc` | Doubles bleed duration via lambda |
| Necrophobia | `periodic.cc` | 25% tick scan for corpses/undead |
| Alcoholism | `obj_food.cc` | Non-alcohol stops at 3 thirst units |
| Tourettes | `periodic.cc` | 25% tick insult to random visible being |
| Healthy | `immunity.cc` | +75 to `IMMUNE_DISEASE` |
| Nightvision | `player_data.cc` | +2 to vision bonus calculation |
| Ambidextrous | `being.h` | `isAmbidextrous()` returns true |
| Fae-Touched | `create_character.cc`, `limits.cc` | Stat bonus at creation, XP halved |

### Point Distribution

Bonus points divide into four stat categories: `combat`, `combat2`, `learn`, `util`. Division is `total/4` per category with remainder distributed round-robin. Creation cannot complete with negative points in any category.

### Fae-Touched Details

At creation, grants `50 + (num_fifties - 1) * 2` random stat points where `num_fifties` is capped at 26. The XP penalty applies once per kill regardless of multiclass (tracked via `fae_reduction_done` flag).

The `num50race` check respects perma-death status: creating a perma-death fae-touched Elf requires an existing L50 perma-death Elf. For non-perma characters, only non-perma L50s count.

### Selection Workflow

The `nannyTraits_input` function handles trait toggling during character creation. Input parsing uses `convertTo<int>` to extract the trait index. When a player selects a trait number:

1. If already selected: calls `remQuestBit` and subtracts point value
2. If not selected: validates L50 requirements via `numFifties`, then calls `setQuestBit` and adds point value

Race restrictions are checked during display generation via `TPlayerRace::disableTrait`.

### Persistence

The `saveToggles` function in `rent.cc` iterates through all `MAX_TOG_INDEX` values and writes each set quest bit to the toggle file. The `loadToggles` function reads numbers back and calls `setQuestBit` for each.

Error handling is minimal: failed file opens are silently ignored in `loadToggles`, treating missing toggle files as having no quest bits set.

### Viewing Traits

- `attribute personal` command displays selected traits via `cmd_attribute.cc`
- `who !` filters fae-touched characters
- `who x` filters perma-death characters

## Troubleshooting

| Symptom | Cause | Fix |
|---------|-------|-----|
| Trait unavailable at creation | Missing L50 requirement | Check `num50race`/`num50any` against account |
| Trait missing after relog | Toggle file not saved | Verify `saveToggles()` called in `storeToSt()` |
| Cowardice trait shows `[*]` for Orc/Goblin | Race restriction | Intended behavior; Goblin/Orc cannot take cowardice |
| Combustible instant death | DELETE_THIS not checked | Verify caller checks return from tick processing |
| Fae-touched no stat bonus | Zero L50s of race | Need at least one L50 of same race on account |
| Wrong XP penalty (multiclass) | Multiple reductions | Check `fae_reduction_done` flag set correctly |
| Trait effect not working | Toggle constant mismatch | Verify `hasQuestBit` uses correct constant from traits array |
| Periodic trait not firing | Character not ticking | Sleeping/resting may not trigger periodic checks |
| Traits lost on save | Disk/permission error | Check write permissions on `lib/mutable/player/` |
| Trait toggle exceeds MAX_TOG_INDEX | Constant too small | Ensure MAX_TOG_INDEX includes all trait toggle numbers |
