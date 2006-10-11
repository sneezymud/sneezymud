//////////////////////////////////////////////////////////////////////////
//
// SneezyMUD - All rights reserved, SneezyMUD Coding Team
//
//
//////////////////////////////////////////////////////////////////////////

#ifndef __OBJ_HARNESS_H
#define __OBJ_HARNESS_H

#include "obj_base_clothing.h"


class THarness : public TBaseClothing {
  private:
  public:
    virtual void assignFourValues(int, int, int, int);
    virtual void getFourValues(int *, int *, int *, int *) const;
    virtual sstring statObjInfo() const;
    virtual itemTypeT itemType() const { return ITEM_HARNESS; }

    virtual void lowCheck();

    THarness();
    THarness(const THarness &a);
    THarness & operator=(const THarness &a);
    virtual ~THarness();
};


#endif
