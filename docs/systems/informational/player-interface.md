---
title: Player Interface Systems
description: Prompt configuration, score display, toggle preferences, help system, and GMCP protocol for player information presentation.
category: informational
keywords: [prompt configuration, score display, terminal settings, help search, GMCP protocol]
primary_symbols:
  functions: [doPrompt, doScore, doToggle, doHelp, doColor, buildHelpIndex, hasColorVt]
  classes: [promptData, TDescriptor]
  enums: [PROMPT_HIT, PROMPT_MANA, PROMPT_MOVE, PROMPT_GOLD, PROMPT_EXP, PROMPT_PIETY, PROMPT_LIFEFORCE, PROMPT_OPPONENT, PROMPT_TANK, PROMPT_TANK_OTHER, PROMPT_ROOM, PROMPT_ROOM_NAME, PROMPT_ZONE_NUM, PROMPT_COORDS, PROMPT_EXPTONEXT_LEVEL, PROMPT_TIME, PROMPT_CR, PROMPT_VTANSI_BAR, PROMPT_CLASSIC_ANSIBAR, PROMPT_CLIENT_PROMPT, PROMPT_BUILDER_ASSISTANT, AUTO_NOSPAM, AUTO_EAT, AUTO_KILL, AUTO_LOOT_MONEY, AUTO_LOOT_NOTMONEY, AUTO_NOHARM, AUTO_NOSHOUT, AUTO_PG13, AUTO_AFK, AUTO_SPLIT, AUTO_POUCH, AUTO_TROPHY, AUTO_TIPS, AUTO_JOIN, AUTO_DISSECT, AUTO_ENGAGE, AUTO_ENGAGE_ALWAYS, AUTO_HUNT, AUTO_NOSPELL, AUTO_HALFSPELL, AUTO_LIMBS, AUTO_NOSPRITE, AUTO_NOTELL, AUTO_AUTOGROUP, AUTO_MAP, AUTO_MAPTAGS, AUTO_SPELLTASK, PLR_BRIEF, PLR_COMPACT, PLR_SHOW_SAVES, PLR_DENY_LOOT, PLR_NEWBIEHELP, PLR_ANONYMOUS]
---

## Overview

How does a player know their character's current state? How do they customize what information they see and when? The player interface systems handle all presentation of game state to the player and provide extensive customization of that presentation.

SneezyMUD's player interface consists of four interconnected systems:

**Prompt System** - A customizable status line displayed after each command, showing real-time character information like hit points, mana, movement, and combat status. Players control exactly which elements appear and how they're colored.

**Score Command** - A comprehensive character status display showing vitals, wealth, progression, conditions, and active effects in a human-readable format.

**Toggle System** - Player preferences that affect gameplay behavior (auto-loot, nospam, wimpy fleeing) and terminal settings (screen size, color mode, ANSI support). These persist across sessions.

**Help System** - Indexed documentation accessible in-game, organized by category (general, skills, spells, immortal, builder) with abbreviation support and search priority ordering.

These systems work together to give players control over their experience. The prompt provides moment-to-moment awareness during gameplay. Score gives a detailed snapshot on demand. Toggles let players tune behavior to their preferences. Help provides self-service documentation.

For clients supporting GMCP (Generic MUD Communication Protocol), the server also sends structured data packets that allow modern MUD clients to render custom UI elements like health bars and minimaps.

## Patterns

### Prompt Configuration

**Enable prompt elements individually rather than using `prompt all`.** The `prompt all` command enables every option, which creates visual clutter. Most players need only hit points, mana/piety/lifeforce (class-appropriate), movement, and combat information.

**Use `tank-other` instead of `tank` for group play.** The `tank` option shows tank status including yourself, which is redundant when you're the tank. The `tank-other` option only displays when someone else is tanking, providing more useful information.

**Enable the VT100/ANSI status bar only after setting terminal type.** The status bar requires `toggle terminal ansi` (or `vt100`) first. Without proper terminal settings, the bar displays garbage characters or fails silently.

**Display current settings when running `prompt` with no arguments.** This prevents players from forgetting what they've enabled and allows easy review of configuration.

### Toggle Management

**Check `IS_SET(desc->autobits, AUTO_*)` for player preference toggles.** The autobits field stores player preferences as a bitmask. Never access these flags directly on the character - they're stored on the descriptor.

