//////////////////////////////////////////////////////////////////////////
//
// SneezyMUD - All rights reserved, SneezyMUD Coding Team
//
//
//////////////////////////////////////////////////////////////////////////


#ifndef __TOGGLE_H
#define __TOGGLE_H

// Toggles

// quest toggles on individual players
typedef struct {
  const sstring name;
  int togmob;
} TOGINFO;



// stuff for global toggles

enum togTypeT {
  TOG_NONE = 0,
  TOG_SHOUTING,
  TOG_SLEEP,
  TOG_NEWBIEPK,
  TOG_GRAVITY,
  TOG_CLIENTS,
  TOG_WIZBUILD,
  TOG_MOBNAMES,
  TOG_TWINK,
  TOG_DBTIMING,
  TOG_GAMELOOP,
  TOG_DOUBLEEXP,
  TOG_TESTCODE1,
  TOG_TESTCODE2,
  TOG_TESTCODE3,
  TOG_TESTCODE4,
  TOG_TESTCODE5,
  TOG_TESTCODE6,
  TOG_QUESTCODE1,
  TOG_QUESTCODE2,
  TOG_QUESTCODE3,
  TOG_QUESTCODE4,
  MAX_TOG_TYPES
};

extern togTypeT & operator++(togTypeT &c, int);


class togEntry {
  public:
  bool toggle;
  bool testcode; // is this a test/quest code?
  const sstring name;
  const sstring descr;

  togEntry(bool, bool, const sstring, const sstring);
  ~togEntry();

  private:
  togEntry();  // deny usage in this format
};


class togInfoT {
  map<togTypeT, togEntry *>toggles;

 public:
  void loadToggles();

  togEntry *operator[] (const togTypeT);

  togInfoT();
  ~togInfoT();  
};



// defines for avenger quest

const int AVENGER_ROOM      = 9359;

const int TOG_AVENGER_ELIGIBLE    = 1;
const int TOG_AVENGER_RULES       = 2;
const int TOG_AVENGER_HUNTING     = 3;
const int TOG_AVENGER_SOLO        = 4;
const int TOG_AVENGER_COMPLETE    = 5;
const int TOG_AVENGER_RECEIVED    = 6;
const int TOG_AVENGER_CHEAT       = 7;
const int TOG_AVENGER_PENANCED    = 8;

// defines for vindicator quest

const int ROOM_VINDICATOR_1       =        11149;
const int ROOM_VINDICATOR_2       =        23276;

const int TOG_VINDICATOR_ELIGIBLE    = 9;
const int TOG_VINDICATOR_FOUND_BLACKSMITH = 10;
const int TOG_VINDICATOR_HUNTING_1   = 11;
const int TOG_VINDICATOR_SOLO_1      = 12;
const int TOG_VINDICATOR_CHEAT       = 13;
const int TOG_VINDICATOR_GOOD_ORE    = 14;
const int TOG_VINDICATOR_START_2     = 15;
const int TOG_VINDICATOR_SEEK_PHOENIX= 16;
const int TOG_VINDICATOR_RULES_2     = 17;
const int TOG_VINDICATOR_HUNTING_2   = 18;
const int TOG_VINDICATOR_SOLO_2      = 19;
const int TOG_VINDICATOR_CHEAT_2     = 20;
const int TOG_VINDICATOR_GOT_FEATHER = 21;
const int TOG_VINDICATOR_GOOD_FEATHER= 22;
const int TOG_VINDICATOR_COMPLETE    = 23;
const int TOG_VINDICATOR_DISHONOR    = 24;
const int TOG_VINDICATOR_ON_PENANCE  = 25;
const int TOG_VINDICATOR_GOT_BOOK    = 26;
const int TOG_VINDICATOR_PURIFIED    = 27;

// toggle #28 has been reassigned

// immortality quest
const int TOG_IMMORTAL_ON_QUEST      = 29;

// defines for silverclaw quest //

const int ROOM_SILVERCLAW_1		= 9489;

