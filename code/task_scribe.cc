//////////////////////////////////////////////////////////////////////////
//
// SneezyMUD - All rights reserved, SneezyMUD Coding Team
//
// $Log: task_scribe.cc,v $
// Revision 1.1  1999/09/12 17:24:04  sneezy
// Initial revision
//
//
//////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////
//
//      SneezyMUD 4.0 - All rights reserved, SneezyMUD Coding Team
//      "task.cc" - All functions related to tasks that keep mobs/PCs busy
//
//////////////////////////////////////////////////////////////////////////

#include "stdsneezy.h"

int task_scribe(TBeing *ch, cmdTypeT cmd, const char *, int pulse, TRoom *, TObj *)
{
  TScroll *scribe_obj = NULL, *p2;
  TObj *obj;
  int w2;
  spellNumT which;
  int how_many;
  char buf[256];
  int knowledge = ch->getSkillValue(SKILL_SCRIBE);

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
          ch->sendTo("You continue scribing your scroll%s.\n\r",
		     (how_many == 1 ? "" : "s"));
        } else {
          // scribing has finished

          // see if a scroll exists for this item type in database
          unsigned int i;
          for (i = 0; i < obj_index.size(); i++) {
            if (obj_index[i].itemtype == ITEM_SCROLL) {
              obj = read_object(i, REAL);
              scribe_obj = dynamic_cast<TScroll *>(obj);
              if ((scribe_obj->getSpell(0) == which) &&
                  (scribe_obj->getSpell(1) == TYPE_UNDEFINED) &&
                  (scribe_obj->getSpell(2) == TYPE_UNDEFINED)) {
                break;
              } else {
                delete scribe_obj;
                scribe_obj = NULL;
              }
            }
          }
          if (scribe_obj == NULL) {
            obj = read_object(OBJ_GENERIC_SCROLL, VIRTUAL);
            scribe_obj = dynamic_cast<TScroll *>(obj);
            if (!scribe_obj) {
              vlogf(9, "Error creating generic scribe scroll.");
              ch->sendTo("Serious error, tell a god what you did.\n\r");
              return FALSE;
            }
            scribe_obj->setSpell(0, which);
            scribe_obj->setSpell(1, TYPE_UNDEFINED);
            scribe_obj->setSpell(2, TYPE_UNDEFINED);
          }
          // we now have a valid scribe_obj with values 1,2,3 set

          // set the level equal to brewer's level
          scribe_obj->setMagicLevel(ch->getClassLevel(CLASS_MAGIC_USER));

          if (bSuccess(ch, knowledge, SKILL_SCRIBE)) {
            // successful scribe, set learnedness to knowledge in the skill
            ch->sendTo("You successfully create your scroll%s.\n\r",
		       (how_many == 1 ? "" : "s"));
            scribe_obj->setMagicLearnedness( ch->getSkillValue(which) );

          } else {
            // failed brew, set learnedness to 0
            if (how_many > 1)
              ch->sendTo("Your scribing incompetence results in illegible scrolls.\n\r");
            else
              ch->sendTo("Your scribing incompetence results in an illegible scroll.\n\r");
            scribe_obj->setMagicLearnedness(0);
          }

          *ch += *scribe_obj;

          sprintf(buf, "You now have %d scroll%s of %s.\n\r",
		  how_many, (how_many == 1 ? "" : "s"), discArray[which]->name);
          ch->sendTo(buf);
          act("$n finishes scribing.", FALSE, ch, 0, 0, TO_ROOM);

          while ((--how_many) > 0) {
            // use copy constructor to duplicate it
            p2 = new TScroll(*scribe_obj);
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
      ch->sendTo("You stop scribing.\n\r");
      act("$n stops scribing.", FALSE, ch, 0, 0, TO_ROOM);
      break;
  case CMD_TASK_FIGHTING:
      ch->sendTo("You are unable to continue scribing while under attack!\n\r");
      ch->sendTo("Your manuscript was lost!\n\r");
      ch->stopTask();
      break;
  default:
      if (cmd < MAX_CMD_LIST)
        warn_busy(ch);
      break;                    // eat the command
  }
  return TRUE;
}
