//////////////////////////////////////////////////////////////////////////
//
// SneezyMUD - All rights reserved, SneezyMUD Coding Team
//
//////////////////////////////////////////////////////////////////////////


#include "being.h"
#include "room.h"

void TBeing::dropWeapon(wearSlotT slot)
{
  char buf[256];
  wearSlotT hand;

  if (((slot <= WEAR_FINGER_L) && (slot >= WEAR_ARM_R)) || (slot == HOLD_RIGHT) || (slot == HOLD_LEFT)) {
    if ((slot == WEAR_HAND_R) || (slot == WEAR_ARM_R) || (slot == WEAR_WRIST_R) || (slot == WEAR_FINGER_R) || (slot == HOLD_RIGHT))
      hand = HOLD_RIGHT;
    else
      hand = HOLD_LEFT;

    TObj *tobj = dynamic_cast<TObj *>(equipment[hand]);
    if (tobj && tobj->isPaired()) 
      hand = getPrimaryHold();

    if (equipment[hand]) {
      TThing *t_dropped = unequip(hand);
      *roomp += *t_dropped;
      sprintf(buf, "$p slips from your %s and falls to the $g!", describeBodySlot(hand).c_str());
      act(buf, FALSE, this, t_dropped, NULL, TO_CHAR);
      sprintf(buf, "$p slips from $n's %s and falls to the $g!", describeBodySlot(hand).c_str());
      act(buf, FALSE, this, t_dropped, NULL, TO_ROOM);
    }
  }
}

/*****************************************************************************/

void TBeing::shatterWeapon(wearSlotT slot, int scrap_it)
{
  char buf[256];
  wearSlotT hand;

  if (((slot >= WEAR_HAND_R) && (slot <= WEAR_ARM_L)) || (slot == HOLD_RIGHT) || (HOLD_LEFT)) {
    if ((slot == WEAR_HAND_R) || (slot == WEAR_ARM_R) || (slot == HOLD_RIGHT))
      hand = HOLD_RIGHT;
    else
      hand = HOLD_LEFT;

    TThing *tt = equipment[hand];
    TObj *shattered = dynamic_cast<TObj *>(tt);
    if (shattered) {
      sprintf(buf, "A miniature lightning bolt appears above you and zaps $p in your %s! You struggle to even keep a grip on it!", describeBodySlot(hand).c_str());
      act(buf, FALSE, this, shattered, NULL, TO_CHAR);
      sprintf(buf, "A miniature lightning bolt appears above $n and zaps $p in $s %s -- $e struggles to keep a grip on it!", describeBodySlot(hand).c_str());
      act(buf, FALSE, this, shattered, NULL, TO_ROOM);

      if (scrap_it) {
        if (!shattered->makeScraps()){
          delete shattered;
	  shattered = NULL;
	}
      } else {
	      shattered->addToStructPoints(-dice(3, 10));
	      if (shattered->getStructPoints() < 0) {
		if (!shattered->makeScraps()){
		  delete shattered;
		  shattered = NULL;
		}
	      } else
	        act("$p seems to be okay, though you can tell it took a lot of damage.", FALSE, this, shattered, NULL, TO_CHAR);
      }
    }
  }
}

/*****************************************************************************/
