//////////////////////////////////////////////////////////////////////////
//
// SneezyMUD - All rights reserved, SneezyMUD Coding Team
//
//////////////////////////////////////////////////////////////////////////


#include "stdsneezy.h"
#include "disc_physical.h"

CDPhysical::CDPhysical() :
      CDiscipline(),
      skDoorbash(),
      skDualWieldWarrior(),
      skPowerMove(),
      skDeathstroke()
{
}

CDPhysical::CDPhysical(const CDPhysical &a) :
      CDiscipline(a),
      skDoorbash(a.skDoorbash),
      skDualWieldWarrior(a.skDualWieldWarrior),
      skPowerMove(a.skPowerMove),
      skDeathstroke(a.skDeathstroke)
{
}

CDPhysical & CDPhysical::operator=(const CDPhysical &a)
{
  if (this == &a) return *this;
  CDiscipline::operator=(a);
  skDoorbash = a.skDoorbash;
  skDualWieldWarrior = a.skDualWieldWarrior;
  skPowerMove = a.skPowerMove;
  skDeathstroke = a.skDeathstroke;
  return *this;
}

CDPhysical::~CDPhysical()
{
}




