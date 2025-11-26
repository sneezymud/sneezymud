//////////////////////////////////////////////////////////////////////////
//
// SneezyMUD - All rights reserved, SneezyMUD Coding Team
//
//////////////////////////////////////////////////////////////////////////

#pragma once

#include "structs.h"

const int MAT_UNDEFINED = 0;
const int MAT_PAPER = 1;
const int MAT_CLOTH = 2;
const int MAT_WAX = 3;
const int MAT_GLASS = 4;
const int MAT_WOOD = 5;
const int MAT_SILK = 6;
const int MAT_FOODSTUFF = 7;
const int MAT_PLASTIC = 8;
const int MAT_RUBBER = 9;
const int MAT_CARDBOARD = 10;
const int MAT_STRING = 11;
const int MAT_PLASMA = 12;
const int MAT_TOUGH_CLOTH = 13;
const int MAT_CORAL = 14;
const int MAT_HORSEHAIR = 15;
const int MAT_HAIR = 16;
const int MAT_POWDER = 17;
const int MAT_PUMICE = 18;
const int MAT_LAMINATED = 19;

const int MAX_MAT_GENERAL = 20;  // Move and change

const int MAT_GEN_ORG = 50;
const int MAT_LEATHER = 51;
const int MAT_TOUGH_LEATHER = 52;
const int MAT_DRAGON_SCALE = 53;
const int MAT_WOOL = 54;
const int MAT_FUR = 55;
const int MAT_FEATHERED = 56;
const int MAT_WATER = 57;
const int MAT_FIRE = 58;
const int MAT_EARTH = 59;
const int MAT_ELEMENTAL = 60;
const int MAT_ICE = 61;
const int MAT_LIGHTNING = 62;
const int MAT_CHAOS = 63;
const int MAT_CLAY = 64;
const int MAT_PORCELAIN = 65;
const int MAT_STRAW = 66;
const int MAT_PEARL = 67;
const int MAT_HUMAN_FLESH = 68;
const int MAT_FUR_CAT = 69;
const int MAT_FUR_DOG = 70;
const int MAT_FUR_RABBIT = 71;
const int MAT_GHOSTLY = 72;
const int MAT_DWARF_LEATHER = 73;
const int MAT_SOFT_LEATHER = 74;
const int MAT_FISHSCALE = 75;
const int MAT_OGRE_HIDE = 76;
const int MAT_HEMP = 77;

const int MAX_MAT_NATURE = 28;  // Move and change  (max+1 - 50)

const int MAT_GEN_MINERAL = 100;
// const int    MAT_JEWELED         =101;
const int MAT_RUNESTONE = 102;
const int MAT_CRYSTAL = 103;
const int MAT_DIAMOND = 104;
const int MAT_EBONY = 105;
const int MAT_EMERALD = 106;
const int MAT_IVORY = 107;
const int MAT_OBSIDIAN = 108;
const int MAT_ONYX = 109;
const int MAT_OPAL = 110;
const int MAT_RUBY = 111;
const int MAT_SAPPHIRE = 112;
const int MAT_MARBLE = 113;
const int MAT_STONE = 114;
const int MAT_BONE = 115;
const int MAT_JADE = 116;
const int MAT_AMBER = 117;
const int MAT_TURQUOISE = 118;
const int MAT_AMETHYST = 119;
const int MAT_MICA = 120;
const int MAT_DRAGONBONE = 121;
const int MAT_MALACHITE = 122;
const int MAT_GRANITE = 123;
const int MAT_QUARTZ = 124;
const int MAT_JET = 125;
const int MAT_CORUNDUM = 126;
const int MAX_MAT_MINERAL = 27;  // Move and change  (max+1 - 100)

// const int    MAT_GEN_METAL       =150;
const int MAT_COPPER = 151;
// const int    MAT_SCALE_MAIL      =152;
// const int    MAT_BANDED_MAIL     =153;
// const int    MAT_CHAIN_MAIL      =154;
// const int    MAT_PLATE           =155;
const int MAT_BRONZE = 156;
const int MAT_BRASS = 157;
const int MAT_IRON = 158;
const int MAT_STEEL = 159;
const int MAT_MITHRIL = 160;
const int MAT_ADAMANTITE = 161;
const int MAT_SILVER = 162;
const int MAT_GOLD = 163;
const int MAT_PLATINUM = 164;
const int MAT_TITANIUM = 165;
const int MAT_ALUMINUM = 166;
// const int    MAT_RINGMAIL        =167;
 const int    MAT_GNICKEL       =168;
