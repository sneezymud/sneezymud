//////////////////////////////////////////////////////////////////////////
//
// SneezyMUD - All rights reserved, SneezyMUD Coding Team
//
//////////////////////////////////////////////////////////////////////////


#include "extern.h"
#include "obj_money.h"

int mapApplyToFile(applyTypeT att)
{
  switch (att) {
    case APPLY_NONE:
      return 0;
    case APPLY_STR:
      return 1;
    case APPLY_INT:
      return 2;
    case APPLY_WIS:
      return 3;
    case APPLY_DEX:
      return 4;
    case APPLY_CON:
      return 5;
    case APPLY_KAR:
      return 6;
    case APPLY_SEX:
      return 7;
    case APPLY_AGE:
      return 8;
    case APPLY_CHAR_HEIGHT:
      return 9;
    case APPLY_CHAR_WEIGHT:
      return 10;
    case APPLY_ARMOR:
      return 11;
    case APPLY_HIT:
      return 12;
    case APPLY_MANA:
      return 13;
    case APPLY_MOVE:
      return 14;
    case APPLY_HITROLL:
      return 18;
    case APPLY_DAMROLL:
      return 19;
    case APPLY_HITNDAM:
      return 20;
    case APPLY_IMMUNITY:
      return 21;
    case APPLY_SPELL:
      return 23;
    case APPLY_SPELL_EFFECT:
      return 24;
    case APPLY_LIGHT:
      return 27;
    case APPLY_NOISE:
      return 28;
    case APPLY_CAN_BE_SEEN:
      return 29;
    case APPLY_VISION:
      return 30;
    case APPLY_PROTECTION:
      return 31;
    case APPLY_BRA:
      return 32;
    case APPLY_AGI:
      return 33;
    case APPLY_FOC:
      return 34;
    case APPLY_SPE:
      return 35;
    case APPLY_PER:
      return 36;
    case APPLY_CHA:
      return 37;
    case APPLY_DISCIPLINE:
      return 38;
    case APPLY_SPELL_HITROLL:
      return 39;
    case APPLY_CURRENT_HIT:
      return 40;
    case APPLY_CRIT_FREQUENCY:
      return 41;
    case APPLY_GARBLE:
      return 42;
    case MAX_APPLY_TYPES:
      break;
  }
  vlogf(LOG_BUG, format("Unknown apply in mapApplyToFile (%d)") %  att);
  return 0;
}

applyTypeT mapFileToApply(int att)
{
  switch (att) {
    case 0:
    case 15:
    case 16:
    case 17:
    case 22:
    case 25:
    case 26:
      return APPLY_NONE;
    case 1:
      return APPLY_STR;
    case 2:
      return APPLY_INT;
    case 3:
      return APPLY_WIS;
    case 4:
      return APPLY_DEX;
    case 5:
      return APPLY_CON;
    case 6:
      return APPLY_KAR;
    case 7:
      return APPLY_SEX;
    case 8:
      return APPLY_AGE;
    case 9:
      return APPLY_CHAR_HEIGHT;
    case 10:
      return APPLY_CHAR_WEIGHT;
    case 11:
      return APPLY_ARMOR;
    case 12:
      return APPLY_HIT;
    case 13:
      return APPLY_MANA;
    case 14:
      return APPLY_MOVE;
    case 18:
      return APPLY_HITROLL;
    case 19:
      return APPLY_DAMROLL;
    case 20:
      return APPLY_HITNDAM;
    case 21:
      return APPLY_IMMUNITY;
    case 23:
      return APPLY_SPELL;
    case 24:
      return APPLY_SPELL_EFFECT;
    case 27:
      return APPLY_LIGHT;
    case 28:
      return APPLY_NOISE;
    case 29:
      return APPLY_CAN_BE_SEEN;
    case 30:
      return APPLY_VISION;
    case 31:
      return APPLY_PROTECTION;
    case 32:
      return APPLY_BRA;
    case 33:
      return APPLY_AGI;
    case 34:
      return APPLY_FOC;
    case 35:
      return APPLY_SPE;
    case 36:
      return APPLY_PER;
    case 37:
      return APPLY_CHA;
    case 38:
      return APPLY_DISCIPLINE;
    case 39:
      return APPLY_SPELL_HITROLL;
    case 40:
      return APPLY_CURRENT_HIT;
    case 41:
      return APPLY_CRIT_FREQUENCY;
    case 42:
      return APPLY_GARBLE;
    default:
      break;
  }
  vlogf(LOG_BUG, format("Unknown apply in mapFileToApply. (%d)") %  att);
  return APPLY_NONE;
}

currencyTypeT & operator++(currencyTypeT &c, int)
{
  return c = (c == MAX_CURRENCY) ? CURRENCY_GRIMHAVEN : currencyTypeT(c+1);
}

territoryT & operator++(territoryT &c, int)
{
  return c = (c == MAX_HOME_TERS) ? HOME_TER_NONE : territoryT(c+1);
}

applyTypeT & operator++(applyTypeT &c, int)
{
  return c = (c == MAX_APPLY_TYPES) ? MIN_APPLY : applyTypeT(c+1);
}

