//////////////////////////////////////////////////////////////////////////
//
// SneezyMUD - All rights reserved, SneezyMUD Coding Team
//
//////////////////////////////////////////////////////////////////////////


#include "stdsneezy.h"
#include "obj_vehicle.h"

TVehicle::TVehicle() :
  TPortal(),
  dir(DIR_NONE)
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


void TVehicle::driveSpeed(TBeing *ch, string arg)
{
  const char *speed[] = {"fast", "medium", "slow", "stop"};
  int nspeeds=4;

  if(getDir() == DIR_NONE){
    ch->sendTo("You need to choose a direction to drive in first.\n\r");
    return;
  }

  for(int i=0;i<nspeeds;++i){
    if(is_abbrev(arg, speed[i])){
      if(speed[i] == "stop"){
	ch->sendTo("You stop your vehicle.\n\r");
      } else {
	ch->sendTo("You begin to drive your vehicle at a %s speed.\n\r",
		   speed[i]);
      }
      return;
    }
  }

  ch->sendTo("You can't figure out how to drive that fast.\n\r");
}

void TVehicle::driveDir(TBeing *ch, dirTypeT dir)
{
  setDir(dir);

  ch->sendTo("You direct your vehicle to the %s.\n\r", dirs[dir]);
}


void TVehicle::driveExit(TBeing *ch)
{
  ch->sendTo("You exit the vehicle.\n\r");
}

void TVehicle::driveLook(TBeing *ch)
{
  ch->sendTo("You look outside.\n\r");
}



