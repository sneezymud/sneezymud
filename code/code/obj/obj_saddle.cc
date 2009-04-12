//////////////////////////////////////////////////////////////////////////
//
// SneezyMUD - All rights reserved, SneezyMUD Coding Team
//
//////////////////////////////////////////////////////////////////////////


// saddle.cc
//

#include "obj_base_clothing.h"
#include "obj_saddle.h"

TSaddle::TSaddle() :
  TBaseClothing()
{
}

TSaddle::TSaddle(const TSaddle &a) :
  TBaseClothing(a)
{
}

TSaddle & TSaddle::operator=(const TSaddle &a)
{
  if (this == &a) return *this;
  TBaseClothing::operator=(a);
  return *this;
}

TSaddle::~TSaddle()
{
}

void TSaddle::assignFourValues(int , int , int , int )
{
}

void TSaddle::getFourValues(int *x1, int *x2, int *x3, int *x4) const
{
  *x1 = 0;
  *x2 = 0;
  *x3 = 0;
  *x4 = 0;
}

sstring TSaddle::statObjInfo() const
{
  sstring a("");
  return a;
}

void TSaddle::lowCheck()
{
  TBaseClothing::lowCheck();
}
