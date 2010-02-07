//////////////////////////////////////////////////////////////////////////
//
// SneezyMUD - All rights reserved, SneezyMUD Coding Team
//
//  Constant integer and sstring arrays
//
//////////////////////////////////////////////////////////////////////////

#include "handler.h"
#include "extern.h"
#include "obj_drug.h"
#include "obj_gun.h"
#include "materials.h"
#include "being.h"

const sstring whitespace=" \f\n\r\t\v";  // from isspace() man page

const dirTypeT rev_dir[MAX_DIR] =
{
  DIR_SOUTH,
  DIR_WEST,
  DIR_NORTH,
  DIR_EAST,
  DIR_DOWN,
  DIR_UP,
  DIR_SOUTHWEST,
  DIR_SOUTHEAST,
  DIR_NORTHWEST,
  DIR_NORTHEAST
};

// all references to this need to go through the mapping function
// since the slot order has changed.  -- bat 06-27-97
const byte ac_percent_pos[MAX_WEAR] =
{
  0,
  1,
  1,
  5,
  20,
  15,
  7,
  7,
  4,
  4,
  2,
  2,
  6,
  6,
  10,
  10,
  1,
  1,
  0,
  0
};

const int TrapDir[] =
{
  TRAP_EFF_NORTH,
  TRAP_EFF_EAST,
  TRAP_EFF_SOUTH,
  TRAP_EFF_WEST,
  TRAP_EFF_UP,
  TRAP_EFF_DOWN,
  TRAP_EFF_NE,
  TRAP_EFF_NW,
  TRAP_EFF_SE,
  TRAP_EFF_SW,
  0,
  0,
  0
};

TTerrainInfo *TerrainInfo[MAX_SECTOR_TYPES];
TTerrainInfo::TTerrainInfo(int m, int t, int hu, int th, int dr, int h, int wet,const char * const n, const char * const p) :
  movement(m),
  thickness(t),
  hunger(hu),
  thirst(th),
  drunk(dr),
  heat(h),
  wetness(wet),
  name(n),
  prep(p)
{
}

TTerrainInfo::~TTerrainInfo()
{
}

// movement cost, thickness, hung, thirst, drunk, temp, humidity, name
void assignTerrainInfo()
{
  TerrainInfo[SECT_SUBARCTIC] = new TTerrainInfo(3,3,2,2,2, -30, -10, "Sub-Arctic Wastes", "in the");
  TerrainInfo[SECT_ARCTIC_WASTE] = new TTerrainInfo(3,3,2,2,2, -20, -10, "Arctic Wastes", "in the");
  TerrainInfo[SECT_ARCTIC_CITY] = new TTerrainInfo(1,6,2,2,2, 40,  -20, "Arctic City", "in an");
  TerrainInfo[SECT_ARCTIC_ROAD] = new TTerrainInfo(1,3,2,2,2, 30,  -10, "Arctic Road", "on an");
  TerrainInfo[SECT_TUNDRA] = new TTerrainInfo(2,0,2,2,2, -10, -10, "Tundra", "in the");
  TerrainInfo[SECT_ARCTIC_MOUNTAINS] = new TTerrainInfo(6,6,4,2,2, -50, 0, "Arctic Mountains", "in the");
  TerrainInfo[SECT_ARCTIC_FOREST] = new TTerrainInfo(4,8,3,2,2, 20, 0, "Arctic Forest", "in an");
  TerrainInfo[SECT_ARCTIC_MARSH] = new TTerrainInfo(5,5,2,2,2, 30, 20, "Arctic Marsh", "in an");
  TerrainInfo[SECT_ARCTIC_RIVER_SURFACE] = new TTerrainInfo(3,1,2,1,2, 30, 50, "Arctic River Surface", "on an");
  TerrainInfo[SECT_ICEFLOW] = new TTerrainInfo(4,2,2,2,2, 0, 50, "Iceflow (ocean)", "on an");
  TerrainInfo[SECT_COLD_BEACH] = new TTerrainInfo(2,3,2,2,2, 20,  20, "Cold Beach", "on a");
  TerrainInfo[SECT_SOLID_ICE] = new TTerrainInfo(8,9,2,2,2, -40, 0, "Solid Ice", "on");
  TerrainInfo[SECT_ARCTIC_BUILDING] = new TTerrainInfo(2,6,2,2,2, 45, -20, "Arctic Building", "in an");
  TerrainInfo[SECT_ARCTIC_CAVE] = new TTerrainInfo(3,7,2,2,2, 40, -10, "Arctic Cave", "in an");
  TerrainInfo[SECT_ARCTIC_ATMOSPHERE] = new TTerrainInfo(0,0,2,2,2, 10, -10, "Arctic Atmosphere", "in the");
  TerrainInfo[SECT_ARCTIC_CLIMBING] = new TTerrainInfo(9,2,3,2,2, 30, -10, "Arctic Vertical", "along an");
  TerrainInfo[SECT_ARCTIC_FOREST_ROAD] = new TTerrainInfo(2,5,2,2,2, 25, -10, "Arctic Forest Road", "on an");
  TerrainInfo[SECT_PLAINS] = new TTerrainInfo(2,3,2,2,2, 50, -10, "Plains", "on the");
  TerrainInfo[SECT_TEMPERATE_CITY] = new TTerrainInfo(1,6,2,2,2, 70,  -20, "Temperate City", "in a");
  TerrainInfo[SECT_TEMPERATE_ROAD] = new TTerrainInfo(1,3,2,2,2, 60,  -10, "Temperate Road", "on a");
  TerrainInfo[SECT_GRASSLANDS] = new TTerrainInfo(3,1,2,2,2, 70, -10, "Grasslands", "on some");
  TerrainInfo[SECT_TEMPERATE_HILLS] = new TTerrainInfo(3,5,3,2,2, 60, -10, "Temperate Hills", "in some");
  TerrainInfo[SECT_TEMPERATE_MOUNTAINS] = new TTerrainInfo(6,6,4,3,2, 50, -10, "Temperate Mountains", "in some");
  TerrainInfo[SECT_TEMPERATE_FOREST] = new TTerrainInfo(4,8,3,2,2, 70, -10, "Temperate Forest", "in a");
  TerrainInfo[SECT_TEMPERATE_SWAMP] = new TTerrainInfo(5,5,2,2,2, 60, 20, "Temperate Swamp", "in a");
  TerrainInfo[SECT_TEMPERATE_OCEAN] = new TTerrainInfo(4,2,2,2,2, 60, 50, "Temperate Ocean", "on a");
  TerrainInfo[SECT_TEMPERATE_RIVER_SURFACE] = new TTerrainInfo(3,1,2,1,2, 60, 50, "Temperate River Surface", "on a");
  TerrainInfo[SECT_TEMPERATE_UNDERWATER] = new TTerrainInfo(8,9,2,1,2, 50, 100, "Temperate Underwater", "in the");
  TerrainInfo[SECT_TEMPERATE_BEACH] = new TTerrainInfo(2,3,2,2,2, 70, 20, "Temperate Beach", "on a");
  TerrainInfo[SECT_TEMPERATE_BUILDING] = new TTerrainInfo(2,6,2,2,2, 75, -20, "Temperate Building", "in a");
  TerrainInfo[SECT_TEMPERATE_CAVE] = new TTerrainInfo(3,7,2,2,2, 70, -10, "Temperate Cave", "in a");
  TerrainInfo[SECT_TEMPERATE_ATMOSPHERE] = new TTerrainInfo(0,0,2,2,2, 60, -10, "Temperate Atmosphere", "in a");
  TerrainInfo[SECT_TEMPERATE_CLIMBING] = new TTerrainInfo(9,2,2,2,2, 60, -10, "Temperate Vertical", "along a");
  TerrainInfo[SECT_TEMPERATE_FOREST_ROAD] = new TTerrainInfo(2,5,2,2,2, 65, -10, "Temperate Forest Road", "on a");
  TerrainInfo[SECT_DESERT] = new TTerrainInfo(2,3,5,6,2, 120, -40, "Desert", "in the");
  TerrainInfo[SECT_SAVANNAH] = new TTerrainInfo(3,1,2,5,2, 90, -30, "Savannah", "in the");
  TerrainInfo[SECT_VELDT] = new TTerrainInfo(3,2,2,4,2, 90, -20, "Veldt", "in the");
  TerrainInfo[SECT_TROPICAL_CITY] = new TTerrainInfo(1,6,2,3,2, 100, -20, "Tropical City", "in a");
  TerrainInfo[SECT_TROPICAL_ROAD] = new TTerrainInfo(1,3,2,3,2, 90, -20, "Tropical Road", "on a");
  TerrainInfo[SECT_JUNGLE] = new TTerrainInfo(5,8,3,3,2, 110, 10, "Jungle", "in the");
  TerrainInfo[SECT_RAINFOREST] = new TTerrainInfo(4,8,3,2,2, 110,  10, "Rain Forest", "in a");
  TerrainInfo[SECT_TROPICAL_HILLS] = new TTerrainInfo(3,5,3,3,2, 90, 0, "Tropical Hills", "in some");
  TerrainInfo[SECT_TROPICAL_MOUNTAINS] = new TTerrainInfo(6,6,4,3,2, 80, 0, "Tropical Mountains", "in some");
  TerrainInfo[SECT_VOLCANO_LAVA] = new TTerrainInfo(6,6,3,3,2, 140, -100, "Volcano/Lava", "in some");
  TerrainInfo[SECT_TROPICAL_SWAMP] = new TTerrainInfo(5,5,3,3,2, 90,  20, "Tropical Swamp", "in a");
  TerrainInfo[SECT_TROPICAL_OCEAN] = new TTerrainInfo(4,2,2,3,2, 90,  50, "Tropical Ocean", "on a");
  TerrainInfo[SECT_TROPICAL_RIVER_SURFACE] = new TTerrainInfo(3,1,2,2,2, 80,  50, "Tropical River Surface", "on a");
  TerrainInfo[SECT_TROPICAL_UNDERWATER] = new TTerrainInfo(8,9,2,1,2, 70, 100, "Tropical Underwater", "in the");
  TerrainInfo[SECT_TROPICAL_BEACH] = new TTerrainInfo(2,3,2,3,2, 80,  20, "Tropical Beach", "on a");
  TerrainInfo[SECT_TROPICAL_BUILDING] = new TTerrainInfo(2,6,2,3,2, 90, -20, "Tropical Building", "in a");
  TerrainInfo[SECT_TROPICAL_CAVE] = new TTerrainInfo(3,7,2,3,2, 85, -10, "Tropical Cave", "in a");
  TerrainInfo[SECT_TROPICAL_ATMOSPHERE] = new TTerrainInfo(0,0,2,3,2, 85,  -10, "Tropical Atmosphere", "in a");
  TerrainInfo[SECT_TROPICAL_CLIMBING] = new TTerrainInfo(9,2,2,3,2, 85,  -10, "Tropical Vertical", "along a");
  TerrainInfo[SECT_RAINFOREST_ROAD] = new TTerrainInfo(2,5,2,2,2, 85,  -10, "Rainforest Road", "on a");
  TerrainInfo[SECT_ASTRAL_ETHREAL] = new TTerrainInfo(0,4,2,2,2, 60,  -10, "Astral/Ethereal Zone", "in a");
  TerrainInfo[SECT_SOLID_ROCK] = new TTerrainInfo(13,9,5,3,2, 60, -10, "Solid Rock", "inside");
  TerrainInfo[SECT_FIRE] = new TTerrainInfo(3,6,2,6,2, 500,-100, "Fire", "in a");
  TerrainInfo[SECT_INSIDE_MOB] = new TTerrainInfo(6,8,3,3,3, 90, 10, "Inside a Mob", "");
  TerrainInfo[SECT_FIRE_ATMOSPHERE] = new TTerrainInfo(0,0,2,6,2, 500, -100, "Fire Atmosphere", "in the");
  TerrainInfo[SECT_MAKE_FLY] = new TTerrainInfo(9,2,2,3,2, 85, -10, "Flying Sector", "in a");
  TerrainInfo[SECT_DEAD_WOODS] = new TTerrainInfo(4,8,3,2,2, 50,  -10, "Dead Woods", "in the");
}

