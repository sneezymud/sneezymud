//////////////////////////////////////////////////////////////////////////
//
// SneezyMUD - All rights reserved, SneezyMUD Coding Team
//
//////////////////////////////////////////////////////////////////////////


/*****************************************************************************

  SneezyMUD++ - All rights reserved, SneezyMUD Coding Team.

  "task_stavecharge.cc"
  All functions and routines related to the stave charging task.

  Created 7/20/99 - Lapsos(William A. Perrotto III)

******************************************************************************/

#include "stdsneezy.h"
#include "obj_component.h"
#include "obj_staff.h"

// How many charges per 'charge' does it cost.
static const int STAVECHARGE_COMPMULTIPLIER = 5;

// Master 4 block:
//   Crit Success, Normal, Failure, Crit Failure
// Slave 4 block:
//   Glowing, Shadowy, Humming, Other
// Slave 2 block:
//   To-Char, To-Room
static const char *tStaveChargeMessages[4][4][2] =
{
  {{"$p flares with power as you slowly charge it.",
    "$p flares with power as $n slowly charges it."},
   {"$p draws forth power as you slowly charge it.",
    "$p draws forth power as $n slowly charges it."},
   {"$p vibrates madly as you slowly charge it.",
    "$p vibrates madly as $n slowly charges it."},
   {"$p shimmers with power as you slowly charge it.",
    "$p shimmers with power as $n slowly charges it."}},
  {{"$p glows gently as you charge it.",
    "$p glows gently as $n slowly charges it."},
   {"$p dims gently as you charge it.",
    "$p dims gently as $n charges it."},
   {"$p hums gently as you charge it.",
    "$p hums gently as $n charges it."},
   {"$p fluxuates gently as you charge it.",
    "$p fluxuates gently as $n charges it."}},
  {{"$p quickly fades as you feel energy being sucked out of you.",
    "$n utters a small wimper as life is sucked out of them."},
   {"$p quickly shines as you feel energy being sucked out of you.",
    "$n utters a quite scream as life is sucked out of them."},
   {"$p slowly falls silent as you feel energy being sucked out of you.",
    "$n utters a small plea as life is sucked out of them."},
   {"$p slowly settles as you feel energy being sucked out of you.",
    "$n utters a light moan as life is sucked out of them."}},
  {{"$p remains dark as mana is sucked from you.",
    "$p lies dark as $n is quickly drained."},
   {"$p remains shining as mana is sucked from you.",
    "$p lies shining as $n is quickly drained."},
   {"$p remains silent as mana is sucked from you.",
    "$p lies silent as $n is quickly drained."},
   {"$p remains still as mana is sucked out of you.",
    "$p lies still as $n quickly drained."}}
};

void stop_stavecharging(TBeing *ch, TObj *tObj)
{
  if (ch->isLinkdead() || (ch->in_room < 0) ||
      (ch->getPosition() < POSITION_RESTING)) {
    act("You stop charging $p and stand up.",
        FALSE, ch, tObj, NULL, TO_CHAR);
    act("$n shops charging $p and stands up.",
        FALSE, ch, tObj, NULL, TO_ROOM);
  }

  ch->stopTask();
}

extern TComponent *comp_from_object(TThing *, spellNumT);

int task_staveChargingCompSkim(TBeing *ch, TThing *tThing, bool tDestroy,
                               int tSpell, int & tCount, bool & tIteration)
{
  int tCost  = tCount,
      tValue = 0;

  TComponent *tComponent;

  while ((tThing = comp_from_object(tThing, spellNumT(tSpell)))) {
    if ((tComponent = dynamic_cast<TComponent *>(tThing)))
      if (tDestroy) {
        tThing = tThing->nextThing;

        if (!tIteration) {
          act("$p shatters from the charge effect.",
              FALSE, ch, tComponent, NULL, TO_CHAR);
          tIteration = true;
        }

        tCost = min(tComponent->getComponentCharges(), tCount);
        tCount -= tCost;
        tComponent->addToComponentCharges(-tCost);

        if (tComponent->getComponentCharges() <= 0) {
          --(*tComponent);
          delete tComponent;
        }

        if (tCount <= 0 || !tThing)
          return 0;
      } else {
        tValue += tComponent->getComponentCharges();

        if (!(tThing = tThing->nextThing))
          break;
      }
  }

  return tValue;
}

