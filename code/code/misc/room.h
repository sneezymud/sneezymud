//////////////////////////////////////////////////////////////////////////
//
// SneezyMUD - All rights reserved, SneezyMUD Coding Team
//
//////////////////////////////////////////////////////////////////////////


#ifndef __ROOM_H
#define __ROOM_H

// cubic inches of burning material where room itself burns
const int ROOM_FIRE_THRESHOLD=20000;
const int ROOM_FLOOD_THRESHOLD=30000;

extern TRoom *room_db[];
extern TRoom *real_roomp(int);
extern int top_of_world;
extern vector<zoneData>zone_table;

// this array is used for cycling through room specials
// cycling through all the rooms takes too long so just store which rooms
// actually  have special procs
extern vector<TRoom *>roomspec_db;
extern vector<TRoom *>roomsave_db;

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

/* For 'Sector types' */
enum sectorTypeT {
     SECT_SUBARCTIC,
     SECT_ARCTIC_WASTE,
     SECT_ARCTIC_CITY,
     SECT_ARCTIC_ROAD,
     SECT_TUNDRA,
     SECT_ARCTIC_MOUNTAINS,
     SECT_ARCTIC_FOREST,
     SECT_ARCTIC_MARSH,
     SECT_ARCTIC_RIVER_SURFACE,
     SECT_ICEFLOW,
     SECT_COLD_BEACH,
     SECT_SOLID_ICE,
     SECT_ARCTIC_BUILDING,
     SECT_ARCTIC_CAVE,
     SECT_ARCTIC_ATMOSPHERE,
     SECT_ARCTIC_CLIMBING,
     SECT_ARCTIC_FOREST_ROAD,
     SECT_PLAINS,
     SECT_TEMPERATE_CITY,
     SECT_TEMPERATE_ROAD,
     SECT_GRASSLANDS,
     SECT_TEMPERATE_HILLS,
     SECT_TEMPERATE_MOUNTAINS,
     SECT_TEMPERATE_FOREST,
     SECT_TEMPERATE_SWAMP,
     SECT_TEMPERATE_OCEAN,
     SECT_TEMPERATE_RIVER_SURFACE,
     SECT_TEMPERATE_UNDERWATER,
     SECT_TEMPERATE_BEACH,
     SECT_TEMPERATE_BUILDING,
     SECT_TEMPERATE_CAVE,
     SECT_TEMPERATE_ATMOSPHERE,
     SECT_TEMPERATE_CLIMBING,
     SECT_TEMPERATE_FOREST_ROAD,
     SECT_DESERT,
     SECT_SAVANNAH,
     SECT_VELDT,
     SECT_TROPICAL_CITY,
     SECT_TROPICAL_ROAD,
     SECT_JUNGLE,
     SECT_RAINFOREST,
     SECT_TROPICAL_HILLS,
     SECT_TROPICAL_MOUNTAINS,
     SECT_VOLCANO_LAVA,
     SECT_TROPICAL_SWAMP,
     SECT_TROPICAL_OCEAN,
     SECT_TROPICAL_RIVER_SURFACE,
     SECT_TROPICAL_UNDERWATER,
     SECT_TROPICAL_BEACH,
     SECT_TROPICAL_BUILDING,
     SECT_TROPICAL_CAVE,
     SECT_TROPICAL_ATMOSPHERE,
     SECT_TROPICAL_CLIMBING,
     SECT_RAINFOREST_ROAD,
     SECT_ASTRAL_ETHREAL,
     SECT_SOLID_ROCK,
     SECT_FIRE,
     SECT_INSIDE_MOB,
     SECT_FIRE_ATMOSPHERE,
     SECT_MAKE_FLY,
     SECT_DEAD_WOODS,
     MAX_SECTOR_TYPES
};
const sectorTypeT MIN_SECTOR_TYPE = sectorTypeT(0);
extern sectorTypeT & operator++(sectorTypeT &, int);

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

#if 0
class WeatherStuff {
  public:
    short temp; 
    short new_temp;
    short avg_temp;
    short pressure;
    short new_pressure;
    byte wind;
    byte moist;
    byte new_moist;

    WeatherStuff() {
      temp = avg_temp = new_temp = 0;
      pressure = new_pressure = 0;
      wind = 0;
      moist = new_moist = 0;
    }
};
#endif

class TRoom : public TThing {
  private:
    sectorTypeT sectorType;
    dirTypeT riverDir;      // River flows toward this exit direction
    byte riverSpeed;        // River flows with this speed
    byte hasWindow;         // whether or not room has a window   
    byte teleLook;          // do a do_look or not when teleported 
    zoneData *zone;         // Room zone (for resetting)          
    sh_int teleTime;        // time to a teleport                
    int teleTarg;        // target room of a teleport       
    ubyte moblim;           // # of mobs allowed in room.       
    int roomHeight;         // room height
    unsigned int roomFlags; // Bitvector os flags for room
    long descPos;           // File offset for the description.
    int x, y, z;            // x,y,z location in the world
    ubyte fished;           // how fished out the room is
    ubyte logsHarvested;           // how deforested the room is
    int treetype;          // the kind of tree growing in the room

  public:
    TThing *tBornInsideMe;  // List of mobs born inside me.

    void operator << (      TThing &); // Add a mob to the born list.
    bool operator |= (const TThing &); // See if a mob is on the born list.
    void operator >> (const TThing &); // Remove mob from the born list.

//    WeatherStuff weather;   // not ready yet - bat
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
    void initWeather();
    void updateWeather();
    void computeNewWeather();
    void saveItems(const sstring &);
    void loadItems();
    weatherT getWeather() const;
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
    void setMoblim(ubyte limit) {
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
    ubyte getMoblim() const {
      return moblim;
    }
    int getTeleTarg() const {
      return teleTarg;
    }
    sh_int getTeleTime() const {
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
    byte getRiverSpeed() const;

    int getRoomHeight() const {
      return roomHeight;
    }
    byte getHasWindow() const {
      return hasWindow;
    }
    void setHasWindow(int window) {
      hasWindow = window;
    }
    byte getTeleLook() const {
      return teleLook;
    }
    void setTeleLook(byte look) {
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