attack_mode_t & operator++(attack_mode_t &c, int)
{
  return c = (c == MAX_ATTACK_MODE_TYPE) ? ATTACK_NORMAL : attack_mode_t(c+1);
}

condTypeT & operator++(condTypeT &c)
{
  return c = (c == MAX_COND_TYPE) ? MIN_COND : condTypeT(c+1);
}

liqTypeT & operator++(liqTypeT &c, int)
{
  return c = (c == MAX_DRINK_TYPES) ? MIN_DRINK_TYPES : liqTypeT(c+1);
}

immuneTypeT & operator++(immuneTypeT &c, int)
{
  return c = (c == MAX_IMMUNES) ? IMMUNE_HEAT : immuneTypeT(c+1);
}

cmdTypeT & operator++(cmdTypeT &c, int)
{
  return c = (c == MAX_CMD_LIST) ? MIN_CMD : cmdTypeT(c+1);
}

int mapDiscToFile(discNumT dn)
{
  switch (dn) {
    case DISC_MAGE:
      return 0;
    case DISC_AIR:
      return 1;
    case DISC_ALCHEMY:
      return 2;
    case DISC_EARTH:
      return 3;
    case DISC_FIRE:
      return 4;
    case DISC_SORCERY:
      return 5;
    case DISC_SPIRIT:
      return 6;
    case DISC_WATER:
      return 7;
    case DISC_CLERIC:
      return 8;
    case DISC_WRATH:
      return 9;
    case DISC_AFFLICTIONS:
      return 10;
    case DISC_AEGIS:
      return 11;
    case DISC_CURES:
      return 12;
    case DISC_HAND_OF_GOD:
      return 13;
    case DISC_WARRIOR:
      return 14;
    case DISC_DUELING:
      return 15;
    case DISC_BRAWLING:
      return 16;
    case DISC_SOLDIERING:
      return 17;
    case DISC_BLACKSMITHING:
      return 18;
    case DISC_RANGER:
      return 19;
    case DISC_BOGUS1:
      return 20;
    case DISC_SHAMAN_ARMADILLO:
      return 21;
    case DISC_ANIMAL:
      return 22;
    case DISC_PLANTS:
      return 23;
    case DISC_BOGUS2:
      return 24;
    case DISC_DEIKHAN:
      return 25;
    case DISC_DEIKHAN_FIGHT:
      return 26;
    case DISC_MOUNTED:
      return 27;
    case DISC_DEIKHAN_AEGIS:
      return 28;
    case DISC_DEIKHAN_CURES:
      return 29;
    case DISC_DEIKHAN_WRATH:
      return 30;
    case DISC_MONK:
      return 31;
    case DISC_MEDITATION_MONK:
      return 32;
    case DISC_LEVERAGE:
      return 33;
    case DISC_MINDBODY:
      return 34;
    case DISC_FOCUSED_ATTACKS:
      return 37;
    case DISC_BAREHAND:
      return 39;
    case DISC_THIEF:
      return 40;
    case DISC_THIEF_FIGHT:
      return 41;
    case DISC_MURDER:
      return 42;
    case DISC_LOOTING:
      return 43;
    case DISC_POISONS:
      return 44;
    case DISC_STEALTH:
      return 45;
    case DISC_TRAPS:
      return 46;
    case DISC_SHAMAN:
      return 47;
    case DISC_SHAMAN_FROG:
      return 48;
    case DISC_SHAMAN_ALCHEMY:
      return 49;
    case DISC_SHAMAN_SKUNK:
      return 50;
    case DISC_SHAMAN_SPIDER:
      return 51;
    case DISC_SHAMAN_CONTROL:
      return 52;
    case DISC_RITUALISM:
      return 53;
    case DISC_WIZARDRY:
      return 54;
    case DISC_FAITH:
      return 55;
    case DISC_SLASH:
      return 56;
    case DISC_BLUNT:
      return 57;
    case DISC_PIERCE:
      return 58;
    case DISC_RANGED:
      return 59;
    case DISC_COMBAT:
      return 60;
    case DISC_ADVENTURING:
      return 61;
    case DISC_THEOLOGY:
      return 62;
    case DISC_LORE:
      return 63;
    case DISC_NATURE:
      return 64;
    case DISC_DEFENSE:
      return 65;
    case DISC_PSIONICS:
      return 66;
    case DISC_SHAMAN_HEALING:
      return 67;
    case DISC_IRON_BODY:
      return 68;
    case DISC_ADVANCED_ADVENTURING:
      return 69;
    case DISC_COMMONER:
      return 70;
    case MAX_DISCS:
    case MAX_SAVED_DISCS:
    case DISC_NONE:
      break;
  }
  vlogf(LOG_BUG, "Bad value to mapDiscToFile");
  return -1;
}

