#include "stdsneezy.h"
#include "database.h"

void updateStockHistory()
{
  TDatabase db(DB_SNEEZY);

  db.query("insert into stockhistory select (select max(n)+1 from stockhistory), ticker, price from stockinfo group by ticker, price");

  db.query("insert into stockhistory select (select max(n) from stockhistory), 'INDX', sum(price*shares)/sum(shares) from stockinfo");
}


void stockSplit(sstring ticker, int price)
{
  TDatabase db(DB_SNEEZY);

  db.query("update stockinfo set shares=shares*2, price=(price/5) where ticker='%s'", ticker.c_str());
  
  db.query("update stockhistory set price=price/2 where ticker='%s'",
  	   ticker.c_str());
}

void stockReverseSplit(sstring ticker, int price)
{
  TDatabase db(DB_SNEEZY);

  db.query("update stockinfo set shares=shares/10, price=(price*10) where ticker='%s'", ticker.c_str());

  db.query("update stockhistory set price=price*10 where ticker='%s'",
	   ticker.c_str());
}

void updateStocks()
{
  TDatabase db(DB_SNEEZY), stocks(DB_SNEEZY);

  stocks.query("select ticker, shares, price, (((random() * volatility * 2) - volatility) / 10000.0) as pricechange from stockinfo");

  while(stocks.fetchRow()){
    double pricechange=1.0+convertTo<float>(stocks["pricechange"]);
    double price=convertTo<float>(stocks["price"]);
    double newprice=price*pricechange;
    sstring ticker=stocks["ticker"];
    int shares=convertTo<int>(stocks["shares"]);

    if(shares<=0)
      continue;

    db.query("update stockinfo set price=%f where ticker='%s'",
	     newprice, ticker.c_str());

    if((newprice) < ::number(1,5)){
      //      stockReverseSplit(ticker, newprice);
    } else if(newprice > ::number(100, 200)){
      //      stockSplit(ticker, newprice);
    }

  }
}

float getAskPrice(float price)
{
  return price*1.03;
}

float getBidPrice(float price)
{
  return price*0.97;
}

int stockBoard(TBeing *ch, cmdTypeT cmd, const char *arg, TObj *o1, TObj *o2)
{
  int found=0;
  TThing *o;
  TObj *to;
  TDatabase db(DB_SNEEZY);
  float price, pricediff;
  int shares;

  if(cmd != CMD_LOOK)
    return FALSE;

  for (o = ch->roomp->getStuff(); o; o = o->nextThing) {
    to = dynamic_cast<TObj *>(o);
    if (to && to->spec == SPEC_STOCK_BOARD &&
	isname(arg, to->name)){
      found=1;
      break;
    }
  }

  if(!found)
    return FALSE;

  ch->sendTo("You examine the board:\n\r");
  ch->sendTo("------------------------------------------------------------\n\r");
  db.query("select sum(sh.price*si.shares)/sum(si.shares) as dayprice, sum(si.price*si.shares)/sum(si.shares) as price from stockinfo si, stockhistory sh, (select ticker, max(n) as n from stockhistory group by ticker) sn where sh.n=sn.n and si.ticker=sh.ticker");
  
  if(db.fetchRow()){
    price=convertTo<float>(db["price"]);
    pricediff=price-convertTo<float>(db["dayprice"]);
    
    ch->sendTo(COLOR_BASIC, fmt("Market: <Y>%.2f  %s%+.2f<1>\n\r") %
	       price % (pricediff>0?"<G>":"<R>") %
	       (price - convertTo<float>(db["dayprice"])));
  }
			


  ch->sendTo("------------------------------------------------------------\n\r");
  ch->sendTo("Ticker   Bid    Ask  Change  Market Cap                                \n\r");
  ch->sendTo("------------------------------------------------------------\n\r");
  
  db.query("select distinct si.ticker, si.price, sh.price as dayprice, si.shares from stockinfo si, stockhistory sh, (select ticker, max(n) as n from stockhistory group by ticker) sn where sh.n=sn.n and si.ticker=sh.ticker and sn.ticker=si.ticker");


  while(db.fetchRow()){
    price=convertTo<float>(db["price"]);
    shares=convertTo<int>(db["shares"]);
    pricediff=price-convertTo<float>(db["dayprice"]);

    ch->sendTo(COLOR_BASIC, fmt("%-6s<Y>%6.2f%8.2f<1>%s%+8.2f<1>   %6s\n\r") %
	       db["ticker"] % getBidPrice(price) % getAskPrice(price) % 
	       (pricediff>0?"<G>":"<R>") % pricediff %
	       talenDisplay((int)(price*(float)shares)));
  }


  ch->sendTo("------------------------------------------------------------\n\r");
  ch->sendTo("Price charts can be seen at: http://sneezy.saw.net/peel/stocks/\n\r");
  ch->sendTo("------------------------------------------------------------\n\r");

  return TRUE;
}