const int MAT_ELECTRUM = 169;
const int MAT_ATHANOR = 170;
const int MAT_TIN = 171;
const int MAT_TUNGSTEN = 172;
const int MAT_STARMETAL = 173;
const int MAT_TERBIUM = 174;
// const int    MAT_ELVENMAIL       =175;
// const int    MAT_ELVENSTEEL      =176;
const int MAT_ETERNIUM = 177;

const int MAX_MAT_METAL = 28;  // Move and change  (max+1 - 150)

struct material_type_numbers {
    short cut_susc;
    short smash_susc;
    short burned_susc;
    short pierced_susc;
    short hardness;
    unsigned short water_susc;
    unsigned short fall_susc;
    unsigned short float_weight;
    short noise;
    unsigned short vol_mult;
    unsigned short conductivity;
    int flammability;
    unsigned short acid_susc;
    float price;
    int (*repair_proc)(TBeing*, TObj* o);
    char mat_name[20];
};

extern const struct material_type_numbers material_nums[200];

// Material tier categories (rarity-based, organized by repair type)
enum MetalTierT {
  METAL_TIER_COMMON,
  METAL_TIER_UNCOMMON,
  METAL_TIER_RARE,
  METAL_TIER_LEGENDARY,
};

enum HideTierT {
  HIDE_TIER_COMMON,
  HIDE_TIER_UNCOMMON,
  HIDE_TIER_RARE,
  HIDE_TIER_LEGENDARY,
};

enum WoodTierT {
  WOOD_TIER_COMMON,
  WOOD_TIER_UNCOMMON,
  WOOD_TIER_RARE,
  WOOD_TIER_LEGENDARY,
};

enum OrganicTierT {
  ORGANIC_TIER_COMMON,
  ORGANIC_TIER_UNCOMMON,
  ORGANIC_TIER_RARE,
  ORGANIC_TIER_LEGENDARY,
};

enum DeadTierT {
  DEAD_TIER_COMMON,
  DEAD_TIER_UNCOMMON,
  DEAD_TIER_RARE,
  DEAD_TIER_LEGENDARY,
};

enum RockTierT {
  ROCK_TIER_COMMON,
  ROCK_TIER_UNCOMMON,
  ROCK_TIER_RARE,
  ROCK_TIER_LEGENDARY,
};

enum CrystalTierT {
  CRYSTAL_TIER_COMMON,
  CRYSTAL_TIER_UNCOMMON,
  CRYSTAL_TIER_RARE,
  CRYSTAL_TIER_LEGENDARY,
};

enum MagicalTierT {
  MAGICAL_TIER_COMMON,
  MAGICAL_TIER_UNCOMMON,
  MAGICAL_TIER_RARE,
  MAGICAL_TIER_LEGENDARY,
};

enum SpiritualTierT {
  SPIRITUAL_TIER_COMMON,
  SPIRITUAL_TIER_UNCOMMON,
  SPIRITUAL_TIER_RARE,
  SPIRITUAL_TIER_LEGENDARY,
};

enum GenericTierT {
  GENERIC_TIER_COMMON,
  GENERIC_TIER_UNCOMMON,
  GENERIC_TIER_RARE,
  GENERIC_TIER_LEGENDARY,
};

// Material properties for crafting (organized by repair type)

// Metal material properties (repairMetal - SKILL_BLACKSMITHING)
struct MetalMaterial {
  int matNum;           // Material number (MAT_COPPER, MAT_IRON, etc.)
  const char* name;     // Display name
  MetalTierT tier;      // Material tier category
  int difficultyMod;    // Subtracted from skill check (higher = harder)
  int structureMod;     // Affects billet durability/structure points
  int levelMod;         // Affects required character level
  int statMod;          // Affects stat bonuses on finished items
  int sharpnessMod;     // Affects sharpness of finished items
  int matReq;           // Material requirement (vnum of raw material)
};

