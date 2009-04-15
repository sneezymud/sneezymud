//////////////////////////////////////////////////////////////////////////
//
// SneezyMUD - All rights reserved, SneezyMUD Coding Team
//
//////////////////////////////////////////////////////////////////////////

#include "being.h"
#include "room.h"
#include "connect.h"

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
	  if (ch->hasClass(CLASS_SHAMAN) && !ch->affectedBySpell(SPELL_SHAPESHIFT)
        && !ch->isImmortal()) {
	    if (ch->GetMaxLevel() > 5) {
	      if (1 > ch->getLifeforce()) {
		ch->updateHalfTickStuff();
	      } else {
		ch->sendTo("Your lack of activity drains your precious lifeforce.\n\r");
		ch->addToLifeforce(-1);
	      }
	    } else {
	      ch->addToHit(1);
	    }
	  } else {
	    ch->addToHit(1);
	  }
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
