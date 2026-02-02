---
title: Monster AI and Behavior
category: important
keywords: [NPC behavior, aggression, hunting, pathfinding, opinions, hatred, fear, targeting, charList, DELETE flags, mobileActivity]
related: [combat-rounds.md, memory-safety.md, movement-terrain-navigation.md, character-foundation.md]
primary_symbols:
  functions: [mobileActivity, aggroCheck, hunt, setHunting, targetFound, dirTrack, addHated, Hates, developHatred, takeFirstHit, senseWimps, assistFriend]
  classes: [TMonster, Mobile_Attitude, opinionData, charList, Responses]
  files: [code/code/misc/mobact.cc, code/code/misc/opinion.cc, code/code/misc/monster.cc, code/code/misc/ai_utility.cc]
---

# Monster AI and Behavior

## Overview

The monster AI system gives NPCs autonomous behavior through personality modeling, memory, and reactive decision-making. TMonster instances evaluate their environment every 1.2 seconds via mobileActivity, choosing between aggression, hunting, fear responses, scavenging, and class-specific combat tactics.

The system operates on three conceptual layers. The personality layer uses Mobile_Attitude to model emotional state through four values—suspicion, greed, malice, anger—each affecting probability of different behaviors. The memory layer tracks specific characters and categorical traits the mob hates or fears via opinionData structures containing charList linked lists and demographic filters. The action layer implements decision trees for aggression targeting, pursuit pathfinding, and combat tactics.

State is managed through raw pointers with strict ownership rules. The specials.hunting pointer tracks pursuit targets but becomes dangling if the target is deleted. All AI functions return DELETE_THIS flags when the mob dies, requiring immediate return propagation to prevent use-after-free crashes. The charList destructor does not delete the chain, requiring manual iteration and deletion to avoid memory leaks.

Aggression is probabilistic rather than deterministic. The aggro function combines anger and malice thresholds with karma-based player protection and level difference guards. Smart mobs use senseWimps to score targets by health, armor, position, and tactical value. Faction mobs have hardcoded territorial behavior for Cult of Logrus versus Brotherhood of Galek conflicts.

Hunting implements multi-room pursuit with distance budgets scaled by level and hatred. Pathfinding through dirTrack uses breadth-first search with vision requirements, concealment counters, and portal handling. High-level clerics may use Summon or Astral Walk spells instead of walking. Persistence decrements per room, with return-to-home navigation when hunt fails.

Common failure modes include not checking DELETE flags from movement functions, accessing stale hunting pointers after target death, forgetting to iterate-and-delete charList chains, and continuing execution after takeFirstHit returns DELETE_VICT. The system also has subtle interactions with combat group management, polymorphed players, and peaceful room restrictions.

## Patterns

### Adding and Managing Hatred

Call addHated with the target being to add them to the mob's charList. The function calculates hate strength based on combined levels and focus stat, storing duration in game hours. Hatred persists across logout unless explicitly removed. Use Hates to check if a mob currently hates a target by name, race, sex, class, or vnum. Call remHated to remove a specific character from the hate list, or remHatred to clear categorical hatred by bitfield.

The developHatred function is called automatically during combat from damage handlers. It uses a patience formula comparing current HP percentage against a difficulty threshold modified by level difference. High-level mobs attacked by low-level players develop hatred quickly to prevent hit-and-flee exploitation. The formula is patience plus random variance less than difficulty, where difficulty increases with level gap.

When adding feared characters with addFeared, the function automatically clears hunting state if the mob is currently hunting that target. This prevents paradoxical behavior where a mob simultaneously fears and pursues the same character.

### Initiating Aggression

Start with aggroCheck called from mobileActivity when not already fighting. The function first delegates to factionAggroCheck for territorial conflicts, then scans all characters in the room. It skips polymorphed players, applies karma checks against intelligent mobs, and uses level difference guards to prevent suicidal attacks. When multiple valid targets exist, selection is random rather than deterministic.

For wandered mobs outside their home zone, aggression is reduced against low-level players. Wimpy mobs only attack sleeping targets. The karma formula scales player karma zero to one hundred against mob intelligence plus anger, allowing high-karma players to avoid aggression from calm, intelligent mobs.

When a valid target is found, call takeFirstHit to initiate combat. Thieves use backstab or throat slit based on level. Other classes may use classStuff for opening moves. Always check the return value for DELETE_VICT if the victim dies or DELETE_THIS if the mob dies during opening strike.

