---
title: Crafting and Extraction Systems
description: Material extraction (skinning, butchering, dissection, foraging), item creation (brewing, scribing), and equipment repair systems.
category: informational
keywords: [material extraction, item creation, equipment repair, resource gathering]
primary_symbols:
  functions: [determineSkinningItem, findSomeComponent, determineDissectionItem, objectRepair, forage, forage_insect, seed_to_plant, task_skinning, task_brew, task_scribe]
  classes: [TComponent, TPlant, TFood, TCorpse]
  enums: [CORPSE_NO_SKIN, CORPSE_HALF_SKIN, CORPSE_PC_SKINNING, CORPSE_NO_BUTCHER, CORPSE_HALF_BUTCHERED, CORPSE_PC_BUTCHERING, CORPSE_NO_DISSECT, CORPSE_NO_REGEN, SKILL_SKIN, SKILL_BUTCHER, SKILL_FORAGE, SKILL_BREW, SKILL_DISSECT, SKILL_SCRIBE, SKILL_BLACKSMITHING, SKILL_SHARPEN, SKILL_REPAIR_SHAMAN, SKILL_REPAIR_MONK, SKILL_REPAIR_MAGE, SKILL_REPAIR_THIEF, SKILL_REPAIR_CLERIC, SKILL_REPAIR_DEIKHAN, SKILL_BLACKSMITHING_ADVANCED, SKILL_READ_MAGIC, TOOL_SKIN_KNIFE, TOOL_BUTCHER_KNIFE, TOOL_WHETSTONE, TOOL_SEED, TALENT_MEATEATER, TALENT_INSECT_EATER, LIQ_MAGICAL_ELIXIR, LIQ_LEMONADE, CMD_TASK_FIGHTING, ACT_STRINGS_CHANGED, TOG_STARTED_MONK_BLUE, ROOM_FLOODED, ROOM_ON_FIRE, Obj::GENERIC_STEAK]
---

## Overview

How do players turn monster corpses into usable materials? How does a Shaman create a potion from components? How does a worn-down sword get restored to fighting condition?

The crafting and extraction systems allow players to harvest materials from corpses, create consumable magic items, repair damaged equipment, and gather resources from the environment. These interconnected systems span multiple character classes: Rangers for skinning, butchering, and foraging; Shamans for brewing and dissection; Mages for scribing; and Warriors for blacksmithing.

Each extraction system uses the task framework, meaning operations take time and can be interrupted by combat or movement. Players invest real gameplay time into crafting, creating meaningful resource gathering loops.

Corpse-based extraction (skinning, butchering, dissection) uses flag systems to track corpse state. A corpse can be partially processed, completely processed, or blocked from certain extractions entirely. This prevents double-harvesting and enables meaningful choices about how to use a corpse.

Item creation (brewing, scribing) consumes components with limited charges and produces magical items containing spells. The output quality depends on skill level, with failure producing useless items rather than partial successes.

Repair systems branch by material type, routing metal items to blacksmithing, organic materials to monk repair, and magical items to mage repair. Each material type requires different tools and room features.

---

## Patterns

### Corpse Flag Discipline

Always check all relevant corpse flags before starting extraction tasks. A corpse may already be in use, already processed, or intrinsically blocked from certain operations.

For skinning, check `CORPSE_NO_SKIN`, `CORPSE_PC_SKINNING`, and `CORPSE_NO_REGEN`. For butchering, check `CORPSE_NO_BUTCHER`, `CORPSE_PC_BUTCHERING`, and `CORPSE_NO_REGEN`. For dissection, check `CORPSE_NO_DISSECT` and `CORPSE_NO_REGEN`.

Starting a task without these checks can cause double-extraction bugs or confusing player messages.

### Set Concurrency Flags During Tasks

Set the appropriate `CORPSE_PC_*` flag when starting an extraction task and clear it when the task completes or is interrupted. This prevents multiple players from simultaneously extracting from the same corpse.

The `CORPSE_PC_SKINNING` and `CORPSE_PC_BUTCHERING` flags serve as mutex locks for corpse operations. Clear these flags on all exit paths—task interruption (combat, movement, linkdead) without clearing the lock permanently blocks corpse processing.

### Handle Tool Breakage Immediately

Tools lose uses during task pulses. When `getToolUses()` reaches zero, delete the tool immediately, stop the task, and return before any further tool access.

Never access a tool after deletion. Store the deletion result and handle the task termination in the same code block.

### Track Component Charges Carefully

