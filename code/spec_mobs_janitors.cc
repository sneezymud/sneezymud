#include "stdsneezy.h"
#include "paths.h"
#include "obj_base_corpse.h"
#include "obj_player_corpse.h"
#include "obj_pool.h"
#include "combat.h"
#include "pathfinder.h"


bool okForJanitor(TMonster *myself, TObj *obj)
{
  // only things that can be taken, and that are not pools
  if (!obj->canWear(ITEM_TAKE) && !dynamic_cast<TPool *>(obj))
    return false;
  if (!myself->canSee(obj) || (obj->in_room == ROOM_DONATION))
    return false;

  TBaseCorpse *corpse = dynamic_cast<TBaseCorpse *>(obj);
  // Don't let them try corpses in gh at all - there are other mobs for that
  if ((myself->mobVnum() == MOB_SWEEPER || myself->mobVnum() == MOB_SWEEPER2)
    && corpse)
    return false;
  // Don't let them try and get corpses that are being skinned.
  if (corpse && corpse->isCorpseFlag(CORPSE_PC_SKINNING))
    return false;
  // nor sacrificing
  if (corpse && corpse->isCorpseFlag(CORPSE_SACRIFICE))
    return false;
  if (corpse && corpse->isCorpseFlag(CORPSE_PC_BUTCHERING))
    return false;

  // Dont let them loot pcorpses with stuff in it
  TPCorpse *tmpcorpse = dynamic_cast<TPCorpse *>(obj);
  if (tmpcorpse && tmpcorpse->getStuff())
    return false;

  // the value check means corpse can be res'd
  // pc corpses can't be res'd, so can't be looted
  // also give pcs a moment to loot
  if (corpse && (corpse->getCorpseFlags() == 0) &&
      (corpse->obj_flags.decay_time <= MAX_NPC_CORPSE_TIME - 1)) {
    if (corpse->getStuff()) {
      TThing *t3, *t4;
      for (t3 = corpse->getStuff(); t3; t3 = t4) {
        t4 = t3->nextThing;
        TObj *obj2 = dynamic_cast<TObj *>(t3);
        if (!obj2)
          return false;
        if (!obj2->canWear(ITEM_TAKE))
          return false;
        if (!myself->canSee(corpse))
          return false;

        // keep this from happening for clutter-search
        if (myself->sameRoom(*corpse))
          get(myself, obj2, corpse, GETOBJOBJ, true);
      }
    }
    // if nothing in the corpse, let them get the corpse
  }
  if (corpse && corpse->getStuff())
    return false;

  // Don't let them take things with riders.
  if (corpse && corpse->getNumRiders(corpse))
    return false;

  return true;
}


static int findSomeClutter(TMonster *myself)
{
  dirTypeT dir;
  int rc;
  TPathFinder path;

  dir=path.findPath(myself->inRoom(), findClutter(myself));

  if (dir >= MIN_DIR) {
    rc = myself->goDirection(dir);
    if (IS_SET_DELETE(rc, DELETE_THIS))
      return DELETE_THIS;
    return TRUE;
  }
  // no clutter found
#if 0
  rc = myself->wanderAround();
  if (IS_SET_DELETE(rc, DELETE_THIS))
    return DELETE_THIS;
  else if (rc)
    return TRUE;

  return FALSE;
#else
  // lots of them piling up with nothing to do
    return DELETE_THIS;
#endif
}

static int findSomeClutterPrison(TMonster *myself)
{
  dirTypeT dir;
  int rc;
  TPathFinder path;

  dir=path.findPath(myself->inRoom(), findClutterPrison(myself));

  if (dir >= MIN_DIR) {
    rc = myself->goDirection(dir);
    if (IS_SET_DELETE(rc, DELETE_THIS))
      return DELETE_THIS;
    return TRUE;
  }
  // no clutter found
  rc = myself->wanderAround();
  if (IS_SET_DELETE(rc, DELETE_THIS))
    return DELETE_THIS;
  else if (rc)
    return TRUE;

  return FALSE;
}

static int findSomeClutterAmber(TMonster *myself)
{
  dirTypeT dir;
  int rc;
  TPathFinder path;

  dir=path.findPath(myself->inRoom(), findClutterAmber(myself));

  if (dir >= MIN_DIR) {
    rc = myself->goDirection(dir);
    if (IS_SET_DELETE(rc, DELETE_THIS))
      return DELETE_THIS;
    return TRUE;
  }
  // no clutter found
  rc = myself->wanderAround();
  if (IS_SET_DELETE(rc, DELETE_THIS))
    return DELETE_THIS;
  else if (rc)
    return TRUE;

  return FALSE;
}


