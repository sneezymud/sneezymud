---
title: Player Interface Systems
category: understanding
keywords: [prompt, score, toggle, help, autobits, color, GMCP, terminal, VT100, ANSI, nospam, noharm, wimpy, doPrompt, doScore, doHelp]
related: [communication-system.md, network-architecture.md, player-interface.md]
primary_symbols:
  functions: [doPrompt, doScore, doToggle, doHelp, buildHelpIndex, hasColorVt, ansi, vt100]
  classes: [promptData, Descriptor]
  files: [code/code/sys/color.cc, code/code/cmd/cmd_score.cc, code/code/misc/toggle.cc, code/code/cmd/cmd_help.cc, code/code/sys/comm.cc]
---

## Overview

How does a player know they are running out of health? What options do they have to customize what they see? How do they access documentation?

SneezyMUD provides a comprehensive player interface system that handles real-time status display, character information, behavior preferences, and documentation access. This system determines how players perceive and interact with the game world.

The interface encompasses four core components:

**Prompt System:** A customizable status line displaying vital statistics, combat information, and location data. Updated after every command, the prompt provides at-a-glance awareness of character state. Players control which statistics appear, their colors, and whether to use modern client protocols or classic VT100/ANSI status bars.

**Score Command:** A comprehensive character sheet showing hit points, mana or class-specific resources, wealth, experience progression, skill totals, playtime, hunger and thirst levels, position, and active affects. This provides detailed information beyond what fits in the prompt.

**Toggle System:** A preference engine controlling over 30 behavioral flags ranging from combat automation (auto-loot, auto-split gold) to display preferences (brief mode, nospam) to safety features (noharm prevents attacking other players). These toggles persist across sessions and affect both what players see and how commands behave.

**Help System:** An indexed documentation system with separate categories for general topics, skills, spells, immortal commands, and builder tools. Help files are plain text on disk, with optional ANSI-colorized versions for color terminals. Skill and spell help includes metadata about discipline, requirements, and mechanical properties.

The interface adapts to client capabilities. Terminals supporting VT100 or ANSI receive a persistent status bar at the bottom of the screen. Clients supporting GMCP (Generic MUD Communication Protocol) receive structured JSON data for richer UI integration. Players without color support see plain text.

Players configure their interface through the prompt, toggle, and color commands. All settings persist in the database and roam across connections. The system balances information density with readability - players can enable every possible prompt element for maximum data, or strip down to minimal output for focused play.

## Patterns

### Prompt Customization

**Always provide a way to view current settings.** Running `prompt` without arguments displays all available options and their current state. This prevents players from forgetting what they've enabled.

**Never enable all options by default.** Information overload degrades usability. Default prompts show hit points, mana or class resource, movement, and experience. Players opt into additional details.

**Always validate terminal capabilities before enabling advanced features.** Check `hasColorVt()` before enabling VT100/ANSI status bars. Attempting to use status bars on incompatible terminals produces garbled output.

**Never allow conflicting prompt options simultaneously.** The `tank` and `tank-other` options are mutually exclusive - enabling one disables the other. This prevents redundant display.

### Toggle Safety

**Always check autobits before performing automated actions.** Before auto-looting corpses, verify `IS_SET(desc->autobits, AUTO_LOOT_MONEY)`. Performing unwanted automation frustrates players.

**Never allow PK without explicit player consent.** The `AUTO_NOHARM` flag prevents accidentally attacking other players. This flag should only be disabled through deliberate player action, never automatically.

**Always respect trait-imposed toggle restrictions.** Characters with `TOG_IS_COWARD` cannot disable wimpy. Characters with `TOG_IS_VICIOUS` cannot enable wimpy. These restrictions come from character background and must be enforced in `doToggle()`.

**Never bypass minimum level requirements for sensitive toggles.** Anonymous mode requires level 5 to prevent abuse by new characters. Immortal toggles require appropriate power flags.

