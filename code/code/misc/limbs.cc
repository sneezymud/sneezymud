//////////////////////////////////////////////////////////////////////////
//
// SneezyMUD - All rights reserved, SneezyMUD Coding Team
//
//////////////////////////////////////////////////////////////////////////


// limbs.cc
//
//  Time for some more magic.

#include "being.h"
#include <cmath>

bool VITAL_PART(wearSlotT pos)
{
  return ((pos == WEAR_HEAD) || (pos == WEAR_BODY) ||
          (pos == WEAR_BACK) || (pos == WEAR_NECK));
}

wearSlotT pickRandomLimb(bool)
{
  int num = ::number(MIN_WEAR, MAX_WEAR-1);

  return wearSlotT(num);
}

int TBeing::hurtLimb(unsigned int dam, wearSlotT part_hit)
{
  unsigned int limHlt = getCurLimbHealth(part_hit);
  sstring buf;

  if (limHlt > 0) {
    addCurLimbHealth(part_hit, -std::min(dam, limHlt));
    if (getCurLimbHealth(part_hit) <= 0) {
      sendTo(COLOR_BASIC, format("%sYour %s has become totally useless!%s\n\r") %
	     red() % describeBodySlot(part_hit) % norm());
      buf=format("$n's %s has become completely useless!") %
	describeBodySlot(part_hit);
      act(buf, TRUE, this, NULL, NULL, TO_ROOM, ANSI_ORANGE);
      addToLimbFlags(part_hit, PART_USELESS);

      int rc = flightCheck();
      if (IS_SET_DELETE(rc, DELETE_THIS))
        return DELETE_THIS;
    }
    return TRUE;
  }
  return FALSE;
}
