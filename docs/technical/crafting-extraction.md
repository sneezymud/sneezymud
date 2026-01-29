---
title: Crafting and Extraction Systems
description: Harvesting materials from corpses via skinning/butchering, creating potions/scrolls through brewing/scribing, repairing equipment with skill-based material consumption, and gathering resources through foraging/dissection/planting.
keywords:
  - SKILL_SKIN
  - SKILL_BUTCHER
  - SKILL_BREW
  - SKILL_SCRIBE
  - SKILL_DISSECT
  - SKILL_FORAGE
  - SKILL_BLACKSMITHING
  - CORPSE_NO_SKIN
  - CORPSE_HALF_SKIN
  - TComponent
  - determineSkinningItem
  - findSomeComponent
  - objectRepair
  - task_skinning
  - task_brew
category: Understanding Systems
related:
  - material-system.md
  - component-system.md
  - task-system.md
  - object-types.md
  - economy-system.md
last_updated: 2026-01-29
source_files:
  - code/code/task/task_skin.cc
  - code/code/disc/disc_advanced_adventuring.cc
  - code/code/task/task_butcher.cc
  - code/code/task/task_brew.cc
  - code/code/disc/disc_shaman_alchemy.cc
  - code/code/task/task_scribe.cc
  - code/code/disc/disc_mage_alchemy.cc
  - code/code/misc/repair.cc
  - code/code/task/task_blacksmithing.cc
  - code/code/task/task_sharpen.cc
  - code/code/obj/obj_base_weapon.cc
  - code/code/disc/disc_basic_combat.cc
  - code/code/cmd/cmd_dissect.cc
  - code/code/obj/obj_base_corpse.cc
  - code/code/task/task_plant.cc
  - code/code/obj/obj_plant.cc
---

# Crafting and Extraction Systems

The crafting and extraction systems in SneezyMUD enable players to harvest materials from corpses, create consumable items, repair equipment, and gather resources from the environment. These interconnected systems involve skills primarily associated with Rangers, Shamans, Mages, and Warriors.

**Misusing these systems causes item/economy issues.** Common errors: ignoring corpse flags (double-skinning), not checking tool uses (crash on deleted tool), incorrect component consumption (duplication), missing skill checks (bypassing progression).

## Skinning/Butchering

### Overview

Skinning and butchering extract materials from corpses. Skinning yields hides and leather used for crafting, while butchering produces edible food. Both require appropriate tools and the corresponding skill.

### Skill Requirements

| Skill | Class | Disc | Purpose |
|-------|-------|------|---------|
| `SKILL_SKIN` | Ranger | Advanced Adventuring | Extract hides from corpses |
| `SKILL_BUTCHER` | Ranger | Basic Adventuring | Extract meat from corpses |

### Skinning Mechanics

**Required Equipment:**
- Primary hand: Slash or pierce weapon (volume <= 6000) OR skinning knife tool (`TOOL_SKIN_KNIFE`)

**Yield Formula:**

```cpp
// Maximum units possible from corpse
maxUnitsP = max(1, (int)((corpse->getWeight() * 0.10) / 2) - 1);

// Units gained per success (skill-based)
unitsPerSuccess = max(1, (int)(learning / 25));

// If corpse was half-skinned previously, halve maximum
if (corpse->isCorpseFlag(CORPSE_HALF_SKIN))
    maxUnitsP /= 2;
```

**Source:** `code/code/task/task_skin.cc`

**Time to Complete:**

```cpp
// Pulse calculation
skin_pulses = 5 + min(
    max((int)(skillLevel * 2) + ((skillValue - 70) / 10), 4),
    (int)(((maxUnitsP / corpseEffect) + 1) / unitsPerSuccess)
);
```

**Source:** `code/code/disc/disc_advanced_adventuring.cc`

**Item Value Calculation:**

```cpp
// Weight and volume
weight = min(corpseWeight, max(1, (int)((corpseWeight * 0.02) * (totalUnits / maxUnitsP))));
volume = min(corpseVolume, max(1, (int)((corpseVolume * 0.02) * (totalUnits / maxUnitsP))));

// Cost
cost = max(1, (int)(corpseLevel * (0.9 + (learning / 100))) * (totalUnits / 3));
```

