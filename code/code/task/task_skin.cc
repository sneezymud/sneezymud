//////////////////////////////////////////////////////////////////////////
//
// SneezyMUD - All rights reserved, SneezyMUD Coding Team
//
//      "task_skinning.cc" - The skinning task
//
//////////////////////////////////////////////////////////////////////////

#include "comm.h"
#include "obj_base_corpse.h"
#include "extern.h"
#include "obj_organic.h"
#include "obj_component.h"
#include "being.h"
#include "obj_tool.h"
#include "obj_base_weapon.h"
#include "connect.h"
#include "skills.h"

void stop_skin(TBeing *ch)
{
  if (ch->getPosition() >= POSITION_RESTING) {
    act("You stop skinning, and look about confused.",
            FALSE, ch, 0, 0, TO_CHAR);
    act("$n stops skinning, and looks about confused and embarrassed.",
            FALSE, ch, 0, 0, TO_ROOM);
  }
  ch->stopTask();
}

int TThing::skinPulse(TBeing *ch, TBaseCorpse *corpse)
{
  TThing *Tobj;
  TBaseWeapon *tobj;
  int  learning = ch->getSkillValue(SKILL_SKIN),
    Ceffect = (corpse->isCorpseFlag(CORPSE_HALF_SKIN)?2:1),
       amount,
       maxUnitsP = max(1, (int)((corpse->getWeight()*.10)/2)-1),
       num;
  TObj *item;
  char msg   [256],
       gl_msg[256];

  if (ch->isImmortal()) {
    ch->sendTo("You pull a superman and clean the corpse in an instant, immortality rocks!\n\r");
    act("$n becomes a blur and instantly skins $p.",
        FALSE, ch, corpse, 0, TO_ROOM);
    ch->task->flags = (int)(maxUnitsP/Ceffect);
  }
  // Make sure we 1) Still have a Weapon capable of this task.
  //              2) Make sure we havn't completely skinned this target.
  //              3) Make sure we havn't ran out of turns yet.
  //              4) We didn't Issue a Stop and end up back here.
  Tobj = ch->heldInPrimHand();
  tobj = dynamic_cast<TBaseWeapon *>(Tobj);

  if ((Tobj && (Tobj->isPierceWeapon() || Tobj->isSlashWeapon())) &&
      (ch->task->flags < (int)(maxUnitsP/Ceffect)) &&
      ch->task->timeLeft > 0) {
    act("You continue to skin $p.",
        FALSE, ch, corpse, 0, TO_CHAR);
    act("$n slices the skin from $p very carefully.",
        FALSE, ch, corpse, NULL, TO_ROOM);
    ch->task->timeLeft--;

    if (!ch->bSuccess(learning, SKILL_SKIN)) {
      // O.k.  Here is the run down:
      //   1 Fail         = +Fail !decay +UnitsGotten -1sharpness
      //   2 Fails        = +Fail !decay !UnitsGotten -2sharpness
      //   2 Fails + Crit = +Fail !decay !UnitsGotten -3sharpness +Damage
      // Basically if a person fails we just want to let the corpse decay
      // a little and if they really fail we scrap what Unit(s) they might
      // have gotten.  Should they extremely mess up, should be rare, we
      // want to apply some damage to the skinner:
      //   5+(max(10, CurSharp/5)/2)  :: 10, ..., 15 HP
      // We also drop a little bit of blood in the room for visual fun,
      // but Do Not start a bleed on the person on question.
      CF(SKILL_SKIN);
      if (ch->bSuccess(learning, SKILL_SKIN)) {
        act("You gently over extend yourself and slightly dull your weapon.",
            FALSE, ch, 0, 0, TO_CHAR);
        if (tobj->getCurSharp() > 2)tobj->addToCurSharp(-1);
        ch->task->flags++;
      } else if (!critFail(ch, SKILL_SKIN)) {
        act("You extend yourself a little too far and dull your weapon.",
            FALSE, ch, 0, 0, TO_CHAR);
        if (tobj->getCurSharp() > 3)tobj->addToCurSharp(-2);
      } else {
        act("You really slip up and cut yourself and a part of the hide.",
            FALSE, ch, 0, 0, TO_CHAR);
        act("$n really slips up and mangles part of the hide and part of $mself",
            FALSE, ch, 0, 0, TO_ROOM);
        if (tobj->getCurSharp() > 4)tobj->addToCurSharp(-3);
        ch->dropBloodLimb((ch->isRightHanded() ? WEAR_FINGER_L : WEAR_FINGER_R));
        if (ch->reconcileDamage(ch, 5+(min(20, tobj->getCurSharp())/2), SKILL_SKIN) == -1) {
          ch->stopTask();
          ch->doSave(SILENT_YES);
          if (corpse->isCorpseFlag(CORPSE_PC_SKINNING))
            corpse->remCorpseFlag(CORPSE_PC_SKINNING);
          return DELETE_THIS;
        }
      }
    } else {
      // We got a success.  So we rack it up, we got 1 more unit of skin and we
      // we also increase the corpse duration by 1, so it doesn't decay on us.
      // Should this be a 10th roll over(unit wise) we want to deduct 1 point
      // of sharpness from the weapon.
      CS(SKILL_SKIN);
      ch->task->flags += max(1, (int) (learning/25));
      corpse->obj_flags.decay_time++;
      if (tobj->getCurSharp() > 2 && (ch->task->flags % 10) == 0)
        tobj->addToCurSharp(-1);
      ch->task->flags = max(1, min(ch->task->flags, maxUnitsP));
    }
    return FALSE;
  } else {
    if (!Tobj || (!Tobj->isPierceWeapon() && !Tobj->isSlashWeapon())) {
      act("Hey, where'd your weapon go?",
          FALSE, ch, 0, 0, TO_CHAR);
      act("$n looks blankly at $s empty hand.",
          FALSE, ch, 0, NULL, TO_ROOM);
      act("With the lack of a weapon, you grab the loose flesh and rip.",
          FALSE, ch, 0, 0, TO_CHAR);
      act("$n grabs the loose hide and rips it the rest of the way off.",
          FALSE, ch, 0, NULL, TO_ROOM);
      if (ch->task->flags > 0)
        ch->task->flags--;
    } else if (ch->task->timeLeft <= 0 && ch->task->timeLeft != -1) {
      act("You don't feel as if you could skin another slice at this time.",
          FALSE, ch, 0, 0, TO_CHAR);
      act("$n seems as if $e couldn't go on.",
          FALSE, ch, 0, NULL, TO_ROOM);
    } else if (ch->task->timeLeft != -1) {
      act("You feel confident that you got all the hide off of this one.",
          FALSE, ch, 0, 0, TO_CHAR);
      act("$n looks up with a sense of pride in $s work.",
          FALSE, ch, 0, NULL, TO_ROOM);
    } else if (ch->task->flags == 0) {
      act("You finish up your skinning and realize you destroyed all the hide.",
          FALSE, ch, 0, 0, TO_CHAR);
      act("$n finishes up and has apparently destroyed all the hide.",
          FALSE, ch, 0, NULL, TO_ROOM);
      ch->stopTask();
      return FALSE;
    } else if (ch->task->timeLeft != -1) {
      act("Something happened that wasn't expected, Tell a god what you did.",
          FALSE, ch, 0, 0, TO_CHAR);
      ch->stopTask();
      return FALSE;
    }
  }

  // We store the TotalUnits gotten, because we will lose the value when
  // we stop the task.  Stop the task then check if we are going to flag
  // this corpse either HALF or NO skin.  If it Was half skin, then it
  // becomes no.  Should it have Not been HALF but only half(or less) of
  // the possible units skinned, we flag HALF otherwise NO.
  int totalUnits = ch->task->flags;
  ch->stopTask();
  if (!corpse->isCorpseFlag(CORPSE_HALF_SKIN) && totalUnits <= (int)(maxUnitsP / 2))
    corpse->addCorpseFlag(CORPSE_HALF_SKIN);
  else
    corpse->addCorpseFlag(CORPSE_NO_SKIN);
  if (corpse->isCorpseFlag(CORPSE_PC_SKINNING))
    corpse->remCorpseFlag(CORPSE_PC_SKINNING);

  num = determineSkinningItem(corpse, &amount, msg, gl_msg);
  if (num == -1 || !(item = read_object(num, VIRTUAL))) {
    // no item, this should not happen (checked previously)
    ch->sendTo("Problem.  tell a god.\n\r");
    vlogf(LOG_BUG, format("Problem in skinning (%s)") %  ch->getName());
    return FALSE;
  }

  // adjust quantity:
  //   Max-Units-Can-Get: (corpseWeight * .10) / 2
  //   Value will be: UnitsGotten * Level(of the mob * 1.9) :: Max
  //   Rough 'Value' Results: Assume Level 50 Ranger with Maxed skinning 0 Failure
  //   This also ignores the fact that a ranger Can't skin for infinity but is limit
  //   by there level and skill in skinning: 104 Turn Limit for non-immort Skinning
  //       fuzzy mouse:   1 Unit      1  Talen / 1.9  Per Unit
  //                ox:  69 Units   786  Talens/11.4  Per Unit
  // deer-white-tailed:   6 Units    60  Talens/19    Per Unit
  //         shracknir: 209 Units    40K Talens/191.9 Per Unit

  TComponent * tcomp = dynamic_cast<TComponent *>(item);
  if (!tcomp) {

    item->setWeight(min((int) corpse->getWeight(), max(1, (int) ((corpse->getWeight()*.02) *(totalUnits/maxUnitsP)))));
    item->setVolume(min((int) corpse->getVolume(), max(1, (int) ((corpse->getVolume()*.02) *(totalUnits/maxUnitsP)))));
    item->obj_flags.cost = max(1, (int) (corpse->getCorpseLevel() *((double).9 + (learning/100)))*(totalUnits/3));

    TOrganic *tOrg = dynamic_cast<TOrganic *>(item);
    if (tOrg) {
      tOrg->setUnits(totalUnits);
      tOrg->setOLevel(corpse->getCorpseLevel());
    }
  } else {
    // item skinned is a component, leave weight/vol/price alone
    // since these are from balance stuff

    // But set the corpse to no skin, this prevents the 2 for one deal.

    if (!corpse->isCorpseFlag(CORPSE_NO_SKIN))
      corpse->addCorpseFlag(CORPSE_NO_SKIN);
  }

  // Tell the people we got the hide.
  act(   msg, FALSE, ch, item, corpse, TO_CHAR);
  act(gl_msg, FALSE, ch, item, corpse, TO_ROOM);

  // Lets make sure the skinner isn't too weak/encumbered to grab this item.
  if (compareWeights(item->getTotalWeight(TRUE),
		     (ch->carryWeightLimit() - ch->getCarriedWeight())) == -1) {
    act("The weight of the hide gets too much for you, so you drop it.",
        FALSE, ch, 0, 0, TO_CHAR);
    act("The weight of the hide gets too much for $n, so $e drops it.",
        FALSE, ch, 0, NULL, TO_ROOM);
    *corpse->roomp += *item;
  } else if (ch->getCarriedVolume() + (item->getTotalVolume()
				       -item->getReducedVolume(NULL)) > ch->carryVolumeLimit()) {
    act("You struggle to hold onto the hide but it slips from your grasp.",
        FALSE, ch, 0, 0, TO_CHAR);
    act("$n struggles to hold onto the hide but it slips from $s grasp.",
        FALSE, ch, 0, NULL, TO_ROOM);
    *corpse->roomp += *item;
  } else *ch += *item;

  return FALSE;
}

