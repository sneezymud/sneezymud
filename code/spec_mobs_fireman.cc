#include "stdsneezy.h"
#include "obj_base_corpse.h"
#include "pathfinder.h"

// yuck, global variable, seems to be the only way to do it
bool fireInGrimhaven=false;

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

  if(!fireInGrimhaven)
    return FALSE;

  // findFire only works in grimhaven, so really we only need a limited range
  // if no fire is found, they'll just head back to the fire station in GH
  path.setRange(100);

  dir=path.findPath(myself->inRoom(), findFire());

  if (dir >= MIN_DIR) {
    rc = myself->goDirection(dir);
    if (IS_SET_DELETE(rc, DELETE_THIS))
      return DELETE_THIS;
    return TRUE;
  } else {
    TRoom *rp;
    for(unsigned int zone = 0; zone < zone_table.size(); zone++) {
      if((rp=real_roomp(zone_table[zone].top)) && rp->inGrimhaven()){
	zone_table[zone].sendTo("<R>The firebells in the distance stop clanging.<1>\n\r", 4673);
      }
    }
    if((rp=real_roomp(4673))){
      rp->sendTo(COLOR_BASIC, "<R>The firebells stop clanging.<1>\n\r");
    }

    fireInGrimhaven=false;
  }
  return FALSE;
}



void firemanSay(TBeing *myself)
{
  if(!myself)
    return;

  switch(::number(0,5)){
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
    case 3:
      myself->doSay("I didn't need those eyebrows anyway.");
      break;
  }
}


int fireman(TBeing *ch, cmdTypeT cmd, const char *, TMonster *myself, TObj *)
{
  TThing *t, *t2;
  TObj *obj = NULL;
  int rc;
  int firehouse=4673;

  if ((cmd != CMD_GENERIC_PULSE) || !ch->awake() || ch->fight())
    return FALSE;

  if(myself->inRoom() != firehouse){
    for (t = myself->roomp->getStuff(); t; t = t2) {
      t2 = t->nextThing;
      
      if(!(obj = dynamic_cast<TObj *>(t)))
	continue;
      
      if (obj->isObjStat(ITEM_BURNING)){
	// this might seem silly, but if you've got like 10 firemen in a
	// room, it looks dumb to have them extinguish 10 fires at the
        // same time.
	if(::number(0,4))
	  return FALSE;

	if(myself->getRace() == RACE_CANINE){
	  act("$n barks excitedly at the fire and runs around in circles.", 
	      0, myself, 0, 0, TO_ROOM);
	} else {
	  t->extinguishMe(myself);
	  firemanSay(myself);
	}
	return TRUE;
      }
    }

    if(myself->roomp->isRoomFlag(ROOM_ON_FIRE)){
      act("$n extinguishes the burning room.",
	  0, myself, 0, 0, TO_ROOM);
      myself->roomp->removeRoomFlagBit(ROOM_ON_FIRE);
      firemanSay(myself);
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
