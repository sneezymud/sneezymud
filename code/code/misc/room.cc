//////////////////////////////////////////////////////////////////////////
//
// SneezyMUD - All rights reserved, SneezyMUD Coding Team
//
//////////////////////////////////////////////////////////////////////////

// room.cc

#include <algorithm>
#include <ranges>
#include <unordered_map>

#include "room.h"
#include <functional>
#include "extern.h"
#include "being.h"
#include "monster.h"
#include "obj.h"
#include "weather.h"
#include "obj_organic.h"
#include "obj_flame.h"

namespace {
  // Per-spell-type vtables for room affects, populated at boot via
  // registerRoomAffect(). Generic dispatch (tick processing, look output)
  // looks the vtable up here instead of switching on spellNumT.
  std::unordered_map<int, const RoomAffectVTable*>& roomAffectRegistry() {
    static std::unordered_map<int, const RoomAffectVTable*> registry;
    return registry;
  }

  const RoomAffectVTable* getRoomAffectVTable(spellNumT type) {
    auto& registry = roomAffectRegistry();
    auto it = registry.find(static_cast<int>(type));
    return it != registry.end() ? it->second : nullptr;
  }
}  // namespace

void registerRoomAffect(spellNumT type, const RoomAffectVTable* vtable) {
  roomAffectRegistry()[static_cast<int>(type)] = vtable;
}

bool TRoom::isCitySector() const {
  switch (getSectorType()) {
    case SECT_TROPICAL_CITY:
    case SECT_TEMPERATE_CITY:
    case SECT_ARCTIC_CITY:
      return TRUE;
    default:
      return FALSE;
  }
}

bool TRoom::isFlyingSector() const {
  switch (getSectorType()) {
    case SECT_MAKE_FLY:
      return TRUE;
    default:
      return FALSE;
  }
}

bool TRoom::isRoadSector() const

{
  switch (getSectorType()) {
    case SECT_TROPICAL_ROAD:
    case SECT_TEMPERATE_ROAD:
    case SECT_ARCTIC_ROAD:
    case SECT_ARCTIC_FOREST_ROAD:
    case SECT_TEMPERATE_FOREST_ROAD:
    case SECT_RAINFOREST_ROAD:
      return TRUE;
    default:
      return FALSE;
  }
}

bool TRoom::isVertSector() const {
  switch (getSectorType()) {
    case SECT_TROPICAL_CLIMBING:
    case SECT_TEMPERATE_CLIMBING:
    case SECT_ARCTIC_CLIMBING:
      return TRUE;
    default:
      return FALSE;
  }
}

bool TRoom::isUnderwaterSector() const {
  switch (getSectorType()) {
    case SECT_TROPICAL_UNDERWATER:
    case SECT_TEMPERATE_UNDERWATER:
      return TRUE;
    default:
      return FALSE;
  }
}

bool TRoom::isNatureSector() const {
  switch (getSectorType()) {
    case SECT_GRASSLANDS:
    case SECT_PLAINS:
    case SECT_SAVANNAH:
    case SECT_TEMPERATE_HILLS:
    case SECT_TROPICAL_HILLS:
    case SECT_VELDT:
    case SECT_JUNGLE:
    case SECT_RAINFOREST:
    case SECT_TEMPERATE_FOREST:
    case SECT_ARCTIC_FOREST:
    case SECT_ARCTIC_FOREST_ROAD:
    case SECT_RAINFOREST_ROAD:
    case SECT_DESERT:  // adding desert
      return TRUE;
    default:
      return FALSE;
  }
}

bool TRoom::isSwampSector() const {
  switch (getSectorType()) {
    case SECT_ARCTIC_MARSH:
    case SECT_TEMPERATE_SWAMP:
    case SECT_TROPICAL_SWAMP:
      return TRUE;
    default:
      return FALSE;
  }
}

