#include "being.h"
#include "extern.h"
#include "room.h"

class TObj;

int task_ride(TBeing *ch, cmdTypeT cmd, const char *, int pulse, TRoom *, TObj *)
{
  dirTypeT camefrom=DIR_NONE, newdir=DIR_NONE;
  int count=0;

  if((ch->utilityTaskCommand(cmd) || ch->nobrainerTaskCommand(cmd)) &&
     cmd != CMD_MOUNT && cmd != CMD_RIDE && cmd != CMD_MOVE)
    return FALSE;

  // basic tasky safechecking
  if (ch->isLinkdead()){
    act("You cease riding.",
        FALSE, ch, 0, 0, TO_CHAR);
    act("$n stops riding.",
        TRUE, ch, 0, 0, TO_ROOM);
    ch->stopTask();
    return FALSE; // returning FALSE lets command be interpreted
  }

  if(!ch->riding){
    ch->sendTo("You aren't riding anything.\n\r");
    ch->stopTask();
    return FALSE;
  }

  if(ch->task->wasInRoom == ch->in_room){
    if((newdir=getDirFromChar(ch->task->orig_arg)) == DIR_NONE){
      ch->sendTo("That isn't a direction.\n\r");
      return FALSE;
    }
  }

  switch (cmd) {
    case CMD_TASK_CONTINUE:
      ch->task->calcNextUpdate(pulse, 1);

      // first time through we set newdir above
      if(newdir==DIR_NONE){
	for(dirTypeT d=DIR_NORTH;d<MAX_DIR;d++){
	  if(ch->roomp->dir_option[d] &&
	     canSeeThruDoor(ch->roomp->dir_option[d])){
	    if(ch->roomp->dir_option[d]->to_room == ch->task->wasInRoom){
	      camefrom=d;
	      ch->task->wasInRoom=ch->in_room;
	    } else {
	      newdir=d;
	    }
	    ++count;
	  }
	}

	if(camefrom == DIR_NONE){
	  ch->sendTo("Your mount is lost.\n\r");
	  ch->stopTask();
	  break;
	}

	// either we continue in the direction we were travelling if possible
	// or we take whatever exit is available if there are 2 exits
	// otherwise stop
	if(ch->roomp->dir_option[rev_dir[camefrom]]){
	  newdir=rev_dir[camefrom];
	} else if(count != 2){
	  ch->sendTo("Your mount comes to a halt.\n\r");
	  ch->stopTask();
	  break;
	}
      }

      ch->doMove(newdir);

      break;
    case CMD_MOVE:
    case CMD_MOUNT:
    case CMD_RIDE:
      ch->stopTask();
      return FALSE;
    case CMD_ABORT:
    case CMD_STOP:
      act("Your mount stops.",
          FALSE, ch, 0, 0, TO_CHAR);
      act("$n's mount stops.",
          TRUE, ch, 0, 0, TO_ROOM);
      ch->stopTask();
      break;
    case CMD_TASK_FIGHTING:
      ch->sendTo("You cannot fight while riding.\n\r");
      ch->stopTask();
      break;
    default:
      if (cmd < MAX_CMD_LIST)
        warn_busy(ch);
      break;                    // eat the command
  }
  return TRUE;
}
