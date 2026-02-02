---
title: Vital Statistics - Hunger, Thirst, and Age
category: critical
keywords: [condTypeT, gainCondition, foodNDrink, calcNutrition, age_mod_for_stat, graf, starvation, TOG_REAL_AGING, DRUNK, FULL, THIRST]
related: [character-foundation.md, race-system.md, scheduler-pulses.md]
primary_symbols:
  functions: [gainCondition, foodNDrink, calcNutrition, age_mod_for_stat, graf, setCond, getCond]
  classes: [TBeing, charFile]
  files: [code/code/misc/limits.cc, code/code/obj/obj_food.cc, code/code/misc/periodic.cc, code/code/misc/stats.cc]
---

## Overview

SneezyMUD tracks character survival and aging through two distinct systems: nutrition management (hunger, thirst, intoxication, and waste elimination) and character aging (lifespan progression with stat modifiers). These systems add realism and resource management to gameplay.

The nutrition system uses five condition types stored in the condTypeT enum. Each condition is tracked on a 0-24 scale, with special value -1 indicating immunity (immortals). Conditions drain over time based on terrain and racial metabolism, requiring players to eat and drink to survive. Starvation deals damage when conditions reach 0 for consecutive ticks, eventually causing death at -10 HP via DAMAGE_STARVATION.

The aging system tracks character age in MUD years (approximately 13.4 real days per year). Age effects are opt-in via the TOG_REAL_AGING quest bit. When enabled, age modifiers affect stats, HP regeneration, and movement costs. Physical stats (STR, AGI, DEX, CON, SPE, BRA) peak in youth and decline with old age, while mental stats (INT, WIS, FOC, PER) improve with age. The graf function interpolates values across age ranges for regeneration rates.

Racial modifiers significantly impact both systems. Races have metabolism factors affecting food and drink effectiveness, body mass modifiers, and special dietary requirements through talents like TALENT_FISHEATER and TALENT_MEATEATER. Age ranges vary by race, with base age and dice rolls determining starting age during character creation.

## Patterns

### Nutrition Drain and Replenishment

The foodNDrink function drains hunger and thirst based on terrain type, called once per half-tick from updateHalfTickStuff. TerrainInfo provides hunger, thirst, and drunk modifiers for each sector type. Desert and jungle sectors drain faster than indoor locations. Each drain is probabilistic: the lower the terrain modifier value, the higher the drain chance. When hunger decreases, POOP increases; when thirst decreases, PEE increases, simulating digestion and hydration processing.

Replenishment happens through gainCondition, which applies racial metabolism modifiers and body mass adjustments. For FULL condition, both Race::getFoodMod and weight relative to 180 pounds modify the gain. For THIRST, only weight matters. For DRUNK, race drink modifier, weight, and alcoholism skill all apply. The alcoholism skill reduces drunkenness gain by up to 5 percent, representing tolerance buildup.

Auto-eat triggers when AUTO_EAT is enabled in descriptor autobits. When FULL or THIRST reaches critical levels, the system searches equipment and inventory for appropriate items, prioritizing items closest to spoiling. Low-level players in center square can pray to the statue for nutrition instead of consuming items.

### Starvation Damage Progression

Starvation occurs when FULL or THIRST remains at 0 for consecutive half-ticks. The check compares both current and previous tick values to ensure the condition persisted. Each tick at 0 deals 1 HP damage with warning messages. Position updates occur after damage to handle unconsciousness transitions. Death occurs at -10 HP, logging the starvation event to LOG_MISC.

Safe zones exist where hunger and thirst don't drain. The inImperia and inLethargica checks prevent nutrition loss in those locations. Only PCs experience starvation damage; NPCs and immortals are immune through descriptor checks.

### Weight Fluctuation

The calcNutrition function accumulates a nutrition balance counter based on FULL level and movement expenditure. Higher FULL values increase the counter (eating more than needed), while low FULL decreases it (burning reserves). Exercise also affects balance: low current move relative to max move reduces the counter through calorie burn, but very high move (above 90 percent) also reduces it slightly.

When the nutrition counter exceeds 5000, weight increases by 1 pound if below racial maximum. When it falls below -5000, weight decreases by 1 pound if above racial minimum. The counter resets to 0 after each weight change. Weight changes send feedback messages to the player about perceived body changes.

