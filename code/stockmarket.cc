#include "stdsneezy.h"
#include "database.h"

void updateStockHistory()
{
  TDatabase db(DB_SNEEZY);

  db.query("insert into stockhistory select (select max(n)+1 from stockhistory), ticker, price from stockinfo group by ticker, price");

  db.query("insert into stockhistory select (select max(n) from stockhistory), 'INDX', sum(price*shares)/sum(shares) from stockinfo");
}


void stockSplit(sstring ticker, float pricechange)
{
  TDatabase db(DB_SNEEZY);

  db.query("update stockinfo set shares=shares*2, price=(price/2)+%f where ticker='%s'", pricechange, ticker.c_str());
  
  db.query("update stockhistory set price=price/2 where ticker='%s'",
  	   ticker.c_str());

  //  db.query("insert into stockhistory select max(n), ticker, max(price) from stockhistory where ticker='%s' group by ticker", ticker.c_str());
  //  db.query("insert into stockhistory select max(n), ticker, min(price) from stockhistory where ticker='%s' group by ticker", ticker.c_str());
}

void stockReverseSplit(sstring ticker, float pricechange)
{
  TDatabase db(DB_SNEEZY);

  db.query("update stockinfo set shares=shares/10, price=(price*2)+%f where ticker='%s'", pricechange, ticker.c_str());

  db.query("update stockhistory set price=price*10 where ticker='%s'",
	   ticker.c_str());
    
  //  db.query("insert into stockhistory select max(n), ticker, max(price) from stockhistory where ticker='%s' group by ticker", ticker.c_str());
  //  db.query("insert into stockhistory select max(n), ticker, min(price) from stockhistory where ticker='%s' group by ticker", ticker.c_str());
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

    //    if((newprice) < ::number(1,5)){
      //      stockReverseSplit(ticker, pricechange);
    //    } else if(newprice > ::number(100, 125)){
      //      stockSplit(ticker, pricechange);
    //    } else {
      db.query("update stockinfo set price=%f where ticker='%s'",
	       newprice, ticker.c_str());
      //    }
    
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

    ch->sendTo(COLOR_BASIC, fmt("%-6s  <Y>%.2f  %.2f<1>  %s%+.2f<1>   %s\n\r") %
	       db["ticker"] % getBidPrice(price) % getAskPrice(price) % 
	       (pricediff>0?"<G>":"<R>") % pricediff %
	       talenDisplay((int)(price*(float)shares)));
  }


  ch->sendTo("------------------------------------------------------------\n\r");
  ch->sendTo("Price charts can be seen at: http://sneezy.saw.net/peel/stocks/\n\r");
  ch->sendTo("------------------------------------------------------------\n\r");

  return TRUE;
}



int stockBroker(TBeing *ch, cmdTypeT cmd, const char *argument, TMonster *me, TObj *)
{
  TDatabase db(DB_SNEEZY);
  sstring arg=argument;

  if(cmd != CMD_LIST && cmd != CMD_BUY && cmd != CMD_SELL)
    return FALSE;

  // list -> point them at stock board
  // buy info -> load up a little pamphlet with stock info and give for 100
  // buy <amt>*<ticker> -> buy stock
  // sell <amt>*<ticker> -> sell stock



  if(cmd==CMD_LIST){
    if(arg.empty()){
      me->doTell(ch->getName(), "Have a look at the stock board if you want to see current listings.");
      me->doTell(ch->getName(), "You can get a prospectus on a particular stock for 100 talens, with 'buy info <ticker>'.");
      return TRUE;
    }
  }

  if(cmd==CMD_BUY){
    if(arg.word(0) == "info"){
      db.query("select ticker, price, volatility, shares, descr from stockinfo where ticker=upper('%s')", arg.c_str());
      
      if(!db.fetchRow()){
	me->doTell(ch->getName(), "I don't seem to have any data on that stock.");
	return TRUE;
      }
      
      me->doTell(ch->getName(), fmt("%s selling at %f with %i shares outstanding.") % 
		 db["ticker"] % getAskPrice(convertTo<float>(db["price"])) %
		 convertTo<int>(db["shares"]));
      me->doTell(ch->getName(), db["descr"]);
    }


    return TRUE;
  }

  


  return FALSE;
}