struct HideMaterial {
  int matNum;           // Material number
  const char* name;     // Display name
  HideTierT tier;       // Material tier category
  int difficultyMod;    // Subtracted from skill check (higher = harder)
  int structureMod;     // Affects item durability/structure points
  int levelMod;         // Affects required character level
  int statMod;          // Affects stat bonuses on finished items
  int matReq;           // Material requirement (vnum of raw material)
};

struct WoodMaterial {
  int matNum;           // Material number
  const char* name;     // Display name
  WoodTierT tier;       // Material tier category
  int difficultyMod;    // Subtracted from skill check (higher = harder)
  int structureMod;     // Affects item durability/structure points
  int levelMod;         // Affects required character level
  int statMod;          // Affects stat bonuses on finished items
  int matReq;           // Material requirement (vnum of raw material)
};

struct OrganicMaterial {
  int matNum;           // Material number
  const char* name;     // Display name
  OrganicTierT tier;    // Material tier category
  int difficultyMod;    // Subtracted from skill check (higher = harder)
  int structureMod;     // Affects item durability/structure points
  int levelMod;         // Affects required character level
  int statMod;          // Affects stat bonuses on finished items
  int matReq;           // Material requirement (vnum of raw material)
};

struct DeadMaterial {
  int matNum;           // Material number
  const char* name;     // Display name
  DeadTierT tier;       // Material tier category
  int difficultyMod;    // Subtracted from skill check (higher = harder)
  int structureMod;     // Affects item durability/structure points
  int levelMod;         // Affects required character level
  int statMod;          // Affects stat bonuses on finished items
  int matReq;           // Material requirement (vnum of raw material)
};

struct RockMaterial {
  int matNum;           // Material number
  const char* name;     // Display name
  RockTierT tier;       // Material tier category
  int difficultyMod;    // Subtracted from skill check (higher = harder)
  int structureMod;     // Affects item durability/structure points
  int levelMod;         // Affects required character level
  int statMod;          // Affects stat bonuses on finished items
  int matReq;           // Material requirement (vnum of raw material)
};

struct CrystalMaterial {
  int matNum;           // Material number
  const char* name;     // Display name
  CrystalTierT tier;    // Material tier category
  int difficultyMod;    // Subtracted from skill check (higher = harder)
  int structureMod;     // Affects item durability/structure points
  int levelMod;         // Affects required character level
  int statMod;          // Affects stat bonuses on finished items
  int matReq;           // Material requirement (vnum of raw material)
};

struct MagicalMaterial {
  int matNum;           // Material number
  const char* name;     // Display name
  MagicalTierT tier;    // Material tier category
  int difficultyMod;    // Subtracted from skill check (higher = harder)
  int structureMod;     // Affects item durability/structure points
  int levelMod;         // Affects required character level
  int statMod;          // Affects stat bonuses on finished items
  int matReq;           // Material requirement (vnum of raw material)
};

struct SpiritualMaterial {
  int matNum;           // Material number
  const char* name;     // Display name
  SpiritualTierT tier;  // Material tier category
  int difficultyMod;    // Subtracted from skill check (higher = harder)
  int structureMod;     // Affects item durability/structure points
  int levelMod;         // Affects required character level
  int statMod;          // Affects stat bonuses on finished items
  int matReq;           // Material requirement (vnum of raw material)
};

struct GenericMaterial {
  int matNum;           // Material number
  const char* name;     // Display name
  GenericTierT tier;    // Material tier category
  int difficultyMod;    // Subtracted from skill check (higher = harder)
  int structureMod;     // Affects item durability/structure points
  int levelMod;         // Affects required character level
  int statMod;          // Affects stat bonuses on finished items
  int matReq;           // Material requirement (vnum of raw material)
};

