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

