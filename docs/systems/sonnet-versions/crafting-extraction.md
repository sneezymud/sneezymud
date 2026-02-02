---
title: Crafting and Extraction Systems
category: important
keywords: [skinning, butchering, brewing, scribing, foraging, planting, dissecting, repair, blacksmithing, components]
related: [material-system.md, component-system.md, task-system.md, economy-system.md]
primary_symbols:
  functions: [determineSkinningItem, findSomeComponent, objectRepair, dissectMe, forage, task_skinning, task_brew, task_scribe]
  classes: [TComponent, TPlant, TCorpse]
  files: [code/task/task_skin.cc, code/task/task_brew.cc, code/task/task_scribe.cc, code/misc/repair.cc, code/cmd/cmd_dissect.cc]
---

## Overview

When a ranger skins a dragon corpse, where does the dragon scale hide come from? When a shaman brews a healing potion, what prevents them from creating infinite potions with a single component? When a warrior repairs their damaged sword at the forge, how does the system ensure they consume the correct materials?

The crafting and extraction systems enable resource harvesting, item creation, equipment maintenance, and resource gathering. These interconnected systems support the game economy by creating material loops: corpses yield materials, materials enable crafting, crafted items degrade and require repair, repair consumes materials harvested from new corpses.

### Core Systems

**Extraction systems** convert corpses and environment into raw materials. Skinning extracts hides from corpses based on race mapping. Butchering produces food. Dissection yields magical components for shamans. Foraging gathers wild food from terrain. These systems prevent double-extraction through corpse flags and enforce concurrency locks.

**Creation systems** consume components to produce items. Brewing creates potions by combining magical elixir, spell components, and brew reagents - draining lifeforce over multiple pulses. Scribing creates scrolls from parchment and spell components, consuming mana. Both support batch creation limited by component charges. Failure produces worthless items rather than nothing, consuming components either way.

**Maintenance systems** restore degraded equipment. Repair requires skill-appropriate tools, room fixtures, and raw material consumption based on weight and structure points being restored. Sharpen restores weapon sharpness using whetstones. Both consume resources and can fail based on skill checks.

**Growth systems** enable agriculture. Planting converts seed tools into plant objects that age over time, eventually producing harvestable items. Plants track age and environmental suitability, withering in invalid conditions.

### Common Scenarios

**Successful extraction:** Ranger with skinning knife examines bear corpse. Corpse weight determines maximum hide units. Ranger's skill level determines units per success and task duration. Task completes over multiple pulses, each checking skill. On completion, bear pelt object created with value based on corpse level and skill learnedness. Corpse flagged CORPSE_HALF_SKIN or CORPSE_NO_SKIN depending on remaining yield.

**Batch crafting:** Mage with 15 parchment charges, 10 spell component charges, and 8 scribe component charges issues "scribe 10 fireball". System limits batch to minimum charge count across components (8 scrolls). Task drains mana each pulse over 16 pulses total. Each scroll receives success check - successful scrolls contain fireball at caster's learnedness, failed scrolls have 0 learnedness. All 8 component charges consumed regardless of individual success.

**Component depletion handling:** Shaman brewing 5 healing potions has flask with 3 charges. After 3 successful brews consume charges, flask hits 0 charges and gets deleted. Task automatically stops, preventing crash from accessing deleted object. Remaining 2 brews never happen.

**Concurrent extraction prevention:** Player A starts skinning corpse, setting CORPSE_PC_SKINNING flag. Player B attempts to skin same corpse, receives "already being skinned" message. Player A completes or stops task, clearing flag. Player B can now attempt skinning if corpse not flagged CORPSE_NO_SKIN.

## Patterns

### Corpse Flag Management

**Always check all relevant corpse flags before starting extraction tasks.** Flags prevent double-extraction, enforce concurrency, and mark unsuitable corpses.

```cpp
// DO: Check flags before task start
if (corpse->isCorpseFlag(CORPSE_NO_SKIN) ||
    corpse->isCorpseFlag(CORPSE_PC_SKINNING) ||
    corpse->isCorpseFlag(CORPSE_NO_REGEN)) {
    ch->sendTo("You cannot skin that.\n\r");
    return;
}
corpse->addCorpseFlag(CORPSE_PC_SKINNING);
start_task(ch, corpse, NULL, TASK_SKINNING, ...);

// DON'T: Assume corpse is valid
start_task(ch, corpse, NULL, TASK_SKINNING, ...);
```

