/*****************************************************************************

  SneezyMUD++ - All rights reserved, SneezyMUD Coding Team.

  "task_whittle.cc"
  All functions and routines related to the whittle task.

  Created 5/24/99 - Lapsos(William A. Perrotto III)

******************************************************************************/

#include "stdsneezy.h"
#include "task_whittle.h"
#include "obj_bow.h"
#include "obj_organic.h"
#include "obj_arrow.h"
#include "obj_general_weapon.h"



map<unsigned long int, taskWhittleEntry>whittleItems;

void initWhittle()
{
  int i = 0;

  // Dummy *** DO NOT TOUCH/MOVE/REMOVE THIS ENTRY! ***
  whittleItems[i++]("big-error dummy empty nada zip",
                    CLASS_RANGER, 101, -1, false, WHITTLE_ERROR);

  // Please Read:
  // Formating for bows and arrows is very strict:
  // the 2nd phrase of the first argument MUST be a type:
  //   "hunting-arrow ->arrow<- long"
  //   "squabble-crossbow ->crossbow<- bow long"
  // This is used to generate a short/long for the object so
  // MUST BE FORMATED THIS WAY FOR PROPER IN GAME FORMAT.
  //   an oak ->arrow<- lies on the ground here.
  //   a cedar ->crossbow<- has been left on the ground here.

  // Format:
  // ("OBJECT_NAME", MUST_HAVE_CLASS(-1 for any),
  //  REQUIRED_WHITTLE_KNOWLEDGE, OBJECT_VNUM, SAVE_UNFINISHED,
  //  WHITTLE_TYPE)
  // (taskWhittleEntry)(const char *, int, int, int, int, whittleTypeT);
  // "OBJECT NAME"   :  is used to find out which object to make.
  // MUST_HAVE_CLASS :  ask and i'll shoot you.
  // REQUIRED_WHITTLE_KNOWELDGE :  Must Have Minimum level.
  // OBJECT_VNUM     :  ask and i'll shoot you.
  // SAVE_UNFINISHED :  Don't use this for non-arrows/non-bows.
  // WHITTLE_TYPE    :  see the .h file for a list.  this is difficulty.

  // Extra Notes:

  // Arrows
  whittleItems[i++]("hunting-arrow arrow long",
                    -1, 30, 166, true, WHITTLE_TIMECONSUMING);
  whittleItems[i++]("fighting-arrow arrow short",
                    -1, 30, 167, true, WHITTLE_TIMECONSUMING);
  whittleItems[i++]("melee-quarrel quarrel long",
                    -1, 30, 168, true, WHITTLE_TIMECONSUMING);
  whittleItems[i++]("common-quarrel quarrel short",
                    -1, 30, 169, true, WHITTLE_TIMECONSUMING);
  whittleItems[i++]("totem small wooden",
                    CLASS_SHAMAN, 1, 31395, true, WHITTLE_TIMECONSUMING);

  // Bows
  whittleItems[i++]("hunting-bow bow long",
                    -1, 70, 170, true, WHITTLE_STANDARD);
  whittleItems[i++]("fighting-bow bow short",
                    -1, 70, 171, true, WHITTLE_STANDARD);
  whittleItems[i++]("melee-crossbow crossbow long",
                    -1, 70, 172, true, WHITTLE_STANDARD);
  whittleItems[i++]("common-crossbow crossbow short",
                    -1, 70, 173, true, WHITTLE_STANDARD);

  // Other
  whittleItems[i++]("small-boat simple boat toy",
                    -1,  1,  188, false, WHITTLE_DELICATE);
  whittleItems[i++]("wooden-staff training staff",
                    -1, 10,  177, false, WHITTLE_GENERAL);
  whittleItems[i++]("wooden-sword training sword",
                    -1, 10, 329, false, WHITTLE_GENERAL);
  whittleItems[i++]("chair-wooden sturdy wooden",
                    -1, 40,  174, false, WHITTLE_EASY);
  whittleItems[i++]("wooden-chest small simple chest",
                    -1, 20,  185, false, WHITTLE_GENERAL);
  whittleItems[i++]("wooden-shield small simple shield",
                    -1, 70,  178, false, WHITTLE_HARD);
  whittleItems[i++]("small-wood-canister carved wood canister",
                    CLASS_RANGER, 45, -1  /* 556 */, false, WHITTLE_GENERAL);
  whittleItems[i++]("large-wood-canister carved wood canister",
                    CLASS_RANGER, 45, -1 /* 557 */, false, WHITTLE_GENERAL);
  whittleItems[i++]("small-box box container wood",
                    -1,  1,  186, false, WHITTLE_EASY);
  whittleItems[i++]("wood-ring ring small simple",
                    -1,  8,  179, false, WHITTLE_VALUABLE);
  whittleItems[i++]("wood-club light small",
                    -1, 50,  176, false, WHITTLE_HARD);
  whittleItems[i++]("minature-figurine figurine small delicate",
                    -1, 99,  191, false, WHITTLE_DELICATE);
  whittleItems[i++]("tiny-statuette statuette small declicate",
                    -1, 99,  190, false, WHITTLE_INVOLVED);
  whittleItems[i++]("simple-dart dart wooden",
                    -1, 30,  175, false, WHITTLE_VALUABLE);
  whittleItems[i++]("simple-pipe pipe wooden",
                    -1, 40,  180, false, WHITTLE_HARD);
  whittleItems[i++]("wooden-pen pen",
                    -1, 20,  181, false, WHITTLE_VALUABLE);
  whittleItems[i++]("toothpick",
                    -1,  1,  182, false, WHITTLE_VALUABLE);
  whittleItems[i++]("wood length sturdy",
                    -1, 20,  189, false, WHITTLE_EASY);
  whittleItems[i++]("stick walking",
                    -1, 40,  183, false, WHITTLE_INVOLVED);
  whittleItems[i++]("totem wooden",
                    -1, 80,  184, false, WHITTLE_DELICATE);
  whittleItems[i++]("wooden-dagger dagger small",
                    -1, 20,  187, false, WHITTLE_HARD);
  whittleItems[i++]("miniature-idol-moath",
                    -1, 99, 192, false, WHITTLE_INVOLVED);
  whittleItems[i++]("miniature-idol-lapsos",
                    -1, 99, 193, false, WHITTLE_INVOLVED);
  whittleItems[i++]("miniature-idol-mithros",
                    -1, 99, 194, false, WHITTLE_INVOLVED);
  whittleItems[i++]("miniature-idol-gringar",
                    -1, 99, 195, false, WHITTLE_INVOLVED);
  whittleItems[i++]("miniature-idol-peel",
                    -1, 99, 197, false, WHITTLE_INVOLVED);
  whittleItems[i++]("pole-fishingpole-very-nice", 
                    -1, 60, 13862, false, WHITTLE_GENERAL);
  
  whittleItems[i++]("elongated-strip strip wood", // For 'create splint' later on
                    -1, 30,  -1, false, WHITTLE_EASY);
}

