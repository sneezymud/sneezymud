#include "stdsneezy.h"
#include "shopowned.h"
#include "database.h"
#include "shop.h"
#include "corporation.h"

// this function relies on the fact that the db will return rows in the order
// that they were created, chronologically.  I'm not sure if this is defined
// behavior or not, so if it stops working, you need to put a timestamp value
// into the table and sort by that
bool sameAccount(sstring buf, int shop_nr){
  charFile st, stthis;

  load_char(buf, &stthis);

  TDatabase db(DB_SNEEZY);

  db.query("select name from shopownedaccess where shop_nr=%i", shop_nr);

  while(db.fetchRow()){
    if (!load_char(db["name"], &st))
      continue;

    if(!strcmp(stthis.aname, st.aname)){
      if(buf.lower() == sstring(db["name"]).lower())
	return FALSE;
      else
	return TRUE;
    }
  }

  return FALSE;
}

// obj is optional
void TShopOwned::doBuyTransaction(int cashCost, const sstring &name, 
			       const sstring &action, TObj *obj)
{
  // take the expense cut out
  if(!doExpenses(cashCost, obj))
    return;

  // buyer gives money to seller
  ch->giveMoney(keeper, cashCost, GOLD_SHOP);

  // log the sale
  shoplog(shop_nr, ch, keeper, name, cashCost, action);

  if(owned){
    doDividend(cashCost, name);
    doReserve();
    chargeTax(cashCost, name, obj);
  }
  
  // save
  keeper->saveItems(fmt("%s/%d") % SHOPFILE_PATH % shop_nr);
  ch->doSave(SILENT_YES);
}

bool TShopOwned::doExpenses(int cashCost, TObj *obj)
{
  double profit_buy=shop_index[shop_nr].getProfitBuy(obj, ch);
  double ratio=getExpenseRatio();
  int sba_nr=160;
  TMonster *sba;
  TBeing *t;
  double value;

  if(ratio == 0)
    return true;

  if(keeper->getMoney() < cashCost)
    return false;

// find the sba shopkeeper
  for(t=character_list;t;t=t->next){
    if(t->number==shop_index[sba_nr].keeper)
      break;
  }

  if(t && (sba=dynamic_cast<TMonster *>(t))){
    value=((double)cashCost/profit_buy) * ratio;
    
    keeper->giveMoney(sba, (int)value, GOLD_SHOP);
    shoplog(shop_nr, sba, keeper, "talens", (int)-value, "expenses");
    shoplog(sba_nr, keeper, sba, "talens", (int)value, "expenses");
    sba->saveItems(fmt("%s/%d") % SHOPFILE_PATH % shop_nr);
  }

  return true;
}


double TShopOwned::getExpenseRatio()
{
  double ratio=0;
  TDatabase db(DB_SNEEZY);

  db.query("select expense_ratio from shop where shop_nr=%i", shop_nr);
  
  if(db.fetchRow())
    ratio=convertTo<double>(db["expense_ratio"]);

  return ratio;
}


int TShopOwned::getPurchasePrice(int talens, int value){
  return (int)(((talens+value)*1.15)+1000000);
}


int getShopAccess(int shop_nr, TBeing *ch){
  int access=0;
  TDatabase db(DB_SNEEZY);

  db.query("select access from shopownedaccess where shop_nr=%i and upper(name)=upper('%s')", shop_nr, ch->getName());
  
  if(db.fetchRow())
    access=convertTo<int>(db["access"]);

#if 0  
  if(sameAccount(ch->getName(), shop_nr) && !ch->isImmortal() && access){
    ch->sendTo("Another character in your account has permissions at this shop, so this character can not use the ownership functions.\n\r");
    access=0;
  }
#endif  

  if(ch->isImmortal())
    access=SHOPACCESS_OWNER;
  
  return access;
}



TShopOwned::TShopOwned(int shop_nr, TMonster *keeper, TBeing *ch) :
  shop_nr(shop_nr),
  keeper(keeper),
  ch(ch)
{
  owned=shop_index[shop_nr].isOwned();
  access=getShopAccess(shop_nr, ch);
}

void TShopOwned::chargeTax(int cost, const sstring &name, TObj *o)
{
  int tax_office;
  TDatabase db(DB_SNEEZY);

  // no tax for messing with inventory if you own the shop
  if(hasAccess(SHOPACCESS_OWNER))
    return;

  db.query("select tax_nr from shopownedtax where shop_nr=%i", shop_nr);

  // no entry, no tax
  if(!db.fetchRow())
    return;

  tax_office=convertTo<int>(db["tax_nr"]);

  cost = (int)((float)cost * shop_index[tax_office].getProfitBuy(o, ch));
  
  TBeing *t;
  for(t=character_list;t;t=t->next){
    if(t->mobVnum()==mob_index[shop_index[tax_office].keeper].virt)
      break;
  }

  TBeing *taxman;
  if(!t || !(taxman=dynamic_cast<TMonster *>(t))){
    vlogf(LOG_PEEL, fmt("taxman not found %i") % 
	  shop_index[tax_office].keeper);
    return;
  }

  keeper->giveMoney(taxman, cost, GOLD_SHOP);
  keeper->saveItems(fmt("%s/%d") % SHOPFILE_PATH % shop_nr);
  dynamic_cast<TMonster *>(taxman)->saveItems(fmt("%s/%d") % 
					      SHOPFILE_PATH % tax_office);
  
  shoplog(shop_nr, keeper, keeper, name, 
	  -cost, "paying tax");
  shoplog(tax_office, keeper, dynamic_cast<TMonster *>(taxman),
	  name, cost, "tax");

  TShopOwned tso(tax_office, dynamic_cast<TMonster *>(taxman), keeper);
  tso.doReserve();
}



