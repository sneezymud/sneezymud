//////////////////////////////////////////////////////////////////////////
//
// SneezyMUD - All rights reserved, SneezyMUD Coding Team
//
//////////////////////////////////////////////////////////////////////////


#include "stdsneezy.h"
#include "combat.h"

bool TBeing::canBodyslam(TBeing *victim, silentTypeT silent)
{
  if (checkBusy())
    return FALSE;

  if (!doesKnowSkill(SKILL_BODYSLAM)) {
    if (!silent)
      sendTo("You know nothing about bodyslamming.\n\r");
    return FALSE;
  }
  if (!hasHands()) {
    if (!silent)
      sendTo("You need hands to bodyslam.\n\r");
    return FALSE;
  }
  if (eitherArmHurt()) {
    if (!silent)
      sendTo("You can't bodyslam with an injured arm.\n\r");
    return FALSE;
  }

  if (checkPeaceful("You feel too peaceful to contemplate violence.\n\r"))
    return FALSE;
  
  if (getCombatMode() == ATTACK_BERSERK) {
    if (!silent)
      sendTo("You are berserking! You can't focus enough to bodyslam anyone!\n\r ");
    return FALSE;
  }

  if (victim->isFlying() && (victim->fight() != this)) {
    if (!silent)
      sendTo("You can only bodyslam fliers that are fighting you.\n\r");
    return FALSE;
  }
  if (victim == this) {
    if (!silent)
      sendTo("You lack the agility to bodyslam yourself!\n\r");
    return FALSE;
  }
  if (noHarmCheck(victim))
    return FALSE;

  if (riding) {
    if (!silent)
      sendTo("You can't bodyslam while mounted!\n\r");
    return FALSE;
  }
  if (!canUseArm(HAND_PRIMARY) || !canUseArm(HAND_SECONDARY)) {
    if (!silent)
      sendTo("You need two working arms to bodyslam someone.\n\r");
    return FALSE;
  }
  if (victim->isImmortal() || IS_SET(victim->specials.act, ACT_IMMORTAL)) {
    if (!silent)
      sendTo("You can't successfully bodyslam an immortal.\n\r");
    return FALSE;
  }
  if (victim->getPosition() < POSITION_STANDING) {
    if (!silent)
      act("$N is already on the $g.  You can't bodyslam $M.",
          FALSE, this, 0, victim, TO_CHAR);
    return FALSE;
  }

  return TRUE;
}

enum bodySlamMissT {
    TYPE_DEFAULT,
    TYPE_DEX,
    TYPE_STR,
    TYPE_MONK
};

static int bodyslamMiss(TBeing *caster, TBeing *victim, bodySlamMissT type)
{
  int rc;

  if (type == TYPE_DEX) {
    act("$N deftly avoids your bodyslam attempt.", FALSE, caster, 
              0, victim, TO_CHAR);
    act("You deftly avoid $n's bodyslam attempt.", FALSE, caster, 
              0, victim, TO_VICT);
    act("$N deftly avoids $n's bodyslam attempt.", FALSE, caster, 
              0, victim, TO_NOTVICT);
  } else if (type == TYPE_MONK) {
    act("$N deftly counters your attempt, throwing you to the $g.", 
              FALSE, caster, 0, victim, TO_CHAR, ANSI_RED);
    act("You deftly counter $n's bodyslam attempt, and throw $m to the $g.", 
              FALSE, caster, 0, victim, TO_VICT);
    act("$N deftly counters $n's bodyslam attempt, and heaves $m to the $g.",
              FALSE, caster, 0, victim, TO_NOTVICT);

    rc = caster->crashLanding(POSITION_SITTING);
    if (IS_SET_DELETE(rc, DELETE_THIS))
      return rc;

    rc = caster->trySpringleap(victim);
    if (IS_SET_DELETE(rc, DELETE_THIS) || IS_SET_DELETE(rc, DELETE_VICT))
      return rc;
  } else if (type == TYPE_STR) {
    act("$n collapses as $e fails to pick $N up.", FALSE, caster, 
              0, victim, TO_NOTVICT);
    act("Your strength gives out as you try to pick $N up for bodyslamming.", 
              FALSE, caster, 0, victim, TO_CHAR);
    act("$n's strength gives out as $e tries to pick you up for bodyslamming.",
              FALSE, caster, 0, victim, TO_VICT);

    rc = caster->crashLanding(POSITION_SITTING);
    if (IS_SET_DELETE(rc, DELETE_THIS))
      return rc;

    caster->sendTo(fmt("%sYou fall to the %s.%s\n\r") % caster->blue() % caster->roomp->describeGround() % caster->norm());

    rc = caster->trySpringleap(victim);
    if (IS_SET_DELETE(rc, DELETE_THIS) || IS_SET_DELETE(rc, DELETE_VICT))
      return rc;
  } else {
    act("$n tries to bodyslam $N, but ends up falling down.", FALSE, 
              caster, 0, victim, TO_NOTVICT);
    act("You try to bodyslam $N, but end up falling on your face.",
                         FALSE, caster, 0, victim, TO_CHAR);
    act("$n fails to bodyslam you, and tumbles to the $g.", FALSE, 
              caster, 0, victim, TO_VICT);

    rc = caster->crashLanding(POSITION_SITTING);
    if (IS_SET_DELETE(rc, DELETE_THIS))
      return rc;

    rc = caster->trySpringleap(victim);
    if (IS_SET_DELETE(rc, DELETE_THIS) || IS_SET_DELETE(rc, DELETE_VICT))
      return rc;
  }

  if (caster->reconcileDamage(victim, 0,SKILL_BODYSLAM) == -1)
    return DELETE_VICT;

  return FALSE;
}

