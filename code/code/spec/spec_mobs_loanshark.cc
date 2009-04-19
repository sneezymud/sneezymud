#include <cmath>

#include "monster.h"
#include "database.h"
#include "extern.h"
#include "shop.h"
#include "shopowned.h"
#include "spec_mobs.h"
#include "weather.h"

// 1 loan per account

// collection:
// transfer from bank account?
// transfer from shops/corp
// send out repo mobs

// setrates <default> <prime> <term> default
// profit_buy = defaulting rate, profit_sell=prime rate

// setrates <X> <Y> <term> loanrate
// X * (lvl ** Y)

double getPenalty(unsigned int shop_nr, const sstring &name)
{
  TDatabase db(DB_SNEEZY);

  db.query("select profit_buy from shopownedplayer where player='%s' and shop_nr=%i", name.c_str(), shop_nr);
  
  if(db.fetchRow())
    return convertTo<double>(db["profit_buy"]);
  else {
    db.query("select profit_buy from shopowned where shop_nr=%i", shop_nr);

    if(db.fetchRow())
      return convertTo<double>(db["profit_buy"]);
  }

  return shop_index[shop_nr].profit_buy;
}

double getRate(unsigned int shop_nr, const sstring &name)
{
  TDatabase db(DB_SNEEZY);

  db.query("select profit_sell from shopownedplayer where player='%s' and shop_nr=%i", name.c_str(), shop_nr);
  
  if(db.fetchRow())
    return convertTo<double>(db["profit_sell"]);
  else {
    db.query("select profit_sell from shopowned where shop_nr=%i", shop_nr);

    if(db.fetchRow())
      return convertTo<double>(db["profit_sell"]);
  }

  return shop_index[shop_nr].profit_sell;
}

int getTerm(unsigned int shop_nr, const sstring &name)
{
  TDatabase db(DB_SNEEZY);

  db.query("select max_num from shopownedplayer where player='%s' and shop_nr=%i", name.c_str(), shop_nr);
  
  if(db.fetchRow())
    return convertTo<int>(db["max_num"]);
  else {
    db.query("select term from shopownedloanrate where shop_nr=%i", shop_nr);
    
    if(db.fetchRow())
      return convertTo<int>(db["term"]);
  }

  return 12;
}



// granted = time_t value of real time that loan was granted
// term = term in mud years
// returns mud time that loan is due
time_info_data whenDue(time_t granted, int term)
{
  time_info_data due;

  GameTime::mudTimePassed(granted, GameTime::getBeginningOfTime(), &due);

  due.year += GameTime::getYearAdjust();
  due.year += term;
  
  return due;
}

int calcInterest(int amt, time_t granted, int term, float rate, float def_charge)
{
  time_info_data due;

  GameTime::mudTimePassed(time(NULL), granted, &due);
  due.year++;

  if(due.year > term) // overdue!
    amt += (int)((float) amt * def_charge);

  while(due.year--){
    amt += (int)((float) amt * rate);
  }

  return amt;
}


