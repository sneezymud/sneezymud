---
title: Object Manipulation
category: important
keywords: [sacrifice, lifeforce, get, drop, put, give, junk, donate, identify, divination, trap, NODROP, NEWBIE, canGet, canCarry, weight, volume, container]
related: [object-system.md, trap-mechanics.md, task-system.md, economy-system.md, material-system.md]
primary_symbols:
  functions: [doSacrifice, doGet, doDrop, doPut, doGive, doJunk, doDonate, canGet, canGetMe, canCarry, putMeInto, checkForGetTrap, checkForInsideTrap, identify, divinationObj, statObjInfo]
  classes: [TBaseCorpse, TOpenContainer, TSpellBag, TKeyring, TQuiver, TMoneypouch, TSuitcase, TToothNecklace, TCardDeck]
  files: [code/code/disc/disc_shaman.cc, code/code/task/task_sacrifice.cc, code/code/cmd/cmd_get.cc, code/code/misc/inventory.cc, code/code/misc/other.cc, code/code/misc/utility.cc, code/code/disc/disc_mage_alchemy.cc]
---

# Object Manipulation

Object manipulation commands form the foundation of inventory management in SneezyMUD. Players interact with items through getting, dropping, putting items into containers, giving to others, sacrificing corpses for lifeforce, junking unwanted items, and identifying properties through divination spells.

Misusing these systems causes crashes. Common errors include ignoring DELETE flags from trap triggers, not checking NODROP flags before operations, allowing simultaneous corpse processing operations, and failing to propagate return codes from nested functions.

## Overview

### Core Commands

The manipulation system provides several command families with distinct purposes. The sacrifice command converts corpses into lifeforce for Shaman spells, requiring totem tools and performing ritual phases through the task system. The get/drop/put commands handle movement of items between rooms, inventory, and containers, with extensive validation for weight, volume, and item type restrictions. The give command transfers items or money between beings, triggering mob response scripts and checking quest restrictions. The junk command destroys items for partial value recovery, while donate sends items to the donation room. The identify and divination spells reveal item properties with varying detail levels.

### Validation Layers

Object manipulation applies multiple validation layers before allowing operations. The canGet check verifies ITEM_WEAR_TAKE flag, prototype status, and type-specific denial conditions through virtual methods. The canCarry check compares item weight and volume against character limits derived from STR and DEX stats. Container-specific checks enforce type restrictions like spell bags accepting only components or keyrings accepting only keys. Trap checks trigger on get and put operations, potentially deleting the character or item. NODROP and NEWBIE flags prevent certain operations or cause item destruction when dropped outside safe areas.

### Corpse Concurrency Protection

The corpse flag system prevents simultaneous operations that would corrupt state. CORPSE_SACRIFICE flag is set during Shaman sacrifice rituals, CORPSE_PC_BUTCHERING during butchering tasks, and CORPSE_PC_SKINNING during skinning operations. Any attempt to start a conflicting operation checks all three flags and blocks with an error message. The flags are cleared when tasks complete or are interrupted, allowing subsequent operations.

### DELETE Flag Propagation

Trap triggers and mob responses return DELETE flags that must be checked immediately and propagated up the call stack. checkForGetTrap and checkForInsideTrap return DELETE_THIS when character dies and DELETE_ITEM when item is destroyed. Mob checkResponses calls can return DELETE_THIS for mob deletion or DELETE_ITEM for item consumption. Continuing execution after DELETE flag detection causes use-after-free crashes. The correct pattern checks IS_SET_DELETE on return codes and immediately returns the flag to the caller.

## Patterns

### Sacrifice Workflow

Shaman sacrifice begins with doSacrifice validation checking skill knowledge, position requirements, totem or mask possession, corpse type, and absence of conflicting corpse flags. Upon validation success, CORPSE_SACRIFICE flag is set and TASK_SACRIFICE is started with timeLeft set to 2. Each task phase decrements timeLeft and displays ritual messages describing rada song, glowing eyes, and pale green face. On timeLeft reaching negative one, the task calculates lifeforce gain or loss based on skill check, adds the amount to character lifeforce, removes CORPSE_SACRIFICE flag, deletes the corpse, and returns true to signal task completion.

### Container Put Operation

