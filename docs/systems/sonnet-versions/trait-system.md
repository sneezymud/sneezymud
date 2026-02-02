---
title: Trait System
category: understanding
keywords: [traits, character creation, advantages, disadvantages, bonus points, quest bits, permanent attributes, TOG_IS_COWARD, TOG_IS_BLIND, TOG_HAS_NIGHTVISION, TOG_FAE_TOUCHED, TOG_PERMA_DEATH_CHAR]
related: [race-system.md, stats-attributes.md, experience-leveling.md, quest-system.md, character-foundation.md]
primary_symbols:
  functions: [nannyTraits_input, saveToggles, loadToggles, numFifties, hasQuestBit]
  classes: [TTraits, TPerson]
  files: [code/code/sys/create_character.cc, code/code/misc/toggle.h, code/code/misc/rent.cc, code/code/misc/player_data.cc]
---

## Overview

The trait system allows players to customize characters during creation by selecting permanent advantages and disadvantages. Positive traits cost stat points while negative traits grant bonus points that can be spent on character statistics. The system provides 17 traits ranging from -100 points (expensive benefit) to +10 points (major penalty).

**Core Mechanism:** Traits are stored as quest bits and checked at runtime via hasQuestBit. This reuses existing quest infrastructure rather than maintaining separate storage. Once selected, traits are permanent and cannot be changed.

**Entry Point:** Three connection states (CON_CREATION_TRAITS1/2/3) split the 17-trait selection interface across multiple screens. Players toggle traits on or off, with the system tracking bonus point totals and enforcing level requirements.

**Storage Model:** Trait toggles persist in toggle files located at lib/mutable/player/{first_letter}/{name}.toggle as space-separated toggle numbers. The saveToggles and loadToggles functions in rent.cc handle persistence, writing all quest bits including trait selections.

**Access Control:** Some traits require account progression. The num50race and num50any requirements verify the account has sufficient level 50 characters before allowing trait selection. Certain races disable specific traits via TPlayerRace::disableTrait.

**Design Rationale:** By leveraging quest bits, traits integrate seamlessly with existing persistence and state management. This avoids duplicating storage infrastructure and allows traits to be checked using the same hasQuestBit pattern used throughout the codebase.

## Patterns

### Trait Definition Pattern

Each trait is defined using the TTraits structure in connect.h, which combines toggle constant, point value, display information, and access requirements. The traits array in create_character.cc orders all 17 traits by point value for display clarity.

Penalty traits have positive point values and grant bonus stats. The cowardice trait (+10 points) uses TOG_IS_COWARD and auto-flees at 50% HP. Blindness (+10 points) uses TOG_IS_BLIND and permanently sets AFF_BLIND. Asthma (+8 points) uses TOG_IS_ASTHMATIC and halves maximum movement. Narcolepsy (+8 points) uses TOG_IS_NARCOLEPTIC and triggers random sleep episodes. Mute (+5 points) uses TOG_IS_MUTE and blocks all speech commands. Combustible (+5 points) uses TOG_IS_COMBUSTIBLE and triggers spontaneous combustion. Hemophilia (+5 points) uses TOG_IS_HEMOPHILIAC and doubles bleeding duration. Necrophobia (+5 points) uses TOG_IS_NECROPHOBIC and triggers fear responses to corpses. Alcoholism (+5 points) uses TOG_IS_ALCOHOLIC and limits thirst satisfaction to alcohol. Tourettes (+1 point) uses TOG_HAS_TOURETTES and causes involuntary insults.

Benefit traits have negative point values and cost stat points. Healthy (-8 points) uses TOG_IS_HEALTHY and adds 75% disease immunity. Nightvision (-8 points) uses TOG_HAS_NIGHTVISION and adds +2 vision bonus. Ambidextrous (-10 points) uses TOG_IS_AMBIDEXTROUS and grants equal facility with both hands. Psionics (-100 points) uses TOG_PSIONICIST and enables innate psionic abilities.

Neutral traits have zero point cost. Perma-death uses TOG_PERMA_DEATH_CHAR and deletes the character on death. Real aging uses TOG_REAL_AGING and makes age affect gameplay. Fae-touched uses TOG_FAE_TOUCHED and grants random stat bonuses at creation but halves XP gain.

### Selection Workflow Pattern

