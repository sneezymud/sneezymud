//////////////////////////////////////////////////////////////////////////
//
// SneezyMUD - All rights reserved, SneezyMUD Coding Team
//
//
//////////////////////////////////////////////////////////////////////////

#ifndef __OBJ_VIAL_H
#define __OBJ_VIAL_H

#include "obj_base_cup.h"


class TVial : public TBaseCup {
  public:
    virtual void findVialAttune(TVial **, int *);
    virtual void getBestVial(TVial **);
    virtual int objectSell(TBeing *, TMonster *);
    virtual bool objectRepair(TBeing *, TMonster *, silentTypeT);
    virtual itemTypeT itemType() const { return ITEM_VIAL; }

    virtual int rentCost() const;
    virtual int suggestedPrice() const;
    virtual void lowCheck();

    TVial();
    TVial(const TVial &a);
    TVial & operator=(const TVial &a);
    virtual ~TVial();
};



#endif
