---
title: Group and Party System
description: Master-follower relationships enabling cooperative gameplay through shared experience distribution and money splitting.
category: critical
keywords: [master-follower, XP distribution, money sharing, party, leadership succession]
primary_symbols:
  functions: [inGroup, addFollower, stopFollower, circleFollow, reformGroup, getExpShare, getExpSharePerc, gainExpPerHit, doSplit, splitShares, dieFollower]
  classes: [TBeing, followData, Descriptor]
  enums: [AFF_GROUP, AFF_CHARM, PLR_SOLOQUEST, PLR_GRPQUEST, AUTO_AUTOGROUP, STOP_FOLLOWER_DEFAULT, STOP_FOLLOWER_CHAR_VICT, STOP_FOLLOWER_CHAR_NOTVICT, STOP_FOLLOWER_SILENT]
---

# Group and Party System

## Overview

How do multiple players cooperate in combat and share rewards? The group system manages cooperative gameplay through master-follower relationships, enabling shared experience distribution and money splitting.

Groups in SneezyMUD are not discrete objects but implicit relationships defined by pointer chains and the `AFF_GROUP` bitvector flag. A character follows another by setting a `master` pointer; the master maintains a linked list of `followers`. This creates a tree structure with a maximum practical depth of two tiers for group membership purposes.

The critical distinction: following someone does not automatically grant group membership. The `AFF_GROUP` flag must be explicitly set via the `group` command. Without this flag, followers do not receive XP or money distribution benefits.

Groups are spatial entities. Only members physically present in the same room as combat receive experience. A level 50 character in another room contributes nothing to and receives nothing from the fight.

XP distribution is level-weighted. Higher-level characters receive proportionally larger shares based on their `mob_exp()` value. Money distribution uses a separate share factor (1-10) set by the group leader, and NPCs receive no money shares regardless of group membership.

When the group leader dies, `reformGroup()` automatically promotes a suitable successor from among the followers, transferring all remaining followers to the new leader. This prevents group dissolution on leader death.

### Common Scenarios

**Party Formation:**
- Player A types `follow B`
- Validation checks: no circular chains, matching quest flags, not mounted
- `addFollower()` sets A's `master=B` and adds A to B's followers linked list
- A still does not receive group benefits
- B types `group A` which sets A's `AFF_GROUP` flag
- Now A receives XP and can participate in splits

**Experience Distribution in Combat:**
- Group of 3 (levels 10, 15, 20) in same room fights a mob
- Each hit calls `gainExpPerHit()`
- Total shares calculated: `mob_exp(10) + mob_exp(15) + mob_exp(20)`
- Each character receives: `(total_exp / total_shares) * their_share * trophy_modifier`
- Trophy modifier penalizes repeated kills of same mob type
- If level 10 moves to different room, they stop receiving XP

**Leader Death and Succession:**
- Group leader dies in combat
- `reformGroup()` called automatically
- Pass 1: Search for first valid player follower with AFF_GROUP
- Found follower removed from list, `master` set to NULL
- All remaining followers transferred to new leader
- Messages announce leadership change
- If no players available, Pass 2 relaxes restrictions to accept immortals/polymorphed

**Money Split:**
- Leader has `group_share=5`, two followers have `3` and `2`
- Leader types `split 1000`
- Total shares: 5+3+2 = 10
- Distribution: 500, 300, 200 talens respectively
- Only characters in same room receive shares
- NPCs receive zero (no descriptor means no group_share value)

## Patterns

### Group Membership Validation

Always use `inGroup()` to check group membership. Do not rely on pointer relationships alone.

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

The `inGroup()` function validates that both characters have `AFF_GROUP` set AND share a direct master relationship (same master, one is master of the other, or mount/rider). Checking only the `master` pointer misses the flag requirement; checking only the flag misses the relationship requirement.

Never assume transitive group membership. If A follows B and B follows C, A is NOT in a group with C. They have different masters. The group system is explicitly two-tier: leader and direct followers only.

```cpp
// WRONG: Assuming nested followers are grouped
Leader (master=NULL)
  +-- Follower1 (master=Leader, AFF_GROUP)
      +-- Follower2 (master=Follower1, AFF_GROUP)

// Follower2 is NOT in Leader's group - different masters
if (Follower2->inGroup(*Leader)) {  // Returns FALSE
    // Never executes
}
```

