---
title: Group and Party System
category: important
keywords: [followData, master-follower, AFF_GROUP, inGroup, addFollower, stopFollower, circleFollow, reformGroup, getExpShare, gainExpPerHit, doSplit, splitShares, XP-distribution, money-sharing]
related: [experience-leveling.md, combat-formulas.md, quest-system.md, delete-flags.md]
primary_symbols:
  functions: [inGroup, addFollower, stopFollower, circleFollow, reformGroup, getExpShare, getExpSharePerc, gainExpPerHit, doSplit, splitShares, doFollow, doGroup, dieFollower, saveFollowers]
  classes: [followData, TBeing]
  files: [code/code/misc/being.h, code/code/misc/movement.cc, code/code/misc/spell_parser.cc, code/code/misc/combat.cc, code/code/misc/other.cc, code/code/misc/utility.cc, code/code/misc/rent.cc, code/code/sys/connect.h]
---

## Overview

How do you split experience and loot among multiple characters fighting together? How do you prevent someone from claiming group benefits while actually solo? The group system answers these questions through an implicit master-follower architecture where cooperative gameplay emerges from pointer relationships and explicit flag validation.

Groups in SneezyMUD are not dedicated objects or classes. They are emergent relationships defined by master and follower pointers combined with the AFF_GROUP flag. A character with `master=NULL` is a potential group leader. Characters pointing to that leader with `master` and having `AFF_GROUP` set are group members. This pointer-based approach means groups are lightweight and memory-limited only, but it also means membership is not automatic - you must explicitly validate both pointer relationships and flag states.

The system serves three core purposes: shared combat participation, proportional experience distribution, and configurable money splitting. When multiple characters group together and fight enemies in the same room, experience is distributed based on level-weighted shares. Money distribution uses a separate configurable share system ranging from 1-10, allowing leaders to adjust individual member payouts.

### Core Concepts

**Master-Follower Tree**: Groups form a hierarchical tree structure with a leader at the root. The leader has `master=NULL` and zero or more followers. Each follower has `master` pointing to the leader and can have their own followers, but groups are limited to two-tier depth. Followers of followers are not transitively grouped - they must share the same master to be in the same group.

**AFF_GROUP Flag Requirement**: Following someone does not automatically grant group membership. The AFF_GROUP affect flag must be explicitly set via the `group` command. This separation allows characters to follow without receiving group benefits, useful for pets, scouts, or temporary followers.

**Spatial Awareness**: Experience distribution is room-scoped. Only group members in the same room as the victim receive experience when damage is dealt. This prevents abuse where characters park in safe zones while collecting XP from distant combat.

**Level-Weighted Shares**: Each character's experience share is proportional to `mob_exp(GetMaxLevel())`, meaning higher-level characters receive larger absolute amounts but the percentage distribution is visible via the group display.

**Configurable Money Shares**: Unlike experience, money distribution uses the `group_share` value (1-10) set by the group leader. NPCs always have zero money share, but they do contribute to and receive experience distribution.

### Common Scenarios

**Scenario: Party Formation**
- Player A types `follow B`
- Validation checks: no circular chains, matching quest flags, not mounted
- `addFollower()` sets A's `master=B` and adds A to B's followers linked list
- A still does not receive group benefits
- B types `group A` which sets A's `AFF_GROUP` flag
- Now A receives XP and can participate in splits

**Scenario: Experience Distribution in Combat**
- Group of 3 (levels 10, 15, 20) in same room fights a mob
- Each hit calls `gainExpPerHit()`
- Total shares calculated: `mob_exp(10) + mob_exp(15) + mob_exp(20)`
- Each character receives: `(total_exp / total_shares) * their_share * trophy_modifier`
- Trophy modifier penalizes repeated kills of same mob type
- If level 10 moves to different room, they stop receiving XP

**Scenario: Leader Death and Succession**
- Group leader dies in combat
- `reformGroup()` called automatically
- Pass 1: Search for first valid player follower with AFF_GROUP
- Found follower removed from list, `master` set to NULL
- All remaining followers transferred to new leader
- Messages announce leadership change
- If no players available, Pass 2 relaxes restrictions to accept immortals/polymorphed

