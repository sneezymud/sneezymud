---
title: Movement and Terrain Navigation
description: Complete movement system including basic movement, flying, climbing/falling, swimming/drowning, portals/teleportation, and DELETE flag safety patterns
keywords: [doMove, rawMove, DELETE_THIS, reconcileDamage, checkFalling, crashLanding, checkDrowning, riverFlow, genericTeleport, movement points, sector types, AFF_FLYING, SECT_VERTIGO, canClimb, fallKill, procCharFalling, procCharDrowning, water breathing, astral walk]
category: Important Systems

last_updated: 2026-01-29
source_files: [code/code/misc/movement.cc, code/code/misc/physics.cc, code/code/misc/being.cc]
related:
  - combat-rounds.md
  - memory-safety.md
  - room-environment.md
  - position-stance.md
  - character-foundation.md
---

# Movement and Terrain Navigation

The movement system handles all character locomotion in SneezyMUD, including basic directional movement, vertical climbing and falling, water environments with drowning mechanics, flying, and instant travel via portals and teleportation spells.

**Misusing these systems causes crashes.** Common errors: not checking DELETE_THIS returns from movement functions, failing to propagate DELETE flags from falling/drowning, ignoring reconcileDamage() -1 return, dereferencing room pointers without validation after random teleports.

**Key files:** `code/code/misc/movement.cc`, `code/code/misc/physics.cc`, `code/code/misc/being.cc`, `code/code/obj/obj_portal.cc`

## Overview

Movement in SneezyMUD consists of several integrated subsystems:
- **Basic Movement**: Direction commands with terrain-based movement point costs
- **Flying**: Airborne movement with reduced costs and combat bonuses
- **Vertical Movement**: Climbing skill checks and fall damage mechanics
- **Water Mechanics**: Swimming, drowning, and river flow
- **Instant Travel**: Portals, teleportation spells, word of recall, summon

All movement operations must properly handle DELETE flags to prevent use-after-free crashes.

## Basic Movement

### Direction Commands

SneezyMUD supports 10 directional movement commands:

| Direction | Command | Enum Value |
|-----------|---------|------------|
| North | `north`, `n` | `DIR_NORTH` (0) |
| East | `east`, `e` | `DIR_EAST` (1) |
| South | `south`, `s` | `DIR_SOUTH` (2) |
| West | `west`, `w` | `DIR_WEST` (3) |
| Up | `up`, `u` | `DIR_UP` (4) |
| Down | `down`, `d` | `DIR_DOWN` (5) |
| Northeast | `northeast`, `ne` | `DIR_NORTHEAST` (6) |
| Northwest | `northwest`, `nw` | `DIR_NORTHWEST` (7) |
| Southeast | `southeast`, `se` | `DIR_SOUTHEAST` (8) |
| Southwest | `southwest`, `sw` | `DIR_SOUTHWEST` (9) |

**Source:** `code/code/misc/enum.h:100-114`

### doMove() - Entry Point

```cpp
int TBeing::doMove(dirTypeT cmd) {
    // Riding check - only horse master can direct movement
    if (riding && (this != riding->horseMaster())) {
        act("You are not the master of $N, and can't control where $E goes.",
            FALSE, this, 0, riding, TO_CHAR);
        return FALSE;
    }

    // Combat check - can't move while fighting
    if (fight()) {
        sendTo("You can't concentrate enough while fighting!\n\r");
        return FALSE;
    }

    // Delegate to moveOne or moveGroup based on followers
    if (!followers && !master) {
        rc = moveOne(cmd);
    } else {
        rc = moveGroup(cmd);
    }
    return rc;
}
```

**Source:** `code/code/misc/movement.cc:1352-1434`

### validMove() - Movement Validation

Before moving, `validMove()` checks multiple conditions:

```cpp
bool TBeing::validMove(dirTypeT cmd) {
    roomDirData* exitp = exitDir(cmd);

    // Check for SPELL_BIND entrapment
    if (aff->type == SPELL_BIND) {
        sendTo("You are entrapped in sticky webs!\n\r");
        return FALSE;
    }

    // Exit must exist and be passable
    if (!exit_ok(exitp, NULL)) {
        notLegalMove();
        return FALSE;
    }

    // Check door states: caved in, closed, warded
    if (IS_SET(exitp->condition, EXIT_CAVED_IN)) { ... }
    if (IS_SET(exitp->condition, EXIT_CLOSED)) { ... }
    if (IS_SET(exitp->condition, EXIT_WARDED)) { ... }

    // Room capacity limit
    if (rp->getMoblim() && MobCountInRoom(rp->stuff) >= rp->getMoblim()) {
        sendTo("Sorry, there is no room to get in there.\n\r");
        return FALSE;
    }

    return TRUE;
}
```

**Source:** `code/code/misc/movement.cc:232-394`

## Movement Point Costs

### Terrain-Based Costs

Movement cost is calculated as the average of source and destination terrain costs:

```cpp
need_movement = (TerrainInfo[from_here->getSectorType()]->movement +
                 TerrainInfo[to_here->getSectorType()]->movement) / 2;
```

**Source:** `code/code/misc/movement.cc:561-566`

### Terrain Cost Table

| Sector Type | Movement Cost | Notes |
|------------|---------------|-------|
| City sectors | 1 | Paved roads, easy travel |
| Road sectors | 1 | Including forest roads |
| Plains/Grasslands | 2-3 | Open terrain |
| Forest | 4 | Dense vegetation |
| Hills | 3 | Moderate elevation change |
| Mountains | 6 | Steep terrain |
| Swamp | 5 | Difficult footing |
| Underwater | 8 | Requires swimming |
| Climbing | 9 | Vertical movement |
| Solid Rock | 13 | Nearly impassable |
| Atmosphere | 0 | Flying sectors |

**Full terrain definitions:** `code/code/misc/constants.cc:77-198`

### Movement Modifiers

Several conditions modify the base movement cost:

