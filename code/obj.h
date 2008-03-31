//////////////////////////////////////////////////////////////////////////
//
// SneezyMUD - All rights reserved, SneezyMUD Coding Team
//
//////////////////////////////////////////////////////////////////////////


#ifndef __OBJ_H
#define __OBJ_H

#ifndef __SPEC_OBJS_H
#include "spec_objs.h"
#endif

class objCost { 
  public:
    int total_cost;
    int no_carried;
    int lowrentobjs;
    bool ok;
    objCost();
    ~objCost();
};



extern vector<objIndexData>obj_index;

typedef list<TObj *> TObjList;
typedef list<TObj *>::iterator TObjIter;

extern TObjList object_list;

extern int commod_index[200];

extern TObj *read_object_buy_build(TBeing *, int nr, readFileTypeT type);
extern TObj *read_object(int nr, readFileTypeT type);
extern void log_object(TObj *obj);

extern liqInfoT liquidInfo;


// weight of 1 sip/unit of drink
// 128 fl.oz = 1 gallon = 8.337 lb. (for water)
// 1 sip = 1 fl.oz = 0.0651328
// 1 gallon = 231 cubic inches
// 231/128 = 1.8046875 ci/sip
const float SIP_WEIGHT         = 0.065;
const float SIP_VOLUME         = 1.8046875;

const int MAX_SWING_AFFECT  = 5;
//const int MAX_OBJ_AFFECT    = 5;     
const int OBJ_NOTIMER    = -1;
const int MAX_TOUNGE  = 3;
const int MAX_AFFECT  = 25;

const int TYPE_ONE_SWING   = -122;

enum itemTypeT {
    ITEM_UNDEFINED,
    ITEM_LIGHT,
    ITEM_SCROLL,
    ITEM_WAND,
    ITEM_STAFF,
    ITEM_WEAPON,
    ITEM_FUEL,
    ITEM_OPAL,
    ITEM_TREASURE,
    ITEM_ARMOR,
    ITEM_POTION,
    ITEM_WORN,
    ITEM_OTHER,
    ITEM_TRASH,
    ITEM_TRAP,
    ITEM_CHEST,
    ITEM_NOTE,
    ITEM_DRINKCON,
    ITEM_KEY,
    ITEM_FOOD,
    ITEM_MONEY,
    ITEM_PEN,
    ITEM_BOAT,
    ITEM_AUDIO,
    ITEM_BOARD,
    ITEM_BOW,
    ITEM_ARROW,
    ITEM_BAG,
    ITEM_CORPSE,
    ITEM_SPELLBAG,
    ITEM_COMPONENT,
    ITEM_BOOK,
    ITEM_PORTAL,
    ITEM_WINDOW,
    ITEM_TREE,
    ITEM_TOOL,
    ITEM_HOLY_SYM,
    ITEM_QUIVER,
    ITEM_BANDAGE,
    ITEM_STATUE,
    ITEM_BED,
    ITEM_TABLE,
    ITEM_RAW_MATERIAL,
    ITEM_GEMSTONE,
    ITEM_MARTIAL_WEAPON,
    ITEM_JEWELRY,
    ITEM_VIAL,
    ITEM_PCORPSE,
    ITEM_POOL,
    ITEM_KEYRING,
    ITEM_RAW_ORGANIC,
    ITEM_FLAME,
    ITEM_APPLIED_SUB,
    ITEM_SMOKE,
    ITEM_ARMOR_WAND,
    ITEM_DRUG_CONTAINER,
    ITEM_DRUG,
    ITEM_GUN,
    ITEM_AMMO,
    ITEM_PLANT,
    ITEM_COOKWARE,
    ITEM_VEHICLE,
    ITEM_CASINO_CHIP,
    ITEM_POISON,
    ITEM_HANDGONNE,
    ITEM_EGG,
    ITEM_CANNON,
    ITEM_TOOTH_NECKLACE,
    ITEM_TRASH_PILE,
    ITEM_CARD_DECK,
    ITEM_SUITCASE,
    ITEM_SADDLE,
    ITEM_HARNESS,
    ITEM_SADDLEBAG,
    ITEM_WAGON,
    MAX_OBJ_TYPES
};
const itemTypeT MIN_OBJ_TYPE = itemTypeT(0);
extern itemTypeT & operator++(itemTypeT &, int);