### Following and Grouping

Always check `circleFollow()` before calling `addFollower()`. Circular follow chains (A follows B follows C follows A) corrupt the master-follower tree and cause infinite loops during traversal.

```cpp
// WRONG: Adding without validation
victim->addFollower(this);  // Creates A->B->A if victim already follows this

// CORRECT: Validate first
if (circleFollow(victim)) {
    sendTo("That would create a circular follow chain!\n\r");
    return;
}
victim->addFollower(this);
```

Never assume `addFollower()` sets `AFF_GROUP`. It does not. The follow command establishes the master-follower relationship; the group command sets the flag. Without the flag, no benefits apply.

Always set `AFF_GROUP` explicitly via the group command or by setting the bitvector directly after establishing the follow relationship.

### XP Distribution

Always check both `AFF_GROUP` flag AND same-room location before distributing XP. Characters in different rooms are excluded from combat XP regardless of group status.

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

Never confuse money shares with XP shares. Money uses `group_share` (1-10 factor set by leader); XP uses `getExpShare()` (level-based `mob_exp()` value). NPCs get XP shares but zero money shares.

Always include the `FRACT()` trophy modifier when distributing XP. Each character's individual trophy history affects their share independently.

### Group Cleanup

Always call `reformGroup()` before deleting a group leader. Deleting without reform leaves followers with dangling `master` pointers. The function promotes a new leader and transfers all followers.

```cpp
// WRONG: Deleting without reform
delete leader;  // All followers now have invalid master pointer

// CORRECT: Reform first
leader->reformGroup();  // Promotes new leader, updates pointers
delete leader;
```

Always call `dieFollower()` when a character dies or disconnects. This breaks all follow relationships in both directions (as follower and as leader), preventing stale pointers.

### Session Configuration

The `group_share` factor on the Descriptor affects only money distribution. It has no effect on XP. Leaders set this via `group share <player> <1-10>`.

NPCs return 0 from `splitShares(ch, k)` because they lack a descriptor. They receive no money from splits but do receive their level-based XP share.

Always validate descriptor existence before accessing group_share. NPCs have no descriptor.

```cpp
// WRONG: Accessing without check
int share = ch->desc->session.group_share;  // Crash if NPC

// CORRECT: Check descriptor first (or use splitShares() helper)
int share = splitShares(ch, k);  // Returns 0 for NPCs
```

### Group Formation Validation

Never allow following while mounted. The `rider` check in `doFollow()` blocks this because mount/rider relationships create implicit grouping via `inGroup()` checks.

Always validate quest flag compatibility. `PLR_SOLOQUEST` prevents all grouping; `PLR_GRPQUEST` requires all group members to have matching flag state.

Never allow charmed followers to follow anyone except their master. Charmed characters are magically compelled and switching follows breaks the charm semantic.

## Reference

### Symbol Quick Reference

| Symbol | Type | Purpose |
|--------|------|---------|
| `followData` | class | Linked list node containing follower pointer |
| `master` | pointer | Who this character follows (NULL for leaders) |
| `followers` | pointer | Head of follower linked list (NULL if no followers) |
| `AFF_GROUP` | flag | Required for XP/money distribution |
| `AFF_CHARM` | flag | Restricts following to charm master only |
| `PLR_SOLOQUEST` | flag | Prevents all grouping during solo quest |
| `PLR_GRPQUEST` | flag | Requires matching flag among group members |
| `AUTO_AUTOGROUP` | flag | Leader automatically groups new followers |
| `inGroup()` | function | Validates group membership between two characters |
| `addFollower()` | function | Establishes master-follower relationship |
| `stopFollower()` | function | Breaks master-follower relationship, clears flags |
| `circleFollow()` | function | Detects circular follow chains |
| `reformGroup()` | function | Promotes new leader on leader death |
| `getExpShare()` | function | Returns level-based XP share value |
| `getExpSharePerc()` | function | Returns percentage of group XP for display |
| `gainExpPerHit()` | function | Distributes combat XP to group members |
| `doSplit()` | function | Distributes money to in-room group members |
| `splitShares(ch, k)` | function | Returns money share factor (0 for NPCs); static function with signature `static int splitShares(const TBeing* ch, const TBeing* k)` |
| `dieFollower()` | function | Breaks all follow relationships on death |

### Session Configuration Fields