static int findSomeClutterBrightmoon(TMonster *myself)
{
  dirTypeT dir;
  int rc;
  TPathFinder path;

  dir=path.findPath(myself->inRoom(), findClutterBrightmoon(myself));

  if (dir >= MIN_DIR) {
    rc = myself->goDirection(dir);
    if (IS_SET_DELETE(rc, DELETE_THIS))
      return DELETE_THIS;
    return TRUE;
  }
  // no clutter found
  rc = myself->wanderAround();
  if (IS_SET_DELETE(rc, DELETE_THIS))
    return DELETE_THIS;
  else if (rc)
    return TRUE;

  return FALSE;
}


int janitor(TBeing *ch, cmdTypeT cmd, const char *, TMonster *myself, TObj *)
{
  TThing *t, *t2;
  TObj *obj = NULL;
  int rc;
  char buf[256];  
  bool trashpile=false;

  if ((cmd != CMD_GENERIC_PULSE) || !ch->awake() || ch->fight())
    return FALSE;

  if (::number(0,3))
    return FALSE;

  for (t = myself->roomp->getStuff(); t; t = t2) {
    t2 = t->nextThing;

    obj = dynamic_cast<TObj *>(t);
    if (!obj)
      continue;

    if (myself->inRoom() == ROOM_DONATION)
      break;

    if(obj->objVnum()==GENERIC_TRASH_PILE && obj->getStuff())
      trashpile=true;

    if (!trashpile && !okForJanitor(myself, obj))
      continue;

    if(trashpile){
      sprintf(buf, "$n empties out $p.");
      act(buf, FALSE, myself, obj, 0, TO_ROOM);
      myself->doGet("all trash-pile");
      trashpile=false;
    } else if (dynamic_cast<TPool *>(obj)){
      sprintf(buf, "$n mops up $p.");
      act(buf, FALSE, myself, obj, 0, TO_ROOM);
      delete obj;
    } else if (dynamic_cast<TBaseCorpse *>(obj)) {
      sprintf(buf, "$n disposes of $p.");
      act(buf, FALSE, myself, obj, 0, TO_ROOM);

      myself->roomp->playsound(SOUND_BRING_DEAD, SOUND_TYPE_NOISE);

      TThing *t;
      while ((t = obj->getStuff())) {
        (*t)--;
        *myself += *t;
      }
      delete obj;
    } else if (!obj->isObjStat(ITEM_PROTOTYPE) && !obj->getNumRiders(obj)) {
      act("$n picks up some trash.", FALSE, myself, 0, 0, TO_ROOM);
      --(*obj);
      *myself += *obj; 
      if(obj->objVnum() == OBJ_PILE_OFFAL)
	delete obj;
    }
    return TRUE;
  }

  // we only get here if there is nothing in my room worth picking up

  if (myself->mobVnum() == MOB_SWEEPER || myself->mobVnum() == MOB_SWEEPER2) {
    if (myself->getStuff()) {
      rc = myself->doDonate(ROOM_DONATION);
      if (IS_SET_DELETE(rc, DELETE_THIS)) {
        return DELETE_THIS;
      }
      return TRUE;
    } else {
      rc = findSomeClutter(myself);
      if (IS_SET_DELETE(rc, DELETE_THIS)) {
        return DELETE_THIS;
      }
      return TRUE;
    }
  }

  return FALSE;
}