**Why:** Multiple players can target same corpse. CORPSE_PC_SKINNING prevents concurrent skinning that would double yield. CORPSE_NO_SKIN marks exhausted corpses. CORPSE_NO_REGEN marks body parts that cannot be skinned. Failing to check allows yield duplication.

**Always clear concurrency flags when task completes or is interrupted.**

```cpp
// DO: Clear flag on all exit paths
corpse->remCorpseFlag(CORPSE_PC_SKINNING);
ch->stopTask();

// DON'T: Only clear on success path
if (success) {
    corpse->remCorpseFlag(CORPSE_PC_SKINNING);
}
```

**Why:** Task interruption (combat, movement, linkdead) without clearing lock permanently blocks corpse skinning.

### Component Charge Tracking

**Always verify component charges before task start and limit batch operations to minimum charge count.**

```cpp
// DO: Calculate batch limit from minimum charges
int how_many = min(comp_scribe->getComponentCharges(),
                   min(comp_spell->getComponentCharges(),
                       comp_gen->getComponentCharges()));

// DON'T: Trust player-provided quantity
int how_many = requested_amount;
```

**Why:** Components track separate charge counts. Starting batch larger than any component allows bypasses component consumption when first component depletes.

**Always consume charges and delete exhausted components, setting pointer to NULL.**

```cpp
// DO: Consume and clean up
comp->addToComponentCharges(-1);
if (comp->getComponentCharges() <= 0) {
    delete comp;
    comp = NULL;
}

// DON'T: Leave pointer dangling
comp->addToComponentCharges(-1);
if (comp->getComponentCharges() <= 0) {
    delete comp;  // comp still points to freed memory
}
```

**Why:** Task pulses continue accessing component pointers. Use-after-free crashes occur when task references deleted component.

### Tool Durability and Breakage

**Always check tool uses remaining before consuming use, and handle deletion gracefully.**

```cpp
// DO: Check, consume, handle deletion
tool->addToToolUses(-1);
if (tool->getToolUses() <= 0) {
    act("Your $o breaks.", FALSE, ch, tool, 0, TO_CHAR);
    delete tool;
    tool = NULL;
    ch->stopTask();
    return FALSE;  // Stop task continuation
}

// DON'T: Continue after deletion
tool->addToToolUses(-1);
if (tool->getToolUses() <= 0) {
    delete tool;
}
// Code continues using tool pointer
```

**Why:** Tools break when uses reach 0. Task pulses continue executing - accessing deleted tool causes crash.

### Task Interruption Safety

**Always handle CMD_TASK_FIGHTING to prevent task continuation during combat.**

```cpp
// DO: Stop task on combat
case CMD_TASK_FIGHTING:
    ch->sendTo("You are unable to continue while under attack!\n\r");
    ch->stopTask();
    break;

// DON'T: Allow task during combat
case CMD_TASK_FIGHTING:
    break;  // Task continues
```

**Why:** Combat can start while task active. Continuing task during combat creates animation inconsistencies and allows bypasses of combat movement restrictions.

**Always validate position and room before task pulse execution.**

```cpp
// DO: Validate state
if (ch->isLinkdead() || (ch->in_room != ch->task->wasInRoom) ||
    (ch->getPosition() < POSITION_RESTING)) {
    stop_task(ch);
    return FALSE;
}

// DON'T: Assume character state unchanged
// Continue task pulse without checks
```

**Why:** Teleportation, forced movement, knockdown all invalidate task context. Continuing creates spatial inconsistencies.

### Damage from Task Failure

**Always check reconcileDamage return value when task can damage player, and propagate DELETE_THIS.**

```cpp
// DO: Check for death
if (ch->reconcileDamage(ch, damage, SKILL_SKIN) == -1) {
    ch->stopTask();
    ch->doSave(SILENT_YES);
    return DELETE_THIS;
}

// DON'T: Ignore return value
ch->reconcileDamage(ch, damage, SKILL_SKIN);
// Continue accessing ch
```

