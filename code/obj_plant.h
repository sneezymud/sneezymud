//////////////////////////////////////////////////////////////////////////
//
// SneezyMUD - All rights reserved, SneezyMUD Coding Team
//
//
//////////////////////////////////////////////////////////////////////////

#ifndef __OBJ_PLANT_H
#define __OBJ_PLANT_H

#include "obj_expandable_container.h"


extern int seed_to_plant(int vnum);

class TPlant : public TExpandableContainer {
  private:
  int planttype, plantage;

  public:
    virtual void assignFourValues(int, int, int, int);
    virtual void getFourValues(int *, int *, int *, int *) const;
    virtual sstring statObjInfo() const;
    virtual itemTypeT itemType() const { return ITEM_PLANT; }
    virtual void updateDesc();
    virtual void updateAge();
    virtual int putSomethingInto(TBeing *, TThing *);

    void setType(int t){ planttype=t; }
    int getType() const { return planttype; }
    void setAge(int t){ plantage=t; }
    int getAge() const { return plantage; }

    void peeOnMe(const TBeing *);

    TPlant();
    TPlant(const TPlant &a);
    TPlant & operator=(const TPlant &a);
    virtual ~TPlant();
};



#endif