int prisonJanitor(TBeing *ch, cmdTypeT cmd, const char *, TMonster *myself, TObj *)
{
  TThing *t, *t2;
  TObj *obj = NULL;
  int rc;
  char buf[256];  
  int DUMP=31905;
  bool trashpile=false;

  if ((cmd != CMD_GENERIC_PULSE) || !ch->awake() || ch->fight())
    return FALSE;

  if (::number(0,3))
    return FALSE;

  for (t = myself->roomp->getStuff(); t; t = t2) {
    t2 = t->nextThing;

    obj = dynamic_cast<TObj *>(t);
    if (!obj)
      continue;

    if (myself->inRoom() == DUMP)
      break;

    if(obj->objVnum() == 26688)
      continue;

    if(obj->objVnum()==GENERIC_TRASH_PILE && obj->getStuff())
      trashpile=true;

    if (!trashpile && !okForJanitor(myself, obj))
      continue;

    if(trashpile){
      sprintf(buf, "$n empties out $p.");
      act(buf, FALSE, myself, obj, 0, TO_ROOM);
      myself->doGet("all trash-pile");
      trashpile=false;
    } else if (dynamic_cast<TPool *>(obj)){
      sprintf(buf, "$n mops up $p.");
      act(buf, FALSE, myself, obj, 0, TO_ROOM);
      delete obj;
    } else if (!obj->isObjStat(ITEM_PROTOTYPE) && !obj->getNumRiders(obj)) {
      act("$n picks up some trash.", FALSE, myself, 0, 0, TO_ROOM);
      --(*obj);
      *myself += *obj; 
      if(obj->objVnum() == OBJ_PILE_OFFAL)
	delete obj;
    }
    return TRUE;
  }

  // we only get here if there is nothing in my room worth picking up

  if (myself->getStuff()) {
    rc = myself->doDonate(DUMP);
    if (IS_SET_DELETE(rc, DELETE_THIS)) {
      return DELETE_THIS;
    }
    return TRUE;
  } else {
    rc = findSomeClutterPrison(myself);
    if (IS_SET_DELETE(rc, DELETE_THIS)) {
      return DELETE_THIS;
    }
    return TRUE;
  }

  return FALSE;
}



int amberJanitor(TBeing *ch, cmdTypeT cmd, const char *, TMonster *myself, TObj *)
{
  TThing *t, *t2;
  TObj *obj = NULL;
  int rc;
  char buf[256];  
  int DUMP=33281;
  bool trashpile=false;

  if ((cmd != CMD_GENERIC_PULSE) || !ch->awake() || ch->fight())
    return FALSE;

  if (::number(0,3))
    return FALSE;

  for (t = myself->roomp->getStuff(); t; t = t2) {
    t2 = t->nextThing;

    obj = dynamic_cast<TObj *>(t);
    if (!obj)
      continue;

    if (myself->inRoom() == DUMP)
      break;

    if(obj->objVnum()==GENERIC_TRASH_PILE && obj->getStuff())
      trashpile=true;

    if (!trashpile && !okForJanitor(myself, obj))
      continue;

    if(trashpile){
      sprintf(buf, "$n empties out $p.");
      act(buf, FALSE, myself, obj, 0, TO_ROOM);
      myself->doGet("all trash-pile");
      trashpile=false;
    } else if (dynamic_cast<TPool *>(obj)){
      sprintf(buf, "$n mops up $p.");
      act(buf, FALSE, myself, obj, 0, TO_ROOM);
      delete obj;
    } else if (!obj->isObjStat(ITEM_PROTOTYPE) && !obj->getNumRiders(obj)) {
      act("$n picks up some trash.", FALSE, myself, 0, 0, TO_ROOM);
      --(*obj);
      *myself += *obj; 
      if(obj->objVnum() == OBJ_PILE_OFFAL)
	delete obj;
    }
    return TRUE;
  }

  // we only get here if there is nothing in my room worth picking up

  if (myself->getStuff()) {
    rc = myself->doDonate(DUMP);
    if (IS_SET_DELETE(rc, DELETE_THIS)) {
      return DELETE_THIS;
    }
    return TRUE;
  } else {
    rc = findSomeClutterAmber(myself);
    if (IS_SET_DELETE(rc, DELETE_THIS)) {
      return DELETE_THIS;
    }
    return TRUE;
  }

  return FALSE;
}

