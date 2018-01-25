// vial.cc

#include "monster.h"
#include "obj_vial.h"
#include "materials.h"

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
}     

int TVial::objectSell(TBeing *ch, TMonster *keeper)
{
  sstring buf;

  if(getDrinkType()!=LIQ_HOLYWATER){
    keeper->doTell(ch, "Hey, that's not holy water!");
    return TRUE;
  }

  if(getDrinkUnits()!=getMaxDrinkUnits()){
    keeper->doTell(ch, "I only purchase full vials.");
    return TRUE;
  }

  return FALSE;
}

bool TVial::objectRepair(TBeing *ch, TMonster *repair, silentTypeT silent)
{
  if (!silent)
    repair->doTell(ch, "You might wanna take that somewhere else!");

  return TRUE;
}

int TVial::suggestedPrice() const
{
  // c.f. balance notes for this
  if (getDrinkType() != LIQ_HOLYWATER)
    return TBaseCup::suggestedPrice();

  return (int) ((133.34 * (float) getMaxDrinkUnits() + 0.5) +
		(int)(10.0 * getWeight() * material_nums[getMaterial()].price));
}

void TVial::lowCheck()
{
  int ap = suggestedPrice();
  if (ap != obj_flags.cost && ap) {
    vlogf(LOG_LOW, format("vial (%s:%d) has a bad price (%d).  should be (%d)") % 
         getName() % objVnum() % obj_flags.cost % ap);
    obj_flags.cost = ap;
  }

  TBaseCup::lowCheck();
}
