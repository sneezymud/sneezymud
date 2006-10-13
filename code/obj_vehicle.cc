//////////////////////////////////////////////////////////////////////////
//
// SneezyMUD - All rights reserved, SneezyMUD Coding Team
//
//////////////////////////////////////////////////////////////////////////

// this code sucks and should be rewritten from scratch at some point

#include "stdsneezy.h"
#include "obj_vehicle.h"


TVehicle::TVehicle() :
  TPortal(),
  dir(DIR_NONE),
  speed(0),
  whole_zone(false)
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



void TVehicle::assignFourValues(int x1, int x2, int x3, int x4)
{
    
  setTarget(x1);
  setType(x2);

  if(x3)
    whole_zone=true;
  else
    whole_zone=false;

  setPortalKey(GET_BITS(x4, 23, 24));
  setPortalFlags(GET_BITS(x4, 31, 8));
}

void TVehicle::getFourValues(int *x1, int *x2, int *x3, int *x4) const
{
  *x1=getTarget();
  *x2=getType();
  
  if(whole_zone)
    *x3=1;
  else
    *x3=0;


  int r = *x4;
  SET_BITS(r, 23, 24, getPortalKey());
  SET_BITS(r, 31, 8, getPortalFlags());
  *x4 = r;
}




unsigned char TVehicle::getPortalType() const
{
  return 0;
}


char TVehicle::getPortalNumCharges() const
{
  return -1;
}


void TVehicle::lookObj(TBeing *ch, int) const
{
  sstring buf;

  buf = fmt("%d look") % getTarget();
  ch->doAt(buf.c_str(), true);
}



void TVehicle::driveSpeed(TBeing *ch, int speed)
{
  if(getDir() == DIR_NONE){
    ch->sendTo("You need to choose a direction to drive in first.\n\r");
    return;
  }

  if(speed==0){
    if(getSpeed() >= FAST_SPEED){
      if(getType()==VEHICLE_BOAT){
	act("$n throws the anchor overboard, bringing $p to a rough halt.",
	    0, ch, this, 0, TO_ROOM);
	act("You throw the anchor overboard, bringing $p to a rough halt.",
	    0, ch, this, 0, TO_CHAR);
      } else {
	act("$n slams on the brakes, bringing $p to a skidding halt.",
	    0, ch, this, 0, TO_ROOM);
	act("You slam on the brakes, bringing $p to a skidding halt.", 
	    0, ch, this, 0, TO_CHAR);
      }
    } else if(getSpeed() == 0){
      act("$p is already stopped.", 0, ch, this, 0, TO_CHAR);
    } else {
      act("$n brings $p to a stop.",
	  0, ch, this, 0, TO_ROOM);
      act("You bring $p to a stop.",
	  0, ch, this, 0, TO_CHAR);
    }
    
    setSpeed(speed);
  } else {
    if(speed > getSpeed()){
      act("$p begins to accelerate.", 0, ch, this, 0, TO_ROOM);
      act("$p begins to accelerate.", 0, ch, this, 0, TO_CHAR);
    } else if(speed < getSpeed()){
      act("$p begins to decelerate.", 0, ch, this, 0, TO_ROOM);
      act("$p begins to decelerate.", 0, ch, this, 0, TO_CHAR);
    } else {
      act("$p is already going that speed.", 0, ch, this, 0, TO_CHAR);
    }
    
    setSpeed(speed);
  }
  return;
}

void TVehicle::driveDir(TBeing *ch, dirTypeT dir)
{
  sstring buf;

  setDir(dir);

  buf = fmt("$n directs $p to the %s.") % dirs[dir];
  act(buf, 0, ch, this, 0, TO_ROOM);
  buf = fmt("You direct $p to the %s.") % dirs[dir];
  act(buf, 0, ch, this, 0, TO_CHAR);
}


void TVehicle::driveExit(TBeing *ch)
{
  act("$n exits $p.", 0, ch, this, 0, TO_ROOM);
  act("You exit $p.", 0, ch, this, 0, TO_CHAR);

  --(*ch);
  *roomp+=*ch;
}

void TVehicle::driveLook(TBeing *ch, bool silent)
{
  sstring buf;

  if(!silent)
    ch->sendTo("You look outside.\n\r");

  buf = fmt("%d look") % in_room;
  ch->doAt(buf.c_str(), true);
}

