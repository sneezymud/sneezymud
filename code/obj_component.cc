//////////////////////////////////////////////////////////////////////////
//
//      SneezyMUD - All rights reserved, SneezyMUD Coding Team
//
//      "obj_component.cc" - All functions and routines related to components
//
//////////////////////////////////////////////////////////////////////////

#include "stdsneezy.h"
#include "shop.h"
#include "database.h"
#include "obj_spellbag.h"
#include "obj_open_container.h"
#include "obj_component.h"
#include "shopowned.h"
#include "corporation.h"
#include "obj_mergeable.h"

vector<compPlace>component_placement(0);
vector<compInfo>CompInfo(0);
vector<COMPINDEX>CompIndex(0);


bool TComponent::willMerge(TMergeable *tm)
{
  TComponent *tComp;


  // Basically find another component of the same type that is:
  // Same VNum.
  // Has a cost greater than 0 (ignore comps from leveling)
  if(!(tComp=dynamic_cast<TComponent *>(tm)) ||
     tComp==this || 
     (tComp->objVnum() != objVnum()) ||
     tComp->obj_flags.cost <= 0 ||  // ignore "free" comps from GM
     obj_flags.cost <= 0){
    return false;
  }
  return true;
}

void TComponent::doMerge(TMergeable *tm)
{
  TRoom *rp = NULL;
  TComponent *tComp;


  if(!(tComp=dynamic_cast<TComponent *>(tm)))
    return;
  
  if (!(rp = roomp)) {
    if (parent) {
      rp = parent->roomp;
    } else {
      if (!(rp = tComp->roomp)) {
	if (tComp->parent) {
	  rp = tComp->parent->roomp;
	}
      }
    }
  }
  if (rp) {
    sstring str = sstring(shortDescr);
    sendrpf(COLOR_BASIC, rp, "%s glows brightly and merges with %s.\n\r", str.cap().c_str(), str.c_str());
  }
  // Compute the decay value of the to-be merged component by performing
  // a weighted average based on charges
  int c_decay = (obj_flags.decay_time * getComponentCharges() +
		 tComp->obj_flags.decay_time * tComp->getComponentCharges()) /
    max(1, (getComponentCharges() + tComp->getComponentCharges()));
  addToComponentCharges(tComp->getComponentCharges());
  obj_flags.cost += tComp->obj_flags.cost;
  obj_flags.decay_time = c_decay;
  --(*tComp);
  delete tComp;
}


