#include "stdsneezy.h"
#include "stockmarket.h"
#include "database.h"

TObj *createReport(const sstring &name, const sstring &content)
{
  TObj *note;

  if (!(note = read_object(GENERIC_NOTE, VIRTUAL))) {
    vlogf(LOG_BUG, "Couldn't make a note for stockbroker!");
    return NULL;
  }
  
  note->swapToStrung();
  delete [] note->name;
  note->name = mud_str_dup(name);
  delete [] note->shortDescr;
  note->shortDescr = mud_str_dup(fmt("a %s") % name); 
  delete [] note->getDescr();
  note->setDescr(mud_str_dup(fmt("A crumpled %s lies here.") % name));

  delete [] note->action_description;
  note->action_description = mud_str_dup(content);

  return note;
}


TObj *getPortfolio(TBeing *ch)
{
  TDatabase db(DB_SNEEZY);
  float price, basis, pricediff;
  int shares;
  sstring buf;

  db.query("select so.ticker, so.shares, so.cost_basis, s.price from stockowner so, stockinfo s where s.ticker=so.ticker and so.player_id=%i", ch->getPlayerID());
  
  buf="ticker shares basis price change worth change\n\r";
  
  while(db.fetchRow()){
    price=convertTo<float>(db["price"]);
    basis=convertTo<float>(db["cost_basis"]);
    pricediff=basis-price;
    shares=convertTo<int>(db["shares"]);
    
    buf += fmt("%-6s %6s %6.2f %6.2f %s%+.2f%c<1> %s %s%+i<1>\n") %
      db["ticker"] % talenDisplay(convertTo<int>(db["shares"])) %
      basis %
      getBidPrice(price) %
      (pricediff>0?"<G>":"<R>") % (((price-basis)/basis)*100)%'%'%
      talenDisplay((int)((float)shares * price)) %
      (pricediff>0?"<G>":"<R>") % 
      (int)(pricediff*convertTo<int>(db["shares"]));
    
  }		   
  
  return createReport("<W>portfolio report<1>", buf);
}

TObj *getProspectus(const sstring &ticker)
{
  TDatabase db(DB_SNEEZY);
  sstring buf;

  db.query("select ticker, price, volatility, shares, descr from stockinfo where ticker=upper('%s')", ticker.c_str());
  
  if(db.fetchRow()){
    buf = fmt("%s selling at %f with %i shares outstanding.") % 
      db["ticker"] % getAskPrice(convertTo<float>(db["price"])) %
      convertTo<int>(db["shares"]);
    buf += db["descr"];
  } else {
    buf = "Stock not found.";
  }
  
  return createReport("<W>stock report<1>", buf);
}

#if 0
TObj *getStockCertificate(const sstring &ticker, int shares)
{
  sstring buf;
  
  buf ="           CERTIFICATE OF STOCK\n\r\n\r";
  buf += "The bearer of this certificate is entitled to:\n\r\n\r";
  buf += "    <W>%i<1> common stock shares of <W>%i<1>\n\r\n\r";
  buf += "                             Notarized: <Y>(*)<1>\n\r";

  buf=fmt(buf) % shares % ticker.upper();

  return createReport("<Y>stock certificate<1>", buf);
}
#endif

int stockBroker(TBeing *ch, cmdTypeT cmd, const char *argument, TMonster *me, TObj *){
  TDatabase db(DB_SNEEZY);
  sstring arg=argument, buf;
  float price;
  TObj *note;

  if(cmd != CMD_LIST && cmd != CMD_BUY && cmd != CMD_SELL)
    return FALSE;

  // list -> show portfolio
  // list <ticker>-> stock detail
  // buy <amt>*<ticker> -> buy stock
  // sell <amt>*<ticker> -> sell stock


  ///////////////////////////////// list
  if(cmd==CMD_LIST){
    ///////////////////////////////// list portfolio
    if(arg.empty()){
      if(!(note=getPortfolio(ch)))
	return TRUE;

      *me += *note;
      me->doGive(ch, note, GIVE_FLAG_DROP_ON_FAIL);
      ///////////////////////////////// list stock info
    } else {
      if(!(note=getProspectus(arg)))
	return TRUE;

      *me += *note;
      me->doGive(ch, note, GIVE_FLAG_DROP_ON_FAIL);    
    }
    ///////////////////////////////// buy
  } else if(cmd == CMD_BUY){
    char tmpname[1024] = "\0";
    int num, shares;
    sstring ticker;

    sscanf(arg.c_str(), "%d*%s", &num, tmpname);
    
    db.query("select si.ticker, si.price, si.shares as total, sum(so.shares) as shares from stockinfo si left outer join stockowner so on (so.ticker=si.ticker) where si.ticker=upper('%s') group by si.shares, si.price, si.ticker", tmpname);

    if(!db.fetchRow()){
      me->doTell(ch->getName(), "I don't have any data on that stock.");
      return TRUE;
    }
    price=convertTo<float>(db["price"]);
    ticker=db["ticker"];
    shares=convertTo<int>(db["total"])-convertTo<int>(db["shares"]);

    if(shares < num){
      me->doTell(ch->getName(), fmt("There are only %i shares available."));
      return TRUE;
    }

    if(num > 100){
      me->doTell(ch->getName(), "You can only buy 100 shares at a time.");
      return TRUE;
    }

    if(ch->getMoney() < price*num){
      me->doTell(ch->getName(), "You're broke!");
      return TRUE;
    }

    ch->addToMoney((int)-(price*num), GOLD_XFER);
    
    db.query("select 1 from stockowner where player_id=%i and ticker=upper('%s')",
	     ch->getPlayerID(), ticker.c_str());

    if(db.fetchRow()){
      db.query("update stockowner set shares=shares+%i, cost_basis=((cost_basis*shares) + (%i*%f))/(shares+%i) where player_id=%i and ticker=upper('%s')",
	       num, num, price, num, ch->getPlayerID(), ticker.c_str());
    } else {
      db.query("insert into stockowner values (%i, upper('%s'), %i, %f)",
	       ch->getPlayerID(), ticker.c_str(), num, price);
    }

    
    me->doTell(ch->getName(), fmt("Ok you own %i shares of %s.") %
	       num % ticker);

    ///////////////////////////////// sell
  } else if(cmd == CMD_SELL){
  }


  return TRUE;
}
