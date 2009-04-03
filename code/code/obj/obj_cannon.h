#ifndef __OBJ_CANNON_H
#define __OBJ_CANNON_H

#include "obj_gun.h"


class TCannon : public TGun {
 public:
  virtual void loadMe(TBeing *ch, TAmmo *ammo);
  virtual void unloadMe(TBeing *ch, TAmmo *ammo);

  virtual itemTypeT itemType() const { return ITEM_CANNON; }

  int shootMeBow(TBeing *, TBeing *, unsigned int, dirTypeT, int);

  TCannon();
  TCannon(const TCannon &a);
  TCannon & operator=(const TCannon &a);
  virtual ~TCannon();
};


#endif
