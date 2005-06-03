//////////////////////////////////////////////////////////////////////////
//
// SneezyMUD - All rights reserved, SneezyMUD Coding Team
//
//////////////////////////////////////////////////////////////////////////


// enum.h
// Copyright (c), SneezyMUD Development Team
// All Rights Reserved.
//
// Contains definitions of all global enums used in the mud

#ifndef __ENUM_H
#define __ENUM_H

enum aiTarg { TARGET_NONE, TARGET_SELF, TARGET_MOB, TARGET_OTHER };

enum getTypeT {
  GETNULL,
  GETALL,
  GETOBJ,
  GETALLALL,
  GETALLOBJ,
  GETOBJALL,
  GETOBJOBJ
};

enum dirTypeT {
     DIR_BOGUS = -2,
     DIR_NONE = -1,
     DIR_NORTH = 0,
     DIR_EAST,
     DIR_SOUTH,
     DIR_WEST,
     DIR_UP,
     DIR_DOWN,
     DIR_NORTHEAST,
     DIR_NORTHWEST,
     DIR_SOUTHEAST,
     DIR_SOUTHWEST,
     MAX_DIR
};
const dirTypeT MIN_DIR = dirTypeT(0);
extern dirTypeT & operator++ (dirTypeT &, int);

enum doorTypeT {
     DOOR_NONE,
     DOOR_DOOR,
     DOOR_TRAPDOOR,
     DOOR_GATE,
     DOOR_GRATE,
     DOOR_PORTCULLIS,
     DOOR_DRAWBRIDGE,
     DOOR_RUBBLE,
     DOOR_PANEL,
     DOOR_SCREEN,
     DOOR_HATCH,
     MAX_DOOR_TYPES
};

// this affects the size of some career data stats
enum attack_mode_t {
     ATTACK_NORMAL, ATTACK_DEFENSE,
     ATTACK_OFFENSE, ATTACK_BERSERK,
     MAX_ATTACK_MODE_TYPE
};
extern attack_mode_t & operator++(attack_mode_t &, int);

// Do not alter this order (insert prior to TYPE_MAX)
// tiny files for weapons use this ordering scheme
enum weaponT {
  WEAPON_TYPE_NONE,	WEAPON_TYPE_STAB, 	WEAPON_TYPE_WHIP,
  WEAPON_TYPE_SLASH,	WEAPON_TYPE_SMASH, 	WEAPON_TYPE_CLEAVE,
  WEAPON_TYPE_CRUSH,	WEAPON_TYPE_BLUDGEON, 	WEAPON_TYPE_CLAW,
  WEAPON_TYPE_BITE,	WEAPON_TYPE_STING, 	WEAPON_TYPE_PIERCE,
  WEAPON_TYPE_PUMMEL,	WEAPON_TYPE_FLAIL, 	WEAPON_TYPE_BEAT,
  WEAPON_TYPE_THRASH,	WEAPON_TYPE_THUMP, 	WEAPON_TYPE_WALLOP,
  WEAPON_TYPE_BATTER,	WEAPON_TYPE_STRIKE, 	WEAPON_TYPE_CLUB,
  WEAPON_TYPE_SLICE,	WEAPON_TYPE_POUND, 	WEAPON_TYPE_THRUST,
  WEAPON_TYPE_SPEAR,	WEAPON_TYPE_SMITE, 	WEAPON_TYPE_BEAK,
  WEAPON_TYPE_AIR,	WEAPON_TYPE_EARTH, 	WEAPON_TYPE_FIRE,
  WEAPON_TYPE_WATER,	WEAPON_TYPE_BEAR_CLAW, 	WEAPON_TYPE_SHOOT, 
  WEAPON_TYPE_MAX
};

enum positionTypeT {
     POSITION_DEAD,
     POSITION_MORTALLYW,
     POSITION_INCAP,
     POSITION_STUNNED,
     POSITION_SLEEPING,
     POSITION_RESTING,
     POSITION_SITTING,
     POSITION_ENGAGED,
     POSITION_FIGHTING,
     POSITION_CRAWLING,
     POSITION_STANDING,
     POSITION_MOUNTED,
     POSITION_FLYING
};

