---
title: Armor and Protection System
category: important
keywords: [armor class, AC, defense, TArmor, TBaseClothing, APPLY_ARMOR, defendRound, Iron Flesh, galvanize, structure points, shields, barding]
related: [combat-formulas.md, combat-rounds.md, equipment-wear.md, object-system.md, tohit-defense.md, weapon-system.md]
primary_symbols:
  functions: [getArmor, defendRound, galvanizeMe, armorPercs, itemAC, getIronFleshArmor]
  classes: [TBaseClothing, TArmor, TArmorWand, TSaddle, THarness]
  files: [code/code/obj/obj_armor.cc, code/code/obj/obj_base_clothing.cc, code/code/misc/being.cc, code/code/misc/combat.cc]
---

# Armor and Protection System

## Overview

The armor and protection system manages defensive equipment, armor class calculations, damage mitigation, and equipment durability. Armor Class is the primary defensive stat determining how hard a character is to hit, with lower values being better (200 is excellent, 800 is poor). Each 25 points of AC provides roughly one level of protection.

The system uses a class hierarchy with TBaseClothing as the abstract base for all wearable items, TArmor for standard protective equipment, and specialized classes like TArmorWand for hybrid items, TSaddle and THarness for mount equipment. Equipment provides AC through APPLY_ARMOR affect modifiers stored in the affected array, not through val0-val3 fields.

Total character AC combines racial baseline (typically 400-600), equipped armor contributions from all 24 wearable slots, monk Iron Flesh bonuses when unarmored, and temporary spell modifiers. The defendRound function converts this AC into a defense bonus that determines hit probability in combat. PCs use the formula (armor - 500) * 2/3, while mobs use (armor - 400) * 5/6, with PC defense capped based on level.

Structure points track equipment durability, affecting item valuation, repair eligibility, and galvanize spell success. Low structure items are worth less and may be too damaged to fix. The galvanize spell can permanently increase maximum structure but risks destroying the item on critical failure.

Different equipment slots contribute different percentages to total armor, with shields in the HOLD position providing the largest single AC contribution at 25%, body armor at 15%, and head armor at 10%. Monks gain Iron Flesh AC bonuses from bare skin when slots are unarmored, with paired items creating penalties to discourage mixed equipment setups.

## Patterns

### Querying Armor Class

Use itemAC to retrieve armor contribution from equipment, which sums all APPLY_ARMOR modifiers in the affected array. Call getArmor on characters to calculate total defensive AC from all sources including racial baseline, equipment, Iron Flesh, and spell affects. Never read val0-val3 fields for armor values as TArmor leaves these empty.

### Creating Armored Items

Add armor protection by populating the affected array with APPLY_ARMOR location entries rather than setting val fields. Negative modifiers improve AC (better defense). Set cur_struct and max_struct for durability tracking. Use armorLevel and structLevel to calculate quality ratings based on slot percentages from armorPercs.

### Defense Calculation Workflow

Call defendRound with target parameter to compute total defense bonus incorporating armor, combat mode, skills, stats, position, and spell effects. For PCs, base defense starts at (armor - 500) * 2/3 capped by level-based maximum. Add combat mode modifiers (defense stance bonus, berserk penalty), skill bonuses (Advanced Defense, Chivalry, Defense skill), agility stat modifier ranging -67 to +84, position bonuses/penalties (mounted +level/4, sitting -level/4), and spell effects like Aura Guardian.

### Monk Iron Flesh Mechanics

Check hasQuestBit for MONK_IRON_FLESH_SKILL, then iterate equipment slots to identify unarmored positions. Calculate AC bonus using slot-specific values from getIronFleshArmor multiplied by skill percentage. Apply paired item penalties by removing Iron Flesh from secondary slots when primary slot is equipped (e.g., wearing left leg armor removes right leg Iron Flesh bonus).

### Shield Usage

Detect shields with isShield checking for "shield" keyword in name. Equip to HOLD_LEFT slot, providing 25% AC contribution (largest single slot). During combat, shields enable parry mechanics blocking incoming attacks and triggering CMD_OBJ_BEEN_HIT spec procs. Shields prevent dual wielding by occupying the secondary hand.

### Repairing and Reinforcing Armor

Use objectRepair for shop-based repairs restoring structure points to damaged items. Apply galvanizeMe spell to permanently increase maximum structure by 1 on success, with failures reducing structure by 2 and critical failures destroying the item. Only galvanize items with at least 2 current and maximum structure points to meet prerequisites.

### Mount Equipment Application

Equip TSaddle items to improve mount control and provide armor benefits to rider. Use THarness for mount safety equipment and carrying capacity bonuses. Both follow TBaseClothing armor evaluation with isBarding checking for barding keyword in mount-specific armor pieces.