void stop_whittling(TBeing *ch)
{
  if (ch->task->obj)
    if (whittleItems[ch->task->flags].affectValue) {
      ch->sendTo("Even if partly finished this item is still good.\n\r");
      act("$n stores $s unfinished object.",
          FALSE, ch, NULL, NULL, TO_ROOM);
      *ch += *ch->task->obj;
      ch->task->obj = NULL;
    } else {
      ch->sendTo("Unfortunatly this object is useless if not finished, so you trash it.\n\r");
      act("$n throws $s unfinished object away.",
          FALSE, ch, NULL, NULL, TO_ROOM);
      delete ch->task->obj;
      ch->task->obj = NULL;
    }

  ch->sendTo("You stop what you are doing and look around.\n\r");
  act("$n stops what $e is doing and looks around.",
      FALSE, ch, NULL, NULL, TO_ROOM);
  ch->stopTask();
}

void task_whittlePulse(TBeing *ch, TArrow *tArrow, whittlePulseT tWhitLevel)
{
  if (!tArrow)
    return;

  double tValue,
         tTemp,
         tLevel = min(50, (int)ch->GetMaxLevel()),
         tSkill = max( 1, (int)ch->getSkillValue(SKILL_WHITTLE));

  switch (tWhitLevel) {
    case WHITTLE_PULSE_ZEROOUT: // Zero everything out.
      tArrow->setArrowHead(0);
      tArrow->setArrowHeadMat(0);
      tArrow->setArrowFlags(0);
      tArrow->setMaxSharp(1);
      tArrow->setCurSharp(1);
      tArrow->setMaxStructPoints(1);
      tArrow->setStructPoints(1);
      tArrow->setWeapDamLvl(1);
      tArrow->setWeapDamDev(1);
      break;
    case WHITTLE_PULSE_CARVED: // Set Structure
      tValue = (tSkill / 100);
      tValue = max(0.0, min(50.0, ((tLevel * tValue) + ::number(-5, 5))));

      // max((getMaxStructPoints()-10), 0) * 2.0 / 3.0;
      tValue = (((tValue * 3.0) / 2.0) + 10);

      tArrow->setMaxStructPoints((sh_int)tValue);
      tTemp  = (tValue * (tSkill / 100));
      tTemp += (double)::number(0, (int)((tValue - tTemp) / 2));
      tArrow->setStructPoints((sh_int)tTemp);
      break;
    case WHITTLE_PULSE_SCRAPED: // Set Sharpness
      tValue = (tSkill / 100);
      tValue = max(0.0, min(50.0, ((tLevel * tValue) + ::number(-5, 5))));

      // max((getMaxSharp()-10), 0) * 2.0 / 3.0
      tValue = (((tValue * 2.0) / 2.0) + 10);

      tArrow->setMaxSharp((int)tValue);
      tTemp  = (tValue * (tSkill / 100));
      tTemp += (double)::number(0, (int)((tValue - tTemp) / 2));
      tArrow->setCurSharp((int)tTemp);
      break;
    case WHITTLE_PULSE_SMOOTHED: // Set Damage&Damage Deviation
      tValue = (tSkill / 100);
      tValue = max(0.0, min(50.0, ((tLevel * tValue) + ::number(-5, 5))));

      // getWeapDamLvl() / 4.0
      tValue = (tValue * 4.0);

      tArrow->setWeapDamLvl((int)tValue);
      tTemp  = max(0.0, (tValue - (tSkill * (tSkill / 100))));
      tTemp -= max(0.0, (double)::number(0, (int)((tValue - tTemp) / 2)));
      tArrow->setWeapDamDev((int)tTemp);
      break;
    default:
      vlogf(LOG_BUG, fmt("task_shittlePulse(TArrow) called with invalid tWhitLevel.  [%d]") % 
            tWhitLevel);
      break;
  }
}