### Setting Up Pursuit

Call setHunting with the target being when the mob should begin tracking. The function sets persistence to the mob's level, calculates tracking distance as fifty plus level, and doubles distance if the mob hates the target. It caches the birth room in oldRoom for return navigation and sets the ACT_HUNTING flag. The specials.hunting pointer is set to the raw target pointer.

Hunting executes in hunt called from mobileActivity. Check persistence—if zero or negative, return home or stop hunting. If the target is visible in the same room, call targetFound to initiate combat. Otherwise use dirTrack to find the next direction and call goDirection to move. Movement distance per tick scales by cube root of level, with probabilistic rounding for fractional components.

High-level clerics have a twenty percent chance to cast Summon for same-zone targets or Astral Walk for cross-zone targets instead of walking. Archers with SPEC_ARCHER shoot at visible targets before moving. Musk gas in rooms costs extra persistence.

Always check DELETE_THIS from goDirection and return immediately if set. Clear the hunting pointer and ACT_HUNTING flag when combat begins in targetFound or when hunt fails. Never access specials.hunting without validating it is non-null and still valid.

### Target Selection in Combat

Smart mobs level fifteen and higher call senseWimps during combat to select optimal targets. The function scores all combatants by summing hit points, hit limit, mana, armor contribution, and karma. Lower score means more attractive target. Position modifiers subtract for sitting, resting, sleeping. Mounted targets get plus five thousand penalty. NPCs get plus five hundred. Newbies under level five get plus seven fifty protection. Hated targets get minus three fifty priority. Wounded targets under thirty percent HP get minus two fifty finish priority.

The anti-tank detection checks if the mob has AFF_AGGRESSOR and is fighting a pet or zombie while a PC is not engaged. If the PC is using the pet as a tank, the mob switches target to the PC directly with a message about taking care of you first.

Use isFriend to determine if another being should be protected. Friends are same group members, same mob vnum, or same race and faction within five levels. Call assistFriend to have the mob join combat defending a friend. Police mobs have special behavior breaking up non-police fights and prioritizing murderers of other police.

### Cleaning Up Opinion Lists

When deleting a TMonster, manually iterate both hates.clist and fears.clist to delete the entire chain. Cache the next pointer before deleting each node. Set the clist pointer to null after deletion. The opinionData destructor only deletes the first node, not the chain.

When a character is being removed from the game world, call DeleteHatreds and DeleteFears to remove them from all mob opinion lists globally. These functions iterate all loaded mobs and call remHated and remFeared. This prevents dangling pointers in charList chains.

The charList structure stores name as a raw character pointer from mud_str_dup. Always delete array the name field before deleting the node. The destructor handles this, but manual cleanup requires explicit delete array.

### Handling DELETE Flags from AI

All AI functions that can cause death return DELETE_THIS if the mob should be removed. Check the return value from hunt, aggroCheck, takeFirstHit, assistFriend, and goDirection immediately. Use IS_SET_DELETE to test the flag. Return DELETE_THIS to the caller without further execution.

When propagating DELETE_VICT from takeFirstHit in targetFound, clear the specials.hunting pointer and ACT_HUNTING flag before returning. This prevents the caller from accessing stale state. When propagating DELETE_THIS from movement, return immediately without decrementing persistence or accessing member variables.

The mobileActivity function returns DELETE_THIS to the scheduler, which converts it to true for deletion. Never continue execution after detecting a delete flag. Never access this pointer or member variables after returning a delete flag.

## Reference

### Mobile_Attitude Class

Models emotional state with suspicion, greed, malice, anger values ranging zero to one hundred. Each attribute has current and default values. The target pointer stores current opinion target but only accepts PCs. The random pointer provides scratch space for interaction logic. The last_cmd field caches the last witnessed command type.

Accessor methods on TMonster include greed, anger, malice, susp for reading values and setGreed, setAnger, setMalice, setSusp for writing. The randomized check methods isGreedy, isAngry, isMalice, isSusp return true if random one hundred is less than the attribute value. Modifier methods US, DS, UG, DG, UA, DA, UM, DMal increase or decrease attributes by randomized amounts up to twice the parameter.

### opinionData Structure

Contains clist as a charList pointer for specific character tracking. The sex field stores hated or feared sex as sexTypeT. The race field stores hated or feared race as race_t. The Class field is a bitmask for hated or feared classes. The vnum field stores hated or feared mob virtual number. TMonster has two instances named hates and fears with corresponding bitfields hatefield and fearfield.

