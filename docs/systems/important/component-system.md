---
title: Spell Component System
description: Material, gestural, and verbal requirements for spellcasting with discipline-based storage access and timed consumption patterns.
keywords: [component requirements, gestural requirements, verbal requirements, consumption timing, storage access]
category: important
primary_symbols:
  functions: [findComponent, useComponent, missingComponent, enforceGestural, enforceVerbal, applyCompCheck, willMerge, doMerge]
  classes: [TComponent, TSpellBag, spellInfo]
  enums: [COMP_GESTURAL, COMP_GESTURAL_INIT, COMP_GESTURAL_END, COMP_GESTURAL_ALWAYS, COMP_GESTURAL_RANDOM, COMP_VERBAL, COMP_VERBAL_INIT, COMP_VERBAL_END, COMP_VERBAL_ALWAYS, COMP_VERBAL_RANDOM, COMP_MATERIAL, COMP_MATERIAL_INIT, COMP_MATERIAL_END, COMP_MATERIAL_ALWAYS, COMP_MATERIAL_RANDOM, COMP_MATERIAL_ALMOST_END, SPELL_TASKED, COMP_DECAY, COMP_SPELL, COMP_POTION, COMP_SCRIBE, CACT_PLACE, CACT_REMOVE, CACT_UNIQUE, WIZ_LEV_COMP_PRIM_OTHER_FREE, WIZ_LEV_COMP_EITHER_OTHER_FREE, WIZ_LEV_COMP_EITHER, WIZ_LEV_COMP_INV, WIZ_LEV_NO_GESTURES, WIZ_LEV_NO_MANTRA, WIZ_LEV_COMP_NECK, WIZ_LEV_COMP_WRIST, WIZ_LEV_COMP_BELT, RIT_LEV_COMP_PRIM_OTHER_FREE, RIT_LEV_COMP_EITHER_OTHER_FREE, RIT_LEV_COMP_EITHER, RIT_LEV_COMP_INV, RIT_LEV_NO_GESTURES, RIT_LEV_NO_MANTRA, RIT_LEV_COMP_NECK, RIT_LEV_COMP_WRIST, RIT_LEV_COMP_BELT, PLR_NOHASSLE]
---

# Spell Component System

## Overview

Spellcasting requires three types of components: material (physical items consumed during casting), gestural (hand movements requiring free hands), and verbal (speech requiring a functional mouth). Material components are stackable objects with charges that deplete on use. The discipline system (Wizardry for mages, Ritualism for clerics/shamans) governs which storage locations a caster can access for components, with higher skill unlocking additional equipment slots. Immortals and NPCs bypass component requirements entirely.

## Patterns

### Component Requirement Specification

Always include both `COMP_MATERIAL` and a timing flag when requiring material components. The base flag enables checking; the timing flag controls consumption.

Never use only `COMP_MATERIAL_INIT` without `COMP_MATERIAL`. The spell will not check for the component at all.

Always add a `CompInfo` entry when defining a new component vnum. Missing entries prevent consumption and display a generic error ("Uh oh, something bogus happened.") to the player while logging a bug.

### Storage Access Enforcement

Never assume all players can access spellbag components. Low Wizardry/Ritualism restricts access to primary hand only.

Always check the caster's discipline progression when debugging component-not-found issues. The component may exist in an inaccessible location.

### Consumption Timing

Never manually reset `spelltask->component_used`. The system clears it automatically on task completion.

Always use `COMP_MATERIAL_END` with `SPELL_TASKED` for multi-round spells that consume at completion. Using `COMP_MATERIAL_INIT` consumes immediately even if the spell is interrupted.

### Component Merging

Components automatically merge when picked up if they share the same vnum. The `willMerge` function validates type (must be TComponent), vnum match, both having cost > 0 (free components from leveling do not merge), and combined charges not exceeding 100 before allowing combination.

### Special Cases

Personalized components belonging to another player are still found by `findComponent` but rejected by `useComponent` with a distinct message: "You can't use a component that is personalized for someone else!" This is logged as a potential exploit.

Always remember that `!isPc()` returns early from component consumption. NPCs cast without needing component inventories.

## Reference

### Component Requirement Flags

