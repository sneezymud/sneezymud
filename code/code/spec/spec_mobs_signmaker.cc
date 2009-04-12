#include "monster.h"
#include "shop.h"
#include "obj_note.h"
#include "extern.h"
#include "shopowned.h"
#include "spec_mobs.h"

const int SIGN_COST = 100000;
const int SIGN_VNUM = 33271;
const unsigned int SIGN_MAX = 255;

int signMaker(TBeing *ch, cmdTypeT cmd, const char *arg, TMonster *me, TObj *o)
{
  TObj *sign;
  sstring sign_text;
  int shop_nr=-1;

  if(me)
    shop_nr=find_shop_nr(me->number);

  if(cmd==CMD_WHISPER)
    return shopWhisper(ch, me, shop_nr, arg);

  if(!o || cmd!=CMD_MOB_GIVEN_ITEM)
    return FALSE;

  if(!(dynamic_cast<TNote *>(o))){
    me->doSay("You need to give me a note with your sign message.");
    me->doDrop(add_bars(o->name), NULL);
    return TRUE;
  } else {
    sign_text=sign_text=o->action_description;
    me->doDrop(add_bars(o->name), NULL);
  }
      
  if (sign_text.length() > SIGN_MAX) {
    me->doSay("I can't make a sign that big.");
    return TRUE;
  }
  
  if (ch->getMoney() >= SIGN_COST) {
    if(!(sign = read_object(SIGN_VNUM, VIRTUAL))){
      vlogf(LOG_BUG, "problem loading generic sign in signMaker proc");
      return TRUE;
    }


    ch->giveMoney(me, SIGN_COST, GOLD_SHOP);
    shoplog(shop_nr, ch, me, sign->getName(), SIGN_COST, "creating sign");
    TShopOwned *tso=new TShopOwned(shop_nr, me, ch);
    tso->doReserve();
    delete tso;
    me->saveItems(shop_nr);
  } else {
    me->doSay("This isn't a free service.");
    me->doAction(fname(me->name), CMD_PEER);
    return TRUE;
  }

  // add the extra description
  sign->swapToStrung();
  extraDescription *new_descr = new extraDescription();
  new_descr->keyword = mud_str_dup(sign->name);
  new_descr->description = mud_str_dup(sign_text);
  new_descr->next = sign->ex_description;
  sign->ex_description = new_descr;

  act("You grab a sign and carve out the message..",
      TRUE, me, sign, 0, TO_CHAR);
  act("$n grabs a sign and carves out a message.",
      TRUE, me, sign, 0, TO_ROOM);

  me->doSay("Well, there you are then.");
  *me += *sign;
  sstring giveBuf = format("%s %s") % add_bars(sign->name) % add_bars(ch->name);
  me->doGive(giveBuf, GIVE_FLAG_IGN_DEX_TEXT);

  return TRUE;
}
