---
title: Combat Round Timing and Structure
description: Distributed attack scheduling across 1.2-second rounds using global combat list iteration, gCombatNext iterator caching, DELETE flag propagation, and the reconcileDamage -1 sentinel value.
keywords: [perform_violence, gCombatList, gCombatNext, Pulse::COMBAT, hit, oneHit, setVictFighting, stopFighting, reconcileDamage, applyDamage, damageEpilog, DELETE_THIS, DELETE_VICT, next_fighting, blowCount, attackers, AFFECT_COMBAT, COMBAT_SOLO_KILL]
category: Critical Systems

last_updated: 2026-01-29
source_files: [code/code/misc/combat.cc, code/code/misc/offense.cc, code/code/misc/damage.cc, code/code/sys/comm.h, code/code/sys/process.cc]
related:
  - combat-formulas.md
  - memory-safety.md
  - position-stance.md
  - damage-pipeline.md
---

# Combat Round Timing and Structure

This document describes the combat round system, attack scheduling, combat list management, damage processing, and the critical safety patterns that prevent crashes during combat.

**Misusing this system causes crashes.** Common errors: using wrong iterator pattern (`ch->next_fighting` instead of `gCombatNext`), forgetting to call `reformGroup()` before deletion, not checking `DELETE_*` flags, continuing execution after `DELETE_THIS`, checking for `DELETE_VICT` instead of `-1` from `reconcileDamage()`.

## Overview

Combat in SneezyMUD operates on a distributed attack model where attacks are spread across a 1.2-second combat round rather than firing simultaneously. The `perform_violence()` function orchestrates all combat, iterating through the global combat list (`gCombatList`) and executing attacks at calculated intervals.

**Key Components:**
- `gCombatList` - Linked list of all fighting characters
- `gCombatNext` - Global iterator cache for safe traversal
- `Pulse::COMBAT` - 12 ticks (1.2 seconds) per round
- `perform_violence()` - Main combat loop
- Attack distribution via modulo arithmetic
- DELETE flag propagation for safe memory management

## Combat Timing

### Pulse Constants

**Source:** `code/code/sys/comm.h`

```cpp
namespace Pulse {
    const int ONE_SECOND = 10;           // 10 pulses = 1 second
    const int COMBAT = 12;               // 12 pulses = 1.2 seconds
    const int SPEC_PROCS = 36;           // 36 pulses = 3.6 seconds
}
```

| Unit | Pulses | Real Time |
|------|--------|-----------|
| 1 Pulse | 1 | 0.1 seconds |
| 1 Combat Round | 12 | 1.2 seconds |
| 1 Spec Proc Cycle | 36 | 3.6 seconds |

### Combat Round Structure

A single call to `perform_violence()` processes **all** attacks for that round by iterating `Pulse::COMBAT` times (0-11) through the combat list:

```cpp
void perform_violence(int pulse) {
    TBeing *ch, *vict;
    int rc;

    // Process entire round in one call
    for (int tmp_pulse = 0; tmp_pulse < Pulse::COMBAT; tmp_pulse++) {
        for (ch = gCombatList; ch; ch = gCombatNext) {
            gCombatNext = ch->next_fighting;  // Cache before operations

            // Get target
            if (!(vict = ch->fight())) {
                ch->stopFighting();
                continue;
            }

            // Validate combat can proceed
            if (!ch->roomp || ch == vict) {
                ch->stopFighting();
                continue;
            }

            // Execute attack if character is awake and in same room
            if (ch->awake() && ch->sameRoom(*vict)) {
                rc = ch->hit(vict, pulse + tmp_pulse);

                // Handle deletion flags
                if (IS_SET_DELETE(rc, DELETE_VICT)) {
                    vict->reformGroup();
                    delete vict;
                    vict = NULL;
                    continue;
                } else if (IS_SET_DELETE(rc, DELETE_THIS)) {
                    ch->reformGroup();
                    delete ch;
                    ch = NULL;
                    break;  // Can't continue with deleted attacker
                }
            }
        }
    }
}
```

**Source:** `code/code/misc/combat.cc`

### Why the Inner Loop?

The nested loop design (12 iterations per `perform_violence()` call) implements **distributed attack scheduling**:

