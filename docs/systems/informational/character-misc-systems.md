---
title: Character Miscellaneous Systems
status: partial
description: Behavioral and social character subsystems covering alignment (moral positioning), pet/charm (followers from spells and purchases), and language (speech comprehension between races).
keywords: [alignment, moral positioning, followers, speech comprehension, races]
category: informational
primary_symbols:
  functions: [reconcileHurt, reconcileHelp, isPet, tooManyFollowers, garble, getLanguageChance, orderDenyCheck, stopFollower, restorePetToPc]
  classes: [factionData, TBeing, affectedData]
  enums: [SPELL_ENSORCER, AFFECT_PET, AFFECT_THRALL, AFFECT_ORPHAN_PET, AFF_CHARM, PETTYPE_PET, PETTYPE_CHARM, PETTYPE_THRALL, SKILL_COMMON, SKILL_SIGN, SKILL_FISHBURBLE, SKILL_GNOLL_JARGON, SKILL_TROGLODYTE_PIDGIN, SKILL_TROLLISH, SKILL_BULLYWUGCROAK, SKILL_AVIAN, SKILL_GUTTER_CANT, TYPE_SIGN, TYPE_DRUNK, TYPE_GLUBGLUB, TYPE_PG13IN, TYPE_PG13OUT, TYPE_FISHTALK, TYPE_TROLLTALK, TYPE_FROGTALK, TYPE_BIRDTALK, TYPE_GUTTER, TYPE_TROGTALK, TYPE_LOLCATS, SPEECH_SAY, SPEECH_ASK, SPEECH_WHISPER, SPEECH_SHOUT, SPEECH_TELL, SPEECH_GROUPTELL, SPEECH_COMMUNE, SPEECH_SIGN, SPEECH_WRITE, SPEECH_EMOTE, SPEECH_ROOMDESC, SPEECH_FLAG_SAY, SPEECH_FLAG_ASK, SPEECH_FLAG_WHISPER, SPEECH_FLAG_SHOUT, SPEECH_FLAG_TELL, SPEECH_FLAG_GROUPTELL, SPEECH_FLAG_COMMUNE, SPEECH_FLAG_SIGN, SPEECH_FLAG_WRITE, SPEECH_FLAG_EMOTE, SPEECH_FLAG_ROOMDESC, ACT_STRINGS_CHANGED, TALENT_MEATEATER, IMMUNE_CHARM]
---

# Character Miscellaneous Systems

## Overview

> **PARTIAL SYSTEM**: The **Alignment** subsystem is disabled via the same `FACTIONS_IN_USE 0` flag that disables faction percentage tracking. The `reconcileHelp()` and `reconcileHurt()` functions are empty when disabled. The **Pet/Charm** and **Language** subsystems remain fully active.

Characters in SneezyMUD possess behavioral and social attributes beyond combat statistics. Three subsystems govern these aspects:

**Alignment** places characters on moral axes that affect deity interactions, class eligibility, and equipment restrictions. Actions shift alignment gradually rather than instantly. *(Currently disabled)*

**Pet/Charm** creates master-follower relationships through spells, purchases, or taming. Different follower types exhibit different levels of obedience and self-preservation.

**Language** simulates communication barriers between races. Speech passes through garble transformations that obscure meaning based on speaker and listener language skills.

## Patterns

### Alignment

Always use `reconcileHurt()` and `reconcileHelp()` for alignment shifts. Direct modification bypasses faction tracking.

Always apply spell `alignMod` values when casting offensive spells. The modifier determines alignment shift direction and magnitude.

Never assume alignment changes are large. Per-hit shifts are 0.005, kill shifts are 0.03, and spell modifiers are typically 0.01-0.05.

### Pet/Charm

Always check `tooManyFollowers()` before creating new charmed followers. Exceeding limits prevents charm establishment.

Always distinguish between `AFFECT_CHARM` (affect type) and `AFF_CHARM` (bitvector flag). Use `affectedBySpell(AFFECT_CHARM)` for type checks and `isAffected(AFF_CHARM)` for flag checks.

Always use `isPet()` with the appropriate `PETTYPE_*` flags to determine follower category. Priority order is thrall, charm, pet.

Never allow charmed beings to issue orders. Check `isAffected(AFF_CHARM)` before processing order commands.

Always set `AFFECT_ORPHAN_PET` when a master disconnects or dies. Orphans have 80 mud hours before becoming permanently wild.

Never expect pets to obey combat commands. Use `orderDenyCheck()` to validate orders for pet-type followers.

### Language

