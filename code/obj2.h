//////////////////////////////////////////////////////////////////////////
//
// SneezyMUD - All rights reserved, SneezyMUD Coding Team
//
// $Log: obj2.h,v $
// Revision 1.2  1999/09/16 05:42:19  peel
// Added setVolume and addToVolume to TSmoke
//
// Revision 1.1  1999/09/12 17:24:04  sneezy
// Initial revision
//
//
//////////////////////////////////////////////////////////////////////////


#ifndef __OBJ2_H
#define __OBJ2_H

class TContainer : public TObj {
  private:
  public:
    virtual void assignFourValues(int, int, int, int) = 0;
    virtual void getFourValues(int *, int *, int *, int *) const = 0;
    virtual string statObjInfo() const = 0;

    virtual int stealModifier();
    virtual void lookObj(TBeing *, int) const = 0;
    virtual void examineObj(TBeing *) const;
    virtual void logMe(const TBeing *, const char *) const;
    virtual int getAllFrom(TBeing *, const char *);
    virtual int getObjFrom(TBeing *, const char *, const char *);
    virtual int putSomethingInto(TBeing *, TThing *) = 0;
    virtual int putSomethingIntoContainer(TBeing *, TRealContainer *);
    virtual void findSomeDrink(TDrinkCon **, TContainer **, TContainer *);
    virtual void findSomeFood(TFood **, TContainer **, TContainer *);
    virtual bool engraveMe(TBeing *, TMonster *, bool);
    virtual int getReducedVolume(const TThing *) const;
    virtual void powerstoneCheck(TOpal **);
    virtual void powerstoneCheckCharged(TOpal **);
    virtual void powerstoneMostMana(int *);
    virtual bool fitsSellType(tObjectManipT, TBeing *, TMonster *, string, itemTypeT, int &, int);

  protected:
    TContainer();
  public:
    TContainer(const TContainer &a);
    TContainer & operator=(const TContainer &a);
    virtual ~TContainer();
};

class TRealContainer : public TContainer {
  private:
    float max_weight;
    unsigned char container_flags;
    unsigned char trap_type;
    char trap_dam;
    int key_num;
    int max_volume;
  public:
    virtual void assignFourValues(int, int, int, int);
    virtual void getFourValues(int *, int *, int *, int *) const;
    virtual string statObjInfo() const;

    virtual void changeObjValue2(TBeing *);
    virtual bool getObjFromMeCheck(TBeing *);
    virtual void lookObj(TBeing *, int) const;
    virtual int disarmMe(TBeing *);
    virtual int detectMe(TBeing *) const;
    virtual void pickMe(TBeing *);
    virtual int trapMe(TBeing *, const char *);
    virtual int openMe(TBeing *);
    virtual void closeMe(TBeing *);
    virtual void lockMe(TBeing *);
    virtual void unlockMe(TBeing *);
    virtual void putMoneyInto(TBeing *, int);
    virtual void describeContains(const TBeing *) const;
    virtual void lowCheck();
    virtual int sellCommod(TBeing *, TMonster *, int, TThing *);
    virtual int putSomethingInto(TBeing *, TThing *);
    virtual string compareMeAgainst(TBeing *, TObj *);

    bool isCloseable() const;
    bool isClosed() const;

    virtual float carryWeightLimit() const;
    void setCarryWeightLimit(float);
    unsigned char getContainerFlags() const;
    void setContainerFlags(unsigned char r);
    void addContainerFlag(unsigned char r);
    void remContainerFlag(unsigned char r);
    bool isContainerFlag(unsigned char r) const;
    unsigned char getContainerTrapType() const;
    void setContainerTrapType(unsigned char r);
    char getContainerTrapDam() const;
    void setContainerTrapDam(char r);
    void setKeyNum(int);
    int getKeyNum() const;
    virtual int carryVolumeLimit() const;
    void setCarryVolumeLimit(int);

  protected:
    TRealContainer();
  public:
    TRealContainer(const TRealContainer &a);
    TRealContainer & operator=(const TRealContainer &a);
    virtual ~TRealContainer();
};

class TExpandableContainer : public TRealContainer {
  private:
  public:
    virtual void assignFourValues(int, int, int, int);
    virtual void getFourValues(int *, int *, int *, int *) const;
    virtual string statObjInfo() const;

    virtual int getTotalVolume() const;
    virtual void addToCarriedVolume(int num);

  protected:
    TExpandableContainer();
  public:
    TExpandableContainer(const TExpandableContainer &a);
    TExpandableContainer & operator=(const TExpandableContainer &a);
    virtual ~TExpandableContainer();
};

class TBag : public TExpandableContainer {
  private:
  public:
    virtual void assignFourValues(int, int, int, int);
    virtual void getFourValues(int *, int *, int *, int *) const;
    virtual string statObjInfo() const;
    virtual itemTypeT itemType() const { return ITEM_BAG; }

    virtual int getMe(TBeing *, TThing *);
    virtual bool objectRepair(TBeing *, TMonster *, silentTypeT);

    TBag();
    TBag(const TBag &a);
    TBag & operator=(const TBag &a);
    virtual ~TBag();
};

class TSpellBag : public TExpandableContainer {
  private:
  public:
    virtual void assignFourValues(int, int, int, int);
    virtual void getFourValues(int *, int *, int *, int *) const;
    virtual string statObjInfo() const;
    virtual itemTypeT itemType() const { return ITEM_SPELLBAG; }

    virtual void findSomeComponent(TComponent **, TComponent **, TComponent **, spellNumT, int);
    virtual bool allowsCast() { return true; }
    virtual void putMoneyInto(TBeing *, int);
    virtual bool objectRepair(TBeing *, TMonster *, silentTypeT);
    virtual int componentSell(TBeing *, TMonster *, int, TThing *);
    virtual int componentValue(TBeing *, TMonster *, int, TThing *);
    virtual bool lowCheckSlots(silentTypeT);

    TSpellBag();
    TSpellBag(const TSpellBag &a);
    TSpellBag & operator=(const TSpellBag &a);
    virtual ~TSpellBag();
    virtual TThing & operator+= (TThing & t);
};

class TChest : public TRealContainer {
  private:
  public:
    virtual void assignFourValues(int, int, int, int);
    virtual void getFourValues(int *, int *, int *, int *) const;
    virtual string statObjInfo() const;
    virtual itemTypeT itemType() const { return ITEM_CHEST; }

    virtual bool objectRepair(TBeing *, TMonster *, silentTypeT);

    TChest();
    TChest(const TChest &a);
    TChest & operator=(const TChest &a);
    virtual ~TChest();
};

class TKeyring : public TExpandableContainer {
  private:
  public:
    virtual void assignFourValues(int, int, int, int);
    virtual void getFourValues(int *, int *, int *, int *) const;
    virtual string statObjInfo() const;
    virtual itemTypeT itemType() const { return ITEM_KEYRING; }
    virtual void putMoneyInto(TBeing *, int);
    virtual bool objectRepair(TBeing *, TMonster *, silentTypeT);

    TKeyring();
    TKeyring(const TKeyring &a);
    TKeyring & operator=(const TKeyring &a);
    virtual ~TKeyring();
};

class TBaseCorpse : public TContainer {
  private:
    unsigned int corpse_flags;
    race_t corpse_race;
    unsigned int corpse_level;
    int corpse_vnum;
  public:
    dissectInfo *tDissections;

    virtual void assignFourValues(int, int, int, int);
    virtual void getFourValues(int *, int *, int *, int *) const;
    virtual string statObjInfo() const = 0;

    virtual void peeOnMe(const TBeing *);
    virtual int dissectMe(TBeing *);
    virtual void update(int);
    virtual void lookObj(TBeing *, int) const;
    virtual int scavengeMe(TBeing *, TObj **);
    virtual void decayMe();
    virtual int objectDecay();
    virtual int putSomethingInto(TBeing *, TThing *);
    virtual void describeObjectSpecifics(const TBeing *) const {}

    void setCorpseFlags(unsigned int);
    unsigned int getCorpseFlags() const;
    void addCorpseFlag(unsigned int);
    void remCorpseFlag(unsigned int);
    bool isCorpseFlag(unsigned int) const;
    void setCorpseRace(race_t);
    race_t getCorpseRace() const;
    void setCorpseLevel(unsigned int);
    unsigned int getCorpseLevel() const;
    void setCorpseVnum(int);
    int getCorpseVnum() const;

    void setupDissectionObjects();

  protected:
    TBaseCorpse();
  public:
    TBaseCorpse(const TBaseCorpse &a);
    TBaseCorpse & operator=(const TBaseCorpse &a);
    virtual ~TBaseCorpse();
};

class TCorpse : public TBaseCorpse {
  private:
  public:
    virtual string statObjInfo() const;
    virtual itemTypeT itemType() const { return ITEM_CORPSE; }
    virtual void describeObjectSpecifics(const TBeing *) const;

    TCorpse();
    TCorpse(const TCorpse &a);
    TCorpse & operator=(const TCorpse &a);
    virtual ~TCorpse();
};