**Without inner loop (true roundless combat):**
- All attacks fire simultaneously
- Massive damage spikes
- No opportunity to flee between attacks
- Spammy and confusing (per code comments)

**With inner loop:**
- Attacks spread across 1.2-second window
- More tactical gameplay (can flee between attacks)
- Prevents simultaneous burst damage
- Better balance for multi-attack characters

The `tmp_pulse` offset (0-11) affects when attacks fire via modulo arithmetic in attack distribution calculations.

## gCombatNext Global Iterator

### The Problem

When a character is deleted or stops fighting during `perform_violence()`, the combat list is modified mid-iteration. Using a local iterator leads to crashes:

```cpp
// CRASH: Local iterator becomes invalid on deletion
for (ch = gCombatList; ch; ch = ch->next_fighting) {
    rc = ch->hit(vict, pulse);
    if (IS_SET_DELETE(rc, DELETE_THIS)) {
        delete ch;  // ch->next_fighting is now garbage!
    }
    // Loop advances to freed memory - CRASH
}
```

### The Solution: Global Iterator Cache

**Declaration:**
```cpp
TBeing* gCombatList = NULL;   // Head of combat list
TBeing* gCombatNext = NULL;   // Global iterator cache
```

**Source:** `code/code/misc/combat.cc`

**Safe Pattern:**
```cpp
// CORRECT: Cache next pointer BEFORE operations
for (ch = gCombatList; ch; ch = gCombatNext) {
    gCombatNext = ch->next_fighting;  // Cache FIRST - critical step

    // Operations that might delete ch or modify list
    rc = ch->hit(vict, pulse + tmp_pulse);
    if (IS_SET_DELETE(rc, DELETE_THIS)) {
        ch->reformGroup();
        delete ch;
        ch = NULL;
        break;
    }
}
```

### stopFighting() Adjustment

When removing a character from the combat list, `stopFighting()` must check and update `gCombatNext`:

```cpp
void TBeing::stopFighting() {
    // ... validation ...

    // CRITICAL: Adjust global iterator if we're removing the cached pointer
    if (gCombatNext == this)
        gCombatNext = next_fighting;

    // Remove from linked list
    if (gCombatList == this) {
        gCombatList = next_fighting;
    } else {
        // Search and unlink
        for (tmp = gCombatList; tmp && (tmp->next_fighting != this);
             tmp = tmp->next_fighting)
            ;
        if (!tmp) {
            vlogf(LOG_COMBAT, "Char fighting not found Error - ABORT");
            abort();  // Fatal: list corruption detected
        }
        tmp->next_fighting = next_fighting;
    }

    // ... continue cleanup ...
}
```

**Source:** `code/code/misc/combat.cc`

### Why Global Instead of Local?

From the code comments (Bat 9/1/98):

> "So WTF are we doing here? glad you asked. GCN maintains the next pointer for the perform_violence loop. Imagine if while I am fighting, I cause the GCN guy to stop fighting. If GCN was local to perform_violence, this would cause it to walk onto a player no longer fighting (bad in itself), plus lose track of later fighters (really bad). So we maintain this globally, and make some checks for GCN in stopFighting, adjusting as appropriate."

A local iterator cannot be adjusted by functions called during iteration. The global variable allows `stopFighting()` to fix the iterator when removing the cached pointer.

## Attack Distribution

### Attack Scheduling Calculation

Each character gets a calculated number of attacks per round (fx for primary hand, fy for secondary). These **float values** are distributed across the combat round using modulo arithmetic:

```cpp
int TBeing::hit(TBeing* victim, int pulse) {
    float fx, fy;
    int len_rnd = Pulse::COMBAT;  // 12 ticks
    int hit_wait;

    // Get attack counts for this round
    blowCount(FALSE, fx, fy);

    // Distribute primary hand attacks
    if (fx) {
        hit_wait = (int)(len_rnd * 10 / fx);
        fx = ((pulse * 10 % hit_wait) < 10) ? 1.0 : 0.0;
    }

    // Distribute secondary hand attacks (phase-shifted)
    if (fy) {
        hit_wait = (int)(len_rnd * 10 / fy);
        fy = ((((pulse * 10) + (hit_wait / 2)) % hit_wait) < 10) ? 1.0 : 0.0;
    }

    // Execute attacks when distribution fires
    while (fx > 0.999) {
        rc = oneHit(victim, HAND_PRIMARY, prim, attackMod, &fx);
        // ... handle DELETE flags ...
    }

    while (fy > 0.999) {
        rc = oneHit(victim, HAND_SECONDARY, sec, attackMod, &fy);
        // ... handle DELETE flags ...
    }

    return rc;
}
```

