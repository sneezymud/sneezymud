//////////////////////////////////////////////////////////////////////////
//
// SneezyMUD - All rights reserved, SneezyMUD Coding Team
//
//////////////////////////////////////////////////////////////////////////


#include "stdsneezy.h"
#include "disc_smythe.h"

CDSmythe::CDSmythe() :
  CDiscipline(),
  skSmytheAdvanced()
{
}

CDSmythe::CDSmythe(const CDSmythe &a) :
  CDiscipline(a),
  skSmytheAdvanced(a.skSmytheAdvanced)
{
}

CDSmythe & CDSmythe::operator=(const CDSmythe &a)
{
  if (this == &a) return *this;
  CDiscipline::operator=(a);
  skSmytheAdvanced = a.skSmytheAdvanced;
  return *this;
}

CDSmythe::~CDSmythe()
{
}

