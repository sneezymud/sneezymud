//////////////////////////////////////////////////////////////////////////
//
// SneezyMUD - All rights reserved, SneezyMUD Coding Team
//
// $Log: obj_pen.cc,v $
// Revision 5.2  2002/01/09 23:27:04  peel
// More splitting up of obj2.h
// renamed food.cc to obj_food.cc
// renamed organic.cc to obj_organic.cc
//
// Revision 5.1  2001/07/13 05:32:20  peel
// renamed a bunch of source files
//
// Revision 5.1.1.1  1999/10/16 04:32:20  batopr
// new branch
//
// Revision 5.1  1999/10/16 04:31:17  batopr
// new branch
//
// Revision 1.1  1999/09/12 17:24:04  sneezy
// Initial revision
//
//
//////////////////////////////////////////////////////////////////////////


// pen.cc

#include "stdsneezy.h"
#include "obj_pen.h"

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

string TPen::statObjInfo() const
{
  string a("");
  return a;
}

void TPen::thingDumped(TBeing *ch, int *)
{
  act("$p vanishes in a puff of smoke.", TRUE, ch, this, 0, TO_ROOM);
  act("$p vanishes in a puff of smoke.", TRUE, ch, this, 0, TO_CHAR);

  delete this;
}

void TPen::writeMePen(TBeing *ch, TThing *note)
{
   note->writeMeNote(ch, this);
}

