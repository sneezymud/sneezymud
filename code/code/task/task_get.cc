//////////////////////////////////////////////////////////////////////////
//
//      SneezyMUD 4.0 - All rights reserved, SneezyMUD Coding Team
//      "task.cc" - All functions related to tasks that keep mobs/PCs busy
//
//////////////////////////////////////////////////////////////////////////

#include "extern.h"
#include "handler.h"
#include "being.h"
#include "obj_player_corpse.h"
#include "room.h"
#include "obj_base_container.h"
#include "obj_table.h"

// anything_getable(TBeing *, Obj *, char *) - Russ Russell c. June 1994
// anything_getable() looks to see if there is anything getable (gee)
// If sub != NULL it looks inside sub, otherwise it looks in the room
// If name exists it looks for item with name in the name, otherwise
// it looks for all items in the room or in the objects sub

bool TBeing::anythingGetable(const TObj *sub, const char *name) const
{
  TThing *o, *n;

  if (!sub) {
    for(StuffIter it=roomp->stuff.begin();it!=roomp->stuff.end();){
      o=*(it++);
      if (!dynamic_cast<TObj *>(o))
        continue;

      if (canGet(o, SILENT_YES)) {
        if (!*name || isname(name, o->name))
          return TRUE;
      }
    }
  } else {
    if (!dynamic_cast<const TTable *>(sub)) {
      for(StuffIter it=sub->stuff.begin();it!=sub->stuff.end();){
        o=*(it++);
        if (!dynamic_cast<TObj *>(o))
          continue;
        if (canGet(o, SILENT_YES))
          if (!*name || isname(name, o->name))
            return TRUE;
      }
    } else {   // tables
      for (o = sub->rider; o; o = n) {
        n = o->nextRider;
        if (canGet(o, SILENT_YES))
          if (!*name || isname(name, o->name))
            return TRUE;
      }
    }
  }
  return FALSE;
}

// task_get - Russ Russell c. June 1994
// Arg holds name of object person is getting. If name is ""
// then they are doing a "get all"
// Flags holds number of items being taken. 0 means "get all"

static void getTaskStop(TObj *sub)
{
  if (sub) {
    TPCorpse * tmpcorpse = dynamic_cast<TPCorpse *>(sub);
    if (tmpcorpse) {
      if (tmpcorpse->stuff.empty())
        tmpcorpse->removeCorpseFromList();
      else
        tmpcorpse->saveCorpseToFile();
    }
  }
}

