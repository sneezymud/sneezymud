---
title: Quest System
description: Binary toggle bit array for tracking quest eligibility, progression stages, completion, and cheat detection.
category: informational
keywords: [quest bits, quest progression, toggle tracking]
primary_symbols:
  functions: [hasQuestBit, setQuestBit, remQuestBit, doMortalQuest]
  classes: [TBeing, TPerson]
  enums: [MAX_TOG_INDEX, TOG_TESTCODE1, TOG_TESTCODE2, TOG_TESTCODE3, TOG_TESTCODE4, TOG_TESTCODE5, TOG_QUESTCODE1, TOG_QUESTCODE2, TOG_QUESTCODE3, TOG_QUESTCODE4, TOG_SHOUTING, TOG_SLEEP, TOG_DOUBLEEXP, TOG_FAE_TOUCHED, TOG_NO_XP_GAIN, togTypeT]
---

## Overview

How do you track whether a player has completed step 3 of a 12-part quest, or whether they're eligible to learn a special spell, or whether they've tried to cheat their way through a quest chain? The quest system answers this through a simple but effective mechanism: a binary toggle bit array.

Each character carries a `toggles[]` array of 454 bits. Each bit represents a discrete state: eligible for a quest, accepted a quest, completed a stage, finished entirely, or caught cheating. Quest logic is not encapsulated in a Quest class. Instead, spec procedures and response handlers check these bits with `hasQuestBit()` and modify them with `setQuestBit()` and `remQuestBit()`.

This approach trades flexibility for simplicity. You cannot store "killed 3 of 10 goblins" in a single bit. But you can represent any linear or branching progression as a sequence of bits, and the entire system compiles down to array bounds checks and bitwise operations.

Quest progression flows through predictable stages. A player might start with bit 1 (eligible), then bit 2 (rules read), then bit 3 (started), through various intermediate bits (components collected, stages cleared), to bit N (finished). Major quests include dedicated cheat detection bits that get set when players skip required steps.

When a player types `quest`, the system scans their toggle array for set bits that have corresponding help files in `lib/mobdata/responses/help/`, showing them what they're currently working on.

Some quest bits are mirrored to the `playertoggle` database table for web interface access.

---

## Patterns

### Quest Bit Operations

Always validate quest bit indices are within bounds before operations. The core functions (`hasQuestBit`, `setQuestBit`, `remQuestBit`) perform this validation internally, but direct array access would bypass it. Out-of-bounds access corrupts memory.

Never assume a quest bit is set without checking. Quest state can change due to admin intervention, character conversion bugs, or quest resets. Always use `hasQuestBit()` to verify state before acting on it.

### Quest State Validation

Always check eligibility before allowing quest start. A spec procedure should reject interaction if the player lacks the prerequisite ELIGIBLE bit. Allowing uneligible players to start quests breaks progression assumptions.

Always check completion status before allowing restart. Players who have FINISHED bits set should not be able to start the quest again (unless intentionally repeatable). Missing this check causes duplicate rewards.

Always verify prerequisites in order. If a quest has stages 1 through 5, check that stage N-1 is complete before advancing to stage N. Missing intermediate checks enables sequence breaking.

### Cheat Detection

Always set cheat bits when detecting sequence violations. If a player has STAGE_3 set but not STAGE_2, they've bypassed intended progression. Set the CHEAT bit, revoke the improperly obtained bit, and log the incident.

Never rely solely on cheat bits for enforcement. Cheat detection is a secondary defense. The primary defense is validating prerequisites before advancing state. Cheat bits catch what slips through.

Always log cheat detections via `LOG_CHEAT`. This creates an audit trail for investigating player reports and identifying exploits.

### Solo Kill Tracking

Quests requiring solo mob kills use combat affects to detect group assistance. An `AFFECT_COMBAT` affect with `COMBAT_SOLO_KILL` modifier is attached to the mob when combat begins, storing a pointer to the initial attacker. The damage pipeline checks this affect and sets cheat bits if any other player participates. This ensures quest credit only goes to solo kills.

### Component Turn-In

Collection quests verify the player possesses required items by searching equipment with `getObjFromEquipment()`. Upon successful verification, the component bit is set, the item is removed with `extract_obj()`, and the mob provides dialogue about the next step. All component bits must be set before the quest can complete.

### Quest Rewards

Always verify quest completion before giving rewards. Check the FINISHED bit (or equivalent completion indicator) before spawning reward items. Trusting player claims without bit verification enables duping.

Consider removing or modifying completion bits after reward delivery. Some quests remove FINISHED to prevent duplicate rewards. Others leave it set to indicate permanent completion. Choose based on quest design intent.

Never give quest items to players without the appropriate quest bits. If an item should only be obtainable through a quest, the spec procedure that gives it must check quest state first.

### Help File Management

