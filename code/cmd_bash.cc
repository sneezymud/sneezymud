//////////////////////////////////////////////////////////////////////////
//
// SneezyMUD - All rights reserved, SneezyMUD Coding Team
//
//////////////////////////////////////////////////////////////////////////


#include "stdsneezy.h"
#include "combat.h"
#include "obj_base_clothing.h"

bool TBeing::canBash(TBeing *victim, silentTypeT silent)
{
  if (checkBusy()) 
    return FALSE;
  
  spellNumT skill = getSkillNum(SKILL_BASH);
  if (!doesKnowSkill(skill)) {
    if (!silent)
      sendTo("You know nothing about bashing.\n\r");
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
        sendTo("You have the wrong form to bash.\n\r");
      return FALSE;
    default:
      break;
  }
  if (desc && (isFlying() && !victim->isFlying()))  {
    if (!silent)
      sendTo("You can't bash while flying if your victim isn't flying.\n\r");
    return FALSE;
  }
  if (checkPeaceful("You feel too peaceful to contemplate violence.\n\r"))
    return FALSE;

  if (eitherLegHurt()) {
    if (!silent)
      sendTo("It's very hard to bash without the use of your legs!\n\r");
    return FALSE;
  }

  if (victim->riding && !dynamic_cast<TBeing *>(victim->riding)) {
    if (!silent)
      sendTo(COLOR_MOBS, fmt("You are unable to bash %s off of %s!\n\r") % victim->getName() % victim->riding->getName());
    return FALSE;
  }
  if (isSwimming()) {
    if (!silent)
      sendTo("It's near impossible to bash while in water.\n\r");
    return FALSE;
  }
  if (victim == this) {
    if (!silent)
      sendTo("Aren't we funny today...\n\r");
    return FALSE;
  }
  if (noHarmCheck(victim))
    return FALSE;

  if (victim->isFlying() && !isFlying()) {
    if (!silent)
      sendTo("You can't bash someone that is flying unless you are also.\n\r");
    return FALSE;
  }
  if (riding) {
    if (!silent)
      sendTo("You can't bash while mounted!\n\r");
    return FALSE;
  }
  if (getHeight() > 3*victim->getHeight()) {
    if (!silent)
      sendTo("That creature is too short to bash properly.\n\r");
    return FALSE;
  }
  if (3*getHeight() < 2*victim->getHeight()) {
    if (!silent)
      sendTo("You don't stand a chance of bashing somebody that tall!\n\r");
    return FALSE;
  }
  if (victim->isImmortal()) {
    if (!silent)
      sendTo("You slam into them but seeing how they are immortal it does no good.\n\r");
    return FALSE;
  }
  if (victim->getPosition() <= POSITION_SITTING) {
    if (!silent)
      sendTo(fmt("How can you bash someone already on the %s?!?\n\r") % roomp->describeGround());
    return FALSE;
  }    
  if (getMove() < 5) {
    if (!silent)
      sendTo("You don't have the vitality to bash anyone!\n\r");
    return FALSE;
  }
  if (!victim->hasLegs()) {
    if (!silent)
      sendTo("You can't knock them over, they have no legs.\n\r");
    return FALSE;
  }

  return TRUE;
}

