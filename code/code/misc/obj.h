//////////////////////////////////////////////////////////////////////////
//
// SneezyMUD - All rights reserved, SneezyMUD Coding Team
//
//////////////////////////////////////////////////////////////////////////

#pragma once

#include <stdint.h>
#include <stdio.h>
#include <time.h>
#include <list>
#include <utility>
#include <vector>

#include "create.h"
#include "db.h"
#include "enum.h"
#include "limbs.h"
#include "parse.h"
#include "spells.h"
#include "sstring.h"
#include "structs.h"
#include "thing.h"
#include "trap.h"
#include "liquids.h"

class TBaseCup;
class TBeing;
class TMonster;
class TObj;
class TOpenContainer;
class TPerson;
class TTrap;

extern std::vector<objIndexData> obj_index;

typedef std::list<TObj*> TObjList;
typedef std::list<TObj*>::iterator TObjIter;

extern TObjList object_list;

extern int commod_index[200];

extern TObj* read_object_buy_build(TBeing*, int nr, readFileTypeT type);
extern TObj* read_object(int nr, readFileTypeT type);
extern void log_object(TObj* obj);

// weight of 1 sip/unit of drink
// 128 fl.oz = 1 gallon = 8.337 lb. (for water)
// 1 sip = 1 fl.oz = 0.0651328
// 1 gallon = 231 cubic inches
// 231/128 = 1.8046875 ci/sip
const float SIP_WEIGHT = 0.065;
const float SIP_VOLUME = 1.8046875;

const int MAX_SWING_AFFECT = 5;
// const int MAX_OBJ_AFFECT    = 5;
const int OBJ_NOTIMER = -1;
const int MAX_TOUNGE = 3;
const int MAX_AFFECT = 25;

const int TYPE_ONE_SWING = -122;

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
  ITEM_GAS,
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
  ITEM_MONEYPOUCH,
  ITEM_FRUIT,
  MAX_OBJ_TYPES
};
const itemTypeT MIN_OBJ_TYPE = itemTypeT(0);
extern itemTypeT& operator++(itemTypeT&, int);

extern TObj* makeNewObj(itemTypeT);

const int MAX_PORTAL_TYPE = 14;

class itemInfo {
  public:
    const char* const name;
    const char* const common_name;
    const char* const val0_info;
    int val0_max;
    int val0_min;
    const char* const val1_info;
    int val1_max;
    int val1_min;
    const char* const val2_info;
    int val2_max;
    int val2_min;
    const char* const val3_info;
    int val3_max;
    int val3_min;

    itemInfo(const char* const n, const char* const cn, const char* const v0,
      int v0x, int v0n, const char* const v1, int v1x, int v1n,
      const char* const v2, int v2x, int v2n, const char* const v3, int v3x,
      int v3n);
    ~itemInfo();

  private:
    //    itemInfo() {} // prevent use
};

extern itemInfo* ItemInfo[MAX_OBJ_TYPES];

