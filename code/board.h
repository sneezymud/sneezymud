//////////////////////////////////////////////////////////////////////////
//
// SneezyMUD - All rights reserved, SneezyMUD Coding Team
//
// $Log: board.h,v $
// Revision 5.1  1999/10/16 04:31:17  batopr
// new branch
//
// Revision 1.1  1999/09/12 17:24:04  sneezy
// Initial revision
//
//
//////////////////////////////////////////////////////////////////////////


#ifndef __BOARD_H
#define __BOARD_H

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
 
int board_show_board(TBeing *ch, const char *arg, TBoard *, boardStruct *b);
void board_fix_long_desc(boardStruct *b);
int board(TBeing *ch, cmdTypeT cmd, const char *arg, TObj **me, TObj **ob2);
int board_display_msg(TBeing *ch, const char *arg, TBoard *, boardStruct *b);
void board_reset_board(boardStruct *b);
void board_load_board(boardStruct *b);
void post_note_on_board(TBeing *ch, const char *arg, boardStruct *b);
boardStruct *FindBoardInRoom(int room);
void OpenBoardFile(boardStruct *b);
void InitABoard(TObj *obj);
void DeleteABoard(TObj *obj);
void InitBoards();
extern int get_note_from_board(TBeing *, const char *, boardStruct *);
extern boardStruct *board_list;

#endif
