//////////////////////////////////////////////////////////////////////
//
//    SneezyMUD - All rights reserved, SneezyMUD Coding Team
//
//    "inventory.cc" - Drop, get etc. commands related to inventory
//
//////////////////////////////////////////////////////////////////////

#include "stdsneezy.h"
#include "games.h"
#include "statistics.h"
#include "disc_looting.h"
#include "shop.h"
#include "obj_spellbag.h"
#include "obj_keyring.h"
#include "obj_base_corpse.h"
#include "obj_player_corpse.h"
#include "obj_open_container.h"
#include "obj_quiver.h"
#include "obj_base_container.h"
#include "obj_table.h"
#include "obj_money.h"
#include "obj_key.h"
#include "obj_trap.h"
#include "obj_portal.h"
#include "obj_arrow.h"
#include "obj_tooth_necklace.h"
#include "obj_potion.h"

// watches rent in, rent out, dropped, etc
#define VERBOSE_LOGS   1
// logs only rare items
#define LOG_ALL_ITEMS  1     

void TThing::logMe(const TBeing *ch, const char *cmdbuf) const
{
  vlogf(LOG_SILENT, fmt("%s%s%s %s.") %  
    (ch ? ch->getName() : "") %
    (ch ? " " : "") %
    cmdbuf % getName());
}

void TObj::logMe(const TBeing *ch, const char *cmdbuf) const
{
  vlogf(LOG_SILENT, fmt("%s%s%s %s. (max: %d, cur: %d)") %  
           (ch ? ch->getName() : "") %
           (ch ? " " : "") %
           cmdbuf % getName() %
           obj_index[getItemIndex()].max_exist %
           obj_index[getItemIndex()].getNumber());
}

void TBeing::logItem(const TThing *obj, cmdTypeT cmd) const
{
  char cmdbuf[200];

  if (cmd == CMD_DROP)
    strcpy(cmdbuf, "dropped");
  else if (cmd == CMD_GET)
    strcpy(cmdbuf, "picked up");
  else if (cmd == CMD_REPAIR)
    strcpy(cmdbuf, "repairing");
  else if (cmd == CMD_MEND)
    strcpy(cmdbuf, "mending");
  else if (cmd == CMD_NORTH)      // kludge
    strcpy(cmdbuf, "repair retrieving");
  else if (cmd == CMD_GIVE)
    strcpy(cmdbuf, "giving away");
  else if (cmd == CMD_EAST)      // kludge
    strcpy(cmdbuf, "give receive");
  else if (cmd == CMD_JUNK)
    strcpy(cmdbuf, "junked");
  else if (cmd == CMD_SOUTH)  // kludge
    strcpy(cmdbuf, "corpsed");
  else if (cmd == CMD_DISARM)
    strcpy(cmdbuf, "was disarmed of");
  else if (cmd == CMD_SELL)
    strcpy(cmdbuf, "selling");
  else if (cmd == CMD_BUY)
    strcpy(cmdbuf, "buying");
  else if (cmd == CMD_DONATE)
    strcpy(cmdbuf, "donated");
#if VERBOSE_LOGS
  // this stuff is for debug purposes mostly
  // really spams the hell out of the logs
  else if (cmd == CMD_LOAD)
    strcpy(cmdbuf, "loaded");
  else if (cmd == CMD_RENT)
    strcpy(cmdbuf, "renting out");
  else if (cmd == CMD_WEST)    // kludge
    strcpy(cmdbuf, "renting in");
  else if (cmd == CMD_NE)    // kludge
    strcpy(cmdbuf, "linkdead purging");
  else if (cmd == CMD_PURGE)
    strcpy(cmdbuf, "purging");
#else
  else if (cmd == CMD_LOAD ||
           cmd == CMD_RENT ||
           cmd == CMD_WEST ||
           cmd == CMD_PURGE ||
           cmd == CMD_NE)
    return;
#endif
  else
    sprintf(cmdbuf, "unknown command %d", cmd);
  
  obj->logMe(this, cmdbuf);
  return;
}

// procedures related to open
// might return DELETE_THIS for this
// might return DELETE_ITEM for obj
int TBeing::rawOpen(TThing *obj)
{
  int rc, res = 0;

  rc = obj->openMe(this);
  if (IS_SET_DELETE(rc, DELETE_THIS)) 
    ADD_DELETE(res, DELETE_ITEM);
  
  if (IS_SET_DELETE(rc, DELETE_VICT)) 
    ADD_DELETE(res, DELETE_THIS);
  
  return res;
}

int TPortal::openMe(TBeing *ch)
{
  char buf[256];

  if (!ch->canSee(this)) {
    ch->sendTo("Open what?\n\r");
    return FALSE;
  }
  if (!isPortalFlag(EX_CLOSED)) {
    ch->sendTo("It's already open!\n\r");
    return FALSE;
  } else if (isPortalFlag( EX_LOCKED)) {
    ch->sendTo("It seems to be locked.\n\r");
    return FALSE;
  }
  if (isPortalFlag( EX_TRAPPED) && ch->doesKnowSkill(SKILL_DETECT_TRAP)) {
    if (detectTrapObj(ch, this)) {
      sprintf(buf, "You start to open $p, but then notice an insidious %s trap...", sstring(trap_types[getPortalTrapType()]).uncap().c_str());
      act(buf, TRUE, ch, this, NULL, TO_CHAR);
      return FALSE;
    }
  }
  if (isPortalFlag( EX_SECRET))
    act("$n opens $p, revealing a secret passageway.", TRUE, ch, this, NULL, TO_ROOM);
  else
    act("$n opens $p.", TRUE, ch, this, NULL, TO_ROOM);

  portal_flag_change(this, EX_CLOSED, "%s is opened from the other side.\n\r",  REMOVE_TYPE);

  act("You open $p.", TRUE, ch, this, NULL, TO_CHAR);
  if (isPortalFlag( EX_TRAPPED)) {
    int rc = ch->triggerPortalTrap(this);
    int res = 0;
    if (IS_SET_DELETE(rc, DELETE_ITEM)) {
      ADD_DELETE(res, DELETE_THIS);
    }
    if (IS_SET_DELETE(rc, DELETE_THIS)) {
      ADD_DELETE(res, DELETE_VICT);
    }
    return res;
  }
  return TRUE;
}

int TThing::openMe(TBeing *ch)
{
  ch->sendTo("That's not a container.\n\r");
  return FALSE;
}

void TObj::dropMe(TBeing *ch, showMeT showme, showRoomT showroom)
{
  if (isObjStat(ITEM_ATTACHED)) {
    if (showme)
      act("You drop $p and reattach it to its place.", 0, ch, this, 0, TO_CHAR);
    if (showroom)
      act("$n drops $p and reattaches it to its place.", 1, ch, this, 0, 
TO_ROOM);
    // since we showed text here, reset and don't show it again
    // but still do recursion
    showme = DONT_SHOW_ME;
    showroom = DONT_SHOW_ROOM;
  } 

  // continue recursion, for text display
  TThing::dropMe(ch, showme, showroom);
}

void TThing::dropMe(TBeing *ch, showMeT showme, showRoomT showroom)
{
  if (showme)
    act("You drop $p.", FALSE, ch, this, 0, TO_CHAR);
  if (showroom)
    act("$n drops $p.", TRUE, ch, this, 0, TO_ROOM);
#if 1
// testing something to avoid problems 8-25 cos
   --(*this);
   *ch->roomp += *this;
#endif
}

void TPCorpse::dropMe(TBeing *ch, showMeT showme, showRoomT showroom)
{
#if 0
// moved below the base call
  setRoomNum(ch->in_room);
  addCorpseToLists();
  saveCorpseToFile();
#endif
  // utilize baseclass so we are recursive and show text, etc.
  TBaseCorpse::dropMe(ch, showme, showroom);
#if 1
  if (ch->roomp && ch->roomp->isRoomFlag(ROOM_SAVE_ROOM))
    return;

  if (getStuff()){
    removeGlobalNext(); // this one may not be necessary
    setRoomNum(in_room);// corpse should be in right room
    addCorpseToLists();
    saveCorpseToFile();
  }
#endif
  return;
}

