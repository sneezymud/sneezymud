// vial.cc

#include "stdsneezy.h"
#include "obj_vial.h"

TVial::TVial() :
  TBaseCup()
{
}

TVial::TVial(const TVial &a) :
  TBaseCup(a)
{
}

TVial & TVial::operator=(const TVial &a)
{
  if (this == &a) return *this;
  TBaseCup::operator=(a);
  return *this;
}

TVial::~TVial()
{
}

void TVial::getBestVial(TVial **best)
{
  if (getDrinkType() != LIQ_HOLYWATER)
    return;

  if (getDrinkUnits() <= 0)
    return;

  if (!*best)
    *best = this;

  if (getDrinkUnits() > (*best)->getDrinkUnits())
    *best = this;

  return;
}     

int TVial::rentCost() const
{
  int num = TBaseCup::rentCost();

  if (!getDrinkUnits())
    return 0;

  if (getDrinkType() != LIQ_HOLYWATER)
    return num;

  num *= getDrinkUnits();
  num /= max(1, getMaxDrinkUnits());
  return num;
}          

int TVial::objectSell(TBeing *ch, TMonster *keeper)
{
  sstring buf;

  if(getDrinkType()!=LIQ_HOLYWATER){
    keeper->doTell(ch->getName(), "Hey, that's not holy water!.");
    return TRUE;
  }

  if(getDrinkUnits()!=getMaxDrinkUnits()){
    keeper->doTell(ch->getName(), "I only purchase full vials.");
    return TRUE;
  }

  return FALSE;
}

bool TVial::objectRepair(TBeing *ch, TMonster *repair, silentTypeT silent)
{
  if (!silent)
    repair->doTell(fname(ch->name), "You might wanna take that somewhere else!");

  return TRUE;
}

int TVial::suggestedPrice() const
{
  // c.f. balance notes for this
  if (getDrinkType() != LIQ_HOLYWATER)
    return TBaseCup::suggestedPrice();

  return (int) (133.34 * (float) getMaxDrinkUnits() + 0.5);
}

void TVial::lowCheck()
{
  int ap = suggestedPrice();
  if (ap != obj_flags.cost && ap) {
    vlogf(LOG_LOW, fmt("vial (%s:%d) has a bad price (%d).  should be (%d)") % 
         getName() % objVnum() % obj_flags.cost % ap);
    obj_flags.cost = ap;
  }

  TBaseCup::lowCheck();
}