extern TObj *makeNewObj(itemTypeT);

const int MAX_PORTAL_TYPE = 14;

class itemInfo
{
  public:   
    const char * const name;
    const char * const common_name;
    const char * const val0_info;
    int  val0_max;
    int  val0_min;
    const char * const val1_info;
    int  val1_max;
    int  val1_min;
    const char * const val2_info;
    int  val2_max;
    int  val2_min;
    const char * const val3_info;
    int  val3_max;
    int  val3_min;

    itemInfo(const char * const n, const char * const cn, const char * const v0, int v0x, int v0n, const char * const v1, int v1x, int v1n, const char * const v2, int v2x, int v2n, const char * const v3, int v3x, int v3n);
    ~itemInfo();
  private:
    //    itemInfo() {} // prevent use
};

extern itemInfo *ItemInfo[MAX_OBJ_TYPES];

enum toolTypeT {
     TOOL_WHETSTONE,
     TOOL_ANVIL,
     TOOL_FORGE,
     TOOL_HAMMER,  //3
     TOOL_LOCKPICK,
     TOOL_NEEDLE,  // 5
     TOOL_THREAD,
     TOOL_POISON,
     TOOL_GARROTTE,
     TOOL_FILE,   
     TOOL_BOWSTRING,//10
     TOOL_SKIN_KNIFE,
     TOOL_HOLYWATER,
     TOOL_FLINTSTEEL,
     TOOL_TOTEM,  
     TOOL_FISHINGBAIT, //15
     TOOL_BUTCHER_KNIFE,
     TOOL_SEED,
     TOOL_TONGS,
     TOOL_OPERATING_TABLE, 
     TOOL_SCALPEL, // 20
     TOOL_FORCEPS,
     TOOL_LADEL,
     TOOL_SOIL,
     TOOL_PLANT_OIL,
     TOOL_CHALK,  // 25
     TOOL_RUNES,
     TOOL_ENERGY,
     TOOL_PENTAGRAM,
     TOOL_CHISEL,
     TOOL_SILICA, // 30
     TOOL_WORKBENCH,
     TOOL_LOUPE,
     TOOL_PLIERS,
     TOOL_PUNCH,
     TOOL_CORDING, // 35
     TOOL_TAPE,
     TOOL_CANDLE,
     TOOL_GLUE,
     TOOL_ALTAR,
     TOOL_BRUSH,  // 40
     TOOL_ASTRAL_RESIN,
     TOOL_BLACK_POWDER,
     MAX_TOOL_TYPE // move and change
};
const toolTypeT MIN_TOOL_TYPE = TOOL_WHETSTONE;

enum organicTypeT {
    ORGANIC_NONE,
    ORGANIC_HIDE,
    ORGANIC_HIDE_REFINED,
    ORGANIC_WOOD,
    ORGANIC_BONE,
    ORGANIC_CORAL,
    ORGANIC_SILK,
    ORGANIC_WOOL,
    ORGANIC_HERBAL
};

/* Bitvector For 'wear_flags' */

const unsigned int ITEM_TAKE              = (1<<0);    // 1
const unsigned int ITEM_WEAR_FINGERS       = (1<<1);    // 2
const unsigned int ITEM_WEAR_NECK         = (1<<2);    // 4
const unsigned int ITEM_WEAR_BODY         = (1<<3);    // 8
const unsigned int ITEM_WEAR_HEAD         = (1<<4);    // 16
const unsigned int ITEM_WEAR_LEGS         = (1<<5);    // 32
const unsigned int ITEM_WEAR_FEET         = (1<<6);    // 64
const unsigned int ITEM_WEAR_HANDS        = (1<<7);    // 128
const unsigned int ITEM_WEAR_ARMS         = (1<<8);    // 256
const unsigned int ITEM_WEAR_UNUSED1      = (1<<9);    // 512
const unsigned int ITEM_WEAR_BACK         = (1<<10);   // 1024
const unsigned int ITEM_WEAR_WAIST       = (1<<11);   // 2048
const unsigned int ITEM_WEAR_WRISTS        = (1<<12);   // 4096
const unsigned int ITEM_WEAR_UNUSED2      = (1<<13);   // 8192
const unsigned int ITEM_HOLD              = (1<<14);   // 16384
const unsigned int ITEM_THROW             = (1<<15);   // 32768

