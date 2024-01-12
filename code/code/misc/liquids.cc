#include "liquids.h"

#include <boost/format.hpp>
#include <map>
#include <utility>

#include "log.h"
#include "sstring.h"

class liqInfoT_pimpl {
  public:
    std::map<liqTypeT, liqEntry> liquids;
};

const liqEntry* liqInfoT::operator[](const liqTypeT i) const {
  auto it = pimpl->liquids.find(i);
  if (it == pimpl->liquids.end()) {
    vlogf(LOG_BUG, format("invalid liquid detected: %i") % i);
    return &(*pimpl->liquids.find(LIQ_WATER)).second;
  } else {
    return &(*it).second;
  }
}

liqInfoT::~liqInfoT() { delete pimpl; }

liqInfoT::liqInfoT() {
  pimpl = new liqInfoT_pimpl();
  auto& liquids = pimpl->liquids;

  liquids.emplace_hint(liquids.end(), LIQ_WATER,
    liqEntry{0, 0, 10, false, false, "clear", "<c>water<1>", 0});
  liquids.emplace_hint(liquids.end(), LIQ_BEER,
    liqEntry{5, -2, 7, false, false, "<o>brown<1>", "<o>beer<1>", 0});
  liquids.emplace_hint(liquids.end(), LIQ_WINE,
    liqEntry{4, -1, 6, false, false, "clear", "<W>white wine<1>", 0});
  liquids.emplace_hint(liquids.end(), LIQ_ALE,
    liqEntry{6, -3, 5, false, false, "<o>brown<1>", "<o>ale<1>", 0});
  liquids.emplace_hint(liquids.end(), LIQ_DARKALE,
    liqEntry{5, -1, 5, false, false, "<k>dark<1>", "<k>dark<1> <o>ale<1>", 0});
  liquids.emplace_hint(liquids.end(), LIQ_WHISKY,
    liqEntry{10, 0, 1, false, false, "<y>golden<1>", "<y>whiskey<1>", 0});
  liquids.emplace_hint(liquids.end(), LIQ_LEMONADE,
    liqEntry{0, 1, 8, false, false, "<y>golden<1>", "<y>lemonade<1>", 0});
  liquids.emplace_hint(liquids.end(), LIQ_FIREBRT,
    liqEntry{15, 0, -3, false, false, "<g>green<1>", "<g>firebreather<1>", 0});
  liquids.emplace_hint(liquids.end(), LIQ_LOCALSPC,
    liqEntry{8, -1, 2, false, false, "clear", "local special", 0});
  liquids.emplace_hint(liquids.end(), LIQ_SLIME,
    liqEntry{0, 1, 8, false, false, "<G>light green<1>", "<G>juice<1>", 0});
  liquids.emplace_hint(liquids.end(), LIQ_MILK,
    liqEntry{0, 2, 6, false, false, "<W>white<1>", "<W>milk<1>", 0});
  liquids.emplace_hint(liquids.end(), LIQ_TEA,
    liqEntry{-1, -1, 6, false, false, "<o>brown<1>", "<o>tea<1>", 0});
  liquids.emplace_hint(liquids.end(), LIQ_COFFEE,
    liqEntry{-2, -3, 5, true, false, "<k>black<1>", "<k>coffee<1>", 0});
  liquids.emplace_hint(liquids.end(), LIQ_BLOOD,
    liqEntry{0, 2, -1, true, false, "<r>red<1>", "<r>blood<1>", 0});
  liquids.emplace_hint(liquids.end(), LIQ_SALTWATER,
    liqEntry{0, 1, -5, false, false, "clear", "salt water", 0});
  liquids.emplace_hint(liquids.end(), LIQ_MEAD,
    liqEntry{2, 2, 4, false, false, "<k>black<1>", "<k>mead<1>", 0});
  liquids.emplace_hint(liquids.end(), LIQ_VODKA,
    liqEntry{12, -3, -1, false, false, "clear", "vodka", 0});
  liquids.emplace_hint(liquids.end(), LIQ_RUM,
    liqEntry{11, -3, -1, false, false, "clear", "rum", 0});
  liquids.emplace_hint(liquids.end(), LIQ_TEQUILA,
    liqEntry{13, -3, -1, false, false, "golden", "tequila", 0});
  liquids.emplace_hint(liquids.end(), LIQ_BRANDY,
    liqEntry{8, 1, 3, false, false, "<o>brown<1>", "<o>brandy<1>", 0});
  liquids.emplace_hint(liquids.end(), LIQ_RED_WINE,
    liqEntry{7, -1, 6, false, false, "<R>red<1>", "<R>red wine<1>", 0});
  liquids.emplace_hint(liquids.end(), LIQ_WARM_MEAD,
    liqEntry{2, 1, 5, false, false, "<k>black<1>", "<k>warm mead<1>", 0});
  liquids.emplace_hint(liquids.end(), LIQ_CHAMPAGNE,
    liqEntry{6, -2, 4, false, false, "bubbly, translucent", "champagne", 0});
  liquids.emplace_hint(liquids.end(), LIQ_HOLYWATER,
    liqEntry{0, 1, 10, true, false, "clear", "holy water", 0});
  liquids.emplace_hint(liquids.end(), LIQ_PORT,
    liqEntry{8, -1, 5, false, false, "<R>red<1>", "<R>port<1>", 0});
  liquids.emplace_hint(liquids.end(), LIQ_MUSHROOM_ALE,
    liqEntry{7, -1, 5, false, false, "<g>green<1>", "<g>mushroom<1><o> ale<1>",
      0});
  liquids.emplace_hint(liquids.end(), LIQ_VOMIT,
    liqEntry{8, -1, 5, false, false, "<G>light green<1>",
      "<G>v<o>o<G>m<o>i<G>t<1>", 0});
  liquids.emplace_hint(liquids.end(), LIQ_COLA,
    liqEntry{-1, 2, 5, false, false, "<o>brown<1>", "<o>cola<1>", 0});
  liquids.emplace_hint(liquids.end(), LIQ_STRAWBERRY_MARGARITA,
    liqEntry{-1, 2, 5, false, false, "<r>smooth, pink<1>",
      "<r>strawberry margarita<1>", 0});
  liquids.emplace_hint(liquids.end(), LIQ_BLUE_MARGARITA,
    liqEntry{-1, 2, 5, false, false, "<b>smooth, blue<1>",
      "<b>blue margarita<1>", 0});
  liquids.emplace_hint(liquids.end(), LIQ_GOLD_MARGARITA,
    liqEntry{-1, 2, 5, false, false, "<Y>smooth, golden<1>",
      "<Y>gold margarita<1>", 0});
  liquids.emplace_hint(liquids.end(), LIQ_STRAWBERRY_DAIQUIRI,
    liqEntry{-1, 2, 5, false, false, "<r>pink, frothy<1>",
      "<r>strawberry daiquiri<1>", 0});
  liquids.emplace_hint(liquids.end(), LIQ_BANANA_DAIQUIRI,
    liqEntry{-1, 2, 5, false, false, "<Y>yellow, frothy<1>",
      "<Y>banana daiquiri<1>", 0});
  liquids.emplace_hint(liquids.end(), LIQ_PINA_COLADA,
    liqEntry{-1, 2, 5, false, false, "<W>frothy, white<1>", "<W>pina colada<1>",
      0});
  liquids.emplace_hint(liquids.end(), LIQ_TEQUILA_SUNRISE,
    liqEntry{-1, 2, 5, false, false, "<o>smooth, yellow<1>",
      "<o>tequila sunrise<1>", 0});
  liquids.emplace_hint(liquids.end(), LIQ_ISLA_VERDE,
    liqEntry{-1, 2, 5, false, false, "<g>green, frothy<1>", "<g>isla verde<1>",
      0});
  liquids.emplace_hint(liquids.end(), LIQ_POT_CURE_POISON,
    liqEntry{-1, -1, 1, true, false, "<W>white<1>", "<W>white potion<1>", 68});
  liquids.emplace_hint(liquids.end(), LIQ_POT_HEAL_LIGHT,
    liqEntry{-1, -1, 1, true, false, "<W>silver<1>", "<W>silver potion<1>",
      72});
  liquids.emplace_hint(liquids.end(), LIQ_POT_HEAL_CRIT,
    liqEntry{-1, -1, 1, true, false, "<Y>golden<1>", "<Y>golden potion<1>",
      331});
  liquids.emplace_hint(liquids.end(), LIQ_POT_HEAL,
    liqEntry{-1, -1, 1, true, false, "<b>blue<1>", "<b>blue potion<1>", 1276});
  liquids.emplace_hint(liquids.end(), LIQ_POT_SANCTUARY,
    liqEntry{-1, -1, 1, true, false, "<c>platinum<1>", "<c>platinum potion<1>",
      1846});
  liquids.emplace_hint(liquids.end(), LIQ_POT_FLIGHT,
    liqEntry{-1, -1, 1, true, false, "<c>cyan<1>", "<c>cyan potion<1>", 6160});
  liquids.emplace_hint(liquids.end(), LIQ_POT_BIND,
    liqEntry{-1, -1, 1, true, false, "<R>light pink<1>",
      "<R>light pink potion<1>", 648});
  liquids.emplace_hint(liquids.end(), LIQ_POT_BLINDNESS,
    liqEntry{-1, -1, 1, true, false, "<P>magenta<1>", "<P>magenta potion<1>",
      1248});
  liquids.emplace_hint(liquids.end(), LIQ_POT_ARMOR,
    liqEntry{-1, -1, 1, true, false, "<k>grey<1>", "<k>grey potion<1>", 24});
  liquids.emplace_hint(liquids.end(), LIQ_POT_REFRESH,
    liqEntry{-1, -1, 1, true, false, "<k>black<1>", "<k>black potion<1>", 76});
  liquids.emplace_hint(liquids.end(), LIQ_POT_SECOND_WIND,
    liqEntry{-1, -1, 1, true, false, "<o>bronze<1>", "<o>bronze potion<1>",
      759});
  liquids.emplace_hint(liquids.end(), LIQ_POT_CURSE,
    liqEntry{-1, -1, 1, true, false, "<r>maroon<1>", "<r>maroon potion<1>",
      15});
  liquids.emplace_hint(liquids.end(), LIQ_POT_DETECT_INVIS,
    liqEntry{-1, -1, 1, true, false, "<Y>chartreuse<1>",
      "<Y>chartreuse potion<1>", 92});
  liquids.emplace_hint(liquids.end(), LIQ_POT_BLESS,
    liqEntry{-1, -1, 1, true, false, "<o>orange<1>", "<o>orange potion<1>",
      26});
  liquids.emplace_hint(liquids.end(), LIQ_POT_INVIS,
    liqEntry{-1, -1, 1, true, false, "<g>puce<1>", "<g>puce potion<1>", 539});
  liquids.emplace_hint(liquids.end(), LIQ_POT_HEAL_FULL,
    liqEntry{-1, -1, 1, true, false, "<r>vermilion<1>",
      "<r>vermilion potion<1>", 3168});
  liquids.emplace_hint(liquids.end(), LIQ_POT_SUFFOCATE,
    liqEntry{-1, -1, 1, true, false, "translucent white",
      "translucent white potion", 1896});
  liquids.emplace_hint(liquids.end(), LIQ_POT_FEATHERY_DESCENT,
    liqEntry{-1, -1, 1, true, false, "dull white", "dull white potion", 92});
  liquids.emplace_hint(liquids.end(), LIQ_POT_DETECT_MAGIC,
    liqEntry{-1, -1, 1, true, false, "<B>bright blue<1>",
      "<B>bright blue potion<1>", 72});
  liquids.emplace_hint(liquids.end(), LIQ_POT_DISPEL_MAGIC,
    liqEntry{-1, -1, 1, true, false, "<R>bright red<1>",
      "<R>bright red potion<1>", 268});
  liquids.emplace_hint(liquids.end(), LIQ_POT_STONE_SKIN,
    liqEntry{-1, -1, 1, true, false, "<k>coarse grey<1>",
      "<k>coarse grey potion<1>", 820});
  liquids.emplace_hint(liquids.end(), LIQ_POT_TRAIL_SEEK,
    liqEntry{-1, -1, 1, true, false, "<o>light brown<1>",
      "<o>light brown potion<1>", 712});
  liquids.emplace_hint(liquids.end(), LIQ_POT_FAERIE_FIRE,
    liqEntry{-1, -1, 1, true, false, "<p>purple<1>", "<p>purple potion<1>",
      30});
  liquids.emplace_hint(liquids.end(), LIQ_POT_FLAMING_FLESH,
    liqEntry{-1, -1, 1, true, false, "<r>crimson<1>", "<r>crimson potion<1>",
      820});
  liquids.emplace_hint(liquids.end(), LIQ_POT_CONJURE_ELE_EARTH,
    liqEntry{-1, -1, 1, true, false, "<k>black<1>", "<k>black potion<1>", 268});
  liquids.emplace_hint(liquids.end(), LIQ_POT_SENSE_LIFE,
    liqEntry{-1, -1, 1, true, false, "<o>stained orange<1>",
      "<o>stained orange potion<1>", 72});
  liquids.emplace_hint(liquids.end(), LIQ_POT_STEALTH,
    liqEntry{-1, -1, 1, true, false, "<k>smooth grey<1>",
      "<k>smooth grey potion<1>", 276});
  liquids.emplace_hint(liquids.end(), LIQ_POT_TRUE_SIGHT,
    liqEntry{-1, -1, 1, true, false, "<W>pure white<1>",
      "<W>pure white potion<1>", 712});
  liquids.emplace_hint(liquids.end(), LIQ_POT_ACCELERATE,
    liqEntry{-1, -1, 1, true, false, "<W>shiny<1>", "<w>shiny potion<1>", 138});
  liquids.emplace_hint(liquids.end(), LIQ_POT_INFRAVISION,
    liqEntry{-1, -1, 1, true, false, "<r>stained red<1>",
      "<r>stained red potion<1>", 234});
  liquids.emplace_hint(liquids.end(), LIQ_POT_SORC_GLOBE,
    liqEntry{-1, -1, 1, true, false, "clear", "clear potion", 52});
  liquids.emplace_hint(liquids.end(), LIQ_POT_POISON,
    liqEntry{-1, -1, 1, true, false, "<p>indigo<1>", "<p>indigo potion<1>",
      60});
  liquids.emplace_hint(liquids.end(), LIQ_POT_BONE_BREAKER,
    liqEntry{-1, -1, 1, true, false, "<k>charcoal gray<1>",
      "<k>charcoal gray potion<1>", 345});
  liquids.emplace_hint(liquids.end(), LIQ_POT_AQUALUNG,
    liqEntry{-1, -1, 1, true, false, "<B>crystal blue<1>",
      "<B>crystal blue potion<1>", 3696});
  liquids.emplace_hint(liquids.end(), LIQ_POT_HASTE,
    liqEntry{-1, -1, 1, true, false, "<g>effervescent<1>",
      "<g>effervescent potion<1>", 772});
  liquids.emplace_hint(liquids.end(), LIQ_POT_TELEPORT,
    liqEntry{-1, -1, 1, true, false, "<k>gray<1>", "<k>gray potion<1>", 95});
  liquids.emplace_hint(liquids.end(), LIQ_POT_GILLS_OF_FLESH,
    liqEntry{-1, -1, 1, true, false, "<B>light blue<1>",
      "<B>light blue potion<1>", 258});
  liquids.emplace_hint(liquids.end(), LIQ_POT_CURE_BLINDNESS,
    liqEntry{-1, -1, 1, true, false, "<Y>sunny yellow<1>",
      "<Y>sunny yellow potion<1>", 235});
  liquids.emplace_hint(liquids.end(), LIQ_POT_CURE_DISEASE,
    liqEntry{-1, -1, 1, true, false, "<P>chalky pink<1>",
      "<P>chalky pink potion<1>", 123});
  liquids.emplace_hint(liquids.end(), LIQ_POT_SHIELD_OF_MISTS,
    liqEntry{-1, -1, 1, true, false, "<g>dark green<1>",
      "<g>dark green potion<1>", 3696});
  liquids.emplace_hint(liquids.end(), LIQ_POT_SENSE_PRESENCE,
    liqEntry{-1, -1, 1, true, false, "translucent", "translucent potion",
      3696});
  liquids.emplace_hint(liquids.end(), LIQ_POT_CHEVAL,
    liqEntry{-1, -1, 1, true, false, "<W>pale white<1>",
      "<W>pale white potion<1>", 3696});
  liquids.emplace_hint(liquids.end(), LIQ_POT_DJALLAS_PROTECTION,
    liqEntry{-1, -1, 1, true, false, "<k>steel grey<1>",
      "<k>steel grey potion<1>", 3696});
  liquids.emplace_hint(liquids.end(), LIQ_POT_LEGBAS_GUIDANCE,
    liqEntry{-1, -1, 1, true, false, "<k>slate grey<1>",
      "<k>slate grey potion<1>", 3696});
  liquids.emplace_hint(liquids.end(), LIQ_POT_DETECT_SHADOW,
    liqEntry{-1, -1, 1, true, false, "<o>sparkling amber<1>",
      "<o>sparkling amber potion<1>", 3696});
  liquids.emplace_hint(liquids.end(), LIQ_POT_CELERITE,
    liqEntry{-1, -1, 1, true, false, "<r>rusty<1>", "<r>rusty potion<1>",
      3696});
  liquids.emplace_hint(liquids.end(), LIQ_POT_CLARITY,
    liqEntry{-1, -1, 1, true, false, "fizzling", "fizzling potion", 3696});
  liquids.emplace_hint(liquids.end(), LIQ_POT_BOILING_BLOOD,
    liqEntry{-1, -1, 1, true, false, "<r>scarlet<1>", "<r>scarlet potion<1>",
      3696});
  liquids.emplace_hint(liquids.end(), LIQ_POT_STUPIDITY,
    liqEntry{-1, -1, 1, true, false, "transparent", "transparent potion",
      3696});
  liquids.emplace_hint(liquids.end(), LIQ_POT_ENLIVEN,
    liqEntry{-1, -1, 1, true, false, "<w>frothy<1> <o>brown<1>",
      "<w>frothy<1> <o>brown<1> potion", 3696});
  liquids.emplace_hint(liquids.end(), LIQ_POT_PLASMA_MIRROR,
    liqEntry{-1, -1, 1, true, false, "<b>sp<B>ar<W>kl<w>y<1> <b>blue<1>",
      "<b>sp<B>ar<W>kl<w>y<1> <b>blue<1> potion", 4896});
  liquids.emplace_hint(liquids.end(), LIQ_POT_SLUMBER,
    liqEntry{-1, -1, 1, true, false, "<W>milky white<1>",
      "<W>milky white potion<1>", 276});
  liquids.emplace_hint(liquids.end(), LIQ_POT_HEAL2,
    liqEntry{-1, -1, 1, true, false, "<r>red<1>", "<r>red potion<1>", 1209});
  liquids.emplace_hint(liquids.end(), LIQ_POT_FEATHERY_DESCENT2,
    liqEntry{-1, -1, 1, true, false, "<Y>yellow<1>", "<Y>yellow potion<1>",
      92});
  liquids.emplace_hint(liquids.end(), LIQ_POT_SANCTUARY2,
    liqEntry{-1, -1, 1, true, false, "<W>milky white<1>",
      "<W>milky white potion<1>", 1846});
  liquids.emplace_hint(liquids.end(), LIQ_POT_STONE_SKIN2,
    liqEntry{-1, -1, 1, true, false, "<w>granite coloured<1>",
      "<w>granite coloured potion<1>", 1094});
  liquids.emplace_hint(liquids.end(), LIQ_POT_INFRAVISION2,
    liqEntry{-1, -1, 1, true, false, "<o>dark orange<1>",
      "<o>dark orange potion<1>", 224});
  liquids.emplace_hint(liquids.end(), LIQ_POT_HEAL_LIGHT2,
    liqEntry{-1, -1, 1, true, false, "clear", "clear potion", 72});
  liquids.emplace_hint(liquids.end(), LIQ_POT_GILLS_OF_FLESH2,
    liqEntry{-1, -1, 1, true, false, "<b>deep blue<1>",
      "<b>deep blue potion<1>", 318});
  liquids.emplace_hint(liquids.end(), LIQ_POT_CELERITE2,
    liqEntry{-1, -1, 1, true, false, "<p>purple<1>", "<p>purple potion<1>",
      3696});
  liquids.emplace_hint(liquids.end(), LIQ_POT_CELERITE3,
    liqEntry{-1, -1, 1, true, false, "<g>sickly green<1>",
      "<g>sickley green potion<1>", 3696});
  liquids.emplace_hint(liquids.end(), LIQ_POT_TELEPORT2,
    liqEntry{-1, -1, 1, true, false, "<r>blood red<1>",
      "<r>blood red potion<1>", 114});
  liquids.emplace_hint(liquids.end(), LIQ_POT_BLESS2,
    liqEntry{-1, -1, 1, true, false, "<Y>dark yellow<1>",
      "<Y>dark yellow potion<1>", 26});
  liquids.emplace_hint(liquids.end(), LIQ_POT_SECOND_WIND2,
    liqEntry{-1, -1, 1, true, false, "<o>bronze<1>", "<o>bronze potion<1>",
      759});
  liquids.emplace_hint(liquids.end(), LIQ_POT_MULTI1,
    liqEntry{-1, -1, 1, true, false, "<Y>yellowish amber<1>",
      "<Y>yellowish amber potion<1>", 532});
  liquids.emplace_hint(liquids.end(), LIQ_POT_MULTI2,
    liqEntry{-1, -1, 1, true, false, "<g>greenish amber<1>",
      "<g>greenish amber potion<1>", 1664});
  liquids.emplace_hint(liquids.end(), LIQ_POT_MULTI3,
    liqEntry{-1, -1, 1, true, false, "<r>ruddy amber<1>",
      "<r>ruddy amber potion<1>", 2024});
  liquids.emplace_hint(liquids.end(), LIQ_POT_MULTI4,
    liqEntry{-1, -1, 1, true, false, "<b>dark blue<1>",
      "<b>dark blue potion<1>", 1288});
  liquids.emplace_hint(liquids.end(), LIQ_POT_MULTI5,
    liqEntry{-1, -1, 1, true, false, "<r>shadowy crimson<1>",
      "<r>shadowy crimson potion<1>", 714});
  liquids.emplace_hint(liquids.end(), LIQ_POT_MULTI6,
    liqEntry{-1, -1, 1, true, false, "<p>mauve<1>", "<p>mauve potion<1>",
      1861});
  liquids.emplace_hint(liquids.end(), LIQ_POT_MULTI7,
    liqEntry{-1, -1, 1, true, false, "<g>rancid green<1>",
      "<g>rancid green potion<1>", 1733});
  liquids.emplace_hint(liquids.end(), LIQ_POT_MULTI8,
    liqEntry{-1, -1, 1, true, false, "<r>rancid red<1>",
      "<r>rancid red potion<1>", 1882});
  liquids.emplace_hint(liquids.end(), LIQ_POT_MULTI9,
    liqEntry{-1, -1, 1, true, false, "<o>rancid brown<1>",
      "<o>rancid brown potion<1>", 2891});
  liquids.emplace_hint(liquids.end(), LIQ_POT_MULTI10,
    liqEntry{-1, -1, 1, true, false, "<k>silvery grey<1>",
      "<k>silvery grey potion<1>", 1900});
  liquids.emplace_hint(liquids.end(), LIQ_POT_MULTI11,
    liqEntry{-1, -1, 1, true, false, "<r>ruby<1>", "<r>ruby potion<1>", 2721});
  liquids.emplace_hint(liquids.end(), LIQ_POT_YOUTH,
    liqEntry{-1, -1, 1, true, false, "<o>glowing<1> <p>purple<1>",
      "<p>youth potion<1>", 20000});
  liquids.emplace_hint(liquids.end(), LIQ_POT_STAT,
    liqEntry{-1, -1, 1, true, false, "<o>glowing<1> <p>purple<1>",
      "<p>characteristics potion<1>", 100000});
  liquids.emplace_hint(liquids.end(), LIQ_POT_LEARNING,
    liqEntry{-1, -1, 1, true, false, "<o>glowing<1> <p>purple<1>",
      "<p>learning potion<1>", 100000});
  liquids.emplace_hint(liquids.end(), LIQ_POISON_STANDARD,
    liqEntry{-1, -1, 1, true, true, "<p>purple<1>", "<p>contact poison<1>",
      90});
  liquids.emplace_hint(liquids.end(), LIQ_POISON_CAMAS,
    liqEntry{-1, -1, 1, true, true, "<g>green<1>", "<g>Death Camas extract<1>",
      750});
  liquids.emplace_hint(liquids.end(), LIQ_POISON_ANGEL,
    liqEntry{-1, -1, 1, true, true, "<W>white<1>",
      "<W>Destroying Angel extract<1>", 350});
  liquids.emplace_hint(liquids.end(), LIQ_POISON_JIMSON,
    liqEntry{-1, -1, 1, true, true, "<o>orange<1>", "<o>Jimson Weed extract<1>",
      350});
  liquids.emplace_hint(liquids.end(), LIQ_POISON_HEMLOCK,
    liqEntry{-1, -1, 1, true, true, "<o>reddish-brown<1>",
      "<o>Hemlock extract<1>", 350});
  liquids.emplace_hint(liquids.end(), LIQ_POISON_MONKSHOOD,
    liqEntry{-1, -1, 1, true, true, "<b>blue<1>", "<b>Monkshood extract<1>",
      50});
  liquids.emplace_hint(liquids.end(), LIQ_POISON_GLOW_FISH,
    liqEntry{-1, -1, 1, true, true, "<r>glowing red<1>",
      "<r>Glow Fish extract<1>", 150});
  liquids.emplace_hint(liquids.end(), LIQ_POISON_SCORPION,
    liqEntry{-1, -1, 1, true, true, "<o>smooth orange<1>",
      "<o>liquor of scorpion<1>", 350});
  liquids.emplace_hint(liquids.end(), LIQ_POISON_VIOLET_FUNGUS,
    liqEntry{-1, -1, 1, true, true, "<p>violet<1>",
      "<p>Violet Fungus Spore extract<1>", 350});
  liquids.emplace_hint(liquids.end(), LIQ_POISON_DEVIL_ICE,
    liqEntry{-1, -1, 1, true, true, "<W>shiny white<1>",
      "<W>Ice Devil Serum<1>", 150});
  liquids.emplace_hint(liquids.end(), LIQ_POISON_FIREDRAKE,
    liqEntry{-1, -1, 1, true, true, "<r>glimmering red<1>",
      "<r>Firedrake Serum<1>", 150});
  liquids.emplace_hint(liquids.end(), LIQ_POISON_INFANT,
    liqEntry{-1, -1, 1, true, true, "<r>dull red<1>", "<r>ichor of infant<1>",
      150});
  liquids.emplace_hint(liquids.end(), LIQ_POISON_PEA_SEED,
    liqEntry{-1, -1, 1, true, true, "<g>dull green<1>",
      "<g>Sweet Pea Seed extract<1>", 50});
  liquids.emplace_hint(liquids.end(), LIQ_POISON_ACACIA,
    liqEntry{-1, -1, 1, true, true, "<o>dull yellow<1>", "<o>Acacia extract<1>",
      150});
  liquids.emplace_hint(liquids.end(), LIQ_LUBRICATION,
    liqEntry{-1, -1, 1, false, false, "<c>clear jelly<1>", "<c>lubricant<1>",
      100});
  liquids.emplace_hint(liquids.end(), LIQ_MAGICAL_ELIXIR,
    liqEntry{-1, -1, 1, true, false, "<o>glowing<1> clear",
      "<p>magical elixir<1>", 70});
  liquids.emplace_hint(liquids.end(), LIQ_URINE,
    liqEntry{0, 1, -5, false, false, "<Y>golden<1>", "<Y>urine<1>", 0});
  liquids.emplace_hint(liquids.end(), LIQ_POT_HEALING_GRASP,
    liqEntry{100, 0, 1, true, false, "<p>light purple<1>",
      "<y>a light purple potion<1>", 0});
  liquids.emplace_hint(liquids.end(), LIQ_POT_CLEANSE,
    liqEntry{100, 0, 1, true, false, "clear with clumps",
      "a clear potion with clumps", 0});
  liquids.emplace_hint(liquids.end(), LIQ_POT_QUICKSILVER,
    liqEntry{-1, -1, -1, true, false, "shiny silver", "a shiny silver potion",
      9999});
  liquids.emplace_hint(liquids.end(), LIQ_POT_MYSTERY,
    liqEntry{-1, -1, 1, true, false, "<G>bright glowing green<1>",
      "<G>mystery potion<1>", 20000});
  liquids.emplace_hint(liquids.end(), LIQ_MUD,
    liqEntry{-1, -1, 1, false, false, "<o>brown<1>", "<o>mud<1>", 0});
  liquids.emplace_hint(liquids.end(), LIQ_POT_FILTH,
    liqEntry{-1, -1, 1, true, false, "<o>filthy<1>", "<o>filth<1>", 0});
  liquids.emplace_hint(liquids.end(), LIQ_GUANO,
    liqEntry{-1, -1, 1, false, false, "<o>dirty white<1>", "<o>guano<1>", 0});
}

liqEntry::liqEntry(int d, int h, int t, bool p, bool x, const char* col,
  const char* n, int pr) :
  drunk(d),
  hunger(h),
  thirst(t),
  potion(p),
  poison(x),
  color(col),
  name(n),
  price(pr) {}
