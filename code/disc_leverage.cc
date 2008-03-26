#include "stdsneezy.h"
#include "combat.h"
#include "disc_leverage.h"
#include "range.h"
#include "obj_window.h"
#include "pathfinder.h"

int TBeing::doHurl(const char *argument, TBeing *vict)
{
  int rc;
  TBeing *victim;
  char name_buf[256], obje[100];
 
  if (checkBusy()) { 
    return FALSE;
  }
  if (!doesKnowSkill(SKILL_HURL)) {
    sendTo("You know nothing about hurling.\n\r");
    return FALSE;
  }
  half_chop(argument, name_buf, obje);
  if (!(victim = vict)) {
    if (!(victim = get_char_room_vis(this, name_buf))) {
      if (!(victim = fight())) {
        sendTo("Hurl whom?\n\r");
        return FALSE;
      }
    }
  }

  if (!sameRoom(*victim)) {
    sendTo("That person isn't around.\n\r");
    return FALSE;
  }
  rc = hurl(this,victim,obje);

  if (IS_SET_DELETE(rc, DELETE_VICT)) {
    if (vict)
      return rc;
    delete victim;
    victim = NULL;
    REM_DELETE(rc, DELETE_VICT);
  }
  return rc;
}

int hurlMiss(TBeing *caster, TBeing *victim){
  int rc;

  act("$n misses $s attempt at hurling $N and falls on $s butt!",
            FALSE, caster, 0, victim, TO_NOTVICT);
  act("You fall as you attempt to hurl $N!",
            FALSE, caster, 0, victim, TO_CHAR);
  act("You manage to avoid $n as $e tries to hurl you!",
            FALSE, caster, 0, victim, TO_VICT);

  rc = caster->crashLanding(POSITION_SITTING);
  if (IS_SET_DELETE(rc, DELETE_THIS))
    return DELETE_THIS;

  caster->reconcileDamage(victim, 0, SKILL_SHOULDER_THROW);
  return TRUE;
}

static int hurlHit(TBeing *caster, TBeing *victim, dirTypeT dr)
{
  int dam;
  int rc, percent = 0;
  char buf[256];

  // default damage
  dam = caster->getSkillDam(victim, SKILL_HURL, caster->getSkillLevel(SKILL_HURL), caster->getAdvLearning(SKILL_HURL));

  if (caster->roomp && !caster->roomp->dir_option[dr] &&
    caster->roomp->dir_option[dr]->to_room &&
    !IS_SET(caster->roomp->dir_option[dr]->condition, EX_CLOSED) &&
    (caster->roomp->dir_option[dr]->to_room |= ROOM_NOWHERE)) {
    caster->sendTo("That direction seems to be blocked.\n\r");
    return FALSE;
  }

  if (victim->doesKnowSkill(SKILL_COUNTER_MOVE) && 
      min(2*victim->GetMaxLevel(),100) > percent) {
    act("$N deftly resists your shove attempt.", 
          FALSE, caster, 0, victim, TO_CHAR);
    act("$N deftly resists $n's shove attempt.", 
          FALSE, caster, 0, victim, TO_NOTVICT);
    act("You deftly resist $n's attempt to shove you.", 
          FALSE, caster, 0, victim, TO_VICT);
  } else {
    //  caster->cantHit += caster->loseRound(2);
    victim->cantHit += victim->loseRound(2);
    if (((victim->hasPart(WEAR_ARM_R)) || (victim->hasPart(WEAR_ARM_L)))) {
      act("$n grabs $N by the arm and throws $M over $s shoulder!", FALSE, caster, 0, victim, TO_NOTVICT);
      act("$n grabs you by the arm and throws you over $s shoulder! ouch!", FALSE, caster, 0, victim, TO_VICT);
      act("You grab $N by the arm and throw $M over your shoulder!", FALSE, caster, 0, victim, TO_CHAR);
    } else {
      act("$n grabs $N by the body and throws $M over $s shoulder!", FALSE, caster, 0, victim, TO_NOTVICT);
      act("$n grabs you by the body and throws you over $s shoulder! ouch!", FALSE, caster, 0, victim, TO_VICT);
      act("You grab $N by the body and throw $M over your shoulder!", FALSE, caster, 0, victim, TO_CHAR);
      caster->cantHit += caster->loseRound(2);
      victim->cantHit += victim->loseRound(1);
      dam += 2;
    }

    caster->sendTo(COLOR_MOBS, fmt("%s is hurled %s out of the room!\n\r") % 
         sstring(victim->getName()).cap() % dirs[dr]);
    victim->sendTo(COLOR_MOBS, fmt("%s hurls you %s out of the room!\n\r") % 
            sstring(caster->getName()).cap() % dirs[dr]);
    sprintf(buf, "$N is hurled %s out of the room by $n.", dirs[dr]);
    act(buf, TRUE, caster, 0, victim, TO_NOTVICT);

    caster->throwChar(victim, dr, FALSE, SILENT_YES, false);
      
    act("$N is hurled into the room!", TRUE, victim, 0, victim, TO_ROOM);
     
    rc = victim->crashLanding(POSITION_SITTING);
    if (IS_SET_DELETE(rc, DELETE_THIS))
      return DELETE_VICT;
      
    victim->addToWait(2 * combatRound(1));
    if (caster->reconcileDamage(victim, dam, SKILL_SHOULDER_THROW) == -1)
      return DELETE_VICT;
      
  }
  
  return TRUE;
}

