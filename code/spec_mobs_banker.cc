#include "stdsneezy.h"
#include "database.h"
#include "shop.h"
#include "shopowned.h"


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
  ch->addToMoney(money, GOLD_XFER);
  myself->addToMoney(-money, GOLD_XFER);
  myself->saveItems(fmt("%s/%d") % SHOPFILE_PATH % shop_nr);
  
  
  db.query("update shopownedbank set talens=talens-%i where player_id=%i and shop_nr=%i", money, ch->getPlayerID(), shop_nr);
  
  shoplog(shop_nr, ch, myself, "talens", -money, "withdrawal");

  return TRUE;
}

int bankDeposit(TBeing *ch, TMonster *myself, TMonster *teller, int shop_nr, int money)
{
  TDatabase db(DB_SNEEZY);

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
  ch->addToMoney(-money, GOLD_XFER);
  myself->addToMoney(money, GOLD_XFER);
  myself->saveItems(fmt("%s/%d") % SHOPFILE_PATH % shop_nr);
  
  db.query("update shopownedbank set talens=talens+%i where player_id=%i and shop_nr=%i", money, ch->getPlayerID(), shop_nr);
  
  shoplog(shop_nr, ch, myself, "talens", money, "deposit");
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
  ch->addToMoney(-100, GOLD_XFER);
  myself->addToMoney(100, GOLD_XFER);
  shoplog(shop_nr, ch, myself, "talens", 100, "new account");
  myself->saveItems(fmt("%s/%d") % SHOPFILE_PATH % shop_nr);
  
  teller->doTell(ch->getName(), "Your account is now open and ready for use.");
  
  return TRUE;
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
    
    db.query("select p.name, b.talens from player p, shopownedbank b where b.shop_nr=%i and b.player_id=p.id", shop_nr);
    
    while(db.fetchRow()){
      myself->doTell(ch->getName(), fmt("%s - %s talens.") % db["name"] %
		     talenDisplay(convertTo<int>(db["talens"])));
    }

    db.query("select count(*) as c, sum(talens) as t from shopownedbank where shop_nr=%i", shop_nr);

    if(db.fetchRow()){
      myself->doTell(ch->getName(), fmt("%i accounts, %s talens.") %
		     convertTo<int>(db["c"]) % 
		     talenDisplay(convertTo<int>(db["t"])));
    }

    return TRUE;
  }


  if(cmd==CMD_BUY && sstring(arg).lower()=="account"){
    money=convertTo<int>(arg);    
    return bankBuyAccount(ch, myself, myself, shop_nr, money);
  } else if(cmd==CMD_BALANCE){
    return bankBalance(ch, myself, myself, shop_nr);
  } else if(cmd==CMD_WITHDRAW){
    money=convertTo<int>(arg);
    return bankWithdraw(ch, myself, myself, shop_nr, money);
  } else if(cmd==CMD_DEPOSIT){
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
    money=convertTo<int>(arg);
    return bankWithdraw(ch, banker, teller, shop_nr, money);
  } else if(cmd==CMD_DEPOSIT){
    money=convertTo<int>(arg);
    return bankDeposit(ch, banker, teller, shop_nr, money);
  }

  return FALSE;
}


