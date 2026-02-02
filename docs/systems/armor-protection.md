---
title: Armor and Protection System
description: Defensive equipment, armor class calculations, damage mitigation, and equipment durability
keywords: [armor, AC, defense, protection, shields, durability, structure, galvanize, iron flesh]
category: Important Systems
created_by_model: opus
last_updated: 2026-02-01
source_files: [code/code/obj/obj_base_clothing.h, code/code/obj/obj_base_clothing.cc, code/code/obj/obj_armor.h, code/code/obj/obj_armor.cc, code/code/obj/obj_armor_wand.cc, code/code/obj/obj_saddle.cc, code/code/obj/obj_harness.cc, code/code/misc/being.cc, code/code/misc/combat.cc, code/code/misc/limbs.h]
related: [combat-formulas.md, combat-rounds.md, equipment-wear.md, object-system.md, weapon-system.md, tohit-defense.md]
---

# Armor and Protection System

## Overview

Armor Class (AC) is the primary defensive stat determining how hard a character is to hit. Lower AC values provide better protection, with typical values ranging from 200 (excellent) to 800 (poor). The system combines racial baselines, equipment bonuses, skill modifiers, and spell effects into a total AC that converts to defense bonus during combat.

Structure points track equipment durability separately from protective value. Damaged items sell for less and may become unrepairable. The galvanize spell can permanently improve maximum structure at risk of destroying the item.

Monks have a unique relationship with armor through Iron Flesh, gaining AC from bare skin when slots are empty but losing this benefit when wearing mixed armor sets.

## Patterns

### Armor Value Storage

Always store armor AC via `APPLY_ARMOR` affects, never through val0-val3 fields. TArmor ignores `assignFourValues()` entirely.

Always use `itemAC()` to query armor contribution, which properly sums all `APPLY_ARMOR` modifiers on the item.

Never assume `getFourValues()` returns meaningful data for armor items. It returns zeros.

### Equipment Handling

Always check `affectShouldApply()` before counting equipment toward total AC. Some conditions prevent affects from applying.

Always consider paired slot penalties when evaluating monk Iron Flesh. Wearing armor in one paired slot (e.g., left leg) removes Iron Flesh from its partner (right leg).

Never forget shields provide 25% of total AC contribution despite occupying a hold slot rather than a primary armor slot.

### Defense Calculation

Always apply combat mode modifiers after base defense calculation. Defensive stance adds `level/4`, offensive subtracts the same.

Always check skill knowledge before applying skill bonuses. Use `doesKnowSkill()` before `getSkillValue()`.

Never ignore the level cap on PC defense: `level * 16.67 + level` maximum regardless of equipment quality.

### Galvanize Safety

Always verify minimum 2 structure points before attempting galvanize. The spell fails on nearly-destroyed items.

Always handle critical failure outcomes where galvanize destroys the item entirely.

Never chain multiple galvanize attempts on valuable items without accepting the cumulative risk of failure.

## Reference

### AC Component Sources

| Component | Source | Typical Range |
|-----------|--------|---------------|
| Base Armor | `getMyRace()->getBaseArmor()` | 400-600 |
| Equipment | `itemAC()` per slot | -500 to 0 |
| Iron Flesh | Slot-specific values | -500 to 0 |
| Spell Affects | `APPLY_ARMOR` in affected list | -200 to 0 |

### Slot AC Contribution

| Slot | AC % | Structure % |
|------|------|-------------|
| Hold (Shield) | 25% | 7% |
| Body | 15% | 26% |
| Head | 10% | 11% |
| Legs (combined) | 10% | 14% |
| Waist | 8% | 9% |
| Arms (combined) | 8% | 10% |
| Feet (combined) | 7% | 8% |
| Back | 7% | 9% |
| Wrists (combined) | 3% | 3% |
| Fingers (combined) | 3% | 1% |
| Neck | 2% | 2% |

### Iron Flesh AC by Slot

| Slot | AC Value | Slot | AC Value |
|------|----------|------|----------|
| Body | -184 | Waist | -98 |
| Head | -86 | Back | -86 |
| Leg R/L | -61 | Arm R/L | -49 |
| Neck | -49 | Hand R/L | -37 |
| Wrist R/L | -24 | Foot R/L | -24 |
| Finger R/L | -12 | | |

### Defense Skill Bonuses

