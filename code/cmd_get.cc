//////////////////////////////////////////////////////////////////////
//
//    SneezyMUD - All rights reserved, SneezyMUD Coding Team
//
//    "cmd_get.cc" - the get command
//
//////////////////////////////////////////////////////////////////////

#include "stdsneezy.h"
#include "games.h"
#include "obj_player_corpse.h"
#include "obj_base_corpse.h"
#include "obj_base_container.h"
#include "obj_trap.h"
#include "obj_base_weapon.h"

void TThing::getMeFrom(TBeing *ch, TThing *t)
{
}

void TPCorpse::getMeFrom(TBeing *ch, TThing *t)
{
  if (!checkOnLists()) {
//    vlogf(LOG_BUG, fmt("Something wrong with get from a corpse, corpse not set right %s (%s).") %  ch->getName() % getName());
  } else {
    if (getStuff())
      saveCorpseToFile();
    else
      removeCorpseFromList();
  }
}

int TThing::getMe(TBeing *ch, TThing *sub)
{
  if (sub)
    sub->getMeFrom(ch, this);

  return FALSE;
}

int TTrap::getMe(TBeing *ch, TThing *sub)
{
  // do baseclass stuff for recusivity
  int rc = TObj::getMe(ch, sub);
  if (rc)
    return rc;

  extraDescription *ed, *ed2, *prev;

  if (sub)
    return FALSE;

  // erase the trap setter
  for (ed = ex_description, prev = ed; ed; ed = ed2) {
    ed2 = ed->next;
    if (!strcmp(ed->keyword, TRAP_EX_DESC)) {
      if (ed == ex_description) {
        // it is the first extra desc, move all pointers to next and delete
        ex_description = ed2;
        prev = ex_description;
        delete ed;
      } else {
        // migrate prev's pointer to my next and delete me
        prev->next = ed2;
        delete ed;
      }
    } else
      prev = ed;
  }
  return FALSE;
}