void TShopOwned::setReserve(sstring arg)
{
  TDatabase db(DB_SNEEZY);

  if(!hasAccess(SHOPACCESS_OWNER)){
    keeper->doTell(ch->getName(), "Sorry, you don't have access to do that.");
    return;
  }

  int min=convertTo<int>(arg.word(0));
  int max=convertTo<int>(arg.word(1));

  if(min > max || (max-min) < 100000){
    keeper->doTell(ch->getName(), "The minimum reserve must be less than the maximum reserve.");
    keeper->doTell(ch->getName(), "The two reserve values must be at least 100k apart.");
    return;
  }

  db.query("update shopowned set reserve_min=%i, reserve_max=%i where shop_nr=%i", min, max, shop_nr);

  keeper->doTell(ch->getName(), fmt("Ok, the minimum reserve is now %i and the maximum reserve is %i.") % min % max);
}

int TShopOwned::getMinReserve()
{
  TDatabase db(DB_SNEEZY);
  db.query("select reserve_min from shopowned where shop_nr=%i", shop_nr);

  if(db.fetchRow())
    return convertTo<int>(db["reserve_min"]);
  
  return 0;  
}

int TShopOwned::getMaxReserve()
{
  TDatabase db(DB_SNEEZY);
  db.query("select reserve_max from shopowned where shop_nr=%i", shop_nr);

  if(db.fetchRow())
    return convertTo<int>(db["reserve_max"]);
  
  return 0;  
}

void TShopOwned::doReserve()
{
  TCorporation corp(getCorpID());
  int min=getMinReserve();
  int max=getMaxReserve();
  int even=min + (int)((max-min)/2);
  int money=keeper->getMoney();
  int amt=0;
  TBeing *banker;
  int bank_nr=corp.getBank();

  if(min<=0 || max<=0 || min>max || (max-min)<100000)
    return;

  for(banker=character_list;banker;banker=banker->next){
    if(banker->number==shop_index[bank_nr].keeper)
      break;
  }
  if(!banker){
    vlogf(LOG_BUG, fmt("couldn't find banker for shop_nr=%i!") % bank_nr);
    return;
  }

  if(money < min){
    amt=even-money;

    if(corp.getMoney() < amt)
      amt=corp.getMoney();

    if(amt==0)
      return;


    corp.setMoney(corp.getMoney() - amt);
    corp.corpLog(keeper->getName(), "reserve", -amt);

    banker->giveMoney(keeper, amt, GOLD_SHOP);

    keeper->saveItems(fmt("%s/%d") % SHOPFILE_PATH % shop_nr);
    dynamic_cast<TMonster *>(banker)->saveItems(fmt("%s/%d") % SHOPFILE_PATH % shop_nr);
    shoplog(bank_nr, keeper, dynamic_cast<TMonster *>(banker), "talens", -amt, "reserve");

    shoplog(shop_nr, keeper, keeper, "talens", amt, "reserve");
  } else if(money > max){
    amt=money-even;

    if(amt==0)
      return;

    corp.setMoney(corp.getMoney() + amt);
    corp.corpLog(keeper->getName(), "reserve", amt);

    keeper->giveMoney(banker, amt, GOLD_SHOP);

    keeper->saveItems(fmt("%s/%d") % SHOPFILE_PATH % shop_nr);
    dynamic_cast<TMonster *>(banker)->saveItems(fmt("%s/%d") % SHOPFILE_PATH % shop_nr);
    
    shoplog(bank_nr, keeper,  dynamic_cast<TMonster *>(banker), "talens", amt, "reserve");

    shoplog(shop_nr, keeper, keeper, "talens", -amt, "reserve");
  }
}

double TShopOwned::getQuality()
{
  TDatabase db(DB_SNEEZY);

  db.query("select quality from shopownedrepair where shop_nr=%i", shop_nr);

  if(db.fetchRow())
    return convertTo<double>(db["quality"]);

  return -1;
}

double TShopOwned::getSpeed()
{
  TDatabase db(DB_SNEEZY);

  db.query("select speed from shopownedrepair where shop_nr=%i", shop_nr);

  if(db.fetchRow())
    return convertTo<double>(db["speed"]);

  return -1;
}


void TShopOwned::setQuality(sstring arg)
{
  TDatabase db(DB_SNEEZY);

  if(!hasAccess(SHOPACCESS_RATES)){
    keeper->doTell(ch->getName(), "Sorry, you don't have access to do that.");
    return;
  }

  double f=convertTo<double>(arg);

  if(f > 1.0 || f <= 0.0){
    keeper->doTell(ch->getName(), "The quality percentage must be less than or equal to 1.0 and greater than 0.0.");
    return;
  }

  db.query("update shopownedrepair set quality=%f where shop_nr=%i", f, shop_nr);

  keeper->doTell(ch->getName(), fmt("Ok, the quality percentage has been set to %f.") % f);
}



void TShopOwned::setSpeed(sstring arg)
{
  TDatabase db(DB_SNEEZY);

  if(!hasAccess(SHOPACCESS_RATES)){
    keeper->doTell(ch->getName(), "Sorry, you don't have access to do that.");
    return;
  }

  double f=convertTo<double>(arg);

  if(f > 5.0 || f <= 0.0){
    keeper->doTell(ch->getName(), "The speed modifier must be less than or equal to 1.0 and greater than 0.0.");
    return;
  }

  db.query("update shopownedrepair set speed=%f where shop_nr=%i", f, shop_nr);

  keeper->doTell(ch->getName(), fmt("Ok, the speed modifier has been set to %f.") % f);
}




