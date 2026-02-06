---
title: Who List and Visibility System
description: Character visibility across who lists, rooms, and zones. Three systems interact to determine detection.
keywords: [who list visibility, room visibility, remote visibility, client synchronization, stealth mode, anonymous mode]
category: important
source_files: [code/code/cmd/cmd_who.cc, code/code/misc/utility.cc, code/code/misc/being.cc, code/code/misc/toggle.cc, code/code/misc/player_data.cc, code/code/sys/client.cc]
primary_symbols:
  functions: [canSeeWho, canSee, canSeeMe, setInvisLevel, fixClientPlayerLists, isLinkdead, doWho, doWhozone]
  classes: [TBeing, Descriptor]
  enums: [MAX_MORT, GOD_LEVEL1, MAX_IMMORT, AFF_INVISIBLE, AFF_SHADOW_WALK, AFF_DETECT_INVISIBLE, AFF_TRUE_SIGHT, AFF_CLARITY, PLR_STEALTH, PLR_ANONYMOUS, POLY_TYPE_NONE]
---

# Who List and Visibility System

## Overview

Three independent visibility systems determine whether one character can detect another. Each applies different rules, meaning a character visible in one context may be hidden in another.

**Who List Visibility** governs the `who` command output. It checks invisibility levels, spell affects, and viewer permissions but ignores room-specific factors like lighting or hiding. Implemented in `canSeeWho()`.

**Room Visibility** handles in-room detection. Beyond invisibility, it considers lighting, infravision, sneak/hide states, and environmental modifiers. Implemented in `canSee()` and `canSeeMe()`.

**Remote Visibility** extends detection across rooms, used by spells and abilities that reveal distant characters. Implemented in `can_see_char_other_room()`.

A player might appear on the who list while remaining invisible in a room, or be detectable in-room but hidden from who. The systems share some mechanics but evaluate them independently.

## Patterns

Always distinguish between invisibility level and spell invisibility. The `invisLevel` field provides level-based masking for immortals. Spell affects like `AFF_INVISIBLE` grant temporary concealment that detection affects can pierce.

Always call `fixClientPlayerLists()` when visibility state changes. Graphical clients maintain their own player lists and must be notified when invisibility toggles, players connect or disconnect, or linkdead state changes.

Always check `canSeeWho()` separately from `canSee()`. Passing one check does not guarantee passing the other. Code that reveals player information must use the appropriate check for the context.

Never assume anonymous mode provides concealment. `PLR_ANONYMOUS` only hides class and level information from the who list. The player remains fully visible and detectable.

Never assume shadow walk always conceals. `AFF_SHADOW_WALK` only provides concealment when room illumination falls below 14. Always consider lighting when testing shadow-based visibility.

Always set `invisLevel` to `GOD_LEVEL1` when a player becomes linkdead. This hides them from mortal who lists while remaining visible to immortals. Reset to 0 on reconnection.

Always respect the visibility threshold for stealth immortals. When `PLR_STEALTH` is set, use `MAX_MORT` as the visibility threshold for action messages to prevent mortals from observing the immortal's activities.

Always use `setInvisLevel()` rather than directly modifying `invisLevel`. The setter sends `CLIENT_INVIS` messages to the character's descriptor. Call `fixClientPlayerLists()` separately afterward to synchronize other clients' player lists.

## Reference

### Who Command Syntax

| Form | Description |
|------|-------------|
| `who` | List all visible players |
| `who -?` | Display help for flags |
| `who <name>` | Search by name |
| `who <level>` | List at or above level |
| `who <L1> <L2>` | List between levels |
| `who -<flags>` | Apply filters |
| `whozone` | List zone players with rooms (immortal) |

### Who Flags

| Flag | Filter | Access |
|------|--------|--------|
| `-l` | Show levels and class | All |
| `-q` | Questers only | All |
| `-g` | Gods only | All |
| `-b` | Builders only | All |
| `-o` | Mortals only | All |
| `-z` | Seeking group | All |
| `-p` | Group leaders/members | All |
| `-y` | Ungrouped only | All |
| `-f` | Faction/guild info | All |
| `-e` | Elf race | All |
| `-t` | Hobbit race | All |
| `-n` | Gnome race | All |
| `-u` | Human race | All |
| `-r` | Ogre race | All |
| `-w` | Dwarf race | All |
| `-1` to `-8` | Class (Mage, Cleric, Warrior, Thief, Deikhan, Monk, Ranger, Shaman) | All |
| `-x` | Perma Death characters | All |
| `-!` | Fae-touched characters | All |
| `-i` | Idle time | Immortal |
| `-h` | HP/mana/move/money | Immortal |
| `-d` | Link-dead players | Immortal |
| `-s` | All stats | Immortal |
| `-a` | Account names | Wizard |

