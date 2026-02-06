---
title: Armor and Protection System
description: Defensive equipment, armor class calculations, damage mitigation, and equipment durability
keywords: [armor class, damage mitigation, equipment durability, shield mechanics, monk unarmored]
category: important
primary_symbols:
  functions: [getArmor, itemAC, defendRound, galvanizeMe, armorPercs, armorLevel, structLevel, suggestedPrice, getIronFleshArmor, affectShouldApply]
  classes: [TBaseClothing, TArmor, TArmorWand, TSaddle, THarness]
  enums: [wearSlotT, WEAR_HEAD, WEAR_BODY, WEAR_LEG_R, WEAR_LEG_L, WEAR_FOOT_R, WEAR_FOOT_L, WEAR_HAND_R, WEAR_HAND_L, WEAR_ARM_R, WEAR_ARM_L, WEAR_WRIST_R, WEAR_WRIST_L, WEAR_FINGER_R, WEAR_FINGER_L, WEAR_BACK, WEAR_WAIST, WEAR_NECK, HOLD_RIGHT, HOLD_LEFT, ITEM_WEAR_TAKE, ITEM_WEAR_BODY, ITEM_WEAR_HEAD, ITEM_STRUNG, ITEM_BURNING, ITEM_NODROP, ITEM_MAGIC, ITEM_BLESS, ITEM_NEWBIE]
---

# Armor and Protection System

## Overview

Armor Class (AC) is the primary defensive stat determining how hard a character is to hit. Lower AC values provide better protection, with typical values ranging from 200 (excellent) to 800 (poor). Each 25 points of AC provides roughly one level of protection. The system combines racial baselines, equipment bonuses, skill modifiers, and spell effects into a total AC that converts to defense bonus during combat.

Structure points track equipment durability separately from protective value. Damaged items sell for less and may become unrepairable. The galvanize spell can permanently improve maximum structure at risk of destroying the item.

Monks have a unique relationship with armor through Iron Flesh, gaining AC from bare skin when slots are empty but losing this benefit when wearing mixed armor sets.

## Patterns

### Armor Value Storage

Always store armor AC via `APPLY_ARMOR` affects, never through val0-val3 fields. TArmor ignores `assignFourValues()` entirely.

Always use `itemAC()` to query armor contribution, which properly sums all `APPLY_ARMOR` modifiers on the item.

Never assume `getFourValues()` returns meaningful data for armor items. It returns zeros.

### Equipment Handling

Always check `affectShouldApply()` before counting equipment toward total AC. Some conditions prevent affects from applying.

Always consider paired slot penalties when evaluating monk Iron Flesh. Wearing armor in one paired slot (e.g., left leg) removes Iron Flesh from its partner (right leg). Use `isPaired()` to check wear flag compatibility.

Never forget shields provide 25% of total AC contribution despite occupying a hold slot rather than a primary armor slot.

### Defense Calculation

Always apply combat mode modifiers after base defense calculation. Defensive stance adds `my_lev / 4`, offensive subtracts the same.

Always check skill knowledge before applying skill bonuses. Use `doesKnowSkill()` before `getSkillValue()`.

Never ignore the level cap on PC defense: `(GetMaxLevel() * 1000 / 60) + my_lev` maximum regardless of equipment quality.

### Galvanize Safety

Always verify minimum 2 structure points before attempting galvanize. The spell fails on nearly-destroyed items.

Always handle critical failure outcomes where galvanize destroys the item entirely via `CF(SPELL_GALVANIZE)`.

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
| Waist | 8% | 5% |
| Head | 7% | 7% |
| Back | 7% | 10% |
| Leg (each) | 5% | 3% |
| Neck | 4% | 4% |
| Arm (each) | 4% | 5% |
| Foot (each) | 2% | 2% |
| Wrist (each) | 2% | 3% |
| Finger (each) | 1% | 1% |

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
| Defense | `my_lev * skill / 100` | 0 to +my_lev |
| Oomlat | `armor * skill / 250` | +0% to +40% armor |
| Ground Fighting | Reduces position penalty | up to 100% reduction |

Note: `my_lev` is a complex doubling-level calculation, not simply `GetMaxLevel()`. It accounts for level progression in a non-linear way.

### Position Modifiers

| Position | Defense Modifier |
|----------|-----------------|
| Flying | +my_lev/3 + 1 |
| Mounted | +my_lev/4 + 1 |
| Standing | 0 |
| Sitting | -my_lev/4 - 1 |
| Resting | -my_lev/3 - 1 |

### Spell Defense Bonuses

| Spell | Defense Bonus |
|-------|---------------|
| Aura Guardian | +40 |

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

### Object Flags Affecting Armor

| Flag | Effect |
|------|--------|
| ITEM_STRUNG | Enables customized names/descriptions |
| ITEM_BURNING | Causes structure damage over time |
| ITEM_NODROP | Prevents dropping (cursed) |
| ITEM_ANTI_* | Restricts by class (MAGE, CLERIC, WARRIOR, etc.) |
| ITEM_MAGIC | Marks magical armor |
| ITEM_BLESS | Marks blessed armor |
| ITEM_NEWBIE | Marks starting equipment |

