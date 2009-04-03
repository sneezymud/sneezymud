//////////////////////////////////////////////////////////////////////////
//
//      SneezyMUD 4.0 - All rights reserved, SneezyMUD Coding Team
//      "task.cc" - All functions related to tasks that keep mobs/PCs busy
//
//////////////////////////////////////////////////////////////////////////

#include "stdsneezy.h"
#include "obj_tool.h"

void stop_dull(TBeing *ch)
{
  if (ch->getPosition() >= POSITION_RESTING) {
    act("You stop blunting, and look about confused.",
         FALSE, ch, 0, 0, TO_CHAR);
    act("$n stops blunting, and looks about confused.",
         FALSE, ch, 0, 0, TO_ROOM);
  }
  ch->stopTask();
}

void TThing::dullPulse(TBeing *ch, TThing *)
{
  stop_dull(ch);
}

void TTool::dullPulse(TBeing *ch, TThing *o)
{
  // sanity check
  if ( getToolType() != TOOL_FILE) {
    stop_dull(ch);
    return;
  }
  o->dullMe(ch, this);
}

void TThing::dullMe(TBeing *ch, TTool *)
{
  act("You can't figure out how to dull $p.", FALSE, ch, this, NULL, TO_CHAR);
  act("$n stops dulling.", FALSE, ch, this, NULL, TO_ROOM);
  ch->stopTask();
  return;
}

int task_dulling(TBeing *ch, cmdTypeT cmd, const char *, int pulse, TRoom *, TObj *)
{
  TThing *w = NULL;
  TThing *o = NULL;

  // sanity check
  if (ch->isLinkdead() ||
      (ch->in_room != ch->task->wasInRoom) ||
      (ch->getPosition() < POSITION_RESTING) ||
      !(o = ch->heldInPrimHand()) ||
      !isname(ch->task->orig_arg, o->name)) {
    stop_dull(ch);
    return FALSE;  // returning FALSE lets command be interpreted
  }
  if (ch->utilityTaskCommand(cmd) ||
      ch->nobrainerTaskCommand(cmd))
    return FALSE;
  switch (cmd) {
  case CMD_TASK_CONTINUE:
    // sanity check
    if (!(w = get_thing_char_using(ch, "file", 0, FALSE, FALSE))) {
        stop_dull(ch);
        return FALSE;  // returning FALSE lets command be interpreted
    }
      ch->task->calcNextUpdate(pulse, 2 * PULSE_MOBACT);
      w->dullPulse(ch, o);
      return FALSE;
  case CMD_ABORT:
  case CMD_STOP:
      act("You stop trying to smooth $p.  Maybe you could find a shop?", FALSE, ch, o, 0, TO_CHAR);
      act("$n stops blunting $p.", FALSE, ch, o, 0, TO_ROOM);
      ch->stopTask();
      break;
  case CMD_TASK_FIGHTING:
      ch->sendTo("You are unable to continue blunting while under attack!\n\r");
      ch->stopTask();
      break;
  default:
      if (cmd < MAX_CMD_LIST)
        warn_busy(ch);
      break;                    // eat the command
  }
  return TRUE;
}
