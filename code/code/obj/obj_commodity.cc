//////////////////////////////////////////////////////////////////////////
//
// SneezyMUD - All rights reserved, SneezyMUD Coding Team
//
//////////////////////////////////////////////////////////////////////////


// commodity.cc

#include <cmath>

#include "extern.h"
#include "low.h"
#include "monster.h"
#include "materials.h"
#include "shop.h"
#include "obj_commodity.h"
#include "shopowned.h"
#include "database.h"

// maximum number of units shop will hold. price=0 at this quantity
const int shop_capacity = 100000;
const float max_commod_price = 250;

bool TCommodity::willMerge(TMergeable *tm)
{
  TCommodity *tCommod;

  if(!(tCommod=dynamic_cast<TCommodity *>(tm)) ||
     tCommod==this ||
     tCommod->getMaterial() != getMaterial() ||
     disableMerge ||
     tCommod->disableMerge)
    return false;

  return true;
}

void TCommodity::doMerge(TMergeable *tm)
{
  TCommodity *tCommod;
  
  if(!(tCommod=dynamic_cast<TCommodity *>(tm)) || !willMerge(tm))
    return;

  setWeight(getWeight() + tCommod->getWeight());
  updateDesc();

  --(*tCommod);
  delete tCommod;
}


bool TCommodity::splitMe(TBeing *ch, const sstring &tString)
{
  int         tCount = 0,
              tValue = 0;
  TCommodity *tCommod;
  sstring      tStString(""),
              tStBuffer("");


  tStString=tString.word(0);
  tStBuffer=tString.word(1);

  if (tString.empty() || ((tCount = convertTo<int>(tStBuffer)) <= 0)) {
    ch->sendTo("Syntax: split <commodity> <units>\n\r");
    return true;
  }

  if (tCount >= numUnits()) {
    ch->sendTo(format("Units must be between 1 and %d.\n\r") %
               (numUnits()-1));
    return true;
  }

  if (!obj_flags.cost || objVnum() < 0) {
    ch->sendTo("This commodity is special, it can not be split up.\n\r");
    return true;
  }

  if ((tValue = real_object(objVnum())) < 0 || 
      tValue > (signed) obj_index.size() ||
      !(tCommod = dynamic_cast<TCommodity *>(read_object(tValue, REAL)))) {
    ch->sendTo("For some reason that commodity resists being split up.\n\r");
    return true;
  }

  tCommod->setMaterial(MAT_UNDEFINED); // this is to prevent merge
  *ch += *tCommod;

  tCommod->setMaterial(getMaterial());
  tCommod->setWeight(tCount/10.0);
  setWeight(getWeight()-(tCount/10.0));
  tCommod->updateDesc();
  updateDesc();

  act("You split $N into two pieces.",
      FALSE, ch, this, this, TO_CHAR);
  act("$n splits $N into two pieces.",
      FALSE, ch, this, this, TO_ROOM);

  return true;
}


TCommodity::TCommodity() :
  TMergeable()
{
  disableMerge = false;
}

TCommodity::TCommodity(const TCommodity &a) :
  TMergeable(a)
{
  disableMerge = false;
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
    vlogf(LOG_LOW, format("raw material without COMMODITY in name (%s : %d)") % 
               getName() % objVnum());
  }
  if (!numUnits()) {
    vlogf(LOG_LOW, format("raw material needs weight above 0.0 (%s : %d)") % 
               getName() % objVnum());
  }
  if (!pricePerUnit()) {
    vlogf(LOG_LOW, format("raw material needs price above 0 (%s : %d)") % 
               getName() % objVnum());
  }

  TObj::lowCheck();
}

float TCommodity::demandCurvePrice(int num, float price, int mat)
{
  return demandCurvePrice(num, price, mat, commod_index[mat]);
}

