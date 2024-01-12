#include "being.h"
#include "comm.h"
#include "enum.h"
#include "limbs.h"
#include "obj.h"
#include "parse.h"
#include "task.h"
#include "thing.h"

class TRoom;

int task_extinguish_my_ass(TBeing* ch, cmdTypeT cmd, const char* arg, int pulse,
  TRoom*, TObj*) {
  if (ch->isLinkdead()) {
    ch->stopTask();
    return false;
  }

  if (ch->utilityTaskCommand(cmd))
    return false;

  switch (cmd) {
    case CMD_TASK_CONTINUE:
      ch->task->calcNextUpdate(pulse, 3);
      if (!ch->task->status) {
        act("You roll around on the ground.", false, ch, 0, 0, TO_CHAR);
        act("$n rolls frantically on the ground.", false, ch, 0, 0, TO_ROOM);
        TThing* t;
        TObj* obj = nullptr;
        int i;
        for (i = MIN_WEAR; i < MAX_WEAR; i++) {
          if (!(t = ch->equipment[i]) || !(obj = dynamic_cast<TObj*>(t)) ||
              !obj->isObjStat(ITEM_BURNING))
            continue;
          if (::number(0, 3) == 1) {
            obj->remBurning(ch);
            act("Your $p is extinguished.", false, ch, obj, 0, TO_CHAR);
          }
        }
      }
      ch->task->status = 0;
      break;
    case CMD_STAND:
      act("You stop rolling around on the ground.", false, ch, 0, 0, TO_CHAR);
      act("$n stops rolling around on the ground.", false, ch, 0, 0, TO_ROOM);
      ch->stopTask();
      break;
    case CMD_ABORT:
    case CMD_STOP:
      act("You stop rolling around on the ground.", false, ch, 0, 0, TO_CHAR);
      act("$n stops rolling around on the ground.", false, ch, 0, 0, TO_ROOM);
      ch->setPosition(POSITION_SITTING);
      ch->stopTask();
      break;
    case CMD_TASK_FIGHTING:
      ch->sendTo("You are unable to fight while extinguishing!\n\r");
      if (!::number(0, 2))
        ch->cantHit += ch->loseRound(1);
      ch->stopTask();
      break;
    default:
      if (cmd < MAX_CMD_LIST)
        return false;  // process command
      break;           // eat the command
  }
  return true;
}
