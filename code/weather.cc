//////////////////////////////////////////////////////////////////////////
//
// SneezyMUD - All rights reserved, SneezyMUD Coding Team
//
//    "weather.cc" - All functions and routines related to weather
//
//////////////////////////////////////////////////////////////////////////

#include <cmath>

#include "stdsneezy.h"
#include "obj_trash_pile.h"
#include "process.h"

// what stage is moon in?  (0 - 31) 
int moontype;

// due to the calculations involved in sunrise/set formula
// it becomes expensive to calculate this each time
// do it as needed, and save it.
static int si_sunRise = 0;
static int si_sunSet = 0;

// to simplify sun and moon time checks, this function returns the
// combination of hour and minute as a single int [0-95]
// the hour is simply val/4, and the minute is val%4
int hourminTime()
{
  return time_info.hours*4 + (time_info.minutes/15);
}

const sstring moonType()
{
  if (moontype < 4)
    return "new";
  else if (moontype < 12)
    return "waxing";
  else if (moontype < 20)
    return "full";
  else if (moontype < 28)
    return "waning";
  else
    return "new";
}

// the set/rise times are loosely based on almanac data.
// I've fudged some factors just for simplicity
// realize sneezy-time and real-world time differ
int moonTime(moonTimeT mtt)
{
  int num;
    
  switch (mtt) {
    case MOON_TIME_SET:
      // almanac: full    6:30  hour=26   moon=16
      // almanac: 3Q     12:30  hour=50   moon=24
      // almanac: new    18:30  hour=74   moon=0
      // almanac: 1Q      0:30  hour=2    moon=8
    
      num = (96 * moontype/32) + 74;
      num %= 96;

      return num;
    case MOON_TIME_RISE:
      // almanac: full   18:00   hr=72   mn=16
      // almanac: 3Q      0:00   hr=0    mn=24
      // almanac: new     6:00   hr=24   mn=0
      // almanac: 1Q     12:00   hr=48   mn=8
    
      num = (96 * moontype/32) + 24;
      num %= 96;

      return num;
  }
  return 0;
}

int sunTime(sunTimeT stt)
{
  switch (stt) {
    case SUN_TIME_DAWN:
      return si_sunRise - 6;
    case SUN_TIME_RISE:
      return si_sunRise;
    case SUN_TIME_DAY:
      return si_sunRise + 6;
    case SUN_TIME_SINK:
      return si_sunSet - 6;
    case SUN_TIME_SET:
      return si_sunSet;
    case SUN_TIME_NIGHT:
      return si_sunSet + 6;
  }
  return 0;
}

// procWeatherAndTime
procWeatherAndTime::procWeatherAndTime(const int &p)
{
  trigger_pulse=p;
  name="procWeatherAndTime";
}

void procWeatherAndTime::run(int pulse) const
{
  anotherHour();
  weatherChange();
  sunriseAndSunset();
}


const sstring describeTime(void)
{
  if (time_info.hours < 5) 
    return "evening";
  else if (time_info.hours < 12)
    return "morning";
  else if (time_info.hours < 18)
    return "afternoon";
  else
    return "evening";
}

