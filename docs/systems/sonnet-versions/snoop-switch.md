---
title: Snoop, Switch, and Return System
category: critical
keywords: [snoop, switch, return, descriptor pointers, body swapping, immortal commands, polymorph, ACT_POLYSELF, POLY_TYPE_SWITCH]
related: [memory-safety.md, network-architecture.md]
primary_symbols:
  functions: [doSnoop, doSwitch, doReturn, outputProcessing, SwitchStuff]
  classes: [Descriptor, snoopData]
  files: [code/code/misc/immortal.cc, code/code/sys/connect.h, code/code/sys/connect.cc]
---

# Snoop, Switch, and Return System

## Overview

Three interconnected immortal commands manage character observation and control through descriptor pointer manipulation. Snoop allows monitoring another character's input and output streams by establishing snoop chains in the descriptor network. Switch enables taking control of a mob by transferring the immortal's descriptor to the target body while storing the original character reference. Return restores control to the original character by reversing the descriptor transfer.

These commands share architectural patterns with the spell-based polymorph system but have distinct safety requirements and behaviors. The critical difference is that admin switch is invisible and non-invasive, reassigning descriptors without touching stats or affects, while polymorph creates a transformation that transfers stats and creates a temporary body.

The system revolves around three descriptor pointers: character points to the currently active body, original points to the true player if switched or polymorphed (NULL otherwise), and the snoop structure tracks monitoring relationships. Mismanaging these pointers causes crashes through NULL dereferences, dangling pointers, or use-after-free when descriptors are moved or destroyed.

Common crash scenarios: switching while snooped creates dangling snoop pointers when the descriptor moves, not checking desc->original before return causes NULL dereference, failing to clear ACT_POLYSELF before deletion triggers heap-use-after-free in destructors, nested switching breaks the restoration chain, and deleting switched bodies without calling doReturn loses the immortal's descriptor.

## Patterns

### Descriptor State Transitions

Normal logged-in state has desc->character pointing to the player, desc->original set to NULL, and empty snoop chains. When snooped, the target's desc->snoop.snoop_by points to the snooper's descriptor, while the snooper's desc->snoop.snooping points back to the target. When switched, the immortal's original body loses its descriptor (set to NULL), the mob gains the descriptor with desc->character pointing to the mob and desc->original pointing to the immortal, and the immortal's polyed flag is set to POLY_TYPE_SWITCH.

### Snoop Chain Management

Establishing a snoop link requires validating that the snooper has POWER_SNOOP, is not already snooping someone, is higher level than the target, the target doesn't have PLR_NOSNOOP set, and the target is not currently switched. After validation, set desc->snoop.snooping on the snooper to point to the target's descriptor, and set the target's desc->snoop.snoop_by to point back to the snooper.

When the snooped character receives output, outputProcessing duplicates the text to the snooper by pushing a SnoopComm to the snooper's output queue. Snoop links are automatically broken when either party logs off through the Descriptor destructor, or when the target switches into another body.

### Switch Execution Flow

Parse the command to distinguish between regular switch (find existing mob) and switch load (create new mob from vnum). For switch load, search mob_index by name, reject shopkeepers and newbie helpers, create the mob with read_mobile, validate permissions with limitPowerCheck, place the mob in the current room, and optionally create starting wealth based on Config::LoadOnDeath.

For both modes, locate the target mob either in the current room or globally. Validate safety: cannot switch into self, cannot switch while snooping or being snooped, cannot switch into occupied bodies (has desc or is TPerson), cannot switch while already switched (desc->original exists), and must pass limitPowerCheck for the target vnum.

Execute the transfer by setting the immortal's polyed flag to POLY_TYPE_SWITCH, moving desc->character to point to the mob, setting desc->original to point to the immortal, transferring the descriptor to the mob with tBeing->desc = desc, and clearing the immortal's descriptor with desc = NULL.

### Return Execution Flow

