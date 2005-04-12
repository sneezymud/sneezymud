#include "stdsneezy.h"
#include "database.h"
#include "shop.h"
#include "shopowned.h"
#include "rent.h"
#include "corporation.h"


sstring getPlayerName(int id)
{
  TDatabase db(DB_SNEEZY);
  
  db.query("select name from player where id=%i", id);
  if(!db.fetchRow()){
    vlogf(LOG_BUG, fmt("Couldn't find player name for %i in getPlayerName()!")%
	  id);
    return "unknown";
  }

  return db["name"];
}

void endAuction(int ticket, int bidder, int seller)
{
  TDatabase db(DB_SNEEZY);
  sstring auctioneer, msg;
  ItemLoad il;
  TObj *obj;

  db.query("select s.keeper as keeper, r.name as name from room r, shop s, shopownedauction soa where r.vnum=s.in_room and s.shop_nr=soa.shop_nr and soa.ticket=%i", ticket);

  if(!db.fetchRow()){
    vlogf(LOG_BUG, "Couldn't find auctioneer name in endAuction()!");
    return;
  }

  auctioneer=db["name"];
  

  il.openFile(fmt("mobdata/auction/%d/%d") % convertTo<int>(db["keeper"]) %
	      ticket);
  il.readVersion();
  obj=il.raw_read_item();


  if(bidder==seller){
    msg=fmt("Your auction %i for %s did not sell.\n\r") % 
      ticket % obj->getName();
    msg+="You will have to come by to pick up your object.";

    store_mail(getPlayerName(seller).c_str(), auctioneer.c_str(), msg.c_str());

    db.query("update shopownedauction set current_bid=0 where ticket=%i",
	     ticket);
  } else {
    msg=fmt("Your auction %i for %s was sold.") % ticket % obj->getName();
    msg+="Your money will be deposited to your bank account as soon as the buyer pays.";
    store_mail(getPlayerName(seller).c_str(), auctioneer.c_str(), msg.c_str());
  }


  return;
}




// called once per mud day
void auctionUpdate()
{
  TDatabase db(DB_SNEEZY);

  db.query("select shop_nr, ticket, bidder, seller from shopownedauction where days==1");
  
  while(db.fetchRow()){
    endAuction(convertTo<int>(db["ticket"]), convertTo<int>(db["bidder"]),
	       convertTo<int>(db["seller"]));
  }

  db.query("update shopownedauction set days=days-1");
}


sstring listItem(int ticket, TObj *obj, int bid, int buyout, int days)
{
  sstring buf;

  buf=fmt("[%3i] %38s %15s %15s\n\r") %
    ticket % obj->getName() % obj->equip_condition(-1) % "fit";
  
  
  buf+=fmt("%34i bid %8i buyout %10i days left\n\r") %
    bid % buyout % days;

  return buf;
}


void auctionList(TBeing *ch, TMonster *myself)
{
  TDatabase db(DB_SNEEZY);
  sstring filename;
  int ticket;
  ItemLoad il;
  TObj *obj;
  int shop_nr=find_shop_nr(myself->number);


  db.query("select ticket, current_bid, days, buyout from shopownedauction where shop_nr=%i and seller=%i", shop_nr, ch->getPlayerID());
  
  if(db.fetchRow()){
    myself->doTell(ch->getName(), "These are your auctions:");
  
    do {
      ticket=convertTo<int>(db["ticket"]);
      
      il.openFile(fmt("mobdata/auction/%d/%d") % myself->mobVnum() % ticket);
      il.readVersion();
      obj=il.raw_read_item();


      ch->sendTo(COLOR_BASIC, listItem(ticket, obj, 
				       convertTo<int>(db["current_bid"]),
				       convertTo<int>(db["buyout"]),
				       convertTo<int>(db["days"])));

      delete obj;
    } while(db.fetchRow());

    ch->sendTo("\n\r");
  }
  

  db.query("select ticket, current_bid from shopownedauction where shop_nr=%i and bidder=%i and days <= 0", shop_nr, ch->getPlayerID());
  
  if(db.fetchRow()){
    myself->doTell(ch->getName(), "These are the items you've won:");
  
    do {
      ticket=convertTo<int>(db["ticket"]);
      
      il.openFile(fmt("mobdata/auction/%d/%d") % myself->mobVnum() % ticket);
      il.readVersion();
      obj=il.raw_read_item();

      ch->sendTo(COLOR_BASIC, listItem(ticket, obj,
				       convertTo<int>(db["current_bid"]),
				       convertTo<int>(db["current_bid"]), 0));
      delete obj;
    } while(db.fetchRow());

    ch->sendTo("\n\r");
  }

  

  db.query("select ticket, current_bid, days, buyout from shopownedauction where shop_nr=%i and days>0", shop_nr);
  
  if(db.fetchRow()){
    myself->doTell(ch->getName(), "This is what I have up for auction:");

    do {
      ticket=convertTo<int>(db["ticket"]);
      
      il.openFile(fmt("mobdata/auction/%d/%d") % myself->mobVnum() % ticket);
      il.readVersion();
      obj=il.raw_read_item();


      ch->sendTo(COLOR_BASIC, listItem(ticket, obj, 
				       convertTo<int>(db["current_bid"]),
				       convertTo<int>(db["buyout"]),
				       convertTo<int>(db["days"])));

      delete obj;
    } while(db.fetchRow());

  } else
    myself->doTell(ch->getName(), "I don't have anything up for auction right now");

  return;
}