## Reference

### Armor Class Hierarchy

TBaseClothing serves as abstract base providing armorLevel (calculate armor quality level), structLevel (calculate structure quality level), armorPercs (return AC/structure percentage by slot), suggestedPrice (price based on armor value and stat bonuses), isShield (detect shield items), and isBarding (detect mount armor).

TArmor extends TBaseClothing for standard protective equipment with itemType returning ITEM_ARMOR, galvanizeMe implementing structure reinforcement spell, and empty assignFourValues/getFourValues indicating no val0-val3 usage.

TArmorWand combines TArmor and TWand through multiple inheritance for hybrid armor-wand items with itemType returning ITEM_ARMOR_WAND and suggestedPrice combining both parent prices minus duplicate weight cost.

TSaddle provides riding equipment as TBaseClothing subclass with itemType returning ITEM_SADDLE and zero val0-val3 values.

THarness provides mount safety equipment as TBaseClothing subclass with itemType returning ITEM_HARNESS and zero val0-val3 values.

### Equipment Slots

Twenty-four wearable positions defined in wearSlotT enum spanning WEAR_HEAD through WEAR_EX_FOOT_L, with WEAR_NOWHERE as unused indicator, MIN_WEAR at 1, MAX_HUMAN_WEAR at 20 for standard humanoid slots, and MAX_WEAR at 24 including extra limb positions for multi-legged races.

### Slot Contribution Percentages

AC percentages per slot: HOLD (shield) 25%, WEAR_BODY 15%, WEAR_HEAD 10%, WEAR_WAIST 8%, WEAR_BACK 7%, WEAR_LEG_R/L each 5%, WEAR_ARM_R/L each 4%, WEAR_FOOT_R/L each 3.5%, WEAR_WRIST_R/L each 1.5%, WEAR_NECK 2%, WEAR_FINGER_R/L each 1.5%.

Structure percentages per slot: WEAR_BODY 26%, WEAR_HEAD 11%, WEAR_WAIST 9%, WEAR_BACK 9%, WEAR_LEG_R/L each 7%, HOLD 7%, WEAR_ARM_R/L each 5%, WEAR_FOOT_R/L each 4%, WEAR_WRIST_R/L each 1.5%, WEAR_NECK 2%, WEAR_FINGER_R/L each 0.5%.

### Defense Formula Components

Base defense for PCs: (armor - 500) * 2 / 3, capped at GetMaxLevel * 1000 / 60 + GetMaxLevel.

Base defense for mobs: (armor - 400) * 5 / 6, no level cap.

Combat mode modifiers: ATTACK_DEFENSE adds GetMaxLevel / 4, ATTACK_OFFENSE subtracts GetMaxLevel / 4, ATTACK_BERSERK subtracts GetMaxLevel * 8 * (100 - berserk_skill) / 100.

Skill modifiers: SKILL_ADVANCED_DEFENSE adds max(1, skillValue / 10), SKILL_CHIVALRY when mounted adds 159 * max(10, skillValue) / 100, SKILL_DEFENSE adds GetMaxLevel * skillValue / 100, SKILL_OOMLAT scales armor with formula armor += armor * skillValue / 250 then recalculates base defense.

Agility modifier: (int)(335 * getStatMod(STAT_AGI) - 335) ranging approximately -67 to +84.

Position modifiers: POSITION_MOUNTED adds GetMaxLevel / 4 + 1, POSITION_FLYING adds GetMaxLevel / 3 + 1, POSITION_RESTING subtracts GetMaxLevel / 3 + 1, POSITION_SITTING subtracts GetMaxLevel / 4 + 1, with SKILL_GROUNDFIGHTING reducing penalties by positionMod * (100 - skillValue) / 100.

Spell modifiers: SPELL_AURA_GUARDIAN adds 40 defense.

### Iron Flesh Slot Values

Body -184, waist -98, head -86, back -86, leg_r -61, leg_l -61, arm_r -49, arm_l -49, neck -49, hand_r -37, hand_l -37, wrist_r -24, wrist_l -24, foot_r -24, foot_l -24, finger_r -12, finger_l -12. Final AC contribution multiplied by getSkillValue(SKILL_IRON_FLESH) / 100.0 for each unarmored slot.

### Galvanize Spell Outcomes

Success when bSuccess returns true: addToMaxStructPoints(1), addToStructPoints(1), return SPELL_SUCCESS.

Failure when bSuccess returns false and critFail returns false: addToMaxStructPoints(-2), addToStructPoints(-2), return SPELL_CRIT_FAIL.

