//////////////////////////////////////////////////////////////////////////
//
// SneezyMUD - All rights reserved, SneezyMUD Coding Team
//
//
//////////////////////////////////////////////////////////////////////////

#ifndef __OBJ_APPLIED_SUB_H
#define __OBJ_APPLIED_SUB_H

#include "obj.h"


class TASubstance : public TObj {
  private:
    int AFlags,
        AMethod,
        ILevel,
        ISpell;
  public:
    virtual void assignFourValues(int, int, int, int);
    virtual void getFourValues(int *, int *, int *, int *) const;
    virtual sstring statObjInfo() const;
    virtual itemTypeT itemType() const {return ITEM_APPLIED_SUB; }

    virtual void describeObjectSpecifics(const TBeing *) const;
    virtual int  applyMe(TBeing *, TThing *);
    void setAFlags(int);
    int getAFlags() const;
    void setAMethod(int);
    int getAMethod() const;
    void setILevel(int);
    int getILevel() const;
    void setISpell(int);
    int getISpell() const;

    TASubstance();
    TASubstance(const TASubstance &a);
    TASubstance & operator=(const TASubstance &a);
    virtual ~TASubstance();
};



#endif