Always apply garbles through `garble()` function. Direct speech without garbling bypasses language barriers.

Always check both speaker Common skill and listener perception. High Common helps the speaker be understood; high perception helps the listener understand.

Never garble speech to immortals. The garble system explicitly exempts immortal listeners.

Always check race garble flags with `getMyRace()->getGarbles()`. Zero means the race has no native accent.

## Reference

### Alignment Scale

| Range | Good/Evil Axis | Lawful/Chaotic Axis |
|-------|----------------|---------------------|
| -1000 to -350 | Evil | Chaotic |
| -349 to 349 | Neutral | Neutral |
| 350 to 1000 | Good | Lawful |

### Alignment Shift Triggers

| Action | Magnitude | Function |
|--------|-----------|----------|
| Combat hit | 0.005 | `reconcileHurt()` |
| Kill | 0.03 | `reconcileHurt()` |
| Spell cast | `spellInfo::alignMod` | `reconcileHurt()` |

### Follower Types

| Type | Affect | Self-Preservation | Order Compliance |
|------|--------|-------------------|------------------|
| Thrall | `AFFECT_THRALL` | None | Complete |
| Charm | `SPELL_ENSORCER` | Limited | High |
| Pet | `AFFECT_PET` | Full | Conditional |

### Follower Limit Weights

| Follower Type | Weight Formula |
|---------------|----------------|
| Zombie/Thrall | 1 + (level / 10) |
| Charm | 2 + (level / 10) |
| Pet | 1 + (level / 7) |

### Language Skills

| Skill | ID | Race |
|-------|----|----- |
| `SKILL_COMMON` | 719 | Universal |
| `SKILL_SIGN` | 576 | Sign language |
| `SKILL_FISHBURBLE` | 718 | Fishman |
| `SKILL_GNOLL_JARGON` | 713 | Gnoll |
| `SKILL_TROGLODYTE_PIDGIN` | 714 | Troglodyte |
| `SKILL_TROLLISH` | 715 | Troll |
| `SKILL_BULLYWUGCROAK` | 716 | Bullywug/Frogman |
| `SKILL_AVIAN` | 717 | Aarakocra |
| `SKILL_GUTTER_CANT` | 712 | Goblin/Orc |

### Garble Types

| Type | Trigger | Effect |
|------|---------|--------|
| `TYPE_SIGN` | Sign skill below max | Sign language errors |
| `TYPE_DRUNK` | Drunk level >= 9 | Slurred speech |
| `TYPE_GLUBGLUB` | Underwater without waterbreath | Replaces with "Glub glub glub." |
| `TYPE_PG13IN` | Player PG13 mode | Filters incoming profanity |
| `TYPE_PG13OUT` | Recipient PG13 mode | Filters outgoing profanity |
| `TYPE_FISHTALK` | Fishman to non-fishman | Fish accent |
| `TYPE_TROLLTALK` | Troll to non-troll | Klingon-like accent |
| `TYPE_FROGTALK` | Frogman to non-frogman | Softened consonants |
| `TYPE_BIRDTALK` | Aarakocra to non-aarakocra | Bird squawks |
| `TYPE_GUTTER` | Goblin/Orc to others | Cockney accent |
| `TYPE_TROGTALK` | Troglodyte to non-trog | Hyphenated words |
| `TYPE_LOLCATS` | Gnoll to non-gnoll | Internet speak |

### Speech Types

| Type | Flag | Commands |
|------|------|----------|
| `SPEECH_SAY` | `SPEECH_FLAG_SAY` | say, sayto |
| `SPEECH_ASK` | `SPEECH_FLAG_ASK` | ask |
| `SPEECH_WHISPER` | `SPEECH_FLAG_WHISPER` | whisper |
| `SPEECH_SHOUT` | `SPEECH_FLAG_SHOUT` | shout |
| `SPEECH_TELL` | `SPEECH_FLAG_TELL` | tell |
| `SPEECH_GROUPTELL` | `SPEECH_FLAG_GROUPTELL` | gtell |
| `SPEECH_COMMUNE` | `SPEECH_FLAG_COMMUNE` | commune |
| `SPEECH_SIGN` | `SPEECH_FLAG_SIGN` | sign |
| `SPEECH_WRITE` | `SPEECH_FLAG_WRITE` | write |
| `SPEECH_EMOTE` | `SPEECH_FLAG_EMOTE` | emote speech |
| `SPEECH_ROOMDESC` | `SPEECH_FLAG_ROOMDESC` | Room descriptions |

### Key Files

