//////////////////////////////////////////////////////////////////////////
//
//      SneezyMUD 4.0 - All rights reserved, SneezyMUD Coding Team
//      "task.cc" - All functions related to tasks that keep mobs/PCs busy
//
//////////////////////////////////////////////////////////////////////////

#include "stdsneezy.h"
#include "obj_potion.h"

int task_brew(TBeing *ch, cmdTypeT cmd, const char *, int pulse, TRoom *, TObj *)
{
  TPotion *potion_obj = NULL;
  int w2, i;
  spellNumT which;
  int how_many;
  int knowledge = ch->getSkillValue(SKILL_BREW);
  int factor1 = (ch->getSkillValue(SKILL_BREW) * 3);
  int factor2 = (ch->getSkillValue(SKILL_BREW) * 2);
  int resulting = ((factor1 + factor2) / 15);
  TThing *t;

  if (ch->isLinkdead()) {
    ch->stopTask();
    return FALSE;
  }
  if (ch->task->wasInRoom != ch->in_room) {
    ch->stopTask();
    return FALSE;
  }

  if (ch->utilityTaskCommand(cmd))
    return FALSE;

  switch(cmd) {
  case CMD_TASK_CONTINUE:
      how_many = ch->task->status;
      w2 = ch->task->flags;
      which = spellNumT(w2);

      ch->task->calcNextUpdate(pulse, 7 * PULSE_MOBACT);

      // init timeLeft to 0 and increment by 1 each iteration
      if (ch->task->timeLeft) {
        if (ch->task->timeLeft < (how_many * 2)) {
          ch->sendTo(fmt("You continue brewing your potion%s.\n\r") %
		     (how_many <= 5 ? "" : "s"));
	  ch->addToLifeforce(-resulting);
        } else {

          // brewing has finished
	  for (i = MIN_WEAR; i < MAX_WEAR; i++) {
	    if ((t = ch->equipment[i])) {
	      if(!potion_obj){
		if((potion_obj=dynamic_cast<TPotion *>(t)) &&
		   potion_obj->getDrinkType() != LIQ_MAGICAL_ELIXIR)
		  potion_obj=NULL;
	      }
	    }
	  }
	  for (t = ch->getStuff(); t; t = t->nextThing) {
	    if(!potion_obj){
	      if((potion_obj=dynamic_cast<TPotion *>(t)) &&
		 potion_obj->getDrinkType() != LIQ_MAGICAL_ELIXIR)
		potion_obj=NULL;
	    }
	  }

	  if(!potion_obj){
	    ch->sendTo("You can't brew without a flask of magical elixir!\n\r");
	    ch->stopTask();
	    return FALSE;
	  }

          if (ch->bSuccess(knowledge, SKILL_BREW)) {
            // successful brew, set learnedness to knowledge in the skill
            ch->sendTo(fmt("You successfully create your potion%s.\n\r") %
		       (how_many <= 5 ? "" : "s"));
	    
	    potion_obj->setDrinkUnits(how_many);
	    potion_obj->setDrinkType(spell_to_liq(which));
            ch->sendTo(fmt("You now have a potion of %s.\n\r") %
                       discArray[which]->name);
          } else {
            // failed brew, set learnedness to 0
            if (how_many > 1)
              ch->sendTo("Your brewing incompetence results in unusable potion.\n\r");
            else
              ch->sendTo("Your brewing incompetence results in unusable potion.\n\r");
            potion_obj->setDrinkUnits(::number(0, 5));
	    potion_obj->setDrinkType(LIQ_LEMONADE);
          }

          act("$n finishes brewing.", FALSE, ch, 0, 0, TO_ROOM);

          ch->stopTask();
          return FALSE;
        }
      }
      ch->task->timeLeft++;
      break;
  case CMD_ABORT:
  case CMD_STOP:
      ch->stopTask();
      ch->sendTo("You stop brewing.\n\r");
      act("$n stops brewing.", FALSE, ch, 0, 0, TO_ROOM);
      break;
  case CMD_TASK_FIGHTING:
      ch->sendTo("You are unable to continue brewing while under attack!\n\r");
      ch->sendTo("Your concoction was lost!\n\r");
      ch->stopTask();
      break;
  default:
      if (cmd < MAX_CMD_LIST)
        warn_busy(ch);
      break;                    // eat the command
  }
  return TRUE;
}
