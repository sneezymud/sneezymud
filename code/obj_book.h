//////////////////////////////////////////////////////////////////////////
//
// SneezyMUD - All rights reserved, SneezyMUD Coding Team
//
//
//////////////////////////////////////////////////////////////////////////

#ifndef __OBJ_BOOK_H
#define __OBJ_BOOK_H

#include "obj.h"


class TBook : public TObj {
  private:
  public:
    virtual void assignFourValues(int, int, int, int);
    virtual void getFourValues(int *, int *, int *, int *) const;
    virtual sstring statObjInfo() const;
    virtual itemTypeT itemType() const { return ITEM_BOOK; }
    virtual void lookAtObj(TBeing *, const char *, showModeT) const;

    TBook();
    TBook(const TBook &a);
    TBook & operator=(const TBook &a);
    virtual ~TBook();
};


#endif