**Check `isPlayerAction(PLR_*)` for player action flags.** These flags (like `PLR_BRIEF`) are stored differently from autobits and require the accessor method.

**Respect wimpy trait restrictions before modifying wimpy settings.** Characters with `TOG_IS_COWARD` cannot disable wimpy. Characters with `TOG_IS_VICIOUS` cannot enable it. Characters with `TOG_IS_CRAVEN` must maintain minimum wimpy. Check these traits before allowing wimpy changes.

**Never allow anonymous toggle below level 5.** The `PLR_ANONYMOUS` flag requires minimum level 5 to prevent newbies from hiding their level during the critical early game period.

### Terminal Detection

**Use `hasColorVt()` to check for VT100 or ANSI support.** This method returns true if either terminal type is enabled, useful for any VT escape sequence support.

**Use `ansi()` specifically for ANSI color support.** ANSI is a superset of VT100, so some features are ANSI-specific.

**Use `vt100()` for VT100-only features.** Some terminal features work on VT100 but not on dumb terminals.

**Provide `color off` for players who need plain text.** Some players have medical conditions that make color distracting, and some terminals render colors poorly.

### Help System Usage

**Be aware of abbreviation matching.** Help topics match by prefix, so `help arm` might match `armor` before `armadillo`. When writing code that programmatically queries help, use exact topic names.

**Check help search priority for visibility.** Immortal help is checked first (if player has `POWER_IMMORTAL_HELP`), then builder help (if builder level), then general, spell, and skill help. A topic in immortal help shadows the same topic in general help.

### GMCP Integration

**Send GMCP updates only to clients that negotiated GMCP.** Check that the descriptor supports GMCP before sending packets. GMCP-unaware clients will display the raw data as garbage.

**Keep GMCP packets small and frequent rather than large and infrequent.** Send `char.vitals` after any HP/mana/move change. Send `room.info` on room entry. This allows clients to maintain responsive UIs.

**Cache previous values to avoid redundant GMCP packets.** Only send updates when data actually changes. Sending the same `char.vitals` packet every combat round wastes bandwidth.

**Include complete data in each packet.** Don't send partial updates. Each `char.vitals` packet should contain all vitals, not just the ones that changed. Clients don't maintain state across packets.

### Score Display

**Handle condition value -1 as immunity, not extreme hunger/thirst.** The value -1 indicates immunity (typically immortals). Check for -1 before displaying condition messages to avoid showing misleading "totally famished" to immortal characters.

**Format large numbers with thousands separators.** Display experience as "1,234,567" not "1234567". Talens, bank balance, and skill totals should all use separators for readability.

## Reference

### Symbol Quick Reference

| Symbol | Type | Purpose |
|--------|------|---------|
| `doPrompt()` | function | Process prompt command, generate prompt output |
| `doScore()` | function | Display comprehensive character status |
| `doToggle()` | function | Manage player and global toggles |
| `doHelp()` | function | Search and display help topics |
| `doColor()` | function | Manage color settings and replacement |
| `buildHelpIndex()` | function | Scan and index help files at startup |
| `hasColorVt()` | function | Check VT100/ANSI terminal support |
| `promptData` | class | Stores prompt configuration and flags |
| `TDescriptor` | class | Player connection, holds autobits and prompt data |

### Prompt Flags

| Flag | Code | Display |
|------|------|---------|
| `PROMPT_HIT` | `hit` | `H:500` |
| `PROMPT_MANA` | `mana` | `M:350` |
| `PROMPT_MOVE` | `movement` | `V:120` |
| `PROMPT_GOLD` | `talens` | `T:5000` |
| `PROMPT_EXP` | `exp` | `E:1,234,567` |
| `PROMPT_PIETY` | `piety` | `P:85.5` |
| `PROMPT_LIFEFORCE` | `lifeforce` | `LF:200` |
| `PROMPT_OPPONENT` | `opponent` | Combat target info |
| `PROMPT_TANK` | `tank` | Tank status (self included) |
| `PROMPT_TANK_OTHER` | `tank-other` | Tank status (self excluded) |
| `PROMPT_ROOM` | `room` | `R:557` |
| `PROMPT_ROOM_NAME` | `roomname` | Room name text |
| `PROMPT_ZONE_NUM` | `zone` | Zone number |
| `PROMPT_COORDS` | `coordinates` | `(10,20,0)` |
| `PROMPT_EXPTONEXT_LEVEL` | `exp_tolevel` | `N:50,000` |
| `PROMPT_TIME` | `time` | `t:14:30:25` |
| `PROMPT_CR` | `cr` | Trailing carriage return |
| `PROMPT_VTANSI_BAR` | `bar` | VT100/ANSI status bar |
| `PROMPT_CLASSIC_ANSIBAR` | `classic-ansi-bar` | Classic bar layout |
| `PROMPT_CLIENT_PROMPT` | `client-prompt` | Client protocol codes |
| `PROMPT_BUILDER_ASSISTANT` | `builder_assistant` | Builder mode (immortals) |

