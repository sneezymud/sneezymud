//////////////////////////////////////////////////////////////////////////
//
// SneezyMUD - All rights reserved, SneezyMUD Coding Team
//
//////////////////////////////////////////////////////////////////////////


#ifndef __BEING_H
#define __BEING_H

#ifndef __DB_H
#include "db.h"
#endif

#ifndef __THING_H
#include "thing.h"
#endif

#ifndef __RESPONSE_H
#include "response.h"
#endif

#ifndef __CONNECT_H
#include "connect.h"
#endif

#ifndef __FACTION_H
#include "faction.h"
#endif

#ifndef __SPELLS_H
#include "spells.h"
#endif

#ifndef __CHARACTERISTICS_H
#include "characteristics.h"
#endif

#ifndef __RACE_H
#include "race.h"
#endif

#ifndef __DISCIPLINE_H
#include "discipline.h"
#endif

#ifndef __SKILLS_H
#include "skills.h"
#endif

#ifndef __STRUCTS_H
#include "structs.h"
#endif


class TTrophy;
class TWindow;
class TGenWeapon;
class TBaseClothing;
class TQuiver;

class objCost;
extern TBeing *character_list;
extern long mobCount;
extern vector<mobIndexData>mob_index;
extern TMonster *read_mobile(int nr, readFileTypeT type);

// these functions are used in methods
extern int number(int from, int to);
extern int dice(int, int);
extern bool roll_chance(double);

const unsigned long ACT_STRINGS_CHANGED    = (1<<0); //1
const unsigned long ACT_SENTINEL   = (1<<1);// 2
const unsigned long ACT_SCAVENGER  = (1<<2);// 4
const unsigned long ACT_DISGUISED  = (1<<3);// 8
const unsigned long ACT_NICE_THIEF = (1<<4);// 16
const unsigned long ACT_AGGRESSIVE = (1<<5);         // 32
const unsigned long ACT_STAY_ZONE  = (1<<6);         // 64
const unsigned long ACT_WIMPY      = (1<<7);         // 128
const unsigned long ACT_ANNOYING   = (1<<8);         // 256
const unsigned long ACT_HATEFUL    = (1<<9);         // 512
const unsigned long ACT_AFRAID    = (1<<10);         // 1024
const unsigned long ACT_IMMORTAL  = (1<<11);         // 2048
const unsigned long ACT_HUNTING   = (1<<12);         // 4096
const unsigned long ACT_DEADLY    = (1<<13);         // 8192
const unsigned long ACT_POLYSELF  = (1<<14);         // 16384
const unsigned long ACT_GUARDIAN  = (1<<15);         // 32768
const unsigned long ACT_SKELETON  = (1<<16);         // 65536
const unsigned long ACT_ZOMBIE    = (1<<17);         // 131072
const unsigned long ACT_GHOST     = (1<<18);         // 262144
const unsigned long ACT_DIURNAL   = (1<<19);         // 524288
const unsigned long ACT_NOCTURNAL = (1<<20);         // 1048576
const unsigned long ACT_PROTECTOR = (1<<21);         // 2097152

const unsigned long ACT_PROTECTEE = (1<<22);         // 4194304

const int MAX_MOB_ACTS    = 24;

const unsigned short HATE_SEX      = (1<<0);
const unsigned short HATE_RACE     = (1<<1);
const unsigned short HATE_CHAR     = (1<<2);
const unsigned short HATE_CLASS    = (1<<3);
const unsigned short HATE_UNUSED   = (1<<4);
const unsigned short HATE_UNUSED2  = (1<<5);
const unsigned short HATE_VNUM     = (1<<6);

const unsigned short FEAR_SEX      = (1<<0);
const unsigned short FEAR_RACE     = (1<<1);
const unsigned short FEAR_CHAR     = (1<<2);
const unsigned short FEAR_CLASS    = (1<<3);
const unsigned short FEAR_UNUSED   = (1<<4);
const unsigned short FEAR_UNUSED2  = (1<<5);
const unsigned short FEAR_VNUM     = (1<<6);

const int MAX_MORT      = 50;
const int GOD_LEVEL1    = 51;
const int GOD_LEVEL2    = 52;
const int GOD_LEVEL3    = 53;
const int GOD_LEVEL4    = 54;
const int GOD_LEVEL5    = 55;
const int GOD_LEVEL6    = 56;
const int GOD_LEVEL7    = 57;
const int GOD_LEVEL8    = 58;
const int GOD_LEVEL9    = 59;
const int GOD_LEVEL10   = 60;
const int MAX_IMMORT    = 60;

const unsigned int AUTO_NOSPAM         = (1<<0);
const unsigned int AUTO_EAT            = (1<<1);
const unsigned int AUTO_KILL           = (1<<2);
const unsigned int AUTO_LOOT_MONEY     = (1<<3);
const unsigned int AUTO_LOOT_NOTMONEY  = (1<<4);
const unsigned int AUTO_NOHARM         = (1<<5);
const unsigned int AUTO_NOSHOUT        = (1<<6);
const unsigned int AUTO_AFK            = (1<<7);
const unsigned int AUTO_SPLIT          = (1<<8);
const unsigned int AUTO_SUCCESS        = (1<<9);
const unsigned int AUTO_POUCH          = (1<<10);
const unsigned int AUTO_JOIN           = (1<<11);
const unsigned int AUTO_DISSECT        = (1<<12);
const unsigned int AUTO_ENGAGE         = (1<<13);
const unsigned int AUTO_ENGAGE_ALWAYS  = (1<<14);
const unsigned int AUTO_HUNT           = (1<<15);
const unsigned int AUTO_NOSPELL        = (1<<16);
const unsigned int AUTO_HALFSPELL      = (1<<17);
const unsigned int AUTO_LIMBS          = (1<<18);
const unsigned int AUTO_TIPS           = (1<<19);
const unsigned int AUTO_TROPHY         = (1<<20);
const unsigned int AUTO_PG13           = (1<<21);

const int MAX_AUTO        = 22;    // move and change

const unsigned long PLR_BRIEF      = (1<<0);
const unsigned long PLR_COMPACT    = (1<<1);
const unsigned long PLR_WIMPY      = (1<<2);
const unsigned long PLR_NEWBIEHELP = (1<<3);
const unsigned long PLR_NOHASSLE   = (1<<4);
const unsigned long PLR_STEALTH    = (1<<5);
const unsigned long PLR_HUNTING    = (1<<6);
const unsigned long PLR_MAILING    = (1<<7);
const unsigned long PLR_LOGGED     = (1<<8);
const unsigned long PLR_BUGGING    = (1<<9) ;
const unsigned long PLR_VT100      = (1<<10);
const unsigned long PLR_COLOR      = (1<<11);
const unsigned long PLR_IMMORTAL   = (1<<12);
const unsigned long PLR_ANSI       = (1<<13);
const unsigned long PLR_SEEKSGROUP = (1<<14);
const unsigned long PLR_BANISHED   = (1<<15);
const unsigned long PLR_REGVT      = (1<<16);
const unsigned long PLR_NOMESS     = (1<<17);
const unsigned long PLR_RT_HANDED  = (1<<18);
const unsigned long PLR_AFK        = (1<<19);
const unsigned long PLR_132COLUMN  = (1<<20);      // used in kludges
const unsigned long PLR_SOLOQUEST  = (1<<21);
const unsigned long PLR_GRPQUEST   = (1<<22);
const unsigned long PLR_NOSNOOP    = (1<<23);     // no way to toggle
const unsigned long PLR_PING       = (1<<24);
const unsigned long PLR_GODNOSHOUT = (1<<25);
const unsigned long PLR_NODIMD     = (1<<26);
const unsigned long PLR_UNUSED5    = (1<<27);
const unsigned long PLR_KILLABLE   = (1<<28);   // switch off newbie pro
const unsigned long PLR_ANONYMOUS  = (1<<29);
const unsigned long PLR_SHOW_SAVES = (1<<30);

const unsigned short int PART_BLEEDING     = (1<<0);
const unsigned short int PART_INFECTED     = (1<<1);
const unsigned short int PART_PARALYZED    = (1<<2);
const unsigned short int PART_BROKEN       = (1<<3);
const unsigned short int PART_SCARRED      = (1<<4);
const unsigned short int PART_BANDAGED     = (1<<5);
const unsigned short int PART_MISSING      = (1<<6);
const unsigned short int PART_USELESS      = (1<<7);
const unsigned short int PART_LEPROSED     = (1<<8);
const unsigned short int PART_TRANSFORMED  = (1<<9);
const unsigned short int PART_ENTANGLED    = (1<<10);

const int MAX_PARTS          = 11;       // move and change

typedef struct _app_typ {
  bool assignable;
  const char *name;
} APP_type;

extern APP_type apply_types[MAX_APPLY_TYPES];

/* 'class' for PC's */
const ush_int CLASS_MAGE         = (1<<0);   // 1
const ush_int CLASS_CLERIC       = (1<<1);   // 2
const ush_int CLASS_WARRIOR      = (1<<2);   // 4
const ush_int CLASS_THIEF        = (1<<3);   // 8
const ush_int CLASS_SHAMAN       = (1<<4);   // 16 
const ush_int CLASS_DEIKHAN      = (1<<5);   // 32
const ush_int CLASS_MONK         = (1<<6);   // 64
const ush_int CLASS_RANGER       = (1<<7);   // 128