// if tDestroy:
//   return == was doable
// else
//   return == total charges found on person.
int task_staveChargingCompLookup(TBeing *ch, bool tDestroy, int tSpell, int tCount)
{
  TThing     *tThing;
  int         tValue = 0,
              tCost  = tCount;
  bool        tIteration = false;

  if (tDestroy) {
    tValue = task_staveChargingCompLookup(ch, false, tSpell, tCount);

    if (tValue < tCount)
      return FALSE;
  }

  if ((tThing = ch->heldInPrimHand()) && tCost > 0)
    tValue += task_staveChargingCompSkim(ch, tThing, tDestroy, tSpell, tCost, tIteration);

  if ((tThing = ch->heldInSecHand()) && tCost > 0)
    tValue += task_staveChargingCompSkim(ch, tThing, tDestroy, tSpell, tCost, tIteration);

  if ((tThing = ch->equipment[WEAR_WAIST]) && tCost > 0)
    tValue += task_staveChargingCompSkim(ch, tThing, tDestroy, tSpell, tCost, tIteration);

  if ((tThing = ch->getStuff()) && tCost > 0)
    tValue += task_staveChargingCompSkim(ch, tThing, tDestroy, tSpell, tCost, tIteration);

  if (tDestroy)
    return TRUE;

  return tValue;
}

void TBeing::doChargeStave(sstring tStString)
{
  /*
  if (strcmp("Lapsos", getName()) != 0) {
    sendTo("This command is still experimental.\n\r");
    return;
  }
  */

  TStaff     *tStaff;
  spellNumT   tSpell;
  int         tMana,
              tCharge,
              tLearn[2],
              tCompCost = STAVECHARGE_COMPMULTIPLIER;
  char        tString[256];

  if (!isPc() || !desc)
    return;

  if (!hasClass(CLASS_MAGE)) {
    sendTo("If you want to be a mage go be one somewhere else...\n\r");
    return;
  }

  if (!(tLearn[0] = getSkillValue(SKILL_STAVECHARGE))) {
    sendTo("I bet you wish you knew how to charge staves.\n\r");
    return;
  }

  if (nomagic("A force prevents this from occuring here."))
    return;

  if (cantHit > 0) {
    sendTo("You are too busy.\n\r");
    return;
  }

  if (roomp->isWaterSector() || roomp->isUnderwaterSector()) {
    sendTo("Treading water and charging staves don't mix well.\n\r");
    return;
  }

  if (isCombatMode(ATTACK_BERSERK)) {
    sendTo("See the red in those eyes?  You are a bit too fired up for this.\n\r");
    return;
  }

  if (riding)
    if (dynamic_cast<TBeing *>(riding)) {
      sendTo("Dismounting first might help some.\n\r");
      return;
    } else if (dynamic_cast<TObj *>(riding)) {
      sendTo("You stand up as you're current position is incorrect.\n\r");
      doStand();
    }

  if (fight() || (task && getPosition() > POSITION_SITTING)) {
    sendTo("Something tells me you are kind of busy at the moment.\n\r");
    return;
  }

  if (!(tStaff = dynamic_cast<TStaff *>(equipment[getPrimaryHold()]))) {
    sendTo("You must be holding the staff you wish to charge to do this.\n\r");
    return;
  }

  if (tStaff->getCurCharges() == tStaff->getMaxCharges()) {
    sendTo("This staff is maxed out, it can not hold anymore power.\n\r");
    return;
  }

  stSpaceOut(tStString);

  if (tStString.empty())
    tSpell = tStaff->getSpell();
  else
    if (((tSpell = searchForSpellNum(tStString, EXACT_YES)) > TYPE_UNDEFINED) ||
        ((tSpell = searchForSpellNum(tStString, EXACT_NO )) > TYPE_UNDEFINED))
      if (discArray[tSpell]->typ != SPELL_MAGE) {
        sendTo("That is not a mage spell.\n\r");
        return;
      }

  if (tSpell <= TYPE_UNDEFINED || !discArray[tSpell]) {
    sendTo("Try a mage spell.  It really does work better.\n\r");
    return;
  }

  if (tStaff->getSpell() != -1 && tStaff->getSpell() != tSpell) {
    sendTo("That spell does not match what the staff has been attuned for.\n\r");
    return;
  }

  if (!(tLearn[1] = getSkillValue(tSpell))) {
    sendTo("You do not know that spell.\n\r");
    return;
  }

  if ((discArray[tSpell]->targets & TAR_CHAR_WORLD) ||
      !discArray[tSpell]->minMana) {
    sendTo("I am afraid this spell can not be charged.\n\r");
    return;
  }

  if (tLearn[1] < 90) {
    sendTo("You are not proficient enough in that spell to charge a stave with it.\n\r");
    return;
  }

  tCharge = (discArray[tSpell]->lag + 2) * 30;
  tMana = ((discArray[tSpell]->minMana /
            (discArray[tSpell]->lag + 2)) *
           discArray[tSpell]->lag + 2) / 10;
  tMana *= tCharge;

  if ((tCompCost -= task_staveChargingCompLookup(this, false, tSpell, tCompCost)) > 0) {
    sendTo("You do not have enough of the component to do this charging.\n\r");
    return;
  }

  if (getMana() < tMana) {
    sendTo("You do not have the mana to charge that spell.\n\r");
    return;
  }

  sprintf(tString, "You rest and begin to charge $p with the powers of %s.",
          discArray[tSpell]->name);
  act(tString, FALSE, this, tStaff, NULL, TO_CHAR);
  act("$n begins to focus on $p.",
      FALSE, this, tStaff, NULL, TO_ROOM);
  setPosition(POSITION_SITTING);

  start_task(this, tStaff, NULL, TASK_STAVECHARGE, NULL, tCharge, in_room, 0, tSpell, 40);
}

