//////////////////////////////////////////////////////////////////////////
//
// SneezyMUD - All rights reserved, SneezyMUD Coding Team
//
//////////////////////////////////////////////////////////////////////////

#include "stdsneezy.h"
#include "obj_tool.h"

void stop_sharpen(TBeing *ch)
{
  if (ch->getPosition() >= POSITION_RESTING) {
    act("You stop sharpening, and look about confused.",
           FALSE, ch, 0, 0, TO_CHAR);
    act("$n stops sharpening, and looks about confused.",
           FALSE, ch, 0, 0, TO_ROOM);
  }
  ch->stopTask();
}

void TThing::sharpenPulse(TBeing *ch, TThing *)
{
  stop_sharpen(ch);
  return;
}

void TTool::sharpenPulse(TBeing *ch, TThing *o)
{
  // sanity check
  if (getToolType() != TOOL_WHETSTONE) {
    stop_sharpen(ch);
    return;
  }
  o->sharpenMe(ch, this);
}

void TThing::sharpenMe(TBeing *ch, TTool *)
{
  act("You can't figure out how to sharpen $p.", FALSE, ch, this, NULL, TO_CHAR);
  act("$n stops sharpening.", FALSE, ch, this, NULL, TO_ROOM);
  ch->stopTask();
  return;
}

int task_sharpening(TBeing *ch, cmdTypeT cmd, const char *, int pulse, TRoom *, TObj *)
{
  TThing *w = NULL, *o = NULL;

  // sanity check
  if (ch->isLinkdead() ||
      (ch->in_room != ch->task->wasInRoom) ||
      (ch->getPosition() < POSITION_RESTING) ||
      !(o = ch->heldInPrimHand()) ||
      !isname(ch->task->orig_arg, o->name)) {
    stop_sharpen(ch);
    return FALSE;  // returning FALSE lets command be interpreted
  }
  if (ch->utilityTaskCommand(cmd) ||
      ch->nobrainerTaskCommand(cmd))
    return FALSE;
  switch (cmd) {
  case CMD_TASK_CONTINUE:
    if ( !(w = get_thing_char_using(ch, "whetstone", 0, FALSE, FALSE))) {
        stop_sharpen(ch);
        return FALSE;  // returning FALSE lets command be interpreted
    }
      ch->task->calcNextUpdate(pulse, 2 * PULSE_MOBACT);
      w->sharpenPulse(ch, o);
      return FALSE;
  case CMD_ABORT:
  case CMD_STOP:
      act("You stop trying to sharpen $p.  Maybe you could find a shop?",
          FALSE, ch, o, 0, TO_CHAR);
      act("$n stops sharpening $p.", FALSE, ch, o, 0, TO_ROOM);
      ch->stopTask();
      break;
  case CMD_TASK_FIGHTING:
      ch->sendTo("You are unable to continue sharpening while under attack!\n\r");
      ch->stopTask();
      break;
  default:
      if (cmd < MAX_CMD_LIST)
        warn_busy(ch);
      break;                    // eat the command
  }
  return TRUE;
}
