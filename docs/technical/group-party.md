---
title: Group and Party System
description: Cooperative gameplay management through master-follower relationships enabling shared combat, experience distribution, and money splitting with spatial awareness and AFF_GROUP flag requirements.
keywords:
  - followData
  - master-follower
  - AFF_GROUP
  - inGroup
  - addFollower
  - stopFollower
  - circleFollow
  - reformGroup
  - getExpShare
  - gainExpPerHit
  - doSplit
  - splitShares
  - group-membership
  - XP-distribution
  - money-sharing
category: Important Systems
related:
  - experience-leveling.md
  - combat-formulas.md
  - quest-system.md
  - polymorph-safety.md
  - delete-flags.md
last_updated: 2026-01-29
source_files:
  - code/code/misc/being.h
  - code/code/misc/movement.cc
  - code/code/misc/spell_parser.cc
  - code/code/misc/combat.cc
  - code/code/misc/other.cc
  - code/code/misc/utility.cc
  - code/code/misc/rent.cc
  - code/code/sys/connect.h
---

# Group and Party System

The group system manages cooperative gameplay through master-follower relationships, enabling shared combat, experience distribution, and money splitting. Understanding this system is critical for correct XP sharing and preventing group state corruption.

**Misusing this system causes XP exploits and state corruption.** Common errors: not checking AFF_GROUP flag before distributing XP, assuming followers of followers are in group, forgetting to check inGroup() before benefits, creating circular follow chains.

## Overview

SneezyMUD uses a **pointer-based master-follower tree architecture** rather than dedicated Group classes. Groups are implicit relationships defined by `master` and `followers` pointers combined with the `AFF_GROUP` bitvector flag.

**Key characteristics:**
- Master-follower tree with two-tier group depth
- AFF_GROUP flag must be explicitly set (not automatic on follow)
- XP distribution requires same-room location
- No hard-coded group size limit (memory-limited only)
- Leadership auto-promotes via reformGroup() on leader death
- Groups are spatial — only in-room members receive XP

## Core Data Structures

### followData Class

```cpp
class followData {
  public:
    TBeing* follower;      // Pointer to follower character
    followData* next;      // Linked list to next follower
    followData();
    followData(const followData& a);
    followData& operator=(const followData& a);
    ~followData();
};
```

**Source:** `code/code/misc/being.h`

Simple linked list node representing a single follower. No maximum size enforced — followers limited only by available memory.

### Master/Follower Pointers on TBeing

```cpp
TBeing* master;              // Who this character is following (NULL if leader)
followData* followers;       // Linked list of followers (NULL if no followers)
```

**Source:** `code/code/misc/being.h`

**Architecture:**
- Master-follower is a **tree structure**, not flat groups
- One master per follower (but master can have multiple followers)
- Followers of followers are NOT transitive — groups are two-tier max depth

### Session Configuration

```cpp
// In Descriptor (connect.h)
byte group_share;           // Individual's XP/money share factor (1-10)
sstring groupName;          // Custom group name
bool amGroupTank;           // Tank flag for group healer focus targeting
```

**Source:** `code/code/sys/connect.h`

The `group_share` factor is set by the group leader and determines money distribution (not XP distribution, which is level-based).

## Group Membership Model

### Master-Follower Hierarchy

Groups form a **three-tier hierarchy:**

1. **Group Leader** — Master of 0+ followers, no master pointer
2. **Followers** — Each has exactly one master pointer, can have their own followers
3. **Followers' Followers** — Can have their own followers (nested groups)

**Example structure:**
```
Leader (master=NULL, AFF_GROUP)
  ├─ Follower1 (master=Leader, AFF_GROUP)
  │   └─ Follower1_A (master=Follower1, AFF_GROUP not set)
  └─ Follower2 (master=Leader, AFF_GROUP)
```

In this example, Follower1_A is NOT in the group with Leader/Follower1/Follower2 because they have different masters.

### Group Membership Detection: inGroup()

**CRITICAL:** Membership is NOT automatic via follower chains. The `inGroup()` function validates group membership:

```cpp
bool TBeing::inGroup(const TBeing& tbt) const {
    // 1. Same person or mounted/rider relationship
    if ((this == &tbt) || (&tbt == mounted) || (&tbt == rider))
        return TRUE;

    // 2. Both must have AFF_GROUP flag
    if (!isAffected(AFF_GROUP) || !tbt.isAffected(AFF_GROUP))
        return FALSE;

    // 3. Rider check (recursive)
    if (rider && inGroup(*rider))
        return TRUE;

    // 4. Relationship check - one of these must be true:
    //    - I am their master
    //    - They are my master
    //    - We share the same master (siblings)
    if (this == tbt.master || master == &tbt ||
        (master && tbt.master && master == tbt.master))
        return TRUE;

    return FALSE;
}
```

**Source:** `code/code/misc/utility.cc`

**Key requirements:**
- **Both characters must have `AFF_GROUP` flag set**
- One must be the other's master, OR both share the same master
- Mount/rider relationships are implicitly grouped

## Group Formation and Management

### follow Command

```
follow <target>
follow self         # Stop following (same as 'stop')
```

**Implementation:** `doFollow()` in `code/code/misc/movement.cc`

**Validation flow:**
1. Cannot follow if `rider` is set (mounted)
2. Quest flag checks:
   - Cannot follow with `PLR_SOLOQUEST` flag
   - Cannot follow target with `PLR_SOLOQUEST` flag
   - Group quest flag must match between follower and leader
3. Charmed check: Charmed followers can only follow their master
4. Circle-follow check: Prevents A→B→C→A loops via `circleFollow()`
5. Calls `target->addFollower(this)` if all validations pass

### addFollower() — Core Follower Addition

```cpp
void TBeing::addFollower(TBeing* foll, bool textLimits) {
    // Error check: Follower cannot already have a master
    if (foll->master) {
        vlogf(LOG_BUG, "addFollower error: %s already has master",
              foll->getName().c_str());
        foll->master = NULL;  // Force clear
    }

    // Set follower's master pointer
    foll->master = this;

    // Create new followData node
    followData* k = new followData(foll);

    // Append to tail of followers linked list
    // ... (traverses list to find tail)

    // Messages
    act("You now follow $N.", FALSE, foll, NULL, this, TO_CHAR);
    act("$n starts following you.", FALSE, foll, NULL, this, TO_VICT);
    act("$n now follows $N.", TRUE, foll, NULL, this, TO_NOTVICT);

    // Auto-group if leader has AUTO_AUTOGROUP bit
    if (IS_SET(specials.act, AUTO_AUTOGROUP))
        doGroup(foll->getName());
}
```

**Source:** `code/code/misc/spell_parser.cc`

**Important:** `addFollower()` does NOT automatically set `AFF_GROUP` flag — that must be done explicitly via the `group` command.

### stopFollower() — Follower Removal

```cpp
void TBeing::stopFollower(bool remove, stopFollowerT textLimits) {
    // If charmed/pet/thrall: Convert to AFFECT_ORPHAN_PET for retrain
    if (isPet(PETTYPE_PET | PETTYPE_CHARM | PETTYPE_THRALL)) {
        // Special handling for pets
    }

    // Send appropriate messages based on textLimits
    // ... (messages to char, master, and room)

    // Find in master's follower linked list and remove
    for (followData* j = master->followers; j; j = j->next) {
        if (j->follower == this) {
            // Unlink from list
            // Delete followData node
        }
    }

    // Clear master pointer
    master = NULL;

    // Remove AFF_CHARM and AFF_GROUP flags
    REMOVE_BIT(specials.affectedBy, AFF_CHARM);
    REMOVE_BIT(specials.affectedBy, AFF_GROUP);
}
```

**Source:** `code/code/misc/spell_parser.cc`

**stopFollowerT message options:**
- `STOP_FOLLOWER_DEFAULT` — Full messages to char, victim, notvict
- `STOP_FOLLOWER_CHAR_VICT` — Only message to follower and leader
- `STOP_FOLLOWER_CHAR_NOTVICT` — Only message to room
- `STOP_FOLLOWER_SILENT` — No messages

### stop Command

```
stop
```

Calls `stopFollower(TRUE)` on the follower. Removes follower relationship and clears `AFF_GROUP` flag.

**Source:** `code/code/misc/other.cc`

### group Command

```
group                               # List group members
group name <name>                   # Set custom group name (leader only)
group lots                          # Throw lots (random selection, leader only)
group share <player> <1-10>         # Set player's XP/money share factor (leader only)
group amtank / group amnottank      # Flag self as group tank
group seeksgroup                    # Mark in who list as seeking group
group all / group followers         # Mass-add followers to group
```