### Level Constants

| Constant | Value | Meaning |
|----------|-------|---------|
| `MAX_MORT` | 50 | Maximum mortal level |
| `GOD_LEVEL1` | 51 | First immortal level |
| `MAX_IMMORT` | 60 | Maximum immortal level |

### Invisibility Level Values

| Value | Effect |
|-------|--------|
| 0 | Fully visible |
| 1-50 | Hidden from players below this level |
| 51+ | Hidden from all mortals (linkdead, invis gods) |

### Visibility Affects

| Affect | Effect |
|--------|--------|
| `AFF_INVISIBLE` | Standard spell invisibility |
| `AFF_SHADOW_WALK` | Invisible in low light (illumination < 14) |
| `AFF_DETECT_INVISIBLE` | Pierce AFF_INVISIBLE and AFF_SHADOW_WALK |
| `AFF_TRUE_SIGHT` | Pierce all invisibility and illusions |
| `AFF_CLARITY` | Functions as TRUE_SIGHT for visibility |

### Player Flags

| Flag | Effect |
|------|--------|
| `PLR_STEALTH` | Suppress action messages from mortals |
| `PLR_ANONYMOUS` | Hide class/level on who list |

### Special Who Display Cases

| Condition | Display | Viewer |
|-----------|---------|--------|
| Polymorphed | "(polymorphed)" suffix | Immortal only |
| Disguised | "(disguised thief)" suffix | Immortal only |
| Switched | "(switched)" suffix | Immortal only |
| Linkdead | Bracketed name | Immortal only |

## Implementation

### Who List Visibility Algorithm

The `canSeeWho()` function in `utility.cc` determines who list visibility through a series of checks.

First, basic validity is confirmed. If either character has an invalid room, visibility fails.

For immortal viewers, visibility depends solely on invisibility level comparison. An immortal can see anyone with `invisLevel` less than or equal to their own level.

For mortal viewers examining immortal targets, if the target's `invisLevel` meets or exceeds `GOD_LEVEL1`, visibility fails. This covers linkdead players and invisible gods.

Spell invisibility is then evaluated for mortal viewers. If the target has `AFF_INVISIBLE`, or has `AFF_SHADOW_WALK` in a room with illumination below 14, the viewer needs `AFF_DETECT_INVISIBLE` to see them. Immortal viewers never reach the spell invisibility check because they return early after the invisLevel comparison, so immortals with `AFF_INVISIBLE` or `AFF_SHADOW_WALK` are visible to other immortals on the who list regardless of detection affects.

Finally, blind viewers cannot see unless they have `AFF_TRUE_SIGHT` or `AFF_CLARITY`.

### Room Visibility Algorithm

The `canSee()` and `canSeeMe()` functions in `utility.cc` handle room-based visibility with additional environmental factors.

Beyond the checks in who list visibility, room visibility considers:
- Ambient room lighting levels
- Infravision (temperature-based detection via `infraTypeT` parameter)
- Active sneak and hide states
- Weather conditions (arctic, tropical environments)
- Various environmental modifiers

This makes room visibility more situational than who list visibility.

### Linkdead State Management

A player becomes linkdead when their network connection drops while their character remains in the game world. The `isLinkdead()` function in `player_data.cc` checks three conditions: the character must be a PC (`isPc()` returns true), must lack a descriptor (`desc` is null), and must not be polymorphed (`polyed` equals `POLY_TYPE_NONE`).

When linkdead state activates, `invisLevel` is set to `GOD_LEVEL1` to hide the character from mortal who lists. Immortals see linkdead players with bracketed names and can list them explicitly with `who -d`. Upon reconnection, `invisLevel` resets to 0.

### Invis Toggle Command

The `invis` toggle in `toggle.cc` controls immortal invisibility. Without arguments, it toggles between 0 (visible) and 51 (invisible to mortals). With a level argument, it sets invisibility to that specific level, hiding the immortal from anyone below that level.

### setInvisLevel Function

The `setInvisLevel()` function in `being.cc` assigns the `invisLevel` field and sends `CLIENT_INVIS` messages to the character's own descriptor. It does not call `fixClientPlayerLists()` directly; the invis toggle command in `toggle.cc` calls `fixClientPlayerLists()` separately after calling `setInvisLevel()`. It accepts any short integer value but typical usage constrains values to 0 through `MAX_IMMORT`. Setting negative values is unsupported and may produce undefined behavior.