void fixSunlight()
{
  int hmt = hourminTime();
  sstring buf;

  if (hmt == moonTime(MOON_TIME_SET)) {
   sendToOutdoor(COLOR_BASIC, "<b>The moon sets.<1>\n\r","<b>The moon sets.<1>\n\r");
  }
  if (hmt == moonTime(MOON_TIME_RISE)) {
    buf = fmt("<b>The %s moon rises in the east.<1>\n\r") % moonType();
    sendToOutdoor(COLOR_BASIC, buf, buf);
  }
  if (hmt == sunTime(SUN_TIME_DAWN)) {
    weather_info.sunlight = SUN_DAWN;
    sendToOutdoor(COLOR_BASIC, "<Y>The skies brighten as dawn begins.<1>\n\r",
          "<Y>Dawn begins to break.<1>\n\r");
  }
  if (hmt == sunTime(SUN_TIME_RISE)) {
    weather_info.sunlight = SUN_RISE;
    sendToOutdoor(COLOR_BASIC, "<y>The sun rises in the east.<1>\n\r",
  "<y>The sun rises in the east.<1>\n\r");
  }
  if (hmt == sunTime(SUN_TIME_DAY)) {
    weather_info.sunlight = SUN_LIGHT;
    sendToOutdoor(COLOR_BASIC, "<W>The day has begun.<1>\n\r",
                      "<W>The day has begun.<1>\n\r");
  }
  if (hmt == sunTime(SUN_TIME_SINK)) {
    weather_info.sunlight = SUN_SET;
    sendToOutdoor(COLOR_BASIC, "<y>The sun slowly sinks in the west.<1>\n\r",
                      "<y>The sun slowly sinks in the west.<1>\n\r");
  }
  if (hmt == sunTime(SUN_TIME_SET)) {
    weather_info.sunlight = SUN_TWILIGHT;
    sendToOutdoor(COLOR_BASIC, "<k>The sun sets as twilight begins.<1>\n\r",
                      "<k>The sun sets as twilight begins.<1>\n\r");
  }
  if (hmt == sunTime(SUN_TIME_NIGHT)) {
    weather_info.sunlight = SUN_DARK;
    sendToOutdoor(COLOR_BASIC, "<k>The night has begun.<1>\n\r","<k>The night has begun.<1>\n\r");
  }
}

void anotherHour()
{
  sstring buf;

  // we have 4 ticks per mud hour (this is called per tick)
  time_info.minutes += 15;

  // check for new hour
  if (time_info.minutes >= 60) {
    time_info.hours++;
    time_info.minutes = 0;

    if (time_info.hours == 12)
      sendToOutdoor(COLOR_NONE, "It is noon.\n\r","It is noon.\n\r");

    // check for new day
    if (time_info.hours >= 24) {
      sendToOutdoor(COLOR_NONE, "It is midnight.\n\r","It is midnight.\n\r");
      time_info.day++;
      time_info.hours = 0;

      // check for new month
      if (time_info.day >= 28) {
        time_info.month++;
        time_info.day = 0;

        // announce new month, etc.
        GetMonth(time_info.month);
    
        // check for new year
        if (time_info.month >= 12) {
          time_info.month = 0;
          time_info.year++;
          buf = fmt("Happy New Year! It is now the Year %d P.S\n\r") % time_info.year;
          descriptor_list->worldSend(buf, NULL);
        }
      }
  
      // on a new day, update the moontime too
      moontype++;

      if (moontype >= 32) {
        moontype = 0;
      }

      // on a new day, determine the new sunrise/sunset
      calcNewSunRise();
      calcNewSunSet();
    }
  }
  fixSunlight();
}

enum weatherMessT {
  WEATHER_MESS_CLOUDY,
  WEATHER_MESS_RAIN_START,
  WEATHER_MESS_SNOW_START,
  WEATHER_MESS_CLOUDS_AWAY,
  WEATHER_MESS_LIGHTNING,
  WEATHER_MESS_BLIZZARD,
  WEATHER_MESS_RAIN_AWAY,
  WEATHER_MESS_SNOW_AWAY,
  WEATHER_MESS_LIGHTNING_AWAY,
  WEATHER_MESS_BLIZZARD_AWAY
};

