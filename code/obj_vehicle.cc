//////////////////////////////////////////////////////////////////////////
//
// SneezyMUD - All rights reserved, SneezyMUD Coding Team
//
//////////////////////////////////////////////////////////////////////////


#include "stdsneezy.h"
#include "obj_vehicle.h"


const char *vehicle_speed[] = {"stop", "slow", "medium", "fast"};


TVehicle::TVehicle() :
  TPortal(),
  dir(DIR_NONE),
  speed(0)
{
}

TVehicle::TVehicle(const TVehicle &a) :
  TPortal(a)
{
}

TVehicle & TVehicle::operator=(const TVehicle &a)
{
  if (this == &a) return *this;
  TPortal::operator=(a);
  return *this;
}

TVehicle::~TVehicle()
{
}


char TVehicle::getPortalNumCharges() const
{
  return -1;
}


void TVehicle::lookObj(TBeing *ch, int) const
{
  string buf;

  ssprintf(buf, "%d look", getTarget());
  ch->doAt(buf.c_str(), true);
}



void TVehicle::driveSpeed(TBeing *ch, string arg)
{
  int nspeeds=4;

  if(getDir() == DIR_NONE){
    ch->sendTo("You need to choose a direction to drive in first.\n\r");
    return;
  }

  for(int i=0;i<nspeeds;++i){
    if(is_abbrev(arg, vehicle_speed[i])){
      if(vehicle_speed[i] == "stop"){
	if(vehicle_speed[getSpeed()] == "fast"){
	  act("$n slams on the brakes, bringing $p to a skidding halt.",
	      0, ch, this, 0, TO_ROOM);
	  act("You slam on the brakes, bringing $p to a skidding halt.", 
	      0, ch, this, 0, TO_CHAR);
	} else if(vehicle_speed[getSpeed()] == "stop"){
	  act("$p is already stopped.", 0, ch, this, 0, TO_CHAR);
	} else {
	  act("$n brings $p to a stop.",
	      0, ch, this, 0, TO_ROOM);
	  act("You bring $p to a stop.",
	      0, ch, this, 0, TO_CHAR);
	}

	setSpeed(i);
      } else {
	if(i > getSpeed()){
	  act("$p begins to accelerate.", 0, ch, this, 0, TO_ROOM);
	  act("$p begins to accelerate.", 0, ch, this, 0, TO_CHAR);
	} else if(i < getSpeed()){
	  act("$p begins to decelerate.", 0, ch, this, 0, TO_ROOM);
	  act("$p begins to decelerate.", 0, ch, this, 0, TO_CHAR);
	} else {
	  act("$p is already going that speed.", 0, ch, this, 0, TO_CHAR);
	}

	setSpeed(i);
      }
      return;
    }
  }

  ch->sendTo("You can't figure out how to drive that fast.\n\r");
}

void TVehicle::driveDir(TBeing *ch, dirTypeT dir)
{
  string buf;

  setDir(dir);

  ssprintf(buf, "$n directs $p to the %s.", dirs[dir]);
  act(buf.c_str(), 0, ch, this, 0, TO_ROOM);
  ssprintf(buf, "You direct $p to the %s.", dirs[dir]);
  act(buf.c_str(), 0, ch, this, 0, TO_CHAR);
}


void TVehicle::driveExit(TBeing *ch)
{
  --(*ch);
  *roomp+=*ch;

  act("$n exits $p.", 0, ch, this, 0, TO_ROOM);
  act("You exit $p.", 0, ch, this, 0, TO_CHAR);
}

void TVehicle::driveLook(TBeing *ch, bool silent=false)
{
  string buf;

  if(!silent)
    ch->sendTo("You look outside.\n\r");

  ssprintf(buf, "%d look", in_room);
  ch->doAt(buf.c_str(), true);
}


void TVehicle::vehiclePulse(int pulse)
{
  TThing *t;
  TBeing *tb;
  TRoom *troom=roomp;
  string buf;
  char shortdescr[256];
  vector<TBeing *>tBeing(0);

  if(!troom)
    return;

  // this is where we regulate speed
  if(getSpeed()==0)
    return;
  
  if(pulse % (ONE_SECOND/getSpeed()))
    return;

  // should stop car and send message
  if(getDir() == DIR_NONE || !troom->dir_option[getDir()])
    return;

  strcpy(shortdescr, shortDescr);
  cap(shortdescr);

  // send message to people in old room here
  if(vehicle_speed[getSpeed()] == "fast"){
    sendrpf(COLOR_OBJECTS, roomp, "%s speeds off to the %s.\n\r",
	    shortdescr, dirs[getDir()]);
  } else if(vehicle_speed[getSpeed()] == "medium"){
    sendrpf(COLOR_OBJECTS, roomp, "%s rolls off to the %s.\n\r",
	    shortdescr, dirs[getDir()]);
  } else if(vehicle_speed[getSpeed()] == "slow"){
    sendrpf(COLOR_OBJECTS, roomp, "%s creeps off to the %s.\n\r",
	    shortdescr, dirs[getDir()]);
  }


  --(*this);
  thing_to_room(this, troom->dir_option[getDir()]->to_room);

  // send message to people in new room here
  if(vehicle_speed[getSpeed()] == "fast"){
    sendrpf(COLOR_OBJECTS, roomp, "%s speeds in from the %s.\n\r",
	    shortdescr, dirs[rev_dir[getDir()]]);
    ssprintf(buf, "$p speeds %s.", dirs[getDir()]);
  } else if(vehicle_speed[getSpeed()] == "medium"){
    sendrpf(COLOR_OBJECTS, roomp, "%s rolls in from the %s.\n\r",
	    shortdescr, dirs[rev_dir[getDir()]]);
    ssprintf(buf, "$p rolls %s.", dirs[getDir()]);
  } else if(vehicle_speed[getSpeed()] == "slow"){
    sendrpf(COLOR_OBJECTS, roomp, "%s creeps in from the %s.\n\r",
	    shortdescr, dirs[rev_dir[getDir()]]);
    ssprintf(buf, "$p creeps %s.", dirs[getDir()]);
  }
  
  // send message to people in vehicle
  troom=real_roomp(getTarget());


  // the doAt in driveLook() would screw up the getStuff list
  // so we "pre-cache" it
  for (t=troom->getStuff(); t; t = t->nextThing)
    if((tb=dynamic_cast<TBeing *>(t)))
      tBeing.push_back(tb);
  
  for(unsigned int tBeingIndex=0;tBeingIndex<tBeing.size();tBeingIndex++){
    act(buf.c_str(), 0, tBeing[tBeingIndex], this, 0, TO_CHAR);
    driveLook(tBeing[tBeingIndex], true);
  }
}


void TVehicle::driveStatus(TBeing *ch)
{
  string buf;

  ssprintf(buf, "$p is pointing to the %s.\n\r", dirs[getDir()]);
  act(buf.c_str(), 0, ch, this, 0, TO_CHAR);
  ssprintf(buf, "$p is traveling at a %s speed.\n\r",
	   vehicle_speed[getSpeed()]);
  act(buf.c_str(), 0, ch, this, 0, TO_CHAR);
}