void TShopOwned::doDividend(int cost, const sstring &name)
{
  if(getDividend()){
    int div=(int)((double)cost * getDividend());
    TCorporation corp(getCorpID());
    div=max(0, min(div,cost));
    TBeing *banker;
    int bank_nr=corp.getBank();

    for(banker=character_list;banker;banker=banker->next){
      if(banker->number==shop_index[bank_nr].keeper)
	break;
    }
    if(!banker){
      vlogf(LOG_BUG, fmt("couldn't find banker for shop_nr=%i!") % bank_nr);
      return;
    }

    keeper->giveMoney(banker, div, GOLD_SHOP);
    dynamic_cast<TMonster *>(banker)->saveItems(fmt("%s/%d") % SHOPFILE_PATH % shop_nr);
    shoplog(bank_nr, keeper,  dynamic_cast<TMonster *>(banker), "talens", div, "dividend");

    keeper->saveItems(fmt("%s/%d") % SHOPFILE_PATH % shop_nr);
    shoplog(shop_nr, ch, keeper, name, -div, "dividend");
    

    corp.setMoney(corp.getMoney() + div);
    corp.corpLog(keeper->getName(), "dividend", div);
  }
}


void TShopOwned::setDividend(sstring arg)
{
  TDatabase db(DB_SNEEZY);

  if(!hasAccess(SHOPACCESS_DIVIDEND)){
    keeper->doTell(ch->getName(), "Sorry, you don't have access to do that.");
    return;
  }

  double f=convertTo<double>(arg);

  if(f > 1.0 || f < 0.0){
    keeper->doTell(ch->getName(), "The dividend percentage must be less than or equal to 1.0 and greater than or equal to 0.0.");
    return;
  }

  db.query("update shopowned set dividend=%f where shop_nr=%i", f, shop_nr);

  keeper->doTell(ch->getName(), fmt("Ok, the dividend percentage has been set to %f.") % f);
}

double TShopOwned::getDividend()
{
  TDatabase db(DB_SNEEZY);
  db.query("select dividend from shopowned where shop_nr=%i", shop_nr);
  
  if(db.fetchRow())
    return convertTo<double>(db["dividend"]);
  
  return 0.0;
}



int TShopOwned::getCorpID()
{
  TDatabase db(DB_SNEEZY);

  db.query("select corp_id from shopowned where shop_nr=%i", shop_nr);

  if(db.fetchRow()){
    return convertTo<int>(db["corp_id"]);
  }
  return 0;
}


TShopOwned::~TShopOwned(){
}

bool TShopOwned::isOwned(){
  return owned;
}

bool TShopOwned::hasAccess(int perm){
  if(getCorpID()){
    TCorporation corp(getCorpID());
    access = access | corp.getAccess(ch);
  }    

  if(access & SHOPACCESS_OWNER &&
     !(perm & SHOPACCESS_SELL)) // must have explicit sell permission
    return true;

  return (access & perm);
}


void TShopOwned::showInfo()
{
  TThing *tt;
  TObj *o;
  int count=0, value=0, tmp=0;
  unsigned int i=0;
  sstring buf;
  TDatabase db(DB_SNEEZY);
  //  int volume=0;

  // if not owned, or owned and has "owner" or "info"
  if(!isOwned() || hasAccess(SHOPACCESS_INFO)){
    for(tt=keeper->getStuff();tt;tt=tt->nextThing){
      o=dynamic_cast<TObj *>(tt);
      ++count;
      //      volume+=o->getVolume();
      value+=o->obj_flags.cost;
      // shopPrice does db queries, it tends to be too slow here
      //      price+=o->shopPrice(1, shop_nr, -1, ch);
    }
    keeper->doTell(ch->getName(),
		   fmt("I have %i talens and %i items worth %i talens and selling for approximately %i talens.") %
		   keeper->getMoney() % count % value %
		   (int)(value * shop_index[shop_nr].profit_buy));

    //    keeper->doTell(ch->getName(), fmt("My inventory takes up %i cubic inches of space.") % volume);
    
    keeper->doTell(ch->getName(), fmt("That puts my total value at %i talens.") %
		   (keeper->getMoney()+value));

    if(getDividend() > 0)
      keeper->doTell(ch->getName(), fmt("My corporate dividend kickback is %f.") % getDividend());
    if(getMinReserve() > 0 || getMaxReserve() > 0)
      keeper->doTell(ch->getName(), fmt("My corporate reserve is %i-%i.") %
		     getMinReserve() % getMaxReserve());
    if(getExpenseRatio() > 0)
      keeper->doTell(ch->getName(), fmt("My expense ratio is %f.") % getExpenseRatio());

  }



  if(!isOwned()){
    keeper->doTell(ch->getName(), "This shop is for sale, however the King charges a sales tax and an ownership fee.");
    
    keeper->doTell(ch->getName(), fmt("That puts the sale price at %i.") %
		   getPurchasePrice(keeper->getMoney(), value));
  } else if(getCorpID()){
    TCorporation corp(getCorpID());
    keeper->doTell(ch->getName(), fmt("This shop is owned by %s.") %
		   corp.getName());
  }

  
  db.query("select r.name as name from room r, shopownedtax st, shop s where r.vnum=s.in_room and s.shop_nr=st.tax_nr and st.shop_nr=%i", shop_nr);

  if(db.fetchRow()){
    keeper->doTell(ch->getName(), fmt("This shop is taxed by %s.") % 
		   db["name"]);
  } else {
    keeper->doTell(ch->getName(), "This shop is untaxed.");
  }
  

  // repair stuff
  if((getQuality() >= 0 && getQuality() != 1) ||
     (getSpeed() >= 0 && getSpeed() != 1))
    keeper->doTell(ch->getName(), fmt("My quality percentage is %f and my speed modifier is %f.") % getQuality() % getSpeed());


  // anyone can see profit_buy, profit_sell and trading types, anytime
  if(keeper->spec==SPEC_LOAN_SHARK){
    keeper->doTell(ch->getName(),
		   fmt("My defaulting penalty is %f and my interest rate is %f.") %
		   shop_index[shop_nr].profit_buy %
		   shop_index[shop_nr].profit_sell);


    db.query("select x, y, term from shopownedloanrate where shop_nr=%i",
	     shop_nr);
    if(db.fetchRow()){
      keeper->doTell(ch->getName(), fmt("My offered term is %i years.") %
		     convertTo<int>(db["term"]));
      keeper->doTell(ch->getName(), fmt("My talens per level X value is %f and my max offering at level 50 is %f.") % 
		     convertTo<double>(db["x"]) % 
		     convertTo<double>(db["y"]));
    }
  } else if(keeper->spec==SPEC_BANKER){
    keeper->doTell(ch->getName(),
		   fmt("I pay out %f in yearly interest, compounded daily.") %
		   (shop_index[shop_nr].profit_sell));
    db.query("select a.talens+b.talens as talens from (select sum(talens) as talens from shopownedbank where shop_nr=%i) a, (select sum(talens) as talens from shopownedcorpbank where shop_nr=%i) b", shop_nr, shop_nr);
    if(db.fetchRow()){
      keeper->doTell(ch->getName(), fmt("My equity value is %i talens.") %
		     (keeper->getMoney()-convertTo<int>(db["talens"])));
    }
    


  } else {
    keeper->doTell(ch->getName(),
		   fmt("My profit_buy is %f and my profit_sell is %f.") %
		   shop_index[shop_nr].profit_buy %
		   shop_index[shop_nr].profit_sell);
    keeper->doTell(ch->getName(),fmt("My maximum inventory per item is %i.") %
		   getMaxNum(NULL));
  }


  if(shop_index[shop_nr].type.size()<=1){
    keeper->doTell(ch->getName(), 
		   "I only sell things, I do not buy anything.");
  } else {
    buf = "I deal in";
    for(i=0;i<shop_index[shop_nr].type.size();++i){
      tmp=shop_index[shop_nr].type[i];
      if(tmp != MAX_OBJ_TYPES && (int) tmp != -1)
	buf = fmt("%s %s,") % buf % ItemInfo[tmp]->name;
    }
    keeper->doTell(ch->getName(), buf);
  }
}