bool TRoom::isBeachSector() const {
  switch (getSectorType()) {
    case SECT_COLD_BEACH:
    case SECT_TEMPERATE_BEACH:
    case SECT_TROPICAL_BEACH:
      return TRUE;
    default:
      return FALSE;
  }
}

bool TRoom::isHillSector() const {
  switch (getSectorType()) {
    case SECT_ARCTIC_WASTE:
    case SECT_TEMPERATE_HILLS:
    case SECT_TROPICAL_HILLS:
      return TRUE;
    default:
      return FALSE;
  }
}

bool TRoom::isMountainSector() const {
  switch (getSectorType()) {
    case SECT_ARCTIC_MOUNTAINS:
    case SECT_TEMPERATE_MOUNTAINS:
    case SECT_TROPICAL_MOUNTAINS:
    case SECT_VOLCANO_LAVA:
      return TRUE;
    default:
      return FALSE;
  }
}

bool TRoom::isForestSector() const {
  switch (getSectorType()) {
    case SECT_JUNGLE:
    case SECT_RAINFOREST:
    case SECT_TEMPERATE_FOREST:
    case SECT_ARCTIC_FOREST:
    case SECT_ARCTIC_FOREST_ROAD:
    case SECT_TEMPERATE_FOREST_ROAD:
    case SECT_RAINFOREST_ROAD:
    case SECT_DEAD_WOODS:
      return TRUE;
    default:
      return FALSE;
  }
}

bool TRoom::isAirSector() const {
  switch (getSectorType()) {
    case SECT_TROPICAL_ATMOSPHERE:
    case SECT_TEMPERATE_ATMOSPHERE:
    case SECT_ARCTIC_ATMOSPHERE:
    case SECT_FIRE_ATMOSPHERE:
      return TRUE;
    default:
      return FALSE;
  }
}

bool TRoom::isOceanSector() const {
  switch (getSectorType()) {
    case SECT_TROPICAL_OCEAN:
    case SECT_TEMPERATE_OCEAN:
    case SECT_ICEFLOW:
      return TRUE;
    default:
      return FALSE;
  }
}

bool TRoom::isRiverSector() const {
  switch (getSectorType()) {
    case SECT_TROPICAL_RIVER_SURFACE:
    case SECT_TEMPERATE_RIVER_SURFACE:
    case SECT_ARCTIC_RIVER_SURFACE:
      return TRUE;
    default:
      return FALSE;
  }
}

bool TRoom::isIndoorSector() const {
  switch (getSectorType()) {
    case SECT_TEMPERATE_BUILDING:
    case SECT_TEMPERATE_CAVE:
    case SECT_TROPICAL_BUILDING:
    case SECT_TROPICAL_CAVE:
    case SECT_ARCTIC_BUILDING:
    case SECT_ARCTIC_CAVE:
      return TRUE;
    default:
      return FALSE;
  }
}

bool TRoom::isArcticSector() const {
  return (getSectorType() >= SECT_SUBARCTIC && getSectorType() < SECT_PLAINS);
}

bool TRoom::isTropicalSector() const {
  // changing this from getSectorType() >= SECT_DESERT
  // will probably mess something up but really... desert = tropical?
  // also killing getSectorType() == SECT_FIRE and SECT_FIRE_ATMOSPHERE
  return (getSectorType() >= SECT_TROPICAL_CITY &&
          getSectorType() < SECT_ASTRAL_ETHREAL);
}

bool TRoom::isWierdSector() const {
  return (getSectorType() == SECT_SOLID_ICE || getSectorType() >= 60);
}

bool TRoom::isFallSector() const {
  return (isAirSector() || isVertSector() || isFlyingSector());
}

bool TRoom::isWaterSector() const {
  return (isRiverSector() || isOceanSector());
}

bool TRoom::isWildernessSector() const {
  return (!isIndoorSector() && !isRoadSector() && !isCitySector() &&
          !isWierdSector());
}

