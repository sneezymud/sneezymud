---
title: Who List and Visibility System
description: The who command displays online players, but visibility depends on invisibility levels, affects, room lighting, and viewer permissions. This document covers three distinct visibility systems that determine if one character can see another.
keywords: canSeeWho, doWho, invisLevel, PLR_ANONYMOUS, PLR_STEALTH, AFF_INVISIBLE, AFF_SHADOW_WALK, AFF_DETECT_INVISIBLE, isLinkdead, fixClientPlayerLists, GOD_LEVEL1, MAX_MORT, doWhozone
category: Important Systems
related:
  - affects-system.md
  - wizard-powers.md
  - snoop-switch.md
  - class-hierarchy.md
last_updated: 2026-01-29
source_files:
  - code/code/cmd/cmd_who.cc
  - code/code/misc/utility.cc
  - code/code/misc/being.cc
  - code/code/misc/toggle.cc
  - code/code/misc/player_data.cc
  - code/code/sys/client.cc
  - code/code/misc/defs.h
  - code/code/misc/toggle.h
---

# Who List and Visibility System

The who command displays online players, but visibility depends on invisibility levels, affects, room lighting, and viewer permissions. Understanding this system is essential for implementing features that depend on character detection.

## Overview

Three distinct visibility systems interact to determine if one character can see another:

1. **Who List Visibility** (`canSeeWho`): Determines if a character appears on the who list
2. **Room Visibility** (`canSee`/`canSeeMe`): Determines if a character can be seen in the same room
3. **Remote Visibility** (`can_see_char_other_room`): Determines if a character can be seen across rooms

Each system has different rules. A character may be visible on the who list but hidden in a room, or vice versa.

## Who Command

**Source:** `code/code/cmd/cmd_who.cc:140-588`

### Syntax

```
who                       -- List all visible players
who -?                    -- Show help for flags
who <name>                -- Search for players by name
who <level>               -- List players at or above level
who <level1> <level2>     -- List players between levels
who -<flags>              -- Filter by flags (see below)
```

### Display Format

The basic who list shows player titles. Additional information can be displayed using flags:

```
Players: (Add -? for online help)
--------
<Player Title>
<Player Title>   (Seeking Group)
<Player Title>   (Newbie-Helper)
<Player Title>   (Newbie)

Total Players : [N] Max since last reboot : [M] Avg Players : [X.X]
```

### Filtering Flags

| Flag | Description | Access |
|------|-------------|--------|
| `-l` | Show levels and class | All |
| `-q` | Show questers only | All |
| `-g` | Show gods only | All |
| `-b` | Show builders only | All |
| `-o` | Show mortals only | All |
| `-z` | Show players seeking group | All |
| `-p` | Show group leaders and members | All |
| `-y` | Show ungrouped players | All |
| `-f` | Show faction/guild info | All |
| `-e` | Filter by elf race | All |
| `-t` | Filter by hobbit race | All |
| `-n` | Filter by gnome race | All |
| `-u` | Filter by human race | All |
| `-r` | Filter by ogre race | All |
| `-w` | Filter by dwarf race | All |
| `-1` through `-8` | Filter by class (Mage, Cleric, Warrior, Thief, Deikhan, Monk, Ranger, Shaman) | All |
| `-x` | Show Perma Death characters | All |
| `-!` | Show Fae-touched characters | All |
| `-i` | Show idle time | Immortal |
| `-h` | Show HP/mana/move/money | Immortal |
| `-d` | Show link-dead players | Immortal |
| `-s` | Show all stats | Immortal |
| `-a` | Show account names | Wizard |

### Whozone Command

**Source:** `code/code/cmd/cmd_who.cc:590-615`

Immortal-only command that lists all players in the same zone with their room locations.

```
whozone    -- List players in current zone (level 51+ only)
```

## Invisibility System

### Invisibility Level (invisLevel)

The `invisLevel` field on `TBeing` controls immortal invisibility. This is level-based masking, not spell invisibility.

**Source:** `code/code/misc/being.cc:1081-1092`

```cpp
short TBeing::getInvisLevel() const { return invisLevel; }
void TBeing::setInvisLevel(short num);
```

| invisLevel Value | Meaning |
|------------------|---------|
| 0 | Fully visible |
| 1-50 | Hidden from players below this level |
| 51+ (GOD_LEVEL1) | Hidden from all mortals, used for linkdead/invis gods |

