#include "stdsneezy.h"
#include "paths.h"

const unsigned int MT_GUARD=31762;
const unsigned int MT_CLERIC=31763;
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

  enum hunt_stateT {
    STATE_NONE,
    STATE_TO_CS,          // gh bank to cs
    STATE_TROLLEY_TO,     // wait for trolley, board, ride to bm, get off
    STATE_BM_DELIVERING,  // walk from bm fountain to bm bank
    STATE_BM_RETURNING,   // walk from bm bank to bm fountain
    STATE_TROLLEY_RET,    // wait for trolley, board, ride to gh, get off
    STATE_TO_BANK,        // cs to gh bank
    STATE_TO_AMBER_BANK,  // cs to amber bank
    STATE_AMBER_TO_CS,    // amber bank to cs
    STATE_TO_LOGRUS_BANK, // cs to logrus bank
    STATE_LOGRUS_TO_CS    // logrus bank to cs
  };

  enum whichBankT {
    BANK_BM,
    BANK_AMBER,
    BANK_LOGRUS
  };

  class hunt_struct {
    public:
      int cur_pos;
      int cur_path;
      hunt_stateT state;
      whichBankT which;

      hunt_struct() :
        cur_pos(0),
        cur_path(0),
        state(STATE_NONE),
	which((whichBankT)::number(BANK_BM, BANK_LOGRUS))
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
    if(::number(0,99))
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

  // if we hate someone, the other guards hate them too
  for (t = myself->roomp->getStuff(); t; t = t->nextThing) {
    vict = dynamic_cast<TBeing *>(t);
    if (!vict)
      continue;
    if (myself->Hates(vict, NULL)){
      for (f = myself->followers; f; f = f->next) {
	f->follower->addHated(vict);
      }
    }
  }

  // if my guards get lost, have them track to me
  for (f = myself->followers; f; f = f->next) {
    if(f->follower->roomp->number != myself->roomp->number &&
       f->follower->isAffected(AFF_GROUP)){
      f->follower->setHunting(myself);
    }
  }  


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
      if(myself->walk_path(money_train_path[job->cur_path], job->cur_pos)){
	if(job->which == BANK_BM){
	  job->cur_path=1;
	  job->cur_pos=0;
	  job->state=STATE_TROLLEY_TO;
	} else if(job->which == BANK_LOGRUS){
	  job->cur_path=6;
	  job->cur_pos=0;
	  job->state=STATE_TO_LOGRUS_BANK;
	} else {
	  job->cur_path=4;
	  job->cur_pos=0;
	  job->state=STATE_TO_AMBER_BANK;
	}	  
      }
      break;
    case STATE_TO_LOGRUS_BANK:
      if(myself->walk_path(money_train_path[job->cur_path], job->cur_pos)){
	phat_lewt(myself);

	act("$n receives several bags of valuables for delivery.",
	    TRUE, myself, 0, 0, TO_ROOM);

	job->cur_path=7;
	job->cur_pos=0;
	job->state=STATE_LOGRUS_TO_CS;
      }
      break;
    case STATE_LOGRUS_TO_CS:
      if(myself->walk_path(money_train_path[job->cur_path], job->cur_pos)){
	job->cur_path=3;
	job->cur_pos=0;
	job->state=STATE_TO_BANK;
      }
      break;
    case STATE_TO_AMBER_BANK:
      if(myself->walk_path(money_train_path[job->cur_path], job->cur_pos)){
	phat_lewt(myself);

	act("$n receives several bags of valuables for delivery.",
	    TRUE, myself, 0, 0, TO_ROOM);

	job->cur_path=5;
	job->cur_pos=0;
	job->state=STATE_AMBER_TO_CS;
      }
      break;
    case STATE_AMBER_TO_CS:
      if(myself->walk_path(money_train_path[job->cur_path], job->cur_pos)){
	job->cur_path=3;
	job->cur_pos=0;
	job->state=STATE_TO_BANK;
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
      break;
    case STATE_BM_DELIVERING:
      if(myself->walk_path(money_train_path[job->cur_path], job->cur_pos)){
	phat_lewt(myself);

	act("$n receives several bags of valuables for delivery.",
	    TRUE, myself, 0, 0, TO_ROOM);

	job->cur_path=2;
	job->cur_pos=0;
	job->state=STATE_BM_RETURNING;
      }
      break;
    case STATE_BM_RETURNING:
      if(myself->walk_path(money_train_path[job->cur_path], job->cur_pos)){
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
      if(myself->walk_path(money_train_path[job->cur_path], job->cur_pos)){
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