### Age Modifier Application

The age_mod_for_stat function returns modifiers based on human-equivalent age. Non-human races normalize their age ranges to human equivalents for consistent mechanics. All age effects require TOG_REAL_AGING quest bit; without it, all functions return 0 modifiers.

Physical stats use youth bonuses and elderly penalties. Under 17 receives +10, ages 17-30 receive declining bonuses, ages 31-60 are neutral, and ages 61+ receive increasing penalties. Mental stats invert this: youth receives -10, middle age is neutral, and elderly receive bonuses up to +10. KAR decays linearly from +10 at youth to -10 at extreme old age. CHA has no age modifiers.

The graf function interpolates values across age brackets for regeneration and other gradual effects. Without TOG_REAL_AGING, graf always returns the value for age 35, treating all characters as middle-aged for mechanical consistency. The function divides the lifespan into six ranges and linearly interpolates between boundary values.

### Vampire Exception Handling

Vampires bypass all age mechanics. The isVampire check in stat calculation skips age_mod_for_stat calls entirely. Instead, vampires receive flat bonuses to STR, SPE, and CHA (+25) regardless of actual age. This preserves the vampire archetype of eternal youth and strength.

## Reference

### Condition Types (condTypeT)

DRUNK (index 0): Intoxication level from alcohol consumption. Range 0-24 or -1 for immunity. Affects perception, combat accuracy, and movement. Drains over time based on terrain drunk modifier.

FULL (index 1): Hunger satiation level. Range 0-24 or -1 for immunity. Value 0 triggers starvation damage. Values 1-2 show "hungry" warnings. Above 20 indicates full satiation. Modified by racial food metabolism and body weight.

THIRST (index 2): Hydration level. Range 0-24 or -1 for immunity. Value 0 triggers dehydration damage. Values 1-2 show "thirsty" warnings. Modified by body weight only (no racial factor for water).

PEE (index 3): Bladder pressure. Range 0-24. Increases when thirst decreases. No immunity value. Relieved through game mechanics.

POOP (index 4): Bowel pressure. Range 0-24. Increases when hunger decreases. No immunity value. Relieved through game mechanics.

### Racial Food Talents

TALENT_FISHEATER: Receives 2x nutrition from fish items, only 5 percent from other foods. Forces dietary specialization.

TALENT_MEATEATER: Receives 2x nutrition from butchered meat, only 5 percent from other foods. Forces carnivorous diet.

TALENT_INSECT_EATER: Can only eat insect items. All other food types rejected.

TALENT_GARBAGEEATER: Can consume organic trash items. Experiences accelerated hunger drain (loses 1-5 FULL randomly when below 19). Takes movement damage when hungry and at 0 move, then HP damage if movement exhausted.

### Age Storage Components

charFile::birth: Real-world Unix timestamp when character was created. Never changes after creation.

charFile::base_age: Starting age rolled during character creation using Race::generateAge. Stored as unsigned short.

charFile::age_mod: Manual age adjustments from magical effects or admin commands. Stored as signed short.

Current age calculation: MUD years elapsed since birth timestamp, plus base_age, plus age_mod.

### Time Conversion Constants

1 MUD hour = 144 real seconds (2.4 real minutes). Defined as Pulse::SECS_PER_MUDHOUR.

1 MUD day = 24 MUD hours = 3456 real seconds (57.6 real minutes). Defined as Pulse::SECS_PER_MUD_DAY.

1 MUD month = 28 MUD days = 96768 real seconds (26.9 real hours). Defined as Pulse::SECS_PER_MUD_MONTH.

1 MUD year = 12 MUD months = 1161216 real seconds (13.4 real days). Defined as Pulse::SECS_PER_MUD_YEAR.

### Age-Based Stat Modifier Tables

Physical stats (STR, BRA, AGI, DEX, CON, SPE) follow this progression:
- Age under 17: +10 bonus
- Age 17-20: +6 to +9 bonus (linear decline)
- Age 21-30: +1 to +5 bonus (linear decline)
- Age 31-60: 0 modifier (prime years)
- Age 61-70: -1 to -4 penalty (linear decline)
- Age 71-80: -5 to -9 penalty (linear decline)
- Age 81+: -10 penalty