| Flag | Bit | Purpose |
|------|-----|---------|
| `COMP_GESTURAL` | 0 | Requires hand gestures |
| `COMP_GESTURAL_INIT` | 1 | Gestures at initialization |
| `COMP_GESTURAL_END` | 2 | Gestures at completion |
| `COMP_GESTURAL_ALWAYS` | 3 | Gestures every round |
| `COMP_GESTURAL_RANDOM` | 4 | Gestures randomly required |
| `COMP_VERBAL` | 5 | Requires speech |
| `COMP_VERBAL_INIT` | 6 | Speech at initialization |
| `COMP_VERBAL_END` | 7 | Speech at completion |
| `COMP_VERBAL_ALWAYS` | 8 | Speech every round |
| `COMP_VERBAL_RANDOM` | 9 | Speech randomly required |
| `COMP_MATERIAL` | 10 | Requires material component |
| `COMP_MATERIAL_INIT` | 11 | Consumed at spell start |
| `COMP_MATERIAL_END` | 12 | Consumed at spell end |
| `COMP_MATERIAL_ALWAYS` | 13 | Consumed every round |
| `COMP_MATERIAL_RANDOM` | 14 | Consumed randomly |
| `COMP_MATERIAL_ALMOST_END` | 15 | Consumed next-to-last round |
| `SPELL_TASKED` | 16 | Multi-round tasked spell |

### Component Action Flags

| Flag | Purpose |
|------|---------|
| `COMP_DECAY` | Component decays over time |
| `COMP_SPELL` | Used for spell casting |
| `COMP_POTION` | Can be used in potions |
| `COMP_SCRIBE` | Can be scribed |
| `CACT_PLACE` | Action: place/create component |
| `CACT_REMOVE` | Action: remove/destroy component |
| `CACT_UNIQUE` | Component is unique |

### Wizardry/Ritualism Access Levels

| Level Constant | Skill Threshold | Access Granted |
|----------------|-----------------|----------------|
| `WIZ_LEV_COMP_PRIM_OTHER_FREE` | 0+ (non-ambidextrous) | Primary hand only |
| `WIZ_LEV_COMP_EITHER_OTHER_FREE` | 15+ | Either hand |
| `WIZ_LEV_COMP_EITHER` | 30+ | Either hand (full) |
| `WIZ_LEV_COMP_INV` | 40+ | Inventory |
| `WIZ_LEV_NO_GESTURES` | 50+ | Gesture bypass |
| `WIZ_LEV_NO_MANTRA` | 60+ | Verbal bypass |
| `WIZ_LEV_COMP_NECK` | 75+ | Neck slot |
| `WIZ_LEV_COMP_BELT` | 99+ | Belt/full access |

Ritualism uses parallel `RIT_LEV_*` constants with identical skill thresholds and slot progression for clerics and shamans. The enum also defines `WIZ_LEV_COMP_WRIST` and `RIT_LEV_COMP_WRIST` but these are never returned by `getWizardryLevel()` or `getRitualismLevel()`.

### TComponent Class API

| Method | Purpose |
|--------|---------|
| `getComponentCharges()` | Returns current charge count |
| `setComponentCharges()` | Updates charge count |
| `getComponentSpell()` | Returns associated spell number |
| `setComponentSpell()` | Sets associated spell |
| `getComponentType()` | Returns component type flags |
| `setComponentType()` | Sets component type flags |
| `willMerge()` | Determines if component can merge with another object |
| `doMerge()` | Combines charges when merging components |
| `itemType()` | Returns `ITEM_COMPONENT` for type identification |

### CompInfo Structure Fields

| Field | Purpose |
|-------|---------|
| `comp_num` | Component object vnum |
| `spell_num` | Associated spell number |
| `to_caster` | Message shown to caster when consuming |
| `to_other` | Message shown to other room occupants |
| `to_vict` | Message shown to spell target |
| `to_self` | Message when self-targeting |
| `to_room` | Generic room message |
| `to_self_object` | Message when targeting owned object |
| `to_room_object` | Message when targeting object visible to room |

### compPlace Structure Fields

| Field | Purpose |
|-------|---------|
| `room1`, `room2` | Room range for placement |
| `mob` | Mob vnum or MOB_NONE for room placement |
| `number` | Component vnum to load |
| `place_act` | CACT_* flags controlling placement behavior |
| `max_number` | Limits total instances |
| `variance` | Percentage chance of placement occurring |
| `hour1`, `hour2` | Hour range for timed placement |
| `day1`, `day2` | Day range |
| `month1`, `month2` | Month range |
| `weather` | Required weather conditions (WEATHER_* constants) |
| `message` | Room message on load or removal |
| `glo_msg` | Global message to room range |
| `sound` | Sound effect on load |
| `sound_loop` | Sound repetition count |

### Error Messages

