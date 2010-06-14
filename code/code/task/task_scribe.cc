//////////////////////////////////////////////////////////////////////////
//
//      SneezyMUD 4.0 - All rights reserved, SneezyMUD Coding Team
//      "task.cc" - All functions related to tasks that keep mobs/PCs busy
//
//////////////////////////////////////////////////////////////////////////

#include "low.h"
#include "being.h"
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
  int readmagic = ch->getSkillValue(SKILL_READ_MAGIC);
  int factor1 = (ch->getSkillValue(SKILL_SCRIBE) * 6);
  int factor2 = (ch->getSkillValue(SKILL_SCRIBE) * 4);
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
      int scribe_ticks = ch->isImmortal() ? 1 : 7;

      ch->task->calcNextUpdate(pulse, scribe_ticks * PULSE_MOBACT);

      // init timeLeft to 0 and increment by 1 each iteration
      if (ch->task->timeLeft) {
        if (ch->task->timeLeft < (how_many * 2)) {
          ch->sendTo(format("You continue drafting your scroll%s.\n\r") % (how_many == 1 ? "" : "s"));
          ch->addToMana(-resulting);
        } else {
          obj = read_object(Obj::GENERIC_SCROLL, VIRTUAL);
          scroll_obj = dynamic_cast<TScroll *>(obj);
          if (!scroll_obj) {
            vlogf(LOG_BUG, "Error creating generic scroll for scribe skill.");
            ch->sendTo("Serious error, tell a god what you did.\n\r");
            return FALSE;
          }
          scroll_obj->swapToStrung();

          delete scroll_obj->name;
          sprintf(buf, "scroll crumpled scribed %s", discArray[which]->name);
          scroll_obj->name=mud_str_dup(buf);

          delete scroll_obj->shortDescr;
          sprintf(buf, "<o>a crumpled %s scroll<1>", discArray[which]->name);
          scroll_obj->shortDescr=mud_str_dup(buf);

          scroll_obj->setSpell(0,which);
          scroll_obj->setSpell(1, TYPE_UNDEFINED);
          scroll_obj->setSpell(2, TYPE_UNDEFINED);
          scroll_obj->setMagicLevel(ch->getClassLevel(CLASS_MAGE));

          if (ch->bSuccess(knowledge, SKILL_SCRIBE) || ch->bSuccess(readmagic, SKILL_READ_MAGIC)) {
            ch->sendTo(format("You have successfully scribed your scroll%s.\n\r") % (how_many == 1 ? "" : "s"));
            scroll_obj->setMagicLearnedness(ch->getSkillValue((ch->getSkillValue(which) > 0) ? which : SKILL_SCRIBE));
          } else {
            // failed brew, set learnedness to 0
            ch->sendTo(format("Your incompetence has resulted in %sunreadable scroll%s.\n\r") %
              ((how_many > 1) ? "an " : "") %
              ((how_many > 1) ? "s" : ""));
            scroll_obj->setMagicLearnedness(0);
          }

	        scroll_obj->obj_flags.cost = scroll_obj->suggestedPrice();
          *ch += *scroll_obj;

          sprintf(buf, "You now have %d scroll%s of %s.\n\r",
            how_many, (how_many == 1 ? "" : "s"), discArray[which]->name);
          ch->sendTo(buf);
          act("$n finishes scribing.", FALSE, ch, 0, 0, TO_ROOM);

          while ((--how_many) > 0) {
            obj = read_object(scroll_obj->number, REAL);
            s2 = dynamic_cast<TScroll *>(obj);
            s2->swapToStrung();

            delete s2->name;
            sprintf(buf, "scroll crumpled scribed %s", discArray[which]->name);
            s2->name=mud_str_dup(buf);

            delete s2->shortDescr;
            sprintf(buf, "<o>a crumpled %s scroll<1>", discArray[which]->name);
            s2->shortDescr=mud_str_dup(buf);
	  
            s2->setSpell(0, scroll_obj->getSpell(0));
            s2->setSpell(1, scroll_obj->getSpell(1));
            s2->setSpell(2, scroll_obj->getSpell(2));
            s2->setMagicLevel(scroll_obj->getMagicLevel());
            s2->setMagicLearnedness(scroll_obj->getMagicLearnedness());
            // s2->addObjStat(ITEM_NORENT);
            s2->obj_flags.cost = s2->suggestedPrice();
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