**Key constants:**
- `MAX_MORT = 50` - Maximum mortal level
- `GOD_LEVEL1 = 51` - First immortal level
- `MAX_IMMORT = 60` - Maximum immortal level

### Setting Invisibility

The `invis` toggle command (`code/code/misc/toggle.cc:787-806`):

```
invis         -- Toggle between invis 0 and invis 51
invis <level> -- Set invisibility to specific level
```

Example behavior:
- `invis` with current invis 0 -> sets to 51 (invisible to mortals)
- `invis` with current invis 51 -> sets to 0 (fully visible)
- `invis 55` -> invisible to players level 54 and below

### Stealth Mode (PLR_STEALTH)

**Source:** `code/code/misc/toggle.h:58`

Immortal flag that suppresses movement/action messages from being shown to mortals. Combined with invisLevel for silent observation.

```cpp
const unsigned long PLR_STEALTH = (1 << 5);
```

When an immortal has PLR_STEALTH set, certain action messages use `MAX_MORT` as a visibility threshold, preventing mortals from seeing messages like "X's body splits into a cloud of atoms."

### Anonymous Mode (PLR_ANONYMOUS)

**Source:** `code/code/misc/toggle.h:82`

```cpp
const unsigned long PLR_ANONYMOUS = (1 << 29);
```

Mortal flag that hides class and level information on the who list. When anonymous:
- Level-based who filters (`who <level>`) won't match the player
- Class-based filters (`who -1` through `-8`) won't match the player
- Level display shows "Anonymous" instead of actual level/class

Immortals can always see through anonymous mode.

## Spell-Based Invisibility

### AFF_INVISIBLE

**Source:** `code/code/misc/defs.h:86`

Standard invisibility from spells like SPELL_INVISIBILITY. Blocked by AFF_DETECT_INVISIBLE.

### AFF_SHADOW_WALK

**Source:** `code/code/misc/defs.h:109`

Shadow-based invisibility that only works in low light (room illumination < 14). Also blocked by AFF_DETECT_INVISIBLE.

### Detection Affects

| Affect | Effect |
|--------|--------|
| `AFF_DETECT_INVISIBLE` | See through AFF_INVISIBLE and AFF_SHADOW_WALK |
| `AFF_TRUE_SIGHT` | See through all invisibility and illusions |
| `AFF_CLARITY` | Functions like TRUE_SIGHT for visibility purposes |

## canSeeWho() - Who List Visibility

**Source:** `code/code/misc/utility.cc:63-93`

This function determines if one character appears on another's who list.

```cpp
bool TBeing::canSeeWho(const TBeing* o) const;
```

### Algorithm

