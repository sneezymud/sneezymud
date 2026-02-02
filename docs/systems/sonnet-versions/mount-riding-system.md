---
title: Mount and Riding System
category: important
keywords: [mounting, riding, stability, Deikhan, combat bonuses, flying mounts, rider chain, DELETE_THIS, fallOffMount, rideCheck, horseMaster, POSITION_MOUNTED]
related: [combat-rounds.md, position-stance.md, memory-safety.md, movement-terrain-navigation.md, spell-skill-framework.md]
primary_symbols:
  functions: [doMount, dismount, fallOffMount, rideCheck, horseMaster, advancedRidingBonus, calmMount, mount, isRideable, canRide, lookForHorse, aiHorse]
  classes: [TBeing, TMonster, TThing]
  files: [code/code/misc/riding.cc, code/code/misc/combat.cc, code/code/disc/disc_deikhan_mounted.cc, code/code/misc/movement.cc, code/code/task/task_ride.cc]
---

## Overview

The mount and riding system allows characters to mount rideable creatures, control their movement, and engage in combat while mounted. The system provides skill-based stability checks requiring two consecutive failures to fall off, Deikhan class specialization with advanced mount skills, multi-rider support with primary rider control, and extensive DELETE flag safety requirements.

Mounts are determined by SPEC_HORSE spec proc or RIDABLE racial characteristic. Primary riders control movement direction while mounts automatically follow them. Maximum 4 rider slots exist per mount, with large riders consuming 2 slots. Height constraints require riders to be 60-250% of mount height. Riders lose one-third of attacks but gain significant accuracy and defense bonuses. Fall-off requires failing two consecutive rideCheck calls. No damage transfers between rider and mount as they maintain independent HP pools.

Rideability operates through two pathways: SPEC_HORSE special procedure flag assigned to specific mobs, or RIDABLE racial characteristic bit flag in race definition. Rider-mount compatibility checks height ratios where mounts must be at least 60% of rider height but less than 250% of rider height. Additional requirements include humanoid form, not already mounted, not berserking, no pack saddle, sufficient weight capacity, mount level constraints, and room capacity limits. Deikhan class bypasses mounting restrictions for fighting creatures.

The architecture uses a three-pointer chain structure with rider pointing to first rider in linked list chain, nextRider pointing to next rider in chain, and riding pointing to what the thing is mounted on. The last rider in the chain serves as primary rider who controls movement. Riders consume slots based on height relative to mount: small riders at or below 66% of mount height use 1 slot, large riders above 66% use 2 slots, enabling configurations like 4 small riders or 2 large riders per mount.

Combat modifications include position bonuses scaling with level as level divided by 4 plus 1 for both attack and defense, attack frequency penalty of 67% resulting in loss of one-third of attacks, special attack bonus of +2 for bash and trip abilities, and Deikhan Chivalry skill providing massive bonuses up to +74 attack and +159 defense at maximum learning. Flying mounts require Deikhan class with SKILL_RIDE_WINGED at 70 or higher, with special handling for dismounting in air triggering fall checks and crash landings.

Stability checks use two-check fall-off system where riders must fail two consecutive rideCheck calls to fall off. When mount takes damage, riders make checks with -5 modifier. When rider takes damage, checks use -10 modifier. Fall triggers include failed rideCheck during movement, mount or rider exhaustion at 0 movement points, drunkenness above 9 with 20% chance per move, mount weight collapse from excessive rider weight, and combat damage with two failed checks. The fallOffMount function handles fall damage and potential death with flying mount consequences triggering crashLanding or checkFalling based on sector type.

Common errors causing crashes include not checking DELETE_THIS returns from fallOffMount, continuing execution after rider dies from fall, modifying rider chain during iteration without caching nextRider, and failing to propagate DELETE flags from fall checks during combat.

## Patterns

