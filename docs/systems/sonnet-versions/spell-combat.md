---
title: Offensive Spell System
category: critical
keywords: [offensive spells, spell damage, genericDam, bSuccess, critSuccess, isLucky, reconcileDamage, immunity system, area effect, VICTIM_DEAD, CASTER_DEAD, spellLuckModifier]
related: [spell-skill-framework.md, damage-pipeline.md, combat-formulas.md, delete-flags.md, affects-system.md]
primary_symbols:
  functions: [genericDam, bSuccess, critSuccess, isLucky, reconcileDamage, spellLuckModifier, getImmunity]
  classes: [TBeing, TMagicItem]
  files: [code/misc/skill_dam.cc, code/misc/spell_info.cc, code/misc/crit_combat.cc, code/misc/being.cc, code/misc/damage.cc, code/disc/disc_mage_fire.cc]
---

# Offensive Spell System

## Overview

The offensive spell system provides damage-dealing magical abilities across eight classes organized into twenty-plus disciplines. Over fifty spells use centralized damage calculation through genericDam for consistent balancing. The system features three-layer success mechanics where spell success feeds into critical success checks which then feed into save checks, each modifying the final damage outcome.

**Memory safety is critical.** reconcileDamage returns negative one when victims die, not a DELETE flag. Magic item versions must translate spell return flags to DELETE flags. Area spells must advance iterators before deletion and handle victim cleanup directly rather than returning death flags.

### Architecture

Spells follow a three-function pattern. The core implementation accepts level, known percentage, and advanced learning parameters to calculate and apply damage. The magic item version calls the core with item statistics and translates spell return flags to DELETE flags for object handling. The player casting version validates class access, retrieves character statistics, and calls the core implementation.

Success proceeds through distinct layers. First bSuccess determines whether the spell succeeds based on task difficulty, skill learning, focus stat, and karma. If successful, critSuccess determines damage multipliers ranging from normal to triple damage. Finally isLucky allows victims to save for half damage. Each layer modifies the outcome independently.

The immunity system provides twenty-eight resistance types covering elemental, energy, and alignment-based damage. Complete immunity at one hundred percent causes spell failure. Partial immunity below one hundred percent reduces damage proportionally. Immunity checks occur before damage calculation to avoid wasted computation.

### Key Constraints

reconcileDamage returns negative one on victim death. Using IS_SET_DELETE to check this return value will never trigger because negative one is not a bitmask pattern. Check explicitly for negative one and return the appropriate spell death flag.

Area effect spells must advance iterators before any operation that might delete victims. The pattern uses post-increment to capture the current element while advancing the iterator. This prevents iterator invalidation when victims are removed from the container.

Magic item spell versions operate in a different deletion context than direct casting. They must translate VICTIM_DEAD to DELETE_VICT and CASTER_DEAD to DELETE_THIS so the item handling code can properly clean up deleted beings.

## Patterns

### Standard Single-Target Spell

Core implementation calculates base damage using getSkillDam which internally calls genericDam. Success check through bSuccess determines whether the spell lands. On success, critSuccess determines damage multipliers. Apply save check through isLucky which may halve damage. Messages display using act function for different observers. Call reconcileDamage to apply final damage and check for negative one to detect death.

Return SPELL_SUCCESS combined with VICTIM_DEAD when reconcileDamage returns negative one. Return SPELL_SUCCESS alone when damage applies without death. Return SPELL_FAIL for missed casts. Return SPELL_CRIT_FAIL for critical failures, combined with CASTER_DEAD if backfire kills the caster.

### Critical Success Handling

The critSuccess function returns an enumeration indicating damage multiplier tier. CRIT_S_NONE applies no multiplier. CRIT_S_DOUBLE multiplies damage by two. CRIT_S_TRIPLE and CRIT_S_KILL both multiply damage by three, with CRIT_S_KILL displaying enhanced messages. Check the return value in a switch statement and apply multipliers before save checks.

### Critical Failure Handling

The critFail function determines backfire severity. CRIT_F_NONE produces a simple failure message through nothingHappens. CRIT_F_HITSELF damages the caster, requiring reconcileDamage check for CASTER_DEAD flag. CRIT_F_HITOTHER redirects the spell to a random target in the room.

Backfire damage typically uses half the calculated spell damage. Check reconcileDamage return value against negative one and return SPELL_CRIT_FAIL combined with CASTER_DEAD if the caster dies from backfire.

### Area Effect Implementation