int task_get(TBeing *ch, cmdTypeT cmd, const char *, int pulse, TRoom *rp, TObj *)
{
  TThing *t, *t2;
  TObj *o;
  sstring buf1, buf2;
  int rc;
  TObj *sub = NULL;


  if (ch->isLinkdead() || (ch->getPosition() < POSITION_RESTING) || !ch->task) {
    ch->stopTask();
    return FALSE;
  }
  if (ch->utilityTaskCommand(cmd))
    return FALSE;

  if (ch->in_room != ch->task->wasInRoom) {
    ch->sendTo("How can you expect to get things while moving around?\n\r");
    ch->stopTask();
    return FALSE;
  }
  switch (cmd) {
  case CMD_TASK_CONTINUE:
    buf1=sstring(ch->task->orig_arg).word(0);
    buf2=sstring(ch->task->orig_arg).word(1);

      if(buf2.empty()){
        if (!thingsInRoomVis(ch, rp)) {
          ch->sendTo("You don't see anything else!\n\r");
          ch->stopTask();
          return FALSE;
        }
        if (!ch->anythingGetable(NULL, "")) {
          ch->sendTo("You can get nothing more!\n\r");
          ch->stopTask();
          return FALSE;
        }
        if (!*ch->task->orig_arg && !ch->task->flags) {
          //    get all
          for(StuffIter it=rp->stuff.begin();it!=rp->stuff.end();){
            t=*(it++);;   // t2 needed since coins get destroyed
            o = dynamic_cast<TObj *>(t);
            if (!o)
              continue;

            // canGet normally handles this, but immortals grab lampposts
	    // attached items
            if (o->isObjStat(ITEM_ATTACHED)) {
              if (ch->isImmortal()) {
                ch->sendTo(COLOR_OBJECTS, format("%s : You'll have to be more specific to get this.\n\r") %
                           sstring(o->getName()).cap());
              } else {
                if (o->canWear(ITEM_TAKE)) {
                  ch->sendTo(COLOR_OBJECTS, format("%s is attached and is not currently getable.\n\r") %
                             o->getName());
                } else {
                  ch->sendTo(COLOR_OBJECTS, format("%s is attached and is not getable.\n\r") %
                             o->getName());
                }
              }
              continue;
            }

            if (!o->canWear(ITEM_TAKE)) {
              if (ch->isImmortal()) {
                ch->sendTo(COLOR_OBJECTS, format("%s : You'll have to be more specific to get this.\n\r") %
                           sstring(o->getName()).cap());
              } else {
		//  ch->sendTo(COLOR_OBJECTS, format("%s : You can't take that.\n\r") %
		//             sstring(o->getName()).cap());
              }
              continue;
            }

            rc = ch->checkForAnyTrap(o);
            if (IS_SET_ONLY(rc, DELETE_THIS)) {
              ch->stopTask();
              ch->doQueueSave();
              return DELETE_THIS;
            } else if (rc) {
              ch->stopTask();
              ch->doQueueSave();
              return FALSE;
            }
            if (!ch->canGet(o, SILENT_NO))
              continue;
            else {
              ch->task->calcNextUpdate(pulse, 1);
              rc = get(ch, t, NULL, GETNULL, true);
              if (IS_SET_DELETE(rc, DELETE_ITEM)) {
                delete t;
                t = NULL;
              }
              if (IS_SET_DELETE(rc, DELETE_THIS)) {
                ch->stopTask();
                ch->doQueueSave();
                return DELETE_THIS;
              }
              if (IS_SET_DELETE(rc, DELETE_ITEM) ||
                  !rc) {
                ch->stopTask();
                ch->doQueueSave();
                return FALSE;  // stop further checks
              }
            }
          }
        } else if (!ch->task->flags) {
          // This is something like get all.sword
          while ((t = get_thing_in_list_getable(ch, ch->task->orig_arg, rp->stuff))) {
            rc = get(ch, t, NULL, GETNULL, true);
	    if (IS_SET_DELETE(rc, DELETE_ITEM)) {
                delete t;
                t = NULL;
	    }
	    if (IS_SET_DELETE(rc, DELETE_THIS)) {
                ch->stopTask();
                ch->doQueueSave();
                return DELETE_THIS;
	    }
              if (IS_SET_DELETE(rc, DELETE_ITEM) ||
                  !rc) {
                ch->stopTask();
                ch->doQueueSave();
                return FALSE;  // stop further checks
              }
          }
          ch->stopTask();
          ch->doQueueSave();
          return FALSE;
        } else {
          // get 6*item   ?
          while ((t = get_thing_in_list_getable(ch, ch->task->orig_arg, rp->stuff))) {
            if (--(ch->task->flags) > 0) {
              rc = get(ch, t, NULL, GETNULL, true);
              if (IS_SET_DELETE(rc, DELETE_ITEM)) {
                delete t;
                t = NULL;
              }
              if (IS_SET_DELETE(rc, DELETE_THIS)) {
                ch->stopTask();
                ch->doQueueSave();
                return DELETE_THIS;
              }
              if (IS_SET_DELETE(rc, DELETE_ITEM) ||
                  !rc) {
                ch->stopTask();
                ch->doQueueSave();
                return FALSE;  // stop further checks
              }
            } else {
              ch->sendTo("You've picked up the specified number of items.\n\r");
              ch->stopTask();
              ch->doQueueSave();
              return FALSE;
            }
          }
          ch->stopTask();
          ch->doQueueSave();
          return FALSE;
        }
        // If we got here we couldn't pick anything up.
        // Or either there was nothing left to get
        ch->sendTo("There is nothing more to get here!\n\r");
        ch->stopTask();
        ch->doQueueSave();
        return TRUE;
      } else {
        // get all bag
        sub = get_obj_vis_accessible(ch, buf2.c_str());
	if (!sub) {
	  TBeing *horse;
	  TObj *tmpobj;
      	  int bits = generic_find(buf2.c_str(), FIND_CHAR_ROOM, ch, &horse, &tmpobj);
	  if (bits)
	    if (horse->isRideable() && horse->equipment[WEAR_BACK]) {
	      TBaseContainer *saddlebag = dynamic_cast<TBaseContainer *>(horse->equipment[WEAR_BACK]);
	      if (saddlebag->isSaddle())
		sub = dynamic_cast<TObj *>(saddlebag);
	    }
	}
	if (!sub) {
	  ch->sendTo(format("The %s is no longer accessible.\n\r") % buf2);
	  ch->stopTask();
	  ch->doQueueSave();
	  return FALSE;
	}
        
        if (ch->task->flags) {
          if (!ch->anythingGetable(sub, buf1.c_str())) {
            act("You can get no more from $p", TRUE, ch, sub, NULL, TO_CHAR);
            ch->stopTask();
            getTaskStop(sub);
            ch->doQueueSave();
            return FALSE;
          }
          while ((t = get_thing_in_list_getable(ch, buf1.c_str(), sub->stuff))) {
            if (--(ch->task->flags) > 0) {
              rc = get(ch, t, sub, GETOBJOBJ, true);
              if (IS_SET_DELETE(rc, DELETE_ITEM)) {
                delete t;
                t = NULL;
              }
              if (IS_SET_DELETE(rc, DELETE_VICT)) {
                delete sub;
                sub = NULL;
              }
              if (IS_SET_DELETE(rc, DELETE_THIS)) {
                ch->stopTask();
                getTaskStop(sub);
                ch->doQueueSave();
                return DELETE_THIS;
              }
              if (IS_SET_DELETE(rc, DELETE_ITEM) ||
                  IS_SET_DELETE(rc, DELETE_VICT) ||
                  !rc) {
                ch->stopTask();
                getTaskStop(sub);
                ch->doQueueSave();
                return FALSE;  // stop further checks
              }
            } else {
              ch->sendTo("You've gotten the specified number of items.\n\r");
              ch->stopTask();
              getTaskStop(sub);
              ch->doQueueSave();
              return FALSE;
            }
          }
          while ((t = get_thing_on_list_getable(ch, buf1.c_str(), sub->rider))) {
            if (--(ch->task->flags) > 0) {
              rc = get(ch, t, sub, GETOBJOBJ, true);
              if (IS_SET_DELETE(rc, DELETE_ITEM)) {
                delete t;
                t = NULL;
              }
              if (IS_SET_DELETE(rc, DELETE_VICT)) {
                delete sub;
                sub = NULL;
              }
              if (IS_SET_DELETE(rc, DELETE_THIS)) {
                ch->stopTask();
                ch->doQueueSave();
                return DELETE_THIS;
              }
              if (IS_SET_DELETE(rc, DELETE_ITEM) ||
                  IS_SET_DELETE(rc, DELETE_VICT) ||
                  !rc) {
                ch->stopTask();
                ch->doQueueSave();
                return FALSE;  // stop further checks
              }
            } else {
              ch->sendTo("You've gotten the specified number of items.\n\r");
              ch->stopTask();
              ch->doQueueSave();
              return FALSE;
            }
          }

          if (!ch->anythingGetable(sub, buf1.c_str())) {
            act("You can get no more from $p", TRUE, ch, sub, NULL, TO_CHAR);
            ch->stopTask();
            getTaskStop(sub);
            ch->doQueueSave();
            return FALSE;
          }
        } else {
          if (ch->task->status) {
            while ((t = get_thing_in_list_getable(ch, buf1.c_str(), sub->stuff))) {
              rc = get(ch, t, sub, GETOBJOBJ, true);
              if (IS_SET_DELETE(rc, DELETE_ITEM)) {
                delete t;
                t = NULL;
              }
              if (IS_SET_DELETE(rc, DELETE_VICT)) {
                delete sub;
                sub = NULL;
              }
              if (IS_SET_DELETE(rc, DELETE_THIS)) {
                ch->stopTask();
                getTaskStop(sub);
                ch->doQueueSave();
                return DELETE_THIS;
              }
              if (IS_SET_DELETE(rc, DELETE_ITEM) ||
                  IS_SET_DELETE(rc, DELETE_VICT) ||
                  !rc) {
                ch->stopTask();
                getTaskStop(sub);
                ch->doQueueSave();
                return FALSE;  // stop further checks
              }
            }

            while ((t = get_thing_on_list_getable(ch, buf1.c_str(), sub->rider))) {
              rc = get(ch, t, sub, GETOBJOBJ, true);
              if (IS_SET_DELETE(rc, DELETE_ITEM)) {
                delete t;
                t = NULL;
              }
              if (IS_SET_DELETE(rc, DELETE_VICT)) {
                delete sub;
                sub = NULL;
              }
              if (IS_SET_DELETE(rc, DELETE_THIS)) {
                ch->stopTask();
                ch->doQueueSave();
                return DELETE_THIS;
              }
              if (IS_SET_DELETE(rc, DELETE_ITEM) ||
                  IS_SET_DELETE(rc, DELETE_VICT) ||
                  !rc) {
                ch->stopTask();
                ch->doQueueSave();
                return FALSE;  // stop further checks
              }
            }
            ch->stopTask();
            getTaskStop(sub);
            ch->doQueueSave();
            return FALSE;
          } else {
            if (!ch->anythingGetable(sub, "")) {
              act("You can get nothing more from $p!", TRUE, ch, sub, NULL, TO_CHAR);
              ch->stopTask();
              getTaskStop(sub);
              ch->doQueueSave();
              return FALSE;
            }
            for(StuffIter it=sub->stuff.begin();it!=sub->stuff.end();){
              t=*(it++);
              rc = ch->checkForInsideTrap(sub);
              if (IS_SET_ONLY(rc, DELETE_THIS)) {
                ch->stopTask();
                getTaskStop(sub);
                ch->doQueueSave();
                return DELETE_THIS;
              } else if (rc) {
                ch->stopTask();
                getTaskStop(sub);
                ch->doQueueSave();
                return FALSE;
              }
              if (!ch->canGet(t, SILENT_NO))
                continue;
              else {
                if (!ch->task) // how does this happen????
                  return FALSE;
                ch->task->calcNextUpdate(pulse, 1);
                rc = get(ch, t, sub, GETOBJOBJ, true);
                if (IS_SET_DELETE(rc, DELETE_ITEM)) {
                  delete t;
                  t = NULL;
                }
                if (IS_SET_DELETE(rc, DELETE_VICT)) {
                  delete sub;
                  sub = NULL;
                }
                if (IS_SET_DELETE(rc, DELETE_THIS)) {
                  ch->stopTask();
                  getTaskStop(sub);
                  ch->doQueueSave();
                  return DELETE_THIS;
                }
                if (IS_SET_DELETE(rc, DELETE_ITEM) ||
                    IS_SET_DELETE(rc, DELETE_VICT) ||
                    !rc) {
                  ch->stopTask();
                  getTaskStop(sub);
                  ch->doQueueSave();
                  return FALSE;  // stop further checks
                }
              }
            }
            for (t = sub->rider; t; t = t2) {
              t2 = t->nextRider;
              rc = ch->checkForInsideTrap(sub);
              if (IS_SET_ONLY(rc, DELETE_THIS)) {
                ch->stopTask();
                ch->doQueueSave();
                return DELETE_THIS;
              } else if (rc) {
                ch->stopTask();
                ch->doQueueSave();
                return FALSE;
              }
              if (!ch->canGet(t, SILENT_NO))
                continue;
              else {
                ch->task->calcNextUpdate(pulse, 1);
                rc = get(ch, t, sub, GETOBJOBJ, true);
                if (IS_SET_DELETE(rc, DELETE_ITEM)) {
                  delete t;
                  t = NULL;
                }
                if (IS_SET_DELETE(rc, DELETE_VICT)) {
                  delete sub;
                  sub = NULL;
                }
                if (IS_SET_DELETE(rc, DELETE_THIS)) {
                  ch->stopTask();
                  ch->doQueueSave();
                  return DELETE_THIS;
                }
                if (IS_SET_DELETE(rc, DELETE_ITEM) ||
                    IS_SET_DELETE(rc, DELETE_VICT) ||
                    !rc) {
                  ch->stopTask();
                  ch->doQueueSave();
                  return FALSE;  // stop further checks
                }
              }
            }
            if (!ch->anythingGetable(sub, "")) {
              act("You can get nothing more from $p!", TRUE, ch, sub, NULL, TO_CHAR);
              ch->stopTask();
              getTaskStop(sub);
              ch->doQueueSave();
              return FALSE;
            }
          }
        }
      }
      break;
  case CMD_ABORT:
  case CMD_STOP:
      act("You stop getting items!", FALSE, ch, NULL, NULL, TO_CHAR);
      act("$n stops getting items!", TRUE, ch, NULL, NULL, TO_ROOM);
      ch->stopTask();
      break;
  case CMD_TASK_FIGHTING:
      ch->sendTo("You are unable to continue getting things while under attack!\n\r");
      ch->stopTask();
      break;
  default:
      if (cmd < MAX_CMD_LIST)
        warn_busy(ch);
      break;
  }
  getTaskStop(sub);
  ch->doQueueSave();
  return TRUE;
}
