//////////////////////////////////////////////////////////////////////////
//
// SneezyMUD - All rights reserved, SneezyMUD Coding Team
//
//////////////////////////////////////////////////////////////////////////


// commodity.cc

#include "stdsneezy.h"
#include "shop.h"
#include "obj_commodity.h"
#include "shopowned.h"

TCommodity::TCommodity() :
  TObj()
{
}

TCommodity::TCommodity(const TCommodity &a) :
  TObj(a)
{
}

TCommodity & TCommodity::operator=(const TCommodity &a)
{
  if (this == &a) return *this;
  TObj::operator=(a);
  return *this;
}

TCommodity::~TCommodity()
{
}

void TCommodity::assignFourValues(int, int, int, int)
{
}

void TCommodity::getFourValues(int *x1, int *x2, int *x3, int *x4) const
{
  *x1 = 0;
  *x2 = 0;
  *x3 = 0;
  *x4 = 0;
}

sstring TCommodity::statObjInfo() const
{
  sstring a("");
  return a;
}

void TCommodity::lowCheck()
{
  if (!isname("commodity", name)) {
    vlogf(LOG_LOW, fmt("raw material without COMMODITY in name (%s : %d)") % 
               getName() % objVnum());
  }
  if (!numUnits()) {
    vlogf(LOG_LOW, fmt("raw material needs weight above 0.0 (%s : %d)") % 
               getName() % objVnum());
  }
  if (!pricePerUnit()) {
    vlogf(LOG_LOW, fmt("raw material needs price above 0 (%s : %d)") % 
               getName() % objVnum());
  }

  TObj::lowCheck();
}

int TCommodity::sellPrice(int num, int shop_nr, float, const TBeing *ch)
{
  int cost_per;
  int price;

  cost_per = pricePerUnit();
  price = (int) (numUnits() * cost_per * shop_index[shop_nr].getProfitSell(this,ch));

  if (obj_flags.cost <= 1) {
    price = max(0, price);
  } else {
    price = max(1, price);
  }

  return price;
}

int TCommodity::shopPrice(int num, int shop_nr, float, const TBeing *ch) const
{
  int cost_per;
  int price;

  cost_per = pricePerUnit();
  price = (int) (num * cost_per * shop_index[shop_nr].getProfitBuy(this, ch));
  price = max(1, price);

  return price;
}

int TCommodity::buyMe(TBeing *ch, TMonster *keeper, int num, int shop_nr)
{
  char buf[256];
  char buf2[80];
  int price, cost_per, vnum;
  TObj *obj2;

  if (parent != keeper) {
    vlogf(LOG_BUG, "Error: buy_treasure():shop.cc  obj not on keeper");
    return -1;
  }
  if (!shop_index[shop_nr].willBuy(this)) {
    keeper->doTell(ch->getName(), shop_index[shop_nr].do_not_buy);
    return -1;
  }
  if (num > (int) (numUnits())) {
    num = (int) (numUnits());
    keeper->doTell(ch->getName(), fmt("I don't have that much %s.  Here's the %d that I do have.") % fname(name) % num);
  }
  cost_per = pricePerUnit();
  price = shopPrice(num, shop_nr, -1, ch);
  vnum = objVnum();

  if (ch->getMoney() < price) {
    keeper->doTell(ch->name, shop_index[shop_nr].missing_cash2);

    switch (shop_index[shop_nr].temper1) {
      case 0:
        keeper->doAction(ch->name, CMD_SMILE);
        return -1;
      case 1:
        act("$n grins happily.", 0, keeper, 0, 0, TO_ROOM);
        return -1;
      default:
        return -1;
    }
  }
  strcpy(buf2, fname(name).c_str());
  --(*this);
  int num2 = (int) (numUnits()) - num;
  if (num2) {
    describeTreasure(buf2, num2, cost_per);
    *keeper += *this;
  } else {
    delete this;
  }

  if (num) {
    obj2 = read_object(vnum, VIRTUAL);
    obj2->describeTreasure(buf2, num, cost_per);
    *ch += *obj2;
    keeper->doTell(ch->getName(), fmt("Here ya go.  That's %d units of %s.") %
		   num % buf2);
    act("$n buys $p.", TRUE, ch, obj2, keeper, TO_NOTVICT);

    ch->giveMoney(keeper, price, GOLD_COMM);
    shoplog(shop_nr, ch, keeper, obj2->getName(), price, "buying");

    TShopOwned tso(shop_nr, keeper, ch);
    tso.doReserve();


  } else {
    // this happens with sub zero weight components
    vlogf(LOG_BUG, fmt("Bogus num %d in buyMe component at %d.  wgt=%.2f") %  num % ch->in_room % getWeight());
  }

  sprintf(buf, "%s/%d", SHOPFILE_PATH, shop_nr);
  keeper->saveItems(buf);
  ch->doSave(SILENT_YES);
  return price;
}

