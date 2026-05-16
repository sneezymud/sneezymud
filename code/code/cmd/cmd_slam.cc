#include "handler.h"
#include "being.h"
#include "combat.h"
#include "obj_base_weapon.h"
#include "obj_general_weapon.h"

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

  // Empty hand is fine (unarmed slam). Reject only when something non-weapon
  // is held (light, scroll, instrument, etc.).
  if (auto* held = heldInPrimHand(); held && !weapon) {
    act("You can't slam with $p.", false, this, held, nullptr, TO_CHAR);
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

  // Resolve the implement: weapon if held, else worn glove/gauntlet, else
  // null (truly bare). Picks the impact-damage source slot and the noun shown
  // in the act messages.
  TThing* implement =
    weapon ? static_cast<TThing*>(weapon) : equipment[getPrimaryHand()];

  // $o resolves to the implement's first keyword ("sword", "gauntlet"); for
  // bare hands fall back to the race-appropriate body-part noun.
  sstring noun =
    implement ? sstring("$o") : describeBodySlot(getPrimaryHand());

  // Send description text to players in the room
  if (getCombatMode() == ATTACK_BERSERK) {
    dam *= 2;
    act(format("$n slams $N with $s %s in a <R>berserk fury<z>, inflicting "
               "considerable damage!") %
          noun,
      false, this, implement, victim, TO_NOTVICT);
    act(format("You slam your %s into $N in a <R>berserk fury<z>, inflicting "
               "considerable damage!") %
          noun,
      false, this, implement, victim, TO_CHAR);
    act(format("$n slams $s %s into you with a <r>berserker's <z>zeal!") %
          noun,
      false, this, implement, victim, TO_VICT);

  } else {
    act(format("$n slams $N with $s %s, inflicting considerable damage!") %
          noun,
      false, this, implement, victim, TO_NOTVICT);
    act(format("You slam your %s into $N, inflicting considerable damage!") %
          noun,
      false, this, implement, victim, TO_CHAR);
    act(format("$n slams $s %s into you!") % noun, false, this, implement,
      victim, TO_VICT);
  }

  // Determine damage type: weapon class for armed, generic for unarmed.
  spellNumT damageType = DAMAGE_NORMAL;
  if (weapon) {
    if (weapon->isBluntWeapon())
      damageType = DAMAGE_CAVED_SKULL;
    else if (weapon->isPierceWeapon())
      damageType = DAMAGE_IMPALE;
    else if (weapon->isSlashWeapon())
      damageType = DAMAGE_HACKED;
  }

  // Apply impact effects. impactSpec reads equipment[damSource] itself, so a
  // worn gauntlet contributes via the hand slot when unarmed.
  wearSlotT targetLimb = victim->getPartHit(this, true);
  wearSlotT damSource = weapon ? getPrimaryHold() : getPrimaryHand();
  dam += impactSpec(this, victim, damSource, targetLimb);

  // Reconcile damage
  if (reconcileDamage(victim, dam, damageType) == -1)
    return DELETE_VICT;

  return true;
}

int TBeing::slamFail(TBeing* victim) {
  if (victim->getPosition() > POSITION_DEAD) {
    auto* weapon = dynamic_cast<TBaseWeapon*>(heldInPrimHand());
    TThing* implement =
      weapon ? static_cast<TThing*>(weapon) : equipment[getPrimaryHand()];
    sstring noun =
      implement ? sstring("$o") : describeBodySlot(getPrimaryHand());

    act(
      format("$n's attempt at slamming $N with $s %s fails to make contact.") %
        noun,
      false, this, implement, victim, TO_NOTVICT);
    act(
      format("Your attempt at slamming $N with your %s fails to make contact.") %
        noun,
      false, this, implement, victim, TO_CHAR);
    act(format("$n attempts to slam you with $s %s but comes up short.") %
          noun,
      false, this, implement, victim, TO_VICT);
  }

  if (reconcileDamage(victim, 0, SKILL_SLAM) == -1)
    return DELETE_VICT;

  return true;
}
