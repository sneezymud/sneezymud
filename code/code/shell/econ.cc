#include <stdio.h>
#include <iostream>
#include <map>
#include <string>

#include "database.h"
#include "configuration.h"
#include "extern.h"
#include "shopaccounting.h"

#include <boost/program_options.hpp>
namespace po = boost::program_options;

using namespace std;

//int shops[]={0, 1, 2, 4, 5, 8, 9, 10, 11, 12, 14, 36, 44, 73, 84, 85, 
//	     86, 87, 89, 90, 91, 92, 93, 94, 95, 127, 144, 
//	     150, 161, 184, 186, 214, -1};

int shops[]={0, 2, 4, 5, 9, 14, 36, 44, 73, 84, 85, 
	     86, 87, 89, 90, 91, 92, 93, 94, 95, 
	     150, 160, 161, 184, 186, 250, -1};

// the purpose of this is to check the sum of the credits and debits in the
// accounting journal and compare that to the cash that the shop actually has.
void check_cash_balance(){
  TDatabase db(DB_SNEEZY);

  cout << "Checking cash balances..." << endl;

  for(int i=0;shops[i]!=-1;++i){
    db.query("select sum(debit-credit) as sum from shoplogjournal where shop_nr=%i and post_ref=100", shops[i]);
    db.fetchRow();
    int balance_cash=convertTo<int>(db["sum"]);
    db.query("select gold from shopowned where shop_nr=%i", shops[i]);
    db.fetchRow();
    int shop_gold=convertTo<int>(db["gold"]);
    db.query("select r.name as name from room r, shop s where s.shop_nr=%i and r.vnum=s.in_room", shops[i]);
    db.fetchRow();
    sstring shop_name=db["name"];


    if(balance_cash != shop_gold){
      cout << format("%-30s (%3i): ") % shop_name % shops[i];
      if(balance_cash < shop_gold){
	cout << format("too much gold  : %10i\n") % (shop_gold-balance_cash);
      } else {
	cout << format("not enough gold: %10i\n") % (balance_cash-shop_gold);
      }
    }
  }
}

// the purpose of this is to check the value of inventory recorded in the
// accounting journal and compare it with what is recorded in the COGS table
void check_inventory_cogs(){
  TDatabase db(DB_SNEEZY);

  cout << "Checking COGS against journal entries..." << endl;

  for(int i=0;shops[i]!=-1;++i){
    db.query("select sum(debit-credit) as sum from shoplogjournal where shop_nr=%i and post_ref=130", shops[i]);
    db.fetchRow();
    int inventory=convertTo<int>(db["sum"]);
    db.query("select sum(total_cost) as sum from shoplogcogs where shop_nr=%i and count>0", shops[i]);
    db.fetchRow();
    int cogs=convertTo<int>(db["sum"]);
    db.query("select r.name as name from room r, shop s where s.shop_nr=%i and r.vnum=s.in_room", shops[i]);
    db.fetchRow();
    sstring shop_name=db["name"];

    if(inventory!=cogs){
      cout << format("%-30s (%3i): ") % shop_name % shops[i];
      if(inventory < cogs){
	cout << format("missing inventory: %10i\n") % (cogs-inventory);
      } else {
	cout << format("extra inventory  : %10i\n") % (inventory-cogs);
      }
    }
  }
}

// this checks the number of items recorded in COGS versus how many are in rent
void check_cogs_count(){
  TDatabase db(DB_SNEEZY);

  cout << "Checking COGS against rent..." << endl;

  for(int i=0;shops[i]!=-1;++i){
    // commod trader doesn't rent items
    if(shops[i]==250)
      continue;


    db.query("select r.name as name from room r, shop s where s.shop_nr=%i and r.vnum=s.in_room", shops[i]);
    db.fetchRow();    
    sstring shop_name=db["name"];

    db.query("select count(*) as count from shoplogcogs where count < 0 and shop_nr=%i", shops[i]);
    db.fetchRow();
    int itemcount=convertTo<int>(db["count"]);
    db.query("select sum(count) as count from shoplogcogs where shop_nr=%i", shops[i]);
    db.fetchRow();
    int cogscount=convertTo<int>(db["count"]);
    db.query("select count(*) as count from rent where owner_type='shop' and owner=%i", shops[i]);
    db.fetchRow();
    int rentcount=convertTo<int>(db["count"]);

    if(cogscount != rentcount){
      cout << format("%-30s (%3i): ") % shop_name % shops[i];
    
      if(itemcount != 0){
	cout << format("negative count in COGS on %i items\n") % itemcount;
      } else if(cogscount < rentcount){
	cout << format("extra rent items: %10i\n") % (rentcount-cogscount);
      } else {
	cout << format("extra cogs items: %10i\n") % (cogscount-rentcount);
      }
    }

  }
}

