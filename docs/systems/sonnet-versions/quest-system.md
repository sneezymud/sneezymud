---
title: Quest System
category: important
keywords: [toggles, hasQuestBit, setQuestBit, remQuestBit, spec procedures, cheat detection, quest command, doMortalQuest, MAX_TOG_INDEX]
related: [spec-procs.md, experience-leveling.md, persistence-storage.md]
primary_symbols:
  functions: [hasQuestBit, setQuestBit, remQuestBit, doMortalQuest]
  classes: [TBeing, TPerson]
  files: [code/code/cmd/cmd_quest.cc, code/code/misc/toggle.h]
---

# Quest System

## Overview

SneezyMUD uses a binary toggle bit array to track quest progress and state. Rather than implementing an object-oriented quest framework with Quest classes and methods, the system employs 454 predefined quest bit constants stored in each character's toggles array. Quest logic is implemented procedurally through spec procedures and mob response files that check and manipulate these bits.

The system architecture consists of three core components: a simple bit array in the character structure, compile-time constants defining all quest bits in toggle.h, and plain text help files stored in lib/mobdata/responses/help. Each quest uses one or more sequential bits to track progression through stages such as eligible, accepted, in-progress, and completed. Major quests include dedicated cheat detection bits that flag players who skip required steps.

Quest bits serve multiple purposes beyond simple progression tracking. They gate access to spells and skills, modify experience gain rates, control equipment loading, and determine mob dialogue. The quest command allows players to view their active quests by iterating the toggles array and displaying help files for set bits.

The persistence model is straightforward: the toggles array is part of the charFile struct written to disk at lib/mutable/player locations. Some quest bits are mirrored to the playertoggle database table for web interface access. Immortals can manipulate quest bits directly via admin commands for testing and debugging.

The fundamental limitation is the fixed array size of 454 bits. Adding new quests requires either reusing bits from retired content or incrementing MAX_TOG_INDEX and rebuilding all player files. There is no built-in support for progress counters, branching paths, or quest variables—these must be simulated using multiple sequential bits.

## Patterns

### Linear Progression Pattern

Simple quests follow a three-stage pattern using sequential bits: eligible, started, finished. Spec procedures check hasQuestBit for eligibility before allowing quest acceptance, set the started bit when the player accepts, and set the finished bit upon completion. The finished bit prevents restart by causing early return in the spec procedure.

### Multi-Stage Progression Pattern

Complex quests like Avenger and Devastator use numerous intermediate bits to track component collection and subtask completion. Each stage has its own bit that must be set before proceeding to the next. Spec procedures enforce strict ordering by checking prerequisite bits before allowing advancement. The pattern typically flows: eligible, rules, hunting, multiple component bits, obtained, finished.

### Cheat Detection Pattern

Major equipment quests implement cheat detection by validating quest state consistency. When a player triggers a later quest stage, the spec procedure verifies all prerequisite bits are set. If the player somehow advanced without completing earlier stages, a dedicated cheat bit is set, progress is revoked with remQuestBit, and the attempt is logged to LOG_CHEAT. The cheat bit causes all future quest interactions to fail with rejection dialogue.

### Solo Kill Tracking Pattern

Quests requiring solo mob kills use combat affects to detect group assistance. An AFFECT_COMBAT affect is attached to the mob when combat begins, storing a pointer to the initial attacker. The damage pipeline checks this affect and sets cheat bits if any other player participates. This ensures quest credit only goes to solo kills.

### Component Turn-In Pattern

Collection quests verify the player possesses required items by searching equipment with getObjFromEquipment. Upon successful verification, the component bit is set, the item is removed with extract_obj, and the mob provides dialogue about the next step. All component bits must be set before the quest can complete.

### Quest Completion Pattern

When all prerequisite stage bits are verified as set, the spec procedure sets the finished bit, clears the started bit, loads the reward object with read_object, adds it to the player's inventory, and displays completion dialogue. The finished bit prevents re-running the quest.

### Spell Unlock Pattern

Spell and skill unlock quests use a single eligible bit checked during spell casting. The casting code verifies hasQuestBit before allowing the spell to execute. If the bit is not set, casting fails with an educational message. Quest completion sets this bit, permanently unlocking the ability.

### Test Code Pattern