**Source:** `code/code/misc/combat.cc`

### Distribution Examples

**2 attacks per round (fx = 2.0):**
```
hit_wait = (12 * 10) / 2 = 60
Fires when: (pulse * 10) % 60 < 10
Result: Attacks at ~pulses 0, 6
```

**3.5 attacks per round (fx = 3.5):**
```
hit_wait = (12 * 10) / 3.5 ~ 34
Fires when: (pulse * 10) % 34 < 10
Result: Attacks at ~pulses 0, 3, 6, 10
```

**Phase-Shifted Secondary Hand:**

The secondary hand adds `(hit_wait / 2)` to the pulse value, staggering attacks:
```cpp
fy = ((((pulse * 10) + (hit_wait / 2)) % hit_wait) < 10) ? 1.0 : 0.0;
```

This prevents primary and secondary attacks from firing simultaneously when they would otherwise sync up.

### Why Modulo Arithmetic?

- **Pulse-dependent timing:** Attack timing varies with absolute pulse count (server uptime), adding variation
- **Distributed execution:** Attacks fire throughout the round, not all at once
- **Fractional attacks:** Supports attack values like 2.5, 3.33, etc.
- **No synchronization:** Characters with same attack count don't all fire together

## Multi-Attack Handling

### blowCount() Function

**Signature:**
```cpp
void TBeing::blowCount(bool check, float& fx, float& fy) const;
```

**Purpose:** Calculate how many attacks per round for primary (fx) and secondary (fy) hands.

**Source:** `code/code/misc/offense.cc`

### NPC Attack Calculation

```cpp
if (!isPc()) {
    num = min(12.0, getMult());  // MOBs: up to 12 attacks

    // Split between hands
    if (isPaired()) {
        num /= 2.0;  // Paired weapons get half each
        fx = 0.60 * num;  // 60% primary
        fy = 0.40 * num;  // 40% secondary
    } else {
        fx = num;  // All attacks with primary weapon
        fy = 0.0;
    }
}
```

**MOB Attack Distribution:**

| getMult() | Primary (fx) | Secondary (fy) | Total |
|-----------|--------------|----------------|-------|
| 1.0 | 0.60 | 0.40 | 1.0 |
| 2.0 | 1.20 | 0.80 | 2.0 |
| 5.0 | 3.00 | 2.00 | 5.0 |
| 12.0 | 7.20 | 4.80 | 12.0 |
| 15.0 | 7.20 | 4.80 | 12.0 (capped) |

### PC Attack Calculation

```cpp
if (isPc()) {
    num = getMult();  // Base monk barehand attacks
    fx = fy = 0;

    // Primary hand
    if (!prim && hasClass(CLASS_MONK)) {
        fx += (0.60 * num);  // Monk barehand
    } else if (prim) {
        fx = prim->blowCountSplitter(this, true);  // Weapon check
        fx += prim->specializationCheck(this);      // Specialization bonus
        if (fx > 0.0)
            fx *= getStatMod(STAT_SPE);  // Speed stat multiplier
    } else {
        fx = 1.0;  // Default barehand
    }

    // Secondary hand (similar logic)
    if (!sec && hasClass(CLASS_MONK)) {
        fy += (0.40 * num);
    } else if (sec) {
        fy = sec->blowCountSplitter(this, false);
        fy += sec->specializationCheck(this);
        if (fy > 0.0)
            fy *= getStatMod(STAT_SPE);
    }

    // Combat mode modifiers
    if (isCombatMode(ATTACK_BERSERK) && getPosition() >= POSITION_STANDING) {
        if (fx > 0.0) fx += 0.5;
        if (fy > 0.0) fy += 0.5;

        if (doesKnowSkill(SKILL_ADVANCED_BERSERKING)) {
            if (fx > 0.0) fx += 0.5;  // Additional +0.5 per hand
            if (fy > 0.0) fy += 0.5;
        }
    }
}
```

### Attack Modifiers Summary

