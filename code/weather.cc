//////////////////////////////////////////////////////////////////////////
//
// SneezyMUD - All rights reserved, SneezyMUD Coding Team
//
// $Log: weather.cc,v $
// Revision 5.1  1999/10/16 04:31:17  batopr
// new branch
//
// Revision 1.1  1999/09/12 17:24:04  sneezy
// Initial revision
//
//
//////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////
//
//    SneezyMUD - All rights reserved, SneezyMUD Coding Team
//    "weather.cc" - All functions and routines related to weather
//
//    The sneezyMUD weather is roughly derived from the original DikuMUD
//    weather system. We at sneezyMUD tried to make the weather do a lot 
//    more than the original.
//
//////////////////////////////////////////////////////////////////////////

#include <cmath>

#include "stdsneezy.h"

// what stage is moon in?  (1 - 32) 
unsigned char moontype;

// due to the calculations involved in sunrise/set formula
// it becomes expensive to calculate this each time
// do it as needed, and save it.
static int si_sunRise = 0;
static int si_sunSet = 0;

const char * moonType()
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
int moonSet()
{
  // almanac: full    6:30  hr=13   mn=17
  // almanac: 3Q     12:30  hr=25   mn=25
  // almanac: new    18:30  hr=37   mn=1
  // almanac: 1Q      0:30  hr=1    mn=9

  int num;

  num = (48 * (moontype - 1)/32) + 37;
  if (num > 47)
    num -= 48;

  return num;
}

int moonRise()
{
  // almanac: full   18:00   hr=36   mn=17
  // almanac: 3Q      0:00   hr=0    mn=25
  // almanac: new     6:00   hr=12   mn=1
  // almanac: 1Q     12:00   hr=24   mn=9
 
  int num;
 
  num = (48 * (moontype - 1)/32) + 12;
  if (num > 47)
    num -= 48;
 
  return num;
}

int sunRise()
{
  return si_sunRise;
}

int sunSet()
{
  return si_sunSet;
}

void weatherAndTime(int mode)
{
  anotherHour();
  if (mode)
    weatherChange();
  sunriseAndSunset();
}


const char *describeTime(void)
{
  if (time_info.hours < 10) 
    return "evening";
  else if (time_info.hours < 24)
    return "morning";
  else if (time_info.hours < 36)
    return "afternoon";
  else
    return "evening";
}