**Source:** `code/code/misc/other.cc`

**Display format:**
```
Group Name consists of:

Leader        [XX.Xhp YY.Ymana ZZ.Zmove, look POSITION.]
              NN shares talens, ZZ.Z% shares XP
Follower1     [...]
```

The percentages shown are based on level-based XP shares (via `getExpSharePerc()`), not the `group_share` factor (which only affects money).

## Circle-Follow Prevention

```cpp
bool TBeing::circleFollow(const TBeing* victim) const {
    for (const TBeing* k = victim; k; k = k->master) {
        if (k == this)  // Found self in master chain
            return TRUE;  // Circular reference detected!
    }
    return FALSE;
}
```

**Source:** `code/code/misc/spell_parser.cc`

This function walks the master chain from the target upward. If it encounters `this`, adding the follower would create a loop (A→B→C→A).

**Example:**
- A wants to follow C
- C's master is B
- B's master is A
- Result: `circleFollow()` returns true, follow is blocked

## Group Leadership

### Normal Leadership

The character with `master == NULL` is the group leader. All group members have the same `master` pointer (either the leader directly, or NULL for the leader itself).

### Leadership Succession: reformGroup()

Called automatically when the group leader dies. Implements a **two-pass algorithm** to find a suitable successor.

**Pass 1 — Primary succession:**
1. Iterate through followers list
2. Skip pure mobs (unless original leader was a mob)
3. Skip those not in group (check `inGroup()`)
4. First valid follower becomes new leader:
   - Remove from followers list via `stopFollower()`
   - Set new leader's `master = NULL`
   - All other followers transferred to new leader

**Pass 2 — Secondary succession (if no suitable leader found):**
1. Relax restrictions: Accept immortals, polymorphed mobs, linkdead players
2. First acceptable follower becomes leader
3. Process remaining followers as in Pass 1

**Messages:**
- New leader: "Group leader has died and you have taken over"
- Others: "Group leader has died and [new_leader] has taken over"

**Source:** `code/code/misc/combat.cc`

**Restriction:** Only pure players (not mobs, not polymorphed) can be automatic successors in Pass 1, unless the original group leader was a mob.

## XP Distribution System

### Level-Based Share Calculation

**getExpShare() — Individual's Base Share**

```cpp
double TBeing::getExpShare() const {
    return mob_exp(GetMaxLevel());
}
```

Returns the base mob experience value for the character's max level. Higher level = larger share of group XP.

**Source:** `code/code/misc/combat.cc`

**Important:** Not modified by `group_share` factor — that only affects money distribution.

### getExpSharePerc() — Percentage Calculation

```cpp
double TBeing::getExpSharePerc() const {
    double totalshares = 0;

    // Get master's share (or own share if leader)
    TBeing* leader = master ? master : this;
    totalshares = leader->getExpShare();

    // Add followers' shares if in group
    for (followData* f = leader->followers; f; f = f->next) {
        if (f->follower->isAffected(AFF_GROUP))
            totalshares += f->follower->getExpShare();
    }

    return (getExpShare() / totalshares * 100);
}
```

**Source:** `code/code/misc/combat.cc`

Returns the percentage of total group XP this character receives. Shown in `group` command output.

### gainExpPerHit() — XP Distribution

Called during combat to distribute experience to group members.

**Algorithm:**

1. **Spatial requirement:** Only characters in the **same room** as the victim receive XP

2. **Restrict XP affect:** Check for `AFFECT_COMBAT` with `COMBAT_RESTRICT_XP`
   - If set and attacker not in affect owner's group: No XP awarded

3. **Calculate total shares:**
   ```cpp
   double total_shares = 0;

   // Master's share
   if (master)
       total_shares += master->getExpShare();
   else
       total_shares += getExpShare();

   // Add in-room followers with AFF_GROUP
   for (followData* f = master->followers; f; f = f->next) {
       if (f->follower->isAffected(AFF_GROUP) &&
           f->follower->in_room == in_room)
           total_shares += f->follower->getExpShare();
   }
   ```

4. **Per-character distribution:**
   ```cpp
   exp_per_level = total_exp / total_shares;
   char_exp = exp_per_level * char_share * FRACT(ch, victim);
   ```