/* Bitvector for 'affected_by' */
const unsigned long AFF_BLIND             = (1<<0);        // 1
const unsigned long AFF_INVISIBLE         = (1<<1);        // 2
const unsigned long AFF_SWIM              = (1<<2);        // 4
const unsigned long AFF_DETECT_INVISIBLE  = (1<<3);        // 8
const unsigned long AFF_DETECT_MAGIC      = (1<<4);        // 16
const unsigned long AFF_SENSE_LIFE        = (1<<5);        // 32
const unsigned long AFF_LEVITATING        = (1<<6);        // 64
const unsigned long AFF_SANCTUARY         = (1<<7);        // 128
const unsigned long AFF_GROUP             = (1<<8);        // 256
const unsigned long AFF_WEB               = (1<<9);        // 512
const unsigned long AFF_CURSE             = (1<<10);       // 1024
const unsigned long AFF_FLYING            = (1<<11);       // 2048
const unsigned long AFF_POISON            = (1<<12);       // 4096
const unsigned long AFF_STUNNED           = (1<<13);       // 8192
const unsigned long AFF_PARALYSIS         = (1<<14);       // 16384
const unsigned long AFF_INFRAVISION       = (1<<15);       // 32768
const unsigned long AFF_WATERBREATH       = (1<<16);       // 65536
const unsigned long AFF_SLEEP             = (1<<17);       // 131072
const unsigned long AFF_SCRYING           = (1<<18);       // 262144
const unsigned long AFF_SNEAK             = (1<<19);       // 524288
const unsigned long AFF_HIDE              = (1<<20);       // 1048576
const unsigned long AFF_SHOCKED           = (1<<21);       // 2097152
const unsigned long AFF_CHARM             = (1<<22);       // 4194304
const unsigned long AFF_SYPHILIS          = (1<<23);       // 8388608
const unsigned long AFF_SHADOW_WALK       = (1<<24);       // 16777216
const unsigned long AFF_TRUE_SIGHT        = (1<<25);       // 33554432
const unsigned long AFF_MUNCHING_CORPSE   = (1<<26);       // 67108864
const unsigned long AFF_DRAGON_RIDE       = (1<<27);       // 134217728
const unsigned long AFF_SILENT            = (1<<28);       // 268435456
const unsigned long AFF_ENGAGER           = (1<<29);       // 436870912
const unsigned long AFF_AGGRESSOR         = (1<<30);       // 873741824 (set automatically)

// these are used to pass deletion bitvectors through functions
// it is used in same functions that return degree of damage done
// these values are essentially negative
//
// damage is sometimes passed in same manner as deletes, so make sure
// value for damage is always less than smallest DELETE value
//
// they are combinations of bits, IS_SET, REMOVE_BIT, SET_BIT can not be used
// use IS_SET_DELETE, ADD_DELETE, REM_DELETE instead
//
#if 0 
const int DELETE_ITEM          = ((1<<5));
const int DELETE_THIS          = ((1<<6));
const int DELETE_VICT          = ((1<<7));
const int ALREADY_DELETED      = ((1<<8));
const int RET_STOP_PARSING     = ((1<<9));

#elif 1
const int DELETE_ITEM          = ((1<<5) | (1<<29));
const int DELETE_THIS          = ((1<<6) | (1<<29));
const int DELETE_VICT          = ((1<<7) | (1<<29));
const int ALREADY_DELETED      = ((1<<8) | (1<<29));
const int RET_STOP_PARSING     = ((1<<9) | (1<<29));

#else
const int DELETE_ITEM          = ((1<<5) | (1<<31));
const int DELETE_THIS          = ((1<<6) | (1<<31));
const int DELETE_VICT          = ((1<<7) | (1<<31));
const int ALREADY_DELETED      = ((1<<8) | (1<<31));
const int RET_STOP_PARSING     = ((1<<9) | (1<<31));
#endif

// This was created to limit the size of getName()
// There are hundreds of places in the code that use char buf[80] to
// hold a copy of getName(), increasing this without changing all of
// those will cause BAD!!! memory corruptions
const int MAX_NAME_LENGTH = 80;

// parts of speech
const int POS_OBJECT            = 1;
const int POS_SUBJECT           = 2;
const int POS_POSSESS           = 3;

class playerData
{
  public:
   const char *longDescr;  

   sexTypeT sex;         
   ubyte level[MAX_SAVED_CLASSES];   
   ubyte max_level;

   ush_int Class;
   byte doneBasic[MAX_SAVED_CLASSES];
   short hometown;  
   territoryT hometerrain;
   struct time_data time;
   playerData();
   playerData(const playerData &a);
   playerData & operator=(const playerData &a);
   ~playerData();
};

class Craps;
class charFile;    // defined below

class pointData { // NOTE: pointdata is saved directly into charfile SO YOU CAN NOT CHANGE THIS SHIT
 public:// without charfile conversion or wipe or whatnot. 
   sh_int mana;         
   sh_int maxMana;
   double piety;
   sh_int lifeforce;
   sh_int hit;   
   sh_int maxHit;      
   sh_int move;  
   sh_int maxMove;     

   int money;        
   int bankmoney;        
   double exp;             
   double max_exp;

   sh_int spellHitroll;
   sh_int hitroll;     
   sbyte damroll;  
   sh_int armor;   // technically, in range -1000 to 1000

   pointData();
   pointData(const pointData &a);
   pointData & operator=(const pointData &a);
   ~pointData();

};

class specialData {
  
  friend class TBeing;

  private:
    sbyte conditions[MAX_COND_TYPE];        
  public:
    TBeing *fighting; 
    TBeing *hunting;  
    
    unsigned long affectedBy;  
  
    positionTypeT position;              
    dirTypeT last_direction;       
    editorChangeTypeT edit;
    byte editFriend;
    unsigned long act;
    int was_in_room;
    sh_int zone;

    specialData();
    ~specialData();
    specialData(const specialData &a);
    specialData & operator=(const specialData &a);
};

class immunityData {
  public:
    byte immune_arr[MAX_IMMUNES];
 
    immunityData();
    immunityData(const immunityData &a);
    immunityData & operator=(const immunityData &a);
    ~immunityData();
};

class factionData {
  public:
    double percent;
#if FACTIONS_IN_USE
    double percX[MAX_FACTIONS];       // used to calculate percent
#endif
    TBeing *captive;
    TBeing *next_captive;
    TBeing *captiveOf;
    TBeing *target;
    factionTypeT type;          // actual affiliation
    unsigned long actions;
    int align_ge;              // alignment on good/evil axis
    int align_lc;              // alignment on chaotic/lawful axis
    int whichfaction;
    int rank;                  // rank in the newfaction system
    factionData();
    factionData(const factionData &a);
    factionData & operator=(const factionData &a);
    ~factionData();
};

class pracData {
  public:
  sh_int prac[MAX_SAVED_CLASSES];
  
  pracData();
  pracData(const pracData &a);
  pracData & operator=(const pracData &a);
  ~pracData();
};

class bodyPartsDamage {
  private:
    unsigned short int flags;
    TThing *stuckIn;
    ubyte health;

  public:
    bodyPartsDamage();
    bodyPartsDamage(const bodyPartsDamage &a);
    bodyPartsDamage & operator=(const bodyPartsDamage &a);
    ~bodyPartsDamage();

    unsigned short int getFlags() const { return flags; }
    void setFlags(unsigned short int num) { flags = num; }
    void addFlags(unsigned short int num) { flags |= num; }
    void remFlags(unsigned short int num) { flags &= ~num; }
    TThing *getStuckIn() const { return stuckIn; }
    void setStuckIn(TThing *t) { stuckIn = t; }
    ubyte getHealth() const { return health; }
    void setHealth(ubyte num) { health = num; }
    void addHealth(ubyte num) { health += num; }
};

class followData {
  public:
    TBeing *follower;
    followData *next;
    followData();
    followData(const followData &a);
    followData & operator=(const followData &a);
    ~followData();
};

class skillApplyData {
  public:
    int skillNum;
    sbyte amount;
    sbyte numApplys;
    skillApplyData *nextApply;
    skillApplyData();
    skillApplyData(const skillApplyData &a);
    skillApplyData & operator=(const skillApplyData &a);
    ~skillApplyData();
};


class spellTaskData {
  public:
    spellNumT spell;
    sbyte target;
    int start_pos;
    int distracted;
    taskDiffT difficulty;
    bool component;
    int nextUpdate;
    int rounds;
    const char *orig_arg;
    int wasInRoom;
    int status;
    int text;
    unsigned int flags;
    TBeing *victim;
    TObj *object;
    TRoom *room;
    void getNextUpdate(int, int);

    spellTaskData();
    spellTaskData(const spellTaskData &a);
    spellTaskData & operator=(const spellTaskData &a);
    ~spellTaskData();
};

class spellStoreData {
public:
  spellTaskData *spelltask;
  bool storing;

  spellStoreData();
  spellStoreData(const spellStoreData &a);
  spellStoreData & operator=(const spellStoreData &a);
  ~spellStoreData();
};

class taskData {
  public:
    taskTypeT task;
    int nextUpdate;
    int timeLeft;
    const char *orig_arg;
    int wasInRoom;
    ubyte status;
    int flags;
    TObj *obj;
    TRoom *room;
    void calcNextUpdate(int, int);
    taskData();
    taskData(const taskData &a);
    taskData & operator=(const taskData &a);
    ~taskData();
};

class equipmentData {
 private:
  TThing *equipment[MAX_WEAR];

 public:
  TThing *operator[] (int slot) const {
    return this->equipment[slot]; 
  }

  float getWeight(){
    float total=0;
    for(wearSlotT i=MIN_WEAR;i<MAX_WEAR;i++){
      if(equipment[i])
	total+=equipment[i]->getWeight();
    }
    return total;
  }


  TThing *remove(enum wearSlotT slot){
    TThing *t=equipment[slot];

    TObj *tobj = dynamic_cast<TObj *>(t);
    if (tobj && tobj->usedAsPaired()) {
      if (slot == WEAR_LEGS_R || 
	  slot == HOLD_RIGHT || 
	  slot == WEAR_EX_LEG_R)
	equipment[slot + 1] = NULL;
      else
	equipment[slot - 1] = NULL;
    }

    equipment[slot]=NULL;
    return t;
  }

  void wear(TThing *t, enum wearSlotT slot){
    TObj *tobj = dynamic_cast<TObj *>(t);
    if (tobj && tobj->usedAsPaired()) {
      if (slot == WEAR_LEGS_R ||
	  slot == HOLD_RIGHT || 
	  slot == WEAR_EX_LEG_R)
	equipment[slot + 1] = t;
      else
	equipment[slot - 1] = t;
    }

    equipment[slot]=t;
  }

   equipmentData();
   equipmentData(const equipmentData &a);
   equipmentData & operator=(const equipmentData &a);
   ~equipmentData();
};



class TBeing : public TThing {

    friend class Descriptor;
    friend class Body;
    friend class Stats;