void TTrap::dropMe(TBeing *ch, showMeT, showRoomT showroom)
{
  extraDescription *ed;

  if (!isname("grenade", name)) {
    ch->sendTo(COLOR_OBJECTS, fmt("You drop %s, concealing and arming it.\n\r") % 
                     sstring(getName()).uncap());

    swapToStrung();
    ed = new extraDescription();
    ed->next = ex_description;
    ex_description = ed;
    ed->keyword = mud_str_dup(TRAP_EX_DESC);
    ed->description = mud_str_dup(getName());

    // utilize baseclass so we are recursive
    // we already displayed appropriate text (room intentionally concealed)
    TObj::dropMe(ch, DONT_SHOW_ME, DONT_SHOW_ROOM);
    return;
  } else {
    ch->sendTo(COLOR_OBJECTS, fmt("You drop %s, activating it.\n\r") % 
                     sstring(getName()).uncap());

    armGrenade(ch);

    // utilize baseclass so we are recursive
    // we already displayed appropriate text to me though
    TObj::dropMe(ch, DONT_SHOW_ME, showroom);
    return;
  }
}

// returns DELETE_THIS, or DELETE_OBJ (tng)
int TBeing::doDrop(const sstring &argument, TThing *tng, bool forcedDrop)
{
  sstring arg, arg2;
  char  newarg[256], buf[256], *buf2;
  TThing *t, *n, *tmp = NULL, *temp = NULL;
  int amount, p = 0, num = 0;
  bool test = false;
  TMoney *money;
  int rc = 0, count = 0, numx = 0;

  arg=argument.word(0);
  arg2=argument.word(1);


  if(arg.empty() && !tng){
    sendTo("Drop what?!?\n\r");
    return FALSE;
  }
  if (!tng && is_number(arg.c_str())) {
    amount = convertTo<int>(arg);
    if (!is_abbrev(arg2, "talens")) {
      sendTo("Sorry, you can't do that (yet)...\n\r");
      return FALSE;
    }
    if (amount <= 0) {
      sendTo("Sorry, you can't do that!\n\r");
      return FALSE;
    }
    if (!isImmortal() && (getMoney() < amount)) {
      sendTo("You haven't got that many talens!\n\r");
      return FALSE;
    }
    sendTo("You drop some money.\n\r");

    act("$n drops some money.", FALSE, this, 0, 0, TO_ROOM);
    if (!(money = create_money(amount))) {
      vlogf(LOG_BUG, "Problem creating money");
      return FALSE;
    }

    *roomp += *money;
    addToMoney(-amount, GOLD_INCOME);

    logItem(money, CMD_DROP);
    rc = genericItemCheck(money);
    if (IS_SET_DELETE(rc, DELETE_ITEM)) {
      delete money;
      money = NULL;
    }
    if (IS_SET_DELETE(rc, DELETE_THIS)) {
      return DELETE_THIS;
    }
    doSave(SILENT_YES);

    return FALSE;
  }
  if (!tng && arg.lower()=="all"){
    for (t = getStuff(); t; t = n) {
      n = t->nextThing;
      if (t->canDrop()) {
        t->dropMe(this, SHOW_ME, SHOW_ROOM);
        logItem(t, CMD_DROP);
        TObj *tobj = dynamic_cast<TObj *>(t);
        if (tobj && tobj->isObjStat(ITEM_NEWBIE) && !tobj->getStuff() &&
              (in_room > 80) && (in_room != ROOM_DONATION)) {
          sendrpf(roomp, "The %s explodes in a flash of white light!\n\r", fname(tobj->name).c_str());
          delete tobj;
          tobj = NULL;
          continue;
        }
        rc = genericItemCheck(t);
        if (IS_SET_DELETE(rc, DELETE_ITEM)) {
          delete t;
          t = NULL;
          continue;
        }
        if (IS_SET_DELETE(rc, DELETE_THIS)) {
          return DELETE_THIS;
        }
        test = true;
      } 
    }
    if (!test) {
      sendTo("You do not seem to have anything.\n\r");
    }
    doSave(SILENT_YES);
    return FALSE;
  } else {
    if (getall(arg.c_str(), newarg)) {
      num = -1;
      arg=newarg;
    } else if ((p = getabunch(arg.c_str(), newarg))) {
      num = p;
      arg=newarg;
    } else
      num = 1;
    strcpy(buf, arg.c_str());
    buf2 = buf;
    numx = get_number(&buf2);
    while (num != 0) {
      if (forcedDrop == TRUE) 
        temp = searchLinkedList(arg, getStuff());
      else 
        temp = searchLinkedListVis(this, arg, getStuff(), &count);
      
      // force it to be obj we passed in
      tmp = tng;
      if (tmp && tmp->parent != this)
        tmp = NULL;
       // otherwise, set it to what we found from arg
      if (!tmp)
        tmp = temp;
      // otherwise, see if forced obj is being equipped
      if (!tmp) {
        tmp = tng;
        if (!tmp || tmp->equippedBy != this)
          tmp = NULL;
      }
      // allow drop weapon to free hands
      // or drop hold-right as alternative
      if (!tmp) {
        tmp = heldInPrimHand();
        if (!tmp || 
            (!isname(buf2, tmp->name) && 
             !is_abbrev(buf2, isRightHanded() ? "hold-right" : "hold-left")) ||
            (numx != 1 && ++count != numx))
          tmp = NULL;
      }
      // same as above case, but check other hand
      if (!tmp) {
        tmp = heldInSecHand();
        TObj *tot = dynamic_cast<TObj *>(tmp);
        if (!tmp ||
            (tot && tot->isPaired()) ||  // don't increment counters if paired
            (!isname(buf2, tmp->name) && 
             !is_abbrev(buf2, isRightHanded() ? "hold-left" : "hold-right")) ||
            (numx != 1 && ++count != numx))
          tmp = NULL;
        
      }
      if (tmp) {
        TObj *tobj = dynamic_cast<TObj *>(tmp);
        if (tobj && tobj->isObjStat(ITEM_NODROP) && !isImmortal()) {
          act("You can't drop $p, it must be CURSED!", 
               FALSE, this, tobj, 0, TO_CHAR);
          num = 0;
          continue;
        } else if (!tmp->canDrop()) {
          num = 0;
          continue;
        }
 
        if (tmp->equippedBy) {
          // we override the showme with the below text
          act("You release your hold on $p, and drop it.",
                           FALSE, this, tmp, 0, TO_CHAR);
          tmp = unequip(tmp->eq_pos);
          *this += *tmp;
          tmp->dropMe(this, DONT_SHOW_ME, SHOW_ROOM);
        } else {
          tmp->dropMe(this, SHOW_ME, SHOW_ROOM);
        }
        test = TRUE;
        logItem(tmp, CMD_DROP);

        if (tobj && tobj->isObjStat(ITEM_NEWBIE) && !tobj->getStuff() &&
              (in_room > 80) && (in_room != ROOM_DONATION)) {
          sendrpf(roomp, "The %s explodes in a flash of white light!\n\r", fname(tobj->name).c_str());
          if (tobj == tng)
            return DELETE_ITEM;
          else {
            delete tobj;
            tobj = NULL;
          }
          test = TRUE;;
          if (num > 0)
            num--;
          continue;
        }
	// Added this to make potions poof if empty --Jesus
	TPotion *tpot = dynamic_cast<TPotion *>(tmp);
	if (tpot && tpot->potIsEmpty()) {
          sendrpf(roomp, "The %s shatters into a million pieces!\n\r", fname(tobj->name).c_str());
          if (tpot == tng)
            return DELETE_ITEM;
          else {
            delete tpot;
            tpot = NULL;
          }
          test = TRUE;;
          if (num > 0)
            num--;
          continue;
	}

        rc = genericItemCheck(tmp);
        if (IS_SET_DELETE(rc, DELETE_ITEM)) {
          if (tmp == tng) 
            return DELETE_ITEM;
          else {
            delete tmp;
            tmp = NULL;
          }
          if (num > 0)
            num--;
          continue;
        }
        if (IS_SET_DELETE(rc, DELETE_THIS)) 
          return DELETE_THIS;
      } else {
        if (num > 0)
          sendTo("You do not have that item.\n\r");
      
        num = 0;
      }
      if (num > 0)
        num--;
    }
    if (test)
      doSave(SILENT_YES);
    return FALSE;
  }
}

