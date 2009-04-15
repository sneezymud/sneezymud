//////////////////////////////////////////////////////////////////////////
//
//      SneezyMUD 4.0 - All rights reserved, SneezyMUD Coding Team
//      "task.cc" - All functions related to tasks that keep mobs/PCs busy
//
//////////////////////////////////////////////////////////////////////////

#include "being.h"
#include "obj.h"
#include "room.h"
#include "connect.h"

int task_meditate(TBeing *ch, cmdTypeT cmd, const char *, int pulse, TRoom *, TObj *)
{
  int learn, gainAmt = 0;

  if (ch->isLinkdead() || (ch->getPosition() < POSITION_RESTING) ||
      (ch->getPosition() > POSITION_STANDING)) {
    ch->stopTask();
    return FALSE;
  }

  if (ch->utilityTaskCommand(cmd))
    return FALSE;

  if (ch->fight() && ch->isAffected(AFF_ENGAGER)) {
    ch->sendTo("You are unable to meditate while engaged in combat.");
    ch->stopTask();
    return FALSE;
  }

  switch(cmd) {
  case CMD_TASK_CONTINUE:
    if (ch->getMana() >= ch->manaLimit()) {
        ch->sendTo("Your mind is sharp and your thoughts are clear.\n\r");
        ch->stopTask();
        return TRUE;
    }
      ch->task->calcNextUpdate(pulse, 4 * PULSE_MOBACT);
      if (!ch->task->status) {
        if (!ch->roomp->isRoomFlag(ROOM_NO_HEAL)) {
          learn = ch->getSkillValue(SKILL_MEDITATE);
          if (ch->bSuccess(learn, SKILL_MEDITATE)) {
            ch->sendTo(format("%sYour meditation focuses your mind%s!\n\r") %
                     ch->green() % ch->norm());
            gainAmt = ch->manaGain() - 1;
            gainAmt = std::max(gainAmt, 1);
            ch->setMana(std::min(ch->getMana() + gainAmt, (int) ch->manaLimit()));
          } else {
	    // give them resting regen if a fail on meditate
            ch->addToMana(1);
            if (::number(0,1))
              ch->sendTo("Your meditation has slightly increased your sense of well being!\n\r");
          }
	  ch->learnFromDoingUnusual(LEARN_UNUSUAL_NORM_LEARN, SKILL_MANA, 20);

          // get this regardless...
          ch->addToHit(1);
          if (ch->getMove() < ch->moveLimit())
            ch->addToMove(1);
        } else {
          ch->sendTo("A magical force in the room stops your meditation!\n\r");
          ch->stopTask();
          return FALSE;
        }
      }
      if (ch->desc && ch->ansi()) {
        ch->desc->updateScreenAnsi(CHANGED_HP);
        ch->desc->updateScreenAnsi(CHANGED_MANA);
        ch->desc->updateScreenAnsi(CHANGED_MOVE);
      } else if (ch->desc && ch->vt100()) {
        ch->desc->updateScreenVt100(CHANGED_HP);
        ch->desc->updateScreenVt100(CHANGED_MANA);
        ch->desc->updateScreenVt100(CHANGED_MOVE);
      }
      ch->task->status = 0;
      break;
  case CMD_ABORT:
  case CMD_STOP:
  case CMD_STAND:
      act("You stop meditating and stand up.", FALSE, ch, 0, 0, TO_CHAR);
      act("$n stops meditating and stands up.", FALSE, ch, 0, 0, TO_ROOM);
      ch->setPosition(POSITION_STANDING);
      ch->stopTask();
      break;
  case CMD_TASK_FIGHTING:
      ch->sendTo("You are unable to continue meditation while under attack!\n\r");
      ch->stopTask();
      break;
  default:
    if (cmd < MAX_CMD_LIST) {
        ch->sendTo("You break your focus...\n\r");
        ch->stopTask();
    }
    return FALSE;                    // eat the command
  }
  return TRUE;
}