  protected:
    Race *race;
    Body *body;
    pointData points;
    Stats chosenStats;
    Stats curStats;

  private:
    double multAtt;

    int heroNum;
  public:
    Race *disguise_race; // for polymorph/disguise code
    bool toggles[MAX_TOG_INDEX];
    Craps * m_craps;
  private:
    byte invisLevel;
    byte my_protection;       // % reduction for sanct
    bodyPartsDamage body_parts[MAX_WEAR];
    attack_mode_t combatMode;      

  public:
    factionData faction;
    CMasterDiscipline *discs;
    bool inPraying;
    bool inQuaffUse;
    sh_int attackers;
    byte visionBonus;
    sh_int age_mod;
    byte cantHit; 
    sh_int wait;   // this goes up in multiples of COMBAT_ROUND (=20)
    polyTypeT polyed;         
    sh_int hunt_dist;    // used by track, mob-hunting, etc
    short wimpy;        

    immunityData immunities;
    playerData player;      
    specialData specials;  
    pracData practices; 
    affectedData *affected;    
    equipmentData equipment;



    TBeing *master;            
    TPerson *orig;    // a pointer to who I really am (if poly'd)
    TBeing *next;           
    TBeing *next_fighting; 
    TBeing *next_caster;
 
    followData *followers;
     
    spellTaskData *spelltask;
    spellStoreData spellstore;
    taskData *task;      
    skillApplyData *skillApplys;
    TTrophy *trophy;

    TMessages msgVariables;
    // Constructor and Destructor
  protected:
    TBeing();
  public:
    TBeing(const TBeing &);
    virtual ~TBeing();
    TBeing & operator= (const TBeing &);
    virtual TThing& operator+= (TThing& t);
    virtual TThing& operator-- ();

    // VIRTUAL FUNCTIONS
    virtual int editAverageMe(TBeing *, const char *);
    virtual const char * getLongDesc() const;
    virtual int chiMe(TBeing *);
    virtual const char *getName() const;
    virtual int getPlayerID() const;
    virtual void remCastingList(TThing *);
    virtual roomDirData *exitDir(dirTypeT door) const;
    virtual int mobVnum() const { return 0; }
    virtual short int manaLimit() const;
    virtual int getMaxMove() const;
    virtual int maxWimpy();
    virtual int getWimpy();
    virtual void setWimpy(int);
    virtual short int hitLimit() const;
    virtual int fallOffMount(TThing *, positionTypeT, bool death = FALSE);
    virtual bool hasQuestBit(int) const;
    virtual void setQuestBit(int);
    virtual void remQuestBit(int);
    bool powerCheck(wizPowerT) const;
    virtual bool hasWizPower(wizPowerT) const;
    virtual void setWizPower(wizPowerT);
    virtual void remWizPower(wizPowerT);

    bool limitPowerCheck(cmdTypeT, int);
    bool isGenericObj(int);
    bool isGenericMob(int);

    virtual sstring yourDeity(spellNumT, personTypeT, const TBeing *who = NULL) const;
    virtual bool isPolice() const { return false; }
    virtual bool isDiurnal() const { return false; }
    virtual bool isNocturnal() const { return false; }
    virtual bool isShopkeeper() const { return false; }
    virtual int pourWaterOnMe(TBeing *, TObj *);
    virtual int getSnum() const { return (snum > -1 ? snum : mobVnum()); };
    virtual sstring thirdPerson(const int);
    // END VIRTUAL FUNCTIONS

    // INLINE FUNCTIONS
    // END INLINE FUNCTIONS

    // pets.cc
    int getAffectedDataFromType(spellNumT, double);
    int getPetOrderLevel();
    int getPetAge();
    bool doRetrainPet(const char *, TBeing *);
    bool restorePetToPc(TBeing *tBeing);

    ubyte GetMaxLevel() const;
    void setMaxLevel(ubyte num);
    sstring const getProfName() const;
    const char * const getProfAbbrevName() const;
    void deityIgnore(silentTypeT = SILENT_NO) const;
    void nothingHappens(silentTypeT = SILENT_NO) const;
    float percModifier() const;
    attack_mode_t getCombatMode() const;
    bool isCombatMode(attack_mode_t n) const;
    bool inQuest() const;
    bool banished() const;
    bool isRightHanded() const;
    bool isPlayerAction(unsigned long num) const;
    void remPlayerAction(unsigned long num);
    void addPlayerAction(unsigned long num);
    bool hasColor() const;
    bool hasColorVt() const;
    const sstring doColorSub() const;
    int getScreen() const;
    bool color() const;
    bool ansi() const;
    bool vt100() const;
    void cls() const;
    void fullscreen() const;
    const sstring ansi_color_bold(const char *s) const;
    const sstring ansi_color_bold(const char *s, unsigned int) const;
    const sstring ansi_color(const char *s) const;
    const sstring ansi_color(const char *s, unsigned int) const;
    const char *highlight(char *s) const;
    const char *whiteBold() const;
    const char *blackBold() const;
    const char *redBold() const;
    const char *underBold() const;
    const char *blueBold() const;
    const char *cyanBold() const;
    const char *greenBold() const;
    const char *orangeBold() const;
    const char *purpleBold() const;
    const char *white() const;
    const char *black() const;
    const char *red() const;
    const char *under() const;
    const char *bold() const;
    const char *norm() const;
    const char *blue() const;
    const char *cyan() const;
    const char *green() const;
    const char *orange() const;
    const char *purple() const;
    const char *invert() const;
    const char *flash() const;
    const char *BlackOnBlack() const;
    const char *BlackOnWhite() const;
    const char *WhiteOnBlue() const;
    const char *WhiteOnCyan() const;
    const char *WhiteOnGreen() const;
    const char *WhiteOnOrange() const;
    const char *WhiteOnPurple() const;
    const char *WhiteOnRed() const;

    bool bSuccess(spellNumT);
    bool bSuccess(int, spellNumT);
    bool bSuccess(int, double, spellNumT);
    race_t getRaceIndex(const char *name) const;
    bool isWinged() const;
    bool isFourLegged() const;
    int getAdvLearning(spellNumT) const;
    int getAdvDoLearning(spellNumT) const;
    spellNumT getSkillNum(spellNumT) const;
    int getSkillDam(const TBeing *, spellNumT, int, int) const;
    void assignCorpsesToRooms();
    void initSkillsBasedOnDiscLearning(discNumT);
    void addAffects(const TObj *);

    bool invalidTarget(const TBeing *target) const;
    bool canFight(TBeing *);
    bool canAttack(primaryTypeT);
    int attackRound(const TBeing *target) const;
    int defendRound(const TBeing *attacker) const;
    int specialAttack(TBeing *target, spellNumT);

    void updateStatistics();
    bool checkForDiceHeld() const;
    TObj *checkForDiceInInv() const;
    void setCombatMode(attack_mode_t n);

    int hits(TBeing *, int);
    int missVictim(TBeing *, TThing *, spellNumT);

    // Postmaster
    void postmasterSendMail(const char *, TMonster *);
    void postmasterReceiveMail(TMonster *);
    void postmasterCheckMail(TMonster *);

    void checkForSpills() const;

    void fixLevels(int);
    bool anythingGetable(const TObj *, const char *) const;

    // tracking stuff
    int track(TBeing *);
    dirTypeT dirTrack(TBeing *);

    int goDirection(dirTypeT); // for hunt?
    void fixClientPlayerLists(bool);
    void gainExpPerHit(TBeing *, double, int);

    void moneyCheck();
    int rideCheck(int);
    spellNumT mountSkillType() const;
    void calmMount(TBeing *);
    int advancedRidingBonus(TMonster *);

    void wipeChar(int);
    void resetEffectsChar();
    void initDescStuff(charFile *);
    int followTime() const;
    bool tooManyFollowers(const TBeing *, newFolTypeT) const;
    int numClasses() const;

    int triggerSpecial(TThing *, cmdTypeT cmd, const char *arg); 
    int triggerSpecialOnPerson(TThing *, cmdTypeT cmd, const char *arg); 
    void sendCastingMessages(bool, bool, int, skillUseTypeT, int);
    void sendFinalCastingMessages(bool, bool, skillUseTypeT);

    bool isValidDiscClass(discNumT, int, int); // disc, classNum, class_ind
    CDiscipline *getDiscipline(discNumT n) const;
    CSkill *getSkill(spellNumT n) const;

    int checkDecharm(forceTypeT, safeTypeT = SAFE_NO);
    int riverFlow(int);
    int applyAutorentPenalties(int);
    int applyRentBenefits(int);
    void verifyWeightVolume();
    virtual int getVolume() const;
    virtual float carryWeightLimit() const;
    virtual int carryVolumeLimit() const;
    float maxWieldWeight(const TThing *, handTypeT) const;
    int checkWeaponWeight(TThing *, handTypeT, bool);
    bool canCarry(const TThing *, silentTypeT) const;
    virtual bool isDragonRideable() const = 0;

    wearSlotT getPrimaryArm() const;
    wearSlotT getSecondaryArm() const;
    wearSlotT getPrimaryHold() const;
    wearSlotT getSecondaryHold() const;
    wearSlotT getPrimaryHand() const;
    wearSlotT getSecondaryHand() const;
    wearSlotT getPrimaryFinger() const;
    wearSlotT getSecondaryFinger() const;
    wearSlotT getPrimaryWrist() const;
    wearSlotT getSecondaryWrist() const;
    wearSlotT getPrimaryFoot() const;
    wearSlotT getSecondaryFoot() const;
    wearSlotT getPrimaryLeg() const;
    wearSlotT getSecondaryLeg() const;

