
#include "ansi.h"
#include "comm.h"
#include "obj_tool.h"
#include "being.h"
#include "handler.h"
#include "materials.h"
#include "skills.h"

void stop_debride(TBeing* ch) {
  if (ch->getPosition() >= POSITION_RESTING) {
    act("You stop debriding, and look about confused.", FALSE, ch, 0, 0,
      TO_CHAR);
    act("$n stops debriding, and looks about confused.", FALSE, ch, 0, 0,
      TO_ROOM);
  }
  ch->stopTask();
}

// Helper function to perform one pulse of debriding
// Returns: 1-3 on success, 0 on failure, -1 if tool breaks
// Handles all consequences: messages, move costs, structure damage, and tool uses
int debridePulse(TBeing* ch, TObj* obj, TThing* w) {
  int moveCost = number(1, 2);
  int strDam = number(1, 2);
  TTool* file = dynamic_cast<TTool*>(w);

  if (ch->bSuccess(SKILL_DEBRIDE)) {
    // Success: reduce timer by 1 or 2
    int timerReduction = ::number(1, 2);
    act ("You scrape the file against $p, removing some rust.", FALSE, ch, obj, 0, TO_CHAR, ANSI_YELLOW);
    act("$n scrapes the file against $p, removing some rust.", FALSE, ch, obj, 0, TO_ROOM, ANSI_YELLOW);
    if (!ch->isFocused()) {
        moveCost += 2;
        ch->sendTo ("Phew! Work this careful requires a lot of concentration.\n\r");
        act("$n begins to sweat.", FALSE, ch, 0, 0, TO_ROOM);
    }
    ch->addToMove(-moveCost);

    return timerReduction;
  } else {
    // Failure: remove 1 point from current structure
    act("Your file slips and damages $p slightly.", FALSE, ch, obj, 0, TO_CHAR, ANSI_ORANGE);
    act("$n's file slips and damages $p slightly.", FALSE, ch, obj, 0, TO_ROOM, ANSI_ORANGE);
    moveCost += 2;
    if (!ch->isFocused()){
        strDam *= 2;
        moveCost *= 3;
        ch->sendTo("You lose your focus, botching the stroke and tiring yourself out.\n\r");
        act("$n looks distracted.", FALSE, ch, 0, 0, TO_ROOM);
    }
    obj->addToStructPoints(-strDam);
    ch->addToMove(-moveCost);

    // Decrement file uses on failure
    if (file) {
      file->addToToolUses(-1);
      if (file->getToolUses() <= 0) {
        act("Your $o has been used up.", FALSE, ch, file, 0, TO_CHAR);
        act("$n's $o has been used up.", FALSE, ch, file, 0, TO_ROOM);
        ch->unequip(ch->getPrimaryHold());
        delete file;
        return -1;  // Signal to stop task
      }
    }
    return 0;
  }
}

int task_debriding(TBeing* ch, cmdTypeT cmd, const char*, int pulse, TRoom*,
  TObj*) {
  TThing *w = NULL;
  TObj *o = NULL;

  // sanity check
  if (ch->isLinkdead() || (ch->in_room != ch->task->wasInRoom) ||
      (ch->getPosition() < POSITION_RESTING)) {
    stop_debride(ch);
    return FALSE;  // returning FALSE lets command be interpreted
  }

  // Find the rusty item in inventory
  for (StuffIter it = ch->stuff.begin(); it != ch->stuff.end(); ++it) {
    if (isname(ch->task->orig_arg, (*it)->name)) {
      o = dynamic_cast<TObj*>(*it);
      break;
    }
  }
  if (!o) {
    stop_debride(ch);
    return FALSE;
  }

  // Verify file is still in primary hand
  TTool* file = ch->getToolSlot(ch->getPrimaryHold(), TOOL_FILE);
  if (!file) {
    stop_debride(ch);
    return FALSE;
  }
  w = file;
  if (ch->utilityTaskCommand(cmd) || ch->nobrainerTaskCommand(cmd))
    return FALSE;
  switch (cmd) {
    case CMD_TASK_CONTINUE: {
      // Perform the debride action
      int timerReduction = debridePulse(ch, o, w);

      // Check if tool broke
      if (timerReduction == -1) {
        ch->stopTask();
        return FALSE;
      }

      if (timerReduction > 0) {
        // Success: reduce timer
        ch->task->timeLeft -= timerReduction;
      }

      // Check if object is destroyed (structure <= 0)
      if (o->getStructPoints() <= 0) {
        act("$p crumbles to dust!", FALSE, ch, o, NULL, TO_CHAR);
        act("$p crumbles to dust!", FALSE, ch, o, NULL, TO_ROOM);
        o->makeScraps();
        ch->stopTask();
        return FALSE;
      }

      // Check if task is complete (timer expired)
      if (ch->task->timeLeft <= 0) {
        o->remObjStat(ITEM_RUSTY);
        act("You finish debriding $p.", FALSE, ch, o, NULL, TO_CHAR);
        act("$n finishes debriding $p.", FALSE, ch, o, NULL, TO_ROOM);

        // Decrement file uses on task completion
        if (file) {
          file->addToToolUses(-1);
          if (file->getToolUses() <= 0) {
            act("Your $o has been used up.", FALSE, ch, file, 0, TO_CHAR);
            act("$n's $o has been used up.", FALSE, ch, file, 0, TO_ROOM);
            ch->unequip(ch->getPrimaryHold());
            delete file;
          }
        }
        ch->stopTask();
        return FALSE;
      }

      ch->task->calcNextUpdate(pulse, 2 * Pulse::MOBACT);
      return FALSE;
    }

    case CMD_ABORT:
    case CMD_STOP:
      act("You stop trying to debride $p. This is harder than it looks.", FALSE,
        ch, o, 0, TO_CHAR);
      act("$n stops debriding $p.", FALSE, ch, o, 0, TO_ROOM);
      ch->stopTask();
      break;

    case CMD_TASK_FIGHTING:
      ch->sendTo(
        "You are unable to continue debriding while under attack!\n\r");
      ch->stopTask();
      break;

    default:
      if (cmd < MAX_CMD_LIST)
        warn_busy(ch);
      break;  // eat the command
  }
  return TRUE;
}
