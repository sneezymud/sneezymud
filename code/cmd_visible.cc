///////////////////////////////////
//
//  SneezyMUD++ - All rights reserved, SneezyMUD Coding Team.
//
//  "cmd_visible.cc"
//  All functions and routines related to mortal invisibility control.
//
///////////////////////////////////

#include "stdsneezy.h"

void TBeing::doVisible(const char *, bool)
{
  sendTo("Silly monster.  Why do you need to go visible??\n\r");
}

void TPerson::doVisible(const char *, bool tSilent)
{

  /*
  if (getPosition() < POSITION_STANDING) {
    sendTo("You must be standing to do this.\n\r");
    return;
  }
  */  // abusable


  /*
  if (task || spelltask || fight()) {
    sendTo("You are a little busy to be doing this.\n\r");
    return;
  }
  */

  affectedData *tAffect;
  double        tDuration;

  if (isAffected(AFF_SHADOW_WALK)) {
    sendTo("You are shadow walking and can not solidify.\n\r");
  }
  if (!isAffected(AFF_INVISIBLE)) {
    if (!tSilent)
      sendTo("You are not invisible to begin with.\n\r");

    return;
  }

  if (!isVampire() &&
      (!doesKnowSkill(SPELL_INVISIBILITY) && ::number(0, 10))) {
    sendTo("You fail to control the magic and lose the power of invisibility.\n\r");
    act("$n quickly becomes visible.",
        FALSE, this, NULL, NULL, TO_ROOM);

    affectFrom(SPELL_INVISIBILITY);

    if (IS_SET(specials.affectedBy, AFF_INVISIBLE))
      REMOVE_BIT(specials.affectedBy, AFF_INVISIBLE);

    return;
  } else if (!tSilent) {
    sendTo("You focus and slowly become visible.\n\r");
    act("$n slowly becomes visible.",
        FALSE, this, NULL, NULL, TO_ROOM);
  }

  if (IS_SET(specials.affectedBy, AFF_INVISIBLE))
    REMOVE_BIT(specials.affectedBy, AFF_INVISIBLE);

  // The affect of 'holding back' the magic causes the time left
  // to be drained.
  for (tAffect = affected; tAffect; tAffect = tAffect->next) {
    if (tAffect->type == SPELL_INVISIBILITY) {
      tDuration = (double) tAffect->duration * .025;
      tDuration = (double) tAffect->duration - tDuration;
      tAffect->duration  = (int) tDuration;
      tAffect->bitvector = 0;
      return;
    }
  }
}

void TBeing::doInvis(const char *)
{
  sendTo("Silly monster.  You can not control this.\n\r");
}

void TPerson::doInvis(const char *)
{
  if (task || spelltask || fight()) {
    sendTo("You are a little busy to be doing this.\n\r");
    return;
  }

  affectedData *tAffect;
  double        tDuration;

  if (isAffected(AFF_INVISIBLE)) {
    sendTo("Yes, you are invisible.  How astute of you to notice.\n\r");
    return;
  }

  if (!affectedBySpell(SPELL_INVISIBILITY) && !isVampire()) {
    sendTo("I'm afraid you can not do this.\n\r");
    return;
  }

  sendTo("You focus and slowly vanish.\n\r");
  act("$n slowly fades away.",
      FALSE, this, NULL, NULL, TO_ROOM);
  SET_BIT(specials.affectedBy, AFF_INVISIBLE);

  // The affect of 're-releasing' the magic causes the time left
  // to be drained.
  for (tAffect = affected; tAffect; tAffect = tAffect->next) {
    if (tAffect->type == SPELL_INVISIBILITY) {
      tDuration = (double) tAffect->duration * .025;
      tDuration = (double) tAffect->duration - tDuration;
      tAffect->duration  = (int) tDuration;
      tAffect->bitvector = AFF_INVISIBLE;
      return;
    }
  }
}
