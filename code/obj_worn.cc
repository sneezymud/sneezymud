//////////////////////////////////////////////////////////////////////////
//
// SneezyMUD - All rights reserved, SneezyMUD Coding Team
//
// $Log: obj_worn.cc,v $
// Revision 5.3  2003/03/13 22:40:54  peel
// added sstring class, same as string but takes NULL as an empty string
// replaced all uses of string to sstring
//
// Revision 5.2  2002/01/12 01:53:24  peel
// removed the remaining class definitions from obj2.h
// obj2.h is no more!
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


// worn.cc
//

#include "stdsneezy.h"
#include "obj_base_clothing.h"
#include "obj_worn.h"

TWorn::TWorn() :
  TBaseClothing()
{
}

TWorn::TWorn(const TWorn &a) :
  TBaseClothing(a)
{
}

TWorn & TWorn::operator=(const TWorn &a)
{
  if (this == &a) return *this;
  TBaseClothing::operator=(a);
  return *this;
}

TWorn::~TWorn()
{
}

void TWorn::assignFourValues(int , int , int , int )
{
}

void TWorn::getFourValues(int *x1, int *x2, int *x3, int *x4) const
{
  *x1 = 0;
  *x2 = 0;
  *x3 = 0;
  *x4 = 0;
}

sstring TWorn::statObjInfo() const
{
  sstring a("");
  return a;
}

void TWorn::lowCheck()
{
  TBaseClothing::lowCheck();
}
