---
title: Object Manipulation System
description: Core object manipulation commands including sacrifice, get/drop/put, give, junk/donate, and identify operations.
category: critical
keywords: [sacrifice, lifeforce, object manipulation, inventory management, specialized containers]
primary_symbols:
  functions: [doSacrifice, doGet, doDrop, doPut, doGive, doJunk, doDonate, canGet, canGetMe, canCarry, putMeInto, checkForGetTrap, checkForInsideTrap, identify, divinationObj, statObjInfo]
  classes: [TBaseCorpse, TOpenContainer, TSpellBag, TKeyring, TQuiver, TMoneypouch, TSuitcase, TToothNecklace, TCardDeck]
  enums: [ITEM_NODROP, ITEM_NEWBIE, ITEM_NOJUNK_PLAYER, ITEM_PROTOTYPE, ITEM_NORENT, ITEM_WEAR_TAKE, CORPSE_SACRIFICE, CORPSE_PC_BUTCHERING, CORPSE_PC_SKINNING, GIVE_FLAG_DEF, GIVE_FLAG_DROP_ON_FAIL, GIVE_FLAG_IGN_DEX_TEXT, GIVE_FLAG_IGN_DEX_NOTEXT, GIVE_FLAG_SILENT_VICT, GETNULL, GETALL, GETOBJ, GETALLOBJ, GETOBJOBJ, TASK_SACRIFICE, DELETE_THIS, DELETE_ITEM, SKILL_SACRIFICE, PLR_SOLOQUEST, PLR_GRPQUEST, GOLD_XFER]
---
# Object Manipulation System

## Overview

Object manipulation commands form the foundation of inventory management. The sacrifice system converts corpses to lifeforce for Shaman spells. Get/drop/put commands move items between rooms, inventory, and containers. Give transfers items or money between beings. Junk destroys items for minimal value; donate sends items to the donation room. Identify and divination reveal item properties with varying detail.

These operations interact with traps, capacity limits, and item flags. Misusing them causes crashes through ignored DELETE flags, concurrent corpse operations, or improper NODROP handling.

## Patterns

**DELETE Flag Handling**

- Always check trap trigger return values before continuing execution
- Propagate DELETE_THIS and DELETE_ITEM flags up the call stack
- Never access objects after functions that may have deleted them
- checkForGetTrap and checkForInsideTrap return DELETE_THIS (via DELETE_VICT) when the character dies; they never return DELETE_ITEM

**Corpse Concurrency**

- Always check corpse flags before starting sacrifice, butcher, or skin operations
- Set the appropriate flag when beginning a corpse operation
- Clear the flag on task completion or interruption

**Item Restrictions**

- Never allow drop, junk, or donate of ITEM_NODROP items
- Check nested items in containers for NODROP before container operations
- Destroy ITEM_NEWBIE items when dropped outside safe areas

**Container Validation**

- Verify specialized containers only accept appropriate item types
- Check both weight and volume limits before insertion
- Use the virtual putMeInto method for type-specific rejection

**Give Operations**

- Validate recipient can see, reach, and carry the item
- Block transfers to solo/group quest players unless on the same quest
- Check mob response triggers after giving to NPCs

## Reference

### Commands and Purposes

| Command | Purpose |
|---------|---------|
| sacrifice | Convert corpses to lifeforce (Shaman) |
| get | Retrieve items from room/containers |
| drop | Move items from inventory to room |
| put | Place items into containers |
| give | Transfer items/money to other beings |
| junk | Destroy items for 0.1% value |
| donate | Send items to donation room |
| identify | Reveal basic item properties |
| divination | Reveal detailed item properties |

### Get Syntax Variants

| Syntax | Type | Behavior |
|--------|------|----------|
| get | GETNULL | Error message |
| get all | GETALL | Get all room items (task) |
| get sword | GETOBJ | Get single room item |
| get all bag | GETALLOBJ | Get all container items |
| get sword bag | GETOBJOBJ | Get specific container item |
| get all all.corpse | Special | Get all from all corpses |

### Specialized Container Acceptance

| Container Type | Accepts Only |
|----------------|--------------|
| TSpellBag | Spell components |
| TKeyring | Keys |
| TQuiver | Arrows |
| TMoneypouch | TMoney objects |
| TSuitcase | Clothing/armor |
| TToothNecklace | Teeth |
| TCardDeck | Playing cards (vnums 7748-7799) |
| TOpenContainer | Most items |