    bool isAmbidextrous() const {
      return ((getStat(STAT_CURRENT, STAT_DEX) > 180) ||
		(race->getRace() == RACE_HOBBIT));
    }
    void logItem(const TThing *, cmdTypeT) const;
    int dentItem(TBeing *, TObj *, int, int);
    int tearItem(TBeing *, TObj *, int, int);
    int pierceItem(TBeing *, TObj *, int, int);
    int wear(TObj *, wearKeyT, TBeing *);
    int remove(TThing *, TBeing *);
    virtual bool canGet(const TThing *, silentTypeT) const;
    virtual bool canGetMe(const TBeing *, silentTypeT) const;
    void aiGet(TThing *);
    bool hasBoat() const;
    bool hasSaddle() const;
    bool canRide(const TBeing *) const;
    virtual int getNumRiders(TThing *) const;
    virtual int getMaxRiders() const;
    virtual int getRiderHeight() const;
    void updateCharObjects();
    void equipChar(TThing *, wearSlotT, silentTypeT = SILENT_NO);
    void reconnectEquipmentHandler(void);
    TThing *pulloutObj(wearSlotT, bool, int *);
    bool hasTransformedLimb() const;
    bool isSingleClass() const;
    bool isDoubleClass() const;
    bool isTripleClass() const;
    classIndT bestClass() const;
    int getClassLevel(int) const;
    void loseSneak();
    int checkEngagementStatus();
    virtual void parseTitle(char *, Descriptor *);
    int onlyClass(int) const;
    int getClassNum(const char *, exactTypeT);
    int getClassNum(classIndT);
    classIndT getClassIndNum(const char *, exactTypeT);
    classIndT getClassIndNum(ush_int, exactTypeT);
    bool hasClass(const char *, exactTypeT) const;
    bool hasClass(ush_int, exactTypeT = EXACT_NO) const;
    void setQuaffUse(bool tmp) { inQuaffUse = tmp; }
    bool getQuaffUse() const { return inQuaffUse; }
    void convertAbilities();
    int setStat(statSetT whichSet, statTypeT whichStat, int value);
    int addToStat(statSetT whichSet, statTypeT whichStat, int modifier);
    int getStat(statSetT fromSet,statTypeT whichStat) const;
    
    double plotStat(statSetT, statTypeT, double, double, double, double n = 1.4) const;
    int plotStat(statSetT, statTypeT, int a, int b, int c, double n = 1.4) const;
    float plotStat(statSetT, statTypeT, float a, float b, float c, double n = 1.4) const;
    Stats getCurStats() const;

    void checkForStr(silentTypeT);
    void doAfk();
    int addCommandToQue(const sstring &);
    int loseRoundWear(double, bool randomize = TRUE, bool check = FALSE);
    int loseRound(double, bool x = TRUE, bool check = FALSE);
    void blowCount(bool, float &, float &);
    bool checkBusy(const sstring &buf="");
    float lagAdjust(lag_t);
    void addSkillLag(spellNumT, int rc);
    virtual void addToWait(int) { return; }
    virtual int getWait(void) const { return 0; }
    virtual void setWait(int) { return; }
    wizardryLevelT getWizardryLevel() const;
    ritualismLevelT getRitualismLevel() const;
    devotionLevelT getDevotionLevel() const;
    void addCaptive(TBeing *);
    void remCaptive(TBeing *);

    virtual byte getImmunity(immuneTypeT type) const;
    virtual void setImmunity(immuneTypeT type, byte amt);
    virtual void addToImmunity(immuneTypeT type, byte amt);
    bool isImmune(immuneTypeT, int modifier = 0) const;
    bool isLucky(int) const;
    int spellLuckModifier(spellNumT);
    void spellMessUp(spellNumT);
    void disturbMeditation(TBeing *) const;
    void setCond(condTypeT i, sbyte val);
    sbyte getCond(condTypeT i) const;
    void gainCondition(condTypeT, int);
    int drunkMinus() const;
    void calcMaxLevel();
    int howManyClasses() const;
    int GetTotLevel() const;
    void affectTo(affectedData *, int renew = 0, silentTypeT = SILENT_NO);
    void affectTotal();
    void affectModify(applyTypeT, long, long, unsigned long, bool, silentTypeT);
    void affectChange(unsigned long, silentTypeT);
    void affectRemove(affectedData *, silentTypeT = SILENT_NO);
    void affectFrom(spellNumT);
    void addObjUsed(TObj *, int);
    bool checkObjUsed(TObj *);
    void transformLimbsBack(const char *, wearSlotT, bool);
    bool hasDisease(diseaseTypeT) const;
    void diseaseFrom(diseaseTypeT);
    int affectJoin(TBeing *, affectedData *, avgDurT, avgEffT, bool text = TRUE);
    int polyAffectJoin(TBeing *);
    void classSpecificStuff();
    virtual int hitGain() = 0;
    virtual int manaGain() = 0;
    int moveGain();
    double pietyGain(double);
    void goBerserk(TBeing *);
    void checkForQuestTog(TBeing *);
    void sendCheatMessage(char *);
    void stopFighting();
    int canBeParalyzeLimbed();
    int checkIdling();
    bool canSpeak();
    int applySoundproof() const;
    void aiWear(TObj *);
    int defaultLimbConnections(wearSlotT);
    int limbConnections(wearSlotT);
    void makePartMissing(wearSlotT, bool, TBeing * = NULL);
    void makeLimbTransformed(TBeing *, wearSlotT, bool);
    void doTransformDrop(wearSlotT);
    int teleportRoomFlow(int);
    int portalLeaveCheck(char *, cmdTypeT);
    int updateAffects();
    int updateTickStuff();
    int updateBodyParts();
    int updateHalfTickStuff();
    int terrainSpecial();
    bool isVampire() const;
    bool isAnimal() const;
    bool isVeggie() const;
    bool isDiabolic() const;
    bool isReptile() const;
    bool isUndead() const;
    bool isGiantish() const;
    bool isPeople() const;
    bool isOther() const;
    bool isLycanthrope() const;
    bool isColdBlooded() const;
    bool isHumanoid() const { return race->isHumanoid(); }
    bool noHarmCheck(TBeing *);
    void loadSetEquipment(int, char *, int);
    bool doesKnowSkill(spellNumT) const;
    int getSkillLevel(spellNumT) const;
    byte getMaxSkillValue(spellNumT) const;
    byte getNatSkillValue(spellNumT) const;
    void setNatSkillValue(spellNumT, int);
    byte getSkillValue(spellNumT) const;
    void setSkillValue(spellNumT, int);
    byte getRawNatSkillValue(spellNumT) const;
    byte getRawSkillValue(spellNumT) const;
    virtual immortalTypeT isImmortal(int = GOD_LEVEL1) const { return IMMORTAL_NO; }

    virtual ush_int getBaseAge() const {
      // ideally this should be a pure virtual with this functionality
      // in TMonster.  ~TBeing() makes use of age (via affectTotal) so
      // some functionality is needed here.
      return 17;
    }

    virtual void showTo(const TThing *t, showModeT i);
    virtual void showMultTo(const TThing *t, showModeT i, unsigned int n);
    virtual bool isSimilar(const TThing *t) const;
    virtual void show_me_mult_to_char(TBeing *, showModeT, unsigned int) const;
    virtual void show_me_to_char(TBeing *, showModeT) const;

    virtual const char *hshr() const;
    virtual const char *hssh() const;
    virtual const char *hmhr() const;

    bool isExtraPlanar() const;
    void setRacialStuff();
    TThing * unequip(wearSlotT);
    void setPosition(positionTypeT);
    positionTypeT getPosition() const;
    bool isAquatic() const;
    bool isDumbAnimal() const { return race->isDumbAnimal(); }
    int isNotPowerful(TBeing *, int, spellNumT, silentTypeT);
    virtual bool hasHands() const;
    void raiseDiscOnce(discNumT);
    int calcRaiseDisc(discNumT, bool) const;
    int pracsBetween(discNumT, int) const;
    double pracsPerLevel(classIndT Class, bool forceBasic);
    sh_int calcNewPracs(classIndT, bool);
    sh_int getPracs(classIndT);
    void setPracs(sh_int, classIndT);
    void addPracs(sh_int, classIndT);
    dirTypeT findDoor(const char *, const char *, doorIntentT, silentTypeT silent = SILENT_YES);

    // this can't be const due to list_thing_in_heap
    void windowLook(const TWindow *);

