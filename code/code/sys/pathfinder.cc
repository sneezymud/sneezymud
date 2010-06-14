#include "handler.h"
#include "room.h"
#include "low.h"
#include "monster.h"
#include "pathfinder.h"
#include "obj_portal.h"
#include "obj_base_corpse.h"
#include "spec_mobs.h"
#include "person.h"

using std::vector;

findFairFight::findFairFight(TBeing *tb)
{
  myself=tb;
}

bool findFairFight::isTarget(int room) const
{
  TRoom *rp = real_roomp(room);

  if (!rp->inGrimhaven())
    return false;
  if(rp->isRoomFlag(ROOM_PEACEFUL))
    return false;

  TThing *t=NULL;
  for(StuffIter it=rp->stuff.begin();it!=rp->stuff.end() && (t=*it);++it) {
    TMonster *tmon=dynamic_cast<TMonster *>(t);
    if (!tmon)
      continue;
    if (tmon->isPlayerAction(PLR_IMMORTAL))
      continue;

    //    vlogf(LOG_PEEL, format("level %f %i") % tmon->getRealLevel() % myself->GetMaxLevel());

    if((int)((tmon->getRealLevel()+0.5)-myself->GetMaxLevel()))
      return false;

    return true;
  }
  return false;
}

// findRoom
findRoom::findRoom(int d)
{
  dest=d;
}

bool findRoom::isTarget(int room) const
{
  return (room==dest);
}
//////////////


// findClutter
findClutter::findClutter(TBeing *tb)
{
  myself=tb;
}

bool findClutter::isTarget(int room) const
{
  if (room == Room::DONATION)
    return false;

  TRoom *rp = real_roomp(room);
  if (!rp->inGrimhaven())
    return false;

  TThing *t=NULL;
  for(StuffIter it=rp->stuff.begin();it!=rp->stuff.end() && (t=*it);++it) {
    TObj * obj = dynamic_cast<TObj *>(t);
    if (!obj)
      continue;
    if (!okForJanitor((TMonster *) myself, obj))
      continue;
    return true;
  }
  return false;
}
////////////

//////////// findClutterPrison
findClutterPrison::findClutterPrison(TBeing *tb)
{
  myself=tb;
}

bool findClutterPrison::isTarget(int room) const
{
  if (room == 31905 || room > 7499 || room <7300)
    return false;

  TRoom *rp = real_roomp(room);

  TThing *t=NULL;
  for(StuffIter it=rp->stuff.begin();it!=rp->stuff.end() && (t=*it);++it) {
    TObj * obj = dynamic_cast<TObj *>(t);
    if (!obj)
      continue;
    if(obj->objVnum() == 26688)
      continue;
    if (!okForJanitor((TMonster *) myself, obj))
      continue;
    return true;
  }
  return false;
}
////////////

//////////// findClutterAmber
findClutterAmber::findClutterAmber(TBeing *tb)
{
  myself=tb;
}

bool findClutterAmber::isTarget(int room) const
{
  if (room == 33281 || (room < 2750 || 
			(room > 2849 && room < 8700) ||
			(room > 8899 && room < 16200) ||
			(room > 16249 && room < 33270) ||
			room > 33299))
    return false;

  TRoom *rp = real_roomp(room);

  TThing *t=NULL;
  for(StuffIter it=rp->stuff.begin();it!=rp->stuff.end() && (t=*it);++it) {
    TObj * obj = dynamic_cast<TObj *>(t);
    if (!obj)
      continue;
    if (!okForJanitor((TMonster *) myself, obj))
      continue;
    return true;
  }
  return false;
}
////////////


//////////// findClutterBrightmoon
findClutterBrightmoon::findClutterBrightmoon(TBeing *tb)
{
  myself=tb;
}

bool findClutterBrightmoon::isTarget(int room) const
{
  if (room == 1385 || room<1200 || room >1399)
    return false;

  TRoom *rp = real_roomp(room);

  TThing *t=NULL;
  for(StuffIter it=rp->stuff.begin();it!=rp->stuff.end() && (t=*it);++it) {
    TObj * obj = dynamic_cast<TObj *>(t);
    if (!obj)
      continue;
    if(obj->objVnum()==Obj::BM_TRASHCAN && !obj->stuff.empty())
      return true;
    if(!okForJanitor((TMonster *) myself, obj))
      continue;
    return true;
  }
  return false;
}
////////////