class TTable : public TObj {
  private:
  public:
    virtual void assignFourValues(int, int, int, int);
    virtual void getFourValues(int *, int *, int *, int *) const;
    virtual string statObjInfo() const;
    virtual itemTypeT itemType() const { return ITEM_TABLE; }

    virtual void lowCheck();
    virtual void writeAffects(int, FILE *) const;
    virtual void lookObj(TBeing *, int) const;
    virtual void examineObj(TBeing *) const;
    virtual bool canGetMeDeny(const TBeing *, silentTypeT) const;
    virtual int getAllFrom(TBeing *, const char *);
    virtual int getObjFrom(TBeing *, const char *, const char *);
    virtual int putSomethingInto(TBeing *, TThing *);
    virtual int putSomethingIntoTable(TBeing *, TTable *);
    virtual void getObjFromMeText(TBeing *, TThing *);
    virtual bool isSimilar(const TThing *t) const;

    TTable();
    TTable(const TTable &a);
    TTable & operator=(const TTable &a);
    virtual ~TTable();
};

class TSeeThru : public TObj {
  private:
    int target_room;
  public:
    virtual void assignFourValues(int, int, int, int);
    virtual void getFourValues(int *, int *, int *, int *) const;
    virtual string statObjInfo() const = 0;

    virtual void purgeMe(TBeing *);
    virtual bool canSeeMe(const TBeing *, infraTypeT) const;
    virtual int riverFlow(int);

    bool givesOutsideLight() const;
    int getLightFromOutside() const;

    int getTarget(int * = NULL) const;
    void setTarget(int);

  protected:
    TSeeThru();
  public:
    TSeeThru(const TSeeThru &a);
    TSeeThru & operator=(const TSeeThru &a);
    virtual ~TSeeThru();
};

class TPortal : public TSeeThru {
  private:
    char charges;
    unsigned char portal_type;
    unsigned char trap_type;
    unsigned short trap_damage;
    unsigned short portal_state;
    int portal_key;
  public:
    virtual void assignFourValues(int, int, int, int);
    virtual void getFourValues(int *, int *, int *, int *) const;
    virtual string statObjInfo() const;
    virtual itemTypeT itemType() const { return ITEM_PORTAL; }

    virtual void changeObjValue1(TBeing *);
    virtual void changeObjValue3(TBeing *);
    virtual void changeObjValue4(TBeing *);
    virtual string displayFourValues();
    virtual int openMe(TBeing *);
    virtual void closeMe(TBeing *);
    virtual void lockMe(TBeing *);
    virtual void unlockMe(TBeing *);
    virtual int enterMe(TBeing *);
    virtual int objectDecay();
    virtual void thingDumped(TBeing *, int *) {}
    virtual int detectMe(TBeing *) const;
    virtual void showMe(TBeing *) const;

    char getPortalNumCharges() const;
    void setPortalNumCharges(char r);
    unsigned char getPortalType() const;
    void setPortalType(unsigned char r);
    unsigned short getPortalFlags() const;
    void setPortalFlags(unsigned short s);
    bool isPortalFlag(unsigned short) const;
    void addPortalFlag(unsigned short);
    void remPortalFlag(unsigned short);
    int getPortalKey() const;
    void setPortalKey(int r);
    unsigned char getPortalTrapType() const;
    void setPortalTrapType(unsigned char r);
    unsigned short getPortalTrapDam() const;
    void setPortalTrapDam(unsigned short r);
    TPortal * findMatchingPortal() const;

    TPortal();
    TPortal(const TPortal &a);
    TPortal & operator=(const TPortal &a);
    virtual ~TPortal();
};

class TWindow : public TSeeThru {
  private:
  public:
    virtual void assignFourValues(int, int, int, int);
    virtual void getFourValues(int *, int *, int *, int *) const;
    virtual string statObjInfo() const;
    virtual itemTypeT itemType() const { return ITEM_WINDOW; }

    virtual void listMe(TBeing *, unsigned int) const;
    virtual void listMeExcessive(TBeing *) const;
    virtual void showMe(TBeing *) const;

    virtual void lookObj(TBeing *, int) const;
    virtual void describeMe(const TBeing *) const {}
    virtual bool listThingRoomMe(const TBeing *) const;
    virtual void show_me_mult_to_char(TBeing *, showModeT, unsigned int) const;
    virtual void show_me_to_char(TBeing *, showModeT) const;

    TWindow();
    TWindow(const TWindow &a);
    TWindow & operator=(const TWindow &a);
    virtual ~TWindow();
};

class TBow : public TObj {
  private:
    int arrowType;
    unsigned int flags;
    unsigned int max_range;
  public:
    virtual void assignFourValues(int, int, int, int);
    virtual void getFourValues(int *, int *, int *, int *) const;
    virtual string statObjInfo() const;
    virtual itemTypeT itemType() const { return ITEM_BOW; }

    virtual string showModifier(showModeT, const TBeing *) const;
    virtual void describeObjectSpecifics(const TBeing *) const;
    virtual bool isBluntWeapon() const;
    virtual void stringMeBow(TBeing *, TThing *);
    virtual void evaluateMe(TBeing *) const;
    virtual int shootMeBow(TBeing *ch, TBeing *, unsigned int, dirTypeT, int);
    virtual void bloadArrowBow(TBeing *, TArrow *);
    virtual bool sellMeCheck(const TBeing *, TMonster *) const;
    virtual string compareMeAgainst(TBeing *, TObj *);
    virtual void dropMe(TBeing *, showMeT, showRoomT);

    int getArrowType() const;
    void setArrowType(int);
    unsigned int getBowFlags() const;
    void setBowFlags(unsigned int r);
    bool isBowFlag(unsigned int r) const;
    void addBowFlags(unsigned int r);
    void remBowFlags(unsigned int r);
    unsigned int getMaxRange() const;
    void setMaxRange(unsigned int);

    TBow();
    TBow(const TBow &a);
    TBow & operator=(const TBow &a);
    virtual ~TBow();
};

class TBaseClothing : public virtual TObj
{
  private:
  public:
    virtual void assignFourValues(int, int, int, int) = 0;
    virtual void getFourValues(int *, int *, int *, int *) const = 0;
    virtual string statObjInfo() const = 0;

    virtual string showModifier(showModeT, const TBeing *) const;
    virtual void objMenu(const TBeing *) const;
    virtual int rentCost() const;
    virtual void lowCheck();
    virtual void descMaxStruct(const TBeing *, int) const;
    virtual bool sellMeCheck(const TBeing *, TMonster *) const;
    virtual int getShopPrice(int *) const;
    virtual void recalcShopData(int, int);
    virtual void evaluateMe(TBeing *) const;
    virtual int scavengeMe(TBeing *, TObj **);
    virtual void describeObjectSpecifics(const TBeing *) const;
    virtual int suggestedPrice() const;
    virtual string compareMeAgainst(TBeing *, TObj *);
    virtual string getNameForShow(bool, bool, const TBeing *) const;
    virtual bool isPluralItem() const;
    virtual void purchaseMe(TBeing *, TMonster *, int, int);
    virtual void sellMeMoney(TBeing *, TMonster *, int, int);

    int armorPriceStruct(armorLevT, double *) const;
    void setDefArmorLevel(float);
    double armorLevel(armorLevT) const;
    virtual double objLevel() const;
    void armorPercs(double *, double *) const;

    bool isSaddle() const;
    bool isShield() const;
    bool isBarding() const;

protected:
    TBaseClothing();
public:
    TBaseClothing(const TBaseClothing &a);
    TBaseClothing & operator=(const TBaseClothing &a);
    virtual ~TBaseClothing();
};

class TArmor : public virtual TBaseClothing {
  private:
  public:
    virtual void assignFourValues(int, int, int, int);
    virtual void getFourValues(int *, int *, int *, int *) const;
    virtual string statObjInfo() const;
    virtual itemTypeT itemType() const { return ITEM_ARMOR; }

    virtual void lowCheck();
    virtual int galvanizeMe(TBeing *, byte);

    TArmor();
    TArmor(const TArmor &a);
    TArmor & operator=(const TArmor &a);
    virtual ~TArmor();
};

class TWorn : public TBaseClothing {
  private:
  public:
    virtual void assignFourValues(int, int, int, int);
    virtual void getFourValues(int *, int *, int *, int *) const;
    virtual string statObjInfo() const;
    virtual itemTypeT itemType() const { return ITEM_WORN; }

    virtual void lowCheck();

    TWorn();
    TWorn(const TWorn &a);
    TWorn & operator=(const TWorn &a);
    virtual ~TWorn();
};

class TJewelry : public TBaseClothing {
  private:
  public:
    virtual void assignFourValues(int, int, int, int);
    virtual void getFourValues(int *, int *, int *, int *) const;
    virtual string statObjInfo() const;
    virtual itemTypeT itemType() const { return ITEM_JEWELRY; }

    virtual void lowCheck();

    TJewelry();
    TJewelry(const TJewelry &a);
    TJewelry & operator=(const TJewelry &a);
    virtual ~TJewelry();
};

class TBaseWeapon : public TObj {
  private:
    int maxSharp;
    int curSharp;
    int damLevel;
    int damDev;
  public:
    virtual void assignFourValues(int, int, int, int);
    virtual void getFourValues(int *, int *, int *, int *) const;
    virtual string statObjInfo() const = 0;

