//////////////////////////////////////////////////////////////////////////
//
// SneezyMUD - All rights reserved, SneezyMUD Coding Team
//
//////////////////////////////////////////////////////////////////////////


#include "stdsneezy.h"
#include "combat.h"

bool TBeing::canStomp(TBeing *victim, silentTypeT silent)
{
  if (checkBusy())
    return FALSE;

  if (!doesKnowSkill(SKILL_STOMP)) {
    if (!silent)
      sendTo("You know nothing about stomping.\n\r");
    return FALSE;
  }
// I don't see why animals with legs can't stomp.
//  if (!isHumanoid()) {
//    if (!silent)
//      sendTo("Only humanoids can stomp.\n\r");
//    return FALSE;
//  }
  if (!hasLegs()) {
    if (!silent)
      sendTo("You need legs to stomp.\n\r");
    return FALSE;
  }
  if (eitherLegHurt()) {
    if (!silent)
      sendTo("You can't stomp with a hurt leg.\n\r");
    return FALSE;
  }

  if (checkPeaceful("You feel too peaceful to contemplate violence.\n\r"))
    return FALSE;
  
  if (victim == this) {
    if (!silent)
      sendTo("Aren't we funny today...\n\r");
    return FALSE;
  }
  if (riding) {
    if (!silent)
      sendTo("You can't stomp while mounted!\n\r");
    return FALSE;
  }
  if (victim->getPartMinHeight(ITEM_WEAR_FEET) > 0) {
    act("You can't stomp $N because $S feet aren't on the $g.",
         FALSE, this, 0, victim, TO_CHAR);
    return FALSE;
  }
  if (victim->isFlying()) {
    if (!silent)
      sendTo("You can't stomp someone that is flying.\n\r");
    return FALSE;
  }
  if (noHarmCheck(victim))
    return FALSE;

  if (victim->isImmortal() || IS_SET(victim->specials.act, ACT_IMMORTAL)) {
    if (!silent)
      sendTo("You can't successfully stomp an immortal.\n\r");
    return FALSE;
  }

  return TRUE;
}

static int stompMiss(TBeing *caster, TBeing *victim)
{
  if (victim->doesKnowSkill(SKILL_COUNTER_MOVE)) {
    act("$N deftly avoids $n's stomp.", 
        FALSE, caster, 0, victim, TO_NOTVICT);
    act("$N deftly avoids your stomp.", 
        FALSE, caster, 0, victim, TO_CHAR);
    act("You deftly avoid $n's stomp.", 
        FALSE, caster, 0, victim, TO_VICT);
  } else if (victim->getPosition() == POSITION_STANDING) {
    act("$N ducks and avoids $n's stomp.", 
        FALSE, caster, 0, victim, TO_NOTVICT);
    act("$N ducks aside, and dodges your stomp.",
        FALSE, caster, 0, victim, TO_CHAR);
    act("$n tries to stomp you, but you duck to the side just in time.", 
        FALSE, caster, 0, victim, TO_VICT);
  } else if ((victim->getPosition() == POSITION_RESTING) ||
             (victim->getPosition() == POSITION_SLEEPING)) {
    act("$N rolls and avoids $n's stomp.", 
        FALSE, caster, 0, victim, TO_NOTVICT);
    act("$N rolls to the side, and dodges your stomp.",
        FALSE, caster, 0, victim, TO_CHAR);
    act("$n tries to stomp you, but you roll to the side.", 
        FALSE, caster, 0, victim, TO_VICT);
  } else {
    act("$N dodges and avoids $n's stomp.", 
        FALSE, caster, 0, victim, TO_NOTVICT);
    act("$N dodges to the side, and dodges your stomp.",
        FALSE, caster, 0, victim, TO_CHAR);
    act("$n tries to stomp you, but you dodge to the side.", 
        FALSE, caster, 0, victim, TO_VICT);
  }

  if (caster->reconcileDamage(victim, 0,SKILL_STOMP) == -1)
    return DELETE_VICT;

  return TRUE;
}

