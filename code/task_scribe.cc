//////////////////////////////////////////////////////////////////////////
//
//      SneezyMUD 4.0 - All rights reserved, SneezyMUD Coding Team
//      "task.cc" - All functions related to tasks that keep mobs/PCs busy
//
//////////////////////////////////////////////////////////////////////////

#include "stdsneezy.h"
#include "obj_scroll.h"

int task_scribe(TBeing *ch, cmdTypeT cmd, const char *, int pulse, TRoom *, TObj *)
{
  TScroll *scroll_obj = NULL, *s2;
  TObj *obj;
  int w2;
  spellNumT which;
  int how_many;
  char buf[256];
  int knowledge = ch->getSkillValue(SKILL_SCRIBE);
  int factor1 = (ch->getSkillValue(SKILL_SCRIBE) * 3);
  int factor2 = (ch->getSkillValue(SKILL_SCRIBE) * 2);
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
          ch->sendTo("You continue drafting your scroll%s.\n\r",
		     (how_many == 1 ? "" : "s"));
	  ch->addToMana(-resulting);
        } else {
          unsigned int i;
          for (i = 0; i < obj_index.size(); i++) {
            if (obj_index[i].itemtype == ITEM_SCROLL) {
              obj = read_object(i, REAL);
              scroll_obj = dynamic_cast<TScroll *>(obj);
              if ((scroll_obj->getSpell(0) == which) &&
                  (scroll_obj->getSpell(1) == TYPE_UNDEFINED) &&
                  (scroll_obj->getSpell(2) == TYPE_UNDEFINED)) {
                break;
              } else {
                delete scroll_obj;
                scroll_obj = NULL;
              }
            }
          }
          if (scroll_obj == NULL) {
            obj = read_object(OBJ_GENERIC_SCROLL, VIRTUAL);
            scroll_obj = dynamic_cast<TScroll *>(obj);
            if (!scroll_obj) {
              vlogf(LOG_BUG, "Error creating generic scroll for scribe skill.");
              ch->sendTo("Serious error, tell a god what you did.\n\r");
              return FALSE;
            }
            scroll_obj->setSpell(0,which);
            scroll_obj->setSpell(1, TYPE_UNDEFINED);
            scroll_obj->setSpell(2, TYPE_UNDEFINED);
          }
          scroll_obj->setMagicLevel(ch->getClassLevel(CLASS_MAGE));

          if (bSuccess(ch, knowledge, SKILL_SCRIBE)) {
            ch->sendTo("You have successfully scribed your scroll%s.\n\r",
		       (how_many == 1 ? "" : "s"));
	    if (ch->getSkillValue(which) > 0) {
	      scroll_obj->setMagicLearnedness(ch->getSkillValue(which));
	    } else {
	      scroll_obj->setMagicLearnedness(ch->getSkillValue(SKILL_SCRIBE));
	    }
          } else {
            // failed brew, set learnedness to 0
            if (how_many > 1)
              ch->sendTo("Your incompetence has resulted in unreadable scrolls.\n\r");
            else
              ch->sendTo("Your incompetence has resulted in unreadable scrolls.\n\r");
            scroll_obj->setMagicLearnedness(0);
          }

          *ch += *scroll_obj;

          sprintf(buf, "You now have %d scroll%s of %s.\n\r",
		  how_many, (how_many == 1 ? "" : "s"), discArray[which]->name);
          ch->sendTo(buf);
	  vlogf(LOG_MISC, "%s has just scribed %d scroll%s of %s", ch->getName(), how_many, (how_many == 1 ? "" : "s"), discArray[which]->name);
          act("$n finishes scribing.", FALSE, ch, 0, 0, TO_ROOM);

          while ((--how_many) > 0) {
            obj = read_object(scroll_obj->number, REAL);
            s2 = dynamic_cast<TScroll *>(obj);
            s2->setSpell(0, scroll_obj->getSpell(0));
            s2->setSpell(1, scroll_obj->getSpell(1));
            s2->setSpell(2, scroll_obj->getSpell(2));
            s2->setMagicLevel(scroll_obj->getMagicLevel());
            s2->setMagicLearnedness(scroll_obj->getMagicLearnedness());
	    s2->addObjStat(ITEM_NORENT);
	    s2->obj_flags.cost = 0;
            *ch += *s2;
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
      ch->sendTo("You stop scribing.\n\r");
      act("$n stops scribing.", FALSE, ch, 0, 0, TO_ROOM);
      break;
  case CMD_TASK_FIGHTING:
      ch->sendTo("You are unable to continue scribing while under attack!\n\r");
      ch->sendTo("Your hand was jarred and the scribe was lost!\n\r");
      ch->stopTask();
      break;
  default:
      if (cmd < MAX_CMD_LIST)
        warn_busy(ch);
      break;                    // eat the command
  }
  return TRUE;
}