The doPut command validates container type restrictions by checking dynamic types and calling putMeInto virtual method on the item being inserted. Specialized containers like TSpellBag, TKeyring, and TQuiver reject incompatible item types through putMeInto returning true to block the operation. Weight validation compares item weight against container carryWeightLimit minus current getCarriedWeight. Volume validation compares item getReducedVolume against container carryVolumeLimit minus current getCarriedVolume, where getReducedVolume applies material-based compression for items stored in containers made of the same material. Trap checks call checkForInsideTrap on the container before insertion, checking return code for DELETE flags before proceeding.

### Get with Trap Handling

The doGet command resolves target item and container references, then calls checkForInsideTrap if getting from container, checking return code and immediately propagating DELETE_THIS or DELETE_ITEM flags. Next it calls checkForGetTrap on the item itself, again checking and propagating DELETE flags. Only after both trap checks succeed does it call canGetMe on the item and canCarry on the character. Success removes item from current location using decrement operator, adds to character inventory using increment operator, and displays success messages. The critical invariant is that DELETE flag checks happen before any location pointer modifications.

### Give with Response Triggers

The doGive command validates recipient can receive the item through canCarryMe check, then removes item from giver inventory and adds to recipient inventory. If recipient is a TMonster, it calls checkResponses with CMD_GIVE, which executes response scripts that may modify item, consume it, or delete the mob. The return code check uses IS_SET_DELETE to detect DELETE_THIS indicating mob deletion or DELETE_ITEM indicating item consumption. If DELETE_ITEM is set, the item pointer is no longer valid and must not be referenced. If DELETE_THIS is set without DELETE_ITEM, the item must be handled by the caller since the mob who received it is now deleted.

### Weight and Volume Limits

Character carry capacity is calculated dynamically based on stats. carryWeightLimit uses plotStat to map STR from minimum 30 pounds at lowest STR to maximum 1920 pounds at highest STR with average 495 pounds, then doubles the result for four-legged beings. carryVolumeLimit uses plotStat to map DEX from minimum 45000 cubic inches to maximum 450000 cubic inches with average 135000 cubic inches, then scales by character getHeight divided by 70 to normalize to human height, clamping final result between 5000 and 1000000. The canCarry check sums current inventory and item weight, comparing against carryWeightLimit, and sums current inventory and item volume, comparing against carryVolumeLimit.

### Identify Information Levels

Basic identify spell rounds values using round_off function that divides by 100 and multiplies by 100 for values over 100, divides by 10 and multiplies by 10 for values over 10, and returns exact value for values 10 or below. This prevents exact value information while giving useful estimates. Divination spell provides precise information by calling statObjInfo virtual method on the object, which returns type-specific details like weapon damage dice, armor AC values, container capacity, food nutrition, drink liquid type, or scroll charges. It then iterates MAX_OBJ_AFFECT array displaying each affect with location and modifier value. Being identification additionally shows level, race, age, stats with perception-based error, active affects, and immunities.

## Reference

### Sacrifice Lifeforce Formula

On successful skill check, factor equals random number between character level and character level plus learning plus 25, with learning being getSkillValue for SKILL_SACRIFICE. This produces range from approximately 50 to 175 for high-level Shamans. The factor is added to character lifeforce using addToLifeforce. On failed skill check, factor2 equals random number between 5 and the quantity of level plus learning plus random 1 to 100, divided by 5. The negative of factor2 is added to lifeforce, causing lifeforce loss.

### Sacrifice Interruption Conditions

Task interrupts and clears CORPSE_SACRIFICE flag when character moves to different room checked by comparing in_room at task start versus current in_room. Interrupt occurs when character position drops below POSITION_RESTING checked at each task phase. Linkdead condition interrupts when character desc pointer is null. Totem breaking interrupts when totem getToolUses reaches zero, which happens because each task phase decrements tool uses by 1. Lifeforce reaching zero interrupts and applies 2 HP damage penalty with message about loa forces stopping. Police mob interrupt occurs when NPC with MOB_POLICEMAN flag observes ritual and calls stopFighting. Combat start interrupts through CMD_TASK_FIGHTING notification.

### Container Type Restrictions

TSpellBag accepts only items with type matching spell component types. TKeyring accepts only items with ITEM_KEY type. TQuiver accepts only items with ITEM_ARROW type. TMoneypouch accepts only TMoney objects. TSuitcase accepts only items wearable on body, head, neck, hands, feet, arms, legs, waist, or wrist positions. TToothNecklace accepts only items with vnum matching tooth vnum range. TCardDeck accepts only items with vnum between 7748 and 7799 representing playing cards. TOpenContainer base class accepts most item types unless specialized subclass overrides putMeInto.

### Give Restriction Conditions

