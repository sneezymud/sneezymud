#ifndef __OBJ_TRASH_PILE_H
#define __OBJ_TRASH_PILE_H

#include "obj_expandable_container.h"

class TTrashPile : public TExpandableContainer {
  private:
  public:
    void updateDesc();
    void overFlow();
    void attractVermin();
    int getSizeIndex();
    void doMerge();
    void doDecay();
    virtual void assignFourValues(int, int, int, int);
    virtual void getFourValues(int *, int *, int *, int *) const;
    virtual sstring statObjInfo() const;
    virtual itemTypeT itemType() const { return ITEM_TRASH_PILE; }
    
    virtual bool objectRepair(TBeing *, TMonster *, silentTypeT);
    
    TTrashPile();
    TTrashPile(const TTrashPile &a);
    TTrashPile & operator=(const TTrashPile &a);
    virtual ~TTrashPile();
};


#endif
