//////////////////////////////////////////////////////////////////////////
//
// SneezyMUD - All rights reserved, SneezyMUD Coding Team
//
//
//////////////////////////////////////////////////////////////////////////

#ifndef __OBJ_TRASH_H
#define __OBJ_TRASH_H

#include "obj.h"

class TTrash : public TObj {
  private:
  public:
    virtual void assignFourValues(int, int, int, int);
    virtual void getFourValues(int *, int *, int *, int *) const;
    virtual sstring statObjInfo() const;
    virtual itemTypeT itemType() const { return ITEM_TRASH; }

    virtual bool objectRepair(TBeing *, TMonster *, silentTypeT);

    TTrash();
    TTrash(const TTrash &a);
    TTrash & operator=(const TTrash &a);
    virtual ~TTrash();
};



#endif