int TObj::taskChargeMe(TBeing *ch, spellNumT, int &)
{
  ch->sendTo("How strange...This is not a staff...\n\r");
  stop_stavecharging(ch, this);

  return TRUE;
}

void TStaff::taskChargeMeUpdate(TBeing *ch, spellNumT tSpell)
{
  char tString[256];
  int  tLevel,
       tLearn;

  // This should never happen.
  if (getSpell() != tSpell && getSpell() != TYPE_UNDEFINED)
    return;

  // This should never happen.
  if (getCurCharges() == getMaxCharges())
    return;

  if (!task_staveChargingCompLookup(ch, true, tSpell, STAVECHARGE_COMPMULTIPLIER)) {
    ch->sendTo("The components you needed have vanished...\n\r");
    return;
  }

  sprintf(tString, "$p glows with the powers of %s.",
          discArray[tSpell]->name);
  act(tString, FALSE, ch, this, NULL, TO_CHAR);

  tLevel = max(1, min( 70, (int) ((getMagicLevel()       + ch->GetMaxLevel())         / 2)));
  tLearn = max(1, min(100, (int) ((getMagicLearnedness() + ch->getSkillValue(tSpell)) / 2)));

  addToCurCharges(1);
  setSpell(tSpell);
  setMagicLevel(tLevel);
  setMagicLearnedness(tLearn);
}

