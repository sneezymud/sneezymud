#include "stdsneezy.h"
#include "paths.h"

// purpose of this guy is to walk around from gh surplus, bm+amber dumps and
// logrus town square.  he picks up trash and the surplus and dumps and
// drops it off in the logrus town square.

int garbageConvoy(TBeing *, cmdTypeT cmd, const char *, TMonster *myself, TObj *)
{
  int rc;
  TThing *t=NULL;
  TObj *o;
  roomDirData *exitp;
  followData *f, *n;
  TBeing *vict;

  enum hunt_stateT {
    STATE_NONE,
    STATE_TO_CS,          // gh surplus to cs
    STATE_TROLLEY_TO,     // wait for trolley, board, ride to bm, get off
    STATE_BM_DELIVERING,  // walk from bm fountain to bm dump
    STATE_BM_RETURNING,   // walk from bm dump to bm fountain
    STATE_TROLLEY_RET,    // wait for trolley, board, ride to gh, get off
    STATE_TO_GH_DUMP,        // cs to gh surplus
    STATE_TO_AMBER_DUMP,  // cs to amber dump
    STATE_AMBER_TO_CS,    // amber dump to cs
    STATE_TO_LOGRUS_DUMP, // cs to logrus
    STATE_LOGRUS_TO_CS    // logrus to cs
  };

  enum whichDumpT {
    DUMP_BM,
    DUMP_AMBER,
    DUMP_LOGRUS,
    DUMP_GH
  };

  class hunt_struct {
    public:
      int cur_pos;
      int cur_path;
      hunt_stateT state;
      whichDumpT which;

      hunt_struct() :
        cur_pos(0),
        cur_path(0),
        state(STATE_BM_RETURNING),
	which((whichDumpT)::number(DUMP_BM, DUMP_GH))
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

  if ((cmd != CMD_GENERIC_PULSE &&
       cmd != CMD_GENERIC_QUICK_PULSE) || !myself->awake() || myself->fight())
    return FALSE;

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
  //  if(::number(0,2))
  //    return FALSE;

  // now the actual actions
  switch(job->state){
    case STATE_NONE:
      job->cur_path=0;
      job->cur_pos=0;
      job->state=STATE_TO_CS; 
      break;
    case STATE_TO_CS:
      if(myself->walk_path(garbage_convoy_path[job->cur_path], job->cur_pos)){
	if(job->which == DUMP_BM){
	  job->cur_path=1;
	  job->cur_pos=0;
	  job->state=STATE_TROLLEY_TO;
	} else if(job->which == DUMP_LOGRUS){
	  job->cur_path=6;
	  job->cur_pos=0;
	  job->state=STATE_TO_LOGRUS_DUMP;
	} else if(job->which == DUMP_AMBER){
	  job->cur_path=4;
	  job->cur_pos=0;
	  job->state=STATE_TO_AMBER_DUMP;
	} else {
	  job->cur_path=3;
	  job->cur_pos=0;
	  job->state=STATE_TO_GH_DUMP;
	}
      }
      break;
    case STATE_TO_LOGRUS_DUMP:
      if(myself->walk_path(garbage_convoy_path[job->cur_path], job->cur_pos)){
	myself->doDrop("all", NULL);

	job->cur_path=7;
	job->cur_pos=0;
	job->state=STATE_LOGRUS_TO_CS;
      }
      break;
    case STATE_LOGRUS_TO_CS:
      if(myself->walk_path(garbage_convoy_path[job->cur_path], job->cur_pos)){
	job->cur_path=3;
	job->cur_pos=0;
	job->state=STATE_TO_GH_DUMP;
      }
      break;
    case STATE_TO_AMBER_DUMP:
      if(myself->walk_path(garbage_convoy_path[job->cur_path], job->cur_pos)){

	myself->doGet("all");

	job->cur_path=5;
	job->cur_pos=0;
	job->state=STATE_AMBER_TO_CS;
      }
      break;
    case STATE_AMBER_TO_CS:
      if(myself->walk_path(garbage_convoy_path[job->cur_path], job->cur_pos)){
	job->cur_path=3;
	job->cur_pos=0;
	job->state=STATE_TO_GH_DUMP;
      }
      break;
    case STATE_TROLLEY_TO:
      if(myself->inRoom()==ROOM_TROLLEY){
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
      break;
    case STATE_BM_DELIVERING:
      if(myself->walk_path(garbage_convoy_path[job->cur_path], job->cur_pos)){
	myself->doGet("all");

	job->cur_path=2;
	job->cur_pos=0;
	job->state=STATE_BM_RETURNING;
      }
      break;
    case STATE_BM_RETURNING:
      if(myself->walk_path(garbage_convoy_path[job->cur_path], job->cur_pos)){
	job->cur_path=3;
	job->cur_pos=0;
	job->state=STATE_TROLLEY_RET;
      }
      break;
    case STATE_TROLLEY_RET:
      if(myself->inRoom()==ROOM_TROLLEY){
        exitp = myself->roomp->exitDir(DIR_NORTH);

	if(exitp->to_room == ROOM_CS){
	  rc=myself->goDirection(garbage_convoy_path[job->cur_path][job->cur_pos + 1].direction);
	  if (IS_SET_DELETE(rc, DELETE_THIS)) {
	    return DELETE_THIS;
	  }
	  
	  job->cur_pos=0;
	  job->cur_path=3;
	  job->state=STATE_TO_GH_DUMP;
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
    case STATE_TO_GH_DUMP:
      if(myself->walk_path(garbage_convoy_path[job->cur_path], job->cur_pos)){
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

	myself->doGet("all");

	job->cur_pos=0;
	job->cur_path=0;
	job->state=STATE_TO_CS;
      }
      break;
  }


  return 0;
}


