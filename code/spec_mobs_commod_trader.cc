#include "stdsneezy.h"
#include "pathfinder.h"
#include "obj_commodity.h"

// shop rooms 558,8734,1393,3709


int commodTrader(TBeing *, cmdTypeT cmd, const char *, TMonster *myself, TObj *)
{
#if 0
  int commod_shops[4]={558,8734,1393,3709};
  int commod_shop_nr[4]={15,56,57,58};
  int *target_shop_idx;
  TPathFinder path;
  dirTypeT dir;
  TCommodity *commod;

  if (cmd == CMD_GENERIC_DESTROYED) {
    delete static_cast<int *>(myself->act_ptr);
    myself->act_ptr = NULL;
    return FALSE;
  }
  if(!myself->act_ptr){
    if (!(myself->act_ptr = new int)){
      vlogf(LOG_BUG, "failed memory allocation in mob proc commodTrader.");
      return FALSE;
    }
    *target_shop_idx=0;
  }
  if (!(target_shop_idx = static_cast<int *>(myself->act_ptr))) {
    vlogf(LOG_BUG, "commodTrader: error, static_cast");
    return TRUE;
  }

  if(myself->in_room == commod_shops[*target_shop_idx]){
    // sell commods
    for(TThing *t=myself->getStuff();t;t=t->nextThing){
      if((commod=dynamic_cast<TCommodity *>(t))){
	commod->sellMe(myself, keeper, commod_shop_nr[*target_shop_idx], 1);
      }
    }


    // do price check
    // set new destination
    // move out
  } else {
    myself->goDirection(path.findPath(myself->in_room, findRoom(commod_shops[*target_shop_idx])));
  }

#endif


  return 0;
}