Validate state by checking desc exists, desc->original exists, and the character is not a forced transformation like lycanthropy. Branch based on ACT_POLYSELF: if set, this is spell-based polymorph requiring stat transfer with SwitchStuff, visual messages, affect removal, and mob cleanup; if not set, this is admin switch requiring only descriptor restoration.

Restore descriptors by setting originalBody->desc to the current descriptor, setting desc->character to desc->original, clearing both desc->original pointers, clearing the original's polyed flag, and clearing the current body's descriptor. For polymorph forms, REMOVE_BIT(specials.act, ACT_POLYSELF) before deletion or storage to avoid heap-use-after-free.

Handle mob cleanup based on deleteMob parameter: if true and ACT_POLYSELF was set, delete the polymorph form; if false, move to Room::POLY_STORAGE for potential recovery. Admin switch forms are never deleted or stored since they are existing mobs.

### Switch vs Polymorph Distinction

Admin switch does not set ACT_POLYSELF, performs no stat transfer, displays no visual transformation messages, removes no affects on return, and performs no mob cleanup. Spell polymorph sets ACT_POLYSELF, calls SwitchStuff to transfer stats, shows "turns liquid and reforms" messages, removes SPELL_POLYMORPH and related affects, and either deletes the form or moves it to POLY_STORAGE.

Check ACT_POLYSELF to distinguish the two systems. Admin switch is transparent to other players and preserves the mob as an independent entity. Polymorph creates a temporary body that merges with the caster's stats and must be cleaned up on return.

### Snoop and Switch Interaction

Never allow switching while snooping or being snooped. Both doSnoop and doSwitch check for conflicts: doSnoop rejects targets with desc->original set, and doSwitch rejects attempts when desc->snoop.snoop_by or desc->snoop.snooping is non-NULL. This prevents dangling pointers when the descriptor moves between characters.

If a snoop relationship exists and a switch is attempted, the descriptor transfer would leave the snoop chain pointing to the wrong character or to a NULL descriptor. This causes crashes when snoop output tries to duplicate to the invalid destination.

## Reference

### Descriptor Pointers

Descriptor::character is the currently active character, which may be a switched mob or polymorph form. Descriptor::original is the true player if switched or polymorphed, NULL otherwise. Descriptor::snoop is a snoopData structure with snoop_by pointing to the descriptor snooping this character, and snooping pointing to the descriptor this character is snooping.

### Snoop Validation Checks

hasWizPower(POWER_SNOOP) returns false: no snoop permission. desc->snoop.snooping is non-NULL: already snooping someone else. Target descriptor equals self descriptor: attempting to snoop self (silently succeeds). GetMaxLevel() less than or equal to target level: cannot snoop equal or higher level immortals. Target has PLR_NOSNOOP flag: player opted out. Target desc->original is non-NULL: target is switched, cannot snoop.

### Switch Validation Checks

this equals tBeing: attempting to switch into self. desc is NULL or desc->snoop.snoop_by or desc->snoop.snooping: mixing snoop and switch. tBeing->desc is non-NULL: target body already has descriptor. dynamic_cast TPerson succeeds: target is player character. desc->original is non-NULL: already switched. limitPowerCheck(CMD_SWITCH, tBeing->number) fails: insufficient permissions. For switch load: mob_index spec is SPEC_SHOPKEEPER or SPEC_NEWBIE_EQUIPPER.

### Return Validation

desc is NULL: no descriptor to restore. desc->original is NULL: not switched or polymorphed. hasQuestBit(TOG_TRANSFORMED_LYCANTHROPE) is true: forced transformation cannot be manually reversed.

### Switch Load Restrictions

Cannot load shopkeepers identified by mob_index[].spec == SPEC_SHOPKEEPER. Cannot load newbie helpers identified by SPEC_NEWBIE_EQUIPPER. Must pass limitPowerCheck for the target vnum. Non-immortals must have equipment[WEAR_NECK] with spec == 139 to enable switch ability.

### polyed Flag Values

