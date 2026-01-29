---
title: Quest System
description: Uses a binary toggle bit array to track quest progress and state with 454 predefined quest bit constants set/cleared as players progress through quests.
keywords:
  - toggles array
  - hasQuestBit
  - setQuestBit
  - remQuestBit
  - TOG_AVENGER_ELIGIBLE
  - TOG_DEVASTATOR_CHEAT
  - TOG_MONK_WHITE_FINISHED
  - TOG_WARRIOR_QUEST
  - TOG_THIEF_QUEST
  - quest command
  - doMortalQuest
  - MAX_TOG_INDEX
  - quest help files
  - cheat detection
  - spec procedures
category: Important Systems
related:
  - spec-procs.md
  - mob-ai.md
  - experience-leveling.md
  - persistence-rent.md
  - logging-patterns.md
last_updated: 2026-01-29
source_files:
  - code/code/cmd/cmd_quest.cc
  - code/code/misc/toggle.h
  - code/code/misc/being.h
  - lib/mobdata/responses/help
---

# Quest System

The quest system in SneezyMUD uses a binary toggle bit array to track quest progress and state. Unlike traditional object-oriented quest systems, SneezyMUD's approach uses 454 predefined quest bit constants that are set/cleared as players progress through quests.

## Overview

The quest system is procedural rather than class-based. Quest states are stored as individual bits in a `toggles[]` array on each character. Each quest uses one or more sequential toggle bits to track progression through stages (e.g., eligible → accepted → in-progress → completed).

**Key characteristics:**
- 454 predefined quest bits (TOG_0 through TOG_453)
- Simple bit array storage in character save files
- Quest help stored as text files in `lib/mobdata/responses/help/{toggle_number}`
- No Quest class - quest logic implemented via spec procedures and hasQuestBit() checks
- Cheat detection built into major quests via dedicated toggle bits

**Source files:**
- `code/code/cmd/cmd_quest.cc` - Core quest bit functions and quest command
- `code/code/misc/toggle.h` - All 454 quest bit definitions
- `code/code/misc/being.h` - Quest bit interface declarations

## Data Structures

### toggles[] Array

Each character stores quest state in a simple array:

```cpp
// In TBeing/TPerson
ubyte toggles[MAX_TOG_INDEX];  // MAX_TOG_INDEX = 454
```

Each element is a single byte, but only the least significant bit is used (0 or 1).

### Quest Bit Constants

All quest bits are defined as `const int` values in `toggle.h`:

```cpp
// Equipment quests
const int TOG_AVENGER_ELIGIBLE = 1;
const int TOG_AVENGER_RULES = 2;
const int TOG_AVENGER_HUNTING = 3;
const int TOG_AVENGER_STARTED = 4;
const int TOG_AVENGER_OBTAINED = 5;
const int TOG_AVENGER_FINISHED = 6;
const int TOG_AVENGER_CHEAT = 7;
const int TOG_AVENGER_PENANCED = 8;

// Monk sash quests
const int TOG_MONK_WHITE_ELIGIBLE = 56;
const int TOG_MONK_WHITE_STARTED = 57;
const int TOG_MONK_WHITE_FINISHED = 58;
// ... through TOG_MONK_BLACK_FINISHED = 75

// Spell unlock quests
const int TOG_TORNADO_ELIGIBLE = 50;
const int TOG_BARKSKIN_ELIGIBLE = 51;
const int TOG_EARTHQUAKE_ELIGIBLE = 52;
// ... etc
```

**Source:** `code/code/misc/toggle.h` (lines 147-732)

### Global Toggles

Some toggles control global server behavior rather than individual quest state:

```cpp
enum togTypeT {
    TOG_NONE = 0,
    TOG_SHOUTING,           // Player can use shout command
    TOG_SLEEP,              // Player auto-sleeps when idle
    TOG_DOUBLEEXP,          // Server-wide double XP event
    TOG_TESTCODE1,          // Builder test code flag 1
    TOG_TESTCODE2,          // Builder test code flag 2
    TOG_QUESTCODE1,         // Quest test code flag 1
    TOG_QUESTCODE2,         // Quest test code flag 2
    // ... etc
    MAX_TOG_TYPES
};
```

**Source:** `code/code/misc/toggle.h` (lines 90-114)

## Core API

### Quest Bit Functions

Both `TBeing` and `TPerson` implement identical quest bit functions:

