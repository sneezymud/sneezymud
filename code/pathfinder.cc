#include "stdsneezy.h"
#include "pathfinder.h"
#include "obj_portal.h"



// findTargetRoom
findTargetRoom::findTargetRoom(int d)
{
  dest=d;
}

bool findTargetRoom::isTarget(int room) const
{
  return (room==dest);
}



TPathFinder::TPathFinder()
{
  thru_doors=false;
  use_portals=false;
  depth=5000;
  stay_zone=false;
  no_mob=true;
}



dirTypeT TPathFinder::findPath(int here, const TPathTarget &pt)
{
  // just to be dumb, check my own room first
  if(pt.isTarget(here))
    return DIR_NONE;

  // create this room as a starting point
  pathData *pd = new pathData();
  pd->direct = DIR_NONE;
  pd->source = -1;
  pd->checked = false;

  map<int, pathData *>path_map;
  path_map[here] = pd;

  for(;;) {
    map<int, pathData *>::const_iterator CI;
    map<int, pathData *>next_map;

    if (path_map.size() > (unsigned int) depth) {
      // clean up allocated memory
      for (CI = path_map.begin(); CI != path_map.end(); ++CI) {
        pd = CI->second;
        delete pd;
      }
      for (CI = next_map.begin(); CI != next_map.end(); ++CI) {
        pd = CI->second;
        delete pd;
      }

      return DIR_NONE;
    }

    for (CI = path_map.begin(); CI != path_map.end(); ++CI) {
      // no need to do things twice
      pd = CI->second;
      if (pd->checked)
        continue;

      dirTypeT dir;
      TRoom *rp = real_roomp(CI->first);
      if (!rp) {
        vlogf(LOG_BUG, "Problem iterating path map.");
        continue;
      }
      if(!use_portals){
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
	    CT = next_map.find(exitp->to_room);
	    if (CT != next_map.end())
	      continue;

	    // is this our target?
	    if(pt.isTarget(exitp->to_room)){
	      // found our target, walk our list backwards
	      pd = CI->second;
	      for (;;) {
		if (pd->source == -1) {
		  // clean up allocated memory
		  for (CI = path_map.begin(); CI != path_map.end(); ++CI) {
		    pathData *tpd = CI->second;
		    delete tpd;
		  }
		  for (CI = next_map.begin(); CI != next_map.end(); ++CI) {
		    pathData *tpd = CI->second;
		    delete tpd;
		  }

		  return dir;
		}
		dir = pd->direct;
		pd = path_map[pd->source];
	      }
	    }
	    // it's not our target, and we don't have this room yet
	    pd = new pathData();
	    pd->source = CI->first; 
	    pd->direct = dir; 
	    pd->checked = false; 
	    next_map[exitp->to_room] = pd;
	  }
	}
      }

      // check for portals that might lead to target
      // return 10 if its the 1st portal in the room, 11 for 2nd, etc
      // 0-9 are obviously real exits (see above)
      if (thru_doors || use_portals) {
        dir = dirTypeT(MAX_DIR-1);
        TThing *t;
        for (t = rp->getStuff(); t; t = t->nextThing) {
          TPortal *tp = dynamic_cast<TPortal *>(t);
          if (!tp) 
            continue;
          dir++;   // increment portal
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
          CT = next_map.find(tmp_room);
          if (CT != next_map.end())
            continue;

          // is this our target?
	  if(pt.isTarget(tmp_room)){
            // found our target, walk our list backwards
            pd = CI->second;
            for (;;) {
              if (pd->source == -1) {
                // clean up allocated memory
                for (CI = path_map.begin(); CI != path_map.end(); ++CI)
                  delete CI->second;
                for (CI = next_map.begin(); CI != next_map.end(); ++CI)
                  delete CI->second;

                return dir;
              }
              dir = pd->direct;
              pd = path_map[pd->source];
            }
          }
          // it's not our target, and we don't have this room yet
          pd = new pathData();
          pd->source = CI->first; 
          pd->direct = dir; 
          pd->checked = false; 
          next_map[tmp_room] = pd;
        }  // stuff in room
      }
      // end portal check

      // we've checked all dirs for this room, so mark it checked
      pd = CI->second;
      pd->checked = true;
    }


    // if we failed to find any new rooms, abort, or be in an endless loop
    if (next_map.size() == 0) {
      // clean up allocated memory
      for (CI = path_map.begin(); CI != path_map.end(); ++CI)
        delete CI->second;
      for (CI = next_map.begin(); CI != next_map.end(); ++CI)
        delete CI->second;

      return DIR_NONE;
    }

    // we've looped over the entire map list, so move the next_map values in
    for (CI = next_map.begin(); CI != next_map.end(); ++CI) {
      path_map[CI->first] = CI->second;
    }
    next_map.clear();
  }
}