int TThing::putMeInto(TBeing *ch, TOpenContainer *sub)
{
  if (dynamic_cast<TSpellBag *>(sub)) {
    act("Sorry, $p can only hold spell components.",
             FALSE, ch, sub, this, TO_CHAR);
    return TRUE;
  }
  if (dynamic_cast<TToothNecklace *>(sub)){
    act("Sorry, $p can only hold teeth.",
	FALSE, ch, sub, this, TO_CHAR);
    return TRUE;
  }
  if (dynamic_cast<TKeyring *>(sub)) {
    act("Sorry, $p can only hold keys.",
	FALSE, ch, sub, this, TO_CHAR);
    return TRUE;
  }
  if (dynamic_cast<TQuiver *>(sub)) {
    act("Sorry, $p can only hold arrows.",
        FALSE, ch, sub, this, TO_CHAR);
    return TRUE;
  }

  return FALSE;
}

// TRUE put ok, FALSE put failed, 2 failed and stop attempting further puts
// DELETE_THIS = ch, DELETE_ITEM = obj, DELETE_VICT = sub
int put(TBeing *ch, TThing *obj, TThing *sub)
{
  int rc;

  if (sub) {
    rc = sub->putSomethingInto(ch, obj);
    int res = 0;
    if (IS_SET_DELETE(rc, DELETE_THIS))
      ADD_DELETE(res, DELETE_VICT);
    if (IS_SET_DELETE(rc, DELETE_VICT))
      ADD_DELETE(res, DELETE_THIS);
    if (IS_SET_DELETE(rc, DELETE_ITEM))
      ADD_DELETE(res, DELETE_ITEM);
    if (res) return res;
    return rc;
      
  } else {
    // no sub specified
    vlogf(LOG_BUG, "put() called with no target.");
    return 2;
  }
}

int TBeing::doPut(const char *argument)
{
  char arg1[128], arg2[128], newarg[100];
  char tmpname[MAX_INPUT_LENGTH], *tmp;
  TObj *obj, *sub;
  TThing *t, *t2;
  int num, p, i, j, iNumb;
  int rc, amount;

  if (gGin.check(this)) {
    if (gGin.move_card(this, argument))
      return FALSE;
  }
  if (checkHearts()) {
    if (gHearts.move_card(this, argument))
      return FALSE;
  }
  if (checkCrazyEights()) {
    if (gEights.move_card(this, argument))
      return FALSE;
  }
  if (checkDrawPoker()) {
    if (gDrawPoker.move_card(this, argument))
      return FALSE;
  }
  if (getPosition() == POSITION_RESTING) {
    sendTo("You can't quite reach from here, so you sit up.\n\r");
    doSit("");
  }
  argument_interpreter(argument, arg1, arg2);
  if (*arg1) {
    if (*arg2) {
      if (is_number(arg1)) {
        amount = convertTo<int>(arg1);
        if (!is_abbrev(arg2, "talens")) {
          sendTo("Sorry, you can't do that (yet)...\n\r");
          return FALSE;
        }
        // reparse to get target
        argument = one_argument(argument, arg1);  // amount
        argument = one_argument(argument, arg1);  // talens
        argument = one_argument(argument, arg2);  // bag

        if (amount <= 0) {
          sendTo("Sorry, you can't do that!\n\r");
          return FALSE;
        }
        if (!isImmortal() && (getMoney() < amount)) {
          sendTo("You haven't got that many talens!\n\r");
          return FALSE;
        }
        if (!(sub = get_obj_vis_accessible(this, arg2))) {
          sendTo(fmt("You don't see any '%s' here.\n\r") % arg2);
          return FALSE;
        }
        sub->putMoneyInto(this, amount);
        return FALSE;
      }

      if (getall(arg1, newarg)) {
        // put all.xx xx
        num = 200;  // limit them to 200 items at a time
        strcpy(arg1, newarg);
      } else if ((p = getabunch(arg1, newarg))) {
        // put 5*xx xx
        num = p;
        strcpy(arg1, newarg);
      } else
        num = 1;

      if (!strcmp(arg1, "all")) {
     

	sub = get_obj_vis_accessible(this, arg2);
	
	if (!sub) {
	  TObj *tmpobj;
	  TBeing *horse;
	  int bits = generic_find(arg2, FIND_CHAR_ROOM, this, &horse, &tmpobj);
	  if (bits)
	    if (horse->isRideable() && horse->equipment[WEAR_BACK]) {
	      TBaseContainer *saddlebag = dynamic_cast<TBaseContainer *>(horse->equipment[WEAR_BACK]);
	      if (saddlebag && saddlebag->isSaddle()) {
		sub = dynamic_cast<TObj *>(saddlebag);
		act("You reach over to $N and open the $o on $s back.",FALSE,this,saddlebag,horse,TO_CHAR);
		act("$n reaches over to $N and opens the $o on $s back.",FALSE,this,saddlebag,horse,TO_NOTVICT);
		act("$n reaches over to you and opens the $o on your back.",FALSE,this,saddlebag,horse,TO_VICT);
	      }
	    }
	}

        if (sub) {

          for (t = getStuff(), i = 0; t; t = t2) {
            t2 = t->nextThing;
            if (t == sub || !canSee(t)) {
              continue;
            }
            rc = put(this, t, sub);
            i++;
            if (IS_SET_DELETE(rc, DELETE_ITEM)) {
              delete t;
              t = NULL;
            } 
            if (IS_SET_DELETE(rc, DELETE_VICT)) {
              delete sub;
              sub = NULL;
              if (IS_SET_DELETE(rc, DELETE_THIS)) {
                return DELETE_THIS;
              } 
              return FALSE;
            } 
            if (IS_SET_DELETE(rc, DELETE_THIS)) {
              return DELETE_THIS;
            } 
            if (rc == 2) {
              // stop putting
              return FALSE;
            }
          }
          if (!i) {
            sendTo("You don't seem to have anything.\n\r");
          }
          return FALSE;
        } else {
          sendTo(fmt("You don't have the '%s'.\n\r") % arg2);
          return FALSE;
        }
      } else {
        strcpy(tmpname, arg1);
        tmp = tmpname;

        if (!(iNumb = get_number(&tmp))) {
          sendTo(fmt("You don't have the '%s'.\n\r") % arg1);
          return FALSE;
        }
	bool firsttimeround=TRUE;
        for (t = getStuff(), i = 0, j = 1; t && i < num; t = t2) {
          t2 = t->nextThing;
          obj = dynamic_cast<TObj *>(t);
          if (obj && isname(tmp, obj->name)) {
            if (j != iNumb) {
              j++;
              continue;
            }
	    sub = dynamic_cast<TObj *>( get_obj_vis_accessible(this, arg2));
	    if (!sub) {
	      TObj *tmpobj;
	      TBeing *horse;
	      int bits = generic_find(arg2, FIND_CHAR_ROOM, this, &horse, &tmpobj);
	      if (bits)
		if (horse->isRideable() && horse->equipment[WEAR_BACK]) {
		  TBaseContainer *saddlebag = dynamic_cast<TBaseContainer *>(horse->equipment[WEAR_BACK]);
		  if (saddlebag && saddlebag->isSaddle()) {
		    sub = dynamic_cast<TObj *>(saddlebag);
		    if (firsttimeround) {
		      act("You reach over to $N and open the $o on $s back.",FALSE,this,saddlebag,horse,TO_CHAR);
		      act("$n reaches over to $N and opens the $o on $s back.",FALSE,this,saddlebag,horse,TO_NOTVICT);
		      act("$n reaches over to you and opens the $o on your back.",FALSE,this,saddlebag,horse,TO_VICT);
		      firsttimeround = FALSE;
		    }
		  }
		}
	    }
	  
	    if (sub) {
	      rc = put(this, obj, sub);
              i++;  // acknowledge the attempt
              if (IS_SET_DELETE(rc, DELETE_ITEM)) {
                delete obj;
                obj = NULL;
              }
              if (IS_SET_DELETE(rc, DELETE_VICT)) {
                delete sub;
                sub = NULL;
              }
              if (IS_SET_DELETE(rc, DELETE_THIS)) {
                return DELETE_THIS;
              }
              if (rc == 2) {
                // stop putting
                num = 0;
              }
            } else {
              sendTo(fmt("You don't have the '%s'.\n\r") % arg2);
              num = 0;
            }
          }
        }  // for loop
        if (i == 0 && num != 0) {
          sendTo(fmt("You don't have the '%s'.\n\r") % arg1);
        }
      }
    } else
      sendTo(fmt("Put %s in what?\n\r") % arg1);
  } else
    sendTo("Put what in what?\n\r");

  return FALSE;
}