### Player Toggles (autobits)

| Flag | Toggle | Effect |
|------|--------|--------|
| `AUTO_NOSPAM` | `nospam` | Hide combat misses and spam |
| `AUTO_EAT` | `autoeat` | Auto eat/drink when hungry/thirsty |
| `AUTO_KILL` | `autokill` | Continue attacking stunned creatures |
| `AUTO_LOOT_MONEY` | `loot-money` | Auto-loot money from corpses |
| `AUTO_LOOT_NOTMONEY` | `loot-all` | Auto-loot everything from corpses |
| `AUTO_NOHARM` | `noharm` | Prevent attacking other players |
| `AUTO_NOSHOUT` | `noshout` | Block shout channel |
| `AUTO_PG13` | `noPG13` | Block vulgar language |
| `AUTO_AFK` | `afk` | Auto-AFK message when idle |
| `AUTO_SPLIT` | `split` | Auto-split gold with group |
| `AUTO_POUCH` | `pouch` | Auto-open money pouches |
| `AUTO_TROPHY` | `trophy` | Show trophy after kills |
| `AUTO_TIPS` | `tips` | Show periodic gameplay tips |
| `AUTO_JOIN` | `join` | Allow faction admission |
| `AUTO_DISSECT` | `dissect` | Auto-dissect corpses |
| `AUTO_ENGAGE` | `engage` | Engage instead of fight when casting |
| `AUTO_ENGAGE_ALWAYS` | `engage-all` | Always engage instead of fight |
| `AUTO_HUNT` | `hunt` | Auto-move toward tracked targets |
| `AUTO_NOSPELL` | `nospells` | Show only first/last spell messages |
| `AUTO_HALFSPELL` | `halfspells` | Show half of spell messages randomly |
| `AUTO_LIMBS` | `limbs` | Show tank limb status after fights |
| `AUTO_NOSPRITE` | `no-hero-sprites` | Disable hero sprites |
| `AUTO_NOTELL` | `notell` | Block incoming tells |
| `AUTO_AUTOGROUP` | `autogroup` | Auto-group new followers |
| `AUTO_MAP` | `map` | Enable automap display |
| `AUTO_MAPTAGS` | `maptags` | Show map location tags |
| `AUTO_SPELLTASK` | `spelltask` | Show spelltask in prompt |

### Player Action Flags (plr_act)

| Flag | Toggle | Effect |
|------|--------|--------|
| `PLR_BRIEF` | `brief` | Short room descriptions |
| `PLR_COMPACT` | `compact` | Compact output mode |
| `PLR_SHOW_SAVES` | `showsaves` | Show save notifications |
| `PLR_DENY_LOOT` | `deny-corpse-loot` | Prevent others looting your corpse |
| `PLR_NEWBIEHELP` | `newbiehelper` | Mark as available to help newbies |
| `PLR_ANONYMOUS` | `anonymous` | Hide level in who list (level 5+) |

### Global Toggles (Admin)

| Toggle | Effect |
|--------|--------|
| `shouting` | Enable/disable shout channel |
| `sleep` | Enable/disable sleep command |
| `newbiepk` | Newbie PK protection |
| `gravity` | Physics simulation |
| `clients` | Allow client connections |
| `wizbuild` | Builder mode |
| `mobnames` | Mobile name display |
| `dbtiming` | Database timing logs |
| `doubleexp` | Double experience mode |
| `dblogging` | Database query logging |
| `testcode1-5` | Test code toggles |
| `questcode1-4` | Quest code toggles |

### Immortal Toggles