const unsigned int MAX_ITEM_WEARS                  = 16;

/* Some different kind of liquids */
enum liqTypeT {
     LIQ_NONE=-1,
     LIQ_WATER, // 0
     LIQ_BEER,
     LIQ_WINE,
     LIQ_ALE,
     LIQ_DARKALE,
     LIQ_WHISKY, // 5
     LIQ_LEMONADE,
     LIQ_FIREBRT,
     LIQ_LOCALSPC,
     LIQ_SLIME,
     LIQ_MILK, // 10
     LIQ_TEA,
     LIQ_COFFEE,
     LIQ_BLOOD,
     LIQ_SALTWATER,
     LIQ_MEAD, // 15
     LIQ_VODKA,
     LIQ_RUM,
     LIQ_BRANDY,
     LIQ_RED_WINE,
     LIQ_WARM_MEAD, // 20
     LIQ_CHAMPAGNE,
     LIQ_HOLYWATER,
     LIQ_PORT,
     LIQ_MUSHROOM_ALE,
     LIQ_VOMIT, // 25
     LIQ_COLA,
     LIQ_STRAWBERRY_MARGARITA,
     LIQ_BLUE_MARGARITA,
     LIQ_GOLD_MARGARITA,
     LIQ_STRAWBERRY_DAIQUIRI, // 30
     LIQ_BANANA_DAIQUIRI,
     LIQ_PINA_COLADA,
     LIQ_TEQUILA_SUNRISE,
     LIQ_ISLA_VERDE,
     LIQ_POT_CURE_POISON, // 35
     LIQ_POT_HEAL_LIGHT,
     LIQ_POT_HEAL_CRIT,
     LIQ_POT_HEAL,
     LIQ_POT_SANCTUARY,
     LIQ_POT_FLIGHT, // 40
     LIQ_POT_BIND,
     LIQ_POT_BLINDNESS,
     LIQ_POT_ARMOR,
     LIQ_POT_REFRESH,
     LIQ_POT_SECOND_WIND, // 45
     LIQ_POT_CURSE,
     LIQ_POT_DETECT_INVIS,
     LIQ_POT_BLESS,
     LIQ_POT_INVIS,
     LIQ_POT_HEAL_FULL, // 50
     LIQ_POT_SUFFOCATE,
     LIQ_POT_FEATHERY_DESCENT,
     LIQ_POT_DETECT_MAGIC,
     LIQ_POT_DISPEL_MAGIC,
     LIQ_POT_STONE_SKIN, // 55
     LIQ_POT_TRAIL_SEEK,
     LIQ_POT_FAERIE_FIRE,
     LIQ_POT_FLAMING_FLESH,
     LIQ_POT_CONJURE_ELE_EARTH,
     LIQ_POT_SENSE_LIFE, // 60
     LIQ_POT_STEALTH,
     LIQ_POT_TRUE_SIGHT,
     LIQ_POT_ACCELERATE,
     LIQ_POT_INFRAVISION,
     LIQ_POT_SORC_GLOBE, // 65
     LIQ_POT_POISON,
     LIQ_POT_BONE_BREAKER,
     LIQ_POT_AQUALUNG,
     LIQ_POT_HASTE,
     LIQ_POT_TELEPORT, // 70
     LIQ_POT_GILLS_OF_FLESH,
     LIQ_POT_CURE_BLINDNESS,
     LIQ_POT_CURE_DISEASE,
     LIQ_POT_SHIELD_OF_MISTS,
     LIQ_POT_SENSE_PRESENCE, // 75
     LIQ_POT_CHEVAL,
     LIQ_POT_DJALLAS_PROTECTION,
     LIQ_POT_LEGBAS_GUIDANCE,
     LIQ_POT_DETECT_SHADOW,
     LIQ_POT_CELERITE, // 80
     LIQ_POT_CLARITY,
     LIQ_POT_BOILING_BLOOD,
     LIQ_POT_STUPIDITY,
     LIQ_POT_SLUMBER,
     LIQ_POT_HEAL2, // 85
     LIQ_POT_FEATHERY_DESCENT2,
     LIQ_POT_SANCTUARY2,
     LIQ_POT_STONE_SKIN2,
     LIQ_POT_INFRAVISION2,
     LIQ_POT_HEAL_LIGHT2, // 90
     LIQ_POT_GILLS_OF_FLESH2,
     LIQ_POT_CELERITE2,
     LIQ_POT_CELERITE3,
     LIQ_POT_TELEPORT2,
     LIQ_POT_BLESS2, // 95
     LIQ_POT_SECOND_WIND2,
     LIQ_POT_MULTI1,
     LIQ_POT_MULTI2,
     LIQ_POT_MULTI3,
     LIQ_POT_MULTI4, // 100
     LIQ_POT_MULTI5,
     LIQ_POT_MULTI6,
     LIQ_POT_MULTI7,
     LIQ_POT_MULTI8,
     LIQ_POT_MULTI9, // 105
     LIQ_POT_MULTI10,
     LIQ_POT_MULTI11,
     LIQ_POT_YOUTH,
     LIQ_POT_STAT,
     LIQ_POT_LEARNING, // 110
     LIQ_POISON_STANDARD,
     LIQ_POISON_CAMAS,
     LIQ_POISON_ANGEL,
     LIQ_POISON_JIMSON,
     LIQ_POISON_HEMLOCK, // 115
     LIQ_POISON_MONKSHOOD,
     LIQ_POISON_GLOW_FISH,
     LIQ_POISON_SCORPION,
     LIQ_POISON_VIOLET_FUNGUS,
     LIQ_POISON_DEVIL_ICE, // 120
     LIQ_POISON_FIREDRAKE,
     LIQ_POISON_INFANT,
     LIQ_POISON_PEA_SEED,
     LIQ_POISON_ACACIA,
     LIQ_LUBRICATION, // 125
     LIQ_MAGICAL_ELIXIR,
     LIQ_URINE,
     LIQ_POT_HEALING_GRASP,
     LIQ_POT_CLEANSE, 
     LIQ_POT_QUICKSILVER, // 130
     LIQ_POT_MYSTERY,
     LIQ_MUD,
     LIQ_TEQUILA,
     LIQ_POT_ENLIVEN,
     LIQ_POT_PLASMA_MIRROR, // 135
     LIQ_POT_FILTH,
     LIQ_GUANO,
     MAX_DRINK_TYPES     // move and change
};
extern liqTypeT & operator++(liqTypeT &, int);
const liqTypeT MIN_DRINK_TYPES = liqTypeT(0);