| Field | Type | Purpose |
|-------|------|---------|
| `group_share` | byte (1-10) | Per-session money share factor |
| `groupName` | sstring | Custom group name |
| `amGroupTank` | bool | Tank flag for healer targeting |

### Follow Validation Checks

| Check | Function | Consequence of Skipping |
|-------|----------|------------------------|
| Circular chain | `circleFollow()` | Infinite loops in master chain traversal |
| Already has master | Check `foll->master` | Orphaned followData nodes |
| Charmed | `AFF_CHARM` check | Charmed followers can only follow their master |
| Quest flags | `PLR_SOLOQUEST`, `PLR_GRPQUEST` | Quest interference |
| Mounted | `riding` check | State conflicts |

### stopFollowerT Message Options

| Constant | Behavior |
|----------|----------|
| `STOP_FOLLOWER_DEFAULT` | Messages to char, victim, and room |
| `STOP_FOLLOWER_CHAR_VICT` | Messages to follower and leader only |
| `STOP_FOLLOWER_CHAR_NOTVICT` | Messages to room only |
| `STOP_FOLLOWER_SILENT` | No messages |

### Leadership Succession Passes

| Pass | Criteria |
|------|----------|
| Pass 1 | Valid player with AFF_GROUP (excludes pure mobs unless original leader was mob) |
| Pass 2 | Relaxed: accepts immortals, polymorphed, linkdead |

### Group Commands

| Command | Access | Purpose |
|---------|--------|---------|
| `group` | All | List group members and shares |
| `group <player>` | Leader | Add follower to group |
| `group all` | Leader | Mass-add all followers |
| `group name <name>` | Leader | Set custom group name |
| `group share <player> <1-10>` | Leader | Set money share factor |
| `group lots` | Leader | Random selection among members |
| `group amtank` | All | Flag self as tank |
| `group seeksgroup` | All | Mark as seeking group in who list |
| `follow <target>` | All | Establish follower relationship |
| `stop` | All | Break follower relationship |
| `split <amount>` | All | Distribute money to group |

### Key Files

| File | Contents |
|------|----------|
| `being.h` | followData class, master/followers pointers |
| `movement.cc` | `doFollow()` implementation |
| `spell_parser.cc` | `circleFollow()`, `stopFollower()`, `addFollower()`, `dieFollower()` |
| `combat.cc` | `getExpShare()`, `getExpSharePerc()`, `gainExpPerHit()`, `reformGroup()` |
| `other.cc` | `splitShares()`, `doSplit()`, `doGroup()`, `doStop()` |
| `utility.cc` | `inGroup()` validation |
| `rent.cc` | `removeFollowers()`, `saveFollowers()` |
| `connect.h` | Session `group_share`, `groupName`, `amGroupTank` |

## Implementation

### Data Structures

The `followData` class is a simple linked list node containing a `follower` pointer to a TBeing and a `next` pointer to the next node. The class provides copy constructor, assignment operator, and destructor for memory management. No maximum follower count is enforced; the list grows as needed.

TBeing contains two group-related pointers: `master` (the character being followed, NULL for leaders) and `followers` (head of the followData linked list, NULL if no followers). This creates a tree structure where each node has one parent and arbitrarily many children.

The Descriptor class holds session-specific group configuration: `group_share` (1-10 money distribution factor), `groupName` (custom group name string), and `amGroupTank` (tank flag for healer targeting).

### Group Membership Validation

The `inGroup()` function on TBeing performs multi-step validation. First, it returns true for identity (same character) or mount/rider relationships. Second, it requires both characters to have `AFF_GROUP` set. Third, it recursively checks the rider chain. Finally, it validates the master relationship: one must be the other's master, or both must share the same master (siblings in the tree).

The function returns false for transitive relationships. If A follows B and B follows C, `A->inGroup(C)` returns false because they do not share the same master nor is one the master of the other.

### Follow Establishment

The `doFollow()` function validates the request before establishing the relationship. It rejects following while mounted (rider set), following characters with `PLR_SOLOQUEST`, following while having `PLR_SOLOQUEST`, mismatched `PLR_GRPQUEST` flags, and circular chains detected by `circleFollow()`. Charmed characters can only follow their charm master.

