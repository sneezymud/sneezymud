#include "handler.h"
#include "being.h"
#include "combat.h"
#include "obj_base_weapon.h"

static int deathstroke(TBeing* caster, TBeing* victim) {
  bool wasSuccess = FALSE;
  const int DEATHSTROKE_MOVE = 15;
  TBaseWeapon* tw;
  TThing* ob;
  spellNumT sktype = SKILL_DEATHSTROKE;
  affectedData aff1, aff2;

  // Ensure this isn't a peaceful room
  if (caster->checkPeaceful(
        "You feel too peaceful to contemplate violence.\n\r"))
    return FALSE;

  // Check for lockout from recently using this skill
  if (caster->affectedBySpell(SKILL_DEATHSTROKE)) {
    caster->sendTo(
      "You are still recovering from your last deathstroke and cannot use this "
      "ability again at this time.\n\r");
    return FALSE;
  }

  // Make sure the player has enough vitality to use the skill
  if (caster->getMove() < DEATHSTROKE_MOVE) {
    caster->sendTo("You don't have the vitality to make the move!\n\r");
    return FALSE;
  }

  // Prevent players from attacking themselves
  if (victim == caster) {
    caster->sendTo("Do you REALLY want to kill yourself?...\n\r");
    return FALSE;
  }

  // Avoid players attacking un-harmable victims
  if (caster->noHarmCheck(victim))
    return FALSE;

  // Limit players from using this while mounted
  if (caster->riding) {
    caster->sendTo("You can't deathstroke while mounted!\n\r");
    return FALSE;
  }

  // Ensure the player has a weapon equipped
  if (!caster->heldInPrimHand() ||
      !(tw = dynamic_cast<TBaseWeapon*>(caster->heldInPrimHand()))) {
    caster->sendTo(
      "You need to hold a weapon in your primary hand to make this a "
      "success.\n\r");
    return FALSE;
  }

  // Only consume vitality for mortals
  if (!(caster->isImmortal() || IS_SET(caster->specials.act, ACT_IMMORTAL)))
    caster->addToMove(-DEATHSTROKE_MOVE);

  int skillLevel = caster->getSkillLevel(SKILL_DEATHSTROKE);
  int skillValue = caster->getSkillValue(SKILL_DEATHSTROKE);
  int successfulHit = caster->specialAttack(victim, SKILL_DEATHSTROKE);
  int successfulSkill = caster->bSuccess(skillValue, SKILL_DEATHSTROKE);
  int dam = caster->getSkillDam(victim, SKILL_DEATHSTROKE, skillLevel,
    caster->getAdvLearning(SKILL_DEATHSTROKE));

  // Assessing an armor penalty regardless of success - this penalty greatly
  // scales with level to ensure that players are not overly penalized early on
  aff1.type = SKILL_DEATHSTROKE;
  aff1.duration = Pulse::UPDATES_PER_MUDHOUR / 3;
  aff1.location = APPLY_ARMOR;
  aff1.modifier =
    (caster->GetMaxLevel() * caster->GetMaxLevel()) / 5 - skillLevel;
  aff1.bitvector = 0;

  // Success use case
  if (!victim->awake() || (successfulHit && successfulSkill &&
                            successfulHit != GUARANTEED_FAILURE)) {
    wasSuccess = TRUE;

    // Handling the use-case where getSkillDam() returns 0
    if (!dam) {
      act("$n's attempt at $N's vital area is ineffective.", FALSE, caster, 0,
        victim, TO_NOTVICT);
      act("You hit to $N's vital area is ineffective.", FALSE, caster, 0,
        victim, TO_CHAR);
      act("$n attempts to hit your vital area, but the blow is ineffective.",
        FALSE, caster, 0, victim, TO_VICT);

    } else {
      act("$n hits $N in $S vital organs!", FALSE, caster, 0, victim,
        TO_NOTVICT);
      act("You hit $N in $S vital organs!", FALSE, caster, 0, victim, TO_CHAR);
      act("$n hits you in your vital organs!", FALSE, caster, 0, victim,
        TO_VICT);

      // You successfully landed your deathstroke - assessing a bonus
      aff2.type = SKILL_DEATHSTROKE;
      aff2.duration = Pulse::UPDATES_PER_MUDHOUR / 3;
      aff2.location = APPLY_HITROLL;
      aff2.modifier = 3;
      aff2.bitvector = 0;
    }

    ob = caster->heldInPrimHand();
    if (ob && ob->isBluntWeapon())
      sktype = DAMAGE_CAVED_SKULL;
    if (caster->reconcileDamage(victim, dam, sktype) == -1)
      return DELETE_VICT;
  }
  // Failure use case
  else {
    if (victim->getPosition() > POSITION_DEAD) {
      act("$n's attempt at $N's vital area falls far short of hitting.", FALSE,
        caster, 0, victim, TO_NOTVICT);
      act("You fail to hit $N's vital area.", FALSE, caster, 0, victim,
        TO_CHAR);
      act("$n attempts to hit your vital area, but fails miserably.", FALSE,
        caster, 0, victim, TO_VICT);
    }

    if (caster->reconcileDamage(victim, 0, sktype) == -1)
      return DELETE_VICT;
  }
  // Applying the debuff and, on success, the buff
  if (wasSuccess) {
    caster->affectJoin(caster, &aff1, AVG_DUR_YES, AVG_EFF_YES, FALSE);
    caster->affectJoin(caster, &aff2, AVG_DUR_YES, AVG_EFF_YES, FALSE);
  } else {
    caster->affectTo(&aff1, -1);
  }

  // Victim gets a chance at counter attack
  int victimSkillValue = victim->getSkillValue(SKILL_DEATHSTROKE);
  int victimSkillModifier = (victim->GetMaxLevel() - caster->GetMaxLevel()) / 2;
  victimSkillModifier += victim->getDexReaction() * 5;
  victimSkillModifier -= caster->getAgiReaction() * 5;
  victimSkillModifier -= skillLevel / 2;

  if (victim->specialAttack(caster, SKILL_DEATHSTROKE) != GUARANTEED_FAILURE &&
      ((victimSkillValue + victimSkillModifier) > 0) &&
      victim->bSuccess(victimSkillValue + victimSkillModifier,
        SKILL_DEATHSTROKE)) {
    // Successful counter attack
    if (victim->getPosition() > POSITION_STUNNED) {
      dam = victim->GetMaxLevel() * 2;
      dam += victim->plotStat(STAT_CURRENT, STAT_STR, 0, 6, 3);
      dam = ::number(victim->GetMaxLevel() / 3, dam);
      dam = victim->getActualDamage(victim, NULL, dam, SKILL_DEATHSTROKE);
      dam /= 3;

      act("$N exploits $n's vulnerable state with a quick hit to the heart.",
        FALSE, caster, 0, victim, TO_NOTVICT);
      act(
        "While you are vulnerable, $N strikes you in the center of your torso.",
        FALSE, caster, 0, victim, TO_CHAR);
      act("You take advantage of $n's vulnerability for a cheap shot!", FALSE,
        caster, 0, victim, TO_VICT);

      ob = victim->heldInPrimHand();
      if (ob && ob->isBluntWeapon())
        sktype = DAMAGE_CAVED_SKULL;
      if (victim->reconcileDamage(caster, dam, sktype) == -1)
        return DELETE_THIS;
    }
  }

  return TRUE;
}

