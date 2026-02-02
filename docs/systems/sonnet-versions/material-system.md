---
title: Material Property System
category: critical
keywords: [materials, hardness, susceptibility, flammability, durability, crafting, weapon-damage, armor-protection, buoyancy, economic-value]
related: [object-system.md, equipment-wear.md, combat-formulas.md, economy-system.md]
primary_symbols:
  functions: [getMaterial, setMaterial, getMaterialTypeNumbers, getWeaponDamageBonus, getCarriedWeight]
  classes: [material_type_numbers, MetalMaterial, HideMaterial, WoodMaterial]
  files: [code/code/misc/materials.h, code/code/misc/constants.cc, code/code/misc/thing.h, code/code/obj/obj_base_weapon.cc]
---

# Material Property System

## Overview

The material system governs physical and economic properties of objects through 83 defined materials distributed across four category ranges. Each material possesses 14 distinct properties affecting weapon damage, armor protection, durability, flammability, buoyancy, economic value, and special behaviors. Materials range from common cloth and wood to legendary mithril and adamantite.

**Why this exists:** Objects need realistic physical properties that affect gameplay mechanics. A steel sword should behave differently from a wooden club. Armor materials determine protection effectiveness. Economic systems require material-based pricing. The centralized material property table enables consistent behavior across all objects without duplicating logic.

**Consequences of misuse:** Setting hardness too low renders weapons useless in combat. Incorrect susceptibility values create invulnerable armor. Missing material assignments cause objects to inherit undefined behavior from MAT_UNDEFINED. Wrong flammability values make non-combustible objects burn or prevent paper from igniting. Price imbalances break the economy when legendary materials cost less than common ones.

**Material organization:** Four category ranges exist with gaps for expansion. General materials (0-19) cover common substances like cloth, wood, and glass. Nature materials (50-77) include organic and elemental types. Mineral materials (100-126) encompass stones and gems. Metal materials (150-177) define metallic types. This leaves 117 unused slots for future materials.

**Core properties:** Hardness determines weapon damage output and armor protection effectiveness. Four susceptibility types (slash, blunt, burn, pierce) control damage vulnerability. Water susceptibility above 100 triggers rust mechanics. Flammability ratings from 0 to 1000 govern fire behavior. Float weight determines buoyancy. Volume multiplier affects inventory space. Conductivity enables lightning damage. Price establishes economic value.

**Integration points:** Combat systems query hardness for damage calculations. Durability systems use susceptibility values for wear rates. Fire mechanics check flammability before ignition. Buoyancy systems consult float_weight for water behavior. Shops multiply base prices by material price values. Crafting systems reference tier classifications and difficulty modifiers.

## Patterns

### Accessing Material Properties

Query material properties through the global material_nums array indexed by material constants. Call getMaterial on any TObj or TThing to retrieve the material type constant, then use it to access the material_type_numbers structure from material_nums.

```cpp
// Standard property lookup pattern
int matType = obj->getMaterial();
int hardness = material_nums[matType].hardness;
float price = material_nums[matType].price;
unsigned short buoyancy = material_nums[matType].float_weight;

// Check flammability before fire interactions
if (material_nums[obj->getMaterial()].flammability > 0) {
    obj->setBurning(tLunatic);
}

// Check rust susceptibility for water damage
if (material_nums[obj->getMaterial()].water_susc > 100) {
    applyWaterDamage(obj);
}

// Validate hardness for weapon effectiveness
if (material_nums[weapon->getMaterial()].hardness < 40) {
    ch->sendTo("This weapon is too soft to be effective.\n");
    return FALSE;
}
```

### Assigning Materials to Objects

Set materials using setMaterial at object creation or transformation. Always assign a material immediately after instantiation to avoid undefined behavior from MAT_UNDEFINED default. Choose materials appropriate to the object type and tier.

```cpp
// Basic assignment at creation
TObj* weapon = read_object(SWORD_VNUM, VIRTUAL);
weapon->setMaterial(MAT_STEEL);

// Crafting assignment from material selection
TObj* armor = read_object(PLATE_VNUM, VIRTUAL);
armor->setMaterial(selectedMaterial->matNum);

// Transformation preserving base material
TObj* enhanced = transformObject(original);
enhanced->setMaterial(original->getMaterial());
```

### Material-Based Damage Calculation

Scale weapon damage by hardness divided by 100 to create a multiplier. Hardness 60 (iron baseline) provides standard damage. Higher values increase effectiveness while lower values reduce output. Apply this multiplier after base damage determination but before defense calculations.