    virtual double baseDamage() const;
    virtual int swungObjectDamage(const TBeing *, const TBeing *) const;
    virtual void lowCheck();
    virtual string showModifier(showModeT, const TBeing *) const;
    virtual void divinateMe(TBeing *) const;
    virtual int damageMe(TBeing *, TBeing *, wearSlotT);
    virtual void changeBaseWeaponValue1(TBeing *, const char *, editorEnterTypeT);
    virtual int sharpenerValueMe(const TBeing *, TMonster *) const;
    virtual int sharpenerGiveMe(TBeing *,TMonster *);
    virtual void sharpenMe(TBeing *, TTool *);
    virtual void dullMe(TBeing *, TTool *);
    virtual void describeObjectSpecifics(const TBeing *) const;
    virtual string describeMySharp(const TBeing *) const;
    virtual void evaluateMe(TBeing *) const;
    virtual int suggestedPrice() const;
    virtual void sharpenMeStoneWeap(TBeing *, TTool *);
    virtual void dullMeFileWeap(TBeing *, TTool *);
    virtual int getShopPrice(int *) const;
    virtual void recalcShopData(int, int);
    virtual int catchSmack(TBeing *, TBeing **, TRoom *, int, int);
    virtual bool isBluntWeapon() const;
    virtual bool isSlashWeapon() const;
    virtual bool isPierceWeapon() const;
    virtual void objMenu(const TBeing *) const;
    virtual void changeObjValue1(TBeing *);
    virtual void changeObjValue2(TBeing *);
    virtual void changeObjValue3(TBeing *);
    virtual string displayFourValues();
    virtual void specializationCheck(TBeing *, float *);
    virtual void descMaxStruct(const TBeing *, int) const;
    virtual int expelPrice(const TBeing *, int) const;
    virtual int wieldMe(TBeing *, char *);
    virtual void curseMe();
    virtual int poisonWeaponWeapon(TBeing *);
    virtual int galvanizeMe(TBeing *, byte);
    virtual int enhanceMe(TBeing *, int, byte);
    virtual int rentCost() const;
    virtual string compareMeAgainst(TBeing *, TObj *);
    virtual string getNameForShow(bool, bool, const TBeing *) const;
    virtual void purchaseMe(TBeing *, TMonster *, int, int);
    virtual void sellMeMoney(TBeing *, TMonster *, int, int);

    double weaponLevel() const;
    int sharpenPrice() const;
    virtual double objLevel() const;
    double damageLevel() const;
    double structLevel() const;
    double sharpLevel() const;

    void setMaxSharp(int n);
    int getMaxSharp() const;
    void addToMaxSharp(int n);
    void setCurSharp(int n);
    int getCurSharp() const;
    void addToCurSharp(int n);
    void setWeapDamLvl(int n);
    int getWeapDamLvl() const;
    void setWeapDamDev(int n);
    int getWeapDamDev() const;

  protected:
    TBaseWeapon();
  public:
    TBaseWeapon(const TBaseWeapon &a);
    TBaseWeapon & operator=(const TBaseWeapon &a);
    virtual ~TBaseWeapon();
};

class TArrow : public TBaseWeapon {
  private:
    unsigned char arrowType;
    unsigned char arrowHead;
    unsigned int  arrowHeadMat;
    unsigned int  arrowFlags;
  public:
    virtual void assignFourValues(int, int, int, int);
    virtual void getFourValues(int *, int *, int *, int *) const;
    virtual string statObjInfo() const;
    virtual itemTypeT itemType() const { return ITEM_ARROW; }
    virtual int suggestedPrice() const;

    unsigned char getArrowType() const;
    void setArrowType(unsigned int);
    unsigned char getArrowHead() const;
    void setArrowHead(unsigned char);
    int hasFlag(const int) const;
    unsigned char getArrowHeadMat() const;
    void setArrowHeadMat(unsigned char);
    unsigned short getArrowFlags() const;
    void setArrowFlags(unsigned short);
    void remArrowFlags(unsigned short);
    bool isArrowFlag(unsigned short);
    void addArrowFlags(unsigned short);

    virtual spellNumT getWtype() const;
    virtual void evaluateMe(TBeing *) const;
    virtual bool engraveMe(TBeing *, TMonster *, bool);
    virtual void bloadBowArrow(TBeing *, TThing *);
    virtual int throwMe(TBeing *, dirTypeT, const char *);
    virtual int putMeInto(TBeing *, TRealContainer *);
    virtual string compareMeAgainst(TBeing *, TObj *);
    virtual void changeObjValue4(TBeing *);
    virtual string displayFourValues();
    virtual bool sellMeCheck(const TBeing *, TMonster *) const;

    TArrow();
    TArrow(const TArrow &a);
    TArrow & operator=(const TArrow &a);
    virtual ~TArrow();
};

class TGenWeapon : public TBaseWeapon {
  private:
    weaponT weapon_type;
  public:
    virtual void assignFourValues(int, int, int, int);
    virtual void getFourValues(int *, int *, int *, int *) const;
    virtual string statObjInfo() const;
    virtual itemTypeT itemType() const { return ITEM_WEAPON; }
    virtual int smiteWithMe(TBeing *, TBeing *);

    virtual spellNumT getWtype() const;
    virtual float blowCountSplitter(const TBeing *, bool) const;
    virtual void lowCheck();
    virtual bool sellMeCheck(const TBeing *, TMonster *) const;
  
    virtual weaponT getWeaponType() const;
    virtual void setWeaponType(weaponT n);

    bool canCudgel() const;
    bool canBackstab() const;
    bool canStab() const;

    TGenWeapon();
    TGenWeapon(const TGenWeapon &a);
    TGenWeapon & operator=(const TGenWeapon &a);
    virtual ~TGenWeapon();
};

class TMagicItem : public virtual TObj
{
  private:
    int magic_level;
    int magic_learnedness;
  public:
    virtual void assignFourValues(int, int, int, int);
    virtual void getFourValues(int *, int *, int *, int *) const;
    virtual string statObjInfo() const = 0;

    virtual void changeObjValue1(TBeing *);
    virtual string displayFourValues();
    virtual void changeMagicItemValue1(TBeing *, const char *, editorEnterTypeT);
    virtual void evaluateMe(TBeing *) const;
    virtual void divinateMe(TBeing *) const = 0;
    virtual int objectSell(TBeing *, TMonster *);

    virtual void descMagicSpells(TBeing *) const = 0;
    virtual string getNameForShow(bool, bool, const TBeing *) const = 0;
    virtual int suggestedPrice() const = 0;
    virtual void objMenu(const TBeing *) const;
    virtual void lowCheck();

    void setMagicLevel(int num);
    int getMagicLevel() const;
    void setMagicLearnedness(int num);
    int getMagicLearnedness() const;

  protected:
    TMagicItem();
  public:
    TMagicItem(const TMagicItem &a);
    TMagicItem & operator=(const TMagicItem &a);
    virtual ~TMagicItem();
};

class TPCorpse : public TBaseCorpse {
  private:
    int on_lists;
    int corpse_in_room;
    int num_corpses_in_room;
    string fileName;
    TPCorpse *nextGlobalCorpse;
    TPCorpse *nextCorpse;
    TPCorpse *previousCorpse;
  public:
    virtual void assignFourValues(int, int, int, int);
    virtual void getFourValues(int *, int *, int *, int *) const;
    virtual string statObjInfo() const;
    virtual void decayMe();
    virtual int getMe(TBeing *, TThing *);
    virtual void getMeFrom(TBeing *, TThing *);
    virtual void dropMe(TBeing *, showMeT, showRoomT);
    virtual itemTypeT itemType() const { return ITEM_PCORPSE; }
    void removeCorpseFromList(bool updateFile = TRUE);
    void addCorpseToLists();
    void saveCorpseToFile();
//    void assignCorpsesToRooms();
    int checkOnLists();
    void togOnCorpseListsOn();
    void togOnCorpseListsOff();
    void setRoomNum(int n);
    int getRoomNum() const;
    void setNumInRoom(int n);
    int getNumInRoom() const;
    void addToNumInRoom(int n);
    void setOwner(const string Name);
    const string & getOwner() const;
    void clearOwner();
    void setNext(TPCorpse *n);
    void removeNext();
    TPCorpse *getNext() const;
    void setPrevious(TPCorpse *n);
    void removePrevious();
    TPCorpse *getPrevious() const;
    void setNextGlobal(TPCorpse *n);
    void removeGlobalNext();
    virtual void describeObjectSpecifics(const TBeing *) const {};
    TPCorpse *getGlobalNext() const;
    TPCorpse();
    TPCorpse(const TPCorpse &a);
    TPCorpse & operator=(const TPCorpse &a);
    virtual ~TPCorpse();
};

class TWand : public virtual TMagicItem {
  private:
    int maxCharges;
    int curCharges;
    spellNumT spell;
    
  public:
    virtual void assignFourValues(int, int, int, int);
    virtual void getFourValues(int *, int *, int *, int *) const;
    virtual string statObjInfo() const;
    virtual itemTypeT itemType() const { return ITEM_WAND; }