void assign_component_placement()
{
  if(gamePort==8900){
    vlogf(LOG_LOW, "Skipping assign_component_placement for builder mud");
    return;
  }

// rainbow bridge 1
  component_placement.push_back(compPlace(BRIDGE_ROOM, -1, MOB_NONE, 
     ITEM_RAINBOW_BRIDGE1,
     CACT_PLACE, 1, 100,
     HOUR_SUNRISE, HOUR_SUNSET, -1, -1, -1, -1, 1<<WEATHER_RAINY | 1<<WEATHER_LIGHTNING,
     "<p>$p slowly shimmers into existence!<1>", ""));
  component_placement.push_back(compPlace(BRIDGE_ROOM, -1, MOB_NONE, 
     ITEM_RAINBOW_BRIDGE1,
     CACT_REMOVE, 1, 100,
     -1, -1, -1, -1, -1, -1, ~(1<<WEATHER_RAINY | 1<<WEATHER_LIGHTNING),
     "<p>$p slowly fades out of existence!<1>", ""));
  component_placement.push_back(compPlace(BRIDGE_ROOM, -1, MOB_NONE, 
     ITEM_RAINBOW_BRIDGE1,
     CACT_REMOVE, 1, 100,
     HOUR_SUNSET, -1, -1, -1, -1, -1, -1,
     "<p>$p slowly fades out of existence!<1>", ""));
// rainbow bridge 2
  component_placement.push_back(compPlace(BRIDGE_ROOM2, -1, MOB_NONE, 
     ITEM_RAINBOW_BRIDGE2,
     CACT_PLACE, 1, 100,
     HOUR_SUNRISE, HOUR_SUNSET, -1, -1, -1, -1, 1<<WEATHER_RAINY | 1<<WEATHER_LIGHTNING,
     "<p>$p slowly shimmers into existence!<1>", ""));
  component_placement.push_back(compPlace(BRIDGE_ROOM2, -1, MOB_NONE, 
     ITEM_RAINBOW_BRIDGE2,
     CACT_REMOVE, 1, 100,
     -1, -1, -1, -1, -1, -1, ~(1<<WEATHER_RAINY | 1<<WEATHER_LIGHTNING),
     "<p>$p slowly fades out of existence!<1>", ""));
  component_placement.push_back(compPlace(BRIDGE_ROOM2, -1, MOB_NONE, 
     ITEM_RAINBOW_BRIDGE2,
     CACT_REMOVE, 1, 100,
     HOUR_SUNSET, -1, -1, -1, -1, -1, -1,
     "<p>$p slowly fades out of existence!<1>", ""));

  // cheval
  component_placement.push_back(compPlace(6101, -1, MOB_NONE, 
     COMP_CHEVAL,
     CACT_PLACE, 9999, 65,
     HOUR_DAY_BEGIN, HOUR_DAY_END, -1, -1, -1, -1, (1<<WEATHER_RAINY),
     "<k>$p stupidly dives into the ground and lands on its back.<1>" ,""));
  component_placement.push_back(compPlace(6101, -1, MOB_NONE, 
     COMP_CHEVAL,
     CACT_REMOVE, -1, 100,
     HOUR_DAY_END, -1, -1, -1, -1, -1, -1,
     "<k>$p skitters away into the night.<1>", ""));
  component_placement.push_back(compPlace(6101, -1, MOB_NONE, 
     COMP_CHEVAL,
     CACT_REMOVE, -1, 100,
     -1, -1, -1, -1, -1, -1, ~(1<<WEATHER_RAINY),
     "<k>$p flips over and flies away.<1>", ""));
  component_placement.push_back(compPlace(6104, -1, MOB_NONE, 
     COMP_CHEVAL,
     CACT_PLACE, 9999, 65,
     HOUR_DAY_BEGIN, HOUR_DAY_END, -1, -1, -1, -1, (1<<WEATHER_RAINY),
     "<k>$p stupidly dives into the ground and lands on its back.<1>" ,""));
  component_placement.push_back(compPlace(6104, -1, MOB_NONE, 
     COMP_CHEVAL,
     CACT_REMOVE, -1, 100,
     HOUR_DAY_END, -1, -1, -1, -1, -1, -1,
     "<k>$p skitters away into the night.<1>", ""));
  component_placement.push_back(compPlace(6104, -1, MOB_NONE, 
     COMP_CHEVAL,
     CACT_REMOVE, -1, 100,
     -1, -1, -1, -1, -1, -1, ~(1<<WEATHER_RAINY),
     "<k>$p flips over and flies away.<1>", ""));
  component_placement.push_back(compPlace(6108, -1, MOB_NONE, 
     COMP_CHEVAL,
     CACT_PLACE, 9999, 65,
     HOUR_DAY_BEGIN, HOUR_DAY_END, -1, -1, -1, -1, (1<<WEATHER_RAINY),
     "<k>$p stupidly dives into the ground and lands on its back.<1>" ,""));
  component_placement.push_back(compPlace(6108, -1, MOB_NONE, 
     COMP_CHEVAL,
     CACT_REMOVE, -1, 100,
     HOUR_DAY_END, -1, -1, -1, -1, -1, -1,
     "<k>$p skitters away into the night.<1>", ""));
  component_placement.push_back(compPlace(6108, -1, MOB_NONE, 
     COMP_CHEVAL,
     CACT_REMOVE, -1, 100,
     -1, -1, -1, -1, -1, -1, ~(1<<WEATHER_RAINY),
     "<k>$p flips over and flies away.<1>", ""));
  component_placement.push_back(compPlace(6200, -1, MOB_NONE, 
     COMP_CHEVAL,
     CACT_PLACE, 9999, 65,
     HOUR_DAY_BEGIN, HOUR_DAY_END, -1, -1, -1, -1, (1<<WEATHER_RAINY),
     "<k>$p stupidly dives into the ground and lands on its back.<1>" ,""));
  component_placement.push_back(compPlace(6200, -1, MOB_NONE, 
     COMP_CHEVAL,
     CACT_REMOVE, -1, 100,
     HOUR_DAY_END, -1, -1, -1, -1, -1, -1,
     "<k>$p skitters away into the night.<1>", ""));
  component_placement.push_back(compPlace(6200, -1, MOB_NONE, 
     COMP_CHEVAL,
     CACT_REMOVE, -1, 100,
     -1, -1, -1, -1, -1, -1, ~(1<<WEATHER_RAINY),
     "<k>$p flips over and flies away.<1>", ""));

  component_placement.push_back(compPlace(6205, -1, MOB_NONE, 
     COMP_CHEVAL,
     CACT_PLACE, 9999, 65,
     HOUR_DAY_BEGIN, HOUR_DAY_END, -1, -1, -1, -1, (1<<WEATHER_RAINY),
     "<k>$p stupidly dives into the ground and lands on its back.<1>" ,""));
  component_placement.push_back(compPlace(6205, -1, MOB_NONE, 
     COMP_CHEVAL,
     CACT_REMOVE, -1, 100,
     HOUR_DAY_END, -1, -1, -1, -1, -1, -1,
     "<k>$p skitters away into the night.<1>", ""));
  component_placement.push_back(compPlace(6205, -1, MOB_NONE, 
     COMP_CHEVAL,
     CACT_REMOVE, -1, 100,
     -1, -1, -1, -1, -1, -1, ~(1<<WEATHER_RAINY),
     "<k>$p flips over and flies away.<1>", ""));

  component_placement.push_back(compPlace(6211, -1, MOB_NONE, 
     COMP_CHEVAL,
     CACT_PLACE, 9999, 65,
     HOUR_DAY_BEGIN, HOUR_DAY_END, -1, -1, -1, -1, (1<<WEATHER_RAINY),
     "<k>$p stupidly dives into the ground and lands on its back.<1>" ,""));
  component_placement.push_back(compPlace(6211, -1, MOB_NONE, 
     COMP_CHEVAL,
     CACT_REMOVE, -1, 100,
     HOUR_DAY_END, -1, -1, -1, -1, -1, -1,
     "<k>$p skitters away into the night.<1>", ""));
  component_placement.push_back(compPlace(6211, -1, MOB_NONE, 
     COMP_CHEVAL,
     CACT_REMOVE, -1, 100,
     -1, -1, -1, -1, -1, -1, ~(1<<WEATHER_RAINY),
     "<k>$p flips over and flies away.<1>", ""));
  component_placement.push_back(compPlace(6220, -1, MOB_NONE, 
     COMP_CHEVAL,
     CACT_PLACE, 9999, 65,
     HOUR_DAY_BEGIN, HOUR_DAY_END, -1, -1, -1, -1, (1<<WEATHER_RAINY),
     "<k>$p stupidly dives into the ground and lands on its back.<1>" ,""));
  component_placement.push_back(compPlace(6220, -1, MOB_NONE, 
     COMP_CHEVAL,
     CACT_REMOVE, -1, 100,
     HOUR_DAY_END, -1, -1, -1, -1, -1, -1,
     "<k>$p skitters away into the night.<1>", ""));
  component_placement.push_back(compPlace(6220, -1, MOB_NONE, 
     COMP_CHEVAL,
     CACT_REMOVE, -1, 100,
     -1, -1, -1, -1, -1, -1, ~(1<<WEATHER_RAINY),
     "<k>$p flips over and flies away.<1>", ""));


// color spray
  component_placement.push_back(compPlace(11328, -1, MOB_NONE, 
     COMP_COLOR_SPRAY,
     CACT_PLACE, 9999, 65,
     HOUR_DAY_BEGIN, HOUR_DAY_END, -1, -1, -1, -1, (1<<WEATHER_RAINY),
     "<B>A small stone nearby begins to sparkle and glow from the kiss of a rainbow.<1>" ,""));
  component_placement.push_back(compPlace(11328, -1, MOB_NONE, 
     COMP_COLOR_SPRAY,
     CACT_REMOVE, -1, 100,
     HOUR_DAY_END, -1, -1, -1, -1, -1, -1,
     "<b>$p dulls and fades away as the night approaches.<1>", ""));
  component_placement.push_back(compPlace(11328, -1, MOB_NONE, 
     COMP_COLOR_SPRAY,
     CACT_REMOVE, -1, 100,
     -1, -1, -1, -1, -1, -1, ~(1<<WEATHER_RAINY),
     "<b>$p dulls and fades away.<1>", ""));

  component_placement.push_back(compPlace(1008, 1015, MOB_NONE, 
     COMP_COLOR_SPRAY,
     CACT_PLACE, 9999, 65,
     HOUR_DAY_BEGIN, HOUR_DAY_END, -1, -1, -1, -1, (1<<WEATHER_RAINY),
     "<B>A small stone nearby begins to sparkle and glow from the kiss of a rainbow.<1>",
     "<y>A beautiful rainbow streaks across the sky.<1>"));
  component_placement.push_back( compPlace(1008, 1015, MOB_NONE, 
     COMP_COLOR_SPRAY,
     CACT_REMOVE, -1, 100,
     HOUR_DAY_END, -1, -1, -1, -1, -1, -1,
     "<b>$p dulls and fades away as the night approaches.<1>", ""));
  component_placement.push_back( compPlace(1008, 1015, MOB_NONE, 
     COMP_COLOR_SPRAY,
     CACT_REMOVE, -1, 100,
     -1, -1, -1, -1, -1, -1, ~(1<<WEATHER_RAINY),
     "<b>$p dulls and fades away.<1>", ""));
  component_placement.push_back( compPlace(11368, -1, MOB_NONE, 
     COMP_COLOR_SPRAY,
     CACT_PLACE, 9999, 65,
     HOUR_DAY_BEGIN, HOUR_DAY_END, -1, -1, -1, -1, (1<<WEATHER_RAINY),
     "<B>A small stone nearby begins to sparkle and glow from the kiss of a rainbow.<1>",
     "<y>A beautiful rainbow streaks across the sky.<1>"));
  component_placement.push_back( compPlace(11368, -1, MOB_NONE, 
     COMP_COLOR_SPRAY,
     CACT_REMOVE, -1, 100,
     HOUR_DAY_END, -1, -1, -1, -1, -1, -1,
     "<b>$p dulls and fades away as the night approaches.<1>", ""));
  component_placement.push_back( compPlace(11368, -1, MOB_NONE, 
     COMP_COLOR_SPRAY,
     CACT_REMOVE, -1, 100,
     -1, -1, -1, -1, -1, -1, ~(1<<WEATHER_RAINY),
     "<b>$p dulls and fades away.<1>", ""));
  component_placement.push_back( compPlace(1018, 1023, MOB_NONE, 
     COMP_COLOR_SPRAY,
     CACT_PLACE, 9999, 65,
     HOUR_DAY_BEGIN, HOUR_DAY_END, -1, -1, -1, -1, (1<<WEATHER_RAINY),
     "<B>A small stone nearby begins to sparkle and glow from the kiss of a rainbow.<1>", ""));
  component_placement.push_back( compPlace(1018, 1023, MOB_NONE, 
     COMP_COLOR_SPRAY,
     CACT_REMOVE, -1, 100,
     HOUR_DAY_END, -1, -1, -1, -1, -1, -1,
     "<b>$p dulls and fades away as the night approaches.<1>",
     ""));
  component_placement.push_back( compPlace(1018, 1023, MOB_NONE, 
     COMP_COLOR_SPRAY,
     CACT_REMOVE, -1, 100,
     -1, -1, -1, -1, -1, -1, ~(1<<WEATHER_RAINY),
     "<b>$p dulls and fades away.<1>", ""));

  component_placement.push_back( compPlace(7510, 7512, MOB_NONE, 
     COMP_COLOR_SPRAY,
     CACT_PLACE, 9999, 65,
     HOUR_DAY_BEGIN, HOUR_DAY_END, -1, -1, -1, -1, (1<<WEATHER_RAINY),
     "<B>A small stone nearby begins to sparkle and glow from the kiss of a rainbow.<1>",
     "<y>A beautiful rainbow streaks across the sky.<1>"));
  component_placement.push_back( compPlace(7510, 7512, MOB_NONE, 
     COMP_COLOR_SPRAY,
     CACT_REMOVE, -1, 100,
     HOUR_DAY_END, -1, -1, -1, -1, -1, -1,
     "<b>$p dulls and fades away as the night approaches.<1>", ""));
  component_placement.push_back( compPlace(7510, 7512, MOB_NONE, 
     COMP_COLOR_SPRAY,
     CACT_REMOVE, -1, 100,
     -1, -1, -1, -1, -1, -1, ~(1<<WEATHER_RAINY),
     "<b>$p dulls and fades away.<1>", ""));

  component_placement.push_back(compPlace(11324, -1, MOB_NONE, 
     COMP_COLOR_SPRAY,
     CACT_PLACE, 9999, 65,
     HOUR_DAY_BEGIN, HOUR_DAY_END, -1, -1, -1, -1, (1<<WEATHER_RAINY),
     "<B>A small stone nearby begins to sparkle and glow from the kiss of a rainbow.<1>",
     "<y>A beautiful rainbow streaks across the sky.<1>"));
  component_placement.push_back(compPlace(11324, -1, MOB_NONE, 
     COMP_COLOR_SPRAY,
     CACT_REMOVE, -1, 100,
     HOUR_DAY_END, -1, -1, -1, -1, -1, -1,
     "<b>$p dulls and fades away as the night approaches.<1>", ""));
  component_placement.push_back(compPlace(11324, -1, MOB_NONE, 
     COMP_COLOR_SPRAY,
     CACT_REMOVE, -1, 100,
     -1, -1, -1, -1, -1, -1, ~(1<<WEATHER_RAINY),
     "<b>$p dulls and fades away.<1>", ""));

  component_placement.push_back(compPlace(7514, -1, MOB_NONE,
     COMP_ACID_BLAST,
     CACT_PLACE | CACT_UNIQUE, -1, 60,
     -1, -1, -1, -1, -1, -1, -1,
     "Drops of clear liquid drip from the ceiling and collect in a forgotten decanter.", "", SOUND_WATER_DROP, 5));

// Some white silicon [sand blast] //
  // sand only CREATED during day, night doesn't destroy
  component_placement.push_back(compPlace(6798, -1, MOB_NONE,
     COMP_SAND_BLAST,
     CACT_PLACE, 9999, 50,
     HOUR_DAY_BEGIN, HOUR_DAY_END, -1, -1, -1, -1, (1<<WEATHER_CLOUDLESS),
     "The extreme heat from the sun dries and cracks the $g.",
     "The blazing sun blisters the $g."));
  component_placement.push_back(compPlace(6798, -1, MOB_NONE,
     COMP_SAND_BLAST,
     CACT_REMOVE, -1, 100,
     -1, -1, -1, -1, -1, -1,
     (1<<WEATHER_RAINY | WEATHER_SNOWY | WEATHER_LIGHTNING),
     "Moisture hangs heavily in the air as bad weather approaches.", ""));
  component_placement.push_back(compPlace(7524, -1, MOB_NONE,
     COMP_SAND_BLAST,
     CACT_PLACE, 9999, 50,
     HOUR_DAY_BEGIN, HOUR_DAY_END, -1, -1, -1, -1, (1<<WEATHER_CLOUDLESS),
     "The extreme heat from the sun dries and cracks the $g.",
     "The blazing sun blisters the $g."));
  component_placement.push_back(compPlace(7524, -1, MOB_NONE,
     COMP_SAND_BLAST,
     CACT_REMOVE, -1, 100,
     -1, -1, -1, -1, -1, -1,
     (1<<WEATHER_RAINY | WEATHER_SNOWY | WEATHER_LIGHTNING),
     "Moisture hangs heavily in the air as bad weather approaches.", ""));
  component_placement.push_back(compPlace(9110, -1, MOB_NONE,
     COMP_SAND_BLAST,
     CACT_PLACE, 9999, 50,
     HOUR_DAY_BEGIN, HOUR_DAY_END, -1, -1, -1, -1, (1<<WEATHER_CLOUDLESS),
     "The extreme heat from the sun dries and cracks the $g.",
     "The blazing sun blisters the $g."));
  component_placement.push_back(compPlace(9110, -1, MOB_NONE,
     COMP_SAND_BLAST,
     CACT_REMOVE, -1, 100,
     -1, -1, -1, -1, -1, -1,
     (1<<WEATHER_RAINY | WEATHER_SNOWY | WEATHER_LIGHTNING),
     "Moisture hangs heavily in the air as bad weather approaches.", ""));
  component_placement.push_back(compPlace(9114, -1, MOB_NONE,
     COMP_SAND_BLAST,
     CACT_PLACE, 9999, 50,
     HOUR_DAY_BEGIN, HOUR_DAY_END, -1, -1, -1, -1, (1<<WEATHER_CLOUDLESS),
     "The extreme heat from the sun dries and cracks the $g.",
     "The blazing sun blisters the $g."));
  component_placement.push_back(compPlace(9114, -1, MOB_NONE,
     COMP_SAND_BLAST,
     CACT_REMOVE, -1, 100,
     -1, -1, -1, -1, -1, -1,
     (1<<WEATHER_RAINY | WEATHER_SNOWY | WEATHER_LIGHTNING),
     "Moisture hangs heavily in the air as bad weather approaches.", ""));
  component_placement.push_back(compPlace(9118, -1, MOB_NONE,
     COMP_SAND_BLAST,
     CACT_PLACE, 9999, 50,
     HOUR_DAY_BEGIN, HOUR_DAY_END, -1, -1, -1, -1, (1<<WEATHER_CLOUDLESS),
     "The extreme heat from the sun dries and cracks the $g.",
     "The blazing sun blisters the $g."));
  component_placement.push_back(compPlace(9118, -1, MOB_NONE,
     COMP_SAND_BLAST,
     CACT_REMOVE, -1, 100,
     -1, -1, -1, -1, -1, -1,
     (1<<WEATHER_RAINY | WEATHER_SNOWY | WEATHER_LIGHTNING),
     "Moisture hangs heavily in the air as bad weather approaches.", ""));
  component_placement.push_back(compPlace(9157, -1, MOB_NONE,
     COMP_SAND_BLAST,
     CACT_PLACE, 9999, 50,
     HOUR_DAY_BEGIN, HOUR_DAY_END, -1, -1, -1, -1, (1<<WEATHER_CLOUDLESS),
     "The extreme heat from the sun dries and cracks the $g.",
     "The blazing sun blisters the $g."));
  component_placement.push_back(compPlace(9157, -1, MOB_NONE,
     COMP_SAND_BLAST,
     CACT_REMOVE, -1, 100,
     -1, -1, -1, -1, -1, -1,
     (1<<WEATHER_RAINY | WEATHER_SNOWY | WEATHER_LIGHTNING),
     "Moisture hangs heavily in the air as bad weather approaches.", ""));
  component_placement.push_back(compPlace(9163, 9164, MOB_NONE,
     COMP_SAND_BLAST,
     CACT_PLACE, 9999, 50,
     HOUR_DAY_BEGIN, HOUR_DAY_END, -1, -1, -1, -1, (1<<WEATHER_CLOUDLESS),
     "The extreme heat from the sun dries and cracks the $g.",
     "The blazing sun blisters the $g."));
  component_placement.push_back(compPlace(9163, 9164, MOB_NONE,
     COMP_SAND_BLAST,
     CACT_REMOVE, -1, 100,
     -1, -1, -1, -1, -1, -1,
     (1<<WEATHER_RAINY | WEATHER_SNOWY | WEATHER_LIGHTNING),
     "Moisture hangs heavily in the air as bad weather approaches.", ""));
  component_placement.push_back(compPlace(9170, -1, MOB_NONE,
     COMP_SAND_BLAST,
     CACT_PLACE, 9999, 50,
     HOUR_DAY_BEGIN, HOUR_DAY_END, -1, -1, -1, -1, (1<<WEATHER_CLOUDLESS),
     "The extreme heat from the sun dries and cracks the $g.",
     "The blazing sun blisters the $g."));
  component_placement.push_back(compPlace(9170, -1, MOB_NONE,
     COMP_SAND_BLAST,
     CACT_REMOVE, -1, 100,
     -1, -1, -1, -1, -1, -1,
     (1<<WEATHER_RAINY | WEATHER_SNOWY | WEATHER_LIGHTNING),
     "Moisture hangs heavily in the air as bad weather approaches.", ""));
  component_placement.push_back(compPlace(12641, 12643, MOB_NONE,
     COMP_SAND_BLAST,
     CACT_PLACE, 9999, 50,
     HOUR_DAY_BEGIN, HOUR_DAY_END, -1, -1, -1, -1, 1<<WEATHER_CLOUDLESS,
     "The extreme heat from the sun dries and cracks the $g.",
     "The blazing sun blisters the $g."));
  component_placement.push_back(compPlace(12641, 12643, MOB_NONE,
     COMP_SAND_BLAST,
     CACT_REMOVE, -1, 100,
     -1, -1, -1, -1, -1, -1, (1<<WEATHER_RAINY | WEATHER_SNOWY | WEATHER_LIGHTNING),
     "Moisture hangs heavily in the air as bad weather approaches.", ""));

  component_placement.push_back(compPlace(12795, 12799, MOB_NONE,
     COMP_SAND_BLAST,
     CACT_PLACE, 9999, 50,
     HOUR_DAY_BEGIN, HOUR_DAY_END, -1, -1, -1, -1, 1<<WEATHER_CLOUDLESS,
     "The extreme heat from the sun dries and cracks the $g.", 
     "The blazing sun blisters the $g."));
  component_placement.push_back(compPlace(12795, 12799, MOB_NONE,
     COMP_SAND_BLAST,
     CACT_REMOVE, -1, 100,
     -1, -1, -1, -1, -1, -1, (1<<WEATHER_RAINY | WEATHER_SNOWY | WEATHER_LIGHTNING),
     "Moisture hangs heavily in the air as bad weather approaches.", ""));

// Some porous bedrock [stoneskin] //
  component_placement.push_back(compPlace(10990, 10992, MOB_NONE, 
     COMP_STONE_SKIN,
     CACT_PLACE | CACT_UNIQUE, 9999, 15,
     -1, -1, -1, -1, -1, -1, -1,
     "A gust of wind exposes $p.", ""));

  // A dryad's footprint [stealth] //
  component_placement.push_back(compPlace(7829, -1, MOB_NONE, 
     COMP_STEALTH,
     CACT_PLACE | CACT_UNIQUE, 9999, 80,
     -1, -1, -1, -1, -1, -1, ~(1<<WEATHER_RAINY | 1<<WEATHER_SNOWY),
     "", ""));
  component_placement.push_back(compPlace(7829, -1, MOB_NONE, 
     COMP_STEALTH,
     CACT_REMOVE, -1, 100,
     -1, -1, -1, -1, -1, -1, (1<<WEATHER_RAINY | 1<<WEATHER_SNOWY),
     "", ""));

  // A prism of condensation [concealment] //
  // created during daylight rain, destroyed by sun
  // nighttime has no effect on destruction 
  component_placement.push_back(compPlace(2712, 2717, MOB_NONE, 
     COMP_CLOUD_OF_CONCEAL,
     CACT_PLACE | CACT_UNIQUE, 9999, 80,
     HOUR_DAY_BEGIN, HOUR_DAY_END, -1, -1, -1, -1, (1<<WEATHER_RAINY),
     "Raindrops slowly fall from the leaves.",
     "The falling rain gathers on the foliage around you."));

  component_placement.push_back(compPlace(2712, 2717, MOB_NONE, 
     COMP_CLOUD_OF_CONCEAL,
     CACT_REMOVE, -1, 100,
     HOUR_DAY_BEGIN, HOUR_DAY_END, -1, -1, -1, -1, (1<<WEATHER_CLOUDLESS),
     "The bright heat from the sun warms the air around you.", ""));

  component_placement.push_back(compPlace(10101, 10159, MOB_NONE, 
     COMP_CLOUD_OF_CONCEAL,
     CACT_PLACE | CACT_UNIQUE, 9999, 80,
     HOUR_DAY_BEGIN, HOUR_DAY_END, -1, -1, -1, -1, (1<<WEATHER_RAINY),
     "Raindrops slowly fall from the leaves.", 
     "The falling rain gathers on the foliage around you."));

  component_placement.push_back(compPlace(10101, 10159, MOB_NONE, 
     COMP_CLOUD_OF_CONCEAL,
     CACT_REMOVE, -1, 100,
     HOUR_DAY_BEGIN, HOUR_DAY_END, -1, -1, -1, -1, (1<<WEATHER_CLOUDLESS),
     "The bright heat from the sun warms the air around you.", ""));

  // Some gnome flour [dispel invisible] //
  component_placement.push_back(compPlace(22480, -1, MOB_NONE, 
     COMP_DISPEL_INVIS,
     CACT_PLACE, 40, 90,
     -1, -1, -1, -1, -1, -1, -1,
     "", ""));

  // A bit of brain coral [telepathy] //
  component_placement.push_back(compPlace(4300, 4399, MOB_NONE, 
     COMP_TELEPATHY,
     CACT_PLACE | CACT_UNIQUE, 9999, 10,
     12, 25, -1, -1, -1, -1, -1,
     "$p washes up onto shore.",
     "The shallow waters churn, kicking up sediment and debris."));
  component_placement.push_back(compPlace(4300, 4399, MOB_NONE, 
     COMP_TELEPATHY,
     CACT_REMOVE, -1, 100,
     25, -1, -1, -1, -1, -1, -1,
     "The waterline recedes as the tides change.", ""));

  component_placement.push_back(compPlace(14111, 14115, MOB_NONE, 
     COMP_TELEPATHY,
     CACT_PLACE | CACT_UNIQUE, 9999, 10,
     40, 2, -1, -1, -1, -1, -1,
     "$p washes up onto shore.",
     "The shallow waters churn kicking up sediment and debris."));

  component_placement.push_back(compPlace(14111, 14115, MOB_NONE, 
     COMP_TELEPATHY,
     CACT_REMOVE, -1, 100,
     2, -1, -1, -1, -1, -1, -1,
     "The waterline recedes as the tides change.", ""));

// A jar of whale grease [fumble] //
  component_placement.push_back(compPlace(1293, -1, 1389, 
     COMP_FUMBLE,
     CACT_PLACE, 9999, 80,
     -1, -1, -1, -1, -1, -1, -1,
     "", ""));

  component_placement.push_back(compPlace(2794, -1, 2773, 
     COMP_FUMBLE,
     CACT_PLACE, 9999, 80,
     -1, -1, -1, -1, -1, -1, -1,
     "", ""));
// A piece of cloud stone [conjure air elemental] //
  component_placement.push_back(compPlace(11000, 11017, MOB_NONE, 
     COMP_CONJURE_AIR,
     CACT_PLACE | CACT_UNIQUE, 9999, 60,
     -1, -1, -1, -1, -1, -1, (1<<WEATHER_CLOUDY | 1<<WEATHER_RAINY),
     "Heavy winds buffet the clouds, sending $p across the $g.",
     "Howling winds scream in your ears."));

  component_placement.push_back(compPlace(11000, 11017, MOB_NONE, 
     COMP_CONJURE_AIR,
     CACT_REMOVE, -1, 100,
     -1, -1, -1, -1, -1, -1, ~(1<<WEATHER_CLOUDY | 1<<WEATHER_RAINY),
     "The winds begin to die down somewhat.", ""));

// A bit of cloud foam [fly] //
  component_placement.push_back(compPlace(11000, 11017, MOB_NONE, 
     COMP_FLIGHT,
     CACT_PLACE | CACT_UNIQUE, 9999, 40,
     -1, -1, -1, -1, -1, -1, (1<<WEATHER_RAINY),
     "The moist air creates $p on the $g.",
     "The $g becomes heavy with moisture."));

  component_placement.push_back(compPlace(11000, 11017, MOB_NONE, 
     COMP_FLIGHT,
     CACT_REMOVE, -1, 100,
     -1, -1, -1, -1, -1, -1, (1<<WEATHER_CLOUDY),
     "The clouds firm up as the rain dissipates.", ""));

// Some liquid brimstone [hellfire] //
  component_placement.push_back(compPlace(9994, -1, 9917, 
     COMP_HELLFIRE,
     CACT_PLACE | CACT_UNIQUE, 9999, 90,
     -1, -1, -1, -1, -1, -1, -1,
     "", ""));
  component_placement.push_back(compPlace(7515, -1, MOB_NONE, 
     COMP_HELLFIRE,
     CACT_PLACE | CACT_UNIQUE, 9999, 70,
     -1, -1, -1, -1, -1, -1, -1,
     "Some brimstone boils out of the earth and collects in a nearby pot.", ""));

  // Brown mushrooms to trade for promethian fire
  // created (revealed) at first morning light
  component_placement.push_back(compPlace(3401, 3449, MOB_NONE, 
     BROWN_MUSHROOM,
     CACT_PLACE, 9999, 10,
     5, -1, -1, -1, -1, -1, -1,
     "The morning light reveals $p growing in the $g.", ""));
  component_placement.push_back(compPlace(3451, 3482, MOB_NONE, 
     BROWN_MUSHROOM,
     CACT_PLACE, 9999, 10,
     5, -1, -1, -1, -1, -1, -1,
     "The morning light reveals $p growing in the $g.", ""));

#if 0
// Please talk to Batopr before enabling these
// mob load doesn't work well, so maybe should be dissect...

// Some eyes from a blind man [sense life] //
  component_placement.push_back(compPlace(1, 99, 7800, 
     COMP_BLIND_EYE,
     CACT_PLACE, 40, 70,
     -1, -1, -1, -1, -1, -1, -1,
     "", ""));

// A golden carrot [true sight] //
  component_placement.push_back(compPlace(10117, 10128, 10108, 
     COMP_GOLDEN_CARROT,
     CACT_PLACE, 40, 15,
     -1, -1, -1, -1, -1, -1, -1,
     "", ""));

// A bag of pixie dust [slumber] //
  component_placement.push_back(compPlace(10258, 10299, 7817, 
     COMP_PIXIE_DUST,
     CACT_PLACE, 40, 80,
     -1, -1, -1, -1, -1, -1, -1,
     "", ""));

  component_placement.push_back(compPlace(10258, 10299, 7805, 
     COMP_PIXIE_DUST,
     CACT_PLACE, 40, 80,
     -1, -1, -1, -1, -1, -1, -1,
     "", ""));

  component_placement.push_back(compPlace(2700, 2740, 7805, 
     COMP_PIXIE_DUST,
     CACT_PLACE, 40, 80,
     -1, -1, -1, -1, -1, -1, -1,
     "", ""));

  component_placement.push_back(compPlace(2700, 2740, 7805, 
     COMP_PIXIE_DUST,
     CACT_PLACE, 40, 80,
     -1, -1, -1, -1, -1, -1, -1,
     "", ""));

// A pixie torch [faerie fire] //
  component_placement.push_back(compPlace(10258, 10299, 7805, 
     COMP_PIXIE_TORCH,
     CACT_PLACE, 40, 85,
     -1, -1, -1, -1, -1, -1, -1,
     "", ""));
  component_placement.push_back(compPlace(10258, 10299, 7805, 
     COMP_PIXIE_TORCH,
     CACT_PLACE, 40, 85,
     -1, -1, -1, -1, -1, -1, -1,
     "", ""));
  component_placement.push_back(compPlace(10258, 10299, 7805, 
     COMP_PIXIE_TORCH,
     CACT_PLACE, 40, 85,
     -1, -1, -1, -1, -1, -1, -1,
     "", ""));

// A dropper of pixie tears [faerie fog] //
  component_placement.push_back(compPlace(199, 245, 7805, 
     COMP_PIXIE_TEAR,
     CACT_PLACE, 40, 50,
     -1, -1, -1, -1, -1, -1, -1,
     "", ""));

  component_placement.push_back(compPlace(199, 245, 7805, 
     COMP_PIXIE_TEAR,
     CACT_PLACE, 40, 50,
     -1, -1, -1, -1, -1, -1, -1,
     "", ""));
  component_placement.push_back(compPlace(199, 245, 7805, 
     COMP_PIXIE_TEAR,
     CACT_PLACE, 40, 50,
     -1, -1, -1, -1, -1, -1, -1,
     "", ""));
  component_placement.push_back(compPlace(199, 245, 7805, 
     COMP_PIXIE_TEAR,
     CACT_PLACE, 40, 50,
     -1, -1, -1, -1, -1, -1, -1,
     "", ""));
  component_placement.push_back(compPlace(199, 245, 7805, 
     COMP_PIXIE_TEAR,
     CACT_PLACE, 40, 50,
     -1, -1, -1, -1, -1, -1, -1,
     "", ""));
  component_placement.push_back(compPlace(199, 245, 7805, 
     COMP_PIXIE_TEAR,
     CACT_PLACE, 40, 50,
     -1, -1, -1, -1, -1, -1, -1,
     "", ""));
  component_placement.push_back(compPlace(199, 245, 7805, 
     COMP_PIXIE_TEAR,
     CACT_PLACE, 40, 50,
     -1, -1, -1, -1, -1, -1, -1,
     "", ""));
  component_placement.push_back(compPlace(199, 245, 7805, 
     COMP_PIXIE_TEAR,
     CACT_PLACE, 40, 50,
     -1, -1, -1, -1, -1, -1, -1,
     "", ""));
  component_placement.push_back(compPlace(199, 245, 7805, 
     COMP_PIXIE_TEAR,
     CACT_PLACE, 40, 50,
     -1, -1, -1, -1, -1, -1, -1,
     "", ""));
  component_placement.push_back(compPlace(199, 245, 7805, 
     COMP_PIXIE_TEAR,
     CACT_PLACE, 40, 50,
     -1, -1, -1, -1, -1, -1, -1,
     "", ""));

#endif
}

