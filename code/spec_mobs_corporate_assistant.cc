#include "stdsneezy.h"
#include "database.h"


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

sstring talenDisplay(int talens)
{
  float t;

  if(talens>1000000){
    t=(int)(talens/100000);
    return fmt("%.1fM") % (t/10.0);
  } else if(talens > 10000){
    t=(int)(talens/1000);
    return fmt("%ik") % (int)t;
  }

  return fmt("%i") % talens;
}


void corpListing(TBeing *ch, TMonster *me)
{
  TDatabase db(DB_SNEEZY);
  int corp_id=0;
  
  db.query("select c.corp_id, c.name, sum(s.gold) as gold from corporation c, shopowned so, shop s where c.corp_id=so.corp_id and so.shop_nr=s.shop_nr group by c.corp_id, c.name order by gold desc");
  
  me->doTell(ch->getName(), "I know about the following corporations:");
  while(db.fetchRow()){
    corp_id=convertTo<int>(db["corp_id"]);

    me->doTell(ch->getName(), fmt("%-2i| %s - %s talens, %s in assets") % 
	       corp_id % db["name"] % 
	       talenDisplay(convertTo<int>(db["gold"])) % 
	       talenDisplay(getAssets(corp_id)));
  }
}

void corpSummary(TBeing *ch, TMonster *me, int corp_id)
{
  TDatabase db(DB_SNEEZY);
  int value=0, gold=0, shopcount=0;
  sstring buf;

  if(!corp_id){
    me->doTell(ch->getName(), "I don't have any information for that corporation.");
    return;
  }
  
  db.query("select c.name, sum(s.gold) as gold, count(s.shop_nr) as shops from corporation c, shopowned so, shop s where c.corp_id=so.corp_id and c.corp_id=%i and so.shop_nr=s.shop_nr group by c.corp_id, c.name order by c.corp_id", corp_id);
  
  if(!db.fetchRow()){
    me->doTell(ch->getName(), "I don't have any information for that corporation.");
    return;
  }
  
  me->doTell(ch->getName(), fmt("%-3i| %s") %
	     corp_id % db["name"]);

  gold=convertTo<int>(db["gold"]);
  value=getAssets(corp_id);
  shopcount=convertTo<int>(db["shops"]);

  me->doTell(ch->getName(), fmt("Talens:      %12s") % 
	     (fmt("%i") % gold).comify());
  me->doTell(ch->getName(), fmt("Assets:      %12s") % 
	     (fmt("%i") % value).comify());
  me->doTell(ch->getName(), fmt("Shops (x1M): %12s") % 
	     (fmt("%i") % (shopcount*1000000)).comify());
  me->doTell(ch->getName(), fmt("Total value: %12s") %
	     (fmt("%i") % (gold+value+(shopcount * 1000000))).comify());


  // officers
  db.query("select name from corpaccess where corp_id=%i", corp_id);
  
  buf="";
  while(db.fetchRow()){
    buf+=" ";
    buf+=db["name"];
  }
  me->doTell(ch->getName(), fmt("Corporate officers are:%s") % buf);




  // shops    
  db.query("select s.shop_nr, s.in_room, s.gold from shop s, shopowned so where s.shop_nr=so.shop_nr and so.corp_id=%i order by s.gold desc", corp_id);
  
  TRoom *tr=NULL;

  me->doTell(ch->getName(), "The following shops are owned by this corporation:");
  
  while(db.fetchRow()){
    if((tr=real_roomp(convertTo<int>(db["in_room"])))){
      gold=convertTo<int>(db["gold"]);
      me->doTell(ch->getName(), fmt("%-3s| %s with %s talens.") %
		 db["shop_nr"] % tr->getName() % talenDisplay(gold));
    }
  }
}


int corporateAssistant(TBeing *ch, cmdTypeT cmd, const char *arg, TMonster *me, TObj *)
{
  if(cmd != CMD_LIST || !ch || !me)
    return FALSE;

  TDatabase db(DB_SNEEZY);
  int tmp=0;
  sstring buf;

  // list short summary of all corporations
  if(!*arg){
    corpListing(ch, me);
    return true;
  }

  // list details of a specific corporation
  if(*arg){
    if((tmp=convertTo<int>(arg))){
      corpSummary(ch, me, tmp);
    }


    return true;
  }    



  return TRUE;
}