// this compares the items stored in cogs with the items stored in rent,
// in an attempt to find out what items haven't been recorded properly
void check_cogs_detail(int cogs_detail){
  TDatabase db(DB_SNEEZY);
  map <sstring, int> items;

  db.query("select obj_name, count from shoplogcogs where count < 0 and shop_nr=%i", cogs_detail);

  cout << "Item entries in COGS with a count < 0:\n";
  while(db.fetchRow()){
    cout << format("%-50s : %3i\n") % db["obj_name"] % db["count"];
  }


  db.query("select obj_name, sum(count) as count from shoplogcogs where shop_nr=%i group by obj_name", cogs_detail);
  while(db.fetchRow()){
    items[db["obj_name"]]=convertTo<int>(db["count"]);
  }
  
  db.query("select o.short_desc as name, count(*) as count from obj o, rent r where owner_type='shop' and owner=%i and r.vnum=o.vnum group by o.vnum", cogs_detail);
  while(db.fetchRow()){
    if(items.find(db["name"]) != items.end()){
      items[db["name"]]-=convertTo<int>(db["count"]);
    }
  }

  cout << "\nFinal item counts that are != 0:\n";
  for(map <sstring,int>::iterator iter=items.begin();iter!=items.end();++iter){
    if((*iter).second != 0)
      cout << format("%-50s : %3i\n") % (*iter).first % (*iter).second;
  }

}


// this checks the basic balance sheet to see if it balances
// assets are cash and inventory value
// liabilities are deposits (for banks)
// equity is PIC (paid-in capital - talens that the owner put in the shop) and
//   retained earnings (profits)
void check_balance_sheet(){
  TDatabase db(DB_SNEEZY);

  cout << "Checking balance sheet totals..." << endl;

  for(int i=0;shops[i]!=-1;++i){
    db.query("select r.name as name from room r, shop s where s.shop_nr=%i and r.vnum=s.in_room", shops[i]);
    db.fetchRow();
    sstring shop_name=db["name"];

    TShopJournal tsj(shops[i]);
    
    int assets=tsj.getAssets();
    int liabilities=tsj.getLiabilities();
    int equity=tsj.getShareholdersEquity();
    
    if(assets != (liabilities+equity)){
      cout << format("%-30s (%3i): ") % shop_name % shops[i];

      if(assets < (liabilities+equity)){
	cout << format("missing assets: %10i\n") % 
	  ((liabilities+equity)-assets);
      } else {
	cout << format("extra assets:   %10i\n") % 
	  (assets - (liabilities+equity));
      }
    }
  }  
}


int main(int argc, char *argv[]){
  Config::doConfiguration();
  gamePort=Config::Port::PROD;

  bool cash_balance=false;
  bool inventory_cogs=false;
  bool balance_sheet=false;
  bool cogs_count=false;
  int cogs_detail=-1;
  bool all_checks=false;

  po::options_description cmdline("Options");
  cmdline.add_options()
    ("help", "produce help message")
    ("all_checks,a", po::value<bool>(&all_checks)->zero_tokens(),
     "run all checks")
    ("cash_balance,c", po::value<bool>(&cash_balance)->zero_tokens(),
     "check cash balances")
    ("inventory_cogs,i", po::value<bool>(&inventory_cogs)->zero_tokens(),
     "check inventory/COGS balances")
    ("balance_sheet,b", po::value<bool>(&balance_sheet)->zero_tokens(),
     "check balance sheet")
    ("cogs_count,g", po::value<bool>(&cogs_count)->zero_tokens(),
     "check inventory/COGS item count")
    ("cogs_detail", po::value<int>(&cogs_detail)->default_value(-1),
     "detailed inventory/COGS item comparison")
    ;
  po::options_description cmdline_options;
  cmdline_options.add(cmdline);
  po::variables_map vm;

  if(argc){
    po::store(po::command_line_parser(argc, argv).
	      options(cmdline_options).run(), vm);
  }
  po::notify(vm);

  if(cash_balance || all_checks)
    check_cash_balance();
  if(inventory_cogs || all_checks)
    check_inventory_cogs();
  if(balance_sheet || all_checks)
    check_balance_sheet();
  if(cogs_count || all_checks)
    check_cogs_count();
  if(cogs_detail!=-1)
    check_cogs_detail(cogs_detail);

  if(argc<=1 || vm.count("help"))  
     cout << cmdline_options;
}
