#include "stdsneezy.h"
#include "obj_tool.h"

bool find_paint_target(TBeing *ch, sstring arg, TBeing **tb, TObj **obj, TRoom **rp)
{
  roomDirData *exitp;

  if(arg.empty()){
    *rp=ch->roomp;
  } else if((exitp=ch->exitDir(getDirFromChar(arg))) &&
	    exitp->to_room && real_roomp(exitp->to_room)){
    *rp=real_roomp(exitp->to_room);
  } else {
    generic_find(arg.c_str(), FIND_CHAR_ROOM | FIND_OBJ_ROOM, ch, tb, obj);
  }

  if(!rp && !obj && !tb)
    return false;
  return true;
}

// permanent = easel, palette, brushes
// charges = paints
// one use = canvas
// easel on ground, canvas in easel, palette and brush held
bool find_paint_supplies(TBeing *ch, TTable **easel, TObj **palette, TObj **brushes, TTool **paints, TNote **canvas)
{
  // let's just pretend this works for now
  return true;
}

void TBeing::doPaint(sstring arg)
{
  TRoom *rp=NULL;
  TObj *obj=NULL;
  TBeing *tb=NULL;

  sendTo("Not yet implemented.\n\r");
  return;

  // make sure they are all setup to paint
  if(!find_paint_supplies(this, NULL, NULL, NULL, NULL, NULL)){
    sendTo("You don't seem to have everything setup to make a painting.\n\r");
    return;
  }

  // find out what they want to paint (room, object, person)
  if(!find_paint_target(this, arg, &tb, &obj, &rp)){
    sendTo("Look, you're no Dali, you better stick to painting things you can see.\n\r");
    return;
  }

  if(task){
    stopTask();
  }

  sendTo(fmt("You start painting %s.\n\r") % (rp ? rp->getName() : obj ? obj->getName() : tb ? tb->getName() : "nothing"));

  start_task(this, NULL, NULL, TASK_PAINT, arg.c_str(), 2, inRoom(), 0, 0, 5);
}



int task_painting(TBeing *ch, cmdTypeT cmd, const char *, int pulse, TRoom *rp, TObj *)
{
  if(ch->utilityTaskCommand(cmd) || ch->nobrainerTaskCommand(cmd))
    return FALSE;

  // basic tasky safechecking
  if (ch->isLinkdead() || (ch->in_room != ch->task->wasInRoom)){
    act("You cease painting.",
        FALSE, ch, 0, 0, TO_CHAR);
    act("$n stops painting.",
        TRUE, ch, 0, 0, TO_ROOM);
    ch->stopTask();
    return FALSE; // returning FALSE lets command be interpreted
  }


  if (ch->task && ch->task->timeLeft < 0){
    ch->sendTo("You pack up and stop painting.\n\r");
    ch->stopTask();
    return FALSE;
  }


  switch (cmd) {
    case CMD_TASK_CONTINUE:
      ch->task->calcNextUpdate(pulse, PULSE_MOBACT * 5);
      break;
    case CMD_ABORT:
    case CMD_STOP:
      act("You cease painting.",
          FALSE, ch, 0, 0, TO_CHAR);
      act("$n stops painting.",
          TRUE, ch, 0, 0, TO_ROOM);
      ch->stopTask();
      break;
    case CMD_TASK_FIGHTING:
      ch->sendTo("You have not yet mastered the art of fighting while painting.\n\r");
      ch->stopTask();
      break;
    default:
      if (cmd < MAX_CMD_LIST)
        warn_busy(ch);
      break;                    // eat the command
  }

  return TRUE;
}

