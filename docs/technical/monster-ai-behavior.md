---
title: Monster AI and Behavior
description: Complete NPC/monster AI system including opinion mechanics, aggression targeting, pursuit/tracking, charList memory management, and DELETE flag safety patterns
keywords: [opinionData, charList, Hates, Fears, addHated, ACT_AGGRESSIVE, ACT_HUNTING, aggroCheck, hunt, setHunting, doTrack, DELETE_THIS, mobileActivity, Mobile_Attitude, race targeting, alignment targeting, pathfinding, addFeared, remHated, remFeared, clist deletion]
category: Important Systems

last_updated: 2026-01-29
source_files: [code/code/misc/monster.cc, code/code/misc/mobact.cc, code/code/misc/opinion.cc, code/code/misc/ai_utility.cc]
related:
  - combat-rounds.md
  - memory-safety.md
  - movement-terrain-navigation.md
  - character-foundation.md
  - class-hierarchy.md
---

# Monster AI and Behavior

The monster AI system gives NPCs (TMonster) autonomous behavior: aggression, hunting, fear reactions, scavenging, combat tactics, and responses to player actions. This document covers the core subsystems and their memory management requirements.

**Misusing this system causes memory leaks and crashes.** Common errors: not deleting `charList` chains in loops, forgetting to check DELETE_* flags from AI functions, accessing stale `target` pointers after combat.

## Core Data Structures

### Mobile_Attitude (Personality System)

The `Mobile_Attitude` class models a mob's emotional state through four values (0-100 range):

```cpp
class Mobile_Attitude {
  private:
    unsigned short suspicion;      // Current suspicion value
    unsigned short greed;          // Current greed value
    unsigned short malice;         // Current malice value
    unsigned short anger;          // Current anger value

    unsigned short def_suspicion;  // Default (base) suspicion
    unsigned short def_greed;      // Default greed
    unsigned short def_malice;     // Default malice
    unsigned short def_anger;      // Default anger

    TBeing* target;   // Current opinion target (PC only!)
  public:
    TBeing* random;   // Scratch pointer for random interactions
    int last_cmd;     // Last command witnessed
};
```

**Source:** `code/code/misc/monster.h:56-79`

| Attribute | Effect on Behavior |
|-----------|-------------------|
| `suspicion` | Triggers investigation, affects steal detection |
| `greed` | Controls scavenging, stealing from players |
| `malice` | Intent to cause harm; combined with anger triggers aggro |
| `anger` | Emotional state; combined with malice triggers aggro |

**Accessor functions on TMonster:**
```cpp
int greed() const;        void setGreed(int);
int anger() const;        void setAnger(int);
int malice() const;       void setMalice(int);
int susp() const;         void setSusp(int);

// Randomized threshold checks
bool isGreedy() const;    // ::number(0,101) < greed()
bool isAngry() const;     // ::number(0,101) < anger()
bool isMalice() const;    // ::number(0,101) < malice()
bool isSusp() const;      // ::number(0,101) < susp()

// Modify values with randomization
void US(int num);  // Increase suspicion by random amount up to 2*num
void DS(int num);  // Decrease suspicion by random amount up to 2*num
void UG(int num);  // Increase greed
void DG(int num);  // Decrease greed
void UA(int num);  // Increase anger
void DA(int num);  // Decrease anger
void UM(int num);  // Increase malice
void DMal(int num); // Decrease malice
```

### opinionData (Hate/Fear Categories)

The `opinionData` class tracks who/what a mob hates or fears:

```cpp
class opinionData {
  public:
    charList* clist;    // Linked list of specific characters (CRITICAL: see memory management)
    sexTypeT sex;       // Hated/feared sex
    race_t race;        // Hated/feared race
    short Class;        // Hated/feared class bitmask
    short vnum;         // Hated/feared mob vnum
};
```

**Source:** `code/code/misc/monster.h:42-54`

TMonster has two instances: `hates` and `fears`, plus corresponding bitfields `hatefield` and `fearfield`.

### charList (Individual Character Tracking)

The `charList` class forms a singly-linked list of specific characters the mob hates or fears:

```cpp
class charList {
  public:
    const char* name;       // Character name (mud_str_dup'd, must delete[])
    long iHateStrength;     // Duration of hatred in game hours
    int account_id;         // For multi-character detection
    int player_id;          // Player ID

    charList* next;         // Next in linked list
};
```

**Source:** `code/code/misc/monster.h:27-40`

## CRITICAL: charList Memory Management

**The `charList` destructor does NOT delete the chain.** You MUST manually iterate and delete each node:

```cpp
// CORRECT: Chain deletion in TMonster destructor (monster.cc:270-280)
charList *k2, *n2;
for (k2 = hates.clist; k2; k2 = n2) {
    n2 = k2->next;
    delete k2;
}
hates.clist = NULL;

for (k2 = fears.clist; k2; k2 = n2) {
    n2 = k2->next;
    delete k2;
}
fears.clist = NULL;
```

```cpp
// WRONG: Memory leak - only deletes first node
delete hates.clist;

// WRONG: Use-after-free - accessing freed memory
for (charList* k = hates.clist; k; k = k->next) {
    delete k;  // k->next is now garbage!
}
```

The `opinionData` destructor (monster.cc:97-101) only deletes `clist` directly, which is why the warning comment exists:
```cpp
// warning: you must remember to manually delete opinionData::next in a loop
opinionData::~opinionData() {
    delete clist;
    clist = NULL;
}
```

## Hate/Fear Bitfield Flags

Defined in `code/code/misc/defs.h:32-46`:

| Hate Flag | Bit | Fear Flag | Meaning |
|-----------|-----|-----------|---------|
| `HATE_SEX` | 1<<0 | `FEAR_SEX` | Hate/fear specific sex |
| `HATE_RACE` | 1<<1 | `FEAR_RACE` | Hate/fear specific race |
| `HATE_CHAR` | 1<<2 | `FEAR_CHAR` | Hate/fear specific characters (uses clist) |
| `HATE_CLASS` | 1<<3 | `FEAR_CLASS` | Hate/fear specific class |
| `HATE_VNUM` | 1<<6 | `FEAR_VNUM` | Hate/fear specific mob vnum |

## Opinion System

### Hate System Functions

#### Adding Hatred

```cpp
// Add hatred of a specific character (adds to clist)
bool TMonster::addHated(TBeing* hatee);

// Add categorical hatred (race, sex, class, vnum)
int TMonster::addHatred(zoneHateT parm_type, int parm);
```

**zoneHateT values:** `OP_SEX`, `OP_RACE`, `OP_CLASS`, `OP_VNUM`

The hate strength calculation (opinion.cc:94-96):
```cpp
list->iHateStrength = (GetMaxLevel() + hatee->GetMaxLevel() + 5) *
                      (getStat(STAT_CURRENT, STAT_FOC) / 120.0);
// Result: 2-219 game hours depending on levels and focus
```

#### Checking Hatred

```cpp
bool TMonster::Hates(const TBeing* v, const char* n) const;
```

Returns true if mob hates the target based on:
1. Character name in `hates.clist` (if `HATE_CHAR` set)
2. Race match (if `HATE_RACE` set)
3. Sex match (if `HATE_SEX` set)
4. Class match (if `HATE_CLASS` set)
5. Vnum match for mobs (if `HATE_VNUM` set)

#### Removing Hatred

```cpp
int TMonster::remHated(const TBeing* hatee, const char* n);
int TMonster::remHatred(unsigned short bitv);  // Remove categorical hatred
```

#### Developing Hatred (Combat)

```cpp
void TMonster::developHatred(TBeing* v);
```

Called during combat (damage.cc). Decides whether to add permanent hatred based on:
- Current HP percentage (patience)
- Level difference (high-level mobs become hateful faster against low-level attackers)
- Random variance

**Source:** `code/code/misc/opinion.cc:530-564`

```cpp
void TMonster::developHatred(TBeing* v) {
    // Base difficulty
    int diff = 50;

    // Level difference modifiers
    int lev = GetMaxLevel() - v->GetMaxLevel();
    if (lev > 10) diff = 100;      // High-level mob vs low-level player
    else if (lev > 5) diff = 75;   // Moderate level gap

    // Patience = current HP percentage
    int patience = (100 * getHit() / hitLimit());

    // Random variance
    int var = ::number(-20, 20);

    if (patience + var < diff)
        addHated(v);
}
```

**Purpose:** Prevents single-hit-and-flee XP exploitation. High-level mobs attacked by low-level players develop hatred quickly.

### Fear System Functions

Mirror the hate system:

```cpp
bool TMonster::Fears(const TBeing* v, const char* s) const;
int TMonster::addFeared(TBeing* hatee);
int TMonster::addFears(zoneHateT parm_type, int parm);
int TMonster::remFeared(const TBeing* hatee, const char* n);
```

**Critical:** Adding a feared character clears hunting state (opinion.cc:378-382):
```cpp
if (hatee == specials.hunting) {
    persist = 0;
    specials.hunting = 0;
    hunt_dist = 0;
}
```

### Global Cleanup Functions

When characters delete themselves, all mobs must forget them:

```cpp
void DeleteHatreds(const TBeing* ch, const char* s);  // Remove from all hate lists
void DeleteFears(const TBeing* ch, const char* s);    // Remove from all fear lists
```

**Source:** `code/code/misc/opinion.cc:497-528`

