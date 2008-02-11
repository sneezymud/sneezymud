#include "stdsneezy.h"
#include "liquids.h"

liqEntry *liqInfoT::operator[] (const liqTypeT i)
{
  if(liquids.find(i) == liquids.end()){
    vlogf(LOG_BUG, fmt("invalid liquid detected: %i") % i);
    return liquids[LIQ_WATER];
  } else {
    return liquids[i];
  }
}

liqInfoT::~liqInfoT()
{
}

liqInfoT::liqInfoT()
{
  liquids[LIQ_WATER]                = new liqEntry(0,  0, 10, false, false, "clear", "<c>water<1>", 0);
  liquids[LIQ_BEER]                 = new liqEntry(5, -2,  7, false, false, "<o>brown<1>", "<o>beer<1>", 0);
  liquids[LIQ_WINE]                 = new liqEntry(4, -1,  6, false, false, "clear", "<W>white wine<1>", 0);
  liquids[LIQ_ALE]                  = new liqEntry(6, -3,  5, false, false, "<o>brown<1>", "<o>ale<1>", 0);
  liquids[LIQ_DARKALE]              = new liqEntry(5, -1,  5, false, false, "<k>dark<1>", "<k>dark<1> <o>ale<1>", 0);
  liquids[LIQ_WHISKY]               = new liqEntry(10,  0,  1, false, false, "<y>golden<1>", "<y>whiskey<1>", 0);
  liquids[LIQ_LEMONADE]             = new liqEntry(0,  1,  8, false, false, "<y>golden<1>", "<y>lemonade<1>", 0);
  liquids[LIQ_FIREBRT]              = new liqEntry(15,  0, -3, false, false, "<g>green<1>", "<g>firebreather<1>", 0);
  liquids[LIQ_LOCALSPC]             = new liqEntry(8, -1,  2, false, false, "clear", "local special", 0);
  liquids[LIQ_SLIME]                = new liqEntry(0,  1,  8, false, false, "<G>light green<1>", "<G>juice<1>", 0);
  liquids[LIQ_MILK]                 = new liqEntry(0,  2,  6, false, false, "<W>white<1>", "<W>milk<1>", 0);
  liquids[LIQ_TEA]                  = new liqEntry(-1, -1,  6, false, false, "<o>brown<1>", "<o>tea<1>", 0);
  liquids[LIQ_COFFEE]               = new liqEntry(-2, -3,  5, true, false, "<k>black<1>", "<k>coffee<1>", 0);
  liquids[LIQ_BLOOD]                = new liqEntry(0,  2, -1, true, false, "<r>red<1>", "<r>blood<1>", 0);
  liquids[LIQ_SALTWATER]            = new liqEntry(0,  1, -5, false, false, "clear", "salt water", 0);
  liquids[LIQ_MEAD]                 = new liqEntry(2,  2,  4, false, false, "<k>black<1>", "<k>mead<1>", 0);
  liquids[LIQ_VODKA]                = new liqEntry(12, -3, -1, false, false, "clear", "vodka", 0);
  liquids[LIQ_RUM]                  = new liqEntry(11, -3, -1, false, false, "clear", "rum", 0);
  liquids[LIQ_TEQUILA]              = new liqEntry(13, -3, -1, false, false, "golden", "tequila", 0);
  liquids[LIQ_BRANDY]               = new liqEntry(8,  1,  3, false, false, "<o>brown<1>", "<o>brandy<1>", 0);
  liquids[LIQ_RED_WINE]             = new liqEntry(7, -1,  6, false, false, "<R>red<1>", "<R>red wine<1>", 0);
  liquids[LIQ_WARM_MEAD]            = new liqEntry(2,  1,  5, false, false, "<k>black<1>", "<k>warm mead<1>", 0);
  liquids[LIQ_CHAMPAGNE]            = new liqEntry(6, -2,  4, false, false, "bubbly, translucent", "champagne", 0);
  liquids[LIQ_HOLYWATER]            = new liqEntry(0,  1, 10, true, false, "clear", "holy water", 0);
  liquids[LIQ_PORT]                 = new liqEntry(8, -1,  5, false, false, "<R>red<1>", "<R>port<1>", 0);
  liquids[LIQ_MUSHROOM_ALE]         = new liqEntry(7, -1,  5, false, false, "<g>green<1>", "<g>mushroom<1><o> ale<1>", 0);
  liquids[LIQ_VOMIT]                = new liqEntry(8, -1,  5, false, false, "<G>light green<1>", "<G>v<o>o<G>m<o>i<G>t<1>", 0);
  liquids[LIQ_COLA]                 = new liqEntry(-1, 2, 5, false, false, "<o>brown<1>", "<o>cola<1>", 0);
  liquids[LIQ_STRAWBERRY_MARGARITA] = new liqEntry(-1, 2, 5, false, false, "<r>smooth, pink<1>", "<r>strawberry margarita<1>", 0);
  liquids[LIQ_BLUE_MARGARITA]       = new liqEntry(-1, 2, 5, false, false, "<b>smooth, blue<1>", "<b>blue margarita<1>", 0);
  liquids[LIQ_GOLD_MARGARITA]       = new liqEntry(-1, 2, 5, false, false, "<Y>smooth, golden<1>", "<Y>gold margarita<1>", 0);
  liquids[LIQ_STRAWBERRY_DAIQUIRI]  = new liqEntry(-1, 2, 5, false, false, "<r>pink, frothy<1>", "<r>strawberry daiquiri<1>", 0);
  liquids[LIQ_BANANA_DAIQUIRI]      = new liqEntry(-1, 2, 5, false, false, "<Y>yellow, frothy<1>", "<Y>banana daiquiri<1>", 0);
  liquids[LIQ_PINA_COLADA]          = new liqEntry(-1, 2, 5, false, false, "<W>frothy, white<1>", "<W>pina colada<1>", 0);
  liquids[LIQ_TEQUILA_SUNRISE]      = new liqEntry(-1, 2, 5, false, false, "<o>smooth, yellow<1>", "<o>tequila sunrise<1>", 0);
  liquids[LIQ_ISLA_VERDE]           = new liqEntry(-1, 2, 5, false, false, "<g>green, frothy<1>", "<g>isla verde<1>", 0);
  liquids[LIQ_POT_CURE_POISON]      = new liqEntry(-1, -1, 1, true, false, "<W>white<1>", "<W>white potion<1>", 68);
  liquids[LIQ_POT_HEAL_LIGHT]       = new liqEntry(-1, -1, 1, true, false, "<W>silver<1>", "<W>silver potion<1>", 72);
  liquids[LIQ_POT_HEAL_CRIT]        = new liqEntry(-1, -1, 1, true, false, "<Y>golden<1>", "<Y>golden potion<1>", 331);
  liquids[LIQ_POT_HEAL]             = new liqEntry(-1, -1, 1, true, false, "<b>blue<1>", "<b>blue potion<1>", 1276);
  liquids[LIQ_POT_SANCTUARY]        = new liqEntry(-1, -1, 1, true, false, "<c>platinum<1>", "<c>platinum potion<1>", 1846);
  liquids[LIQ_POT_FLIGHT]           = new liqEntry(-1, -1, 1, true, false, "<c>cyan<1>", "<c>cyan potion<1>", 6160);
  liquids[LIQ_POT_BIND]           = new liqEntry(-1, -1, 1, true, false, "<R>light pink<1>", "<R>light pink potion<1>", 648);
  liquids[LIQ_POT_BLINDNESS]           = new liqEntry(-1, -1, 1, true, false, "<P>magenta<1>", "<P>magenta potion<1>", 1248);
  liquids[LIQ_POT_ARMOR]           = new liqEntry(-1, -1, 1, true, false, "<k>grey<1>", "<k>grey potion<1>", 24);
  liquids[LIQ_POT_REFRESH]           = new liqEntry(-1, -1, 1, true, false, "<k>black<1>", "<k>black potion<1>", 76);
  liquids[LIQ_POT_SECOND_WIND]           = new liqEntry(-1, -1, 1, true, false, "<o>bronze<1>", "<o>bronze potion<1>", 759);
  liquids[LIQ_POT_CURSE]           = new liqEntry(-1, -1, 1, true, false, "<r>maroon<1>", "<r>maroon potion<1>", 15);
  liquids[LIQ_POT_DETECT_INVIS]           = new liqEntry(-1, -1, 1, true, false, "<Y>chartreuse<1>", "<Y>chartreuse potion<1>", 92);
  liquids[LIQ_POT_BLESS]           = new liqEntry(-1, -1, 1, true, false, "<o>orange<1>", "<o>orange potion<1>", 26);
  liquids[LIQ_POT_INVIS]           = new liqEntry(-1, -1, 1, true, false, "<g>puce<1>", "<g>puce potion<1>", 539);
  liquids[LIQ_POT_HEAL_FULL]           = new liqEntry(-1, -1, 1, true, false, "<r>vermilion<1>", "<r>vermilion potion<1>", 3168);
  liquids[LIQ_POT_SUFFOCATE]           = new liqEntry(-1, -1, 1, true, false, "translucent white", "translucent white potion", 1896);
  liquids[LIQ_POT_FEATHERY_DESCENT]           = new liqEntry(-1, -1, 1, true, false, "dull white", "dull white potion", 92);
  liquids[LIQ_POT_DETECT_MAGIC]           = new liqEntry(-1, -1, 1, true, false, "<B>bright blue<1>", "<B>bright blue potion<1>", 72);
  liquids[LIQ_POT_DISPEL_MAGIC]           = new liqEntry(-1, -1, 1, true, false, "<R>bright red<1>", "<R>bright red potion<1>", 268);
  liquids[LIQ_POT_STONE_SKIN]           = new liqEntry(-1, -1, 1, true, false, "<k>coarse grey<1>", "<k>coarse grey potion<1>", 820);
  liquids[LIQ_POT_TRAIL_SEEK]           = new liqEntry(-1, -1, 1, true, false, "<o>light brown<1>", "<o>light brown potion<1>", 712);
  liquids[LIQ_POT_FAERIE_FIRE]           = new liqEntry(-1, -1, 1, true, false, "<p>purple<1>", "<p>purple potion<1>", 30);
  liquids[LIQ_POT_FLAMING_FLESH]           = new liqEntry(-1, -1, 1, true, false, "<r>crimson<1>", "<r>crimson potion<1>", 820);
  liquids[LIQ_POT_CONJURE_ELE_EARTH]           = new liqEntry(-1, -1, 1, true, false, "<k>black<1>", "<k>black potion<1>", 268);
  liquids[LIQ_POT_SENSE_LIFE]           = new liqEntry(-1, -1, 1, true, false, "<o>stained orange<1>", "<o>stained orange potion<1>", 72);
  liquids[LIQ_POT_STEALTH]           = new liqEntry(-1, -1, 1, true, false, "<k>smooth grey<1>", "<k>smooth grey potion<1>", 276);
  liquids[LIQ_POT_TRUE_SIGHT]           = new liqEntry(-1, -1, 1, true, false, "<W>pure white<1>", "<W>pure white potion<1>", 712);
  liquids[LIQ_POT_ACCELERATE]           = new liqEntry(-1, -1, 1, true, false, "<W>shiny<1>", "<w>shiny potion<1>", 138);
  liquids[LIQ_POT_INFRAVISION]           = new liqEntry(-1, -1, 1, true, false, "<r>stained red<1>", "<r>stained red potion<1>", 234);
  liquids[LIQ_POT_SORC_GLOBE]           = new liqEntry(-1, -1, 1, true, false, "clear", "clear potion", 52);
  liquids[LIQ_POT_POISON]           = new liqEntry(-1, -1, 1, true, false, "<p>indigo<1>", "<p>indigo potion<1>", 60);
  liquids[LIQ_POT_BONE_BREAKER]           = new liqEntry(-1, -1, 1, true, false, "<k>charcoal gray<1>", "<k>charcoal gray potion<1>", 345);
  liquids[LIQ_POT_AQUALUNG]           = new liqEntry(-1, -1, 1, true, false, "<B>crystal blue<1>", "<B>crystal blue potion<1>", 3696);
  liquids[LIQ_POT_HASTE]           = new liqEntry(-1, -1, 1, true, false, "<g>effervescent<1>", "<g>effervescent potion<1>", 772);
  liquids[LIQ_POT_TELEPORT]           = new liqEntry(-1, -1, 1, true, false, "<k>gray<1>", "<k>gray potion<1>", 95);
  liquids[LIQ_POT_GILLS_OF_FLESH]           = new liqEntry(-1, -1, 1, true, false, "<B>light blue<1>", "<B>light blue potion<1>", 258);
  liquids[LIQ_POT_CURE_BLINDNESS]           = new liqEntry(-1, -1, 1, true, false, "<Y>sunny yellow<1>", "<Y>sunny yellow potion<1>", 235);
  liquids[LIQ_POT_CURE_DISEASE]           = new liqEntry(-1, -1, 1, true, false, "<P>chalky pink<1>", "<P>chalky pink potion<1>", 123);
  liquids[LIQ_POT_SHIELD_OF_MISTS]           = new liqEntry(-1, -1, 1, true, false, "<g>dark green<1>", "<g>dark green potion<1>", 3696);
  liquids[LIQ_POT_SENSE_PRESENCE]           = new liqEntry(-1, -1, 1, true, false, "translucent", "translucent potion", 3696);
  liquids[LIQ_POT_CHEVAL]           = new liqEntry(-1, -1, 1, true, false, "<W>pale white<1>", "<W>pale white potion<1>", 3696);
  liquids[LIQ_POT_DJALLAS_PROTECTION]           = new liqEntry(-1, -1, 1, true, false, "<k>steal grey<1>", "<k>steel grey potion<1>", 3696);
  liquids[LIQ_POT_LEGBAS_GUIDANCE]           = new liqEntry(-1, -1, 1, true, false, "<k>slate grey<1>", "<k>slate grey potion<1>", 3696);
  liquids[LIQ_POT_DETECT_SHADOW]           = new liqEntry(-1, -1, 1, true, false, "<o>sparkling amber<1>", "<o>sparkling amber potion<1>", 3696);
  liquids[LIQ_POT_CELERITE]           = new liqEntry(-1, -1, 1, true, false, "<r>rusty<1>", "<r>rusty potion<1>", 3696);
  liquids[LIQ_POT_CLARITY]           = new liqEntry(-1, -1, 1, true, false, "fizzling", "fizzling potion", 3696);
  liquids[LIQ_POT_BOILING_BLOOD]           = new liqEntry(-1, -1, 1, true, false, "<r>scarlet<1>", "<r>scarlet potion<1>", 3696);
  liquids[LIQ_POT_STUPIDITY]           = new liqEntry(-1, -1, 1, true, false, "transparent", "transparent potion", 3696);
  liquids[LIQ_POT_ENLIVEN]           = new liqEntry(-1, -1, 1, true, false, "<w>frothy<1> <o>brown<1>", "<w>frothy<1> <o>brown<1> potion", 3696);
  liquids[LIQ_POT_PLASMA_MIRROR]           = new liqEntry(-1, -1, 1, true, false, "<b>sp<B>ar<W>kl<w>y<1> <b>blue<1>", "<b>sp<B>ar<W>kl<w>y<1> <b>blue<1> potion", 4896);
  liquids[LIQ_POT_SLUMBER]           = new liqEntry(-1, -1, 1, true, false, "<W>milky white<1>", "<W>milky white potion<1>", 276);
  liquids[LIQ_POT_HEAL2]           = new liqEntry(-1, -1, 1, true, false, "<r>red<1>", "<r>red potion<1>", 1209);
  liquids[LIQ_POT_FEATHERY_DESCENT2]           = new liqEntry(-1, -1, 1, true, false, "<Y>yellow<1>", "<Y>yellow potion<1>", 92);
  liquids[LIQ_POT_SANCTUARY2]           = new liqEntry(-1, -1, 1, true, false, "<W>milky white<1>", "<W>milky white potion<1>", 1846);
  liquids[LIQ_POT_STONE_SKIN2]           = new liqEntry(-1, -1, 1, true, false, "<w>granite coloured<1>", "<w>granite coloured potion<1>", 1094);
  liquids[LIQ_POT_INFRAVISION2]           = new liqEntry(-1, -1, 1, true, false, "<o>dark orange<1>", "<o>dark orange potion<1>", 224);
  liquids[LIQ_POT_HEAL_LIGHT2]           = new liqEntry(-1, -1, 1, true, false, "clear", "clear potion", 72);
  liquids[LIQ_POT_GILLS_OF_FLESH2]           = new liqEntry(-1, -1, 1, true, false, "<b>deep blue<1>", "<b>deep blue potion<1>", 318);
  liquids[LIQ_POT_CELERITE2]           = new liqEntry(-1, -1, 1, true, false, "<p>purple<1>", "<p>purple potion<1>", 3696);
  liquids[LIQ_POT_CELERITE3]           = new liqEntry(-1, -1, 1, true, false, "<g>sickly green<1>", "<g>sickley green potion<1>", 3696);
  liquids[LIQ_POT_TELEPORT2]           = new liqEntry(-1, -1, 1, true, false, "<r>blood red<1>", "<r>blood red potion<1>", 114);
  liquids[LIQ_POT_BLESS2]           = new liqEntry(-1, -1, 1, true, false, "<Y>dark yellow<1>", "<Y>dark yellow potion<1>", 26);
  liquids[LIQ_POT_SECOND_WIND2]           = new liqEntry(-1, -1, 1, true, false, "<o>bronze<1>", "<o>bronze potion<1>", 759);
  liquids[LIQ_POT_MULTI1]           = new liqEntry(-1, -1, 1, true, false, "<Y>yellowish amber<1>", "<Y>yellowish amber potion<1>", 532);
  liquids[LIQ_POT_MULTI2]           = new liqEntry(-1, -1, 1, true, false, "<g>greenish amber<1>", "<g>greenish amber potion<1>", 1664);
  liquids[LIQ_POT_MULTI3]           = new liqEntry(-1, -1, 1, true, false, "<r>ruddy amber<1>", "<r>ruddy amber potion<1>", 2024);
  liquids[LIQ_POT_MULTI4]           = new liqEntry(-1, -1, 1, true, false, "<b>dark blue<1>", "<b>dark blue potion<1>", 1288);
  liquids[LIQ_POT_MULTI5]           = new liqEntry(-1, -1, 1, true, false, "<r>shadowy crimson<1>", "<r>shadowy crimson potion<1>", 714);
  liquids[LIQ_POT_MULTI6]           = new liqEntry(-1, -1, 1, true, false, "<p>mauve<1>", "<p>mauve potion<1>", 1861);
  liquids[LIQ_POT_MULTI7]           = new liqEntry(-1, -1, 1, true, false, "<g>rancid green<1>", "<g>rancid green potion<1>", 1733);
  liquids[LIQ_POT_MULTI8]           = new liqEntry(-1, -1, 1, true, false, "<r>rancid red<1>", "<r>rancid red potion<1>", 1882);
  liquids[LIQ_POT_MULTI9]           = new liqEntry(-1, -1, 1, true, false, "<o>rancid brown<1>", "<o>rancid brown potion<1>", 2891);
  liquids[LIQ_POT_MULTI10]           = new liqEntry(-1, -1, 1, true, false, "<k>silvery grey<1>", "<k>silvery grey potion<1>", 1900);
  liquids[LIQ_POT_MULTI11]           = new liqEntry(-1, -1, 1, true, false, "<r>ruby<1>", "<r>ruby potion<1>", 2721);
  liquids[LIQ_POT_YOUTH]             = new liqEntry(-1, -1, 1, true, false, "<o>glowing<1> <p>purple<1>", "<p>youth potion<1>", 20000);
  liquids[LIQ_POT_STAT]             = new liqEntry(-1, -1, 1, true, false, "<o>glowing<1> <p>purple<1>", "<p>characteristics potion<1>", 100000);
  liquids[LIQ_POT_LEARNING]             = new liqEntry(-1, -1, 1, true, false, "<o>glowing<1> <p>purple<1>", "<p>learning potion<1>", 100000);
  liquids[LIQ_POISON_STANDARD] = new liqEntry(-1,-1,1,true, true, "<p>purple<1>", "<p>contact poison<1>", 90);
  liquids[LIQ_POISON_CAMAS] = new liqEntry(-1,-1,1,true, true, "<g>green<1>", "<g>Death Camas extract<1>", 750);
  liquids[LIQ_POISON_ANGEL] = new liqEntry(-1,-1,1,true, true, "<W>white<1>", "<W>Destroying Angel extract<1>", 350);
  liquids[LIQ_POISON_JIMSON] = new liqEntry(-1,-1,1,true, true, "<o>orange<1>", "<o>Jimson Weed extract<1>", 350);
  liquids[LIQ_POISON_HEMLOCK] = new liqEntry(-1,-1,1,true, true, "<o>reddish-brown<1>", "<o>Hemlock extract<1>", 350);
  liquids[LIQ_POISON_MONKSHOOD] = new liqEntry(-1,-1,1,true, true, "<b>blue<1>", "<b>Monkshood extract<1>", 50);
  liquids[LIQ_POISON_GLOW_FISH] = new liqEntry(-1,-1,1,true, true, "<r>glowing red<1>", "<r>Glow Fish extract<1>", 150);
  liquids[LIQ_POISON_SCORPION] = new liqEntry(-1,-1,1,true, true, "<o>smooth orange<1>", "<o>liquor of scorpion<1>", 350);
  liquids[LIQ_POISON_VIOLET_FUNGUS] = new liqEntry(-1,-1,1,true, true, "<p>violet<1>", "<p>Violet Fungus Spore extract<1>", 350);
  liquids[LIQ_POISON_DEVIL_ICE] = new liqEntry(-1,-1,1,true, true, "<W>shiny white<1>", "<W>Ice Devil Serum<1>", 150);
  liquids[LIQ_POISON_FIREDRAKE] = new liqEntry(-1,-1,1,true, true, "<r>glimmering red<1>", "<r>Firedrake Serum<1>", 150);
  liquids[LIQ_POISON_INFANT] = new liqEntry(-1,-1,1,true, true, "<r>dull red<1>", "<r>ichor of infant<1>", 150);
  liquids[LIQ_POISON_PEA_SEED] = new liqEntry(-1,-1,1,true, true, "<g>dull green<1>", "<g>Sweet Pea Seed extract<1>", 50);
  liquids[LIQ_POISON_ACACIA] = new liqEntry(-1,-1,1,true, true, "<o>dull yellow<1>", "<o>Acacia extract<1>", 150);
  liquids[LIQ_LUBRICATION] = new liqEntry(-1,-1,1,false, false, "<c>clear jelly<1>", "<c>lubricant<1>", 100);
  liquids[LIQ_MAGICAL_ELIXIR] = new liqEntry(-1,-1,1,true,false, "<o>glowing<1> clear", "<p>magical elixir<1>", 70);
  liquids[LIQ_URINE] = new liqEntry(0,1,-5,false, false, "<Y>golden<1>", "<Y>urine<1>", 0);
  liquids[LIQ_POT_HEALING_GRASP]               = new liqEntry(100,  0,  1, true, false, "<p>light purple<1>", "<y>a light purple potion<1>", 0);
  liquids[LIQ_POT_CLEANSE]               = new liqEntry(100,  0,  1, true, false, "clear with clumps", "a clear potion with clumps", 0);
  liquids[LIQ_POT_QUICKSILVER]            = new liqEntry(-1,  -1,  -1, true, false, "shiny silver", "a shiny silver potion", 9999);  
  liquids[LIQ_POT_MYSTERY]                = new liqEntry(-1,-1,1, true, false, "<G>bright glowing green<1>", "<G>mystery potion<1>", 20000);
  liquids[LIQ_MUD] = new liqEntry(-1,-1,1,false, false, "<o>brown<1>", "<o>mud<1>", 0);
  liquids[LIQ_POT_FILTH] = new liqEntry(-1,-1,1,true, false, "<o>filthy<1>", "<o>filth<1>", 0);
  liquids[LIQ_GUANO] = new liqEntry(-1,-1,1,false, false, "<o>dirty white<1>", "<o>guano<1>", 0);
}



liqEntry::liqEntry(int d, int h, int t, bool p, bool x, const char *col, const char *n, int pr) :
  drunk(d),
  hunger(h),
  thirst(t),
  potion(p),
  poison(x),
  color(col),
  name(n),
  price(pr)
{
}

liqEntry & liqEntry::operator = (const liqEntry &a) 
{
  if (this == &a) return *this;

  drunk = a.drunk;
  hunger = a.hunger;
  thirst = a.thirst;
  potion = a.potion;
  poison = a.poison;
  color = a.color;
  name = a.name;
  price = a.price;

  return *this;
}

liqEntry::~liqEntry()
{
}



