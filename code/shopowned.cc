#include "stdsneezy.h"
#include "shopowned.h"
#include "database.h"
#include "shop.h"


// this function relies on the fact that the db will return rows in the order
// that they were created, chronologically.  I'm not sure if this is defined
// behavior or not, so if it stops working, you need to put a timestamp value
// into the table and sort by that
bool sameAccount(string buf, int shop_nr){
  charFile st, stthis;

  load_char(buf.c_str(), &stthis);

  TDatabase db("sneezy");

  db.query("select name from shopownedaccess where shop_nr=%i", shop_nr);

  while(db.fetchRow()){
    if (!load_char(db.getColumn(0), &st))
      continue;

    if(!strcmp(stthis.aname, st.aname)){
      if(!strcmp(lower(buf).c_str(), lower(db.getColumn(0)).c_str()))
	return FALSE;
      else
	return TRUE;
    }
  }

  return FALSE;
}

int TShopOwned::getPurchasePrice(int talens, int value){
  return (int)(((talens+value)*1.15)+1000000);
}


int getShopAccess(int shop_nr, TBeing *ch){
  int access=0;
  TDatabase db("sneezy");

  db.query("select access from shopownedaccess where shop_nr=%i and upper(name)=upper('%s')", shop_nr, ch->getName());
  
  if(db.fetchRow())
    access=atoi_safe(db.getColumn(0));
  
  if(sameAccount(ch->getName(), shop_nr) && !ch->isImmortal() && access){
    ch->sendTo("Another character in your account has permissions at this shop, so this character can not use the ownership functions.\n\r");
    access=0;
  }
  
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


TShopOwned::~TShopOwned(){
}

bool TShopOwned::isOwned(){
  return owned;
}

bool TShopOwned::hasAccess(int perm){
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
  string buf;
  int volume=0;

  // if not owned, or owned and has "owner" or "info"
  if(!isOwned() || hasAccess(SHOPACCESS_INFO)){
    for(tt=keeper->getStuff();tt;tt=tt->nextThing){
      o=dynamic_cast<TObj *>(tt);
      ++count;
      volume+=o->getVolume();
      value+=o->obj_flags.cost;
      // shopPrice does db queries, it tends to be too slow here
      //      price+=o->shopPrice(1, shop_nr, -1, &discount);
    }
    keeper->doTell(ch->getName(), "I have %i talens and %i items worth %i talens and selling for approximately %i talens.", keeper->getMoney(), count, value, (int)(value * shop_index[shop_nr].profit_buy));
    keeper->doTell(ch->getName(), "My inventory takes up %i cubic inches of space.", volume);
    
    keeper->doTell(ch->getName(), "That puts my total value at %i talens.",
		   keeper->getMoney()+value);
    
    if(!owned){
      keeper->doTell(ch->getName(), "This shop is for sale, however the King charges a sales tax and an ownership fee.");
      
      keeper->doTell(ch->getName(), "That puts the sale price at %i.",
		     getPurchasePrice(keeper->getMoney(), value));
    } 
  }

  // anyone can see profit_buy, profit_sell and trading types, anytime
  keeper->doTell(ch->getName(),"My profit_buy is %f and my profit_sell is %f.",
		 shop_index[shop_nr].profit_buy,
		 shop_index[shop_nr].profit_sell);
  keeper->doTell(ch->getName(),"My maximum inventory per item is %i.",
		 getMaxNum(NULL));


  if(shop_index[shop_nr].type.size()<=1){
    keeper->doTell(ch->getName(), 
		   "I only sell things, I do not buy anything.");
  } else {
    ssprintf(buf, "%s I deal in", ch->getName());
    for(i=0;i<shop_index[shop_nr].type.size();++i){
      tmp=shop_index[shop_nr].type[i];
      if(tmp != MAX_OBJ_TYPES && (int) tmp != -1)
	ssprintf(buf, "%s %s,", buf.c_str(), ItemInfo[tmp]->name);
    }
    keeper->doTell(buf.c_str());
  }
}


int TShopOwned::setRates(string arg)
{
  TDatabase db("sneezy");
  string buf;
  float profit_buy, profit_sell;
  int max_num, argc=0;

  if(!hasAccess(SHOPACCESS_RATES)){
    keeper->doTell(ch->getName(), "Sorry, you don't have access to do that.");
    return FALSE;
  }

  arg = one_argument(arg, buf);
  profit_buy = atof_safe(buf.c_str());
  if(buf != "")
    argc++;


  if(buf == "clear"){ /////////////////////////////////////////////
    arg = one_argument(arg, buf);
    if(buf == "all"){
      db.query("delete from shopownedratios where shop_nr=%i", shop_nr);
      db.query("delete from shopownedmatch where shop_nr=%i", shop_nr);
      keeper->doTell(ch->getName(), "Ok, I cleared all of the individual profit ratios.");
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
      TThing *tt = searchLinkedListVis(ch, buf.c_str(), keeper->getStuff());
      
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
  profit_sell = atof_safe(buf.c_str());
  if(buf != "")
    argc++;

  arg = one_argument(arg, buf);
  max_num = atoi_safe(buf.c_str());
  if(buf != "")
    argc++;
  
  arg = one_argument(arg, buf);
  if(buf != "")
    argc++;


  if(profit_buy>5 || profit_buy<0 ||
     profit_sell>5 || profit_sell<0){
    keeper->doTell(ch->getName(), "Due to fraud regulations, I cannot set my profit_sell or profit_buy to more than 5 or less than 0.");
    return FALSE;
  }
  
  
  if(argc==0){
    db.query("select obj_nr, profit_buy, profit_sell, max_num from shopownedratios where shop_nr=%i", shop_nr);
    
    while(db.fetchRow()){
      keeper->doTell(ch->getName(), "%f %f %i %s", atof_safe(db.getColumn(1)), 
		     atof_safe(db.getColumn(2)), atoi_safe(db.getColumn(3)),
	       obj_index[real_object(atoi_safe(db.getColumn(0)))].short_desc);
    }

    db.query("select match, profit_buy, profit_sell, max_num from shopownedmatch where shop_nr=%i", shop_nr);
    
    while(db.fetchRow()){
      keeper->doTell(ch->getName(), "%f %f %i %s", atof_safe(db.getColumn(1)), 
		     atof_safe(db.getColumn(2)), atoi_safe(db.getColumn(3)),
		     db.getColumn(0));
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
    
    keeper->doTell(ch->getName(), "Ok, my profit_buy is now %f, my profit_sell is now %f and my max_num is now %i.", shop_index[shop_nr].profit_buy, shop_index[shop_nr].profit_sell, max_num);
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
    
    keeper->doTell(ch->getName(), "Ok, my profit_buy is now %f, my profit_sell is now %f and my max_num is now %i, all for keyword %s.", profit_buy, profit_sell, max_num, buf.c_str());    
  } else { ////////////////////////////////////////////////////////////////
    // find item in inventory matching keywords in arg
    // get vnum, then store in db
    TThing *tt = searchLinkedListVis(ch, buf.c_str(), keeper->getStuff());
    
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
    
    keeper->doTell(ch->getName(), "Ok, my profit_buy is now %f, my profit_sell is now %f and my max_num is %i, all for %s.", profit_buy, profit_sell, max_num, o->getName());
  }
  
  return TRUE;
}


int TShopOwned::buyShop(){
  int value=0;
  TDatabase db("sneezy");
  string buf;
  TThing *tt;
  TObj *o;

  if(isOwned()){
    keeper->doTell(ch->getName(), "Sorry, this shop isn't for sale.");
    return TRUE;
  }
  
  for(tt=keeper->getStuff();tt;tt=tt->nextThing){
    o=dynamic_cast<TObj *>(tt);
    value+=o->obj_flags.cost;
  }
  value=getPurchasePrice(keeper->getMoney(), value);
  
  if(ch->getMoney()<value){
    keeper->doTell(ch->getName(), "Sorry, you can't afford this shop.  The price is %i.", value);
    return TRUE;
  }
  ch->setMoney(ch->getMoney()-value);
  
  
  db.query("insert into shopowned (shop_nr, profit_buy, profit_sell) values (%i, %f, %f)", shop_nr, shop_index[shop_nr].profit_buy, shop_index[shop_nr].profit_sell);
  
  db.query("insert into shopownedaccess (shop_nr, name, access) values (%i, '%s', %i)", shop_nr, ch->getName(),  SHOPACCESS_OWNER);
  
  ssprintf(buf, "%s/%d", SHOPFILE_PATH, shop_nr);
  keeper->saveItems(buf.c_str());
  
  keeper->doTell(ch->getName(), "Congratulations, you now own this shop.");

  return TRUE;
}
  



int TShopOwned::sellShop(){
  TDatabase db("sneezy");
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

  db.query("delete from shopownematch where shop_nr=%i", shop_nr);
  
  value+=keeper->getMoney();
  ch->setMoney(ch->getMoney()+value);
  
  shop_index[shop_nr].profit_buy=1.1;
  shop_index[shop_nr].profit_sell=0.9;
  
  keeper->doTell(ch->getName(), "Ok, you no longer own this shop.");
  
  return TRUE;
}



int TShopOwned::giveMoney(string arg){
  string buf;

  if(!hasAccess(SHOPACCESS_GIVE)){
    keeper->doTell(ch->getName(), "Sorry, you don't have access to do that.");
    return FALSE;
  }
  
  arg = one_argument(arg, buf);
  int amount=atoi_safe(buf.c_str());

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
    
    shoplog(shop_nr, ch, keeper, "talens", amount, "receiving");
    
    ssprintf(buf, "$n gives you %d talen%s.\n\r", amount,
	    (amount == 1) ? "" : "s");
    act(buf.c_str(), TRUE, keeper, NULL, ch, TO_VICT);
    act("$n gives some money to $N.", 1, keeper, 0, ch, TO_NOTVICT);
  } else {
    keeper->doTell(ch->getName(), "I don't have that many talens.");
    keeper->doTell(ch->getName(), "I have %i talens.",keeper->getMoney());
  }

  return TRUE;
}

int TShopOwned::setAccess(string arg)
{
  string buf, buf2;
  TDatabase db("sneezy");
  unsigned int access;

  if(!hasAccess(SHOPACCESS_ACCESS)){
    keeper->doTell(ch->getName(), "Sorry, you don't have access to do that.");
    return FALSE;
  }
  
  arg = one_argument(arg, buf);
  arg = one_argument(arg, buf2);

  if(!buf2.empty()){ // set value
    db.query("delete from shopownedaccess where shop_nr=%i and upper(name)=upper('%s')", shop_nr, buf.c_str());
    
    if(atoi_safe(buf2.c_str()) != 0)
      db.query("insert into shopownedaccess (shop_nr, name, access) values (%i, '%s', %i)", shop_nr, buf.c_str(), atoi_safe(buf2.c_str()));
    
  } else {
    if(!buf.empty()){
      db.query("select name, access from shopownedaccess where shop_nr=%i and upper(name)=upper('%s')", shop_nr, buf.c_str());
    } else {
      db.query("select name, access from shopownedaccess where shop_nr=%i order by access", shop_nr);
    }
    while(db.fetchRow()){
      access=atoi_safe(db.getColumn(1));
      
      ssprintf(buf, "%s Access for %s is set to %i, commands/abilities:",
	       ch->getName(), db.getColumn(0), access);
      
      if(access>=SHOPACCESS_LOGS){
	access-=SHOPACCESS_LOGS;
	ssprintf(buf2, " logs");
	buf+=buf2;
      }
      if(access>=SHOPACCESS_ACCESS){
	access-=SHOPACCESS_ACCESS;
	ssprintf(buf2, " access");
	buf+=buf2;
      }
      if(access>=SHOPACCESS_SELL){
	access-=SHOPACCESS_SELL;
	ssprintf(buf2, " sell");
	buf+=buf2;
      }
      if(access>=SHOPACCESS_GIVE){
	access-=SHOPACCESS_GIVE;
	ssprintf(buf2, " give");
	buf+=buf2;
      }
      if(access>=SHOPACCESS_RATES){
	access-=SHOPACCESS_RATES;
	ssprintf(buf2, " rates");
	buf+=buf2;
      }
      if(access>=SHOPACCESS_INFO){
	access-=SHOPACCESS_INFO;
	ssprintf(buf2, " info");
	buf+=buf2;
      }
      if(access>=SHOPACCESS_OWNER){
	access-=SHOPACCESS_OWNER;
	ssprintf(buf2, " owner");
	buf+=buf2;
      }
      
      keeper->doTell(buf.c_str());
    }
  }
  
  return TRUE;
}




int TShopOwned::doLogs(string arg)
{
  TDatabase db("sneezy");
  string buf;

  if(!hasAccess(SHOPACCESS_LOGS)){
    keeper->doTell(ch->getName(), "Sorry, you don't have access to do that.");
    return FALSE;
  }
  string sb;

  if(arg==" clear"){
    db.query("delete from shoplog where shop_nr=%i", shop_nr);
    ch->sendTo("Done.\n\r");
  } else if(arg==" summaries"){
    db.query("select name, action, sum(talens) as tsum from shoplog where shop_nr=%i group by name, action order by tsum desc", shop_nr);
    
    ssprintf(buf, "<r>%-12.12s %-10.10s %s<1>\n\r",
	     "Person", "Action", "Total Talens");
    sb += buf;

    while(db.fetchRow()){
      ssprintf(buf, "%-12.12s %-10.10s %8i\n\r", 
	      db.getColumn(0), db.getColumn(1), atoi_safe(db.getColumn(2)));
      sb += buf;
    }
    
    
    //////////
    sb += "\n\r";
    
    db.query("select item, action, sum(talens) as tsum from shoplog where shop_nr=%i group by item, action order by tsum desc", shop_nr);

    ssprintf(buf, "<r>%-26.26s %-10.10s %s<1>\n\r",
	     "Item", "Action" ,"Total Talens");
    sb += buf;
    
    while(db.fetchRow()){
      ssprintf(buf, "%-32.32s %-10.10s %8i\n\r", 
	      db.getColumn(0), db.getColumn(1), atoi_safe(db.getColumn(2)));
      sb += buf;
    }
    
    
    /////////
    sb += "\n\r";
    
    db.query("select action, sum(talens) as tsum from shoplog where shop_nr=%i group by action order by tsum desc", shop_nr);
    
    ssprintf(buf, "<r>%-12.12s %s<1>\n\r",
	     "Action", "Total Talens");
    sb += buf;

    while(db.fetchRow()){
      ssprintf(buf, "%-12.12s %8i\n\r", 
	      db.getColumn(0), atoi_safe(db.getColumn(1)));
      sb += buf;
    }

    /////////
    sb += "\n\r";
    int profit=0, loss=0;
    
    ssprintf(buf, "<r>Sales Balance<1>\n\r");
    sb += buf;

    db.query("select sum(talens) from shoplog where shop_nr=%i and talens > 0 and action != 'receiving' and action != 'giving'", shop_nr);
    
    if(db.fetchRow())
      profit=atoi_safe(db.getColumn(0));

    ssprintf(buf, "%-15.15s %i\n\r", "Sales Profit", profit);
    sb += buf;

    db.query("select sum(talens) from shoplog where shop_nr=%i and talens < 0 and action != 'receiving' and action != 'giving'", shop_nr);

    if(db.fetchRow())
      loss=atoi_safe(db.getColumn(0));
    
    ssprintf(buf, "%-15.15s %i\n\r", "Sales Loss", loss);
    sb += buf;
    
    ssprintf(buf, "%-15.15s %i\n\r", "Sales Income", profit+loss);
    sb += buf;


    /////////
    sb += "\n\r";
    profit=loss=0;
    

    ssprintf(buf, "<r>Gross Balance<1>\n\r");
    sb += buf;
    
    db.query("select sum(talens) from shoplog where shop_nr=%i and talens > 0",
	     shop_nr);
    if(db.fetchRow())
      profit=atoi_safe(db.getColumn(0));
    
    ssprintf(buf, "%-15.15s %i\n\r", "Gross Profit", profit);
    sb += buf;

    db.query("select sum(talens) from shoplog where shop_nr=%i and talens < 0",
	     shop_nr);
    if(db.fetchRow())
      loss=atoi_safe(db.getColumn(0));
    
    ssprintf(buf, "%-15.15s %i\n\r", "Gross Loss", loss);
    sb += buf;
    
    ssprintf(buf, "%-15.15s %i\n\r", "Net Income", profit+loss);
    sb += buf;

    
    /////////
    if (ch->desc)
      ch->desc->page_string(sb.c_str(), SHOWNOW_NO, ALLOWREP_YES);
  } else if(arg==" balance"){
    db.query("select action, sum(talens) as tsum from shoplog where shop_nr=%i group by action order by tsum desc", shop_nr);
    
    ssprintf(buf, "<r>%-12.12s %s<1>\n\r", 
	     "Action", "Total Talens");
    sb += buf;

    while(db.fetchRow()){
      ssprintf(buf, "%-12.12s %8i\n\r", 
	      db.getColumn(0), atoi_safe(db.getColumn(1)));
      sb += buf;
    }

    /////////
    sb += "\n\r";
    int profit=0, loss=0;

    ssprintf(buf, "<r>Balance Sheet<1>\n\r");
    sb += buf;
    
    db.query("select sum(talens) from shoplog where shop_nr=%i and talens > 0",
	     shop_nr);
    if(db.fetchRow())
      profit=atoi_safe(db.getColumn(0));
    
    ssprintf(buf, "%-15.15s %i\n\r", "Gross Profit", profit);
    sb += buf;

    db.query("select sum(talens) from shoplog where shop_nr=%i and talens < 0",
	     shop_nr);
    if(db.fetchRow())
      loss=atoi_safe(db.getColumn(0));
    
    ssprintf(buf, "%-15.15s %i\n\r", "Gross Loss", loss);
    sb += buf;
    
    ssprintf(buf, "%-15.15s %i\n\r", "Net Income", profit+loss);
    sb += buf;

    
    /////////
    if (ch->desc)
      ch->desc->page_string(sb.c_str(), SHOWNOW_NO, ALLOWREP_YES);
  } else {
    if(!arg.empty()){
      db.query("select name, action, item, talens, shoptalens, shopvalue, logtime from shoplog where shop_nr=%i and action!='paying tax' and upper(name)=upper('%s') order by logtime desc, action desc", shop_nr, arg.c_str());      
    } else {
      db.query("select name, action, item, talens, shoptalens, shopvalue, logtime from shoplog where shop_nr=%i and action!='paying tax' order by logtime desc, action desc", shop_nr);
    }    

    while(db.fetchRow()){
      ssprintf(buf, "%s  Talens: %8i  Value: %8i  Total: %8i\n\r", db.getColumn(6), atoi_safe(db.getColumn(4)), atoi_safe(db.getColumn(5)), atoi_safe(db.getColumn(4))+atoi_safe(db.getColumn(5)));
      sb += buf;
      
      ssprintf(buf, "%-12.12s %-10.10s %-32.32s for %8i talens.\n\r\n\r",
	      db.getColumn(0), db.getColumn(1), db.getColumn(2), atoi_safe(db.getColumn(3)));
      sb += buf;
    }
    
    if (ch->desc)
      ch->desc->page_string(sb.c_str(), SHOWNOW_NO, ALLOWREP_YES);
    
    
  }
  return TRUE;
}



int TShopOwned::getMaxNum(const TObj *o)
{
  TDatabase db("sneezy");
  
  if(o){
    db.query("select match, max_num from shopownedmatch where shop_nr=%i", shop_nr);
    
    while(db.fetchRow()){
      if(isname(db.getColumn(0), o->name))
	return atoi_safe(db.getColumn(1));
    }
    
    db.query("select max_num from shopownedratios where shop_nr=%i and obj_nr=%i", shop_nr, o->objVnum());
    
    if(db.fetchRow())
      return atoi_safe(db.getColumn(0));
  }


  db.query("select max_num from shopowned where shop_nr=%i", shop_nr);
  
  if(db.fetchRow())
    return atoi_safe(db.getColumn(0));

  // shouldn't get here...
  return 9;
}

