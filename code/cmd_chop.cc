//////////////////////////////////////////////////////////////////////////
//
// SneezyMUD - All rights reserved, SneezyMUD Coding Team
//
// chop.cc
//
//////////////////////////////////////////////////////////////////////////


#include "stdsneezy.h"
#include "combat.h"
#include "disc_monk.h"

static int chopMiss(TBeing *c, TBeing *v)
{
  act("$n swings wildly as $e tries to chop $N.", FALSE, c, 0, v, TO_NOTVICT);
  act("You miss your chop attack at $N.", FALSE, c, 0, v, TO_CHAR);
  act("Stepping backward, you manage to dodge $n's chop.", FALSE, c, 0 , v, TO_VICT);

  c->reconcileDamage(v, 0, SKILL_CHOP);

  return TRUE;
}

// returns DELETE_VICT
static int chopHit(TBeing *c, TBeing *v, int score)
{
  int rc;
  int slot;
  int temp, limb_dam;
  wearSlotT pos = WEAR_NOWHERE;
  TObj *item;

  temp = ::number(1, score);
  if (temp < 30 ) {
    if ((c->isRightHanded()) && (v->hasPart(WEAR_ARM_L))) {
      pos = WEAR_ARM_L;
      slot = 1;
    } 
    else if(!c->isRightHanded() && v->hasPart(WEAR_ARM_R)) {
      pos = WEAR_ARM_R;
      slot = 1;
    }
    else
      slot = 2;
  }
  else if (temp < 80)
    slot = 2;                     // body shot
  else if (temp < 90)
    slot = 3;                     // neck shot
  else 
    slot = 4;                     // head shot
  if (!v->isHumanoid())
    slot = 5;                     // non-human side shot
  
  // Set default damage
  int dam = c->getSkillDam(v, SKILL_CHOP, c->getSkillLevel(SKILL_CHOP), c->getAdvLearning(SKILL_CHOP));

  // Default lag
  //  c->cantHit += c->loseRound(1);
  
  switch (slot) {
    case 1:         // ARM SHOT
      act("$n slams $s chop into $N's arm.", FALSE, c, 0, v, TO_NOTVICT);
      act("You're hit in the arm by $n's mighty chop!",FALSE, c, 0, v, TO_VICT);
      act("Your mighty chop hits $N's arm.", FALSE, c, 0, v, TO_CHAR);
      dam /= 3;
      limb_dam = (::number(1, c->getSkillLevel(SKILL_CHOP)))/3;
      item = dynamic_cast<TObj *>(v->equipment[pos]);
      if (!item) {
        v->sendTo("You should think about wearing armor on your arms!\n\r");
	//        v->cantHit += v->loseRound(1);
        rc = c->damageLimb(v, pos, NULL, &limb_dam);
        if (IS_SET_DELETE(rc, DELETE_VICT))
          return DELETE_VICT;
      } else if (c->dentItem(v, item, 1, c->getPrimaryHand()) == DELETE_ITEM) {
        delete item;
        item = NULL;
      }
      break;
    case 2:   // BODY SHOT
      act("$n hits $N's chest with a mighty chop!", FALSE, c, 0, v, TO_NOTVICT);
      act("Your chest throbs with pain as $n's chop hits you!", FALSE, c, 0, v, TO_VICT);
      act("Your chop lands square on $N's chest!", FALSE, c, 0, v, TO_CHAR);
      item = dynamic_cast<TObj *>(v->equipment[WEAR_BODY]);
      if (!item) {
        v->sendTo("You should think about wearing armor on your body!\n\r");
	//        v->cantHit += v->loseRound(1);
        dam += 3;
      } else if (c->dentItem(v, item, 1, c->getPrimaryHand()) == DELETE_ITEM) {
        delete item;
        item = NULL;
      }
      break;
    case 3:       // NECK SHOT
      act("$n lands a mighty chop on $N's neck!", FALSE, c, 0, v, TO_NOTVICT);
      act("Your neck thobs with pain as $n chops it! OUCH!", FALSE, c, 0, v, TO_VICT);
      act("Your chop lands square on $N's neck!", FALSE, c, 0, v, TO_CHAR);
      //     v->cantHit += v->loseRound(1);
     dam += 2;
      item = dynamic_cast<TObj *>(v->equipment[WEAR_NECK]);
      if (!item) {
        v->sendTo("You should think about wearing armor on your neck!\n\r");
	v->cantHit += v->loseRound(1);
        dam += 4;
      } else if (c->dentItem(v, item, 1, c->getPrimaryHand()) == DELETE_ITEM) {
        delete item;
        item = NULL;
      }
      break;
    case 4:     // HEAD SHOT
      act("$n hits $N's head with a mighty chop! Oh, the humanity!", FALSE, c, 0, v, TO_NOTVICT);
      act("Your head throbs with pain as $n chops it! OUCH!", FALSE, c, 0, v, TO_VICT);
      act("Your chop lands square on $N's head! YIPE!", FALSE, c, 0,v, TO_CHAR);
      v->cantHit += v->loseRound(1);
      dam += 4;
      item = dynamic_cast<TObj *>(v->equipment[WEAR_HEAD]);
      if (!item) {
        v->sendTo("You should think about wearing armor on your head!\n\r");
	//        v->cantHit += v->loseRound(1);
        dam += 2;
      } else if (c->dentItem(v, item, 1, c->getPrimaryHand()) == DELETE_ITEM) {
        delete item;
        item = NULL;
      }     
      break;
    case 5:    // SIDE SHOT
    default:
      act("$n hits $N in the side with a mighty chop!", 0, c, 0, v, TO_NOTVICT);
      act("You're hit in the side by $n's mighty chop!", FALSE, c, 0,v,TO_VICT);
      act("Your chop lands square on $N's side!", FALSE, c, 0, v, TO_CHAR);
      break; 
  }

  item = dynamic_cast<TObj *>(c->equipment[c->getPrimaryHand()]);
  if (item)
    if (item->isSpiked() || item->isObjStat(ITEM_SPIKED)) {

      act("The spikes on your $o sink into $N.", FALSE, c, item, v, TO_CHAR);
      act("The spikes on $n's $o sink into $N.", FALSE, c, item, v, TO_NOTVICT);
      act("The spikes on $n's $o sink into you.", FALSE, c, item, v, TO_VICT);

      if (c->reconcileDamage(v, (int)(dam*0.15), TYPE_STAB) == -1)
	return DELETE_VICT;
    }

  if (c->reconcileDamage(v, dam, SKILL_CHOP) == -1)
    return DELETE_VICT;

  return TRUE;
}