const char * const exits[] =
{
  "North",
  "East ",
  "South",
  "West ",
  "Up   ",
  "Down ",
  "Northeast",
  "Northwest",
  "Southeast",
  "Southwest",
};

const char * const dirs[] =
{
  "north",
  "east",
  "south",
  "west",
  "up",
  "down",
  "northeast",
  "northwest",
  "southeast",
  "southwest",
  "\n"
};

const char * const scandirs[] =
{
  "north",
  "east",
  "south",
  "west",
  "up",
  "down",
  "ne",
  "nw",
  "se",
  "sw",
  "\n"
};

const char * const dirs_to_leading[] =
{
  "to the north",
  "to the east",
  "to the south",
  "to the west",
  "leading upward",
  "leading downward",
  "to the northeast",
  "to the northwest",
  "to the southeast",
  "to the southwest"
};

const char * const dirs_to_blank[] =
{
  "to the north",
  "to the east",
  "to the south",
  "to the west",
  "upwards",
  "downwards",
  "to the northeast",
  "to the northwest",
  "to the southeast",
  "to the southwest"
};

const char * const weekdays[7] =
{
  "Sunday",
  "Monday",
  "Tuesday",
  "Wednesday",
  "Thursday",
  "Friday",
  "Saturday"
};

const char * const month_name[12] =
{
  "January",
  "February",
  "March",
  "April",
  "May",
  "June",
  "July",
  "August",
  "September",
  "October",
  "November",
  "December",
};

const char * const fullness[] =
{
  "less than half ",
  "about half ",
  "more than half ",
  ""
};

itemInfo *ItemInfo[MAX_OBJ_TYPES];

itemInfo::itemInfo(const char * const n, const char * const cn, const char * const v0, int v0x, int v0n, const char * const v1, int v1x, int v1n, const char * const v2, int v2x, int v2n, const char * const v3, int v3x, int v3n) :
  name(n),
  common_name(cn),
  val0_info(v0),
  val0_max(v0x), val0_min(v0n),
  val1_info(v1),
  val1_max(v1x), val1_min(v1n),
  val2_info(v2),
  val2_max(v2x), val2_min(v2n),
  val3_info(v3),
  val3_max(v3x), val3_min(v3n)
{
}

itemInfo::~itemInfo()
{
}