int TBeing::aiHurl(dirTypeT dr, TBeing *victim)
{
  int rc, i;
  int bKnown = getSkillValue(SKILL_HURL);
 
  if (checkBusy()) { 
    return FALSE;
  }

  if (!sameRoom(*victim)) {
    sendTo("That person isn't around.\n\r");
    return FALSE;
  }

  if (victim->getPosition() <= POSITION_STUNNED) {
    rc = hurlHit(this, victim, dr);
    if (IS_SET_DELETE(rc, DELETE_THIS))
      return DELETE_THIS;
    if (IS_SET_DELETE(rc, DELETE_VICT))
      return DELETE_VICT;
  } else if ((i = specialAttack(victim,SKILL_HURL)) &&
             (i != GUARANTEED_FAILURE) &&
             bSuccess(bKnown, SKILL_HURL)) {
    rc = hurlHit(this, victim, dr);
    if (rc) {
      addSkillLag(SKILL_HURL, rc);
    }
    if (IS_SET_DELETE(rc, DELETE_VICT))
      return DELETE_VICT;
    if (IS_SET_DELETE(rc, DELETE_THIS))
      return DELETE_THIS;
  } else {
    rc = hurlMiss(this, victim);
    if (rc) {
      addSkillLag(SKILL_HURL, rc);
    }
    if (IS_SET_DELETE(rc, DELETE_THIS))
      return DELETE_THIS;
  }

  /*
  if (IS_SET_DELETE(rc, DELETE_VICT)) {
    if (vict)
      return rc;
    delete victim;
    victim = NULL;
    REM_DELETE(rc, DELETE_VICT);
  }
  */
  return rc;
}

