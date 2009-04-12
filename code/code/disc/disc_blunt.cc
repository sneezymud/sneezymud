//////////////////////////////////////////////////////////////////////////
//
// SneezyMUD - All rights reserved, SneezyMUD Coding Team
//
//////////////////////////////////////////////////////////////////////////


#include "disc_blunt.h"

CDBash::CDBash() :
  CDiscipline(),
  skBluntSpec()
{
}

CDBash::CDBash(const CDBash &a) :
  CDiscipline(a),
  skBluntSpec(a.skBluntSpec)
{
}

CDBash & CDBash::operator=(const CDBash &a)
{
  if (this == &a) return *this;
  CDiscipline::operator=(a);
  skBluntSpec = a.skBluntSpec;
  return *this;
}

CDBash::~CDBash()
{
}
