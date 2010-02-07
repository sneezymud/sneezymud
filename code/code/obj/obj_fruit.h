//////////////////////////////////////////////////////////////////////////
//
// SneezyMUD - All rights reserved, SneezyMUD Coding Team
//
//
//////////////////////////////////////////////////////////////////////////

#ifndef __OBJ_FRUIT_H
#define __OBJ_FRUIT_H

#include "obj.h"
#include "obj_food.h"

class TFruit : public TFood {
  private:
    int seedVNum;
  public:
    virtual void assignFourValues(int, int, int, int);
    virtual void getFourValues(int *, int *, int *, int *) const;
    virtual sstring statObjInfo() const;
    virtual itemTypeT itemType() const { return ITEM_FRUIT; }


    virtual void eatMe(TBeing *);
    virtual void tasteMe(TBeing *);
    virtual int objectDecay();

    void createSeeds();

    int getSeedVNum() const;
    void setSeedVNum(int r);

    TFruit();
    TFruit(const TFruit &a);
    TFruit & operator=(const TFruit &a);
    virtual ~TFruit();

};
#endif