int hurl(TBeing *caster, TBeing *victim, char *direction)
{
  int percent;
  int i = 0,level;
  int rc;
  const int THROW_MOVE        = 10;
 
  if (caster->checkPeaceful("You feel too peaceful to contemplate violence.\n\r"))
    return FALSE;
 
  if (caster->noHarmCheck(victim))
    return FALSE;
 
  if (victim->isImmortal() || IS_SET(victim->specials.act, ACT_IMMORTAL)) {
    caster->sendTo("Your hurl would not affect an immortal.\n\r");
    return FALSE;
  }
  if (caster->eitherArmHurt()) {
    caster->sendTo("It's very hard to hurl someone without use of your arms!\n\r");
    return FALSE;
  }
  if (caster->eitherLegHurt()) {
    caster->sendTo("You need the use of both legs to balance your hurl!\n\r");
    return FALSE;
  }
  if (caster->equipment[HOLD_RIGHT] && caster->equipment[HOLD_LEFT]) {
    caster->sendTo("You need a free hand to hurl.\n\r");
    return FALSE;
  }
  if (caster->getMove() < THROW_MOVE) {
    caster->sendTo("You are much too tired to execute a hurl.\n\r");
    return FALSE;
  }
  if (victim == caster) {
    caster->sendTo("Hurting yourself isn't a good idea.\n\r");
    return FALSE;
  }
  if (!victim->isHumanoid()) {
    caster->sendTo("You can only hurl humanoid creatures.\n\r");
    return FALSE;
  }
  if (caster->riding) {
    caster->sendTo("You can't do that while mounted!\n\r");
    return FALSE;
  }
  if (victim->riding && dynamic_cast<TBeing *> (victim->riding)) {
    caster->sendTo("You can't use that attack on a mounted person!\n\r");
    return FALSE;
  } else if (victim->riding) {
    caster->sendTo(COLOR_MOBS, fmt("You can't use that attack while %s is on %s!\n\r") % victim->getName() % victim->riding->getName());
    return FALSE;
  }
  if (victim->getPosition() < POSITION_STANDING)  {
    caster->sendTo(fmt("You can't hurl someone whom is already on the %s.\n\r") % 
    caster->roomp->describeGround());
    return FALSE;
  }
  if (caster->getPosition() != POSITION_STANDING)  {
    // deny to mounted and crawling, allow standing and fighting
    caster->sendTo("You can't hurl from your present position.\n\r");
    return FALSE;
  }
  if (victim->isFlying()) {
    caster->sendTo("You can't hurl someone that is flying.\n\r");
    return FALSE;
  }
  if (caster->getHeight() > 2*victim->getHeight()) {
    caster->sendTo("That creature is too short to hurl properly.\n\r");
    return FALSE;
  }
  if (3*caster->getHeight() < 2*victim->getHeight()) {
    caster->sendTo("You don't stand a chance of hurling somebody that tall!\n\r");
    return FALSE;
  }

  dirTypeT dr = getDirFromChar(direction);
  if (dr == DIR_NONE) {
    caster->sendTo("You need to give a direction to hurl.\n\r");
    return FALSE;
  }
  if (caster->roomp && !caster->roomp->dir_option[dr]){
    caster->sendTo("That direction seems to be blocked.\n\r");
    return FALSE;
  }

  TRoom *tRoom = (caster->roomp->dir_option[dr]->to_room ?
                  real_roomp(caster->roomp->dir_option[dr]->to_room) :
                  NULL);

  if (!tRoom || tRoom->isRoomFlag(ROOM_PEACEFUL)) {
    caster->sendTo("That is a peaceful room, you can not just hurl people in there!\n\r");
    return FALSE;
  }

  percent = 0;               
  level = caster->getSkillLevel(SKILL_HURL);
  int bKnown = caster->getSkillValue(SKILL_HURL);
  if (!caster->isImmortal())
    caster->addToMove(-THROW_MOVE); 

  if (victim->getPosition() <= POSITION_STUNNED) {
    rc = hurlHit(caster, victim, dr);
    if (IS_SET_DELETE(rc, DELETE_THIS))
      return DELETE_THIS;
    if (IS_SET_DELETE(rc, DELETE_VICT))
      return DELETE_VICT;
  } else if ((i = caster->specialAttack(victim,SKILL_HURL)) &&
             (i != GUARANTEED_FAILURE) &&
             caster->bSuccess(bKnown + percent, SKILL_HURL)) {
    rc = hurlHit(caster, victim, dr);
    if (rc) {
      caster->addSkillLag(SKILL_HURL, rc);
    }

    if (IS_SET_DELETE(rc, DELETE_VICT))
      return DELETE_VICT;
    if (IS_SET_DELETE(rc, DELETE_THIS))
      return DELETE_THIS;
  } else {
    rc = hurlMiss(caster, victim);
    if (rc) {
      caster->addSkillLag(SKILL_HURL, rc);
    }

    if (IS_SET_DELETE(rc, DELETE_THIS))
      return DELETE_THIS;
  }
  return FALSE;
}


int TBeing::doShoulderThrow(const char *argument, TBeing *vict)
{
  int rc;
  TBeing *victim;
  char name_buf[256];
    
  if (checkBusy()) { 
    return FALSE;
  }
  if (!doesKnowSkill(SKILL_SHOULDER_THROW)) {
    sendTo("You know nothing about shoulder throwing.\n\r");
    return FALSE;
  }
  strcpy(name_buf, argument);
  if (!(victim = vict)) {
    if (!(victim = get_char_room_vis(this, name_buf))) {
      if (!(victim = fight())) {
        sendTo("Shoulder throw whom?\n\r");
        return FALSE;
      }
    }
  }
  if (!sameRoom(*victim)) {
    sendTo("That person isn't around.\n\r");
    return FALSE;
  }
  rc = shoulderThrow(this,victim);

  if (rc){
    addSkillLag(SKILL_SHOULDER_THROW, rc);
    cantHit += loseRound(1);
  }

  if (IS_SET_DELETE(rc, DELETE_VICT)) {
    if (vict)
      return rc;
    delete victim;
    victim = NULL;
    REM_DELETE(rc, DELETE_VICT);
  }
  return rc;
}

