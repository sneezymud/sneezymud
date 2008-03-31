#include "stdsneezy.h"
#include "pathfinder.h"
#include "obj_commodity.h"
#include "shop.h"
#include "shopowned.h"
#include "obj_open_container.h"

// shop rooms 558,8734,1393,3709

TObj *findCart(TBeing *myself)
{
  TObj *cart=NULL;
  int cartnum=33296;

  for(TThing *t=myself->roomp->getStuff();t;t=t->nextThing){
    if((cart=dynamic_cast<TObj *>(t)) && cart->objVnum()==cartnum)
      break;
  }
  if(!cart){
    // ok cart isn't in our current room, so find out if it's in the world
    for(TObjIter iter=object_list.begin();iter!=object_list.end();++iter){
      if(*iter && (cart=*iter) && (cart->objVnum()==cartnum)){
	--(*cart);
	*myself->roomp+=*cart;
	break;
      }
      cart=NULL;
    }

    if(!cart){
      // no cart in the world, gotta create a new one
      cart=read_object(cartnum, VIRTUAL);
      *myself->roomp+=*cart;
    }
  }

  if(!cart){
    vlogf(LOG_BUG, "commodity trader couldn't find or make a cart");
    return NULL;
  }

  TOpenContainer *toc;
  if((toc=dynamic_cast<TOpenContainer *>(cart))){
    toc->addContainerFlag(CONT_CLOSED);
    toc->addContainerFlag(CONT_LOCKED);
  }

  return cart;
}

int commodTrader(TBeing *, cmdTypeT cmd, const char *, TMonster *myself, TObj *)
{
#if 1
  int commod_shops[4]={558,8734,1393,3709};
  int commod_shop_nr[4]={15,56,57,58};
  int *target_shop_idx=NULL;
  TPathFinder path;
  TCommodity *commod;
  int materials[200];
  int price=0;
  TObj *cart=NULL;
  TShopOwned homebase(250, myself);

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


  // find our cart
  cart=findCart(myself);
  if(!cart)
    return TRUE;
 
  if(myself->in_room == commod_shops[*target_shop_idx]){
    TShopOwned tso(commod_shop_nr[*target_shop_idx], myself);
    TThing *t2;

    // get stuff from the cart
    for(TThing *t=cart->getStuff();t;t=t2){
      t2=t->nextThing;

      --(*t);
      *myself+=*t;
    }

    // sell commods
    for(TThing *t=myself->getStuff();t;t=t2){
      t2=t->nextThing;
      if((commod=dynamic_cast<TCommodity *>(t))){
	price=commod->sellPrice(commod->numUnits(), 
				commod_shop_nr[*target_shop_idx], -1, myself);
	int rc;
	if(rc=commod->sellMe(myself, tso.getKeeper(), 
			  commod_shop_nr[*target_shop_idx], 1)){
	  homebase.doBuyTransaction(price, fmt("%s x %i") % 
				    commod->getName() % commod->numUnits(), 
				    TX_SELLING, commod);

	  if(IS_SET_DELETE(rc, DELETE_THIS))
	    delete commod;

	  sstring buf = fmt("%s/%d") % SHOPFILE_PATH % 250;
	  homebase.getKeeper()->saveItems(buf);
	  cart->roomp->saveItems("");

	}
	
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
	  int diff=materials[commod->getMaterial()]-commod->numUnits();

	  // risk mediation, for robbings etc
	  diff=min(1000, diff);

	  if(diff >= 100){
	    // do buy
	    int buy_mat=commod->getMaterial();

	    for(TThing *tt=tso.getStuff();tt;tt=tt->nextThing){
	      if((commod=dynamic_cast<TCommodity *>(tt)) &&
		 commod->getMaterial()==buy_mat){
		price=commod->shopPrice(diff/2, 
			  commod_shop_nr[*target_shop_idx], -1, myself);
		homebase.doSellTransaction(price, 
				fmt("%s x %i") % commod->getName() % (diff/2), 
					  TX_BUYING, commod);
		commod->buyMe(myself, tso.getKeeper(), diff/2, 
			      commod_shop_nr[*target_shop_idx]);
		*target_shop_idx=i;

		TThing *ttt2;
		for(TThing *ttt=myself->getStuff();ttt;ttt=ttt2){
		  ttt2=ttt->nextThing;
		  if(dynamic_cast<TCommodity *>(ttt)){
		    --(*ttt);
		    *cart += *ttt;
		  }
		}
		sstring buf = fmt("%s/%d") % SHOPFILE_PATH % 250;
		homebase.getKeeper()->saveItems(buf);
		cart->roomp->saveItems("");
		return TRUE;
	      }
	    }
	  }
	}
      }
    }

    // didn't find any deals
    *target_shop_idx=::number(0,3);
  } else {
    // speed
    if(::number(0,2))
      return FALSE;

    path.setNoMob(false);
    dirTypeT dir=path.findPath(myself->in_room, 
			       findRoom(commod_shops[*target_shop_idx]));

    myself->goDirection(dir);

    --(*cart);
    *myself->roomp += *cart;
    myself->roomp->saveItems("");
  }

#endif


  return 0;
}


