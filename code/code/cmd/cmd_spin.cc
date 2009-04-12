//////////////////////////////////////////////////////////////////////////
//
// SneezyMUD - All rights reserved, SneezyMUD Coding Team
//
//////////////////////////////////////////////////////////////////////////


#include "handler.h"
#include "extern.h"
#include "being.h"
#include "enum.h"
#include "combat.h"

bool TBeing::canSpin(TBeing *victim, silentTypeT silent)
{
  if (checkBusy())
    return FALSE;

  if (!doesKnowSkill(SKILL_SPIN)) {
    if (!silent)
      sendTo("You know nothing about spinning.\n\r");
    return FALSE;
  }
  if (!hasHands()) {
    if (!silent)
      sendTo("You need hands to spin someone.\n\r");
    return FALSE;
  }
  if (!victim->hasHands()) {
    if (!silent)
      sendTo("You need something to grab to make this work.\n\r");
    return FALSE;
  }
  if (eitherArmHurt()) {
    if (!silent)
      sendTo("You can't spin someone with an injured arm.\n\r");
    return FALSE;
  }

  if (checkPeaceful("You feel too peaceful to contemplate violence.\n\r"))
    return FALSE;
  
  if (getCombatMode() == ATTACK_BERSERK) {
    if (!silent)
      sendTo("You are berserking! You can't focus enough to spin anyone!\n\r ");
    return FALSE;
  }

  if (victim->isFlying() && (victim->fight() != this)) {
    if (!silent)
      sendTo("You can only spin fliers that are fighting you.\n\r");
    return FALSE;
  }
  if (victim == this) {
    if (!silent)
      sendTo("Don't be stupid...that would suck if you could.\n\r");
    return FALSE;
  }
  if (noHarmCheck(victim))
    return FALSE;

  if (riding) {
    if (!silent)
      sendTo("You can't spin someone while mounted!\n\r");
    return FALSE;
  }
  if (!canUseArm(HAND_PRIMARY) || !canUseArm(HAND_SECONDARY)) {
    if (!silent)
      sendTo("You need two working arms to spin someone.\n\r");
    return FALSE;
  }
  if (victim->isImmortal() || IS_SET(victim->specials.act, ACT_IMMORTAL)) {
    if (!silent)
      sendTo("You can't successfully spin an immortal...unless you want to dance.\n\r");
    return FALSE;
  }
  if (victim->getPosition() < POSITION_STANDING) {
    if (!silent)
      act("$N is on the $g.  You can't spin $M.",
          FALSE, this, 0, victim, TO_CHAR);
    return FALSE;
  }

  return TRUE;
}

enum spinMissT {
    TYPE_DEFAULT,
    TYPE_DEX,
    TYPE_MONK
};

static int spinMiss(TBeing *caster, TBeing *victim, spinMissT type)
{
  int rc;

  if (type == TYPE_DEX) {
    act("$N deftly avoids your attempt at spinning $M.", FALSE, caster, 
              0, victim, TO_CHAR);
    act("You deftly avoid $n's attempt at spinning you.", FALSE, caster, 
              0, victim, TO_VICT);
    act("$N deftly avoids $n's attempt at spinning $M.", FALSE, caster, 
              0, victim, TO_NOTVICT);
  } else if (type == TYPE_MONK) {
    act("$N deftly counters your attempt at spinning $M.", FALSE, caster, 0, victim, TO_CHAR, ANSI_RED);
    act("You trip and land on the $g.", FALSE, caster, 0, victim, TO_CHAR, ANSI_RED);
    act("You deftly counter $n's attempt at spinning you.", FALSE, caster, 0, victim, TO_VICT);
    act("You stick out your foot and trip $m to the $g.", FALSE, caster, 0, victim, TO_VICT);
    act("$N deftly counters $n's attempt at spinning $M.", FALSE, caster, 0, victim, TO_NOTVICT);
    act("$N sticks out $S foot tripping $n to the $g.", FALSE, caster, 0, victim, TO_NOTVICT);

    rc = caster->crashLanding(POSITION_SITTING);
    if (IS_SET_DELETE(rc, DELETE_THIS))
      return rc;

    rc = caster->trySpringleap(victim);
    if (IS_SET_DELETE(rc, DELETE_THIS) || IS_SET_DELETE(rc, DELETE_VICT))
      return rc;
  } else {
    act("$n tries to spin $N, but ends up falling down.", FALSE, caster, 0, victim, TO_NOTVICT);
    act("You try to spin $N, but end up falling on your face.", FALSE, caster, 0, victim, TO_CHAR);
    act("$n fails to spin you, and tumbles to the $g.", FALSE, caster, 0, victim, TO_VICT);

    rc = caster->crashLanding(POSITION_SITTING);
    if (IS_SET_DELETE(rc, DELETE_THIS))
      return rc;

    rc = caster->trySpringleap(victim);
    if (IS_SET_DELETE(rc, DELETE_THIS) || IS_SET_DELETE(rc, DELETE_VICT))
      return rc;
  }

  if (caster->reconcileDamage(victim, 0,SKILL_SPIN) == -1)
    return DELETE_VICT;

  return FALSE;
}