bool TVehicle::isAllowedPath(int rnum)
{
  // this isn't the right place to store this
  const int elevator[]={2352, 2354, 2355, 2356, 2357, 2368, 2369, 2362, -1};
  const int trolley[]={100, 175, 176, 177, 178, 179, 180, 181, 182, 183, 
		       184, 185,
	      200, 215, 31050, 31051, 31052, 31053, 31054, 31055, 31056, 
	      31057, 31058, 31059, 31060, 31061, 31062, 31063, 31064, 31065, 
	      31066, 31067, 31068, 31069, 31070, 31071, 31072, 31073, 31074, 
	      31075, 31076, 31077, 31078, 31079, 31080, 31081, 31082, 31083, 
	      31084, 31085, 31086, 31087, 31088, 31089, 
	      650, 651, 652, 653, 654, 655, 656, 657, 658, 659,
	      660, 667, 668, 669, 670, 671, 672, 673, 674, 700, 701, 702,
	      703, 704, 705, 706, 707, 708, 709, 710, 711, 712, 713, 714,
	      715, 716, 728, 729, 730, 731, 732, 733, 734, 
	      34768, 34767, 34766, 34765, 34764, 34763, 34762, 34761, 
	      34760, 34759, 34758, 34757, 34756, 34755, 34754, 34753,
		       34752,
	      34751, 34750, 34749, 34748, 34747, 34746, 34745, 34744,
	      34743, 34742, 34741, 34740, 34739, 34738, 34737, 34736,
	      34735, 34734, 34733, 34732, 34731, 34730, 34729, 34728,
	      34727, 34726, 34725, 34724, 34723, 34722, 34721, 34720,
	      34719, 34718, 34717, 34716, 34715, 34714, 34713,
	      34712, 34711, 34710, 34709, 34708, 34707, 34706, 34705,
	      34704, 34703, 34702, 34701, 34700, 735, 736, 737,
	      738, 739, 1381, 1200, 1201, 1204, 1207, 1215, 1218, 1221, 
	      1301, 1302, 1303, -1};


  switch(objVnum()){
    case 15344:
      for(int i=0;trolley[i]!=-1;++i)
	if(rnum==trolley[i])
	  return true;
      break;
    case 2360:
      for(int i=0;elevator[i]!=-1;++i)
	if(rnum==elevator[i])
	  return true;
      break;
    default:
      return true;
  }

  return false;
}


void TVehicle::changeObjValue3(TBeing *ch)
{
  int x1, x2, x3, x4;
  getFourValues(&x1, &x2, &x3, &x4);

  ch->sendTo(VT_HOMECLR);
  ch->sendTo(fmt("What does this value do? :\n\r %s\n\r") %
       ItemInfo[itemType()]->val2_info);
  ch->specials.edit = CHANGE_OBJ_VALUE3;

  ch->sendTo(fmt("Value 3 for %s : %d\n\r\n\r") %
       sstring(getName()).uncap() % x3);
  ch->sendTo(fmt(VT_CURSPOS) % 10 % 1);
  ch->sendTo("Enter new value.\n\r--> ");
}

void update_exits(TVehicle *vehicle)
{
  TRoom *vehicleroom=real_roomp(vehicle->getTarget());

  if(!vehicle->whole_zone){
    if(!vehicle->roomp){
      if(vehicle->parent && vehicle->parent->roomp){
	// update the exit even if we're being carried or something
	for(int i=MIN_DIR;i<MAX_DIR;++i){
	  if(vehicleroom->dir_option[i])
	    vehicleroom->dir_option[i]->to_room=vehicle->parent->roomp->number;
	}
      }
      return;
    }
        
    for(int i=MIN_DIR;i<MAX_DIR;++i){
      if(vehicleroom->dir_option[i])
	vehicleroom->dir_option[i]->to_room=vehicle->roomp->number;
    }
  } else {
    // update all exits in this zone that go outside this zone
    TRoom *rp;
    int zone_num=vehicleroom->getZoneNum();

    if(!vehicle->roomp){
      if(vehicle->parent && vehicle->parent->roomp){
	// update the exit even if we're being carried or something
	for(int r=zone_table[zone_num].bottom;
	    r<=zone_table[zone_num].top;++r){
	  rp=real_roomp(r);

	  if(rp){
	    for(int i=MIN_DIR;i<MAX_DIR;++i){
	      if(rp->dir_option[i] && 
		 (rp->dir_option[i]->to_room < zone_table[zone_num].bottom ||
		  rp->dir_option[i]->to_room > zone_table[zone_num].top))
		rp->dir_option[i]->to_room=vehicle->parent->roomp->number;
	    }
	  }
	}
      }
      return;
    }


    for(int r=zone_table[zone_num].bottom;
	r<=zone_table[zone_num].top;++r){
      rp=real_roomp(r);

      if(rp){
	for(int i=MIN_DIR;i<MAX_DIR;++i){
	  if(rp->dir_option[i] && 
	     (rp->dir_option[i]->to_room < zone_table[zone_num].bottom ||
	      rp->dir_option[i]->to_room > zone_table[zone_num].top)){
	    rp->dir_option[i]->to_room=vehicle->roomp->number;
	  }
	}
      }
    }
  }
}

