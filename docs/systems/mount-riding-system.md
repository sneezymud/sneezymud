---
title: Mount and Riding System
description: Mount/rider mechanics including stability checks, combat bonuses, Deikhan specialization, and DELETE flag safety
keywords: [riding, mount, horse, POSITION_MOUNTED, rideCheck, fallOffMount, Deikhan, Chivalry]
category: Important Systems
related: [combat-rounds, position-stance, movement-terrain-navigation, spell-skill-framework]
last_updated: 2026-02-01
created_by_model: opus
source_files: [code/code/misc/riding.cc, code/code/misc/combat.cc, code/code/disc/disc_deikhan_mounted.cc, code/code/misc/movement.cc]
---

# Mount and Riding System

## Overview

Characters can mount rideable creatures to gain combat advantages at the cost of attack frequency. The system supports multiple riders per mount, with the last rider in the chain controlling movement direction. Stability is checked through a two-fail system: riders only fall off when they fail two consecutive ride checks.

Mounts and riders maintain independent HP pools with no damage transfer between them. When either takes damage, the rider must pass stability checks or fall. Deikhans receive significant bonuses and bypasses through the Chivalry skill and mounted discipline.

The mounting relationship creates a follower bond where the mount follows the primary rider. Mount AI is suppressed while ridden, but the creature may become hostile if the rider falls off or fails to mount.

## Patterns

### DELETE Flag Handling

**Always check fallOffMount return values.** The function can return DELETE_THIS when falling causes death (flying mounts over fall sectors, crash landing damage). Never continue execution after calling fallOffMount without checking the return.

**Always cache nextRider before modifying rider chains.** The dismount operation modifies the linked list. Iterate with `t2 = t->nextRider` before any operation that could remove a rider.

**Always validate riding pointer as TBeing before operations.** Use dynamic_cast and check for nullptr. The riding pointer may point to furniture (beds, chairs) which lacks TBeing methods.

### Stability System

**Always use two consecutive rideCheck calls for fall-off determination.** A single failed check does not dismount the rider. Both checks must fail. The modifiers stack: -5 when mount takes damage, -10 when rider takes damage.

**Never assume damage transfers between rider and mount.** They maintain independent HP pools. Healing the mount does not heal the rider. A mount at 1 HP can carry a full-health rider; a full-health mount can carry a dying rider.

### Movement Control

**Always check horseMaster before allowing movement control.** Only the last rider in the chain can direct the mount. Secondary riders attempting movement receive an error message.

**Never allow non-Deikhans to mount or dismount fighting creatures.** The Deikhan class bypasses these restrictions. Check hasClass(CLASS_DEIKHAN) before denying mount-related combat actions.

### Mount Ownership

**Always apply AFFECT_HORSEOWNED when PCs dismount.** This 1 mud-hour affect prevents NPCs from mounting recently-dismounted horses, blocking "horse theft" scenarios.

**Never allow mounting of another player's pet.** Check the master pointer and ownership before mount attempts.

## Reference

### Rideability Requirements

| Requirement | Threshold | Notes |
|-------------|-----------|-------|
| Creature rideability | SPEC_HORSE or RIDABLE racial flag | Two pathways to being mountable |
| Rider form | Humanoid | Cannot mount if transformed to non-humanoid |
| Min mount height | rider height * 60% | Mount too small below this |
| Max mount height | rider height * 250% | Mount too large at or above |
| Max rider height for mounting | mount height * 150% | Checked separately in mount() |
| Rider slots per mount | 4 | Fixed maximum |
| Large rider threshold | rider > mount * 66% | Consumes 2 slots instead of 1 |

### rideCheck Modifiers

| Modifier | Value | Effective Skill Change |
|----------|-------|------------------------|
| Saddle (primary rider only) | +8 | +24 skill |
| Deikhan base | +5 | +15 skill |
| Advanced riding bonus | +0 to +6 | +0 to +18 skill |
| Secondary rider | -5 | -15 skill |
| Mount damaged | -5 | -15 skill |
| Rider damaged | -10 | -30 skill |

Formula: effective_skill = base_skill + (3 * sum_of_modifiers)

### Combat Bonuses