**Why:** reconcileDamage returns -1 on death, not a DELETE flag. Critical skinning failures can kill low-hp players. Continuing after death is use-after-free.

### Material Consumption Validation

**Always calculate material needs based on object weight, structure, and repair ratio - never consume more than available.**

```cpp
// DO: Calculate and verify
int mats_needed = (int)((weight / maxStructPoints) * 10.0);
mats_needed = (int)(repair_mats_ratio * mats_needed);
if (commodity->getComponentCharges() < mats_needed) {
    ch->sendTo("Insufficient materials.\n\r");
    return FALSE;
}

// DON'T: Assume materials sufficient
commodity->addToComponentCharges(-mats_needed);
```

**Why:** Negative charges on commodities create infinite materials when wrapping occurs.

### Skill Success Probability

**Never assume skill checks guarantee success - always provide failure path that consumes resources.**

```cpp
// DO: Handle both success and failure
if (ch->bSuccess(knowledge, SKILL_BREW)) {
    potion->setDrinkSpellNum(0, spell_num);
    potion->setDrinkSpellLearnedness(0, knowledge);
} else {
    // Failure: potion becomes lemonade
    potion->setDrinkType(LIQ_LEMONADE);
    potion->setDrinkUnits(::number(0, 5));
}
comp->addToComponentCharges(-1);  // Consumed either way

// DON'T: Only handle success
if (ch->bSuccess(knowledge, SKILL_BREW)) {
    potion->setDrinkSpellNum(0, spell_num);
    comp->addToComponentCharges(-1);
}
// Failure path allows retry without consumption
```

**Why:** Failure without resource consumption enables infinite attempts until success, bypassing skill progression.

### Race and Vnum Mapping

**Always check specific mob vnum mappings before falling back to generic race mappings.**

```cpp
// DO: Vnum-specific first, then race generic
int determineSkinningItem(TCorpse* corpse) {
    // Check vnum-specific special cases
    switch (corpse->getCorpseVnum()) {
        case 12345: return SPECIAL_ITEM;
    }
    // Fall back to race mapping
    switch (corpse->getCorpseRace()) {
        case RACE_BEAR: return 2405;
    }
    return -1;
}

// DON'T: Only check race
int determineSkinningItem(TCorpse* corpse) {
    switch (corpse->getCorpseRace()) {
        case RACE_BEAR: return 2405;
    }
}
```

**Why:** Quest mobs and special encounters need unique drops. Race-only checking prevents special mob differentiation.

### Batch Operation Limits

**Always limit batch crafting to component minimum and validate each iteration independently.**

```cpp
// DO: Independent validation per item
for (int i = 0; i < how_many; i++) {
    if (ch->bSuccess(knowledge, SKILL_SCRIBE) ||
        ch->bSuccess(readmagic, SKILL_READ_MAGIC)) {
        scroll->setScrollLevel(0, knowledge);
    } else {
        scroll->setScrollLevel(0, 0);
    }
}

// DON'T: Single check for all items
if (ch->bSuccess(knowledge, SKILL_SCRIBE)) {
    for (int i = 0; i < how_many; i++) {
        scroll->setScrollLevel(0, knowledge);
    }
}
```

**Why:** Single check makes batch all-or-nothing, removing skill failure probability that balances economy.

## Reference

### Primary Symbols

| Symbol | Type | Purpose |
|--------|------|---------|
| `determineSkinningItem()` | function | Maps corpse race/vnum to hide object vnum |
| `findSomeComponent()` | function | Locates brewing or scribing components in inventory |
| `objectRepair()` | function | Shop-based repair with time delay and talen cost |
| `dissectMe()` | function | Extracts shaman components from corpse |
| `forage()` | function | Gathers wild food based on terrain sector |
| `task_skinning()` | function | Multi-pulse skinning task handler |
| `task_brew()` | function | Multi-pulse brewing task handler |
| `task_scribe()` | function | Multi-pulse scribing task handler |
| `TComponent` | class | Component object with charge tracking |
| `TPlant` | class | Plant object with age-based growth |
| `TCorpse` | class | Corpse object with extraction flags |

### Extraction Skills