| Condition | Modifier | Source |
|-----------|----------|--------|
| Sneaking | +2 | movement.cc:568-569 |
| Dwarf in water | +20 | movement.cc:571-574 |
| AFF_SWIM in water | /2 | movement.cc:577-578 |
| AFF_SWIM underwater | /4 | movement.cc:579-580 |
| Both legs hurt (crawling) | +20 | movement.cc:586-588 |
| One arm hurt (crawling) | +20 additional | movement.cc:589-594 |
| Foot wound | +5, chance to fall | movement.cc:605-624 |
| One leg hurt | +10, chance to fall | movement.cc:624-634 |
| Drunk (>9) | +1, chance to fall | movement.cc:635-656 |
| Crawling | +8 (horizontal), +16 (vertical) | movement.cc:657-662 |
| Flying | max(1, cost/4) | movement.cc:671-672 |
| Levitating | max(5, cost/4) | movement.cc:674-675 |
| Haste/Accelerate spells | /2 | movement.cc:677-679 |
| SKILL_HIKING (forest/mountain/swamp) | reduced by skill% | movement.cc:1048-1053 |

### Movement Deduction

```cpp
if (riding) {
    TBeing* tbr = dynamic_cast<TBeing*>(riding);
    if (tbr)
        tbr->addToMove(-need_movement);  // Mount pays full cost
    addToMove(-::number(0, need_movement) / 3);  // Rider pays 0-33%
} else
    addToMove(-need_movement);
```

**Source:** `code/code/misc/movement.cc:1071-1078`

## Door and Exit System

### Exit Flags

Each exit can have the following condition flags:

| Flag | Bit | Description |
|------|-----|-------------|
| `EXIT_CLOSED` | 0 | Door is shut, blocks passage |
| `EXIT_LOCKED` | 1 | Requires key to open |
| `EXIT_SECRET` | 2 | Hidden from casual observation |
| `EXIT_DESTROYED` | 3 | Door has been broken down |
| `EXIT_NOENTER` | 4 | Cannot pass through |
| `EXIT_TRAPPED` | 5 | Trap is set on the door |
| `EXIT_CAVED_IN` | 6 | Passage blocked by debris |
| `EXIT_WARDED` | 7 | Magical barrier requires ward key |
| `EXIT_SLOPED_UP` | 8 | Upward slope |
| `EXIT_SLOPED_DOWN` | 9 | Downward slope |
| `EXIT_JAMMED` | 10 | Stuck closed |

### Door Types

Different door types require different manipulation commands:

| Door Type | Open Command | Close Command |
|-----------|--------------|---------------|
| `DOOR_DOOR` | open | close |
| `DOOR_TRAPDOOR` | open | close |
| `DOOR_GATE` | open | close |
| `DOOR_PORTCULLIS` | raise | lower |
| `DOOR_DRAWBRIDGE` | lower | raise |

**Source:** `code/code/misc/enum.h:118-131`

**Important:** Portcullis and drawbridge logic is inverted:
- **Portcullis**: `raise` opens, `lower` closes
- **Drawbridge**: `lower` opens, `raise` closes

### Door Commands

#### open/close

```cpp
int TBeing::doOpen(const char* argument) {
    // Find the door
    door = findDoor(type, dir, DOOR_INTENT_OPEN, SILENT_NO);

    // Check door weight against character strength
    if ((exitp->weight * tRidingManip) > maxWieldWeight(NULL, HAND_TYPE_PRIM)) {
        sendTo("The door is too large and heavy for you to budge it.\n\r");
        return FALSE;
    }

    // Check door state
    if (IS_SET(exitp->condition, EXIT_DESTROYED)) { ... }
    if (IS_SET(exitp->condition, EXIT_CAVED_IN)) { ... }
    if (!IS_SET(exitp->condition, EXIT_CLOSED)) { ... }  // Already open
    if (IS_SET(exitp->condition, EXIT_LOCKED)) { ... }   // Locked

    // Check for traps
    if (IS_SET(exitp->condition, EXIT_TRAPPED)) {
        rc = triggerDoorTrap(door);
        if (IS_SET_DELETE(rc, DELETE_THIS))
            return DELETE_THIS;
    }

    rawOpenDoor(door);
}
```

**Source:** `code/code/misc/movement.cc:1850-1989`

**Critical:** Door operations modify BOTH sides (current exit AND reverse exit in destination room).

#### lock/unlock

```cpp
void TBeing::doLock(const char* argument) {
    // Validate door exists and is lockable
    if (exitp->key < 0)
        sendTo("There does not seem to be any keyholes.\n\r");
    else if (!has_key(this, exitp->key))
        sendTo("You don't have the proper key.\n\r");
    else if (!IS_SET(exitp->condition, EXIT_CLOSED))
        sendTo("You have to close it first, I'm afraid.\n\r");
    else if (IS_SET(exitp->condition, EXIT_LOCKED))
        sendTo("It's already locked!\n\r");
    else {
        SET_BIT(exitp->condition, EXIT_LOCKED);
        // Also lock the other side
        if (back = rp->dir_option[rev_dir(door)])
            SET_BIT(back->condition, EXIT_LOCKED);
        sendTo("*Click*\n\r");
    }
}
```

**Source:** `code/code/misc/movement.cc:2331-2473`

### Key System

Keys are checked across inventory, held items, worn items, and keyrings:

```cpp
bool has_key(TBeing* ch, int key) {
    // Check inventory
    for (t = ch->stuff; t; t = t->nextThing)
        if (keyCheck(o, key)) return true;

    // Check keyrings in inventory
    for (TKeyring* ring : keyrings)
        for (t2 = ring->stuff; t2; t2 = t2->nextThing)
            if (keyCheck(o, key)) return true;

    // Check held items
    if (t = ch->heldInPrimHand())
        if (keyCheck(o, key)) return true;

    // Check worn items
    for (wearSlotT i = WEAR_HEAD; i < MAX_WEAR; i++)
        if (keyCheck(ch->equipment[i], key)) return true;

    return false;
}
```

**Source:** `code/code/misc/movement.cc:2265-2329`

### Warded Exits

Warded exits require special ward keys worn or held to pass:

