---
title: Character Miscellaneous Systems
description: Character subsystems covering Alignment tracking moral positioning, Pet/Charm management of followers created through spells and purchases, and Language handling speech comprehension between races.
keywords: [factionData, align_ge, align_lc, reconcileHurt, reconcileHelp, AFFECT_THRALL, AFFECT_CHARM, AFFECT_PET, isPet, addHated, opinionData, garble, getGarbles, SKILL_COMMON, garbleFunction, getLanguageChance, doOrder, tooManyFollowers]
category: Understanding Systems

last_updated: 2026-01-29
source_files: [code/code/misc/being.h, code/code/misc/faction.cc, code/code/misc/combat.cc, code/code/misc/damage.cc, code/code/misc/spell_info.cc, code/code/misc/alignment_deity.cc, code/code/misc/pets.cc, code/code/misc/offense.cc, code/code/misc/utility.cc, code/code/disc/disc_mage_spirit.cc, code/code/misc/spell_parser.cc, code/code/misc/garble.cc, code/code/misc/race.cc, code/code/misc/player_data.cc, code/code/misc/spell_num.cc]
related:
  - affects-system.md
  - character-foundation.md
  - faction-system.md
  - monster-ai-behavior.md
  - spell-skill-framework.md
---

# Character Miscellaneous Systems

This document covers three character subsystems: Alignment, Pet/Charm management, and Languages (garbles).

## Overview

These systems represent behavioral and social aspects of characters beyond core combat mechanics:

- **Alignment** tracks moral positioning on good/evil and lawful/chaotic axes
- **Pet/Charm** manages followers created through spells, purchases, or special mechanics
- **Language** handles speech comprehension between races with different native tongues

## Alignment System

SneezyMUD uses a two-axis alignment system tracking both good/evil and lawful/chaotic orientations.

### Alignment Storage

Alignment is stored in the `factionData` structure attached to each being:

```cpp
class factionData {
    int align_ge;  // alignment on good/evil axis
    int align_lc;  // alignment on chaotic/lawful axis
    // ...
};
```

**Source:** `code/code/misc/being.h:168-169`

### Alignment Scale

Both axes use the same numeric scale:

| Value Range | Good/Evil | Lawful/Chaotic |
|-------------|-----------|----------------|
| -1000 to -350 | Evil | Chaotic |
| -349 to 349 | Neutral | Neutral |
| 350 to 1000 | Good | Lawful |

### Alignment Shift Mechanics

Alignment shifts occur through the `reconcileHurt()` and `reconcileHelp()` functions, which are called during combat and helpful actions.

```cpp
void TPerson::reconcileHurt(TBeing* victim, double amp) {
    reconcileHelp(victim, -amp);
}
```

**Source:** `code/code/misc/faction.cc:1623-1625`

**Shift triggers:**

| Action | Alignment Effect | Code Location |
|--------|------------------|---------------|
| Combat damage | `reconcileHurt(victim, 0.005)` | `combat.cc:4147` |
| Kill damage | `reconcileHurt(victim, 0.03)` | `damage.cc:1010` |
| Spell casting | `reconcileHurt(victim, discArray[spell]->alignMod)` | Various disc files |

### Spell Alignment Modifiers

Each spell has an `alignMod` field in its `spellInfo` definition that determines how much alignment shifts when cast:

```cpp
spellInfo::spellInfo(..., float alignMod, ...) :
    // ...
    alignMod(alignMod),
    // ...
```

**Source:** `code/code/misc/spell_info.cc:57`, `code/code/misc/spell2.h:461`

Positive `alignMod` values shift toward good/lawful when cast on enemies; negative values shift toward evil/chaotic. The modifier is typically small (0.01-0.05) to create gradual shifts.

### Alignment Restrictions

Certain game mechanics check alignment:

- **Deity interactions** - Alignment deities reward/punish based on faction percentage
- **Class restrictions** - Some classes (Deikhan) have alignment requirements
- **Item restrictions** - Equipment may have `ITEM_ANTI_GOOD` or `ITEM_ANTI_EVIL` flags
- **Spell targeting** - Some spells only affect aligned/unaligned targets

**Source:** `code/code/misc/alignment_deity.cc`

## Pet and Charm System