Hate and fear flags include HATE_SEX and FEAR_SEX for sex targeting, HATE_RACE and FEAR_RACE for race targeting, HATE_CHAR and FEAR_CHAR for character list activation, HATE_CLASS and FEAR_CLASS for class targeting, HATE_VNUM and FEAR_VNUM for mob vnum targeting.

### charList Structure

Forms singly-linked list via next pointer. The name field is a character pointer from mud_str_dup requiring delete array. The iHateStrength field stores duration in game hours calculated as combined max level plus five times focus stat over one twenty, yielding two to two nineteen hours. The account_id and player_id fields enable multi-character detection.

### Hunting State

The specials.hunting field on TMonster is a raw TBeing pointer to the pursuit target. The hunt_dist field stores remaining tracking distance in rooms. The persist field is a counter decrementing per tracking attempt, initialized to mob level. The oldRoom field caches birth room for return navigation. The ACT_HUNTING flag in specials.act marks active pursuit state.

### ACT Flags Affecting Behavior

ACT_SENTINEL prevents wandering from birth room. ACT_SCAVENGER enables item pickup. ACT_AGGRESSIVE attacks players on sight bypassing emotion checks. ACT_STAY_ZONE prevents leaving birth zone. ACT_WIMPY only attacks sleeping targets and flees at low HP. ACT_HATEFUL marks active hate list. ACT_AFRAID marks active fear list. ACT_HUNTING marks active tracking. ACT_IMMORTAL prevents gaining hatred.

### Aggression Threshold Formula

The aggro function combines emotion checks with categorical flags. If both isAngry and isMalice return true, the formula checks four times anger plus five times malice greater or equal to four fifty. ACT_AGGRESSIVE bypasses emotion checks entirely. Pets, charmed creatures, thralls, utility mobs, and guild mobs never aggro. The pissed function is a simpler check using only isAngry and isMalice without threshold, used for minor annoyances.

### Target Scoring in senseWimps

Base score sums current hit points, hit limit, mana, two thousand minus armor value, and karma scaled to two thousand. Position modifiers include plus five thousand for mounted, plus five hundred for NPCs, plus seven fifty for newbies under level five, minus four hundred for sitting, minus six hundred for resting, minus eight hundred for sleeping, minus three hundred for fighting the mob, minus three fifty for hated targets, minus two fifty for wounded under thirty percent HP.

### Tracking Distance Formulas

Base distance for setHunting is fifty plus GetMaxLevel, doubled if Hates returns true. Movement per hunt tick is cube root of GetMaxLevel with probabilistic rounding. For player doTrack command, rangers and thieves get two times skill value, warriors get skill divided by two, mages get skill plus level, others get skill value. Race modifiers include times two for giants and elves, MAX_ROOMS for devils and demons. Quest bit TOG_IS_CRAVEN subtracts twenty-five, TOG_IS_VICIOUS adds twenty-five. SPELL_TRAIL_SEEK adds fifty.

### dirTrack Vision Requirements

Requires one of roomp light plus vision bonus greater than zero, ROOM_ALWAYS_LIT flag, AFF_TRUE_SIGHT affect, AFF_CLARITY affect, or isImmortal status. Without sufficient vision, tracking fails with message about not seeing well enough. SKILL_CONCEALMENT affect on target blocks tracking probabilistically based on modifier value—one fifty modifier blocks one hundred percent, fifty modifier blocks thirty-three percent.

### Class-Specific Combat Functions

fighterMove for warriors uses bash, bodyslam, spin, kick, disarm. monkMove for monks uses springleap, hurl, bonebreak, shoulder throw, chi. thiefMove for thieves uses stab and disarm. mageMove for mages casts offensive spells from best discipline. clerMove for clerics casts healing and harm spells. shamanMove for shamans uses spirit spells and flatulence. deikhanMove for deikhans uses charge when mounted or fighter moves. rangMove for rangers uses nature skills.

### mobileActivity Timing Intervals

Every pulse handles spell effects, lag, position. Every two MOBACT pulses triggers rescue and spec procs. Every five MOBACT pulses handles charmee and protection. Every seven MOBACT pulses finds horses and assists friends. Every eleven MOBACT pulses scavenges. Every thirteen MOBACT pulses steals. Every sixteen MOBACT pulses removes stuck items. Every thirty MOBACT pulses returns to default position. Every fifty MOBACT pulses fires alignment spec procs.

