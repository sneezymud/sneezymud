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
  float cost_per;
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
  float cost_per;
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
  int price, vnum;
  float cost_per;
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
    setWeight(num2/10.0);
    *keeper += *this;
  } else {
    delete this;
  }

  if (num) {
    obj2 = read_object(vnum, VIRTUAL);
    obj2->setWeight(num/10.0);
    *ch += *obj2;
    keeper->doTell(ch->getName(), fmt("Here ya go.  That's %d units of %s.") %
       num % buf2);
    act("$n buys $p.", TRUE, ch, obj2, keeper, TO_NOTVICT);

    TShopOwned tso(shop_nr, keeper, ch);
    tso.doBuyTransaction(price, getName(), TX_BUYING, obj2);

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
    obj2->setWeight(num/10.0);
    *keeper += *obj2;
    --(*this);

    TShopOwned tso(shop_nr, keeper, ch);
    tso.doSellTransaction(price, obj2->getName(), TX_SELLING, obj2);


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
  if (!ch->delaySave)
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

  keeper->doTell(ch->getName(), fmt("Hmm, I'd give you %d talens for that.") % price);
  return;
}

const sstring TCommodity::shopList(const TBeing *ch, const sstring &arg, int min_amt, int max_amt, int, int, int k, unsigned long int) const
{
  char buf[256];
  float cost = pricePerUnit();

  sprintf(buf, "[%2d] COMMODITY: %-20.20s  : %5d units    %.2f talens (per unit)\n\r",
            k + 1, material_nums[getMaterial()].mat_name,
            (int) (numUnits()),  cost);
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

float TCommodity::pricePerUnit() const
{
  /*
  if(!material_nums[getMaterial()].price)
    vlogf(LOG_BUG, fmt("commodity '%s' has no price for material %i") %
	  getName() % getMaterial());
  */

  return material_nums[getMaterial()].price;
}

int TCommodity::getSizeIndex() const
{
  int weight=numUnits();

  if(weight<=2){
    return 0;
  } else if(weight<=4){
    return 1;
  } else if(weight<=6){
    return 2;
  } else if(weight<=8){ 
    return 3;
  } else if(weight<=10){
    return 4;
  } else if(weight<=15){
    return 5;
  } else {
    return 6;
  }
}



void TCommodity::updateDesc()
{
  int sizeindex=getSizeIndex();
  char buf[256];
  
  const char *metalname [] =
  {
    "bit",
    "nugget",
    "ingot",
    "sovereign",
    "rod",
    "bar",
    "pile"
  };

  const char *mineralname [] =
  {
    "tiny piece",
    "small piece",
    "piece",
    "large piece",
    "huge piece",
    "gigantic piece",
    "massive piece"
  };

  
  if (isObjStat(ITEM_STRUNG)) {
    delete [] name;
    delete [] shortDescr;
    delete [] descr;

    extraDescription *exd;
    while ((exd = ex_description)) {
      ex_description = exd->next;
      delete exd;
    }
    ex_description = NULL;
    delete [] action_description;
    action_description = NULL;
  } else {
    addObjStat(ITEM_STRUNG);
    ex_description = NULL;
    action_description = NULL;
  }

  if(isMineral()){
    sprintf(buf, (fmt("commodity %s %s") % mineralname[sizeindex] %
	    material_nums[getMaterial()].mat_name).c_str());
    name = mud_str_dup(buf);
    
    sprintf(buf, (fmt("a %s of rough %s") % mineralname[sizeindex] %
		  material_nums[getMaterial()].mat_name).c_str());
    shortDescr = mud_str_dup(buf);
    
    sprintf(buf, (fmt("A %s of rough %s has been left here.  What luck!") %
		  mineralname[sizeindex] %
		  material_nums[getMaterial()].mat_name).c_str());
    setDescr(mud_str_dup(buf));
  } else {
    sprintf(buf, (fmt("commodity %s %s") % metalname[sizeindex] %
	    material_nums[getMaterial()].mat_name).c_str());
    name = mud_str_dup(buf);    

    sprintf(buf, (fmt("a %s of %s") % metalname[sizeindex] %
		  material_nums[getMaterial()].mat_name).c_str());
    shortDescr = mud_str_dup(buf);
    
    sprintf(buf, (fmt("A %s of %s has been left here.  What luck!") %
		  metalname[sizeindex] %
		  material_nums[getMaterial()].mat_name).c_str());
    setDescr(mud_str_dup(buf));
  }
  obj_flags.cost = suggestedPrice();

}

int TCommodity::suggestedPrice() const 
{
  return (int)(numUnits() * pricePerUnit());
};


void TCommodity::setWeight(const float w)
{
  TObj::setWeight(w);

  //  if(!bootTime)
    updateDesc();
}

void TCommodity::setMaterial(ubyte num)
{
  TObj::setMaterial(num);

  //  if(!bootTime)
    updateDesc();
}