The pet/charm system manages three distinct types of followers that share the `AFF_CHARM` flag but have different behaviors and restrictions.

### Follower Categories

| Type | Affect Type | Self-Preservation | Order Compliance | Example Sources |
|------|-------------|-------------------|------------------|-----------------|
| **Thrall** | `AFFECT_THRALL` | None | Complete | Animate dead, golems |
| **Charm** | `AFFECT_CHARM` / `SPELL_ENSORCER` | Limited | High (fights beguile) | Ensorcer spell |
| **Pet** | `AFFECT_PET` | Full | Conditional | Pet shops, ranger taming |

**Source:** `code/code/misc/pets.cc:14-44`

### Priority Rules

When multiple charm effects stack, precedence is:
1. Thrall (overrides all)
2. Charm (overrides pet)
3. Pet (base state)

```cpp
bool TBeing::isPet(const unsigned int bv) const {
    if (isPc() || !master)
        return false;
    if (!isAffected(AFF_CHARM))
        return false;
    // Check specific types in priority order
    if (IS_SET(bv, PETTYPE_THRALL)) {
        if (affectedBySpell(AFFECT_THRALL))
            return true;
    }
    // ...
}
```

**Source:** `code/code/misc/pets.cc:92-119`

### Charm Save Mechanics

The ensorcer (charm person) spell has multiple save checks:

```cpp
// From disc_mage_spirit.cc:436-451
if (victim->isImmune(IMMUNE_CHARM, WEAR_BODY) ||
    victim->GetMaxLevel() > caster->GetMaxLevel() ||
    (!victim->isPc() && dynamic_cast<TMonster*>(victim)->Hates(caster, NULL)) ||
    caster->isNotPowerful(victim, level, SPELL_ENSORCER, SILENT_YES) ||
    (victim->isLucky(caster->spellLuckModifier(SPELL_ENSORCER)))) {
    victim->failCharm(caster);
    // Charm fails, mob becomes hostile
}
```

**Charm fails if:**
- Victim has charm immunity
- Victim's level exceeds caster's level
- Mob already hates the caster
- Power differential too high
- Victim makes luck save

**Source:** `code/code/disc/disc_mage_spirit.cc:380-526`

### Charm Duration

Duration scales with caster level and is reduced by victim's charm immunity:

```cpp
aff.duration = caster->durationModify(SPELL_ENSORCER,
    3 * level * Pulse::UPDATES_PER_MUDHOUR);

// Reduce by immunity percentage
aff.duration *= (100 - victim->getImmunity(IMMUNE_CHARM));
aff.duration /= 100;
```

Critical success doubles or triples duration; luck save halves it.

**Source:** `code/code/disc/disc_mage_spirit.cc:465-488`

### Follower Limits

Characters are limited in how many followers they can control based on level and charisma:

```cpp
bool TBeing::tooManyFollowers(const TBeing* pet, newFolTypeT type) const {
    int max_followers = (GetMaxLevel() + plotStat(STAT_CURRENT, STAT_CHA, -15, 15, 0)) / 20;

    int count = 0;
    for (followData* k = followers; k; k = k->next) {
        if (k->follower->isPet(PETTYPE_PET | PETTYPE_CHARM | PETTYPE_THRALL))
            tot_num++;
    }

    // Weight varies by follower type
    if (type == FOL_ZOMBIE)
        count += 1 + (pet->GetMaxLevel() / 10);
    else if (type == FOL_CHARM)
        count += 2 + (pet->GetMaxLevel() / 10);
    else if (type == FOL_PET)
        count += 1 + (pet->GetMaxLevel() / 7);

    if (tot_num >= max_followers)
        return TRUE;
    return FALSE;
}
```

**Source:** `code/code/misc/utility.cc:1286-1333`

### Order Command

The `order` command allows controlling charmed followers:

```cpp
int TBeing::doOrder(const char* argument) {
    // Charmed beings cannot give orders
    if (isAffected(AFF_CHARM)) {
        sendTo("Your superior would not approve of you giving orders.\n\r");
        return FALSE;
    }

    // Check if target is following and charmed by this
    if (v->master == this && v->isAffected(AFF_CHARM)) {
        // Pets have limited command acceptance
        if (v->isPet(PETTYPE_PET)) {
            if (!orderDenyCheck(cmd_buf))
                legitimate = true;
        } else {
            // Thralls and charms obey all orders
            legitimate = true;
        }
    }
    // ...
}
```