static int bodyslamHit(TBeing *caster, TBeing *victim)
{
  int rc;

  if (!victim->riding) {
    act("$n lifts $N over $s head and slams $M to the $g.",
           FALSE, caster, 0, victim, TO_NOTVICT);
    act("You lift $N over your head and slam $M to the $g.",
           FALSE, caster, 0, victim, TO_CHAR);
    act("You get a great view as $n lifts you over $s head.",
           FALSE, caster, 0, victim, TO_VICT);
    act("Suddenly, the $g rushes upward and knocks the wind out of you!",
           FALSE, caster, 0, victim, TO_VICT, ANSI_RED);
  } else {
    act("$n lifts $N off $S $o and slams $M to the $g.",
           FALSE, caster, victim->riding, victim, TO_NOTVICT);
    act("You lift $N off $S $o and slam $M to the $g.",
           FALSE, caster, victim->riding, victim, TO_CHAR);
    act("You get a great view as $n lifts you off your $o over $s head.",
           FALSE, caster, victim->riding, victim, TO_VICT);
    act("Suddenly, the $g rushes upward and knocks the wind out of you!",
           FALSE, caster, victim->riding, victim, TO_VICT, ANSI_RED);
    victim->dismount(POSITION_RESTING);
  }

  rc = victim->crashLanding(POSITION_SITTING);
  if (IS_SET_DELETE(rc, DELETE_THIS))
    return DELETE_VICT;

  rc = victim->trySpringleap(caster);
  if (IS_SET_DELETE(rc, DELETE_THIS) && IS_SET_DELETE(rc, DELETE_VICT))
    return rc;
  else if (IS_SET_DELETE(rc, DELETE_THIS))
    return DELETE_VICT;
  else if (IS_SET_DELETE(rc, DELETE_VICT))
    return DELETE_THIS;

  // see the balance notes for details on what's going on here.
  float wt = combatRound(discArray[SKILL_BODYSLAM]->lag);
  // adjust based on bash difficulty
  wt = (wt * 100.0 / getSkillDiffModifier(SKILL_BODYSLAM));

  // since we cost some moves to perform, allow an extra 1/2 round of lag
  wt += 1.5;

  // since success and failure both have reciprocal positional changes
  // there is no reason to account for that here.

  // round up
  wt += 0.5;

  victim->addToWait((int) wt);

  // in general, we should not do BOTH damage and command lock-out
  // however, since Bslam has nasty requirements on strength and
  // dex to lift person up, doing this damage will counter-balance
  // those penalties.  Warrior-skill damage isn't all that high
  // to begin with...
  int dam = caster->getSkillDam(victim, SKILL_BODYSLAM, caster->getSkillLevel(SKILL_BODYSLAM), caster->getAdvLearning(SKILL_BODYSLAM));

  if (caster->reconcileDamage(victim, dam,SKILL_BODYSLAM) == -1)
    return DELETE_VICT;

  return TRUE;
}

