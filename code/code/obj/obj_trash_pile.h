#ifndef __OBJ_TRASH_PILE_H
#define __OBJ_TRASH_PILE_H

#include "obj_expandable_container.h"
#include "obj_mergeable.h"

class TTrashPile : public TExpandableContainer, public TMergeable {
  private:
  public:
    void updateDesc();
    void overFlow();
    void attractVermin();
    int getSizeIndex();
    void doDecay();
    virtual void assignFourValues(int, int, int, int);
    virtual void getFourValues(int *, int *, int *, int *) const;
    virtual sstring statObjInfo() const;
    virtual itemTypeT itemType() const { return ITEM_TRASH_PILE; }
    
    virtual bool objectRepair(TBeing *, TMonster *, silentTypeT);

    virtual bool willMerge(TMergeable *);
    virtual void doMerge(TMergeable *);
    
    TTrashPile();
    TTrashPile(const TTrashPile &a);
    TTrashPile & operator=(const TTrashPile &a);
    virtual ~TTrashPile();
};


#endif
