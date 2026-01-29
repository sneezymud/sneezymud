---
title: Object Manipulation System
description: Core object manipulation commands in SneezyMUD including sacrificing corpses for lifeforce, getting/dropping/putting items, giving items to other beings, and identifying objects through spells.
keywords: [doSacrifice, TASK_SACRIFICE, CORPSE_SACRIFICE, lifeforce, doGet, doDrop, doPut, doGive, canGet, canGetMe, putMeInto, checkForGetTrap, checkForInsideTrap, ITEM_NODROP, ITEM_NEWBIE, identify, divinationObj, statObjInfo, carryWeightLimit, carryVolumeLimit]
category: Important Systems

  - trap-mechanics.md
  - delete-flags.md
  - task-system.md
  - economy-system.md
  - material-system.md
last_updated: 2026-01-29
source_files:
  - code/code/disc/disc_shaman.cc
  - code/code/task/task_sacrifice.cc
  - code/code/cmd/cmd_get.cc
  - code/code/task/task_get.cc
  - code/code/misc/inventory.cc
  - code/code/misc/other.cc
  - code/code/misc/utility.cc
  - code/code/disc/disc_mage_alchemy.cc
related: [object-system.md]
---
# Object Manipulation System

This document covers the core object manipulation commands in SneezyMUD: sacrificing corpses for lifeforce, getting/dropping/putting items, giving items to other beings, and identifying objects through spells.

**Misusing these systems causes crashes.** Common errors: ignoring DELETE flags from trap triggers, not checking NODROP flags, allowing manipulation of objects mid-sacrifice/butcher, and failing to propagate return codes from nested functions.

## Overview

Object manipulation commands form the foundation of inventory management:

| System | Command(s) | Purpose |
|--------|-----------|---------|
| Sacrifice | `sacrifice` | Convert corpses to lifeforce (Shaman) |
| Get/Drop/Put | `get`, `drop`, `put` | Move items between room/inventory/containers |
| Give | `give` | Transfer items/money to other beings |
| Junk/Donate | `junk`, `donate` | Destroy or share items |
| Identify | `identify`, `divination` | Reveal item properties |

## Sacrifice System

The sacrifice skill allows Shamans to convert corpses into lifeforce, a resource used by Shaman spells.

### Requirements

**Source:** `code/code/disc/disc_shaman.cc:40-109`

To begin a sacrifice:
1. Must know `SKILL_SACRIFICE`
2. Must be standing or flying (`POSITION_STANDING` or `POSITION_FLYING`)
3. Must hold a totem (`TOOL_TOTEM`) or wear a Shaman's Totem Mask
4. Target must be a corpse (`TBaseCorpse`)
5. Corpse cannot be currently being butchered, skinned, or sacrificed

### Sacrifice Task Flow

**Source:** `code/code/task/task_sacrifice.cc:7-239`

The sacrifice uses the task system (`TASK_SACRIFICE`) with a 3-phase ritual:

| Phase | timeLeft | Description |
|-------|----------|-------------|
| 2 | Initial | Sing rada song to the loa |
| 1 | Mid | Totem/mask eyes glow blood red |
| 0 | Final | Corpse face glows pale green |
| -1 | Complete | Sacrifice finished, corpse deleted |

### Lifeforce Formula

**Source:** `code/code/task/task_sacrifice.cc:11-16`

```cpp
int learning = ch->getSkillValue(SKILL_SACRIFICE);
int clev = ch->GetMaxLevel();

// On successful skill check
int factor = ::number(clev, learning + clev + 25);  // 50-175 range
ch->addToLifeforce(factor);

// On failed skill check
int factor2 = ::number(5, (((clev + learning) + ::number(1, 100)) / 5));
ch->addToLifeforce(-factor2);  // Lose lifeforce!
```

**Lifeforce gain:** Random between `level` and `level + learning + 25`
**Lifeforce loss:** Random between 5 and `((level + learning + random(1,100)) / 5)`

### Sacrifice Interruption

The sacrifice fails if:
- Character moves to a different room
- Character position drops below resting
- Character goes linkdead
- Totem breaks (runs out of uses)
- Lifeforce drops to 0 (loa forces stop, -2 HP penalty)
- Police mob sees the ritual and objects
- Combat starts (`CMD_TASK_FIGHTING`)