| Toggle | Effect |
|--------|--------|
| `invisibility` | Set invisible to specified level |
| `stealth` | Silent movement mode |
| `nohassle` | Mobs don't attack |
| `immortal` | Toggle immortal/mortal mode |
| `success` | Automatic skill success/failure |

### Condition Thresholds

| Condition | Level | Message |
|-----------|-------|---------|
| Hunger | -1 | (immune - no message) |
| Hunger | 0 | "You are totally famished." (red) |
| Hunger | 1-5 | "Your stomach is growling loudly." |
| Hunger | 6-10 | "You could use a little bite to eat." |
| Hunger | 11-20 | "You are slightly hungry." |
| Hunger | 21+ | "Your hunger is the least of your worries." |
| Thirst | -1 | (immune - no message) |
| Thirst | 0 | "You are totally parched." (red) |
| Thirst | 1-5 | "Your throat is very dry." |
| Thirst | 6-10 | "You could use a little drink." |
| Thirst | 11-20 | "You are slightly thirsty." |
| Thirst | 21+ | "Your thirst is the least of your worries." |
| Drunk | 0 | (no message) |
| Drunk | 1-3 | "You are feeling tipsy." |
| Drunk | 4-9 | "You are intoxicated." |
| Drunk | 10-14 | "You are drunk." |
| Drunk | 15-19 | "You are very drunk." |
| Drunk | 20+ | "You are VERY drunk." |

### Help Categories

| Category | Path | Priority |
|----------|------|----------|
| Immortal | `lib/help/_immortal` | 1 (highest, requires `POWER_IMMORTAL_HELP`) |
| Builder | `lib/help/_builder` | 2 (requires builder level) |
| General | `lib/help/` | 3 |
| Spells | `lib/help/_spells` | 4 |
| Skills | `lib/help/_skills` | 5 (lowest) |

### GMCP Packages

| Package | Content |
|---------|---------|
| `char.vitals` | HP, mana, moves, piety, lifeforce |
| `char.maxstats` | Maximum HP, mana, moves |
| `char.status` | XP, level, talens, hunger, thirst |
| `char.position` | Position, fighting status |
| `room.info` | Room number, name, exits, coordinates |
| `room.area` | Zone information |
| `room.mobs` | Creatures in room |

### Prompt Colors

| Type | Colors |
|------|--------|
| Basic | `blue`, `red`, `green`, `white`, `purple`, `cyan`, `orange`, `yellow`, `charcoal` |
| Bold | `boldred`, `boldgreen`, `boldblue`, `boldpurple`, `boldcyan` |
| Background | `white_on_blue`, `white_on_red`, `white_on_purple`, `white_on_green`, `white_on_yellow`, `white_on_cyan` |
| Special | `invert`, `blinking` |

### Key Files

| File | Contents |
|------|----------|
| `code/code/sys/color.cc` | `doPrompt()`, `doColor()`, prompt generation |
| `code/code/sys/connect.h` | Prompt flags, `promptData` class |
| `code/code/cmd/cmd_score.cc` | `doScore()` implementation |
| `code/code/misc/toggle.cc` | `doToggle()`, autobits handling |
| `code/code/misc/toggle.h` | Toggle constants and flags |
| `code/code/cmd/cmd_help.cc` | `doHelp()`, help index building |
| `code/code/misc/help.h` | Help system declarations |
| `code/code/sys/gmcphandlers.cc` | GMCP functions |

## Implementation

### Prompt Generation

The prompt is generated after each command completes. The system reads the player's `desc->prompt_d.type` bitmask to determine which elements to include.

For each enabled flag, the corresponding data is retrieved from the character and formatted. Hit points come from `getHit()` and `hitLimit()`. Mana comes from `getMana()` and `manaLimit()`. The system checks class to determine whether to show mana, piety, or lifeforce - clerics and deikhans show piety, shamans show lifeforce, mages/monks/psionicists show mana.

Combat-related elements (opponent, tank) only appear when the character is fighting. The opponent display shows the target's name and health percentage. Tank display shows whoever is taking hits for the group.

The VT100/ANSI status bar uses terminal escape sequences to create a persistent display area at the bottom of the screen. This reserves 3-4 lines that don't scroll with normal output. Line 1 shows vitals, line 2 shows room/affiliation/wealth, and line 3 (modern layout) shows additional info like TNL. The bar requires `desc->screen_size` to match the actual terminal height - mismatches cause the bar to appear in the wrong location or overwrite normal output.