int TTool::skinPulse(TBeing *ch, TBaseCorpse *corpse)
{
  int  learning = ch->getSkillValue(SKILL_SKIN),
       amount,
       num;
  TObj *item;
  char msg   [256],
       gl_msg[256];


  if (getToolType() != TOOL_SKIN_KNIFE) {
    act("Hey, where'd your skinning knife go?", FALSE, ch, 0, 0, TO_CHAR);
    stop_skin(ch);
    return FALSE;
  }

  if (ch->task->timeLeft > 0) {
    act("You continue to skin $p.", FALSE, ch, corpse, 0, TO_CHAR);
    act("$n slices the skin from $p very carefully.",
          FALSE, ch, corpse, NULL, TO_ROOM);

    addToToolUses(-1);
    if (getToolUses() <= 0) {
      act("Your $o breaks due to overuse.", FALSE, ch, this, 0, TO_CHAR);
      act("$n looks startled as $e breaks $P while skinning.", FALSE, ch, 0, this, TO_ROOM);
      ch->stopTask();
      delete this;
      return FALSE;
    }

    if (!ch->bSuccess(learning, SKILL_SKIN)) {
      if (!ch->bSuccess(learning, SKILL_SKIN)) {
        // a doubele failure
        CF(SKILL_SKIN);  // failure on that skin
        act("You slip up and destory a part of the hide.",
             FALSE, ch, 0, 0, TO_CHAR);
        act("$n really slips up and mangles part of the hide.",
             FALSE, ch, 0, 0, TO_ROOM);
        corpse->addCorpseFlag(CORPSE_NO_SKIN);
        ch->stopTask();
      }
    } else {
      ch->task->timeLeft--;
    }
  } else {
    CS(SKILL_SKIN);  // success on that skin
    ch->stopTask();
    corpse->addCorpseFlag(CORPSE_NO_SKIN);

    num = determineSkinningItem(corpse, &amount, msg, gl_msg);
    if (num == -1 || !(item = read_object(num, VIRTUAL))) {
      // no item, this should not happen (checked previously)
      ch->sendTo("Problem.  tell a god.\n\r");
      vlogf(LOG_BUG, format("Problem in skinning (%s)") %  ch->getName());
      return FALSE;
    }

    // adjust quantity
    amount = amount/10 + (amount%10 ? 1 : 0);  // each "unit" weighs 1/10 lb.
    item->setWeight(amount);
    item->obj_flags.cost *= amount;

    act(msg, FALSE, ch, item, corpse, TO_CHAR);
    act(gl_msg, FALSE, ch, item, corpse, TO_ROOM);

    *ch += *item;
  }

  return FALSE;
}

