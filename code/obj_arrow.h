//////////////////////////////////////////////////////////////////////////
//
// SneezyMUD - All rights reserved, SneezyMUD Coding Team
//
//
//////////////////////////////////////////////////////////////////////////

#ifndef __OBJ_ARROW_H
#define __OBJ_ARROW_H

#include "obj_base_weapon.h"


class TArrow : public TBaseWeapon {
  private:
    unsigned char arrowType;
    unsigned char arrowHead;
    unsigned int  arrowHeadMat;
    unsigned int  arrowFlags;
    unsigned int trap_level;
    doorTrapT trap_dam_type;
  public:
    virtual void assignFourValues(int, int, int, int);
    virtual void getFourValues(int *, int *, int *, int *) const;
    virtual sstring statObjInfo() const;
    virtual itemTypeT itemType() const { return ITEM_ARROW; }
    virtual int suggestedPrice() const;

    int getTrapLevel() const;
    void setTrapLevel(int r);
    doorTrapT getTrapDamType() const;
    void setTrapDamType(doorTrapT r);
    int getTrapDamAmount() const;


    unsigned char getArrowType() const;
    void setArrowType(unsigned int);
    unsigned char getArrowHead() const;
    void setArrowHead(unsigned char);
    int hasFlag(const int) const;
    unsigned char getArrowHeadMat() const;
    void setArrowHeadMat(unsigned char);
    unsigned short getArrowFlags() const;
    void setArrowFlags(unsigned short);
    void remArrowFlags(unsigned short);
    bool isArrowFlag(unsigned short);
    void addArrowFlags(unsigned short);

    virtual spellNumT getWtype(int which=-1) const;
    virtual void evaluateMe(TBeing *) const;
    virtual bool engraveMe(TBeing *, TMonster *, bool);
    virtual void bloadBowArrow(TBeing *, TThing *);
    virtual int throwMe(TBeing *, dirTypeT, const char *);
    virtual int putMeInto(TBeing *, TOpenContainer *);
    virtual sstring compareMeAgainst(TBeing *, TObj *);
    virtual void changeObjValue3(TBeing *);
    virtual void changeObjValue4(TBeing *);
    virtual sstring displayFourValues();
    virtual bool sellMeCheck(TBeing *, TMonster *, int) const;

    TArrow();
    TArrow(const TArrow &a);
    TArrow & operator=(const TArrow &a);
    virtual ~TArrow();
};


#endif