Prompt colors are stored per-element. The `prompt color <stat> <color>` command updates the color for a specific prompt element. These colors are applied during prompt generation using ANSI escape codes (when the terminal supports them).

### Score Display

The `doScore()` function constructs a multi-section character summary. It queries the character for all relevant stats and formats them with appropriate descriptions.

Vitals display is class-aware - the function checks character class to determine whether to show mana, piety, or lifeforce. Movement is described using `DescMoves()` which converts the percentage of current/max moves to a descriptive string ranging from "totally exhausted" to "completely rested."

Wealth shows current talens, bank balance (queried from the database), and experience. Session tracking shows XP earned since login (computed from a baseline set at login) and playtime for the current session and lifetime total.

Condition display uses threshold tables to convert numeric hunger/thirst/drunk values to descriptive messages. Zero values use red color codes to draw attention to critical states. The special value -1 indicates immunity and produces no condition message.

Position and combat mode are shown at the bottom. Active affects are listed if any exist.

### Toggle Storage and Processing

Player toggles are stored in two places: `desc->autobits` for most gameplay preferences, and character flags for player action flags.

The `doToggle()` function parses the toggle name and maps it to the appropriate storage location and flag constant. For autobits, it uses `SET_BIT()` and `REMOVE_BIT()` to flip flags. For player actions, it uses `setPlayerAction()` and `remPlayerAction()`.

Terminal settings (screensize, terminal type) are stored on the descriptor and persisted to the account. The screensize value is bounded to 1-128 lines.

Wimpy is stored as an integer threshold. When checking wimpy, the system compares current HP to `desc->wimpy` and triggers automatic flee if HP drops below. The `maxWimpy()` function calculates the maximum allowed wimpy (typically half max HP). Wimpy handling validates trait restrictions through `hasQuestBit()` - `TOG_IS_COWARD` prevents disabling wimpy below a minimum, `TOG_IS_VICIOUS` prevents enabling wimpy at all, `TOG_IS_CRAVEN` enforces a minimum wimpy value.

Global toggles require `POWER_TOGGLE` and affect server-wide state. These are stored in global variables and affect all players.

### Help Index and Search

At server startup, `buildHelpIndex()` scans all help directories and builds an in-memory index. Each directory is scanned recursively. Files ending in `.ansi` are excluded from the index - these are auto-selected variants for color terminals.

When a player requests help, `doHelp()` searches the index in priority order: immortal help first (if authorized), then builder help (if builder level), then general, spell, and skill help. The first match wins.

Abbreviation matching allows partial topic names. The system compares the query as a prefix against all indexed topics in the current category before moving to the next category.

For skill and spell help, additional metadata is injected before the help file content. This includes discipline, specialization, learning rate, modifier stat, spell component, difficulty, immunity type, casting requirements, and behavioral flags (offensive, area effect, etc.).

Help files with corresponding `.ansi` versions are selected based on terminal capability. If the player has ANSI color enabled and an `.ansi` version exists, that version is displayed instead of the plain text version. Long help files automatically paginate based on `desc->screen_size`.

### Color System

Color settings are stored per-player and control which output categories receive color codes. Categories include communications, objects, mobiles, rooms, room names, shouts, and spells.

The color replacement system allows players to substitute colors they find difficult to see. A player can designate a "replace" color and a "substitute" color. All instances of the replace color in output are converted to the substitute color before display.

The `doColor()` command manages these settings. `color test` displays all available colors so players can see which work on their terminal. `color all` enables all categories, `color none` disables all.

### GMCP Protocol

For clients that negotiate GMCP during connection, the server sends structured JSON packets alongside normal text output. These packets allow clients to build custom UI elements.

The `char.vitals` package is sent after any change to HP, mana, moves, piety, or lifeforce. It includes both current and maximum values so clients can render percentage bars.

The `room.info` package is sent on room entry and includes room vnum, name, exits, and coordinates. This enables automap features in clients.

GMCP packets are sent through a separate code path that checks for GMCP capability before transmitting. Non-GMCP clients never see this data. The system caches previous values and compares current values before sending - only actual changes trigger transmission to avoid redundant packets.

## Troubleshooting

### VT100/ANSI Bar Shows Garbage

**Symptom:** Player enables `prompt bar` and sees control characters or broken formatting instead of a status bar.

