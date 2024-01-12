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

bool TBeing::canSpin(TBeing* victim, silentTypeT silent) {
  if (checkBusy())
    return false;

  if (!doesKnowSkill(SKILL_SPIN)) {
    if (!silent)
      sendTo("You know nothing about spinning.\n\r");
    return false;
  }
  if (!hasHands()) {
    if (!silent)
      sendTo("You need hands to spin someone.\n\r");
    return false;
  }
  if (!victim->hasHands()) {
    if (!silent)
      sendTo("You need something to grab to make this work.\n\r");
    return false;
  }
  if (eitherArmHurt()) {
    if (!silent)
      sendTo("You can't spin someone with an injured arm.\n\r");
    return false;
  }

  if (checkPeaceful("You feel too peaceful to contemplate violence.\n\r"))
    return false;

  if (getCombatMode() == ATTACK_BERSERK) {
    if (!silent)
      sendTo("You are berserking! You can't focus enough to spin anyone!\n\r ");
    return false;
  }

  if (victim->isFlying() && (victim->fight() != this)) {
    if (!silent)
      sendTo("You can only spin fliers that are fighting you.\n\r");
    return false;
  }
  if (victim == this) {
    if (!silent)
      sendTo("Don't be stupid...that would suck if you could.\n\r");
    return false;
  }
  if (noHarmCheck(victim))
    return false;

  if (riding) {
    if (!silent)
      sendTo("You can't spin someone while mounted!\n\r");
    return false;
  }
  if (!canUseArm(HAND_PRIMARY) || !canUseArm(HAND_SECONDARY)) {
    if (!silent)
      sendTo("You need two working arms to spin someone.\n\r");
    return false;
  }
  if (victim->isImmortal() || IS_SET(victim->specials.act, ACT_IMMORTAL)) {
    if (!silent)
      sendTo(
        "You can't successfully spin an immortal...unless you want to "
        "dance.\n\r");
    return false;
  }
  if (victim->getPosition() < POSITION_STANDING) {
    if (!silent)
      act("$N is on the $g.  You can't spin $M.", false, this, 0, victim,
        TO_CHAR);
    return false;
  }

  return true;
}

int TBeing::spinMiss(TBeing* victim, skillMissT type) {
  int rc;

  if (type == TYPE_DEX) {
    act("$N deftly avoids your attempt at spinning $M.", false, this, 0, victim,
      TO_CHAR);
    act("You deftly avoid $n's attempt at spinning you.", false, this, 0,
      victim, TO_VICT);
    act("$N deftly avoids $n's attempt at spinning $M.", false, this, 0, victim,
      TO_NOTVICT);
  } else if (type == TYPE_DEFENSE) {
    act("$N is too fast and avoids your attempt at spinning $M.", false, this,
      0, victim, TO_CHAR);
    act("Your defensive training helps you avoid $n's feeble spin attempt.",
      false, this, 0, victim, TO_VICT);
    act("$N deftly avoids $n's attempt at spinning $M.", false, this, 0, victim,
      TO_NOTVICT);

  } else if (type == TYPE_MONK) {
    act("$N deftly counters your attempt at spinning $M.", false, this, 0,
      victim, TO_CHAR, ANSI_RED);
    act("You trip and land on the $g.", false, this, 0, victim, TO_CHAR,
      ANSI_RED);
    act("You deftly counter $n's attempt at spinning you.", false, this, 0,
      victim, TO_VICT);
    act("You stick out your foot and trip $m to the $g.", false, this, 0,
      victim, TO_VICT);
    act("$N deftly counters $n's attempt at spinning $M.", false, this, 0,
      victim, TO_NOTVICT);
    act("$N sticks out $S foot tripping $n to the $g.", false, this, 0, victim,
      TO_NOTVICT);

    rc = crashLanding(POSITION_SITTING);
    if (IS_SET_DELETE(rc, DELETE_THIS))
      return rc;

    rc = trySpringleap(victim);
    if (IS_SET_DELETE(rc, DELETE_THIS) || IS_SET_DELETE(rc, DELETE_VICT))
      return rc;
  } else {
    act("$n tries to spin $N, but ends up falling down.", false, this, 0,
      victim, TO_NOTVICT);
    act("You try to spin $N, but end up falling on your face.", false, this, 0,
      victim, TO_CHAR);
    act("$n fails to spin you, and tumbles to the $g.", false, this, 0, victim,
      TO_VICT);

    rc = crashLanding(POSITION_SITTING);
    if (IS_SET_DELETE(rc, DELETE_THIS))
      return rc;

    rc = trySpringleap(victim);
    if (IS_SET_DELETE(rc, DELETE_THIS) || IS_SET_DELETE(rc, DELETE_VICT))
      return rc;
  }

  if (reconcileDamage(victim, 0, SKILL_SPIN) == -1)
    return DELETE_VICT;

  return false;
}