Critical failure when critFail returns true: item destroyed via CF(SPELL_GALVANIZE), return SPELL_CRIT_FAIL_2.

Prerequisites: getMaxStructPoints >= 2 and getStructPoints >= 2 or spell fails immediately.

### Armor Tier Classification

Tier_Clothing: no class restrictions or race-only restrictions (robes, shirts, pants).

Tier_Light: anti-mage and anti-shaman flags (leather armor, light robes).

Tier_Medium: Tier_Light restrictions plus anti-monk and anti-thief flags (ringmail, scale mail).

Tier_Heavy: Tier_Medium restrictions plus anti-cleric and anti-ranger flags (plate mail, full plate).

### Object Flags Affecting Armor

ITEM_STRUNG enables customized names and descriptions stored per instance. ITEM_BURNING causes structure damage over time. ITEM_NODROP prevents dropping (cursed). ITEM_ANTI_* flags restrict by class (MAGE, CLERIC, WARRIOR, THIEF, MONK, DEIKHAN, SHAMAN, RANGER). ITEM_MAGIC marks magical armor. ITEM_BLESS marks blessed armor. ITEM_NEWBIE marks starting equipment.

### Wear Flags

ITEM_WEAR_TAKE enables pickup. ITEM_WEAR_BODY, ITEM_WEAR_HEAD, ITEM_WEAR_LEGS, ITEM_WEAR_FEET, ITEM_WEAR_HANDS, ITEM_WEAR_ARMS, ITEM_WEAR_BACK, ITEM_WEAR_WAIST, ITEM_WEAR_NECK, ITEM_WEAR_WRISTS, ITEM_WEAR_FINGERS control slot eligibility. ITEM_WEAR_HOLD enables shield/held equipment.

### Limb Status Flags

PART_MISSING prevents equipping armor in that slot. PART_PARALYZED disables held items. PART_INJURED reduces skill effectiveness for skills like Focused Avoidance with 25% penalty for leg injuries. PART_BLEEDING, PART_INFECTED, PART_BROKEN, PART_BANDAGED, PART_ENTANGLED affect combat performance.

## Implementation

### AC Storage Architecture

TArmor stores AC values exclusively through the affected array with location field set to APPLY_ARMOR and modifier field containing the AC value (negative improves defense). The val0, val1, val2, val3 fields remain unused with assignFourValues doing nothing and getFourValues returning all zeros. This differs from weapons which use val fields for damage dice.

Structure points track durability using cur_struct for current points and max_struct for maximum capacity, both stored in TObj base class fields rather than subclass-specific storage. Methods addToStructPoints and addToMaxStructPoints modify these values during repair, galvanize operations, and decay.

### Total Armor Aggregation in getArmor

Start with racial baseline from getMyRace()->getBaseArmor() returning values typically 400-600. Iterate all equipment slots from MIN_WEAR to MAX_WEAR, checking each slot for equipped object, verifying affectShouldApply returns true, then summing itemAC which accumulates APPLY_ARMOR modifiers from object's affected array.

For monks with MONK_IRON_FLESH_SKILL quest bit, iterate slots again checking for empty equipment positions. When unarmored, add getIronFleshArmor(slot) * getSkillValue(SKILL_IRON_FLESH) / 100.0 to armor total. For equipped paired items, subtract Iron Flesh contribution from secondary slot using getSecondarySlot to identify paired position and checking that secondary slot is also unarmored.

Process character affects by iterating affected linked list, checking each affectedData node for location equal to APPLY_ARMOR, and adding modifier field to armor total. This captures temporary spell and ability bonuses beyond equipment.

### Mob AC Default Calculation

Calculate default AC as 600 - (20 * mob->ACLevel) providing baseline scaling from level. Compare default against mob->getArmor() which sums equipment and affects. Select minimum (better) value as actual_ac, allowing either level-based progression or equipment to dominate depending on which is more protective.

### Defense Conversion in defendRound

Branch on isPc() to select formula. PCs calculate (armor - 500) * 2 / 3 for base defense, then clamp result with min(defense, GetMaxLevel() * 1000 / 60 + GetMaxLevel()) to enforce level cap. Mobs calculate (armor - 400) * 5 / 6 with no cap, providing better conversion ratio and unlimited scaling.

Apply combat mode by checking isCombatMode for ATTACK_DEFENSE adding GetMaxLevel() / 4, ATTACK_OFFENSE subtracting GetMaxLevel() / 4, or ATTACK_BERSERK subtracting GetMaxLevel() * 8 * (100 - getSkillValue(SKILL_BERSERK)) / 100 where higher berserk skill reduces penalty.

