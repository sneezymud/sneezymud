//////////////////////////////////////////////////////////////////////////
//
// SneezyMUD - All rights reserved, SneezyMUD Coding Team
//
//      "base_light.cc" - Methods for TBaseLight class
//
///////////////////////////////////////////////////////////////////////////

#include "obj_base_light.h"

TBaseLight::TBaseLight() :
  TObj(),
  amtLight(0),
  maxBurn(0),
  curBurn(0)
{
}

TBaseLight::TBaseLight(const TBaseLight &a) :
  TObj(a),
  amtLight(a.amtLight),
  maxBurn(a.maxBurn),
  curBurn(a.curBurn)
{
}

TBaseLight & TBaseLight::operator=(const TBaseLight &a)
{
  if (this == &a) return *this;
  TObj::operator=(a);
  amtLight = a.amtLight;
  maxBurn = a.maxBurn;
  curBurn = a.curBurn;
  return *this;
}

TBaseLight::~TBaseLight()
{
}

void TBaseLight::addToLightAmt(int n)
{
  amtLight += n;
}

void TBaseLight::setLightAmt(int n)
{
  amtLight = n;
}

int TBaseLight::getLightAmt() const
{
  return amtLight;
}

void TBaseLight::addToMaxBurn(int n)
{
  maxBurn += n;
}

void TBaseLight::setMaxBurn(int n)
{
  maxBurn = n;
}

int TBaseLight::getMaxBurn() const
{
  return maxBurn;
}

void TBaseLight::addToCurBurn(int n)
{
  curBurn += n;
}

void TBaseLight::setCurBurn(int n)
{
  curBurn = n;
}

int TBaseLight::getCurBurn() const
{
  return curBurn;
}

bool TBaseLight::monkRestrictedItem(const TBeing *) const
{
  return FALSE;
}

bool TBaseLight::shamanRestrictedItem(const TBeing *) const
{
  return FALSE;
}

bool TBaseLight::rangerRestrictedItem(const TBeing *) const
{
  return FALSE;
}

void TBaseLight::assignFourValues(int x1, int x2, int x3, int)
{
  setLightAmt(x1);
  setMaxBurn(x2);
  setCurBurn(x3);
}

void TBaseLight::getFourValues(int *x1, int *x2, int *x3, int *) const
{
  *x1 = getLightAmt();
  *x2 = getMaxBurn();
  *x3 = getCurBurn();
}

void TBaseLight::putLightOut()
{
  int i;

  for (i = 0; i < MAX_OBJ_AFFECT; i++) {
    if (affected[i].location == APPLY_LIGHT) {
      affected[i].location = APPLY_NONE;
      addToLight(-affected[i].modifier);
      affected[i].modifier = 0;
    }
  }
}