void auctionSell(TBeing *ch, TMonster *myself, sstring arg)
{
  TObj *obj;
  ItemSave is;
  TDatabase db(DB_SNEEZY);
  int ticket=1;
  int shop_nr=find_shop_nr(myself->number);
  TShopOwned tso(shop_nr, myself, ch);
  TCorporation corp(tso.getCorpID());

  sstring name=arg.word(0);
  int min_bid=convertTo<int>(arg.word(1));
  int days=convertTo<int>(arg.word(2));
  int buyout=convertTo<int>(arg.word(3));

  if(min_bid <= 0 || days <= 0 || (buyout && buyout < min_bid)){
    myself->doTell(ch->getName(), "Usage: sell <item> <minimum bid> <mud days to run auction> <buyout bid>");
    myself->doTell(ch->getName(), "");
    myself->doTell(ch->getName(), fmt("There is a listing fee of %i talens per mud day, as well as a fee of %f percent of the final sale price, if the item sells.") %
		   (int)shop_index[shop_nr].getProfitBuy(NULL, ch) %
		   (shop_index[shop_nr].getProfitSell(NULL, ch) * 100));
    myself->doTell(ch->getName(), "The proceeds will be automatically deposited to your bank account when the buyer pays for the item.");
    return;
  }

  // find the object
  if(!(obj=generic_find_obj(name, FIND_OBJ_INV, ch))){
    myself->doTell(ch->getName(), "You don't have that!");
    return;
  }
  if (!(shop_index[shop_nr].willBuy(obj))) {
    myself->doTell(ch->name, shop_index[shop_nr].do_not_buy);
    return;
  }
  if (will_not_buy(ch, myself, obj, shop_nr)) 
    return;


  // make sure they have a bank account at our bank
  db.query("select 1 from shopownedbank where player_id=%i and shop_nr=%i",
	   ch->getPlayerID(), corp.getBank());

  if(!db.fetchRow()){
    TRoom *tr=real_roomp(shop_index[corp.getBank()].in_room);
    myself->doTell(ch->getName(), fmt("You need to have a bank account at %s in order to sell items here.") % tr->getName());
    return;
  }


  // get the next free ticket number
  db.query("select max(ticket)+1 as ticket from shopownedauction");
  
  if(db.fetchRow())
    ticket=convertTo<int>(db["ticket"]);

  if(!ticket)
    ticket=1;

  // save the item
  --(*obj);
  is.openFile(fmt("mobdata/auction/%d/%d") % myself->mobVnum() % ticket);
  is.st.version=CURRENT_RENT_VERSION;
  is.writeVersion();
  is.raw_write_item(obj);

  // create the auction record for the item
  db.query("insert into shopownedauction (shop_nr, ticket, current_bid, max_bid, bidder, buyout, days, seller) values (%i, %i, %i, %i, %i, %i, %i, %i)",
	   shop_nr, ticket, min_bid, min_bid, ch->getPlayerID(), buyout, days, ch->getPlayerID());

  int fee=(int)shop_index[shop_nr].getProfitBuy(obj, ch) * days;
  tso.doBuyTransaction(fee, "listing fee", "paying");

  delete obj;
  
  myself->doTell(ch->getName(), "Your item has been placed on the auction block.");

  return;
}

void auctionBuy(TBeing *ch, TMonster *myself, sstring arg)
{
  int ticket=convertTo<int>(arg.word(0));
  TDatabase db(DB_SNEEZY);
  int shop_nr=find_shop_nr(myself->number);
  int days, bidder, bid, fee;
  TShopOwned tso(shop_nr, myself, ch);
  TCorporation corp(tso.getCorpID());

  if(!ticket){
    myself->doTell(ch->getName(), "That isn't a valid item number.");
    return;
  }

  db.query("select current_bid, bidder, days from shopownedauction where shop_nr=%i and ticket=%i", shop_nr, ticket);
  
  if(!db.fetchRow()){
    myself->doTell(ch->getName(), "That isn't a valid item number.");
    return;
  }
  
  days=convertTo<int>(db["days"]);
  bidder=convertTo<int>(db["bidder"]);
  bid=convertTo<int>(db["current_bid"]);
  fee=(int)((float)bid * shop_index[shop_nr].getProfitSell(NULL, ch));

  if(days > 0){
    myself->doTell(ch->getName(), "That auction isn't over yet.");
  } else if(bidder != ch->getPlayerID()){
    myself->doTell(ch->getName(), "You didn't win that auction.");
  } else if(ch->getMoney() < bid){
    myself->doTell(ch->getName(), "You can't afford to pay for that item.");
  } else {
    ItemLoad il;

    il.openFile(fmt("mobdata/auction/%d/%d") % myself->mobVnum() % ticket);
    il.readVersion();
    TObj *obj=il.raw_read_item();
    
    TShopOwned tso(shop_nr, myself, ch);
    tso.doBuyTransaction(bid, obj->getName(), "buying", obj);

    myself->addToMoney(-(bid-fee), GOLD_SHOP);
    db.query("update shopownedbank set talens=talens+%i where player_id=%i and shop_nr=%i", (bid-fee), ch->getPlayerID(), corp.getBank());
    shoplog(corp.getBank(), ch, myself, myself->getName(), (bid-fee), "auction");
    shoplog(shop_nr, ch, myself, "talens", -(bid-fee), "auction");
    

    *ch += *obj;
    myself->doTell(ch->getName(), fmt("That'll be %i talens.") % bid);
    ch->sendTo(COLOR_BASIC, fmt("You now have %s.") % obj->getName());
    
    db.query("delete from shopownedauction where ticket=%i", ticket);
  }

  return;
}