Mount eligibility checking uses isRideable to test for SPEC_HORSE spec proc or RIDABLE racial characteristic, canRide to validate specific rider-mount pairing including height constraints and that rideable creatures cannot ride other rideables, and height ratio calculations where minimum mount height equals rider height times 60% and maximum equals rider height times 250% minus 1.

Mounting process flow validates target exists, checks mounting requirements including humanoid form and not already mounted, performs combat mounting check if in combat with skill penalties, executes mount level check potentially causing bucking if mount exceeds rider level, runs rideCheck for success, establishes mount pointers through mount function, and sets up follower relationship making mount follow primary rider.

Combat mounting penalties reduce effectiveness to 25% of skill when tanking for non-Deikhan or 33% for Deikhan, 50% when fighting but not tanking for both classes, and full skill when engaged only or not in combat. Mount level rejection occurs when horse level exceeds rider level causing random buck-off message, rider sitting position, one combat round wait, and mount becoming hostile via aiHorse.

Rider chain management requires caching next pointer before any modification operations as dismount modifies chain structure. Pattern uses for loop with t equals rider, t exists, t equals t2 where t2 equals t->nextRider cached before modification and TBeing pointer validated before operations. Mount death forcibly dismounts all riders without damage using cached nextRider iteration. Rider death while mounted causes corpse to fall off mount without affecting mount.

Primary rider determination finds last rider in chain by iterating through nextRider pointers until NULL. Primary rider receives saddle bonus of +8 to rideCheck, controls mount movement direction, has mount follow as master, and receives automatic saddle transfer if primary dismounts. Secondary riders have -5 penalty to rideCheck, cannot control movement direction, and receive same combat bonuses.

Movement control permits only primary rider horseMaster to control direction. Non-primary riders receive message "You are not the master of $N, and can't control where $E goes." Movement costs have mount paying full terrain cost while rider pays random 0 to cost divided by 3. Levitating mounts reduce cost to cost divided by 4. Flying mounts cost fixed 1 movement. Mount exhaustion stops movement when mount has less than need_movement available.

Movement validation checks include mount fighting blocking movement with "Your mount is fighting", mount not standing requiring "Your mount must be standing", door too low causing "Your mount refuses to go that way", fish mount in non-water refusing movement, death room refusal, underwater sector refusal when mount cannot swim, and climbing sector refusal.

DELETE flag safety requires checking IS_SET_DELETE on rc equals fallOffMount return value and immediately returning DELETE_THIS if set. Iterator safety demands caching nextRider before dismount operations. Null validation requires dynamic_cast to TBeing before accessing riding pointer methods. Position state consistency requires synchronizing POSITION_MOUNTED with riding pointer state.

Voluntary dismount restrictions prevent dismounting when mount is fighting unless Deikhan class, when berserking with "Your berserker rage prevents you from dismounting", when room at mob limit, or when on flying mount without ability to fly or coax to land. Flying mount handling dismounts to POSITION_FLYING in flying sector, triggers doFly when can fly in air or vertical sector, coaxes mount to land with SKILL_RIDE_WINGED check, or blocks with "Order your mount to land" message.

Dismount chain maintenance removes rider from chain updating rider pointer if first rider or finding previous and updating nextRider pointer, applies AFFECT_HORSEOWNED duration 1 mud hour when PC dismounts to prevent mob theft, handles master transfer to new horseMaster or stops follower, and clears riding and nextRider pointers setting position.

Continuous riding task starts with ride command followed by direction, continues in initial direction while path exists, follows non-backtrack exit at two-way intersections, stops at multi-way intersections or dead ends, and interrupts on combat or other commands.

Deikhan specialization uses advancedRidingBonus calculating average of SKILL_ADVANCED_RIDING and mount-type skill divided by 2 with range 0 to 100, adding random 0 to bonus divided by 15 to rideCheck for +0 to +6 modifier. Calm mount reduces anger malice and suspicion by random 0 to combined skill divided by 30 for 0 to 6 reduction when emotion exceeds default plus 20. Train mount retains mount following after dismount with skill divided by 2 success roll.