/* special addition for drinks */
const unsigned int DRINK_POISON  = (1<<0);
const unsigned int DRINK_PERM    = (1<<1);
const unsigned int DRINK_SPILL   = (1<<2);
const unsigned int DRINK_FROZEN  = (1<<3);

/* special addition for drinks */
const unsigned int FOOD_POISON   = (1<<0);
const unsigned int FOOD_SPOILED  = (1<<1);
const unsigned int FOOD_FISHED   = (1<<2);
const unsigned int FOOD_BUTCHERED= (1<<3);

/* special addition for corpses */
const unsigned int CORPSE_NO_REGEN       = (1<<0);
const unsigned int CORPSE_NO_DISSECT     = (1<<1);
const unsigned int CORPSE_NO_SKIN        = (1<<2);
const unsigned int CORPSE_HALF_SKIN      = (1<<3);
const unsigned int CORPSE_PC_SKINNING    = (1<<4);
const unsigned int CORPSE_SACRIFICE      = (1<<5);
const unsigned int CORPSE_NO_BUTCHER     = (1<<6);
const unsigned int CORPSE_HALF_BUTCHERED = (1<<7);
const unsigned int CORPSE_PC_BUTCHERING  = (1<<8);
const unsigned int CORPSE_DENY_LOOT      = (1<<9);

const int MAX_CORPSE_FLAGS     = 10;  // move and change

/* for containers  - value[1] */

