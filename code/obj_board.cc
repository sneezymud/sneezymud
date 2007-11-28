/////////////////////////////////////////////////////////////////////////
//
// SneezyMUD - All rights reserved, SneezyMUD Coding Team
//
//////////////////////////////////////////////////////////////////////////


#include "stdsneezy.h"
#include "obj_board.h"
#include "obj_note.h"
#include "database.h"

int board(TBeing *ch, cmdTypeT cmd, const char *arg, TObj *me, TObj *)
{
  // spec object just launches this handler
  return me->boardHandler(ch, cmd, arg);
}

int TObj::boardHandler(TBeing *, cmdTypeT cmd, const char *)
{
  if (cmd != CMD_GENERIC_DESTROYED){
    vlogf(LOG_PROC, fmt("board handler with non-board (%s) cmd=%d") %  getName() % cmd);
  } else {
    // this msg comes in ~TObj() so we will never be a TBoard when we get it.
    // vlogf(LOG_PROC, fmt("Um, deleted a board?"));
  }
  return FALSE;
}

int TBoard::boardHandler(TBeing *ch, cmdTypeT cmd, const char *arg)
{
  if (cmd == CMD_GENERIC_CREATED)
    return FALSE;

  if (!ch || (cmd >= MAX_CMD_LIST))
    return FALSE;

  if (!ch->desc)
    return FALSE;
  
  switch (cmd){
    case CMD_LOOK:
      return lookBoard(ch, arg);
    case CMD_READ:
      return readPost(ch, arg);
    case CMD_POST:
      return postToBoard(ch, arg);
    case CMD_GET:
      return removeFromBoard(ch, arg);
    default:
      return FALSE;
  }
}

int TBoard::readPost(TBeing *ch, const char *arg)
{
  char numb[MAX_INPUT_LENGTH], buffer[MAX_STRING_LENGTH];
  int post_num;
  sstring sb;

  if (ch->isAffected(AFF_BLIND)){
    ch->sendTo("You are blind! This board does not support braille.\n\r");
    return TRUE;
  }

  one_argument(arg, numb, cElements(numb));
  
  // "read" or "read mail"
  if (!*numb || !isdigit(*numb))
    return FALSE;
  
  // "read 2.mail"
  if (strchr(numb, '.'))
    return FALSE;
  
  // "read 0"
  if (!(post_num = convertTo<int>(numb)))
    return FALSE;
  
  if (!ch->isImmortal()){
    if (objVnum() == FACT_BOARD_BROTHER && ch->getFaction() != FACT_BROTHERHOOD){
      ch->sendTo("This board is for the Brotherhood of Galek only.\n\r");
      return TRUE;
    }
    if (objVnum() == FACT_BOARD_SERPENT && ch->getFaction() != FACT_SNAKE){
      ch->sendTo("This board is for the Order of Serpents only.\n\r");
      return TRUE;
    }
    if (objVnum() == FACT_BOARD_LOGRUS && ch->getFaction() != FACT_CULT){
      ch->sendTo("This board is for the Cult of Logrus only.\n\r");
      return TRUE;
    }
  }

  if (getBoardLevel() > ch->GetMaxLevel()){
    act("You are too lowly to look at $p.", TRUE, ch, this, 0, TO_CHAR, NULL);
    return TRUE;
  }
  
  if (post_num < 1){
    ch->sendTo("That message exists only in your imagination...\n\r");
    return TRUE;
  }

  TDatabase db(DB_SNEEZY);
  db.query("select post_num, date_format(date_posted, '%%b %%e %%H:%%i %%Y') as date_posted, subject, author, post from board_message where board_vnum = %i and date_removed is null and post_num = %i", objVnum(), post_num);
  
  if (db.fetchRow()){
     if (ch->desc){
       if (ch->desc->m_bIsClient){
         sprintf(buffer, "Message %d : [%s] %s (%s)\n\r\n\r%s\n\rEnd of message %d.\n\r", post_num, mud_str_dup(db["date_posted"]), mud_str_dup(db["subject"]), mud_str_dup(db["author"]), mud_str_dup(db["post"]), post_num);
        
         sstring sb = buffer;
         processStringForClient(sb);
   
         ch->desc->clientf(fmt("%d") % CLIENT_NOTE);
         ch->sendTo(COLOR_BASIC, sb);
         ch->desc->clientf(fmt("%d") % CLIENT_NOTE_END);
       } else {
         sprintf(buffer, "Message %d : [%s] %s (%s)\n\r\n\r%s\n\r%sEnd of message %d.\n\r", post_num, mud_str_dup(db["date_posted"]), mud_str_dup(db["subject"]), mud_str_dup(db["author"]), mud_str_dup(db["post"]), ch->norm(), post_num);
         sstring sb = buffer;
         ch->desc->page_string(sb);
       }
       return TRUE;
     }
  } else {
    ch->sendTo("That message exists only in your imagination...\n\r");
    return TRUE;
  }
  return FALSE;
}