ITEM_NODROP flag blocks give with cursed message because character cannot voluntarily release item. ITEM_PROTOTYPE flag blocks unless both giver and recipient have isImmortal true, preventing prototype items from entering player economy. PLR_SOLOQUEST flag on recipient blocks give from any non-immortal, preserving solo quest integrity. PLR_GRPQUEST flag blocks unless giver is on same group quest checked through descriptor quest tracking. Recipient lacking hands determined by hasHands returning false blocks give with message about no hands to hold item. Recipient weight or volume over limit determined by canCarryMe returning false blocks give, with behavior depending on give flags either dropping item or returning it to giver.

### Give Flag Effects

GIVE_FLAG_DEF performs normal give with canSee visibility check, showing standard messages to giver, recipient, and room. GIVE_FLAG_DROP_ON_FAIL drops item in room if recipient canCarryMe fails instead of returning to giver. GIVE_FLAG_IGN_DEX_TEXT bypasses weight and volume checks, showing normal messages, used for immortal gifts and quest rewards. GIVE_FLAG_IGN_DEX_NOTEXT bypasses weight and volume checks with no messages, used for silent admin operations. GIVE_FLAG_SILENT_VICT shows messages to giver and room but suppresses message to recipient, used for surprise deliveries.

### Junk Value Recovery

junkMe calculates refund as item obj_flags.cost divided by 1000, taking maximum of result and 1 to ensure at least 1 talen return. Items with ITEM_NEWBIE flag or names containing prop substring return no value. Items with cost of 0 or negative return no value. The refund uses GOLD_INCOME money source for economy tracking. Race-specific junk messages apply only to corpse items, with ogre/giant/troll/golem/minotaur showing limb ripping, dragon/dinosaur/lion/bear/tiger showing devouring, tytan showing crumpling, and all other races showing trash disintegration.

### Donate Item Handling

doDonate checks ITEM_NODROP flag and blocks with cursed message. Personalized items checked through isObjStat ITEM_PERSONALIZED flag are blocked. Items containing NODROP or personalized items detected through recursive container scan are blocked. Items with zero cost or ITEM_NEWBIE or ITEM_NORENT flags are junked instead of donated, with junking occurring through doJunk call. Valid items are moved to Room::DONATION using genericTeleport, with messages shown to room and donation room occupants.

### Identify Decay Time Ranges

Decay time of negative one or greater than 800 shows well into the future. Decay time less than 100 shows a few days at most. Decay time between 100 and 199 shows about a week. Decay time between 200 and 399 shows only a couple of weeks. Decay time between 400 and 799 shows around a month. These ranges provide useful temporal information without exact tick counts.

### statObjInfo Return Values

TWeapon returns damage dice in format like 3d6 plus damage type like slash or pierce. TArmor returns AC value and coverage percentage. TOpenContainer returns capacity in pounds and cubic inches, lock status, trapped status, closed status. TFood returns nutrition value as bites remaining, poisoned status. TDrinkCon returns liquid type, current amount, maximum capacity, poisoned status. TScroll returns spell level and spell names for up to 3 spells. TWand returns charges remaining, maximum charges, spell stored. TStaff returns similar charge and spell information.

## Implementation

### Sacrifice Task State Machine

TASK_SACRIFICE task stores corpse pointer in task obj field and tracks ritual phase through timeLeft counter. Each game pulse calls task_sacrifice function which switches on timeLeft value. timeLeft value 2 sends rada song message to room using sendrpf with corpse name. timeLeft value 1 checks whether character has totem or mask, sends appropriate glowing eyes message for tool type, decrements totem uses if applicable, and deletes totem if uses reach zero. timeLeft value 0 sends pale green glow message and performs skill check using bSuccess. Success path calculates factor, adds to lifeforce, advances learning, shows success message. Failure path calculates factor2, subtracts from lifeforce, shows failure message. Both paths then clear CORPSE_SACRIFICE flag, delete corpse, and return true. Interrupt handling checks conditions at each phase, clears CORPSE_SACRIFICE flag, removes task, shows interrupt message, and returns true.

### Get Command Syntax Parsing

doGet tokenizes argument string and checks first token. Empty string triggers GETNULL error. Token all with no second token triggers GETALL type, starting TASK_GET for bulk room get. Token all with second token triggers GETALLOBJ type for getting all from container. Single word token with no second token triggers GETOBJ type for getting specific item from room. Two word tokens trigger GETOBJOBJ type for getting specific item from container. Special case all all.corpse is detected through substring search and handled with iteration over room stuff finding all corpse items. Each syntax type resolves object pointers using get_obj_vis for GETOBJ, find_obj_in_list for container items, and real_roomp followed by stuff iteration for room items.

