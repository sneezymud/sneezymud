//////////////////////////////////////////////////////////////////////////
//
// SneezyMUD - All rights reserved, SneezyMUD Coding Team
//
//////////////////////////////////////////////////////////////////////////


#include "stdsneezy.h"
#include "disc_slash.h"

CDSlash::CDSlash() :
  CDiscipline(),
  skSlashSpec()
{
}

CDSlash::CDSlash(const CDSlash &a) :
  CDiscipline(a),
  skSlashSpec(a.skSlashSpec)
{
}

CDSlash & CDSlash::operator=(const CDSlash &a)
{
  if (this == &a) return *this;
  CDiscipline::operator=(a);
  skSlashSpec = a.skSlashSpec;
  return *this;
}

CDSlash::~CDSlash()
{
}