### Help System Access

**Always search help categories in priority order.** Immortals with `POWER_IMMORTAL_HELP` should see immortal help first, then builder help if applicable, then general help. This ensures privileged commands are documented before general commands with the same name.

**Never require exact topic matches.** Support abbreviations - `help com` should match `combat`. This reduces friction for players who know roughly what they're looking for.

**Always provide metadata for skills and spells.** Beyond the help text, display discipline, learn rate, modifier stat, requirements, and mechanical properties. This information is critical for player decisions about character development.

**Never index ANSI color variants separately.** Files ending in `.ansi` are alternate versions of the base help file, automatically selected for color-capable terminals. Only index the base filename.

### Terminal Compatibility

**Always detect terminal type before sending control codes.** Check `ansi()` before sending ANSI color codes. Check `vt100()` before sending cursor positioning sequences. Sending control codes to incompatible terminals produces visible garbage.

**Never assume all players want color.** Provide `color off` to disable all color codes. Some players have medical conditions that make color distracting, and some terminals render colors poorly.

**Always provide a text-only fallback for all features.** Every colored element must have a plain text version. Every GMCP data packet must also update the prompt. Not all clients support modern protocols.

**Never reserve screen real estate without player consent.** The VT100/ANSI status bar reserves the bottom 3-4 lines of the terminal. This must be opt-in via `prompt bar`, not automatically enabled.

### Score Display

**Always show class-appropriate resources.** Warriors see hit points and movement. Mages see mana. Clerics see piety. Shamans see lifeforce. Don't show irrelevant statistics.

**Never display misleading condition values.** Condition value -1 means immunity (typically immortals), not extreme hunger/thirst. Check for -1 before displaying condition messages.

**Always format large numbers for readability.** Display experience as "1,234,567" not "1234567". Talens, bank balance, and skill totals should all use thousands separators.

**Never omit session statistics.** Show both session playtime and lifetime total. Show both current XP and XP earned this session. Players track progression over individual gaming sessions.

### GMCP Integration

**Always send GMCP updates when vitals change.** When hit points, mana, or movement change, send `char.vitals`. When the player moves, send `room.info`. Modern clients rely on this data to update their UI in real time.

**Never send redundant GMCP packets.** Cache previous values and only send updates when data actually changes. Sending the same `char.vitals` packet every combat round wastes bandwidth.

**Always include complete data in each packet.** Don't send partial updates. Each `char.vitals` packet should contain all vitals, not just the ones that changed. Clients don't maintain state across packets.

## Reference

### Symbol Quick Reference

| Symbol | Type | Purpose |
|--------|------|---------|
| `doPrompt()` | function | Generate and send prompt to player |
| `doScore()` | function | Display comprehensive character status |
| `doToggle()` | function | Manage player preference flags |
| `doHelp()` | function | Display help topic or index |
| `buildHelpIndex()` | function | Scan help directories and build search index |
| `hasColorVt()` | function | Check if terminal supports VT100 or ANSI |
| `ansi()` | function | Check if terminal supports ANSI color |
| `vt100()` | function | Check if terminal supports VT100 codes |
| `promptData` | class | Stores prompt configuration flags and colors |
| `Descriptor` | class | Network connection with terminal settings and autobits |

### Prompt Flags