    void dropWeapon(wearSlotT);
    void shatterWeapon(wearSlotT, int);
    bool hasLegs() const;
    void findFood();
    TBeing *findDiffZoneSameRace();
    int validEquipSlot(wearSlotT);
    void assignSkillsClass();
    void assignSkills(classIndT, discNumT, vector<discNumT>);
    void assignDisciplinesClass();
    const sstring bogus_slot(wearSlotT) const;
    const sstring bogus_slot_worn(wearSlotT) const;
    const sstring default_body_slot(wearSlotT) const;
    const sstring defaultEquipmentSlot(wearSlotT) const;
    const sstring slotPlurality(int) const;
    const sstring describeBodySlot(wearSlotT) const;
    const sstring describeBodySlot2(wearSlotT) const;
    const sstring describeEquipmentSlot(wearSlotT) const;
    int shouldDescTransLimb(wearSlotT) const;
    bool isTransformableLimb(wearSlotT, int);
    const sstring describeTransLimb(wearSlotT) const;
    const sstring describeTransBodySlot(wearSlotT) const;
    const sstring describeTransEquipSlot(wearSlotT) const;
    sstring describeImmunities(const TBeing *, int) const;
    void describeObject(const TThing *);
    sstring describeSharpness(const TThing *) const;
    sstring describePointiness(const TBaseWeapon *) const;
    sstring describeBluntness(const TBaseWeapon *) const;
    void describeMaxStructure(const TObj *, int) const;
    void describeMaxSharpness(const TBaseWeapon *, int) const;
    void describeMaxBluntness(const TBaseWeapon *, int) const;
    void describeMaxPointiness(const TBaseWeapon *, int) const;
    void describeOtherFeatures(const TGenWeapon *, int) const;
    void describeBowRange(const TBow *, int);
    void describeArrowDamage(const TArrow *, int);
    void describeArrowSharpness(const TArrow *, int);
    void describeNoise(const TObj *, int) const;
    void describeArmor(const TBaseClothing *, int);
    void describeRoomLight();
    void describeWeaponDamage(const TBaseWeapon *, int) const;
    void describeMagicLevel(const TMagicItem *, int) const;
    void describeMagicLearnedness(const TMagicItem *, int) const;
    void describeMagicSpell(const TMagicItem *, int);
    void describeSymbolOunces(const TSymbol *, int) const;
    void describeComponentSpell(const TComponent *, int) const;
    void describeComponentUseage(const TComponent *, int) const;
    void describeComponentDecay(const TComponent *, int) const;
    void describeTrapEffect(const TTrap *, int) const;
    void describeTrapLevel(const TTrap *, int) const;
    void describeTrapCharges(const TTrap *, int) const;
    void describeTrapDamType(const TTrap *, int) const;
    bool castDenyCommand(cmdTypeT);
    bool castAllowCommand(cmdTypeT);
    bool utilityTaskCommand(cmdTypeT);
    bool nobrainerTaskCommand(cmdTypeT);
    void listExits(const TRoom *) const;
    virtual bool listThingRoomMe(const TBeing *) const;
    void displayHelpFile(char *, char *);
    void genericKillFix();
    virtual int genericMovedIntoRoom(TRoom *, int, checkFallingT = CHECK_FALL_YES);
    int genericItemCheck(TThing *);
    void genericEvaluateItem(const TThing *);
    void preKillCheck(bool rent = FALSE);
    int regenTime();
    void describeWeather(int);
    int dummyCold();
    int dummyFlu();
    void dummyLeprosy(wearSlotT);
    void bodySpread(int, affectedData *);
    wearSlotT getPartHit(TBeing *, bool);
    wearSlotT getCritPartHit();
    bool makesNoise() const;
    int addToDistracted(int, int);
    int remFromDistracted(int);
    int clearDistracted();
    int setDistracted(int, int);
    int getDistracted();
    void stopCast(stopCastT);
    void stopTask();
    time_info_data *age() const;
    int moveOne(dirTypeT);
    int moveGroup(dirTypeT);
    int displayOneMove(dirTypeT, int);
    int displayGroupMove(dirTypeT, int, int);
    int displayMove(dirTypeT, int, int);
    virtual float getTotalWeight(bool) const;
    int rawMove(dirTypeT);
    int rawSummon(TBeing *);
    void rawBlind(int, int, saveTypeT);
    int rawSleep(int, int, int, saveTypeT);
    int rawBleed(wearSlotT, int, silentTypeT, checkImmunityT);
    int dropPool(int, liqTypeT);
    int dropBloodLimb(wearSlotT);
    int rawInfect(wearSlotT, int, silentTypeT, checkImmunityT);
    int tooTired();
    void deathCry();
    void appear();
    void updatePos();
    int reconcilePiety(spellNumT, bool);
    int reconcileMana(spellNumT, bool, int = 0);
    int useMana(spellNumT);
    int reconcileLifeforce(spellNumT, bool, int = 0);
    int useLifeforce(spellNumT);
    double usePiety(spellNumT);
    int reconcileDamage(TBeing *, int, spellNumT);
    int doRent(const sstring &);
    void doRestring(const sstring &);
    void doRelease(const sstring &);
    void doCapture(const sstring &);
    void doCompare(const char *);
    void doMortalCompare(const char *);
    void doTestFight(const char *);
    void doList(const char *);
    void doFactions(const sstring &);
    void doClients();
    int doCrit(sstring);
    void incorrectCommand() const;
    void doEgoTrip(const char *);
    void doComment(const char *);
    int doCommand(cmdTypeT, const sstring &, TThing *, bool);
    int doCharge(const char *, TBeing *);
    void doChargeStave(sstring);
    int doSmite(const char *, TBeing *);
    void doDescription();
    int doDissect(sstring);
    int socialLimbBad(TBeing *, cmdTypeT);
    int doPray(const char *);
    int preCastCheck();
    int preDiscCheck(spellNumT);
    int doCast(const char *);
    spellNumT parseSpellNum(char *);
    int parseTarget(spellNumT, char *, TThing **ret);
    int doTrigger(const char *);
    int doStore(const char *);
    int passOut();
    int shieldAbsorbDamage(int);
    void chlorineRoom();
    int flameEngulfed();
    int frostEngulfed();
    int acidEngulfed();
    int lightningEngulfed();
    int chlorineEngulfed();
    void flameRoom();
    void acidRoom();
    void freezeRoom();

    int parseCommand(const sstring &, bool);
    TComponent *findComponent(spellNumT) const;
    TSymbol *findHolySym(silentTypeT) const;
    int enforceVerbal();
    int doSpellCast(TBeing *, TBeing *, TObj *, TRoom *, spellNumT, skillUseTypeT);
    int checkBadSpellCondition(TBeing *, int);
    int cast_spell(TBeing *, cmdTypeT, int);
    int checkHolySymbol(spellNumT);
    int applyCompCheck(spellNumT, int, int);
    int useComponent(TComponent *, TBeing *, checkOnlyT = CHECK_ONLY_NO);
    int useComponentObj(TComponent *, TObj *, checkOnlyT = CHECK_ONLY_NO);
    void failCalm(TBeing *);
    virtual void failCharm(TBeing *) = 0;
    void failSleep(TBeing *);
    void failPara(TBeing *);
    bool inGroup(const TBeing &) const;
    int inCamp() const;
    int bumpHead(int *);
    virtual int bumpHeadDoor(roomDirData *, int *);
    bool willBump(int) const;
    bool willBumpHead(TRoom *) const;
    bool willBumpHeadDoor(roomDirData *, int *) const;
    void sendTrapMessage(const char *, trap_targ_t, int);
    bool hasTrapComps(const char *, trap_targ_t, int, int *price = NULL);
    int goofUpTrap(doorTrapT, trap_targ_t);
    int springTrap(TTrap *);
    int triggerTrap(TTrap *);
    int triggerDoorTrap(dirTypeT);
    int triggerPortalTrap(TPortal *);
    int triggerContTrap(TOpenContainer *);
    int triggerArrowTrap(TArrow *);
    int checkForMoveTrap(dirTypeT);
    int checkForInsideTrap(TThing *);
    int checkForGetTrap(TThing *);
    int checkForAnyTrap(TThing *);
    int trapDoorSlashDamage(int, dirTypeT);
    int trapDoorFireDamage(int, dirTypeT);
    int trapDoorPierceDamage(int, dirTypeT);
    int trapDoorTntDamage(int, dirTypeT);
    int trapDoorAcidDamage(int, dirTypeT);
    int trapDoorHammerDamage(int, dirTypeT);
    int trapDoorEnergyDamage(int, dirTypeT);
    int trapDoorFrostDamage(int, dirTypeT);
    virtual int grenadeHit(TTrap *);
    virtual bool addHated(TBeing *);
    virtual void setHunting(TBeing *) {}
    void throwGrenade(TTrap *, dirTypeT);
    int getDoorTrapDam(doorTrapT);
    int getContainerTrapDam(doorTrapT);
    int getMineTrapDam(doorTrapT);
    int getGrenadeTrapDam(doorTrapT);
    int getArrowTrapDam(doorTrapT);
    int getDoorTrapLearn(doorTrapT);
    int getContainerTrapLearn(doorTrapT);
    int getMineTrapLearn(doorTrapT);
    int getGrenadeTrapLearn(doorTrapT);
    int getArrowTrapLearn(doorTrapT);
    bool canDoSummon() const;
    bool isSummonable() const;
    bool isTanking();
    int spellWearOff(spellNumT, safeTypeT = SAFE_NO);
    void spellWearOffSoon(spellNumT);
    int diseaseStop(affectedData *);
    bool canSeeWho(const TBeing *) const;
    virtual bool canSeeMe(const TBeing *, infraTypeT) const;
    void wizFileRead();
    virtual void wizFileSave() = 0;
    void checkWeatherConditions();
    int slamIntoWall(roomDirData *);
    int bashFail(TBeing *, spellNumT);
    int bashSuccess(TBeing *, spellNumT);
    int tripFail(TBeing *, spellNumT);
    int tripSuccess(TBeing *, spellNumT);
    void bandage(TBeing *, wearSlotT);
    virtual int learnFromDoingUnusual(learnUnusualTypeT, spellNumT, int) = 0;
    virtual int learnFromDoing(spellNumT, silentTypeT, unsigned int) = 0;
    bool canBash(TBeing *, silentTypeT);
    bool canTrip(TBeing *, silentTypeT);
    bool canDisarm(TBeing *, silentTypeT);
    bool canStomp(TBeing *, silentTypeT);
    bool canHeadbutt(TBeing *, silentTypeT);
    bool canBodyslam(TBeing *, silentTypeT);
    bool canSpin(TBeing *, silentTypeT);
    bool canKick(TBeing *, silentTypeT);
    bool canKneestrike(TBeing *, silentTypeT);
    bool canGrapple(TBeing *, silentTypeT);
    bool canDeathstroke(TBeing *, silentTypeT);
    bool canWither(TBeing *, silentTypeT);
    bool canBoneBreak(TBeing *, silentTypeT);
    bool canParalyzeLimb(TBeing *, silentTypeT);
    int weaponImmune();
    int checkSinking(int);
    int checkDrowning();
    bool canClimb();
    bool canFly() const;
    bool sectorSafe();

    // this can't be const due to checkDoneBasic
    void pracPath(TMonster *, classIndT);

    void dropLevel(classIndT);
    virtual int trapSleep(int);
    virtual void trapPoison(int);
    virtual void trapDisease(int);
    virtual int trapTeleport(int);
    void informMess();
    int objDam(spellNumT, int, TThing *);
    int objDamage(spellNumT, int, TThing *);
    virtual void sendTo(colorTypeT, const sstring &) const;
    virtual void sendTo(const sstring &) const;
    void sendRoomName(TRoom *) const;
    void sendRoomDesc(TRoom *) const;
    void addFollower(TBeing *, bool = FALSE);
    void addSkillApply(spellNumT, sbyte);
    void remSkillApply(spellNumT, sbyte);
    virtual int checkFalling();
    bool circleFollow(const TBeing *) const;
    void saySpell(spellNumT);
    void stopFollower(bool, stopFollowerT = STOP_FOLLOWER_DEFAULT);
    void checkCharmMana();
    bool fallingMobHitMob(TRoom *, int);
    int fallKill();
    int mostPowerstoneMana() const;
    bool affectedBySpell(spellNumT) const;
    int checkForSkillAttempt(spellNumT);
    void removeSkillAttempt(spellNumT);

