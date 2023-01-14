//////////////////////////////////////////////////////////////////////////
//
// SneezyMUD - All rights reserved, SneezyMUD Coding Team
//
//
//////////////////////////////////////////////////////////////////////////

#pragma once

#include "obj.h"

class TTree : public TObj {
  private:
  public:
    virtual void assignFourValues(int, int, int, int);
    virtual void getFourValues(int*, int*, int*, int*) const;
    virtual itemTypeT itemType() const { return ITEM_TREE; }
    virtual sstring statObjInfo() const;

    virtual int treeMe(TBeing*, const char*, int, int*);

    TTree();
    TTree(const TTree& a);
    TTree& operator=(const TTree& a);
    virtual ~TTree();
};
