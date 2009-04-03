//////////////////////////////////////////////////////////////////////////
//
// SneezyMUD - All rights reserved, SneezyMUD Coding Team
//
//////////////////////////////////////////////////////////////////////////


#include "stdsneezy.h"
#include "combat.h"
#include "obj_base_clothing.h"

static int grapple(TBeing *c, TBeing *victim, spellNumT skill)
{
  int percent;
  int level, i = 0;
  int rc;
  int bKnown;
  int grapple_move = 25 + ::number(1,10);

  TThing *obj = NULL;
  TBaseClothing *tbc = NULL;
  int suitDam = 0;

  if (c->checkPeaceful("You feel too peaceful to contemplate violence.\n\r"))
    return FALSE;

  if (c->getCombatMode() == ATTACK_BERSERK) {
    c->sendTo("You are berserking! You can't focus enough to grapple anyone!\n\r");
    return FALSE;
  }
  if (victim == c) {
    c->sendTo("Aren't we funny today?\n\r");
    return FALSE;
  }
  if (c->noHarmCheck(victim))
    return FALSE;

  if (victim->isImmortal() || IS_SET(victim->specials.act, ACT_IMMORTAL)) {
    c->sendTo("You can't grapple an immortal.\n\r");
    return FALSE;
  }

  if (c->riding) {
    c->sendTo("Grappling while mounted is impossible!\n\r");
    return FALSE;
  }
  if (victim->isFlying()) {
    c->sendTo("You can't grapple someone that is flying.\n\r");
    return FALSE;
  }

  if (c->getMove() < grapple_move) {
    c->sendTo("You lack the vitality to grapple.\n\r");
    return FALSE;
  }
  c->addToMove(-grapple_move);

  percent = 0;

  percent += c->getDexReaction() * 5;
  percent -= victim->getAgiReaction() * 10;

  if (victim->riding) {
    // difficult
    percent -= 35;
  }
  level = c->getSkillLevel(skill);

  bKnown = c->getSkillValue(skill);

  if ((c->bSuccess(bKnown + percent, skill) &&
         // insure they can hit this critter
         (i = c->specialAttack(victim,skill)) && 
         i != GUARANTEED_FAILURE &&
         // make sure they have reasonable training
         (percent < bKnown)) ||
      !victim->awake()) {
    if (victim->canCounterMove(bKnown/2)) {
      SV(skill);
      act("$N blocks your grapple attempt and knocks you to the $g.", TRUE, c, 0, victim, TO_CHAR, ANSI_RED);
      act("$N blocks $n's attempt to grapple, and knocks $m to the $g.", TRUE, c, 0, victim, TO_NOTVICT);
      act("You evade $n's attempt to grapple, and knock $m to the $g.", TRUE, c, 0, victim, TO_VICT);
      c->cantHit += c->loseRound(5 - (min(50, level) / 12));

      rc = c->crashLanding(POSITION_SITTING);
      if (IS_SET_DELETE(rc, DELETE_THIS))
        return rc;

      rc = c->trySpringleap(victim);
      if (IS_SET_DELETE(rc, DELETE_THIS) || IS_SET_DELETE(rc, DELETE_VICT))
        return rc;
    } else {
      if (victim->riding) {
        act("You pull $N off $p.", FALSE, c, victim->riding, victim, TO_CHAR);
        act("$n pulls $N off $p.", FALSE, c, victim->riding, victim, TO_NOTVICT);
        act("$n pulls you off $p.", FALSE, c, victim->riding, victim, TO_VICT);
        victim->dismount(POSITION_STANDING);
      }
      c->sendTo("You tie your opponent up, with an excellent maneuver.\n\r");
      act("$n wrestles $N to the $g with an excellent maneuver.", TRUE, c, 0, victim, TO_NOTVICT);
      act("$n wrestles you to the $g.", TRUE, c, 0, victim, TO_VICT);


      // if eqipment worn on body is spiked, add a little extra 10-20-00, -dash
      if (((obj = c->equipment[WEAR_BODY]) &&
	   (tbc = dynamic_cast<TBaseClothing *>(obj)) &&
	   (tbc->isSpiked()||tbc->isObjStat(ITEM_SPIKED)))) {
	suitDam = (int)((tbc->getWeight()/10) + 1);

	act("The spikes on your $o sink into $N.", FALSE, c, tbc, victim, TO_CHAR);
	act("The spikes on $n's $o sink into $N.", FALSE, c, tbc, victim, TO_NOTVICT);
	act("The spikes on $n's $o sink into you.", FALSE, c, tbc, victim, TO_VICT);

	if (c->reconcileDamage(victim, suitDam, TYPE_STAB) == -1)
	  return DELETE_VICT;

      }

      rc = c->crashLanding(POSITION_SITTING);
      if (IS_SET_DELETE(rc, DELETE_THIS))
        return rc;

      rc = c->trySpringleap(victim);
      if (IS_SET_DELETE(rc, DELETE_THIS) || IS_SET_DELETE(rc, DELETE_VICT))
        return rc;
   
      rc = victim->crashLanding(POSITION_SITTING);
      if (IS_SET_DELETE(rc, DELETE_THIS))
        return DELETE_VICT;

      rc = victim->trySpringleap(c);
      if (IS_SET_DELETE(rc, DELETE_THIS) && IS_SET_DELETE(rc, DELETE_VICT))
        return rc;
      else if (IS_SET_DELETE(rc, DELETE_THIS))
        return DELETE_VICT;
      else if (IS_SET_DELETE(rc, DELETE_VICT))
        return DELETE_THIS;

      if (!victim->fight()) {
        if (c->fight()) {
          if (c->fight() != victim) {
            act("You now turn your attention to $N!", 
                TRUE, c, 0, victim, TO_CHAR);
            act("$n now turns $s attention to $N!", 
                TRUE, c, 0, victim, TO_NOTVICT);
            act("$n has turned $s attention to you!", 
                TRUE, c, 0, victim, TO_VICT);
          }
          c->stopFighting();
          c->setCharFighting(victim);
        } else {
          c->setCharFighting(victim);
        }
        c->setVictFighting(victim);
      } else if (::number(1,5) < 4) {
        if (c->fight() && (c->fight() != victim)) {
          act("You now turn your attention to $N!",
              TRUE, c, 0, victim, TO_CHAR);
          act("$n now turns $s attention to $N!",
              TRUE, c, 0, victim, TO_NOTVICT);
          act("$n has turned $s attention to you!",
              TRUE, c, 0, victim, TO_VICT);
        }
        if (victim->fight() && (victim->fight() != c)) {
          act("$N now turns $S attention to you!", 
              TRUE, c, 0, victim, TO_CHAR);
          act("$N now turns $S attention to $n!", 
              TRUE, c, 0, victim, TO_NOTVICT);
          act("You now turn your attention to $n!", 
              TRUE, c, 0, victim, TO_VICT);

        }
        if (c->fight())
          c->stopFighting();

        if (victim->fight())
          victim->stopFighting();
        c->setCharFighting(victim);
        c->setVictFighting(victim);
      }
      c->cantHit += c->loseRound(4);
      victim->cantHit += victim->loseRound(4);

      if (victim->spelltask) {
        victim->addToDistracted(1, FALSE);
      }
      victim->addToWait(combatRound(5));
    }
  } else {
    c->cantHit += c->loseRound(5 - (min(level, 50)/ 12));
   c->addToWait(combatRound(1));

    rc = c->crashLanding(POSITION_SITTING);
    if (IS_SET_DELETE(rc, DELETE_THIS))
      return rc;

    rc = c->trySpringleap(victim);
    if (IS_SET_DELETE(rc, DELETE_THIS) || IS_SET_DELETE(rc, DELETE_VICT))
      return rc;

    act("You try to wrestle $N to the $g, but end up falling on your butt.", TRUE, c, 0, victim, TO_CHAR);
    act("$n makes a nice wrestling move, but falls on $s butt.", TRUE, c, 0, 0, TO_ROOM);

    if (!victim->fight()) {
      act("$N turns $S attention to $n", TRUE, c, 0, victim, TO_NOTVICT);
      c->setCharFighting(victim);
      c->setVictFighting(victim);
    }
  }
  c->reconcileHurt(victim, 0.01);
  return TRUE;
}

int TBeing::doGrapple(const char *argument, TBeing *vict)
{
  int rc;
  char name_buf[30];
  TBeing *victim;

  spellNumT skill = getSkillNum(SKILL_GRAPPLE);

  if (checkBusy()) {
    return FALSE;
  }
  strcpy(name_buf, argument);
  if (!(victim = vict)) {
    if (!(victim = get_char_room_vis(this, name_buf)))  {
      if (!(victim = fight())) {
        sendTo("Grapple whom?\n\r");
        return FALSE;
      }
    }
  }

  if (!doesKnowSkill(skill)) {
    sendTo("You know nothing about grappling.\n\r");
    return FALSE;
  }

  if (!sameRoom(*victim)) {
    sendTo("That person isn't around.\n\r");
    return FALSE;
  }
  rc = grapple(this, victim, skill);
  if (rc)
    addSkillLag(skill, rc);
  if (IS_SET_DELETE(rc, DELETE_VICT)) {
    if (vict)
      return rc;
    delete victim;
    victim = NULL;
    REM_DELETE(rc, DELETE_VICT);
  }
  return rc;
}




