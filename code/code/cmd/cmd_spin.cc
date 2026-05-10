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
      sendTo(
        "You can't successfully spin an immortal...unless you want to "
        "dance.\n\r");
    return FALSE;
  }
  if (victim->getPosition() < POSITION_STANDING) {
    if (!silent)
      act("$N is on the $g.  You can't spin $M.", FALSE, this, 0, victim,
        TO_CHAR);
    return FALSE;
  }

  return TRUE;
}

int TBeing::spinMiss(TBeing* victim, skillMissT type) {
  if (type == TYPE_DEX) {
    act("$N deftly avoids your attempt at spinning $M.", FALSE, this, 0, victim,
      TO_CHAR);
    act("You deftly avoid $n's attempt at spinning you.", FALSE, this, 0,
      victim, TO_VICT);
    act("$N deftly avoids $n's attempt at spinning $M.", FALSE, this, 0, victim,
      TO_NOTVICT);
  } else if (type == TYPE_DEFENSE) {
    act("$N is too fast and avoids your attempt at spinning $M.", FALSE, this,
      0, victim, TO_CHAR);
    act("Your defensive training helps you avoid $n's feeble spin attempt.",
      FALSE, this, 0, victim, TO_VICT);
    act("$N deftly avoids $n's attempt at spinning $M.", FALSE, this, 0, victim,
      TO_NOTVICT);

  } else if (type == TYPE_MONK) {
    act("$N deftly counters your attempt at spinning $M.", FALSE, this, 0,
      victim, TO_CHAR, ANSI_RED);
    act("$N sticks out $S foot and trips you, toppling you.", false, this, 0,
      victim, TO_CHAR, ANSI_RED);
    act("You deftly counter $n's attempt at spinning you.", FALSE, this, 0,
      victim, TO_VICT);
    act("You stick out your foot and trip $m, toppling them.", false, this, 0,
      victim, TO_VICT);
    act("$N deftly counters $n's attempt at spinning $M.", FALSE, this, 0,
      victim, TO_NOTVICT);
    act("$N sticks out $S foot and trips $n, toppling $m.", false, this, 0,
      victim, TO_NOTVICT);

    int rc = stumble();
    if (IS_SET_DELETE(rc, DELETE_THIS))
      return rc;
  } else {
    act("$n tries to spin $N but loses $s footing.", false, this, 0, victim,
      TO_NOTVICT);
    act("You try to spin $N but lose your footing.", false, this, 0, victim,
      TO_CHAR);
    act("$n tries to spin you but loses $s footing.", false, this, 0, victim,
      TO_VICT);

    int rc = stumble();
    if (IS_SET_DELETE(rc, DELETE_THIS))
      return rc;
  }

  if (reconcileDamage(victim, 0, SKILL_SPIN) == -1)
    return DELETE_VICT;

  return FALSE;
}

int TBeing::spinHit(TBeing* victim) {
  const bool wasMounted = (victim->riding != nullptr);

  // Describe the wind-up only — crashLanding/knockOffMount narrates the result.
  act("You take ahold of $N and pull hard!", false, this, nullptr, victim,
    TO_CHAR);
  act("$n takes ahold of you and pulls hard!", false, this, nullptr, victim,
    TO_VICT, ANSI_RED);
  act("$n takes ahold of $N and pulls hard!", false, this, nullptr, victim,
    TO_NOTVICT);

  int rc = wasMounted ? victim->knockOffMount(getSkillValue(SKILL_SPIN) / 3)
                      : victim->crashLanding();
  if (IS_SET_DELETE(rc, DELETE_THIS))
    return DELETE_VICT;

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

  return TRUE;
}

int TBeing::spin(TBeing* victim) {
  int rc;
  int flycheck = ::number(1, 10);
  const int SPIN_COST = 6;  // movement cost to spin

  if (!canSpin(victim, SILENT_NO))
    return FALSE;

  if (victim->isFlying()) {
    if (flycheck > 5) {
      act("Your spin attempt on $N is more difficult because $E is flying.",
        FALSE, this, 0, victim, TO_CHAR, ANSI_YELLOW);
      act(
        "The fact that you are flying makes $n's spin attempt much more "
        "difficult.",
        FALSE, this, 0, victim, TO_VICT, ANSI_YELLOW);
      act("The fact that $N is flying makes $n's spin attempt more difficult.",
        FALSE, this, 0, victim, TO_NOTVICT, ANSI_YELLOW);
      return (spinMiss(victim, TYPE_DEFAULT));
    } else {
      act("Your spin attempt on $N is more difficult because $E is flying.",
        FALSE, this, 0, victim, TO_CHAR, ANSI_YELLOW);
      act(
        "The fact that you are flying makes $n's spin attempt much more "
        "difficult.",
        FALSE, this, 0, victim, TO_VICT, ANSI_YELLOW);
      act("The fact that $N is flying makes $n's spin attempt more difficult.",
        FALSE, this, 0, victim, TO_NOTVICT, ANSI_YELLOW);
      // continue spinning
      // the above is to make spin less annoying to flyers
    }
  }
  if (getMove() < SPIN_COST) {
    sendTo("You don't have the vitality to spin anyone!\n\r");
    return FALSE;
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

  return TRUE;
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
        sendTo(
          "You are not skilled enough to spin someone with something in your "
          "hands!\n\r");
        return FALSE;
      }
    } else if (learning <= 50) {
      if (heldInPrimHand()) {
        sendTo(
          "You are not skilled enough to spin someone with something in your "
          "primary hand!\n\r");
        return FALSE;
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
    victim = NULL;
    REM_DELETE(rc, DELETE_VICT);
  }
  return rc;
}