Flying mount mounting requires Deikhan class, SKILL_RIDE_WINGED at 70 or higher, passing random -10 to skill greater than 0 check, and successfully coaxing mount to land. Non-Deikhans cannot mount flying creatures receiving "You can't mount something that is flying" message.

Mount AI suppression blocks wandering and random movement, aggression checks including aggro and aggroCheck, scavenging, hunting, and fear reactions while rider pointer exists. Exceptions include spec procs still firing, spell effects continuing, and combat actions when attacked directly.

NPC mounting through lookForHorse requires horse without AFFECT_HORSEOWNED, horse level at most NPC level minus 4, horse not following anyone, horse at full health, and NPC not being utility mob, sentinel, shopkeeper, low health, already mounted, having rider, fighting, or rideable.

## Reference

rideCheck calculates effective skill as base SKILL_RIDE plus 3 times modifiers. Saddle bonus adds +8 for primary rider with saddled mount. Deikhan base bonus adds +5 for Deikhan class. Advanced riding bonus adds +0 to +6 from random 0 to advancedRidingBonus divided by 15. Secondary rider penalty subtracts -5 for non-primary rider. Mount damage modifier subtracts -5 when mount takes damage. Rider damage modifier subtracts -10 when rider takes damage.

Mount type classification maps races to skills where SKILL_RIDE_DOMESTIC includes horse bovine ox pig sheep baanta canine goat, SKILL_RIDE_NONDOMESTIC includes rhino tiger giraffe bear boar elephant deer, SKILL_RIDE_WINGED includes griffon hippogriff wyvern dragon dragonne lammasu shedu sphinx, and SKILL_RIDE_EXOTIC includes feline basilisk centaur chimera frog lamia manticore turtle lion leopard cougar wyvelin plus default.

Deikhan mount skills in DISC_MOUNTED discipline provide SKILL_CALM_MOUNT starting level 1 learn rate 2, SKILL_TRAIN_MOUNT starting level 26 learn rate 2, SKILL_ADVANCED_RIDING starting level 46 learn rate 2, SKILL_RIDE_DOMESTIC starting level 5 learn rate 2, SKILL_RIDE_NONDOMESTIC starting level 36 learn rate 2, SKILL_RIDE_WINGED starting level 66 learn rate 3, and SKILL_RIDE_EXOTIC starting level 85 learn rate 7.

Saddle types include riding saddle value 1 granting +8 rideCheck bonus to primary rider, and pack saddle value 2 preventing mounting with "You cannot ride $N when it is saddled with a pack" message. Saddle detection checks WEAR_BACK equipment slot for TBaseClothing or TBaseContainer with isSaddle flag.

Combat position bonuses for POSITION_MOUNTED provide attack bonus level divided by 4 plus 1 and defense bonus level divided by 4 plus 1. Special attack bonus adds +2 modifier to bash trip disarm attempts. Attack frequency penalty multiplies by 0.67 factor reducing to 67% frequency losing one-third of attacks.

Chivalry skill bonuses require SKILL_CHIVALRY and POSITION_MOUNTED providing attack bonus 74 times max of 10 or skill value divided by 100 ranging from +7 at 10 learning to +74 at 100 learning, and defense bonus 159 times max of 10 or skill value divided by 100 ranging from +16 at 10 learning to +159 at 100 learning.

Movement point costs distribute full terrain cost to mount and random 0 to cost divided by 3 to rider. Levitating mount pays cost divided by 4 with rider paying random 0 to cost divided by 4 then divided by 3. Flying mount pays 1 with rider paying random 0 to 1 divided by 3.

Slot calculation counts riders where height greater than mount height times 2 divided by 3 consumes 2 slots and height at or below threshold consumes 1 slot. Maximum riders fixed at 4 allowing 4 small riders, 2 large riders, or 2 small plus 1 large rider.