The nannyTraits_input function handles trait toggling during character creation. When a player selects a trait number, the function checks if the trait is already selected using hasQuestBit. If selected, it calls remQuestBit and subtracts the point value from bonus_points.total. If not selected, it validates level requirements against num50race and num50any before calling setQuestBit and adding the point value.

Race restrictions apply during display generation. The traits array is filtered by TPlayerRace::disableTrait, which currently blocks blindness for goblins and orcs. The display shows [X] for selected traits, [*] for unavailable traits, and [ ] for available traits.

Level requirement validation uses numFifties to count qualifying characters on the account. The num50race field requires specific race matches, considering perma-death status separately. The num50any field counts all races together. Requirements like "1 L50 any" appear in trait descriptions and gate access to powerful or disruptive traits.

Bonus point distribution occurs after trait selection completes. The total bonus_points.total value is divided by four and distributed into combat, combat2, learn, and util categories. Excess points are distributed round-robin. Players cannot exit creation with negative points in any category.

### Runtime Effect Pattern

Each trait effect checks hasQuestBit at the appropriate trigger point. These checks occur during character load, periodic ticks, command execution, or calculation functions depending on the trait's nature.

Load-time effects apply permanent modifications. The loadToggles function restores quest bits from the toggle file when the character loads. Blindness sets AFF_BLIND in player_data.cc. Nightvision adds vision bonus in player_data.cc. Cowardice sets wimpy to maxWimpy in connect.cc.

Periodic tick effects trigger in periodic.cc during updatePeriodic. Narcolepsy has a 1% chance per tick to apply sleep affects. Combustible has a 1% chance per tick to call flameEngulfed. Necrophobia has a 25% chance per tick to scan for corpses and undead. Tourettes has a 25% chance per tick to generate random insults.

Command effects block or modify player actions. Mute checks occur in talk.cc for say, shout, tell, whisper, ask, order, and emote commands. Cowardice prevents changing wimpy setting via toggle.cc.

Calculation effects modify numeric values. Asthma halves movement points in limits.cc. Healthy adds 75% disease immunity in immunity.cc. Hemophilia doubles bleeding duration in magicutils.cc. Alcoholism modifies thirst satisfaction in obj_food.cc. Ambidextrous overrides isAmbidextrous in being.h. Fae-touched halves XP gain in limits.cc.

### Persistence Pattern

Trait persistence uses the existing toggle save/load infrastructure. The saveToggles function in rent.cc iterates through all MAX_TOG_INDEX values and writes each set quest bit as a space-separated number to the toggle file. The loadToggles function reads these numbers back and calls setQuestBit for each one.

Toggle files store all quest bits together, not just traits. This means trait toggles (278-299 range) share storage with quest progress flags. The toggle number range must remain stable across code changes because these numbers are stored in player files.

The storeToSt function in player_data.cc calls saveToggles during character save operations. This ensures trait selections persist whenever the character is saved to disk. The loadCharacter path calls loadToggles to restore the full quest bit state.

Race-specific requirements persist across character creation cycles. The numFifties function queries the account's character history to count qualifying level 50 characters. For perma-death fae-touched characters, only perma-death level 50s of the same race count. For non-perma characters, only non-perma level 50s count.

## Reference

### TTraits Structure

Defined in connect.h with MAX_TRAITS constant set to 17. The tog field holds the toggle constant. The points field stores the point value (positive for penalty, negative for benefit). The name field contains the display name. The desc field holds the in-game description. The num50race field specifies required level 50 characters of the same race. The num50any field specifies required level 50 characters of any race.

### Trait Catalog

Cowardice (index 0) grants +10 points via TOG_IS_COWARD. Sets auto-flee at 50% HP and prevents wimpy changes. No level requirements.

Blindness (index 1) grants +10 points via TOG_IS_BLIND. Permanently sets AFF_BLIND affect. Requires 1 L50 any. Disabled for goblins and orcs.

Asthma (index 2) grants +8 points via TOG_IS_ASTHMATIC. Halves maximum movement. No level requirements.

Narcolepsy (index 3) grants +8 points via TOG_IS_NARCOLEPTIC. Triggers random sleep episodes at 1% per tick. No level requirements.

Mute (index 4) grants +5 points via TOG_IS_MUTE. Blocks speech commands except tells to immortals. Requires 1 L50 any.

