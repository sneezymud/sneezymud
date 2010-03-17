//////////////////////////////////////////////////////////////////////////
//
// SneezyMUD - All rights reserved, SneezyMUD Coding Team
//
//    "weather.cc" - All functions and routines related to weather
//
//////////////////////////////////////////////////////////////////////////

#include <cmath>

#include "weather.h"
#include "room.h"
#include "extern.h"
#include "being.h"
#include "person.h"
#include "colorstring.h"
#include "monster.h"
#include "obj_trash_pile.h"
#include "process.h"

// static data defs
int Weather::pressure;
int Weather::change;
Weather::skyT Weather::sky;
Weather::sunT Weather::sunlight;
int Weather::moontype;
int Weather::si_sunRise=0;
int Weather::si_sunSet=0;
const int Weather::WET_MAXIMUM=100;



const sstring Weather::moonType()
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
int Weather::moonTime(moonTimeT mtt)
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

int Weather::sunTime(sunTimeT stt)
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

void procWeatherAndTime::run(const TPulse &) const
{
  GameTime::anotherHour();
  Weather::weatherChange();
  Weather::sunriseAndSunset();
}


const sstring describeTime(void)
{
  if (GameTime::getHours() < 5) 
    return "evening";
  else if (GameTime::getHours() < 12)
    return "morning";
  else if (GameTime::getHours() < 18)
    return "afternoon";
  else
    return "evening";
}

void Weather::fixSunlight()
{
  int hmt = GameTime::hourminTime();
  sstring buf;

  if (hmt == moonTime(MOON_TIME_SET)) {
   sendToOutdoor(COLOR_BASIC, "<b>The moon sets.<1>\n\r","<b>The moon sets.<1>\n\r");
  }
  if (hmt == moonTime(MOON_TIME_RISE)) {
    buf = format("<b>The %s moon rises in the east.<1>\n\r") % moonType();
    sendToOutdoor(COLOR_BASIC, buf, buf);
  }
  if (hmt == sunTime(SUN_TIME_DAWN)) {
    setSunlight(SUN_DAWN);
    sendToOutdoor(COLOR_BASIC, "<Y>The skies brighten as dawn begins.<1>\n\r",
          "<Y>Dawn begins to break.<1>\n\r");
  }
  if (hmt == sunTime(SUN_TIME_RISE)) {
    setSunlight(SUN_RISE);
    sendToOutdoor(COLOR_BASIC, "<y>The sun rises in the east.<1>\n\r",
  "<y>The sun rises in the east.<1>\n\r");
  }
  if (hmt == sunTime(SUN_TIME_DAY)) {
    setSunlight(SUN_LIGHT);
    sendToOutdoor(COLOR_BASIC, "<W>The day has begun.<1>\n\r",
                      "<W>The day has begun.<1>\n\r");
  }
  if (hmt == sunTime(SUN_TIME_SINK)) {
    setSunlight(SUN_SET);
    sendToOutdoor(COLOR_BASIC, "<y>The sun slowly sinks in the west.<1>\n\r",
                      "<y>The sun slowly sinks in the west.<1>\n\r");
  }
  if (hmt == sunTime(SUN_TIME_SET)) {
    setSunlight(SUN_TWILIGHT);
    sendToOutdoor(COLOR_BASIC, "<k>The sun sets as twilight begins.<1>\n\r",
                      "<k>The sun sets as twilight begins.<1>\n\r");
  }
  if (hmt == sunTime(SUN_TIME_NIGHT)) {
    setSunlight(SUN_DARK);
    sendToOutdoor(COLOR_BASIC, "<k>The night has begun.<1>\n\r","<k>The night has begun.<1>\n\r");
  }
}