int TBeing::spinHit(TBeing* victim) {
  int rc;

  if (!victim->riding) {
    act("$n grabs $N's arm and spins $M!", false, this, 0, victim, TO_NOTVICT);
    act("Now dizzy, $N trips and falls to the $g.", false, this, 0, victim,
      TO_NOTVICT);
    act("You grab $N's arm and spin $M HARD!", false, this, 0, victim, TO_CHAR);
    act("Now dizzy $N trips and falls to the $g.", false, this, 0, victim,
      TO_CHAR);
    act("$n grabs you by the arm and spins you violently.", false, this, 0,
      victim, TO_VICT);
    act(
      "As the world spins into a blur before your eyes you become "
      "dazed,\n\rand fall face first to the $g.",
      false, this, 0, victim, TO_VICT, ANSI_RED);
  } else {
    act("$n grabs $N's arm and rips $M off of $S $o!", false, this,
      victim->riding, victim, TO_NOTVICT);
    act("$N slams head first into the $g.", false, this, victim->riding, victim,
      TO_NOTVICT);
    act("You grab $N's arm and pull $M off of $S $o!", false, this,
      victim->riding, victim, TO_CHAR);
    act("$N slams head first into the $g.", false, this, victim->riding, victim,
      TO_CHAR);
    act("$n suddenly grabs your arm and gives a hard yank!", false, this,
      victim->riding, victim, TO_VICT);
    act("Suddenly, the $g rushes upward as you fall off of your $o.", false,
      this, victim->riding, victim, TO_VICT, ANSI_RED);
    act("OOFFF!! Yuck, dirt tastes AWFUL!", false, this, victim->riding, victim,
      TO_VICT, ANSI_RED);
    victim->dismount(POSITION_RESTING);
  }

  rc = victim->crashLanding(POSITION_SITTING);
  if (IS_SET_DELETE(rc, DELETE_THIS))
    return DELETE_VICT;

  rc = victim->trySpringleap(this);
  if (IS_SET_DELETE(rc, DELETE_THIS) && IS_SET_DELETE(rc, DELETE_VICT))
    return rc;
  else if (IS_SET_DELETE(rc, DELETE_THIS))
    return DELETE_VICT;
  else if (IS_SET_DELETE(rc, DELETE_VICT))
    return DELETE_THIS;

  // see the balance notes for details on what's going on here.
  float wt = (combatRound(discArray[SKILL_SPIN]->lag) / 3);

  // since we cost some moves to perform, allow an extra 1/2 round of lag
  wt += 1.5;

  // since success and failure both have reciprocal positional changes
  // there is no reason to account for that here.

  victim->addToWait((int)wt);

  int dam = getSkillDam(victim, SKILL_SPIN, getSkillLevel(SKILL_SPIN),
    getAdvLearning(SKILL_SPIN));

  if (reconcileDamage(victim, dam, SKILL_SPIN) == -1)
    return DELETE_VICT;

  return true;
}

