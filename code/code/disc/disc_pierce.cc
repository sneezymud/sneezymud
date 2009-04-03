//////////////////////////////////////////////////////////////////////////
//
// SneezyMUD - All rights reserved, SneezyMUD Coding Team
//
//////////////////////////////////////////////////////////////////////////


#include "stdsneezy.h"
#include "disc_pierce.h"

CDPierce::CDPierce() :
  CDiscipline(),
  skPierceSpec()
{
}

CDPierce::CDPierce(const CDPierce &a) :
  CDiscipline(a),
  skPierceSpec(a.skPierceSpec)
{
}

CDPierce & CDPierce::operator=(const CDPierce &a)
{
  if (this == &a) return *this;
  CDiscipline::operator=(a);
  skPierceSpec = a.skPierceSpec;
  return *this;
}

CDPierce::~CDPierce()
{
}

