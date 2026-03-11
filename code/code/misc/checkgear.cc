//////////////////////////////////////////////////////////////////////////
//
// SneezyMUD - All rights reserved, SneezyMUD Coding Team
//
//////////////////////////////////////////////////////////////////////////

#include "being.h"
#include "room.h"
#include "extern.h"
#include "db.h"
#include "loadset.h"
#include "monster.h"
#include "handler.h"

// Helper function to get slot name for display
static sstring getSlotName(int slot) {
  if (slot >= 0 && slot < MAX_WEAR)
    return bodyParts[slot];
  return "unknown";
}

void TObj::objLoadSource(TBeing* ch) const {
  int objRnum = real_object(objVnum(), true);
  if (objRnum < 0 || objRnum >= (int)obj_index.size()) {
    return;
  }

  sstring objName = obj_index[objRnum].short_desc;
  const std::vector<CheckgearSourceInfo>* mobList = NULL;
  if (objRnum >= 0 && objRnum < (int)obj_load_sources_cache.size())
    mobList = &obj_load_sources_cache[objRnum];

  // Display results
  if (!mobList || mobList->empty()) {
    ch->sendTo("It doesn't seem like this object is frequently coveted by any known enemies.\n\r");
    return;
  }

  ch->sendTo(format("%s can be found wielded by...\n\r") % sstring(objName).cap());
  for (const auto& info : *mobList) {
    ch->sendTo(format("... %s in (%s)\n\r") %
               info.mobName %
               info.zoneName);
  }
}

void TBeing::mobGearList(TBeing* ch) const {
  const TMonster* mob = dynamic_cast<const TMonster*>(this);
  if (!mob) {
    ch->sendTo("That's not a mob.\n\r");
    return;
  }

  int mobRnum = real_mobile(mob->mobVnum());
  if (mobRnum < 0 || mobRnum >= (int)mob_index.size()) {
    ch->sendTo("Invalid mob.\n\r");
    return;
  }

  // Get mob short description and capitalize it
  sstring mobShortDesc = sstring(mob_index[mobRnum].short_desc).cap();

  const std::vector<CheckgearGearInfo>* gearList = NULL;
  if (mobRnum >= 0 && mobRnum < (int)mob_gear_cache.size())
    gearList = &mob_gear_cache[mobRnum];

  // Display results
  if (!gearList || gearList->empty()) {
    ch->sendTo(format("%s owns nothing of significance.\n\r") % mobShortDesc);
    return;
  }

  ch->sendTo(format("%s can be found to wield...\n\r") % mobShortDesc);
  for (const auto& gear : *gearList) {
    ch->sendTo(format("... %s (%s) %d%%\n\r") %
               gear.objName %
               getSlotName(gear.slot) %
               gear.chance);
  }
}
