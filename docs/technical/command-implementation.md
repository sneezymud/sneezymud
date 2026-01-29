---
title: Command System Guide
description: Player command flow through parsing, lookup, dispatch, and implementation patterns including DELETE flag ownership rules and three-tier target resolution.
keywords: [parseCommand, doCommand, searchForCommandNum, commandArray, cmdTypeT, triggerSpecial, doMove, doSay, doKick, vict parameter, DELETE_VICT, DELETE_THIS, get_char_room_vis, three-tier resolution, flag translation, reconcileDamage]
category: Important Systems

last_updated: 2026-01-29
source_files: [code/code/misc/parse.cc, code/code/misc/parse.h, code/code/cmd/cmd_kick.cc, code/code/cmd/cmd_bash.cc, code/code/cmd/cmd_trip.cc, code/code/cmd/cmd_headbutt.cc, code/code/cmd/cmd_grapple.cc, code/code/cmd/cmd_slam.cc, code/code/cmd/cmd_disarm.cc, code/code/cmd/cmd_stomp.cc, code/code/cmd/cmd_deathstroke.cc, code/code/cmd/cmd_whirlwind.cc, code/code/cmd/cmd_steal.cc, code/code/cmd/cmd_rescue.cc, code/code/cmd/cmd_get.cc]
related:
  - memory-safety.md
  - damage-pipeline.md
  - communication-system.md
  - spec-procs.md
---

# Command System Guide

This document covers the complete command system in SneezyMUD: how player input flows through parsing, lookup, and dispatch, plus the critical patterns for implementing command handlers.

---

## Part 1: Command Dispatch

Player commands flow through parsing, alias expansion, lookup, validation, spec proc hooks, and finally execution via a large switch statement.

### Entry Point: parseCommand()

`TBeing::parseCommand()` (parse.cc) extracts the first word, expands aliases, looks up the command, and calls `doCommand()`:

```cpp
int TBeing::parseCommand(const sstring& orig_arg, bool typedIn, bool doAlias);
```

### Alias Expansion

Aliases expand BEFORE command lookup:

| Syntax | Meaning |
|--------|---------|
| `%` | Replaced with remaining arguments |
| `~` | Multiline separator (executes as separate commands) |

```cpp
// alias "att" = "attack %" -> "att goblin" becomes "attack goblin"
// alias "buff" = "cast armor~cast bless" -> executes both commands
```

### Command Lookup: searchForCommandNum()

Linear search with abbreviation matching (parse.cc):

```cpp
cmdTypeT searchForCommandNum(const sstring& argument) {
    for (cmdTypeT i = MIN_CMD; i < MAX_CMD_LIST; i++) {
        if (!commandArray[i]) continue;
        if (is_abbrev(argument, commandArray[i]->name))
            return i;
    }
    return MAX_CMD_LIST;  // Not found
}
```

`is_abbrev()` allows prefix matching: "lo" matches "look". **Order matters:** first match wins.

### Command Metadata: commandArray

```cpp
class commandInfo {
    const char* name;           // Command name for matching
    positionTypeT minPosition;  // Minimum position required
    int minLevel;               // Minimum immortal level (0 = mortal)
};

// Example from buildCommandArray() (parse.cc)
commandArray[CMD_LOOK] = new commandInfo("look", POSITION_RESTING, 0);
commandArray[CMD_ECHO] = new commandInfo("echo", POSITION_SLEEPING, GOD_LEVEL1);
```

### doCommand() Validation (parse.cc)

Before dispatch, `doCommand()` checks:
1. **Level** - `GetMaxLevel() >= commandArray[cmd]->minLevel`
2. **Paralysis/Stun** - Cannot act if affected and command requires movement
3. **Position** - `getPosition() >= commandArray[cmd]->minPosition`
4. **Captive** - Limited commands when captured

### Spec Proc Hooks: triggerSpecial()

Before the switch, `triggerSpecial()` (parse.cc) lets spec procs intercept commands:

```cpp
rc = triggerSpecial(NULL, cmd, newarg.c_str());
if (IS_SET_DELETE(rc, DELETE_THIS)) return DELETE_THIS;
else if (rc) return FALSE;  // Spec proc ate the command
```

