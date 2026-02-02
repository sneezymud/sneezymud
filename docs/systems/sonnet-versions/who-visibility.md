---
title: Who List and Visibility System
category: important
keywords: [canSeeWho, doWho, invisLevel, PLR_ANONYMOUS, PLR_STEALTH, AFF_INVISIBLE, AFF_SHADOW_WALK, AFF_DETECT_INVISIBLE, isLinkdead, fixClientPlayerLists]
related: [affects-system.md, wizard-powers.md, snoop-switch.md, class-hierarchy.md]
primary_symbols:
  functions: [canSeeWho, canSeeMe, can_see_char_other_room, doWho, doWhozone, fixClientPlayerLists, isLinkdead, getInvisLevel, setInvisLevel]
  classes: [TBeing]
  files: [code/code/cmd/cmd_who.cc, code/code/misc/utility.cc, code/code/misc/being.cc, code/code/misc/toggle.cc, code/code/misc/player_data.cc, code/code/sys/client.cc]
---

## Overview

The who command displays online players, but visibility depends on multiple layered systems. Three distinct visibility mechanisms interact to determine if one character can see another:

**Who List Visibility** determines which players appear on the global who command output. Controlled by the canSeeWho function, this system evaluates invisibility levels, spell-based invisibility, and viewer permissions.

**Room Visibility** determines whether characters can see each other when in the same location. The canSeeMe function incorporates room lighting, infravision, environmental factors, and stealth mechanics beyond what the who list considers.

**Remote Visibility** determines whether characters can be detected across room boundaries. The can_see_char_other_room function handles scrying, remote sensing, and long-range detection abilities.

These systems are independent. A character may be visible on the who list but hidden in a room, or vice versa. The linkdead state demonstrates this layering: linkdead characters automatically receive invisLevel GOD_LEVEL1, hiding them from mortal who lists while remaining present in their current room.

Two fundamentally different invisibility systems exist. The invisLevel field provides level-based masking, primarily used for immortal observation without player awareness. Spell-based affects like AFF_INVISIBLE provide magical concealment that can be detected and countered. These systems stack independently and serve different gameplay purposes.

## Patterns

### Who Command Usage

The basic who command lists all visible players with their titles. Optional flags filter and display additional information. Syntax supports name searches, level ranges, and boolean filters.

Basic patterns include showing all visible players, searching by partial name match, listing players at or above a specific level, and listing players between two level boundaries. The flag system allows combining multiple filters with a single dash prefix.

Race filters use single-letter codes: e for elf, t for hobbit, n for gnome, u for human, r for ogre, w for dwarf. Class filters use numeric codes 1 through 8 for Mage, Cleric, Warrior, Thief, Deikhan, Monk, Ranger, and Shaman respectively.

Display modifiers include showing levels and class with -l, showing questers with -q, gods with -g, builders with -b, mortals with -o, players seeking groups with -z, group structure with -p, ungrouped players with -y, and faction information with -f. The -x flag shows Perma Death characters, while -! shows Fae-touched characters.

Immortal-only flags reveal additional information. The -i flag shows idle time, -h shows HP/mana/move/money, -d shows linkdead players, and -s shows all stats. Wizard-level access enables -a to show account names.

The whozone command provides immortals with zone-specific player listings including room locations. This command requires level 51 or higher and restricts output to players in the viewer's current zone.

### Invisibility Level Management

The invisLevel field stores an integer from 0 to MAX_IMMORT. Setting invisLevel to 0 makes the character fully visible to all players. Values from 1 to 50 hide the character from players below that level threshold. Values at or above GOD_LEVEL1 hide the character from all mortals.

The invis toggle command without arguments toggles between invisLevel 0 and 51. With a numeric argument, it sets invisLevel to the specified value. This allows fine-grained control over which immortal ranks can observe the character.

Linkdead state automatically sets invisLevel to GOD_LEVEL1. When the player reconnects, invisLevel resets to 0. This mechanism prevents mortals from seeing disconnected characters on the who list while allowing immortals to track and assist them.