**Scenario: Money Split**
- Leader has `group_share=5`, two followers have `3` and `2`
- Leader types `split 1000`
- Total shares: 5+3+2 = 10
- Distribution: 500, 300, 200 talens respectively
- Only characters in same room receive shares
- NPCs receive zero (no descriptor means no group_share value)

## Patterns

### Group Membership Validation

**Always use `inGroup()` before granting group benefits.** Never assume master pointer alone proves membership.

```cpp
// WRONG: Checking only master pointer
if (ch->master == leader) {
    giveXP(ch, amount);  // Fails if AFF_GROUP not set
}

// CORRECT: Validate via inGroup()
if (ch->inGroup(*leader)) {
    giveXP(ch, amount);
}
```

**Why:** The master pointer indicates following relationship, not group membership. A character can follow without AFF_GROUP set (pets, scouts), and `inGroup()` validates both pointer relationship and flag state. Missing this check causes XP exploits where non-grouped followers receive benefits.

**Never assume transitive group membership.** Followers of followers are not in the same group.

```cpp
// WRONG: Assuming nested followers are grouped
Leader (master=NULL)
  └─ Follower1 (master=Leader, AFF_GROUP)
      └─ Follower2 (master=Follower1, AFF_GROUP)

// Follower2 is NOT in Leader's group - different masters
if (Follower2->inGroup(*Leader)) {  // Returns FALSE
    // Never executes
}
```

**Why:** `inGroup()` requires same master or direct master-follower relationship. Follower2's master is Follower1, not Leader, so they are in separate groups even though both have AFF_GROUP set.

### Experience Distribution

**Always check room location before distributing XP.** Only same-room members receive experience.

```cpp
// WRONG: Distributing to all group members
for (followData* f = master->followers; f; f = f->next) {
    if (f->follower->isAffected(AFF_GROUP)) {
        f->follower->gain_exp(amount);  // Gets XP even in different room
    }
}

// CORRECT: Validate room location
for (followData* f = master->followers; f; f = f->next) {
    if (f->follower->isAffected(AFF_GROUP) &&
        f->follower->in_room == in_room) {
        f->follower->gain_exp(amount);
    }
}
```

**Why:** Experience is spatially restricted to prevent abuse where characters park safely while group fights. The `gainExpPerHit()` function explicitly checks `in_room` for each follower.

**Never confuse money shares with experience shares.** They use completely different systems.

```cpp
// Money: Uses group_share (1-10), NPCs get 0
int money = splitShares(ch);  // Returns desc->session.group_share or 0

// Experience: Uses mob_exp(level), NPCs participate
double xp = ch->getExpShare();  // Returns mob_exp(GetMaxLevel())
```

**Why:** NPCs and pets contribute to experience distribution but receive no money. The `group_share` value only affects money via `doSplit()`, while `getExpShare()` uses level-based calculations for XP. Confusing these leads to incorrect distribution calculations.

### Follower Management

**Always check for circular chains before adding followers.** Use `circleFollow()` to prevent loops.

```cpp
// WRONG: Adding without validation
victim->addFollower(this);  // Creates A→B→A if victim already follows this

// CORRECT: Validate first
if (circleFollow(victim)) {
    sendTo("That would create a circular follow chain!\n\r");
    return;
}
victim->addFollower(this);
```

**Why:** Circular chains create infinite loops in master traversal. If A follows B and B follows A, any operation walking the master chain hangs. `circleFollow()` walks the master chain from target upward checking for self-reference.

**Never assume `addFollower()` sets AFF_GROUP.** Set the flag explicitly via `group` command.

```cpp
// WRONG: Expecting automatic group membership
target->addFollower(this);
// Character follows but gets no group benefits

// CORRECT: Set flag explicitly
target->addFollower(this);
SET_BIT(specials.affectedBy, AFF_GROUP);
// Or use: doGroup(follower_name)
```

**Why:** Following and grouping are separate concepts. `addFollower()` establishes master-follower pointer relationship only. The group command sets AFF_GROUP to enable benefit sharing. This separation allows pets and scouts to follow without receiving XP.

