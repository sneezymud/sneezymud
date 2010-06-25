#include <cmath>
#include <unistd.h>
#include <algorithm>

#include "handler.h"
#include "room.h"
#include "monster.h"
#include "extern.h"
#include "configuration.h"
#include "shop.h"
#include "materials.h"
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
#include "spec_rooms.h"
#include "obj_commodity.h"
#include "liquids.h"
#include "shopaccounting.h"
#include "spec_mobs.h"
#include "weather.h"

extern int kick_mobs_from_shop(TMonster *myself, TBeing *ch, int from_room);

std::vector<shopData>shop_index(0);

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
    vlogf(LOG_BUG, format("Warning... shop # for mobile %d (real nr) not found.") % mob_index[mobvnum].virt);
    return 0;
  }   

  return shop_nr;
}

float shopData::getProfitSell(const TObj *obj, const TBeing *ch)
{
  std::map <sstring,float>::iterator iter;
  float profit=profit_sell;

  // if the shop is player owned, we check custom pricing
  if(!isOwned() || !ensureCache())
    return profit;

  if(obj)
  {
    int vnum = obj->objVnum();

    if(sell_ratios_cache.count(vnum))
    {
      profit=sell_ratios_cache[vnum];
    }
    else
    {
      // ok, shop is owned and there is no ratio set for this specific object
      // so check keywords
      for(iter=sell_matches_cache.begin();iter!=sell_matches_cache.end();++iter)
      {
        if(isname((*iter).first, obj->name))
        {
          profit=(*iter).second;
          break;
        }
      }
    }
  }

  // check pricing for player
  for(iter=sell_player_cache.begin();ch && iter!=sell_player_cache.end();++iter)
  {
    if((*iter).first == (sstring) ch->name)
    {
      profit = ((*iter).second);
      break;
    }
  }

  return profit;
}

bool shopData::isRepairShop()
{
  if(shop_nr >= 127 and shop_nr <= 134)
    return true;

  return false;
}

void shopData::clearCache()
{
  buy_ratios_cache.clear();
  buy_matches_cache.clear();
  buy_player_cache.clear();
  sell_ratios_cache.clear();
  sell_matches_cache.clear();
  sell_player_cache.clear();
  max_ratios_cache.clear();
  max_matches_cache.clear();
  max_player_cache.clear();
  isCached = false;
}

bool shopData::ensureCache()
{
  if (isCached)
    return true;

  TDatabase db(DB_SNEEZY);
  db.query("select obj_nr, profit_buy, profit_sell, max_num from shopownedratios where shop_nr=%i", shop_nr);
  while(db.fetchRow())
  {
    buy_ratios_cache[convertTo<int>(db["obj_nr"])]=convertTo<float>(db["profit_buy"]);
    sell_ratios_cache[convertTo<int>(db["obj_nr"])]=convertTo<float>(db["profit_sell"]);
    max_ratios_cache[convertTo<int>(db["obj_nr"])]=convertTo<int>(db["max_num"]);
  }

  db.query("select match_str, profit_buy, profit_sell, max_num from shopownedmatch where shop_nr=%i", shop_nr);
  while(db.fetchRow())
  {
    buy_matches_cache[db["match_str"]]=convertTo<float>(db["profit_buy"]);
    sell_matches_cache[db["match_str"]]=convertTo<float>(db["profit_sell"]);
    max_matches_cache[db["match_str"]]=convertTo<int>(db["max_num"]);
  }

  db.query("select player, profit_buy, profit_sell, max_num from shopownedplayer where shop_nr=%i", shop_nr);
  while(db.fetchRow())
  {
    buy_player_cache[db["player"]]=convertTo<float>(db["profit_buy"]);
    sell_player_cache[db["player"]]=convertTo<float>(db["profit_sell"]);
    max_player_cache[db["player"]]=convertTo<int>(db["max_num"]);
  }

  // clear all regular shopowned values to defaults (set only if query returns)
  max_num = -1;
  corp_id = 0;
  dividend = 0;
  tax_nr = -1;
  repair_speed = -1;
  repair_quality = -1;
  reserve_min = 0;
  reserve_max = 0;

  db.query("select max_num, corp_id, dividend, tax_nr, reserve_min, reserve_max from shopowned where shop_nr=%i", shop_nr);
  if(db.fetchRow())
  {
    max_num = convertTo<int>(db["max_num"]);
    corp_id = convertTo<int>(db["corp_id"]);
    dividend = convertTo<double>(db["dividend"]);
    tax_nr = convertTo<int>(db["tax_nr"]);
    reserve_min = convertTo<int>(db["reserve_min"]);
    reserve_max = convertTo<int>(db["reserve_max"]);
  }

  // inventory
  inventory_count = 0;

  db.query("select count(*) as count from rent where owner_type='shop' and owner=%i", shop_nr);
  if (db.fetchRow())
    inventory_count =convertTo<int>(db["count"]);

  // repair data
  if (isRepairShop())
  {
    db.query("select speed, quality from shopownedrepair where shop_nr=%i", shop_nr);
    if(db.fetchRow())
    {
      repair_speed=convertTo<float>(db["speed"]);
      repair_quality=convertTo<float>(db["quality"]);
    }
  }

  // initialize bank data
  hasCentralBank = false;
  centralbank = 4;
  bank_reserve_min = -1;

  db.query("select centralbank from shopownedcentralbank where bank=%i", shop_nr);
  if (db.fetchRow())
  {
    hasCentralBank = true;
    centralbank = convertTo<int>(db["centralbank"]);
    float reserve = shop_index[centralbank].getProfitBuy(NULL, NULL);

    // so we want the total of deposits * the reserver
    db.query("select ((sb.t+sbc.t)*%f) as t from (select count(*) as c, sum(talens) as t from shopownedbank where shop_nr=%i) sb, (select count(*) as c, sum(talens) as t from shopownedcorpbank where shop_nr=%i) sbc", reserve, shop_nr, shop_nr);
    if (db.fetchRow())
      bank_reserve_min = convertTo<int>(db["t"]);
  }

  // clear all regular shop values to defaults (expected to return)
  expense_ratio = 0;

  db.query("select expense_ratio from shop where shop_nr=%i", shop_nr);
  if(db.fetchRow())
  {
    expense_ratio = convertTo<double>(db["expense_ratio"]);
  }

  isCached = true;
  return true;
}


int shopData::getMaxNum(const TBeing* ch, const TObj* o, int defaultMax)
{
  std::map <sstring,int>::iterator iter;

  if (!isOwned() || !ensureCache())
    return defaultMax;

  for(iter=max_matches_cache.begin();o && iter!=max_matches_cache.end();++iter)
    if(isname((*iter).first, o->name))
      return (*iter).second;

  if(o && buy_ratios_cache.count(o->objVnum()))
    return max_ratios_cache[o->objVnum()];

  for(iter=max_player_cache.begin();ch && iter!=max_player_cache.end();++iter)
    if((*iter).first == (sstring) ch->name)
      return (*iter).second;

  return max_num >= 0 ? max_num : defaultMax;
}