enum classIndT {
     MAGE_LEVEL_IND,
     CLERIC_LEVEL_IND,
     WARRIOR_LEVEL_IND,
     THIEF_LEVEL_IND,
     SHAMAN_LEVEL_IND,
     DEIKHAN_LEVEL_IND,
     MONK_LEVEL_IND,
     RANGER_LEVEL_IND,
     COMMONER_LEVEL_IND,
     UNUSED1_LEVEL_IND,
     UNUSED2_LEVEL_IND,
     MAX_SAVED_CLASSES
};
// this affects charFile size
const classIndT MAX_CLASSES = UNUSED1_LEVEL_IND;
const classIndT MIN_CLASS_IND = classIndT(0);
extern classIndT & operator++(classIndT &, int);

enum giveTypeT {
    GIVE_FLAG_DEF,
    GIVE_FLAG_IGN_DEX_TEXT,
    GIVE_FLAG_IGN_DEX_NOTEXT,
    GIVE_FLAG_DROP_ON_FAIL,
    GIVE_FLAG_SILENT_VICT,
};

enum doorIntentT {
  DOOR_INTENT_OPEN,
  DOOR_INTENT_CLOSE,
  DOOR_INTENT_LOCK,
  DOOR_INTENT_UNLOCK,
  DOOR_INTENT_LOWER,
  DOOR_INTENT_RAISE
};

enum doorUniqueT {
  DOOR_UNIQUE_DEF,
  DOOR_UNIQUE_OPEN_ONLY,
  DOOR_UNIQUE_CLOSE_ONLY,
  DOOR_UNIQUE_CLOSE_ONLY_FORCE,
  DOOR_UNIQUE_OPEN_ONLY_FORCE
};

enum handTypeT {
  HAND_TYPE_SEC,
  HAND_TYPE_PRIM,
  HAND_TYPE_BOTH
};

/* sex */
enum sexTypeT {
     SEX_NEUTER,
     SEX_MALE,
     SEX_FEMALE,
};

enum stopFollowerT {
  STOP_FOLLOWER_DEFAULT,
  STOP_FOLLOWER_CHAR_VICT,
  STOP_FOLLOWER_CHAR_NOTVICT,
  STOP_FOLLOWER_SILENT
};

// failures
enum critFailT {
     CRIT_F_NONE,
     CRIT_F_HITSELF,
     CRIT_F_HITOTHER
};

// success
enum critSuccT {
     CRIT_S_NONE,
     CRIT_S_DOUBLE,
     CRIT_S_TRIPLE,
     CRIT_S_KILL
};

enum stopCastT {
  STOP_CAST_NONE,
  STOP_CAST_NOT_AROUND,
  STOP_CAST_DEATH,
  STOP_CAST_GENERIC,
  STOP_CAST_LOCATION,
};

enum armorLevT {
  ARMOR_LEV_REAL,
  ARMOR_LEV_AC,
  ARMOR_LEV_STR
};

enum wizardryLevelT {
  WIZ_LEV_NONE = -1,
  WIZ_LEV_COMP_PRIM_OTHER_FREE = 0,
  WIZ_LEV_COMP_EITHER_OTHER_FREE,
  WIZ_LEV_COMP_EITHER,
  WIZ_LEV_COMP_INV,
  WIZ_LEV_NO_GESTURES,
  WIZ_LEV_NO_MANTRA,
  WIZ_LEV_COMP_BELT,
  WIZ_LEV_COMP_NECK,
  WIZ_LEV_COMP_WRIST,
  WIZ_LEV_MAXED
};

enum ritualismLevelT {
  RIT_LEV_NONE = -1,
  RIT_LEV_COMP_PRIM_OTHER_FREE = 0,
  RIT_LEV_COMP_EITHER_OTHER_FREE,
  RIT_LEV_COMP_EITHER,
  RIT_LEV_COMP_INV,
  RIT_LEV_NO_GESTURES,
  RIT_LEV_NO_MANTRA,
  RIT_LEV_COMP_BELT,
  RIT_LEV_COMP_NECK,
  RIT_LEV_COMP_WRIST,
  RIT_LEV_MAXED
};