### Corpse Protection

**Source:** `code/code/disc/disc_shaman.cc:84-94`, `code/code/disc/disc_basic_adventuring.cc:265-269`

Corpses being sacrificed have `CORPSE_SACRIFICE` flag set to prevent:
- Simultaneous butchering
- Simultaneous skinning
- Multiple sacrifice attempts

```cpp
// Check before butcher/sacrifice
if (corpse->isCorpseFlag(CORPSE_SACRIFICE)) {
  act("$p: That corpse is actively being sacrificed!", false, ch, corpse, nullptr, TO_CHAR);
  return;
}
```

## Get/Drop/Put System

### Get Command

**Source:** `code/code/cmd/cmd_get.cc:79-618`

The get command handles retrieving items from rooms, containers, tables, and corpses.

#### Get Syntax Variants

| Syntax | Type | Description |
|--------|------|-------------|
| `get` | GETNULL | Error: "Get what?" |
| `get all` | GETALL | Get all items from room (uses task) |
| `get sword` | GETOBJ | Get single item from room |
| `get all bag` | GETALLOBJ | Get all items from container |
| `get sword bag` | GETOBJOBJ | Get specific item from container |
| `get all all.corpse` | Special | Get all from all corpses |

#### canGet Validation

**Source:** `code/code/misc/utility.cc:1160-1252`

The `canGet()` function validates whether an item can be picked up:

```cpp
bool TObj::canGetMe(const TBeing* ch, silentTypeT silent) const {
  // Immortals can get anything
  if (ch->isImmortal() ||
      (canWear(ITEM_WEAR_TAKE) && !isObjStat(ITEM_PROTOTYPE))) {

    // Type-specific denial (canGetMeDeny virtual)
    if (canGetMeDeny(ch, silent))
      return FALSE;

    // Weight/volume check
    return ch->canCarry(this, silent);
  }

  // Can't get non-TAKE items
  if (!silent) {
    if (canWear(ITEM_WEAR_TAKE))
      ch->sendTo(format("%s : You can't take that.\n\r") % getName());
    // else no message for truly non-takeable items
  }
  return FALSE;
}
```

#### Weight and Volume Limits

**Source:** `code/code/misc/inventory.cc:1279-1309`

```cpp
// Weight limit based on STR
float TBeing::carryWeightLimit() const {
  auto num = plotStat(STAT_CURRENT, STAT_STR, 30.0, 1920.0, 495.0);
  if (isFourLegged())
    num *= 2.0;
  return num;
}

// Volume limit based on DEX and height
int TBeing::carryVolumeLimit() const {
  int vol = plotStat(STAT_CURRENT, STAT_DEX, 45000, 450000, 135000);
  vol *= getHeight();
  vol /= 70;  // Normalize to human height
  return min(max(vol, 5000), 1000000);
}
```

| Stat | Min Carry | Max Carry | Average |
|------|-----------|-----------|---------|
| STR (weight) | 30.0 lbs | 1920.0 lbs | 495.0 lbs |
| DEX (volume) | 45,000 cu.in. | 450,000 cu.in. | 135,000 cu.in. |

#### Trap Triggering During Get

**Source:** `code/code/cmd/cmd_get.cc:148-166`

Getting items from containers can trigger traps:

```cpp
// Check container trap (inside trap)
rc = ch->checkForInsideTrap(sub);
if (IS_SET_DELETE(rc, DELETE_ITEM | DELETE_THIS))
  return DELETE_VICT | DELETE_THIS;

// Check item trap (get trap on the item itself)
rc = ch->checkForGetTrap(ttt);
if (IS_SET_DELETE(rc, DELETE_ITEM | DELETE_THIS))
  return DELETE_ITEM | DELETE_THIS;
```

Two trap types apply:
1. **Inside trap** - Triggers when reaching into a trapped container
2. **Get trap** - Triggers when picking up a trapped item

### Drop Command

**Source:** `code/code/misc/inventory.cc:269-493`

The drop command moves items from inventory to the room.

#### NODROP Flag

Items with `ITEM_NODROP` cannot be dropped:

```cpp
if (tobj && tobj->isObjStat(ITEM_NODROP) && !isImmortal()) {
  act("You can't drop $p, it must be CURSED!", FALSE, this, tobj, 0, TO_CHAR);
  return FALSE;
}
```

#### NEWBIE Item Destruction

**Source:** `code/code/misc/inventory.cc:332-339`

Newbie items (non-rentable starter gear) are destroyed when dropped outside safe areas:

```cpp
if (tobj && tobj->isObjStat(ITEM_NEWBIE) && tobj->stuff.empty() &&
    (in_room > 80) && (in_room != Room::DONATION)) {
  sendrpf(roomp, "The %s explodes in a flash of white light!\n\r",
    fname(tobj->name).c_str());
  delete tobj;
}
```

#### Trap Drop Behavior

**Source:** `code/code/misc/inventory.cc:236-266`

Dropping a trap item arms it:

```cpp
void TTrap::dropMe(TBeing* ch, showMeT, showRoomT showroom) {
  if (!isname("grenade", name)) {
    ch->sendTo(COLOR_OBJECTS,
      format("You drop %s, concealing and arming it.\n\r") % getName());
    // Trap becomes hidden, armed when dropped
  } else {
    ch->sendTo(COLOR_OBJECTS,
      format("You drop %s, activating it.\n\r") % getName());
    armGrenade(ch);  // Grenades detonate on timer
  }
}
```

### Put Command

**Source:** `code/code/misc/inventory.cc:557-768`

The put command places items into containers.

#### Container Restrictions

Different container types accept different items:

| Container Type | Accepts |
|----------------|---------|
| `TSpellBag` | Spell components only |
| `TKeyring` | Keys only |
| `TQuiver` | Arrows only |
| `TMoneypouch` | Money only |
| `TSuitcase` | Clothing/armor only |
| `TToothNecklace` | Teeth only |
| `TCardDeck` | Playing cards (vnums 7748-7799) |
| `TOpenContainer` | Most items |

**Source:** `code/code/misc/inventory.cc:495-530`

```cpp
int TThing::putMeInto(TBeing* ch, TOpenContainer* sub) {
  if (dynamic_cast<TSpellBag*>(sub)) {
    act("Sorry, $p can only hold spell components.", FALSE, ch, sub, this, TO_CHAR);
    return TRUE;  // Block put
  }
  // ... similar checks for other specialized containers
  return FALSE;  // Allow put
}
```

#### Volume and Weight Checks

**Source:** `code/code/misc/inventory.cc:1617-1625`

```cpp
// Weight check
if (compareWeights(getWeight(),
      (cont->carryWeightLimit() - cont->getCarriedWeight())) == -1) {
  act("$p isn't strong enough to hold $N.", FALSE, ch, cont, this, TO_CHAR);
  return FALSE;
}

// Volume check (with material-based reduction)
if (getReducedVolume(cont) > (cont->carryVolumeLimit() - cont->getCarriedVolume())) {
  act("$p isn't big enough to hold $N.", FALSE, ch, cont, this, TO_CHAR);
  return FALSE;
}
```

## Give/Receive System

**Source:** `code/code/misc/inventory.cc:771-1249`

The give command transfers items or money between beings.

### Money Transfer

**Source:** `code/code/misc/inventory.cc:828-997`

Syntax: `give <amount> talens <recipient>`

```cpp
// Validation
if (amount <= 0) {
  sendTo("Sorry, you can't do that!\n\r");
  return FALSE;
}
if ((getMoney() < amount) && !hasWizPower(POWER_GOD)) {
  sendTo("You haven't got that many talens!\n\r");
  return FALSE;
}

// Transfer
giveMoney(vict, amount, GOLD_XFER);
```

Large transfers (>100k to someone with >500k) are logged.

### Item Transfer

**Source:** `code/code/misc/inventory.cc:998-1249`

#### Give Restrictions