    int removeCurseObj(TObj *, int, byte, spellNumT);
    void removeCurseObj(TObj *);
    void removeCurseObj(TObj *, TMagicItem *, spellNumT);
    int removeCurseBeing(TBeing *, int, byte, spellNumT);
    void removeCurseBeing(TBeing *);
    void removeCurseBeing(TBeing *, TMagicItem *, spellNumT);

    void addObjCost(TBeing *, TObj *, objCost *, sstring &str); 
    TObj *findMostExpensiveItem();
    bool recepOffer(TBeing *, objCost *);
    void makeRentNote(TBeing *);
    void removeFollowers();
    void removeRent();
    void removePlayerFile();
    void saveCareerStats();
    void loadCareerStats();
    void saveDrugStats();
    void loadDrugStats();
    void saveTitle();
    void loadTitle();
    void saveFactionStats();
    void loadFactionStats();
    bool saveFollowers(bool);
    bool loadFollowers();
    void goThroughPortalMsg(const TPortal *) const;
    void exitFromPortalMsg(const TPortal *) const;
    int rawOpen(TThing *);
    void rawOpenDoor(dirTypeT);
    void rawCloseDoor(dirTypeT);
    void rawUnlockDoor(roomDirData *, dirTypeT);
    void notLegalMove() const;
    void putOutLightsInWater();
    spellNumT monkDamType() const;
    void openUniqueDoor(dirTypeT, doorUniqueT, const char *, const char *, const char *, const char *, const char *, const char *, const char *, const char *);
    int doDonate(int);
    int doMorgue();
    void doZones(sstring);
    void doZonesSingle(sstring);
    int eyeSight(TRoom *) const;
    int doExec();
    void doEvaluate(const char *);
    void igniteObject(const char *, TThing *);
    virtual void doEdit(const char *);
    virtual void doRload(const char *);
    virtual void doRsave(const char *);
    virtual void doRedit(const char *);
    virtual void doPowers(const sstring &) const;
    int getFactionAuthority(factionTypeT, int);
    void doMakeLeader(const char *);
    void doNewMember(const char *);
    void doRMember(const char *);
    void doDisband();
    void doSend(sstring);
    void doHelp(const char *);
    int doEncamp();
    int doSoothBeast(const char *);
    int doBefriendBeast(const char *);
    int doCharmBeast(const char *);
    int doTransfix(const char *);
    int doSummonBeast(const char *);
    int doBarkskin(const char *);
    int doFeralWrath(const char *);
    int doSkySpirit(const char *);
    int doEarthmaw(const char *);
    int doTransform(const char *);
    void playsound(soundNumT, const sstring &, int vol = 100, int prior = 50, int loop = 1);
    void playmusic(musicNumT, const sstring &, int vol = 100, int cont = 1, int loop = 1);
    void stopsound();
    void stopmusic();
    void doPlay(const char *);
    void doSort(const char *) const;
    void doGamestats(const sstring &);
    void doScan(const char *);
    int dislodgeWeapon(TBeing *, TThing *, wearSlotT);
    void doInsult(const char *);
    void doStay();
    void doEmail(const char *);
    void doDeal(const char *);
    void doPass(const char *);
    int doAs(const char *);
    int doAsOther(const sstring &);
    void doAttack(const char *);
    void doBet(const char *);
    void doCall(const sstring &);
    void doFold(const sstring &);
    void doBreak(const char *, int);
    void doAccount(const sstring &);
    virtual void doOEdit(const char *);
    void doScratch(const char *);
    void doPee(const sstring &);
    void doPoop(void);
    void doPoint(const sstring &);
    void doPoke(const sstring &);
    void doTip(const sstring &);
    int doBite(const sstring &);
    int doJump(const sstring &);
    int doPick(const char *);
    int doSearch(const char *);
    int doSpy();
    int doDodge();
    int doParry();
    int SpyCheck();
    int disarmTrap(const char *, TObj *);
    int detectTrap(const char *, int);
    int doSetTraps(const char *);
    int doBerserk();
    int doShoot(const char *);
    void doSeekwater();
    void doTrack(const char *);
    void doConceal(sstring);
    void doDivine(const char *);
    void doForage();
    void doTan();
    void doButcher();
    int  doApplyHerbs(const sstring &);
    void doSkin(const char *);
    void doButcher(const char *);
    int doPlant(sstring);
    int doSeedPlant(sstring);
    int doThiefPlant(sstring);
    void doCook(sstring);
    void doDrive(sstring);
    void doSacrifice(const char *);
    void doWhittle(const char *);
    void doBload(const char *);
    vector <TBow *> getBows();
    TArrow * autoGetAmmoQuiver(TBow *, TQuiver *);
    TArrow * autoGetAmmo(TBow *);
    void doGload(sstring);
    void doPrerequisite(const char *, int);
    void doBrew(const char *);
    void doScribe(const char *);
    void doFly();
    void doLand();
    int crashLanding(positionTypeT, bool force = FALSE, bool dam = TRUE);
    int doTurn(const char *, TBeing *);
    virtual void doMedit(const char *);
    virtual void doSEdit(const char *);
    void edit_faction(const char *);
    void show_faction(const char *);
    void add_faction(const char *);
    void doJoin(const char *);
    void doRecruit(const char *);
    void doDefect(const char *);
    bool hasOffer(TFaction *);
    void removeOffers();
    void addOffer(TFaction *);
    bool recentlyDefected();
    void setDefected();


    int doMendLimb(const sstring &);
    void doYoginsa();
    void doMeditate();
    void doPenance();
    int doShove(const char *, TBeing *);
    int doGrapple(const char *, TBeing *);
    int doRescue(const char *);
    void doReset(sstring);
    void doBoot(const sstring &);
    void doResize(const char *);
    int doDeathstroke(const char *, TBeing *);
    void doBandage(const sstring &);
    int doBodyslam(const char *, TBeing *);
    int doSpin(const char *, TBeing *);
    int doStomp(const char *, TBeing *);
    int doHeadbutt(const char *, TBeing *);
    int doKneestrike(const char *, TBeing *);
    int doDoorbash(const sstring &);
    void doTranceOfBlades(const char *);
    void doAttune(const char *);
    void doSharpen(const char *);
    void doCombine(const sstring &);
    void doAdjust(const char *);
    void doDull(const char *);
    void doRepair(const char *);
    void doEat(const char *);
    int doDrink(const char *);
    void foodNDrink(sectorTypeT, int);
    void doSip(const char *);
    int doPour(const char *);
    void doFill(const char *);
    void doTaste(const char *);
    void doBreath(const char *);
    void doQuest(const char *);
    void doMortalQuest(const char *);
    virtual void doStat(const char *);
    void statRoom(TRoom *);
    void statZone(const char *);
    void statObj(const TObj *);
    void statObjForDivman(const TObj *);
    void statBeing(TBeing *);
    virtual void doChange(const char *);
    void lowTasks(const sstring &);
    void lowRace(const char *);
    void lowMobs(const char *);
    void lowObjs(const char *);
    void lowWeaps(const char *);
    void lowPath(const sstring &);
    int doLower(const char *);
    int doRaise(const char *, cmdTypeT);
    virtual void doLow(const sstring &);
    bool isNaked() const;
    bool isWieldingWeapon();
    bool hasPart(wearSlotT) const;
    void doResetMargins();
    void doWeather(const char *);
    void doHeaven(const sstring &);
    void wearNTear(void);
    void doPrompt(const char *);
    virtual void doPurge(const char *);
    virtual void doSet(const char *);
    int backstabHit(TBeing *, TThing *);
    int throatSlitHit(TBeing *, TThing *);
    int critFailureChance(TBeing *, TThing *, spellNumT);
    void critHitEqDamage(TBeing *, TThing *, int);
    int critSuccessChance(TBeing *, TThing *, wearSlotT *, spellNumT, int *, int=-1);
    int critPierce(TBeing *, TThing *, wearSlotT *, spellNumT, int *, int);
    int critSlash(TBeing *, TThing *, wearSlotT *, spellNumT, int *, int);
    int critBlunt(TBeing *, TThing *, wearSlotT *, spellNumT, int *, int);
    int numValidSlots();
    int checkShield(TBeing *, TThing *, wearSlotT, spellNumT, int);
    int getWeaponDam(const TBeing *, const TThing *, primaryTypeT) const;
    virtual float getStrDamModifier() const;
    virtual float getDexDamModifier() const;
    int getDexReaction() const;
    int getAgiReaction() const;
    int getConShock() const;
    float getConHpModifier() const;
    float getDexMod() const;
    float getSpeMod() const;
    float getAgiMod() const;
    float getBraMod() const;
    float getFocMod() const;
    float getIntModForPracs() const;
    float getChaShopPenalty() const;
    float getSwindleBonus();
    void combatFatigue(TThing *);
    int weaponCheck(TBeing *v, TThing *o, spellNumT type, int dam);
    virtual void reconcileHelp(TBeing *, double) { return; }
    virtual void reconcileHurt(TBeing *, double) { return; }
    int oneHit(TBeing *, primaryTypeT, TThing *, int, float *);
    bool isHitableAggr(TBeing *);
    void normalHitMessage(TBeing *, TThing *, spellNumT, int, wearSlotT);
    int monkDodge(TBeing *, TThing *, int *, int, wearSlotT);
    int thiefDodge(TBeing *, TThing *, int *, int, wearSlotT);
    int parryWarrior(TBeing *, TThing *, int *, int, wearSlotT);
    int damageItem(TBeing *, wearSlotT, spellNumT, TThing *, int, const char * = NULL);
    int damageWeapon(TBeing *, wearSlotT, TThing **);
    void damageArm(bool, int);
    int acForEq() const;
    int acForPosSkin(wearSlotT) const;
    int acForPos(wearSlotT) const;
    bool damCheckDeny(const TBeing *, spellNumT) const;
    bool damDetailsOk(const TBeing *, int, bool) const;
    spellNumT getAttackType(const TThing *) const;
    spellNumT getFormType() const;
    bool isOppositeFaction(const TBeing *) const;
    int setCharFighting(TBeing *, int dam = 0);
    int setVictFighting(TBeing *, int dam = 0);
    void setFighting(TThing *, int, bool);
    int damageTrivia(TBeing *v, TThing *o, int dam, spellNumT type);
    void doDamage(int, spellNumT);
    int tellStatus(int, bool, bool);
    int getActualDamage(TBeing *, TThing *, int, spellNumT);
    int damageEm(int, sstring, spellNumT);
    int skipImmortals(int) const;
    int applyDamage(TBeing *, int, spellNumT);
    int preProcDam(spellNumT, int) const;
    TBeing *findAnAttacker() const;
    int damageEpilog(TBeing *, spellNumT);
    void catchLostLink(TBeing *);
    void throwChar(TBeing *v, dirTypeT dir, bool throwerMove, silentTypeT silent, bool forceStand);
    bool checkPeaceful(const sstring &) const;
    bool checkPeacefulVictim(const sstring &, const TThing *) const;
    int extraDam(const TBeing *, const TBaseWeapon *) const;
    TThing * makeCorpse(spellNumT, TBeing * = NULL, float = 0);
    int die(spellNumT, TBeing * = NULL);
    int slotChance(wearSlotT num) const;
    void makeBodyPart(wearSlotT, TBeing * = NULL);
    void makeDiseasedPart(wearSlotT);
    void makeOtherPart(const char *, const char *, TBeing * = NULL);
    bool checkCut(TBeing *, wearSlotT, spellNumT, TThing *, int);
    bool checkPierced(TBeing *, wearSlotT, spellNumT, TThing *, int);
    bool checkSmashed(TBeing *, wearSlotT, spellNumT, TThing *, int, const char * = NULL);
    int hit(TBeing *, int pulse = -1);
    bool canCounterMove(int);
    int trySpringleap(TBeing *);
    int damageLimb(TBeing *, wearSlotT, TThing *, int *);
    int damageHand(TBeing *, wearSlotT);
    void woundedHand(bool);
    bool bothLegsHurt() const;
    bool eitherLegHurt() const;
    bool eitherHandHurt() const;
    bool bothArmsHurt() const;
    bool bothHandsHurt() const;
    bool eitherArmHurt() const;
    bool canUseEquipment(const TObj *, silentTypeT, wearKeyT=WEAR_KEY_NONE) const;
    bool canUseLimb(wearSlotT) const;
    bool canUseHand(bool) const;
    bool canUseArm(primaryTypeT) const;
    bool canUseLeg(primLegT) const;
    int checkPassWard(dirTypeT) const;
    bool canSwim(dirTypeT);
    int rawKill(spellNumT, TBeing * = NULL, float = 0);
    bool validMove(dirTypeT);
    const sstring movementType(bool) const;
    ubyte getMaxLimbHealth(wearSlotT) const;
    bool removeAllCasinoGames() const;
    bool checkHearts(bool = false) const;
    bool checkCrazyEights(bool = false) const;
    bool checkDrawPoker(bool = false) const;
    bool checkBlackjack(bool = false) const;
    bool checkHiLo(bool = false) const;
    bool checkHoldem(bool = false) const;
    bool checkPoker(bool = false) const;
    bool checkBaccarat(bool = false) const;
    bool checkSlots() const;
    bool checkSlotPlayer() const;
    bool isSwimming() const;
    void doPeelPk(const char *);
    bool cutPeelPkDam() const;
    int peelPkRespawn(TBeing *, spellNumT);
    bool inPkZone() const;
    void removeFromPeelPk(TBeing *);