**Source:** `code/code/task/task_skin.cc`

**Skinning Item Lookup:**

Skinning items are determined by `determineSkinningItem()` which checks:
1. Specific mob vnum (hardcoded special cases)
2. Generic race mapping (RACE_* to item vnum)

**Source:** `code/code/disc/disc_advanced_adventuring.cc`

| Race | Item vnum | Description |
|------|-----------|-------------|
| RACE_SQUIRREL | 2401 | Squirrel pelt |
| RACE_DEER | 2402 | Deer hide |
| RACE_BEAR | 2405 | Bear pelt |
| RACE_WOLF | 2403 | Wolf fur |
| RACE_DRAGON | Various | Dragon scales |
| (many more) | ... | ... |

### Butchering Mechanics

**Required Equipment:**
- Primary hand: Slash or pierce weapon OR butcher knife tool (`TOOL_BUTCHER_KNIFE`)
- OR: Bare hands if race has `TALENT_MEATEATER` talent

**Yield Formula:**

```cpp
// Maximum food units possible
maxUnitsP = max(0, (int)(corpse->getWeight() * 0.10) - 1);

// Food fill value
FoodUnits = max(0, min(100, (maxUnitsP / Ceffect)));
```

**Source:** `code/code/task/task_butcher.cc`

**Output:**
- Creates a generic steak object (`Obj::GENERIC_STEAK`)
- Steak name includes race name and random cut type
- Weight: `FoodUnits / 10.0`
- Volume: `FoodUnits * 10`

**Meat Cut Types:**

```cpp
const static char* meats[] = {
    "rib-eye steak", "chuck-eye steak", "skirt steak",
    "flank steak", "t-bone steak", "porterhouse steak",
    "tenderloin steak", "sirloin steak", "tri-tip steak",
    "chuck steak", "set of ribs", "short loin steak",
    "filet mignon steak"
};
```

**Source:** `code/code/task/task_butcher.cc`

### Corpse Flags

| Flag | Meaning | Effect |
|------|---------|--------|
| `CORPSE_NO_SKIN` | Cannot be skinned | Skinning blocked |
| `CORPSE_HALF_SKIN` | Partially skinned | Halves maximum yield |
| `CORPSE_PC_SKINNING` | Currently being skinned | Prevents concurrent skinning |
| `CORPSE_NO_BUTCHER` | Cannot be butchered | Butchering blocked |
| `CORPSE_HALF_BUTCHERED` | Partially butchered | Halves maximum yield |
| `CORPSE_PC_BUTCHERING` | Currently being butchered | Prevents concurrent butchering |
| `CORPSE_NO_REGEN` | Body part (not full corpse) | Blocks skinning/butchering |

### Failure Effects

**Skinning Failures:**

| Failure Type | Effect |
|--------------|--------|
| Single fail | Weapon dulls by 1 sharpness, still gain units |
| Double fail | Weapon dulls by 2 sharpness, no units gained |
| Critical fail | Weapon dulls by 3, player takes damage (5 + weapon_sharpness/2), drops blood |

**Source:** `code/code/task/task_skin.cc`

**Butchering Failures:**

Similar structure to skinning, with bare-hands butchering causing 25 damage on critical fail.

**Source:** `code/code/task/task_butcher.cc`

## Brewing/Scribing

### Overview

Brewing creates potions (Shaman skill), while scribing creates scrolls (Mage skill). Both consume components and produce magical items containing spells.

### Skill Requirements

| Skill | Class | Disc | Purpose |
|-------|-------|------|---------|
| `SKILL_BREW` | Shaman | Shaman Alchemy | Create potions |
| `SKILL_SCRIBE` | Mage | Mage Alchemy | Create scrolls |

### Brewing Mechanics

**Required Components:**
1. Flask of magical elixir (`LIQ_MAGICAL_ELIXIR`)
2. Spell component (for target spell)
3. Brew component (spell-specific brewing reagent)

**Lifeforce Cost:**

```cpp
int factor1 = (skillValue * 3);
int factor2 = (skillValue * 2);
int resulting = ((factor1 + factor2) / 15);
// Lifeforce drained each pulse: resulting
```

**Source:** `code/code/task/task_brew.cc`

**Duration:**