// procedures related to get 
// might return DELETE_THIS for ch
// might return DELETE_ITEM for obj
// might return DELETE_VICT for sub
// returns FALSE if get failed
int get(TBeing *ch, TThing *ttt, TThing *sub, getTypeT tType, bool isFirst)
{
  int rc = 0;

  // redundant checks also done in doGet but allows code to call get() directly
  if (!ch->hasHands() && !ch->isImmortal()) {
    ch->sendTo("How do you expect to do that without any hands?!?\n\r");
    return FALSE;
  }
  TObj *obj = dynamic_cast<TObj *>(ttt);
  if (!obj) {
    if (!ch->canGet(ttt, SILENT_NO))
      return FALSE;
  }
  if (obj && obj->isObjStat(ITEM_ATTACHED)) {
    if (!ch->isImmortal()) {
      if (obj->canWear(ITEM_TAKE)) {
        if (obj->riding) { 
          ch->sendTo(COLOR_OBJECTS, fmt("%s is attached to %s and is not currently getable.\n\r") % obj->getName() % obj->riding->getName());
        } else 
          ch->sendTo(COLOR_OBJECTS, fmt("%s is attached and is not currently getable.\n\r") % obj->getName());
      } else 
        ch->sendTo(COLOR_OBJECTS, fmt("%s is attached and is not getable.\n\r") % obj->getName());
      
      return FALSE;
    }
  }

  // this is mostly here to stop auto-loot kicking in while 'zerking
  // do allow them to get a weapon if necessary (disarmed)
  if (ch->isCombatMode(ATTACK_BERSERK)) {
    if (sub || !dynamic_cast<TBaseWeapon *>(ttt)) {
      ch->sendTo(COLOR_BASIC, "<r>You are way too blood crazed at the moment to be getting stuff.<1>\n\r");
      return FALSE;
    }
  }

  if (ch->getPosition() == POSITION_RESTING) {
    ch->sendTo("You can't quite reach from here, so you sit up.\n\r");
    ch->doSit("");
  }

  // this handles tables
  if (!sub)
    sub = ttt->riding;
  else {
    if (ttt->parent && ttt->parent != sub) {
      // very bad
      vlogf(LOG_BUG, fmt("get(): obj (%s) gotten with parent (%s) and sub (%s)") % 
          ttt->getName() % ttt->parent->getName() % sub->getName());
    }
  }

  if (sub) {
    if (sub->getObjFromMeCheck(ch)) 
      return FALSE;
    
    // getting from a bag ought to cause some loss of attacks
    if (ch->fight())
      ch->cantHit += ch->loseRound(1 + ttt->getVolume() / 2250);

    rc = ch->checkForInsideTrap(sub);
    if (IS_SET_DELETE(rc, DELETE_ITEM | DELETE_THIS))
      return DELETE_VICT | DELETE_THIS;
    if (IS_SET_DELETE(rc, DELETE_ITEM))
      return DELETE_VICT;
    if (IS_SET_DELETE(rc, DELETE_THIS))
      return DELETE_THIS;
    if (rc)
      return FALSE;
  
    rc = ch->checkForGetTrap(ttt);
    if (IS_SET_DELETE(rc, DELETE_ITEM | DELETE_THIS))
      return DELETE_ITEM | DELETE_THIS;
    if (IS_SET_DELETE(rc, DELETE_ITEM))
      return DELETE_ITEM;
    if (IS_SET_DELETE(rc, DELETE_THIS))
      return DELETE_THIS;
    if (rc)
      return FALSE;
  
    sub->getObjFromMeText(ch, ttt, tType, isFirst);

    ch->logItem(ttt, CMD_GET);
    ch->aiGet(ttt);
    rc = ch->genericItemCheck(ttt);
    if (IS_SET_DELETE(rc, DELETE_ITEM | DELETE_THIS))
      return DELETE_ITEM | DELETE_THIS;
    if (IS_SET_DELETE(rc, DELETE_ITEM))
      return DELETE_ITEM;
    if (IS_SET_DELETE(rc, DELETE_THIS))
      return DELETE_THIS;

  } else {
    if (ttt->inRoom() == ROOM_NOWHERE)
      ttt->setRoom(ch->inRoom());

    rc = ch->checkForGetTrap(ttt);
    if (IS_SET_DELETE(rc, DELETE_ITEM) && IS_SET_DELETE(rc, DELETE_THIS))
      return DELETE_THIS | DELETE_ITEM;
    if (IS_SET_DELETE(rc, DELETE_ITEM))
      return DELETE_ITEM;
    if (IS_SET_DELETE(rc, DELETE_THIS))
      return DELETE_THIS;
    if (rc)
      return FALSE;

    --(*ttt);
    *ch += *ttt;
    TObj *tobj = dynamic_cast<TObj *>(ttt);
    if (tobj && tobj->isObjStat(ITEM_ATTACHED)) {
      act("You detach $p from its place and get it.", 0, ch, ttt, 0, TO_CHAR);
      act("$n detachs $p from its place and gets it.", 1, ch, ttt, 0, TO_ROOM);
    } else {
      act("You get $p.", 0, ch, ttt, 0, TO_CHAR);
      act("$n gets $p.", 1, ch, ttt, 0, TO_ROOM);
    }
    ch->logItem(ttt, CMD_GET);
    ch->aiGet(ttt);
    rc = ch->genericItemCheck(ttt);
    if (IS_SET_DELETE(rc, DELETE_ITEM) ||
        IS_SET_DELETE(rc, DELETE_THIS)) {
      return rc;
    }
  }

  rc = ttt->getMe(ch, sub);
  if (IS_SET_DELETE(rc, DELETE_THIS)) 
    return DELETE_ITEM;
  else if (IS_SET_DELETE(rc, DELETE_VICT))
    return DELETE_THIS;
  else if (rc)  // stop parsing further
    return TRUE;


  rc = ttt->checkSpec(ch, CMD_OBJ_GOTTEN, NULL, sub);
  if (IS_SET_DELETE(rc, DELETE_ITEM))
    return DELETE_VICT;  // nuke sub
  if (IS_SET_DELETE(rc, DELETE_THIS))
    return DELETE_ITEM;  // nuke ttt
  if (IS_SET_DELETE(rc, DELETE_VICT))
    return DELETE_THIS;  // nuke ch
  if (rc)
    return TRUE;

  return TRUE;
}

