---
title: Trait System
description: Character customization via permanent advantages/disadvantages selected at creation, stored as quest bits.
keywords: [TTraits, traits, bonus_points, TOG_IS_*, hasQuestBit, character creation]
category: Understanding Systems
related: [character-foundation.md, quest-system.md, experience-leveling.md]
last_updated: 2026-02-01
created_by_model: opus
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
| Cowardice | +10 | `TOG_IS_COWARD` | Auto-flee at 50% HP, wimpy locked | - |
| Blindness | +10 | `TOG_IS_BLIND` | Permanent `AFF_BLIND` | 1 L50 |
| Asthma | +8 | `TOG_IS_ASTHMATIC` | Max movement halved | - |
| Narcolepsy | +8 | `TOG_IS_NARCOLEPTIC` | 1% sleep chance/tick | - |
| Mute | +5 | `TOG_IS_MUTE` | Cannot speak/tell/shout/emote | 1 L50 |
| Combustible | +5 | `TOG_IS_COMBUSTIBLE` | 1% fire damage/tick | - |
| Hemophilia | +5 | `TOG_IS_HEMOPHILIAC` | 2x bleed duration; limb bleeds permanent | - |
| Necrophobia | +5 | `TOG_IS_NECROPHOBIC` | Fear response to corpses/undead | - |
| Alcoholism | +5 | `TOG_IS_ALCOHOLIC` | Only alcohol quenches thirst | - |
| Tourettes | +1 | `TOG_HAS_TOURETTES` | 25% chance/tick to insult | 1 L50 |

### Neutral Traits

| Trait | Pts | Toggle | Effect | Req |
|-------|-----|--------|--------|-----|
| Perma-Death | 0 | `TOG_PERMA_DEATH_CHAR` | Character deleted on death | 1 L50 |
| Real Aging | 0 | `TOG_REAL_AGING` | Age affects gameplay | 1 L50 |
| Fae-Touched | 0 | `TOG_FAE_TOUCHED` | Random bonus stats, 50% XP | 1 L50 same race |

### Benefit Traits (Cost Stat Points)

| Trait | Pts | Toggle | Effect | Req |
|-------|-----|--------|--------|-----|
| Healthy | -8 | `TOG_IS_HEALTHY` | +75% disease immunity | - |
| Nightvision | -8 | `TOG_HAS_NIGHTVISION` | +2 vision bonus | - |
| Ambidextrous | -10 | `TOG_IS_AMBIDEXTROUS` | Equal facility both hands | - |
| Psionics | -100 | `TOG_PSIONICIST` | Innate psionic abilities | 1 L50 |

### Race Restrictions

| Race | Disabled Trait |
|------|----------------|
| Goblin | Blindness |
| Orc | Blindness |

### Connection States

| State | Traits Shown |
|-------|--------------|
| `CON_CREATION_TRAITS1` | 1-6 |
| `CON_CREATION_TRAITS2` | 7-12 |
| `CON_CREATION_TRAITS3` | 13-17 |

### Storage

**Location:** `lib/mutable/player/{first_letter}/{name}.toggle`

**Format:** Space-separated toggle numbers (e.g., `278 282 289`)

## Implementation

### TTraits Structure

Defined in `connect.h`, the `TTraits` struct holds trait metadata:

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

The `num50race` check respects perma-death status: creating a perma-death fae-touched Elf requires an existing L50 perma-death Elf.

### Viewing Traits

- `attribute personal` command displays selected traits via `cmd_attribute.cc`
- `who !` filters fae-touched characters
- `who x` filters perma-death characters

## Troubleshooting

| Symptom | Cause | Fix |
|---------|-------|-----|
| Trait unavailable at creation | Missing L50 requirement | Check `num50race`/`num50any` against account |
| Trait missing after relog | Toggle file not saved | Verify `saveToggles()` called in `storeToSt()` |
| Blindness trait shows `[*]` for Orc | Race restriction | Intended behavior; Goblin/Orc cannot take blindness |
| Combustible instant death | DELETE_THIS not checked | Verify caller checks return from tick processing |
| Fae-touched no stat bonus | Zero L50s of race | Need at least one L50 of same race on account |
| Wrong XP penalty (multiclass) | Multiple reductions | Check `fae_reduction_done` flag set correctly |
