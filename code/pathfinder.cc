#include "stdsneezy.h"
#include "pathfinder.h"
#include "obj_portal.h"
#include "obj_base_corpse.h"


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
  if (room == ROOM_DONATION)
    return false;

  TRoom *rp = real_roomp(room);
  if (!rp->inGrimhaven())
    return false;

  TThing *t;
  for (t = rp->getStuff(); t; t = t->nextThing) {
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

  TThing *t;
  for (t = rp->getStuff(); t; t = t->nextThing) {
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

  TThing *t;
  for (t = rp->getStuff(); t; t = t->nextThing) {
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

  TThing *t;
  for (t = rp->getStuff(); t; t = t->nextThing) {
    TObj * obj = dynamic_cast<TObj *>(t);
    if (!obj)
      continue;
    if(obj->objVnum()==OBJ_BM_TRASHCAN && obj->getStuff())
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
  TThing *t;
  rp = real_roomp(room);

  for (t = rp->getStuff(); t; t = t->nextThing) {
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
  if (room == ROOM_MORGUE)
    return FALSE;
  
  TRoom *rp = real_roomp(room);

  // don't leave gh
  if (!rp->inGrimhaven())
    return FALSE;

  TThing *t;
  for (t = rp->getStuff(); t; t = t->nextThing) {
    if (!dynamic_cast<TBaseCorpse *>(t) || t->getStuff())
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

  TThing *t;
  for (t = rp->getStuff(); t; t = t->nextThing) {
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
  return (searchLinkedList(name, real_roomp(room)->getStuff(), TYPEBEING) != NULL);
}


//////////


// findWater

findWater::findWater(){
}


bool findWater::isTarget(int room) const
{
  TRoom *rp;
  TThing *t;
  rp = real_roomp(room);

  if (rp->isRiverSector())
    return TRUE;

  for (t = rp->getStuff(); t; t = t->nextThing) {
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
  TThing *t;
  rp = real_roomp(room);

  for (t = rp->getStuff(); t; t = t->nextThing) {
    TMonster *leper = dynamic_cast<TMonster *>(t);
    if (!leper)
      continue;
    if (leper->hasDisease(DISEASE_LEPROSY) || leper->spec==SPEC_LEPER)
      return true;
  }
  return false;
}


//////////


///////////

TPathFinder::~TPathFinder()
{
  deque<pathData *>::iterator it;
  for(it=path.begin();it!=path.end();++it){
    delete *it;
  }
}

TPathFinder::TPathFinder()
{
  thru_doors=true;
  use_portals=true;
  range=5000;
  stay_zone=false;
  no_mob=true;
  dest=ROOM_NOWHERE;
}

TPathFinder::TPathFinder(int depth)
{
  TPathFinder();
  setRange(depth);
}

void TPathFinder::setRange(int d){ 
  if(d<0){
    // old find_path used negative depth to set certain options
    // this is depreciated, so check for erroneous usage
    vlogf(LOG_BUG, fmt("TPathFinder::setRange called with negative depth (%i)!") % d);
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

  // create this room as a starting point

  map<int, pathData *>path_map;
  path_map[here] = new pathData(here, DIR_NONE, -1, false, 0);

  map<int, pathData *>::const_iterator CI;
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
	  if (stay_zone && (hp->getZoneNum() != rp->getZoneNum())) {
	    continue;
	  }
	  
	  // do we have this room already?
	  map<int, pathData *>::const_iterator CT;
	  CT = path_map.find(exitp->to_room);
	  if (CT != path_map.end())
	    continue;
	  
	  // is this our target?
	  if(pt.isTarget(exitp->to_room)){
	    // found our target, walk our list backwards
	    dest = exitp->to_room;
	    dist = distance;
	    
	    path.push_front(new pathData(exitp->to_room, dir,
					 CI->first, false, distance+1));

	    pd = CI->second;
	    for (;;) {
	      path.push_front(new pathData(pd));

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
        TThing *t;
        for (t = rp->getStuff(); t; t = t->nextThing) {
          TPortal *tp = dynamic_cast<TPortal *>(t);
          if (!tp) 
            continue;
	  // dirTypeT ++ wraps around - stupid
	  dir = (dirTypeT) (dir + 1);
          if (tp->isPortalFlag(EX_LOCKED | EX_CLOSED))
            continue;
          int tmp_room = tp->getTarget();   // next room
          TRoom *hp = real_roomp(tmp_room);
          if (!hp) {
            continue;
          }

          // check stay_zone criteria
          if (stay_zone && (hp->getZoneNum() != rp->getZoneNum())) {
            continue;
          }

          // do we have this room already?
          map<int, pathData *>::const_iterator CT;
          CT = path_map.find(tmp_room);
          if (CT != path_map.end())
            continue;

          // is this our target?
	  if(pt.isTarget(tmp_room)){
            // found our target, walk our list backwards
	    dest = tmp_room;
	    dist = distance;

	    path.push_front(new pathData(tmp_room, dir,
					 CI->first, false, distance+1));

            pd = CI->second;
	    dirTypeT tmpdir=dir;
            for (;;) {
	      path.push_front(new pathData(pd));
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
  dest=ROOM_NOWHERE;
  dist=distance;
  
  // clean up allocated memory
  for (CI = path_map.begin(); CI != path_map.end(); ++CI)
    delete CI->second;
  
  return DIR_NONE;
}

