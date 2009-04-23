#include "room.h"
#include "extern.h"
#include "handler.h"
#include "monster.h"
#include "account.h"
#include "obj_vehicle.h"
#include "pathfinder.h"
#include "obj_casino_chip.h"
#include "games.h"
#include "database.h"
#include "person.h"

int trolleyBoatCaptain(TBeing *, cmdTypeT cmd, const char *, TMonster *myself, TObj *)
{
  const int trolleynum=15344;
  static int timer;
  TObj *trolley=NULL;
  int *job=NULL;
  int i;
  TVehicle *vehicle=NULL;

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
  int i;
  TVehicle *vehicle=NULL;

  struct fishingData {
    int target;
    TPathFinder *path;
  } *job;

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
    TThing *tt=NULL;

    for(StuffIter it=boatroom->stuff.begin();it!=boatroom->stuff.end();++it){
      tt=*it;
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
    myself->act_ptr = new fishingData();
    job = static_cast<fishingData *>(myself->act_ptr);
    job->target=13108;
    job->path=new TPathFinder();
    job->path->setUsePortals(false);
    job->path->setThruDoors(false);
  } else {
    job = static_cast<fishingData *>(myself->act_ptr);
  }

  if(boat->in_room == job->target){
    myself->doDrive("stop");
    myself->doSay("Crew, pull us in to dock and hold her steady.");
    myself->doSay("Passengers, feel free to stick around for another sail.");

    if(job->target==15150){
      job->target=13108;
    } else {
      timer=50;
      job->target=15150;
    }
    return TRUE;
  }

  i=job->path->findPath(boat->in_room, findRoom(job->target));

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
  for(StuffIter it=myself->roomp->stuff.begin();it!=myself->roomp->stuff.end();++it){
    if((elevator=dynamic_cast<TVehicle *>(*it)) && elevator->objVnum()==2360){
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

class ship_masters {
	/* class to handle ship captains and methods to see who can control them */
public:
	std::map<int, int> captains;
	ship_masters () {
		// captain vnum, ship vnum
	  captains.insert(std::make_pair(19000, 19077)); // captain matho & the tequila sunrise
	  captains.insert(std::make_pair(15375, 15375)); // viking norseman & the viking ship of shopping
	}
	int may_control (TMonster *captain, TBeing *ersatz_master) {
		if (!ersatz_master)
			return FALSE;
		TDatabase db(DB_SNEEZY);
		db.query("select case when account_id = %i then 2 when player_id = %i then 1 else 0 end as privileges from ship_master where captain_vnum =  %i and (account_id = %i or player_id = %i)", ersatz_master->desc->account->account_id, ersatz_master->desc->playerID, captain->mobVnum(), ersatz_master->desc->account->account_id, ersatz_master->desc->playerID);
		if(!db.fetchRow()){
		  return 0;
		} else {
			return convertTo<int>(db["privileges"]);
		}
	}
};

ship_masters captains_and_masters; // singleton instance

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
  int privileges = 0; // 0 = none, 1 = delegated (minimal control), 2 = full control of captain
  std::map<int, int>::iterator ship;
  ship = captains_and_masters.captains.find(myself->mobVnum());
  if (ship == captains_and_masters.captains.end())
  	return FALSE;

  if (cmd != CMD_GENERIC_PULSE) {
  	if ((cmd == CMD_SAY || cmd == CMD_SAY2) && argument.word(0).lower() == "captain,") {
  		privileges = captains_and_masters.may_control(myself, ch);
  		if (privileges == 0)
  			return FALSE;
  	} else {
  		return FALSE;
  	}
  }

  // find the boat
  for(TObjIter iter=object_list.begin();iter!=object_list.end();++iter){
    if((*iter)->objVnum() == ship->second){
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
    sstring speed;
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
    job->speed="medium";
  } else {
    job = static_cast<struct sail_data *>(myself->act_ptr);
  }

  //// commands
  if(cmd == CMD_SAY || cmd == CMD_SAY2){
    if(!has_key(myself, vehicle->getPortalKey())){
    	myself->doSay("If I were 'captain' I'd have control of the ship, wouldn't I?");
      return TRUE;
    }
    if (argument.word(1) == "destination"){
    	if (argument.word(2).empty()) {
      	// if no destination supplied, list out known destinations
      	db.query("select d1.name, r1.name as aka from ship_destinations d1 left join room r1 on r1.vnum = d1.room where d1.vnum = %i", myself->mobVnum());
				if(!db.fetchRow()){
					myself->doSay("We've no known destinations!");
					return TRUE;
				}
				do {
					myself->doSay(format("I know the way to <W>%s<1> - a/k/a %s.") % db["name"] % db["aka"]);
				} while(db.fetchRow());
    	} else if (privileges == 2) {
				// save a new destination
				myself->doSay(format("Aye aye, this 'ere be <W>%s<1>.") % argument.word(2));
				db.query("delete from ship_destinations where vnum=%i and name='%s'", myself->mobVnum(), argument.word(2).c_str());
				db.query("insert into ship_destinations (vnum, name, room) values (%i, '%s', %i)", myself->mobVnum(), argument.word(2).c_str(), vehicle->in_room);
    	} else {
    		myself->doSay("I shall not alter the charts for the likes of ye, missy.");
    	}
    } else if (argument.word(1) == "forget" && !argument.word(2).empty()){
    	// forget a destination
      if (privileges == 2) {
	myself->doSay("Arr like I never even heard of it!");
	db.query("delete from ship_destinations where vnum=%i and name='%s'", myself->mobVnum(), argument.word(2).c_str());
    } else {
      myself->doSay("I shall not alter the charts for the likes of ye, missy.");
    }
  } else if (argument.word(1) == "sail" || argument.word(1) == "cruise") {
    	// make for a destination
      if (argument.word(2).empty()) {
    		// what is our current destination?
  			if (!job || job->cur == -1) {
  				myself->doSay("We 'ave no destination!");
  				return TRUE;
  			}
      	db.query("select d1.name, r1.name as aka from ship_destinations d1 left join room r1 on r1.vnum = d1.room where d1.vnum = %i and d1.room = %i", myself->mobVnum(), job->room[job->cur]);
      	sstring buf = "a";
      	if (job->speed == "fast" || job->speed == "slow") {
      		buf = format("a %s") % job->speed;
      	}
  			while (db.fetchRow()) {
  				myself->doSay(format("We arr on %s course for <W>%s<1> - a/k/a %s.") %buf % db["name"] % db["aka"]);
  				return TRUE;
  			}
      } else {
      	myself->doSay(format("Aye aye, settin' sail for <W>%s<1>.") % argument.word(2));
      	// parse list of destnations and add to buf in sql format
	sstring buf;
	for(int i=2;i<12;++i){
	  if (!argument.word(i).empty()) {
	    // use %q here to escape input because query() won't check with %r
	    if (buf.empty())
	      buf = format("'%s'") % argument.word(i).escape(sstring::SQL);
	    else
	      buf = format("%s, '%s'") % buf % argument.word(i).escape(sstring::SQL);
	  }
	}
	if (!buf.empty()) {
	  // %r should be safe here because we escaped in fmt above
	  db.query("select room from ship_destinations where vnum = %i and name in (%r)", myself->mobVnum(), buf.c_str());
	  if(!db.fetchRow()){
	    myself->doSay("What the...?!  I've never 'eard of that!");
	    return TRUE;
	  }
	  // first clear out existing route
	  for(int i=0;i<10;++i)
	    job->room[i]=0;
	  
	  int i=0;
	  do {
	    job->room[i++] = convertTo<int>(db["room"]);
	  } while (db.fetchRow());
	  job->cur=0;
	  if (argument.word(1)=="cruise" && db.rowCount() > 1)
	    job->cruise=true;
	  else
	    job->cruise=false;
	}
      }
    } else if(argument.word(1) == "stop") {
      myself->doSay("Sail here, sail there, stop here, for the love o' me beard make up yer mind!");
      myself->doDrive("stop");
      for(int i=0;i<10;++i)
	job->room[i]=0;
      job->cur=-1;
    } else if (argument.word(1) == "take" && argument.word(2) == "five"){
      if (privileges == 2) {
	myself->doSay("Enough of that, then.");
	myself->doDrive("stop");
	for(int i=0;i<10;++i)
	  job->room[i]=0;
	job->cur=-1;
	myself->doSay("Take care of 'er.");
	myself->doGive(format("shipkey %s") % ch->getName());
	myself->doEmote(format("begins untangling %s salt encrusted beard.") % myself->hshr()); // a female captain would, of course, untangle HER beard
      } else {
	myself->doSay("Arr de arr arr!");
      }
    } else if (argument.word(1) == "go"){
      if (!job || job->cur == -1) {
	myself->doSay("Where to?");
	return TRUE;
      }
      if (argument.word(2) == "slow") {
	myself->doSay("If ye say slow...");
	job->speed = "slow";
      } else if (argument.word(2) == "medium") {
	myself->doSay("We ain't chasin' anyone then?");
	job->speed = "medium";
      } else if (argument.word(2) == "fast") {
	myself->doSay("Best to give these louts a thing to do!");
	job->speed = "fast";
      } else {
	myself->doSay("Arr ye plannin' on finishin' that thought?");
      }
    } else if (argument.word(1) == "obey" && privileges == 2) {
      // delegate command to specific player (limited command set)
      // use player_id column in ship_master
      // if no second arg, show list of delegates
      TBeing *delegate = NULL;
      if (!argument.word(2).empty()) {
	if (!(delegate = get_pc_world(myself, argument.word(2), EXACT_YES))) {
	  if (!(delegate = get_pc_world(myself, argument.word(2), EXACT_NO))) {
	    myself->doSay("I 'aven't the foggiest as to who that be.");
	    myself->doSay("Per'aps ye can point 'em out to me some time...");
	    return TRUE;
	  }
	}
	if (delegate->desc) {
	  db.query("delete from ship_master where captain_vnum = %i and player_id = %i and account_id is null", myself->mobVnum(), delegate->desc->playerID);
	  db.query("insert into ship_master (captain_vnum, player_id) select %i, %i", myself->mobVnum(), delegate->desc->playerID);
	  myself->doSay(format("Aye, <W>%s<1> may set us upon any known course.") % delegate->name);
	} else {
	  myself->doSay("That layabout?!");
	}
	
      } else {
	// list delegates
	db.query("select p1.name from ship_master s1 join player p1 on s1.player_id = p1.id and captain_vnum = %i and s1.player_id is not null", myself->mobVnum());
	if(!db.fetchRow()){
	  myself->doSay("Ye've wisely shown me no pretenders to your authority!");
	  return TRUE;
	}
	do {
	  myself->doSay(format("<W>%s<1> may bid for passage to any of our fine destinations.") % db["name"].cap());
	} while(db.fetchRow());
      }
    } else if (argument.word(1) == "ignore" && privileges == 2) {
      // remove a player's command (added via "obey")
      // use player_id column in ship_master
      TBeing *delegate = NULL;
      if (!argument.word(2).empty()) {
	if (!(delegate = get_pc_world(myself, argument.word(2), EXACT_YES))) {
	  if (!(delegate = get_pc_world(myself, argument.word(2), EXACT_NO))) {
	    myself->doSay("Never 'eard of 'em!");
	    return TRUE;
	  }
	}
	if (delegate->desc) {
	  db.query("delete from ship_master where captain_vnum = %i and player_id = %i and account_id is null", myself->mobVnum(), delegate->desc->playerID);
	  if (db.rowCount() == 0)
	    myself->doSay("I am already ignorin' that shrunken weevil!");
	  else
	    myself->doSay("'Twas a mistake ever allowin' them aboard!");
	}
      }
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
  	// destination acheived
  	// move onto next location or stop if none specified (or restart loop if "cruising")
    myself->doDrive("stop");
    myself->doSay("Avast!  We have reached arrr destination.");

    if(job->cur==9 || !job->room[job->cur+1]){
      if(job->cruise) {
      	job->cur=0;
      } else {
				for(int i=0;i<10;++i)
					job->room[i]=0;
      	job->cur=-1;
      }
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

  switch(::number(0, 200)){
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

  myself->doDrive(job->speed);

  return TRUE;
}