**Likely cause:** Terminal type not set correctly.

**Diagnostic approach:** Check `toggle` output for terminal setting. Should be `ansi` or `vt100`.

**Fix:** Have player run `toggle terminal ansi` before enabling the bar.

### Status Bar Appears in Wrong Location

**Symptom:** Status bar appears in the middle of the screen or overwrites normal output.

**Likely cause:** Screen size mismatch between server setting and actual terminal height.

**Diagnostic approach:** Check `desc->screen_size` versus actual terminal height. The status bar requires the bottom 3-4 lines.

**Fix:** Set correct screen size with `toggle screensize <height>`. Disable status bar with `prompt bar` if sizing issues persist.

### Help Topic Returns Wrong Content

**Symptom:** Player requests `help <topic>` and gets unexpected content.

**Likely cause:** Abbreviation matched a different topic, or higher-priority category shadowed the topic.

**Diagnostic approach:** Check if the topic exists in multiple help directories. Check if the query is a prefix of multiple topics.

**Fix:** Use the full topic name. For shadowed topics, consider renaming one of the files or documenting the ambiguity.

### Help Returns "No Help Available"

**Symptom:** Help command returns "No help available" for a topic that should exist.

**Likely cause:** Help index not built at startup, or help file permissions prevent reading.

**Diagnostic approach:** Verify `buildHelpIndex()` ran at startup. Check that help files exist and are readable by the server process.

**Fix:** Restart server to rebuild help index. Verify help directory paths and file permissions.

### Wimpy Won't Enable/Disable

**Symptom:** Player tries to change wimpy setting but it doesn't take effect.

**Likely cause:** Character has a trait restricting wimpy modification.

**Diagnostic approach:** Check character for `TOG_IS_COWARD` (can't disable), `TOG_IS_VICIOUS` (can't enable), or `TOG_IS_CRAVEN` (minimum required).

**Fix:** This is intentional behavior based on character traits. The trait must be removed to allow the change.

### Anonymous Toggle Fails

**Symptom:** Player can't enable anonymous mode.

**Likely cause:** Character is below level 5.

**Diagnostic approach:** Check character level with `score`.

**Fix:** Anonymous requires level 5+. This is intentional to prevent newbie level hiding.

### Anonymous Toggle Fails Despite Level 5+

**Symptom:** Player is level 5 or higher but anonymous toggle is rejected.

**Likely cause:** Level check uses current level, not base level. Polymorphed or level-drained characters may fail the check.

**Diagnostic approach:** Verify actual character level versus displayed level. Check for polymorph, level drain, or temporary level modifications.

**Fix:** Ensure character is in normal form with true level. Temporary level changes can prevent anonymous mode until reverted.

### Tank/Tank-Other Conflict

**Symptom:** Player enables both tank options but only one appears in prompt.

**Likely cause:** The options are mutually exclusive - only one can be active.

**Diagnostic approach:** Check prompt settings.

**Fix:** Choose one or the other. `tank` includes self in display, `tank-other` excludes self. For most group play, `tank-other` is more useful.

### GMCP Data Not Received by Client

**Symptom:** Modern client not receiving GMCP packets despite expecting them.

**Likely cause:** GMCP negotiation failed during connection, or client doesn't have GMCP enabled.

**Diagnostic approach:** Check telnet option negotiation logs. Verify client has GMCP enabled in its settings. Test with a known-good GMCP client.

**Fix:** Ensure client has GMCP explicitly enabled. Some clients require manual GMCP activation. Verify server GMCP module is compiled and active.

### Toggle Settings Don't Persist

**Symptom:** Toggle settings reset to defaults on each login.

**Likely cause:** Database save failure or descriptor not properly associated with character.

**Diagnostic approach:** Check database write permissions. Verify `doQueueSave()` is called after toggle changes. Check for database errors in logs.

**Fix:** Ensure database connectivity. Verify toggle changes trigger save functions. Check file system permissions for database files.

### Color Replacement Not Working

**Symptom:** Player set color replacement but specific color still appears unchanged.

**Likely cause:** Replacement mapping not set correctly, or color name mismatch.

**Diagnostic approach:** Check color replacement settings with `color` command. Verify exact color names match expected values. Use `color test` to see actual colors.

**Fix:** Set both replacement and substitute colors explicitly using exact color names from the color command list.