int TShopOwned::setRates(sstring arg)
{
  TDatabase db(DB_SNEEZY);
  sstring buf;
  float profit_buy, profit_sell;
  int max_num, argc=0;

  if(!hasAccess(SHOPACCESS_RATES)){
    keeper->doTell(ch->getName(), "Sorry, you don't have access to do that.");
    return FALSE;
  }

  // reset the price cache
  cached_shop_nr=-1;

  arg = one_argument(arg, buf);
  profit_buy = convertTo<float>(buf);
  if(buf != "")
    argc++;


  if(buf == "clear"){ /////////////////////////////////////////////
    arg = one_argument(arg, buf);
    if(buf == "all"){
      db.query("delete from shopownedratios where shop_nr=%i", shop_nr);
      db.query("delete from shopownedmatch where shop_nr=%i", shop_nr);
      db.query("delete from shopownedplayer where shop_nr=%i", shop_nr);
      keeper->doTell(ch->getName(), "Ok, I cleared all of the individual profit ratios.");
      return TRUE;
    } else if(buf == "player"){
      arg = one_argument(arg, buf);

      db.query("delete from shopownedplayer where shop_nr=%i and player='%s'",
	       shop_nr, buf.c_str());
      
      keeper->doTell(ch->getName(), "Done.");
      return TRUE;
    } else if(buf == "match"){
      arg = one_argument(arg, buf);

      db.query("delete from shopownedmatch where shop_nr=%i and match='%s'",
	       shop_nr, buf.c_str());
      
      keeper->doTell(ch->getName(), "Done.");
      return TRUE;
    } else {
      // find item in inventory matching keywords in arg
      // get vnum, then store in db
      TThing *tt = searchLinkedListVis(ch, buf, keeper->getStuff());
      
      if(!tt){
	keeper->doTell(ch->getName(), "I don't have that item.");
	return FALSE;
      }
      
      TObj *o=dynamic_cast<TObj *>(tt);

      db.query("delete from shopownedratios where shop_nr=%i and obj_nr=%i",
	       shop_nr, o->objVnum());
      
      keeper->doTell(ch->getName(), "Done.");
      return TRUE;
    }
  }


  arg = one_argument(arg, buf);
  profit_sell = convertTo<float>(buf);
  if(buf != "")
    argc++;

  arg = one_argument(arg, buf);
  max_num = convertTo<int>(buf);
  if(buf != "")
    argc++;
  
  arg = one_argument(arg, buf);
  if(buf != "")
    argc++;

  if(keeper->spec != SPEC_REPAIRMAN &&
     keeper->spec != SPEC_LOAN_SHARK &&
     keeper->spec != SPEC_RECEPTIONIST &&
     keeper->spec != SPEC_AUCTIONEER){
    if(profit_buy>5 || profit_buy<0 ||
       profit_sell>5 || profit_sell<0){
      keeper->doTell(ch->getName(), "Due to fraud regulations, I cannot set my profit_sell or profit_buy to more than 5 or less than 0.");
      return FALSE;
    }
    
    if(profit_buy < profit_sell){
      keeper->doTell(ch->getName(), "You can't set your buy profit lower than your sell profit, you'd lose all your money!");
      return FALSE;
    }
  }
  
  if(argc==0){
    db.query("select obj_nr, profit_buy, profit_sell, max_num from shopownedratios where shop_nr=%i", shop_nr);
    
    while(db.fetchRow()){
      keeper->doTell(ch->getName(), fmt("%f %f %i item %s") %
		     convertTo<float>(db["profit_buy"]) % 
		     convertTo<float>(db["profit_sell"]) % 
		     convertTo<int>(db["max_num"]) %
		     obj_index[real_object(convertTo<int>(db["obj_nr"]))].short_desc);
    }

    db.query("select match, profit_buy, profit_sell, max_num from shopownedmatch where shop_nr=%i", shop_nr);
    
    while(db.fetchRow()){
      keeper->doTell(ch->getName(),fmt( "%f %f %i match %s") %
		     convertTo<float>(db["profit_buy"]) % 
		     convertTo<float>(db["profit_sell"]) % 
		     convertTo<int>(db["max_num"]) %
		     db["match"]);
    }    

    db.query("select player, profit_buy, profit_sell, max_num from shopownedplayer where shop_nr=%i", shop_nr);
    
    while(db.fetchRow()){
      keeper->doTell(ch->getName(),fmt( "%f %f %i player %s") %
		     convertTo<float>(db["profit_buy"]) % 
		     convertTo<float>(db["profit_sell"]) % 
		     convertTo<int>(db["max_num"]) %
		     db["player"]);
    }    

    return TRUE;
  } else if(argc<4){
    keeper->doTell(ch->getName(), "I don't understand.");
    return FALSE;
  }
  
  if(buf == "default"){ ///////////////////////////////////////////////////
    shop_index[shop_nr].profit_buy=profit_buy;
    shop_index[shop_nr].profit_sell=profit_sell;
    
    db.query("update shopowned set profit_buy=%f, profit_sell=%f, max_num=%i where shop_nr=%i", shop_index[shop_nr].profit_buy, shop_index[shop_nr].profit_sell, max_num, shop_nr);
    
    keeper->doTell(ch->getName(), 
		   fmt("Ok, my profit_buy is now %f, my profit_sell is now %f and my max_num is now %i.") % 
		   shop_index[shop_nr].profit_buy %
		   shop_index[shop_nr].profit_sell % max_num);
  } else if(buf == "match"){ /////////////////////////////////////////////
    arg = one_argument(arg, buf);

    db.query("select 1 from shopownedmatch where shop_nr=%i and match='%s'",
	     shop_nr, buf.c_str());
    
    if(!db.fetchRow()){
      db.query("insert into shopownedmatch values (%i, '%s', %f, %f, %i)",
	       shop_nr, buf.c_str(), profit_buy, profit_sell, max_num);
    } else {
      db.query("update shopownedmatch set profit_buy=%f, profit_sell=%f, max_num=%i where shop_nr=%i and match='%s'", profit_buy, profit_sell, max_num, shop_nr, buf.c_str());
    }
    
    keeper->doTell(ch->getName(), fmt("Ok, my profit_buy is now %f, my profit_sell is now %f and my max_num is now %i, all for keyword %s.") %
		   profit_buy % profit_sell % max_num % buf);    
  } else if(buf == "player"){ ////////////////////////////////////////////
    arg = one_argument(arg, buf);

    db.query("select 1 from shopownedplayer where shop_nr=%i and player='%s'",
	     shop_nr, buf.c_str());
    
    if(!db.fetchRow()){
      db.query("insert into shopownedplayer values (%i, '%s', %f, %f, %i)",
	       shop_nr, buf.c_str(), profit_buy, profit_sell, max_num);
    } else {
      db.query("update shopownedplayer set profit_buy=%f, profit_sell=%f, max_num=%i where shop_nr=%i and player='%s'", profit_buy, profit_sell, max_num, shop_nr, buf.c_str());
    }
    
    keeper->doTell(ch->getName(), fmt("Ok, my profit_buy is now %f, my profit_sell is now %f and my max_num is now %i, all for player %s.") %
		   profit_buy % profit_sell % max_num % buf);    
  } else if(buf == "loanrate"){
    db.query("select 1 from shopownedloanrate where shop_nr=%i", shop_nr);

    if(!db.fetchRow()){
      db.query("insert into shopownedloanrate values (%i, %f, %f, %i)",
	       shop_nr, profit_buy, profit_sell, max_num);
    } else {
      db.query("update shopownedloanrate set x=%f, y=%f, term=%i where shop_nr=%i", profit_buy, profit_sell, max_num, shop_nr);
    }    

    keeper->doTell(ch->getName(), 
		   fmt("Ok, my loanrate X value is now %f, my Y value is now %f and my max term is %i.") % profit_buy % profit_sell % max_num);
  } else if(buf == "repair"){
    db.query("select 1 from shopownedrepair where shop_nr=%i", shop_nr);

    if(!db.fetchRow()){
      db.query("insert into shopownedrepair values (%i, %f, %f)",
	       shop_nr, profit_buy, profit_sell);
    } else {
      db.query("update shopownedrepair set quality=%f, speed=%f where shop_nr=%i", profit_buy, profit_sell, shop_nr);
    }    

    keeper->doTell(ch->getName(), 
		   fmt("Ok, my quality percentage is now %f and my speed modifier is now %f.") % profit_buy % profit_sell);

  } else { ////////////////////////////////////////////////////////////////
    // find item in inventory matching keywords in arg
    // get vnum, then store in db
    TThing *tt = searchLinkedListVis(ch, buf, keeper->getStuff());
    
    if(!tt){
      keeper->doTell(ch->getName(), "I don't have that item.");
      return FALSE;
    }
    
    TObj *o=dynamic_cast<TObj *>(tt);
    
    db.query("select 1 from shopownedratios where shop_nr=%i and obj_nr=%i",
	     shop_nr, o->objVnum());
    
    if(!db.fetchRow()){
      db.query("insert into shopownedratios values (%i, %i, %f, %f, %i)",
	       shop_nr, o->objVnum(), profit_buy, profit_sell, max_num);
    } else {
      db.query("update shopownedratios set profit_buy=%f, profit_sell=%f, max_num=%i where shop_nr=%i and obj_nr=%i", profit_buy, profit_sell, max_num, shop_nr, o->objVnum());
    }
    
    keeper->doTell(ch->getName(), fmt("Ok, my profit_buy is now %f, my profit_sell is now %f and my max_num is %i, all for %s.") %
		   profit_buy % profit_sell % max_num % o->getName());
  }
  
  return TRUE;
}