```cpp
int tryPassWardedExit(TBeing& ch, dirTypeT cmd) {
    // Immortals and ghosts pass through
    if (ch.isImmortal() || IS_SET(ch.specials.act, ACT_GHOST)) {
        ch.sendTo("You make yourself ethereal to pass through the ward.\n\r");
        return 1;
    }

    // Check for ward key in equipment
    for (int i = MIN_WEAR; i < MAX_WEAR; ++i) {
        const TObj* eq = dynamic_cast<TObj*>(ch.equipment[i]);
        if (eq && eq->objVnum() == wardedDir->second) {
            act("Your $o protects you from the magic and allows you to move.",
                false, &ch, eq, nullptr, TO_CHAR);
            return 1;
        }
    }

    act("You are prevented from moving by a magical ward.", false, &ch, ...);
    return 0;
}
```

**Source:** `code/code/misc/movement.cc:183-229`

## Flying and Levitation

### Flight Sources

Characters can fly through several mechanisms:

| Source | Method | Persist |
|--------|--------|---------|
| `SPELL_FLY` | Magical spell | Duration-based |
| `AFF_FLYING` | Racial/innate | Permanent until removed |
| `POSITION_FLYING` | Manual fly command | Until landing |
| Flying mount | Riding flying creature | While mounted |
| `SECT_MAKE_FLY` | Special sector type | While in sector |

### canFly() Check

```cpp
bool TBeing::canFly() const {
    // Check for fly spell
    if (affectedBySpell(SPELL_FLY))
        return true;

    // Check for flying affect
    if (isAffected(AFF_FLYING))
        return true;

    // Check racial flying ability
    if (race->isWinged())
        return true;

    return false;
}
```

**Source:** `code/code/misc/being.h:1149`

### isFlying() vs isLevitating()

```cpp
bool TBeing::isFlying() const {
    return (getPosition() == POSITION_FLYING);
}

bool TBeing::isLevitating() const {
    return isAffected(AFF_LEVITATING);
}
```

**Key differences:**
- **Flying**: Active flight at `POSITION_FLYING`, movement cost = 1
- **Levitating**: Floating above ground, movement cost = cost/4 (max 5)

### doFly() - Take Flight

```cpp
void TBeing::doFly() {
    if (isFlying()) {
        sendTo("You are already flying.\n\r");
        return;
    }
    if (!canFly()) {
        sendTo("You flap and flap, but seem to be less than flightworthy.\n\r");
        return;
    }
    if (riding) {
        sendTo("You can't fly while riding something else.\n\r");
        return;
    }
    if (roomp->isUnderwaterSector()) {
        sendTo("It hurts your brain too much even contemplating how to fly underwater?!?\n\r");
        return;
    }
    // Winged races need preening
    if (race->isWinged() && race->isFeathered() &&
        !isAffected(AFF_FLIGHTWORTHY) && !isImmortal()) {
        sendTo("Your flight feathers are too dirty to fly properly.\n\r");
        return;
    }

    act("You take to the air and start flying about.", TRUE, this, 0, 0, TO_CHAR);
    setPosition(POSITION_FLYING);
}
```

**Source:** `code/code/misc/movement.cc:3390-3422`

### doLand() - Landing

```cpp
void TBeing::doLand() {
    if (!isFlying()) {
        sendTo("It seems you have already landed.\n\r");
        return;
    }
    if (roomp->isFlyingSector()) {
        sendTo("The magic in the air prevents you from landing.\n\r");
        return;
    }
    // Must descend first in air/vertical sectors
    if ((roomp->isAirSector() || roomp->isVertSector()) &&
        exit_ok(roomp->exitDir(DIR_DOWN), NULL)) {
        sendTo("You have to descend before you can land.\n\r");
        return;
    }

    act("You come in for a landing and stop flying.", TRUE, this, 0, 0, TO_CHAR);
    setPosition(POSITION_STANDING);
}
```

**Source:** `code/code/misc/movement.cc:3424-3453`

### Flying Sector Mechanics

When entering a `SECT_MAKE_FLY` sector, characters automatically begin flying:

```cpp
if (to_here->isFlyingSector()) {
    if (!isFlying()) {
        sendTo("Without effort, you start to fly around.\n\r");
        setPosition(POSITION_FLYING);
    }
}
```

When leaving a flying sector, characters must have actual flight ability or fall:

```cpp
if (from_here->isFlyingSector() && !to_here->isFlyingSector()) {
    if (to_here->isAirSector() || to_here->isVertSector()) {
        if (!affectedBySpell(SPELL_FLY) && !isAffected(AFF_FLYING)) {
            sendTo("You would need to control your flying to go there.\n\r");
            return FALSE;
        }
    } else if (getPosition() == POSITION_FLYING) {
        if (!affectedBySpell(SPELL_FLY) && !isAffected(AFF_FLYING)) {
            setPosition(POSITION_STANDING);
            sendTo("The magic leaves you and you are set on your feet.\n\r");
        }
    }
}
```

**Source:** `code/code/misc/movement.cc:839-878`

## Climbing and Falling System

### canClimb() - Skill Check

```cpp
int TBeing::canClimb(TRoom* room, dirTypeT dir) {
    // Base skill check
    int skill = getSkillValue(SKILL_CLIMB);
    if (skill <= 0)
        return FALSE;

    // Calculate modifiers
    int mod = 0;

    // Agility modifier (±10 based on AGI)
    mod += getAgiReaction() / 2;

    // Weight/volume penalty (heavier = harder)
    float ratio = (float)getTotalWeight() / (float)carryWeightLimit();
    if (ratio > 0.75)
        mod -= 20;
    else if (ratio > 0.5)
        mod -= 10;

    // Combat penalty
    if (fight())
        mod -= 20;

    // Injury penalties
    if (isLimbFlags(WEAR_ARM_R, PART_BROKEN | PART_USELESS))
        mod -= 15;
    if (isLimbFlags(WEAR_ARM_L, PART_BROKEN | PART_USELESS))
        mod -= 15;
    if (isLimbFlags(WEAR_LEG_R, PART_BROKEN | PART_USELESS))
        mod -= 10;
    if (isLimbFlags(WEAR_LEG_L, PART_BROKEN | PART_USELESS))
        mod -= 10;

    // Final check
    return bSuccess(skill, SKILL_CLIMB, mod);
}
```

