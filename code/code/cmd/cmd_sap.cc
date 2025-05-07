#include "enum.h"
#include "handler.h"
#include "being.h"
#include "combat.h"
#include "obj_base_weapon.h"
#include "obj_general_weapon.h"
#include "monster.h"

int TBeing::doSap(const char* argument, TBeing* vict) {
  int rc;
  TBeing* victim;
  const int SAP_MOVE = 3;

  if (checkBusy()) {
    return false;
  }

  // Ensure player even knows the skill before continuing
  if (!doesKnowSkill(SKILL_SAP)) {
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

  auto* weapon = dynamic_cast<TGenWeapon*>(heldInPrimHand());

  // Ensure this isn't a peaceful room
  if (checkPeaceful("You feel too peaceful to contemplate violence.\n\r"))
    return false;

  // Make sure the player has enough vitality to use the skill
  if (getMove() < SAP_MOVE) {
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

  if (victim->isFlying() && !this->isFlying()) {
    sendTo("You can't sap a flying target unless you are also flying!\n\r");
    return false;
  }

  // Ensure the player has a weapon equipped
  if (!weapon) {
    sendTo(
      "You need to hold a weapon in your primary hand to make this a "
      "success.\n\r");
    return false;
  }

  if (!weapon->canSap()) {
    act("You can't use a $o to sap.", false, this, weapon, NULL, TO_CHAR);
    return FALSE;
  }

  // Only consume vitality for mortals
  if (!(isImmortal() || IS_SET(specials.act, ACT_IMMORTAL)))
    addToMove(-SAP_MOVE);

  int modifier = 0;

  // Add stealth modifiers similar to backstab
  modifier -= noise(this) / 20;
  modifier += visibility() / 15;

  if (isAffected(AFF_SNEAK))
    modifier += 5;
  if (isAffected(AFF_HIDE))
    modifier += 5;
  if (victim->fight() && !fight())
    modifier += 5;
  if (makesNoise() && victim->awake()) {
    modifier -= 10;

    act(
      "$n's armor makes too much noise, and $N is alerted to $n's sap attempt.",
      FALSE, this, 0, victim, TO_NOTVICT);
    act("You make too much noise, and $N hears you coming.", FALSE, this, 0,
      victim, TO_CHAR);
    act(
      "You hear $n's armor, and quickly attempt to dodge $s attempt to sap "
      "you.",
      FALSE, this, 0, victim, TO_VICT);
  }

  // Add awareness check like backstab
  if (victim->awake() && victim->canSee(this) && !victim->isPc() &&
      dynamic_cast<TMonster*>(victim)->isSusp()) {
    modifier -= 10;

    act("$E is able to see you and notices you coming at the last moment.",
      FALSE, this, 0, victim, TO_CHAR);
    act("You sense $m coming as $n attempts to sap you.", FALSE, this, 0,
      victim, TO_VICT);
  }

  int skillValue = getSkillValue(SKILL_SAP);
  // Pass the modifier to specialAttack
  int successfulHit = specialAttack(victim, SKILL_SAP, modifier);
  int successfulSkill = bSuccess(skillValue, SKILL_SAP);

  // Success use case
  if (!victim->awake() || (successfulHit && successfulSkill &&
                            successfulHit != GUARANTEED_FAILURE)) {
    rc = sapSuccess(victim);
  }
  // Fail use case
  else {
    rc = sapFail(victim);
  }

  if (rc)
    addSkillLag(SKILL_SAP, rc);

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

int TBeing::sapSuccess(TBeing* victim) {
  int skillLevel = getSkillLevel(SKILL_SAP);
  int dam =
    getSkillDam(victim, SKILL_SAP, skillLevel, getAdvLearning(SKILL_SAP));
  auto* weapon = dynamic_cast<TGenWeapon*>(heldInPrimHand());

  // Define the possible limbs and select one
  static const wearSlotT upperLimbs[] = {WEAR_ARM_R, WEAR_ARM_L, WEAR_WRIST_R,
    WEAR_WRIST_L, WEAR_HAND_R, WEAR_HAND_L, WEAR_FINGER_R, WEAR_FINGER_L,
    WEAR_BACK};

  wearSlotT limb;
  do {
    limb = upperLimbs[::number(0, 8)];  // 9 slots (0-8)
  } while (!victim->hasPart(limb));

  sstring buf = format("$n whaps $N with $s $o, rattling $S %s!") %
                victim->describeBodySlot(limb);
  act(buf, FALSE, this, weapon, victim, TO_NOTVICT);

  buf = format("You whap $N with your $o, rattling $S %s!") %
        victim->describeBodySlot(limb);
  act(buf, FALSE, this, weapon, victim, TO_CHAR);

  buf = format("$n whaps you with $s $o, rattling your %s!") %
        victim->describeBodySlot(limb);
  act(buf, FALSE, this, weapon, victim, TO_VICT);

  int limbDam = dam / 2;
  if (victim->isBruised(limb)) {
    limbDam = limbDam * 1.5;
  }

  if (weapon->isObjStat(ITEM_SPIKED)) {
    // Spiked weapons do 25% more damage to limb
    limbDam = limbDam * 1.25;
    spikesHit(victim, this, weapon, limb);
  }

  if (this->getPosition() >= POSITION_STANDING || this->isFlying()) {
    if (victim->getPosition() > POSITION_SITTING) {
      // Both are in active positions - apply height difference
      double heightDiff =
        (static_cast<double>(this->getHeight()) - victim->getHeight()) /
        (95.0 - 29.0);
      double multiplier =
        1.0 + (heightDiff * 0.75);  // 0.75 at -1.0, 1.5 at 1.0
      limbDam = static_cast<int>(limbDam * multiplier);
    } else {
      // Victim is sitting or worse - apply max bonus
      limbDam = static_cast<int>(limbDam * 1.5);
    }
  }

  if (weapon->isMaul()) {
    // Mauls do 50% more damage to limb
    limbDam = limbDam * 1.5;
  }

  int rc = victim->hurtLimb(limbDam, limb);
  if (IS_SET_DELETE(rc, DELETE_THIS)) {
    return DELETE_VICT;
  }

  if (!victim->isTough()) {
    rawBruise(limb, 250, SILENT_YES, CHECK_IMMUNITY_NO);
  }

  if (limbDam * 2 > victim->getCurLimbHealth(limb) && !victim->isTough()) {
    // Paralyze the limb
    victim->addToLimbFlags(limb, PART_PARALYZED);

    // Set up affect to remove paralysis after 2 rounds
    affectedData aff;
    aff.type = AFFECT_SKILL_ATTEMPT;
    aff.level = getSkillLevel(SKILL_SAP);
    aff.duration = 2;           // 2 rounds
    aff.location = APPLY_NONE;  // Use APPLY_NONE for the affect
    aff.modifier = SKILL_SAP;
    aff.modifier2 = limb;  // Store limb information in modifier2
    aff.bitvector = 0;
    victim->affectTo(&aff);
    victim->dropWeapon(limb);
    buf = format("Your $p smashes into $s %s, making it go numb!") %
          victim->describeBodySlot(limb);
    act(buf, false, this, weapon, victim, TO_CHAR);

    buf = format("$n's $p smashes into $s %s, making it go numb!") %
          victim->describeBodySlot(limb);
    act(buf, false, this, weapon, victim, TO_NOTVICT);

    buf = format("$n's $p smashes into your %s, making it go numb!") %
          victim->describeBodySlot(limb);
    act(buf, false, this, weapon, victim, TO_VICT);
  }

  rc = reconcileDamage(victim, dam, SKILL_SAP);
  if (rc == -1) {
    return DELETE_VICT;
  }

  return true;
}

int TBeing::sapFail(TBeing* victim) {
  auto* weapon = dynamic_cast<TGenWeapon*>(heldInPrimHand());

  if (victim->getPosition() > POSITION_DEAD) {
    act("$n's attempt to sap $N with $s $p fails awkwardly.", FALSE, this,
      weapon, victim, TO_NOTVICT);
    act("Your attempt to sap $N with your $p fails awkwardly.", FALSE, this,
      weapon, victim, TO_CHAR);
    act("$n attempts to sap you with $s $p but fails!", FALSE, this, weapon,
      victim, TO_VICT);
  }

  if (reconcileDamage(victim, 0, SKILL_SAP) == -1)
    return DELETE_VICT;

  return true;
}
