---
title: Player Interface Systems
description: Player-facing UI systems in SneezyMUD including customizable prompt, score display, toggles, and help system.
keywords: [prompt, promptData, PROMPT_HIT, PROMPT_MANA, doPrompt, doScore, toggle, autobits, AUTO_NOSPAM, AUTO_NOHARM, PLR_BRIEF, wimpy, maxWimpy, doHelp, buildHelpIndex, GMCP, char.vitals, room.info, color, hasColorVt, VT100, ANSI]
category: Understanding Systems
related:
  - configuration-reference.md
  - stats-attributes.md
  - combat-rounds.md
  - logging-patterns.md
last_updated: 2026-01-29
source_files:
  - code/code/sys/color.cc
  - code/code/sys/connect.h
  - code/code/cmd/cmd_score.cc
  - code/code/misc/toggle.cc
  - code/code/misc/toggle.h
  - code/code/cmd/cmd_help.cc
  - code/code/misc/help.h
  - code/code/sys/comm.cc
---
# Player Interface Systems

This document describes the player-facing UI systems in SneezyMUD, including the prompt system, score command, player toggles, and the help system.

## Overview

SneezyMUD provides extensive customization for how players receive information. The interface is built around:

- **Prompt System:** Customizable status line showing vital stats, combat info, and location
- **Score Command:** Comprehensive character status display
- **Toggle System:** Player preferences for gameplay behavior
- **Help System:** Indexed help files across multiple categories

## Prompt System

The prompt is the status line displayed after each command, showing real-time character information. Players can fully customize what appears in their prompt.

### Prompt Command

The `prompt` command (without arguments) shows current settings:

```
Prompt Line Options:
--------------------
Hit        : (yes): H:500
Piety      : ( no): P:85.5
Lifeforce  : ( no): LF:200
Mana       : (yes): M:350
Movement   : (yes): V:120
Talens     : ( no): T:5000
Exp        : (yes): E:1,234,567
Time       : ( no): t:14:30:25
Exp_tolevel: ( no): N:50,000
Room       : ( no): R:557
Coordinate : ( no): (10,20,0)
--------------------
Opponent  : ( no): Current target when in battle.
Tank      : ( no): Current tank when in battle, including self.
Tank-other: (yes): Current tank when in battle, excluding self.
--------------------
Classic Bar   : Off
Client Prompts: Off
Carriage Return: On
```

**Source:** `code/code/sys/color.cc:18-168`

### Prompt Codes

| Code | Displays | Example |
|------|----------|---------|
| `H:` | Current hit points | `H:500` |
| `M:` | Current mana (mages/monks/psionicists) | `M:350` |
| `V:` | Movement points | `V:120` |
| `T:` | Talens (money) | `T:5000` |
| `E:` | Total experience | `E:1,234,567` |
| `P:` | Piety (clerics/deikhans) | `P:85.5` |
| `LF:` | Lifeforce (shamans) | `LF:200` |
| `N:` | Experience to next level | `N:50,000` |
| `R:` | Room vnum | `R:557` |
| `t:` | Current time | `t:14:30:25` |

### Available Prompt Options

| Option | Bit Flag | Description |
|--------|----------|-------------|
| `hit` | `PROMPT_HIT` | Show current HP |
| `mana` | `PROMPT_MANA` | Show current mana |
| `movement` | `PROMPT_MOVE` | Show movement points |
| `talens` | `PROMPT_GOLD` | Show money on hand |
| `exp` | `PROMPT_EXP` | Show total experience |
| `piety` | `PROMPT_PIETY` | Show piety (class-specific) |
| `lifeforce` | `PROMPT_LIFEFORCE` | Show lifeforce (shamans) |
| `opponent` | `PROMPT_OPPONENT` | Show combat opponent info |
| `tank` | `PROMPT_TANK` | Show tank status (self included) |
| `tank-other` | `PROMPT_TANK_OTHER` | Show tank status (self excluded) |
| `room` | `PROMPT_ROOM` | Show room vnum |
| `roomname` | `PROMPT_ROOM_NAME` | Show room name |
| `zone` | `PROMPT_ZONE_NUM` | Show zone number |
| `coordinates` | `PROMPT_COORDS` | Show x,y,z coordinates |
| `exp_tolevel` | `PROMPT_EXPTONEXT_LEVEL` | Show XP needed to level |
| `time` | `PROMPT_TIME` | Show real-world time |
| `cr` | `PROMPT_CR` | Add carriage return to prompt |
| `bar` | `PROMPT_VTANSI_BAR` | Enable VT100/ANSI status bar |
| `classic-ansi-bar` | `PROMPT_CLASSIC_ANSIBAR` | Use classic bar layout |
| `client-prompt` | `PROMPT_CLIENT_PROMPT` | Enable client protocol codes |
| `builder_assistant` | `PROMPT_BUILDER_ASSISTANT` | Builder mode (immortals) |

