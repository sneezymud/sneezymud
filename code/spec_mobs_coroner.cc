#include "stdsneezy.h"
#include "obj_base_corpse.h"
#include "pathfinder.h"

// returns DELETE_THIS
int goToMorgue(TBeing *myself)
{
  int room = ROOM_MORGUE;
  dirTypeT dir;
  int rc;
  TPathFinder path;

  if (myself->in_room != ROOM_MORGUE) {
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
  } else {
    rc = myself->doDrop("all.corpse" , NULL);
    if (IS_SET_DELETE(rc, DELETE_THIS))
      return DELETE_THIS;

    rc = myself->doDrop("all.lost-limb", NULL);
    if (IS_SET_DELETE(rc, DELETE_THIS))
      return DELETE_THIS;

    return TRUE;
  }
}


static int findACorpse(TMonster *myself)
{
  dirTypeT dir;
  int rc;
  TPathFinder path;

  // findCorpse only works in GH, so limit range
  path.setRange(50);

  dir=path.findPath(myself->inRoom(), findCorpse());

  if (dir >= MIN_DIR) {
    rc = myself->goDirection(dir);
    if (IS_SET_DELETE(rc, DELETE_THIS))
      return DELETE_THIS;
    return TRUE;
  }
  return FALSE;
}






int coroner(TBeing *ch, cmdTypeT cmd, const char *, TMonster *myself, TObj *)
{
  TThing *t, *t2;
  TObj *obj = NULL;
  int rc;
  char buf[256];  

  if ((cmd != CMD_GENERIC_PULSE) || !ch->awake() || ch->fight())
    return FALSE;

  if (::number(0,3))
    return FALSE;

  int found=0;
  
  // check if we have any corpses
  if (myself->getStuff()) {
    TThing *t;
    for(t=myself->getStuff();t;t=t->nextThing){
      if(dynamic_cast<TBaseCorpse *>(t))
	found++;
    }
  }
  
  // pick up corpses
  if(myself->inRoom() != ROOM_MORGUE && found < 3){

    // look for a corpse
    for (t = myself->roomp->getStuff(); t; t = t2) {
      t2 = t->nextThing;
      
      obj = dynamic_cast<TObj *>(t);
      if (!obj || !okForJanitor(myself, obj))
	continue;
      
      if (dynamic_cast<TBaseCorpse *>(obj)) {
	sprintf(buf, "$n gets $p and prepares it for delivery to the morgue.");
	act(buf, FALSE, myself, obj, 0, TO_ROOM);
	TThing *t;
	while ((t = obj->getStuff())) {
	  (*t)--;
	  *myself += *t;
	}
	(*obj)--;
	*myself += *obj;
	return TRUE;
      }
    }
  }

  
  // if we have corpses, head towards the morgue
  if(found){
    rc = goToMorgue(myself);
    
    if (IS_SET_DELETE(rc, DELETE_THIS)) {
      return DELETE_THIS;
    }
    
    return TRUE;
  } else { // otherwise, look for a corpse
    rc = findACorpse(myself);
    if (IS_SET_DELETE(rc, DELETE_THIS)) {
      return DELETE_THIS;
    }
    return TRUE;
  }

  return FALSE;
}
