#include "stdsneezy.h"
#include "obj_vehicle.h"
#include "pathfinder.h"
#include "obj_casino_chip.h"
#include "games.h"


int trolleyBoatCaptain(TBeing *, cmdTypeT cmd, const char *, TMonster *myself, TObj *)
{
  const int trolleynum=15344;
  static int timer;
  TObj *trolley=NULL;
  int *job=NULL;
  int i;
  TVehicle *vehicle=NULL;
  TPathFinder path;
  path.setUsePortals(false);
  path.setThruDoors(true);

  if(cmd != CMD_GENERIC_PULSE)
    return FALSE;

  // find the trolley
  for(TObjIter iter=object_list.begin();iter!=object_list.end();++iter){
    if((*iter)->objVnum() == trolleynum){
      trolley=*iter;
      break;
    }
  }
  if(!trolley){
    return FALSE;
  }

  if(!(vehicle=dynamic_cast<TVehicle *>(trolley))){
    vlogf(LOG_BUG, "couldn't cast trolley to vehicle!");
    return FALSE;
  }

  if(!has_key(myself, vehicle->getPortalKey())){
    return FALSE;
  }

  vehicle->unlockMe(myself);
  vehicle->openMe(myself);

  if((--timer)>0)
    return FALSE;

  // ok, let's sail

  // first, get out action pointer, which tells us which way to go
  if (!myself->act_ptr) {
    if (!(myself->act_ptr = new int)) {
     perror("failed new of fishing trolley.");
     exit(0);
    }
    job = static_cast<int *>(myself->act_ptr);
    *job=1303;
  } else {
    job = static_cast<int *>(myself->act_ptr);
  }

  if(trolley->in_room == *job){
    myself->doDrive("stop");

    if(*job==100){
      myself->doSay("Grimhaven stop, Grimhaven.  Trolley will be departing for Brightmoon shortly.");

      *job=1303;
    } else {
      myself->doSay("Passengers, we have arrived in Brightmoon.");
      myself->doSay("If you're not heading for Grimhaven, then you'd better get off now.");

      *job=100;
    }
    timer=100;

    return TRUE;
  }
  
  int j;
  for(j=0;trolley_path[j].cur_room!=trolley->in_room;++j){
    if(trolley_path[j].cur_room==-1){
      vlogf(LOG_BUG, "fishing trolley jumped the tracks!");
      return FALSE;
    }
  }

  if(*job==100){
    i=rev_dir[trolley_path[j].direction];
  } else {
    i=trolley_path[j+1].direction;
  }

  switch(::number(0,80)){
    case 0:
      myself->doSay("Those damn cyclopses better stay off the tracks!");
      break;
    case 1:
      myself->doSay("Keep your limbs inside the trolley please, unless you want to lose them.");
      break;
    case 2:
      myself->doAction("", CMD_YAWN);
      break;
    case 3:
      myself->doSay("Hold on a minute buddy, how many eyes do you have?");
      myself->doAction("", CMD_PEER);
      myself->doSay("Oh, ok.  You're fine.");
      break;
    case 4:
      myself->doEmote("hums a sea shanty.");
      break;
  }
	

  if(vehicle->getDir() != i)
    myself->doDrive(dirs[i]);

  myself->doDrive("fast");

  return TRUE;
}



int fishingBoatCaptain(TBeing *, cmdTypeT cmd, const char *, TMonster *myself, TObj *)
{
  const int cockpit=15349;
  const int boatnum=15345;
  static int timer;
  TObj *boat=NULL;
  TRoom *boatroom=real_roomp(cockpit);
  int *job=NULL;
  int i;
  TThing *tt;
  TVehicle *vehicle=NULL;
  TPathFinder path;
  path.setUsePortals(false);
  path.setThruDoors(false);

  if(cmd != CMD_GENERIC_PULSE)
    return FALSE;

  // find the boat
  for(TObjIter iter=object_list.begin();iter!=object_list.end();++iter){
    if((*iter)->objVnum() == boatnum)
      break;
  }
  if(!boat)
    return FALSE;

  if(!(vehicle=dynamic_cast<TVehicle *>(boat))){
    vlogf(LOG_BUG, "couldn't cast boat to vehicle!");
    return FALSE;
  }

  if(!has_key(myself, vehicle->getPortalKey())){
    return FALSE;
  }

  vehicle->unlockMe(myself);
  vehicle->openMe(myself);

  // wait until we have passengers before we leave the docks
  if(boat->in_room == 15150 && timer<=0 && vehicle->getSpeed()==0){
    for(tt=boatroom->getStuff();tt;tt=tt->nextThing){
      if(dynamic_cast<TPerson *>(tt))
	break;
    }
    if(!tt)
      return FALSE;
    else
      timer=50;
  }

  if(timer == 40){
    myself->doEmote("begins making preparations to leave port.");
  } else if(timer == 30){
    myself->doSay("Crew, prepare to leave port!");
  } else if(timer == 20){
    myself->doSay("Passengers, we will be sailing soon.");
    myself->doSay("Please get your luggage and companions on board.");
  } else if(timer == 10){
    myself->doSay("Last call for boarding, we will be departing shortly!");
  } else if(timer == 1){
    myself->doSay("Cast off the lines and push us away from dock!");
  }

  if((--timer)>0)
    return FALSE;

  // ok, let's sail

  // first, get out action pointer, which tells us which way to go
  if (!myself->act_ptr) {
    if (!(myself->act_ptr = new int)) {
     perror("failed new of fishing boat.");
     exit(0);
    }
    job = static_cast<int *>(myself->act_ptr);
    *job=13108;
  } else {
    job = static_cast<int *>(myself->act_ptr);
  }

  if(boat->in_room == *job){
    myself->doDrive("stop");
    myself->doSay("Crew, pull us in to dock and hold her steady.");
    myself->doSay("Passengers, feel free to stick around for another sail.");

    if(*job==15150){
      *job=13108;
    } else {
      timer=50;
      *job=15150;
    }
    return TRUE;
  }

  i=path.findPath(boat->in_room, findRoom(*job));

  if(i==DIR_NONE){
    vlogf(LOG_BUG, "fishing boat lost");
    return FALSE;
  }

  switch(::number(0,99)){
    case 0:
      myself->doEmote("whistles a sea shanty.");
      break;
    case 1:
      myself->doSay("Sailor, get over here and swab this deck!");
      break;
    case 2:
      myself->doSay("Uh oh!  I think I see a pirate sail!");
      myself->doAction("", CMD_CHORTLE);
      myself->doSay("Just joking.");
      break;
    case 3:
      myself->doSay("So how's the fishing today?");
      break;
    case 4:
      myself->doEmote("hums a sea shanty.");
      break;
  }
	

  if(vehicle->getDir() != i)
    myself->doDrive(dirs[i]);

  myself->doDrive("20");

  return TRUE;
}