int shoulderThrowMiss(TBeing *caster, TBeing *victim)
{
  int rc;

  act("$n misses $s attempt at shoulder throwing $N and falls on $s butt!",
            FALSE, caster, 0, victim, TO_NOTVICT);
  act("You fall as you attempt to shoulder throw $N!",
            FALSE, caster, 0, victim, TO_CHAR);
  act("You manage to avoid $n as $e tries to shoulder throw you!",
            FALSE, caster, 0, victim, TO_VICT);

  rc = caster->crashLanding(POSITION_SITTING);
  if (IS_SET_DELETE(rc, DELETE_THIS))
    return DELETE_THIS;

  caster->reconcileDamage(victim, 0, SKILL_CHOP);
  return TRUE;
}

int shoulderThrowHit(TBeing *caster, TBeing *victim, int)
{
  int rc;

  int dam = caster->getSkillDam(victim, SKILL_SHOULDER_THROW, caster->getSkillLevel(SKILL_SHOULDER_THROW), caster->getAdvLearning(SKILL_SHOULDER_THROW));

  //  caster->cantHit += caster->loseRound(2);
  victim->cantHit += victim->loseRound(1);
  if (((victim->hasPart(WEAR_ARM_R)) || (victim->hasPart(WEAR_ARM_L)))) {
    act("$n grabs $N by the arm and throws $M over $s shoulder!", FALSE, caster, 0, victim, TO_NOTVICT);
    act("$n grabs you by the arm and throws you over $s shoulder! ouch!", FALSE, caster, 0, victim, TO_VICT);
    act("You grab $N by the arm and throw $M over your shoulder!", FALSE, caster, 0, victim, TO_CHAR);
  } else {
    act("$n grabs $N by the body and throws $M over $s shoulder!", FALSE, caster, 0, victim, TO_NOTVICT);
    act("$n grabs you by the body and throws you over $s shoulder! ouch!", FALSE, caster, 0, victim, TO_VICT);
    act("You grab $N by the body and throw $M over your shoulder!", FALSE, caster, 0, victim, TO_CHAR);
    //    caster->cantHit += caster->loseRound(2);
    victim->cantHit += victim->loseRound(1);
    dam += 2;
  }
  act("$N lands flat on $S back!", FALSE, caster, 0, victim, TO_NOTVICT);
  act("$N lands flat on $S back!", FALSE, caster, 0, victim, TO_CHAR);
  act("You land flat on your back!", FALSE, caster, 0, victim, TO_VICT);

  rc = victim->crashLanding(POSITION_SITTING);
  if (IS_SET_DELETE(rc, DELETE_THIS))
    return DELETE_VICT;

  victim->addToWait(2 * combatRound(1));
  if (caster->reconcileDamage(victim, dam, SKILL_SHOULDER_THROW) == -1)
    return DELETE_VICT;
 
  return TRUE;
}
   