### Response System Triggers

The Responses class contains respList linked list, respCount for number of responses, respMemory for interaction history. The resp structure contains cmd for triggering command type, args for pattern matching, cmds for command execution list, next for list traversal. loadResponses queries mobresponses database table. checkResponses is called from triggerSpecial. modifiedDoCommand executes with special handling for CMD_RESP_TOROOM, CMD_RESP_TOVICT, CMD_RESP_TONOTVICT message routing, CMD_LOAD and CMD_RESP_LOADMOB spawning, CMD_FLAG and CMD_RESP_UNFLAG quest manipulation.

### Friend Determination Logic

inGroup returns true if both have same master and AFF_GROUP flag. Same mob vnum determined by comparing mob_index virtual numbers. Race and faction match requires both isSameRace and isSameFaction returning true with level difference less than five or both over fifty.

## Implementation

The personality system stores four emotional attributes as unsigned short in Mobile_Attitude with separate current and default values. Accessor methods on TMonster delegate to the attitude member. Randomized check methods generate random zero to one oh one and compare against attribute value. Modifier methods add or subtract random amount up to twice the parameter, clamping to zero to one hundred range.

The opinion system uses opinionData containing categorical filters and a charList pointer. When adding hatred via addHated, the function allocates a new charList node, calls mud_str_dup for the name, calculates iHateStrength from level and focus formulas, stores player_id and account_id, and prepends to clist. The HATE_CHAR bit is set in hatefield. The Hates function checks each active bitfield category in sequence, returning true on first match.

When checking character list hatred, iteration walks clist comparing names via is_abbrev for partial match or exact match based on parameter. Account_id and player_id provide multi-character detection. When removing hatred via remHated, the function iterates clist to find the matching node, unlinks it, deletes the name array, deletes the node, and clears HATE_CHAR if clist becomes empty.

The hunting system initializes in setHunting by storing raw target pointer, setting persistence to mob level, calculating distance as fifty plus level doubled if hated, caching birth room, and setting ACT_HUNTING flag. The hunt function checks persistence first—if zero or negative, navigation toward oldRoom begins or hunting stops. If target is in room and visible via canSee with INFRA_YES, targetFound is called. Otherwise dirTrack returns next direction, portal index is calculated if code greater than nine, and goDirection executes movement.

Pathfinding in dirTrack validates vision requirements checking light, ROOM_ALWAYS_LIT, AFF_TRUE_SIGHT, AFF_CLARITY, or immortal status. It checks target concealment affect and generates random one to one fifty against modifier. Global pathfinding activates for level greater or equal MIN_GLOB_TRACK_LEV of thirty, SPELL_TRAIL_SEEK affect, or ACT_HUNTING flag. Portal handling encodes index as code minus nine for values greater than nine.

The aggression system calls factionAggroCheck first for Cult and Brotherhood territorial logic. It scans room characters checking canSee, polymorphed player exclusion, level difference guards, wandered mob low-level protection, and wimpy sleeping-only restriction. The karma check compares player karma scaled zero to one hundred against mob intelligence scaled zero to two hundred plus anger. Multiple valid targets use random selection.

Target scoring in senseWimps builds score from hit total, mana, armor contribution, and karma. It applies position modifiers, mounted penalty, NPC penalty, newbie protection, hatred bonus, and wounded finish bonus. The anti-tank detection checks AFF_AGGRESSOR flag, verifies fighting a non-PC, scans for PC using the target as tank, and switches with flavor message.

The mobileActivity dispatcher checks spell effects for plague locusts and sticks-to-snakes, decrements lag, handles position for combat readiness, calls charmeeStuff and protectionStuff for pets, fires spec procs with CMD_GENERIC_PULSE and CMD_MOB_ALIGN_PULSE, removes stuck items, calls senseWimps for intelligent switching, calls findABetterWeapon for equipment, processes combat with fear checks and combat spec procs, handles non-combat via notFightingMove, processes hate and fear via doHatefulStuff and fearCheck, and checks aggression via aggroCheck.

Cleanup in TMonster destructor iterates hates.clist caching next pointer, deleting each node, setting clist to null, then repeats for fears.clist. Global cleanup via DeleteHatreds and DeleteFears iterates character_list calling remHated and remFeared on each TMonster instance.