Mental stats (INT, WIS, FOC, PER) follow inverted progression:
- Age under 17: -10 penalty
- Age 17-30: -9 to -1 penalty (linear improvement)
- Age 31-60: 0 modifier
- Age 61-80: +1 to +9 bonus (linear improvement)
- Age 81+: +10 bonus

KAR special progression:
- Age under 17: +10 bonus
- Age 17-60: Linear decay from +10 to 0
- Age 61+: Linear decay from 0 to -10

CHA: No age modifiers in any age range.

### HP Regeneration Graf Parameters

graf function call for hitGain: graf(age, 2, 4, 5, 9, 4, 3, 2)

Age bracket values:
- Under 15: value 2
- 15-29: value 4
- 30-44: value 5
- 45-59: value 9 (peak regeneration)
- 60-79: value 4
- 80+: value 3

Linear interpolation between bracket boundaries. Without TOG_REAL_AGING, always returns value for age 35 (interpolated in 30-44 bracket).

### Movement Cost Modifiers

Age 35-49: Additional movement cost = (current_age - base_age - 30) / 5. Increases by 1 move point per 5 years over 30.

Age 50+: Additional cost = previous calculation + (current_age - base_age - 50) / 10. Plus probabilistic extra point based on (years_over_50 mod 10) compared to random 1-10 roll.

Maximum range penalty: Age over 80 (human equivalent) reduces max_range by 3 for ranged combat.

## Implementation

### Condition Storage and Access

TBeing::specials.conditions array holds all five condition values as signed shorts. The setCond function enforces immunity for characters above MAX_MORT level by always setting -1 regardless of requested value. The getCond function returns the raw value without validation.

Persistence happens through charFile::conditions array, written during character save operations. The charFile structure maintains binary compatibility, so the conditions array position and size cannot change without breaking existing save files.

### Terrain-Based Drain Mechanics

The foodNDrink function receives sector type and modifier parameters. The modifier parameter scales drain rates for special situations (normal value is 4). Each terrain has hunger, thirst, and drunk factors in TerrainInfo lookup table.

Drain calculation: effective_modifier = base_value - (modifier * terrain_factor), clamped to minimum 0. Base values are 11 for food, 9 for thirst and drunk. Higher terrain factors mean slower drain (less harsh environment).

Probability checks use two-stage random: first check (0-9 range, threshold 5 or 6) determines if category drains this tick, second check (0 to effective_modifier) determines actual drain. If effective_modifier is 0, drain always occurs when first check passes.

When FULL drains, POOP increments by 1. When THIRST drains, PEE increments by 1. No overflow checks; values clamp at 24 in gainCondition.

### Body Mass Calculations

Weight modifiers use 180 pounds as baseline. The formula: adjusted_value = base_value * 180 / character_weight, with minimum result 1. Heavier characters receive proportionally less benefit per unit consumed. Lighter characters receive more benefit.

For FULL condition, race food modifier applies first, then weight modifier: value = (value * race_food_mod) * 180 / weight. For DRUNK, race drink modifier applies first, then weight, then alcoholism: value = (value * race_drink_mod * 180 / weight) * (105 - alcoholism_skill) / 100.

The calcNutrition function uses similar weight-relative logic for determining calorie balance. Body weight affects both consumption efficiency and drain rate, creating feedback loop: gaining weight makes food less effective, losing weight makes it more effective.

### Racial Dietary Restrictions

Food talent checks happen in TFood::eat method. The code compares item object type or flags against character's racial talents. For TALENT_FISHEATER, non-fish food provides only 5 percent nutrition: nutrition_value = original_value / 20. For TALENT_MEATEATER, non-meat provides the same penalty.

TALENT_GARBAGEEATER has special drain in updateHalfTickStuff. Each half-tick, if FULL is below 19, random loss of 1-5 points occurs. If FULL reaches 0, movement damage applies (4-8 points). If movement also at 0, HP damage applies (1 point). This represents extreme metabolic demands of garbage-eating biology.

### Auto-Eat Search Algorithm

When AUTO_EAT triggers, the code iterates through equipment slots first, then inventory. For each container found, recursively searches contents. Tracks the item closest to spoiling (lowest decay timer) as preferred consumption target.