const unsigned int CONT_CLOSEABLE      = (1 << 0);
const unsigned int CONT_PICKPROOF      = (1 << 1);
const unsigned int CONT_CLOSED         = (1 << 2);
const unsigned int CONT_LOCKED         = (1 << 3);
const unsigned int CONT_TRAPPED        = (1 << 4);
const unsigned int CONT_SECRET         = (1 << 5); // Contianer cannot be seen(window)
const unsigned int CONT_EMPTYTRAP      = (1 << 6); // Can not have a fake trap.
const unsigned int CONT_GHOSTTRAP      = (1 << 7); // Thief *THOUGHT* they saw a trap.
const unsigned int CONT_WEIGHTLESS     = (1 << 8);
const unsigned int MAX_CONTAINER_FLAG = 9;  // move and change

const unsigned int BOW_STRING_BROKE    = (1 << 0);
const unsigned int BOW_CARVED          = (1 << 1);
const unsigned int BOW_SCRAPED         = (1 << 2);
const unsigned int BOW_SMOOTHED        = (1 << 3);

const unsigned int ARROW_FEATHERED     = (1 << 0);
const unsigned int ARROW_CARVED        = (1 << 1);
const unsigned int ARROW_SCRAPED       = (1 << 2);
const unsigned int ARROW_SMOOTHED      = (1 << 3);


// NOTE:::
// I cleared way for 4 new extra flags to be used. The last flag
// I added (nolocate) in my opinion was done improperly however
// it works so should remain unchanged. Please remove the last part
// of the comment on the line you use for your extra flag because
// I use them as a marker for quick search...Thank you
// -Jesus 10-19-2000

const unsigned int ITEM_GLOW            = (1<<0);     // 1
const unsigned int ITEM_HUM             = (1<<1);     // 2
const unsigned int ITEM_STRUNG          = (1<<2);     // 4
const unsigned int ITEM_SHADOWY         = (1<<3);     // 8
const unsigned int ITEM_PROTOTYPE       = (1<<4);     // 16
const unsigned int ITEM_INVISIBLE       = (1<<5);     // 32
const unsigned int ITEM_MAGIC           = (1<<6);     // 64
const unsigned int ITEM_NODROP          = (1<<7);     // 128
const unsigned int ITEM_BLESS           = (1<<8);     // 256
const unsigned int ITEM_SPIKED          = (1<<9);     // 512 USE ME FIRST
const unsigned int ITEM_HOVER           = (1<<10);    // 1024
const unsigned int ITEM_RUSTY           = (1<<11);    // 2048
const unsigned int ITEM_ANTI_CLERIC     = (1<<12);    // 4096
const unsigned int ITEM_ANTI_MAGE       = (1<<13);    // 8192
const unsigned int ITEM_ANTI_THIEF      = (1<<14);    // 16384
const unsigned int ITEM_ANTI_WARRIOR    = (1<<15);    // 32768
const unsigned int ITEM_ANTI_SHAMAN     = (1<<16);    // 65536
const unsigned int ITEM_ANTI_DEIKHAN    = (1<<17);    // 131072
const unsigned int ITEM_ANTI_RANGER     = (1<<18);    // 262144
const unsigned int ITEM_ANTI_MONK       = (1<<19);    // 524288
const unsigned int ITEM_PAIRED          = (1<<20);    // 1048576
const unsigned int ITEM_NORENT          = (1<<21);    // 2097152
const unsigned int ITEM_FLOAT           = (1<<22);    // 4194304
const unsigned int ITEM_NOPURGE         = (1<<23);    // 8388608
const unsigned int ITEM_NEWBIE          = (1<<24);    // 16777216
const unsigned int ITEM_NOJUNK_PLAYER   = (1<<25);    // 33554432 USE THIS SPOT FOR EXTRA
const unsigned int ITEM_NOT_USED2       = (1<<26);    // 67108864 USE THIS SPOT FOR EXTRA
const unsigned int ITEM_NOT_USED3       = (1<<27);    // 134217728  USE THIS SPOT FOR EXTRA
const unsigned int ITEM_ATTACHED        = (1<<28);    // 268435456 
const unsigned int ITEM_BURNING         = (1<<29);    // 536870912
const unsigned int ITEM_CHARRED         = (1<<30);    // 1073741824
const unsigned int ITEM_NOLOCATE        = (unsigned) (1<<31);    // returns negitive int

