#include "stdsneezy.h"
#include "database.h"
#include "corporation.h"
#include "shop.h"

int getAssets(int corp_id)
{
  int value=0, keepernum=0;
  TDatabase db(DB_SNEEZY);
  TObj *o=NULL;
  TRoom *room;
  TMonster *keeper;

  db.query("select in_room, keeper from shop where shop_nr in (select shop_nr from shopowned where corp_id=%i)", corp_id);
  
  while(db.fetchRow()){
    room=real_roomp(convertTo<int>(db["in_room"]));
    keepernum=convertTo<int>(db["keeper"]);

    for(TThing *tt=room->getStuff();tt;tt=tt->nextThing){
      if((keeper=dynamic_cast<TMonster *>(tt)) &&
	 keeper->mobVnum() == keepernum){
	for(TThing *t=keeper->getStuff();t;t=t->nextThing){
	  o=dynamic_cast<TObj *>(t);
	  value+=o->obj_flags.cost;
	}
        break;
      }
    }
  }

  return value;
}


void corpListing(TBeing *ch, TMonster *me)
{
  TDatabase db(DB_SNEEZY);
  int corp_id=0, val=0, gold=0, shopval=0, bankowner=0, bankgold=0;
  multimap <int, sstring, std::greater<int> > m;
  multimap <int, sstring, std::greater<int> >::iterator it;

  db.query("select c.corp_id, c.name, sum(s.gold) as gold, b.talens as bankgold, count(so.shop_nr) as shopcount, sob.corp_id as bankowner from (((corporation c left outer join shopownedcorpbank b on (b.corp_id=c.corp_id and c.bank=b.shop_nr)) left outer join shopowned sob on (sob.shop_nr=c.bank)) left outer join shopowned so on (c.corp_id=so.corp_id)) left outer join shop s on (so.shop_nr=s.shop_nr) group by c.corp_id, c.name, b.talens, sob.corp_id order by sum(s.gold)+b.talens desc");
  
  while(db.fetchRow()){
    corp_id=convertTo<int>(db["corp_id"]);
    gold=convertTo<int>(db["gold"]);
    bankgold=convertTo<int>(db["bankgold"]);
    shopval=convertTo<int>(db["shopcount"]) * 1000000;
    bankowner=convertTo<int>(db["bankowner"]);
   
    // if we don't own the bank, record our gold that's in the bank
    // otherwise we end up counting it twice
    if(bankowner!=corp_id)
      gold += bankgold;

    val=gold+getAssets(corp_id)+shopval;


    m.insert(pair<int,sstring>(val,fmt("%-2i| <r>%s<1>") % corp_id % db["name"]));
    m.insert(pair<int,sstring>(val,fmt("  | %s talens, %s in assets") %
			       talenDisplay(gold) % 
			       talenDisplay(getAssets(corp_id)+shopval)));
  }

  me->doTell(ch->getName(), "I know about the following corporations:");

  for(it=m.begin();it!=m.end();++it)
    me->doTell(ch->getName(), (*it).second);
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
    db.query("select name, action, sum(talens) as talens, round(avg(corptalens)) as corptalens, date_trunc('day', logtime) as logtime from corplog where  corp_id = %i group by date_trunc('day', logtime), name, action order by date_trunc('day', logtime) desc, talens desc", corp_id);

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
  int value=0, gold=0, shopcount=0, banktalens=0, bank=4, bankowner=0;
  sstring buf;
  TRoom *tr=NULL;


  if(!corp_id){
    me->doTell(ch->getName(), "I don't have any information for that corporation.");
    return;
  }
  

  db.query("select c.name, sum(s.gold) as gold, b.talens as banktalens, count(s.shop_nr) as shops, bank, sob.corp_id as bankowner from shopowned sob, corporation c left outer join shopownedcorpbank b on (c.corp_id=b.corp_id), shopowned so, shop s where sob.shop_nr=c.bank and c.corp_id=so.corp_id and c.corp_id=%i and so.shop_nr=s.shop_nr group by c.corp_id, c.name, b.talens, c.bank, sob.corp_id order by c.corp_id", corp_id);
  
  if(!db.fetchRow()){
    me->doTell(ch->getName(), "I don't have any information for that corporation.");
    return;
  }
  
  me->doTell(ch->getName(), fmt("%-3i| %s") %
	     corp_id % db["name"]);

  bank=convertTo<int>(db["bank"]);
  bankowner=convertTo<int>(db["bankowner"]);
  banktalens=convertTo<int>(db["banktalens"]);
  gold=convertTo<int>(db["gold"]);
  value=getAssets(corp_id);
  shopcount=convertTo<int>(db["shops"]);

  // we own the bank, so don't count our money twice
  if(bankowner == corp_id)
    gold -= banktalens;
  
  me->doTell(ch->getName(), fmt("Bank Talens: %12s") %
	     (fmt("%i") % banktalens).comify());
  me->doTell(ch->getName(), fmt("Talens:      %12s") % 
	     (fmt("%i") % gold).comify());
  me->doTell(ch->getName(), fmt("Assets:      %12s") % 
	     (fmt("%i") % value).comify());
  me->doTell(ch->getName(), fmt("Shops (x1M): %12s") % 
	     (fmt("%i") % (shopcount*1000000)).comify());
  me->doTell(ch->getName(), fmt("Total value: %12s") %
	     (fmt("%i") % (banktalens+gold+value+(shopcount * 1000000))).comify());


  // officers
  db.query("select name from corpaccess where corp_id=%i", corp_id);
  
  buf="";
  while(db.fetchRow()){
    buf+=" ";
    buf+=db["name"];
  }
  me->doTell(ch->getName(), fmt("Corporate officers are:%s") % buf);

  // bank
  if((tr=real_roomp(shop_index[bank].in_room))){
    me->doTell(ch->getName(), "Corporate bank is:");
    me->doTell(ch->getName(), fmt("%-3i| %s") % bank % tr->getName());
  }

  // shops    
  db.query("select s.shop_nr, s.in_room, s.gold from shop s, shopowned so where s.shop_nr=so.shop_nr and so.corp_id=%i order by s.gold desc", corp_id);
  
  me->doTell(ch->getName(), "The following shops are owned by this corporation:");
  
  while(db.fetchRow()){
    if((tr=real_roomp(convertTo<int>(db["in_room"])))){
      gold=convertTo<int>(db["gold"]);
      me->doTell(ch->getName(), fmt("%-3s| %s with %s talens.") %
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
