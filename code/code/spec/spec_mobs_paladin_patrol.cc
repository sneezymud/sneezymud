#include "low.h"
#include "room.h"
#include "monster.h"
#include "paths.h"
#include "pathfinder.h"
#include "obj_commodity.h"

int paladinPatrol(TBeing *ch, cmdTypeT cmd, const char *, TMonster *myself, TObj *)
{
  int i;
  TBeing *mob;
  int MOB_PALADIN_SOLDIER=1348;
  TPathFinder path;
  int GH_ROOM=650;
  int BM_ROOM=1303;
  dirTypeT dir;
  TMonster *tm;

  enum patrolStateT {
    STATE_NONE,
    STATE_TO_GH,
    STATE_TO_BM
  };

  class hunt_struct {
    public:
      patrolStateT state;

      hunt_struct() :
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

  // all this fighting really pisses us off
  // but we're servants of the public good, stay friendly!
  followData *f, *n;
  for (f = myself->followers; f; f = n) {
    n = f->next;
    if((tm=dynamic_cast<TMonster *>(f->follower))&& tm->inGroup(*myself)){
      tm->setAnger(0);
      tm->setMalice(0);
    }
  }
  tm=dynamic_cast<TMonster *>(myself);
  tm->setAnger(0);
  tm->setMalice(0);


  // look for cyclopses
  for(StuffIter it=myself->roomp->stuff.begin();it!=myself->roomp->stuff.end();++it){
    if((tm=dynamic_cast<TMonster *>(*it)) && tm->getRace()==RACE_TYTAN){
      switch(::number(0,5)){
	case 0:
	  myself->doSay("Have at them, boys!");
	  break;
	case 1:
	  myself->doSay("In the land of the blind the one-eyed man is king.");
	  myself->doSay("But we're not blind, sucker!");
	  break;
	case 2:
	  myself->doSay("For Galek!");
	  break;
	case 3:
	  myself->doSay("GET SOME!");
	  break;
	default:
	  break;
      }

      return tm->takeFirstHit(*myself);
    }
  }

  // Not doing anything yet, time to start a patrol
  if (!myself->act_ptr) {
    if(::number(0,25))
      return FALSE;

    if (!(myself->act_ptr = new hunt_struct())) {
      vlogf(LOG_BUG, "failed memory allocation in mob proc grimhavenPosse.");
      return FALSE;
    }
    job = static_cast<hunt_struct *>(myself->act_ptr);
    job->state = STATE_NONE;

    act("$n decides to gather a patrol and hunt for cyclopses!",
        TRUE, myself, 0, 0, TO_ROOM);

    act("Some paladin soldiers come to his aid.",
        TRUE, myself, 0, 0, TO_ROOM);
    
    SET_BIT(myself->specials.affectedBy, AFF_GROUP);
    for (i=0;i<3;i++) {
      if (!(mob = read_mobile(MOB_PALADIN_SOLDIER, VIRTUAL))) {
        vlogf(LOG_BUG, "Bad load of paladin soldier.");
        continue;
      }
      *myself->roomp += *mob;
      myself->addFollower(mob);
      SET_BIT(mob->specials.affectedBy, AFF_GROUP);
      mob->spec=0;
    }
  }

  if (!(job = static_cast<hunt_struct *>(myself->act_ptr))) {
    vlogf(LOG_BUG, "paladinPatrol: error, static_cast");
    return TRUE;
  }

  // allow us to abort it.
  if (myself->inRoom() == Room::HELL)
    return FALSE;

  if(::number(0,2))
    return FALSE;


  // now the actual actions
  switch(job->state){
    case STATE_NONE:
      myself->doSay("Enough loafing, let's go find some cyclopses!");
      job->state=STATE_TO_GH; 
      break;
    case STATE_TO_GH:
      if(myself->in_room!=GH_ROOM){
	switch((dir=path.findPath(myself->in_room, findRoom(GH_ROOM)))){
	  case 0: case 1: case 2: case 3: case 4: 
	  case 5: case 6: case 7: case 8: case 9:
	    myself->goDirection(dir);
	    break;
	  case -1: // lost
	  default: // portal
	    myself->doSay("Damn, I think I'm lost.");
	    if(myself->act_ptr){
	      delete static_cast<hunt_struct *>(myself->act_ptr);
	      myself->act_ptr = NULL;
	    }
	    break;
	}
      } else {
	myself->doSay("Alright boys, let's turn around and head back to Brightmoon.");
	job->state=STATE_TO_BM;
      }
      break;
    case STATE_TO_BM:
      if(myself->in_room!=BM_ROOM){
	switch((dir=path.findPath(myself->in_room, findRoom(BM_ROOM)))){
	  case 0: case 1: case 2: case 3: case 4: 
	  case 5: case 6: case 7: case 8: case 9:
	    myself->goDirection(dir);
	    break;
	  case -1: // lost
	  default: // portal
	    myself->doSay("Damn, I think I'm lost.");
	    if(myself->act_ptr){
	      delete static_cast<hunt_struct *>(myself->act_ptr);
	      myself->act_ptr = NULL;
	    }
	    break;
	}
      } else {
	followData *f, *n;
	TBeing *vict;
	
	// get rid of valuables, these accumulate
	for(StuffIter it=myself->stuff.begin();it!=myself->stuff.end();){
	  TThing *t=*(it++);
	  --(*t);
	  if(dynamic_cast<TCommodity *>(t) ||
	     t->number == Obj::GENERIC_MONEYPOUCH){
	    delete t;
	  } else {
	    *myself->roomp += *t;
	  }
	}
	myself->setMoney(0);


	for (f = myself->followers; f; f = n) {
	  n = f->next;
	  if((vict=f->follower)&& vict->inGroup(*myself) && !vict->fight()){
	    TMonster *tmons = dynamic_cast<TMonster *>(vict);
	    if (!tmons)
	      continue;
	    // get rid of valuables, these accumulate
	    for(StuffIter it=tmons->stuff.begin();it!=tmons->stuff.end();){
	      TThing *t=*(it++);
	      --(*t);
	      if(dynamic_cast<TCommodity *>(t) ||
		 t->number == Obj::GENERIC_MONEYPOUCH){
		delete t;
	      } else {
		*myself->roomp += *t;
	      }
	    }
	    tmons->setMoney(0);
	  }
	}
	job->state=STATE_NONE;
      }
      break;
  }

  return 0;
}
