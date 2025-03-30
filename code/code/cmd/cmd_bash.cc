//////////////////////////////////////////////////////////////////////////
//
// SneezyMUD - All rights reserved, SneezyMUD Coding Team
//
//////////////////////////////////////////////////////////////////////////

#include "being.h"
#include "combat.h"
#include "obj_base_clothing.h"
#include "obj_base_weapon.h"
#include "being.h"
#include "room.h"
#include "extern.h"
#include "handler.h"

bool TBeing::canBash(TBeing* victim, silentTypeT silent) {
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

  if (checkPeaceful("You feel too peaceful to contemplate violence.\n\r"))
    return FALSE;

  if (eitherLegHurt() && !isFlying()) {
    if (!silent)
      sendTo("It's very hard to bash without the use of your legs!\n\r");
    return FALSE;
  }

  if (victim->riding && !dynamic_cast<TBeing*>(victim->riding)) {
    if (!silent)
      sendTo(COLOR_MOBS, format("You are unable to bash %s off of %s!\n\r") %
                           victim->getName() % victim->riding->getName());
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

  if (victim->isImmortal()) {
    if (!silent)
      sendTo(
        "You slam into them but seeing how they are immortal it does no "
        "good.\n\r");
    return FALSE;
  }

  if (victim->getPosition() <= POSITION_SITTING) {
    if (!silent)
      sendTo(format("How can you bash someone already on the %s?!?\n\r") %
             roomp->describeGround());
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

  if ((isSwimming() || victim->isSwimming())) {
    if (!silent)
      sendTo("Bashing while swimming doesn't work very well.\n\r");
    return FALSE;
  }

  return TRUE;
}

static int bash(TBeing* attacker, TBeing* victim, spellNumT skill) {
  int advLearnedness = attacker->getAdvLearning(skill);
  TBaseWeapon* weaponInPrimaryHand =
    dynamic_cast<TBaseWeapon*>(attacker->heldInPrimHand());
  TBaseWeapon* weaponInSecondaryHand =
    dynamic_cast<TBaseWeapon*>(attacker->heldInSecHand());
  TBaseClothing* itemInSecondaryHand =
    dynamic_cast<TBaseClothing*>(attacker->heldInSecHand());
  bool isWielding2Hander =
    weaponInPrimaryHand && weaponInPrimaryHand->isPaired();
  bool isHoldingShield = itemInSecondaryHand && itemInSecondaryHand->isShield();
  bool isBarehanded = !weaponInPrimaryHand && !weaponInSecondaryHand;

  // Without this check, most mobs won't be able to bash
  if (attacker->isPc()) {
    /*
      Allow bash with weapon + no shield after 10% in advanced disc
    */
    if (advLearnedness <= 10 && !isHoldingShield) {
      attacker->sendTo(
        "You're not skilled enough to bash without a shield!\n\r");
      return FALSE;
    }

    /*
      Bashing while dual wielding, or single wielding a one-handed weapon in
      either hand with no shield, requires even higher advanced disc as there's
      no shield or heavy weapon to assist with the bash
    */
    if (advLearnedness < 50 && !isHoldingShield && !isWielding2Hander &&
        (weaponInPrimaryHand || weaponInSecondaryHand)) {
      attacker->sendTo(
        "You're not skilled enough to bash with one-handed weapons!\n\r");
      return FALSE;
    }

    /*
      Require near-maxed advanced disc to bash with no weapons or shield at all.
      Probably not a super common scenario.
    */
    if (advLearnedness < 90 && isBarehanded && !isHoldingShield) {
      attacker->sendTo(
        "You're not skilled enough to bash without a weapon!\n\r");
      return FALSE;
    }
  }

  /*
    If attacker is not tanking, pass agility check to successfully get into
    position for a bash attempt. Makes bashing while not tanking more difficult
    but still possible.
  */
  if (victim->fight() && victim->fight() != attacker && !attacker->isAgile(0)) {
    attacker->sendTo(
      "You try to line up a bash but can't find a good angle!\n\r");
    // Still add skill lag, as with deikhan charge
    return TRUE;
  }

  /*
    Calculate modifier for specialAttack

    Per combat.cc balance notes, 16.667 points of mod is equivalent to one
    level's worth of advantage/disadvantage.
  */
  const float ONE_LEVEL = 16.667;
  float modifier = 0.0;

  /*
     Maxed advanced disc lets one bash as if they were 10 levels higher. Bonus
     increases linearly with advanced disc learnedness.
   */
  modifier += ONE_LEVEL * ((float)advLearnedness / 10.0);

  /*
    Attacker with advanced disc using shield to bash can get another bonus of
    up to +5 levels to distinguish shield bashing from weapon/shoulder bashing
    and also makes bash more of a tank-centric ability.
  */
  modifier += ONE_LEVEL * ((float)advLearnedness / 20.0);

  float attackerWeight = attacker->getWeight();
  float victimWeight = victim->getWeight();

  // Include attacker's shield or two-handed weapon in weight calculation
  if (isHoldingShield)
    attackerWeight += itemInSecondaryHand->getWeight();
  else if (isWielding2Hander)
    attackerWeight += weaponInPrimaryHand->getWeight();

  // Each 20% weight difference between attacker and victim modifies
  // chance by one level's worth, up to max of +/- 10 levels.
  if (attackerWeight > victimWeight)
    modifier +=
      ONE_LEVEL * min(10.0, ((attackerWeight - victimWeight) / victimWeight) *
                              100.0 / 20.0);
  else if (victimWeight > attackerWeight)
    modifier -=
      ONE_LEVEL * min(10.0, ((victimWeight - attackerWeight) / attackerWeight) *
                              100.0 / 20.0);

  /*
    Other possible modifiers that could be applied here: stat-based
    (bonus/penalty based on low/high attacker/victim str/bra/agi/spd?),
    environmental factors such as weather/lighting, group-related situations (#
    of attackers, certain spell effects on victim?)

    Basically, anything that would situationally affect the success of *this
    specific attempt* can be converted into a +/- level modifier passed to the
    specialAttack overload, in effect using the circumstances at the time of the
    attack to make it more or less likely to land against this specific victim
    based on their level/AC/other defense.
  */

  int bKnown = attacker->getSkillValue(skill);
  bool wasSkillExecutionSuccessful = attacker->bSuccess(bKnown, skill);

  // Don't give bonuses/penalties to mobs - though could be a possibility
  // for future development
  int attackResult = attacker->isPc()
                       ? attacker->specialAttack(victim, skill, modifier)
                       : attacker->specialAttack(victim, skill);
  bool wasAttackCountered = victim->canCounterMove(bKnown / 2) ||
                            victim->canFocusedAvoidance(bKnown / 2);

  if (wasSkillExecutionSuccessful && attackResult &&
      attackResult != GUARANTEED_FAILURE && !wasAttackCountered) {
    int rc = attacker->bashSuccess(victim, skill, isHoldingShield,
      itemInSecondaryHand);
    if (IS_SET_DELETE(rc, DELETE_THIS) || IS_SET_DELETE(rc, DELETE_VICT))
      return rc;
    return TRUE;
  } else {
    int rc = attacker->bashFail(victim, skill, wasSkillExecutionSuccessful,
      attackResult, wasAttackCountered);
    if (IS_SET_DELETE(rc, DELETE_THIS) || IS_SET_DELETE(rc, DELETE_VICT))
      return rc;
    return TRUE;
  }
}

int TBeing::bashFail(TBeing* victim, spellNumT skill,
  bool wasSkillExecutionSuccessful, int attackResult, bool wasAttackCountered) {
  /*
    Replacing single fail message with specific ones depending on how the skill
    failed, to give players more information about what went wrong.
  */

  // Message if attacker's skill check failed - need to hone bash more
  if (!wasSkillExecutionSuccessful) {
    act("$n makes a sloppy effort to bash $N, who easily avoids the attack.",
      FALSE, this, 0, victim, TO_NOTVICT);
    act("Your form is sloppy and $N easily avoids your bash.", FALSE, this, 0,
      victim, TO_CHAR);
    act("You easily avoid $n's sloppy attempt to bash into you.", FALSE, this,
      0, victim, TO_VICT);
  }
  // Message if specialAttack check failed - victim's defense was too strong
  else if (!attackResult || attackResult == GUARANTEED_FAILURE) {
    act(
      "$n almost connects with an impressive bash but $N manages to dodges the "
      "attack.",
      FALSE, this, 0, victim, TO_NOTVICT);
    act(
      "You almost connect with an impressive bash but $N manages dodges the "
      "attack.",
      FALSE, this, 0, victim, TO_CHAR);
    act(
      "$n almost catches you with a bash, but you manage to dodge out of the "
      "way!",
      FALSE, this, 0, victim, TO_VICT);
  }
  // Message if victim countered bash via countermove or focused avoidance
  else if (wasAttackCountered) {
    act(
      "$N skillfully counters $n's bash, stepping aside at just the right "
      "moment.",
      FALSE, this, 0, victim, TO_NOTVICT);
    act("You attempt to bash $N but $e skillfully counters your attack.", FALSE,
      this, 0, victim, TO_CHAR);
    act("You predict $n's bash and easily counter the attack.", FALSE, this, 0,
      victim, TO_VICT);
  }

  if (hasLegs()) {
    int rc = crashLanding(POSITION_SITTING);
    if (IS_SET_DELETE(rc, DELETE_THIS))
      return DELETE_THIS;

    sendTo(format("%sYou fall over.%s\n\r") % red() % norm());
    act("$n falls over.", TRUE, this, 0, 0, TO_ROOM);

    rc = trySpringleap(victim);
    if (IS_SET_DELETE(rc, DELETE_THIS) || IS_SET_DELETE(rc, DELETE_VICT))
      return rc;
  }

  reconcileDamage(victim, 0, skill);
  return FALSE;
}

int TBeing::bashSuccess(TBeing* victim, spellNumT skill, bool isHoldingShield,
  TObj* itemInSecondaryHand) {
  if (victim->riding) {
    act("You knock $N off $p.", FALSE, this, victim->riding, victim, TO_CHAR);
    act("$n knocks $N off $p.", FALSE, this, victim->riding, victim,
      TO_NOTVICT);
    act("$n knocks you off $p.", FALSE, this, victim->riding, victim, TO_VICT);
    victim->dismount(POSITION_SITTING);
  } else {
    act("$n knocks $N on $S butt!", FALSE, this, 0, victim, TO_NOTVICT);
    act("You send $N sprawling.", FALSE, this, 0, victim, TO_CHAR);
    act("You tumble as $n knocks you over", FALSE, this, 0, victim, TO_VICT,
      ANSI_BLUE);
  }

  int shieldDam =
    getSkillDam(victim, skill, getSkillLevel(skill), getAdvLearning(skill));

  // extra damage done by shield with spikes 10-20-00 -dash
  if (isHoldingShield && itemInSecondaryHand) {
    shieldDam += (int)((itemInSecondaryHand->getWeight() / 4.0) + 1.0);

    if (itemInSecondaryHand->isSpiked() || itemInSecondaryHand->isObjStat(ITEM_SPIKED)) {
       int spikeDam = :: number (1,4);
      shieldDam += spikeDam;
      wearSlotT limb = victim->getPartHit(this, false);
      victim->addCurLimbHealth(limb, -spikeDam);
      if (victim->isUndead() || victim->isImmune(IMMUNE_BLEED,limb) || victim->isLimbFlags(limb, PART_MISSING)) {
      act("The spikes on your $o sink into $N's %s.", FALSE, this,
        itemInSecondaryHand, victim, TO_CHAR);
      act("The spikes on $n's $o sink into $N's %s.", FALSE, this,
        itemInSecondaryHand, victim, TO_NOTVICT);
      act("The spikes on $n's $o sink into your %s.", FALSE, this,
        itemInSecondaryHand, victim, TO_VICT);
      } else {
      act("The spikes on your $o sink into $N's %s and open up a bloody wound!", FALSE, this,
        itemInSecondaryHand, victim, TO_CHAR);
      act("The spikes on $n's $o sink into $N's %s and open up a bloody wound! .", FALSE, this,
        itemInSecondaryHand, victim, TO_NOTVICT);
      act("The spikes on $n's $o sink into your %s and open up a bloody wound!", FALSE, this,
        itemInSecondaryHand, victim, TO_VICT);
      victim->rawBleed(limb, 250, SILENT_YES, CHECK_IMMUNITY_NO);
      }
      if (!::number(0,3)) {
      itemInSecondaryHand->addToMaxStructPoints(-1);
      itemInSecondaryHand->addToStructPoints(-spikeDam);
      act("$o is damaged slightly as the spikes rip free.", false, this, itemInSecondaryHand, victim, TO_CHAR);
      }
    }
  }

  if (reconcileDamage(victim, shieldDam, SKILL_BASH) == -1)
    return DELETE_VICT;

  int distractionBonus = 1;
  /*
    This seems like it could be a good place to add more distraction
    bonus (higher chance to disrupt casting on successful bash) based
    on brawling learnedness or other things
  */
  if (isLucky(levelLuckModifier(victim->GetMaxLevel())))
    distractionBonus++;

  int rc = victim->crashLanding(POSITION_SITTING);
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
  float waitTimeMod = combatRound(discArray[SKILL_BASH]->lag);

  // Take a few moves from basher
  // since we cost some moves to perform, allow an extra 1/2 round of lag
  addToMove(-5);
  waitTimeMod += 1.0;

  // the fact that we fall over on fail is matched by knocking person
  // over on success, so award no benefit for this
  victim->addToWait((int)waitTimeMod);

  if (victim->spelltask)
    victim->addToDistracted(distractionBonus, FALSE);

  reconcileHurt(victim, 0.01);

  return FALSE;
}

int TBeing::doBash(const char* argument, TBeing* vict) {
  int rc;
  TBeing* victim;
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

  /*
    Move most of the hard-fail checks into this function, as appears
    to be the intention based on how most other skills are coded.
  */
  if (!canBash(victim, SILENT_NO))
    return FALSE;

  if ((rc = bash(this, victim, skill)))
    addSkillLag(skill, rc);

  if (IS_SET_DELETE(rc, DELETE_VICT)) {
    if (vict)
      return rc;

    delete victim;
    victim = NULL;
    REM_DELETE(rc, DELETE_VICT);
  }
  return rc;
}