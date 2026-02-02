---
title: Object Maintenance
category: important
keywords: [decay_time, struct_points, OBJ_NOTIMER, objectDecay, damageItem, makeScraps, repairPrice, willDent, willTear, willPuncture, genericDamCheck]
related: [object-system.md, material-system.md, weapon-system.md, combat-formulas.md, economy-system.md, task-system.md]
primary_symbols:
  functions: [objectTickUpdate, decayMe, objectDecay, damageItem, makeScraps, repairPrice, repair_time, maxFix, willDent, willTear, willPuncture, genericDamCheck]
  classes: [TObj, objFlagData, TBaseCorpse, TPlayerCorpse]
  files: [code/code/misc/periodic.cc, code/code/misc/combat.cc, code/code/misc/repair.cc, code/code/misc/materials.cc, code/code/task/task_blacksmithing.cc]
---

## Overview

Objects in SneezyMUD deteriorate through two independent health systems. The decay timer system deletes temporary items after a countdown expires. The structure points system tracks physical durability that degrades from combat damage and environmental effects.

Every object stores a decay timer in its object flags. When enabled, this timer decrements each game tick while the object resides in a room. Objects in inventory or equipment slots do not decay. Timer value -1 disables decay entirely, making the item permanent. Timer value 0 triggers deletion on the next tick. Decay serves different gameplay purposes across object types: corpses decompose to prevent permanent loot storage, food spoils to create supply logistics, summoned items vanish when magic expires, and temporary quest items disappear after use.

Structure points represent physical integrity. Combat damage from weapons reduces structure points based on material susceptibility to blunt, slashing, and piercing attacks. Items display condition descriptions ranging from "brand new" to "destroyed" based on the ratio of current to maximum structure points. When structure points reach zero, the item scraps into unusable debris unless it carries a monogram personalization.

Repair mechanics restore structure points through either NPC repair shops or player skills. Repair shops charge based on item value, damage percentage, and material costs. Player repair requires class-specific skills, appropriate tools, and raw material commodities. Both systems cap maximum repair quality at 95% of original maximum to prevent indefinite item preservation.

The systems interact at key points. Decay ignores structure damage entirely and deletes items regardless of condition. Scrapping from zero structure creates debris but does not trigger decay timers on the resulting trash. Monogrammed items receive special protection from both systems: they cannot scrap from structure damage and require lower material costs for repair.

## Patterns

### Decay Implementation Pattern

Type-specific decay behavior overrides the base objectDecay virtual function. The base implementation relocates contained items before deletion. Corpse types log player death information and handle equipment recovery. Light objects display "burns out" messages. Food items show spoilage descriptions. Portal objects delete silently.

Decay processing occurs in objectTickUpdate during the game tick pulse. The system checks whether decay_time exceeds -1, calls decayMe to decrement the timer only for room-located objects, then invokes objectDecay when the timer reaches zero. If objectDecay returns non-zero, deletion proceeds immediately. Otherwise, the system relocates contents to parent containers or rooms before deletion.

Location-specific messaging informs players of decay events. Equipped items "decay into nothing" with appropriate verb conjugation. Inventory items "disintegrate in your hands" to indicate the player held them. Room-located items "fade into insignificance" as passive observation. Message selection uses the act system with appropriate flags for third-person and plural forms.

### Combat Damage Pattern

Material susceptibility determines whether attacks damage equipment. The genericDamCheck function implements a two-stage probability check: 30% base chance to evaluate damage, then comparison between attacker hardness and defender susceptibility. Hardness values cap at 30, with susceptibility from material properties. The combined value must exceed a random number between 20 and 120 for damage to occur.

Damage type functions willDent, willTear, and willPuncture check specific material vulnerabilities. Blunt weapons dent metals and minerals. Slashing weapons tear cloth and leather. Piercing weapons puncture soft materials. Each function queries material properties to determine susceptibility, then passes attacker hardness and defender susceptibility to genericDamCheck.

Combat flow calls damage type functions from dentItem, tearItem, and pierceItem methods on TBeing. After confirming the damage check succeeds, the system displays appropriate messages describing dents, chips, tears, or punctures. The damageItem method then reduces structure points by the damage amount, clamped to prevent structure from falling below zero. When structure reaches zero, makeScraps handles item destruction.

Arena fights disable all equipment damage by checking the ROOM_ARENA flag before damage calculations. This prevents permanent item loss during practice combat and competitive matches.

