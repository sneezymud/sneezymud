//////////////////////////////////////////////////////////////////////////
//
// SneezyMUD - All rights reserved, SneezyMUD Coding Team
//
// $Log: disc_mindbody.cc,v $
// Revision 5.1  1999/10/16 04:31:17  batopr
// new branch
//
// Revision 1.1  1999/09/12 17:24:04  sneezy
// Initial revision
//
//
//////////////////////////////////////////////////////////////////////////


#include "stdsneezy.h"
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
