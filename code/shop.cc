#include <cmath>
#include <unistd.h>
#include <algorithm>

#include "stdsneezy.h"
#include "shop.h"
#include "statistics.h"
#include "database.h"
#include "obj_drug.h"
#include "obj_spellbag.h"
#include "obj_symbol.h"
#include "obj_general_weapon.h"
#include "obj_base_clothing.h"
#include "obj_magic_item.h"
#include "shopowned.h"
#include "obj_casino_chip.h"
#include "spec_objs_lottery_ticket.h"
#include "obj_component.h"
#include "obj_potion.h"

vector<shopData>shop_index(0);
int cached_shop_nr;
map <int,float> ratios_cache;
map <sstring,float> matches_cache;
map <sstring,float> player_cache;

const unsigned int SHOP_DUMP = 124;

// A note on gold_modifier[GOLD_SHOP] :
// This is a global variable used to keep economy in check
// it serves as a multiplier to raise/lower the price a shop buys an item
// for from a player.  It does NOT affect the sell price at all.  The
// reasoning on that was that selling prices should be low, but not less than
// the item's true cost.  But, a shop could reasonably buy an item for
// pennies on the dollar.
// This variable is automated in checkShopPrices().  Basically, if players
// are making money from the shops, it lowers the modifier, and if shops are
// raking in too much money, it raises it.
// It's presumed that shops should be a slight economic drain overall.  That
// is, for items (armor, weapons, items) it ought to be a wash, but food and
// other expendables slightly drain money from PCs.
// Obviously, by lowering the price shops buy items for, we also encourage
// PCs to use alternative (auctions) methods of item exchange.
// Batopr 1/21/99


bool shopData::isOwned(){
  return owned;
}


unsigned int find_shop_nr(int mobvnum)
{
  unsigned int shop_nr=0;

  for (shop_nr = 0; (shop_nr < shop_index.size()) && (shop_index[shop_nr].keeper != mobvnum); shop_nr++);

  if (shop_nr >= shop_index.size()) {
    vlogf(LOG_BUG, fmt("Warning... shop # for mobile %d (real nr) not found.") % mobvnum);
    return 0;
  }   

  return shop_nr;
}

float shopData::getProfitSell(const TObj *obj, const TBeing *ch)
{
  float profit_sell=shop_index[shop_nr].profit_sell;

  // if the shop is player owned, we check custom pricing
  if(shop_index[shop_nr].isOwned()){
    TDatabase db(DB_SNEEZY);

    db.query("select profit_sell from shopownedratios where shop_nr=%i and obj_nr=%i", shop_nr, obj->objVnum());

    if(db.fetchRow())
      profit_sell=convertTo<float>(db["profit_sell"]);
    else {
      // ok, shop is owned and there is no ratio set for this specific object
      // so check keywords
      db.query("select match, profit_sell from shopownedmatch where shop_nr=%i", shop_nr);

      while(db.fetchRow()){
	if(isname(db["match"], obj->name)){
	  profit_sell=convertTo<float>(db["profit_sell"]);
	  break;
	}
      }
    }

    db.query("select profit_sell from shopownedplayer where shop_nr=%i and lower(player)=lower('%s')", shop_nr, ch->name);
    
    if(db.fetchRow())
      profit_sell = convertTo<float>(db["profit_sell"]);

  }

  return profit_sell;
}

float shopData::getProfitBuy(const TObj *obj, const TBeing *ch)
{
  float profit_buy=-1;
  map <sstring,float>::iterator iter;
  TDatabase db(DB_SNEEZY);

  // we do this caching so that if we get the shopPrice on many items at once
  // (list) it doesn't have to query for each one
  // overhead is that with one at a time shopPrices (value/sell/buy) we may
  // rebuild the cache needlessly.  On the upside, when you've got someone
  // doing value/buy/sell in a shop, they're likely to do a list pretty soon
  // as well, and then we'll have the cache ready.
  if(cached_shop_nr != shop_nr){
    cached_shop_nr=shop_nr;
    ratios_cache.clear();
    matches_cache.clear();
    player_cache.clear();

    db.query("select obj_nr, profit_buy from shopownedratios where shop_nr=%i",
	     shop_nr);
    while(db.fetchRow())
      ratios_cache[convertTo<int>(db["obj_nr"])]=convertTo<float>(db["profit_buy"]);

    db.query("select match, profit_buy from shopownedmatch where shop_nr=%i",
	     shop_nr);
    while(db.fetchRow())
      matches_cache[db["match"]]=convertTo<float>(db["profit_buy"]);

    db.query("select player, profit_buy from shopownedplayer where shop_nr=%i",
	     shop_nr);
    while(db.fetchRow())
      player_cache[db["player"]]=convertTo<float>(db["profit_buy"]);
    
  }


  // if the shop is player owned, we check custom pricing
  if(shop_index[shop_nr].isOwned() && obj){  
    if(cached_shop_nr==shop_nr){
      if(ratios_cache.count(obj->objVnum()))
	profit_buy=ratios_cache[obj->objVnum()];
    } else {
      db.query("select profit_buy from shopownedratios where shop_nr=%i and obj_nr=%i", shop_nr, obj->objVnum());
      if(db.fetchRow())
	profit_buy=convertTo<float>(db["profit_buy"]);
    }
    
    if(profit_buy==-1){
      // ok, shop is owned and there is no ratio set for this specific object
      // so check keywords
      if(cached_shop_nr==shop_nr){
	for(iter=matches_cache.begin();iter!=matches_cache.end();++iter){
	  if(isname((*iter).first, obj->name)){
	    profit_buy=(*iter).second;
	    break;
	  }
	}
      } else {
	db.query("select match, profit_buy from shopownedmatch where shop_nr=%i", shop_nr);
	
	while(db.fetchRow()){
	  if(isname(db["match"], obj->name)){
	    profit_buy=convertTo<float>(db["profit_buy"]);
	    break;
	  }
	}
      }
    }
  }

  // no custom price found, so use the normal shop pricing
  if(profit_buy == -1)
    profit_buy=shop_index[shop_nr].profit_buy;


  // check for player specific modifiers
  if(shop_index[shop_nr].isOwned()){
    if(cached_shop_nr==shop_nr){
      for(iter=player_cache.begin();iter!=player_cache.end();++iter){
	if((*iter).first == (sstring) ch->name){
	  profit_buy = ((*iter).second);
	  break;
	}
      }
    } else {
      db.query("select profit_buy from shopownedplayer where shop_nr=%i and lower(player)=lower('%s')", shop_nr, ch->name);
      
      if(db.fetchRow()){
	profit_buy = convertTo<float>(db["profit_buy"]);
      }
    }
  }

  return profit_buy;
}


// this is the price the shop will buy an item for
int TObj::sellPrice(int, int shop_nr, float chr, const TBeing *ch)
{
  // adjust cost based on structure
  double cost = adjPrice();

  // adjust cost based on shop pricing
  cost *= shop_index[shop_nr].getProfitSell(this, ch);

  // adjust for charisma/swindle modifier
  if (chr != -1 && chr!=0)
    cost /= chr;

  // make sure we don't have a negative cost
  cost = max(1.0, cost);

  return (int) cost;
}

// this is price shop will sell it at
int TObj::shopPrice(int num, int shop_nr, float chr, const TBeing *ch) const
{    
  // adjust cost based on structure
  double cost = adjPrice();

  // adjust cost based on shop pricing
  cost *= shop_index[shop_nr].getProfitBuy(this, ch);

  // adjust for charisma/swindle modifier
  if(chr != -1)
    cost *= chr;

  // multiply by the number of items
  cost *= num;

  // make sure we don't have a negative cost
  cost = max(1.0, cost);

  return (int) cost;
}




bool shopData::willTradeWith(TMonster *keeper, TBeing *ch)
{
  int hmt = hourminTime();

  if (shop_index[shop_nr].open1 > hmt) {
     keeper->doSay("Come back later!");
    return FALSE;
  } else if (shop_index[shop_nr].close1 < hmt) {
    if (shop_index[shop_nr].open2 > hmt) {
      keeper->doSay("Sorry, we have closed, but come back later.");
      return FALSE;
    } else if (shop_index[shop_nr].close2 < hmt) {
      keeper->doSay("Sorry, come back tomorrow.");
      return FALSE;
    }
  }

  if (!keeper->canSee(ch) && !ch->isImmortal()) {
    keeper->doSay("I don't trade with someone I can't see!");
    return FALSE;
  }

  if (dynamic_cast<TMonster *>(ch) && (ch != keeper)) {
    keeper->doSay("Hey, no animals in my shop!");
    return FALSE;
  }

  return TRUE;
}

bool shopData::willBuy(const TObj *item)
{
  int counter, max_trade;
  bool mat_ok=FALSE;

  if (item->getValue() < 1 || 
      item->isObjStat(ITEM_NEWBIE) ||
      item->isObjStat(ITEM_PROTOTYPE))
    return FALSE;

  // check if there is a material type restriction
  if(IS_SET(shop_index[shop_nr].flags, SHOP_FLAG_MAT_RESTRICTED)){
    int max_mat_trade=0;

    max_mat_trade=shop_index[shop_nr].mat_type.size();

    for (counter = 0; counter < max_mat_trade; counter++) {
      if (shop_index[shop_nr].mat_type[counter] == item->getMaterial()){
	mat_ok=TRUE;
	break;
      }
    }
    if(mat_ok==FALSE)
      return FALSE;
  }

  // check normal shop types
  max_trade=shop_index[shop_nr].type.size();
  for (counter = 0; counter < max_trade; counter++) {
    if ((int) shop_index[shop_nr].type[counter] == item->itemType())
      return TRUE;
  }

  // obviously we will trade in anything we produce
  if (shop_index[shop_nr].isProducing(item))
    return TRUE;

  // if no produced/traded items are specified, but we do have a material
  // type, assume we take all items of that material type
  if(mat_ok==TRUE &&
     (shop_index[shop_nr].producing[0]== -1) &&
     (shop_index[shop_nr].type[0] == MAX_OBJ_TYPES))
    return TRUE;
  else
    return FALSE;
}


bool shopData::isProducing(const TObj *item)
{
  int counter, max_prod=0;
  TObj *o;

  if (item->number < 0)
    return FALSE;

  max_prod=producing.size();

  for (counter = 0; counter < max_prod; counter++) {
    if (producing[counter] <= -1)
      continue;

    if (producing[counter] == item->number) {
      if (!(o = read_object(producing[counter], REAL))) {
        vlogf(LOG_BUG, fmt("Major problems with shopkeeper number %d and item number %d.") %  shop_nr % item->number);
        return FALSE;
      }
      if (o->getName() && item->getName() && !strcmp(o->getName(), item->getName())) {
        delete o;
        o = NULL;
        return TRUE;
      }
      delete o;
      o = NULL;
    }
  }
  return FALSE;
}


