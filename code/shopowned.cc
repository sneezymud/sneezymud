#include "stdsneezy.h"
#include "shopowned.h"
#include "database.h"
#include "shop.h"

bool sameAccount2(const char *buf, int shop_nr){
  charFile st, stthis;

  load_char(buf, &stthis);

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
  
  if(sameAccount2(ch->getName(), shop_nr) && !ch->isImmortal() && access){
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
  int count=0, value=0, price=0, discount=100, tmp=0;
  unsigned int i=0;
  char buf[256];

  // if not owned, or owned and has "owner" or "info"
  if(!isOwned() || hasAccess(SHOPACCESS_INFO)){
    for(tt=keeper->getStuff();tt;tt=tt->nextThing){
      o=dynamic_cast<TObj *>(tt);
      ++count;
      value+=o->obj_flags.cost;
      price+=o->shopPrice(1, shop_nr, -1, &discount);
    }
    keeper->doTell(ch->getName(), "I have %i talens and %i items worth %i talens and selling for %i talens.", keeper->getMoney(), count, value, price);
    
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
  
  if(shop_index[shop_nr].type.size()<=1){
    keeper->doTell(ch->getName(), 
		   "I only sell things, I do not buy anything.");
  } else {
    sprintf(buf, "%s I deal in", ch->getName());
    for(i=0;i<shop_index[shop_nr].type.size();++i){
      tmp=shop_index[shop_nr].type[i];
      if(tmp != MAX_OBJ_TYPES && (int) tmp != -1)
	sprintf(buf+strlen(buf), " %s,",
		ItemInfo[tmp]->name);
    }
    buf[strlen(buf)-1]='\0';
    keeper->doTell(buf);
  }
}


int TShopOwned::setProfitBuy(const char *arg)
{
  TDatabase db("sneezy");
  char buf[256];

  if(!hasAccess(SHOPACCESS_PROFITS)){
    keeper->doTell(ch->getName(), "Sorry, you don't have access to do that.");
    return FALSE;
  }

  arg = one_argument(arg, buf);  

  if(!*buf){
    db.query("select obj_nr, profit_buy, profit_sell from shopownedratios where shop_nr=%i", shop_nr);
    
    while(db.fetchRow()){
      keeper->doTell(ch->getName(), "%f %f %s", atof_safe(db.getColumn(1)), 
		     atof_safe(db.getColumn(2)), obj_index[real_object(atoi_safe(db.getColumn(0)))].short_desc);
    }
    
    return TRUE;
  } else if(!strcmp(buf, "clear")){
    db.query("delete from shopownedratios where shop_nr=%i", shop_nr);
    keeper->doTell(ch->getName(), "Ok, I cleared all of the individual profit ratios.");
    return TRUE;
  }
  
  if(atof_safe(buf)>5){
    keeper->doTell(ch->getName(), "Because of fraud regulations, I can't set the profit_buy higher than 5!");
    return FALSE;
  }
  
  
  if(*arg){
    // find item in inventory matching keywords in arg
    // get vnum, then store in db
    TThing *tt = searchLinkedListVis(ch, arg, keeper->getStuff());
    
    if(!tt){
      keeper->doTell(ch->getName(), "I don't have that item.");
      return FALSE;
    }
    
    TObj *o=dynamic_cast<TObj *>(tt);
    
    // create the entry if it doesn't exist, use default profit buy/sell
    db.query("select 1 from shopownedratios where shop_nr=%i and obj_nr=%i", shop_nr, o->objVnum());
    
    if(!db.fetchRow()){
      // get the default profit buy/sell
      db.query("select profit_buy, profit_sell from shop where shop_nr=%i", shop_nr);
      db.fetchRow();
      db.query("insert into shopownedratios values (%i, %i, %f, %f)", shop_nr, o->objVnum(), atof_safe(buf), atof_safe(db.getColumn(1)));
    } else {
      db.query("update shopownedratios set profit_buy=%f where shop_nr=%i and obj_nr=%i", atof_safe(buf), shop_nr, o->objVnum());
    }
    
    keeper->doTell(ch->getName(), "Ok, my profit_buy is now %f for %s.", 
		   atof_safe(buf), o->getName());
  } else { //////////////////////////////
    shop_index[shop_nr].profit_buy=atof_safe(buf);
    
    db.query("update shopowned set profit_buy=%f where shop_nr=%i", shop_index[shop_nr].profit_buy, shop_nr);
    
    keeper->doTell(ch->getName(), "Ok, my profit_buy is now %f", 
		   shop_index[shop_nr].profit_buy);
  }

  return TRUE;
}




int TShopOwned::setProfitSell(const char *arg)
{
  TDatabase db("sneezy");
  char buf[256];

  if(!hasAccess(SHOPACCESS_PROFITS)){
    keeper->doTell(ch->getName(), "Sorry, you don't have access to do that.");
    return FALSE;
  }

  arg = one_argument(arg, buf);
  
  if(!*buf){
    db.query("select obj_nr, profit_buy, profit_sell from shopownedratios where shop_nr=%i", shop_nr);
    
    while(db.fetchRow()){
      keeper->doTell(ch->getName(), "%f %f %s", atof_safe(db.getColumn(1)), 
		     atof_safe(db.getColumn(2)), 
		     obj_index[real_object(atoi_safe(db.getColumn(0)))].short_desc);
    }
    
    return TRUE;
  } else if(!strcmp(buf, "clear")){
    db.query("delete from shop ownedratios where shop_nr=%i", shop_nr);
    
    keeper->doTell(ch->getName(), "Ok, I cleared all of the individual profit ratios.");
    return TRUE;
  }
  
  if(*arg){
    // find item in inventory matching keywords in arg
    // get vnum, then store in db
    TThing *tt = searchLinkedListVis(ch, arg, keeper->getStuff());
    
    if(!tt){
      keeper->doTell(ch->getName(), "I don't have that item.");
      return FALSE;
    }
    
    TObj *o=dynamic_cast<TObj *>(tt);
    
    // create the entry if it doesn't exist, use default profit buy/sell
    db.query("select 1 from shopownedratios where shop_nr=%i and obj_nr=%i", shop_nr, o->objVnum());
    
    if(!db.fetchRow()){
      // get the default profit buy/sell
      db.query("select profit_buy, profit_sell from shop where shop_nr=%i", shop_nr);
      db.fetchRow();
      
      db.query("insert into shopownedratios values (%i, %i, %f, %f)", shop_nr, o->objVnum(), atof_safe(db.getColumn(0)), atof_safe(buf));
    } else {
      db.query("update shopownedratios set profit_sell=%f where shop_nr=%i and obj_nr=%i", atof_safe(buf), shop_nr, o->objVnum());
    }
    
    keeper->doTell(ch->getName(), "Ok, my profit_sell is now %f for %s.",
		   atof_safe(buf), o->getName());
  } else {
    shop_index[shop_nr].profit_sell=atof_safe(buf);
    
    db.query("update shopowned set profit_sell=%f where shop_nr=%i", shop_index[shop_nr].profit_sell, shop_nr);
    
    keeper->doTell(ch->getName(), "Ok, my profit_sell is now %f", 
		   shop_index[shop_nr].profit_sell);
  }
  return TRUE;
}


int TShopOwned::buyShop(){
  int value=0;
  TDatabase db("sneezy");
  char buf[256];
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
  
  keeper->saveItems(buf);
  
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
  
  value+=keeper->getMoney();
  ch->setMoney(ch->getMoney()+value);
  
  shop_index[shop_nr].profit_buy=1.1;
  shop_index[shop_nr].profit_sell=0.9;
  
  keeper->doTell(ch->getName(), "Ok, you no longer own this shop.");
  
  return TRUE;
}



int TShopOwned::giveMoney(const char *arg){
  char buf[256];

  if(!hasAccess(SHOPACCESS_GIVE)){
    keeper->doTell(ch->getName(), "Sorry, you don't have access to do that.");
    return FALSE;
  }
  
  arg = one_argument(arg, buf);
  int amount=atoi_safe(buf);
  
  if(keeper->getMoney()>=amount){
    keeper->setMoney(keeper->getMoney()-amount);
    keeper->saveChar(ROOM_AUTO_RENT);
    ch->setMoney(ch->getMoney()+amount);
    ch->saveChar(ROOM_AUTO_RENT);
    
    shoplog(shop_nr, ch, keeper, "talens", amount, "receiving");
    
    sprintf(buf, "$n gives you %d talen%s.\n\r", amount,
	    (amount == 1) ? "" : "s");
    act(buf, TRUE, keeper, NULL, ch, TO_VICT);
    act("$n gives some money to $N.", 1, keeper, 0, ch, TO_NOTVICT);
  } else {
    keeper->doTell(ch->getName(), "I don't have that many talens.");
    keeper->doTell(ch->getName(), "I have %i talens.",keeper->getMoney());
  }

  return TRUE;
}

int TShopOwned::setAccess(const char *arg)
{
  char buf[256], buf2[256];
  TDatabase db("sneezy");
  unsigned int access;

  if(!hasAccess(SHOPACCESS_ACCESS)){
    keeper->doTell(ch->getName(), "Sorry, you don't have access to do that.");
    return FALSE;
  }
  
  arg = one_argument(arg, buf);
  arg = one_argument(arg, buf2);
  
  if(*buf2){ // set value
    db.query("delete from shopownedaccess where shop_nr=%i and upper(name)=upper('%s')", shop_nr, buf);
    
    if(atoi_safe(buf2) != 0)
      db.query("insert into shopownedaccess (shop_nr, name, access) values (%i, '%s', %i)", shop_nr, buf, atoi_safe(buf2));
    
  } else {
    if(*buf){
      db.query("select name, access from shopownedaccess where shop_nr=%i and upper(name)=upper('%s')", shop_nr, buf);
    } else {
      db.query("select name, access from shopownedaccess where shop_nr=%i order by access", shop_nr);
    }
    while(db.fetchRow()){
      access=atoi_safe(db.getColumn(1));
      
      sprintf(buf2, "%s Access for %s is set to %i, commands/abilities:", ch->getName(),
	      db.getColumn(0), access);
      
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
      
      keeper->doTell(buf2);
    }
    
  }
  
  return TRUE;
}




int TShopOwned::doLogs(const char *arg)
{
  TDatabase db("sneezy");
  string buf;

  if(!hasAccess(SHOPACCESS_LOGS)){
    keeper->doTell(ch->getName(), "Sorry, you don't have access to do that.");
    return FALSE;
  }
  string sb;
  
  if(!strcmp(arg, " clear")){
    db.query("delete from shoplog where shop_nr=%i", shop_nr);
    ch->sendTo("Done.\n\r");
  } else if(!strcmp(arg, " summaries")){
    db.query("select name, action, sum(talens) as tsum from shoplog where shop_nr=%i group by name, action order by tsum desc", shop_nr);
    
    ssprintf(sb, "%s<r>%-12.12s %-10.10s %s<1>\n\r", sb.c_str(),
	     "Person", "Action", "Total Talens");

    while(db.fetchRow()){
      ssprintf(buf, "%-12.12s %-10.10s %8i\n\r", 
	      db.getColumn(0), db.getColumn(1), atoi_safe(db.getColumn(2)));
      sb += buf;
    }
    
    
    //////////
    sb += "\n\r";
    
    db.query("select item, action, sum(talens) as tsum from shoplog where shop_nr=%i group by item, action order by tsum desc", shop_nr);

    ssprintf(sb, "%s<r>%-26.26s %-10.10s %s<1>\n\r", sb.c_str(),
	     "Item", "Action" ,"Total Talens");
    
    while(db.fetchRow()){
      ssprintf(buf, "%-32.32s %-10.10s %8i\n\r", 
	      db.getColumn(0), db.getColumn(1), atoi_safe(db.getColumn(2)));
      sb += buf;
    }
    
    
    /////////
    sb += "\n\r";
    
    db.query("select action, sum(talens) as tsum from shoplog where shop_nr=%i group by action order by tsum desc", shop_nr);
    
    ssprintf(sb, "%s<r>%-12.12s %s<1>\n\r", sb.c_str(),
	     "Action", "Total Talens");

    while(db.fetchRow()){
      ssprintf(buf, "%-12.12s %8i\n\r", 
	      db.getColumn(0), atoi_safe(db.getColumn(1)));
      sb += buf;
    }

    /////////
    sb += "\n\r";
    int profit=0, loss=0;

    ssprintf(sb, "%s<r>Balance Sheet<1>\n\r", sb.c_str());

    db.query("select sum(talens) from shoplog where shop_nr=%i and talens > 0",
	     shop_nr);
    if(db.fetchRow())
      profit=atoi_safe(db.getColumn(0));
    
    ssprintf(sb, "%s%-15.15s %i\n\r", sb.c_str(), "Gross Profit", profit);


    db.query("select sum(talens) from shoplog where shop_nr=%i and talens < 0",
	     shop_nr);
    if(db.fetchRow())
      loss=atoi_safe(db.getColumn(0));
    
    ssprintf(sb, "%s%-15.15s %i\n\r", sb.c_str(), "Gross Loss", loss);
    
    ssprintf(sb, "%s%-15.15s %i\n\r", sb.c_str(), "Net Income", profit+loss);
    
    
    /////////
    if (ch->desc)
      ch->desc->page_string(sb.c_str(), SHOWNOW_NO, ALLOWREP_YES);

  } else {
    if(*arg){
      while (*arg && isspace(*arg))
	arg++;

      db.query("select name, action, item, talens, shoptalens, shopvalue, logtime from shoplog where shop_nr=%i and action!='paying tax' and upper(name)=upper('%s') order by logtime desc, shoptalens+shopvalue desc", shop_nr, arg);      
    } else {
      db.query("select name, action, item, talens, shoptalens, shopvalue, logtime from shoplog where shop_nr=%i and action!='paying tax' order by logtime desc, shoptalens+shopvalue desc", shop_nr);
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


