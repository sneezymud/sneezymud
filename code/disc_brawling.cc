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
  skBrawlAvoidance(),
  skBodyslam(),
  skSpin(),
  skCloseQuartersFighting()
{
}

CDBrawling::CDBrawling(const CDBrawling &a) :
  CDiscipline(a),
  skGrapple(a.skGrapple),
  skStomp(a.skStomp),
  skBrawlAvoidance(a.skBrawlAvoidance),
  skBodyslam(a.skBodyslam),
  skSpin(a.skSpin),
  skCloseQuartersFighting(a.skCloseQuartersFighting)
{
}

CDBrawling & CDBrawling::operator=(const CDBrawling &a)
{
  if (this == &a) return *this;
  CDiscipline::operator=(a);
  skGrapple = a.skGrapple;
  skStomp = a.skStomp;
  skBrawlAvoidance = a.skBrawlAvoidance;
  skBodyslam = a.skBodyslam;
  skSpin = a.skSpin;
  skCloseQuartersFighting = a.skCloseQuartersFighting;
  return *this;
}

CDBrawling::~CDBrawling()
{
}
















