//////////////////////////////////////////////////////////////////////////
//
// SneezyMUD - All rights reserved, SneezyMUD Coding Team
//
//
//////////////////////////////////////////////////////////////////////////

#ifndef __OBJ_GUN_H
#define __OBJ_GUN_H

#include "obj_general_weapon.h"

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


    int shootMeBow(TBeing *, TBeing *, unsigned int, dirTypeT, int);
    virtual int suggestedPrice() const;
    virtual weaponT TGun::getWeaponType(int) const { return WEAPON_TYPE_SHOOT; }
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



#endif
