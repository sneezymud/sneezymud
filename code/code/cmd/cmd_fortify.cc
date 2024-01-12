#include "handler.h"
#include "being.h"
#include "combat.h"
#include "obj_base_clothing.h"

int TBeing::doFortify() {
  affectedData aff1, aff2;
  // Check if caster at least knows the skill they're attempting to use
  if (!doesKnowSkill(SKILL_FORTIFY)) {
    sendTo("You know nothing about advanced defensive maneuvers.\n\r");
    return false;
  }

  // Check for lockout
  if (affectedBySpell(SKILL_FORTIFY)) {
    sendTo("You are still recovering from your last shield wall.\n\r");
    return false;
  }

  // Ensure player is using a shield
  auto* obj = dynamic_cast<TBaseClothing*>(heldInSecHand());
  if (!obj || !obj->isShield()) {
    sendTo(
      "You cannot execute this defensive maneuver without a shield "
      "equipped!\n\r");
    return false;
  }

  int castLevel = getSkillLevel(SKILL_FORTIFY);
  int skillLevel = getSkillValue(SKILL_FORTIFY);
  int successfulSkill = bSuccess(skillLevel, SKILL_FORTIFY);

  // Apply a lockout buff on the caster
  aff1.type = SKILL_FORTIFY;
  aff1.duration = Pulse::UPDATES_PER_MUDHOUR;
  aff1.modifier = 0;
  aff1.bitvector = 0;
  affectTo(&aff1, -1);

  // Skill failure
  if (!successfulSkill) {
    act(
      "You attempt to fortify your defenses but fail to execute the maneuver.",
      false, this, nullptr, nullptr, TO_CHAR);
    act("$n attempts a defensive maneuver but fails.", false, this, nullptr, nullptr,
      TO_ROOM);

    return false;
  }

  // Skill success
  act("You sink in behind your shield and defend against incoming attacks!",
    false, this, nullptr, nullptr, TO_CHAR);
  act("$n raises $s shield and strikes a defensive posture!", false, this, nullptr,
    nullptr, TO_ROOM);

  // Damage resistance
  aff2.type = AFFECT_FORTIFY;
  aff2.duration = Pulse::TICK * 4;
  aff2.location = APPLY_PROTECTION;
  aff2.modifier = 25 + castLevel;
  aff2.bitvector = 0;
  affectTo(&aff2, -1);

  return true;
}
