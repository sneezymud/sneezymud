---
title: Mount and Riding System
description: Complete mount/riding system including mounting mechanics, movement control, combat modifications, stability checks, Deikhan skills, and DELETE flag safety patterns
keywords: [doMount, dismount, riding, horseMaster, rideCheck, fallOffMount, SKILL_RIDE, POSITION_MOUNTED, advancedRidingBonus, rider chain, DELETE_THIS, combat bonuses, Chivalry, nextRider, isRideable, canRide, AFFECT_HORSEOWNED, bucking, flying mounts]
category: Important Systems

last_updated: 2026-01-29
source_files: [code/code/misc/riding.cc, code/code/misc/combat.cc, code/code/disc/disc_deikhan_mounted.cc]
related:
  - combat-rounds.md
  - position-stance.md
  - memory-safety.md
  - spell-skill-framework.md
  - movement-terrain-navigation.md
---

# Mount and Riding System

The mount and riding system allows characters to mount rideable creatures, control their movement, and engage in combat while mounted. The system provides skill-based stability checks, Deikhan class specialization, multi-rider support, and extensive DELETE flag safety requirements.

**Misusing this system causes crashes.** Common errors: not checking DELETE_THIS returns from fallOffMount(), continuing execution after rider dies from fall, modifying rider chain during iteration without caching nextRider, failing to propagate DELETE flags from fall checks during combat.

## Overview and Requirements

SneezyMUD's mounting system provides:
- Mounting and dismounting creatures and objects (beds, chairs)
- Multi-rider support with primary/secondary rider mechanics
- Skill-based ride checks for stability
- Automatic movement following for mounts
- Flying mount handling with special dismount requirements
- Mount type categorization with Deikhan specialization skills
- Combat bonuses and modifications when mounted
- Two-check fall-off system for stability
- Independent HP tracking between rider and mount

**Key characteristics:**
- Mounts are determined by SPEC_HORSE spec proc or RIDABLE racial characteristic
- Primary rider (horseMaster) controls movement direction
- Maximum 4 rider slots per mount, with large riders consuming 2 slots
- Height constraints: rider must be 60-250% of mount height
- Mounts automatically follow their primary rider
- Riders lose 1/3 of attacks but gain significant accuracy/defense
- Fall-off requires failing TWO consecutive rideCheck() calls
- No damage transfer between rider and mount (independent HP pools)

### What Can Be Mounted

A creature is rideable if:

```cpp
// code/code/misc/riding.cc:80-87
bool TMonster::isRideable() const {
  if (spec == SPEC_HORSE)
    return TRUE;
  if (race->isRidable())  // RIDABLE racial characteristic
    return TRUE;
  return FALSE;
}
```

**Two pathways to rideability:**
1. **SPEC_HORSE** - Special procedure flag assigned to specific mobs
2. **RIDABLE racial characteristic** - Bit flag in race definition

**Source:** `code/code/misc/riding.cc:80-87`

### Rider-Mount Compatibility

The `canRide()` function validates whether a specific character can ride a specific mount:

```cpp
// code/code/misc/riding.cc:89-106
bool TBeing::canRide(const TBeing* horse) const {
  if (!horse->isRideable())
    return FALSE;

  // Rideable creatures cannot ride other rideables
  if (isRideable())
    return FALSE;

  // Height constraints
  if (horse->getHeight() <= (6 * getHeight() / 10))  // Too small
    return FALSE;
  if (horse->getHeight() >= (5 * getHeight() / 2))   // Too large
    return FALSE;

  return TRUE;
}
```

**Height ratio constraints:**

| Constraint | Formula | Meaning |
|------------|---------|---------|
| Minimum mount height | mount >= rider * 60% | Mount must be at least 60% of rider height |
| Maximum mount height | mount < rider * 250% | Mount must be less than 250% of rider height |

**Example for a 72-inch rider:**
- Minimum mount height: 43 inches (72 * 0.6)
- Maximum mount height: 179 inches (72 * 2.5 - 1)

**Source:** `code/code/misc/riding.cc:89-106`

### Additional Mount Requirements

The `doMount()` function enforces additional requirements:

| Requirement | Error Message | Bypass |
|-------------|---------------|--------|
| Humanoid form | "You can't ride things!" | None |
| Not already mounted | "You are already riding." | None |
| Not berserking | "Your berserker rage scares the mount." | None |
| No pack saddle | "You cannot ride $N when it is saddled with a pack." | None |
| Weight capacity | "$N can't carry all your weight." | None |
| Mount not fighting (non-Deikhan) | "You do not have the skill to mount something that is fighting!" | Deikhan class |
| Mount level <= rider level | Various buck-off messages | Quest exception |
| Not someone else's pet | "You can't ride someone else's pet." | None |
| Room not at mob limit | "There isn't enough room in here..." | Immortal |

**Source:** `code/code/misc/riding.cc:305-642`

## Mounting Architecture

### Rider-Mount Pointer Structure

The mounting system uses a three-pointer chain structure defined in `TThing`:

```cpp
class TThing {
    TThing* rider;       // First rider in linked list chain
    TThing* nextRider;   // Next rider in chain (NULL if last)
    TThing* riding;      // What this thing is mounted on
};
```

**Chain structure:**
```
Mount                     Rider Chain
+--------+               +---------+    +---------+    +---------+
| rider  |-------------->| riding  |    | riding  |    | riding  |
+--------+               |nextRider|--->|nextRider|--->|nextRider|-->NULL
                         +---------+    +---------+    +---------+
                         (1st rider)   (2nd rider)   (primary/last)
```

**Source:** `code/code/misc/being.h`

### Primary Rider (horseMaster)

The **last rider in the chain** is the primary rider who controls movement:

```cpp
// code/code/misc/riding.cc:928-936
TThing* TThing::horseMaster(void) const {
  TThing* t;
  for (t = rider; t && t->nextRider; t = t->nextRider);
  return t;  // Returns last rider
}
```

**Primary rider privileges:**
- Controls mount movement direction
- Receives saddle bonus (+8 to rideCheck)
- Mount follows this rider as master
- Automatic saddle transfer if primary dismounts

