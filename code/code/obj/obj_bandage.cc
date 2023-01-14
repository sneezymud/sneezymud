//////////////////////////////////////////////////////////////////////////
//
// SneezyMUD - All rights reserved, SneezyMUD Coding Team
//
//////////////////////////////////////////////////////////////////////////

#include "obj_bandage.h"
#include "being.h"

TBandage::TBandage() : TObj() {}

TBandage::TBandage(const TBandage& a) : TObj(a) {}

TBandage& TBandage::operator=(const TBandage& a) {
  if (this == &a)
    return *this;
  TObj::operator=(a);
  return *this;
}

TBandage::~TBandage() {}

void TBandage::assignFourValues(int, int, int, int) {}

void TBandage::getFourValues(int* x1, int* x2, int* x3, int* x4) const {
  *x1 = 0;
  *x2 = 0;
  *x3 = 0;
  *x4 = 0;
}

sstring TBandage::statObjInfo() const {
  sstring a("");
  return a;
}

void TBandage::scrapMe(TBeing* ch) { ch->remLimbFlags(eq_pos, PART_BANDAGED); }

void TBandage::findBandage(int* count) { (*count)++; }

void TBandage::destroyBandage(int* count) {
  (*count)++;
  delete this;
}