**Always call `reformGroup()` before deleting group leaders.** Failing to do so leaves dangling master pointers.

```cpp
// WRONG: Deleting without reform
delete leader;  // All followers now have invalid master pointer

// CORRECT: Reform first
leader->reformGroup();  // Promotes new leader, updates pointers
delete leader;
```

**Why:** When the leader dies or disconnects, all followers' master pointers reference that leader. Deleting without reform leaves dangling pointers causing crashes on next access. `reformGroup()` finds a successor and updates all follower master pointers atomically.

### Group Formation Validation

**Never allow following while mounted.** Check rider status in `doFollow()`.

```cpp
// Implementation in doFollow()
if (rider) {
    sendTo("You can't follow while mounted.\n\r");
    return FALSE;
}
```

**Why:** Mount/rider relationships create implicit grouping via `inGroup()` rider checks. Allowing mounted characters to follow creates ambiguous group hierarchies and spatial relationship conflicts.

**Always validate quest flag compatibility.** Solo quest prevents all grouping, group quest requires matching.

```cpp
// Check for solo quest
if (IS_SET(specials.act, PLR_SOLOQUEST)) {
    sendTo("You cannot group during solo quests.\n\r");
    return;
}

// Check for group quest match
if (IS_SET(specials.act, PLR_GRPQUEST) != IS_SET(target->specials.act, PLR_GRPQUEST)) {
    sendTo("Quest flags must match.\n\r");
    return;
}
```

**Why:** Solo quests require solitary completion for balance. Group quests require all members to have the flag to prevent mixing quested and non-quested characters, which could allow carrying.

**Never allow charmed followers to follow anyone except their master.** Check charm status in `doFollow()`.

```cpp
if (isAffected(AFF_CHARM) && master && victim != master) {
    act("But you only feel like following $N!", FALSE, this,
        NULL, master, TO_CHAR);
    return FALSE;
}
```

**Why:** Charmed characters are magically compelled to follow their charm master. Allowing them to switch follows breaks the charm semantic and creates exploit potential.

### Leadership and Succession

**Always clean up followers on death or disconnect.** Call `dieFollower()` to break all relationships.

```cpp
// Called automatically on death/disconnect
void TBeing::dieFollower() {
    // Stop following own master
    if (master)
        stopFollower(TRUE);

    // Stop all followers
    while (followers) {
        followers->follower->stopFollower(TRUE);
    }
}
```

**Why:** Death or disconnect invalidates all group pointers. Leaving relationships intact causes dangling references. `dieFollower()` cleans up both directions (as follower and as leader).

### Money Distribution

**Always validate descriptor existence before accessing group_share.** NPCs have no descriptor.

```cpp
// WRONG: Accessing without check
int share = ch->desc->session.group_share;  // Crash if NPC

// CORRECT: Check descriptor first
int share = 0;
if (ch->desc) {
    share = ch->desc->session.group_share;
}
```

**Why:** Only player characters have descriptors and session data. NPCs return 0 for money shares but the descriptor is NULL, causing segfault on access. Use `splitShares()` helper which handles this check.

## Reference

### Symbol Quick Reference

| Symbol | Type | Purpose |
|--------|------|---------|
| `followData` | class | Linked list node representing single follower |
| `TBeing::master` | pointer | Who this character is following (NULL if leader) |
| `TBeing::followers` | pointer | Head of followers linked list |
| `inGroup()` | function | Validates group membership between two characters |
| `addFollower()` | function | Establishes master-follower pointer relationship |
| `stopFollower()` | function | Breaks follower relationship and clears flags |
| `circleFollow()` | function | Detects circular follow chains in master tree |
| `reformGroup()` | function | Promotes new leader when current leader dies |
| `getExpShare()` | function | Returns character's base XP share (level-based) |
| `getExpSharePerc()` | function | Returns percentage of total group XP |
| `gainExpPerHit()` | function | Distributes XP to same-room group members |
| `doSplit()` | function | Divides money based on group_share factors |
| `splitShares()` | function | Returns character's money share (1-10 or 0) |
| `doFollow()` | function | Handles follow command with validation |
| `doGroup()` | function | Handles group command (display/manage/add) |
| `dieFollower()` | function | Cleanup on death/disconnect |
| `saveFollowers()` | function | Persists NPC followers to .fol files |
| `AFF_GROUP` | flag | Signals character is grouped (required for benefits) |
| `PLR_SOLOQUEST` | flag | Prevents all grouping during solo quest |
| `PLR_GRPQUEST` | flag | Requires matching flag among group members |
| `AFF_CHARM` | flag | Restricts following to charm master only |
| `AUTO_AUTOGROUP` | flag | Leader automatically groups new followers |

