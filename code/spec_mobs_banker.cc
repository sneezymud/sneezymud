#include "stdsneezy.h"
#include "database.h"
#include "shop.h"
#include "shopowned.h"
#include "process.h"


// procBankInterest
procBankInterest::procBankInterest(const int &p)
{
  trigger_pulse=p;
  name="procBankInterest";
}

void procBankInterest::run(int pulse) const 
{
  TDatabase db(DB_SNEEZY), in(DB_SNEEZY), out(DB_SNEEZY);
  double profit_sell;
  unsigned int shop_nr;
  map <int, int> player_gain;
  map <int, int> corp_gain;

  db.query("update shopownedbank set earned_interest=0 where earned_interest is null");
  db.query("update shopownedcorpbank set earned_interest=0 where earned_interest is null");
  db.query("update shopownedbank set talens=0 where talens is null");
  db.query("update shopownedcorpbank set talens=0 where talens is null");

  db.query("select s1.shop_nr, s1.keeper from shop s1 join mob m1 on s1.keeper = m1.vnum where spec_proc = %i", SPEC_BANKER);
  while(db.fetchRow()){
    shop_nr=convertTo<int>(db["shop_nr"]);
    profit_sell=shop_index[shop_nr].profit_sell;

    if(profit_sell==1.0)
      continue;

    // make a list of current player talens
    in.query("select player_id, talens from shopownedbank where shop_nr=%i", shop_nr);
    while(in.fetchRow())
      player_gain[convertTo<int>(in["player_id"])]=convertTo<int>(in["talens"]);

    // make a list of current corporate talens
    in.query("select corp_id, talens from shopownedcorpbank where shop_nr=%i", shop_nr);
    while(in.fetchRow())
      corp_gain[convertTo<int>(in["corp_id"])]=convertTo<int>(in["talens"]);

    // calculate interest
    in.query("update shopownedbank set earned_interest=earned_interest + (talens * (%f / 365.0)) where shop_nr=%i", profit_sell, shop_nr);

    // doll out earned interest that isn't fractional
    in.query("update shopownedbank set talens=talens + truncate(earned_interest,0), earned_interest=earned_interest - truncate(earned_interest,0) where shop_nr=%i", shop_nr);

    // calculate interest
    in.query("update shopownedcorpbank set earned_interest=earned_interest + (talens * (%f / 365.0)) where shop_nr=%i", profit_sell, shop_nr);

    // doll out earned interest that isn't fractional
    in.query("update shopownedcorpbank set talens=talens + truncate(earned_interest,0), earned_interest=earned_interest - truncate(earned_interest,0) where shop_nr=%i", shop_nr);

    TShopOwned tso(shop_nr, NULL, NULL);

    // log player gains
    in.query("select p.name as name, sob.player_id as player_id, sob.talens as talens from shopownedbank sob, player p where shop_nr=%i and sob.player_id=p.id", shop_nr);
    while(in.fetchRow()){
      if((convertTo<int>(in["talens"]) - player_gain[convertTo<int>(in["player_id"])]) != 0){
        int amt=convertTo<int>(in["talens"]) - player_gain[convertTo<int>(in["player_id"])];

        tso.journalize(in["name"], "talens", TX_PAYING_INTEREST, amt, 0, 0, 0);

        out.query("insert into shoplog values (%i, '%s', 'interest', 'talens', %i, %i, 0, now(), 0)", 
            shop_nr, 
            in["name"].c_str(),
            amt,
            convertTo<int>(in["talens"]));

      }
    }

    // log corporate gains
    in.query("select c.name, sob.corp_id, sob.talens from shopownedcorpbank sob, corporation c where c.corp_id=sob.corp_id and sob.shop_nr=%i",
       shop_nr);
    while(in.fetchRow()){
      if((convertTo<int>(in["talens"]) - corp_gain[convertTo<int>(in["corp_id"])]) != 0){
        int amt=convertTo<int>(in["talens"]) - corp_gain[convertTo<int>(in["corp_id"])];

        tso.journalize(in["name"], "talens", TX_PAYING_INTEREST, amt, 0, 0, 0);

        out.query("insert into shoplog values (%i, '%s', 'interest', 'talens', %i, %i, 0, now(), 0)", 
            shop_nr, 
            in["name"].c_str(),
            amt,
            convertTo<int>(in["talens"]));
      }	
    }
  }
}