```cpp
// Weapon damage scaling in getWeaponDamageBonus
int hardness = material_nums[weapon->getMaterial()].hardness;
float damageMultiplier = hardness / 100.0;
int finalDamage = baseDamage * damageMultiplier;

// Armor protection scaling
float armorBonus = (hardness / 100.0) * baseACBonus;
```

### Material-Based Durability Loss

Calculate structure point loss using susceptibility values matched to damage types. Higher susceptibility increases degradation rate. Divide incoming damage by hardness to determine structure loss magnitude. Water damage applies only when water_susc exceeds 100. Fire damage requires non-zero flammability.

```cpp
// Combat wear on weapons
int structureLoss = (baseWeaponDamage * enemyDefense) / hardness;
weapon->addToStructPoints(-structureLoss);

// Armor degradation from attacks
int armorDamage = (incomingDamage * materialSusceptibility) / hardness;
armor->addToStructPoints(-armorDamage);

// Environmental rust damage
if (material_nums[obj->getMaterial()].water_susc > 100 && inWater) {
    obj->addToStructPoints(-waterDamagePerTick);
}

// Fire consumption
if (material_nums[obj->getMaterial()].flammability > 0 && obj->isObjStat(ITEM_BURNING)) {
    obj->addToStructPoints(-fireDamagePerTick);
}
```

### Crafting Material Selection

Reference the crafting material structures (MetalMaterial, HideMaterial, etc.) to retrieve difficulty modifiers and quality bonuses. Use findMetalMaterial or category-specific lookup functions to locate materials by number or name. Apply difficultyMod to skill checks, structureMod to durability, levelMod to requirements, and statMod to finished item bonuses.

```cpp
// Locate crafting material data
MetalMaterial* metal = findMetalMaterial(MAT_STEEL);
if (!metal) return FALSE;

// Apply crafting difficulty
int craftingDifficulty = baseDifficulty + metal->difficultyMod;
if (!skillCheck(ch, SKILL_BLACKSMITHING, craftingDifficulty)) {
    return FALSE;
}

// Set finished item properties
item->setMaxStructPoints(baseStructure + metal->structureMod);
item->addToSharpness(metal->sharpnessMod);
item->setMaterial(metal->matNum);
```

### Economic Value Calculation

Multiply object base value by material price property to determine worth. Shops apply additional markup or discount multipliers. Player charisma modifies final prices between 1.0 and 1.3. Materials range from 0.05 gold (stone) to 250 gold (diamond) per unit.

```cpp
// Shop pricing calculation
float finalPrice = objectBaseValue * material_nums[obj->getMaterial()].price;
float shopAdjusted = finalPrice * shopProfitMultiplier;
float charismaFactor = calculateCharismaModifier(ch);
int customerPrice = static_cast<int>(shopAdjusted * charismaFactor);
```

### Repair Function Dispatch

Invoke the repair_proc function pointer stored in material_type_numbers to route repairs to appropriate craftspeople. Each material category has specialized repair functions: repairMetal for metals, repairHide for leather, repairWood for wooden items, repairRock for stone, repairCrystal for gems, repairMagical for enchanted materials, repairSpiritual for elementals, repairGeneric for everything else.

```cpp
// Dynamic repair routing
int (*repairFunc)(TBeing*, TObj*) = material_nums[obj->getMaterial()].repair_proc;
if (repairFunc) {
    int result = repairFunc(craftsman, obj);
    return result;
}
```

### Material Tier Classification

Query material tier enums to determine rarity and quality classifications. Use MetalTierT for metals, HideTierT for leather, RockTierT for minerals. Tiers range from COMMON (copper, leather, stone) through UNCOMMON (iron, tough leather) and RARE (mithril, dragon scale) to LEGENDARY (adamantite, diamond). Higher tiers impose increased crafting difficulty and grant superior finished item properties.

```cpp
// Tier-based restrictions
MetalMaterial* metal = findMetalMaterial(selectedMaterial);
if (metal->tier == METAL_TIER_LEGENDARY && ch->getLevel() < 50) {
    ch->sendTo("You lack the skill to work with legendary materials.\n");
    return FALSE;
}
```

## Reference

### Material Type Constants

**MAT_UNDEFINED (0):** Default error state with minimal properties. Objects should never remain undefined.

