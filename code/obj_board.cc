//////////////////////////////////////////////////////////////////////////
//
// SneezyMUD - All rights reserved, SneezyMUD Coding Team
//
// $Log: obj_board.cc,v $
// Revision 5.1  1999/10/16 04:31:17  batopr
// new branch
//
// Revision 1.1  1999/09/12 17:24:04  sneezy
// Initial revision
//
//
//////////////////////////////////////////////////////////////////////////


#include "stdsneezy.h"
#include "board.h"

#define POST_IN_REVERSE 0

boardStruct *board_list;

void InitBoards()
{
  board_list = NULL;
}

void DeleteABoard(TObj *obj)
{
  boardStruct *b, *prev;

  prev = NULL;
  for (b = board_list; b; b = b->next) {
    if (b->Rnum == obj->number) {
      b->num_loaded--;
      if (b->num_loaded <= 0) {
        if (b == board_list) {
          board_list = b->next;
          delete b;
        } else {
          prev->next = b->next;
          delete b;
        }
      }
      break;
    }
    prev = b;
  }
}

void InitABoard(TObj *obj)
{
  boardStruct *n, *tmp;

  if (board_list) {
    for (tmp = board_list; tmp; tmp = tmp->next) {
      if (tmp->Rnum == obj->number) {
        // the board is already loaded, why do anything?
	// board_load_board(tmp);
        tmp->num_loaded++;
	return;
      }
    }
  }
  n = new boardStruct(obj);
  if (!n) {
    perror("InitABoard(new call)");
    exit(0);
  }

  board_load_board(n);

  n->next = board_list;
  board_list = n;
}

void OpenBoardFile(boardStruct *b)
{
  b->file = fopen(b->filename, "r+");

  if (!b->file) {
    // attempt to create one
    b->file = fopen(b->filename, "w+");
  }
  if (!b->file) {
    perror("OpenBoardFile(fopen)");
    vlogf(10, "Fatal, since could not open \"%s\"", b->filename);
    exit(0);
  }
}

void board_save_board(boardStruct *b)
{
  unsigned int ind;
  int len;

  if (!b)
    return;

  // Someone rips off the only message - Russ 
  if (!b->msg_num) {
    // this will just truncate to 0 size
    OpenBoardFile(b);
    fclose(b->file);
    return;
  }

  OpenBoardFile(b);

  fwrite(&b->msg_num, sizeof(unsigned int), 1, b->file);
  for (ind = 0; ind < b->msg_num; ind++) {
    len = strlen(b->head[ind]) + 1;
    fwrite(&len, sizeof(int), 1, b->file);
    fwrite(b->head[ind], sizeof(char), len, b->file);
    len = strlen(b->writer[ind]) + 1;
    fwrite(&len, sizeof(int), 1, b->file);
    fwrite(b->writer[ind], sizeof(char), len, b->file);
    if (!b->msgs[ind]) {
      b->msgs[ind] = mud_str_dup("Generic Message");
    }
    len = strlen(b->msgs[ind]) + 1;
    fwrite(&len, sizeof(int), 1, b->file);
    fwrite(b->msgs[ind], sizeof(char), len, b->file);
  }
  fclose(b->file);
  return;
}

void board_load_board(boardStruct *b)
{
  unsigned int ind;
  int len = 0;
  char buf[MAX_STRING_LENGTH];

  OpenBoardFile(b);

  if (feof(b->file)) {
    // check for empty board
    fclose(b->file);
    return;
  }

  board_reset_board(b);

  fread(&b->msg_num, sizeof(unsigned int), 1, b->file);

  b->num_loaded++;

  if (b->msg_num < 1 || b->msg_num > MAX_MSGS) {
    vlogf(10, "Board-message file corrupt or nonexistent.");
    fclose(b->file);
    return;
  }
  for (ind = 0; ind < b->msg_num; ind++) {
    fread(&len, sizeof(int), 1, b->file);
    fread(buf, sizeof(char), len, b->file);
    b->head[ind] = mud_str_dup(buf);
    if (!b->head[ind]) {
      vlogf(10, "new for board header failed.\n\r");
      board_reset_board(b);
      fclose(b->file);
      return;
    }
    fread(&len, sizeof(int), 1, b->file);
    fread(buf, sizeof(char), len, b->file);
    b->writer[ind] = mud_str_dup(buf);
    if (!b->writer[ind]) {
      vlogf(10, "new for board writer failed.\n\r");
      board_reset_board(b);
      fclose(b->file);
      return;
    }
    fread(&len, sizeof(int), 1, b->file);
    fread(buf, sizeof(char), len, b->file);
    b->msgs[ind] = mud_str_dup(buf);
    if (!b->msgs[ind]) {
      vlogf(10, "new for board msg failed..\n\r");
      board_reset_board(b);
      fclose(b->file);
      return;
    }
  }
  fclose(b->file);
  return;
}