| Flag | Purpose |
|------|---------|
| `PROMPT_HIT` | Show current hit points |
| `PROMPT_MANA` | Show current mana |
| `PROMPT_MOVE` | Show movement points |
| `PROMPT_GOLD` | Show talens on hand |
| `PROMPT_EXP` | Show total experience |
| `PROMPT_PIETY` | Show piety (clerics/deikhans) |
| `PROMPT_LIFEFORCE` | Show lifeforce (shamans) |
| `PROMPT_OPPONENT` | Show combat opponent status |
| `PROMPT_TANK` | Show tank status (including self) |
| `PROMPT_TANK_OTHER` | Show tank status (excluding self) |
| `PROMPT_ROOM` | Show room vnum |
| `PROMPT_ROOM_NAME` | Show room name |
| `PROMPT_ZONE_NUM` | Show zone number |
| `PROMPT_COORDS` | Show x,y,z coordinates |
| `PROMPT_EXPTONEXT_LEVEL` | Show XP needed for next level |
| `PROMPT_TIME` | Show real-world time |
| `PROMPT_CR` | Add carriage return after prompt |
| `PROMPT_VTANSI_BAR` | Enable VT100/ANSI status bar |
| `PROMPT_CLASSIC_ANSIBAR` | Use classic bar layout |
| `PROMPT_CLIENT_PROMPT` | Enable client protocol codes |
| `PROMPT_BUILDER_ASSISTANT` | Builder mode prompt (immortals) |

### Toggle Flags (autobits)

| Flag | Purpose |
|------|---------|
| `AUTO_NOSPAM` | Hide combat misses and repetitive messages |
| `AUTO_EAT` | Automatically eat/drink when hungry/thirsty |
| `AUTO_KILL` | Continue attacking stunned creatures |
| `AUTO_LOOT_MONEY` | Auto-loot money from corpses |
| `AUTO_LOOT_NOTMONEY` | Auto-loot items from corpses |
| `AUTO_NOHARM` | Prevent attacking other players |
| `AUTO_NOSHOUT` | Block shout channel |
| `AUTO_PG13` | Block vulgar language |
| `AUTO_AFK` | Auto-AFK message when idle |
| `AUTO_SPLIT` | Auto-split gold with group |
| `AUTO_POUCH` | Auto-open money pouches |
| `AUTO_TROPHY` | Show trophy after kills |
| `AUTO_TIPS` | Show periodic gameplay tips |
| `AUTO_JOIN` | Allow faction admission |
| `AUTO_DISSECT` | Auto-dissect corpses |
| `AUTO_ENGAGE` | Engage instead of fight when casting |
| `AUTO_ENGAGE_ALWAYS` | Always engage instead of fight |
| `AUTO_HUNT` | Auto-move toward tracked targets |
| `AUTO_NOSPELL` | Show only first/last spell messages |
| `AUTO_HALFSPELL` | Show half of spell messages randomly |
| `AUTO_LIMBS` | Show tank limb status after fights |
| `AUTO_NOSPRITE` | Disable hero sprites |
| `AUTO_NOTELL` | Block incoming tells |
| `AUTO_AUTOGROUP` | Auto-group new followers |
| `AUTO_MAP` | Enable automap display |
| `AUTO_MAPTAGS` | Show map location tags |
| `AUTO_SPELLTASK` | Show spelltask in prompt |

### Player Action Flags

| Flag | Purpose |
|------|---------|
| `PLR_BRIEF` | Short room descriptions |
| `PLR_COMPACT` | Compact output mode |
| `PLR_SHOW_SAVES` | Show save notifications |
| `PLR_DENY_LOOT` | Prevent others from looting corpse |
| `PLR_NEWBIEHELP` | Available to help newbies |
| `PLR_ANONYMOUS` | Hide level in who list (requires level 5) |

### Help Categories

| Category | Directory | Access Level |
|----------|-----------|--------------|
| Immortal | `lib/help_immortal/` | `POWER_IMMORTAL_HELP` |
| Builder | `lib/help_builder/` | Builder level |
| General | `lib/help/` | All players |
| Spells | `lib/help_spells/` | All players |
| Skills | `lib/help_skills/` | All players |

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

### Condition Thresholds

| Range | Hunger Message | Thirst Message |
|-------|----------------|----------------|
| -1 | (immune) | (immune) |
| 0 | "You are totally famished." | "You are totally parched." |
| 1-5 | "Your stomach is growling loudly." | "Your throat is very dry." |
| 6-10 | "You could use a little bite to eat." | "You could use a little drink." |
| 11-20 | "You are slightly hungry." | "You are slightly thirsty." |
| 21+ | "Your hunger is the least of your worries." | "Your thirst is the least of your worries." |

