//////////////////////////////////////////////////////////////////////////
//
// SneezyMUD - All rights reserved, SneezyMUD Coding Team
//
//////////////////////////////////////////////////////////////////////////


#ifndef __OBJ2_H
#define __OBJ2_H

#include "obj_drug.h"

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