## Aggression and Targeting

### pissed() vs aggro()

```cpp
// Basic anger check - used for minor annoyances
int TMonster::pissed() {
    if (UtilMobProc(this)) return FALSE;
    return isAngry() && isMalice();  // Random checks against both
}

// Full aggression check - used for combat initiation
int TMonster::aggro() {
    if (UtilMobProc(this)) return FALSE;
    if (GuildProcs(spec)) return FALSE;
    if (isPet(PETTYPE_PET | PETTYPE_CHARM | PETTYPE_THRALL)) return FALSE;

    // Threshold check: 4*anger + 5*malice >= 450
    if (isAngry() && isMalice()) {
        if ((4 * anger() + 5 * malice()) >= 450)
            return TRUE;
    }

    // ACT_AGGRESSIVE flag bypasses emotion checks
    if (IS_SET(specials.act, ACT_AGGRESSIVE))
        return TRUE;

    return FALSE;
}
```

**Source:** `code/code/misc/ai_utility.cc:28-60`

### aggroCheck() - Main Aggression Loop

```cpp
int TMonster::aggroCheck(bool mobpulse);  // Returns DELETE_THIS, DELETE_VICT
```

Called from `mobileActivity()`. Checks all characters in room for valid targets.

**Source:** `code/code/misc/mobact.cc:3967-4134`

**Key logic:**
1. First calls `factionAggroCheck()` for faction-based aggression
2. Skips if already fighting or polymorphed player
3. Wandered mobs (outside home zone) have reduced aggro against low-levels
4. Random target selection when multiple valid targets exist
5. Level difference checks prevent suicidal attacks
6. Wimpy mobs only attack sleeping targets

**The Karma Check:**

```cpp
if (((tmp_ch->plotStat(STAT_CURRENT, STAT_KAR, 0, 100, 50)) <=
      (plotStat(STAT_CURRENT, STAT_INT, 0, 200, 100) + anger())) ||
    IS_SET(specials.act, ACT_AGGRESSIVE))
```

Players with high Karma can avoid aggression from intelligent, calm mobs. The formula:
- Player Karma: 0-100 range (scaled from stat)
- Mob threshold: 0-200 (INT scaled) + anger (0-100)
- Attack if player karma <= mob threshold

### factionAggroCheck() - Territorial Faction Combat

**Source:** `code/code/misc/mobact.cc:3834-3965`

Handles Cult of Logrus vs Brotherhood of Galek territorial aggression.

```cpp
int TMonster::factionAggroCheck() {
    // Only applies to faction mobs in their home territory
    if ((!isCult() && !isBrother()) ||
        (isCult() && !inLogrus()) ||
        (isBrother() && !inBrightmoon()))
        return FALSE;

    // Attack opposing faction members on sight
    if ((isCult() && (tmp_ch->isBrother() || tmp_ch->isSnake())) ||
        (isBrother() && tmp_ch->isCult())) {
        // ... flavor text and attack
    }
}
```

**Snake faction special case:** Players with `isSnake()` faction in Logrus can carry item vnum 8879 (trade pass) to prevent Cult mob aggression.

### takeFirstHit() - Combat Initiation

**Source:** `code/code/misc/mobact.cc:3018-3140`

Called when a mob decides to attack. Handles class-specific opening moves.

| Class | Opening Move |
|-------|--------------|
| Thief | Backstab (level <35) or Throat Slit (level 35+) |
| Other | `classStuff()` for high-attack mobs, else direct `hit()` |

**Return value:** Returns DELETE_VICT if victim dies, DELETE_THIS if mob dies.

### doHatefulStuff() - Hateful Mob Behavior

**Source:** `code/code/misc/mobact.cc:5070-5156`

```cpp
int TMonster::doHatefulStuff() {
    if ((tmp_ch = findAHatee()) && canSee(tmp_ch)) {
        // Hatee in room - attack or confront
        if (checkPeaceful(...)) {
            // Can't fight in peaceful room
            act("$n growls '$N, would you care to step outside?'", ...);
        } else {
            // Attack with flavor text based on hate type
            rc = takeFirstHit(*tmp_ch);
        }
    } else if (!IS_SET(specials.act, ACT_HUNTING)) {
        // Hatee not in room - consider hunting
        if (health >= 50% && ::number(0,1)) {
            for (i = hates.clist; i; i = i->next) {
                tmp_ch = get_char(i->name, EXACT_YES);
                if (tmp_ch) {
                    setHunting(tmp_ch);
                    return true;
                }
            }
        }
    }
}
```

### senseWimps() - Targeting Priority

**Source:** `code/code/misc/mobact.cc:809-1000`

Smart mobs (level 15+) use `senseWimps()` to select optimal targets during combat.

**Target Scoring:**

