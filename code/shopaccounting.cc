#include "stdsneezy.h"
#include "shopowned.h"
#include "database.h"
#include "shop.h"
#include "corporation.h"
#include "obj_note.h"
#include "shopaccounting.h"

// pull data from archive
TShopJournal::TShopJournal(int shop, int y)
{
  TDatabase db(DB_SNEEZY);

  if(y == time_info.year){
    db.query("select a.name, sum(credit)-sum(debit) as amt from shoplogjournal, shoplogaccountchart a where shop_nr=%i and a.post_ref=shoplogjournal.post_ref group by a.name", shop);
  } else {
    db.query("select a.name, sum(credit)-sum(debit) as amt from shoplogjournalarchive, shoplogaccountchart a where sneezy_year=%i and shop_nr=%i and a.post_ref=shoplogjournalarchive.post_ref group by a.name", y, shop);
  }

  while(db.fetchRow()){
    values[db["name"]]=abs(convertTo<int>(db["amt"]));
  }

  shop_nr=shop;
  year=y;
}

// pull current data
TShopJournal::TShopJournal(int shop)
{
  TDatabase db(DB_SNEEZY);
  year=time_info.year;

  db.query("select a.name, sum(credit)-sum(debit) as amt from shoplogjournal, shoplogaccountchart a where shop_nr=%i and a.post_ref=shoplogjournal.post_ref group by a.name", shop);

  while(db.fetchRow()){
    values[db["name"]]=abs(convertTo<int>(db["amt"]));
  }
  
  shop_nr=shop;
}

int TShopJournal::getValue(const sstring &val)
{
  return values[val];
}

int TShopJournal::getExpenses()
{
  return values["COGS"]+values["Tax"]+values["Expenses"];
}

int TShopJournal::getNetIncome()
{
  return values["Sales"]-getExpenses();
}

int TShopJournal::getPrevRetainedEarnings()
{
  TDatabase db(DB_SNEEZY);
  
  db.query("select retained_earnings from shoplog_retained_earnings where shop_nr=%i and sneezy_year=%i", shop_nr, year-1);
  db.fetchRow();

  return convertTo<int>(db["retained_earnings"]);
}

int TShopJournal::getRetainedEarnings()
{
  return (getNetIncome()+getPrevRetainedEarnings())-values["Dividends"];
}

int TShopJournal::getAssets()
{
  return values["Cash"]+values["Inventory"];
}

int TShopJournal::getLiabilities()
{
  // no debt or anything yet!  no liabilities
  return 0;
}

int TShopJournal::getShareholdersEquity()
{
  return values["Paid-in Capital"]+getRetainedEarnings();
}


void TShopJournal::closeTheBooks()
{
  TDatabase db(DB_SNEEZY);

  db.query("insert into shoplog_retained_earnings (shop_nr, retained_earnings, sneezy_year) values (%i, %i, %i)", shop_nr, getRetainedEarnings(), year);  
  db.query("insert into shoplogjournalarchive select * from shoplogjournal where shop_nr=%i and year=%i", shop_nr, year);
  db.query("delete from shoplogjournal where shop_nr=%i and year=%i", shop_nr, year);
}


void TShopOwned::journalize_debit(int post_ref, const sstring &customer,
				  const sstring &name, int amt, bool new_id)
{
  TDatabase db(DB_SNEEZY);

  db.query("insert into shoplogjournal (shop_nr, journal_id, customer_name, obj_name, sneezy_year, logtime, post_ref, debit, credit) values (%i, %s, '%s', '%s', %i, now(), %i, %i, 0)", shop_nr, (new_id?"NULL":"LAST_INSERT_ID()"), customer.c_str(), name.c_str(), time_info.year, post_ref, amt);
}
				  