// Metal materials table - ordered by tier
static constexpr MetalMaterial metalMaterials[] = {
  // COMMON tier
  {MAT_TIN, "tin", METAL_TIER_COMMON, -10, 40, 1, -1, -1, 0},
  {MAT_BRONZE, "bronze", METAL_TIER_COMMON, -10, 55, 3, 1, 1, 0},
  {MAT_COPPER, "copper", METAL_TIER_COMMON, -10, 50, 1, 0, 0, 0},
  {MAT_BRASS, "brass", METAL_TIER_COMMON, -10, 45, 2, 0, 0, 0},
  {MAT_ALUMINUM, "aluminum", METAL_TIER_COMMON, -10, 60, 4, 0, 1, 0},

  // UNCOMMON tier
  {MAT_GOLD, "gold", METAL_TIER_UNCOMMON, 10, 70, 12, 3, 1, 0},
  {MAT_SILVER, "silver", METAL_TIER_UNCOMMON, 10, 75, 8, 1, 2, 0},
  {MAT_PLATINUM, "platinum", METAL_TIER_UNCOMMON, 10, 90, 18, 3, 3, 0},
  {MAT_ELECTRUM, "electrum", METAL_TIER_UNCOMMON, 10, 80, 9, 2, 2, 0},

  // RARE tier
  {MAT_IRON, "iron", METAL_TIER_RARE, 25, 70, 5, 1, 2, 0},
  {MAT_STEEL, "steel", METAL_TIER_RARE, 25, 85, 10, 2, 3, 0},
  {MAT_TUNGSTEN, "tungsten", METAL_TIER_RARE, 25, 95, 15, 2, 4, 0},
  {MAT_TITANIUM, "titanium", METAL_TIER_RARE, 25, 105, 28, 4, 5, 0},

  // LEGENDARY tier
  {MAT_ETERNIUM, "eternium", METAL_TIER_LEGENDARY, 40, 130, 50, 7, 8, 0},
  {MAT_MITHRIL, "mithril", METAL_TIER_LEGENDARY, 40, 100, 25, 4, 5, 0},
  {MAT_ADAMANTITE, "adamantite", METAL_TIER_LEGENDARY, 40, 110, 30, 5, 6, 0},
  {MAT_ATHANOR, "athanor", METAL_TIER_LEGENDARY, 40, 120, 40, 6, 7, 0},
  {MAT_STARMETAL, "starmetal", METAL_TIER_LEGENDARY, 40, 115, 35, 5, 6, 0},
};

static constexpr int NUM_METAL_MATERIALS = sizeof(metalMaterials) / sizeof(MetalMaterial);

// Hide materials table (repairHide - SKILL_REPAIR_MONK)
static constexpr HideMaterial hideMaterials[] = {
  // COMMON tier
  {MAT_CLOTH, "cloth", HIDE_TIER_COMMON, -10, 40, 1, 0, 0},
  {MAT_FUR_CAT, "cat fur", HIDE_TIER_COMMON, -10, 45, 2, 0, 0},
  {MAT_FUR_DOG, "dog fur", HIDE_TIER_COMMON, -10, 45, 2, 0, 0},
  {MAT_SOFT_LEATHER, "soft leather", HIDE_TIER_COMMON, -10, 52, 2, 0, 0},
  {MAT_WOOL, "wool", HIDE_TIER_COMMON, -10, 45, 1, 0, 0},
  {MAT_FUR, "fur", HIDE_TIER_COMMON, -10, 45, 2, 0, 0},
  {MAT_HORSEHAIR, "horsehair", HIDE_TIER_COMMON, -10, 40, 1, 0, 0},

  // UNCOMMON tier
  {MAT_HAIR, "hair", HIDE_TIER_UNCOMMON, 10, 40, 1, 0, 0},
  {MAT_TOUGH_CLOTH, "toughened cloth", HIDE_TIER_UNCOMMON, 10, 48, 2, 0, 0},
  {MAT_FUR_RABBIT, "rabbit fur", HIDE_TIER_UNCOMMON, 10, 45, 2, 0, 0},
  {MAT_DWARF_LEATHER, "dwarven leather", HIDE_TIER_UNCOMMON, 10, 52, 2, 0, 0},
  {MAT_LEATHER, "leather", HIDE_TIER_UNCOMMON, 10, 50, 2, 0, 0},

  // RARE tier
  {MAT_TOUGH_LEATHER, "toughened leather", HIDE_TIER_RARE, 25, 55, 3, 0, 0},
  {MAT_HEMP, "hemp", HIDE_TIER_RARE, 25, 48, 2, 0, 0},
  {MAT_RUBBER, "rubber", HIDE_TIER_RARE, 25, 42, 1, 0, 0},

  // LEGENDARY tier
  {MAT_SILK, "silk", HIDE_TIER_LEGENDARY, 40, 45, 2, 0, 0},
  {MAT_LAMINATED, "laminate", HIDE_TIER_LEGENDARY, 40, 50, 3, 0, 0},
};

