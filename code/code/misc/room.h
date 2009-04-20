//////////////////////////////////////////////////////////////////////////
//
// SneezyMUD - All rights reserved, SneezyMUD Coding Team
//
//////////////////////////////////////////////////////////////////////////


#ifndef __ROOM_H
#define __ROOM_H

#include "sound.h"
#include "ansi.h"
#include "structs.h"
#include "db.h"
#include "thing.h"
#include "obj.h"

class TRoom;

// cubic inches of burning material where room itself burns
const int ROOM_FIRE_THRESHOLD=20000;
const int ROOM_FLOOD_THRESHOLD=30000;

extern TRoom *room_db[];
extern TRoom *real_roomp(int);
extern int top_of_world;
extern std::vector<zoneData>zone_table;

// this array is used for cycling through room specials
// cycling through all the rooms takes too long so just store which rooms
// actually  have special procs
extern std::vector<TRoom *>roomspec_db;
extern std::vector<TRoom *>roomsave_db;

// this is used for track range
const unsigned int MAX_ROOMS   = 5000;

class Croom_q
{
  public:
    int room_nr;
    Croom_q *next_q;

    Croom_q()
      : room_nr(0),
        next_q(NULL) {
    }
    ~Croom_q() {
      // don't delete, it gets done by creater function
//      delete next_q;
//      next_q = NULL;
    }
};

const unsigned int ROOM_ALWAYS_LIT    = (1<<0);     // 1
const unsigned int ROOM_DEATH         = (1<<1);     // 2
const unsigned int ROOM_NO_MOB        = (1<<2);     // 4
const unsigned int ROOM_INDOORS       = (1<<3);     // 8
const unsigned int ROOM_PEACEFUL      = (1<<4);     // 16
const unsigned int ROOM_NO_STEAL      = (1<<5);     // 32
const unsigned int ROOM_NO_ESCAPE     = (1<<6);     // 64
const unsigned int ROOM_NO_MAGIC      = (1<<7);     // 128
const unsigned int ROOM_NO_PORTAL     = (1<<8);     // 256
const unsigned int ROOM_PRIVATE       = (1<<9);     // 512
const unsigned int ROOM_SILENCE       = (1<<10);    // 1024
const unsigned int ROOM_NO_ORDER      = (1<<11);    // 2048
const unsigned int ROOM_NO_FLEE       = (1<<12);    // 4096
const unsigned int ROOM_HAVE_TO_WALK  = (1<<13);    // 8192
const unsigned int ROOM_ARENA         = (1<<14);    // 16384
const unsigned int ROOM_NO_HEAL       = (1<<15);    // 32768
const unsigned int ROOM_HOSPITAL      = (1<<16);    // 65536
const unsigned int ROOM_SAVE_ROOM     = (1<<17);    // 131072
const unsigned int ROOM_NO_AUTOFORMAT = (1<<18);    // 262144
const unsigned int ROOM_BEING_EDITTED = (1<<19);    // 524288
const unsigned int ROOM_ON_FIRE       = (1<<20);  
const unsigned int ROOM_FLOODED       = (1<<21);

const int MAX_ROOM_BITS      = 22;          /* move and change */

const unsigned int EX_CLOSED       = (1<<0);   // 1
const unsigned int EX_LOCKED       = (1<<1);   // 2
const unsigned int EX_SECRET       = (1<<2);   // 4
const unsigned int EX_DESTROYED    = (1<<3);   // 8
const unsigned int EX_NOENTER      = (1<<4);   // 16
const unsigned int EX_TRAPPED      = (1<<5);   // 32
const unsigned int EX_CAVED_IN     = (1<<6);   // 64
const unsigned int EX_WARDED       = (1<<7);   // 128
const unsigned int EX_SLOPED_UP    = (1<<8);   // 256
const unsigned int EX_SLOPED_DOWN  = (1<<9);   // 512

const int MAX_DOOR_CONDITIONS = 10;   // move and change


class TTerrainInfo {
  public:
  int movement;
  int thickness;
  int hunger;
  int thirst;
  int drunk;
  int heat;
  int wetness;
  const char * const name;
  const char * const prep;
  TTerrainInfo(int, int, int, int, int, int, int, const char *,const char *);
  ~TTerrainInfo();
  private:
  //    TTerrainInfo() {} // prevent use
};

extern TTerrainInfo *TerrainInfo[MAX_SECTOR_TYPES];

// wetness factor.  Positive makes you more wet, negative less wet
// this compares with the number of fluid ounces that you have in water over you
// a typical waterskin holds 70 fl ounces, so pouring it overf your head is +70 wetness
// a 0 means you dont dry or get any wetter.
// a general rating system:
//   100 - underwater
//   50 - most river surfaces (standing only. This number is doubled if you sit/rest)
//   20 - wet.  a muddy swamp, puddles/mud etc.
//   10 - damp.  foggy, a steamy jungle, etc
//   0 - what we would consider "damp". overcast day, wet air
//  -10 - what we could consider a 'regular' day.  most zones, dry cold, or cool semi-humid
//  -20 - a warm day in a temperate zone (most cities/indoors)
//  -40 - a desert
// ignore weather - this is added later


class TRoom : public TThing {
  private:
    sectorTypeT sectorType;
    dirTypeT riverDir;      // River flows toward this exit direction
    short riverSpeed;        // River flows with this speed
    short hasWindow;         // whether or not room has a window   
    short teleLook;          // do a do_look or not when teleported 
    zoneData *zone;         // Room zone (for resetting)          
    short teleTime;        // time to a teleport                
    int teleTarg;        // target room of a teleport       
    unsigned short moblim;           // # of mobs allowed in room.       
    int roomHeight;         // room height
    unsigned int roomFlags; // Bitvector os flags for room
    long descPos;           // File offset for the description.
    int x, y, z;            // x,y,z location in the world
    unsigned short fished;           // how fished out the room is
    unsigned short logsHarvested;           // how deforested the room is
    int treetype;          // the kind of tree growing in the room