static int number_objects_in_list(const TObj *item, const TObj *list)
{
  const TObj *i = NULL;
  const TThing *t;
  int count = 0;

  for (t = list; t; t = t->nextThing) {
    if(!(i = dynamic_cast<const TObj *>(t)))
      continue;

    if ((i->number == item->number) &&
        (i->getName() && item->getName() &&
	 !strcmp(i->getName(), item->getName())) &&
        (i->adjPrice() == item->adjPrice()))
      count++;
  }
  return (count);
}

void shopping_buy(const char *arg, TBeing *ch, TMonster *keeper, int shop_nr)
{
  char argm[MAX_INPUT_LENGTH], newarg[MAX_INPUT_LENGTH];
  int num = 1;
  TObj *temp1 = NULL;
  TThing *tt;

  *argm = '\0';

  if (!(shop_index[shop_nr].willTradeWith(keeper, ch)))
    return;

  strcpy(argm, arg);
  if (!*argm) {
    keeper->doTell(ch->name, "What do you want to buy??");
    return;
  }
  if ((num = getabunch(argm, newarg)))
    strcpy(argm, newarg);

  if (!num)
    num = 1;

  tt = searchLinkedListVis(ch, argm, keeper->getStuff());
  if (!tt || !(temp1 = dynamic_cast<TObj *>(tt))) {
    if (!(temp1 = get_num_obj_in_list(ch, convertTo<int>(argm), keeper->getStuff(), shop_nr))) {
      keeper->doTell(ch->name, shop_index[shop_nr].no_such_item1);
      return;
    }
  }

  if (temp1->getValue() <= 0) {
    keeper->doTell(ch->name, shop_index[shop_nr].no_such_item1);
    delete temp1;
    temp1 = NULL;
    return;
  }
  if (temp1->isObjStat(ITEM_PROTOTYPE | ITEM_NEWBIE)) {
    keeper->doSay("Where did this piece of junk come from???");
    delete temp1;
    temp1 = NULL;
    return;
  }

  temp1->buyMe(ch, keeper, num, shop_nr);
}

int TObj::buyMe(TBeing *ch, TMonster *keeper, int num, int shop_nr)
{
  sstring buf;
  char argm[256];
  int cost;
  int count = 0;
  int tmp;
  float chr;
  int i;

  if ((ch->getCarriedVolume() + (num * getTotalVolume())) > ch->carryVolumeLimit()) {
    ch->sendTo(fmt("%s: You can't carry that much volume.\n\r") % fname(name));
    return -1;
  }
  // obj-weight > free ch limit
  if (compareWeights(getTotalWeight(TRUE),
       ((ch->carryWeightLimit() - ch->getCarriedWeight())/num)) == -1) {
    ch->sendTo(fmt("%s: You can't carry that much weight.\n\r") % fname(name));
    return -1;
  }
  
  if (shop_index[shop_nr].isProducing(this) &&
      number_objects_in_list(this, (TObj *) keeper->getStuff()) <= 1){

    chr = ch->getChaShopPenalty() - ch->getSwindleBonus();
    chr = max((float)1.0,chr);

    cost = shopPrice(1, shop_nr, chr, ch);

    while (num-- > 0) {
      TObj *temp1;

      if(keeper->getMoney() < getValue()){
	keeper->doTell(ch->name, shop_index[shop_nr].missing_cash1);
	break;
      }



      temp1 = read_object(number, REAL);

      if ((ch->getMoney() < cost) && !ch->hasWizPower(POWER_GOD)) {
        keeper->doTell(ch->name, shop_index[shop_nr].missing_cash2);
    
        switch (shop_index[shop_nr].temper1) {
          case 0:
            keeper->doAction(ch->name, CMD_SMILE);
            break;
          case 1:
            act("$n grins happily.", 0, keeper, 0, 0, TO_ROOM);
            break;
          default:
            break;
        }
        delete temp1;
        temp1 = NULL;
        break;
      }
      *ch += *temp1;

      temp1->purchaseMe(ch, keeper, cost, shop_nr);
      // for unlimited items, charge the shopkeeper for production
      if(!dynamic_cast<TCasinoChip *>(this) &&
	 objVnum() != OBJ_LOTTERY_TICKET){
	keeper->addToMoney(-getValue(), GOLD_SHOP);
	shoplog(shop_nr, ch, keeper, temp1->getName(), -getValue(), "producing");
      }

      ch->logItem(temp1, CMD_BUY);
      count++;
    }
    buf = fmt("%s/%d") % SHOPFILE_PATH % shop_nr;
    keeper->saveItems(buf);
  } else {
    tmp = number_objects_in_list(this, (TObj *) keeper->getStuff());
    if (num > tmp) {
      keeper->doTell(ch->name, fmt("I don't have %d of that item. Here %s the %d I do have.") %
		     num  % ((tmp > 1) ? "are" : "is") % tmp);
    } else
      tmp = num;

    strcpy(argm, name);

    strcpy(argm, add_bars(argm).c_str());
    chr = ch->getChaShopPenalty() - ch->getSwindleBonus();
    chr = max((float)1.0,chr);

    cost = shopPrice(1, shop_nr, chr, ch);

    for (i = 0; i < tmp; i++) {
      TThing *t_temp1 = searchLinkedListVis(ch, argm, keeper->getStuff());
      TObj *temp1 = dynamic_cast<TObj *>(t_temp1);

#if !(NO_DAMAGED_ITEMS_SHOP)
      while (!temp1->isShopSimilar(this)) {
        // it's the same item, but in a different condition
        // keep scrolling through list
        if (temp1->nextThing) {
          t_temp1 = searchLinkedListVis(ch, argm, temp1->nextThing);
          temp1 = dynamic_cast<TObj *>(t_temp1);
          if (!temp1) {
            vlogf(LOG_BUG, "Error (2) in buyMe()");
            break;
          }
        } else {
          vlogf(LOG_BUG, "Error (1) in buyMe()");
          break;
        }
      }
#endif

      if ((ch->getMoney() < cost) && !ch->hasWizPower(POWER_GOD)) {
        keeper->doTell(ch->name, shop_index[shop_nr].missing_cash2);

        switch (shop_index[shop_nr].temper1) {
          case 0:
            keeper->doAction(ch->name, CMD_SMILE);
            break;
          case 1:
            act("$n grins happily.", 0, keeper, 0, 0, TO_ROOM);
            break;
          default:
            break;
        }
        // PC can't afford item, so just leave it on the keeper
        break;
      }

      if (!temp1 || !temp1) break;
      --(*temp1);
      *ch += *temp1;

      temp1->purchaseMe(ch, keeper, cost, shop_nr);

      ch->logItem(temp1, CMD_BUY);
      count++;
    }
    buf = fmt("%s/%d") % SHOPFILE_PATH % shop_nr;
    keeper->saveItems(buf);
  }
  if (!count)
    return -1;

  keeper->doTell(ch->name, fmt(shop_index[shop_nr].message_buy) %
          (cost * count));

  ch->sendTo(COLOR_OBJECTS, fmt("You now have %s (*%d).\n\r") % 
          sstring(getName()).uncap() % count);
  if (count == 1) 
    act("$n buys $p.", FALSE, ch, this, NULL, TO_ROOM); 
  else {
    buf = fmt("$n buys %s [%d].") % fname(name) % count;
    act(buf, FALSE, ch, this, 0, TO_ROOM);
  }
  ch->doSave(SILENT_YES);
  return cost;
}

bool will_not_buy(TBeing *ch, TMonster *keeper, TObj *temp1, int shop_nr)
{
  sstring buf;

  if (!ch->isImmortal() && temp1->objectSell(ch, keeper)) {
    return TRUE;
  }
#if NO_DAMAGED_ITEMS_SHOP
  if (temp1->getStructPoints() != temp1->getMaxStructPoints()) {
    buf = fmt("%s I don't buy damaged goods.") % ch->getName();
    keeper->doTell(buf);
    return TRUE;
  }
#endif
  if (temp1->getStuff()) {
    keeper->doTell(ch->getName(), "Sorry, I don't buy items that contain other items.");
    return TRUE;
  }
  // Notes have been denied by objectSell() above
  if (temp1->action_description) {
    keeper->doTell(ch->getName(), "I'm sorry, I don't buy monogrammed goods.");
    return TRUE;
  }
  if(temp1->isObjStat(ITEM_BURNING) || temp1->isObjStat(ITEM_CHARRED)){
    keeper->doTell(ch->getName(), "I'm sorry, I don't buy fire damaged goods.");
    return TRUE;
  }

  if (shop_index[shop_nr].isOwned() && temp1->isObjStat(ITEM_NORENT)){
    keeper->doTell(ch->getName(), "This shop is privately owned and we don't purchase non-rentable items.");
    return TRUE;
  }

  if(temp1->sellPrice(1, shop_nr, -1, ch) < 0){
    keeper->doTell(ch->getName(), "You'd have to pay me to buy that!");
    return TRUE;
  }

  return FALSE;
}


bool TObj::sellMeCheck(TBeing *ch, TMonster *keeper, int) const
{
  int total = 0;
  TThing *t;
  sstring buf;
  unsigned int shop_nr;

  for (shop_nr = 0; (shop_nr < shop_index.size()) && (shop_index[shop_nr].keeper != (keeper)->number); shop_nr++);

  if (shop_nr >= shop_index.size()) {
    vlogf(LOG_BUG, fmt("Warning... shop # for mobile %d (real nr) not found.") %  (keeper)->number);
    return FALSE;
  }
  
  TShopOwned tso(shop_nr, keeper, ch);
  int max_num=tso.getMaxNum(this);

  if(max_num == 0){
    keeper->doTell(ch->name, "I don't wish to buy any of those right now.");
    return TRUE;
  }

  for (t = keeper->getStuff(); t; t = t->nextThing) {
    if ((t->number == number) &&
        (t->getName() && getName() &&
         !strcmp(t->getName(), getName()))) {
      total += 1;
      if (total >= max_num) {
        keeper->doTell(ch->name, "I already have plenty of those.");
        return TRUE;
      }
    }
  }
  return FALSE;
}

void generic_num_sell(TBeing *ch, TMonster *keeper, TObj *obj, int shop_nr, int num)
{
  TComponent *tComp = dynamic_cast<TComponent *>(obj);

  if (obj->isObjStat(ITEM_NODROP)) {
    ch->sendTo("You can't let go of it, it must be CURSED!\n\r");
    return;
  }
  if (obj->isObjStat(ITEM_PROTOTYPE)) {
    ch->sendTo("That's a prototype, no selling that!\n\r");
    return;
  }
  if (!shop_index[shop_nr].willBuy(obj)) {
    keeper->doTell(ch->getName(), shop_index[shop_nr].do_not_buy);
    return;
  }
  if (will_not_buy(ch, keeper, obj, shop_nr)) 
    return;
  
  if (tComp) {
    tComp->sellMe(ch, keeper, shop_nr, num);
  } else {
    obj->sellMe(ch, keeper, shop_nr, 1);
  }
  // obj may be invalid here
}

