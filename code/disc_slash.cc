//////////////////////////////////////////////////////////////////////////
//
// SneezyMUD - All rights reserved, SneezyMUD Coding Team
//
// $Log: disc_slash.cc,v $
// Revision 1.1  1999/09/12 17:24:04  sneezy
// Initial revision
//
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