discNumT mapFileToDisc(int num)
{
  switch (num) {
    case 0:
      return DISC_MAGE;
    case 1:
      return DISC_AIR;
    case 2:
      return DISC_ALCHEMY;
    case 3:
      return DISC_EARTH;
    case 4:
      return DISC_FIRE;
    case 5:
      return DISC_SORCERY;
    case 6:
      return DISC_SPIRIT;
    case 7:
      return DISC_WATER;
    case 8:
      return DISC_CLERIC;
    case 9:
      return DISC_WRATH;
    case 10:
      return DISC_AFFLICTIONS;
    case 11:
      return DISC_AEGIS;
    case 12:
      return DISC_CURES;
    case 13:
      return DISC_HAND_OF_GOD;
    case 14:
      return DISC_WARRIOR;
    case 15:
      return DISC_DUELING;
    case 16:
      return DISC_BRAWLING;
    case 17:
      return DISC_SOLDIERING;
    case 18:
      return DISC_BLACKSMITHING;
    case 19:
      return DISC_RANGER;
    case 20:
      return DISC_BOGUS1;
    case 21:
      return DISC_SHAMAN_ARMADILLO;
    case 22:
      return DISC_ANIMAL;
    case 23:
      return DISC_PLANTS;
    case 24:
      return DISC_BOGUS2;
    case 25:
      return DISC_DEIKHAN;
    case 26:
      return DISC_DEIKHAN_FIGHT;
    case 27:
      return DISC_MOUNTED;
    case 28:
      return DISC_DEIKHAN_AEGIS;
    case 29:
      return DISC_DEIKHAN_CURES;
    case 30:
      return DISC_DEIKHAN_WRATH;
    case 31:
      return DISC_MONK;
    case 32:
      return DISC_MEDITATION_MONK;
    case 33:
      return DISC_LEVERAGE;
    case 34:
      return DISC_MINDBODY;
    case 37:
      return DISC_FOCUSED_ATTACKS;
    case 39:
      return DISC_BAREHAND;
    case 40:
      return DISC_THIEF;
    case 41:
      return DISC_THIEF_FIGHT;
    case 42:
      return DISC_MURDER;
    case 43:
      return DISC_LOOTING;
    case 44:
      return DISC_POISONS;
    case 45:
      return DISC_STEALTH;
    case 46:
      return DISC_TRAPS;
    case 47:
      return DISC_SHAMAN;
    case 48:
      return DISC_SHAMAN_FROG;
    case 49:
      return DISC_SHAMAN_ALCHEMY;
    case 50:
      return DISC_SHAMAN_SKUNK;
    case 51:
      return DISC_SHAMAN_SPIDER;
    case 52:
      return DISC_SHAMAN_CONTROL;
    case 53:
      return DISC_RITUALISM;
    case 54:
      return DISC_WIZARDRY;
    case 55:
      return DISC_FAITH;
    case 56:
      return DISC_SLASH;
    case 57:
      return DISC_BLUNT;
    case 58:
      return DISC_PIERCE;
    case 59:
      return DISC_RANGED;
    case 60:
      return DISC_COMBAT;
    case 61:
      return DISC_ADVENTURING;
    case 62:
      return DISC_THEOLOGY;
    case 63:
      return DISC_LORE;
    case 64:
      return DISC_NATURE;
    case 65:
      return DISC_DEFENSE;
    case 66:
      return DISC_PSIONICS;
    case 67:
      return DISC_SHAMAN_HEALING;
    case 68:
      return DISC_IRON_BODY;
    case 69:
      return DISC_ADVANCED_ADVENTURING;
  }
  vlogf(LOG_BUG, "Bad value to mapFileToDisc");
  return DISC_NONE;
}

discNumT & operator++(discNumT &c, int)
{
  return c = (c == MAX_DISCS) ? MIN_DISC : discNumT(c+1);
}

factionTypeT & operator++(factionTypeT &c, int)
{
  return c = (c == ABS_MAX_FACTION) ? MIN_FACTION : factionTypeT(c+1);
}

wearSlotT & operator++(wearSlotT &c, int)
{
  return c = (c == MAX_WEAR) ? MIN_WEAR : wearSlotT(c+1);
}

dirTypeT & operator++(dirTypeT &c, int)
{
   return c = (c == MAX_DIR) ? MIN_DIR : dirTypeT(c+1);
}

dirTypeT getDirFromCmd(cmdTypeT cmd)
{
  switch (cmd) {
    case CMD_NORTH:
      return DIR_NORTH;
    case CMD_EAST:
      return DIR_EAST;
    case CMD_SOUTH:
      return DIR_SOUTH;
    case CMD_WEST:
      return DIR_WEST;
    case CMD_UP:
      return DIR_UP;
    case CMD_DOWN:
      return DIR_DOWN;
    case CMD_NE:
      return DIR_NORTHEAST;
    case CMD_NW:
      return DIR_NORTHWEST;
    case CMD_SE:
      return DIR_SOUTHEAST;
    case CMD_SW:
      return DIR_SOUTHWEST;
    default:
      vlogf(LOG_BUG, "bad cmd to get dir from");
      return MAX_DIR;
  }
}