float shopData::getProfitBuy(int vnum, sstring name, const TBeing *ch)
{
  float profit=-1;
  std::map <sstring,float>::iterator iter;

  if (!ensureCache())
    return profit_buy;

  // if the shop is player owned, we check custom pricing
  if(isOwned())
  {  
    if(buy_ratios_cache.count(vnum))
      profit=buy_ratios_cache[vnum];

    if(profit==-1)
    {
      // ok, shop is owned and there is no ratio set for this specific object
      // so check keywords
      for(iter=buy_matches_cache.begin();iter!=buy_matches_cache.end();++iter)
      {
        if(isname((*iter).first, name))
        {
          profit=(*iter).second;
          break;
        }
      }
    }
  }

  // no custom price found, so use the normal shop pricing
  if(profit == -1)
    profit=profit_buy;

  // check for player specific modifiers
  if(isOwned() && ch)
  {
    for(iter=buy_player_cache.begin();iter!=buy_player_cache.end();++iter)
    {
      if((*iter).first == (sstring) ch->name)
      {
        profit = ((*iter).second);
        break;
      }
    }
  }

  // check for speed and quality for repair shops
  if(isRepairShop())
  {
    if(repair_speed>0)
      profit /= repair_speed;
    if(repair_quality>0)
      profit *= repair_quality;
  }

  return profit;
}

float shopData::getProfitBuy(const TObj *obj, const TBeing *ch)
{
  return getProfitBuy(obj?obj->objVnum():-1, obj?obj->name:"", ch);
}


int shopData::getMinReserve()
{
  if (!ensureCache())
    return 0;

  if (bank_reserve_min > -1)
    return bank_reserve_min;

  return reserve_min;
}

int shopData::getMaxReserve()
{
  if (!ensureCache())
    return 0;

  if (hasCentralBank)
    return reserve_max + getMinReserve();
  return reserve_max;
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

  // make sure we don't have a negative cost
  cost = max(1.0, cost);

  // cast this back to an int so that we can multiple without inflating the price
  int singleCost = (int) cost;

  // finally do the multiplication for number of items
  // we do this last so that the actual price is the same as the single-object quoted price * num 
  return singleCost * num;
}