void TVehicle::vehiclePulse(int pulse)
{
  TThing *t;
  TBeing *tb;
  TRoom *troom=roomp;
  sstring buf;
  char shortdescr[256];
  vector<TBeing *>tBeing(0);

  update_exits(this);

  if(getSpeed()==0)
    return;

  // this is where we regulate speed
  if(pulse % max(1, (ONE_SECOND*10)/getSpeed()))
    return;

  if(getDir() == DIR_NONE)
    return;

  strcpy(shortdescr, shortDescr);
  strcpy(shortdescr, sstring(shortdescr).cap().c_str());

  if(!troom->dir_option[getDir()] ||
     !isAllowedPath(troom->dir_option[getDir()]->to_room)){
    // first count how many valid exits we have
    int dcount=0;
    for(dirTypeT dir=DIR_NORTH;dir<MAX_DIR;dir++){
      if(troom->dir_option[dir] && dir != rev_dir[getDir()] &&
	 isAllowedPath(troom->dir_option[dir]->to_room)){
	++dcount;
      }
    }
    
    // if there's only one that isn't the way we came, change direction
    if(dcount == 1){
      for(dirTypeT dir=DIR_NORTH;dir<MAX_DIR;dir++){
	if(troom->dir_option[dir] && dir != rev_dir[getDir()] &&
	   isAllowedPath(troom->dir_option[dir]->to_room)){
	  setDir(dir);

	  sendrpf(COLOR_OBJECTS, roomp, "%s changes direction to the %s and keeps moving.\n\r", shortdescr, dirs[getDir()]);

	  break;
	}
      }
    } else
      return; // otherwise just stop
  }

  // we let them go one room into non-water, like "beaching" the boat
  // or entering docks.
  TRoom *dest=real_roomp(troom->dir_option[getDir()]->to_room);
  if(getType() == VEHICLE_BOAT && !dest->isWaterSector() && 
     !roomp->isWaterSector()){
    return;
  }

  // tracked vehicles
  if(!isAllowedPath(troom->dir_option[getDir()]->to_room))
    return;

  // send message to people in old room here
  if(getType()==VEHICLE_BOAT){
    if(getSpeed() >= FAST_SPEED){
      sendrpf(COLOR_OBJECTS, roomp, "%s sails rapidly to the %s.\n\r",
	      shortdescr, dirs[getDir()]);
    } else if(getSpeed() >= MED_SPEED){
      sendrpf(COLOR_OBJECTS, roomp, "%s sails to the %s.\n\r",
	      shortdescr, dirs[getDir()]);
    } else {
      sendrpf(COLOR_OBJECTS, roomp, "%s drifts to the %s.\n\r",
	      shortdescr, dirs[getDir()]);
    }
  } else if(getType()==VEHICLE_TROLLEY){
    if(getSpeed() >= FAST_SPEED){
      sendrpf(COLOR_OBJECTS, roomp, "%s rumbles to the %s.\n\r",
	      shortdescr, dirs[getDir()]);
    } else if(getSpeed() >= MED_SPEED){
      sendrpf(COLOR_OBJECTS, roomp, "%s rumbles to the %s.\n\r",
	      shortdescr, dirs[getDir()]);
    } else {
      sendrpf(COLOR_OBJECTS, roomp, "%s rolls to the %s.\n\r",
	      shortdescr, dirs[getDir()]);
    }
  } else {
    if(getSpeed() >= FAST_SPEED){
      sendrpf(COLOR_OBJECTS, roomp, "%s speeds off to the %s.\n\r",
	      shortdescr, dirs[getDir()]);
    } else if(getSpeed() >= MED_SPEED){
      sendrpf(COLOR_OBJECTS, roomp, "%s rolls off to the %s.\n\r",
	      shortdescr, dirs[getDir()]);
    } else {
      sendrpf(COLOR_OBJECTS, roomp, "%s creeps off to the %s.\n\r",
	      shortdescr, dirs[getDir()]);
    }
  }


  --(*this);
  thing_to_room(this, troom->dir_option[getDir()]->to_room);

  // send message to people in new room here
  if(getType() == VEHICLE_BOAT){
    if(getSpeed() >= FAST_SPEED){
      sendrpf(COLOR_OBJECTS, roomp, "%s sails rapidly in from the %s.\n\r",
	      shortdescr, dirs[rev_dir[getDir()]]);
      buf = fmt("$p sails %s.") % dirs[getDir()];
    } else if(getSpeed() >= MED_SPEED){
      sendrpf(COLOR_OBJECTS, roomp, "%s sails in from the %s.\n\r",
	      shortdescr, dirs[rev_dir[getDir()]]);
      buf = fmt("$p sails %s.") % dirs[getDir()];
    } else {
      sendrpf(COLOR_OBJECTS, roomp, "%s drifts in from the %s.\n\r",
	      shortdescr, dirs[rev_dir[getDir()]]);
      buf = fmt("$p drifts %s.") % dirs[getDir()];
    }
  } else if(getType() == VEHICLE_TROLLEY){
    if(getSpeed() >= FAST_SPEED){
      sendrpf(COLOR_OBJECTS, roomp, "%s rumbles in from the %s.\n\r",
	      shortdescr, dirs[rev_dir[getDir()]]);
      buf = fmt("$p rumbles %s.") % dirs[getDir()];
    } else if(getSpeed() >= MED_SPEED){
      sendrpf(COLOR_OBJECTS, roomp, "%s rumbles in from the %s.\n\r",
	      shortdescr, dirs[rev_dir[getDir()]]);
      buf = fmt("$p rumbles %s.") % dirs[getDir()];
    } else {
      sendrpf(COLOR_OBJECTS, roomp, "%s rolls in from the %s.\n\r",
	      shortdescr, dirs[rev_dir[getDir()]]);
      buf = fmt("$p drifts %s.") % dirs[getDir()];
    }
  } else {
    if(getSpeed() >= FAST_SPEED){
      sendrpf(COLOR_OBJECTS, roomp, "%s speeds in from the %s.\n\r",
	      shortdescr, dirs[rev_dir[getDir()]]);
      buf = fmt("$p speeds %s.") % dirs[getDir()];
    } else if(getSpeed() >= MED_SPEED){
      sendrpf(COLOR_OBJECTS, roomp, "%s rolls in from the %s.\n\r",
	      shortdescr, dirs[rev_dir[getDir()]]);
      buf = fmt("$p rolls %s.") % dirs[getDir()];
    } else {
      sendrpf(COLOR_OBJECTS, roomp, "%s creeps in from the %s.\n\r",
	      shortdescr, dirs[rev_dir[getDir()]]);
      buf = fmt("$p creeps %s.") % dirs[getDir()];
    }
  }
  
  // update exits
  update_exits(this);

  // send message to people in vehicle
  troom=real_roomp(getTarget());

  // the doAt in driveLook() would screw up the getStuff list
  // so we "pre-cache" it
  for (t=troom->getStuff(); t; t = t->nextThing)
    if((tb=dynamic_cast<TBeing *>(t)))
      tBeing.push_back(tb);
  
  for(unsigned int tBeingIndex=0;tBeingIndex<tBeing.size();tBeingIndex++){
    act(buf, 0, tBeing[tBeingIndex], this, 0, TO_CHAR);
    driveLook(tBeing[tBeingIndex], true);
  }
}


void TVehicle::driveStatus(TBeing *ch)
{
  sstring buf;

  buf = fmt("$p is pointing to the %s.\n\r") % dirs[getDir()];
  act(buf, 0, ch, this, 0, TO_CHAR);
  buf = fmt("$p is traveling at a %i speed.\n\r") % getSpeed();
  act(buf, 0, ch, this, 0, TO_CHAR);
}


sstring TVehicle::statObjInfo() const
{
  sstring buf, sbuf;

  TPortal::statObjInfo();

  buf = fmt("It is pointing to the %s.\n\r") % dirs[getDir()];
  sbuf+=buf;
  buf = fmt("It is traveling at %i speed.\n\r") % getSpeed();
  sbuf+=buf;

  return sbuf;
}
