//////////////////////////////////////////////////////////////////////////
//
// SneezyMUD - All rights reserved, SneezyMUD Coding Team
//
//////////////////////////////////////////////////////////////////////////


#ifndef __SPELLS2_H
#define __SPELLS2_H

enum skillUseTypeT {
     SPELL_UNDEFINED,
     SPELL_CASTER,
     SPELL_DANCER,
     SPELL_PRAYER,
     SKILL_CASTER ,
     SKILL_DANCER,
     SKILL_PRAYER ,
     SKILL_FIGHTER,
};

enum skillUseClassT {
     SPELL_NOCLASS,
     SPELL_SHAMAN,
     SPELL_MAGE ,
     SPELL_RANGER ,
     SPELL_ANTI   ,
     SPELL_CLERIC ,
     SPELL_DEIKHAN,
     SKILL_GENERAL,
     SKILL_WARRIOR,
     SKILL_DEIKHAN,
     SKILL_THIEF ,
     SKILL_MONK  ,
     SKILL_RANGER,
     SKILL_CLERIC,
     SKILL_MAGE,
     SKILL_SHAMAN,
     SKILL_CLERIC_TYPES,
     SKILL_MAGE_TYPES
};

extern skillUseTypeT getSpellType(skillUseClassT);

const unsigned int TAR_IGNORE		 = (1<<0);
const unsigned int TAR_CHAR_ROOM    = (1<<1);
const unsigned int TAR_CHAR_WORLD   = (1<<2);
const unsigned int TAR_FIGHT_SELF   = (1<<3);
const unsigned int TAR_FIGHT_VICT   = (1<<4);
const unsigned int TAR_SELF_ONLY    = (1<<5) ;
const unsigned int TAR_SELF_NONO    = (1<<6) ;
const unsigned int TAR_OBJ_INV		  = (1<<7);
const unsigned int TAR_OBJ_ROOM     = (1<<8);
const unsigned int TAR_OBJ_WORLD    = (1<<9);
const unsigned int TAR_OBJ_EQUIP    = (1<<10);
const unsigned int TAR_NAME	       = (1<<11);
const unsigned int TAR_VIOLENT		  = (1<<12);
const unsigned int TAR_ROOM	       = (1<<13); 
const unsigned int TAR_AREA	       = (1<<14); 
const unsigned int TAR_CHAR_VIS_WORLD  = (1<<15);

const unsigned int COMP_GESTURAL		= (1<<0);
const unsigned int COMP_GESTURAL_INIT		     = (1<<1);
const unsigned int COMP_GESTURAL_END		    = (1<<2);
const unsigned int COMP_GESTURAL_ALWAYS		       = (1<<3);
const unsigned int COMP_GESTURAL_RANDOM		       = (1<<4);
const unsigned int COMP_VERBAL			      = (1<<5);
const unsigned int COMP_VERBAL_INIT		   = (1<<6);
const unsigned int COMP_VERBAL_END		  = (1<<7);
const unsigned int COMP_VERBAL_ALWAYS		     = (1<<8);
const unsigned int COMP_VERBAL_RANDOM		     = (1<<9);
const unsigned int COMP_MATERIAL		= (1<<10);
const unsigned int COMP_MATERIAL_INIT		     = (1<<11);
const unsigned int COMP_MATERIAL_END		    = (1<<12);
const unsigned int COMP_MATERIAL_ALWAYS		       = (1<<13);
const unsigned int COMP_MATERIAL_RANDOM		       = (1<<14);
const unsigned int COMP_MATERIAL_ALMOST_END	   = (1<<15);
const unsigned int SPELL_TASKED			       = (1<<16);
const unsigned int SPELL_IGNORE_POSITION          = (1<<17);
const unsigned int SPELL_TASKED_EVERY             = (1<<18);

// this set will be obsolete eventually
enum taskDiffT {
     TASK_TRIVIAL,
     TASK_EASY,
     TASK_NORMAL,
     TASK_DIFFICULT,
     TASK_DANGEROUS,
     TASK_HOPELESS,
     TASK_IMPOSSIBLE
};

enum lag_t {
   LAG_0, LAG_1, LAG_2, LAG_3, LAG_4,
   LAG_5, LAG_6, LAG_7, LAG_8, LAG_9
};