Always create help files for quest bits that players should see in their quest log. The `quest` command only counts and displays bits that have corresponding files in `lib/mobdata/responses/help/{bit_number}`.

Never create help files for internal-only bits. Cheat detection bits, temporary state bits, and completion bits (where you want the quest to disappear after completion) should not have help files.

### Integration with Spec Procedures

Always return TRUE from spec procedures after handling quest interaction. This prevents the command from being processed by default handlers, which could cause unintended side effects.

Always check `hasQuestBit()` early in spec procedures. If the player isn't at the right quest stage, bail out immediately rather than processing expensive logic.

---

## Reference

### Symbol Quick Reference

| Symbol | Type | Purpose |
|--------|------|---------|
| `hasQuestBit()` | function | Check if a quest bit is set |
| `setQuestBit()` | function | Set a quest bit to 1 |
| `remQuestBit()` | function | Clear a quest bit to 0 |
| `doMortalQuest()` | function | Handle the `quest` command |
| `file_to_sstring()` | function | Load help file content for paging |
| `page_string()` | function | Page content to player's descriptor |
| `TBeing` | class | Base class with quest bit interface |
| `TPerson` | class | Player class with quest bit storage |
| `toggles[]` | array | Per-character quest bit storage |
| `MAX_TOG_INDEX` | constant | Maximum number of quest bits (454) |

### Major Quest Bit Ranges

| Quest | Bit Range | Notes |
|-------|-----------|-------|
| Avenger | 1-8 | 8 bits |
| Vindicator | 9-26 | 18 bits |
| Silverclaw | 30-45 | 16 bits |
| Holy Devastator | 44-77 | 34 bits |
| Monk Sashes (White) | 132-134 | |
| Monk Sashes (Yellow) | 135-137 | |
| Monk Sashes (Purple) | 138-145 | |
| Monk Sashes (Blue) | 146-150 | |
| Monk Sashes (Green) | 151-156 | |
| Monk Sashes (Red) | 81-84 | |
| Monk Sashes (Black) | 215-218 | |
| Warrior Progression (L7) | 162-173 + 28 reused | |
| Warrior Progression (L14) | 174-186 | |
| Warrior Progression (L21) | 195-212 | |
| Warrior Progression (L41) | 219-223 | |
| Thief Progression | 374-432 | Starts at TOG_THIEF_L5_ELIGIBLE |

### Spell Unlock Bits

| Spell | Bits | Notes |
|-------|------|-------|
| Tornado | 103-104 | TOG_TORNADO_ELIGIBLE / TOG_HAS_TORNADO |
| Barkskin | 105-106 | Eligible / Has |
| Earthquake | 107-108 | Eligible / Has |

Note: Bits 50-55 are Holy Devastator quest bits, not spell unlocks.

### Test Code Bits

Note: These are `togTypeT` enum positions, not quest bit indices. The enum values define symbolic names used as indices into the `toggles[]` array.

| Toggle | Enum Name |
|--------|-----------|
| `TOG_TESTCODE1-5` | Enum positions in togTypeT |
| `TOG_QUESTCODE1-4` | Enum positions in togTypeT |

### Common Bit Suffixes

| Suffix | Meaning |
|--------|---------|
| `_ELIGIBLE` | Player can begin this quest/stage |
| `_RULES` | Player has read quest rules |
| `_STARTED` | Quest officially in progress |
| `_HUNTING` | Player searching for components |
| `_FINISHED` | Quest completed |
| `_CHEAT` | Cheat detection triggered |
| `_PENANCED` | Player has served penalty for cheating |

### Global Toggle Types

| Toggle | Purpose |
|--------|---------|
| `TOG_SHOUTING` | Player can use shout command |
| `TOG_SLEEP` | Player auto-sleeps when idle |
| `TOG_DOUBLEEXP` | Server-wide double XP event |
| `TOG_TESTCODE1-5` | Builder test flags |
| `TOG_QUESTCODE1-4` | Quest development flags |

### Key Files

| File | Purpose |
|------|---------|
| `cmd_quest.cc` | Quest bit functions and command |
| `toggle.h` | All 454 quest bit definitions |
| `being.h` | Quest bit interface declarations |
| `lib/mobdata/responses/help/` | Quest help files (named by bit number) |

---

## Implementation

### Data Storage

Each character maintains a `toggles[]` array as part of their persistent data. The array has `MAX_TOG_INDEX` (454) elements, each storing a single byte. Only the least significant bit of each byte is used, making the array effectively boolean.

The array is declared in `TBeing` and `TPerson`. Both classes implement identical quest bit methods. The array persists across sessions as part of the `charFile` struct written to player save files at `lib/mutable/player/{first_letter}/{charname}`.

Some quest bits are mirrored to the `playertoggle` database table for web interface queries.

### Core Functions

The three core functions provide the entire quest bit API.