5. **Apply to master and each in-room follower with `AFF_GROUP`**

**Source:** `code/code/misc/combat.cc`

**CRITICAL:** The `FRACT()` trophy modifier applies to each character individually. This means characters who have killed the mob type many times receive reduced XP.

**See also:** [Experience and Leveling](experience-leveling.md) for complete XP formulas and trophy system details.

## Money Distribution

### splitShares() — Individual Money Share Factor

```cpp
int splitShares(const TBeing* k) {
    if (k->desc)
        return k->desc->session.group_share;  // Player: 1-10
    return 0;  // NPC/pet: no share
}
```

**Source:** `code/code/misc/other.cc`

Returns the character's money share factor (1-10), set by the group leader via `group share <player> <value>`.

**Key difference from XP:** NPCs return 0 for money splits (no share), but they DO count for XP distribution via `getExpShare()`.

### doSplit() — Money Distribution

```
split <amount>
```

Divides money among group members based on their `group_share` factors.

**Calculation:**
```cpp
// Calculate total shares
int total_shares = 0;
for each in-room group member:
    total_shares += splitShares(member);

// Distribute to each member
for each in-room group member:
    member_amount = total_amount * (splitShares(member) / total_shares);
    giveMoney(member, member_amount, GOLD_SPLIT);
```

**Source:** `code/code/misc/other.cc` (implementation of doSplit)

**Example:**
- Leader has group_share = 5
- Follower1 has group_share = 3
- Follower2 has group_share = 2
- Split 1000 talens:
  - Total shares = 10
  - Leader gets: 1000 * (5/10) = 500
  - Follower1 gets: 1000 * (3/10) = 300
  - Follower2 gets: 1000 * (2/10) = 200

## Group Restrictions

### Quest Flag Restrictions

**Solo Quest (`PLR_SOLOQUEST`):**
- Cannot follow anyone
- Cannot be followed
- Enforced in `doFollow()` — blocked before group formation

**Group Quest (`PLR_GRPQUEST`):**
- All group members must have this flag
- Leader can add anyone with matching flag
- Prevents mixed quest/non-quest groups

### Charm Restrictions

Charmed followers can ONLY follow their master:

```cpp
if (isAffected(AFF_CHARM) && master) {
    act("But you only feel like following $N!", FALSE, this,
        NULL, master, TO_CHAR);
    return FALSE;  // Cannot follow anyone else
}
```

Attempting to follow a different character while charmed fails.

### Mounted/Rider Exclusion

```cpp
if (victim == riding)  // In doGroup()
    continue;          // Skip mount when grouping
```

Cannot group with your mount — the spatial relationship (mount/rider) makes group membership problematic.

### Immortal Mob Restriction

`ACT_IMMORTAL` mobs with `master == this` refuse grouping:

```cpp
if (IS_SET(victim->specials.act, ACT_IMMORTAL) &&
    !victim->isPc() &&
    victim->master == this && this != victim) {
    // Skip - won't join group
}
```

### Polymorph Handling

Groups survive polymorph transformation:
- Original character stored as `desc->original`
- Mob form takes group leadership via `addFollower()`
- On return, group relationships restored automatically

**See also:** [Polymorph Safety](polymorph-safety.md) for transformation mechanics.

## Group Persistence

### Follower Saving

On rent/logout, only **NPC followers** are saved to `.fol` files:

```cpp
void TPerson::saveFollowers() {
    // Iterate followers list
    for (followData* tmpf = followers; tmpf; tmpf = tmpf->next) {
        // Check isSaveMob() - only saves mobs (not PCs)
        if (tmpf->follower->isSaveMob()) {
            // Strip affects (prevent doubling on rent return)
            tmpf->follower->clearAffects();
            // Save mob data to .fol file
            // Re-apply affects
        }
    }
}
```

**Source:** `code/code/misc/rent.cc`

On load, followers are restored via `loadFollowers()` which calls `addFollower()` for each saved mob.

### Group Cleanup on Logout

```cpp
void TPerson::removeFollowers() {
    // Delete follower files
    // Character removed from master's follower list via dieFollower()
}
```

**Source:** `code/code/misc/rent.cc`

### dieFollower() — Death/Disconnect Cleanup

