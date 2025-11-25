//////////////////////////////////////////////////////////////////////////
//
// SneezyMUD - All rights reserved, SneezyMUD Coding Team
//
// chop.cc
//
//////////////////////////////////////////////////////////////////////////

#include "handler.h"
#include "being.h"
#include "combat.h"
#include "disc_monk.h"
#include "obj_general_weapon.h"


static int chopMiss(TBeing* c, TBeing* v) {
  act("$n swings wildly as $e tries to chop $N.", FALSE, c, 0, v, TO_NOTVICT);
  act("You miss your chop attack at $N.", FALSE, c, 0, v, TO_CHAR);
  act("Stepping backward, you manage to dodge $n's chop.", FALSE, c, 0, v,
    TO_VICT);

  c->reconcileDamage(v, 0, SKILL_CHOP);

  return TRUE;
}

// returns DELETE_VICT
static int chopHit(TBeing* c, TBeing* v, int score) {
  int slot;
  int temp;
  wearSlotT pos = WEAR_NOWHERE;
  wearSlotT handSlot = WEAR_HAND_R;

  temp = ::number(1, score);
  if (temp < 30) {
    if ((c->isRightHanded()) && (v->hasPart(WEAR_ARM_L))) {
      pos = WEAR_ARM_L;
      slot = 1;
    } else if (!c->isRightHanded() && v->hasPart(WEAR_ARM_R)) {
      pos = WEAR_ARM_R;
      slot = 1;
    } else {
      pos = WEAR_BODY; 
      slot = 2;
    }
  } else if (temp < 80) {
    pos = WEAR_BODY;
    slot = 2;  // body shot
  } else if (temp < 90) {
    pos = WEAR_NECK;
    slot = 3;  // neck shot
  } else {
    pos = WEAR_HEAD;
    slot = 4;  // head shot
  }
  if (!v->isHumanoid()) {
   pos = WEAR_WAIST; 
   slot = 5;  // non-human side shot
  }
  // Set default damage
  int dam = c->getSkillDam(v, SKILL_CHOP, c->getSkillLevel(SKILL_CHOP),
    c->getAdvLearning(SKILL_CHOP));

  // Default lag
  //  c->cantHit += c->loseRound(1);

  switch (slot) {
    case 1:  // ARM SHOT
      act("$n slams $s chop into $N's arm.", FALSE, c, 0, v, TO_NOTVICT);
      act("You're hit in the arm by $n's mighty chop!", FALSE, c, 0, v,
        TO_VICT);
      act("Your mighty chop hits $N's arm.", FALSE, c, 0, v, TO_CHAR);
      dam /= 3;
      break;
    case 2:  // BODY SHOT
      act("$n hits $N's chest with a mighty chop!", FALSE, c, 0, v, TO_NOTVICT);
      act("Your chest throbs with pain as $n's chop hits you!", FALSE, c, 0, v,
        TO_VICT);
      act("Your chop lands square on $N's chest!", FALSE, c, 0, v, TO_CHAR);
      if (!v->equipment[WEAR_BODY]) {
        v->sendTo("You should think about wearing armor on your body!\n\r");
        dam += 3;
      }
      break;
    case 3:  // NECK SHOT
      act("$n lands a mighty chop on $N's neck!", FALSE, c, 0, v, TO_NOTVICT);
      act("Your neck thobs with pain as $n chops it! OUCH!", FALSE, c, 0, v,
        TO_VICT);
      act("Your chop lands square on $N's neck!", FALSE, c, 0, v, TO_CHAR);
      dam += 2;
      if (!v->equipment[WEAR_NECK]) {
        v->sendTo("You should think about wearing armor on your neck!\n\r");
        v->cantHit += v->loseRound(1);
        dam += 4;
      }
      break;
    case 4:  // HEAD SHOT
      act("$n hits $N's head with a mighty chop! Oh, the humanity!", FALSE, c,
        0, v, TO_NOTVICT);
      act("Your head throbs with pain as $n chops it! OUCH!", FALSE, c, 0, v,
        TO_VICT);
      act("Your chop lands square on $N's head! YIPE!", FALSE, c, 0, v,
        TO_CHAR);
      v->cantHit += v->loseRound(1);
      dam += 4;
      if (!v->equipment[WEAR_HEAD]) {
        v->sendTo("You should think about wearing armor on your head!\n\r");
        dam += 2;
      }
      break;
    case 5:  // SIDE SHOT
    default:
      act("$n hits $N in the side with a mighty chop!", 0, c, 0, v, TO_NOTVICT);
      act("You're hit in the side by $n's mighty chop!", FALSE, c, 0, v,
        TO_VICT);
      act("Your chop lands square on $N's side!", FALSE, c, 0, v, TO_CHAR);
      break;
  }
  if (!c->isRightHanded()){
    handSlot = WEAR_HAND_L;
  }

  // Use impactSpec to handle all impact effects (spikes, thornflesh, hardness)
  dam += impactSpec(c, v, handSlot, pos);

  if (c->reconcileDamage(v, dam, SKILL_CHOP) == -1)
    return DELETE_VICT;

  return TRUE;
}