void do_components(int situ)
{
  unsigned int i;
  int j;
  TRoom *rp = NULL;
  TThing *m = NULL, *temp = NULL, *o = NULL;
  TThing *t, *t2;
  TObj *obj = NULL;
  int value;
  int l_room, t_room;
  int found;
  int start, stop;

  // situ == -1 indicates a tick pulse
  // situ == 0-6 indicates a weather change
  //    1 = getting cloudy, 2 = rain/snow start
  //    3 = clouds disappear, 4 = blizzard/lightning start
  //    5 = rain/snow stop, 6 = bliz/light stop
  if (situ != -1)
    return;

  for (i = 0; i < component_placement.size(); i++) {
    if ((value = real_object(component_placement[i].number)) < 0) {
      vlogf(LOG_MISC, fmt("Bad component (%d, %d)") %  i % component_placement[i].number);
      continue;
    }

    // Stupid hack to make this function work.  I don't want to debug this
    // anymore.
    if (false) {
      vlogf(LOG_SILENT, fmt("Trying to place object %d") % component_placement[i].number);
    }

    bool placed = FALSE;

    // check hour info
    start = component_placement[i].hour1;
    stop = component_placement[i].hour2;
    if (start == HOUR_SUNRISE)
      start = sunTime(SUN_TIME_RISE);
    else if (start == HOUR_SUNSET)
      start = sunTime(SUN_TIME_SET);
    else if (start == HOUR_MOONRISE)
      start = moonTime(MOON_TIME_RISE);
    else if (start == HOUR_MOONSET)
      start = moonTime(MOON_TIME_SET);
    else if (start == HOUR_DAY_BEGIN)
      start = sunTime(SUN_TIME_DAY);
    else if (start == HOUR_DAY_END)
      start = sunTime(SUN_TIME_SINK);
    if (stop == HOUR_SUNRISE)
      stop = sunTime(SUN_TIME_RISE);
    else if (stop == HOUR_SUNSET)
      stop = sunTime(SUN_TIME_SET);
    else if (stop == HOUR_MOONRISE)
      stop = moonTime(MOON_TIME_RISE);
    else if (stop == HOUR_MOONSET)
      stop = moonTime(MOON_TIME_SET);
    else if (stop == HOUR_DAY_BEGIN)
      stop = sunTime(SUN_TIME_DAY);
    else if (stop == HOUR_DAY_END)
      stop = sunTime(SUN_TIME_SINK);

    if (start != -1) {
      if (stop == -1) {
        // only the one hour was specified
        if (start != time_info.hours)
          continue;
      } else if (time_info.hours < start)
        continue;  // too early
      else if (time_info.hours >= stop)
        continue;  // too late
    }

    // check day data
    if (component_placement[i].day1 != -1) {
      if (component_placement[i].day2 == -1) {
        // only the one day was specified
        if (component_placement[i].day1 != time_info.day)
          continue;
      } else if (time_info.day < component_placement[i].day1)
        continue;  // too early
      else if (time_info.day >= component_placement[i].day2)
        continue;  // too late
    }

    // check month data
    if (component_placement[i].month1 != -1) {
      if (component_placement[i].month2 == -1) {
        // only the one month was specified
        if (component_placement[i].month1 != time_info.month)
          continue;
      } else if (time_info.month < component_placement[i].month1)
        continue;  // too early
      else if (time_info.month >= component_placement[i].month2)
        continue;  // too late
    }

    if (component_placement[i].room2 == -1)
      l_room = component_placement[i].room1;
    else {
      // set ACTUAL l_room
      j = 0;
      do {
        l_room = ::number(component_placement[i].room1, component_placement[i].room2);
        j++;
     
      } while (!real_roomp(l_room) && j < 20);
      if (j >= 20)
        continue;
    }

    if ((rp = real_roomp(l_room))) {
      // check weather condition
      if (component_placement[i].weather != -1) {
        if (!IS_SET(component_placement[i].weather, (1<<rp->getWeather())))
          continue;
      }

      if (IS_SET(component_placement[i].place_act, CACT_PLACE)) {

        // first, send message to all rooms
        if (*component_placement[i].glo_msg) {
          for (t_room = component_placement[i].room1; t_room <= component_placement[i].room2; t_room++) {
            if ((rp = real_roomp(t_room)) && rp->getStuff()) {
              act(component_placement[i].glo_msg,
                    TRUE, rp->getStuff(), NULL, NULL, TO_CHAR);
              act(component_placement[i].glo_msg, 
                    TRUE, rp->getStuff(), NULL, NULL, TO_ROOM);
            }
          }
          rp = real_roomp(l_room);  // reset rp pointer
        }
        // check variance here, so global messages still shown regardless
        if (::number(1,100) > component_placement[i].variance)
          continue;

        if ((component_placement[i].max_number <= 0) ||
            (obj_index[value].getNumber() < component_placement[i].max_number)) {

          // uniqueness check
          if (IS_SET(component_placement[i].place_act, CACT_UNIQUE)) {
            found = FALSE;
            if (component_placement[i].mob) {
              // limit to 1 on any 1 mob in room
              for (m = rp->getStuff(); m && !found; m = m->nextThing) {
                TMonster *tmon = dynamic_cast<TMonster *>(m);
                if (!tmon)
                  continue;
                if (!tmon->isPc() && 
                    (tmon->mobVnum() == component_placement[i].mob)) {
                  for (t = tmon->getStuff(); t; t = t->nextThing) {
                    TObj *tobj = dynamic_cast<TObj *>(t);
                    if (!tobj)
                      continue;
                    if (tobj->objVnum() == component_placement[i].number) {
                      found = TRUE;
                      break;
                    }
                  }
                }
              }
              if (found) 
                continue;
            } else {
              // limit to 1 in the room
              for (t = rp->getStuff(); t; t = t->nextThing) {
                TObj *tobj = dynamic_cast<TObj *>(t);
                if (!tobj)
                  continue;
                if (tobj->objVnum() == component_placement[i].number) {
                  found = TRUE;
                  break;
                }
              }
              if (found) 
                continue;
            }
          }

          if (!(obj = read_object(component_placement[i].number, VIRTUAL)))
            continue;
          placed = FALSE;
          // if mob ALSO specified, place it on mob in room
          if (component_placement[i].mob) {
            for (m = rp->getStuff(); m; m = m->nextThing) {
              TMonster *tmon = dynamic_cast<TMonster *>(m);
              if (!tmon)
                continue;
        
              if (!tmon->isPc() && 
                  (tmon->mobVnum() == component_placement[i].mob)) {
                *tmon += *obj;
                placed = TRUE;
                vlogf(LOG_SILENT, fmt("Placing object %d on mob (room %d)") % component_placement[i].number % l_room);
                break;
              }
            }
            if (!placed) {
              delete obj;
              obj = NULL;
            }
          } else {
            m = NULL;
            *rp += *obj;
            placed = TRUE;
            vlogf(LOG_SILENT, fmt("Placing object %d on ground (room %d)") % component_placement[i].number % l_room);
          }
        }
        if (placed && *component_placement[i].message && rp->getStuff()) {
          act(component_placement[i].message, TRUE, rp->getStuff(), obj, m, TO_CHAR);
          act(component_placement[i].message, TRUE, rp->getStuff(), obj, m, TO_ROOM);

          if (component_placement[i].sound != SOUND_OFF)
            rp->playsound(component_placement[i].sound, SOUND_TYPE_NOISE, 100, 5, component_placement[i].sound_loop);
        }
      } else if (IS_SET(component_placement[i].place_act, CACT_REMOVE)) {
        // removing the object
        // check variance
        if (::number(1,100) > component_placement[i].variance)
          continue;

        found = 0;
        for (t_room = component_placement[i].room1;
             (t_room <= component_placement[i].room2 || t_room == component_placement[i].room1) && (found < component_placement[i].max_number || component_placement[i].max_number < 0);
             t_room++) {
          placed = FALSE;
          if ((rp = real_roomp(t_room))) {
            if (component_placement[i].mob) {
              for (m = rp->getStuff(); m; m = m->nextThing) {
                TMonster *tmon = dynamic_cast<TMonster *>(m);
                if (!tmon)
                  continue;
                if (!tmon->isPc() &&
                     tmon->mobVnum() == component_placement[i].mob) {
                  for (t = tmon->getStuff(); t; t = t2) {
                    t2 = t->nextThing;
                    TObj * tobj = dynamic_cast<TObj *>(t);
                    if (!tobj)
                      continue;
                    if (tobj->objVnum() == component_placement[i].number) {
                      if (!placed && *component_placement[i].message && rp->getStuff()) {
                        act(component_placement[i].message,
                                TRUE, rp->getStuff(), tobj, tmon, TO_CHAR);
                        act(component_placement[i].message,
                                TRUE, rp->getStuff(), tobj, tmon, TO_ROOM);
                      }
                      placed = TRUE;
                      found++;
                      delete tobj;
                      t2 = tmon->getStuff();
                      vlogf(LOG_SILENT, fmt("Removing object %d from mob (room %d)") % component_placement[i].number % t_room);
                      continue;
                    }
                  }
                }
              }
            } else {
              for (o = rp->getStuff(); o; o = temp) {
                temp = o->nextThing;
                TObj *to = dynamic_cast<TObj *>(o);
                if (to) {
                  if (to->objVnum() == component_placement[i].number) {
                    if (!placed && *component_placement[i].message && rp->getStuff()) {
                      act(component_placement[i].message,
                             TRUE, rp->getStuff(), to, NULL, TO_CHAR);
                      act(component_placement[i].message,
                             TRUE, rp->getStuff(), to, NULL, TO_ROOM);
                    }
                    delete to;
                    placed = TRUE;
                    found++;
                    vlogf(LOG_SILENT, fmt("Removing object %d from ground (room %d)") % component_placement[i].number % t_room);
                    continue;
                  }
                }
              }
            }
          } // else no room
        } // end of for loop
      } else {
        vlogf(LOG_MISC, fmt("No direction specified on component placement (%d)") %  i);
      }
    } else {
      vlogf(LOG_MISC, fmt("No room specified on component placement (%d)") %  i);
    }
  }
}

