#include "stdsneezy.h"
#include "obj_corpse.h"
#include "obj_drinkcon.h"
#include "obj_trash.h"
#include "mail.h"

const int CART_VNUM = 33313;
const int CONTENTS_VNUM = 33314;
const int FEE = 5;

int limbDispo(TBeing *ch, cmdTypeT cmd, const char *arg, TMonster *mob, TObj *)
{
  if (!(cmd == CMD_PUT || cmd == CMD_GIVE || cmd == CMD_GENERIC_PULSE)) {
     return FALSE;
  }
  sstring sarg = arg;

  TThing *t = NULL;
  TObj *cart = NULL;
  TObj *contents = NULL;
  for (t = mob->roomp->getStuff(); t; t = t->nextThing) {
    if (!(cart = dynamic_cast<TObj *>(t))) {
      continue;
    }
    if (obj_index[cart->getItemIndex()].virt ==  CART_VNUM)
      break;
  }
  if (!cart || obj_index[cart->getItemIndex()].virt != CART_VNUM) {
    mob->doSay("Hm, wonder where that dratted cart went.");
    act("You clap yours hands twice and a cart appears.", 
        TRUE, mob, NULL, 0, TO_CHAR);
    act("$n claps his hands twice and a cart appears.",
        TRUE, mob, NULL, 0, TO_ROOM);
    cart = read_object(CART_VNUM, VIRTUAL);
    contents = read_object(CONTENTS_VNUM, VIRTUAL);
    if (!cart || !contents) {
      vlogf(LOG_LOW, fmt("Error loading objects in spec_mobs_limbDispo.cc"));
      return TRUE;
    }
    *mob->roomp += *cart;
    *cart += *contents;
  }
  
  if (cmd == CMD_GENERIC_PULSE && ::number(0,200)) {
    return FALSE;
  }

  if (cmd == CMD_GIVE) {
    if (!isname(sarg.word(1), mob->name)) {
      return FALSE;
    } else {
      mob->doSay("Put it in the cart yourself, you lazy ass.");
      return TRUE;
    }
  } else if (cmd == CMD_GENERIC_PULSE) {
    TRoom *rp = mob->roomp;
    int rc;
    act("You grin a toothless smile.", TRUE, mob, NULL, 0, TO_CHAR);
    act("$n grins a toothless smile.", TRUE, mob, NULL, 0, TO_ROOM);
    mob->doSay("All out of bodyparts then, are we?");
    act("You lift your cart and give it a push.", TRUE, mob, NULL, 0, TO_CHAR);
    act("$n lifts the cart and gives it a push.", TRUE, mob, NULL, 0, TO_ROOM);
    rc = mob->wanderAround();
    if (IS_SET_DELETE(rc, DELETE_THIS))
      return DELETE_THIS;
    if (rp == mob->roomp) {
      act("You lose your grip and the cart comes to an abrupt stop.", TRUE, mob, NULL, 0, TO_CHAR);
      act("$n's loses his grip and the cart comes to an abrupt stop.", TRUE, mob, NULL, 0, TO_ROOM);
      mob->doAction("", CMD_GRUNT);
      mob->doSay("Damn this thing is heavy!");
    } else {
      --(*cart);
      *mob->roomp += *cart;
    }
    return TRUE;
  }
    

  if(!is_abbrev(sarg.word(1), "cart")) {
    return FALSE;
  }
  
  t = NULL;
  TCorpse *limb = NULL;
  TTrash *tooth = NULL;
  TDrinkCon *heart = NULL;
  TObj *bodypart = NULL;
  int foundsomething = 0;
  for (t = ch->getStuff(); t; t = t->nextThing) {
    if (!isname(sarg.word(0), t->name))
        continue;
    foundsomething++;
    
    limb = dynamic_cast<TCorpse *>(t);
    if (limb)
      bodypart = limb;
    else {
      tooth = dynamic_cast<TTrash *>(t);
      if (tooth && isname("tooth", tooth->name))
        bodypart = tooth;
      if (!tooth) {
        heart = dynamic_cast<TDrinkCon *>(t);
        if (heart && isname("heart", heart->name))
          bodypart = heart;
      }
    }
    
    if (bodypart) {
      break;
    }
  }
  
  if (!foundsomething) {
    mob->doSay("Trying to give me stuff you don't have can only lead to trouble.");
    return TRUE;
  }
  if (!bodypart) {
    act("$n quickly covers over the cart.", TRUE, mob, NULL, 0, TO_ROOM);
    mob->doSay("Whoa, now!  I'll have none of those dodgy goods!");
    return TRUE;
  }
  
  if (ch->getMoney() < FEE) {
    act("$n quickly covers over the cart.", TRUE, mob, NULL, 0, TO_ROOM);
    mob->doSay("If you can't afford my services, then you can carry it to the dump yourself.");
    return TRUE;
  }
  
  act("You put $p into $N's cart.", TRUE, ch, mob, bodypart, TO_CHAR);
  act("$n puts $p into $N's cart.", TRUE, ch, mob, bodypart, TO_ROOM); 
  sstring stmp = fmt("That'll be %d talens for clearing up your filth.") % FEE;
  mob->doSay(stmp);
  
  ch->addToMoney(-FEE, GOLD_SHOP_RESPONSES);
  
  time_t lt = time(0);
  sstring buf = fmt("%s deposited by %s at %s") % bodypart->getName()
    % ch->getName() % asctime(localtime(&lt));
  autoMail(NULL, "bump", buf.c_str());
  vlogf(LOG_MAROR, fmt("%s") % buf);
  
  delete bodypart;
  bodypart=NULL;

  sstring resp;
  switch(::number(1,10)) {
    case 1:
    case 2:
    case 3:
    case 4:
      resp = "Thanks.";
      break;
    case 5:
      resp = "Trying to hide the evidence, eh?  Don't worry, your secret is safe with me.";
      break;
    case 6:
      resp = "Hmm... I've seen fresher.";
      break;
    case 7:
      resp = "Barbarian!  Get a real job!";
      break;
    case 8:
      resp = "This is nothing compared to what the last guy gave me.";
      break;
    case 9:
      resp = "Hey!  That came from a good friend of mine!";
      break;
    default:
      resp = "Seen a lot of those this week, I have.";
      break;
  }
  mob->doWhisper(fmt("%s %s") % ch->name % resp);
  
  return TRUE;
}