```cpp
void TBeing::dieFollower() {
    // Stop following own master
    if (master)
        stopFollower(TRUE);

    // Iterate followers list and stop each
    while (followers) {
        followers->follower->stopFollower(TRUE);
    }
}
```

**Source:** `code/code/misc/spell_parser.cc`

Called when a character dies or disconnects. Breaks all follow relationships (both as follower and as leader).

## Common Bugs and Edge Cases

### Bug: Assuming Transitive Group Membership

```cpp
// WRONG: Assuming followers of followers are in group
Leader follows Nobody
Follower1 follows Leader (AFF_GROUP set)
Follower2 follows Follower1 (AFF_GROUP set)

// Follower2 is NOT in group with Leader!
// Different masters: Leader has master=NULL, Follower2 has master=Follower1
```

**Correct pattern:**
```cpp
// Check group membership explicitly
if (inGroup(*leader, *follower2)) {
    // This will return FALSE in above scenario
}
```

### Bug: Forgetting to Set AFF_GROUP

```cpp
// WRONG: Assuming addFollower() sets AFF_GROUP
target->addFollower(this);
// Group benefits DON'T apply yet!

// CORRECT: Set flag explicitly
target->addFollower(this);
SET_BIT(specials.affectedBy, AFF_GROUP);
```

The `group` command sets `AFF_GROUP`. The `addFollower()` function does not.

### Bug: Not Checking AFF_GROUP Before XP Distribution

```cpp
// WRONG: Distributing XP to all followers
for (followData* f = master->followers; f; f = f->next) {
    f->follower->gain_exp(amount, dam);  // Gives XP even without AFF_GROUP!
}

// CORRECT: Check AFF_GROUP flag first
for (followData* f = master->followers; f; f = f->next) {
    if (f->follower->isAffected(AFF_GROUP))
        f->follower->gain_exp(amount, dam);
}
```

### Bug: Out-of-Room Members Getting XP

```cpp
// WRONG: Distributing to all group members regardless of location
for (followData* f = master->followers; f; f = f->next) {
    if (f->follower->isAffected(AFF_GROUP))
        f->follower->gain_exp(amount, dam);  // Gets XP even in different room!
}

// CORRECT: Check same-room location
for (followData* f = master->followers; f; f = f->next) {
    if (f->follower->isAffected(AFF_GROUP) &&
        f->follower->in_room == in_room)
        f->follower->gain_exp(amount, dam);
}
```

### Bug: Circular Follow Chain

```cpp
// WRONG: Not checking circleFollow before adding
victim->addFollower(this);  // Creates A→B→A loop if victim follows this

// CORRECT: Check first
if (!circleFollow(victim)) {
    victim->addFollower(this);
} else {
    sendTo("That would create a circular follow chain!\n\r");
}
```

### Bug: Money vs XP Share Confusion

```cpp
// splitShares() returns group_share (1-10) for players, 0 for NPCs
// getExpShare() returns mob_exp(level) for ALL characters

// NPCs get XP but not money
if (npc->desc)  // FALSE for NPCs
    money_share = npc->desc->session.group_share;  // Returns 0
else
    money_share = 0;

// But NPCs still get XP:
xp_share = npc->getExpShare();  // Returns mob_exp(npc_level)
```

### Bug: Not Calling reformGroup() Before Deletion

```cpp
// WRONG: Deleting leader without reforming group
delete leader;  // Followers now have dangling master pointer!

// CORRECT: Reform first
leader->reformGroup();  // Promotes new leader
delete leader;          // Safe
```

## Key Source Files

| File | Contents |
|------|----------|
| `code/code/misc/being.h` | followData class, master/followers pointers |
| `code/code/misc/movement.cc` | doFollow() implementation |
| `code/code/misc/spell_parser.cc` | circleFollow(), stopFollower(), addFollower(), dieFollower() |
| `code/code/misc/combat.cc` | getExpShare(), getExpSharePerc(), gainExpPerHit(), reformGroup() |
| `code/code/misc/other.cc` | splitShares(), doSplit(), doGroup(), doStop() |
| `code/code/misc/utility.cc` | inGroup() validation |
| `code/code/misc/rent.cc` | removeFollowers(), saveFollowers() |
| `code/code/sys/connect.h` | Session group_share, groupName, amGroupTank |