bool TRoom::notRangerLandSector() const {
  return (isCitySector() || isRoadSector() || isFallSector() ||
          isUnderwaterSector() || isWaterSector() || isIndoorSector());
}

roomDirData* TRoom::exitDir(dirTypeT door) const {
  // door>=MAX_DIR would mean a portal, sometimes we pass this by accident
  if (door >= MAX_DIR || door < 0)
    return NULL;

  return (dir_option[door]);
}

roomDirData* TBeing::exitDir(dirTypeT door) const {
  return (roomp ? roomp->exitDir(door) : NULL);
}

roomDirData* TObj::exitDir(dirTypeT door) const {
  return (roomp ? roomp->exitDir(door) : NULL);
}

void room_iterate(TRoom*[],
  void (*func)(int, TRoom*, sstring&, struct show_room_zone_struct*),
  sstring& sbdata, void* srzdata) {
  int i;
  for (i = 0; i < WORLD_SIZE; i++) {
    TRoom* temp = real_roomp(i);

    if (temp)
      (*func)(i, temp, sbdata, (struct show_room_zone_struct*)srzdata);
  }
}

// returns the wintery equivalent of the current sector, if there is one
sectorTypeT TRoom::getArcticSectorType() const {
  // don't use getSectorType() here, or you'll get into a loop
  switch (getSectorType()) {
    case SECT_PLAINS:
      return SECT_SUBARCTIC;
    case SECT_TEMPERATE_CITY:
      return SECT_ARCTIC_CITY;
    case SECT_TEMPERATE_ROAD:
      return SECT_ARCTIC_ROAD;
    case SECT_GRASSLANDS:
      return SECT_TUNDRA;
    case SECT_TEMPERATE_HILLS:
    case SECT_TEMPERATE_MOUNTAINS:
      return SECT_ARCTIC_MOUNTAINS;
    case SECT_TEMPERATE_FOREST:
      return SECT_ARCTIC_FOREST;
    case SECT_TEMPERATE_SWAMP:
      return SECT_ARCTIC_MARSH;
    case SECT_TEMPERATE_OCEAN:
      return SECT_ICEFLOW;
    case SECT_TEMPERATE_RIVER_SURFACE:
      return SECT_ARCTIC_RIVER_SURFACE;
    case SECT_TEMPERATE_UNDERWATER:
      return SECT_SOLID_ICE;
    case SECT_TEMPERATE_BEACH:
      return SECT_COLD_BEACH;
    case SECT_TEMPERATE_BUILDING:
      return SECT_ARCTIC_BUILDING;
    case SECT_TEMPERATE_CAVE:
      return SECT_ARCTIC_CAVE;
    case SECT_TEMPERATE_ATMOSPHERE:
      return SECT_ARCTIC_ATMOSPHERE;
    case SECT_TEMPERATE_CLIMBING:
      return SECT_ARCTIC_CLIMBING;
    case SECT_TEMPERATE_FOREST_ROAD:
      return SECT_ARCTIC_FOREST_ROAD;
    default:
      return getSectorType();
  }
}

sectorTypeT TRoom::getSectorType() const {
  // it would be nice if this was non-const, and we could just call
  // some function like "makeRiver()", so we could get tropical/arctic
  // rivers and so on.  we'll have to settle for this for now.

  // this is a really, really stupid kluge to avoid getting into a loop,
  // as getWeather() calls getSectorType().  this is a way of making that
  // getWeather() call (and any sub-calls) to ignore this code.
  static bool looped = false;
  if (!looped) {
    looped = true;
    if (Weather::getWeather(*this) == Weather::SNOWY) {
      sectorTypeT sec = getArcticSectorType();
      looped = false;
      return sec;
    }
    looped = false;
  }

  if ((roomFlags & ROOM_FLOODED) != 0)
    return SECT_TEMPERATE_RIVER_SURFACE;

  if ((roomFlags & ROOM_ON_FIRE) != 0) {
    if (sectorType == SECT_TROPICAL_ATMOSPHERE ||
        sectorType == SECT_TEMPERATE_ATMOSPHERE ||
        sectorType == SECT_ARCTIC_ATMOSPHERE ||
        sectorType == SECT_FIRE_ATMOSPHERE)
      return SECT_FIRE_ATMOSPHERE;
    else
      return SECT_FIRE;
  }

  return sectorType;
}

