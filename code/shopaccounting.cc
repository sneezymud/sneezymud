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

  db.query("select 1 from shoplogjournal where shop_nr=%i and sneezy_year=%i", shop, y);

  if(db.fetchRow()){
    db.query("select a.name, sum(credit)-sum(debit) as amt from shoplogjournal, shoplogaccountchart a where sneezy_year=%i and shop_nr=%i and a.post_ref=shoplogjournal.post_ref group by a.name", y, shop);
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
  return (values["Sales"]+values["Recycling"])-getExpenses();
}


int TShopJournal::getRetainedEarnings()
{
  return (getNetIncome()+values["Retained Earnings"])-values["Dividends"];
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

  // we have to assume here that all of the journal entries for the
  // specified year exist in the shoplogjournal table

  if(year == time_info.year){
    vlogf(LOG_BUG, "closeTheBooks() called for current year!");
    return;
  }

  // shouldn't be an entries for last year in here if books have been closed
  db.query("select 1 from shoplogjournal where shop_nr=%i and sneezy_year=%i", shop_nr, year);
  
  if(!db.fetchRow()){
    // seems as the books have already been closed.
    //    vlogf(LOG_BUG, "closeTheBooks() called when retained earnings already set!");
    return;
  }

  TShopOwned tso(shop_nr, NULL, NULL);

  // carryover entry for cash
  tso.journalize_debit(100, "Accountant", "Year End Accounting", 
			getValue("Cash"), true);
  // carryover entry for inventory
  tso.journalize_debit(130, "Accountant", "Year End Accounting", 
			getValue("Inventory"));
  // carryover entry for PIC
  tso.journalize_credit(300, "Accountant", "Year End Accounting", 
			getValue("Paid-in Capital"));
  // carryover entry for RE
  tso.journalize_credit(800, "Accountant", "Year End Accounting", 
			getRetainedEarnings());

  // move old journal into archive
  db.query("insert into shoplogjournalarchive select * from shoplogjournal where shop_nr=%i and sneezy_year=%i", shop_nr, year);
  db.query("delete from shoplogjournal where shop_nr=%i and sneezy_year=%i", shop_nr, year);
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
  } else if(action == "selling" || action == "producing"){ 
    // player selling something, so shop is buying inventory
    // inventory
    journalize_debit(130, customer, name, amt, true);
    // cash
    journalize_credit(100, customer, name, amt);
    
    // record COGS
    COGS_add(name, amt);
  } else if(action == "buying service" || 
	    action == "buying" ||
	    action == "recycling"){
    // first the easy part
    // cash
    journalize_debit(100, customer, name, amt, true);
    // sales
    if(action == "recycling")
      journalize_credit(510, customer, name, amt);
    else
      journalize_credit(500, customer, name, amt);

    int COGS=0;

    if(action == "buying service"){
      // expenses
      journalize_debit(630, customer, name, expenses);
      // cash
      journalize_credit(100, customer, name, expenses);
    } else if(action == "buying" || action == "recycling"){
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


void TShopOwned::giveStatements(sstring arg)
{
  if(!hasAccess(SHOPACCESS_LOGS)){
    keeper->doTell(ch->getName(), "Sorry, you don't have access to do that.");
    return;
  }

  int year=convertTo<int>(arg);
  if(!year)
    year=time_info.year;

  TShopJournal tsj(shop_nr, year);
  sstring keywords, short_desc, long_desc, buf, name;
  
  name=real_roomp(shop_index[shop_nr].in_room)->getName();
  keywords=fmt("statement income financial %i %i %s") % 
    shop_nr % year % name;
  short_desc=fmt("an income statement for '<p>%s<1>', year <r>%i<1>") %
    name % year;
  long_desc="A crumpled up financial statement lies here.";

  if(year == time_info.year)
    buf=fmt("Income statement for '%s', current year %i.\n\r") % 
      name % year;
  else
    buf=fmt("Income statement for '%s', year ending %i.\n\r") % 
      name % year;

  sstring prev_re=fmt("Retained earnings %i") % (year-1);

  buf+="-----------------------------------------------------------------\n\r";
  buf+=fmt("%-36s %10s %10i\n\r") % 
    "Sales revenue" % "" % tsj.getValue("Sales");
  buf+=fmt("%-36s %10s %10i\n\r") % 
    "Recycling revenue" % "" % tsj.getValue("Recycling");
  buf+=fmt("  %-34s %10i\n\r") %
    "Cost of goods sold" % tsj.getValue("COGS");
  buf+=fmt("  %-34s %10i\n\r") %
    "Sales tax" % tsj.getValue("Tax");
  buf+=fmt("  %-34s %10i\n\r") %
    "Service expenses" % tsj.getValue("Expenses");
  buf+=fmt("%-36s %10s %10i\n\r") %
    "Total expenses" % "" % tsj.getExpenses();
  buf+=fmt("%-36s %10s %10s\n\r") % "" % "----------" % "----------";
  buf+=fmt("%-36s %10s %10i\n\r") %
    "Net income" % "" % tsj.getNetIncome();
  buf+=fmt("%-36s %10s %10i\n\r") %
    "Dividends" % "" % tsj.getValue("Dividends");
  buf+=fmt("%-36s %10s %10i\n\r") %
    prev_re % "" % tsj.getValue("Retained Earnings");
  buf+=fmt("%-36s %10s %10s\n\r") % "" % "----------" % "----------";
  buf+=fmt("%-36s %10s %10i\n\r") %
    "Retained earnings" % "" % tsj.getRetainedEarnings();
  
  TNote *income_statement = createNote(mud_str_dup(buf));
  delete [] income_statement->name;
  income_statement->name = mud_str_dup(keywords);
  delete [] income_statement->shortDescr;
  income_statement->shortDescr = mud_str_dup(short_desc);
  delete [] income_statement->getDescr();
  income_statement->setDescr(mud_str_dup(long_desc));

  *keeper += *income_statement;
  keeper->doGive(ch, income_statement, GIVE_FLAG_DROP_ON_FAIL);


  name=real_roomp(shop_index[shop_nr].in_room)->getName();
  keywords=fmt("sheet balance financial statement %i %i %s") % 
    shop_nr % year % name;
  short_desc=fmt("a balance sheet for '<p>%s<1>', year <r>%i<1>") %
    name % year;
  long_desc="A crumpled up financial statement lies here.";


  if(year == time_info.year)
    buf=fmt("Balance sheet for '%s', current year %i.\n\r\n\r") % 
      name % year;
  else
    buf=fmt("Balance sheet for '%s', year ending %i.\n\r\n\r") % 
      name % year;

  buf+=fmt("%-36s   %-36s\n\r") % 
    "Assets" % "Liabilities";
  buf+="-----------------------------------------------------------------\n\r";
  buf+=fmt("%-36s | %-25s\n\r") %
    "" % "Liabilities";
  buf+=fmt("%-25s %10i | %31s\n") %
    "Cash" % tsj.getValue("Cash") % "";
  buf+=fmt("%-25s %10i | %-36s\n\r") %
    "Inventory" % tsj.getValue("Inventory") % "Shareholders' equity";
  buf+=fmt("%-36s | %-25s %10i\n\r") %
    "" % "  Paid-in capital" % tsj.getValue("Paid-in Capital");
  buf+=fmt("%-36s | %-25s %10i\n\r") %
    "" % "  Retained earnings" % tsj.getRetainedEarnings();
  buf+=fmt("%-25s %10s | %-25s %10s\n\r") %
    "" % "----------" % "" % "----------";
  buf+=fmt("%-25s %10i | %-25s %10i\n\r") %
    "Total assets" % tsj.getAssets() %
    "Total liabilities & SHE" % 
    (tsj.getLiabilities()+tsj.getShareholdersEquity());

  
  TNote *balance_sheet = createNote(mud_str_dup(buf));
  delete [] balance_sheet->name;
  balance_sheet->name = mud_str_dup(keywords);
  delete [] balance_sheet->shortDescr;
  balance_sheet->shortDescr = mud_str_dup(short_desc);
  delete [] balance_sheet->getDescr();
  balance_sheet->setDescr(mud_str_dup(long_desc));

  *keeper += *balance_sheet;
  keeper->doGive(ch, balance_sheet, GIVE_FLAG_DROP_ON_FAIL);

}
