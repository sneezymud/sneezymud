#ifndef __DISC_CMD_TROPHY_H
#define __DISC_CMD_TROPHY_H

// TTrophy is a class for interacting with the trophy data
// for a particular player
//
// ch->trophy->getExpModDescr(mobvnum);
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
// TTrophy(string) - This initializer will store the string that is passed
// and store it for use.  The string passed is the name of the player whose
// trophy data you want to interact with.
//
// setName(string) - Sets the name variable to the passed string and unlinks
// the TBeing pointer that is stored.  Use this if you want to re-use a
// TTrophy instance.
//
// float getExpModVal(int) - Returns the exp mod value for the mob vnum that
// is passed.  The value will be in the range 0.0 - 1.0.
//
// const char *getExpModDescr(int) - Returns a text description of the exp
// mod value, such as "full", "much", "some", etc.
//
// addToCount(int vnum, double cnt) - Adds cnt to the count for the mob vnum.
//
// getCount(int vnum) - Returns the count value for the mob vnum.
//
// wipe() - Clears the trophy for the parent, this is for character deletion.

class TDatabase;

class TTrophy {
  TDatabase *db;
  const TBeing *parent;
  string name;

  string getMyName();

  TTrophy(){}; // no blank initializations
 public:
  void setName(string);

  float getExpModVal(int);
  const char *getExpModDescr(int);
  void addToCount(int, double);
  float getCount(int);

  void wipe();
  
  TTrophy(string);
  TTrophy(TBeing *);
};


#endif