**Common Materials (0-19):**
- MAT_PAPER (1): Flammable paper, hardness 0, flammability 1000
- MAT_CLOTH (2): Flexible fabric, hardness 5, flammability 900
- MAT_WAX (3): Low melting point, hardness 0, flammability 500
- MAT_GLASS (4): Brittle transparent, hardness 60, shatters on impact
- MAT_WOOD (5): Moderate strength, hardness 12, flammability 500
- MAT_SILK (6): Fine fabric, hardness 10, price 2.0
- MAT_FOODSTUFF (7): Perishable organic, hardness 0
- MAT_PLASTIC (8): Modern synthetic, hardness 20
- MAT_RUBBER (9): Elastic waterproof, hardness 10
- MAT_CARDBOARD (10): Low durability, hardness 0
- MAT_STRING (11): Thin fragile, hardness 0
- MAT_PLASMA (12): Magical elemental, hardness 10
- MAT_TOUGH_CLOTH (13): Reinforced fabric, hardness 7, flammability 600
- MAT_CORAL (14): Oceanic calcium, hardness 45
- MAT_HORSEHAIR (15): Animal fiber, hardness 0
- MAT_HAIR (16): Fine fiber, hardness 0
- MAT_POWDER (17): Dispersible particulate, hardness 0
- MAT_PUMICE (18): Porous volcanic rock, hardness 10
- MAT_LAMINATED (19): Layered composite, hardness 15

**Nature Materials (50-77):**
- MAT_GEN_ORG (50): Generic organic, hardness 10
- MAT_LEATHER (51): Processed hide, hardness 25, price 2.0
- MAT_TOUGH_LEATHER (52): Reinforced hide, hardness 30, price 4.0
- MAT_DRAGON_SCALE (53): Rare dragon scales, hardness 60, price 157.5
- MAT_WOOL (54): Sheep fiber, hardness 3, flammability 800
- MAT_FUR (55): Animal fur, hardness 5, flammability 800
- MAT_FEATHERED (56): Plumage, hardness 0, flammability 900
- MAT_WATER (57): Elemental water, hardness 0, water_susc 249
- MAT_FIRE (58): Elemental fire, hardness 10, burned_susc 0
- MAT_EARTH (59): Elemental earth, hardness 80
- MAT_ELEMENTAL (60): Generic elemental, hardness 50
- MAT_ICE (61): Frozen water, hardness 0, water_susc 205
- MAT_LIGHTNING (62): Electrical elemental, hardness 0, conductivity 1
- MAT_CHAOS (63): Chaotic elemental, hardness 0
- MAT_CLAY (64): Earthenware, hardness 30
- MAT_PORCELAIN (65): Refined clay, hardness 40
- MAT_STRAW (66): Plant fiber, hardness 0, flammability 1000
- MAT_PEARL (67): Lustrous gem, hardness 45, price 171
- MAT_HUMAN_FLESH (68): Organic tissue, hardness 10
- MAT_FUR_CAT (69): Feline fur, hardness 5
- MAT_FUR_DOG (70): Canine fur, hardness 5
- MAT_FUR_RABBIT (71): Rodent fur, hardness 3
- MAT_GHOSTLY (72): Spectral matter, hardness 0
- MAT_DWARF_LEATHER (73): Dwarven-crafted hide, hardness 35
- MAT_SOFT_LEATHER (74): Supple hide, hardness 20
- MAT_FISHSCALE (75): Aquatic scales, hardness 30
- MAT_OGRE_HIDE (76): Giant hide, hardness 45
- MAT_HEMP (77): Plant fiber rope, hardness 5