static bool getAllObjChecks(TBeing *ch)
{
  if (ch->getPosition() <= POSITION_SITTING) {
    ch->sendTo("You need to be standing to do that.\n\r");
    if (!ch->awake())
      return true;   // sleeping
    ch->doStand();

    if (ch->fight())
      return true;  // don't fall through
  }
  return false;
}

// might return DELETE_THIS (for traps)
int TBeing::doGet(const char *argument)
{
  char arg1[160], arg2[160], newarg[100];
  const char *tmp_desc = NULL;
  char *tptr;
  TObj *sub;
  TThing *t;
  bool found = FALSE, autoloot = FALSE;
  int rc;
  TBeing *horse = NULL;
  TObj *tmpobj = NULL;

  int p;
  getTypeT type = GETALLALL;

  if ((tptr = strstr(argument, "-autoloot")) != NULL){
    autoloot=TRUE;
    *tptr='\0';
  }

  argument_interpreter(argument, arg1, arg2);

  if (checkHearts()) {
    if (gHearts.get_pass(this, arg1))
      return FALSE;
  }
  if (checkCrazyEights()) {
    if (gEights.get(this, arg1))
      return FALSE;
  }
  if (!hasHands() && !isImmortal()) {
    sendTo("How do you expect to do that without any hands?!?\n\r");
    return FALSE;
  }
  if (!*arg1)
    type = GETNULL;

  if (*arg1 && !*arg2) {
    if (!strcasecmp(arg1, "all"))
      type = GETALL;
    else
      type = GETOBJ;
  }
  if (*arg1 && *arg2) {
    if (!strcasecmp(arg1, "all")) {
      if (!strcasecmp(arg2, "all"))
        type = GETALLALL;
      else
        type = GETALLOBJ;
    } else {
      if (!strcasecmp(arg2, "all"))
        type = GETOBJALL;
      else
        type = GETOBJOBJ;
    }
  }
  switch (type) {
    case GETNULL:
      sendTo("Get what?\n\r");
      break;
    case GETALL:
      if (!thingsInRoomVis(this, roomp)) {
        sendTo("You don't see anything to get!\n\r");
        return FALSE;
      }
      if (getPosition() <= POSITION_SITTING) {
        sendTo("You need to be standing to do that.\n\r");
        if (!awake())
          return FALSE;   // sleeping
        doStand();

        if (fight())
          return FALSE;  // don't fall through
      }
      if (riding) {
        sendTo("The things are spread around too much to get from horseback!\n\r");
        return FALSE;
      }
      sendTo("You start picking up things from the room.\n\r");
      act("$n starts picking up things from the room.",TRUE, this, 0, 0, TO_ROOM);
      start_task(this, roomp->getStuff(), roomp, TASK_GET_ALL, "", 350, in_room, 0, 0, 0);

      // this is a kludge, task_get still has a tiny delay on it
      // this dumps around it and goes right to the guts
      rc = (*(tasks[TASK_GET_ALL].taskf))
          (this, CMD_TASK_CONTINUE, "", 0, roomp, 0);
      if (IS_SET_ONLY(rc, DELETE_THIS)) 
        return DELETE_THIS;
      

      break;
    case GETOBJ:
      if (!thingsInRoomVis(this, roomp)) {
        sendTo("You don't see anything to get!\n\r");
        return FALSE;
      }
      if (getall(arg1, newarg)) {
        if (!searchLinkedListVis(this, newarg, roomp->getStuff())) {
          sendTo(fmt("There are no \"%s\"'s visible in this room.\n\r") % newarg);
          return FALSE;    
        }
        if (getPosition() <= POSITION_SITTING) {
          sendTo("You need to be standing to do that.\n\r");
          if (!awake())
            return FALSE;   // sleeping
          doStand();
  
          if (fight())
            return FALSE;  // don't fall through
        }
        if (dynamic_cast<TBeing *>(riding)) {
          sendTo("You can't get things from the room while mounted!\n\r");
          return FALSE;
        }
        sendTo("You start picking up things from the room.\n\r");
        act("$n starts picking up things from the room.",TRUE, this, 0, 0, TO_ROOM);
        start_task(this, roomp->getStuff(), roomp, TASK_GET_ALL, newarg, 350, in_room, 0, 0 ,0);
        // this is a kludge, task_get still has a tiny delay on it
        // this dumps around it and goes right to the guts
        rc = (*(tasks[TASK_GET_ALL].taskf))
            (this, CMD_TASK_CONTINUE, "", 0, roomp, 0);
        if (IS_SET_ONLY(rc, DELETE_THIS)) 
          return DELETE_THIS;

        break;
      } else if ((p = getabunch(arg1, newarg))) {
        if (!searchLinkedListVis(this, newarg, roomp->getStuff())) {
          sendTo(fmt("There are no \"%s\"'s visible in this room.\n\r") % newarg);
          return FALSE;
        }
        if (getPosition() <= POSITION_SITTING) {
          sendTo("You need to be standing to do that.\n\r");
          if (!awake())
            return FALSE;   // sleeping
          doStand();
 
          if (fight())
            return FALSE;  // don't fall through
        }
        if (dynamic_cast<TBeing *>(riding)) {
          sendTo("You can't get things from the room while mounted!\n\r");
          return FALSE;
        }
        sendTo("You start picking up things from the room.\n\r");
        act("$n starts picking up things from the room.",TRUE, this, 0, 0, TO_ROOM);
        start_task(this, roomp->getStuff(), roomp, TASK_GET_ALL, newarg, 350, in_room, 0, p + 1, 0);
        // this is a kludge, task_get still has a tiny delay on it
        // this dumps around it and goes right to the guts
        rc = (*(tasks[TASK_GET_ALL].taskf))
            (this, CMD_TASK_CONTINUE, "", 0, roomp, 0);
        if (IS_SET_ONLY(rc, DELETE_THIS)) {
          return DELETE_THIS;
        }

        break;
      }
      if ((t = searchLinkedListVis(this, arg1, roomp->getStuff()))) {
        if (canGet(t, SILENT_NO)) {
          rc = get(this,t, NULL, GETOBJ, found);
          // get all has no lag, is this needed?
          // addToWait(ONE_SECOND);
          if (IS_SET_DELETE(rc, DELETE_ITEM)) {
            delete t;
            t = NULL;
          }
          if (IS_SET_DELETE(rc, DELETE_THIS)) {
            return DELETE_THIS;
          }
          found = TRUE;
        }
      } else {
        if ((tmp_desc = roomp->ex_description->findExtraDesc(arg1)))
          sendTo(fmt("You can't get a %s.\n\r") % arg1);
        else
          sendTo(fmt("You don't see a %s here.\n\r") % arg1);
      }
      break;
    case GETALLALL:
      sendTo("You must be joking?!\n\r");
      break;
    case GETALLOBJ:
      // handle special case "get all all.corpse"
      if (is_abbrev(arg2, "all.corpse") && strlen(arg2) > 6) {
        if (getAllObjChecks(this))
          return FALSE;

        if (dynamic_cast<TBeing *>(riding)) {
          act("You can't get things from corpses while mounted!", 
               FALSE, this, NULL, 0, TO_CHAR);
          return FALSE;
        }

        TThing *t;
        for (t = roomp->getStuff(); t; t = t->nextThing) {
          TBaseCorpse *tbc = dynamic_cast<TBaseCorpse *>(t);
          // we do no name check here, since "pile dust" won't hit "corpse"
          if (tbc) {
	    sstring namebuf;
	    TThing *tt;
	    int counter=1;
	    for (tt = roomp->getStuff(); tt; tt = tt->nextThing) {
	      if(dynamic_cast<TBaseCorpse *>(tt) == tbc)
		break;
	      if(dynamic_cast<TBaseCorpse *>(tt) &&
		 !strcmp(tbc->name, tt->name)){
		++counter;
	      }
	    }
	    namebuf=fmt("all %i.%s") % counter % add_bars(tbc->name);
	    
            rc = tbc->getAllFrom(this, namebuf.c_str());
            if (IS_SET_DELETE(rc, DELETE_VICT))
              return DELETE_THIS;
          }
        }

        break;
      }

      sub = get_obj_vis_accessible(this, arg2);

      if (!sub) {
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
      
      if (!sub) {
	if(autoloot==TRUE)
	  sendTo("You do not see or have the corpse.\n\r");
	else 
	  sendTo(fmt("You do not see or have the %s.\n\r") % arg2);
        break;
      } else {
        if (getAllObjChecks(this))
          return FALSE;

        if (dynamic_cast<TBeing *>(riding) &&
             (sub->inRoom() != ROOM_NOWHERE)) {
          act("You can't get things from $p while mounted!", 
               FALSE, this, sub, 0, TO_CHAR);
          return FALSE;
        }
        rc = sub->getAllFrom(this, argument);
        if (IS_SET_DELETE(rc, DELETE_VICT))
          return DELETE_THIS;
      }
      break;
    case GETOBJALL:
      sendTo("You can't take a thing from multiple containers.\n\r");
      break;
    case GETOBJOBJ:

      sub = get_obj_vis_accessible(this, arg2);

      if (!sub) {
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
        rc = sub->getObjFrom(this, arg1, arg2);
        if (IS_SET_DELETE(rc, DELETE_VICT))
          return DELETE_THIS;
        else if (rc)
          return TRUE;
      } else {
	if(autoloot==TRUE)
	  sendTo("You do not see or have the corpse.\n\r");
	else
	  sendTo(fmt("You do not see or have the %s.\n\r") % arg2);
        break;
      }
      if ((t = searchLinkedListVis(this, arg1, sub->getStuff()))) {
        if (canGet(t, SILENT_NO)) {
          rc = get(this, t, sub, GETOBJOBJ, found);

          // none of the other gets apply wait, so justify this if restored
          // addToWait(ONE_SECOND);

          if (IS_SET_DELETE(rc, DELETE_ITEM)) {
            delete t;
            t = NULL;
          }
          if (IS_SET_DELETE(rc, DELETE_VICT)) {
            delete sub;
            sub = NULL;
          }
          if (IS_SET_DELETE(rc, DELETE_THIS)) {
            return DELETE_THIS;
          }
          found = TRUE;
        }
      } else if ((t = get_thing_on_list_vis(this, arg1, sub->rider))) {
        if (canGet(t, SILENT_NO)) {
          rc = get(this, t, sub, GETOBJOBJ, found);

          // none of the other gets apply wait, so justify this if restored
          // addToWait(ONE_SECOND);

          if (IS_SET_DELETE(rc, DELETE_ITEM)) {
            delete t;
            t = NULL;
          }
          if (IS_SET_DELETE(rc, DELETE_VICT)) {
            delete sub;
            sub = NULL;
          }
          if (IS_SET_DELETE(rc, DELETE_THIS)) {
            return DELETE_THIS;
          }
          found = TRUE;
        }
      } else {
        sendTo(COLOR_OBJECTS, fmt("%s does not contain the %s.\n\r") %
	       sstring(sub->getName()).cap() % arg1);
      }
      break;
  }
  if (found == TRUE)
    doSave(SILENT_YES);

  return FALSE;
}