void TCommodity::sellMe(TBeing *ch, TMonster *keeper, int shop_nr, int)
{
  TThing *t;
  TCommodity *obj2 = NULL;
  int num, price;
  char buf[256], buf2[80];

  strcpy(buf2, fname(name).c_str());
  price = sellPrice(1, shop_nr, -1, ch);

  if (isObjStat(ITEM_NODROP)) {
    ch->sendTo("You can't let go of it, it must be CURSED!\n\r");
    return;
  }
  if (isObjStat(ITEM_PROTOTYPE)) {
    ch->sendTo("That's a prototype, no selling that!\n\r");
    return;
  }
  if (will_not_buy(ch, keeper, this, shop_nr))
    return;

  if (!shop_index[shop_nr].willBuy(this)) {
    keeper->doTell(ch->getName(), shop_index[shop_nr].do_not_buy);
    return;
  }
  if (keeper->getMoney() < price) {
    keeper->doTell(ch->getName(), shop_index[shop_nr].missing_cash1);
    return;
  }
  for (t = keeper->getStuff(); t; t = t->nextThing) {
    obj2 = dynamic_cast<TCommodity *>(t);
    if (!obj2)
      continue;

    if (obj2->objVnum() == objVnum())
      break;
  }
  if (!t) {
    TObj *to = read_object(objVnum(), VIRTUAL);
    obj2 = dynamic_cast<TCommodity *>(to);
    obj2->setWeight(0.0);
  } else
    --(*obj2);
  num = obj2->numUnits() + numUnits();
  num = max(min(num, 10000), 0);

  if (num) {
    int cost_per = pricePerUnit();
    obj2->describeTreasure(buf2, num, cost_per);
    *keeper += *obj2;
    --(*this);

    keeper->giveMoney(ch, price, GOLD_COMM);
    shoplog(shop_nr, ch, keeper, obj2->getName(), -price, "selling");

    TShopOwned tso(shop_nr, keeper, ch);
    tso.doReserve();


    keeper->doTell(ch->getName(), fmt("Thanks, here's your %d talens.") % price);
    act("$n sells $p.", TRUE, ch, this, 0, TO_ROOM);
    if (ch->isAffected(AFF_GROUP) && ch->desc &&
            IS_SET(ch->desc->autobits, AUTO_SPLIT) &&
            (ch->master || ch->followers)){
      sprintf(buf, "%d", price);
      ch->doSplit(buf, false);
    }
    delete this;
  }
  ch->doSave(SILENT_YES);
  sprintf(buf, "%s/%d", SHOPFILE_PATH, shop_nr);
  keeper->saveItems(buf);
  return;
}

int TCommodity::sellCommod(TBeing *ch, TMonster *keeper, int shop_nr, TThing *bag)
{
  int rc;

  if (equippedBy)
    *ch += *ch->unequip(eq_pos);

  if (bag && bag != ch) {
    rc = get(ch, this, bag, GETOBJOBJ, true);
    if (IS_SET_DELETE(rc, DELETE_ITEM)) {
      return DELETE_THIS;
    }
    if (IS_SET_DELETE(rc, DELETE_VICT)) {
      return DELETE_ITEM;
    }
    if (IS_SET_DELETE(rc, DELETE_THIS)) {
      return DELETE_VICT;
    }
    if (!dynamic_cast<TBeing *>(parent)) {
      // could fail on volume or sumthing
      return FALSE;
    }
  }

  sellMe(ch, keeper, shop_nr, 1);
  return FALSE;
}

void TCommodity::valueMe(TBeing *ch, TMonster *keeper, int shop_nr, int)
{
  int price;
  char buf2[80];

  strcpy(buf2, fname(name).c_str());
  price = sellPrice(1, shop_nr, -1, ch);

  if (!shop_index[shop_nr].willBuy(this)) {
    keeper->doTell(ch->getName(), shop_index[shop_nr].do_not_buy);
    return;
  }

  keeper->doTell(ch->getName(), "Hmm, I'd give you %d talens for that.", price);
  return;
}

const sstring TCommodity::shopList(const TBeing *ch, const sstring &arg, int min_amt, int max_amt, int, int, int k, unsigned long int) const
{
  char buf[256];
  int cost = pricePerUnit();

  sprintf(buf, "[%2d] COMMODITY: %-20.20s  : %5d units    %5d talens (per unit)\n\r",
            k + 1, fname(name).c_str(),
            (int) (numUnits()), (int) cost);
  if (arg.empty() && min_amt == 999999)     /* everything */
  /* specific item */
    return buf;
  else if (isname(arg, name) && min_amt == 999999)
    return buf;
  /* specific item and specific cost */
  else if (isname(arg, name) && cost >= min_amt && cost <= max_amt)
    return buf;
  else if (arg.empty() && cost >= min_amt && cost <= max_amt)   /* specific cost */
    return buf;
  else
    return "";
}

int TCommodity::numUnits() const
{
  return (int) (10.0 * getWeight());
}

int TCommodity::pricePerUnit() const
{
  int num = numUnits();
  int price = (int) (num ? obj_flags.cost / num : 0);

  if (obj_flags.cost) {
    price = max(price, 1);
//    obj_flags.cost = price * num;  // just correct it
  }

  return price;
}
