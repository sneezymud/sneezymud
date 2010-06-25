#include "monster.h"
#include "pathfinder.h"
#include "obj_commodity.h"
#include "shop.h"
#include "shopowned.h"
#include "obj_open_container.h"
#include "room.h"
#include "database.h"
#include "materials.h"

// shop rooms 558,8734,1393,3709

TObj *findCart(TBeing *myself)
{
  TObj *cart=NULL;
  int cartnum=33296;

  for(StuffIter it=myself->roomp->stuff.begin();it!=myself->roomp->stuff.end();++it){
    if((cart=dynamic_cast<TObj *>(*it)) && cart->objVnum()==cartnum)
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
  int commod_shops[4]={558,8734,1393,3709};
  int commod_shop_nr[4]={15,56,57,58};
  int *target_shop_idx=NULL;
  TPathFinder path;
  TCommodity *commod;
  int price=0;
  TObj *cart=NULL;
  TShopOwned homebase(250, myself);
  TDatabase db(DB_SNEEZY);

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

    // get stuff from the cart
    for(StuffIter it=cart->stuff.begin();it!=cart->stuff.end();){
      TThing *t=*(it++);

      --(*t);
      *myself+=*t;
    }

    // sell commods
    vlogf(LOG_PEEL, format("In target shop (%i) selling commods") %
	  commod_shop_nr[*target_shop_idx]);
    for(StuffIter it=myself->stuff.begin();it!=myself->stuff.end();){
      TThing *t=*(it++);
      if((commod=dynamic_cast<TCommodity *>(t))){
	price=commod->sellPrice(commod->numUnits(), 
				commod_shop_nr[*target_shop_idx], -1, myself);
	int rc;
	if(rc=commod->sellMe(myself, tso.getKeeper(), 
			  commod_shop_nr[*target_shop_idx], 1)){

	  homebase.journalize(tso.getKeeper()->getName(),
			      material_nums[commod->getMaterial()].mat_name,
			      TX_BUYING, price, 0, 0, 0, commod->numUnits());
	  myself->giveMoney(homebase.getKeeper(), myself->getMoney(), GOLD_SHOP);
	  homebase.getKeeper()->saveItems(250);

	  shoplog(250, myself, tso.getKeeper(), 
		  material_nums[commod->getMaterial()].mat_name, 
		  price, "selling");

	  vlogf(LOG_PEEL, format("sold %i for %i from %i") %
		commod->numUnits() % price % commod_shop_nr[*target_shop_idx]);

	  if(IS_SET_DELETE(rc, DELETE_THIS))
	    delete commod;
	}
      }
    }
    
      

    // do price check, buy commods
    db.query("\
select here.rent_id, others.owner, \
  (here.weight-min(others.weight))*10 as diff \
from \
  rent others, \
  (select rent_id, material, weight from rent \
    where vnum=50 and owner_type='shop' and owner=%i) here \
where others.vnum=50 and others.owner_type='shop' and \
  others.owner in (%i,%i,%i,%i) and here.material!=0 and \
  others.material=here.material \
group by owner, others.material \
order by diff desc limit 1", 
	     commod_shop_nr[*target_shop_idx], 
	     commod_shop_nr[0], commod_shop_nr[1], 
	     commod_shop_nr[2], commod_shop_nr[3]);
    db.fetchRow();
    
    int diff=(int)(convertTo<float>(db["diff"]));
    int rent_id=convertTo<int>(db["rent_id"]);
    int owner=convertTo<int>(db["owner"]);    

    vlogf(LOG_PEEL, format("best deal found at (%i): diff=%i") % owner % diff);

    // risk mediation, for robbings etc
    diff=min(5000, diff);
    
    if(diff >= 500 && owner!=commod_shop_nr[*target_shop_idx]){
      // do buy
      TObj *temp1=tso.getKeeper()->loadItem(commod_shop_nr[*target_shop_idx], rent_id);

      if(!(commod=dynamic_cast<TCommodity *>(temp1))){
	vlogf(LOG_BUG, format("commod trader tried to load non-commod: %i %i") %
	      rent_id % temp1->objVnum());
	delete temp1;
	return FALSE;
      }
      
      // make obj2 the amount we want to buy, then adjust an re-save commod
      TObj *obj2 = read_object(50, VIRTUAL);
      diff/=2;
      obj2->setWeight(diff/20.0);
      commod->setWeight(commod->getWeight() - obj2->getWeight());
      obj2->setMaterial(commod->getMaterial());
      tso.getKeeper()->deleteItem(commod_shop_nr[*target_shop_idx], rent_id);
      tso.getKeeper()->saveItem(commod_shop_nr[*target_shop_idx], commod);

      delete commod;
      commod=dynamic_cast<TCommodity *>(obj2);
      (*tso.getKeeper()) += *commod;

      price=commod->shopPrice(commod->numUnits(),
			      commod_shop_nr[*target_shop_idx], 
			      -1, myself);

      vlogf(LOG_PEEL, format("units: %i, price: %i") % commod->numUnits() % price);

      homebase.getKeeper()->giveMoney(myself, price, GOLD_SHOP);
      sstring commodname=material_nums[commod->getMaterial()].mat_name;
      int units=commod->numUnits();
      if((commod->buyMe(myself, tso.getKeeper(), commod->numUnits(), 
			commod_shop_nr[*target_shop_idx])) != -1){
	// commod is invalid here

	homebase.journalize(tso.getKeeper()->getName(), commodname,
			    TX_SELLING, price, 0, 0, 0, units);

	shoplog(250, myself, tso.getKeeper(), 
		commodname, -price, "buying");

      } else {
	vlogf(LOG_PEEL, "buy failed");
      }
      
      myself->giveMoney(homebase.getKeeper(), myself->getMoney(), GOLD_SHOP);
      homebase.getKeeper()->saveItems(250);

      vlogf(LOG_PEEL, format("bought %i for %i from %i") %
	    units % price % commod_shop_nr[*target_shop_idx]);

      for(int i=0;i<4;++i)
	if(commod_shop_nr[i]==owner)
	  *target_shop_idx=i;

      vlogf(LOG_PEEL, format("now moving to %i") % 
	    commod_shop_nr[*target_shop_idx]);

      for(StuffIter it=myself->stuff.begin();it!=myself->stuff.end();){
	TThing *ttt=*(it++);
	if(dynamic_cast<TCommodity *>(ttt)){
	  --(*ttt);
	  *cart += *ttt;
	}
      }
      return TRUE;
    }

    // didn't find any deals
    *target_shop_idx=::number(0,3);
  } else {
    // speed
    //    if(::number(0,2))
    //      return FALSE;


    for(int i=0;i<4;++i){
      path.setNoMob(false);
    dirTypeT dir=path.findPath(myself->in_room, 
			       findRoom(commod_shops[*target_shop_idx]));

    myself->goDirection(dir);

    --(*cart);
    *myself->roomp += *cart;
    myself->roomp->saveItems("");
    }
  }

  return 0;
}