static constexpr int NUM_HIDE_MATERIALS = sizeof(hideMaterials) / sizeof(HideMaterial);

// Wood materials table (repairWood - SKILL_REPAIR_MONK)
static constexpr WoodMaterial woodMaterials[] = {
  // COMMON tier
  {MAT_WOOD, "wood", WOOD_TIER_COMMON, -10, 60, 1, 0, 0},

  // RARE tier
  {MAT_EBONY, "ebony", WOOD_TIER_RARE, 25, 85, 15, 3, 0},
};

static constexpr int NUM_WOOD_MATERIALS = sizeof(woodMaterials) / sizeof(WoodMaterial);

// Organic materials table (repairOrganic - SKILL_REPAIR_MONK)
static constexpr OrganicMaterial organicMaterials[] = {
  // UNCOMMON tier
  {MAT_CORAL, "coral", ORGANIC_TIER_UNCOMMON, 10, 65, 5, 1, 0},
  {MAT_FISHSCALE, "fishscale", ORGANIC_TIER_UNCOMMON, 10, 70, 7, 1, 0},

  // LEGENDARY tier
  {MAT_DRAGON_SCALE, "dragon scale", ORGANIC_TIER_LEGENDARY, 40, 110, 30, 5, 0},
};

static constexpr int NUM_ORGANIC_MATERIALS = sizeof(organicMaterials) / sizeof(OrganicMaterial);

// Dead materials table (repairDead - SKILL_REPAIR_SHAMAN)
static constexpr DeadMaterial deadMaterials[] = {
  // COMMON tier
  {MAT_POWDER, "ash", DEAD_TIER_COMMON, -10, 35, 1, 0, 0},
  {MAT_HUMAN_FLESH, "flesh", DEAD_TIER_COMMON, -10, 40, 2, 0, 0},
  {MAT_BONE, "bone", DEAD_TIER_COMMON, -10, 50, 3, 0, 0},

  // UNCOMMON tier
  {MAT_OGRE_HIDE, "ogre hide", DEAD_TIER_UNCOMMON, 10, 70, 7, 1, 0},

  // RARE tier
  {MAT_IVORY, "ivory", DEAD_TIER_RARE, 25, 85, 15, 3, 0},

  // LEGENDARY tier
  {MAT_DRAGONBONE, "dragonbone", DEAD_TIER_LEGENDARY, 40, 120, 40, 6, 0},
};

static constexpr int NUM_DEAD_MATERIALS = sizeof(deadMaterials) / sizeof(DeadMaterial);

// Rock materials table (repairRock - SKILL_REPAIR_MAGE or SKILL_REPAIR_MONK)
static constexpr RockMaterial rockMaterials[] = {
  // COMMON tier
  {MAT_PUMICE, "pumice", ROCK_TIER_COMMON, -10, 35, 1, 0, 0},
  {MAT_STONE, "stone", ROCK_TIER_COMMON, -10, 50, 2, 0, 0},
  {MAT_GRANITE, "granite", ROCK_TIER_COMMON, -10, 55, 2, 0, 0},

  // UNCOMMON tier
  {MAT_MARBLE, "marble", ROCK_TIER_UNCOMMON, 10, 60, 3, 0, 0},
  {MAT_AMBER, "amber", ROCK_TIER_UNCOMMON, 10, 65, 5, 1, 0},
  {MAT_JADE, "jade", ROCK_TIER_UNCOMMON, 10, 75, 8, 2, 0},

  // RARE tier
  {MAT_PEARL, "pearl", ROCK_TIER_RARE, 25, 90, 18, 3, 0},
  {MAT_TURQUOISE, "turquoise", ROCK_TIER_RARE, 25, 65, 5, 1, 0},

  // LEGENDARY tier
  {MAT_OBSIDIAN, "obsidian", ROCK_TIER_LEGENDARY, 40, 70, 7, 1, 0},
  {MAT_MALACHITE, "malachite", ROCK_TIER_LEGENDARY, 40, 68, 6, 1, 0},
};

