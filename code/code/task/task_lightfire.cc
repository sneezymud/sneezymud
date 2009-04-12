//////////////////////////////////////////////////////////////////////////
//
// SneezyMUD - All rights reserved, SneezyMUD Coding Team
//
//////////////////////////////////////////////////////////////////////////

#include "monster.h"
#include "room.h"
#include "obj_tool.h"
#include "materials.h"
#include "handler.h"

int task_lightfire(TBeing *ch, cmdTypeT cmd, const char *, int pulse, TRoom *, TObj *obj)
{
  TThing *t;
  TMonster *guard;
  TTool *flintsteel=NULL;
  int found=0;

  if (ch->utilityTaskCommand(cmd) || ch->nobrainerTaskCommand(cmd))
    return FALSE;

  // basic tasky safechecking
  if (ch->isLinkdead() || (ch->in_room != ch->task->wasInRoom) || !obj){
    act("You cease your pyrotechnic activity.",
        FALSE, ch, 0, 0, TO_CHAR);
    act("$n stops trying to start a fire.",
        TRUE, ch, 0, 0, TO_ROOM);
    ch->stopTask();
    return FALSE; // returning FALSE lets command be interpreted
  }

  // make sure the thing we're trying to burn is still around
  if((obj != ch->heldInPrimHand()) && (obj != ch->heldInSecHand())){
    for(StuffIter it=ch->roomp->stuff.begin();it!=ch->roomp->stuff.end();){
      t=*(it++);
      if(obj==dynamic_cast<TObj *>(t))
        found=1;
    }
    if(!found){
      act("You can't find your target and stop trying to light it on fire.",
          FALSE, ch, 0, 0, TO_CHAR);
      act("$n stops trying to start a fire.",
          TRUE, ch, 0, 0, TO_ROOM);
      ch->stopTask();
      return FALSE; // returning FALSE lets command be interpreted
    }
  }

  // find our tool here
  if (!(t = get_thing_char_using(ch, "flintsteel", 0, FALSE, FALSE)) ||
      !(flintsteel=dynamic_cast<TTool *>(t))){
    ch->sendTo("You need to own some flint and steel to light that.\n\r");
    ch->stopTask();
    return FALSE;
  }

  // check for guards that prevent
  for(StuffIter it=ch->roomp->stuff.begin();it!=ch->roomp->stuff.end();){
    t=*(it++);
    guard = dynamic_cast<TMonster *>(t);
    if (!guard)
      continue;
    if (!guard->isPolice() || !guard->canSee(ch) ||
         !guard->awake())
      continue;
    guard->doSay("Hey!  Cut that out, you trying to burn the place down?");
    act("You cease your pyrotechnic activity.",
        FALSE, ch, 0, 0, TO_CHAR);
    act("$n stops trying to start a fire.",
        TRUE, ch, 0, 0, TO_ROOM);
    ch->stopTask();
    return FALSE;
  }

  if (ch->task->timeLeft < 0){
    if(material_nums[obj->getMaterial()].flammability){
      obj->setBurning(ch);

      act("You set $p on fire.",
          FALSE, ch, obj, 0, TO_CHAR);
      act("$n sets $p on fire.",
          TRUE, ch, obj, 0, TO_ROOM);
    } else {
      act("That $o doesn't seem to be flammable.",
          FALSE, ch, obj, 0, TO_CHAR);
      act("$n stops trying to start a fire.",
          TRUE, ch, 0, 0, TO_ROOM);
    }
    ch->stopTask();
    return FALSE;
  }
  switch (cmd) {
  case CMD_TASK_CONTINUE:
    // low flammability is very slow (10), wood is about 3
      ch->task->calcNextUpdate(pulse, PULSE_MOBACT *
             (10-(material_nums[obj->getMaterial()].flammability/70)));

      flintsteel->addToToolUses(-1);
      if (flintsteel->getToolUses() <= 0) {
        act("Oops, your $o has been used up.",
            FALSE, ch, flintsteel, 0, TO_CHAR);
        act("$n looks startled as $e realizes that his $o has been used up.",
            FALSE, ch, flintsteel, 0, TO_ROOM);
        ch->stopTask();
        delete flintsteel;
        return FALSE;
      }

      switch (ch->task->timeLeft) {
      case 2:
          act("You lay some thin <o>wood shavings<1> around the $o.",
              FALSE, ch, obj, 0, TO_CHAR);
          act("$n lays some thin <o>wood shavings<1> around the $o.",
              TRUE, ch, obj, 0, TO_ROOM);
          ch->task->timeLeft--;
          break;
      case 1:
          act("You strike your $o, sending <Y>sparks<1> into the <o>wood shavings<1>.",
              FALSE, ch, flintsteel, 0, TO_CHAR);
          act("$n strikes $s $o, sending <Y>sparks<1> into the <o>wood shavings<1>.",
              TRUE, ch, flintsteel, 0, TO_ROOM);
          ch->task->timeLeft--;
          break;
      case 0:
          act("You carefully fan the <r>flames<1> towards $p.",
              FALSE, ch, obj, 0, TO_CHAR);
          act("$n carefully fans the <r>flames<1> towards $p.",
              TRUE, ch, obj, 0, TO_ROOM);
          ch->task->timeLeft--;
          break;
      }
      break;
  case CMD_ABORT:
  case CMD_STOP:
      act("You cease your pyrotechnic activity.",
          FALSE, ch, 0, 0, TO_CHAR);
      act("$n stops trying to start a fire.",
          TRUE, ch, 0, 0, TO_ROOM);
      ch->stopTask();
      break;
  case CMD_TASK_FIGHTING:
      ch->sendTo("You can't properly start a fire while under attack.\n\r");
      ch->stopTask();
      break;
  default:
      if (cmd < MAX_CMD_LIST)
        warn_busy(ch);
      break;                    // eat the command
  }
  return TRUE;
}

