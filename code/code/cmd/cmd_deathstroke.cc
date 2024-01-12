#include "handler.h"
#include "being.h"
#include "combat.h"
#include "obj_base_weapon.h"

int TBeing::doDeathstroke(const char* argument, TBeing* vict) {
  int rc;
  TBeing* victim;
  char v_name[MAX_INPUT_LENGTH];

  if (checkBusy()) {
    return false;
  }

  // Ensure player even knows the skill before continuing
  if (!doesKnowSkill(SKILL_DEATHSTROKE)) {
    sendTo("You know nothing about deathblows.\n\r");
    return false;
  }

  strcpy(v_name, argument);

  if (!(victim = vict)) {
    if (!(victim = get_char_room_vis(this, v_name))) {
      if (!(victim = fight())) {
        sendTo("Deathstroke whom?\n\r");
        return false;
      }
    }
  }
  if (!sameRoom(*victim)) {
    sendTo("That person isn't around.\n\r");
    return false;
  }

  if (victim->isImmortal() || IS_SET(victim->specials.act, ACT_IMMORTAL)) {
    sendTo("You cannot attack an immortal..\n\r");
    return false;
  }

  bool wasSuccess = false;
  const int DEATHSTROKE_MOVE = 8;
  TBaseWeapon* tw;
  affectedData aff1, aff2;

  // Ensure this isn't a peaceful room
  if (checkPeaceful("You feel too peaceful to contemplate violence.\n\r"))
    return false;

  // Check for lockout from recently using this skill
  if (affectedBySpell(SKILL_DEATHSTROKE)) {
    sendTo(
      "You are still recovering from your last deathstroke and cannot use this "
      "ability again at this time.\n\r");
    return false;
  }

  // Make sure the player has enough vitality to use the skill
  if (getMove() < DEATHSTROKE_MOVE) {
    sendTo("You don't have the vitality to make the move!\n\r");
    return false;
  }

  // Prevent players from attacking themselves
  if (victim == this) {
    sendTo("Do you REALLY want to kill yourself?...\n\r");
    return false;
  }

  if (getCombatMode() == ATTACK_BERSERK) {
    sendTo(
      "You are berserking! You can't focus enough to deathstroke anyone!\n\r ");
    return false;
  }

  // Avoid players attacking un-harmable victims
  if (noHarmCheck(victim))
    return false;

  // Limit players from using this while mounted
  if (riding) {
    sendTo("You can't deathstroke while mounted!\n\r");
    return false;
  }

  // Ensure the player has a weapon equipped
  if (!heldInPrimHand() ||
      !(tw = dynamic_cast<TBaseWeapon*>(heldInPrimHand()))) {
    sendTo(
      "You need to hold a weapon in your primary hand to make this a "
      "success.\n\r");
    return false;
  }

  // Only consume vitality for mortals
  if (!(isImmortal() || IS_SET(specials.act, ACT_IMMORTAL)))
    addToMove(-DEATHSTROKE_MOVE);

  int skillLevel = getSkillLevel(SKILL_DEATHSTROKE);
  int skillValue = getSkillValue(SKILL_DEATHSTROKE);
  int successfulHit = specialAttack(victim, SKILL_DEATHSTROKE);
  int successfulSkill = bSuccess(skillValue, SKILL_DEATHSTROKE);

  // Assessing an armor penalty regardless of success - this penalty greatly
  // scales with level to ensure that players are not overly penalized early on
  aff1.type = SKILL_DEATHSTROKE;
  aff1.duration = Pulse::UPDATES_PER_MUDHOUR / 3;
  aff1.location = APPLY_ARMOR;
  aff1.modifier = (GetMaxLevel() * GetMaxLevel()) / 5 - skillLevel;
  aff1.bitvector = 0;

  // Success use case
  if (!victim->awake() || (successfulHit && successfulSkill &&
                            successfulHit != GUARANTEED_FAILURE)) {
    rc = deathstrokeSuccess(victim);
    wasSuccess = true;

    // You successfully landed your deathstroke - assessing a bonus
    aff2.type = SKILL_DEATHSTROKE;
    aff2.duration = Pulse::UPDATES_PER_MUDHOUR / 3;
    aff2.location = APPLY_HITROLL;
    aff2.modifier = 3;
    aff2.bitvector = 0;
  } else {
    rc = deathstrokeFail(victim);
  }
  // Applying the debuff and, on success, the buff
  if (wasSuccess) {
    affectJoin(this, &aff1, AVG_DUR_YES, AVG_EFF_YES, false);
    affectJoin(this, &aff2, AVG_DUR_YES, AVG_EFF_YES, false);
  } else {
    affectTo(&aff1, -1);
  }

  addSkillLag(SKILL_DEATHSTROKE, rc);

  // Delete victim if success/fail functions set the DELETE_VICT flag
  if (IS_SET_DELETE(rc, DELETE_VICT)) {
    if (vict)
      return rc;
    delete victim;
    victim = nullptr;
    REM_DELETE(rc, DELETE_VICT);
  } else {
    // Allow victim a chance to counterattack
    if (victim->doesKnowSkill(SKILL_DEATHSTROKE))
      rc = deathstrokeCounterattack(victim);
  }

  if (IS_SET_DELETE(rc, DELETE_THIS))
    return DELETE_THIS;

  return true;
}