### Repair Shop Pattern

Repair cost calculation subtracts material cost portion from base item value, computes the percentage of structure being restored, applies repair economy gold modifiers, and scales by repair percentage. Destroyed items with structure points at or below zero incur a 5x cost multiplier. Material costs add to the final price based on findRepairMaterials calculation.

Repair time derives from structure point damage divided by assumed success rate, converted to real seconds, multiplied by a 1.25 NPC service penalty, and modified by shop speed settings. The base formula assumes 30% success rate matching player repair skill difficulty.

Maximum repair quality limits prevent full restoration. The maxFix function returns 95% of maximum structure points minus depreciation, scaled by shop quality modifiers. Owned shops can adjust quality between 0 and 1.0 to create repair shop variety.

Item submission uses the standard shop give command. Players give items to repair NPCs, receiving numbered tickets in exchange. The NPC stores items in shop inventory with repair_time counters. When repair completes, players give tickets back to retrieve repaired items with rust removed and structure restored to maximum repair quality.

### Player Repair Pattern

Class-specific repair skills require appropriate tools and work surfaces. Warriors use SKILL_BLACKSMITHING with hammers, tongs, forges, and anvils for metal items. Shamans use SKILL_REPAIR_SHAMAN with scalpels, forceps, and operating tables for organic materials. Monks use SKILL_REPAIR_MONK with ladles, soil, and water for wood. Other classes have equivalent skills with specific material affinities.

Repair tasks consume raw material commodities based on item weight and structure damage. The system calculates materials needed as 10% of item weight divided by max structure points, scaled by repair percentage. Monogrammed items receive 75% material cost reduction. The task searches inventory for matching commodity materials and deducts units as repair progresses.

Success checks compare skill value against a random number modified by dexterity reaction. Critical failure on roll of 101 always fails regardless of skill. Success adds 1 structure point per repair action. Failure subtracts 1 structure point if current structure exceeds zero. This creates a risk-reward tension where low-skilled repairs risk further damage.

### Scrapping Pattern

The makeScraps function handles zero-structure item destruction. It removes ITEM_BURNING flags for safety, drops liquid from containers into rooms, relocates contained items to parent containers or rooms, then checks for monogram personalization. Monogrammed items call scrapMonogrammed for special handling that preserves the item in damaged state rather than destroying it.

Non-monogrammed items display scrap messages describing how they fall apart, create TTrash objects with appropriate descriptions based on material type, then delete the original object. The trash object weighs the same as the original and inherits material properties for description purposes.

Scrapping can return DELETE_THIS when item deletion occurs, requiring callers to check return values and avoid dereferencing destroyed objects. The pattern matches the DELETE flag system used throughout combat and item manipulation code.

## Reference

### Decay Timer Values

OBJ_NOTIMER constant equals -1 and disables decay entirely. Zero triggers immediate decay on next tick. Positive values count down remaining ticks until decay. Decay timer only decrements for objects in rooms, not inventory or equipment slots.

### Object Condition Display

Condition ratio calculations divide current structure points by maximum structure points. Greater than 100% shows "better than new" in white. Exactly 100% shows "brand new" in cyan. Above 90% shows "like new" in cyan. Above 80% shows "excellent" in blue. Above 70% shows "very good" in blue. Above 60% shows "good" in purple. Above 50% shows "fine" in purple. Above 40% shows "fair" in green. Above 30% shows "poor" in green. Above 20% shows "very poor" in yellow. Above 10% shows "bad" in orange. Above 0.1% shows "very bad" in red. At or below 0.1% shows "destroyed" in red.

### Structure Point Accessors

getStructPoints returns current structural health. setStructPoints assigns current structure. addToStructPoints modifies current structure by positive or negative amount. getMaxStructPoints returns maximum structural health. setMaxStructPoints assigns maximum structure. All methods operate on objFlagData members struct_points and max_struct_points.

### Unrepairable Item Conditions

Not rentable items cannot be repaired. Items already at maximum structure reject repair. Items with maxed depreciation have permanent damage. Items with zero repair time are already repaired. Virtual items with objVnum -1 cannot be repaired. ITEM_NODROP cursed items cannot be repaired. ITEM_BURNING items pose safety hazards. Items over max_exist limits fall under reclamation contracts. Non-empty containers must be emptied before repair.

### Repair Command Syntax

VALUE inspects repair cost and time for one item. VALUE all.damaged inspects all damaged inventory items. GIVE submits items to repair NPCs. GIVE all.damaged submits all damaged items. GIVE ticket retrieves repaired items.

