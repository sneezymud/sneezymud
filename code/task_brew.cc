//////////////////////////////////////////////////////////////////////////
//
//      SneezyMUD 4.0 - All rights reserved, SneezyMUD Coding Team
//      "task.cc" - All functions related to tasks that keep mobs/PCs busy
//
//////////////////////////////////////////////////////////////////////////

#include "stdsneezy.h"

int task_brew(TBeing *ch, cmdTypeT cmd, const char *, int pulse, TRoom *, TObj *)
{
  TPotion *potion_obj = NULL, *p2;
  TObj *obj;
  int w2;
  spellNumT which;
  int how_many;
  char buf[256];
  int knowledge = ch->getSkillValue(SKILL_BREW);
  int factor1 = (ch->getSkillValue(SKILL_BREW) * 3);
  int factor2 = (ch->getSkillValue(SKILL_BREW) * 2);
  int resulting = ((factor1 + factor2) / 15);

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
          ch->sendTo("You continue brewing your potion%s.\n\r",
		     (how_many == 1 ? "" : "s"));
	  ch->addToLifeforce(-resulting);
        } else {

          // brewing has finished

          // see if a potion exists for this item type in database
          unsigned int i;
          for (i = 0; i < obj_index.size(); i++) {
            if (obj_index[i].itemtype == ITEM_POTION) {
              obj = read_object(i, REAL);
              potion_obj = dynamic_cast<TPotion *>(obj);
              if ((potion_obj->getSpell(0) == which) &&
                  (potion_obj->getSpell(1) == TYPE_UNDEFINED) &&
                  (potion_obj->getSpell(2) == TYPE_UNDEFINED)) {
                break;
              } else {
                delete potion_obj;
                potion_obj = NULL;
              }
            }
          }
          if (potion_obj == NULL) {
            obj = read_object(OBJ_GENERIC_POTION, VIRTUAL);
            potion_obj = dynamic_cast<TPotion *>(obj);
            if (!potion_obj) {
              vlogf(LOG_BUG, "Error creating generic brew potion.");
              ch->sendTo("Serious error, tell a god what you did.\n\r");
              return FALSE;
            }
            potion_obj->setSpell(0,which);
            potion_obj->setSpell(1, TYPE_UNDEFINED);
            potion_obj->setSpell(2, TYPE_UNDEFINED);
          }
          // we now have a valid potion_obj with values 1,2,3 set

          // set the level equal to brewer's level
          potion_obj->setMagicLevel(ch->getClassLevel(CLASS_SHAMAN));

          if (bSuccess(ch, knowledge, SKILL_BREW)) {
            // successful brew, set learnedness to knowledge in the skill
            ch->sendTo("You successfully create your potion%s.\n\r",
		       (how_many == 1 ? "" : "s"));
	    // potion_obj->setMagicLearnedness( ch->getSkillValue(which) );
	    potion_obj->setMagicLearnedness(ch->getSkillValue(SKILL_BREW));
          } else {
            // failed brew, set learnedness to 0
            if (how_many > 1)
              ch->sendTo("Your brewing incompetence results in unusable potions.\n\r");
            else
              ch->sendTo("Your brewing incompetence results in unusable potions.\n\r");
            potion_obj->setMagicLearnedness(0);
          }

          *ch += *potion_obj;

          sprintf(buf, "You now have %d potion%s of %s.\n\r",
		  how_many, (how_many == 1 ? "" : "s"), discArray[which]->name);
          ch->sendTo(buf);
	  vlogf(LOG_MISC, "%s has just brewed %d potion%s of %s", ch->getName(), how_many, (how_many == 1 ? "" : "s"), discArray[which]->name);
          act("$n finishes brewing.", FALSE, ch, 0, 0, TO_ROOM);

          while ((--how_many) > 0) {
            obj = read_object(potion_obj->number, REAL);
            p2 = dynamic_cast<TPotion *>(obj);
            p2->setSpell(0, potion_obj->getSpell(0));
            p2->setSpell(1, potion_obj->getSpell(1));
            p2->setSpell(2, potion_obj->getSpell(2));
            p2->setMagicLevel(potion_obj->getMagicLevel());
            p2->setMagicLearnedness(potion_obj->getMagicLearnedness());
            *ch += *p2;
          }
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