dirTypeT getDirFromChar(const sstring direction)
{
  sstring dirbuf;

  one_argument(direction, dirbuf);
  if (dirbuf.empty())
    return DIR_NONE;

  // KLUDGE for abbreviated directions - bat
  if(dirbuf.lower() == "northeast")
    dirbuf = "ne";
  else if (dirbuf.lower() == "northwest")
    dirbuf = "nw";
  else if (dirbuf.lower() == "southeast")
    dirbuf = "se";
  else if (dirbuf.lower() == "southwest")
    dirbuf = "sw";

  int dr = search_block(dirbuf, scandirs, false);
  if (dr == -1)
    return DIR_NONE;

  return dirTypeT(dr);
}

statTypeT & operator++(statTypeT &c, int)
{
  return c = (c == MAX_STATS) ? MIN_STAT : statTypeT(c+1);
}

sectorTypeT mapFileToSector(int num)
{
  switch (num) {
    case 0:
      return SECT_SUBARCTIC;
    case 1:
      return SECT_ARCTIC_WASTE;
    case 2:
      return SECT_ARCTIC_CITY;
    case 3:
      return SECT_ARCTIC_ROAD;
    case 4:
      return SECT_TUNDRA;
    case 5:
      return SECT_ARCTIC_MOUNTAINS;
    case 6:
      return SECT_ARCTIC_FOREST;
    case 7:
      return SECT_ARCTIC_MARSH;
    case 8:
      return SECT_ARCTIC_RIVER_SURFACE;
    case 9:
      return SECT_ICEFLOW;
    case 10:
      return SECT_COLD_BEACH;
    case 11:
      return SECT_SOLID_ICE;
    case 12:
      return SECT_ARCTIC_BUILDING;
    case 13:
      return SECT_ARCTIC_CAVE;
    case 14:
      return SECT_ARCTIC_ATMOSPHERE;
    case 15:
      return SECT_ARCTIC_CLIMBING;
    case 16:
      return SECT_ARCTIC_FOREST_ROAD;
    case 20:
      return SECT_PLAINS;
    case 21:
      return SECT_TEMPERATE_CITY;
    case 22:
      return SECT_TEMPERATE_ROAD;
    case 23:
      return SECT_GRASSLANDS;
    case 24:
      return SECT_TEMPERATE_HILLS;
    case 25:
      return SECT_TEMPERATE_MOUNTAINS;
    case 26:
      return SECT_TEMPERATE_FOREST;
    case 27:
      return SECT_TEMPERATE_SWAMP;
    case 28:
      return SECT_TEMPERATE_OCEAN;
    case 29:
      return SECT_TEMPERATE_RIVER_SURFACE;
    case 30:
      return SECT_TEMPERATE_UNDERWATER;
    case 31:
      return SECT_TEMPERATE_BEACH;
    case 32:
      return SECT_TEMPERATE_BUILDING;
    case 33:
      return SECT_TEMPERATE_CAVE;
    case 34:
      return SECT_TEMPERATE_ATMOSPHERE;
    case 35:
      return SECT_TEMPERATE_CLIMBING;
    case 36:
      return SECT_TEMPERATE_FOREST_ROAD;
    case 40:
      return SECT_DESERT;
    case 41:
      return SECT_SAVANNAH;
    case 42:
      return SECT_VELDT;
    case 43:
      return SECT_TROPICAL_CITY;
    case 44:
      return SECT_TROPICAL_ROAD;
    case 45:
      return SECT_JUNGLE;
    case 46:
      return SECT_RAINFOREST;
    case 47:
      return SECT_TROPICAL_HILLS;
    case 48:
      return SECT_TROPICAL_MOUNTAINS;
    case 49:
      return SECT_VOLCANO_LAVA;
    case 50:
      return SECT_TROPICAL_SWAMP;
    case 51:
      return SECT_TROPICAL_OCEAN;
    case 52:
      return SECT_TROPICAL_RIVER_SURFACE;
    case 53:
      return SECT_TROPICAL_UNDERWATER;
    case 54:
      return SECT_TROPICAL_BEACH;
    case 55:
      return SECT_TROPICAL_BUILDING;
    case 56:
      return SECT_TROPICAL_CAVE;
    case 57:
      return SECT_TROPICAL_ATMOSPHERE;
    case 58:
      return SECT_TROPICAL_CLIMBING;
    case 59:
      return SECT_RAINFOREST_ROAD;
    case 60:
      return SECT_ASTRAL_ETHREAL;
    case 61:
      return SECT_SOLID_ROCK;
    case 62:
      return SECT_FIRE;
    case 63:
      return SECT_INSIDE_MOB;
    case 64:
      return SECT_FIRE_ATMOSPHERE;
    case 65:
      return SECT_MAKE_FLY;
    case 66:
      return SECT_DEAD_WOODS;
    default:
      vlogf(LOG_BUG, format("Bad num (%d) in file to sector") %  num);
      return SECT_ASTRAL_ETHREAL;
  }
}