**Source:** `code/code/sys/connect.h:9-34`

### Prompt Customization Commands

```
prompt hit          - Toggle hit points
prompt mana         - Toggle mana
prompt movement     - Toggle movement
prompt talens       - Toggle money
prompt exp          - Toggle experience
prompt opponent     - Toggle opponent info
prompt tank         - Toggle tank info (includes self)
prompt tank-other   - Toggle tank info (excludes self)
prompt room         - Toggle room vnum
prompt roomname     - Toggle room name
prompt zone         - Toggle zone number
prompt coordinates  - Toggle coordinates
prompt exp_tolevel  - Toggle XP to next level
prompt time         - Toggle time display
prompt cr           - Toggle carriage return
prompt bar          - Toggle VT100/ANSI status bar
prompt all          - Enable all prompt options
prompt none/off     - Disable all prompt options
```

### Prompt Coloring

Prompt elements can be individually colored:

```
prompt color <stat> <color>
prompt color off               - Disable prompt colors
```

**Available colors:**
- Basic: `blue`, `red`, `green`, `white`, `purple`, `cyan`, `orange`, `yellow`, `charcoal`
- Bold: `boldred`, `boldgreen`, `boldblue`, `boldpurple`, `boldcyan`
- Background: `white_on_blue`, `white_on_red`, `white_on_purple`, `white_on_green`, `white_on_yellow`, `white_on_cyan`
- Special: `invert`, `blinking`

**Example:**
```
prompt color hit red
prompt color mana blue
prompt color movement green
```

**Source:** `code/code/sys/color.cc:291-442`

### VT100/ANSI Status Bar

For terminals supporting VT100 or ANSI, a persistent status bar can be displayed at the bottom of the screen:

```
toggle terminal ansi    - Enable ANSI mode
prompt bar              - Enable the status bar
```

The status bar shows:
- **Line 1:** Hits / Piety or Mana or Lifeforce / Moves
- **Line 2:** Room (immortals) or Affiliation / Talens / Exp
- **Line 3 (modern):** TNL / Talens (additional layout)

**Source:** `code/code/sys/color.cc:599-706`

## Score Command

The `score` command displays comprehensive character status information.

### Score Output

```
You have 500/600 hit points, 85.50% piety, and 120/150 moves.
You are slightly winded.
You have 1,234,567 exp, and have 5000 talens plus 10000 talens in the bank.
You have earned 50,000 exp this session.
You have a total of 1500 skill points with an average of 45 per skill.
You have been playing for 2 hours, 30 minutes and 15 seconds in this session.
For a lifetime total of 5 days and 12 hours.
Your level: Warrior lev 30          This ranks you as:
Batopr the Sword Master
You need 100,000 experience points to be a Level 31 Warrior.
Your thirst is the least of your worries.
Your hunger is the least of your worries.
You are standing.
You are in normal attack mode.
```

### Score Sections

| Section | Description |
|---------|-------------|
| **Vitals** | HP, mana/piety/lifeforce, movement (class-appropriate) |
| **Fatigue** | Movement condition description |
| **Wealth** | Experience, talens, bank balance |
| **Session** | XP earned this session, skill totals |
| **Playtime** | Session duration, lifetime total |
| **Level** | Class(es), level(s), title |
| **Progression** | XP needed for next level |
| **Conditions** | Hunger, thirst, intoxication |
| **Position** | Standing, sitting, mounted, etc. |
| **Combat** | Attack mode, wimpy setting |
| **Affects** | Active spell/affect conditions |

**Source:** `code/code/cmd/cmd_score.cc:13-295`

### Condition Descriptions

**Movement condition (from `DescMoves()`):**
- "totally exhausted" to "completely rested" based on move percentage

