#include "being.h"
#include "handler.h"
#include "disc_deikhan_absolution.h"

int TBeing::doLayHands(const char* arg) {
  int amt;
  affectedData aff;
  char name_buf[MAX_INPUT_LENGTH + 1];
  TBeing* vict;

  if (!doesKnowSkill(SKILL_LAY_HANDS)) {
    sendTo("You know nothing about laying on of hands.\n\r");
    return FALSE;
  }
  if (checkForSkillAttempt(SKILL_LAY_HANDS)) {
    sendTo("You are unprepared to attempt to lay hands again.\n\r");
    return FALSE;
  }
  if (affectedBySpell(SKILL_LAY_HANDS)) {
    sendTo("You are unable to lay hands again at this time.\n\r");
    return FALSE;
  }
  one_argument(arg, name_buf, cElements(name_buf));

  if (!(vict = get_char_room_vis(this, name_buf))) {
    sendTo("No one here by that name.\n\r");
    return FALSE;
  }

  // Prevent back-to-back attempts
  aff.type = AFFECT_SKILL_ATTEMPT;
  aff.duration = 1.5 * Pulse::UPDATES_PER_MUDHOUR;
  aff.modifier = SKILL_LAY_HANDS;
  aff.location = APPLY_NONE;
  aff.bitvector = 0;
  affectTo(&aff, -1);

  if (vict == this) {
    sendTo("You attempt to lay hands on yourself.\n\r");
    act("$n tries to lay hands on $mself.", FALSE, this, NULL, NULL, TO_ROOM);
  } else {
    act("You attempt to lay hands on $N.", FALSE, this, NULL, vict, TO_CHAR);
    act("$n attempts to lay hands on you.", FALSE, this, NULL, vict, TO_VICT);
    act("$n attempts to lay hands on $N.", FALSE, this, NULL, vict, TO_NOTVICT);
  }
  amt = ::number(20, 200) + (5 * getClassLevel(CLASS_DEIKHAN));

  if (bSuccess(getSkillValue(SKILL_LAY_HANDS), getPerc(), SKILL_LAY_HANDS)) {
    LogDam(this, SKILL_LAY_HANDS, amt);

    if (this != vict) {
      act("A soft yellow light surrounds your hands as you touch $N.", FALSE,
        this, 0, vict, TO_CHAR);
      act("A soft yellow light surrounds $n's hands as $e touches you.", FALSE,
        this, 0, vict, TO_VICT);
      act("A soft yellow light surrounds $n's hands as $e touches $N.", FALSE,
        this, 0, vict, TO_NOTVICT);
    } else {
      act("A soft yellow light surrounds your hands as you touch yourself.",
        FALSE, this, 0, vict, TO_CHAR);
      act("A soft yellow light surrounds $n's hands as $e touches $mself.",
        FALSE, this, 0, vict, TO_ROOM);
    }
    vict->addToHit(amt);
    vict->setHit(std::min(vict->getHit(), (int)vict->hitLimit()));

    // success prevents from working for longer
    aff.type = SKILL_LAY_HANDS;
    aff.duration = 4 * Pulse::UPDATES_PER_MUDHOUR;
    aff.location = APPLY_NONE;
    aff.modifier = 0;
    aff.bitvector = 0;
    affectTo(&aff, -1);
  }
  reconcileHelp(vict, discArray[SKILL_LAY_HANDS]->alignMod);
  return TRUE;
}