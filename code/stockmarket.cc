#include "stdsneezy.h"
#include "database.h"

void updateStockHistory()
{
  TDatabase db(DB_SNEEZY);

  // stockhistory table needs to be seeded with at least one entry for
  // each stock, for this query to work
  db.query("insert into stockhistory select sh.n+1, s.ticker, s.price from stockinfo s left outer join (select ticker, max(n) as n from stockhistory group by ticker) sh on (s.ticker=sh.ticker)");
}

void updateStocks()
{
  TDatabase db(DB_SNEEZY);
  float delta=0.0;

  db.query("select ticker from stockinfo");

  while(db.fetchRow()){
    delta=(::number(-100, 100) / 100.0);

    db.query("update stockinfo set price=round(price+%f, 2) where ticker='%s'",
	     delta, db["ticker"].c_str());
  }
}