```cpp
// Brewing takes (how_many * 2) pulses
// Each pulse is 7 * Pulse::MOBACT
```

**Source:** `code/code/task/task_brew.cc`

**Success/Failure:**

| Outcome | Effect |
|---------|--------|
| Success | Potion contains target spell with full learnedness |
| Failure | Potion becomes lemonade (0-5 random units) |

**Source:** `code/code/task/task_brew.cc`

### Scribing Mechanics

**Required Components:**
1. Parchment component (generic scribe component)
2. Spell component (for target spell)
3. Scribe component (spell-specific scribing reagent)

**Mana Cost:**

```cpp
int factor1 = (skillValue * 6);
int factor2 = (skillValue * 4);
int resulting = ((factor1 + factor2) / 15);
// Mana drained each pulse: resulting
```

**Source:** `code/code/task/task_scribe.cc`

**Batch Scribing:**

Players can scribe multiple scrolls at once using syntax: `scribe <number> <spell>`

```cpp
// Limited by minimum charges across all components
how_many = min(comp_scribe->getComponentCharges(),
               min(comp_spell->getComponentCharges(),
                   comp_gen->getComponentCharges()));
```

**Source:** `code/code/disc/disc_mage_alchemy.cc`

**Success Check:**

```cpp
if (ch->bSuccess(knowledge, SKILL_SCRIBE) ||
    ch->bSuccess(readmagic, SKILL_READ_MAGIC)) {
    // Success: scroll gets proper spell learnedness
} else {
    // Failure: scroll learnedness = 0 (unreadable)
}
```

**Source:** `code/code/task/task_scribe.cc`

### Component System Integration

Both brewing and scribing use `findSomeComponent()` to locate required components:

```cpp
// type 1 = brewing, type 2 = scribing
t->findSomeComponent(&comp_gen, &comp_spell, &comp_class, which_spell, type);
```

Components have:
- `getComponentCharges()` - Number of uses remaining
- `addToComponentCharges(n)` - Add/subtract charges
- Component type flags for spell/brew/scribe classification

## Forging/Repair

### Overview

The repair system allows players to restore damaged equipment. Different material types require different repair skills and tools.

### Repair Skills by Material

| Material Category | Skill | Class | Tools Required |
|-------------------|-------|-------|----------------|
| Metal (iron, steel, mithril, etc.) | `SKILL_BLACKSMITHING` | Warrior | Hammer, tongs, forge (room), anvil (room) |
| Dead (bone, flesh, ivory) | `SKILL_REPAIR_SHAMAN` | Shaman | Scalpel, forceps, operating table (room) |
| Organic (coral, scales) | `SKILL_REPAIR_MONK` | Monk | Ladle, plant oil, water sector |
| Wood (wood, ebony) | `SKILL_REPAIR_MONK` | Monk | Ladle, soil, water sector |
| Crystal/Gem | `SKILL_REPAIR_THIEF` or `SKILL_BLACKSMITHING_ADVANCED` | Thief/Warrior | Loupe, pliers, workbench (room) |
| Magical (plasma, runed, elemental) | `SKILL_REPAIR_MAGE` | Mage | Runes, energy, pentagram (room) |
| Rock/Stone | `SKILL_REPAIR_MAGE` or `SKILL_REPAIR_MONK` | Mage/Monk | Chisel, silica, pentagram (room) |
| Leather/Hide | `SKILL_REPAIR_MONK` | Monk | Punch, cording |
| Spiritual (ghostly, foodstuff) | `SKILL_REPAIR_CLERIC` or `SKILL_REPAIR_DEIKHAN` | Cleric/Deikhan | Brush, astral resin, altar (room) |

**Source:** `code/code/task/task_blacksmithing.cc`

### Shop Repair vs Player Repair

**Shop Repair:**
- NPCs with repairman spec proc
- Time-delayed (real-time wait)
- Costs talens (material cost + labor)
- Quality limited by shop setting

**Player Repair:**
- Immediate (task-based pulses)
- Costs movement/mana/lifeforce
- Consumes raw materials (commodities)
- Quality limited by skill

### Repair Quality Formula