Components have limited charges consumed by brewing and scribing. After consuming charges, check if the component is empty and delete it if so. Set the component pointer to NULL after deletion to prevent dangling pointer access.

When batch-scribing multiple scrolls, calculate the maximum batch size as the minimum charges across all required components. Never trust player-provided quantity—always limit to component availability.

### Handle Task Interruption from Combat

Every task callback must handle `CMD_TASK_FIGHTING` by stopping the task and notifying the player. Combat interrupts all crafting operations. Missing this handler allows tasks to continue during combat, creating animation inconsistencies.

### Validate Room and Position Continuously

Each task pulse should verify the player is still in the same room, is not linkdead, and is in an appropriate position (at least resting). Movement or disconnection should abort the task cleanly.

### Check reconcileDamage Return for Death

Tasks that can damage the player (critical skinning/butchering failures) must check if `reconcileDamage()` returns -1. On death, stop the task, save the character, and return `DELETE_THIS`. Note: `reconcileDamage()` returns -1 on death, not a DELETE flag.

### Use Half-Processed Flags Correctly

When extraction partially completes (task interrupted, skill failure), set `CORPSE_HALF_SKIN` or `CORPSE_HALF_BUTCHERED` rather than `CORPSE_NO_SKIN` or `CORPSE_NO_BUTCHER`. Half-processed corpses can be processed again with reduced yields.

### Consume Resources on Failure

Always provide failure paths that consume resources. Failure without resource consumption enables infinite attempts until success, bypassing skill progression. Both success and failure should decrement component charges.

---

## Reference

### Symbol Quick Reference

| Symbol | Type | Purpose |
|--------|------|---------|
| `determineSkinningItem()` | function | Map corpse race/vnum to skin item vnum |
| `findSomeComponent()` | function | Locate brewing/scribing components in inventory |
| `determineDissectionItem()` | function | Map corpse race/vnum to dissection item |
| `objectRepair()` | function | Shop repair price and execution |
| `forage()` | function | Wild food gathering by terrain |
| `forage_insect()` | function | Insect gathering for insect-eating races |
| `seed_to_plant()` | function | Map seed vnum to plant type index |
| `task_skinning()` | function | Multi-pulse skinning task handler |
| `task_brew()` | function | Multi-pulse brewing task handler |
| `task_scribe()` | function | Multi-pulse scribing task handler |
| `TComponent` | class | Component items with charges |
| `TPlant` | class | Growing plants from seeds |
| `TFood` | class | Edible items including butchered meat |
| `TCorpse` | class | Corpse objects with extraction flags |

### Skills by Class

| Skill | Class | Discipline | Purpose |
|-------|-------|------------|---------|
| `SKILL_SKIN` | Ranger | Advanced Adventuring | Extract hides from corpses |
| `SKILL_BUTCHER` | Ranger | Basic Adventuring | Extract meat from corpses |
| `SKILL_FORAGE` | Ranger | Advanced Adventuring | Gather wild food |
| `SKILL_BREW` | Shaman | Shaman Alchemy | Create potions |
| `SKILL_DISSECT` | Shaman | Shaman Control | Extract components from corpses |
| `SKILL_SCRIBE` | Mage | Mage Alchemy | Create scrolls |
| `SKILL_BLACKSMITHING` | Warrior | (base) | Repair metal items |
| `SKILL_SHARPEN` | Warrior | Basic Combat | Restore weapon sharpness |

### Corpse Flags

| Flag | Meaning | Effect |
|------|---------|--------|
| `CORPSE_NO_SKIN` | Cannot be skinned | Skinning blocked |
| `CORPSE_HALF_SKIN` | Partially skinned | Halves maximum yield |
| `CORPSE_PC_SKINNING` | Currently being skinned | Prevents concurrent skinning |
| `CORPSE_NO_BUTCHER` | Cannot be butchered | Butchering blocked |
| `CORPSE_HALF_BUTCHERED` | Partially butchered | Halves maximum yield |
| `CORPSE_PC_BUTCHERING` | Currently being butchered | Prevents concurrent butchering |
| `CORPSE_NO_DISSECT` | Already dissected | Dissection blocked |
| `CORPSE_NO_REGEN` | Body part, not full corpse | Blocks skinning, butchering, dissection |

### Repair Skills by Material

