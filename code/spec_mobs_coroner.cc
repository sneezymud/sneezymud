#include "stdsneezy.h"

// returns DELETE_THIS
int goToMorgue(TBeing *myself)
{
  int room = ROOM_MORGUE;
  dirTypeT dir;
  int rc;

  if (myself->in_room != ROOM_MORGUE) {
    if ((dir = find_path(myself->in_room, is_target_room_p, 
			 (void *) room, myself->trackRange(), 0)) < 0){
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

    // chance to purge the room of objects
    if (myself->roomp->roomIsEmpty(FALSE) && !::number(0,34)) {
      TThing *t, *t2;
      for (t = myself->roomp->stuff; t; t = t2) {
        t2 = t->nextThing;
        if (!dynamic_cast<TObj *> (t)) 
          continue;
        dynamic_cast<TObj *> (t)->purgeMe(myself);
        // t is possibly invalid here.
      }
    }
    return TRUE;
  }
}


// used as a conditional to find_path
static int find_closest_corpse(int room, void *myself)
{
  // don't track corpses in the morgue
  if (room == ROOM_MORGUE)
    return FALSE;
  
  TRoom *rp = real_roomp(room);

  // don't leave gh
  if (!rp->inGrimhaven())
    return FALSE;

  TThing *t;
  for (t = rp->stuff; t; t = t->nextThing) {
    if (!dynamic_cast<TBaseCorpse *>(t))
      continue;
    return TRUE;
  }
  return FALSE;
}

static int findACorpse(TMonster *myself)
{
  dirTypeT dir;
  int rc;

  dir = find_path(myself->inRoom(), find_closest_corpse, (void *) myself, myself->trackRange(), 0);

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
  
  // pick up corpses
  if(myself->inRoom() != ROOM_MORGUE){

    // look for a corpse
    for (t = myself->roomp->stuff; t; t = t2) {
      t2 = t->nextThing;
      
      obj = dynamic_cast<TObj *>(t);
      if (!obj || !okForJanitor(myself, obj))
	continue;
      
      if (dynamic_cast<TBaseCorpse *>(obj)) {
	sprintf(buf, "$n gets $p and prepares it for delivery to the morgue.");
	act(buf, FALSE, myself, obj, 0, TO_ROOM);
	TThing *t;
	while ((t = obj->stuff)) {
	  (*t)--;
	  *myself += *t;
	}
	(*obj)--;
	*myself += *obj;
      }
    }
  }

  bool found=FALSE;
  
  // check if we have any corpses
  if (myself->stuff) {
    TThing *t;
    for(t=myself->stuff;t;t=t->nextThing){
      if(dynamic_cast<TBaseCorpse *>(t))
	found=TRUE;
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