```cpp
// Maximum repairable structure
int maxFix = getMaxStructPoints() - getDepreciation();
maxFix = (maxFix * 95) / 100;  // 95% cap

// If shop-owned, apply quality modifier
if (shop_index[shop_nr].isOwned()) {
    float quality = shop_index[shop_nr].getRepairQuality();
    if (quality <= 1.0 && quality > 0)
        maxFix = (int)((float)maxFix * quality);
}
```

**Source:** `code/code/misc/repair.cc`

### Raw Material Consumption

```cpp
// Units of material needed per structure point repaired
int mats_needed = (int)((weight / maxStructPoints) * 10.0);
mats_needed = (int)(repair_mats_ratio * mats_needed);  // 0.10 ratio

// Monogrammed items: 25% of normal materials
if (obj->isMonogrammed())
    mats_needed = mats_needed / 4;
```

**Source:** `code/code/task/task_blacksmithing.cc`

### Sharpen Mechanics

Sharpening restores weapon sharpness using a whetstone.

**Command:** `sharpen [weapon]`

**Requirements:**
- `SKILL_SHARPEN` skill
- Weapon held in primary hand
- Whetstone tool (`TOOL_WHETSTONE`) in possession

**Mechanics:**

```cpp
// Movement cost per pulse
sharp_move = dice(2, 3);

// Success check
if (ch->bSuccess(SKILL_SHARPEN))
    addToCurSharp((itemType() == ITEM_ARROW) ? 2 : 1);

// Cannot exceed max sharpness
if (getMaxSharp() <= getCurSharp()) {
    ch->sendTo("It doesn't seem to be getting any sharper.\n\r");
    ch->stopTask();
}
```

**Source:** `code/code/obj/obj_base_weapon.cc`

**Tool Wear:**
- Whetstone loses 1 use per pulse
- Tool destroyed when uses reach 0

## Dissect/Analyze

### Overview

Dissection extracts shaman components and special items from corpses. This is distinct from skinning, which extracts hides.

### Skill Requirements

| Skill | Class | Disc | Purpose |
|-------|-------|------|---------|
| `SKILL_DISSECT` | Shaman | Shaman Control | Extract components from corpses |

### Dissection Mechanics

**Command:** `dissect <corpse>`

**Requirements:**
- `SKILL_DISSECT` skill
- Working hands
- Not in berserk mode
- Not mounted

**Item Determination:**

Dissection items come from three sources (checked in order):
1. Race-specific hardcoded items
2. Vnum-specific hardcoded items (special quest cases)
3. `dissect_array` data file (`objdata/dissect`)

```cpp
// Race-specific examples
switch (corpse->getCorpseRace()) {
    case RACE_PHOENIX:
        num = COMP_FLAMING_SWORD;  // or Obj::PHOENIX_FEATHER
        break;
    case RACE_DEER:
        num = Obj::VENISON;
        break;
}
```

**Source:** `code/code/cmd/cmd_dissect.cc`

### Dissect Data File Format

The `objdata/dissect` file defines mob-to-item mappings:

```
<mob_vnum> <item_vnum> <amount>
<message_to_self>
<message_to_others>
```

**Source:** `code/code/cmd/cmd_dissect.cc`

### Success Formula

```cpp
// Skill check
if (!caster->bSuccess(bKnown, SKILL_DISSECT) &&
    !caster->hasQuestBit(TOG_STARTED_MONK_BLUE)) {
    // Failure: nothing found
    return TRUE;
}

// Amount check (percentage chance)
if (::number(0, 99) >= amount) {
    // Failed amount roll
    CF(SKILL_DISSECT);  // Mark critical fail
    return TRUE;
}
```

**Source:** `code/code/obj/obj_base_corpse.cc`

### Corpse Flags

| Flag | Effect |
|------|--------|
| `CORPSE_NO_REGEN` | Body part - cannot dissect |
| `CORPSE_NO_DISSECT` | Already dissected - blocked |

After successful dissection, `CORPSE_NO_DISSECT` is added to prevent re-dissection.

## Plant/Forage

### Overview

Planting grows crops from seeds, while foraging gathers wild food. Both are terrain-dependent skills.

### Skill Requirements

| Skill | Class | Disc | Purpose |
|-------|-------|------|---------|
| `SKILL_FORAGE` | Ranger | Advanced Adventuring | Gather wild food |
| (No skill) | Any | - | Plant seeds (uses `SKILL_PLANT` for thief plant) |