**Mineral Materials (100-126):**
- MAT_GEN_MINERAL (100): Generic mineral, hardness 50
- MAT_RUNESTONE (102): Magical stone, hardness 80, price 100
- MAT_CRYSTAL (103): Clear crystal, hardness 65, price 100
- MAT_DIAMOND (104): Hardest mineral, hardness 100, price 250
- MAT_EBONY (105): Dark hardwood, hardness 20, price 120
- MAT_EMERALD (106): Green gem, hardness 75, price 184.6
- MAT_IVORY (107): Carved bone, hardness 50, price 180
- MAT_OBSIDIAN (108): Volcanic glass, hardness 70, price 90
- MAT_ONYX (109): Black mineral, hardness 70, price 120
- MAT_OPAL (110): Iridescent gem, hardness 55, price 159
- MAT_RUBY (111): Red precious stone, hardness 90, price 195.5
- MAT_SAPPHIRE (112): Blue precious stone, hardness 70, price 150
- MAT_MARBLE (113): Sculptable stone, hardness 60, price 60
- MAT_STONE (114): Generic stone, hardness 95, price 0.05
- MAT_BONE (115): Processed bone, hardness 40, price 2.0
- MAT_JADE (116): Green ornamental, hardness 80, price 120
- MAT_AMBER (117): Fossilized resin, hardness 25, price 70
- MAT_TURQUOISE (118): Blue-green stone, hardness 60, price 75
- MAT_AMETHYST (119): Purple crystal, hardness 65, price 90
- MAT_MICA (120): Flaky mineral, hardness 30, price 15
- MAT_DRAGONBONE (121): Dragon skeletal, hardness 85, price 153
- MAT_MALACHITE (122): Banded green stone, hardness 40, price 60
- MAT_GRANITE (123): Igneous rock, hardness 90, price 1.0
- MAT_QUARTZ (124): Common crystal, hardness 65, price 30
- MAT_JET (125): Black fossilized wood, hardness 40, price 90
- MAT_CORUNDUM (126): Aluminum oxide gem, hardness 100, price 180

**Metal Materials (150-177):**
- MAT_COPPER (151): Soft reddish metal, hardness 35, price 0.5
- MAT_BRONZE (156): Copper-tin alloy, hardness 50, price 9.1
- MAT_BRASS (157): Copper-zinc alloy, hardness 45, price 7.5
- MAT_IRON (158): Common ferrous, hardness 60, price 5.0
- MAT_STEEL (159): Iron-carbon alloy, hardness 70, price 8.334
- MAT_MITHRIL (160): Legendary light metal, hardness 80, price 150
- MAT_ADAMANTITE (161): Legendary hard metal, hardness 95, price 90.9
- MAT_SILVER (162): Precious metal, hardness 40, price 11.112
- MAT_GOLD (163): Yellow precious, hardness 30, price 30
- MAT_PLATINUM (164): Rare precious, hardness 50, price 60
- MAT_TITANIUM (165): Light strong metal, hardness 75, price 50
- MAT_ALUMINUM (166): Light nonferrous, hardness 40, price 20
- MAT_GNICKEL (168): Nickel alloy, hardness 55, price 10
- MAT_ELECTRUM (169): Gold-silver alloy, hardness 35, price 20
- MAT_ATHANOR (170): Philosophical metal, hardness 80, price 100
- MAT_TIN (171): Soft silvery metal, hardness 25, price 5
- MAT_TUNGSTEN (172): Hard refractory, hardness 85, price 40
- MAT_STARMETAL (173): Celestial metal, hardness 90, price 120
- MAT_TERBIUM (174): Rare earth metal, hardness 60, price 80
- MAT_ETERNIUM (177): Eternal magical metal, hardness 95, price 200

### Property Definitions

**cut_susc (0-100):** Slash weapon vulnerability. Higher values increase damage from swords, axes, and slashing attacks. Metals typically have 0, while organic materials range from 10-50.

**smash_susc (0-100):** Blunt weapon vulnerability. Higher values increase damage from clubs, hammers, and crushing impacts. Brittle materials like glass have high values (75), while flexible materials resist.

**burned_susc (0-100):** Fire and heat vulnerability. Higher values increase fire damage and burn rates. Organic materials have 50-100, while metals and minerals have 0.

**pierced_susc (0-100):** Piercing weapon vulnerability. Higher values increase damage from arrows, spears, and stabbing attacks. Rigid materials typically have 0-10, flexible materials have higher values.

**hardness (0-100):** Material strength determining weapon damage output and armor protection effectiveness. Diamond has maximum 100, wood has minimum 12. Iron baseline at 60 provides standard combat effectiveness.

**water_susc (0-249):** Water exposure and corrosion vulnerability. Values above 100 trigger rust mechanics for metals or rot for organics. Mithril and adamantite have 0 (rust-proof). Iron and steel have 101 (rust-prone).

**fall_susc (0-249):** Impact damage from falls and drops. Higher values increase breakage probability. Brittle materials like glass have 200+, durable materials have 50-100.

**float_weight (0-255):** Buoyancy in water. 0 sinks immediately, 255 floats indefinitely. Metals have 0, wood typically 150-200, cork near 255.

**noise (-5 to +10):** Sound generation level affecting stealth. Negative values are silent (water -5), positive values are loud (metal 27). Cloth has 0 (neutral).