POLY_TYPE_NONE: not switched or polymorphed. POLY_TYPE_SWITCH: admin switch active. POLY_TYPE_DISGUISE: disguise skill active. POLY_TYPE_SHAPESHIFT: shapeshift spell active. POLY_TYPE_POLYMORPH: polymorph spell active.

### SwitchStuff Function

Transfers stats, equipment, affects, and other character state between polymorphed form and original body. Only called during polymorph return when ACT_POLYSELF is set. Not called during admin switch return.

### doReturn Parameters

argument: optional limb name for limb transformation return. limb: specific limb slot for transformation reversal. tell: if true, display transformation messages. deleteMob: if true, delete the polymorph form; if false, move to POLY_STORAGE.

## Implementation

### Snoop Output Duplication

outputProcessing in connect.cc checks if snoop.snoop_by exists and the snooper's descriptor is valid. If so, creates a SnoopComm object containing the snooped character's name and the output text, then pushes it to the snooper's output queue. This duplicates all output from the snooped character to the snooper in real-time.

SnoopComm is a specialized communication type that prefixes the output with the snooped character's name, allowing the snooper to distinguish multiple snoop sessions if the system is extended to support that.

### Descriptor Lifecycle and Snoop Cleanup

The Descriptor destructor automatically breaks snoop chains by clearing the snoop_by pointer on any target being snooped, and clearing the snooping pointer on any snooper. This prevents dangling pointers when either party disconnects.

When a character switches, their descriptor moves to the new body, but the snoop validation in doSwitch prevents this by rejecting the switch attempt if any snoop relationship exists. This avoids the complexity of preserving or migrating snoop chains across body transfers.

### Switch Load Mob Creation

read_mobile creates a new mob instance from the prototype in mob_index. The mob's oldRoom field stores the room where it was created, used by some systems to determine the mob's home location. If Config::LoadOnDeath returns false, createWealth generates starting equipment and money for the mob based on its configuration. If LoadOnDeath is true, the mob spawns empty and only generates loot when killed.

limitPowerCheck validates immortal permissions against the mob vnum, consulting database configuration that maps command types and vnums to minimum access levels. This allows fine-grained control over which immortals can switch into powerful or sensitive mobs.

### Special Object Switch Enablement

Non-immortals can gain switch ability through a special object equipped in the WEAR_NECK slot with spec == 139. This enables quest systems or events to grant temporary switch ability without immortal promotion. The switch command checks for this object before enforcing immortal status.

### Polymorph Return Stat Transfer

When ACT_POLYSELF is set during return, SwitchStuff transfers hit points, mana, moves, position, fighting state, equipment, inventory, affects, and other runtime state from the polymorph form back to the original body. This preserves the player's progress during the transformation.

After stat transfer, affectFrom removes SPELL_POLYMORPH, SKILL_DISGUISE, and SPELL_SHAPESHIFT affects from the original body. These affects were applied when the transformation began and must be removed to restore the character's true state.

### ACT_POLYSELF Cleanup Ordering

REMOVE_BIT(specials.act, ACT_POLYSELF) must execute before the delete or storage operations. Some destructors and cleanup functions check ACT_POLYSELF to determine polymorph state, and if the flag is still set during destruction, they may attempt to access the original body pointer which may be invalid or already freed.

The deleteMob parameter controls whether the polymorph form is deleted immediately or moved to Room::POLY_STORAGE. Scheduler processes use deleteMob=false because the scheduler handles deletion timing separately. Manual return commands use deleteMob=true to immediately clean up the temporary form.

### Idle Timeout and Switched Characters

limits.cc checks if desc->original exists and desc->original->getTimer() exceeds the idle threshold. If the character has ACT_POLYSELF set, it calls SwitchStuff to transfer stats back before swapping the descriptor and disconnecting. Admin switch does not trigger stat transfer on idle, only descriptor restoration.

### Death While Switched