void board_reset_board(boardStruct *b)
{
  unsigned int ind;

  for (ind = 0; ind < MAX_MSGS; ind++) {
    if (ind >= MAX_MSGS) forceCrash("reset board error");
    delete [] b->head[ind];
    delete [] b->msgs[ind];
    delete [] b->writer[ind];
    b->head[ind] = NULL;
    b->msgs[ind] = NULL;
    b->writer[ind] = NULL;
  }
  b->msg_num = 0;
  return;
}

boardStruct *FindBoardInRoom(int room)
{
  TThing *o;
  boardStruct *nb;
  TRoom *rp;

  if (!(rp = real_roomp(room)))
    return NULL;

  for (o = rp->stuff; o; o = o->nextThing) {
    TObj *to = dynamic_cast<TObj *>(o);
    if (to && to->spec == SPEC_BOARD) {
      for (nb = board_list; nb; nb = nb->next) {
        if (nb->Rnum == o->number)
          return (nb);
      }
      vlogf(8, "Uh oh! Board with proc, but not in board list in room %d", room);
      return NULL;
    }
  }
  return NULL;
}

int board(TBeing *ch, cmdTypeT cmd, const char *arg, TObj *me, TObj *)
{
  return me->boardHandler(ch, cmd, arg);
}

int TObj::boardHandler(TBeing *, cmdTypeT cmd, const char *)
{
  if (cmd != CMD_GENERIC_DESTROYED) {
    vlogf(10, "board handler with non-board (%s) cmd=%d", getName(), cmd);
  } else {
    // this msg comes in ~TObj() so we will never be a TBoard when we get it.
    DeleteABoard(this);
  }
  return FALSE;
}

int TBoard::boardHandler(TBeing *ch, cmdTypeT cmd, const char *arg)
{
  boardStruct *nb;

  if (cmd == CMD_GENERIC_CREATED) {
    InitABoard(this);
    return FALSE;
  }

  if (!ch || (cmd >= MAX_CMD_LIST))
    return FALSE;

  if (!(nb = FindBoardInRoom(ch->in_room)))
    return FALSE;

  if (!ch->desc)
    return FALSE;

  switch (cmd) {
    case CMD_LOOK:		
      return (board_show_board(ch, arg, this, nb));
    case CMD_GET:			
      return (get_note_from_board(ch, arg, nb));
    case CMD_READ:	
      return (board_display_msg(ch, arg, this, nb));
    case CMD_POST:
      post_note_on_board(ch, arg, nb);
      return TRUE;
    default:
      return FALSE;
  }
}

