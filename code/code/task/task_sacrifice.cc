#include <boost/format.hpp>
#include <list>
#include <memory>

#include "being.h"
#include "comm.h"
#include "enum.h"
#include "liquids.h"
#include "log.h"
#include "monster.h"
#include "obj.h"
#include "obj_base_corpse.h"
#include "obj_tool.h"
#include "parse.h"
#include "room.h"
#include "spells.h"
#include "sstring.h"
#include "task.h"
#include "thing.h"

int task_sacrifice(TBeing* ch, cmdTypeT cmd, const char*, int pulse, TRoom*,
  TObj* obj) {
  TThing* t;
  TMonster* guard;
  int learning = ch->getSkillValue(SKILL_SACRIFICE);
  TBaseCorpse* corpse = dynamic_cast<TBaseCorpse*>(obj);
  int clev = ch->GetMaxLevel();
  // random number between 50 and 175
  int factor = ::number(clev, learning + clev + 25);
  int factor2 = ::number(5, (((clev + learning) + ::number(1, 100)) / 5));

  if (!ch || !ch->task) {
    vlogf(LOG_BUG,
      format("No %s in task_sacrifice!") % (ch ? "character" : "task"));
    return false;
  }

  if (ch->utilityTaskCommand(cmd) || ch->nobrainerTaskCommand(cmd)) {
    return false;
  }

  if (!corpse) {
    act("You can't find the object of the ritual! Wasn't there a corpse here?",
      false, ch, 0, 0, TO_CHAR);
    act("$n stops singing and looks confused.", true, ch, 0, 0, TO_ROOM);
    ch->stopTask();
    vlogf(LOG_BUG,
      format(
        "task_sacrifice.cc: sacrifice task entered by %s without a corpse!") %
        ch->getName());
    return true;
  }

  if (ch->isLinkdead() || (ch->in_room != ch->task->wasInRoom) ||
      (ch->getPosition() < POSITION_RESTING)) {
    act("You cease the ritual sacrifice of $p.", false, ch, corpse, 0, TO_CHAR);
    act("$n stops trying to sacrifice $p.", true, ch, corpse, 0, TO_ROOM);
    ch->stopTask();
    if (corpse->isCorpseFlag(CORPSE_SACRIFICE))
      corpse->remCorpseFlag(CORPSE_SACRIFICE);
    return true;
  }

  auto mask = ch->getWornShamanMask();
  auto totem = ch->getHeldTotem();
  if (!mask && !totem) {
    ch->sendTo("You must be holding a totem to perform the ritual.\n\r");
    ch->stopTask();
    if (corpse->isCorpseFlag(CORPSE_SACRIFICE))
      corpse->remCorpseFlag(CORPSE_SACRIFICE);
    return true;
  }

  for (StuffIter it = ch->roomp->stuff.begin(); it != ch->roomp->stuff.end();) {
    t = *(it++);
    guard = dynamic_cast<TMonster*>(t);
    if (!guard)
      continue;
    if (!guard->isPolice() || !guard->canSee(ch) || !guard->awake())
      continue;
    int saything = (::number(0, 3));
    switch (saything) {
      case 0:
        guard->doSay("Hey! Get the hell out of here! Damn Voodoo Witch!");
        break;
      case 1:
        guard->doSay(
          "Hey! Don't you have any respect for the dead?!? Get the hell out of "
          "here!");
        break;
      case 2:
        guard->doSay("Damn Shaman! Take your voodoo crap and PISS OFF!!!");
        break;
      case 3:
        guard->doSay(
          "Hey witch!! Are you some sort of whacked out necrophiliac? Bugger "
          "off!!");
        break;
    }
    act("You cease the ritual sacrifice of $p.", false, ch, corpse, 0, TO_CHAR);
    act("$n stops chanting over the corpse of $p.", true, ch, corpse, 0,
      TO_ROOM);
    ch->stopTask();
    if (corpse->isCorpseFlag(CORPSE_SACRIFICE))
      corpse->remCorpseFlag(CORPSE_SACRIFICE);
    return true;
  }

  if (ch->task->timeLeft < 0) {
    act("You have completed the sacrifice of $p.", false, ch, corpse, 0,
      TO_CHAR);
    act("$n has completed $s ritual sacrifice of $p.", true, ch, corpse, 0,
      TO_ROOM);
    act("Some <r>blood<z> from $p has been left behind.", false, ch, corpse, 0,
      TO_ROOM);
    act("Some <r>blood<z> from $p has been left behind for the dogs!", true, ch,
      corpse, 0, TO_CHAR);
    ch->dropPool(1, LIQ_BLOOD);
    ch->stopTask();
    delete corpse;
    corpse = nullptr;
    return true;
  }

  switch (cmd) {
    case CMD_TASK_CONTINUE:
      if (ch->bSuccess(learning, SKILL_SACRIFICE)) {
        act("Your sacrifice is being accepted by the loa.", false, ch, 0, 0,
          TO_CHAR);
        ch->addToLifeforce(factor);
      } else {
        ch->addToLifeforce(-factor2);
        act("Your sacrificial attempts aren't pleasing the loa.", false, ch, 0,
          0, TO_CHAR);
        if (0 >= ch->getLifeforce()) {
          ch->setLifeforce(0);
          act(
            "The loa demands that you cease this vain sacrifice, and you "
            "comply.",
            false, ch, 0, 0, TO_CHAR);
          ch->addToHit(-2);
          // let's not allow this to stun them, cuz it deletes the task and
          // crashes the damn mud
          if (ch->getHit() < 1)
            ch->setHit(1);
          ch->task->timeLeft = -1;
        }
      }

      ch->task->calcNextUpdate(pulse, 2 * Pulse::MOBACT);

      if (!mask) {
        totem->addToToolUses(-1);
        if (totem->getToolUses() <= 0) {
          act(
            "Your $o has been confiscated by the loa! It must have been too "
            "weak.",
            false, ch, totem, 0, TO_CHAR);
          act("$n looks pale as $s $o shatters.", true, ch, totem, 0, TO_ROOM);
          ch->stopTask();
          delete totem;
          totem = nullptr;
          if (corpse->isCorpseFlag(CORPSE_SACRIFICE))
            corpse->remCorpseFlag(CORPSE_SACRIFICE);
          return true;
        }
      }

      if (10 >= corpse->obj_flags.decay_time)
        corpse->obj_flags.decay_time = 10;

      switch (ch->task->timeLeft) {
        case 2:
          act(
            "You continue the rada song to the loa in hopes they will accept "
            "your sacrifice.",
            false, ch, 0, 0, TO_CHAR);
          act("$n sings in an unfamiliar tongue over $p.", true, ch, corpse, 0,
            TO_ROOM);
          if (ch->bSuccess(learning, SKILL_SACRIFICE))
            ch->task->timeLeft--;
          break;
        case 1:
          act("Your $o's eyes glow <r>blood red<1>.", false, ch,
            mask ? mask : totem, 0, TO_CHAR);
          act("The eyes on $n's $o begin to glow a <r>blood red<1>.", true, ch,
            mask ? mask : totem, 0, TO_ROOM);
          if (ch->bSuccess(learning, SKILL_SACRIFICE))
            ch->task->timeLeft--;
          break;
        case 0:
          act("You continue to sing the rada song over $p.", false, ch, corpse,
            0, TO_CHAR);
          act(
            "$n's ritual sacrifice causes $p's face to glow <G>pale green<1>.",
            true, ch, corpse, 0, TO_ROOM);
          if (ch->bSuccess(learning, SKILL_SACRIFICE))
            ch->task->timeLeft--;
          break;
        case -1:
          act(
            "You feel the loa ignoring your vain attempt and feel compelled to "
            "stop.",
            false, ch, 0, 0, TO_CHAR);
          act("$n has stopped $s ritual sacrifice of $p.", true, ch, corpse, 0,
            TO_ROOM);
          ch->stopTask();
          if (corpse->isCorpseFlag(CORPSE_SACRIFICE))
            corpse->remCorpseFlag(CORPSE_SACRIFICE);
          break;
        default:
          // BUG - somehow you land here if you run your life force down
          // when sacrificing and the loa forces you to stop
          act("Bug Maror if you get this message.", false, ch, 0, 0, TO_CHAR);
          vlogf(LOG_BUG, format("no appropriate option in switch in "
                                "sacrifice.cc, timeleft value was %d") %
                           ch->task->timeLeft);
          ch->stopTask();
          if (corpse->isCorpseFlag(CORPSE_SACRIFICE))
            corpse->remCorpseFlag(CORPSE_SACRIFICE);
          break;
      }
      break;
    case CMD_TASK_FIGHTING:
      ch->sendTo("You can't sacrifice a corpse while under attack.\n\r");
      // don't put a break in here
    case CMD_ABORT:
    case CMD_STOP:
      act("You stop your sacrifice of $p.", false, ch, corpse, 0, TO_CHAR);
      act("$n has stopped $s ritual sacrifice of $p.", true, ch, corpse, 0,
        TO_ROOM);
      act("Some <r>blood<z> from $p has been left behind.", false, ch, corpse,
        0, TO_ROOM);
      act("Some <r>blood<z> from $p has been left behind for the dogs!", true,
        ch, corpse, 0, TO_CHAR);
      ch->dropPool(1, LIQ_BLOOD);
      ch->stopTask();
      delete corpse;
      corpse = nullptr;
      break;
    default:
      if (cmd < MAX_CMD_LIST) {
        ch->addToLifeforce(-factor * 2);
        ch->sendTo(
          "The loa are upset with your flagrant disregard for the houngan ways "
          "and punish you!\n\r");
      }

      warn_busy(ch);
      break;
  }
  return true;
}