int TBeing::doDeathstroke(const char* argument, TBeing* vict) {
  int rc;
  TBeing* victim;
  char v_name[MAX_INPUT_LENGTH];

  if (checkBusy()) {
    return FALSE;
  }

  // Ensure player even knows the skill before continuing
  if (!doesKnowSkill(SKILL_DEATHSTROKE)) {
    sendTo("You know nothing about deathblows.\n\r");
    return FALSE;
  }

  strcpy(v_name, argument);

  if (!(victim = vict)) {
    if (!(victim = get_char_room_vis(this, v_name))) {
      if (!(victim = fight())) {
        sendTo("Deathstroke whom?\n\r");
        return FALSE;
      }
    }
  }
  if (!sameRoom(*victim)) {
    sendTo("That person isn't around.\n\r");
    return FALSE;
  }

  if (victim->isImmortal() || IS_SET(victim->specials.act, ACT_IMMORTAL)) {
    sendTo("You cannot attack an immortal..\n\r");
    return FALSE;
  }

  rc = deathstroke(this, victim);

  if (rc)
    addSkillLag(SKILL_DEATHSTROKE, rc);

  if (IS_SET_DELETE(rc, DELETE_VICT)) {
    if (vict)
      return rc;
    delete victim;
    victim = NULL;
    REM_DELETE(rc, DELETE_VICT);
  }

  if (IS_SET_DELETE(rc, DELETE_THIS)) {
    return DELETE_THIS;
  }

  return rc;
}