void TRoom::setSectorType(sectorTypeT type) { sectorType = type; }

dirTypeT TRoom::getRiverDir() const { return riverDir; }

short TRoom::getRiverSpeed() const { return riverSpeed; }

void TRoom::setDescr(const sstring& tDescription) { descr = tDescription; }

const sstring& TRoom::getDescr() { return descr; }

bool TRoom::putInDb(int vnum) {
  if (real_roomp(vnum))
    return FALSE;

  room_db[vnum] = this;
  return TRUE;
}

int TRoom::chiMe(TBeing* tLunatic) {
  TBeing* tSucker;
  int tRc = 0;
  TThing* tThing;

  if (tLunatic->getSkillValue(SKILL_CHI) < 100 ||
      tLunatic->getDiscipline(DISC_MEDITATION_MONK)->getLearnedness() < 25) {
    tLunatic->sendTo("I'm afraid you don't have the training to do this.\n\r");
    return FALSE;
  }

  if (tLunatic->checkPeaceful(
        "You feel too peaceful to contemplate violence here.\n\r"))
    return FALSE;

  act(
    "You focus your <c>mind<z> and unleash a <r>blast of chi<z> upon your "
    "foes!",
    FALSE, tLunatic, NULL, NULL, TO_CHAR);
  act("$n suddenly <r>radiates with power<z> and brings harm to $s enemies!",
    TRUE, tLunatic, NULL, NULL, TO_ROOM);

  for (StuffIter it = stuff.begin(); it != stuff.end();) {
    tThing = *(it++);

    if (!(tSucker = dynamic_cast<TBeing*>(tThing)) || tSucker == tLunatic)
      continue;

    tRc = tSucker->chiMe(tLunatic);

    if (IS_SET_DELETE(tRc, RET_STOP_PARSING)) {
      tLunatic->sendTo("You are forced to stop.\n\r");
      return true;
    }

    if (IS_SET_DELETE(tRc, DELETE_THIS))
      return (DELETE_THIS | RET_STOP_PARSING);

    if (IS_SET_DELETE(tRc, DELETE_VICT)) {
      delete tThing;
      tThing = NULL;
    }
  }

  return true;
}

void TRoom::operator<<(TThing& tThing) {
  // assign birthRoom
  TMonster* tmon = dynamic_cast<TMonster*>(&tThing);
  if (tmon)
    tmon->brtRoom = this->number;

  if (!tBornInsideMe) {
    tBornInsideMe = &tThing;
    tThing.nextBorn = NULL;
    return;
  }

  TThing* tList;

  // creates forward-linked list
  for (tList = tBornInsideMe; tList->nextBorn; tList = tList->nextBorn) {
    if (&tThing == tList) {
      vlogf(LOG_BUG,
        format("Mob already in born list being added again. [%s]") %
          tThing.getName());
      return;
    }
  }

  tList->nextBorn = &tThing;
  tThing.nextBorn = NULL;
}

bool TRoom::operator|=(const TThing& tThing) {
  TThing* tList;

  for (tList = tBornInsideMe; tList; tList = tList->nextBorn)
    if (tList == &tThing)
      return true;

  return false;
}

void TRoom::operator>>(const TThing& tThing) {
  TThing *tList, *tLast = NULL;

  for (tList = tBornInsideMe; tList; tList = tList->nextBorn) {
    if (&tThing == tList) {
      if (tLast)
        tLast->nextBorn = tList->nextBorn;
      else
        tBornInsideMe = tList->nextBorn;

      tList->nextBorn = NULL;

      return;
    }

    tLast = tList;
  }

  vlogf(LOG_BUG,
    format(
      "Attempt to remove mob from born list that isn't in born list! [%s]") %
      tThing.getName());
}