int board_display_msg(TBeing *ch, const char *arg, TBoard *me, boardStruct *b)
{
  char numb[MAX_INPUT_LENGTH], buffer[MAX_STRING_LENGTH];
  int msg;
  string sb;

  if (ch->isAffected(AFF_BLIND)) {
    ch->sendTo("You are blind! This board does not support braile.\n\r");
    return FALSE;
  }

  one_argument(arg, numb);
  if (!*numb || !isdigit(*numb))   // "read" or "read mail"
    return FALSE;

  if (strchr(numb, '.'))  // "read 2.mail"
    return FALSE;

  if (!(msg = atoi(numb)))   // "read 0"
    return FALSE;

  if (!b->msg_num) {
    ch->sendTo("The board is empty!\n\r");
    return TRUE;
  }
  if (me->getBoardLevel() > ch->GetMaxLevel()) {
    ch->sendTo("Your eyes are too lowly to look at this board.\n\r");
    return TRUE;
  }
  if (msg < 1 || msg > (int) b->msg_num) {
    ch->sendTo("That message exists only in your imagination...\n\r");
    return TRUE;
  }

  if (ch->desc) {
    if (ch->desc->client) {
      sprintf(buffer, "Message %d : %s\n\r\n\r%s\n\rEnd of message %d.\n\r",
                      msg, b->head[msg - 1], b->msgs[msg - 1], msg);
     
      string sb = buffer;
      processStringForClient(sb);

      ch->desc->clientf("%d", CLIENT_NOTE);
      ch->sendTo(COLOR_BASIC, sb.c_str());
      ch->desc->clientf("%d", CLIENT_NOTE_END);
    } else {
      sprintf(buffer, "Message %d : %s\n\r\n\r", msg, b->head[msg - 1]);
      sb += buffer;
      sb += b->msgs[msg - 1];
      sb += "\n\r";
      sprintf(buffer, "%sEnd of message %d.\n\r",ch->norm(), msg);
      sb += buffer;
      if (ch->desc)
        ch->desc->page_string(sb.c_str(), 0, FALSE);

      return TRUE;
    }
  }
  return TRUE;
}

int board_show_board(TBeing *ch, const char *arg, TBoard *me, boardStruct *b)
{
  char buf[MAX_STRING_LENGTH];
  char boardname[MAX_INPUT_LENGTH];
  char flagsbuf[MAX_INPUT_LENGTH];

  arg = one_argument(arg, boardname);

  if (!*boardname || !isname(boardname, "board bulletin"))
    return FALSE;

  bool reverse = false;
  one_argument(arg, flagsbuf);
  if (*flagsbuf && is_abbrev(flagsbuf, "reverse"))
    reverse = true;

  if (me->getBoardLevel() > ch->GetMaxLevel()) {
    ch->sendTo("Your eyes are too lowly to look at this board.\n\r");
    return TRUE;
  }
  act("$n studies the board.", TRUE, ch, 0, 0, TO_ROOM);

  strcpy(buf, "This is a bulletin board. You can POST notes or READ/GET notes.\n\r");
  strcat(buf, "To view the latest message first, LOOK BOARD REVERSE\n\r");

  if (!b->msg_num)
    strcat(buf, "The board is empty.\n\r");
  else if (b->msg_num == 1)
    sprintf(buf + strlen(buf), "There is 1 message on the board.\n\r");
  else
    sprintf(buf + strlen(buf), "There are %u messages on the board.\n\r",
	    b->msg_num);

  if (!reverse) {
    unsigned int i;
    for (i = 0; i < b->msg_num; i++) {
      // recall that we stored the string into head as "[date] title (name)"
      // parse this in order to colorize
      char date_string[256];
      char head_string[256];
      char name_string[256];
      strcpy(date_string, b->head[i]);
      char *tmp = strchr(date_string, ']');
      if (!tmp) {
        vlogf(9, "Serious error in show_board (1).");
        b->msg_num = i;
        continue;
      }
      *(tmp+1) = '\0';

      strcpy(head_string, &tmp[2]);
      tmp = strrchr(head_string, '(');
      if (!tmp) {
        vlogf(9, "Serious error in show_board (2).");
        b->msg_num = i;
        continue;
      }
      *(tmp-1) = '\0';
      strcpy(name_string, tmp);

      sprintf(buf + strlen(buf), "%s%-2d%s : %s%s %s%s %s%s%s\n\r", 
              ch->cyan(), i + 1, ch->norm(), ch->green(), date_string,
              ch->purple(), head_string, ch->orange(), name_string, ch->norm());
    }
  } else {
    int i;
    for (i = b->msg_num-1; i >= 0; i--) {
      // recall that we stored the string into head as "[date] title (name)"
      // parse this in order to colorize
      char date_string[256];
      char head_string[256];
      char name_string[256];
      strcpy(date_string, b->head[i]);
      char *tmp = strchr(date_string, ']');
      *(tmp+1) = '\0';
      strcpy(head_string, &tmp[2]);
      tmp = strrchr(head_string, '(');
      *(tmp-1) = '\0';
      strcpy(name_string, tmp);

      sprintf(buf + strlen(buf), "%s%-2d%s : %s%s %s%s %s%s%s\n\r", 
              ch->cyan(), i + 1, ch->norm(), ch->green(), date_string,
              ch->purple(), head_string, ch->orange(), name_string, ch->norm());
    }
  }

  if (ch->desc)
    ch->desc->page_string(buf, FALSE);

  return TRUE;
}