| System | Files |
|--------|-------|
| Alignment | `being.h`, `faction.cc`, `combat.cc`, `damage.cc`, `alignment_deity.cc` |
| Pet/Charm | `pets.cc`, `offense.cc`, `utility.cc`, `disc_mage_spirit.cc`, `spell_parser.cc` |
| Language | `garble.cc`, `race.cc`, `player_data.cc`, `spell_num.cc` |

## Implementation

### Alignment Storage

Alignment values reside in the `factionData` structure attached to each being. Two fields track position: `align_ge` for the good/evil axis and `align_lc` for the lawful/chaotic axis. Both use identical scales from -1000 to 1000. Access occurs through direct field references on the being's faction data member.

### Alignment Shift Mechanics

The `reconcileHurt()` function delegates to `reconcileHelp()` with negated amplitude. All damage-based alignment shifts flow through this path. Combat calls `reconcileHurt()` with amplitude 0.005 per hit. Kill shots use amplitude 0.03.

Spells define alignment impact through the `alignMod` field in their `spellInfo` definition. Positive values shift toward good/lawful when cast on enemies. Typical modifiers range from 0.01 to 0.05, creating gradual shifts over many casts.

### Alignment Checks

Several game systems query alignment: deity interactions reward or punish based on faction percentage, Deikhan class restricts to specific alignment ranges, equipment with `ITEM_ANTI_GOOD` or `ITEM_ANTI_EVIL` flags reject misaligned wearers, and certain spells target only aligned or unaligned beings.

### Follower Type Priority

The `isPet()` function checks follower types in priority order. When multiple charm effects apply, thrall status takes precedence, then charm, then pet. This ordering determines behavior when conflicting instructions arise.

### Charm Save Mechanics

The ensorcer spell performs multiple checks before establishing charm: immunity check via `isImmune(IMMUNE_CHARM, WEAR_BODY)`, level comparison (victim level cannot exceed caster level), hatred check for mobs that already hate the caster, power differential via `isNotPowerful()`, and luck save via `isLucky()`. Failing any check causes the charm to fail and the mob to become hostile through `addHated()`.

### Charm Duration

Base duration scales with caster level multiplied by 3 and `Pulse::UPDATES_PER_MUDHOUR`. The victim's charm immunity percentage reduces this duration proportionally. Critical success on the spell doubles or triples the duration. A luck save by the victim halves it.

The charm affect stores the caster's name in the `be` field for ownership tracking after the caster disconnects.

### Follower Limits

Maximum followers derive from level plus charisma modifier, divided by 30, plus 1, clamped to a minimum of 1 and maximum of 3. Each follower type has a different weight formula: zombies and thralls use 1 plus level/10, charms use 2 plus level/10, and pets use 1 plus level/7. However, the actual limit check at line 1330 compares the count of followers against max_followers, not total weight.

The `tooManyFollowers()` function iterates the followers linked list, counting followers, then compares the count against the maximum (which is capped at 3).

### Order Processing

The `doOrder()` function validates order eligibility. Charmed beings cannot issue orders. The target must be following the orderer and charmed by them. Pets undergo additional validation through `orderDenyCheck()` which rejects combat and suicidal commands. Thralls and charms obey all orders. Room flag `ROOM_NO_ORDER` blocks orders entirely.

Valid commands are queued into the follower's command queue for execution on their next action pulse.

### Orphan System

When a master logs out or dies, `stopFollower()` applies `AFFECT_ORPHAN_PET` with 80 mud hour duration. Orphans can be reclaimed by the original owner returning or by rangers using the `retrain` command. The `restorePetToPc()` function handles reclamation, checking for `AFFECT_ORPHAN_PET` and verifying ownership.

Retraining carries a 20% chance of permanent rejection, after which the mob goes fully wild.

### Pet Persistence

Pets with the `ACT_STRINGS_CHANGED` flag (indicating they were named) save to the database. The `petSave()` function inserts a row into the pet table with player ID, mob vnum, name, experience, and level. Loading occurs during character login by querying the pet table and spawning mobs that match the owner ID. Unnamed pets do not persist across sessions.

### Garble Application Flow

The `garble()` function retrieves active garbles via `getGarbles()`, then iterates through each garble type from zero to `TYPE_MAX`. Each active garble that matches the speech type and scope applies its transformation function. Immortal listeners bypass all garbles.

### Racial Garble Activation

Racial garbles activate when speaker and listener have different native garbles and the listener has intelligence below 180. The `getMyRace()->getGarbles()` function returns the race's garble flags. Intelligence 180 or higher completely bypasses racial language barriers.

