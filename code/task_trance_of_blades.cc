//////////////////////////////////////////////////////////////////////////
//
// SneezyMUD - All rights reserved, SneezyMUD Coding Team
//
// $Log: task_trance_of_blades.cc,v $
// Revision 1.1  2000/12/22 07:12:03  dash
// Initial revision
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
//      SneezyMUD 4.0 - All rights reserved, SneezyMUD Coding Team
//      "task.cc" - All functions related to tasks that keep mobs/PCs busy
//
//////////////////////////////////////////////////////////////////////////

#include "stdsneezy.h"

void stop_trance_of_blades(TBeing *ch)
{
  if (ch->getPosition() >= POSITION_RESTING) {
    act("You suddenly snap out of your trance.",
	FALSE, ch, 0, 0, TO_CHAR);
    act("$n suddenly snaps out of $s trance.",
	FALSE, ch, 0, 0, TO_ROOM);
  }
  ch->stopTask();
}

void TBaseWeapon::tranceOfBladesPulse(TBeing *ch, TThing *)
{
  stop_trance_of_blades(ch);
  return;
}


int task_trance_of_blades(TBeing *ch, cmdTypeT cmd, const char *, int pulse, TRoom *, TObj *)
{
  TThing *o = NULL;

  // sanity check
  if (ch->isLinkdead() ||
      (ch->in_room != ch->task->wasInRoom) ||
      (ch->getPosition() < POSITION_RESTING) ||
      !(o = ch->heldInPrimHand())) {
    stop_trance_of_blades(ch);
    return FALSE;  // returning FALSE lets command be interpreted
  }
  if (ch->utilityTaskCommand(cmd) ||
      ch->nobrainerTaskCommand(cmd))
    return FALSE;
  switch (cmd) {
  case CMD_TASK_CONTINUE:
    ch->task->calcNextUpdate(pulse, 2 * PULSE_MOBACT);
    //w->sharpenPulse(ch, o);
    return FALSE;
  case CMD_ABORT:
  case CMD_STOP:
    act("You slowly come out of the trance.",
	FALSE, ch, o, 0, TO_CHAR);
    act("$n slowly comes out of $s trance.", FALSE, ch, o, 0, TO_ROOM);
    ch->stopTask();
    break;
  case CMD_TASK_FIGHTING:
    act("Your $o becomes a blur as you concentrate on your defensive trance.", FALSE, ch, o, 0, TO_CHAR);
    act("$n's $o becomes a blur as $e concentrates on $s defensive trance.", FALSE, ch, o, 0, TO_ROOM);
    break;
  default:
    if (cmd < MAX_CMD_LIST)
      warn_busy(ch);
    break;                    // eat the command
  }
  return TRUE;
}