```cpp
bool TBeing::hasQuestBit(int value) const {
    if (value < 0 || value >= MAX_TOG_INDEX) {
        vlogf(LOG_BUG, format("Bad check of hasQuestBit(%d)") % value);
        return FALSE;
    }
    return (toggles[value]);
}

void TBeing::setQuestBit(int value) {
    if (value < 0 || value >= MAX_TOG_INDEX) {
        vlogf(LOG_BUG, format("Bad check of setQuestBit(%d)") % value);
        return;
    }
    toggles[value] |= 0x1;
}

void TBeing::remQuestBit(int value) {
    if (value < 0 || value >= MAX_TOG_INDEX) {
        vlogf(LOG_BUG, format("Bad check of remQuestBit(%d)") % value);
        return;
    }
    toggles[value] &= ~(0x1);
}
```

**Source:** `code/code/cmd/cmd_quest.cc` (lines 8-60)

**Safety features:**
- Bounds checking on all operations prevents array overruns
- Logging of invalid bit numbers to `LOG_BUG`
- Bitwise operations ensure only bit 0 is modified

### Quest Command

The `quest` command displays active quests to players:

```cpp
void TBeing::doMortalQuest(const char* tArg) {
    sendTo("Your current quest status:\n\r");

    // Immortals can view specific quest help directly
    if (GetMaxLevel() > MAX_MORT) {
        // ... immortal-specific code
    }

    int questNumber = convertTo<int>(tArg);
    unsigned int totFound = 0;
    int questRes = -1;

    if (questNumber <= 0)
        questNumber = 1;

    // Iterate backwards through all quest bits
    for (int questIndex = (MAX_TOG_INDEX - 1); questIndex > -1; questIndex--) {
        if (hasQuestBit(questIndex)) {
            sprintf(questPath, "mobdata/responses/help/%d", questIndex);
            FILE* fp = fopen(questPath, "r");
            if (fp) {
                totFound++;
                fclose(fp);
                if (questNumber == (int)totFound)
                    questRes = questIndex;
            }
        }
    }

    sendTo(format("You have %d total current quest goals.\n\r") % totFound);

    if (questRes == -1) {
        sendTo(format("You don't seem to have a quest goal #%d\n\r") % questNumber);
        return;
    }

    // Display quest help file via pager
    sprintf(questPath, "mobdata/responses/help/%d", questRes);
    if (file_to_sstring(questPath, tStString))
        desc->page_string(tStString);
}
```

**Source:** `code/code/cmd/cmd_quest.cc` (lines 62-126)

**Behavior:**
- Without arguments: Shows count of active quests
- With number: Shows help for that quest (e.g., `quest 2` shows second active quest)
- Help files must exist at `lib/mobdata/responses/help/{toggle_number}`
- Only counts quests that have associated help files

## Major Quest Categories

### Equipment Quests

Large multi-stage quests for major equipment items:

#### Avenger Quest (8 bits)

```cpp
const int TOG_AVENGER_ELIGIBLE = 1;    // Can start quest
const int TOG_AVENGER_RULES = 2;       // Has read rules
const int TOG_AVENGER_HUNTING = 3;     // Hunting for components
const int TOG_AVENGER_STARTED = 4;     // Quest officially started
const int TOG_AVENGER_OBTAINED = 5;    // Got the item
const int TOG_AVENGER_FINISHED = 6;    // Quest complete
const int TOG_AVENGER_CHEAT = 7;       // Cheat detection flag
const int TOG_AVENGER_PENANCED = 8;    // Penanced for cheating
```

#### Vindicator Quest (18 bits)

```cpp
const int TOG_VIND_ELIGIBLE = 9;       // Can start quest
const int TOG_VIND_RULES = 10;         // Has read rules
const int TOG_VIND_STARTED = 11;       // Quest started
// ... 12 through 25 for various quest stages
const int TOG_VIND_FINISHED = 26;      // Quest complete
```

#### Silverclaw Quest (16 bits)

Ranger-specific quest for the Silverclaw weapon.

```cpp
const int TOG_SILVERCLAW_ELIGIBLE = 28;
const int TOG_SILVERCLAW_RULES = 29;
// ... through TOG_SILVERCLAW_FINISHED = 43
```

#### Holy Devastator Quest (34 bits)

Most complex equipment quest with extensive cheat detection:

```cpp
const int TOG_DEVASTATOR_ELIGIBLE = 44;
const int TOG_DEVASTATOR_RULES = 45;
// ... many intermediate stages
const int TOG_DEVASTATOR_CHEAT_1 = 64;
const int TOG_DEVASTATOR_CHEAT_2 = 65;
const int TOG_DEVASTATOR_CHEAT_3 = 66;
const int TOG_DEVASTATOR_CHEAT_4 = 67;
const int TOG_DEVASTATOR_CHEAT_5 = 68;
// ... through TOG_DEVASTATOR_FINISHED = 77
```