enum toolTypeT {
  TOOL_WHETSTONE,
  TOOL_ANVIL,
  TOOL_FORGE,
  TOOL_HAMMER,  // 3
  TOOL_LOCKPICK,
  TOOL_NEEDLE,  // 5
  TOOL_THREAD,
  TOOL_POISON,
  TOOL_GARROTTE,
  TOOL_FILE,
  TOOL_BOWSTRING,  // 10
  TOOL_SKIN_KNIFE,
  TOOL_HOLYWATER,
  TOOL_FLINTSTEEL,
  TOOL_TOTEM,
  TOOL_FISHINGBAIT,  // 15
  TOOL_BUTCHER_KNIFE,
  TOOL_SEED,
  TOOL_TONGS,
  TOOL_OPERATING_TABLE,
  TOOL_SCALPEL,  // 20
  TOOL_FORCEPS,
  TOOL_LADEL,
  TOOL_SOIL,
  TOOL_PLANT_OIL,
  TOOL_CHALK,  // 25
  TOOL_RUNES,
  TOOL_ENERGY,
  TOOL_PENTAGRAM,
  TOOL_CHISEL,
  TOOL_SILICA,  // 30
  TOOL_WORKBENCH,
  TOOL_LOUPE,
  TOOL_PLIERS,
  TOOL_PUNCH,
  TOOL_CORDING,  // 35
  TOOL_TAPE,
  TOOL_CANDLE,
  TOOL_GLUE,
  TOOL_ALTAR,
  TOOL_BRUSH,  // 40
  TOOL_ASTRAL_RESIN,
  TOOL_BLACK_POWDER,
  MAX_TOOL_TYPE  // move and change
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

const unsigned int ITEM_WEAR_TAKE = (1 << 0);      // 1
const unsigned int ITEM_WEAR_FINGERS = (1 << 1);   // 2
const unsigned int ITEM_WEAR_NECK = (1 << 2);      // 4
const unsigned int ITEM_WEAR_BODY = (1 << 3);      // 8
const unsigned int ITEM_WEAR_HEAD = (1 << 4);      // 16
const unsigned int ITEM_WEAR_LEGS = (1 << 5);      // 32
const unsigned int ITEM_WEAR_FEET = (1 << 6);      // 64
const unsigned int ITEM_WEAR_HANDS = (1 << 7);     // 128
const unsigned int ITEM_WEAR_ARMS = (1 << 8);      // 256
const unsigned int ITEM_WEAR_UNUSED1 = (1 << 9);   // 512
const unsigned int ITEM_WEAR_BACK = (1 << 10);     // 1024
const unsigned int ITEM_WEAR_WAIST = (1 << 11);    // 2048
const unsigned int ITEM_WEAR_WRISTS = (1 << 12);   // 4096
const unsigned int ITEM_WEAR_UNUSED2 = (1 << 13);  // 8192
const unsigned int ITEM_WEAR_HOLD = (1 << 14);     // 16384
const unsigned int ITEM_WEAR_THROW = (1 << 15);    // 32768

const unsigned int MAX_ITEM_WEARS = 16;

/* special addition for drinks */
const unsigned int DRINK_POISON = (1 << 0);
const unsigned int DRINK_PERM = (1 << 1);
const unsigned int DRINK_SPILL = (1 << 2);
const unsigned int DRINK_FROZEN = (1 << 3);

/* special addition for drinks */
const unsigned int FOOD_POISON = (1 << 0);
const unsigned int FOOD_SPOILED = (1 << 1);
const unsigned int FOOD_FISHED = (1 << 2);
const unsigned int FOOD_BUTCHERED = (1 << 3);

/* special addition for corpses */
const unsigned int CORPSE_NO_REGEN = (1 << 0);
const unsigned int CORPSE_NO_DISSECT = (1 << 1);
const unsigned int CORPSE_NO_SKIN = (1 << 2);
const unsigned int CORPSE_HALF_SKIN = (1 << 3);
const unsigned int CORPSE_PC_SKINNING = (1 << 4);
const unsigned int CORPSE_SACRIFICE = (1 << 5);
const unsigned int CORPSE_NO_BUTCHER = (1 << 6);
const unsigned int CORPSE_HALF_BUTCHERED = (1 << 7);
const unsigned int CORPSE_PC_BUTCHERING = (1 << 8);
const unsigned int CORPSE_DENY_LOOT = (1 << 9);

const int MAX_CORPSE_FLAGS = 10;  // move and change

/* for containers  - value[1] */

const unsigned int CONT_CLOSEABLE = (1 << 0);
const unsigned int CONT_PICKPROOF = (1 << 1);
const unsigned int CONT_CLOSED = (1 << 2);
const unsigned int CONT_LOCKED = (1 << 3);
const unsigned int CONT_TRAPPED = (1 << 4);
const unsigned int CONT_SECRET = (1 << 5);  // Contianer cannot be seen(window)
const unsigned int CONT_EMPTYTRAP = (1 << 6);  // Can not have a fake trap.
const unsigned int CONT_GHOSTTRAP =
  (1 << 7);  // Thief *THOUGHT* they saw a trap.
const unsigned int CONT_WEIGHTLESS = (1 << 8);
const unsigned int CONT_JAMMED = (1 << 9);
const unsigned int MAX_CONTAINER_FLAG = 9;  // move and change

const unsigned int BOW_STRING_BROKE = (1 << 0);
const unsigned int BOW_CARVED = (1 << 1);
const unsigned int BOW_SCRAPED = (1 << 2);
const unsigned int BOW_SMOOTHED = (1 << 3);

const unsigned int ARROW_FEATHERED = (1 << 0);
const unsigned int ARROW_CARVED = (1 << 1);
const unsigned int ARROW_SCRAPED = (1 << 2);
const unsigned int ARROW_SMOOTHED = (1 << 3);

// NOTE:::
// I cleared way for 4 new extra flags to be used. The last flag
// I added (nolocate) in my opinion was done improperly however
// it works so should remain unchanged. Please remove the last part
// of the comment on the line you use for your extra flag because
// I use them as a marker for quick search...Thank you
// -Jesus 10-19-2000

const unsigned int ITEM_GLOW = (1 << 0);           // 1
const unsigned int ITEM_HUM = (1 << 1);            // 2
const unsigned int ITEM_STRUNG = (1 << 2);         // 4
const unsigned int ITEM_SHADOWY = (1 << 3);        // 8
const unsigned int ITEM_PROTOTYPE = (1 << 4);      // 16
const unsigned int ITEM_INVISIBLE = (1 << 5);      // 32
const unsigned int ITEM_MAGIC = (1 << 6);          // 64
const unsigned int ITEM_NODROP = (1 << 7);         // 128
const unsigned int ITEM_BLESS = (1 << 8);          // 256
const unsigned int ITEM_SPIKED = (1 << 9);         // 512 USE ME FIRST
const unsigned int ITEM_HOVER = (1 << 10);         // 1024
const unsigned int ITEM_RUSTY = (1 << 11);         // 2048
const unsigned int ITEM_ANTI_CLERIC = (1 << 12);   // 4096
const unsigned int ITEM_ANTI_MAGE = (1 << 13);     // 8192
const unsigned int ITEM_ANTI_THIEF = (1 << 14);    // 16384
const unsigned int ITEM_ANTI_WARRIOR = (1 << 15);  // 32768
const unsigned int ITEM_ANTI_SHAMAN = (1 << 16);   // 65536
const unsigned int ITEM_ANTI_DEIKHAN = (1 << 17);  // 131072
const unsigned int ITEM_ANTI_RANGER = (1 << 18);   // 262144
const unsigned int ITEM_ANTI_MONK = (1 << 19);     // 524288
const unsigned int ITEM_PAIRED = (1 << 20);        // 1048576
const unsigned int ITEM_NORENT = (1 << 21);        // 2097152
const unsigned int ITEM_FLOAT = (1 << 22);         // 4194304
const unsigned int ITEM_NOPURGE = (1 << 23);       // 8388608
const unsigned int ITEM_NEWBIE = (1 << 24);        // 16777216
const unsigned int ITEM_NOJUNK_PLAYER =
  (1 << 25);  // 33554432 USE THIS SPOT FOR EXTRA
const unsigned int ITEM_SILVERED = (1 << 26);  // 67108864
const unsigned int ITEM_NOT_USED3 =
  (1 << 27);  // 134217728  USE THIS SPOT FOR EXTRA
const unsigned int ITEM_ATTACHED = (1 << 28);            // 268435456
const unsigned int ITEM_BURNING = (1 << 29);             // 536870912
const unsigned int ITEM_CHARRED = (1 << 30);             // 1073741824
const unsigned int ITEM_NOLOCATE = (1U << 31);  // returns negitive int

const int MAX_OBJ_STAT = 32;  // move and change

extern long objCount;

class objFlagData {
    friend class TObj;

