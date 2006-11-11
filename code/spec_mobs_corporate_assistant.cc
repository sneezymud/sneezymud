#include "stdsneezy.h"
#include "database.h"
#include "corporation.h"
#include "shop.h"

void corpListing(TBeing *ch, TMonster *me)
{
  TDatabase db(DB_SNEEZY);
  multimap <int, sstring, std::greater<int> > m;
  multimap <int, sstring, std::greater<int> >::iterator it;
  vector <corp_list_data> corp_list;

  corp_list=getCorpListingData();

  for(unsigned int i=0;i<corp_list.size();++i){
    m.insert(pair<int,sstring>(corp_list[i].rank,
		 fmt("%-2i| <r>%-38s<1> | %6s talens  %6s in assets\n\r") % 
			       corp_list[i].corp_id % corp_list[i].name %
			       talenDisplay(corp_list[i].gold) %
			       talenDisplay(corp_list[i].assets)));
  }

  me->doTell(ch->getName(), "I know about the following corporations:");

  sstring buf;
  for(it=m.begin();it!=m.end();++it)
    buf += (*it).second;

  ch->desc->page_string(buf);
}

void corpLogs(TBeing *ch, TMonster *me, sstring arg, sstring corp_arg)
{
  TDatabase db(DB_SNEEZY);
  sstring buf, sb;
  int corp_id=0;

  if(ch->isImmortal()){
    corp_id=convertTo<int>(corp_arg);
  } else {
    db.query("select corp_id from corpaccess where lower(name)='%s'",
	     sstring(ch->getName()).lower().c_str());

    if(corp_arg.empty()){
      if(db.fetchRow())
	corp_id=convertTo<int>(db["corp_id"]);

      if(db.fetchRow()){
	me->doTell(ch->getName(), "You must specify the ID of the corporation you wish to deposit the money for.");
	return;
      }
    } else {
      if(convertTo<int>(corp_arg) == 0){
	me->doTell(ch->getName(), "You must specify the ID of the corporation you wish look at the logs of.");
	return;
      }

      while(db.fetchRow()){
	if(convertTo<int>(db["corp_id"]) == convertTo<int>(corp_arg)){
	  corp_id=convertTo<int>(corp_arg);
	  break;
	}
      }
    }

    if(!corp_id){
      me->doTell(ch->getName(), "You must specify the ID of the corporation you wish to deposit the money for.");
      return;
    }
  }

  if(arg == "daily"){
    db.query("select name, action, sum(talens) as talens, round(avg(corptalens)) as corptalens, concat(extract(year from logtime),'-',extract(month from logtime),'-',extract(day from logtime)) as logtime from corplog where  corp_id = %i group by concat(extract(year from logtime),'-',extract(month from logtime),'-',extract(day from logtime)), name, action order by concat(extract(year from logtime),'-',extract(month from logtime),'-',extract(day from logtime)) desc, talens desc", corp_id);

    sstring last_date="", color_code="<r>";
    int total=0;

    while(db.fetchRow()){
      if(last_date != db["logtime"]){
	if(!last_date.empty()){
	  color_code=(color_code=="<b>")?"<r>":"<b>";
	  buf=fmt("%16.16s  %20.20s %10s %10i<1>\n\r") %
	    "" % "Total" % "" % total;
	  sb += buf;
	}
	total=0;
	last_date=db["logtime"];
      }
      total+=convertTo<int>(db["talens"]);

      buf = fmt("%s%16.16s  %20.20s %10s %10s  Total: %s<1>\n\r") %
	color_code %
	db["logtime"] % db["name"] % db["action"] %
	db["talens"] % db["corptalens"];
      sb += buf;
    }
  } else {
    db.query("select name, action, talens, corptalens, logtime from corplog where corp_id = %i order by logtime desc", corp_id);
    
    while(db.fetchRow()){
      buf = fmt("%16.16s  %20.20s %10s %10s  Total: %s\n\r") %
	db["logtime"] % db["name"] % db["action"] %
	db["talens"] % db["corptalens"];
      sb += buf;
    }
  }
  
  if (ch->desc)
    ch->desc->page_string(sb, SHOWNOW_NO, ALLOWREP_YES);
}