int TStaff::taskChargeMe(TBeing *ch, spellNumT tSpell, int & tCharge)
{
  if (!tCharge) {
    act("You have successfully charged $p.",
        FALSE, ch, this, NULL, TO_CHAR);
    act("$n beams with pride as they finish charging $p.",
        FALSE, ch, this, NULL, TO_ROOM);
    ch->stopTask();

    taskChargeMeUpdate(ch, tSpell);

    return TRUE;
  }

  if (tSpell <= TYPE_UNDEFINED || tSpell >= MAX_SKILL) {
    ch->sendTo("That is not a magic spell, how strange.\n\r");
    stop_stavecharging(ch, this);

    return TRUE;
  }

  if (tSpell != getSpell() && getCurCharges() != 0) {
    ch->sendTo("Unfortunatly that spell is not the same as the one in the stave.\n\r");
    stop_stavecharging(ch, this);

    return TRUE;
  }

  int tLearn   = ch->getSkillValue(SKILL_STAVECHARGE),
      tManaReq = ((discArray[tSpell]->minMana /
                   (discArray[tSpell]->lag + 2)) *
                  discArray[tSpell]->lag + 2) / 10;

  int tMasterBlock = 0,
      tSlaveBlock  = (isObjStat(ITEM_GLOW) ? 0 :
                      (isObjStat(ITEM_SHADOWY) ? 1 :
                       (isObjStat(ITEM_HUM) ? 2 : 3)));

  if (ch->bSuccess(tLearn, SKILL_STAVECHARGE)) {
    if (critSuccess(ch, tSpell)) {
      tMasterBlock = 0;
      tCharge -= min(tCharge, ::number(2, 10));
      tManaReq /= 2;
    } else {
      tMasterBlock = 1;
      tCharge--;
    }
  } else {
    if (critFail(ch, tSpell)) {
      tMasterBlock = 3;
      tCharge += min(tCharge, ::number(2, 5));
      tManaReq *= 2;
    } else {
      tMasterBlock = 2;
    }

    tManaReq *= 2;
  }

  act(tStaveChargeMessages[tMasterBlock][tSlaveBlock][0],
      FALSE, ch, this, NULL, TO_CHAR);

  if (!(tCharge % 10) || tMasterBlock == 0 || tMasterBlock == 3)
    act(tStaveChargeMessages[tMasterBlock][tSlaveBlock][1],
        FALSE, ch, this, NULL, TO_ROOM);

  // Mana Cost:
  //      Crit: Half of 1 rounds worth.
  //    Normal: 1 rounds worth.
  //      Fail: 2 rounds worth.
  // Crit Fail: 4 rounds worth.

  int tManaCost = min(ch->getMana(), tManaReq);

  ch->reconcileMana(TYPE_UNDEFINED, 0, tManaCost);

  tManaCost = tManaReq - tManaCost;

  if (tManaCost) {
    ch->sendTo("You're mana is gone so you stop casting but pay the price regardless.\n\r");

    tManaReq = min((ch->getHit() - 1), tManaCost);

    if (tManaReq) {
      ch->sendTo("You feel the life draining out of you.\n\r");

      // Should never happen, but screwy things Do happen.
      if (ch->reconcileDamage(ch, tManaReq, SKILL_STAVECHARGE) == -1) {
        ch->stopTask();
        ch->doSave(SILENT_YES);

        return DELETE_THIS;
      }
    }

    tManaCost = tManaCost - tManaReq;

    if (tManaCost) {
      tManaCost = min(ch->getMove(), tManaCost);
      ch->sendTo("You feel the vitality draining out of you.\n\r");
      ch->addToMove(-tManaCost);
    }

    ch->stopTask();
  }

  return TRUE;
}

int task_stavecharging(TBeing *ch, cmdTypeT tCmd, const char *, int pulse, TRoom *, TObj *tObj)
{
  spellNumT tSpell  = spellNumT(ch->task->flags);

  if (ch->isLinkdead() || (ch->in_room < 0) ||
      (ch->getPosition() < POSITION_RESTING)) {
    stop_stavecharging(ch, tObj);
    return FALSE;
  }

  if (ch->utilityTaskCommand(tCmd) ||
      ch->nobrainerTaskCommand(tCmd))
    return FALSE;

  switch (tCmd) {
    case CMD_TASK_CONTINUE:
      if (!tObj) {
        ch->sendTo("How strange, the stave you were charging has vanished!\n\r");
        act("$n stares about, something is apparently wrong.",
            FALSE, ch, NULL, NULL, TO_ROOM);
        ch->stopTask();
        return TRUE;
      }

      return tObj->taskChargeMe(ch, tSpell, ch->task->timeLeft);

      break;
    case CMD_ABORT:
    case CMD_STOP:
      act("You stop charging $p", FALSE, ch, tObj, NULL, TO_CHAR);
      stop_stavecharging(ch, tObj);

      break;
    case CMD_TASK_FIGHTING:
      ch->sendTo("You can not continue under these conditions!\n\r");
      stop_stavecharging(ch, tObj);

      break;
    default:
      if (tCmd < MAX_CMD_LIST)
        warn_busy(ch);

      break;
  };

  return TRUE;
}
