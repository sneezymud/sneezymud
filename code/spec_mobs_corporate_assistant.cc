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
  int corp_id=0, val=0, gold=0, shopval=0;
  multimap <int, sstring, std::greater<int> > m;
  multimap <int, sstring, std::greater<int> >::iterator it;

  db.query("select c.corp_id, c.name, sum(s.gold)+b.talens as gold, count(so.shop_nr) as shopcount from shopowned so, shop s, corporation c left outer join shopownedcorpbank b on (b.corp_id=c.corp_id) where c.bank=b.shop_nr and c.corp_id=so.corp_id and so.shop_nr=s.shop_nr group by c.corp_id, c.name, b.talens order by gold desc");
  
  while(db.fetchRow()){
    corp_id=convertTo<int>(db["corp_id"]);
    gold=convertTo<int>(db["gold"]);
    shopval=convertTo<int>(db["shopcount"]) * 1000000;
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

void corpLogs(TBeing *ch, TMonster *me, sstring arg)
{
  TDatabase db(DB_SNEEZY);
  sstring buf, sb;
  int corp_id=0;

  if(ch->isImmortal()){
    corp_id=convertTo<int>(arg);
  } else {
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
	me->doTell(ch->getName(), "You must specify the ID of the corporation you wish look at the logs of.");
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
  }
  

  db.query("select name, action, talens, corptalens, logtime from corplog where corp_id = %i order by logtime desc", corp_id);
  
  while(db.fetchRow()){
    buf = fmt("%19.19s  %12s %10s %10s  Total: %s\n\r") %
      db["logtime"] % db["name"] % db["action"] %
      db["talens"] % db["corptalens"];
    sb += buf;
  }
  
  if (ch->desc)
    ch->desc->page_string(sb, SHOWNOW_NO, ALLOWREP_YES);
}

void corpSummary(TBeing *ch, TMonster *me, int corp_id)
{
  TDatabase db(DB_SNEEZY);
  int value=0, gold=0, shopcount=0, banktalens=0, bank=4;
  sstring buf;
  TRoom *tr=NULL;


  if(!corp_id){
    me->doTell(ch->getName(), "I don't have any information for that corporation.");
    return;
  }
  

  db.query("select c.name, sum(s.gold) as gold, b.talens as banktalens, count(s.shop_nr) as shops, bank from corporation c left outer join shopownedcorpbank b on (c.corp_id=b.corp_id), shopowned so, shop s where c.corp_id=so.corp_id and c.corp_id=%i and so.shop_nr=s.shop_nr group by c.corp_id, c.name, b.talens, c.bank order by c.corp_id", corp_id);
  
  if(!db.fetchRow()){
    me->doTell(ch->getName(), "I don't have any information for that corporation.");
    return;
  }
  
  me->doTell(ch->getName(), fmt("%-3i| %s") %
	     corp_id % db["name"]);

  bank=convertTo<int>(db["bank"]);
  banktalens=convertTo<int>(db["banktalens"]);
  gold=convertTo<int>(db["gold"]);
  value=getAssets(corp_id);
  shopcount=convertTo<int>(db["shops"]);
  
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

  me->doTell(ch->getName(), fmt("Ok, you are depositing %i gold.") % gold);
  ch->addToMoney(-gold, GOLD_XFER);
  corp.setMoney(corp.getMoney() + gold);
  corp.corpLog(ch->getName(), "deposit", gold);
  
  shop_nr=corp.getBank();
  for(banker=character_list;banker;banker=banker->next){
    if(banker->number==shop_index[shop_nr].keeper)
      break;
  }

  if(banker){
    banker->addToMoney(gold, GOLD_XFER);
    dynamic_cast<TMonster *>(banker)->saveItems(fmt("%s/%d") % SHOPFILE_PATH % shop_nr);
    shoplog(shop_nr, ch, dynamic_cast<TMonster *>(banker), "talens", gold, "corporate deposit");
  } else
    vlogf(LOG_BUG, fmt("couldn't find banker for shop_nr=%i, gold=%i!") % 
	  shop_nr % gold);

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
    me->doTell(ch->getName(), fmt("Your corporation only has %i talens.") % tmp);
    return;
  }


  shop_nr=corp.getBank();
  for(banker=character_list;banker;banker=banker->next){
    if(banker->number==shop_index[shop_nr].keeper)
      break;
  }

  if(banker){
    bank_amt=banker->getMoney();
    shoplog(shop_nr, ch, dynamic_cast<TMonster *>(banker), "talens", -gold, "corporate withdrawal");
  } else {
    vlogf(LOG_BUG, fmt("couldn't find banker for shop_nr=%i!") % shop_nr);
    me->doTell(ch->getName(), "serious problem!");
    return;
  }

  if(bank_amt < gold){
    me->doTell(ch->getName(), "I'm afraid your bank has overextended itself, and doesn't have your cash available write now.");
    return;
  }


  corp.setMoney(corp.getMoney() - gold);
  corp.corpLog(ch->getName(), "withdrawal", -gold);

  ch->addToMoney(gold, GOLD_XFER);
  banker->addToMoney(-gold, GOLD_XFER);
   dynamic_cast<TMonster *>(banker)->saveItems(fmt("%s/%d") % SHOPFILE_PATH % shop_nr);

  me->doTell(ch->getName(), fmt("Ok, here is %i talens.") % gold);
  me->doTell(ch->getName(), fmt("Your balance is %i.") % (tmp-gold));
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

  db.query("select gold from corporation where corp_id=%i", corp_id);
  db.fetchRow();

  me->doTell(ch->getName(), fmt("Your balance is %s.") % db["gold"]);


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
      corpLogs(ch, me, arg.word(1));
    } else if(!arg.empty()){
      // list details of a specific corporation
      tmp=convertTo<int>(arg);
      corpSummary(ch, me, tmp);
    }
  } else if(cmd==CMD_DEPOSIT){
    tmp=convertTo<int>(arg);
    corpDeposit(ch, me, tmp, arg.word(2));
  } else if(cmd==CMD_WITHDRAW){
    tmp=convertTo<int>(arg);
    corpWithdraw(ch, me, tmp, arg.word(2));
  } else if(cmd==CMD_BALANCE){
    corpBalance(ch, me, arg);
  } else
    return false;


  return true;
}
