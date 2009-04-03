//////////////////////////////////////////////////////////////////////////
//
// SneezyMUD - All rights reserved, SneezyMUD Coding Team
//
//
//////////////////////////////////////////////////////////////////////////

#ifndef __OBJ_OTHER_OBJ_H
#define __OBJ_OTHER_OBJ_H

#include "obj.h"


class TOtherObj : public TObj {
  private:
  public:
    virtual void assignFourValues(int, int, int, int);
    virtual void getFourValues(int *, int *, int *, int *) const;
    virtual itemTypeT itemType() const { return ITEM_OTHER; }
    virtual sstring statObjInfo() const;

    virtual void writeAffects(int, FILE *) const;
    virtual int addApply(TBeing *, applyTypeT);
    virtual void lowCheck();

    TOtherObj();
    TOtherObj(const TOtherObj &a);
    TOtherObj & operator=(const TOtherObj &a);
    virtual ~TOtherObj();
};



#endif
