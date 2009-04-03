//////////////////////////////////////////////////////////////////////////
//
// SneezyMUD - All rights reserved, SneezyMUD Coding Team
//
//
//////////////////////////////////////////////////////////////////////////

#ifndef __OBJ_MAGIC_ITEM_H
#define __OBJ_MAGIC_ITEM_H

#include "obj.h"


class TMagicItem : public virtual TObj
{
  private:
    int magic_level;
    int magic_learnedness;
  public:
    virtual void assignFourValues(int, int, int, int);
    virtual void getFourValues(int *, int *, int *, int *) const;
    virtual sstring statObjInfo() const = 0;

    virtual void changeObjValue1(TBeing *);
    virtual sstring displayFourValues();
    virtual void changeMagicItemValue1(TBeing *, const char *, editorEnterTypeT);
    virtual void evaluateMe(TBeing *) const;
    virtual void divinateMe(TBeing *) const = 0;
    virtual int objectSell(TBeing *, TMonster *);

    virtual void descMagicSpells(TBeing *) const = 0;
    virtual sstring getNameForShow(bool, bool, const TBeing *) const = 0;
    virtual int suggestedPrice() const = 0;
    virtual void objMenu(const TBeing *) const;
    virtual void lowCheck();

    void setMagicLevel(int num);
    int getMagicLevel() const;
    void setMagicLearnedness(int num);
    int getMagicLearnedness() const;

  protected:
    TMagicItem();
  public:
    TMagicItem(const TMagicItem &a);
    TMagicItem & operator=(const TMagicItem &a);
    virtual ~TMagicItem();
};

#endif
