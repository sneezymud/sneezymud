//////////////////////////////////////////////////////////////////////////
//
// SneezyMUD - All rights reserved, SneezyMUD Coding Team
//
//
//////////////////////////////////////////////////////////////////////////

#ifndef __OBJ_BED_H
#define __OBJ_BED_H

#include "obj.h"


class TBed : public TObj {
  private:
    int min_pos_use;
    int max_users;
    int max_size;
    int seat_height;
    int regen;
  public:
    virtual void assignFourValues(int, int, int, int);
    virtual void getFourValues(int *, int *, int *, int *) const;
    virtual sstring statObjInfo() const;
    virtual itemTypeT itemType() const { return ITEM_BED; }

    virtual bool isRideable() const { return TRUE; }
    virtual bool canGetMeDeny(const TBeing *, silentTypeT) const;
    virtual void sitMe(TBeing *);
    virtual void restMe(TBeing *);
    virtual void sleepMe(TBeing *);
    virtual int getMaxRiders() const;
    virtual int getRiderHeight() const;
    virtual int mobPulseBed(TMonster *, short int);
    void bedRegen(TBeing *, int *, silentTypeT) const;
    virtual void changeObjValue1(TBeing *);
    virtual void changeBedValue1(TBeing *, const char *, editorEnterTypeT);
    virtual void lowCheck();
    int putSomethingInto(TBeing *, TThing *);
    virtual int getAllFrom(TBeing *, const char *);
    virtual int getObjFrom(TBeing *, const char *, const char *);
    virtual void getObjFromMeText(TBeing *, TThing *, getTypeT, bool);
    virtual bool isSimilar(const TThing *t) const;


    int getMinPosUse() const;
    void setMinPosUse(int n);
    int getMaxUsers() const;
    void setMaxUsers(int n);
    int getMaxSize() const;
    void setMaxSize(int n);
    int getSeatHeight() const;
    void setSeatHeight(int n);
    int getRegen() const;
    void setRegen(int n);

    TBed();
    TBed(const TBed &a);
    TBed & operator=(const TBed &a);
    virtual ~TBed();
};

#endif