void task_whittlePulse(TBeing *ch, TBow *tBow, whittlePulseT tWhitLevel)
{
  if (!tBow)
    return;

  double tValue,
         tTemp,
         tLevel = min(50, (int) ch->GetMaxLevel()),
         tSkill = max( 1, (int) ch->getSkillValue(SKILL_WHITTLE));

  switch (tWhitLevel) {
    case WHITTLE_PULSE_ZEROOUT: // Zero everything out.
      tBow->setBowFlags(0);
      tBow->addBowFlags(BOW_STRING_BROKE);
      tBow->setMaxRange(1);
      break;
    case WHITTLE_PULSE_CARVED: // Set Structure
      tValue = (tSkill / 100);
      tValue = max(0.0, min(50.0, ((tLevel * tValue) + ::number(-5, 5))));

      // max((getMaxStructPoints()-10), 0) * 2.0 / 3.0;
      tValue = (((tValue * 3.0) / 2.0) + 10);

      tBow->setMaxStructPoints((int)tValue);
      tTemp  = (tValue * (tSkill / 100));
      tTemp += (double)::number(0, (int)((tValue - tTemp) / 2));
      tBow->setStructPoints((int)tTemp);
      break;
    case WHITTLE_PULSE_SCRAPED: // Set 1/2 Range
      tValue = (tSkill / 100);
      tValue = max(0.0, min(50.0, ((tLevel * tValue) + ::number(-5, 5))));

      tValue = min(5.0, max(1.0, ((10 * (tValue / 50)) / 2)));

      tBow->setMaxRange((int)tValue);
      break;
    case WHITTLE_PULSE_SMOOTHED: // Set 1/2 Range
      tValue = (tSkill / 100);
      tValue = max(0.0, min(50.0, ((tLevel * tValue) + ::number(-5, 5))));

      tValue = min(5.0, max(1.0, ((10 * (tValue / 50)) / 2)));

      tBow->setMaxRange((int)(tValue + tBow->getMaxRange()));
      break;
    default:
      vlogf(LOG_BUG, fmt("task_shittlePulse(TBow) called with invalid tWhitLevel.  [%d]") % 
            tWhitLevel);
      break;
  }
}