Builders use dedicated test toggle bits (TOG_TESTCODE1 through TOG_TESTCODE5 and TOG_QUESTCODE1 through TOG_QUESTCODE4) to develop and debug quest logic without affecting actual quest state. These bits are manipulated through immortal commands and do not persist in production quest workflows.

### Quest Help Display Pattern

The quest command iterates the toggles array backwards from MAX_TOG_INDEX to zero, checking for set bits that have corresponding help files. It counts matching files and allows the player to specify a number to view a specific quest's help. The help file path is constructed as mobdata/responses/help/{bit_number} and paged to the player's descriptor.

### Experience Modification Pattern

Quest bits can modify experience gain by being checked in the gain_exp function. Cursed states like TOG_FAE_TOUCHED halve experience gain, while penalty states like TOG_NO_XP_GAIN prevent all experience. The checks occur before experience is awarded.

### Eligibility Gating Pattern

Level-based quests like warrior and thief progression use eligible bits that are set automatically when the character reaches the required level. Spec procedures check the eligible bit before presenting quest dialogue, ensuring players cannot access content prematurely.

## Reference

### Quest Bit Functions

hasQuestBit checks if a quest bit is set by indexing the toggles array. It validates the bit number is within bounds zero to MAX_TOG_INDEX minus one, logging to LOG_BUG and returning false for invalid values. For valid bits it returns the array value at that index.

setQuestBit sets a quest bit by performing bitwise OR with 0x1 on the toggles array element. It validates bounds identically to hasQuestBit and logs invalid attempts to LOG_BUG. Only bit zero is modified by the OR operation.

remQuestBit clears a quest bit by performing bitwise AND with the complement of 0x1 on the toggles array element. It validates bounds and logs identically to the other functions. Only bit zero is modified by the AND operation.

doMortalQuest implements the quest command. Without arguments it counts all set bits that have help files and displays the total. With a numeric argument it displays help for that quest number by iterating backwards through the toggles array, counting help files until reaching the requested number, then paging the help file to the player's descriptor via file_to_sstring and page_string. Immortals can specify bit numbers directly to view any quest help file.

### Major Quest Bit Ranges

Equipment quests use extensive bit ranges: Avenger occupies bits 1-8 with stages for eligible, rules, hunting, started, obtained, finished, cheat, and penanced. Vindicator spans bits 9-26 with 18 total bits. Silverclaw occupies bits 28-43 for 16 bits. Holy Devastator is the most complex at bits 44-77 with 34 total bits including five dedicated cheat detection bits.

Monk sash quests use 40 bits total across seven colored sashes. Each sash follows a three-bit pattern: eligible, started, finished. White sash is bits 56-58, progressing through yellow, purple, blue, green, red, and black at bits 59-75.

Warrior progression quests span bits 78-113 with four level-gated quests at levels 7, 14, 21, and 40. Each quest uses multiple bits for stages and completion tracking.

Thief progression quests are the most extensive at 70 bits starting at bit 345, with level-gated quests at levels 5, 10, 15, 20, and 25, each with numerous intermediate stages.

Spell unlock quests use single bits for eligibility: tornado at 50, barkskin at 51, earthquake at 52, lava at 53, flatulence at 54, plasma mirror at 55, with additional unlock bits scattered throughout the range.

Test code toggles are allocated at bits 11-15 for TESTCODE1-5 and bits 18-21 for QUESTCODE1-4.

### Quest Help Files

Help files are plain text files stored at lib/mobdata/responses/help with the filename being the decimal quest bit number. The quest command constructs paths as mobdata/responses/help/{bit_number}, opens the file with fopen, and pages content to players. Files can contain any text format. Common structure includes quest name, current stage, objectives, and return instructions. Only bits with existing help files are counted by the quest command.

### Integration Points

Experience system integration occurs in gain_exp where quest bits modify or block experience gain. Spell and skill access checks happen in casting code where hasQuestBit gates ability usage. Equipment loading occurs in spec procedures where quest completion bits enable reward object instantiation. Mob responses use quest bits to select dialogue branches in response files. Combat integration happens in the damage pipeline where affects track solo kill requirements and set cheat bits on group participation.

## Implementation

### Data Storage Architecture

Each TBeing and TPerson contains a toggles member declared as ubyte toggles[MAX_TOG_INDEX] where MAX_TOG_INDEX equals 454. Each array element is a single unsigned byte but only the least significant bit is used as a boolean flag. The array is zero-indexed with valid indices from 0 to 453.

