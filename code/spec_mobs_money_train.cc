#include "stdsneezy.h"
#include "paths.h"

const unsigned int MT_GUARD=31762;
const unsigned int MT_CLERIC=31763;
const int ROOM_TROLLEY=15344;
const int MONEY_BAG=31776;
const unsigned int PROMISSORY_NOTE=31777;

// load up some money train phat lewt
void phat_lewt(TMonster *myself)
{
  TObj *o, *loot;

  for(int i=0;i<3;++i){
    if(!(o = read_object(MONEY_BAG, VIRTUAL))){
      vlogf(LOG_LOW, "Error money bag in money train");
    } else {
      switch(::number(0,4)){
	case 0: case 1: case 2:
	  // load some talens
	  for(int j=::number(10,25);j>=0;--j){
	    if(!(loot=read_object(::number(35,36), VIRTUAL))){
	      vlogf(LOG_LOW, "Error money bag in money train");
	    } else {
	      *o += *loot;
	    }
	  }
	  break;
	  
	case 3:
	  // load some gems
	  for(int j=::number(3,15);j>=0;--j){
	    if(!(loot=read_object(::number(515,517), VIRTUAL))){
	      vlogf(LOG_LOW, "Error money bag in money train");
	    } else {
	      *o += *loot;
	    }
	  }
	  break;
	  
	case 4:
	  // promissory notes
	  for(int j=::number(1,10);j>=0;--j){
	    if(!(loot=read_object(PROMISSORY_NOTE, VIRTUAL))){
	      vlogf(LOG_LOW, "Error money bag in money train");
	    } else {
	      *o += *loot;
	    }
	  }
	  break;
      }
      
      *myself += *o;
    }
  }
}


// returns true if reached end of path
int walk_path(TMonster *myself, const path_struct *p, int &pos)
{
  int rc;

  if (p[(pos + 1)].direction == -1){
    return TRUE;
  } else if (p[pos].cur_room != myself->in_room){
    dirTypeT dir;
    for (dir=MIN_DIR; dir < MAX_DIR;dir++) {
      if (myself->canGo(dir) && 
	  myself->roomp->dir_option[dir]->to_room ==
	  p[pos].cur_room){
	rc = myself->goDirection(dir);
	if (IS_SET_DELETE(rc, DELETE_THIS))
	  return DELETE_THIS;
	return FALSE;
      }
    }
    
    // trace along entire route and see if I can correct
    pos = -1;
    do {
      pos += 1;
      if (p[pos].cur_room == myself->in_room)
	return FALSE;
    } while (p[pos].cur_room != -1);
  } else if (myself->getPosition() < POSITION_STANDING) {
    myself->doStand();
  } else {
    rc=myself->goDirection(p[pos + 1].direction);
    if (IS_SET_DELETE(rc, DELETE_THIS)) {
      return DELETE_THIS;
    } else if(rc){
      pos++;
    }
  }
  return 0;
}


void gather_posse(TMonster *myself)
{
  int i;
  TBeing *mob;

  SET_BIT(myself->specials.affectedBy, AFF_GROUP);
  for (i=0;i<5;i++) {
    if (!(mob = read_mobile((i>2) ? MT_CLERIC : MT_GUARD, VIRTUAL))) {
      vlogf(LOG_BUG, "Bad load of money train guard.");
      continue;
    }
    *myself->roomp += *mob;
    myself->addFollower(mob);
    SET_BIT(mob->specials.affectedBy, AFF_GROUP);
    REMOVE_BIT(mob->specials.act, ACT_DIURNAL);
    mob->spec=0;
  }
}



