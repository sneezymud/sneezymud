---
title: Spell Component System
category: important
keywords: [components, spellcasting, material, gestural, verbal, TComponent, spellbag, wizardry, ritualism, charges]
related: [spell-combat.md, affects-system.md, spell-skill-framework.md]
primary_symbols:
  functions: [findComponent, useComponent, enforceGestural, enforceVerbal, applyCompCheck]
  classes: [TComponent, TSpellBag, compInfo]
  files: [code/code/obj/obj_component.cc, code/code/misc/magicutils.cc, code/code/misc/discipline.cc]
---

## Overview

Spells require three types of components: material components are physical items with charges consumed during casting, gestural components require functional hands for movement patterns, and verbal components require speech capability. Component requirements are defined through bitflags in spellInfo.comp_types, combining component type flags with consumption timing flags to control when requirements are enforced.

Storage access for material components expands with discipline progression. Wizardry for mages and Ritualism for clerics and shamans determine which equipment slots players can access for components. Novices access only held items, while masters can draw from neck pouches, wrist containers, belts, and specialized spellbag containers. The findComponent function searches accessible locations based on current skill level.

Components are individual objects of class TComponent, inheriting from TMergeable to allow charge stacking. Each component has a vnum identifier, spell association, charge count, and type flags. When charges deplete to zero, the object self-destructs. Components can be personalized to specific players, preventing use by others.

Consumption timing varies by spell design. Material components may be consumed at spell initialization, completion, every round of multi-round spells, randomly during casting, or on the next-to-last round. The applyCompCheck function determines consumption timing for tasked spells based on current round and maximum rounds.

Immortals with IMMU_NOHASSLE and all NPCs bypass component requirements entirely. This prevents mobs from needing component inventories and allows testing spells without gathering materials. High-skill casters can bypass verbal requirements at 75 learnedness in Wizardry or Ritualism.

## Patterns

### Defining Spell Component Requirements

Set component type and timing flags in spellInfo.comp_types during spell definition in buildSpellArray. Combine the base component type flag with a timing flag to specify when consumption occurs. Use COMP_GESTURAL for hand requirements, COMP_VERBAL for speech requirements, and COMP_MATERIAL with a timing suffix for physical components.

For instant-cast spells requiring material at start, combine COMP_MATERIAL with COMP_MATERIAL_INIT. For multi-round spells consuming material at completion, use COMP_MATERIAL with COMP_MATERIAL_END. Spells requiring material every round use COMP_MATERIAL_ALWAYS. Random consumption uses COMP_MATERIAL_RANDOM, while consumption on next-to-last round uses COMP_MATERIAL_ALMOST_END.

Always include the base COMP_MATERIAL flag when using any COMP_MATERIAL_* timing flag. The base flag enables material component checking, while timing flags specify when consumption occurs. Omitting the base flag causes the timing flag to be ignored.

Define component vnums in obj_component.h using COMP_* constants. Add corresponding CompInfo entries mapping spell numbers to component vnums and defining consumption messages. The CompInfo entry provides messages shown to caster, room, target, and self when components are consumed.

### Component Access and Retrieval

The findComponent function searches equipment slots based on Wizardry or Ritualism skill level. It checks locations in priority order: primary hand, secondary hand, belt spellbag, neck pouch, wrist containers, general inventory, and contents of spellbag containers. The function returns the first matching component found within accessible locations.

Access level constants WIZ_LEV_COMP_* and RIT_LEV_COMP_* define skill thresholds unlocking storage locations. Low-level casters access only held items. Mid-level casters access inventory. High-level casters access worn containers at neck, wrist, and belt slots. The getAccessLevel helper determines current access level from discipline learnedness.

Spellbag containers provide expandable storage specifically for components. The TSpellBag class extends TExpandableContainer with a findSomeComponent method locating components by spell number within the container. Spellbags are typically worn on the belt slot and become fully accessible at high Wizardry or Ritualism skill.

