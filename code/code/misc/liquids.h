#ifndef __LIQUIDS_H
#define __LIQUIDS_H

// this contains data for a single liquid type
// meant to be used as an element in a list of types
class liqEntry {
  public:
  int drunk;
  int hunger;
  int thirst;
  bool potion;
  bool poison;
  const char * color;
  const char * name;
  int price;

  liqEntry(int, int, int, bool, bool, const char *, const char *, int);
  liqEntry & operator=(const liqEntry &a);
  ~liqEntry();

  private:
  liqEntry();  // deny usage in this format
};


// this is a container class for a list of liquid types
class liqInfoT {
  map<liqTypeT, liqEntry *>liquids;

 public:
  liqEntry *operator[] (const liqTypeT);

  liqInfoT();
  ~liqInfoT();  
};



#endif