    virtual int changeItemVal4Check(TBeing *, int);
    virtual void descMagicSpells(TBeing *) const;
    virtual void divinateMe(TBeing *) const;
    virtual string getNameForShow(bool, bool, const TBeing *) const;
    virtual int useMe(TBeing *, const char *);
    virtual int objectSell(TBeing *, TMonster *);
    virtual int foodItemUsed(TBeing *ch, const char *arg);
    virtual void lowCheck();
    virtual bool objectRepair(TBeing *, TMonster *, silentTypeT);
    virtual int rentCost() const;
    virtual int suggestedPrice() const;
    virtual void generalUseMessage(const TBeing *, unsigned int, const TBeing *, const TObj *) const;

    void setMaxCharges(int n);
    int getMaxCharges() const;
    void addToMaxCharges(int n);
    void setCurCharges(int n);
    int getCurCharges() const;
    void addToCurCharges(int n);
    void setSpell(spellNumT n);
    spellNumT getSpell() const;

    TWand();
    TWand(const TWand &a);
    TWand & operator=(const TWand &a);
    virtual ~TWand();
};

class TStaff : public TMagicItem {
  private:
    int maxCharges;
    int curCharges;
    spellNumT spell;
    
  public:
    virtual void assignFourValues(int, int, int, int);
    virtual void getFourValues(int *, int *, int *, int *) const;
    virtual string statObjInfo() const;
    virtual itemTypeT itemType() const { return ITEM_STAFF; }

    virtual int changeItemVal4Check(TBeing *, int);
    virtual void descMagicSpells(TBeing *) const;
    virtual void divinateMe(TBeing *) const;
    virtual string getNameForShow(bool, bool, const TBeing *) const;
    virtual int useMe(TBeing *, const char *);
    virtual int objectSell(TBeing *, TMonster *);
    virtual int foodItemUsed(TBeing *ch, const char *arg);
    virtual void lowCheck();
    virtual bool objectRepair(TBeing *, TMonster *, silentTypeT);
    virtual int rentCost() const;
    virtual int suggestedPrice() const;

    virtual int taskChargeMe(TBeing *, spellNumT, int &);
    virtual void taskChargeMeUpdate(TBeing *, spellNumT);

    void setMaxCharges(int n);
    int getMaxCharges() const;
    void addToMaxCharges(int n);
    void setCurCharges(int n);
    int getCurCharges() const;
    void addToCurCharges(int n);
    void setSpell(spellNumT n);
    spellNumT getSpell() const;

    TStaff();
    TStaff(const TStaff &a);
    TStaff & operator=(const TStaff &a);
    virtual ~TStaff();
};

class TPotion : public TMagicItem {
  private:
    spellNumT spells[3];
  public:
    virtual void assignFourValues(int, int, int, int);
    virtual void getFourValues(int *, int *, int *, int *) const;
    virtual string statObjInfo() const;
    virtual itemTypeT itemType() const { return ITEM_POTION; }

    virtual void descMagicSpells(TBeing *) const;
    virtual int changeItemVal2Check(TBeing *, int);
    virtual int changeItemVal3Check(TBeing *, int);
    virtual int changeItemVal4Check(TBeing *, int);
    virtual string getNameForShow(bool, bool, const TBeing *) const;
    virtual void lowCheck();
    virtual bool objectRepair(TBeing *, TMonster *, silentTypeT);
    virtual void divinateMe(TBeing *) const;
    virtual int quaffMe(TBeing *);
    virtual int drinkMe(TBeing *);
    virtual bool sellMeCheck(const TBeing *, TMonster *) const;
    virtual int suggestedPrice() const;

    spellNumT getSpell(int num) const;
    void setSpell(int num, spellNumT xx);

    TPotion();
    TPotion(const TPotion &a);
    TPotion & operator=(const TPotion &a);
    virtual ~TPotion();
};

class TScroll : public TMagicItem {
  private:
    spellNumT spells[3];
  public:
    virtual void assignFourValues(int, int, int, int);
    virtual void getFourValues(int *, int *, int *, int *) const;
    virtual string statObjInfo() const;
    virtual itemTypeT itemType() const { return ITEM_SCROLL; }

    virtual void descMagicSpells(TBeing *) const;
    virtual int copyMe(TBeing *, byte);
    virtual int changeItemVal2Check(TBeing *, int);
    virtual int changeItemVal3Check(TBeing *, int);
    virtual int changeItemVal4Check(TBeing *, int);
    virtual int reciteMe(TBeing *, const char *);
    virtual void divinateMe(TBeing *) const;
    virtual string getNameForShow(bool, bool, const TBeing *) const;
    virtual void lowCheck();
    virtual bool objectRepair(TBeing *, TMonster *, silentTypeT);
    virtual int suggestedPrice() const;

    spellNumT getSpell(int num) const;
    void setSpell(int num, spellNumT xx);

    TScroll();
    TScroll(const TScroll &a);
    TScroll & operator=(const TScroll &a);
    virtual ~TScroll();
};

class TCommodity : public TObj {
  private:
  public:
    virtual void assignFourValues(int, int, int, int);
    virtual void getFourValues(int *, int *, int *, int *) const;
    virtual string statObjInfo() const;
    virtual itemTypeT itemType() const { return ITEM_RAW_MATERIAL; }

    virtual void lowCheck();
    virtual void logMe(const TBeing *, const char *) const {}
    virtual void buyMe(TBeing *, TMonster *, int, int);
    virtual void sellMe(TBeing *, TMonster *, int);
    virtual int sellCommod(TBeing *, TMonster *, int, TThing *);
    virtual void valueMe(TBeing *, TMonster *, int);
    virtual const string shopList(const TBeing *, const char *, int, int, int, int, int, unsigned long int) const;
    virtual int shopPrice(int, int, int, int *) const;
    virtual int sellPrice(int, int, int *);

    int pricePerUnit() const;
    int numUnits() const;

    TCommodity();
    TCommodity(const TCommodity &a);
    TCommodity & operator=(const TCommodity &a);
    virtual ~TCommodity();
};

class TSymbol : public TObj {
  private:
    int strength;
    int max_strength;
    factionTypeT faction;
  public:
    virtual void assignFourValues(int, int, int, int);
    virtual void getFourValues(int *, int *, int *, int *) const;
    virtual string statObjInfo() const;
    virtual itemTypeT itemType() const { return ITEM_HOLY_SYM; }
    
    virtual bool lowCheckSlots(silentTypeT);
    virtual void purchaseMe(TBeing *, TMonster *, int, int);
    virtual void sellMeMoney(TBeing *, TMonster *, int, int);
    virtual int objectSell(TBeing *, TMonster *);
    virtual bool sellMeCheck(const TBeing *, TMonster *) const;
    virtual int getShopPrice(int *) const;
    virtual void recalcShopData(int, int);
    virtual void describeObjectSpecifics(const TBeing *) const;
    virtual bool allowsCast() { return true; }
    virtual void findSym(TSymbol **);
    virtual bool objectRepair(TBeing *, TMonster *, silentTypeT);
    virtual void lowCheck();
    virtual void attuneMe(TBeing *, TVial *);
    virtual void attunePulse(TBeing *);
    virtual void attunerValue(TBeing *, TMonster *);
    virtual void attunerGiven(TBeing *, TMonster *);
    virtual int suggestedPrice() const;
    virtual void objMenu(const TBeing *) const;
    double getSymbolLevel() const;
    virtual double objLevel() const;
    virtual string showModifier(showModeT, const TBeing *) const;
    virtual void evaluateMe(TBeing *) const;
    virtual string compareMeAgainst(TBeing *, TObj *);
    virtual string getNameForShow(bool, bool, const TBeing *) const;

    int getSymbolCurStrength() const;
    void setSymbolCurStrength(int r);
    void addToSymbolCurStrength(int r);
    int getSymbolMaxStrength() const;
    void setSymbolMaxStrength(int r);
    void addToSymbolMaxStrength(int r);
    factionTypeT getSymbolFaction() const;
    void setSymbolFaction(factionTypeT r);

    TSymbol();
    TSymbol(const TSymbol &a);
    TSymbol & operator=(const TSymbol &a);
    virtual ~TSymbol();
};

class TTool : public TObj {
  private:
    toolTypeT tool_type;
    int tool_uses;
    int max_tool_uses;
  public:
    virtual void assignFourValues(int, int, int, int);
    virtual void getFourValues(int *, int *, int *, int *) const;
    virtual string statObjInfo() const;
    virtual itemTypeT itemType() const { return ITEM_TOOL; }

