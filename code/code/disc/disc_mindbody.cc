//////////////////////////////////////////////////////////////////////////
//
// SneezyMUD - All rights reserved, SneezyMUD Coding Team
//
//////////////////////////////////////////////////////////////////////////


#include "disc_mindbody.h"

CDMindBody::CDMindBody() :
  CDiscipline(),
  skFeignDeath(),
  skBlur()
{
}

CDMindBody::CDMindBody(const CDMindBody &a) :
  CDiscipline(a),
  skFeignDeath(a.skFeignDeath),
  skBlur(a.skBlur)
{
}
 
CDMindBody & CDMindBody::operator=(const CDMindBody &a)
{
  if (this == &a) return *this;
  CDiscipline::operator=(a);
  skFeignDeath = a.skFeignDeath;
  skBlur = a.skBlur;

  return *this;
}

CDMindBody::~CDMindBody()
{
}

CDMindBody * CDMindBody::cloneMe()
{
  return new CDMindBody(*this);
}