int loanShark(TBeing *ch, cmdTypeT cmd, const char *arg, TMonster *me, TObj *o)
{
  unsigned int shop_nr;
  TDatabase db(DB_SNEEZY);
  time_info_data due;
  sstring buf;

  if(cmd != CMD_WHISPER && cmd != CMD_LIST && cmd != CMD_BUY &&
     cmd != CMD_MOB_GIVEN_COINS)
    return false;
  
  if(!(shop_nr=find_shop_nr(me->number)))
    return false;
    
  if(cmd==CMD_WHISPER)
    return shopWhisper(ch, me, shop_nr, arg);

  TShopOwned tso(shop_nr, me, ch);


  db.query("select x, y from shopownedloanrate where shop_nr=%i",
	   shop_nr);
  if(!db.fetchRow()){
    vlogf(LOG_DB, format("couldn't find loanrate table for shop %i") % shop_nr);
    me->doTell(ch->getName(), "Hm, I can't seem to find my paperwork!");
    return false;
  }

  double X=convertTo<double>(db["x"]);
  double Y=convertTo<double>(db["y"]);
  int term=getTerm(shop_nr, ch->getName());
  int amt=(int)(pow(ch->GetMaxLevel(), X) / pow(50, X) * Y);
  
  ////////////////////////////
  if(cmd==CMD_LIST){
    ///////////////////////////
    if(sstring(arg)=="loans" && tso.hasAccess(SHOPACCESS_INFO)){
      db.query("select name, amt, granted_time, term, rate, default_charge from player p, shopownedloans l where p.id=l.player_id order by granted_time");
      
      
      while(db.fetchRow()){
	due=whenDue(convertTo<int>(db["granted_time"]), convertTo<int>(db["term"]));
	
	amt=calcInterest(convertTo<int>(db["amt"]), 
			 convertTo<int>(db["granted_time"]),
			 convertTo<int>(db["term"]), 
			 convertTo<float>(db["rate"]),
			 convertTo<float>(db["default_charge"]));

	me->doTell(ch->getName(), 
		   format("%s for %i talens at %.2f%c due %s of %s, %d P.S") %
		   db["name"] % convertTo<int>(db["amt"]) % 
		   (convertTo<float>(db["rate"]) * 100) % '%' %
		   numberAsString(due.day) % month_name[due.month] %
		   due.year);
	me->doTell(ch->getName(),
		   format("%s currently owes %i talens.") %
		   db["name"] % amt);
      }
    } else if(sstring(arg)=="repo" && tso.hasAccess(SHOPACCESS_INFO)){
      me->doTell(ch->getName(), 
		 "I have the following options for collecting on loans.");
      me->doTell(ch->getName(), format("1.[1%c]) I can do a direct transfer from the debtors bank account.") % '%');
      me->doTell(ch->getName(), format("2.[1%c]) I can do a direct transfer from the debtors corporate account.") % '%');
      me->doTell(ch->getName(), format("3.[3%c]) I can do a direct transfer from the debtors shops.") % '%');
      me->doTell(ch->getName(), format("4.[5%c]) I can repossess items from the debtors shops and give them to you.") % '%');
      me->doTell(ch->getName(), format("5.[7%c]) I can repossess entire shops if need be.") % '%');
      me->doTell(ch->getName(), format("6.[9%c]) I can send out a collection agent to collect the debt by force.") % '%');
      me->doTell(ch->getName(), "The cost of the collection method is a percent of the debt, as listed.");
      me->doTell(ch->getName(), "You will be charged even if the collection isn't successful.");
      me->doTell(ch->getName(), "Shops won't be repossessed unless the debt exceeds the value of the shop.");
      me->doTell(ch->getName(), "To use one of these collection methods, do 'buy repo <number> <playername>");
    } else {
      db.query("select amt, granted_time, term, rate, default_charge from shopownedloans where player_id=%i", ch->getPlayerID());
      
      if(db.fetchRow()){
	amt=convertTo<int>(db["amt"]);
	due=whenDue(convertTo<int>(db["granted_time"]), convertTo<int>(db["term"]));
	
	me->doTell(ch->getName(), format("You have a loan for %i talens, due on the %s day of %s, Year %d P.S.") %
		   amt %
		   numberAsString(due.day) % 
		   month_name[due.month] %
		   due.year);
	
	amt=calcInterest(amt, convertTo<int>(db["granted_time"]),
			 convertTo<int>(db["term"]), convertTo<float>(db["rate"]),
			 convertTo<float>(db["default_charge"]));
	
	
	me->doTell(ch->getName(), format("With interest, you owe %i talens.") % amt);
      } else {
	me->doTell(ch->getName(), format("I can extend you a loan for %i talens.") % amt);
	me->doTell(ch->getName(), format("A yearly cumulative interest rate of %.2f%c will apply.") % 
		   (getRate(shop_nr, ch->getName()) * 100) % '%');
	me->doTell(ch->getName(), format("The term length I can offer is %i years.") % term);
	me->doTell(ch->getName(), "One mud year is about 2 weeks in real time.");
	me->doTell(ch->getName(), format("If you default on the loan, you will be charged an additional %.2f%c.") %
		   (getPenalty(shop_nr, ch->getName()) * 100) % '%');
	me->doTell(ch->getName(), "Do \"buy loan <amt>\" to take out the loan.");
      }
    }
    return true;
  }



  ////////////////////////////
  if(cmd==CMD_BUY){
    if(sstring(arg).word(0) == "repo" && tso.hasAccess(SHOPACCESS_OWNER)){
      db.query("select amt, granted_time, term, rate, default_charge from shopownedloans, player where player_id=id and lower(name)=lower('%s')", sstring(arg).word(2).c_str());
      
      if(!db.fetchRow()){
	      me->doTell(ch->getName(), "I can't find a loan for that player.");
	      return true;
      }

      // repo stuff here



    } else if(sstring(arg).word(0) == "loan"){    
      db.query("select 1 from shopownedloans where player_id=%i", 
	       ch->getPlayerID());
      if(db.fetchRow()){
	      me->doTell(ch->getName(), "You already have a loan!");
	      return true;
      }

      int loanamt=convertTo<int>(sstring(arg).word(1));
    
      if(loanamt > amt || loanamt <= 0){
	      me->doTell(ch->getName(), format("You can't take out a loan for that much.  The most I can give you is %i.") % amt);
	      return true;
      }

      amt=loanamt;

      if(amt > me->getMoney()){
	      me->doTell(ch->getName(), "At the moment, I don't have the necessary capital to extend a loan to you.");
	      return true;
      }

      db.query("insert into shopownedloans values (%i, %i, %i, %i, %i, %f, %f)",
	       shop_nr, ch->getPlayerID(), amt, time(NULL),
	       term, getRate(shop_nr, ch->getName()), 
	       getPenalty(shop_nr, ch->getName()));


      me->giveMoney(ch, amt, GOLD_SHOP);
      me->saveItems(shop_nr);


      me->doTell(ch->getName(), format("There you go.  Remember, I need the money back, plus interest, within %i years.") % term);
      me->doTell(ch->getName(), "Do 'list' again at anytime to see how much you owe with interest included.");
      me->doTell(ch->getName(), "You can give me talens at any time to make a payment on your loan.");
    
      shoplog(shop_nr, ch, me, "talens", -amt, "loaning");
    } else {
      me->doTell(ch->getName(), "If you want to take out the loan, do \"buy loan <amount>\".");
    }
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
	      me->doTell(ch->getName(), "Alright, everything appears to be in order here.  Consider your loan paid off!");
	      db.query("delete from shopownedloans where player_id=%i", ch->getPlayerID());
	      shoplog(shop_nr, ch, me, "talens", coins, "giving");
      } else {
	// how much of the amount owed is the principle
	      float perc=(float)principle / (float)amt;
	      principle -= (int)(perc*coins);

	      db.query("update shopownedloans set amt=%i where player_id=%i",
		       principle, ch->getPlayerID());

	      me->doTell(ch->getName(), format("Thanks for the payment.  You paid down the principle by %i talens, the rest went to interest.") % (int)(perc*coins));

	      shoplog(shop_nr, ch, me, "talens", coins, "giving");
      }
    } else {
      me->doTell(ch->getName(), "Uhh... thanks!");
    }
  }

  me->saveItems(shop_nr);

  return false;
}