int shoulderThrow(TBeing *caster, TBeing *victim)
{
  int percent;
  int i = 0,level;
  int rc;
  const int THROW_MOVE        = 10;
 
  if (caster->checkPeaceful("You feel too peaceful to contemplate violence.\n\r"))
    return FALSE;
 
  if (caster->noHarmCheck(victim))
    return FALSE;
 
  if (victim->isImmortal() || IS_SET(victim->specials.act, ACT_IMMORTAL)) {
    caster->sendTo("Your shoulder throw would not affect an immortal.\n\r");
    return FALSE;
  }
  if (caster->eitherArmHurt()) {
    caster->sendTo("It's very hard to use the shoulder throw attack without use of your arms!\n\r");
    return FALSE;
  }
  if (caster->eitherLegHurt()) {
    caster->sendTo("You need the use of both legs to balance your shoulder throw!\n\r");
    return FALSE;
  }
  if (caster->equipment[HOLD_RIGHT] && caster->equipment[HOLD_LEFT]) {
    caster->sendTo("You need a free hand to shoulder throw.\n\r");
    return FALSE;
  }
  if (caster->getMove() < THROW_MOVE) {
    caster->sendTo("You are much too tired to execute a shoulder throw.\n\r");
    return FALSE;
  }
  if (victim == caster) {
    caster->sendTo("Hurting yourself isn't a good idea.\n\r");
    return FALSE;
  }
  if (!victim->isHumanoid()) {
    caster->sendTo("You can only shoulder throw humanoid creatures.\n\r");
    return FALSE;
  }
  if (caster->riding) {
    caster->sendTo("You can't do that while mounted!\n\r");
    return FALSE;
  }
  if (victim->riding && dynamic_cast<TBeing *> (victim->riding)) {
    caster->sendTo("You can't use that attack on a mounted person!\n\r");
    return FALSE;
  } else if (victim->riding) {
    caster->sendTo(COLOR_MOBS, fmt("You can't use that attack while %s is on %s!\n\r") % victim->getName() % victim->riding->getName());
    return FALSE;
  }

  if (victim->getPosition() < POSITION_STANDING)  {
    caster->sendTo(fmt("You can't shoulder throw someone whom is already on the %s.\n\r") % caster->roomp->describeGround());
    return FALSE;
  }
  if (caster->getPosition() != POSITION_STANDING)  {
    // deny to mounted and crawling, allow standing and fighting
    caster->sendTo("You can't shoulder throw from your present position.\n\r");
    return FALSE;
  }
  if (victim->isFlying()) {
    caster->sendTo("You can't shoulder throw someone that is flying.\n\r");
    return FALSE;
  }
  if (caster->getHeight() > 2*victim->getHeight()) {
    caster->sendTo("That creature is too short to shoulder throw properly.\n\r");
    return FALSE;
  }
  if (3*caster->getHeight() < 2*victim->getHeight()) {
    caster->sendTo("You don't stand a chance of shoulder throwing somebody that tall!\n\r");
    return FALSE;
  }
  percent = 0;               
  level = caster->getSkillLevel(SKILL_SHOULDER_THROW);
  int bKnown = caster->getSkillValue(SKILL_SHOULDER_THROW);
  if (!caster->isImmortal())
    caster->addToMove(-THROW_MOVE); 

  if (victim->getPosition() <= POSITION_STUNNED) {
    rc = shoulderThrowHit(caster, victim, bKnown + percent);
    if (IS_SET_DELETE(rc, DELETE_THIS))
      return DELETE_THIS;
    if (IS_SET_DELETE(rc, DELETE_VICT))
      return DELETE_VICT;
  } else if ((i = caster->specialAttack(victim,SKILL_SHOULDER_THROW)) &&
             (i != GUARANTEED_FAILURE) &&
             caster->bSuccess(bKnown + percent, SKILL_SHOULDER_THROW)) {
    rc = shoulderThrowHit(caster, victim, bKnown + percent);
    if (IS_SET_DELETE(rc, DELETE_VICT))
      return DELETE_VICT;
    if (IS_SET_DELETE(rc, DELETE_THIS))
      return DELETE_THIS;
  } else {
    rc = shoulderThrowMiss(caster, victim);
    if (IS_SET_DELETE(rc, DELETE_THIS))
      return DELETE_THIS;
  }
  return TRUE;

}





int TBeing::doDefenestrate(const char *argument, TBeing *vict)
{
  int rc;
  TBeing *victim;
  char name_buf[256], obje[100];
 
  if (checkBusy()) { 
    return FALSE;
  }
  if (!doesKnowSkill(SKILL_DEFENESTRATE)) {
    sendTo("You know nothing about defenestration.\n\r");
    return FALSE;
  }
  half_chop(argument, name_buf, obje);
  if (!(victim = vict)) {
    if (!(victim = get_char_room_vis(this, name_buf))) {
      if (!(victim = fight())) {
        sendTo("Defenestrate whom?\n\r");
        return FALSE;
      }
    }
  }
  if (!sameRoom(*victim)) {
    sendTo("That person isn't around.\n\r");
    return FALSE;
  }
  rc = defenestrate(this,victim,obje);

  if (IS_SET_DELETE(rc, DELETE_VICT)) {
    if (vict)
      return rc;
    delete victim;
    victim = NULL;
    REM_DELETE(rc, DELETE_VICT);
  }
  return rc;
}

int defenestrateMiss(TBeing *caster, TBeing *victim){
  int rc;

  act("$n misses $s attempt at defenestrating $N and falls on $s butt!",
            FALSE, caster, 0, victim, TO_NOTVICT);
  act("You fall as you attempt to defenestrate $N!",
            FALSE, caster, 0, victim, TO_CHAR);
  act("You manage to avoid $n as $e tries to defenestrate you!",
            FALSE, caster, 0, victim, TO_VICT);

  rc = caster->crashLanding(POSITION_SITTING);
  if (IS_SET_DELETE(rc, DELETE_THIS))
    return DELETE_THIS;

  caster->reconcileDamage(victim, 0, SKILL_SHOULDER_THROW);
  return TRUE;
}