Response loading queries database with vnum, parses response string into resp structures, allocates command linked lists, and attaches to respList. Response checking in checkResponses matches cmd type and args pattern. Response execution in modifiedDoCommand interprets special command types for messages, spawning, and quest flags.

Cleric hunting magic activates for CLASS_CLERIC level thirty or higher with twenty percent random chance. If target is under level fifteen, magic is skipped. For same zone, SPELL_SUMMON is cast. For different zone, SPELL_ASTRAL_WALK is cast. Both return FALSE to skip walking.

Player tracking via doTrack applies class distance formula, race modifier, quest bit adjustment, and SPELL_TRAIL_SEEK bonus. It calls dirTrack with distance budget and stay-zone setting. With AUTO_HUNT autobit, the command is queued automatically for seamless pursuit.

Faction aggression checks isCult and isBrother status, validates territory via inLogrus and inBrightmoon, checks for opposing faction members, allows trade pass exception for item vnum eight eight seven nine, and initiates combat with faction-specific flavor text.

## Troubleshooting

### Mob Not Attacking Players

Verify aggro returns true by checking anger and malice values—four times anger plus five times malice must exceed four fifty without ACT_AGGRESSIVE flag. Check if mob is utility or guild type via UtilMobProc and GuildProcs. Verify mob is not pet, charm, or thrall. Check karma—high player karma may exceed mob intelligence plus anger threshold. Verify level difference allows attack—mobs avoid suicidal targeting. Check wandered status—wandered mobs reduce aggression against low-levels. For wimpy mobs, target must be sleeping.

### Hunting Target Lost

Check persistence—when zero or negative, hunting stops. Verify hunt_dist has not reached zero. Check if target entered peaceful room causing hunting cancelation. Verify vision—dirTrack requires sufficient light or magical sight. Check concealment—high SKILL_CONCEALMENT affect blocks tracking. Verify target is still in world—if deleted, hunting pointer is dangling. Check if addFeared was called clearing hunting state.

### Memory Leak in Opinion System

Verify TMonster destructor iterates both hates.clist and fears.clist with cached next pointer. Check that clist is set to null after deletion. Ensure charList name field is deleted with delete array. Verify DeleteHatreds and DeleteFears are called when characters are removed. Check for early returns in destructor preventing cleanup.

### Crash in mobileActivity

Check all movement function return values for DELETE_THIS—hunt, goDirection, assistFriend must propagate flags immediately. Verify specials.hunting pointer is validated before access—check non-null, isPc, and desc existence. Ensure takeFirstHit return value is checked for DELETE_THIS and DELETE_VICT. Verify iterator safety when processing room stuff—cache next before deletion. Check that hunting pointer is cleared before propagating DELETE_VICT.

### Mob Not Developing Hatred

Verify developHatred is called from damage handlers. Check patience formula—current HP percentage must be low enough. Check level difference—higher level gaps reduce difficulty threshold. Verify random variance allows threshold crossing. Check ACT_IMMORTAL flag blocking hatred. Ensure addHated return value is not ignored.

### Target Selection Ignoring Wounded

Verify mob is level fifteen or higher to activate senseWimps. Check that wounded threshold is under thirty percent HP. Ensure senseWimps is called during combat phase. Verify scoring modifiers are not overwhelming wounded bonus. Check that canSee succeeds for wounded target.

### Faction Aggression Not Triggering

Verify mob has isCult or isBrother status. Check territory—Cult mobs must be inLogrus, Brotherhood mobs must be inBrightmoon. Verify target has opposing faction status. For Snake faction in Logrus, check for trade pass item vnum eight eight seven nine. Ensure factionAggroCheck is called before normal aggroCheck.

### Response Not Executing

Verify loadResponses was called during mob creation. Check database mobresponses table for vnum entry. Verify cmd type matches triggering action. Check args pattern matching logic. Ensure checkResponses is called from triggerSpecial. Verify command list is properly parsed and allocated.

### Cleric Not Using Hunting Magic

Verify mob is CLASS_CLERIC level thirty or higher. Check random chance—only twenty percent probability. Verify target is level fifteen or higher. Check mana—spell casting requires sufficient mana. Ensure zone comparison logic for Summon versus Astral Walk.

### Mob Returning Home Instead of Hunting

Check persistence value—zero or negative triggers return. Verify hunt_dist is not exhausted. Check if oldRoom is valid and pathable. Ensure return navigation is not blocked by closed doors or terrain. Verify ACT_SENTINEL is not set preventing return movement.
