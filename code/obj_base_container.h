//////////////////////////////////////////////////////////////////////////
//
// SneezyMUD - All rights reserved, SneezyMUD Coding Team
//
//
//////////////////////////////////////////////////////////////////////////

#ifndef __OBJ_BASE_CONTAINER_H
#define __OBJ_BASE_CONTAINER_H

#include "obj.h"

// Things can be in a base container (but people can't put things into it)
class TBaseContainer : public TObj {
  private:
  public:
    virtual void assignFourValues(int, int, int, int) = 0;
    virtual void getFourValues(int *, int *, int *, int *) const = 0;
    virtual sstring statObjInfo() const = 0;

    virtual int stealModifier();
    virtual void lookObj(TBeing *, int) const = 0;
    virtual void examineObj(TBeing *) const;
    virtual void logMe(const TBeing *, const char *) const;
    virtual int getAllFrom(TBeing *, const char *);
    virtual int getObjFrom(TBeing *, const char *, const char *);
    virtual int putSomethingInto(TBeing *, TThing *) = 0;
    virtual int putSomethingIntoContainer(TBeing *, TOpenContainer *);
    virtual void findSomeDrink(TDrinkCon **, TBaseContainer **, TBaseContainer *);
    virtual void findSomeFood(TFood **, TBaseContainer **, TBaseContainer *);
    virtual bool engraveMe(TBeing *, TMonster *, bool);
    virtual int getReducedVolume(const TThing *) const;
    virtual void powerstoneCheck(TOpal **);
    virtual void powerstoneCheckCharged(TOpal **);
    virtual void powerstoneMostMana(int *);
    virtual bool fitsSellType(tObjectManipT, TBeing *, TMonster *, sstring, itemTypeT, int &, int);
    virtual sstring showModifier(showModeT, const TBeing *) const;
    virtual void purchaseMe(TBeing *, TMonster *, int, int);

    int isSaddle() const;

  protected:
    TBaseContainer();
  public:
    TBaseContainer(const TBaseContainer &a);
    TBaseContainer & operator=(const TBaseContainer &a);
    virtual ~TBaseContainer();
};

#endif