const int TOG_SILVERCLAW_TO_SCAR	 =   30;
const int TOG_SILVERCLAW_EEL_SKIN    = 31;
const int TOG_SILVERCLAW_MAP_HOBBIT  = 32;
const int TOG_SILVERCLAW_FIND_GOBLIN = 33;
const int TOG_SILVERCLAW_GIVE_WARLORD  = 34;
const int TOG_SILVERCLAW_GRUUM_RITUAL  = 35;
const int TOG_SILVERCLAW_BISH_TABLET = 36;
const int TOG_SILVERCLAW_SOLO_TABLET = 37;
const int TOG_SILVERCLAW_FIND_CHIEF  = 38;
const int TOG_SILVERCLAW_TO_CLOUD    = 39;
const int TOG_SILVERCLAW_TO_BRONZE   = 40;
const int TOG_SILVERCLAW_TO_WORK     = 41;
const int TOG_SILVERCLAW_TO_GOLD     = 42;
const int TOG_SILVERCLAW_TO_DUK      = 43;
const int TOG_SILVERCLAW_TO_RALI     = 44;
const int TOG_SILVERCLAW_END_QUEST   = 45;

// defines for holy devastator quest //

const int ROOM_CREED		= 	2495;
const int ROOM_TAILLE		= 	2273;
const int ROOM_OVERLORD		= 	5442;
const int ROOM_GRIZWALD		= 	9331;
const int ROOM_MOAT_MONSTER	= 	9736;
const int ROOM_ABNOR		= 	9326;
const int ROOM_ASSASSIN		= 	3857;
const int ROOM_LORTO		= 	28880;
const int ROOM_YOLA		= 	9148;
const int ROOM_SULTRESS		= 	3459;
const int ROOM_NESNUM		= 	7531;

const int TOG_DEVASTATOR_FIND_BEN    = 46;
const int TOG_DEVASTATOR_TOOK_BRIBE  = 47;
const int TOG_DEVASTATOR_FIND_OPAL   = 48;
const int TOG_DEVASTATOR_DO_RIDDLE   = 49;
const int TOG_DEVASTATOR_KILL_BEN    = 50;
const int TOG_DEVASTATOR_FIND_MED    = 51;
const int TOG_DEVASTATOR_FIND_CRUCIFIX  = 52;
const int TOG_DEVASTATOR_FOUND_CRUCIFIX  = 53;
const int TOG_DEVASTATOR_GAVE_CRUCIFIX  = 54;
const int TOG_DEVASTATOR_KILL_PC     = 55;
const int TOG_DEVASTATOR_GOT_WINE    = 56;
const int TOG_DEVASTATOR_DO_RIDDLE2  = 57;
const int TOG_DEVASTATOR_FIND_SWORD   = 58;
const int TOG_DEVASTATOR_FIND_RING   = 59;
const int TOG_DEVASTATOR_FORFEIT_VIN  = 60;
const int TOG_DEVASTATOR_NODEAL      = 61;
const int TOG_DEVASTATOR_TOOK_DEAL   = 62;
const int TOG_DEVASTATOR_GOT_INFO    = 63;
const int TOG_DEVASTATOR_GET_FLOWER  = 64;
const int TOG_DEVASTATOR_DO_RIDDLE3  = 65;
const int TOG_DEVASTATOR_FIND_LORTO  = 66;
const int TOG_DEVASTATOR_A_LIE       = 67;
const int TOG_DEVASTATOR_FIND_SULT   = 68;
const int TOG_DEVASTATOR_FIND_BARA   = 69;
const int TOG_DEVASTATOR_GAVE_DRESS  = 70;
const int TOG_DEVASTATOR_FIND_SLOTH  = 71;
const int TOG_DEVASTATOR_CAN_GET_DEVASTATOR = 72;
const int TOG_DEVASTATOR_CHEAT_MISER_BEN = 73;
const int TOG_DEVASTATOR_CHEAT_SPARTAGUS = 74;
const int TOG_DEVASTATOR_CHEAT_MARCUS = 75;
const int TOG_DEVASTATOR_CHEAT_TAILLE = 76;
const int TOG_DEVASTATOR_CHEAT_ABNOR = 77;
const int TOG_DEVASTATOR_CHEAT_SULTRESS = 78;
const int TOG_DEVASTATOR_CHEAT_NESMUM = 79;