**Source:** `code/code/misc/physics.cc`

### Climbing Modifiers

| Condition | Modifier | Effect |
|-----------|----------|--------|
| High agility | +5 to +10 | Easier climbing |
| Low agility | -5 to -10 | Harder climbing |
| Weight > 75% capacity | -20 | Heavy load penalty |
| Weight > 50% capacity | -10 | Moderate load penalty |
| Fighting | -20 | Combat distraction |
| Broken right arm | -15 | Can't grip properly |
| Broken left arm | -15 | Can't grip properly |
| Broken right leg | -10 | Can't push off |
| Broken left leg | -10 | Can't push off |

### checkFalling() - Main Loop

```cpp
int TBeing::checkFalling(int count) {
    TRoom* rp = roomp;
    dirTypeT fall_dir = DIR_DOWN;
    int rc;

    // Immortals bounce
    if (isImmortal() && GetMaxLevel() >= GOD_LEVEL1) {
        sendTo("You bounce!\n\r");
        return FALSE;
    }

    // Flying characters don't fall
    if (isFlying() || (riding && riding->isFlying()))
        return FALSE;

    // Calculate fall distance thresholds
    int num1 = hasSkill(SKILL_CATFALL) ||
               affectedBySpell(SPELL_FEATHERY_DESCENT) ? 10 : 5;
    int num2 = num1 + 5;  // Damage zone

    // Find room below
    if (!rp || !rp->dir_option[fall_dir] ||
        rp->dir_option[fall_dir]->to_room == Room::NOWHERE)
        return FALSE;

    TRoom* nrp = real_roomp(rp->dir_option[fall_dir]->to_room);
    if (!nrp)
        return FALSE;

    // Flying sector auto-catch
    if (nrp->isAirSector()) {
        if (!isFlying()) {
            sendTo("You begin to fly!\n\r");
            setFlying(TRUE);
        }
        return FALSE;
    }

    // Move to lower room
    --(*this);
    *nrp += *this;

    // Check if landed
    if (!nrp->isAirSector()) {
        // Lethal fall threshold
        if (count >= num1) {
            rc = fallKill(count);  // May return DELETE_THIS
            if (IS_SET_DELETE(rc, DELETE_THIS))
                return DELETE_THIS;
        }

        // Damage zone
        if (count >= num2) {
            int dam = count * number(40, 80);

            // Skills reduce damage by 50%
            if (hasSkill(SKILL_CATFALL) ||
                affectedBySpell(SPELL_FEATHERY_DESCENT))
                dam /= 2;

            // Water sectors reduce damage by 50%
            if (nrp->isWaterSector())
                dam /= 2;

            if (reconcileDamage(this, dam, DAMAGE_FALL) == -1)
                return DELETE_THIS;
        }

        // Crash landing
        rc = crashLanding(POSITION_SITTING);
        if (IS_SET_DELETE(rc, DELETE_THIS))
            return DELETE_THIS;

        return FALSE;
    }

    // Still falling - recurse
    return checkFalling(count + 1);
}
```

**Source:** `code/code/misc/physics.cc`

### Fall Distance Thresholds

| Distance | With Skills | Without Skills | Effect |
|----------|-------------|----------------|--------|
| 0-4 rooms | No damage | No damage | Safe fall |
| 5-9 rooms | No damage | Damage zone | Skills protect |
| 10+ rooms | Damage zone | Lethal threshold | Maximum danger |
| 20+ rooms | Instant death | Instant death | No damage calculation |

### Fall Damage Calculation

```cpp
int dam = count * number(40, 80);  // 40-80 damage per room

// Modifiers:
// - SKILL_CATFALL: dam /= 2 (50% reduction)
// - SPELL_FEATHERY_DESCENT: dam /= 2 (50% reduction)
// - Water sector landing: dam /= 2 (50% reduction)
// - Skills + water: dam /= 4 (75% reduction total)
```

**Example:**
- 10-room fall without skills: 10 * 40-80 = 400-800 damage
- 10-room fall with catfall: 10 * 20-40 = 200-400 damage
- 10-room fall with catfall into water: 10 * 10-20 = 100-200 damage

### crashLanding() Function

```cpp
int TBeing::crashLanding(positionTypeT position, bool force) {
    int rc;

    // Force landing (immediate, no checks)
    if (force) {
        setPosition(position);
        updatePos();

        if (riding) {
            rc = fallOffMount(riding, position);
            if (IS_SET_DELETE(rc, DELETE_THIS))
                return DELETE_THIS;
        }
        return FALSE;
    }

    // Normal landing (may resist based on position/skills)
    if (getPosition() >= POSITION_STANDING) {
        // Ground fighting skill reduces chance
        if (doesKnowSkill(SKILL_GROUNDFIGHTING)) {
            int skill = getSkillValue(SKILL_GROUNDFIGHTING);
            if (bSuccess(skill, SKILL_GROUNDFIGHTING))
                return FALSE;  // Stayed standing
        }

        // Position check
        if (bSuccess(getPosition(), position))
            return FALSE;  // Stayed standing
    }

    // Fall to new position
    setPosition(position);
    updatePos();

    // Handle mount
    if (riding) {
        rc = fallOffMount(riding, position);
        if (IS_SET_DELETE(rc, DELETE_THIS))
            return DELETE_THIS;
    }

    // Flying landing (special case)
    if (position == POSITION_FLYING) {
        if (isFlying())
            return FALSE;

        // Start falling if not flying
        act("$n begins to plummet!", TRUE, this, 0, 0, TO_ROOM);
        act("You begin to plummet!", FALSE, this, 0, 0, TO_CHAR);

        rc = checkFalling();
        if (IS_SET_DELETE(rc, DELETE_THIS))
            return DELETE_THIS;
    }

    return FALSE;
}
```