### stopFollower Message Modes

| Mode | Effect |
|------|--------|
| `STOP_FOLLOWER_DEFAULT` | Full messages to char, victim, notvict |
| `STOP_FOLLOWER_CHAR_VICT` | Message to follower and leader only |
| `STOP_FOLLOWER_CHAR_NOTVICT` | Message to room only |
| `STOP_FOLLOWER_SILENT` | No messages |

### Session Configuration Fields

| Field | Type | Purpose |
|-------|------|---------|
| `group_share` | byte (1-10) | Individual's money share factor |
| `groupName` | sstring | Custom group name |
| `amGroupTank` | bool | Tank flag for healer targeting |

### Leadership Succession Passes

| Pass | Criteria |
|------|----------|
| Pass 1 | Valid player with AFF_GROUP (excludes pure mobs unless original leader was mob) |
| Pass 2 | Relaxed: accepts immortals, polymorphed, linkdead |

### Key Files

| File | Contents |
|------|----------|
| `being.h` | followData class, master/followers pointers |
| `movement.cc` | doFollow() implementation |
| `spell_parser.cc` | circleFollow(), stopFollower(), addFollower(), dieFollower() |
| `combat.cc` | XP functions, reformGroup() |
| `other.cc` | Money split, group command |
| `utility.cc` | inGroup() validation |
| `rent.cc` | Follower persistence |
| `connect.h` | Session group configuration |

## Implementation

### Pointer-Based Architecture

Groups are implicit relationships emerging from pointer topology rather than explicit group objects. Each TBeing has two group-related pointers: `master` points to who the character follows (NULL for leaders), and `followers` points to the head of a linked list of followData nodes representing all followers. This forms a tree where the leader is the root with `master=NULL`, followers are children with `master` pointing to the root, and followers of followers form separate subtrees.

The followData class is a simple linked list node containing a TBeing pointer to the follower and a next pointer for list traversal. There is no maximum size enforced - groups are limited only by available memory. Each followData node is allocated on the heap when `addFollower()` is called and freed when `stopFollower()` is called.

This architecture means operations like finding all group members require traversing the followers linked list. Finding the leader requires walking master pointers upward until finding NULL. Checking membership requires comparing master pointers and validating flags. No centralized group roster exists - the distributed pointer network defines group topology.

### Group Membership Validation

The `inGroup()` function implements the authoritative membership test. It returns true if the argument character shares a group with `this` by checking several conditions in order. First, it handles identity and mount/rider relationships - a character is always in group with itself and with its mount or rider. Second, both characters must have AFF_GROUP set or the check fails immediately. Third, it recursively checks if the rider is in group to handle mounted combat. Fourth, it validates pointer relationships: true if this is the other's master, the other is this's master, or both share the same non-NULL master (sibling followers).

This function is critical because it enforces the two-tier depth limit. Followers of followers have different masters, so they fail the shared-master check and return false. The AFF_GROUP requirement prevents automatic grouping on follow - the flag must be explicitly set. Mount/rider special handling allows mounted combat without explicit grouping.

### Follower Addition Flow

When `doFollow()` processes the follow command, it performs extensive validation before calling `addFollower()`. It checks if the character is mounted (riders cannot follow), quest flags match, charm status allows following this target, and `circleFollow()` detects no loops. Only after all validations pass does it call `target->addFollower(this)`.