    virtual int objectSell(TBeing *, TMonster *);
    virtual void sharpenMeStone(TBeing *, TThing *);
    virtual void dullMeFile(TBeing *, TThing *);
    virtual int poisonMePoison(TBeing *, TBaseWeapon *);
    virtual int garotteMe(TBeing *, TBeing *);
    virtual void stringMeString(TBeing *, TBow *);
    virtual void skinMe(TBeing *, const char *);
    virtual int pickWithMe(TBeing *, const char *, const char *, const char *);
    virtual void repairMeHammer(TBeing *, TObj *);
    virtual int garottePulse(TBeing *, affectedData *);
    virtual void unequipMe(TBeing *);
    virtual void describeCondition(const TBeing *) const;
    virtual void describeObjectSpecifics(const TBeing *) const;
    virtual void findSmytheTools(TTool **, TTool**);
    virtual void smythePulse(TBeing *, TObj *);
    virtual void pickPulse(TBeing *);
    virtual int  skinPulse(TBeing *, TBaseCorpse *);
    virtual void sharpenPulse(TBeing *, TThing *);
    virtual void dullPulse(TBeing *, TThing *);

    toolTypeT getToolType() const;
    void setToolType(toolTypeT r);
    int getToolUses() const;
    void setToolUses(int r);
    void addToToolUses(int r);
    int getToolMaxUses() const;
    void setToolMaxUses(int r);
    void addToToolMaxUses(int r);

    TTool();
    TTool(const TTool &a);
    TTool & operator=(const TTool &a);
    virtual ~TTool();
};

class TGemstone : public TObj {
  private:
  public:
    virtual void assignFourValues(int, int, int, int);
    virtual void getFourValues(int *, int *, int *, int *) const;
    virtual string statObjInfo() const;
    virtual itemTypeT itemType() const { return ITEM_GEMSTONE; }

    TGemstone();
    TGemstone(const TGemstone &a);
    TGemstone & operator=(const TGemstone &a);
    virtual ~TGemstone();
};

class TBook : public TObj {
  private:
  public:
    virtual void assignFourValues(int, int, int, int);
    virtual void getFourValues(int *, int *, int *, int *) const;
    virtual string statObjInfo() const;
    virtual itemTypeT itemType() const { return ITEM_BOOK; }
    virtual void lookAtObj(TBeing *, const char *, showModeT) const;

    TBook();
    TBook(const TBook &a);
    TBook & operator=(const TBook &a);
    virtual ~TBook();
};

class TBed : public TObj {
  private:
    int min_pos_use;
    int max_users;
    int max_size;
    int seat_height;
    int regen;
  public:
    virtual void assignFourValues(int, int, int, int);
    virtual void getFourValues(int *, int *, int *, int *) const;
    virtual string statObjInfo() const;
    virtual itemTypeT itemType() const { return ITEM_BED; }

    virtual bool isRideable() const { return TRUE; }
    virtual bool canGetMeDeny(const TBeing *, silentTypeT) const;
    virtual void sitMe(TBeing *);
    virtual void restMe(TBeing *);
    virtual void sleepMe(TBeing *);
    virtual int getMaxRiders() const;
    virtual int getRiderHeight() const;
    virtual int mobPulseBed(TMonster *);
    void bedRegen(TBeing *, int *, silentTypeT) const;
    virtual void changeObjValue1(TBeing *);
    virtual void changeBedValue1(TBeing *, const char *, editorEnterTypeT);
    virtual void lowCheck();

    int getMinPosUse() const;
    void setMinPosUse(int n);
    int getMaxUsers() const;
    void setMaxUsers(int n);
    int getMaxSize() const;
    void setMaxSize(int n);
    int getSeatHeight() const;
    void setSeatHeight(int n);
    int getRegen() const;
    void setRegen(int n);

    TBed();
    TBed(const TBed &a);
    TBed & operator=(const TBed &a);
    virtual ~TBed();
};

class TBandaid : public TObj {
  public:
    virtual void assignFourValues(int, int, int, int);
    virtual void getFourValues(int *, int *, int *, int *) const;
    virtual string statObjInfo() const;
    virtual itemTypeT itemType() const { return ITEM_BANDAGE; }

    virtual int removeMe(TBeing *, wearSlotT);
    virtual void scrapMe(TBeing *);
    virtual void findBandage(int *);
    virtual void destroyBandage(int *);

    TBandaid();
    TBandaid(const TBandaid &a);
    TBandaid & operator=(const TBandaid &a);
    virtual ~TBandaid();
};

class TQuiver : public TExpandableContainer {
  private:
  public:
    virtual void assignFourValues(int, int, int, int);
    virtual void getFourValues(int *, int *, int *, int *) const;
    virtual string statObjInfo() const;
    virtual itemTypeT itemType() const { return ITEM_QUIVER; }
    virtual bool objectRepair(TBeing *, TMonster *, silentTypeT);
    virtual void putMoneyInto(TBeing *, int);
    virtual void closeMe(TBeing *);
    virtual int  openMe(TBeing *);
    virtual void lockMe(TBeing *);
    virtual void unlockMe(TBeing *);

    TQuiver();
    TQuiver(const TQuiver &a);
    TQuiver & operator=(const TQuiver &a);
    virtual ~TQuiver();
};

class TTree : public TObj {
  private:
  public:
    virtual void assignFourValues(int, int, int, int);
    virtual void getFourValues(int *, int *, int *, int *) const;
    virtual itemTypeT itemType() const { return ITEM_TREE; }
    virtual string statObjInfo() const;

    virtual int treeMe(TBeing *, const char *, int, int *);

    TTree();
    TTree(const TTree &a);
    TTree & operator=(const TTree &a);
    virtual ~TTree();
};

class TComponent : public TObj {
  private:
    int charges;
    int max_charges;
    spellNumT comp_spell;
    unsigned int comp_type;
  public:
    virtual bool fitInShop(const char *, const TBeing *) const;
    virtual void assignFourValues(int, int, int, int);
    virtual void getFourValues(int *, int *, int *, int *) const;
    virtual itemTypeT itemType() const { return ITEM_COMPONENT; }
    virtual string statObjInfo() const;
    virtual string getNameForShow(bool, bool, const TBeing *) const;
    virtual const string shopList(const TBeing *, const char *, int, int, int, int, int, unsigned long int) const;

    virtual void purchaseMe(TBeing *, TMonster *, int, int);
    virtual void sellMeMoney(TBeing *, TMonster *, int, int);
    virtual bool objectRepair(TBeing *, TMonster *, silentTypeT);
    virtual void lowCheck();
    virtual void evaluateMe(TBeing *) const;
    virtual void changeObjValue4(TBeing *);
    virtual void changeComponentValue4(TBeing *, const char *, editorEnterTypeT);
    virtual void boottimeInit();
    virtual void findSomeComponent(TComponent **, TComponent **, TComponent **, spellNumT, int);
    virtual bool allowsCast() { return true; }
    virtual void update(int);
    virtual void describeObjectSpecifics(const TBeing *) const;
    virtual int putMeInto(TBeing *, TRealContainer *);
    virtual void findComp(TComponent **, spellNumT);
    virtual void decayMe();
    virtual int objectSell(TBeing *, TMonster *);
    virtual bool sellMeCheck(const TBeing *, TMonster *) const;
    virtual int componentSell(TBeing *, TMonster *, int, TThing *);
    virtual int componentValue(TBeing*, TMonster *, int, TThing *);
    virtual int getShopPrice(int *) const;
    virtual void recalcShopData(int, int);
    virtual int rentCost() const;
    virtual bool splitMe(TBeing *, const char *);
    virtual int putSomethingIntoContainer(TBeing *, TRealContainer *);
    virtual int suggestedPrice() const;
    virtual void objMenu(const TBeing *) const;
    double priceMultiplier() const;
    virtual int noteMeForRent(string &, TBeing *, TThing *, int *);
    virtual void sellMe(TBeing *, TMonster *, int);
    virtual void buyMe(TBeing *, TMonster *, int, int);

    int getComponentCharges() const;
    void setComponentCharges(int n);
    void addToComponentCharges(int n);
    int getComponentMaxCharges() const;
    void setComponentMaxCharges(int n);
    void addToComponentMaxCharges(int n);
    spellNumT getComponentSpell() const;
    void setComponentSpell(spellNumT n);
    unsigned int getComponentType() const;
    void setComponentType(unsigned int num);
    void addComponentType(unsigned int num);
    void remComponentType(unsigned int num);
    bool isComponentType(unsigned int num) const;

    TComponent();
    TComponent(const TComponent &a);
    TComponent & operator=(const TComponent &a);
    virtual ~TComponent();
    virtual TThing & operator-- ();
};

class TBoard : public TObj {
  private:
    int board_level;
  public:
    virtual void assignFourValues(int, int, int, int);
    virtual void getFourValues(int *, int *, int *, int *) const;
    virtual itemTypeT itemType() const { return ITEM_BOARD; }
    virtual string statObjInfo() const;

    virtual void purgeMe(TBeing *);
    virtual int boardHandler(TBeing *, cmdTypeT, const char *);

    int getBoardLevel() const;
    void setBoardLevel(int n);

    TBoard();
    TBoard(const TBoard &a);
    TBoard & operator=(const TBoard &a);
    virtual ~TBoard();
};

class TKey : public TObj {
  private:
  public:
    virtual void assignFourValues(int, int, int, int);
    virtual void getFourValues(int *, int *, int *, int *) const;
    virtual string statObjInfo() const;
    virtual itemTypeT itemType() const { return ITEM_KEY; }
    virtual int putMeInto(TBeing *, TRealContainer *);