static constexpr int NUM_ROCK_MATERIALS = sizeof(rockMaterials) / sizeof(RockMaterial);

// Crystal materials table (repairCrystal - SKILL_REPAIR_THIEF or SKILL_BLACKSMITHING_ADVANCED)
static constexpr CrystalMaterial crystalMaterials[] = {
  // COMMON tier
  {MAT_MICA, "mica", CRYSTAL_TIER_COMMON, -10, 60, 4, 1, 0},
  {MAT_QUARTZ, "quartz", CRYSTAL_TIER_COMMON, -10, 65, 5, 1, 0},
  {MAT_CRYSTAL, "crystal", CRYSTAL_TIER_COMMON, -10, 70, 7, 1, 0},

  // UNCOMMON tier
  {MAT_ONYX, "onyx", CRYSTAL_TIER_UNCOMMON, 10, 70, 7, 1, 0},
  {MAT_OPAL, "opal", CRYSTAL_TIER_UNCOMMON, 10, 72, 7, 2, 0},

  // RARE tier
  {MAT_AMETHYST, "amethyst", CRYSTAL_TIER_RARE, 25, 68, 6, 1, 0},
  {MAT_CORUNDUM, "corundum", CRYSTAL_TIER_RARE, 25, 100, 20, 4, 0},

  // LEGENDARY tier
  {MAT_DIAMOND, "diamond", CRYSTAL_TIER_LEGENDARY, 40, 120, 40, 6, 0},
  {MAT_RUBY, "ruby", CRYSTAL_TIER_LEGENDARY, 40, 115, 35, 5, 0},
  {MAT_EMERALD, "emerald", CRYSTAL_TIER_LEGENDARY, 40, 115, 35, 5, 0},
  {MAT_SAPPHIRE, "sapphire", CRYSTAL_TIER_LEGENDARY, 40, 115, 35, 5, 0},
};

static constexpr int NUM_CRYSTAL_MATERIALS = sizeof(crystalMaterials) / sizeof(CrystalMaterial);

// Magical materials table (repairMagical - SKILL_REPAIR_MAGE)
static constexpr MagicalMaterial magicalMaterials[] = {
  // COMMON tier
  {MAT_WATER, "liquid", MAGICAL_TIER_COMMON, -10, 45, 2, 0, 0},
  {MAT_ICE, "ice", MAGICAL_TIER_COMMON, -10, 55, 3, 1, 0},
  {MAT_ELEMENTAL, "generic elemental", MAGICAL_TIER_COMMON, -10, 58, 4, 1, 0},

  // UNCOMMON tier
  {MAT_FIRE, "fire", MAGICAL_TIER_UNCOMMON, 10, 50, 2, 0, 0},
  {MAT_EARTH, "earth", MAGICAL_TIER_UNCOMMON, 10, 48, 2, 0, 0},

  // RARE tier
  {MAT_CHAOS, "chaos", MAGICAL_TIER_RARE, 25, 65, 6, 1, 0},
  {MAT_PLASMA, "plasma", MAGICAL_TIER_RARE, 25, 55, 3, 1, 0},

  // LEGENDARY tier
  {MAT_LIGHTNING, "lightning", MAGICAL_TIER_LEGENDARY, 40, 60, 5, 1, 0},
  {MAT_RUNESTONE, "runestone", MAGICAL_TIER_LEGENDARY, 40, 110, 30, 5, 0},
};

static constexpr int NUM_MAGICAL_MATERIALS = sizeof(magicalMaterials) / sizeof(MagicalMaterial);