const char *tailMessages[] =
{
  "has been left on the $g here.",
  "rests upon the $g.",
  "lies upon the $g here.",
  "sits upon the $g here.",
  "is here on the $g."
};

// tStPost will contain the pre-word, such as 'a' or 'an', and also
// the intial color index should it exist.  So:
//   <R>an oak log<z>
// Will become:
//   <R>an oak arrow<z>
//   <R>An oak arrow sits upon the $g here.<z>
void task_whittleSetupObject(TBeing *ch, TObj *tObj, TOrganic *tWood, int tIndex)
{
  TArrow *tArrow = NULL;
  TBow   *tBow   = NULL;
  sstring  tStPost, tStWood, tStObject, tStString, tString;

  if ((tArrow = dynamic_cast<TArrow *>(tObj)) ||
      (tBow   = dynamic_cast<TBow   *>(tObj))) {
    tObj->swapToStrung();

    tStString = tWood->getName();
    tStPost=tStString.word(0);
    tStWood=tStString.word(1);
    tStObject = whittleItems[tIndex].getName(true);

    tString=fmt("%s %s %s<z>") %
      tStPost % tStWood % tStObject;
    delete [] tObj->shortDescr;
    tObj->shortDescr = mud_str_dup(tString);

    tString=fmt("%s %s %s") %
            tStObject % tStWood % whittleItems[tIndex].getName(false);;
    delete [] tObj->name;
    tObj->name = mud_str_dup(tString);

    tString=fmt("%s %s %s %s<z>") %
      sstring(tStPost).cap() % tStWood %
      tStObject % tailMessages[::number(0, 4)];
    delete [] tObj->descr;
    tObj->descr = mud_str_dup(tString);

    tObj->setWeight((whittleItems[tIndex].weiSize / 1.10));
    tObj->setVolume((int)(whittleItems[tIndex].volSize / 1.10));

    if (tBow) {
      task_whittlePulse(ch, tBow, WHITTLE_PULSE_ZEROOUT);
    }

    if (tArrow) {
      task_whittlePulse(ch, tArrow, WHITTLE_PULSE_ZEROOUT);
    }
  }

  tObj->setMaterial(tWood->getMaterial());
}