static void sendWeatherMessage(weatherMessT num)
{
  Descriptor *i;
  TBeing *ch;
  sstring text;
  soundNumT snd;

  for (i = descriptor_list; i; i = i->next) {
    if (!i->connected && (ch = i->character)) {
      if (ch->outside() && ch->awake() && ch->roomp  &&
          !(ch->isPlayerAction(PLR_MAILING | PLR_BUGGING))) {

        switch (num) {
          case WEATHER_MESS_CLOUDY:
            text="<b>The sky is getting cloudy<1>.\n\r";
            break;
          case WEATHER_MESS_RAIN_START:
            switch (ch->roomp->getSectorType()) {
              case SECT_DESERT:
                text="<k>The clouds overhead darken and look more ominous.<1>\n\r";
                break;
              default:
                text="<B>It starts to rain.<1>\n\r";
                ch->playsound(SOUND_RAIN_START, SOUND_TYPE_NOISE);
            }
            break;
          case WEATHER_MESS_SNOW_START:
            switch (ch->roomp->getSectorType()) {
              case SECT_JUNGLE:
                text="You are caught in a sudden jungle downpour.\n\r";
                ch->playsound(SOUND_RAIN_START, SOUND_TYPE_NOISE);
                break;
              case SECT_DESERT:
                text="<B>The cooler temperatures and worsening weather allow a light desert rain to fall.<1>\n\r";
                ch->playsound(SOUND_RAIN_START, SOUND_TYPE_NOISE);
                break;
              case SECT_SAVANNAH:
              case SECT_VELDT:
              case SECT_TROPICAL_CITY:
              case SECT_TROPICAL_ROAD:
              case SECT_RAINFOREST:
              case SECT_TROPICAL_HILLS:
              case SECT_TROPICAL_MOUNTAINS:
              case SECT_VOLCANO_LAVA:
              case SECT_TROPICAL_SWAMP:
              case SECT_TROPICAL_OCEAN:
              case SECT_TROPICAL_RIVER_SURFACE:
              case SECT_TROPICAL_UNDERWATER:
              case SECT_TROPICAL_BEACH:
              case SECT_TROPICAL_BUILDING:
              case SECT_TROPICAL_CAVE:
              case SECT_TROPICAL_ATMOSPHERE:
              case SECT_TROPICAL_CLIMBING:
              case SECT_RAINFOREST_ROAD:
                ch->playsound(SOUND_RAIN_START, SOUND_TYPE_NOISE);
                text="<B>It starts to rain.<1>\n\r";
                break;
              default:
                text="<W>A light fluffy snow starts to fall.<1>\n\r";
            }
            break;
          case WEATHER_MESS_CLOUDS_AWAY:
            text="<d>The clouds disappear.<1>\n\r";
            break;
          case WEATHER_MESS_LIGHTNING:
            snd = pickRandSound(SOUND_THUNDER_1, SOUND_THUNDER_4);
            switch (ch->roomp->getSectorType()) {
              case SECT_DESERT:
                text="<k>The clouds overhead grow dark and thunder can be heard in the distance.<1>\n\rIt's not going to rain this time of year though.\n\r";
                ch->playsound(snd, SOUND_TYPE_NOISE, 50);
                break;
              default:
                text="<W>You are caught in a lightning storm.<1>\n\r";
                ch->playsound(snd, SOUND_TYPE_NOISE);
            }
            break;
          case WEATHER_MESS_BLIZZARD:
            switch (ch->roomp->getSectorType()) {
              case SECT_JUNGLE:
                snd = pickRandSound(SOUND_THUNDER_1, SOUND_THUNDER_4);
                text="The rains turn torrential!\n\r";
                ch->playsound(snd, SOUND_TYPE_NOISE);
                break;
              case SECT_DESERT:
                snd = pickRandSound(SOUND_THUNDER_1, SOUND_THUNDER_4);
                text="<W>The desert rain intensifies and you are caught in a lightning storm.<1>\n\r";
                ch->playsound(snd, SOUND_TYPE_NOISE);
                break;
              case SECT_SAVANNAH:
              case SECT_VELDT:
              case SECT_TROPICAL_CITY:
              case SECT_TROPICAL_ROAD:
              case SECT_RAINFOREST:
              case SECT_TROPICAL_HILLS:
              case SECT_TROPICAL_MOUNTAINS:
              case SECT_VOLCANO_LAVA:
              case SECT_TROPICAL_SWAMP:
              case SECT_TROPICAL_OCEAN:
              case SECT_TROPICAL_RIVER_SURFACE:
              case SECT_TROPICAL_UNDERWATER:
              case SECT_TROPICAL_BEACH:
              case SECT_TROPICAL_BUILDING:
              case SECT_TROPICAL_CAVE:
              case SECT_TROPICAL_ATMOSPHERE:
              case SECT_TROPICAL_CLIMBING:
              case SECT_RAINFOREST_ROAD:
                text="<W>You are caught in a lightning storm.<1>\n\r";
                snd = pickRandSound(SOUND_THUNDER_1, SOUND_THUNDER_4);
                ch->playsound(snd, SOUND_TYPE_NOISE);
                break;
              default:
                text="<W>You are caught in a blizzard.<1>\n\r";
            }
            break;
          case WEATHER_MESS_RAIN_AWAY:
            switch (ch->roomp->getSectorType()) {
              case SECT_DESERT:
                text="<d>The clouds overhead thin and begin to clear.<1>\n\r";
                break;
              default:
                text="<B>The rain has stopped.<1>\n\r";
            }
            break;
          case WEATHER_MESS_SNOW_AWAY:
            switch (ch->roomp->getSectorType()) {
              case SECT_JUNGLE:
                text="The jungle rain has stopped.\n\r";
                break;
              case SECT_DESERT:
                text="<B>The desert rain has stopped.<1>\n\r";
                break;
              case SECT_SAVANNAH:
              case SECT_VELDT:
              case SECT_TROPICAL_CITY:
              case SECT_TROPICAL_ROAD:
              case SECT_RAINFOREST:
              case SECT_TROPICAL_HILLS:
              case SECT_TROPICAL_MOUNTAINS:
              case SECT_VOLCANO_LAVA:
              case SECT_TROPICAL_SWAMP:
              case SECT_TROPICAL_OCEAN:
              case SECT_TROPICAL_RIVER_SURFACE:
              case SECT_TROPICAL_UNDERWATER:
              case SECT_TROPICAL_BEACH:
              case SECT_TROPICAL_BUILDING:
              case SECT_TROPICAL_CAVE:
              case SECT_TROPICAL_ATMOSPHERE:
              case SECT_TROPICAL_CLIMBING:
              case SECT_RAINFOREST_ROAD:
                text="<B>The rain has stopped.<1>\n\r";
                break;
              default:
		text="<W>The snow has stopped.<1>\n\r";
            }
            break;
          case WEATHER_MESS_LIGHTNING_AWAY:
            switch (ch->roomp->getSectorType()) {
              case SECT_DESERT:
                text="<k>The dark clouds overhead begin to dissipate.<1>\n\r";
                break;
              default:
                text="<B>The lightning has gone, but it is still raining.<1>\n\r";
            }
            break;
          case WEATHER_MESS_BLIZZARD_AWAY:
            switch (ch->roomp->getSectorType()) {
              case SECT_JUNGLE:
                text="<B>The lightning has gone, but a jungle rain continues to fall.<1>\n\r";
                break;
              case SECT_DESERT:
                text="<B>The lightning has gone, but a light desert rain continues to fall.<1>\n\r";
                break;
              case SECT_SAVANNAH:
              case SECT_VELDT:
              case SECT_TROPICAL_CITY:
              case SECT_TROPICAL_ROAD:
              case SECT_RAINFOREST:
              case SECT_TROPICAL_HILLS:
              case SECT_TROPICAL_MOUNTAINS:
              case SECT_VOLCANO_LAVA:
              case SECT_TROPICAL_SWAMP:
              case SECT_TROPICAL_OCEAN:
              case SECT_TROPICAL_RIVER_SURFACE:
              case SECT_TROPICAL_UNDERWATER:
              case SECT_TROPICAL_BEACH:
              case SECT_TROPICAL_BUILDING:
              case SECT_TROPICAL_CAVE:
              case SECT_TROPICAL_ATMOSPHERE:
              case SECT_TROPICAL_CLIMBING:
              case SECT_RAINFOREST_ROAD:
                text="<B>The lightning has gone, but it is still raining.<1>\n\r";
                break;
              default:
                text="<W>The blizzard is over, but it is still snowing.<1>\n\r";
            }
            break;
          default:
            vlogf(LOG_BUG, fmt("Bad num %d sent to sendWeatherMessage") %  num);
            break;
        }

        sstring buf = colorString(ch, i, text, NULL, COLOR_BASIC, FALSE);
        i->output.putInQ(buf);
      }
    }
  }
}