  private:
    unsigned int extra_flags; /* If it hums,glows etc             */
    byte depreciation;

  public:
    unsigned int wear_flags; /* Where you can wear it            */
    int cost;                /* Value when sold (gp.)            */
    long bitvector;          /* To set chars bits                */
    short decay_time;
    short struct_points;
    short max_struct_points;
    int volume;

    objFlagData();
    objFlagData(const objFlagData& a);
    objFlagData& operator=(const objFlagData& a);
    ~objFlagData();
};

class TObj : public TThing {
  public:
    objFlagData obj_flags;
    objAffData affected[MAX_OBJ_AFFECT];

    sstring action_description; /* What to write when used          */
    const char* owners;

  private:
    bool isTasked;
    bool isLocked;  // set if the object should be protected from damage
                    // temporarily

  protected:
    TObj();

  public:
    virtual TThingKind getKind() const;
    TObj(const TObj&);
    TObj& operator=(const TObj&);
    virtual TThing& operator+=(TThing& t);
    virtual ~TObj();

    // VIRTUAL FUNCTIONS
    virtual sstring showModifier(showModeT, const TBeing*) const { return ""; }
    virtual bool isPersonalized() { return !action_description.empty(); }
    virtual int getVolume() const { return (obj_flags.volume); }
    virtual const sstring& getName() const { return shortDescr; }
    virtual int getSnum() const { return (snum > -1 ? snum : objVnum()); };