static int bodyslam(TBeing *caster, TBeing *victim)
{
  int percent;
  int i = 0;
  int rc;
  const int BODYSLAM_COST = 30;       // movement cost to slam

  if (!caster->canBodyslam(victim, SILENT_NO))
    return FALSE;

  // AC makes less difference here ... 
  percent = ((10 + (victim->getArmor() / 200)) << 1);
  int bKnown = caster->getSkillValue(SKILL_BODYSLAM);

  if (caster->getMove() < BODYSLAM_COST) {
    caster->sendTo("You don't have the vitality to bodyslam anyone!\n\r");
    return FALSE;
  }
  caster->addToMove(-BODYSLAM_COST);

  if (victim->getPosition() <= POSITION_INCAP) 
    return (bodyslamHit(caster, victim));
  
  // remember, F = MA :) need to take  weight into account 
  if (caster->bSuccess(bKnown + percent, SKILL_BODYSLAM) &&
         (i = caster->specialAttack(victim,SKILL_BODYSLAM)) && 
         i != GUARANTEED_FAILURE &&
         (percent < bKnown))  {
    int modif = 1;
    modif += (caster->getPrimaryHold() ? 0 : 2);
    modif += (caster->getSecondaryHold() ? 0 : 1);

    if (victim->canCounterMove(bKnown/3)) {
      SV(SKILL_BODYSLAM);
      rc = bodyslamMiss(caster, victim, TYPE_MONK);
      if (IS_SET_DELETE(rc, DELETE_THIS) || IS_SET_DELETE(rc, DELETE_VICT))
        return rc;
    } else if (((caster->getDexReaction() - 
                victim->getAgiReaction()) > ::number(-10,20)) &&
               victim->awake() && victim->getPosition() >= POSITION_STANDING) {
      CS(SKILL_BODYSLAM);
      rc = bodyslamMiss(caster, victim, TYPE_DEX);
      if (IS_SET_DELETE(rc, DELETE_THIS) || IS_SET_DELETE(rc, DELETE_VICT))
        return rc;
    } else if (compareWeights(victim->getTotalWeight(TRUE), 
                              caster->carryWeightLimit() * modif) == -1) {
      CF(SKILL_BODYSLAM);
      rc = bodyslamMiss(caster, victim, TYPE_STR);
      if (IS_SET_DELETE(rc, DELETE_THIS) || IS_SET_DELETE(rc, DELETE_VICT))
        return rc;
    } else 
      return bodyslamHit(caster, victim);
  } else {
    rc = bodyslamMiss(caster, victim, TYPE_DEFAULT);
    if (IS_SET_DELETE(rc, DELETE_THIS) || IS_SET_DELETE(rc, DELETE_VICT))
      return rc;
  }
   
  return TRUE;
}

int TBeing::doBodyslam(const char *argument, TBeing *vict)
{
  int rc = 0, learning = 0;
  TBeing *victim;
  char name_buf[256];
  
  strcpy(name_buf, argument);
  
  if (!(victim = vict)) {
    if (!(victim = get_char_room_vis(this, name_buf))) {
      if (!(victim = fight())) {
        sendTo("Bodyslam whom?\n\r");
        return FALSE;
      }
    }
  }
  if (!sameRoom(*victim)) {
    sendTo("That person isn't around.\n\r");
    return FALSE;
  }
  if (desc) {
    if ((learning = getAdvLearning(SKILL_BODYSLAM)) <= 40) {
      if (heldInPrimHand() || heldInSecHand()) {
        sendTo("You are not skilled enough to bodyslam with something in your hands!\n\r");
        return FALSE;
      }
    } else if (learning <= 75) {
      if (heldInPrimHand()) {
        sendTo("You are not skilled enough to bodyslam with something in your primary hand!\n\r");
        return FALSE;
      }
    } else {
      // no restrictions
    }
  }
  rc = bodyslam(this, victim);
  if (rc)
    addSkillLag(SKILL_BODYSLAM, rc);
  if (IS_SET_DELETE(rc, DELETE_VICT)) {
    if (vict)
      return rc;
    delete victim;
    victim = NULL;
    REM_DELETE(rc, DELETE_VICT);
  }
  return rc;
}

