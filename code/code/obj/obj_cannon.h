#pragma once

#include "enum.h"
#include "obj.h"
#include "obj_gun.h"

class TBeing;

class TCannon : public TGun {
  public:
    virtual void loadMe(TBeing* ch, TAmmo* ammo);
    virtual void unloadMe(TBeing* ch, TAmmo* ammo);

    virtual itemTypeT itemType() const { return ITEM_CANNON; }

    int shootMeBow(TBeing*, TBeing*, unsigned int, dirTypeT, int);

    TCannon();
    TCannon(const TCannon& a);
    TCannon& operator=(const TCannon& a);
    virtual ~TCannon();
};
