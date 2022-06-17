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
      skDeathstroke(),
      sk2hSpecWarrior(),
      skFortify()
{
}

CDSoldiering::CDSoldiering(const CDSoldiering &a) :
      CDiscipline(a),
      skDoorbash(a.skDoorbash),
      skDualWieldWarrior(a.skDualWieldWarrior),
      skPowerMove(a.skPowerMove),
      skDeathstroke(a.skDeathstroke),
      sk2hSpecWarrior(a.sk2hSpecWarrior),
      skFortify(a.skFortify)
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
  sk2hSpecWarrior = a.sk2hSpecWarrior;
  skFortify = a.skFortify;
  return *this;
}

CDSoldiering::~CDSoldiering()
{
}