void auctionBid(TBeing *ch, TMonster *myself, sstring arg)
{
  int ticket=convertTo<int>(arg.word(0));
  int my_bid=convertTo<int>(arg.word(1));
  int current_bid, buyout, max_bid, bidder, seller;
  TDatabase db(DB_SNEEZY);
  int shop_nr=find_shop_nr(myself->number);

  if(!ticket){
    myself->doTell(ch->getName(), "Make a bid on what item?");
    return;
  }

  db.query("select seller, bidder, current_bid, max_bid, buyout from shopownedauction where ticket=%i", ticket);

  if(!db.fetchRow()){
    myself->doTell(ch->getName(), "I don't have that item.");
    return;
  }

  current_bid=convertTo<int>(db["current_bid"]);
  max_bid=convertTo<int>(db["max_bid"]);
  buyout=convertTo<int>(db["buyout"]);
  bidder=convertTo<int>(db["bidder"]);
  seller=convertTo<int>(db["seller"]);

  if(my_bid >= buyout){
    myself->doTell(ch->getName(), "Congratulations, you've won the auction.");
    db.query("update shopownedauction set days=0, current_bid=%i, bidder=%i where ticket=%i",
	     my_bid, ch->getPlayerID(), ticket);
    shoplog(shop_nr, ch, myself, fmt("ticket %i, bid %i") % ticket % my_bid, 
	    0, "buyout");
    endAuction(ticket, ch->getPlayerID(), seller);
  } else  if((bidder == ch->getPlayerID()) && my_bid < max_bid){
    myself->doTell(ch->getName(), "You're already the high bidder!");
    myself->doTell(ch->getName(), "Your bid must be greater than your previous max bid if you want to increase it.");
  } else if(bidder == ch->getPlayerID()){
    myself->doTell(ch->getName(), "You're already the high bidder!");
    myself->doTell(ch->getName(), "Your max bid has been increased.");
    db.query("update shopownedauction set max_bid=%i where ticket=%i",
	     my_bid, ticket);
    shoplog(shop_nr, ch, myself, "max bid increased", 0, fmt("%i talens") % 
	    my_bid);
  } else if(my_bid <= current_bid){
    myself->doTell(ch->getName(), "That bid is less than the current bid!");
  } else if(my_bid == max_bid){
    myself->doTell(ch->getName(), "You've been outbidded.");
    db.query("update shopownedauction set current_bid=%i where ticket=%i", 
	     my_bid, ticket);
    shoplog(shop_nr, ch,myself, "outbidded", 0, fmt("%i talens") % my_bid);
  } else if(my_bid < max_bid){
    myself->doTell(ch->getName(), "You've been outbidded.");
    db.query("update shopownedauction set current_bid=%i where ticket=%i", 
	     my_bid+1, ticket);
    shoplog(shop_nr, ch,myself, "outbidded", 0, fmt("%i talens") % (my_bid+1));
  } else {
    myself->doTell(ch->getName(), "Congratulations, you're the high bidder.");
    db.query("update shopownedauction set current_bid=%i, bidder=%i, max_bid=%i where ticket=%i", (max_bid+1), ch->getPlayerID(), my_bid, ticket);
    shoplog(shop_nr, ch, myself, "made bid", 0, fmt("%i talens") % (max_bid+1));
  }

  return;
}


int auctioneer(TBeing *ch, cmdTypeT cmd, const char *arg, TMonster *myself, TObj *)
{
  TDatabase db(DB_SNEEZY);
  int shop_nr;

  if(cmd!=CMD_WHISPER && cmd!=CMD_BUY && cmd!=CMD_LIST && 
     cmd!=CMD_SELL && cmd!=CMD_BID)
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
    case CMD_BID:
      auctionBid(ch, myself, arg);
      break;
    case CMD_SELL:
      auctionSell(ch, myself, arg);
      break;
    default:
      break;
  }

  return true;
}