void generic_sell(TBeing *ch, TMonster *keeper, TObj *obj, int shop_nr)
{
  TComponent *tComp = dynamic_cast<TComponent *>(obj);

  if (obj->isObjStat(ITEM_NODROP)) {
    ch->sendTo("You can't let go of it, it must be CURSED!\n\r");
    return;
  }
  if (obj->isObjStat(ITEM_PROTOTYPE)) {
    ch->sendTo("That's a prototype, no selling that!\n\r");
    return;
  }
  if (!shop_index[shop_nr].willBuy(obj)) {
    keeper->doTell(ch->getName(), shop_index[shop_nr].do_not_buy);
    return;
  }
  if (will_not_buy(ch, keeper, obj, shop_nr)) 
    return;
  
  if (tComp) {
    tComp->sellMe(ch, keeper, shop_nr, 1);
  } else {
    obj->sellMe(ch, keeper, shop_nr, 1);
  }
  // obj may be invalid here
}

void TObj::sellMe(TBeing *ch, TMonster *keeper, int shop_nr, int num = 1)
{
  int cost;
  sstring buf;
  float chr;

  if (!shop_index[shop_nr].profit_sell) {
    keeper->doTell(ch->getName(), shop_index[shop_nr].do_not_buy);
    return;
  }
  
  
  if (getValue() <= 1 || isObjStat(ITEM_NEWBIE)) {
    keeper->doTell(ch->getName(), "I'm sorry, I don't buy valueless items.");
    return;
  }
  if (sellMeCheck(ch, keeper, num))
    return;
  
  chr = ch->getChaShopPenalty() - ch->getSwindleBonus();
  chr = max((float)1.0,chr);
  cost = sellPrice(1, shop_nr, chr, ch);

  if (getStructPoints() != getMaxStructPoints()) {
    cost *= 6;    /* base deduction of 60% */
    cost /= 10;
    if (getMaxStructPoints() > 0) {
      cost *= getStructPoints();
      cost /= getMaxStructPoints();
    }
#if NO_DAMAGED_ITEMS_SHOP
    buf = fmt("%s It's been damaged, but I guess I can buy it as scrap.") %
      fname(ch->name);
    keeper->doTell(buf);
#endif
  }
  max(cost, 1);   // at least 1 talen 
  if (keeper->getMoney() < cost) {
    keeper->doTell(ch->getName(), shop_index[shop_nr].missing_cash1);
    return;
  }
  if (obj_index[getItemIndex()].max_exist <= 10) {
    keeper->doTell(ch->name, "Wow!  This is one of those limited items.");
    keeper->doTell(ch->name, "You should really think about auctioning it.");
  }
  act("$n sells $p.", FALSE, ch, this, 0, TO_ROOM);

  keeper->doTell(ch->getName(), fmt(shop_index[shop_nr].message_sell)% cost);

  ch->sendTo(COLOR_OBJECTS, fmt("The shopkeeper now has %s.\n\r") % sstring(getName()).uncap());
  ch->logItem(this, CMD_SELL);

  --(*this);
  *keeper += *this;

  sellMeMoney(ch, keeper, cost, shop_nr);

  if (ch->isAffected(AFF_GROUP) && ch->desc &&
           IS_SET(ch->desc->autobits, AUTO_SPLIT) && 
          (ch->master || ch->followers)){
    buf = fmt("%d") % cost;
    ch->doSplit(buf.c_str(), false);
  }

#if NO_DAMAGED_ITEMS_SHOP
  else if (getStructPoints() != getMaxStructPoints()) {
    // delete it as its "scrap"
    delete this;
  }
#endif


  buf = fmt("%s/%d") % SHOPFILE_PATH % shop_nr;
  keeper->saveItems(buf);
  ch->doSave(SILENT_YES);
}

int TThing::componentSell(TBeing *ch, TMonster *keeper, int shop_nr, TThing *)
{
  return FALSE;
}

int TThing::componentValue(TBeing *ch, TMonster *keeper, int shop_nr, TThing *)
{
  return FALSE;
}

int TThing::sellHidenSkin(TBeing *, TMonster *, int, TThing *)
{
  return FALSE;
}

// returns DELETE_THIS, VICT (ch), ITEM(sub)
int TSpellBag::componentValue(TBeing *ch, TMonster *keeper, int shop_nr,
TThing *)
{
  TThing *t, *t2;
  int rc;

  if (isClosed()) 
    return TRUE;
  
  for (t = getStuff(); t; t = t2) {
    t2 = t->nextThing;
    rc = t->componentValue(ch, keeper, shop_nr, this);
    if (IS_SET_DELETE(rc, DELETE_THIS)) {
      delete t;
      t = NULL;
    }
    if (IS_SET_DELETE(rc, DELETE_VICT))
      return DELETE_VICT;

    if (IS_SET_DELETE(rc, DELETE_ITEM))
      return DELETE_THIS;
  }
  return FALSE;
}


// returns DELETE_THIS, VICT (ch), ITEM(sub)
int TSpellBag::componentSell(TBeing *ch, TMonster *keeper, int shop_nr, TThing *)
{
  TThing *t, *t2;
  int rc;

  if (isClosed()) {
    // ignore closed spellbags
    return TRUE;
  }
  for (t = getStuff(); t; t = t2) {
    t2 = t->nextThing;
    rc = t->componentSell(ch, keeper, shop_nr, this);
    if (IS_SET_DELETE(rc, DELETE_THIS)) {
      delete t;
      t = NULL;
    }
    if (IS_SET_DELETE(rc, DELETE_VICT))
      return DELETE_VICT;

    if (IS_SET_DELETE(rc, DELETE_ITEM))
      return DELETE_THIS;
  }
  return FALSE;
}

int TThing::sellCommod(TBeing *ch, TMonster *keeper, int shop_nr, TThing *)
{
  return FALSE;
}

tObjectManipT ObjectManipType(sstring tStString, sstring & tStBuffer, itemTypeT & tItem)
{
  if (tStString.empty()) {
    return OBJMAN_NULL;
    tStBuffer = "";
  }

  sstring tStType(""),
         tStPassed("");

  tStBuffer = one_argument(tStString, tStType);
  tStPassed = tStType;
  if (!tStPassed.empty())
    if (!tStPassed.find("."))
      tStPassed = "";
    else
      tStPassed.erase(0, (tStPassed.find(".") + 1));

  if (!tStType.empty() && tStType.find("."))
    tStType.erase(tStType.find("."), (tStPassed.length() + 1));

  if (is_abbrev(tStType, "all")) {
    if (tStPassed.empty())
      return OBJMAN_ALL;

    if (is_abbrev(tStPassed, "fit"))
      return OBJMAN_FIT;

    if (is_abbrev(tStPassed, "nofit"))
      return OBJMAN_NOFIT;

    for (itemTypeT tItemType = MIN_OBJ_TYPE; tItemType < MAX_OBJ_TYPES; tItemType++)
      if (isname(tStPassed, ItemInfo[tItemType]->name)) {
        tItem = tItemType;
        return OBJMAN_TYPE;
      }

    return OBJMAN_ALLT;
  }

  return OBJMAN_NONE;
}

bool TObj::fitsSellType(tObjectManipT tObjectManip,
                        TBeing *ch, TMonster *tKeeper,
                        sstring tStString, itemTypeT tItemType,
                        int & tCount, int tShop)
{
  switch (tObjectManip) {
    case OBJMAN_NONE: // sell <object>
      if (isname(tStString, name) && tCount < 1) {
        tCount++;
        return true;
      }
      break;
    case OBJMAN_ALL: // sell all
      if (shop_index[tShop].willBuy(this)) {
        tCount++;
        return true;
      }
      break;
    case OBJMAN_ALLT: // sell all.<object>
      if (isname(tStString, name)) {
        tCount++;
        return true;
      }
      break;
    case OBJMAN_FIT: // sell all.fit
      if (fitInShop("yes", ch) && shop_index[tShop].willBuy(this)) {
        tCount++;
        return true;
      }
      break;
    case OBJMAN_NOFIT: // sell all.nofit
      if (!fitInShop("yes", ch) && shop_index[tShop].willBuy(this)) {
        tCount++;
        return true;
      }
      break;
    case OBJMAN_TYPE: // sell all.<type[type == 'component'/'light']>
      if (itemType() == tItemType) {
        tCount++;
        return true;
      }
      break;
    default:
      break;
  }

  return false;
}