float TCommodity::demandCurvePrice(int num, float price, int mat, int total_units)
{
  total_units = max(1, min(shop_capacity, total_units));

  float multiplier=-(max_commod_price / log(shop_capacity));

  // now calculate the dynamic price at each level for the
  // requested number of units to be sold
  float total_price=0;
  for(int i=1;i<=abs(num);++i){
    // positive num means we're selling commodities, so our total units
    // goes DOWN with each one sold.  negative num means the opposite.
    if(num >= 0){
      if((total_units-i) <= 0) // edge case: small values of total_units, large values of num can return infinite
	      total_price+=max_commod_price;
      else
	      total_price+=(multiplier * log(total_units-i) + max_commod_price);
    } else {
      //if(!(total_units-i))
	    //  total_price+=0; // Don't set price floor - this causes edge cases when total_units == 1 and num == -1
      //else
	      total_price+=(multiplier * log(total_units+i) + max_commod_price);
    }
  }

  // commod fluctuations are capped which makes them slightly more predictable
  // ideally, we'd adjust the entire curve and not just trim the edges like this
  float max_price = abs(num)*material_nums[mat].price*10;
  float min_price = abs(num)*material_nums[mat].price/10;
  return max(min_price, min(max_price, total_price));

  // p = desired price
  // e = 2.718281828
  // N = e ^ (

  // N = e ^ ((p-max_price) / multiplier)


  // example prices:
  // max price = 1000, shop capacity = 100000
  // 100000 =  0.0
  //  90000 =  9.15
  //  75000 = 24.99 
  //  50000 = 60.21 
  //  25000 = 120.41
  //  10000 = 200.00
  //   5000 = 260.21
  //   1000 = 400.00
  //      1 = 1000.00

}

int TCommodity::sellPrice(int num, int shop_nr, float total_units /*hack to save db lookups*/, const TBeing *ch)
{
  float price;

  price = (pricePerUnit() * shop_index[shop_nr].getProfitSell(this,ch));

  if (total_units < 0)
  {
    TShopOwned tso(shop_nr, NULL);
    total_units = tso.getInventoryCount(this);
  }

  price=demandCurvePrice(-num, price, getMaterial(), int(total_units));

  if(price < 1)
    price=0;

  return (int)price;
}

float TCommodity::shopPriceFloat(int num, int shop_nr, float, const TBeing *ch) const
{
  float price;

  price = (pricePerUnit() * shop_index[shop_nr].getProfitBuy(this, ch));

  TShopOwned tso(shop_nr, NULL);

  int total_units = tso.getInventoryCount(this);

  price=demandCurvePrice(num, price, getMaterial(), total_units);

  return price;
}

int TCommodity::shopPrice(int num, int shop_nr, float, const TBeing *ch) const
{
  return max(1, (int)shopPriceFloat(num, shop_nr, 0, ch));
}

int TCommodity::buyMe(TBeing *ch, TMonster *keeper, int num, int shop_nr)
{
  int price, vnum;
  float cost_per;
  TObj *obj2;

  // note that commods have a constant volume, so this check isn't needed
  // however, this should be changed in the future, and when it is changed
  // someone will probably find this code trying to figure out why no one
  // has enough volume to buy commods. :)
  if ((ch->getCarriedVolume() + getTotalVolume()) > ch->carryVolumeLimit()) {
    ch->sendTo(format("%s: You can't carry that much volume.\n\r") % fname(name));
    return -1;
  }
  // obj-weight > free ch limit
  if (compareWeights(num/10.0,
       (ch->carryWeightLimit() - ch->getCarriedWeight())) == -1) {
    ch->sendTo(format("%s: You can't carry that much weight.\n\r") % fname(name));
    return -1;
  }


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
    keeper->doTell(ch->getName(), format("I don't have that much %s.  Here's the %d that I do have.") % fname(name) % num);
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

  --(*this);
  int num2 = (int) (numUnits()) - num;
  
  if (num2) {
    setWeight(num2/10.0);
  } else {
    setWeight(0);
  }

  if (num) {
    obj2 = read_object(vnum, VIRTUAL);
    obj2->setWeight(num/10.0);
    obj2->setMaterial(getMaterial());
    *ch += *obj2;
    keeper->doTell(ch->getName(), format("That'll be %i.  Here's %d units of %s.") %
		   price % num % material_nums[getMaterial()].mat_name);
    act("$n buys $p.", TRUE, ch, obj2, keeper, TO_NOTVICT);

    TShopOwned tso(shop_nr, keeper, ch);
    tso.doBuyTransaction(price, getName(), TX_BUYING, obj2);

  } else {
    // this happens with sub zero weight components
    vlogf(LOG_BUG, format("Bogus num %d in buyMe commodity at %d.  wgt=%.2f") %  num % ch->in_room % getWeight());
  }

  keeper->saveItems(shop_nr);
  ch->doQueueSave();
  if (numUnits() > 0)
    keeper->saveItem(shop_nr, this);
  delete this;
  return price;
}