enum devotionLevelT {
  DEV_LEV_NONE = -1,
  DEV_LEV_SYMB_PRIM_OTHER_FREE = 0,
  DEV_LEV_SYMB_EITHER_OTHER_FREE,
  DEV_LEV_SYMB_PRIM_OTHER_EQUIP,
  DEV_LEV_SYMB_EITHER_OTHER_EQUIP,
  DEV_LEV_SYMB_NECK,
  DEV_LEV_NO_GESTURES,
  DEV_LEV_NO_MANTRA,
  DEV_LEV_MAXED
};

enum showModeT {
  SHOW_MODE_DESC_PLUS,        // getDescr() + glow/hum
  SHOW_MODE_SHORT_PLUS,       // getName() + glow/hum
  SHOW_MODE_SHORT_PLUS_INV,   // getName() + glow/hum + inventory
  SHOW_MODE_SHORT,            // getName()
  SHOW_MODE_TYPE,             // showMe()
  SHOW_MODE_PLUS              // glow/hum
};

enum wearKeyT {
  WEAR_KEY_NONE,
  WEAR_KEY_FINGER,
  WEAR_KEY_NECK,
  WEAR_KEY_BODY,
  WEAR_KEY_HEAD,
  WEAR_KEY_LEGS,
  WEAR_KEY_FEET,
  WEAR_KEY_HANDS,
  WEAR_KEY_ARMS,
  WEAR_KEY_BACK,
  WEAR_KEY_WAISTE,
  WEAR_KEY_WRIST,
  WEAR_KEY_HOLD,
  WEAR_KEY_HOLD_R,
  WEAR_KEY_HOLD_L
};

enum applyTypeT {
     APPLY_NONE,
     APPLY_STR,
     APPLY_INT,
     APPLY_WIS,
     APPLY_DEX,
     APPLY_CON,
     APPLY_KAR,
     APPLY_SEX,
     APPLY_AGE,
     APPLY_CHAR_HEIGHT,
     APPLY_CHAR_WEIGHT,
     APPLY_ARMOR,
     APPLY_HIT,
     APPLY_MANA,
     APPLY_MOVE,
     APPLY_HITROLL,
     APPLY_DAMROLL,
     APPLY_HITNDAM,
     APPLY_IMMUNITY,
     APPLY_SPELL,
     APPLY_SPELL_EFFECT,
     APPLY_LIGHT,
     APPLY_NOISE,
     APPLY_CAN_BE_SEEN,
     APPLY_VISION,
     APPLY_PROTECTION,
     APPLY_BRA,
     APPLY_AGI,
     APPLY_FOC,
     APPLY_SPE,
     APPLY_PER,
     APPLY_CHA,
     APPLY_DISCIPLINE,
     APPLY_SPELL_HITROLL,
     APPLY_CURRENT_HIT,
     MAX_APPLY_TYPES
};
const applyTypeT MIN_APPLY = applyTypeT(0);
extern applyTypeT & operator++(applyTypeT &, int);

enum newFolTypeT {
  FOL_CHARM,
  FOL_ZOMBIE,
  FOL_PET
};

/* Predifined  conditions */
// Do not alter order, saved in charFile
// Do not increase in size, saved as array
enum condTypeT {
       DRUNK,    //        = 0;
       FULL,    //         = 1;
       THIRST,    //       = 2;
       PEE,
       POOP,
       MAX_COND_TYPE
};
const condTypeT MIN_COND = DRUNK;
extern condTypeT & operator++(condTypeT &c);