// Spiritual materials table (repairSpiritual - SKILL_REPAIR_CLERIC or SKILL_REPAIR_DEIKHAN)
static constexpr SpiritualMaterial spiritualMaterials[] = {
  // COMMON tier
  {MAT_FOODSTUFF, "foodstuff", SPIRITUAL_TIER_COMMON, -10, 40, 1, 0, 0},

  // LEGENDARY tier
  {MAT_GHOSTLY, "ghostly", SPIRITUAL_TIER_LEGENDARY, 40, 100, 25, 4, 0},
};

static constexpr int NUM_SPIRITUAL_MATERIALS = sizeof(spiritualMaterials) / sizeof(SpiritualMaterial);

// Generic materials table (repairGeneric - Anyone with SKILL_MEND)
static constexpr GenericMaterial genericMaterials[] = {
  // COMMON tier
  {MAT_PAPER, "paper", GENERIC_TIER_COMMON, -10, 30, 1, 0, 0},
  {MAT_WAX, "wax", GENERIC_TIER_COMMON, -10, 35, 1, 0, 0},
  {MAT_PLASTIC, "plastic", GENERIC_TIER_COMMON, -10, 40, 2, 0, 0},
  {MAT_CARDBOARD, "cardboard", GENERIC_TIER_COMMON, -10, 35, 1, 0, 0},
  {MAT_STRING, "string", GENERIC_TIER_COMMON, -10, 35, 1, 0, 0},
  {MAT_CLAY, "clay", GENERIC_TIER_COMMON, -10, 38, 1, 0, 0},
  {MAT_PORCELAIN, "porcelain", GENERIC_TIER_COMMON, -10, 50, 2, 0, 0},
  {MAT_STRAW, "straw", GENERIC_TIER_COMMON, -10, 35, 1, 0, 0},
};

static constexpr int NUM_GENERIC_MATERIALS = sizeof(genericMaterials) / sizeof(GenericMaterial);

// Helper functions to find materials by material number or name

// Metal material helpers
inline const MetalMaterial* findMetalMaterial(int matNum) {
  for (int i = 0; i < NUM_METAL_MATERIALS; ++i) {
    if (metalMaterials[i].matNum == matNum) {
      return &metalMaterials[i];
    }
  }
  return nullptr;
}

inline const MetalMaterial* findMetalMaterialByName(const char* name) {
  if (!name) return nullptr;
  for (int i = 0; i < NUM_METAL_MATERIALS; ++i) {
    if (!strcasecmp(metalMaterials[i].name, name)) {
      return &metalMaterials[i];
    }
  }
  return nullptr;
}

// Hide material helpers
inline const HideMaterial* findHideMaterial(int matNum) {
  for (int i = 0; i < NUM_HIDE_MATERIALS; ++i) {
    if (hideMaterials[i].matNum == matNum) {
      return &hideMaterials[i];
    }
  }
  return nullptr;
}

inline const HideMaterial* findHideMaterialByName(const char* name) {
  if (!name) return nullptr;
  for (int i = 0; i < NUM_HIDE_MATERIALS; ++i) {
    if (!strcasecmp(hideMaterials[i].name, name)) {
      return &hideMaterials[i];
    }
  }
  return nullptr;
}

// Wood material helpers
inline const WoodMaterial* findWoodMaterial(int matNum) {
  for (int i = 0; i < NUM_WOOD_MATERIALS; ++i) {
    if (woodMaterials[i].matNum == matNum) {
      return &woodMaterials[i];
    }
  }
  return nullptr;
}

inline const WoodMaterial* findWoodMaterialByName(const char* name) {
  if (!name) return nullptr;
  for (int i = 0; i < NUM_WOOD_MATERIALS; ++i) {
    if (!strcasecmp(woodMaterials[i].name, name)) {
      return &woodMaterials[i];
    }
  }
  return nullptr;
}

// Organic material helpers
inline const OrganicMaterial* findOrganicMaterial(int matNum) {
  for (int i = 0; i < NUM_ORGANIC_MATERIALS; ++i) {
    if (organicMaterials[i].matNum == matNum) {
      return &organicMaterials[i];
    }
  }
  return nullptr;
}

inline const OrganicMaterial* findOrganicMaterialByName(const char* name) {
  if (!name) return nullptr;
  for (int i = 0; i < NUM_ORGANIC_MATERIALS; ++i) {
    if (!strcasecmp(organicMaterials[i].name, name)) {
      return &organicMaterials[i];
    }
  }
  return nullptr;
}