| Skill | Class | Discipline | Purpose |
|-------|-------|------------|---------|
| `SKILL_SKIN` | Ranger | Advanced Adventuring | Extract hides from corpses |
| `SKILL_BUTCHER` | Ranger | Basic Adventuring | Extract meat from corpses |
| `SKILL_DISSECT` | Shaman | Shaman Control | Extract components from corpses |
| `SKILL_FORAGE` | Ranger | Advanced Adventuring | Gather wild food from terrain |

### Creation Skills

| Skill | Class | Discipline | Purpose |
|-------|-------|------------|---------|
| `SKILL_BREW` | Shaman | Shaman Alchemy | Create potions from components |
| `SKILL_SCRIBE` | Mage | Mage Alchemy | Create scrolls from components |

### Repair Skills by Material

| Material Category | Skill | Class | Required Tools |
|-------------------|-------|-------|----------------|
| Metal (iron, steel, mithril) | `SKILL_BLACKSMITHING` | Warrior | Hammer, tongs, forge room, anvil room |
| Dead (bone, flesh, ivory) | `SKILL_REPAIR_SHAMAN` | Shaman | Scalpel, forceps, operating table room |
| Organic (coral, scales) | `SKILL_REPAIR_MONK` | Monk | Ladle, plant oil, water sector |
| Wood (wood, ebony) | `SKILL_REPAIR_MONK` | Monk | Ladle, soil, water sector |
| Crystal/Gem | `SKILL_REPAIR_THIEF` or `SKILL_BLACKSMITHING_ADVANCED` | Thief/Warrior | Loupe, pliers, workbench room |
| Magical (plasma, runed, elemental) | `SKILL_REPAIR_MAGE` | Mage | Runes, energy, pentagram room |
| Rock/Stone | `SKILL_REPAIR_MAGE` or `SKILL_REPAIR_MONK` | Mage/Monk | Chisel, silica, pentagram room |
| Leather/Hide | `SKILL_REPAIR_MONK` | Monk | Punch, cording |
| Spiritual (ghostly, foodstuff) | `SKILL_REPAIR_CLERIC` or `SKILL_REPAIR_DEIKHAN` | Cleric/Deikhan | Brush, astral resin, altar room |

### Corpse Extraction Flags

| Flag | Meaning | Effect |
|------|---------|--------|
| `CORPSE_NO_SKIN` | Cannot be skinned or exhausted | Skinning blocked |
| `CORPSE_HALF_SKIN` | Partially skinned | Halves maximum yield |
| `CORPSE_PC_SKINNING` | Currently being skinned | Prevents concurrent skinning |
| `CORPSE_NO_BUTCHER` | Cannot be butchered or exhausted | Butchering blocked |
| `CORPSE_HALF_BUTCHERED` | Partially butchered | Halves maximum yield |
| `CORPSE_PC_BUTCHERING` | Currently being butchered | Prevents concurrent butchering |
| `CORPSE_NO_DISSECT` | Already dissected | Dissection blocked |
| `CORPSE_NO_REGEN` | Body part, not full corpse | Blocks skinning, butchering, dissection |

### Forage Terrain Items

| Sector Type | Item Vnum Range | Description |
|-------------|-----------------|-------------|
| Normal outdoor | 276-281 | Standard forage items |
| Arctic | 37130-37133 | Arctic forage items |
| Cave/Indoor | 37134-37136 | Cave forage items |
| Desert | 37137-37140 | Desert forage items |

### Plant Growth Stages

| Age Range | Description |
|-----------|-------------|
| 0-9 | Mound of dirt |
| 10-19 | Tiny sprout |
| 20-29 | Small plant |
| 30+ | Full plant |
| Withered | Old, dying plant |

### Task Tool Types

| Tool Type | Used By | Purpose |
|-----------|---------|---------|
| `TOOL_SKIN_KNIFE` | Skinning | Alternative to slash/pierce weapon |
| `TOOL_BUTCHER_KNIFE` | Butchering | Alternative to slash/pierce weapon |
| `TOOL_WHETSTONE` | Sharpen | Restore weapon sharpness |
| `TOOL_SEED` | Planting | Seeds to plant |

## Implementation

### Skinning Yield Calculation