// findPolice
findPolice::findPolice(){
}

bool findPolice::isTarget(int room) const
{
  TRoom *rp;
  TThing *t=NULL;
  rp = real_roomp(room);

  for(StuffIter it=rp->stuff.begin();it!=rp->stuff.end() && (t=*it);++it) {
    TBeing *ch = dynamic_cast<TBeing *>(t);
    if (!ch)
      continue;
    if (ch->isPc() && !ch->isImmortal())
      return true;
    if (ch->isPolice())
      return true;
  }
  return false;
}
//////////////

// findOutdoors

findOutdoors::findOutdoors(){
}

bool findOutdoors::isTarget(int room) const
{
  TRoom *rp = real_roomp(room);
  
  if(rp->isRoomFlag(ROOM_INDOORS))
    return FALSE;

  return TRUE;
}


///////////////



// findCorpse
findCorpse::findCorpse(){
}

bool findCorpse::isTarget(int room) const
{
  // don't track corpses in the morgue
  if (room == Room::MORGUE)
    return FALSE;
  
  TRoom *rp = real_roomp(room);

  // don't leave gh
  if (!rp->inGrimhaven())
    return FALSE;

  TThing *t=NULL;
  for(StuffIter it=rp->stuff.begin();it!=rp->stuff.end() && (t=*it);++it) {
    if (!dynamic_cast<TBaseCorpse *>(t) || !t->stuff.empty())
      continue;
    return TRUE;
  }
  return FALSE;
}
///////////////

// findFire
findFire::findFire(){
}

bool findFire::isTarget(int room) const
{
  TRoom *rp = real_roomp(room);
  TObj *o;

  // don't leave gh
  if (!rp->inGrimhaven())
    return FALSE;

  if(rp->isRoomFlag(ROOM_ON_FIRE))
    return TRUE;

  TThing *t=NULL;
  for(StuffIter it=rp->stuff.begin();it!=rp->stuff.end() && (t=*it);++it) {
    if((o=dynamic_cast<TObj *>(t)) && o->isObjStat(ITEM_BURNING))
      return TRUE;
  }
  return FALSE;
}
/////////////

// findBeing

findBeing::findBeing(sstring n){
  name=n;
}

bool findBeing::isTarget(int room) const
{
  return (searchLinkedList(name, real_roomp(room)->stuff, TYPEBEING) != NULL);
}


//////////


// findWater

findWater::findWater(){
}


bool findWater::isTarget(int room) const
{
  TRoom *rp;
  TThing *t=NULL;
  rp = real_roomp(room);

  if (rp->isRiverSector())
    return TRUE;

  for(StuffIter it=rp->stuff.begin();it!=rp->stuff.end() && (t=*it);++it) {
    if (t->spec == SPEC_FOUNTAIN)
      return TRUE;
    if (t->waterSource())
      return TRUE;
  }
  return FALSE;
}




//////////


// findLeper


findLeper::findLeper(){
}

bool findLeper::isTarget(int room) const
{
  TRoom *rp;
  TThing *t=NULL;
  rp = real_roomp(room);

  for(StuffIter it=rp->stuff.begin();it!=rp->stuff.end() && (t=*it);++it) {
    TMonster *leper = dynamic_cast<TMonster *>(t);
    if (!leper)
      continue;
    if (leper->hasDisease(DISEASE_LEPROSY) || leper->spec==SPEC_LEPER)
      return true;
  }
  return false;
}


//////////

// findEquipment finds eq once belonging to a specific being

bool findEquipment::isTarget(int room) const
{
  return findInStuff(real_roomp(room)->stuff);
}

bool findEquipment::findInStuff(StuffList stuff) const
{
  // loop all objects in stuff, check obj
  for(StuffIter it = stuff.begin(); it != stuff.end(); ++it) {
    TThing *t = *it;
    if (findInThing(t))
      return true;
  }
  return false;
}