enum manaCostT {
  MANA_0 = 0,
  MANA_5          =  5,
  MANA_10         = 10,
  MANA_15         = 15,
  MANA_17         = 17,
  MANA_20         = 20,
  MANA_23         = 23,
  MANA_25         = 25,
  MANA_27         = 27,
  MANA_30         = 30,
  MANA_33         = 33,
  MANA_35         = 35,
  MANA_38         = 38,
  MANA_40         = 40,
  MANA_45         = 45,
  MANA_47         = 47,
  MANA_50         = 50,
  MANA_53         = 53,
  MANA_54         = 54,
  MANA_55         = 55,
  MANA_60         = 60,
  MANA_65         = 65,
  MANA_70         = 70,
  MANA_75         = 75,
  MANA_80         = 80,
  MANA_85         = 85,
  MANA_90         = 90,
  MANA_95         = 95,
  MANA_100        = 100,
  MANA_200        = 200
};

enum lifeforceCostT {
  LIFEFORCE_0 = 0,
  LIFEFORCE_5          =  5,
  LIFEFORCE_10         = 10,
  LIFEFORCE_15         = 15,
  LIFEFORCE_20         = 20,
  LIFEFORCE_25         = 25,
  LIFEFORCE_30         = 30,
  LIFEFORCE_35         = 35,
  LIFEFORCE_40         = 40,
  LIFEFORCE_45         = 45,
  LIFEFORCE_50         = 50,
  LIFEFORCE_55         = 55,
  LIFEFORCE_60         = 60,
  LIFEFORCE_65         = 65,
  LIFEFORCE_70         = 70,
  LIFEFORCE_75         = 75,
  LIFEFORCE_80         = 80,
  LIFEFORCE_85         = 85,
  LIFEFORCE_90         = 90,
  LIFEFORCE_95         = 95,
  LIFEFORCE_100        = 100,
  LIFEFORCE_110        = 110,
  LIFEFORCE_120        = 120,
  LIFEFORCE_125        = 125,
  LIFEFORCE_130        = 130,
  LIFEFORCE_140        = 140,
  LIFEFORCE_150        = 150,
  LIFEFORCE_160        = 160,
  LIFEFORCE_170        = 170,
  LIFEFORCE_175        = 175,
  LIFEFORCE_180        = 180,
  LIFEFORCE_190        = 190,
  LIFEFORCE_200        = 200,
  LIFEFORCE_210        = 210,
  LIFEFORCE_220        = 220,
  LIFEFORCE_225        = 225,
  LIFEFORCE_230        = 230,
  LIFEFORCE_240        = 240,
  LIFEFORCE_250        = 250,
  LIFEFORCE_260        = 260,
  LIFEFORCE_270        = 270,
  LIFEFORCE_275        = 275,
  LIFEFORCE_280        = 280,
  LIFEFORCE_290        = 290,
  LIFEFORCE_300        = 300,
  LIFEFORCE_310        = 310,
  LIFEFORCE_320        = 320,
  LIFEFORCE_325        = 325,
  LIFEFORCE_330        = 330,
  LIFEFORCE_340        = 340,
  LIFEFORCE_350        = 350,
  LIFEFORCE_360        = 360,
  LIFEFORCE_370        = 370,
  LIFEFORCE_375        = 375,
  LIFEFORCE_380        = 380,
  LIFEFORCE_390        = 390,
  LIFEFORCE_400        = 400,
  LIFEFORCE_410        = 410,
  LIFEFORCE_420        = 420,
  LIFEFORCE_425        = 425,
  LIFEFORCE_430        = 430,
  LIFEFORCE_440        = 440,
  LIFEFORCE_450        = 450,
  LIFEFORCE_460        = 460,
  LIFEFORCE_470        = 470,
  LIFEFORCE_475        = 475,
  LIFEFORCE_480        = 480,
  LIFEFORCE_490        = 490,
  LIFEFORCE_500        = 500
};

// enum can't be float unfortunately, which we want
// KLUDGE: values here will be 4* the float we really want 
// the ctor for spellInfo will turn into float
enum pietyCostT {
   PRAY_0 = 0,
   PRAY_025       = 5,
   PRAY_050       = 10,
   PRAY_075       = 15,
   PRAY_100       = 20,
   PRAY_150       = 30,
   PRAY_200       = 40,
   PRAY_250       = 50,
   PRAY_300       = 60,
   PRAY_350       = 75,
   PRAY_400       = 80,
   PRAY_450       = 90,
   PRAY_500       = 100
};

