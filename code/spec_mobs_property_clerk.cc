#include "stdsneezy.h"
#include "database.h"

int propertyClerk(TBeing *ch, cmdTypeT cmd, const char *argument, TMonster *me, TObj *)
{
  sstring arg=argument;
  TDatabase db(DB_SNEEZY);

  if(!me || !ch)
    return false;

  if(cmd==CMD_BUY){
    if(arg.empty()){
      me->doTell(ch->getName(), "Which property do you want to buy a replacement key for?");
    } else {
      db.query("select p.owner as owner_id, p.key_vnum as key_vnum from property p, obj o where p.key_vnum=o.vnum and p.id=%s", arg.word(0).c_str());

      db.fetchRow();

      if(ch->getPlayerID() != convertTo<int>(db["owner_id"])){
	me->doTell(ch->getName(), "Hey, you don't own that property!");
	return true;
      } else {
	if(ch->getMoney() < 25000){
	  me->doTell(ch->getName(), "It costs 25000 talens to mint a new high security key.  You don't have enough!");
	  return true;
	} else {
	  me->doTell(ch->getName(), "That will be 25000 talens to mint a new high security key.");
	  ch->addToMoney(-25000, GOLD_SHOP);
	}

	TObj *key=read_object(convertTo<int>(db["key_vnum"]), VIRTUAL);
	*ch += *key;
	me->doTell(ch->getName(), "Here you go!");
	act("$n gives you $p.", 0, me, key, ch, TO_VICT);
      }
    }
  }
  

  if(cmd==CMD_LIST){
    if(arg.empty()){
      db.query("select id, name from property order by id");
      
      while(db.fetchRow()){
	me->doTell(ch->getName(), fmt("%-2i| %s") % 
		   convertTo<int>(db["id"]) % db["name"]);
      }
    } else {
      db.query("select p.id as id, p.name as name, p.owner as owner_id, pl.name as owner, p.key_vnum as key_vnum, o.short_desc as keyname, p.entrance as entrance_id, r.name as entrance from property p, player pl, obj o, room r where p.owner=pl.id and p.key_vnum=o.vnum and r.vnum=p.entrance and p.id=%s", arg.word(0).c_str());
      
      while(db.fetchRow()){
	if(ch->isImmortal()){
	  me->doTell(ch->getName(), fmt("%-2i| %s") % 
		     convertTo<int>(db["id"]) % db["name"]);
	  me->doTell(ch->getName(), fmt("Owned by: %s (%i)") % 
		     db["owner"] % convertTo<int>(db["owner_id"]));
	  me->doTell(ch->getName(), fmt("Key: %s (%i)") % 
		     db["keyname"] % convertTo<int>(db["key_vnum"]));
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