**vol_mult (1-8):** Volume multiplier affecting inventory space. Most materials use 1 (standard density). Dense materials like lead use 2-3. Magical materials with extreme density use 4-8.

**conductivity (0 or 1):** Electrical conductivity. 1 means conducts electricity (all metals, lightning elementals), 0 means insulator (wood, cloth, stone). Affects lightning damage transmission.

**flammability (0-1000):** Fire ignition and spread rating. 0 is fireproof (metals, stone, glass), 1000 is extremely flammable (paper, straw). Wood typically 500, cloth 900.

**acid_susc (0-100):** Acid corrosion vulnerability. Higher values increase acid damage. Metals typically 50-80, stone 20-40, organic materials 60-90.

**price (0.05-250 gold):** Base economic value per unit. Stone baseline at 0.05, diamond maximum at 250. Multiplied by object size to determine total value.

**repair_proc (function pointer):** Material-specific repair function. Points to repairMetal, repairHide, repairWood, repairRock, repairCrystal, repairMagical, repairSpiritual, or repairGeneric.

**mat_name (char[20]):** Material name string for display. Maximum 20 characters. Used in object descriptions and crafting interfaces.

### Crafting Structure Fields

**matNum:** Material type constant (MAT_IRON, MAT_STEEL, etc.) corresponding to material_nums array index.

**name:** Display name for crafting menus. String literal matching mat_name from material_type_numbers.

**tier:** Rarity classification enum. COMMON tier 0, UNCOMMON tier 1, RARE tier 2, LEGENDARY tier 3. Higher tiers increase difficulty and improve results.

**difficultyMod:** Integer added to crafting skill check difficulty. Negative values make crafting easier, positive values harder. Legendary materials typically +20 to +40.

**structureMod:** Integer added to finished item max_struct. Positive values increase durability, negative values reduce it. Better materials grant +10 to +30.

**levelMod:** Integer added to minimum crafter level requirement. Restricts access to materials. Legendary materials require +20 to +40 levels.

**statMod:** Integer bonus applied to finished item statistics. Magical materials grant +2 to +5, mundane materials 0.

**sharpnessMod:** Integer added to weapon sharpness (metals only). Harder metals hold edges better. Steel grants +10, adamantite +25.

**matReq:** Object vnum for raw material requirement. References database object that must be consumed during crafting.

### Material Tier Enumerations

**MetalTierT:** METAL_TIER_COMMON (copper, bronze, brass), METAL_TIER_UNCOMMON (iron, steel), METAL_TIER_RARE (mithril, silver, gold), METAL_TIER_LEGENDARY (adamantite, eternium).

**HideTierT:** HIDE_TIER_COMMON (leather, wool), HIDE_TIER_UNCOMMON (tough leather), HIDE_TIER_RARE (dragon scale), HIDE_TIER_LEGENDARY (ogre hide).

**WoodTierT:** WOOD_TIER_COMMON (generic wood), WOOD_TIER_UNCOMMON (hardwoods), WOOD_TIER_RARE (ebony), WOOD_TIER_LEGENDARY (mystical woods).

**RockTierT:** ROCK_TIER_COMMON (stone, bone), ROCK_TIER_UNCOMMON (jade, amber), ROCK_TIER_RARE (emerald, ruby, sapphire), ROCK_TIER_LEGENDARY (diamond).

**CrystalTierT:** CRYSTAL_TIER_COMMON (generic crystal), CRYSTAL_TIER_UNCOMMON (quartz, amethyst), CRYSTAL_TIER_RARE (special crystals), CRYSTAL_TIER_LEGENDARY (magical crystals).

**OrganicTierT, DeadTierT, MagicalTierT, SpiritualTierT, GenericTierT:** Follow same four-tier pattern for respective categories.

### API Functions

**getMaterial():** Returns unsigned short material type constant. Call on any TObj or TThing instance. Used as index into material_nums array.

**setMaterial(unsigned short num):** Assigns material type constant. Call during object creation or transformation. Validates num is within material_nums bounds.

**getMaterialTypeNumbers():** Returns const pointer to material_type_numbers structure. Direct access to all 14 properties. Null if material undefined.

**findMetalMaterial(int matNum):** Returns pointer to MetalMaterial structure matching matNum, or nullptr if not found. Search crafting metals array.

**findMetalMaterialByName(const sstring& name):** Returns pointer to MetalMaterial matching name string, or nullptr if not found. Case-sensitive search.

