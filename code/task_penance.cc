//////////////////////////////////////////////////////////////////////////
//
//      SneezyMUD 4.0 - All rights reserved, SneezyMUD Coding Team
//      "task.cc" - All functions related to tasks that keep mobs/PCs busy
//
//////////////////////////////////////////////////////////////////////////

#include "stdsneezy.h"

int task_penance(TBeing *ch, cmdTypeT cmd, const char *, int pulse, TRoom *, TObj *)
{
  int learn;
  double val = 0.0;
  double amt = 0.0;
  double randomizer = 0.0;

  if (ch->isLinkdead() || (ch->getPosition() < POSITION_RESTING) ||
      (ch->getPosition() > POSITION_STANDING)) {
    ch->stopTask();
    return FALSE;
  }

  if (ch->utilityTaskCommand(cmd))
    return FALSE;

  if (ch->fight() && ch->isAffected(AFF_ENGAGER)) {
    ch->sendTo("You are unable to repent while engaged in combat.");
    return FALSE;
  }

  switch(cmd) {
  case CMD_TASK_CONTINUE:
    if (ch->getPiety() >= 100.0) {
        ch->sendTo("You repent your sins and you feel your soul completely cleansed.\n\r");
        ch->stopTask();
        return TRUE;
    }
      ch->task->calcNextUpdate(pulse, 5 * PULSE_MOBACT);
      ch->task->timeLeft++;
      val = (double) ch->task->timeLeft * 0.3;
      if (!ch->task->status) {
        if (!(ch->roomp->getRoomFlags() & ROOM_NO_HEAL)) {
          learn = ch->getSkillValue(SKILL_PENANCE);
          if (ch->bSuccess(learn, ch->getPerc(), SKILL_PENANCE)) {
            amt = ch->pietyGain(val);
	    
	    if ((ch->getPiety() + amt) < 100.0){
	      // we want the value to vary by +-10%   number(-amt/10, amt/10)
	      // can't randomize a double, so mult by 100 then divide by same
	      randomizer = (double) (::number((int) (-10 * amt), (int) (10 * amt)) / 100.0);
	      amt += randomizer;
	    }

            your_deity_val = SKILL_PENANCE;
            if (amt > 0.0) {
              act("Your repentance has been accepted by $d.",
                   FALSE, ch, 0, 0, TO_CHAR, ANSI_GREEN);
              ch->addToPiety(amt);
              if (ch->ansi())
                ch->desc->updateScreenAnsi(CHANGED_PIETY);
              else if (ch->vt100())
                ch->desc->updateScreenVt100(CHANGED_PIETY);
            } else if (FactionInfo[ch->getFaction()].faction_power) {
              act("$d ignores you.  More penance is needed.",
                     FALSE, ch, 0, 0, TO_CHAR, ANSI_RED);
            } else {
              act("$d are powerless to help you at this time.",
                     FALSE, ch, 0, 0, TO_CHAR, ANSI_RED);
            }
          } else {
            // penance task pulse is about 3* longer than resting pulse 
            // but resting also gives HP, MV and we want penancing to beat
            // resting in general
            amt = (::number(6,8));
            amt /= 10;
            your_deity_val = SKILL_PENANCE;
            act("Your repentance has been partially accepted by $d.",
            FALSE, ch, 0, 0, TO_CHAR, ANSI_GREEN);
            ch->addToPiety(amt);
            if (ch->ansi())
              ch->desc->updateScreenAnsi(CHANGED_PIETY);
            else if (ch->vt100())
              ch->desc->updateScreenVt100(CHANGED_PIETY);
          }
        } else {
          ch->sendTo(fmt("%sAn earthly force in the room stops your penance!%s\n\r") %
                     ch->red() % ch->norm());
          ch->stopTask();
          return FALSE;
        }
      }
      ch->task->status = 0;
      break;
  case CMD_ABORT:
  case CMD_STOP:
  case CMD_STAND:
      act("You stop repenting and stand up.", FALSE, ch, 0, 0, TO_CHAR);
      act("$n stops repenting and stands up.", FALSE, ch, 0, 0, TO_ROOM);
      ch->setPosition(POSITION_STANDING);
      ch->stopTask();
      break;
  case CMD_TASK_FIGHTING:
      ch->sendTo("You are unable to continue your repenting while under attack!\n\r");
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




