//////////////////////////////////////////////////////////////////////////
//
// SneezyMUD - All rights reserved, SneezyMUD Coding Team
//
//
//////////////////////////////////////////////////////////////////////////

#ifndef __OBJ_EGG_H
#define __OBJ_EGG_H

#include "obj.h"
#include "obj_food.h"

class TEgg : public TFood {
  private:
    bool touched;
    int incubationTimer;
    int mobVNum;
  public:
    virtual void assignFourValues(int, int, int, int);
    virtual void getFourValues(int *, int *, int *, int *) const;
    virtual sstring displayFourValues();
    virtual sstring statObjInfo() const;
    virtual itemTypeT itemType() const { return ITEM_EGG; }
    virtual void changeObjValue1(TBeing *);

    virtual void lowCheck();
    virtual int chiMe(TBeing *);
    virtual int getMe(TBeing *, TThing *);
    virtual void eatMe(TBeing *);
    virtual void tasteMe(TBeing *);

    void hatch(TRoom *);
    bool getEggTouched() const;
    void setEggTouched(bool r);
    int eggIncubate();
    int getEggTimer() const;
    void setEggTimer(int r);
    int getEggMobVNum() const;
    void setEggMobVNum(int r);

    TEgg();
    TEgg(const TEgg &a);
    TEgg & operator=(const TEgg &a);
    virtual ~TEgg();

};
#endif
