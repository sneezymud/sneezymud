#include "low.h"
#include "monster.h"
#include "obj_base_corpse.h"
#include "pathfinder.h"
#include "room.h"
#include "handler.h"
#include "spec_mobs.h"

// returns DELETE_THIS
int goToMorgue(TBeing *myself)
{
  int room = Room::MORGUE;
  dirTypeT dir;
  int rc;
  TPathFinder path;

  if (myself->in_room != Room::MORGUE) {
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
  TThing *t;
  TObj *obj = NULL;
  int rc;
  char buf[256];  

  if ((cmd != CMD_GENERIC_PULSE) || !ch->awake() || ch->fight())
    return FALSE;

  if (::number(0,3))
    return FALSE;

  int found=0;
  
  // check if we have any corpses
  if (!myself->stuff.empty()){
    for(StuffIter it=myself->stuff.begin();it!=myself->stuff.end();++it){
      if(dynamic_cast<TBaseCorpse *>(*it))
	found++;
    }
  }
  
  // pick up corpses
  if(myself->inRoom() != Room::MORGUE && found < 3){
    // look for a corpse
    for(StuffIter it=myself->roomp->stuff.begin();it!=myself->roomp->stuff.end();){
      t=*(it++);
      
      obj = dynamic_cast<TObj *>(t);
      if (!obj || !okForJanitor(myself, obj))
	continue;
      
      if (dynamic_cast<TBaseCorpse *>(obj)) {
	sprintf(buf, "$n gets $p and prepares it for delivery to the morgue.");
	act(buf, FALSE, myself, obj, 0, TO_ROOM);
	TThing *tt;
	for(StuffIter itt=obj->stuff.begin();itt!=obj->stuff.end();){
	  tt=*(it++);
	  (*tt)--;
	  *myself += *tt;
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