| Condition | Message |
|-----------|---------|
| Missing component (non-ranger) | "You seem to lack the proper materials to complete your task." |
| Missing component (ranger, non-combat) | "You seem to lack the proper materials to complete this magic skill." |
| Missing component (ranger, in combat) | "You are unable to concentrate on casting while fighting without your components in hand." |
| Hands occupied (mage/shaman) | "You must have one hand free and usable to perform the ritual's gestures!" |
| Arms non-functional (mage) | "You cannot perform the ritual's gestures without arms and hands!" |
| Arms non-functional (shaman) | "You cannot invoke the ritual without arms and hands!" |
| Position penalty | "Restricted movement while [position] causes you to mess up the ritual's gestures." |
| Silenced (mage/shaman) | "You are unable to chant the incantation!" |
| Silenced (cleric/deikhan) | "You are unable to recite the sacred words!" |

### Source Files

| File | Contents |
|------|----------|
| `obj_component.h` | TComponent class, compInfo structure, component vnums |
| `obj_component.cc` | TComponent method implementations |
| `obj_spellbag.h` | TSpellBag container class |
| `spell2.h` | COMP_* flags, spellInfo structure |
| `magicutils.cc` | findComponent, useComponent, missingComponent |
| `discipline.cc` | enforceGestural, enforceVerbal |
| `spelltask.cc` | applyCompCheck for multi-round consumption |
| `gaining.cc` | Wizardry/Ritualism level constants |

## Implementation

### TComponent Class Hierarchy

TComponent inherits from TMergeable, enabling same-type components to combine into stacks with merged charges. The class tracks charge count, associated spell, and type flags. When a player acquires a component matching one already held, the charges combine automatically. Charge depletion to zero triggers automatic deletion.

### Storage Location Search Order

The `findComponent` function determines access level from Wizardry or Ritualism skill, then searches locations in priority order: primary hand, secondary hand, belt slot (spellbag), neck slot (component pouch), wrist pouches, inventory, and finally inside spellbag containers. The search terminates at the first location containing a valid component for the requested spell.

### Consumption Timing Dispatch

The `applyCompCheck` function maps consumption flags to patterns (1=INIT, 2=END, 3=ALWAYS, 4=RANDOM, 5=ALMOST_END) and dispatches based on the `round` parameter, which counts down from max to 0. INIT (`COMP_MATERIAL_INIT`) returns TRUE before reaching the switch — the component is verified but never consumed. END consumes on the final round (`round == 0`). ALWAYS consumes every round. ALMOST_END consumes on the second-to-last round (`round == 1`). RANDOM has a bug: `if ((round = 0))` is an assignment (always false), so the branch always falls through to check `roll`, which was pre-computed as `max(0, number(0, round) - 1)` using the original round value. This makes consumption probability roughly `2 / (round + 1)` — nearly certain on late rounds, rare on early ones — rather than a uniform 50%.

### Gestural Enforcement

The `enforceGestural` method checks COMP_GESTURAL flag presence, then validates hand availability and arm functionality. Mages require at least one free hand (neither heldInPrimHand nor heldInSecHand may be occupied). Position penalties apply when not standing, with failure chance increasing as position decreases. High Wizardry/Ritualism (`WIZ_LEV_NO_GESTURES`, skill 50+) bypasses gestural requirements entirely.

### Verbal Enforcement

The `enforceVerbal` method checks for silence spell effects, paralysis, and head/mouth functionality. High Wizardry or Ritualism (`WIZ_LEV_NO_MANTRA`/`RIT_LEV_NO_MANTRA`, skill 60+) grants immunity to verbal requirements, allowing silent casting. The method broadcasts mystical utterance messages to the room on success.

### Component Merging Mechanism

The `willMerge` function inherited from TMergeable examines object type, vnum, and cost. Both objects must be TComponent instances with matching vnums and cost > 0 (free components from leveling are excluded). Combined charges must not exceed 100. The `doMerge` function adds charges from the merged object to the target, then deletes the merged object. Merging occurs automatically during object acquisition through `operator+=` overloads, which scan existing contents for mergeable candidates before adding new objects.

### Component Placement System

The `compPlace` structure defines world component spawning with room ranges, mob targets, timing conditions (hour/day/month ranges), weather requirements, and variance percentages. Placement can be conditional on CACT_UNIQUE flag, limiting instances in the game world. Messages and sound effects accompany component appearance.

### Component Acquisition Through Practice

Components can be awarded during spell practice through the `learnFromDoing` function. When a player practices a spell, the function checks CompInfo for the practiced spell and creates component objects when defined. This provides a progression path where practicing spells generates components needed for future casting.

### compInfo Message Dispatch

