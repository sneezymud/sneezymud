#include <cmath>
#include <unistd.h>
#include <algorithm>

#include "stdsneezy.h"
#include "shop.h"
#include "statistics.h"
#include "drug.h"

vector<shopData>shop_index(0);

#if SHOP_PRICES_FLUXUATE
vector<shop_pricing>ShopPriceIndex(0);
#endif

#define IMMORTEQTEST 1
#define FLUX_SHOP_DEBUG     0

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


bool shopOwned(int shop_nr){
  bool owned;
  int rc;
  MYSQL_RES *res;
  MYSQL_ROW row;
  
  if((rc=dbquery(&res, "sneezy", "shopOwned", "select * from shopownedaccess where shop_nr=%i", shop_nr))==-1){
    vlogf(LOG_BUG, "Database error in shop_keeper");
    return FALSE;
  }
  if(!(row=mysql_fetch_row(res)))
    owned=false;
  else
    owned=true;
  mysql_free_result(res);
  return owned;
}


// this is the price the shop will buy an item for
int TObj::sellPrice(int shop_nr, int chr, int *discount)
{
  int cost;
  if (chr != -1)
    cost = (int) (adjPrice(discount) * shop_index[shop_nr].profit_sell *
                 (100 - (2 * (18 - chr))) / 100);
  else
    cost = (int) (adjPrice(discount) * shop_index[shop_nr].profit_sell);

  if (obj_flags.cost <= 1) {
    cost = max(0, cost);
  } else {
    cost = max(1, cost);
  }

  // scale based on global settings
  cost = (int) (cost * gold_modifier[GOLD_SHOP].getVal());

  return cost;
}

// this is price shop will sell it at
int TObj::shopPrice(int num, int shop_nr, int chr, int *discount) const
{
  int cost;
  if (chr != -1)
    cost = (int) (adjPrice(discount) * shop_index[shop_nr].profit_buy *
                 (100 + (2 * (18 - chr))) / 100);
  else
    cost = (int) (adjPrice(discount) * shop_index[shop_nr].profit_buy);

  cost *= num;
  cost = max(1, cost);

  return cost;
}

bool is_ok(TMonster *keeper, TBeing *ch, int shop_nr)
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
#if 0
  switch (shop_index[shop_nr].flags) {
    case 0:
      return TRUE;
    case 1:
      return TRUE;
    default:
      return TRUE;
  }
#endif
  return TRUE;
}

