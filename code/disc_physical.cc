//////////////////////////////////////////////////////////////////////////
//
// SneezyMUD - All rights reserved, SneezyMUD Coding Team
//
// $Log: disc_physical.cc,v $
// Revision 5.1  1999/10/16 04:31:17  batopr
// new branch
//
// Revision 1.1  1999/09/12 17:24:04  sneezy
// Initial revision
//
//
//////////////////////////////////////////////////////////////////////////


#include "stdsneezy.h"
#include "disc_physical.h"

CDPhysical::CDPhysical() :
      CDiscipline(),
      skDoorbash(),
      skDeathstroke()
{
}

CDPhysical::CDPhysical(const CDPhysical &a) :
      CDiscipline(a),
      skDoorbash(a.skDoorbash),
      skDeathstroke(a.skDeathstroke)
{
}

CDPhysical & CDPhysical::operator=(const CDPhysical &a)
{
  if (this == &a) return *this;
  CDiscipline::operator=(a);
  skDoorbash = a.skDoorbash;
  skDeathstroke = a.skDeathstroke;
  return *this;
}

CDPhysical::~CDPhysical()
{
}