bool task_whittleCreateNew(TBeing *ch, sstring tStWood, int tIndex)
{
  TOrganic *tWood        = NULL,
           *tOldWood     = NULL;
  TThing   *tObjTemp     = (ch ? ch->getStuff() : NULL),
           *tObjTempNext = (ch ? ch->getStuff() : NULL);
  bool      realCreate   = (ch->task ? true : false),
            deleteOld    = false;
  double    totalWood[2] = {0, 0};

  while ((tObjTemp = searchLinkedListVis(ch, tStWood, tObjTemp))) {
    if ((tWood = dynamic_cast<TOrganic *>(tObjTemp))) {
      if (tWood->getOType() != ORGANIC_WOOD) {
        tWood = NULL;
        continue;
      }

      totalWood[0] += tWood->getWeight();
      totalWood[1] += tWood->getVolume();
    }

    tObjTemp = tObjTemp->nextThing;
  }

  if (!tWood)
    return false;

  double newWeight = whittleItems[tIndex].weiSize,
         newVolume = whittleItems[tIndex].volSize;
  int    totalUsed = 0;

  if (totalWood[0] >= newWeight && totalWood[1] >= newVolume) {
    if (!realCreate)
      return true;

    tObjTemp = ch->getStuff();

    while ((tObjTemp = searchLinkedListVis(ch, tStWood, tObjTempNext))) {
      tObjTempNext = tObjTemp->nextThing;

      if ((tWood = dynamic_cast<TOrganic *>(tObjTemp))) {
        if (tWood->getOType() != ORGANIC_WOOD)
          continue;

        if (!tOldWood)
          tOldWood = tWood;

        act("You use $p for your object.",
            FALSE, ch, tWood, NULL, TO_CHAR);

        if (newWeight > 0)
          newWeight -= tWood->getWeight();

        if (newVolume > 0)
          newVolume -= tWood->getVolume();

        if (newWeight > -1 || newVolume > -1) {
          act("$p is pretty much used up so you scrap what's left.",
              FALSE, ch, tWood, NULL, TO_CHAR);
          totalUsed++;

          --(*tWood);

          if (deleteOld)
            delete tWood;
          else
            deleteOld = true;
        } else {
          tWood->setWeight(-(newWeight));
          tWood->setVolume((int)(-newVolume));
          act("You cut a chunk of wood off of $p to create the object.",
              FALSE, ch, tWood, NULL, TO_CHAR);

          break;
        }
      }
    }

    if (!(ch->task->obj = read_object(real_object(whittleItems[tIndex].itemVnum), REAL))) {
      ch->sendTo("Something bad happened.  Tell a god.\n\r");
      vlogf(LOG_BUG, fmt("Player in whittle got to item that doesn't exist!  [%d]") % 
            whittleItems[tIndex].itemVnum);
      return false;
    }

    task_whittleSetupObject(ch, ch->task->obj, tOldWood, tIndex);

    if (tOldWood && deleteOld) {
      delete tOldWood;
      tOldWood = NULL;
    }

    return true;
  }

  return false;
}

int checkForSlipup(TBeing *ch)
{
  int        tCheck   = min(100, max(0, (ch->getSkillValue(SKILL_WHITTLE) - 10)));
  TThing     *tWeapon = NULL;
  TGenWeapon *tWeap   = NULL;

  tWeap = dynamic_cast<TGenWeapon *>((tWeapon = ch->heldInPrimHand()));

  if (tCheck < ::number(-50, 100) && !ch->isImmortal()) {
    act("You slip up and cut yourself on $p.",
        FALSE, ch, tWeapon, NULL, TO_CHAR);
    act("$n slips up and cuts $mself on $p",
        FALSE, ch, tWeapon, NULL, TO_ROOM);

    if (tWeap->getCurSharp() > 4)
      tWeap->addToCurSharp(-3);

    ch->dropBloodLimb((ch->isRightHanded() ? WEAR_FINGER_L : WEAR_FINGER_R));

    if (ch->reconcileDamage(ch, 5+(min(20, tWeap->getCurSharp()) / 2), SKILL_WHITTLE) == -1) {
      if (ch->task->obj) {
        delete ch->task->obj;
        ch->task->obj = NULL;
      }

      ch->stopTask();
      ch->doSave(SILENT_YES);
      return DELETE_THIS;
    }

    return TRUE;
  }

  if (tWeap->getCurSharp() && tCheck < ::number(0, 100) && !ch->isImmortal())
    if (tWeap->getCurSharp() > 4)
      tWeap->addToCurSharp(-3);

  return FALSE;
}

