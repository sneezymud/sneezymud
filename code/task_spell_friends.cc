//////////////////////////////////////////////////////////////////////////
//
// SneezyMUD - All rights reserved, SneezyMUD Coding Team
//
//////////////////////////////////////////////////////////////////////////


#include "stdsneezy.h"

int task_spell_friends(TBeing *ch, cmdTypeT cmd, const char *, int pulse, TRoom *, TObj *)
{
  if (ch->isLinkdead() || (ch->getPosition() < POSITION_RESTING)) {
    ch->stopTask();
    return FALSE;
  }
  if (ch->utilityTaskCommand(cmd))
    return FALSE;

  if (ch->task->timeLeft < 0) {
      act("You finish casting.", FALSE, ch, 0, 0, TO_CHAR);
      act("$n finish casting.", FALSE, ch, 0, 0, TO_ROOM);
      ch->stopTask();
  }
  switch (cmd) {
  case CMD_TASK_CONTINUE:
      ch->task->calcNextUpdate(pulse, 2 * PULSE_MOBACT);
      switch (ch->task->timeLeft) {

      case 5:
	//          ch->sendCastingMessage(CASTING_ROUND5, status);
	//          general roll for failure
	//          check for change in position -- use status in bash or combat
	//          where if a successful spell distraction status goes to > 0
	//          thus in each spell continue, it checks for distraction set
	//          to anything.  The code does a roll for 1. total fail, 2. time added
	//          or nothing then clears distraction bit.
	//          distractions will be bash, or skills or big w lloping hits

          ch->sendTo("Casting Round 1\n\r");
          ch->task->timeLeft--;
          break;
      case 4:
	//          ch->sendCastingMessage(buf2, CASTING_ROUND4, status);
          ch->sendTo("Casting Round 2\n\r");
          ch->task->timeLeft--;
          break;
      case 3:
	//          ch->sendCastingMessage(buf2, CASTING_ROUND3, status);
          ch->sendTo("Casting Round 1\n\r");
          ch->task->timeLeft--;
          break;
      case 2:
	//          ch->sendCastingMessage(buf2, CASTING_ROUND2, status);
          ch->sendTo("Casting Round 2\n\r");
          ch->task->timeLeft--;
          break;
      case 1:
	//          ch->sendCastingMessage(buf2, CASTING_ROUND_LAST, status);
          ch->sendTo("Casting Round 3\n\r");
          ch->task->timeLeft--;
          break;
      case 0:
	//          ch->sendCastingMessage(buf2, CASTING_ROUND_DONE, 4);
      act("You finish casting.", FALSE, ch, 0, 0, TO_CHAR);
      act("$n finishes casting.", FALSE, ch, 0, 0, TO_ROOM);
      ch->stopTask();
          break;
      }
     break;
  case CMD_ABORT:
  case CMD_STOP:
      act("You stop casting.", FALSE, ch, 0, 0, TO_CHAR);
      act("$n stops casting.", FALSE, ch, 0, 0, TO_ROOM);
      ch->stopTask();
      break;
  case CMD_TASK_FIGHTING:
      ch->sendTo("You are unable to continue spell-casting while under attack!\n\r");
      ch->stopTask();
      break;
  default:
      if (cmd < MAX_CMD_LIST)
        warn_busy(ch);
      break;                    // eat the command
  }
  return TRUE;
}
