//////////////////////////////////////////////////////////////////////////
//
//      SneezyMUD - All rights reserved, SneezyMUD Coding Team
//      "task.cc" - All functions related to tasks that keep mobs/PCs busy
//
//////////////////////////////////////////////////////////////////////////

#include "stdsneezy.h"
#include "obj_portal.h"
#include "pathfinder.h"

void stop_tracking(TBeing *ch)
{
  if (!ch->isLinkdead() && (ch->in_room > 0) &&
      (ch->getPosition() >= POSITION_RESTING)) {
    act("You stop and look about blankly.",
            FALSE, ch, 0, 0, TO_CHAR);
    act("$n stops and looks about blankly.",
            FALSE, ch, 0, 0, TO_ROOM);
  }
  ch->stopTask();
  ch->remPlayerAction(PLR_HUNTING);
  if (ch->affectedBySpell(SKILL_TRACK)     )
    ch->affectFrom(SKILL_TRACK);
  if (ch->affectedBySpell(SKILL_SEEKWATER) )
    ch->affectFrom(SKILL_SEEKWATER);
  ch->specials.hunting = NULL;
  ch->hunt_dist = 0;
}

int task_tracking(TBeing *ch, cmdTypeT cmd, const char *argument, int pulse, TRoom *, TObj *obj)
{
  // Do a hard return now if they have already been given their message.
  if ((!ch || !ch->task || (ch->task->flags > 0 && ch->task->flags != 100)) &&
      cmd == CMD_TASK_CONTINUE)
    return FALSE;

  affectedData *aff;
  roomDirData *Eroom=NULL;
  TThing *t;
  int code=-1;
  int skill;
  int targetRm = -1;
  bool isSW = ch->affectedBySpell(SKILL_SEEKWATER);
  bool isTR = ch->affectedBySpell(SKILL_TRACK);
  bool isTS = ch->affectedBySpell(SPELL_TRAIL_SEEK);
  bool worked;
  char buf[256], buf2[256];

  if (ch->isLinkdead() || (ch->in_room < 0) ||
      (ch->getPosition() < POSITION_RESTING)) {
    stop_tracking(ch);
    return FALSE;  // return FALSE lets the command be interpreted
  }

  if (ch->utilityTaskCommand(cmd) ||
      ch->nobrainerTaskCommand(cmd))
    return FALSE;

  if (!ch->specials.hunting && !isSW) {
    act("For some reason your quarry can no longer be found.",
        FALSE, ch, 0, 0, TO_CHAR);
    stop_tracking(ch);
    return FALSE;
  }

  skill = 0;

  if (ch->specials.hunting)
    for (aff = ch->specials.hunting->affected; aff; aff = aff->next)
      if (aff->type == SKILL_CONCEALMENT)
        skill = -aff->modifier;

  if (isSW) {
    skill += ch->getSkillValue(SKILL_SEEKWATER);
    worked = ch->bSuccess(skill, SKILL_SEEKWATER);
  } else if (isTR) {
    skill += ch->getSkillValue(SKILL_TRACK);
    worked = ch->bSuccess(skill, SKILL_TRACK);
  } else if (isTS) {
    skill += 50;
    worked = (::number(0, 110) < skill);
  } else {
    // in theory, we would never get here.
    // however, say a mob hates a creature so has specials.hunting.
    // a god switches into the mob, and does a look.
    // look logic kicks in and we get here.
    return TRUE;
  }

  switch(cmd) {
  case CMD_TRACK:
    if (!isSW) {
        if (*argument)
          ch->sendTo("You give up tracking your quarry and choose a new one.\n\r");
        stop_tracking(ch);
        ch->addToWait(combatRound(1));
        if (*argument)
          return FALSE;
        else
          return TRUE;
    }
      warn_busy(ch);
      return TRUE;
  case CMD_SEEKWATER:
    if (isSW) {
        ch->sendTo("Your already seeking water.\n\r");
        return TRUE;
    }
      ch->sendTo("You give up tracking your quarry and choose to seek some water.\n\r");
      stop_tracking(ch);
      ch->addToWait(combatRound(1));
      return FALSE;
  case CMD_NORTH:
  case CMD_EAST:
  case CMD_SOUTH:
  case CMD_WEST:
  case CMD_UP:
  case CMD_DOWN:
  case CMD_NE:
  case CMD_NW:
  case CMD_SE:
  case CMD_SW:
      Eroom=ch->exitDir(getDirFromCmd(cmd));
  case CMD_ENTER:
    // Each time the person moves we reduce the flags value by 1.  This way
    // we eat up 1 'CMD_TASK_CONTINUE' when it is called for each move.  Now
    // we must make sure were not simply dropping this to 0, because if we
    // moved we apparently need to try and find the target again but we also
    // don't want this to occure to fast, so we drop it to -1 at Least.
    // We also eat some moves should the exit exist.
      if (ch->task->flags > -5)
        ch->task->flags = min(-1, ch->task->flags - 1);
      if (exit_ok(Eroom, NULL))
        ch->addToMove(-(int) (110-
			      ch->getSkillValue((isSW ? SKILL_SEEKWATER :
						 (isTR ? SKILL_TRACK : SPELL_TRAIL_SEEK))))/20);
      return FALSE;
  case CMD_TELL:
  case CMD_WHISPER:
      if (ch->task->flags > -5 && ch->task->flags < 1)
        ch->task->flags--;
      return FALSE;
  case CMD_SCAN:
      if (ch->task->flags > -4 && ch->task->flags < 1);
        ch->task->flags -= 2;
      return FALSE;
  case CMD_TASK_CONTINUE:
    // Are we supposed to eat this continue?  If less than 0, you betcha.
    // Add 1 to flags to mark the eat then return back.
    if (ch->task->flags < 0) {
        ch->task->flags++;
        return TRUE;
    }
      if (ch->task->flags > 0)
        return TRUE;
      // Guess this one wasn't to be ate.  So we check to see if we've found
      // our target yet (0==no, >=1 == yes).  If not, then we try again.
      // 100 means it was to dark last time, so lets give it another shot.
      if (ch->task->flags == 0 || ch->task->flags == 100) {
        // Is it too dark to see?  If so then we see if it was last time, also.
        // If not we pump out a too dark to see message then return, if it was
        // then we just return without redundant messaging.
        if (ch->roomp && !ch->isImmortal() &&
            (ch->roomp->getLight() + ch->visionBonus <= 0) &&
            !ch->roomp->isRoomFlag(ROOM_ALWAYS_LIT) &&
            !ch->isAffected(AFF_TRUE_SIGHT)) {
          if (ch->task->flags != 100) {
            ch->task->flags = 100;
            ch->sendTo(fmt("You can't see well enough to %s.\n\r") %
                       (isSW ? "seek water" : "track"));
          }
          return TRUE;
        }
        // Must be able to see now.  So if we couldn't last time, lets ditch
        // the 100 value and go back to no-target-found 0.
        if (ch->task->flags == 100)
          ch->task->flags = 0;
        if (!ch->specials.hunting && !isSW) {
          vlogf(LOG_BUG, "problem in task_track()");
          stop_tracking(ch);
          return TRUE;
        }
        // Are we seeking water?  Must be if we don't have a hunt target.
        if (!ch->specials.hunting) {
          if (isSW){
	    TPathFinder path(ch->hunt_dist);
	    
	    code=path.findPath(ch->in_room, findWater());
	    targetRm=path.getDest();
	  } else {
            vlogf(LOG_BUG, "problem in task_track()");
            stop_tracking(ch);
            return TRUE;
          }
        } else {
          // Guess we have a hunt target, lets find them.
          if (ch->isImmortal())
            code = choose_exit_global(ch->in_room, ch->specials.hunting->in_room, ch->hunt_dist);
          else if ((ch->GetMaxLevel() >= MIN_GLOB_TRACK_LEV) || ch->affectedBySpell(SPELL_TRAIL_SEEK)
		   || !ch->isPc())
            code = choose_exit_global(ch->in_room, ch->specials.hunting->in_room, ch->hunt_dist);
          else
            code = choose_exit_in_zone(ch->in_room, ch->specials.hunting->in_room, ch->hunt_dist);
        }
        // This is actually checked in track(), which should have been called before
        // us.  But you never know, bad things Do happen.
        // An example in this case.  We start to track, fail, mob were tracking moves into
        // our room, we Don't do a look to trigger track(), and the task pulse hits causeing
        // this bit of code to become active.
        if ((ch->specials.hunting && ch->sameRoom(*ch->specials.hunting)) ||
            (targetRm != -1 && targetRm == ch->inRoom())) {
          ch->sendTo(fmt("%s###You have found %s!%s\n\r") % ch->orange() %
                     (isSW ? "some water" : "your target") % ch->norm());
          if (ch->desc && ch->desc->m_bIsClient)
            ch->desc->clientf(fmt("%d") % CLIENT_TRACKOFF);
          ch->stopTask();
          ch->remPlayerAction(PLR_HUNTING);
          if (ch->affectedBySpell(SKILL_TRACK)     ) ch->affectFrom(SKILL_TRACK);
          if (ch->affectedBySpell(SKILL_SEEKWATER) ) ch->affectFrom(SKILL_SEEKWATER);
          ch->specials.hunting = NULL;
          ch->addToWait(combatRound(1));
          return TRUE;
        }
        // This should never happen, but just in case.
        if (code < 0) {
          ch->sendTo("Your target has vanished, how odd.\n\r");
          stop_tracking(ch);
          return TRUE;
        }
        // Success
        if (worked) {
          // This way we can find out the exit in look and compare.
          ch->task->flags = code+1;
          // If the exit code is less than 10, then it has to be on the 10-exit compass
          if (code >= 0 && code <= 9) {
            ch->sendTo(fmt("%s###You track %s %s.%s\n\r") %
                   ch->purple() %
		       (isSW ? "some water" : "your target") %
                   dirs_to_blank[code] % ch->norm());
            // Client check
            if (ch->desc && ch->desc->m_bIsClient)
              ch->desc->clientf(fmt("%d|%d") % CLIENT_TRACKING % (1 << code));
            if (ch->desc && (ch->desc->autobits & AUTO_HUNT)) {
              strcpy(buf, dirs[code]);
              ch->addCommandToQue(buf);
            }
          } else if (code > 9) {
            // It's above 9, so it's a special exit.  Portal or something.
            int count = code - 9,
                seen  = 0;
            for (t = ch->roomp->getStuff(); t; t = t->nextThing) {
              TPortal *tp = dynamic_cast<TPortal *>(t);
              if (tp) {
                seen++;
                if (count == seen) {
                  ch->sendTo(COLOR_OBJECTS, fmt("%sYou track %s through %s.%s\n\r") % ch->purple() %
                             (isSW ? "some water" : "your quarry") % tp->getName() % ch->norm());
                  break;
                }
              }
            }
            // Bad, our special exit is Gone...even tho it was there a second ago.
            if (!t) {
              ch->sendTo("Error finding path target!  Tell a god.\n\r");
              vlogf(LOG_BUG, "Error finding path (task_tracking).");
              stop_tracking(ch);
              return TRUE;
            }
            // Client check.
            if (ch->desc && ch->desc->m_bIsClient)
              ch->desc->clientf(fmt("%d|%d") % CLIENT_TRACKING % (1 << code));
            if (ch->desc && (ch->desc->autobits & AUTO_HUNT)) {
              strcpy(buf, t->name);
              strcpy(buf, add_bars(buf).c_str());
              sprintf(buf2, "enter %s", buf);
              ch->addCommandToQue(buf2);
            }
          }
          return TRUE;
        } else {
          // Failure.
          ch->sendTo(COLOR_MOBS, fmt("You continue tracking %s, but fail this turn.\n\r") %
                     (isSW ? "some water" : ch->specials.hunting->getName()));
          ch->task->flags -= 2;
          ch->addToWait(combatRound(1));
          ch->addToMove((int) min(10, (-2-((110-ch->getSkillValue(
		        (isSW ? SKILL_SEEKWATER : (isTR ? SKILL_TRACK : SPELL_TRAIL_SEEK))))/6))));
          if (ch->getMove() <= 0) {
            act("You just don't feel like you could go on right now.",
                FALSE, ch, 0, 0, TO_CHAR);
            act("$n looks incredibly tired.",
                FALSE, ch, 0, 0, TO_ROOM);
            stop_tracking(ch);
          }
        }
      }
      break;
  case CMD_ABORT:
  case CMD_STOP:
      ch->sendTo(COLOR_MOBS, fmt("You stop %s %s.\n\r") %
                 (isSW ? "seeking" : "tracking") %
                 (isSW ? "water"   : ch->specials.hunting->getName()));
      stop_tracking(ch);
      ch->addToWait(combatRound(1));
      break;
  case CMD_TASK_FIGHTING:
      ch->sendTo(fmt("You can not continue %s while under attack!\n\r") %
                 (isSW ? "seeking water" : "tracking"));
      stop_tracking(ch);
      break;
  default:
      if (cmd < MAX_CMD_LIST)
        warn_busy(ch);
    break;
  }
  return TRUE;
}