static void ChangeWeather(changeWeatherT change)
{
  switch (change) {
    case WEATHER_CHANGE_NONE:
      break;
    case WEATHER_CHANGE_CLOUDS:
      // getting cloudy
      sendWeatherMessage(WEATHER_MESS_CLOUDY);
      weather_info.sky = SKY_CLOUDY;
      break;
    case WEATHER_CHANGE_RAIN:
      if ((time_info.month > 2) && (time_info.month < 11)) {
        // starts to rain
        sendWeatherMessage(WEATHER_MESS_RAIN_START);
      } else {
        // starts to snow
        sendWeatherMessage(WEATHER_MESS_SNOW_START);
      }
      weather_info.sky = SKY_RAINING;
      break;
    case WEATHER_CHANGE_CLOUDS_AWAY:
      // clouds disappear
      sendWeatherMessage(WEATHER_MESS_CLOUDS_AWAY);
      weather_info.sky = SKY_CLOUDLESS;
      break;
    case WEATHER_CHANGE_STORM:
      if ((time_info.month > 2) && (time_info.month < 11)) {
        // caught in lightning
        sendWeatherMessage(WEATHER_MESS_LIGHTNING);
      } else {
        // caught in blizzard
        sendWeatherMessage(WEATHER_MESS_BLIZZARD);
      }
      weather_info.sky = SKY_LIGHTNING;
      break;
    case WEATHER_CHANGE_RAIN_AWAY:
      if ((time_info.month > 2) && (time_info.month < 11)) {
        // rain has stopped
        sendWeatherMessage(WEATHER_MESS_RAIN_AWAY);
      } else {
        // snow has stopped
        sendWeatherMessage(WEATHER_MESS_SNOW_AWAY);
      }
      weather_info.sky = SKY_CLOUDY;
      break;
    case WEATHER_CHANGE_STORM_AWAY:
      if ((time_info.month > 2) && (time_info.month < 11)) {
        sendWeatherMessage(WEATHER_MESS_LIGHTNING_AWAY);
      } else {
        sendWeatherMessage(WEATHER_MESS_BLIZZARD_AWAY);
      }
      weather_info.sky = SKY_RAINING;
      break;
    default:
      break;
  }
}