int TRoom::getLight() {
  if (isRoomFlag(ROOM_ALWAYS_LIT))
    return 18;
  if (hasRoomAffect(SKILL_DROW_DARKNESS))
    return 0;
  return TThing::getLight();
}

TThing* TRoom::findInRoom(const std::function<bool(TThing*)>& predicate) {
  auto found = std::ranges::find_if(stuff, predicate);
  return found != stuff.end() ? *found : nullptr;
}

const TThing* TRoom::findInRoom(
  const std::function<bool(const TThing*)>& predicate) const {
  auto found = std::ranges::find_if(stuff, predicate);
  return found != stuff.end() ? *found : nullptr;
}

bool TRoom::hasCampfire() const {
  return std::ranges::any_of(stuff, [](const TThing* obj) {
    // Check for burning logs
    if (const auto* log = dynamic_cast<const TOrganic*>(obj)) {
      return log->itemType() == ITEM_RAW_ORGANIC &&
             log->isObjStat(ITEM_BURNING);
    }

    // Check for flame objects
    return dynamic_cast<const TFFlame*>(obj) != nullptr;
  });
}

bool TRoom::hasRoomAffect(spellNumT type) const {
  return std::ranges::any_of(roomAffects,
    [type](const auto& a) { return a.type == type; });
}

RoomAffectData* TRoom::getRoomAffect(spellNumT type) {
  auto it = std::ranges::find_if(roomAffects,
    [type](const auto& a) { return a.type == type; });
  return it != roomAffects.end() ? &(*it) : nullptr;
}

void TRoom::addRoomAffect(const RoomAffectData& affect) {
  roomAffects.push_back(affect);

  if (std::ranges::find(affectedRooms_db, this) == affectedRooms_db.end())
    affectedRooms_db.push_back(this);
}

void TRoom::removeRoomAffect(spellNumT type) {
  std::erase_if(roomAffects, [type](const auto& a) { return a.type == type; });

  if (roomAffects.empty())
    std::erase(affectedRooms_db, this);
}

void sendRoomAffectDescs(TBeing* ch, TRoom* rp, bool here) {
  if (!rp)
    return;
  for (const auto& affect : rp->getRoomAffects()) {
    const RoomAffectVTable* vt = getRoomAffectVTable(affect.type);
    if (!vt || !vt->lookDesc)
      continue;
    sstring desc = vt->lookDesc(here);
    if (!desc.empty())
      ch->sendTo(desc);
  }
}

bool TRoom::tickRoomAffects() {
  // duration semantics: number of damage ticks remaining. Tick first, then
  // decrement, then expire when it hits 0. So a freshly-cast affect with
  // duration N fires N onTick callbacks followed by one onExpire.
  //
  // Iterate backwards so we can erase expired entries safely. Re-acquire the
  // reference after each callback since onTick can transitively call
  // addRoomAffect (e.g. a death cascade triggers a mob spec proc that casts
  // a room-affect spell), which may reallocate the vector.
  for (int i = static_cast<int>(roomAffects.size()) - 1; i >= 0; --i) {
    auto idx = static_cast<size_t>(i);
    spellNumT type = roomAffects[idx].type;

    if (const auto* vt = getRoomAffectVTable(type); vt && vt->onTick)
      vt->onTick(this, roomAffects[idx]);

    // Re-validate: callback may have mutated the vector.
    if (idx >= roomAffects.size() || roomAffects[idx].type != type)
      continue;

    if (--roomAffects[idx].duration <= 0) {
      if (const auto* vt = getRoomAffectVTable(type); vt && vt->onExpire)
        vt->onExpire(this, roomAffects[idx]);
      roomAffects.erase(roomAffects.begin() + static_cast<ptrdiff_t>(i));
    }
  }
  return !roomAffects.empty();
}