// This process is supposed to take a TON of time.  Please keep it that way.
int task_whittleObject(TBeing *ch, sstring tStWood)
{
  int    nRc = TRUE;
    // dRc;
  TArrow *tArrow = dynamic_cast<TArrow *>(ch->task->obj);
  TBow   *tBow   = dynamic_cast<TBow   *>(ch->task->obj);

  //  Dash requested damage from whittle be removed
  //
  //  if ((dRc = checkForSlipup(ch)) != FALSE)
  //  return dRc;

  ch->learnFromDoingUnusual(LEARN_UNUSUAL_NORM_LEARN, SKILL_WHITTLE, 1);

  double objSize = (((ch->task->obj->getWeight() +
                      ch->task->obj->getVolume()) / 25) *
                    whittleItems[ch->task->flags].itemType);
  double process = ((++ch->task->timeLeft) / objSize);

  if (process >= 0.00 && process <= 0.40) {
    act("You gently carve a part of $p.",
        FALSE, ch, ch->task->obj, NULL, TO_CHAR);
  }

  if (process >= 0.41 && process <= 0.80) {
    if (tArrow) {
      tArrow->addArrowFlags(ARROW_CARVED);
      task_whittlePulse(ch, tArrow, WHITTLE_PULSE_CARVED);
    }
    if (tBow) {
      tBow->addBowFlags(BOW_CARVED);
      task_whittlePulse(ch, tBow, WHITTLE_PULSE_CARVED);
    }

    act("You gently scrape the nodes off of $p.",
        FALSE, ch, ch->task->obj, NULL, TO_CHAR);
  }

  if (process >= 0.81 && process <= 0.99) {
    if (tArrow) {
      tArrow->addArrowFlags(ARROW_SCRAPED);
      task_whittlePulse(ch, tArrow, WHITTLE_PULSE_SCRAPED);
    }
    if (tBow) {
      tBow->addBowFlags(BOW_SCRAPED);
      task_whittlePulse(ch, tBow, WHITTLE_PULSE_SCRAPED);
    }

    act("You gently smooth $p.",
        FALSE, ch, ch->task->obj, NULL, TO_CHAR);
  }

  if (process >= 1.00) {
    if (tArrow) {
      tArrow->addArrowFlags(ARROW_SMOOTHED);
      task_whittlePulse(ch, tArrow, WHITTLE_PULSE_SMOOTHED);
    }
    if (tBow) {
      tBow->addBowFlags(BOW_SMOOTHED);
      task_whittlePulse(ch, tBow, WHITTLE_PULSE_SMOOTHED);
    }

    act("You are done with $p.",
        FALSE, ch, ch->task->obj, NULL, TO_CHAR);
    act("$n finishes whittling $p.",
        FALSE, ch, ch->task->obj, NULL, TO_ROOM);
    TThing *tThing;
    tThing = ch->task->obj;
    *ch += *tThing;
    ch->task->obj = NULL;

    if (tArrow) {
      if (tStWood.empty()) {
        ch->sendTo("You finish your modifications and stop.\n\r");
        act("$n finishes $s work and stops.",
            FALSE, ch, NULL, NULL, TO_ROOM);
        ch->stopTask();
      } else if (!task_whittleCreateNew(ch, tStWood, ch->task->flags)) {
        ch->sendTo("You don't have enough wood to make another arrow.\n\r");
        ch->stopTask();
      } else {
        ch->sendTo("You begin making another arrow.\n\r");
        act("$n starts on a new arrow.",
            FALSE, ch, NULL, NULL, TO_ROOM);
      }
    } else
      ch->stopTask();
  }

  return nRc;
}