```cpp
score = tmp_victim->getHit() + tmp_victim->hitLimit();
score += tmp_victim->getMana();
score += (2000 - tmp_victim->getArmor());
score += tmp_victim->plotStat(STAT_CURRENT, STAT_KAR, 0, 2000, 1000);
```

Lower score = more attractive target.

**Score Modifiers:**

| Condition | Modifier | Reason |
|-----------|----------|--------|
| Mounted | +5000 | Harder to hit mounted targets |
| NPC | +500 | Prefer PC targets |
| Newbie (<5) | +750 | Protect new players |
| Sitting | -400 | Easy target |
| Resting | -600 | Easier target |
| Sleeping | -800 | Easiest target |
| Fighting me | -300 | Already engaged |
| Hated | -350 | Priority target |
| Wounded (<30% HP) | -250 | Finish off wounded |

**Anti-Exploitation: Tank Detection**

```cpp
// If mob is AFF_AGGRESSOR and fighting a pet/zombie
// while a PC is not fighting, switch to the PC
if (isAffected(AFF_AGGRESSOR) && !fight()->isPc()) {
    for (stuff in room) {
        if (wimp->fight() == this || wimp->fight() == fight()) {
            beingUsed = true;
        }
    }
    if (beingUsed) {
        doSay("I think I'll just take care of you first!");
        // Switch target to the PC using the tank
    }
}
```

### Friend/Foe Determination

**isFriend()** - Source: `code/code/misc/mobact.cc:4368-4384`

```cpp
bool TMonster::isFriend(TBeing& myfriend) {
    // Same group
    if (inGroup(myfriend)) return TRUE;

    // Same mob vnum
    if (mob_index[getMobIndex()].virt == mob_index[fm->getMobIndex()].virt)
        return TRUE;

    // Same race and faction, within 5 levels (or both 50+)
    if (!myfriend.isPc() && isSameRace(&myfriend) && isSameFaction(&myfriend) &&
        ((myfriend.GetMaxLevel() + 5) > GetMaxLevel() ||
         (GetMaxLevel() > 50 && myfriend.GetMaxLevel() > 50)))
        return TRUE;

    return FALSE;
}
```

**assistFriend()** - Source: `code/code/misc/mobact.cc:4140-4250`

Mobs assist friends in combat. Police mobs have special behavior:
- Break up non-police fights
- Prioritize attacking murderers of other police

## Pursuit and Tracking

### Mob Hunting State (TMonster)

```cpp
class TMonster : public TBeing {
  public:
    TBeing* specials.hunting;  // Current hunting target (RAW POINTER)
    int hunt_dist;              // Remaining tracking distance
    ubyte persist;              // Persistence counter (decrements per room)
    int oldRoom;                // Birth room for return navigation
    unsigned long specials.act; // ACT_HUNTING flag when active
};
```

**Source:** `code/code/misc/monster.h`, `code/code/misc/being.h`

**CRITICAL: Raw Pointer Hazard** - `specials.hunting` is a raw pointer. If the target is deleted while hunting, this becomes a dangling pointer. Always validate before use.

### setHunting() - Initialization

```cpp
void TMonster::setHunting(TBeing* tch) {
  int dist;
  persist = GetMaxLevel();
  dist = 50 + GetMaxLevel();
  if (Hates(tch, NULL))
    dist *= 2;
  SET_BIT(specials.act, ACT_HUNTING);
  specials.hunting = tch;
  hunt_dist = dist;
  oldRoom = inRoom();

  if (tch->isImmortal())
    tch->sendTo(COLOR_MOBS, format(">>%s is hunting you from %s\n\r") %
                getName() % roomp->name);
}
```

**Source:** `code/code/misc/opinion.cc:566-584`

**Key mechanics:**
- **Distance formula**: `50 + GetMaxLevel()` rooms base
- **Hatred bonus**: Distance doubled if `Hates(target, NULL)` returns true
- **Persistence**: `persist = GetMaxLevel()` (decrements each tracking attempt)
- **Old room cache**: Stores birth room for return navigation when hunt fails
- **ACT_HUNTING flag**: Set to mark active hunting state

### hunt() - Main Hunting Loop

```cpp
int TMonster::hunt();  // Returns DELETE_THIS on mob death
```

Called from `mobileActivity()`. Behavior:

1. **If persist <= 0:** Return home or stop hunting
2. **If target visible in room:** Call `targetFound()` to attack
3. **Otherwise:** Use `dirTrack()` to find path, move toward target

Tracking distance per tick scales with level: `cbrt(GetMaxLevel())` rooms.

**Source:** `code/code/misc/mobact.cc:409-558`

**Movement scaling:** Cube root formula creates non-linear progression:

