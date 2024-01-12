//////////////////////////////////////////////////////////////////////////
//
// SneezyMUD - All rights reserved, SneezyMUD Coding Team
//
//////////////////////////////////////////////////////////////////////////

#include "being.h"
#include "comm.h"
#include "enum.h"
#include "handler.h"
#include "obj.h"
#include "obj_tool.h"
#include "parse.h"
#include "task.h"
#include "thing.h"

class TRoom;

void stop_sharpen(TBeing* ch) {
  if (ch->getPosition() >= POSITION_RESTING) {
    act("You stop sharpening, and look about confused.", false, ch, 0, 0,
      TO_CHAR);
    act("$n stops sharpening, and looks about confused.", false, ch, 0, 0,
      TO_ROOM);
  }
  ch->stopTask();
}

void TThing::sharpenPulse(TBeing* ch, TThing*) { stop_sharpen(ch); }

void TTool::sharpenPulse(TBeing* ch, TThing* o) {
  // sanity check
  if (getToolType() != TOOL_WHETSTONE) {
    stop_sharpen(ch);
    return;
  }
  o->sharpenMe(ch, this);
}

void TThing::sharpenMe(TBeing* ch, TTool*) {
  act("You can't figure out how to sharpen $p.", false, ch, this, nullptr,
    TO_CHAR);
  act("$n stops sharpening.", false, ch, this, nullptr, TO_ROOM);
  ch->stopTask();
}

int task_sharpening(TBeing* ch, cmdTypeT cmd, const char*, int pulse, TRoom*,
  TObj*) {
  TThing *w = nullptr, *o = nullptr;

  // sanity check
  if (ch->isLinkdead() || (ch->in_room != ch->task->wasInRoom) ||
      (ch->getPosition() < POSITION_RESTING) || !(o = ch->heldInPrimHand()) ||
      !isname(ch->task->orig_arg, o->name)) {
    stop_sharpen(ch);
    return false;  // returning false lets command be interpreted
  }
  if (ch->utilityTaskCommand(cmd) || ch->nobrainerTaskCommand(cmd))
    return false;
  switch (cmd) {
    case CMD_TASK_CONTINUE:
      if (!(w = get_thing_char_using(ch, "whetstone", 0, false, false))) {
        stop_sharpen(ch);
        return false;  // returning false lets command be interpreted
      }
      ch->task->calcNextUpdate(pulse, 2 * Pulse::MOBACT);
      w->sharpenPulse(ch, o);
      return false;
    case CMD_ABORT:
    case CMD_STOP:
      act("You stop trying to sharpen $p.  Maybe you could find a shop?", false,
        ch, o, 0, TO_CHAR);
      act("$n stops sharpening $p.", false, ch, o, 0, TO_ROOM);
      ch->stopTask();
      break;
    case CMD_TASK_FIGHTING:
      ch->sendTo(
        "You are unable to continue sharpening while under attack!\n\r");
      ch->stopTask();
      break;
    default:
      if (cmd < MAX_CMD_LIST)
        warn_busy(ch);
      break;  // eat the command
  }
  return true;
}
