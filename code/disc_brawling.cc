//////////////////////////////////////////////////////////////////////////
//
// SneezyMUD - All rights reserved, SneezyMUD Coding Team
//
//////////////////////////////////////////////////////////////////////////


#include "stdsneezy.h"
#include "disc_brawling.h"

CDBrawling::CDBrawling() :
  CDiscipline(),
  skGrapple(),
  skStomp(),
  skBodyslam(),
  skSpin()
{
}

CDBrawling::CDBrawling(const CDBrawling &a) :
  CDiscipline(a),
  skGrapple(a.skGrapple),
  skStomp(a.skStomp),
  skBodyslam(a.skBodyslam),
  skSpin(a.skSpin)
{
}

CDBrawling & CDBrawling::operator=(const CDBrawling &a)
{
  if (this == &a) return *this;
  CDiscipline::operator=(a);
  skGrapple = a.skGrapple;
  skStomp = a.skStomp;
  skBodyslam = a.skBodyslam;
  skSpin = a.skSpin;
  return *this;
}

CDBrawling::~CDBrawling()
{
}
















