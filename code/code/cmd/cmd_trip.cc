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
#include "obj_base_clothing.h"

bool TBeing::canTrip(TBeing *victim, silentTypeT silent)
{
  if (checkBusy()) 
    return FALSE;
  
  spellNumT skill = getSkillNum(SKILL_TRIP);
  if (!doesKnowSkill(skill)) {
    if (!silent)
      sendTo("You know nothing about tripping.\n\r");
    return FALSE;
  }

  if (!sameRoom(*victim)) {
    if (!silent)
      sendTo("That person isn't around.\n\r");
    return FALSE;
  }
  switch (race->getBodyType()) {
    case BODY_MOSS:
    case BODY_MANTICORE:
    case BODY_GRIFFON:
    case BODY_SHEDU:
    case BODY_SPHINX:
    case BODY_LAMMASU:
    case BODY_WYVERN:
    case BODY_DRAGONNE:
    case BODY_HIPPOGRIFF:
    case BODY_CHIMERA:
    case BODY_FISH:
    case BODY_SNAKE:
    case BODY_NAGA:
    case BODY_BIRD:
    case BODY_TREE:
    case BODY_PARASITE:
    case BODY_VEGGIE:
    case BODY_LION:
    case BODY_FELINE:
    case BODY_REPTILE:
    case BODY_DINOSAUR:
    case BODY_FOUR_LEG:
    case BODY_PIG:
    case BODY_FROG:
    case BODY_WYVELIN:
      if (!silent)
        sendTo("You have the wrong form to trip.\n\r");
      return FALSE;
    default:
      break;
  }
  if (isFlying())  {
    if (!silent)
      sendTo("You can't trip while flying.\n\r");
    return FALSE;
  }
  if (victim->isFlying())  {
    if (!silent)
      sendTo("You can't trip them while they are flying.\n\r");
    return FALSE;
  }
  if (checkPeaceful("You feel too peaceful to contemplate violence.\n\r"))
    return FALSE;

  if (eitherLegHurt()) {
    if (!silent)
      sendTo("It's very hard to trip without the use of your legs!\n\r");
    return FALSE;
  }

  if (victim->riding) {
    if (!silent)
      sendTo(COLOR_MOBS, format("You are unable to trip %s off of %s!\n\r") % victim->getName() % victim->riding->getName());
    return FALSE;
  }
  if (isSwimming()) {
    if (!silent)
      sendTo("It's near impossible to trip someone while in water.\n\r");
    return FALSE;
  }
  if (victim == this) {
    if (!silent)
      sendTo("Aren't we funny today...\n\r");
    return FALSE;
  }
  if (noHarmCheck(victim))
    return FALSE;

  if (riding) {
    if (!silent)
      sendTo("You can't trip someone while mounted!\n\r");
    return FALSE;
  }
  if (victim->isImmortal()) {
    if (!silent)
      sendTo("You can't trip an immortal.\n\r");
    return FALSE;
  }
  if (victim->getPosition() <= POSITION_SITTING) {
    if (!silent)

      sendTo(format("How can you trip someone already on the %s?!?\n\r") % roomp->describeGround());
    return FALSE;
  }    
  if (getMove() < 5) {
    if (!silent)
      sendTo("You don't have the vitality to trip anyone!\n\r");
    return FALSE;
  }
  if (!victim->hasLegs()) {
    if (!silent)
      sendTo("You can't trip someone who has no legs.\n\r");
    return FALSE;
  }

  if (victim->getHeight() < 12) {
    if (!silent)
      sendTo("That creature has less ground clearance than the height of your foot.\n\r");
    return FALSE;
  }
  
  if (3*getHeight() < victim->getHeight()) {
    if (!silent)
      sendTo("Rule of thumb:  you can't trip someone when their kneecaps are higher than your eye level.\n\r");
    return FALSE;
  }

  return TRUE;
}

