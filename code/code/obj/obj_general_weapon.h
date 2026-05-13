//////////////////////////////////////////////////////////////////////////
//
// SneezyMUD - All rights reserved, SneezyMUD Coding Team
//
//
//////////////////////////////////////////////////////////////////////////

#pragma once

#include "obj_base_weapon.h"

class TGenWeapon : public TBaseWeapon {
  private:
    weaponT weapon_type[3];
    int wtype_frequency[3];

  public:
    virtual void assignFourValues(int, int, int, int);
    virtual void getFourValues(int*, int*, int*, int*) const;
    virtual sstring statObjInfo() const;
    virtual itemTypeT itemType() const { return ITEM_WEAPON; }
    virtual int smiteWithMe(TBeing*, TBeing*);

    virtual spellNumT getWtype(int which = -1) const;
    virtual float blowCountSplitter(const TBeing*, bool) const;
    virtual void lowCheck();
    virtual bool sellMeCheck(TBeing*, TMonster*, int, int) const;

    virtual weaponT getWeaponType(int which = -1) const;
    virtual void setWeaponType(int n, int which = 0);
    int getWeaponFreq(int which) const;
    void setWeaponFreq(int n, int which = 0);

    virtual bool canCudgel() const;
    virtual bool canBackstab() const;
    virtual bool canStab() const;

    TGenWeapon();
    TGenWeapon(const TGenWeapon& a);
    TGenWeapon& operator=(const TGenWeapon& a);
    virtual ~TGenWeapon();
};

int impactSpec(TBeing* ch, TBeing* victim, wearSlotT damSource, wearSlotT pos);
int spikesHit(TBeing* victim, TBeing* ch, TObj* obj, wearSlotT limb);
int spikesBreak(TBeing* victim, TBeing* ch, TObj* obj);
int thornsHit(TBeing* victim, TBeing* ch, wearSlotT chLimb, wearSlotT vicLimb);
int hardHit(TBeing* victim, TBeing* ch, TObj* obj, wearSlotT vicLimb, wearSlotT chLimb);
