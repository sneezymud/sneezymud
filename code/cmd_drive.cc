#include "stdsneezy.h"
#include "obj_vehicle.h"

TVehicle *findVehicle(TBeing *ch)
{
  TObj *o;
  TVehicle *v;

  for(o=object_list;o;o=o->next){
    if((v=dynamic_cast<TVehicle *>(o)) && v->getTarget() == ch->in_room)
      return v;
  }

  return NULL;
}

// drive speed <fast|medium|slow|stop>
// drive <north|south|etc>
// drive exit
// drive look
void TBeing::doDrive(string arg)
{
  string buf;
  TVehicle *vehicle;

  if(!(vehicle=findVehicle(this))){
    sendTo("You aren't inside of a vehicle.\n\r");
    return;
  }

  if(!vehicle->roomp){
    sendTo("This vehicle is located in the void!  You can't drive it now!\n\r");
    return;
  }


  arg=lower(arg);
  arg=one_argument(arg, buf);



  // drive <direction>
  // this is a little wacky, let me explain:
  // we loop through all of the directions and check for a match
  // if we have a match, THEN we check for keys and return
  // we do this so that directions can be parsed first BUT if no
  // direction is entered we can fall through to non-keys commands
  for(dirTypeT dir=DIR_NORTH;dir<MAX_DIR;dir++){
    if(is_abbrev(buf, dirs[dir]) ||
       (dirs[dir]=="northwest" && buf=="nw") ||
       (dirs[dir]=="southwest" && buf=="sw") ||
       (dirs[dir]=="northeast" && buf=="ne") ||
       (dirs[dir]=="southeast" && buf=="se")){
      if(!has_key(this, vehicle->getPortalKey())){
	sendTo("You need the keys to drive this vehicle!\n\r");
	return;
      }

      vehicle->driveDir(this, dir);
      return;
    }
  }

  // any passenger can use these commands
  if(is_abbrev(buf, "exit")){
    vehicle->driveExit(this);
    return;
  }

  if(is_abbrev(buf, "look")){
    vehicle->driveLook(this);
    return;
  }

  if(is_abbrev(buf, "status")){
    vehicle->driveStatus(this);
    return;
  }

  if(is_abbrev(buf, "close")){
    vehicle->closeMe(this);
    return;
  }

  if(is_abbrev(buf, "lock")){
    vehicle->lockMe(this);
    return;
  }
		   
  // authenticated commands
  if(is_abbrev(buf, "fast") || is_abbrev(buf, "medium") || 
     is_abbrev(buf, "slow") || is_abbrev(buf, "stop")){
    if(!has_key(this, vehicle->getPortalKey())){
      sendTo("You need the keys to drive this vehicle!\n\r");
      return;
    }

    vehicle->driveSpeed(this, buf);
    return;
  }


  sendTo("Syntax: drive <direction>\n\r");
  sendTo("        drive <fast|medium|slow|stop>\n\r");
  sendTo("        drive <exit|look|close|lock>\n\r");
}