`hasQuestBit(int value)` validates that the bit index is within bounds (0 to MAX_TOG_INDEX-1). If out of bounds, it logs a `LOG_BUG` message and returns FALSE. Otherwise, it returns the boolean value of `toggles[value]`.

`setQuestBit(int value)` performs the same bounds validation, then sets bit 0 of the array element using `toggles[value] |= 0x1`. The bitwise OR ensures only the target bit is modified.

`remQuestBit(int value)` clears the bit using `toggles[value] &= ~(0x1)`. The bitwise AND with inverted mask ensures only bit 0 is cleared.

All three functions are implemented in `cmd_quest.cc`. The bounds checking prevents buffer overflows but means invalid bit numbers silently fail (after logging).

### Quest Command Implementation

The `quest` command is handled by `doMortalQuest()`. Without arguments, it counts active quests. With a number argument, it displays the help file for that quest.

The function iterates backwards through all quest bits (from MAX_TOG_INDEX-1 down to 0). For each set bit, it checks if a help file exists at `lib/mobdata/responses/help/{bit_number}`. Only bits with help files count as "quests" for display purposes.

When showing a specific quest, the function loads the help file into a string using `file_to_sstring()` and pages it to the player via `desc->page_string()`. This allows long quest descriptions without flooding the player's screen.

Immortals (characters where `GetMaxLevel()` exceeds `MAX_MORT`) have extended functionality: they can view quest help for any bit number directly, regardless of whether they have the bit set.

### Quest Bit Constants

All 454 quest bits are defined as `const int` values in `toggle.h`. The bits are organized by quest system:

Equipment quests (Avenger, Vindicator, Silverclaw, Holy Devastator) occupy bits 1-77. Each follows a pattern: ELIGIBLE, RULES, STARTED, various stage bits, CHEAT bits, and FINISHED. The Devastator quest implements the most sophisticated cheat detection with seven cheat bits (73-79): MISER_BEN, SPARTAGUS, MARCUS, TAILLE, ABNOR, SULTRESS, NESMUM.

Monk sash quests span scattered ranges. Seven sashes (white through black) use varying numbers of bits: White (132-134), Yellow (135-137), Purple (138-145), Blue (146-150), Green (151-156), Red (81-84), Black (215-218).

Spell/skill unlock bits occupy scattered higher ranges (e.g., Tornado at 103-104, Barkskin at 105-106, Earthquake at 107-108). Bits 50-55 are Holy Devastator quest bits, not spell unlocks.

Warrior progression quests use scattered ranges across levels: L7 (162-173 + bit 28 reused), L14 (174-186), L21 (195-212), L41 (219-223). Thief progression starts at 374 (TOG_THIEF_L5_ELIGIBLE) through 432. These are level-gated quest chains.

Global toggles (TOG_SHOUTING, TOG_DOUBLEEXP, etc.) are defined as an enum `togTypeT` separate from the quest bit constants. These control server behavior rather than quest state.

### Help File System

Quest help is stored as plain text files in `lib/mobdata/responses/help/`. Each file is named with just the bit number (no extension). The file content can be any text describing the current quest stage.

When a player types `quest` (or `quest N`), the system only considers bits that have corresponding help files. This means:
- Bits without help files are invisible to players
- Help files control which bits appear in the quest log
- Creating/deleting help files changes quest visibility without recompilation

Help files typically describe the quest stage, current objectives, and hints for progression.

### Integration Points

Quest bits integrate with multiple systems through `hasQuestBit()` checks:

**Experience system**: Some bits modify XP gain. TOG_FAE_TOUCHED halves experience. TOG_NO_XP_GAIN blocks it entirely. These checks occur in `gain_exp()`.

**Spell/skill access**: Bits like TOG_TORNADO_ELIGIBLE gate whether a player can cast certain spells. The spell casting code checks these bits before allowing the spell.

**Equipment loading**: Spec procedures check completion bits before giving quest reward items. This ensures players cannot obtain quest gear without completing the quest.

**Combat tracking**: The damage pipeline can set cheat bits. If a quest requires solo kills, the system tracks who participated in combat and sets CHEAT bits if others helped.

**Mob responses**: Response handlers check quest bits to determine dialogue. A mob might have different responses for players who haven't started, are in progress, or have completed a quest.

### Progression Patterns

Linear quests use sequential bits. ELIGIBLE leads to STARTED leads to FINISHED. Spec procedures check the current bit and advance to the next when conditions are met.

Multi-stage quests add intermediate bits. COMPONENT_1 through COMPONENT_N track collection progress. The final FINISHED bit is only set when all components are collected.

Cheat detection uses dedicated CHEAT bits. When a spec procedure detects an impossible state (stage 3 without stage 2), it sets the CHEAT bit. Subsequent interactions check for CHEAT bits and respond accordingly (refusing service, requiring penance).