    virtual void peeOnMe(const TBeing *);
    virtual bool canSee(const TThing *, infraTypeT = INFRA_NO) const;

    // functions for protected member manipulation
    virtual bool isPc() const {
      return ((specials.act & ACT_POLYSELF) != 0);
    }
    void setCurLimbHealth(wearSlotT, ubyte);
    ubyte getCurLimbHealth(wearSlotT) const;
    void addCurLimbHealth(wearSlotT, int);
    TThing * getStuckIn(wearSlotT limb) const;
    void setStuckIn(wearSlotT limb, TThing *item);
    unsigned short int getLimbFlags(wearSlotT limb) const;
    void setLimbFlags(wearSlotT limb, unsigned short int num);
    void addToLimbFlags(wearSlotT limb, unsigned short int num);
    void remLimbFlags(wearSlotT limb, unsigned short int num);
    bool isLimbFlags(wearSlotT limb, int num) const;

    bool isDead() const {
        return (getHit() < -10);
    }
    bool willStun(int dam) const {
        return (getHit() - dam <= 0);
    }
    bool willKill(TBeing *, int dam, spellNumT, bool);
    ubyte getLevel(classIndT i) const;
    void setLevel(classIndT i, ubyte);
    ush_int getClass() const;
    void setClass(ush_int num);

    bool isLinkdead() const;
    double deathExp();
    double deathSkillLoss();
    int raceHasNoBones() { return race->hasNoBones(); }
    int raceHasNaturalClimb() { return race->hasNaturalClimb(); }
    void setRace(race_t r);
    double getMaxExp() const;
    void setMaxExp(double n);
    double getExp() const;
    void setExp(double n);
    void addToExp(double n);
    double getPiety() const;
    void setPiety(double num);
    void addToPiety(double num);
    double pietyLimit() const;
    int getSpellHitroll() const;
    void setSpellHitroll(int h);
    int getHitroll() const;
    void setHitroll(int h);
    int getDamroll() const;
    void setDamroll(int d);
    int getBank() const;
    void setBank(int bank);
    int getMoney() const;
    void setMoney(int money);
    void addToMoney(int money, moneyTypeT type);
    void giveMoney(TBeing *, int money, moneyTypeT);
    sexTypeT getSex() const;
    void setSexUnsafe(int sex);
    void setSex(sexTypeT sex);
    byte getInvisLevel() const;
    void setInvisLevel(byte num);
    double getMult() const;
    void setMult(double mult);
    sh_int getArmor() const;
    void setArmor(sh_int armor);
    sh_int suggestArmor() const;
    int getHit() const;
    void addToHit(int add);
    void setHit(int newhit);
    virtual void setMaxHit(int);
    double getPercHit(void);
    int getMove() const;
    void addToMove(int add);
    void setMove(int move);
    void setMaxMove(int move);
    short int moveLimit() const;
    int getMana() const;
    void setMana(int mana);
    void addToMana(int mana);
    double getPercMana(void);
    void setMaxMana(int mana);
    int getLifeforce() const;
    void setLifeforce(int lifeforce);
    void addToLifeforce(int lifeforce);

    TBeing *fight() const;

    virtual TThing *heldInPrimHand() const {
      return (isRightHanded() ? 
	      equipment[HOLD_RIGHT] : 
	      equipment[HOLD_LEFT]);
    }
    virtual TThing *heldInSecHand() const {
      return (isRightHanded() ? 
	      equipment[HOLD_LEFT] : 
	      equipment[HOLD_RIGHT]);
    }
    bool isAffected(unsigned long bv) const;
    unsigned int rentCredit() const;
    byte getProtection() const;
    void setProtection(byte num);
    void addToProtection(byte num);

    int getHeroNum() const {
      return heroNum;
    }
    void setHeroNum(int num) {
      heroNum = num;
    }
    void addToHero(int num);
    // new faction functions - dash
    TFaction * newfaction() const;
    const char * rank();
    bool canCreateFaction(bool);
    bool hasPermission(unsigned int);

    factionTypeT getFaction() const;
    void setFaction(factionTypeT num);
    double getPerc() const;
    void setPerc(double num);
#if FACTIONS_IN_USE
    void addToPerc(double num);
    double getPercX(factionTypeT num) const;
    void setPercX(double num, factionTypeT fact);
    void addToPercX(double num, factionTypeT fact);
#endif
    unsigned int getFactAct() const;
    void setFactAct(unsigned int r);
    void setFactActBit(int i);
    void removeFactActBit(int i);
    TBeing *getCaptive() const;
    TBeing *getNextCaptive() const;
    TBeing *getCaptiveOf() const;
    void setCaptive(TBeing *ch);
    void setNextCaptive(TBeing *ch);
    void setCaptiveOf(TBeing *ch);
    TBeing *getFactionTarget() const;
    void setFactionTarget(TBeing *ch);
    bool isBrother() const;
    bool isCult() const;
    bool isSnake() const;
    bool isUnaff() const;
    bool isSameFaction(TBeing *ch) const;
    race_t getRace() const;
    Race * getMyRace() const;
    bool isSameRace(const TBeing *ch) const;
    bool isGuildmaster() const;
    bool noMana(int mana) const;
    bool noLifeforce(int lifeforce) const;
#if FACTIONS_IN_USE
    bool percLess(double perc) const;
#endif
    bool awake() const;
    virtual bool isFlying() const;
    virtual bool isLevitating() const;
    bool nomagic(const sstring &, const sstring &msg_rm="") const;
    void dieFollower();
    int doDiscipline(spellNumT, const char *);
    int unloadBow(const char *);
    int legalDeckAddition(int);

    // User commands go here
    void doSysMid();
    void doFindEmail(const char *);
    void doNameChange(const char *);
    void doSysTraceroute(const sstring &);
    void doSysLoglist();
    void doSysTasks(const sstring &);
    void doSysChecklog(const sstring &);
    void doSysViewoutput();
    int doSubterfuge(const char *);
    int doBackstab(const char *, TBeing *);
    int doThroatSlit(const char *, TBeing *);
    void doGuard(const sstring &);
    void doSplit(const char *, bool);
    void doReply(const sstring &);
    void doReport(const char *);
    virtual void doTitle(const char *);
    int doTithe();
    void doMessage(const char *);
    int getTrainerPracs(const TBeing *, const TMonster *, classIndT, discNumT, int) const;
    int checkTrainDeny(const TBeing *, TMonster *, discNumT, int) const;
    int checkForPreReqs(const TBeing *, TMonster *, discNumT, classIndT, int, int) const;
    int initiateSkillsLearning(discNumT, int, int);
    void setSpellEligibleToggle(TMonster *, spellNumT, silentTypeT);
    int doTraining(TBeing *ch, TMonster *, classIndT, int, int) const;
    int getCombatPrereqNumber(classIndT) const;

    // this and ch can't be const due to setting doneBasic
    int checkDoneBasic(TBeing *, classIndT, int, int);

