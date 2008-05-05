//////////////////////////////////////////////////////////////////////////
//
// SneezyMUD - All rights reserved, SneezyMUD Coding Team
//
//
//////////////////////////////////////////////////////////////////////////

#ifndef __OBJ_BOW_H
#define __OBJ_BOW_H

#include "obj.h"

class TBow : public TObj {
  private:
    int arrowType;
    unsigned int flags;
    unsigned int max_range;
  public:
    virtual void assignFourValues(int, int, int, int);
    virtual void getFourValues(int *, int *, int *, int *) const;
    virtual sstring statObjInfo() const;
    virtual itemTypeT itemType() const { return ITEM_BOW; }

    virtual sstring showModifier(showModeT, const TBeing *) const;
    virtual void describeObjectSpecifics(const TBeing *) const;
    virtual bool isBluntWeapon() const;
    virtual void sstringMeBow(TBeing *, TThing *);
    virtual void evaluateMe(TBeing *) const;
    virtual int shootMeBow(TBeing *ch, TBeing *, unsigned int, dirTypeT, int);
    virtual void bloadArrowBow(TBeing *, TArrow *);
    virtual bool sellMeCheck(TBeing *, TMonster *, int, int) const;
    virtual sstring compareMeAgainst(TBeing *, TObj *);
    virtual void dropMe(TBeing *, showMeT, showRoomT);

    int getArrowType() const;
    void setArrowType(int);
    unsigned int getBowFlags() const;
    void setBowFlags(unsigned int r);
    bool isBowFlag(unsigned int r) const;
    void addBowFlags(unsigned int r);
    void remBowFlags(unsigned int r);
    unsigned int getMaxRange() const;
    void setMaxRange(unsigned int);

    TBow();
    TBow(const TBow &a);
    TBow & operator=(const TBow &a);
    virtual ~TBow();
};


#endif
