//////////////////////////////////////////////////////////////////////////
//
// SneezyMUD - All rights reserved, SneezyMUD Coding Team
//
//
//////////////////////////////////////////////////////////////////////////

#ifndef __OBJ_TABLE_H
#define __OBJ_TABLE_H

#include "obj.h"

class TTable : public TObj {
  private:
  public:
    virtual void assignFourValues(int, int, int, int);
    virtual void getFourValues(int *, int *, int *, int *) const;
    virtual sstring statObjInfo() const;
    virtual itemTypeT itemType() const { return ITEM_TABLE; }

    virtual void putMoneyInto(TBeing *, int);
    virtual void lowCheck();
    virtual void writeAffects(int, FILE *) const;
    virtual void lookObj(TBeing *, int) const;
    virtual void examineObj(TBeing *) const;
    virtual bool canGetMeDeny(const TBeing *, silentTypeT) const;
    virtual int getAllFrom(TBeing *, const char *);
    virtual int getObjFrom(TBeing *, const char *, const char *);
    virtual int putSomethingInto(TBeing *, TThing *);
    virtual int putSomethingIntoTable(TBeing *, TTable *);
    virtual void getObjFromMeText(TBeing *, TThing *, getTypeT, bool);
    virtual bool isSimilar(const TThing *t) const;

    TTable();
    TTable(const TTable &a);
    TTable & operator=(const TTable &a);
    virtual ~TTable();
};


#endif
