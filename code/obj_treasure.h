//////////////////////////////////////////////////////////////////////////
//
// SneezyMUD - All rights reserved, SneezyMUD Coding Team
//
//
//////////////////////////////////////////////////////////////////////////

#ifndef __OBJ_TREASURE_H
#define __OBJ_TREASURE_H

#include "obj.h"

class TTreasure : public TObj {
  private:
  public:
    virtual void assignFourValues(int, int, int, int);
    virtual void getFourValues(int *, int *, int *, int *) const;
    virtual sstring statObjInfo() const;
    virtual itemTypeT itemType() const { return ITEM_TREASURE; }

    virtual int scavengeMe(TBeing *, TObj **);

    TTreasure();
    TTreasure(const TTreasure &a);
    TTreasure & operator=(const TTreasure &a);
    virtual ~TTreasure();
};


#endif