AFFECT_HORSEOWNED provides duration 1 mud hour applied when PC dismounts preventing mobs from mounting via lookForHorse affectedBySpell check.

aiHorse increases anger by 3, malice by 1, and suspicion by 4 via UA UM US calls plus aiTarget on failed rider. Called when mount attempt fails, rider falls off involuntarily, or mount bucked off by high-level mount.

Follower relationship makes mount become follower of primary rider appearing in group command, following during movement, having hate and fear lists cleared, and having hunting target cleared via setTarg NULL.

## Implementation

Mount function validates height check where rider height must not exceed mount height times 3 divided by 2 returning FALSE with "You are too big to ride that" message. Slot availability check calculates slots_needed as 2 if rider height greater than mount height times 2 divided by 3 else 1, counts slots_used from existing riders, and rejects if sum exceeds MAX_RIDERS with "There is no room for you" message.

Adding to rider chain sets thing->rider equals this if no existing rider pointer, else finds end of chain iterating through nextRider pointers and appends t->nextRider equals this. Sets riding equals thing and nextRider equals NULL. Changes position to POSITION_MOUNTED if current position greater than POSITION_SITTING. Mount becomes follower of primary rider if this equals thing->horseMaster via addFollower call when mount has no master.

Dismount function removes from chain by setting riding->rider equals nextRider if this is first rider, else iterating to find previous and setting t->nextRider equals nextRider. Applies AFFECT_HORSEOWNED when PC dismounts mob creating affectedData with type AFFECT_HORSEOWNED and duration 1 times Pulse::UPDATES_PER_MUDHOUR via affectTo. Handles master transfer checking if mount->master equals this and either stops follower or transfers to new horseMaster. Clears nextRider and riding pointers and sets position.

fallOffMount announces "loses control and falls off" to room, calls aiHorse on non-PC mounts making them hostile, executes dismount to specified position, and handles flying consequences. Flying mount in flying sector calls crashLanding with POSITION_FLYING. Flying mount in fall sector calls checkFalling for full damage. Flying mount otherwise calls crashLanding with POSITION_FLYING if can fly else POSITION_RESTING for land damage. Non-flying mount in fall sector calls checkFalling. Returns DELETE_THIS if IS_SET_DELETE on any consequence rc.

Two-check fall system executes first rideCheck with modifier, returns TRUE if passes, executes second rideCheck with same modifier if first fails, returns TRUE if passes, calls fallOffMount if both fail returning result. Mount damage triggers check with -5 modifier for rider on mount via if riding and dynamic_cast to TBeing. Rider damage triggers check with -10 modifier for all riders via for loop iterating rider chain.

Combat damage handling iterates mount riders when mount damaged checking if not tb->rideCheck(-10) then if not tb->rideCheck(-10) then calling rc equals tb->fallOffMount(this, POSITION_SITTING) and if IS_SET_DELETE(rc, DELETE_THIS) then delete tb setting NULL.

Mount death dismount loop uses for t equals rider with t2 equals t->nextRider cached before operations, validates TBeing pointer, calls dismount with POSITION_SITTING, and sends "Your mount collapses beneath you" message without damage.

Rider death dismount checks if riding pointer exists and calls dismount with POSITION_DEAD making corpse fall off mount leaving mount unaffected.

doMount validates target exists via get_char_room_vis, checks mounting requirements via canRide returning FALSE if fails, checks humanoid form via isHumanoid, checks not already mounted via riding pointer, checks not berserking via isCombatMode ATTACK_BERSERK, checks no pack saddle via hasSaddle not equals 2, checks weight capacity via getTotalWeight and getWeight comparison.

Combat mounting check skips for immortals, calculates learn as getSkillValue SKILL_RIDE plus advancedRidingBonus, applies penalties dividing learn by 4 if non-Deikhan tanking, by 3 if Deikhan tanking, by 2 if fighting not tanking, and runs bSuccess with modified learn returning FALSE on failure setting POSITION_SITTING and adding 2 round wait.