static int spinHit(TBeing *caster, TBeing *victim)
{
  int rc;

  if (!victim->riding) {
    act("$n grabs $N's arm and spins $M!", FALSE, caster, 0, victim, TO_NOTVICT);
    act("Now dizzy, $N trips and falls to the $g.", FALSE, caster, 0, victim, TO_NOTVICT);
    act("You grab $N's arm and spin $M HARD!", FALSE, caster, 0, victim, TO_CHAR);
    act("Now dizzy $N trips and falls to the $g.", FALSE, caster, 0, victim, TO_CHAR);
    act("$n grabs you by the arm and spins you violently.", FALSE, caster, 0, victim, TO_VICT);
    act("As the world spins into a blur before your eyes you become dazed,\n\rand fall face first to the $g.",
           FALSE, caster, 0, victim, TO_VICT, ANSI_RED);
  } else {
    act("$n grabs $N's arm and rips $M off of $S $o!", FALSE, caster, victim->riding, victim, TO_NOTVICT);
    act("$N slams head first into the $g.", FALSE, caster, victim->riding, victim, TO_NOTVICT);
    act("You grab $N's arm and pull $M off of $S $o!", FALSE, caster, victim->riding, victim, TO_CHAR);
    act("$N slams head first into the $g.", FALSE, caster, victim->riding, victim, TO_CHAR);
    act("$n suddenly grabs your arm and gives a hard yank!", FALSE, caster, victim->riding, victim, TO_VICT);
    act("Suddenly, the $g rushes upward as you fall off of your $o.", FALSE, caster, victim->riding, victim, TO_VICT, ANSI_RED);
    act("OOFFF!! Yuck, dirt tastes AWFUL!", FALSE, caster, victim->riding, victim, TO_VICT, ANSI_RED);
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
  float wt = (combatRound(discArray[SKILL_SPIN]->lag) / 3);

  // adjust based on bash difficulty
  wt = (wt * 100.0 / getSkillDiffModifier(SKILL_SPIN));

  // since we cost some moves to perform, allow an extra 1/2 round of lag
  wt += 1.5;

  // since success and failure both have reciprocal positional changes
  // there is no reason to account for that here.

  victim->addToWait((int) wt);

  // round up
  wt += 0.5;

  int dam = caster->getSkillDam(victim, SKILL_SPIN, caster->getSkillLevel(SKILL_SPIN), caster->getAdvLearning(SKILL_SPIN));

  if (caster->reconcileDamage(victim, dam,SKILL_SPIN) == -1)
    return DELETE_VICT;

  return TRUE;
}

static int spin(TBeing *caster, TBeing *victim)
{
  int percent;
  int i = 0;
  int rc;
  int flycheck = ::number(1, 10);
  const int SPIN_COST = 13;       // movement cost to spin

  if (!caster->canSpin(victim, SILENT_NO))
    return FALSE;

  // AC makes less difference here ... 
  percent = ((10 + (victim->getArmor() / 200)) << 1);
  int bKnown = caster->getSkillValue(SKILL_SPIN);

  if (victim->isFlying()) {
    if (flycheck > 5) {
      act("Your spin attempt on $N is more difficult because $E is flying.", FALSE, caster, 0, victim, TO_CHAR, ANSI_YELLOW);
      act("The fact that you are flying makes $n's spin attempt much more difficult.", FALSE, caster, 0, victim, TO_VICT, ANSI_YELLOW);
      act("The fact that $N is flying makes $n's spin attempt more difficult.", FALSE, caster, 0, victim, TO_NOTVICT, ANSI_YELLOW);
      return (spinMiss(caster, victim, TYPE_DEFAULT));
    } else {
      act("Your spin attempt on $N is more difficult because $E is flying.", FALSE, caster, 0, victim, TO_CHAR, ANSI_YELLOW);
      act("The fact that you are flying makes $n's spin attempt much more difficult.", FALSE, caster, 0, victim, TO_VICT, ANSI_YELLOW);
      act("The fact that $N is flying makes $n's spin attempt more difficult.", FALSE, caster, 0, victim, TO_NOTVICT, ANSI_YELLOW);
      // continue spinning
      // the above is to make spin less annoying to flyers
    }
  }
  if (caster->getMove() < SPIN_COST) {
    caster->sendTo("You don't have the vitality to spin anyone!\n\r");
    return FALSE;
  }
  caster->addToMove(-SPIN_COST);

  if (victim->getPosition() <= POSITION_INCAP) 
    return (spinHit(caster, victim));
  
  // remember, F = MA :) need to take  weight into account 
  if (caster->bSuccess(bKnown + percent, SKILL_SPIN) &&
         (i = caster->specialAttack(victim,SKILL_SPIN)) && 
         i != GUARANTEED_FAILURE &&
         (percent < bKnown))  {
    int modif = 1;
    modif += (caster->getPrimaryHold() ? 0 : 2);
    modif += (caster->getSecondaryHold() ? 0 : 1);

    if (victim->canCounterMove(bKnown/3)) {
      SV(SKILL_SPIN);
      rc = spinMiss(caster, victim, TYPE_MONK);
      if (IS_SET_DELETE(rc, DELETE_THIS) || IS_SET_DELETE(rc, DELETE_VICT))
        return rc;
    } else if (((caster->getDexReaction() - 
                victim->getAgiReaction()) > ::number(-10,20)) &&
               victim->awake() && victim->getPosition() >= POSITION_STANDING) {
      CS(SKILL_SPIN);
      rc = spinMiss(caster, victim, TYPE_DEX);
      if (IS_SET_DELETE(rc, DELETE_THIS) || IS_SET_DELETE(rc, DELETE_VICT))
        return rc;
    } else 
      return spinHit(caster, victim);
  } else {
    rc = spinMiss(caster, victim, TYPE_DEFAULT);
    if (IS_SET_DELETE(rc, DELETE_THIS) || IS_SET_DELETE(rc, DELETE_VICT))
      return rc;
  }
   
  return TRUE;
}

int TBeing::doSpin(const char *argument, TBeing *vict)
{
  int rc = 0, learning = 0;
  TBeing *victim;
  char name_buf[256];
  
  strcpy(name_buf, argument);
  
  if (!(victim = vict)) {
    if (!(victim = get_char_room_vis(this, name_buf))) {
      if (!(victim = fight())) {
        sendTo("Spin whom?\n\r");
        return FALSE;
      }
    }
  }
  if (!sameRoom(*victim)) {
    sendTo("That person isn't around.\n\r");
    return FALSE;
  }
  if (desc) {
    if ((learning = getAdvLearning(SKILL_SPIN)) <= 25) {
      if (heldInPrimHand() || heldInSecHand()) {
        sendTo("You are not skilled enough to spin someone with something in your hands!\n\r");
        return FALSE;
      }
    } else if (learning <= 50) {
      if (heldInPrimHand()) {
        sendTo("You are not skilled enough to spin someone with something in your primary hand!\n\r");
        return FALSE;
      }
    } else {
      // no restrictions
    }
  }
  rc = spin(this, victim);
  if (rc)
    addSkillLag(SKILL_SPIN, rc);
  if (IS_SET_DELETE(rc, DELETE_VICT)) {
    if (vict)
      return rc;
    delete victim;
    victim = NULL;
    REM_DELETE(rc, DELETE_VICT);
  }
  return rc;
}