void assign_item_info()
{
  ItemInfo[ITEM_UNDEFINED] = new itemInfo("Undefined","something unknown",
     "",0, 0, 
     "",0, 0, 
     "",0, 0,
     "",0, 0);
  ItemInfo[ITEM_LIGHT] = new itemInfo("Light","a lighting device",
     "Amount of light obj gives off when lit", 20, -2,
     "Max number of ticks for refueling light. -1 means no refueling.",10000,-1,
     "Ticks left before burns out.",10000,0,
     "1 = lit, 0 = unlit.  Should be set 0", 1, 0);
  ItemInfo[ITEM_SCROLL] = new itemInfo("Scroll","a scroll",
     "Level/learnedness of spells on scroll", 50, 1,
     "Which spell (-1 = NONE)", MAX_SKILL - 1, -1,
     "Which spell (-1 = NONE)", MAX_SKILL - 1, -1,
     "Which spell (-1 = NONE)", MAX_SKILL - 1, -1);
  ItemInfo[ITEM_WAND] = new itemInfo("Wand","a wand",
     "Level/learnedness of spell in wand", 50, 1,
     "Max Charges wand has", 10, 0,
     "Charges Left", 10, 0,
     "Which spell (-1 = NONE)", MAX_SKILL - 1, -1);
  ItemInfo[ITEM_STAFF] = new itemInfo("Staff","a staff",
     "Level/learnedness of spell on staff", 50, 1,
     "Max Charges staff has", 10, 0,
     "Changes Left", 10, 0,
     "Which spell (-1 = NONE)", MAX_SKILL - 1, -1);
  ItemInfo[ITEM_WEAPON] = new itemInfo("Weapon","some sort of weapon",
     "Special: sharpness", 0, 0,
     "Damage Level * 4 and Damage Precision", 10000, 0,
     "Weapon types 1 and 2.  See HELP WEAPON TYPES", 2147483647, 0,
     "Weapon type 3. See HELP WEAPON TYPES", 2147483647, 0);
  ItemInfo[ITEM_FUEL] = new itemInfo("Fuel","a flammable liquid",
     "Amount of fuel", 256, 0,
     "Max Amount of Fuel", 256, 0,
     "", 0, 0,
     "", 0, 0);
  ItemInfo[ITEM_OPAL] = new itemInfo("Opal/Powerstone","an opal",
     "Opal size in carats. >10 unusual  >20 very rare", 50, 1,
     "Powerstone strength (<= carat weight) (0 = not a powerstone yet)", 50, 0,
     "Current mana (<= 10* powerstone strength)", 500, 0,
     "Current successive failures. 2= can't grow stronger.", 2, 0);
  ItemInfo[ITEM_TREASURE] = new itemInfo("Treasure","an item of value",
     "Unused", 0, 0,
     "Unused", 0, 0,
     "Unused", 0, 0,
     "Unused", 0, 0);
  ItemInfo[ITEM_ARMOR] = new itemInfo("Armor","a piece of armor",
     "Apply to armor - DO NOT SET!", 0, 0,
     "Unused", 0, 0,
     "Unused", 0, 0,
     "Unused", 0, 0);
  ItemInfo[ITEM_WORN] = new itemInfo("Worn Object","a piece of clothing",
     "Unused", 0, 0,
     "Unused", 0, 0,
     "Unused", 0, 0,
     "Unused", 0, 0);
  ItemInfo[ITEM_OTHER] = new itemInfo("Other","something",
     "Unused", 0, 0,
     "Unused", 0, 0,
     "Unused", 0, 0,
     "Unused", 0, 0);
  ItemInfo[ITEM_TRASH] = new itemInfo("Trash","a piece of garbage",
     "Unused", 0, 0,
     "Unused", 0, 0,
     "Unused", 0, 0,
     "Unused", 0, 0);
  ItemInfo[ITEM_TRAP] = new itemInfo("Trap","a dangerous trap",
     "Level of trap", 50, 0,
     "What triggers trap.", (1<<MAX_TRAP_EFF) - 1, 0,
     "Trap-Type (damage-type)", MAX_TRAP_TYPES, 0,
     "Charges", 50, 0);
  ItemInfo[ITEM_CHEST] = new itemInfo("Chest","a large container",
     "Max weight inside", 300, 1,
     "Special - container flags", 0, 0,
     "Vnum of key that unlocks.  -1 == none", WORLD_SIZE - 1, -1,
     "Volume chest can hold", 30000, 1);
  ItemInfo[ITEM_NOTE] = new itemInfo("Note","a sheet of paper",
     "Unused", 0, 0,
     "Unused", 0, 0,
     "Unused", 0, 0,
     "Unused", 0, 0);
  ItemInfo[ITEM_DRINKCON] = new itemInfo("Liquid Container","a liquid container",
     "Max drink units   (1 drink unit = 1.0 fl.oz., 1 gallon = 128 units)", 2560, 0,
     "Number of units left", 2560, 0,
     "Liquid type - see help liquids", MAX_DRINK_TYPES - 1, 0,
     "1 = Poisoned, 2 = Unlimited drinks, 4 = Spillable, 8 = Frozen", 7, 0);
  ItemInfo[ITEM_KEY] = new itemInfo("Key","some sort of key",
     "unused?", 4, 0,
     "Unused", 0, 0,
     "Unused", 0, 0,
     "Unused", 0, 0);
  ItemInfo[ITEM_FOOD] = new itemInfo("Food","something edible",
     "Number of hours filled", 24, 0,
     "Unused", 0, 0,
     "Unused", 0, 0,
     "1 = Poisoned, 2 = Spoiled, 4 = Fished, 8 = Butchered. (can be added)", 3, 0);
  ItemInfo[ITEM_MONEY] = new itemInfo("Money","a pile of money",
     "Number of talens in money pile", 50000, 0,
				      "Currency type", MAX_CURRENCY, 0,
     "Unused", 0, 0,
     "Unused", 0, 0);
  ItemInfo[ITEM_PEN] = new itemInfo("Pen","a writing implement",
     "Unused", 0, 0,
     "Unused", 0, 0,
     "Unused", 0, 0,
     "Unused", 0, 0);
  ItemInfo[ITEM_BOAT] = new itemInfo("Boat","a flotation device",
     "Unused", 0, 0,
     "Unused", 0, 0,
     "Unused", 0, 0,
     "Unused", 0, 0);
  ItemInfo[ITEM_AUDIO] = new itemInfo("Audio","a communication device",
     "Frequency of noise", 500, 0,
     "Unused", 0, 0,
     "Unused", 0, 0,
     "Unused", 0, 0);
  ItemInfo[ITEM_BOARD] = new itemInfo("Board","a bulletin board",
     "Min. Level to View", 60, 0,
     "Unused", 0, 0,
     "Unused", 0, 0,
     "Unused", 0, 0);
  ItemInfo[ITEM_BOW] = new itemInfo("Bow","a bow",
     "Unused", 0, 0,
     "Internal usage (bow flags)", 1, 0,
     "Arrow Type  [SEE HELP ARROWS]", 7, 0,
     "Range of bow(in rooms)", 10, 0);
  ItemInfo[ITEM_ARROW] = new itemInfo("Arrow","an arrow",
     "Sharpness of arrow.", 100, 0,
     "Damage Level * 4", 240, 0,
     "Arrow Trap-Type / Trap-Level", 10, 0,
     "Arrow Type  [SEE HELP ARROWS]", 7, 0);
  ItemInfo[ITEM_BAG] = new itemInfo("Bag","a portable container",
     "Weight bag can hold", 50000, 1,
     "Special - container flags", 0, 0,
     "Vnum of key that unlocks.  -1 == none", WORLD_SIZE - 1, -1,
     "Volume bag can hold", 10000000, 1);
  ItemInfo[ITEM_CORPSE] = new itemInfo("Corpse","a dead body",
     "Flags", 1<<MAX_CORPSE_FLAGS - 1, 0,
     "Former Race", MAX_RACIAL_TYPES -1, 1,
     "Former Level", 0, 0,
     "Former Vnum", 0, 0);
  ItemInfo[ITEM_SPELLBAG] = new itemInfo("Spellbag","an enchanted bag",
     "Weight spellbag can hold", 500, 1,
     "Special - container flags", 0, 0,
     "Vnum of key that unlocks.  -1 == none", WORLD_SIZE - 1, -1,
     "Volume spellbag can hold", 100000, 1);
  ItemInfo[ITEM_COMPONENT] = new itemInfo("Component","a spell component",
     "Number of uses left", 10, 0,
     "Maximum number of uses", 10, 0,
     "Spell # component is for", 1000, 0,
     "Special - decay/useage", 15, 0);
  ItemInfo[ITEM_BOOK] = new itemInfo("Book","a book",
     "", 0, 0,
     "", 0, 0,
     "", 0, 0,
     "", 0, 0);
  ItemInfo[ITEM_PORTAL] = new itemInfo("Portal","an exit",
     "Special: destination & charges", 0, 0,
     "Portal type  See HELP PORTAL INFO", 12, 0,
     "Special: trap type", 0, 0,
     "Special: portal states & key number", 0, 0);
  ItemInfo[ITEM_WINDOW] = new itemInfo("Window","a window",
     "The vnum of the room window \"looks\" into", WORLD_SIZE - 1, -(WORLD_SIZE - 1),
     "", 0, 0,
     "", 0, 0,
     "", 0, 0);
  ItemInfo[ITEM_TREE] = new itemInfo("Tree","some sort of vegetation",
     "", 0, 0,
     "", 0, 0,
     "", 0, 0,
     "", 0, 0);
  ItemInfo[ITEM_TOOL] = new itemInfo("Tool","a tool",
     "Tool type", MAX_TOOL_TYPE - 1, 0,
     "Strength of tool. number of uses left", 100, 0,
     "Max Strength", 100, 0,
     "", 0, 0);
  ItemInfo[ITEM_HOLY_SYM] = new itemInfo("Holy Symbol","a holy symbol",
     "Strength of the holy symbol", 1250000, 0,
     "Max Strength of the symbol", 1250000, 0,
     "Faction of the Symbol", MAX_FACTIONS, 0,
     "", 0, 0);
  ItemInfo[ITEM_QUIVER] = new itemInfo("Quiver","a quiver",
     "Weight quiver can hold", 500, 1,
     "Special - container flags", 0, 0,
     "Unused", 0, 0,
     "Volume quiver can hold", 100000, 1);
  ItemInfo[ITEM_BANDAGE] = new itemInfo("Bandage","a bandage",
     "", 0, 0,
     "", 0, 0,
     "", 0, 0,
     "", 0, 0);
  ItemInfo[ITEM_STATUE] = new itemInfo("Statue","a piece of statuary",
     "", 0, 0,
     "", 0, 0,
     "", 0, 0,
     "", 0, 0);
  ItemInfo[ITEM_BED] = new itemInfo("Bed/Chair","a piece of furniture",
     "Special : min. position, max users", 0, 0,
     "Maximum Designed Size (users height in inches)", 100, 2,
     "Seat height above ground (in inches)", 100, 1,
     "Extra regen", 40, -1);
  ItemInfo[ITEM_TABLE] = new itemInfo("Table","a table",
     "", 0, 0,
     "", 0, 0,
     "", 0, 0,
     "", 0, 0);
  ItemInfo[ITEM_RAW_MATERIAL] = new itemInfo("Raw Material","a commodity",
     "", 0, 0,
     "", 0, 0,
     "", 0, 0,
     "", 0, 0);
  ItemInfo[ITEM_GEMSTONE] = new itemInfo("Gemstone","a gemstone",
     "", 0, 0,
     "", 0, 0,
     "", 0, 0,
     "", 0, 0);
  ItemInfo[ITEM_MARTIAL_WEAPON] = new itemInfo("Martial Weapon","a weapon of the martial-arts",
     "Special: sharpness", 0, 0,
     "Number of damage dice", 30, 1,
     "Size of damage dice", 30, 1,
     "Weapon type.  See HELP WEAPON TYPES", TYPE_MAX_HIT - TYPE_MIN_HIT, 1);
  ItemInfo[ITEM_JEWELRY] = new itemInfo("Jewelry","a piece of jewelry",
     "Unused", 0, 0,
     "Unused", 0, 0,
     "Unused", 0, 0,
     "Unused", 0, 0);
  ItemInfo[ITEM_VIAL] = new itemInfo("Vial","a vial",
     "Max liquid units", 3000, 0,
     "Number of units left", 3000, 0,
     "Liquid type - see help liquids", MAX_DRINK_TYPES - 1, 0,
     "1 = Poisoned, 2 = Unlimited drinks, 4 = Spillable, 8 = Frozen", 7, 0);
  ItemInfo[ITEM_PCORPSE] = new itemInfo("Player Corpse","a player's dead body",
     "Flags", 1<<MAX_CORPSE_FLAGS - 1, 0,
     "Former Race", MAX_RACIAL_TYPES -1, 1,
     "Former Level", 0, 0,
     "Former Vnum", 0, 0);
  ItemInfo[ITEM_POOL] = new itemInfo("Pool", "a pool of liquid",
     "Unused", 0, 0,		      
     "Number of drink units", 2560, 0,
     "Liquid type - see help liquids", MAX_DRINK_TYPES - 1, 0,
     "Decay (non-zero = no)", 1, 0);
  ItemInfo[ITEM_KEYRING] = new itemInfo("Keyring", "a keyring",
     "Weight keyring can hold", 500, 1,
     "Special - container flags", 0, 0,
     "Unused", 0, 0,
     "Volume keyring can hold", 100000, 1);
  ItemInfo[ITEM_RAW_ORGANIC] = new itemInfo("Fabric/Organic", "a commodity",
     "Classification Type.  See HELP RAW ORGANIC", 10, 1,
     "Total Units (-1 == Non-Unit)", 500, -1,
     "Level", 127, 1,
     "Added Effect.  See HELP RAW ORGANIC", 10, -1);
  ItemInfo[ITEM_FLAME] = new itemInfo("Fire/Flame", "a flame",
     "Light Value", 100, -100,
     "Heat Value", 99, 1,
     "", 0, 0,
     "", 0, 0);
  ItemInfo[ITEM_APPLIED_SUB] = new itemInfo("Applied Substance", "an applied substance",
     "Apply Flags", 1, 0,
     "Apply Method.  See HELP APPLY METHODS", 10, 1,
     "Level", 50, 1,
     "Spell/Skillnum", MAX_SKILL - 1, 0);
  ItemInfo[ITEM_GAS] = new itemInfo("Gas", "a gas",
      "Unused", 0, 0,
      "Unused", 0, 0,
      "Unused", 0, 0,
      "Unused", 0, 0);
  ItemInfo[ITEM_ARMOR_WAND] = new itemInfo("ArmorWand","power armor",
     "Level/learnedness of spell in armor-wand", 50, 1,
     "Max Charges armor-wand has", 10, 0,
     "Charges Left", 10, 0,
     "Which spell (-1 = NONE)", MAX_SKILL - 1, -1);
  ItemInfo[ITEM_DRUG_CONTAINER] = new itemInfo("Drug Container", "a drug container",
     "Type of Drug", MAX_DRUG-1, 0,
     "Max number of ticks for refuel light. -1 means no refuleing.",10000,-1,
     "Ticks left before burns out.",10000,0,
     "1 = lit, 0 = unit.  Should be set 0", 1, 0);   
  ItemInfo[ITEM_DRUG] = new itemInfo("Drug","a drug",
     "Amount of drug", 75, 0,
     "Max Amount of drug", 75, 0,
     "Type of Drug", MAX_DRUG-1, 0,
     "", 0, 0);
  ItemInfo[ITEM_GUN] = new itemInfo("Gun","a gun",
				    "Rate of fire", 10, 0,
				    "Damage Level * 4 and Damage Precision", 10000, 0,
				    "Bit flags", 2147483647, 0,
				    "Ammo type", AMMO_MAX-1, AMMO_NONE+1);
  ItemInfo[ITEM_AMMO] = new itemInfo("Ammo", "some ammo",
				     "Ammo type", AMMO_MAX-1, AMMO_NONE+1,
				     "Rounds", 500, 0,
				     "Unused", 0, 0,
				     "Unused", 0, 0);
  ItemInfo[ITEM_PLANT] = new itemInfo("Plant","a plant",
     "Type", 0, 0,
     "Age", 0, 0,
     "Unused", 0, 0,
     "Unused", 0, 0);
  ItemInfo[ITEM_COOKWARE] = new itemInfo("Cookware","a piece of cookware",
     "Max weight inside", 300, 1,
     "Special - container flags", 0, 0,
     "Vnum of key that unlocks.  -1 == none", WORLD_SIZE - 1, -1,
     "Volume chest can hold", 30000, 1);

  ItemInfo[ITEM_VEHICLE] = new itemInfo("Vehicle", "a vehicle",
     "Inside room of vehicle", 0, 0,
     "Vehicle type", 1, 0,
     "0=one room vehicle, 1=whole zone vehicle", 1, 0,
     "Special: portal states & key number", 0, 0);
  ItemInfo[ITEM_CASINO_CHIP] = new itemInfo("Casino Chip","a casino chip",
     "Unused", 0, 0,
     "Unused", 0, 0,
     "Unused", 0, 0,
     "Unused", 0, 0);
  ItemInfo[ITEM_POTION] = new itemInfo("Potion","a potion container",
     "Max drink units ", 2560, 0,
     "Number of units left", 2560, 0,
     "Liquid type - see help liquids", MAX_DRINK_TYPES - 1, 0,
     "1 = Poisoned, 2 = Unlimited drinks, 4 = Spillable, 8 = Frozen", 7, 0);
  ItemInfo[ITEM_POISON] = new itemInfo("Poison","a poison container",
     "Max drink units ", 2560, 0,
     "Number of units left", 2560, 0,
     "Liquid type - see help liquids", MAX_DRINK_TYPES - 1, 0,
     "1 = Poisoned, 2 = Unlimited drinks, 4 = Spillable, 8 = Frozen", 7, 0);
  ItemInfo[ITEM_HANDGONNE] = new itemInfo("Handgonne","a handgonne",
				    "Rate of fire", 1, 1,
				    "Damage Level * 4 and Damage Precision", 10000, 0,
				    "Bit flags", 2147483647, 0,
				    "Ammo type", AMMO_LEAD_SHOT, AMMO_LEAD_SHOT);
  ItemInfo[ITEM_EGG] = new itemInfo("Egg","an egg",
     "Special: Touched; Number of hours filled", 0, 0,
     "Incubation period", 500, 0,
     "VNum of mob to hatch", 2147483647, 0,
     "1 = Poisoned, 2 = Spoiled, 3 = both", 3, 0);
  ItemInfo[ITEM_CANNON] = new itemInfo("Cannon","a cannon",
				    "Rate of fire", 1, 1,
				    "Damage Level * 4 and Damage Precision", 10000, 0,
				    "Bit flags", 2147483647, 0,
				    "Ammo type", AMMO_CANNON_BALL, AMMO_CANNON_BALL);
  ItemInfo[ITEM_TOOTH_NECKLACE] = new itemInfo("Tooth necklace","a necklace of teeth",
     "Weight necklace can hold", 500, 1,
     "Special - container flags", 0, 0,
     "Unused", 0, 0,
     "Volume necklace can hold", 100000, 1);

  ItemInfo[ITEM_TRASH_PILE] = new itemInfo("trash pile","a pile of trash",
     "Weight pile can hold", 50000, 1,
     "Special - container flags", 0, 0,
     "Vnum of key that unlocks.  -1 == none", WORLD_SIZE - 1, -1,
     "Volume pile can hold", 10000000, 1);
  
  ItemInfo[ITEM_CARD_DECK] = new itemInfo("Card Deck", "a deck of cards",
     "Weight bag can hold", 50000, 1,
     "Special - container flags", 0, 0,
     "Vnum of key that unlocks.  -1 == none", WORLD_SIZE - 1, -1,
     "Volume bag can hold", 10000000, 1);

  ItemInfo[ITEM_SUITCASE] = new itemInfo("Suitcase","a suitcase",
     "Weight bag can hold", 50000, 1,
     "Special - container flags", 0, 0,
     "Vnum of key that unlocks.  -1 == none", WORLD_SIZE - 1, -1,
     "Volume bag can hold", 10000000, 1);
  ItemInfo[ITEM_SADDLE] = new itemInfo("Saddle","a saddle",
     "Unused", 0, 0,
     "Unused", 0, 0,
     "Unused", 0, 0,
     "Unused", 0, 0);
  ItemInfo[ITEM_HARNESS] = new itemInfo("Harness","a harness",
     "Unused", 0, 0,
     "Unused", 0, 0,
     "Unused", 0, 0,
     "Unused", 0, 0);
  ItemInfo[ITEM_SADDLEBAG] = new itemInfo("Saddlebag","a saddlebag",
     "Weight bag can hold", 50000, 1,
     "Special - container flags", 0, 0,
     "Vnum of key that unlocks.  -1 == none", WORLD_SIZE - 1, -1,
     "Volume bag can hold", 10000000, 1);
  ItemInfo[ITEM_WAGON] = new itemInfo("Wagon","a wagon",
     "Max weight inside", 5000, 1,
     "Special - container flags", 0, 0,
     "Vnum of key that unlocks.  -1 == none", WORLD_SIZE - 1, -1,
     "Volume wagon can hold", 500000, 1);
  ItemInfo[ITEM_MONEYPOUCH] = new itemInfo("Moneypouch","a moneypouch",
     "Weight bag can hold", 50000, 1,
     "Special - container flags", 0, 0,
     "Vnum of key that unlocks.  -1 == none", WORLD_SIZE - 1, -1,
     "Volume bag can hold", 10000000, 1);
  ItemInfo[ITEM_FRUIT] = new itemInfo("Fruit","something edible",
     "Number of hours filled", 24, 0,
     "Seed vnum", 2147483647, 0,
     "Unused", 0, 0,
     "1 = Poisoned, 2 = Spoiled, 4 = Fished, 8 = Butchered. (can be added)", 3, 0);
};

