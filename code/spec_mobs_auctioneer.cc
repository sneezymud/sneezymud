#include "stdsneezy.h"
#include "database.h"
#include "shop.h"
#include "shopowned.h"
#include "rent.h"

// shop_nr
// ticket
// version
// bid
// bidder
// buyout



void auctionList(TBeing *ch, TMonster *myself)
{
  TDatabase db(DB_SNEEZY);
  sstring filename;
  int ticket;
  ItemLoad il;
  TObj *obj;
  int shop_nr=find_shop_nr(myself->number);

  myself->doTell(ch->getName(), "This is what I have up for auction:");

  db.query("select ticket, bid, days from shopownedauction where shop_nr=%i", shop_nr);

  while(db.fetchRow()){
    ticket=convertTo<int>(db["ticket"]);

    il.openFile(fmt("mobdata/auction/%d/%d") % myself->mobVnum() % ticket);
    il.readVersion();
    obj=il.raw_read_item();
    ch->sendTo(COLOR_BASIC, fmt("%i) %s - %i days - %i talens\n\r") % ticket %
	       obj->getName() % convertTo<int>(db["days"]) %
	       convertTo<int>(db["bid"]));
    delete obj;
  }

  return;
}


void auctionSell(TBeing *ch, TMonster *myself, sstring arg)
{
  sstring name=arg.word(0);
  int min_bid=convertTo<int>(arg.word(1));
  int days=convertTo<int>(arg.word(2));
  int buyout=convertTo<int>(arg.word(3));
  TObj *obj;
  ItemSave is;
  TDatabase db(DB_SNEEZY);
  int ticket=1;
  int shop_nr=find_shop_nr(myself->number);

  if(min_bid <= 0 || days <= 0 || (buyout && buyout < min_bid)){
    myself->doTell(ch->getName(), "You have to specify a minimum bid and optionally a buyout price.");
    myself->doTell(ch->getName(), "The minimum bid has to be greater than zero, and if you specify a buyout price it has to be higher than the minimum bid.");
    return;
  }


  if(!(obj=generic_find_obj(name, FIND_OBJ_INV, ch))){
    myself->doTell(ch->getName(), "I can't find that object!");
    return;
  }
  
  --(*obj);
  
  db.query("select max(ticket)+1 as ticket from shopownedauction");
  
  if(db.fetchRow())
    ticket=convertTo<int>(db["ticket"]);

  is.openFile(fmt("mobdata/auction/%d/%d") % myself->mobVnum() % ticket);
  is.st.version=CURRENT_RENT_VERSION;
  is.writeVersion();
  is.raw_write_item(obj);

  db.query("insert into shopownedauction values (%i, %i, %i, %i, %i, %i)",
	   shop_nr, ticket, min_bid, ch->getPlayerID(), buyout, days);
  delete obj;

  myself->doTell(ch->getName(), "Your item has been placed on the auction block.");

  return;
}


void auctionBuy(TBeing *ch, TMonster *myself, sstring arg)
{


}


int auctioneer(TBeing *ch, cmdTypeT cmd, const char *arg, TMonster *myself, TObj *)
{
  TDatabase db(DB_SNEEZY);
  int shop_nr;

  if(cmd!=CMD_WHISPER && cmd!=CMD_BUY && cmd!=CMD_LIST && cmd!=CMD_SELL)
    return false;

  shop_nr=find_shop_nr(myself->number);

  switch(cmd){
    case CMD_WHISPER:
      return shopWhisper(ch, myself, shop_nr, arg);
    case CMD_LIST:
      auctionList(ch, myself);
      break;
    case CMD_BUY:
      auctionBuy(ch, myself, arg);
      break;
    case CMD_SELL:
      auctionSell(ch, myself, arg);
      break;
    default:
      break;
  }

  return true;
}