### Key Files

| File | Primary Content |
|------|-----------------|
| `code/code/sys/color.cc` | Prompt generation, color commands, status bar rendering |
| `code/code/sys/connect.h` | Prompt flags, promptData class definition |
| `code/code/cmd/cmd_score.cc` | Score command implementation, condition descriptions |
| `code/code/misc/toggle.cc` | Toggle command, wimpy handling, global toggles |
| `code/code/misc/toggle.h` | Toggle flag constants, trait restrictions |
| `code/code/cmd/cmd_help.cc` | Help search, index building, metadata display |
| `code/code/misc/help.h` | Help system declarations |
| `code/code/sys/comm.cc` | GMCP packet generation and transmission |

## Implementation

### Prompt Generation Flow

The prompt system generates status lines based on flags stored in `desc->prompt_d.type`. When a command completes, the server calls `doPrompt()` which builds a string from enabled components.

For basic prompts, each enabled flag triggers inclusion of a specific statistic with its prefix. Hit points appear as `H:500`, mana as `M:350`, movement as `V:120`. Colors come from `desc->prompt_d.color[PROMPT_*]` arrays, allowing per-element customization.

Combat-specific elements (opponent, tank, tank-other) only appear when the player is fighting. These show the target's condition or the current tank's status. Tank and tank-other are mutually exclusive - `doToggle()` clears one when enabling the other.

The VT100/ANSI status bar takes a different path. When `PROMPT_VTANSI_BAR` is set, `doPrompt()` sends cursor positioning codes to place text at the bottom of the terminal. Line 1 shows vitals, line 2 shows wealth and experience, line 3 (in modern layout) shows additional statistics. The bar persists across commands because the cursor is repositioned above it before normal output.

Color application happens through terminal capability detection. `hasColorVt()` checks both `desc->terminal` flags and client capabilities. If color is supported and enabled, the prompt wraps elements in ANSI escape codes. If not, plain text is sent.

GMCP-capable clients receive parallel updates. When the prompt is generated, if the client supports GMCP, `sendGmcpVitals()` and related functions send JSON packets with current statistics. This allows graphical clients to display bars, gauges, and custom UI elements synchronized with the text prompt.

### Score Command Components

`doScore()` assembles information from multiple character subsystems. Vitals come directly from `getHit()`, `getMana()` or class-specific resource methods, and `getMove()`. The display adapts based on class - warriors don't see mana, mages don't see piety, shamans see lifeforce instead of mana.

Wealth calculation combines `getMoney()` for cash on hand with database queries for bank balance. Experience calls `getExp()` and `getExpLevel()` to show current total and requirement for next level. Session experience tracking maintains a baseline set at login and computes the difference.

Condition descriptions use threshold ranges. Movement condition comes from `DescMoves()`, which maps move percentage to fatigue descriptions. Hunger, thirst, and intoxication check `getCond()` against predefined ranges. The special value -1 indicates immunity and is handled separately.

Playtime comes from two sources. Session duration is computed from login timestamp to current time. Lifetime total comes from the database `played` field, which accumulates across all sessions. Both are formatted as hours and minutes.

Position, combat mode, and wimpy setting come from character state flags. Active affects are enumerated by iterating `affected` list elements and formatting each affect type.

The score output is constructed as a single formatted string and sent via `sendTo()`. Color codes are embedded if the player's terminal supports them.

### Toggle System Architecture

Toggle management operates on three flag fields: `desc->autobits` for behavioral preferences, character action flags for display modes, and global server flags for administrative settings.

The `doToggle()` function parses the toggle name, determines which flag field to modify, and validates the request. For autobits, it sets or clears the appropriate `AUTO_*` flag. For player actions, it calls `setPlayerAction()` or `remPlayerAction()`. For global toggles, it checks for `POWER_TOGGLE` and modifies server state.

