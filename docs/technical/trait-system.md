---
title: Trait System
description: Character trait system allowing players to customize characters during creation by selecting advantages and disadvantages in exchange for bonus stat points, stored as permanent quest bits.
keywords:
  - TTraits
  - traits
  - TOG_IS_COWARD
  - TOG_IS_BLIND
  - TOG_HAS_NIGHTVISION
  - TOG_PERMA_DEATH_CHAR
  - TOG_FAE_TOUCHED
  - hasQuestBit
  - bonus_points
  - CON_CREATION_TRAITS
  - saveToggles
  - loadToggles
  - character creation
  - permanent attributes
category: Understanding Systems

  - race-system.md
  - stats-attributes.md
  - experience-leveling.md
  - quest-system.md
  - charfile-format.md
last_updated: 2026-01-29
source_files:
  - code/code/sys/create_character.cc
  - code/code/sys/connect.h
  - code/code/misc/toggle.h
  - code/code/misc/rent.cc
  - code/code/misc/toggle.cc
  - code/code/misc/player_data.cc
  - code/code/misc/limits.cc
  - code/code/misc/periodic.cc
  - code/code/misc/talk.cc
  - code/code/misc/magicutils.cc
  - code/code/obj/obj_food.cc
  - code/code/misc/immunity.cc
  - code/code/misc/being.h
  - code/code/cmd/cmd_attribute.cc
  - code/code/cmd/cmd_who.cc
related: [character-foundation.md]
---

This document describes SneezyMUD's character trait system, which allows players to customize their characters during creation by selecting advantages and disadvantages in exchange for bonus stat points.

## Overview

Character traits are permanent attributes selected during character creation that modify gameplay mechanics. Positive traits (benefits) cost stat points, while negative traits (penalties) grant bonus stat points that can be spent on character statistics.

**Design Pattern:** Traits are stored as quest bits (toggles) and checked at runtime via `hasQuestBit()`. The trait system reuses the existing quest bit infrastructure rather than maintaining separate storage.

**Key characteristics:**
- 17 available traits (`MAX_TRAITS`)
- Points range from -100 (expensive benefit) to +10 (major penalty)
- Some traits require level 50 characters on the account
- Certain races disable specific traits
- Traits are permanent and cannot be changed after creation

**Source files:**
- `code/code/sys/create_character.cc` - Trait definitions and selection UI
- `code/code/sys/connect.h` - `TTraits` struct and `MAX_TRAITS` constant
- `code/code/misc/toggle.h` - Toggle constants (`TOG_IS_*`, `TOG_HAS_*`)
- `code/code/misc/rent.cc` - Trait persistence (`saveToggles`, `loadToggles`)

## TTraits Structure

**Source:** `/code/code/sys/connect.h` (lines 38-44)

```cpp
const int MAX_TRAITS = 17;

struct TTraits {
    int tog, points;
    sstring name, desc;
    int num50race, num50any;
};
```

| Field | Type | Description |
|-------|------|-------------|
| `tog` | `int` | Toggle constant (e.g., `TOG_IS_COWARD`) |
| `points` | `int` | Point cost (negative = benefit, positive = penalty) |
| `name` | `sstring` | Display name (e.g., "cowardice") |
| `desc` | `sstring` | In-game description |
| `num50race` | `int` | Required L50 characters of same race |
| `num50any` | `int` | Required L50 characters of any race |

## Available Traits

**Source:** `/code/code/sys/create_character.cc` (lines 28-65)

Traits are ordered by point value for display purposes.

### Penalty Traits (Grant Bonus Points)

These traits impose gameplay disadvantages in exchange for bonus stat points.

| Trait | Points | Toggle | Effect | Requirements |
|-------|--------|--------|--------|--------------|
| Cowardice | +10 | `TOG_IS_COWARD` | Auto-flee at 50% HP, cannot change wimpy | None |
| Blindness | +10 | `TOG_IS_BLIND` | Permanent `AFF_BLIND` | 1 L50 any |
| Asthma | +8 | `TOG_IS_ASTHMATIC` | Max movement halved | None |
| Narcolepsy | +8 | `TOG_IS_NARCOLEPTIC` | Random sleep episodes (1% per tick) | None |
| Mute | +5 | `TOG_IS_MUTE` | Cannot speak, tell, shout, or emote | 1 L50 any |
| Combustible | +5 | `TOG_IS_COMBUSTIBLE` | Random spontaneous combustion (1% per tick) | None |
| Hemophilia | +5 | `TOG_IS_HEMOPHILIAC` | Bleeding duration doubled; limb bleeds permanent | None |
| Necrophobia | +5 | `TOG_IS_NECROPHOBIC` | Fear response to corpses/undead | None |
| Alcoholism | +5 | `TOG_IS_ALCOHOLIC` | Thirst only quenched by alcohol | None |
| Tourettes | +1 | `TOG_HAS_TOURETTES` | Random involuntary insults | 1 L50 any |

