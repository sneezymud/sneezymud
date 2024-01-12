#include "handler.h"
#include "being.h"
#include "combat.h"
#include "obj_base_weapon.h"

int TBeing::doFocusAttack(const char* argument, TBeing* vict) {
  const int FOCUS_ATTACK_MOVE = 2;
  TBeing* victim = nullptr;

  if (checkBusy()) {
    return false;
  }
  if (!doesKnowSkill(SKILL_FOCUS_ATTACK)) {
    sendTo("You know nothing about focused attacks.\n\r");
    return false;
  }

  if (checkPeaceful("You feel too peaceful to contemplate violence.\n\r"))
    return false;

  // Adding a lockout
  if (affectedBySpell(SKILL_FOCUS_ATTACK)) {
    sendTo(
      "You are still recovering from your last focused attack and cannot use "
      "this ability again at this time.\n\r");
    return false;
  }

  if (getMove() < FOCUS_ATTACK_MOVE) {
    sendTo("You don't have the vitality to make the move!\n\r");
    return false;
  }

  if (getCombatMode() == ATTACK_BERSERK) {
    sendTo("You are berserking! You aren't able to focus!\n\r ");
    return false;
  }

  if (!(isImmortal() || IS_SET(specials.act, ACT_IMMORTAL)))
    addToMove(-FOCUS_ATTACK_MOVE);

  if (!(victim = vict) && (!(victim = get_char_room_vis(this, argument))) &&
      (!(victim = fight()))) {
    sendTo("Who do you want to attack?\n\r");
    return false;
  }

  if (!sameRoom(*victim)) {
    sendTo("That person isn't around.\n\r");
    return false;
  }

  if (victim->isImmortal() || IS_SET(victim->specials.act, ACT_IMMORTAL)) {
    sendTo("You cannot attack an immortal.\n\r");
    return false;
  }

  if (!isImmortal()) {
    affectedData aff1;
    aff1.type = SKILL_FOCUS_ATTACK;
    aff1.duration = Pulse::UPDATES_PER_MUDHOUR / 2;
    aff1.bitvector = 0;
    affectTo(&aff1, -1);
  }

  int rc;
  if (!bSuccess(getSkillValue(SKILL_FOCUS_ATTACK), SKILL_FOCUS_ATTACK)) {
    rc = focusAttackFail(victim);

  } else {
    rc = focusAttackSuccess(victim);
  }

  return rc;
}

int TBeing::focusAttackSuccess(TBeing* victim) {
  // Hit happens instantly when skill is triggered by advanced berserking and
  // can't be used manually while berserk, so in that case suppress this message
  if (getCombatMode() != ATTACK_BERSERK) {
    act("You focus on identifying a weakness in your opponent's defense.",
      false, this, nullptr, nullptr, TO_CHAR);
    act("$n concentrates on $s opponent, focusing intensely!", false, this,
      nullptr, nullptr, TO_ROOM);
  }
  // Set the flag that we will later check for to trigger an attack
  SET_BIT(specials.affectedBy, AFF_FOCUS_ATTACK);

  // If the characters aren't currently fighting, initiate combat
  if (!fight()) {
    setCharFighting(victim);
    setVictFighting(victim);
    reconcileHurt(victim, 0.01);
  }

  return true;
}

int TBeing::focusAttackFail(TBeing*) {
  act("You attempt to perform a focused attack, but lose your concentration!",
    false, this, nullptr, nullptr, TO_CHAR);
  act("$n attempts to focus, but loses $s concentration.", false, this, nullptr,
    nullptr, TO_ROOM);

  return false;
}
