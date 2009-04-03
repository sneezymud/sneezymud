//////////////////////////////////////////////////////////////////////////
//
// SneezyMUD - All rights reserved, SneezyMUD Coding Team
//
//
//////////////////////////////////////////////////////////////////////////

#ifndef __OBJ_TRAP_H
#define __OBJ_TRAP_H

#include "obj.h"

class TTrap : public TObj {
  private:
    int trap_level;
    int trap_effect;
    doorTrapT trap_dam_type;
    int trap_charges;
  public:
    virtual void assignFourValues(int, int, int, int);
    virtual void getFourValues(int *, int *, int *, int *) const;
    virtual itemTypeT itemType() const { return ITEM_TRAP; }
    virtual sstring statObjInfo() const;

    virtual void changeObjValue2(TBeing *);
    virtual void changeObjValue3(TBeing *);
    virtual void changeTrapValue2(TBeing *, const char *, editorEnterTypeT);
    virtual void changeTrapValue3(TBeing *, const char *, editorEnterTypeT);
    virtual int moveTrapCheck(TBeing *, dirTypeT);
    virtual int insideTrapCheck(TBeing *, TThing *);
    virtual int anyTrapCheck(TBeing *);
    virtual int getTrapCheck(TBeing *);
    virtual int disarmMe(TBeing *);
    virtual int detectMe(TBeing *) const;
    virtual void evaluateMe(TBeing *) const;

    virtual void listMe(TBeing *, unsigned int) const;
    virtual void listMeExcessive(TBeing *) const;

    virtual int getMe(TBeing *, TThing *);
    virtual void dropMe(TBeing *, showMeT, showRoomT);
    virtual int throwMe(TBeing *, dirTypeT, const char *);
    virtual int detonateGrenade();
    virtual void makeTrapLand(TBeing *, doorTrapT, const char *);
    virtual void makeTrapGrenade(TBeing *, doorTrapT, const char *);
    virtual bool canDrop() const;

    void armGrenade(TBeing *);
    int getTrapLevel() const;
    void setTrapLevel(int r);
    int getTrapEffectType() const;
    void setTrapEffectType(int r);
    bool isTrapEffectType(unsigned int r);
    void remTrapEffectType(unsigned int r);
    void addTrapEffectType(unsigned int r);
    doorTrapT getTrapDamType() const;
    void setTrapDamType(doorTrapT r);
    int getTrapCharges() const;
    void setTrapCharges(int r);
    int getTrapDamAmount() const;

    TTrap();
    TTrap(const TTrap &a);
    TTrap & operator=(const TTrap &a);
    virtual ~TTrap();
};

#endif