**Source:** `code/code/misc/movement.cc`

### CRITICAL: Multiple DELETE_THIS Return Points

```cpp
// CORRECT: Check all three branches
rc = crashLanding(POSITION_SITTING);
if (IS_SET_DELETE(rc, DELETE_THIS))
    return DELETE_THIS;

// WRONG: Assuming crashLanding never returns DELETE_THIS
crashLanding(POSITION_SITTING);
// Character may be deleted here!
```

## Swimming and Drowning System

### Water Sector Types

```cpp
// Underwater (requires AFF_WATERBREATH)
bool sectorTypeT::isUnderwaterSector() const {
    return (*this == SECT_TEMPERATE_UNDERWATER ||
            *this == SECT_TROPICAL_UNDERWATER);
}

// Any water (includes surface)
bool sectorTypeT::isWaterSector() const {
    return (isUnderwaterSector() ||
            *this == SECT_TEMPERATE_OCEAN_SURFACE ||
            *this == SECT_TROPICAL_OCEAN_SURFACE ||
            *this == SECT_TEMPERATE_RIVER_SURFACE ||
            *this == SECT_TROPICAL_RIVER_SURFACE);
}
```

### Underwater Sector Characteristics

- Requires `AFF_WATERBREATH` or racial water breathing to survive
- Movement costs doubled without water breathing
- Speech is garbled/impossible
- Certain spells and skills cannot be used
- Characters take drowning damage every 3.6 seconds without protection

### procCharDrowning - Scheduler Process

The `procCharDrowning` scheduler process runs every `Pulse::DROWNING` (36 ticks = 3.6 seconds):

```cpp
bool procCharDrowning::run(const TPulse& pulse, TBeing* ch) const {
    int rc = ch->checkDrowning();
    if (IS_SET_DELETE(rc, DELETE_THIS))
        return true;  // Signal scheduler to delete
    return false;
}
```

**Source:** `code/code/sys/socket.cc`

### checkDrowning() - Core Implementation

```cpp
int TBeing::checkDrowning() {
    // Only affects PCs in underwater sectors
    if (!isPc() || !roomp || !roomp->isUnderwaterSector())
        return FALSE;

    // Water breathing immunity
    if (isAffected(AFF_WATERBREATH))
        return FALSE;

    // Apply drowning damage
    int dam = number(20, 40);
    if (reconcileDamage(this, dam, DAMAGE_DROWN) == -1)
        return DELETE_THIS;  // Character died

    // Suffocation messages
    if (getHit() < (getMaxHit() / 4)) {
        act("You are drowning!", FALSE, this, NULL, NULL, TO_CHAR);
        act("$n is drowning!", TRUE, this, NULL, NULL, TO_ROOM);
    }

    return FALSE;
}
```

**Source:** `code/code/misc/being.cc`

### CRITICAL: reconcileDamage() Returns -1, Not DELETE Flag

```cpp
// CORRECT: Check for -1 sentinel value
if (reconcileDamage(this, dam, DAMAGE_DROWN) == -1)
    return DELETE_THIS;

// WRONG: IS_SET_DELETE won't detect -1
if (IS_SET_DELETE(rc, DELETE_VICT)) { ... }  // Never triggers!
```

The `-1` return value is a special sentinel indicating death. It is NOT a DELETE flag and cannot be checked with `IS_SET_DELETE()`. Always check with `== -1`.

### Water Breathing Systems

#### Racial Water Breathing

Some races have innate water breathing:

```cpp
bool Race::hasNaturalWaterBreath() const {
    return (race_bits & RACE_WATERBREATH);
}
```

**Races with natural water breathing:**
- Merman/Mermaid
- Fishman
- Sea Elf

#### Magical Water Breathing

The `AFF_WATERBREATH` flag provides temporary water breathing:

**Sources:**
- `SPELL_WATERBREATH` (Shaman spell)
- `SPELL_AQUALUNG` (Mage spell)
- Equipment with waterbreath enchantment

### Swimming Mechanics

Swimming success is determined by comparing character density to water density:

```cpp
bool TBeing::isSwimming() const {
    if (!roomp || !roomp->isWaterSector())
        return false;

    // Calculate character density
    float charDensity = getTotalWeight() / getTotalVolume();
    const float WATER_DENSITY = 1.0;

    return (charDensity > WATER_DENSITY);
}
```

**Key factors:**
- **Weight**: Total carried weight + body weight
- **Volume**: Total carried volume + body volume
- **Density**: weight/volume ratio

**If density > 1.0:** Character sinks and must swim
**If density ≤ 1.0:** Character floats naturally

### River Flow System

#### procCharRiverFlow - Scheduler Process

Characters in river sectors are moved by current:

```cpp
bool procCharRiverFlow::run(const TPulse& pulse, TBeing* ch) const {
    if (!ch->roomp || !ch->roomp->isRiverSector())
        return false;

    int rc = ch->riverFlow();
    if (IS_SET_DELETE(rc, DELETE_THIS))
        return true;
    return false;
}
```

#### riverFlow() - Current Movement

```cpp
int TBeing::riverFlow() {
    if (!roomp || roomp->riverSpeed <= 0)
        return FALSE;

    dirTypeT dir = roomp->riverDir;
    int speed = roomp->riverSpeed;

    // Sitting characters flow faster
    if (getPosition() == POSITION_SITTING)
        speed *= 2;

    // Check if we should flow this pulse
    if (number(0, 100) > speed)
        return FALSE;

    // Attempt to swim against current
    if (bSuccess(getSkillValue(SKILL_SWIMMING), SKILL_SWIMMING)) {
        sendTo("You swim against the current.\n\r");
        return FALSE;
    }

    // Flow with current
    act("The current sweeps you $T!", FALSE, this, NULL, dirs[dir], TO_CHAR);
    act("The current sweeps $n $T!", TRUE, this, NULL, dirs[dir], TO_ROOM);

    return doMove(dir);  // May return DELETE_THIS
}
```