int bankWithdraw(TBeing *ch, TMonster *myself, TMonster *teller, int shop_nr, int money)
{
  TDatabase db(DB_SNEEZY);
  int bankmoney;

  if (!ch->isPc() || dynamic_cast<TMonster *>(ch)) {
    teller->doTell(ch->getName(), "Stupid monster can't bank here!");
    return TRUE;
  }

  db.query("select talens from shopownedbank where shop_nr=%i and player_id=%i", shop_nr, ch->getPlayerID());

  if(!db.fetchRow()){
    teller->doTell(ch->getName(), "You don't have an account here.");
    teller->doTell(ch->getName(), "To open an account, type 'buy account'.");
    teller->doTell(ch->getName(), "The new account fee is 100 talens.");
    return FALSE;
  } else
    bankmoney = convertTo<int>(db["talens"]);
  
  if (money > bankmoney) {
    teller->doTell(ch->getName(), "You don't have enough in the bank for that!");
    return TRUE;
  } else if(myself->getMoney() < money){
    teller->doTell(ch->getName(), "The bank doesn't have your funds available right now!");
    return TRUE;
  } else if (money <= 0) {
    teller->doTell(ch->getName(), "Go away, you bother me.");
    return TRUE;
  }

  teller->doTell(ch->getName(), "Thank you.");

  TShopOwned tso(shop_nr, teller, ch);
  tso.doSellTransaction(money, "talens", TX_WITHDRAWAL);

  db.query("update shopownedbank set talens=talens-%i where player_id=%i and shop_nr=%i", money, ch->getPlayerID(), shop_nr);
  

  return TRUE;
}

int bankDeposit(TBeing *ch, TMonster *myself, TMonster *teller, int shop_nr, int money)
{
  TDatabase db(DB_SNEEZY);
  int bankmoney;

  if (!ch->isPc() || dynamic_cast<TMonster *>(ch)) {
    teller->doTell(ch->getName(), "Stupid monster can't bank here!");
    return TRUE;
  }

  db.query("select talens from shopownedbank where shop_nr=%i and player_id=%i", shop_nr, ch->getPlayerID());

  if(!db.fetchRow()){
    teller->doTell(ch->getName(), "You don't have an account here.");
    teller->doTell(ch->getName(), "To open an account, type 'buy account'.");
    teller->doTell(ch->getName(), "The new account fee is 100 talens.");
    return TRUE;
  }
  
  if (money <= 0) {
    teller->doTell(ch->getName(), "Go away, you bother me.");
    return TRUE;
  } else if (money > ch->getMoney()) {
    teller->doTell(ch->getName(), "You don't have enough for that!");
    return TRUE;
  }
  
  teller->doTell(ch->getName(), "Thank you.");

  TShopOwned tso(shop_nr, teller, ch);
  tso.doBuyTransaction(money, "talens", TX_DEPOSIT);
  
  db.query("update shopownedbank set talens=talens+%i where player_id=%i and shop_nr=%i", money, ch->getPlayerID(), shop_nr);
  

  db.query("select talens from shopownedbank where shop_nr=%i and player_id=%i", shop_nr, ch->getPlayerID());

  if (!db.fetchRow()) {
    teller->doTell(ch->getName(), "You really should not see me, this is an error...");
    vlogf(LOG_BUG, fmt("Banking error, was unable to retrieve balance after a deposit: %s") % ch->getName());

    return TRUE;
  } else
    bankmoney = convertTo<int>(db["talens"]);

  teller->doTell(ch->getName(), fmt("...Your new balance is %i") % bankmoney);

  return TRUE;
}

int bankBalance(TBeing *ch, TMonster *myself, TMonster *teller, int shop_nr)
{
  TDatabase db(DB_SNEEZY);
  int bankmoney;

  db.query("select talens from shopownedbank where shop_nr=%i and player_id=%i", shop_nr, ch->getPlayerID());


  if(!db.fetchRow()){
    teller->doTell(ch->getName(), "You don't have an account here.");
    teller->doTell(ch->getName(), "To open an account, type 'buy account'.");
    teller->doTell(ch->getName(), "The new account fee is 100 talens.");
    return TRUE;
  } else
    bankmoney = convertTo<int>(db["talens"]);

  teller->doTell(ch->getName(), fmt("Your balance is %i.") % bankmoney);

  return TRUE;
}

