#include "stdsneezy.h"
#include "database.h"

int corporateAssistant(TBeing *ch, cmdTypeT cmd, const char *arg, TMonster *me, TObj *)
{
  if(cmd != CMD_LIST || !ch || !me)
    return FALSE;

  TDatabase db(DB_SNEEZY);
  int corp_id=convertTo<int>(arg);
  sstring buf;

  // list short summary of all corporations
  if(!*arg){
    db.query("select c.corp_id, c.name, sum(s.gold) as gold from corporation c, shopowned so, shop s where c.corp_id=so.corp_id and so.shop_nr=s.shop_nr group by c.corp_id, c.name order by c.corp_id");
    
    me->doTell(ch->getName(), "I know about the following corporations:");
    while(db.fetchRow()){
      me->doTell(ch->getName(), fmt("%s) %s - %s talens") % db["corp_id"] % db["name"] % db["gold"]);
    }

    return true;
  }

  // list details of a specific corporation
  if(*arg){
    if(!corp_id){
      me->doTell(ch->getName(), "I don't have any information for that corporation.");
      return FALSE;
    }
    
    db.query("select c.name, sum(s.gold) as gold from corporation c, shopowned so, shop s where c.corp_id=so.corp_id and c.corp_id=%i and so.shop_nr=s.shop_nr group by c.corp_id, c.name order by c.corp_id", corp_id);

    if(!db.fetchRow()){
      me->doTell(ch->getName(), "I don't have any information for that corporation.");
      return FALSE;
    }

    me->doTell(ch->getName(), fmt("%s - %s talens") %
	       db["name"] % db["gold"]);
    me->doTell(ch->getName(), fmt("Corporate ID: %i") % corp_id);


    db.query("select name from corpaccess where corp_id=%i", corp_id);

    buf="";
    while(db.fetchRow()){
      buf+=" ";
      buf+=db["name"];
    }
    me->doTell(ch->getName(), fmt("Corporate officers are:%s") % buf);


    db.query("select s.in_room, s.gold from shop s, shopowned so where s.shop_nr=so.shop_nr and so.corp_id=%i", corp_id);
    
    TRoom *tr=NULL;
    int i=1;
    
    me->doTell(ch->getName(), "The following shops are owned by this corporation:");
    
    while(db.fetchRow()){
      if((tr=real_roomp(convertTo<int>(db["in_room"])))){
	me->doTell(ch->getName(), fmt("%i) %s with %s talens.") %
		   i % tr->getName() % db["gold"]);
      }
      ++i;
    }

    return true;
  }    



  return TRUE;
}