const char * const wear_bits[MAX_ITEM_WEARS] =
{
  "Take",
  "Finger",
  "Neck",
  "Body",
  "Head",
  "Legs",
  "Feet",
  "Hands",
  "Arms",
  "Unused",
  "Back",
  "Waist",
  "Wrist",
  "Unused",
  "Hold",
  "Throw"
};

const char * const bodyParts[MAX_WEAR + 1] =
{
  "",
  "head",
  "neck",
  "body",
  "back",
  "arm-right",
  "arm-left",
  "wrist-right",
  "wrist-left",
  "hand-right",
  "hand-left",
  "finger-right",
  "finger-left",
  "waist",
  "leg-right",
  "leg-left",
  "foot-right",
  "foot-left",
  "hold-right",
  "hold-left",
  "leg-back-right",
  "leg-back-left",
  "foot-back-right",
  "foot-back-left",
  "\n"
};

const char * const extra_bits[] =
{
  "Glow",
  "Hum",
  "Strung",
  "Shadowy",
  "Prototype",
  "Invisible",
  "Magic",
  "Nodrop",
  "Bless",
  "Spiked",
  "Hover",
  "Rusty",
  "Anti-CLERIC",
  "Anti-MAGE",
  "Anti-THIEF",
  "Anti-WARRIOR",
  "Anti-SHAMAN",
  "Anti-DEIKHAN",
  "Anti-RANGER",
  "Anti-MONK",
  "Paired",
  "No rent",
  "Float",
  "No purge",
  "Newbie",
  "No-Junk (Player)", // now unused
  "*Do Not Use*", // now unused
  "*Do Not Use*", // now unused
  "Item Attached",
  "Burning",
  "Charred",
  "No Locate",
  "\n"
};

const char * const room_bits[MAX_ROOM_BITS] =
{
  "ALWAYS-LIT",
  "DEATH",
  "NO-MOB",
  "INDOORS",
  "PEACEFUL",
  "NO-STEAL",
  "NO-ESCAPE",
  "NO-MAGIC",
  "NO-PORTAL",
  "PRIVATE",
  "SILENCE",
  "NO-ORDER",
  "NO-FLEE",
  "HAVE-TO-WALK",
  "ARENA",
  "NO-HEAL",
  "HOSPITAL",
  "SAVE ROOMS",
  "NO-AUTOFORMAT",
  "BEING EDITED",
  "ON-FIRE",
  "FLOODED"
};


const char * const exit_bits[MAX_DOOR_CONDITIONS] =
{
  "Closed",
  "Locked",
  "Secret",
  "Destroyed",
  "No-enter",
  "Trapped",
  "Caved-In",
  "Magically Warded",
  "Sloped up",
  "Sloped down"
};

const char * const chest_bits[] =
{
  "Closeable",
  "Pickproof",
  "Closed",
  "Locked",
  "Trapped",
  "Secret (No-See)",
  "Detect-Trap/Empty-Trap",
  "Detect-Trap/Ghost-Trap",
  "Weightless/Volumeless",
};

const char * const door_types[] =
{
  "None",
  "Door",
  "Trapdoor",
  "Gate",
  "Grate",
  "Portcullis",
  "Drawbridge",
  "Rubble",
  "Panel",
  "Screen",
  "Hatch",
};

const char * const affected_bits[AFF_MAX+1] =
{"Blind",
 "Invisible",
 "Swimming",
 "Detect-invisible",
 "Detect-magic",
 "Sense-life",
 "Levitating",
 "Sanctuary",
 "Group",
 "Webbed",
 "Curse",
 "Flying",
 "Poison",
 "Stunned",
 "Paralysis",
 "Infravision",
 "Water-breath",
 "Sleep",
 "Scrying",
 "Sneak",
 "Hide",
 "",
 "Charm",
 "",
 "Unused",
 "True_sight",
 "Eating Corpse",
 "Dragon Ride",
 "Silent",
 "",
 "Aggressor",
 "Clarity",
 "Flightworthy",
 "\n"
};

APP_type apply_types[MAX_APPLY_TYPES] = 
{
  {TRUE,"None"},
  {TRUE, "Strength"},
  {TRUE, "Intelligence"},
  {TRUE, "Wisdom"},
  {TRUE, "Dexterity"},
  {TRUE, "Constitution"},    // 5 
  {TRUE, "Karma"},
  {TRUE, "Sex"},
  {TRUE, "Age"},
  {TRUE, "Height"},
  {TRUE, "Weight"},      // 10 
  {TRUE, "Armor/AC"},
  {TRUE, "Max Hit Points"},
  {TRUE, "Mana"},
  {TRUE, "Move"},
  {FALSE, "Hitroll"},
  {FALSE, "Damroll"},
  {FALSE, "Hit & Dam"},  // 20 
  {TRUE, "Immunity"},
  {TRUE, "Skill/Spell"},
  {TRUE, "Magic Affect"},
  {TRUE, "Light"},
  {TRUE, "Noise"},
  {TRUE, "Can Be Seen"},
  {TRUE, "Vision"},   // 30 
  {FALSE, "Protection"},
  {TRUE, "Brawn"},
  {TRUE, "Agility"},
  {TRUE, "Focus"},
  {TRUE, "Speed"},   // 35 
  {TRUE, "Perception"},
  {TRUE, "Charisma"},
  {TRUE, "Discipline"},
  {FALSE, "Spell Hitroll"},
  {FALSE, "Hit Points"},
  {TRUE, "Crit. Frequency"},
  {TRUE, "Garbled Speech"},
};


const struct class_info classInfo[MAX_CLASSES] =
{
  {true, MAGE_LEVEL_IND, CLASS_MAGE, "mage", 
   DISC_MAGE, DISC_LORE, 0.43, 7.5, "M"},

  {true, CLERIC_LEVEL_IND, CLASS_CLERIC, "cleric",
   DISC_CLERIC, DISC_THEOLOGY, 0.47, 8, "C"},

  {true, WARRIOR_LEVEL_IND, CLASS_WARRIOR, "warrior",
   DISC_WARRIOR, DISC_NONE, 0.47, 8.5, "W"},

  {true, THIEF_LEVEL_IND, CLASS_THIEF, "thief",
   DISC_THIEF, DISC_NONE, 0.41, 8, "T"},

  {true, SHAMAN_LEVEL_IND, CLASS_SHAMAN, "shaman",
   DISC_SHAMAN, DISC_NONE, 0.39, 7.5, "S"},

  {true, DEIKHAN_LEVEL_IND, CLASS_DEIKHAN, "deikhan",
   DISC_DEIKHAN, DISC_THEOLOGY, 0.44, 7.5, "D"},

  {true, MONK_LEVEL_IND, CLASS_MONK, "monk",
   DISC_MONK, DISC_NONE, 0.44, 5.5, "K"},

  {false, RANGER_LEVEL_IND, CLASS_RANGER, "ranger",
   DISC_RANGER, DISC_NONE, 0.46, 7, "R"},

  {false, COMMONER_LEVEL_IND, CLASS_COMMONER, "commoner",
   DISC_ADVENTURING, DISC_NONE, 0.40, 5, "O"}
};


const char * const action_bits[] =
{
  "Changed Strings",
  "Sentinel",
  "Scavenger",
  "Disguised",
  "Nice-thief",
  "Aggressive",
  "Stay-zone",
  "Wimpy",
  "Annoying",
  "Hateful",
  "Afraid",
  "Immortal",
  "Hunting",
  "Deadly Poison",
  "Polymorphed",
  "Guarding",
  "Skeleton",
  "Zombie",
  "Ghost",
  "Diurnal",
  "Nocturnal",
  "Protector",
  "Protectee",
  "\n"
};


const char * const player_bits[] =
{
  "Brief",
  "Compact",
  "Wimpy",
  "Newbie-Help",
  "No-Hassle",
  "Stealth",
  "Hunting",
  "Mailing",
  "Logged",
  "Bugging",
  "Vt100",
  "Color",
  "Immortal",
  "Ansi",
  "Seeksgroup",
  "Banished",
  "Regular-Vt100",
  "No-Messages",
  "Right-Handed",
  "AFK",
  "132 columns",
  "Solo_quest",
  "Group_quest",
  "No snoop",
  "",
  "God-NoShout",
  "",
  "",
  "Killable",
  "Anonymous",
  "PG-13",
  "\n"
};


const char * const attr_player_bits[] =
{
  "Brief",
  "Compact",
  "Wimpy",
  "Newbie-Helper",
  "No-hassle",
  "Stealth",
  "Hunting",
  "Tell-an-immort-you-saw-this (MAILING)",
  "",				// note ... no LOGGED 
  "Tell-an-immort-you-saw-this (BUGGING)",
  "VT-100",
  "Color",
  "Immortal",
  "Ansi",
  "Seeks Group",
  "*BANISHED*",
  "Regular-VT100",
  "No-Messages",
  "Right-Handed",
  "AFK",
  "132 columns",
  "Solo quest",
  "Group quest",
  "",
  "Tell-an-immort-you-saw-this (unused1)",
  "God No-Shout",
  "Tell-an-immort-you-saw-this (NODIMD)",
  "Tell-an-immort-you-saw-this (unused5)",
  "Killable",
  "Anonymous",
  "PG-13",
  "\n"
};