The `addFollower()` function receives the follower as a parameter. It first checks if the follower already has a master and logs an error if so, force-clearing the pointer. It sets the follower's `master` pointer to `this`, creates a new followData node, and appends it to the tail of the followers linked list. Messages are sent to follower, master, and room. If the master has `AUTO_AUTOGROUP` set, it automatically calls `doGroup()` to add the follower to the group.

The `circleFollow()` function walks the master chain from the proposed target upward. If it encounters `this` in that chain, adding this follower would create a circular reference. The function returns true to indicate the follow should be blocked.

### Follow Termination

The `stopFollower()` function handles relationship cleanup. For charmed pets and thralls, it converts the mob to `AFFECT_ORPHAN_PET` status for potential retraining. It sends messages based on the stopFollowerT parameter. It finds and removes the follower from the master's linked list, deleting the followData node. It clears the follower's master pointer and removes both `AFF_CHARM` and `AFF_GROUP` flags.

The `dieFollower()` function provides complete cleanup on character death or disconnect. It first calls `stopFollower()` if the character has a master. Then it iterates through the followers list, calling `stopFollower()` on each, until the list is empty.

### Leadership Succession

The `reformGroup()` function handles automatic leadership transfer when the leader dies. It uses a two-pass algorithm.

Pass 1 applies strict criteria: iterate through followers, skip pure mobs (unless the original leader was a mob), skip those not passing `inGroup()` validation. The first valid follower becomes the new leader via `stopFollower()` to remove from the old list, setting their `master` to NULL, and transferring all remaining followers to them via `addFollower()`.

Pass 2 relaxes restrictions if Pass 1 found no suitable leader. It accepts immortals, polymorphed characters, and linkdead players. The first acceptable follower becomes leader and remaining followers transfer.

Messages inform the new leader ("Group leader has died and you have taken over") and other members ("Group leader has died and [name] has taken over").

### XP Distribution

The `getExpShare()` function returns `mob_exp(GetMaxLevel())` - the base mob experience value for the character's maximum level. Higher-level characters have proportionally larger shares. This is used for both calculating individual shares and computing percentage displays.

The `getExpSharePerc()` function calculates what percentage of total group XP this character receives. It sums the leader's share plus all AFF_GROUP-flagged followers' shares, then returns this character's share divided by the total, times 100.

The `gainExpPerHit()` function distributes combat experience. It enforces spatial requirements: only characters in the same room as the victim receive XP. It checks for `AFFECT_COMBAT` with `COMBAT_RESTRICT_XP` which can prevent XP if the attacker is not in the affect owner's group.

The distribution calculates total shares from in-room group members with `AFF_GROUP`, then iterates through those members giving each their proportional share. The `FRACT()` trophy modifier applies individually to each character, reducing XP for mob types they have killed many times.

### Money Distribution

The `splitShares()` static function takes two parameters `(const TBeing* ch, const TBeing* k)` and returns the character's money share factor. For characters with a descriptor (players), it returns `desc->session.group_share`. For NPCs without descriptors, it returns 0. This excludes NPCs from money splits entirely.

The `doSplit()` function divides money among in-room group members. It sums total shares from all qualifying members, then distributes proportionally via `addToMoney()` with the `GOLD_XFER` flag. The calculation divides total amount by total shares, then multiplies by each member's individual share factor.

### Group Persistence

The `saveFollowers()` function on TPerson saves NPC followers to `.fol` files on rent/logout. It iterates the followers list, checking `isSaveMob()` to filter for NPCs only. It strips affects before saving to prevent doubling on reload, writes mob data, then reapplies affects.

The `loadFollowers()` function restores saved followers on character load, calling `addFollower()` for each mob.

The `removeFollowers()` function handles cleanup on logout, deleting follower files and removing the character from any master's follower list via `dieFollower()`.

### Quest Flag Handling

`PLR_SOLOQUEST` completely prevents group formation. Characters with this flag cannot follow anyone and cannot be followed. The check occurs in `doFollow()` before any relationship is established.

`PLR_GRPQUEST` requires matching flags. A follower with the flag can only join a group where the leader also has the flag. This prevents mixed quest/non-quest groups that could interfere with quest mechanics.

### Special Cases

Charmed characters (`AFF_CHARM`) can only follow their charm master. Attempting to follow a different character while charmed produces the message "But you only feel like following [master]!" and fails.

Mounted characters cannot be grouped with their mounts. The `doGroup()` function explicitly skips the `riding` target.