int mapSectorToFile(sectorTypeT sec)
{
  switch (sec) {
    case SECT_SUBARCTIC:
      return 0;
    case SECT_ARCTIC_WASTE:
      return 1;
    case SECT_ARCTIC_CITY:
      return 2;
    case SECT_ARCTIC_ROAD:
      return 3;
    case SECT_TUNDRA:
      return 4;
    case SECT_ARCTIC_MOUNTAINS:
      return 5;
    case SECT_ARCTIC_FOREST:
      return 6;
    case SECT_ARCTIC_MARSH:
      return 7;
    case SECT_ARCTIC_RIVER_SURFACE:
      return 8;
    case SECT_ICEFLOW:
      return 9;
    case SECT_COLD_BEACH:
      return 10;
    case SECT_SOLID_ICE:
      return 11;
    case SECT_ARCTIC_BUILDING:
      return 12;
    case SECT_ARCTIC_CAVE:
      return 13;
    case SECT_ARCTIC_ATMOSPHERE:
      return 14;
    case SECT_ARCTIC_CLIMBING:
      return 15;
    case SECT_ARCTIC_FOREST_ROAD:
      return 16;
    case SECT_PLAINS:
      return 20;
    case SECT_TEMPERATE_CITY:
      return 21;
    case SECT_TEMPERATE_ROAD:
      return 22;
    case SECT_GRASSLANDS:
      return 23;
    case SECT_TEMPERATE_HILLS:
      return 24;
    case SECT_TEMPERATE_MOUNTAINS:
      return 25;
    case SECT_TEMPERATE_FOREST:
      return 26;
    case SECT_TEMPERATE_SWAMP:
      return 27;
    case SECT_TEMPERATE_OCEAN:
      return 28;
    case SECT_TEMPERATE_RIVER_SURFACE:
      return 29;
    case SECT_TEMPERATE_UNDERWATER:
      return 30;
    case SECT_TEMPERATE_BEACH:
      return 31;
    case SECT_TEMPERATE_BUILDING:
      return 32;
    case SECT_TEMPERATE_CAVE:
      return 33;
    case SECT_TEMPERATE_ATMOSPHERE:
      return 34;
    case SECT_TEMPERATE_CLIMBING:
      return 35;
    case SECT_TEMPERATE_FOREST_ROAD:
      return 36;
    case SECT_DESERT:
      return 40;
    case SECT_SAVANNAH:
      return 41;
    case SECT_VELDT:
      return 42;
    case SECT_TROPICAL_CITY:
      return 43;
    case SECT_TROPICAL_ROAD:
      return 44;
    case SECT_JUNGLE:
      return 45;
    case SECT_RAINFOREST:
      return 46;
    case SECT_TROPICAL_HILLS:
      return 47;
    case SECT_TROPICAL_MOUNTAINS:
      return 48;
    case SECT_VOLCANO_LAVA:
      return 49;
    case SECT_TROPICAL_SWAMP:
      return 50;
    case SECT_TROPICAL_OCEAN:
      return 51;
    case SECT_TROPICAL_RIVER_SURFACE:
      return 52;
    case SECT_TROPICAL_UNDERWATER:
      return 53;
    case SECT_TROPICAL_BEACH:
      return 54;
    case SECT_TROPICAL_BUILDING:
      return 55;
    case SECT_TROPICAL_CAVE:
      return 56;
    case SECT_TROPICAL_ATMOSPHERE:
      return 57;
    case SECT_TROPICAL_CLIMBING:
      return 58;
    case SECT_RAINFOREST_ROAD:
      return 59;
    case SECT_ASTRAL_ETHREAL:
      return 60;
    case SECT_SOLID_ROCK:
      return 61;
    case SECT_FIRE:
      return 62;
    case SECT_INSIDE_MOB:
      return 63;
    case SECT_FIRE_ATMOSPHERE:
      return 64;
    case SECT_MAKE_FLY:
      return 65;
    case SECT_DEAD_WOODS:
      return 66;
    case MAX_SECTOR_TYPES:
      break;
  }
  vlogf(LOG_BUG, format("Bad sec (%d) in file to sector") %  sec);
  return -1;
}

sectorTypeT & operator++(sectorTypeT &c, int)
{
  return c = (c == MAX_SECTOR_TYPES) ? MIN_SECTOR_TYPE : sectorTypeT(c+1);
}