const char * const prompt_mesg[] =
{
  "near death",
  "horrid",
  "awful",
  "bad",
  "wounded",
  "hurt",
  "injured",
  "good",
  "excellent",
  "perfect",
  "heroic",
  "\n"
};


const sstring position_types[] =
{
  "Dead",
  "Mortally wounded",
  "Incapacitated",
  "Stunned",
  "Sleeping",
  "Resting",
  "Sitting",
  "Engaged",
  "Fighting",
  "Crawling",
  "Standing",
  "Mounted",
  "Flying",
  "\n"
};

const char * const connected_types[MAX_CON_STATUS] =
{
  "Playing",
  "Get name",
  "Confirm name",
  "Read Password",
  "Confirm password",
  "Read motd",
  "Confirm changed pword",
  "Wizlock",
  "General Delete",
  "Disconnection",
  "New account", 
  "Account Password",
  "New login",
  "New account Password",
  "Account email",
  "Account terminal", 
  "Account connection",
  "Account change password",
  "Account change password",
  "Account change password",
  "Account delete character",
  "Account delete account",
  "Editing",
  "Timezone",
  "Delete confirmation",
  "Wizlock New",

  "CC: Error",
  "CC: Name",
  "CC: Disclaimer",
  "CC: Disclaimer",
  "CC: Disclaimer",
  "CC: Multiplay",
  "CC: Resetting",
  "CC: Launchpad",
  "CC: Rename",
  "CC: Sex",
  "CC: Hands",
  "CC: Race",
  "CC: Terrain",
  "CC: Class",
  "CC: Traits",
  "CC: Traits",
  "CC: Traits",
  "CC: Stats",
  "CC: Stats",
  "CC: Stats",
  "CC: Stats",
  "CC: Stats",
  "CC: Done",
  "CC: Max"
};


const char * const portal_self_enter_mess[MAX_PORTAL_TYPE] =
{
  "enter",
  "step into",
  "step onto",
  "climb down",
  "climb up",
  "slide down",
  "are sucked into",
  "are swallowed by",
  "are flushed down",
  "fall into",
  "step through",
  "fall down",
  "climb over",
  "climb into"
};


const char * const portal_other_enter_mess[MAX_PORTAL_TYPE] =
{
  "enters",
  "steps into",
  "steps onto",
  "climbs down",
  "climbs up",
  "slides down",
  "is sucked into",
  "is swallowed by",
  "is flushed down",
  "falls into",
  "steps through",
  "falls down",
  "climbs over",
  "climbs into"
};


const char * const portal_self_exit_mess[MAX_PORTAL_TYPE] =
{
  "exits",
  "steps out of",
  "steps off of",
  "climbs up from",
  "climbs down from",
  "climbs off of",
  "is thrown into the room from",
  "is spit out of",
  "is spewed out of",
  "climbs out of",
  "steps from",
  "climbs out of",
  "climbs over",
  "climbs out of"
};

const char * const editor_types_oedit[] =
{
  "save",           //  1
  "load",           //  2
  "modify",         //  3
  "list",           //  4
  "remove",         //  5
  "create",         //  6
  "name",           //  7
  "long_desc",      //  8
  "short_desc",     //  9
  "max_struct",     // 10
  "cur_struct",     // 11
  "weight",         // 12
  "volume",         // 13
  "cost",           // 14
  "decay_time",     // 15
  "extra",          // 16
  "can_be_seen",    // 17
  "max_exist",      // 18
  "average",        // 19
  "replace",        // 20
  "resave",         // 21
  "\n"
};

const char * const editor_types_medit[] =
{
  "save",           //  1
  "load",           //  2
  "modify",         //  3
  "list",           //  4
  "remove",         //  5
  "name",           //  6
  "short_desc",     //  7
  "long_desc",      //  8
  "description",    //  9
  "level",          // 10
  "attacks",        // 11
  "hitroll",        // 12
  "armor_class",    // 13
  "hp_bonus",       // 14
  "faction",        // 15
  "bare_hand",      // 16
  "money_constant", // 17
  "race",           // 18
  "sex",            // 19
  "max_exist",      // 20
  "default_pos",    // 21
  "height",         // 22
  "weight",         // 23
  "class",          // 24
  "vision",         // 25
  "can_be_seen",    // 26
  "room_sound",     // 27
  "oroom_sound",    // 28
  "replace",        // 29
  "resave",         // 30
  "average",        // 31
  "\n"
};

const char * const material_groups[] =
{
  "General",
  "Nature/Skin Types",
  "Mineral",
  "Metals",
  "\n"
};

const char * const attack_modes[] =
{
  "normal",
  "defensive",
  "offensive",
  "berserk",
  "\n"
};

// formula for fall/water susceptibility is:
// chance in 10 of being damaged + (10 * points to remove) 
// So... if the number is 99 it would mean, 90% (9 in 10)
// chance of being damaged, with 9 struct points removed
// when it is damaged.  Note, maximum damage rate is:
// 249, meaning 90% chance of 24 struct points damage.
// 0 means no chance of damage.  11 would mean, 10% chance
// of being damaged 1 struct point.


extern int repairMetal(TBeing *,TObj *);
extern int repairDead(TBeing *,TObj *);
extern int repairOrganic(TBeing *,TObj *);
extern int repairWood(TBeing *,TObj *);
extern int repairMagical(TBeing *,TObj *);
extern int repairRock(TBeing *,TObj *);
extern int repairCrystal(TBeing *,TObj *);
extern int repairHide(TBeing *,TObj *);
extern int repairGeneric(TBeing *, TObj *);
extern int repairSpiritual(TBeing *, TObj *);
//extern int repairScale(TBeing *, TObj *);

// float weight == maximum weight an object can be of this
// type and stay floating.  0 means object will sink no matter
// what.  255 means object will float no matter what.  */
// hardness: 0 = air
// 5 = paper, 10 = rubber, 25 = wood, 45+ = metal, 100 = dimond 
// fall/water susc:
// chance of damage: dice(1,10) <= number%10
// amount of damage: number/10
// flammability: how many cubic inches burn per tick
// slash susc, blunt susc, fire susc, pierce susc,
// hardness, water susc, fall susc, float weight, noise, vol_mult, conduct
// flammability, acid_susc, price, availability, repair proc, mat_name */

