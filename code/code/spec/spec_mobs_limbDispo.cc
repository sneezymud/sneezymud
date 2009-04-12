#include "monster.h"
#include "room.h"
#include "obj_corpse.h"
#include "obj_drinkcon.h"
#include "obj_trash.h"
#include "mail.h"
#include "database.h"

const int CART_VNUM = 33313;
const int CONTENTS_VNUM = 33314;
const int FEE = 1;

int limbDispo(TBeing *ch, cmdTypeT cmd, const char *arg, TMonster *mob, TObj *)
{
  if (!(cmd == CMD_PUT || cmd == CMD_GIVE || cmd == CMD_GENERIC_PULSE)) {
     return FALSE;
  }
  sstring sarg = arg;

  TThing *t = NULL;
  TObj *cart = NULL;
  TObj *contents = NULL;
  for(StuffIter it=mob->roomp->stuff.begin();it!=mob->roomp->stuff.end() && (t=*it);++it) {
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
      vlogf(LOG_LOW, "Error loading objects in spec_mobs_limbDispo.cc");
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
    

  if(!is_abbrev(sarg.word(1), "cart") &&
      !(is_abbrev(sarg.word(1), "in") && is_abbrev(sarg.word(2), "cart"))) {
    return FALSE;
  }
  
  t = NULL;
  TCorpse *limb = NULL;
  TTrash *tooth = NULL;
  TDrinkCon *heart = NULL;
  TObj *bodypart = NULL;
  int foundsomething = 0;
  for(StuffIter it=ch->stuff.begin();it!=ch->stuff.end() && (t=*it);++it) {
    if (!isname(sarg.word(0), t->name))
        continue;
    foundsomething++;
    
    limb = dynamic_cast<TCorpse *>(t);
    if (limb)
      bodypart = limb;
    else {
      tooth = dynamic_cast<TTrash *>(t);
      if (tooth && isname("tooth lost limb", tooth->name))
        bodypart = tooth;
      if (!tooth) {
        heart = dynamic_cast<TDrinkCon *>(t);
        if (heart && isname("heart lost limb", heart->name))
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

  act("You put $N into $p's cart.", TRUE, ch, mob, bodypart, TO_CHAR);
  act("$n puts $N into $p's cart.", TRUE, ch, mob, bodypart, TO_ROOM); 
  sstring stmp = format("There's your %d talen, compliments of our most generous and sanitary King.") % FEE;
  mob->doSay(stmp);
  
  ch->addToMoney(FEE, GOLD_SHOP_RESPONSES);
  
  /* record data for the limb quest */
  sstring partname = bodypart->name;

  // look for the chopped limb quest flag
  // we don't want any other stuff to enter this block
  if (partname.find("[q]") != sstring::npos) {
    // record stuff for limb questing
    std::vector <sstring> partinfo;
    split_string(partname, " []\n\r\t", partinfo);
    bool record_part = TRUE;
    
    // we're expecting the end of the partname to be [bodypart] [slot #] [mob vnum] [player that chopped it]
    // parsing from the end to the beginning
    
    // who chopped it
    sstring chopper = partinfo.back();
    while (chopper.length() == 0 and partinfo.size() > 0) {
      partinfo.erase(partinfo.end()-1);
      chopper = partinfo.back();
    }
    if (chopper.isNumber()) {
      chopper = "UNKNOWN";
      record_part = FALSE;
    }
    
    // mob vnum
    sstring mob_vnum = "";
    while (mob_vnum.length() == 0 and partinfo.size() > 0) {
      partinfo.erase(partinfo.end()-1);
      mob_vnum = partinfo.back();
    }
    int m_vnum = 0;
    if (is_number(mob_vnum))
      m_vnum = convertTo<int>(mob_vnum);
    else
      record_part = FALSE;
    
    // slot # (expect 0 for hearts, eyes, genitals)
    // a tooth should be 0 or -1 if it was collected from before the limb quest
    // assuming the tooth generating code was updated in crit_combat.cc at start/finish of quest
    sstring slot_num = "";
    while (slot_num.length() == 0 and partinfo.size() > 0) {
      partinfo.erase(partinfo.end()-1);
      slot_num = partinfo.back();
    }
    int slot = 0;
    if (is_number(slot_num))
      slot = convertTo<int>(slot_num);
    else
      record_part = FALSE;
    
    // part name
    sstring mob_part = "";
    while (mob_part.length() == 0 and partinfo.size() > 0) {
      partinfo.erase(partinfo.end()-1);
      mob_part = partinfo.back();
    }
    mob_part = mob_part.replaceString("-", " ");

    if (chopper.length() > 80 || mob_part.length() > 80) {
      record_part = FALSE;
    }
    
    if (record_part) {
      TDatabase db(DB_SNEEZY);
      // get team affiliation for cutesy message below
      sstring team;
      bool samaritan = FALSE;
      db.query("select (select team from quest_limbs_team where player = '%s') as chopper_team, (select team from quest_limbs_team where player = '%s') as caddy_team", chopper.c_str(), ch->name);
      if (db.fetchRow()) {
        team = db["chopper_team"];
        if (chopper.compare(ch->name)) {
          // turning in someone else's limb
          samaritan = TRUE;
        }
      }
      else
        team = "";
      db.query("insert quest_limbs (player, team, mob_vnum, slot_num, slot_name) select '%s', '%s', %i, %i, '%s'", chopper.c_str(), team.c_str(), m_vnum, slot, mob_part.c_str());
      vlogf(LOG_MAROR, format("Chop shop: %s") % partname);
      
      if (!team.empty()) {
        if (samaritan) {
          mob->doWhisper(format("%s Why ain't you thoughtful, picking up after %s's mess!") % ch->name % chopper);
          if (team.compare(db["caddy_team"]))
            mob->doWhisper(format("%s I'll make sure their gang, <o>%s<1>, gets the blame for this one.") % ch->name % team);
          else
            mob->doWhisper(format("%s I'll make sure your gang, <o>%s<1>, gets the nod for this one.") % ch->name % team);
        } else {
          mob->doWhisper(format("%s I'll make sure your gang, <o>%s<1>, gets the nod for this one.") % ch->name % team);
        }
        delete bodypart;
        bodypart=NULL;
        return TRUE;
      }
    } else {
      vlogf(LOG_MAROR, format("Chop shop not recorded in db: %s") % partname);
    }
  }
  
  delete bodypart;
  bodypart=NULL;

  sstring resp;
  switch(::number(1,11)) {
    case 1:
    case 2:
    case 3:
    case 4:
      resp = "Thanks for helping to keep Grimhaven clean.";
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
    case 10:
      resp = "Whoa!  That's a whopper!";
    default:
      resp = "Seen a lot of those this week, I have.";
      break;
  }
  mob->doWhisper(format("%s %s") % ch->name % resp);
  
  return TRUE;
}
