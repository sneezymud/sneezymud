//////////////////////////////////////////////////////////////////////////
//
// SneezyMUD - All rights reserved, SneezyMUD Coding Team
//
//////////////////////////////////////////////////////////////////////////

#include <string.h>

#include "ansi.h"
#include "being.h"
#include "combat.h"
#include "comm.h"
#include "defs.h"
#include "enum.h"
#include "handler.h"
#include "limbs.h"
#include "obj.h"
#include "spells.h"
#include "structs.h"
#include "thing.h"

bool TBeing::canStomp(TBeing* victim, silentTypeT silent) {
  if (checkBusy())
    return false;

  if (!doesKnowSkill(SKILL_STOMP)) {
    if (!silent)
      sendTo("You know nothing about stomping.\n\r");
    return false;
  }
  // I don't see why animals with legs can't stomp.
  //  if (!isHumanoid()) {
  //    if (!silent)
  //      sendTo("Only humanoids can stomp.\n\r");
  //    return false;
  //  }
  if (!hasLegs()) {
    if (!silent)
      sendTo("You need legs to stomp.\n\r");
    return false;
  }
  if (eitherLegHurt()) {
    if (!silent)
      sendTo("You can't stomp with a hurt leg.\n\r");
    return false;
  }

  if (checkPeaceful("You feel too peaceful to contemplate violence.\n\r"))
    return false;

  if (victim == this) {
    if (!silent)
      sendTo("Aren't we funny today...\n\r");
    return false;
  }
  if (riding) {
    if (!silent)
      sendTo("You can't stomp while mounted!\n\r");
    return false;
  }
  if (victim->getPartMinHeight(ITEM_WEAR_FEET) > 0) {
    act("You can't stomp $N because $S feet aren't on the $g.", false, this, 0,
      victim, TO_CHAR);
    return false;
  }
  if (victim->isFlying()) {
    if (!silent)
      sendTo("You can't stomp someone that is flying.\n\r");
    return false;
  }
  if (noHarmCheck(victim))
    return false;

  if (victim->isImmortal() || IS_SET(victim->specials.act, ACT_IMMORTAL)) {
    if (!silent)
      sendTo("You can't successfully stomp an immortal.\n\r");
    return false;
  }

  return true;
}

int TBeing::stompMiss(TBeing* victim) {
  if (victim->doesKnowSkill(SKILL_COUNTER_MOVE)) {
    act("$N deftly avoids $n's stomp.", false, this, 0, victim, TO_NOTVICT);
    act("$N deftly avoids your stomp.", false, this, 0, victim, TO_CHAR);
    act("You deftly avoid $n's stomp.", false, this, 0, victim, TO_VICT);
  } else if (victim->getPosition() == POSITION_STANDING) {
    act("$N ducks and avoids $n's stomp.", false, this, 0, victim, TO_NOTVICT);
    act("$N ducks aside, and dodges your stomp.", false, this, 0, victim,
      TO_CHAR);
    act("$n tries to stomp you, but you duck to the side just in time.", false,
      this, 0, victim, TO_VICT);
  } else if ((victim->getPosition() == POSITION_RESTING) ||
             (victim->getPosition() == POSITION_SLEEPING)) {
    act("$N rolls and avoids $n's stomp.", false, this, 0, victim, TO_NOTVICT);
    act("$N rolls to the side, and dodges your stomp.", false, this, 0, victim,
      TO_CHAR);
    act("$n tries to stomp you, but you roll to the side.", false, this, 0,
      victim, TO_VICT);
  } else {
    act("$N dodges and avoids $n's stomp.", false, this, 0, victim, TO_NOTVICT);
    act("$N dodges to the side, and dodges your stomp.", false, this, 0, victim,
      TO_CHAR);
    act("$n tries to stomp you, but you dodge to the side.", false, this, 0,
      victim, TO_VICT);
  }

  if (reconcileDamage(victim, 0, SKILL_STOMP) == -1)
    return DELETE_VICT;

  return true;
}