void Weather::sendWeatherMessage(weatherMessT num)
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
          case MESS_CLOUDY:
            text="<b>The sky is getting cloudy<1>.\n\r";
            break;
          case MESS_RAIN_START:

            switch (ch->roomp->getSectorType()) {
              case SECT_DESERT:
                text="<k>The clouds overhead darken and look more ominous.<1>\n\r";
                break;
              default:
                text="<B>It starts to rain.<1>\n\r";
                ch->playsound(SOUND_RAIN_START, SOUND_TYPE_NOISE);
            }
            break;
          case MESS_SNOW_START:
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
          case MESS_CLOUDS_AWAY:
            text="<d>The clouds disappear.<1>\n\r";
            break;
          case MESS_LIGHTNING:
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
          case MESS_BLIZZARD:
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
          case MESS_RAIN_AWAY:
            switch (ch->roomp->getSectorType()) {
              case SECT_DESERT:
                text="<d>The clouds overhead thin and begin to clear.<1>\n\r";
                break;
              default:
                text="<B>The rain has stopped.<1>\n\r";
            }
            break;
          case MESS_SNOW_AWAY:
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
          case MESS_LIGHTNING_AWAY:
            switch (ch->roomp->getSectorType()) {
              case SECT_DESERT:
                text="<k>The dark clouds overhead begin to dissipate.<1>\n\r";
                break;
              default:
                text="<B>The lightning has gone, but it is still raining.<1>\n\r";
            }
            break;
          case MESS_BLIZZARD_AWAY:
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
            vlogf(LOG_BUG, format("Bad num %d sent to sendWeatherMessage") %  num);
            break;
        }

        sstring buf = colorString(ch, i, text, NULL, COLOR_BASIC, FALSE);
        i->output.putInQ(new UncategorizedComm(buf));
      }
    }
  }
}

void Weather::ChangeWeather(changeWeatherT change)
{
  switch (change) {
    case CHANGE_NONE:
      break;
    case CHANGE_CLOUDS:
      // getting cloudy
      sendWeatherMessage(MESS_CLOUDY);
      setSky(SKY_CLOUDY);
      break;
    case CHANGE_RAIN:
      if ((GameTime::getMonth() > 2) && (GameTime::getMonth() < 11)) {
        // starts to rain
        sendWeatherMessage(MESS_RAIN_START);
      } else {
        // starts to snow
        sendWeatherMessage(MESS_SNOW_START);
      }
      setSky(SKY_RAINING);
      break;
    case CHANGE_CLOUDS_AWAY:
      // clouds disappear
      sendWeatherMessage(MESS_CLOUDS_AWAY);
      setSky(SKY_CLOUDLESS);
      break;
    case CHANGE_STORM:
      if ((GameTime::getMonth() > 2) && (GameTime::getMonth() < 11)) {
        // caught in lightning
        sendWeatherMessage(MESS_LIGHTNING);
      } else {
        // caught in blizzard
        sendWeatherMessage(MESS_BLIZZARD);
      }
      setSky(SKY_LIGHTNING);
      break;
    case CHANGE_RAIN_AWAY:
      if ((GameTime::getMonth() > 2) && (GameTime::getMonth() < 11)) {
        // rain has stopped
        sendWeatherMessage(MESS_RAIN_AWAY);
      } else {
        // snow has stopped
        sendWeatherMessage(MESS_SNOW_AWAY);
      }
      setSky(SKY_CLOUDY);
      break;
    case CHANGE_STORM_AWAY:
      if ((GameTime::getMonth() > 2) && (GameTime::getMonth() < 11)) {
        sendWeatherMessage(MESS_LIGHTNING_AWAY);
      } else {
        sendWeatherMessage(MESS_BLIZZARD_AWAY);
      }
      setSky(SKY_RAINING);
      break;
    default:
      break;
  }
}

void Weather::AlterWeather(changeWeatherT *change)
{
  switch (getSky()) {
    case SKY_CLOUDLESS:
      if (getPressure() < 990)
	*change = CHANGE_CLOUDS;
      else if (getPressure() < 1010)
	if (dice(1, 4) == 1)
	  *change = CHANGE_CLOUDS;
      break;
    case SKY_CLOUDY:
      if (getPressure() < 970)
	*change = CHANGE_RAIN;
      else if (getPressure() < 990)
	if (dice(1, 4) == 1)
	  *change = CHANGE_RAIN;
	else
	  *change = CHANGE_NONE;
      else if (getPressure() > 1030)
	if (dice(1, 4) == 1)
	  *change = CHANGE_CLOUDS_AWAY;
      break;
    case SKY_RAINING:
      if (getPressure() < 970)
	if (dice(1, 4) == 1)
	  *change = CHANGE_STORM;
	else
	  *change = CHANGE_NONE;
      else if (getPressure() > 1030)
	*change = CHANGE_RAIN_AWAY;
      else if (getPressure() > 1010)
	if (dice(1, 4) == 1)
	  *change = CHANGE_RAIN_AWAY;
      break;
    case SKY_LIGHTNING:
      if (getPressure() > 1010)
	*change = CHANGE_STORM_AWAY;
      else if (getPressure() > 990)
	if (dice(1, 4) == 1)
	  *change = CHANGE_STORM_AWAY;
      break;
    default:
      *change = CHANGE_NONE;
      setSky(SKY_CLOUDLESS);
      break;
  }
  ChangeWeather(*change);
}