### Neutral Traits (No Point Cost)

| Trait | Points | Toggle | Effect | Requirements |
|-------|--------|--------|--------|--------------|
| Perma-Death | 0 | `TOG_PERMA_DEATH_CHAR` | Character deleted on death | 1 L50 any |
| Real Aging | 0 | `TOG_REAL_AGING` | Age affects gameplay over time | 1 L50 any |
| Fae-Touched | 0 | `TOG_FAE_TOUCHED` | Random bonus stats, 50% XP gain | 1 L50 same race |

### Benefit Traits (Cost Stat Points)

These traits provide advantages but reduce available stat points.

| Trait | Points | Toggle | Effect | Requirements |
|-------|--------|--------|--------|--------------|
| Healthy | -8 | `TOG_IS_HEALTHY` | +75% disease immunity | None |
| Nightvision | -8 | `TOG_HAS_NIGHTVISION` | +2 vision bonus | None |
| Ambidextrous | -10 | `TOG_IS_AMBIDEXTROUS` | Equal facility with both hands | None |
| Psionics | -100 | `TOG_PSIONICIST` | Innate psionic abilities | 1 L50 any |

## Trait Selection During Creation

### Connection States

Trait selection spans three connection states to accommodate the trait list:

**Source:** `/code/code/sys/connect.h` (lines 98-100)

```cpp
CON_CREATION_TRAITS1,  // Traits 1-6
CON_CREATION_TRAITS2,  // Traits 7-12
CON_CREATION_TRAITS3,  // Traits 13-17
```

### Selection UI

**Source:** `/code/code/sys/create_character.cc` (lines 858-932)

The trait selection screen displays:
- Trait name and point value
- Description of effect
- Selection marker: `[X]` = selected, `[*]` = unavailable, `[ ]` = available
- Current bonus point total

```
[X]  1. cowardice   ( 10 points)
        You flee combat if you get below 1/2 hit points.
[*]  2. blindness   ( 10 points)
        Your vision has been damaged and you are permanently blind.
[ ]  3. asthma      (  8 points)
        You have asthma and thus are easily winded.
...
Bonus points          [ 10]
```

### Selection Logic

**Source:** `/code/code/sys/create_character.cc` (lines 904-932)

```cpp
connectStateT nannyTraits_input(Descriptor* desc, sstring& output,
  const sstring input) {
  int num50race = numFifties(desc->character->getRace(),
    desc->character->hasQuestBit(TOG_PERMA_DEATH_CHAR), desc->account->name);
  int num50any = numFifties(RACE_NORACE, false, desc->account->name);
  int iTrait = convertTo<int>(input);

  if (desc->character->hasQuestBit(traits[iTrait].tog)) {
    // Toggle off - remove trait and subtract points
    desc->character->remQuestBit(traits[iTrait].tog);
    desc->bonus_points.total -= traits[iTrait].points;
  } else if (traits[iTrait].num50race > num50race) {
    // Not enough L50s of this race
    output = "Invalid Choice! N L50 characters needed for trait (race-specific).";
  } else if (traits[iTrait].num50any > num50any) {
    // Not enough L50s of any race
    output = "Invalid Choice! N L50 characters needed for trait.";
  } else {
    // Toggle on - add trait and add points
    desc->character->setQuestBit(traits[iTrait].tog);
    desc->bonus_points.total += traits[iTrait].points;
  }
  return desc->connected;
}
```

### Race Restrictions

Certain races disable specific traits via `TPlayerRace::disableTrait`:

**Source:** `/code/code/sys/create_character.cc` (lines 130-147)

| Race | Disabled Trait Index |
|------|---------------------|
| Goblin | 1 (Blindness) |
| Orc | 1 (Blindness) |

## Trait Effects Implementation

### Cowardice

**Source:** `/code/code/misc/toggle.cc` (lines 818-821), `/code/code/sys/connect.cc` (lines 1324-1327)

- Sets wimpy to `maxWimpy()` on login
- Prevents changing wimpy setting
- Auto-flees when HP drops below 50%