void AlterWeather(changeWeatherT *change)
{
  switch (weather_info.sky) {
    case SKY_CLOUDLESS:
      if (weather_info.pressure < 990)
	*change = WEATHER_CHANGE_CLOUDS;
      else if (weather_info.pressure < 1010)
	if (dice(1, 4) == 1)
	  *change = WEATHER_CHANGE_CLOUDS;
      break;
    case SKY_CLOUDY:
      if (weather_info.pressure < 970)
	*change = WEATHER_CHANGE_RAIN;
      else if (weather_info.pressure < 990)
	if (dice(1, 4) == 1)
	  *change = WEATHER_CHANGE_RAIN;
	else
	  *change = WEATHER_CHANGE_NONE;
      else if (weather_info.pressure > 1030)
	if (dice(1, 4) == 1)
	  *change = WEATHER_CHANGE_CLOUDS_AWAY;
      break;
    case SKY_RAINING:
      if (weather_info.pressure < 970)
	if (dice(1, 4) == 1)
	  *change = WEATHER_CHANGE_STORM;
	else
	  *change = WEATHER_CHANGE_NONE;
      else if (weather_info.pressure > 1030)
	*change = WEATHER_CHANGE_RAIN_AWAY;
      else if (weather_info.pressure > 1010)
	if (dice(1, 4) == 1)
	  *change = WEATHER_CHANGE_RAIN_AWAY;
      break;
    case SKY_LIGHTNING:
      if (weather_info.pressure > 1010)
	*change = WEATHER_CHANGE_STORM_AWAY;
      else if (weather_info.pressure > 990)
	if (dice(1, 4) == 1)
	  *change = WEATHER_CHANGE_STORM_AWAY;
      break;
    default:
      *change = WEATHER_CHANGE_NONE;
      weather_info.sky = SKY_CLOUDLESS;
      break;
  }
  ChangeWeather(*change);
}