Weather::weatherT Weather::getWeather(const TRoom &room)
{
  if (room.isRoomFlag(ROOM_INDOORS))
    return NONE;

  if (room.isUnderwaterSector())
    return NONE;

  switch (getSky()) {
    case SKY_RAINING:
      if ((GameTime::getMonth() <= 2) || (GameTime::getMonth() >= 11)) {
        if (room.isTropicalSector())
          return RAINY;
        else
          return SNOWY;
      } else {
        if (room.isTropicalSector())
          return CLOUDY;
        else
          return RAINY;
      }
    case SKY_LIGHTNING:
      if ((GameTime::getMonth() <= 2) || (GameTime::getMonth() >= 11)) {
        if (room.isTropicalSector())
          return LIGHTNING;
        else
          return SNOWY;
      } else {
        if (room.isTropicalSector())
          return CLOUDY;
        else
          return LIGHTNING;
      }
    case SKY_CLOUDY:
      return CLOUDY;
    case SKY_CLOUDLESS:
      return CLOUDLESS;     
    default:
      return NONE;
  }
}

void Weather::GetMonth(int month)
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
  buf = format("It is now the %s of %s.\n\r") % numberAsString(GameTime::getDay() + 1) % month_name[month];
  descriptor_list->worldSend(buf, NULL);
}

int TRoom::outdoorLight(void)
{
  int num = 0;

  switch (Weather::getSunlight()) {
    case Weather::SUN_DAWN:
      num = 2;
      break;
    case Weather::SUN_RISE:
      num = 10;
      break;
    case Weather::SUN_LIGHT:
      num = 25;
      break;
    case Weather::SUN_SET:
      num = 10;
      break;
    case Weather::SUN_TWILIGHT:
      num = 1;
      break;
    case Weather::SUN_DARK:
      num = 0;
      break;
    default:
      break;
  }
  switch (Weather::getWeather(*this)) {
    case Weather::Weather::CLOUDY:
//  case Weather::WINDY:
    case Weather::Weather::LIGHTNING:
      num -= 1;
      break;
    case Weather::Weather::RAINY:
      num -= 2;
      break;
    case Weather::SNOWY:
      num -= 3;
      break;
    default:
      break;
  }
  if ((Weather::getMoon() >= 12) && (Weather::getMoon() < 20))    // full moon
    if (Weather::moonIsUp() && !Weather::sunIsUp())
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

  switch (Weather::getSunlight()) {
    case Weather::SUN_RISE:
    case Weather::SUN_SET:
      num = 6;
      break;
    case Weather::SUN_LIGHT:
      num = 13;
      break;
    case Weather::SUN_DARK:
    case Weather::SUN_TWILIGHT:
    case Weather::SUN_DAWN:
      num = 0;
      break;
    default:
      break;
  }
  switch (Weather::getWeather(*this)) {
    case Weather::Weather::CLOUDY:
//  case Weather::WINDY:
    case Weather::Weather::LIGHTNING:
      num -= 1;
      break;
    case Weather::Weather::RAINY:
      num -= 2;
      break;
    case Weather::SNOWY:
      num -= 3;
      break;
    default:
      break;
  }
  if ((Weather::getMoon() >= 12) && (Weather::getMoon() < 20))    // full moon
    if (Weather::moonIsUp() && !Weather::sunIsUp())
      num += 1;

  return num;
}


void Weather::sunriseAndSunset(void)
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
    vlogf(LOG_BUG, format("No roomp for room %d in describeWeather for %s") %  room % getName());
    return;
  }
  wth = Weather::getWeather(*rp);
 
  if (wth == Weather::SNOWY)
    sendTo(COLOR_BASIC, "<W>Snow falls and covers the landscape.<1>\n\r");
  else if (wth == Weather::Weather::LIGHTNING)
    sendTo(COLOR_BASIC, "<B>It is raining heavily.<1>\n\r");
  else if (wth == Weather::Weather::RAINY)
    sendTo(COLOR_BASIC, "<B>The rain comes down in sheets, soaking you to the bone.<1>\n\r");
  else if (wth == Weather::Weather::CLOUDY)
    sendTo(COLOR_BASIC, "<k>Dark clouds cover the sky.<1>\n\r");