### Stealth Mode Behavior

The PLR_STEALTH flag suppresses movement and action messages from being shown to mortals. When combined with elevated invisLevel, this provides silent observation capability. Immortals with PLR_STEALTH active use MAX_MORT as the visibility threshold for action messages, preventing messages like teleportation or polymorph effects from being visible to mortal observers.

### Anonymous Mode Behavior

The PLR_ANONYMOUS flag hides class and level information from the who list display without hiding the player's presence. Anonymous players appear on the who list but show Anonymous instead of level and class. Level-based filters and class-based filters do not match anonymous players. Immortals always see through anonymous mode.

### Spell-Based Invisibility Interaction

AFF_INVISIBLE provides standard magical concealment blocked by AFF_DETECT_INVISIBLE. AFF_SHADOW_WALK provides shadow-based concealment that only functions in rooms with light level below 14. Both affects are countered by AFF_DETECT_INVISIBLE.

AFF_TRUE_SIGHT and AFF_CLARITY provide comprehensive detection, seeing through both spell invisibility and environmental concealment. Blind viewers cannot see any characters unless they have AFF_TRUE_SIGHT or AFF_CLARITY, regardless of target invisibility state.

Immortals with spell invisibility remain hidden from the who list even from other immortals, unless the viewer has detection affects. This allows immortal-level concealment when desired.

### Client Synchronization

Graphical clients maintain local player lists that must be synchronized when visibility changes. The fixClientPlayerLists function broadcasts visibility updates when players log in or out, when invisibility levels change, and when players enter or leave linkdead state.

Failing to call fixClientPlayerLists after visibility changes creates client desynchronization where the graphical client shows stale player lists. This function iterates all connected descriptors and updates their client state.

## Reference

### Who List Visibility Algorithm

The canSeeWho function implements a decision tree evaluating viewer and target properties. Null or invalid room conditions immediately return false. Immortal viewers can see any target with invisLevel less than or equal to the viewer's level. Targets with invisLevel higher than the viewer's maximum level return false for any viewer.

Mortal viewers cannot see targets with invisLevel at or above GOD_LEVEL1. This threshold includes linkdead players and invisible immortals.

Spell invisibility evaluation checks if the target has AFF_INVISIBLE, or has AFF_SHADOW_WALK in a room with light level below 14. If the target is immortal, return false. If the viewer lacks AFF_DETECT_INVISIBLE, return false.

Blind viewers cannot see targets unless the viewer has AFF_TRUE_SIGHT or AFF_CLARITY.

### Room Visibility Algorithm

The canSeeMe function extends who list visibility with environmental factors. In addition to invisLevel and spell invisibility checks, it evaluates room lighting level, infravision for temperature-based detection, sneak and hide states, weather conditions in arctic and tropical climates, and environmental modifiers.

### Constants

MAX_MORT defines the maximum mortal level as 50. GOD_LEVEL1 defines the first immortal level as 51. MAX_IMMORT defines the maximum immortal level as 60. These constants determine invisibility thresholds and permission boundaries.

### Special Display Cases

Polymorphed characters appear on the who list only to immortals, with the notation polymorphed appended to their perceived name. Disguised thieves appear only to immortals with the notation disguised thief. Switched immortals appear to other immortals with the notation switched.

### Linkdead Detection

A player is linkdead when isPc returns true, desc is null, and polyed equals POLY_TYPE_NONE. Linkdead characters have invisLevel automatically set to GOD_LEVEL1, hiding them from mortal who lists. Immortals see linkdead players with bracket notation around the player name. The -d flag for the who command shows linkdead players to immortals.

## Implementation

### canSeeWho Function Structure

The canSeeWho function in utility.cc performs null and invalid room checks first, returning false if either being has invalid room state or if the target or target room pointer is null. It retrieves the target room's light level for later evaluation.

For immortal viewers, if the viewer's maximum level is less than the target's invisLevel, return false. Otherwise return true. This short-circuits further checks for immortal viewers who can see the target.

