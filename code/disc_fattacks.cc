//////////////////////////////////////////////////////////////////////////
//
// SneezyMUD - All rights reserved, SneezyMUD Coding Team
//
//////////////////////////////////////////////////////////////////////////


#include "stdsneezy.h"
#include "disc_fattacks.h"

CDFAttacks::CDFAttacks() :
  CDiscipline(),
  skQuiveringPalm(),
  skCriticalHitting()
{
}

CDFAttacks::CDFAttacks(const CDFAttacks &a) :
  CDiscipline(a),
  skQuiveringPalm(a.skQuiveringPalm),
  skCriticalHitting(a.skCriticalHitting)
{
}

CDFAttacks & CDFAttacks::operator=(const CDFAttacks &a)
{
  if (this == &a) return *this;
  CDiscipline::operator=(a);
  skQuiveringPalm = a.skQuiveringPalm;
  skCriticalHitting = a.skCriticalHitting;
  return *this;
}

CDFAttacks::~CDFAttacks()
{
}

CDFAttacks * CDFAttacks::cloneMe()
{
  return new CDFAttacks(*this);
}