### Drop Command Location Handling

doDrop removes item from character inventory by decrementing reference, checks ITEM_NODROP flag and returns if set, checks ITEM_NEWBIE flag in combination with room number greater than 80 and not equal to Room::DONATION, destroying item if conditions met. For trap items detected through dynamic_cast to TTrap, calls dropMe which arms trap and sets concealed flag, or armGrenade for grenade type which starts detonation timer. For all other items, adds item to room using increment operator with roomp from character in_room, sends drop message to character and room using appropriate color codes.

### Put Command Type Validation

doPut calls putMeInto virtual method on item being inserted, passing character and container pointers. Base TThing implementation uses dynamic_cast to check container type. For TSpellBag, returns true unless item is component type. For TKeyring, returns true unless item is ITEM_KEY. For TQuiver, returns true unless item is ITEM_ARROW. For TMoneypouch, returns true unless item is TMoney. For TSuitcase, returns true unless item has wear flags for clothing slots. For TToothNecklace, returns true unless vnum matches tooth range. For TCardDeck, returns true unless vnum is between 7748 and 7799. Return value true blocks put operation, false allows it. After putMeInto check passes, weight comparison uses compareWeights on item getWeight against container carryWeightLimit minus getCarriedWeight. Volume comparison checks item getReducedVolume against container carryVolumeLimit minus getCarriedVolume.

### Give Money Transfer Flow

doGive money branch validates amount is positive, checks giver getMoney exceeds amount unless giver hasWizPower POWER_GOD, then calls giveMoney with recipient, amount, and GOLD_XFER source. giveMoney calls addToMoney on recipient with amount and source, then calls addToMoney on giver with negative amount and source. Both parties are marked with doQueueSave to ensure persistence. Large transfers exceeding 100000 talens to recipient with over 500000 existing money trigger immortal log entry with giver name, recipient name, amount, and timestamp.

### Give Item Mob Response Execution

After item transfer to mob recipient, checkResponses is called with giver character pointer, item pointer, null command target, and CMD_GIVE command type. Response system iterates response list on mob, matching trigger conditions, and executes response action script. Script commands can modify item properties, call delete on item setting DELETE_ITEM flag in return code, call mob functions that delete mob setting DELETE_THIS flag, or transfer item to other locations. Return code is checked with IS_SET_DELETE for DELETE_THIS indicating mob deletion, checked with IS_SET_DELETE for DELETE_ITEM indicating item consumption. If DELETE_THIS is set, mob pointer becomes invalid. If DELETE_ITEM is set, item pointer becomes invalid.

### Trap Check Return Code Handling

checkForInsideTrap is called with container pointer, executing trap trigger logic that may damage or kill character or destroy container. Return code may have DELETE_THIS flag if character died, DELETE_ITEM flag if container destroyed. Caller checks IS_SET_DELETE on return code for DELETE_THIS, immediately returning DELETE_VICT if vict parameter was passed or DELETE_THIS if character resolved locally. Similar check for DELETE_ITEM returns appropriate flag. checkForGetTrap follows identical pattern but operates on item being gotten rather than container. Critical requirement is checking return code before any subsequent item or character pointer dereference, as DELETE flags indicate pointer is no longer valid.

### Container Weight Calculation

getCarriedWeight recursively sums weight of all items in container stuff list. For each item in stuff, adds item getWeight which includes base obj_flags.weight plus any contained item weight. Items made of weightless materials may return zero from getWeight. Modification factors from affects may scale final weight. Maximum container weight determined by carryWeightLimit virtual method, which for most containers returns large fixed value but may be limited by material strength for cloth or paper containers.

### Container Volume Calculation

getCarriedVolume recursively sums volume of all items in container stuff list. For each item, calls getReducedVolume passing container pointer, which checks if item material matches container material and applies reduction factor if so, otherwise returns full getVolume. Base getVolume reads obj_flags.volume value. Material-based reduction uses material property compression_factor from material table, typically reducing volume by 10 to 50 percent for same-material items. carryVolumeLimit returns container internal capacity in cubic inches, derived from obj_flags.volume divided by material space efficiency factor.

### Identify Being Stat Display