**Source:** `code/code/misc/toggle.h` (lines 147-258)

### Monk Sash Quests (40+ bits)

Seven colored sashes representing monk progression:

```cpp
// White Sash (entry level)
const int TOG_MONK_WHITE_ELIGIBLE = 56;
const int TOG_MONK_WHITE_STARTED = 57;
const int TOG_MONK_WHITE_FINISHED = 58;

// Yellow Sash
const int TOG_MONK_YELLOW_ELIGIBLE = 59;
const int TOG_MONK_YELLOW_STARTED = 60;
const int TOG_MONK_YELLOW_FINISHED = 61;

// Purple, Blue, Green, Red, Black sashes follow same pattern
// ... through TOG_MONK_BLACK_FINISHED = 75
```

**Source:** `code/code/misc/toggle.h` (lines 306-344)

### Warrior Progression Quests (30+ bits)

Level-gated warrior quests:

```cpp
// Level 7 Quest
const int TOG_WARRIOR_QUEST_7_ELIGIBLE = 78;
const int TOG_WARRIOR_QUEST_7_STARTED = 79;
// ... stages
const int TOG_WARRIOR_QUEST_7_FINISHED = 87;

// Level 14, 21, and 40 quests follow similar patterns
// ... through TOG_WARRIOR_QUEST_40_FINISHED = 113
```

**Source:** `code/code/misc/toggle.h` (lines 356-431)

### Spell/Skill Unlock Quests (15+ bits)

Quest bits that gate access to specific spells or skills:

```cpp
const int TOG_TORNADO_ELIGIBLE = 50;
const int TOG_BARKSKIN_ELIGIBLE = 51;
const int TOG_EARTHQUAKE_ELIGIBLE = 52;
const int TOG_LAVA_ELIGIBLE = 53;
const int TOG_FLATULENCE_ELIGIBLE = 54;
const int TOG_PLASMA_MIRROR_ELIGIBLE = 55;
// ... etc
```

**Source:** `code/code/misc/toggle.h` (lines 275-304)

### Thief Progression Quests (70+ bits)

Extensive thief guild progression system:

```cpp
// Level-based thief quests
const int TOG_THIEF_QUEST_5_ELIGIBLE = 345;
const int TOG_THIEF_QUEST_10_ELIGIBLE = 350;
const int TOG_THIEF_QUEST_15_ELIGIBLE = 355;
const int TOG_THIEF_QUEST_20_ELIGIBLE = 360;
const int TOG_THIEF_QUEST_25_ELIGIBLE = 365;
// ... many intermediate stages and completion flags
```

**Source:** `code/code/misc/toggle.h` (lines 632-703)

## Quest Progression Patterns

### Linear Progression

Simple quests use sequential bits:

```
TOG_QUEST_ELIGIBLE → TOG_QUEST_STARTED → TOG_QUEST_FINISHED
```

Example usage in spec procedure:

```cpp
if (!ch->hasQuestBit(TOG_AVENGER_ELIGIBLE)) {
    mob->doTell(ch->getName(), "You are not yet worthy!");
    return TRUE;
}

if (ch->hasQuestBit(TOG_AVENGER_FINISHED)) {
    mob->doTell(ch->getName(), "You have already completed this quest.");
    return TRUE;
}

// Quest logic here...
ch->setQuestBit(TOG_AVENGER_STARTED);
```

### Multi-Stage Progression

Complex quests use many intermediate bits:

```
ELIGIBLE → RULES → HUNTING → COMPONENT_1 → COMPONENT_2 → ... → FINISHED
```

### Cheat Detection

Major quests include cheat detection bits:

```cpp
// In quest mob spec procedure
if (ch->hasQuestBit(TOG_DEVASTATOR_CHEAT_1)) {
    // Player tried to skip required steps
    mob->doTell(ch->getName(), "I know what you did. Begone!");
    return TRUE;
}

// Check for required prerequisite
if (!ch->hasQuestBit(TOG_DEVASTATOR_COMPONENT_3)) {
    // Player skipped ahead - mark as cheater
    ch->setQuestBit(TOG_DEVASTATOR_CHEAT_1);
    mob->doAction("glares at you suspiciously.");
    return TRUE;
}
```

## Quest Help Files

Quest information is stored as plain text files in `lib/mobdata/responses/help/`:

**File naming:** The filename is the toggle bit number (e.g., `3` for TOG_AVENGER_HUNTING).

**Example structure:**

```
lib/mobdata/responses/help/
  1       # TOG_AVENGER_ELIGIBLE help
  2       # TOG_AVENGER_RULES help
  3       # TOG_AVENGER_HUNTING help
  ...
```