### Forage Mechanics

**Command:** `forage`

**Valid Sectors:**
- Forest, Beach, Hills, Mountains, Nature, Road, Swamp, Arctic sectors
- NOT: City, Flying, Vertical, Underwater, Air, Ocean, River

**Invalid Conditions:**
- Room flooded (`ROOM_FLOODED`)
- Room on fire (`ROOM_ON_FIRE`)
- Indoor sectors (except caves)

**Movement Cost:**

```cpp
int forage_move = ::number(5, 15);
if (caster->getMove() < forage_move) {
    caster->sendTo("You lack the vitality to forage.\n\r");
    return SPELL_FAIL;
}
caster->addToMove(-forage_move);
```

**Source:** `code/code/disc/disc_advanced_adventuring.cc`

**Cooldown:**

```cpp
// Success cooldown: 4 mud hours
aff.duration = 4 * Pulse::UPDATES_PER_MUDHOUR;

// Failure cooldown: 2 mud hours
aff.duration = 2 * Pulse::UPDATES_PER_MUDHOUR;
```

**Source:** `code/code/disc/disc_advanced_adventuring.cc`

### Forage Items by Terrain

| Sector Type | Item Range | Description |
|-------------|------------|-------------|
| Normal outdoor | 276-281 | Standard forage items |
| Arctic | 37130-37133 | Arctic forage items |
| Cave/Indoor | 37134-37136 | Cave forage items |
| Desert | 37137-37140 | Desert forage items |

**Source:** `code/code/disc/disc_advanced_adventuring.cc`

### Multiple Item Yield

```cpp
int foodpile = 1000;
while (::number(0, 999) < foodpile) {
    // Load item based on terrain
    foodpile /= 3;  // Decreasing chance for additional items
}
```

**Source:** `code/code/disc/disc_advanced_adventuring.cc`

### Insect Foraging (Racial Talent)

Races with `TALENT_INSECT_EATER` use insect foraging instead:

```cpp
// Food calculation
food = (::number(5, 20) / 10);
food *= terrain_insects[sector].baseFood;
food *= FORAGE_INSECT_FOOD;  // 4

// If foraging skill known, up to +50% bonus
if (doesKnowSkill(SKILL_FORAGE) && bSuccess(SKILL_FORAGE))
    food *= (skillValue + 50) / 50;
```

**Source:** `code/code/disc/disc_advanced_adventuring.cc`

### Planting Mechanics

**Command:** `plant <seeds>`

**Requirements:**
- Seeds tool (`TOOL_SEED`) in inventory
- Valid outdoor sector (not fall, water, indoor, underwater)
- Room not full of plants (maximum 8 plants per room)

**Task Duration:** 3 pulses (dig, plant, cover)

**Poisonous Seeds:**

Certain seeds poison the planter:

```cpp
switch(seeds->objVnum()) {
    case 31026:  // death camas
    case 31027:  // jimson weed
    case 31028:  // hemlock
    case 31029:  // monkshood
    case 31030:  // sweet pea
    case 31031:  // acacia
        // Apply poison if not immune
}
```

**Source:** `code/code/task/task_plant.cc`

### Plant Growth

Plants grow over time through age increments:

| Age Range | Description |
|-----------|-------------|
| 0-9 | Mound of dirt |
| 10-19 | Tiny sprout |
| 20-29 | Small plant |
| 30+ | Full plant |
| Withered | Old, dying plant |

**Source:** `code/code/obj/obj_plant.cc`

### Seed to Plant Mapping

```cpp
int seed_to_plant(int vnum) {
    switch (vnum) {
        case 13880: return 0;   // Tomato
        case 13881: return 1;   // Red rose
        case 13882: return 2;   // Apple tree
        case 13883: return 3;   // White rose
        case 13884: return 4;   // Yellow rose
        case 13885: return 5;   // Orange tree
        case 13886: return 6;   // Money tree
        // ... etc
    }
}
```

**Source:** `code/code/obj/obj_plant.cc`

## Code References

