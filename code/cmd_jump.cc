/*****************************************************************************

  SneezyMUD++ - All rights reserved, SneezyMUD Coding Team.

  "task_stavecharge.cc"
  All functions and routines related to the stave charging task.

  Created 7/20/99 - Lapsos(William A. Perrotto III)

******************************************************************************/

struct tJumpCmdInformation
{
  int  tFromRoom, // Room they have to be in to trigger this.
       tToRoom, // Room they will be moved to.
       tDamageLight[2], // Damage (min, max) for saved fall.
       tDamageMedium[2], // Damage (min, max) for normal fall.
       tDamageHeavy[2]; // Damage (min, max) for when they really screw up.
  char tTrigger[256], // Word that triggers jump (jump <trigger>)
       tJumpChar[256], // General Jump message in fromRoom (to-char)
       tJumpRoom[256], // General Jump message in fromRoom (to-room)
       tLightChar[256], // Light Damage Message (to-char)
       tLightRoom[256], // Light Damage Message (to-room)
       tMediumChar[256], // Medium Damage Message (to-char)
       tMediumRoom[256], // Medium Damage Message (to-room)
       tHeavyChar[256], // Heavy Damage Message (to-char)
       tHeavyRoom[256], // Heavy Damage Message (to-room)
       tImmortalJumpChar[256], // Immortal Jump (to-char)
       tImmortalJumpRoom[256], // Immortal Jump (to-room)
       tImmortalLandChar[256], // Immortal Land (to-char)
       tImmortalLandRoom[256], // Immortal Land (to-room)
       tFlyJumpChar[256], // Fly Jump Message (to-char)
       tFlyJumpRoom[256], // Fly Jump Message (to-room)
       tFlyChar[256], // Fly Land Message (to-char)
       tFlyRoom[256]; // Fly Land Message (to-room)
} tJumpData[] = {
  {20525, 20412, {10, 20}, {15, 30}, {30, 60}, "bridge",
   "You dive over the edge of the bridge...GERONIMO!\n\r",
   "$n dives over the edge of the bridge, cross your fingers!",
   "You dive softly into the water, lessening the impact.\n\r",
   "$n dives into the waters from above, lucky bastard!!",
   "You slam into the water, well there goes the childhood memories.\n\r",
   "$n slams into the water from above, that hard to HURT!",
   "You screwup and slam HARD into the water...Well there goes the brain!\n\r",
   "$n arrives from above and attempts to break the water with their head!",
   "You gracefully dive over the edge of the bridge.\n\r",
   "$n does a perfect dive over the edge of the bridge.",
   "You dive utterly perfect into the water, damn being immortal rocks!\n\r",
   "$n dives perfectly into the water from above.",
   "You fly over the edge of the bridge and suddenly fall!\n\r",
   "$n flies over the edge of the bridge then vanishes!",
   "Luckily you regain control at the last moment...\n\r",
   "$n falls from above but stops meer inches from the water!"},

  {20526, 20412, {10, 20}, {15, 30}, {30, 60}, "bridge",
   "You dive over the edge of the bridge...GERONIMO!\n\r",
   "$n dives over the edge of the bridge, cross your fingers!",
   "You dive softly into the water, lessening the impact.\n\r",
   "$n dives into the waters from above, lucky bastard!!",
   "You slam into the water, well there goes the childhood memories.\n\r",
   "$n slams into the water from above, that hard to HURT!",
   "You screwup and slam HARD into the water...Well there goes the brain!\n\r",
   "$n arrives from above and attempts to break the water with their head!",
   "You gracefully dive over the edge of the bridge.\n\r",
   "$n does a perfect dive over the edge of the bridge.",
   "You dive utterly perfect into the water, damn being immortal rocks!\n\r",
   "$n dives perfectly into the water from above.",
   "You fly over the edge of the bridge and suddenly fall!\n\r",
   "$n flies over the edge of the bridge then vanishes!",
   "Luckily you regain control at the last moment...\n\r",
   "$n falls from above but stops meer inches from the water!"}
};

#include "stdsneezy.h"

int TBeing::doJump(const sstring &tArg)
{
  // Ignore invalid or errored locations.
  if (!roomp || !roomp->number)
    return FALSE;

  if (!tArg.empty())
    for (unsigned int tJumpIndex = 0;
         tJumpIndex < (sizeof(tJumpData) / sizeof(tJumpCmdInformation));
         tJumpIndex++)
      if (roomp->number == tJumpData[tJumpIndex].tFromRoom &&
          is_abbrev(tArg, tJumpData[tJumpIndex].tTrigger)) {
        if (isImmortal()) {
          sendTo(tJumpData[tJumpIndex].tImmortalJumpChar);
          act(tJumpData[tJumpIndex].tImmortalJumpRoom,
              TRUE, this, NULL, NULL, TO_ROOM);
          --(*this);
          thing_to_room(this, tJumpData[tJumpIndex].tToRoom);
          doLook("", CMD_LOOK);
          sendTo(tJumpData[tJumpIndex].tImmortalLandChar);
          act(tJumpData[tJumpIndex].tImmortalLandRoom,
              TRUE, this, NULL, NULL, TO_ROOM);
        } else if (isFlying()) {
          sendTo(tJumpData[tJumpIndex].tFlyJumpChar);
          act(tJumpData[tJumpIndex].tFlyJumpRoom,
              TRUE, this, NULL, NULL, TO_ROOM);
          --(*this);
          thing_to_room(this, tJumpData[tJumpIndex].tToRoom);
          doLook("", CMD_LOOK);
          sendTo(tJumpData[tJumpIndex].tFlyChar);
          act(tJumpData[tJumpIndex].tFlyRoom,
              TRUE, this, NULL, NULL, TO_ROOM);
        } else {
          sendTo(tJumpData[tJumpIndex].tJumpChar);
          act(tJumpData[tJumpIndex].tJumpRoom,
              TRUE, this, NULL, NULL, TO_ROOM);
          --(*this);
          thing_to_room(this, tJumpData[tJumpIndex].tToRoom);
          doLook("", CMD_LOOK);

          int tDamage = 0;

          if (::number(0, 3) || isAgile(0)) {
            sendTo(tJumpData[tJumpIndex].tLightChar);
            act(tJumpData[tJumpIndex].tLightRoom,
                TRUE, this, NULL, NULL, TO_ROOM);
            tDamage = ::number(tJumpData[tJumpIndex].tDamageLight[0],
                               tJumpData[tJumpIndex].tDamageLight[1]);
          } else if (::number(0, 20) || isAgile(0)) {
            sendTo(tJumpData[tJumpIndex].tMediumChar);
            act(tJumpData[tJumpIndex].tMediumRoom,
                TRUE, this, NULL, NULL, TO_ROOM);
            tDamage = ::number(tJumpData[tJumpIndex].tDamageMedium[0],
                               tJumpData[tJumpIndex].tDamageMedium[1]);
          } else {
            sendTo(tJumpData[tJumpIndex].tHeavyChar);
            act(tJumpData[tJumpIndex].tHeavyRoom,
                TRUE, this, NULL, NULL, TO_ROOM);
            tDamage = ::number(tJumpData[tJumpIndex].tDamageHeavy[0],
                               tJumpData[tJumpIndex].tDamageHeavy[1]);
          }

          if (reconcileDamage(this, tDamage, DAMAGE_FALL) == -1)
            return DELETE_VICT;
        }

        return FALSE;
      }

  sendTo("You jump up and down for joy.\n\r");
  act("$n jumps up and down for joy.",
      TRUE, this, NULL, NULL, TO_ROOM);

  return FALSE;
}