**Source:** `code/code/misc/riding.cc:928-936`

### Rider Slot Calculation

Riders consume slots based on their height relative to the mount:

```cpp
// code/code/misc/riding.cc:938-951
int TBeing::getNumRiders(TThing* ch) const {
  int num = 0;
  for (t = rider; t; t = t->nextRider) {
    if (t == ch) continue;
    if (t->getHeight() > getHeight() * 2 / 3)
      num += 2;  // Large rider
    else
      num++;     // Small rider
  }
  return num;
}

int TBeing::getMaxRiders() const {
  return 4;  // Fixed maximum
}
```

**Slot consumption:**

| Rider Size | Height Threshold | Slots Used |
|------------|-----------------|------------|
| Small | rider <= mount * 66% | 1 slot |
| Large | rider > mount * 66% | 2 slots |

**Examples for a mount with 4 slots:**
- 4 small riders (halflings on a horse)
- 2 large riders (humans on a horse)
- 2 small + 1 large rider

**Source:** `code/code/misc/riding.cc:938-958`

## Mounting and Dismounting

### The Mount Command

**Command Flow:**

```
mount <target>
  |
  v
doMount(arg, CMD_MOUNT, NULL)
  |
  +--> Validate target exists
  |
  +--> Check mounting requirements
  |
  +--> Combat mounting check (if in combat)
  |
  +--> Mount level check (bucking)
  |
  +--> rideCheck() for success
  |
  +--> mount() - establish pointers
  |
  +--> Setup follower relationship
```

### Combat Mounting Difficulty

Mounting while in combat requires a skill check with penalties:

```cpp
// code/code/misc/riding.cc:467-512
if (!isImmortal() && (fight() || horse->fight())) {
  learn = getSkillValue(SKILL_RIDE) +
          advancedRidingBonus(dynamic_cast<TMonster*>(horse));

  if (isTanking() || horse->isTanking()) {
    if (!hasClass(CLASS_DEIKHAN)) {
      learn /= 4;  // 25% skill effectiveness
    } else {
      learn /= 3;  // 33% for Deikhan
    }
    fightCheck = 1;
  } else if (!isAffected(AFF_ENGAGER)) {
    learn /= 2;   // 50% skill effectiveness
    fightCheck = 2;
  }

  if (!bSuccess(learn, SKILL_RIDE)) {
    // Failure - fall on butt, 2 round wait
  }
}
```

**Combat mounting penalties:**

| Combat State | Non-Deikhan | Deikhan |
|--------------|-------------|---------|
| Tanking (you or mount) | skill / 4 (25%) | skill / 3 (33%) |
| Fighting (not tanking) | skill / 2 (50%) | skill / 2 (50%) |
| Engaged only | full skill | full skill |
| Not in combat | full skill | full skill |

**Source:** `code/code/misc/riding.cc:467-512`

### Mount Level Rejection

Mounts higher level than the rider will reject mounting attempts:

```cpp
// code/code/misc/riding.cc:549-587
if (horse->GetMaxLevel() > GetMaxLevel()) {
  switch (::number(0, 3)) {
    case 0: act("$N bucks you off...");
    case 1: act("$N quickly moves...");
    case 2: act("You get your foot caught...");
    default: act("$N turns and knocks you down...");
  }
  setPosition(POSITION_SITTING);
  addToWait(combatRound(1));
  // Mount becomes hostile via aiHorse()
}
```

**Result:** Rider ends up sitting, 1 combat round wait, mount may become aggressive.

**Source:** `code/code/misc/riding.cc:549-587`

### mount() - Core Mount Function

```cpp
// code/code/misc/riding.cc:84-222
int TThing::mount(TThing* thing) {
  // 1. Height check (max 150% mount height)
  if (getHeight() > (thing->getHeight() * 3 / 2)) {
    sendTo("You are too big to ride that!\n\r");
    return FALSE;
  }

  // 2. Slot availability check
  int slots_needed = (getHeight() > thing->getHeight() * 2 / 3) ? 2 : 1;
  int slots_used = countRiders(thing);
  if (slots_used + slots_needed > MAX_RIDERS) {
    sendTo("There is no room for you!\n\r");
    return FALSE;
  }

  // 3. Add to rider chain
  if (!thing->rider) {
    thing->rider = this;  // First rider
  } else {
    // Find end of chain
    for (t = thing->rider; t->nextRider; t = t->nextRider);
    t->nextRider = this;  // Append to chain
  }

  // 4. Set pointers
  riding = thing;
  nextRider = NULL;

  // 5. Change position
  if (getPosition() > POSITION_SITTING)
    setPosition(POSITION_MOUNTED);

  // 6. Mount becomes follower of primary rider
  if (thing->horseMaster() == this) {
    TBeing* mount = dynamic_cast<TBeing*>(thing);
    if (mount && !mount->master) {
      addFollower(mount);
    }
  }

  return TRUE;
}
```

**Source:** `code/code/misc/riding.cc:84-222`

### Voluntary Dismount

The dismount command handles safe voluntary dismounting:

```cpp
// code/code/misc/riding.cc:643-729
if (cmd == CMD_DISMOUNT) {
  // Validation
  if (!riding) return FALSE;
  if (horse->fight() && !hasClass(CLASS_DEIKHAN))
    return FALSE;  // Can't dismount fighting mount
  if (isCombatMode(ATTACK_BERSERK))
    return FALSE;  // Can't dismount while berserking
  if (roomp->getMoblim() && MobCountInRoom >= moblim)
    return FALSE;  // No room

  // Flying mount handling
  if (roomp->isFlyingSector()) {
    dismount(POSITION_FLYING);
  } else if (horse->isFlying()) {
    if (canFly()) {
      dismount(POSITION_STANDING);
      doFly();
    } else if (SKILL_RIDE_WINGED check passes) {
      // Coax mount to land
      horse->doLand();
      dismount(POSITION_STANDING);
    } else {
      return FALSE;  // "Order your mount to land"
    }
  } else {
    dismount(POSITION_STANDING);
  }
}
```