| Level | cbrt(level) | Rooms per Hunt |
|-------|-------------|----------------|
| 1 | 1.0 | 1 |
| 8 | 2.0 | 2 |
| 27 | 3.0 | 3 |
| 64 | 4.0 | 4 |
| 125 | 5.0 | 5 |

**Probabilistic rounding:** The fractional component of `cbrt(GetMaxLevel())` is converted to a probability:
```cpp
if (::number(0, 99) < (int)(100 * (cbrt(GetMaxLevel()) - mpr)))
  mpr++;
```

For level 10: `cbrt(10) = 2.154`, so 15.4% chance to move 3 rooms instead of 2.

Special behaviors:
- Clerics may cast Summon or Astral Walk to reach distant targets
- Archers (SPEC_ARCHER) shoot at visible targets before moving
- Musk gas in rooms costs extra persistence

**Cleric Hunting Magic:**

Clerics have a 20% chance to use magic instead of walking when hunting:

```cpp
// In hunt() - mobact.cc:474-509
if (getClass() == CLASS_CLERIC && GetMaxLevel() >= 30) {
  if (::number(0, 4) == 0) {  // 20% chance
    if (specials.hunting->GetMaxLevel() < 15) {
      // Target too low level for magic
    } else if (inZone() == specials.hunting->inZone()) {
      // Same zone: use SPELL_SUMMON
      doCast(SPELL_SUMMON, specials.hunting->getName());
      return FALSE;
    } else {
      // Different zone: use SPELL_ASTRAL_WALK
      doCast(SPELL_ASTRAL_WALK, specials.hunting->getName());
      return FALSE;
    }
  }
}
```

**Requirements:**
- Cleric class
- Level 30+
- 20% random chance
- Target level 15+

**Spell selection:**
- **Same zone**: `SPELL_SUMMON` pulls target to caster
- **Cross-zone**: `SPELL_ASTRAL_WALK` teleports caster to target

### targetFound() - Target Found

```cpp
int TMonster::targetFound();  // Returns DELETE_THIS
```

Initiates combat when hunt target is found in the same room.

**Source:** `code/code/misc/mobact.cc:373-406`

```cpp
int TMonster::targetFound() {
  if (!specials.hunting) {
    vlogf(LOG_BUG, format("%s: targetFound with NULL hunting") % getName());
    return FALSE;
  }

  if (!canSee(specials.hunting, INFRA_YES)) {
    // Lost sight of target
    return FALSE;
  }

  // Check peaceful room
  if (roomp && roomp->isRoomFlag(ROOM_PEACEFUL)) {
    doAction(getName(), CMD_FUME);
    specials.hunting = NULL;
    REMOVE_BIT(specials.act, ACT_HUNTING);
    return FALSE;
  }

  // Announce attack
  if (intelligence > 10) {
    doSay(format("Time to die, %s!") % specials.hunting->getName());
  } else {
    doAction(getName(), CMD_GROWL);
  }

  // Initiate combat
  int rc = takeFirstHit(specials.hunting);

  if (IS_SET_DELETE(rc, DELETE_VICT)) {
    specials.hunting = NULL;  // Clear pointer before propagating
    REMOVE_BIT(specials.act, ACT_HUNTING);
    return DELETE_VICT;
  }

  if (IS_SET_DELETE(rc, DELETE_THIS)) {
    return DELETE_THIS;
  }

  // Clear hunting state (now in normal combat)
  specials.hunting = NULL;
  REMOVE_BIT(specials.act, ACT_HUNTING);

  return TRUE;
}
```

**Key behaviors:**
- **Vision validation**: Requires `canSee(target, INFRA_YES)`
- **Peaceful room handling**: Fume instead of attacking
- **Intelligence-based messaging**: Smart mobs say "Time to die", animals growl
- **Combat initiation**: `takeFirstHit()` starts the fight
- **State cleanup**: Clears `specials.hunting` and `ACT_HUNTING` flag
- **DELETE flag propagation**: Handles both `DELETE_VICT` (target died) and `DELETE_THIS` (hunter died)

### dirTrack() - Core Pathfinding

```cpp
dirTypeT TBeing::dirTrack(TBeing* vict);
```

**Source:** `code/code/disc/disc_thief_stealth.cc:331-402`

Returns direction to move toward target, or DIR_NONE if path not found.

**Vision/Light Requirements:**

Tracking requires one of:
1. `roomp->getLight() + visionBonus > 0` (sufficient light)
2. `roomp->isRoomFlag(ROOM_ALWAYS_LIT)` (always-lit room)
3. `isAffected(AFF_TRUE_SIGHT)` (magical vision)
4. `isAffected(AFF_CLARITY)` (clarity spell)
5. `isImmortal()` (immortals ignore light)

