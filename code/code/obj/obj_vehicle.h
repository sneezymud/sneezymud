//////////////////////////////////////////////////////////////////////////
//
// SneezyMUD - All rights reserved, SneezyMUD Coding Team
//
//
//////////////////////////////////////////////////////////////////////////

#pragma once

#include "obj_portal.h"

const int VEHICLE_NONE = 0;
const int VEHICLE_BOAT = 1;
const int VEHICLE_TROLLEY = 2;

const int FAST_SPEED = 100;
const int MED_SPEED = 50;
const int SLOW_SPEED = 25;

class TVehicle : public TPortal {
  private:
    dirTypeT dir;
    int speed;
    int type;

  public:
    bool whole_zone;

    void setSpeed(int s) { speed = s; }
    int getSpeed() const { return speed; }
    void setType(int s) { type = s; }
    int getType() const { return type; }
    void setDir(dirTypeT d) { dir = d; }
    dirTypeT getDir() const { return dir; };
    virtual sstring statObjInfo() const;

    bool isAllowedPath(int rnum);
    virtual void assignFourValues(int, int, int, int);
    virtual void getFourValues(int*, int*, int*, int*) const;
    virtual void changeObjValue3(TBeing*);
    virtual itemTypeT itemType() const { return ITEM_VEHICLE; }
    unsigned char getPortalType() const;
    char getPortalNumCharges() const;
    void driveSpeed(TBeing*, int);
    void driveDir(TBeing*, dirTypeT);
    void vehiclePulse(int);
    void driveStatus(TBeing*);
    void driveExit(TBeing*);
    void driveLook(TBeing* ch, bool silent = false);

    void lookObj(TBeing* ch, int) const;

    TVehicle();
    TVehicle(const TVehicle& a);
    TVehicle& operator=(const TVehicle& a);
    virtual ~TVehicle();
};
