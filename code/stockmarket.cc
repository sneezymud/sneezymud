#include "stdsneezy.h"
#include "database.h"

void updateStockHistory()
{
  TDatabase db(DB_SNEEZY);

  // stockhistory table needs to be seeded with at least one entry for
  // each stock, for this query to work
  db.query("insert into stockhistory select sh.n+1, s.ticker, s.price from stockinfo s left outer join (select ticker, max(n) as n from stockhistory group by ticker) sh on (s.ticker=sh.ticker)");
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

  db.query("update stockinfo set shares=shares/2, price=(price*2)+%f where ticker='%s'", pricechange, ticker.c_str());

  db.query("update stockhistory set price=price*2 where ticker='%s'",
	   ticker.c_str());
  
  //  db.query("insert into stockhistory select max(n), ticker, max(price) from stockhistory where ticker='%s' group by ticker", ticker.c_str());
  //  db.query("insert into stockhistory select max(n), ticker, min(price) from stockhistory where ticker='%s' group by ticker", ticker.c_str());
}

void updateStocks()
{
  TDatabase db(DB_SNEEZY), stocks(DB_SNEEZY);


  stocks.query("select ticker, shares, price, (((random() * volatility * 2) - volatility) / 100) as pricechange from stockinfo");

  while(stocks.fetchRow()){
    float pricechange=convertTo<float>(stocks["pricechange"]);
    float price=convertTo<float>(stocks["price"]);
    sstring ticker=stocks["ticker"];

    if((price+pricechange) < 10.0){
      stockReverseSplit(ticker, pricechange);
    } else if(price+pricechange > 100.0){
      stockSplit(ticker, pricechange);
    } else {
      db.query("update stockinfo set price=price+%f where ticker='%s'",
	       pricechange, ticker.c_str());
    }
    
  }


}