All quest bit constants are defined in toggle.h as const int values. The constants serve as semantic anchors for array indexing, making quest logic readable rather than using magic numbers. The constants are organized by quest category with sequential numbering for multi-stage quests.

The toggles array is part of the charFile struct persisted in binary player files at lib/mutable/player locations. The save system writes the entire array as a fixed-size block. Some quest bits are mirrored to the playertoggle database table for web interface queries.

### Quest Bit Manipulation Implementation

The hasQuestBit implementation first validates the value parameter is non-negative and less than MAX_TOG_INDEX. Invalid values trigger a vlogf call to LOG_BUG with a format string showing the bad value and return false. Valid values return the toggles array element cast to boolean.

The setQuestBit implementation performs identical validation, logging bad values to LOG_BUG and returning early. For valid values it executes toggles[value] OR-equals 0x1, ensuring only bit zero is set regardless of other bits in the byte. This prevents corruption from multi-threaded access or uninitialized memory.

The remQuestBit implementation validates identically and executes toggles[value] AND-equals the bitwise complement of 0x1, clearing only bit zero. This preserves any higher bits that might be set.

Both TBeing and TPerson implement these three functions with identical code, allowing quest bit manipulation on both player characters and mob instances.

### Quest Command Implementation

The doMortalQuest function begins by sending "Your current quest status:" to the character. If the character's GetMaxLevel exceeds MAX_MORT, immortal-specific code allows direct bit number lookup.

For mortals the function converts the argument string to an integer questNumber with convertTo, defaulting to 1 if zero or negative. It initializes totFound to zero and questRes to negative one.

A for loop iterates from MAX_TOG_INDEX minus one down to zero, checking hasQuestBit for each index. If a bit is set, the function constructs a path as mobdata/responses/help/{index} and attempts to fopen the file. If the file exists it increments totFound, closes the file, and checks if totFound equals questNumber. If so it stores the index in questRes.

After iteration it sends a message showing the total count in totFound. If questRes is still negative one it sends an error message that the requested quest number does not exist and returns.

For valid questRes it constructs the help file path again, loads the file content into a string with file_to_sstring, and pages the string to the player's descriptor with desc->page_string.

### Cheat Detection Implementation

Cheat detection occurs in spec procedures by checking quest state consistency. When a player attempts to advance to stage N, the spec procedure verifies all bits for stages 1 through N minus 1 are set. If any prerequisite bit is missing, the spec procedure sets a dedicated cheat bit, calls remQuestBit to revoke the current stage progress, logs the attempt with vlogf to LOG_CHEAT including the character name, and returns early with rejection dialogue.

The Devastator quest implements the most sophisticated cheat detection with five separate cheat bits for different violation types. Each spec procedure interaction validates multiple prerequisite conditions and sets the appropriate cheat bit when violations are detected.

Once a cheat bit is set, all future quest interactions check it first and immediately return with hostile dialogue, permanently blocking quest completion until an immortal manually clears the cheat bit.

### Solo Kill Tracking Implementation

Solo kill requirements are enforced through the combat affect system. When combat begins for a quest requiring solo kills, an AFFECT_COMBAT affect is created with the affect modifier set to COMBAT_SOLO_KILL and the affect being pointer set to the initial attacker.

During the damage pipeline in reconcileDamage, the code iterates through all affects on the victim. For each AFFECT_COMBAT affect with modifier COMBAT_SOLO_KILL, it checks if the current attacker differs from the stored being pointer. If they differ, it sets the cheat bit on the original attacker with setQuestBit, removes the affect from the victim with affectRemove, and prevents quest credit.

This implementation ensures quest credit only goes to players who complete the entire kill without any assistance.

### Quest Completion Reward Implementation

When a spec procedure determines all completion conditions are met, it calls setQuestBit with the quest's finished bit constant. It typically clears the started bit with remQuestBit to reset state. It loads the reward object by calling read_object with the reward's vnum and VIRTUAL mode. It adds the loaded object to the character's inventory using the operator+= overload. It sends completion dialogue via doTell or sendTo. The finished bit prevents re-entering the quest on subsequent interactions.

### Experience Modification Implementation

Quest bit checks in gain_exp occur before experience is awarded to the character. The function calls hasQuestBit for modifier bits like TOG_FAE_TOUCHED and applies mathematical operations to the gain variable, such as dividing by two for curse effects. For penalty bits like TOG_NO_XP_GAIN it returns early without awarding any experience. These checks allow quest states to dynamically modify character progression rates.

