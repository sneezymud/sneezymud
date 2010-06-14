#include "handler.h"
#include "room.h"
#include "being.h"
#include "low.h"
#include "process.h"

// simple function to do the non-bird social
void preen_social(TBeing *ch, TThing *target)
{
  if (!target || target == dynamic_cast<TThing*>(ch))
  {
      act("You clean yourself up a bit.  Don't you look dandy?", FALSE, ch, NULL, target, TO_CHAR);
      act("$n vainly cleans $mself up.", TRUE, ch, NULL, target, TO_ROOM);
      return;
  }
  act("You clean $N up a bit.  Well, as much as you can.", FALSE, ch, NULL, target, TO_CHAR);
  act("$n cleans you up a bit.  Well, isn't that nice?", FALSE, ch, NULL, target, TO_VICT);
  act("$n cleans $N up a bit.  Well, isn't that nice?", TRUE, ch, NULL, target, TO_NOTVICT);
}


// Preening is a social command, although it helps feathered birds fly.
// If you have feathers and preen, you can fly.  If you havent preened
// for a while, no flight for you.
void TBeing::doPreen(sstring &argument)
{
  if (in_room < 0)
    return;


  TThing *target = !argument.empty() ? searchLinkedListVis(this, argument, roomp->stuff) : dynamic_cast<TThing*>(this);
  bool being = target && dynamic_cast<TBeing*>(target) != NULL;
  bool feathered = being && dynamic_cast<TBeing*>(target)->getMyRace()->isFeathered();

  if (!target)
  {
    sendTo("You can't preen that!\n\r");
    return;
  }
  else if (!feathered)
  {
    preen_social(this, target);
    return;
  }
  if (!getMyRace()->isFeathered())
  {
    sendTo("You don't know the first thing about preening those feathers!\n\r");
    return;
  }
  else if (isFlying())
  {
    sendTo("You can't preen while flying!\n\r");
    return;
  }
  else if (getPosition() != POSITION_STANDING && getPosition() != POSITION_SITTING)
  {
    sendTo("You need to be standing or sitting in order to preen.\n\r");
    return;
  }
  else if (being && dynamic_cast<TBeing*>(target)->isFlying())
  {
    sendTo("You can't preen while they are flying!\n\r");
    return;
  }
  else if (roomp->isUnderwaterSector())
  {
    sendTo("Preening underwater would ruin feathers!\n\r");
    return;
  }

  // self
  if (!target || target == dynamic_cast<TThing*>(this))
  {
    act("You begin to clean up some of your feathers.", FALSE, this, NULL, target, TO_CHAR);
    act("$n begins to preen $s feathers.", TRUE, this, NULL, target, TO_ROOM);
  }
  else // someone else
  {
    act("You begin to clean up some of $Ns feathers.", FALSE, this, NULL, target, TO_CHAR);
    act("$n begins preening your feathers for you.", FALSE, this, NULL, target, TO_VICT);
    act("$n begins to preen $Ns feathers.", TRUE, this, NULL, target, TO_NOTVICT);
  }

  // start a preen task
  start_task(this, NULL, roomp, TASK_PREEN, target->name, 3, inRoom(), 0, 0, 5);
}


