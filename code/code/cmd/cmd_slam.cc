#include "handler.h"
#include "being.h"
#include "combat.h"
#include "obj_base_weapon.h"

int TBeing::doSlam(const char* argument, TBeing* vict) {
  int rc;
  TBeing* victim;
  const int SLAM_MOVE = 3;

  if (checkBusy()) {
    return false;
  }

  // Ensure player even knows the skill before continuing
  if (!doesKnowSkill(SKILL_SLAM)) {
    sendTo(
      "You wouldn't even know where to begin in executing that maneuver.\n\r");
    return false;
  }

  if (!(victim = vict)) {
    if (!(victim = get_char_room_vis(this, argument))) {
      if (!(victim = fight())) {
        sendTo("Hit whom?\n\r");
        return false;
      }
    }
  }
  if (!sameRoom(*victim)) {
    sendTo("That person isn't around.\n\r");
    return false;
  }

  auto* weapon = dynamic_cast<TBaseWeapon*>(heldInPrimHand());

  // Ensure this isn't a peaceful room
  if (checkPeaceful("You feel too peaceful to contemplate violence.\n\r"))
    return false;

  // Make sure the player has enough vitality to use the skill
  if (getMove() < SLAM_MOVE) {
    sendTo("You don't have the vitality to make the move!\n\r");
    return false;
  }

  // Prevent players from attacking immortals
  if (victim->isImmortal() || IS_SET(victim->specials.act, ACT_IMMORTAL)) {
    sendTo("Attacking an immortal would be a bad idea.\n\r");
    return false;
  }

  // Prevent players from attacking themselves
  if (victim == this) {
    sendTo("Do you REALLY want to kill yourself?...\n\r");
    return false;
  }

  // Avoid players attacking un-harmable victims
  if (noHarmCheck(victim))
    return false;

  // Limit players from using this while mounted
  if (riding) {
    sendTo("You can't perform that attack while mounted!\n\r");
    return false;
  }

  // Ensure the player has a weapon equipped
  if (!weapon) {
    sendTo(
      "You need to hold a weapon in your primary hand to make this a "
      "success.\n\r");
    return false;
  }

  // Only consume vitality for mortals
  if (!(isImmortal() || IS_SET(specials.act, ACT_IMMORTAL)))
    addToMove(-SLAM_MOVE);

  int skillValue = getSkillValue(SKILL_SLAM);
  int successfulHit = specialAttack(victim, SKILL_SLAM);
  int successfulSkill = bSuccess(skillValue, SKILL_SLAM);

  // Success use case
  if (!victim->awake() || (successfulHit && successfulSkill &&
                            successfulHit != GUARANTEED_FAILURE)) {
    rc = slamSuccess(victim);
  }
  // Fail use case
  else {
    rc = slamFail(victim);
  }

  if (rc)
    addSkillLag(SKILL_SLAM, rc);

  if (IS_SET_DELETE(rc, DELETE_VICT)) {
    if (vict)
      return rc;
    delete victim;
    victim = nullptr;
    REM_DELETE(rc, DELETE_VICT);
  }

  if (IS_SET_DELETE(rc, DELETE_THIS)) {
    return DELETE_THIS;
  }

  return rc;
}

int TBeing::slamSuccess(TBeing* victim) {
  int skillLevel = getSkillLevel(SKILL_SLAM);
  int dam =
    getSkillDam(victim, SKILL_SLAM, skillLevel, getAdvLearning(SKILL_SLAM));
  auto* weapon = dynamic_cast<TBaseWeapon*>(heldInPrimHand());

  // Scaling damage here due to the limitations of getSkillDam and how it treats
  // skills learned at low levels This formula is designed to allow the damage
  // to scale up to 0.75% of max hp to have some effectiveness against high
  // level opponents, while dealing a respectable amount of damage to lower
  // level enemies
  static const std::map<int, float> scalingDamageConstants = {{10, 0.15},
    {20, 0.08}, {30, 0.05}, {40, 0.03}, {50, 0.02}};

  // Default value for higher level enemies
  float scalingConstant = 0.0075;

  // For enemies level 1-50, retrieving
  for (const auto& damageConstant : scalingDamageConstants) {
    if (victim->GetMaxLevel() <= damageConstant.first) {
      scalingConstant = damageConstant.second;
      break;
    }
  }

  // Apply the scaling constant
  dam = max((int)(victim->hitLimit() * scalingConstant), dam);

  // Send description text to players in the room
  if (getCombatMode() == ATTACK_BERSERK) {
    dam *= 2;
    act(
      "$n slams $N with $s weapon in a <R>berserk fury<z>, inflicting "
      "considerable damage!",
      FALSE, this, 0, victim, TO_NOTVICT);
    act(
      "You slam your weapon into $N in a <R>berserk fury<z>, inflicting "
      "considerable damage!",
      FALSE, this, 0, victim, TO_CHAR);
    act("$n slams $s weapon into you with a <r>berserker's <z>zeal!", FALSE,
      this, 0, victim, TO_VICT);

  } else {
    act("$n slams $N with $s weapon, inflicting considerable damage!", FALSE,
      this, 0, victim, TO_NOTVICT);
    act("You slam your weapon into $N, inflicting considerable damage!", FALSE,
      this, 0, victim, TO_CHAR);
    act("$n slams $s weapon into you!", FALSE, this, 0, victim, TO_VICT);
  }

  // Special use-case for blunt weapons
  // Determine damage type
  spellNumT damageType = DAMAGE_NORMAL;

  if (weapon->isBluntWeapon())
    damageType = DAMAGE_CAVED_SKULL;
  else if (weapon->isPierceWeapon())
    damageType = DAMAGE_IMPALE;
  else if (weapon->isSlashWeapon())
    damageType = DAMAGE_HACKED;
  // Reconcile damage
  if (reconcileDamage(victim, dam, damageType) == -1)
    return DELETE_VICT;

  return true;
}

int TBeing::slamFail(TBeing* victim) {
  if (victim->getPosition() > POSITION_DEAD) {
    act(
      "$n's attempt at slamming $N's with $s his weapon fails to make contact.",
      FALSE, this, 0, victim, TO_NOTVICT);
    act("Your attempt at slamming $N with your weapon fails to make contact.",
      FALSE, this, 0, victim, TO_CHAR);
    act("$n attempts to slam you with $s weapon but comes up short.", FALSE,
      this, 0, victim, TO_VICT);
  }

  if (reconcileDamage(victim, 0, SKILL_SLAM) == -1)
    return DELETE_VICT;

  return true;
}