**Hunger levels:**
| Level | Message |
|-------|---------|
| 0 | "You are totally famished." (red) |
| 1-5 | "Your stomach is growling loudly." |
| 6-10 | "You could use a little bite to eat." |
| 11-20 | "You are slightly hungry." |
| 21+ | "Your hunger is the least of your worries." |

**Thirst levels:**
| Level | Message |
|-------|---------|
| 0 | "You are totally parched." (red) |
| 1-5 | "Your throat is very dry." |
| 6-10 | "You could use a little drink." |
| 11-20 | "You are slightly thirsty." |
| 21+ | "Your thirst is the least of your worries." |

**Intoxication levels:**
| Level | Message |
|-------|---------|
| 0 | (no message) |
| 1-3 | "You are feeling tipsy." |
| 4-9 | "You are intoxicated." |
| 10-14 | "You are drunk." |
| 15-19 | "You are very drunk." |
| 20+ | "You are VERY drunk." |

**Source:** `code/code/cmd/cmd_score.cc:151-183`

## Toggle System

The `toggle` command manages player preferences that affect gameplay behavior, terminal settings, and display options.

### Viewing Toggles

Running `toggle` without arguments displays all current settings:

```
Player Toggles
-----------------------------------------------------------------------
Nospam           : on   | Autoeat          : off  | Autokill         : on
Loot-money       : on   | Loot-all         : off  | Noharm           : on
...

Terminal Toggles
-----------------------------------------------------------------------
Screensize        : 24   | Terminal          : ansi | Boss Mode         : off
MSP Sound         : off  | Account Terminal  : ansi | Show Saves        : off
Brief             : off  | Compact           : off

Immortal Toggles (if applicable)
-----------------------------------------------------------------------
Invisibility      : off  | Auto Success      : off  | Stealth Mode      : off
No Hassle         : off  | Immortality       : on

Global Toggles
-----------------------------------------------------------------------
Double Exp        : off
```

### Player Toggles (autobits)

| Toggle | Flag | Effect |
|--------|------|--------|
| `nospam` | `AUTO_NOSPAM` | Hide combat misses and other spam |
| `autoeat` | `AUTO_EAT` | Automatically eat/drink when hungry/thirsty |
| `autokill` | `AUTO_KILL` | Continue attacking stunned creatures |
| `loot-money` | `AUTO_LOOT_MONEY` | Auto-loot money from corpses |
| `loot-all` | `AUTO_LOOT_NOTMONEY` | Auto-loot everything from corpses |
| `noharm` | `AUTO_NOHARM` | Prevent attacking other players |
| `noshout` | `AUTO_NOSHOUT` | Block shout channel |
| `noPG13` | `AUTO_PG13` | Block vulgar language |
| `afk` | `AUTO_AFK` | Auto-AFK message when idle |
| `split` | `AUTO_SPLIT` | Auto-split gold with group |
| `pouch` | `AUTO_POUCH` | Auto-open money pouches |
| `trophy` | `AUTO_TROPHY` | Show trophy after kills |
| `tips` | `AUTO_TIPS` | Show periodic gameplay tips |
| `join` | `AUTO_JOIN` | Allow faction admission |
| `dissect` | `AUTO_DISSECT` | Auto-dissect corpses |
| `engage` | `AUTO_ENGAGE` | Engage instead of fight when casting |
| `engage-all` | `AUTO_ENGAGE_ALWAYS` | Always engage instead of fight |
| `hunt` | `AUTO_HUNT` | Auto-move toward tracked targets |
| `nospells` | `AUTO_NOSPELL` | Show only first/last spell messages |
| `halfspells` | `AUTO_HALFSPELL` | Show half of spell messages randomly |
| `limbs` | `AUTO_LIMBS` | Show tank limb status after fights |
| `no-hero-sprites` | `AUTO_NOSPRITE` | Disable hero sprites |
| `notell` | `AUTO_NOTELL` | Block incoming tells |
| `autogroup` | `AUTO_AUTOGROUP` | Auto-group new followers |
| `map` | `AUTO_MAP` | Enable automap display |
| `maptags` | `AUTO_MAPTAGS` | Show map location tags |
| `spelltask` | `AUTO_SPELLTASK` | Show spelltask in prompt |

