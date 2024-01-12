//////////////////////////////////////////////////////////////////////////
//
// SneezyMUD - All rights reserved, SneezyMUD Coding Team
//
//
//////////////////////////////////////////////////////////////////////////

#pragma once

#include "enum.h"
#include "obj.h"
#include "obj_gun.h"

class TBeing;

class THandgonne : public TGun {
  public:
    virtual void loadMe(TBeing* ch, TAmmo* ammo);
    virtual void unloadMe(TBeing* ch, TAmmo* ammo);

    virtual itemTypeT itemType() const { return ITEM_HANDGONNE; }

    int shootMeBow(TBeing*, TBeing*, unsigned int, dirTypeT, int);

    THandgonne();
    THandgonne(const THandgonne& a);
    THandgonne& operator=(const THandgonne& a);
    virtual ~THandgonne();
};