weatherT TRoom::getWeather()
{
  if (isRoomFlag(ROOM_INDOORS))
    return WEATHER_NONE;

  if (isUnderwaterSector())
    return WEATHER_NONE;

  switch (weather_info.sky) {
    case SKY_RAINING:
      if ((time_info.month <= 2) || (time_info.month >= 11)) {
        if (isTropicalSector())
          return WEATHER_RAINY;
        else
          return WEATHER_SNOWY;
      } else {
        if (isTropicalSector())
          return WEATHER_CLOUDY;
        else
          return WEATHER_RAINY;
      }
    case SKY_LIGHTNING:
      if ((time_info.month <= 2) || (time_info.month >= 11)) {
        if (isTropicalSector())
          return WEATHER_LIGHTNING;
        else
          return WEATHER_SNOWY;
      } else {
        if (isTropicalSector())
          return WEATHER_CLOUDY;
        else
          return WEATHER_LIGHTNING;
      }
    case SKY_CLOUDY:
#if 0
      if ((time_info.month <=2) || (time_info.month >= 11)) {
        if (isMountainSector())
          return WEATHER_WINDY;
        else
          return WEATHER_CLOUDY;
        } else {
          if (isArcticSector())
            return WEATHER_CLOUDY;
          else
            return WEATHER_WINDY;
      }
#endif
      return WEATHER_CLOUDY;
    case SKY_CLOUDLESS:
      return WEATHER_CLOUDLESS;     
    default:
      return WEATHER_NONE;
  }
}

void GetMonth(int month)
{
  // at the time this is called, month has rolled over, but we haven't
  // reset december+1 to january.  month is in range [1-12]
  // correct this by doing...
  month = month%12;

  switch (month) {
    case 0:
      sendToOutdoor(COLOR_NONE, "It is bitterly cold outside.\n\r",
          "It is rather chilly outdoors.\n\r");
      break;
    case 1:
      sendToOutdoor(COLOR_NONE, "It is very cold.\n\r", "The chill in the air begins to lessen.\n\r");
      break;
    case 2:
      sendToOutdoor(COLOR_NONE, "It is chilly outside.\n\r","It begins to warm up dramatically.\n\r");
      break;
    case 3:
      sendToOutdoor(COLOR_NONE, "The flowers start to bloom.\n\r","The flowers start to bloom.\n\r");
      break;
    case 4:
    case 5:
    case 6:
      sendToOutdoor(COLOR_NONE, "It is warm and humid.\n\r","A hot dry breeze blows from the west.\n\r");
      break;
    case 7:
      sendToOutdoor(COLOR_NONE, "It starts to get a little windy.\n\r","A cool breeze blows across the humid land.\n\r");
      break;
    case 8:
      sendToOutdoor(COLOR_NONE, "The air is getting chilly.\n\r","The weather is less humid nowadays.\n\r");
      break;
    case 9:
      sendToOutdoor(COLOR_NONE, "The leaves start to change colors. \n\r","The weather cools somewhat.\n\r");
      break;
    case 10:
      sendToOutdoor(COLOR_NONE, "It starts to get cold.\n\r","There is a definite chill in the air.\n\r");
      break;
    case 11:
      sendToOutdoor(COLOR_NONE, "It is bitterly cold outside.\n\r","It is becoming cold outside.\n\r");
      break;
    default:
      break;
  }

  sstring buf;
  buf = fmt("It is now the %s of %s.\n\r") % numberAsString(time_info.day + 1) % month_name[month];
  descriptor_list->worldSend(buf, NULL);
}

int TRoom::outdoorLight(void)
{
  int num = 0;

  switch (weather_info.sunlight) {
    case SUN_DAWN:
      num = 2;
      break;
    case SUN_RISE:
      num = 10;
      break;
    case SUN_LIGHT:
      num = 25;
      break;
    case SUN_SET:
      num = 10;
      break;
    case SUN_TWILIGHT:
      num = 1;
      break;
    case SUN_DARK:
      num = 0;
      break;
    default:
      break;
  }
  switch (getWeather()) {
    case WEATHER_CLOUDY:
//  case WEATHER_WINDY:
    case WEATHER_LIGHTNING:
      num -= 1;
      break;
    case WEATHER_RAINY:
      num -= 2;
      break;
    case WEATHER_SNOWY:
      num -= 3;
      break;
    default:
      break;
  }
  if ((moontype >= 12) && (moontype < 20))    // full moon
    if (moonIsUp() && !sunIsUp())
      num += 1;

  return num;
}


