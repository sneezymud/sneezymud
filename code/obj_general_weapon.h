//////////////////////////////////////////////////////////////////////////
//
// SneezyMUD - All rights reserved, SneezyMUD Coding Team
//
//
//////////////////////////////////////////////////////////////////////////

#ifndef __OBJ_GENERAL_WEAPON_H
#define __OBJ_GENERAL_WEAPON_H

#include "obj_base_weapon.h"


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



#endif