void TShopOwned::journalize_credit(int post_ref, const sstring &customer,
				  const sstring &name, int amt, bool new_id)
{
  TDatabase db(DB_SNEEZY);

  db.query("insert into shoplogjournal (shop_nr, journal_id, customer_name, obj_name, sneezy_year, logtime, post_ref, debit, credit)values (%i, %s, '%s', '%s', %i, now(), %i, 0, %i)", shop_nr, (new_id?"NULL":"LAST_INSERT_ID()"), customer.c_str(), name.c_str(), time_info.year, post_ref, amt);
}

void TShopOwned::COGS_add(const sstring &name, int amt)
{
  TDatabase db(DB_SNEEZY);


  db.query("select 1 from shoplogcogs where obj_name='%s' and shop_nr=%i", name.c_str(), shop_nr);

  if(!db.fetchRow()){
    db.query("insert into shoplogcogs (shop_nr, obj_name, count, total_cost) values (%i, '%s', %i, %i)", shop_nr, name.c_str(), 1, amt);
  } else {
    db.query("update shoplogcogs set count=count+1, total_cost=total_cost+%i where obj_name='%s' and shop_nr=%i", amt, name.c_str(), shop_nr);
  }
}

void TShopOwned::COGS_remove(const sstring &name)
{
  TDatabase db(DB_SNEEZY);

  db.query("update shoplogcogs set total_cost=total_cost-(total_cost/count), count=count-1 where obj_name='%s' and shop_nr=%i", name.c_str(), shop_nr);
}

int TShopOwned::COGS_get(const sstring &name)
{
  TDatabase db(DB_SNEEZY);

  db.query("select total_cost/count as cost from shoplogcogs where shop_nr=%i and obj_name='%s'", shop_nr, name.c_str());
  
  if(db.fetchRow())
    return convertTo<int>(db["cost"]);
  else
    return 0;
}

void TShopOwned::journalize(const sstring &customer, const sstring &name, 
			    const sstring &action, 
			    int amt, int tax, int corp_cash, int expenses)
{
  TDatabase db(DB_SNEEZY);

  if(action == "receiving"){
    // shop giving money to owner
    // we might want to record this as salary or something?
    // perhaps we need a way for owners to differentiate between PIC and salary
    // withdrawals

    // PIC
    journalize_debit(300, customer, name, amt, true);
    // cash
    journalize_credit(100, customer, name, amt);
  } if(action == "giving"){
    // owner giving money to the shop
    // cash
    journalize_debit(100, customer, name, amt, true);
    // PIC
    journalize_credit(300, customer, name, amt);
  } else if(action == "selling"){ 
    // player selling something, so shop is buying inventory
    // inventory
    journalize_debit(130, customer, name, amt, true);
    // cash
    journalize_credit(100, customer, name, amt);
    
    // record COGS
    COGS_add(name, amt);

  } else if(action == "buying service" || action == "buying"){
    // first the easy part
    // cash
    journalize_debit(100, customer, name, amt, true);
    // sales
    journalize_credit(500, customer, name, amt);

    int COGS=0;

    if(action == "buying service"){
      // expenses
      journalize_debit(630, customer, name, expenses);
      // cash
      journalize_credit(100, customer, name, expenses);
    } else if(action == "buying"){
      // now we have to calculate COGS for this item
      // (COGS = cost of goods sold)
      COGS=COGS_get(name);

      // now log it
      // COGS
      journalize_debit(600, customer, name, COGS);
      // inventory
      journalize_credit(130, customer, name, COGS);
    }

    // now log the sales tax
    if(tax){
      // tax
      journalize_debit(700, customer, name, tax);
      // cash
      journalize_credit(100, customer, name, tax);
    }      

      // now log the corporate cash flow
    if(corp_cash > 0){
      // receiving money from corp, this counts as PIC
      // cash
      journalize_debit(100, customer, name, corp_cash);
      // PIC
      journalize_credit(300, customer, name, corp_cash);
    } else if (corp_cash < 0) {
      // giving money to corp, this counts as dividends
      // dividends
      journalize_debit(101, customer, name, -corp_cash);
      // cash
      journalize_credit(100, customer, name, -corp_cash);
    }

    // now log COGS
    COGS_remove(name);
  }
}

