#ifndef __GAMETIME_H
#define __GAMETIME_H

#include <time.h>

class sstring;

class time_info_data
{
  public:
  int seconds, minutes, hours, day, month, year;

  time_info_data();
  time_info_data(const time_info_data &a);
  time_info_data & operator=(const time_info_data &a);
  ~time_info_data();
};

struct time_data
{
  time_t birth;    /* This represents the characters age                */
  time_t logon;    /* Time of the last logon (used to calculate played) */
  int played;      /* This is the total accumulated time played in secs */
  time_t last_logon;
};


class GameTime {
 private:
  GameTime();

  static struct time_info_data time_info;

  // this represents the arbitrary starting point for mud-time functions
  // It is Fri Aug 10 18:05:15 1990  (Gamma 0.0 release?)
  // If people care, May 1, 1992 is around 704700000 (SneezyMUD opening)
  static const unsigned long BEGINNING_OF_TIME;

  // Beginning_OF_TIME will be Jan 1, year 0 + YEAR_ADJUST
  static const int YEAR_ADJUST;

 public:
  static int hourminTime();
  static sstring hmtAsString(int);
  static void anotherHour();
  static bool is_daytime();
  static bool is_nighttime();
  static void realTimePassed(time_t, time_t, struct time_info_data *);
  static void mudTimePassed(time_t, time_t, struct time_info_data *);
  static void reset_time(void);

  // accessors
  static int getSeconds(){ return time_info.seconds; }
  static int getMinutes(){ return time_info.minutes; }
  static int getHours(){ return time_info.hours; }
  static int getDay(){ return time_info.day; }
  static int getMonth(){ return time_info.month; }
  static int getYear(){ return time_info.year; }

  static int getYearAdjust(){ return YEAR_ADJUST; }
  static unsigned long getBeginningOfTime(){ return BEGINNING_OF_TIME; }

  // manipulators
  static void setMonth(int m){ time_info.month=m; }
};

#endif