int TBeing::deathstrokeSuccess(TBeing* victim) {
  TThing* ob;
  int skillLevel = getSkillLevel(SKILL_DEATHSTROKE);
  int dam = getSkillDam(victim, SKILL_DEATHSTROKE, skillLevel,
    getAdvLearning(SKILL_DEATHSTROKE));

  // Handling the use-case where dam is 0
  if (!dam) {
    act("$n's attempt at $N's vital area is ineffective.", false, this, 0,
      victim, TO_NOTVICT);
    act("You hit to $N's vital area is ineffective.", false, this, 0, victim,
      TO_CHAR);
    act("$n attempts to hit your vital area, but the blow is ineffective.",
      false, this, 0, victim, TO_VICT);

  } else {
    act("$n hits $N in $S vital organs!", false, this, 0, victim, TO_NOTVICT);
    act("You hit $N in $S vital organs!", false, this, 0, victim, TO_CHAR);
    act("$n hits you in your vital organs!", false, this, 0, victim, TO_VICT);
  }

  ob = heldInPrimHand();
  spellNumT sktype =
    (ob && ob->isBluntWeapon()) ? DAMAGE_CAVED_SKULL : SKILL_DEATHSTROKE;
  if (reconcileDamage(victim, dam, sktype) == -1)
    return DELETE_VICT;

  return true;
}

int TBeing::deathstrokeFail(TBeing* victim) {
  if (victim->getPosition() > POSITION_DEAD) {
    act("$n's attempt at $N's vital area falls far short of hitting.", false,
      this, 0, victim, TO_NOTVICT);
    act("You fail to hit $N's vital area.", false, this, 0, victim, TO_CHAR);
    act("$n attempts to hit your vital area, but fails miserably.", false, this,
      0, victim, TO_VICT);
  }

  if (reconcileDamage(victim, 0, SKILL_DEATHSTROKE) == -1)
    return DELETE_VICT;

  return true;
}

int TBeing::deathstrokeCounterattack(TBeing* victim) {
  int dam;

  // Victim gets a chance at counter attack
  int victimSkillValue = victim->getSkillValue(SKILL_DEATHSTROKE);
  int victimSkillModifier = (victim->GetMaxLevel() - GetMaxLevel()) / 2;
  victimSkillModifier += victim->getDexReaction() * 5;
  victimSkillModifier -= getAgiReaction() * 5;
  victimSkillModifier -= getSkillLevel(SKILL_DEATHSTROKE) / 2;

  if (victim->specialAttack(this, SKILL_DEATHSTROKE) != GUARANTEED_FAILURE &&
      ((victimSkillValue + victimSkillModifier) > 0) &&
      victim->bSuccess(victimSkillValue + victimSkillModifier,
        SKILL_DEATHSTROKE)) {
    // Successful counter attack
    if (victim->getPosition() > POSITION_STUNNED) {
      dam = victim->GetMaxLevel() * 2;
      dam += victim->plotStat(STAT_CURRENT, STAT_STR, 0, 6, 3);
      dam = ::number(victim->GetMaxLevel() / 3, dam);
      dam = victim->getActualDamage(victim, nullptr, dam, SKILL_DEATHSTROKE);
      dam /= 3;

      act("$N exploits $n's vulnerable state with a quick hit to the heart.",
        false, this, 0, victim, TO_NOTVICT);
      act(
        "While you are vulnerable, $N strikes you in the center of your torso.",
        false, this, 0, victim, TO_CHAR);
      act("You take advantage of $n's vulnerability for a cheap shot!", false,
        this, 0, victim, TO_VICT);

      TThing* ob = victim->heldInPrimHand();
      spellNumT sktype =
        (ob && ob->isBluntWeapon()) ? DAMAGE_CAVED_SKULL : SKILL_DEATHSTROKE;
      if (victim->reconcileDamage(this, dam, sktype) == -1)
        return DELETE_THIS;
    }
  }

  return true;
}