| Modifier | Primary (fx) | Secondary (fy) | Notes |
|----------|--------------|----------------|-------|
| Berserk | +0.5 | +0.5 | Must be standing |
| Advanced Berserking | +0.5 | +0.5 | Stacks with Berserk |
| Haste spell | +0.5 | +0.5 | Magical speed |
| Speed stat (SPE) | x modifier | x modifier | plotStat(0.8, 1.25) |
| Weapon specialization | +bonus | +bonus | Varies by weapon type |
| Blur (monk) | x 2 | x 2 | One round only |
| Riposte | +1 | - | One extra attack, removed after |
| Focus Attack | +1 | - | One extra attack |

### Obstacles and Penalties

**Equipment in hands:**
- Shield in hand: -1 attack from that hand
- Non-weapon item held: -1 attack from that hand

**Mounted:**
- Attack count x 0.67 (reduced by one-third)

## Damage Pipeline

Once attacks land, damage flows through a three-function pipeline: `reconcileDamage()` -> `applyDamage()` -> `damageEpilog()`. Understanding this pipeline is critical for avoiding crashes and correctly handling character death.

### Pipeline Flow

```
reconcileDamage()          Entry point for all damage
    |
    +-> checkSpec()        Trigger combat-start specials
    +-> getActualDamage()  Apply resistances/modifiers
    +-> applyDamage()      Core damage application
    |       |
    |       +-> doDamage()      Actually subtract HP
    |       +-> tellStatus()    Death messages, crash landings
    |       +-> damageEpilog()  Post-damage effects
    |
    +-> return -1 on death, else damage dealt
```

### Critical: The `-1` Return Value

**`reconcileDamage()` returns `-1` when the victim dies, NOT `DELETE_VICT`.**

This is a magic sentinel value that must be checked with `== -1`, not `IS_SET_DELETE()`:

```cpp
// CORRECT: Check for -1
int rc = attacker->reconcileDamage(victim, damage, DAMAGE_TYPE);
if (rc == -1)
    return DELETE_VICT;  // Translate to DELETE flag for caller

// CRASH: Using IS_SET_DELETE won't detect -1
if (IS_SET_DELETE(rc, DELETE_VICT)) { ... }  // WRONG - never triggers
```

The `-1` value is chosen because damage amounts are always non-negative, so `-1` is an unambiguous death signal. See `damage.cc`.

### reconcileDamage() (`damage.cc`)

Entry point that orchestrates damage dealing:

1. Triggers `CMD_MOB_COMBAT_ONATTACK` / `CMD_MOB_COMBAT_ONATTACKED` specials
2. Sets aggressor flags for PvP tracking
3. Calls `getActualDamage()` to apply resistances
4. Makes flying mobs take flight if appropriate
5. Logs damage for statistics
6. Develops monster hatred toward attacker
7. Calls `applyDamage()` to actually deal damage
8. **Returns `-1` if victim died, otherwise returns the damage dealt**

### applyDamage() (`damage.cc`)

Core damage processing that handles death logic:

1. Sets combatants fighting if not already
2. Applies protection (sanctuary, etc.)
3. **Quest mob solo-kill detection**
4. Caps overkill damage: `dam = 11 + v->getHit()`
5. Awards trophy and XP
6. Calls `doDamage()` to subtract HP
7. Calls `tellStatus()` for death messages
8. Calls `damageEpilog()` for post-damage effects
9. **Returns `DELETE_VICT` if victim died, `TRUE` otherwise**

### damageEpilog() (`damage.cc`)

Post-damage cleanup: removes transfix, reveals invisible attackers, handles mount fall-off, triggers `CMD_RESP_KILLED`, creates mob loot on death, handles PK aftermath.

### Return Value Summary

| Function | On Death | On Survival |
|----------|----------|-------------|
| `reconcileDamage()` | `-1` | Damage dealt (>= 0) |
| `applyDamage()` | `DELETE_VICT` | `TRUE` |
| `damageEpilog()` | `DELETE_VICT` | `FALSE` |

### Safe Pattern: Calling reconcileDamage

```cpp
int rc = attacker->reconcileDamage(victim, damage, SKILL_FIREBALL);
if (rc == -1)
    return DELETE_VICT;  // Victim is dead, propagate to caller

// Safe to continue using victim here
```