rawKill and die functions check if the dying character has desc->original set and desc->original->polyed equals POLY_TYPE_SWITCH. If so, doReturn is called with deleteMob=false before processing death, restoring the immortal to their original body. The mob dies and is deleted through normal death handling, but the immortal survives.

For spell polymorph deaths, the same check applies but ACT_POLYSELF is set, so doReturn performs stat transfer and returns the player to their original body before death processing. This prevents polymorphed players from dying while transformed.

### Nested Switch Prevention

desc->original being non-NULL indicates an active switch or polymorph. Allowing a second switch would overwrite desc->original with the intermediate mob pointer instead of the true player, breaking the restoration chain. The player would be unable to return to their original body.

### Snoop Target Validation Loop

doSnoop iterates descriptor_list to find a descriptor where the character's name matches the target argument. It skips descriptors in account mode (desc->account set) since those are at the login screen without an active character. The first matching descriptor with a valid character is selected as the snoop target.

### Switch Target Search

For regular switch, get_char_room searches the current room first. If not found, get_char with EXACT_YES searches globally for exact name matches, then EXACT_NO searches for partial matches. This allows switching into mobs anywhere in the world, not just the current room.

## Troubleshooting

### NULL Pointer Dereference in doReturn

Symptom: crash when typing return, dereferencing desc or desc->original. Cause: desc is NULL or desc->original is NULL when not actually switched. Solution: always validate both pointers exist before attempting return. Check if desc is NULL (shouldn't happen for active character but defensive) and if desc->original is NULL (not switched).

### Heap-Use-After-Free During Polymorph Cleanup

Symptom: ASan reports use-after-free when deleting polymorphed mob. Cause: ACT_POLYSELF flag still set during deletion, destructors access original body pointer which may be freed. Solution: REMOVE_BIT(specials.act, ACT_POLYSELF) before any delete or storage operation on the polymorph form.

### Dangling Snoop Pointers After Switch

Symptom: crash when snooped character switches, snooper's output processing crashes. Cause: switch was allowed while snoop relationship existed, descriptor moved but snoop pointers not updated. Solution: never allow switch when desc->snoop.snoop_by or desc->snoop.snooping is set. Both commands must reject the operation when any snoop relationship exists.

### Stuck in Switched State

Symptom: immortal cannot return, desc->original is NULL despite being in a mob body. Cause: nested switch attempted and succeeded, overwriting desc->original. Solution: prevent nested switch by checking desc->original before allowing switch. If already switched, reject with "You already seem to be switched."

### Switch Target Already Occupied

Symptom: two descriptors control same character, commands conflict. Cause: switch allowed into a mob that already has a descriptor. Solution: check tBeing->desc is NULL and dynamic_cast TPerson fails before allowing switch. Any existing descriptor or player character type must reject the switch.

### Lost Descriptor After Switched Mob Death

Symptom: immortal disconnects when switched mob dies. Cause: death handling did not call doReturn before deleting mob, descriptor deleted with mob. Solution: check desc->original and polyed flag in death functions, call doReturn before processing mob death.

### Snoop Output Missing or Misdirected

Symptom: snooper sees no output or wrong character's output. Cause: snoop chain broken or descriptor pointer stale. Solution: validate snoop.snoop_by exists and has valid desc pointer in outputProcessing before duplicating output. Break snoop links when descriptors are destroyed or characters switch.

### Return Transfers Wrong Stats

Symptom: character has wrong stats after polymorph return. Cause: SwitchStuff called during admin switch when it should only apply to polymorph. Solution: check ACT_POLYSELF flag before calling SwitchStuff. Admin switch never sets this flag and should skip stat transfer entirely.

### Immortal Cannot Switch Into Specific Mob

Symptom: "You're not allowed to switch into that mobile" despite sufficient level. Cause: limitPowerCheck rejects the vnum based on database configuration. Solution: verify immortal's access level in database, check wizard power settings, ensure the mob vnum is not restricted to higher access tiers.