Penance bits (like TOG_AVENGER_PENANCED) track whether a player has served their penalty for cheating. This allows quest re-entry after appropriate consequences.

### Limitations

The array size is fixed at compile time. Adding new quest bits requires incrementing MAX_TOG_INDEX, recompiling, and handling existing save files (which have smaller arrays). Reusing bits from retired quests avoids this.

Bits are binary only. Progress counters ("3 of 10 goblins killed") cannot be stored in a single bit. These must be simulated with multiple bits or external tracking.

No quest journal exists. Players can only see quests via the `quest` command, which shows help files for currently set bits. There's no way to see available quests or track completed ones.

No formal dependency system exists. Prerequisites are checked procedurally in spec procedures. If a quest requires another quest's completion, the spec procedure must check that bit explicitly.

---

## Troubleshooting

### Quest Bit Not Being Set

**Symptom:** Player completes quest requirements but progress doesn't advance.

**Likely cause:** The spec procedure's `setQuestBit()` call isn't being reached due to condition checks failing.

**Diagnostic approach:** Add temporary logging before and after condition checks. Verify the player's current bits match expected state. Check that the correct bit constant is being used.

**Fix:** Ensure all prerequisite conditions are actually met before the setQuestBit call. Verify bit constants match between eligibility checks and state advances.

### Quest Shows Wrong Help File

**Symptom:** Player sees help text that doesn't match their current quest stage.

**Likely cause:** Multiple bits are set from different stages, and the wrong one has a help file, or help files are misnamed.

**Diagnostic approach:** Check which bits the player has set using immortal commands. Verify help file names match the expected bit numbers.

**Fix:** Ensure spec procedures properly clear previous stage bits when advancing. Verify help file names are correct (just the bit number, no extension).

### Cheat Bit Incorrectly Set

**Symptom:** Player reports being marked as cheater when they didn't cheat.

**Likely cause:** Quest stage validation is too strict, or there's a race condition in multi-step interactions.

**Diagnostic approach:** Review the spec procedure's cheat detection logic. Check if legitimate paths could trigger the detection. Common causes include quest bits from previous versions not being cleared, teleportation bypassing required travel stages, or timing issues where prerequisite bits are checked before being set.

**Fix:** Adjust validation to account for legitimate edge cases. Consider logging more context when setting cheat bits for future debugging. Immortals can clear cheat bits manually with the set command: `set quest {player} {cheat_bit_number} 0`.

### Quest Bits Lost on Relog

**Symptom:** Player's quest progress disappears after logout/login.

**Likely cause:** Character save failed, or the player is below the save threshold, or save occurred before quest bit was set.

**Diagnostic approach:** Check save timing relative to quest bit modification. Verify the player meets minimum level requirements for saving. Check for save errors in logs.

**Fix:** Ensure quest bit modifications trigger or are followed by a save. Consider calling `doQueueSave()` after critical quest state changes.

### Quest Cannot Be Restarted After Completion

**Symptom:** Player wants to repeat a quest but cannot interact with the quest NPC.

**Likely cause:** This is intentional design. The finished bit check causes early return in the spec procedure.

**Diagnostic approach:** Verify the quest is not designed to be repeatable.

**Fix:** If a quest should be repeatable, the spec procedure must clear the finished bit when giving the reward or provide explicit restart logic. If a player needs to restart a completed quest, an immortal must manually clear the finished bit with `remQuestBit` or the set command.

### Quest Bit Lost After Polymorph or Switch

**Symptom:** Quest progress disappears after polymorph or switch operation.

**Likely cause:** The toggles array is not being copied correctly during character transfer.

**Diagnostic approach:** Verify the toggles array is being copied during character transfer. Check that original pointers are being preserved correctly.

**Fix:** Quest bits should persist across these operations as they are fundamental character data. If they're not persisting, examine the switch and polymorph code to ensure toggles array preservation.

### Invalid Quest Bit Logged

**Symptom:** LOG_BUG shows "Bad check of hasQuestBit(X)" with unexpected bit number.

**Likely cause:** Code is using an incorrect bit constant, or a negative/overflow value is being passed.

**Diagnostic approach:** Search for the logged bit number to find where it originates. Check for integer overflow or variable corruption. Verify quest bit constants from toggle.h are being used, not hardcoded integers.

**Fix:** Correct the bit constant usage. Add validation before calling quest bit functions if the value comes from player input or calculation.

### Database Quest Bit Sync Issues

**Symptom:** Web interface shows incorrect quest state compared to in-game.

**Likely cause:** The database sync to `playertoggle` table failed or is out of date.

**Diagnostic approach:** Verify the database sync code is calling appropriate insert or update statements after `setQuestBit()` and `remQuestBit()`. Check database connection and query execution.

**Fix:** Ensure database sync occurs after quest bit changes. Check database table structure and web interface queries.
