#include "stdsneezy.h"
#include "obj_base_corpse.h"
#include "pathfinder.h"

// returns DELETE_THIS
int goToFirehouse(TBeing *myself)
{
  int room = 4673;
  dirTypeT dir;
  int rc;
  TPathFinder path;

  if (myself->in_room != room) {
    if((dir=path.findPath(myself->in_room, findRoom(room))) < 0){
      // unable to find a path 
      if (room >= 0) {
        myself->doSay("Time for my coffee break");
        act("$n has left into the void.",0, myself, 0, 0, TO_ROOM);
        --(*myself);
        thing_to_room(myself, room);
        act("$n comes back to work.", 0, myself, 0, 0, TO_ROOM);
      }
      return FALSE;
    }
    rc = myself->goDirection(dir);
    if (IS_SET_DELETE(rc, DELETE_THIS))
      return DELETE_THIS;
    return TRUE;
  }
  return FALSE;
}



static int findAFire(TMonster *myself)
{
  dirTypeT dir;
  int rc;
  TPathFinder path;

  // findFire only works in grimhaven, so really we only need a limited range
  // if no fire is found, they'll just head back to the fire station in GH
  path.setRange(50);

  dir=path.findPath(myself->inRoom(), findFire());

  if (dir >= MIN_DIR) {
    rc = myself->goDirection(dir);
    if (IS_SET_DELETE(rc, DELETE_THIS))
      return DELETE_THIS;
    return TRUE;
  }
  return FALSE;
}






int fireman(TBeing *ch, cmdTypeT cmd, const char *, TMonster *myself, TObj *)
{
  TThing *t, *t2;
  TObj *obj = NULL;
  int rc;
  int room=4673;

  if ((cmd != CMD_GENERIC_PULSE) || !ch->awake() || ch->fight())
    return FALSE;

  if(myself->inRoom() != room){
    for (t = myself->roomp->getStuff(); t; t = t2) {
      t2 = t->nextThing;
      
      if(!(obj = dynamic_cast<TObj *>(t)))
	continue;
      
      if (obj->isObjStat(ITEM_BURNING)){
	if(myself->getRace() == RACE_CANINE){
	  act("$n barks excitedly at the fire and runs around in circles.", 
	      0, myself, 0, 0, TO_ROOM);
	} else {
	  t->extinguishMe(myself);
	}
	
	switch(::number(0,3)){
	  case 0:
	    myself->doSay("Whew that was a close one!");
	    break;
	  case 1:
	    myself->doSay("Is there an arsonist around here?!");
	    myself->doAction("", CMD_PEER);
	    break;
	  case 2:
	    myself->doSay("Good thing that didn't spread.");
	    break;
	}
	break;
      }
    }
  }

  rc = findAFire(myself);

  if(!rc){
    rc = goToFirehouse(myself);
    
    if (IS_SET_DELETE(rc, DELETE_THIS)) {
      return DELETE_THIS;
    }
    
    return TRUE;
  } else {
    if (IS_SET_DELETE(rc, DELETE_THIS)) {
      return DELETE_THIS;
    }
    return TRUE;
  }

  return FALSE;
}