int TBeing::stompHit(TBeing* victim) {
  int h_dam;
  int height, targ_height;
  int rc;

  int dam = getSkillDam(victim, SKILL_STOMP, getSkillLevel(SKILL_STOMP),
    getAdvLearning(SKILL_STOMP));

  if (victim->getPosition() == POSITION_STANDING) {
    // can stomp head if my height > 5*victim
    // otherwise, stomp toes

    height = getHeight();
    targ_height = victim->getPosHeight();

    if (height >= 5 * targ_height) {
      act("$n lifts $s leg high over $N's head, stomping $M hard on the head!",
        false, this, 0, victim, TO_NOTVICT);
      act(
        "You lift your leg high over $N's head, stomping $M hard on the head!",
        false, this, 0, victim, TO_CHAR);
      act(
        "You look upward just in time to see the bottom of $n's foot "
        "descending toward you!",
        false, this, 0, victim, TO_VICT, ANSI_RED);

      TObj* item = dynamic_cast<TObj*>(victim->equipment[WEAR_HEAD]);
      if (!item) {
        h_dam = 1 + dam / 5;
        rc = damageLimb(victim, WEAR_HEAD, 0, &h_dam);
        if (IS_SET_DELETE(rc, DELETE_VICT))
          return DELETE_VICT;
      } else if (dentItem(victim, item, 1, getPrimaryFoot()) == DELETE_ITEM) {
        delete item;
        item = nullptr;
      }
    } else {
      act("$n lifts $s leg high, stomping $N's toes hard!", false, this, 0,
        victim, TO_NOTVICT);
      act("You lift your leg high and stomp $N's toes hard.", false, this, 0,
        victim, TO_CHAR);
      act("$n crushes your toes with $s stomp.", false, this, 0, victim,
        TO_VICT, ANSI_RED);

      dam /= 5;

      TObj* item = dynamic_cast<TObj*>(victim->equipment[WEAR_FOOT_L]);
      if (!item) {
        h_dam = 1 + dam / 4;
        rc = damageLimb(victim, WEAR_FOOT_L, 0, &h_dam);
        if (IS_SET_DELETE(rc, DELETE_VICT))
          return DELETE_VICT;
      } else if (dentItem(victim, item, 1, getPrimaryFoot()) == DELETE_ITEM) {
        delete item;
        item = nullptr;
      }
    }
  } else {
    act("$n lifts $s leg high, stomping $N while $E is down.", false, this, 0,
      victim, TO_NOTVICT);
    act("You lift your leg high and stomp $N hard while $E is down.", false,
      this, 0, victim, TO_CHAR);
    act("$n stomps you hard while you are down!", false, this, 0, victim,
      TO_VICT, ANSI_RED);
  }
  if (reconcileDamage(victim, dam, SKILL_STOMP) == -1)
    return DELETE_VICT;

  return true;
}

int TBeing::stomp(TBeing* victim) {
  const int STOMP_MOVE = 10;

  if (!canStomp(victim, SILENT_NO))
    return false;

  if (getMove() < STOMP_MOVE) {
    sendTo("You lack the vitality.\n\r");
    return false;
  }
  addToMove(-STOMP_MOVE);

  int bKnown = getSkillValue(SKILL_STOMP);
  int successfulHit = specialAttack(victim, SKILL_STOMP);
  int successfulSkill = bSuccess(bKnown, SKILL_STOMP);

  if (!victim->awake() || (successfulSkill && successfulHit &&
                            successfulHit != GUARANTEED_FAILURE &&
                            !victim->canCounterMove(bKnown * 2 / 5) &&
                            !victim->canFocusedAvoidance(bKnown * 2 / 5))) {
    return (stompHit(victim));
  } else {
    stompMiss(victim);
  }
  return true;
}

int TBeing::doStomp(const char* argument, TBeing* vict) {
  int rc;
  TBeing* victim;
  char name_buf[256];

  strcpy(name_buf, argument);

  if (!(victim = vict)) {
    if (!(victim = get_char_room_vis(this, name_buf))) {
      if (!(victim = fight())) {
        sendTo("Stomp whom?\n\r");
        return false;
      }
    }
  }
  if (!sameRoom(*victim)) {
    sendTo("That person isn't around.\n\r");
    return false;
  }
  rc = stomp(victim);
  if (rc)
    addSkillLag(SKILL_STOMP, rc);

  if (IS_SET_DELETE(rc, DELETE_VICT)) {
    if (vict)
      return rc;
    delete victim;
    victim = nullptr;
    REM_DELETE(rc, DELETE_VICT);
  }
  if (IS_SET_DELETE(rc, DELETE_THIS))
    return DELETE_THIS;

  return rc;
}