const int TOG_SPELL_READ_MAGIC = 80;

const int TOG_DEVASTATOR_KILL_ABNOR = 87; // before toggle 63 chronologically
const int TOG_DEVASTATOR_HAS_DEVASTATOR = 96;

//Toggles for the Spirit of the Warrior Quest
const int TOG_SPIRIT_OF_WARRIOR_1 = 97;
const int TOG_SPIRIT_OF_WARRIOR_2 = 98;
const int TOG_SPIRIT_OF_WARRIOR_3 = 99;

//Toggle for Omen's Quest
const int TOG_SCULPTURE_1   = 100;
const int TOG_SCULPTURE_2   = 101;
const int TOG_SCULPTURE_3   = 102;

//Toggles for Various Spells/Ksills
const int TOG_TORNADO_ELIGIBLE     = 103;
const int TOG_HAS_TORNADO          = 104;
const int TOG_ELIGIBLE_BARKSKIN    = 105;
const int TOG_HAS_BARKSKIN         = 106;
const int TOG_ELIGIBLE_EARTHQUAKE  = 107;
const int TOG_HAS_EARTHQUAKE       = 108;
const int TOG_ELIGIBLE_DUAL_WIELD  = 109;
const int TOG_HAS_DUAL_WIELD       = 110;
const int TOG_ELIGIBLE_SHAPE_SHIFT = 111;
const int TOG_HAS_SHAPE_SHIFT      = 112;
const int TOG_ELIGIBLE_FIREBALL    = 113;
const int TOG_HAS_FIREBALL         = 114;
const int TOG_ELIGIBLE_ICE_STORM   = 115;
const int TOG_FIND_DARK_ROBES      = 116;
const int TOG_HAS_ICE_STORM        = 117;
const int TOG_ELIGIBLE_STONESKIN   = 118;
const int TOG_FIND_GRANITE_SIGNET  = 119;
const int TOG_HAS_GRANITE_SIGNET   = 120;
const int TOG_HAS_STONESKIN        = 121;
const int TOG_ELIGIBLE_GALVANIZE   = 122;
const int TOG_FIND_PURPLE_ROBE     = 123;
const int TOG_HAS_GALVANIZE        = 124;
const int TOG_ELIGIBLE_POWERSTONE  = 125; 
const int TOG_FIND_SNAKESTAFF      = 126;
const int TOG_HAS_POWERSTONE       = 127;
const int TOG_HAS_ADVANCED_KICKING = 128;
const int TOG_PAID_TOLL            = 129;
const int TOG_ELIGIBLE_ADVANCED_KICKING = 130;
const int TOG_LOGRUS_INITIATION    = 131;

// Monk sash quests
const int TOG_ELIGIBLE_MONK_WHITE   = 132;
const int TOG_STARTED_MONK_WHITE    = 133;
const int TOG_HAS_MONK_WHITE        = 134;

const int TOG_ELIGIBLE_MONK_YELLOW  = 135;
const int TOG_FINISHED_MONK_YELLOW  = 136;
const int TOG_HAS_MONK_YELLOW       = 137;

const int TOG_MONK_PURPLE_ELIGIBLE  = 138;
const int TOG_MONK_PURPLE_STARTED   = 139;
const int TOG_MONK_PURPLE_LEPER_KILLED1    = 140;
const int TOG_MONK_PURPLE_LEPER_KILLED2    = 141;
const int TOG_MONK_PURPLE_LEPER_KILLED3    = 142;
const int TOG_MONK_PURPLE_LEPER_KILLED4    = 143;
const int TOG_MONK_PURPLE_FINISHED  = 144;
const int TOG_MONK_PURPLE_OWNED     = 145;

