//////////////////////////////////////////////////////////////////////////
//
// SneezyMUD - All rights reserved, SneezyMUD Coding Team
//
// $Log: fuel.cc,v $
// Revision 1.1  1999/09/12 17:24:04  sneezy
// Initial revision
//
//
//////////////////////////////////////////////////////////////////////////


///////////////////////////////////////////////////////////////////////////
//
//      SneezyMUD++ 4.5 - All rights reserved, SneezyMUD Coding Team
//      "fuel.cc" - Methods for TFuel class
//
//      Last revision December 18, 1997.
//
///////////////////////////////////////////////////////////////////////////

#include "stdsneezy.h"

TFuel::TFuel() :
  TObj(),
  curFuel(0),
  maxFuel(0)
{
}

TFuel::TFuel(const TFuel &a) :
  TObj(a),
  curFuel(a.curFuel),
  maxFuel(a.maxFuel)
{
}

TFuel & TFuel::operator=(const TFuel &a)
{
  if (this == &a) return *this;

  TObj::operator=(a);
  curFuel = a.curFuel;
  maxFuel = a.maxFuel;
  return *this;
}

TFuel::~TFuel()
{
}

void TFuel::addToMaxFuel(int n)
{
  maxFuel += n;
}

void TFuel::setMaxFuel(int n)
{
  maxFuel = n;
}

int TFuel::getMaxFuel() const
{
  return maxFuel;
}

void TFuel::addToCurFuel(int n)
{
  curFuel += n;
}

void TFuel::setCurFuel(int n)
{
  curFuel = n;
}

int TFuel::getCurFuel() const
{
  return curFuel;
}

void TFuel::describeObjectSpecifics(const TBeing *ch) const
{
  double diff;

  if (getMaxFuel()) {
    diff = (double) ((double) getCurFuel() / (double) getMaxFuel());
    ch->sendTo(COLOR_OBJECTS,
          "You can tell that %s has %s of its fuel left.\n\r",
          good_uncap(getName()).c_str(),
          ((diff < .20) ? "very little" : ((diff < .50) ? "some" :
          ((diff < .75) ? "a good bit of" : "almost all of its"))));
  }
}

void TFuel::refuelMeFuel(TBeing *ch, TLight *lamp)
{
  int use;

  if (lamp->getMaxBurn() < 0) {
    act("$p can't be refueled.", FALSE, ch, lamp, 0, TO_CHAR);
    return;
  }
  if (lamp->getCurBurn() == lamp->getMaxBurn()) {
    act("$p is already full of fuel.", FALSE, ch, lamp, 0, TO_CHAR);
    return;
  }
  if (lamp->isLit()) {
    ch->sendTo("You better not fuel that while lit, it might explode.\n\r");
    return;
  }
  use = lamp->getMaxBurn() - lamp->getCurBurn();
  use = min(use, getCurFuel());

  act("$n refuels $s $o.", TRUE, ch, lamp, 0, TO_ROOM);
  ch->sendTo("You refuel your %s.\n\r", fname(lamp->name).c_str());

  addToCurFuel(-use);
  lamp->addToCurBurn(use);

  if (getCurFuel() <= 0) {
    ch->sendTo("Your fuel is all used up, and you discard it.\n\r");
    if (equippedBy) {
      dynamic_cast<TBeing *>(equippedBy)->unequip(eq_pos);
    } else
      --(*this);

    delete this;
  }
}

void TFuel::assignFourValues(int x1, int x2, int, int)
{
  setCurFuel(x1);
  setMaxFuel(x2);
}

void TFuel::getFourValues(int *x1, int *x2, int *x3, int *x4) const
{
  *x1 = getCurFuel();
  *x2 = getMaxFuel();
  *x3 = 0;
  *x4 = 0;
}

void TFuel::lowCheck()
{
  if (getCurFuel() > getMaxFuel())
    vlogf(LOW_ERROR,"fuel %s had more current fuel than max.", getName());

  TObj::lowCheck();
}

int TFuel::objectSell(TBeing *ch, TMonster *keeper)
{
  char buf[256];

  sprintf(buf, "%s I'm sorry, I don't buy back fuel.", ch->getName());
  keeper->doTell(buf);
  return TRUE;
}

string TFuel::statObjInfo() const
{
  char buf[256];

  sprintf(buf, "Refuel capability : current : %d, max : %d",
         getCurFuel(), getMaxFuel());

  string a(buf);
  return a;
}

int TFuel::getVolume() const
{
  int amt = TObj::getVolume();

  amt *= getCurFuel();
  if(getMaxFuel())
    amt /= getMaxFuel();

  return amt;
}

float TFuel::getTotalWeight(bool pweight) const
{
  float amt = TObj::getTotalWeight(pweight);

  amt *= getCurFuel();

  if(getMaxFuel())
    amt /= getMaxFuel();

  return amt;
}
