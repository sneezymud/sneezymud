//////////////////////////////////////////////////////////////////////////
//
// SneezyMUD - All rights reserved, SneezyMUD Coding Team
//
//
//////////////////////////////////////////////////////////////////////////


#ifndef __OBJ_BOARD_H
#define __OBJ_BOARD_H

#include "obj.h"

const unsigned int MAX_MSGS = 50; /* Max number of messages.          */
  
class boardStruct {
  public:
  const char *writer[MAX_MSGS];
  const char *msgs[MAX_MSGS];
  const char *head[MAX_MSGS];
  unsigned int msg_num;
  const char *filename;
  FILE *file;  /* file that is opened */
  int Rnum;    /* Real # of object that this board hooks to */
  int num_loaded;
  boardStruct *next;

  private:
    boardStruct() {};  // prevent use
  public:
    boardStruct(TObj *obj);
    ~boardStruct();
};


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

    TBoard();
    TBoard(const TBoard &a);
    TBoard & operator=(const TBoard &a);
    virtual ~TBoard();
};

 
int board_show_board(TBeing *ch, const char *arg, TBoard *, boardStruct *b);
void board_fix_long_desc(boardStruct *b);
int board(TBeing *ch, cmdTypeT cmd, const char *arg, TObj **me, TObj **ob2);
int board_display_msg(TBeing *ch, const char *arg, TBoard *, boardStruct *b);
void board_reset_board(boardStruct *b);
void board_load_board(boardStruct *b);
void post_note_on_board(TBeing *ch, const sstring &arg, boardStruct *b);
boardStruct *FindBoardInRoom(TBeing *, const char *arg);
void OpenBoardFile(boardStruct *b);
void InitABoard(TObj *obj);
void DeleteABoard(TObj *obj);
void InitBoards();
extern int get_note_from_board(TBeing *, const char *, boardStruct *, TBoard *);
extern boardStruct *board_list;


#endif
