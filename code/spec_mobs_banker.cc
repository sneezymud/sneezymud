#include "stdsneezy.h"
#include "database.h"
#include "shop.h"
#include "shopowned.h"

int banker(TBeing *ch, cmdTypeT cmd, const char *arg, TMonster *myself, TObj *)
{
  int shop_nr, money, bankmoney=-1;
  TDatabase db(DB_SNEEZY);

  if((cmd!=CMD_WITHDRAW && cmd!=CMD_DEPOSIT && cmd!=CMD_BUY && cmd!=CMD_LIST &&
     cmd!=CMD_BALANCE && cmd!=CMD_WHISPER) || !ch || !myself)
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

    db.query("select count(*) as c, sum(talens) as t from shopowned bank");

    if(db.fetchRow()){
      myself->doTell(ch->getName(), fmt("%i accounts, %s talens.") %
		     convertTo<int>(db["c"]) % 
		     talenDisplay(convertTo<int>(db["t"])));
    }

    return TRUE;
  }


  db.query("select talens from shopownedbank where player_id=%i and shop_nr=%i", ch->getPlayerID(), shop_nr);
  
  if(!db.fetchRow()){
    if(cmd==CMD_BUY && sstring(arg).lower()=="account" && bankmoney==-1 &&
       ch->getMoney()>=100){
      db.query("insert into shopownedbank (player_id, shop_nr, talens) values (%i, %i, 0)", ch->getPlayerID(), shop_nr);
      ch->addToMoney(-100, GOLD_XFER);
      myself->addToMoney(100, GOLD_XFER);
      shoplog(shop_nr, ch, myself, "talens", 100, "new account");
      
      myself->doTell(ch->getName(), "Your account is now open and ready for use.");
      return TRUE;
    }

    myself->doTell(ch->getName(), "You don't have an account here.");
    myself->doTell(ch->getName(), "To open an account, type 'buy account'.");
    myself->doTell(ch->getName(), "The new account fee is 100 talens.");
    return TRUE;
  } else
    bankmoney=convertTo<int>(db["talens"]);

  if(cmd==CMD_BALANCE){
    myself->doTell(ch->getName(), fmt("Your balance is %i.") % bankmoney);
  } else if(cmd==CMD_WITHDRAW){
    money=convertTo<int>(arg);

    if (!ch->isPc() || dynamic_cast<TMonster *>(ch)) {
      myself->doTell(ch->getName(), "Stupid monster can't bank here!");
      return TRUE;
    }

    if (is_abbrev(arg, "all"))
      money = bankmoney;

    if (money > bankmoney) {
      myself->doTell(ch->getName(), "You don't have enough in the bank for that!");
      return TRUE;
    } else if(myself->getMoney() < money){
      myself->doTell(ch->getName(), "The bank doesn't have your funds available right now!");
      return TRUE;
    } else if (money <= 0) {
      myself->doTell(ch->getName(), "Go away, you bother me.");
      return TRUE;
    } else {
      myself->doTell(ch->getName(), "Thank you.");
      ch->addToMoney(money, GOLD_XFER);
      myself->addToMoney(-money, GOLD_XFER);
      
      db.query("update shopownedbank set talens=talens-%i where player_id=%i and shop_nr=%i", money, ch->getPlayerID(), shop_nr);

      shoplog(shop_nr, ch, myself, "talens", -money, "withdrawal");

      myself->doTell(ch->getName(), fmt("Your balance is %d.") % 
		     (bankmoney-money));
      return TRUE;
    }    
  } else if(cmd==CMD_DEPOSIT){
    money=convertTo<int>(arg);
    
    if (!ch->isPc() || dynamic_cast<TMonster *>(ch)) {
      myself->doTell(ch->getName(), "Stupid monster can't bank here!");
      return TRUE;
    }
    
    if (is_abbrev(arg, "all"))
      money = ch->getMoney();
    
    if (money <= 0) {
      myself->doTell(ch->getName(), "Go away, you bother me.");
      return TRUE;
    } else if (money > ch->getMoney()) {
      myself->doTell(ch->getName(), "You don't have enough for that!");
      return TRUE;
    }

    myself->doTell(ch->getName(), "Thank you.");
    ch->addToMoney(-money, GOLD_XFER);
    myself->addToMoney(money, GOLD_XFER);
    
    db.query("update shopownedbank set talens=talens+%i where player_id=%i and shop_nr=%i", money, ch->getPlayerID(), shop_nr);

    shoplog(shop_nr, ch, myself, "talens", money, "deposit");

    myself->doTell(ch->getName(), fmt("Your balance is %i.") %
		   (bankmoney+money));
  }
  
  return TRUE;
}