const int TOG_ELIGIBLE_MONK_BLUE    = 146;
const int TOG_STARTED_MONK_BLUE     = 147;
const int TOG_MONK_KILLED_SHARK     = 148;
const int TOG_FINISHED_MONK_BLUE    = 149;
const int TOG_HAS_MONK_BLUE         = 150;

const int TOG_MONK_GREEN_ELIGIBLE   = 151;
const int TOG_MONK_GREEN_STARTED    = 152;
const int TOG_MONK_GREEN_FALLING    = 153;
const int TOG_MONK_GREEN_FALLEN     = 154;
const int TOG_MONK_GREEN_OWNED      = 156;

const int TOG_MONK_RED_ELIGIBLE     = 81;
const int TOG_STARTED_MONK_RED      = 82;
const int TOG_FINISHED_MONK_RED     = 83;
const int TOG_HAS_MONK_RED          = 84;

const int TOG_MONK_BLACK_ELIGIBLE   = 215;
const int TOG_MONK_BLACK_STARTED    = 216;
const int TOG_MONK_BLACK_FINISHED   = 217;
const int TOG_MONK_BLACK_OWNED      = 218;


const int TOG_GAVE_ESSENCE_RAT_KING = 155; // non-monk toggle reused per request

const int TOG_MAGE_BELT_ELIGIBLE    = 157;
const int TOG_MAGE_BELT_STARTED     = 158;
const int TOG_MAGE_BELT_THREAD_HUNT = 159;
const int TOG_MAGE_BELT_OWNED       = 160;

const int TOG_HAS_CATFALL           = 161;

// Ranger Level 7 Quest

const int TOG_RANGER_FIRST_ELIGIBLE    = 162;
const int TOG_RANGER_FIRST_FOUND_HERMIT= 28;  // safe to reuse
const int TOG_RANGER_FIRST_STARTED     = 163;
const int TOG_RANGER_FIRST_GNOBLE      = 164;
const int TOG_RANGER_FIRST_FARMER      = 165;
const int TOG_RANGER_FIRST_CHILDREN    = 166;
const int TOG_RANGER_FIRST_FARMHAND    = 167;
const int TOG_RANGER_FIRST_KILLED_OK   = 168;
const int TOG_RANGER_FIRST_GAVE_HIDE   = 169;
const int TOG_RANGER_FIRST_GAVE_PELT   = 170;
const int TOG_RANGER_FIRST_HUNTING     = 171;
const int TOG_RANGER_FIRST_GOT_SCROLL  = 172;
const int TOG_RANGER_FIRST_FINISHED    = 173;

// Ranger Level 14 Quest

const int TOG_ELIGIBLE_RANGER_L14   = 174;
const int TOG_STARTED_RANGER_L14    = 175;
const int TOG_SEEN_KOBOLD_POACHER   = 176;
const int TOG_SEEKING_ORC_POACHER   = 177;
const int TOG_SEEN_ORC_POACHER      = 178;
const int TOG_SEEKING_BONE_WOMAN    = 179;    
const int TOG_SEEKING_APPLE         = 180;
const int TOG_GOT_CARVED_BUCKLE     = 181;
const int TOG_SEEKING_ORC_MAGI      = 182;
const int TOG_FAILED_TO_KILL_MAGI   = 183;
const int TOG_PROVING_SELF          = 184;
const int TOG_KILLED_ORC_MAGI       = 185;
const int TOG_FINISHED_RANGER_L14   = 186;

const int TOG_DEVESTATOR_ELIGIBLE   = 187;

// Mage Robe Quest - Level 25

const int TOG_ELIGIBLE_MAGE_ROBE    = 188;
const int TOG_MAGE_ROBE_SEEK_DRUID  = 189;
const int TOG_MAGE_ROBE_GET_OIL     = 190;
const int TOG_MAGE_ROBE_GET_SYMBOL  = 191;
const int TOG_MAGE_ROBE_GET_METAL   = 192;
const int TOG_MAGE_ROBE_GET_FABRIC  = 193;
const int TOG_HAS_MAGE_ROBE         = 194;