bool findEquipment::findInThing(TThing *t) const
{
  if (!t)
    return false;

  // ignore owner
  if (owner == dynamic_cast<TPerson*>(t))
    return false;

  // check if we've seen this obj before, if so, skip it
  if (contains(t))
    return false;

  // check if this is our obj (cheat to allow us to modify this obj in const function)
  if (checkOwner(dynamic_cast<TObj *>(t))) {
    findEquipment *pThis = const_cast<findEquipment *>(this);
    pThis->foundlist.push_back(t);
    return true;
  }

  // if thing is container, loop its stuff
  if (findInStuff(t->stuff))
    return true;

  // if thing is table, loop that
  TThing *r = t->rider;
  while(r) {
    if (findInThing(r))
      return true;
    r = r->nextRider;
  }

  // loop mobs worn and stuckIn too
  TBeing *b = dynamic_cast<TBeing*>(t);
  for(wearSlotT slot = WEAR_HEAD; b && slot < MAX_WEAR; slot++) {
    if (findInThing(b->equipment[slot]))
      return true;
    if (findInThing(b->getStuckIn(slot)))
      return true;
  }

  return false;
}

bool findEquipment::contains(TThing *t) const
{
  for(vector<TThing *>::const_iterator it = foundlist.begin(); it < foundlist.end(); it++)
    if (t == *it)
      return true;
  return false;
}

bool findEquipment::checkOwner(TObj *o) const
{
  if (!o)
    return false;

  sstring ownerList = o->owners;
  sstring prevOwner;

  ownerList = one_argument(ownerList, prevOwner);
  while (!prevOwner.empty()) {  
    if (strcmp(owner->getName(), prevOwner.c_str()) == 0)
      return true;
    ownerList = one_argument(ownerList, prevOwner);
  }

  return false;
}


//////////


///////////

TPathFinder::~TPathFinder()
{
}

TPathFinder::TPathFinder() :
  thru_doors(true),
  use_portals(true),
  range(5000),
  stay_zone(false),
  no_mob(true),
  ship_only(false),
  use_cached(false),
  dest(Room::NOWHERE),
  dist(0)
{
}

TPathFinder::TPathFinder(int depth) :
  thru_doors(true),
  use_portals(true),
  range(5000),
  stay_zone(false),
  no_mob(true),
  ship_only(false),
  use_cached(false),
  dest(Room::NOWHERE),
  dist(0)
{
  setRange(depth);
}

void TPathFinder::setRange(int d){ 
  if(d<0){
    // old find_path used negative depth to set certain options
    // this is depreciated, so check for erroneous usage
    vlogf(LOG_BUG, format("TPathFinder::setRange called with negative depth (%i)!") % d);
    d=-d;
  }
  
  range=d; 
}