Accumulate skill bonuses checking doesKnowSkill and applying formulas: Advanced Defense adds skill/10 capped at 1 minimum, Chivalry when riding adds 159 * max(10, skill) / 100, Defense skill adds level * skill / 100, Oomlat recalculates armor with scaling factor armor * skill / 250 then recomputes base defense from modified armor.

Apply agility with formula (int)(335 * getStatMod(STAT_AGI) - 335) where getStatMod returns approximately 0.8 to 1.25 producing range roughly -67 to +84.

Determine position modifier from getPosition() returning MOUNTED for +level/4+1, FLYING for +level/3+1, RESTING for -level/3-1, SITTING for -level/4-1. When modifier is negative and character knows SKILL_GROUNDFIGHTING, reduce penalty multiplying by (100 - skill) / 100.

Check affectedBySpell(SPELL_AURA_GUARDIAN) adding 40 if present. Sum all components returning total defense value.

### Slot Percentage Calculation in armorPercs

Function takes wearSlotT slot and bool forStruct parameters. When forStruct is true, return structure percentage via switch statement mapping WEAR_BODY to 0.26, HOLD_RIGHT and HOLD_LEFT to 0.07, WEAR_HEAD to 0.11, continuing through all slots with paired slots each returning half of total contribution.

When forStruct is false, return AC percentage mapping HOLD_RIGHT and HOLD_LEFT to 0.25, WEAR_BODY to 0.15, WEAR_HEAD to 0.10, with paired slots again splitting total percentage. Return 0.0 for unrecognized or unused slots.

### Armor Evaluation Formulas

Calculate armor_ac using (baseACLevel * 25 * ac_perc) + (NEWBIE_AC * ac_perc) where NEWBIE_AC equals 500.0 and ac_perc comes from armorPercs(slot, false). This scales AC linearly with level while providing baseline contribution independent of level.

Calculate structure using max(base_structure, NEWBIE_STR * sqrt(str_perc / BODY_STR)) where NEWBIE_STR equals 30.0, BODY_STR equals 0.26 as body slot reference, and str_perc comes from armorPercs(slot, true). Square root scaling reduces structure contribution for smaller slots relative to body.

For non-paired items, apply additional clamping with max(structure, NEWBIE_STR * sqrt(str_perc / BODY_STR) * 0.5) effectively halving minimum structure for unpaired equipment.

### Pricing in suggestedPrice

Accumulate base weight cost as (int)(10.0 * getWeight() * material_price_modifier) where material_price_modifier varies by material type (leather, metal, cloth).

Add armor value from armorPriceStruct function combining AC and structure contributions using tier-based multipliers and slot percentages.

Iterate MAX_OBJ_AFFECT entries in affected array, checking location field not equal to APPLY_NONE or APPLY_ARMOR, and adding abs(modifier) * 0.25 for each stat bonus. This values attribute bonuses at quarter-point per modifier.

For TArmorWand, call both TArmor::suggestedPrice() and TWand::suggestedPrice() then subtract obj_flags.weight * 10 to remove duplicate weight counting from multiple inheritance.

### Shield Parry Mechanics

During combat hit resolution, check defender's equipment in HOLD_LEFT slot for shield object. Call shieldBlocks() to determine if parry succeeds based on shield skill and random factors.

When parry succeeds, send act messages to char (You parry), room (defender parries), and attacker (defender parries your blow) all including shield object in message.

Invoke shield->checkSpec(defender, CMD_OBJ_BEEN_HIT, "", attacker) to trigger any shield-specific spec proc. Check return code and propagate non-zero values indicating special handling or DELETE flags.

Return TRUE to indicate attack was parried, preventing damage and further hit processing.

### Galvanize Implementation

Check prerequisites by testing getMaxStructPoints() >= 2 and getStructPoints() >= 2. Send failure message and return SPELL_FAIL if either check fails.

Call caster->bSuccess(skillValue, SPELL_GALVANIZE) to determine outcome. On success, call addToMaxStructPoints(1) and addToStructPoints(1), send success messages via act to both caster and room, return SPELL_SUCCESS.

On failure of bSuccess, check critFail(caster, SPELL_GALVANIZE) for critical failure. If true, send destruction messages, invoke CF(SPELL_GALVANIZE) to trigger item destruction cleanup, return SPELL_CRIT_FAIL_2.

On regular failure (bSuccess false, critFail false), call addToMaxStructPoints(-2) and addToStructPoints(-2), send weakening messages, return SPELL_CRIT_FAIL indicating failure without destruction.

### Focused Avoidance Check

Return false immediately if !doesKnowSkill(SKILL_FOCUSED_AVOIDANCE), !awake(), or isAffected(AFF_STUNNED).

