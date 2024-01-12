//////////////////////////////////////////////////////////////////////////
//
// SneezyMUD - All rights reserved, SneezyMUD Coding Team
//
//////////////////////////////////////////////////////////////////////////

#include <list>

#include "being.h"
#include "comm.h"
#include "handler.h"
#include "materials.h"
#include "monster.h"
#include "obj.h"
#include "obj_tool.h"
#include "parse.h"
#include "room.h"
#include "task.h"
#include "thing.h"

int task_lightfire(TBeing* ch, cmdTypeT cmd, const char*, int pulse, TRoom*,
  TObj* obj) {
  TThing* t;
  TMonster* guard;
  TTool* flintsteel = nullptr;
  int found = 0;

  if (ch->utilityTaskCommand(cmd) || ch->nobrainerTaskCommand(cmd))
    return false;

  // basic tasky safechecking
  if (ch->isLinkdead() || (ch->in_room != ch->task->wasInRoom) || !obj) {
    act("You cease your pyrotechnic activity.", false, ch, 0, 0, TO_CHAR);
    act("$n stops trying to start a fire.", true, ch, 0, 0, TO_ROOM);
    ch->stopTask();
    return false;  // returning false lets command be interpreted
  }

  // make sure the thing we're trying to burn is still around
  if ((obj != ch->heldInPrimHand()) && (obj != ch->heldInSecHand())) {
    for (StuffIter it = ch->roomp->stuff.begin();
         it != ch->roomp->stuff.end();) {
      t = *(it++);
      if (obj == dynamic_cast<TObj*>(t))
        found = 1;
    }
    if (!found) {
      act("You can't find your target and stop trying to light it on fire.",
        false, ch, 0, 0, TO_CHAR);
      act("$n stops trying to start a fire.", true, ch, 0, 0, TO_ROOM);
      ch->stopTask();
      return false;  // returning false lets command be interpreted
    }
  }

  // find our tool here
  if (!(t = get_thing_char_using(ch, "flintsteel", 0, false, false)) ||
      !(flintsteel = dynamic_cast<TTool*>(t))) {
    ch->sendTo("You need to own some flint and steel to light that.\n\r");
    ch->stopTask();
    return false;
  }

  // check for guards that prevent
  for (StuffIter it = ch->roomp->stuff.begin(); it != ch->roomp->stuff.end();) {
    t = *(it++);
    guard = dynamic_cast<TMonster*>(t);
    if (!guard)
      continue;
    if (!guard->isPolice() || !guard->canSee(ch) || !guard->awake())
      continue;
    guard->doSay("Hey!  Cut that out, you trying to burn the place down?");
    act("You cease your pyrotechnic activity.", false, ch, 0, 0, TO_CHAR);
    act("$n stops trying to start a fire.", true, ch, 0, 0, TO_ROOM);
    ch->stopTask();
    return false;
  }

  if (ch->task->timeLeft < 0) {
    if (material_nums[obj->getMaterial()].flammability) {
      obj->setBurning(ch);

      act("You set $p on fire.", false, ch, obj, 0, TO_CHAR);
      act("$n sets $p on fire.", true, ch, obj, 0, TO_ROOM);
    } else {
      act("That $o doesn't seem to be flammable.", false, ch, obj, 0, TO_CHAR);
      act("$n stops trying to start a fire.", true, ch, 0, 0, TO_ROOM);
    }
    ch->stopTask();
    return false;
  }
  switch (cmd) {
    case CMD_TASK_CONTINUE:
      // low flammability is very slow (10), wood is about 3
      ch->task->calcNextUpdate(pulse,
        Pulse::MOBACT *
          (10 - (material_nums[obj->getMaterial()].flammability / 70)));

      flintsteel->addToToolUses(-1);
      if (flintsteel->getToolUses() <= 0) {
        act("Oops, your $o has been used up.", false, ch, flintsteel, 0,
          TO_CHAR);
        act("$n looks startled as $e realizes that his $o has been used up.",
          false, ch, flintsteel, 0, TO_ROOM);
        ch->stopTask();
        delete flintsteel;
        return false;
      }

      switch (ch->task->timeLeft) {
        case 2:
          act("You lay some thin <o>wood shavings<1> around the $o.", false, ch,
            obj, 0, TO_CHAR);
          act("$n lays some thin <o>wood shavings<1> around the $o.", true, ch,
            obj, 0, TO_ROOM);
          ch->task->timeLeft--;
          break;
        case 1:
          act(
            "You strike your $o, sending <Y>sparks<1> into the <o>wood "
            "shavings<1>.",
            false, ch, flintsteel, 0, TO_CHAR);
          act(
            "$n strikes $s $o, sending <Y>sparks<1> into the <o>wood "
            "shavings<1>.",
            true, ch, flintsteel, 0, TO_ROOM);
          ch->task->timeLeft--;
          break;
        case 0:
          act("You carefully fan the <r>flames<1> towards $p.", false, ch, obj,
            0, TO_CHAR);
          act("$n carefully fans the <r>flames<1> towards $p.", true, ch, obj,
            0, TO_ROOM);
          ch->task->timeLeft--;
          break;
      }
      break;
    case CMD_ABORT:
    case CMD_STOP:
      act("You cease your pyrotechnic activity.", false, ch, 0, 0, TO_CHAR);
      act("$n stops trying to start a fire.", true, ch, 0, 0, TO_ROOM);
      ch->stopTask();
      break;
    case CMD_TASK_FIGHTING:
      ch->sendTo("You can't properly start a fire while under attack.\n\r");
      ch->stopTask();
      break;
    default:
      if (cmd < MAX_CMD_LIST)
        warn_busy(ch);
      break;  // eat the command
  }
  return true;
}