Components automatically merge when picked up. The TMergeable inheritance enables canMerge and doMerge operations. When a player picks up a component matching one already carried, charges combine into a single stack. This prevents inventory clutter and simplifies component management.

### Component Consumption During Casting

The useComponent function locates, validates, and consumes components. It first checks immortal status and monster type for bypass conditions. For players, it verifies component existence via findComponent, validates personalization if set, displays consumption messages from CompInfo, decrements charges, and deletes the component object if charges reach zero.

Multi-round spells use spelltask.component_used to prevent double-consumption. When a component is consumed, this flag is set on the task object. Subsequent rounds check the flag before attempting additional consumption. The flag resets automatically on task completion.

The applyCompCheck function determines when to consume components during multi-round spells. It examines comp_types flags to identify the consumption pattern, then applies pattern-specific logic comparing current round to maximum rounds. Pattern 1 consumes at initialization, pattern 2 at completion, pattern 3 every round, pattern 4 randomly, and pattern 5 on next-to-last round.

For gestural and verbal requirements, enforceGestural and enforceVerbal check physical capability before casting. Gestural enforcement verifies at least one functional arm with no item held or one hand free. Verbal enforcement checks for silence effects, paralysis, and mouth functionality. Both functions apply position-based penalties for non-standing casters.

### Component Creation and Distribution

The component_placement vector defines automatic placement rules. Each compPlace entry specifies room ranges, mob associations, timing conditions based on hour, day, month, and weather, load variance percentage, uniqueness constraints, and messages displayed on creation or removal.

Components can be awarded during spell practice through the learning-by-doing system. The learnFromDoing function checks CompInfo for the practiced spell and creates component objects when defined. This provides a progression path where practicing spells generates components needed for future casting.

NPC trainers and special procedures can create and gift components to players. The read_object function with VIRTUAL mode instantiates components from vnums. Setting charges via setComponentCharges before transferring to players provides multi-use components.

Shops can sell components as regular items. Since TComponent extends TMergeable, shop mechanics handle component stacking automatically. Players purchasing multiple components receive merged stacks with combined charges.

## Reference

### Component Type Flags

COMP_GESTURAL requires hand gestures. COMP_GESTURAL_INIT enforces gestures at spell start. COMP_GESTURAL_END enforces at completion. COMP_GESTURAL_ALWAYS enforces every round. COMP_GESTURAL_RANDOM enforces randomly during casting.

COMP_VERBAL requires verbal components. COMP_VERBAL_INIT enforces speech at start. COMP_VERBAL_END enforces at completion. COMP_VERBAL_ALWAYS enforces every round. COMP_VERBAL_RANDOM enforces randomly.

COMP_MATERIAL requires material components. COMP_MATERIAL_INIT consumes at start. COMP_MATERIAL_END consumes at completion. COMP_MATERIAL_ALWAYS consumes every round. COMP_MATERIAL_RANDOM consumes randomly. COMP_MATERIAL_ALMOST_END consumes on next-to-last round.

SPELL_TASKED marks multi-round spells requiring task tracking. This flag enables applyCompCheck logic for timed component consumption.

### Component Object Flags

COMP_DECAY indicates component decays over time. COMP_SPELL marks components usable for spellcasting. COMP_POTION marks components usable in potions. COMP_SCRIBE marks components usable for scribing scrolls.

CACT_PLACE signals component creation action. CACT_REMOVE signals component removal action. CACT_UNIQUE restricts component to single instance in game world.

### Access Level Constants

WIZ_LEV_COMP_PRIM_OTHER_FREE allows primary hand access with enemy item stealing. WIZ_LEV_COMP_EITHER_OTHER_FREE allows either hand with stealing. WIZ_LEV_COMP_EITHER allows either hand without stealing. WIZ_LEV_COMP_INV allows inventory access. WIZ_LEV_COMP_NECK allows neck slot access. WIZ_LEV_COMP_WRIST allows wrist slot access. WIZ_LEV_COMP_BELT allows belt slot access with full spellbag support.