int brightmoonJanitor(TBeing *ch, cmdTypeT cmd, const char *, TMonster *myself, TObj *)
{
  TThing *t, *t2;
  TObj *obj = NULL;
  int rc;
  char buf[256];  
  int DUMP=1385;
  bool trashcan=false;
  bool trashpile=false;

  if ((cmd != CMD_GENERIC_PULSE) || !ch->awake() || ch->fight())
    return FALSE;

  if (::number(0,3))
    return FALSE;

  for (t = myself->roomp->getStuff(); t; t = t2) {
    t2 = t->nextThing;

    obj = dynamic_cast<TObj *>(t);
    if (!obj)
      continue;

    if (myself->inRoom() == DUMP)
      break;

    if(obj->objVnum()==OBJ_BM_TRASHCAN && obj->getStuff())
      trashcan=true;

    if(obj->objVnum()==GENERIC_TRASH_PILE && obj->getStuff())
      trashpile=true;

    if (!trashcan && !trashpile && !okForJanitor(myself, obj))
      continue;

    if(trashpile){
      sprintf(buf, "$n empties out $p.");
      act(buf, FALSE, myself, obj, 0, TO_ROOM);
      myself->doGet("all trash-pile");
      trashpile=false;
    } else if(trashcan){
      sprintf(buf, "$n empties out $p.");
      act(buf, FALSE, myself, obj, 0, TO_ROOM);
      myself->doGet("all trashcan");
      trashcan=false;
    } else if (dynamic_cast<TPool *>(obj)){
      sprintf(buf, "$n mops up $p.");
      act(buf, FALSE, myself, obj, 0, TO_ROOM);
      delete obj;
    } else if (!obj->isObjStat(ITEM_PROTOTYPE) && !obj->getNumRiders(obj)) {
      act("$n picks up some trash.", FALSE, myself, 0, 0, TO_ROOM);
      --(*obj);
      *myself += *obj; 
      if(obj->objVnum() == OBJ_PILE_OFFAL)
	delete obj;
    }
    return TRUE;
  }

  // we only get here if there is nothing in my room worth picking up

  if (myself->getStuff()) {
    rc = myself->doDonate(DUMP);
    if (IS_SET_DELETE(rc, DELETE_THIS)) {
      return DELETE_THIS;
    }
    return TRUE;
  } else {
    rc = findSomeClutterBrightmoon(myself);
    if (IS_SET_DELETE(rc, DELETE_THIS)) {
      return DELETE_THIS;
    }
    return TRUE;
  }

  return FALSE;
}


// for use by janitors to drop stuff in donation 
// returns DELETE_THIS
int TBeing::doDonate(int room)
{
  dirTypeT dir;
  int rc;
  TPathFinder path;

  if (in_room != room) {
    if((dir=path.findPath(in_room, findRoom(room))) < 0){
      // unable to find a path 
      if (room >= 0) {
        doSay("Time for my coffee break");
        act("$n has left into the void.",0, this, 0, 0, TO_ROOM);
        --(*this);
        thing_to_room(this, room);
        act("$n comes back to work.", 0, this, 0, 0, TO_ROOM);
      }
      return FALSE;
    }
    rc = goDirection(dir);
    if (IS_SET_DELETE(rc, DELETE_THIS))
      return DELETE_THIS;
    return TRUE;
  } else {
    rc = doDrop("all" , NULL);

    // Deal with cursed items, since they can not be dropped.
    if (real_roomp(in_room))
      while (getStuff()) {
        TThing * tThing = getStuff();

        (*tThing)--;
        (*real_roomp(in_room)) += *tThing;
      }

    return TRUE;
  }
}


// purpose of this guy is to walk around from gh surplus, bm+amber dumps and
// logrus town square.  he picks up trash and the surplus and dumps and
// drops it off in the logrus town square.

//paths:

/*
CS to amber
CS to surplus
CS to brightmoon
CS to logrus
*/

const int CART_VNUM = 33270;


void dropAllCart(TMonster *myself, TObj *cart)
{
  for(int i=0;i<10 && cart->getStuff();++i){
    myself->doGet("all cart");
    myself->doDrop("all", NULL);
  }
}

void putAllCart(TMonster *myself, TObj *cart)
{
  for(int i=0;i<10 && myself->roomp->getStuff();++i){
    myself->doGet("all");
    myself->doPut("all cart");
  }
}


void moveCart(TMonster *mob, TObj *cart)
{
  --(*cart);
  *mob->roomp += *cart;
}

TObj *findCart(TMonster *mob)
{
  TThing *t = NULL;
  TObj *cart = NULL;

  for (t = mob->roomp->getStuff(); t; t = t->nextThing) {
    if (!(cart = dynamic_cast<TObj *>(t))) {
      continue;
    }
    if (obj_index[cart->getItemIndex()].virt ==  CART_VNUM)
      break;
  }
  if (!cart || obj_index[cart->getItemIndex()].virt != CART_VNUM) {
    if(!(cart = read_object(CART_VNUM, VIRTUAL))){
      vlogf(LOG_LOW, fmt("Error loading cart in spec_mobs_garbage_convoy.cc"));
      return NULL;
    }
    *mob->roomp += *cart;
  }
  return cart;
}
  