Skinning yield depends on corpse weight, skill level, and prior extraction. Maximum units available from a corpse is 10% of corpse weight divided by 2, minus 1, with minimum 1 unit. If corpse has CORPSE_HALF_SKIN flag, maximum is halved.

Units gained per successful skill check equals skill learnedness divided by 25, minimum 1. Higher skill extracts more units per pulse.

Task duration is calculated as 5 base pulses plus the lesser of: skill-adjusted duration (skillLevel * 2 plus bonus from skillValue above 70) or yield-adjusted duration (maximum units divided by corpse effect, plus 1, divided by units per success). This caps duration so high-skill players don't waste time on small corpses.

Created item weight is minimum of corpse weight or 2% of corpse weight scaled by units extracted versus maximum. Volume follows same formula. Cost equals corpse level times skill-adjusted multiplier (0.9 + learnedness/100), times total units divided by 3.

Skinning item vnum comes from determineSkinningItem which first checks corpse vnum for hardcoded special cases, then falls back to race mapping table. This allows quest-specific mob drops while providing sensible defaults for standard races.

Critical failures damage the player by 5 plus half weapon sharpness, create blood object in room, and dull weapon by 3 sharpness. Double failures dull by 2 without damage. Single failures dull by 1 but still grant units. Weapon sharpness affects cutting effectiveness - dull weapons increase failure chance.

### Butchering Yield Calculation

Butchering maximum units equals 10% of corpse weight minus 1, minimum 0. If corpse has CORPSE_HALF_BUTCHERED flag, maximum is halved. Food fill value is capped at 100, calculated from maximum units divided by corpse effect.

Created steak object uses Obj::GENERIC_STEAK base object. Name includes corpse race name and randomly selected cut type from predefined list of 13 cuts (rib-eye, chuck-eye, skirt, flank, t-bone, porterhouse, tenderloin, sirloin, tri-tip, chuck, ribs, short loin, filet mignon). Weight equals food fill value divided by 10. Volume equals food fill value times 10.

Bare-hands butchering requires TALENT_MEATEATER racial talent. Critical failure deals 25 damage on bare-hands butchering versus variable damage with tools.

### Brewing Component Resolution

Brewing requires three components: flask containing magical elixir liquid type, spell component matching target spell, and brew component specific to spell being brewed. findSomeComponent searches inventory for these three component types.

Component type 1 (brewing) searches for generic brew component, then spell-specific brew component, then spell component. Type 2 (scribing) searches for generic scribe component, then spell-specific scribe component, then spell component. This ordering allows spell component to satisfy multiple roles if specific components unavailable.

Task duration is requested quantity times 2 pulses, each pulse lasting 7 * Pulse::MOBACT. Each pulse drains lifeforce based on skill value: (skillValue * 3 + skillValue * 2) / 15.

Success check uses bSuccess on SKILL_BREW. Success sets potion spell number and learnedness. Failure sets drink type to LIQ_LEMONADE with 0-5 random units. Components consumed regardless of success.

### Scribing Batch Limits

Batch scribing calculates how_many as minimum of: scribe component charges, spell component charges, generic component charges. This ensures no component depletes mid-batch causing access to deleted component.

Each scroll in batch receives independent skill check against either SKILL_SCRIBE or SKILL_READ_MAGIC. Success sets scroll learnedness to caster knowledge. Failure sets learnedness to 0, making scroll unreadable.

Mana drain per pulse: (skillValue * 6 + skillValue * 4) / 15. Higher skill costs more mana but produces better scrolls.

### Repair Material Consumption

Material consumption for repair is calculated from object weight divided by max structure points, times 10, times repair materials ratio (0.10). This gives units of raw material needed per structure point being restored.

Monogrammed items receive 75% reduction in material cost - materials needed divided by 4. This represents personal attunement making repairs more efficient.

Maximum repairable structure is object max structure minus depreciation, times 95%. This 95% cap prevents perfect restoration - wear accumulates over time. Shop-owned repair applies quality modifier to this cap. Quality values less than 1.0 reduce maximum, representing lower-quality shop work.

Player repair is immediate task-based, consuming movement/mana/lifeforce and raw material commodities. Shop repair is real-time delayed, consuming talens but not materials. Shop repair quality is shop-configurable. Player repair quality is skill-limited.