### Call Chain Differences

**Melee:** `perform_violence() -> hit() -> oneHit() -> applyDamage()` (returns DELETE_VICT)

**Spells/skills:** `doFireball() -> reconcileDamage() -> applyDamage()` (returns -1)

`oneHit()` bypasses `reconcileDamage()` because combat specials and hatred are handled elsewhere in melee.

### DELETE Flags vs Damage Values

DELETE flags use bit 29 (`const int DELETE_VICT = ((1 << 7) | (1 << 29))` in `defs.h`) to avoid collision with damage values. This allows functions to return damage OR deletion flags in the same int.

**Critical exception:** `reconcileDamage()` returns `-1` (not a DELETE flag) on death because it returns actual damage dealt on survival, making a flag-based return ambiguous.

## Combat List Management

### Adding to Combat List: setVictFighting()

```cpp
void TBeing::setVictFighting(TBeing* tbv, int dam) {
    if (!tbv)
        return;

    // Already fighting someone else?
    if (fight()) {
        vlogf(LOG_COMBAT, format("Fighting character (%s) set to fighting another. (%s)") %
                         getName() % tbv->getName());
        return;
    }

    // Increment defender's attacker count (for MAX_COMBAT_ATTACKERS limit)
    if (!isAffected(AFF_ENGAGER))
        tbv->attackers++;

    // Add to head of combat list (O(1) operation)
    next_fighting = gCombatList;
    gCombatList = this;

    // Wake up if sleeping and damage didn't kill
    if (getPosition() == POSITION_SLEEPING)
        if (getHit() > dam)
            doWake("");

    // Set fighting relationship
    specials.fighting = tbv;
    sendPositionGmcp();  // Notify MUD client
    playmusic(pickRandMusic(MUSIC_COMBAT_01, MUSIC_COMBAT_03), MUSIC_TYPE_COMBAT);
}
```

**Source:** `code/code/misc/combat.cc`

**Key Points:**
- O(1) insertion at list head
- Increments `attackers` count on defender (used for overcrowding checks)
- Wakes sleeping victim if not killed
- Triggers combat music on client

### Removing from Combat List: stopFighting()

```cpp
void TBeing::stopFighting() {
    TBeing* tmp;

    // Validation
    if (!fight()) {
        vlogf(LOG_BUG, "Character not fighting at stopFighting invocation");
        return;
    }

    // CRITICAL: Adjust global iterator if we're removing the cached pointer
    if (gCombatNext == this)
        gCombatNext = next_fighting;

    // Remove from linked list
    if (gCombatList == this) {
        // Head of list - simple case
        gCombatList = next_fighting;
    } else {
        // Middle/end of list - must search
        for (tmp = gCombatList; tmp && (tmp->next_fighting != this);
             tmp = tmp->next_fighting)
            ;
        if (!tmp) {
            vlogf(LOG_COMBAT, "Char fighting not found Error - ABORT");
            abort();  // Fatal: list corruption detected
        }
        tmp->next_fighting = next_fighting;
    }

    // Quest solo-kill tracking
    affectedData* af;
    for (af = affected; af; af = af->next) {
        if (af->type == AFFECT_COMBAT && af->modifier == COMBAT_SOLO_KILL) {
            if (awake()) {
                vlogf(LOG_COMBAT, "Removing Solo Combat Affect from: " + getName());
                sendCheatMessage("Your quest progress has been interrupted!");
                affectRemove(af);
            }
        }
    }

    // Decrement attacker count on defender
    if (!isAffected(AFF_ENGAGER))
        fight()->attackers--;

    // Clear combat flags
    REMOVE_BIT(specials.affectedBy, AFF_AGGRESSOR);
    REMOVE_BIT(specials.affectedBy, AFF_ENGAGER);
    REMOVE_BIT(specials.affectedBy, AFF_RIPOSTE);
    REMOVE_BIT(specials.affectedBy, AFF_FOCUS_ATTACK);

    // Clear fighting relationship
    next_fighting = NULL;
    specials.fighting = NULL;
    sendPositionGmcp();
    updatePos();  // Adjust position based on HP

    stopmusic();
}
```

**Source:** `code/code/misc/combat.cc`