int shopping_sell(const char *tString, TBeing *ch, TMonster *tKeeper, int shop_nr)
{
  char argm[MAX_INPUT_LENGTH], newarg[MAX_INPUT_LENGTH];
  sstring buf;
  TObj *temp1 = NULL;
  TThing *t, *t2;
  int rc = 0, i;
  int num = 1;

  if (!(shop_index[shop_nr].willTradeWith(tKeeper, ch)))
    return FALSE;

  strcpy(argm, tString);

  if (!*argm) {
    tKeeper->doTell(ch->getName(), "What do you want to sell??");
    return FALSE;
  }

  if ((num = getabunch(argm, newarg)))
    strcpy(argm, newarg);

  if (!num)
    num = 1;

  if (0 && gamePort != PROD_GAMEPORT) {
    sstring         tStString("");
    itemTypeT      tItemType;
    tObjectManipT  tObjectManip;
    int            tCount = 0;
    wearSlotT      tWear;
    TThing        *tThing,
                  *tThingTemp;
    TObj          *tObj;
    int            tShop = shop_nr;

    tObjectManip = ObjectManipType(argm, tStString, tItemType);

    if (tObjectManip == OBJMAN_NULL) {
        tKeeper->doTell(ch->getName(), "And what is it you want to sell??");
    }

    if (tObjectManip != OBJMAN_NONE)
      for (tWear = MIN_WEAR; tWear < MAX_WEAR; tWear++) {
        if (!ch->sameRoom(*tKeeper) || !ch->awake())
          break;

        if (!(tThing = ch->equipment[tWear]) ||
            !(tObj   = dynamic_cast<TObj *>(tThing)))
          continue;

        if (tObj->fitsSellType(tObjectManip, ch, tKeeper, tStString, tItemType, tCount, tShop));
          //generic_sell(ch, tKeeper, tObj, tShop);
      }

    for (tThing = ch->getStuff(); tThing; tThing = tThingTemp) {
      tThingTemp = tThing->nextThing;

      if (!ch->sameRoom(*tKeeper) || !ch->awake())
        break;

      if (!(tObj = dynamic_cast<TObj *>(tThing)))
        continue;

      if (tObj->fitsSellType(tObjectManip, ch, tKeeper, tStString, tItemType, tCount, tShop))
        generic_sell(ch, tKeeper, tObj, tShop);

      if (tObjectManip == OBJMAN_NONE && tCount >= 1)
        return FALSE;
    }

    return FALSE;
  } else {
    if (is_abbrev(argm, "all.commodity")) {
      for (i = MIN_WEAR; i < MAX_WEAR; i++) {
        // there's a chance to be moved (teleport moneypouch) so this is here
        if (!ch->sameRoom(*tKeeper))
          break;
        // check for sleep pouches
        if (!ch->awake())
          break;
        if (!(t = ch->equipment[i]))
          continue;
        rc = t->sellCommod(ch, tKeeper, shop_nr, NULL);
        if (IS_SET_DELETE(rc, DELETE_THIS)) {
          delete t;
          t = NULL;
          continue;
        }
        if (IS_SET_DELETE(rc, DELETE_VICT)) {
          return DELETE_THIS;
        }
      }
      for (t = ch->getStuff(); t; t = t2) {
        t2 = t->nextThing;
        if (!ch->sameRoom(*tKeeper))
          break;
        // check for sleep pouches
        if (!ch->awake())
          break;
        rc = t->sellCommod(ch, tKeeper, shop_nr, NULL);
        if (IS_SET_DELETE(rc, DELETE_THIS)) {
          delete t;
          t = NULL;
          continue;
        }
        if (IS_SET_DELETE(rc, DELETE_VICT)) {
          return DELETE_THIS;
        }
      }
      return FALSE;
    } else if (is_abbrev(argm, "all.components")) {
      for (i = MIN_WEAR; i < MAX_WEAR; i++) {
        if (!(t = ch->equipment[i]))
          continue;
        if (!ch->sameRoom(*tKeeper))
          break;
        // check for sleep pouches
        if (!ch->awake())
          break;

        TComponent *temp2 = dynamic_cast<TComponent *>(t);
        if (temp2) {
          if (num > 1) {
            rc = temp2->componentNumSell(ch, tKeeper, shop_nr, NULL, num);
          } else {
            rc = temp2->componentSell(ch, tKeeper, shop_nr, NULL);
          }
        }
        if (IS_SET_DELETE(rc, DELETE_THIS)) {
          delete t;
          t = NULL;
        }
        if (IS_SET_DELETE(rc, DELETE_VICT)) 
          return DELETE_THIS;
      }
      for (t = ch->getStuff(); t; t = t2) {
        t2 = t->nextThing;
        if (!ch->sameRoom(*tKeeper))
          break;
        // check for sleep pouches
        if (!ch->awake())
          break;

        TComponent *temp2 = dynamic_cast<TComponent *>(t);
        if (temp2) {
          if (num > 1) {
            rc = temp2->componentNumSell(ch, tKeeper, shop_nr, NULL, num);
          } else {
            rc = temp2->componentSell(ch, tKeeper, shop_nr, NULL);
          }
        }
        if (IS_SET_DELETE(rc, DELETE_THIS)) {
          delete t;
          t = NULL;
        }
        if (IS_SET_DELETE(rc, DELETE_VICT)) 
          return DELETE_THIS;
      }
      return FALSE;
    } else if (is_abbrev(argm, "all.hide") || is_abbrev(argm, "all.skin")) {
      for (i = MIN_WEAR; i < MAX_WEAR; i++) {
        if (!(t = ch->equipment[i]))
          continue;
        if (!ch->sameRoom(*tKeeper))
          break;
        // check for sleep pouches
        if (!ch->awake())
          break;

        rc = t->sellHidenSkin(ch, tKeeper, shop_nr, NULL);
        if (IS_SET_DELETE(rc, DELETE_THIS)) {
          delete t;
          t = NULL;
        }
        if (IS_SET_DELETE(rc, DELETE_VICT))
          return DELETE_THIS;
      }
      for (t = ch->getStuff(); t; t = t2) {
        t2 = t->nextThing;
        if (!ch->sameRoom(*tKeeper))
          break;
        // check for sleep pouches
        if (!ch->awake())
          break;

        rc = t->sellHidenSkin(ch, tKeeper, shop_nr, NULL);
        if (IS_SET_DELETE(rc, DELETE_THIS)) {
          delete t;
          t = NULL;
        }
        if (IS_SET_DELETE(rc, DELETE_VICT))
          return DELETE_THIS;
      }
      return FALSE;
    }
  }
  TThing *t_temp1 = searchLinkedListVis(ch, argm, ch->getStuff());
  temp1 = dynamic_cast<TObj *>(t_temp1);
  TComponent *temp2 = dynamic_cast<TComponent *>(temp1);

  if (!temp1) {
    tKeeper->doTell(ch->getName(), shop_index[shop_nr].no_such_item2);
    return FALSE;
  }
  if (temp2) {
    temp2->componentNumSell(ch, tKeeper, shop_nr, NULL, num);
  } else {
    generic_sell(ch, tKeeper, temp1, shop_nr);
  }

  return FALSE;
}

void shopping_value(const char *arg, TBeing *ch, TMonster *keeper, int shop_nr)
{
  char argm[MAX_INPUT_LENGTH], newarg[MAX_INPUT_LENGTH];
  sstring buf;
  TObj *temp1;
  int num = 1;

  if (!(shop_index[shop_nr].willTradeWith(keeper, ch)))
    return;

  strcpy(argm, arg);

  if (!*argm) {
    keeper->doTell(ch->name, "What do you want me to evaluate??");
    return;
  }
  
  if ((num = getabunch(argm, newarg)))
    strcpy(argm, newarg);

  if (!num)
    num = 1;

  if (is_abbrev(argm, "all.components")) {
    TThing *t, *t2;
    int i;
    for (i = MIN_WEAR; i < MAX_WEAR; i++) {
      if (!(t = ch->equipment[i]))
        continue;

      TComponent *temp2 = dynamic_cast<TComponent *>(t);
      if (temp2) {
        if (num > 1) {
          temp2->componentNumValue(ch, keeper, shop_nr, NULL, num);
        } else {
          temp2->componentValue(ch, keeper, shop_nr, NULL);
        }
      }
    }
    for (t = ch->getStuff(); t; t = t2) {
      t2 = t->nextThing;

      TComponent *temp2 = dynamic_cast<TComponent *>(t);
      if (temp2) {
        if (num > 1) {
          temp2->componentNumValue(ch, keeper, shop_nr, NULL, num);
        } else {
          temp2->componentValue(ch, keeper, shop_nr, NULL);
        }
      }
    }
    return;
  }

  TThing *t_temp1 = searchLinkedListVis(ch, argm, ch->getStuff());
  temp1 = dynamic_cast<TObj *>(t_temp1);
  if (!temp1) {
    keeper->doTell(ch->name, shop_index[shop_nr].no_such_item2);
    return;
  }
  if (!(shop_index[shop_nr].willBuy(temp1))) {
    keeper->doTell(ch->name, shop_index[shop_nr].do_not_buy);
    return;
  }
  if (will_not_buy(ch, keeper, temp1, shop_nr)) 
    return;
  
  TComponent *temp2 = dynamic_cast<TComponent *>(temp1);
  if (temp2) {
    temp2->valueMe(ch, keeper, shop_nr, num);
  } else {
    temp1->valueMe(ch, keeper, shop_nr, 1);
  }
}

void TObj::valueMe(TBeing *ch, TMonster *keeper, int shop_nr, int num = 1)
{
  float chr;
  int cost;
  sstring buf;
  int willbuy=0;
  
#if 0
  if (sellMeCheck(ch, keeper, num))
    return;
#else
  willbuy=!sellMeCheck(ch, keeper, num);
#endif

  if (!shop_index[shop_nr].willBuy(this)) {
    keeper->doTell(ch->getName(), shop_index[shop_nr].do_not_buy);
    return;
  }

  chr = ch->getChaShopPenalty();

  // do not adjust for swindle on valueing, give them worst case price
  chr = max((float)1.0,chr);

  cost = sellPrice(1, shop_nr, chr, ch);

  if (obj_index[getItemIndex()].max_exist <= 10) {
    keeper->doTell(ch->name, "Wow!  This is one of those limited items.");
    keeper->doTell(ch->name, "You should really think about auctioning it.");
  }

  if ((getStructPoints() != getMaxStructPoints()) &&
       (getMaxStructPoints() > 0)) {
    cost *= 6;    /* base deduction of 60% */
    cost /= 10;
    cost *= getStructPoints();
    cost /= getMaxStructPoints();
#if NO_DAMAGED_ITEMS_SHOP
    buf = fmt("%s It's been damaged, but I guess I can buy it as scrap.") %
             fname(ch->name);
    keeper->doTell(buf);
#endif
  }
  max(cost, 1);  // at least 1 talen
  if(willbuy){
    buf = fmt("I'll give you %d talens for %s!") % cost % getName();
  } else {
    buf = fmt("Normally, I'd give you %d talens for %s!") % cost % getName();
  }
  keeper->doTell(ch->name, buf);

  if (keeper->getMoney() < cost) {
    keeper->doTell(fname(ch->name), "Unfortunately, at the moment, I can not afford to buy that item from you.");
    return;
  }

  return;
}

