/*****************************************************************************

  SneezyMUD++ - All rights reserved, SneezyMUD Coding Team.

  "task_stavecharge.cc"
  All functions and routines related to the stave charging task.

  Created 7/20/99 - Lapsos(William A. Perrotto III)

******************************************************************************/

const int TBridgeA = -1;

#include "stdsneezy.h"

int TBeing::doJump(const char *tArg)
{
  if (!tArg || !*tArg)
    return FALSE;

#if 0
  if (is_abbrev(tArg, "bridge") && roomp->number == TBridgeA) {
    if (isFlying()) {
      sendTo("You fly over the edge of the bridge and suddenly fall!\n\r");
      sendTo("Luckily you regain control at the last moment...\n\r");
    } else {
      sendTo("You dive over the edge of the bridge....GERONIMO!\n\r");
      sendTo("You slam into the ground HARD!\n\r");
    }
  }
#endif

  sendTo("You jump up and down for joy.\n\r");
  act("$n jumps up and down for joy.",
      TRUE, this, NULL, NULL, TO_ROOM);

  return FALSE;
}