**Without meeting any requirement:**
```
You can't see well enough to find a trail.
```

**SKILL_CONCEALMENT Counter:**

Targets with `SKILL_CONCEALMENT` affect can probabilistically block tracking:

```cpp
if (::number(1, 150) < aff->modifier) {
  // Tracking blocked
}
```

**Concealment values:**

| Skill Level | Modifier | Block Chance |
|-------------|----------|--------------|
| 10 | 10 | 6.7% |
| 50 | 50 | 33.3% |
| 100 | 100 | 66.7% |
| 150 | 150 | 100% |

**Global vs Zone Pathfinding:**

**Global pathfinding** (choose_exit_global) used when:
- Character level >= `MIN_GLOB_TRACK_LEV` (level threshold)
- Affected by `SPELL_TRAIL_SEEK` (magical enhancement)
- Has `ACT_HUNTING` flag set (mob hunting)

**Zone-limited pathfinding** (choose_exit_in_zone) used otherwise.

**Constant:** `MIN_GLOB_TRACK_LEV = 30` (defined in `code/code/misc/extern.h`)

**Portal Handling:**

Normal directions return codes 0-9 (DIR_NORTH through DIR_SOUTHWEST). Portal exits return codes > 9:

```cpp
int portalIndex = code - 9;
```

The portal index indicates which portal object in the room's `stuff` list leads to the destination.

### Player Tracking System

**doTrack() - Player Command** - Source: `code/code/disc/disc_thief_stealth.cc:20-219`

**Distance Formulas by Class:**

| Class | Formula | Example (skill=100, level=20) |
|-------|---------|-------------------------------|
| Ranger | `2 × skillValue` | 200 rooms |
| Thief | `2 × skillValue` | 200 rooms |
| Warrior | `skillValue / 2` | 50 rooms |
| Mage | `skillValue + GetMaxLevel()` | 120 rooms |
| Other | `skillValue` | 100 rooms |

**Race Modifiers:**

| Race | Modifier | Effect |
|------|----------|--------|
| Giant | ×2 | Double tracking distance |
| Elven | ×2 | Double tracking distance |
| Devil | MAX_ROOMS | Unlimited tracking |
| Demon | MAX_ROOMS | Unlimited tracking |

**Quest Bit Modifiers:**

| Quest Bit | Modifier | Effect |
|-----------|----------|--------|
| `TOG_IS_CRAVEN` | -25 | Reduced tracking distance |
| `TOG_IS_VICIOUS` | +25 | Increased tracking distance |

**SPELL_TRAIL_SEEK Enhancement:**

The Trail Seek spell adds +50 to tracking distance and enables global pathfinding:

```cpp
if (affectedBySpell(SPELL_TRAIL_SEEK)) {
  distance += 50;
  path.setStayZone(false);  // Enable cross-zone tracking
}
```

**AUTO_HUNT Integration:**

If the player has `AUTO_HUNT` autobit set, the tracking command automatically queues movement:

```cpp
if (hasAutobit(AUTO_HUNT)) {
  queueCommand(format("%s") % dirs[code]);
}
```

This creates seamless automated pursuit - the player keeps moving toward the target without needing to manually enter direction commands.

## Mobile Activity Loop

### mobileActivity() - The Main AI Loop

```cpp
int TMonster::mobileActivity(int pulse);  // Returns DELETE_THIS
```

**Source:** `code/code/misc/mobact.cc:3328-3779`

Called every `Pulse::MOBACT` (1.2 seconds) via `procCharMobileActivity`. This is the central dispatcher for all mob AI.

### Processing Order

1. **Spell checks:** Plague locusts, sticks-to-snakes dissipation
2. **Lag handling:** Decrement and skip if in lag
3. **Position management:** Stand up if needed for combat/hunting
4. **Charmed pet behavior:** `charmeeStuff()`, `protectionStuff()`
5. **Spec proc triggers:** `CMD_GENERIC_PULSE`, `CMD_MOB_ALIGN_PULSE`
6. **Stuck item removal:** `remove()` pulls arrows from limbs
7. **Combat targeting:** `senseWimps()` for intelligent target switching
8. **Weapon selection:** `findABetterWeapon()`
9. **In-combat behavior:**
   - Fear checks
   - Combat spec procs (`CMD_MOB_COMBAT`, `CMD_MOB_COMBAT2`)
   - Vampire/lycanthrope bite attacks
   - Class-specific combat (`classStuff()`)
10. **Non-combat behavior:** `notFightingMove()`
11. **Hate/fear processing:** `doHatefulStuff()`, `fearCheck()`
12. **Aggression:** `aggroCheck()`

### Key Timing Intervals