// Dead material helpers
inline const DeadMaterial* findDeadMaterial(int matNum) {
  for (int i = 0; i < NUM_DEAD_MATERIALS; ++i) {
    if (deadMaterials[i].matNum == matNum) {
      return &deadMaterials[i];
    }
  }
  return nullptr;
}

inline const DeadMaterial* findDeadMaterialByName(const char* name) {
  if (!name) return nullptr;
  for (int i = 0; i < NUM_DEAD_MATERIALS; ++i) {
    if (!strcasecmp(deadMaterials[i].name, name)) {
      return &deadMaterials[i];
    }
  }
  return nullptr;
}

// Rock material helpers
inline const RockMaterial* findRockMaterial(int matNum) {
  for (int i = 0; i < NUM_ROCK_MATERIALS; ++i) {
    if (rockMaterials[i].matNum == matNum) {
      return &rockMaterials[i];
    }
  }
  return nullptr;
}

inline const RockMaterial* findRockMaterialByName(const char* name) {
  if (!name) return nullptr;
  for (int i = 0; i < NUM_ROCK_MATERIALS; ++i) {
    if (!strcasecmp(rockMaterials[i].name, name)) {
      return &rockMaterials[i];
    }
  }
  return nullptr;
}

// Crystal material helpers
inline const CrystalMaterial* findCrystalMaterial(int matNum) {
  for (int i = 0; i < NUM_CRYSTAL_MATERIALS; ++i) {
    if (crystalMaterials[i].matNum == matNum) {
      return &crystalMaterials[i];
    }
  }
  return nullptr;
}

inline const CrystalMaterial* findCrystalMaterialByName(const char* name) {
  if (!name) return nullptr;
  for (int i = 0; i < NUM_CRYSTAL_MATERIALS; ++i) {
    if (!strcasecmp(crystalMaterials[i].name, name)) {
      return &crystalMaterials[i];
    }
  }
  return nullptr;
}

// Magical material helpers
inline const MagicalMaterial* findMagicalMaterial(int matNum) {
  for (int i = 0; i < NUM_MAGICAL_MATERIALS; ++i) {
    if (magicalMaterials[i].matNum == matNum) {
      return &magicalMaterials[i];
    }
  }
  return nullptr;
}

inline const MagicalMaterial* findMagicalMaterialByName(const char* name) {
  if (!name) return nullptr;
  for (int i = 0; i < NUM_MAGICAL_MATERIALS; ++i) {
    if (!strcasecmp(magicalMaterials[i].name, name)) {
      return &magicalMaterials[i];
    }
  }
  return nullptr;
}

// Spiritual material helpers
inline const SpiritualMaterial* findSpiritualMaterial(int matNum) {
  for (int i = 0; i < NUM_SPIRITUAL_MATERIALS; ++i) {
    if (spiritualMaterials[i].matNum == matNum) {
      return &spiritualMaterials[i];
    }
  }
  return nullptr;
}

inline const SpiritualMaterial* findSpiritualMaterialByName(const char* name) {
  if (!name) return nullptr;
  for (int i = 0; i < NUM_SPIRITUAL_MATERIALS; ++i) {
    if (!strcasecmp(spiritualMaterials[i].name, name)) {
      return &spiritualMaterials[i];
    }
  }
  return nullptr;
}

// Generic material helpers
inline const GenericMaterial* findGenericMaterial(int matNum) {
  for (int i = 0; i < NUM_GENERIC_MATERIALS; ++i) {
    if (genericMaterials[i].matNum == matNum) {
      return &genericMaterials[i];
    }
  }
  return nullptr;
}

inline const GenericMaterial* findGenericMaterialByName(const char* name) {
  if (!name) return nullptr;
  for (int i = 0; i < NUM_GENERIC_MATERIALS; ++i) {
    if (!strcasecmp(genericMaterials[i].name, name)) {
      return &genericMaterials[i];
    }
  }
  return nullptr;
}

extern sstring describeMaterial(const int);
extern sstring describeMaterial(const TThing*);

extern ubyte convertV9MaterialToV10(ubyte oldMat);