void corpSummary(TBeing *ch, TMonster *me, int corp_id)
{
  TDatabase db(DB_SNEEZY);
  int value=0, gold=0, banktalens=0, bank=4, bankowner=0;
  sstring buf;
  TRoom *tr=NULL;


  if(!corp_id){
    me->doTell(ch->getName(), "I don't have any information for that corporation.");
    return;
  }
  
  db.query("\
  select c.name, sum(so.gold) as gold, b.talens as banktalens, \
    count(s.shop_nr) as shops, bank, sob.corp_id as bankowner \
  from shopowned sob, \
    corporation c left outer join shopownedcorpbank b on(c.corp_id=b.corp_id),\
    shopowned so, shop s \
  where sob.shop_nr=c.bank and c.corp_id=so.corp_id and c.corp_id=%i and \
    so.shop_nr=s.shop_nr \
  group by c.corp_id, c.name, b.talens, c.bank, sob.corp_id \
  order by c.corp_id", corp_id);
  
  if(!db.fetchRow()){
    // they don't own any shops, so use a different query because I'm too
    // lazy to put more outer joins in the one above
    db.query("select c.name, 0 as gold, b.talens as banktalens, 0 as shops, bank, sob.corp_id as bankowner from shopowned sob, corporation c left outer join shopownedcorpbank b on(c.corp_id=b.corp_id) where sob.shop_nr=c.bank and c.corp_id=%i group by c.corp_id, c.name, b.talens, c.bank, sob.corp_id order by c.corp_id", corp_id);

    if(!db.fetchRow()){
      me->doTell(ch->getName(), "I don't have any information for that corporation.");
      return;
    }
  }

  me->doTell(ch->getName(), "This is what I know about that corporation:");
  
  ch->sendTo(COLOR_BASIC, fmt("%-3i| <r>%s<1>\n\r") % corp_id % db["name"]);

  bank=convertTo<int>(db["bank"]);
  bankowner=convertTo<int>(db["bankowner"]);
  banktalens=convertTo<int>(db["banktalens"]);
  gold=convertTo<int>(db["gold"]);
  TCorporation corp(corp_id);
  value=corp.getAssets();

  // we own the bank, so don't count our money twice
  if(bankowner == corp_id)
    gold -= banktalens;

  ch->sendTo(COLOR_BASIC, fmt("<r>Bank Talens:<1> %12s\n\r") %
  	     (fmt("%i") % banktalens).comify());
  ch->sendTo(COLOR_BASIC, fmt("<r>Talens:<1>      %12s\n\r") % 
	     (fmt("%i") % gold).comify());
  ch->sendTo(COLOR_BASIC, fmt("<r>Assets:<1>      %12s\n\r") % 
	     (fmt("%i") % value).comify());
  ch->sendTo(COLOR_BASIC, fmt("<r>Total value:<1> %12s\n\r") %
	     (fmt("%i") % (banktalens+gold+value)).comify());


  // officers
  db.query("select name from corpaccess where corp_id=%i", corp_id);
  
  buf="";
  while(db.fetchRow()){
    buf+=" ";
    buf+=db["name"].cap();
  }
  ch->sendTo(COLOR_BASIC, fmt("Corporate officers are:<r>%s<1>\n\r") % buf);

  // bank
  if((tr=real_roomp(shop_index[bank].in_room))){
    ch->sendTo(COLOR_BASIC, "Corporate bank is:\n\r");
    ch->sendTo(COLOR_BASIC, fmt("%-3i| <r>%s<1>\n\r") % bank % tr->getName());
  }

  // shops    
  db.query("select s.shop_nr, s.in_room, so.gold from shop s, shopowned so where s.shop_nr=so.shop_nr and so.corp_id=%i order by so.gold desc", corp_id);
  
  ch->sendTo(COLOR_BASIC, "The following shops are owned by this corporation:\n\r");
  
  while(db.fetchRow()){
    if((tr=real_roomp(convertTo<int>(db["in_room"])))){
      gold=convertTo<int>(db["gold"]);
      ch->sendTo(COLOR_BASIC, fmt("%-3s| <r>%s<1> with %s talens.\n\r") %
		 db["shop_nr"] % tr->getName() % talenDisplay(gold));
    }
  }
}


