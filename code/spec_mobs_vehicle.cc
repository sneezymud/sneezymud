#include "stdsneezy.h"
#include "obj_vehicle.h"
#include "pathfinder.h"
#include "obj_casino_chip.h"
#include "games.h"
#include "database.h"

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
    if((*iter)->objVnum() == boatnum){
      boat=(*iter);
      break;
    }
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

// this function can be easily adapted for multiple use
// you just need to add a mechanism to store mobvnum, boatnum and owners
// for each use. database already stores by mobvnum.
int shipCaptain(TBeing *ch, cmdTypeT cmd, const char *arg, TMonster *myself, TObj *)
{
  TObj *boat=NULL;
  int i;
  TVehicle *vehicle=NULL;
  TPathFinder path;
  path.setUsePortals(false);
  path.setThruDoors(false);
  path.setNoMob(false);
  path.setShipOnly(true);
  sstring argument=arg;
  TDatabase db(DB_SNEEZY);
  const int boatnum=19077;
  
  if(cmd != CMD_GENERIC_PULSE &&
     !((cmd == CMD_SAY || cmd == CMD_SAY2) && 
       (argument.word(0).lower()=="captain,") && ch &&
       (ch->desc->account->name == "trav" ||
	ch->desc->account->name == "scout" ||
	ch->desc->account->name == "laren" ||
	ch->desc->account->name == "ekeron")))
    return FALSE;


  // find the boat
  for(TObjIter iter=object_list.begin();iter!=object_list.end();++iter){
    if((*iter)->objVnum() == boatnum){
      boat=(*iter);
      break;
    }
  }
  if(!boat)
    return FALSE;

  if(!(vehicle=dynamic_cast<TVehicle *>(boat))){
    vlogf(LOG_BUG, "couldn't cast boat to vehicle!");
    return FALSE;
  }

  struct sail_data {
    int room[10];
    int cur;
    bool cruise;
  } *job;


  // first, get our action pointer, which tells us which way to go
  if (!myself->act_ptr) {
    if (!(myself->act_ptr = new struct sail_data)) {
     perror("failed new of ship.");
     exit(0);
    }
    job = static_cast<struct sail_data *>(myself->act_ptr);
    for(int i=0;i<10;++i)
      job->room[i]=0;
    job->cur=-1;
    job->cruise=false;
  } else {
    job = static_cast<struct sail_data *>(myself->act_ptr);
  }


  //// commands
  if(cmd == CMD_SAY || cmd == CMD_SAY2){
    if(argument.word(1) == "destination"){
      myself->doSay(fmt("Aye aye, this 'ere be %s") % argument.word(2));
      db.query("delete from ship_destinations where vnum=%i and name='%s'",
	       myself->mobVnum(), argument.word(2).c_str());
      db.query("insert into ship_destinations (vnum, name, room) values (%i, '%s', %i)", myself->mobVnum(), argument.word(2).c_str(), vehicle->in_room);
    } else if(argument.word(1) == "forget"){
      myself->doSay(fmt("Arr like I never even heard of it!"));
      db.query("delete from ship_destinations where vnum=%i and name='%s'",
	       myself->mobVnum(), argument.word(2).c_str());
    } else if(argument.word(1) == "sail" ||
	      argument.word(1) == "cruise"){
      if(argument.word(2).empty()){
	myself->doSay("Where ye be wantin' to sail?");
	
	db.query("select name from ship_destinations where vnum=%i", myself->mobVnum());

	sstring buf;
	while(db.fetchRow()){
	  if(buf.empty())
	    buf=db["name"];
	  else
	    buf=fmt("%s, %s") % buf % db["name"];
	}

	myself->doSay(fmt("I think I knows the way to %s") % buf);
      } else {
	myself->doSay(fmt("Aye aye, settin' sail for %s") % argument.word(2));

	// parse list of destnations and add to buf in sql format
	sstring buf;
	for(int i=2;i<12;++i){
	  if(!argument.word(i).empty() && argument.word(i).isWord()){
	    if(buf.empty())
	      buf=fmt("'%s'") % argument.word(i);
	    else
	      buf=fmt("%s, '%s'") % buf % argument.word(i);
	  }
	}

	// %r should be save here because we used isWord() above
	db.query("select room from ship_destinations where vnum=%i and name in (%r)", myself->mobVnum(), buf.c_str());
	if(!db.fetchRow()){
	  myself->doSay("What the..?!  I've never 'eard of that!");
	  return TRUE;
	}

	int i=0;
	do {
	  job->room[i++] = convertTo<int>(db["room"]);
	} while(db.fetchRow());
	job->cur=0;
	if(argument.word(1)=="cruise")
	  job->cruise=true;
	else
	  job->cruise=false;
      }
    } else if(argument.word(1) == "stop"){
      myself->doSay("Sail here, sail there, stop here, for the love o' me beard make up yer mind!");
      myself->doDrive("stop");
      for(int i=0;i<10;++i)
	job->room[i]=0;
      job->cur=0;
    } else if(argument.word(1) == "take" &&
	      argument.word(2) == "five"){
      myself->doSay("Take care of 'er.");
      myself->doGive(fmt("gilt %s") % ch->getName());
      myself->doEmote("begins untangling his salt encrusted beard.");
    } else {
      myself->doSay("Arr what are ye talkin' about?");
    }
    return TRUE;
  }

  //// sailing

  if(!has_key(myself, vehicle->getPortalKey())){
    return FALSE;
  }

  vehicle->unlockMe(myself);
  vehicle->openMe(myself);

  // ok, let's sail


  // no destination
  if(!job || job->cur==-1)
    return FALSE;

  if(boat->in_room == job->room[job->cur]){
    myself->doDrive("stop");
    myself->doSay("Avast!  We have reached arrr destination.");


    if(job->cur==9 || !job->room[job->cur+1]){
      if(job->cruise)
	job->cur=0;
      else
	job->cur=-1;
    } else {
      ++job->cur;
    }

    return TRUE;
  }

  i=path.findPath(boat->in_room, findRoom(job->room[job->cur]));

  if(i==DIR_NONE){
    //    vlogf(LOG_BUG, "ship lost");
    return FALSE;
  }

  switch(::number(0,99)){
    case 0:
      myself->doSay("Bother someone else, ye drivelswigger!");
      break;
    case 1:
      myself->doSay("Bah!  Go swing the lead elsewhere, Jack Tar!");
      break;
    case 2:
      myself->doSay("Go take a caulk a few fathoms yonder, landlubber!");
      break;
    case 3:
      myself->doSay("Empty yer black jack in a rullock, picaroon!");
      break;
    case 4:
      myself->doSay("Aye, powder monkey, get back below decks where ye belong!");
      break;
    case 5:
      myself->doSay("Scuttle up the shrouds with yer duffle, scallywag!");
      break;
    case 6:
      myself->doEmote("whistles a sea shanty.");
      break;
    case 7:
      myself->doSay("Sailor, get over here and swab this deck!");
      break;
    case 8:
      myself->doEmote("hums a sea shanty.");
      break;
  }
	

  if(vehicle->getDir() != i)
    myself->doDrive(dirs[i]);

  myself->doDrive("50");

  return TRUE;
}