**Key mechanics:**
- Flow chance = river speed (0-100%)
- Sitting doubles flow chance
- Swimming skill can resist current
- Movement may kill character (traps, death rooms, etc.)

### CRITICAL: Iterator Safety in River Flow

When moving multiple riders/contents, the iterator must be cached BEFORE modifications:

```cpp
// CORRECT: Cache next pointer before modification
for (TThing* t = rider; t; t = t2) {
    t2 = t->nextRider;  // Cache BEFORE any operations
    --(*t);             // Remove from current location
    *new_room += *t;    // Add to new location
}

// WRONG: Iterator invalidated
for (TThing* t = rider; t; t = t->nextRider) {
    --(*t);  // Invalidates t->nextRider!
    *new_room += *t;
}
```

**Why this matters:** The `--(*t)` operation clears `t->nextRider`, making the loop advance fail. Caching the pointer before modification prevents use-after-free.

## Portal and Teleportation System

### Transport Methods

| Method | Class/Skill | Destination | Restrictions |
|--------|-------------|-------------|--------------|
| **Portal spell** | Cleric (Hand of God) | Fixed portal rooms | Bidirectional, charges |
| **Dimensional Fold** | Psionic skill | Player or home | Bidirectional, single use |
| **Teleport** | Mage (Sorcery) | Random | Dangerous, can target others |
| **Word of Recall** | Cleric | Hometown | Faith-dependent |
| **Astral Walk** | Cleric (Hand of God) | Target creature | Can fail catastrophically |
| **Summon** | Cleric | Caster's location | Brings target to caster |
| **Portal objects** | Builder-placed | Fixed destination | Charges, traps, locks |

### TPortal Class

**Source:** `code/code/obj/obj_portal.h`, `code/code/obj/obj_portal.cc`

Portals are objects that allow instant travel to a destination room. They inherit from `TSeeThru` (allowing players to look through them) and implement `ITEM_PORTAL` type.

#### Class Definition

```cpp
class TPortal : public TSeeThru {
  private:
    char charges;                  // Number of uses remaining (-1 = infinite)
    unsigned char portal_type;     // Entry/exit message style (0-13)
    unsigned char trap_type;       // doorTrapT if trapped
    unsigned short trap_damage;    // Trap damage amount
    unsigned short portal_state;   // EXIT_* flags (closed, locked, etc.)
    int portal_key;                // Key vnum for locked portals

  public:
    virtual itemTypeT itemType() const { return ITEM_PORTAL; }
    virtual int enterMe(TBeing*);  // Returns DELETE_THIS, DELETE_VICT
    virtual int objectDecay();     // Returns DELETE_THIS on decay
    TPortal* findMatchingPortal() const;  // Finds paired portal at destination
};
```

#### Portal Entry Flow

**Source:** `code/code/obj/obj_portal.cc:263-406`

```cpp
// returns DELETE_THIS, DELETE_VICT(ch)
int TPortal::enterMe(TBeing* ch) {
    TRoom* rp;

    // 1. Check portal state
    if (isPortalFlag(EXIT_CLOSED)) {
        ch->sendTo("You can't enter that! It's closed!\n\r");
        return FALSE;
    }
    if (isPortalFlag(EXIT_NOENTER)) {
        ch->sendTo("You can't seem to find a way to enter that.\n\r");
        return FALSE;
    }

    // 2. Combat restriction
    if (ch->isCombatMode(ATTACK_BERSERK) && ch->fight()) {
        ch->sendTo("You are too overwhelmed with rage to leave!\n\r");
        return FALSE;
    }

    // 3. Validate destination
    if (!(rp = real_roomp(getTarget()))) {
        ch->sendTo("The sheer terror of that chaos prevents you...\n\r");
        return FALSE;
    }

    // 4. Mob limit check
    if (rp->getMoblim() && MobCountInRoom(rp->stuff) >= rp->getMoblim()) {
        act("An invisible wall bars your entry.", FALSE, ch, this, NULL, TO_CHAR);
        return FALSE;
    }

    // 5. Handle trap
    if (isPortalFlag(EXIT_TRAPPED)) {
        int rc = ch->triggerPortalTrap(this);
        if (IS_SET_DELETE(rc, DELETE_ITEM | DELETE_THIS))
            return DELETE_THIS | DELETE_VICT;
        if (IS_SET_DELETE(rc, DELETE_THIS))
            return DELETE_VICT;
        if (IS_SET_DELETE(rc, DELETE_ITEM))
            return FALSE;
        if (!sameRoom(*ch))  // Teleported elsewhere by trap
            return FALSE;
    }

    // 6. Transfer character
    --(*ch);
    thing_to_room(ch, getTarget());
    ch->doLook("", CMD_LOOK);

    // 7. Handle followers (two passes: mounts first, then others)
    // ... follower handling code ...

    // 8. Consume charges on both ends
    TPortal* otherport = findMatchingPortal();
    if (otherport && otherport->getPortalNumCharges() >= 1) {
        if (otherport->getPortalNumCharges() == 1)
            delete otherport;  // Last use
        else
            otherport->setPortalNumCharges(charges - 1);
    }

    if (getPortalNumCharges() >= 1) {
        if (getPortalNumCharges() == 1)
            return DELETE_THIS;  // Last use
        else
            setPortalNumCharges(charges - 1);
    }

    return FALSE;
}
```

**CRITICAL:** The caller must check `IS_SET_DELETE()` on the return value and handle `DELETE_THIS` (portal consumed) and `DELETE_VICT` (character died) appropriately.

### Portal Spell (Cleric)

**Source:** `code/code/disc/disc_cleric_hand_of_god.cc:888-979`

The portal spell creates a bidirectional portal between the caster's location and a designated portal room. Only specific portal rooms can be targeted.

#### Portal Room Destinations

```cpp
portalRoomT portalRooms[] = {
    {15346, "grimhaven"},
    {15347, "brightmoon"},
    {15348, "logrus"},
    {33760, "amber"},
};
```

Players use: `pray portal <destination_name>`

### Teleport Spell (Mage)