**findHideMaterial, findWoodMaterial, findRockMaterial, etc.:** Category-specific lookup functions following same patterns as metal functions. Each searches corresponding material category array.

**repairMetal(TBeing* repairer, TObj* item):** Metal repair function. Requires blacksmithing skill. Returns success code.

**repairHide(TBeing* repairer, TObj* item):** Leather repair function. Requires leatherworking skill.

**repairWood, repairRock, repairCrystal, repairMagical, repairSpiritual, repairGeneric:** Category-specific repair functions with corresponding skill requirements.

### Database Schema

**obj.material (int):** Material type constant stored per object. Indexed for performance. Not nullable, defaults to MAT_UNDEFINED.

**obj.max_struct (int):** Maximum structure points representing pristine condition durability. Set at creation based on material hardness and crafting quality.

**obj.cur_struct (int):** Current structure points tracking damage accumulation. Decreases from combat, environmental damage, and wear. Object breaks at 0.

## Implementation

The material system centers on the material_nums global array defined in constants.cc containing 200 material_type_numbers structures. Each structure stores 14 properties controlling physical behavior and economic value. Material constants from materials.h serve as array indices, enabling fast O(1) property lookup without hash tables or searches.

Objects store a single material type integer in their thing.h material_type member. The getMaterial method returns this value for use as a material_nums index. The setMaterial method validates and assigns the material type. The getMaterialTypeNumbers method returns a const pointer to the corresponding material_type_numbers structure for direct multi-property access.

Combat damage calculations in obj_base_weapon.cc retrieve hardness through material_nums[getMaterial()].hardness. The getWeaponDamageBonus function multiplies base weapon damage by hardness divided by 100, creating a percentage scaling factor. Iron baseline hardness of 60 produces 60% effectiveness, while diamond hardness of 100 produces 100% effectiveness. Lower hardness values proportionally reduce damage output.

Armor protection follows identical hardness scaling in armor calculation functions. The material hardness determines what percentage of theoretical maximum armor class the item provides. Leather with hardness 25 provides only 25% effectiveness compared to steel with hardness 70 providing 70% effectiveness for identical armor types.

Durability degradation consumes structure points based on susceptibility properties matched to damage types. Slash damage queries cut_susc, blunt damage queries smash_susc, fire damage queries burned_susc, pierce damage queries pierced_susc. Higher susceptibility values cause greater structure point loss per damage instance. The formula divides incoming damage by hardness after applying susceptibility multiplier, so hard materials with low susceptibility lose minimal structure while soft materials with high susceptibility degrade rapidly.

Water damage checks water_susc values. When exceeding 100, objects in water or wet environments trigger periodic structure loss based on how far above 100 the value extends. Iron at 101 rusts slowly, while organic materials at 150+ rot quickly. Materials with water_susc at or below 100 remain unaffected by moisture.

Fire mechanics evaluate flammability before ignition attempts. Zero flammability prevents burning entirely. Non-zero values enable ignition with probability proportional to flammability rating. Once burning, structure points decrease each tick at rates determined by flammability value. Paper at 1000 burns rapidly, wood at 500 burns moderately, leather at 400 burns slowly.

The crafting system maintains parallel material arrays for each category in materials.h. The MetalMaterial, HideMaterial, WoodMaterial, and similar structures extend basic material_type_numbers with crafting-specific fields. These structures store difficulty modifiers, structure bonuses, level requirements, stat modifications, and raw material vnums. The findMetalMaterial and category-specific lookup functions search these arrays by material number or name string.

Crafting calculations add material difficultyMod to base skill check thresholds. Higher difficulty modifiers require greater skill for success. The structureMod adds to finished item max_struct values, making items from superior materials more durable. The levelMod enforces minimum crafter levels by adding to base requirements. The statMod applies magical bonuses to finished items when using enchanted materials.

Economic pricing multiplies object base value by material price property. Shop keeper functions in obj_armor_wand.cc perform this multiplication then apply shop-specific markup or discount percentages. Character charisma provides additional price modification during purchase transactions. Final prices scale exponentially with material rarity, ensuring legendary materials cost orders of magnitude more than common materials.

Repair functions dispatch through repair_proc function pointers stored in material_type_numbers. Each material category links to its specialized repair function. The repair system invokes the stored function pointer passing the repairing character and damaged object. Repair functions check appropriate skill levels, consume repair materials, and restore structure points based on material properties and damage extent.