void buildComponentArray()
{
  CompInfo.push_back(compInfo( SPELL_IMMOBILIZE,
    "You twirl $p around your finger as you point at $N.",
    "$n twirls $p around $s finger as $e points at $N.",
    "$n twirls $p around $s finger as $e points at you.",
    "You twirl $p around your finger as you point at yourself.",
    "$n twirls $p around $s finger as $e points at $mself.",
    "",
    ""));
  CompInfo.push_back(compInfo(SPELL_SUFFOCATE,
    "You heave $p at $N.",
    "$n heaves $p at $N.",
    "$n heaves $p at you.",
    "You heave $p over your head.",
    "$n heaves $p over $s head.",
    "",
    ""));
  CompInfo.push_back(compInfo(SPELL_DUST_STORM,
    "",                       
    "",
    "",
    "You open $p allowing a bit of vapor to seep out.",
    "$n opens $p allowing a bit of vapor to seep out.",
    "",
    ""));
  CompInfo.push_back(compInfo(SPELL_TORNADO,
    "",                       
    "",
    "",
    "You open $p, unleashing a tiny vortex.",
    "$n opens $p, unleashing a tiny vortex.",
    "",
    ""));
  CompInfo.push_back(compInfo(SPELL_CONJURE_AIR,
    "",
    "",
    "",
    "You toss $p upward and it forms into a small grey cloud.",
    "$n tosses $p upward and it forms into a cloud.",
    "",                       
    ""));
  CompInfo.push_back(compInfo(SPELL_FEATHERY_DESCENT,
    "You fling $p at $N.",
    "$n flings $p at $N.",
    "$n flings $p at you.",                       
    "You fling $p at yourself.",
    "$n flings $p at $mself.",
    "",
    ""));
  CompInfo.push_back(compInfo(SPELL_FALCON_WINGS,
    "You run $p up and down $N's arms.",
    "$n runs $p up and down $N's arms.",
    "$n runs $p up and down your arms.",                       
    "You run $p up and down your arms.",
    "$n runs $p up and down $s arms.",
    "",
    ""));
  CompInfo.push_back(compInfo(SPELL_ANTIGRAVITY,
    "",
    "",
    "",
    "You chew on $p.",
    "$n chews on $p.",
    "",
    ""));
  CompInfo.push_back(compInfo(SPELL_IDENTIFY,
    "You pop $p into your eye and examine $N.",
    "$n pops $p into $s eye and examines $N.",
    "$n pops $p into $s eye and examines you.",
    "You pop $p into your eye.",
    "$n pops $p into $s eye.",
    "",
    ""));
  CompInfo.push_back(compInfo(SPELL_DIVINATION,
    "You hold $p high, and aim it at $N.",
    "$n holds $p high, and aims it at $N.",
    "$n holds $p high, and aims it at you.",
    "You hold $p high over your head.",
    "$n holds $p high over $s head.",
    "",
    ""));
  CompInfo.push_back(compInfo(SPELL_EYES_OF_FERTUMAN,
    "",
    "",
    "",
    "You allow $p to spin.",
    "$n allows $p to spin.",
    "",
    ""));
  CompInfo.push_back(compInfo(SPELL_POWERSTONE,
    "",
    "",
    "",
    "",
    "",
    "You crush $p between your fingers and rub it over $N.",
    "$n crushes $p between $s fingers and rubs it over $N."));
  CompInfo.push_back(compInfo(SPELL_SHATTER,
    "",
    "",
    "",
    "You blow into $p causing a high pitched scream to be emitted.",
    "$n blows into $p causing a high pitched scream to be emitted.",
    "",
    ""));
  CompInfo.push_back(compInfo(SPELL_ILLUMINATE,
    "",                       
    "",
    "",
    "",
    "",
    "You crumple $p and scatter the dust over $N.",
    "$n crumples $p and scatters the dust over $N."));
  CompInfo.push_back(compInfo(SPELL_DETECT_MAGIC,
    "You toss $p onto $N.",
    "$n tosses $p onto $N.",
    "$n tosses $p onto you.",                       
    "You toss $p on yourself.",
    "$n tosses $p onto $mself.",
    "",
    ""));
  CompInfo.push_back(compInfo(SPELL_DISPEL_MAGIC,
    "",
    "",
    "",
    "You squeeze $p tightly.",
    "$n squeezes $p.",
    "You squeeze $p tightly.",
    "$n squeezes $p."));
  CompInfo.push_back(compInfo(SPELL_CHASE_SPIRIT,
    "",
    "",
    "",
    "You swallow $p.",
    "$n swallows $p.",
    "You swallow $p.",
    "$n swallows $p."));
  CompInfo.push_back(compInfo(SPELL_COPY,
    "",                       
    "",
    "",
    "",
    "",
    "You stretch $p between your forefingers.",
    "$n stretches $p between $s forefingers."));
  CompInfo.push_back(compInfo(SPELL_EMBALM,
    "",
    "",
    "",
    "You uncap $p and sprinkle the corpse with it.",
    "$n uncaps $p and sprinkles the corpse with it.",
    "You uncap $p and sprinkle the corpse with it.",
    "$n uncaps $p and sprinkles the corpse with it."));
  CompInfo.push_back(compInfo(SPELL_VOODOO,
    "",                       
    "",
    "",
    "You taste $p and spit it out.",
    "$n tastes $p and then $e spits it out.",
    "You taste $p and spit it out.",
    "$n tastes $p and then $e spits it out."));
  CompInfo.push_back(compInfo(SPELL_DANCING_BONES,
    "",                       
    "",
    "",
    "You blow $p into the air.",
    "$n blows $p into the air.",
    "You blow $p into the air.",
    "$n blows $p into the air."));
  CompInfo.push_back(compInfo(SPELL_RESURRECTION,
    "",                       
    "",
    "",
    "You toss $p into the air and it vanishes.",
    "$n tosses $p into the air and it vanishes.",
    "You toss $p into the air and it vanishes.",
    "$n tosses $p into the air and it vanishes."));
  CompInfo.push_back(compInfo(SPELL_GALVANIZE,
    "",                       
    "",
    "",
    "",
    "",
    "You drop $p and command it to begin heating.",
    "$n drops $p and issues a command word."));
  CompInfo.push_back(compInfo(SPELL_GRANITE_FISTS,
    "You tightly squeeze $p as you stare at $N.",
    "$n tightly squeezes $p as $e stares at $N.",
    "$n tightly squeezes $p as $e stares at you.",                       
    "You tightly squeeze $p as your eyes roll back into your head.",
    "$n tightly squeezes $p as $s eyes roll back into $s head.",
    "",
    ""));
  CompInfo.push_back(compInfo(SPELL_METEOR_SWARM,
    "You toss $p at $N.  The stone floats skywards before it can hit $M.",
    "$n tosses $p at $N.  The stone floats skywards before it can hit $M.",
    "$n tosses $p at you, but it soars into the sky before it can strike you.",
    "You toss $p at yourself.  The stone floats skywards before it can hit you.",
    "$n tosses $p at $mself.  The stone floats skywards before it can hit $m.",
    "",
    ""));
  CompInfo.push_back(compInfo(SPELL_PEBBLE_SPRAY,
    "",                       
    "",
    "",
    "You press $p to the $g.",
    "$n presses $p to the $g.",
    "",
    ""));
  CompInfo.push_back(compInfo(SPELL_FLATULENCE,
    "",                       
    "",
    "",
    "You quickly eat $p.",
    "$n quickly eats $p.",
    "",
    ""));
  CompInfo.push_back(compInfo(SPELL_SAND_BLAST,
    "",                       
    "",
    "",
    "You pour a portion of sand from $p.",
    "$n pours a bit of sand from $p.",
    "",
    ""));
  CompInfo.push_back(compInfo(SPELL_LAVA_STREAM,
    "",
    "",
    "",
    "You press $p between your hands.",
    "$n presses $p between $s hands.",
    "",
    ""));
  CompInfo.push_back(compInfo(SPELL_DEATH_MIST,
    "",
    "",
    "",
    "You eat $p and begin to choke on them.",
    "$n eats $p and begins to choke on them.",
    "",
    ""));
  CompInfo.push_back(compInfo(SPELL_STONE_SKIN,
    "You rub $p upon $N.",
    "$n rubs $p on $N.",
    "$n rubs $p on you.",                       
    "You rub $p on yourself.",
    "$n rubs $p on $mself.",
    "",
    ""));
  CompInfo.push_back(compInfo(SPELL_TRAIL_SEEK,
    "You touch $p to $N's nose.",
    "$n touches $p to $N's nose.",
    "$n touches $p to your nose.",                       
    "You touch $p to your nose.",
    "$n touches $p to $s nose.",
    "",
    ""));
  CompInfo.push_back(compInfo(SPELL_LEVITATE,
    "You uncork $p and hold it beneath $N's nose.",
    "$n uncorks $p and holds it beneath $N's nose.",
    "$n uncorks $p and holds it beneath your nose.",                       
    "You uncork $p and sniff it.",
    "$n uncorks $p and sniffs it.",
    "",
    ""));
  CompInfo.push_back(compInfo(SPELL_CONJURE_EARTH,
    "",                       
    "",
    "",
    "You crush $p in your hand.",
    "$n crushes $p in $s hand.",
    "",
    ""));
  CompInfo.push_back(compInfo(SPELL_FAERIE_FIRE,
    "You hold $p between two fingers and look at $N.",
    "$n holds $p between two fingers and looks at $N.",
    "$n holds $p between two fingers while looking at you.",
    "You hold $p between two fingers and close your eyes.",
    "$n holds $p between two fingers and closes $s eyes.",
    "",
    ""));
  CompInfo.push_back(compInfo(SPELL_STUPIDITY,
    "You repeat $p and look at $N.",
    "$n repeats $p and looks at $N.",
    "$n repeats $p while looking at you.",
    "You repeat $p and close your eyes.",
    "$n repeats $p and closes $s eyes.",
    "",
    ""));
  CompInfo.push_back(compInfo(SPELL_FLAMING_SWORD,
    "You grip $p, slashing at the air before $N.",
    "$n grips $p, slashing at the air before $N.",
    "$n grips $p, slashing it at the air before you.",                       
    "You grip $p, slashing at the air around you.",
    "$n grips $p, slashing at the air around $m.",
    "",
    ""));
  CompInfo.push_back(compInfo(SPELL_INFERNO,
    "You blow $p in $N's direction.",
    "$n blows $p in $N's direction.",
    "$n blows $p in your direction.",                       
    "You blow $p in a cloud about yourself.",
    "$n blows $p in a cloud about $mself.",
    "",
    ""));
  CompInfo.push_back(compInfo(SPELL_FIREBALL,
    "",                       
    "",
    "",
    "You scatter $p into the air.",
    "$n scatters $p into the air.",
    "",
    ""));
  CompInfo.push_back(compInfo(SPELL_HELLFIRE,
    "",                       
    "",
    "",
    "You pour $p into your palm.",
    "$n pours $p into your palm.",
    "",
    ""));
  CompInfo.push_back(compInfo(SPELL_CONJURE_FIRE,
    "",                       
    "",
    "",
    "You touch $p to your forehead.",
    "$n touches $p to $s forehead.",
    "",
    ""));
  CompInfo.push_back(compInfo(SPELL_FLAMING_FLESH,
    "You apply $p to $N.",
    "$n applies $p to $N.",
    "$n applies $p to you.",                       
    "You apply $p to yourself.",
    "$n applies $p to $mself.",
    "",
    ""));
  CompInfo.push_back(compInfo(SPELL_CLEANSE,
    "You wash $N's private area with $p! There HAS to be a better way!",
    "ACK!! $n washes $N's privates with $p! YUCKO!!",
    "$n washes your privates with $p. ACK!!!",                       
    "You wash *cough* yourself with $p. Sheesh! How about some damn privacy!",
    "$n washes $mself in a private area with $p for a DAMN LONG TIME!",
    "",
    ""));
  CompInfo.push_back(compInfo(SPELL_FLARE,
    "",                       
    "",
    "",
    "You take a globe from $p launching it upwards.",
    "$n launches a small globe of sodium vapor upwards.",
    "",
    ""));
  CompInfo.push_back(compInfo(SPELL_STUNNING_ARROW,
    "You wrap $p around your wrist and reach for $N.",
    "$n wraps $p around a wrist and reaches for $N.",
    "$n wraps $p around a wrist and reaches for you.",                       
    "You wrap $p around your wrist and grab yourself.",
    "$n wraps $p around a wrist and grabs $mself.",
    "",
    ""));
  CompInfo.push_back(compInfo(SPELL_BLAST_OF_FURY,
    "You lick $p and spit it at $N.",
    "$n licks $p and spits it at $N.",
    "$n licks $p and spits it at you.",                       
    "You lick $p and start to drool.",
    "$n licks $p and drools on $mself.",
    "",
    ""));
  CompInfo.push_back(compInfo(SPELL_ENERGY_DRAIN,
    "You wave $p at $N.",
    "$n waves $p at $N.",
    "$n waves $p at you.",                       
    "You wave $p at yourself.",
    "$n waves $p at $mself.",
    "",
    ""));
  CompInfo.push_back(compInfo(SPELL_LICH_TOUCH,
    "You fling $p at $N.",
    "$n flings $p at $N.",
    "$n flings $p at you.",                       
    "You fling $p at yourself.",
    "$n flings $p at $mself.",
    "",
    ""));
  CompInfo.push_back(compInfo(SPELL_SQUISH,
    "You squeeze $p tightly.",
    "$n squeezes $p tightly.",
    "$n squeezes $p tightly.",                       
    "You squeeze $p tightly.",
    "$n squeezes $p tightly.",
    "",
    ""));
  CompInfo.push_back(compInfo(SPELL_CARDIAC_STRESS,
    "You chew up $p and then kiss $N deeply on the mouth.",
    "$n chews up $p and then kisses $N deeply on the mouth.",
    "$n chews up $p and then kisses you deeply on the mouth.",                       
    "You chew up $p and accidentally swallow the poisonous juice.",
    "$n chews up $p and accidentally swallows it.",
    "",
    ""));
  CompInfo.push_back(compInfo(SPELL_SOUL_TWIST,
    "You wrap $p around your wrist and reach for $N.",
    "$n wraps $p around a wrist and reaches for $N.",
    "$n wraps $P around a wrist and reaches for you.",                       
    "You wrap $p around your wrist and grab yourself.",
    "$n wraps $p around a wrist and grabs $mself.",
    "",
    ""));
  CompInfo.push_back(compInfo(SPELL_VAMPIRIC_TOUCH,
    "You consume $p and spit it on $N.",
    "$n consumes $p and spits it on $N.",
    "$n consumes $p and spits it on you.",                       
    "You consume $p and accidentally swallow it.",
    "$n consumes $p and swallows it down.",
    "",
    ""));
  CompInfo.push_back(compInfo(SPELL_BLOOD_BOIL,
    "You spit $p at $N.",
    "$n spits $p at $N.",
    "$n spits $p at you.",                       
    "You swallow $p and start to drool.",
    "$n swallows $p and drools on $mself.",
    "",
    ""));
  CompInfo.push_back(compInfo(SPELL_LIFE_LEECH,
    "You throw $p onto $N's chest.",
    "$n throws $p onto $N's chest.",
    "$n throws $p onto your chest!",                       
    "You fumble $p and it latches on to you.",
    "$n fumbles $p and it latches tightly to $s chest.",
    "",
    ""));
  CompInfo.push_back(compInfo(SPELL_ATOMIZE,
    "You blow $p at $N.",
    "$n blows $p at $N.",
    "$n blows $p at you.",                       
    "You blow $p at yourself.",
    "$n blows $p at $mself.",
    "",
    ""));
  CompInfo.push_back(compInfo(SPELL_DEATHWAVE,
    "You blow $p at $N.",
    "$n blows $p at $N.",
    "$n blows $p at you.",                       
    "You blow $p at yourself.",
    "$n blows $p at $mself.",
    "",
    ""));
  CompInfo.push_back(compInfo(SPELL_COLOR_SPRAY,
    "",                       
    "",
    "",
    "You squeeze $p and wave it over your head.",
    "$n squeezes $p and waves it over $s head.",
    "",
    ""));
  CompInfo.push_back(compInfo(SPELL_ACID_BLAST,
    "",                       
    "",
    "",
    "You uncork $p.",
    "$n uncorks $p.",
    "",
    ""));
  CompInfo.push_back(compInfo(SPELL_ANIMATE,
    "",                       
    "",                       
    "",
    "You toss $p into the pile of armor.",
    "$n tosses $p into the pile of armor.",
    "",
    ""));
  CompInfo.push_back(compInfo(SPELL_CONTROL_UNDEAD,
    "",                       
    "",                       
    "",
    "You toss $p on the ground.",
    "$n tosses $p on the ground.",
    "",
    ""));
  CompInfo.push_back(compInfo(SPELL_SHIELD_OF_MISTS,
    "You masticate $p and spit it at $N.",
    "$n masticates $p and spits it at $N.",
    "$n masticates $p and spits it at you.",                       
    "You chew $p and swallow it down.",
    "$n chews $p and swallows it down.",
    "",
    ""));
  CompInfo.push_back(compInfo(SPELL_SORCERERS_GLOBE,
    "You twirl $p in front of $N.",
    "$n twirls $p in front of $N.",
    "$n twirls $p in front of you.",                       
    "You twirl $p in front of yourself.",
    "$n twirls $p in front of $mself.",
    "",
    ""));
  CompInfo.push_back(compInfo(SPELL_BIND,
    "You drape $p over a finger and point at $N.",
    "$n drapes $p over a finger and points at $N.",
    "$n drapes $p over a finger and points at you.",                       
    "You drape $p over a finger and point at yourself.",
    "$n drapes $p over a finger and points at $mself.",
    "",
    ""));
  CompInfo.push_back(compInfo(SPELL_TELEPORT,
    "You crumble $p and toss it at $N.",
    "$n crumbles $p and tosses it at $N.",
    "$n crumbles $p and tosses it at you.",                       
    "You crumble $p and toss it at yourself.",
    "$n crumbles $p and tosses it at $mself.",
    "",
    ""));
#if 0
  CompInfo.push_back(compInfo(SPELL_FIND_FAMILIAR,
    "",                       
    "",
    "",
    "You clutch $p and stare off into space.",
    "$n clutches $p and stares off into space.",
    "",
    ""));
#endif
  CompInfo.push_back(compInfo(SPELL_SENSE_LIFE,
    "You touch $p to $N's head.",
    "$n touches $p to $N's head.",
    "$n touches $p to your head.",                       
    "You touch $p to your head.",
    "$n touches $p to $s head.",
    "",
    ""));
  CompInfo.push_back(compInfo(SPELL_SENSE_LIFE_SHAMAN,
    "You touch $p to $N's head.",
    "$n touches $p to $N's head.",
    "$n touches $p to your head.",                       
    "You touch $p to your head.",
    "$n touches $p to $s head.",
    "",
    ""));
  CompInfo.push_back(compInfo(SPELL_FARLOOK,
    "",                       
    "",
    "",
    "You look into $p.",
    "$n looks into $p.",
    "",
    ""));
  CompInfo.push_back(compInfo(SPELL_SILENCE,
    "You unleash $p at $N.",
    "$n unleashes $p at $N.",
    "$n unleashes $p at you.",                       
    "You unleash $p upon yourself.",
    "$n unleashes $p at $mself.",
    "",
    ""));
  CompInfo.push_back(compInfo(SPELL_STEALTH,
    "You place $p on $N's sole.",
    "$n places $p on $N's sole.",
    "$n places $p on your sole.",                       
    "You place $p on your sole.",
    "$n places $p on $s sole.",
    "",
    ""));
  CompInfo.push_back(compInfo(SPELL_CALM,
    "You hold up $p at $N.",
    "$n holds up $p at $N.",
    "$n holds up $p at you.",                       
    "You hold up $p.",
    "$n holds up $p.",
    "",
    ""));
  CompInfo.push_back(compInfo(SPELL_ENSORCER,
    "You tie $p in a knot while thinking of $N.",
    "$n ties $p in a knot.",
    "$n ties $p in a knot.",                       
    "You tie $p in a knot while thinking of yourself.",
    "$n ties $p in a knot.",
    "",
    ""));
  CompInfo.push_back(compInfo(SPELL_INTIMIDATE,
    "You flick $p at $N.",
    "$n flicks $p at $N.",
    "$n flicks $p at you.",                       
    "You rub $p on your face? YUCK!",
    "$n rubs $p on $s face! What a dweeb!",
    "",
    ""));
  CompInfo.push_back(compInfo(SPELL_FEAR,
    "You throw $p at $N.",
    "$n throws $p at $N.",
    "$n throws $p at you.",                       
    "You throw $p on the ground.",
    "$n throws $p on the ground.",
    "",
    ""));
  CompInfo.push_back(compInfo(SPELL_INVISIBILITY,
    "You douse $N with $p.",
    "$n douses $N with $p.",
    "$n douses you with $p.",                       
    "You douse yourself with $p.",
    "$n douses $mself with $p.",
    "",
    ""));
  CompInfo.push_back(compInfo(SPELL_CLOUD_OF_CONCEALMENT,
    "",                       
    "",
    "",
    "You hold $p before your group, bending the light around it.",
    "$n holds $p before $s group, bending the light around it.",
    "",
    ""));
  CompInfo.push_back(compInfo(SPELL_DETECT_INVISIBLE,
    "You toss $p at $N.",
    "$n tosses $p at $N.",
    "$n tosses $p at you.",                       
    "You toss $p on yourself.",
    "$n tosses $p on $mself.",
    "",
    ""));
  CompInfo.push_back(compInfo(SPELL_DETECT_SHADOW,
    "You sprinkle $p on $N.",
    "$n sprinkles $p on $N.",
    "$n sprinkles $p on you.",                       
    "You sprinkle $p on yourself.",
    "$n sprinkles $p on $mself.",
    "",
    ""));
  CompInfo.push_back(compInfo(SPELL_DISPEL_INVISIBLE,
    "You scatter $p near $N.",
    "$n scatters $p near $N.",
    "$n scatters $p near you.",                       
    "You scatter $p on the $g.",
    "$n scatters $p on the $g at $s feet.",
    "",
    ""));
  CompInfo.push_back(compInfo(SPELL_TELEPATHY,
    "",                       
    "",
    "",
    "You touch $p to your forehead.",
    "$n touches $p to $s forehead.",
    "",
    ""));
  CompInfo.push_back(compInfo(SPELL_ROMBLER,
    "",                       
    "",
    "",
    "You stretch out $p.",
    "$n stretches out $p.",
    "",
    ""));
  CompInfo.push_back(compInfo(SPELL_RAZE,
    "You toss $p at $N.",
    "$n tosses $p at $N.",
    "$n tosses $p at you.",                       
    "You toss $p to the ground.",
    "$n tosses $p to the ground.",
    "",
    ""));
  CompInfo.push_back(compInfo(SPELL_TRUE_SIGHT,
    "You nibble on $p and spit it upon $N.",
    "$n nibbles on $p and spits it at $N.",
    "$n nibbles on $p and spits it at you.",                       
    "You nibble on $p and swallow it down.",
    "$n nibbles on $p and swallows it down.",
    "",
    ""));
  CompInfo.push_back(compInfo(SPELL_POLYMORPH,
    "",                       
    "",
    "",
    "You wave $p over your head.",
    "$n waves $p over $s head.",
    "",
    ""));
  CompInfo.push_back(compInfo(SPELL_ACCELERATE,
    "You shake $p at $N.",
    "$n shakes $p at $N.",
    "$n shakes $p at you.",                       
    "You shake $p at yourself.",
    "$n shakes $p at $mself.",
    "",
    ""));
  CompInfo.push_back(compInfo(SPELL_HASTE,
    "You shake $p at $N.",
    "$n shakes $p at $N.",
    "$n shakes $p at you.",                       
    "You shake $p at yourself.",
    "$n shakes $p at $mself.",
    "",
    ""));
  CompInfo.push_back(compInfo(SPELL_SLUMBER,
    "You blow $p upon $N.",
    "$n blows $p upon $N.",
    "$n blows $p upon you.",                       
    "You inhale $p.",
    "$n inhales $p.",
    "",
    ""));
  CompInfo.push_back(compInfo(SPELL_FUMBLE,
    "You lather with $p and point at $N.",
    "$n lathers with $p and points at $N.",
    "$n lathers with $p and points at you.",                       
    "You lather with $p and hug yourself.",
    "$n lathers with $p and hugs $mself.",
    "",
    ""));
  CompInfo.push_back(compInfo(SPELL_FAERIE_FOG,
    "",                       
    "",
    "",
    "You squeeze $p.",
    "$n squeezes $p.",
    "",
    ""));
  CompInfo.push_back(compInfo(SPELL_ICY_GRIP,
    "You clutch $p, shaking it at $N.",
    "$n clutches $p, shaking it at $N.",
    "$n clutches $p, shaking it at you.",                       
    "You clutch $p, shaking it at no one in particular.",
    "$n clutches $p, shaking it at no one in particular.",
    "",
    ""));
  CompInfo.push_back(compInfo(SPELL_WATERY_GRAVE,
    "You toss $p at $N.",
    "$n tosses $p at $N.",
    "$n tosses $p at you.",                       
    "You toss $p at yourself.",
    "$n tosses $p at $mself.",
    "",
    ""));
  CompInfo.push_back(compInfo(SPELL_ARCTIC_BLAST,
    "",                       
    "",
    "",
    "You breathe through $p.",
    "$n breathes through $p.",
    "",
    ""));
  CompInfo.push_back(compInfo(SPELL_ICE_STORM,
    "",                       
    "",
    "",
    "You shatter $p.",
    "$n shatters $p.",
    "",
    ""));
  CompInfo.push_back(compInfo(SPELL_TSUNAMI,
    "",                       
    "",
    "",
    "You uncork $p.",
    "$n uncorks $p.",
    "",
    ""));
  CompInfo.push_back(compInfo(SPELL_CONJURE_WATER,
    "",                       
    "",
    "",
    "You shatter $p.",
    "$n shatters $p.",
    "",
    ""));
  CompInfo.push_back(compInfo(SPELL_GILLS_OF_FLESH,
    "You apply $p to $N's throat.",
    "$n applies $p to $N's throat.",
    "$n applies $p to your throat.",                       
    "You apply $p to your throat.",
    "$n applies $p to $s throat.",
    "",
    ""));
  CompInfo.push_back(compInfo(SPELL_BREATH_OF_SARAHAGE,
    "",                       
    "",
    "",
    "You squeeze $p in your hands.",
    "$n squeezes $p in $s hands.",
    "",
    ""));
  CompInfo.push_back(compInfo(SPELL_INFRAVISION,
    "You grind $p against $N's eyelids.",
    "$n grinds $p against $N's eyelids.",
    "$n grinds $p against your eyelids.",                       
    "You grind $p against your eyelids.",
    "$n grinds $p against $s eyelids.",
    "",
    ""));
  CompInfo.push_back(compInfo(SPELL_ENHANCE_WEAPON,
    "",                       
    "",
    "",
    "You toss $p into the air.",
    "$n tosses $p into the air.",
    "You toss $p into the air and wave $N through it.",
    "$n tosses $p into the air and waves $N through it."));
  CompInfo.push_back(compInfo(SPELL_FLY,
    "You smear $p upon $N.",
    "$n smears $p on $N.",
    "$n smears $p upon you.",                       
    "You smear $p upon yourself.",
    "$n smears $p on $mself.",
    "",
    ""));
  CompInfo.push_back(compInfo(SKILL_BARKSKIN,
    "You smear $p upon $N.",
    "$n smears $p on $N.",
    "$n smears $p upon you.",                       
    "You smear $p upon yourself.",
    "$n smears $p on $mself.",
    "",
    ""));
  CompInfo.push_back(compInfo(SPELL_STORMY_SKIES,
    "",                       
    "",
    "",
    "You crush $p releasing a blue-green mist.",
    "$n crushes $p which produces a blue-green mist.",
    "",
    ""));
  CompInfo.push_back(compInfo(SPELL_STICKS_TO_SNAKES,
    "",                       
    "",
    "",
    "You wave $p over your head.",
    "$n waves $p over $s head.",
    "",
    ""));
  CompInfo.push_back(compInfo(SPELL_LIVING_VINES,
    "You squeeze $p at $N.",
    "$n squeezes $p in $N's direction.",
    "$n squeezes $p in your direction.",                       
    "You squeeze $p at yourself.",
    "$n squeezes $p.",
    "",
    ""));
  CompInfo.push_back(compInfo(SPELL_HYPNOSIS,
    "You shake $p in $N's face.",
    "$n shakes $p.",
    "$n shakes $p.",                       
    "You shake $p in your own face.",
    "$n shakes $p.",
    "",
    ""));
  CompInfo.push_back(compInfo(SPELL_CHEVAL,
    "",                       
    "",
    "",
    "You swallow $p.",
    "$n swallows $p.",
    "",
    ""));
  CompInfo.push_back(compInfo(SPELL_CELERITE,
    "",                       
    "",
    "",
    "You eat $p.",
    "$n eats $p.",
    "",
    ""));
  CompInfo.push_back(compInfo(SPELL_CLARITY,
    "",
    "",
    "",                       
    "You put a few drops of $p in your eyes.",
    "$n put a few drops of $p in $s eyes.",
    "",
    ""));
  CompInfo.push_back(compInfo(SPELL_ENLIVEN,
    "You squeeze $p at $N.",
    "$n squeezes $p at $N.",
    "$n squeezes $p at you.",                       
    "You squeeze $p at yourself.",
    "$n squeezes $p at $mself.",
    "",
    ""));
  CompInfo.push_back(compInfo(SPELL_SHADOW_WALK,
    "",
    "",
    "",                       
    "You drop $p to the ground and crush it under your feet.",
    "$n drops $p to the ground and crushes it under $s feet.",
    "",
    ""));
#if 1
  CompInfo.push_back(compInfo(SPELL_EARTHMAW,
			      "You grind $p into the ground.",
			      "$n grinds $p into the ground.",
			      "$n grinds $p into the ground.",
			      "You drive $p into the ground.",
			      "$n drives $p into the ground.",
			      "",
			      ""));
  CompInfo.push_back(compInfo(SPELL_CREEPING_DOOM,
                              "You uncork $p blow it in $N's direction.",
                              "$n uncorks $p and blows it in $N's direction.",
                              "$n uncorks $p and blows in in your direction.",
                              "You uncork $p and sprinkle some on yourself.",
                              "$n uncorks $p and sprinkles some on $mself.",
                              "",
                              ""));
  CompInfo.push_back(compInfo(SPELL_FERAL_WRATH,
                              "",
                              "",
                              "",
                              "You thrust $p into your palm.",
                              "$n thrusts $p into $s palm.",
                              "",
                              ""));
  CompInfo.push_back(compInfo(SPELL_SKY_SPIRIT,
                              "You toss some seeds from $p at $N.",
                              "$n tosses some seeds from $p at $N.",
                              "$n tosses some seeds from $p at you.",
                              "You toss some seeds from $p into the air.",
                              "$n tosses some seeds from $p into the air.",
                              "",
                              ""));
#endif
  CompInfo.push_back(compInfo(SKILL_BEAST_SOOTHER,
    "You coat $N with $p.",
    "$n coats $N with $p.",
    "$n coats you with $p.",                       
    "You coat yourself with $p.",
    "$n coats $mself with $p.",
    "",
    ""));
  CompInfo.push_back(compInfo(SKILL_TRANSFIX,
    "You smear $N with $p.",
    "$n smears $N with $p.",
    "$n smears you with $p.",                       
    "You smear yourself with $p.",
    "$n smears $mself with $p",
    "",
    ""));
  CompInfo.push_back(compInfo(SKILL_BEAST_SUMMON,
    "",                       
    "",
    "",
    "You rattle $p over your head.",
    "$n rattles $p over $s head.",
    "",
    ""));
  CompInfo.push_back(compInfo(SPELL_SHAPESHIFT,
    "",                       
    "",
    "",
    "You sip $p and quickly spit it out.",
    "$n sips $p and quickly spits it out.",
    "",
    ""));
  CompInfo.push_back(compInfo(SKILL_BEAST_CHARM,
    "You crumple $p and toss it at $N.",
    "$n crumples $p and tosses it in $N's direction.",
    "$n crumples $p and tosses it in your direction.",                       
    "You crumple $p and toss it on yourself.",
    "$n crumples $p and tosses it on $mself",
    "",
    ""));
  CompInfo.push_back(compInfo(SKILL_BEFRIEND_BEAST,
    "You point $p at $N.",
    "$n points $p in $N's direction.",
    "$n points $p in your direction.",                       
    "You point $p at yourself.",
    "$n points $p at $mself.",
    "",
    ""));
  CompInfo.push_back(compInfo(SPELL_DJALLA,
    "You smear $p on $N's forehead.",
    "$n smears $p on $N's forehead.",
    "$n smears $p on your forehead.",                       
    "You smear $p on your forehead.",
    "$n smears $p on $s forehead.",
    "",
    ""));
  CompInfo.push_back(compInfo(SPELL_LEGBA,
    "You smear $p on $N's forehead.",
    "$n smears $p on $N's forehead.",
    "$n smears $p on your forehead.",                       
    "You smear $p on your forehead.",
    "$n smears $p on $s forehead.",
    "",
    ""));
  CompInfo.push_back(compInfo(SPELL_PROTECTION_FROM_FIRE,
    "You touch $p to $N's forehead.",
    "$n touches $p to $N's forehead.",
    "$n touches $p to your forehead.",                       
    "You touch $p to your forehead.",
    "$n touches $p to $s forehead.",
    "",
    ""));
  CompInfo.push_back(compInfo(SPELL_PROTECTION_FROM_EARTH,
    "You touch $p to $N's forehead.",
    "$n touches $p to $N's forehead.",
    "$n touches $p to your forehead.",                       
    "You touch $p to your forehead.",
    "$n touches $p to $s forehead.",
    "",
    ""));
  CompInfo.push_back(compInfo(SPELL_PROTECTION_FROM_AIR,
    "You touch $p to $N's forehead.",
    "$n touches $p to $N's forehead.",
    "$n touches $p to your forehead.",                       
    "You touch $p to your forehead.",
    "$n touches $p to $s forehead.",
    "",
    ""));
  CompInfo.push_back(compInfo(SPELL_PROTECTION_FROM_WATER,
    "You touch $p to $N's forehead.",
    "$n touches $p to $N's forehead.",
    "$n touches $p to your forehead.",                       
    "You touch $p to your forehead.",
    "$n touches $p to $s forehead.",
    "",
    ""));
  CompInfo.push_back(compInfo(SPELL_PROTECTION_FROM_ELEMENTS,
    "You touch $p to $N's forehead.",
    "$n touches $p to $N's forehead.",
    "$n touches $p to your forehead.",                       
    "You touch $p to your forehead.",
    "$n touches $p to $s forehead.",
    "",
    ""));
  CompInfo.push_back(compInfo(SKILL_TRANSFORM_LIMB,
    "",
    "",
    "",
    "You put a drop from $p on the tip of your tongue.",
    "$n puts a drop from $p on the tip of $s tongue.",
    "",
    ""));
  CompInfo.push_back(compInfo(SPELL_GARMULS_TAIL,
    "You smear $p on $N.",
    "$n smears $p on $N.",
    "$n smears $p on you.",
    "You smear $p on your skin.",
    "$n smears $p on $s skin.",
    "",
    ""));
  CompInfo.push_back(compInfo(SPELL_ROOT_CONTROL,
    "You take a drop from $p and throw it on the $g.",
    "$n takes a drop from $p and throws it on the $g.",
    "$n takes a drop from $p and throws it on the $g.",
    "You take a drop from $p and throw it on the $g.",
    "$n takes a drop from $p and throws it on the $g.",
    "",
    ""));
  CompInfo.push_back(compInfo(SPELL_TREE_WALK,
    "",
    "",
    "",
    "You ignite $p and inhale the fumes.",
    "$n ignites $p and inhales the fumes.",
    "",
    ""));
  CompInfo.push_back(compInfo(SPELL_PLASMA_MIRROR,
    "",
    "",
    "",
    "You twirl $p about yourself.",
    "$n twirls $p about $mself.",
    "",
    ""));
  CompInfo.push_back(compInfo(SPELL_AQUALUNG,
    "You apply $p to $N's face.",
    "$n applies $p to $N's face.",
    "$n applies $p to your face.",                       
    "You apply $p to your face.",
    "$n applies $p to $s face.",
    "",
    ""));
  CompInfo.push_back(compInfo(SPELL_AQUATIC_BLAST,
    "",
    "",
    "",
    "You drink $p and, before swallowing it, spit it out forcefully.",
    "$n drinks $p and, before swallowing it, spits it out forcefully.",
    "",
    ""));
  CompInfo.push_back(compInfo(SPELL_THORNFLESH,
    "",
    "",
    "",
    "You nibble on $p and swallow it down.",
    "$n nibbles on $p and swallows it down.",
    "",
    ""));
  CompInfo.push_back(compInfo(SPELL_ENTHRALL_SPECTRE,
    "",                       
    "",
    "",
    "You sprinkle $p on the ground.",
    "$n sprinkles $p on the ground.",
    "",
    ""));
  CompInfo.push_back(compInfo(SPELL_ENTHRALL_GHAST,
    "",                       
    "",
    "",
    "You sprinkle $p on the ground.",
    "$n sprinkles $p on the ground.",
    "",
    ""));
  CompInfo.push_back(compInfo(SPELL_ENTHRALL_GHOUL,
    "",                       
    "",
    "",
    "You sprinkle $p on the ground.",
    "$n sprinkles $p on the ground.",
    "",
    ""));
  CompInfo.push_back(compInfo(SPELL_ENTHRALL_DEMON,
    "",                       
    "",
    "",
    "You sprinkle $p on the ground.",
    "$n sprinkles $p on the ground.",
    "",
    ""));
  CompInfo.push_back(compInfo(SPELL_CREATE_WOOD_GOLEM,
    "",                       
    "",
    "",
    "You throw $p into the air and it disappears!",
    "$n throws $p into the air and it disappears!",
    "",
    ""));
  CompInfo.push_back(compInfo(SPELL_CREATE_ROCK_GOLEM,
    "",                       
    "",
    "",
    "You throw $p into the air and it disappears!",
    "$n throws $p into the air and it disappears!",
    "",
    ""));
  CompInfo.push_back(compInfo(SPELL_CREATE_IRON_GOLEM,
    "",                       
    "",
    "",
    "You throw $p into the air and it disappears!",
    "$n throws $p into the air and it disappears!",
    "",
    ""));
  CompInfo.push_back(compInfo(SPELL_CREATE_DIAMOND_GOLEM,
    "",                       
    "",
    "",
    "You throw $p into the air and it disappears!",
    "$n throws $p into the air and it disappears!",
    "",
    ""));
  CompInfo.push_back(compInfo(SPELL_HEALING_GRASP,
    "You apply $p to $N's eyes.",
    "$n applies $p to $N's eyes.",
    "$n applies $p to your eyes.",                       
    "You apply $p to your eyes.",
    "$n applies $p to $s eyes.",
    "",
    ""));


  COMPINDEX ci;
  unsigned int j;
  int vnum, usage;
  spellNumT spell;
  TDatabase db(DB_SNEEZY);

  if(!db.query("select vnum, val2, val3 from obj where type=%i", mapFileToItemType(ITEM_COMPONENT))){
    vlogf(LOG_BUG, "Terminal database error (buildComponentArray)!");
    exit(0);
  }
  while(db.fetchRow()){
    vnum=convertTo<int>(db["vnum"]);
    spell=mapFileToSpellnum(convertTo<int>(db["val2"]));
    usage=convertTo<int>(db["val3"]);

    if(spell != TYPE_UNDEFINED &&
       (((usage & COMP_SPELL) != 0))){
      for(j=0;j<CompInfo.size();j++){
	if(CompInfo[j].spell_num == spell){
	  if(CompInfo[j].comp_num == -1 ||
	     vnum < CompInfo[j].comp_num){
	    CompInfo[j].comp_num = vnum;
	    break;
	  }
	}
      }
    }

    ci.comp_vnum = vnum;
    ci.spell_num = spell;
    ci.usage = usage;
    CompIndex.push_back(ci);
  }
}

