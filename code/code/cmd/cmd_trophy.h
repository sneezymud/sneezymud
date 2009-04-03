#ifndef __DISC_CMD_TROPHY_H
#define __DISC_CMD_TROPHY_H

// TTrophy is a class for interacting with the trophy data
// for a particular player
//
// ch->trophy->getExpModDescr(ch->trophy->getCount(mobvnum));
//
// The TBeing class contains an instance of TTrophy that is initialized for
// that particular TBeing.  Normally, you would use that instance of TTrophy.
// In cases where there is no TBeing instance (such as a player wipe) a
// seperate instance can be initialized.  This class only has affect on
// players, but is safe to use on mobs and there will be no affect.
//
// TTrophy(const TBeing *) - This initializer will store the TBeing pointer
// that is passed and extract the name from it for use.  The TBeing passed
// is the player whose trophy data you want to interact with.
//
// TTrophy(sstring) - This initializer will store the sstring that is passed
// and store it for use.  The sstring passed is the name of the player whose
// trophy data you want to interact with.
//
// setName(sstring) - Sets the name variable to the passed sstring and unlinks
// the TBeing pointer that is stored.  Use this if you want to re-use a
// TTrophy instance.
//
// float getExpModVal(float) - Returns the exp mod value for the count that
// is passed.  The value will be in the range 0.0 - 1.0.
//
// const char *getExpModDescr(float) - Returns a text description of the exp
// mod value, such as "full", "much", "some", etc.
//
// addToCount(int vnum, double cnt) - Adds cnt to the count for the mob vnum.
//
// getCount(int vnum) - Returns the count value for the mob vnum.
//
// wipe() - Clears the trophy for the parent, this is for character deletion.
//
//
// On retrospect, it'd probably be better to move getCount and AddToCount
// into TBeing, and leave TTrophy as a generic trophy count manipulation class


class TDatabase;

class TTrophy {
  TDatabase *db;
  const TBeing *parent;
  sstring name;

  sstring getMyName();

  TTrophy(){}; // no blank initializations
 public:
  void setName(sstring);

  float getExpModVal(float, int);
  const char *getExpModDescr(float, int);
  void addToCount(int, double);
  float getCount(int);
  float getTotalCount(int);

  void wipe();
  
  TTrophy(sstring);
  TTrophy(TBeing *);
};


#endif
