//////////////////////////////////////////////////////////////////////////
//
// SneezyMUD - All rights reserved, SneezyMUD Coding Team
//
// $Log: weather.h,v $
// Revision 1.1  1999/09/12 17:24:04  sneezy
// Initial revision
//
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

const unsigned long BEGINNING_OF_TIME      =650336715;
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

#endif
