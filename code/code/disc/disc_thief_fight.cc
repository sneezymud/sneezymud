#include "handler.h"
#include "extern.h"
#include "being.h"
#include "combat.h"

int TBeing::thiefDodge(TBeing* v, TThing* weapon, int* dam, int w_type,
  wearSlotT part_hit) {
  char buf[256], type[16];

  // presumes thief is in appropriate position for dodging already

  if (!v->doesKnowSkill(SKILL_DODGE_THIEF))
    return FALSE;

  w_type -= TYPE_HIT;

  // the higher amt is, the more things get dodged
  int amt = 45;

  if (::number(0, 999) >= amt)
    return FALSE;

  // check bSuccess after above check, so that we limit how often we
  // call the learnFrom stuff
  if (v->bSuccess(SKILL_DODGE_THIEF)) {
    *dam = 0;

    strcpy(type, "dodge");
    if (toggleInfo[TOG_TWINK]->toggle) {
      sprintf(buf, "You %s $n's %s at your %s.", type,
        attack_hit_text_twink[w_type].singular,
        v->describeBodySlot(part_hit).c_str());
    } else {
      sprintf(buf, "You %s $n's %s at your %s.", type,
        attack_hit_text[w_type].singular,
        v->describeBodySlot(part_hit).c_str());
    }
    act(buf, FALSE, this, 0, v, TO_VICT, ANSI_CYAN);
    if (toggleInfo[TOG_TWINK]->toggle) {
      sprintf(buf, "$N %ss your %s at $S %s.", type,
        attack_hit_text_twink[w_type].singular,
        v->describeBodySlot(part_hit).c_str());
    } else {
      sprintf(buf, "$N %ss your %s at $S %s.", type,
        attack_hit_text[w_type].singular,
        v->describeBodySlot(part_hit).c_str());
    }
    act(buf, FALSE, this, 0, v, TO_CHAR, ANSI_CYAN);
    if (toggleInfo[TOG_TWINK]->toggle) {
      sprintf(buf, "$N %ss $n's %s at $S %s.", type,
        attack_hit_text_twink[w_type].singular,
        v->describeBodySlot(part_hit).c_str());
    } else {
      sprintf(buf, "$N %ss $n's %s at $S %s.", type,
        attack_hit_text[w_type].singular,
        v->describeBodySlot(part_hit).c_str());
    }
    act(buf, TRUE, this, 0, v, TO_NOTVICT);

    return TRUE;
  }
  return FALSE;
}
