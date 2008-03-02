#include "stdsneezy.h"
#include "pathfinder.h"
#include "obj_commodity.h"
#include "shop.h"
#include "shopowned.h"

// shop rooms 558,8734,1393,3709


int commodTrader(TBeing *, cmdTypeT cmd, const char *, TMonster *myself, TObj *)
{
#if 1
  int commod_shops[4]={558,8734,1393,3709};
  int commod_shop_nr[4]={15,56,57,58};
  int *target_shop_idx=NULL;
  TPathFinder path;
  TCommodity *commod;
  int materials[200];
  //  TShopOwned homebase(250, myself);
  TShopOwned homebase(16, myself);
  int price=0;

  if (cmd == CMD_GENERIC_DESTROYED) {
    delete static_cast<int *>(myself->act_ptr);
    myself->act_ptr = NULL;
    return FALSE;
  }
  if (cmd != CMD_GENERIC_PULSE || !myself->awake() || myself->fight())
    return FALSE;

  if(!myself->act_ptr){
    if (!(myself->act_ptr = new int)){
      vlogf(LOG_BUG, "failed memory allocation in mob proc commodTrader.");
      return FALSE;
    }
    if (!(target_shop_idx = static_cast<int *>(myself->act_ptr))) {
      vlogf(LOG_BUG, "commodTrader: error, static_cast");
      return TRUE;
    }
    *target_shop_idx=0;
  }
  if (!(target_shop_idx = static_cast<int *>(myself->act_ptr))) {
    vlogf(LOG_BUG, "commodTrader: error, static_cast");
    return TRUE;
  }

  if(myself->in_room == commod_shops[*target_shop_idx]){
    TShopOwned tso(commod_shop_nr[*target_shop_idx], myself);

    // sell commods
    TThing *t2;
    for(TThing *t=myself->getStuff();t;t=t2){
      t2=t->nextThing;
      if((commod=dynamic_cast<TCommodity *>(t))){
	price=commod->sellPrice(commod->numUnits(), commod_shop_nr[*target_shop_idx], -1, myself);
	homebase.doSellTransaction(-price, commod->getName(), TX_SELLING, commod);
	commod->sellMe(myself, tso.getKeeper(), commod_shop_nr[*target_shop_idx], 1);
	homebase.getKeeper()->setMoney(homebase.getKeeper()->getMoney()+myself->getMoney());
	myself->setMoney(0);
      }
    }

    // do price check, buy commods
    for(TThing *t=tso.getStuff();t;t=t->nextThing){
      if((commod=dynamic_cast<TCommodity *>(t))){
	materials[commod->getMaterial()]=commod->numUnits();
      }
    }
    for(int i=0;i<4;++i){
      if(i==*target_shop_idx)
	continue;

      TShopOwned tsot(commod_shop_nr[i], myself);
      
      for(TThing *t=tsot.getStuff();t;t=t->nextThing){
	if((commod=dynamic_cast<TCommodity *>(t))){
	  if((commod->numUnits()+1000) < materials[commod->getMaterial()]){
	    // do buy
	    int buy_mat=commod->getMaterial();
	    
	    for(TThing *tt=tso.getStuff();tt;tt=tt->nextThing){
	      if((commod=dynamic_cast<TCommodity *>(tt)) &&
		 commod->getMaterial()==buy_mat){
		price=commod->shopPrice(1000, commod_shop_nr[*target_shop_idx], -1, myself);
		myself->setMoney(myself->getMoney()+price);
		homebase.getKeeper()->setMoney(homebase.getKeeper()->getMoney()-price);
		homebase.doBuyTransaction(-price, commod->getName(), TX_BUYING, commod);
		commod->buyMe(myself, tso.getKeeper(), 1000, 
			      commod_shop_nr[*target_shop_idx]);
		*target_shop_idx=i;
		return TRUE;
	      }
	    }
	  }
	}
      }
    }
    

  } else {
    dirTypeT dir=path.findPath(myself->in_room, findRoom(commod_shops[*target_shop_idx]));
    myself->goDirection(dir);
  }

#endif


  return 0;
}