**Critical Behaviors:**
- O(n) removal in worst case (end of list)
- FATAL error (`abort()`) if character not found in list
- Quest solo-kill affects prevent leveling on quest mobs with interference
- Calls `reformGroup()` implicitly (not shown but required before deletion)

## Combat Interrupts

### What Stops Combat?

The `canFight()` function checks various conditions that prevent combat from continuing:

```cpp
int TBeing::canFight(TBeing* target, silentTypeT silent) {
    // 1. Peaceful room
    if (roomp && roomp->isRoomFlag(ROOM_PEACEFUL)) {
        if (silent == SILENT_NO)
            sendTo("This room has a peaceful feeling - you can't fight here.\n\r");
        stopFighting();
        return FALSE;
    }

    // 2. Exhaustion
    if (tooTired()) {
        sendTo("PANIC! You are so exhausted, you cannot attack!");
        stopFighting();
        return FALSE;
    }

    // 3. Not in same room
    if (!sameRoom(*target)) {
        vlogf(LOG_COMBAT, "NOT in same room when fighting");
        if (fight())
            stopFighting();
        return FALSE;
    }

    // 4. Overcrowding (MAX_COMBAT_ATTACKERS = 9999)
    if (target->attackers >= MAX_COMBAT_ATTACKERS &&
        (specials.fighting != target)) {
        sendTo("You can't attack them, no room!");
        return FALSE;
    }
    if ((attackers >= MAX_COMBAT_ATTACKERS) && (target->fight() != this) &&
        (specials.fighting != target)) {
        sendTo("There are too many other people in the way.");
        return FALSE;
    }

    // 5. Target dead
    if (target->getPosition() == POSITION_DEAD)
        return FALSE;

    // 6. Attacker stunned/incapacitated
    if (getPosition() <= POSITION_STUNNED)
        return FALSE;

    // 7. Shape transformation breaks combat
    if (affectedBySpell(SPELL_FALCON_WINGS)) {
        sendTo("Your feathery wings disappear as you attempt to fight.");
        affectFrom(SPELL_FALCON_WINGS);
        return FALSE;
    }

    // 8. Mount issues - can return DELETE_THIS!
    if (riding) {
        if (dynamic_cast<TBeing*>(riding)) {
            if (!rideCheck(-5) && !rideCheck(-5)) {
                int rc = fallOffMount(riding, POSITION_SITTING);
                if (IS_SET_DELETE(rc, DELETE_THIS))
                    return DELETE_THIS;  // Rider died from fall!
                return FALSE;
            }
        }
    }

    return TRUE;
}
```

**Source:** `code/code/misc/combat.cc`

### Combat-Breaking Conditions Summary

| Condition | Effect | Calls stopFighting() |
|-----------|--------|---------------------|
| ROOM_PEACEFUL flag | Blocks all combat | Yes |
| Exhausted (move <= 0) | Cannot attack | Yes |
| Different rooms | Separation | Yes |
| Target dead | Combat ends | No (handled elsewhere) |
| Attacker stunned | Cannot act | No |
| Shape transformation | Breaks specific spells | No |
| Failed mount check | Fall off mount | No |
| Overcrowding (9999 attackers) | No room to fight | No |

### CRITICAL: canFight() Can Return DELETE_THIS

When mount operations fail catastrophically:
```cpp
int rc = fallOffMount(riding, POSITION_SITTING);
if (IS_SET_DELETE(rc, DELETE_THIS))
    return DELETE_THIS;  // Must propagate to caller!
```

Callers must check for DELETE flags from `canFight()`:
```cpp
rc = canFight(target);
if (IS_SET_DELETE(rc, DELETE_THIS))
    return DELETE_THIS;
```

## DELETE Flag Handling

### The Pattern

Combat functions return DELETE flags to signal that objects should be deleted by their caller:

```cpp
int rc = ch->hit(vict, pulse + tmp_pulse);

if (IS_SET_DELETE(rc, DELETE_VICT)) {
    vict->reformGroup();  // Break follower relationships FIRST
    delete vict;
    vict = NULL;
    continue;  // Process next combatant
} else if (IS_SET_DELETE(rc, DELETE_THIS)) {
    ch->reformGroup();
    delete ch;
    ch = NULL;
    break;  // Can't continue without attacker
}
```

### reformGroup() Must Be Called