TComponent::TComponent() :
  TMergeable(),
  charges(0),
  comp_spell(TYPE_UNDEFINED),
  comp_type(0)
{
}

TComponent::TComponent(const TComponent &a) :
  TMergeable(a),
  charges(a.charges),
  comp_spell(a.comp_spell), 
  comp_type(a.comp_type)
{
}

TComponent & TComponent::operator=(const TComponent &a)
{
  if (this == &a) return *this;
  TObj::operator=(a);
  charges = a.charges;
  comp_spell = a.comp_spell;
  comp_type = a.comp_type;
  return *this;
}

TComponent::~TComponent()
{
  // because of weight/volume manipulation for comps and spellbags
  // make sure comp is out of any spellbag it might be in
  if (parent)
    --(*this);
}

int TComponent::getComponentCharges() const
{
  return charges;
}

void TComponent::setComponentCharges(int n)
{
  charges = n;
}

void TComponent::addToComponentCharges(int n)
{
  charges += n;
}

spellNumT TComponent::getComponentSpell() const
{
  return comp_spell;
}

void TComponent::setComponentSpell(spellNumT n)
{
  comp_spell = n;
}

unsigned int TComponent::getComponentType() const
{
  return comp_type;
}

void TComponent::setComponentType(unsigned int num)
{
  comp_type = num;
}