  public:
    TThing *tBornInsideMe;  // List of mobs born inside me.

    void operator << (      TThing &); // Add a mob to the born list.
    bool operator |= (const TThing &); // See if a mob is on the born list.
    void operator >> (const TThing &); // Remove mob from the born list.

    roomDirData *dir_option[MAX_DIR]; // Exits

    TRoom(int);
    virtual TThing& operator+= (TThing& t);
    virtual ~TRoom();

    // Code to load the descs straight from the db file instead of
    // storing those big honkers in the memory.
    void setDescr(const char *);
    const char * getDescr();

    int dropPool(int, liqTypeT);
    void flameRoom();
    virtual int chiMe(TBeing *);
    int checkPointroll();
    virtual void sendTo(colorTypeT, const sstring &) const;
    virtual void sendTo(const sstring &) const;
    void clientf(const sstring &);
    void loadOne(FILE *, bool);
    void colorRoom(int, int);
    sstring daynightColorRoom() const;
    virtual int getLight();
    void initLight();
    void saveItems(const sstring &);
    void loadItems();
    int outdoorLight();
    int outdoorLightWindow();
    bool putInDb(int vnum);
    void setRoomFlags(unsigned int flag) {
      roomFlags = flag;
    }
    void setTeleTarg(int targ) {
      teleTarg = targ;
    }
    void setTeleTime(int t_time) {
      teleTime = t_time;
    }
    void setMoblim(unsigned short limit) {
      moblim = limit;
    }
    void setRoomFlagBit(unsigned int i) {
      roomFlags |= i;
    }
    void removeRoomFlagBit(unsigned int i) {
      roomFlags = roomFlags & ~i;
    }
    bool isRoomFlag(unsigned int i) const {
      return ((roomFlags & i) != 0);
    }
    void clearRiverFields() {
      riverSpeed = 0;
      riverDir = DIR_NONE;
    }
    void setRiverDir(dirTypeT dir) {
      riverDir = dir;
    }
    void setRiverSpeed(int speed) {
      riverSpeed = speed;
    }
    void setRoomHeight(int r_height) {
      roomHeight = r_height;
    }
    zoneData *getZone() {
      return zone;
    }
    void setZoneNum(int z) {
      zone = &zone_table[z];
    }
    int getZoneNum() const {
      return zone?zone->zone_nr:-1;
    }
    unsigned short getMoblim() const {
      return moblim;
    }
    int getTeleTarg() const {
      return teleTarg;
    }
    short getTeleTime() const {
      return teleTime;
    }
    unsigned int getRoomFlags() const {
      return roomFlags;
    }
    int getXCoord() const {
      return x;
    }
    int getYCoord() const {
      return y;
    }
    int getZCoord() const {
      return z;
    }
    void setXCoord(int newx) {
      x=newx;
    }
    void setYCoord(int newy) {
      y=newy;
    }
    void setZCoord(int newz) {
      z=newz;
    }
    int getFished() const {
      return fished;
    }
    void setFished(int newfished) {
      fished=newfished;
    }
    int getLogsHarvested() const {
      return logsHarvested;
    }
    void setLogsHarvested(int newLogsHarvested) {
      logsHarvested=newLogsHarvested;
    }
    int getTreetype() const {
      return treetype;
    }
    void setTreetype(int newtreetype) {
      treetype = newtreetype;
    }


    bool isCitySector() const;
    bool isRoadSector() const;
    bool isFlyingSector() const;
    bool isVertSector() const;
    bool isUnderwaterSector() const;
    bool isNatureSector() const;
    bool isSwampSector() const;
    bool isBeachSector() const;
    bool isHillSector() const;
    bool isMountainSector() const;
    bool isForestSector() const;
    bool isAirSector() const;
    bool isOceanSector() const;
    bool isRiverSector() const;
    bool isIndoorSector() const;
    bool isArcticSector() const;
    bool isTropicalSector() const;
    bool isWierdSector() const;
    bool isFallSector() const;
    bool isWaterSector() const;
    bool isWildernessSector() const;
    bool notRangerLandSector() const;

    sectorTypeT getSectorType() const;
    sectorTypeT getArcticSectorType() const;
    void setSectorType(sectorTypeT type);
    dirTypeT getRiverDir() const;
    short getRiverSpeed() const;

    int getRoomHeight() const {
      return roomHeight;
    }
    short getHasWindow() const {
      return hasWindow;
    }
    void setHasWindow(int window) {
      hasWindow = window;
    }
    short getTeleLook() const {
      return teleLook;
    }
    void setTeleLook(short look) {
      teleLook = look;
    }
    void decrementWindow() {
      hasWindow--;
    }
    void incrementWindow() {
      hasWindow++;
    }
    bool roomIsEmpty(bool) const;
    virtual int checkSpec(TBeing *, cmdTypeT, const char *, TThing *);
    virtual roomDirData *exitDir(dirTypeT door) const;
    sstring describeGround() const;
    sstring describeGroundWeather() const;
    sstring describeGroundType() const;
    
    void playsound(soundNumT, const sstring &, int = 100, int = 50, int = 1) const;
    void stopsound() const;
    int brightSunlight() { return getLight() > 20; }
    int pitchBlackDark() { return getLight() <= 0; }

};

const int ZONE_MAX_TIME      = 50;

#endif