// returns DELETE_THIS IF this got toasted (PC ONLY)
int TBeing::doGive(TBeing *victim, TThing *obj, giveTypeT flags)
{
  int rc = 0;
  char buf[128];
  char buf2[128];
  char arg[256]; 
 
  if (!victim || !obj) {
    vlogf(LOG_BUG, "Bad give in doGive");
    return FALSE;
  }
  if (!*victim->name) {
    vlogf(LOG_BUG, "Bad give names in doGive");
    return FALSE;
  }

  strcpy(buf, obj->name);
  strcpy(buf, add_bars(buf).c_str());

  strcpy(buf2, victim->name);
  strcpy(buf2, add_bars(buf2).c_str());
  sprintf(arg, "%s %s", buf, buf2);
  
  rc = doGive(arg, flags);
  return rc;
}

static int genericGiveDrop(TBeing *ch, TObj *obj)
{
  int rc;

  act("You point towards the $g as you start to drop $p.",
       0, ch, obj, 0, TO_CHAR); 
  act("$n points towards the $g as $e starts to drop $p.", 
      0, ch, obj, 0, TO_ROOM);
  rc = ch->doDrop("", obj);
  if (IS_SET_DELETE(rc, DELETE_THIS))
    return DELETE_THIS;

  // Yet another kludge, maybe mob can not see the obj to drop it
  // (which might explain why the give failed too...)
  if (obj->parent == ch) {
    // we still have the obj
    (*obj)--;
    *ch->roomp += *obj;
  }
  return FALSE;
}