**Order restrictions:**
- Cannot order in `ROOM_NO_ORDER` flagged rooms
- Cannot order if you are charmed yourself
- Pets refuse combat/suicidal commands (checked by `orderDenyCheck()`)
- Mounts have ego checks that may cause bucking

**Source:** `code/code/misc/offense.cc:574-719`

### Orphan Pets

When a master logs out or dies, pets become "orphaned" and can be retrained:

```cpp
if (affectedBySpell(AFFECT_PET) || affectedBySpell(AFFECT_CHARM) ||
    affectedBySpell(AFFECT_THRALL)) {
    aff.type = AFFECT_ORPHAN_PET;
    aff.level = 0;
    aff.duration = 80 * Pulse::UPDATES_PER_MUDHOUR;
    // ...
    affectTo(&aff, -1);
}
```

Orphaned pets can be reclaimed by:
- Original owner returning
- Rangers using `retrain` command (can retrain for others)

There's a 20% chance of rejection when retraining:

```cpp
if (!::number(0, 4)) {
    act("$N rejects your retraining and remains wild.", FALSE, this, NULL, v, TO_CHAR);
    v->affectFrom(AFFECT_ORPHAN_PET);
    return FALSE;
}
```

**Source:** `code/code/misc/pets.cc:237-416`, `code/code/misc/spell_parser.cc:156-175`

### Pet Persistence

Named pets (with `ACT_STRINGS_CHANGED` flag) are saved to the database:

```cpp
void TBeing::petSave() {
    if (!(specials.act & ACT_STRINGS_CHANGED))
        return;  // Only save named pets

    db.query("insert into pet (player_id, vnum, name, exp, level) values (%i, %i, '%s', %f, %i)",
        owner_id, mobVnum(), name.c_str(), getExp(), GetMaxLevel());
}
```

**Source:** `code/code/misc/pets.cc:46-89`

## Language System

SneezyMUD implements a "garble" system that modifies speech between beings of different races or conditions.

### Language Skills

Languages are implemented as skills that affect speech comprehension:

| Skill | ID | Used By |
|-------|----|---------|
| `SKILL_COMMON` | 719 | Universal speech skill |
| `SKILL_SIGN` | 576 | Sign language |
| `SKILL_FISHBURBLE` | 718 | Fishman race |
| `SKILL_GNOLL_JARGON` | 713 | Gnoll race (lolcats garble) |
| `SKILL_TROGLODYTE_PIDGIN` | 714 | Troglodyte race |
| `SKILL_TROLLISH` | 715 | Troll race |
| `SKILL_BULLYWUGCROAK` | 716 | Bullywug/Frogman race |
| `SKILL_AVIAN` | 717 | Aarakocra race |
| `SKILL_GUTTER_CANT` | 712 | Goblin/Orc races |

**Source:** `code/code/misc/spell_num.cc:1012-1027`

### Garble System Architecture

The garble system processes speech through multiple transformations:

```cpp
sstring TBeing::garble(TBeing* to, const sstring& arg,
    Garble::SPEECHTYPE speechType, Garble::SCOPE garbleScope) {

    sstring garbled = arg;
    int garbleFlags = getGarbles(to);

    // No garbles for immortals
    if (!garbleFlags || (to && to->isImmortal()))
        return garbled;

    // Apply each active garble in order
    for (int iGarble = 0; iGarble < Garble::TYPE_MAX; iGarble++) {
        if (!(1 << iGarble & garbleFlags))
            continue;
        if (!(garbleScope & GarbleData[iGarble].scope))
            continue;
        if (!((1 << speechType) & GarbleData[iGarble].speechFlags))
            continue;

        garbled = GarbleData[iGarble].garbleFunction(this, to, garbled, speechType);
    }
    return garbled;
}
```

**Source:** `code/code/misc/garble.cc:253-287`

### Garble Types

