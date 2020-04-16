//////////////////////////////////////////////////////////////////////////
//
// SneezyMUD - All rights reserved, SneezyMUD Coding Team
//
//////////////////////////////////////////////////////////////////////////


#include "disc_defense.h"

CDDefense::CDDefense() :
  CDiscipline(),
  skAdvancedDefense(),
  skFocusedAvoidance()
{
}

CDDefense::CDDefense(const CDDefense &a) :
  CDiscipline(a),
  skAdvancedDefense(a.skAdvancedDefense),
  skFocusedAvoidance(a.skFocusedAvoidance)
{
}

CDDefense & CDDefense::operator=(const CDDefense &a)
{
  if (this == &a) return *this;
  CDiscipline::operator=(a);
  skAdvancedDefense = a.skAdvancedDefense;
  skFocusedAvoidance = a.skFocusedAvoidance;
  return *this;
}

CDDefense::~CDDefense()
{
}
