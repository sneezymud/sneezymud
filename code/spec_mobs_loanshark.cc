#include "stdsneezy.h"
#include "database.h"
#include "shop.h"
#include "shopowned.h"

// 1 loan per account

// collection:
// transfer from bank account?
// transfer from shops/corp
// send out repo mobs

// setrates <default> <prime> <term> default
// profit_buy = defaulting rate, profit_sell=prime rate

// setrates <X> <Y> <unused> loanrate
// X * (lvl ** Y)


// granted = time_t value of real time that loan was granted
// term = term in mud months
// returns mud time that loan is due
time_info_data whenDue(time_t granted, int term)
{
  time_info_data due;

  mudTimePassed(granted, BEGINNING_OF_TIME, &due);

  due.year += YEAR_ADJUST;

  due.month+=term;
  
  while(due.month>12){
    due.month-=12;
    due.year++;
  }

  return due;
}

int calcInterest(int amt, time_t granted, int term, float rate, float def_charge)
{
  time_info_data due;

  mudTimePassed(time(NULL), granted, &due);
  
  int months=(due.year*12)+due.month+1;
  
  if(months > term) // overdue!
    amt += (int)((float) amt * def_charge);

  while(months--){
    amt += (int)((float) amt * rate);
  }

  return amt;
}


int loanShark(TBeing *ch, cmdTypeT cmd, const char *arg, TMonster *me, TObj *o)
{
  unsigned int shop_nr;
  TDatabase db(DB_SNEEZY);
  time_info_data due;

  if(cmd != CMD_WHISPER && cmd != CMD_LIST && cmd != CMD_BUY &&
     cmd != CMD_MOB_GIVEN_COINS)
   return false;

  for (shop_nr = 0; (shop_nr < shop_index.size()) && (shop_index[shop_nr].keeper != me->number); shop_nr++);

  if (shop_nr >= shop_index.size()) {
    vlogf(LOG_BUG, fmt("Warning... shop # for mobile %d (real nr) not found.") % me->number);
    return FALSE;
  }
    
  if(cmd==CMD_WHISPER)
    return shopWhisper(ch, me, shop_nr, arg);

  TShopOwned tso(shop_nr, me, ch);

  db.query("select x, y, term from shopownedloanrate where shop_nr=%i",
	   shop_nr);
  if(!db.fetchRow()){
    vlogf(LOG_DB, fmt("couldn't find loanrate table for shop %i") % shop_nr);
    me->doSay("Hm, I can't seem to find my paperwork!");
    return false;
  }

  double X=convertTo<double>(db["x"]);
  double Y=convertTo<double>(db["y"]);
  int term=convertTo<int>(db["term"]);
  int amt=(int)(pow(ch->GetMaxLevel(), X) / pow(50, X) * Y);
  
  ////////////////////////////
  if(cmd==CMD_LIST){
    ///////////////////////////
    if(sstring(arg)=="loans" && tso.hasAccess(SHOPACCESS_INFO)){
      db.query("select name, amt, granted_time, term, rate, default_charge from player p, shopownedloans l where p.id=l.player_id order by granted_time");
      
      
      while(db.fetchRow()){
	due=whenDue(convertTo<int>(db["granted_time"]), convertTo<int>(db["term"]));
;
	me->doTell(ch->getName(),
		   fmt("%s for %i talens at %.2f%c, due %s of %s, %d P.S") %
		   db["name"] %
		   convertTo<int>(db["amt"]) %
		   (convertTo<float>(db["rate"]) * 100) % '%' %
		   numberAsString(due.day) %
		   month_name[due.month] %
		   due.year);
      }
      return true;
    }
    
    /////////////////////////////
    db.query("select amt, granted_time, term, rate, default_charge from shopownedloans where player_id=%i", ch->getPlayerID());

    if(db.fetchRow()){
      amt=convertTo<int>(db["amt"]);
      due=whenDue(convertTo<int>(db["granted_time"]), convertTo<int>(db["term"]));

      me->doSay(fmt("You have a loan for %i talens, due on the %s day of %s, Year %d P.S.") %
		amt %
		numberAsString(due.day) % 
		month_name[due.month] %
		due.year);

      amt=calcInterest(amt, convertTo<int>(db["granted_time"]),
		  convertTo<int>(db["term"]), convertTo<float>(db["rate"]),
		  convertTo<float>(db["default_charge"]));
	
      
      me->doSay(fmt("With interest, you owe %i talens.") % amt);

    } else {
      me->doSay(fmt("I can extend you a loan for %i talens.") % amt);
      me->doSay(fmt("A monthly cumulative interest rate of %.2f%c will apply.") % 
		(shop_index[shop_nr].profit_sell * 100) % '%');
      me->doSay(fmt("The term length I can offer is %i months.") % term);
      me->doSay(fmt("If you default on the loan, you will be charged an additional %.2f%c.") %
		(shop_index[shop_nr].profit_buy * 100) % '%');
      me->doSay("Do \"buy loan\" to take out the loan.");
    }
    return true;
  }



  ////////////////////////////
  if(cmd==CMD_BUY){
    if(sstring(arg) != "loan"){
      me->doSay("If you want to take out the loan, do \"buy loan\".");
      return true;
    }

    if(amt > me->getMoney()){
      me->doSay("At the moment, I don't have the necessary capital to extend a loan to you.");
      return true;
    }

    db.query("insert into shopownedloans values (%i, %i, %i, %i, %i, %f, %f)",
    	     shop_nr, ch->getPlayerID(), amt, time(NULL),
	     term, shop_index[shop_nr].profit_sell, 
	     shop_index[shop_nr].profit_buy);


    me->addToMoney(-amt, GOLD_SHOP);
    ch->addToMoney(amt, GOLD_SHOP);

    me->doSay(fmt("There you go.  Remember, I need the money back, plus interest, within %i months.") % term);
  }


  /////////////////
  if(cmd==CMD_MOB_GIVEN_COINS){
    int coins=(int) o;

    db.query("select amt, granted_time, term, rate, default_charge from shopownedloans where player_id=%i", ch->getPlayerID());
    
    if(db.fetchRow()){
      int principle=convertTo<int>(db["amt"]);
      amt=calcInterest(principle, convertTo<int>(db["granted_time"]),
		       convertTo<int>(db["term"]), 
		       convertTo<float>(db["rate"]),
		       convertTo<float>(db["default_charge"]));

      if(coins>=amt){
	me->doSay("Alright, everything appears to be in order here.  Consider your loan paid off!");
	db.query("delete from shopownedloans where player_id=%i", ch->getPlayerID());
      } else {
	// how much of the amount owed is the principle
	float perc=(float)principle / (float)amt;
	principle -= (int)(perc*coins);
	
	db.query("update shopownedloans set amt=%i where player_id=%i",
		 principle, ch->getPlayerID());

	me->doSay(fmt("Thanks for the payment.  You still owe %i on the principle.") % principle);
      }
    } else {
      me->doSay("Uhh... thanks!");
    }
  }

  return false;
}
