//////////////////////////////////////////////////////////////////////////
//
// SneezyMUD - All rights reserved, SneezyMUD Coding Team
//
// $Log: obj_drinkcon.cc,v $
// Revision 5.1  2001/07/13 05:32:20  peel
// renamed a bunch of source files
//
// Revision 5.1.1.4  2001/01/25 03:28:55  dash
// added cola drinktype
//
// Revision 5.1.1.3  2000/09/04 04:38:08  jesus
// *** empty log message ***
//
// Revision 5.1.1.2  2000/01/27 17:49:53  batopr
// renamed classes project
//
// Revision 5.1.1.1  1999/10/16 04:32:20  batopr
// new branch
//
// Revision 5.1  1999/10/16 04:31:17  batopr
// new branch
//
// Revision 1.1  1999/09/12 17:24:04  sneezy
// Initial revision
//
//
//////////////////////////////////////////////////////////////////////////


// drinkcon.cc

#include "stdsneezy.h"

drinkInfo * DrinkInfo[MAX_DRINK_TYPES];

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
  if ((getDrinkUnits() > 0) && (getLiqThirst() > 0)) {
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

drinkInfo::drinkInfo(int d, int h, int t, const char *col, const char *n) :
  drunk(d),
  hunger(h),
  thirst(t),
  color(col),
  name(n)
{
}

drinkInfo & drinkInfo::operator = (const drinkInfo &a) 
{
  if (this == &a) return *this;

  drunk = a.drunk;
  hunger = a.hunger;
  thirst = a.thirst;
  color = a.color;
  name = a.name;

  return *this;
}

drinkInfo::~drinkInfo()
{
}

void assign_drink_types()
{
  DrinkInfo[LIQ_WATER] = new drinkInfo(0,  0, 10, "clear", "<c>water<1>");
  DrinkInfo[LIQ_BEER] = new drinkInfo(5, -2,  7, "<o>brown<1>", "<o>beer<1>");
  DrinkInfo[LIQ_WINE] = new drinkInfo(4, -1,  6, "clear", "<W>white wine<1>");
  DrinkInfo[LIQ_ALE] = new drinkInfo(6, -3,  5, "<o>brown<1>", "<o>ale<1>");
  DrinkInfo[LIQ_DARKALE] = new drinkInfo(5, -1,  5, "<k>dark<1>", "<k>dark<1> <o>ale<1>");
  DrinkInfo[LIQ_WHISKY] = new drinkInfo(10,  0,  1, "<y>golden<1>", "<y>whiskey<1>");
  DrinkInfo[LIQ_LEMONADE] = new drinkInfo(0,  1,  8, "<y>golden<1>", "<y>lemonade<1>");
  DrinkInfo[LIQ_FIREBRT] = new drinkInfo(15,  0, -3, "<g>green<1>", "<g>firebreather<1>");
  DrinkInfo[LIQ_LOCALSPC] = new drinkInfo(8, -1,  2, "clear", "local special");
  DrinkInfo[LIQ_SLIME] = new drinkInfo(0,  1,  8, "<G>light green<1>", "<G>juice<1>");
  DrinkInfo[LIQ_MILK] = new drinkInfo(0,  2,  6, "<W>white<1>", "<W>milk<1>");
  DrinkInfo[LIQ_TEA] = new drinkInfo(-1, -1,  6, "<o>brown<1>", "<o>tea<1>");
  DrinkInfo[LIQ_COFFEE] = new drinkInfo(-2, -3,  5, "<k>black<1>", "<k>coffee<1>");
  DrinkInfo[LIQ_BLOOD] = new drinkInfo(0,  2, -1, "<r>red<1>", "<r>blood<1>");
  DrinkInfo[LIQ_SALTWATER] = new drinkInfo(0,  1, -5, "clear", "salt water");
  DrinkInfo[LIQ_MEAD] = new drinkInfo(2,  2,  4, "<k>black<1>", "<k>mead<1>");
  DrinkInfo[LIQ_VODKA] = new drinkInfo(12, -3, -1, "clear", "vodka");
  DrinkInfo[LIQ_RUM] = new drinkInfo(11, -3, -1, "clear", "rum");
  DrinkInfo[LIQ_BRANDY] = new drinkInfo(8,  1,  3, "<o>brown<1>", "<o>brandy<1>");
  DrinkInfo[LIQ_RED_WINE] = new drinkInfo(7, -1,  6, "<R>red<1>", "<R>red wine<1>");
  DrinkInfo[LIQ_WARM_MEAD] = new drinkInfo(2,  1,  5, "<k>black<1>", "<k>warm mead<1>");
  DrinkInfo[LIQ_CHAMPAGNE] = new drinkInfo(6, -2,  4, "bubbly, translucent", "champagne");
  DrinkInfo[LIQ_HOLYWATER] = new drinkInfo(0,  1, -5, "clear", "holy water");
  DrinkInfo[LIQ_PORT] = new drinkInfo(8, -1,  5, "<R>red<1>", "<R>port<1>");
  DrinkInfo[LIQ_MUSHROOM_ALE] = new drinkInfo(7, -1,  5, "<g>green<1>", "<g>mushroom<1><o> ale<1>");
  DrinkInfo[LIQ_VOMIT] = new drinkInfo(8, -1,  5, "<G>light green<1>", "<G>v<o>o<G>m<o>i<G>t<1>");
  DrinkInfo[LIQ_COLA] = new drinkInfo(-1, 2, 5, "<o>brown<1>", "<o>cola<1>");
}