Check order: task interruption, spell task, room spec, equipment specs, inventory specs, room contents.

**Returning TRUE eats the command** - normal processing stops.

### The Switch Statement (parse.cc)

Over 500 cases dispatch to handler functions:

```cpp
switch (cmd) {
    case CMD_NORTH: case CMD_SOUTH: /* ... */
        rc = doMove(cmd);
        break;
    case CMD_SAY: case CMD_SAY2:
        rc = doSay(newarg);
        addToLifeforce(1);
        break;
    case CMD_LOOK:
        doLook(newarg, cmd);
        addToLifeforce(1);
        break;
    // ... hundreds more
}
```

### Adding a New Command

1. **Add enum** in `parse.h` (before `MAX_CMD_LIST`):
```cpp
CMD_MYCOMMAND,
MAX_CMD_LIST,  // Keep last
```

2. **Add array entry** in `buildCommandArray()`:
```cpp
commandArray[CMD_MYCOMMAND] = new commandInfo("mycommand", POSITION_STANDING, 0);
```

3. **Add switch case** in `doCommand()`:
```cpp
case CMD_MYCOMMAND:
    rc = doMyCommand(newarg);
    break;
```

4. **Implement handler** (see Part 2 for patterns):
```cpp
int TBeing::doMyCommand(const sstring& arg) {
    // Implementation
    return FALSE;  // or DELETE_* flags
}
```

### Position Types

| Position | Commands |
|----------|----------|
| `POSITION_DEAD` | score, who, help |
| `POSITION_SLEEPING` | list, wake |
| `POSITION_RESTING` | look, say, get |
| `POSITION_SITTING` | buy, sell |
| `POSITION_STANDING` | dance, most actions |
| `POSITION_FIGHTING` | kill, attack |

### Special Shortcuts

Single-character shortcuts (parse.cc): `'` = say, `:` = emote, `,` = emote

### Hide Behavior

Most commands break hide. Exceptions include: look, score, inventory, help, who, equipment, save, exits, consider. Backstab/slit break hide AFTER execution.

---

## Part 2: Implementation Patterns

With the dispatch mechanics understood, this section covers the critical patterns for implementing command handlers correctly. **Misusing these patterns causes crashes.** The most common errors: not checking `vict` parameter before deleting, failing to translate DELETE flags when roles change between caller and callee, ignoring return values from helper functions.

### Three-Tier Target Resolution

Combat commands follow a consistent three-tier fallback pattern to find their target:

```cpp
int TBeing::doKick(const char* argument, TBeing* vict) {
    TBeing* victim;

    // Tier 1: Use explicit parameter if provided
    if (!(victim = vict)) {
        // Tier 2: Parse argument to find target by name
        if (!(victim = get_char_room_vis(this, argument))) {
            // Tier 3: Default to current fight target
            if (!(victim = fight())) {
                sendTo("Kick whom?\n\r");
                return FALSE;
            }
        }
    }
    // ... execute command with victim
}
```

#### The Three Tiers

| Tier | Source | When Used |
|------|--------|-----------|
| 1 | `vict` parameter | Called programmatically (spec procs, other commands) |
| 2 | `get_char_room_vis()` | Player typed a target name |
| 3 | `fight()` | No target specified, use current opponent |

This pattern appears in nearly all combat commands: `doKick`, `doBash`, `doTrip`, `doHeadbutt`, `doStomp`, `doGrapple`, `doSlam`, `doDeathstroke`, `doDisarm`, `doSteal`, etc.

### DELETE_VICT Ownership Rules

**Critical:** The caller who provides a `vict` parameter **owns** that pointer and is responsible for deletion. If the callee resolves the victim itself (via `get_char_room_vis` or `fight()`), the callee owns it.

#### The Standard Deletion Pattern

```cpp
int TBeing::doKick(const char* argument, TBeing* vict) {
    int rc;
    TBeing* victim;

    // ... target resolution ...

    rc = kick(this, victim, skill);

    if (IS_SET_DELETE(rc, DELETE_VICT)) {
        if (vict)           // Caller passed the victim?
            return rc;      // YES: Return flag, let caller delete
        delete victim;      // NO: We resolved it, we delete it
        victim = NULL;
        REM_DELETE(rc, DELETE_VICT);  // Clear flag before returning
    }
    return rc;
}
```

