//////////////////////////////////////////////////////////////////////////
//
// SneezyMUD - All rights reserved, SneezyMUD Coding Team
//
//////////////////////////////////////////////////////////////////////////


#include "stdsneezy.h"
#include "disc_blacksmithing.h"

CDBlacksmithing::CDBlacksmithing() :
  CDiscipline(),
  skBlacksmithingAdvanced()
{
}

CDBlacksmithing::CDBlacksmithing(const CDBlacksmithing &a) :
  CDiscipline(a),
  skBlacksmithingAdvanced(a.skBlacksmithingAdvanced)
{
}

CDBlacksmithing & CDBlacksmithing::operator=(const CDBlacksmithing &a)
{
  if (this == &a) return *this;
  CDiscipline::operator=(a);
  skBlacksmithingAdvanced = a.skBlacksmithingAdvanced;
  return *this;
}

CDBlacksmithing::~CDBlacksmithing()
{
}