The CompInfo vector stores per-spell consumption messages with separate strings for caster, room observers, victim, self-target, and object-target cases. The `useComponent` function performs a linear search through this vector matching the component's spell number, then dispatches appropriate `act()` calls based on target presence and type.

### Immortal and NPC Bypass

The `useComponent` function checks `isImmortal() && isPlayerAction(PLR_NOHASSLE)` for immortals and `!isPc()` for NPCs, returning early without component consumption. This allows testing spells without gathering and prevents mobs from requiring component inventories.

## Troubleshooting

### Spell casts without consuming component

**Symptom:** Player casts successfully but component charges never decrease.

**Cause:** Missing `COMP_MATERIAL` base flag in spell definition. Timing flags alone do not enable consumption.

**Diagnostic:** Check `spellInfo.comp_types` for the spell. Verify both `COMP_MATERIAL` and a timing flag (INIT/END/ALWAYS/RANDOM/ALMOST_END) are present.

**Fix:** Add `COMP_MATERIAL` to the comp_types flags alongside the timing flag.

---

### Component consumed but no message displayed

**Symptom:** Charges decrease but player sees no consumption message.

**Cause:** Missing CompInfo entry for the spell/component combination.

**Diagnostic:** Check if `CompInfo[spell_number]` is populated. Verify `comp_num` field matches the component vnum.

**Fix:** Add complete CompInfo entry with all message strings populated.

---

### Player cannot find component they possess

**Symptom:** Player has component in spellbag but gets "lack materials" message.

**Cause:** Wizardry/Ritualism skill too low to access belt/spellbag location.

**Diagnostic:** Check player's Wizardry or Ritualism level against WIZ_LEV_COMP_BELT threshold. Verify component location matches accessible slots.

**Fix:** Player must train Wizardry/Ritualism higher, or move component to accessible location (hand, inventory if unlocked).

---

### Component consumed twice in multi-round spell

**Symptom:** Two charges consumed for single spell cast.

**Cause:** COMP_MATERIAL_INIT combined with COMP_MATERIAL_ALWAYS or similar double-consumption pattern.

**Diagnostic:** Review comp_types flags for conflicting timing flags. Also check if `useComponent` is called from multiple code paths during the same round.

**Fix:** Use only one timing flag per spell. INIT for immediate consumption, END for completion, ALWAYS for per-round. Remove duplicate `useComponent` calls if present.

---

### Personalized component rejected

**Symptom:** Component exists but caster receives "You can't use a component that is personalized for someone else!" message.

**Cause:** Component personalized to different character name. This is logged as a potential exploit (`LOG_MISC`).

**Diagnostic:** Examine component's personalization field via `isPersonalized()`. Compare against caster's name using `isname()`.

**Fix:** Acquire non-personalized component or one personalized to correct character.

---

### Gestural check fails unexpectedly

**Symptom:** "You cannot gesture properly" when arms appear functional.

**Cause:** Both arms disabled by affects or equipment, or position penalty triggered.

**Diagnostic:** Check canUseLimb for primary and secondary arms. Check position against POSITION_STANDING. Look for equipment worn on arms that sets dysfunction flags.

**Fix:** Stand up, remove arm-disabling affects, or free functional arm. Remove or repair problematic arm equipment.

---

### Verbal check fails for high-level caster

**Symptom:** Silenced caster cannot cast despite high Wizardry.

**Cause:** Wizardry bypass only triggers at `WIZ_LEV_NO_MANTRA` (skill 60+); caster below threshold.

**Diagnostic:** Check `getWizardryLevel()` return value against `WIZ_LEV_NO_MANTRA`.

**Fix:** Train Wizardry to 60+ for verbal requirement bypass.

---

### Components not merging when picked up

**Symptom:** Player has multiple stacks of the same component instead of merged charges.

**Cause:** Components have different vnums, one or both have cost <= 0 (free from leveling), or combined charges would exceed 100.

**Diagnostic:** Compare vnums between components. Check `obj_flags.cost` values (both must be > 0). Check if combined charges would exceed 100.

**Fix:** Ensure components have identical vnums and both have cost > 0. Components with different vnums will not merge. Free components from leveling (cost 0) never merge.

---

### Component placement not spawning

**Symptom:** Components defined in component_placement never appear in the world.

**Cause:** Timing conditions too restrictive, low variance, or max_number already reached.

**Diagnostic:** Check variance field (percentage chance per placement attempt). Verify hour/day/month/weather conditions are being met. Check existing component count against max_number.

**Fix:** Increase variance value, expand timing condition ranges, or increase max_number limit. Implement component decay if max_number consistently reached.