For any viewer, if the viewer's maximum level is less than the target's invisLevel and the target is immortal, return false. This handles mortal viewers seeing invisible immortals.

For mortal viewers, if the target's invisLevel is at or above GOD_LEVEL1, return false. This catches linkdead players.

Spell invisibility evaluation checks if the target has AFF_INVISIBLE or if room illumination is below 14 and the target has AFF_SHADOW_WALK. If true, check if target is immortal and return false. Check if viewer lacks AFF_DETECT_INVISIBLE and return false.

Finally, if the viewer is blind and lacks AFF_TRUE_SIGHT and AFF_CLARITY, return false.

If all checks pass, return true.

### doWho Function Structure

The doWho function in cmd_who.cc parses command arguments to determine filtering flags, level ranges, and name searches. It builds a set of filter criteria from the dash-prefixed flags, extracting race filters, class filters, and display modifiers.

The function iterates all descriptors, checking each connected character against the filter criteria. For each character, it calls canSeeWho to determine base visibility. Additional filters check anonymous mode, polymorph state, disguise state, and switch state.

Characters matching all filter criteria are accumulated into a display buffer. After iterating all characters, the function outputs the formatted who list with headers and footers including total player count, maximum players since reboot, and average player count.

### doWhozone Function Structure

The doWhozone function in cmd_who.cc restricts output to immortals and iterates descriptors looking for characters in the same zone as the viewer. For each matching character, it displays the character name and room location. This provides zone-focused player tracking for area management and immortal assistance.

### setInvisLevel Function Behavior

The setInvisLevel function in being.cc assigns the invisLevel field and calls fixClientPlayerLists to synchronize client state. It accepts any short integer value but typical usage constrains values to 0 through MAX_IMMORT. Setting negative values is unsupported and may produce undefined behavior.

### fixClientPlayerLists Function Behavior

The fixClientPlayerLists function in client.cc accepts a boolean parameter indicating whether visibility was lost. It iterates all game descriptors and sends player list update packets to graphical clients. The lost parameter determines whether to send a remove-player or add-player message.

This function ensures client-side player lists remain synchronized with server-side visibility state. Without this synchronization, clients display stale player lists that don't reflect invisibility changes.

## Troubleshooting

### Invisible Player Still Appears on Who List

Verify invisLevel is set correctly using getInvisLevel. Confirm the viewer's maximum level is lower than the target's invisLevel. Check that fixClientPlayerLists was called after changing invisLevel.

For spell-based invisibility, verify the target has AFF_INVISIBLE affect applied. Check room lighting if using AFF_SHADOW_WALK, as it only functions in rooms with light level below 14. Confirm the viewer lacks AFF_DETECT_INVISIBLE.

### Linkdead Player Not Hidden

Confirm the player is actually linkdead using isLinkdead. Verify desc is null and polyed equals POLY_TYPE_NONE. Check that invisLevel was automatically set to GOD_LEVEL1 when the connection dropped.

### Anonymous Player Shows Class Information

Confirm PLR_ANONYMOUS is set in the player flags. Verify the viewer is mortal, as immortals always see through anonymous mode. Check that the who command implementation correctly checks for PLR_ANONYMOUS before displaying class and level.

### Immortal Cannot See Another Immortal

Verify the viewer's maximum level is greater than or equal to the target's invisLevel. Check if the target has spell invisibility applied, as immortals with AFF_INVISIBLE remain hidden even from other immortals unless the viewer has AFF_DETECT_INVISIBLE.

### Client Shows Wrong Player List

Verify fixClientPlayerLists is called when visibility changes. Check that all code paths modifying invisLevel, affect flags, or connection state invoke fixClientPlayerLists. Reconnecting the client forces a full resynchronization if the server-side state is correct.

### canSeeWho Returns True But Player Not Visible in Room

Remember that canSeeWho and canSeeMe use different algorithms. Check room lighting level, as canSeeMe factors environmental visibility. Verify sneak and hide states if the target is using stealth abilities. Confirm infravision and temperature-based detection if the viewer has limited vision.
