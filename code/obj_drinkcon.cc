//////////////////////////////////////////////////////////////////////////
//
// SneezyMUD - All rights reserved, SneezyMUD Coding Team
//
// $Log: obj_drinkcon.cc,v $
// Revision 5.6  2002/11/29 20:04:03  peel
// added a few more potion liquids
//
// Revision 5.5  2002/11/29 00:11:47  peel
// added framework for true liquid potions
//
// Revision 5.4  2002/07/15 16:58:34  dash
// fixed a crash bug
//
// Revision 5.3  2002/01/11 00:16:47  peel
// splitting obj2.h etc
// almost done
//
// Revision 5.2  2002/01/08 21:05:12  peel
// removed the TBaseContainer hierarchy from obj2.h
// added header files for those objects
// inserted appropriate includes
//
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
#include "obj_base_container.h"
#include "obj_drinkcon.h"

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

drinkInfo::drinkInfo(int d, int h, int t, bool p, const char *col, const char *n) :
  drunk(d),
  hunger(h),
  thirst(t),
  potion(p),
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
  potion = a.potion;
  color = a.color;
  name = a.name;

  return *this;
}

drinkInfo::~drinkInfo()
{
}

void assign_drink_types()
{
  DrinkInfo[LIQ_WATER]                = new drinkInfo(0,  0, 10, false, "clear", "<c>water<1>");
  DrinkInfo[LIQ_BEER]                 = new drinkInfo(5, -2,  7, false, "<o>brown<1>", "<o>beer<1>");
  DrinkInfo[LIQ_WINE]                 = new drinkInfo(4, -1,  6, false, "clear", "<W>white wine<1>");
  DrinkInfo[LIQ_ALE]                  = new drinkInfo(6, -3,  5, false, "<o>brown<1>", "<o>ale<1>");
  DrinkInfo[LIQ_DARKALE]              = new drinkInfo(5, -1,  5, false, "<k>dark<1>", "<k>dark<1> <o>ale<1>");
  DrinkInfo[LIQ_WHISKY]               = new drinkInfo(10,  0,  1, false, "<y>golden<1>", "<y>whiskey<1>");
  DrinkInfo[LIQ_LEMONADE]             = new drinkInfo(0,  1,  8, false, "<y>golden<1>", "<y>lemonade<1>");
  DrinkInfo[LIQ_FIREBRT]              = new drinkInfo(15,  0, -3, false, "<g>green<1>", "<g>firebreather<1>");
  DrinkInfo[LIQ_LOCALSPC]             = new drinkInfo(8, -1,  2, false, "clear", "local special");
  DrinkInfo[LIQ_SLIME]                = new drinkInfo(0,  1,  8, false, "<G>light green<1>", "<G>juice<1>");
  DrinkInfo[LIQ_MILK]                 = new drinkInfo(0,  2,  6, false, "<W>white<1>", "<W>milk<1>");
  DrinkInfo[LIQ_TEA]                  = new drinkInfo(-1, -1,  6, false, "<o>brown<1>", "<o>tea<1>");
  DrinkInfo[LIQ_COFFEE]               = new drinkInfo(-2, -3,  5, false, "<k>black<1>", "<k>coffee<1>");
  DrinkInfo[LIQ_BLOOD]                = new drinkInfo(0,  2, -1, false, "<r>red<1>", "<r>blood<1>");
  DrinkInfo[LIQ_SALTWATER]            = new drinkInfo(0,  1, -5, false, "clear", "salt water");
  DrinkInfo[LIQ_MEAD]                 = new drinkInfo(2,  2,  4, false, "<k>black<1>", "<k>mead<1>");
  DrinkInfo[LIQ_VODKA]                = new drinkInfo(12, -3, -1, false, "clear", "vodka");
  DrinkInfo[LIQ_RUM]                  = new drinkInfo(11, -3, -1, false, "clear", "rum");
  DrinkInfo[LIQ_BRANDY]               = new drinkInfo(8,  1,  3, false, "<o>brown<1>", "<o>brandy<1>");
  DrinkInfo[LIQ_RED_WINE]             = new drinkInfo(7, -1,  6, false, "<R>red<1>", "<R>red wine<1>");
  DrinkInfo[LIQ_WARM_MEAD]            = new drinkInfo(2,  1,  5, false, "<k>black<1>", "<k>warm mead<1>");
  DrinkInfo[LIQ_CHAMPAGNE]            = new drinkInfo(6, -2,  4, false, "bubbly, translucent", "champagne");
  DrinkInfo[LIQ_HOLYWATER]            = new drinkInfo(0,  1, -5, false, "clear", "holy water");
  DrinkInfo[LIQ_PORT]                 = new drinkInfo(8, -1,  5, false, "<R>red<1>", "<R>port<1>");
  DrinkInfo[LIQ_MUSHROOM_ALE]         = new drinkInfo(7, -1,  5, false, "<g>green<1>", "<g>mushroom<1><o> ale<1>");
  DrinkInfo[LIQ_VOMIT]                = new drinkInfo(8, -1,  5, false, "<G>light green<1>", "<G>v<o>o<G>m<o>i<G>t<1>");
  DrinkInfo[LIQ_COLA]                 = new drinkInfo(-1, 2, 5, false, "<o>brown<1>", "<o>cola<1>");
  DrinkInfo[LIQ_STRAWBERRY_MARGARITA] = new drinkInfo(-1, 2, 5, false, "<r>smooth, pink<1>", "<r>strawberry margarita<1>");
  DrinkInfo[LIQ_BLUE_MARGARITA]       = new drinkInfo(-1, 2, 5, false, "<b>smooth, blue<1>", "<b>blue margarita<1>");
  DrinkInfo[LIQ_GOLD_MARGARITA]       = new drinkInfo(-1, 2, 5, false, "<Y>smooth, golden<1>", "<Y>gold margarita<1>");
  DrinkInfo[LIQ_STRAWBERRY_DAIQUIRI]  = new drinkInfo(-1, 2, 5, false, "<r>pink, frothy<1>", "<r>strawberry daiquiri<1>");
  DrinkInfo[LIQ_BANANA_DAIQUIRI]      = new drinkInfo(-1, 2, 5, false, "<Y>yellow, frothy<1>", "<Y>banana daiquiri<1>");
  DrinkInfo[LIQ_PINA_COLADA]          = new drinkInfo(-1, 2, 5, false, "<W>frothy, white<1>", "<W>pina colada<1>");
  DrinkInfo[LIQ_TEQUILA_SUNRISE]      = new drinkInfo(-1, 2, 5, false, "<o>smooth, yellow<1>", "<o>tequila sunrise<1>");
  DrinkInfo[LIQ_ISLA_VERDE]           = new drinkInfo(-1, 2, 5, false, "<g>green, frothy<1>", "<g>isla verde<1>");
  DrinkInfo[LIQ_POT_CURE_POISON]      = new drinkInfo(-1, -1, 1, true, "<W>white<1>", "<W>white potion<1>");
  DrinkInfo[LIQ_POT_HEAL_LIGHT]       = new drinkInfo(-1, -1, 1, true, "<W>silver<1>", "<W>silver potion<1>");
  DrinkInfo[LIQ_POT_HEAL_CRIT]        = new drinkInfo(-1, -1, 1, true, "<Y>golden<1>", "<Y>golden potion<1>");
  DrinkInfo[LIQ_POT_HEAL]             = new drinkInfo(-1, -1, 1, true, "<b>blue<1>", "<b>blue potion<1>");
  DrinkInfo[LIQ_POT_SANCTUARY]        = new drinkInfo(-1, -1, 1, true, "<c>platinum<1>", "<c>platinum potion<1>");
  DrinkInfo[LIQ_POT_FLIGHT]           = new drinkInfo(-1, -1, 1, true, "<c>cyan<1>", "<c>cyan potion<1>");
}