int bankBuyAccount(TBeing *ch, TMonster *myself, TMonster *teller, int shop_nr, int money)
{
  TDatabase db(DB_SNEEZY);
  
  if(ch->getMoney() < 100){
    teller->doTell(ch->getName(), "You don't have enough money to open an account.");
    return TRUE;
  }
  
  db.query("select talens from shopownedbank where player_id=%i and shop_nr=%i", ch->getPlayerID(), shop_nr);
  
  if(db.fetchRow()){
    teller->doTell(ch->getName(), "You already have an account.");
    return TRUE;
  }
  
  db.query("insert into shopownedbank (player_id, shop_nr, talens) values (%i, %i, 0)", ch->getPlayerID(), shop_nr);
  ch->giveMoney(myself, 100, GOLD_XFER);
  shoplog(shop_nr, ch, myself, "talens", 100, "new account");
  myself->saveItems(fmt("%s/%d") % SHOPFILE_PATH % shop_nr);
  
  teller->doTell(ch->getName(), "Your account is now open and ready for use.");
  
  return TRUE;
}

int centralBanker(TBeing *ch, cmdTypeT cmd, const char *arg, TMonster *myself, TObj *)
{
  int shop_nr;
  TDatabase db(DB_SNEEZY);
  TDatabase db2(DB_SNEEZY);
  sstring buf;

  if((cmd != CMD_WHISPER &&
      cmd != CMD_LIST) ||
     !ch || !myself)
    return FALSE;

  if(!(shop_nr=find_shop_nr(myself->number)))
    return FALSE;
  
  if(cmd==CMD_WHISPER)
    return shopWhisper(ch, myself, shop_nr, arg);

  if(cmd==CMD_LIST){
    db.query("select s.shop_nr, r.name from room r, shop s, shopownedcentralbank socb where socb.centralbank=%i and s.shop_nr=socb.bank and r.vnum=s.in_room", shop_nr);
    
    while(db.fetchRow()){
      db2.query("select (sb.c+sbc.c) as c, (sb.t+sbc.t) as t from (select count(*) as c, sum(talens) as t from shopownedbank where shop_nr=%i) sb, (select count(*) as c, sum(talens) as t from shopownedcorpbank where shop_nr=%i) sbc", db["shop_nr"].c_str(), db["shop_nr"].c_str());

      if(db2.fetchRow()){
	buf += fmt("<c>%s - %s accounts, %s in deposits.<1>\n\r") % 
	  db["name"] % db2["c"] %
	  talenDisplay(convertTo<int>(db2["t"]));
      }
    }
    ch->desc->page_string(buf);
    return TRUE;
  }

  return FALSE;
}