**Before deleting ANY combatant:**
```cpp
vict->reformGroup();  // Breaks follower/master relationships
delete vict;
```

Failing to call `reformGroup()` leaves group pointers dangling, causing crashes when other group members access the freed memory.

### Always Use IS_SET_DELETE, NOT IS_SET

```cpp
// WRONG - uses wrong bitmask check
if (IS_SET(rc, DELETE_THIS)) { }

// CORRECT - DELETE flags use bit 29
if (IS_SET_DELETE(rc, DELETE_THIS)) { }
```

The `IS_SET_DELETE()` macro checks for the combined bit pattern `(flag | (1 << 29))`. Regular `IS_SET()` will fail to detect DELETE flags.

## Quest Solo-Kill Tracking

### AFFECT_COMBAT with COMBAT_SOLO_KILL

Certain quest mobs require solo kills. When a character joins combat with the quest mob, an affect is added:

```cpp
affectedData af;
af.type = AFFECT_COMBAT;
af.modifier = COMBAT_SOLO_KILL;
af.be = questHolder;  // Pointer to player who should get credit
```

### Quest Mob Detection (`damage.cc`)

Certain quest mobs (TROLL_GIANT, CAPTAIN_RYOKEN, TREE_SPIRIT, JOHN_RUSTLER, ORC_MAGI, CLERIC_VOLCANO, CLERIC_ARDEN) track solo-kill requirements via `AFFECT_COMBAT` with `COMBAT_SOLO_KILL`. When someone other than the quest-holder deals damage, the quest is marked failed:

```cpp
if (af->type == AFFECT_COMBAT && af->modifier == COMBAT_SOLO_KILL) {
    TBeing* tbt = dynamic_cast<TBeing*>(af->be);
    if (tbt && tbt != this) {
        tbt->setQuestBit(TOG_AVENGER_CHEAT);  // Quest failed
        v->affectRemove(af);
    }
}
```

### Enforcement in stopFighting()

If combat ends for ANY reason other than death, the quest fails:

```cpp
for (af = affected; af; af = af->next) {
    if (af->type == AFFECT_COMBAT && af->modifier == COMBAT_SOLO_KILL) {
        if (awake()) {  // If still alive, quest failed
            vlogf(LOG_COMBAT, "Removing Solo Combat Affect from: " + getName());
            sendCheatMessage("Your quest progress has been interrupted!");
            affectRemove(af);
        }
    }
}
```

**Scenarios that fail the quest:**
- Player flees
- Mob flees
- Combat interrupted by movement (teleport, summon, etc.)
- Player dies
- Another player assists

The only valid outcome is staying in combat until the mob dies.

## Common Bugs and Anti-Patterns

### 1. Wrong Iterator Pattern

```cpp
// CRASH: Local iterator invalidated on deletion
for (ch = gCombatList; ch; ch = ch->next_fighting) {
    rc = ch->hit(vict, pulse);
    if (IS_SET_DELETE(rc, DELETE_THIS)) {
        delete ch;  // ch->next_fighting now invalid!
    }
    // Loop advances to freed memory - CRASH
}

// CORRECT: Use global iterator cache
for (ch = gCombatList; ch; ch = gCombatNext) {
    gCombatNext = ch->next_fighting;  // Cache FIRST
    rc = ch->hit(vict, pulse);
    if (IS_SET_DELETE(rc, DELETE_THIS)) {
        ch->reformGroup();
        delete ch;
        ch = NULL;
        break;
    }
}
```

### 2. Forgetting reformGroup()

```cpp
// BUG: Leaves group pointers dangling
if (IS_SET_DELETE(rc, DELETE_VICT)) {
    delete vict;  // Other group members now have stale pointer!
}

// CORRECT: Break relationships first
if (IS_SET_DELETE(rc, DELETE_VICT)) {
    vict->reformGroup();
    delete vict;
    vict = NULL;
}
```

### 3. Continuing After DELETE_THIS

```cpp
// BUG: Accessing ch after DELETE_THIS
rc = ch->hit(vict, pulse);
if (IS_SET_DELETE(rc, DELETE_THIS)) {
    delete ch;
}
ch->doSomethingElse();  // CRASH - ch is freed!

// CORRECT: Early exit after deletion
rc = ch->hit(vict, pulse);
if (IS_SET_DELETE(rc, DELETE_THIS)) {
    ch->reformGroup();
    delete ch;
    ch = NULL;
    break;  // Exit loop immediately
}
ch->doSomethingElse();  // Safe - only reached if ch alive
```