### Dissection Item Resolution

Dissection items determined by three-level lookup. First checks race-specific hardcoded items. Phoenix yields flaming sword or phoenix feather. Deer yields venison. Dragon yields scales. This allows special drops for significant creatures.

Second checks vnum-specific hardcoded items. Quest mobs and unique encounters map to specific component vnums.

Third reads dissect_array from objdata/dissect file. Format is mob vnum, item vnum, percentage chance, self message, others message. This allows builders to configure dissection without code changes.

Success requires passing bSuccess on SKILL_DISSECT or having TOG_STARTED_MONK_BLUE quest bit. Then requires passing percentage roll against amount value. Failure calls CF macro marking critical fail for skill tracking.

CORPSE_NO_DISSECT flag added after successful dissection prevents repeat dissection. Unlike skinning/butchering which allow partial extraction, dissection is one-time.

### Forage Terrain Validation

Forage checks room sector type against valid list. Forest, beach, hills, mountains, nature, road, swamp, arctic allowed. City, flying, vertical, underwater, air, ocean, river disallowed. Indoor sectors allowed only if cave type.

Room flags ROOM_FLOODED and ROOM_ON_FIRE block foraging. These represent environmental hazards preventing resource gathering.

Movement cost is random 5-15 points. Insufficient movement prevents forage attempt.

Multiple item yield uses decreasing probability loop. Initial probability 1000 out of 1000. Each successful item load divides probability by 3. Loop continues until random roll fails. This creates exponential falloff - most forages yield 1-2 items, rare cases yield 3-4.

Insect foraging for races with TALENT_INSECT_EATER uses separate terrain_insects table. Food calculation: random 5-20 divided by 10, times terrain base food, times FORAGE_INSECT_FOOD constant (4). If SKILL_FORAGE known and success, multiply by (skillValue + 50) / 50 for up to 50% bonus.

Success adds 4 mud hour cooldown affect. Failure adds 2 mud hour cooldown. This prevents forage spam.

### Plant Growth and Aging

Plant objects track age counter incremented on update. Age determines description shown. 0-9 age shows "mound of dirt". 10-19 shows "tiny sprout". 20-29 shows "small plant". 30+ shows full plant name based on seed type.

Withering occurs when environment becomes invalid or age exceeds threshold. Withered plants show special description.

Seed vnum maps to plant type through seed_to_plant function. Each seed vnum corresponds to specific plant type with defined growth stages and mature harvest.

Planting task lasts 3 pulses representing dig, plant, cover. Certain seed vnums (31026-31031) apply poison to planter during task if not immune. This represents dangerous plants requiring care.

Maximum 8 plants per room enforced by counting existing plant objects in room. This prevents resource spam.

### Sharpen Weapon Mechanics

Sharpen task consumes whetstone tool uses each pulse. Movement cost 2d3 per pulse. Success check on SKILL_SHARPEN adds 1 sharpness (2 for arrows).

Current sharpness cannot exceed max sharpness. When reached, task stops with "doesn't seem to be getting any sharper" message.

Tool breaks when uses reach 0. Deletion handled by setting pointer NULL and stopping task. This prevents subsequent pulse accessing freed tool memory.

Weapon sharpness affects combat effectiveness. Dulling from skinning/butchering failures reduces effectiveness until sharpened.

### Component Charge Management

Components track integer charge count. addToComponentCharges accepts negative values for consumption. When charges reach 0 or below, component should be deleted and pointer set NULL.

Batch operations calculate minimum charges across all required components before starting. This ensures no component depletes mid-task.

Tasks store component pointers in task structure. Task pulses decrement charges. When component deleted, task must stop or handle NULL component gracefully.

findSomeComponent searches inventory for components matching required types. Returns pointers to found components or NULL if insufficient.

## Troubleshooting

### Symptom: Corpse allows infinite skinning

**Likely cause:** CORPSE_NO_SKIN or CORPSE_HALF_SKIN flag not being set after extraction.

**Diagnostic approach:** Check task completion code. Verify flag setting occurs on all completion paths including success, failure, and interruption. Examine corpse object flags after extraction attempt.

