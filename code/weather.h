//////////////////////////////////////////////////////////////////////////
//
// SneezyMUD - All rights reserved, SneezyMUD Coding Team
//
//////////////////////////////////////////////////////////////////////////


#ifndef __WEATHER_H
#define __WEATHER_H

enum weatherT { WEATHER_NONE, WEATHER_CLOUDLESS, WEATHER_CLOUDY, WEATHER_RAINY, WEATHER_LIGHTNING, WEATHER_SNOWY };
// need to add in WEATHER_WINDY here - Rix

enum sunT { SUN_DARK, SUN_DAWN, SUN_RISE, SUN_LIGHT, SUN_SET, SUN_TWILIGHT };

enum skyT { SKY_CLOUDLESS, SKY_CLOUDY, SKY_RAINING, SKY_LIGHTNING };

struct weather_data
{
   int pressure;        /* How is the pressure ( Mb ) */
   int change;  /* How fast and what way does it change. */
   skyT sky;     /* How is the sky. */
   sunT sunlight;        /* And how much sun. */
};

// this represents the arbitrary starting point for mud-time functions
// It is Fri Aug 10 18:05:15 1990  (Gamma 0.0 release?)
// If people care, May 1, 1992 is around 704700000 (SneezyMUD opening)
const unsigned long BEGINNING_OF_TIME      =650336715;

// Beginning_OF_TIME will be Jan 1, year 0 + YEAR_ADJUST
const int YEAR_ADJUST            =550;

enum changeWeatherT {
  WEATHER_CHANGE_NONE,
  WEATHER_CHANGE_CLOUDS,
  WEATHER_CHANGE_RAIN,
  WEATHER_CHANGE_STORM,
  WEATHER_CHANGE_CLOUDS_AWAY,
  WEATHER_CHANGE_RAIN_AWAY,
  WEATHER_CHANGE_STORM_AWAY
};

extern void AlterWeather(changeWeatherT *);
extern void calcNewSunRise();
extern void calcNewSunSet();
extern int hourminTime();
extern sstring hmtAsString(int);
extern void weatherAndTime(int);
extern struct weather_data weather_info;
extern void anotherHour();
extern void weatherChange();
extern void GetMonth(int);
extern void sunriseAndSunset();

extern int moontype;
enum moonTimeT {
  MOON_TIME_SET,
  MOON_TIME_RISE,
};

extern int moonTime(moonTimeT);
extern const sstring moonType();
extern bool moonIsUp();

enum sunTimeT {
  SUN_TIME_DAWN,
  SUN_TIME_RISE,
  SUN_TIME_DAY,
  SUN_TIME_SINK,
  SUN_TIME_SET,
  SUN_TIME_NIGHT,
};

extern int sunTime(sunTimeT);
extern bool sunIsUp();

#endif
