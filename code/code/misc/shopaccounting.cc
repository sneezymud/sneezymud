#include "shopowned.h"
#include "database.h"
#include "shop.h"
#include "extern.h"
#include "corporation.h"
#include "obj_note.h"
#include "shopaccounting.h"
#include "monster.h"
#include "room.h"

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
    if(db["name"] == "Retained Earnings"){
      values[db["name"]]=convertTo<int>(db["amt"]);
    } else {
      values[db["name"]]=abs(convertTo<int>(db["amt"]));
    }
  }

  shop_nr=shop;
  year=y;
}

// pull current data
TShopJournal::TShopJournal(int shop)
{
  TDatabase db(DB_SNEEZY);
  year=GameTime::getYear();

  db.query("select a.name, sum(credit)-sum(debit) as amt from shoplogjournal, shoplogaccountchart a where shop_nr=%i and a.post_ref=shoplogjournal.post_ref group by a.name", shop);

  while(db.fetchRow()){
    if(db["name"] == "Retained Earnings"){
      values[db["name"]]=convertTo<int>(db["amt"]);
    } else {
      values[db["name"]]=abs(convertTo<int>(db["amt"]));
    }
  }
  
  shop_nr=shop;
}

int TShopJournal::getValue(const sstring &val)
{
  return values[val];
}


