/*****************************************************************************

  SneezyMUD++ - All rights reserved, SneezyMUD Coding Team.

  "spec_mobs_goring.cc"
  All functions and routines related to the Boar/Tusk Goring code.

******************************************************************************/

#include <algorithm>

#include "being.h"
#include "comm.h"
#include "enum.h"
#include "monster.h"
#include "parse.h"
#include "spells.h"

class TObj;

int tuskGoring(TBeing* ch, cmdTypeT tCmd, const char* tArg, TMonster* tMyself,
  TObj* tObj) {
  TBeing* tVictim = tMyself->fight();

  if (tCmd != CMD_MOB_COMBAT || !tMyself || !tMyself->awake() ||
      tMyself->spelltask || !tVictim || tVictim->riding ||
      tVictim->getPosition() > POSITION_STANDING ||
      !tMyself->sameRoom(*tVictim) || ::number(0, 7) ||
      tMyself->getPosition() < POSITION_STANDING)
    return false;

  if (!tVictim->isAgile(0) && ::number(0, 4)) {
    act("$n charges into $N, impaling him fiercly!", true, tMyself, nullptr,
      tVictim, TO_NOTVICT);
    act("$n barrels down on you, impaling you painfully!", true, tMyself, nullptr,
      tVictim, TO_VICT);
    act("You charge down upon $N, impaling them!", true, tMyself, nullptr, tVictim,
      TO_CHAR);

    int tDamage =
      max(10, (int)(((tMyself->GetMaxLevel() * 5) + ::number(-10, 10)) / 2));

    if (tMyself->reconcileDamage(tVictim, tDamage, DAMAGE_RAMMED) == -1) {
      delete tVictim;
      tVictim = nullptr;
      return true;
    }

    tMyself->cantHit += tMyself->loseRound(1);
    tVictim->cantHit += tVictim->loseRound(2);
    tVictim->setPosition(POSITION_SITTING);

    return true;
  } else {
    act("$n charges towards $N, but they easily dodges them.", true, tMyself,
      nullptr, tVictim, TO_NOTVICT);
    act("$n barrels down on you, but you easily dodge them.", true, tMyself,
      nullptr, tVictim, TO_VICT);
    act(
      "You charge down upon $N, but they easily dodge you making you look the "
      "ass!",
      true, tMyself, nullptr, tVictim, TO_CHAR);
  }

  return true;
}