void corpDeposit(TBeing *ch, TMonster *me, int gold, sstring arg)
{
  TDatabase db(DB_SNEEZY);
  int corp_id=0;
  unsigned int shop_nr;
  TBeing *banker;

  db.query("select corp_id from corpaccess where lower(name)='%s'",
	   sstring(ch->getName()).lower().c_str());

  if(arg.empty()){
    if(db.fetchRow())
      corp_id=convertTo<int>(db["corp_id"]);

    if(db.fetchRow()){
      me->doTell(ch->getName(), "You must specify the ID of the corporation you wish to deposit the money for.");
      return;
    }
  } else {
    if(convertTo<int>(arg) == 0){
      me->doTell(ch->getName(), "You must specify the ID of the corporation you wish to deposit the money for.");
      return;
    }

    while(db.fetchRow()){
      if(convertTo<int>(db["corp_id"]) == convertTo<int>(arg)){
	corp_id=convertTo<int>(arg);
	break;
      }
    }
  }

  if(!corp_id){
      me->doTell(ch->getName(), "You must specify the ID of the corporation you wish to deposit the money for.");
      return;
  }
  
  TCorporation corp(corp_id);

  if(ch->getMoney() < gold){
    me->doTell(ch->getName(), "You don't have that many talens.");
    return;
  }

  // find the banker
  shop_nr=corp.getBank();
  for(banker=character_list;banker;banker=banker->next){
    if(banker->number==shop_index[shop_nr].keeper)
      break;
  }

  if(!banker){
    vlogf(LOG_BUG, fmt("couldn't find banker for shop %i!") % shop_nr);
    me->doTell(ch->getName(), "I couldn't find the bank!  Tell a god!");
    return;
  }

  me->doTell(ch->getName(), fmt("Ok, you are depositing %i gold.") % gold);

  // transfer
  ch->giveMoney(banker, gold, GOLD_XFER);

  corp.setMoney(corp.getMoney() + gold);
  corp.corpLog(ch->getName(), "deposit", gold);
  
  // save
  dynamic_cast<TMonster *>(banker)->saveItems(fmt("%s/%d") % SHOPFILE_PATH % shop_nr);

  // log
  shoplog(shop_nr, ch, dynamic_cast<TMonster *>(banker), "talens", gold, "corporate deposit");

  me->doTell(ch->getName(), fmt("Your balance is %i.") % corp.getMoney());
}