Container search uses last_cont pointer to remember location. If item is in container, extraction happens before consumption. If item is equipped, unequip happens first. The fname function extracts first keyword from item name for command parsing.

Low-level special case: characters level 3 or below in center square (Room::CS) execute "pray statue" instead of searching inventory. This provides newbie safety net for hunger.

### MUD Time Calculation

The mudTimePassed function converts real-world seconds to MUD time components. Input is two Unix timestamps (current and reference). Difference in seconds becomes basis for all conversions.

Hours calculation: (seconds / Pulse::SECS_PER_MUDHOUR) mod 24. Day: (seconds / Pulse::SECS_PER_MUD_DAY) mod 28. Month: (seconds / Pulse::SECS_PER_MUD_MONTH) mod 12. Year: (seconds / Pulse::SECS_PER_MUD_YEAR) with no modulo (accumulates indefinitely).

The time_info_data structure holds resulting components. Age calculation uses only the year component, ignoring sub-year precision. Character age = base_age + years_since_birth + age_mod.

### Age Toggle Integration

Every age-related function checks TOG_REAL_AGING quest bit first. Functions include age_mod_for_stat, graf, movement cost calculations, and range penalties. Without the bit, functions return neutral values (0 modifiers, age-35 equivalent for graf).

The toggle sets during character creation as optional quest. The create_character.cc file presents it as choice with explanation of old age effects. Default is disabled, preserving compatibility with players who prefer stat stability.

Vampire check happens before age check in stat calculation. If isVampire returns true, age modifier code never executes. The vampire flat bonuses replace age modifiers entirely.

### Graf Interpolation Logic

The graf function receives seven boundary values (p0 through p6) defining six age ranges. Age parameter is human-equivalent years. Function determines which bracket the age falls into, then linearly interpolates between bracket boundaries.

Interpolation formula: low_value + ((age - range_start) * (high_value - low_value)) / (range_end - range_start). This creates smooth transitions rather than step functions.

For non-aging characters, the function calculates age-35 value directly: p2 + ((35 - 30) * (p3 - p2)) / 15. This picks a point one-third into the third bracket (30-44 range), representing mature adult without elderly penalties or youth bonuses.

### Regeneration Rate Modifiers

hitGain, manaGain, and moveGain functions all check hunger and thirst conditions. If either FULL or THIRST is 0, regeneration gain right-shifts by 2 (divides by 4). This applies after all other modifiers (constitution, position, age).

HP regeneration has additional restriction: if hungry or thirsty, characters only gain 1 HP if in STUNNED position, otherwise 0. The quarter-rate penalty applies to mana and move, but HP effectively stops.

Age affects HP regeneration through graf function. The regeneration rate multiplier comes from graf output, scaled by position and constitution. Higher graf values mean faster regeneration.

### Movement Cost Scaling

The rawMove function calculates base movement cost for terrain, then applies age penalties if TOG_REAL_AGING is set. Current age minus base age gives MUD years lived. Age 35+ adds (years - 30) / 5, using integer division (rounds down).

Age 50+ has second penalty: (years - 50) / 10. Plus probabilistic component: (years - 50) mod 10 compared to random 1-10. If mod result is greater than or equal to random value, add 1 additional move point. This creates escalating cost with some variance.

Maximum move pool calculation uses opposite logic: aging characters get higher pool. The formula adds (current_age - base_age) to base move calculation. This compensates for higher per-move costs by increasing total pool.

## Troubleshooting

### Condition -1 Misinterpretation

When getCond returns -1, do not treat it as critical hunger/thirst. Value -1 means immunity, typically for immortals or special states. Always check for -1 before comparing to 0. Code like "if (getCond(FULL) == 0)" is safe because -1 does not equal 0, but "if (getCond(FULL) <= 0)" catches immunity incorrectly.

The setCond function for high-level characters always forces -1, overriding any requested value. This means admin characters cannot test hunger systems on themselves without temporarily de-leveling.

### Racial Food Incompatibility

Characters with TALENT_FISHEATER or TALENT_MEATEATER receive only 5 percent nutrition from non-specialty foods. This means eating bread or vegetables provides almost no benefit. Players without access to fish or butchered meat will starve despite eating regularly.