int mapWizPowerToFile(wizPowerT att)
{
  switch (att) {
    case POWER_WIZNET:
      return 0;
    case POWER_REDIT:
      return 1;
    case POWER_REDIT_ENABLED:
      return 2;
    case POWER_EDIT:
      return 3;
    case POWER_RLOAD:
      return 4;
    case POWER_RSAVE:
      return 5;
    case POWER_POWERS:
      return 6;
    case POWER_MEDIT:
      return 7;
    case POWER_MEDIT_LOAD_ANYWHERE:
      return 8;
    case POWER_MEDIT_IMP_POWER:
      return 9;
    case POWER_OEDIT:
      return 10;
    case POWER_OEDIT_COST:
      return 11;
    case POWER_OEDIT_APPLYS:
      return 12;
    case POWER_OEDIT_NOPROTOS:
      return 13;
    case POWER_OEDIT_IMP_POWER:
      return 14;
    case POWER_VIEW_IMM_ACCOUNTS:
      return 15;
    case POWER_STAT:
      return 16;
    case POWER_STAT_MOBILES:
      return 17;
    case POWER_STAT_OBJECT:
      return 18;
    case POWER_STAT_SKILL:
      return 19;
    case POWER_OEDIT_WEAPONS:
      return 20;
    case POWER_IMM_EVAL:
      return 21;
    case POWER_RENAME:
      return 22;
    case POWER_COLOR_LOGS:
      return 23;
    case POWER_BOARD_POLICE:
      return 24;
    case POWER_SET:
      return 25;
    case POWER_SET_IMP_POWER:
      return 26;
    case POWER_VISIBLE:
      return 27;
    case POWER_MULTIPLAY:
      return 28;
    case POWER_BUILDER:
      return 29;
    case POWER_GOD:
      return 30;
    case POWER_WIZARD:
      return 31;
    case POWER_GOTO:
      return 32;
    case POWER_GOTO_IMP_POWER:
      return 33;
    case POWER_ECHO:
      return 34;
    case POWER_TRANSFER:
      return 35;
    case POWER_FLAG:
      return 36;
    case POWER_FLAG_IMP_POWER:
      return 37;
    case POWER_FORCE:
      return 38;
    case POWER_SNOOP:
      return 39;
    case POWER_CHANGE:
      return 40;
    case POWER_TOGGLE_INVISIBILITY:
      return 41;
    case POWER_TOGGLE:
      return 42;
    case POWER_LOG:
      return 43;
    case POWER_BREATHE:
      return 44;
    case POWER_WIPE:
      return 45;
    case POWER_LOAD:
      return 46;
    case POWER_LOAD_SET:
      return 47;
    case POWER_LOAD_LIMITED:
      return 48;
    case POWER_LOAD_IMP_POWER:
      return 49;
    case POWER_SHOW:
      return 50;
    case POWER_SHOW_MOB:
      return 51;
    case POWER_SHOW_OBJ:
      return 52;
    case POWER_SHOW_TRUSTED:
      return 53;
    case POWER_PURGE:
      return 54;
    case POWER_PURGE_PC:
      return 55;
    case POWER_PURGE_ROOM:
      return 56;
    case POWER_PURGE_LINKS:
      return 57;
    case POWER_LOAD_NOPROTOS:
      return 58;
    case POWER_LONGDESC:
      return 59;
    case POWER_COMMENT:
      return 60;
    case POWER_FINDEMAIL:
      return 61;
    case POWER_CLIENTS:
      return 62;
    case POWER_LOW:
      return 63;
    case POWER_TRACEROUTE:
      return 64;
    case POWER_HOSTLOG:
      return 65;
    case POWER_ACCESS:
      return 66;
    case POWER_DEATHCHECK:
      return 67;
    case POWER_SNOWBALL:
      return 68;
    case POWER_PEE:
      return 69;
    case POWER_SWITCH:
      return 70;
    case POWER_USERS:
      return 71;
    case POWER_STEALTH:
      return 72;
    case POWER_WIZLOCK:
      return 73;
    case POWER_CUTLINK:
      return 74;
    case POWER_EGOTRIP:
      return 75;
    case POWER_WIZNET_ALWAYS:
      return 76;
    case POWER_IMMORTAL_HELP:
      return 77;
    case POWER_QUEST:
      return 78;
    case POWER_COMPARE:
      return 79;
    case POWER_SEE_COMMENTARY:
      return 80;
    case POWER_AT:
      return 81;
    case POWER_WHERE:
      return 82;
    case POWER_SYSTEM:
      return 83;
    case POWER_GAMESTATS:
      return 84;
    case POWER_HEAVEN:
      return 85;
    case POWER_ACCOUNT:
      return 86;
    case POWER_ZONEFILE_UTILITY:
      return 87;
    case POWER_INFO:
      return 88;
    case POWER_INFO_TRUSTED:
      return 89;
    case POWER_RESTORE:
      return 90;
    case POWER_RESTORE_MORTAL:
      return 91;
    case POWER_NOSHOUT:
      return 92;
    case POWER_CHECKLOG:
      return 93;
    case POWER_LOGLIST:
      return 94;
    case POWER_REPLACE:
      return 95;
    case POWER_RESIZE:
      return 96;
    case POWER_SEDIT:
      return 97;
    case POWER_SEDIT_IMP_POWER:
      return 98;
    case POWER_CRIT:
      return 99;
    case POWER_SHUTDOWN:
      return 100;
    case POWER_SLAY:
      return 101;
    case POWER_TIMESHIFT:
      return 102;
    case POWER_RESET:
      return 103;
    case POWER_REPLACE_PFILE:
      return 104;
    case POWER_IMMORTAL_OUTFIT:
      return 105;
    case POWER_SEE_FACTION_SENDS:
      return 106;
    case POWER_SETSEV:
      return 107;
    case POWER_SETSEV_IMM:
      return 108;
    case POWER_IDLED:
      return 109;
    case POWER_NO_LIMITS:
      return 110;
    case POWER_CLONE:
      return 111;
    case MAX_POWER_INDEX:
      break;
  }
  vlogf(LOG_BUG, format("Bad power (%d) in mapWizPowerToFile") %  att);
  return -1;
}

