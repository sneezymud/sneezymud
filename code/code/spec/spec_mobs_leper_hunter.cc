#include "monster.h"
#include "paths.h"
#include "pathfinder.h"
#include "room.h"
#include "obj_commodity.h"
#include "spec_mobs.h"

int leperHunter(TBeing *, cmdTypeT cmd, const char *, TMonster *myself, TObj *)
{
  TPathFinder *path;
  dirTypeT dir;
  int rc;
  TMonster *leper=NULL;

  if (cmd != CMD_GENERIC_PULSE || !myself->awake() || myself->fight())
    return FALSE;

  if(::number(0,2))
    return FALSE;

  if(!myself->act_ptr)
    myself->act_ptr=new TPathFinder();

  path=static_cast<TPathFinder *>(myself->act_ptr);
  path->setUseCached(true);
  
  dir=path->findPath(myself->inRoom(), findLeper());

  if(dir==DIR_NONE){
    for(StuffIter it=myself->roomp->stuff.begin();
	it!=myself->roomp->stuff.end();++it){
      leper = dynamic_cast<TMonster *>(*it);
      if (!leper)
	continue;
      
      if (leper->spec==SPEC_LEPER || leper->hasDisease(DISEASE_LEPROSY))
	break;

      leper=NULL;
    }

  } else {
    rc = myself->goDirection(dir);
    if (IS_SET_DELETE(rc, DELETE_THIS))
      return DELETE_THIS;
    return TRUE;
  }

  if(!leper)
    return FALSE;


  switch(::number(0,5)){
    case 0:
      myself->doSay("Take your filth to the underworld!");
      break;
    case 1:
      myself->doSay("I've got a leprosy cure right here... it's called MY FIST!");
      break;
    case 2:
      myself->doSay("For Galek!");
      break;
    case 3:
      myself->doSay("I will cleanse your soul, leper!");
      break;
    default:
      break;
  }


  return leper->takeFirstHit(*myself);
}
