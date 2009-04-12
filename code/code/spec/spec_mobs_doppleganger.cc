/*****************************************************************************

  SneezyMUD++ - All rights reserved, SneezyMUD Coding Team.

  "spec_mobs_doppleganger.cc"
  All functions and routines related to the Doppleganger Monster Routines.

  Created 10/20/99 - Lapsos(William A. Perrotto III)

******************************************************************************/

#include "handler.h"
#include "room.h"
#include "monster.h"

class mimicStructure
{
  public:
    sstring tName,
           tShort,
           tLong,
           tDesc,
           tAssumed;

};

int awardForClass(TBeing *tSucker)
{
  if (tSucker->hasClass(CLASS_CLERIC))
    return 5;

  if (tSucker->hasClass(CLASS_MAGE))
    return 4;

  if (tSucker->hasClass(CLASS_MONK))
    return 1;

  if (tSucker->hasClass(CLASS_RANGER) ||
      tSucker->hasClass(CLASS_THIEF))
    return -2;

  return 0;
}

TBeing * dopplegangerFindBetter(TBeing *tSucker, TBeing *tPatsy)
{
  int tScores[2] = {0, 0},
      tValue;

  if (!tPatsy)
    return tSucker;

  if (!tSucker)
    return tPatsy;

  if (tSucker->getClass() != tPatsy->getClass()) {
    tScores[0] += awardForClass(tSucker);
    tScores[1] += awardForClass(tPatsy);
  }

  tScores[0] += tSucker->GetMaxLevel();
  tScores[1] += tPatsy->GetMaxLevel();

  tValue = (1000 - tSucker->suggestArmor()) - tSucker->getArmor();
  tScores[0] += (tValue >= 0 ? -2 : 2);
  tValue = (1000 - tPatsy->suggestArmor()) - tPatsy->getArmor();
  tScores[1] += (tValue >= 0 ? -2 : 2);

  tScores[0] += ::number(-20, 20);
  tScores[1] += ::number(-20, 20);

  // Basically the values will range from: 
  // -23, ..., 77

  return (tScores[0] > tScores[1] ? tSucker : tPatsy);
}

TBeing * dopplegangerFindTarget(TRoom *tRoom)
{
  TBeing *tSucker,
         *tBestSucker = NULL;
  TThing *tObj=NULL;
  TRoom  *tNewRoom;


  for(StuffIter it=tRoom->stuff.begin();it!=tRoom->stuff.end() && (tObj=*it);++it)
    if ((tSucker = dynamic_cast<TBeing *>(tObj)))
      tBestSucker = dopplegangerFindBetter(tSucker, tBestSucker);

  for (dirTypeT tDir = MIN_DIR; tDir < MAX_DIR; tDir++)
    if (tRoom->dir_option[tDir] &&
        (tNewRoom = real_roomp(tRoom->dir_option[tDir]->to_room)))
      for(StuffIter it=tNewRoom->stuff.begin();it!=tNewRoom->stuff.end() && (tObj=*it);++it)
        if ((tSucker = dynamic_cast<TBeing *>(tObj)))
          tBestSucker = dopplegangerFindBetter(tSucker, tBestSucker);

  return tBestSucker;
}

int doppleganger(TBeing *ch, cmdTypeT cmd, const char *tArg, TMonster *tMyself, TObj *tObj)
{
  mimicStructure *tJob = NULL;
  TBeing         *tSucker,
                 *tPatsy;
  followData     *tFollowerA,
                 *tFollowerB;

  if (!tMyself)
    return FALSE;

  if (tMyself->act_ptr)
    tJob = static_cast<mimicStructure *>(tMyself->act_ptr);

  switch (cmd) {
    case CMD_GENERIC_CREATED:
      if (tMyself->act_ptr) {
        vlogf(LOG_PROC, format("%s created with action pointer already existing.\n\r") %  tMyself->getName());
        return FALSE;
      }

      if (!(tMyself->act_ptr = new mimicStructure())) {
        vlogf(LOG_PROC, "Failed allocation of new Mimic proc.");
        return FALSE;
      }

      vlogf(LOG_LAPSOS, "Mimic: Created");

      tJob = static_cast<mimicStructure *>(tMyself->act_ptr);
      tJob->tName    = tMyself->getName();
      tJob->tShort   = tMyself->shortDescr;
      tJob->tLong    = tMyself->getLongDesc();
      tJob->tDesc    = tMyself->getDescr();
      tJob->tAssumed = "";

      break;
    case CMD_GENERIC_DESTROYED:
      if (tMyself->act_ptr) {
        delete static_cast<mimicStructure *>(tMyself->act_ptr);
        tMyself->act_ptr = NULL;
      }

      vlogf(LOG_LAPSOS, "Mimic: Deleted");

      break;
    case CMD_MOB_MOVED_INTO_ROOM:
      if (tMyself->act_ptr && !tJob->tAssumed.empty() &&
          (tSucker = get_char_room_vis(tMyself, tJob->tAssumed, NULL, EXACT_YES)) &&
          tSucker->isPc() && !tSucker->isImmortal()) {
        if (tSucker->master && tSucker->master->isPc() && !tSucker->isAffected(AFF_CHARM)) {
          tSucker->sendTo("Looking upon yourself you lose your focus...\n\r");
          tSucker->stopFollower(TRUE);

          tSucker->stopTask();
        } else if (tSucker->followers) {
          for (tFollowerA = tSucker->followers; tFollowerA; tFollowerA = tFollowerB) {
            tFollowerB = tFollowerA->next;
            tPatsy = tFollowerA->follower;

            if (tPatsy->isPc() && !tPatsy->isAffected(AFF_CHARM)) {
              act("You stop following $n seeing that there is now a pair of them.",
                  TRUE, tSucker, NULL, tPatsy, TO_VICT);
              tPatsy->stopFollower(TRUE);
            }
          }
        }
      }

      vlogf(LOG_LAPSOS, "Mimic: Moved");

      break;
    case CMD_GENERIC_PULSE:
      if (tMyself->act_ptr) {
        if (tJob->tAssumed.empty()) {
          if ((tSucker = dopplegangerFindTarget(tMyself->roomp))) {
            tMyself->swapToStrung();

            delete [] tMyself->name;
            tMyself->name = mud_str_dup(tSucker->name);

            delete [] tMyself->shortDescr;
            tMyself->shortDescr = mud_str_dup(tSucker->shortDescr);

            delete [] tMyself->player.longDescr;
            tMyself->player.longDescr = mud_str_dup(tSucker->getLongDesc());

            delete [] tMyself->descr;
            tMyself->descr = mud_str_dup(tSucker->descr);

            tMyself->fixLevels(tSucker->GetMaxLevel());
            tMyself->setMult(2.5);
            tMyself->setRace(tSucker->getRace());
            tMyself->setSex(tSucker->getSex());
            tMyself->setHeight(tSucker->getHeight());
            tMyself->setWeight(tSucker->getWeight());

            // Verify stats and other such things here.

            tMyself->setExp(tMyself->determineExp());
          }
        } else {
	  // Verify our 'sucker' is still around and findable.
          // If not then nuke tAssumed and roll over.
          // else just verify a few things and act accordingly.
        }
      }

      break;
    case CMD_TELL:
    case CMD_SAY:
    case CMD_WHISPER:
    case CMD_ASK:
      // Allow creators to do:
      // mimic target <player> to set a target
      // mimic ignore to de-assume current target
      // mimic boost <value> to 'increase' mimic's level by <value>
      // mimic 
      break;
    default:
      break;
  }

  return FALSE;
}
