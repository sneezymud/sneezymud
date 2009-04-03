//////////////////////////////////////////////////////////////////////////
//
// SneezyMUD - All rights reserved, SneezyMUD Coding Team
//
//
//////////////////////////////////////////////////////////////////////////

#ifndef __OBJ_STATUE_H
#define __OBJ_STATUE_H

#include "obj.h"

class TStatue : public TObj {
  private:
  public:
    TMonster *myGargoyle;

    virtual void assignFourValues(int, int, int, int);
    virtual void getFourValues(int *, int *, int *, int *) const;
    virtual sstring statObjInfo() const;
    virtual itemTypeT itemType() const { return ITEM_STATUE; }

    virtual void writeAffects(int, FILE *) const;
    virtual int addApply(TBeing *, applyTypeT);
    virtual void lowCheck();

    TStatue();
    TStatue(const TStatue &a);
    TStatue & operator=(const TStatue &a);
    virtual ~TStatue();
};



#endif