    virtual roomDirData* exitDir(dirTypeT door) const;
    virtual bool shouldntBeShown(wearSlotT) const;
    virtual int putSomethingIntoContainer(TBeing*, TOpenContainer*);
    virtual bool isShopSimilar(const TThing*) const;
    virtual bool isSimilar(const TThing*) const;
    virtual bool isLevitating() const;
    virtual bool fitInShop(const char*, const TBeing*) const;
    virtual bool canDrop() const;
    virtual int checkSpec(TBeing*, cmdTypeT, const char*, TThing*);
    virtual itemTypeT itemType() const = 0;
    // END VIRTUAL FUNCTIONS

    sstring monogramOwner() const;
    bool isMonogrammed() const { return !monogramOwner().empty(); }
    bool isMonogramOwner(TBeing* b, bool crossAccount) const;
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

    std::pair<int64_t, int64_t> sumAffectedByApplyType(
      applyTypeT location) const;

    virtual void setVolume(int vol) { obj_flags.volume = vol; }
    virtual void addToVolume(int vol) { obj_flags.volume += vol; }
    bool canWear(int part) const { return (obj_flags.wear_flags & part); }

    bool isUnique() const;
    short getStructPoints() const { return obj_flags.struct_points; }
    void setStructPoints(short num) { obj_flags.struct_points = num; }
    void addToStructPoints(short num) { obj_flags.struct_points += num; }
    short getMaxStructPoints() const { return obj_flags.max_struct_points; }
    void setMaxStructPoints(short num) { obj_flags.max_struct_points = num; }
    void addToMaxStructPoints(short num) { obj_flags.max_struct_points += num; }
    short getDepreciation() const {
      return 0;
      //      return obj_flags.depreciation;
    }
    void setDepreciation(short num) { obj_flags.depreciation = num; }
    void addToDepreciation(short num) { obj_flags.depreciation += num; }

    // if an object is being used by a task currently
    bool isTaskObj() const { return isTasked; }
    void setIsTaskObj(bool t) { isTasked = t; }

    // locks an object from taking damage temporarily
    bool getLocked() const { return isLocked; }
    void setLocked(bool l) { isLocked = l; }

