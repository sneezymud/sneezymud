//////////////////////////////////////////////////////////////////////////
//
// SneezyMUD - All rights reserved, SneezyMUD Coding Team
//
//
//////////////////////////////////////////////////////////////////////////

#ifndef __OBJ_OPEN_CONTAINER_H
#define __OBJ_OPEN_CONTAINER_H

#include "obj_base_container.h"

// a base-container that can be opened and closed
// permits things to be put into it by players
// volume remains constant as contents are added
class TOpenContainer : public TBaseContainer {
  private:
    float max_weight;
    int container_flags;
    doorTrapT trap_type;
    char trap_dam;
    int key_num;
    int max_volume;
  public:
    virtual void assignFourValues(int, int, int, int);
    virtual void getFourValues(int *, int *, int *, int *) const;
    virtual sstring statObjInfo() const;

    virtual void changeObjValue2(TBeing *);
    virtual bool getObjFromMeCheck(TBeing *);
    virtual void lookObj(TBeing *, int) const;
    virtual int disarmMe(TBeing *);
    virtual int detectMe(TBeing *) const;
    virtual void pickMe(TBeing *);
    virtual int trapMe(TBeing *, const char *);
    virtual int openMe(TBeing *);
    virtual void closeMe(TBeing *);
    virtual void lockMe(TBeing *);
    virtual void unlockMe(TBeing *);
    virtual void putMoneyInto(TBeing *, int);
    virtual void describeContains(const TBeing *) const;
    virtual void lowCheck();
    virtual int sellCommod(TBeing *, TMonster *, int, TThing *);
    virtual int putSomethingInto(TBeing *, TThing *);
    virtual sstring compareMeAgainst(TBeing *, TObj *);
    virtual void purchaseMe(TBeing *, TMonster *, int, int);
    virtual sstring showModifier(showModeT, const TBeing *) const;

    bool isCloseable() const;
    bool isClosed() const;

    virtual float carryWeightLimit() const;
    void setCarryWeightLimit(float);
    int getContainerFlags() const;
    void setContainerFlags(int r);
    void addContainerFlag(int r);
    void remContainerFlag(int r);
    bool isContainerFlag(int r) const;
    doorTrapT getContainerTrapType() const;
    void setContainerTrapType(doorTrapT r);
    char getContainerTrapDam() const;
    void setContainerTrapDam(char r);
    void setKeyNum(int);
    int getKeyNum() const;
    virtual int carryVolumeLimit() const;
    void setCarryVolumeLimit(int);

  protected:
    TOpenContainer();
  public:
    TOpenContainer(const TOpenContainer &a);
    TOpenContainer & operator=(const TOpenContainer &a);
    virtual ~TOpenContainer();
};


#endif