const struct material_type_numbers material_nums[200] =
{
  {100, 100, 100, 100, 50, 11, 11, 5,10, 1, 0, 0, 0, 0, NULL, "undefined"},
  {100,  20, 100, 100, 5, 209, 0, 5,-1,8, 0, 1000, 75, 0.06, repairGeneric,"paper"},
  {90, 0, 100, 100, 1, 11, 0, 30, -3,8, 0, 750, 65, 1, repairGeneric,"cloth"},
  {50, 50, 95, 80, 7, 153, 105, 0,-2,1, 0, 250, 40, 0.2, repairGeneric,"wax"},
  {0, 100, 0, 50, 40, 0, 249, 0, 3,1, 0, 0, 5, 0.5, NULL,"glass"},
  {40, 30, 75, 50, 25, 42, 53, 255, 3,1, 0, 500, 25, 0.05, repairWood,"wood"},
  {90, 0, 50, 100, 7, 22, 0, 40, -2,6, 0, 900, 80, 3, repairGeneric,"silk"},
  {75, 75, 75, 100, 5, 35, 55, 30, 1,2, 0, 0, 95, 0.8, repairSpiritual,"foodstuff"},
  {65, 25, 25, 100, 3, 0, 51, 0, -1,2, 0, 500, 50, 6, repairGeneric,"plastic"},
  {75, 0, 0, 100, 10, 0, 0, 0, -2,1, 0, 0, 66, 3, repairGeneric,"rubber"},
  {40, 20, 100, 100, 6, 209, 0, 10, 0,1, 0, 900, 80, 0.08, repairGeneric,"cardboard"},
  {40, 10, 75, 50, 2, 105, 0, 100, -1,2, 0, 750, 85, 0.75, repairGeneric,"string"},
  {75, 50, 25, 50, 4, 59, 79, 2, 0,1, 0, 0, 25, 10, repairMagical,"plasma"},
  {83, 0, 100, 95, 2, 11, 0, 25, -3,6, 0, 600, 60, 1.25, repairGeneric,"toughened cloth"},
  {80, 100, 10, 40, 3, 0, 99, 20, 3, 1, 0, 0, 90, 3.2, repairOrganic,"coral"},
  {80, 3, 80, 10, 2, 101, 0, 100, -1, 3, 0, 750, 70, 1, repairGeneric,"horsehair"},
  {40, 10, 75, 50, 2, 105, 0, 100, -1, 2, 0, 750, 85, 1.2, repairGeneric,"hair"},
  {90, 5,  2, 50, 2, 103, 11,  99, -1, 4, 0, 0, 30, 0.3, repairDead,"ash"},
  {50, 65, 70, 25, 2,  92, 53, 100, -1, 1, 0, 0, 60, 1.3, repairRock, "pumice"},
// slash susc, blunt susc, fire susc, pierce susc,
// hardness, water susc, fall susc, float weight, noise, vol_mult, conduct
// flammability, acid_susc, price, base commod vnum, repair proc, mat_name 
  {50, 25, 4, 40, 12, 30, 36, 3, -1, 3, 0, 0, 25, 4.1, repairHide,"laminate"},
  {0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, NULL,""},
  {0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, NULL,""},
  {0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, NULL,""},
  {0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, NULL,""},
  {0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, NULL,""},
  {0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, NULL,""},
  {0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, NULL,""},
  {0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, NULL,""},
  {0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, NULL,""},
  {0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, NULL,""},
  {0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, NULL,""},
  {0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, NULL,""},
  {0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, NULL,""},
  {0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, NULL,""},
  {0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, NULL,""},
  {0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, NULL,""},
  {0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, NULL,""},
  {0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, NULL,""},
  {0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, NULL,""},
  {0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, NULL,""},
  {0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, NULL,""},
  {0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, NULL,""},
  {0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, NULL,""},
  {0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, NULL,""},
  {0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, NULL,""},
  {0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, NULL,""},
  {0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, NULL,""},
  {0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, NULL,""},
  {0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, NULL,""},
  {0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, NULL,""},
  {50, 50, 50, 50, 0, 11, 55, 5, 10, 1, 0, 500, 50, 0.8, NULL,"generic organic"},
  {75, 5, 5, 70, 20, 55, 55, 3, -2, 4, 0, 400, 20, 2, repairHide,"leather"},
  {55, 5, 1, 50, 25, 35, 36, 3, -1, 3, 0, 200, 30, 2.5, repairHide,"toughened leather"},
  {10, 50, 10, 30, 50, 11, 0, 0, 5, 1, 0, 0, 10, 157.5, repairOrganic,"dragon scale"},
  {70, 0, 100, 75, 15, 33, 0, 50, -2, 4, 0, 800, 0,   1.1,   repairGeneric,"wool"},
  {45, 5, 100, 60, 15, 33, 0, 50, -2, 4, 0, 800, 30,   1.6,   repairHide,"fur"},
  {30, 5, 100, 45, 15, 0, 0, 60, -3, 1, 0, 900, 15,   1.4,   repairHide,"feathered"},
  {0, 0, 0, 0, 4, 0, 0, 255, 3, 1, 0, 0, 3,   15,   repairMagical,"liquid"},
  {0, 0, 0, 0, 0, 249, 105, 0, 8, 1, 0, 0, 3,   15,   repairMagical,"fire"},
  {0, 0, 0, 0, 0, 0, 205, 0, 10, 1, 0, 0, 55,   0.05,   repairMagical,"earth"},
  {25, 25, 100, 25, 0, 0, 205, 0, 10, 1, 0, 0, 35,   12,   repairMagical,"generic elemental"},
  {25, 25, 25, 25, 22, 205, 109, 255, 2, 1, 0, 0, 6,   15,   repairMagical,"ice"},
  {0, 0, 0, 0, 0, 249, 0, 0, 13, 1, 0, 0, 7,   15,   repairMagical,"lightning"},
  {0, 0, 5, 0, 0, 0, 0, 0, 5, 1, 0, 0, 10,   16,   repairMagical,"chaos"},
  {45, 25, 0, 50, 5, 101, 249, 0, 3, 1, 0, 0, 3,   0.05,   repairGeneric,"clay"},
  {0, 100, 50, 0, 30, 101, 249, 0, 10, 1, 0, 0, 2,   4,   repairGeneric,"porcelain"},
  {50, 10, 100, 10, 10, 22, 0, 30, 2, 1, 0, 1000, 77,   0.2,   repairGeneric,"straw"},
  {5, 50, 10, 0, 40, 0, 39, 0, 10, 1, 0, 0, 66,   165,   repairRock,"pearl"},
  {100, 10, 60, 100, 12, 11, 0, 10, 0, 1, 0, 150, 85,   0.5,   repairDead,"flesh"},
  {55, 5, 63, 60, 15, 11, 0, 10, -1, 1, 0, 800, 72,   1.2,   repairHide,"cat fur"},
  {45, 5, 60, 70, 15, 11, 0, 10, 1, 1, 0, 800, 66,   1.2,   repairHide,"dog fur"},
  {65, 5, 60, 80, 13, 11, 0, 10, -2, 1, 0, 800, 68,   2,   repairHide,"rabbit fur"},
  {45, 10, 10, 50, 1, 249, 0, 255, -5, 1, 0, 0, 5,   2,   repairSpiritual,"ghostly"},
  {43, 7, 5, 45, 24, 36, 36, 0, -1, 5, 0, 300, 25,   1.5,   repairHide,"dwarven leather"},
  {83, 13,15, 75, 25, 56, 56, 9, -3, 6, 0, 400, 33,   1.3,   repairHide,"soft leather"},
  {53,57,45, 65, 30, 0, 12,30, 2, 2, 0, 0, 12,   1.1,   repairOrganic,"fishscale"},
  {90, 15,20,95,33,12,11, 7, 1, 2, 0, 200, 10,   2.75,   repairDead,"ogre hide"},
  {70, 0, 100, 75, 10, 33, 0, 50, -2, 4, 0, 750, 0,   3,   repairGeneric,"hemp"},
// slash susc, blunt susc, fire susc, pierce susc,
// hardness, water susc, fall susc, float weight, noise, vol_mult, conduct
// flammability, acid_susc, price, base commod vnum, repair proc, mat_name 
  {0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0,   0,   NULL,""},
  {0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0,   0,   NULL,""},
  {0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0,   0,   NULL,""},
  {0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0,   0,   NULL,""},
  {0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0,   0,   NULL,""},
  {0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0,   0,   NULL,""},
  {0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0,   0,   NULL,""},
  {0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0,   0,   NULL,""},
  {0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0,   0,   NULL,""},
  {0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0,   0,   NULL,""},
  {0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0,   0,   NULL,""},
  {0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0,   0,   NULL,""},
  {0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0,   0,   NULL,""},
  {0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0,   0,   NULL,""},
  {0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0,   0,   NULL,""},
  {0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0,   0,   NULL,""},
  {0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0,   0,   NULL,""},
  {0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0,   0,   NULL,""},
  {0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0,   0,   NULL,""},
  {0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0,   0,   NULL,""},
  {0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0,   0,   NULL,""},
  {0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0,   0,   NULL,""},
  {5, 50, 8, 20, 50, 0, 39, 0, 10, 1, 0, 0, 31,   171,   repairCrystal,"jeweled"},  
  {0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0,   0,   NULL,""},
  {10, 60, 10, 24, 48, 0, 39, 0, 10, 1, 1, 0, 31,   95,   repairMagical,"runed"},
  {25, 45, 0, 25, 60, 0, 39, 0, 13, 1, 0, 0, 31,   70,   repairCrystal,"crystal"},
  {1, 12, 0, 5, 100, 0, 0, 0, 14, 1, 0, 0, 21,   250,   repairCrystal,"diamond"},
  {10, 30, 0, 10, 50, 0, 39, 0, 10, 1, 0, 0, 24,   70,   repairWood,"ebony"},
  {3, 15, 0, 3, 75, 0, 0, 0, 10, 1, 0, 0, 31,   184.6,   repairCrystal,"emerald"},
  {21, 20, 0, 21, 50, 0, 39, 0, 10, 1, 0, 0, 31,   70,   repairDead,"ivory"},
  {21, 20, 0, 21, 50, 0, 39, 0, 10, 1, 0, 0, 31,   133.334,   repairRock,"obsidian"},
  {21, 20, 0, 21, 50, 0, 39, 0, 10, 1, 0, 0, 31,   80,   repairCrystal,"onyx"},
  {11, 18, 0, 11, 60, 0, 39, 0, 10, 1, 0, 0, 31,   159,   repairCrystal,"opal"},
  {2, 56, 0, 2, 80, 0, 39, 0, 10, 1, 0, 0, 31,   195.5,   repairCrystal,"ruby"},
  {5, 47, 0, 5, 70, 0, 39, 0, 10, 1, 0, 0, 31,   150,   repairCrystal,"sapphire"},
  {6, 20, 0, 6, 90, 0, 79, 0, 10, 1, 0, 0, 31,   65,   repairRock,"marble"},
  {8, 24, 0, 8, 95, 0, 99, 0, 10, 1, 0, 0, 31,   0.05,   repairRock,"stone"},
  {15, 40, 0, 15, 40, 0, 39, 0, 8, 1, 0, 0, 83,   2.7,   repairDead,"bone"},
  {13, 35, 0, 13, 80, 0, 29, 0, 10, 1, 0, 0, 44,   70,   repairRock,"jade"},
  {5, 60, 0, 10, 65, 0, 39, 0, 10, 1, 0, 0, 20,   40,   repairRock,"amber"},
  {11, 45, 0, 15, 55, 0, 39, 0, 11, 1, 0, 0, 31,   35,   repairRock,"turquoise"},
  {16, 42, 0, 12, 55, 0, 39, 0, 11, 1, 0, 0, 31,   35,   repairCrystal,"amethyst"},
  {45, 15, 0, 25, 40, 0, 39, 0, 10, 1, 1, 0, 25,   80,   repairCrystal,"mica"},
  {15, 15, 0, 10, 85, 0, 10, 0, 14, 1, 0, 0, 25,   153,   repairDead,"dragonbone"},
  {16, 42, 0, 12, 55, 0, 39, 0, 11, 1, 0, 0, 31,   85,   repairRock,"malachite"},
  {6, 20, 0, 6, 90, 0, 71, 0, 10, 1, 0, 0, 31,   7,   repairRock,"granite"},
  {25, 45, 0, 15, 60, 0, 89, 0, 6, 1, 0, 0, 31,   70,   repairCrystal,"quartz"},
  {2, 56, 0, 2, 90, 0, 39, 0, 6, 1, 0, 0, 31,   70,   repairRock,"jet"},
  {1, 12, 0, 5, 100, 0, 0, 0, 14, 1, 0, 0, 21,   201,   repairCrystal,"corundum"},
// slash susc, blunt susc, fire susc, pierce susc,
// hardness, water susc, fall susc, float weight, noise, vol_mult, conduct
// flammability, acid_susc, price, base commod vnum, repair proc, mat_name 
  {0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0,   0,   NULL,""},
  {0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0,   0,   NULL,""},
  {0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0,   0,   NULL,""},
  {0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0,   0,   NULL,""},
  {0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0,   0,   NULL,""},
  {0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0,   0,   NULL,""},
  {0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0,   0,   NULL,""},
  {0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0,   0,   NULL,""},
  {0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0,   0,   NULL,""},
  {0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0,   0,   NULL,""},
  {0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0,   0,   NULL,""},
  {0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0,   0,   NULL,""},
  {0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0,   0,   NULL,""},
  {0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0,   0,   NULL,""},
  {0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0,   0,   NULL,""},
  {0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0,   0,   NULL,""},
  {0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0,   0,   NULL,""},
  {0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0,   0,   NULL,""},
  {0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0,   0,   NULL,""},
  {0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0,   0,   NULL,""},
  {0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0,   0,   NULL,""},
  {0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0,   0,   NULL,""},
  {0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0,   0,   NULL,""},
  {0, 50, 0, 20, 55, 101, 101, 0, 20, 1, 1, 0, 70,   5,   repairMetal,"generic metal"},
  {0, 75, 0, 20, 45, 101, 101, 0, 20, 1, 1, 0, 70,   0.5,   repairMetal,"copper"},
  {0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0,   0,   NULL,""},
  {0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0,   0,   NULL,""},
  {0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0,   0,   NULL,""},	
  {0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0,   0,   NULL,""},
  {0, 70, 0, 10, 50, 101, 101, 0, 25, 1, 1, 0, 70,   9.1,    repairMetal,"bronze"},
  {0, 79, 0, 5,  50, 101, 101, 0, 25, 1, 1, 0, 70,   3.334,   repairMetal,"brass"},
  {0, 50, 0, 0,  60, 101, 101, 0, 27, 1, 1, 0, 70,   5,    repairMetal,"iron"},
  {0, 45, 0, 0,  63, 101, 101, 0, 33, 1, 1, 0, 70,   8.334,   repairMetal,"steel"},
  {0, 40, 0, 0,  65, 101, 101, 0, 23, 1, 1, 0, 50,   150,    repairMetal,"mithril"},
  {0, 40, 0, 0,  70, 101, 101, 0, 29, 1, 1, 0, 45,   90.9,   repairMetal,"admantium"},
  {0, 50, 0, 0,  50, 101, 101, 0, 35, 1, 1, 0, 70,   11.112,   repairMetal,"silver"},
  {0, 50, 0, 2,  49, 101, 101, 0, 35, 1, 1, 0, 70,   30,   repairMetal,"gold"},
  {0, 50, 0, 0,  50, 101, 101, 0, 39, 1, 1, 0, 70,   45.45,   repairMetal,"platinum"},
  {0, 25, 0, 0,  65, 101,  51, 0, 31, 1, 1, 0, 70,   66.667,   repairMetal,"titanium"},
  {0, 80, 0, 30, 43, 101, 101, 0, 34, 1, 1, 0, 70,   0.125,   repairMetal,"aluminum"},
  {0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0,   0,   NULL,""},
  {0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0,   0,   NULL,""},
  {0, 50, 0, 0, 55, 101, 101, 0, 30, 1, 1, 0, 42,   10.7,   repairMetal,"electrum"},
  {30, 20, 5, 20, 55, 101, 101, 0, 37, 1, 1, 0, 10,   142.86,  repairMetal,"athanor"},
  {0, 85, 0, 0, 40, 101, 101, 0, 33, 1, 1, 0, 77,   0.334,   repairMetal, "tin"},
  {0, 47, 0, 3, 47, 101, 101, 0, 35, 1, 1, 0, 70,   9,   repairMetal,"tungsten"},
  {0, 50, 0, 20, 65, 101, 101, 0, 10, 1, 1, 0, 70,   9,   repairMetal,"admintite"},
  {0, 25, 0,  0, 50,   0,  25, 0, 10, 1, 0, 0, 89,   9,   repairMetal, "terbium"},
  {0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0,   0,   NULL,""},
  {0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0,   0,   NULL,""},
  {0, 30, 15, 0,  68, 0, 101, 0, 23, 1, 1, 0, 40,   163.5,   repairMetal,"eternium"},
  {0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0,   0,   NULL,""},
// slash susc, blunt susc, fire susc, pierce susc,
// hardness, water susc, fall susc, float weight, noise, vol_mult, conduct
// flammability, acid_susc, price, base commod vnum, repair proc, mat_name 
};