int TBoard::lookBoard(TBeing *ch, const char *arg)
{
  sstring sbuf1, sbuf2;
  char flagsbuf[MAX_INPUT_LENGTH], boardname[MAX_INPUT_LENGTH];
  bool reverse = false;
  
  arg = one_argument(arg, boardname, cElements(boardname));
  
  if (!*boardname || !isname(boardname, this->name)) 
    return FALSE;

  one_argument(arg, flagsbuf, cElements(flagsbuf));
  
  if (*flagsbuf && is_abbrev(flagsbuf, "reverse"))
    reverse = true;
  
  if (!ch->isImmortal()){
    if (this->objVnum() == FACT_BOARD_BROTHER && ch->getFaction() != FACT_BROTHERHOOD){
      ch->sendTo("This board is for the Brotherhood of Galek only.\n\r");
      return TRUE;
    }
    if (this->objVnum() == FACT_BOARD_SERPENT && ch->getFaction() != FACT_SNAKE){
      ch->sendTo("This board is for the Order of Serpents only.\n\r");
      return TRUE;
    }
    if (this->objVnum() == FACT_BOARD_LOGRUS && ch->getFaction() != FACT_CULT){
      ch->sendTo("This board is for the Cult of Logrus only.\n\r");
      return TRUE;
    }
  }

  if (getBoardLevel() > ch->GetMaxLevel()) {
    act("You are too lowly to look at $p.", TRUE, ch, this, 0, TO_CHAR, NULL);
    return TRUE;
  }

  TDatabase db(DB_SNEEZY);
  db.query("select post_num, date_format(date_posted, '%%b %%e %%H:%%i %%Y') as date_posted, subject, author from board_message where board_vnum = %i and date_removed is null order by post_num %s", this->objVnum(), reverse ? "desc" : "asc");
  act("$n studies $p.", TRUE, ch, this, 0, TO_ROOM);
  sbuf1 = fmt("This is a bulletin board. You can %sPOST%s, %sREAD <#>%s or %sGET <#>%s.\n\r") % ch->green() % ch->norm() % ch->green() % ch->norm() % ch->green() % ch->norm();
  sbuf1 += fmt("To view the latest message first, %sLOOK <BOARD> REVERSE%s.\n\r\n\r") % ch->green() % ch->norm();
  
  ch->desc->page_string(sbuf1);
  
  sbuf1 = "";
  int num = 0;
  while (db.fetchRow()){
    sbuf1 += fmt("%s%-2d%s : %s[%s] %s%s %s(%s)%s\n\r") %
	ch->cyan() % convertTo<int>(db["post_num"]) % ch->norm() % 
        ch->green() % db["date_posted"] %
	ch->purple() % db["subject"] % 
        ch->orange() % db["author"] % ch->norm();
    num++;
  }
  
  if (num == 0){
    act("$p is empty.", TRUE, ch, this, 0, TO_CHAR);
  } else if (num == 1){
    act("There is 1 message on $p.\n\r", TRUE, ch, this, 0, TO_CHAR);
    ch->desc->page_string(sbuf1);
  } else {
    sbuf2 = fmt("There are %i messages on $p.\n\r") % num;
    act(sbuf2, TRUE, ch, this, 0, TO_CHAR);
    ch->desc->page_string(sbuf1);
  }

  return TRUE;
}

void TThing::postMe(TBeing *ch, const char *, TBoard *)
{
  ch->sendTo("Only notes can be posted to the board.\n\r");
  return;
}

void TNote::postMe(TBeing *ch, const char *arg2, TBoard *b)
{
  if (!action_description) {
    ch->sendTo("Blank notes cannot be posted to the board.\n\r");
    return;
  }
  
  TDatabase db(DB_SNEEZY);
  db.query("insert board_message (board_vnum, subject, author, post, post_num) select %i, '%s', '%s', '%s', count(*) + 1 from board_message where board_vnum = %i and date_removed is null", b->objVnum(), arg2, ch->getName(), mud_str_dup(action_description), b->objVnum());
  act("You post your note on $p.", TRUE, ch, b, 0, TO_CHAR, NULL);
  act("$n posts a note on $p.", TRUE, ch, b, 0, TO_ROOM);
  delete this;
}

int TBoard::postCount() const
{
  TDatabase db(DB_SNEEZY);
  db.query("select count(*) as post_count from board_message where board_vnum = %i and date_removed is null", this->objVnum());
  if (db.fetchRow()){
    return convertTo<int>(db["post_count"]);
  } else {
    return 0;
  }
}