int moneyTrain(TBeing *, cmdTypeT cmd, const char *, TMonster *myself, TObj *)
{
  int rc;
  TThing *t=NULL;
  TObj *o;
  roomDirData *exitp;
  followData *f, *n;
  TBeing *vict;


  enum bmStateT {
    STATE_NONE,
    STATE_TO_CS,
    STATE_TROLLEY_TO,
    STATE_DELIVERING,
    STATE_RETURNING,
    STATE_TROLLEY_RET,
    STATE_TO_BANK
  };


  class hunt_struct {
    public:
      int cur_pos;
      int cur_path;
      bmStateT state;

      hunt_struct() :
        cur_pos(0),
        cur_path(0),
        state(STATE_NONE)
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

  if (cmd != CMD_GENERIC_PULSE || !myself->awake() || myself->fight())
    return FALSE;

  // Not doing anything yet, time to start a posse  
  if (!myself->act_ptr) {
    if(::number(0,25))
      return FALSE;

    if (!(myself->act_ptr = new hunt_struct())) {
      vlogf(LOG_BUG, "failed memory allocation in mob proc moneyTrain.");
      return FALSE;
    }
    
    act("$n prepares to make the money train delivery.",
	TRUE, myself, 0, 0, TO_ROOM);
    
    act("A group of <Y>money train<1> guards join him.",
	TRUE, myself, 0, 0, TO_ROOM);
    
    gather_posse(myself);
  }

  if (!(job = static_cast<hunt_struct *>(myself->act_ptr))) {
    vlogf(LOG_BUG, "moneyTrain: error, static_cast");
    return TRUE;
  }

  // allow us to abort it.
  if (myself->inRoom() == ROOM_HELL)
    return FALSE;

  // speed
  if(::number(0,2))
    return FALSE;


  // now the actual actions
  switch(job->state){
    case STATE_NONE:
      job->cur_path=0;
      job->cur_pos=0;
      job->state=STATE_TO_CS; 
      break;
    case STATE_TO_CS:
      if(walk_path(myself, money_train_path[job->cur_path], job->cur_pos)){
	job->cur_path=1;
	job->cur_pos=0;
	job->state=STATE_TROLLEY_TO;
      }
      break;
    case STATE_TROLLEY_TO:
      if(myself->inRoom()==ROOM_TROLLEY){
        exitp = myself->roomp->exitDir(DIR_NORTH);

	if(exitp->to_room == 1303){
	  rc=myself->goDirection(money_train_path[job->cur_path][job->cur_pos + 1].direction);
	  if (IS_SET_DELETE(rc, DELETE_THIS)) {
	    return DELETE_THIS;
	  }
	  
	  job->cur_pos=0;
	  job->cur_path=1;
	  job->state=STATE_DELIVERING;
	}
      } else {
	for(t=myself->roomp->getStuff();t;t=t->nextThing){
	  if((o=dynamic_cast<TObj *>(t)) && o->objVnum()==15344){
	    myself->doEnter("trolley", NULL);
	    break;
	  }
	}
      }
      break;
    case STATE_DELIVERING:
      if(walk_path(myself, money_train_path[job->cur_path], job->cur_pos)){
	phat_lewt(myself);

	job->cur_path=2;
	job->cur_pos=0;
	job->state=STATE_RETURNING;
      }
      break;
    case STATE_RETURNING:
      if(walk_path(myself, money_train_path[job->cur_path], job->cur_pos)){
	job->cur_path=3;
	job->cur_pos=0;
	job->state=STATE_TROLLEY_RET;
      }
      break;
    case STATE_TROLLEY_RET:
      if(myself->inRoom()==ROOM_TROLLEY){
        exitp = myself->roomp->exitDir(DIR_NORTH);

	if(exitp->to_room == ROOM_CS){
	  rc=myself->goDirection(money_train_path[job->cur_path][job->cur_pos + 1].direction);
	  if (IS_SET_DELETE(rc, DELETE_THIS)) {
	    return DELETE_THIS;
	  }
	  
	  job->cur_pos=0;
	  job->cur_path=3;
	  job->state=STATE_TO_BANK;
	}
      } else {
	for(t=myself->roomp->getStuff();t;t=t->nextThing){
	  if((o=dynamic_cast<TObj *>(t)) && o->objVnum()==15344){
	    myself->doEnter("trolley", NULL);
	    break;
	  }
	}
      }
      break;
    case STATE_TO_BANK:
      if(walk_path(myself, money_train_path[job->cur_path], job->cur_pos)){
	for (f = myself->followers; f; f = n) {
	  n = f->next;
	  if((vict=f->follower)&& vict->inGroup(*myself) && !vict->fight()){
	    TMonster *tmons = dynamic_cast<TMonster *>(vict);
	    if (!tmons)
	      continue;
	    delete tmons;
	  }
	}
	delete static_cast<hunt_struct *>(myself->act_ptr);
	myself->act_ptr = NULL;

	act("$n hands off his delivery and dismisses his guards.",
	    TRUE, myself, 0, 0, TO_ROOM);

	TThing *tnext;
	for(t=myself->getStuff();t;t=tnext){
	  tnext=t->nextThing;
	  if((o=dynamic_cast<TObj *>(t)) && o->objVnum() == MONEY_BAG){
	    delete o;
	  }
	}

      }
      break;
  }


  return 0;
}