enum symbolStressT {
    SYMBOL_STRESS_0          = 0,
    SYMBOL_STRESS_5         =  5,
    SYMBOL_STRESS_10        = 10,
    SYMBOL_STRESS_12      = 12,
    SYMBOL_STRESS_15        = 15,
    SYMBOL_STRESS_17        = 17,
    SYMBOL_STRESS_18        = 18,
    SYMBOL_STRESS_20        = 20,
    SYMBOL_STRESS_24      = 24,
    SYMBOL_STRESS_25        = 25,
    SYMBOL_STRESS_30        = 30,
    SYMBOL_STRESS_35        = 35,
    SYMBOL_STRESS_36      = 36,
    SYMBOL_STRESS_40        = 40,
    SYMBOL_STRESS_45        = 45,
    SYMBOL_STRESS_48      = 48,
    SYMBOL_STRESS_50        = 50,
    SYMBOL_STRESS_55        = 55,
    SYMBOL_STRESS_60        = 60,
    SYMBOL_STRESS_65        = 65,
    SYMBOL_STRESS_70        = 70,
    SYMBOL_STRESS_75        = 75,
    SYMBOL_STRESS_80        = 80,
    SYMBOL_STRESS_85        = 85,
    SYMBOL_STRESS_90        = 90,
    SYMBOL_STRESS_95        = 95,
    SYMBOL_STRESS_100      = 100  // high
};

enum discStartT {
  START_0 = 0,
  START_1 = 1,
  START_3 = 3,
  START_5 = 5,
  START_6 = 6,
  START_8 = 8,
  START_10 = 10,
  START_11 = 11,
  START_12 = 12,
  START_15 = 15,
  START_16 = 16,
  START_18 = 18,
  START_20 = 20,
  START_21 = 21,
  START_24 = 24,
  START_25 = 25,
  START_26 = 26,
  START_27 = 27,
  START_28 = 28,
  START_30 = 30,
  START_31 = 31,
  START_32 = 32,
  START_33 = 33,
  START_34 = 34,
  START_35 = 35,
  START_36 = 36,
  START_37 = 37,
  START_38 = 38,
  START_39 = 39,
  START_40 = 40,
  START_41 = 41,
  START_42 = 42,
  START_43 = 43,
  START_44 = 44,
  START_45 = 45,
  START_46 = 46,
  START_47 = 47,
  START_50 = 50,
  START_51 = 51,
  START_52 = 52,
  START_53 = 53,
  START_55 = 55,
  START_56 = 56,
  START_57 = 57,
  START_58 = 58,
  START_59 = 59,
  START_60 = 60,
  START_61 = 61,
  START_63 = 63,
  START_65 = 65,
  START_66 = 66,
  START_67 = 67,
  START_68 = 68,
  START_70 = 70,
  START_71 = 71,
  START_72 = 72,
  START_75 = 75,
  START_76 = 76,
  START_79 = 79,
  START_80 = 80,
  START_81 = 81,
  START_83 = 83,
  START_84 = 84,
  START_85 = 85,
  START_86 = 86,
  START_99 = 99,
  START_100 = 100
};

enum discLearnT {
  LEARN_0 = 0,
  LEARN_1 = 1,
  LEARN_2 = 2,
  LEARN_3 = 3,
  LEARN_4 = 4,
  LEARN_5 = 5,
  LEARN_6 = 6,
  LEARN_7 = 7,
  LEARN_8 = 8,
  LEARN_9 = 9,
  LEARN_10 = 10,
  LEARN_11 = 11,
  LEARN_12 = 12,
  LEARN_13 = 13,
  LEARN_14 = 14,
  LEARN_15 = 15,
  LEARN_16 = 16,
  LEARN_17 = 17,
  LEARN_18 = 18,
  LEARN_20 = 20,
  LEARN_23 = 23,
  LEARN_25 = 25,
  LEARN_33 = 33,
  LEARN_34 = 34,
  LEARN_35 = 35,
  LEARN_50 = 50,
  LEARN_100 = MAX_SKILL_LEARNEDNESS
};

enum discStartDoT {
  START_DO_NO = -1,
  START_DO_1 = 1,
  START_DO_10 = 10,
  START_DO_20 = 20,
  START_DO_25 = 25,
  START_DO_30 = 30,
  START_DO_35 = 35,
  START_DO_40 = 40,
  START_DO_45 = 45,
  START_DO_50 = 50,
  START_DO_55 = 55,
  START_DO_60 = 60,
};

enum discLearnDoT {
  LEARN_DO_NO = -1,
  LEARN_DO_1 = 1,
  LEARN_DO_2 = 2,
  LEARN_DO_3 = 3,
  LEARN_DO_4 = 4,
  LEARN_DO_5 = 5,
  LEARN_DO_7 = 7,
  LEARN_DO_10 = 10
};

