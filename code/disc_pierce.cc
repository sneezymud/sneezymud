//////////////////////////////////////////////////////////////////////////
//
// SneezyMUD - All rights reserved, SneezyMUD Coding Team
//
// $Log: disc_pierce.cc,v $
// Revision 5.1  1999/10/16 04:31:17  batopr
// new branch
//
// Revision 1.1  1999/09/12 17:24:04  sneezy
// Initial revision
//
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