int TShopOwned::buyShop(sstring arg){
  int value=0;
  TDatabase db(DB_SNEEZY);
  sstring buf;
  TThing *tt;
  TObj *o;
  int corp_id=0;

  if(isOwned()){
    keeper->doTell(ch->getName(), "Sorry, this shop isn't for sale.");
    return TRUE;
  }
  
  for(tt=keeper->getStuff();tt;tt=tt->nextThing){
    o=dynamic_cast<TObj *>(tt);
    value+=o->obj_flags.cost;
  }
  value=getPurchasePrice(keeper->getMoney(), value);

  db.query("select corp_id from corpaccess where lower(name)='%s'",
	   sstring(ch->getName()).lower().c_str());

  if(arg.empty()){
    if(db.fetchRow())
      corp_id=convertTo<int>(db["corp_id"]);

    if(db.fetchRow()){
      keeper->doTell(ch->getName(), "You must specify the ID of the corporation you wish to buy this shop for.");
      return TRUE;
    }
  } else {
    if(convertTo<int>(arg) == 0){
      keeper->doTell(ch->getName(), "You must specify the ID of the corporation you wish to buy this shop for.");
      return TRUE;
    }

    while(db.fetchRow()){
      if(convertTo<int>(db["corp_id"]) == convertTo<int>(arg)){
	corp_id=convertTo<int>(arg);
	break;
      }
    }
  }

  if(!corp_id){
      keeper->doTell(ch->getName(), "You must specify the ID of the corporation you wish to buy this shop for.");
      return TRUE;
  }

  
  if(ch->getMoney()<value){
    keeper->doTell(ch->getName(), fmt("Sorry, you can't afford this shop.  The price is %i.") % value);
    return TRUE;
  }
  ch->setMoney(ch->getMoney()-value);
  
  
  db.query("insert into shopowned (shop_nr, profit_buy, profit_sell, corp_id) values (%i, %f, %f, %i)", shop_nr, shop_index[shop_nr].profit_buy, shop_index[shop_nr].profit_sell, corp_id);
  
  buf = fmt("%s/%d") % SHOPFILE_PATH % shop_nr;
  keeper->saveItems(buf);
  
  keeper->doTell(ch->getName(), "Congratulations, you now own this shop.");
  shop_index[shop_nr].owned=true;

  return TRUE;
}
  

