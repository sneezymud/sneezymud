#include "shop.h"
#include "database.h"
#include "spec_mobs.h"
#include "monster.h"

int taxman(TBeing *ch, cmdTypeT cmd, const char *arg, TMonster *me, TObj *)
{
  TDatabase db(DB_SNEEZY);
  int shop_nr=find_shop_nr(me->number);


  if(cmd==CMD_WHISPER){
    return shopWhisper(ch, me, shop_nr, arg);
  } else if(cmd==CMD_LIST){
    db.query("select s.shop_nr as shop_nr, r.name as name from shop s, shopowned st, room r where st.tax_nr=%i and st.shop_nr=s.shop_nr and r.vnum=s.in_room", shop_nr);
    
    while(db.fetchRow()){
      me->doTell(ch->name, format("%-3s| <r>%s<1>") %
		 db["shop_nr"] % db["name"]);
    }
    
    return TRUE;
  }

  return FALSE;
}