// whittle <object> <wood>
void TBeing::doWhittle(const char *tArg)
{
  if (!doesKnowSkill(SKILL_WHITTLE)) {
    sendTo("I bet you wish you knew how to whittle.\n\r");
    return;
  }

  if (!tArg || !*tArg) {
      sendTo("Syntax: whittle <object> <wood>\n\r");

    if (isImmortal()) {
      sendTo("Object Name                                      Vol    Wei Req% VNum  Df Time\n\r");
      sendTo("------------------------------------------------------------------------------\n\r");

      for (int whittleIndex = 0; whittleIndex < (signed) whittleItems.size(); whittleIndex++) {
        double whittleTime = (((whittleItems[whittleIndex].volSize +
                                whittleItems[whittleIndex].weiSize) / 25) *
                              whittleItems[whittleIndex].itemType);

        sendTo(fmt("%-45s:%c: %6.0f %3.0f %3.0d%c %5d %2d %4.0f\n\r") %
               whittleItems[whittleIndex].name %
               (whittleItems[whittleIndex].valid ? '*' : ' ') %
               whittleItems[whittleIndex].volSize %
               whittleItems[whittleIndex].weiSize %
               whittleItems[whittleIndex].whittleReq % '%' %
               whittleItems[whittleIndex].itemVnum %
               whittleItems[whittleIndex].itemType %
               whittleTime);
      }
    }

    return;
  }

  sstring      tStString(tArg),
              tStObject(""),
              tStWood("");
  TThing     *tObj;
  int         knownLevel = 0;
  int         tIndex     = -1;
  TGenWeapon *tWeapon = dynamic_cast<TGenWeapon *>(heldInPrimHand());

  tStObject=tStString.word(0);
  tStWood=tStString.word(1);


  if (tStObject.empty()) {
    sendTo("Syntax: whittle <object> <wood>\n\r");
    return;
  }

  if (!tWeapon || (!tWeapon->isPierceWeapon() &&
                   !tWeapon->isSlashWeapon())) {
    sendTo("You must be wielding a slash or pierce weapon to do this.\n\r");
    return;
  }

  if (tStWood.empty()) {
    if (!(tObj = searchLinkedListVis(this, tStObject, getStuff()))) {
      sendTo("Do you have that item?  No, so you cannot whittle on it.\n\r");
      return;
    }

    TArrow *tArrow;
    TBow   *tBow;

    if (!(tArrow = dynamic_cast<TArrow *>(tObj)) &&
        !(tBow   = dynamic_cast<TBow   *>(tObj))) {
      sendTo("I'm afraid you cannot continue whittling on that item.\n\r");
      return;
    }

    if (tArrow) {
      if (tArrow->isArrowFlag(ARROW_SMOOTHED)) {
        sendTo("That arrow is already finished, you cannot continue whittling it.\n\r");
        return;
      }

      int knownLevel = 0;

      if (tArrow->isArrowFlag(ARROW_CARVED))
        knownLevel = 5;

      if (tArrow->isArrowFlag(ARROW_SCRAPED))
        knownLevel = 7;
    } else {
      if (tBow->isBowFlag(BOW_SMOOTHED)) {
        sendTo("That bow is already finished, you cannot continue whittling it.\n\r");
        return;
      }

      if (tBow->isBowFlag(BOW_CARVED))
        knownLevel = 10;

      if (tBow->isBowFlag(BOW_SCRAPED))
        knownLevel = 15;
    }

    TObj *tObjTemp = dynamic_cast<TObj *>(tObj);

    if (tObjTemp)
      for (int objectIndex = 0; objectIndex < (signed) whittleItems.size(); objectIndex++)
        if (tObjTemp->objVnum() == whittleItems[objectIndex].itemVnum) {
          tIndex = objectIndex;
          break;
        }

    if (tIndex == -1) {
      sendTo("I'm afraid you cannot whittle on that.\n\r");
      return;
    }

    --(*tObj);

    start_task(this, tObj, roomp, TASK_WHITTLE, "", knownLevel, in_room, 1, tIndex, 40);
  } else {
    for (int objectIndex = 0; objectIndex < (signed) whittleItems.size(); objectIndex++)
      if (whittleItems[objectIndex] == tStObject &&
          ((whittleItems[objectIndex].tClass == -1 ||
            hasClass(whittleItems[objectIndex].tClass) ||
            isImmortal())) &&
          getSkillValue(SKILL_WHITTLE) >= whittleItems[objectIndex].whittleReq) {
        tIndex = objectIndex;
        break;
      }

    if (tIndex == -1) {
      sendTo("You have no idea how to whittle that.\n\r");
      return;
    }

    if (tWeapon->getVolume() > whittleItems[tIndex].weaSize) {
      sendTo("I'm afraid that weapon is just too big to whittle that.\n\r");
      return;
    }

    if (!task_whittleCreateNew(this, tStWood, tIndex)) {
      sendTo("I'm afraid you don't have the materials to make that.\n\r");
      return;
    }

    start_task(this, NULL, roomp, TASK_WHITTLE, tStWood.c_str(), 0, in_room, 1, tIndex, 40);

    if (!task_whittleCreateNew(this, tStWood, tIndex)) {
      sendTo("You did something bad.  Tell a god.\n\r");
      stopTask();
      return;
    }
  }
}