int garbageConvoy(TBeing *, cmdTypeT cmd, const char *, TMonster *myself, TObj *)
{
  int rc;
  TThing *t=NULL;
  TObj *o, *cart;
  roomDirData *exitp;
  const int DEBUG=0;

  enum hunt_stateT {
    STATE_NONE,
    STATE_TO_CS,          // gh surplus to cs
    STATE_TROLLEY_TO,     // wait for trolley, board, ride to bm, get off
    STATE_BM_DELIVERING,  // walk from bm fountain to bm dump
    STATE_BM_RETURNING,   // walk from bm dump to bm fountain
    STATE_TROLLEY_RET,    // wait for trolley, board, ride to gh, get off
    STATE_TO_GH_SURPLUS,        // cs to gh surplus
    STATE_TO_AMBER_DUMP,  // cs to amber dump
    STATE_AMBER_TO_CS,    // amber dump to cs
    STATE_TO_LOGRUS_DUMP, // cs to logrus
    STATE_LOGRUS_TO_CS,    // logrus to cs
    STATE_TO_GH_DUMP,
    STATE_DUMP_TO_CS
  };


  sstring hunt_text_stateT[] = {
    "doing nothing",
    "going to cs from gh surplus",
    "waiting for trolley to bm",
    "going to bm dump",
    "going to bm fountain",
    "waiting for trolley to gh",
    "going to gh surplus",        // cs to gh surplus
    "going to amber dump",  // cs to amber dump
    "going to cs from amber",    // amber dump to cs
    "going to logrus dump", // cs to logrus
    "going to cs from logrus",    // logrus to cs
    "going to gh dump",
    "going to cs from gh dump"
  };


  class hunt_struct {
    public:
      int cur_pos;
      int cur_path;
      hunt_stateT state;

      hunt_struct() :
        cur_pos(0),
        cur_path(0),
        state(STATE_TO_CS)
      {
      }
      ~hunt_struct()
      {
      }
  } *job;

  if (cmd == CMD_GENERIC_DESTROYED) {
    delete static_cast<hunt_struct *>(myself->act_ptr);
    myself->act_ptr = NULL;
    return FALSE;
  }

  if ((cmd != CMD_GENERIC_PULSE && cmd != CMD_GENERIC_QUICK_PULSE) 
      || !myself->awake() || myself->fight())
    return FALSE;

  cart=findCart(myself);

  // Not doing anything yet, time to start the convoy
  if (!myself->act_ptr) {
    //    if(::number(0,99))
    //      return FALSE;

    if (!(myself->act_ptr = new hunt_struct())) {
      vlogf(LOG_BUG, "failed memory allocation in mob proc garbageconvoy.");
      return FALSE;
    }

    // load cart?
  }

  if (!(job = static_cast<hunt_struct *>(myself->act_ptr))) {
    vlogf(LOG_BUG, "garbageconvoy: error, static_cast");
    return TRUE;
  }

  // allow us to abort it.
  if (myself->inRoom() == ROOM_HELL)
    return FALSE;

  // speed
  if(!DEBUG && ::number(0,2))
    return FALSE;

  if(DEBUG)
    myself->doSay(fmt("I am %s.") % hunt_text_stateT[job->state]);

  // now the actual actions
  switch(job->state){
    case STATE_NONE:
      job->cur_path=0;
      job->cur_pos=0;
      job->state=STATE_TO_CS; 
      break;
    case STATE_TO_CS:
      if(!myself->walk_path(garbage_convoy_path[job->cur_path], job->cur_pos)){
	if(!cart->getStuff()){
	  switch(::number(0,3)){
	    case 0:
	      job->cur_pos=0;
	      job->state=STATE_TROLLEY_TO;
	      break;
	    case 1:
	      job->cur_path=4;
	      job->cur_pos=0;
	      job->state=STATE_TO_AMBER_DUMP;
	      break;
	    case 2:
	      job->cur_path=3;
	      job->cur_pos=0;
	      job->state=STATE_TO_GH_SURPLUS;
	      break;
	    case 3:
	      job->cur_path=6;
	      job->cur_pos=0;
	      job->state=STATE_TO_LOGRUS_DUMP;
	      break;
	  }
	} else {
	  job->cur_path=8;
	  job->cur_pos=0;
	  job->state=STATE_TO_GH_DUMP;
	}
      } else
	moveCart(myself, cart);
      break;
    case STATE_TO_GH_DUMP:
      if(!myself->walk_path(garbage_convoy_path[job->cur_path], job->cur_pos)){
	dropAllCart(myself, cart);

	job->cur_path=9;
	job->cur_pos=0;
	job->state=STATE_DUMP_TO_CS;
      } else
	moveCart(myself, cart);
      break;
    case STATE_DUMP_TO_CS:
      if(!myself->walk_path(garbage_convoy_path[job->cur_path], job->cur_pos)){
	job->cur_path=0;
	job->cur_pos=0;
	job->state=STATE_TO_CS;
      } else
	moveCart(myself, cart);
      break;
    case STATE_TO_LOGRUS_DUMP:
      if(!myself->walk_path(garbage_convoy_path[job->cur_path], job->cur_pos)){
	putAllCart(myself, cart);

	job->cur_path=7;
	job->cur_pos=0;
	job->state=STATE_LOGRUS_TO_CS;
      } else
	moveCart(myself, cart);
      break;
    case STATE_LOGRUS_TO_CS:
      if(!myself->walk_path(garbage_convoy_path[job->cur_path], job->cur_pos)){
	job->cur_path=0;
	job->cur_pos=0;
	job->state=STATE_TO_CS;
      } else
	moveCart(myself, cart);
      break;
    case STATE_TO_AMBER_DUMP:
      if(!myself->walk_path(garbage_convoy_path[job->cur_path], job->cur_pos)){
	putAllCart(myself, cart);

	job->cur_path=5;
	job->cur_pos=0;
	job->state=STATE_AMBER_TO_CS;
      } else
	moveCart(myself, cart);      
      break;
    case STATE_AMBER_TO_CS:
      if(!myself->walk_path(garbage_convoy_path[job->cur_path], job->cur_pos)){
	job->cur_path=0;
	job->cur_pos=0;
	job->state=STATE_TO_CS;
      } else
	moveCart(myself, cart);
      break;
    case STATE_TROLLEY_TO:
      if(!myself->inRoom()==ROOM_TROLLEY){
        exitp = myself->roomp->exitDir(DIR_NORTH);

	if(exitp->to_room == 1303){
	  rc=myself->goDirection(garbage_convoy_path[job->cur_path][job->cur_pos + 1].direction);
	  if (IS_SET_DELETE(rc, DELETE_THIS)) {
	    return DELETE_THIS;
	  }
	  
	  job->cur_pos=0;
	  job->cur_path=1;
	  job->state=STATE_BM_DELIVERING;
	}
      } else {
	for(t=myself->roomp->getStuff();t;t=t->nextThing){
	  if((o=dynamic_cast<TObj *>(t)) && o->objVnum()==15344){
	    myself->doEnter("trolley", NULL);
	    break;
	  }
	}
      }
      moveCart(myself, cart);
      break;
    case STATE_BM_DELIVERING:
      if(!myself->walk_path(garbage_convoy_path[job->cur_path], job->cur_pos)){
	putAllCart(myself, cart);

	job->cur_path=2;
	job->cur_pos=0;
	job->state=STATE_BM_RETURNING;
      } else
	moveCart(myself, cart);
      break;
    case STATE_BM_RETURNING:
      if(!myself->walk_path(garbage_convoy_path[job->cur_path], job->cur_pos)){
	job->cur_path=3;
	job->cur_pos=0;
	job->state=STATE_TROLLEY_RET;
      } else
	moveCart(myself, cart);
      break;
    case STATE_TROLLEY_RET:
      if(myself->inRoom()==ROOM_TROLLEY){
        exitp = myself->roomp->exitDir(DIR_NORTH);

	if(exitp->to_room == ROOM_CS){
	  rc=myself->goDirection(DIR_NORTH);
	  if (IS_SET_DELETE(rc, DELETE_THIS)) {
	    return DELETE_THIS;
	  }
	  
	  job->cur_pos=0;
	  job->cur_path=0;
	  job->state=STATE_TO_CS;
	}
      } else {
	for(t=myself->roomp->getStuff();t;t=t->nextThing){
	  if((o=dynamic_cast<TObj *>(t)) && o->objVnum()==15344){
	    myself->doEnter("trolley", NULL);
	    break;
	  }
	}
      }
      moveCart(myself, cart);
      break;
    case STATE_TO_GH_SURPLUS:
      if(!myself->walk_path(garbage_convoy_path[job->cur_path], job->cur_pos)){
	putAllCart(myself, cart);

	job->cur_pos=0;
	job->cur_path=0;
	job->state=STATE_TO_CS;
      } else
	moveCart(myself, cart);
      break;
  }

  return 0;
}