const int MAX_OBJ_STAT        = 32;        // move and change

extern long objCount;

class objFlagData {
    friend class TObj;
  private:
    unsigned int extra_flags;    /* If it hums,glows etc             */
    byte depreciation;
  public:
    unsigned int wear_flags;     /* Where you can wear it            */
    int cost;           /* Value when sold (gp.)            */
    long bitvector;     /* To set chars bits                */
    sh_int decay_time;
    sh_int struct_points;
    sh_int max_struct_points;
    int volume;

  objFlagData();
  objFlagData(const objFlagData &a);
  objFlagData & operator=(const objFlagData &a);
  ~objFlagData();
};

class affectedData;

class TObj : public TThing {
  public:
    objFlagData obj_flags;
    objAffData affected[MAX_OBJ_AFFECT];

    const char *action_description;      /* What to write when used          */
    const char * owners;
private:
    bool isTasked;
    bool isLocked; // set if the object should be protected from damage temporarily

  protected:
    TObj();
  public:
    TObj(const TObj &);
    TObj & operator=(const TObj &);
    virtual TThing& operator+= (TThing& t);
    virtual ~TObj();

    // VIRTUAL FUNCTIONS
    virtual sstring showModifier(showModeT, const TBeing *) const { return ""; }
    virtual bool isPersonalized() { return action_description != NULL; }
    virtual int getVolume() const { return (obj_flags.volume); }
    virtual const char *getName() const { return shortDescr; }
    virtual int getSnum() const { return (snum > -1 ? snum : objVnum()); };

    virtual roomDirData *exitDir(dirTypeT door) const;
    virtual bool shouldntBeShown(wearSlotT) const;
    virtual int putSomethingIntoContainer(TBeing *, TOpenContainer *);
    virtual bool isShopSimilar(const TThing *) const;
    virtual bool isSimilar(const TThing *) const;
    virtual bool isLevitating() const;
    virtual bool fitInShop(const char *, const TBeing *) const;
    virtual bool canDrop() const;
    virtual int checkSpec(TBeing *, cmdTypeT, const char *, TThing *);
    virtual itemTypeT itemType() const = 0;
    // END VIRTUAL FUNCTIONS

    bool isMonogrammed() const;
    bool isImmMonogrammed() const;
    bool deMonogram(bool erase_imm_monogram);
    
    int objectTickUpdate(int);
    int updateBurning(void);
    bool isObjStat(unsigned int num) const;
    unsigned int getObjStat() const;
    void swapToStrung();
    void setObjStat(unsigned int num);
    void remObjStat(unsigned int num);
    void addObjStat(unsigned int num);
    bool isPaired() const;
    bool usedAsPaired() const;
    int objVnum() const;
    int adjPrice() const;

    virtual void setVolume(int vol) {
      obj_flags.volume = vol;
    }
    virtual void addToVolume(int vol) {
      obj_flags.volume += vol;
    }
    bool canWear(int part) const {
      return (obj_flags.wear_flags & part);
    }

    bool isUnique() const;
    sh_int getStructPoints() const {
      return obj_flags.struct_points;
    }
    void setStructPoints(sh_int num) {
      obj_flags.struct_points = num;
    }
    void addToStructPoints(sh_int num) {
      obj_flags.struct_points += num;
    }
    sh_int getMaxStructPoints() const {
      return obj_flags.max_struct_points;
    }
    void setMaxStructPoints(sh_int num) {
      obj_flags.max_struct_points = num;
    }
    void addToMaxStructPoints(sh_int num) {
      obj_flags.max_struct_points += num;
    }
    byte getDepreciation() const {
      return 0;
      //      return obj_flags.depreciation;
    }
    void setDepreciation(byte num) {
      obj_flags.depreciation = num;
    }
    void addToDepreciation(byte num) {
      obj_flags.depreciation += num;
    }

    // if an object is being used by a task currently
    bool isTaskObj() const {
      return isTasked;
    }
    void setIsTaskObj(bool t) {
      isTasked = t;
    }

