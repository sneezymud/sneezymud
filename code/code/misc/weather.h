//////////////////////////////////////////////////////////////////////////
//
// SneezyMUD - All rights reserved, SneezyMUD Coding Team
//
//////////////////////////////////////////////////////////////////////////


#ifndef __H
#define __H

#include "enum.h"

class TBeing;
class TRoom;
class sstring;

class Weather {
 public:  
  enum weatherT { NONE, CLOUDLESS, CLOUDY, 
		  RAINY, LIGHTNING, SNOWY };
  
  enum sunT { SUN_DARK, SUN_DAWN, SUN_RISE, SUN_LIGHT, SUN_SET, SUN_TWILIGHT };
  
  enum skyT { SKY_CLOUDLESS, SKY_CLOUDY, SKY_RAINING, SKY_LIGHTNING };
  
  enum changeWeatherT {
    CHANGE_NONE,
    CHANGE_CLOUDS,
    CHANGE_RAIN,
    CHANGE_STORM,
    CHANGE_CLOUDS_AWAY,
    CHANGE_RAIN_AWAY,
    CHANGE_STORM_AWAY
  };

  enum moonTimeT {
    MOON_TIME_SET,
    MOON_TIME_RISE,
  };

  enum sunTimeT {
    SUN_TIME_DAWN,
    SUN_TIME_RISE,
    SUN_TIME_DAY,
    SUN_TIME_SINK,
    SUN_TIME_SET,
    SUN_TIME_NIGHT,
  };

  enum weatherMessT {
    MESS_CLOUDY,
    MESS_RAIN_START,
    MESS_SNOW_START,
    MESS_CLOUDS_AWAY,
    MESS_LIGHTNING,
    MESS_BLIZZARD,
    MESS_RAIN_AWAY,
    MESS_SNOW_AWAY,
    MESS_LIGHTNING_AWAY,
    MESS_BLIZZARD_AWAY
  };
  
  static const int WET_MAXIMUM;
  
  static void AlterWeather(changeWeatherT *);
  static void calcNewSunRise();
  static void calcNewSunSet();
  static int hourminTime();
  static sstring hmtAsString(int);
  static void anotherHour();
  static void weatherChange();
  static void GetMonth(int);
  static void sunriseAndSunset();
  static int moonTime(moonTimeT);
  static const sstring moonType();
  static bool moonIsUp();
  static int sunTime(sunTimeT);
  static bool sunIsUp();
  static int getWet(TBeing *ch, TRoom* room, silentTypeT silent);
  static const sstring describeWet(int wetness);
  static const sstring describeWet(const TBeing *ch);
  static int addWetness(TBeing *ch, int diffWet);
  static void update_world_weather();
  static int getWeather(int);
  static bool is_daytime();
  static bool is_nighttime();
  static weatherT getWeather(const TRoom &);
  static void fixSunlight();
  static void sendWeatherMessage(weatherMessT);
  static void ChangeWeather(changeWeatherT);

  // accessors
  static int getPressure(){ return pressure; }
  static int getChange(){ return change; }
  static int getSky(){ return sky; }
  static int getSunlight(){ return sunlight; }
  static int getMoon(){ return moontype; }

  // manipulators
  static void setPressure(int p){ pressure=p; }
  static void addToPressure(int p){ pressure+=p; }
  static void setChange(int c){ change=c; }
  static void addToChange(int c){ change+=c; }
  static void setSunlight(sunT s){ sunlight=s; } 
  static void setSky(skyT s){ sky=s; }
  static void setMoon(int m){ moontype=m; }

 private:
  Weather();
  
  static int pressure;        /* How is the pressure ( Mb ) */
  static int change;          /* How fast and what way does it change. */
  static skyT sky;            /* How is the sky. */
  static sunT sunlight;       /* And how much sun. */
  static int moontype;        // what stage is moon in?  (0 - 31) 
  
  // due to the calculations involved in sunrise/set formula
  // it becomes expensive to calculate this each time
  // do it as needed, and save it.
  static int si_sunRise;
  static int si_sunSet;
};


// this represents the arbitrary starting point for mud-time functions
// It is Fri Aug 10 18:05:15 1990  (Gamma 0.0 release?)
// If people care, May 1, 1992 is around 704700000 (SneezyMUD opening)
const unsigned long BEGINNING_OF_TIME      =650336715;

// Beginning_OF_TIME will be Jan 1, year 0 + YEAR_ADJUST
const int YEAR_ADJUST            =550;


extern int getRoomWetness(TRoom*);




#endif