| Source | Attack Bonus | Defense Bonus |
|--------|--------------|---------------|
| POSITION_MOUNTED | level/4 + 1 | level/4 + 1 |
| Chivalry (100 learning) | +74 | +159 |
| Special attacks | +2 | N/A |

Attack frequency penalty: 0.67x (lose 1/3 of attacks)

### Combat Mounting Difficulty

| Combat State | Non-Deikhan Effectiveness | Deikhan Effectiveness |
|--------------|---------------------------|----------------------|
| Tanking | 25% | 33% |
| Fighting (not tanking) | 50% | 50% |
| Engaged only | 100% | 100% |
| Not in combat | 100% | 100% |

### Mount Type Skills (Deikhan)

| Skill | Start Level | Mount Types |
|-------|-------------|-------------|
| SKILL_RIDE_DOMESTIC | 5 | Horse, bovine, ox, pig, sheep, baanta, canine, goat |
| SKILL_RIDE_NONDOMESTIC | 36 | Rhino, tiger, giraffe, bear, boar, elephant, deer |
| SKILL_RIDE_WINGED | 66 | Griffon, hippogriff, wyvern, dragon, dragonne, lammasu, shedu, sphinx |
| SKILL_RIDE_EXOTIC | 85 | Feline, basilisk, centaur, chimera, frog, lamia, manticore, turtle, lion, leopard, cougar, wyvelin |

### Dismount Restrictions

| Condition | Bypass |
|-----------|--------|
| Mount fighting | Deikhan class |
| Berserking | None |
| Room at mob limit | Immortal |
| Flying mount, cannot fly | SKILL_RIDE_WINGED coax to land |

### Key Constants

| Constant | Value |
|----------|-------|
| MAX_RIDERS | 4 |
| POSITION_MOUNTED | 11 |
| HORSEOWNED_DURATION | 1 mud hour |
| ATTACK_FREQUENCY_MULTIPLIER | 0.67 |

## Implementation

### Rider Chain Structure

Mounts use a three-pointer system in TThing: `rider` points to the first rider, `nextRider` chains additional riders, and `riding` back-references the mount. The chain is ordered by mount time, with the last rider (found by traversing until nextRider is null) designated as horseMaster with movement control.

The `mount()` function appends new riders to the end of the chain, making them the new primary. The `dismount()` function removes riders from any position in the chain, repairing the linked list. If the dismounting rider was the master, the mount either transfers to a new master or stops following.

### Stability Mechanics

The two-check fall system works by calling `rideCheck()` twice in sequence. The function applies all modifiers (saddle, Deikhan bonuses, secondary rider penalty, damage context), multiplies the sum by 3, adds it to base SKILL_RIDE, and makes a bSuccess roll. Both calls must fail for `fallOffMount()` to trigger.

Fall consequences depend on mount flight state and room sector. Flying mounts over fall sectors trigger full falling damage via `checkFalling()`. Flying mounts elsewhere trigger `crashLanding()` with appropriate position. Ground mounts in fall sectors also check falling.

### Combat Integration

POSITION_MOUNTED provides automatic bonuses through the position system in attackRound and defendRound. The Chivalry skill adds substantial bonuses on top of position bonuses when the rider has POSITION_MOUNTED.

Attack frequency reduction happens in offense.cc where primary and secondary hand attack counts are multiplied by 0.67 for mounted characters. This penalty offsets the accuracy and defense gains.

Fall-off checks occur in damage processing. When a mount takes damage, all riders must pass two rideCheck(-5) calls. When a rider takes damage, they must pass two rideCheck(-10) calls. These checks happen per-damage-instance, not per-round.

### Movement Control Flow

When a mounted character attempts movement, `doMove()` first verifies they are horseMaster. If not, movement is rejected with a message. If yes, `validMove()` runs mount-specific checks (mount standing, not fighting, door height clearance, sector compatibility).

Movement point costs use the mount's movement pool for full cost, while the rider pays random(0, cost)/3. Flying mounts reduce cost to 1; levitating mounts reduce to cost/4.

Movement failures (mount exhaustion, rider exhaustion, weight collapse, drunkenness) trigger `fallOffMount()` with appropriate messaging.

### Deikhan Advanced System