Ritualism uses parallel RIT_LEV_COMP_* constants with identical slot progression for clerics and shamans.

### TComponent Class Members

getComponentCharges returns current charge count. setComponentCharges updates charge count. getComponentSpell returns associated spell number. setComponentSpell sets associated spell. getComponentType returns component type flags. setComponentType sets component type flags.

canMerge determines if component can merge with another object. doMerge combines charges when merging components. itemType returns ITEM_COMPONENT for type identification.

### CompInfo Structure Fields

comp_num holds component object vnum. spell_num holds associated spell number. to_caster provides message shown to caster when consuming. to_other provides message shown to other room occupants. to_vict provides message shown to spell target. to_self provides message when self-targeting. to_room provides generic room message. to_self_object provides message when targeting owned object. to_room_object provides message when targeting object visible to room.

### compPlace Structure Fields

room1 and room2 define room range for placement. mob specifies mob vnum or MOB_NONE for room placement. number holds component vnum to load. place_act holds CACT_* flags controlling placement behavior. max_number limits total instances. variance provides percentage chance of placement occurring.

hour1 and hour2 define hour range for timed placement. day1 and day2 define day range. month1 and month2 define month range. weather specifies required weather conditions using WEATHER_* constants.

message provides room message on load or removal. glo_msg provides global message to room range. sound specifies sound effect on load. sound_loop controls sound repetition count.

### Error Messages

Missing component for non-rangers displays "You seem to lack the proper materials to complete your task." Missing component for rangers displays "You seem to lack the proper natural materials to complete your task."

Gestural failures display "You cannot gesture properly." or "You need a free hand to cast this spell." depending on specific constraint violated. Sitting or prone positions may display "You struggle to gesture from this position."

Verbal failures display "You cannot speak!" when silenced or affected by silence spell. Paralysis displays "You are paralyzed!" Mouth dysfunction displays "Your mouth doesn't work properly!"

## Implementation

### Component Search Algorithm

The findComponent function begins by determining caster access level through getAccessLevel. This function examines Wizardry discipline learnedness for mages or Ritualism learnedness for clerics and shamans, returning the highest WIZ_LEV_COMP_* or RIT_LEV_COMP_* constant satisfied by current skill.

Search proceeds through locations in fixed priority order. Primary hand is checked first regardless of access level. If access level permits either hand, secondary hand is checked next. Belt slot spellbags are checked when access level reaches WIZ_LEV_COMP_BELT. Neck pouches are checked at WIZ_LEV_COMP_NECK. Wrist containers are checked at WIZ_LEV_COMP_WRIST. General inventory is checked at WIZ_LEV_COMP_INV.

For each location, the search examines held or worn objects. If the object is a TComponent with matching spell number and sufficient charges, it is returned immediately. If the object is a container like TSpellBag, findSomeComponent is called to search container contents recursively.

The search returns nullptr when no matching component is found in accessible locations. This triggers missingComponent error message display and spell failure in calling code.

### Component Consumption Flow

The bPassMageChecks, bPassShamanChecks, and similar validation functions examine comp_types during spell initiation. When COMP_MATERIAL is set, they determine if material is needed at initialization by checking COMP_MATERIAL_INIT. If set, useComponent is called before allowing spell to proceed.

The useComponent function performs bypass checks first. Immortals with isImmunity IMMU_NOHASSLE return nullptr without consuming components. Monsters identified via isMonster return nullptr. If spelltask.component_used is already set, nullptr is returned to prevent double-consumption.

Component location proceeds through findComponent. If nullptr is returned, missingComponent displays error message and useComponent returns nullptr to signal failure. If found, personalization is validated by comparing getPersonalizedTo result with caster getName. Mismatches trigger missingComponent and nullptr return.