### Spell Unlock Implementation

Spell and skill gating occurs in the casting code before spell execution. The code checks hasQuestBit with the spell-specific unlock bit constant. If the bit is not set it sends an educational message to the character such as "You have not learned this spell yet" and returns SPELL_FAIL to abort casting. If the bit is set casting proceeds normally. Quest completion sets the unlock bit permanently, enabling the spell for all future use.

## Troubleshooting

### Quest Bit Not Persisting After Server Restart

The toggles array save occurs during character file write operations. If a quest bit is set but does not persist across restarts, the player file may not have been saved before shutdown. Call doQueueSave on the character after setting quest bits to force immediate persistence. Check that the player file timestamp at lib/mutable/player/{first_letter}/{charname} updated after the quest interaction. Verify the toggles array is being written correctly by examining the binary file size matches expectations for the charFile struct.

### Quest Command Shows Wrong Quest Count

The quest command only counts bits that have corresponding help files at lib/mobdata/responses/help/{bit_number}. If a quest bit is set but the count is incorrect, verify the help file exists with the exact bit number as the filename. Check file permissions ensure the server can read the help directory. Confirm the help file is not empty, as empty files may be skipped.

### Cheat Bit Set Incorrectly

Cheat bits are set when prerequisite validation fails in spec procedures. If a cheat bit is set incorrectly, examine the spec procedure logic for the quest to identify which validation check triggered. Common causes include quest bits from previous versions not being cleared, teleportation bypassing required travel stages, or timing issues where prerequisite bits are checked before being set. Immortals can clear cheat bits manually with the set command: set quest {player} {cheat_bit_number} 0.

### Quest Help File Not Displaying

If the quest command finds a quest but file_to_sstring fails, check the file path construction in doMortalQuest uses mobdata/responses/help as the relative path from the lib directory. Verify the server's working directory is set correctly so relative paths resolve properly. Check file encoding is plain ASCII or UTF-8 without BOM. Examine file permissions ensure read access.

### Quest Cannot Be Restarted After Completion

Quest restart prevention is intentional design using the finished bit check. If a quest should be repeatable, the spec procedure must clear the finished bit when giving the reward or provide explicit restart logic. If a player needs to restart a completed quest, an immortal must manually clear the finished bit with remQuestBit or the set command.

### Experience Not Modified Despite Quest Bit Set

Experience modification occurs in gain_exp which may not be called during all experience gain scenarios. Verify the specific modifier bit is being checked in gain_exp. Confirm the character is actually gaining experience by checking combat messages and level progress. Some experience modifications only apply to specific types of gains such as combat experience versus quest experience. Check for other quest bits that might override or conflict with the expected modifier.

### Spec Procedure Not Recognizing Quest Bit

If a spec procedure does not recognize a quest bit that appears set in the character data, verify the spec procedure is using the correct quest bit constant from toggle.h. Check for typos in constant names. Confirm the spec procedure is actually executing by adding temporary sendTo calls. Verify hasQuestBit is being called on the character pointer not a mob pointer. Check the character pointer is valid and not null before calling hasQuestBit.

### Quest Bit Lost After Polymorph or Switch

Polymorph and switch operations transfer character data to different objects. If quest bits are lost during these operations, verify the toggles array is being copied during character transfer. Check that original pointers are being preserved correctly. Examine the switch and polymorph code to ensure toggles array persistence. Quest bits should persist across these operations as they are fundamental character data.

### Invalid Quest Bit Logging

LOG_BUG messages about invalid quest bits indicate out-of-bounds array access attempts. Check the calling code is using valid quest bit constants from toggle.h, not hardcoded integers. Verify MAX_TOG_INDEX has not changed between compilation and runtime. Confirm no quest bits exceed 453. Check for integer overflow or type conversion issues if bit numbers are calculated dynamically.

### Database Quest Bit Sync Issues

Some quest bits are mirrored to the playertoggle database table for web interface access. If web displays show incorrect quest state, verify the database sync code is calling appropriate insert or update statements after setQuestBit and remQuestBit. Check database table structure matches expectations. Confirm database connection is active and queries are not failing silently. Examine web interface queries ensure they join player and playertoggle tables correctly.