const sstring TObj::shopList(const TBeing *ch, const sstring &arg, int iMin, int iMax, int num, int shop_nr, int k, unsigned long int FitT) const
{
  int cost, found = FALSE;
  char buf[256];
  char buf3[256];
  char buf4[256];
  char capbuf[256];
  char atbuf[25];
  char wcolor[25];
  int counter;
  float chr;
  float perc;
  bool isWearable = false;
  const TGenWeapon * tWeapon = dynamic_cast<const TGenWeapon *>(this);
  const TComponent *tComp = dynamic_cast<const TComponent *>(this);

  // display spells on things like scrolls
  // don't show the "level" of weaps/armor though
  if (shop_index[shop_nr].isProducing(this) &&
      dynamic_cast<const TMagicItem *>(this)) {
    sprintf(capbuf, "%s", getNameForShow(false, false, ch).c_str());
  } else
    sprintf(capbuf, "%s", getNameNOC(ch).c_str());

  sprintf(atbuf, "%d", num);

  if (strlen(capbuf) > 29) {
    capbuf[26] = '.';
    capbuf[27] = '.';
    capbuf[28] = '.';
    capbuf[29] = '\0';
  }
  chr = ch->getChaShopPenalty();
  // do not adjust for swindle on list, give them worst case price
  chr = max((float)1.0, chr);

  cost = shopPrice(1, shop_nr, chr, ch);

  wearSlotT slot;
  slot = slot_from_bit(obj_flags.wear_flags);

  if (slot > 0) {
    perc = (((double) ch->getHeight()) * (double) race_vol_constants[mapSlotToFile(slot)]);
    if (isPaired())
      perc *= 2;
  } else
    perc = 0;

  const TBaseClothing *tbc = dynamic_cast<const TBaseClothing *>(this);

  if (!ch->canUseEquipment(this, SILENT_YES)) {
    sprintf(buf3, "forbidden");
  } else if (tbc && tbc->isSaddle()) {
    sprintf(buf3, "for mounts");
  } else if ((slot == HOLD_LEFT) || (slot == HOLD_RIGHT)) {
    if (isPaired()) {
      // weight > ch-wield_weight
      if (compareWeights(getWeight(), ch->maxWieldWeight(this, HAND_TYPE_BOTH)) == -1) {
        if ((isSlashWeapon() &&
             (ch->getSkillValue(SKILL_SLASH_PROF) < MAX_SKILL_LEARNEDNESS)) ||
             (isPierceWeapon() &&
             (ch->getSkillValue(SKILL_PIERCE_PROF) < MAX_SKILL_LEARNEDNESS)) ||
             (isBluntWeapon() &&
             (ch->getSkillValue(SKILL_BLUNT_PROF) < MAX_SKILL_LEARNEDNESS))) {
          sprintf(buf3, "not proficient");
        } else {
          sprintf(buf3, "too heavy");
        }
      } else {
        sprintf(buf3, "paired");
        isWearable = true;
      }
    } else {
      // see if I can hold it in primary hand
      // weight > ch-wield_weight
      if (compareWeights(getWeight(), ch->maxWieldWeight(this, HAND_TYPE_PRIM)) == -1) {
        if ((isSlashWeapon() &&
             (ch->getSkillValue(SKILL_SLASH_PROF) < MAX_SKILL_LEARNEDNESS)) ||
             (isPierceWeapon() &&
             (ch->getSkillValue(SKILL_PIERCE_PROF) < MAX_SKILL_LEARNEDNESS)) ||
             (isBluntWeapon() &&
             (ch->getSkillValue(SKILL_BLUNT_PROF) < MAX_SKILL_LEARNEDNESS))) {
          sprintf(buf3, "not proficient");
        } else 
          sprintf(buf3, "too heavy");
      } else {
        // weight > ch-wield_weight
        if (compareWeights(getWeight(), ch->maxWieldWeight(this, HAND_TYPE_SEC)) == -1) {
          if (tbc && tbc->isShield())
            sprintf(buf3, "too heavy");   // shields are secondary hand only
          else {
            sprintf(buf3, "primary only");
            isWearable = true;
          }
        } else {
          if (tbc && tbc->isShield()) {
            sprintf(buf3, "secondary only");
            isWearable = true;
          } else {
            sprintf(buf3, "either hand");
            isWearable = true;
          }
        }
      }
    }
  } else if ((slot != WEAR_NECK) && (slot != WEAR_FINGER_R) && (slot != WEAR_FINGER_L)) {
    if (dynamic_cast<const TBaseClothing *>(this)) {
      if (getVolume() > (int) (perc/0.85))
        sprintf(buf3, "too big");
      else if (getVolume() < (int) (perc/1.15))
        sprintf(buf3, "too small");
      else {
        sprintf(buf3, "yes");
        isWearable = true;
      }
    } else 
      sprintf(buf3, "N/A");
  } else {
    sprintf(buf3, "yes");
    isWearable = true;
  } 

  if (tComp) {
    if (shop_index[shop_nr].isProducing(this)){
      sprintf(buf4, "[%s]", "Unlimited");
    } else {
      sprintf(buf4, "[%d]", tComp->getComponentCharges());
    }
  } else {
    sprintf(buf4, "[%s]", (shop_index[shop_nr].isProducing(this) ? "Unlimited" : atbuf));
  }
  found = FALSE;
  char equipCond[256];
  char equipColor[80];
  int max_trade;

  max_trade=shop_index[shop_nr].type.size()-1;

  strcpy(wcolor, ANSI_NORMAL);

  if (isWearable)
    if (tWeapon) {
      if (isPaired() || (tbc && tbc->isShield()) || compareWeights(getWeight(), ch->maxWieldWeight(this, HAND_TYPE_SEC) == -1))
        strcpy(wcolor, ch->greenBold());
      else
        strcpy(wcolor, ch->green());

    } else {
      if (isPaired())
        strcpy(wcolor, ch->greenBold());
      else
        strcpy(wcolor, ch->green());
    }

  for (counter = 0; counter < max_trade; counter++) {
    if ((shop_index[shop_nr].type[counter] == ITEM_WORN) ||
        (shop_index[shop_nr].type[counter] == ITEM_ARMOR) ||
        (shop_index[shop_nr].type[counter] == ITEM_JEWELRY) ||
        (shop_index[shop_nr].type[counter] == ITEM_BOW) ||
        (shop_index[shop_nr].type[counter] == ITEM_MARTIAL_WEAPON) ||
        (shop_index[shop_nr].type[counter] == ITEM_HOLY_SYM) ||
        (shop_index[shop_nr].type[counter] == ITEM_WEAPON)) {
      strcpy(equipColor, equip_condition(-1).c_str());
      strcpy(equipCond, equipColor + 3); 
      equipColor[3] = '\0';
      sprintf(buf, "%s[%2d] %-29s %s%-12s %-6d %-5s %s%s\n\r",
             wcolor, k + 1, sstring(capbuf).cap().c_str(),
             equipColor, equipCond, cost, 
             buf4, buf3, ch->norm());
      found = TRUE;
      strcpy(wcolor, ch->norm());
      break;
    }
  }

  if (!found) {
    strcpy(equipColor, equip_condition(-1).c_str());
    strcpy(equipCond, equipColor + 3);
    equipColor[3] = '\0';
    sprintf(buf, "%s[%2d] %-31s %s%-12s %-6d %-5s\n\r",
            wcolor, k + 1, sstring(capbuf).cap().c_str(), equipColor, equipCond, cost, buf4);
  }

  // This is for quick listing, fast and simple.
  if (arg.empty() && iMin == 999999 && FitT == 0)
    return buf;

  // This is designed to be a large list of checks.  All the checks but
  // 2 are compared as a single.  If they want 'fit' then it HAS to match.
  // This way: list fit head back  will list out anything that is wearable
  // on the head or back And fits, not Or fits.  The same holds true for
  // name matching And price matching.  this way:
  // list 100 1000 fit head back neck hard-leather
  // Lists all items between 100-1000, fits on the head or back or neck, and
  // has hard-leather in the name And fits the current lister.

  if (((FitT & (1 << 0)) == 0 || fitInShop(buf3, ch)) &&
      (arg.empty() || isname(arg, name)) &&
      (iMin == 999999 || (cost >= iMin && cost <= iMax)) &&
      ((FitT & (1 << 15)) == 0 || isObjStat(ITEM_GLOW)) &&
      ((FitT & (1 << 16)) == 0 || isObjStat(ITEM_SHADOWY)) &&
      ((FitT & (1 << 18)) == 0 || (tWeapon && tWeapon->canStab())) &&
      ((FitT & (1 << 19)) == 0 || (tWeapon && tWeapon->canCudgel())) &&
      ((FitT & (1 << 20)) == 0 || (tWeapon && tWeapon->canBackstab())) &&
      (((FitT & (1 <<  1)) && isSlashWeapon()) ||
       ((FitT & (1 <<  2)) && isPierceWeapon()) ||
       ((FitT & (1 <<  3)) && isBluntWeapon()) ||
       ((FitT & (1 <<  4)) && canWear(ITEM_WEAR_BODY)) ||
       ((FitT & (1 <<  5)) && canWear(ITEM_WEAR_FINGER)) ||
       ((FitT & (1 <<  6)) && canWear(ITEM_WEAR_WRIST)) ||
       ((FitT & (1 <<  7)) && canWear(ITEM_WEAR_LEGS)) ||
       ((FitT & (1 <<  8)) && canWear(ITEM_WEAR_ARMS)) ||
       ((FitT & (1 <<  9)) && canWear(ITEM_WEAR_NECK)) ||
       ((FitT & (1 << 10)) && canWear(ITEM_WEAR_FEET)) ||
       ((FitT & (1 << 11)) && canWear(ITEM_WEAR_HANDS)) ||
       ((FitT & (1 << 12)) && canWear(ITEM_WEAR_HEAD)) ||
       ((FitT & (1 << 13)) && canWear(ITEM_WEAR_BACK)) ||
       ((FitT & (1 << 14)) && canWear(ITEM_WEAR_WAISTE)) ||
       ((FitT & (1 << 17)) && isPaired()) ||
       ((FitT - (FitT & ((1 << 0) | (1 << 15) | (1 << 16)))) == 0)))
    return buf;
  else
    return "";
}