// Do not alter order, saved in charFile
enum territoryT {
  HOME_TER_NONE,
  HOME_TER_HUMAN_URBAN,         HOME_TER_HUMAN_VILLAGER,
  HOME_TER_HUMAN_PLAINS,        HOME_TER_HUMAN_RECLUSE,
  HOME_TER_HUMAN_HILL,          HOME_TER_HUMAN_MOUNTAIN,
  HOME_TER_HUMAN_FOREST,        HOME_TER_HUMAN_MARINER,
  HOME_TER_ELF_URBAN,           HOME_TER_ELF_TRIBE,
  HOME_TER_ELF_PLAINS,          HOME_TER_ELF_SNOW,
  HOME_TER_ELF_RECLUSE,
  HOME_TER_ELF_WOOD,            HOME_TER_ELF_SEA,
  HOME_TER_DWARF_URBAN,         HOME_TER_DWARF_VILLAGER,
  HOME_TER_DWARF_RECLUSE,
  HOME_TER_DWARF_HILL,          HOME_TER_DWARF_MOUNTAIN,
  HOME_TER_GNOME_URBAN,         HOME_TER_GNOME_VILLAGER,
  HOME_TER_GNOME_HILL,          HOME_TER_GNOME_SWAMP,
  HOME_TER_OGRE_VILLAGER,       HOME_TER_OGRE_PLAINS,
  HOME_TER_OGRE_HILL,
  HOME_TER_HOBBIT_URBAN,        HOME_TER_HOBBIT_SHIRE,
  HOME_TER_HOBBIT_GRASSLANDS,   HOME_TER_HOBBIT_HILL,
  HOME_TER_HOBBIT_WOODLAND,     HOME_TER_HOBBIT_MARITIME,
  MAX_HOME_TERS
};
extern territoryT & operator++(territoryT &c, int);


enum moneyTypeT {
     GOLD_XFER,
     GOLD_INCOME,
     GOLD_REPAIR,
     GOLD_SHOP,
     GOLD_COMM,
     GOLD_HOSPITAL,
     GOLD_GAMBLE,
     GOLD_RENT,
     GOLD_DUMP,
     GOLD_TITHE,
     GOLD_SHOP_SYMBOL,
     GOLD_SHOP_WEAPON,
     GOLD_SHOP_ARMOR,
     GOLD_SHOP_PET,
     GOLD_SHOP_FOOD,
     GOLD_SHOP_COMPONENTS,
     GOLD_SHOP_RESPONSES,
     MAX_MONEY_TYPE
};

enum polyTypeT {
  POLY_TYPE_NONE,
  POLY_TYPE_SWITCH,
  POLY_TYPE_DISGUISE,
  POLY_TYPE_SHAPESHIFT,
  POLY_TYPE_POLYMORPH
};

enum learnUnusualTypeT {
  LEARN_UNUSUAL_NONE,
  LEARN_UNUSUAL_PROFICIENCY,
  LEARN_UNUSUAL_NORM_LEARN,
  LEARN_UNUSUAL_FORCED_LEARN
};

enum avgDurT {
  AVG_DUR_NO = false,
  AVG_DUR_YES = true
};

enum avgEffT {
  AVG_EFF_NO = false,
  AVG_EFF_YES = true
};

enum exactTypeT {
  EXACT_NO = false,
  EXACT_YES = true
};

enum multipleTypeT {
  MULTIPLE_NO = false,
  MULTIPLE_YES = true
};

enum tinyfileTypeT {
  TINYFILE_NO = false,
  TINYFILE_YES = true
};

enum infraTypeT {
  INFRA_NO = false,
  INFRA_YES = true
};

enum visibleTypeT {
  VISIBLE_NO = false,
  VISIBLE_YES = true
};

enum silentTypeT {
  SILENT_NO = false,
  SILENT_YES = true
};

enum saveTypeT {
  SAVE_NO = false,
  SAVE_YES = true
};

enum checkImmunityT {
  CHECK_IMMUNITY_NO = false,
  CHECK_IMMUNITY_YES = true
};

enum concatT {
  CONCAT_NO = false,
  CONCAT_YES = true
};

enum allowReplaceT {
  ALLOWREP_NO = false,
  ALLOWREP_YES = true
};

enum showNowT {
  SHOWNOW_NO = false,
  SHOWNOW_YES = true
};

enum showMeT {
  DONT_SHOW_ME = false,
  SHOW_ME = true
};

