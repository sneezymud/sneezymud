//////////////////////////////////////////////////////////////////////////
//
// SneezyMUD - All rights reserved, SneezyMUD Coding Team
//
//////////////////////////////////////////////////////////////////////////

#include "being.h"
#include "room.h"
#include "obj.h"
#include "extern.h"
int task_search(TBeing *ch, cmdTypeT cmd, const char *, int pulse, TRoom *, TObj *)
{
  roomDirData *fdd = NULL;
  int moveCost   = 3, // 10 exits *3 = 30, old cost.
      bKnown     = ch->getSkillValue(SKILL_SEARCH),
      tsSuccess  = ch->bSuccess(bKnown, SKILL_SEARCH),
      eDirection = ch->task->flags;
  char buf[256];

  if (ch->isLinkdead() || (ch->in_room < 0) ||
      (ch->getPosition() < POSITION_RESTING)) {
    ch->stopTask();
    return FALSE;
  }
  if (ch->utilityTaskCommand(cmd) ||
      ch->nobrainerTaskCommand(cmd))
    return FALSE;

  // Make sure our leader didn't move us out of the room.
  if (ch->task->wasInRoom != ch->in_room) {
    ch->sendTo("You stop your searching due to your sudden move.\n\r");
    act("$n stops searching and glares about.",
        FALSE, ch, 0, 0, TO_ROOM);
  }
  if (ch->getMove() < moveCost) {
    ch->sendTo("You are too tired to continue searching.\n\r");
    ch->stopTask();
    return TRUE;
  }
  if (ch->task->flags >= 100)
    eDirection -= 100;
  // Exits are 0, ..., 9.  This value[ch->task->flags] should NEVER get above/below that mark.
  if (eDirection < 0 || eDirection > 9) {
    ch->sendTo("Something went wrong in your searching, tell a god what you did.\n\r");
    ch->stopTask();
    return TRUE;
  }
  switch(cmd) {
  case CMD_TASK_CONTINUE:
    if (eDirection == DIR_UP && ch->roomp->getRoomHeight() <= 0) {
        ch->sendTo("You decide to skip the ceiling, seeing that there isn't one.\n\r");
        act("$n looks up and decides to skip the ceiling, seeing that there isn't one.",
            FALSE, ch, 0, 0, TO_ROOM);
        ch->task->flags++;
        if (ch->task->flags >= 100) {
          ch->stopTask();
          return TRUE;
        } else
          return task_search(ch, cmd, "", pulse, NULL, NULL);
    } else if (eDirection == DIR_DOWN && (fdd = ch->roomp->dir_option[eDirection]) &&
                 ((IS_SET(fdd->condition, EX_SECRET) &&
                  !IS_SET(fdd->condition, EX_CLOSED)) ||
                  !IS_SET(fdd->condition, EX_SECRET))) {
        act("You decide to not search the $g.  Seeing there is an exit there.\n\r",
            TRUE, ch, 0, NULL, TO_CHAR);
        ch->task->flags++;
        if (ch->task->flags >= 100) {
          ch->stopTask();
          return TRUE;
        } else
          return task_search(ch, cmd, "", pulse, NULL, NULL);
    } else if ((fdd = ch->roomp->dir_option[eDirection]) &&
                 ((IS_SET(fdd->condition, EX_SECRET) &&
                  !IS_SET(fdd->condition, EX_CLOSED)) ||
                  !IS_SET(fdd->condition, EX_SECRET))) {
        ch->sendTo(format("You decide to skip searching %s.  Seeing there is an exit there.\n\r") %
                   dirs_to_blank[eDirection]);
        if (++ch->task->flags == 10) {
          ch->sendTo("You finish your searching and stop.\n\r");
          act("$n finishes searching and stops.",
              FALSE, ch, 0, 0, TO_ROOM);
          ch->stopTask();
          return TRUE;
        }
        if (ch->task->flags >= 100) {
          ch->stopTask();
          return TRUE;
        } else
          return task_search(ch, cmd, "", pulse, NULL, NULL);
    }
    // If we've searched 3, 6, or 9 directions lets attempt a gain.  This means
    // Search will gain 3 times like this but will take some more work to actually
    // get the increases.
    // Skipped exits, above this code, do Not count as searched directions.
      if ((ch->task->timeLeft % 3) == 0 && ch->task->timeLeft != 0)
        ch->learnFromDoingUnusual(LEARN_UNUSUAL_NORM_LEARN, SKILL_SEARCH, 5);
      // Basic messages to the thief and others in the room to let them know we are
      // searching for exits.
      if (eDirection == DIR_UP) {
        act("$n searches the ceiling for secret doors.",
            FALSE, ch, 0, 0, TO_ROOM);
        ch->sendTo("You search the ceiling for secret doors.\n\r");
        ch->task->timeLeft++;
      } else if (eDirection == DIR_DOWN) {
        act("$n searches the $g for secret doors.",
            FALSE, ch, 0, 0, TO_ROOM);
        act("You search the $g for secret doors.",
            FALSE, ch, 0, 0, TO_CHAR);
        ch->task->timeLeft++;
      } else {
        sprintf(buf, "$n searches %s for secret doors.", dirs_to_blank[eDirection]);
        act(buf, FALSE, ch, 0, 0, TO_ROOM);
        ch->sendTo(format("You search %s for secret doors.\n\r") % dirs_to_blank[eDirection]);
        ch->task->timeLeft++;
      }
      // Basic Checks:
      //   1. Successful Search
      //   2. Is set Secret And Closed
      //   3. Is Not named '_unique_door_'
      if (tsSuccess && (fdd = ch->roomp->dir_option[eDirection]) &&
          IS_SET(fdd->condition, EX_SECRET) &&
          IS_SET(fdd->condition, EX_CLOSED) &&
		  fdd->keyword && 
          strcmp(fdd->keyword, "_unique_door_")) {

        const char * foundPrint = "Secret %s found %s!\n\r";
        const char * toRoomPrint = "$n exclaims, \"Look!  A Secret %s found %s!\"\n\r";
		sstring doorName = fname(fdd->keyword);

        // if the doorname is empty, use a different foundPrint & doorName
        // this is valid if, the door was only opened via room proc, like 'press button' or 'twist lid off'
        if (doorName.empty() || doorName == " ") {
            foundPrint = "Secret exit found %s%s, but you can't determine how it is opened!\n\r";
            toRoomPrint = "$n exclaims, \"Look!  A Secret exit %s%s!\"\n\r";
            doorName = "";
		}

        sprintf(buf, foundPrint, doorName.c_str(), dirs[eDirection]);
        ch->sendTo(buf);
        sprintf(buf, toRoomPrint, doorName.c_str(), dirs[eDirection]);
        act(buf, FALSE, ch, 0, 0, TO_ROOM);
	  }
      // We remove the moves here, just in case we checked the last direction.
      ch->setMove(max(0, (ch->getMove() - moveCost)));
      // Here is where we increase our current direction marker and see if we've
      // searched every possible direction in this room.
      if (++ch->task->flags == 10) {
        ch->sendTo("You finish your searching and stop.\n\r");
        act("$n finishes searching and stops.",
            FALSE, ch, 0, 0, TO_ROOM);
        ch->stopTask();
        return TRUE;
      }
      if (ch->task->flags >= 100) {
        ch->stopTask();
        return TRUE;
      }
    break;
  case CMD_STOP:
  case CMD_ABORT:
      ch->stopTask();
      ch->sendTo("You stop searching for secret exits.\n\r");
      act("$n stops searching.", FALSE, ch, 0, 0, TO_ROOM);
    break;
  case CMD_TASK_FIGHTING:
      ch->sendTo("You are unable to continue searching while under attack!\n\r");
      ch->stopTask();
    break;
  default:
      if (cmd < MAX_CMD_LIST)
        warn_busy(ch);
  }
  return TRUE;
}