Combustible (index 5) grants +5 points via TOG_IS_COMBUSTIBLE. Causes spontaneous combustion at 1% per tick. No level requirements.

Hemophilia (index 6) grants +5 points via TOG_IS_HEMOPHILIAC. Doubles bleeding duration and makes limb bleeds permanent. No level requirements.

Necrophobia (index 7) grants +5 points via TOG_IS_NECROPHOBIC. Triggers fear responses to corpses and undead. No level requirements.

Alcoholism (index 8) grants +5 points via TOG_IS_ALCOHOLIC. Limits thirst satisfaction to alcohol only. No level requirements.

Tourettes (index 9) grants +1 point via TOG_HAS_TOURETTES. Causes random involuntary insults. Requires 1 L50 any.

Perma-death (index 10) costs 0 points via TOG_PERMA_DEATH_CHAR. Deletes character on death. Requires 1 L50 any.

Real aging (index 11) costs 0 points via TOG_REAL_AGING. Makes age affect gameplay over time. Requires 1 L50 any.

Fae-touched (index 12) costs 0 points via TOG_FAE_TOUCHED. Grants random stat bonuses scaling with num50race (50 + (num50race-1)*2 points, capped at 26 fifties). Halves XP gain. Requires 1 L50 same race.

Healthy (index 13) costs -8 points via TOG_IS_HEALTHY. Adds +75% disease immunity. No level requirements.

Nightvision (index 14) costs -8 points via TOG_HAS_NIGHTVISION. Adds +2 vision bonus. No level requirements.

Ambidextrous (index 15) costs -10 points via TOG_IS_AMBIDEXTROUS. Grants equal facility with both hands regardless of DEX. No level requirements.

Psionics (index 16) costs -100 points via TOG_PSIONICIST. Enables innate psionic abilities. Requires 1 L50 any.

### Connection States

CON_CREATION_TRAITS1 displays traits 1-6. CON_CREATION_TRAITS2 displays traits 7-12. CON_CREATION_TRAITS3 displays traits 13-17. Each state calls nannyTraits_input for selection processing.

### Toggle Constants

Trait toggles occupy the 278-299 range in toggle.h. TOG_IS_COWARD is 278. TOG_IS_BLIND is 279. TOG_IS_ASTHMATIC is 280. TOG_IS_NARCOLEPTIC is 281. TOG_IS_MUTE is 282. TOG_IS_COMBUSTIBLE is 283. TOG_IS_HEMOPHILIAC is 284. TOG_IS_NECROPHOBIC is 285. TOG_IS_ALCOHOLIC is 286. TOG_HAS_TOURETTES is 287. TOG_PERMA_DEATH_CHAR is 288. TOG_REAL_AGING is 289. TOG_FAE_TOUCHED is 290. TOG_IS_HEALTHY is 291. TOG_HAS_NIGHTVISION is 292. TOG_IS_AMBIDEXTROUS is 293. TOG_PSIONICIST is 299.

### Display Commands

The attribute personal command in cmd_attribute.cc iterates through MAX_TRAITS and displays names for all traits where hasQuestBit returns true. Output format is "Your character traits are: {names}".

The who command in cmd_who.cc supports filtering. "who !" shows fae-touched characters. "who x" shows perma-death characters.

### File Locations

Toggle files reside at lib/mutable/player/{first_letter}/{name}.toggle. Example: lib/mutable/player/a/alice.toggle. Format is space-separated toggle numbers like "278 282 289".

## Implementation

### Initialization and Validation

The traits array in create_character.cc initializes all 17 TTraits structures at file scope. Array ordering follows point values from highest (most beneficial penalty) to lowest (most expensive benefit). Each entry specifies tog constant, points value, name string, description string, num50race requirement, and num50any requirement.

The nannyTraits_input function validates selection attempts by calling numFifties twice. The first call passes the character's getRace result and hasQuestBit(TOG_PERMA_DEATH_CHAR) status to count race-specific fifties. The second call passes RACE_NORACE and false to count all fifties. These counts compare against the trait's num50race and num50any fields.

Race restriction checking occurs during display generation. The TPlayerRace::disableTrait method returns true for forbidden trait indices. Currently only goblins and orcs disable trait index 1 (blindness). The display loop marks disabled traits with [*] regardless of level requirements.