// returns DELETE_THIS if this got toasted (PC only)
int TBeing::doGive(const sstring &oarg, giveTypeT flags)
{
  sstring obj_name, vict_name, arg, buf, argument=oarg;
  char newarg[100];
  int amount, num, p, rc = 0;
  TBeing *vict = NULL;
  TObj *obj = NULL;
  bool badWeight = 0, badVol = 0;

  argument = one_argument(argument, obj_name);
  if (is_number(obj_name)) {
    amount = convertTo<int>(obj_name);
    argument = one_argument(argument, arg);
    if (!is_abbrev(arg,"talens")) {
      sendTo("Syntax: give <amount> talens <person>\n\r");
      return FALSE;
    }
    if (amount <= 0) {
      sendTo("Sorry, you can't do that!\n\r");
      return FALSE;
    }
    if ((getMoney() < amount) && !hasWizPower(POWER_GOD)) {
      sendTo("You haven't got that many talens!\n\r");
      return FALSE;
    }
    argument = one_argument(argument, vict_name);

    if (!vict_name.empty() && vict_name.lower()=="to")
      argument = one_argument(argument, vict_name);

    if (vict_name.empty()){
      sendTo("To whom?\n\r");
      return FALSE;
    }
    if (!(vict = get_char_room_vis(this, vict_name))) {
        sendTo("To whom?\n\r");
        return FALSE;
    }
    if (vict->isPlayerAction(PLR_SOLOQUEST) && 
        (flags != GIVE_FLAG_IGN_DEX_TEXT) && 
        (flags != GIVE_FLAG_IGN_DEX_NOTEXT)) {
      act("$N is on a solo quest; you can't give anything to $M.", FALSE, this, NULL, vict, TO_CHAR);
      return FALSE;
    }
    if (!vict->hasHands() && 
        (flags != GIVE_FLAG_IGN_DEX_TEXT) && 
        (flags != GIVE_FLAG_IGN_DEX_NOTEXT)) {
      act("$N has no hands, you can't give $M things.", FALSE, this, 0, vict, TO_CHAR);
      return FALSE;
    }
    if (vict->isPlayerAction(PLR_GRPQUEST) && !isPlayerAction(PLR_GRPQUEST) &&
        (flags != GIVE_FLAG_IGN_DEX_TEXT) && 
        (flags != GIVE_FLAG_IGN_DEX_NOTEXT)) {
      act("$N is on a group quest that you aren't on; you can't give anything to $M.", FALSE, this, NULL, vict, TO_CHAR);
      return FALSE;
    }
    if (fight()) {
      act("Not while fighting.", FALSE, this, 0, 0, TO_CHAR);
      return FALSE;
    }
    if (vict->fight()) {
      act("Not while $N is fighting.", FALSE, this, 0, vict, TO_CHAR);
      return FALSE;
    }
    sendTo(COLOR_MOBS, fmt("You give %d talen%s to %s.\n\r") % amount %             ((amount == 1) ? "" : "s") % pers(vict));
    buf=fmt("$n gives you %d talen%s.\n\r") % 
      amount % ((amount == 1) ? "" : "s");
    if(flags != GIVE_FLAG_SILENT_VICT){
      act(buf, TRUE, this, NULL, vict, TO_VICT);
      act("$n gives some money to $N.", 1, this, 0, vict, TO_NOTVICT);
    }

    giveMoney(vict, amount, GOLD_XFER);

    saveChar(ROOM_AUTO_RENT);
    vict->saveChar(ROOM_AUTO_RENT);
    if ((vict->getMoney() > 500000) && (amount > 100000))
      vlogf(LOG_MISC,fmt("%s gave %d talens to %s.") %  getName() % amount % vict->getName());

    if (!vict->isPc()) {
      // log if it's an owned shopkeeper
      if(vict->spec && dynamic_cast<TMonster *>(vict)->isShopkeeper()){
	unsigned int shop_nr;
	for (shop_nr = 0; (shop_nr < shop_index.size()) && (shop_index[shop_nr].keeper != (vict)->number); shop_nr++);
	
	shoplog(shop_nr, this, dynamic_cast<TMonster *>(vict), "talens", amount, "giving");
      }
      
      // check reponses
      buf=fmt("%i") % amount;

      rc = dynamic_cast<TMonster *>(vict)->checkResponses(this, NULL, buf, CMD_GIVE);
      TMonster *tMob;

      if (rc) {
        // giving money is a trigger, delete the money
        vict->addToMoney(-amount, GOLD_XFER);


        // fix the statistics
        // add it back to xfer, take it out out reqshop
        if (isPc() && GetMaxLevel() < 60) {
          gold_statistics[GOLD_XFER][GetMaxLevel()-1] += amount;
          gold_statistics[GOLD_SHOP_RESPONSES][GetMaxLevel()-1] -= amount;
          // since -amount < 0, I need do nothing to gold_positive
        }
      } else if (!vict->isPc() && (tMob = dynamic_cast<TMonster *>(vict)) &&
                   (mob_index[tMob->getMobIndex()].max_exist > 1) &&
                   !IS_SET(tMob->specials.act, ACT_SENTINEL)) {
          // vict is known to be an NPC, so need not worry about statistics
          // Let's have a little fun.
          // If mob and isn't sentinal...

          if ((amount           >= (vict->GetMaxLevel() * 1000)) ||
              (vict->getMoney() >= (vict->GetMaxLevel() * 1000))) {
            act("$N stares at $n and smiles.",
                TRUE, this, NULL, vict, TO_ROOM);
            act(fmt("$N stares at you and smiles, I think %s likes you.")
                % thirdPerson(POS_SUBJECT),
                TRUE, this, NULL, vict, TO_CHAR);
            act(fmt("$n leaves to make use of %s new found cash.")
                % thirdPerson(POS_POSSESS),
                TRUE, vict, NULL, NULL, TO_ROOM);

            ADD_DELETE(rc, DELETE_THIS);
          }
      }

      if (IS_SET_DELETE(rc, DELETE_THIS)) {
        delete vict;
        vict = NULL;
        REM_DELETE(rc, DELETE_THIS);
      }
      if (IS_SET_DELETE(rc, DELETE_ITEM)) {
        // item was something i wanted, default is to get rid of the item
        delete obj;
        obj = NULL;
        doSave(SILENT_YES);

        if (vict)
          vict->doSave(SILENT_YES);

        REM_DELETE(rc, DELETE_ITEM);
        return rc;
      }
      if (IS_SET_DELETE(rc, DELETE_VICT))
        return DELETE_THIS;
    }

    if (vict) {
      rc = vict->checkSpec(this, CMD_MOB_GIVEN_COINS, arg.c_str(), (TObj *) amount);

      if (IS_SET_DELETE(rc, DELETE_THIS)) {
        delete vict;
        vict = NULL;
      }

      if (IS_SET_DELETE(rc, DELETE_VICT))
        return DELETE_THIS;
    }
    return FALSE;
  } else {
    argument = one_argument(argument, vict_name);

    if (obj_name.empty() || vict_name.empty()){
      sendTo("Give what to whom?\n\r");
      return FALSE;
    }
    if (getall(obj_name.c_str(), newarg)) {
      num = -1;
      obj_name=newarg;
    } else if ((p = getabunch(obj_name.c_str(), newarg))) {
      num = p;
      obj_name=newarg;
    } else
      num = 1;

    // there is a real potential for endless loops here
    // e.g. give all.sword beavis, he gives sword back if not need fixing
    // this would set up a give back&forth endless loop
    // to get around this, save the thing given, if the next time
    // I come through while(), i give it again, indicating it was given
    // back to me, break out.
    TThing *last_given = NULL;
    while (num != 0) {
      if (flags == GIVE_FLAG_DEF) {
        TThing *t_obj = searchLinkedListVis(this, obj_name, getStuff());
        obj = dynamic_cast<TObj *>(t_obj);
        if (!obj) {
          if (num >= -1)
            sendTo("You do not seem to have anything like that.\n\r");
          return FALSE;
        }
      } else {
        // for other flags, lets also ignore visibility...
        TThing *t_obj = searchLinkedList(obj_name, getStuff());
        obj = dynamic_cast<TObj *>(t_obj);
        if (!obj) {
          if (num >= -1)
            sendTo("You do not seem to have anything like that.\n\r");
          return FALSE;
        }
      }
      if (obj == last_given) {
        sendTo("For some reason, you decide to cease giving further things away.\n\r");
        break;
      }
      last_given = obj;

      if (obj->isObjStat(ITEM_NODROP) && !isImmortal() &&
        (flags != GIVE_FLAG_IGN_DEX_TEXT) && 
        (flags != GIVE_FLAG_IGN_DEX_NOTEXT)) {
        sendTo("You can't let go of it; it must be CURSED!\n\r");
        return FALSE;
      }
      // if we aren't using the normal flag, skip visibility
      if (flags == GIVE_FLAG_DEF) {
        if (!(vict = get_char_room_vis(this, vict_name))) {
          sendTo("No one by that name around here.\n\r");
          return FALSE;
        }
      } else {
        if (!(vict = get_char_room(vict_name, in_room))) {
          sendTo("No one by that name around here.\n\r");
          return FALSE;
        }
      }
      if (obj->isObjStat(ITEM_PROTOTYPE) && (!isImmortal() || !vict->isImmortal())) {
        sendTo("A compelling force prevents you from doing so.\n\r");
        return FALSE;
      }
      if (vict == this) {
        sendTo("Gee, giving it to yourself.  How surreal.\n\r");
        return FALSE;
      }
      if (!vict->hasHands() &&
        (flags != GIVE_FLAG_IGN_DEX_TEXT) && 
        (flags != GIVE_FLAG_IGN_DEX_NOTEXT)) {
        act("$N has no hands, you can't give $M things.", FALSE, this, 0, vict, TO_CHAR);
        return FALSE;
      }
      if (flags != GIVE_FLAG_IGN_DEX_TEXT &&
          flags != GIVE_FLAG_IGN_DEX_NOTEXT) {
        if ((obj->getTotalVolume() + vict->getCarriedVolume()) > vict->carryVolumeLimit()) {
          if (flags != GIVE_FLAG_DEF) 
            badVol = TRUE;
          else {
	    if(!isImmortal()){
	      act("$N seems to have $S hands full.", 0, this, 0, vict, TO_CHAR);
	      if(flags != GIVE_FLAG_SILENT_VICT){
		act("$n offers $p to you, but your hands are full.",
		    TRUE, this, obj, vict, TO_VICT);
		act("$n offers $p to $N, but $E is too encumbered to accept it.",
		    TRUE, this, obj, vict, TO_NOTVICT);
	      }
	      
	      return FALSE;
	    } else {
	      act("$N seems to have $S hands full, but you don't let that stop you.", 0, this, 0, vict, TO_CHAR);
	    }
          }
        }
        // obj-weight > vict->carry weight limit
        if (compareWeights(obj->getTotalWeight(TRUE), (vict->carryWeightLimit() - vict->getCarriedWeight())) == -1) {
          if (flags != GIVE_FLAG_DEF) {
            badWeight = TRUE;
          } else {
	    if(!isImmortal()){
	      act("$E can't carry that much weight.", 0, this, 0, vict, TO_CHAR);
	      if(flags != GIVE_FLAG_SILENT_VICT){
		act("$n offers $p to you, but you can't carry that much weight.",
		    TRUE, this, obj, vict, TO_VICT);
		act("$n offers $p to $N, but $E is too wimpy to carry it.",
		    TRUE, this, obj, vict, TO_NOTVICT);
	      }
	      return FALSE;
	    } else {
	      act("$E can't carry that much weight, but you don't let that stop you.", 0, this, 0, vict, TO_CHAR);
	    }
          }
        }
      }  // IGN_DEX
// FIRST force give with text or not bad give
      if ((flags != GIVE_FLAG_IGN_DEX_NOTEXT) && !badWeight && !badVol) {
	if(flags != GIVE_FLAG_SILENT_VICT){
	  act("$n gives $p to $N.", 1, this, obj, vict, TO_NOTVICT);
	  act("$n gives you $p.", 0, this, obj, vict, TO_VICT);
	}
        act("You give $p to $N.", 0, this, obj, vict, TO_CHAR);
        vict->logItem(obj, CMD_EAST);    // kludge to acknowledge receive
        --(*obj);
        *vict += *obj;
        logItem(obj, CMD_GIVE);
      } else if (flags == GIVE_FLAG_IGN_DEX_NOTEXT)  {
        vict->logItem(obj, CMD_EAST);    // kludge to acknowledge receive
        --(*obj);
        *vict += *obj;
        logItem(obj, CMD_GIVE);
      } else if (flags != GIVE_FLAG_DEF) {   // drop
        if (badWeight && badVol) {
          if (flags == GIVE_FLAG_DROP_ON_FAIL) {
            doSay("You can't carry it! I'll just drop it here for you!");
            rc = genericGiveDrop(this, obj);
            if (IS_SET_DELETE(rc, DELETE_THIS))
              return DELETE_THIS;
          }
        } else if (badWeight) {
          if (flags == GIVE_FLAG_DROP_ON_FAIL) {
            doSay("This is too big for you to carry! I'll just drop it here for you!");
            rc = genericGiveDrop(this, obj);
            if (IS_SET_DELETE(rc, DELETE_THIS))
              return DELETE_THIS;
          }
        } else if (badVol) { 
          if (flags == GIVE_FLAG_DROP_ON_FAIL) { // drop it no matter what
            doSay("This is too big for you to carry! I'll just drop it here for you!");
            rc = genericGiveDrop(this, obj);
            if (IS_SET_DELETE(rc, DELETE_THIS))
              return DELETE_THIS;
          }
        } else 
          vlogf(LOG_BUG, fmt("Bad flags in doGive (%s)") %  getName());
        
        doSave(SILENT_YES);
        if (vict)
          vict->doSave(SILENT_YES);
      }
      if (!isImmortal() && isPc() && 
          !vict->isImmortal() && vict->isPc()) {
        if (obj->obj_flags.cost >= 100) {
          switch (CheckStorageChar(this, vict)) {
            case 1:
              vlogf(LOG_MISC, fmt("Storage Character %s giving %s to %s") % getName() %obj->getName() %vict->getName());
              break;
            case 2:
              vlogf(LOG_MISC, fmt("Storage Character %s w/low KAR giving %s to %s w/high KAR") % getName() % obj->getName() %vict->getName());
              break;
            case 0:
            default:
              break;
          }
        }
      }

      if (num > 0)
        num--;

      if (!vict->isPc()) {
        rc = dynamic_cast<TMonster *>(vict)->checkResponses(this, obj, NULL, CMD_GIVE);
        if (IS_SET_DELETE(rc, DELETE_THIS)) {
          delete vict;
          vict = NULL;
          REM_DELETE(rc, DELETE_THIS);
        }
        if (IS_SET_DELETE(rc, DELETE_ITEM)) {
          // item was something i wanted, default is to get rid of the item
          delete obj;
          obj = NULL;
          doSave(SILENT_YES);
          if (vict)
            vict->doSave(SILENT_YES);
          REM_DELETE(rc, DELETE_ITEM);
          return rc;
        }
        if (IS_SET_DELETE(rc, DELETE_VICT))
          return DELETE_THIS;

        if (!vict)
          return rc;
      }

      rc = vict->checkSpec(this, CMD_MOB_GIVEN_ITEM, arg.c_str(), obj);
      if (IS_SET_DELETE(rc, DELETE_THIS)) {
        delete vict;
        vict = NULL;
      }
      if (IS_SET_DELETE(rc, DELETE_ITEM)) {
        delete obj;
        obj = NULL;
      }
      if (IS_SET_DELETE(rc, DELETE_VICT))
        return DELETE_THIS;
      if (obj && vict) {
        rc = vict->genericItemCheck(obj);
        if (IS_SET_DELETE(rc, DELETE_ITEM)) {
          delete obj;
          obj = NULL;
        }
        if (IS_SET_DELETE(rc, DELETE_THIS)) {
          delete vict;
          vict = NULL;
        }
      }
      if (obj) {
        rc = genericItemCheck(obj);
        if (IS_SET_DELETE(rc, DELETE_ITEM)) {
          delete obj;
          obj = NULL;
        }
        if (IS_SET_DELETE(rc, DELETE_THIS)) {
          return DELETE_THIS;
        }
      }
    }
    doSave(SILENT_YES);
    vict->doSave(SILENT_YES);
  }
  return TRUE;
}