### Carry Limits by Stat

| Stat | Minimum | Maximum | Average |
|------|---------|---------|---------|
| STR (weight) | 30 lbs | 1920 lbs | 495 lbs |
| DEX (volume) | 45,000 cu.in. | 450,000 cu.in. | 135,000 cu.in. |

Volume scales by character height divided by 70 (human baseline), clamped between 5,000 and 1,000,000. Four-legged beings have doubled weight capacity.

### Sacrifice Phases

| Phase (timeLeft) | Description |
|------------------|-------------|
| 2 | Sing rada song to the loa |
| 1 | Totem/mask eyes glow blood red |
| 0 | Corpse face glows pale green |
| -1 | Sacrifice complete, corpse deleted |

### Identify Decay Descriptions

| Decay Time | Description |
|------------|-------------|
| -1 or >800 | Well into the future |
| <100 | A few days tops |
| 100-199 | About a week |
| 200-399 | Only a couple of weeks |
| 400-799 | Around a month |

### Give Restrictions

| Restriction | Condition |
|-------------|-----------|
| NODROP | Cursed item, cannot release |
| PROTOTYPE | Both parties must be immortal |
| Solo Quest | Recipient has PLR_SOLOQUEST |
| Group Quest | Recipient has PLR_GRPQUEST (unless same quest) |
| No Hands | Recipient lacks hands |
| Capacity | Recipient cannot carry |

### Give Flag Effects

| Flag | Behavior |
|------|----------|
| GIVE_FLAG_DEF | Normal give with visibility check and standard messages |
| GIVE_FLAG_DROP_ON_FAIL | Drop item in room if recipient cannot carry |
| GIVE_FLAG_IGN_DEX_TEXT | Bypass capacity checks, show messages (quest rewards) |
| GIVE_FLAG_IGN_DEX_NOTEXT | Bypass capacity checks, silent (admin operations) |
| GIVE_FLAG_SILENT_VICT | Suppress message to recipient (surprise deliveries) |

### Junk Restrictions

| Flag/Condition | Effect |
|----------------|--------|
| ITEM_NOJUNK_PLAYER | Cannot junk |
| ITEM_NODROP | Cannot junk |
| Personalized | Cannot junk |
| Non-empty container | Won't junk with AUTO_POUCH |

### statObjInfo Return Values

| Object Type | Information Returned |
|-------------|---------------------|
| TWeapon | Damage dice (e.g., 3d6), damage type |
| TArmor | AC value, coverage percentage |
| TOpenContainer | Capacity, lock/trap/closed status |
| TFood | Nutrition (bites remaining), poisoned status |
| TDrinkCon | Liquid type, current/max amount, poisoned status |
| TScroll | Spell level, up to 3 spell names |
| TWand/TStaff | Charges remaining/max, spell stored |

## Implementation

### Sacrifice Mechanics

The sacrifice skill uses TASK_SACRIFICE with a multi-phase ritual. Requirements: SKILL_SACRIFICE known, standing or flying position, totem held or mask worn, target is TBaseCorpse, corpse has no active flags.

**Lifeforce Formula (success):** Random between level and (level + learning + 25), typically 50-175 range.

**Lifeforce Formula (failure):** Lose random between 5 and ((level + learning + random(1,100)) / 5).

Sacrifice interrupts on: room change, position drop, linkdead, totem break, zero lifeforce (causes -2 HP), police mob intervention, combat start.

Totems lose uses during sacrifice; masks do not. Zero uses destroys the totem.

### Get/Drop Mechanics

The canGetMe virtual method validates pickup: checks ITEM_WEAR_TAKE flag, ITEM_PROTOTYPE status, type-specific denial via canGetMeDeny, and weight/volume via canCarry.

Four-legged beings have doubled weight capacity. Volume scales with DEX and height, normalized to human height (70 units).

Two trap types trigger during get: inside traps (checkForInsideTrap on container) and get traps (checkForGetTrap on the item itself). Both return DELETE flags that must be checked.

ITEM_NEWBIE items explode when dropped outside rooms 0-80 and the donation room.