| Interval | Behavior |
|----------|----------|
| Every pulse | Spell effects, lag, position |
| 2x MOBACT | Rescue allies, spec procs |
| 5x MOBACT | Charmee/protection stuff |
| 7x MOBACT | Horse finding, assist friends |
| 11x MOBACT | Scavenging |
| 13x MOBACT | Thief stealing |
| 16x MOBACT | Remove stuck items |
| 30x MOBACT | Return to default position |
| 50x MOBACT | Alignment spec procs |

## Response System

Mobs can have scripted responses to player actions stored in the `mobresponses` database table.

### Data Structures

```cpp
class Responses {
    resp* respList;           // Linked list of response triggers
    int respCount;            // Number of responses
    RespMemory* respMemory;   // Memory of recent interactions
};

class resp {
    cmdTypeT cmd;    // Triggering command (CMD_SAY, CMD_GIVE, etc.)
    char* args;      // Argument pattern to match
    command* cmds;   // Linked list of commands to execute
    resp* next;      // Next response in list
};
```

**Source:** `code/code/misc/response.h`

### Loading Responses

```cpp
void TMonster::loadResponses(int virt, const sstring& immortal = "");
```

Loads from database at mob creation. Query: `select response from mobresponses where vnum=%i`

### Checking Responses

```cpp
int TMonster::checkResponses(TBeing*, TThing*, const sstring&, cmdTypeT);
```

Called from `triggerSpecial()` when players perform actions near mobs.

### Executing Responses

```cpp
int TMonster::modifiedDoCommand(cmdTypeT cmd, const sstring& arg, TBeing* mob, const resp* respo);
```

Executes response commands with special handling for:
- `CMD_RESP_TOROOM`, `CMD_RESP_TOVICT`, `CMD_RESP_TONOTVICT` - message routing
- `CMD_LOAD`, `CMD_RESP_LOADMOB` - item/mob spawning
- `CMD_FLAG`, `CMD_RESP_UNFLAG` - quest flag manipulation

## ACT_* Flags Affecting AI

Defined in `code/code/misc/defs.h`:

| Flag | Effect |
|------|--------|
| `ACT_SENTINEL` (1<<1) | Won't wander from birth room |
| `ACT_SCAVENGER` (1<<2) | Picks up items |
| `ACT_AGGRESSIVE` (1<<5) | Attacks players on sight |
| `ACT_STAY_ZONE` (1<<6) | Won't leave birth zone |
| `ACT_WIMPY` (1<<7) | Only attacks sleeping targets, flees at low HP |
| `ACT_HATEFUL` (1<<9) | Has active hate list |
| `ACT_AFRAID` (1<<10) | Has active fear list |
| `ACT_HUNTING` (1<<12) | Currently tracking a target |
| `ACT_IMMORTAL` (1<<22) | Cannot gain hatred |

## Target Management

### aiTarget()

```cpp
void TMonster::aiTarget(TBeing* vict);
```

Sets the mob's attention target. **Important:** Target must be a PC:
```cpp
if (vict->isPc())
    setTarg(vict);
else {
    if (vict->master && vict->master->isPc())
        setTarg(vict->master);
    else
        setTarg(NULL);  // Mob acting on its own
}
```

This prevents infinite mob-mob interaction loops.

### findAHatee() / findAFearee()

```cpp
TBeing* TMonster::findAHatee();  // Find hated character in room
TBeing* TMonster::findAFearee(); // Find feared character in room
```

Scan room contents for characters matching hate/fear criteria.

## Class-Based Combat AI

`classStuff()` dispatches to class-specific combat functions:

| Function | Class | Behaviors |
|----------|-------|-----------|
| `fighterMove()` | Warrior | Bash, bodyslam, spin, kick, disarm |
| `monkMove()` | Monk | Springleap, hurl, bonebreak, shoulder throw, chi |
| `thiefMove()` | Thief | Stab, disarm |
| `mageMove()` | Mage | Offensive spells based on best discipline |
| `clerMove()` | Cleric | Healing, harm spells |
| `shamanMove()` | Shaman | Spirit spells, flatulence |
| `deikhanMove()` | Deikhan | Charge (mounted), fighter moves |
| `rangMove()` | Ranger | Nature skills |

## DELETE Safety Patterns

AI functions frequently return DELETE flags. Always check and propagate:

```cpp
rc = mobileActivity(pulse);
if (IS_SET_DELETE(rc, DELETE_THIS))
    return true;  // Scheduler will delete mob
return false;
```

Common return patterns:
- `DELETE_THIS` - The mob died or should be removed
- `DELETE_VICT` - The target died (rare from AI functions)
- `TRUE` - Action taken, stop processing
- `FALSE` - No action, continue processing

### Pattern 1: hunt() Return Value Propagation