int TBeing::spin(TBeing* victim) {
  int rc;
  int flycheck = ::number(1, 10);
  const int SPIN_COST = 6;  // movement cost to spin

  if (!canSpin(victim, SILENT_NO))
    return false;

  if (victim->isFlying()) {
    if (flycheck > 5) {
      act("Your spin attempt on $N is more difficult because $E is flying.",
        false, this, 0, victim, TO_CHAR, ANSI_YELLOW);
      act(
        "The fact that you are flying makes $n's spin attempt much more "
        "difficult.",
        false, this, 0, victim, TO_VICT, ANSI_YELLOW);
      act("The fact that $N is flying makes $n's spin attempt more difficult.",
        false, this, 0, victim, TO_NOTVICT, ANSI_YELLOW);
      return (spinMiss(victim, TYPE_DEFAULT));
    } else {
      act("Your spin attempt on $N is more difficult because $E is flying.",
        false, this, 0, victim, TO_CHAR, ANSI_YELLOW);
      act(
        "The fact that you are flying makes $n's spin attempt much more "
        "difficult.",
        false, this, 0, victim, TO_VICT, ANSI_YELLOW);
      act("The fact that $N is flying makes $n's spin attempt more difficult.",
        false, this, 0, victim, TO_NOTVICT, ANSI_YELLOW);
      // continue spinning
      // the above is to make spin less annoying to flyers
    }
  }
  if (getMove() < SPIN_COST) {
    sendTo("You don't have the vitality to spin anyone!\n\r");
    return false;
  }
  addToMove(-SPIN_COST);

  int bKnown = getSkillValue(SKILL_SPIN);
  int successfulHit = specialAttack(victim, SKILL_SPIN);
  int successfulSkill = bSuccess(bKnown, SKILL_SPIN);

  // Success case
  if (!victim->awake() || (successfulSkill && successfulHit &&
                            successfulHit != GUARANTEED_FAILURE)) {
    // Allow victim a chance to counter
    if (victim->canCounterMove(bKnown / 3)) {
      SV(SKILL_SPIN);
      rc = spinMiss(victim, TYPE_MONK);
      if (IS_SET_DELETE(rc, DELETE_THIS) || IS_SET_DELETE(rc, DELETE_VICT))
        return rc;
      // Allow victim a chance to successfully avoid via focused avoidance
    } else if (victim->canFocusedAvoidance(bKnown / 3)) {
      SV(SKILL_SPIN);
      rc = spinMiss(victim, TYPE_DEFENSE);
      if (IS_SET_DELETE(rc, DELETE_THIS) || IS_SET_DELETE(rc, DELETE_VICT))
        return rc;
      // Successful hit
    } else
      return spinHit(victim);
    // Failure case
  } else {
    rc = spinMiss(victim, TYPE_DEFAULT);
    if (IS_SET_DELETE(rc, DELETE_THIS) || IS_SET_DELETE(rc, DELETE_VICT))
      return rc;
  }

  return true;
}

int TBeing::doSpin(const char* argument, TBeing* vict) {
  int rc = 0, learning = 0;
  TBeing* victim;
  char name_buf[256];

  strcpy(name_buf, argument);

  if (!(victim = vict)) {
    if (!(victim = get_char_room_vis(this, name_buf))) {
      if (!(victim = fight())) {
        sendTo("Spin whom?\n\r");
        return false;
      }
    }
  }
  if (!sameRoom(*victim)) {
    sendTo("That person isn't around.\n\r");
    return false;
  }
  if (desc) {
    if ((learning = getAdvLearning(SKILL_SPIN)) <= 25) {
      if (heldInPrimHand() || heldInSecHand()) {
        sendTo(
          "You are not skilled enough to spin someone with something in your "
          "hands!\n\r");
        return false;
      }
    } else if (learning <= 50) {
      if (heldInPrimHand()) {
        sendTo(
          "You are not skilled enough to spin someone with something in your "
          "primary hand!\n\r");
        return false;
      }
    } else {
      // no restrictions
    }
  }
  rc = spin(victim);
  if (rc)
    addSkillLag(SKILL_SPIN, rc);
  if (IS_SET_DELETE(rc, DELETE_VICT)) {
    if (vict)
      return rc;
    delete victim;
    victim = nullptr;
    REM_DELETE(rc, DELETE_VICT);
  }
  return rc;
}