`addFollower()` first checks for error conditions - if the follower already has a master, it logs a bug and forcibly clears the master pointer. It then sets the follower's master pointer to `this`, allocates a new followData node, and appends it to the tail of the followers linked list by traversing to the end. Messages are sent to the follower, the new master, and observers. Finally, if the leader has AUTO_AUTOGROUP set, it automatically calls `doGroup()` to set the AFF_GROUP flag.

The separation between following and grouping is intentional. Following establishes spatial relationship (followers typically stay in same room as master through automatic movement), while grouping enables benefit sharing. This allows pets, scouts, and temporary followers to follow without receiving XP.

### Circular Chain Prevention

`circleFollow()` detects circular reference by walking the master chain from the prospective follower upward. Starting at the victim, it iterates through each master pointer (`k = k->master`) until hitting NULL or finding `this`. If it finds `this` in the chain, adding the follower would create a loop (e.g., A wants to follow C, but C's master is B and B's master is A).

Circular chains cause infinite loops in any code traversing master pointers. Functions like `reformGroup()`, XP distribution, and group display would hang. The check is O(n) in group depth but groups are shallow (typically 2 tiers), making this acceptable.

### Leadership Succession Mechanics

`reformGroup()` implements a two-pass algorithm for finding a new leader when the current leader dies or disconnects. Pass 1 iterates through the followers linked list looking for the first valid successor: must have AFF_GROUP set, must be in group via `inGroup()` check, must not be a pure mob (unless the original leader was a mob), must not be immortal/polymorphed/linkdead.

When a valid successor is found, it is removed from the followers list via `stopFollower(FALSE)` with silent message mode, its master pointer is set to NULL (making it the new leader), and all remaining followers are transferred to the new leader by updating their master pointers and moving their followData nodes to the new leader's followers list. Messages announce the leadership change to all members.

If Pass 1 finds no suitable leader, Pass 2 relaxes restrictions and accepts immortals, polymorphed characters, and linkdead players. If Pass 2 also fails, the group dissolves with each follower calling `stopFollower()`.

This automatic succession prevents group dissolution on leader death, maintaining cooperative gameplay continuity. The preference for players over mobs reflects the social nature of groups.

### Experience Share Calculation

Each character's experience share is determined by `getExpShare()` which returns `mob_exp(GetMaxLevel())`. The `mob_exp()` function calculates the base XP value a mob of that level would give, creating level-based weighting. Higher-level characters have larger `mob_exp()` values and therefore larger shares.

`getExpSharePerc()` calculates the percentage by summing total shares for the entire group (leader plus all followers with AFF_GROUP), then dividing the character's individual share by total shares and multiplying by 100. This percentage is shown in the group display so players can see their relative contribution.

The formula means a level 20 character in a group with level 10 and level 15 characters receives more absolute XP than the lower-level members, but the percentage reflects their proportional contribution. This rewards higher-level participation while still distributing to all members.

### Experience Distribution During Combat

`gainExpPerHit()` is called each time damage is dealt during combat. It distributes experience only to group members in the same room as the victim. First, it checks for AFFECT_COMBAT with COMBAT_RESTRICT_XP modifier - if present and the attacker is not in the affect owner's group, no XP is awarded (used by quest mobs).

Next, it calculates total shares by iterating followers, checking each for AFF_GROUP and same-room location via `in_room == in_room` comparison. Only followers meeting both criteria contribute to total shares. The master's share is always included if the attacker is a follower.

Per-character distribution uses the formula: `exp_per_level = total_exp / total_shares`, then `char_exp = exp_per_level * char_share * FRACT(ch, victim)`. The FRACT macro applies trophy-based reduction based on how many times the character has killed this mob type. This means each group member has an independent trophy modifier - repeated farmers receive less XP while first-timers receive full value.

Finally, the calculated experience is applied to the master (if present) and each in-room follower with AFF_GROUP via calls to individual experience gain functions.

### Money Distribution System

Money splitting uses a different share system than experience. The `splitShares()` helper returns `desc->session.group_share` for players (value 1-10) or 0 for NPCs (no descriptor). This value is set by the group leader via `group share <player> <1-10>` and defaults to a mid-range value on character creation.