Mount level rejection executes when horse->GetMaxLevel exceeds GetMaxLevel using number 0 to 3 selecting random buck message, setting POSITION_SITTING, adding 1 combat round wait via addToWait(combatRound(1)), and triggering aiHorse hostile behavior.

Final mount attempt runs rideCheck with 0 modifier returning FALSE on failure with "You slip and miss" message, calls mount(horse) function establishing pointers, sets up follower relationship if not already master, clears hunting and targ, nulls hates and fears charlists.

Movement control in doMove checks if riding and this not equals riding->horseMaster sending "You are not the master of $N" message and returning FALSE. If this is mount with riders, gets tb from rider pointer, runs tb->rideCheck(0), calls rc equals tb->doMove(cmd) if passes, calls rc equals tb->fallOffMount(this, POSITION_SITTING) if fails, and returns rc.

Movement cost calculation checks if riding then if riding->isLevitating divides need_movement by 4, if riding->isFlying sets need_movement to 1, gets tbr from dynamic_cast riding to TBeing, checks if tbr->getMove less than need_movement returning FALSE with "Your mount is too exhausted" message. Movement deduction checks if riding then gets tbr and calls tbr->addToMove(-need_movement) and calls addToMove(-number(0, need_movement) / 3).

Movement failure on weight collapse compares compareWeights(riding->getWeight(), getTotalWeight(TRUE)) equals 1 causing act "$N collapses beneath your weight", sets tbr->setMove(0), and calls rc equals fallOffMount. Drunkenness check compares getCond(DRUNK) greater than 9 sending wobble message and 20% chance via number 0 to 4 equals 0 sending "purple elephants pushed you off" and calling fallOffMount. Rider exhaustion check compares getMove less than 1 sending "too tired to stay on your $o" and calling fallOffMount.

validMove mount checks verify if riding with tbt equals dynamic_cast riding to TBeing, if tbt and tbt->fight returning FALSE with "Your mount is fighting", if tbt and tbt->getPosition less than POSITION_FIGHTING returning FALSE with "Your mount must be standing", if tbt and tbt->willBumpHeadDoor returning FALSE with "Your mount refuses to go that way" for low doors, if tbt and getRace equals RACE_FISH and not isUnderwaterSector refusing fish out of water, if isRoomFlag ROOM_DEATH refusing death room.

Continuous riding task starts via start_task(this, NULL, NULL, TASK_RIDE, arg, 2, inRoom(), 0, 0, 5) when not task and riding and getDirFromChar(arg) not equals DIR_NONE. Task implementation in task_ride.cc continues movement checking for path existence, intersection handling, dead end detection, and combat interruption.

advancedRidingBonus returns 0 if not mount pointer, initializes skillTotal to 0, checks doesKnowSkill SKILL_ADVANCED_RIDING and if bSuccess adds getSkillValue SKILL_ADVANCED_RIDING to skillTotal, checks doesKnowSkill mount->mountSkillType and if bSuccess adds getSkillValue mount->mountSkillType to skillTotal, returns skillTotal divided by 2 averaging both skills.

calmMount validates mount and doesKnowSkill SKILL_CALM_MOUNT and bSuccess SKILL_CALM_MOUNT returning early if fails, calculates skillTotal as getSkillValue SKILL_CALM_MOUNT plus advancedRidingBonus, sets amt as number 0 to skillTotal divided by 30, reduces anger via mount->DA(amt) if mount->anger() plus 20 greater than mount->defanger(), reduces malice via mount->DMal(amt) if condition, reduces suspicion via mount->DS(amt) if condition.

trainMount check during dismount validates ch->doesKnowSkill SKILL_TRAIN_MOUNT and ch->bSuccess(ch->getSkillValue(SKILL_TRAIN_MOUNT) / 2, SKILL_TRAIN_MOUNT), calls tbt->stopFollower(TRUE) and transfers to new horseMaster if fails, continues following if succeeds.