| Type | Flag | Effect | Automatic |
|------|------|--------|-----------|
| `TYPE_SIGN` | Applied when sign skill < max | Sign language errors | Yes |
| `TYPE_DRUNK` | Applied when drunk >= 9 | Slurred speech | Yes |
| `TYPE_GLUBGLUB` | Underwater without waterbreath | "Glub glub glub." | Yes |
| `TYPE_PG13IN` | Player has PG13 mode enabled | Filters incoming swears | Yes |
| `TYPE_PG13OUT` | Recipient has PG13 mode | Filters outgoing swears | Yes |
| `TYPE_FISHTALK` | Fishman speaking to non-fishman | Fish accent | Racial |
| `TYPE_TROLLTALK` | Troll speaking to non-troll | Klingon-like accent | Racial |
| `TYPE_FROGTALK` | Frogman speaking to non-frogman | Soft consonants | Racial |
| `TYPE_BIRDTALK` | Aarakocra speaking to non-aarakocra | Bird squawks | Racial |
| `TYPE_GUTTER` | Goblin/Orc speaking to others | Cockney accent | Racial |
| `TYPE_TROGTALK` | Troglodyte speaking to non-trog | Hyphenated words | Racial |
| `TYPE_LOLCATS` | Gnoll speaking to non-gnoll | Internet speak | Racial |

**Source:** `code/code/misc/garble.cc:126-182`

### Race-Based Language Application

Racial garbles are applied when speaker and listener have different native garbles:

```cpp
if (to && to->getMyRace()->getGarbles() != getMyRace()->getGarbles() &&
    to->getStat(STAT_CURRENT, STAT_INT) < 180) {
    garbleFlags |= getMyRace()->getGarbles();
}
```

High intelligence (180+) bypasses racial language barriers.

**Source:** `code/code/misc/garble.cc:209-212`

### Comprehension Formula

Language comprehension combines speaker and listener skills:

```cpp
int getLanguageChance(TBeing* from, TBeing* to, spellNumT language) {
    // Listener's ability to understand
    int learning = to ? to->getSkillValue(language) : 0;
    int chance = to && to->bSuccess(language) ? learning * 9 / 10 : 0;

    // Perception bonus for accent comprehension
    chance += to ? to->plotStat(STAT_CURRENT, STAT_PER, 0, 16, 8) : 0;

    // Speaker's Common skill bonus
    if (from && from->doesKnowSkill(SKILL_COMMON) && from->bSuccess(SKILL_COMMON)) {
        chance += from->getSkillValue(SKILL_COMMON) * 4 / 5;
    }

    // Intelligence modifier (-10 to +10)
    chance += from ? from->plotStat(STAT_CURRENT, STAT_INT, -10, 10, 0) : 0;

    return min(100, max(0, 100 - chance));
}
```

**Returns:** Percentage chance of garbling each word (0 = perfect understanding, 100 = complete garble).

**Source:** `code/code/misc/garble.cc:294-314`

### Speech Types

Garbles apply selectively based on speech type:

| Type | Flag | Examples |
|------|------|----------|
| `SPEECH_SAY` | `SPEECH_FLAG_SAY` | say, sayto |
| `SPEECH_ASK` | `SPEECH_FLAG_ASK` | ask |
| `SPEECH_WHISPER` | `SPEECH_FLAG_WHISPER` | whisper |
| `SPEECH_SHOUT` | `SPEECH_FLAG_SHOUT` | shout |
| `SPEECH_TELL` | `SPEECH_FLAG_TELL` | tell |
| `SPEECH_GROUPTELL` | `SPEECH_FLAG_GROUPTELL` | gtell |
| `SPEECH_COMMUNE` | `SPEECH_FLAG_COMMUNE` | commune |
| `SPEECH_SIGN` | `SPEECH_FLAG_SIGN` | sign language |
| `SPEECH_WRITE` | `SPEECH_FLAG_WRITE` | writing |
| `SPEECH_EMOTE` | `SPEECH_FLAG_EMOTE` | emote speech |
| `SPEECH_ROOMDESC` | `SPEECH_FLAG_ROOMDESC` | Room descriptions (drunk effect) |

**Source:** `code/code/misc/garble.cc:16-47`

### Learning SKILL_COMMON

Races without native garbles automatically receive maxed `SKILL_COMMON`:

```cpp
if (snt == SKILL_COMMON && getMyRace() && getMyRace()->getGarbles() == 0) {
    setNatSkillValue(snt, MAX_SKILL_LEARNEDNESS);
    setSkillValue(snt, MAX_SKILL_LEARNEDNESS);
    continue;
}
```