| Skill | Formula | Range |
|-------|---------|-------|
| Advanced Defense | `skillValue / 10` | +1 to +10 |
| Chivalry (mounted) | `159 * skill / 100` | +16 to +159 |
| Defense | `level * skill / 100` | 0 to +level |
| Oomlat | `armor * skill / 250` | +0% to +40% armor |
| Ground Fighting | Reduces position penalty | up to 100% reduction |

### MOB Default AC

| ACLevel | Default AC |
|---------|------------|
| 0 | 600 |
| 10 | 400 |
| 20 | 200 |
| 30 | 0 |

Formula: `600 - (20 * ACLevel)`

### Armor Tier Classification

| Tier | Class Restrictions |
|------|-------------------|
| Clothing | None or race-only |
| Light | +Mage, +Shaman |
| Medium | +Monk, +Thief |
| Heavy | +Cleric, +Ranger |

### Galvanize Outcomes

| Result | Max Structure | Current Structure |
|--------|---------------|-------------------|
| Success | +1 | +1 |
| Failure | -2 | -2 |
| Critical Failure | Item destroyed | Item destroyed |

## Implementation

### Class Hierarchy

`TBaseClothing` serves as the abstract base for all wearable items. It provides armor evaluation via `armorLevel()`, `structLevel()`, and `armorPercs()`, plus pricing through `suggestedPrice()` and `armorPriceStruct()`. The `isShield()` and `isBarding()` virtuals identify specialized armor types by checking for those keywords in the item name.

`TArmor` inherits from `TBaseClothing` as the primary armor class. It implements `galvanizeMe()` for the reinforcement spell. Critically, `assignFourValues()` and `getFourValues()` are no-ops returning zeros; armor AC must be stored via the `affected[]` array with `APPLY_ARMOR` location.

`TArmorWand` uses multiple inheritance from both `TArmor` and `TWand`, creating hybrid spell-casting armor. Its `suggestedPrice()` combines both parent prices but subtracts duplicate weight cost.

`TSaddle` and `THarness` inherit from `TBaseClothing` for mount equipment. They provide armor benefits to mounted creatures without specialized storage fields.

### Equipment Slots

The `wearSlotT` enum in `limbs.h` defines 24 equipment positions. Humanoid slots run from `WEAR_HEAD` (1) through `HOLD_LEFT` (19), with `MAX_HUMAN_WEAR` at 20. Extended slots (20-23) accommodate creatures with extra limbs. `WEAR_NOWHERE` (0) indicates unequipped.

Paired slots (left/right variants of arms, legs, hands, feet, wrists, fingers) share combined contribution percentages split between them. The hold slots (`HOLD_RIGHT`, `HOLD_LEFT`) are the only positions for shields.

### Total AC Calculation

`TBeing::getArmor()` in `being.cc` aggregates AC from multiple sources. It starts with the racial baseline from `getMyRace()->getBaseArmor()`. Equipment contributes via `itemAC()` calls on each equipped item that passes `affectShouldApply()`.

For monks with Iron Flesh, empty slots add AC based on `getIronFleshArmor()` scaled by skill percentage. The paired item penalty subtracts Iron Flesh from the secondary slot when the primary wears armor.

Finally, character affects with `APPLY_ARMOR` location add their modifiers to the total.

### Defense Conversion

`TBeing::defendRound()` in `combat.cc` converts AC to defense bonus. PCs use formula `(armor - 500) * 2/3` capped at a level-based maximum. MOBs use `(armor - 400) * 5/6` with no explicit cap, giving them better AC-to-defense conversion.

Combat mode applies additive modifiers: defensive stance adds `level/4`, offensive subtracts it, berserk subtracts `level * 8 * (100 - skill) / 100`.

Skill bonuses stack additively. Advanced Defense adds up to +10. Chivalry adds up to +159 when mounted. The general Defense skill adds up to +level. Oomlat uniquely modifies the armor value before conversion rather than adding to defense directly.

Agility contributes `335 * getStatMod(STAT_AGI) - 335`, ranging from -67 to +84.

Position modifiers range from +level/3 (flying) to -level/3 (resting). Ground Fighting skill reduces negative position penalties proportionally.

### Structure and Durability

Structure points use `cur_struct` and `max_struct` fields in `TObj`, not the val fields. The `addToStructPoints()` and `addToMaxStructPoints()` methods modify durability.

`galvanizeMe()` in `obj_armor.cc` handles the reinforcement spell. Success adds +1 to both current and maximum. Regular failure subtracts -2 from both. Critical failure destroys the item entirely. The spell requires at least 2 structure points to attempt.