| System | File | Key Lines |
|--------|------|-----------|
| Skinning task | `code/code/task/task_skin.cc` | skinPulse, task_skinning |
| Skinning items | `code/code/disc/disc_advanced_adventuring.cc` | determineSkinningItem |
| Butchering task | `code/code/task/task_butcher.cc` | butcherPulse, task_butchering |
| Bare-hands butcher | `code/code/task/task_butcher.cc` | bareHandsButcherPulse |
| Brewing task | `code/code/task/task_brew.cc` | task_brew |
| Brewing command | `code/code/disc/disc_shaman_alchemy.cc` | doBrew |
| Scribing task | `code/code/task/task_scribe.cc` | task_scribe |
| Scribing command | `code/code/disc/disc_mage_alchemy.cc` | doScribe |
| Shop repair | `code/code/misc/repair.cc` | repairman, maxFix, repairPrice |
| Player repair | `code/code/task/task_blacksmithing.cc` | BaseRepair, MetalRepair |
| Sharpen task | `code/code/task/task_sharpen.cc` | task_sharpening |
| Sharpen weapon | `code/code/obj/obj_base_weapon.cc` | sharpenMe |
| Sharpen command | `code/code/disc/disc_basic_combat.cc` | doSharpen, sharpen |
| Dissect command | `code/code/cmd/cmd_dissect.cc` | doDissect |
| Dissect corpse | `code/code/obj/obj_base_corpse.cc` | dissectMe |
| Dissect items | `code/code/cmd/cmd_dissect.cc` | determineDissectionItem |
| Forage | `code/code/disc/disc_advanced_adventuring.cc` | forage |
| Insect forage | `code/code/disc/disc_advanced_adventuring.cc` | forage_insect |
| Plant task | `code/code/task/task_plant.cc` | task_plant |
| Plant command | `code/code/task/task_plant.cc` | doSeedPlant |
| Plant object | `code/code/obj/obj_plant.cc` | TPlant, seed_to_plant |

## Common Gotchas

### 1. Corpse Flag Checks

Always check corpse flags before starting extraction tasks:

```cpp
// BAD: Starting skinning without checking
start_task(ch, corpse, NULL, TASK_SKINNING, ...);

// GOOD: Check all relevant flags
if (corpse->isCorpseFlag(CORPSE_NO_SKIN) ||
    corpse->isCorpseFlag(CORPSE_PC_SKINNING) ||
    corpse->isCorpseFlag(CORPSE_NO_REGEN)) {
    // Handle appropriately
}
```

### 2. Tool Use Tracking

Tools break when uses reach 0 - check and handle deletion:

```cpp
tool->addToToolUses(-1);
if (tool->getToolUses() <= 0) {
    act("Your $o breaks.", FALSE, ch, tool, 0, TO_CHAR);
    ch->stopTask();
    delete tool;  // Tool is now invalid
    return FALSE;
}
```

### 3. Component Charge Consumption

Always consume charges and delete components when empty:

```cpp
comp->addToComponentCharges(-how_many);
if (comp->getComponentCharges() <= 0) {
    delete comp;
    comp = NULL;  // Prevent dangling pointer
}
```

### 4. Task Interruption

Tasks can be interrupted by combat - always handle `CMD_TASK_FIGHTING`:

```cpp
case CMD_TASK_FIGHTING:
    ch->sendTo("You are unable to continue while under attack!\n\r");
    ch->stopTask();
    break;
```

### 5. Room/Position Validation

Check room validity and position before task continuation:

```cpp
if (ch->isLinkdead() || (ch->in_room != ch->task->wasInRoom) ||
    (ch->getPosition() < POSITION_RESTING)) {
    stop_task(ch);
    return FALSE;
}
```

### 6. reconcileDamage Return Value

When tasks can damage the player, check for death:

```cpp
if (ch->reconcileDamage(ch, damage, SKILL_SKIN) == -1) {
    ch->stopTask();
    ch->doSave(SILENT_YES);
    return DELETE_THIS;  // Player died
}
```

## Related Documentation

- [Material System](material-system.md) - Material properties affecting repairs
- [Component System](component-system.md) - Component types for brewing/scribing
- [Task System](task-system.md) - Task infrastructure underlying all crafting
- [Object Types](object-types.md) - TPlant, TFood, TComponent classes
- [Economy System](economy-system.md) - Shop repair pricing