### Client Synchronization

The `fixClientPlayerLists()` function in `client.cc` notifies graphical clients when visibility state changes. It accepts a boolean parameter indicating whether visibility was lost, determining whether to send remove-player or add-player messages.

This function must be called when players log in or out, when invisibility level changes, and when linkdead state transitions occur. Failure to call this function leaves client-side player lists out of sync with the server.

### Whozone Implementation

The `doWhozone()` function in `cmd_who.cc` is an immortal-only command (level 51+) that lists all players in the current zone along with their room locations. This provides spatial awareness beyond the standard who list.

### doWho Implementation

The `doWho()` function in `cmd_who.cc` parses command arguments to determine filtering flags, level ranges, and name searches. It builds filter criteria from dash-prefixed flags, then iterates all descriptors checking each connected character against the criteria. Characters matching all filters are accumulated into a display buffer. Output includes headers and footers with total player count, maximum players since reboot, and average player count.

## Troubleshooting

**Symptom:** Player appears on who list but cannot be seen in room

**Cause:** Room visibility applies additional checks (lighting, sneak/hide, infravision) that who list visibility does not.

**Diagnostic:** Compare `canSeeWho()` result to `canSee()` result. Check room lighting level and target's hide/sneak state.

**Fix:** These systems are independent by design. If the discrepancy is undesired, adjust the specific factor (lighting, remove hide, etc.) rather than conflating the systems.

---

**Symptom:** Graphical client shows stale player list after visibility changes

**Cause:** `fixClientPlayerLists()` was not called when visibility state changed.

**Diagnostic:** Verify visibility-changing code paths call the synchronization function.

**Fix:** Add `fixClientPlayerLists()` call after any code that modifies `invisLevel` or causes login/logout/linkdead transitions. Reconnecting the client forces full resynchronization if server-side state is correct.

---

**Symptom:** Shadow walking character visible in dark room

**Cause:** Room illumination is at or above 14, or viewer has `AFF_DETECT_INVISIBLE`.

**Diagnostic:** Check room's `getLight()` return value. Check viewer's affect flags.

**Fix:** Shadow walk requires illumination below 14. Move to a darker room or ensure lighting conditions are met.

---

**Symptom:** Anonymous player matched by level/class who filter

**Cause:** Viewer is an immortal. Immortals see through `PLR_ANONYMOUS`.

**Diagnostic:** Verify viewer's level.

**Fix:** This is correct behavior. Anonymous mode only hides information from mortals.

---

**Symptom:** Mortal seeing immortal actions despite PLR_STEALTH

**Cause:** The specific action message does not check for stealth, or the visibility threshold was not applied.

**Diagnostic:** Review the message-sending code for stealth consideration.

**Fix:** Action messages for stealthy immortals should use `MAX_MORT` as the visibility threshold.

---

**Symptom:** Linkdead player visible on mortal who list

**Cause:** `invisLevel` was not set to `GOD_LEVEL1` when linkdead state activated.

**Diagnostic:** Check the character's `invisLevel` value and `isLinkdead()` return. Verify `desc` is null and `polyed` equals `POLY_TYPE_NONE`.

**Fix:** Ensure linkdead transition code sets `invisLevel = GOD_LEVEL1`.

---

**Symptom:** Invisible player still appears on who list

**Cause:** Either `invisLevel` is not set correctly, or spell invisibility is being countered.

**Diagnostic:** Use `getInvisLevel()` to verify the invisLevel value. For spell invisibility, verify `AFF_INVISIBLE` is applied and check if viewer has `AFF_DETECT_INVISIBLE`.

**Fix:** Ensure `setInvisLevel()` was called (not direct assignment) so client synchronization occurs. Confirm the viewer's level is below the target's `invisLevel`.

---

**Symptom:** Immortal cannot see another immortal on who list

**Cause:** The target immortal has a higher `invisLevel` than the viewer's level.

**Diagnostic:** Verify viewer's level versus target's `invisLevel`. Note that spell invisibility (`AFF_INVISIBLE`, `AFF_SHADOW_WALK`) does not affect immortal-to-immortal who list visibility because immortal viewers return early after the invisLevel check.

**Fix:** The target must lower their `invisLevel` to be at or below the viewer's level, or the viewer must use a higher-level character.