### Comprehension Calculation

The `getLanguageChance()` function returns the percentage chance of garbling each word. It combines listener's language skill value (90% if skill check succeeds), perception bonus via `plotStat()` (0-16), speaker's Common skill (80% if check succeeds), and intelligence modifier via `plotStat()` (-10 to +10). Result clamps to 0-100 range where 0 is perfect understanding.

### Common Skill Initialization

Races without native garbles (garble flags of zero) automatically receive maximum `SKILL_COMMON` during character creation. Races with garbles must learn Common through practice to reduce speech garbling to other races.

### Underwater Speech

The `TYPE_GLUBGLUB` garble activates when speaking underwater without waterbreath. Unlike other garbles that transform text, this garble replaces the entire message with "Glub glub glub." regardless of original content.

## Troubleshooting

### Alignment not shifting during combat

**Symptom:** Player alignment remains static despite extended combat.

**Cause:** The faction system may be disabled via `FACTIONS_IN_USE` preprocessor flag.

**Diagnostic:** Check if `reconcileHurt()` calls are reaching `reconcileHelp()`. Verify the flag is defined in build configuration.

**Fix:** Enable the faction system in compile flags or verify that alignment shifts are reaching the storage.

### Charm spell fails unexpectedly

**Symptom:** Ensorcer fails on targets that should be charmable.

**Cause:** Multiple conditions cause charm failure: immunity, level difference, existing hatred, power differential, or luck save.

**Diagnostic:** Check `failCharm()` message output. Verify target level does not exceed caster level. Check if mob already has caster in hate list.

**Fix:** Target lower-level mobs, improve caster level, or avoid mobs that have been previously hostile.

### Pet refuses commands

**Symptom:** Pet ignores valid orders.

**Cause:** Pets (not thralls or charms) refuse combat and suicidal commands through `orderDenyCheck()`.

**Diagnostic:** Verify follower type via `isPet(PETTYPE_PET)`. Check if command is in the denial list.

**Fix:** Use thralls or charms for combat orders. Pets maintain self-preservation and reject dangerous commands.

### Too many followers error

**Symptom:** Player cannot charm or acquire new followers despite having few visible pets.

**Cause:** High-level followers consume more weight slots than low-level ones.

**Diagnostic:** Calculate total follower weight by summing each follower's contribution based on type and level. Compare against maximum followers from level plus charisma divided by 30, capped at 3.

**Fix:** Dismiss high-level followers to free weight capacity, or increase character level and charisma.

### Orphan pet cannot be reclaimed

**Symptom:** Player cannot reclaim their orphaned pet.

**Cause:** The 80 mud hour window expired, or the 20% retraining rejection triggered.

**Diagnostic:** Check if `AFFECT_ORPHAN_PET` still exists on the mob. Verify the mob did not reject retraining.

**Fix:** Orphan window cannot be extended. Rejected mobs become permanently wild. Prevent by returning before timeout.

### Named pet not persisting

**Symptom:** A named pet does not appear after login.

**Cause:** The `ACT_STRINGS_CHANGED` flag may not be set, or the save did not complete.

**Diagnostic:** Verify the flag is set on the mob. Check the pet table in the database for an entry with matching owner ID and mob vnum.

**Fix:** Ensure the pet was properly named through game mechanics that set `ACT_STRINGS_CHANGED`. Verify `petSave()` is being called during appropriate save points.

### Speech completely unintelligible

**Symptom:** All speech from a character displays as garbled.

**Cause:** Multiple garbles stacking or underwater garble (`TYPE_GLUBGLUB`) replacing all text.

**Diagnostic:** Check if character is underwater without waterbreath. Check drunk level. Verify racial garble flags.

**Fix:** For underwater, apply waterbreath. For drunk, wait for sobriety. For racial, improve Common skill.

### Language skill not helping

**Symptom:** High language skill does not improve comprehension.

**Cause:** Common skill helps the speaker be understood, not the listener. Listener needs the specific racial language skill.

**Diagnostic:** Check which character has which language skills. Speaker needs Common; listener needs the racial language.

**Fix:** Have speakers learn Common. Have listeners learn relevant racial languages or rely on high perception.

### Garbles not applying to immortals

**Symptom:** Immortals receive ungarbled messages.

**Cause:** Intended behavior. The garble system explicitly exempts immortal listeners.

**Diagnostic:** Verify recipient's immortal status.

**Fix:** Not a bug. Immortals bypass garbles by design for monitoring and administration purposes.