bool shopData::willTradeWith(TMonster *keeper, TBeing *ch)
{
  int hmt = GameTime::hourminTime();

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
  bool is_commod = dynamic_cast<const TCommodity*>(item) != NULL;

  if ((!is_commod && item->getValue() < 1) || 
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

  if (item->number < 0)
    return FALSE;

  max_prod=producing.size();

  for (counter = 0; counter < max_prod; counter++) {
    if (producing[counter] <= -1)
      continue;

    if (producing[counter] == item->number) {
      if(obj_index[producing[counter]].name && item->name && 
	 !strcmp(obj_index[producing[counter]].name, item->name)){
        return TRUE;
      }
    }
  }
  return FALSE;
}


static int number_objects_in_list(const TObj *item, const StuffList list)
{
  const TObj *i = NULL;
  int count = 0;

  for(StuffIter it=list.begin();it!=list.end();++it){
    if(!(i = dynamic_cast<const TObj *>(*it)))
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
  char newarg[MAX_INPUT_LENGTH];
  sstring argm;
  int num = 1, rent_id;
  TObj *temp1 = NULL;
  TDatabase db(DB_SNEEZY);

  if (!(shop_index[shop_nr].willTradeWith(keeper, ch)))
    return;

  argm = sstring(arg).trim();
  if (argm.empty()) {
    keeper->doTell(ch->name, "What do you want to buy??");
    return;
  }
  if ((num = getabunch(argm.c_str(), newarg)))
    argm = newarg;

  if (!num)
    num = 1;

  std::vector<int>objects;
  std::vector<TObj *>objects_p;

  if(!(rent_id=convertTo<int>(argm))){
    db.query("select r.rent_id as rent_id, coalesce(rs.name, o.name) as name\
                from rent r left outer join rent_strung rs \
                on (r.rent_id=rs.rent_id), obj o \
              where r.vnum=o.vnum and r.owner_type='shop' and r.owner=%i",
             shop_nr);

    for(int i=0;i<num && db.fetchRow();){
      if(!isname(argm, db["name"]))
        continue;
      
      rent_id=convertTo<int>(db["rent_id"]);
      temp1=keeper->loadItem(shop_nr, rent_id);
      *keeper += *temp1;
      objects.push_back(rent_id);
      objects_p.push_back(temp1);
      ++i;
    }
  } else {
    db.query("select r1.rent_id from rent r1 left outer join rent_strung rs1 on (r1.rent_id=rs1.rent_id), obj o1, rent r2 left outer join rent_strung rs2 on (r2.rent_id=rs2.rent_id), obj o2 where r1.vnum=r2.vnum and coalesce(rs1.short_desc, o1.short_desc)=coalesce(rs2.short_desc, o2.short_desc) and r1.owner_type='shop' and r1.owner=%i and o1.vnum=r1.vnum and r2.vnum=o2.vnum and r2.rent_id=%i", shop_nr, rent_id);

    for(int i=0;i<num && db.fetchRow();++i){
      rent_id=convertTo<int>(db["rent_id"]);
      temp1=keeper->loadItem(shop_nr, rent_id);
      *keeper += *temp1;
      objects.push_back(rent_id);
      objects_p.push_back(temp1);
    }
  }

  if(!temp1){
    keeper->doTell(ch->name, shop_index[shop_nr].no_such_item1);
    return;
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

  // sell/purchase the object, if that fails destroy all in-memory stuff
  if (temp1->buyMe(ch, keeper, num, shop_nr) == -1) {
    for(unsigned int i=0;i<objects_p.size();++i)
      delete objects_p[i];
    return;
  }

  // delete objects left on keeper, remove objects sold from db
  for(unsigned int i=0;i<objects_p.size();++i) {
    TObj *cleanupObj = objects_p[i];
    if (!cleanupObj)
      continue;
    if (cleanupObj->parent == keeper)
      delete objects_p[i];
    else
      keeper->deleteItem(shop_nr, objects[i]);
  }
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
  float swindle;

  if (!ch->isImmortal())
  {
    if ((ch->getCarriedVolume() + (num * getTotalVolume())) > ch->carryVolumeLimit()) {
      ch->sendTo(format("%s: You can't carry that much volume.\n\r") % fname(name));
      return -1;
    }
    // obj-weight > free ch limit
    if (compareWeights(getTotalWeight(TRUE),
         ((ch->carryWeightLimit() - ch->getCarriedWeight())/num)) == -1) {
      ch->sendTo(format("%s: You can't carry that much weight.\n\r") % fname(name));
      return -1;
    }
  }
  
  tmp = number_objects_in_list(this, keeper->stuff);
  if (num > tmp) {
    keeper->doTell(ch->name, format("I don't have %d of that item. Here %s the %d I do have.") %
		   num  % ((tmp > 1) ? "are" : "is") % tmp);
  } else
    tmp = num;
  
  strcpy(argm, name);
  
  strcpy(argm, add_bars(argm).c_str());
  swindle=ch->getSwindleBonus();
  chr = ch->getChaShopPenalty() - swindle;
  chr = max((float)1.0,chr);

  cost = shopPrice(1, shop_nr, chr, ch);
  
  for (i = 0; i < tmp; i++) {
    TThing *t_temp1 = searchLinkedList(argm, keeper->stuff);
    TObj *temp1 = dynamic_cast<TObj *>(t_temp1);
      
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

    if (!temp1) break;
    --(*temp1);
    *ch += *temp1;

    temp1->purchaseMe(ch, keeper, cost, shop_nr);

    ch->logItem(temp1, CMD_BUY);
    count++;
  }

  keeper->saveItems(shop_nr);

  if (!count) {
    keeper->doTell(ch->name, "I can't seem to find any of those!");
    return -1;
  }

  //  ch->sendTo(format("You manage to swindle the shopkeeper into a %i%s discount.\n\r") % (int)(swindle*100) % "%");
  keeper->doTell(ch->name, format(shop_index[shop_nr].message_buy) %
		 (cost * count));

  ch->sendTo(COLOR_OBJECTS, format("You now have %s (*%d).\n\r") % 
	     sstring(getName()).uncap() % count);
  if (count == 1) 
    act("$n buys $p.", FALSE, ch, this, NULL, TO_ROOM); 
  else {
    buf = format("$n buys %s [%d].") % fname(name) % count;
    act(buf, FALSE, ch, this, 0, TO_ROOM);
  }
  ch->doQueueSave();
  return cost;
}


bool will_not_buy(TBeing *ch, TMonster *keeper, TObj *temp1, int shop_nr)
{
  sstring buf;

  if(temp1->objectSell(ch, keeper)){
    if(ch->isImmortal())
      keeper->doTell(ch->getName(), "Since you're immortal, I'll make an exception.");
    else 
      return TRUE;
  }
  if(Config::NoDamagedItemsShop()){
    if (temp1->getStructPoints() != temp1->getMaxStructPoints()) {
      keeper->doTell(ch->getName(), "I don't buy damaged goods.");
      return TRUE;
    }
  }

  if (!temp1->stuff.empty()) {
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
  if(temp1->isObjStat(ITEM_RUSTY)){
    keeper->doTell(ch->getName(), "I'm sorry, I don't buy rusty goods.");
    return TRUE;
  }

  if (shop_index[shop_nr].isOwned() && temp1->isObjStat(ITEM_NORENT)){
    keeper->doTell(ch->getName(), "This shop is privately owned and we don't purchase non-rentable items.");
    return TRUE;
  }

  if (dynamic_cast<TCommodity *>(temp1))
    return FALSE;

  if(temp1->sellPrice(1, shop_nr, -1, ch) < 0){
    keeper->doTell(ch->getName(), "You'd have to pay me to buy that!");
    return TRUE;
  }

  if(shop_index[shop_nr].getInventoryCount() >= (int)MAX_SHOP_INVENTORY){
    keeper->doTell(ch->getName(), "My inventory is full, I can't buy anything!");
    return TRUE;
  }

  return FALSE;
}


bool TObj::sellMeCheck(TBeing *ch, TMonster *keeper, int, int defaultMax) const
{
  int total = 0;
  sstring buf;
  unsigned int shop_nr;

  for (shop_nr = 0; (shop_nr < shop_index.size()) && (shop_index[shop_nr].keeper != (keeper)->number); shop_nr++);

  if (shop_nr >= shop_index.size()) {
    vlogf(LOG_BUG, format("Warning... shop # for mobile %d (real nr) not found.") %  mob_index[keeper->number].virt);
    return FALSE;
  }
  
  TShopOwned tso(shop_nr, keeper, ch);
  int max_num=tso.getMaxNum(ch, this, defaultMax);

  if(max_num == 0){
    keeper->doTell(ch->name, "I don't wish to buy any of those right now.");
    return TRUE;
  }

  total=tso.getInventoryCount(this);

  if (total >= max_num && !shop_index[shop_nr].isProducing(this)) {
    keeper->doTell(ch->name, "I already have plenty of those.");
    return TRUE;
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
  
  int rc;
  if (tComp) {
    rc=tComp->sellMe(ch, keeper, shop_nr, num);
    if(IS_SET_DELETE(rc, DELETE_THIS))
      delete tComp;
  } else {
    rc=obj->sellMe(ch, keeper, shop_nr, 1);
    if(IS_SET_DELETE(rc, DELETE_THIS))
      delete obj;
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
  
  int rc;
  if (tComp) {
    rc=tComp->sellMe(ch, keeper, shop_nr, 1);
    if(IS_SET_DELETE(rc, DELETE_THIS))
      delete obj;
  } else {
    rc=obj->sellMe(ch, keeper, shop_nr, 1);
    if(IS_SET_DELETE(rc, DELETE_THIS))
      delete obj;
  }
  // obj may be invalid here
}


int TObj::sellMe(TBeing *ch, TMonster *keeper, int shop_nr, int num = 1)
{
  int cost;
  sstring buf;
  float chr;

  if (!shop_index[shop_nr].profit_sell) {
    keeper->doTell(ch->getName(), shop_index[shop_nr].do_not_buy);
    return false;
  }
  
  
  if (getValue() <= 1 || isObjStat(ITEM_NEWBIE)) {
    keeper->doTell(ch->getName(), "I'm sorry, I don't buy valueless items.");
    return false;
  }
  if (sellMeCheck(ch, keeper, num, 9))
    return false;
  
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
    if(Config::NoDamagedItemsShop()){
      keeper->doTell(fname(ch->name), "It's been damaged, but I guess I can buy it as scrap.");
    }
  }
  max(cost, 1);   // at least 1 talen 
  if (keeper->getMoney() < cost) {
    keeper->doTell(ch->getName(), shop_index[shop_nr].missing_cash1);
    return false;
  }
  if (obj_index[getItemIndex()].max_exist <= 10) {
    keeper->doTell(ch->name, "Wow!  This is one of those limited items.");
    keeper->doTell(ch->name, "You should really think about auctioning it.");
  }
  act("$n sells $p.", FALSE, ch, this, 0, TO_ROOM);

  keeper->doTell(ch->getName(), format(shop_index[shop_nr].message_sell)% cost);

  ch->sendTo(COLOR_OBJECTS, format("The shopkeeper now has %s.\n\r") % sstring(getName()).uncap());
  ch->logItem(this, CMD_SELL);

  --(*this);
  keeper->saveItem(shop_nr, this);

  sellMeMoney(ch, keeper, cost, shop_nr);

  if (ch->isAffected(AFF_GROUP) && ch->desc &&
           IS_SET(ch->desc->autobits, AUTO_SPLIT) && 
          (ch->master || ch->followers)){
    buf = format("%d") % cost;
    ch->doSplit(buf.c_str(), false);
  }

  ch->doQueueSave();
  return DELETE_THIS;
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
  TThing *t;
  int rc;

  if (isClosed()) 
    return TRUE;
  
  for(StuffIter it=stuff.begin();it!=stuff.end();){
    t=*(it++);
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
  TThing *t;
  int rc;

  if (isClosed()) {
    // ignore closed spellbags
    return TRUE;
  }
  for(StuffIter it=stuff.begin();it!=stuff.end();){
    t=*(it++);
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
  TThing *t;
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

  if (0 && gamePort != Config::Port::PROD) {
    sstring         tStString("");
    itemTypeT      tItemType;
    tObjectManipT  tObjectManip;
    int            tCount = 0;
    wearSlotT      tWear;
    TThing        *tThing;
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

    for(StuffIter it=ch->stuff.begin();it!=ch->stuff.end();){
      tThing=*(it++);

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
        if (!(t = ch->equipment[i]))
          continue;
        if (!ch->sameRoom(*tKeeper))
          break;
        // check for sleep pouches
        if (!ch->awake())
          break;

        rc = t->sellCommod(ch, tKeeper, shop_nr, NULL);
        if (IS_SET_DELETE(rc, DELETE_THIS)) {
          delete t;
          t = NULL;
        }
        if (IS_SET_DELETE(rc, DELETE_VICT)) {
          return DELETE_THIS;
        }
      }
      for(StuffIter it=ch->stuff.begin();it!=ch->stuff.end();){
        t=*(it++);
        if (!ch->sameRoom(*tKeeper))
          break;
        // check for sleep pouches
        if (!ch->awake())
          break;
        rc = t->sellCommod(ch, tKeeper, shop_nr, NULL);
        if (IS_SET_DELETE(rc, DELETE_THIS)) {
          delete t;
          t = NULL;
        }
        if (IS_SET_DELETE(rc, DELETE_VICT)) {
          return DELETE_THIS;
        }
      }
      ch->doQueueSave();
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
      for(StuffIter it=ch->stuff.begin();it!=ch->stuff.end();){
        t=*(it++);
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
      ch->doQueueSave();
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
      for(StuffIter it=ch->stuff.begin();it!=ch->stuff.end();){
        t=*(it++);
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
  TThing *t_temp1 = searchLinkedListVis(ch, argm, ch->stuff);
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
    TThing *t;
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
    for(StuffIter it=ch->stuff.begin();it!=ch->stuff.end();){
      t=*(it++);

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

  TThing *t_temp1 = searchLinkedListVis(ch, argm, ch->stuff);
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
  
  willbuy=!sellMeCheck(ch, keeper, num, 9);

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
    if(Config::NoDamagedItemsShop()){
      keeper->doTell(fname(ch->name), "It's been damaged, but I guess I can buy it as scrap.");
    }

  }
  max(cost, 1);  // at least 1 talen
  if(willbuy){
    buf = format("I'll give you %d talens for %s!") % cost % getName();
  } else {
    buf = format("Normally, I'd give you %d talens for %s!") % cost % getName();
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
    sprintf(buf4, "[%d]", tComp->getComponentCharges());
  } else {
    sprintf(buf4, "[%s]", atbuf);
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
       ((FitT & (1 <<  5)) && canWear(ITEM_WEAR_FINGERS)) ||
       ((FitT & (1 <<  6)) && canWear(ITEM_WEAR_WRISTS)) ||
       ((FitT & (1 <<  7)) && canWear(ITEM_WEAR_LEGS)) ||
       ((FitT & (1 <<  8)) && canWear(ITEM_WEAR_ARMS)) ||
       ((FitT & (1 <<  9)) && canWear(ITEM_WEAR_NECK)) ||
       ((FitT & (1 << 10)) && canWear(ITEM_WEAR_FEET)) ||
       ((FitT & (1 << 11)) && canWear(ITEM_WEAR_HANDS)) ||
       ((FitT & (1 << 12)) && canWear(ITEM_WEAR_HEAD)) ||
       ((FitT & (1 << 13)) && canWear(ITEM_WEAR_BACK)) ||
       ((FitT & (1 << 14)) && canWear(ITEM_WEAR_WAIST)) ||
       ((FitT & (1 << 17)) && isPaired()) ||
       ((FitT - (FitT & ((1 << 0) | (1 << 15) | (1 << 16)))) == 0)))
    return buf;
  else
    return "";
}


sstring equip_cond(int cur_str, int max_str)
{
  double p = ((double) cur_str) / ((double) max_str);

  if(p > 1.0){
    // shouldn't happen theoretically
    sstring a("<W>better than new<1>");
    return a;
  } else if (p == 1) {
    sstring a("<C>brand new<1>");
    return a;
  } else if (p > .9) {
    sstring a("<c>like new<1>");
    return a;
  } else if (p > .8) {
    sstring a("<B>excellent<1>");
    return a;
  } else if (p > .7) {
    sstring a("<b>very good<1>");
    return a;
  } else if (p > .6) {
    sstring a("<P>good<1>");
    return a;
  } else if (p > .5) {
    sstring a("<p>fine<1>");
    return a;
  } else if (p > .4) {
    sstring a("<G>fair<1>");
    return a;
  } else if (p > .3) {
    sstring a("<g>poor<1>");
    return a;
  } else if (p > .2) {
    sstring a("<y>very poor<1>");
    return a;
  } else if (p > .1) {
    sstring a("<o>bad<1>");
    return a;
  } else if (p > .001) {
    sstring a("<R>very bad<1>");
    return a;
  } else {
    sstring a("<r>destroyed<1>");
    return a;
  }
}

// truncate string to len without counting color codes,
// and with a terminating <1>, and pad with blanks
// eg, if len is "7", then "<b>a shirt<1>" is acceptable
sstring list_string(sstring buf, int len)
{
  sstring obuf="";

  for(unsigned int i=0;i<buf.length() && len;++i){
    if(buf[i]=='<'){
      obuf+=buf[i++];
      obuf+=buf[i++];
      obuf+=buf[i];
      continue;
    }
    obuf+=buf[i];    
    --len;
  }

  while(len>0){
    obuf+=" ";
    --len;
  }

  obuf += "<1>";
  
  return obuf;
}

void shopping_list(sstring argument, TBeing *ch, TMonster *keeper, int shop_nr)
{
  TDatabase db(DB_SNEEZY);
  sstring buf, keyword="", short_desc;
  float price, perc;
  bool fit=true;
  int extra_flags, volume, type;
  wearSlotT slot;
  unsigned long int FitT = 0;
  bool isPierce=false, isBlunt=false, isSlash=false;

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
    else if (is_abbrev(argument.word(i), "stab")) {
      if ((ch->doesKnowSkill(SKILL_STABBING)) || ch->isImmortal()) {
        FitT |= (1 << 15);
        FitT |= (1 <<  2);
      }
    } else if (is_abbrev(argument.word(i), "cudgel")) {
      if ((ch->doesKnowSkill(SKILL_CUDGEL)) || ch->isImmortal()) {
        FitT |= (1 << 16);
        FitT |= (1 <<  3);
      }
    } else if (is_abbrev(argument.word(i), "backstab")) {
      if ((ch->doesKnowSkill(SKILL_BACKSTAB)) || ch->isImmortal()) {
        FitT |= (1 << 17);
        FitT |= (1 <<  2);
      }
    } else if (is_abbrev(argument.word(i), "slit")) {
      if ((ch->doesKnowSkill(SKILL_THROATSLIT)) || ch->isImmortal()) {
        FitT |= (1 << 17);
        FitT |= (1 <<  2);
      }
    } else if (is_number(argument.word(i))){
      buf=format("and r.rent_id=%s") % argument;
    } else if (!argument.word(i).empty()) {
      keyword=argument.word(i);
    }

    if (argument.empty())
      break;
  }

  db.query("select r.rent_id as rent_id, \
                case o.type when %i then r.weight*10 \
                            when %i then r.val0 \
                            else count(*) end as count, \
                case o.type when %i then r.material \
                  else coalesce(rs.short_desc, o.short_desc) end \
                  as short_desc, \
                coalesce(rs.name, o.name) as name, \
                case o.type when %i then r.price/(r.weight*10) \
                            when %i then r.price/r.val0  \
                            else r.price end as price, \
                r.cur_struct as cur_struct, r.max_struct as max_struct, \
                r.volume as volume, r.extra_flags as extra_flags, \
                o.wear_flag as wear_flag, o.vnum as vnum, o.type as type, \
                r.val0 as val0, r.val1 as val1, r.val2 as val2, r.val3 as val3\
              from rent r left outer join rent_strung rs on \
                (rs.rent_id=r.rent_id), obj o \
              where o.vnum=r.vnum and owner_type='shop' and owner=%i \
                %s \
              group by o.vnum, short_desc \
              order by o.vnum",
	   ITEM_RAW_MATERIAL, ITEM_COMPONENT,
	   ITEM_RAW_MATERIAL,
	   ITEM_RAW_MATERIAL, ITEM_COMPONENT,
	   shop_nr,
	   buf.c_str());

  keeper->doTell(ch->getName(), "You can buy:");

  buf="Item #     Item Name                                Info       Number     Price\n\r";
  buf+="-------------------------------------------------------------------------------\n\r";
  while(db.fetchRow()){
    if(!keyword.empty() && !isname(keyword, db["name"]))
      continue;

    type=convertTo<int>(db["type"]);
    short_desc=db["short_desc"];

    // base price
    price=convertTo<float>(db["price"]);

    if(type==ITEM_RAW_MATERIAL){
      int mat = convertTo<int>(db["short_desc"]);
      price=TCommodity::demandCurvePrice(1, 0, mat, convertTo<int>(db["count"]));
      short_desc=format("COMMODITY: %s") % material_nums[mat].mat_name;
    }

    // modify price for structure damage
    price *= ((convertTo<float>(db["max_struct"]) <= 0) ? 1 :
	      (convertTo<float>(db["cur_struct"]) /
	       convertTo<float>(db["max_struct"])));

    if(type==ITEM_POTION)
      price=liquidInfo[(liqTypeT)convertTo<int>(db["val2"])]->price * convertTo<int>(db["val1"]);

    // modify price for the shop profit ratio
    price *= shop_index[shop_nr].getProfitBuy(convertTo<int>(db["vnum"]),
					      db["name"], ch);

    // modify price for charisma bonus/penalty
    price *= max((float)1.0, ch->getChaShopPenalty());

    // check class restriction
    extra_flags = convertTo<int>(db["extra_flags"]);
      
    fit=true;
    if(ch->hasClass(CLASS_MAGE) && (extra_flags & ITEM_ANTI_MAGE))
      fit=false;
    if(ch->hasClass(CLASS_CLERIC) && (extra_flags & ITEM_ANTI_CLERIC))
      fit=false;
    if(ch->hasClass(CLASS_WARRIOR) && (extra_flags & ITEM_ANTI_WARRIOR))
      fit=false;
    if(ch->hasClass(CLASS_THIEF) && (extra_flags & ITEM_ANTI_THIEF))
      fit=false;
    if(ch->hasClass(CLASS_SHAMAN) && (extra_flags & ITEM_ANTI_SHAMAN))
      fit=false;
    if(ch->hasClass(CLASS_DEIKHAN) && (extra_flags & ITEM_ANTI_DEIKHAN))
      fit=false;
    if(ch->hasClass(CLASS_MONK) && (extra_flags & ITEM_ANTI_MONK))
      fit=false;

    volume=convertTo<int>(db["volume"]);
    slot = slot_from_bit(convertTo<int>(db["wear_flag"]));
    if(type==ITEM_ARMOR || type==ITEM_ARMOR_WAND || type==ITEM_WORN){
      // check size restriction      
      perc=(((double) ch->getHeight()) * 
	    (double) race_vol_constants[mapSlotToFile(slot)]);
      if(extra_flags & ITEM_PAIRED)
	perc *= 2;
      
      
      if ((slot != WEAR_NECK) && (slot != WEAR_FINGER_R) && 
	  (slot != WEAR_FINGER_L) && (slot != WEAR_NOWHERE)) {
	if (volume > (int) (perc/0.85) ||
	    volume < (int) (perc/1.15))
	  fit=false;
      }
    }

    // class restrictions
    if((ch->hasClass(CLASS_MONK) || ch->hasClass(CLASS_SHAMAN)) &&
       (type==ITEM_ARMOR || type==ITEM_ARMOR_WAND))
      fit=false;
    
    
    // determine damage type for weapons
    isPierce=isBlunt=isSlash=false;
    if(type==ITEM_WEAPON){
      int x3=convertTo<int>(db["val2"]);
      int x4=convertTo<int>(db["val3"]);
      
      weaponT damage_type[3];
      int damage_freq[3];
      
      damage_type[0]=(weaponT)GET_BITS(x3, 7, 8);
      damage_freq[0]=GET_BITS(x3, 15, 8);
      damage_type[1]=(weaponT)GET_BITS(x3, 23, 8);
      damage_freq[1]=GET_BITS(x3, 31, 8);
      damage_type[2]=(weaponT)GET_BITS(x4, 7, 8);
      damage_freq[2]=GET_BITS(x4, 15, 8);
      
      int count_pierce=0, count_blunt=0, count_slash=0, total=0;
      
      for(int i=0;i<3;++i){
	// we have a lot of weapons with one damage type and frequency of 0
	if(!i && !damage_freq[0] && !damage_freq[1] && !damage_freq[2]){
	  damage_freq[0]=100;
	}

	if(pierceType(getWtype_kluge(damage_type[i]))){
	  count_pierce+=damage_freq[i];
	}
	if(bluntType(getWtype_kluge(damage_type[i]))){
	  count_blunt+=damage_freq[i];
	}
	if(slashType(getWtype_kluge(damage_type[i]))){
	  count_slash+=damage_freq[i];
	}
	total+=damage_freq[i];
      }

      if(count_pierce > (total/3.0*2.0))
	isPierce=true;
      if(count_blunt > (total/3.0*2.0))
	isBlunt=true;
      if(count_slash > (total/3.0*2.0))
	isSlash=true;
    }

    // check restrictions
    if(((FitT & (1 << 0)) && !fit) ||
       ((FitT & (1 << 1)) && !isSlash) ||
       ((FitT & (1 << 2)) && !isPierce) ||
       ((FitT & (1 << 3)) && !isBlunt) ||
       ((FitT & (1 << 4)) && slot != WEAR_BODY) ||
       ((FitT & (1 << 5)) && slot != WEAR_FINGER_L && 
                             slot != WEAR_FINGER_R) ||
       ((FitT & (1 << 6)) && slot != WEAR_WRIST_L &&
                             slot != WEAR_WRIST_R) ||
       ((FitT & (1 << 7)) && slot != WEAR_LEG_L &&
                             slot != WEAR_LEG_R) ||
       ((FitT & (1 << 8)) && slot != WEAR_ARM_L &&
                             slot != WEAR_ARM_R) ||
       ((FitT & (1 << 9)) && slot != WEAR_NECK) ||
       ((FitT & (1 << 10)) && slot != WEAR_FOOT_L &&
                              slot != WEAR_FOOT_R) ||
       ((FitT & (1 << 11)) && slot != WEAR_HAND_L &&
                              slot != WEAR_HAND_R) ||
       ((FitT & (1 << 12)) && slot != WEAR_HEAD) ||
       ((FitT & (1 << 13)) && slot != WEAR_BACK) ||
       ((FitT & (1 << 14)) && slot != WEAR_WAIST) ||
       ((FitT & (1 << 15)) && volume > 2000) ||
       ((FitT & (1 << 16)) && volume > 1500) ||
       ((FitT & (1 << 17)) && volume > 1500))
      continue;


    // buffer output
    if(type==ITEM_RAW_MATERIAL){
      buf+=format("[%8i] %s COMMODITY  [%6i] %7.3f\n\r") %
	convertTo<int>(db["rent_id"]) %
	list_string(short_desc, 40) % 
	convertTo<int>(db["count"]) %
	(max((float)1.0, price));
    } else if(type==ITEM_COMPONENT){
      sstring spell="";
      if(mapFileToSpellnum(convertTo<int>(db["val2"])) > -1){
	if(ch->doesKnowSkill(mapFileToSpellnum(convertTo<int>(db["val2"]))))
	  spell=discArray[mapFileToSpellnum(convertTo<int>(db["val2"]))]->name;
      }
      buf+=format("[%8i] %s %s [%6i] %7i\n\r") %
	convertTo<int>(db["rent_id"]) %
	list_string(short_desc, 30) % 
	list_string(spell, 20) %
	convertTo<int>(db["count"]) %
	(int)(max((float)1.0, price));      
    } else {
      buf+=format("[%8i] %s %s [%6i] %7i\n\r") %
	convertTo<int>(db["rent_id"]) %
	list_string(short_desc, 40) % 
	list_string(equip_cond(convertTo<int>(db["cur_struct"]),
			       convertTo<int>(db["max_struct"])), 10) %
	convertTo<int>(db["count"]) %
	(int)(max((float)1.0, price));
    }
  }
  
  if(ch->desc)
    ch->desc->page_string(buf, SHOWNOW_NO, ALLOWREP_YES);

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

  if (!stuff.empty())  // just can't see the shopkeepers inventory so lists nada?
    return;

  vlogf(LOG_MISC,format("Creating a new shopfile for %s (shop #%d)") % getName() %shop_nr);
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
  TObj *temp1;
  int rent_id;
  TDatabase db(DB_SNEEZY);
  char buf[256];

  if (!*arg) 
    return FALSE;   // generic: look

  if (!(shop_index[shop_nr].willTradeWith(keeper, ch)) || !ch->desc)
    return FALSE;

  if(!(rent_id=convertTo<int>(arg))){
    sstring query="select r.rent_id from obj o, rent r left outer join rent_strung rs on (rs.rent_id=r.rent_id) where r.vnum=o.vnum and r.owner_type='shop' and r.owner=%i ";
    sstring arg_words=arg;
    arg_words=arg_words.replaceString("-"," ");

    for(int i=0;!arg_words.word(i).empty();++i){
      mysql_escape_string(buf, arg_words.word(i).c_str(), arg_words.word(i).length());

      query += format("and ((rs.name is not null and rs.name like '%s%s%s') or (o.name like '%s%s%s'))") % 
	"%%" % buf % "%%" %
	"%%" % buf % "%%";
    }

    db.query(query.c_str(), shop_nr);
    db.fetchRow();
    rent_id=convertTo<int>(db["rent_id"]);
  }

  temp1=keeper->loadItem(shop_nr, rent_id);

  if (!temp1) 
    return FALSE;

  sstring str = "You examine ";
  str += temp1->getName();
  str += " sold by $N.\n\r";

  str += temp1->shopList(ch, "", 0, 999999, 0, shop_nr, rent_id-1, 0);

  act(str, FALSE, ch, temp1, keeper, TO_CHAR);

  tmp_desc = NULL;
  if ((tmp_desc = temp1->ex_description->findExtraDesc(fname(temp1->name).c_str()))) {
    ch->desc->page_string(tmp_desc);
  } else {
    ch->sendTo("You see nothing special.\n\r");
  }
  ch->describeObject(temp1);
  ch->showTo(temp1, SHOW_MODE_PLUS);  // tack on glowing, humming, etc

  delete temp1;
  return TRUE;
}

// if we process the command, return TRUE.
// if they are looking at something else (shopkeeper, etc) return FALSE
static bool shopping_evaluate(const char *arg, TBeing *ch, TMonster *keeper, int shop_nr)
{
  char newarg[100];
  int num;
  TObj *temp1;
  char buf[256];
  int rent_id;
  TDatabase db(DB_SNEEZY);

  if (!*arg) 
    return FALSE;   // generic: look

  if (!(shop_index[shop_nr].willTradeWith(keeper, ch)) || !ch->desc)
    return FALSE;

  if (!(num = getabunch(arg, newarg)))
    strcpy(newarg, arg);

  if (!num)
    num = 1;

  if(!(rent_id=convertTo<int>(arg))){
    sstring query="select r.rent_id from obj o, rent r left outer join rent_strung rs on (rs.rent_id=r.rent_id) where r.vnum=o.vnum and r.owner_type='shop' and r.owner=%i ";
    sstring arg_words=arg;
    arg_words=arg_words.replaceString("-"," ");

    for(int i=0;!arg_words.word(i).empty();++i){
      mysql_escape_string(buf, arg_words.word(i).c_str(), arg_words.word(i).length());

      query += format("and ((rs.name is not null and rs.name like '%s%s%s') or (o.name like '%s%s%s'))") % 
	"%%" % buf % "%%" %
	"%%" % buf % "%%";
    }

    db.query(query.c_str(), shop_nr);
    db.fetchRow();
    rent_id=convertTo<int>(db["rent_id"]);
  }

  temp1=keeper->loadItem(shop_nr, rent_id);

  if (!temp1) 
    return FALSE;

  act("You evaluate $p sold by $N.", FALSE, ch, temp1, keeper, TO_CHAR);

  ch->genericEvaluateItem(temp1);

  delete temp1;
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


// todo: this should really call kick_mobs_from_shop, but keep for now to
// preserve functionality
int kickFromShop(TMonster *keeper, TBeing *vagrant)
{
	for (dirTypeT dir = MIN_DIR; dir < MAX_DIR; dir++) {
	  if (exit_ok(keeper->exitDir(dir), NULL)) {

	    // at least one valid dir exists
	    // select the true direction at random
	    do {
	      dir = dirTypeT(::number(MIN_DIR, MAX_DIR-1));
	    } while (!exit_ok(keeper->exitDir(dir), NULL));
	    
	    act("$n throws you from $s shop.", FALSE, keeper, 0, vagrant, TO_VICT);
	    act("$n throws $N from $s shop.", FALSE, keeper, 0, vagrant, TO_NOTVICT);

	    keeper->throwChar(vagrant, dir, FALSE, SILENT_NO, true);
	    return TRUE;
	  }
	}
  return FALSE;
}


// produce items for this shop
int shopping_produce(TMonster *keeper)
{
  unsigned int shop_nr = find_shop_nr(keeper->number);
  TMonster *sbaKeeper = NULL;
  std::vector<int>::iterator iter;

  if (!keeper)
    return FALSE;
  if (!shop_index[shop_nr].producing.size())
    return FALSE;

  sbaKeeper = shop_index[SBA_SHOP_NR].getKeeper();
  if(!sbaKeeper)
    return FALSE;

  TShopOwned tso(shop_nr, keeper, sbaKeeper);

  // loop through items to produce
  for(iter=shop_index[shop_nr].producing.begin();iter!=shop_index[shop_nr].producing.end();++iter){

    TObj *o;

    if(*iter <= -1)
      continue;

    if (!(o = read_object(*iter, REAL))) {
      vlogf(LOG_BUG, format("Shopkeeper %d couldn't load produced item.") %  
      shop_nr);
      return FALSE;
    }

    TDatabase db(DB_SNEEZY);
    db.query("select count(*) as count from rent where owner_type='shop' and owner=%i and vnum=%i", shop_nr, o->objVnum());
    db.fetchRow();
    int count=convertTo<int>(db["count"]);
    
    if(count >= tso.getMaxNum(NULL, o, 10)){
      delete o;
      continue;
    }

    int cost=o->getValue();

    // obviously we shouldn't have to pay a million talens to create a
    // million talen casino chip
    if(dynamic_cast<TCasinoChip *>(o))
      cost=1;

    if(keeper->getMoney() < cost) {
      delete o;
      continue;
    }

    tso.doSellTransaction(cost, o->getName(), TX_PRODUCING);

    // money goes to sba
    TShopOwned tsba(SBA_SHOP_NR, sbaKeeper, keeper);
    tsba.journalize(keeper->getName(), o->getName(), TX_BUYING_SERVICE, 
		    cost, 0,0,0);
    shoplog(SBA_SHOP_NR, keeper, sbaKeeper, o->getName(), cost, "producing");

    sbaKeeper->saveItems(SBA_SHOP_NR);
    keeper->saveItem(shop_nr, o);
    delete o;
  }

  return FALSE;
}



int shop_keeper(TBeing *ch, cmdTypeT cmd, const char *arg, TMonster *myself, TObj *o)
{
  int rc;
  unsigned int shop_nr;

  // clear pointers that cache myself
  if (cmd == CMD_GENERIC_DESTROYED) {
    shop_nr=find_shop_nr(myself->number);
    shop_index[shop_nr].clearKeeper();
    return FALSE;
  }

  // Toss out idlers
  if (cmd == CMD_GENERIC_PULSE) {
    TThing *t=NULL;
    TBeing *tbt;

    for(StuffIter it=myself->roomp->stuff.begin();it!=myself->roomp->stuff.end() && (t=*it);++it){
      if((tbt=dynamic_cast<TBeing *>(t)) && tbt->getTimer()>1 && !tbt->isImmortal()){
        if ((tbt->master) && tbt->master->inRoom() == tbt->inRoom()) {
	        continue;
	      }
	      myself->doSay("Hey, no loitering!  Make room for the other customers.");
        kickFromShop(myself, tbt);
        return TRUE; // only kick one per pulse to avoid StuffIter issues
      }
    }
    return TRUE;
  }  

  // sanity check - shop mobs are unique
  if (cmd == CMD_GENERIC_INIT) {
    if (!myself->isUnique()) {
      vlogf(LOG_BUG, format("Warning!  %s attempted to be loaded, when not unique.") %  myself->getName());
      return TRUE;
    }
    return FALSE;
  }

  // kick fighting mobs out of the shop
  if (cmd == CMD_MOB_VIOLENCE_PEACEFUL) {
    myself->doSay("Hey!  Take it outside.");

    // o is really a being, so downcast, and then bring it back up
    TThing *ttt = o;
    if (!ttt)
      return FALSE;
    TBeing *tbt = dynamic_cast<TBeing *>(ttt);
    if (!tbt)
      return FALSE;

    kickFromShop(myself, ch);
    kickFromShop(myself, tbt);

    return TRUE;
  }
  
  // keep mobs out of our room
  if (cmd == CMD_MOB_MOVED_INTO_ROOM) {

    shop_nr=find_shop_nr(myself->number);
    if (myself->in_room == shop_index[shop_nr].in_room)
      return kick_mobs_from_shop(myself, ch, (int)o);
    return FALSE;
  }
  
  // produce stuff I'm supposed to have
  if (cmd == CMD_MOB_ALIGN_PULSE) {

    // called on a long period....
    if (::number(0,10))
      return FALSE;

    // produce new items
    return shopping_produce(myself);
  }

  // from here we begin the processing main set of shop commands
  if (cmd >= MAX_CMD_LIST)
    return FALSE;

  // init shop_nr here
  shop_nr=find_shop_nr(myself->number);

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

    if (!pRoom || (pRoom->spec != SPEC_ROOM_GH_DUMP)) {
      // possible alternative would be to move dropped stuff to Room::DONATION
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
  TThing *tt=NULL;
  TObj *o;  

  for(StuffIter it=keeper->stuff.begin();it!=keeper->stuff.end() && (tt=*it);++it){
    ++count;
    o=dynamic_cast<TObj *>(tt);
    value+=o->getValue();
  }
  
  TDatabase db(DB_SNEEZY);

  //  db.query("insert into shoplog values (%i, '%s', '%s', '%s', %i, %i, %i, now(), %i)", shop_nr, ch?ch->getName():"unknown", action.c_str(), name.c_str(), cost, keeper->getMoney(), value, count);

  queryqueue.push(format("insert into shoplog values (%i, '%s', '%s', '%s', %i, %i, %i, now(), %i)") % shop_nr % ((sstring)(ch?ch->getName():"unknown")).escape(sstring::SQL) % action.escape(sstring::SQL) % name.escape(sstring::SQL) % cost % keeper->getMoney() % value % count);


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
  isowned_db.query("select distinct shop_nr from shopowned order by shop_nr asc");
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

  // let kingdoms not tax their own holdings... they get all the profit anyway
  db.query("select shop_nr from shopowned where tax_nr is null and corp_id not in (21, 27, 29, 28)");

  while(db.fetchRow()){
    vlogf(LOG_LOW, format("Shop %s is untaxed.") % db["shop_nr"]);
  }


  db.query("select shop_nr from shop where shop_nr not in (select shop_nr from shopowned)");

  while(db.fetchRow()){
    vlogf(LOG_LOW, format("Shop %s is unowned.") % db["shop_nr"]);
  }



}


bool safe_to_save_shop_stuff(TMonster *ch)
{

  if (mob_index[ch->getMobIndex()].getNumber() < 1) {
     vlogf(LOG_BUG, format("Shopkeeper #%d got safe_to_save_shop_stuff called when none in world!") % 
            mob_index[ch->getMobIndex()].virt);
    ch->doSay("I'm not functioning properly.  Tell a god to check the logs, case 1.");
    return FALSE;
  }
  if (mob_index[ch->getMobIndex()].getNumber() > 1) {
    vlogf(LOG_BUG, format("More than one shopkeeper #%d in world.  Now the shop won't work!") % 
          mob_index[ch->getMobIndex()].virt);
    ch->doSay("I'm not functioning properly.  Tell a god to check the logs, case 2.");
    return FALSE;
  }
  return TRUE;
}


/*void processShopFile(const char *cFname)
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
    vlogf(LOG_BUG, format("  Error opening the shop file for shop #%s") %  cFname);
    return;
  }
  if (fread(&ucVersion, sizeof(ucVersion), 1, fp) != 1) {
    vlogf(LOG_BUG, format("Error reading version from %s.") %  fileName);
    fclose(fp);
    return;
  }

  if (!noteLimitedItems(fp, fileName, ucVersion, FALSE))
    vlogf(LOG_BUG, format("  Unable to count limited items in file  %s") %  fileName);
  fclose(fp);
}


void processShopFiles(void)
{
   dirwalk(SHOPFILE_PATH, processShopFile);
}*/


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
  close2(0),
  mkeeper(NULL),
  isCached(false)
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
  close2(t.close2),
  mkeeper(NULL),
  isCached(false)
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
  // don't copy mkeeper - this is a cache value

  clearCache();

  return *this;
}

TMonster *shopData::getKeeper()
{
  if (mkeeper)
    return mkeeper;

  TRoom *r = real_roomp(in_room);
  if (!r)
    return NULL;

  // uncached: find the shopkeeper
  for(StuffIter it=r->stuff.begin();it!=r->stuff.end();++it){
    TThing *t = *it;
    if (!t)
      continue;
    mkeeper = dynamic_cast<TMonster*>(t);
    if (!mkeeper)
      continue;
    if (mkeeper->number == keeper)
      break;
    mkeeper = NULL;
  }

  return mkeeper;
}


void factoryProduction(int shop_nr)
{
  TDatabase db_vnum(DB_SNEEZY);
  TDatabase db(DB_SNEEZY);
  TShopOwned tso(shop_nr);
  int vnum, count;
  TObj *obj;
  TMonster *keeper=tso.getKeeper();
  bool ready=false;
  std::map<sstring, int>supplies;
  std::map<sstring, int>::iterator iter;

  if (!keeper) // keeper is null on test ports
    return;

  // find the sba shopkeeper
  TMonster *sba = shop_index[SBA_SHOP_NR].getKeeper();
  if(!sba)
    return;

  db_vnum.query("select fp.vnum as vnum, count(r.vnum) as count from factoryproducing fp left outer join rent r on (fp.vnum=r.vnum and r.owner_type='shop' and r.owner=fp.shop_nr) where fp.shop_nr=%i group by fp.vnum", shop_nr);

  while(db_vnum.fetchRow()){
    vnum=convertTo<int>(db_vnum["vnum"]);
    count=convertTo<int>(db_vnum["count"]);

    db.query("select fb.supplyamt as required, fs.supplyamt as avail, fs.supplyname as name from factoryblueprint fb, factorysupplies fs where fb.vnum=%i and fs.shop_nr=%i and fs.supplytype=fb.supplytype", vnum, shop_nr);

    ready=true;
    while(db.fetchRow() && ready){
      if(convertTo<int>(db["required"]) > convertTo<int>(db["avail"]))
	ready=false;
      supplies[db["name"]]=convertTo<int>(db["required"]);
    }
     
    if(!ready)
      continue;

    // read object
    if (!(obj = read_object(vnum, VIRTUAL))) {
      vlogf(LOG_BUG, format("Unable to load object %i for factory") % vnum);
      continue;
    }

    if(count >= tso.getMaxNum(NULL, obj, 10)){
      delete obj;
      continue;
    }

    // place in shop
    keeper->saveItem(shop_nr, obj);
    keeper->giveMoney(sba, obj->productionPrice(), GOLD_SHOP);

    // money goes to sba
    TShopOwned tsba(SBA_SHOP_NR, sba, keeper);
    tsba.journalize(keeper->getName(), obj->getName(), TX_BUYING_SERVICE,
		    obj->productionPrice(), 0,0,0);
    shoplog(SBA_SHOP_NR, keeper, sba, obj->getName(), 
	    obj->productionPrice(), "producing");
    
    // subtract raw materials
    int COGS=0, total_cogs=0;
    bool first_time=true;
    for(iter=supplies.begin();iter!=supplies.end();++iter){
      sstring name=(*iter).first;
      int num=(*iter).second;

      // COGS of this material
      COGS=tso.COGS_get(name, num);
      total_cogs+=COGS;
      
      // inventory - remove this material
      tso.journalize_credit(130, keeper->getName(), name, COGS, first_time);
      first_time=false;

      // record COGS
      tso.COGS_remove(name, num);
    }

    // cash - labor costs for production
    tso.journalize_credit(100, keeper->getName(), 
			  obj->getName(), obj->productionPrice());
    
    // inventory - add the value of the newly produced item
    tso.journalize_debit(130, keeper->getName(), obj->getName(), 
		     obj->productionPrice()+total_cogs);
    
    // record COGS
    tso.COGS_add(obj->getName(), obj->productionPrice()+total_cogs, 1);

    // log the sale
    shoplog(shop_nr, keeper, keeper, obj->getName(), -(obj->productionPrice()), "factory production");

    delete obj;

    // save avail amt
    db.query("update factorysupplies, factoryblueprint set factorysupplies.supplyamt=factorysupplies.supplyamt-factoryblueprint.supplyamt where factorysupplies.shop_nr=%i and factoryblueprint.vnum=%i and factorysupplies.supplytype=factoryblueprint.supplytype", shop_nr, vnum);

  }
}