int TBoard::postToBoard(TBeing *ch, const sstring &argument)
{
  sstring arg1, arg2;
  TThing *note;

  arg2 = one_argument(argument, arg1);
  
  if (arg2.empty() || arg1.empty()) {
    ch->sendTo("Syntax : post <note> <subject>\n\r");
    return TRUE;
  }
  
  if(arg2.word(0) == "board"){
    ch->sendTo("You may not post with the subject of 'board'.  Please use another subject.\n\r");
    return TRUE;
  }

  if (!(note = searchLinkedListVis(ch, arg1, ch->getStuff()))){
    ch->sendTo("You don't have a note to post!\n\r");
    return TRUE;
  }
  
  // uncomment to put the cap on the amount of notes one board can hold 
  /*if (postCount() >= MAX_MSGS) {
    ch->sendTo("There is no room on the board for a new note.\n\r");
    return TRUE;
  }*/

  if (!ch->isImmortal()){
    if (this->objVnum() == FACT_BOARD_BROTHER && ch->getFaction() != FACT_BROTHERHOOD){
      ch->sendTo("This board is for the Brotherhood of Galek only.\n\r");
      return TRUE;
    }
    if (this->objVnum() == FACT_BOARD_SERPENT && ch->getFaction() != FACT_SNAKE){
      ch->sendTo("This board is for the Order of Serpents only.\n\r");
      return TRUE;
    }
    if (this->objVnum() == FACT_BOARD_LOGRUS && ch->getFaction() != FACT_CULT){
      ch->sendTo("This board is for the Cult of Logrus only.\n\r");
      return TRUE;
    }
  }

  if (getBoardLevel() > ch->GetMaxLevel()) {
    act("You are too lowly to use $p.", TRUE, ch, this, 0, TO_CHAR, NULL);
    return TRUE;
  }
  
  note->postMe(ch, arg2.c_str(), this);
  return TRUE;
}

int TBoard::removeFromBoard(TBeing *ch, const char *arg)
{
  int post_num;
  char numb[MAX_INPUT_LENGTH];

  one_argument(arg, numb, cElements(numb));

  if (!*numb || !isdigit(*numb))
    return FALSE;
  
  if (strchr(numb, '.'))
    return FALSE;

  if (!(post_num = convertTo<int>(numb)))
    return FALSE;
  
  if (!ch->isImmortal() && !ch->hasWizPower(POWER_BOARD_POLICE)){
    if (this->objVnum() == FACT_BOARD_BROTHER && ch->getFaction() != FACT_BROTHERHOOD){
      ch->sendTo("This board is for the Brotherhood of Galek only.\n\r");
      return TRUE;
    }
    if (this->objVnum() == FACT_BOARD_SERPENT && ch->getFaction() != FACT_SNAKE){
      ch->sendTo("This board is for the Order of Serpents only.\n\r");
      return TRUE;
    }
    if (this->objVnum() == FACT_BOARD_LOGRUS && ch->getFaction() != FACT_CULT){
      ch->sendTo("This board is for the Cult of Logrus only.\n\r");
      return TRUE;
    }
  }

  if (getBoardLevel() > ch->GetMaxLevel()) {
    act("You are too lowly to use $p.", TRUE, ch, this, 0, TO_CHAR, NULL);
    return TRUE;
  }
  
  if (post_num < 1) {
    ch->sendTo("That message doesn't exist!\n\r");
    return TRUE;
  }

  if (!postCount()) {
    act("$p is empty!", TRUE, ch, this, 0, TO_CHAR, NULL);
    return TRUE;
  }
  
  TDatabase db(DB_SNEEZY);
  db.query("select author, post from board_message where board_vnum = %i and post_num = %i and date_removed is null", objVnum(), post_num);
  if (db.fetchRow()){
    // only the original author, a board police wiz or a faction power wanker at a faction board can remove a note
    if (strcmp(mud_str_dup(db["author"]), ch->getName()) && 
      !ch->hasWizPower(POWER_BOARD_POLICE) && 
      !(objVnum() == FACT_BOARD_SERPENT && ch->getFactionAuthority(FACT_SNAKE, 0)) && 
      !(objVnum() == FACT_BOARD_BROTHER && ch->getFactionAuthority(FACT_BROTHERHOOD, 0)) && 
      !(objVnum() == FACT_BOARD_LOGRUS && ch->getFactionAuthority(FACT_CULT, 0))){
      ch->sendTo("You didn't write that note!\n\r");
      return TRUE;
    } else {
      // create the note and give to ch
      TNote *note = createNote(mud_str_dup(db["post"]));
      *ch += *note;
      ch->sendTo("You get the note.\n\r");
      act("$n pulls a note off $p.", FALSE, ch, this, 0, TO_ROOM);
      
      // update board_message - remove message and adjust remaining post_nums
      TDatabase db(DB_SNEEZY);
      db.query("update board_message set date_removed = now(), post_num = null where board_vnum = %i and post_num = %i and date_removed is null", objVnum(), post_num);
      db.query("update board_message set post_num = post_num - 1 where board_vnum = %i and post_num > %i and date_removed is null", objVnum(), post_num);
      return TRUE;
    }
  } else {
    ch->sendTo("That message doesn't exist!\n\r");
    return TRUE;
  }
  return FALSE;
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

sstring TBoard::statObjInfo() const
{
  char buf[256];
  sprintf(buf, "Minimum level to view board: %d", getBoardLevel());

  sstring a(buf);
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

