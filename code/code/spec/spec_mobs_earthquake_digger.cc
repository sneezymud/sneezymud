/*****************************************************************************

  SneezyMUD++ - All rights reserved, SneezyMUD Coding Team.

  "spec_mobs_earthquake_digger.cc"
  All functions and routines related to the Tunneler/Earthquake code.

  Created 10/21/99 - Lapsos(William A. Perrotto III)

******************************************************************************/

#include "room.h"
#include "monster.h"

int tunnelerEarthquake(TBeing *ch, cmdTypeT tCmd, const char *tArg, TMonster *tMyself, TObj *tObj)
{
  if (!tMyself || !tMyself->roomp)
    return FALSE;

  if (tMyself->roomp->isUnderwaterSector() ||
      tMyself->roomp->isWierdSector() ||
      tMyself->roomp->isFallSector() ||
      tMyself->roomp->isWaterSector())
    return FALSE;

  if (tCmd != CMD_GENERIC_PULSE &&
      tCmd != CMD_MOB_COMBAT)
    return FALSE;

  if (tCmd == CMD_GENERIC_PULSE && !::number(0, 9) && !tMyself->fight()) {
    act("$n rams into the ground then thunders up somewhere else.",
        FALSE, tMyself, NULL, NULL, TO_ROOM);

    return FALSE;
  }

  if (tCmd == CMD_MOB_COMBAT && !::number(0, 1) && tMyself->fight()) {
    int     tFighters = 0,
            tDamage;
    TThing *tThing=NULL;
    TBeing *tBeing;

    act("$n rises into the air then slams into the ground, vanishing.",
        FALSE, tMyself, NULL, NULL, TO_ROOM);

    for(StuffIter it=tMyself->roomp->stuff.begin();it!=tMyself->roomp->stuff.end();){
      tThing=*(it++);

      if ((tBeing = dynamic_cast<TBeing *>(tThing)))
        if (tBeing->isFlying())
          act("The ground, far below you, shakes violently.",
              FALSE, tBeing, NULL, NULL, TO_CHAR);
        else if (tBeing->isLevitating())
          act("The ground below you shakes violently.",
              FALSE, tBeing, NULL, NULL, TO_CHAR);
        else if (!::number(0, tMyself->GetMaxLevel()))
          act("The ground below you shakes violently, but you manage to stay on your feet.",
              FALSE, tBeing, NULL, NULL, TO_CHAR);
        else {
          act("The ground below you shakes violently and you fall on your face!",
              FALSE, tBeing, NULL, NULL, TO_CHAR);
          tBeing->setPosition(POSITION_SITTING);

          tDamage = max(4, (tMyself->GetMaxLevel() / 4));

          if (tBeing->fight() == tMyself)
            if (tMyself->reconcileDamage(tBeing, tDamage, SPELL_EARTHQUAKE) == -1) {
              delete tBeing;
              tBeing = NULL;
            } else
              tFighters++;
        }
    }

    // We only switch if our attackers are more than 3.
    if (tFighters < 3 ||
        ::number(-(tMyself->GetMaxLevel() / 2), tMyself->GetMaxLevel()) <= 0) {
      act("$n thunders back out of the ground.",
          TRUE, tMyself, NULL, NULL, TO_ROOM);

      return TRUE;
    }

    tFighters = ::number(1, tFighters);


    for(StuffIter it=tMyself->roomp->stuff.begin();it!=tMyself->roomp->stuff.end() && (tThing=*it);++it)
      if ((tBeing = dynamic_cast<TBeing *>(tThing)) &&
          tBeing->fight() == tMyself && !(--tFighters)) {
        tMyself->stopFighting();
        tMyself->setCharFighting(tBeing);

        act("$n thunders out of the ground and lunges towards you!",
            FALSE, tMyself, NULL, tBeing, TO_VICT);
        act("$n thunders out of the ground towards $N!",
            FALSE, tMyself, NULL, tBeing, TO_NOTVICT);
        act("You thunder out of the ground towards $N!",
            FALSE, tMyself, NULL, tBeing, TO_CHAR);

        return TRUE;
      }

    vlogf(LOG_BUG, "earthquake_digger had a problem, tell a coder.");
  }

  return FALSE;
}
