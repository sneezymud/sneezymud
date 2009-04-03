//////////////////////////////////////////////////////////////////////////
//
// SneezyMUD - All rights reserved, SneezyMUD Coding Team
//
//
//////////////////////////////////////////////////////////////////////////

#ifndef __OBJ_ARMOR_WAND_H
#define __OBJ_ARMOR_WAND_H

#include "obj_armor.h"
#include "obj_wand.h"

class TArmorWand : public virtual TArmor, public virtual TWand
{
  public:
    virtual void assignFourValues(int, int, int, int);
    virtual void getFourValues(int *, int *, int *, int *) const;
    virtual sstring statObjInfo() const;
    virtual itemTypeT itemType() const { return ITEM_ARMOR_WAND; }

    virtual void lowCheck();
    virtual int suggestedPrice() const;
    virtual int rentCost() const;
    virtual void objMenu(const TBeing *) const;
    virtual void evaluateMe(TBeing *) const;
    virtual void generalUseMessage(const TBeing *, unsigned int, const TBeing *, const TObj *) const;
    virtual sstring getNameForShow(bool, bool, const TBeing *) const;

    TArmorWand();
    TArmorWand(const TArmorWand &a);
    TArmorWand & operator=(const TArmorWand &a);
    virtual ~TArmorWand();
};

#endif