**Source:** `code/code/misc/toggle.h:22-51`

### Player Action Toggles (plr_act)

| Toggle | Flag | Effect |
|--------|------|--------|
| `brief` | `PLR_BRIEF` | Short room descriptions |
| `compact` | `PLR_COMPACT` | Compact output mode |
| `showsaves` | `PLR_SHOW_SAVES` | Show save notifications |
| `deny-corpse-loot` | `PLR_DENY_LOOT` | Prevent others from looting your corpse |
| `newbiehelper` | `PLR_NEWBIEHELP` | Mark as available to help newbies |
| `anonymous` | `PLR_ANONYMOUS` | Hide level in who list (requires level 5) |

**Source:** `code/code/misc/toggle.h:53-84`

### Terminal Toggles

| Toggle | Command | Effect |
|--------|---------|--------|
| Screensize | `toggle screensize <num>` | Set screen height (1-128) |
| Terminal | `toggle terminal <type>` | Set terminal: `ansi`, `vt100`, `none` |
| Account terminal | `toggle account <type>` | Set default terminal for account |
| Boss mode | `toggle boss` | Minimal display mode |
| MSP sound | `toggle msp` | MUD Sound Protocol |

### Wimpy Toggle

The `wimpy` toggle automatically flees combat when HP drops below a threshold:

```
toggle wimpy <number>   - Set wimpy HP threshold
toggle wimpy off        - Disable wimpy
toggle wimpy max        - Set to maximum allowed
```

**Wimpy limits:**
- Maximum wimpy is `maxWimpy()` (typically half max HP)
- Characters with `TOG_IS_COWARD` trait cannot disable wimpy
- Characters with `TOG_IS_VICIOUS` trait cannot enable wimpy
- Characters with `TOG_IS_CRAVEN` trait must maintain minimum wimpy

**Source:** `code/code/misc/toggle.cc:808-849`

### Immortal Toggles

| Toggle | Command | Effect |
|--------|---------|--------|
| Invisibility | `toggle invisibility [level]` | Set invisible to level |
| Stealth | `toggle stealth` | Silent movement mode |
| No hassle | `toggle nohassle` | Mobs don't attack |
| Immortal | `toggle immortal` | Toggle immortal/mortal mode |
| Auto success | `toggle success` | Automatic skill success/failure |

### Global Toggles (Admin Only)

Administrators with `POWER_TOGGLE` can control server-wide settings:

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

**Source:** `code/code/misc/toggle.h:90-114`

## Help System

The help system provides in-game documentation through indexed help files stored on disk.

### Help Command

```
help <topic>      - Display help on topic
help index        - Display all help topics
help              - Display general help
```

### Help Categories

| Category | Path | Description |
|----------|------|-------------|
| General | `lib/help/` | Player-facing help files |
| Skills | `lib/help_skills/` | Skill documentation |
| Spells | `lib/help_spells/` | Spell documentation |
| Immortal | `lib/help_immortal/` | Staff commands |
| Builder | `lib/help_builder/` | Zone building help |

**Source:** `code/code/cmd/cmd_help.cc:1112-1208`

### Help File Structure

Help files are plain text files stored in the appropriate help directory. The filename becomes the help topic name (case-insensitive).

**Example:** `lib/help/combat` is accessed via `help combat`

Files with `.ansi` extension provide ANSI-colorized versions for color-capable terminals.

### Help Search Priority

The help system searches in this order:
1. Immortal help (if immortal with `POWER_IMMORTAL_HELP`)
2. Builder help (if builder level)
3. General help
4. Spell help
5. Skill help

Abbreviations are supported - `help com` matches `combat`.

**Source:** `code/code/cmd/cmd_help.cc:391-1036`

### Skill/Spell Help Display

For skills and spells, the help system shows additional metadata:

```
FIREBALL                       (Last Updated: Jan 15 2025)

Discipline       : Elemental Fire
Specialization   : Fire Magic
Learned in Disc. : 15%
Disc. Learn Rate : Fast
Learn By Doing   : Yes
Modifier Stat    : INT

Spell Component  : a vial of fire oil
Difficulty       : Moderate
Immunity Type    : Fire
Command lock-out : 2.5 seconds
Minimum Mana     : 30, current : 25
Requires         : Gestural Moves, Spoken Incantation
Offensive        : Yes    Area Effect          : No
Cast on Self     : No     Object Castable      : No
Cast on Others   : Yes

[help file content follows]
```