**Source:** `code/code/disc/disc_mage_sorcery.cc:1432-1528`

The teleport spell transports the caster or a target to a random location. This is a high-risk spell with no control over the destination.

#### Generic Teleport Function

**Source:** `code/code/misc/magicutils.cc:1088-1168`

The `genericTeleport()` function selects a random valid room and moves the target:

```cpp
int TThing::genericTeleport(silentTypeT silent, bool keepZone, bool unsafe) {
    int to_room;
    TRoom* rp;

    for (;;) {
        // Random room selection (100 to top_of_world)
        if (keepZone) {
            int minroom = zone_table[roomp->getZoneNum() - 1].top + 1;
            int maxroom = zone_table[roomp->getZoneNum()].top;
            to_room = ::number(minroom, maxroom);
        } else {
            to_room = ::number(100, top_of_world);
        }

        if (!(rp = real_roomp(to_room)))
            continue;
        if (!zone_table[rp->getZoneNum()].enabled)
            continue;

        // Safety checks (unless unsafe mode)
        if (!unsafe) {
            if (rp->isRoomFlag(ROOM_PRIVATE))
                continue;
            if (rp->isRoomFlag(ROOM_HAVE_TO_WALK))
                continue;
            if (rp->isRoomFlag(ROOM_DEATH))
                continue;
            if (rp->isFlyingSector())
                continue;
        }
        break;
    }

    if (!silent) {
        act("$n shimmers out of existence.", FALSE, this, NULL, NULL, TO_ROOM);
        act("You shimmer out of existence.", FALSE, this, NULL, NULL, TO_CHAR);
    }

    // Dismount handling
    while (rider) { ... }
    if (riding) { ... }

    --(*this);
    *rp += *this;

    if (!silent) {
        act("$n shimmers into existence.", FALSE, this, NULL, NULL, TO_ROOM);
        TBeing* tbt = dynamic_cast<TBeing*>(this);
        if (tbt) {
            tbt->doLook("", CMD_LOOK);
            int rc = tbt->genericMovedIntoRoom(rp, -1);
            if (IS_SET_DELETE(rc, DELETE_THIS))
                return DELETE_THIS;
        }
    }
    return FALSE;
}
```

#### Teleport Safety Exclusions

| Room Flag | Excluded | Reason |
|-----------|----------|--------|
| `ROOM_PRIVATE` | Yes | Restricted access rooms |
| `ROOM_HAVE_TO_WALK` | Yes | Must walk to enter |
| `ROOM_DEATH` | Yes | Instant death rooms |
| Flying sector | Yes | Requires flight to survive |
| Disabled zone | Yes | Zone not active |

### Word of Recall (Cleric)

**Source:** `code/code/disc/disc_cleric_hand_of_god.cc:320-480`

Word of Recall transports the target to their hometown. Success depends on the cleric's faction standing with their deity.

**Restrictions:**
- Cannot recall from `ROOM_ARENA` or `ROOM_NO_ESCAPE`
- Blocked if caster or victim has `AFFECT_PLAYERKILL`
- Failure can result in random teleport or being sent to Hell

### Astral Walk (Cleric)

**Source:** `code/code/disc/disc_cleric_hand_of_god.cc:26-166`

Astral Walk transports the caster to a target creature's location. Unlike summon, the caster moves rather than the target.

**Restrictions:**
- Cannot target immortals
- Cannot use from or to flying sectors
- Cannot use from `ROOM_NO_ESCAPE`
- Cannot target `ROOM_PRIVATE`, `ROOM_HAVE_TO_WALK`, or `ROOM_NO_MAGIC` rooms
- Quest mob protection (Tiger Shark, Elephant)

### Summon (Cleric)

**Source:** `code/code/disc/disc_cleric_hand_of_god.cc:482-742`

Summon pulls a target to the caster's location. Has extensive restrictions to prevent abuse.

**Restrictions:**
- Cannot summon self
- Cannot summon immortals
- Cannot summon from `ROOM_NO_ESCAPE` or `ROOM_ARENA`
- Cannot summon to `ROOM_HAVE_TO_WALK` or fall sectors
- Quest mob protection
- Immune to summon check

### Room Teleportation Restrictions

| Flag | Effect on Teleport | Effect on Portal |
|------|-------------------|------------------|
| `ROOM_NO_ESCAPE` | Cannot teleport/recall FROM | Cannot astral walk FROM |
| `ROOM_NO_PORTAL` | - | Cannot cast portal spell |
| `ROOM_NO_MAGIC` | - | Cannot astral walk TO |
| `ROOM_PRIVATE` | Cannot teleport TO | Cannot astral walk TO |
| `ROOM_HAVE_TO_WALK` | Cannot teleport TO | Cannot summon TO |
| `ROOM_ARENA` | Cannot recall FROM | Cannot summon FROM |
| `ROOM_DEATH` | Excluded from random teleport | - |
| Disabled zone | Cannot teleport TO | Cannot portal TO |
| Flying sector | Excluded from random teleport | Portal handles flight state |

## DELETE Safety Patterns

### Pattern 1: Immediate Propagation

```cpp
// In movement/falling/drowning functions
rc = crashLanding(POSITION_SITTING);
if (IS_SET_DELETE(rc, DELETE_THIS))
    return DELETE_THIS;  // Propagate immediately

// NEVER continue after DELETE_THIS
sendTo("You land safely.\n\r");  // Only reached if alive
```

### Pattern 2: reconcileDamage() -1 Check

```cpp
// CORRECT: Check for -1 sentinel value
if (reconcileDamage(this, dam, DAMAGE_FALL) == -1)
    return DELETE_THIS;

// WRONG: IS_SET_DELETE won't detect -1
if (IS_SET_DELETE(rc, DELETE_VICT)) { ... }  // Never triggers!
```

The `-1` return value is a special sentinel indicating death. It is NOT a DELETE flag and cannot be checked with `IS_SET_DELETE()`.

### Pattern 3: Mount Falling Chain