**Content format:**

Help files can contain any text. Common format:

```
Quest: Avenger's Weapon
Stage: Component Hunting

You must collect the following components:
  - Dragon scale from Ancient Red Dragon
  - Phoenix feather from Firebird
  - Unicorn horn from Silvermane

Return to the quest giver when you have all components.
```

The `quest` command pages these files to players.

## Integration with Other Systems

### Experience System

Quest bits can modify XP gain:

```cpp
// In gain_exp() - code/code/misc/limits.cc
if (ch->hasQuestBit(TOG_FAE_TOUCHED)) {
    gain /= 2;  // Half XP while cursed
}

if (ch->hasQuestBit(TOG_NO_XP_GAIN)) {
    return;  // No XP gain at all
}
```

**Source:** See [Experience and Leveling](experience-leveling.md) for complete XP system

### Spell/Skill Access

Quest bits gate spell access:

```cpp
// In spell casting code
if (spell == SPELL_TORNADO && !ch->hasQuestBit(TOG_TORNADO_ELIGIBLE)) {
    ch->sendTo("You have not learned this spell yet.\n\r");
    return SPELL_FAIL;
}
```

### Equipment Loading

Quest completion bits allow equipment to be loaded:

```cpp
// In spec procedure for quest reward NPC
if (ch->hasQuestBit(TOG_AVENGER_FINISHED)) {
    TObj* weapon = read_object(AVENGER_VNUM, VIRTUAL);
    *ch += *weapon;
    ch->sendTo("You have earned the Avenger!\n\r");
    ch->remQuestBit(TOG_AVENGER_FINISHED);  // Prevent duplicates
}
```

### Mob Responses

Quest state determines mob dialogue via response system:

```cpp
// In mob response file
if (!ch->hasQuestBit(TOG_QUEST_STARTED)) {
    // Show "not started" dialogue
} else if (ch->hasQuestBit(TOG_QUEST_COMPONENT_1)) {
    // Show "component collected" dialogue
} else {
    // Show "in progress" dialogue
}
```

**Source:** See [Mob AI and Behavior](mob-ai.md) for response system details

### Combat System

Quest bits can affect combat:

```cpp
// Solo-kill quest tracking in damage pipeline
if (af->type == AFFECT_COMBAT && af->modifier == COMBAT_SOLO_KILL) {
    TBeing* tbt = dynamic_cast<TBeing*>(af->be);
    if (tbt && tbt != this) {
        tbt->setQuestBit(TOG_AVENGER_CHEAT);  // Failed solo requirement
        v->affectRemove(af);
    }
}
```

**Source:** `code/code/misc/damage.cc` (lines 402-481)

## Common Usage Patterns

### Checking Quest Eligibility

```cpp
// In spec procedure
if (!ch->hasQuestBit(TOG_QUEST_ELIGIBLE)) {
    mob->doTell(ch->getName(), "You are not ready for this quest.");
    return TRUE;
}
```

### Starting a Quest

```cpp
// In response to player action (e.g., saying "accept")
if (ch->hasQuestBit(TOG_QUEST_ELIGIBLE) &&
    !ch->hasQuestBit(TOG_QUEST_STARTED)) {
    ch->setQuestBit(TOG_QUEST_STARTED);
    ch->sendTo("You have accepted the quest!\n\r");
    // Give quest item or instructions
}
```

### Completing Quest Stages

```cpp
// When player turns in component
if (ch->hasQuestBit(TOG_QUEST_STARTED) && !ch->hasQuestBit(TOG_QUEST_COMPONENT_1)) {
    // Check if player has required item
    TObj* item = ch->getObjFromEquipment(ch, COMPONENT_1_VNUM);
    if (item) {
        ch->setQuestBit(TOG_QUEST_COMPONENT_1);
        extract_obj(item);  // Remove component from player
        mob->doTell(ch->getName(), "Excellent! Now bring me the next component.");
    }
}
```

### Quest Completion

```cpp
// When all stages complete
if (ch->hasQuestBit(TOG_QUEST_COMPONENT_1) &&
    ch->hasQuestBit(TOG_QUEST_COMPONENT_2) &&
    ch->hasQuestBit(TOG_QUEST_COMPONENT_3)) {

    ch->setQuestBit(TOG_QUEST_FINISHED);
    ch->remQuestBit(TOG_QUEST_STARTED);

    // Give reward
    TObj* reward = read_object(REWARD_VNUM, VIRTUAL);
    *ch += *reward;

    mob->doTell(ch->getName(), "You have proven yourself worthy!");
}
```

### Preventing Quest Restart