### Repair Skill Requirements

SKILL_BLACKSMITHING requires hammers, tongs, forges, and anvils for metal repairs by warriors. SKILL_REPAIR_SHAMAN requires scalpels, forceps, and operating tables for organic repairs by shamans. SKILL_REPAIR_MONK requires ladles, soil, and water sources for wood repairs by monks. Other class skills have equivalent tool and material requirements.

### Object Lock System

getLocked returns current lock state. setLocked assigns lock state. Locked items temporarily disable damage processing for scripted scenarios.

## Implementation

### Decay Processing Flow

objectTickUpdate in periodic.cc implements the main decay loop. It iterates through all objects during game ticks, checking decay_time against -1 to identify decaying objects. For decaying objects in rooms, decayMe decrements the timer. When timers reach zero, objectDecay executes type-specific decay behavior.

decayMe performs a simple decrement operation conditional on two checks: decay_time must exceed zero and in_room must not equal Room::NOWHERE. This prevents inventory items from decaying and ensures objects only decay while existing in the game world.

objectDecay virtual function allows type specialization. TBaseCorpse logs corpse information and displays decay messages. TPlayerCorpse extends corpse logging with player death details and content relocation logic. TLight shows "burns out" messages. TFood shows spoilage messages. TPortal deletes silently. Default TObj implementation relocates contents then deletes.

Content relocation during decay searches for parent containers first, then falls back to room placement. ITEM_NEWBIE objects inside decaying containers are destroyed except in donation rooms and starting locations. Tables and furniture preserve items placed on them through special handling.

### Structure Damage Flow

damageItem accepts a damage amount parameter and clamps it to prevent negative values that would heal items. The function adds the negative damage amount to current structure points, clamped to prevent structure from falling below zero. When structure reaches zero, makeScraps handles item destruction.

makeScraps first clears ITEM_BURNING flags. For liquid containers, it drops liquid content into rooms. All contained items relocate to parent containers or rooms. Monogram checks determine whether to call scrapMonogrammed or proceed with normal scrapping.

Normal scrapping displays appropriate messages based on item location. Equipped items show scrap messages to the wearer. Inventory items show scrap messages to the holder. Room items show scrap messages to room occupants. After messaging, TTrash objects are created with descriptions matching original material and weight. Original objects are deleted and DELETE_THIS may be returned.

scrapMonogrammed provides alternative handling for personalized items. Instead of creating trash, it drops the damaged item to preserve player investment in monogrammed equipment. This allows recovery and repair rather than permanent loss.

### Repair Shop Processing

repairPrice calculates costs in multiple stages. Base item cost minus material cost portion establishes workmanship value. Percentage repaired relative to maximum structure determines scale. Gold modifier for repair economy adjusts for server economic settings. Material costs from findRepairMaterials add raw material expenses. Destroyed items multiply final cost by 5.

repair_time computes NPC repair duration from structure damage divided by 0.3 success rate, converted to seconds, multiplied by 1.25 NPC penalty, and scaled by shop speed modifiers. Speed values between 0 and 5.0 adjust base time. Default speed of 1.0 applies no modification.

maxFix determines quality cap by subtracting depreciation from maximum structure points, multiplying by 95, and dividing by 100 to get 95% of max. Shop quality modifiers further reduce maximum repair quality for owned shops with quality values below 1.0.

Item submission creates repair tickets as TNote objects containing item references and repair timers. Shop inventory stores actual items with repair_time counters that decrement during ticks. When timers reach zero, items move to ready state. Ticket retrieval deletes tickets, removes items from shop inventory, restores structure points via setStructPoints to maxFix values, removes ITEM_RUSTY flags, and gives items back to players.

### Player Repair Processing

Repair tasks implement multi-stage state machines. Start phase validates item condition, checks tool presence, and verifies material availability. Repair phase executes skill checks each pulse, consuming materials on success, applying structure point modifications, and advancing repair progress. Finish phase completes repair, displays success messages, and ends the task.

ConsumeRepairMats calculates material needs from item weight divided by max structure points, multiplied by 10.0, scaled by repair_mats_ratio of 0.10. Monogrammed items divide material needs by 4 for 25% cost. getRepairMaterial searches inventory for matching commodity materials. Material consumption reduces commodity weight by materials needed divided by 10.