    virtual void lowCheck();
    virtual bool objectRepair(TBeing *, TMonster *, silentTypeT);
    virtual int stealModifier();

    TKey();
    TKey(const TKey &a);
    TKey & operator=(const TKey &a);
    virtual ~TKey();
};

class TTrap : public TObj {
  private:
    int trap_level;
    int trap_effect;
    trap_t trap_dam_type;
    int trap_charges;
  public:
    virtual void assignFourValues(int, int, int, int);
    virtual void getFourValues(int *, int *, int *, int *) const;
    virtual itemTypeT itemType() const { return ITEM_TRAP; }
    virtual string statObjInfo() const;

    virtual void changeObjValue2(TBeing *);
    virtual void changeObjValue3(TBeing *);
    virtual void changeTrapValue2(TBeing *, const char *, editorEnterTypeT);
    virtual void changeTrapValue3(TBeing *, const char *, editorEnterTypeT);
    virtual int moveTrapCheck(TBeing *, dirTypeT);
    virtual int insideTrapCheck(TBeing *, TThing *);
    virtual int anyTrapCheck(TBeing *);
    virtual int getTrapCheck(TBeing *);
    virtual int disarmMe(TBeing *);
    virtual int detectMe(TBeing *) const;
    virtual void evaluateMe(TBeing *) const;

    virtual void listMe(TBeing *, unsigned int) const;
    virtual void listMeExcessive(TBeing *) const;

    virtual int getMe(TBeing *, TThing *);
    virtual void dropMe(TBeing *, showMeT, showRoomT);
    virtual int throwMe(TBeing *, dirTypeT, const char *);
    virtual int detonateGrenade();
    virtual void makeTrapLand(TBeing *, trap_t, const char *);
    virtual void makeTrapGrenade(TBeing *, trap_t, const char *);
    virtual bool canDrop() const;

    void armGrenade(TBeing *);
    int getTrapLevel() const;
    void setTrapLevel(int r);
    int getTrapEffectType() const;
    void setTrapEffectType(int r);
    bool isTrapEffectType(unsigned int r);
    void remTrapEffectType(unsigned int r);
    void addTrapEffectType(unsigned int r);
    trap_t getTrapDamType() const;
    void setTrapDamType(trap_t r);
    int getTrapCharges() const;
    void setTrapCharges(int r);
    int getTrapDamAmount() const;

    TTrap();
    TTrap(const TTrap &a);
    TTrap & operator=(const TTrap &a);
    virtual ~TTrap();
};

class TOtherObj : public TObj {
  private:
  public:
    virtual void assignFourValues(int, int, int, int);
    virtual void getFourValues(int *, int *, int *, int *) const;
    virtual itemTypeT itemType() const { return ITEM_OTHER; }
    virtual string statObjInfo() const;

    virtual void writeAffects(int, FILE *) const;
    virtual int addApply(TBeing *, applyTypeT);
    virtual void lowCheck();

    TOtherObj();
    TOtherObj(const TOtherObj &a);
    TOtherObj & operator=(const TOtherObj &a);
    virtual ~TOtherObj();
};

class TNote : public TObj {
  private:
    int repairman;
    int time_adjust;
    int obj_v;
  public:
    virtual void assignFourValues(int, int, int, int);
    virtual void getFourValues(int *, int *, int *, int *) const;
    virtual string statObjInfo() const;
    virtual itemTypeT itemType() const { return ITEM_NOTE; }
    virtual void showMe(TBeing *) const;

    virtual void postMe(TBeing *, const char *, boardStruct *);
    virtual int personalizedCheck(TBeing *) { return FALSE; }
    virtual void describeMe(const TBeing *) const {}
    virtual void writeMeNote(TBeing *, TPen *);
    virtual void thingDumped(TBeing *, int *);
    virtual int objectSell(TBeing *, TMonster *);
    virtual bool isPersonalized() { return FALSE; } // action_desc is not personalization
    virtual void giveToRepairNote(TMonster *, TBeing *ch, int *);
    virtual void giveToRepair(TMonster *, TBeing *ch, int *);
    virtual void junkMe(TBeing *);
    virtual void noteMe(TMonster *, TBeing *, TObj *, time_t, int);

    int getRepairman() const;
    void setRepairman(int n);
    int getTimeAdj() const;
    void setTimeAdj(int n);
    int getObjV() const;
    void setObjV(int n);

    TNote();
    TNote(const TNote &a);
    TNote & operator=(const TNote &a);
    virtual ~TNote();
};

class TMoney : public TObj {
  private:
    int money;
  public:
    virtual void assignFourValues(int, int, int, int);
    virtual void getFourValues(int *, int *, int *, int *) const;
    virtual string statObjInfo() const;
    virtual itemTypeT itemType() const { return ITEM_MONEY; }

    virtual int scavengeMe(TBeing *, TObj **);
    virtual int getMe(TBeing *, TThing *);
    virtual int moneyMeMoney(TBeing *, TThing *);
    virtual void logMe(const TBeing *, const char *) const {}
    virtual int rentCost() const;
    virtual void moneyMove(TBeing *);
    virtual bool canCarryMe(const TBeing *, silentTypeT) const;
    virtual bool isPluralItem() const;

    int getMoney() const;
    void setMoney(int n);

    TMoney();
    TMoney(const TMoney &a);
    TMoney & operator=(const TMoney &a);
    virtual ~TMoney();
};

class TTreasure : public TObj {
  private:
  public:
    virtual void assignFourValues(int, int, int, int);
    virtual void getFourValues(int *, int *, int *, int *) const;
    virtual string statObjInfo() const;
    virtual itemTypeT itemType() const { return ITEM_TREASURE; }

    virtual int scavengeMe(TBeing *, TObj **);

    TTreasure();
    TTreasure(const TTreasure &a);
    TTreasure & operator=(const TTreasure &a);
    virtual ~TTreasure();
};

class TBoat : public TObj {
  private:
  public:
    virtual void assignFourValues(int, int, int, int);
    virtual void getFourValues(int *, int *, int *, int *) const;
    virtual string statObjInfo() const;
    virtual itemTypeT itemType() const { return ITEM_BOAT; }

    virtual int  putSomethingInto(TBeing *, TThing *);
    virtual int  getObjFrom(TBeing *, const char *, const char *);
    virtual int  getLight() const;
    virtual void usingBoat(int *n);

    TBoat();
    TBoat(const TBoat &a);
    TBoat & operator=(const TBoat &a);
    virtual ~TBoat();
};

class TTrash : public TObj {
  private:
  public:
    virtual void assignFourValues(int, int, int, int);
    virtual void getFourValues(int *, int *, int *, int *) const;
    virtual string statObjInfo() const;
    virtual itemTypeT itemType() const { return ITEM_TRASH; }

    virtual bool objectRepair(TBeing *, TMonster *, silentTypeT);

    TTrash();
    TTrash(const TTrash &a);
    TTrash & operator=(const TTrash &a);
    virtual ~TTrash();
};

class TAudio : public TObj {
  private:
    int freq;
  public:
    virtual void assignFourValues(int, int, int, int);
    virtual void getFourValues(int *, int *, int *, int *) const;
    virtual string statObjInfo() const;
    virtual itemTypeT itemType() const { return ITEM_AUDIO; }

    virtual void audioCheck(int) const;

    int getFreq() const;
    void setFreq(int n);
    TAudio();
    TAudio(const TAudio &a);
    TAudio & operator=(const TAudio &a);
    virtual ~TAudio();
};

class TOrganic : public TObj {
  private:
    organicTypeT OCType;
    int TUnits;
    int OLevel;
    int TAEffect;
  public:
    virtual void   assignFourValues(int, int, int, int);
    virtual void   getFourValues(int *, int *, int *, int *) const;
    virtual string statObjInfo() const;
    virtual itemTypeT itemType() const {return ITEM_RAW_ORGANIC; }

    virtual int  objectSell(TBeing *, TMonster *);
    virtual void describeObjectSpecifics(const TBeing *) const;
    virtual bool splitMe(TBeing *, const char *);
    virtual void lightMe(TBeing *, silentTypeT);
    virtual int  sellPrice(int, int, int *);
    virtual int  shopPrice(int, int, int, int *) const;
    virtual void buyMe(TBeing *, TMonster *, int, int);
    virtual void sellMe(TBeing *ch, TMonster *, int);
    virtual void valueMe(TBeing *, TMonster *, int);
    virtual const string shopList(const TBeing *, const char *, int, int, int, int, int, unsigned long int) const;
    virtual int sellHidenSkin(TBeing *, TMonster *, int, TThing *);
    void setOType(organicTypeT);
    organicTypeT getOType() const;
    void setUnits(int);
    int  getUnits() const;
    void setOLevel(int);
    int  getOLevel() const;
    void setAEffect(int);
    int  getAEffect() const;

    TOrganic();
    TOrganic(const TOrganic &a);
    TOrganic & operator=(const TOrganic &a);
    virtual ~TOrganic();
};