Weight calculations in thing.cc incorporate vol_mult from material properties. The getCarriedWeight function sums contained object weights, but inventory space calculations multiply object volume by material vol_mult. Dense materials with higher vol_mult consume more inventory space despite identical physical size. This creates realistic carry capacity constraints where a small gold bar occupies more space than a large wooden plank.

Buoyancy mechanics query float_weight to determine water behavior. Objects with float_weight 0 sink immediately to the bottom. Values from 1-100 cause slow sinking. Values 101-200 maintain neutral buoyancy. Values above 200 float with increasing buoyancy. This simulates realistic material density effects in water.

Conductivity affects lightning damage transmission through equipped items. Metal armor with conductivity 1 amplifies incoming lightning damage and transmits shocks between connected equipment. Insulating materials with conductivity 0 block transmission and reduce damage. The combat system checks material conductivity when resolving electrical attacks.

Noise generation queries the noise property during stealth checks. Negative noise values provide stealth bonuses by creating silent movement. Positive values penalize stealth by generating sound. Metal armor with noise 27 significantly impairs sneaking while soft leather with noise 5 minimally affects it. The position system multiplies noise by movement speed for total stealth impact.

The obj database table stores material as an integer column. Object loading queries this column and calls setMaterial with the retrieved value. Object saving writes the current material type back to the database. This persistence ensures material properties survive server restarts and object transfers.

Default material assignment occurs during object creation for undefined materials. Builder tools enforce material selection when creating new object types. Common defaults follow logical patterns: weapons default to steel or iron, armor defaults to leather or metal based on type, bags default to cloth or leather, wooden items default to wood, corpses default to organic flesh types.

Material validation prevents invalid assignments. The setMaterial function clamps material type values to valid ranges, defaulting to MAT_UNDEFINED for out-of-bounds values. Code defensively checks getMaterial results against zero before array indexing to catch undefined materials. This prevents crashes from invalid array access.

Special elemental materials override standard property behavior. MAT_WATER with water_susc 249 maximizes water affinity rather than vulnerability. MAT_FIRE with burned_susc 0 grants fire immunity to living fire. MAT_LIGHTNING with conductivity 1 represents pure electrical energy. These materials enable magical creature types with non-standard physics.

Organic vs inorganic classification determines decay behavior. Materials 50-77 (nature range) undergo organic decay processes including rot, fungal growth, and consumption by creatures. Materials 100-177 (mineral and metal ranges) experience inorganic degradation through rust, corrosion, and oxidation. General materials 0-19 split between organic (paper, cloth, wood) and inorganic (glass, plastic, metal) based on their source.

The material tier system enables progressive crafting advancement. Players begin working with COMMON tier materials like copper and leather. Skill improvements unlock UNCOMMON tier access to iron and steel. Advanced crafters gain RARE tier privileges for mithril and dragon scale. Master crafters achieve LEGENDARY tier capability with adamantite and diamond. This creates clear progression paths and achievement goals.

Material category ranges leave intentional gaps for expansion. Range 20-49 remains unused between general and nature materials. Range 78-99 remains unused between nature and mineral materials. Range 127-149 remains unused between mineral and metal materials. Range 178-199 remains unused after metal materials. This provides 117 open slots for future material additions without restructuring existing systems.

## Troubleshooting

**Objects have zero damage despite correct weapon type:** Check material assignment first. Objects defaulting to MAT_UNDEFINED have hardness 0, producing no damage output. Verify getMaterial returns a valid material constant, not 0. Assign appropriate material using setMaterial with constants like MAT_STEEL or MAT_IRON.

**Armor provides no protection from any attack type:** Examine material hardness value in material_nums array. Hardness below 20 provides negligible armor effectiveness. Verify susceptibility values are not 0 across all damage types. Materials with zero susceptibility prevent all damage, creating invulnerable armor. Balance susceptibility values between 10-50 for most materials.

**Metal objects ignite and burn down:** Check flammability property of the material. Metals should have flammability 0. If non-zero, the material definition in constants.cc is incorrect. Correct the entry and rebuild. Remember flammability represents ignition probability and burn rate, not just combustion capability.

**Wooden or cloth items cannot catch fire:** Verify flammability property is non-zero. Paper should have 1000, cloth 900, wood 500. Zero flammability completely prevents ignition regardless of fire exposure. Check setBurning calls properly query flammability before ignition attempts.

