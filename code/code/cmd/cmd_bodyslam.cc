//////////////////////////////////////////////////////////////////////////
//
// SneezyMUD - All rights reserved, SneezyMUD Coding Team
//
//////////////////////////////////////////////////////////////////////////

#include "handler.h"
#include "extern.h"
#include "room.h"
#include "being.h"
#include "combat.h"
#include "enum.h"
#include "spells.h"
#include "being.h"

bool TBeing::canBodyslam(TBeing* victim, silentTypeT silent) {
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
      sendTo(
        "You are berserking! You can't focus enough to bodyslam anyone!\n\r ");
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
      act("$N is already on the $g.  You can't bodyslam $M.", FALSE, this, 0,
        victim, TO_CHAR);
    return FALSE;
  }

  return TRUE;
}

int TBeing::bodyslamMiss(TBeing* victim, skillMissT type) {
  if (type == TYPE_DEX) {
    act("$N deftly avoids your bodyslam attempt.", FALSE, this, 0, victim,
      TO_CHAR);
    act("You deftly avoid $n's bodyslam attempt.", FALSE, this, 0, victim,
      TO_VICT);
    act("$N deftly avoids $n's bodyslam attempt.", FALSE, this, 0, victim,
      TO_NOTVICT);
  } else if (type == TYPE_MONK) {
    act("$N deftly counters your bodyslam, and throws you to the side.", false,
      this, 0, victim, TO_CHAR, ANSI_RED);
    act("You deftly counter $n's bodyslam, and throw $m to the side.", false,
      this, 0, victim, TO_VICT);
    act("$N deftly counters $n's bodyslam, and throws $m to the side.", false,
      this, 0, victim, TO_NOTVICT);

    int rc = stumble();
    if (IS_SET_DELETE(rc, DELETE_THIS))
      return rc;
  } else if (type == TYPE_STR) {
    act("$n tries to bodyslam $N but fails to lift $M.", false, this, 0, victim,
      TO_NOTVICT);
    act("You try to bodyslam $N but fail to lift $M.", false, this, 0, victim,
      TO_CHAR);
    act("$n tries to bodyslam you but fails to lift you.", false, this, 0,
      victim, TO_VICT);

    int rc = stumble();
    if (IS_SET_DELETE(rc, DELETE_THIS))
      return rc;
  } else {
    act("$n tries to bodyslam $N but loses $s footing.", false, this, 0, victim,
      TO_NOTVICT);
    act("You try to bodyslam $N but lose your footing.", false, this, 0, victim,
      TO_CHAR);
    act("$n tries to bodyslam you but loses $s footing.", false, this, 0,
      victim, TO_VICT);

    int rc = stumble();
    if (IS_SET_DELETE(rc, DELETE_THIS))
      return rc;
  }

  if (reconcileDamage(victim, 0, SKILL_BODYSLAM) == -1)
    return DELETE_VICT;

  return FALSE;
}