Consumption messages are retrieved from CompInfo vector using spell number as index. The act function displays to_caster message to caster, to_other to room occupants, to_vict to spell target if present and alive, and to_self when target equals caster. Message substitutions replace $n with caster name, $p with component object, and other standard act placeholders.

Charge decrement occurs via getComponentCharges and setComponentCharges. When new charge count reaches zero, the component object is removed from its location via operator-- and deleted. Otherwise, the object persists with reduced charges for future use.

The spelltask.component_used flag is set to true after successful consumption. This prevents multi-round spells from consuming components multiple times during a single casting.

### Multi-Round Consumption Timing

The applyCompCheck function is called during task continuation for multi-round spells. It begins by extracting comp_types from discArray for the spell being cast. The function identifies consumption pattern by checking COMP_MATERIAL_* timing flags in priority order.

Pattern 1 for COMP_MATERIAL_INIT takes no action during continuation because component was already consumed during spell initialization. Pattern 2 for COMP_MATERIAL_END compares task.rounds to task.maxRounds minus one. When equal, this is the final round, so useComponent is called.

Pattern 3 for COMP_MATERIAL_ALWAYS calls useComponent unconditionally every round. Pattern 4 for COMP_MATERIAL_RANDOM generates a random number between 0 and 1. If result is 1, useComponent is called for this round. Pattern 5 for COMP_MATERIAL_ALMOST_END compares task.rounds to task.maxRounds minus two. When equal, useComponent is called on the next-to-last round.

Return value is TRUE for successful check completion. If useComponent fails and returns nullptr, the spell task may be interrupted depending on calling code handling. Some spell implementations continue despite component depletion, while others abort the task.

### Gestural and Verbal Enforcement

The enforceGestural function checks comp_types for COMP_GESTURAL flag. If not set, TRUE is returned immediately with no validation. For mages, both arms are examined via heldInPrimHand and heldInSecHand. If both hands hold items, "You need a free hand to cast this spell." is sent and FALSE returned.

Arm functionality is verified through canUseLimb on both getPrimaryArm and getSecondaryArm. If neither arm is functional, "You cannot gesture properly." is sent and FALSE returned. Position affects gesture effectiveness through penalty calculation. Standing position has zero penalty. Each position level below standing adds 10 percent penalty. A random number between 0 and 100 is compared to penalty. If random number is less than penalty, "You struggle to gesture from this position." is sent and FALSE returned.

Shamans receive similar validation with class-specific constraints. Both caster types can potentially cast with one hand occupied if the other is free and functional.

The enforceVerbal function checks comp_types for COMP_VERBAL flag. If not set, TRUE is returned immediately. Silence detection checks affectedBySpell SPELL_SILENCE and isAffected AFF_PARALYSIS. Either condition sends "You cannot speak!" or "You are paralyzed!" and returns FALSE.

Mouth functionality is verified via canUseLimb WEAR_HEAD. Failure sends "Your mouth doesn't work properly!" and returns FALSE. High-skill bypass checks discipline learnedness. Mages examine DISC_WIZARDRY learnedness, clerics and shamans examine DISC_RITUALISM. If learnedness reaches 75 or higher, TRUE is returned without requiring verbal component.

For normal verbal casting, act displays mystical words message to room and TRUE is returned to allow spell continuation.

### Component Merging Mechanism

The canMerge function inherited from TMergeable examines object type, vnum, and mergeable status. Both objects must be TComponent instances with identical component spell numbers and matching vnums. If conditions are satisfied, canMerge returns positive result.

The doMerge function is called when objects are combined. Charges from the merged object are added to charges of the target object via getComponentCharges and setComponentCharges. The merged object is then removed from the game via operator-- and deleted. Only the target object persists with combined charge count.

Merging occurs automatically during object acquisition through operator+= overloads. When an object is added to a character or container, existing contents are scanned for mergeable candidates. If canMerge succeeds with an existing object, doMerge is called instead of adding a separate object instance.