Wimpy handling is more complex because character traits impose restrictions. `TOG_IS_COWARD` prevents disabling wimpy below a minimum. `TOG_IS_VICIOUS` prevents enabling wimpy at all. `TOG_IS_CRAVEN` enforces a minimum wimpy value. `doToggle()` checks these traits through `hasQuestBit()` before allowing wimpy changes.

Maximum wimpy is computed by `maxWimpy()`, typically half of maximum hit points. Attempting to set wimpy above this threshold clamps it to the maximum. Setting wimpy to 0 disables it unless trait restrictions apply.

Anonymous mode requires level 5 to prevent abuse by disposable characters. Terminal type changes validate the requested type (ansi, vt100, none) before applying. Screen size is clamped to the range 1-128 to prevent display issues.

Global toggles affect server-wide behavior. Double experience, newbie PK protection, and database logging can be enabled or disabled by administrators. These settings are stored in global state variables and checked throughout the codebase when relevant operations occur.

### Help System Indexing and Search

At startup, `buildHelpIndex()` scans five directories: immortal help, builder help, general help, spell help, and skill help. For each directory, it opens and reads filenames, excluding files ending in `.ansi`. Each filename becomes a searchable topic.

The index is stored in memory as a collection of help file entries, each recording the filename, category, and last modified time. This allows fast lookup without disk access on every help request.

When a player requests help, `doHelp()` searches categories in priority order. Immortals with `POWER_IMMORTAL_HELP` check immortal help first. Builders check builder help next. All players check general, spell, and skill help.

Abbreviation matching uses prefix comparison. If the player types `help com`, the system finds the first topic starting with "com" in the current category. If multiple topics match, the first alphabetically is selected. This can lead to unexpected matches when abbreviations are ambiguous.

For skills and spells, help display includes metadata lookup. The system calls `getDisciplineNumber()` to find the discipline, then queries skill/spell properties including learn rate, modifier stat, requirements, and mechanical flags. This metadata is formatted above the help text.

ANSI-colorized help files are automatically selected when the player's terminal supports color. The system checks for a `.ansi` version of the requested file and sends it if the player has `hasColorVt()` and color enabled. Otherwise, the plain text version is sent.

Help text is sent through `sendTo()` with paging if the text exceeds screen size. Long help files automatically paginate based on `desc->screen_size`.

### Color System Operation

Color management spans several layers. At the lowest level, `hasColorVt()` checks terminal capabilities and player preferences. If both are positive, color codes are embedded in output.

The `doColor()` command modifies player color preferences. Enabling color sets terminal flags and updates the descriptor. Disabling color clears flags and strips color from subsequent output. Individual color categories (rooms, objects, mobiles, communications, spells) can be toggled independently.

Color replacement handles accessibility. Players who find certain colors difficult to see can map them to alternatives. The replacement mapping is stored per descriptor and applied during output generation.

Prompt coloring uses per-element color arrays. Each prompt component (hit, mana, movement, etc.) has an associated color index. When the prompt is built, the appropriate color codes wrap each element. This allows red hit points, blue mana, and green movement in the same prompt.

The status bar color system differs because it uses absolute cursor positioning. Color codes must be carefully placed to avoid disrupting the layout. The bar rendering code tracks cursor position and sends positioning codes between colored segments.

ANSI escape codes follow the format `\033[<params>m` where params specify foreground color, background color, bold, blinking, and invert. The color system constructs these codes based on player preferences and terminal capabilities.

### GMCP Data Synchronization

GMCP packets are JSON-formatted messages sent to clients supporting the protocol. When vitals change, `sendGmcpVitals()` constructs a JSON object containing current HP, max HP, mana, max mana, and movement, then sends it via the descriptor.

