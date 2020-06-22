#include "disc_offense.h"


CDOffense::CDOffense() :
  CDiscipline(),
  skAdvancedOffense()
{
}

CDOffense::CDOffense(const CDOffense &a) :
  CDiscipline(a),
  skAdvancedOffense(a.skAdvancedOffense)
{
}

CDOffense & CDOffense::operator=(const CDOffense &a)
{
  if (this == &a) return *this;
  CDiscipline::operator=(a);
  skAdvancedOffense = a.skAdvancedOffense;
  return *this;
}

CDOffense::~CDOffense()
{
}