`doSplit()` parses the amount to split and iterates all group members in the same room, summing their `splitShares()` values to get total shares. Each member receives `(total_amount * member_shares) / total_shares` talens via `giveMoney()` calls. NPCs receive zero because their share is always zero.

This separation from XP distribution allows leaders to reward members differently for loot (e.g., giving scouts or supports lower shares while combat-heavy characters get more), while XP remains purely level-based. The configurable nature supports various loot distribution philosophies.

### Group Restrictions and Validations

Quest flags impose strict grouping restrictions. PLR_SOLOQUEST prevents both following and being followed - the character cannot join any group. PLR_GRPQUEST requires all group members to have matching flag state - either all have it or none do. These checks occur in `doFollow()` before establishing relationships.

Charmed characters via AFF_CHARM can only follow their charm master. Attempting to follow anyone else fails with a message about only wanting to follow the master. This prevents charm exploits where victims could be redirected to follow someone else.

Mounted characters cannot follow anyone - the `rider` check in `doFollow()` blocks this. The inverse restriction (cannot group with mount) is enforced in `doGroup()` by skipping mounts during mass-add operations. Mount/rider relationships create implicit grouping via `inGroup()` checks, making explicit following problematic.

ACT_IMMORTAL mobs with `master == this` refuse grouping in `doGroup()`, preventing certain special mobs from receiving group benefits even if followed.

### Polymorph Handling

When a player polymorphs into a mob form, the group relationship transfers to the mob. The player's original TBeing is stored in `desc->original`, and the new mob form takes over leadership via `addFollower()` calls that update follower master pointers. Group benefits continue to apply to the mob form.

On returning from polymorph via `doReturn()`, the reverse happens - the mob's followers transfer back to the player's original form, and master pointers update. The group persists transparently through transformation because the descriptor maintains continuity and the pointer updates are atomic.

### Persistence and Cleanup

On rent or quit, `saveFollowers()` iterates the followers list and saves only NPC followers to `.fol` files in the player's rent directory. Player followers are not saved because they maintain independent persistence. Before saving each NPC, affects are stripped via `clearAffects()` to prevent doubling on reload, then the mob data is serialized, then affects are restored.

On load, `loadFollowers()` reads the `.fol` file, recreates each saved NPC via mob loading functions, and calls `addFollower()` to reestablish the master-follower relationship. The AFF_GROUP flag is restored from saved state.

On disconnect or death, `dieFollower()` performs complete cleanup. If the character has a master, it calls `stopFollower(TRUE)` to remove itself from the master's followers list and clear its master pointer. Then it iterates its own followers list, calling `stopFollower(TRUE)` on each to break their following relationships. This cascading cleanup prevents dangling pointers on both sides of the relationship.

`stopFollower()` handles special cases for pets - charmed followers are converted to AFFECT_ORPHAN_PET for potential retrieval via retrain system. It sends messages based on the stopFollowerT enum parameter, finds the follower's followData node in the master's list and removes it, clears the master pointer, and removes AFF_CHARM and AFF_GROUP flags.

## Troubleshooting

### Symptom: Follower receives no XP despite being in group

**Likely cause:** AFF_GROUP flag not set, or character in different room.

**Diagnostic approach:** Check `isAffected(AFF_GROUP)` return value for the follower. Verify `in_room` matches between follower and victim location. Use group display to confirm membership and percentages.

**Fix:** If AFF_GROUP missing, leader must execute `group <follower_name>` to set flag. If room mismatch, follower must move to combat room before next damage tick.

### Symptom: Nested follower (follower's follower) gets XP

**Likely cause:** Not validating shared master in `inGroup()` check, or incorrectly iterating followers of followers.

**Diagnostic approach:** Print master pointers for both characters - if different and neither is the other's master, they should not be grouped. Check if XP distribution code walks followers recursively instead of checking only direct followers.

**Fix:** Use `inGroup()` for all membership tests. Never iterate followers of followers assuming transitive membership. Only the leader's direct followers list matters.