// Ranger Level 21 Quest

const int TOG_ELIGIBLE_RANGER_L21   = 195;
const int TOG_STARTED_RANGER_L21    = 196;
const int TOG_KILLED_CLERIC_V       = 197;
const int TOG_FAILED_CLERIC_V       = 198;
const int TOG_PENANCE_R21_1         = 199;
const int TOG_SEEKING_CLERIC_A      = 200;
const int TOG_KILLED_CLERIC_A       = 201;
const int TOG_FAILED_CLERIC_A       = 202;
const int TOG_PENANCE_R21_2         = 203;
const int TOG_SEEKING_FEATHERS      = 204;
const int TOG_TALKED_CHIEF_A        = 205;
const int TOG_GOT_FEATHERS          = 206;
const int TOG_SEEKING_2_PELTS       = 207;
const int TOG_GAVE_1_PELT           = 208;
const int TOG_SEEKING_OIL           = 209;
const int TOG_SEEKING_BANDITS       = 210;
const int TOG_GOT_OIL               = 211;
const int TOG_FINISHED_RANGER_L21   = 212;

const int TOG_FACTIONS_ELIGIBLE     = 213;

const int TOG_BOUGHT_CALDONIA_SHOT  = 214;

// Warrior lvl 40 quest

const int TOG_ELIGIBLE_WARRIOR_L41 = 219; //f
const int TOG_KILL_SHAMAN = 220; // c
const int TOG_KILL_CHIEF = 221;
const int TOG_GAVE_HEAD_CHIEF = 222; // d
const int TOG_FINISHED_WARRIOR_L41 = 223; //b

// Shaman level 15 juju bag quest

const int TOG_ELIGIBLE_JUJU = 224;
const int TOG_GET_THONG = 225;
const int TOG_MARE_HIDE = 226;
const int TOG_GET_SINEW = 227;
const int TOG_GET_BEADS = 228;
const int TOG_DONE_JUJU = 229;


// please recycle these toggles before assigning new ones
const int TOG_IMMORTAL_STAT =  85;
const int TOG_IMMORTAL_LOGS =  86;
                                     //toggles 88-96 are blank
const int TOG_IMMORTAL_BLANK_2 = 88; //reserved for later immortal toggles
const int TOG_IMMORTAL_BLANK_3 = 89;
const int TOG_IMMORTAL_BLANK_4 = 90;
const int TOG_IMMORTAL_BLANK_5 = 91;
const int TOG_IMMORTAL_BLANK_6 = 92;
const int TOG_IMMORTAL_BLANK_7 = 93;
const int TOG_IMMORTAL_BLANK_8 = 94;
const int TOG_IMMORTAL_BLANK_9 = 95;
const int TOG_IMMORTAL_BLANK_10 = 96;
// end recycle

const int TOG_HAS_PAID_FACT_FEE = 230;
const int TOG_HAS_CREATED_FACTION = 231;

const int TOG_TOTEM_MASK_ELIGIBLE              = 232;
const int TOG_TOTEM_MASK_STARTED               = 233;
const int TOG_TOTEM_MASK_FIND_FORSAKEN         = 234;
const int TOG_TOTEM_MASK_FIND_WOODEN_PLANK     = 235;
const int TOG_TOTEM_MASK_HAS_SAPLESS_WOOD      = 236;
const int TOG_TOTEM_MASK_FIND_SCALED_HIDE      = 237;
const int TOG_TOTEM_MASK_GIVE_COVERED_GONDOLFO = 238;
const int TOG_TOTEM_MASK_RECOVER_VIAL          = 239;
const int TOG_TOTEM_MASK_FIND_ELRIC_GRIS_GRIS  = 240;
const int TOG_TOTEM_MASK_KILL_BARON_SAMEDI     = 241;
const int TOG_TOTEM_MASK_HAS_BARONS_VISION     = 242;
const int TOG_TOTEM_MASK_KILLED_ELRIC          = 243;
const int TOG_TOTEM_MASK_FACE_TRUE_EVIL        = 244;
const int TOG_TOTEM_MASK_KILLED_FATHERS_SPIRIT = 245;
const int TOG_TOTEM_MASK_FINISHED              = 246;