void TComponent::addComponentType(unsigned int num)
{
  comp_type |= num;
}

void TComponent::remComponentType(unsigned int num)
{
  comp_type &= ~num;
}

bool TComponent::isComponentType(unsigned int num) const
{
  return ((comp_type & num) != 0);
}

void TComponent::changeObjValue4(TBeing *ch)
{
  ch->specials.edit = CHANGE_COMPONENT_VALUE4;
  change_component_value4(ch, this, "", ENTER_CHECK);
  return;
}

bool TComponent::sellMeCheck(TBeing *ch, TMonster *keeper, int num) const
{
  int total = 0;
  TThing *t;
  sstring buf;
  unsigned int shop_nr;

  for (shop_nr = 0; (shop_nr < shop_index.size()) && (shop_index[shop_nr].keeper != (keeper)->number); shop_nr++);

  if (shop_nr >= shop_index.size()) {
    vlogf(LOG_BUG, fmt("Warning... shop # for mobile %d (real nr) not found.") %  mob_index[keeper->number].virt);
    return FALSE;
  }
  
  TShopOwned tso(shop_nr, keeper, ch);
  int max_num=50;

  if(tso.isOwned())
    max_num=tso.getMaxNum(this);

  if(max_num == 0){
    keeper->doTell(ch->name, "I don't wish to buy any of those right now.");
    return TRUE;
  }


  for (t = keeper->getStuff(); t; t = t->nextThing) {
    if ((t->number == number) &&
	(t->getName() && getName() &&
	 !strcmp(t->getName(), getName()))) {
      if (TComponent *c = dynamic_cast<TComponent *>(t)) {
        total += c->getComponentCharges();
        break;
      }
    }
  }
  if (total >= max_num) {
    keeper->doTell(ch->getName(), fmt("I already have plenty of %s.") % getName());
    return TRUE;
  } else if (total + num > max_num) {
    keeper->doTell(ch->getName(), fmt("I'll buy no more than %d charge%s of %s.") % (max_num - total) % (max_num - total > 1 ? "s" : "") % getName());
    return FALSE;
  }

  return FALSE;
}