// returns 1 : player logged in less then 15(?) minutes
// returns 2 : same host on 2 players, playing time differential extreme and karma
int CheckStorageChar (TBeing *ch, TBeing *other)
{
  time_t ct = time(0);
  time_t lt = ch->player.time.last_logon;

  if ((ct - lt) <= 900)
    return 1;

  if (other && ch) {
    if (ch->desc && other->desc) {
      if (ch->desc->host == other->desc->host) {
        if ((ch->getStat(STAT_CHOSEN, STAT_CHA) <= -10) &&
            (other->getStat(STAT_CHOSEN, STAT_CHA) > 10)) {
          if ((ch->player.time.played * 4) < other->player.time.played) {
            return 2;
          } else if ((other->player.time.played * 4) < ch->player.time.played) {
            return 2;
          }
        }
      }
    }
  }
  return 0;
}

float TBeing::carryWeightLimit() const
{
  float num;
  num = plotStat(STAT_CURRENT, STAT_STR, (float) 10.0, (float) 640.0, (float) 165.0);
  if (isFourLegged())
    num *= 2.0;

  return num;
}

int TBeing::carryVolumeLimit() const
{
  int vol = plotStat(STAT_CURRENT, STAT_DEX, 15000, 150000, 45000);

  // flat racial modifiers

  // modify by height, this should be same as changing based on race
//  vol *= min(70,getHeight());
  vol *= getHeight();
  vol /= 70;

  vol = min(max(vol, 5000), 1000000);

#if 0
  if (equipment[HOLD_LEFT] && equipment[HOLD_RIGHT])
    return vol/3;
  else if (equipment[HOLD_LEFT] || equipment[HOLD_RIGHT])
    return vol/2;
  else
#endif

    return vol;
}

bool TBeing::canCarry(const TThing *obj, silentTypeT silent) const
{
  return obj->canCarryMe(this, silent);
}

bool TMoney::canCarryMe(const TBeing *, silentTypeT) const
{
  // wouldn't want them to not be able to pick up talen piles
  return true;
}

bool TThing::canCarryMe(const TBeing *ch, silentTypeT silent) const
{
  // If there immortal, they can carry what they want.
  if (ch->isImmortal())
    return TRUE;

  if (dynamic_cast<TBeing *>(thingHolding()) == ch) {
    // it's in a bag I'm holding or something, so chances are I'm already
    // alloting a portion of its volume, and all of its weight

    if (ch->getCarriedVolume() + (getTotalVolume() - getReducedVolume(NULL)) > ch->carryVolumeLimit()) {
      if (!silent)
        ch->sendTo(COLOR_OBJECTS,fmt("%s : You need more dexterity to carry that much volume.\n\r") % sstring(getName()).cap());
      return FALSE;
    }
  } else {
    // I'm not holding onto it...

    // obj-weight > free carry weight 
    if (compareWeights(getTotalWeight(TRUE),
                  (ch->carryWeightLimit() - ch->getCarriedWeight())) == -1) {
      if (!silent)
        ch->sendTo(COLOR_OBJECTS, fmt("%s : You don't have enough strength to carry that much weight.\n\r") % sstring(getName()).cap());
      return FALSE;
    }

    if (ch->getCarriedVolume() + getTotalVolume() > ch->carryVolumeLimit()) {
      if (!silent)
        ch->sendTo(COLOR_OBJECTS,fmt("%s : You need more dexterity to carry that much volume.\n\r") % sstring(getName()).cap());
      return FALSE;
    }
  }

  return TRUE;
}

int TThing::getDrechels(int total) const
{
  int num;
  
  if (total) 
    num = (int) (1000.0 * (getTotalWeight(TRUE) - (int) getTotalWeight(TRUE) + .0002));
  else
    num = (int) (1000.0 * (getWeight() - (int) getWeight() + .0002));

  return num;
}

// mostly used to cause "instantaneous" events caused by inv/eq changes.
// make sure it is done AFTER item goes to room
int TBeing::genericItemCheck(TThing *obj)
{
  int ret = 0;
  int rc;

  if (obj->roomp) {
    rc = obj->checkFalling();
    if (IS_SET_DELETE(rc, DELETE_THIS))
      ADD_DELETE(ret, DELETE_ITEM);
  }
  
  // dropping item may make them lose ability to fly
  if (roomp) {
    rc = checkFalling();
    if (IS_SET_DELETE(rc, DELETE_THIS))
      ADD_DELETE(ret, DELETE_THIS);
  }
 
  return ret;
}

bool TObj::canDrop() const
{
  return (!isObjStat(ITEM_NODROP));
}

bool TTrap::canDrop() const
{
  TThing *ch;
  if ((ch = thingHolding())) {
    if (ch->roomp->isRoomFlag(ROOM_PEACEFUL)) {
      act("A strange force prevents you from doing this!",
           FALSE, ch, 0, 0, TO_CHAR);
      return FALSE;
    }
  }

  return TObj::canDrop();
}

int TObj::getAllFrom(TBeing *ch, const char *argument)
{
  ch->sendTo(COLOR_OBJECTS, fmt("%s is not a container.\n\r") % sstring(getName()).cap());
  return FALSE;
}

// return TRUE will stop the get
int TObj::getObjFrom(TBeing *ch, const char *, const char *)
{
  act("$p is not a container.", FALSE, ch, this, 0, TO_CHAR);
  return TRUE;
}