void anotherHour()
{
  char buf[100];

  time_info.hours++;

  if (time_info.hours == moonSet()) {
   sendToOutdoor(COLOR_BASIC, "<b>The moon sets.<1>\n\r","<b>The moon sets.<1>\n\r");
  }
  if (time_info.hours == sunRise() - 3) {
    weather_info.sunlight = SUN_DAWN;
    sendToOutdoor(COLOR_BASIC, "<Y>Dawn begins to break.<1>\n\r",
          "<Y>Dawn begins to break.<1>\n\r");
  }
  if (time_info.hours == sunRise()) {
    weather_info.sunlight = SUN_RISE;
    sendToOutdoor(COLOR_BASIC, "<y>The sun rises in the east.<1>\n\r",
  "<y>The sun rises in the east.<1>\n\r");
  }
  if (time_info.hours == sunRise() + 3) {
    weather_info.sunlight = SUN_LIGHT;
    sendToOutdoor(COLOR_BASIC, "<W>The day has begun.<1>\n\r",
                      "<W>The day has begun.<1>\n\r");
  }
  if (time_info.hours == 24) {
    sendToOutdoor(COLOR_NONE, "It is noon.\n\r","It is noon.\n\r");
  }
  if (time_info.hours == sunSet() - 3) {
    weather_info.sunlight = SUN_SET;
    sendToOutdoor(COLOR_BASIC, "<y>The sun slowly sinks in the west.<1>\n\r",
                      "<y>The sun slowly sinks in the west.<1>\n\r");
  }
  if (time_info.hours == sunSet()) {
    weather_info.sunlight = SUN_TWILIGHT;
    sendToOutdoor(COLOR_BASIC, "<k>The sun sets as twilight begins.<1>\n\r",
                      "<k>The sun sets as twilight begins.<1>\n\r");
  }
  if (time_info.hours == sunSet() + 3) {
    weather_info.sunlight = SUN_DARK;
    sendToOutdoor(COLOR_BASIC, "<k>The night has begun.<1>\n\r","<k>The night has begun.<1>\n\r");
  }
  if (time_info.hours == moonRise()) {
    sprintf(buf, "<b>The %s moon rises in the east.<1>\n\r", moonType());
    sendToOutdoor(COLOR_BASIC, buf, buf);
  }
  
  if (time_info.hours > 47) {	// Changed by HHS due to bug ??? 
    sendToOutdoor(COLOR_NONE, "It is midnight.\n\r","It is midnight.\n\r");

    time_info.hours -= 48;
    time_info.day++;

    moontype++;
    if (moontype > 32)
      moontype = 1;

    if (time_info.day > 27) {
      time_info.day = 0;
      time_info.month++;
      GetMonth(time_info.month);
      sprintf(buf, "It is now the %s of %s.\n\r", numberAsString(time_info.day + 1).c_str(), month_name[time_info.month]);
      descriptor_list->worldSend(buf, NULL);

      if (time_info.month > 11) {
	time_info.month = 0;
	time_info.year++;
        sprintf(buf, "Happy New Year! It is now the Year %d P.S\n\r", time_info.year);
        descriptor_list->worldSend(buf, NULL);
      }
    }
    calcNewSunRise();
    calcNewSunSet();
  }
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
  char text[256];;
  soundNumT snd;

  for (i = descriptor_list; i; i = i->next) {
    if (!i->connected && (ch = i->character)) {
      if (ch->outside() && ch->awake() && ch->roomp  &&
          !(ch->isPlayerAction(PLR_MAILING | PLR_BUGGING))) {

        switch (num) {
          case WEATHER_MESS_CLOUDY:
            sprintf(text, "<b>The sky is getting cloudy<1>.\n\r");
            break;
          case WEATHER_MESS_RAIN_START:
            switch (ch->roomp->getSectorType()) {
              case SECT_DESERT:
                sprintf(text, "<k>The clouds overhead darken and look more ominous.<1>\n\r");
                break;
              default:
                sprintf(text, "<B>It starts to rain.<1>\n\r");
                ch->playsound(SOUND_RAIN_START, SOUND_TYPE_NOISE);
            }
            break;
          case WEATHER_MESS_SNOW_START:
            switch (ch->roomp->getSectorType()) {
              case SECT_JUNGLE:
                sprintf(text, "You are caught in a sudden jungle downpour.\n\r");
                ch->playsound(SOUND_RAIN_START, SOUND_TYPE_NOISE);
                break;
              case SECT_DESERT:
                sprintf(text, "<B>The cooler temperatures and worsening weather allow a light desert rain to fall.<1>\n\r");
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
                sprintf(text, "<B>It starts to rain.<1>\n\r");
                break;
              default:
                sprintf(text, "<W>A light fluffy snow starts to fall.<1>\n\r");
            }
            break;
          case WEATHER_MESS_CLOUDS_AWAY:
            sprintf(text, "<d>The clouds disappear.<1>\n\r");
            break;
          case WEATHER_MESS_LIGHTNING:
            snd = pickRandSound(SOUND_THUNDER_1, SOUND_THUNDER_4);
            switch (ch->roomp->getSectorType()) {
              case SECT_DESERT:
                sprintf(text, "<k>The clouds overhead grow dark and thunder can be heard in the distance.<1>\n\rIt's not going to rain this time of year though.\n\r");
                ch->playsound(snd, SOUND_TYPE_NOISE, 50);
                break;
              default:
                sprintf(text, "<W>You are caught in a lightning storm.<1>\n\r");
                ch->playsound(snd, SOUND_TYPE_NOISE);
            }
            break;
          case WEATHER_MESS_BLIZZARD:
            switch (ch->roomp->getSectorType()) {
              case SECT_JUNGLE:
                snd = pickRandSound(SOUND_THUNDER_1, SOUND_THUNDER_4);
                sprintf(text, "The rains turn torrential!\n\r");
                ch->playsound(snd, SOUND_TYPE_NOISE);
                break;
              case SECT_DESERT:
                snd = pickRandSound(SOUND_THUNDER_1, SOUND_THUNDER_4);
                sprintf(text, "<W>The desert rain intensifies and you are caught in a lightning storm.<1>\n\r");
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
                sprintf(text, "<W>You are caught in a lightning storm.<1>\n\r");
                snd = pickRandSound(SOUND_THUNDER_1, SOUND_THUNDER_4);
                ch->playsound(snd, SOUND_TYPE_NOISE);
                break;
              default:
                sprintf(text, "<W>You are caught in a blizzard.<1>\n\r");
            }
            break;
          case WEATHER_MESS_RAIN_AWAY:
            switch (ch->roomp->getSectorType()) {
              case SECT_DESERT:
                sprintf(text, "<d>The clouds overhead thin and begin to clear.<1>\n\r");
                break;
              default:
                sprintf(text,"<B>The rain has stopped.<1>\n\r");
            }
            break;
          case WEATHER_MESS_SNOW_AWAY:
            switch (ch->roomp->getSectorType()) {
              case SECT_JUNGLE:
                sprintf(text, "The jungle rain has stopped.\n\r");
                break;
              case SECT_DESERT:
                sprintf(text, "<B>The desert rain has stopped.<1>\n\r");
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
                sprintf(text,"<B>The rain has stopped.<1>\n\r");
                break;
              default:
                sprintf(text,"<W>The snow has stopped.<1>\n\r");
            }
            break;
          case WEATHER_MESS_LIGHTNING_AWAY:
            switch (ch->roomp->getSectorType()) {
              case SECT_DESERT:
                sprintf(text, "<k>The dark clouds overhead begin to dissipate.<1>\n\r");
                break;
              default:
                sprintf(text,"<B>The lightning has gone, but it is still raining.<1>\n\r");
            }
            break;
          case WEATHER_MESS_BLIZZARD_AWAY:
            switch (ch->roomp->getSectorType()) {
              case SECT_JUNGLE:
                sprintf(text, "<B>The lightning has gone, but a jungle rain continues to fall.<1>\n\r");
                break;
              case SECT_DESERT:
                sprintf(text, "<B>The lightning has gone, but a light desert rain continues to fall.<1>\n\r");
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
                sprintf(text,"<B>The lightning has gone, but it is still raining.<1>\n\r");
                break;
              default:
                sprintf(text,"<W>The blizzard is over, but it is still snowing.<1>\n\r");
            }
            break;
          default:
            vlogf(5, "Bad num %d sent to sendWeatherMessage", num);
            break;
        }

        string buf = colorString(ch, i, text, NULL, COLOR_BASIC, FALSE);
        (&i->output)->putInQ(buf.c_str());
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
  if (month < 0)
    return;

  if (month <= 1)
    sendToOutdoor(COLOR_NONE, "It is bitterly cold outside.\n\r",
        "It is rather chilly outdoors.\n\r");
  else if (month <= 2)
    sendToOutdoor(COLOR_NONE, "It is very cold.\n\r", "The chill in the air begins to lessen.\n\r");
  else if (month <= 3)
    sendToOutdoor(COLOR_NONE, "It is chilly outside.\n\r","It begins to warm up dramatically.\n\r");
  else if (month <= 4)
    sendToOutdoor(COLOR_NONE, "The flowers start to bloom.\n\r","The flowers start to bloom.\n\r");
  else if (month <= 7)
    sendToOutdoor(COLOR_NONE, "It is warm and humid.\n\r","A hot dry breeze blows from the west.\n\r");
  else if (month <= 8)
    sendToOutdoor(COLOR_NONE, "It starts to get a little windy.\n\r","A cool breeze blows across the humid land.\n\r");
  else if (month <= 9)
    sendToOutdoor(COLOR_NONE, "The air is getting chilly.\n\r","The weather is less humid nowadays.\n\r");
  else if (month <= 10)
    sendToOutdoor(COLOR_NONE, "The leaves start to change colors. \n\r","The weather cools somewhat.\n\r");
  else if (month <= 11)
    sendToOutdoor(COLOR_NONE, "It starts to get cold.\n\r","There is a definite chill in the air.\n\r");
  else if (month <= 12)
    sendToOutdoor(COLOR_NONE, "It is bitterly cold outside.\n\r","It is becoming cold outside.\n\r");
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
    if ((time_info.hours >= 44) || (time_info.hours < 6))   // moon is up
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

#if 0
void TBeing::checkWeatherConditions()
{
  if (!roomp) {
    vlogf(7,"Error: NULL roomp for %s.  was in room %d",getName(),in_room);
    char_to_room(this,0);
    return;
  }
}

// this doesn't do anything.  not used anywhere (yet)  - bat
void TRoom::initWeather(void)
{
  // check all surrounding rooms, if weather exists, this room
  // should have an average.
  // if weather is 0 (not inited) in all rooms, make a random number
  // most values are set to 0 in constructor, so no need to do explicitely

  dirTypeT door;
  unsigned int num_rooms = 0;
  TRoom *temp;
  for (door = MIN_DIR; door < MAX_DIR; door++) {
    if (dir_option[door]) {
      if (temp = real_roomp(dir_option[door]->to_room)) {
        // a valid exit was found
        if (temp == this)
          continue;
        if (temp->weather.pressure) {
          weather.new_pressure += temp->weather.pressure;
          num_rooms++;
          weather.new_temp += temp->weather.temp;
        }
      }
    } 
  }
  // looked at all rooms
  if (num_rooms == 0) {
    // no rooms were found, randomize pressure
    weather.pressure = ::number(970,1030);
  } else {
    weather.pressure = weather.new_pressure / num_rooms;
  }
  weather.new_temp += TerrainInfo[getSectorType()]->heat;
  weather.temp = weather.new_temp / (num_rooms + 1);
  weather.moist = TerrainInfo[getSectorType()]->humidity *= 10;
}

void TRoom::computeNewWeather()
{
  weather.new_pressure = weather.pressure;
  weather.new_temp = weather.temp;
  weather.moist += TerrainInfo[getSectorType()]->humidity;
  dirTypeT door;
  TRoom *temp;
  int diff = 0, best = 0, dir = -1;
  int num_rooms = 1;   // 1 for this room itself

  for (door = MIN_DIR; door < MAX_DIR; door++) {
    if (dir_option[door]) {
      if ((temp = real_roomp(dir_option[door]->to_room)) != NULL) {
        // a valid exit was found
        if (temp == this)
          continue;
        weather.new_pressure += temp->weather.pressure;
        num_rooms++;
        weather.new_temp += temp->weather.temp;
        diff = weather.pressure - temp->weather.pressure;
        if (diff < 0)
          diff = -diff;
        if (diff > best) {
          best = diff;
          dir = door;
        }
      }
    }
  }
  weather.wind = dir;   // wind in direction of greatest pressure difference
  weather.new_pressure /= num_rooms;
#if 0
  int deltalow = (weather.pressure - 1000)  / -30;
  int deltahigh = (weather.pressure - 1000)  / -6;
  weather.new_pressure += ::number(deltalow,deltahigh);   // kicks toward norm
  weather.new_pressure += ::number(-5,5);  // butterfly effect
#endif
  weather.new_temp /= num_rooms;
#if 0
  weather.new_temp -= (weather.pressure - 1000) / 5;
     // high pressure = cold air, low pressure = warm
  weather.new_temp += (25 - outdoorLight()/2)/2;
    // cool it down if sun not up, warm it up at midday
  switch (time_info.month) {
    case 0:
      weather.new_temp -= ::number(5,15);
      break;
    case 1:
    case 2:
    case 10:
    case 11:
      weather.new_temp -= ::number(0,10);
      break;
    case 3:
    case 4:
    case 8:
    case 9:
      weather.new_temp += ::number(5,-5);
      break;
    case 5:
    case 7:
      weather.new_temp += ::number(0,10);
      break;
    case 6:
    default:
      weather.new_temp += ::number(5,15);
  }
#endif
}

// copy the new weather (from ComputeWeather from temp to current
void TRoom::updateWeather()
{
  weather.temp = weather.new_temp;
  weather.pressure = weather.new_pressure;
}

void updateWorldWeather(void)
{
  register int i;
  register TRoom *rp;

  anotherHour();
  for (i = 0; i < WORLD_SIZE; i++) {
    rp = real_roomp(i);
    if (rp)
      rp->computeNewWeather(); 
  }
  for (i = 0; i < WORLD_SIZE; i++) {
    rp = real_roomp(i);
    if (rp)
      rp->updateWeather();
  }
}
#endif

void doGlobalRoomStuff(void)
{
  int i;
  TRoom *rp;

  //weather noise
  for (i = 0; i < WORLD_SIZE; i++) {
    rp = real_roomp(i);
    if (!rp)
      continue;
    if (rp->getWeather() == WEATHER_LIGHTNING) {
      if (!::number(0,9)) {
        TThing *in_room;

        soundNumT snd = pickRandSound(SOUND_THUNDER_1, SOUND_THUNDER_4);
        for (in_room = rp->stuff; in_room; in_room = in_room->nextThing) {
          TBeing *ch = dynamic_cast<TBeing *>(in_room);
          if (!ch || !ch->desc)
            continue;

          act("A flash of lightning illuminates the land.", FALSE, ch, 0, 0, TO_CHAR, ANSI_YELLOW);
          ch->playsound(snd, SOUND_TYPE_NOISE);
        } 
      }
    }
  }
}

void TBeing::describeWeather(int room)
{
  TRoom *rp;
  int wth;
 
  rp = real_roomp(room);
  if (!rp) {
    forceCrash("No roomp for room %d in describeWeather for %s", room, getName());
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
  int mr = moonRise();
  int ms = moonSet();

  // moon might set before it rises
  if (((mr < ms) &&
          time_info.hours >= mr && time_info.hours < ms) ||
      ((mr > ms) &&
          (time_info.hours < ms || time_info.hours >= mr)))
    return TRUE;
  return FALSE;
}

bool sunIsUp()
{
  int sr = sunRise();
  int ss = sunSet();

  // assumption that sr is always < ss
  if (time_info.hours >= sr && time_info.hours < ss)
    return TRUE;

  return FALSE;
}

bool is_daytime()
{
  return (time_info.hours >= (sunRise() + 3) &&
           time_info.hours < (sunSet() - 3));
}

bool is_nighttime()
{
  return (time_info.hours < (sunRise() - 3) ||
           time_info.hours > (sunSet() + 3));
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

  // sun up warms land
  if ((time_info.hours >= 18) && (time_info.hours < 30))
    weather_info.change -= 1;
  else if ((time_info.hours < 6) || (time_info.hours >= 42))
    weather_info.change += 1;

  // precipitation lessens air pressure
  if (weather_info.sky == SKY_RAINING)
    weather_info.change += dice(1,4);
  else if (weather_info.sky == SKY_LIGHTNING)
    weather_info.change += dice(2,3);

  weather_info.change += (dice(1, 3) * diff + dice(2, 8) - dice(2, 6));

  weather_info.change = max(-12, min(weather_info.change, 12));
  weather_info.pressure += weather_info.change;

  weather_info.pressure = min(weather_info.pressure, 1040);
  weather_info.pressure = max(weather_info.pressure, 960);
  AlterWeather(&change);
  do_components(change);
}

void calcNewSunRise()
{
  // calc new sunrise
  // seasonal variation
  // Almanac data had to be adjusted for daylight-savings
  // winter = 9 hours daylight, summer = 15 hours daylight
  // center on 5am, on APR 1
  // winter = 6:30, summer = 3:30

  int num;
  int day = (time_info.month) * 28 + time_info.day + 1;
  int equinox = 3 * 28 + 1;  // april 1st
  
  // treat whole year as sinusoidal with APR 1 as origin
  // sneezy year = 12 months of 28 days
  // 3 value is to get proper fluxuation for time
  double x = -3 * sin( 2 * M_PI * ((double) (day-equinox))/(28.0 * 12.0));

  // 5am = 10hr, 0.5 is for proper rounding
  num = 10 + (int) (x + 0.5);
  si_sunRise = num;
}

void calcNewSunSet()
{
  // seasonal variation
  // Almanac data had to be adjusted for daylight-savings
  // winter = 9 hours daylight, summer = 15 hours daylight
  // make day a little longer then normal
  // center on 17:30
  // winter = 16:00, summer = 19:00

  int num;
  int day = (time_info.month) * 28 + time_info.day + 1;
  int equinox = 3 * 28 + 1;  // april 1st
  double x;
 
  // treat whole year as sinusoidal with APR 1 as origin
  // sneezy year = 12 months of 28 days
  // 3 value is to get proper fluxuation for time
  x = 3 * sin(2 * M_PI *((double) (day-equinox))/(28.0 * 12.0));
 
  // 17:30 = 35hr, 0.5 is for proper rounding
  num = 35 + (int) (x + 0.5);
  si_sunSet = num;;
}