Races with garbles must learn Common through practice to speak clearly to others.

**Source:** `code/code/misc/player_data.cc:702-707`

## Code References

### Alignment

| File | Function/Line | Purpose |
|------|---------------|---------|
| `code/code/misc/being.h:168-169` | `factionData` | Alignment storage |
| `code/code/misc/faction.cc:1623-1625` | `reconcileHurt()` | Alignment shift trigger |
| `code/code/misc/combat.cc:4147` | Combat alignment | Per-hit alignment shift |
| `code/code/misc/damage.cc:1010` | Kill alignment | Kill alignment shift |
| `code/code/misc/spell_info.cc:57,76` | `spellInfo::alignMod` | Spell alignment modifier |
| `code/code/misc/alignment_deity.cc` | `alignment_deity()` | Deity reward/punish system |

### Pet/Charm

| File | Function/Line | Purpose |
|------|---------------|---------|
| `code/code/misc/pets.cc:14-44` | Comment block | Pet type documentation |
| `code/code/misc/pets.cc:92-119` | `isPet()` | Pet type checking |
| `code/code/misc/pets.cc:237-324` | `doRetrainPet()` | Pet retraining |
| `code/code/misc/pets.cc:333-416` | `restorePetToPc()` | Orphan restoration |
| `code/code/misc/offense.cc:574-719` | `doOrder()` | Order command |
| `code/code/misc/utility.cc:1286-1333` | `tooManyFollowers()` | Follower limits |
| `code/code/disc/disc_mage_spirit.cc:380-526` | `ensorcer()` | Charm spell |
| `code/code/misc/spell_parser.cc:156-175` | `stopFollower()` | Orphan creation |

### Language/Garble

| File | Function/Line | Purpose |
|------|---------------|---------|
| `code/code/misc/garble.cc:126-182` | `GarbleData[]` | Garble type definitions |
| `code/code/misc/garble.cc:185-215` | `getGarbles()` | Get active garbles |
| `code/code/misc/garble.cc:253-287` | `garble()` | Apply garbles to speech |
| `code/code/misc/garble.cc:294-314` | `getLanguageChance()` | Comprehension calculation |
| `code/code/misc/garble.cc:901-976` | `garble_fishtalk()` | Example racial garble |
| `code/code/misc/race.cc:1306` | `getGarbles()` | Race garble flags |
| `code/code/misc/player_data.cc:702-707` | Common skill init | Auto-max for garble-less races |

## Common Gotchas

### Alignment

1. **Alignment is per-hit, not per-fight**: Small shifts accumulate over combat duration
2. **Spell alignMod is directional**: Positive = good shift when hurting evil targets
3. **Faction system is conditionally compiled**: Check `FACTIONS_IN_USE` preprocessor flag

### Pet/Charm

1. **AFFECT_CHARM vs AFF_CHARM**: `AFFECT_CHARM` is the affect type tracking the owner; `AFF_CHARM` is the bitvector flag indicating charmed state
2. **Order denial**: Pets refuse combat orders via `orderDenyCheck()`; thralls/charms do not
3. **Owner tracking**: The `be` field in `affectedData` stores owner name as `char*`, not a `TThing*` pointer
4. **Orphan duration**: 80 mud hours; if not reclaimed, mob goes fully wild
5. **Retraining has failure chance**: 20% chance of permanent rejection

### Language/Garble

1. **Garbles apply in order**: Order in `GarbleData[]` matters for stacking effects
2. **Intelligence bypass**: 180+ INT completely bypasses racial garbles
3. **Immortals are exempt**: Telling to immortals bypasses all garbles
4. **SKILL_COMMON helps speaker**: A speaker with high Common makes themselves easier to understand
5. **Underwater garble is absolute**: `TYPE_GLUBGLUB` replaces all speech with "Glub glub glub."

## Related Documentation

- [Affects System](affects-system.md) - How AFF_CHARM and AFFECT_* types work
- [Race System](race-system.md) - Racial garble assignments
- [Faction System](faction-system.md) - Broader faction mechanics
- [Mob AI](mob-ai.md) - How charmed mobs behave
- [Discipline System](discipline-system.md) - Language skill learning