The `advancedRidingBonus()` function combines SKILL_ADVANCED_RIDING with the mount-type-specific skill (domestic, nondomestic, winged, exotic) and returns half their sum. This bonus feeds into rideCheck modifiers and calm mount effectiveness.

Calm mount reduces anger, malice, and suspicion by random(0, (SKILL_CALM_MOUNT + advancedRidingBonus) / 30) points. This allows Deikhans to pacify aggressive mounts before or during riding.

Train mount skill enables mounts to continue following after dismount. The check uses skill/2 effectiveness; failure causes the mount to stop following and potentially transfer to a new horseMaster if other riders remain.

### Flying Mount Handling

Non-Deikhans cannot mount flying creatures. Deikhans require SKILL_RIDE_WINGED >= 70 and must pass a skill check to coax the mount to land before mounting.

Dismounting flying mounts requires either: natural flight ability (dismount to POSITION_FLYING), being in a flying sector (dismount to POSITION_FLYING), or SKILL_RIDE_WINGED to coax the mount to land first. Otherwise dismount is blocked.

### NPC Mount Seeking

The `lookForHorse()` function allows NPCs to automatically find and mount horses in their room. It checks: the NPC is not a utility mob, sentinel, or shopkeeper; the horse has no AFFECT_HORSEOWNED; the horse level is at least 4 below NPC level; the horse is not already following someone.

## Troubleshooting

### Crash: Use-after-free following fallOffMount

**Symptom:** Crash or corruption after rider falls off mount, especially from flying mounts.

**Cause:** Code continues executing after fallOffMount returned DELETE_THIS, accessing freed memory.

**Diagnostic:** Check if fallOffMount return value is captured and tested with IS_SET_DELETE.

**Fix:** Always check `if (IS_SET_DELETE(rc, DELETE_THIS)) return DELETE_THIS;` immediately after fallOffMount calls.

### Crash: Invalid iterator during rider chain traversal

**Symptom:** Crash when mount with multiple riders dies or when dismounting riders in a loop.

**Cause:** Loop uses `t->nextRider` after dismount modified the chain.

**Diagnostic:** Look for rider iteration patterns without cached next pointer.

**Fix:** Cache next pointer before operations: `for (t = rider; t; t = t2) { t2 = t->nextRider; ...operations... }`

### Bug: Rider controls mount despite not being horseMaster

**Symptom:** Non-primary rider can direct mount movement.

**Cause:** Movement code checks `if (riding)` instead of `if (riding && riding->horseMaster() == this)`.

**Diagnostic:** Verify horseMaster check exists before movement processing.

**Fix:** Add horseMaster comparison before allowing movement commands.

### Bug: Rider falls off too easily

**Symptom:** Riders dismount after single damage hit instead of requiring sustained instability.

**Cause:** Code uses single rideCheck instead of two consecutive checks.

**Diagnostic:** Count rideCheck calls in the fall-off decision path.

**Fix:** Require two consecutive failures: `if (!rideCheck(mod) && !rideCheck(mod))`.

### Bug: Position mismatch after mounting/dismounting

**Symptom:** Character has POSITION_MOUNTED but riding is null, or has riding set but wrong position.

**Cause:** Position not synchronized with mount state during edge cases (death, teleport, polymorph).

**Diagnostic:** Add validation: `if (riding && getPosition() != POSITION_MOUNTED)` logs a bug.

**Fix:** Ensure all paths that modify riding also update position appropriately.

### Bug: NPC mounts player's horse immediately after dismount

**Symptom:** Player dismounts, nearby NPC immediately mounts their horse.

**Cause:** AFFECT_HORSEOWNED not applied or lookForHorse not checking for it.

**Diagnostic:** Verify dismount applies the affect and lookForHorse checks `affectedBySpell(AFFECT_HORSEOWNED)`.

**Fix:** Ensure dismount path includes AFFECT_HORSEOWNED application for PC dismounts.

### Bug: Flying mount dismount blocked incorrectly

**Symptom:** Player with flight ability cannot dismount flying mount, receives "order your mount to land" message.

**Cause:** canFly() check missing or ordered incorrectly in dismount logic.

**Diagnostic:** Trace dismount path for flying mount + flying rider case.

**Fix:** Check canFly() before checking SKILL_RIDE_WINGED in dismount logic.