**Dismount restrictions:**

| Condition | Result | Bypass |
|-----------|--------|--------|
| Mount is fighting | "You can't dismount while your mount is fighting!" | Deikhan class |
| Berserking | "Your berserker rage prevents you from dismounting." | None |
| Room at mob limit | "There isn't enough room in here to dismount." | Immortal |
| Flying mount, no flight | Must coax to land or "Order your mount to land" | SKILL_RIDE_WINGED >= 0 |

**Source:** `code/code/misc/riding.cc:643-729`

### Dismount Chain Maintenance

The `dismount()` function properly maintains the rider chain:

```cpp
// code/code/misc/riding.cc:224-301
TThing* TThing::dismount(positionTypeT pos) {
  if (!riding) {
    vlogf(LOG_BUG, "%s not riding in call to dismount().");
    return NULL;
  }

  // Remove from chain
  if (riding->rider == this)
    riding->rider = nextRider;  // Was first rider
  else {
    // Find previous rider
    for (t = riding->rider; t && t->nextRider != this; t = t->nextRider);
    if (t) t->nextRider = nextRider;
  }

  // Apply AFFECT_HORSEOWNED to prevent mob theft
  if (isPc() && tmons) {
    affectedData aff;
    aff.type = AFFECT_HORSEOWNED;
    aff.duration = 1 * Pulse::UPDATES_PER_MUDHOUR;
    tmons->affectTo(&aff);
  }

  // Handle master transfer or follower stop
  if (tbt->master == this) {
    if (!SKILL_TRAIN_MOUNT check) {
      tbt->stopFollower(TRUE);
      // Transfer master to new horseMaster
      t = tbt->horseMaster();
      if (tb3) tb3->addFollower(tbt);
    }
  }

  // Clear pointers
  nextRider = NULL;
  riding = NULL;
  ch->setPosition(pos);

  return this;
}
```

**AFFECT_HORSEOWNED:** When a PC dismounts, the mount gets a 1 mud-hour protection from mobs mounting it, preventing "horse theft" by NPCs.

**Source:** `code/code/misc/riding.cc:224-301`

## Movement While Mounted

### Movement Control

Only the primary rider (horseMaster) can control movement:

```cpp
// code/code/misc/movement.cc:1357-1394
int TBeing::doMove(dirTypeT cmd) {
  if (riding && (this != riding->horseMaster())) {
    act("You are not the master of $N, and can't control where $E goes.",
        FALSE, this, 0, riding, TO_CHAR);
    return FALSE;
  }

  // If this is a mount with riders, rider controls movement
  TBeing* tb = dynamic_cast<TBeing*>(rider);
  if (tb) {
    if (tb->rideCheck(0)) {
      rc = tb->doMove(cmd);  // Rider moves, mount follows
    } else {
      rc = tb->fallOffMount(this, POSITION_SITTING);
    }
    return rc;
  }
  ...
}
```

**Source:** `code/code/misc/movement.cc:1357-1394`

### Movement Point Costs

Movement costs are modified when mounted:

```cpp
// code/code/misc/movement.cc:780-792
if (riding) {
  if (riding->isLevitating())
    need_movement /= 4;
  if (riding->isFlying())
    need_movement = 1;

  TBeing* tbr = dynamic_cast<TBeing*>(riding);
  if (tbr) {
    if (tbr->getMove() < need_movement) {
      sendTo("Your mount is too exhausted.\n\r");
      return FALSE;
    }
  }
}

// Movement deduction (code/code/misc/movement.cc:1071-1077)
if (riding) {
  TBeing* tbr = dynamic_cast<TBeing*>(riding);
  if (tbr) tbr->addToMove(-need_movement);  // Mount pays full
  addToMove(-::number(0, need_movement) / 3);  // Rider pays ~1/3
}
```

**Movement cost distribution:**

| Mount State | Mount Cost | Rider Cost |
|-------------|------------|------------|
| Walking | Full terrain cost | random(0, cost) / 3 |
| Levitating | cost / 4 | random(0, cost/4) / 3 |
| Flying | 1 | random(0, 1) / 3 |

**Source:** `code/code/misc/movement.cc:780-792, 1071-1077`

### Movement Failure Conditions

Several conditions cause movement to fail while mounted:

```cpp
// code/code/misc/movement.cc:750-800
// Weight collapse
if (riding && compareWeights(riding->getWeight(), getTotalWeight(TRUE)) == 1) {
  act("$N collapses beneath your weight.", ...);
  tbr->setMove(0);
  rc = fallOffMount(riding, POSITION_SITTING);
}

// Drunkenness
if (riding && getCond(DRUNK) > 9) {
  sendTo("You wobble drunkenly as your mount moves along.\n\r");
  if (!::number(0, 4)) {  // 20% chance
    sendTo("One of those purple elephants pushed you off.\n\r");
    rc = fallOffMount(riding, POSITION_SITTING);
  }
}

// Rider exhaustion
if (riding && getMove() < 1) {
  act("You're too tired to stay on your $o.", ...);
  rc = fallOffMount(riding, POSITION_SITTING);
}
```

**Source:** `code/code/misc/movement.cc:750-800`

### Movement Validation

The `validMove()` function adds mount-specific checks:

```cpp
// code/code/misc/movement.cc:258-280
if (riding) {
  tbt = dynamic_cast<TBeing*>(riding);
  if (tbt && tbt->fight()) {
    sendTo("Your mount is fighting!\n\r");
    return FALSE;
  }
  if (tbt && tbt->getPosition() < POSITION_FIGHTING) {
    sendTo("Your mount must be standing!\n\r");
    return FALSE;
  }
}

// code/code/misc/movement.cc:366-384
if (tbt && tbt->willBumpHeadDoor(exitp, &iHeight)) {
  sendTo("Your mount refuses to go that way.\n\r");  // Door too low
  return FALSE;
}
if (tbt && (tbt->getRace() == RACE_FISH) && !rp->isUnderwaterSector()) {
  sendTo("Your mount refuses to go that way.\n\r");  // Fish out of water
  return FALSE;
}
if (rp->isRoomFlag(ROOM_DEATH)) {
  if (riding) {
    sendTo("Your mount refuses to go that way.\n\r");  // Death room
    return FALSE;
  }
}
```