Calculate base damage once before iteration. Iterate through roomp stuff container using the post-increment pattern to advance the iterator before accessing elements. Filter targets to exclude the caster, group members, immortals, and beings the caster cannot see.

Apply area effect damage penalty of seventy-five percent by multiplying damage by three and dividing by four. Call reconcileDamage for each valid target. When reconcileDamage returns negative one, immediately delete the victim and set the pointer to nullptr to prevent use-after-free.

Return SPELL_SUCCESS without VICTIM_DEAD flag because area spells handle victim deletion internally rather than propagating death flags to callers.

### Magic Item Translation

Initialize an integer to hold DELETE flags. Call the core spell implementation with magic item level, learnedness, and zero for advanced learning. Check the return value using IS_SET for VICTIM_DEAD and CASTER_DEAD flags. Translate VICTIM_DEAD to DELETE_VICT and CASTER_DEAD to DELETE_THIS using ADD_DELETE. Return the translated flags.

This translation allows the object system to properly handle being deletion based on standard DELETE flag conventions rather than spell-specific return values.

### Immunity Checking

Check getImmunity for the relevant damage type before calculating damage. If immunity equals or exceeds one hundred percent, display immunity messages and return SPELL_FAIL. Calculate the immunity percentage and reduce damage proportionally by multiplying damage by one hundred minus immunity percent and dividing by one hundred.

Common immunity types include IMMUNE_HEAT for fire spells, IMMUNE_COLD for ice spells, IMMUNE_ACID for acid spells, IMMUNE_ELECTRICITY for lightning spells, IMMUNE_AIR for wind spells, IMMUNE_WATER for water spells, IMMUNE_EARTH for earth spells, and IMMUNE_HOLY for divine spells.

### Save for Reduced Damage

After calculating final damage including critical success multipliers and immunity reductions, call isLucky with spellLuckModifier as the parameter. If isLucky returns true, display save message using the SV macro and halve the damage value before calling reconcileDamage.

The save check provides victims a final chance to reduce incoming damage based on their luck stat and the spell's difficulty modifier.

## Reference

### Spell Return Value Constants

SPELL_SUCCESS has bit value two indicating successful spell execution. SPELL_FAIL has bit value four indicating failed spell execution. SPELL_CRIT_FAIL has bit value eight indicating critical failure. CASTER_DEAD has bit value sixty-four indicating caster died during execution. VICTIM_DEAD has bit value one hundred twenty-eight indicating victim died from spell damage.

Combine flags using bitwise OR or addition since values do not overlap. Check individual flags using IS_SET macro.

### Critical Success Types

CRIT_S_NONE applies standard damage. CRIT_S_DOUBLE applies two times damage multiplier. CRIT_S_TRIPLE applies three times damage multiplier. CRIT_S_KILL applies three times damage multiplier with enhanced death messages.

### Critical Failure Types

CRIT_F_NONE produces simple failure message. CRIT_F_HITSELF backfires damage onto caster. CRIT_F_HITOTHER redirects spell to random room occupant. Additional failure types exist for various spell-specific outcomes.

### Core Damage Functions

getSkillDam on TBeing accepts victim pointer, spell number, caster level, and advanced learning. Returns base damage value before multipliers and saves. Internally calls genericDam with spell-specific parameters.

genericDam on TBeing accepts caster pointer, spell number, level, advanced learning, and base damage value. Returns final damage after applying class multipliers, task difficulty, stat modifiers, random variance, area effect penalties, NPC damage reduction, and PvP damage reduction.

bSuccess on TBeing accepts skill known percentage and spell number. Returns boolean indicating whether spell succeeds based on task difficulty, skill learning, focus stat, and karma modifiers.

critSuccess accepts caster pointer and spell number. Returns critSuccessTypeT enumeration indicating damage multiplier tier.

critFail accepts caster pointer and spell number. Returns critFailTypeT enumeration indicating failure severity.

isLucky on TBeing accepts integer luck modifier. Returns boolean indicating whether victim saves against spell effect.

spellLuckModifier on TBeing accepts spell number. Returns integer modifier affecting victim save chance based on spell difficulty.

reconcileDamage on TBeing accepts victim pointer, damage amount, and spell number. Returns negative one if victim dies, otherwise returns zero or positive damage dealt. Does not return DELETE flags despite the name pattern.

getImmunity on TBeing accepts immunityTypeT enumeration. Returns integer percentage from zero to one hundred plus indicating resistance level.

### Immunity Types