Structure damage occurs from combat, decay over time, fire (ITEM_BURNING), spell failures, and admin intervention.

### Armor Evaluation

`armorPercs()` returns percentage contributions by slot, with separate values for AC calculation (first parameter false) and structure calculation (first parameter true). Shields dominate AC contribution at 25% but only 7% of structure. Body armor dominates structure at 26% but only 15% of AC.

`armorLevel()` and `structLevel()` calculate quality ratings. The base AC formula: `(baseACLevel * 25 * ac_perc) + (NEWBIE_AC * ac_perc)` where `NEWBIE_AC` is 500. Structure formula: `max(base_structure, NEWBIE_STR * sqrt(str_perc / BODY_STR))` where `NEWBIE_STR` is 30 and `BODY_STR` is 0.26.

### Pricing

`suggestedPrice()` combines weight cost (`weight * 10 * material_modifier`), armor value from `armorPriceStruct()`, and stat bonuses (`modifier * 0.25` per non-armor affect). `TArmorWand` overrides to sum both parent prices minus duplicate weight.

### Shield Mechanics

`isShield()` checks for "shield" in the item name. Shields must equip in `HOLD_LEFT`. During combat, `shieldBlocks()` determines parry success, triggering shield spec procs via `checkSpec()` with `CMD_OBJ_BEEN_HIT`. Successful parries display distinct messages to all participants and return early from the attack.

### Limb Interactions

Body part flags (`PART_MISSING`, `PART_PARALYZED`, `PART_INJURED`) affect armor usage. Missing limbs prevent equipment. Injured legs reduce Focused Avoidance effectiveness by 25%. Paralyzed limbs prevent using held items.

`canFocusedAvoidance()` in `disc_advanced_defense.cc` implements dodge mechanics. It checks leg injury status, applies AGI stat multiplier, and runs a skill success check influenced by FOC stat.

### Strung Items

`swapToStrung()` enables customization by copying prototype strings to the instance and setting `ITEM_STRUNG`. After stringing, name, shortDescr, longDescr, and actionDescr can be freely modified. Strung items persist custom strings to rent files and the `rent_strung` database table.

## Troubleshooting

### Armor provides no protection

**Symptom:** Equipped armor does not reduce incoming damage or improve AC display.

**Cause:** Armor AC stored in val0-val3 instead of APPLY_ARMOR affect.

**Diagnostic:** Check `obj->itemAC()` return value. If zero despite "armor" appearance, affects are misconfigured.

**Fix:** Add proper `APPLY_ARMOR` affect to the item's `affected[]` array with negative modifier.

### Iron Flesh not applying

**Symptom:** Monk shows no AC benefit from empty slots despite having the skill.

**Cause:** Paired slot penalty removing benefit, or skill not learned.

**Diagnostic:** Verify `doesKnowSkill(SKILL_IRON_FLESH)` returns true. Check if any paired slot has equipment.

**Fix:** Remove all armor for full Iron Flesh benefit, or accept partial benefit when mixing.

### Defense lower than expected

**Symptom:** Character defense bonus below what equipment suggests.

**Cause:** Level cap on PC defense, position penalties, or combat mode.

**Diagnostic:** Calculate expected defense manually: `(armor - 500) * 2/3`. Compare to `level * 16.67 + level` cap.

**Fix:** Higher level characters can utilize better armor. Check combat mode and position.

### Galvanize keeps failing

**Symptom:** Galvanize spell consistently fails or destroys items.

**Cause:** Low structure points, low skill, or bad luck.

**Diagnostic:** Verify `getStructPoints() >= 2` and `getMaxStructPoints() >= 2`. Check caster skill level.

**Fix:** Repair items before galvanizing. Improve skill before attempting on valuable items.

### Shield not parrying

**Symptom:** Equipped shield never triggers parry messages.

**Cause:** Item not recognized as shield, wrong slot, or parry check failing.

**Diagnostic:** Verify `isShield()` returns true (requires "shield" in name). Confirm equipped in `HOLD_LEFT`.

**Fix:** Ensure item name contains "shield" keyword and is in correct slot.

### MOB armor not scaling

**Symptom:** MOB takes consistent damage regardless of ACLevel setting.

**Cause:** `getArmor()` returning better (lower) value than ACLevel default.

**Diagnostic:** Compare `600 - (20 * ACLevel)` to `mob->getArmor()`. System uses whichever is lower.

**Fix:** Adjust ACLevel or remove equipment if default calculation should apply.
