#include "stdsneezy.h"
#include "database.h"

int propertyClerk(TBeing *ch, cmdTypeT cmd, const char *argument, TMonster *me, TObj *)
{
  sstring arg=argument;
  TDatabase db(DB_SNEEZY);

  if(cmd!=CMD_LIST || !me || !ch)
    return false;


  if(cmd==CMD_LIST){
    if(arg.empty()){
      db.query("select id, name from property order by id");
      
      while(db.fetchRow()){
	me->doTell(ch->getName(), fmt("%-2i| %s") % 
		   convertTo<int>(db["id"]) % db["name"]);
      }
    } else {
      db.query("select p.id as id, p.name as name, p.owner as owner_id, pl.name as owner, p.key as key, o.short_desc as keyname, p.entrance as entrance_id, r.name as entrance from property p, player pl, obj o, room r where p.owner=pl.id and p.key=o.vnum and r.vnum=p.entrance and p.id=%s", arg.word(0).c_str());
      
      while(db.fetchRow()){
	if(ch->isImmortal()){
	  me->doTell(ch->getName(), fmt("%-2i| %s") % 
		     convertTo<int>(db["id"]) % db["name"]);
	  me->doTell(ch->getName(), fmt("Owned by: %s (%i)") % 
		     db["owner"] % convertTo<int>(db["owner_id"]));
	  me->doTell(ch->getName(), fmt("Key: %s (%i)") % 
		     db["keyname"] % convertTo<int>(db["key"]));
	  me->doTell(ch->getName(), fmt("Entrance: %s (%i)") %
		     db["entrance"] % convertTo<int>(db["entrance_id"]));
	} else {
	  me->doTell(ch->getName(), fmt("%-2i| %s") % 
		     convertTo<int>(db["id"]) % db["name"]);
	  me->doTell(ch->getName(), fmt("Owned by: %s") % 
		     db["owner"]);
	  me->doTell(ch->getName(), fmt("Entrance: %s") %
		     db["entrance"]);
	}
      }
    }
    return true;
  }

  return false;
}