IMMUNE_HEAT covers fire damage. IMMUNE_COLD covers ice damage. IMMUNE_ACID covers acid damage. IMMUNE_POISON covers poison damage. IMMUNE_AIR covers wind damage. IMMUNE_ENERGY covers pure energy damage. IMMUNE_ELECTRICITY covers lightning damage. IMMUNE_WATER covers water damage. IMMUNE_EARTH covers earth damage. IMMUNE_HOLY covers divine damage. Twenty-eight total types exist covering all damage flavors.

### Message Display Macros

CS macro displays critical success message for spell number. CF macro displays critical failure message for spell number. SV macro displays save message for spell number. These macros lookup and format predefined messages from the spell information database.

act function displays formatted messages to different observer groups. First parameter provides message template with special tokens. TO_CHAR sends to caster. TO_VICT sends to victim. TO_NOTVICT sends to other room occupants. TO_ROOM sends to all room occupants.

## Implementation

### Damage Calculation Formula

genericDam starts with base damage from class amount multiplied by lag rounds multiplied by caster level. Class amount ranges from point five to four point zero providing spell-specific damage scaling. Lag rounds come from the lag_t enumeration representing zero to ten point eight seconds of casting delay.

Apply task difficulty modifier retrieved from getSkillDiffModifier which ranges from thirty-five to one hundred ten percent based on skill difficulty relative to character level. Multiply base damage by this modifier to scale damage based on task appropriateness.

Apply primary stat modifier using plotStat with STAT_CURRENT type, spell-specific modifier stat, minimum value point eight, maximum value one point two five, and baseline one point zero. This scales damage by caster's relevant casting stat like intelligence or wisdom.

Add random variance by adding a random number between negative level divided by four and positive level divided by two. This creates unpredictable damage ranges while maintaining average expectations.

Check isAreaSpell for the spell number. If true, multiply damage by point seven five to apply area effect penalty compensating for hitting multiple targets.

Check if caster isNPC. If true, multiply damage by point five one nine five to reduce NPC damage output to approximately half of player damage for balance.

Check if victim isPc. If true, multiply damage by point five to reduce PvP damage to half to extend combat duration between players.

Return the final calculated damage value as an integer.

### Success Calculation

bSuccess calculates a limit value starting at one hundred percent. Multiply by getSkillDiffModifier to scale by task difficulty. Multiply by known percentage divided by one hundred to scale by skill learning. Multiply by getStatMod for STAT_FOC to scale by focus stat. Multiply by plotStat with STAT_CURRENT, STAT_KAR, minimum point nine, maximum one point one two five, baseline one point zero to scale by karma.

Convert limit to integer by multiplying by one hundred. Generate random number from zero to ninety-nine. Return true if random number is less than limit, false otherwise. Higher skill, better stats, and appropriate task difficulty increase success chance.

### Critical Success Determination

critSuccess in crit_combat.cc generates random roll modified by luck and skill factors. Compare roll against threshold tables to determine outcome tier. Low rolls produce CRIT_S_NONE. Medium rolls produce CRIT_S_DOUBLE. High rolls produce CRIT_S_TRIPLE. Exceptional rolls produce CRIT_S_KILL.

Skill level, relevant stats, and character luck affect the roll. Higher values increase chances of better critical outcomes.

### Save Check Mechanics

isLucky generates random roll modified by victim's luck stat and the provided modifier parameter. spellLuckModifier retrieves spell-specific difficulty affecting save chance. Higher spell difficulty makes saves harder. Higher victim luck makes saves easier.

Return true when victim successfully saves, false when save fails. Callers typically halve damage on successful save.

### Death Detection Mechanism

reconcileDamage in damage.cc applies damage to victim's hit points. Track damage dealt and apply combat flags. Check if victim hit points drop to zero or below. If victim dies, call die function to handle death processing. Return negative one to signal death occurred.

The negative one return value is distinct from normal damage returns which are zero or positive. Callers must check explicitly for negative one rather than using bitmask checks.

### Iterator Safety in Area Spells

The stuff container in TRoom holds TThing pointers including TBeing instances. Standard iterator increment happens after loop body execution. If loop body deletes an element, the iterator becomes invalid before increment occurs.

Post-increment iterator pattern captures current element while advancing iterator before loop body. Assign current element using dereference and post-increment in single expression. The iterator now points to the next element regardless of current element deletion.

Dynamic cast the TThing pointer to TBeing pointer to identify valid targets. Perform all filtering and damage application. Delete victims immediately when reconcileDamage returns negative one. Set victim pointer to nullptr even though it goes out of scope, as defensive programming against accidental reuse.