int TShopOwned::setString(sstring arg)
{
  sstring which, s;

  s=one_argument(arg, which);
  
  if(!hasAccess(SHOPACCESS_OWNER)){
    keeper->doTell(ch->getName(), "Sorry, you don't have access to do that.");
    return FALSE;
  }


  if(which.empty() && s.empty()){
    keeper->doTell(ch->getName(), fmt("no_such_item1: %s") %
		   shop_index[shop_nr].no_such_item1);
    keeper->doTell(ch->getName(), fmt("no_such_item2: %s") %
		   shop_index[shop_nr].no_such_item2);
    keeper->doTell(ch->getName(), fmt("do_not_buy: %s") %
		   shop_index[shop_nr].do_not_buy);
    keeper->doTell(ch->getName(), fmt("missing_cash1: %s") %
		   shop_index[shop_nr].missing_cash1);
    keeper->doTell(ch->getName(), fmt("missing_cash2: %s") %
		   shop_index[shop_nr].missing_cash2);
    keeper->doTell(ch->getName(), fmt("message_buy: %s") %
		   shop_index[shop_nr].message_buy);
    keeper->doTell(ch->getName(), fmt("message_sell: %s") %
		   shop_index[shop_nr].message_sell);
    return TRUE;
  } 
  

  if(which=="no_such_item1"){
    delete [] shop_index[shop_nr].no_such_item1;
    shop_index[shop_nr].no_such_item1=mud_str_dup(s);
  } else if(which=="no_such_item2"){
    delete [] shop_index[shop_nr].no_such_item2;
    shop_index[shop_nr].no_such_item2=mud_str_dup(s);
  } else if(which=="do_not_buy"){
    delete [] shop_index[shop_nr].do_not_buy;
    shop_index[shop_nr].do_not_buy=mud_str_dup(s);
  } else if(which=="missing_cash1"){
    delete [] shop_index[shop_nr].missing_cash1;
    shop_index[shop_nr].missing_cash1=mud_str_dup(s);
  } else if(which=="missing_cash2"){
    delete [] shop_index[shop_nr].missing_cash2;
    shop_index[shop_nr].missing_cash2=mud_str_dup(s);
  } else if(which=="message_buy"){
    delete [] shop_index[shop_nr].message_buy;
    shop_index[shop_nr].message_buy=mud_str_dup(s);
  } else if(which=="message_sell"){
    delete [] shop_index[shop_nr].message_sell;
    shop_index[shop_nr].message_sell=mud_str_dup(s);
  } else {
    keeper->doTell(ch->getName(), "You need to specify a string to change.");
    return FALSE;
  }


  TDatabase db(DB_SNEEZY);

  db.query("update shopowned set %s='%s' where shop_nr=%i", 
	   which.c_str(), s.c_str(), shop_nr);

  keeper->doTell(ch->getName(), "Alright, I changed that response.");
  
  return TRUE;
}