void TThing::postMe(TBeing *ch, const char *, boardStruct *)
{
  ch->sendTo("Only notes can be posted to the board.\n\r");
  return;
}

void TNote::postMe(TBeing *ch, const char *arg2, boardStruct *b)
{
  time_t ot;
  char *otmstr;
  char buf[100], arg3[1024];

  // always post messages to beginning of board.
  // shift other messages down
  // remove messages beyond MAX_MSGS

#if 0
  if (b->msg_num > (MAX_MSGS - 2)) {
    ch->sendTo("The board is full.\n\r");
    return;
  }
#endif

  if (!action_description) {
    ch->sendTo("Please don't post blank notes. Thanks.\n\r");
    return;
  }

  // posting begins here
  // first, shift existing messages up
  unsigned int iter;
#if POST_IN_REVERSE
  for (iter = b->msg_num-1; iter < b->msg_num; iter--) {
    // wipe if it's overflow
    if (iter+1 >= MAX_MSGS) {
      delete [] b->head[iter];
      delete [] b->writer[iter];
      delete [] b->msgs[iter];
      b->msg_num--;
    
      continue;
    }

    b->head[iter+1] = b->head[iter];
    b->msgs[iter+1] = b->msgs[iter];
    b->writer[iter+1] = b->writer[iter];
  }
#else
  // post incrementally.
  // at the cutoff, delete first message, shift all others down
  if (b->msg_num >= MAX_MSGS) {
    for (iter = 0; iter < b->msg_num-1; iter++) {
      if (iter == 0) {
        delete [] b->head[iter];
        delete [] b->writer[iter];
        delete [] b->msgs[iter];
        b->msg_num--;
      }
  
      b->head[iter] = b->head[iter+1];
      b->msgs[iter] = b->msgs[iter+1];
      b->writer[iter] = b->writer[iter+1];
    }
  }
#endif

  ot = time(0);
  otmstr = asctime(localtime(&ot));
  *(otmstr + strlen(otmstr) - 1) = '\0';
  sprintf(buf, "%s", otmstr);

  sprintf(arg3, "[%s] %s (%s)", buf, arg2, ch->getName());

#if !POST_IN_REVERSE
  b->head[b->msg_num] = mud_str_dup(arg3);
  if (!b->head[b->msg_num]) {
    vlogf(10, "new for board header failed.\n\r");
    ch->sendTo("The board is screwed up sorry.\n\r");
    return;
  }
  ch->sendTo("You post your note to the board.\n\r");
  act("$n posts a note to the board.", FALSE, ch, 0, 0, TO_ROOM);
  b->writer[b->msg_num] = mud_str_dup(ch->name);

  b->msgs[b->msg_num] = NULL;
  b->msgs[b->msg_num] = mud_str_dup(action_description);
#else
  b->head[0] = mud_str_dup(arg3);
  if (!b->head[0]) {
    vlogf(10, "new for board header failed.\n\r");
    ch->sendTo("The board is screwed up sorry.\n\r");
    return;
  }
  ch->sendTo("You post your note to the board.\n\r");
  act("$n posts a note to the board.", FALSE, ch, 0, 0, TO_ROOM);
  b->writer[0] = mud_str_dup(ch->name);

  b->msgs[0] = mud_str_dup(action_description);
#endif
  b->msg_num++;
  board_save_board(b);

  delete this;
}

void post_note_on_board(TBeing *ch, const char *argument, boardStruct *b)
{
  char arg1[128], arg2[128];
  TThing *note;

  half_chop(argument, arg1, arg2);

  if (!(note = searchLinkedListVis(ch, arg1, ch->stuff))) {
    ch->sendTo("You don't have anything like that!\n\r");
    return;
  }
  if (!*arg2) {
    ch->sendTo("Syntax : post <note> <title>\n\r");
    return;
  }
  note->postMe(ch, arg2, b);
  // note is possibly invalid here
}

