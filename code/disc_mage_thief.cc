#include "stdsneezy.h"
#include "disc_mage_thief.h"

CDMageThief::CDMageThief() :
  CDiscipline(),
  skBackstabMT()
{
}

CDMageThief::CDMageThief(const CDMageThief &a) :
  CDiscipline(a),
  skBackstabMT(a.skBackstabMT)
{
}

CDMageThief & CDMageThief::operator=(const CDMageThief &a)
{
      if (this == &a) return *this;
      CDiscipline::operator=(a);
      skBackstabMT = a.skBackstabMT;
      return *this;
}

CDMageThief::~CDMageThief()
{
}