| Restriction | Condition |
|-------------|-----------|
| NODROP | Item is cursed, cannot let go |
| PROTOTYPE | Requires both parties to be immortal |
| Solo Quest | Cannot give to `PLR_SOLOQUEST` players |
| Group Quest | Cannot give to `PLR_GRPQUEST` players (unless you're on it) |
| No Hands | Recipient has no hands |
| Weight/Volume | Recipient cannot carry item |

#### Give Flags

```cpp
enum giveTypeT {
  GIVE_FLAG_DEF,           // Normal give with visibility checks
  GIVE_FLAG_DROP_ON_FAIL,  // Drop item if recipient can't carry
  GIVE_FLAG_IGN_DEX_TEXT,  // Ignore DEX/weight, show text
  GIVE_FLAG_IGN_DEX_NOTEXT,// Ignore DEX/weight, silent
  GIVE_FLAG_SILENT_VICT    // Don't notify recipient
};
```

### Mob Response Triggers

**Source:** `code/code/misc/inventory.cc:920-977`

Giving items/money to NPCs triggers response scripts:

```cpp
rc = dynamic_cast<TMonster*>(vict)->checkResponses(this, obj, NULL, CMD_GIVE);
if (IS_SET_DELETE(rc, DELETE_THIS)) {
  delete vict;  // Response deleted the mob
}
if (IS_SET_DELETE(rc, DELETE_ITEM)) {
  delete obj;   // Response consumed the item
}
```

## Junk and Donate

### Junk Command

**Source:** `code/code/misc/other.cc:219-384`

The junk command destroys items, returning a small portion of their value.

```cpp
void TObj::junkMe(TBeing* ch) {
  // Get 0.1% of item cost back
  if (obj_flags.cost > 0 && !isObjStat(ITEM_NEWBIE) && !isname("[prop]", name))
    ch->addToMoney(max(1, obj_flags.cost / 1000), GOLD_INCOME);
}
```

#### Junk Restrictions

| Flag | Effect |
|------|--------|
| `ITEM_NOJUNK_PLAYER` | Player protected item, cannot junk |
| `ITEM_NODROP` | Cursed, cannot let go |
| Personalized | Monogrammed items cannot be junked |
| Non-empty | With `AUTO_POUCH`, won't junk containers with items |

#### Race-specific Junk Messages

**Source:** `code/code/misc/other.cc:116-162`

Different races junk corpses differently:

| Race | Behavior |
|------|----------|
| Ogre, Giant, Troll, Golem, Minotaur | Rip limb by limb |
| Dragon, Dinosaur, Lion, Bear, Tiger | Devour the corpse |
| Tytan | Crumple and throw |
| Default | Trash and disintegrate |

### Donate Command

**Source:** `code/code/misc/inventory.cc:1909-2007`

The donate command sends items to the donation room (`Room::DONATION`).

Restrictions:
- Cannot donate `ITEM_NODROP` items
- Cannot donate personalized items
- Cannot donate items containing cursed/personalized items
- Items with no value or `ITEM_NEWBIE`/`ITEM_NORENT` are junked instead

## Identify System

### Identify Spell

**Source:** `code/code/disc/disc_mage_alchemy.cc:36-127`

The basic identify spell reveals:
- Item type and material
- Approximate decay time
- Volume and weight (rounded)
- Estimated value (rounded)

```cpp
// Decay time descriptions
if (obj->obj_flags.decay_time == -1 || decay_time > 800)
  "well into the future"
else if (decay_time < 100)
  "a few days *tops*"
else if (decay_time < 200)
  "about a week"
else if (decay_time < 400)
  "only a couple of weeks"
else if (decay_time < 800)
  "around a month"
```

#### Identify Rounding

**Source:** `code/code/disc/disc_mage_alchemy.cc:28-34`

Values are rounded to prevent exact information:

```cpp
int round_off(int value) {
  if (value > 100)
    return (value / 100) * 100;  // Round to hundreds
  if (value > 10)
    return (value / 10) * 10;    // Round to tens
  return value;
}
```

### Divination Spell

**Source:** `code/code/disc/disc_mage_alchemy.cc:234-356`

The divination spell provides detailed item information:

```cpp
caster->sendTo(format("It is %s.\n\r") % ItemInfo[obj->itemType()]->common_name);
caster->sendTo(obj->wear_flags_to_sentence());
caster->sendTo(format("%s\n\r") % obj->statObjInfo());
obj->divinateMe(caster);  // Type-specific details

// Show affects
for (i = 0; i < MAX_OBJ_AFFECT; i++) {
  if (obj->affected[i].location != APPLY_NONE) {
    // Show modifier name and value
  }
}
```

#### statObjInfo Virtual Method

Each object type implements `statObjInfo()` to return type-specific information:

| Object Type | Information Revealed |
|-------------|---------------------|
| Weapon | Damage dice, damage type |
| Armor | AC value, coverage |
| Container | Capacity, lock status |
| Food | Nutrition value, poisoned |
| Drink | Liquid type, amount |
| Scroll/Wand/Staff | Charges, spell |

### Identify Being

**Source:** `code/code/disc/disc_mage_alchemy.cc:129-227`

Identifying beings reveals:
- Level description and race
- Age (for players)
- Height and weight
- Armor class description
- Current stats (with perception-based error)
- Active affects
- Immunities/susceptibilities

## Code References

| File | Lines | Contents |
|------|-------|----------|
| `code/code/disc/disc_shaman.cc` | 17-109 | `doSacrifice()`, totem/mask checks |
| `code/code/task/task_sacrifice.cc` | 7-239 | Sacrifice task, lifeforce formulas |
| `code/code/cmd/cmd_get.cc` | 79-618 | `doGet()`, `get()`, trap checks |
| `code/code/task/task_get.cc` | 75-526 | `task_get()`, bulk get operations |
| `code/code/misc/inventory.cc` | 269-493 | `doDrop()` implementation |
| `code/code/misc/inventory.cc` | 495-768 | `putMeInto()`, `doPut()` |
| `code/code/misc/inventory.cc` | 771-1249 | `doGive()`, money/item transfer |
| `code/code/misc/inventory.cc` | 1279-1362 | Weight/volume limits, `canCarryMe()` |
| `code/code/misc/other.cc` | 111-384 | `junkMe()`, `doJunk()` |
| `code/code/misc/other.cc` | 1909-2007 | `doDonate()` |
| `code/code/misc/utility.cc` | 1160-1252 | `canGet()`, `canGetMe()` |
| `code/code/disc/disc_mage_alchemy.cc` | 36-356 | `identify()`, `divinationObj()` |

## Common Gotchas

### 1. DELETE Flag Propagation

Always check and propagate DELETE flags from trap triggers:

```cpp
// WRONG: Ignoring return code
ch->checkForGetTrap(obj);
ch->sendTo("You got it!\n\r");  // May be dead!

// CORRECT: Check and propagate
rc = ch->checkForGetTrap(obj);
if (IS_SET_DELETE(rc, DELETE_THIS))
  return DELETE_THIS;
ch->sendTo("You got it!\n\r");
```

### 2. Concurrent Corpse Operations

Never allow butcher/skin/sacrifice to operate on the same corpse:

```cpp
if (corpse->isCorpseFlag(CORPSE_SACRIFICE | CORPSE_PC_BUTCHERING | CORPSE_PC_SKINNING)) {
  ch->sendTo("That corpse is being processed by someone else.\n\r");
  return;
}
```

### 3. NODROP Items in Containers

Check nested items for NODROP before junking/donating containers:

```cpp
for (TThing* t : container->stuff) {
  TObj* tobj = dynamic_cast<TObj*>(t);
  if (tobj && tobj->isObjStat(ITEM_NODROP)) {
    sendTo("Something inside is CURSED!\n\r");
    return FALSE;
  }
}
```

### 4. canGet vs canWear(ITEM_WEAR_TAKE)

`canWear(ITEM_WEAR_TAKE)` only checks the flag. `canGet()` performs full validation including weight, volume, and type-specific denials.

### 5. Sacrifice Totem Consumption

Totems lose uses during sacrifice; masks do not:

```cpp
if (!mask) {  // Using totem, not mask
  totem->addToToolUses(-1);
  if (totem->getToolUses() <= 0) {
    // Totem destroyed by loa
    delete totem;
  }
}
```

## Related Documentation

- [Container System](container-system.md) - Container types and nesting
- [Trap Mechanics](trap-mechanics.md) - Trap triggering details
- [DELETE Flags](delete-flags.md) - Memory management signaling
- [Task System](task-system.md) - Sacrifice and bulk-get tasks
- [Economy System](economy-system.md) - Money transfer mechanics
- [Material System](material-system.md) - Volume reduction by material