int TTable::getObjFrom(TBeing *ch, const char *arg1, const char *arg2)
{
  char newarg[100], capbuf[256];
  int rc;
  int p;

  if (getall(arg1, newarg)) {
    if (!get_thing_on_list_vis(ch, newarg, rider)) {
      ch->sendTo(COLOR_OBJECTS, fmt("There are no \"%s\"'s visible on %s.\n\r") % newarg % getName());
      return TRUE;
    }
    if (ch->getPosition() <= POSITION_SITTING) {
      ch->sendTo("You need to be standing to do that.\n\r");
      if (!ch->awake())
        return TRUE;   // sleeping
      ch->doStand();
 
      if (ch->fight())
        return TRUE;  // don't fall through
    }
    if (dynamic_cast<TBeing *>(ch->riding) && (in_room != ROOM_NOWHERE)) {
      act("You can't get things from $p while mounted!", 
           FALSE, ch, this, 0, TO_CHAR);
      return TRUE;
    }
    sprintf(capbuf, "%s %s", newarg, arg2);
    act("You start getting items off $p.", TRUE, ch, this, NULL, TO_CHAR);
    act("$n starts getting items off $p.", TRUE, ch, this, NULL, TO_ROOM);
    start_task(ch, ch->roomp->getStuff(), ch->roomp, TASK_GET_ALL,capbuf,350, ch->in_room,1,0, 0);
    // this is a kludge, task_get still has a tiny delay on it
    // this dumps around it and goes right to the guts
    rc = (*(tasks[TASK_GET_ALL].taskf))
        (ch, CMD_TASK_CONTINUE, "", 0, ch->roomp, 0);
    if (IS_SET_DELETE(rc, DELETE_THIS)) {
      return DELETE_VICT;
    }
    return TRUE;
  } else if ((p = getabunch(arg1, newarg))) {
    if (!get_thing_on_list_vis(ch, newarg, rider)) {
      ch->sendTo(COLOR_OBJECTS, fmt("There are no \"%s\"'s visible on %s.\n\r") % newarg % getName());
      return TRUE;
    }
    if (ch->getPosition() <= POSITION_SITTING) {
      ch->sendTo("You need to be standing to do that.\n\r");
      if (!ch->awake())
        return TRUE;   // sleeping
      ch->doStand();
 
      if (ch->fight())
        return TRUE;  // don't fall through
    }
    if (dynamic_cast<TBeing *>(ch->riding) && (in_room != ROOM_NOWHERE)) {
      act("You can't get things from $p while mounted!", 
           FALSE, ch, this, 0, TO_CHAR);
      return TRUE;
    }
    sprintf(capbuf, "%s %s", newarg, arg2);
    act("You start getting items off $p.", TRUE, ch, this, NULL, TO_CHAR);
    act("$n starts getting items off $p.", TRUE, ch, this, NULL, TO_ROOM);
    start_task(ch, ch->roomp->getStuff(), ch->roomp,TASK_GET_ALL,capbuf,350, ch->in_room,0,p+1,0);
    // this is a kludge, task_get still has a tiny delay on it
    // this dumps around it and goes right to the guts
    rc = (*(tasks[TASK_GET_ALL].taskf))
        (ch, CMD_TASK_CONTINUE, "", 0, ch->roomp, 0);
    if (IS_SET_DELETE(rc, DELETE_THIS)) {
      return DELETE_VICT;
    }
    return TRUE;
  }
  return FALSE;
}

// TRUE put ok, FALSE put failed, 2 failed and stop attempting further puts
// DELETE_THIS, DELETE_ITEM(obj), DELETE_VICT(ch)
int TThing::putSomethingInto(TBeing *ch, TThing *)
{
  act("$p can't hold other things.", FALSE, ch, this, 0, TO_CHAR);
  return 2;
}

int TObj::putSomethingIntoContainer(TBeing *ch, TOpenContainer *cont)
{
  if (isObjStat(ITEM_NODROP)) {
    act("You can't let go of $N, it must be CURSED!", 
           FALSE, ch, 0, this, TO_CHAR);
    return FALSE;
  }

  return TThing::putSomethingIntoContainer(ch, cont);
}

// DELETE_THIS = this
// DELETE_VICT = ch
// DELETE_ITEM = cont
int TThing::putSomethingIntoContainer(TBeing *ch, TOpenContainer *cont)
{
  int rc;

  if (cont->isClosed()) {
    act("$p is closed.", FALSE, ch, cont, this, TO_CHAR);
    return 2;     // stop trying to put
  }
  if (cont == this) {
    act("You attempt to fold $p into itself, but fail.", 
                       FALSE, ch, this, 0, TO_CHAR);
    return 2;
  }
  if (putMeInto(ch, cont))
    return FALSE;

  // obj-weight > this weight limit
  if (compareWeights(getWeight(), (cont->carryWeightLimit() - cont->getCarriedWeight())) == -1) {
    act("$p isn't strong enough to hold $N.", FALSE, ch, cont, this, TO_CHAR);
    return FALSE;
  } else if (getReducedVolume(this) > (cont->carryVolumeLimit() - cont->getCarriedVolume())) {
    act("$p isn't big enough to hold $N.", FALSE, ch, cont, this, TO_CHAR);
    return FALSE;
  }
  if (ch->fight())
    ch->cantHit += ch->loseRound(1 + getVolume() / 2250);

  rc = ch->checkForInsideTrap(cont);
  if (IS_SET_DELETE(rc, DELETE_ITEM | DELETE_THIS))
    return DELETE_ITEM | DELETE_VICT;
  if (IS_SET_DELETE(rc, DELETE_ITEM))
    return DELETE_ITEM;
  if (IS_SET_DELETE(rc, DELETE_THIS))
    return DELETE_VICT;
  if (rc)
    return FALSE;

  rc = ch->checkForGetTrap(this);
  if (IS_SET_DELETE(rc, DELETE_ITEM | DELETE_THIS))
    return DELETE_THIS | DELETE_VICT;
  if (IS_SET_DELETE(rc, DELETE_ITEM))
    return DELETE_THIS;
  if (IS_SET_DELETE(rc, DELETE_THIS))
    return DELETE_VICT;
  if (rc)
    return FALSE;

  rc = checkSpec(ch, CMD_OBJ_PUT_INSIDE_SOMETHING, NULL, cont);
  if (IS_SET_DELETE(rc, DELETE_ITEM)) 
    return DELETE_ITEM;  // nuke cont
    
  if (IS_SET_DELETE(rc, DELETE_THIS)) 
    return DELETE_THIS;  // nuke obj
    
  if (IS_SET_DELETE(rc, DELETE_VICT)) 
    return DELETE_VICT;  // nuke ch
    
  if (rc)
    return TRUE;

  rc = cont->checkSpec(ch, CMD_OBJ_HAVING_SOMETHING_PUT_INTO, NULL, this);
  if (IS_SET_DELETE(rc, DELETE_ITEM)) {
    return DELETE_THIS;  // nuke this
  }
  if (IS_SET_DELETE(rc, DELETE_THIS)) {
    return DELETE_ITEM;  // nuke cont
  }
  if (rc)
    return TRUE;

  if(dynamic_cast<TToothNecklace *>(cont) &&
     dynamic_cast<TObj *>(this) &&
     dynamic_cast<TObj *>(this)->objVnum()==GENERIC_TOOTH){
    act("You attach $p to $P.",
	TRUE, ch, this, cont, TO_CHAR);
    act("$n attaches $p to $P.",
	TRUE, ch, this, cont, TO_ROOM); 
    dynamic_cast<TToothNecklace *>(cont)->updateDesc();
  } else if (dynamic_cast<TKeyring *>(cont) &&
     dynamic_cast<TKey *>(this)) {
    act("You attach $p to $P.",
	TRUE, ch, this, cont, TO_CHAR);
    act("$n attaches $p to $P.",
	TRUE, ch, this, cont, TO_ROOM); 
  } else if (dynamic_cast<TQuiver *>(cont) &&
             dynamic_cast<TArrow *>(this)) {
    act("You slide $p into $P.",
        TRUE, ch, this, cont, TO_CHAR);
    act("$n slides $p into $P.",
        TRUE, ch, this, cont, TO_ROOM);
  } else { 
    switch (getMaterial()) {
      case MAT_PAPER:
      case MAT_CLOTH:
      case MAT_SILK:
      case MAT_TOUGH_CLOTH:
      case MAT_WOOL:
      case MAT_FUR:
      case MAT_HUMAN_FLESH:
      case MAT_FUR_CAT:
      case MAT_FUR_DOG:
      case MAT_FUR_RABBIT:
      case MAT_GHOSTLY:
      case MAT_DWARF_LEATHER:
      case MAT_SOFT_LEATHER:
      case MAT_FISHSCALE:
      case MAT_OGRE_HIDE:
      case MAT_LAMINATED:
	act("You fold $p into $P.",
            TRUE, ch, this, cont, TO_CHAR);
	act("$n folds $p into $P.",
            TRUE, ch, this, cont, TO_ROOM);
	break;
      case MAT_GLASS:
      case MAT_CORAL:
      case MAT_ICE:
	act("You gently place $p in $P.",
	    TRUE, ch, this, cont, TO_CHAR);
	act("$n gently places $p in $P.",
	    TRUE, ch, this, cont, TO_ROOM);
	break;
      case MAT_POWDER:
	act("You carefully pour $p into $P.",
	    TRUE, ch, this, cont, TO_CHAR);
	act("$n carefully pours $p into $P.",
	    TRUE, ch, this, cont, TO_ROOM);
	break;
      default:
	act("You pack $p into $P.",
	    TRUE, ch, this, cont, TO_CHAR);
	act("$n packs $p into $P.",
	    TRUE, ch, this, cont, TO_ROOM);
	break;
    }
  }
  --(*this);
  *cont += *this;

  rc = ch->genericItemCheck(this);
  if (IS_SET_DELETE(rc, DELETE_ITEM)) {
    return DELETE_THIS;
  }
  if (IS_SET_DELETE(rc, DELETE_THIS)) {
    return DELETE_VICT;
  }

  return TRUE;
}