Input parsing uses convertTo<int> to extract the trait index from player input. Invalid indices and out-of-range values are rejected silently. Valid indices trigger the toggle logic: already-selected traits are removed via remQuestBit and their points subtracted, unselected available traits are added via setQuestBit and their points added.

### Effect Implementation Locations

Cowardice implementation spans toggle.cc and connect.cc. The hasQuestBit check in toggle.cc prevents changing wimpy setting. The check in connect.cc sets wimpy to maxWimpy during login processing.

Blindness implementation occurs in player_data.cc within the character load path. After loading base data, the hasQuestBit check sets AFF_BLIND using SET_BIT on specials.affectedBy.

Asthma implementation modifies movement calculation in limits.cc. The getMoveLimit function computes iMax normally, then halves it if hasQuestBit(TOG_IS_ASTHMATIC) returns true.

Narcolepsy implementation resides in periodic.cc within updatePeriodic. The 1% random check calls number(0,99) and proceeds only on zero. The effect creates an affectedData struct with AFFECT_DUMMY type and duration matching the sleep spell, then calls setPosition(POSITION_SLEEPING).

Mute implementation appears throughout talk.cc in multiple command functions. Each speech command (say, shout, tell, whisper, ask, order, emote) checks hasQuestBit(TOG_IS_MUTE) and returns FALSE with a message. The tell command exempts immortal targets from this check.

Combustible implementation lives in periodic.cc next to narcolepsy. The 1% random check calls flameEngulfed which deals fire damage. The return value is checked with IS_SET_DELETE(rc, DELETE_THIS) and propagated if the character dies.

Hemophilia implementation modifies bleeding in magicutils.cc. The updateDuration lambda checks hasQuestBit(TOG_IS_HEMOPHILIAC) and doubles duration if true. Limb bleeding checks the flag separately to prevent natural healing.

Necrophobia implementation occupies a large block in periodic.cc. The 25% random check scans room contents for corpses using isCorpse checks and undead beings using isUndead checks. Finding either triggers fear messages and flee attempts.

Alcoholism implementation modifies drinking in obj_food.cc. The drinkMe function checks hasQuestBit(TOG_IS_ALCOHOLIC) and branches. Alcoholic drinks call gainCondition with (getLiqDrunk * amount) / 10. Non-alcoholic drinks stop satisfying at 3 thirst units with a blocking message.

Tourettes implementation generates random insults in periodic.cc. The 25% random check builds a list of visible beings excluding self, selects one randomly, and constructs an insulting message using random insult text.

Healthy implementation boosts disease immunity in immunity.cc. The getImmunity function checks hasQuestBit(TOG_IS_HEALTHY) when type equals IMMUNE_DISEASE and adds 75 to the imm value.

Nightvision implementation adds vision bonus in player_data.cc. The visionBonus calculation checks hasQuestBit(TOG_HAS_NIGHTVISION) and adds 2.

Ambidextrous implementation overrides the isAmbidextrous method in being.h. The inline method checks hasQuestBit(TOG_IS_AMBIDEXTROUS) first and returns true, bypassing the normal DEX and skill checks.

Fae-touched creation bonus applies in create_character.cc after trait selection. The code calls numFifties to get race-specific count, clamps it to max 26, then calls addToRandomStat with 50 + (num_fifties - 1) * 2.

Fae-touched XP penalty applies in limits.cc within the experience gain function. The hasQuestBit check halves gain using "gain /= 2". A fae_reduction_done flag ensures the penalty applies only once for multiclass characters.

### Persistence Implementation

The saveToggles function in rent.cc constructs the file path using "mutable/player/%c/%s.toggle" format with LOWER(name[0]) and lowercased name. It opens the file for writing, iterates num from 1 to MAX_TOG_INDEX, and writes "%d " for each number where hasQuestBit returns true. The file is flushed and closed.

The loadToggles function follows the same path construction. It opens for reading and loops calling fscanf(fp, "%d ", &num) until EOF. Each successfully read number is passed to setQuestBit. The file is closed after reading completes or fails.

The storeToSt function in player_data.cc calls saveToggles early in the save process, before writing the charFile structure. This ensures toggle state is flushed even if the main save fails partway through. The matching loadCharacter path calls loadToggles after loading the charFile data.

Error handling is minimal. Failed file opens are silently ignored in loadToggles, treating missing toggle files as having no quest bits set. The saveToggles function does not check for write failures. File permissions and disk space issues may cause silent data loss.