bool TCommodity::sellMeCheck(TBeing *ch, TMonster *keeper, int num, int total_inventory /* hack to save db lookups*/) const
{
  sstring buf;
  TShopOwned tso(keeper, ch);
  int max_num=shop_capacity;
  
  if(tso.isOwned())
    max_num=tso.getMaxNum(ch, this, shop_capacity);

  if(max_num == 0){
    keeper->doTell(ch->name, "I don't wish to buy any of those right now.");
    return TRUE;
  }

  if (total_inventory < 0)
    total_inventory = tso.getInventoryCount(this);
  
  if (total_inventory >= max_num) {
    keeper->doTell(ch->getName(), format("I already have plenty of %s.") % 
		   getName());
    return TRUE;
  } else if (total_inventory + num > max_num) {
    keeper->doTell(ch->getName(), format("I'll buy no more than %d unit%s of %s.") % (max_num - total_inventory) % (max_num - total_inventory > 1 ? "s" : "") % getName());
    return TRUE;
  }
  
  return false;
}

int TCommodity::sellMe(TBeing *ch, TMonster *keeper, int shop_nr, int)
{
  int num, price, rent_id = -1, units = 0;
  char buf[256];
  TDatabase db(DB_SNEEZY);

  db.query("select rent_id, weight*10 as units from rent where owner_type='shop' and owner=%i and material=%i and vnum=%i",
	   shop_nr, getMaterial(), Obj::GENERIC_COMMODITY);

  if(db.fetchRow()){
    rent_id=convertTo<int>(db["rent_id"]);
    units=convertTo<int>(db["units"]);
  }

  price = sellPrice(numUnits(), shop_nr, float(units), ch);

  if (isObjStat(ITEM_NODROP)) {
    ch->sendTo("You can't let go of it, it must be CURSED!\n\r");
    return false;
  }
  if (isObjStat(ITEM_PROTOTYPE)) {
    ch->sendTo("That's a prototype, no selling that!\n\r");
    return false;
  }
  if (will_not_buy(ch, keeper, this, shop_nr)){
    return false;
  }
  if (sellMeCheck(ch, keeper, numUnits(), units)){
    return false;
  }
  if (!shop_index[shop_nr].willBuy(this)) {
    keeper->doTell(ch->getName(), shop_index[shop_nr].do_not_buy);
    return false;
  }
  if (keeper->getMoney() < price) {
    keeper->doTell(ch->getName(), shop_index[shop_nr].missing_cash1);
    return false;
  }

  num = units + numUnits();

  if (num > 0) {

    --(*this);

    TShopOwned tso(shop_nr, keeper, ch);
    tso.doSellTransaction(price, getName(), TX_SELLING);

    keeper->doTell(ch->getName(), format("Thanks, here's your %d talens.") % price);
    act("$n sells $p.", TRUE, ch, this, 0, TO_ROOM);
    if (ch->isAffected(AFF_GROUP) && ch->desc &&
            IS_SET(ch->desc->autobits, AUTO_SPLIT) &&
            (ch->master || ch->followers)){
      sprintf(buf, "%d", price);
      ch->doSplit(buf, false);
    }
    if (rent_id < 0)
    {
      TObj *to = read_object(objVnum(), VIRTUAL);
      TCommodity *obj2 = dynamic_cast<TCommodity *>(to);
      obj2->setWeight(num/10.0);
      obj2->setMaterial(getMaterial());
      keeper->saveItem(shop_nr, rent_id, obj2);
      delete obj2;
    }
    else
    {
      db.query("update rent set weight=(%i/10) where rent_id=%i", num, rent_id);
    }
  }
  else
    keeper->saveItems(shop_nr);

  ch->doQueueSave();
  return DELETE_THIS;
}