int casinoElevatorOperator(TBeing *, cmdTypeT cmd, const char *, TMonster *myself, TObj *)
{
  const int elevatornum=2360;
  static int timer;
  TObj *elevator=NULL;
  int *job=NULL;
  int i;
  TVehicle *vehicle=NULL;
  TPathFinder path;
  path.setUsePortals(false);
  path.setThruDoors(false);

  if (cmd == CMD_GENERIC_DESTROYED) {
    delete static_cast<int *>(myself->act_ptr);
    myself->act_ptr = NULL;
    return FALSE;
  }

  if(cmd != CMD_GENERIC_PULSE)
    return FALSE;

  // find the elevator
  for(TObjIter iter=object_list.begin();iter!=object_list.end();++iter){
    if((*iter)->objVnum() == elevatornum){
      elevator=*iter;
      break;
    }
  }
  if(!elevator)
    return FALSE;

  if(!(vehicle=dynamic_cast<TVehicle *>(elevator))){
    vlogf(LOG_BUG, "couldn't cast elevator to vehicle!");
    return FALSE;
  }

  if(!has_key(myself, vehicle->getPortalKey())){
    return FALSE;
  }

  vehicle->unlockMe(myself);
  vehicle->openMe(myself);


  if((--timer)>0)
    return FALSE;

  // first, get out action pointer, which tells us which way to go
  if (!myself->act_ptr) {
    if (!(myself->act_ptr = new int)) {
     perror("failed new of fishing elevator.");
     exit(0);
    }
    job = static_cast<int *>(myself->act_ptr);
    *job=2362;
  } else {
    job = static_cast<int *>(myself->act_ptr);
  }

  if(elevator->in_room == *job){
    myself->doDrive("stop");

    if(*job==2352){
      timer=10;
      *job=2362;
    } else {
      timer=10;
      *job=2352;
    }
    return TRUE;
  }

  i=path.findPath(elevator->in_room, findRoom(*job));

  if(i==DIR_NONE){
    if(elevator->in_room==2374)
      i=DIR_UP;
    else if(elevator->in_room==2367)
      i=DIR_DOWN;
    else {
      vlogf(LOG_BUG, "fishing elevator lost");
      return FALSE;
    }
  }

  if(vehicle->getDir() != i)
    myself->doDrive(dirs[i]);

  myself->doDrive("fast");

  return TRUE;
}


int casinoElevatorGuard(TBeing *ch, cmdTypeT cmd, const char *, TMonster *myself, TObj *o)
{
  if(cmd != CMD_ENTER && cmd != CMD_MOB_GIVEN_ITEM)
    return false;
  

  if(cmd == CMD_ENTER){
    myself->doSay("Entering the elevator requires a 100 talen chip.");
    myself->doEmote("stretches out his hand expectantly.");
    return true;
  }

  if(!ch || !o) // something weird going on if this happens
    return false;

  if(o->objVnum() != CHIP_100){
    myself->doSay("What the hell is this?");
    myself->doDrop("", o);
    return false;
  }

  TVehicle *elevator;
  for(TThing *tt=myself->roomp->getStuff();tt;tt=tt->nextThing){
    if((elevator=dynamic_cast<TVehicle *>(tt)) && elevator->objVnum()==2360){
      myself->doSay("Thank you, enjoy your stay!");
      ch->doEnter("", elevator);
      (*o)--;
      delete o;
      return true;
    }
  }

  myself->doSay("You'll have to wait for the elevator.");
  myself->doDrop("", o);
  return false;
}
