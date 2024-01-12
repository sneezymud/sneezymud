//////////////////////////////////////////////////////////////////////////
//
//      SneezyMUD 4.0 - All rights reserved, SneezyMUD Coding Team
//      "task.cc" - All functions related to tasks that keep mobs/PCs busy
//
//////////////////////////////////////////////////////////////////////////

#include "comm.h"
#include "obj_tool.h"
#include "handler.h"
#include "being.h"

void stop_dull(TBeing* ch) {
  if (ch->getPosition() >= POSITION_RESTING) {
    act("You stop blunting, and look about confused.", false, ch, 0, 0,
      TO_CHAR);
    act("$n stops blunting, and looks about confused.", false, ch, 0, 0,
      TO_ROOM);
  }
  ch->stopTask();
}

void TThing::dullPulse(TBeing* ch, TThing*) { stop_dull(ch); }

void TTool::dullPulse(TBeing* ch, TThing* o) {
  // sanity check
  if (getToolType() != TOOL_FILE) {
    stop_dull(ch);
    return;
  }
  o->dullMe(ch, this);
}

void TThing::dullMe(TBeing* ch, TTool*) {
  act("You can't figure out how to dull $p.", false, ch, this, nullptr, TO_CHAR);
  act("$n stops dulling.", false, ch, this, nullptr, TO_ROOM);
  ch->stopTask();
}

int task_dulling(TBeing* ch, cmdTypeT cmd, const char*, int pulse, TRoom*,
  TObj*) {
  TThing* w = nullptr;
  TThing* o = nullptr;

  // sanity check
  if (ch->isLinkdead() || (ch->in_room != ch->task->wasInRoom) ||
      (ch->getPosition() < POSITION_RESTING) || !(o = ch->heldInPrimHand()) ||
      !isname(ch->task->orig_arg, o->name)) {
    stop_dull(ch);
    return false;  // returning false lets command be interpreted
  }
  if (ch->utilityTaskCommand(cmd) || ch->nobrainerTaskCommand(cmd))
    return false;
  switch (cmd) {
    case CMD_TASK_CONTINUE:
      // sanity check
      if (!(w = get_thing_char_using(ch, "file", 0, false, false))) {
        stop_dull(ch);
        return false;  // returning false lets command be interpreted
      }
      ch->task->calcNextUpdate(pulse, 2 * Pulse::MOBACT);
      w->dullPulse(ch, o);
      return false;
    case CMD_ABORT:
    case CMD_STOP:
      act("You stop trying to smooth $p.  Maybe you could find a shop?", false,
        ch, o, 0, TO_CHAR);
      act("$n stops blunting $p.", false, ch, o, 0, TO_ROOM);
      ch->stopTask();
      break;
    case CMD_TASK_FIGHTING:
      ch->sendTo("You are unable to continue blunting while under attack!\n\r");
      ch->stopTask();
      break;
    default:
      if (cmd < MAX_CMD_LIST)
        warn_busy(ch);
      break;  // eat the command
  }
  return true;
}