//else if (wth == WINDY)
//  sendTo(COLOR_BASIC, "<c>The wind starts to pick up slightly.<1>\n\r");
}

// return true if nighttime mob, and it is not nighttime.
bool TMonster::isNocturnal() const
{
  return (IS_SET(specials.act, ACT_NOCTURNAL) && !GameTime::is_nighttime());
}

// return true if daytime mob, and it is not daytime.
bool TMonster::isDiurnal() const
{
  return (IS_SET(specials.act, ACT_DIURNAL) && !GameTime::is_daytime());
}

bool Weather::moonIsUp()
{
  int mr = moonTime(MOON_TIME_RISE);
  int ms = moonTime(MOON_TIME_SET);
  int hmt = GameTime::hourminTime();

  // moon might set before it rises
  if (((mr < ms) &&
          hmt >= mr && hmt < ms) ||
      ((mr > ms) &&
          (hmt < ms || hmt >= mr)))
    return TRUE;
  return FALSE;
}

bool Weather::sunIsUp()
{
  int sr = sunTime(Weather::SUN_TIME_RISE);
  int ss = sunTime(Weather::SUN_TIME_SET);
  int hmt = GameTime::hourminTime();

  // assumption that sr is always < ss
  if (hmt >= sr && hmt < ss)
    return TRUE;

  return FALSE;
}

void Weather::weatherChange()
{
  // high pressure = cold, low pressure = warm
  // pressure drops signals worse weather coming
  // keep in range 1040 - 960

  int diff = 0;
  changeWeatherT change;

  // create nice fluxuating driven toward 1000
  if (Weather::getPressure() > 1024)   
    diff = -2;
  else if (Weather::getPressure() > 1008)   
    diff = -1;
  else if (Weather::getPressure() > 992)
    diff = 0;
  else if (Weather::getPressure() > 976)
    diff = +1;
  else
    diff = +2;

#if 0
// a worthy idea, but seems to make for crappy weather
  // summer months are warm, winter months cold : drive pressure accordingly
  if ((GameTime::getMonth() == 6) || (GameTime::getMonth() == 7))
    Weather::addToChange(-2);
  else if ((GameTime::getMonth() == 5) || (GameTime::getMonth() == 8))
    Weather::addToChange(-1);
  else if ((GameTime::getMonth() == 4) || (GameTime::getMonth() == 9))
    Weather::addToChange(-0);
  else if ((GameTime::getMonth() == 3) || (GameTime::getMonth() == 10))
    Weather::addToChange(1);
  else if ((GameTime::getMonth() == 2) || (GameTime::getMonth() == 11))
    Weather::addToChange(2);
  else
    Weather::addToChange(3);
#endif

  // sun up warms land
  if (sunIsUp())
    Weather::addToChange(-1);
  else if (GameTime::is_nighttime())
    Weather::addToChange(1);

  // precipitation lessens air pressure
  if (Weather::getSky() == Weather::SKY_RAINING)
    Weather::addToChange(dice(1,4));
  else if (Weather::getSky() == Weather::SKY_LIGHTNING)
    Weather::addToChange(dice(2,3));

  // slightly randomize things
  Weather::addToChange((dice(1, 3) * diff + dice(2, 8) - dice(2, 6)));

  // limit to range -12..+12
  Weather::setChange(max(-12, min(Weather::getChange(), 12)));

  // this function gets called every tick (15 mud minutes)
  // lets keep this from changing WAY too radically
  Weather::addToPressure(Weather::getChange()/10);

  if(toggleInfo[TOG_QUESTCODE3]->toggle) {
    Weather::setChange(-5);
  }

  if (Weather::getChange() > 0) {
    if (::number(0,9) < Weather::getChange()%10)
      Weather::addToPressure(1);
  } else {
    if (::number(0,9) < (-Weather::getChange())%10)
      Weather::addToPressure(-1);
  }

  

  Weather::setPressure(min(Weather::getPressure(), 1040));
  Weather::setPressure(max(Weather::getPressure(), 960));
  AlterWeather(&change);
  do_components(change);
}