int TShopOwned::sellShop(){
  TDatabase db(DB_SNEEZY);
  int value=0;

  if(!hasAccess(SHOPACCESS_SELL)){
    keeper->doTell(ch->getName(), "Sorry, you don't have access to do that.");
    keeper->doTell(ch->getName(), "And remember, when you do sell this shop, I won't pay you for the inventory.");
    keeper->doTell(ch->getName(), "I'll just give you the money I have on me, but nothing for the inventory.");
    return FALSE;
  }
  
  db.query("delete from shopowned where shop_nr=%i", shop_nr);
  
  db.query("delete from shopownedaccess where shop_nr=%i", shop_nr);
  
  db.query("delete from shopownedratios where shop_nr=%i", shop_nr);

  db.query("delete from shopownedmatch where shop_nr=%i", shop_nr);
  
  value+=keeper->getMoney();
  ch->setMoney(ch->getMoney()+value);
  
  shop_index[shop_nr].profit_buy=1.1;
  shop_index[shop_nr].profit_sell=0.9;
  
  keeper->doTell(ch->getName(), "Ok, you no longer own this shop.");
  shop_index[shop_nr].owned=false;
  
  return TRUE;
}



int TShopOwned::giveMoney(sstring arg){
  sstring buf;

  if(!hasAccess(SHOPACCESS_GIVE)){
    keeper->doTell(ch->getName(), "Sorry, you don't have access to do that.");
    return FALSE;
  }
  
  arg = one_argument(arg, buf);
  int amount=convertTo<int>(buf);

  if(amount<=0){
    keeper->doAction(ch->getName(), CMD_SLAP);
    keeper->doTell(ch->getName(), "Don't be an idiot.");
    return FALSE;
  }  

  if(keeper->getMoney()>=amount){
    keeper->setMoney(keeper->getMoney()-amount);
    keeper->saveChar(ROOM_AUTO_RENT);
    ch->setMoney(ch->getMoney()+amount);
    ch->saveChar(ROOM_AUTO_RENT);
    
    shoplog(shop_nr, ch, keeper, "talens", -amount, "receiving");
    
    buf = fmt("$n gives you %d talen%s.") % amount %
      ((amount == 1) ? "" : "s");
    act(buf, TRUE, keeper, NULL, ch, TO_VICT);
    act("$n gives some money to $N.", 1, keeper, 0, ch, TO_NOTVICT);
  } else {
    keeper->doTell(ch->getName(), "I don't have that many talens.");
    keeper->doTell(ch->getName(), fmt("I have %i talens.") % keeper->getMoney());
  }

  return TRUE;
}

int TShopOwned::setAccess(sstring arg)
{
  sstring buf, buf2;
  TDatabase db(DB_SNEEZY);
  unsigned int access;

  if(!hasAccess(SHOPACCESS_ACCESS)){
    keeper->doTell(ch->getName(), "Sorry, you don't have access to do that.");
    return FALSE;
  }

  buf=arg.word(0);
  buf2=arg.word(1);


  if(!buf2.empty()){ // set value
    if(buf=="password"){
      db.query("update shopowned set password='%s' where shop_nr=%i", buf2.c_str(), shop_nr);
      keeper->doTell(ch->getName(), "Ok, I changed the access password.");
    } else {
      db.query("delete from shopownedaccess where shop_nr=%i and upper(name)=upper('%s')", shop_nr, buf.c_str());
      
      if(convertTo<int>(buf2) != 0)
	db.query("insert into shopownedaccess (shop_nr, name, access) values (%i, '%s', %i)", shop_nr, buf.c_str(), convertTo<int>(buf2));
    }
  } else {
    if(!buf.empty()){
      db.query("select name, access from shopownedaccess where shop_nr=%i and upper(name)=upper('%s')", shop_nr, buf.c_str());
    } else {
      db.query("select name, access from shopownedaccess where shop_nr=%i order by access", shop_nr);
    }
    while(db.fetchRow()){
      access=convertTo<int>(db["access"]);
      
      buf = fmt("Access for %s is set to %i, commands/abilities:") %
	       db["name"] % access;
      
      if(access>=SHOPACCESS_LOGS){
	access-=SHOPACCESS_LOGS;
	buf+=" logs";
      }
      if(access>=SHOPACCESS_ACCESS){
	access-=SHOPACCESS_ACCESS;
	buf+=" access";
      }
      if(access>=SHOPACCESS_SELL){
	access-=SHOPACCESS_SELL;
	buf+=" sell";
      }
      if(access>=SHOPACCESS_GIVE){
	access-=SHOPACCESS_GIVE;
	buf+=" give";
      }
      if(access>=SHOPACCESS_RATES){
	access-=SHOPACCESS_RATES;
	buf+=" rates";
      }
      if(access>=SHOPACCESS_INFO){
	access-=SHOPACCESS_INFO;
	buf+=" info";
      }
      if(access>=SHOPACCESS_OWNER){
	access-=SHOPACCESS_OWNER;
	buf+=" owner";
      }
      
      keeper->doTell(ch->getName(), buf);
    }
  }
  
  return TRUE;
}




