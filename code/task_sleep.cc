//////////////////////////////////////////////////////////////////////////
//
// SneezyMUD - All rights reserved, SneezyMUD Coding Team
//
// $Log: task_sleep.cc,v $
// Revision 5.2  2001/06/03 07:58:14  jesus
// temporary fix to an annoying -hp bug with shaman
//
// Revision 5.1.1.2  2001/04/01 07:03:11  jesus
// shaman regen
//
// Revision 5.1.1.1  1999/10/16 04:32:20  batopr
// new branch
//
// Revision 5.1  1999/10/16 04:31:17  batopr
// new branch
//
// Revision 1.1  1999/09/12 17:24:04  sneezy
// Initial revision
//
//
//////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////
//
//      SneezyMUD 5.1 - All rights reserved, SneezyMUD Coding Team
//      "task_sleep.cc" - All functions related to tasks that keep mobs/PCs busy
//
//////////////////////////////////////////////////////////////////////////

#include "stdsneezy.h"

int task_sleep(TBeing *ch, cmdTypeT cmd, const char *arg, int pulse, TRoom *, TObj *)
{
  if (ch->isLinkdead() || (ch->getPosition() != POSITION_SLEEPING)) {
    ch->stopTask();
    return FALSE;
  }
  if (ch->utilityTaskCommand(cmd))
    return FALSE;

  int regentime = ch->regenTime();
  switch(cmd) {
  case CMD_TASK_CONTINUE:
      ch->task->calcNextUpdate(pulse, regentime);
      if (!ch->task->status) {
        if (!ch->roomp->isRoomFlag(ROOM_NO_HEAL)) {
          ch->addToMana(1);
          ch->addToLifeforce(-2);
          ch->addToHit(1);

          if (ch->getMove() < ch->moveLimit())
            ch->addToMove(1);

          if (ch->ansi()) {
            ch->desc->updateScreenAnsi(CHANGED_HP);
            ch->desc->updateScreenAnsi(CHANGED_MANA);
            ch->desc->updateScreenAnsi(CHANGED_LIFEFORCE);
            ch->desc->updateScreenAnsi(CHANGED_MOVE);
          } else if (ch->vt100()) {
            ch->desc->updateScreenVt100(CHANGED_HP);
            ch->desc->updateScreenVt100(CHANGED_MANA);
            ch->desc->updateScreenVt100(CHANGED_LIFEFORCE);
            ch->desc->updateScreenVt100(CHANGED_MOVE);
          }
        }
      }
      ch->updatePos();
      ch->task->status = 0;
      break;
  case CMD_ABORT:
  case CMD_STOP:
  case CMD_STAND:
  case CMD_WAKE:
      ch->stopTask();
      ch->doWake(arg);
      break;
  case CMD_SLEEP:
      ch->sendTo("You start to dream about sleeping.\n\r");
      break;
  case CMD_TASK_FIGHTING:
      ch->sendTo("You are unable to sleep while under attack!\n\r");
      ch->cantHit += ch->loseRound(1);
      if (!::number(0,1))
        ch->cantHit += ch->loseRound(1);
      ch->stopTask();
      break;
  default:
      if (cmd < MAX_CMD_LIST)
        return FALSE;           // process command
      break;                    // eat the command
  }
  return TRUE;
}
