//////////////////////////////////////////////////////////////////////////
//
// SneezyMUD - All rights reserved, SneezyMUD Coding Team
//
//
//////////////////////////////////////////////////////////////////////////

#ifndef __OBJ_VEHICLE_H
#define __OBJ_VEHICLE_H

#include "obj_portal.h"


class TVehicle : public TPortal {
 private:
  dirTypeT dir;
  int speed;
  
 public:
  void setSpeed(int s) { speed=s; }
  int getSpeed() const { return speed; }
  void setDir(dirTypeT d) { dir=d; }
  dirTypeT getDir() const { return dir; };

  virtual itemTypeT itemType() const { return ITEM_VEHICLE; }
  char getPortalNumCharges() const;
  void driveSpeed(TBeing *, string);
  void driveDir(TBeing *, dirTypeT);
  void vehiclePulse(int);
  void driveStatus(TBeing *);
  void driveExit(TBeing *);
  void driveLook(TBeing *ch, bool silent=false);

  void lookObj(TBeing *ch, int) const;
  
  TVehicle();
  TVehicle(const TVehicle &a);
  TVehicle & operator=(const TVehicle &a);
  virtual ~TVehicle();
};


#endif