    bool canBeMailed(sstring to) const;
    void releaseObject(TBeing*);
    virtual int checkFalling();
    void checkObjStats();
    virtual void update(int);
    virtual bool isBluntWeapon() const;
    bool canRust();
    bool willDent(int);
    bool willTear(int);
    bool willPuncture(int);
    virtual int riverFlow(int);
    int teleportRoomFlow(int);
    virtual bool isTrash();
    virtual bool joinTrash();
    virtual void onObjLoad();
    bool willFloat();
    int itemAC() const;
    int itemNoise() const;
    int itemDamroll() const;
    int itemHitroll() const;
    virtual int detonateGrenade() { return false; }
    virtual int getNumRiders(TThing*) const;
    int getItemIndex() const { return (number < 0 ? 0 : number); }
    virtual bool isPluralItem() const;

    virtual void updateDesc(){};
    virtual void weightCorrection() {}
    virtual bool monkRestrictedItem(const TBeing* ch) const;
    virtual bool shamanRestrictedItem(const TBeing* ch) const;
    virtual bool rangerRestrictedItem(const TBeing* ch) const;
    virtual void assignFourValues(int, int, int, int) = 0;
    virtual void getFourValues(int*, int*, int*, int*) const = 0;
    virtual int objectSell(TBeing*, TMonster*) { return false; }
    virtual bool objectRepair(TBeing*, TMonster*, silentTypeT) { return false; }
    virtual int objectDecay() { return false; }
    virtual void lowCheck();
    virtual bool lowCheckSlots(silentTypeT);
    virtual sstring statObjInfo() const = 0;
    virtual void setEmpty() {}
    virtual void lookObj(TBeing*, int) const;
    virtual void examineObj(TBeing*) const {}
    virtual void fillMe(const TBeing*, liqTypeT);
    virtual void pourMeOut(TBeing*);
    virtual void pourMeIntoDrink2(TBeing*, TBaseCup*);
    virtual void pourMeIntoDrink1(TBeing*, TObj*);
    virtual void waterCreate(const TBeing*, int);
    virtual void junkMe(TBeing*);
    virtual int scavengeMe(TBeing*, TObj**);
    virtual int illuminateMe(TBeing*, int, short);
    virtual int personalizedCheck(TBeing*);
    virtual void describeMe(TBeing*) const;
    virtual void giveToRepair(TMonster*, TBeing*, int*);
    int maxFix(const TBeing*, depreciationTypeT) const;
    int repairPrice(TBeing*, TBeing*, depreciationTypeT, bool,
      int* matcost) const;
    virtual void writeAffects(int, FILE*) const;
    virtual int addApply(TBeing*, applyTypeT);
    virtual void noteMe(TMonster*, TBeing*, TObj*, time_t, int) {}
    virtual sstring displayFourValues();
    virtual void changeObjValue1(TBeing*);
    virtual void changeObjValue2(TBeing*);
    virtual void changeObjValue3(TBeing*);
    virtual void changeObjValue4(TBeing*);
    virtual int disarmMe(TBeing*);
    virtual void changeTrapValue2(TBeing*, const char*, editorEnterTypeT) {}
    virtual void changeTrapValue3(TBeing*, const char*, editorEnterTypeT) {}
    virtual void makeTrapLand(TBeing*, doorTrapT, const char*) {}
    virtual void makeTrapGrenade(TBeing*, doorTrapT, const char*) {}
    virtual void purgeMe(TBeing*);
    virtual int boardHandler(TBeing*, cmdTypeT, const char*);
    virtual void changeComponentValue4(TBeing*, const char*, editorEnterTypeT) {
    }
    virtual void boottimeInit() {}
    virtual void decayMe();
    virtual bool sellMeCheck(TBeing*, TMonster*, int num, int defaultMax) const;
    virtual bool fitsSellType(tObjectManipT, TBeing*, TMonster*, sstring,
      itemTypeT, int&, int);
    virtual int treeMe(TBeing*, const char*, int, int*);
    virtual bool canGetMeDeny(const TBeing*, silentTypeT) const;
    virtual bool canGetMe(const TBeing*, silentTypeT) const;
    virtual void changeBedValue1(TBeing*, const char*, editorEnterTypeT) {}
    virtual void changeMagicItemValue1(TBeing*, const char*, editorEnterTypeT) {
    }
    virtual int buyMe(TBeing*, TMonster*, int, int);
    virtual int sellMe(TBeing*, TMonster*, int, int);
    virtual void valueMe(TBeing*, TMonster*, int, int);
    virtual const sstring shopList(const TBeing*, const sstring&, int, int, int,
      int, int, unsigned long int) const;
    virtual int damageItem(short);
    virtual bool makeScraps();
    virtual int shopPrice(int, int, float, const TBeing*) const;
    virtual int sellPrice(int, int, float, const TBeing*);
    virtual void divinateMe(TBeing*) const;
    virtual int copyMe(TBeing*, short);
    virtual int changeItemVal2Check(TBeing*, int);
    virtual int changeItemVal3Check(TBeing*, int);
    virtual int changeItemVal4Check(TBeing*, int);
    virtual sstring getNameForShow(bool = true, bool = true,
      const TBeing* = nullptr) const;
    virtual int foodItemUsed(TBeing* ch, const char* arg);
    virtual void changeBaseWeaponValue1(TBeing*, const char*,
      editorEnterTypeT) {}
    // virtual void changeBaseWeaponValue2(TBeing *, const char *,
    // editorEnterTypeT) {} virtual void changeBaseWeaponValue3(TBeing *, const
    // char *, editorEnterTypeT) {}
    virtual void objMenu(const TBeing*) const;
    virtual int galvanizeMe(TBeing*, short);
    virtual int grenadeHit(TTrap*);
    int burnObject(TBeing*, int);
    int meltObject(TBeing*, int);
    int freezeObject(TBeing*, int);
    virtual int thawObject(TBeing*, int);
    virtual bool isRentable() const;
    virtual void peeMe(const TBeing*, liqTypeT);
    virtual bool engraveMe(TBeing*, TMonster*, bool);
    virtual void closeMe(TBeing*);
    virtual void lockMe(TBeing*);
    virtual void unlockMe(TBeing*);
    virtual int enterMe(TBeing*);
    virtual int getAllFrom(TBeing*, const char*);
    virtual int getObjFrom(TBeing*, const char*, const char*);
    virtual void peeOnMe(const TBeing*);
    virtual int dissectMe(TBeing*);
    virtual bool listThingRoomMe(const TBeing*) const;
    virtual bool canSeeMe(const TBeing*, infraTypeT) const;
    virtual void dropMe(TBeing*, showMeT, showRoomT);
    virtual void pickMe(TBeing*);
    virtual int trapMe(TBeing*, const char*);
    virtual void putMoneyInto(TBeing*, int);
    virtual void show_me_mult_to_char(TBeing*, showModeT, unsigned int) const;
    virtual void show_me_to_char(TBeing*, showModeT) const;
    virtual void descMaxStruct(const TBeing*, int) const {}
    sstring equip_condition(int) const;
    virtual void describeCondition(const TBeing*) const;
    virtual int drinkMe(TBeing*);
    virtual void sipMe(TBeing*);
    virtual void eatMe(TBeing*);
    virtual void tasteMe(TBeing*);
    virtual void logMe(const TBeing*, const char*) const;
    virtual void extinguishWater(TBeing*);
    virtual void extinguishWater();
    virtual void setBurning(TBeing*);
    virtual void remBurning(TBeing*);
    wearKeyT getWearKey() const;
    virtual int suggestedPrice() const;
    virtual int productionPrice() const;
    void addGlowEffects();
    bool checkOwnersList(const TPerson*, bool = false);
    virtual double objLevel() const;
    virtual void purchaseMe(TBeing*, TMonster*, int, int);
    virtual void sellMeMoney(TBeing*, TMonster*, int, int);
    virtual int taskChargeMe(TBeing*, spellNumT, int&);
    virtual int getValue() const;
    virtual sstring wear_flags_to_sentence() const;
};