enum showRoomT {
  DONT_SHOW_ROOM = false,
  SHOW_ROOM = true
};

enum setRemT {
  REMOVE_TYPE = false,
  SET_TYPE = true
};

enum castTypeT {
  CAST_PRAYER = false,
  CAST_SPELL = true
};

enum primaryTypeT {
  HAND_SECONDARY = false,
  HAND_PRIMARY = true,
};

enum primLegT {
  LEG_SECONDARY = 0,
  LEG_PRIMARY = 1,
  LEG_SECONDARY_BACK = 2,
  LEG_PRIMARY_BACK = 3
};

enum checkOnlyT {
  CHECK_ONLY_NO = false,
  CHECK_ONLY_YES = true
};

enum restoreTypeT {
  RESTORE_FULL = false,
  RESTORE_PARTIAL = true
};

enum depreciationTypeT {
  DEPRECIATION_NO = false,
  DEPRECIATION_YES = true
};

enum immortalTypeT {
  IMMORTAL_NO = false,
  IMMORTAL_YES = true
};

enum safeTypeT {
  SAFE_NO = false,
  SAFE_YES = true
};

enum forceTypeT {
  FORCE_NO = false,
  FORCE_YES = true
};

enum dropNukeT {
  DROP_IN_ROOM = false,
  NUKE_ITEMS = true
};

// New object manipulation types.
enum tObjectManipT
{
  OBJMAN_NULL,  // drop
  OBJMAN_NONE,  // drop <object>
  OBJMAN_ALL,   // drop all
  OBJMAN_ALLT,  // drop all.<object>
  OBJMAN_FIT,   // drop all.fit
  OBJMAN_NOFIT, // drop all.nofit
  OBJMAN_TYPE,  // drop all.<type[type == 'component'/'light']>
  OBJMAN_MAX
};

enum logTypeT
{
  LOG_SILENT  = -2, // Log is recoreded but not echoed to immortals (anti-spam)
  LOG_NONE    = -1, // Empty
  LOG_MISC    =  0, // Anything not yet defined below
  LOG_LOW     =  1, // LOW Errors
  LOG_FILE    =  2, // File io Errors
  LOG_BUG     =  3, // 'Bugs' and other such reports
  LOG_PROC    =  4, // Errors regarding mob/obj/room procs
  LOG_PIO     =  5, // Player Login/Logout reports
  LOG_IIO     =  6, // Immortal Login/Logout 'additives'
  LOG_CLIENT  =  7, // Various errors associated with the SneezyMUD Client.
  LOG_COMBAT  =  8, // Various errors associated with the combat code.
  LOG_CHEAT   =  9, // Various logs associated with the cheating code.
  LOG_FACT    = 10, // Various Faction Stuff
  LOG_DB      = 11, // Database stuff

  LOG_MOB     = 15, // Errors in Mobiles not yet defined below
  LOG_MOB_AI  = 16, // Errors in Mobile Logic
  LOG_MOB_RS  = 17, // Errors in Mobile Response Scripts

  LOG_OBJ     = 18, // Errors in Objects not yet defined below

  LOG_EDIT    = 21, // Various 'edit' errors

  LOG_MAX     = 23, // This is here to prevent unwarrented use of the belows.

  LOG_BATOPR  = 24, // Batopr only logs
  LOG_BRUTIUS = 25, // Brutius only logs
  LOG_COSMO   = 26, // Cosmo only logs
  LOG_MAROR   = 27, // Maror only logs
  LOG_PEEL    = 28,  // Peel only logs
  LOG_LAPSOS  = 29, // Lapsos only logs
  LOG_DASH    = 30, // Dash only
  LOG_ANGUS   = 31,  // Angus only
  LOG_JESUS   = 23  // Jesus only
};

enum checkFallingT
{
  CHECK_FALL_NO = 0,
  CHECK_FALL_YES
};

enum walkPathT
{
  WALK_PATH_END = 0,
  WALK_PATH_MOVED,
  WALK_PATH_LOST
};

#endif  // __ENUM_H inclusion sandwich