class TASubstance : public TObj {
  private:
    int AFlags,
        AMethod,
        ILevel,
        ISpell;
  public:
    virtual void assignFourValues(int, int, int, int);
    virtual void getFourValues(int *, int *, int *, int *) const;
    virtual string statObjInfo() const;
    virtual itemTypeT itemType() const {return ITEM_APPLIED_SUB; }

    virtual void describeObjectSpecifics(const TBeing *) const;
    virtual int  applyMe(TBeing *, TThing *);
    void setAFlags(int);
    int getAFlags() const;
    void setAMethod(int);
    int getAMethod() const;
    void setILevel(int);
    int getILevel() const;
    void setISpell(int);
    int getISpell() const;

    TASubstance();
    TASubstance(const TASubstance &a);
    TASubstance & operator=(const TASubstance &a);
    virtual ~TASubstance();
};

class TOpal : public TObj {
  private:
    int psSize;
    int psStrength;
    int psMana;
    int psFails;
  public:
    virtual void assignFourValues(int, int, int, int);
    virtual void getFourValues(int *, int *, int *, int *) const;
    virtual int objectSell(TBeing *, TMonster *);
    virtual string statObjInfo() const;
    virtual void describeObjectSpecifics(const TBeing *) const;
    virtual void powerstoneCheck(TOpal **);
    virtual void powerstoneCheckCharged(TOpal **);
    virtual void powerstoneMostMana(int *);
    virtual int powerstoneMe(TBeing *, int, byte);
    virtual itemTypeT itemType() const { return ITEM_OPAL; }
    virtual string compareMeAgainst(TBeing *, TObj *);
    virtual int suggestedPrice() const;
    virtual void lowCheck();

    int psGetStrength() const;
    void psSetStrength(int num);
    void psAddStrength(int num);
    int psGetMana() const;
    void psSetMana(int num);
    void psAddMana(int num);
    int psGetConsecFails() const;
    void psSetConsecFails(int num);
    void psAddConsecFails(int num);
    int psGetMaxMana() const;
    int psGetCarats() const;
    void psSetCarats(int num);

    TOpal();
    TOpal(const TOpal &a);
    TOpal & operator=(const TOpal &a);
    virtual ~TOpal();
};

class TPen : public TObj {
  private:
  public:
    virtual void assignFourValues(int, int, int, int);
    virtual void getFourValues(int *, int *, int *, int *) const;
    virtual string statObjInfo() const;
    virtual void junkMe(TBeing *) {}
    virtual void thingDumped(TBeing *, int *);
    virtual itemTypeT itemType() const { return ITEM_PEN; }

    virtual void writeMePen(TBeing *, TThing *);

    TPen();
    TPen(const TPen &a);
    TPen & operator=(const TPen &a);
    virtual ~TPen();
};

class TFuel : public TObj {
  private:
    int curFuel;
    int maxFuel;
  public:
    virtual void assignFourValues(int, int, int, int);
    virtual void getFourValues(int *, int *, int *, int *) const;
    virtual void lowCheck();
    virtual void describeObjectSpecifics(const TBeing *) const;
    virtual string statObjInfo() const;
    virtual int objectSell(TBeing *, TMonster *);
    virtual void refuelMeFuel(TBeing *, TLight *);
    virtual itemTypeT itemType() const { return ITEM_FUEL; }
    virtual int getVolume() const;
    virtual float getTotalWeight(bool) const;

    void addToMaxFuel(int n);
    void setMaxFuel(int n);
    int getMaxFuel() const;
    void addToCurFuel(int n);
    void setCurFuel(int n);
    int getCurFuel() const;

    TFuel();
    TFuel(const TFuel &a);
    TFuel & operator=(const TFuel &a);
    virtual ~TFuel();
};

class TBaseLight : public TObj {
  protected:
    int amtLight;
    int maxBurn;
    int curBurn;
  public:
    virtual void lookObj(TBeing *, int) const {}
    virtual void assignFourValues(int, int, int, int);
    virtual void getFourValues(int *, int *, int *, int *) const;

    virtual void lightMe(TBeing *, silentTypeT) = 0;
    virtual bool monkRestrictedItem(const TBeing *) const;
    virtual bool rangerRestrictedItem(const TBeing *) const;
    virtual void putLightOut();
    virtual string compareMeAgainst(TBeing *, TObj *);

    void addToLightAmt(int n);
    void setLightAmt(int n);
    int getLightAmt() const;
    void addToMaxBurn(int n);
    void setMaxBurn(int n);
    int getMaxBurn() const;
    void addToCurBurn(int n);
    void setCurBurn(int n);
    int getCurBurn() const;
    virtual bool isLit() const = 0;

    TBaseLight();
    TBaseLight(const TBaseLight &a);
    TBaseLight & operator=(const TBaseLight &a);
    virtual ~TBaseLight();
};

class TLight : public TBaseLight {
  protected:
    bool lit;
  public:
    virtual void assignFourValues(int, int, int, int);
    virtual void getFourValues(int *, int *, int *, int *) const;
    virtual void lowCheck();
    virtual void describeObjectSpecifics(const TBeing *) const;
    virtual string statObjInfo() const;
    virtual itemTypeT itemType() const { return ITEM_LIGHT; }

    virtual int illuminateMe(TBeing *, int, byte);
    virtual void refuelMeLight(TBeing *, TThing*);
    virtual int objectDecay();
    virtual void unequipMe(TBeing *);
    virtual void extinguishWater(TBeing *);
    virtual void extinguishWater();
    virtual void lampLightStuff(TMonster *);
    virtual void lightDecay();
    virtual string showModifier(showModeT, const TBeing *) const;
    virtual void adjustLight();
    virtual int getMe(TBeing *, TThing *);
    virtual bool isSimilar(const TThing *t) const;
    virtual void peeOnMe(const TBeing *);
    virtual void lightMe(TBeing *, silentTypeT);
    virtual void extinguishMe(TBeing *);
    virtual void putLightOut();

    void genericExtinguish(const TBeing *);
    void setLit(bool n);
    virtual bool isLit() const;

    TLight();
    TLight(const TLight &a);
    TLight & operator=(const TLight &a);
    virtual ~TLight();
};

class TDrug : public TObj {
  private:
    int curFuel;
    int maxFuel;
    drugTypeT drugType;
  public:
    virtual void assignFourValues(int, int, int, int);
    virtual void getFourValues(int *, int *, int *, int *) const;
    virtual void lowCheck();
    virtual void describeObjectSpecifics(const TBeing *) const;
    virtual string statObjInfo() const;
    virtual int objectSell(TBeing *, TMonster *);
    virtual void refuelMeDrug(TBeing *, TDrugContainer *);
    virtual itemTypeT itemType() const { return ITEM_DRUG; }
    virtual int getVolume() const;
    virtual float getTotalWeight(bool) const;

    void addToMaxFuel(int n);
    void setMaxFuel(int n);
    int getMaxFuel() const;
    void addToCurFuel(int n);
    void setCurFuel(int n);
    int getCurFuel() const;
    void setDrugType(drugTypeT n);
    drugTypeT getDrugType() const;

    TDrug();
    TDrug(const TDrug &a);
    TDrug & operator=(const TDrug &a);
    virtual ~TDrug();
};

class TDrugContainer : public TObj {
  protected:
    drugTypeT drugType;
    int maxBurn;
    int curBurn;
    bool lit;
  public:
    virtual void lookObj(TBeing *, int) const {}
    virtual void assignFourValues(int, int, int, int);
    virtual void getFourValues(int *, int *, int *, int *) const;
    virtual void lowCheck();
    virtual void describeObjectSpecifics(const TBeing *) const;
    virtual string statObjInfo() const;
    virtual itemTypeT itemType() const { return ITEM_DRUG_CONTAINER; }

    virtual bool isSimilar(const TThing *t) const;
    virtual void lightDecay();
    virtual int objectDecay();
    virtual void extinguishWater(TBeing *);
    virtual void extinguishWater();
    virtual string showModifier(showModeT, const TBeing *) const;
    virtual void lightMe(TBeing *, silentTypeT);
    virtual void extinguishMe(TBeing *);
    virtual bool monkRestrictedItem(const TBeing *) const;
    virtual bool rangerRestrictedItem(const TBeing *) const;
    virtual void refuelMeLight(TBeing *, TThing *);
    virtual void peeOnMe(const TBeing *);

    void putLightOut();

    void addToMaxBurn(int n);
    void setMaxBurn(int n);
    int getMaxBurn() const;
    void addToCurBurn(int n);
    void setCurBurn(int n);
    int getCurBurn() const;
    void setLit(bool n);
    bool getLit() const;
    void setDrugType(drugTypeT n);
    drugTypeT getDrugType() const;

    TDrugContainer();
    TDrugContainer(const TDrugContainer &a);
    TDrugContainer & operator=(const TDrugContainer &a);
    virtual ~TDrugContainer();
};

const unsigned int TFFLAME_INVHEAT  = (1 << 0); //  1
const unsigned int TFFLAME_INVLIGHT = (1 << 1); //  2
const unsigned int TFFLAME_MAGHEAT  = (1 << 2); //  4
const unsigned int TFFLAME_MAGLIGHT = (1 << 3); //  8
const unsigned int TFFLAME_IMMORTAL = (1 << 4); // 16