```cpp
if (character->hasQuestBit(TOG_IS_COWARD)) {
    character->wimpy = character->maxWimpy();
}
```

### Blindness

**Source:** `/code/code/misc/player_data.cc` (lines 124-126)

- Permanently sets `AFF_BLIND` affect flag on character load

```cpp
if (hasQuestBit(TOG_IS_BLIND)) {
    SET_BIT(specials.affectedBy, AFF_BLIND);
}
```

### Asthma

**Source:** `/code/code/misc/limits.cc` (lines 194-195)

- Halves maximum movement points

```cpp
if (hasQuestBit(TOG_IS_ASTHMATIC))
    iMax /= 2;
```

### Narcolepsy

**Source:** `/code/code/misc/periodic.cc` (lines 947-964)

- 1% chance per tick to fall asleep
- Applies `AFFECT_DUMMY` with `POSITION_SLEEPING`

```cpp
if (hasQuestBit(TOG_IS_NARCOLEPTIC) && awake() && !::number(0, 99)) {
    affectedData af;
    af.type = AFFECT_DUMMY;
    // ... sets up sleep affect
    setPosition(POSITION_SLEEPING);
}
```

### Mute

**Source:** `/code/code/misc/talk.cc` (multiple locations)

- Blocks: `say`, `shout`, `tell`, `whisper`, `ask`, `order`, `emote`
- Exception: can tell to immortals

```cpp
if (hasQuestBit(TOG_IS_MUTE)) {
    sendTo("You're mute, you can't talk.\n\r");
    return FALSE;
}
```

### Combustible

**Source:** `/code/code/misc/periodic.cc` (lines 941-945)

- 1% chance per tick to catch fire
- Calls `flameEngulfed()` which deals fire damage

```cpp
if (hasQuestBit(TOG_IS_COMBUSTIBLE) && !::number(0, 99)) {
    rc = flameEngulfed();
    if (IS_SET_DELETE(rc, DELETE_THIS))
        return DELETE_THIS;
}
```

### Hemophilia

**Source:** `/code/code/misc/magicutils.cc` (lines 1563-1565, 1587-1589)

- Doubles bleeding duration for normal wounds
- Makes limb bleeding permanent (does not stop naturally)

```cpp
const auto updateDuration = [this, pos](int dur) {
    if (hasQuestBit(TOG_IS_HEMOPHILIAC) || hasDisease(DISEASE_SCURVY) ||
        isLimbFlags(pos, PART_LEPROSED | PART_GANGRENOUS))
        dur *= 2;
    return dur;
};
```

### Necrophobia

**Source:** `/code/code/misc/periodic.cc` (lines 1299-1366)

- 25% chance per tick to trigger when awake
- Scans room for corpses or undead beings
- Triggers fear response: flee attempts, terror messages

### Alcoholism

**Source:** `/code/code/obj/obj_food.cc` (lines 193-217)

- Alcohol quenches thirst normally
- Non-alcoholic drinks stop satisfying thirst at 3 units
- Message: "Only sweet, sweet alcohol can quench your thirst any further."

```cpp
if (ch->hasQuestBit(TOG_IS_ALCOHOLIC)) {
    ch->gainCondition(THIRST, (getLiqDrunk() * amount) / 10);
    ch->sendTo("The deliciously satisfying alcohol quenches your thirst.\n\r");
}
```

### Tourettes

**Source:** `/code/code/misc/periodic.cc` (lines 1369-1420)

- 25% chance per tick to insult someone in the room
- Randomly selects visible being (excluding self)
- Generates random insulting message

### Healthy

**Source:** `/code/code/misc/immunity.cc` (lines 171-173)

- Adds +75% to disease immunity

```cpp
if (hasQuestBit(TOG_IS_HEALTHY) && type == IMMUNE_DISEASE) {
    imm += 75;
}
```

### Nightvision

**Source:** `/code/code/misc/player_data.cc` (lines 102-103)

- Adds +2 to vision bonus (improves ability to see in dark)

```cpp
if (hasQuestBit(TOG_HAS_NIGHTVISION))
    visionBonus += 2;
```

### Ambidextrous

**Source:** `/code/code/misc/being.h` (lines 671-678)

- `isAmbidextrous()` returns true regardless of DEX
- Normally requires DEX > 180 or `SKILL_AMBIDEXTERITY`

```cpp
bool isAmbidextrous() const {
    if (hasQuestBit(TOG_IS_AMBIDEXTROUS))
        return true;
    return ((getStat(STAT_CURRENT, STAT_DEX) > 180) ||
            doesKnowSkill(SKILL_AMBIDEXTERITY));
}
```