Skill check formula generates random number between 1 and 101, subtracts dexterity reaction multiplied by 3, then compares against skill value. Roll of 101 forces failure regardless of modifiers. Success adds 1 to structure points. Failure subtracts 1 from structure points if current structure exceeds zero. This creates gradual repair progress with risk of damage on failure.

Tool validation checks inventory and room for required items. Warriors need hammers and tongs in inventory plus forges and anvils in room. Shamans need scalpels and forceps in inventory plus operating tables in room. Monks need ladles in inventory plus soil and water in room. Missing tools abort the repair task with appropriate error messages.

### Damage Susceptibility Checks

genericDamCheck implements the core damage probability algorithm. First stage checks whether random number 0-999 is less than 300, providing 30% base chance to evaluate damage. Second stage caps attacker sharpness at 30, adds defender susceptibility, and compares sum against random number 20-120. Both checks must succeed for damage to occur.

willDent queries material hardness for blunt damage susceptibility. Metals and minerals have high hardness making them vulnerable to denting. willTear queries material flexibility for slashing damage susceptibility. Cloth and leather have high flexibility making them vulnerable to tearing. willPuncture queries material softness for piercing damage susceptibility. Soft materials have high softness making them vulnerable to puncturing.

dentItem, tearItem, and pierceItem methods on TBeing call damage type functions with attacker hardness from weapon or body part. For unarmed attacks, body part material determines hardness. For weapon attacks, weapon material and sharpness determine hardness. Defender equipment provides material properties for susceptibility checks.

Arena check precedes all damage calculations. ROOM_ARENA flag on victim room causes immediate return of FALSE, bypassing all damage processing. This prevents equipment degradation during practice and competitive fights.

### Depreciation System

Depreciation tracking exists in objFlagData with depreciation member variable. getDepreciation currently returns hardcoded 0, disabling the system. setDepreciation and addToDepreciation methods exist for future implementation. When enabled, each repair cycle would add 1 depreciation, and maxFix would subtract accumulated depreciation from maximum structure.

The depreciation concept creates permanent wear that repair cannot remove. Items would eventually reach unrepairable state when depreciation equals maximum structure points. This prevents indefinite item preservation through repeated repairs and creates item lifecycle pressure.

## Troubleshooting

### Items Not Decaying

Verify decay_time is not -1. Check whether object resides in a room versus inventory or equipment. Confirm objectTickUpdate executes during game ticks. Examine objectDecay implementation for type-specific decay prevention logic.

### Unexpected Scrapping

Check current structure points with getStructPoints. Review recent combat logs for damage events. Verify material susceptibility matches expected values. Confirm genericDamCheck probability calculations align with attack frequency.

### Repair Failures

Confirm item passes unrepairable condition checks. Verify player has sufficient talen for shop repairs or sufficient materials for skill repairs. Check shop quality modifiers and maxFix calculations. Review skill values and dexterity modifiers for player repairs.

### Missing Repair Materials

Calculate expected material needs from item weight divided by max structure points multiplied by repair percentage. Verify commodity material types match item material. Confirm material units in inventory match or exceed calculated needs.

### Incorrect Repair Costs

Review repairPrice calculation stages for each cost component. Verify gold_modifier GOLD_REPAIR value matches expected economy settings. Check whether item has zero structure points triggering 5x multiplier. Confirm material costs align with weight and material price.

### Monogram Protection Not Working

Verify item has monogram set through isMonogrammed check. Review scrapMonogrammed call path in makeScraps. Confirm monogrammed items drop rather than create trash. Check material cost reduction applies in ConsumeRepairMats.

### Decay Message Missing

Confirm object type implements objectDecay with appropriate messaging. Verify act calls use correct message strings and flags. Check whether decay occurs in room with observers versus empty room. Review log output for suppressed messages.

### Structure Points Above Maximum

Check for "better than new" repairs exceeding 100% ratio. Review addToStructPoints calls for positive values without capping. Verify maxFix enforces 95% cap during repairs. Confirm setStructPoints does not bypass maximum structure limits.

### Depreciation Not Accumulating

Recognize getDepreciation returns hardcoded 0 with current implementation disabled. Review commented code in getDepreciation for future activation. Understand depreciation system exists as infrastructure but requires enabling to function.

### Items Disappearing From Repair Shops

Verify repair tickets remain in inventory until item retrieval. Check shop inventory for items with active repair_time counters. Review 15-day abandonment policy from help file. Confirm ticket serial numbers match shop records.
