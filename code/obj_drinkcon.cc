//////////////////////////////////////////////////////////////////////////
//
// SneezyMUD - All rights reserved, SneezyMUD Coding Team
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

drinkInfo::drinkInfo(int d, int h, int t, bool p, const char *col, const char *n, int pr) :
  drunk(d),
  hunger(h),
  thirst(t),
  potion(p),
  color(col),
  name(n),
  price(pr)
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
  price = a.price;

  return *this;
}

drinkInfo::~drinkInfo()
{
}

void assign_drink_types()
{
  DrinkInfo[LIQ_WATER]                = new drinkInfo(0,  0, 10, false, "clear", "<c>water<1>", 0);
  DrinkInfo[LIQ_BEER]                 = new drinkInfo(5, -2,  7, false, "<o>brown<1>", "<o>beer<1>", 0);
  DrinkInfo[LIQ_WINE]                 = new drinkInfo(4, -1,  6, false, "clear", "<W>white wine<1>", 0);
  DrinkInfo[LIQ_ALE]                  = new drinkInfo(6, -3,  5, false, "<o>brown<1>", "<o>ale<1>", 0);
  DrinkInfo[LIQ_DARKALE]              = new drinkInfo(5, -1,  5, false, "<k>dark<1>", "<k>dark<1> <o>ale<1>", 0);
  DrinkInfo[LIQ_WHISKY]               = new drinkInfo(10,  0,  1, false, "<y>golden<1>", "<y>whiskey<1>", 0);
  DrinkInfo[LIQ_LEMONADE]             = new drinkInfo(0,  1,  8, false, "<y>golden<1>", "<y>lemonade<1>", 0);
  DrinkInfo[LIQ_FIREBRT]              = new drinkInfo(15,  0, -3, false, "<g>green<1>", "<g>firebreather<1>", 0);
  DrinkInfo[LIQ_LOCALSPC]             = new drinkInfo(8, -1,  2, false, "clear", "local special", 0);
  DrinkInfo[LIQ_SLIME]                = new drinkInfo(0,  1,  8, false, "<G>light green<1>", "<G>juice<1>", 0);
  DrinkInfo[LIQ_MILK]                 = new drinkInfo(0,  2,  6, false, "<W>white<1>", "<W>milk<1>", 0);
  DrinkInfo[LIQ_TEA]                  = new drinkInfo(-1, -1,  6, false, "<o>brown<1>", "<o>tea<1>", 0);
  DrinkInfo[LIQ_COFFEE]               = new drinkInfo(-2, -3,  5, false, "<k>black<1>", "<k>coffee<1>", 0);
  DrinkInfo[LIQ_BLOOD]                = new drinkInfo(0,  2, -1, false, "<r>red<1>", "<r>blood<1>", 0);
  DrinkInfo[LIQ_SALTWATER]            = new drinkInfo(0,  1, -5, false, "clear", "salt water", 0);
  DrinkInfo[LIQ_MEAD]                 = new drinkInfo(2,  2,  4, false, "<k>black<1>", "<k>mead<1>", 0);
  DrinkInfo[LIQ_VODKA]                = new drinkInfo(12, -3, -1, false, "clear", "vodka", 0);
  DrinkInfo[LIQ_RUM]                  = new drinkInfo(11, -3, -1, false, "clear", "rum", 0);
  DrinkInfo[LIQ_BRANDY]               = new drinkInfo(8,  1,  3, false, "<o>brown<1>", "<o>brandy<1>", 0);
  DrinkInfo[LIQ_RED_WINE]             = new drinkInfo(7, -1,  6, false, "<R>red<1>", "<R>red wine<1>", 0);
  DrinkInfo[LIQ_WARM_MEAD]            = new drinkInfo(2,  1,  5, false, "<k>black<1>", "<k>warm mead<1>", 0);
  DrinkInfo[LIQ_CHAMPAGNE]            = new drinkInfo(6, -2,  4, false, "bubbly, translucent", "champagne", 0);
  DrinkInfo[LIQ_HOLYWATER]            = new drinkInfo(0,  1, -5, true, "clear", "holy water", 0);
  DrinkInfo[LIQ_PORT]                 = new drinkInfo(8, -1,  5, false, "<R>red<1>", "<R>port<1>", 0);
  DrinkInfo[LIQ_MUSHROOM_ALE]         = new drinkInfo(7, -1,  5, false, "<g>green<1>", "<g>mushroom<1><o> ale<1>", 0);
  DrinkInfo[LIQ_VOMIT]                = new drinkInfo(8, -1,  5, false, "<G>light green<1>", "<G>v<o>o<G>m<o>i<G>t<1>", 0);
  DrinkInfo[LIQ_COLA]                 = new drinkInfo(-1, 2, 5, false, "<o>brown<1>", "<o>cola<1>", 0);
  DrinkInfo[LIQ_STRAWBERRY_MARGARITA] = new drinkInfo(-1, 2, 5, false, "<r>smooth, pink<1>", "<r>strawberry margarita<1>", 0);
  DrinkInfo[LIQ_BLUE_MARGARITA]       = new drinkInfo(-1, 2, 5, false, "<b>smooth, blue<1>", "<b>blue margarita<1>", 0);
  DrinkInfo[LIQ_GOLD_MARGARITA]       = new drinkInfo(-1, 2, 5, false, "<Y>smooth, golden<1>", "<Y>gold margarita<1>", 0);
  DrinkInfo[LIQ_STRAWBERRY_DAIQUIRI]  = new drinkInfo(-1, 2, 5, false, "<r>pink, frothy<1>", "<r>strawberry daiquiri<1>", 0);
  DrinkInfo[LIQ_BANANA_DAIQUIRI]      = new drinkInfo(-1, 2, 5, false, "<Y>yellow, frothy<1>", "<Y>banana daiquiri<1>", 0);
  DrinkInfo[LIQ_PINA_COLADA]          = new drinkInfo(-1, 2, 5, false, "<W>frothy, white<1>", "<W>pina colada<1>", 0);
  DrinkInfo[LIQ_TEQUILA_SUNRISE]      = new drinkInfo(-1, 2, 5, false, "<o>smooth, yellow<1>", "<o>tequila sunrise<1>", 0);
  DrinkInfo[LIQ_ISLA_VERDE]           = new drinkInfo(-1, 2, 5, false, "<g>green, frothy<1>", "<g>isla verde<1>", 0);
  DrinkInfo[LIQ_POT_CURE_POISON]      = new drinkInfo(-1, -1, 1, true, "<W>white<1>", "<W>white potion<1>", 68);
  DrinkInfo[LIQ_POT_HEAL_LIGHT]       = new drinkInfo(-1, -1, 1, true, "<W>silver<1>", "<W>silver potion<1>", 72);
  DrinkInfo[LIQ_POT_HEAL_CRIT]        = new drinkInfo(-1, -1, 1, true, "<Y>golden<1>", "<Y>golden potion<1>", 331);
  DrinkInfo[LIQ_POT_HEAL]             = new drinkInfo(-1, -1, 1, true, "<b>blue<1>", "<b>blue potion<1>", 1276);
  DrinkInfo[LIQ_POT_SANCTUARY]        = new drinkInfo(-1, -1, 1, true, "<c>platinum<1>", "<c>platinum potion<1>", 1846);
  DrinkInfo[LIQ_POT_FLIGHT]           = new drinkInfo(-1, -1, 1, true, "<c>cyan<1>", "<c>cyan potion<1>", 6160);
  DrinkInfo[LIQ_POT_BIND]           = new drinkInfo(-1, -1, 1, true, "<R>light pink<1>", "<R>light pink potion<1>", 648);
  DrinkInfo[LIQ_POT_BLINDNESS]           = new drinkInfo(-1, -1, 1, true, "<P>magenta<1>", "<P>magenta potion<1>", 1248);
  DrinkInfo[LIQ_POT_ARMOR]           = new drinkInfo(-1, -1, 1, true, "<k>grey<1>", "<k>grey potion<1>", 24);
  DrinkInfo[LIQ_POT_REFRESH]           = new drinkInfo(-1, -1, 1, true, "<k>black<1>", "<k>black potion<1>", 76);
  DrinkInfo[LIQ_POT_SECOND_WIND]           = new drinkInfo(-1, -1, 1, true, "<o>bronze<1>", "<o>bronze potion<1>", 759);
  DrinkInfo[LIQ_POT_CURSE]           = new drinkInfo(-1, -1, 1, true, "<r>maroon<1>", "<r>maroon potion<1>", 15);
  DrinkInfo[LIQ_POT_DETECT_INVIS]           = new drinkInfo(-1, -1, 1, true, "<Y>chartreuse<1>", "<Y>chartreuse potion<1>", 92);
  DrinkInfo[LIQ_POT_BLESS]           = new drinkInfo(-1, -1, 1, true, "<o>orange<1>", "<o>orange potion<1>", 26);
  DrinkInfo[LIQ_POT_INVIS]           = new drinkInfo(-1, -1, 1, true, "<g>puce<1>", "<g>puce potion<1>", 539);
  DrinkInfo[LIQ_POT_HEAL_FULL]           = new drinkInfo(-1, -1, 1, true, "<r>vermilion<1>", "<r>vermilion potion<1>", 3168);
  DrinkInfo[LIQ_POT_SUFFOCATE]           = new drinkInfo(-1, -1, 1, true, "translucent white", "translucent white potion", 1896);
  DrinkInfo[LIQ_POT_FEATHERY_DESCENT]           = new drinkInfo(-1, -1, 1, true, "dull white", "dull white potion", 92);
  DrinkInfo[LIQ_POT_DETECT_MAGIC]           = new drinkInfo(-1, -1, 1, true, "<B>bright blue<1>", "<B>bright blue potion<1>", 72);
  DrinkInfo[LIQ_POT_DISPEL_MAGIC]           = new drinkInfo(-1, -1, 1, true, "<R>bright red<1>", "<R>bright red potion<1>", 268);
  DrinkInfo[LIQ_POT_STONE_SKIN]           = new drinkInfo(-1, -1, 1, true, "<k>coarse grey<1>", "<k>coarse grey potion<1>", 820);
  DrinkInfo[LIQ_POT_TRAIL_SEEK]           = new drinkInfo(-1, -1, 1, true, "<o>light brown<1>", "<o>light brown potion<1>", 712);
  DrinkInfo[LIQ_POT_FAERIE_FIRE]           = new drinkInfo(-1, -1, 1, true, "<p>purple<1>", "<p>purple potion<1>", 30);
  DrinkInfo[LIQ_POT_FLAMING_FLESH]           = new drinkInfo(-1, -1, 1, true, "<r>crimson<1>", "<r>crimson potion<1>", 820);
  DrinkInfo[LIQ_POT_CONJURE_ELE_EARTH]           = new drinkInfo(-1, -1, 1, true, "<k>black<1>", "<k>black potion<1>", 268);
  DrinkInfo[LIQ_POT_SENSE_LIFE]           = new drinkInfo(-1, -1, 1, true, "<o>stained orange<1>", "<o>stained orange potion<1>", 72);
  DrinkInfo[LIQ_POT_STEALTH]           = new drinkInfo(-1, -1, 1, true, "<k>smooth grey<1>", "<k>smooth grey potion<1>", 276);
  DrinkInfo[LIQ_POT_TRUE_SIGHT]           = new drinkInfo(-1, -1, 1, true, "<W>pure white<1>", "<W>pure white potion<1>", 712);
  DrinkInfo[LIQ_POT_ACCELERATE]           = new drinkInfo(-1, -1, 1, true, "<W>shiny<1>", "<w>shiny potion<1>", 138);
  DrinkInfo[LIQ_POT_INFRAVISION]           = new drinkInfo(-1, -1, 1, true, "<r>stained red<1>", "<r>stained red potion<1>", 234);
  DrinkInfo[LIQ_POT_SORC_GLOBE]           = new drinkInfo(-1, -1, 1, true, "clear", "clear potion", 52);
  DrinkInfo[LIQ_POT_POISON]           = new drinkInfo(-1, -1, 1, true, "<p>indigo<1>", "<p>indigo potion<1>", 60);
  DrinkInfo[LIQ_POT_BONE_BREAKER]           = new drinkInfo(-1, -1, 1, true, "<k>charcoal gray<1>", "<k>charcoal gray potion<1>", 345);
  DrinkInfo[LIQ_POT_AQUALUNG]           = new drinkInfo(-1, -1, 1, true, "<B>crystal blue<1>", "<B>crystal blue potion<1>", 3696);
  DrinkInfo[LIQ_POT_HASTE]           = new drinkInfo(-1, -1, 1, true, "<g>effervescent<1>", "<g>effervescent potion<1>", 772);
  DrinkInfo[LIQ_POT_TELEPORT]           = new drinkInfo(-1, -1, 1, true, "<k>gray<1>", "<k>gray potion<1>", 95);
  DrinkInfo[LIQ_POT_GILLS_OF_FLESH]           = new drinkInfo(-1, -1, 1, true, "<B>light blue<1>", "<B>light blue potion<1>", 258);
  DrinkInfo[LIQ_POT_CURE_BLINDNESS]           = new drinkInfo(-1, -1, 1, true, "<Y>sunny yellow<1>", "<Y>sunny yellow potion<1>", 235);
  DrinkInfo[LIQ_POT_CURE_DISEASE]           = new drinkInfo(-1, -1, 1, true, "<P>chalky pink<1>", "<P>chalky pink potion<1>", 123);
  DrinkInfo[LIQ_POT_SHIELD_OF_MISTS]           = new drinkInfo(-1, -1, 1, true, "<g>dark green<1>", "<g>dark green potion<1>", 3696);
  DrinkInfo[LIQ_POT_SENSE_PRESENCE]           = new drinkInfo(-1, -1, 1, true, "translucent", "translucent potion", 3696);
  DrinkInfo[LIQ_POT_CHEVAL]           = new drinkInfo(-1, -1, 1, true, "<W>pale white<1>", "<W>pale white potion<1>", 3696);
  DrinkInfo[LIQ_POT_DJALLAS_PROTECTION]           = new drinkInfo(-1, -1, 1, true, "<k>steal grey<1>", "<k>steel grey potion<1>", 3696);
  DrinkInfo[LIQ_POT_LEGBAS_GUIDANCE]           = new drinkInfo(-1, -1, 1, true, "<k>slate grey<1>", "<k>slate grey potion<1>", 3696);
  DrinkInfo[LIQ_POT_DETECT_SHADOW]           = new drinkInfo(-1, -1, 1, true, "<o>sparkling amber<1>", "<o>sparkling amber potion<1>", 3696);
  DrinkInfo[LIQ_POT_CELERITE]           = new drinkInfo(-1, -1, 1, true, "<r>rusty<1>", "<r>rusty potion<1>", 3696);
  DrinkInfo[LIQ_POT_CLARITY]           = new drinkInfo(-1, -1, 1, true, "fizzling", "fizzling potion", 3696);
  DrinkInfo[LIQ_POT_BOILING_BLOOD]           = new drinkInfo(-1, -1, 1, true, "<r>scarlet<1>", "<r>scarlet potion<1>", 3696);
  DrinkInfo[LIQ_POT_STUPIDITY]           = new drinkInfo(-1, -1, 1, true, "transparent", "transparent potion", 3696);
  DrinkInfo[LIQ_POT_SLUMBER]           = new drinkInfo(-1, -1, 1, true, "<W>milky white<1>", "<W>milky white potion<1>", 276);
  DrinkInfo[LIQ_POT_HEAL2]           = new drinkInfo(-1, -1, 1, true, "<r>red<1>", "<r>red potion<1>", 1209);
  DrinkInfo[LIQ_POT_FEATHERY_DESCENT2]           = new drinkInfo(-1, -1, 1, true, "<Y>yellow<1>", "<Y>yellow potion<1>", 92);
  DrinkInfo[LIQ_POT_SANCTUARY2]           = new drinkInfo(-1, -1, 1, true, "<W>milky white<1>", "<W>milky white potion<1>", 1846);
  DrinkInfo[LIQ_POT_STONE_SKIN2]           = new drinkInfo(-1, -1, 1, true, "<w>granite coloured<1>", "<w>granite coloured potion<1>", 1094);
  DrinkInfo[LIQ_POT_INFRAVISION2]           = new drinkInfo(-1, -1, 1, true, "<o>dark orange<1>", "<o>dark orange potion<1>", 224);
  DrinkInfo[LIQ_POT_HEAL_LIGHT2]           = new drinkInfo(-1, -1, 1, true, "clear", "clear potion", 72);
  DrinkInfo[LIQ_POT_GILLS_OF_FLESH2]           = new drinkInfo(-1, -1, 1, true, "<b>deep blue<1>", "<b>deep blue potion<1>", 318);
  DrinkInfo[LIQ_POT_CELERITE2]           = new drinkInfo(-1, -1, 1, true, "<p>purple<1>", "<p>purple potion<1>", 3696);
  DrinkInfo[LIQ_POT_CELERITE3]           = new drinkInfo(-1, -1, 1, true, "<g>sickly green<1>", "<g>sickley green potion<1>", 3696);
  DrinkInfo[LIQ_POT_TELEPORT2]           = new drinkInfo(-1, -1, 1, true, "<r>blood red<1>", "<r>blood red potion<1>", 114);
  DrinkInfo[LIQ_POT_BLESS2]           = new drinkInfo(-1, -1, 1, true, "<Y>dark yellow<1>", "<Y>dark yellow potion<1>", 26);
  DrinkInfo[LIQ_POT_SECOND_WIND2]           = new drinkInfo(-1, -1, 1, true, "<o>bronze<1>", "<o>bronze potion<1>", 759);
  DrinkInfo[LIQ_POT_MULTI1]           = new drinkInfo(-1, -1, 1, true, "<Y>yellowish amber<1>", "<Y>yellowish amber potion<1>", 532);
  DrinkInfo[LIQ_POT_MULTI2]           = new drinkInfo(-1, -1, 1, true, "<g>greenish amber<1>", "<g>greenish amber potion<1>", 1664);
  DrinkInfo[LIQ_POT_MULTI3]           = new drinkInfo(-1, -1, 1, true, "<r>ruddy amber<1>", "<r>ruddy amber potion<1>", 2024);
  DrinkInfo[LIQ_POT_MULTI4]           = new drinkInfo(-1, -1, 1, true, "<b>dark blue<1>", "<b>dark blue potion<1>", 1288);
  DrinkInfo[LIQ_POT_MULTI5]           = new drinkInfo(-1, -1, 1, true, "<r>shadowy crimson<1>", "<r>shadowy crimson potion<1>", 714);
  DrinkInfo[LIQ_POT_MULTI6]           = new drinkInfo(-1, -1, 1, true, "<p>mauve<1>", "<p>mauve potion<1>", 1861);
  DrinkInfo[LIQ_POT_MULTI7]           = new drinkInfo(-1, -1, 1, true, "<g>rancid green<1>", "<g>rancid green potion<1>", 1733);
  DrinkInfo[LIQ_POT_MULTI8]           = new drinkInfo(-1, -1, 1, true, "<r>rancid red<1>", "<r>rancid red potion<1>", 1882);
  DrinkInfo[LIQ_POT_MULTI9]           = new drinkInfo(-1, -1, 1, true, "<o>rancid brown<1>", "<o>rancid brown potion<1>", 2891);
  DrinkInfo[LIQ_POT_MULTI10]           = new drinkInfo(-1, -1, 1, true, "<k>silvery grey<1>", "<k>silvery grey potion<1>", 1900);
  DrinkInfo[LIQ_POT_MULTI11]           = new drinkInfo(-1, -1, 1, true, "<r>ruby<1>", "<r>ruby potion<1>", 2721);
  DrinkInfo[LIQ_POT_YOUTH]             = new drinkInfo(-1, -1, 1, true, "<o>glowing<1> <p>purple<1>", "<p>youth potion<1>", 20000);
  DrinkInfo[LIQ_POT_STAT]             = new drinkInfo(-1, -1, 1, true, "<o>glowing<1> <p>purple<1>", "<p>characteristics potion<1>", 100000);
  DrinkInfo[LIQ_POT_LEARNING]             = new drinkInfo(-1, -1, 1, true, "<o>glowing<1> <p>purple<1>", "<p>learning potion<1>", 100000);
  DrinkInfo[LIQ_POISON_STANDARD] = new drinkInfo(-1,-1,1,true, "<p>purple<1>", "<p>contact poison<1>", 90);
  DrinkInfo[LIQ_POISON_CAMAS] = new drinkInfo(-1,-1,1,true, "<g>green<1>", "<g>Death Camas extract<1>", 750);
  DrinkInfo[LIQ_POISON_ANGEL] = new drinkInfo(-1,-1,1,true, "<W>white<1>", "<W>Destroying Angel extract<1>", 350);
  DrinkInfo[LIQ_POISON_JIMSON] = new drinkInfo(-1,-1,1,true, "<o>orange<1>", "<o>Jimson Weed extract<1>", 350);
  DrinkInfo[LIQ_POISON_HEMLOCK] = new drinkInfo(-1,-1,1,true, "<o>reddish-brown<1>", "<o>Hemlock extract<1>", 350);
  DrinkInfo[LIQ_POISON_MONKSHOOD] = new drinkInfo(-1,-1,1,true, "<b>blue<1>", "<b>Monkshood extract<1>", 50);
  DrinkInfo[LIQ_POISON_GLOW_FISH] = new drinkInfo(-1,-1,1,true, "<r>glowing red<1>", "<r>Glow Fish extract<1>", 150);
  DrinkInfo[LIQ_POISON_SCORPION] = new drinkInfo(-1,-1,1,true, "<o>smooth orange<1>", "<o>liquor of scorpion<1>", 350);
  DrinkInfo[LIQ_POISON_VIOLET_FUNGUS] = new drinkInfo(-1,-1,1,true, "<p>violet<1>", "<p>Violet Fungus Spore extract<1>", 350);
  DrinkInfo[LIQ_POISON_DEVIL_ICE] = new drinkInfo(-1,-1,1,true, "<W>shiny white<1>", "<W>Ice Devil Serum<1>", 150);
  DrinkInfo[LIQ_POISON_FIREDRAKE] = new drinkInfo(-1,-1,1,true, "<r>glimmering red<1>", "<r>Firedrake Serum<1>", 150);
  DrinkInfo[LIQ_POISON_INFANT] = new drinkInfo(-1,-1,1,true, "<r>dull red<1>", "<r>ichor of infant<1>", 150);
  DrinkInfo[LIQ_POISON_PEA_SEED] = new drinkInfo(-1,-1,1,true, "<g>dull green<1>", "<g>Sweet Pea Seed extract<1>", 50);
  DrinkInfo[LIQ_POISON_ACACIA] = new drinkInfo(-1,-1,1,true, "<o>dull yellow<1>", "<o>Acacia extract<1>", 150);
  DrinkInfo[LIQ_LUBRICATION] = new drinkInfo(-1,-1,1,false, "<c>clear jelly<1>", "<c>lubricant<1>", 100);

}