divinationMe on TBeing sends level description using levelDesc function that maps level ranges to descriptive terms. Race is shown using race getName. Player age calculated from time_t birth date compared to current time. Height and weight shown from body statistics. Armor class converted to descriptive term using describeArmorClass. For each stat, getStats returns current value, getSkillValue SKILL_PERCEPTION on caster determines error range, random error added to stat before display. Active affects iterated through affected list, each showing affect type name and modifiers. Immunities shown by iterating immunity bitvector, susceptibilities shown by iterating susceptibility bitvector.

## Troubleshooting

### Character Dies After Getting Item

Symptom occurs when character executes get command and item transfer completes, then character becomes invalid leading to crash or corruption. Root cause is checkForGetTrap returning DELETE_THIS without propagation. Get command calls checkForGetTrap after item resolution, trap triggers deadly effect, return code has DELETE_THIS flag set, but calling code continues execution accessing character pointer. Fix requires adding IS_SET_DELETE check on return code from checkForGetTrap, immediately returning DELETE_THIS to caller when flag detected, preventing any subsequent character pointer access.

### Corpse Processed by Multiple Operations

Symptom shows butcher and sacrifice operating on same corpse simultaneously, leading to double-free when both operations try to delete corpse. Root cause is missing corpse flag checks before starting operations. Sacrifice sets CORPSE_SACRIFICE but butcher code fails to check flag before starting TASK_BUTCHER. Fix requires checking all three flags CORPSE_SACRIFICE, CORPSE_PC_BUTCHERING, CORPSE_PC_SKINNING using isCorpseFlag at start of each operation, showing error message and returning if any flag is set.

### NODROP Item Disappears from Container

Symptom occurs when player junks or donates container, NODROP item inside vanishes instead of blocking operation. Root cause is junk or donate checking only top-level item NODROP flag without recursing into container contents. Container passes NODROP check, delete is called on container, container destructor deletes all stuff contents including NODROP items. Fix requires recursive scan of container stuff before junk or donate, checking each item isObjStat ITEM_NODROP, blocking operation if any contained item has flag set.

### Give Succeeds with Overweight Recipient

Symptom shows item given to recipient despite recipient being over weight or volume limit, then recipient cannot perform any inventory operations. Root cause is give using GIVE_FLAG_IGN_DEX_TEXT flag inappropriately, bypassing canCarryMe check. Some code paths use ignore dex flags for quest rewards without checking if recipient can actually hold item. Fix requires using GIVE_FLAG_DEF for normal gives, reserving GIVE_FLAG_IGN_DEX flags for immortal commands and system-generated transfers where policy decision is made to allow overweight condition.

### Mob Response Deletes Item Still Referenced

Symptom manifests as crash after give command completes, with item pointer dereferenced after mob response execution. Root cause is mob checkResponses returning DELETE_ITEM flag but calling code continuing to reference item pointer. Response script contained item consumption logic that called delete on item and set DELETE_ITEM in return code. Calling code received return code but failed to check IS_SET_DELETE for DELETE_ITEM flag before calling subsequent methods on item pointer. Fix requires adding explicit check after checkResponses call, checking IS_SET_DELETE on return code for DELETE_ITEM, returning immediately when flag detected without any item pointer access.

### Sacrifice Fails with Totem Immediately Breaking

Symptom shows sacrifice starting successfully but interrupting after first phase with totem destroyed message. Root cause is totem having exactly 1 use remaining when sacrifice starts. Sacrifice task phase 1 decrements totem uses, use count reaches zero, totem is deleted, task checks totem pointer in next phase finding null, interrupt occurs. This is working as designed but may surprise players. Mitigation requires checking totem getToolUses at doSacrifice start, warning player if uses are low, or allowing sacrifice to continue without totem after task starts by caching totem requirement.

### Container Volume Appears Wrong

Symptom occurs when container shows much more space remaining than expected based on item volumes. Root cause is getReducedVolume applying material compression without player visibility into formula. Items made of same material as container get significant volume reduction through compression factor, but identify spell shows base volume not reduced volume. Player sees items totaling more volume than container capacity because reduction is not visible. This is working as designed. Documentation or identify spell enhancement could show reduced volume when item is inside specific container material.

### Identify Shows Wrong Decay Time

Symptom manifests when identify spell shows well into the future for item that decays within hours. Root cause is obj_flags.decay_time being current tick count rather than ticks remaining. Some item types store absolute decay time as game pulse count when item will decay, others store relative ticks remaining. identify spell assumes relative ticks remaining, showing incorrect description for absolute time items. Fix requires normalizing decay_time semantics across all item types, or identify spell checking whether value is absolute versus relative based on magnitude comparison against current pulse count.