int task_skinning(TBeing *ch, cmdTypeT cmd, const char *, int pulse, TRoom *, TObj *obj)
{
  TThing *knife;
  TBaseCorpse *corpse = NULL;
  int rc;

  // sanity check
  if (ch->isLinkdead() || (ch->in_room < 0) ||
      (ch->getPosition() < POSITION_RESTING)) {
    stop_skin(ch);
    return FALSE;  // returning FALSE lets command be interpreted
  }
  if (ch->utilityTaskCommand(cmd) ||
      ch->nobrainerTaskCommand(cmd))
    return FALSE;

  if (!obj || !ch->sameRoom(*obj) ||
      !(corpse = dynamic_cast<TBaseCorpse *>(obj))) {
    act("Hey, where'd that corpse go?", FALSE, ch, 0, 0, TO_CHAR);
    stop_skin(ch);
    return FALSE;  // returning FALSE lets command be interpreted
  }
  if (corpse->isCorpseFlag(CORPSE_NO_REGEN)) {
    // a body part or something
    act("$p: You are not able to skin that.",
          FALSE, ch, corpse, 0, TO_CHAR);
    return FALSE;
  }
  if (corpse->isCorpseFlag(CORPSE_NO_SKIN)) {
    act("Something unfortunate has happened to $p and it can't be skinned",
              FALSE, ch, corpse, 0, TO_CHAR);
    stop_skin(ch);
    return FALSE;  // returning FALSE lets command be interpreted
  }

  switch (cmd) {
  case CMD_TASK_CONTINUE:
    if (!(knife = ch->heldInPrimHand())) {
        act("Hey, where'd your skinning knife go?", FALSE, ch, 0, 0, TO_CHAR);
        stop_skin(ch);
        return FALSE;  // returning FALSE lets command be interpreted
    }
    // each pulse is constatnt duration apart
    // the # of pulses was set in start_task as based on skill
      ch->task->calcNextUpdate(pulse, PULSE_MOBACT);
      rc = knife->skinPulse(ch, corpse);
      // knife may be invalid here
      return rc;
  case CMD_ABORT:
  case CMD_STOP:
      if (!(knife = ch->heldInPrimHand())) {
        act("You go to stop skinning, but your knife is gone!",
            FALSE, ch, 0, 0, TO_CHAR);
        act("You grab the loose skin and tear it away from the rest.",
            FALSE, ch, 0, 0, TO_CHAR);
        act("$n grabs the loose skin and tears it away from the rest.",
            TRUE, ch, 0, 0, TO_ROOM);
      } else {
        act("You make the final slice and stop skinning.",
            FALSE, ch, 0, 0, TO_CHAR);
        act("$n makes the final slice and stops skinning.",
            TRUE, ch, 0, 0, TO_ROOM);
      }
      ch->task->timeLeft=-1;
      rc = knife->skinPulse(ch, corpse);
      return rc;
  case CMD_TASK_FIGHTING:
      ch->sendTo("You are unable to continue skinning while under attack!\n\r");
      ch->stopTask();
      break;
  default:
      if (cmd < MAX_CMD_LIST)
        warn_busy(ch);
      break;                    // eat the command
  }
  return TRUE;
}
