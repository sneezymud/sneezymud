//////////////////////////////////////////////////////////////////////////
//
// SneezyMUD - All rights reserved, SneezyMUD Coding Team
//
//
//////////////////////////////////////////////////////////////////////////

#ifndef __OBJ_TOOL_H
#define __OBJ_TOOL_H

#include "obj.h"


class TTool : public TObj {
  private:
    toolTypeT tool_type;
    int tool_uses;
    int max_tool_uses;
  public:
    virtual void assignFourValues(int, int, int, int);
    virtual void getFourValues(int *, int *, int *, int *) const;
    virtual sstring statObjInfo() const;
    virtual itemTypeT itemType() const { return ITEM_TOOL; }

    virtual int objectSell(TBeing *, TMonster *);
    virtual void sharpenMeStone(TBeing *, TThing *);
    virtual void dullMeFile(TBeing *, TThing *);
    virtual int garotteMe(TBeing *, TBeing *);
    virtual void sstringMeString(TBeing *, TBow *);
    virtual void skinMe(TBeing *, const char *);
    virtual void butcherMe(TBeing *, const char *);
    virtual void sacrificeMe(TBeing *, const char *);
    virtual int pickWithMe(TBeing *, const char *, const char *, const char *);
    virtual void repairMeHammer(TBeing *, TObj *);
    virtual int garottePulse(TBeing *, affectedData *);
    virtual void unequipMe(TBeing *);
    virtual void describeCondition(const TBeing *) const;
    virtual void describeObjectSpecifics(const TBeing *) const;
    virtual void findBlacksmithingTools(TTool **, TTool**);
    virtual void blacksmithingPulse(TBeing *, TObj *);
    virtual void pickPulse(TBeing *);
    virtual int  skinPulse(TBeing *, TBaseCorpse *);
    virtual int  butcherPulse(TBeing *, TBaseCorpse *);
    virtual void sharpenPulse(TBeing *, TThing *);
    virtual void dullPulse(TBeing *, TThing *);

    toolTypeT getToolType() const;
    void setToolType(toolTypeT r);
    int getToolUses() const;
    void setToolUses(int r);
    void addToToolUses(int r);
    int getToolMaxUses() const;
    void setToolMaxUses(int r);
    void addToToolMaxUses(int r);

    TTool();
    TTool(const TTool &a);
    TTool & operator=(const TTool &a);
    virtual ~TTool();
};



#endif