TObj *createReport(const sstring &name, const sstring &content)
{
  TObj *note;

  if (!(note = read_object(GENERIC_NOTE, VIRTUAL))) {
    vlogf(LOG_BUG, "Couldn't make a note for stockbroker!");
    return NULL;
  }
  
  note->swapToStrung();
  delete [] note->name;
  note->name = mud_str_dup(name);
  delete [] note->shortDescr;
  note->shortDescr = mud_str_dup(fmt("a <W>%s<1>") % name); 
  delete [] note->getDescr();
  note->setDescr(mud_str_dup(fmt("A crumpled <W>%s<1> lies here.") % name));

  delete [] note->action_description;
  note->action_description = mud_str_dup(content);

  return note;
}


int stockBroker(TBeing *ch, cmdTypeT cmd, const char *argument, TMonster *me, TObj *)
{
  TDatabase db(DB_SNEEZY);
  sstring arg=argument, buf;
  float price, basis, pricediff;
  int shares;
  TObj *note;

  if(cmd != CMD_LIST && cmd != CMD_BUY && cmd != CMD_SELL)
    return FALSE;

  // list -> show portfolio
  // list <ticker>-> stock detail
  // buy <amt>*<ticker> -> buy stock
  // sell <amt>*<ticker> -> sell stock


  ///////////////////////////////// list
  if(cmd==CMD_LIST){
    ///////////////////////////////// list portfolio
    if(arg.empty()){
      db.query("select so.ticker, so.shares, so.cost_basis, s.price from stockowner so, stockinfo s where s.ticker=so.ticker and so.player_id=%i", ch->getPlayerID());
      
      buf="ticker shares basis price change worth change\n\r";

      while(db.fetchRow()){
	price=convertTo<float>(db["price"]);
	basis=convertTo<float>(db["cost_basis"]);
	pricediff=basis-price;
	shares=convertTo<int>(db["shares"]);

	buf += fmt("%-6s %6s %6.2f %6.2f %s%+.2f%c<1> %s %s%+i<1>\n") %
	  db["ticker"] % talenDisplay(convertTo<int>(db["shares"])) %
	  basis %
	  getBidPrice(price) %
	  (pricediff>0?"<G>":"<R>") % (((price-basis)/basis)*100)%'%'%
	  talenDisplay((int)((float)shares * price)) %
	  (pricediff>0?"<G>":"<R>") % 
	  (int)(pricediff*convertTo<int>(db["shares"]));
	
      }		   

      note=createReport("portfolio report", buf);
      *me += *note;
      me->doGive(ch, note, GIVE_FLAG_DROP_ON_FAIL);
      ///////////////////////////////// list stock info
    } else {
      db.query("select ticker, price, volatility, shares, descr from stockinfo where ticker=upper('%s')", arg.word(0).c_str());
      
      if(!db.fetchRow()){
	me->doTell(ch->getName(), "I don't have any data on that stock.");
	return TRUE;
      }

      
      buf = fmt("%s selling at %f with %i shares outstanding.") % 
	db["ticker"] % getAskPrice(convertTo<float>(db["price"])) %
	convertTo<int>(db["shares"]);
      buf += db["descr"];
      
      note=createReport("stock report", buf);
      *me += *note;
      me->doGive(ch, note, GIVE_FLAG_DROP_ON_FAIL);    
    }
    ///////////////////////////////// buy
  } else if(cmd == CMD_BUY){
    me->doTell(ch->getName(), "I'm not selling anything yet");
  }


  return TRUE;
}