`ACT_IMMORTAL` mobs that follow a player refuse grouping. The check in `doGroup()` skips them silently.

Groups survive polymorph transformation. The original character is stored as `desc->original`, the mob form takes the group relationships via `addFollower()`, and relationships restore automatically on return to original form.

## Troubleshooting

### Followers Not Receiving XP

**Symptom:** Group members report receiving no XP from combat despite being in the party.

**Likely cause:** Missing `AFF_GROUP` flag, or follower is in a different room.

**Diagnostic approach:** Check the character's bitvector for `AFF_GROUP`. Verify `in_room` matches the combat location. Use the `group` command to see membership status.

**Fix:** Ensure the `group` command was used after `follow`. Verify physical presence in the combat room.

### Nested Follower Gets XP Incorrectly

**Symptom:** A follower's follower receives XP when they should not.

**Likely cause:** XP distribution code walks followers recursively instead of checking only direct followers.

**Diagnostic approach:** Print master pointers for both characters - if different and neither is the other's master, they should not be grouped.

**Fix:** Use `inGroup()` for all membership tests. Never iterate followers of followers assuming transitive membership. Only the leader's direct followers list matters.

### Circular Follow Chain Crashes

**Symptom:** Server hangs or crashes when processing group operations.

**Likely cause:** Circular master chain created without `circleFollow()` validation.

**Diagnostic approach:** Walk the master chain manually from the problematic character, checking for repeated entries.

**Fix:** Always call `circleFollow()` before `addFollower()`. If a circular chain exists, break it by calling `stopFollower()` on one participant.

### Dangling Master Pointer After Leader Death

**Symptom:** Followers crash or behave erratically after their leader dies.

**Likely cause:** Leader was deleted without calling `reformGroup()` first.

**Diagnostic approach:** Check if followers' `master` pointers reference freed memory.

**Fix:** Always call `reformGroup()` before deleting any character that might be a group leader. The function transfers followers to a new leader or breaks relationships cleanly if no successor exists.

### NPCs Getting Money From Splits

**Symptom:** Money splits distribute to NPCs when they should not.

**Likely cause:** Code is not using `splitShares()` which returns 0 for NPCs lacking descriptors.

**Diagnostic approach:** Verify the split calculation uses `splitShares()` for the money share factor, not `getExpShare()`.

**Fix:** Money distribution must check `splitShares()` which returns 0 for characters without descriptors. Only `getExpShare()` (for XP) includes NPCs.

### Group Members With Different Masters

**Symptom:** Characters who appear to be in the same group fail `inGroup()` checks.

**Likely cause:** Transitive following - A follows B, B follows C, but A is not in group with C.

**Diagnostic approach:** Check the `master` pointer of each character. Group membership requires sharing the same master (or one being the master of the other).

**Fix:** All characters who need to share benefits must follow the same leader directly. Reorganize the follow chain so everyone follows a single leader.

### Group Share Percentages Do Not Sum to 100%

**Symptom:** Displayed group percentages add up to more or less than 100%.

**Likely cause:** `getExpSharePerc()` sums different members than XP distribution uses (e.g., including out-of-room members in total).

**Diagnostic approach:** Compare follower iteration in `getExpSharePerc()` versus `gainExpPerHit()`.

**Fix:** Percentages show potential shares assuming all members are present. Actual distribution only includes in-room members. This is documented behavior, not a bug.

### Charmed Follower Follows Wrong Master

**Symptom:** A charmed character's master pointer differs from their charm caster.

**Likely cause:** Charm affect added but master pointer points to different character, or charm cleared without removing follower relationship.

**Diagnostic approach:** Check `isAffected(AFF_CHARM)` and compare master pointer to expected charm caster.

**Fix:** When applying charm, call `stopFollower()` to clear any existing master, then set master to caster and `addFollower()`. When charm expires, call `stopFollower()` to break relationship.

### Stale FollowData Node After StopFollower

**Symptom:** Follower's master pointer is NULL but character still appears in group list.

**Likely cause:** `stopFollower()` did not properly remove followData node from master's followers list before clearing master pointer.

**Diagnostic approach:** Iterate leader's followers list and check each follower's master pointer - should all point back to leader.

**Fix:** Ensure `stopFollower()` removes from followers list before clearing master pointer. Never manually clear master pointer without removing from list.