// returns DELETE_THIS, VICT (ch), ITEM(sub)
int TComponent::componentNumSell(TBeing *ch, TMonster *keeper, int shop_nr, TThing *sub, int num)
{
  int rc;

  if (equippedBy)
    *ch += *ch->unequip(eq_pos);

  if (sub) {
    rc = get(ch, this, sub, GETOBJOBJ, true);
    if (IS_SET_DELETE(rc, DELETE_ITEM))
      return DELETE_THIS;

    if (IS_SET_DELETE(rc, DELETE_VICT))
      return DELETE_ITEM;

    if (IS_SET_DELETE(rc, DELETE_THIS))
      return DELETE_VICT;

    if (!parent || dynamic_cast<TBeing *>(parent)) {
      // failed on volume or sumthing
      generic_num_sell(ch, keeper, this, shop_nr, num);
    }
  } else
    generic_num_sell(ch, keeper, this, shop_nr, num);

  return FALSE;
}

// returns DELETE_THIS, VICT (ch), ITEM(sub)
int TComponent::componentSell(TBeing *ch, TMonster *keeper, int shop_nr, TThing *sub)
{
  int rc;

  if (equippedBy)
    *ch += *ch->unequip(eq_pos);

  if (sub) {
    rc = get(ch, this, sub, GETOBJOBJ, true);
    if (IS_SET_DELETE(rc, DELETE_ITEM))
      return DELETE_THIS;

    if (IS_SET_DELETE(rc, DELETE_VICT))
      return DELETE_ITEM;

    if (IS_SET_DELETE(rc, DELETE_THIS))
      return DELETE_VICT;

    if (!parent || dynamic_cast<TBeing *>(parent)) {
      // failed on volume or sumthing
      generic_sell(ch, keeper, this, shop_nr);
    }
  } else
    generic_sell(ch, keeper, this, shop_nr);

  return FALSE;
}

int TComponent::componentNumValue(TBeing *ch, TMonster *keeper, int shop_nr, TThing *sub, int num)
{
  int rc;

  if (equippedBy)
    *ch += *ch->unequip(eq_pos);

  if (sub) {
    rc = get(ch, this, sub, GETOBJOBJ, true);
    if (IS_SET_DELETE(rc, DELETE_ITEM))
      return DELETE_THIS;

    if (IS_SET_DELETE(rc, DELETE_VICT))
      return DELETE_ITEM;

    if (IS_SET_DELETE(rc, DELETE_THIS))
      return DELETE_VICT;

    if (!parent || dynamic_cast<TBeing *>(parent)) 
      valueMe(ch, keeper, shop_nr, num);
  } else
    valueMe(ch, keeper, shop_nr, num);

  return FALSE;
}

int TComponent::componentValue(TBeing *ch, TMonster *keeper, int shop_nr, TThing *sub)
{
  int rc;

  if (equippedBy)
    *ch += *ch->unequip(eq_pos);

  if (sub) {
    rc = get(ch, this, sub, GETOBJOBJ, true);
    if (IS_SET_DELETE(rc, DELETE_ITEM))
      return DELETE_THIS;

    if (IS_SET_DELETE(rc, DELETE_VICT))
      return DELETE_ITEM;

    if (IS_SET_DELETE(rc, DELETE_THIS))
      return DELETE_VICT;

    if (!parent || dynamic_cast<TBeing *>(parent)) 
      valueMe(ch, keeper, shop_nr);
  } else
    valueMe(ch, keeper, shop_nr);

  return FALSE;
}

void TComponent::changeComponentValue4(TBeing *ch, const char *arg, editorEnterTypeT type)
{
  int loc_update;
  int bittog = 0;
  char buf[256];

  if (type != ENTER_CHECK) {
    if (!*arg || (*arg == '\n')) {
      ch->specials.edit = CHANGE_OBJ_VALUES;
      change_obj_values(ch, this, "", ENTER_CHECK);
      return;
    }
  }
  loc_update = convertTo<int>(arg);

  switch (loc_update) {
    case 1:
    case 2:
    case 3:
    case 4:
      bittog = 1<<(loc_update-1);
      if (isComponentType(bittog))
        remComponentType(bittog);
      else
        addComponentType(bittog);
    default:
      break;
  }

  ch->sendTo(VT_HOMECLR);

  ch->sendTo(fmt(VT_CURSPOS) % 3 % 5);
  sprintf(buf, "1: [%c]  Decay-Enabled",
             (isComponentType(COMP_DECAY) ? 'X' : ' '));
  ch->sendTo(buf);

  ch->sendTo(fmt(VT_CURSPOS) % 4 % 5);
  sprintf(buf, "2: [%c]  Component for Spell-casting",
             (isComponentType( COMP_SPELL) ? 'X' : ' '));
  ch->sendTo(buf);

  ch->sendTo(fmt(VT_CURSPOS) % 5 % 5);
  sprintf(buf, "3: [%c]  Component for Potion-brewing",
             (isComponentType( COMP_POTION) ? 'X' : ' '));
  ch->sendTo(buf);

  ch->sendTo(fmt(VT_CURSPOS) % 6 % 5);
  sprintf(buf, "4: [%c]  Component for Scribing",
             (isComponentType( COMP_SCRIBE) ? 'X' : ' '));
  ch->sendTo(buf);

  ch->sendTo(fmt(VT_CURSPOS) % 22 % 1);
  ch->sendTo("Hit return when finished.");
  ch->sendTo(fmt(VT_CURSPOS) % 23 % 1);
  ch->sendTo("Select the property to toggle.\n\r--> ");
}

void TComponent::boottimeInit()
{
  unsigned int j;

  // init component info
  if (getComponentSpell() != TYPE_UNDEFINED &&
      (isComponentType(COMP_SPELL))) {
    for (j = 0; j < CompInfo.size(); j++) {
      if (CompInfo[j].spell_num == getComponentSpell()) {
        if (CompInfo[j].comp_num == -1 ||
            objVnum() < CompInfo[j].comp_num) {
          CompInfo[j].comp_num = objVnum();
          break;
        }
      }
    }
  }
  COMPINDEX ci;
  ci.comp_vnum = objVnum();
  ci.spell_num = getComponentSpell();
  ci.usage = getComponentType();
  CompIndex.push_back(ci);
}

int TComponent::rentCost() const
{
  int num = TObj::rentCost();

  num *= getComponentCharges();

  num = (int) (num / priceMultiplier());

  return num;
}

void TComponent::decayMe()
{
  TMonster *tm;

  if (obj_flags.decay_time <= 0)
    return;

  // not a decaying component
  if (!isComponentType(COMP_DECAY))
    return;

  // don't decay if a shopkeeper has the comp
  if ((tm=dynamic_cast<TMonster *>(parent)) &&
      tm->isShopkeeper())
    return;

  // decay if it doesn't have a parent (lying on the ground)
  // or if the parent is not spellbag
  // or random chance
  if(!parent ||
     !dynamic_cast<TSpellBag *>(parent) || 
     (!::number(0,50))){
    //    setComponentCharges(getComponentCharges()-1);

    if(getComponentCharges() <= 0)
      obj_flags.decay_time--;
  }
}

void TComponent::assignFourValues(int x1, int x2, int x3, int x4)
{
  setComponentCharges(x1);
  setComponentSpell(mapFileToSpellnum(x3));
  setComponentType(x4);
}

void TComponent::getFourValues(int *x1, int *x2, int *x3, int *x4) const
{
  *x1 = getComponentCharges();
  *x3 = mapSpellnumToFile(getComponentSpell());
  *x4 = getComponentType();
}

sstring TComponent::statObjInfo() const
{
  sstring buf, sbuf;

  if (getComponentSpell() <= TYPE_UNDEFINED) {
    buf = "UNKNOWN/BOGUS spell\n\r";
  } else {
    buf = fmt("Spell for : %d (%s)\n\r") %
      getComponentSpell() %
      ((discArray[getComponentSpell()] &&
       discArray[getComponentSpell()]->name) ? 
       discArray[getComponentSpell()]->name :
       "UNKNOWN/BOGUS");
  }

  if (isComponentType(COMP_DECAY))
    buf += "Component will decay.\n\r";
  if (isComponentType(COMP_SPELL))
    buf += "Component is for spell casting.\n\r";
  if (isComponentType(COMP_POTION))
    buf += "Component is for brewing.\n\r";
  if (isComponentType(COMP_SCRIBE))
    buf += "Component is for scribing.\n\r";

  sbuf = fmt("Charges left : %d") % getComponentCharges();
  buf+=sbuf;

  sstring a(buf);
  return a;
}

bool TComponent::objectRepair(TBeing *ch, TMonster *repair, silentTypeT silent)
{
  if (!silent) {
    repair->doTell(fname(ch->getName()), "You might wanna take that to the magic shop!");
  }
  return TRUE;
}

void TComponent::lowCheck()
{
  if (!isname("component", name)) {
    vlogf(LOG_LOW, fmt("Component without COMPONENT in name (%s : %d)") %  getName() % objVnum());
  }
  int sp = suggestedPrice();
  // added the 2 vnums of the flask and parchment so they dont throw price error
  if ((objVnum() == 1400) || (objVnum() == 1487)) {
    return;
  }
  if ((obj_flags.cost != sp)) {
    vlogf(LOG_LOW, fmt("component (%s:%d) with bad price %d should be %d.") % 
          getName() % objVnum() % obj_flags.cost % sp);
    obj_flags.cost = sp;
  }

  TObj::lowCheck();
}

int TComponent::objectSell(TBeing *ch, TMonster *keeper)
{
  if (false)
    return FALSE;

#if 0
  char buf[256];

  if ((getComponentCharges() != getComponentMaxCharges())) {
    sprintf(buf, "%s I'm sorry, I don't buy back partially used components.", ch->getName());
    keeper->doTell(buf);
    return TRUE;
  }
#endif
  return FALSE;
}

void TComponent::findComp(TComponent **best, spellNumT spell)
{
  if (getComponentSpell() == spell &&
      isComponentType(COMP_SPELL)) {
    // it's the proper component
    if (!*best) {
      *best = this;
      return;
    }
    // avoid 0-cost components if at all possible
    // otherwise, use one with least charges left
    int rc_me = rentCost();
    int rc_best = (*best)->rentCost();
    if (rc_best <= 0 && rc_me > 0) {
      *best = this;
      return;
    } else if (rc_best > 0 && rc_me <= 0)
      return; 

    // check charges ONLY if: both comps are 0 cost, or both are non-0 cost
    if (getComponentCharges() < (*best)->getComponentCharges()) {
      *best = this;
    }
  }
}

int TComponent::putMeInto(TBeing *, TOpenContainer *)
{
  // components can only be put into spellbags
  return FALSE;
}

void TComponent::describeObjectSpecifics(const TBeing *ch) const
{
  ch->sendTo(COLOR_OBJECTS,fmt("%s has about %d uses left.\n\r") %
        sstring(getName()).cap() % getComponentCharges());
}

void TComponent::update(int use)
{
  if (obj_flags.decay_time > 0) {
    if (!parent || !dynamic_cast<TSpellBag *>(parent))
      obj_flags.decay_time -= use;
  }

  if (getStuff())
    getStuff()->update(use);

  if (nextThing) {
    if (nextThing != this)
      nextThing->update(use);
  }
}

void TComponent::findSomeComponent(TComponent **comp_gen, TComponent **spell_comp, TComponent **comp_brew, spellNumT which, int type)
{
  int dink;
  if (type == 1)
    dink = COMP_POTION;
  else if (type == 2)
    dink = COMP_SCRIBE;
  else {
    vlogf(LOG_MISC, "Unknown type in findSomeComps");
    return;
  }

  if (!*comp_gen && (getComponentSpell() == TYPE_UNDEFINED) &&
       isComponentType(dink))
          *comp_gen = this;
  else if (!*spell_comp && (getComponentSpell() == which) &&
       isComponentType(COMP_SPELL))
          *spell_comp = this;
  else if (!*comp_brew && (getComponentSpell() == which) &&
              isComponentType(dink))
          *comp_brew = this;
}

void TComponent::evaluateMe(TBeing *ch) const
{
  int learn;

  learn = ch->getSkillValue(SKILL_EVALUATE);

  ch->learnFromDoingUnusual(LEARN_UNUSUAL_NORM_LEARN, SKILL_EVALUATE, 10);

  // adjust for knowledge about magic stuff
  if (ch->hasClass(CLASS_RANGER)) {
    learn *= ch->getClassLevel(CLASS_RANGER);
    learn /= 200;
  } else if (ch->hasClass(CLASS_SHAMAN)) {
    learn *= ch->getClassLevel(CLASS_SHAMAN);
    learn /= 100;
  } else {
    learn *= ch->getSkillValue(SPELL_IDENTIFY);
    learn /= 100;
  }

  if (learn > 10)
    ch->describeComponentSpell(this, learn);

  if (learn > 20)
    ch->describeComponentUseage(this, learn);

  if (learn > 30)
    ch->describeComponentDecay(this, learn);
}

bool TComponent::fitInShop(const char *, const TBeing *ch) const
{
  if (ch->hasClass(CLASS_MAGE | CLASS_RANGER | CLASS_SHAMAN)) {
    // skip brew and scribe comps
    if (!IS_SET(getComponentType(), COMP_SPELL))
      return false;

    spellNumT skill = getComponentSpell();
    if (skill <= TYPE_UNDEFINED)
      // a "generic" comp, this should probably "fit"
      return TRUE;

    // if bogus spell, skip
    discNumT which = getDisciplineNumber(skill, FALSE);
    if (which == DISC_NONE)
      return false;

    return (ch->getSkillValue(skill));  
  }
  return FALSE;
}

bool TComponent::splitMe(TBeing *ch, const sstring &tString)
{
  if (false)
    return false;

  int         tCount = 0,
              tValue = 0;
  double      tCost  = 0.0;
  TComponent *tComponent;
  sstring      tStString(""),
              tStBuffer("");


  tStString=tString.word(0);
  tStBuffer=tString.word(1);

  if (tString.empty() || ((tCount = convertTo<int>(tStBuffer)) <= 0)) {
    ch->sendTo("Syntax: split <component> <charges>\n\r");
    return true;
  }

  if (tCount >= getComponentCharges()) {
    ch->sendTo(fmt("Charges must be between 1 and %d.\n\r") %
               (getComponentCharges() -1));
    return true;
  }

  if (!obj_flags.cost || objVnum() < 0) {
    ch->sendTo("This component is special, it can not be split up.\n\r");
    return true;
  }

  if ((tValue = real_object(objVnum())) < 0 || tValue > (signed) obj_index.size() ||
      !(tComponent = dynamic_cast<TComponent *>(read_object(tValue, REAL)))) {
    ch->sendTo("For some reason that component resists being split up.\n\r");
    return true;
  }

  act("You split $N into two pieces.",
      FALSE, ch, this, tComponent, TO_CHAR);
  act("$n splits $N into two pieces.",
      FALSE, ch, this, tComponent, TO_ROOM);
  tCost = ((double) (getComponentCharges() - tCount) / (double) getComponentCharges());

  tComponent->obj_flags.cost = 0;

  tComponent->setComponentCharges(getComponentCharges() - tCount);
  setComponentCharges(tCount);

  *ch += *tComponent;

  tComponent->obj_flags.cost = (int) ((double) obj_flags.cost * tCost);
  obj_flags.cost -= tComponent->obj_flags.cost;
  return true;
}

