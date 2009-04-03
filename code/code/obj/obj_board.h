//////////////////////////////////////////////////////////////////////////
//
// SneezyMUD - All rights reserved, SneezyMUD Coding Team
//
//
//////////////////////////////////////////////////////////////////////////


#ifndef __OBJ_BOARD_H
#define __OBJ_BOARD_H

#include "obj.h"

const int MAX_MSGS = 100; /* max number of messages per board */

class TBoard : public TObj {
  private:
    int board_level;
  public:
    virtual void assignFourValues(int, int, int, int);
    virtual void getFourValues(int *, int *, int *, int *) const;
    virtual itemTypeT itemType() const { return ITEM_BOARD; }
    virtual sstring statObjInfo() const;

    virtual void purgeMe(TBeing *);
    virtual int boardHandler(TBeing *, cmdTypeT, const char *);

    int getBoardLevel() const;
    void setBoardLevel(int n);
    
    int postCount() const;
    int lookBoard(TBeing *ch, const char *arg);
    int readPost(TBeing *ch, const char *arg);
    int postToBoard(TBeing *ch, const sstring &arg);
    int removeFromBoard(TBeing *, const char *);
    
    TBoard();
    TBoard(const TBoard &a);
    TBoard & operator=(const TBoard &a);
    virtual ~TBoard();
};

int board(TBeing *ch, cmdTypeT cmd, const char *arg, TObj **me, TObj **ob2);

#endif