## Troubleshooting

### Spell Ignores Material Component Requirement

Verify COMP_MATERIAL base flag is set in addition to timing flag like COMP_MATERIAL_INIT. Setting only the timing flag without COMP_MATERIAL causes component checking to be skipped entirely. The base flag enables material component validation, while timing flags specify when consumption occurs.

Check that CompInfo vector contains an entry for the spell number. If the spell index exceeds CompInfo.size or the comp_num field is negative, component lookup fails silently without error messages. Add a properly initialized compInfo structure with valid component vnum.

### Component Not Found Despite Being Carried

Examine caster Wizardry or Ritualism skill level. Low-skill casters can only access held items, not inventory or worn containers. If component is in inventory but access level is WIZ_LEV_COMP_PRIM_OTHER_FREE, findComponent cannot locate it. Move component to primary hand or increase discipline skill.

Verify component spell association matches spell being cast. The getComponentSpell value must equal the spell number. Components created with wrong spell association will not be found during search. Use setComponentSpell to correct association.

Check component charge count via getComponentCharges. Components with zero or negative charges are treated as depleted and ignored by search logic. Recharge component using setComponentCharges with positive value.

### Component Consumed Multiple Times Per Cast

Examine spelltask.component_used flag handling. If code manually resets this flag during task continuation, double-consumption can occur. The flag should only be cleared automatically on task completion or failure.

Verify useComponent is not called from multiple code paths during the same round. Some spell implementations call useComponent directly in spell logic in addition to applyCompCheck automatic consumption. Remove duplicate calls.

### Gestural Requirement Fails Despite Free Hands

Check for equipment worn on arms that sets arm dysfunction flags. Items marked as damaged or cursed may prevent canUseLimb from returning true even though no item is held in hands. Remove or repair problematic equipment.

Examine caster position. Sitting, resting, or prone positions apply random penalties to gestural casting. The penalty increases with distance from standing position. Solution is standing up before casting or accepting occasional failures.

### Verbal Requirement Bypassed Unexpectedly

Verify caster Wizardry or Ritualism learnedness is below 75. The high-skill bypass at 75 learnedness intentionally removes verbal component requirements. This is expected behavior for advanced casters.

Check for immortal status with IMMU_NOHASSLE. Immortals bypass all component requirements including verbal. This is intentional for testing and immortal flexibility.

### Components Not Merging When Picked Up

Examine component personalization status. Personalized components do not merge with non-personalized or differently-personalized components. The isPersonalized check in canMerge prevents merging when personalization differs.

Verify both components have identical spell associations via getComponentSpell. Components with different spell numbers will not merge even if vnums match. This prevents combining unrelated component types.

Check for object modifications that might affect mergeable status. Custom object flags or altered charge mechanisms can interfere with TMergeable inheritance behavior. Ensure component objects are standard TComponent instances without unusual modifications.

### Component Creation Failing in Placement System

Examine variance field in compPlace entry. Variance provides percentage chance of placement occurring. Low variance values like 5 or 10 result in infrequent creation. Increase variance or add multiple placement entries to improve availability.

Verify timing conditions are being met. Hour, day, month, and weather fields create narrow windows for component appearance. Expand ranges or remove constraints to allow more flexible placement.

Check max_number constraint against existing component instances. If max_number is reached, no additional components will be created until existing ones are consumed or decay. Increase limit or implement component decay.

### Missing Component Messages Not Appearing

Confirm missingComponent function is being called when findComponent returns nullptr. Some spell implementations fail silently instead of displaying error messages. Add explicit missingComponent call after failed component lookup.

Verify CompInfo messages are populated with non-empty strings. Empty message fields in CompInfo result in silent consumption without visible feedback. Define appropriate messages for all CompInfo fields.

Check for message suppression from immortal or monster status. Bypass code paths may skip message display to reduce spam. This is expected behavior for NPCs and testing immortals.
