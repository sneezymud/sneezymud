//////////////////////////////////////////////////////////////////////////
//
// SneezyMUD - All rights reserved, SneezyMUD Coding Team
//
//////////////////////////////////////////////////////////////////////////


#ifndef __OBJ2_H
#define __OBJ2_H

#include "obj_drug.h"


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

    virtual int chiMe(TBeing *);
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


class TBaseClothing : public virtual TObj
{
  private:
  public:
    virtual void assignFourValues(int, int, int, int) = 0;
    virtual void getFourValues(int *, int *, int *, int *) const = 0;
    virtual string statObjInfo() const = 0;

    virtual int editAverageMe(TBeing *, const char *);
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

    virtual int editAverageMe(TBeing *, const char *);
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
    virtual int putMeInto(TBeing *, TOpenContainer *);
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

class TAmmo : public TObj {
  private:
    int ammotype;
    int rounds;

  public:
    void setAmmoType(int a) { ammotype=a; }
    int getAmmoType() const { return ammotype; }
    void setRounds(int);
    int getRounds() const { return rounds; }

    virtual itemTypeT itemType() const { return ITEM_AMMO; }
    virtual void assignFourValues(int, int, int, int);
    virtual void getFourValues(int *, int *, int *, int *) const;
    virtual string statObjInfo() const;
    virtual void describeObjectSpecifics(const TBeing *) const;
    virtual string showModifier(showModeT , const TBeing *) const;

    TAmmo();
    TAmmo(const TAmmo &a);
    TAmmo & operator=(const TAmmo &a);
    virtual ~TAmmo();
};


class TGun : public TGenWeapon {
  private:
    int rof;
    int ammotype;
  
    TAmmo *ammo;
  public:    
    void setROF(int r) { rof=r; }
    int getROF() const { return rof; }
    void setAmmoType(int a) { ammotype=a; }
    int getAmmoType() const { return ammotype; }
    void setAmmo(TAmmo *a) { ammo=a; }
    TAmmo *getAmmo() const { return ammo; }

    virtual int suggestedPrice() const;
    virtual weaponT TGun::getWeaponType() const { return WEAPON_TYPE_SHOOT; }
    virtual itemTypeT itemType() const { return ITEM_GUN; }
    virtual int getCurSharp() const { return 100; }
    virtual int getMaxSharp() const { return 100; }
    virtual void assignFourValues(int, int, int, int);
    virtual void getFourValues(int *, int *, int *, int *) const;
    virtual void TGun::changeBaseWeaponValue1(TBeing *, const char *, editorEnterTypeT);
    virtual void describeObjectSpecifics(const TBeing *) const;
    virtual string showModifier(showModeT , const TBeing *) const;
    virtual string statObjInfo() const;
    TGun();
    TGun(const TGun &a);
    TGun & operator=(const TGun &a);
    virtual ~TGun();
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
    virtual void butcherMe(TBeing *, const char *);
    virtual void sacrificeMe(TBeing *, const char *);
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
    virtual int  butcherPulse(TBeing *, TBaseCorpse *);
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
    virtual int putMeInto(TBeing *, TOpenContainer *);
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
    virtual int putSomethingIntoContainer(TBeing *, TOpenContainer *);
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
    virtual int putMeInto(TBeing *, TOpenContainer *);

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
    doorTrapT trap_dam_type;
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
    virtual void makeTrapLand(TBeing *, doorTrapT, const char *);
    virtual void makeTrapGrenade(TBeing *, doorTrapT, const char *);
    virtual bool canDrop() const;

    void armGrenade(TBeing *);
    int getTrapLevel() const;
    void setTrapLevel(int r);
    int getTrapEffectType() const;
    void setTrapEffectType(int r);
    bool isTrapEffectType(unsigned int r);
    void remTrapEffectType(unsigned int r);
    void addTrapEffectType(unsigned int r);
    doorTrapT getTrapDamType() const;
    void setTrapDamType(doorTrapT r);
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
    virtual bool shamanRestrictedItem(const TBeing *) const;
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

    virtual int chiMe(TBeing *);
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

    virtual int    chiMe(TBeing *);
    virtual void   addFlameToMe(TBeing *, const char *, TThing *, bool);
    virtual void   peeOnMe(const TBeing *);
    virtual void   updateFlameInfo();
    virtual void   addFlameMessages();
    virtual void   decayMe();
    virtual int    objectDecay();
    virtual int    getMe(TBeing *, TThing *);
    virtual void   lightMe(TBeing *, silentTypeT);
    virtual void   extinguishMe(TBeing *);
    virtual int    pourWaterOnMe(TBeing *, TObj *);
    virtual string showModifier(showModeT, const TBeing *) const;
    virtual void   refuelMeLight(TBeing *, TThing *);
    virtual void   describeObjectSpecifics(const TBeing *) const;
    virtual void   assignFourValues(int, int, int, int);
    virtual bool   isLit() const;
    virtual void   getFourValues(int *, int *, int *, int *) const;
    virtual void   putLightOut();

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

    virtual int chiMe(TBeing *);
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
    virtual void evaporate(TBeing *, silentTypeT);
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
    virtual void findSomeDrink(TDrinkCon **, TBaseContainer **, TBaseContainer *);
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
