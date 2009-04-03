//////////////////////////////////////////////////////////////////////////
//
// SneezyMUD - All rights reserved, SneezyMUD Coding Team
//
//
//////////////////////////////////////////////////////////////////////////

#ifndef __OBJ_SADDLE_H
#define __OBJ_SADDLE_H

#include "obj_base_clothing.h"


class TSaddle : public TBaseClothing {
  private:
  public:
    virtual void assignFourValues(int, int, int, int);
    virtual void getFourValues(int *, int *, int *, int *) const;
    virtual sstring statObjInfo() const;
    virtual itemTypeT itemType() const { return ITEM_SADDLE; }

    virtual void lowCheck();

    TSaddle();
    TSaddle(const TSaddle &a);
    TSaddle & operator=(const TSaddle &a);
    virtual ~TSaddle();
};


#endif