Room information updates occur on movement. `sendGmcpRoom()` includes room vnum, room name, zone, coordinates, and exit list. Exit data includes direction, destination, and flags like closed doors.

Character status packets include experience, level, talens, bank balance, hunger, and thirst. Position packets include current position (standing, sitting, resting) and combat status.

The system avoids redundant packets by checking whether values actually changed. A flag on the descriptor tracks the last sent values. Before sending a GMCP packet, the system compares current values to cached values. Only differences trigger transmission.

GMCP is negotiated during connection setup through telnet option negotiation. If the client supports GMCP, the descriptor is flagged, and subsequent game events trigger GMCP updates. If the client doesn't support GMCP, these calls become no-ops.

## Troubleshooting

### Symptom: Prompt shows garbled characters or strange symbols

**Likely cause:** Terminal type mismatch - server sending VT100/ANSI codes to incompatible terminal.

**Diagnostic approach:** Check `desc->terminal` flags. Verify player's actual terminal emulator. Test with `toggle terminal none` to disable all control codes.

**Fix:** Set appropriate terminal type with `toggle terminal <type>`. Use `none` for basic terminals, `ansi` for color support, `vt100` for full control code support.

### Symptom: Status bar appears in middle of screen or overwrites normal output

**Likely cause:** Screen size mismatch between server and client terminal.

**Diagnostic approach:** Check `desc->screen_size` versus actual terminal height. The status bar requires the bottom 3-4 lines, so screen_size must match reality.

**Fix:** Set correct screen size with `toggle screensize <height>`. Disable status bar with `prompt bar` if sizing issues persist.

### Symptom: Toggle settings don't persist across logins

**Likely cause:** Database save failure or descriptor not associated with character.

**Diagnostic approach:** Check database write permissions. Verify `doQueueSave()` is called after toggle changes. Check for database errors in logs.

**Fix:** Ensure database connectivity. Verify toggle changes call save functions. Check file system permissions for database files.

### Symptom: Help command returns "No help available" for valid topics

**Likely cause:** Help index not built, or help file permissions prevent reading.

**Diagnostic approach:** Check if `buildHelpIndex()` ran at startup. Verify help files exist in expected directories. Check file permissions on help directories.

**Fix:** Restart server to rebuild help index. Verify help files are readable by server process. Check that help directory paths are correct in configuration.

### Symptom: GMCP data not received by client

**Likely cause:** GMCP negotiation failed or client doesn't support protocol.

**Diagnostic approach:** Check telnet option negotiation logs. Verify client actually supports GMCP. Test with known-good GMCP client.

**Fix:** Ensure client has GMCP enabled in settings. Some clients require explicit GMCP activation. Check that server GMCP module is compiled and active.

### Symptom: Wimpy won't disable despite valid toggle command

**Likely cause:** Character has trait restricting wimpy settings.

**Diagnostic approach:** Check for `TOG_IS_COWARD`, `TOG_IS_VICIOUS`, or `TOG_IS_CRAVEN` quest bits. These traits enforce wimpy restrictions.

**Fix:** Trait-imposed restrictions are intentional character limitations. If restriction is incorrect, remove the quest bit through immortal commands. Otherwise, wimpy restrictions are working as designed.

### Symptom: Color replacement not working for specific color

**Likely cause:** Replacement mapping not set, or color name mismatch.

**Diagnostic approach:** Check color replacement settings with `color` command. Verify exact color names match expected values. Test with `color test` to see actual colors.

**Fix:** Set both replacement and substitute colors explicitly. Use exact color names from the color command list. Some colors require terminal capability checks.

### Symptom: Anonymous toggle rejected despite being level 5+

**Likely cause:** Level check uses current level, not base level. Polymorphed or level-drained characters may fail check.

**Diagnostic approach:** Verify actual character level versus displayed level. Check for polymorph, level drain, or temporary level modifications.

**Fix:** Ensure character is in normal form with true level visible. Temporary level changes can prevent anonymous mode until reverted.