// feather-cleaning task
int task_preen(TBeing *ch, cmdTypeT cmd, const char *arg, int pulse, TRoom *rp, TObj *)
{
  TBeing *target = NULL;
  if(!ch || ch->utilityTaskCommand(cmd) || ch->nobrainerTaskCommand(cmd))
    return FALSE;

  // stop preening
  if (!ch->task || ch->isLinkdead() || (ch->in_room != ch->task->wasInRoom))
  {
    act("You stop your preening.",FALSE, ch, 0, 0, TO_CHAR);
    act("$n stops $s preening.",TRUE, ch, 0, 0, TO_ROOM);
    ch->stopTask();
    return FALSE;
  }

  if (ch->task->timeLeft < 0)
  {
    ch->sendTo("You can't find any feathers that need preening.\n\r");
    ch->stopTask();
    return TRUE;
  }

  target = get_char_room_vis(ch, ch->task->orig_arg, NULL, EXACT_YES, INFRA_YES);
  if (!target)
  {
    ch->sendTo("The person you were preening is no longer here!\n\r");
    ch->stopTask();
    return TRUE;
  }

  static const sstring selfPreen[18] = {
  "You inspect your feathers for dirt and damage.", // to self 3
  "", // to vict 3
  "$n inspects $s feathers for dirt and damage.", // to room 3
  "You grab some of your feathers and ruffle them a bit.", // to self 2
  "", // to vict 2
  "$n grabs some of $s feathers and ruffles them a bit.", // to room 2
  "You smooth some of your feathers, making them lay flat and shiny.", // to self 1
  "", // to vict 1
  "$n smoothes some of $s feathers, making them lay flat and shiny.", // to room 1
  "You finish up preening yourself.  You feel nice and clean now.", // to self 0
  "", // to vict 0
  "$n finishes up $s preening.", // to room 0
  "You discard one of your damaged feathers.", // to self DROP FEATHER
  "", // to vict DROP FEATHER
  "$n discards a damaged feather.  It floats slowly to the $g.", // to room DROP FEATHER
  "You stop preening your feathers.", // to self STOP
  "", // to vict STOP
  "$n stops preening $s feathers.", // to room STOP
  };
  static const sstring otherPreen[18] = {
  "You inspect $Ns feathers for dirt and damage.", // to self 3
  "$n inspects your featers for dirt and damage.", // to vict 3
  "$n inspects $Ns feathers for dirt and damage.", // to room 3
  "You grab some of $Ns feathers and ruffle them a bit.", // to self 2
  "$n grabs some of your feathers and ruffles them a bit.", // to vict 2
  "$n grabs some of $Ns feathers and ruffles them a bit.", // to room 2
  "You smooth some of $Ns feathers, making them lay flat and shiny.", // to self 1
  "$n smoothes some of your feathers, making them lay flat and shiny.", // to vict 1
  "$n smoothes some of $Ns feathers, making them lay flat and shiny.", // to room 1
  "You finish up preening $N.  $E looks nice and clean now.", // to self 0
  "$n finishes preening your feathers.  What a nice gesture!", // to vict 0
  "$n finishes up preening $N.", // to room 0
  "You discard one of $Ns damaged feathers.", // to self DROP FEATHER
  "$n plucks a damaged feather from your plumage and discards it.", // to vict DROP FEATHER
  "$n discards one of $Ns damaged feathers.  It floats slowly to the $g.", // to room DROP FEATHER
  "You stop preening $Ns feathers.", // to self STOP
  "$n stops preening your feathers.", // to vict STOP
  "$n stops preening $Ns feathers.", // to room STOP
  };
  const sstring * preenAct = otherPreen;
  int preenChance = 1;
  if (target == ch)
    preenAct = selfPreen;
  if (ch->affectedBySpell(AFFECT_WET))
    preenChance = 2;

  switch (cmd) {
    case CMD_TASK_CONTINUE:
      ch->task->calcNextUpdate(pulse, PULSE_MOBACT * 5);

      // valid timeleft states are 3, 2, 1, 0
      if (ch->task->timeLeft > 3 || ch->task->timeLeft < 0)
        ch->task->timeLeft = 3;

      // you lose movement each 'tick' of preening
      ch->addToMove(-1);
      if (ch->getMove() < 6)
      {
        ch->sendTo("You are much too tired to continue preening.\n\r");
        act(preenAct[15], FALSE, ch, NULL, target, TO_CHAR);
        act(preenAct[16], TRUE, ch, NULL, target, TO_VICT);
        act(preenAct[17], TRUE, ch, NULL, target, TO_NOTVICT);
        ch->stopTask();
        return FALSE;
      }

      // send regular preen message for state
      int index = 9 - (3 * ch->task->timeLeft);
      act(preenAct[index++], FALSE, ch, NULL, target, TO_CHAR);
      act(preenAct[index++], TRUE, ch, NULL, target, TO_VICT);
      act(preenAct[index++], TRUE, ch, NULL, target, TO_NOTVICT);

      // random chance to drop a feather during grooming (3% chance)
      if (ch->task->timeLeft < 3 && ch->task->timeLeft > 0 && !::number(0, 29))
      {
        TObj *obj = read_object(Obj::PREEN_FEATHER, VIRTUAL);
        if(!obj)
        {
          vlogf(LOG_BUG, "problem loading feather in task_preen()");
          return TRUE;
        }
        act(preenAct[12], FALSE, ch, NULL, target, TO_CHAR);
        act(preenAct[13], TRUE, ch, NULL, target, TO_VICT);
        act(preenAct[14], TRUE, ch, NULL, target, TO_NOTVICT);
        *ch->roomp += *obj;
      }

      if (ch->task->timeLeft <= 0)
      {
        int duration = UPDATES_PER_MUDHOUR * 8;
        if (target != ch)
          duration *= 2;

        if (!target->affectedBySpell(AFFECT_PREENED))
        {
          // add the 'preened' buff
          affectedData aff;
          aff.type = AFFECT_PREENED;
          aff.location = APPLY_NONE;
          aff.duration = duration;
          aff.bitvector = AFF_FLIGHTWORTHY;
          aff.modifier = 0;
          aff.renew = duration;
          target->affectTo(&aff, duration);
        }
        else
        {
          // clobber durations of preened with new preen time
          for (affectedData *hjp = target->affected; hjp; hjp = hjp->next)
            if (hjp->type == AFFECT_PREENED && hjp->location == APPLY_NONE && hjp->duration < duration)
            {
              hjp->bitvector = AFF_FLIGHTWORTHY;
              hjp->duration = duration;
            }
        }

        ch->stopTask();
      }
      else if (ch->task->timeLeft > 0 && ::number(0,preenChance))
        ch->task->timeLeft--;
      break;
    case CMD_ABORT:
    case CMD_STOP:
      act(preenAct[15], FALSE, ch, NULL, target, TO_CHAR);
      act(preenAct[16], TRUE, ch, NULL, target, TO_VICT);
      act(preenAct[17], TRUE, ch, NULL, target, TO_NOTVICT);
      ch->stopTask();
      break;
    case CMD_TASK_FIGHTING:
      ch->sendTo("You can't preen feathers and fight at the same time!\n\r");
      ch->stopTask();
      break;
    default:
      if (cmd < MAX_CMD_LIST)
        warn_busy(ch);
      break;
  }
  return TRUE;
}