int TBeing::bodyslamHit(TBeing* victim) {
  const bool wasMounted = (victim->riding != nullptr);

  // Setup: the lift attempt. Then a payoff line that varies by mount status.
  // crashLanding/knockOffMount narrates the impact result afterward.
  act("You grab $N around the middle, attempting to lift $M overhead!", false,
    this, nullptr, victim, TO_CHAR);
  act("$n grabs you around the middle, attempting to lift you overhead!", false,
    this, nullptr, victim, TO_VICT, ANSI_RED);
  act("$n grabs $N around the middle, attempting to lift $M overhead!", false,
    this, nullptr, victim, TO_NOTVICT);

  int rc;
  if (wasMounted) {
    TThing* mount =
      victim->riding;  // capture before knockOffMount may dismount
    rc = victim->knockOffMount(getSkillValue(SKILL_BODYSLAM) / 2);
    if (IS_SET_DELETE(rc, DELETE_THIS))
      return DELETE_VICT;
    // Only narrate the throw when the rider was actually dismounted; if they
    // hung on, knockOffMount printed its own "hangs on tight" flavor.
    if (!victim->riding) {
      act("You pull $N from $p, throwing $M down!", false, this, mount, victim,
        TO_CHAR);
      act("$n pulls you from $p, throwing you down!", false, this, mount,
        victim, TO_VICT, ANSI_RED);
      act("$n pulls $N from $p, throwing $M down!", false, this, mount, victim,
        TO_NOTVICT);
    }
  } else {
    act("You throw $N down hard!", false, this, nullptr, victim, TO_CHAR);
    act("$n throws you down hard!", false, this, nullptr, victim, TO_VICT,
      ANSI_RED);
    act("$n throws $N down hard!", false, this, nullptr, victim, TO_NOTVICT);
    rc = victim->crashLanding();
    if (IS_SET_DELETE(rc, DELETE_THIS))
      return DELETE_VICT;
  }

  // see the balance notes for details on what's going on here.
  float wt = combatRound(discArray[SKILL_BODYSLAM]->lag);

  // since we cost some moves to perform, allow an extra 1/2 round of lag
  wt += 1.5;

  // since success and failure both have reciprocal positional changes
  // there is no reason to account for that here.

  // round up
  wt += 0.5;

  victim->addToWait((int)wt);

  // in general, we should not do BOTH damage and command lock-out
  // however, since Bslam has nasty requirements on strength and
  // dex to lift person up, doing this damage will counter-balance
  // those penalties.  Warrior-skill damage isn't all that high
  // to begin with...
  int dam = getSkillDam(victim, SKILL_BODYSLAM, getSkillLevel(SKILL_BODYSLAM),
    getAdvLearning(SKILL_BODYSLAM));

  if (reconcileDamage(victim, dam, SKILL_BODYSLAM) == -1)
    return DELETE_VICT;

  return TRUE;
}

int TBeing::bodyslam(TBeing* victim) {
  int rc;
  const int BODYSLAM_COST = 10;  // movement cost to slam

  if (!canBodyslam(victim, SILENT_NO))
    return FALSE;

  if (getMove() < BODYSLAM_COST) {
    sendTo("You don't have the vitality to bodyslam anyone!\n\r");
    return FALSE;
  }
  addToMove(-BODYSLAM_COST);

  int bKnown = getSkillValue(SKILL_BODYSLAM);
  int successfulHit = specialAttack(victim, SKILL_BODYSLAM);
  int successfulSkill = bSuccess(bKnown, SKILL_BODYSLAM);

  // Success case
  if (!victim->awake() || (successfulSkill && successfulHit &&
                            successfulHit != GUARANTEED_FAILURE)) {
    // Allow victim a chance to counter
    if (victim->canCounterMove(bKnown / 3)) {
      SV(SKILL_BODYSLAM);
      rc = bodyslamMiss(victim, TYPE_MONK);
      if (IS_SET_DELETE(rc, DELETE_THIS) || IS_SET_DELETE(rc, DELETE_VICT))
        return rc;
      // Ensure
    } else if (compareWeights(victim->getTotalWeight(TRUE),
                 carryWeightLimit() * 3) == -1) {
      CF(SKILL_BODYSLAM);
      rc = bodyslamMiss(victim, TYPE_STR);
      if (IS_SET_DELETE(rc, DELETE_THIS) || IS_SET_DELETE(rc, DELETE_VICT))
        return rc;
    } else
      return bodyslamHit(victim);
  } else {
    rc = bodyslamMiss(victim, TYPE_DEFAULT);
    if (IS_SET_DELETE(rc, DELETE_THIS) || IS_SET_DELETE(rc, DELETE_VICT))
      return rc;
  }

  return TRUE;
}

int TBeing::doBodyslam(const char* argument, TBeing* vict) {
  int rc = 0, learning = 0;
  TBeing* victim;
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
        sendTo(
          "You are not skilled enough to bodyslam with something in your "
          "hands!\n\r");
        return FALSE;
      }
    } else if (learning <= 75) {
      if (heldInPrimHand()) {
        sendTo(
          "You are not skilled enough to bodyslam with something in your "
          "primary hand!\n\r");
        return FALSE;
      }
    } else {
      // no restrictions
    }
  }
  rc = bodyslam(victim);
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
