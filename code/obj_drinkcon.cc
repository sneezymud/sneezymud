#include "stdsneezy.h"
#include "obj_base_container.h"
#include "obj_drinkcon.h"


TDrinkCon::TDrinkCon() :
  TBaseCup()
{
}

TDrinkCon::TDrinkCon(const TDrinkCon &a) :
  TBaseCup(a)
{
}

TDrinkCon & TDrinkCon::operator=(const TDrinkCon &a)
{
  if (this == &a) return *this;
  TBaseCup::operator=(a);
  return *this;
}

TDrinkCon::~TDrinkCon()
{
}

void TDrinkCon::findSomeDrink(TDrinkCon **last_good, TBaseContainer **last_cont, TBaseContainer *cont)
{
  // 6-1-2004 - We want to make sure this is Water, if they want to drink something else they need to do it manually.  -Lapsos
  if ((getDrinkUnits() > 0) && (getLiqThirst() > 0) && (getDrinkType() == LIQ_WATER)) {
    *last_good = this;
    *last_cont = cont;
  }
}

void TDrinkCon::waterCreate(const TBeing *caster, int level)
{
  int water;

  if ((getDrinkType() != LIQ_WATER) && getDrinkUnits()) {
    CF(SPELL_CREATE_WATER);
    setDrinkType(LIQ_SLIME);
    act("A harsh green light surrounds $p!",
            FALSE, caster, this, NULL, TO_CHAR);
    act("A harsh green light surrounds $p!",
            FALSE, caster, this, NULL, TO_ROOM);
  } else {
    water = level;
    water *= 4;
    water /= 3;

    if ((caster->roomp->getWeather() == WEATHER_RAINY) ||
        (caster->roomp->getWeather() == WEATHER_LIGHTNING) ||
        (caster->roomp->getWeather() == WEATHER_SNOWY)) {
      water *= 2;
      CS(SPELL_CREATE_WATER);
    }

    // Calculate water it can contain, or water created
    water = min(getMaxDrinkUnits() - getDrinkUnits(), water);

    if (water > 0) {
      LogDam(caster, SPELL_CREATE_WATER, water);
      setDrinkType(LIQ_WATER);
      addToDrinkUnits(water);

      weightChangeObject(water * SIP_WEIGHT);

      if (getMaxDrinkUnits() == getDrinkUnits()) {
        act("$p is completely filled with water!",
            FALSE, caster, this, NULL, TO_CHAR);
        act("$p is completely filled with water!",
            FALSE, caster, this, NULL, TO_ROOM);
      } else {
        act("$p is partially filled with water.",
            FALSE, caster, this, NULL, TO_CHAR);
        act("$p is partially filled with water.",
            FALSE, caster, this, NULL, TO_ROOM);
      }
    } else {
      act("A vapor of steam appears over $p, but nothing else happens.",
            FALSE, caster, this, NULL, TO_CHAR);
      act("A vapor of steam appears over $p, but nothing else happens.",
            FALSE, caster, this, NULL, TO_ROOM);
    }
  }
}