    // locks an object from taking damage temporarily
    bool getLocked() const {
      return isLocked;
    }
    void setLocked(bool l) {
      isLocked = l;
    }

    void releaseObject(TBeing *);
    virtual int checkFalling();
    void checkObjStats();
    virtual void update(int);
    virtual bool isBluntWeapon() const;
    bool canRust();
    bool willDent(int);
    bool willTear(int);
    bool willPuncture(int);
    virtual int riverFlow(int);
    virtual bool isTrash();
    virtual bool joinTrash();
    virtual void onObjLoad();
    bool willFloat();
    int itemAC() const;
    int itemNoise() const;
    int itemDamroll() const;
    int itemHitroll() const;
    virtual int detonateGrenade() { return FALSE; }
    virtual int getNumRiders(TThing *) const;
    int getItemIndex() const { return (number < 0 ? 0 : number); }
    virtual bool isPluralItem() const;

    virtual void updateDesc(){};
    virtual void weightCorrection() {}
    virtual bool monkRestrictedItem(const TBeing *ch) const;
    virtual bool shamanRestrictedItem(const TBeing *ch) const;
    virtual bool rangerRestrictedItem(const TBeing *ch) const;
    virtual void assignFourValues(int, int, int, int) = 0;
    virtual void getFourValues(int *, int *, int *, int *) const = 0;
    virtual int objectSell(TBeing *, TMonster *) { return FALSE; }
    virtual bool objectRepair(TBeing *, TMonster *, silentTypeT) { return FALSE; }
    virtual int objectDecay() { return FALSE; }
    virtual void lowCheck();
    virtual bool lowCheckSlots(silentTypeT);
    virtual sstring statObjInfo() const = 0;
    virtual void setEmpty() {}
    virtual void lookObj(TBeing *, int) const;
    virtual void examineObj(TBeing *) const {}
    virtual void fillMe(const TBeing *, liqTypeT);
    virtual void pourMeOut(TBeing *);
    virtual void pourMeIntoDrink2(TBeing *, TBaseCup *);
    virtual void pourMeIntoDrink1(TBeing *, TObj *);
    virtual void waterCreate(const TBeing *, int);
    virtual void thingDumped(TBeing *, int *);
    virtual void junkMe(TBeing *);
    virtual int scavengeMe(TBeing *, TObj **);
    virtual int illuminateMe(TBeing *, int, byte);
    virtual int personalizedCheck(TBeing *);
    virtual void describeMe(TBeing *) const;
    virtual void giveToRepair(TMonster *, TBeing *, int *);
    int maxFix(const TBeing *, depreciationTypeT) const;
    int repairPrice(TBeing *, TBeing *, depreciationTypeT, bool, int * matcost) const;
    virtual void writeAffects(int, FILE *) const;
    virtual int addApply(TBeing *, applyTypeT);
    virtual void noteMe(TMonster *, TBeing*, TObj *, time_t, int) {}
    virtual int noteMeForRent(sstring &, TBeing *, TThing *, int *);
    virtual sstring displayFourValues();
    virtual void changeObjValue1(TBeing *);
    virtual void changeObjValue2(TBeing *);
    virtual void changeObjValue3(TBeing *);
    virtual void changeObjValue4(TBeing *);
    virtual int disarmMe(TBeing *);
    virtual void changeTrapValue2(TBeing *, const char *, editorEnterTypeT) {}
    virtual void changeTrapValue3(TBeing *, const char *, editorEnterTypeT) {}
    virtual void makeTrapLand(TBeing *, doorTrapT, const char *) {}
    virtual void makeTrapGrenade(TBeing *, doorTrapT, const char *) {}
    virtual void purgeMe(TBeing *);
    virtual int boardHandler(TBeing *, cmdTypeT, const char *);
    virtual void changeComponentValue4(TBeing *, const char *, editorEnterTypeT) {}
    virtual void boottimeInit() {}
    virtual void decayMe();
    virtual bool sellMeCheck(TBeing *, TMonster *, int) const;
    virtual bool fitsSellType(tObjectManipT, TBeing *, TMonster *, sstring, itemTypeT, int &, int);
    virtual int treeMe(TBeing *, const char *, int, int*);
    virtual bool canGetMeDeny(const TBeing *, silentTypeT) const;
    virtual bool canGetMe(const TBeing *, silentTypeT) const;
    virtual void changeBedValue1(TBeing *, const char *, editorEnterTypeT) {}
    virtual void changeMagicItemValue1(TBeing *, const char *, editorEnterTypeT) {}
    virtual int buyMe(TBeing *, TMonster *, int, int);
    virtual int sellMe(TBeing *, TMonster *, int, int);
    virtual void valueMe(TBeing *, TMonster *, int, int);
    virtual const sstring shopList(const TBeing *, const sstring &, int, int, int, int, int, unsigned long int) const;
    virtual int damageItem(sh_int);
    virtual void scrapMonogrammed();
    virtual void makeScraps();
    virtual int shopPrice(int, int, float, const TBeing *) const;
    virtual int sellPrice(int ,int, float, const TBeing *);
    virtual void divinateMe(TBeing *) const;
    virtual int copyMe(TBeing *, byte);
    virtual int changeItemVal2Check(TBeing *, int);
    virtual int changeItemVal3Check(TBeing *, int);
    virtual int changeItemVal4Check(TBeing *, int);
    virtual sstring getNameForShow(bool = true, bool = true, const TBeing * = NULL) const;
    virtual int foodItemUsed(TBeing *ch, const char *arg);
    virtual void changeBaseWeaponValue1(TBeing *, const char *, editorEnterTypeT) {}
    //virtual void changeBaseWeaponValue2(TBeing *, const char *, editorEnterTypeT) {}
    //virtual void changeBaseWeaponValue3(TBeing *, const char *, editorEnterTypeT) {}
    virtual void objMenu(const TBeing *) const;
    virtual int rentCost() const;
    virtual int galvanizeMe(TBeing *, byte);
    virtual int grenadeHit(TTrap *);
    int burnObject(TBeing *, int);
    int meltObject(TBeing *, int);
    int freezeObject(TBeing *, int);
    virtual int thawObject(TBeing *, int);
    virtual bool isRentable() const;
    virtual void peeMe(const TBeing *, liqTypeT);
    virtual bool engraveMe(TBeing *, TMonster *, bool);
    virtual void closeMe(TBeing *);
    virtual void lockMe(TBeing *);
    virtual void unlockMe(TBeing *);
    virtual int enterMe(TBeing *);
    virtual int getAllFrom(TBeing *, const char *);
    virtual int getObjFrom(TBeing *, const char *, const char *);
    virtual void peeOnMe(const TBeing *);
    virtual int dissectMe(TBeing *);
    virtual bool listThingRoomMe(const TBeing *) const;
    virtual bool canSeeMe(const TBeing *, infraTypeT) const;
    virtual void dropMe(TBeing *, showMeT, showRoomT);
    virtual void pickMe(TBeing *);
    virtual int trapMe(TBeing *, const char *);
    virtual void putMoneyInto(TBeing *, int);
    virtual void show_me_mult_to_char(TBeing *, showModeT, unsigned int) const;
    virtual void show_me_to_char(TBeing *, showModeT) const;
    virtual void descMaxStruct(const TBeing *, int) const {}
    sstring equip_condition(int) const;
    virtual void describeCondition(const TBeing *) const;
    virtual int drinkMe(TBeing *);
    virtual void sipMe(TBeing *);
    virtual void eatMe(TBeing *);
    virtual void tasteMe(TBeing *);
    virtual void logMe(const TBeing *, const char *) const;
    virtual void extinguishWater(TBeing *);
    virtual void extinguishWater();
    virtual void setBurning(TBeing *);
    virtual void remBurning(TBeing *);
    wearKeyT getWearKey() const;
    virtual int suggestedPrice() const;
    virtual int productionPrice() const;
    void addGlowEffects();
    bool checkOwnersList(const TPerson *, bool = false);
    virtual double objLevel() const;
    virtual void purchaseMe(TBeing *, TMonster *, int, int);
    virtual void sellMeMoney(TBeing *, TMonster *, int, int);
    virtual int taskChargeMe(TBeing *, spellNumT, int &);
    virtual int getValue() const; 
    virtual sstring wear_flags_to_sentence() const;
};

#endif
