//////////////////////////////////////////////////////////////////////////
//
// SneezyMUD - All rights reserved, SneezyMUD Coding Team
//
// $Log: task_whittle.h,v $
// Revision 5.2  2003/03/13 22:40:54  peel
// added sstring class, same as string but takes NULL as an empty string
// replaced all uses of string to sstring
//
// Revision 5.1.1.1  1999/10/16 04:32:20  batopr
// new branch
//
// Revision 5.1  1999/10/16 04:31:17  batopr
// new branch
//
// Revision 1.1  1999/09/12 17:24:04  sneezy
// Initial revision
//
//
//////////////////////////////////////////////////////////////////////////


#ifndef __TASK_WHITTLE_H
#define __TASK_WHITTLE_H

enum whittleTypeT
{
  WHITTLE_ERROR,
  WHITTLE_GENERAL,
  WHITTLE_EASY,
  WHITTLE_MEDIUM,
  WHITTLE_STANDARD,
  WHITTLE_HARD,
  WHITTLE_TOUGH,
  WHITTLE_DELICATE,
  WHITTLE_INVOLVED,
  WHITTLE_STRONG,
  WHITTLE_TIMECONSUMING,
  WHITTLE_VALUABLE,
  WHITTLE_MAX
};

enum whittlePulseT
{
  WHITTLE_PULSE_ZEROOUT,
  WHITTLE_PULSE_CARVED,
  WHITTLE_PULSE_SCRAPED,
  WHITTLE_PULSE_SMOOTHED,
  WHITTLE_PULSE_MAX
};

class taskWhittleEntry
{
  public:
    sstring        name;
    double        volSize,
                  weiSize,
                  weaSize;
    int           whittleReq,
                  itemVnum,
                  tClass;
    whittleTypeT  itemType;
    bool          valid,
                  affectValue;

    bool   operator==(sstring);
    sstring getName(bool);
    void   operator()(sstring, int, int, int, bool, whittleTypeT);

    taskWhittleEntry();
    ~taskWhittleEntry();
};

#endif