### Wear Flags

| Flag | Slot |
|------|------|
| ITEM_WEAR_TAKE | Enables pickup |
| ITEM_WEAR_BODY | Body slot |
| ITEM_WEAR_HEAD | Head slot |
| ITEM_WEAR_LEGS | Leg slots |
| ITEM_WEAR_FEET | Foot slots |
| ITEM_WEAR_HANDS | Hand slots |
| ITEM_WEAR_ARMS | Arm slots |
| ITEM_WEAR_BACK | Back slot |
| ITEM_WEAR_WAIST | Waist slot |
| ITEM_WEAR_NECK | Neck slot |
| ITEM_WEAR_WRISTS | Wrist slots |
| ITEM_WEAR_FINGERS | Finger slots |
| ITEM_WEAR_HOLD | Shield/held slot |

### Galvanize Outcomes

| Result | Max Structure | Current Structure |
|--------|---------------|-------------------|
| Success | +1 | +1 |
| Failure | -2 | -2 |
| Critical Failure | Item destroyed | Item destroyed |

## Implementation

### Class Hierarchy

`TBaseClothing` serves as the abstract base for all wearable items. It provides armor evaluation via `armorLevel()`, `structLevel()`, and `armorPercs()`, plus pricing through `suggestedPrice()` and `armorPriceStruct()`. The `isShield()` and `isBarding()` virtuals identify specialized armor types by checking for those keywords in the item name.

`TArmor` inherits from `TBaseClothing` as the primary armor class. It implements `galvanizeMe()` for the reinforcement spell. The `itemType()` method returns `ITEM_ARMOR`. Critically, `assignFourValues()` and `getFourValues()` are no-ops returning zeros; armor AC must be stored via the `affected[]` array with `APPLY_ARMOR` location.

`TArmorWand` uses multiple inheritance from both `TArmor` and `TWand`, creating hybrid spell-casting armor. The `itemType()` method returns `ITEM_ARMOR_WAND`. Its `suggestedPrice()` combines both parent prices but subtracts duplicate weight cost.

`TSaddle` and `THarness` inherit from `TBaseClothing` for mount equipment. They provide armor benefits to mounted creatures without specialized storage fields. Their `itemType()` methods return `ITEM_SADDLE` and `ITEM_HARNESS` respectively.

### Equipment Slots

The `wearSlotT` enum in `limbs.h` defines 24 equipment positions. Humanoid slots run from `WEAR_HEAD` (1) through `HOLD_LEFT` (19), with `MAX_HUMAN_WEAR` at 20. Extended slots (20-23) accommodate creatures with extra limbs. `WEAR_NOWHERE` (0) indicates unequipped.

Paired slots (left/right variants of arms, legs, hands, feet, wrists, fingers) share combined contribution percentages split between them. The hold slots (`HOLD_RIGHT`, `HOLD_LEFT`) are the only positions for shields.

### Total AC Calculation

`TBeing::getArmor()` in `being.cc` aggregates AC from multiple sources. It starts with the racial baseline from `getMyRace()->getBaseArmor()`. Equipment contributes via `itemAC()` calls on each equipped item that passes `affectShouldApply()`.

For monks with Iron Flesh, empty slots add AC based on `getIronFleshArmor()` scaled by skill percentage. The paired item penalty subtracts Iron Flesh from the secondary slot when the primary wears armor.

Finally, character affects with `APPLY_ARMOR` location add their modifiers to the total.

### Defense Conversion

`TBeing::defendRound()` in `combat.cc` converts AC to defense bonus. PCs use formula `(armor - 500) * 2/3` capped at a level-based maximum. MOBs use `(armor - 400) * 5/6` with no explicit cap, giving them better AC-to-defense conversion.

Combat mode applies additive modifiers: defensive stance adds `my_lev / 4`, offensive subtracts it, berserk subtracts `my_lev * 8 * (100 - skill) / 100`.

Skill bonuses stack additively. Advanced Defense adds up to +10. Chivalry adds up to +159 when mounted. The general Defense skill adds up to +my_lev. Oomlat uniquely modifies the armor value before conversion rather than adding to defense directly.

Agility contributes `335 * getStatMod(STAT_AGI) - 335`, ranging from -67 to +84.

Position modifiers range from +my_lev/3 (flying) to -my_lev/3 (resting). Ground Fighting skill reduces negative position penalties proportionally.

Spell bonuses add directly to defense, such as `SPELL_AURA_GUARDIAN` which adds +40.

### Structure and Durability

Structure points use `cur_struct` and `max_struct` fields in `TObj`, not the val fields. The `addToStructPoints()` and `addToMaxStructPoints()` methods modify durability.