class TFFlame : public TBaseLight {
  private:
    int magBV;
  public:
    virtual string statObjInfo() const;
    virtual itemTypeT itemType() const { return ITEM_FLAME; }

    virtual void   addFlameToMe(TBeing *, const char *, TThing *, bool);
    virtual void   peeOnMe(const TBeing *);
    virtual void   updateFlameInfo();
    virtual void   addFlameMessages();
    virtual void   decayMe();
    virtual int    objectDecay();
    virtual int    getMe(TBeing *, TThing *);
    virtual void lightMe(TBeing *, silentTypeT);
    virtual void   extinguishMe(TBeing *);
    virtual int    pourWaterOnMe(TBeing *, TObj *);
    virtual string showModifier(showModeT, const TBeing *) const;
    virtual void   refuelMeLight(TBeing *, TThing *);
    virtual void   describeObjectSpecifics(const TBeing *) const;
    virtual void   assignFourValues(int, int, int, int);
    virtual bool isLit() const;
    virtual void   getFourValues(int *, int *, int *, int *) const;
    virtual void putLightOut();

    int  igniteMessage(TBeing *) const;
    void setMagBV(int);
    int  getMagBV() const;
    bool hasMagBV(int) const;
    void addMagBV(int);
    void remMagBV(int);

    TFFlame();
    TFFlame(const TFFlame &a);
    TFFlame & operator=(const TFFlame &a);
    virtual ~TFFlame();
};

class TSmoke : public TObj {
  public:    
    virtual void assignFourValues(int, int, int, int);
    virtual void getFourValues(int *, int *, int *, int *) const;
    virtual string statObjInfo() const;
    void updateDesc();
    virtual void decayMe();
    virtual bool isPluralItem() const;
    virtual itemTypeT itemType() const { return ITEM_SMOKE; }
    virtual void setVolume(int);
    virtual void addToVolume(int);

    int getSizeIndex() const;

    TSmoke();
    TSmoke(const TSmoke &a);
    TSmoke & operator=(const TSmoke &a);
    virtual ~TSmoke();
};

class TFood : public TObj {
  private:
    unsigned int foodFlags;
    int foodFill;
  public:
    virtual void assignFourValues(int, int, int, int);
    virtual void getFourValues(int *, int *, int *, int *) const;
    virtual int objectSell(TBeing *, TMonster *);
    virtual bool objectRepair(TBeing *, TMonster *, silentTypeT);
    virtual int objectDecay();
    virtual void describeCondition(const TBeing *) const;
    virtual void describeObjectSpecifics(const TBeing *) const;
    virtual void lowCheck();
    virtual string statObjInfo() const;
    virtual itemTypeT itemType() const { return ITEM_FOOD; }
    virtual string compareMeAgainst(TBeing *, TObj *);
    virtual int suggestedPrice() const;

    virtual bool poisonObject();
    virtual void nukeFood();
    virtual void findSomeFood(TFood **, TContainer **, TContainer *);
    virtual void eatMe(TBeing *);
    virtual void tasteMe(TBeing *);
    virtual void purchaseMe(TBeing *, TMonster *, int, int);
    virtual void sellMeMoney(TBeing *, TMonster *, int, int);

    int getFoodFill() const;
    void setFoodFill(int r);
    unsigned int getFoodFlags() const;
    void setFoodFlags(unsigned int r);
    bool isFoodFlag(unsigned int r) const;
    void addFoodFlags(unsigned int r);
    void remFoodFlags(unsigned int r);

    TFood();
    TFood(const TFood &a);
    TFood & operator=(const TFood &a);
    virtual ~TFood();

};

class TBaseCup : public TObj {
  private:
    int maxDrinks;
    int curDrinks;
    liqTypeT liquidType;
    unsigned int drinkFlags;
  public:
    virtual void assignFourValues(int, int, int, int);
    virtual void getFourValues(int *, int *, int *, int *) const;
    virtual int objectSell(TBeing *, TMonster *);
    virtual bool objectRepair(TBeing *, TMonster *, silentTypeT);
    virtual void describeContains(const TBeing *) const;
    virtual void lowCheck();
    virtual bool waterSource();
    virtual string statObjInfo() const;

    virtual bool poisonObject();
    virtual int freezeObject(TBeing *, int);
    virtual void nukeFood();
    virtual int drinkMe(TBeing *);
    virtual int quaffMe(TBeing *);
    virtual void sipMe(TBeing *);
    virtual void tasteMe(TBeing *);
    virtual void pourMeOut(TBeing *);
    virtual void pourMeIntoDrink2(TBeing *, TBaseCup *);
    virtual void pourMeIntoDrink1(TBeing *, TObj *);
    virtual void spill(const TBeing *);
    virtual void fillMe(const TBeing *, liqTypeT);
    virtual void weightCorrection();
    virtual void evaporate(TBeing *);
    virtual void weightChangeObject(float);
    virtual void setEmpty();
    virtual void lookObj(TBeing *, int) const;
    virtual void examineObj(TBeing *) const;
    virtual void peeMe(const TBeing *);
    virtual int getReducedVolume(const TThing *) const;

    void genericEmpty();
    unsigned int getDrinkConFlags() const;
    void setDrinkConFlags(unsigned int r);
    bool isDrinkConFlag(unsigned int r) const;
    void addDrinkConFlags(unsigned int r);
    void remDrinkConFlags(unsigned int r);
    int getMaxDrinkUnits() const;
    void setMaxDrinkUnits(int n);
    void addToMaxDrinkUnits(int n);
    int getDrinkUnits() const;
    virtual void setDrinkUnits(int n);
    virtual void addToDrinkUnits(int n);
    liqTypeT getDrinkType() const;
    virtual void setDrinkType(liqTypeT n);
    int getLiqDrunk() const;
    int getLiqHunger() const;
    int getLiqThirst() const;

    TBaseCup();
    TBaseCup(const TBaseCup &a);
    TBaseCup & operator=(const TBaseCup &a);
    virtual ~TBaseCup();
};

class TDrinkCon : public TBaseCup {
  public:
    virtual itemTypeT itemType() const { return ITEM_DRINKCON; }
    virtual void findSomeDrink(TDrinkCon **, TContainer **, TContainer *);
    virtual void waterCreate(const TBeing *, int);
    virtual int divineMe(TBeing *, int, byte);

    TDrinkCon();
    TDrinkCon(const TDrinkCon &a);
    TDrinkCon & operator=(const TDrinkCon &a);
    virtual ~TDrinkCon();
};

class TVial : public TBaseCup {
  public:
    virtual void findVialAttune(TVial **, int *);
    virtual void getBestVial(TVial **);
    virtual int objectSell(TBeing *, TMonster *);
    virtual bool objectRepair(TBeing *, TMonster *, silentTypeT);
    virtual itemTypeT itemType() const { return ITEM_VIAL; }

    virtual int rentCost() const;
    virtual int suggestedPrice() const;
    virtual void lowCheck();

    TVial();
    TVial(const TVial &a);
    TVial & operator=(const TVial &a);
    virtual ~TVial();
};

class TPool : public TBaseCup {
  public:    
    void fillMeAmount(int, liqTypeT);
    void updateDesc();

    virtual void setDrinkUnits(int);    
    virtual void addToDrinkUnits(int);    
    virtual void decayMe();
    virtual void setDrinkType(liqTypeT);
    virtual void weightChangeObject(float);
    virtual void peeMe(const TBeing *);
    virtual bool isPluralItem() const;
    virtual itemTypeT itemType() const { return ITEM_POOL; }

    int getDrinkIndex() const;

    TPool();
    TPool(const TPool &a);
    TPool & operator=(const TPool &a);
    virtual ~TPool();
};

class TStatue : public TObj {
  private:
  public:
    TMonster *myGargoyle;

    virtual void assignFourValues(int, int, int, int);
    virtual void getFourValues(int *, int *, int *, int *) const;
    virtual string statObjInfo() const;
    virtual itemTypeT itemType() const { return ITEM_STATUE; }

    virtual void writeAffects(int, FILE *) const;
    virtual int addApply(TBeing *, applyTypeT);
    virtual void lowCheck();

    TStatue();
    TStatue(const TStatue &a);
    TStatue & operator=(const TStatue &a);
    virtual ~TStatue();
};

class TArmorWand : public virtual TArmor, public virtual TWand
{
  public:
    virtual void assignFourValues(int, int, int, int);
    virtual void getFourValues(int *, int *, int *, int *) const;
    virtual string statObjInfo() const;
    virtual itemTypeT itemType() const { return ITEM_ARMOR_WAND; }

    virtual void lowCheck();
    virtual int suggestedPrice() const;
    virtual int rentCost() const;
    virtual void objMenu(const TBeing *) const;
    virtual void evaluateMe(TBeing *) const;
    virtual void generalUseMessage(const TBeing *, unsigned int, const TBeing *, const TObj *) const;
    virtual string getNameForShow(bool, bool, const TBeing *) const;

    TArmorWand();
    TArmorWand(const TArmorWand &a);
    TArmorWand & operator=(const TArmorWand &a);
    virtual ~TArmorWand();
};

#endif