int TShopOwned::doLogs(sstring arg)
{
  TDatabase db(DB_SNEEZY);
  sstring buf;

  if(!hasAccess(SHOPACCESS_LOGS)){
    keeper->doTell(ch->getName(), "Sorry, you don't have access to do that.");
    return FALSE;
  }
  sstring sb;

  if(arg=="clear"){
    db.query("delete from shoplog where shop_nr=%i", shop_nr);
    ch->sendTo("Done.\n\r");
  } else if(arg=="summaries" || arg=="balance"){
    if(arg=="summaries"){
      db.query("select name, sum(talens) as tsum from shoplog where shop_nr=%i group by name order by tsum desc", shop_nr);
      
      buf = fmt("<r>%-10s %-65.65s<1>\n\r") % "Profit" % "Person";
      sb += buf;
      
      while(db.fetchRow()){
	buf = fmt("%10i %-65.65s\n\r") %
	  convertTo<int>(db["tsum"]) % db["name"];
	sb += buf;
      }
      
      //////////
      sb += "\n\r";
      
      db.query("select item, sum(talens) as tsum from shoplog where shop_nr=%i group by item order by tsum desc", shop_nr);

      buf = fmt("<r>%-10s %-65.65s<1>\n\r") % "Profit" % "Item";
      sb += buf;
      
      while(db.fetchRow()){
	buf = fmt("%10i %-65.65s\n\r") %
	  convertTo<int>(db["tsum"]) % db["item"];
	sb += buf;
      }

      sb += "\n\r";
    }
    
    db.query("select action, sum(talens) as tsum from shoplog where shop_nr=%i group by action order by tsum desc", shop_nr);
    
    buf = fmt("<r>%-12.12s %s<1>\n\r") %
      "Action" % "Total Talens";
    sb += buf;
    
    while(db.fetchRow()){
      buf = fmt("%-12.12s %8i\n\r") %
	db["action"] % convertTo<int>(db["tsum"]);
      sb += buf;
    }
    
    /////////
    sb += "\n\r";
    int profit=0, loss=0;
    
    sb+="<r>Sales Balance<1>\n\r";
    
    db.query("select sum(talens) as talens from shoplog where shop_nr=%i and talens > 0 and action='buying'", shop_nr);
    
    if(db.fetchRow())
      profit=convertTo<int>(db["talens"]);
    
    buf = fmt("%-15.15s %i\n\r") % "Sales" % profit;
    sb += buf;
    
    db.query("select sum(talens) as talens from shoplog where shop_nr=%i and talens < 0 and action='selling'", shop_nr);
    
    if(db.fetchRow())
      loss=convertTo<int>(db["talens"]);
    
    buf = fmt("%-15.15s %i\n\r") % "Purchases" % loss;
    sb += buf;
    
    buf = fmt("%-15.15s %i\n\r") % "Sales Income" % (profit+loss);
    sb += buf;
    
    /////////
    sb += "\n\r";
    profit=loss=0;
    
    sb+="<r>Gross Balance<1>\n\r";
    
    db.query("select sum(talens) as talens from shoplog where shop_nr=%i and talens > 0",
	     shop_nr);
    if(db.fetchRow())
      profit=convertTo<int>(db["talens"]);
    
    buf = fmt("%-15.15s %i\n\r") % "Income" % profit;
    sb += buf;
    
    db.query("select sum(talens) as talens from shoplog where shop_nr=%i and talens < 0",
	     shop_nr);
    if(db.fetchRow())
      loss=convertTo<int>(db["talens"]);
    
    buf = fmt("%-15.15s %i\n\r") % "Expenditures" % loss;
    sb += buf;
    
    buf = fmt("%-15.15s %i\n\r") % "Total" % (profit+loss);
    sb += buf;
    
    /////////
    if (ch->desc)
      ch->desc->page_string(sb, SHOWNOW_NO, ALLOWREP_YES);
    ///////////////////////////////////////////////////////////////////////
  } else {
    if(!arg.empty()){
      db.query("select name, action, item, talens, shoptalens, shopvalue, logtime from shoplog where shop_nr=%i and upper(name)=upper('%s') order by logtime desc, action desc", shop_nr, arg.c_str());      
    } else {
      db.query("select name, action, item, talens, shoptalens, shopvalue, logtime from shoplog where shop_nr=%i order by logtime desc, action desc", shop_nr);
    }    

    while(db.fetchRow()){
      buf = fmt("%19.19s  Talens: %8i  Value: %8i  Total: %8i\n\r") % db["logtime"] % convertTo<int>(db["shoptalens"]) % convertTo<int>(db["shopvalue"]) % (convertTo<int>(db["shopvalue"])+convertTo<int>(db["shoptalens"]));
      sb += buf;
      
      buf = fmt("%-12.12s %-10.10s %-32.32s for %8i talens.\n\r\n\r") %
	db["name"] % db["action"] % db["item"] %
	convertTo<int>(db["talens"]);
      sb += buf;
    }
    
    if (ch->desc)
      ch->desc->page_string(sb, SHOWNOW_NO, ALLOWREP_YES);
    
    
  }
  return TRUE;
}



int TShopOwned::getMaxNum(const TObj *o)
{
  TDatabase db(DB_SNEEZY);
  
  if(o){
    db.query("select match, max_num from shopownedmatch where shop_nr=%i", shop_nr);
    
    while(db.fetchRow()){
      if(isname(db["match"], o->name))
	return convertTo<int>(db["max_num"]);
    }
    
    db.query("select max_num from shopownedratios where shop_nr=%i and obj_nr=%i", shop_nr, o->objVnum());
    
    if(db.fetchRow())
      return convertTo<int>(db["max_num"]);
  }


  db.query("select max_num from shopowned where shop_nr=%i", shop_nr);
  
  if(db.fetchRow())
    return convertTo<int>(db["max_num"]);

  // this is non-owned shop default
  return 9;
}