#### Why This Matters

When `vict` is provided by the caller:
- The caller holds a reference to the victim
- If we delete it here, the caller has a dangling pointer
- Returning `DELETE_VICT` tells caller to perform the deletion

When we resolved `victim` ourselves:
- No external code holds a reference
- We must delete it ourselves
- We clear the flag so caller doesn't try to delete a nullptr

#### Ownership Decision Tree

```
Was vict parameter non-NULL?
├── YES → Caller owns victim
│         └── Return DELETE_VICT, let caller delete
└── NO → We own victim (found via get_char_room_vis or fight())
         └── Delete here, clear DELETE_VICT flag
```

### Flag Translation Between Contexts

When calling helper functions, the semantic meaning of DELETE flags changes based on which parameter maps to which role.

#### Same-Context Example (No Translation)

When the helper's `victim` is also our `victim`:

```cpp
// In doKick: victim is our victim
rc = kick(this, victim, skill);
// kick() returns DELETE_VICT when victim dies
if (IS_SET_DELETE(rc, DELETE_VICT)) {
    // DELETE_VICT means delete victim - direct pass-through
    return DELETE_VICT;
}
```

#### Cross-Context Translation Examples

When the helper's "this" or "victim" maps to different entities in our context:

```cpp
// In bashSuccess: victim->crashLanding() where victim is caller's VICT
rc = victim->crashLanding(POSITION_SITTING);
// crashLanding returns DELETE_THIS when the being dies
// But crashLanding's THIS is our VICT!
if (IS_SET_DELETE(rc, DELETE_THIS))
    return DELETE_VICT;  // Translate: their THIS is our VICT
```

```cpp
// In tripSuccess: victim->trySpringleap(this) - roles are swapped!
rc = victim->trySpringleap(this);
// trySpringleap: DELETE_THIS = delete victim, DELETE_VICT = delete this
// But in our context: victim is VICT, this is THIS
if (IS_SET_DELETE(rc, DELETE_THIS) && IS_SET_DELETE(rc, DELETE_VICT))
    return rc;  // Both die - flags happen to align
else if (IS_SET_DELETE(rc, DELETE_THIS))
    return DELETE_VICT;  // Their THIS is our VICT
else if (IS_SET_DELETE(rc, DELETE_VICT))
    return DELETE_THIS;  // Their VICT is our THIS
```

#### Translation Table

| Helper Call Pattern | Helper's `DELETE_THIS` | Helper's `DELETE_VICT` |
|--------------------|------------------------|------------------------|
| `this->helper(victim)` | Our `DELETE_THIS` | Our `DELETE_VICT` |
| `victim->helper(this)` | Our `DELETE_VICT` | Our `DELETE_THIS` |
| `victim->helper(item)` | Our `DELETE_VICT` | Our `DELETE_ITEM` |
| `item->helper(victim)` | Our `DELETE_ITEM` | Our `DELETE_VICT` |

### Common Command Structure

Most combat commands follow this structure:

```cpp
// 1. Public interface with target resolution
int TBeing::doXXX(const char* argument, TBeing* vict) {
    int rc;
    TBeing* victim;
    char name_buf[256];

    strcpy(name_buf, argument);

    // Three-tier target resolution
    if (!(victim = vict)) {
        if (!(victim = get_char_room_vis(this, name_buf))) {
            if (!(victim = fight())) {
                sendTo("XXX whom?\n\r");
                return FALSE;
            }
        }
    }

    // Basic validation
    if (!sameRoom(*victim)) {
        sendTo("That person isn't around.\n\r");
        return FALSE;
    }

    // Call implementation
    rc = xxx(this, victim, skill);

    // Add skill lag on success
    if (rc)
        addSkillLag(skill, rc);

    // Handle DELETE_VICT with ownership check
    if (IS_SET_DELETE(rc, DELETE_VICT)) {
        if (vict)
            return rc;
        delete victim;
        victim = NULL;
        REM_DELETE(rc, DELETE_VICT);
    }

    // Propagate DELETE_THIS
    if (IS_SET_DELETE(rc, DELETE_THIS))
        return DELETE_THIS;

    return rc;
}

// 2. Implementation function (static or member)
static int xxx(TBeing* ch, TBeing* victim, spellNumT skill) {
    // Validation via canXXX()
    if (!ch->canXXX(victim, SILENT_NO))
        return FALSE;

    // Resource checks and consumption
    if (ch->getMove() < XXX_MOVE) {
        ch->sendTo("You lack the vitality.\n\r");
        return FALSE;
    }
    ch->addToMove(-XXX_MOVE);

    // Skill checks
    int bKnown = ch->getSkillValue(skill);
    int successfulHit = ch->specialAttack(victim, skill);
    int successfulSkill = ch->bSuccess(bKnown, skill);

    // Branch to success/fail
    if (successfulHit && successfulSkill && successfulHit != GUARANTEED_FAILURE) {
        return ch->xxxSuccess(victim, skill);
    } else {
        return ch->xxxFail(victim, skill);
    }
}

// 3. Success handler
int TBeing::xxxSuccess(TBeing* victim, spellNumT skill) {
    // Calculate damage
    int dam = getSkillDam(victim, skill, level, advLearning);

    // Messages
    act("...", FALSE, this, 0, victim, TO_NOTVICT);

    // Apply damage - check for -1 death return
    if (reconcileDamage(victim, dam, skill) == -1)
        return DELETE_VICT;

    return TRUE;
}

// 4. Fail handler
int TBeing::xxxFail(TBeing* victim, spellNumT skill) {
    // Failure messages
    act("...", FALSE, this, 0, victim, TO_NOTVICT);

    // May have side effects (crashLanding, etc.)
    int rc = crashLanding(POSITION_SITTING);
    if (IS_SET_DELETE(rc, DELETE_THIS))
        return DELETE_THIS;

    // Zero-damage reconcile to start combat
    reconcileDamage(victim, 0, skill);

    return TRUE;  // or FALSE depending on whether lag should apply
}
```

### Dangerous Anti-Patterns

#### Forgetting the vict Check

```cpp
// CRASH: Always deletes, even when caller owns
if (IS_SET_DELETE(rc, DELETE_VICT)) {
    delete victim;  // Caller now has dangling pointer!
}

// CORRECT: Check ownership first
if (IS_SET_DELETE(rc, DELETE_VICT)) {
    if (vict)
        return rc;  // Let caller delete
    delete victim;
    REM_DELETE(rc, DELETE_VICT);
}
```

#### Not Translating Flags

```cpp
// CRASH: Wrong flag returned
rc = victim->crashLanding(POSITION_SITTING);
if (IS_SET_DELETE(rc, DELETE_THIS))
    return DELETE_THIS;  // WRONG! victim died, not this

// CORRECT: Translate the flag
rc = victim->crashLanding(POSITION_SITTING);
if (IS_SET_DELETE(rc, DELETE_THIS))
    return DELETE_VICT;  // Their THIS is our VICT
```

#### Ignoring Return Values

```cpp
// CRASH: Continuing after potential death
rc = reconcileDamage(victim, dam, skill);
victim->sendTo("You feel pain!\n\r");  // victim may be dead!

// CORRECT: Check immediately, return early
if (reconcileDamage(victim, dam, skill) == -1)
    return DELETE_VICT;
victim->sendTo("You feel pain!\n\r");  // Safe: only reached if alive
```

#### Using IS_SET Instead of IS_SET_DELETE

```cpp
// BUG: Won't detect DELETE flags (they use bit 29)
if (IS_SET(rc, DELETE_VICT)) { ... }

// CORRECT: Always use IS_SET_DELETE for DELETE flags
if (IS_SET_DELETE(rc, DELETE_VICT)) { ... }
```

### Special Cases

#### Commands That Can Kill the Caster

Some commands (like `grapple`) can result in both combatants dying:

```cpp
rc = victim->trySpringleap(this);
if (IS_SET_DELETE(rc, DELETE_THIS) && IS_SET_DELETE(rc, DELETE_VICT))
    return rc;  // Both flags set - return as-is
else if (IS_SET_DELETE(rc, DELETE_THIS))
    return DELETE_VICT;  // Translate
else if (IS_SET_DELETE(rc, DELETE_VICT))
    return DELETE_THIS;  // Translate
```

#### Area-of-Effect Commands

Commands like `whirlwind` that affect multiple targets must delete victims during iteration carefully:

```cpp
// Build list first to avoid iterator invalidation
std::vector<TBeing*> validTargets;
for (TThing* thing : roomp->stuff) {
    auto* being = dynamic_cast<TBeing*>(thing);
    if (being && isValidTarget(being))
        validTargets.push_back(being);
}

// Then process and delete in separate loop
for (TBeing* being : validTargets) {
    if (being->inRoom() != in_room)
        continue;  // May have fled
    rc = attack(this, being);
    if (IS_SET_DELETE(rc, DELETE_VICT)) {
        delete being;
        being = nullptr;
    }
}
```

#### Object Commands

Object commands (like `get`) use `DELETE_ITEM` for objects:

```cpp
// might return DELETE_THIS for ch
// might return DELETE_ITEM for obj
// might return DELETE_VICT for sub (container)
int get(TBeing* ch, TThing* obj, TThing* sub, ...) {
    rc = ch->checkForInsideTrap(sub);
    // checkForInsideTrap's DELETE_THIS means sub should die
    // In get()'s return semantics, sub maps to DELETE_VICT
    if (IS_SET_DELETE(rc, DELETE_THIS))
        return DELETE_VICT;
}
```

### reconcileDamage Return Value

**Important:** `reconcileDamage()` returns `-1` on victim death, NOT `DELETE_VICT`. This is a magic sentinel value that must be checked with `== -1`:

```cpp
// CORRECT: Check for -1
if (reconcileDamage(victim, dam, skill) == -1)
    return DELETE_VICT;

// WRONG: IS_SET_DELETE won't detect -1
if (IS_SET_DELETE(rc, DELETE_VICT)) { ... }  // Never triggers for reconcileDamage
```

See [Damage Pipeline](damage-pipeline.md) for full documentation.

---

## Files

| File | Content |
|------|---------|
| `code/code/misc/parse.cc` | `parseCommand()`, `doCommand()`, `searchForCommandNum()`, `triggerSpecial()`, `buildCommandArray()` |
| `code/code/misc/parse.h` | `cmdTypeT` enum, `commandInfo` class, `commandArray` declaration |
| `code/code/cmd/cmd_kick.cc` | doKick, kick, kickHit, kickMiss |
| `code/code/cmd/cmd_bash.cc` | doBash, bash, bashSuccess, bashFail |
| `code/code/cmd/cmd_trip.cc` | doTrip, trip, tripSuccess, tripFail |
| `code/code/cmd/cmd_headbutt.cc` | doHeadbutt, headbutt, headbuttHit, headbuttMiss |
| `code/code/cmd/cmd_grapple.cc` | doGrapple, grapple |
| `code/code/cmd/cmd_slam.cc` | doSlam, slamSuccess, slamFail |
| `code/code/cmd/cmd_disarm.cc` | doDisarm, disarm |
| `code/code/cmd/cmd_stomp.cc` | doStomp, stomp, stompHit, stompMiss |
| `code/code/cmd/cmd_deathstroke.cc` | doDeathstroke, deathstrokeSuccess, deathstrokeFail |
| `code/code/cmd/cmd_whirlwind.cc` | doWhirlwind, whirlwind, whirlwindSuccess |
| `code/code/cmd/cmd_steal.cc` | doSteal, steal, failSteal |
| `code/code/cmd/cmd_rescue.cc` | doRescue, rescue |
| `code/code/cmd/cmd_get.cc` | doGet, get |

## See Also

- [DELETE Flag System](delete-flags.md) - Complete DELETE_* flag documentation
- [Damage Pipeline](damage-pipeline.md) - reconcileDamage() and the -1 return value
- [Spec Procs](spec-procs.md) - Special procedures that may call commands programmatically