`galvanizeMe()` in `obj_armor.cc` handles the reinforcement spell. It calls `bSuccess()` to determine outcome. Success adds +1 to both current and maximum. Regular failure (when `critFail()` returns false) subtracts -2 from both. Critical failure (when `critFail()` returns true) destroys the item via `CF(SPELL_GALVANIZE)`. The spell requires at least 2 structure points to attempt.

Structure damage occurs from combat, decay over time, fire (`ITEM_BURNING` flag), spell failures, and admin intervention.

### Armor Evaluation

`armorPercs()` returns percentage contributions by slot, with separate values for AC calculation (first parameter false) and structure calculation (first parameter true). Shields dominate AC contribution at 25% but only 7% of structure. Body armor dominates structure at 26% but only 15% of AC.

`armorLevel()` and `structLevel()` calculate quality ratings. The base AC formula: `(baseACLevel * 25 * ac_perc) + (NEWBIE_AC * ac_perc)` where `NEWBIE_AC` is 500. Structure formula: `max(base_structure, NEWBIE_STR * sqrt(str_perc / BODY_STR))` where `NEWBIE_STR` is 30 and `BODY_STR` is 0.26.

### Pricing

`suggestedPrice()` combines weight cost (`weight * 10 * material_modifier`), armor value from `armorPriceStruct()`, and stat bonuses (`modifier * 0.25` per non-armor affect). `TArmorWand` overrides to sum both parent prices minus duplicate weight.

### Shield Mechanics

`isShield()` checks for "shield" in the item name. Shields must equip in the secondary hold slot (attempting to equip in the primary hold auto-redirects to secondary). During combat, shield parry checks `canUseArm(HAND_SECONDARY)` to verify the arm is usable, then triggers shield spec procs via `checkSpec()` with `CMD_OBJ_BEEN_HIT`. Successful parries display distinct messages to all participants and return early from the attack.

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

**Diagnostic:** Verify `doesKnowSkill(SKILL_IRON_FLESH)` returns true. Check if any paired slot has equipment. Confirm `hasQuestBit(MONK_IRON_FLESH_SKILL)` for quest completion. Verify `getSkillValue(SKILL_IRON_FLESH)` returns non-zero percentage.

**Fix:** Remove all armor for full Iron Flesh benefit, or accept partial benefit when mixing.

### Defense lower than expected

**Symptom:** Character defense bonus below what equipment suggests.

**Cause:** Level cap on PC defense, position penalties, or combat mode.

**Diagnostic:** Calculate expected defense manually: `(armor - 500) * 2/3`. Compare to `(GetMaxLevel() * 1000 / 60) + my_lev` cap. Check combat mode (ATTACK_OFFENSE and ATTACK_BERSERK apply penalties). Review agility stat as low values apply negative defense bonus.

**Fix:** Higher level characters can utilize better armor. Check combat mode and position.

### Galvanize keeps failing

**Symptom:** Galvanize spell consistently fails or destroys items.

**Cause:** Low structure points, low skill, or bad luck.

**Diagnostic:** Verify `getStructPoints() >= 2` and `getMaxStructPoints() >= 2`. Check caster skill level affecting `bSuccess()` probability.

**Fix:** Repair items before galvanizing. Improve skill before attempting on valuable items. Accept that critical failure destroys items via `CF(SPELL_GALVANIZE)` as intended risk.

### Shield not parrying

**Symptom:** Equipped shield never triggers parry messages.

**Cause:** Item not recognized as shield, wrong slot, or parry check failing.

**Diagnostic:** Verify `isShield()` returns true (requires "shield" in name). Confirm equipped in secondary hold slot. Check shield has structure points remaining.

**Fix:** Ensure item name contains "shield" keyword and is in correct slot.

### MOB armor not scaling

**Symptom:** MOB takes consistent damage regardless of ACLevel setting.

**Cause:** `getArmor()` returning better (lower) value than ACLevel default.

**Diagnostic:** Compare `600 - (20 * ACLevel)` to `mob->getArmor()`. System uses whichever is lower.

**Fix:** Adjust ACLevel or remove equipment if default calculation should apply.

### Paired item penalties incorrect

**Symptom:** Iron Flesh bonus applied or removed unexpectedly for paired slots.

**Cause:** Misunderstanding of paired slot mechanics.

**Diagnostic:** Use `isPaired()` to check wear flag compatibility. Verify penalty applies only when primary slot equipped but secondary slot empty.

**Fix:** Full armor sets (both slots equipped) avoid penalties. Mixed equipment (one equipped, one bare) triggers penalty as intended to discourage partial armor.

### Mount equipment not functioning

**Symptom:** Saddle or harness not providing expected benefits.

**Cause:** Item equipped to rider instead of mount, or wrong item type.

**Diagnostic:** Confirm `itemType()` returns `ITEM_SADDLE` or `ITEM_HARNESS`. Verify items equipped to mount, not rider. Check `isBarding()` for mount armor pieces.

**Fix:** Equip saddle/harness to mount directly. Verify chivalry skill bonus only applies when `riding()` returns true.