**Mount movement restrictions:**

| Condition | Error Message |
|-----------|---------------|
| Mount fighting | "Your mount is fighting!" |
| Mount not standing | "Your mount must be standing!" |
| Door too low | "Your mount refuses to go that way." |
| Fish mount + non-water | "Your mount refuses to go that way." |
| Death room | "Your mount refuses to go that way." |
| Underwater (mount can't swim) | "Your mount refuses to go underwater." |
| Climbing sector | "Your mount refuses to climb for you." |

**Source:** `code/code/misc/movement.cc:258-280, 366-384`

### Continuous Riding (TASK_RIDE)

The `ride <direction>` command starts continuous movement:

```cpp
// code/code/misc/riding.cc:311-316
if (!task && riding && (getDirFromChar(arg) != DIR_NONE)) {
  sendTo("You urge your mount forward.\n\r");
  start_task(this, NULL, NULL, TASK_RIDE, arg, 2, inRoom(), 0, 0, 5);
  return TRUE;
}
```

**Task behavior (`code/code/task/task_ride.cc`):**
- Continues in initial direction while path exists
- At intersections with 2 exits, follows the non-backtrack exit
- At multi-way intersections or dead ends, stops
- Interrupted by combat, other commands, or explicit stop

**Source:** `code/code/task/task_ride.cc:1-97`

## The Ride Skill

### SKILL_RIDE and rideCheck()

The base riding skill available to all classes:

```cpp
// code/code/misc/riding.cc:736-767
int TBeing::rideCheck(int mod) {
  if (isImmortal())
    return TRUE;

  TBeing* tbt = dynamic_cast<TBeing*>(riding);

  // Saddle bonus (primary rider only)
  if (tbt && tbt->hasSaddle() && tbt->horseMaster() == this)
    mod += 8;

  // Deikhan bonuses
  if (tbt && hasClass(CLASS_DEIKHAN)) {
    mod += 5;  // Base Deikhan bonus
    mod += ::number(0, advancedRidingBonus(tbt)) / 15;  // 0-6 additional
  }

  // Secondary rider penalty
  if (tbt && tbt->horseMaster() != this)
    mod -= 5;

  int learn = getSkillValue(SKILL_RIDE);
  learn += (3 * mod);  // Each modifier point = 3 skill points

  return bSuccess(learn, SKILL_RIDE);
}
```

**Formula:** `effective_skill = base_skill + (3 × modifiers)`

**rideCheck modifiers:**

| Modifier | Value | Condition |
|----------|-------|-----------|
| Saddle bonus | +8 | Primary rider with saddled mount |
| Deikhan base | +5 | Deikhan class |
| Advanced riding | +0 to +6 | Deikhan with SKILL_ADVANCED_RIDING |
| Secondary rider | -5 | Not the primary rider |
| Mount damage (combat) | -5 | When mount takes damage |
| Rider damage (combat) | -10 | When rider takes damage |

**Source:** `code/code/misc/riding.cc:736-767`

### Saddle Detection

```cpp
// code/code/misc/riding.cc:108-122
bool TBeing::hasSaddle() const {
  if (!isRideable())
    return FALSE;
  if (!(obj = equipment[WEAR_BACK]))
    return FALSE;
  TBaseClothing* tbc = dynamic_cast<TBaseClothing*>(obj);
  TBaseContainer* tbc2 = dynamic_cast<TBaseContainer*>(obj);
  if (tbc && tbc->isSaddle())
    return 1;          // Riding saddle
  if (tbc2 && tbc2->isSaddle())
    return (tbc2->isSaddle());  // May return 2 for pack saddle
  return FALSE;
}
```

**Saddle types:**
- **Riding saddle (1):** Grants +8 rideCheck bonus to primary rider
- **Pack saddle (2):** Prevents mounting ("You cannot ride $N when it is saddled with a pack.")

**Source:** `code/code/misc/riding.cc:108-122`

## Mount Types and Deikhan Skills

### Mount Type Classification

Each mount race maps to a riding skill:

```cpp
// code/code/misc/riding.cc:14-58
spellNumT TBeing::mountSkillType() const {
  switch (getRace()) {
    // Domestic mounts
    case RACE_HORSE:
    case RACE_BOVINE:
    case RACE_OX:
    case RACE_PIG:
    case RACE_SHEEP:
    case RACE_BAANTA:
    case RACE_CANINE:
    case RACE_GOAT:
      return SKILL_RIDE_DOMESTIC;

    // Non-domestic mounts
    case RACE_RHINO:
    case RACE_TIGER:
    case RACE_GIRAFFE:
    case RACE_BEAR:
    case RACE_BOAR:
    case RACE_ELEPHANT:
    case RACE_DEER:
      return SKILL_RIDE_NONDOMESTIC;

    // Winged mounts
    case RACE_GRIFFON:
    case RACE_HIPPOGRIFF:
    case RACE_WYVERN:
    case RACE_DRAGON:
    case RACE_DRAGONNE:
    case RACE_LAMMASU:
    case RACE_SHEDU:
    case RACE_SPHINX:
      return SKILL_RIDE_WINGED;

    // Exotic mounts
    case RACE_FELINE:
    case RACE_BASILISK:
    case RACE_CENTAUR:
    case RACE_CHIMERA:
    case RACE_FROG:
    case RACE_LAMIA:
    case RACE_MANTICORE:
    case RACE_TURTLE:
    case RACE_LION:
    case RACE_LEOPARD:
    case RACE_COUGAR:
    case RACE_WYVELIN:
      return SKILL_RIDE_EXOTIC;

    default:
      return SKILL_RIDE_EXOTIC;
  }
}
```

**Source:** `code/code/misc/riding.cc:14-58`

### Deikhan Mount Skills

Deikhans have specialized mount skills in the DISC_MOUNTED discipline:

| Skill | Start Level | Learn Rate | Description |
|-------|-------------|------------|-------------|
| SKILL_CALM_MOUNT | 1 | 2 | Reduces mount anger/malice/suspicion |
| SKILL_TRAIN_MOUNT | 26 | 2 | Mount continues following after dismount |
| SKILL_ADVANCED_RIDING | 46 | 2 | Improves all riding bonuses |
| SKILL_RIDE_DOMESTIC | 5 | 2 | Riding domestic animals |
| SKILL_RIDE_NONDOMESTIC | 36 | 2 | Riding wild animals |
| SKILL_RIDE_WINGED | 66 | 3 | Riding flying creatures |
| SKILL_RIDE_EXOTIC | 85 | 7 | Riding unusual creatures |

**Source:** `code/code/misc/spell_info.cc:2049-2089`

### Advanced Riding Bonus

```cpp
// code/code/disc/disc_deikhan_mounted.cc:5-23
int TBeing::advancedRidingBonus(TMonster* mount) {
  int skillTotal = 0;

  if (!mount) return 0;

  if (doesKnowSkill(SKILL_ADVANCED_RIDING)) {
    if (bSuccess(SKILL_ADVANCED_RIDING))
      skillTotal += getSkillValue(SKILL_ADVANCED_RIDING);
  }
  if (doesKnowSkill(mount->mountSkillType())) {
    if (bSuccess(mount->mountSkillType()))
      skillTotal += getSkillValue(mount->mountSkillType());
  }

  return (skillTotal / 2);  // Average of both skills
}
```

**Range:** 0-100 (average of SKILL_ADVANCED_RIDING and mount-type skill)

**Usage in rideCheck:** `mod += ::number(0, advancedRidingBonus(tbt)) / 15` = +0 to +6 bonus

**Source:** `code/code/disc/disc_deikhan_mounted.cc:5-23`

### Calm Mount

```cpp
// code/code/disc/disc_deikhan_mounted.cc:25-43
void TBeing::calmMount(TBeing* m) {
  TMonster* mount = dynamic_cast<TMonster*>(m);

  if (!mount || !doesKnowSkill(SKILL_CALM_MOUNT) ||
      !bSuccess(SKILL_CALM_MOUNT))
    return;

  int skillTotal = getSkillValue(SKILL_CALM_MOUNT);
  skillTotal += advancedRidingBonus(mount);

  int amt = ::number(0, skillTotal / 30);  // 0-6 reduction

  if ((mount->anger() + 20) > mount->defanger())
    mount->DA(amt);
  if ((mount->malice() + 20) > mount->defmalice())
    mount->DMal(amt);
  if ((mount->susp() + 20) > mount->defsusp())
    mount->DS(amt);
}
```

**Reduction formula:** `random(0, (SKILL_CALM_MOUNT + advancedRidingBonus) / 30)`

**Reduction range:** 0 to 6 points per call (based on combined skill / 30)

**Use case:** Prevents mount from attacking due to high anger/malice, allows mounting aggressive creatures.

**Source:** `code/code/disc/disc_deikhan_mounted.cc:25-43`

### Train Mount

SKILL_TRAIN_MOUNT allows mounts to continue following after dismount:

```cpp
// code/code/misc/riding.cc:260-282
if (tbt->master == this) {
  if (!ch->doesKnowSkill(SKILL_TRAIN_MOUNT) ||
      !ch->bSuccess(ch->getSkillValue(SKILL_TRAIN_MOUNT) / 2, SKILL_TRAIN_MOUNT)) {
    tbt->stopFollower(TRUE);
    // Transfer to new horseMaster
  }
  // Else: mount continues following despite dismount
}
```

**Check:** skill / 2 success roll to retain mount following

**Source:** `code/code/misc/riding.cc:260-282`

## Flying Mounts

### Mounting Flying Creatures

Non-Deikhans cannot mount flying creatures:

```cpp
// code/code/misc/riding.cc:415-466
if (horse->isFlying() && !isFlying()) {
  if (!hasClass(CLASS_DEIKHAN)) {
    sendTo("You can't mount something that is flying.\n\r");
    return FALSE;
  }
  // Deikhan must coax mount to land
  if (getSkillValue(SKILL_RIDE_WINGED) < 70) {
    sendTo("You don't know enough about winged creatures...\n\r");
    return FALSE;
  }
  if (::number(-10, getSkillValue(SKILL_RIDE_WINGED)) > 0) {
    act("You coax $N to land so you can mount.", ...);
    horse->doLand();
  } else {
    act("$N ignores you.", ...);
    return FALSE;
  }
}
```

**Requirements to mount flying creature:**
1. Be Deikhan class
2. Have SKILL_RIDE_WINGED >= 70
3. Pass skill check: random(-10, skill) > 0

**Source:** `code/code/misc/riding.cc:415-466`

### Dismounting Flying Mounts

Special handling when dismounting in air:

| Sector Type | Dismount Result |
|-------------|-----------------|
| FLYING sector | Dismount to POSITION_FLYING (magic air) |
| AIR/VERT + can fly | Dismount + doFly() |
| Flying mount + can fly | Dismount + doFly() |
| Flying mount + SKILL_RIDE_WINGED | Coax to land, then dismount |
| Otherwise | "Order your mount to land" - blocked |

**Source:** `code/code/misc/riding.cc:668-728`

### Falling Off Flying Mounts

Falling off a flying mount triggers fall checks:

```cpp
// code/code/misc/riding.cc:793-811
if (h->isFlying()) {
  if (h->roomp->isFlyingSector()) {
    rc = crashLanding(POSITION_FLYING);
  } else if (h->roomp->isFallSector()) {
    rc = checkFalling();  // Full falling damage
  } else {
    if (canFly()) {
      rc = crashLanding(POSITION_FLYING);
    } else {
      rc = crashLanding(POSITION_RESTING);  // Land damage
    }
  }
}
```

**Source:** `code/code/misc/riding.cc:793-811`

## Combat Modifications

### Position Bonuses

POSITION_MOUNTED (value 11) provides automatic combat bonuses:

```cpp
// code/code/misc/combat.cc:2596-2760 (attackRound)
case POSITION_MOUNTED:
    val = my_lev / 4 + 1;  // Attack bonus
    break;

// code/code/misc/combat.cc:2764-2964 (defendRound)
case POSITION_MOUNTED:
    val += my_lev / 4 + 1;  // Defense bonus
    break;
```

**Bonus scaling by level:**

| Level | Attack Bonus | Defense Bonus |
|-------|--------------|---------------|
| 10 | +3 | +3 |
| 20 | +6 | +6 |
| 30 | +8 | +8 |
| 40 | +11 | +11 |
| 50 | +13 | +13 |

**Source:** `code/code/misc/combat.cc` (lines 2596-2964)

### Attack Frequency Penalty

Mounted characters lose **1/3 of their attacks**:

```cpp
// code/code/misc/offense.cc:2263-2271
if (dynamic_cast<TBeing*>(riding)) {
    float factor = 0.67;  // 67% attack frequency

    fx *= factor;  // Primary hand attacks
    fy *= factor;  // Secondary hand attacks
}
```

**Example attack counts:**

| Unmounted Attacks | Mounted Attacks | Lost |
|-------------------|-----------------|------|
| 3.0 | 2.0 | 1.0 |
| 4.5 | 3.0 | 1.5 |
| 6.0 | 4.0 | 2.0 |

**Trade-off:** Fewer attacks offset by increased accuracy/defense from position bonuses and Chivalry.

**Source:** `code/code/misc/offense.cc` (lines 2263-2271)

### Special Attack Bonus

Mounted position provides **+2 bonus** to special attack rolls (bash, trip, etc.):

```cpp
// code/code/misc/combat.cc:2971-2997
case POSITION_MOUNTED:
    mod += 2;
    break;
```

This improves success chance for skills like `SKILL_BASH`, `SKILL_TRIP`, `SKILL_DISARM`.

**Source:** `code/code/misc/combat.cc` (lines 2971-2997)

### Chivalry Skill Bonuses

Deikhan characters with `SKILL_CHIVALRY` receive massive combat bonuses when mounted:

**Attack Bonus:**

```cpp
// code/code/misc/combat.cc:2635-2645
if (doesKnowSkill(SKILL_CHIVALRY) && getPosition() == POSITION_MOUNTED) {
    int amt = 74;  // Base bonus
    amt *= max(10, (int)getSkillValue(SKILL_CHIVALRY));
    amt /= MAX_SKILL_LEARNEDNESS;  // 100
    bonus += amt;
}
```

**Scaling:**

| Chivalry Learning | Attack Bonus |
|-------------------|--------------|
| 10 (minimum) | +7 |
| 50 | +37 |
| 100 (max) | +74 |

**Defense Bonus:**

```cpp
// code/code/misc/combat.cc:2867-2878
if (doesKnowSkill(SKILL_CHIVALRY) && getPosition() == POSITION_MOUNTED) {
    int amt = 159;  // Gets divided by 4 in formula
    amt *= max(10, (int)getSkillValue(SKILL_CHIVALRY));
    amt /= 100;
    bonus += amt;  // Final result: 159 at 100 learning
}
```

**Scaling:**

| Chivalry Learning | Defense Bonus |
|-------------------|---------------|
| 10 (minimum) | +16 |
| 50 | +80 |
| 100 (max) | +159 |

**Source:** `code/code/misc/combat.cc` (lines 2635-2645, 2867-2878)

## Stability and Fall-Off System

### Two-Check Fall-Off System

Riders must **fail TWO consecutive rideCheck() calls** to fall off. This provides more stability than a single check.

**When Mount Takes Damage:**

```cpp
// code/code/misc/combat.cc:4761-4788
if (riding) {
    if (dynamic_cast<TBeing*>(riding)) {
        // First check with -5 modifier
        if (!rideCheck(-5)) {
            // Second check with -5 modifier
            if (!rideCheck(-5)) {
                // Failed both - fall off!
                rc = fallOffMount(riding, POSITION_SITTING);
                if (IS_SET_DELETE(rc, DELETE_THIS))
                    return DELETE_THIS;
                return FALSE;
            }
        }
    }
}
```

**When Rider Takes Damage:**

All riders on mount must check when mount is damaged:

```cpp
// code/code/misc/combat.cc:4789-4806
for (t = rider; t; t = t->nextRider) {
    TBeing* tb = dynamic_cast<TBeing*>(t);
    if (tb) {
        // First check with -10 modifier
        if (!tb->rideCheck(-10)) {
            // Second check with -10 modifier
            if (!tb->rideCheck(-10)) {
                // Failed both - rider falls off
                rc = tb->fallOffMount(this, POSITION_SITTING);
                if (IS_SET_DELETE(rc, DELETE_THIS)) {
                    delete tb;
                    tb = NULL;
                }
                return FALSE;
            }
        }
    }
}
```

**Modifier comparison:**

| Trigger | Modifier | Effective Skill Penalty |
|---------|----------|-------------------------|
| Mount takes damage | -5 | -15 skill points |
| Rider takes damage | -10 | -30 skill points |

**Source:** `code/code/misc/combat.cc` (lines 4761-4806)

### fallOffMount() - Involuntary Dismount

Handles fall damage and potential death:

```cpp
// code/code/misc/riding.cc:774-843
int TBeing::fallOffMount(TThing* h, positionTypeT pos, bool death) {
  act("$n loses control and falls off of $N.", ...);

  if (!h->isPc())
    dynamic_cast<TMonster*>(h)->aiHorse(this);  // Mount becomes hostile

  dismount(pos);

  // Fall consequences for flying mounts
  if (h->isFlying()) {
    if (h->roomp->isFlyingSector()) {
      rc = crashLanding(POSITION_FLYING);
    } else if (h->roomp->isFallSector()) {
      rc = checkFalling();
    } else {
      if (canFly()) rc = crashLanding(POSITION_FLYING);
      else rc = crashLanding(POSITION_RESTING);
    }
  } else if (h->roomp->isFallSector()) {
    rc = checkFalling();
  }

  if (IS_SET_DELETE(rc, DELETE_THIS))
    return DELETE_THIS;
  return TRUE;
}
```

**Fall triggers:**
- Failed rideCheck() during movement
- Mount exhaustion (0 movement points)
- Rider exhaustion (0 movement points)
- Drunkenness (DRUNK > 9, 20% chance per move)
- Mount weight collapse (rider too heavy)
- Combat damage (two failed rideCheck() calls)

**Source:** `code/code/misc/riding.cc:774-843`

### No Damage Transfer Between Rider and Mount

**IMPORTANT:** Rider and mount maintain **independent HP pools**. There is NO automatic damage sharing.

**Rider Takes Damage:**
```cpp
// Rider HP decreases
// Mount HP unchanged
// Rider makes two rideCheck(-10) calls
// Rider may fall off if both checks fail
```

**Mount Takes Damage:**
```cpp
// Mount HP decreases
// Rider HP unchanged
// Rider makes two rideCheck(-5) calls
// Rider may fall off if both checks fail
```

**Mount Death:**

When mount dies:

```cpp
// code/code/misc/combat.cc:620-641 (in die() function)
for (t = rider; t; t = t2) {
    t2 = t->nextRider;  // Cache next before operations
    TBeing* tb = dynamic_cast<TBeing*>(t);
    if (tb) {
        tb->dismount(POSITION_SITTING);
        tb->sendTo("Your mount collapses beneath you!\n\r");
    }
}
```

Riders are forcibly dismounted but take **no damage** from mount death (unless subsequently trampled by corpse weight, handled separately).

**Rider Death:**

When rider dies while mounted:

```cpp
// code/code/misc/combat.cc:470-490 (in rawKill() function)
if (riding) {
    dismount(POSITION_DEAD);  // Corpse falls off mount
}
```

Mount is unaffected; rider corpse falls to ground.

## Mount AI and Follower System

### Follower Relationship

When mounted, the mount becomes a follower of the primary rider:

```cpp
// code/code/misc/riding.cc:617-627
// Mount was following someone else
if (horse->master && horse->master != this && !horse->rider)
  horse->stopFollower(TRUE);

// Mount should follow primary rider
if (!horse->master)
  addFollower(horse);

horse->specials.hunting = 0;
dynamic_cast<TMonster*>(horse)->setTarg(NULL);
dynamic_cast<TMonster*>(horse)->hates.clist = NULL;
dynamic_cast<TMonster*>(horse)->fears.clist = NULL;
```

**Effects:**
- Mount appears in `group` command
- Mount follows rider during movement
- Mount's hate/fear lists are cleared
- Mount's hunting target is cleared

**Source:** `code/code/misc/riding.cc:617-627`

### Mount AI Suppression

While mounted, mount's autonomous AI is suppressed:

```cpp
// In mobileActivity():
if (rider)
    return FALSE;  // Skip AI processing if anyone is riding
```

**Suppressed behaviors:**
- Wandering/random movement
- Aggression checks (`aggro()`, `aggroCheck()`)
- Scavenging
- Hunting
- Fear reactions

**Exceptions:**
- Spec procs still fire (mount-specific behaviors)
- Spell effects continue (buffs, debuffs)
- Combat actions when attacked directly

### aiHorse() - Mount Reaction

When a rider falls off or fails to mount, the mount may become hostile:

```cpp
// code/code/misc/ai_utility.cc:270-275
void TMonster::aiHorse(TBeing* ch) {
  aiTarget(ch);  // May target the failed rider
  UA(3);         // Increase anger by 3
  UM(1);         // Increase malice by 1
  US(4);         // Increase suspicion by 4
}
```

**Called when:**
- Mount attempt fails
- Rider falls off involuntarily
- Mount is bucked off by high-level mount

**Source:** `code/code/misc/ai_utility.cc:270-275`

### lookForHorse() - NPC Mounting

NPCs can automatically look for and mount horses:

```cpp
// code/code/misc/riding.cc:125-222
int TMonster::lookForHorse() {
  // Skip if: utility mob, sentinel, shopkeeper, low health
  // Skip if: already mounted, has rider, is fighting, is rideable

  for (horse in room) {
    if (canRide(horse) && horse->master == NULL &&
        !horse->affectedBySpell(AFFECT_HORSEOWNED) &&
        horse->GetMaxLevel() + 4 <= GetMaxLevel()) {
      rc = doMount(NULL, CMD_MOUNT, horse);
      return TRUE;
    }
  }
  return FALSE;
}
```

**NPC mounting restrictions:**
- Horse must not have AFFECT_HORSEOWNED (anti-theft protection)
- Horse level must be <= NPC level - 4
- Horse must not already be following someone
- Horse must be at full health

**Source:** `code/code/misc/riding.cc:125-222`

## DELETE Flag Safety Patterns

### Critical Pattern: DELETE_THIS from fallOffMount()

**WRONG: Ignoring return value**
```cpp
fallOffMount(riding, POSITION_SITTING);
sendTo("You fell!\n\r");  // Crash if dead
```

**CORRECT: Check and propagate**
```cpp
int rc = fallOffMount(riding, POSITION_SITTING);
if (IS_SET_DELETE(rc, DELETE_THIS))
  return DELETE_THIS;
sendTo("You fell!\n\r");
```

### Critical Pattern: Iterator Invalidation in Rider Loops

**WRONG: Chain modified during iteration**
```cpp
for (t = rider; t; t = t->nextRider) {
  TBeing* tb = dynamic_cast<TBeing*>(t);
  tb->dismount(POSITION_SITTING);  // Modifies nextRider!
}
```

**CORRECT: Cache next pointer**
```cpp
for (t = rider; t; t = t2) {
  t2 = t->nextRider;  // Cache before modification
  TBeing* tb = dynamic_cast<TBeing*>(t);
  if (tb) tb->dismount(POSITION_SITTING);
}
```

### Critical Pattern: Null Checks Before Mount Operations

**WRONG: No validation**
```cpp
rideCheck(-5);  // Assumes riding is valid TBeing
```

**CORRECT: Validate first**
```cpp
TBeing* tbt = dynamic_cast<TBeing*>(riding);
if (!tbt)
    return TRUE;  // Or appropriate handling
rideCheck(-5);
```

### Critical Pattern: Position State Consistency

**BUG: Position mismatch**
```cpp
if (riding && getPosition() != POSITION_MOUNTED) {
    // Inconsistent state - rider on mount but wrong position
    vlogf(LOG_BUG, "Rider %s has invalid position", getName());
}
```

**CORRECT: Always sync position with mount state**
```cpp
if (riding)
    setPosition(POSITION_MOUNTED);
else if (getPosition() == POSITION_MOUNTED)
    setPosition(POSITION_STANDING);
```

### Critical Pattern: Single vs Two rideCheck() Calls

**BUG: Fall-off too easily**
```cpp
if (!rideCheck(-5)) {
    fallOffMount(riding, POSITION_SITTING);
}
```

**CORRECT: Two checks required**
```cpp
if (!rideCheck(-5) && !rideCheck(-5)) {
    fallOffMount(riding, POSITION_SITTING);
}
```

## Common Gotchas

### 1. Primary Rider vs Any Rider

```cpp
// WRONG: Assuming any rider controls movement
if (riding)
  doMove(dir);  // Only works if horseMaster

// CORRECT: Check if primary rider
if (riding && riding->horseMaster() == this)
  doMove(dir);
else if (riding)
  sendTo("You don't control this mount.\n\r");
```

### 2. Deikhan Bypass Checks

Several restrictions are bypassed for Deikhans:
- Mounting fighting creatures
- Dismounting while mount fights
- Mounting flying creatures (with skill)

Always check `hasClass(CLASS_DEIKHAN)` before denying mount-related actions.

### 3. Assuming Damage Transfer

```cpp
// BUG: Expecting damage sharing
if (mount->getHit() < 50) {
    rider->sendTo("Your mount is badly wounded!\n\r");
    // Wrong: Assuming rider needs healing too
}

// CORRECT: Independent HP tracking
if (mount->getHit() < 50) {
    rider->sendTo("Your mount is badly wounded!\n\r");
    // Rider may be at full health
}
if (rider->getHit() < 50) {
    rider->sendTo("You are badly wounded!\n\r");
    // Mount may be at full health
}
```

### 4. Multiple Rider Coordination

All riders on the same mount share certain restrictions:

```cpp
// Movement: Only primary rider controls direction
// Position: All riders must have POSITION_MOUNTED
// Dismount: Any rider can dismount independently
// Combat: Each rider has separate combat state
```

**Example scenario:**
- Primary rider (warrior) controls movement
- Secondary rider (mage) casts spells
- Tertiary rider (cleric) heals party
- All receive mounted combat bonuses
- All make separate rideCheck() calls when mount damaged

## Key Constants

| Constant | Value | Description |
|----------|-------|-------------|
| MAX_RIDERS | 4 | Maximum rider slots per mount |
| POSITION_MOUNTED | 11 | Position enum value when mounted |
| SADDLE_BONUS | +8 | rideCheck modifier for saddled mount |
| DEIKHAN_BASE_BONUS | +5 | rideCheck modifier for Deikhan |
| SECONDARY_RIDER_PENALTY | -5 | rideCheck modifier for non-primary rider |
| MOUNT_DAMAGE_MODIFIER | -5 | rideCheck penalty when mount damaged |
| RIDER_DAMAGE_MODIFIER | -10 | rideCheck penalty when rider damaged |
| MIN_HEIGHT_RATIO | 60% | Minimum mount height relative to rider |
| MAX_HEIGHT_RATIO | 250% | Maximum mount height relative to rider |
| LARGE_RIDER_THRESHOLD | 66% | Rider height relative to mount for 2 slots |
| HORSEOWNED_DURATION | 1 mud hour | Protection time after PC dismounts |
| ATTACK_FREQUENCY_PENALTY | 0.67 | Multiplication factor for attacks (lose 1/3) |
| CHIVALRY_ATTACK_BONUS | +74 | Max attack bonus at 100 Chivalry learning |
| CHIVALRY_DEFENSE_BONUS | +159 | Max defense bonus at 100 Chivalry learning |
| POSITION_ATTACK_BONUS | level/4 + 1 | Attack bonus from POSITION_MOUNTED |
| POSITION_DEFENSE_BONUS | level/4 + 1 | Defense bonus from POSITION_MOUNTED |
| SPECIAL_ATTACK_BONUS | +2 | Special attack roll bonus when mounted |

## Key Source Files

| File | Lines | Purpose |
|------|-------|---------|
| `code/code/misc/riding.cc` | 961 | Core mounting mechanics, rideCheck, fallOffMount |
| `code/code/misc/movement.cc` | ~2000+ | Movement while mounted, validation |
| `code/code/misc/combat.cc` | 6200+ | Combat bonuses, fall-off triggers, mounted position handling |
| `code/code/misc/offense.cc` | 2282 | Attack frequency penalty calculation |
| `code/code/task/task_ride.cc` | 97 | Continuous riding task |
| `code/code/disc/disc_deikhan_mounted.cc` | 44 | Deikhan advanced riding and calm mount abilities |
| `code/code/misc/ai_utility.cc` | ~300 | aiHorse() mount AI |
| `code/code/misc/race.cc` | ~1300 | isRidable() racial flag |

## Related Documentation

- [Combat Rounds](combat-rounds.md) - Combat round mechanics, attack frequency
- [Position Stance](position-stance.md) - POSITION_MOUNTED effects
- [DELETE Flags](delete-flags.md) - DELETE_THIS handling patterns
- [Skill Learning](skill-learning.md) - Skill system, bSuccess() checks
- [Movement System](movement-system.md) - Movement mechanics, terrain costs
