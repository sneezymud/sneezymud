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

  db.query("update stockinfo set price=price + (((random() * volatility * 2) - volatility) / 100)");



}
