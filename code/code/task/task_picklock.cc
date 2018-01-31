#include "extern.h"
#include "comm.h"
#include "room.h"
#include "obj_tool.h"
#include "being.h"

static void pick_pulse(TBeing *ch, TThing *pick)
{
  TTool *tool = dynamic_cast<TTool *>(pick);
  if (!tool || tool->getToolType() != TOOL_LOCKPICK) {
    ch->sendTo("Hey, where'd your lockpick go?!?\n\r");
    ch->stopTask();
    return;
  }

  roomDirData *exit;
  if (!(exit = ch->roomp->dir_option[ch->task->status])) {
    ch->sendTo("Errr, what happened to the exit???\n\r");
    ch->stopTask();
    return;
  }

  if (!IS_SET(exit->condition, EXIT_LOCKED)) {
    ch->sendTo("Some nice person seems to have unlocked it for you.\n\r");
    ch->stopTask();
    return;
  }

  ch->task->timeLeft++;
  int skill = 2 * ch->getSkillValue(SKILL_PICK_LOCK)
      + 3 * ch->task->timeLeft // longer they try, better chance
      + ch->plotStat(STAT_CURRENT, STAT_FOC, 3, 18, 13)
      + ::number(-15,15);

  if (!tool->addToToolUses(-1)) {
    act("Your $o snap in half!   Oops!", true, ch, tool, nullptr, TO_CHAR);
    act("$n's $o snaps in half.", true, ch, tool, nullptr, TO_ROOM);
    ch->stopTask();
    ch->unequip(ch->getPrimaryHold());
    delete tool;
    return;
  }

  bool pickable = !IS_SET(exit->condition, EXIT_JAMMED) && exit->lock_difficulty < 100;

  if (!pickable && ch->bSuccess(skill, SKILL_PICK_LOCK)) {
    act("This lock is totally impossible to pick.  You give up.", true, ch, tool, nullptr, TO_CHAR);
    ch->stopTask();
    return;
  }

  int difficulty = 3 * exit->lock_difficulty;
  if ((!pickable || difficulty > skill) &&
      difficulty > (skill + 100) &&
      !ch->isDextrous()) {
    act("Uhoh, $n seems to have jammed the lock!", true, ch, nullptr, nullptr, TO_ROOM);
    ch->sendTo("Uhoh.  You seemed to have jammed the lock!\n\r");
    SET_BIT(exit->condition, EXIT_JAMMED);
    ch->stopTask();
    return;
  }

  if (pickable && ch->bSuccess(skill, SKILL_PICK_LOCK)) {
    REMOVE_BIT(exit->condition, EXIT_LOCKED);
    sstring msg = !exit->keyword.empty() ?
        sstring(format("$n skillfully picks the lock of the %s.") % fname(exit->keyword)) :
        sstring("$n picks the lock.");
    act(msg, true, ch, nullptr, nullptr, TO_ROOM);
    ch->sendTo("The lock quickly yields to your skills.\n\r");

    // now for unlocking the other side, too
    TRoom *other = real_roomp(exit->to_room);
    roomDirData *back;
    if (other &&
        (back=other->dir_option[rev_dir[ch->task->status]]) &&
        back->to_room == ch->in_room)
      REMOVE_BIT(back->condition, EXIT_LOCKED);
    ch->stopTask();
    return;
  }

  if (!::number(0,2))
    act("You wiggle $p in the lock, without luck. (yet...)", true, ch, tool, nullptr, TO_CHAR);
}


int task_picklock(TBeing *ch, cmdTypeT cmd, const char *, int pulse, TRoom *, TObj *)
{
  if (ch->isLinkdead() || (ch->getPosition() <= POSITION_SITTING)) {
    ch->stopTask();
    return FALSE;
  }

  if (ch->utilityTaskCommand(cmd) || ch->nobrainerTaskCommand(cmd))
    return FALSE;

  if (ch->in_room != ch->task->wasInRoom) {
    ch->sendTo("Hey, where'd the lock go?!?\n\r");
    act("$n looks about, confused and bewildered.",TRUE,ch,0,0,TO_ROOM);
    ch->stopTask();
    return FALSE;
  }

  if (!ch->doesKnowSkill(SKILL_PICK_LOCK)) {
    ch->sendTo("I bet you wish you knew how to pick locks.\n\r");
    ch->stopTask();
    return FALSE;
  }

  switch(cmd) {
    case CMD_TASK_CONTINUE:
      TThing *pick;
      if (!(pick = ch->heldInPrimHand())) {
        ch->sendTo("Hey, where'd your lockpick go?!?\n\r");
        ch->stopTask();
        return FALSE;
      }
      ch->task->calcNextUpdate(pulse, Pulse::MOBACT);
      pick_pulse(ch, pick);
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