static int stompHit(TBeing *caster, TBeing *victim)
{
  int h_dam;
  int height, targ_height;
  int rc;

  int dam = caster->getSkillDam(victim, SKILL_STOMP, caster->getSkillLevel(SKILL_STOMP), caster->getAdvLearning(SKILL_STOMP));

  if (victim->getPosition() == POSITION_STANDING) {
    // can stomp head if my height > 5*victim
    // otherwise, stomp toes

    height = caster->getHeight();
    targ_height = victim->getPosHeight();
  
    if (height >= 5 * targ_height) {
      act("$n lifts $s leg high over $N's head, stomping $M hard on the head!",
           FALSE, caster, 0, victim, TO_NOTVICT);
      act("You lift your leg high over $N's head, stomping $M hard on the head!",
           FALSE, caster, 0, victim, TO_CHAR);
      act("You look upward just in time to see the bottom of $n's foot descending toward you!",
           FALSE, caster, 0, victim, TO_VICT, ANSI_RED);

      TObj *item = dynamic_cast<TObj *>(victim->equipment[WEAR_HEAD]);
      if (!item) {
        h_dam = 1 + dam/5;
        rc = caster->damageLimb(victim, WEAR_HEAD, 0, &h_dam);
        if (IS_SET_DELETE(rc, DELETE_VICT))
          return DELETE_VICT;
      } else if (caster->dentItem(victim, item, 1, caster->getPrimaryFoot()) == DELETE_ITEM) {
        delete item;
        item = NULL;
      }
    } else {
      act("$n lifts $s leg high, stomping $N's toes hard!",
           FALSE, caster, 0, victim, TO_NOTVICT);
      act("You lift your leg high and stomp $N's toes hard.",
           FALSE, caster, 0, victim, TO_CHAR);
      act("$n crushes your toes with $s stomp.",
           FALSE, caster, 0, victim, TO_VICT, ANSI_RED);

      dam /= 5;

      TObj *item = dynamic_cast<TObj *>(victim->equipment[WEAR_FOOT_L]);
      if (!item) {
        h_dam = 1 + dam/4;
        rc = caster->damageLimb(victim, WEAR_FOOT_L, 0, &h_dam);
        if (IS_SET_DELETE(rc, DELETE_VICT))
          return DELETE_VICT;
      } else if (caster->dentItem(victim, item, 1, caster->getPrimaryFoot()) == DELETE_ITEM) {
        delete item;
        item = NULL;
      }
    }
  } else {
    act("$n lifts $s leg high, stomping $N while $E is down.",
         FALSE, caster, 0, victim, TO_NOTVICT);
    act("You lift your leg high and stomp $N hard while $E is down.",
         FALSE, caster, 0, victim, TO_CHAR);
    act("$n stomps you hard while you are down!",
         FALSE, caster, 0, victim, TO_VICT, ANSI_RED);
  }
  if (caster->reconcileDamage(victim, dam,SKILL_STOMP) == -1)
    return DELETE_VICT;

  return TRUE;
}

static int stomp(TBeing *c, TBeing *victim)
{
  int percent, i;
  const int STOMP_MOVE  = 12;
  
  if (!c->canStomp(victim, SILENT_NO))
    return FALSE;

  if (c->getMove() < STOMP_MOVE) {
    c->sendTo("You lack the vitality.\n\r");
    return FALSE;
  }
  c->addToMove(-STOMP_MOVE);

  percent = ((10 - (victim->getArmor() / 100)) << 1);
  percent += c->getDexReaction() * 5;
  percent -= victim->getAgiReaction() * 5;
  
  int bKnown = c->getSkillValue(SKILL_STOMP);

  if (victim->getPosition() <= POSITION_STUNNED) {
    return (stompHit(c, victim));
  } else if (c->bSuccess(bKnown + percent, SKILL_STOMP) &&
         (i = c->specialAttack(victim,SKILL_STOMP)) &&
         i != GUARANTEED_FAILURE &&
        percent < bKnown&&
         !victim->canCounterMove(bKnown*2/5)) {
    return (stompHit(c, victim));
  } else {
    stompMiss(c, victim);
  }
  return TRUE;
}

int TBeing::doStomp(const char *argument, TBeing *vict)
{
  int rc;
  TBeing *victim;
  char name_buf[256];
  
  strcpy(name_buf, argument);
  
  if (!(victim = vict)) {
    if (!(victim = get_char_room_vis(this, name_buf))) {
      if (!(victim = fight())) {
        sendTo("Stomp whom?\n\r");
        return FALSE;
      }
    }
  }
  if (!sameRoom(*victim)) {
    sendTo("That person isn't around.\n\r");
    return FALSE;
  }
  rc = stomp(this, victim);
  if (rc)
    addSkillLag(SKILL_STOMP, rc);

  if (IS_SET_DELETE(rc, DELETE_VICT)) {
    if (vict)
      return rc;
    delete victim;
    victim = NULL;
    REM_DELETE(rc, DELETE_VICT);
  }
  if (IS_SET_DELETE(rc, DELETE_THIS))
    return DELETE_THIS;

  return rc;
}

