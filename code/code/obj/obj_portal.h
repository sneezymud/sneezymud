//////////////////////////////////////////////////////////////////////////
//
// SneezyMUD - All rights reserved, SneezyMUD Coding Team
//
//
//////////////////////////////////////////////////////////////////////////

#ifndef __OBJ_PORTAL_H
#define __OBJ_PORTAL_H

#include "obj_seethru.h"


class TPortal : public TSeeThru {
  private:
    char charges;
    unsigned char portal_type;
    unsigned char trap_type;
    unsigned short trap_damage;
    unsigned short portal_state;
    int portal_key;
  public:
    virtual void assignFourValues(int, int, int, int);
    virtual void getFourValues(int *, int *, int *, int *) const;
    virtual sstring statObjInfo() const;
    virtual itemTypeT itemType() const { return ITEM_PORTAL; }

    virtual int chiMe(TBeing *);
    virtual void changeObjValue1(TBeing *);
    virtual void changeObjValue3(TBeing *);
    virtual void changeObjValue4(TBeing *);
    virtual sstring displayFourValues();
    virtual int openMe(TBeing *);
    virtual void closeMe(TBeing *);
    virtual void lockMe(TBeing *);
    virtual void unlockMe(TBeing *);
    virtual int enterMe(TBeing *);
    virtual int objectDecay();
    virtual void thingDumped(TBeing *, int *) {}
    virtual int detectMe(TBeing *) const;
    virtual void showMe(TBeing *) const;

    char getPortalNumCharges() const;
    void setPortalNumCharges(char r);
    unsigned char getPortalType() const;
    void setPortalType(unsigned char r);
    unsigned short getPortalFlags() const;
    void setPortalFlags(unsigned short s);
    bool isPortalFlag(unsigned short) const;
    void addPortalFlag(unsigned short);
    void remPortalFlag(unsigned short);
    int getPortalKey() const;
    void setPortalKey(int r);
    unsigned char getPortalTrapType() const;
    void setPortalTrapType(unsigned char r);
    unsigned short getPortalTrapDam() const;
    void setPortalTrapDam(unsigned short r);
    TPortal * findMatchingPortal() const;

    TPortal();
    TPortal(const TRoom *);
    TPortal(const TPortal &a);
    TPortal & operator=(const TPortal &a);
    virtual ~TPortal();
};


#endif