const int TOG_PERMA_DEATH_CHAR                 = 247;

const int TOG_PSIONICIST                       = 248;

const int TOG_DRAGON_ARMOR_RED_SCALES          = 249;
const int TOG_DRAGON_ARMOR_GREEN_SCALES        = 250;
const int TOG_DRAGON_ARMOR_WHITE_SCALES        = 251;
const int TOG_DRAGON_ARMOR_TUNGSTEN_SUIT       = 252;    
const int TOG_DRAGON_ARMOR_HUGE_OPAL           = 253;
const int TOG_DRAGON_ARMOR_DRAGON_BONE_1       = 254;
const int TOG_DRAGON_ARMOR_DRAGON_BONE_2       = 255;
const int TOG_DRAGON_ARMOR_DRAGON_BONE_3       = 256;
const int TOG_DRAGON_ARMOR_DRAGON_BONE_4       = 257;
const int TOG_DRAGON_ARMOR_DRAGON_BONE_5       = 258;
const int TOG_DRAGON_ARMOR_ILLITHID_SINEW      = 259;
const int TOG_DRAGON_ARMOR_THREAD_ELEMENT      = 260;
const int TOG_DRAGON_ARMOR_TALENS              = 261;
const int TOG_DRAGON_ARMOR_WARHAMMER           = 262;
const int TOG_DRAGON_ARMOR_GAVE_SCALES         = 263;

const int TOG_BITTEN_BY_VAMPIRE                = 264;
const int TOG_VAMPIRE                          = 265;

const int TOG_LYCANTHROPE                      = 266;
const int TOG_TRANSFORMED_LYCANTHROPE          = 267;
const int TOG_LYCANTHROPE_VIRGIN_FLASK         = 268;
const int TOG_LYCANTHROPE_YYRN_TERFEFLY        = 269;
const int TOG_LYCANTHROPE_YURNO_INGREDIENTS    = 270;
const int TOG_LYCANTHROPE_YURNO_URINE          = 271;
const int TOG_LYCANTHROPE_YURNO_WOLVESBANE     = 272;
const int TOG_LYCANTHROPE_YURNO_SILVER         = 273;
const int TOG_LYCANTHROPE_YURNO_HEMLOCK        = 274;

const int TOG_BLAHBLAH                         = 275;

const int TOG_MONK_PAID_TABUDA                 = 276;

const int TOG_HAS_CATLEAP                      = 277;

const int TOG_IS_COWARD                        = 278;
const int TOG_IS_BLIND                         = 279;
const int TOG_IS_MUTE                          = 280;
const int TOG_IS_DEAF                          = 281;
const int TOG_IS_ASTHMATIC                     = 282;
const int TOG_IS_NECROPHOBIC                   = 283;
const int TOG_IS_NARCOLEPTIC                   = 284;
const int TOG_IS_COMBUSTIBLE                   = 285;
const int TOG_IS_HEMOPHILIAC                   = 286;
const int TOG_IS_AMBIDEXTROUS                  = 287;
const int TOG_IS_HEALTHY                       = 288;
const int TOG_HAS_NIGHTVISION                  = 289;
const int TOG_IS_ALCOHOLIC                     = 290;
const int TOG_HAS_TOURETTES                    = 291;

const int TOG_IS_PK_CHAR                       = 292;

const int TOG_PSI_RUBY                         = 293;
const int TOG_PSI_BLUESTEEL                    = 294;
const int TOG_PSI_ESSENCE                      = 295;
const int TOG_PSI_ROCKFISH                     = 296;
const int TOG_PSI_TALENS                       = 297;

const int MAX_TOG_INDEX       = 298;  // move and change

extern TOGINFO TogIndex[MAX_TOG_INDEX + 1];
extern togInfoT toggleInfo;

#endif