```cpp
// Block restart of completed quest
if (ch->hasQuestBit(TOG_QUEST_FINISHED)) {
    mob->doTell(ch->getName(), "You have already completed this quest.");
    return TRUE;
}
```

### Cheat Detection

```cpp
// Detect skipping required steps
if (ch->hasQuestBit(TOG_QUEST_STAGE_3) &&
    !ch->hasQuestBit(TOG_QUEST_STAGE_2)) {
    // Player somehow got stage 3 without stage 2
    ch->setQuestBit(TOG_QUEST_CHEAT);
    ch->remQuestBit(TOG_QUEST_STAGE_3);  // Revoke progress

    vlogf(LOG_CHEAT, format("%s attempted to skip quest stages") % ch->getName());
}
```

## Quest Bit Persistence

Quest bits are saved in character files automatically. The `toggles[]` array is part of the `charFile` struct written to disk.

**Save location:** `lib/mutable/player/{first_letter}/{charname}`

**Database sync:** Some quest bits may also be mirrored in the `playertoggle` table for web interface access.

**Source:** See [Persistence and Rent](persistence-rent.md) for save system details

## Debugging and Testing

### Immortal Commands

Immortals can manipulate quest bits directly:

```cpp
// In @set command
@set quest <player> <bit_number> <0|1>

// View all set quest bits
@show quest <player>
```

### Test Code Toggles

Dedicated test toggles for quest development:

```cpp
const int TOG_TESTCODE1 = 11;
const int TOG_TESTCODE2 = 12;
const int TOG_TESTCODE3 = 13;
const int TOG_TESTCODE4 = 14;
const int TOG_TESTCODE5 = 15;

const int TOG_QUESTCODE1 = 18;
const int TOG_QUESTCODE2 = 19;
const int TOG_QUESTCODE3 = 20;
const int TOG_QUESTCODE4 = 21;
```

These allow builders to test quest logic without affecting actual quest bits.

### Logging

Quest-related actions can be logged:

```cpp
vlogf(LOG_CHEAT, format("%s set quest bit %d") % ch->getName() % bit_num);
vlogf(LOG_BUG, format("Invalid quest bit %d requested") % bit_num);
```

**Source:** See [Logging Patterns](logging-patterns.md) for logging system

## Limitations and Considerations

### Fixed Array Size

The `MAX_TOG_INDEX` constant is hardcoded to 454. Adding new quest bits requires:

1. Incrementing `MAX_TOG_INDEX` in `toggle.h`
2. Recompiling the server
3. Rebuilding all player files (existing saves have smaller array)

**Workaround:** Reuse quest bits from retired/removed quests.

### No Quest Object Model

Unlike modern quest systems, there is no Quest class with methods like:
- `quest->isComplete()`
- `quest->getCurrentStage()`
- `quest->giveReward()`

All quest logic is implemented procedurally in spec procedures and response files.

### No Quest Journal

Players cannot view a list of available quests or track active objectives beyond the `quest` command showing currently set bits with help files.

### Binary State Only

Each quest bit is binary (set or cleared). There is no built-in support for:
- Progress counters (e.g., "kill 10 goblins, currently at 3")
- Branching quest paths
- Quest variables

These must be simulated using multiple sequential quest bits or external tracking mechanisms.

## Future Enhancements

Potential improvements to the quest system:

1. **Quest Class System:** Object-oriented quest framework with inheritance
2. **Dynamic Quest Bits:** Runtime allocation of quest bits instead of compile-time constants
3. **Quest Journal:** In-game UI showing available/active/completed quests
4. **Progress Tracking:** Integer counters for "collect N items" or "kill N mobs" quests
5. **Quest Rewards:** Structured reward system instead of ad-hoc item loading
6. **Quest Prerequisites:** Formal dependency system between quests
7. **Database Storage:** Move quest state from binary files to database for web interface integration

## Key Source Files

| File | Contents | Lines |
|------|----------|-------|
| `code/code/cmd/cmd_quest.cc` | Quest bit functions, quest command | 127 |
| `code/code/misc/toggle.h` | All 454 quest bit definitions | 147-732 |
| `code/code/misc/being.h` | Quest bit interface declarations | 463-465 |
| `lib/mobdata/responses/help/` | Quest help files (one per quest bit) | - |

## Related Documentation

- [Spec Procs](spec-procs.md) - How mobs implement quest logic
- [Mob AI](mob-ai.md) - Response system for quest dialogue
- [Experience and Leveling](experience-leveling.md) - Quest bit effects on XP
- [Persistence and Rent](persistence-rent.md) - How quest bits are saved
- [Logging Patterns](logging-patterns.md) - Quest event logging