wizPowerT mapFileToWizPower(int att)
{
  switch (att) {
    case 0:
      return POWER_WIZNET;
    case 1:
      return POWER_REDIT;
    case 2:
      return POWER_REDIT_ENABLED;
    case 3:
      return POWER_EDIT;
    case 4:
      return POWER_RLOAD;
    case 5:
      return POWER_RSAVE;
    case 6:
      return POWER_POWERS;
    case 7:
      return POWER_MEDIT;
    case 8:
      return POWER_MEDIT_LOAD_ANYWHERE;
    case 9:
      return POWER_MEDIT_IMP_POWER;
    case 10:
      return POWER_OEDIT;
    case 11:
      return POWER_OEDIT_COST;
    case 12:
      return POWER_OEDIT_APPLYS;
    case 13:
      return POWER_OEDIT_NOPROTOS;
    case 14:
      return POWER_OEDIT_IMP_POWER;
    case 15:
      return POWER_VIEW_IMM_ACCOUNTS;
    case 16:
      return POWER_STAT;
    case 17:
      return POWER_STAT_MOBILES;
    case 18:
      return POWER_STAT_OBJECT;
    case 19:
      return POWER_STAT_SKILL;
    case 20:
      return POWER_OEDIT_WEAPONS;
    case 21:
      return POWER_IMM_EVAL;
    case 22:
      return POWER_RENAME;
    case 23:
      return POWER_COLOR_LOGS;
    case 24:
      return POWER_BOARD_POLICE;
    case 25:
      return POWER_SET;
    case 26:
      return POWER_SET_IMP_POWER;
    case 27:
      return POWER_VISIBLE;
    case 28:
      return POWER_MULTIPLAY;
    case 29:
      return POWER_BUILDER;
    case 30:
      return POWER_GOD;
    case 31:
      return POWER_WIZARD;
    case 32:
      return POWER_GOTO;
    case 33:
      return POWER_GOTO_IMP_POWER;
    case 34:
      return POWER_ECHO;
    case 35:
      return POWER_TRANSFER;
    case 36:
      return POWER_FLAG;
    case 37:
      return POWER_FLAG_IMP_POWER;
    case 38:
      return POWER_FORCE;
    case 39:
      return POWER_SNOOP;
    case 40:
      return POWER_CHANGE;
    case 41:
      return POWER_TOGGLE_INVISIBILITY;
    case 42:
      return POWER_TOGGLE;
    case 43:
      return POWER_LOG;
    case 44:
      return POWER_BREATHE;
    case 45:
      return POWER_WIPE;
    case 46:
      return POWER_LOAD;
    case 47:
      return POWER_LOAD_SET;
    case 48:
      return POWER_LOAD_LIMITED;
    case 49:
      return POWER_LOAD_IMP_POWER;
    case 50:
      return POWER_SHOW;
    case 51:
      return POWER_SHOW_MOB;
    case 52:
      return POWER_SHOW_OBJ;
    case 53:
      return POWER_SHOW_TRUSTED;
    case 54:
      return POWER_PURGE;
    case 55:
      return POWER_PURGE_PC;
    case 56:
      return POWER_PURGE_ROOM;
    case 57:
      return POWER_PURGE_LINKS;
    case 58:
      return POWER_LOAD_NOPROTOS;
    case 59:
      return POWER_LONGDESC;
    case 60:
      return POWER_COMMENT;
    case 61:
      return POWER_FINDEMAIL;
    case 62:
      return POWER_CLIENTS;
    case 63:
      return POWER_LOW;
    case 64:
      return POWER_TRACEROUTE;
    case 65:
      return POWER_HOSTLOG;
    case 66:
      return POWER_ACCESS;
    case 67:
      return POWER_DEATHCHECK;
    case 68:
      return POWER_SNOWBALL;
    case 69:
      return POWER_PEE;
    case 70:
      return POWER_SWITCH;
    case 71:
      return POWER_USERS;
    case 72:
      return POWER_STEALTH;
    case 73:
      return POWER_WIZLOCK;
    case 74:
      return POWER_CUTLINK;
    case 75:
      return POWER_EGOTRIP;
    case 76:
      return POWER_WIZNET_ALWAYS;
    case 77:
      return POWER_IMMORTAL_HELP;
    case 78:
      return POWER_QUEST;
    case 79:
      return POWER_COMPARE;
    case 80:
      return POWER_SEE_COMMENTARY;
    case 81:
      return POWER_AT;
    case 82:
      return POWER_WHERE;
    case 83:
      return POWER_SYSTEM;
    case 84:
      return POWER_GAMESTATS;
    case 85:
      return POWER_HEAVEN;
    case 86:
      return POWER_ACCOUNT;
    case 87:
      return POWER_ZONEFILE_UTILITY;
    case 88:
      return POWER_INFO;
    case 89:
      return POWER_INFO_TRUSTED;
    case 90:
      return POWER_RESTORE;
    case 91:
      return POWER_RESTORE_MORTAL;
    case 92:
      return POWER_NOSHOUT;
    case 93:
      return POWER_CHECKLOG;
    case 94:
      return POWER_LOGLIST;
    case 95:
      return POWER_REPLACE;
    case 96:
      return POWER_RESIZE;
    case 97:
      return POWER_SEDIT;
    case 98:
      return POWER_SEDIT_IMP_POWER;
    case 99:
      return POWER_CRIT;
    case 100:
      return POWER_SHUTDOWN;
    case 101:
      return POWER_SLAY;
    case 102:
      return POWER_TIMESHIFT;
    case 103:
      return POWER_RESET;
    case 104:
      return POWER_REPLACE_PFILE;
    case 105:
      return POWER_IMMORTAL_OUTFIT;
    case 106:
      return POWER_SEE_FACTION_SENDS;
    case 107:
      return POWER_SETSEV;
    case 108:
      return POWER_SETSEV_IMM;
    case 109:
      return POWER_IDLED;
    case 110:
      return POWER_NO_LIMITS;
    case 111:
      return POWER_CLONE;
    default:
      break;
  }
  vlogf(LOG_BUG, format("Unknown power (%d) in mapFileToWizPower") %  att);
  return MAX_POWER_INDEX;
}

