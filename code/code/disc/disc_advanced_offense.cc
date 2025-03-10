#include "disc_advanced_offense.h"

CDOffense::CDOffense() :
  CDiscipline(),
  skAdvancedOffense(),
  skInevitability() {}

CDOffense::CDOffense(const CDOffense& a) :
  CDiscipline(a),
  skAdvancedOffense(a.skAdvancedOffense),
  skInevitability(a.skInevitability) {}

CDOffense& CDOffense::operator=(const CDOffense& a) {
  if (this == &a)
    return *this;
  CDiscipline::operator=(a);
  skAdvancedOffense = a.skAdvancedOffense;
  skInevitability = a.skInevitability;
  return *this;
}

CDOffense::~CDOffense() {}