horseMaster function iterates for t equals rider with t and t->nextRider with t equals t->nextRider returning last t pointer. Returns NULL if no riders.

getNumRiders counts slots initializing num to 0, iterating for t equals rider with t incrementing t equals t->nextRider, skipping if t equals ch, adding 2 to num if t->getHeight() greater than getHeight() times 2 divided by 3, adding 1 otherwise, returning num total.

getMaxRiders returns fixed 4 constant.

mountSkillType switch on getRace returning SKILL_RIDE_DOMESTIC for domestic races, SKILL_RIDE_NONDOMESTIC for wild races, SKILL_RIDE_WINGED for flying races, SKILL_RIDE_EXOTIC for unusual races and default.

hasSaddle checks isRideable returning FALSE if not rideable, gets obj from equipment[WEAR_BACK] returning FALSE if null, dynamic_casts to TBaseClothing and TBaseContainer, returns 1 if tbc and tbc->isSaddle, returns tbc2->isSaddle() result potentially 2 if pack saddle, returns FALSE otherwise.

lookForHorse validates not utility mob sentinel shopkeeper low health already mounted having rider fighting or rideable, iterates room stuff looking for horse, checks canRide and horse->master equals NULL and not affectedBySpell AFFECT_HORSEOWNED and horse->GetMaxLevel() plus 4 less than or equal GetMaxLevel, calls rc equals doMount(NULL, CMD_MOUNT, horse) returning TRUE if found.

Flying mount mounting checks if horse->isFlying and not isFlying, requires hasClass CLASS_DEIKHAN else "You can't mount something that is flying" returning FALSE, requires getSkillValue SKILL_RIDE_WINGED greater than or equal 70 else "You don't know enough about winged creatures" returning FALSE, checks number(-10, getSkillValue(SKILL_RIDE_WINGED)) greater than 0 calling act "You coax $N to land" and horse->doLand else act "$N ignores you" returning FALSE.

Flying mount dismount checks if roomp->isFlyingSector calling dismount POSITION_FLYING, else if horse->isFlying checks if canFly calling dismount POSITION_STANDING then doFly, else if SKILL_RIDE_WINGED check passes coaxing horse->doLand then dismount POSITION_STANDING, else returning FALSE with "Order your mount to land" message.

## Troubleshooting

Crash after fallOffMount indicates missing DELETE_THIS check. Verify rc equals fallOffMount call followed by if IS_SET_DELETE(rc, DELETE_THIS) return DELETE_THIS before any subsequent operations. Add immediate return preventing use-after-free when rider dies from fall damage or crash landing.

Crash during rider iteration indicates missing nextRider cache. Convert for t equals rider with t with t equals t->nextRider pattern to for t equals rider with t with t equals t2 where t2 equals t->nextRider before loop body. Cache protects against dismount or delete modifying chain during iteration.

Rider stuck on dead mount indicates missing dismount in mount death handler. Verify die function contains for loop iterating riders with t2 cache calling tb->dismount(POSITION_SITTING) before corpse creation. Mount death must force dismount all riders without damage.

Mount refuses movement indicates failed validation check. Verify mount not fighting via tbt->fight returning FALSE, mount standing via tbt->getPosition greater than or equal POSITION_FIGHTING, door height clearance via not willBumpHeadDoor, fish mount in water via getRace not RACE_FISH or isUnderwaterSector, not death room via not isRoomFlag ROOM_DEATH.

Fall-off too easy indicates single rideCheck instead of two-check system. Replace if not rideCheck(mod) calling fallOffMount with if not rideCheck(mod) and not rideCheck(mod) calling fallOffMount. Both checks must fail to fall requiring consecutive failures.

Fall-off never happens indicates missing combat damage checks. Verify mount damage handler calls if not rideCheck(-5) and not rideCheck(-5) for rider when mount damaged. Verify rider damage handler iterates all mount riders calling if not tb->rideCheck(-10) and not tb->rideCheck(-10) for each.