void shopping_list(sstring argument, TBeing *ch, TMonster *keeper, int shop_nr)
{
  vector<TObj *>cond_obj_vec;
  vector<int>cond_tot_vec;
  TObj *i = NULL;
  unsigned int k;
  unsigned long int FitT = 0;
  int iMin = 999999, iMax = 0;
  int found_obj, counter, rc;
  bool found = FALSE;
  vector <sstring> args;
  sstring buf, sb, arg;
  bool hasComponents = false;
  bool owned=shop_index[shop_nr].isOwned();

#if 0
  if (gamePort != PROD_GAMEPORT) {
    if (ch->desc && ch->desc->m_bIsClient)
      desc->clientShoppingList(arg, keeper, shop_nr);
  }
#endif

  if (!shop_index[shop_nr].willTradeWith(keeper, ch))
    return;

  if (!ch->desc)
    return;

  // Here we rip apart what they might have passed.  We do it
  // this way to allow them to form it in any fashion they want.
  // For numbers.  First number found is givin to iMax.  Second
  // number found is givin to iMax and iMin is givin the old iMax
  // value.  So:
  //   1 number : No floor value, value is considered max.
  //   2 numbers: First is floor, 2nd is max.
  for(int i=0;!argument.word(i).empty();++i){
         if (is_abbrev(argument.word(i), "fit")    ) FitT |= (1 <<  0);
    else if (is_abbrev(argument.word(i), "slash")  ) FitT |= (1 <<  1);
    else if (is_abbrev(argument.word(i), "pierce") ) FitT |= (1 <<  2);
    else if (is_abbrev(argument.word(i), "blunt")  ) FitT |= (1 <<  3);
    else if (is_abbrev(argument.word(i), "body")   ) FitT |= (1 <<  4);
    else if (is_abbrev(argument.word(i), "finger") ) FitT |= (1 <<  5);
    else if (is_abbrev(argument.word(i), "wrist")  ) FitT |= (1 <<  6);
    else if (is_abbrev(argument.word(i), "legs")   ) FitT |= (1 <<  7);
    else if (is_abbrev(argument.word(i), "arms")   ) FitT |= (1 <<  8);
    else if (is_abbrev(argument.word(i), "neck")   ) FitT |= (1 <<  9);
    else if (is_abbrev(argument.word(i), "feet")   ) FitT |= (1 << 10);
    else if (is_abbrev(argument.word(i), "hands")  ) FitT |= (1 << 11);
    else if (is_abbrev(argument.word(i), "head")   ) FitT |= (1 << 12);
    else if (is_abbrev(argument.word(i), "back")   ) FitT |= (1 << 13);
    else if (is_abbrev(argument.word(i), "waist")  ) FitT |= (1 << 14);
    else if (is_abbrev(argument.word(i), "glowing")) FitT |= (1 << 15);
    else if (is_abbrev(argument.word(i), "shadowy")) FitT |= (1 << 16);
    else if (is_abbrev(argument.word(i), "paired") ) FitT |= (1 << 17);
    else if (is_abbrev(argument.word(i), "stab")) {
      if ((ch->doesKnowSkill(SKILL_STABBING)) || ch->isImmortal()) {
        FitT |= (1 << 18);
        FitT |= (1 <<  2);
      }
    } else if (is_abbrev(argument.word(i), "cudgel")) {
      if ((ch->doesKnowSkill(SKILL_CUDGEL)) || ch->isImmortal()) {
        FitT |= (1 << 19);
        FitT |= (1 <<  3);
      }
    } else if (is_abbrev(argument.word(i), "backstab")) {
      if ((ch->doesKnowSkill(SKILL_BACKSTAB)) || ch->isImmortal()) {
        FitT |= (1 << 20);
        FitT |= (1 <<  2);
      }
    } else if (is_abbrev(argument.word(i), "slit")) {
      if ((ch->doesKnowSkill(SKILL_THROATSLIT)) || ch->isImmortal()) {
        FitT |= (1 << 20);
        FitT |= (1 <<  2);
      }
    } else if (is_number(argument.word(i))) {
      if (iMin == 999999) {
        iMin = 0;
        iMax = convertTo<int>(argument.word(i));
      } else if (iMin == 0) {
        iMin = iMax;
        iMax = convertTo<int>(argument.word(i));
      }
    } else if (!argument.word(i).empty()) {
      arg=argument.word(i);
    }
    if (argument.empty())
      break;
  }

  int max_trade=0;
  max_trade=shop_index[shop_nr].type.size()-1;

  keeper->doTell(ch->getName(), "You can buy:");
  for (counter = 0; counter < max_trade; counter++) {
    if (shop_index[shop_nr].type[counter] == ITEM_COMPONENT)
      hasComponents = true;

    if (shop_index[shop_nr].type[counter] == ITEM_WORN || 
        shop_index[shop_nr].type[counter] == ITEM_ARMOR || 
        shop_index[shop_nr].type[counter] == ITEM_JEWELRY || 
        shop_index[shop_nr].type[counter] == ITEM_WEAPON || 
        shop_index[shop_nr].type[counter] == ITEM_MARTIAL_WEAPON || 
        shop_index[shop_nr].type[counter] == ITEM_HOLY_SYM || 
        shop_index[shop_nr].type[counter] == ITEM_BOW) {
      sb += "     Item Name                          Condition Price    Number      Fit?\n\r";
      found = TRUE;
      break;
    }
  }
  if (!found)
    if (!hasComponents || gamePort == PROD_GAMEPORT)
      sb += "     Item Name                            Condition Price     Number\n\r";
    else
      sb += "     Item Name                            Condition Price     Number\n\r";

  found = FALSE;
  sb += "-------------------------------------------------------------------------------\n\r";

  found_obj = FALSE;

  TThing *t, *t2;
  for (t = keeper->getStuff(); t; t = t2) {
    t2 = t->nextThing;
    i = dynamic_cast<TObj *>(t);
    if (!i)
      continue;
    if (ch->canSee(i)) {
      if ((i->getValue() > 1) &&
          !i->isObjStat(ITEM_NEWBIE) &&
#if NO_DAMAGED_ITEMS_SHOP
          (i->getMaxStructPoints() == i->getStructPoints()) &&
#endif
          shop_index[shop_nr].willBuy(i)) {
        found = FALSE;
        for (k = 0; (k < cond_obj_vec.size() && !found); k++) {
          if (cond_obj_vec.size() > 0) {
            if (i->isShopSimilar(cond_obj_vec[k])) {
	      cond_tot_vec[k] += 1;
	      found = TRUE;
            }
          }
        }
        if (!i)
          continue;

        if (!found) {
          cond_obj_vec.push_back(i);
          cond_tot_vec.push_back(1);
        }
      } else {
        if (i->isPersonalized()) {
          keeper->doSay("Hmmm, I didn't notice this monogram before...");
          keeper->doSay("Well, no one's going to buy it now.");
          // emote the junk since doJunk aborts on monogram
          act("$n junks $p.", FALSE, keeper, i, 0, TO_ROOM);
          delete i;
          i = NULL;
          continue;
        }
        // pawn shop shouldn't junk
        if (shop_index[shop_nr].in_room != 562 || owned) {
          keeper->doSay("How did I get this piece of junk?!?!");
          rc = keeper->doJunk("", i);
          // doJunk might fail (cursed, etc), delete regardless
          delete i;
          i = NULL;
          continue;
        }
      }
    }
  }                             // for loop 
  if (cond_obj_vec.size()) {
    found_obj = TRUE;
    for (k = 0; k < cond_obj_vec.size(); k++) 
      sb += cond_obj_vec[k]->shopList(ch, arg, iMin, iMax, cond_tot_vec[k], shop_nr, k, FitT);
  }
  if (!found_obj) {
    buf = "Nothing!\n\r";
    sb += buf;

    if (ch->desc)
      ch->desc->page_string(sb, SHOWNOW_NO, ALLOWREP_YES);

    keeper->autoCreateShop(shop_nr);
    buf = fmt("%s/%d") % SHOPFILE_PATH % shop_nr;
    keeper->saveItems(buf);
    return;
  }
  if (ch->desc) {
    if (!ch->desc->m_bIsClient)
      ch->desc->page_string(sb, SHOWNOW_NO, ALLOWREP_YES);
    else 
      ch->desc->page_string(sb, SHOWNOW_NO, ALLOWREP_YES);
  }
  return;
}

void TMonster::autoCreateShop(int shop_nr)
{
  TObj *obj;
  int max_prod=0;

  // have to watch to make sure that first slot is not -1 unless all
  // slots are -1
  if (shop_index[shop_nr].producing[0] == -1)
    return;

  if (getStuff())  // just can't see the shopkeepers inventory so lists nada?
    return;

  vlogf(LOG_MISC,fmt("Creating a new shopfile for %s (shop #%d)") % getName() %shop_nr);
  doSay("Whoops, I seem to have run out of everything.");
  doSay("One moment while I go back and get some more stuff.");
  act("$n slips quickly into the storeroom.",0, this, 0, 0, TO_ROOM);
  act("$n returns laden with new goodies ready to sell.",0, this, 0, 0, TO_ROOM);
  doSay("OK, sorry for the delay, list again to see my NEW list.");

  max_prod=shop_index[shop_nr].producing.size()-1;
  for (int i=max_prod-1; i >= 0; i--) {

    if (shop_index[shop_nr].producing[i] >= 0) {
      obj = read_object(shop_index[shop_nr].producing[i], REAL);
      *this += *obj;
    }
  }
  return;
}

// if we process the command, return TRUE.
// if they are looking at something else (shopkeeper, etc) return FALSE
static bool shopping_look(const char *arg, TBeing *ch, TMonster *keeper, int shop_nr)
{
  const char *tmp_desc;
  int value;
  TObj *temp1;

  if (!*arg) 
    return FALSE;   // generic: look

  if (!(shop_index[shop_nr].willTradeWith(keeper, ch)) || !ch->desc)
    return FALSE;

  TThing *t_temp1 = searchLinkedListVis(ch, arg, keeper->getStuff());
  temp1 = dynamic_cast<TObj *>(t_temp1);
  if (!temp1) {
    // check for 4.xxx syntax, we already know anything like that is NOT
    // in shopkeepers possession
    if (strchr(arg, '.'))
      return FALSE;
    value = convertTo<int>(arg);
    if (!value || 
    !(temp1 = get_num_obj_in_list(ch, value, keeper->getStuff(), shop_nr))) {
      // it's not one of my objects so see if the look thing is in room
      return FALSE;
    }
  }
  sstring str = "You examine ";
  if (shop_index[shop_nr].isProducing(temp1)) {
    str += temp1->getNameForShow(true, false, ch);
  } else
    str += temp1->getName();
  str += " sold by $N.";

  act(str, FALSE, ch, temp1, keeper, TO_CHAR);

  tmp_desc = NULL;
  if ((tmp_desc = temp1->ex_description->findExtraDesc(fname(temp1->name).c_str()))) {
    ch->desc->page_string(tmp_desc);
  } else {
    ch->sendTo("You see nothing special.\n\r");
  }
  ch->describeObject(temp1);
  ch->showTo(temp1, SHOW_MODE_PLUS);  // tack on glowing, humming, etc
  return TRUE;
}

// if we process the command, return TRUE.
// if they are looking at something else (shopkeeper, etc) return FALSE
static bool shopping_evaluate(const char *arg, TBeing *ch, TMonster *keeper, int shop_nr)
{
  char newarg[100];
  int num;
  TObj *temp1;

  if (!*arg) 
    return FALSE;   // generic: look

  if (!(shop_index[shop_nr].willTradeWith(keeper, ch)) || !ch->desc)
    return FALSE;

  if (!(num = getabunch(arg, newarg)))
    strcpy(newarg, arg);

  if (!num)
    num = 1;

  TThing *t_temp1 = searchLinkedListVis(ch, newarg, keeper->getStuff());
  temp1 = dynamic_cast<TObj *>(t_temp1);
  if (!temp1) {
    if (!(temp1 = get_num_obj_in_list(ch, convertTo<int>(newarg), keeper->getStuff(), shop_nr))) {
      // it's not one of my objects so see if the look thing is in room
      return FALSE;
    }
  }
  act("You evaluate $p sold by $N.", FALSE, ch, temp1, keeper, TO_CHAR);

  ch->genericEvaluateItem(temp1);
  return TRUE;
}

void shopping_kill(const char *, TBeing *ch, TBeing *keeper, int shop_nr)
{
  sstring buf;

  switch (shop_index[shop_nr].temper2) {
    case 0:
      keeper->doTell(ch->name, "Don't ever try that again!");
      return;
    case 1:
      keeper->doTell(ch->name, "Scram - midget!");
      return;

    default:
      return;
  }
}

void waste_shop_file(int shop_nr)
{
  sstring buf;

  buf = fmt("%s/%d") % SHOPFILE_PATH % shop_nr;
  unlink(buf.c_str());
}