// this is how blunt/sharp/pointy weapon is
const byte sharpness[TYPE_MAX_HIT - TYPE_MIN_HIT] =
{
  20,         // TYPE_HIT
  50,         // TYPE_BLUDGEON
  20,         // TYPE_WHIP
  60,         // TYPE_CRUSH
  60,         // TYPE_SMASH
  40,         // TYPE_SMITE
  30,         // TYPE_PUMMEL
  25,         // TYPE_FLAIL
  15,         // TYPE_BEAT
  66,         // TYPE_THRASH
  70,         // TYPE_THUMP
  80,         // TYPE_WALLOP
  75,         // TYPE_BATTER
  55,         // TYPE_STRIKE
  45,         // TYPE_CLUB
  50,         // TYPE_POUND

  70,         // TYPE_PIERCE
  35,         // TYPE_BITE
  65,         // TYPE_STING
  70,         // TYPE_STAB
  55,         // TYPE_THRUST
  60,         // TYPE_SPEAR
  50,         // TYPE_BEAK


  50,         // TYPE_SLASH
  40,         // TYPE_CLAW
  60,         // TYPE_CLEAVE
  70,         // TYPE_SLICE

  50,         // TYPE_AIR
  50,         //TYPE_EARTH
  50,         //TYPE_FIRE
  50,         //TYPE_WATER
  50,         //TYPE_BEAR_CLAW
  50,         //TYPE_KICK
  50,         //TYPE_MAUL
  50,         //TYPE_SHOOT
  50,         //TYPE_CANNON
  65,         //TYPE_SHRED
};

const char * const body_flags[] =
{
  "bleeding",
  "infected",
  "paralyzed",
  "broken",
  "scarred",
  "bandaged",
  "missing",
  "useless",
  "leprosed",
  "transformed",
  "entangled",
  "bruised",
  "gangrenous"
  "\n"
};

const char * const card_names[14] =
{
  "Nothing",
  "Ace",
  "Two",
  "Three",
  "Four",
  "Five",
  "Six",
  "Seven",
  "Eight",
  "Nine",
  "Ten",
  "Jack",
  "Queen",
  "King"
};


// these numbers are cubic inches of volume for each inch of height
// 157.14 for body, for a 70 inch tall person would require an item of about
// 11000 cubic inches for a perfect fit (157.14 * 70)
const double race_vol_constants[MAX_WEAR] = {
  0,       // none
  0,       // finger R
  0,       // finger L
  12.86,   // neck
  157.14,  // body
  35.71,   // head
  85.71,   // leg right
  85.71,   // leg left
  22.86,   // foot right
  22.86,   // foot left
  11.43,   // hand right
  11.43,   // hand left
  28.57,   // arm right
  28.57,   // arm left
  35.71,   // back
  57.14,   // waist
  5.71,    // wrist right
  5.71,    // wrist left
  0,       // hold right
  0,       // hold left
  85.71,   // extra leg right
  85.71,   // extra leg left
  22.86,   // extra foot right
  22.86,   // extra foot left
};

const char * const immunity_names[MAX_IMMUNES] =
{
  "Heat",     // 0
  "Cold",
  "Acid",
  "Poison",
  "Sleep",
  "Paralysis",     // 5
  "Charm",
  "Pierce",
  "Slash",
  "Blunt",
  "Non-Magic",   // 10
  "+1 Weapons",
  "+2 Weapons",
  "+3 Weapons",
  "Air",
  "Energy/Thaumaturgy",    // 15
  "Electricity",
  "Disease",
  "Suffocation",
  "Skin Problems",
  "Bone Problems",      // 20
  "Bleeding",
  "Water",
  "Drain",
  "Fear",
  "Earth",     // 25
  "Summon",
  "",
};

// make first letter '*' to block the sstring anywhere in the name
// otherwise, blocks the exact name (case insensative)
// that is "*fuck" blocks "somefuck", "fucksome", and "sfucky"
const char * const illegalnames[] = 
{
  // SWEAR WORDS
  "*fuk",
  "*fuck",
  "*shit",
  "*dick",
  "*cunt",
  "*pussy",
  "*twat",
  "*bitch",
  "*bastard",
  "*fart",
  "*feces",
  "*mofo",
  "*pimp",
  "*nigger",
  "*penis",
  "*crap",
  "jackmeoff",
  "piss",
  "pee",
  "urine",
  "cum",
  "butt",
  "suck",
  "crotch",
  "ass",

  // IMM NAME SIMILARITY
  "brutus",
  "fatopr",
  "batoper",
  
  // NEWBIE PROTECTION
  "*admin",
  "root",

  // BASIC OBJECTS
  "*shield",
  "*talen",

  // COLORS
  "gold",
  "silver",
  "bronze",
  "iron",
  "metal",
  "black",
  "white",
  "red",
  "blue",
  "green",

  // SILLY COMMON WORDS
  "*some",  // someone, something, etc
  "himself",
  "itself",
  "herself",
  "the",
  "an",
  "and",
  "there",
  "you",
  "then",
  "women",
  "men",

  // BLOCKED AS CAUSES CODE PROBLEMS
  "blob",     // needed for infravision 
  "blobs",    // needed for infravision 
  "link",     // needed for purging links 
  "links",    // needed for purging links 
  "noone",   // needed for makeleader
  "comment", // needed for comments
  "all",      // "group all"
  "unknown",   // Log messages use this sometimes
  "share",   // group needs this

  // MAJOR CHARACTERS
  "logrus",
  "galek",
  "anilstathis",
  "theoman",
  "mezan", // deities
  "luna", // deities
  "icon", // deities
  "elyon", // deities
  "jevon", // deities
  "omnon", // deities
  "amana", // deities
  "menanon", // deities

  // DIRECTIONS that are abbrevs
  "nor",
  "nort",
  "north",
  "eas",
  "east",
  "sou",
  "sout",
  "south",
  "wes",
  "west",
  "up",
  "dow",
  "down",
  "northe",
  "northea",
  "northeas",
  "northeast",
  "northw",
  "northwe",
  "northwes",
  "northwest",
  "southe",
  "southea",
  "southeas",
  "southeast",
  "southw",
  "southwe",
  "southwes",
  "southwest",
  "exit",
  "exits",
  "\n"
};

// CHANGE NUMBER if options added
const char * const color_options[10] =
{
  "Color Basic           (color enabled)",
  "Color Objects         (color objects)",
  "Color Creatures       (color creatures)",
  "Color Rooms           (color rooms)",
  "Color Room Name       (color room_name)",
  "Color Spellcasting    (color spells)",
  "Color Communications  (color communications)",
  "Color Shouts          (color shouts)",
  "Color Logs (Imm Only) (color logs)",
  "Color Codes(Imm Only) (color codes)",
};

const char * const auto_name[MAX_AUTO] =
{
  "Spam Control",
  "Auto Eat",
  "Auto Kill",
  "Auto Loot Money",
  "Auto Loot All",
  "No Harm PCs",
  "No Shouts",
  "Auto AFK",
  "Auto Split",
  "Auto Success",
  "Auto Open Pouches",
  "Auto Faction Join",
  "Auto Dissect",
  "Auto Engage",
  "Auto Engage All",
  "Auto Hunt",
  "Auto No Spells",
  "Auto Half Spells",
  "Auto Limbs",
  "Tips",
  "Auto Trophy",
  "PG13",
  "No Hero Sprites",
  "No Tells",
  "Auto Group"
};

