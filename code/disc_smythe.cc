//////////////////////////////////////////////////////////////////////////
//
// SneezyMUD - All rights reserved, SneezyMUD Coding Team
//
// $Log: disc_smythe.cc,v $
// Revision 5.1  1999/10/16 04:31:17  batopr
// new branch
//
// Revision 1.1  1999/09/12 17:24:04  sneezy
// Initial revision
//
//
//////////////////////////////////////////////////////////////////////////


#include "stdsneezy.h"
#include "disc_smythe.h"

CDSmythe::CDSmythe() :
  CDiscipline()
{
}

CDSmythe::CDSmythe(const CDSmythe &a) :
  CDiscipline(a)
{
}

CDSmythe & CDSmythe::operator=(const CDSmythe &a)
{
  if (this == &a) return *this;
  CDiscipline::operator=(a);
  return *this;
}

CDSmythe::~CDSmythe()
{
}