int TComponent::putSomethingIntoContainer(TBeing *ch, TOpenContainer *cont)
{
  int rc = TObj::putSomethingIntoContainer(ch, cont);
  if (rc != TRUE)
    return rc;

  // put succeeded
  mud_assert(parent == cont, "Bizarre situation in putSomethig int (%d)", rc);

  // Enable for !prod for re-introduction.
  if (false) {
    TThing *t;
    TComponent *tComp;

    for (t = cont->getStuff(); t; t = t->nextThing) {
      // Basically find another component of the same type that is:
      // Same VNum.
      // Has a cost greater than 0 (ignore comps from leveling)
      if (t == this || !(tComp = dynamic_cast<TComponent *>(t)) ||
          (tComp->objVnum() != objVnum()) || tComp->obj_flags.cost <= 0 ||
          obj_flags.cost <= 0)
        continue;

      // they just threw the same component into the bag, merge them...
      act("$p glows brightly and merges with $N.",
          FALSE, ch, this, tComp, TO_CHAR);
      addToComponentCharges(tComp->getComponentCharges());
      obj_flags.cost += tComp->obj_flags.cost;
      --(*tComp);
      delete tComp;
    }
  }

  return rc;
}

sstring TComponent::getNameForShow(bool useColor, bool useName, const TBeing *ch) const
{
  sstring buf;
  buf = useName ? name : (useColor ? getName() : getNameNOC(ch));

  spellNumT spell_num = getComponentSpell();
  if (spell_num > TYPE_UNDEFINED && discArray[spell_num]) {
    buf += " [";
    buf += discArray[spell_num]->name;
    buf += "]";
  }

  return buf;
}

compInfo::compInfo(spellNumT sn, const char *tc, const char *to, const char *tv, const char *ts, const char *tr, const char *tso, const char *tro) :
  comp_num(-1),
  spell_num(sn),
  to_caster(tc),
  to_other(to),
  to_vict(tv),
  to_self(ts),
  to_room(tr),
  to_self_object(tso),
  to_room_object(tro)
{
}

int TComponent::suggestedPrice() const
{
  spellNumT curspell = getComponentSpell();
  int value = 0;
  if (curspell > TYPE_UNDEFINED) {
    // since it's from a component, treat the level as lowest possible
    // a correction to lev is made inside SpellCost, so we can safely
    // pass it L1 and let SpellCost fix it for us
    // for level, we'll assign a value of 100% arbitrarily
    value = getSpellCost(curspell, 1, 100);
    value *= getComponentCharges();
  }

  value = (int) (value * priceMultiplier());

  // add material value
  value += (int)(10.0 * getWeight() * material_nums[getMaterial()].price);

  return value;
}

void TComponent::objMenu(const TBeing *ch) const
{
  ch->sendTo(fmt(VT_CURSPOS) % 3 % 1);
  ch->sendTo(fmt("%sSuggested price:%s %d%s") %
             ch->purple() % ch->norm() % suggestedPrice() %
             (suggestedPrice() != obj_flags.cost ? " *" : ""));
}

bool isInkComponent(int vnum)
{
  if(vnum>=1500 && vnum<=1548)
    return true;
  return false;
}

bool isBrewComponent(int vnum)
{
  if(vnum>=1401 && vnum<=1412)
    return true;
  return false;
}

bool isDissectComponent(int vnum)
{
  switch (vnum) {
    case COMP_SAND_BLAST:   // periodic loads
    case COMP_COLOR_SPRAY:   // periodic loads
    case COMP_ACID_BLAST:        // periodic loads
    case COMP_TELEPATHY:        // periodic loads
    case COMP_FUMBLE:        // periodic loads
    case COMP_STONE_SKIN:        // periodic loads
    case COMP_STEALTH:        // periodic loads
      //case COMP_CLOUD_OF_CONCEAL:        // periodic loads
    case COMP_CONJURE_AIR:        // periodic loads
    case COMP_FLIGHT:        // periodic loads
    case COMP_HELLFIRE:        // periodic loads

    // hard coded dissects
    case COMP_FLAMING_SWORD:           // dissect loads

    // items in lib/objdata/dissect
    case COMP_FEATHERY_DESCENT:        // dissect loads
    // case COMP_FUMBLE:  // also a periodic load
    case COMP_SENSE_LIFE:        // dissect loads
    case COMP_SHATTER:          // dissect loads
    case COMP_FALCON_WINGS:          // dissect loads
    case COMP_ENERGY_DRAIN:        // dissect loads
    case COMP_FARLOOK:        // dissect loads
    case COMP_ENSORCER:        // dissect loads
    case COMP_FEAR:        // dissect loads
    case COMP_ACCELERATE:        // dissect loads
    case COMP_GILLS_OF_FLESH:        // dissect loads
    case COMP_INFRAVISION:        // dissect loads
    case COMP_BREATH_SARAHAGE:        // dissect loads
    case COMP_HASTE:        // dissect loads
    case COMP_BIND:        // dissect loads
    case COMP_STUNNING_ARROW:               // dissect loads
    case COMP_ANTIGRAVITY:        // dissect loads

    // skinning items
    case COMP_POLYMORPH:
    case COMP_COPY:
    // numerous hides not accounted for here...

    // these are just generic items (non-comps), from dissect
    case 4791:
    case OBJ_AQUA_DRAG_HEAD:
    case 20425:
    case 20409:
    case 20437:
    case 20440:
    case 20441:
    case 20442:
    case 20443:
    case 20450:

      return true;
    default:
      return false;
  }
}

// we want some components to have an inflated price so they can be sold
// for mega-bucks, and are thus sought ought
// but we don't want to make them expensive to rent at the same time
double TComponent::priceMultiplier() const
{
  int ov = objVnum();

  if (ov == COMP_ENHANCE_WEAPON)  // enhance weapon
    return 100.0;
  if (ov == COMP_GALVANIZE)
    return 30.0;

  if (isDissectComponent(ov))
    return 3.0;

  return 1.0;
}

void TComponent::purchaseMe(TBeing *ch, TMonster *keeper, int cost, int shop_nr)
{
  TShopOwned tso(shop_nr, keeper, ch);
  tso.doBuyTransaction(cost, getName(), TX_BUYING, this);
}

void TComponent::sellMeMoney(TBeing *ch, TMonster *keeper, int cost, int shop_nr)
{

    TShopOwned tso(shop_nr, keeper, ch);
    tso.doSellTransaction(cost, getName(), TX_SELLING, this);

}

TThing & TComponent::operator -- ()
{

  TObj::operator -- ();

  return *this;
}


int TComponent::buyMe(TBeing *ch, TMonster *tKeeper, int tNum, int tShop)
{
  if (false) {
    TObj::buyMe(ch, tKeeper, tNum, tShop);
    return -1;
  }

  float     tChr;
  int        tCost,
          tValue = 0;
  sstring tString;
  TObj   *tObj;

  if ((ch->getCarriedVolume() + getTotalVolume()) > ch->carryVolumeLimit()) {
    ch->sendTo(fmt("%s: You can not carry that much volume.\n\r") % fname(name));
    return -1;
  }

  if (compareWeights(getTotalWeight(TRUE),
                     (ch->carryWeightLimit() - ch->getCarriedWeight())) == -1) {
    ch->sendTo(fmt("%s: You can not carry that much weight.\n\r") % fname(name));
    return -1;
  }

  tChr = ch->getChaShopPenalty() - ch->getSwindleBonus();
  tChr   = max((float)1.0, tChr);
  tCost  = (int) shopPrice(tNum, tShop, tChr, ch);

  if ((ch->getMoney() < tCost) && !ch->hasWizPower(POWER_GOD)) {
    tKeeper->doTell(ch->name, shop_index[tShop].missing_cash2);

    switch (shop_index[tShop].temper1) {
      case 0:
        tKeeper->doAction(ch->getName(), CMD_SMILE);
        break;
      case 1:
        act("$n grins happily.", 0, tKeeper, 0, 0, TO_ROOM);
        break;
      default:
        break;
    }
    return -1;
  }

  int charges = getComponentCharges();
  
  if (tNum > charges) {
    tKeeper->doTell(ch->getName(), fmt("I don't have %d charges of %s.  Here %s the %d I do have.") % tNum % getName() % ((charges > 2) ? "are" : "is") % charges);
    tNum  = charges;
    tCost = shopPrice(tNum, tShop, tChr, ch);
  }
  
  if (charges == tNum) {
    tObj = this;
    --(*tObj);
  } else {
    if (!(tObj = read_object(number, REAL))) {
      vlogf(LOG_MISC, fmt("Shop with item not in db!  [%d]") %  number);
      return -1;
    }
    
    if (TComponent *tComponent = dynamic_cast<TComponent *>(tObj)) {
      int cost_per;
      
      cost_per = tComponent->pricePerUnit();
      tComponent->setComponentCharges(tNum);
      addToComponentCharges(-tNum);
      
      tComponent->obj_flags.cost = tNum * cost_per;
      obj_flags.cost = getComponentCharges() * cost_per;
    }
  }
  
  tObj->purchaseMe(ch, tKeeper, tCost, tShop);
  tKeeper->doTell(ch->name, fmt(shop_index[tShop].message_buy) % tCost);
  
  ch->sendTo(COLOR_OBJECTS, fmt("You now have %s (*%d charges).\n\r") %
	     sstring(getName()).uncap() % tNum);
  act("$n buys $p.", FALSE, ch, this, NULL, TO_ROOM);
  
  *ch += *tObj;
  ch->logItem(tObj, CMD_BUY);
  tValue++;
  
  tString = fmt("%s/%d") % SHOPFILE_PATH % tShop;
  tKeeper->saveItems(tString);

  if (!tValue)
    return -1;

  ch->doSave(SILENT_YES);

  return tCost;
}

int TComponent::sellMe(TBeing *ch, TMonster *tKeeper, int tShop, int num)
{
  sstring buf;
  float  tChr;
  int     tCost;

  if (!shop_index[tShop].profit_sell) {
    tKeeper->doTell(ch->getName(), shop_index[tShop].do_not_buy);
    return false;
  }

  if (obj_flags.cost <= 1 || isObjStat(ITEM_NEWBIE)) {
    tKeeper->doTell(ch->getName(), "I'm sorry, I don't buy valueless items.");
    return false;
  }

  if (sellMeCheck(ch, tKeeper, num))
    return false;

  TShopOwned tso(tShop, tKeeper, ch);
  int max_num = 50;
  int total = 0;

  if(tso.isOwned())
    max_num=tso.getMaxNum(this);

  for (TThing *t = tKeeper->getStuff(); t; t = t->nextThing) {
    if ((t->number == number) &&
	(t->getName() && getName() &&
	 !strcmp(t->getName(), getName()))) {
      if (TComponent *c = dynamic_cast<TComponent *>(t)) {
        total += c->getComponentCharges();
        break;
      }
    }
  }
  if (total + num > max_num) {
    num = max_num - total;
  }

  num = min(num, getComponentCharges());
  tChr = ch->getChaShopPenalty() - ch->getSwindleBonus();
  tChr   = max((float)1.0, tChr);
  tCost  = max(1, sellPrice(num, tShop, tChr, ch));

  if (tKeeper->getMoney() < tCost) {
    tKeeper->doTell(ch->name, shop_index[tShop].missing_cash1);
    return false;
  }

  if (obj_index[getItemIndex()].max_exist <= 10) {
    tKeeper->doTell(ch->getName(), "Wow!  This is one of those limited items.");
    tKeeper->doTell(ch->getName(), "You should really think about auctioning it.");
  }

  act("$n sells $p.", FALSE, ch, this, 0, TO_ROOM);

  tKeeper->doTell(ch->getName(), fmt(shop_index[tShop].message_sell) % tCost);

  ch->sendTo(COLOR_OBJECTS, fmt("The shopkeeper now has %s.\n\r") % sstring(getName()).uncap());
  ch->logItem(this, CMD_SELL);

  sellMeMoney(ch, tKeeper, tCost, tShop);

  if (ch->isAffected(AFF_GROUP) && ch->desc &&
           IS_SET(ch->desc->autobits, AUTO_SPLIT) &&
          (ch->master || ch->followers)) {
    buf = fmt("%d") % tCost;
    ch->doSplit(buf.c_str(), false);
  }

  if (num == getComponentCharges()) {
    --(*this);

    *tKeeper += *this;
  } else {
    int tValue = 0;
    // double tCost = 0.0;
    TComponent *tComponent;

    if ((tValue = real_object(objVnum())) < 0 || tValue > (signed) obj_index.size() ||
        !(tComponent = dynamic_cast<TComponent *>(read_object(tValue, REAL)))) {
      ch->sendTo(COLOR_OBJECTS, fmt("For some reason %s resists being partially sold.\n\r") % getName());
      return false;
    }
    int cost_per = 0;

    cost_per = tComponent->pricePerUnit();
    tComponent->setComponentCharges(num);
    addToComponentCharges(-num);

    tComponent->obj_flags.cost = num * cost_per;
    obj_flags.cost = getComponentCharges() * cost_per;

    /*
    tCost = ((double) (num / (double) getComponentCharges()));

    tComponent->setComponentCharges(num);
    addToComponentCharges(-num);

    tComponent->obj_flags.cost = (int) ((double) obj_flags.cost * tCost + 0.5);
    obj_flags.cost -= tComponent->obj_flags.cost;
    */

    *tKeeper += *tComponent;
  }

  buf = fmt("%s/%d") % SHOPFILE_PATH % tShop;
  tKeeper->saveItems(buf);
  if (!ch->delaySave)
    ch->doSave(SILENT_YES);

  return true;
}

int TComponent::sellPrice(int num, int shop_nr, float, const TBeing *ch)
{
  int cost_per;
  int price;

  cost_per = pricePerUnit();
  num = min(num, getComponentCharges());
  price = (int) (num * cost_per * shop_index[shop_nr].getProfitSell(this, ch));

  if (obj_flags.cost <= 1) {
    price = max(0, price);
  } else {
    price = max(1, price);
  }

  return price;
}

int TComponent::shopPrice(int num, int shop_nr, float, const TBeing *ch) const
{
  int cost_per;
  int price;

  cost_per = pricePerUnit();
  price = (int) (num * cost_per * shop_index[shop_nr].getProfitBuy(this, ch));
  price = max(1, price);

  return price;
}

int TComponent::pricePerUnit() const
{
  int charges = getComponentCharges();
  int price = (int) (charges ? (obj_flags.cost / charges) + 0.5 : 0);

  if (obj_flags.cost) {
    price = max(price, 1);
  }

  return price;
}

void TComponent::valueMe(TBeing *ch, TMonster *keeper, int shop_nr, int num)
{
  int price;
  sstring buf;
  int willbuy = 0;

  willbuy=!sellMeCheck(ch, keeper, num);
  price = sellPrice(num, shop_nr, -1, ch);

  if (!shop_index[shop_nr].willBuy(this)) {
    keeper->doTell(ch->getName(), shop_index[shop_nr].do_not_buy);
    return;
  }

  if (willbuy) {
    buf = fmt("I'll give you %d talens for %s!") % price % getName();
  } else {
    buf = fmt("Normally, I'd give you %d talens for %s!") % price % getName();
  }
  keeper->doTell(ch->getName(), buf);
  return;
}
