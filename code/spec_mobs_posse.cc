//////////////////////////////////////////////////////////////////////////
//
// SneezyMUD - All rights reserved, SneezyMUD Coding Team
//
//      "posse.cc" - Special procedures for GH posse
//
///////////////////////////////////////////////////////////////////////////

#include "stdsneezy.h"
#include "paths.h"

int grimhavenPosse(TBeing *ch, cmdTypeT cmd, const char *, TMonster *myself, TObj *)
{
  int i, rc, found=0;
  TBeing *mob, *vict=NULL;
  const int criminals[3]={180, 134, 131};
  TThing *t=NULL;
  TMonster *tmons=NULL;
  char buf[256], *tmp;
  followData *f, *n;

  enum posseeStateT {
    STATE_NONE,
    STATE_LEAVE_OFFICE,
    STATE_FIND_CRIM,
    STATE_ARREST_FAST,
    STATE_BOOK_UM,
    STATE_TO_JAIL,
    STATE_RETURN_OFFICE
  };

  class hunt_struct {
    public:
      int cur_pos;
      int cur_path;
      posseeStateT state;
      int arrest_state;
      TMonster *criminal;

      hunt_struct() :
        cur_pos(0),
        cur_path(0),
        state(STATE_NONE),
        arrest_state(0),
        criminal(NULL)
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
      vlogf(LOG_BUG, "failed memory allocation in mob proc grimhavenPosse.");
      return FALSE;
    }
    job = static_cast<hunt_struct *>(myself->act_ptr);
    job->cur_pos = 0;
    job->state = STATE_NONE;
    job->cur_path=0;
    job->arrest_state=0;
    job->criminal=NULL;

    act("$n decides to gather a posse and hunt for fugitives!",
        TRUE, myself, 0, 0, TO_ROOM);

    REMOVE_BIT(myself->specials.act, ACT_SENTINEL);

    act("Some cityguards come to his aid.",
        TRUE, myself, 0, 0, TO_ROOM);
    
    SET_BIT(myself->specials.affectedBy, AFF_GROUP);
    for (i=0;i<3;i++) {
      if (!(mob = read_mobile(MOB_CITYGUARD, VIRTUAL))) {
        vlogf(LOG_BUG, "Bad load of cityguard.");
        continue;
      }
      *myself->roomp += *mob;
      myself->addFollower(mob);
      SET_BIT(mob->specials.affectedBy, AFF_GROUP);
      REMOVE_BIT(mob->specials.act, ACT_DIURNAL);
      mob->spec=0;
    }
  }

  if (!(job = static_cast<hunt_struct *>(myself->act_ptr))) {
    vlogf(LOG_BUG, "grimhavenPosse: error, static_cast");
    return TRUE;
  }

  // allow us to abort it.
  if (myself->inRoom() == ROOM_HELL)
    return FALSE;

  // Make sure our criminal is still alive and in the same room
  if(job->criminal){
    for(t=myself->roomp->getStuff(); t; t=t->nextThing){
      if((tmons=dynamic_cast<TMonster *>(t)) && tmons==job->criminal)
	found=1;
    }
    if(!found){
      myself->doSay("Bah.  We'll have to find another one.");
      job->criminal=NULL;
      job->arrest_state=STATE_NONE;
      if(job->state==STATE_ARREST_FAST || job->state==STATE_BOOK_UM)
	job->state=STATE_FIND_CRIM;
      return TRUE;	
    }
  }
  
  // seperate switch for "speed"
  switch(job->state){
    case STATE_NONE: // nothing happens here
    case STATE_ARREST_FAST: // arresting criminal, fast
      break;
    case STATE_FIND_CRIM: // finding a criminal, go slow
    case STATE_RETURN_OFFICE: // head back to office
      if(::number(0,2))
	return FALSE;
      break;
    case STATE_LEAVE_OFFICE: // leaving barracks, go fast
    case STATE_BOOK_UM: // taking criminal in, fast
    case STATE_TO_JAIL:
      if(::number(0,1))
	return FALSE;
      break;
  }

  // now the actual actions
  switch(job->state){
    case STATE_NONE:
      job->cur_path=0;
      job->cur_pos=0;
      job->state=STATE_LEAVE_OFFICE; 
      break;
    case STATE_LEAVE_OFFICE: // get out of barracks
    case STATE_TO_JAIL: // go to jail
    case STATE_RETURN_OFFICE: // go back to office
      if (head_guard_path[job->cur_path][(job->cur_pos + 1)].direction == -1) {
	// end of path, start on the lamp boy path now
	switch(job->state){
   	  case STATE_LEAVE_OFFICE:
	    act("$n looks at the sky and takes a deep breath of fresh air.",
		TRUE, myself, 0, 0, TO_ROOM);
	    myself->doSay("Alright boys, let's find us some criminals!");
	    job->state=STATE_FIND_CRIM;
	    job->cur_pos = 9;
	    job->cur_path = 0;
	    break;
  	  case STATE_TO_JAIL:
	    if(job->criminal){
	      myself->doSay("You stay here until you can behave yourself.");
	      myself->doRelease(job->criminal->name);
	    }
	    rc = myself->goDirection(DIR_EAST);
	    if (IS_SET_DELETE(rc, DELETE_THIS))
	      return DELETE_THIS;
	    myself->doClose("cell w");
	    if(job->criminal){
	      act("$n slinks off into the shadows to serve out his term.",
		  0, job->criminal, 0, 0, TO_ROOM);
	      delete job->criminal;
	      job->criminal=NULL;
	    }
	    job->cur_path=2;
	    job->state=STATE_RETURN_OFFICE;
	    break;
  	  case STATE_RETURN_OFFICE:
	    // delete guards
	    for (f = myself->followers; f; f = n) {
	      n = f->next;
	      if((vict=f->follower)&& vict->inGroup(*myself) && !vict->fight()){
		TMonster *tmons = dynamic_cast<TMonster *>(vict);
		if (!tmons)
		  continue;
		act("$N salutes $n briskly and goes back to normal duty.",
		    TRUE, myself, 0, tmons, TO_ROOM);
		delete tmons;
	      }
	    }
	    
	    act("$n relaxes after a hard day of criminal hunting.",
		TRUE, myself, 0, 0, TO_ROOM);
	    SET_BIT(myself->specials.act, ACT_SENTINEL);
	    delete static_cast<hunt_struct *>(myself->act_ptr);
	    myself->act_ptr = NULL;
	    break;
          default:
	    break;
	}
	return TRUE;
      } else if (head_guard_path[job->cur_path][job->cur_pos].cur_room != myself->in_room){
	// not in correct room
	// check surrounding rooms, I probably fled
        dirTypeT dir;
	for (dir=MIN_DIR; dir < MAX_DIR;dir++) {
	  if (myself->canGo(dir) && 
	      myself->roomp->dir_option[dir]->to_room ==
	      head_guard_path[job->cur_path][job->cur_pos].cur_room){
	    rc = myself->goDirection(dir);
	    if (IS_SET_DELETE(rc, DELETE_THIS))
	      return DELETE_THIS;
	    return TRUE;
	  }
	}
	
	// trace along entire route and see if I can correct
	job->cur_pos = -1;
	do {
	  job->cur_pos += 1;
	  if (head_guard_path[job->cur_path][job->cur_pos].cur_room == myself->in_room)
	    return TRUE;
	} while (head_guard_path[job->cur_path][job->cur_pos].cur_room != -1);
	
        // vlogf(LOG_BUG, "grimhavenPosse: head guard got lost");
	act("$n grows weary of searching for criminals.",
	    0, myself, 0, 0, TO_ROOM);
	act("$n goes back to his office.",
	    0, myself, 0, 0, TO_ROOM);

	if (myself->riding)
	  myself->dismount(POSITION_STANDING);
	--(*myself);
	thing_to_room(myself, 274);
	SET_BIT(myself->specials.act, ACT_SENTINEL);
	delete static_cast<hunt_struct *>(myself->act_ptr);
	myself->act_ptr = NULL;
	act("$n has arrived.", 0, myself, 0, 0, TO_ROOM);
	return TRUE;
      } else if (myself->getPosition() < POSITION_STANDING) {
	myself->doStand();
      } else {
	rc=myself->goDirection(head_guard_path[job->cur_path][job->cur_pos + 1].direction);
	if (IS_SET_DELETE(rc, DELETE_THIS)) {
	  return DELETE_THIS;
	} else if(rc){
	  job->cur_pos++;

	  if(job->state==STATE_TO_JAIL && !::number(0,2) && job->criminal){
	    switch(::number(0,4)){
    	      case 0:
		act("$n struggles to get free, but $N holds him tightly.",
		    0, job->criminal, 0, myself, TO_ROOM);
		break;
	      case 1:
		act("$n looks at you pleadingly, with a panicked look in his eye.",
		    0, job->criminal, 0, 0, TO_ROOM);
		act("$n jerks him around roughly.",
		    0, myself, 0, 0, TO_ROOM);
		break;
	      case 2: 
		myself->doSay("Clear the way, guards coming through, we've got a criminal.");
		break;
              case 3:
		act("$n whispers to you, \"Please you've got to help me!  I didn't do it, it's all a big setup!\"",
		    0, job->criminal, 0, 0, TO_ROOM);
		act("$n casts a reproving look your way.",
		    0, myself, 0, 0, TO_ROOM);
		break;
	    }
	  }
	}
      }
      break;
    case STATE_FIND_CRIM: // find a criminal
    case STATE_BOOK_UM: // take criminal to cs
      if (lamp_path_pos[job->cur_path][(job->cur_pos + 1)].direction == -1) {
	if(job->state==STATE_FIND_CRIM){
	  // end of path, get a new one
	  job->cur_pos=0;
	  job->cur_path=0;
	  job->cur_path = ::number(MIN_GRIM_PATHS, MAX_GRIM_PATHS);
	} else {
	  job->cur_path=1;
	  job->state=STATE_TO_JAIL;
	}
      } else if (lamp_path_pos[job->cur_path][job->cur_pos].cur_room != myself->in_room){
	// not in correct room
	// check surrounding rooms, I probably fled
        dirTypeT dir;
	for (dir=MIN_DIR; dir < MAX_DIR; dir++) {
	  if (myself->canGo(dir) && 
	      myself->roomp->dir_option[dir]->to_room ==
	      lamp_path_pos[job->cur_path][job->cur_pos].cur_room){
	    rc = myself->goDirection(dir);
	    if (IS_SET_DELETE(rc, DELETE_THIS))
	      return DELETE_THIS;
	    return TRUE;
	  }
	}
	
	// trace along entire route and see if I can correct
	job->cur_pos = -1;
	do {
	  job->cur_pos += 1;
	  if (lamp_path_pos[job->cur_path][job->cur_pos].cur_room == myself->in_room)
	    return TRUE;
	} while (lamp_path_pos[job->cur_path][job->cur_pos].cur_room != -1);
	
        // vlogf(LOG_BUG, "grimhavenPosse: head guard got lost");

	act("$n grows weary of searching for criminals.",
	    0, myself, 0, 0, TO_ROOM);
	act("$n goes back to his office.",
	    0, myself, 0, 0, TO_ROOM);

	if (myself->riding)
	  myself->dismount(POSITION_STANDING);
	--(*myself);
	thing_to_room(myself, 274);
	SET_BIT(myself->specials.act, ACT_SENTINEL);
	delete static_cast<hunt_struct *>(myself->act_ptr);
	myself->act_ptr = NULL;
	act("$n has arrived.", 0, myself, 0, 0, TO_ROOM);
	return TRUE;
      } else if (myself->getPosition() < POSITION_STANDING) {
	myself->doStand();
      } else {
	rc=myself->goDirection(lamp_path_pos[job->cur_path][job->cur_pos + 1].direction);
	if (IS_SET_DELETE(rc, DELETE_THIS)) {
	  return DELETE_THIS;
	} else if(rc){
	  // look for a criminal, change state
	  if(job->state==STATE_FIND_CRIM && !::number(0,0)){
	    for(t=myself->roomp->getStuff(); t && job->state==STATE_FIND_CRIM;t=t->nextThing){
	      if((tmons=dynamic_cast<TMonster *>(t))){
		for(i=0;i<3;++i){
		  if(tmons->mobVnum() == criminals[i]){
		    job->criminal=tmons;
		    job->state=STATE_ARREST_FAST;
		    break;
		  }
		}
	      }
	    }
	  } else if(job->state==STATE_BOOK_UM && !::number(0,2) && job->criminal){
	    switch(::number(0,4)){
    	      case 0:
		act("$n struggles to get free, but $N holds him tightly.",
		    0, job->criminal, 0, myself, TO_ROOM);
		break;
	      case 1:
		act("$n looks at you pleadingly, with a panicked look in his eye.",
		    0, job->criminal, 0, 0, TO_ROOM);
		act("$n jerks him around roughly.",
		    0, myself, 0, 0, TO_ROOM);
		break;
	      case 2: 
		myself->doSay("Clear the way, guards coming through, we've got a criminal.");
		break;
              case 3:
		act("$n whispers to you, \"Please you've got to help me!  I didn't do it, it's all a big setup!\"",
		    0, job->criminal, 0, 0, TO_ROOM);
		act("$n casts a reproving look your way.",
		    0, myself, 0, 0, TO_ROOM);
		break;
	    }
	  }
	  job->cur_pos++;
	}
      }
      break;
    case STATE_ARREST_FAST: // arrest the criminal
      if(!job->criminal || job->criminal->roomp != myself->roomp){
	job->state=STATE_FIND_CRIM;
	job->criminal=NULL;
	job->arrest_state=0;
	return TRUE;
      }

      switch(job->arrest_state){
        case 0:
	  act("$n glares at $N intently for a moment.", 
	      0, myself, 0, job->criminal, TO_ROOM);
	  myself->doSay("You're coming with us, we've got some questions to ask you.");
	  job->arrest_state=1;
	  break;
        case 1:
	  job->criminal->doSay("Wh-what?  I didn't do anything.");
	  act("$n looks about innocently.",
	      0, job->criminal, 0, 0, TO_ROOM);
	  job->arrest_state=2;
	  break;
        case 2:
	  act("$n meets $N's cold hard eyes for a moment and then looks away.",
	      0, job->criminal, 0, myself, TO_ROOM);
	  act("$n panics and attempts to flee!",
	      0, job->criminal, 0, 0, TO_ROOM);
          act("With lightning fast reflexes, $n sticks a foot out and trips $N!",
	      0, myself, 0, job->criminal, TO_ROOM);
	  // sometimes they don't stand up fast enough...
	  //	  rc=job->criminal->crashLanding(POSITION_SITTING);
	  // if (IS_SET_DELETE(rc, DELETE_THIS))
	  //  return DELETE_THIS;
	  job->arrest_state=3;
	  break;
        case 3:
	  sprintf(buf, "That's it, %s is coming with us!", 
		  job->criminal->getName());
	  myself->doSay(buf);
	  tmp=mud_str_dup(job->criminal->name);
	  strcpy(tmp, add_bars(tmp).c_str());
	  myself->doCapture(tmp);
	  delete tmp;
	  REMOVE_BIT(job->criminal->specials.act, ACT_DIURNAL);
	  REMOVE_BIT(job->criminal->specials.act, ACT_NOCTURNAL);
	  job->state=STATE_BOOK_UM;      
	  job->arrest_state=0;
	  break;
      }
  }

  return 0;
}
