#include "handler.h"
#include "being.h"
#include "combat.h"
#include "obj_base_weapon.h"

int TBeing::doRally() {
  if (!doesKnowSkill(SKILL_RALLY)) {
    sendTo("You know nothing about motivational speaking.\n\r");
    return FALSE;
  }

  if (affectedBySpell(SKILL_RALLY)) {
    sendTo("You are still recovering from your last rallying cry.\n\r");
    return FALSE;
  }

  int castLevel = getSkillLevel(SKILL_RALLY);
  int skillLevel = getSkillValue(SKILL_RALLY);
  int successfulSkill = bSuccess(skillLevel, SKILL_RALLY);
  int modifierValue = 35 + castLevel;

  // Skill failure
  if (!successfulSkill) {
    act("Your half-hearted battlecry fails to motivate your allies.", FALSE,
      this, NULL, NULL, TO_CHAR);
    act("$n attempts a rally battlecry but fails to motivate anyone.", FALSE,
      this, NULL, NULL, TO_ROOM);

    // Apply a lockout buff on the caster
    affectedData aff1;
    aff1.type = SKILL_RALLY;
    aff1.duration = Pulse::UPDATES_PER_MUDHOUR;
    aff1.modifier = 0;
    aff1.bitvector = 0;
    affectTo(&aff1, -1);

    return FALSE;
  }

  // Skill success
  act("You bellow a warcry, encouraging your allies to continue the battle!",
    FALSE, this, NULL, NULL, TO_CHAR);
  act(
    "$n bellows a motivational warcry, encouraging $s allies to continue the "
    "battle!",
    FALSE, this, NULL, NULL, TO_ROOM);

  // Max HP
  affectedData aff1;
  aff1.type = SKILL_RALLY;
  aff1.duration = Pulse::UPDATES_PER_MUDHOUR * 3;
  aff1.location = APPLY_HIT;
  aff1.modifier = modifierValue;
  aff1.bitvector = 0;

  // Max MOVE
  affectedData aff2;
  aff2.type = SKILL_RALLY;
  aff2.duration = Pulse::UPDATES_PER_MUDHOUR * 3;
  aff2.location = APPLY_MOVE;
  aff2.modifier = modifierValue;
  aff2.bitvector = 0;

  // Loop for each person in room
  bool found = false;
  for (TThing* thing : roomp->stuff) {
    auto* person = dynamic_cast<TBeing*>(thing);

    if (!person || !inGroup(*person))
      continue;

    person->sendTo(
      "You feel revitalized and ready to continue the battle!\n\r");
    person->affectJoin(this, &aff1, AVG_DUR_NO, AVG_EFF_YES, false);
    person->affectJoin(this, &aff2, AVG_DUR_NO, AVG_EFF_YES, false);
    person->addToMove(modifierValue);
    person->addToHit(modifierValue);

    if (person != this)
      found = true;
  }

  if (!found)
    sendTo("But, there's nobody in your group.\n\r");
  // end loop

  return TRUE;
}