int TShopJournal::getExpenses()
{
  return values["COGS"]+values["Tax"]+values["Expenses"]+values["Interest"];
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
  return values["Deposits"];
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

  // have to clear out any cached shop logs first
  while(!queryqueue.empty()){
    db.query(queryqueue.front().c_str());
    queryqueue.pop();
  }

  if(year == GameTime::getYear()){
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

  //// assets
  // carryover entry for cash
  tso.journalize_debit(100, "Accountant", "Year End Accounting", 
			getValue("Cash"), true);
  // carryover entry for inventory
  tso.journalize_debit(130, "Accountant", "Year End Accounting", 
			getValue("Inventory"));

  //// liabilities
  // carryover entry for deposits
  tso.journalize_credit(310, "Accountant", "Year End Accounting",
			getValue("Deposits"));

  // carryover entry for PIC
  tso.journalize_credit(300, "Accountant", "Year End Accounting", 
			getValue("Paid-in Capital"));
  // carryover entry for RE
  // sometimes RE can be negative, so debit if needed
  if(getRetainedEarnings() >= 0){
    tso.journalize_credit(800, "Accountant", "Year End Accounting", 
			  getRetainedEarnings());
  } else {
    tso.journalize_debit(800, "Accountant", "Year End Accounting", 
			  -getRetainedEarnings());
  }

  // move old journal into archive
  db.query("insert into shoplogjournalarchive select * from shoplogjournal where shop_nr=%i and sneezy_year=%i", shop_nr, year);
  db.query("delete from shoplogjournal where shop_nr=%i and sneezy_year=%i", shop_nr, year);
}


void TShopOwned::journalize_debit(int post_ref, const sstring &customer,
				  const sstring &name, int amt, bool new_id)
{
  TDatabase db(DB_SNEEZY);

  //    db.query("insert into shoplogjournal (shop_nr, journal_id, customer_name, obj_name, sneezy_year, logtime, post_ref, debit, credit) values (%i, %s, '%s', '%s', %i, now(), %i, %i, 0)", shop_nr, (new_id?"NULL":"LAST_INSERT_ID()"), customer.c_str(), name.c_str(), GameTime::getYear(), post_ref, amt);

  queryqueue.push(format("insert into shoplogjournal (shop_nr, journal_id, customer_name, obj_name, sneezy_year, logtime, post_ref, debit, credit) values (%i, %s, '%s', '%s', %i, now(), %i, %i, 0)") % shop_nr % ((sstring)(new_id?"NULL":"LAST_INSERT_ID()")).escape(sstring::SQL) % customer.escape(sstring::SQL) % name.escape(sstring::SQL) % GameTime::getYear() % post_ref % amt);
}
				  
void TShopOwned::journalize_credit(int post_ref, const sstring &customer,
				  const sstring &name, int amt, bool new_id)
{
  TDatabase db(DB_SNEEZY);

  queryqueue.push(format("insert into shoplogjournal (shop_nr, journal_id, customer_name, obj_name, sneezy_year, logtime, post_ref, debit, credit)values (%i, %s, '%s', '%s', %i, now(), %i, 0, %i)") % shop_nr % ((sstring)(new_id?"NULL":"LAST_INSERT_ID()")).escape(sstring::SQL) % customer.escape(sstring::SQL) % name.escape(sstring::SQL) % GameTime::getYear() % post_ref % amt);
}

void TShopOwned::COGS_add(const sstring &name, int amt, int num)
{
  TDatabase db(DB_SNEEZY);


  db.query("select 1 from shoplogcogs where obj_name='%s' and shop_nr=%i", name.c_str(), shop_nr);

  if(!db.fetchRow()){
    // this needs to be done immediately, otherwise there will be multiple
    // inserts queued up the next time COGS_add() is called, if the queue
    // hasn't been processed yet.
    db.query("insert into shoplogcogs (shop_nr, obj_name, count, total_cost) values (%i, '%s', %i, %i)", shop_nr, name.escape(sstring::SQL).c_str(), num, amt);
    //    queryqueue.push(format("insert into shoplogcogs (shop_nr, obj_name, count, total_cost) values (%i, '%s', %i, %i)") % shop_nr % name.escape(sstring::SQL) % num % amt);
  } else {
    queryqueue.push(format("update shoplogcogs set count=count+%i, total_cost=total_cost+%i where obj_name='%s' and shop_nr=%i") % num % amt % name.escape(sstring::SQL) % shop_nr);
  }
}

void TShopOwned::COGS_remove(const sstring &name, int num)
{
  TDatabase db(DB_SNEEZY);

  //  db.query("update shoplogcogs set total_cost=total_cost-((total_cost/count)*%i), count=count-%i where obj_name='%s' and shop_nr=%i", num, num, name.c_str(), shop_nr);

  queryqueue.push(format("update shoplogcogs set total_cost=floor(total_cost-(total_cost/count)*%i), count=count-%i where obj_name='%s' and shop_nr=%i") % num % num % name.escape(sstring::SQL) % shop_nr);
}

int TShopOwned::COGS_get(const sstring &name, int num)
{
  TDatabase db(DB_SNEEZY);

  db.query("select (total_cost/count)*%i as cost from shoplogcogs where shop_nr=%i and obj_name='%s'", num, shop_nr, name.c_str());
  
  if(db.fetchRow())
    return convertTo<int>(db["cost"]);
  else
    return 0;
}

void TShopOwned::journalize(const sstring &customer, const sstring &name, 
			    transactionTypeT action, 
			    int amt, int tax, int corp_cash, 
			    int expenses, int num)
{
  TDatabase db(DB_SNEEZY);
  int COGS=0;

  switch(action){
    case TX_RECEIVING_TALENS:
      // shop giving money to owner
      // we might want to record this as salary or something?
      // perhaps we need a way for owners to differentiate between PIC and 
      // salary withdrawals
      
      // PIC
      journalize_debit(300, customer, name, amt, true);
      // cash
      journalize_credit(100, customer, name, amt);
      break;
    case TX_GIVING_TALENS:
      // owner giving money to the shop
      // cash
      journalize_debit(100, customer, name, amt, true);
      // PIC
      journalize_credit(300, customer, name, amt);
      break;
    case TX_DEPOSIT:
      // cash
      journalize_debit(100, customer, name, amt, true);
      // deposits
      journalize_credit(310, customer, name, amt);
      break;
    case TX_WITHDRAWAL:
      // deposits
      journalize_debit(310, customer, name, amt, true);
      // cash
      journalize_credit(100, customer, name, amt);
      break;
    case TX_PAYING_INTEREST:
      // interest
      journalize_debit(610, customer, name, amt, true);      
      // cash
      journalize_credit(100, customer, name, amt);      
      break;
    case TX_FACTORY:
      break;
    case TX_SELLING:
    case TX_PRODUCING:
      // player selling something, so shop is buying inventory
      // inventory
      journalize_debit(130, customer, name, amt, true);
      // cash
      journalize_credit(100, customer, name, amt);
      
      // record COGS
      COGS_add(name, amt, num);
      break;
    case TX_BUYING_SERVICE:
    case TX_BUYING:
    case TX_RECYCLING:
      // first the easy part
      // cash
      journalize_debit(100, customer, name, amt, true);
      // sales
      if(action == TX_RECYCLING)
	journalize_credit(510, customer, name, amt);
      else
	journalize_credit(500, customer, name, amt);
      
      if(action == TX_BUYING_SERVICE){
      } else if(action == TX_BUYING || action == TX_RECYCLING){
	// now we have to calculate COGS for this item
	// (COGS = cost of goods sold)
	COGS=COGS_get(name, num);
	
	// now log it
	// COGS
	journalize_debit(600, customer, name, COGS);
	// inventory
	journalize_credit(130, customer, name, COGS);

	// now update COGS table
	COGS_remove(name, num);
      }
      
      
      break;
  }

  // now we log miscellaneous things that apply to everything if passed


  ///// log any expenses
  if(expenses){
    // expenses
    journalize_debit(630, customer, name, expenses);
    // cash
    journalize_credit(100, customer, name, expenses);
  }

  ///// now log the sales tax
  if(tax){
    // tax
    journalize_debit(700, customer, name, tax);
    // cash
    journalize_credit(100, customer, name, tax);
  }      


  ///// now log the corporate cash flow
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
}


void TShopOwned::giveStatements(sstring arg)
{
  if(!hasAccess(SHOPACCESS_LOGS)){
    keeper->doTell(ch->getName(), "Sorry, you don't have access to do that.");
    return;
  }

  int year=convertTo<int>(arg);
  if(!year)
    year=GameTime::getYear();

  TShopJournal tsj(shop_nr, year);
  sstring keywords, short_desc, long_desc, buf, name;
  
  name=real_roomp(shop_index[shop_nr].in_room)->getName();
  keywords=format("statement income financial %i %i %s") % 
    shop_nr % year % name;
  short_desc=format("an income statement for '<p>%s<1>', year <r>%i<1>") %
    name % year;
  long_desc="A crumpled up financial statement lies here.";

  if(year == GameTime::getYear())
    buf=format("Income statement for '%s', current year %i.\n\r") % 
      name % year;
  else
    buf=format("Income statement for '%s', year ending %i.\n\r") % 
      name % year;

  sstring prev_re=format("Retained earnings %i") % (year-1);

  buf+="-----------------------------------------------------------------\n\r";
  buf+=format("%-36s %10s %10i\n\r") % 
    "Sales revenue" % "" % tsj.getValue("Sales");
  if(tsj.getValue("Recycling"))
    buf+=format("%-36s %10s %10i\n\r") % 
      "Recycling revenue" % "" % tsj.getValue("Recycling");
  buf+=format("  %-34s %10i\n\r") %
    "Cost of goods sold" % tsj.getValue("COGS");
  buf+=format("  %-34s %10i\n\r") %
    "Sales tax" % tsj.getValue("Tax");
  if(tsj.getValue("Expenses"))
    buf+=format("  %-34s %10i\n\r") %
      "Service expenses" % tsj.getValue("Expenses");
  if(tsj.getValue("Interest"))
    buf+=format("  %-34s %10i\n\r") %
      "Interest expense" % tsj.getValue("Interest");
  buf+=format("%-36s %10s %10i\n\r") %
    "Total expenses" % "" % tsj.getExpenses();
  buf+=format("%-36s %10s %10s\n\r") % "" % "----------" % "----------";
  buf+=format("%-36s %10s %10i\n\r") %
    "Net income" % "" % tsj.getNetIncome();
  buf+=format("%-36s %10s %10i\n\r") %
    "Dividends" % "" % tsj.getValue("Dividends");
  buf+=format("%-36s %10s %10i\n\r") %
    prev_re % "" % tsj.getValue("Retained Earnings");
  buf+=format("%-36s %10s %10s\n\r") % "" % "----------" % "----------";
  buf+=format("%-36s %10s %10i\n\r") %
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
  keywords=format("sheet balance financial statement %i %i %s") % 
    shop_nr % year % name;
  short_desc=format("a balance sheet for '<p>%s<1>', year <r>%i<1>") %
    name % year;
  long_desc="A crumpled up financial statement lies here.";


  if(year == GameTime::getYear())
    buf=format("Balance sheet for '%s', current year %i.\n\r\n\r") % 
      name % year;
  else
    buf=format("Balance sheet for '%s', year ending %i.\n\r\n\r") % 
      name % year;

  buf+=format("%-36s   %-36s\n\r") % 
    "Assets" % "Liabilities";
  buf+="-----------------------------------------------------------------\n\r";
  
  if(tsj.getValue("Deposits")){
    buf+=format("%-36s | %-25s\n\r") %
      "" % "Liabilities";
    buf+=format("%-25s %10i | %-25s %10i\n") %
      "Cash" % tsj.getValue("Cash") % 
      "  Deposits" % tsj.getValue("Deposits");
  } else {
    buf+=format("%-25s %10i |\n") %
      "Cash" % tsj.getValue("Cash");
  }

  buf+=format("%-25s %10i | %-36s\n\r") %
    "Inventory" % tsj.getValue("Inventory") % "";
  buf+=format("%-36s | %-36s\n\r") %
    "" % "Shareholders' equity";
  buf+=format("%-36s | %-25s %10i\n\r") %
    "" % "  Paid-in capital" % tsj.getValue("Paid-in Capital");
  buf+=format("%-36s | %-25s %10i\n\r") %
    "" % "  Retained earnings" % tsj.getRetainedEarnings();
  buf+=format("%-25s %10s | %-25s %10s\n\r") %
    "" % "----------" % "" % "----------";
  buf+=format("%-25s %10i | %-25s %10i\n\r") %
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