### Bonus Point Distribution

After trait selection completes, the bonus_points.total value is divided by 4 to get a base allocation per category. The four categories (combat, combat2, learn, util) each receive this base amount. Remaining points after division are distributed one per category in order until exhausted.

The creation code validates that no category has negative points before allowing the player to proceed. Since penalty traits add points and benefit traits subtract points, a character selecting expensive benefits without sufficient penalties will have negative totals. The player must adjust trait selections until all categories are non-negative.

Race restrictions interact with point distribution by limiting which penalty traits are available. Goblins and orcs cannot select blindness, reducing their maximum potential bonus points by 10. This affects min-max strategies for those races.

## Troubleshooting

### Trait Not Appearing Selected

If attribute personal does not show a trait that was selected during creation, check the toggle file at lib/mutable/player/{first_letter}/{name}.toggle. The file should contain the toggle number for that trait. If missing, the saveToggles call may have failed silently during character creation.

Verify the toggle constant matches between create_character.cc traits array and toggle.h definitions. Mismatched constants will cause the wrong toggle to be set/checked.

Check for race restrictions via TPlayerRace::disableTrait. Goblins and orcs attempting to select blindness will have it ignored despite appearing selectable in the UI.

### Trait Effect Not Working

If a trait's gameplay effect does not occur, verify the hasQuestBit check uses the correct toggle constant. Each effect implementation must check the exact constant defined in the traits array.

For periodic effects (narcolepsy, combustible, necrophobia, tourettes), confirm the character is ticking. Sleeping or resting characters may not trigger periodic checks depending on position requirements.

For command-blocking effects (mute), verify the command path includes the hasQuestBit check. Some communication paths may bypass the check, particularly system messages or immortal-specific channels.

For calculation effects (asthma, healthy, nightvision, ambidextrous), confirm the calculation occurs after character load when quest bits are available. Effects checked before loadToggles runs will not see trait selections.

### Bonus Points Miscalculation

If bonus points do not match expected trait values, verify the nannyTraits_input function is correctly adding or subtracting the trait's points value. Toggle-off should subtract, toggle-on should add.

Check that the points field in the traits array matches the documented value. Mismatched point values will cause incorrect totals during selection.

Verify the point distribution logic divides total by 4 and distributes remainders. Hand-calculating expected values should account for integer division truncation.

### Level Requirement Issues

If traits requiring level 50 characters are not selectable, verify numFifties is correctly counting qualifying characters. The function must match race for num50race requirements and respect perma-death status.

For fae-touched specifically, the num50race check considers perma-death status. Creating a perma-death fae-touched character requires perma-death fifties of the same race, not regular fifties.

Check account name is correctly passed to numFifties. Character counts are per-account, so incorrect account names will return zero qualifying characters.

### Persistence Failures

If traits do not persist across logout/login, verify the toggle file is being written. Check file system permissions on lib/mutable/player/ directories and ensure they are writable by the MUD process.

Confirm loadToggles is called during character load. Missing the call will leave quest bits uninitialized, causing all traits to appear unselected.

Verify no code is calling remQuestBit for trait toggles during normal gameplay. Traits should be permanent; any code removing trait toggles outside of character deletion is a bug.

Check that MAX_TOG_INDEX is sufficiently large to include all trait toggle numbers. If trait toggles exceed MAX_TOG_INDEX, saveToggles will not write them.

### Fae-Touched Stat Bonus Missing

If fae-touched characters do not receive the creation stat bonus, verify numFifties returns a count greater than zero. The bonus only applies if num_fifties > 0.

Check that addToRandomStat is being called with the correct calculation: 50 + (num_fifties - 1) * 2. The minimum bonus is 50 points for 1 fifty, maximum is 100 points for 26 fifties.

Verify the bonus is applied before entering the game. The creation path should call addToRandomStat during the CON_CREATION_TRAITS flow, not during normal gameplay.

### Fae-Touched XP Penalty Not Applying

If fae-touched characters gain full XP, verify the hasQuestBit check in limits.cc is reached. The check must occur after quest bits are loaded.

Confirm the fae_reduction_done flag is properly scoped. The flag prevents double-application for multiclass characters but should not persist across separate XP gain events.

Check that the gain value is actually halved. Debug output before and after the "gain /= 2" line should show the value reduced by 50%.