Secondary rider controls mount indicates missing horseMaster check. Verify doMove checks if riding and this not equals riding->horseMaster before allowing movement command. Only primary rider last in chain can control direction.

Deikhan cannot mount fighting creature indicates missing class bypass. Add hasClass CLASS_DEIKHAN check to skip "You do not have the skill to mount something that is fighting" restriction. Deikhan bypasses combat mounting restriction.

Cannot dismount flying mount indicates missing SKILL_RIDE_WINGED handling. Verify dismount checks if horse->isFlying and getSkillValue SKILL_RIDE_WINGED greater than or equal 0 allowing coax to land attempt. Non-Deikhan or low skill requires "Order your mount to land" message.

NPC steals player mount indicates missing AFFECT_HORSEOWNED. Verify dismount applies affectedData type AFFECT_HORSEOWNED duration 1 times Pulse::UPDATES_PER_MUDHOUR via affectTo when isPc dismounts TMonster. Verify lookForHorse checks not affectedBySpell AFFECT_HORSEOWNED before mounting.

Mount breaks following after dismount indicates missing SKILL_TRAIN_MOUNT. Verify dismount checks if tbt->master equals this and ch->doesKnowSkill SKILL_TRAIN_MOUNT and bSuccess with skill divided by 2. Failure calls stopFollower and transfers master. Success preserves following.

Position mismatch indicates missing position sync. Verify mount sets position to POSITION_MOUNTED if getPosition greater than POSITION_SITTING. Verify dismount sets position to specified pos parameter. Verify riding pointer and position state remain consistent.

Weight collapse loop indicates missing movement deduction. Verify mount pays full cost via tbr->addToMove(-need_movement) and rider pays partial via addToMove(-number(0, need_movement) / 3). Exhaustion stops movement before collapse.

Rider takes mount damage indicates incorrect damage implementation. Mount and rider maintain independent HP pools with no automatic sharing. Mount damage only affects mount HP triggering rideCheck for riders. Rider damage only affects rider HP. No transfer exists between pools.

Saddle bonus not applying indicates missing horseMaster check. Verify rideCheck adds 8 to mod only if tbt and tbt->hasSaddle and tbt->horseMaster() equals this. Only primary rider with saddled mount receives bonus.

Chivalry bonus missing indicates missing position check. Verify combat bonus checks doesKnowSkill SKILL_CHIVALRY and getPosition equals POSITION_MOUNTED before calculating and adding amt. Requires both skill and mounted position.

Advanced riding bonus wrong indicates skill calculation error. Verify advancedRidingBonus returns skillTotal divided by 2 averaging SKILL_ADVANCED_RIDING and mount->mountSkillType. Both must succeed bSuccess check to contribute. Used via random 0 to bonus divided by 15 in rideCheck.

Mount AI still active indicates missing rider check. Verify mobileActivity returns FALSE when rider pointer exists preventing autonomous behavior. Spec procs fire but wandering aggression scavenging hunting fear reactions all suppressed while riders present.

Height check fails incorrectly indicates wrong comparison. Verify mount function checks getHeight() greater than thing->getHeight() times 3 divided by 2 for too big message. Verify canRide checks horse->getHeight() less than or equal 6 times getHeight() divided by 10 for too small and horse->getHeight() greater than or equal 5 times getHeight() divided by 2 for too large.

Slot calculation wrong indicates threshold error. Verify getNumRiders adds 2 if t->getHeight() greater than getHeight() times 2 divided by 3 else adds 1. Large rider threshold 66% of mount height consumes double slots.

Flying mount mounting blocked indicates missing Deikhan or skill. Verify hasClass CLASS_DEIKHAN bypasses non-Deikhan restriction. Verify getSkillValue SKILL_RIDE_WINGED greater than or equal 70 allows coax attempt. Verify number(-10, skill) greater than 0 succeeds calling horse->doLand.