static int defenestrateHit(TBeing *caster, TBeing *victim, int to_room, TWindow *window)
{
  int dam;
  int rc, percent = 0;
  char buf[256];

  // default damage
  dam = caster->getSkillDam(victim, SKILL_DEFENESTRATE, caster->getSkillLevel(SKILL_DEFENESTRATE), caster->getAdvLearning(SKILL_DEFENESTRATE));

  if (victim->doesKnowSkill(SKILL_COUNTER_MOVE) && 
      min(2*victim->GetMaxLevel(),100) > percent) {
    act("$N deftly resists your defenestration attempt.", 
          FALSE, caster, 0, victim, TO_CHAR);
    act("$N deftly resists $n's defenestration attempt.", 
          FALSE, caster, 0, victim, TO_NOTVICT);
    act("You deftly resist $n's attempt to defenestration you.", 
          FALSE, caster, 0, victim, TO_VICT);
  } else {
    //  caster->cantHit += caster->loseRound(2);
    victim->cantHit += victim->loseRound(2);
    if (((victim->hasPart(WEAR_ARM_R)) || (victim->hasPart(WEAR_ARM_L)))) {
      act("$n grabs $N by the arm and throws $M through $p.", 
	  FALSE, caster, window, victim, TO_NOTVICT);
      act("$n grabs you by the arm and throws you through $p.", 
	  FALSE, caster, window, victim, TO_VICT);
      act("You grab $N by the arm and throw $M through $p.", 
	  FALSE, caster, window, victim, TO_CHAR);
    } else {
      act("$n grabs $N by the body and throws $M through $p.", 
	  FALSE, caster, window, victim, TO_NOTVICT);
      act("$n grabs you by the body and throws you through $p.", 
	  FALSE, caster, window, victim, TO_VICT);
      act("You grab $N by the body and throw $M through $p.", 
	  FALSE, caster, window, victim, TO_CHAR);
      caster->cantHit += caster->loseRound(2);
      victim->cantHit += victim->loseRound(1);
      dam += 2;
    }

    caster->sendTo(COLOR_MOBS, fmt("%s is defenestrated out of the room!\n\r") % 
         sstring(victim->getName()).cap());
    victim->sendTo(COLOR_MOBS, fmt("%s defenestrates you out of the room!\n\r") % 
            sstring(caster->getName()).cap());
    sprintf(buf, "$N is defenestrated out of the room by $n.");
    act(buf, TRUE, caster, 0, victim, TO_NOTVICT);

    caster->throwChar(victim, to_room, FALSE, SILENT_YES, false);
      
    act("$N is defenestrated into the room!", TRUE, victim, 0, victim, TO_ROOM);
     
    rc = victim->crashLanding(POSITION_SITTING);
    if (IS_SET_DELETE(rc, DELETE_THIS))
      return DELETE_VICT;
      
    victim->addToWait(2 * combatRound(1));
    if (caster->reconcileDamage(victim, dam, SKILL_SHOULDER_THROW) == -1)
      return DELETE_VICT;
      
  }
  
  return TRUE;
}