static int chop(TBeing* c, TBeing* v) {
  int rc;
  int percent;
  int i;  // level;
  const int CHOP_MOVE = 5;

  if (c->checkPeaceful("You feel too peaceful to contemplate violence.\n\r"))
    return FALSE;

  if (c->noHarmCheck(v))
    return FALSE;

  if (IS_SET(v->specials.act, ACT_IMMORTAL)) {
    c->sendTo("You decide that your chop would not affect an immortal.\n\r");
    return FALSE;
  }
  if (c->eitherArmHurt()) {
    c->sendTo(
      "It's very hard to use the chop attack without use of your arms!\n\r");
    return FALSE;
  }
  if (v == c) {
    c->sendTo("Hurting yourself isn't a good idea.\n\r");
    return FALSE;
  }
  if (c->getMove() < CHOP_MOVE) {
    c->sendTo("You are too tired to use the chop attack.\n\r");
    return FALSE;
  }
  if (c->riding) {
    c->sendTo("You can't do that while mounted!\n\r");
    return FALSE;
  }
  if (v->riding && dynamic_cast<TBeing*>(v->riding)) {
    c->sendTo("You can't use that attack on a mounted person!\n\r");
    return FALSE;
  } else if (v->riding) {
    c->sendTo(COLOR_MOBS,
      format("You can't use that attack while %s is on %s!\n\r") %
        v->getName() % v->riding->getName());
    return FALSE;
  }

  if (v->isFlying() && !c->isFlying()) {
    c->sendTo(
      "You can't chop at them while they are flying unless you are flying as "
      "well!\n\r");
    return FALSE;
  }
  if (c->equipment[HOLD_RIGHT] || c->equipment[HOLD_LEFT]) {
    c->sendTo("You can't chop while holding things!\n\r");
    return FALSE;
  }
  percent = 0;
  //  level = c->getSkillLevel(SKILL_CHOP);
  int bKnown = c->getSkillValue(SKILL_CHOP);

  if (!c->isImmortal())
    c->addToMove(-CHOP_MOVE);

  i = c->specialAttack(v, SKILL_CHOP);
  if (!v->awake() ||
      (i && i != GUARANTEED_FAILURE && c->bSuccess(bKnown, SKILL_CHOP))) {
    rc = chopHit(c, v, bKnown + percent);
    if (IS_SET_DELETE(rc, DELETE_VICT))
      return DELETE_VICT;
  } else {
    chopMiss(c, v);
  }
  return TRUE;
}

int TBeing::doChop(const char* arg, TBeing* vict) {
  int rc;
  TBeing* victim;
  char v_name[MAX_INPUT_LENGTH];

  if (checkBusy()) {
    return FALSE;
  }
  if (!doesKnowSkill(SKILL_CHOP)) {
    sendTo("You do not know the secrets of the chop attack.\n\r");
    return FALSE;
  }

  strcpy(v_name, arg);

  if (!(victim = vict)) {
    if (!(victim = get_char_room_vis(this, v_name))) {
      if (!(victim = fight())) {
        sendTo("You want to execute a chop on whom?\n\r");
        return FALSE;
      }
    }
  }
  if (!sameRoom(*victim)) {
    sendTo("That person isn't around.\n\r");
    return FALSE;
  }
  rc = chop(this, victim);
  if (rc)
    addSkillLag(SKILL_CHOP, rc);
  if (IS_SET_DELETE(rc, DELETE_VICT)) {
    if (vict)
      return rc;
    delete victim;
    victim = NULL;
    REM_DELETE(rc, DELETE_VICT);
  }
  return rc;
}
