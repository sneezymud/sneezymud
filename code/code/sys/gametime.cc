#include "gametime.h"
#include "weather.h"
#include "comm.h"
#include "person.h"

// static defs
struct time_info_data GameTime::time_info;
const unsigned long GameTime::BEGINNING_OF_TIME=650336715;
const int GameTime::YEAR_ADJUST=550;


// to simplify sun and moon time checks, this function returns the
// combination of hour and minute as a single int [0-95]
// the hour is simply val/4, and the minute is val%4
int GameTime::hourminTime()
{
  return time_info.hours*4 + (time_info.minutes/15);
}

// display time (given in hourminTime format) as a string
sstring GameTime::hmtAsString(int hmt)
{
  int hour = hmt/4;
  int minute = hmt%4 * 15;

  sstring buf;
  buf = format("%d:%2.2d %s") %
    (!(hour % 12) ? 12 : hour%12) %
    minute %
    ((hour >= 12) ? "PM" : "AM");
  return buf;
}


bool GameTime::is_daytime()
{
  int hmt = hourminTime();

  return (hmt >= Weather::sunTime(Weather::SUN_TIME_DAY) &&
           hmt < Weather::sunTime(Weather::SUN_TIME_SINK));
}

bool GameTime::is_nighttime()
{
  int hmt = hourminTime();

  return (hmt < Weather::sunTime(Weather::SUN_TIME_DAWN) ||
          hmt > Weather::sunTime(Weather::SUN_TIME_NIGHT));
}


void GameTime::anotherHour()
{
  sstring buf;

  // we have 4 ticks per mud hour (this is called per tick)
  time_info.minutes += 15;

  // check for new hour
  if (time_info.minutes >= 60) {
    time_info.hours++;
    time_info.minutes = 0;

    if (time_info.hours == 12)
      sendToOutdoor(COLOR_BASIC, "<Y>It is noon.<1>\n\r","<Y>It is noon.<1>\n\r");

    // check for new day
    if (time_info.hours >= 24) {
      sendToOutdoor(COLOR_BASIC, "<k>It is midnight.<1>\n\r","<k>It is midnight.<1>\n\r");
      time_info.day++;
      time_info.hours = 0;

      // check for new month
      if (time_info.day >= 28) {
        time_info.month++;
        time_info.day = 0;

        // announce new month, etc.
        Weather::GetMonth(time_info.month);
    
        // check for new year
        if (time_info.month >= 12) {
          time_info.month = 0;
          time_info.year++;
          buf = format("Happy New Year! It is now the Year %d P.S\n\r") % time_info.year;
          descriptor_list->worldSend(buf, NULL);
        }
      }
  
      // on a new day, update the moontime too
      Weather::addToMoon(1);

      if (Weather::getMoon() >= 32) {
        Weather::setMoon(0);
      }

      // on a new day, determine the new sunrise/sunset
      Weather::calcNewSunRise();
      Weather::calcNewSunSet();
    }
  }
  Weather::fixSunlight();
}


time_info_data::time_info_data() :
  seconds(0),
  minutes(0),
  hours(0),
  day(0),
  month(0),
  year(0)
{
}

time_info_data::time_info_data(const time_info_data &a)
  : seconds(a.seconds), minutes(a.minutes),
    hours(a.hours), day(a.day),
    month(a.month), year(a.year)
{
}

time_info_data & time_info_data::operator=(const time_info_data &a)
{
  if (this == &a) return *this;
  seconds = a.seconds;
  minutes = a.minutes;
  hours = a.hours;
  day = a.day;
  month = a.month;
  year = a.year;
  return *this;
}

time_info_data::~time_info_data()
{
}

// Calculate the REAL time passed over the last t2-t1 centuries (secs) 
void GameTime::realTimePassed(time_t t2, time_t t1, struct time_info_data *now)
{
  long secs;

  secs = (long) (t2 - t1);

  now->minutes = (secs / SECS_PER_REAL_MIN) % 60;
  secs -= SECS_PER_REAL_MIN * now->minutes;

  now->hours = (secs / SECS_PER_REAL_HOUR) % 24;
  secs -= SECS_PER_REAL_HOUR * now->hours;

  now->day = (secs / SECS_PER_REAL_DAY);
  secs -= SECS_PER_REAL_DAY * now->day;

  now->month = -1;
  now->year = -1;
  now->seconds = secs;

  return;
}

// Calculate the MUD time passed over the last t2-t1 centuries (secs) 
void GameTime::mudTimePassed(time_t t2, time_t t1, struct time_info_data *now)
{
  long secs;

  secs = (long) (t2 - t1);

  now->minutes = (secs / SECS_PER_UPDATE) % 4;	
  secs -= SECS_PER_UPDATE * now->minutes;

  // values are 0, 15, 30, 45...
  now->minutes *= 15;

  now->hours = (secs / SECS_PER_MUDHOUR) % 24;	
  secs -= SECS_PER_MUDHOUR * now->hours;

  now->day = (secs / SECS_PER_MUD_DAY) % 28;	
  secs -= SECS_PER_MUD_DAY * now->day;

  now->month = (secs / SECS_PER_MUD_MONTH) % 12;		
  secs -= SECS_PER_MUD_MONTH * now->month;

  now->year = (secs / SECS_PER_MUD_YEAR);	

  return;
}

void GameTime::reset_time(void)
{
  mudTimePassed(time(0), getBeginningOfTime(), &time_info);
  time_info.year += getYearAdjust();

  Weather::setMoon(getDay());

  Weather::calcNewSunRise();
  Weather::calcNewSunSet();

  Weather::fixSunlight();

  vlogf(LOG_MISC, format("   Current Gametime: %dm, %dH %dD %dM %dY.") %  
        getMinutes() % getHours() % getDay() % 
	getMonth() % getYear());

  Weather::setPressure(960);
  if ((getMonth() >= 7) && (getMonth() <= 12))
    Weather::addToPressure(dice(1, 50));
  else
    Weather::addToPressure(dice(1, 80));

  Weather::setChange(0);

  if (Weather::getPressure() <= 980) {
    if ((getMonth() >= 3) && (getMonth() <= 9))
      Weather::setSky(Weather::SKY_LIGHTNING);
    else
      Weather::setSky(Weather::SKY_LIGHTNING);
  } else if (Weather::getPressure() <= 1000) {
    if ((getMonth() >= 3) && (getMonth() <= 9))
      Weather::setSky(Weather::SKY_RAINING);
    else
      Weather::setSky(Weather::SKY_RAINING);
  } else if (Weather::getPressure() <= 1020) {
    Weather::setSky(Weather::SKY_CLOUDY);
  } else {
    Weather::setSky(Weather::SKY_CLOUDLESS);
  }
}


