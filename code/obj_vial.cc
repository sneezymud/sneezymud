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
  char buf[256];

  sprintf(buf, "%s I'm sorry, I don't purchase vials.", ch->getName());
  keeper->doTell(buf);
  return TRUE;
}

bool TVial::objectRepair(TBeing *ch, TMonster *repair, silentTypeT silent)
{
  if (!silent) {
    char buf[256];

    sprintf(buf, "%s you might wanna take that somewhere else!", fname(ch->name).c_str());
    repair->doTell(buf);
  }
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
    vlogf(LOG_LOW, "vial (%s:%d) has a bad price (%d).  should be (%d)",
         getName(), objVnum(), obj_flags.cost, ap);
    obj_flags.cost = ap;
  }

  TBaseCup::lowCheck();
}