Dropping trap items arms them; grenades activate on a timer instead.

### Put Mechanics

Specialized containers reject inappropriate items via putMeInto virtual method. Volume reduction applies based on material properties when checking container capacity. Items made of the same material as the container get significant volume reduction through the material compression factor.

### Give/Receive Mechanics

Money transfers use giveMoney with GOLD_XFER type. Large transfers (>100k to someone with >500k) are logged.

Item transfers trigger checkResponses on NPC recipients, which may delete the mob (DELETE_THIS) or consume the item (DELETE_ITEM).

Give flags control behavior: GIVE_FLAG_DROP_ON_FAIL drops item if recipient cannot carry, GIVE_FLAG_IGN_DEX variants bypass capacity checks, GIVE_FLAG_SILENT_VICT suppresses recipient notification.

### Junk/Donate Mechanics

Junk returns 0.1% of item cost (minimum 1 talen). ITEM_NEWBIE and prop items yield nothing.

Race-specific junk messages: Ogre/Giant/Troll/Golem/Minotaur rip corpses limb by limb; Dragon/Dinosaur/Lion/Bear/Tiger devour; Tytan crumples and throws; others trash and disintegrate.

Donate sends items to Room::DONATION via `thing_to_room(t_o, Room::DONATION)`. Items with no value, ITEM_NEWBIE, or ITEM_NORENT are junked instead. Personalized items and items containing NODROP or personalized items (detected through recursive container scan) are blocked.

### Identify Mechanics

Basic identify reveals: item type, material, approximate decay time, rounded volume/weight, rounded value. Values over 100 round to hundreds; over 10 round to tens.

Divination adds: wear flags, statObjInfo output (type-specific details), divinateMe output, all object affects. Being identification reveals level, race, age, height, weight, AC description, current stats with perception-based error, active affects, and immunities/susceptibilities.

### Source Files

| File | Contents |
|------|----------|
| disc/disc_shaman.cc | doSacrifice, totem/mask validation |
| task/task_sacrifice.cc | Sacrifice task, lifeforce formulas |
| cmd/cmd_get.cc | doGet, trap checks |
| task/task_get.cc | Bulk get operations |
| misc/inventory.cc | doDrop, putMeInto, doPut, doGive, carry limits, doDonate |
| misc/other.cc | junkMe, doJunk |
| misc/utility.cc | canGet, canGetMe |
| disc/disc_mage_alchemy.cc | identify, divinationObj |

## Troubleshooting

| Symptom | Cause | Fix |
|---------|-------|-----|
| Crash after getting trapped item | DELETE flag from checkForGetTrap ignored | Check IS_SET_DELETE on return, propagate flags |
| Crash during concurrent corpse ops | No flag check before sacrifice/butcher/skin | Check CORPSE_SACRIFICE, CORPSE_PC_BUTCHERING, CORPSE_PC_SKINNING |
| Cursed item dropped/junked | NODROP check missing | Check isObjStat(ITEM_NODROP) before operation |
| Cursed item inside container junked | Nested NODROP check missing | Iterate container stuff checking each item |
| Item put in wrong container type | Missing putMeInto virtual call | Call putMeInto before actual insertion |
| Capacity exceeded | Using canWear instead of canGet | Use canGet for full validation including capacity |
| Totem vanishes unexpectedly | Uses not tracked | Totems consume uses; masks do not |
| Newbie item vanishes | Dropped outside safe area | ITEM_NEWBIE items destroy when dropped in rooms >80 |
| Mob deletes after give | checkResponses DELETE_THIS ignored | Check and handle DELETE_THIS from response |
| Item consumed by mob silently | checkResponses DELETE_ITEM ignored | Check and handle DELETE_ITEM from response |
| Give succeeds with overweight recipient | GIVE_FLAG_IGN_DEX used inappropriately | Use GIVE_FLAG_DEF for normal gives; reserve IGN_DEX for immortal/quest transfers |
| Sacrifice interrupts immediately | Totem has exactly 1 use remaining | Check getToolUses before starting; warn if low |
| Container volume appears wrong | Material compression not visible | getReducedVolume applies compression; identify shows base volume |
| Identify shows wrong decay time | Absolute vs relative decay_time | Some items store absolute pulse count, others relative ticks remaining |
