//////////////////////////////////////////////////////////////////////////
//
// SneezyMUD - All rights reserved, SneezyMUD Coding Team
//
//////////////////////////////////////////////////////////////////////////


#include "disc_soldiering.h"

CDSoldiering::CDSoldiering() :
      CDiscipline(),
      skDoorbash(),
      skDualWieldWarrior(),
      skPowerMove(),
      skDeathstroke()
{
}

CDSoldiering::CDSoldiering(const CDSoldiering &a) :
      CDiscipline(a),
      skDoorbash(a.skDoorbash),
      skDualWieldWarrior(a.skDualWieldWarrior),
      skPowerMove(a.skPowerMove),
      skDeathstroke(a.skDeathstroke)
{
}

CDSoldiering & CDSoldiering::operator=(const CDSoldiering &a)
{
  if (this == &a) return *this;
  CDiscipline::operator=(a);
  skDoorbash = a.skDoorbash;
  skDualWieldWarrior = a.skDualWieldWarrior;
  skPowerMove = a.skPowerMove;
  skDeathstroke = a.skDeathstroke;
  return *this;
}

CDSoldiering::~CDSoldiering()
{
}