// sunrise and sunset have seasonal variations
// equinox on april 1 (month=3, day=0)
// 12 hours of daylight on the equinox, 6am-6pm
// winter solstices is 9 hours of daylight (7:30-4:30)
// summer solstices is 15 hours of daylight (4:30-7:30)

void Weather::calcNewSunRise()
{
  // calc new sunrise
  int day = (GameTime::getMonth()) * 28 + GameTime::getDay() + 1;
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

void Weather::calcNewSunSet()
{
  // calc new sunset
  int day = (GameTime::getMonth()) * 28 + GameTime::getDay() + 1;
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

// true if we're getting wet from stepping on the ground
bool isGroundWater(sectorTypeT sector)
{
  return TerrainInfo[sector] > 0 &&
     (sector == SECT_ARCTIC_MARSH ||
      sector == SECT_ARCTIC_RIVER_SURFACE ||
      sector == SECT_ICEFLOW ||
      sector == SECT_COLD_BEACH ||
      sector == SECT_TEMPERATE_SWAMP ||
      sector == SECT_TEMPERATE_OCEAN ||
      sector == SECT_TEMPERATE_RIVER_SURFACE ||
      sector == SECT_TEMPERATE_BEACH ||
      sector == SECT_TROPICAL_SWAMP ||
      sector == SECT_TROPICAL_OCEAN ||
      sector == SECT_TROPICAL_RIVER_SURFACE ||
      sector == SECT_TROPICAL_BEACH);
}

int getRoomWetness(TBeing *ch, TRoom* room, sstring & better,  sstring & worse)
{
  // get wetness
  int wetness = TerrainInfo[room->getSectorType()]->wetness;

  if (wetness > 0 && ch && isGroundWater(room->getSectorType()))
  {
    if (ch->hasBoat())
    {
      wetness = -10;
      better = "you were using your boat";
    }
    // if youre not touching the ground, no wet feet!
    else if (ch->isLevitating() || ch->getPosition() >= POSITION_MOUNTED)
    {
      wetness = -10;
      better = "you aren't touching the ground";
    }
    // if youre sitting, resting or lower you end up getting more wet with groundwater
    else if ((ch->getPosition() <= POSITION_SITTING || ch->getPosition() == POSITION_CRAWLING) && !ch->riding)
    {
      wetness *= 2;
      worse = "you are not standing";
    }
  }

  // burning obj in room
  unsigned int beforeFireLen = better.length();
  for(StuffIter it= room->stuff.begin();it!= room->stuff.end();++it)
  {
    TObj *o = dynamic_cast<TObj *>(*it);
    if (o && o->isObjStat(ITEM_BURNING))
    {
      wetness -= int(100 * (o->getVolume() / (ROOM_FIRE_THRESHOLD * 3)));
      if (better.length() != beforeFireLen)
        continue;
      if (!better.empty())
        better += " and ";
      better = format("%s dries you") % o->getName();
    }
  }

  // flaming flesh
  if (ch && ch->affectedBySpell(SPELL_FLAMING_FLESH))
  {
    wetness -= 10;
    if (!better.empty())
      better += " and ";
    better = format("your fire magic dries you");
  }

  // weather
  if (Weather::getWeather(*room) == Weather::Weather::LIGHTNING)
  {
    wetness += 20;
    if (!worse.empty())
      worse += " and ";
    worse += "it is raining";
  }
  else if (Weather::getWeather(*room) == Weather::Weather::RAINY)
  {
    wetness += 30;
    if (!worse.empty())
      worse += " and ";
    worse += "it is pouring rain";
  }
  else if (Weather::getSunlight() == Weather::SUN_LIGHT && 
	   Weather::getWeather(*room) == Weather::CLOUDLESS)
  {
    wetness -= 10;
    if (!better.empty())
      better += " and ";
    better += "it is sunny";
  }

  return min(wetness, Weather::WET_MAXIMUM);
}

// returns wetness for a room
int getRoomWetness(TRoom* room)
{
  sstring a, b;
  return getRoomWetness(NULL, room, a, b);
}

// returns how wet you are
int getWetness(const TBeing *ch)
{
  affectedData *wetAffect = NULL;
  for (wetAffect = ch->affected; wetAffect; wetAffect = wetAffect->next)
    if (wetAffect->type == AFFECT_WET)
      return max(0L, min(wetAffect->modifier, long(Weather::WET_MAXIMUM)));
  return 0;
}


// apply wetness code here
// we either add more wetness, or we 'dry' the character out (remove wetness)
void Weather::getWet(TBeing *ch, TRoom* room)
{
  sstring better, worse;
  int maxWet = getRoomWetness(ch, room, better, worse);
  int wetness = maxWet / 3; // the delta
  int oldWet = getWetness(ch);

  // getting wetter goes faster than getting dry
  if (maxWet < 0)
    wetness = maxWet / 5;

  // even though its wet out, we are wetter and will dry off some
  if (oldWet > maxWet && wetness > 0)
    wetness = -wetness;

  // the min amount of wetness change should be +/- 10
  if (wetness != 0 && abs(wetness) < 10)
    wetness = wetness > 0 ? 10 : -10;

  // if we'd overshoot the maxWet, when just go up to maxWet OR
  // if we'd undershoot the maxWet (drying in a wet room), just stop at maxWet
  if ((wetness > 0 && wetness + oldWet > maxWet) ||
    (wetness < 0 && maxWet > 0 && oldWet + wetness < maxWet))
    wetness = maxWet-oldWet;

  // add new wetness affect
  if (wetness != 0 && oldWet != maxWet && (wetness > 0 || oldWet > 0)){
    int newWet = addWetness(ch, wetness);
    if (newWet == oldWet)
      return;

    sstring wetShow;
    
    if (wetness > 0 && oldWet <= 0){
      wetShow = format("You begin to get wet from staying %s %s") %
	TerrainInfo[room->getSectorType()]->prep %
	sstring(TerrainInfo[room->getSectorType()]->name).lower();
    } else if (newWet == 0){
      wetShow = "You feel completely dried off now";
    } else {
      wetShow = format("Your time %s %s means you %s") %
	TerrainInfo[room->getSectorType()]->prep %
	sstring(TerrainInfo[room->getSectorType()]->name).lower() %
	(wetness > 0 ? "get wetter" : "dry off some");
    }
    
    if (!better.empty()){
      wetShow += (wetness < 0) ? ", in part because " : " even though ";
      wetShow += better;
    }
    if (!worse.empty()){
      wetShow += (wetness > 0) ? ", in part because " : " even though ";
      wetShow += worse;
    }
    wetShow += ".\n\r";
    act(wetShow, false, ch, NULL, NULL, TO_CHAR);
  }

  return;
}

// describes wetness for a char
const sstring Weather::describeWet(const TBeing *ch)
{
  int wetness = getWetness(ch);
  const char * color = wetness > WET_MAXIMUM/2 ? ch->blue() : ch->cyan();
  return format("%s%s%s") % color % describeWet(wetness) % ch->norm();
}

// generically describes wetness (room eval)
const sstring Weather::describeWet(int wetness)
{
  static const sstring DescribeDry[] = {
    "normal humidity",
    "slightly dry",
    "dry",
    "arid",
    "very arid",
    "parched",
     };
  static const sstring DescribeWet[] = {
    "slightly damp",
    "damp",
    "slightly wet",
    "wet",
    "dripping wet",
    "sopping wet",
    "drenched",
    "completely soaked",
    "waterlogged" };

  if (wetness <= 0)
    return DescribeDry[max(min((int)cElements(DescribeDry)-1, -(wetness/10)), 0)];
  return DescribeWet[max(min((int)cElements(DescribeWet)-1, (wetness/10)-1), 0)];
}

// adds (or removes) wetness from the character
int Weather::addWetness(TBeing *ch, int diffWet)
{
  affectedData *wetAffect = NULL;

  for (wetAffect = ch->affected; wetAffect; wetAffect = wetAffect->next)
    if (wetAffect->type == AFFECT_WET)
      break;

  if (!wetAffect && diffWet > 0)
  {
    affectedData aff;
    aff.type = AFFECT_WET;
    aff.modifier = diffWet;
    aff.duration = PERMANENT_DURATION;
    ch->affectJoin(ch, &aff, AVG_DUR_NO, AVG_EFF_YES, FALSE);
    return diffWet;
  }
  else if (wetAffect)
  {
    diffWet = wetAffect->modifier = max(0, min(WET_MAXIMUM, int(wetAffect->modifier) + diffWet));
    if (diffWet <= 0)
      wetAffect->duration = 0;
    return diffWet;
  }

  return 0;
}