int TTable::putSomethingInto(TBeing *ch, TThing *obj)
{
  int rc = obj->putSomethingIntoTable(ch, this);
  if (IS_SET_DELETE(rc, DELETE_THIS))
    return DELETE_ITEM;
  if (IS_SET_DELETE(rc, DELETE_VICT))
    return DELETE_VICT;
  if (IS_SET_DELETE(rc, DELETE_ITEM))
    return DELETE_THIS;
  return rc;
}

int TThing::putSomethingIntoTable(TBeing *ch, TTable *table)
{
  int rc;

  TObj *tobj = dynamic_cast<TObj *>(this);
  if (tobj && tobj->isObjStat(ITEM_ATTACHED)) {
    act("You attach $p to $P.", 0, ch, tobj, table, TO_CHAR);
    act("$n attaches $p to $P.", 1, ch, tobj, table, TO_ROOM);
  } else {
    act("You pile $p onto $P.", TRUE, ch, this, table, TO_CHAR);
    act("$n piles $p onto $P.", TRUE, ch, this, table, TO_ROOM);
  }
  --(*this);
  mount(table);

  rc = ch->genericItemCheck(this);
  if (IS_SET_DELETE(rc, DELETE_ITEM)) {
    return DELETE_THIS;
  }
  if (IS_SET_DELETE(rc, DELETE_THIS)) {
    return DELETE_VICT;
  }

  return TRUE;
}

int TTable::putSomethingIntoTable(TBeing *ch, TTable *table)
{
  // this would have rideable things on rideable things.  BAD!
  act("It is dangerous to put $p on $P.",
          TRUE, ch, this, table, TO_CHAR);
  return 1;
}

void TObj::putMoneyInto(TBeing *ch, int)
{
  ch->sendTo("You can't put money into that.\n\r");
}

void TSpellBag::putMoneyInto(TBeing *ch, int amount)
{
  ch->sendTo("You can't put money into that.\n\r");
}

void TToothNecklace::putMoneyInto(TBeing *ch, int amount)
{
  ch->sendTo("You can't put money into that.\n\r");
}

void TKeyring::putMoneyInto(TBeing *ch, int amount)
{
  ch->sendTo("You can't put money into that.\n\r");
}

void TQuiver::putMoneyInto(TBeing *ch, int)
{
  ch->sendTo("You can't put money into that.\n\r");
}

bool TThing::getObjFromMeCheck(TBeing *)
{
  return FALSE;
}

void TTable::getObjFromMeText(TBeing *ch, TThing *obj, getTypeT, bool)
{
  TBeing *tbt = dynamic_cast<TBeing *>(obj);
  positionTypeT new_pos = POSITION_DEAD;
  if (tbt)
    new_pos = tbt->getPosition();
  obj->dismount(new_pos);

  *ch += *obj;

  TObj *tobj = dynamic_cast<TObj *>(obj);
  if (tobj && tobj->isObjStat(ITEM_ATTACHED)) {
    act("You detach $p from $P and get it.", 0, ch, tobj, this, TO_CHAR);
    act("$n detachs $p from $P and gets it.", 1, ch, tobj, this, TO_ROOM);
  } else {
    act("You get $p off $P.", 0, ch, obj, this, TO_CHAR);
    act("$n gets $p off $P.", 1, ch, obj, this, TO_ROOM);
  }
}

void TThing::getObjFromMeText(TBeing *ch, TThing *obj, getTypeT, bool)
{
  --(*obj);
  *ch += *obj;
  act("You get $p from $P.", 0, ch, obj, this, TO_CHAR);
  act("$n gets $p from $P.", 1, ch, obj, this, TO_ROOM);
}

void TPerson::dropItemsToRoom(safeTypeT ok, dropNukeT actually_nuke)
{
  if (GetMaxLevel() <= MAX_MORT) {
    // move equipment into inventory
    wearSlotT wst;
    for (wst = MIN_WEAR; wst < MAX_WEAR; wst++) {
      TThing *t = equipment[wst];
      if (t)
        *this += *unequip(wst);
    }

    while (getStuff()) {
      TThing *i = getStuff();
      --(*i);
      TObj *tobj = dynamic_cast<TObj *>(i);
      if (tobj && tobj->isObjStat(ITEM_NEWBIE) && !tobj->getStuff() &&
          (in_room > 80) && (in_room != ROOM_DONATION)) {
        delete tobj;
        tobj = NULL;
      } else {
        // this is here to find quipment duplication problems
        // generally, caused by a bad return code causing DELETE of wrong thing
        // known cases handled OK:
        // corpse generation moves items off pre-delete
        // quit calls this with ok=true before deleting
        // idle-timeout moves player to dump, and does ok=true call
        if (!ok)
          vlogf(LOG_BUG, fmt("%s had objects going to room somehow.  Investigate immediately.  Tell Batopr!") %  getName());

        if (actually_nuke) {
          // this is mostly a handler for idle-time-out
          // we use to move them to the dump, and just simulate the drop all
          // (of inv only, with ~TBeing handing eq?)
          // this seemed hokey, so let's just make the delete occur here.

          // seeing that the item is also saved in rent and we are NOT trying
          // to alter the counters, make appropriate allowances
          if (tobj && tobj->number)
            obj_index[tobj->number].addToNumber(1);

          delete i;
          i = NULL;
        } else
          *roomp += *i;
      }
    }
  }
}

int TBeing::doDonate(const char *argument)
{
  char arg[256], newarg[256];
  int num;
  int p;
  char buf[256];
  // this next if controls if people can donate or not - dash
#if 1
  sendTo("The donate command has been disabled.\n\r");
  return FALSE;
#endif

  strcpy(arg, argument);
  if (!*arg) {
    sendTo("Donate what?\n\r");
    return FALSE;
  }

  if (getall(arg, newarg)) {
    num = -1;
    strcpy(arg, newarg);
  } else if ((p = getabunch(arg, newarg))) {
    num = p;
    strcpy(arg, newarg);
  } else
    num = 1;

  int count = 0;
  while (num != 0) {
    TThing *t_o = searchLinkedListVis(this, arg, getStuff());

    // while modeled on junk, thought it would be cooler to let people donate
    // beings they had picked up too.

    if (!t_o)
      break;

    TObj *o = dynamic_cast<TObj *>(t_o);
    if (o) {
      if (o->isObjStat(ITEM_NODROP)) {
        sendTo("You can't let go of it, it must be CURSED!\n\r");
        return FALSE;
      }
      if (o->isPersonalized()) {
        sendTo("Monogrammed items can't be donated.\n\r");
        return FALSE;
      }
      if (o->getStuff() && desc && (desc->autobits & AUTO_POUCH)) {
        sendTo("There is still stuff in there, you choose not to donate it.\n\r");
        return FALSE;
      }
    }
    TThing *t;
    for (t = t_o->getStuff(); t; t = t->nextThing) {
      TObj *tobj = dynamic_cast<TObj *>(t);
      if (!tobj)
        continue;
      if (tobj->isObjStat(ITEM_NODROP)) {
        sendTo("You can't let go of it, something inside it must be CURSED!\n\r");
        return FALSE;
      }
      if (tobj->isPersonalized()) {
        sendTo("There is a monogrammed item inside it which can't be donated.\n\r");
        return FALSE;
      }
    }

    if (o && (o->obj_flags.cost <= 0 || o->isObjStat(ITEM_NORENT))) {
      act("$p has no real long term value, so you junk it instead.",
          FALSE, this, o, NULL, TO_CHAR);
      doJunk("", o);
    } else {
      act("You donate $p.", false, this, t_o, NULL, TO_CHAR);
      act("$n donates $p.", false, this, t_o, NULL, TO_ROOM);

      logItem(t_o, CMD_DONATE);
      for (t = t_o->getStuff(); t; t = t->nextThing)
        logItem(t, CMD_DONATE);

      --(*t_o);
      thing_to_room(t_o, ROOM_DONATION);
      sprintf(buf,"A small portal appears for an instant, dropping %s in the room.\n\r",t_o->getName());
      sendToRoom(COLOR_OBJECTS,buf, ROOM_DONATION);
    }

    if (num > 0)
      num--;

    count++;
  }

  if (!count) {
    sendTo("You don't have anything like that.\n\r");
    return FALSE;
  }

  return TRUE;
}