### Symptom: Circular follow chain created, game hangs

**Likely cause:** Failed to call `circleFollow()` before `addFollower()`, or manual pointer manipulation bypassing validation.

**Diagnostic approach:** Trace follow command execution - check if `doFollow()` reached `circleFollow()` call. Examine master chain from each involved character walking upward - if it loops back to start, chain is circular.

**Fix:** Always call `circleFollow()` before establishing any follow relationship. Never manually set master/followers pointers outside of `addFollower()`/`stopFollower()`.

### Symptom: Group disbands on leader death instead of reforming

**Likely cause:** No valid successors found in either pass of `reformGroup()`, or function not called at all.

**Diagnostic approach:** Add logging to `reformGroup()` showing which followers are considered and why they are rejected. Check if all followers are mobs (rejected in Pass 1 unless leader was mob). Verify `reformGroup()` is called before leader deletion.

**Fix:** Ensure at least one player follower with AFF_GROUP exists. If all followers are mobs/NPCs, promote one manually before leader dies. Verify combat death handling calls `reformGroup()` before deletion.

### Symptom: NPC followers receive money from splits

**Likely cause:** `splitShares()` not checking descriptor existence, or manual money distribution bypassing share calculation.

**Diagnostic approach:** Check if NPCs have `desc != NULL` (they should not). Examine split distribution code - verify it uses `splitShares()` helper. Log share values during distribution.

**Fix:** Always use `splitShares()` helper which returns 0 for NPCs (no descriptor). Never directly access `desc->session.group_share` without descriptor check.

### Symptom: Follower master pointer is NULL but character still appears in group

**Likely cause:** Stale followData node in master's followers list, or incorrect cleanup in `stopFollower()`.

**Diagnostic approach:** Iterate leader's followers list and check each follower's master pointer - should all point back to leader. If any are NULL, follower list is corrupted. Check if `stopFollower()` properly removes followData node before clearing master pointer.

**Fix:** Ensure `stopFollower()` removes from followers list before clearing master pointer. Never manually clear master pointer without removing from list. Add validation to detect and log this state.

### Symptom: Group share percentages do not sum to 100%

**Likely cause:** `getExpSharePerc()` summing different set of members than XP distribution uses (e.g., including out-of-room members in total).

**Diagnostic approach:** Compare follower iteration in `getExpSharePerc()` versus `gainExpPerHit()`. Check if percentage calculation includes all followers while distribution only includes in-room followers.

**Fix:** Both functions should iterate identically - leader plus followers with AFF_GROUP. If percentages include out-of-room members, they will not match actual distribution. Document that percentages show potential shares, not actual shares if members are split.

### Symptom: Cannot follow target, "circular chain" error but no obvious loop

**Likely cause:** Target's master chain leads back to follower through intermediate masters.

**Diagnostic approach:** Manually walk master chain from target: target->master->master->master... until NULL. If you encounter the would-be follower anywhere in chain, it is circular. Check for A→B→C→A patterns with 3+ characters.

**Fix:** One character in the chain must `stop` following to break the chain before new follow relationship can form. Educate players that follow chains cannot loop.

### Symptom: Charmed follower follows wrong master

**Likely cause:** Charm affect added but master pointer points to different character, or charm cleared without removing follower relationship.

**Diagnostic approach:** Check `isAffected(AFF_CHARM)` and compare master pointer to expected charm caster. Verify charm affect creation sets master pointer atomically.

**Fix:** When applying charm, call `stopFollower()` to clear any existing master, then set master to caster and `addFollower()`. When charm expires, call `stopFollower()` to break relationship.

### Symptom: Follower saved to .fol file is not PC but should be

**Likely cause:** `isSaveMob()` returning incorrect value, or mob/PC distinction corrupted.

**Diagnostic approach:** Check `isPc()` versus `isSaveMob()` return values for the follower. Examine character flags - PC followers should not be saved to .fol. Only NPC pets/charms save.

**Fix:** Verify `saveFollowers()` only iterates saveable mobs via `isSaveMob()` check. Player followers persist independently via rent system.
