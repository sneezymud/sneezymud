
//////////////////////////////////////////////////////////////////////////
//
// SneezyMUD - All rights reserved, SneezyMUD Coding Team
//
//
//////////////////////////////////////////////////////////////////////////

#ifndef __OBJ_BASE_WEAPON_H
#define __OBJ_BASE_WEAPON_H

#include "obj.h"

class TBaseWeapon : public TObj {
  private:
    int maxSharp;
    int curSharp;
    int damLevel;
    int damDev;
    liqTypeT poison;
  public:
    virtual void assignFourValues(int, int, int, int);
    virtual void getFourValues(int *, int *, int *, int *) const;
    virtual sstring statObjInfo() const = 0;

    virtual bool isPoisoned() const;
    virtual void applyPoison(TBeing *);
    virtual void setPoison(liqTypeT);
    virtual liqTypeT getPoison() const { return poison; }
    virtual int editAverageMe(TBeing *, const char *);
    virtual double baseDamage() const;
    virtual int swungObjectDamage(const TBeing *, const TBeing *) const;
    virtual void lowCheck();
    virtual sstring showModifier(showModeT, const TBeing *) const;
    virtual void divinateMe(TBeing *) const;
    virtual int damageMe(TBeing *, TBeing *, wearSlotT);
    virtual void changeBaseWeaponValue1(TBeing *, const char *, editorEnterTypeT);
    virtual int sharpenerValueMe(const TBeing *, TMonster *) const;
    virtual int sharpenerGiveMe(TBeing *,TMonster *);
    virtual void sharpenMe(TBeing *, TTool *);
    virtual void dullMe(TBeing *, TTool *);
    virtual void describeObjectSpecifics(const TBeing *) const;
    virtual sstring describeMySharp(const TBeing *) const;
    virtual void evaluateMe(TBeing *) const;
    virtual int suggestedPrice() const;
    virtual void sharpenMeStoneWeap(TBeing *, TTool *);
    virtual void dullMeFileWeap(TBeing *, TTool *);
    virtual int catchSmack(TBeing *, TBeing **, TRoom *, int, int);
    virtual bool isBluntWeapon() const;
    virtual bool isSlashWeapon() const;
    virtual bool isPierceWeapon() const;
    virtual void objMenu(const TBeing *) const;
    virtual void changeObjValue1(TBeing *);
    virtual void changeObjValue2(TBeing *);
    virtual void changeObjValue3(TBeing *);
    virtual sstring displayFourValues();
    virtual void specializationCheck(TBeing *, float *);
    virtual void descMaxStruct(const TBeing *, int) const;
    virtual int expelPrice(const TBeing *, int) const;
    virtual int wieldMe(TBeing *, char *);
    virtual void curseMe();
    virtual int poisonWeaponWeapon(TBeing *, TThing *);
    virtual int galvanizeMe(TBeing *, byte);
    virtual int enhanceMe(TBeing *, int, byte);
    virtual int rentCost() const;
    virtual sstring compareMeAgainst(TBeing *, TObj *);
    virtual sstring getNameForShow(bool, bool, const TBeing *) const;
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


#endif
