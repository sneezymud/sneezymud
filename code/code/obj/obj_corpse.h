//////////////////////////////////////////////////////////////////////////
//
// SneezyMUD - All rights reserved, SneezyMUD Coding Team
//
//
//////////////////////////////////////////////////////////////////////////

#pragma once

#include "obj_base_corpse.h"

// corpses for non-PCs
class TCorpse : public TBaseCorpse {
  private:
  public:
    virtual sstring statObjInfo() const;
    virtual itemTypeT itemType() const { return ITEM_CORPSE; }
    virtual void describeObjectSpecifics(const TBeing*) const;

    TCorpse();
    TCorpse(const TCorpse& a);
    TCorpse& operator=(const TCorpse& a);
    virtual ~TCorpse();
};