class spellInfo {
  public:
    const char *name;
    int start;
    int learn;
    unsigned int uses;
    long unsigned levels;
    long unsigned learned;
    long unsigned damage;
    long unsigned pot_damage;
    long unsigned pot_victims;
    long unsigned pot_level;
    unsigned int victims;
    unsigned int crits;
    unsigned int critf;
    unsigned int success;
    unsigned int potSuccess;
    unsigned int fail;
    unsigned int focusValue;
    unsigned int newAttempts;
    unsigned int lowAttempts;
    unsigned int midAttempts;
    unsigned int goodAttempts;
    unsigned int highAttempts;
    unsigned int engAttempts;
    unsigned int genFail;
    unsigned int focFail;
    unsigned int engFail;
    unsigned int saves;
    unsigned long learnAttempts;
    unsigned int learnSuccess;
    unsigned int learnLearn;
    unsigned int learnLevel;
    unsigned int learnFail;
    unsigned long learnBoost;
    unsigned int learnDiscSuccess;
    unsigned int learnAdvDiscSuccess;
    unsigned int mobUses;
    unsigned long mobLevels;
    unsigned long mobLearned;
    unsigned long mobDamage;
    unsigned int mobVictims;
    unsigned int mobCrits;
    unsigned int mobCritf;
    unsigned int mobSuccess;
    unsigned int potSuccessMob;
    unsigned int mobFail;
    unsigned int mobSaves;
    unsigned int immUses;
    unsigned long immLevels;
    unsigned long immLearned;
    unsigned long immDamage;
    unsigned int immVictims;
    unsigned int immCrits;
    unsigned int immCritf;
    unsigned int immSuccess;
    unsigned int potSuccessImm;
    unsigned int immFail;
    unsigned int immSaves;
    lag_t lag;
    skillUseClassT typ;
    taskDiffT task;
    positionTypeT minPosition;
    int minMana;
    int minLifeforce; // shaman
    float minPiety;
    unsigned int targets;
    int holyStrength;
    const char *fadeAway, *fadeAwayRoom, *fadeAwaySoon, *fadeAwaySoonRoom;
    float alignMod;
    unsigned int comp_types;
    unsigned int toggle;
    discNumT disc;
    discNumT assDisc;
    byte startLearnDo;
    byte amtLearnDo;
    int learnDoDiff;
    byte secStartLearnDo;
    byte secAmtLearnDo;
    map<int, int>sectorData;
    map<int, int>weatherData;

  private:
    spellInfo() {}  // prevent default constructor from being used
  public:
    spellInfo(skillUseClassT styp, 
         discNumT discipline, 
         discNumT assDiscipline, 
         const char *n,
         taskDiffT cast_diff,
         lag_t l,
         positionTypeT pos,
         manaCostT mana,
         lifeforceCostT lifeforce,
         pietyCostT align,
         unsigned int t, 
         symbolStressT h, 
         const char *fa, 
         const char *far, 
         const char *fas, 
         const char *fasr, 
         discStartT starting,
         discLearnT learning,
         discStartDoT learnDoStarting,
         discLearnDoT learningDoAmt,
         discStartDoT secLearnDoStart,
         discLearnDoT secLearnDoAmt,
         int learningDoDiff, float modifier, unsigned int ctyp, 
         unsigned int tgl);
    ~spellInfo();
    spellInfo(const spellInfo &);
    spellInfo & operator = (const spellInfo &);
}; 

#if 0
// defunct, but remains in some commented out code
enum spellTypeT {
     SPELL_TYPE_SPELL,
     SPELL_TYPE_POTION,
     SPELL_TYPE_WAND,
     SPELL_TYPE_STAFF,
     SPELL_TYPE_SCROLL
};
#endif

struct attack_hit_type {
  const char *singular;
  const char *plural;
  const char *hitting;
};

const int LEARN_DIFF_UNUSUAL  = -1;
const int LEARN_DIFF_NORMAL    = 0;
const int LEARN_DIFF_SPELLS    = 0;
const int LEARN_DIFF_PRAYERS    = 0;
const int LEARN_DIFF_SKILLS    = 0;

struct PolyType {
  const char * const name;
  int  level;
  int  learning;
  int  number;
  discNumT  discipline;
  unsigned long int tRace;
};

extern spellInfo *discArray[MAX_SKILL+1];

#endif
