//////////////////////////////////////////////////////////////////////////
//
// SneezyMUD - All rights reserved, SneezyMUD Coding Team
//
// $Log: cmd_visible.cc,v $
// Revision 1.1  1999/09/12 17:24:04  sneezy
// Initial revision
//
//
//////////////////////////////////////////////////////////////////////////


/*****************************************************************************

  SneezyMUD++ - All rights reserved, SneezyMUD Coding Team.

  "cmd_visible.cc"
  All functions and routines related to mortal invisibility control.

  Created 8/05/99 - Lapsos(William A. Perrotto III)

******************************************************************************/

#include "stdsneezy.h"

void TBeing::doVisible(const char *, bool)
{
  sendTo("Silly monster.  Why do you need to go visible??\n\r");
}

void TPerson::doVisible(const char *, bool tSilent)
{
  if (getPosition() < POSITION_STANDING) {
    sendTo("You must be standing to do this.\n\r");
    return;
  }

  /*
  if (task || spelltask || fight()) {
    sendTo("You are a little busy to be doing this.\n\r");
    return;
  }
  */

  affectedData *tAffect;
  double        tDuration;

  if (!isAffected(AFF_INVISIBLE)) {
    if (!tSilent)
      sendTo("You are not invisble to begin with.\n\r");
    return;
  }

  REMOVE_BIT(specials.affectedBy, AFF_INVISIBLE);

  if (!tSilent) {
    sendTo("You focus and slowly become visible.\n\r");
    act("$n slowly becomes visible.",
        FALSE, this, NULL, NULL, TO_ROOM);
  }

  // The affect of 'holding back' the magic causes the time left
  // to be drained.
  for (tAffect = affected; tAffect; tAffect = tAffect->next)
    if (tAffect->type == SPELL_INVISIBILITY) {
      tDuration = (double) tAffect->duration * .025;
      tDuration = (double) tAffect->duration - tDuration;
      tAffect->duration  = (int) tDuration;
      tAffect->bitvector = 0;
      return;
    }
}

void TBeing::doMortalInvis(const char *)
{
  sendTo("Silly monster.  You can not control this.\n\r");
}

void TPerson::doMortalInvis(const char *)
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

  if (!affectedBySpell(SPELL_INVISIBILITY)) {
    sendTo("I'm afraid you can not do this.\n\r");
    return;
  }

  sendTo("You focus and slowly vanish.\n\r");
  act("$n slowly fades away.",
      FALSE, this, NULL, NULL, TO_ROOM);
  SET_BIT(specials.affectedBy, AFF_INVISIBLE);

  // The affect of 're-releasing' the magic causes the time left
  // to be drained.
  for (tAffect = affected; tAffect; tAffect = tAffect->next)
    if (tAffect->type == SPELL_INVISIBILITY) {
      tDuration = (double) tAffect->duration * .025;
      tDuration = (double) tAffect->duration - tDuration;
      tAffect->duration  = (int) tDuration;
      tAffect->bitvector = AFF_INVISIBLE;
      return;
    }
}
