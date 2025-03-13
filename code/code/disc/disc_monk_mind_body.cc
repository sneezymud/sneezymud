#include "disc_monk_mind_body.h"
#include "being.h"

int TBeing::doLeap(const sstring& arg) {
  int rc;

  if (checkBusy()) {
    return FALSE;
  }

  if (arg.empty()) {
    sendTo("Which way do you want to leap?\n\r");
    return FALSE;
  }

  if (fight()) {
    sendTo("You can't leap away while fighting!\n\r");
    return FALSE;
  }

  if (!doesKnowSkill(SKILL_CATLEAP)) {
    sendTo("You do not know the secrets of cat-like leaping.\n\r");
    return FALSE;
  }

  if (roomp->isFallSector()) {
    sendTo("There's no ground beneath you to leap off of here!\n\r");
    return FALSE;
  }

  if (getMove() < 15) {
    sendTo("You're too tired to be jumping around.\n\r");
    return FALSE;
  }

  dirTypeT dir = getDirFromChar(arg);
  if (dir == -1 || !exitDir(dir)) {
    sendTo("You can't go that way.\n\r");
    return FALSE;
  }

  bool was_flying = IS_SET(specials.affectedBy, AFF_FLYING);

  // make them fly
  SET_BIT(specials.affectedBy, AFF_FLYING);
  setPosition(POSITION_FLYING);

  act("You leap into the air!", FALSE, this, 0, 0, TO_CHAR);
  act("$n takes a great leap into the air!", FALSE, this, 0, 0, TO_ROOM);
  addToMove(-15);

  if (!bSuccess(SKILL_CATLEAP)) {
    act("You don't make it very far.", FALSE, this, 0, 0, TO_CHAR);
    act("$n doesn't make it very far.", FALSE, this, 0, 0, TO_ROOM);
    rc = crashLanding(POSITION_SITTING);
  } else {
    rc = doMove(getDirFromChar(arg));
  }

  if (!was_flying)
    REMOVE_BIT(specials.affectedBy, AFF_FLYING);
  return rc;
}

int TBeing::doDodge() {
  sendTo("Dodging is not yet supported in this fashion.\n\r");
  return 0;
}

int TBeing::monkDodge(TBeing* v, TThing* weapon, int* dam, int w_type,
  wearSlotT part_hit) {
  char buf[256], type[16];

  // presumes monk is in appropriate position for dodging already

  if (!v->doesKnowSkill(SKILL_JIRIN))
    return FALSE;

  // Balance notes: dodging is in some ways a replacement for Monk's
  // lack of AC
  // In theory, eq alone gets them hit 90% of time in fair fight
  // and "skills" is supposed to lower that to 78%
  // we've already passed through the hits() function when we get here
  // and monks have no intrinsic AC boost
  // So technically, we should be blocking 12/90 = 13.3% of damage
  w_type -= TYPE_HIT;

  // the higher amt is, the more things get blocked
  int amt = 100;

  if (::number(0, 999) >= amt)
    return FALSE;

  // check bSuccess after above check, so that we limit how often we
  // call the learnFrom stuff
  if (v->bSuccess(SKILL_JIRIN)) {
    *dam = 0;

    switch (::number(0, 2)) {
      case 0:
        strcpy(type, "dodge");
        break;
      case 1:
        strcpy(type, "block");
        break;
      case 2:
        strcpy(type, "deflect");
        break;
    }

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