int get_note_from_board(TBeing *ch, const char *arg, boardStruct *b)
{
  unsigned int ind;
  int msg;
  char buf[256], numb[MAX_INPUT_LENGTH];
  TObj *note;

  one_argument(arg, numb);

  if (!*numb || !isdigit(*numb))
    return FALSE;

  if (!(msg = atoi(numb)))
    return FALSE;

  if (!b->msg_num) {
    ch->sendTo("The board is empty!\n\r");
    return TRUE;
  }
  if (msg < 1 || msg > (int) b->msg_num) {
    ch->sendTo("That message doesn't exist!\n\r");
    return TRUE;
  }
  ind = msg - 1;

  if (strcmp(b->writer[ind], ch->name)) {
    if (!ch->hasWizPower(POWER_BOARD_POLICE)) {
      ch->sendTo("You didn't write that note!\n\r");
      return TRUE;
    }
  }
  // copy over stuff from note on board, and put it on a new note  
  // and give that note to the player who removed the note - Russ 

  if (!(note = read_object(GENERIC_NOTE, VIRTUAL))) {
    vlogf(10, "Couldn't make a note removed from board!");
    return TRUE;
  }
  note->swapToStrung();
  delete [] note->action_description;
  note->action_description = mud_str_dup(b->msgs[ind]);

  *ch += *note;

  delete [] b->head[ind];
  b->head[ind] = NULL;
  delete [] b->writer[ind];
  b->writer[ind] = NULL;
  delete [] b->msgs[ind];
  b->msgs[ind] = NULL;

  for (; ind < b->msg_num - 1; ind++) {
    b->head[ind] = b->head[ind + 1];
    b->msgs[ind] = b->msgs[ind + 1];
    b->writer[ind] = b->writer[ind + 1];
  }
  b->msg_num--;
  ch->sendTo("Note removed.\n\r");
  sprintf(buf, "$n just took note %d from the board.", msg);
  act(buf, FALSE, ch, 0, 0, TO_ROOM);
  board_save_board(b);

  return TRUE;
}

void TBoard::purgeMe(TBeing *)
{
  // intentional do-nothing
}

void TBoard::assignFourValues(int x1, int, int, int)
{
  setBoardLevel(x1);
}

void TBoard::getFourValues(int *x1, int *x2, int *x3, int *x4) const
{
  *x1 = getBoardLevel();
  *x2 = 0;
  *x3 = 0;
  *x4 = 0;
}

string TBoard::statObjInfo() const
{
  char buf[256];
  sprintf(buf, "Minimum level to view board: %d", getBoardLevel());

  string a(buf);
  return a;
}

int TBoard::getBoardLevel() const
{
  return board_level;
}

void TBoard::setBoardLevel(int n)
{
  board_level = n;
}

TBoard::TBoard() :
  TObj(), 
  board_level(0)
{
}

TBoard::TBoard(const TBoard &a) :
  TObj(a),
  board_level(a.board_level)
{
}

TBoard & TBoard::operator=(const TBoard &a)
{
  if (this == &a) return *this;
  TObj::operator=(a);
  board_level = a.board_level;
  return *this;
}

TBoard::~TBoard()
{
}

boardStruct::boardStruct(TObj *obj) :
  msg_num(0),
  file(NULL),
  Rnum(obj->number),
  num_loaded(0),
  next(NULL)
{
  unsigned int i;
  char buf[256];
  for (i = 0; i < MAX_MSGS; i++) {
    writer[i] = NULL;
    msgs[i] = NULL;
    head[i] = NULL;
  }

  sprintf(buf, "objdata/boards/%d.messages", obj_index[obj->getItemIndex()].virt);
  filename = mud_str_dup(buf);
}

boardStruct::~boardStruct()
{
  unsigned int i;
  for (i = 0; i < MAX_MSGS; i++) {
    delete [] writer[i];
    delete [] msgs[i];
    delete [] head[i];
  }
  delete [] filename;
}