| Material Category | Skill | Class | Room/Tool Requirements |
|-------------------|-------|-------|------------------------|
| Metal (iron, steel, mithril) | `SKILL_BLACKSMITHING` | Warrior | Forge, anvil, hammer, tongs |
| Dead (bone, flesh, ivory) | `SKILL_REPAIR_SHAMAN` | Shaman | Operating table, scalpel, forceps |
| Organic (coral, scales) | `SKILL_REPAIR_MONK` | Monk | Water sector, ladle, plant oil |
| Wood (wood, ebony) | `SKILL_REPAIR_MONK` | Monk | Water sector, ladle, soil |
| Rock/Stone | `SKILL_REPAIR_MAGE` or `SKILL_REPAIR_MONK` | Mage/Monk | Pentagram, chisel, silica |
| Crystal/Gem | `SKILL_REPAIR_THIEF` or `SKILL_BLACKSMITHING_ADVANCED` | Thief/Warrior | Workbench, loupe, pliers |
| Magical (plasma, runed) | `SKILL_REPAIR_MAGE` | Mage | Pentagram, runes, energy |
| Leather/Hide | `SKILL_REPAIR_MONK` | Monk | Punch, cording |
| Spiritual (ghostly) | `SKILL_REPAIR_CLERIC` or `SKILL_REPAIR_DEIKHAN` | Cleric/Deikhan | Altar, brush, astral resin |

### Required Tools

| Operation | Tool Type | Alternative |
|-----------|-----------|-------------|
| Skinning | `TOOL_SKIN_KNIFE` | Slash/pierce weapon (volume <= 6000) |
| Butchering | `TOOL_BUTCHER_KNIFE` | Slash/pierce weapon, or bare hands with `TALENT_MEATEATER` |
| Sharpening | `TOOL_WHETSTONE` | None |
| Planting | `TOOL_SEED` | None |

### Forage Items by Terrain

| Sector Type | Item Range |
|-------------|------------|
| Standard outdoor | 276-281 |
| Arctic | 37130-37133 |
| Cave/Indoor | 37134-37136 |
| Desert | 37137-37140 |

---

## Implementation

### Skinning Mechanics

Skinning extracts hides from corpses based on corpse weight and player skill.

**Yield Calculation:** Maximum units derive from corpse weight at roughly 10% weight divided by 2, minus 1 (minimum 1). Units gained per successful skill check scale with skill level divided by 25 (minimum 1). Half-skinned corpses yield half the maximum.

**Task Duration:** Duration in pulses starts at 5 plus a skill-based factor. Higher skill reduces time, but there's a minimum of 4 pulses. The formula incorporates skill level, skill value relative to 70, corpse size, and units per success.

**Item Determination:** `determineSkinningItem()` first checks for mob-specific overrides (special quest mobs), then falls back to race-based mapping. Each race has a default skin item: squirrels yield pelts, deer yield hides, dragons yield scales.

**Output Properties:** Skinned item weight and volume scale with corpse properties and extraction percentage. Item value scales with corpse level and skill learning, roughly level times 0.9 plus learning/100, multiplied by units over 3.

**Failure Consequences:** Single skill failure dulls the weapon by 1 sharpness but still yields units. Double failure dulls by 2 with no yield. Critical failure dulls by 3, damages the player (5 plus half weapon sharpness), and creates a blood pool.

### Butchering Mechanics

Butchering produces edible meat from corpses.

**Yield Calculation:** Maximum food units derive from corpse weight at 10% minus 1 (minimum 0). The actual food fill value clamps between 0 and 100 based on extraction efficiency.

**Output:** Creates a `Obj::GENERIC_STEAK` object with a random cut type (rib-eye, chuck-eye, skirt, flank, t-bone, porterhouse, tenderloin, sirloin, tri-tip, chuck, ribs, short loin, or filet mignon). Steak weight is food units divided by 10, volume is food units times 10.

**Bare Hands:** Races with `TALENT_MEATEATER` can butcher without tools. Critical failure with bare hands inflicts 25 damage to the player.

### Brewing Mechanics

Brewing creates potions containing spells.

**Required Components:** Three components needed - a flask containing magical elixir (`LIQ_MAGICAL_ELIXIR`), a spell component for the target spell, and a class-specific brew component.

**Resource Cost:** Lifeforce drains each pulse based on skill value. The formula combines skill value times 3 and skill value times 2, divided by 15.

**Task Duration:** Brewing takes the batch size times 2 pulses, with each pulse being 7 times `Pulse::MOBACT`.

**Success/Failure:** Success creates a potion containing the spell at full learnedness. Failure produces lemonade (`LIQ_LEMONADE`, 0-5 random units of useless liquid). Components consumed regardless of success.