    void doPracSkill(const char *, spellNumT);
    void doPracDisc(const char *, int);
    void doSpells(const sstring &);
    void doRituals(const sstring &);
    void doPrayers(const sstring &);
    void sendSkillsList(discNumT);
    void doPractice(const char *);
    virtual void doIdea(const sstring &);
    virtual void doBug(const sstring &);
    virtual void doTypo(const sstring &);
    virtual void doColor(const char *);
    void setColor(setColorFieldT, setColorKolorT);
    int doQuaff(sstring);
    void reformGroup();
    void doGroup(const char *);
    int doRecite(const char *);
    int doUse(sstring);
    int doUnsaddle(sstring);
    int doSaddle(sstring);
    int doChop(const char *, TBeing *);
    int doHurl(const char *, TBeing *);
    int doChi(const char *, TThing *);
    int doLeap(const sstring &);
    int doVote(const sstring &);
    int doLayHands(const char *);
    int doSmoke(const char *);
    void doExtinguish(const sstring &);
    void doLight(const sstring &);
    void doRefuel(const sstring &);
    void doStop(const sstring &);
    void doContinue(const char *);
    void doFish(sstring);
    void doHistory();
    int  doCreate(const char *);
    int  ChargePulse(TBeing *ch);
    int getPosHeight() const;
    int getPartMinHeight(int) const;
    void doDrag(TBeing *, dirTypeT);
    void doDrag(TObj *, dirTypeT);
    void doDrag(const sstring &);
    void doCommand(const char *);
    int doAssist(const char *, TBeing *, bool flags = FALSE);
    void doRoll(TBeing *, dirTypeT);
    void doRoll(TObj *, dirTypeT);
    void doRoll(const sstring &);
    void doEcho(const char *);
    void doHighfive(const sstring &);
    virtual void doToggle(const char *);
    virtual void doTrans(const char *);
    virtual void doSwitch(const char *);
    virtual void doCutlink(const char *);
    virtual void doInvis(const char *);
    virtual void doVisible(const char *, bool);
    virtual void doClone(const sstring &);
    virtual void doAccess(const sstring &);
    virtual void doOffice(sstring);
    void doWizlock(const char *);
    void doFlag(const char *);
    void doSystem(const sstring &);
    void doWiznews();
    void doNoshout(const sstring &);
    int doHide();
    int doSneak(const char *);
    void doLoglist(const char *, int);
    int doExits(const char *, cmdTypeT);
    void doWipe(const char *);
    void doReplace(const sstring &);
    void doSetsev(const char *);
    void doTimeshift(const char *);
    void doInfo(const char *);
    void doLog(const char *);
    void doHostlog(const char *);
    virtual void doShow(const char *);
    void doChecklog(const char *, int);
    void doDeathcheck(const sstring &);
    int doGive(TBeing *, TThing *, giveTypeT = GIVE_FLAG_DEF);
    int doGive(const sstring &, giveTypeT = GIVE_FLAG_DEF);
    int doMount(const char *, cmdTypeT, TBeing *, silentTypeT silent = SILENT_NO);
    int doJunk(const char *, TObj *);
    int doNoJunk(const char *, TObj *);
    int doDonate(const char *);
    int doSteal(const sstring &, TBeing *);
    void doRestore(const char *);
    void doThrow(const sstring &);
    void doWear(const char *);
    int stickIn(TThing *o, wearSlotT pos, silentTypeT silent = SILENT_NO);
    int doEmote(const sstring &);
    void doNotHere() const;
    void doWield(const char *);
    // psionics
    int doPTell(const char *, bool);
    int doPSay(const char *);
    void doPShout(const char *);
    void doTelevision(const char *);
    void doMindfocus(const char *);
    int doPsiblast(const char *);
    int doMindthrust(const char *);
    int doPsycrush(const char *);
    int doKwave(const char *);
    int doPsidrain(const char *);


    // Monk Skills
    int doQuiveringPalm(const char *, TBeing *);
    int doSpringleap(sstring, bool, TBeing *);
    int doShoulderThrow(const char *, TBeing *);
    int doGrappleMonk(const char *, int, TBeing *);
    int doFeignDeath();
    int doMendLimb(const char *, int);
    int aiHurl(dirTypeT, TBeing *);
#if 0
    int doFizal(const char *, int);
    int doDoubleKick(const char *, int, TBeing *);
    int doTripleKick(const char *, int, TBeing *);
#endif

    int doDrop(const sstring &, TThing *, bool forceDrop = FALSE);
    int doGet(const char *);
    void doGrab(const char *);
    int doPut(const char *);
    int doKick(const char *, TBeing *);
    bool isSaveMob(const TBeing *) const;
    bool isPet(const unsigned int) const;
    bool isElemental() const;
    int doBash(const char *, TBeing *);
    int doTrip(const char *, TBeing *);
    virtual void doSnoop(const char *);
    virtual int doAt(const char *, bool);
    void doPeek() const;
    int doAction(const sstring &, cmdTypeT);
    void doCls(bool);
    int doRemove(const sstring &, TThing *);
    void doSave(silentTypeT, const char * = NULL);
    void doZonefile(const sstring &);
    void doLoot(const sstring &);
    void saveChar(short int);
    int doFlee(const char *);
    int doDisarm(sstring, TThing *);
    int dieReturn(const char *, spellNumT, int);
    void doReturn(const char *, wearSlotT, bool, bool deleteMob = TRUE);
    int doHit(const sstring &, TBeing *);
    int doEngagedHit(const char *, TBeing *);
    int doKill(const char *, TBeing *);
    int doEngage(const char *, TBeing *);
    int doDisengage();
    int doOrder(const char *);
    bool isOrderAllowed(const char *);
    virtual int doQuit2() = 0;
    void doQuit();
    void doBruttest(const char *);
    int doMove(cmdTypeT);
    int doMove(dirTypeT);
    int doSay(const char *fmt, ...);
    int doSay(const sstring &);
    virtual void doForce(const char *);
    void doCommune(const sstring &);
    void mudMessage(TBeing *, int, const char *); 
    virtual void doShutdow();
    virtual void doShutdown(const char *);
    const sstring addColorRoom(TRoom *, int) const;
    sstring autoFormatDesc(const sstring &, bool) const;
    sstring dynColorRoom(TRoom *, int, bool) const;
    sstring garble(const sstring &, int) const;
    sstring PG13filter(const sstring &)const;
    sstring blahblah(const sstring &) const;
    void doLook(const sstring &, cmdTypeT, TThing *specific = NULL);
    void doShout(const char *);
    int doWhisper(const sstring &);
    int doTell(const sstring &, const sstring &, bool visible = TRUE);
    int doClientMessage(const char *);
    int doAsk(const sstring &);
    void doSign(const sstring &);
    void doGrouptell(const sstring &);
    void doWrite(const char *);
    void doExamine(const char *, TThing *specific = NULL);
    void doRead(const char *);
    void doWake(const char *);
    int doOpen(const char *);
    int doLeave(const char *);
    int doEnter(const char *, TPortal *);
    void doSit(const sstring &);
    void doRest(const sstring &);
    void doSleep(const sstring &);
    void doClose(const char *);
    void doUnlock(const char *);
    void doLock(const char *);
    void doScore();
    void doTrophy(const sstring &);
    void doWhozone();
    void doWho(const char *);
    void doTime(const char *);
    void doWizhelp();
    virtual void raiseLevel(classIndT);
    virtual void doUsers(const sstring &);
    void doInventory(const char *);
    void doEquipment(const sstring &);
    void doOutfit(const sstring &);
    void doCredits();
    void doNews(const char *);
    void doWizlist();
    void doWhere(const char *);
    void doLevels(const char *);
    void doConsider(const char *);
    void immortalEvaluation(const TMonster *) const;
    void doSpelllist(const char *, int);
    void doWorld();
    void doAttribute(const char *);
    void doClear(const char *);
    void doAlias(const char *);
    void doGlance(const char *);
    void doMotd(const char *);
    void doLimbs(const sstring &);
    void describeLimbDamage(const TBeing *) const;
    sstring describeAffects(TBeing *, showMeT) const;
    void doStand();
    void doCrawl();
    void doFollow(const char *);
    void doTestCode(const char *);
    int doGoto(const sstring &);
    int doMortalGoto(const sstring &);
    void doNewbieEqLoad(race_t, ush_int, bool);
    virtual void doLoad(const char *);
    int doDisguise(const char *);
    int doPoisonWeapon(sstring);
    int doGarrotte(const char *, TBeing *);
    int doStab(const char *, TBeing *);
    int doCudgel(const char *, TBeing *);
    virtual int moneyMeBeing(TThing *mon, TThing *sub);
    virtual unsigned int getTimer() const = 0;
    virtual void setTimer(unsigned int) = 0;
    virtual void addToTimer(unsigned int) = 0;
    bool isStrong() const;
    bool isPerceptive() const;
    bool isAgile(int num) const;
    bool isTough() const;
    bool isUgly() const;
    bool isRealUgly() const;
    bool isWary() const;
    void makeWary();
    sstring displayExp() const;
    int hurtLimb(unsigned int, wearSlotT);
    int flightCheck();
    int hpGainForLevel(classIndT) const;
    int hpGainForClass(classIndT) const;
    unsigned int numberInGroupInRoom() const;
    double getExpShare() const;
    double getExpSharePerc() const;
    int genericRestore(restoreTypeT);
    void makeOutputPaged();
    TThing * findArrow(const char *, silentTypeT) const;
    
    int get_metal_tools(TTool **, TTool **, TTool **, TTool **);
    int get_dead_tools(TTool **, TTool **, TTool **);
    int get_wood_tools(TTool **, TTool **);
    int get_shell_tools(TTool **, TTool **);
    int get_magic_tools(TTool **, TTool **, TTool **);
    int get_rock_tools(TTool **, TTool **, TTool **);
    int get_gemmed_tools(TTool **, TTool **, TTool **);
    int get_leather_tools(TTool **, TTool **);
    int get_paper_tools(TTool **);
    int get_melt_tools(TTool **);
    int get_weave_tools(TTool **);
    int get_sew_tools(TTool **, TTool **);
    int get_ceramic_tools(TTool **);
    int get_spirit_tools(TTool **, TTool **, TTool **);
};

#endif