### 4. Not Checking Weapon DELETE Flags

```cpp
// BUG: oneHit() can return DELETE_ITEM (weapon destroyed)
while (fx > 0.999) {
    oneHit(target, HAND_PRIMARY, weapon, mod, &fx);
    weapon->doSomething();  // weapon might be deleted!
}

// CORRECT: Check DELETE_ITEM flag
while (fx > 0.999) {
    rc = oneHit(target, HAND_PRIMARY, weapon, mod, &fx);
    if (IS_SET_DELETE(rc, DELETE_ITEM)) {
        delete weapon;
        weapon = NULL;
        break;  // Can't continue without weapon
    }
}
```

### 5. Ignoring canFight() Return Value

```cpp
// BUG: canFight() can return DELETE_THIS
canFight(target);
// Continue execution - might crash if rider died from fall!

// CORRECT: Check return value
int rc = canFight(target);
if (IS_SET_DELETE(rc, DELETE_THIS))
    return DELETE_THIS;
if (!rc)
    return FALSE;
```

### 6. Modifying gCombatNext During Iteration

```cpp
// BUG: Modifying gCombatNext breaks iteration
for (ch = gCombatList; ch; ch = gCombatNext) {
    gCombatNext = ch->next_fighting;

    if (someCondition)
        gCombatNext = NULL;  // BREAKS the loop!
}

// CORRECT: Don't modify gCombatNext manually
// Let stopFighting() adjust it if needed
```

### 7. Wrong Check for reconcileDamage Death

```cpp
// CRASH: Wrong check for reconcileDamage death
rc = attacker->reconcileDamage(victim, dam, type);
if (IS_SET_DELETE(rc, DELETE_VICT)) { ... }  // Never triggers! Use == -1

// CRASH: Not propagating death to caller
rc = attacker->reconcileDamage(victim, dam, type);
if (rc == -1) { /* forgot to return DELETE_VICT */ }  // Use-after-free!

// CRASH: Ignoring return value entirely
ch->hit(vict, pulse);
vict->sendTo("You dodge!");  // vict may be deleted!
```

## Performance Characteristics

| Operation | Complexity | Notes |
|-----------|------------|-------|
| Add to combat list | O(1) | Prepend to head |
| Remove from combat list | O(n) | Linear search required |
| Iterate combat list | O(n) | 12 iterations per round |
| Attack distribution | O(1) | Modulo arithmetic |
| blowCount() calculation | O(1) | Float operations |

**Combat list size:** Typically 10-50 characters in busy areas, up to 200+ during large raids.

**Per-round cost:** With 50 combatants and 12 inner loop iterations = 600 `hit()` calls per 1.2 seconds = 500 calls/second.

## Scheduler Integration

### procPerformViolence

Combat is triggered by the scheduler every `Pulse::COMBAT` (12 ticks):

```cpp
scheduler.add(new procPerformViolence(Pulse::COMBAT));

// In process.cc
bool procPerformViolence::run(const TPulse& pulse) const {
    perform_violence(pulse.pulse);
    return false;  // Never delete (persistent process)
}
```

**Source:** `code/code/sys/socket.cc`, `code/code/sys/process.cc`

The absolute pulse number is passed to `perform_violence()`, which distributes attacks based on server uptime (not just round-relative timing).

## Key Files Reference

| File | Contents |
|------|----------|
| `code/code/misc/combat.cc` | `perform_violence()` main loop, `hit()` attack distribution, `setVictFighting()`, `stopFighting()`, `canFight()` validation |
| `code/code/misc/offense.cc` | `blowCount()` calculation |
| `code/code/misc/damage.cc` | `reconcileDamage()`, `applyDamage()`, `damageEpilog()` |
| `code/code/sys/comm.h` | Pulse constants |
| `code/code/sys/process.cc` | `procPerformViolence` |

## See Also

- [Combat Formulas](combat-formulas.md) - Hit probability and damage calculations
- [DELETE Flag System](delete-flags.md) - Memory management signaling
- [Position Stance](position-stance.md) - Position effects on combat