// disc_num, class_nums, practice
// COSMO CLASS MARKER
const struct disc_names_data discNames[MAX_DISCS] =
{
  {DISC_MAGE, CLASS_MAGE, "mage abilities", 
   "Mage Abilities"},   // 0 
  {DISC_AIR, CLASS_MAGE, "air magic", 
   "Air Magic"},         // 1
  {DISC_ALCHEMY, CLASS_MAGE, "alchemy", 
   "Alchemy"},
  {DISC_EARTH, CLASS_MAGE, "earth magic", 
   "Earth Magic"},
  {DISC_FIRE, CLASS_MAGE, "fire magic", 
   "Fire Magic"},
  {DISC_SORCERY, CLASS_MAGE, "sorcery", 
   "Sorcery"},        // 5
  {DISC_SPIRIT, CLASS_MAGE, "spirit magic", 
   "Spirit Magic"},
  {DISC_WATER, CLASS_MAGE, "water magic", 
   "Water Magic"},
  {DISC_CLERIC, CLASS_CLERIC, "clerical abilities", 
   "Clerical Abilities"},
  {DISC_WRATH, CLASS_CLERIC, "wrath of the deities", 
   "Wrath of the Deities"},
  {DISC_AFFLICTIONS, CLASS_CLERIC, "afflictions", 
   "Afflictions"},   // 10
  {DISC_AEGIS, CLASS_CLERIC, "aegis", 
   "Aegis of the Deities"},
  {DISC_CURES, CLASS_CLERIC, "cures", 
   "Cures"},
  {DISC_HAND_OF_GOD, CLASS_CLERIC, "hand of the deities", 
   "Hand of the Deities"},
  {DISC_WARRIOR, CLASS_WARRIOR, "warrior abilities", 
   "Warrior Abilities"},
  {DISC_DUELING, CLASS_WARRIOR, "dueling", 
   "Dueling Skills"},  //15
  {DISC_BRAWLING, CLASS_WARRIOR, "brawling skills", 
   "Brawling Skills"},
  {DISC_SOLDIERING, CLASS_WARRIOR, "soldiering skills", 
   "Soldiering Skills"},
  {DISC_BLACKSMITHING, CLASS_WARRIOR, "blacksmithing", 
   "Blacksmithing Skills"},
  {DISC_RANGER, CLASS_RANGER, "ranger abilities", 
   "Ranger Abilities"},
  {DISC_BOGUS1, 0, "bogus1", 
   "unused"},  //20
  {DISC_SHAMAN_ARMADILLO, CLASS_SHAMAN, "armadillo abilities", 
   "Armadillo Abilities"},
  {DISC_ANIMAL, CLASS_RANGER, "animals", 
   "Animal Abilities"},
  {DISC_PLANTS, CLASS_RANGER, "plants", 
   "Plant Abilities"},
  {DISC_BOGUS2, 0, "bogus2", 
   "unused"}, 
  {DISC_DEIKHAN, CLASS_DEIKHAN, "deikhan abilities", 
   "Deikhan Abilities"},   // 25
  {DISC_DEIKHAN_FIGHT, CLASS_DEIKHAN, "fighting skills", 
   "Fighting Skills"},
  {DISC_MOUNTED, CLASS_DEIKHAN, "mounted skills", 
   "Mounted Skills"},
  {DISC_DEIKHAN_AEGIS, CLASS_DEIKHAN, "aegis", 
   "Aegis of the Deities"},
  {DISC_DEIKHAN_CURES, CLASS_DEIKHAN, "cures", 
   "Cures"},
  {DISC_DEIKHAN_WRATH, CLASS_DEIKHAN, "wrath of the deities", 
   "Wrath of the Deities"},  // 30
  {DISC_MONK, CLASS_MONK, "monk abilities", 
   "Monk Abilities"},
  {DISC_MEDITATION_MONK, CLASS_MONK, "meditation/internal abilities", 
   "Meditation/Internal Abilities"},
  {DISC_LEVERAGE, CLASS_MONK, "balance and leverage skill", 
   "Balanced and Leverage Skill"},
  {DISC_MINDBODY, CLASS_MONK, "mind and body control", 
   "Mind and Body Control"},
  {DISC_FOCUSED_ATTACKS, CLASS_MONK, "focused attacks", 
   "Focused Attacks"},
  {DISC_BAREHAND, CLASS_MONK, "barehand", 
   "Barehand Specialization"},
  {DISC_THIEF, CLASS_THIEF, "thief abilities", 
   "Thief Abilities"},          // 40
  {DISC_THIEF_FIGHT, CLASS_THIEF, "fighting skills",
   "Fighting Skills"},
  {DISC_MURDER, CLASS_THIEF, "murder",
   "Murder"},
  {DISC_LOOTING, CLASS_THIEF, "looting",
   "Looting Skills"},
  {DISC_POISONS, CLASS_THIEF, "poisons",
   "Poisons"},
  {DISC_STEALTH, CLASS_THIEF, "stealth",
   "Stealth"},               // 45
  {DISC_TRAPS, CLASS_THIEF, "traps",
   "Traps"},
  {DISC_SHAMAN, CLASS_SHAMAN, "shaman abilities",
   "Shaman Abilities"},
  {DISC_SHAMAN_FROG, CLASS_SHAMAN, "frog abilities",
   "Frog Abilities"},
  {DISC_SHAMAN_ALCHEMY, CLASS_SHAMAN, "alchemy",
   "Alchemy Abilities"},
  {DISC_SHAMAN_SKUNK, CLASS_SHAMAN, "skunk abilities",
   "Skunk Abilities"},       // 50
  {DISC_SHAMAN_SPIDER, CLASS_SHAMAN, "spider abilities",
   "Spider Abilities"},
  {DISC_SHAMAN_CONTROL, CLASS_SHAMAN, "control",
   "Control Abilities"},
  {DISC_RITUALISM, CLASS_SHAMAN, "ritualism",
   "Ritualism"},
  {DISC_WIZARDRY, CLASS_MAGE, "wizardry",
   "Wizardry"},
  {DISC_FAITH, CLASS_CLERIC | CLASS_DEIKHAN, "faith",
   "Faith"},  // 55
  {DISC_SLASH, CLASS_WARRIOR | CLASS_RANGER | CLASS_THIEF | CLASS_DEIKHAN | CLASS_MONK | CLASS_CLERIC | CLASS_MAGE | CLASS_SHAMAN, "slash",
   "Slash Weapons"},
  {DISC_BLUNT, CLASS_WARRIOR | CLASS_RANGER | CLASS_THIEF | CLASS_DEIKHAN | CLASS_MONK | CLASS_CLERIC | CLASS_MAGE | CLASS_SHAMAN, "blunt",
   "Blunt Weapons"},
  {DISC_PIERCE, CLASS_WARRIOR | CLASS_RANGER | CLASS_THIEF | CLASS_DEIKHAN | CLASS_MONK | CLASS_CLERIC | CLASS_MAGE | CLASS_SHAMAN, "pierce",
   "Pierce Weapons"},
  {DISC_RANGED, CLASS_WARRIOR | CLASS_RANGER | CLASS_THIEF | CLASS_DEIKHAN | CLASS_MONK | CLASS_CLERIC | CLASS_MAGE | CLASS_SHAMAN, "ranged",
   "Ranged Weapons"},
  {DISC_COMBAT, CLASS_ALL, "combat",
   "Combat Skills"},                           // 60
  {DISC_ADVENTURING, CLASS_ALL, "adventuring",
   "Adventuring Skills"},
  {DISC_THEOLOGY, CLASS_CLERIC | CLASS_DEIKHAN, "theology",
   "Theological Learning"},
  {DISC_LORE, CLASS_MAGE, "lore",
   "Magic Lore"},
  {DISC_NATURE, CLASS_RANGER, "nature",
   "Nature"},
  {DISC_DEFENSE, CLASS_WARRIOR | CLASS_DEIKHAN | CLASS_RANGER | CLASS_MONK, "defense",
   "Defensive Abilities"},
  {DISC_PSIONICS, 0, "psionics",
   "Psionic Abilities"},
  {DISC_SHAMAN_HEALING, CLASS_SHAMAN, "healing",
   "Healing Abilities"},
  {DISC_IRON_BODY, CLASS_MONK, "iron body",
   "Iron Body"},
  {DISC_ADVANCED_ADVENTURING, CLASS_WARRIOR | CLASS_RANGER | CLASS_THIEF | CLASS_DEIKHAN | CLASS_MONK | CLASS_CLERIC | CLASS_MAGE | CLASS_SHAMAN, "advanced adventuring",
   "Advanced Adventuring"},
  {DISC_COMMONER, CLASS_COMMONER, "commoner abilities",
   "Commoner Abilities"},
};

const char* const home_terrains[MAX_HOME_TERS] =
{
  "unknown",

  // human
  "urban dweller",
  "villager",
  "plains person",
  "recluse",
  "hills person",
  "mountaineer",
  "forester",
  "mariner",

  //elf
  "urban dweller",
  "tribal villager",
  "plains elf",
  "snow elf",
  "recluse",
  "wood elf",
  "sea elf",

  // dwarf
  "urban dweller",
  "villager",
  "recluse",
  "hill dwarf",
  "mountain dwarf",

  // gnome
  "urban dweller",
  "villager",
  "hill gnome",
  "swamp gnome",

  // ogre
  "villager",
  "plains ogre",
  "hill ogre",

  // hobbit
  "urban dweller",
  "shire hobbit",
  "grasslands hobbit",
  "hill hobbit",
  "woodland hobbit",
  "maritime hobbit",

  // goblin
  "gutter goblin",
  "garrison goblin",
  "UNUSED",
  "outcast goblin",
  "UNUSED",
  "rock goblin",
  "forest goblin",
  "UNUSED",

  // gnoll
  "UNUSED",
  "tribal gnoll",
  "savanna gnoll",
  "scavenger gnoll", // UNUSED
  "ruins gnoll",
  "UNUSED",
  "jungle gnoll",
  "warf gnoll",

  // troglodyte
  "UNUSED",
  "lounge troglodyte",
  "cavern troglodyte",
  "UNUSED",
  "tunnel troglodyte",
  "labyrinth troglodyte",
  "UNUSED",
  "UNUSED",

  // orc
  "streetwise orc",
  "skirmish orc",
  "UNUSED",
  "lone orc",
  "UNUSED",
  "boulder orc",
  "deforester orc",
  "UNUSED",

  // frogman
  "UNUSED",
  "tribal bullywug",
  "lillypad bullywug",
  "muck-burrower bullywug",
  "UNUSED",
  "jungle bullywug",
  "deep-bog bullywug",
  "salt-water bullywug",

  // kalysian
  "schooler kalysian",
  "reefer kalysian",
  "UNUSED",
  "hermit kalysian",
  "UNUSED",
  "trencher kalysian",
  "kelper kalysian",
  "nomad kalysian",

  // aarakocra
  "colony aarakocra",
  "flocker aarakocra",
  "UNUSED",
  "UNUSED",
  "UNUSED",
  "cliff-nester aarakocra",
  "tree-nester aarakocra",
  "coastal aarakocra",

  // troll
  "clan troll",
  "pack troll",
  "UNUSED",
  "wandering troll",
  "cave troll",
  "deep troll",
  "UNUSED",
  "UNUSED",
};

const char * const corpse_flags[MAX_CORPSE_FLAGS] =
{
  "No-Resurrection",
  "No-Dissection",
  "No-Skinning",
};

const char * const deities[MAX_DEITIES] =
{
  "none",
  "Mezan, the father",
  "Luna, the lifegiver",
  "Sa'Sukey, the stable",
  "Sin'Sukey, the unstable",
  "Icon, the rich",
  "Elyon, the wise",
  "Jevon, the thinker",
  "Omnon, the omnipresent",
  "Menanon, the unbound",
  "Amana, the pure",
  "Malshyra, the glorious",
  "Shroud, the undying one",
  "Lesprit, the desirable",
  "Talana, the lawgiver",
  "Salurel, the proud",
};




const char *shelldesc [] =
{
  "None",                       // 0
  "10mm pistol",                // 1
  "9mm Parabellem pistol",      // 2
  ".45cal ACP pistol",          // 3
  ".50cal Action Express",      // 4
  ".44cal Magnum",              // 5
  ".32cal ACP",                 // 6
  ".50cal BMG",                 // 7
  "5.56mm NATO pistol",         // 8
  "SS190",                      // 9
  "9mm Parabellem rifle",       // 10
  ".45cal ACP rifle",           // 11
  "5.56mm rifle",               // 12
  "7.62mm rifle",               // 13
  "30cal rifle",                // 14
  "flechette",                  // 15
  "LAW rocket",                 // 16
  "lead shot",                  // 17
  "cannon ball",                // 18
};

const char *shellkeyword [] = 
{
  "None",                       // 0
  "10mmPistol",                 // 1
  "9mmPistol",                  // 2
  "45calPistol",                // 3
  "50calAE",                    // 4
  "44calMag",                   // 5
  "32calACP",                   // 6
  "50calBMG",                   // 7
  "556mmPistol",                 // 8
  "SS190",                      // 9
  "9mmRifle",                   // 10
  "45calRifle",                 // 11
  "556mmRifle",                 // 12
  "762mmRifle",                 // 13
  "30calRifle",                  // 14
  "flechette",                   // 15
  "lawrocket",
  "leadshot",                    // 17
  "cannonball",                  // 18
};






const char * const trap_effects[MAX_TRAP_EFF] =
{
  "Movement",
  "Get/Put",
  "Affect-Room",
  "North",
  "East",
  "South",
  "West",
  "Up",
  "Down",
  "Northeast",
  "Northwest",
  "Southeast",
  "Southwest",
  "Throw",
  "Thrown_armed_1",
  "Thrown_armed_2",
  "Thrown_armed_3",
};


const char * heraldcodes[] =
{
  "<1>", // "none",
  "<b>", // "blue",
  "<r>", // "red",
  "<g>", // "green",
  "<W>", // "white",
  "<p>", // "purple",
  "<c>", // "cyan",
  "<o>", // "orange",
  "<Y>", // "yellow",
  "<k>", // "gray",
  "<R>", // "boldred",
  "<G>", // "boldgreen",
  "<B>", // "boldblue",
  "<P>", // "boldpurple",
  "<C>", // "boldcyan",
  "<1>"  // "\n"
};


const char * heraldcolors[] =
{
  "none",
  "blue",
  "red",
  "green",
  "white",
  "purple",
  "cyan",
  "orange",
  "yellow",
  "gray",
  "boldred",
  "boldgreen",
  "boldblue",
  "boldpurple",
  "boldcyan",
  "\n"
};