int TRoom::outdoorLightWindow(void)
{
  int num = 0;
  if (isRoomFlag(ROOM_INDOORS)) {
    num = getLight()/3;
    return (num);
  }

  switch (weather_info.sunlight) {
    case SUN_RISE:
    case SUN_SET:
      num = 6;
      break;
    case SUN_LIGHT:
      num = 13;
      break;
    case SUN_DARK:
    case SUN_TWILIGHT:
    case SUN_DAWN:
      num = 0;
      break;
    default:
      break;
  }
  switch (getWeather()) {
    case WEATHER_CLOUDY:
//  case WEATHER_WINDY:
    case WEATHER_LIGHTNING:
      num -= 1;
      break;
    case WEATHER_RAINY:
      num -= 2;
      break;
    case WEATHER_SNOWY:
      num -= 3;
      break;
    default:
      break;
  }
  if ((moontype >= 12) && (moontype < 20))    // full moon
    if (moonIsUp() && !sunIsUp())
      num += 1;

  return num;
}


void sunriseAndSunset(void)
{
  TRoom *rp;
  int i;

  for (i = 0; i < WORLD_SIZE; i++) 
    if ((rp = real_roomp(i)) != NULL)
      rp->initLight();
}

void TBeing::describeWeather(int room)
{
  TRoom *rp;
  int wth;
 
  rp = real_roomp(room);
  if (!rp) {
    vlogf(LOG_BUG, fmt("No roomp for room %d in describeWeather for %s") %  room % getName());
    return;
  }
  wth = rp->getWeather();
 
  if (wth == WEATHER_SNOWY)
    sendTo(COLOR_BASIC, "<W>Snow falls and covers the landscape.<1>\n\r");
  else if (wth == WEATHER_LIGHTNING)
    sendTo(COLOR_BASIC, "<B>It is raining heavily.<1>\n\r");
  else if (wth == WEATHER_RAINY)
    sendTo(COLOR_BASIC, "<B>The rain comes down in sheets, soaking you to the bone.<1>\n\r");
  else if (wth == WEATHER_CLOUDY)
    sendTo(COLOR_BASIC, "<k>Dark clouds cover the sky.<1>\n\r");
//else if (wth == WEATHER_WINDY)
//  sendTo(COLOR_BASIC, "<c>The wind starts to pick up slightly.<1>\n\r");
}

// return true if nighttime mob, and it is not nighttime.
bool TMonster::isNocturnal() const
{
  return (IS_SET(specials.act, ACT_NOCTURNAL) && !is_nighttime());
}

// return true if daytime mob, and it is not daytime.
bool TMonster::isDiurnal() const
{
  return (IS_SET(specials.act, ACT_DIURNAL) && !is_daytime());
}

bool moonIsUp()
{
  int mr = moonTime(MOON_TIME_RISE);
  int ms = moonTime(MOON_TIME_SET);
  int hmt = hourminTime();

  // moon might set before it rises
  if (((mr < ms) &&
          hmt >= mr && hmt < ms) ||
      ((mr > ms) &&
          (hmt < ms || hmt >= mr)))
    return TRUE;
  return FALSE;
}

bool sunIsUp()
{
  int sr = sunTime(SUN_TIME_RISE);
  int ss = sunTime(SUN_TIME_SET);
  int hmt = hourminTime();

  // assumption that sr is always < ss
  if (hmt >= sr && hmt < ss)
    return TRUE;

  return FALSE;
}

bool is_daytime()
{
  int hmt = hourminTime();

  return (hmt >= sunTime(SUN_TIME_DAY) &&
           hmt < sunTime(SUN_TIME_SINK));
}

bool is_nighttime()
{
  int hmt = hourminTime();

  return (hmt < sunTime(SUN_TIME_DAWN) ||
          hmt > sunTime(SUN_TIME_NIGHT));
}