**Objects float when they should sink or vice versa:** Examine float_weight property. Metals should have 0 (sinks immediately). Wood typically 150-200 (floats). Mistaken float_weight assignments cause unrealistic buoyancy. Heavy materials like iron with float_weight above 100 appear to float on water.

**Weapons degrade instantly during combat:** Check hardness and susceptibility balance. Very high susceptibility values (80-100) combined with low hardness (20-30) cause extreme degradation. Weapons should have hardness 40-100 with susceptibility values below 50. The degradation formula divides damage by hardness, so low hardness amplifies structure loss.

**Objects never rust despite being metal:** Verify water_susc exceeds 100. Values at or below 100 prevent rust mechanics. Iron and steel should have 101. Legendary metals like mithril and adamantite have 0 (rust-proof). Objects must also be exposed to water or damp environments for rust to trigger.

**Crafting fails repeatedly with sufficient skill:** Check material difficultyMod value. Extremely high modifiers (+40 or more) can make crafting impossible even at maximum skill. Verify levelMod does not exceed character level. The effective difficulty is base difficulty plus difficultyMod, which may exceed skill even at 100%.

**Finished crafted items have worse stats than expected:** Examine structureMod and statMod in the crafting material structure. Negative structureMod reduces durability. Zero statMod provides no bonuses. Ensure the correct material tier is selected - COMMON tier materials provide minimal bonuses while LEGENDARY tier provides maximum bonuses.

**Shop prices are incorrect for material value:** Check price property in material_nums. Verify the multiplication occurs: final_price = base_value * material_price. Shop-specific markup should apply after material pricing. Charisma modifiers apply last. If price is still wrong, verify material assignment matches the object's actual composition.

**Repair attempts fail to find appropriate craftsperson:** Check repair_proc function pointer assignment in material_nums. Ensure the correct repair function is linked: repairMetal for metals, repairHide for leather, etc. Verify the craftsperson NPC has the matching skill type and sufficient skill level.

**Lightning damage does not conduct through metal armor:** Verify conductivity is set to 1 for metal materials. Check combat system properly queries conductivity during electrical damage resolution. Ensure equipped items are checked, not just carried items. Conductivity only affects equipped armor and weapons.

**Heavy objects take minimal inventory space:** Check vol_mult property. Most materials should use 1 (standard). Dense materials like gold or lead should use 2-3. Objects appearing lighter than expected have vol_mult set too low. Multiply vol_mult value to increase inventory space consumption.

**Stealth fails despite wearing soft materials:** Examine noise property values. Positive values penalize stealth. Metal armor typically has noise 20-30, causing major penalties. Leather should have noise 5-10. Cloth near 0. Sum total noise across all equipped items to determine total stealth impact.

**Materials missing from crafting menus:** Verify material exists in category-specific crafting arrays (metals[], hides[], woods[], etc.) in materials.h. Missing entries cause lookup functions to return nullptr. Add material entry to appropriate array with correct tier and modifiers.

**Undefined material crash during property lookup:** Check getMaterial return value before using as array index. MAT_UNDEFINED (0) has valid properties but indicates missing assignment. Defensive code should detect 0 and assign appropriate default material. Never use getMaterial result as index without bounds checking.

**Fire damage kills objects instantly:** Check burned_susc and structure points. Materials with burned_susc 100 take maximum fire damage. Low max_struct values cause rapid destruction. Flammability determines ignition ease, burned_susc determines ongoing damage rate. Balance both properties to achieve desired burn behavior.

**Acid attacks deal no damage:** Verify acid_susc is non-zero. Check acid damage code queries acid_susc property. Materials with acid_susc 0 are acid-proof. Most materials should have 20-80. Metals typically 50-70, stone 20-40, organic materials 60-90.

**Custom material causes assertion failures:** Validate all property values are within documented ranges. Susceptibility values must be 0-100, not 0-255. Hardness must be 0-100. Float_weight must be 0-255. Vol_mult must be 1-8. Noise must be -5 to +10. Out-of-range values trigger assertions or produce undefined behavior.

**Material tier restrictions not enforced:** Check tier enum assignment in crafting material structure. Verify levelMod is positive for higher tier materials. Ensure crafting code checks crafter level against base requirement plus material levelMod. Missing checks allow premature access to legendary materials.

**Repair costs exceed material value:** Check price property and repair function implementation. Repair costs typically scale with max_struct relative to cur_struct. Materials with very high price values produce expensive repairs. Legendary materials naturally cost more to repair than replace common items.