static int trip(TBeing *c, TBeing *victim, spellNumT skill)
{
  int percent = 0, i = 0, rc = 0;

  if (!c->canTrip(victim, SILENT_NO))
    return FALSE;

#if 0
  if (::number(0,99) < victim->GetMaxLevel()) {
    act("You rush at $N, but $E steps backward avoiding your trip.",
       FALSE, c, 0, victim, TO_CHAR);
    act("$n rushes at you, but you step backward avoiding $s trip.",
       FALSE, c, 0, victim, TO_VICT);
    act("$n rushes at $N, but $E steps backward avoiding $s trip.",
       FALSE, c, 0, victim, TO_NOTVICT);
    // don't allow this to prevent a fight
    c->reconcileDamage(victim, 0, skill);
    // return true so the lag and stuff
    return TRUE;
  }
#endif

  // some modifications to account for dexterity, and level 
  percent = 0;
  percent += ((int) c->getWeight() - (int) victim->getWeight())/20;

  // Make a level based adjustment - Brutius
  int level = c->getSkillLevel(skill);
  percent += 2*(level - victim->GetMaxLevel());

  // things with more legs are tougher - Maror
  if (victim->isFourLegged()) {
    c->sendTo("The stability of this creature makes it more difficult to trip.\n\r");
    percent -= 50;
  }

  int bKnown = c->getSkillValue(skill);
  i = c->specialAttack(victim,skill);
  if (i &&
      (bKnown > 0) &&
      (i != GUARANTEED_FAILURE) &&
      (!victim->canCounterMove(bKnown)) &&
      c->bSuccess(bKnown + percent, skill)) {
    rc = c->tripSuccess(victim, skill);
    if (IS_SET_DELETE(rc, DELETE_THIS) || IS_SET_DELETE(rc, DELETE_VICT))
      return rc;
    return TRUE;
  } else {
    rc = c->tripFail(victim, skill);
    if (IS_SET_DELETE(rc, DELETE_THIS) || IS_SET_DELETE(rc, DELETE_VICT))
      return rc;
    return TRUE;
  }
}

int TBeing::tripFail(TBeing *victim, spellNumT skill)
{
  int rc;

  act("$n attempts to trip $N but $E quickly hops over $n's leg.", 
      FALSE, this, 0, victim, TO_NOTVICT);
  act("You attempt to trip $N but $E quickly hops over your leg.", 
      FALSE, this, 0, victim, TO_CHAR);
  act("$n tries to trip you but you quickly hop over $s leg.", 
      FALSE, this, 0, victim, TO_VICT);

  if (hasLegs()) {
    rc = crashLanding(POSITION_SITTING);
    if (IS_SET_DELETE(rc, DELETE_THIS))
      return DELETE_THIS;

    sendTo(format("%sYou lose your balance and fall over.%s\n\r") % red() % norm());
    act("<r>$n loses $s balance and falls over.<1>",TRUE, this, 0, 0, TO_ROOM);

    rc = trySpringleap(victim);
    if (IS_SET_DELETE(rc, DELETE_THIS) || IS_SET_DELETE(rc, DELETE_VICT))
      return rc;
  }
  reconcileDamage(victim, 0, skill);
  return FALSE;
}

int TBeing::tripSuccess(TBeing *victim, spellNumT skill)
{
  int rc = 0;
  int distNum = 0;


  act("$n sticks a leg out and trips $N to the ground!", FALSE, this, 0, victim, TO_NOTVICT);
  act("You quickly stick your leg out and trip $N to the ground!", FALSE, this, 0, victim, TO_CHAR);
  act("$n sticks a leg out and trips you!", FALSE, this, 0, victim, TO_VICT, ANSI_BLUE);
  act("You fall flat on your face!", FALSE, this, 0, victim, TO_VICT, ANSI_BLUE);
 
  distNum = 1;
  if (isLucky(levelLuckModifier(victim->GetMaxLevel())))
    distNum++;
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

  float wait = combatRound(discArray[SKILL_TRIP]->lag);
  wait = (wait * 100.0 / getSkillDiffModifier(SKILL_TRIP));
  addToMove(-5);
  wait += 0.5;
  // round up
  wait += 0.5;

  victim->addToWait((int) wait);

  if (victim->spelltask) 
    victim->addToDistracted(distNum, FALSE);

  reconcileHurt(victim, 0.01);

  if (reconcileDamage(victim, 0, skill) == -1)
    return DELETE_VICT;

  return FALSE;
}

int TBeing::doTrip(const char *argument, TBeing *vict)
{
  int rc;
  TBeing *victim;
  char name_buf[256];

  spellNumT skill = getSkillNum(SKILL_TRIP);

  strcpy(name_buf, argument);

  if (!(victim = vict)) {
    if (!(victim = get_char_room_vis(this, name_buf))) {
      if (!(victim = fight())) {
        sendTo("Trip whom?\n\r");
        return FALSE;
      }
    }
  }

  if ((rc = trip(this, victim, skill)))
    addSkillLag(skill,rc);

  if (IS_SET_DELETE(rc, DELETE_VICT)) {
    if (vict)
      return rc;
       
    delete victim;
    victim = NULL;
    REM_DELETE(rc, DELETE_VICT);
  }
  return rc;
}