int banker(TBeing *ch, cmdTypeT cmd, const char *arg, TMonster *myself, TObj *)
{
  int shop_nr, money=0;
  TDatabase db(DB_SNEEZY);

  if((cmd!=CMD_WITHDRAW && 
      cmd!=CMD_DEPOSIT && 
      cmd!=CMD_BUY && 
      cmd!=CMD_LIST &&
      cmd!=CMD_BALANCE && 
      cmd!=CMD_WHISPER) ||
     !ch || !myself)
    return FALSE;

  if(!(shop_nr=find_shop_nr(myself->number)))
    return FALSE;
  
  if(cmd==CMD_WHISPER)
    return shopWhisper(ch, myself, shop_nr, arg);

  if(cmd==CMD_LIST){
    TShopOwned tso(shop_nr, myself, ch);

    if(!tso.hasAccess(SHOPACCESS_LOGS)){
      myself->doTell(ch->getName(), "Sorry, you don't have access to do that.");
      return FALSE;
    }

    db.query("select p.name, b.talens from player p, shopownedbank b where b.shop_nr=%i and b.player_id=p.id union select c.name, b.talens from corporation c, shopownedcorpbank b where b.shop_nr=c.bank and b.shop_nr=%i and b.corp_id=c.corp_id order by talens desc", shop_nr, shop_nr);

    sstring buf;
    int empty=0;
    while(db.fetchRow()){
      if(convertTo<int>(db["talens"]) == 0)
	empty++;
      else
	buf += fmt("<c>%s - %s talens.<1>\n\r") % db["name"] %
	  talenDisplay(convertTo<int>(db["talens"]));
    }
    buf += fmt("%i empty accounts.\n\r") % empty;

    db.query("select (sb.c+sbc.c) as c, (sb.t+sbc.t) as t from (select count(*) as c, sum(talens) as t from shopownedbank where shop_nr=%i) sb, (select count(*) as c, sum(talens) as t from shopownedcorpbank where shop_nr=%i) sbc", shop_nr, shop_nr);


    if(db.fetchRow()){
      buf += fmt("%i total accounts, %s talens.\n\r") %
	convertTo<int>(db["c"]) % 
	talenDisplay(convertTo<int>(db["t"]));
    }

    ch->desc->page_string(buf);

    return TRUE;
  }


  if(cmd==CMD_BUY && sstring(arg).lower()=="account"){
    money=convertTo<int>(arg);    
    return bankBuyAccount(ch, myself, myself, shop_nr, money);
  } else if(cmd==CMD_BALANCE){
    return bankBalance(ch, myself, myself, shop_nr);
  } else if(cmd==CMD_WITHDRAW){
    if(!strcmp(arg, "all"))
      money=ch->getBank();
    else
      money=convertTo<int>(arg);

    return bankWithdraw(ch, myself, myself, shop_nr, money);
  } else if(cmd==CMD_DEPOSIT){
    if(!strcmp(arg, "all"))
      money=ch->getMoney();
    else
      money=convertTo<int>(arg);
    return bankDeposit(ch, myself, myself, shop_nr, money);
  }
  
  return TRUE;
}



int bankRoom(TBeing *ch, cmdTypeT cmd, const char *arg, TRoom *rp)
{
  TMonster *teller, *banker;
  TBeing *t;
  int tmp=130, money, shop_nr;

  // tellers/branches can only do these commands
  if(cmd!=CMD_WITHDRAW &&
     cmd!=CMD_DEPOSIT &&
     cmd!=CMD_BALANCE &&
     cmd!=CMD_BUY)
    return FALSE;

  // find out which teller for this room
  switch(rp->number){
    case 31751: 
    case 31756:
    case 2351:
      tmp=31750;
      break;
    case 1295:
      tmp=1295;
      break;
    case 3755:
      tmp=3755;
      break;
    case 8756:
      tmp=8756;
      break;
  }

  // find the teller
  for(t=character_list;t;t=t->next){
    if(t->mobVnum()==tmp)
      break;
  }

  if(!t || !(teller=dynamic_cast<TMonster *>(t)))
    return FALSE;

  // find out which banker for this room
  switch(rp->number){
    case 31751: 
    case 31756:
    case 2351:
      tmp=31765;
      break;
    case 1295:
      tmp=1295;
      break;
    case 3755:
      tmp=3755;
      break;
    case 8756:
      tmp=8756;
      break;
  }

  // find the banker
  for(t=character_list;t;t=t->next){
    if(t->mobVnum()==tmp)
      break;
  }

  if(!t || !(banker=dynamic_cast<TMonster *>(t)))
    return FALSE;

  if(!(shop_nr=find_shop_nr(banker->number)))
    return FALSE;


  if(cmd==CMD_BUY && sstring(arg).lower()=="account"){
    money=convertTo<int>(arg);    
    return bankBuyAccount(ch, banker, teller, shop_nr, money);
  } else if(cmd==CMD_BALANCE){
    return bankBalance(ch, banker, teller, shop_nr);
  } else if(cmd==CMD_WITHDRAW){
    if(!strcmp(arg, "all"))
      money=ch->getBank();
    else
      money=convertTo<int>(arg);
    return bankWithdraw(ch, banker, teller, shop_nr, money);
  } else if(cmd==CMD_DEPOSIT){
    if(!strcmp(arg, "all"))
      money=ch->getMoney();
    else
      money=convertTo<int>(arg);
    return bankDeposit(ch, banker, teller, shop_nr, money);
  }

  return FALSE;
}