### Fae-Touched

**Source:** `/code/code/sys/create_character.cc` (lines 743-750), `/code/code/misc/limits.cc` (lines 1033-1036)

Creation bonus:
```cpp
if (desc->character->hasQuestBit(TOG_FAE_TOUCHED)) {
    int num_fifties = numFifties(desc->character->getRace(), ...);
    if (num_fifties > 0) {
        num_fifties = min(num_fifties, 26);
        desc->character->addToRandomStat(50 + (num_fifties - 1) * 2);
    }
}
```

XP penalty:
```cpp
if (ch->hasQuestBit(TOG_FAE_TOUCHED) and !fae_reduction_done) {
    fae_reduction_done = true;  // only apply once for multiclass
    gain /= 2;
}
```

## Trait Storage

### File Format

Traits are stored as quest bits (toggles) in player toggle files.

**Location:** `lib/mutable/player/{first_letter}/{name}.toggle`

**Format:** Space-separated list of toggle numbers

**Example:** `278 282 289` (Cowardice, Asthma, Nightvision)

### Save/Load Implementation

**Source:** `/code/code/misc/rent.cc` (lines 3417-3456)

```cpp
void TPerson::loadToggles() {
    char caFilebuf[128];
    sprintf(caFilebuf, "mutable/player/%c/%s.toggle", LOWER(name[0]),
        sstring(name).lower().c_str());

    FILE* fp = fopen(caFilebuf, "r");
    if (!fp) return;

    int num;
    while (fscanf(fp, "%d ", &num) == 1) {
        setQuestBit(num);
    }
    fclose(fp);
}

void TPerson::saveToggles() {
    // ... similar pattern, writes all quest bits
    for (num = 1; num < MAX_TOG_INDEX; num++) {
        if (hasQuestBit(num)) {
            fprintf(fp, "%d ", num);
        }
    }
}
```

### When Saved

**Source:** `/code/code/misc/player_data.cc` (line 370)

Toggles are saved as part of `storeToSt()`, called during character save:

```cpp
void TPerson::storeToSt(charFile* st) {
    saveToggles();
    saveWizPowers();
    // ... rest of save
}
```

## Viewing Traits

### Attribute Command

**Source:** `/code/code/cmd/cmd_attribute.cc` (lines 682-689)

The `attribute personal` command shows selected traits:

```cpp
buf = "";
for (int i = 0; i < MAX_TRAITS; ++i) {
    if (hasQuestBit(traits[i].tog)) {
        buf += traits[i].name;
        buf += " ";
    }
}
if (buf != "")
    sendTo(format("Your character traits are: %s\n\r") % buf);
```

### Who Command

**Source:** `/code/code/cmd/cmd_who.cc` (line 324)

The `who` command supports filtering by traits:
- `who !` - Shows fae-touched characters
- `who x` - Shows perma-death characters

## Common Gotchas

### Trait Points vs Stat Points

Trait points (`bonus_points.total`) are distributed into four stat categories before spending:

```cpp
int* bonusPoints[4] = {&desc->bonus_points.combat,
    &desc->bonus_points.combat2, &desc->bonus_points.learn,
    &desc->bonus_points.util};
int limit = desc->bonus_points.total / 4;
```

Excess points are distributed round-robin. Cannot leave creation with negative points in any category.

### Race-Specific Requirements

`num50race` counts only level 50 characters of the **same race** as being created:

```cpp
int num50race = numFifties(desc->character->getRace(),
    desc->character->hasQuestBit(TOG_PERMA_DEATH_CHAR),
    desc->account->name);
```

### Perma-Death Interaction

Fae-touched's `num50race` check respects perma-death:
- If creating perma-death fae-touched Elf, need L50 perma-death Elf
- If creating non-perma fae-touched Elf, need L50 non-perma Elf

### Toggle Number Stability

Trait toggles use specific numbers in the 278-299 range. These numbers are:
- Stored in player files
- Must remain stable across code changes
- Part of the quest bit system (shared with quest progress)

## Related Documentation

- [Class System](class-system.md) - Class selection during creation
- [Race System](race-system.md) - Race selection and stat impacts
- [Stats/Attributes](stats-attributes.md) - How bonus points affect stats
- [Experience/Leveling](experience-leveling.md) - Fae-touched XP penalty
- [Quest System](quest-system.md) - Quest bit infrastructure
- [Charfile Format](charfile-format.md) - Player data persistence