```cpp
// CORRECT: Check immediately, return early
int rc = goDirection(code);
if (IS_SET_DELETE(rc, DELETE_THIS))
  return DELETE_THIS;  // Propagate to caller

// WRONG: Continuing after potential deletion
int rc = goDirection(code);
doSomething();  // May crash if deleted!
```

### Pattern 2: targetFound() State Cleanup Before Return

```cpp
// CORRECT: Clear pointer before propagating DELETE_VICT
if (IS_SET_DELETE(rc, DELETE_VICT)) {
  specials.hunting = NULL;  // Clear first
  REMOVE_BIT(specials.act, ACT_HUNTING);
  return DELETE_VICT;
}

// WRONG: Returning without cleanup
if (IS_SET_DELETE(rc, DELETE_VICT))
  return DELETE_VICT;  // Pointer still set!
```

### Pattern 3: Portal Iteration Safety

```cpp
// CORRECT: Cache next before operations
for (StuffIter it = roomp->stuff.begin(); it != roomp->stuff.end(); ++it) {
  TPortal* tp = dynamic_cast<TPortal*>(*it);
  if (tp && ++seen == count) {
    int rc = portal_cost(tp);
    if (IS_SET_DELETE(rc, DELETE_THIS))
      return DELETE_THIS;  // Safe - already advanced iterator
    break;
  }
}
```

## Common Gotchas

### 1. charList Chain Deletion

```cpp
// MEMORY LEAK - only deletes first node
delete hates.clist;

// CORRECT - delete entire chain
for (charList* k = hates.clist; k; ) {
    charList* next = k->next;
    delete k;
    k = next;
}
```

### 2. Target Pointer Validity

```cpp
// CRASH - target may have logged off
TBeing* targ = targ();
targ->doSomething();

// CORRECT - validate target
TBeing* targ = targ();
if (!targ || !sameRoom(*targ) || !canSee(targ))
    return FALSE;
```

### 3. DELETE Flag Propagation

```cpp
// BUG - ignoring return value
assistFriend();
// mob may be dead here!

// CORRECT
rc = assistFriend();
if (IS_SET_DELETE(rc, DELETE_THIS))
    return DELETE_THIS;
```

### 4. Utility/Guild Mob Checks

Many AI functions skip utility and guild mobs:
```cpp
if (UtilMobProc(this)) return FALSE;
if (GuildProcs(spec)) return FALSE;
```

Forgetting these checks causes shopkeepers to attack customers.

### 5. Not Propagating DELETE_THIS from goDirection()

```cpp
// CRASH: Ignoring return value
goDirection(code);
persist--;  // Mob may be dead here!

// CORRECT: Check and propagate
int rc = goDirection(code);
if (IS_SET_DELETE(rc, DELETE_THIS))
  return DELETE_THIS;
persist--;  // Safe - only reached if alive
```

### 6. Accessing Invalid hunting Pointer

```cpp
// CRASH: Pointer may be dangling
if (specials.hunting) {
  specials.hunting->sendTo("I'm coming for you!");  // May crash
}

// CORRECT: Validate pointer is still valid
if (specials.hunting && specials.hunting->isPc() &&
    specials.hunting->desc) {
  specials.hunting->sendTo("I'm coming for you!");
}
```

### 7. Not Clearing hunting Pointer Before Propagating DELETE_VICT

```cpp
// BUG: Pointer still set when returning
int rc = takeFirstHit(specials.hunting);
if (IS_SET_DELETE(rc, DELETE_VICT))
  return DELETE_VICT;  // Caller may still see stale pointer

// CORRECT: Clear pointer first
int rc = takeFirstHit(specials.hunting);
if (IS_SET_DELETE(rc, DELETE_VICT)) {
  specials.hunting = NULL;  // Clear before propagating
  REMOVE_BIT(specials.act, ACT_HUNTING);
  return DELETE_VICT;
}
```

## Key Source Files

| File | Contents |
|------|----------|
| `code/code/misc/monster.h` | TMonster class, charList, opinionData, Mobile_Attitude |
| `code/code/misc/monster.cc` | TMonster constructor/destructor, charList chain cleanup |
| `code/code/misc/opinion.cc` | Hate/fear functions, hunting setup, setHunting(), developHatred() |
| `code/code/misc/mobact.cc` | mobileActivity(), combat AI, aggroCheck(), hunt(), targetFound() |
| `code/code/misc/ai_utility.cc` | pissed(), aggro(), aiTarget() |
| `code/code/misc/ai_responses.cc` | Response system execution |
| `code/code/misc/response.cc` | Response data structure implementations |
| `code/code/misc/response.h` | Response class declarations |
| `code/code/misc/defs.h` | ACT_* flags, HATE_* flags, FEAR_* flags |
| `code/code/disc/disc_thief_stealth.cc` | dirTrack(), doTrack() player command |