The 5 percent calculation divides original nutrition by 20. For low-value foods, this rounds to 1 or 0, making them nearly worthless. Stock appropriate food types in areas where these races spawn or start.

### Weight Modifier Confusion

Heavier characters need more food to satisfy hunger, but calcNutrition uses same threshold (5000) regardless of weight. This creates asymmetry: gaining weight makes eating less efficient, but weight gain threshold stays constant. Very heavy characters may struggle to gain weight further, while very light characters gain easily.

The 180-pound baseline means characters of that exact weight get unmodified nutrition. Lighter than 180 gets bonus, heavier gets penalty. This is racial-neutral; all races use same formula despite having different typical weights.

### Auto-Eat Limitations

AUTO_EAT does not work during tasks. If character is in combat, crafting, or other task state, the auto-eat trigger may fire but command execution fails. The task system blocks most commands during active tasks.

Auto-eat searches equipped items and inventory but not room objects. Food on the ground or in containers the character doesn't own will not be considered. Characters can starve surrounded by food if they don't pick it up first.

The center square statue prayer only works for characters level 3 or below. Level 4+ characters in center square will search inventory normally, not trigger statue interaction.

### Age Effects Opt-In Confusion

Most characters do not have TOG_REAL_AGING set. When debugging stat discrepancies, check quest bits before assuming age modifiers apply. The toggle is not visible in standard "score" output, requiring "quest" command or direct bit inspection.

Characters created before the toggle existed do not have it set by default. This means very old characters (many MUD years) still use age-35 neutral values unless they manually enabled aging later.

### Graf Age-35 Default

When TOG_REAL_AGING is disabled, graf always returns the age-35 value from the 30-44 bracket. This is not necessarily the middle value (p3); it's interpolated as one-third into that specific bracket. For HP regeneration, this gives value 6 out of possible range 2-9, which is above midpoint but not maximum.

Do not assume "no aging" means "average value." The age-35 choice was deliberate balance decision, giving modest benefit without extreme youth bonuses or elderly penalties.

### Vampire Age Calculation

Vampires skip age_mod_for_stat but still have age values. Their age()->year continues to increase, and they display age in score/stat commands. The age exists for roleplay purposes but has no mechanical effect.

The flat +25 to STR/SPE/CHA applies regardless of vampire's actual age. A newly-turned vampire and an ancient vampire have identical stat bonuses from vampire status. Age-based differentiation requires other systems (vampire power progression, not age modifiers).

### Starvation in Safe Zones

Imperia and Lethargica prevent nutrition drain but do not freeze conditions. If character enters Imperia already hungry, they remain hungry indefinitely without further drain. Auto-eat may still trigger if conditions are already critical.

Leaving safe zone resumes normal drain immediately. Characters who idle in safe zones for long periods start draining again as soon as they move to normal zones, potentially causing rapid starvation if they forgot to eat.

### GARBAGEEATER Damage Loop

Characters with TALENT_GARBAGEEATER take escalating damage when hungry. The damage progresses from FULL loss, to movement loss, to HP loss. If character reaches 0 move and 0 FULL simultaneously, they take both movement damage (4-8 points, but already 0) and HP damage (1 point) each half-tick.

This creates potential death spiral: hunger prevents regeneration, HP damage continues, character cannot heal without eating, but may be unable to find garbage items. Stock garbage-eating races with appropriate food or warn players of high difficulty.

### Movement Cost Age Breakpoints

The age penalties create sudden cost jumps at specific ages. Age 35 is first breakpoint (adds 1 move cost). Age 40 adds another, age 45 another, etc. Age 50 triggers second penalty tier with both deterministic and random components.

The probabilistic component at age 50+ means movement cost can vary by 1 point between identical moves. This is intentional age-related inconsistency representing physical decline variance.

### Racial Age Generation Variability

Race age generation uses dice rolls, creating starting age ranges. Two characters of same race can start with different ages (e.g., 17 vs 23 for "age 15+2d4"). If both have TOG_REAL_AGING enabled, they will have different stat modifiers from creation, even at level 1.

This starting age difference persists throughout character lifetime. The older-starting character will reach elderly penalties earlier in real-world time. Competitive players may min-max by rerolling characters to get youngest possible starting age.
