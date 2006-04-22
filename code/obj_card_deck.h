//////////////////////////////////////////////////////////////////////////
//
// SneezyMUD - All rights reserved, SneezyMUD Coding Team
//
//////////////////////////////////////////////////////////////////////////

#ifndef __OBJ_CARD_DECK_H
#define __OBJ_CARD_DECK_H

#include "obj_expandable_container.h"

class TCardDeck : public TExpandableContainer {
  private:
  public:
    virtual void assignFourValues(int, int, int, int);
    virtual void getFourValues(int *, int *, int *, int *) const;
    virtual sstring statObjInfo() const;
    virtual itemTypeT itemType() const { return ITEM_CARD_DECK; }
    virtual void putMoneyInto(TBeing *, int);
    virtual bool objectRepair(TBeing *, TMonster *, silentTypeT);
    virtual void describeObjectSpecifics(const TBeing *) const;
    virtual void lookObj(TBeing *, int) const;
    virtual void getObjFromMeText(TBeing *, TThing *, getTypeT, bool);

    TCardDeck();
    TCardDeck(const TCardDeck &a);
    TCardDeck & operator=(const TCardDeck &a);
    virtual ~TCardDeck();
};



#endif
