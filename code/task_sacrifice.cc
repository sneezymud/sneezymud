#include "stdsneezy.h"
#include "obj_base_corpse.h"
#include "obj_tool.h"

int task_sacrifice(TBeing *ch, cmdTypeT cmd, const char *, int pulse, TRoom *, TObj *obj)
{
  TThing *t, *t2;
  TMonster *guard;
  TTool *totem=NULL;
  int learning = ch->getSkillValue(SKILL_SACRIFICE);
  TBaseCorpse *corpse = dynamic_cast<TBaseCorpse *>(obj);
  int clev = ch->GetMaxLevel();
  int percent = ::number(1, 100);
  int factor = ::number(5, (((clev + learning) + percent) / 2));
  int factor2 = ::number(5, (((clev + learning) + percent) / 5));

  if (!ch || !ch->task) {
    vlogf(LOG_BUG, fmt("No %s in task_sacrifice!") % (ch ? "character" : "task"));
    return FALSE;
  }

  if (ch->utilityTaskCommand(cmd) || ch->nobrainerTaskCommand(cmd)) {
    return FALSE;
  }

  if (!corpse) {
	  act("You can't find the object of the ritual! Wasn't there a corpse here?", 
      FALSE, ch, 0, 0, TO_CHAR);
	  act("$n stops singing and looks confused.", TRUE, ch, 0, 0, TO_ROOM);
    ch->stopTask();
    vlogf(LOG_BUG,
        fmt("task_sacrifice.cc: sacrifice task entered by %s without a corpse!") 
        % ch->getName());
    return TRUE;
  }

  if (ch->isLinkdead() || (ch->in_room != ch->task->wasInRoom) || 
      (ch->getPosition() < POSITION_RESTING)) {
    act("You cease the ritual sacrifice of $p.", FALSE, ch, corpse, 0, TO_CHAR);
    act("$n stops trying to sacrifice $p.", TRUE, ch, corpse, 0, TO_ROOM);
    ch->stopTask();
    if (corpse->isCorpseFlag(CORPSE_SACRIFICE))
      corpse->remCorpseFlag(CORPSE_SACRIFICE);
    return TRUE;
  }

  if (!(t = get_thing_char_using(ch, "totem", 0, FALSE, FALSE)) || 
      !(totem=dynamic_cast<TTool *>(t))) {
    ch->sendTo("You need to own a totem to perform the ritual.\n\r");
    ch->stopTask();
    if (corpse->isCorpseFlag(CORPSE_SACRIFICE))
      corpse->remCorpseFlag(CORPSE_SACRIFICE);
    return TRUE;
  }

  for (t = ch->roomp->getStuff(); t; t = t2) {
    t2 = t->nextThing;
    guard = dynamic_cast<TMonster *>(t);
    if (!guard)
      continue;
    if (!guard->isPolice() || !guard->canSee(ch) || !guard->awake())
      continue;
    int saything = (::number(0,3));
    switch (saything) {
      case 0:
	      guard->doSay("Hey! Get the hell out of here! Damn Voodoo Witch!");
	      break;
      case 1:
	      guard->doSay("Hey! Don't you have any respect for the dead?!? Get the hell out of here!");
	      break;
      case 2:
	      guard->doSay("Damn Shaman! Take your voodoo crap and PISS OFF!!!");
	      break;
      case 3:
	    guard->doSay("Hey witch!! Are you some sort of whacked out necrophiliac? Bugger off!!");
	    break;
    }
    act("You cease the ritual sacrifice of $p.", 
        FALSE, ch, corpse, 0, TO_CHAR);
    act("$n stops chanting over the corpse of $p.", 
        TRUE, ch, corpse, 0, TO_ROOM);
    ch->stopTask();
    if (corpse->isCorpseFlag(CORPSE_SACRIFICE))
      corpse->remCorpseFlag(CORPSE_SACRIFICE);
    return TRUE;
  }

  if (ch->task->timeLeft < 0){
    act("You have completed the sacrifice of $p.", 
        FALSE, ch, corpse, 0, TO_CHAR);
    act("$n has completed $s ritual sacrifice of $p.", 
        TRUE, ch, corpse, 0, TO_ROOM);
    act("Some <r>blood<z> from $p has been left behind.", 
        FALSE, ch, corpse, 0, TO_ROOM);
    act("Some <r>blood<z> from $p has been left behind for the dogs!", 
        TRUE, ch, corpse, 0, TO_CHAR);
    ch->dropPool(1, LIQ_BLOOD);
    ch->stopTask();
    delete corpse;
    corpse = NULL;
    return TRUE;
  }

  switch (cmd) {
    case CMD_TASK_CONTINUE:
      if (percent < ch->getSkillValue(SKILL_SACRIFICE)) {
        act("Your sacrifice is being accepted by the loa.", 
            FALSE, ch, 0, 0, TO_CHAR);
        ch->addToLifeforce(factor);
        ch->updatePos();
      } else {
        ch->addToLifeforce(-factor2);
        if (0 >= ch->getLifeforce()) {
          ch->setLifeforce(0);
          act("The loa demands that you cease this vain sacrifice, and you comply.", 
              FALSE, ch, 0, 0, TO_CHAR);
          ch->addToHit(-2);
          ch->updatePos();
          ch->task->timeLeft = -1;
        }
      }

      ch->task->calcNextUpdate(pulse, 2 * PULSE_MOBACT);
      totem->addToToolUses(-1);
      if (totem->getToolUses() <= 0) {
        act("Your $o has been confiscated by the loa! It must have been too weak.", 
            FALSE, ch, totem, 0, TO_CHAR);
        act("$n looks pale as $s $o shatters.", TRUE, ch, totem, 0, TO_ROOM);
        ch->stopTask();
        delete totem;
        totem = NULL;
        if (corpse->isCorpseFlag(CORPSE_SACRIFICE))
          corpse->remCorpseFlag(CORPSE_SACRIFICE);
        return TRUE;
      }
      if (10 >= corpse->obj_flags.decay_time)
        corpse->obj_flags.decay_time = 10;

      switch (ch->task->timeLeft) {
        case 2:
          act("You continue the rada song to the loa in hopes they will accept your sacrifice.", 
              FALSE, ch, 0, 0, TO_CHAR);
          act("$n sings in an unfamiliar tongue over $p.", 
              TRUE, ch, corpse, 0, TO_ROOM);
	        if (ch->bSuccess(learning, SKILL_SACRIFICE))
	          ch->task->timeLeft--;
	        break;
        case 1:
          act("Your $o's eyes glow <r>blood red<1>.", 
              FALSE, ch, totem, 0, TO_CHAR);
          act("The eyes on $n's $o begin to glow a <r>blood red<1>.", 
              TRUE, ch, totem, 0, TO_ROOM);
	        if (ch->bSuccess(learning, SKILL_SACRIFICE))
	          ch->task->timeLeft--;
	        break;
        case 0:
	        act("You continue to sing the rada song over $p.", 
              FALSE, ch, corpse, 0, TO_CHAR);
	        act("$n's ritual sacrifice causes $p's face to glow <G>pale green<1>.", 
              TRUE, ch, corpse, 0, TO_ROOM);
          if (ch->bSuccess(learning, SKILL_SACRIFICE))
            ch->task->timeLeft--;
          break;
        case -1:
          act("You feel the loa ignoring your vain attempt and feel completed to stop.",
              false, ch, 0, 0, TO_CHAR);
          act("$n has stopped $s ritual sacrifice of $p.", 
              TRUE, ch, corpse, 0, TO_ROOM);
          ch->stopTask();
          if (corpse->isCorpseFlag(CORPSE_SACRIFICE))
            corpse->remCorpseFlag(CORPSE_SACRIFICE);
          break;
        default:
          // BUG - somehow you land here if you run your life force down
          // when sacrificing and the loa forces you to stop
          act("Bug Maror if you get this message.", false, ch, 0, 0, TO_CHAR);
          vlogf(LOG_BUG, fmt("no appropriate option in switch in sacrifice.cc, timeleft value was %d")
              % ch->task->timeLeft);
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
      act("You stop your sacrifice of $p.", 
          FALSE, ch, corpse, 0, TO_CHAR);
      act("$n has stopped $s ritual sacrifice of $p.", 
          TRUE, ch, corpse, 0, TO_ROOM);
      act("Some <r>blood<z> from $p has been left behind.", 
          FALSE, ch, corpse, 0, TO_ROOM);
      act("Some <r>blood<z> from $p has been left behind for the dogs!", 
          TRUE, ch, corpse, 0, TO_CHAR);
      ch->dropPool(1, LIQ_BLOOD);
      ch->stopTask();
      delete corpse;
      corpse = NULL;
      break;
    default:
      if (cmd < MAX_CMD_LIST) {
        ch->addToLifeforce(-factor * 2);
        ch->sendTo("The loa are upset with your flagrant disregard for the houngan ways and punish you!\n\r");
      }

      warn_busy(ch);
      break;
  }
  return TRUE;
}
