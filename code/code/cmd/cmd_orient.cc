#include "handler.h"
#include "being.h"
#include "combat.h"
#include "obj_base_weapon.h"
#include "monster.h"

int TBeing::doOrient() {
  const int ORIENT_MOVE = 10;

  if (checkBusy()) {
    return false;
  }

  if (!doesKnowSkill(SKILL_ORIENT)) {
    sendTo("You know nothing about preparing your mount for a charge.\n\r");
    return false;
  }

  TMonster* mount = dynamic_cast<TMonster*>(this->riding);

  if (!mount || (getPosition() != POSITION_MOUNTED)) {
    sendTo(
      "You can't prepare your mount for a charge if you aren't mounted!\n\r");
    return false;
  }

  if (mount->horseMaster() != this) {
    act("You are not in control of $p and can't prepare it for a charge.",
      false, this, mount, nullptr, TO_CHAR);
    return false;
  }

  if (!mount->hasLegs()) {
    act("You can't prepare a legless $o for a charge!", false, this, mount,
      nullptr, TO_CHAR);
    return false;
  }

  if (mount->eitherLegHurt()) {
    act("Your $o's injury prevents you from preparing it for a charge!", false,
      this, mount, nullptr, TO_CHAR);
    return false;
  }

  if (checkPeaceful("You feel too peaceful to contemplate violence.\n\r"))
    return false;

  // Adding a lockout
  if (affectedBySpell(SKILL_ORIENT)) {
    sendTo("You cannot prepare your mount for a charge again so soon.\n\r");
    return false;
  }

  if (getMove() < ORIENT_MOVE) {
    sendTo("You don't have the vitality!\n\r");
    return false;
  }

  if (!(isImmortal() || IS_SET(specials.act, ACT_IMMORTAL)))
    addToMove(-ORIENT_MOVE);

  affectedData aff1;
  aff1.type = SKILL_ORIENT;
  aff1.duration = Pulse::UPDATES_PER_MUDHOUR;
  aff1.bitvector = 0;
  affectTo(&aff1, -1);

  if (!bSuccess(getSkillValue(SKILL_ORIENT), SKILL_ORIENT)) {
    act("You fail your attempt to prepare your mount for a charge!", false,
      this, nullptr, nullptr, TO_CHAR);
    act("$n attempts to prepare for a charge but is unsuccessful.", false, this,
      nullptr, nullptr, TO_ROOM);
    return false;
  }

  act("You prepare your mount for a devastating charge.", false, this, nullptr,
    nullptr, TO_CHAR);
  act("$n prepares $s his mount for a devastating charge!", false, this, nullptr,
    nullptr, TO_ROOM);

  // Set the flag that we will later check for to trigger an attack
  SET_BIT(specials.affectedBy, AFF_ORIENT);

  return true;
}