static int chop(TBeing *c, TBeing *v)
{
  int rc;
  int percent;
  int i;//level;
  const int CHOP_MOVE = 9;

  if (c->checkPeaceful("You feel too peaceful to contemplate violence.\n\r"))
    return FALSE;

  if (c->noHarmCheck(v))
    return FALSE;

  if (IS_SET(v->specials.act, ACT_IMMORTAL)) {
    c->sendTo("You decide that your chop would not affect an immortal.\n\r");
    return FALSE;
  }
  if (c->eitherArmHurt()) {
    c->sendTo("It's very hard to use the chop attack without use of your arms!\n\r");
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
  if (v->riding && dynamic_cast<TBeing *>(v->riding)) {
    c->sendTo("You can't use that attack on a mounted person!\n\r");
    return FALSE;
  } else if (v->riding) {
    c->sendTo(COLOR_MOBS, fmt("You can't use that attack while %s is on %s!\n\r") %	      v->getName() % v->riding->getName());
    return FALSE;
  }

  if (v->isFlying() && !c->isFlying()) {
    c->sendTo("You can't chop at them while they are flying unless you are flying as well!\n\r");
    return FALSE;
  }
  if (c->isSwimming()) {
    c->sendTo("You can't focus while swimming!\n\r");
    return FALSE;
  }
  if (c->equipment[HOLD_RIGHT] ||
      c->equipment[HOLD_LEFT]) {
    c->sendTo("You can't chop while holding things!\n\r");
    return FALSE;
  }
  percent = 0;
  //  level = c->getSkillLevel(SKILL_CHOP);
  int bKnown = c->getSkillValue(SKILL_CHOP);

  if (!c->isImmortal())
    c->addToMove(-CHOP_MOVE);

  i = c->specialAttack(v,SKILL_CHOP);
  if (!v->awake() ||
         (i && i != GUARANTEED_FAILURE &&
         c->bSuccess(bKnown, SKILL_CHOP))) {
    rc = chopHit(c, v, bKnown + percent);
    if (IS_SET_DELETE(rc, DELETE_VICT))
      return DELETE_VICT;
  } else {
    chopMiss(c, v);
  }
  return TRUE;
}

int TBeing::doChop(const char *arg, TBeing *vict)
{
  int rc;
  TBeing *victim;
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