1. **Null/invalid checks**: Return FALSE if either being has invalid room
2. **Immortal viewer**: Can see anyone with invisLevel <= their own level
3. **Immortal target with higher invisLevel**: Return FALSE (can't see higher-level invis gods)
4. **Mortal viewer, target invisLevel >= GOD_LEVEL1**: Return FALSE (includes linkdead)
5. **Spell invisibility check**:
   - If target has AFF_INVISIBLE OR (low light AND AFF_SHADOW_WALK)
   - AND target is immortal: Return FALSE
   - AND viewer lacks AFF_DETECT_INVISIBLE: Return FALSE
6. **Blind viewer**: Return FALSE (unless AFF_TRUE_SIGHT or AFF_CLARITY)

### Code Reference

```cpp
bool TBeing::canSeeWho(const TBeing* o) const {
  if (inRoom() < 0 || o->inRoom() < 0 || !o || !o->roomp)
    return FALSE;

  int illum = o->roomp->getLight();

  if (isImmortal()) {
    if (GetMaxLevel() < o->getInvisLevel())
      return FALSE;
    else
      return TRUE;
  }
  if ((GetMaxLevel() < o->getInvisLevel()) && (o->isImmortal()))
    return FALSE;  // invis gods

  if (!isImmortal() && (o->getInvisLevel() >= GOD_LEVEL1))
    return FALSE;  // link deads

  if (o->isAffected(AFF_INVISIBLE) ||
      (illum < 14 && o->isAffected(AFF_SHADOW_WALK))) {
    if (o->isImmortal())
      return FALSE;
    if (!isAffected(AFF_DETECT_INVISIBLE))
      return FALSE;
  }
  if (isAffected(AFF_BLIND) && !isAffected(AFF_TRUE_SIGHT) &&
      !isAffected(AFF_CLARITY))
    return FALSE;

  return TRUE;
}
```

## canSeeMe() - Room Visibility

**Source:** `code/code/misc/utility.cc:838-923`

More complex than who list visibility, this handles room-based visibility including infravision and environmental factors.

```cpp
bool TBeing::canSeeMe(const TBeing* ch, infraTypeT infra) const;
```

### Additional Factors

Beyond invisLevel and spell invisibility:
- Room lighting level
- Infravision (temperature-based detection)
- Sneak/hide states
- Weather conditions (arctic/tropical)
- Environmental modifiers

## Linkdead State

**Source:** `code/code/misc/player_data.cc:1701-1703`

```cpp
bool TBeing::isLinkdead() const {
  return (isPc() && !desc && polyed == POLY_TYPE_NONE);
}
```

A player becomes linkdead when their connection drops but their character remains in the game. Linkdead characters:
- Have `invisLevel` automatically set to `GOD_LEVEL1` (51)
- Are hidden from the who list for mortals
- Are visible to immortals with `[playername]` bracket notation
- Can be shown with `who -d` (immortal only)

When a player reconnects, `invisLevel` is reset to 0.

## Special Display Cases

### Polymorphed Characters

Only immortals see polymorphed players on the who list:

```cpp
if (canSeeWho(p) && IS_SET(p->specials.act, ACT_POLYSELF)) {
  buf = format("%s (polymorphed)\n\r") % sstring(pers(p)).cap();
}
```

### Disguised Thieves

Only immortals see disguised players:

```cpp
if (canSeeWho(p) && IS_SET(p->specials.act, ACT_DISGUISED)) {
  buf = format("%s (disguised thief)\n\r") % sstring(pers(p)).cap();
}
```

### Switched Immortals

Immortals who have switched into mobs appear with "(switched)" notation to other immortals.

## Client Integration

**Source:** `code/code/sys/client.cc:1323-1365`

The `fixClientPlayerLists()` function updates graphical clients when visibility changes:

```cpp
void TBeing::fixClientPlayerLists(bool lost);
```

Called when:
- Player logs in/out
- Invisibility level changes
- Player enters/leaves linkdead state

## Common Gotchas

1. **invisLevel vs AFF_INVISIBLE**: These are completely different systems. `invisLevel` is level-based masking (primarily for immortals), while `AFF_INVISIBLE` is a spell affect that can be detected.

2. **Linkdead detection**: Mortals cannot see linkdead players even if they could see them before disconnection.

3. **canSeeWho vs canSee**: A player visible on the who list may not be visible when looking in a room, and vice versa. These are separate checks.

4. **Anonymous vs Invisible**: Anonymous only hides class/level info, not the player's presence. Invisible hides the player entirely.

5. **Shadow Walk lighting**: AFF_SHADOW_WALK only provides concealment in rooms with light level < 14. Check room lighting when testing.

6. **Client synchronization**: Always call `fixClientPlayerLists()` when changing visibility to keep graphical clients in sync.

## Code References

| File | Lines | Description |
|------|-------|-------------|
| `code/code/cmd/cmd_who.cc` | 140-588 | doWho() implementation |
| `code/code/cmd/cmd_who.cc` | 590-615 | doWhozone() implementation |
| `code/code/misc/utility.cc` | 63-93 | canSeeWho() |
| `code/code/misc/utility.cc` | 832-923 | canSee()/canSeeMe() for TBeing |
| `code/code/misc/utility.cc` | 983-1040 | can_see_char_other_room() |
| `code/code/misc/being.cc` | 1081-1092 | getInvisLevel()/setInvisLevel() |
| `code/code/misc/toggle.cc` | 787-806 | invis toggle handling |
| `code/code/misc/player_data.cc` | 1701-1703 | isLinkdead() |
| `code/code/sys/client.cc` | 1323-1365 | fixClientPlayerLists() |
| `code/code/misc/defs.h` | 48-50 | MAX_MORT, GOD_LEVEL1, MAX_IMMORT |
| `code/code/misc/defs.h` | 85-110 | AFF_* visibility flags |
| `code/code/misc/toggle.h` | 58, 82 | PLR_STEALTH, PLR_ANONYMOUS |

## Related Documentation

- [Affects System](affects-system.md) - Details on AFF_* flags and affect management
- [Wizard Powers](wizard-powers.md) - Immortal access control
- [Snoop and Switch](snoop-switch.md) - Related immortal observation systems
- [Class Hierarchy](class-hierarchy.md) - TBeing/TPerson/TMonster relationships