int defenestrate(TBeing *caster, TBeing *victim, sstring direction)
{
  int percent;
  int i = 0,level;
  int rc;
  const int THROW_MOVE        = 10;
 
  if (caster->checkPeaceful("You feel too peaceful to contemplate violence.\n\r"))
    return FALSE;
 
  if (caster->noHarmCheck(victim))
    return FALSE;
 
  if (victim->isImmortal() || IS_SET(victim->specials.act, ACT_IMMORTAL)) {
    caster->sendTo("Your defenestrate would not affect an immortal.\n\r");
    return FALSE;
  }
  if (caster->eitherArmHurt()) {
    caster->sendTo("It's very hard to defenestrate someone without use of your arms!\n\r");
    return FALSE;
  }
  if (caster->eitherLegHurt()) {
    caster->sendTo("You need the use of both legs to balance your defenestration!\n\r");
    return FALSE;
  }
  if (caster->equipment[HOLD_RIGHT] && caster->equipment[HOLD_LEFT]) {
    caster->sendTo("You need a free hand to defenestrate.\n\r");
    return FALSE;
  }
  if (caster->getMove() < THROW_MOVE) {
    caster->sendTo("You are much too tired to execute a defenestrate.\n\r");
    return FALSE;
  }
  if (victim == caster) {
    caster->sendTo("Hurting yourself isn't a good idea.\n\r");
    return FALSE;
  }
  if (!victim->isHumanoid()) {
    caster->sendTo("You can only defenestrate humanoid creatures.\n\r");
    return FALSE;
  }
  if (caster->riding) {
    caster->sendTo("You can't do that while mounted!\n\r");
    return FALSE;
  }
  if (victim->riding && dynamic_cast<TBeing *> (victim->riding)) {
    caster->sendTo("You can't use that attack on a mounted person!\n\r");
    return FALSE;
  } else if (victim->riding) {
    caster->sendTo(COLOR_MOBS, fmt("You can't use that attack while %s is on %s!\n\r") % victim->getName() % victim->riding->getName());
    return FALSE;
  }
  if (victim->getPosition() < POSITION_STANDING)  {
    caster->sendTo(fmt("You can't defenestrate someone whom is already on the %s.\n\r") % 
    caster->roomp->describeGround());
    return FALSE;
  }
  if (caster->getPosition() != POSITION_STANDING)  {
    // deny to mounted and crawling, allow standing and fighting
    caster->sendTo("You can't defenestrate from your present position.\n\r");
    return FALSE;
  }
  if (victim->isFlying()) {
    caster->sendTo("You can't defenestrate someone that is flying.\n\r");
    return FALSE;
  }
  if (caster->getHeight() > 2*victim->getHeight()) {
    caster->sendTo("That creature is too short to defenestrate properly.\n\r");
    return FALSE;
  }
  if (3*caster->getHeight() < 2*victim->getHeight()) {
    caster->sendTo("You don't stand a chance of defenestrating somebody that tall!\n\r");
    return FALSE;
  }

  // find window
  TObj *o=NULL;
  TWindow *window;
  if(direction.empty())
    direction="window";
  if(!(o=generic_find_obj(direction, FIND_OBJ_ROOM, caster))){
    caster->sendTo("You can't find that window.\n\r");
    return FALSE;
  }
  if(!(window=dynamic_cast<TWindow *>(o))){
    caster->sendTo("That's not a window!\n\r");
    return FALSE;
  }

  TPathFinder path;
  path.setNoMob(false);
  if(path.findPath(caster->in_room, findRoom(window->getTarget()))==DIR_NONE){
    caster->sendTo("That window doesn't appear to be appropriate for defenestration.\n\r");
    return FALSE;
  }


  TRoom *tRoom = real_roomp(window->getTarget());

  if (!tRoom || tRoom->isRoomFlag(ROOM_PEACEFUL)) {
    caster->sendTo("That is a peaceful room, you can not just defenestrate people in there!\n\r");
    return FALSE;
  }

  percent = 0;               
  level = caster->getSkillLevel(SKILL_DEFENESTRATE);
  int bKnown = caster->getSkillValue(SKILL_DEFENESTRATE);
  if (!caster->isImmortal())
    caster->addToMove(-THROW_MOVE); 

  if (victim->getPosition() <= POSITION_STUNNED) {
    rc = defenestrateHit(caster, victim, window->getTarget(), window);
    if (IS_SET_DELETE(rc, DELETE_THIS))
      return DELETE_THIS;
    if (IS_SET_DELETE(rc, DELETE_VICT))
      return DELETE_VICT;
  } else if ((i = caster->specialAttack(victim,SKILL_DEFENESTRATE)) &&
             (i != GUARANTEED_FAILURE) &&
             caster->bSuccess(bKnown + percent, SKILL_DEFENESTRATE)) {
    rc = defenestrateHit(caster, victim, window->getTarget(), window);
    if (rc) {
      caster->addSkillLag(SKILL_DEFENESTRATE, rc);
    }

    if (IS_SET_DELETE(rc, DELETE_VICT))
      return DELETE_VICT;
    if (IS_SET_DELETE(rc, DELETE_THIS))
      return DELETE_THIS;
  } else {
    rc = defenestrateMiss(caster, victim);
    if (rc) {
      caster->addSkillLag(SKILL_DEFENESTRATE, rc);
    }

    if (IS_SET_DELETE(rc, DELETE_THIS))
      return DELETE_THIS;
  }
  return FALSE;
}
