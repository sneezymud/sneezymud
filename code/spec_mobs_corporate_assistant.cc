#include "stdsneezy.h"
#include "database.h"

sstring talenDisplay(int talens)
{
  float t;

  if(talens>1000000){
    t=(int)(talens/100000);
    return fmt("%.1fM") % (t/10.0);
  } else if(talens > 1000){
    t=(int)(talens/1000);
    return fmt("%fM") % t;
  }

  return fmt("%i") % talens;
}


void corpListing(TBeing *ch, TMonster *me)
{
  TDatabase db(DB_SNEEZY);
  
  db.query("select c.corp_id, c.name, sum(s.gold) as gold from corporation c, shopowned so, shop s where c.corp_id=so.corp_id and so.shop_nr=s.shop_nr group by c.corp_id, c.name order by gold desc");
  
  me->doTell(ch->getName(), "I know about the following corporations:");
  while(db.fetchRow()){
    me->doTell(ch->getName(), fmt("%-2s| %s - %s talens") % db["corp_id"] % db["name"] % talenDisplay(convertTo<int>(db["gold"])));
  }
}

void corpSummary(TBeing *ch, TMonster *me, int corp_id)
{
  TDatabase db(DB_SNEEZY);
  int value=0, gold=0;
  TObj *o=NULL;
  TMonster *keeper;
  
  if(!corp_id){
    me->doTell(ch->getName(), "I don't have any information for that corporation.");
    return;
  }
  
  db.query("select c.name, sum(s.gold) as gold from corporation c, shopowned so, shop s where c.corp_id=so.corp_id and c.corp_id=%i and so.shop_nr=s.shop_nr group by c.corp_id, c.name order by c.corp_id", corp_id);
  
  if(!db.fetchRow()){
    me->doTell(ch->getName(), "I don't have any information for that corporation.");
    return;
  }
  
  me->doTell(ch->getName(), fmt("%-3i| %s") %
	     corp_id % db["name"]);

  gold=convertTo<int>(db["gold"]);



  vector <int> keepers;
  db.query("select keeper from shop where shop_nr in (select shop_nr from shopowned where corp_id=%i)", corp_id);
  while(db.fetchRow()){
    keepers.push_back(convertTo<int>(db["keeper"]));
  }


  for(TBeing *tb=character_list;tb;tb=tb->next){
    for(unsigned int k=0;k<keepers.size();++k){
      if((keeper=dynamic_cast<TMonster *>(tb)) &&
	 keeper->mobVnum() == keepers[k]){
	for(TThing *tt=keeper->getStuff();tt;tt=tt->nextThing){
	  o=dynamic_cast<TObj *>(tt);
	  value+=o->obj_flags.cost;
	}
      }
    }
  }
  
  me->doTell(ch->getName(), fmt("Talens:      %10i") % gold);
  me->doTell(ch->getName(), fmt("Assets:      %10i") % value);
  me->doTell(ch->getName(), fmt("Shops:       %10i") % keepers.size());

  me->doTell(ch->getName(), fmt("Total value: %10i") %
	     (gold+value+(keepers.size() * 1000000)));


  // officers
  db.query("select name from corpaccess where corp_id=%i", corp_id);
  
  sstring buf="";
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
      me->doTell(ch->getName(), fmt("%-3s| %s with %s talens.") %
		 db["shop_nr"] % tr->getName() % db["gold"]);
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
    tmp=convertTo<int>(arg);
    corpSummary(ch, me, tmp);
    return true;
  }    



  return TRUE;
}
