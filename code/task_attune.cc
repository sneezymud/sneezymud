//////////////////////////////////////////////////////////////////////////
//
//      SneezyMUD - All rights reserved, SneezyMUD Coding Team
//      "task.cc" - All functions related to tasks that keep mobs/PCs busy
//
//////////////////////////////////////////////////////////////////////////

#include "stdsneezy.h"
#include "obj_symbol.h"
#include "obj_vial.h"

static void stop_attune(TBeing *ch, silentTypeT silent_char, silentTypeT silent_room)
{
  if (ch->getPosition() >= POSITION_RESTING) {
    if (!silent_char)
      act("You stop attuning your symbol.",
             FALSE, ch, 0, 0, TO_CHAR);
    if (!silent_room)
      act("$n stops attuning $s symbol.",
             FALSE, ch, 0, 0, TO_ROOM);
  }
  ch->stopTask();
}

void TThing::attunePulse(TBeing *ch)
{
  stop_attune(ch, SILENT_NO, SILENT_NO);
}

void TVial::findVialAttune(TVial **water, int *uses)
{
  if (!(getDrinkType() == LIQ_HOLYWATER))
    return;
  if (getDrinkUnits() <= 0)
    return;

  *water = this;
  *uses += getDrinkUnits();
}

bool checkAttuneUsage(TBeing *ch, int * uses, int * reqUses, TVial **water, TSymbol *sym)
{
  TThing *tmp = NULL;

  *uses = 0;
  for (tmp = ch->getStuff(); tmp; tmp = tmp->nextThing) {
    tmp->findVialAttune(water, uses);
  }

  if (!water) {
    ch->sendTo("You do not have any holy water to finish the attuning process.\n\r");
    stop_attune(ch, SILENT_YES, SILENT_NO);
    return false;
  }

  *reqUses = (int) (0.005 * sym->obj_flags.cost);
  if ((*uses + (ch->task ? ch->task->flags : 0)) < *reqUses) {
    ch->sendTo(COLOR_OBJECTS, fmt("You do not have enough holy water to finish attuning %s!\n\r") %
               sym->getName());
    stop_attune(ch, SILENT_YES, SILENT_NO);
    return false;
  }
  return true;
}

void TSymbol::attunePulse(TBeing *ch)
{
  int uses = 0, num = 0, reqUses = 0;

  if (getSymbolFaction() != FACT_UNDEFINED) {
    ch->sendTo(COLOR_OBJECTS, fmt("%s has already been attuned.\n\r") % getName());
    stop_attune(ch, SILENT_YES, SILENT_NO);
    return;
  }

  ch->addToMove(-1);
  if (ch->getMove() < 10) {
    act("You are much too tired to continue to attune $p.", FALSE, ch, this, NULL, TO_CHAR);
    act("$n stops attuning $p and looks up.", FALSE, ch, this, NULL, TO_ROOM);
    stop_attune(ch, SILENT_YES, SILENT_YES);
    return;
  }

  // check holy water
  TVial *water = NULL;
  if (!checkAttuneUsage(ch, &uses, &reqUses, &water, this))
    return;

  if (ch->task->status) {
    // this will randomly add water onto the symbol, we track this so we always
    // use the same amount
    if (!(::number(0,2)) && (ch->task->flags < reqUses)) {
      act("You apply some holy water from $N to $p.", FALSE, ch, this, water, TO_CHAR);
      act("$n applies some holy water to $p.", FALSE, ch, this, NULL, TO_ROOM);
      water->addToDrinkUnits(-1);
      ch->task->flags++;
    }
    if (ch->task->status == 1) {
      act("You feel that $p has almost been attuned.", FALSE, ch, this, NULL, TO_CHAR);
    } else if (!(::number(0,1))) {
      act("You continue attuning $p.", FALSE, ch, this, NULL, TO_CHAR);
      act("$n continues to pray over $p.", TRUE, ch, this, NULL, TO_ROOM);
    }
    return;
  } else {
    // get total uses required
    uses = (int) (0.005 * obj_flags.cost);
    // account for usage during the  task
    uses = max(1, uses - ch->task->flags);

    TThing *tmp;
    for (tmp = ch->getStuff(); tmp; tmp = tmp->nextThing) {
      water = NULL;
      num = 0;
      tmp->findVialAttune(&water, &num);
      if (water) {
        if (num >= uses) {
          water->addToDrinkUnits(-uses);
          ch->task->flags += uses;
          uses = 0;
          break;
        } else {
          // the water is in multiple containers, use up one container at
          // a time and just keep calling until its correct
          water->setEmpty();
          uses -= num;
          ch->task->flags += num;
          continue;
        }
      }
    }
    return;
  }
  return;
}

int task_attuning(TBeing *ch, cmdTypeT cmd, const char *, int pulse, TRoom *, TObj *obj)
{
  TThing *o = NULL;
  int learning;
  TSymbol *symbol;

  // sanity check
  if (ch->isLinkdead() ||
      (ch->in_room != ch->task->wasInRoom) ||
      (ch->getPosition() < POSITION_RESTING) ||
      !(o = ch->heldInPrimHand()) ||
      !obj ||
      (obj != o)) {
    stop_attune(ch, SILENT_NO, SILENT_NO);
    return FALSE;  // returning FALSE lets command be interpreted
  }
  if (!ch->task) {
    vlogf(LOG_BUG, "Got to bad spot in attune, tell Cosmo");
    ch->stopTask();
    return FALSE;
  }

  if (ch->utilityTaskCommand(cmd) || ch->nobrainerTaskCommand(cmd))
    return FALSE;

  symbol = dynamic_cast<TSymbol *>(obj);
  switch (cmd) {
  case CMD_TASK_CONTINUE:
    if (ch->task->status) {
        learning = ch->getSkillValue(SKILL_ATTUNE);
        ch->task->calcNextUpdate(pulse, 2 * PULSE_MOBACT);
        if (ch->bSuccess(learning, SKILL_ATTUNE)) {
          symbol->attunePulse(ch);
          if (ch->task)
            ch->task->status--;
          else {
            ch->stopTask();
            return FALSE;
          }
        } else if (!(::number(0,1)))
          act("You continue trying to slowly attune $p.", FALSE, ch, symbol, NULL, TO_CHAR);
        return FALSE;
    } else {
        symbol->attunePulse(ch);
        symbol->setSymbolFaction(ch->getFaction());
        act("Your prayers comes to their end as you finish sanctifying $p.",
            FALSE, ch, symbol, 0, TO_CHAR);
        act("You apply a last measure of holy water to $p as you feel it become fully attuned to $d." , FALSE, ch, symbol, 0, TO_CHAR);
        act("$n finishes attuning $p.", FALSE, ch, symbol, 0, TO_ROOM);
        stop_attune(ch, SILENT_YES, SILENT_YES);
        return FALSE;
    }
  case CMD_ABORT:
  case CMD_STOP:
      act("You stop trying to attune $p.", FALSE, ch, symbol, 0, TO_CHAR);
      act("$n stops attuning $p.", FALSE, ch, symbol, 0, TO_ROOM);
      ch->stopTask();
      break;
  case CMD_TASK_FIGHTING:
      ch->sendTo("You are unable to continue attuning while under attack!\n\r");
      stop_attune(ch, SILENT_YES, SILENT_NO);
      break;
  default:
      if (cmd < MAX_CMD_LIST)
        warn_busy(ch);
      break;                    // eat the command
  }
  return TRUE;
}