**Fix:** Ensure task completion sets appropriate flag. Success should set CORPSE_NO_SKIN if all units extracted, CORPSE_HALF_SKIN otherwise. All exit paths must clear CORPSE_PC_SKINNING concurrency lock.

### Symptom: Crash during brewing task pulse

**Likely cause:** Component deleted due to charge depletion but pointer not set NULL. Subsequent pulse accesses freed memory.

**Diagnostic approach:** Enable ASan. Reproduce brewing task. Check for heap-use-after-free reports. Examine component charge values before crash pulse.

**Fix:** After calling addToComponentCharges, immediately check if charges <= 0. If so, delete component and set pointer to NULL. Verify all subsequent pulse code checks component != NULL before accessing.

### Symptom: Players can skin corpses concurrently

**Likely cause:** CORPSE_PC_SKINNING flag not being checked or not being set before task start.

**Diagnostic approach:** Log all skinning attempts with corpse flags. Check if flag present when second player attempts skinning. Verify flag being set in task start code.

**Fix:** Before starting skinning task, check corpse->isCorpseFlag(CORPSE_PC_SKINNING). If set, deny attempt. If not set, add flag immediately before start_task call. Ensure task cleanup removes flag.

### Symptom: Batch scribing creates more scrolls than components should allow

**Likely cause:** Batch size calculated from requested amount rather than minimum component charges.

**Diagnostic approach:** Add logging showing component charge values and calculated batch size. Compare batch size to each component's charges. Check if any component has fewer charges than batch size.

**Fix:** Calculate batch size as minimum of all component charge counts: min(comp1->getComponentCharges(), min(comp2->getComponentCharges(), comp3->getComponentCharges())). Never trust player-provided quantity.

### Symptom: Tool continues working after uses reach 0

**Likely cause:** Tool uses decremented but tool not deleted, or task continues after deletion.

**Diagnostic approach:** Log tool uses value before and after decrement. Check if deletion code being reached. Verify task stops when deletion occurs.

**Fix:** After addToToolUses(-1), immediately check getToolUses() <= 0. If true, delete tool, set pointer NULL, call ch->stopTask(), and return FALSE to prevent pulse continuation.

### Symptom: Task continues during combat

**Likely cause:** CMD_TASK_FIGHTING case missing or not calling stopTask.

**Diagnostic approach:** Review task switch statement. Check if CMD_TASK_FIGHTING case exists. Verify case calls stopTask.

**Fix:** Add case CMD_TASK_FIGHTING: with message to player and ch->stopTask() call. Ensure break statement prevents fallthrough.

### Symptom: Player dies during skinning but game crashes

**Likely cause:** reconcileDamage return value not checked. Code continues accessing deleted player object.

**Diagnostic approach:** Enable ASan. Check for use-after-free in skinning critical failure path. Verify DELETE_THIS propagation.

**Fix:** Check if (ch->reconcileDamage(ch, damage, SKILL_SKIN) == -1). If true, call ch->stopTask(), ch->doSave(SILENT_YES), and return DELETE_THIS immediately.

### Symptom: Foraged items appear in city or underwater

**Likely cause:** Sector type validation missing or incomplete.

**Diagnostic approach:** Log sector type when forage called. Check against valid sector list. Verify room flags being checked.

**Fix:** Before forage yield calculation, validate sector in allowed list. Check room flags for ROOM_FLOODED and ROOM_ON_FIRE. Return SPELL_FAIL if invalid.

### Symptom: Repair consumes negative materials or creates materials

**Likely cause:** Material calculation not validated against available commodity charges.

**Diagnostic approach:** Log calculated material needs and commodity charge count. Check if material needs exceeds available charges. Look for negative values.

**Fix:** After calculating mats_needed, verify commodity->getComponentCharges() >= mats_needed. If insufficient, return failure before calling addToComponentCharges. Never allow negative charge values.

### Symptom: Plant appears in invalid sector

**Likely cause:** Sector validation during planting missing.

**Diagnostic approach:** Check room sector when plant task starts. Verify sector against valid outdoor types. Log plant creation events with sector.

**Fix:** Before starting plant task, validate sector is outdoor and not fall/water/underwater. Return failure if invalid. Ensure plant aging checks sector and withers in invalid locations.