### Flag Translation Requirements

The object system expects DELETE_THIS and DELETE_VICT flags following the standard DELETE flag pattern. Spell functions return CASTER_DEAD and VICTIM_DEAD flags specific to spell execution.

Magic item versions bridge this gap by checking spell return flags and setting corresponding DELETE flags. The translation uses IS_SET to check spell flags and ADD_DELETE to set DELETE flags. This maintains proper ownership semantics where object code handles being deletion.

### Group Membership Filtering

Area spells must avoid hitting group members. Call inGroup method passing potential victim as parameter. This checks whether victim shares same master pointer as caster and both have AFF_GROUP flag set. Skip victims where inGroup returns true.

The check prevents friendly fire damage which would break group combat tactics and feel unintuitive to players.

### Visibility and Targeting

Call canSee method passing potential victim as parameter before revealing information or applying effects. This respects invisibility, darkness, blind status, and other visibility modifiers. Skip victims the caster cannot see.

Some spells may intentionally ignore visibility for balance or flavor reasons, but most offensive spells respect targeting restrictions.

## Troubleshooting

### Crash after reconcileDamage Call

Symptom occurs when code continues executing after reconcileDamage returns negative one. The victim pointer now references deleted memory. Any access to victim members or methods produces use-after-free crash.

Check all code paths after reconcileDamage calls. Ensure immediate return when negative one is detected. Never cache victim pointers across reconcileDamage calls without checking return value.

### Magic Item Spells Not Cleaning Up

Symptom occurs when magic item spell versions return VICTIM_DEAD or CASTER_DEAD flags instead of DELETE_VICT or DELETE_THIS flags. The object system does not recognize spell flags and fails to delete beings.

Verify all magic item spell versions contain flag translation logic. Check IS_SET for spell flags and ADD_DELETE for DELETE flags. The translation is mandatory for proper memory management.

### Area Spell Iterator Crashes

Symptom occurs when iterator is incremented after victim deletion. The deleted victim was the iterator target causing invalid iterator state. Next iteration attempt crashes.

Check all area spell loops use post-increment pattern. The statement should read element assignment equals asterisk open-paren iterator post-increment close-paren. Verify deletion happens after iterator advancement.

### Spell Success Check Wrong Flag Type

Symptom occurs when code checks reconcileDamage return with IS_SET_DELETE macro. Negative one is not a valid bitmask so the check always fails. Victims die but code does not detect death.

Replace IS_SET_DELETE checks with explicit negative one comparison. The return value is not a bitmask despite following DELETE naming conventions for caller handling.

### Immunity Not Preventing Damage

Symptom occurs when immunity check happens after damage application or uses wrong immunity type. Victims take full damage despite high immunity values.

Move immunity check to top of spell function before any damage calculation. Verify immunity type matches spell damage type. Complete immunity at one hundred percent or higher should return SPELL_FAIL immediately.

### Missing Save Mechanics

Symptom occurs when spells apply damage without save check. Victims cannot reduce damage through luck and defensive stats. Damage feels unfair and unbalanced.

Add isLucky check after critical success handling but before reconcileDamage. Pass spellLuckModifier as parameter to properly scale save difficulty. Halve damage on successful save.

### Wrong Critical Multiplier

Symptom occurs when critical success check is missing or damage multipliers apply incorrectly. Spells deal inconsistent damage not matching critical tier.

Add critSuccess call after bSuccess check. Use switch statement to handle all critical tiers. Apply multipliers before save check but after base damage calculation. CRIT_S_KILL and CRIT_S_TRIPLE both use three times multiplier.

### Group Member Friendly Fire

Symptom occurs when area spells damage group members. This breaks group tactics and feels broken to players.

Add inGroup check in target filtering section. Skip any victim where caster inGroup victim returns true. This check must happen before damage application.

### Caster Self-Damage Not Handled

Symptom occurs when critical failure damages caster but code does not check for caster death. The caster pointer becomes invalid but execution continues.

Check reconcileDamage return value when applying backfire damage to caster. Return SPELL_CRIT_FAIL combined with CASTER_DEAD when negative one is detected. Never access caster members after potential caster death.

### Player Casting Version Missing Validation

Symptom occurs when player casting version does not check class access. Players cast spells they should not know. Game balance breaks.

Call bPassClassChecks with caster pointer and spell number before attempting spell execution. Return FALSE with appropriate error message when check fails. Only proceed to core implementation when validation passes.
