//////////////////////////////////////////////////////////////////////////
//
// SneezyMUD - All rights reserved, SneezyMUD Coding Team
//
//
//////////////////////////////////////////////////////////////////////////

#ifndef __OBJ_SMOKE_H
#define __OBJ_SMOKE_H

#include "obj.h"

class TSmoke : public TObj {
  public:    
    virtual void assignFourValues(int, int, int, int);
    virtual void getFourValues(int *, int *, int *, int *) const;
    virtual sstring statObjInfo() const;
    void updateDesc();
    virtual void decayMe();
    virtual bool isPluralItem() const;
    virtual itemTypeT itemType() const { return ITEM_SMOKE; }
    virtual void setVolume(int);
    virtual void addToVolume(int);

    int getSizeIndex() const;

    void doDrift();
    void doMerge();
    void doChoke();

    TSmoke();
    TSmoke(const TSmoke &a);
    TSmoke & operator=(const TSmoke &a);
    virtual ~TSmoke();
};



#endif