wizPowerT & operator++(wizPowerT &c, int)
{
  return c = (c == MAX_POWER_INDEX) ? MIN_POWER_INDEX : wizPowerT(c+1);
}

int mapDrugToFile(drugTypeT d)
{
  switch (d) {
    case DRUG_NONE:
      return 0;
    case DRUG_PIPEWEED:
      return 1;
    case DRUG_OPIUM:
      return 2;
    case DRUG_POT:
      return 3;
    case DRUG_FROGSLIME:
      return 4;
    case MAX_DRUG:
      break;
  }
  vlogf(LOG_BUG, format("Bad drug to mapDrugToFile %d") %  d);
  return -1;
}

drugTypeT mapFileToDrug(int d)
{
  switch (d) {
    case 0:
      return DRUG_NONE;
    case 1:
      return DRUG_PIPEWEED;
    case 2:
      return DRUG_OPIUM;
    case 3:
      return DRUG_POT;
    case 4:
      return DRUG_FROGSLIME;
    default:
      break;
  }
  vlogf(LOG_BUG, format("Bad drug to mapFileToDrug %d") %  d);
  return MAX_DRUG;
}


spellNumT mapWeaponT(weaponT w) 
{
  // divorced this from TGenWeapon
  switch (w) {
    case WEAPON_TYPE_NONE:
      return TYPE_SMITE;
    case WEAPON_TYPE_STAB:
      return TYPE_STAB;
    case WEAPON_TYPE_WHIP:
      return TYPE_WHIP;
    case WEAPON_TYPE_SLASH:
      return TYPE_SLASH;
    case WEAPON_TYPE_SMASH:
      return TYPE_SMASH;
    case WEAPON_TYPE_CLEAVE:
      return TYPE_CLEAVE;
    case WEAPON_TYPE_CRUSH:
      return TYPE_CRUSH;
    case WEAPON_TYPE_BLUDGEON:
      return TYPE_BLUDGEON;
    case WEAPON_TYPE_CLAW:
      return TYPE_CLAW;
    case WEAPON_TYPE_BITE:
      return TYPE_BITE;
    case WEAPON_TYPE_STING:
      return TYPE_STING;
    case WEAPON_TYPE_PIERCE:
      return TYPE_PIERCE;
    case WEAPON_TYPE_PUMMEL:
      return TYPE_PUMMEL;
    case WEAPON_TYPE_FLAIL:
      return TYPE_FLAIL;
    case WEAPON_TYPE_BEAT:
      return TYPE_BEAT;
    case WEAPON_TYPE_THRASH:
      return TYPE_THRASH;
    case WEAPON_TYPE_THUMP:
      return TYPE_THUMP;
    case WEAPON_TYPE_WALLOP:
      return TYPE_WALLOP;
    case WEAPON_TYPE_BATTER:
      return TYPE_BATTER;
    case WEAPON_TYPE_STRIKE:
      return TYPE_STRIKE;
    case WEAPON_TYPE_CLUB:
      return TYPE_CLUB;
    case WEAPON_TYPE_SLICE:
      return TYPE_SLICE;
    case WEAPON_TYPE_POUND:
      return TYPE_POUND;
    case WEAPON_TYPE_THRUST:
      return TYPE_THRUST;
    case WEAPON_TYPE_SPEAR:
      return TYPE_SPEAR;
    case WEAPON_TYPE_SMITE:
      return TYPE_SMITE;
    case WEAPON_TYPE_BEAK:
      return TYPE_BEAK;
    case WEAPON_TYPE_AIR:
      return TYPE_AIR;
    case WEAPON_TYPE_EARTH:
      return TYPE_EARTH;
    case WEAPON_TYPE_FIRE:
      return TYPE_FIRE;
    case WEAPON_TYPE_WATER:
      return TYPE_WATER;
    case WEAPON_TYPE_BEAR_CLAW:
      return TYPE_BEAR_CLAW;
    case WEAPON_TYPE_SHOOT:
      return TYPE_SHOOT;
    case WEAPON_TYPE_CANNON:
      return TYPE_CANNON;
    case WEAPON_TYPE_SHRED:
      return TYPE_SHRED;
    default:
      return TYPE_HIT;
  }
}