### Scribing Mechanics

Scribing creates scrolls containing spells.

**Required Components:** Three components needed - generic parchment, a spell component, and a class-specific scribe component.

**Resource Cost:** Mana drains each pulse. The formula uses skill value times 6 plus skill value times 4, divided by 15 (higher than brewing).

**Batch Scribing:** Players can scribe multiple scrolls with `scribe <number> <spell>`. Batch size is limited by the minimum charges across all three components.

**Success Check:** Requires passing either a `SKILL_SCRIBE` check or a `SKILL_READ_MAGIC` check. A single skill check is performed once for the batch, then the while loop creates additional scrolls by copying the learnedness value from the first scroll. All scrolls in a batch share the same success/failure result. Success gives scrolls proper spell learnedness; failure produces unreadable scrolls (learnedness 0).

### Component Location

Both brewing and scribing use `findSomeComponent()` to search player inventory for required components. The function takes output pointers for generic, spell, and class components, plus the target spell and operation type (1 for brewing, 2 for scribing).

Components track charges via `getComponentCharges()` and `addToComponentCharges()`. Component type flags distinguish spell, brew, and scribe classifications. Component type 1 (brewing) searches for generic brew component, then spell-specific brew component, then spell component. Type 2 (scribing) follows the same pattern for scribe components.

### Repair System

**Shop vs Player Repair:** Shop repair uses NPC repairmen with real-time delays and talen costs. Player repair uses task-based pulses with movement/mana/lifeforce costs and raw material consumption.

**Quality Cap:** Maximum repair is 95% of max structure minus depreciation. Shop quality further limits this based on shop quality setting (0-1 multiplier).

**Material Consumption:** Units needed scale with item weight divided by max structure, times 10, times the repair ratio (0.10). Monogrammed items use only 25% of normal materials (materials needed divided by 4).

**Material Validation:** Always verify commodity charges are sufficient before consuming. Never allow negative charge values—this can create infinite materials when wrapping occurs.

### Sharpening Mechanics

Sharpening restores weapon sharpness using a whetstone.

**Requirements:** `SKILL_SHARPEN`, weapon in primary hand, whetstone in possession.

**Per-Pulse:** Movement cost is 2d3. Successful skill check adds 1 sharpness (2 for arrows). Whetstone loses 1 use. Task stops when current sharpness reaches maximum.

### Dissection Mechanics

Dissection extracts shaman components from corpses, distinct from skinning which extracts hides.

**Item Sources (checked in order):**
1. Race-specific hardcoded items (phoenix yields flaming sword component or phoenix feather)
2. Vnum-specific hardcoded items (special quest mobs)
3. `dissect_array` data file at `objdata/dissect`

**Data File Format:** Each entry has mob vnum, item vnum, amount (percentage chance), message to self, and message to others.

**Success Check:** Must pass `SKILL_DISSECT` check (or have `TOG_STARTED_MONK_BLUE` quest bit). Then a percentage roll against the amount field determines if the item appears. Failure calls the `CF` macro marking critical fail for skill tracking.

After dissection, `CORPSE_NO_DISSECT` is set to prevent re-extraction. Unlike skinning/butchering which allow partial extraction, dissection is one-time.

### Forage Mechanics

Foraging gathers wild food based on terrain.

**Valid Sectors:** Forest, beach, hills, mountains, nature, road, swamp, and arctic. Invalid in city, flying, vertical, underwater, air, ocean, river, flooded, or burning rooms. Room flags `ROOM_FLOODED` and `ROOM_ON_FIRE` block foraging.

**Cost and Cooldown:** Movement cost is 5-15 random. Success cooldown is 4 mud hours. Failure cooldown is 2 mud hours.

**Multiple Items:** Initial 1000/1000 chance for first item, then chance divides by 3 for each additional item, creating diminishing returns (most forages yield 1-2 items, rare cases yield 3-4).

**Insect Foraging:** Races with `TALENT_INSECT_EATER` use specialized insect foraging. Food scales with terrain insect density times a base multiplier (`FORAGE_INSECT_FOOD` constant, value 4). Having `SKILL_FORAGE` provides up to 50% bonus on successful check.

### Planting Mechanics

Planting grows crops from seed items.

**Requirements:** Seed tool in inventory, valid outdoor sector (not fall/water/indoor/underwater), room not at plant capacity (8 maximum).

**Task Duration:** 3 pulses (dig, plant, cover phases).

