//////////////////////////////////////////////////////////////////////////
//
//      SneezyMUD 4.0 - All rights reserved, SneezyMUD Coding Team
//      "task.cc" - All functions related to tasks that keep mobs/PCs busy
//
//////////////////////////////////////////////////////////////////////////

#include "extern.h"
#include "comm.h"
#include "room.h"
#include "obj_tool.h"
#include "being.h"

void TThing::pickPulse(TBeing *ch)
{
  ch->sendTo("Hey, where'd your lockpick go?!?\n\r");
  ch->stopTask();
  return;
}

void TTool::pickPulse(TBeing *ch)
{
  int skill,difficulties;
  TRoom *temp_rp;
  roomDirData *exitp, *back;

  if (getToolType() != TOOL_LOCKPICK) {
    ch->sendTo("Hey, where'd your lockpick go?!?\n\r");
    ch->stopTask();
    return;
  }
  if (!(exitp = ch->roomp->dir_option[ch->task->status])) {
    ch->sendTo("Errr, what happened to the exit???\n\r");
    ch->stopTask();
    return;
  }
  if (!IS_SET(exitp->condition, EX_LOCKED)) {
    ch->sendTo("Some nice person seems to have unlocked it for you.\n\r");
    ch->stopTask();
    return;
  }
  skill = 2 * ch->getSkillValue(SKILL_PICK_LOCK);

  // longer they try, better chance
  ch->task->timeLeft++;
  skill += 3 * ch->task->timeLeft;

  skill += ch->plotStat(STAT_CURRENT, STAT_FOC, 3, 18, 13);
  skill += ::number(-15,15);

  addToToolUses(-1);
  if (getToolUses() <= 0) {
    act("Your $o snap in half!   Oops!",TRUE,ch,this,0,TO_CHAR);
    act("$n's $o snaps in half.",TRUE,ch,this,0,TO_ROOM);
    ch->stopTask();
    if (this != ch->unequip(ch->getPrimaryHold())) {
      vlogf(LOG_BUG, "whacked out unequip in task_picklock");
      return;
    }
    delete this;
    return;
  }
  difficulties = 3 * exitp->lock_difficulty;
  if (exitp->lock_difficulty >= 100) {
    // guaranteed unpickable, see if they recognize this fact 
    if (ch->bSuccess(skill, SKILL_PICK_LOCK)) {
      act("This lock is totally impossible to pick.  You give up.",
          TRUE, ch, this, 0, TO_CHAR);
      ch->stopTask();
      return;
    }
  }
  if ((exitp->lock_difficulty >= 100) || (difficulties > skill)) {
    if ((difficulties > (skill + 100)) && !ch->isAgile(0)) {
      act("Uhoh, $n seems to have jammed the lock!",TRUE,ch,0,0,TO_ROOM);
      ch->sendTo("Uhoh.  You seemed to have jammed the lock!\n\r");
      exitp->lock_difficulty = min(100,exitp->lock_difficulty+10);
      ch->stopTask();
      return;
    } else if (!::number(0,2)) {
      act("You wiggle $p in the lock, without luck. (yet...)",
          TRUE,ch,this,0,TO_CHAR);
    }
  } else if (ch->bSuccess(skill, SKILL_PICK_LOCK)) {
    // this used to just be an automatic for the else case
    // but what the heck, this makes it a bit harder to pick
    // and helps make pick-lock a learn-by-doing too

    REMOVE_BIT(exitp->condition, EX_LOCKED);
    if (exitp->keyword) {
      act(format("$n skillfully picks the lock of the %s.\n\r") % 
	  fname(exitp->keyword),
	  TRUE,ch,0,0,TO_ROOM);
    } else
      act("$n picks the lock.", TRUE, ch, 0, 0, TO_ROOM);
    ch->sendTo("The lock quickly yields to your skills.\n\r");

    // now for unlocking the other side, too
    temp_rp = real_roomp(exitp->to_room);
    if (temp_rp &&

        (back = temp_rp->dir_option[rev_dir[ch->task->status]]) &&
        back->to_room == ch->in_room)
      REMOVE_BIT(back->condition, EX_LOCKED);
    ch->stopTask();
  }
}

int task_picklock(TBeing *ch, cmdTypeT cmd, const char *, int pulse, TRoom *, TObj *)
{
  TThing *pick;

  if (ch->isLinkdead() || (ch->getPosition() <= POSITION_SITTING)) {
    ch->stopTask();
    return FALSE;
  }
  if (ch->utilityTaskCommand(cmd) ||
      ch->nobrainerTaskCommand(cmd))
    return FALSE;

  if (ch->in_room != ch->task->wasInRoom) {
    ch->sendTo("Hey, where'd the lock go?!?\n\r");
    act("$n looks about confused and bewildered.",TRUE,ch,0,0,TO_ROOM);
    ch->stopTask();
    return FALSE;
  }
  if (!ch->doesKnowSkill(SKILL_PICK_LOCK)) {
    ch->sendTo("I bet you wish you knew how to pick locks.\n\r");
    ch->stopTask();
    return FALSE;
  }
  int pulses_to_wait;
  switch(cmd) {
    case CMD_TASK_CONTINUE:
      if (!(pick = ch->heldInPrimHand())) {
        ch->sendTo("Hey, where'd your lockpick go?!?\n\r");
        ch->stopTask();
        return FALSE;
      }
      pulses_to_wait = 1;
      ch->task->calcNextUpdate(pulse, max(pulses_to_wait, 1) * PULSE_MOBACT);
      pick->pickPulse(ch);
      return FALSE;
    case CMD_ABORT:
    case CMD_STOP:
      act("You stop trying to pick the lock.", FALSE, ch, 0, 0, TO_CHAR);
      act("$n stops fiddling with something.", FALSE, ch, 0, 0, TO_ROOM);
      ch->stopTask();
      break;
    case CMD_TASK_FIGHTING:
      ch->sendTo("You are unable to continue lockpicking while under attack!\n\r");
      ch->stopTask();
      break;
    default:
      if (cmd < MAX_CMD_LIST)
        warn_busy(ch);
      break;                    // eat the command
  }
  return TRUE;
}