static int bash(TBeing *c, TBeing *victim, spellNumT skill)
{
  int percent = 0, i = 0, shieldAdj = 0,  learning = 0;
  int rc = 0;
  float shieldWtBonus=0.0;
  TThing *obj = NULL;
  TBaseClothing *tbc = NULL;

  if (!c->canBash(victim, SILENT_NO))
    return FALSE;

  if ((learning = c->getAdvLearning(skill)) <= 10) {
    if ((obj = c->heldInSecHand()) &&
        (tbc = dynamic_cast<TBaseClothing *>(obj)) &&
        tbc->isShield()) {
      shieldWtBonus=(tbc->getWeight() / 5) + 1;
    } else {
      c->sendTo("You are not skilled enough to bash without a shield!\n\r");
      return FALSE;
    }
  } else if (skill == SKILL_BASH) {
    // warrior bashing with disc_brawling > 10%
    if (learning <= 25) 
      shieldAdj = learning;
    else 
      shieldAdj = 25 + ((learning-25)/4);
  } else {
    if (((obj = c->heldInSecHand()) &&
        (tbc = dynamic_cast<TBaseClothing *>(obj)) &&
         tbc->isShield())) {
        shieldWtBonus=(tbc->getWeight() / 5) + 1;
    } else if (learning == MAX_DISC_LEARNEDNESS) {
    } else {
      c->sendTo("You are not skilled enough to bash without a shield!\n\r");
      return FALSE;
    }
  }

  // high level warriors are able to abuse bash
  // mob on ground, attacks/defends horribly and takes extra dam
  // encourage warriors to use alternative more costly methods like
  // grapple, and bslam
  // bat 11/13/98
#if 0
  if (::number(0,99) < victim->GetMaxLevel()) {
    act("You rush at $N, but $E steps backward avoiding your bash.",
       FALSE, c, 0, victim, TO_CHAR);
    act("$n rushes at you, but you step backward avoiding $s bash.",
       FALSE, c, 0, victim, TO_VICT);
    act("$n rushes at $N, but $E steps backward avoiding $s bash.",
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

  if (victim->fight() && victim->fight() != c) {
    // make it hard for non-tanks to successfully bash
    percent -= 35;
  }
  if (victim->riding) 
    percent -= 33;

  percent += (int) shieldWtBonus;

  // Make a level based adjustment - Brutius
  int level = c->getSkillLevel(skill);
  percent += 2*(level - victim->GetMaxLevel());

  int bKnown = c->getSkillValue(skill);
  if (shieldAdj)
    percent -= (50 - shieldAdj);

  // we use to check for bKnown > percent
  // I changed it to bKnown > 0
  // otherwise a L50 bashing an L1 would fail due to the level adjustment
  // I believe the reason for the check is to keep 0 learn, but good percent
  // earners from bashing.
  // keep bSuccess check at end so counters are all OK
  i = c->specialAttack(victim,skill);
  if (i &&
      (bKnown > 0) &&
      (i != GUARANTEED_FAILURE) &&
      (!victim->canCounterMove(bKnown)) &&
      c->bSuccess(bKnown + percent, skill)) {
    rc = c->bashSuccess(victim, skill);
    if (IS_SET_DELETE(rc, DELETE_THIS) || IS_SET_DELETE(rc, DELETE_VICT))
      return rc;
    return TRUE;
  } else {
    rc = c->bashFail(victim, skill);
    if (IS_SET_DELETE(rc, DELETE_THIS) || IS_SET_DELETE(rc, DELETE_VICT))
      return rc;
    return TRUE;
  }
}

int TBeing::bashFail(TBeing *victim, spellNumT skill)
{
  int rc;

  act("$n attempts to bash $N who deftly dodges the attack.", 
      FALSE, this, 0, victim, TO_NOTVICT);
  act("You attempt to bash $N who deftly dodges the attack.", 
      FALSE, this, 0, victim, TO_CHAR);
  act("You deftly dodge $n's attempt to bash you!", 
      FALSE, this, 0, victim, TO_VICT);

  if (hasLegs()) {
    rc = crashLanding(POSITION_SITTING);
    if (IS_SET_DELETE(rc, DELETE_THIS))
      return DELETE_THIS;

    sendTo(fmt("%sYou fall over.%s\n\r") % red() % norm());
    act("$n falls over.",TRUE, this, 0, 0, TO_ROOM);

    rc = trySpringleap(victim);
    if (IS_SET_DELETE(rc, DELETE_THIS) || IS_SET_DELETE(rc, DELETE_VICT))
      return rc;
  }
  reconcileDamage(victim, 0, skill);
  return FALSE;
}

int TBeing::bashSuccess(TBeing *victim, spellNumT skill)
{
  int rc = 0;
  int distNum = 0;
  //  int level = 0;


  TThing *obj = NULL;
  TBaseClothing *tbc = NULL;
  int shieldDam = 0;


  if (victim->riding) {
    act("You knock $N off $p.", FALSE, this, victim->riding, victim, TO_CHAR);
    act("$n knocks $N off $p.", FALSE, this, victim->riding, victim, TO_NOTVICT);
    act("$n knocks you off $p.", FALSE, this, victim->riding, victim, TO_VICT);
    victim->dismount(POSITION_STANDING);
  }

  act("$n knocks $N on $S butt!", FALSE, this, 0, victim, TO_NOTVICT);
  act("You send $N sprawling.", FALSE, this, 0, victim, TO_CHAR);
  act("You tumble as $n knocks you over",
        FALSE, this, 0, victim, TO_VICT, ANSI_BLUE);
 
  //extra damage done by shield with spikes 10-20-00 -dash
  if (((obj = this->heldInSecHand()) &&
       (tbc = dynamic_cast<TBaseClothing *>(obj)) &&
       tbc->isShield() && (tbc->isSpiked() || tbc->isObjStat(ITEM_SPIKED)))) {
    shieldDam = (int)((tbc->getWeight()/3) + 1);

    act("The spikes on your $o sink into $N.", FALSE, this, tbc, victim, TO_CHAR);
    act("The spikes on $n's $o sink into $N.", FALSE, this, tbc, victim, TO_NOTVICT);
    act("The spikes on $n's $o sink into you.", FALSE, this, tbc, victim, TO_VICT);

    if (this->reconcileDamage(victim, shieldDam, TYPE_STAB) == -1)
      return DELETE_VICT;

  }

  //  level = getSkillLevel(skill);
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

  // see balance notes for bash:
  // the effect here should be strictly to prevent skill-use
  // it should NOT cause loss of attacks, or do damage
  float wt = combatRound(discArray[SKILL_BASH]->lag);
  // adjust based on bash difficulty
  wt = (wt * 100.0 / getSkillDiffModifier(SKILL_BASH));

  // Take a few moves from basher
  // since we cost some moves to perform, allow an extra 1/2 round of lag
  addToMove(-5);
  wt += 0.5;

  // round up
  wt += 0.5;

  // the fact that we fail over on fail is matched by knocking person
  // over on success, so award no benefit for this

  victim->addToWait((int) wt);

  if (victim->spelltask) 
    victim->addToDistracted(distNum, FALSE);

  reconcileHurt(victim, 0.01);

  if (reconcileDamage(victim, 0, skill) == -1)
    return DELETE_VICT;

  return FALSE;
}

int TBeing::doBash(const char *argument, TBeing *vict)
{
  int rc;
  TBeing *victim;
  char name_buf[256];

  spellNumT skill = getSkillNum(SKILL_BASH);

  strcpy(name_buf, argument);

  if (!(victim = vict)) {
    if (!(victim = get_char_room_vis(this, name_buf))) {
      if (!(victim = fight())) {
        sendTo("Bash whom?\n\r");
        return FALSE;
      }
    }
  }

  if ((rc = bash(this, victim, skill)))
    addSkillLag(skill,rc);

  if (IS_SET_DELETE(rc, DELETE_VICT)) {
    if (vict)
      return rc;
       
    delete victim;
    victim = NULL;
    REM_DELETE(rc, DELETE_VICT);;
  }
  return rc;
}