```cpp
// In crashLanding()
if (riding) {
    rc = fallOffMount(riding, position);
    if (IS_SET_DELETE(rc, DELETE_THIS))
        return DELETE_THIS;  // Rider died from fall
}

// In fallOffMount()
rc = crashLanding(POSITION_SITTING);
if (IS_SET_DELETE(rc, DELETE_THIS))
    return DELETE_THIS;  // Recursive crash landing
```

Mount falling creates a chain of DELETE_THIS returns that must be propagated at each level.

### Pattern 4: Iterator Caching

```cpp
// ALWAYS cache next pointer before modifications
for (TThing* t = list; t; t = cached_next) {
    cached_next = t->next;  // Cache FIRST
    // Now safe to modify t, even delete it
}
```

### Pattern 5: Adapter Pattern Conversion

Scheduler processes convert DELETE flags to bool:

```cpp
bool procCharDrowning::run(const TPulse& pulse, TBeing* ch) const {
    int rc = ch->checkDrowning();       // Game layer: returns DELETE_* flags
    if (IS_SET_DELETE(rc, DELETE_THIS)) // Check the flag
        return true;                    // Scheduler layer: signal deletion
    return false;                       // Scheduler layer: keep character
}
```

## Common Gotchas

### 1. Always Check DELETE_THIS from Movement Functions

```cpp
// CORRECT: Check return value
int rc = doMove(dir);
if (IS_SET_DELETE(rc, DELETE_THIS))
    return DELETE_THIS;

// WRONG: Ignoring potential death
doMove(dir);  // Character may be dead!
sendTo("You arrived safely.\n\r");  // Crash
```

### 2. Door Operations Modify Both Sides

Opening/closing/locking a door affects both the current room's exit AND the corresponding exit in the destination room:

```cpp
REMOVE_BIT(exitp->condition, EXIT_CLOSED);
// Also open the other side
if (back = rp->dir_option[rev_dir(door)])
    REMOVE_BIT(back->condition, EXIT_CLOSED);
```

### 3. Portcullis vs Drawbridge Logic is Inverted

```cpp
// Portcullis: raise = open, lower = close
// Drawbridge: lower = open, raise = close
```

### 4. Flying Sector Entry is Automatic

Characters entering `SECT_MAKE_FLY` sectors automatically get `POSITION_FLYING` set, but leaving requires actual flight ability or they cannot proceed.

### 5. Movement While Mounted Uses Mount's Movement Points

```cpp
if (riding) {
    tbr->addToMove(-need_movement);  // Mount pays
    addToMove(-::number(0, need_movement) / 3);  // Rider pays partial
}
```

### 6. Not Checking crashLanding() Return

```cpp
// CRASH: Continuing after potential death
crashLanding(POSITION_SITTING);
sendTo("You feel pain!\n\r");  // Character may be dead!

// CORRECT: Check immediately
rc = crashLanding(POSITION_SITTING);
if (IS_SET_DELETE(rc, DELETE_THIS))
    return DELETE_THIS;
sendTo("You feel pain!\n\r");  // Safe
```

### 7. Wrong Death Check from reconcileDamage()

```cpp
// CRASH: Using IS_SET_DELETE for reconcileDamage
int rc = reconcileDamage(this, dam, DAMAGE_FALL);
if (IS_SET_DELETE(rc, DELETE_VICT)) {  // Never triggers!
    return DELETE_THIS;
}

// CORRECT: Check for -1
if (reconcileDamage(this, dam, DAMAGE_FALL) == -1)
    return DELETE_THIS;
```

### 8. Not Checking DELETE After Teleport

```cpp
// CRASH: Not checking if character died
rc = victim->genericTeleport(SILENT_NO);
victim->sendTo("You arrived safely!\n\r");  // victim may be dead!

// CORRECT: Check DELETE flag
rc = victim->genericTeleport(SILENT_NO);
if (IS_SET_DELETE(rc, DELETE_THIS))
    return SPELL_SUCCESS + VICTIM_DEAD;
victim->sendTo("You arrived safely!\n\r");
```

### 9. Dereferencing Room After Random Teleport

```cpp
// BUG: Room may be different after teleport
TRoom* orig_room = caster->roomp;
caster->genericTeleport(SILENT_NO);
orig_room->sendTo("Someone just teleported!\n\r");  // Sending to wrong room

// CORRECT: Use room before teleport
act("$n shimmers away.", FALSE, caster, NULL, NULL, TO_ROOM);
caster->genericTeleport(SILENT_NO);
```

### 10. Iterator Invalidation in Group Falls/Flow

```cpp
// CRASH: Modifying rider chain during iteration
for (TThing* t = rider; t; t = t->nextRider) {
    rdr->checkFalling();  // May modify t->nextRider
}

// CORRECT: Cache next pointer
for (TThing* t = rider; t; t = cached) {
    cached = t->nextRider;  // Cache before modification
    rdr->checkFalling();
}
```

## Key Source Files

| File | Contents |
|------|----------|
| `code/code/misc/movement.cc` | doMove, validMove, rawMove, doors, flying, crashLanding |
| `code/code/misc/physics.cc` | canClimb, checkFalling, fallKill |
| `code/code/misc/being.cc` | checkDrowning, riverFlow |
| `code/code/sys/socket.cc` | procCharFalling, procCharDrowning adapters |
| `code/code/obj/obj_portal.cc` | TPortal class, enterMe |
| `code/code/disc/disc_cleric_hand_of_god.cc` | Portal, Word of Recall, Astral Walk, Summon |
| `code/code/disc/disc_mage_sorcery.cc` | Teleport spell |
| `code/code/misc/magicutils.cc` | genericTeleport, rawSummon |

## Related Documentation

- [DELETE Flag System](delete-flags.md) - Complete DELETE_* flag documentation
- [Damage Pipeline](damage-pipeline.md) - reconcileDamage() and the -1 return value
- [Combat Rounds](combat-rounds.md) - Combat integration with movement
- [Room Environment](room-environment.md) - Room flags, sector types, exit system
- [Position Stance](position-stance.md) - Position effects on movement