void weatherChange()
{
  // high pressure = cold, low pressure = warm
  // pressure drops signals worse weather coming
  // keep in range 1040 - 960

  int diff = 0;
  changeWeatherT change;

  // create nice fluxuating driven toward 1000
  if (weather_info.pressure > 1024)   
    diff = -2;
  else if (weather_info.pressure > 1008)   
    diff = -1;
  else if (weather_info.pressure > 992)
    diff = 0;
  else if (weather_info.pressure > 976)
    diff = +1;
  else
    diff = +2;

#if 0
// a worthy idea, but seems to make for crappy weather
  // summer months are warm, winter months cold : drive pressure accordingly
  if ((time_info.month == 6) || (time_info.month == 7))
    weather_info.change -= 2;
  else if ((time_info.month == 5) || (time_info.month == 8))
    weather_info.change -= 1;
  else if ((time_info.month == 4) || (time_info.month == 9))
    weather_info.change -= 0;
  else if ((time_info.month == 3) || (time_info.month == 10))
    weather_info.change += 1;
  else if ((time_info.month == 2) || (time_info.month == 11))
    weather_info.change += 2;
  else
    weather_info.change += 3;
#endif

  // sun up warms land
  if (sunIsUp())
    weather_info.change -= 1;
  else if (is_nighttime())
    weather_info.change += 1;

  // precipitation lessens air pressure
  if (weather_info.sky == SKY_RAINING)
    weather_info.change += dice(1,4);
  else if (weather_info.sky == SKY_LIGHTNING)
    weather_info.change += dice(2,3);

  // slightly randomize things
  weather_info.change += (dice(1, 3) * diff + dice(2, 8) - dice(2, 6));

  // limit to range -12..+12
  weather_info.change = max(-12, min(weather_info.change, 12));

  // this function gets called every tick (15 mud minutes)
  // lets keep this from changing WAY too radically
  weather_info.pressure += weather_info.change/10;

  if(toggleInfo[TOG_QUESTCODE3]->toggle) {
    weather_info.change = -5;
  }

  if (weather_info.change > 0) {
    if (::number(0,9) < weather_info.change%10)
      weather_info.pressure++;
  } else {
    if (::number(0,9) < (-weather_info.change)%10)
      weather_info.pressure--;
  }

  

  weather_info.pressure = min(weather_info.pressure, 1040);
  weather_info.pressure = max(weather_info.pressure, 960);
  AlterWeather(&change);
  do_components(change);
}

// sunrise and sunset have seasonal variations
// equinox on april 1 (month=3, day=0)
// 12 hours of daylight on the equinox, 6am-6pm
// winter solstices is 9 hours of daylight (7:30-4:30)
// summer solstices is 15 hours of daylight (4:30-7:30)

void calcNewSunRise()
{
  // calc new sunrise
  int day = (time_info.month) * 28 + time_info.day + 1;
  int equinox = 3 * 28 + 1;  // april 1st
  
  // treat whole year as sinusoidal with APR 1 as origin
  // sneezy year = 12 months of 28 days
  double x = sin( 2 * M_PI * ((double) (day-equinox))/(28.0 * 12.0));
  // x is in range [-1.0 (winter solstice) to +1.0 (summer solstice)]

  // at solstices, there are +-3 hours of daylight
  // so move sunrise back by HALF that amount
  x *= -1.5;
  
  // 6am  + seasonal value
  // the 0.5 is for proper rounding
  // convert our number into hourminTime
  si_sunRise = (6*4+0) + (int) (x*4 + 0.5);
}

void calcNewSunSet()
{
  // calc new sunset
  int day = (time_info.month) * 28 + time_info.day + 1;
  int equinox = 3 * 28 + 1;  // april 1st
 
  // treat whole year as sinusoidal with APR 1 as origin
  // sneezy year = 12 months of 28 days
  double x = sin(2 * M_PI *((double) (day-equinox))/(28.0 * 12.0));
  // x is in range [-1.0 (winter solstice) to +1.0 (summer solstice)]

  // at solstices, there are +-3 hours of daylight
  // so move sunset ahead by HALF that amount
  x *= 1.5;
  
  // 6pm  + seasonal value
  // the 0.5 is for proper rounding
  // convert our number into hourminTime
  si_sunSet = (18*4+0) + (int) (x*4 + 0.5);
}

// display time (given in hourminTime format) as a string
sstring hmtAsString(int hmt)
{
  int hour = hmt/4;
  int minute = hmt%4 * 15;

  sstring buf;
  buf = fmt("%d:%2.2d %s") %
    (!(hour % 12) ? 12 : hour%12) %
    minute %
    ((hour >= 12) ? "PM" : "AM");
  return buf;
}