int TCommodity::sellCommod(TBeing *ch, TMonster *keeper, int shop_nr, TThing *bag)
{
  int rc;

  if (equippedBy)
    *ch += *ch->unequip(eq_pos);

  if (bag && bag != ch) {

    disableMerge = true;
    rc = get(ch, this, bag, GETOBJOBJ, true);
    disableMerge = false;

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

  rc=sellMe(ch, keeper, shop_nr, 1);
  if(IS_SET_DELETE(rc, DELETE_THIS))
    return DELETE_THIS;

  return FALSE;
}

void TCommodity::valueMe(TBeing *ch, TMonster *keeper, int shop_nr, int)
{
  int price;
  /*char buf2[80];
  TDatabase db(DB_SNEEZY);
  TObj *obj2;

  db.query("select rent_id from rent where owner_type='shop' and owner=%i and material=%i and vnum=%i",
	   shop_nr, getMaterial(), Obj::GENERIC_COMMODITY);

  if(db.fetchRow()){
    TObj *to=keeper->loadItem(shop_nr, convertTo<int>(db["rent_id"]));
    obj2 = dynamic_cast<TCommodity *>(to);
  } else {
    TObj *to = read_object(objVnum(), VIRTUAL);
    obj2 = dynamic_cast<TCommodity *>(to);
    obj2->setWeight(0.0);
    obj2->setMaterial(getMaterial());
  }
  *keeper += *obj2;
  strcpy(buf2, fname(name).c_str());*/

  if (!shop_index[shop_nr].willBuy(this)) {
    keeper->doTell(ch->getName(), shop_index[shop_nr].do_not_buy);
    //delete obj2;
    return;
  }

  price = sellPrice(numUnits(), shop_nr, -1, ch);

  keeper->doTell(ch->getName(), format("Hmm, I'd give you %d talens for your %i units of that.") % price % numUnits());

  //delete obj2;
  return;
}

const sstring TCommodity::shopList(const TBeing *ch, const sstring &arg, int min_amt, int max_amt, int, int shop_nr, int k, unsigned long int) const
{
  char buf[256];

  float cost = shopPriceFloat(1, shop_nr, -1, ch);

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
    vlogf(LOG_BUG, format("commodity '%s' has no price for material %i") %
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

  const char *miscname [] =
    {
      "bit",
      "tiny pile",
      "small pile",
      "pile",
      "big pile",
      "large pile",
      "huge pile"
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
    sprintf(buf, ((sstring)(format("commodity %s %s") % mineralname[sizeindex] %
	    material_nums[getMaterial()].mat_name)).c_str());
    name = mud_str_dup(buf);
    
    sprintf(buf, ((sstring)(format("a %s of rough %s") % mineralname[sizeindex] %
		  material_nums[getMaterial()].mat_name)).c_str());
    shortDescr = mud_str_dup(buf);
    
    sprintf(buf, ((sstring)(format("A %s of rough %s has been left here.  What luck!") %
		  mineralname[sizeindex] %
		  material_nums[getMaterial()].mat_name)).c_str());
    setDescr(mud_str_dup(buf));
  } else if (isMetal()) {
    sprintf(buf, ((sstring)(format("commodity %s %s") % metalname[sizeindex] %
	    material_nums[getMaterial()].mat_name)).c_str());
    name = mud_str_dup(buf);    

    sprintf(buf, ((sstring)(format("a %s of %s") % metalname[sizeindex] %
		  material_nums[getMaterial()].mat_name)).c_str());
    shortDescr = mud_str_dup(buf);
    
    sprintf(buf, ((sstring)(format("A %s of %s has been left here.  What luck!") %
		  metalname[sizeindex] %
		  material_nums[getMaterial()].mat_name)).c_str());
    setDescr(mud_str_dup(buf));
  } else {
    sprintf(buf, ((sstring)(format("commodity %s %s") % miscname[sizeindex] %
	    material_nums[getMaterial()].mat_name)).c_str());
    name = mud_str_dup(buf);    

    sprintf(buf, ((sstring)(format("a %s of %s") % miscname[sizeindex] %
		  material_nums[getMaterial()].mat_name)).c_str());
    shortDescr = mud_str_dup(buf);
    
    sprintf(buf, ((sstring)(format("A %s of %s has been left here.  What luck!") %
		  miscname[sizeindex] %
		  material_nums[getMaterial()].mat_name)).c_str());
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

void TCommodity::setMaterial(unsigned short num)
{
  TObj::setMaterial(num);

  //  if(!bootTime)
    updateDesc();
}