bool trade_with(const TObj *item, int shop_nr)
{
  int counter, max_trade;
  bool mat_ok=FALSE;

  if (item->obj_flags.cost < 1 || item->isObjStat(ITEM_NEWBIE))
    return FALSE;

  if (item->isObjStat(ITEM_PROTOTYPE))
    return FALSE;

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

  max_trade=shop_index[shop_nr].type.size();

  for (counter = 0; counter < max_trade; counter++) {
    if ((int) shop_index[shop_nr].type[counter] == item->itemType())
      return TRUE;
  }

  if (shop_producing(item, shop_nr))
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

static void load_shop_file(TMonster *ch, int shop_nr)
{
  char buf[256];

  sprintf(buf, "%s/%d", SHOPFILE_PATH, shop_nr);
  ch->loadItems(buf);

  // do some discounting where appropriate
  TThing *t;
  TObj *obj;
  int discount = 100, price;
  for (t = ch->stuff; t; t = t->nextThing) {
    obj = dynamic_cast<TObj *>(t);
    if (!obj)
      continue;
    price = obj->shopPrice(1, shop_nr, -1, &discount);
    if (discount < 100) {
      // item has not been sold in quite awhile
      // fake a sell/buy to A) reset the discount timer and B) reduce the price
      obj->recalcShopData(TRUE, price);
      price = obj->sellPrice(shop_nr, -1, &discount);
      obj->recalcShopData(FALSE, price);
    }
  }
}


bool shop_producing(const TObj *item, int shop_nr)
{
  int counter, max_prod=0;
  TObj *o;

  if (item->number < 0)
    return FALSE;

  max_prod=shop_index[shop_nr].producing.size();

  for (counter = 0; counter < max_prod; counter++) {
    if (shop_index[shop_nr].producing[counter] <= -1)
      continue;

    if (shop_index[shop_nr].producing[counter] == item->number) {
      if (!(o = read_object(shop_index[shop_nr].producing[counter], REAL))) {
        vlogf(LOG_BUG, "Major problems with shopkeeper number %d and item number %d.", shop_nr, item->number);
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

void shopping_trade(const char *, TBeing *, TBeing *, int)
{
}

static int number_objects_in_list(const TObj *item, const TObj *list)
{
  const TObj *i = NULL;
  const TThing *t;
  int count = 0;
  int discount;

  for (t = list; t; t = t->nextThing) {
    i = dynamic_cast<const TObj *>(t);
    if (!i)
      continue;
    if ((i->number == item->number) &&
        (i->getName() && item->getName() && !strcmp(i->getName() , item->getName())) &&
        (i->adjPrice(&discount) == item->adjPrice(&discount)))
      count++;
  }
  return (count);
}

void shopping_buy(const char *arg, TBeing *ch, TMonster *keeper, int shop_nr)
{
  char argm[MAX_INPUT_LENGTH], buf[512], newarg[MAX_INPUT_LENGTH];
  int num = 1;
  TObj *temp1 = NULL;
  TThing *tt;

  *argm = '\0';

  if (!(is_ok(keeper, ch, shop_nr)))
    return;

  only_argument(arg, argm);
  if (!*argm) {
    sprintf(buf, "%s What do you want to buy??", ch->name);
    keeper->doTell(buf);
    return;
  }
  if ((num = getabunch(argm, newarg)))
    strcpy(argm, newarg);

  if (!num)
    num = 1;

  tt = searchLinkedListVis(ch, argm, keeper->stuff);
  if (!tt || !(temp1 = dynamic_cast<TObj *>(tt))) {
    if (!(temp1 = get_num_obj_in_list(ch, atoi(argm), keeper->stuff, shop_nr))) {
      sprintf(buf, shop_index[shop_nr].no_such_item1, ch->name);
      keeper->doTell(buf);
      return;
    }
  }

  if (temp1->obj_flags.cost <= 0) {
    sprintf(buf, shop_index[shop_nr].no_such_item1, ch->name);
    keeper->doTell(buf);
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
  if (ch->desc && IS_SET(ch->desc->account->flags, ACCOUNT_IMMORTAL) &&
      !ch->isImmortal() &&
      (obj_index[temp1->getItemIndex()].max_exist <= MIN_EXIST_IMMORTAL)) {
#if IMMORTEQTEST
    sprintf(buf, "%s This item is restricted for immortals.  If it ever reaches its max exist, it will be taken from you.", fname(ch->name).c_str());
    keeper->doTell(buf);
#else
    sprintf(buf, "%s I'd love to sell it to you, but your immortality prevents you from renting it...", fname(ch->name).c_str());
    keeper->doTell(buf);
    return;
#endif
  }

  temp1->buyMe(ch, keeper, num, shop_nr);
}

void TObj::buyMe(TBeing *ch, TMonster *keeper, int num, int shop_nr)
{
  char buf[256];
  char argm[256];
  int cost;
  int count = 0;
  int tmp;
  int chr;
  int i;
  int discount = 100;

  if ((ch->getCarriedVolume() + (num * getTotalVolume())) > ch->carryVolumeLimit()) {
    ch->sendTo("%s: You can't carry that much volume.\n\r", fname(name).c_str());
    return;
  }
  // obj-weight > free ch limit
  if (compareWeights(getTotalWeight(TRUE),
       ((ch->carryWeightLimit() - ch->getCarriedWeight())/num)) == -1) {
    ch->sendTo("%s: You can't carry that much weight.\n\r", fname(name).c_str());
    return;
  }
  if (shop_producing(this, shop_nr)) {
    chr = ch->plotStat(STAT_CURRENT, STAT_CHA, 3, 18, 13);
    
    if (ch->doesKnowSkill(SKILL_SWINDLE)) {
      // make 5 separate rolls so chr goes up amount based on learning
      for (i = 0; i < 5; i++)
        if (bSuccess(ch, ch->getSkillValue(SKILL_SWINDLE), SKILL_SWINDLE))
          chr += 1;
    }
    chr = min(18,chr);

    cost = shopPrice(1, shop_nr, chr, &discount);

    while (num-- > 0) {
      TObj *temp1;
      temp1 = read_object(number, REAL);

      if ((ch->getMoney() < cost) && !ch->hasWizPower(POWER_GOD)) {
        sprintf(buf, shop_index[shop_nr].missing_cash2, ch->name);
        keeper->doTell(buf);
    
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
      keeper->addToMoney(-obj_flags.cost, GOLD_SHOP);
      

      ch->logItem(temp1, CMD_BUY);
      count++;
      temp1->recalcShopData(TRUE, cost);
    }
    keeper->saveItems(buf);
  } else {
    tmp = number_objects_in_list(this, (TObj *) keeper->stuff);
    if (num > tmp) {
      sprintf(buf, "%s I don't have %d of that item. Here %s the %d I do have.",
                ch->name, num , (tmp > 1) ? "are" : "is", tmp);
      keeper->doTell(buf);
    } else
      tmp = num;

    strcpy(argm, name);

    add_bars(argm);
    chr = ch->plotStat(STAT_CURRENT, STAT_CHA, 3, 18, 13);

    if (ch->doesKnowSkill(SKILL_SWINDLE)) {
      // make 5 separate rolls so chr goes up amount based on learning
      for (i = 0; i < 5; i++)
        if (bSuccess(ch, ch->getSkillValue(SKILL_SWINDLE), SKILL_SWINDLE))
          chr += 1;
    }
    chr = min(18,chr);

    cost = shopPrice(1, shop_nr, chr, &discount);

    for (i = 0; i < tmp; i++) {
      TThing *t_temp1 = searchLinkedListVis(ch, argm, keeper->stuff);
      TObj *temp1 = dynamic_cast<TObj *>(t_temp1);

#if !(NO_DAMAGED_ITEMS_SHOP)
      while (!temp1->isSimilar(this)) {
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
        sprintf(buf, shop_index[shop_nr].missing_cash2, ch->name);
        keeper->doTell(buf);

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
      --(*temp1);
      *ch += *temp1;

      temp1->purchaseMe(ch, keeper, cost, shop_nr);

      ch->logItem(temp1, CMD_BUY);
      count++;
      temp1->recalcShopData(TRUE, cost);
    }
    sprintf(buf, "%s/%d", SHOPFILE_PATH, shop_nr);
    keeper->saveItems(buf);
  }
  if (!count)
    return;

  sprintf(buf, shop_index[shop_nr].message_buy, ch->name,
          (cost * count));
  keeper->doTell(buf);

  ch->sendTo(COLOR_OBJECTS, "You now have %s (*%d).\n\r", 
          good_uncap(getName()).c_str(), count);
  if (count == 1) 
    act("$n buys $p.", FALSE, ch, this, NULL, TO_ROOM); 
  else {
    sprintf(buf, "$n buys %d %ss.", count, fname(name).c_str());
    act(buf, FALSE, ch, this, 0, TO_ROOM);
  }
  ch->doSave(SILENT_YES);
  return;
}

bool will_not_buy(TBeing *ch, TMonster *keeper, TObj *temp1, int)
{
  char buf[160];

  if (!ch->isImmortal() && temp1->objectSell(ch, keeper)) {
    return TRUE;
  }
#if NO_DAMAGED_ITEMS_SHOP
  if (temp1->getStructPoints() != temp1->getMaxStructPoints()) {
    sprintf(buf, "%s I don't buy damaged goods.", ch->getName());
    keeper->doTell(buf);
    return TRUE;
  }
#endif
  if (temp1->stuff) {
    sprintf(buf, "%s Sorry, I don't buy items that contain other items.", ch->getName());
    keeper->doTell(buf);
    return TRUE;
  }
  // Notes have been denied by objectSell() above
  if (temp1->action_description) {
    sprintf(buf, "%s I'm sorry, I don't buy monogrammed goods.", ch->getName());
    keeper->doTell(buf);
    return TRUE;
  }
  if(temp1->isObjStat(ITEM_BURNING) || temp1->isObjStat(ITEM_CHARRED)){
    sprintf(buf, "%s I'm sorry, I don't buy fire damaged goods.", ch->getName());
    keeper->doTell(buf);
    return TRUE;
  }

  return FALSE;
}


bool TObj::sellMeCheck(const TBeing *, TMonster *) const
{
  return FALSE;
}

void generic_sell(TBeing *ch, TMonster *keeper, TObj *obj, int shop_nr)
{
  if (obj->isObjStat(ITEM_NODROP)) {
    ch->sendTo("You can't let go of it, it must be CURSED!\n\r");
    return;
  }
  if (obj->isObjStat(ITEM_PROTOTYPE)) {
    ch->sendTo("That's a prototype, no selling that!\n\r");
    return;
  }
  if (!trade_with(obj, shop_nr)) {
    char buf[256];
    sprintf(buf, shop_index[shop_nr].do_not_buy, ch->getName());
    keeper->doTell(buf);
    return;
  }
  if (will_not_buy(ch, keeper, obj, shop_nr)) 
    return;
  
  obj->sellMe(ch, keeper, shop_nr);
  // obj may be invalid here
}

void TObj::sellMe(TBeing *ch, TMonster *keeper, int shop_nr)
{
  int cost;
  char buf[256];
  int chr;
  int j;
  int discount = 100;

  if (!shop_index[shop_nr].profit_sell) {
    sprintf(buf, shop_index[shop_nr].do_not_buy, ch->getName());
    keeper->doTell(buf);
    return;
  }
  if (obj_flags.cost <= 1 || isObjStat(ITEM_NEWBIE)) {
    sprintf(buf, "%s I'm sorry, I don't buy valueless items.", ch->getName());
    keeper->doTell(buf);
    return;
  }
  if (sellMeCheck(ch, keeper))
    return;

  chr = ch->plotStat(STAT_CURRENT, STAT_CHA, 3, 18, 13);

  if (ch->doesKnowSkill(SKILL_SWINDLE)) {
    // make 5 separate rolls so chr goes up amount based on learning
    for (j = 0; j < 5; j++)
      if (bSuccess(ch, ch->getSkillValue(SKILL_SWINDLE), SKILL_SWINDLE))
        chr += 1;
  }
  chr = min(18,chr);


  cost = sellPrice(shop_nr, chr, &discount);

  if (getStructPoints() != getMaxStructPoints()) {
    cost *= 6;    /* base deduction of 60% */
    cost /= 10;
    if (getMaxStructPoints() > 0) {
      cost *= getStructPoints();
      cost /= getMaxStructPoints();
    }
#if NO_DAMAGED_ITEMS_SHOP
    sprintf(buf, "%s It's been damaged, but I guess I can buy it as scrap.", 
                 fname(ch->name).c_str());
    keeper->doTell(buf);
#endif
  }
  max(cost, 1);   // at least 1 talen 
  if (keeper->getMoney() < cost) {
    sprintf(buf, shop_index[shop_nr].missing_cash1, ch->getName());
    keeper->doTell(buf);
    return;
  }
  if (isRare() && obj_index[getItemIndex()].max_exist <= 10) {
    sprintf(buf, "%s Wow!  This is one of those limited items.", ch->name);
    keeper->doTell(buf);
    sprintf(buf, "%s You should really think about auctioning it.", ch->name);
    keeper->doTell(buf);
  }
  act("$n sells $p.", FALSE, ch, this, 0, TO_ROOM);

  sprintf(buf, shop_index[shop_nr].message_sell, ch->getName(), cost);
  keeper->doTell(buf);

  ch->sendTo(COLOR_OBJECTS, "The shopkeeper now has %s.\n\r", good_uncap(getName()).c_str());
  ch->logItem(this, CMD_SELL);

  --(*this);
  if (!shop_producing(this, shop_nr)) {
    *keeper += *this;
  }


  sellMeMoney(ch, keeper, cost, shop_nr);
  recalcShopData(FALSE, cost);

  if (ch->isAffected(AFF_GROUP) && ch->desc &&
           IS_SET(ch->desc->autobits, AUTO_SPLIT) && 
          (ch->master || ch->followers)){
    sprintf(buf, "%d", cost);
    ch->doSplit(buf, false);
  }

  // exception to the eq-sharing log, if person sells item to shop as char#1
  // permit them to buy it as char#2
  // this means the "owners" log should be reset on sell
  delete [] owners;
  owners = NULL;


  if (shop_producing(this, shop_nr)) {
    // unlimited item, so we just get the value of the item in talens
    keeper->addToMoney(this->obj_flags.cost, GOLD_SHOP);
    delete this;
  }
#if NO_DAMAGED_ITEMS_SHOP
  else if (getStructPoints() != getMaxStructPoints()) {
    // delete it as its "scrap"
    delete this;
  }
#endif


  sprintf(buf, "%s/%d", SHOPFILE_PATH, shop_nr);
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
  
  for (t = stuff; t; t = t2) {
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
  for (t = stuff; t; t = t2) {
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

tObjectManipT ObjectManipType(string tStString, string & tStBuffer, itemTypeT & tItem)
{
  if (tStString.empty()) {
    return OBJMAN_NULL;
    tStBuffer = "";
  }

  string tStType(""),
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
      if (isname(tStPassed.c_str(), ItemInfo[tItemType]->name)) {
        tItem = tItemType;
        return OBJMAN_TYPE;
      }

    return OBJMAN_ALLT;
  }

  return OBJMAN_NONE;
}

bool TObj::fitsSellType(tObjectManipT tObjectManip,
                        TBeing *ch, TMonster *tKeeper,
                        string tStString, itemTypeT tItemType,
                        int & tCount, int tShop)
{
  switch (tObjectManip) {
    case OBJMAN_NONE: // sell <object>
      if (isname(tStString.c_str(), name) && tCount < 1) {
        tCount++;
        return true;
      }
      break;
    case OBJMAN_ALL: // sell all
      if (trade_with(this, tShop)) {
        tCount++;
        return true;
      }
      break;
    case OBJMAN_ALLT: // sell all.<object>
      if (isname(tStString.c_str(), name)) {
        tCount++;
        return true;
      }
      break;
    case OBJMAN_FIT: // sell all.fit
      if (fitInShop("yes", ch) && trade_with(this, tShop)) {
        tCount++;
        return true;
      }
      break;
    case OBJMAN_NOFIT: // sell all.nofit
      if (!fitInShop("yes", ch) && trade_with(this, tShop)) {
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
  char argm[100], buf[512],
       tBuffer[256];
  TObj *temp1 = NULL;
  TThing *t, *t2;
  int rc, i;

  if (!(is_ok(tKeeper, ch, shop_nr)))
    return FALSE;

  only_argument(tString, argm);

  if (!*argm) {
    sprintf(buf, "%s What do you want to sell??", ch->getName());
    tKeeper->doTell(buf);
    return FALSE;
  }

  if (0 && gamePort != PROD_GAMEPORT) {
    string         tStString("");
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
        sprintf(tBuffer, "%s And what is it you want to sell??",
                ch->getName());
        tKeeper->doTell(tBuffer);
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

    for (tThing = ch->stuff; tThing; tThing = tThingTemp) {
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
      for (t = ch->stuff; t; t = t2) {
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

        rc = t->componentSell(ch, tKeeper, shop_nr, NULL);
        if (IS_SET_DELETE(rc, DELETE_THIS)) {
          delete t;
          t = NULL;
        }
        if (IS_SET_DELETE(rc, DELETE_VICT)) 
          return DELETE_THIS;
      }
      for (t = ch->stuff; t; t = t2) {
        t2 = t->nextThing;
        if (!ch->sameRoom(*tKeeper))
          break;
        // check for sleep pouches
        if (!ch->awake())
          break;

        rc = t->componentSell(ch, tKeeper, shop_nr, NULL);
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
      for (t = ch->stuff; t; t = t2) {
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
  TThing *t_temp1 = searchLinkedListVis(ch, argm, ch->stuff);
  temp1 = dynamic_cast<TObj *>(t_temp1);
  if (!temp1) {
    sprintf(buf, shop_index[shop_nr].no_such_item2, ch->getName());
    tKeeper->doTell(buf);
    return FALSE;
  }
  generic_sell(ch, tKeeper, temp1, shop_nr);

  return FALSE;
}

void shopping_value(const char *arg, TBeing *ch, TMonster *keeper, int shop_nr)
{
  char argm[100], buf[512];
  TObj *temp1;

  if (!(is_ok(keeper, ch, shop_nr)))
    return;

  only_argument(arg, argm);

  if (!*argm) {
    sprintf(buf, "%s What do you want me to evaluate??", ch->name);
    keeper->doTell(buf);
    return;
  } else if (is_abbrev(argm, "all.components")) {
    TThing *t, *t2;
    int i;
    for (i = MIN_WEAR; i < MAX_WEAR; i++) {
      if (!(t = ch->equipment[i]))
        continue;

      t->componentValue(ch, keeper, shop_nr, NULL);
    }
    for (t = ch->stuff; t; t = t2) {
      t2 = t->nextThing;

      t->componentValue(ch, keeper, shop_nr, NULL);
    }
    return;
  }

  TThing *t_temp1 = searchLinkedListVis(ch, argm, ch->stuff);
  temp1 = dynamic_cast<TObj *>(t_temp1);
  if (!temp1) {
    sprintf(buf, shop_index[shop_nr].no_such_item2, ch->name);
    keeper->doTell(buf);
    return;
  }
  if (!(trade_with(temp1, shop_nr))) {
    sprintf(buf, shop_index[shop_nr].do_not_buy, ch->name);
    keeper->doTell(buf);
    return;
  }
  if (will_not_buy(ch, keeper, temp1, shop_nr)) 
    return;
  
  temp1->valueMe(ch, keeper, shop_nr);
}

void TObj::valueMe(TBeing *ch, TMonster *keeper, int shop_nr)
{
  int chr;
  int cost;
  char buf[256];
  int discount = 100;
  int willbuy=0;
  
#if 0
  if (sellMeCheck(ch, keeper))
    return;
#else
  willbuy=!sellMeCheck(ch, keeper);
#endif

  if (!trade_with(this, shop_nr)) {
    char buf[256];
    sprintf(buf, shop_index[shop_nr].do_not_buy, ch->getName());
    keeper->doTell(buf);
    return;
  }

  chr = ch->plotStat(STAT_CURRENT, STAT_CHA, 3, 18, 13);

  // do not adjust for swindle on valueing, give them worst case price
  chr = min(18,chr);

  cost = sellPrice(shop_nr, chr, &discount);

  if (isRare() && obj_index[getItemIndex()].max_exist <= 10) {
    sprintf(buf, "%s Wow!  This is one of those limited items.", ch->name);
    keeper->doTell(buf);
    sprintf(buf, "%s You should really think about auctioning it.", ch->name);
    keeper->doTell(buf);
  }

  if ((getStructPoints() != getMaxStructPoints()) &&
       (getMaxStructPoints() > 0)) {
    cost *= 6;    /* base deduction of 60% */
    cost /= 10;
    cost *= getStructPoints();
    cost /= getMaxStructPoints();
#if NO_DAMAGED_ITEMS_SHOP
    sprintf(buf, "%s It's been damaged, but I guess I can buy it as scrap.", 
             fname(ch->name).c_str());
    keeper->doTell(buf);
#endif
  }
  max(cost, 1);  // at least 1 talen
  if(willbuy){
    sprintf(buf, "%s I'll give you %d talens for %s!", ch->name, cost, getName());
  } else {
    sprintf(buf, "%s Normally, I'd give you %d talens for %s!", ch->name, cost, getName());
  }
  keeper->doTell(buf);

  if (keeper->getMoney() < cost) {
    sprintf(buf, "%s Unfortunately, at the moment, I can not afford to buy that item from you.", fname(ch->name).c_str());
    keeper->doTell(buf);
    return;
  }

  return;
}

const string TObj::shopList(const TBeing *ch, const char *arg, int iMin, int iMax, int num, int shop_nr, int k, unsigned long int FitT) const
{
  int cost, found = FALSE;
  char buf[256];
  char buf3[256];
  char buf4[256];
  char capbuf[256];
  char atbuf[25];
  char wcolor[25];
  int counter;
  int chr;
  float perc;
  int discount = 100;
  bool isWearable = false;
  const TGenWeapon * tWeapon = dynamic_cast<const TGenWeapon *>(this);

  // display spells on things like scrolls
  // don't show the "level" of weaps/armor though
  if (shop_producing(this, shop_nr) &&
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
  chr = ch->plotStat(STAT_CURRENT, STAT_CHA, 3, 18, 13);
  // do not adjust for swindle on list, give them worst case price
  chr = min(18, chr);

  cost = shopPrice(1, shop_nr, chr, &discount);

  wearSlotT slot;
  slot = slot_from_bit(obj_flags.wear_flags);

  if (slot > 0) {
    perc = (((double) ch->getHeight() / (double) 70) * (double) race_vol_constants[mapSlotToFile(slot)]);
    if (isPaired())
      perc *= 2;
  } else
    perc = 0;

  const TBaseClothing *tbc = dynamic_cast<const TBaseClothing *>(this);

  if (!ch->canUseEquipment(this, SILENT_YES)) {
    sprintf(buf3, "forbidden");
  } else if (tbc && tbc->isSaddle()) {
    sprintf(buf3, "for mounts");
  } else if (ch->desc && IS_SET(ch->desc->account->flags, ACCOUNT_IMMORTAL) &&
             obj_index[getItemIndex()].max_exist < MIN_EXIST_IMMORTAL &&
	     !IMMORTEQTEST) {
    sprintf(buf3, "imm. prohib.");
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

  sprintf(buf4, "[%s]", (shop_producing(this, shop_nr) ? "Unlimited" : atbuf));
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
      sprintf(buf, "%s[%2d] %-29s %s%-12s %-6d%s %-5s %c%s%s\n\r",
             wcolor, k + 1, cap(capbuf),
             equipColor, equipCond, cost, 
             (discount < 100 ? "(*)" : "   "),
             buf4, ((ch->desc && 
		     IS_SET(ch->desc->account->flags, ACCOUNT_IMMORTAL) &&
		     obj_index[getItemIndex()].max_exist<MIN_EXIST_IMMORTAL) ?
		    '*' : ' '), buf3, ch->norm());
      found = TRUE;
      strcpy(wcolor, ch->norm());
      break;
    }
  }

  if (!found) {
    strcpy(equipColor, equip_condition(-1).c_str());
    strcpy(equipCond, equipColor + 3);
    equipColor[3] = '\0';
    sprintf(buf, "%s[%2d] %-31s %s%-12s %-6d%s %-5s\n\r",
            wcolor, k + 1, cap(capbuf), equipColor, equipCond, cost,
            (discount < 100 ? "(*)" : "   "), buf4);
  }

  // This is for quick listing, fast and simple.
  if (!*arg && iMin == 999999 && FitT == 0)
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
      (!*arg || isname(arg, name)) &&
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

void shopping_list(const char *argument, TBeing *ch, TMonster *keeper, int shop_nr)
{
  vector<TObj *>cond_obj_vec;
  vector<int>cond_tot_vec;

  TObj *i = NULL;
  unsigned int k;
  unsigned long int FitT = 0;
  bool found = FALSE;
  int iMin = 999999, iMax = 0;
  char buf2[100], stString[256] = "\0";
  int found_obj;
  int counter;
  string sb;
  int rc;
  bool hasComponents = false;

#if 0
  if (gamePort != PROD_GAMEPORT) {
    if (ch->desc && ch->desc->m_bIsClient)
      desc->clientShoppingList(arg, keeper, shop_nr);
  }
#endif

  *buf2 = '\0';

  if (!is_ok(keeper, ch, shop_nr))
    return;

  if (!ch->desc)
    return;

  char arg[256];
  strcpy(arg, argument);
  cleanCharBuf(arg);

  // Here we rip apart what they might have passed.  We do it
  // this way to allow them to form it in any fashion they want.
  // For numbers.  First number found is givin to iMax.  Second
  // number found is givin to iMax and iMin is givin the old iMax
  // value.  So:
  //   1 number : No floor value, value is considered max.
  //   2 numbers: First is floor, 2nd is max.
  *arg = '\000';
  for (; isspace(*argument); argument++);
  argument = one_argument(argument, stString);
  for (; ; argument = one_argument(argument, stString)) {
         if (is_abbrev(stString, "fit")    ) FitT |= (1 <<  0);
    else if (is_abbrev(stString, "slash")  ) FitT |= (1 <<  1);
    else if (is_abbrev(stString, "pierce") ) FitT |= (1 <<  2);
    else if (is_abbrev(stString, "blunt")  ) FitT |= (1 <<  3);
    else if (is_abbrev(stString, "body")   ) FitT |= (1 <<  4);
    else if (is_abbrev(stString, "finger") ) FitT |= (1 <<  5);
    else if (is_abbrev(stString, "wrist")  ) FitT |= (1 <<  6);
    else if (is_abbrev(stString, "legs")   ) FitT |= (1 <<  7);
    else if (is_abbrev(stString, "arms")   ) FitT |= (1 <<  8);
    else if (is_abbrev(stString, "neck")   ) FitT |= (1 <<  9);
    else if (is_abbrev(stString, "feet")   ) FitT |= (1 << 10);
    else if (is_abbrev(stString, "hands")  ) FitT |= (1 << 11);
    else if (is_abbrev(stString, "head")   ) FitT |= (1 << 12);
    else if (is_abbrev(stString, "back")   ) FitT |= (1 << 13);
    else if (is_abbrev(stString, "waist")  ) FitT |= (1 << 14);
    else if (is_abbrev(stString, "glowing")) FitT |= (1 << 15);
    else if (is_abbrev(stString, "shadowy")) FitT |= (1 << 16);
    else if (is_abbrev(stString, "paired") ) FitT |= (1 << 17);
    else if (is_abbrev(stString, "stab")) {
      if ((ch->hasClass(CLASS_THIEF) && ch->doesKnowSkill(SKILL_STABBING)) || ch->isImmortal()) {
        FitT |= (1 << 18);
        FitT |= (1 <<  2);
      }
    } else if (is_abbrev(stString, "cudgel")) {
      if ((ch->hasClass(CLASS_THIEF) && ch->doesKnowSkill(SKILL_CUDGEL)) || ch->isImmortal()) {
        FitT |= (1 << 19);
        FitT |= (1 <<  3);
      }
    } else if (is_abbrev(stString, "backstab")) {
      if ((ch->hasClass(CLASS_THIEF) && ch->doesKnowSkill(SKILL_BACKSTAB)) || ch->isImmortal()) {
        FitT |= (1 << 20);
        FitT |= (1 <<  2);
      }
    } else if (is_number(stString)) {
      if (iMin == 999999) {
        iMin = 0;
        iMax = atoi(stString);
      } else if (iMin == 0) {
        iMin = iMax;
        iMax = atoi(stString);
      }
    } else if (*stString) {
      strcpy(arg, stString);
    }
    if (!*argument) break;
  }

  int max_trade=0;
  max_trade=shop_index[shop_nr].type.size()-1;

  sprintf(buf2, "%s You can buy:", ch->getName());
  keeper->doTell(buf2);
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
      sb += "     Item Name                            Condition  Price    Number\n\r";
    else
      sb += "     Iten Name                            Charges Spell\n\r";

  found = FALSE;
  sb += "-------------------------------------------------------------------------------\n\r";

  found_obj = FALSE;

  TThing *t, *t2;
  for (t = keeper->stuff; t; t = t2) {
    t2 = t->nextThing;
    i = dynamic_cast<TObj *>(t);
    if (!i)
      continue;
    if (ch->canSee(i)) {
      if ((i->obj_flags.cost > 1) &&
          !i->isObjStat(ITEM_NEWBIE) &&
#if NO_DAMAGED_ITEMS_SHOP
          (i->getMaxStructPoints() == i->getStructPoints()) &&
#endif
          trade_with(i, shop_nr)) {
        found = FALSE;
        for (k = 0; (k < cond_obj_vec.size() && !found); k++) {
          if (cond_obj_vec.size() > 0) {
            if (i->isSimilar(cond_obj_vec[k])) {
              if (!shop_producing(cond_obj_vec[k], shop_nr)) {
                cond_tot_vec[k] += 1;
                found = TRUE;
              } else {
                delete i;
                i = NULL;
                found = TRUE;
              }
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
        if (shop_index[shop_nr].in_room != 562 || shopOwned(shop_nr)) {
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
    strcpy(buf2, "Nothing!\n\r");
    sb += buf2;

    if (ch->desc)
      ch->desc->page_string(sb.c_str(), SHOWNOW_NO, ALLOWREP_YES);

    keeper->autoCreateShop(shop_nr);
    sprintf(buf2, "%s/%d", SHOPFILE_PATH, shop_nr);
    keeper->saveItems(buf2);
    return;
  }
  if (ch->desc) {
    if (!ch->desc->m_bIsClient)
      ch->desc->page_string(sb.c_str(), SHOWNOW_NO, ALLOWREP_YES);
    else 
      ch->desc->page_string(sb.c_str(), SHOWNOW_NO, ALLOWREP_YES);
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

  if (stuff)  // just can't see the shopkeepers inventory so lists nada?
    return;

  vlogf(LOG_MISC,"Creating a new shopfile for %s (shop #%d)",getName(),shop_nr);
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
  char *tmp_desc;
  int value;
  TObj *temp1;

  if (!*arg) 
    return FALSE;   // generic: look

  if (!(is_ok(keeper, ch, shop_nr)) || !ch->desc)
    return FALSE;

  TThing *t_temp1 = searchLinkedListVis(ch, arg, keeper->stuff);
  temp1 = dynamic_cast<TObj *>(t_temp1);
  if (!temp1) {
    // check for 4.xxx syntax, we already know anything like that is NOT
    // in shopkeepers possession
    if (strchr(arg, '.'))
      return FALSE;
    value = atoi(arg);
    if (!value || 
    !(temp1 = get_num_obj_in_list(ch, value, keeper->stuff, shop_nr))) {
      // it's not one of my objects so see if the look thing is in room
      return FALSE;
    }
  }
  string str = "You examine ";
  if (shop_producing(temp1, shop_nr)) {
    str += temp1->getNameForShow(true, false, ch).c_str();
  } else
    str += temp1->getName();
  str += " sold by $N.";

  act(str.c_str(), FALSE, ch, temp1, keeper, TO_CHAR);

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

  if (!(is_ok(keeper, ch, shop_nr)) || !ch->desc)
    return FALSE;

  if (!(num = getabunch(arg, newarg)))
    strcpy(newarg, arg);

  if (!num)
    num = 1;

  TThing *t_temp1 = searchLinkedListVis(ch, newarg, keeper->stuff);
  temp1 = dynamic_cast<TObj *>(t_temp1);
  if (!temp1) {
    if (!(temp1 = get_num_obj_in_list(ch, atoi(newarg), keeper->stuff, shop_nr))) {
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
  char buf[100];

  switch (shop_index[shop_nr].temper2) {
    case 0:
      sprintf(buf, "%s Don't ever try that again!", ch->name);
      keeper->doTell(buf);
      return;
    case 1:
      sprintf(buf, "%s Scram - midget!", ch->name);
      keeper->doTell(buf);
      return;

    default:
      return;
  }
}

void waste_shop_file(int shop_nr)
{
  char buf[1024];

  sprintf(buf, "%s/%d", SHOPFILE_PATH, shop_nr);
  unlink(buf);
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
    for(t=myself->roomp->stuff;t;t=t->nextThing){
      if((tbt=dynamic_cast<TBeing *>(t)) && 
	 tbt->getTimer()>1 && !tbt->isImmortal()){
        if ((tbt->master) && tbt->master->inRoom() == tbt->inRoom()) {
          //vlogf(LOG_DASH, "saving %s from loitering code, master is %s, room is (%d == %d)",tbt->getName(),
          //      tbt->master->getName(), tbt->inRoom(), tbt->master->inRoom());
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
    vlogf(LOG_BUG, "Warning... shop # for mobile %d (real nr) not found.", (myself)->number);
    return FALSE;
  }

  //    if(shopOwned(shop_nr)){
  //   vlogf(LOG_PEEL, "shop_nr %i, charged tax", shop_nr);
  //    }



  if (cmd == CMD_GENERIC_INIT) {
    if (!myself->isUnique()) {
      vlogf(LOG_BUG, "Warning!  %s attempted to be loaded, when not unique.", myself->getName());
      return TRUE;
    } else
      return FALSE;
  } else if (cmd == CMD_GENERIC_CREATED) {
    // Little kludge I put in to set pawnman is set for rent stuff - Russ 
    if (myself->mobVnum() == MOB_PAWNGUY) {
      vlogf(LOG_MISC, "Setting Pawn Broker pointer for rent functions!");
      pawnman = myself;
    }
    load_shop_file(myself, shop_nr);
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
  } else if (cmd == CMD_MOB_MOVED_INTO_ROOM && 
         (myself->in_room == shop_index[shop_nr].in_room)) {
    if (dynamic_cast<TBeing *>(ch->riding)) {
      char buf[256];
      sprintf(buf, "Hey, get that damn %s out of my shop!", 
          fname(ch->riding->name).c_str());
      myself->doSay(buf);
      act("You throw $N out.", FALSE, myself, 0, ch, TO_CHAR);
      act("$n throws you out of $s shop.", FALSE, myself, 0, ch, TO_VICT);
      act("$n throws $N out of $s shop.", FALSE, myself, 0, ch, TO_NOTVICT);
      --(*ch->riding);
      thing_to_room(ch->riding, (int) o);
      --(*ch);
      thing_to_room(ch, (int) o);
      return TRUE;
    } else if (dynamic_cast<TBeing *>(ch->rider)) {
      --(*ch->rider);
      thing_to_room(ch->rider, (int) o);
      --(*ch);
      thing_to_room(ch, (int) o);
      return TRUE;
    }
    return FALSE;
  } else if (cmd == CMD_MOB_ALIGN_PULSE) {
    // called on a long period....
    // have items in shop slowly repair themselves...
    TThing *t, *t2;
    if (::number(0,10))
      return FALSE;
    for (t = myself->stuff; t; t = t2) {
      t2 = t->nextThing;
      TObj * obj = dynamic_cast<TObj *>(t);
      if (!obj)
        continue;
      if (!::number(0,99) && !shop_producing(obj, shop_nr) &&
	  !shopOwned(shop_nr)) {
        // random recycling
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
	  !shopOwned(shop_nr)) {
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
    only_argument(arg, argm);

    if ((myself) == get_char_room(argm, ch->in_room)) {
      shopping_kill(arg, (ch), myself, shop_nr);
      return TRUE;
    }
    return FALSE;
  }
  if ((cmd == CMD_CAST) || (cmd == CMD_RECITE) || 
      (cmd == CMD_USE) || (cmd == CMD_PRAY)) {
    char tString[256];

    if (myself->canSee(ch)) {
      sprintf(tString, "%s <r>No magic here - kid!<z>", ch->getNameNOC(ch).c_str());
      myself->doTell(tString);
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

#if 0
  // the sweepers should be reasonably efficient about cleaning up, so this
  // probably isn't needed.  Non-GH might still suffer though....

  if ((cmd == CMD_DROP) && (ch->in_room == shop_index[shop_nr].in_room)) {
    // possible alternative would be to move dropped stuff to ROOM_DONATION
    act("$N tells you, 'HEY!  Don't clutter up my shop'.",
        FALSE, ch, 0, myself, TO_CHAR);
    return TRUE;
  }
#endif

  if(cmd == CMD_WHISPER /*&& ch->isImmortal()*/ ){
    char buf[256], buf2[256];
    TThing *tt;
    int count=0, value=0, price=0, discount=100, rc;
    unsigned int access=0;
    bool owned=shopOwned(shop_nr);
    unsigned int i, tmp;
    TObj *o;
    MYSQL_RES *res;
    MYSQL_ROW row;

    arg = one_argument(arg, buf);
    if(!is_abbrev(buf, myself->getName()))
      return FALSE;

    if((rc=dbquery(&res, "sneezy", "shop_keeper", "select access from shopownedaccess where shop_nr=%i and name='%s'", shop_nr, ch->getName()))==-1){
      vlogf(LOG_BUG, "Database error in shop_keeper");
      return FALSE;
    }
    if((row=mysql_fetch_row(res))){
      access=atoi(row[0]);
    }
    mysql_free_result(res);

    if(ch->isImmortal())
      access=SHOPACCESS_OWNER;

    arg = one_argument(arg, buf);
    
    if(!strcmp(buf, "info")){ /////////////////////////////////////////
      if(owned && !(access & SHOPACCESS_OWNER) && !(access & SHOPACCESS_INFO)){
	sprintf(buf, "%s Sorry, you don't have access to do that.", ch->getName());
	myself->doTell(buf);
	return FALSE;
      }

      for(tt=myself->stuff;tt;tt=tt->nextThing){
	o=dynamic_cast<TObj *>(tt);
	++count;
	value+=o->obj_flags.cost;
	price+=o->shopPrice(1, shop_nr, -1, &discount);
      }
      sprintf(buf, "%s I have %i talens and %i items worth %i talens and selling for %i talens.", ch->getName(), myself->getMoney(), count, value, price);
      myself->doTell(buf);
      sprintf(buf, "%s That puts my total value at %i talens.",
	      ch->getName(), myself->getMoney()+value);
      myself->doTell(buf);
      
      if(!owned){
	sprintf(buf, "%s This shop is for sale, however the King charges a sales tax and an ownership fee.", ch->getName());
	myself->doTell(buf);
	sprintf(buf, "%s That puts the sale price at %i.", ch->getName(),
		(int)((myself->getMoney()+value)*1.15)+1000000);
	myself->doTell(buf);
      } 

      sprintf(buf, "%s My profit_buy is %f and my profit_sell is %f.",
	      ch->getName(), shop_index[shop_nr].profit_buy,
	      shop_index[shop_nr].profit_sell);
      myself->doTell(buf);

      sprintf(buf, "%s I deal in", ch->getName());
      for(i=0;i<shop_index[shop_nr].type.size();++i){
	tmp=shop_index[shop_nr].type[i];
	if((int)tmp != -1)
	  sprintf(buf+strlen(buf), " %s,",
		  ItemInfo[tmp]->name);
      }
      buf[strlen(buf)-1]='\0';
      myself->doTell(buf);

    } else if(!strcmp(buf, "set")){ //////////////////////////////////
      if(!(access & SHOPACCESS_OWNER) && !(access & SHOPACCESS_PROFITS)){
	sprintf(buf, "%s Sorry, you don't have access to do that.", ch->getName());
	myself->doTell(buf);
	return FALSE;
      }
      arg = one_argument(arg, buf);
      
      if(!strcmp(buf, "profit_buy")){
	if(atof(arg)>5){
	  sprintf(buf, "%s Because of fraud regulations, I can't set the profit_buy higher than 5!", ch->getName());
	  myself->doTell(buf);
	  return FALSE;
	}

	shop_index[shop_nr].profit_buy=atof(arg);

	if((rc=dbquery(&res, "sneezy", "shop_keeper", "update shopowned set profit_buy=%f where shop_nr=%i", shop_index[shop_nr].profit_buy, shop_nr))){
	  if(rc==-1){
	    vlogf(LOG_BUG, "Database error in shop_keeper");
	    return FALSE;
	  }
	}

	sprintf(buf, "%s Ok, my profit_buy is now %f", 
		ch->getName(), shop_index[shop_nr].profit_buy);
	myself->doTell(buf);
      } else if(!strcmp(buf, "profit_sell")){ 
	shop_index[shop_nr].profit_sell=atof(arg);

	if((rc=dbquery(&res, "sneezy", "shop_keeper", "update shopowned set profit_sell=%f where shop_nr=%i", shop_index[shop_nr].profit_sell, shop_nr))){
	  if(rc==-1){
	    vlogf(LOG_BUG, "Database error in shop_keeper");
	    return FALSE;
	  }
	}

	sprintf(buf, "%s Ok, my profit_sell is now %f", 
		ch->getName(), shop_index[shop_nr].profit_sell);
	myself->doTell(buf);
      } else {
	sprintf(buf, "%s Sorry, I don't understand.  You can set either my profit_buy or profit_sell values.", ch->getName());
	myself->doTell(buf);
      }
    } else if(!strcmp(buf, "buy")){ /////////////////////////////////
#if 0
      if(!ch->isImmortal()){
	sprintf(buf, "%s Shop ownership is in beta testing right now, you can not purchase this shop.", ch->getName());
	myself->doTell(buf);
	return TRUE;
      }
#endif

      if(owned){
	sprintf(buf, "%s Sorry, this shop isn't for sale.", ch->getName());
	myself->doTell(buf);
	return TRUE;
      }
      
      for(tt=myself->stuff;tt;tt=tt->nextThing){
	o=dynamic_cast<TObj *>(tt);
	value+=o->obj_flags.cost;
      }
      value+=myself->getMoney();
      value=(int)(value*1.15)+1000000;
      if(ch->getMoney()<value){
	sprintf(buf, "%s Sorry, you can't afford this shop.  The price is %i.",
		ch->getName(), value);
	myself->doTell(buf);
	return TRUE;
      }
      ch->setMoney(ch->getMoney()-value);
      

      if((rc=dbquery(&res, "sneezy", "shop_keeper", "insert into shopowned (shop_nr, profit_buy, profit_sell) values (%i, %f, %f)", shop_nr, shop_index[shop_nr].profit_buy, shop_index[shop_nr].profit_sell))){
	if(rc==-1)
	  vlogf(LOG_BUG, "Database error in shop_keeper");
	return FALSE;
      }

      if((rc=dbquery(&res, "sneezy", "shop_keeper", "insert into shopownedaccess (shop_nr, name, access) values (%i, '%s', %i)", shop_nr, ch->getName(), SHOPACCESS_OWNER))){
	if(rc==-1)
	  vlogf(LOG_BUG, "Database error in shop_keeper");
	return FALSE;
      }

      myself->saveItems(buf);
            
      sprintf(buf, "%s Congratulations, you now own this shop.",
	      ch->getName());
      myself->doTell(buf);
    } else if(!strcmp(buf, "sell")){ //////////////////////////////////
      // don't let the shop owner do it by default, to prevent accidental sells
      if(/* !(access & SHOPACCESS_OWNER) && */ !(access & SHOPACCESS_SELL)){
	sprintf(buf, "%s Sorry, you don't have access to do that.", ch->getName());
	myself->doTell(buf);
	return FALSE;
      }

      if((rc=dbquery(&res, "sneezy", "shop_keeper", "delete from shopowned where shop_nr=%i", shop_nr))){
	if(rc){
	  vlogf(LOG_BUG, "Database error in shop_keeper");
	  return FALSE;
	}
      }

      if((rc=dbquery(&res, "sneezy", "shop_keeper", "delete from shopownedaccess where shop_nr=%i", shop_nr))){
	if(rc){
	  vlogf(LOG_BUG, "Database error in shop_keeper");
	  return FALSE;
	}
      }

      for(tt=myself->stuff;tt;tt=tt->nextThing){
	o=dynamic_cast<TObj *>(tt);
	value+=o->obj_flags.cost;
      }
      value+=myself->getMoney();
      ch->setMoney(ch->getMoney()+value);

      shop_index[shop_nr].profit_buy=1.1;
      shop_index[shop_nr].profit_sell=0.9;

      sprintf(buf, "%s Ok, you no longer own this shop.", ch->getName());
      myself->doTell(buf);
    } else if(!strcmp(buf, "give")){ /////////////////////////////
      if(!(access & SHOPACCESS_OWNER) && !(access & SHOPACCESS_GIVE)){
	sprintf(buf, "%s Sorry, you don't have access to do that.", ch->getName());
	myself->doTell(buf);
	return FALSE;
      }

      arg = one_argument(arg, buf);
      int amount=atoi(buf);

      if(myself->getMoney()>=amount){
	myself->setMoney(myself->getMoney()-amount);
	myself->saveChar(ROOM_AUTO_RENT);
	ch->setMoney(ch->getMoney()+amount);
	ch->saveChar(ROOM_AUTO_RENT);

	shoplog(shop_nr, ch, myself, "talens", amount, "receiving");

	sprintf(buf, "$n gives you %d talen%s.\n\r", amount,
		(amount == 1) ? "" : "s");
	act(buf, TRUE, myself, NULL, ch, TO_VICT);
	act("$n gives some money to $N.", 1, myself, 0, ch, TO_NOTVICT);
      } else {
	sprintf(buf, "%s I don't have that many talens.", ch->getName());
	myself->doTell(buf);
	sprintf(buf, "%s I have %i talens.", ch->getName(),myself->getMoney());
	myself->doTell(buf);
      }
    } else if(!strcmp(buf, "access")){ ////////////////////////////
      if(!(access & SHOPACCESS_OWNER) && !(access & SHOPACCESS_ACCESS)){
	sprintf(buf, "%s Sorry, you don't have access to do that.", ch->getName());
	myself->doTell(buf);
	return FALSE;
      }

      arg = one_argument(arg, buf);
      arg = one_argument(arg, buf2);

      if(*buf2){ // set value
	if((rc=dbquery(&res, "sneezy", "shop_keeper", "delete from shopownedaccess where shop_nr=%i and name='%s'", shop_nr, buf))){
	  if(rc==-1)
	    vlogf(LOG_BUG, "Database error in shop_keeper");
	  return FALSE;
	}
	if((rc=dbquery(&res, "sneezy", "shop_keeper", "insert into shopownedaccess (shop_nr, name, access) values (%i, '%s', %i)", shop_nr, buf, atoi(buf2)))){
	  if(rc==-1)
	    vlogf(LOG_BUG, "Database error in shop_keeper");
	  return FALSE;
	}
      } else if(*buf){
	if((rc=dbquery(&res, "sneezy", "shop_keeper", "select access from shopownedaccess where shop_nr=%i and name='%s'", shop_nr, buf))==-1){
	  vlogf(LOG_BUG, "Database error in shop_keeper");
	  return FALSE;
	}
	if((row=mysql_fetch_row(res))){
	  access=atoi(row[0]);

	  sprintf(buf2, "%s Access for %s is set to %i, commands/abilities:", ch->getName(),
		  buf, access);

          if(access>=SHOPACCESS_LOGS){
	    access-=SHOPACCESS_LOGS;
	    sprintf(buf2+strlen(buf2), " logs");
	  }
          if(access>=SHOPACCESS_ACCESS){
	    access-=SHOPACCESS_ACCESS;
	    sprintf(buf2+strlen(buf2), " access");
	  }
          if(access>=SHOPACCESS_SELL){
	    access-=SHOPACCESS_SELL;
	    sprintf(buf2+strlen(buf2), " sell");
	  }
          if(access>=SHOPACCESS_GIVE){
	    access-=SHOPACCESS_GIVE;
	    sprintf(buf2+strlen(buf2), " give");
	  }
          if(access>=SHOPACCESS_PROFITS){
	    access-=SHOPACCESS_PROFITS;
	    sprintf(buf2+strlen(buf2), " set");
	  }
          if(access>=SHOPACCESS_INFO){
	    access-=SHOPACCESS_INFO;
	    sprintf(buf2+strlen(buf2), " info");
	  }
          if(access>=SHOPACCESS_OWNER){
	    access-=SHOPACCESS_OWNER;
	    sprintf(buf2+strlen(buf2), " owner");
	  }


	  myself->doTell(buf2);
	}
	mysql_free_result(res);
      }
    } else if(!strcmp(buf, "logs")){ /////////////////////////////////////////
      if(!(access & SHOPACCESS_OWNER) && !(access & SHOPACCESS_LOGS)){
	sprintf(buf, "%s Sorry, you don't have access to do that.", ch->getName());
	myself->doTell(buf);
	return FALSE;
      }
      MYSQL_ROW row;
      MYSQL_RES *res;
      int rc;
      string sb;

      if(!strcmp(arg, " clear")){
	dbquery(NULL, "sneezy", "shop_keeper logs clear", "delete from shoplog where shop_nr=%i", shop_nr);
	ch->sendTo("Done.\n\r");
      } else if(!strcmp(arg, " summaries")){
	rc=dbquery(&res, "sneezy", "shop_keeper logs summaries", "select name, action, sum(talens) tsum from shoplog where shop_nr=%i group by name, action order by tsum desc", shop_nr);

	while((row=mysql_fetch_row(res))){
	  sprintf(buf, "%-12.12s %-10.10s %8i\n\r", 
		  row[0], row[1], atoi(row[2]));
	  sb += buf;
	}
	mysql_free_result(res);

	//////////
	sb += "\n\r";

	rc=dbquery(&res, "sneezy", "shop_keeper logs summaries", "select item, action, sum(talens) tsum from shoplog where shop_nr=%i group by item, action order by tsum desc", shop_nr);

	while((row=mysql_fetch_row(res))){
	  sprintf(buf, "%-32.32s %-10.10s %8i\n\r", 
		  row[0], row[1], atoi(row[2]));
	  sb += buf;
	}
	mysql_free_result(res);

	/////////
	sb += "\n\r";

	rc=dbquery(&res, "sneezy", "shop_keeper logs summaries", "select action, sum(talens) tsum from shoplog where shop_nr=%i group by action order by tsum desc", shop_nr);

	while((row=mysql_fetch_row(res))){
	  sprintf(buf, "%-12.12s %8i\n\r", 
		  row[0], atoi(row[1]));
	  sb += buf;
	}

	if (ch->desc)
	  ch->desc->page_string(sb.c_str(), SHOWNOW_NO, ALLOWREP_YES);

	mysql_free_result(res);
      } else {
	rc=dbquery(&res, "sneezy", "shop_keeper logs", "select name, action, item, talens, shoptalens, shopvalue, logtime from shoplog where shop_nr=%i and action!='paying tax' order by logtime desc, shoptalens+shopvalue desc", shop_nr);
	
	while((row=mysql_fetch_row(res))){
	  sprintf(buf, "%s  Talens: %8i  Value: %8i  Total: %8i\n\r", row[6], atoi(row[4]), atoi(row[5]), atoi(row[4])+atoi(row[5]));
	  sb += buf;
	  
	  sprintf(buf, "%-12.12s %-10.10s %-32.32s for %8i talens.\n\r\n\r",
		  row[0], row[1], row[2], atoi(row[3]));
	  sb += buf;
	}
	
	if (ch->desc)
	  ch->desc->page_string(sb.c_str(), SHOWNOW_NO, ALLOWREP_YES);
	
	mysql_free_result(res);
      }
    } else {
      myself->doTell("Read 'help shop owner' for assistance.");
    }


    return TRUE;
  }

  return FALSE;
}

void shoplog(int shop_nr, TBeing *ch, TMonster *keeper, const char *name, int cost, const char *action){
  int rc, value=0, count=0;
  MYSQL_RES *res;
  TThing *tt;
  TObj *o;  

  if(!shopOwned(shop_nr)){
    return;
  }

  for(tt=keeper->stuff;tt;tt=tt->nextThing){
    ++count;
    o=dynamic_cast<TObj *>(tt);
    value+=o->obj_flags.cost;
  }


  if((rc=dbquery(&res, "sneezy", "shoplog", "insert ignore into shoplog values (%i, '%s', '%s', '%s', %i, %i, %i, now(), %i)", shop_nr, ch->getName(), action, name, cost, keeper->getMoney(), value, count))){
    if(rc==-1){
      vlogf(LOG_BUG, "Database error in shoplog");
    }
  }
  mysql_free_result(res);      

}


#if !USE_SQL
 void bootTheShops()
{
  char *buf=0;
  int temp;
  FILE *shop_f;

  if (!(shop_f = fopen(SHOP_FILE, "r"))) {
    perror("Error in boot shop.\n");
    exit(0);
  }

  for (;;) {
    buf = fread_string(shop_f);
    if (*buf == '#') {          /* a new shop */

      shopData sd;
      fscanf(shop_f, "%d \n", &temp);
      while(temp>=0){
	sd.producing.push_back(real_object(temp));
	fscanf(shop_f, "%d \n", &temp);
      }
      sd.producing.push_back(-1);
      fscanf(shop_f, "%f \n", &sd.profit_buy);
      fscanf(shop_f, "%f \n", &sd.profit_sell);

      fscanf(shop_f, "%d \n", &temp);
      while(temp>=0){
	sd.type.push_back(temp);
	fscanf(shop_f, "%d \n", &temp);
      }      
      sd.type.push_back(-1);
      fscanf(shop_f, "%d \n", &temp);
      while(temp>=0){
	sd.mat_type.push_back(temp);
	fscanf(shop_f, "%d \n", &temp);
      }      
      sd.mat_type.push_back(MAX_OBJ_TYPES);
      sd.no_such_item1 = fread_string(shop_f);
      sd.no_such_item2 = fread_string(shop_f);
      sd.do_not_buy = fread_string(shop_f);
      sd.missing_cash1 = fread_string(shop_f);
      sd.missing_cash2 = fread_string(shop_f);
      sd.message_buy = fread_string(shop_f);
      sd.message_sell = fread_string(shop_f);
      fscanf(shop_f, "%d \n", &sd.temper1);
      fscanf(shop_f, "%d \n", &sd.temper2);
      fscanf(shop_f, "%d \n", &sd.keeper);

      sd.keeper = real_mobile(sd.keeper);

      fscanf(shop_f, "%u \n", &sd.flags);
      fscanf(shop_f, "%d \n", &sd.in_room);
      fscanf(shop_f, "%d \n", &sd.open1);
      fscanf(shop_f, "%d \n", &sd.close1);
      fscanf(shop_f, "%d \n", &sd.open2);
      fscanf(shop_f, "%d \n", &sd.close2);

      shop_index.push_back(sd);
    } else if (*buf == '$') {
      /* EOF */
      delete [] buf;
      break;
    }

    delete [] buf;
  }
  fclose(shop_f);
}
#else
void bootTheShops()
{
  int shop_nr;
  MYSQL_RES *res, *producing_res, *type_res, *material_res, *owned_res;
  MYSQL_ROW row, producing_row, type_row, material_row, owned_row;
  MYSQL *producing_db, *type_db, *material_db, *owned_db;

  /****** producing ******/
  producing_db=mysql_init(NULL);
  if(!mysql_real_connect(producing_db, NULL, "sneezy", NULL, 
	  (gamePort!=PROD_GAMEPORT ? "sneezybeta" : "sneezy"), 0, NULL, 0)){
    vlogf(LOG_BUG, "Could not connect (1) to database 'sneezy'.");
    exit(0);
  }

  if(mysql_query(producing_db, "select shop_nr, producing from shopproducing order by shop_nr")){
    vlogf(LOG_BUG, "Database query failed: %s\n", mysql_error(producing_db));
    exit(0);
  }
  producing_res=mysql_use_result(producing_db);
  producing_row=mysql_fetch_row(producing_res);

  /****** type ******/
  type_db=mysql_init(NULL);
  if(!mysql_real_connect(type_db, NULL, "sneezy", NULL, 
	  (gamePort!=PROD_GAMEPORT ? "sneezybeta" : "sneezy"), 0, NULL, 0)){
    vlogf(LOG_BUG, "Could not connect (1) to database 'sneezy'.");
    exit(0);
  }

  if(mysql_query(type_db, "select shop_nr, type from shoptype order by shop_nr")){
    vlogf(LOG_BUG, "Database query failed: %s\n", mysql_error(type_db));
    exit(0);
  }
  type_res=mysql_use_result(type_db);
  type_row=mysql_fetch_row(type_res);

  /****** material ******/
  material_db=mysql_init(NULL);
  if(!mysql_real_connect(material_db, NULL, "sneezy", NULL, 
	  (gamePort!=PROD_GAMEPORT ? "sneezybeta" : "sneezy"), 0, NULL, 0)){
    vlogf(LOG_BUG, "Could not connect (1) to database 'sneezy'.");
    exit(0);
  }

  if(mysql_query(material_db, "select shop_nr, mat_type from shopmaterial order by shop_nr")){
    vlogf(LOG_BUG, "Database query failed: %s\n", mysql_error(material_db));
    exit(0);
  }
  material_res=mysql_use_result(material_db);
  material_row=mysql_fetch_row(material_res);

  /****** owned ******/
  owned_db=mysql_init(NULL);
  if(!mysql_real_connect(owned_db, NULL, "sneezy", NULL, 
	  (gamePort!=PROD_GAMEPORT ? "sneezybeta" : "sneezy"), 0, NULL, 0)){
    vlogf(LOG_BUG, "Could not connect (1) to database 'sneezy'.");
    exit(0);
  }

  if(mysql_query(owned_db, "select shop_nr, profit_buy, profit_sell from shopowned order by shop_nr")){
    vlogf(LOG_BUG, "Database query failed: %s\n", mysql_error(owned_db));
    exit(0);
  }
  owned_res=mysql_use_result(owned_db);
  owned_row=mysql_fetch_row(owned_res);

  if(dbquery(&res, "sneezy", "bootTheShops", "select shop_nr, no_such_item1, no_such_item2, do_not_buy, missing_cash1, missing_cash2, message_buy, message_sell, temper1, temper2, keeper, flags, in_room, open1, close1, open2, close2, profit_buy, profit_sell from shop order by shop_nr")){
    vlogf(LOG_BUG, "Database error: bootTheShops");
    exit(0);
  }

  while((row=mysql_fetch_row(res))){
    shopData sd;

    shop_nr=atoi(row[0]);
    sd.no_such_item1 = mud_str_dup(row[1]);
    sd.no_such_item2 = mud_str_dup(row[2]);
    sd.do_not_buy = mud_str_dup(row[3]);
    sd.missing_cash1 = mud_str_dup(row[4]);
    sd.missing_cash2 = mud_str_dup(row[5]);
    sd.message_buy = mud_str_dup(row[6]);
    sd.message_sell = mud_str_dup(row[7]);
    sd.temper1=atoi(row[8]);
    sd.temper2=atoi(row[9]);
    sd.keeper=real_mobile(atoi(row[10]));
    sd.flags=atoi(row[11]);
    sd.in_room=atoi(row[12]);
    sd.open1=atoi(row[13]);
    sd.close1=atoi(row[14]);
    sd.open2=atoi(row[15]);
    sd.close2=atoi(row[16]);

    if(owned_row && (atoi(owned_row[0]))==shop_nr){
      sd.profit_buy=atof(owned_row[1]);
      sd.profit_sell=atof(owned_row[2]);
      owned_row=mysql_fetch_row(owned_res);
    } else {
      sd.profit_buy=atof(row[17]);
      sd.profit_sell=atof(row[18]);
    }

    while(type_row && atoi(type_row[0])==shop_nr){
      sd.type.push_back(atoi(type_row[1]));
      type_row=mysql_fetch_row(type_res);
    }
    
    while(producing_row && atoi(producing_row[0])==shop_nr){
      sd.producing.push_back(real_object(atoi(producing_row[1])));
      producing_row=mysql_fetch_row(producing_res);
    }
    sd.producing.push_back(-1);
    
    while(material_row && atoi(material_row[0])==shop_nr){
      sd.mat_type.push_back(atoi(material_row[1]));
      material_row=mysql_fetch_row(material_res);
    }
    sd.mat_type.push_back(MAX_OBJ_TYPES);

    shop_index.push_back(sd);
  }  

  mysql_free_result(res);
  mysql_free_result(producing_res);
  mysql_close(producing_db);
  mysql_free_result(type_res);
  mysql_close(type_db);
  mysql_free_result(material_res);
  mysql_close(material_db);
}

#endif

bool safe_to_save_shop_stuff(TMonster *ch)
{

  if (mob_index[ch->getMobIndex()].number < 1) {
     vlogf(LOG_BUG, "Shopkeeper #%d got safe_to_save_shop_stuff called when none in world!",
            mob_index[ch->getMobIndex()].virt);
    ch->doSay("I'm not functioning properly.  Tell a god to check the logs, case 1.");
    return FALSE;
  }
  if (mob_index[ch->getMobIndex()].number > 1) {
    vlogf(LOG_BUG, "More than one shopkeeper #%d in world.  Now the shop won't work!",
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
    vlogf(LOG_BUG, "  Error opening the shop file for shop #%s", cFname);
    return;
  }
  if (fread(&ucVersion, sizeof(ucVersion), 1, fp) != 1) {
    vlogf(LOG_BUG, "Error reading version from %s.", fileName);
    fclose(fp);
    return;
  }

  if (!noteLimitedItems(fp, fileName, ucVersion, FALSE))
    vlogf(LOG_BUG, "  Unable to count limited items in file  %s", fileName);
  fclose(fp);
}


void processShopFiles(void)
{
   dirwalk(SHOPFILE_PATH, processShopFile);
}

void loadShopPrices(void)
{
#if SHOP_PRICES_FLUXUATE
  FILE *fp;

  if ((fp = fopen( SHOP_PRICING, "r+b")) == NULL) {
    vlogf(LOG_BUG, "No shop pricing data exists.  creating.");
    if ((fp = fopen( SHOP_PRICING, "w+b")) == NULL) {
      vlogf(LOG_BUG, "Could not create pricing data file");
      exit(0);
    }
    fclose(fp);
    return;
  }

  unsigned char version;
  if (fread(&version, sizeof(version), 1, fp) != 1) {
    vlogf(LOG_BUG, "Serious error in shopprice read (version).");
    // we effectively start dynamic shop prices over
    return;
  }

  struct shop_pricing sp;

  for (;;) {
    if (fread(&sp, sizeof(struct shop_pricing), 1, fp) != 1) {
      if (feof(fp) != 0)
        break;
      vlogf(LOG_BUG, "Error in fread of loadShopPrices()");
      break;
    }
    ShopPriceIndex.push_back(sp);
  }
  fclose(fp);
#endif
  return;
}

void saveShopPrices(void)
{
#if SHOP_PRICES_FLUXUATE
  FILE *fp;

  if ((fp = fopen( SHOP_PRICING, "w+b")) == NULL) {
    vlogf(LOG_BUG, "Error saving shop pricing data.");
    return;
  }
  // write out version
  unsigned char version = SHOP_FILE_VERSION;
  fwrite(&version, sizeof(version), 1, fp);

  int i, maxsize = ShopPriceIndex.size();
  for (i = 0; i < maxsize; i++) {
    if (fwrite(&ShopPriceIndex[i], sizeof(struct shop_pricing), 1, fp) != 1) {
      vlogf(LOG_BUG, "Error in fwrite of saveShopPrices()");
    }
  }
  fclose(fp);
#endif
}

// adjusts the shop price based on structure
int TObj::adjPrice(int *discount) const
{
  int value = ((getMaxStructPoints() <= 0) ? getShopPrice(discount) :
               (int) (getShopPrice(discount) * 
                getStructPoints() / getMaxStructPoints()));

  return value;
}

int TObj::shop_price(int *discount) const
{
  int base_price = obj_flags.cost;

#if SHOP_PRICES_FLUXUATE
  int i;
  for (i = 0; i < ShopPriceIndex.size(); i++) {
    if (objVnum() == ShopPriceIndex[i].obj_vnum) {
      int sum_buy_sell = ShopPriceIndex[i].rel_sold + ShopPriceIndex[i].rel_bought;
      int total_traded = ShopPriceIndex[i].num_sold + ShopPriceIndex[i].num_bought;
      double real_price = (double) base_price;
      real_price += (double) ((double) sum_buy_sell / (double) total_traded);

      long durat = time(0) - ShopPriceIndex[i].last_touch_buy;
      if (durat >= 30 * SECS_PER_REAL_DAY)
        *discount = 40;
      else if (durat >= 20 * SECS_PER_REAL_DAY)
        *discount = 50;
      else if (durat >= 14 * SECS_PER_REAL_DAY)
        *discount = 60;
      else if (durat >= 10 * SECS_PER_REAL_DAY)
        *discount = 70;
      else if (durat >= 7 * SECS_PER_REAL_DAY)
        *discount = 80;
      else if (durat >= 5 * SECS_PER_REAL_DAY)
        *discount = 90;
      else
        *discount = 100;

      // if we are selling as many as we take in, raise price
      switch (ShopPriceIndex[i].num_sold * 100 / 
                max(1, ShopPriceIndex[i].num_bought)) {
        case 95:
        case 96:
        case 97:
        case 98:
        case 99:
        case 100:
        case 101:
        case 102:
        case 103:
        case 104:
        case 105:
          *discount *= 12;
          *discount /= 10;
          break;
        case 106:
        case 107:
        case 108:
        case 109:
        case 110:
        case 94:
        case 93:
        case 92:
        case 91:
        case 90:
          *discount *= 11;
          *discount /= 10;
          break;
        default:
          break;
      }
      real_price *= *discount;
      real_price /= 100;

#if FLUX_SHOP_DEBUG
      char *tmstr;
      vlogf(LOG_MISC, "%s had shop-price: %d, set at: %d", 
                getName(), (int) real_price, base_price);

      tmstr = asctime(localtime(&ShopPriceIndex[i].last_touch_buy));
      *(tmstr + strlen(tmstr) - 1) = '\0';
      vlogf(LOG_MISC, "sold: %d, bought %d, last sale: %s",
         ShopPriceIndex[i].num_sold, ShopPriceIndex[i].num_bought, tmstr);
#endif
      return max(1, (int) real_price);
    }
  }
#endif
  *discount = 100;
  return base_price;
}

int TObj::getShopPrice(int *discount) const
{
  return obj_flags.cost;
}

#if 0
int TSymbol::getShopPrice(int *discount) const
{
  return shop_price(discount);
}
#endif

int TSymbol::getShopPrice(int *) const
{
  return obj_flags.cost;
}

// bought is TRUE if PC is buying something
void TObj::genericCalc(bool bought, int cost)
{
#if SHOP_PRICES_FLUXUATE
  int i;

#if NO_DAMAGED_ITEMS_SHOP
  // item was sold as scrap, if we allow this to affect price, it will
  // cause prices to be dragged way down
  if (getStructPoints() != getMaxStructPoints())
    return;
#endif

  for (i = 0; i < ShopPriceIndex.size(); i++) {
    if (objVnum() == ShopPriceIndex[i].obj_vnum) {
      if (bought) {
        ShopPriceIndex[i].num_bought++;
        ShopPriceIndex[i].rel_bought += cost - obj_flags.cost;
        ShopPriceIndex[i].last_touch_buy = time(0);
      } else {
        ShopPriceIndex[i].num_sold++;
        ShopPriceIndex[i].rel_sold += cost - obj_flags.cost;
        ShopPriceIndex[i].last_touch_sell = time(0);
      }
      saveShopPrices();
      return;
    }
  }
  
  // item was not found, add new entry
  
  shop_pricing sp;
  sp.obj_vnum = objVnum();
  sp.num_bought = 0;
  sp.rel_bought = 0;
  sp.num_sold = 0;
  sp.rel_sold = 0;
  sp.last_touch_buy = time(0);
  sp.last_touch_sell = time(0);

  if (bought) {
    sp.num_bought++;
    sp.rel_bought += cost - obj_flags.cost;
  } else {
    sp.num_sold++;
    sp.rel_sold += cost - obj_flags.cost;
  }
  ShopPriceIndex.push_back(sp);
  saveShopPrices();
  return;
#endif
}

void TSymbol::recalcShopData(int bought, int cost)
{
  genericCalc(bought, cost);
}

shopData::shopData() :
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

  unsigned int max_mat_type=t.type.size();
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

  temper1 = t.temper1;
  temper2 = t.temper2;
  keeper = t.keeper;
  flags = t.flags;
  in_room = t.in_room;
  open1 = t.open1;
  open2 = t.open2;
  close1 = t.close1;
  close2 = t.close2;

  return *this;
}