int shop_keeper(TBeing *ch, cmdTypeT cmd, const char *arg, TMonster *myself, TObj *o)
{
  int rc;
  dirTypeT dir = DIR_NONE;
  unsigned int shop_nr;
  TBeing *tbt = NULL;

  if (cmd == CMD_GENERIC_PULSE) {
    TThing *t;
    TBeing *tbt;

    // Toss out idlers
    for(t=myself->roomp->getStuff();t;t=t->nextThing){
      if((tbt=dynamic_cast<TBeing *>(t)) && 
	 tbt->getTimer()>1 && !tbt->isImmortal()){
        if ((tbt->master) && tbt->master->inRoom() == tbt->inRoom()) {
          //vlogf(LOG_DASH, fmt("saving %s from loitering code, master is %s, room is (%d == %d)") % tbt->getName() %
          //      tbt->master->getName() % tbt->inRoom() % tbt->master->inRoom());
	  continue;
	}
	myself->doSay("Hey, no loitering!  Make room for the other customers.");
	for (dir = MIN_DIR; dir < MAX_DIR; dir++) {
	  if (exit_ok(myself->exitDir(dir), NULL)) {
	    // at least one valid dir exists
	    // select the true direction at random
	    do {
	      dir = dirTypeT(::number(MIN_DIR, MAX_DIR-1));
	    } while (!exit_ok(myself->exitDir(dir), NULL));
	    
	    act("$n throws you from $s shop.",
		FALSE, myself, 0, tbt, TO_VICT);
	    act("$n throws $N from $s shop.",
		FALSE, myself, 0, tbt, TO_NOTVICT);
	    myself->throwChar(tbt, dir, FALSE, SILENT_NO, true);
	    return TRUE;
	  }
	}
      }
    }
    return TRUE;
  }  

  // determine shop_nr here to avoid overhead before CMD_GENERIc_PULSE
  for (shop_nr = 0; (shop_nr < shop_index.size()) && (shop_index[shop_nr].keeper != (myself)->number); shop_nr++);

  if (shop_nr >= shop_index.size()) {
    vlogf(LOG_BUG, fmt("Warning... shop # for mobile %d (real nr) not found.") %  (myself)->number);
    return FALSE;
  }

  //    if(shop_index[shop_nr].isOwned()){
  //   vlogf(LOG_PEEL, fmt("shop_nr %i, charged tax") %  shop_nr);
  //    }



  if (cmd == CMD_GENERIC_INIT) {
    if (!myself->isUnique()) {
      vlogf(LOG_BUG, fmt("Warning!  %s attempted to be loaded, when not unique.") %  myself->getName());
      return TRUE;
    } else
      return FALSE;
  } else if (cmd == CMD_GENERIC_CREATED) {
    // Little kludge I put in to set pawnman is set for rent stuff - Russ 
    if (myself->mobVnum() == MOB_PAWNGUY) {
      vlogf(LOG_MISC, "Setting Pawn Broker pointer for rent functions!");
      pawnman = myself;
    }
    myself->loadItems(fmt("%s/%d") % SHOPFILE_PATH % shop_nr);
    return FALSE;
  } else if (cmd == CMD_MOB_VIOLENCE_PEACEFUL) {
    myself->doSay("Hey!  Take it outside.");
    // o is really a being, so downcast, and then bring it back up
    TThing *ttt = o;
    tbt = dynamic_cast<TBeing *>(ttt);

    for (dir = MIN_DIR; dir < MAX_DIR; dir++) {
      if (exit_ok(myself->exitDir(dir), NULL)) {
        // at least one valid dir exists
        // select the true direction at random
        do {
          dir = dirTypeT(::number(MIN_DIR, MAX_DIR-1));
        } while (!exit_ok(myself->exitDir(dir), NULL));

        act("$n throws you from $s shop.",
               FALSE, myself, 0, ch, TO_VICT);
        act("$n throws $N from $s shop.",
               FALSE, myself, 0, ch, TO_NOTVICT);
        myself->throwChar(ch, dir, FALSE, SILENT_NO, true);
        act("$n throws you from $s shop.",
               FALSE, myself, 0, tbt, TO_VICT);
        act("$n throws $N from $s shop.",
               FALSE, myself, 0, tbt, TO_NOTVICT);
        myself->throwChar(tbt, dir, FALSE, SILENT_NO, true);
        return TRUE;
      }
    }
    return TRUE;
  } else if (cmd == CMD_MOB_MOVED_INTO_ROOM &&  (myself->in_room == shop_index[shop_nr].in_room)) {
    if (dynamic_cast<TBeing *>(ch->riding)) {
      sstring buf;
      buf = fmt("Hey, get that damn %s out of my shop!") % fname(ch->riding->name);
      myself->doSay(buf);

      if (!dynamic_cast<TMonster *>(ch)) {
        act("You throw $N out.", FALSE, myself, 0, ch, TO_CHAR);
        act("$n throws you out of $s shop.", FALSE, myself, 0, ch, TO_VICT);
        act("$n throws $N out of $s shop.", FALSE, myself, 0, ch, TO_NOTVICT);
        --(*ch->riding);
        thing_to_room(ch->riding, (int) o);
        --(*ch);
        thing_to_room(ch, (int) o);
      } else {
	// Just kick out the mount, not the mobile. -Lapsos
	TThing *tMount = ch->riding;

	act("You throw $N out.", FALSE, myself, 0, ch, TO_CHAR);
	act("$n throws your mount out of $s shop.", FALSE, myself, 0, ch, TO_VICT);
	act("$n throws $N out of $s shop.", FALSE, myself, 0, ch->riding, TO_NOTVICT);

	ch->dismount(POSITION_STANDING);

	--(*tMount);
	thing_to_room(tMount, (int)o);
      }

      return TRUE;
    } else if (dynamic_cast<TBeing *>(ch->rider)) {
      if (!dynamic_cast<TMonster *>(ch->rider)) {
	--(*ch->rider);
	thing_to_room(ch->rider, (int) o);
	--(*ch);
	thing_to_room(ch, (int) o);
      } else {
	// Just kick out the mount, not the mobile. -Lapsos
	ch->rider->dismount(POSITION_STANDING);

	--(*ch);
	thing_to_room(ch, (int) o);
      }

      return TRUE;
    }

    return FALSE;
  } else if (cmd == CMD_MOB_ALIGN_PULSE) {
    // called on a long period....
    // have items in shop slowly repair themselves...
    TThing *t, *t2;
    if (::number(0,10))
      return FALSE;
    for (t = myself->getStuff(); t; t = t2) {
      t2 = t->nextThing;
      TObj * obj = dynamic_cast<TObj *>(t);
      if (!obj)
        continue;

      if(IS_SET(shop_index[shop_nr].flags, SHOP_FLAG_RECYCLE) &&
	 !::number(0,24)){
	int val=(int)(obj->getValue() * shop_index[shop_nr].profit_sell);
	
	// no profit for recycling right now
	// experiment in closed loop economy
	val=0;
	//

	myself->addToMoney(val, GOLD_SHOP);
	shoplog(shop_nr, myself, myself, obj->getName(), val, "recycling");
	delete obj;

	vlogf(LOG_OBJ, fmt("shop %s (%i) recycling %s for %i talens") %  myself->getName() % shop_nr % obj->getName() % (int)(obj->getValue() * shop_index[shop_nr].profit_sell));

	continue;
      }


      if (!::number(0,99) && !shop_index[shop_nr].isProducing(obj) &&
	  !shop_index[shop_nr].isOwned()) {
        // random recycling
	vlogf(LOG_OBJ, fmt("shop %s (%i) recycling %s") %  myself->getName() % shop_nr % obj->getName());
        delete obj;
        continue;
      }
      if (::number(0,3))
        continue;
#if 0
      if (obj->getMaxStructPoints() >= 0 &&
          obj->getStructPoints() < obj->getMaxStructPoints()) {
#else
      // this keeps ithe fixing limited by depreciation
      if (obj->getMaxStructPoints() >= 0 &&
          obj->getStructPoints() < obj->maxFix(NULL, DEPRECIATION_YES) &&
	  !shop_index[shop_nr].isOwned()) {
#endif
        obj->addToStructPoints(1);
      }
    }
    return FALSE;
  } else if (cmd >= MAX_CMD_LIST)
    return FALSE;

  if ((cmd == CMD_BUY) && (ch->in_room == shop_index[shop_nr].in_room)) {
    if (!safe_to_save_shop_stuff(myself))
      return TRUE;

    shopping_buy(arg, ch, myself, shop_nr);
    return TRUE;
  }
  if ((cmd == CMD_SELL) && (ch->in_room == shop_index[shop_nr].in_room)) {
    if (!safe_to_save_shop_stuff(myself))
      return TRUE;
    rc = shopping_sell(arg, ch, myself, shop_nr);
    if (IS_SET_DELETE(rc, DELETE_THIS))
      return DELETE_VICT;  // nuke ch
    return TRUE;
  }
  if ((cmd == CMD_VALUE) && (ch->in_room == shop_index[shop_nr].in_room)) {
    shopping_value(arg, (ch), (myself), shop_nr);
    return TRUE;
  }
  if ((cmd == CMD_LIST) && (ch->in_room == shop_index[shop_nr].in_room)) {
    shopping_list(arg, (ch), (myself), shop_nr);
    return TRUE;
  }
  if ((cmd == CMD_KILL) || (cmd == CMD_HIT)) {
    char argm[MAX_INPUT_LENGTH];
    strcpy(argm, arg);

    if ((myself) == get_char_room(argm, ch->in_room)) {
      shopping_kill(arg, (ch), myself, shop_nr);
      return TRUE;
    }
    return FALSE;
  }
  if ((cmd == CMD_CAST) || (cmd == CMD_RECITE) || 
      (cmd == CMD_USE) || (cmd == CMD_PRAY)) {
    if (myself->canSee(ch)) {
      myself->doTell(ch->getNameNOC(ch), "<r>No magic here - kid!<z>");
    } else
      act("I may not be able to see you kid, but there is no magic in here.",
          FALSE, ch, 0, myself, TO_CHAR);
    return TRUE;
  }
  if (cmd == CMD_LOOK || cmd == CMD_EXAMINE) {
    return shopping_look(arg, ch, myself, shop_nr);
  }
  if (cmd == CMD_EVALUATE) {
    return shopping_evaluate(arg, ch, myself, shop_nr);
  }

#if 1
  // the sweepers should be reasonably efficient about cleaning up, so this
  // probably isn't needed.  Non-GH might still suffer though....
  // -Cept they don't Enter shops, simply prevent them from dropping like before.

  if ((cmd == CMD_DROP) && (ch->in_room == shop_index[shop_nr].in_room)) {
    TRoom * pRoom = real_roomp(ch->in_room);

// This is just a quick fix so we can check it below.
extern int grimhavenDump(TBeing *, cmdTypeT, const char *, TRoom *);

    if (!pRoom || (pRoom->funct != grimhavenDump)) {
      // possible alternative would be to move dropped stuff to ROOM_DONATION
      act("$N tells you, 'HEY!  Don't clutter up my shop'.", FALSE, ch, 0, myself, TO_CHAR);
      return TRUE;
    }
  }
#endif

  if(cmd == CMD_WHISPER){
    return shopWhisper(ch, myself, shop_nr, arg);
  }

  return FALSE;
}

void shoplog(int shop_nr, TBeing *ch, TMonster *keeper, const sstring &name, int cost, const sstring &action){
  int value=0, count=0;
  TThing *tt;
  TObj *o;  

  for(tt=keeper->getStuff();tt;tt=tt->nextThing){
    ++count;
    o=dynamic_cast<TObj *>(tt);
    value+=o->getValue();
  }
  
  TDatabase db(DB_SNEEZY);

  db.query("insert into shoplog values (%i, '%s', '%s', '%s', %i, %i, %i, now(), %i)", shop_nr, ch?ch->getName():"unknown", action.c_str(), name.c_str(), cost, keeper->getMoney(), value, count);

}

void bootTheShops()
{
  int shop_nr;

  /****** producing ******/
  TDatabase producing_db(DB_SNEEZY);
  producing_db.query("select shop_nr, producing from shopproducing order by shop_nr");
  producing_db.fetchRow();

  /****** type ******/
  TDatabase type_db(DB_SNEEZY);
  type_db.query("select shop_nr, type from shoptype order by shop_nr");
  type_db.fetchRow();

  /****** material ******/
  TDatabase material_db(DB_SNEEZY);
  material_db.query("select shop_nr, mat_type from shopmaterial order by shop_nr");
  material_db.fetchRow();

  /****** owned ******/
  TDatabase owned_db(DB_SNEEZY);
  owned_db.query("select shop_nr, profit_buy, profit_sell, no_such_item1, no_such_item2, do_not_buy, missing_cash1, missing_cash2, message_buy, message_sell from shopowned order by shop_nr");
  owned_db.fetchRow();


  /****** is owned ******/
  TDatabase isowned_db(DB_SNEEZY);
  isowned_db.query("select distinct shop_nr from shopowned");
  isowned_db.fetchRow();

  
  TDatabase db(DB_SNEEZY);

  db.query("select shop_nr, no_such_item1, no_such_item2, do_not_buy, missing_cash1, missing_cash2, message_buy, message_sell, temper1, temper2, keeper, flags, in_room, open1, close1, open2, close2, profit_buy, profit_sell from shop order by shop_nr");

  while(db.fetchRow()){
    shopData sd;

    shop_nr=convertTo<int>(db["shop_nr"]);
    sd.shop_nr=shop_nr;

    if(!owned_db["no_such_item1"].empty())
      sd.no_such_item1 = mud_str_dup(owned_db["no_such_item1"]);
    else
      sd.no_such_item1 = mud_str_dup(db["no_such_item1"]);

    if(!owned_db["no_such_item2"].empty())
      sd.no_such_item2 = mud_str_dup(owned_db["no_such_item2"]);
    else
      sd.no_such_item2 = mud_str_dup(db["no_such_item2"]);

    if(!owned_db["do_not_buy"].empty())
      sd.do_not_buy = mud_str_dup(owned_db["do_not_buy"]);
    else
      sd.do_not_buy = mud_str_dup(db["do_not_buy"]);

    if(!owned_db["missing_cash1"].empty())
      sd.missing_cash1 = mud_str_dup(owned_db["missing_cash1"]);
    else
      sd.missing_cash1 = mud_str_dup(db["missing_cash1"]);

    if(!owned_db["missing_cash2"].empty())
      sd.missing_cash2 = mud_str_dup(owned_db["missing_cash2"]);
    else
      sd.missing_cash2 = mud_str_dup(db["missing_cash2"]);

    if(!owned_db["message_buy"].empty())
      sd.message_buy = mud_str_dup(owned_db["message_buy"]);
    else
      sd.message_buy = mud_str_dup(db["message_buy"]);

    if(!owned_db["message_sell"].empty())
      sd.message_sell = mud_str_dup(owned_db["message_sell"]);
    else
      sd.message_sell = mud_str_dup(db["message_sell"]);


    sd.temper1=convertTo<int>(db["temper1"]);
    sd.temper2=convertTo<int>(db["temper2"]);
    sd.keeper=real_mobile(convertTo<int>(db["keeper"]));
    sd.flags=convertTo<int>(db["flags"]);
    sd.in_room=convertTo<int>(db["in_room"]);
    sd.open1=convertTo<int>(db["open1"]);
    sd.close1=convertTo<int>(db["close1"]);
    sd.open2=convertTo<int>(db["open2"]);
    sd.close2=convertTo<int>(db["close2"]);

    if(!owned_db["shop_nr"].empty() &&
       (convertTo<int>(owned_db["shop_nr"]))==shop_nr){
      sd.profit_buy=convertTo<float>(owned_db["profit_buy"]);
      sd.profit_sell=convertTo<float>(owned_db["profit_sell"]);
      owned_db.fetchRow();
    } else {
      sd.profit_buy=convertTo<float>(db["profit_buy"]);
      sd.profit_sell=convertTo<float>(db["profit_sell"]);
    }

    if(!isowned_db["shop_nr"].empty() &&
       (convertTo<int>(isowned_db["shop_nr"]))==shop_nr){
      sd.owned=true;
      isowned_db.fetchRow();
    } else {
      sd.owned=false;
    }

    while(!type_db["shop_nr"].empty() &&
	  convertTo<int>(type_db["shop_nr"])==shop_nr){
      sd.type.push_back(convertTo<int>(type_db["type"]));
      type_db.fetchRow();
    }
    sd.type.push_back(MAX_OBJ_TYPES);
    
    while(!producing_db["shop_nr"].empty() &&
	  convertTo<int>(producing_db["shop_nr"])==shop_nr){
      sd.producing.push_back(real_object(convertTo<int>(producing_db["producing"])));
      producing_db.fetchRow();
    }
    sd.producing.push_back(-1);
    
    while(!material_db["shop_nr"].empty() &&
	  convertTo<int>(material_db["shop_nr"])==shop_nr){
      sd.mat_type.push_back(convertTo<int>(material_db["mat_type"]));
      material_db.fetchRow();
    }
    sd.mat_type.push_back(MAX_OBJ_TYPES);

    shop_index.push_back(sd);
  }  
}


bool safe_to_save_shop_stuff(TMonster *ch)
{

  if (mob_index[ch->getMobIndex()].getNumber() < 1) {
     vlogf(LOG_BUG, fmt("Shopkeeper #%d got safe_to_save_shop_stuff called when none in world!") % 
            mob_index[ch->getMobIndex()].virt);
    ch->doSay("I'm not functioning properly.  Tell a god to check the logs, case 1.");
    return FALSE;
  }
  if (mob_index[ch->getMobIndex()].getNumber() > 1) {
    vlogf(LOG_BUG, fmt("More than one shopkeeper #%d in world.  Now the shop won't work!") % 
          mob_index[ch->getMobIndex()].virt);
    ch->doSay("I'm not functioning properly.  Tell a god to check the logs, case 2.");
    return FALSE;
  }
  return TRUE;
}


void processShopFile(const char *cFname)
{
  char fileName[128];
  FILE *fp;
  unsigned char ucVersion;

  if (!cFname) {
    vlogf(LOG_BUG, "  processShopFile called with NULL filename!");
    return;
  }
  sprintf(fileName, "%s/%s", SHOPFILE_PATH, cFname);
  if (!(fp = fopen(fileName, "r"))) {
    vlogf(LOG_BUG, fmt("  Error opening the shop file for shop #%s") %  cFname);
    return;
  }
  if (fread(&ucVersion, sizeof(ucVersion), 1, fp) != 1) {
    vlogf(LOG_BUG, fmt("Error reading version from %s.") %  fileName);
    fclose(fp);
    return;
  }

  if (!noteLimitedItems(fp, fileName, ucVersion, FALSE))
    vlogf(LOG_BUG, fmt("  Unable to count limited items in file  %s") %  fileName);
  fclose(fp);
}


void processShopFiles(void)
{
   dirwalk(SHOPFILE_PATH, processShopFile);
}


// adjusts the shop price based on structure
int TObj::adjPrice() const
{
  int value = ((getMaxStructPoints() <= 0) ? getValue() :
               (int) (getValue() * 
                getStructPoints() / getMaxStructPoints()));

  return value;
}



shopData::shopData() :
  shop_nr(0),
  owned(false),
  profit_buy(1.0),
  profit_sell(1.0),
  no_such_item1(NULL),
  no_such_item2(NULL),
  missing_cash1(NULL),
  missing_cash2(NULL),
  do_not_buy(NULL),
  message_buy(NULL),
  message_sell(NULL),
  temper1(0),
  temper2(0),
  keeper(0),
  flags(0),
  in_room(0),
  open1(0),
  open2(0),
  close1(0),
  close2(0)
{
}

shopData::~shopData()
{
  delete [] no_such_item1;
  delete [] no_such_item2;
  delete [] missing_cash1;
  delete [] missing_cash2;
  delete [] do_not_buy;
  delete [] message_buy;
  delete [] message_sell;

  producing.erase(producing.begin(), producing.end());
  type.erase(type.begin(), type.end());
  mat_type.erase(mat_type.begin(), mat_type.end());
}

shopData::shopData(const shopData &t) :
  shop_nr(t.shop_nr),
  owned(t.owned),
  profit_buy(t.profit_buy),
  profit_sell(t.profit_sell),
  temper1(t.temper1),
  temper2(t.temper2),
  keeper(t.keeper),
  flags(t.flags),
  in_room(t.in_room),
  open1(t.open1),
  open2(t.open2),
  close1(t.close1),
  close2(t.close2)
{
  unsigned int i;
  unsigned int max_prod=t.producing.size();
  producing.erase(producing.begin(), producing.end());
  for(i=0;i<max_prod;++i)
    producing.push_back(t.producing[i]);

  unsigned int max_type=t.type.size();
  type.erase(type.begin(), type.end());
  for(i=0;i<max_type;++i)
    type.push_back(t.type[i]);

  unsigned int max_mat_type=t.mat_type.size();
  mat_type.erase(mat_type.begin(), mat_type.end());
  for(i=0;i<max_mat_type;++i)
    mat_type.push_back(t.mat_type[i]);

  no_such_item1 = mud_str_dup(t.no_such_item1);
  no_such_item2 = mud_str_dup(t.no_such_item2);
  missing_cash1 = mud_str_dup(t.missing_cash1);
  missing_cash2 = mud_str_dup(t.missing_cash2);
  do_not_buy = mud_str_dup(t.do_not_buy);
  message_buy = mud_str_dup(t.message_buy);
  message_sell = mud_str_dup(t.message_sell);

}

shopData & shopData::operator =(const shopData &t)
{
  unsigned int i;

  if (this == &t) return *this;

  unsigned int max_prod=t.producing.size();
  producing.erase(producing.begin(), producing.end());
  for(i=0;i<max_prod;++i)
    producing.push_back(t.producing[i]);

  profit_buy = t.profit_buy;
  profit_sell = t.profit_sell;

  unsigned int max_type=t.type.size();
  type.erase(type.begin(), type.end());
  for(i=0;i<max_type;++i)
    type.push_back(t.type[i]);

  unsigned int max_mat_type=t.mat_type.size();
  mat_type.erase(mat_type.begin(), mat_type.end());
  for(i=0;i<max_mat_type;++i)
    mat_type.push_back(t.mat_type[i]);

  no_such_item1 = mud_str_dup(t.no_such_item1);
  no_such_item2 = mud_str_dup(t.no_such_item2);
  missing_cash1 = mud_str_dup(t.missing_cash1);
  missing_cash2 = mud_str_dup(t.missing_cash2);
  do_not_buy = mud_str_dup(t.do_not_buy);
  message_buy = mud_str_dup(t.message_buy);
  message_sell = mud_str_dup(t.message_sell);

  owned = t.owned;
  temper1 = t.temper1;
  temper2 = t.temper2;
  keeper = t.keeper;
  flags = t.flags;
  in_room = t.in_room;
  open1 = t.open1;
  open2 = t.open2;
  close1 = t.close1;
  close2 = t.close2;
  shop_nr = t.shop_nr;

  return *this;
}