**Source:** `code/code/cmd/cmd_help.cc:597-872`

### Building Help Index

The help index is built at startup by scanning all help directories:

```cpp
void buildHelpIndex() {
    // Scans and indexes:
    // - Path::IMMORTAL_HELP
    // - Path::BUILDER_HELP
    // - Path::HELP
    // - Path::SKILL_HELP
    // - Path::SPELL_HELP
}
```

Files ending in `.ansi` are excluded from the index (they're auto-selected for color terminals).

**Source:** `code/code/cmd/cmd_help.cc:1112-1208`

## Color Command

The `color` command controls color display settings:

```
color              - Show current settings
color test         - Display color samples
color enabled/on   - Enable basic color
color disabled/off - Disable color
color all          - Enable all color options
color none         - Disable all color options
```

### Color Categories

| Category | Effect |
|----------|--------|
| `communications` | Color in tells, says, shouts |
| `objects` | Color in object descriptions |
| `mobiles` | Color in creature descriptions |
| `rooms` | Color in room descriptions |
| `room_name` | Different colors per room type |
| `shouts` | Color in shout channel |
| `spells` | Color in spell messages |

### Color Replacement

Players can replace specific colors they find hard to see:

```
color replace <color>      - Replace color with substitute
color substitute <color>   - Set the substitute color
```

**Source:** `code/code/sys/color.cc:710-1365`

## GMCP Support

For clients supporting GMCP (Generic MUD Communication Protocol), the server sends structured data:

| Package | Content |
|---------|---------|
| `char.vitals` | HP, mana, moves, piety, lifeforce |
| `char.maxstats` | Maximum HP, mana, moves |
| `char.status` | XP, level, talens, hunger, thirst |
| `char.position` | Position, fighting status |
| `room.info` | Room number, name, exits, coordinates |
| `room.area` | Zone information |
| `room.mobs` | Creatures in room |

**Source:** `code/code/sys/comm.cc:186-368`

## Common Patterns

### Checking Player Preferences

```cpp
// Check autobits (toggle settings)
if (IS_SET(desc->autobits, AUTO_NOSPAM)) {
    // Player has nospam enabled
}

// Check player actions
if (isPlayerAction(PLR_BRIEF)) {
    // Player wants brief mode
}
```

### Sending Prompt Updates

The prompt is automatically generated and sent after each command based on `desc->prompt_d.type` flags.

### Terminal Detection

```cpp
if (hasColorVt()) {
    // Has VT100 or ANSI terminal
}
if (ansi()) {
    // Has ANSI color support
}
if (vt100()) {
    // Has VT100 support
}
```

## Key Source Files

| File | Contents |
|------|----------|
| `code/code/sys/color.cc` | `doPrompt()`, `doColor()`, `doCls()` |
| `code/code/sys/connect.h` | Prompt flags, `promptData` class |
| `code/code/cmd/cmd_score.cc` | `doScore()` implementation |
| `code/code/misc/toggle.cc` | `doToggle()`, autobits, global toggles |
| `code/code/misc/toggle.h` | Toggle constants and flags |
| `code/code/cmd/cmd_help.cc` | `doHelp()`, help index building |
| `code/code/misc/help.h` | Help system declarations |
| `code/code/sys/comm.cc` | GMCP functions |

## Gotchas

1. **Tank vs Tank-Other:** Only one can be enabled at a time - `tank` includes self, `tank-other` excludes self
2. **Color prerequisite:** Advanced color options require `color enabled` first
3. **Wimpy restrictions:** Character traits (coward, vicious, craven) can restrict wimpy settings
4. **Anonymous minimum level:** Must be level 5+ to use anonymous toggle
5. **Help abbreviations:** Can match unintended topics - `help arm` might match `armor` instead of `armadillo`
6. **ANSI bar margin:** The VT100/ANSI bar reserves bottom 3-4 lines of terminal

## Related Documentation

- [Configuration Reference](configuration-reference.md) - Server configuration options
- [Stats and Attributes](stats-attributes.md) - Stat system details
- [Combat Rounds](combat-rounds.md) - Combat timing and flow
- [Logging Patterns](logging-patterns.md) - Server logging system