void corpWithdraw(TBeing *ch, TMonster *me, int gold, sstring arg)
{
  TDatabase db(DB_SNEEZY);
  int corp_id=0, bank_amt=0;
  unsigned int shop_nr;
  TBeing *banker;

  db.query("select corp_id from corpaccess where lower(name)='%s'",
	   sstring(ch->getName()).lower().c_str());

  if(arg.empty()){
    if(db.fetchRow())
      corp_id=convertTo<int>(db["corp_id"]);

    if(db.fetchRow()){
      me->doTell(ch->getName(), "You must specify the ID of the corporation you wish to withdraw the money from.");
      return;
    }
  } else {
    if(convertTo<int>(arg) == 0){
      me->doTell(ch->getName(), "You must specify the ID of the corporation you wish to withdraw the money from.");
      return;
    }

    while(db.fetchRow()){
      if(convertTo<int>(db["corp_id"]) == convertTo<int>(arg)){
	corp_id=convertTo<int>(arg);
	break;
      }
    }
  }

  if(!corp_id){
      me->doTell(ch->getName(), "You must specify the ID of the corporation you wish to withdraw the money from.");
      return;
  }

  TCorporation corp(corp_id);

  int tmp=corp.getMoney();

  if(tmp < gold){
    me->doTell(ch->getName(), fmt("Your corporation only has %i talens.") % 
	       tmp);
    return;
  }


  shop_nr=corp.getBank();
  for(banker=character_list;banker;banker=banker->next){
    if(banker->number==shop_index[shop_nr].keeper)
      break;
  }
  
  if(!banker){
    vlogf(LOG_BUG, fmt("couldn't find banker for shop_nr=%i!") % shop_nr);
    me->doTell(ch->getName(), "I couldn't find the bank, tell a god!");
    return;
  }
  
  bank_amt=banker->getMoney();

  if(bank_amt < gold){
    me->doTell(ch->getName(), "I'm afraid your bank has overextended itself, and doesn't have your cash available right now.");
    return;
  }


  corp.setMoney(corp.getMoney() - gold);
  corp.corpLog(ch->getName(), "withdrawal", -gold);

  banker->giveMoney(ch, gold, GOLD_XFER);

  dynamic_cast<TMonster *>(banker)->saveItems(fmt("%s/%d") % SHOPFILE_PATH % shop_nr);

  me->doTell(ch->getName(), fmt("Ok, here is %i talens.") % gold);
  me->doTell(ch->getName(), fmt("Your balance is %i.") % (tmp-gold));

  shoplog(shop_nr, ch, dynamic_cast<TMonster *>(banker), "talens", -gold, "corporate withdrawal");
}


void corpBalance(TBeing *ch, TMonster *me, sstring arg)
{
  TDatabase db(DB_SNEEZY);
  int corp_id=0;

  db.query("select corp_id from corpaccess where lower(name)='%s'",
	   sstring(ch->getName()).lower().c_str());

  if(arg.empty()){
    if(db.fetchRow())
      corp_id=convertTo<int>(db["corp_id"]);

    if(db.fetchRow()){
      me->doTell(ch->getName(), "You must specify the ID of the corporation you wish to withdraw the money from.");
      return;
    }
  } else {
    if(convertTo<int>(arg) == 0){
      me->doTell(ch->getName(), "You must specify the ID of the corporation you wish to withdraw the money from.");
      return;
    }

    while(db.fetchRow()){
      if(convertTo<int>(db["corp_id"]) == convertTo<int>(arg)){
	corp_id=convertTo<int>(arg);
	break;
      }
    }
  }

  if(!corp_id){
      me->doTell(ch->getName(), "You must specify the ID of the corporation you wish to withdraw the money from.");
      return;
  }

  TCorporation corp(corp_id);

  me->doTell(ch->getName(), fmt("Your balance is %i.") % corp.getMoney());


}


int corporateAssistant(TBeing *ch, cmdTypeT cmd, const char *argument, TMonster *me, TObj *)
{
  if(!ch || !me)
    return FALSE;

  TDatabase db(DB_SNEEZY);
  int tmp=0;
  sstring buf, arg=argument;

  if(cmd==CMD_LIST){
    if(arg.empty()){
      // list short summary of all corporations
      corpListing(ch, me);
    } else if(arg.word(0) == "logs"){
      if(arg.word(1) == "daily"){
	corpLogs(ch, me, arg.word(1), arg.word(2));
      } else {
	corpLogs(ch, me, "", arg.word(1));
      }
    } else if(!arg.empty()){
      // list details of a specific corporation
      tmp=convertTo<int>(arg);
      corpSummary(ch, me, tmp);
    }
  } else if(cmd==CMD_DEPOSIT){
    tmp=convertTo<int>(arg);
    corpDeposit(ch, me, tmp, arg.word(1));
  } else if(cmd==CMD_WITHDRAW){
    tmp=convertTo<int>(arg);
    corpWithdraw(ch, me, tmp, arg.word(1));
  } else if(cmd==CMD_BALANCE){
    corpBalance(ch, me, arg);
  } else
    return false;


  return true;
}