Retrieve skill value from getSkillValue(SKILL_FOCUSED_AVOIDANCE) and check for leg injuries with isLimbFlags testing WEAR_LEG_R and WEAR_LEG_L for PART_INJURED flag. If either leg injured, multiply skill by 75 / 100 applying 25% penalty.

Scale by agility with skill = (int)(skill * getStatMod(STAT_AGI)) where stat modifier typically ranges 0.8 to 1.25.

Call bSuccess(skill, SKILL_FOCUSED_AVOIDANCE) which factors FOC stat and perc parameter into success determination. Return boolean result indicating whether attack is completely avoided.

## Troubleshooting

### Armor Not Providing Protection

Check that APPLY_ARMOR affects are present in affected array using itemAC() which should return non-zero value for protective armor. Verify that val0-val3 fields are not being used as TArmor ignores these completely. Confirm equipment slots have affectShouldApply returning true indicating affects are active.

Inspect getArmor() calculation by checking racial baseline, summing equipment AC contributions, verifying Iron Flesh prerequisites for monks, and confirming spell affects in character's affected list. Use defendRound() to confirm AC converts to defense bonus with expected formula.

### Monk Iron Flesh Not Applying

Verify hasQuestBit(MONK_IRON_FLESH_SKILL) returns true indicating quest completion. Check equipment slots are empty as Iron Flesh only applies to unarmored positions. Look for paired item penalties where wearing one side of paired equipment removes Iron Flesh from opposite slot.

Confirm getSkillValue(SKILL_IRON_FLESH) returns non-zero percentage as 0% skill provides no AC. Check slot-specific values from getIronFleshArmor return expected negative values (-184 for body, -98 for waist, etc.).

### Galvanize Spell Failing

Confirm current structure at least 2 with getStructPoints() >= 2 and maximum structure at least 2 with getMaxStructPoints() >= 2. Items below these thresholds return SPELL_FAIL immediately.

Check skill value passed to galvanizeMe affects success rate where higher values increase bSuccess probability. Accept that failure reduces structure by 2 points and critical failure destroys item entirely as intended galvanize risk.

Verify caster has sufficient mana and spell access. Inspect CF(SPELL_GALVANIZE) handling to ensure item destruction cleanup occurs properly on critical failures.

### Defense Bonus Unexpectedly Low

Verify AC value is low (better) rather than high as 200 AC provides better defense than 600 AC. Check level cap on PC defense equals GetMaxLevel() * 1000 / 60 + GetMaxLevel() and confirm defense isn't being clamped below expected value due to low level.

Review combat mode setting as ATTACK_OFFENSE and ATTACK_BERSERK apply penalties subtracting from defense. Confirm skill values for Advanced Defense, Chivalry, Defense, and Oomlat are learning correctly and applying expected bonuses.

Check position with getPosition() as SITTING and RESTING apply substantial penalties (-level/4 and -level/3). Verify Ground Fighting skill reduces these penalties when known. Review agility stat with getStatMod(STAT_AGI) as low agility (0.8 modifier) applies negative defense bonus.

### Shield Not Parrying

Confirm shield equipped in HOLD_LEFT slot as shields in other positions don't provide parry mechanics. Verify isShield() returns true by checking "shield" keyword present in name field.

Check shieldBlocks() success calculation incorporates shield skill level and random factors. Review spec proc return value from checkSpec as non-zero returns can bypass normal parry messaging.

Ensure shield has structure points remaining as destroyed shields may not function properly. Verify combat system calls shield parry logic before damage application.

### Paired Item Penalties Incorrect

Identify paired items with isPaired() checking wear flags indicate paired slot compatibility. Use getSecondarySlot() to retrieve opposite slot position for paired equipment.

Verify Iron Flesh penalty applies only when primary slot equipped but secondary slot empty. Confirm penalty removes Iron Flesh from secondary slot specifically rather than affecting both slots or neither.

Check that full armor sets (both slots equipped) avoid penalties entirely while mixed equipment (one equipped, one bare) triggers penalty as intended to discourage partial armor.

### Mount Equipment Not Functioning

Confirm TSaddle and THarness items have correct itemType returning ITEM_SADDLE and ITEM_HARNESS respectively. Verify mount has appropriate slots for saddle/harness equipment and items are equipped to mount rather than rider.

Check barding detection with isBarding() for armor pieces intended for mount wearing. Review chivalry skill bonus applies only when riding() returns true indicating mounted state.

Ensure mount equipment affects propagate to rider defense calculations when mounted. Verify equipment weight and encumbrance calculations account for mount-worn items separately from rider equipment.