**Poisonous Seeds:** Certain seed vnums (death camas, jimson weed, hemlock, monkshood, sweet pea, acacia) poison the planter if not immune.

**Growth Stages:** Plants age over time: 0-9 (mound of dirt), 10-19 (tiny sprout), 20-29 (small plant), 30+ (full plant), then eventually withering.

**Seed Mapping:** `seed_to_plant()` maps seed vnums to plant type indices (tomato, roses of various colors, apple tree, orange tree, money tree, etc.).

---

## Troubleshooting

### Double-Extraction Bug

**Symptom:** Same corpse yields materials twice, or "already being skinned" message appears incorrectly.

**Likely cause:** Missing corpse flag checks before starting task, or failing to set/clear concurrency flags.

**Diagnostic approach:** Verify `CORPSE_PC_SKINNING` or `CORPSE_PC_BUTCHERING` is set when task starts and cleared when task ends (including interruption paths).

**Fix:** Add comprehensive flag checks at task start and ensure all exit paths clear concurrency flags.

### Tool Crash After Breakage

**Symptom:** Crash or undefined behavior after tool uses reach zero.

**Likely cause:** Continuing to access tool pointer after deletion.

**Diagnostic approach:** Check task pulse handler for tool use decrement and subsequent access patterns.

**Fix:** Immediately delete tool when uses reach zero, stop task, and return before any further tool operations.

### Component Duplication

**Symptom:** Brewing/scribing consumes fewer charges than expected, or component not deleted when empty.

**Likely cause:** Not decrementing charges for all consumed components, or not deleting empty components.

**Diagnostic approach:** Trace component charge manipulation through entire task lifecycle.

**Fix:** Decrement all component charges, check each for zero charges, delete and null empty components.

### Repair Fails on Valid Material

**Symptom:** Player cannot repair an item that should be repairable with their skill.

**Likely cause:** Material category mapping doesn't match expected skill, or missing room/tool requirement.

**Diagnostic approach:** Check item's material type, find the corresponding repair skill in the material mapping, verify player has that skill, check room for required features (forge, altar, etc.).

**Fix:** Ensure material-to-skill mapping is correct and all prerequisites are met.

### Forage Returns Nothing

**Symptom:** Foraging fails consistently even with good skill.

**Likely cause:** Invalid sector type, room flags (flooded/burning), or cooldown still active.

**Diagnostic approach:** Check room sector type against valid forage sectors, check room flags, check for forage affect (cooldown).

**Fix:** Move to valid outdoor terrain, wait for cooldown, ensure room is not flooded or burning.

### Plant Won't Grow

**Symptom:** Planted seed remains a dirt mound indefinitely.

**Likely cause:** Invalid sector preventing growth ticks, or plant age not incrementing.

**Diagnostic approach:** Check room sector, verify plant age field is being updated by periodic processing.

**Fix:** Plant in valid outdoor sector, verify plant update loop is running.

### Task Continues During Combat

**Symptom:** Crafting task doesn't stop when combat begins.

**Likely cause:** `CMD_TASK_FIGHTING` case missing or not calling `stopTask()`.

**Diagnostic approach:** Review task switch statement for `CMD_TASK_FIGHTING` case.

**Fix:** Add `CMD_TASK_FIGHTING` case with message to player and `ch->stopTask()` call.

### Death During Task Causes Crash

**Symptom:** Crash during skinning/butchering critical failure that damages player.

**Likely cause:** `reconcileDamage()` return value not checked, code continues accessing deleted player.

**Diagnostic approach:** Enable ASan, check for use-after-free in critical failure path.

**Fix:** Check if `reconcileDamage()` returns -1 (death). If so, call `stopTask()`, `doSave()`, and return `DELETE_THIS` immediately.

### Batch Creates More Than Components Allow

**Symptom:** Batch scribing creates more scrolls than component charges should permit.

**Likely cause:** Batch size calculated from requested amount rather than minimum component charges.

**Diagnostic approach:** Log component charge values and calculated batch size.

**Fix:** Calculate batch size as minimum of all component charge counts. Never trust player-provided quantity.

### Repair Creates Materials

**Symptom:** Repair operation results in negative material consumption (creating materials).

**Likely cause:** Material calculation not validated against available commodity charges.

**Diagnostic approach:** Log calculated material needs and commodity charge count.

**Fix:** Verify `commodity->getComponentCharges() >= mats_needed` before consuming. Never allow negative charge values.