dirTypeT TPathFinder::findPath(int here, const TPathTarget &pt)
{
  // just to be dumb, check my own room first
  if(pt.isTarget(here)){
    dest = here;
    dist = 0;
    return DIR_NONE;
  }

  if(use_cached && (path.size()>0 && pt.isTarget(path[path.size()-1]->room))){
    for(unsigned int i=0;i<path.size();++i){
      // reached end of path, but let's re-calc it before we report that
      // the target was found - it may have moved
      if(i==path.size()-1)
	break;

      if(path[i]->room==here)
	return path[i+1]->direct;
    }
  }

  // create this room as a starting point
  std::map<int, pathData *>path_map;
  path_map[here] = new pathData(here, DIR_NONE, -1, false, 0);

  std::map<int, pathData *>::const_iterator CI;
  bool found=true;
  pathData *pd;
  int distance=0;

  for(distance=0;found;++distance){
    found=false;

    if (distance > range) {
      dest = path_map.size();
      dist = distance;

      // clean up allocated memory
      for (CI = path_map.begin(); CI != path_map.end(); ++CI)
        delete CI->second;

      return DIR_NONE;
    }

    for (CI = path_map.begin(); CI != path_map.end(); ++CI) {
      // no need to do things twice
      pd = CI->second;
      if (pd->checked)
        continue;

      if(pd->distance > distance)
	continue;

      dirTypeT dir;
      TRoom *rp = real_roomp(CI->first);
      if (!rp) {
        vlogf(LOG_BUG, "Problem iterating path map.");
        continue;
      }

      for (dir = MIN_DIR; dir < MAX_DIR; dir++) {
	roomDirData *exitp = rp->dir_option[dir];
	TRoom *hp = NULL;
	if (exitp && 
	    (hp = real_roomp(exitp->to_room)) &&
	    (!no_mob || !(hp->isRoomFlag(ROOM_NO_MOB))) &&
	    (thru_doors ? go_ok_smarter(exitp) : go_ok(exitp))) {
	  // check stay_zone criteria
	  if (stay_zone && (hp->getZoneNum() != rp->getZoneNum()))
	    continue;

	  if(ship_only && (!hp->isWaterSector() && !rp->isWaterSector()))
	    continue;

	  // do we have this room already?
	  std::map<int, pathData *>::const_iterator CT;
	  CT = path_map.find(exitp->to_room);
	  if (CT != path_map.end())
	    continue;
	  
	  // is this our target?
	  if(pt.isTarget(exitp->to_room)){
	    // found our target, walk our list backwards
	    dest = exitp->to_room;
	    dist = distance;


	    path.push_front(pathDataPtr(new pathData(exitp->to_room, dir,
					 CI->first, false, distance+1)));

	    pd = CI->second;
	    for (;;) {
	      path.push_front(pathDataPtr(new pathData(pd)));

	      if (pd->source == -1) {
		      // clean up allocated memory
		      for (CI = path_map.begin(); CI != path_map.end(); ++CI)
		        delete CI->second;
		      return dir;
	      }
	      dir = pd->direct;
	      pd = path_map[pd->source];
	    }
	  }
	  // it's not our target, and we don't have this room yet
	  path_map[exitp->to_room] = new pathData(exitp->to_room, dir, 
						 CI->first, false, distance+1);
	  found=true;
	}
      }

      // check for portals that might lead to target
      // return 10 if its the 1st portal in the room, 11 for 2nd, etc
      // 0-9 are obviously real exits (see above)
      if (use_portals) {
        dir = dirTypeT(MAX_DIR-1);
        TThing *t=NULL;
        for(StuffIter it=rp->stuff.begin();it!=rp->stuff.end() && (t=*it);++it) {
          TPortal *tp = dynamic_cast<TPortal *>(t);
          if (!tp) 
            continue;
	  // dirTypeT ++ wraps around - stupid
	  dir = (dirTypeT) (dir + 1);
          if (tp->isPortalFlag(EX_LOCKED | EX_CLOSED))
            continue;
          int tmp_room = tp->getTarget();   // next room
          TRoom *hp = real_roomp(tmp_room);
          if (!hp || hp->isRoomFlag(ROOM_NO_MOB)) {
            continue;
          }

          // check stay_zone criteria
          if (stay_zone && (hp->getZoneNum() != rp->getZoneNum())) {
            continue;
          }

          // do we have this room already?
          std::map<int, pathData *>::const_iterator CT;
          CT = path_map.find(tmp_room);
          if (CT != path_map.end())
            continue;

          // is this our target?
	  if(pt.isTarget(tmp_room)){
            // found our target, walk our list backwards
	    dest = tmp_room;
	    dist = distance;

	    path.push_front(pathDataPtr(new pathData(tmp_room, dir,
					 CI->first, false, distance+1)));

            pd = CI->second;
	    dirTypeT tmpdir=dir;
            for (;;) {
	      path.push_front(pathDataPtr(new pathData(pd)));
              if (pd->source == -1) {
                // clean up allocated memory
                for (CI = path_map.begin(); CI != path_map.end(); ++CI)
                  delete CI->second;

                return tmpdir;
              }
              tmpdir = pd->direct;
              pd = path_map[pd->source];
            }
          }

          // it's not our target, and we don't have this room yet
          path_map[tmp_room] = new pathData(tmp_room, dir, CI->first, 
					    false, distance+1);

	  found=true;
        }  // stuff in room
      }
      // end portal check

      // we've checked all dirs for this room, so mark it checked
      pd = CI->second;
      pd->checked = true;
    }
  }

  
  // if we failed to find any new rooms, abort, or be in an endless loop
  dest=Room::NOWHERE;
  dist=distance;
  
  // clean up allocated memory
  for (CI = path_map.begin(); CI != path_map.end(); ++CI)
    delete CI->second;
  
  return DIR_NONE;
}

