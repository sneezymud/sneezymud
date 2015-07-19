//////////////////////////////////////////////////////////////////////////
//
// SneezyMUD - All rights reserved, SneezyMUD Coding Team
//
//////////////////////////////////////////////////////////////////////////


// pen.cc

#include "comm.h"
#include "obj_pen.h"
#include "thing.h"
#include "being.h"

TPen::TPen() :
  TObj()
{
}

TPen::TPen(const TPen &a) :
  TObj(a)
{
}

TPen & TPen::operator=(const TPen &a)
{
  if (this == &a) return *this;
  TObj::operator=(a);
  return *this;
}

TPen::~TPen()
{
}

void TPen::assignFourValues(int, int, int, int)
{
}

void TPen::getFourValues(int *x1, int *x2, int *x3, int *x4) const
{
  *x1 = 0;
  *x2 = 0;
  *x3 = 0;
  *x4 = 0;
}

sstring TPen::statObjInfo() const
{
  sstring a("");
  return a;
}

void TPen::writeMePen(TBeing *ch, TThing *note)
{
   note->writeMeNote(ch, this);
}