int task_whittle(TBeing *ch, cmdTypeT cmd, const char *tArg, int pulse, TRoom *, TObj *tObj)
{
  int         nRc = TRUE;
  sstring      tStWood(tArg);
  TGenWeapon *tWeapon;

  if (ch->isLinkdead() || (ch->in_room != ch->task->wasInRoom) ||
      ch->task->flags < 1 || ch->task->flags >= (signed) whittleItems.size()) {
    stop_whittling(ch);
    return FALSE;
  }

  if (ch->utilityTaskCommand(cmd) || ch->nobrainerTaskCommand(cmd) ||
      cmd == CMD_EAT || cmd == CMD_DRINK)
    return FALSE;

  if (!tObj) {
    ch->sendTo("Somehow the object you were making has vanished, how odd...\n\r");
    act("$n looks around really confused.",
        FALSE, ch, NULL, NULL, TO_ROOM);
    ch->stopTask();
    return TRUE;
  }

  if (!(tWeapon = dynamic_cast<TGenWeapon *>(ch->heldInPrimHand())) ||
      (!tWeapon->isPierceWeapon() && !tWeapon->isSlashWeapon())) {
    ch->sendTo("Hrm.  Your no longer using an appropriate weapon, so you are forced to stop.\n\r");
    act("$n looks around really confused.",
        FALSE, ch, NULL, NULL, TO_ROOM);
    stop_whittling(ch);
    return TRUE;
  }

  switch (cmd) {
    case CMD_TASK_CONTINUE:
      nRc = task_whittleObject(ch, tStWood);
      break;
    case CMD_STOP:
    case CMD_ABORT:
      stop_whittling(ch);
      break;
    case CMD_TASK_FIGHTING:
      ch->sendTo("You are unable to continue whittling while under attack!\n\r");
      stop_whittling(ch);
      break;
    default:
      if (cmd < MAX_CMD_LIST)
        warn_busy(ch);

      break;
  }

  return nRc;
}

//***** All below here is for the 'class taskWhittleEntry' stuff

bool taskWhittleEntry::operator==(sstring tString)
{
  return (valid && isname(tString, name));
}

sstring taskWhittleEntry::getName(bool showSecond)
{
  sstring tStString(name), tStName, tStExcess, tName;

  tStName=tStString.word(0);
  tStExcess=tStString.word(1);

  if (showSecond)
    tName=tStExcess;
  else
    tName=tStName;

  for(unsigned int i=0;i<tName.size();++i)
    if(tName[i] == '-')
      tName[i]=' ';

  return (tName);
}

void taskWhittleEntry::operator()(sstring tString, int newtClass,
                                  int newWhittleReq, int newItemVnum,
                                  bool newAffectValue,
                                  whittleTypeT newItemType = WHITTLE_ERROR)
{
  TObj *tObj = NULL;
  int   tRealNum = -1;

  name        = tString;
  itemType    = newItemType;
  whittleReq  = newWhittleReq;
  itemVnum    = newItemVnum;
  affectValue = newAffectValue;
  tClass      = newtClass;

  if (itemVnum >= 0)
    tRealNum = real_object(itemVnum);

  if (tRealNum < 0 || tRealNum > (signed) obj_index.size() ||
      !(tObj = read_object(tRealNum, REAL)))
    valid = false;
  else {
    volSize = tObj->getVolume();
    volSize *= 1.10;
    weiSize = tObj->getWeight();
    weiSize *= 1.10;
    weaSize = max(400.0, ((volSize + weiSize) / 10));

    vlogf(LOG_MISC, fmt("Adding Whittle: [%.1f %d : %.1f] [%.1f %.1f : %.1f]") % 
          volSize % tObj->getVolume() % (tObj->getVolume() * 1.10) %
          weiSize % tObj->getWeight() % (tObj->getWeight() * 1.10));

    valid = true;
    delete tObj;
    tObj = NULL;
  }
}

taskWhittleEntry::taskWhittleEntry() :
  name(""),
  volSize(0.0),
  weiSize(0.0),
  weaSize(0.0),
  whittleReq(0),
  itemVnum(-1),
  tClass(-1),
  itemType(WHITTLE_ERROR),
  valid(false),
  affectValue(false)
{
}

taskWhittleEntry::~taskWhittleEntry()
{
}
