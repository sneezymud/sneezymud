#ifndef __PATHFINDER_H
#define __PATHFINDER_H

#include <deque>
#include <boost/shared_ptr.hpp>


class TPathTarget {
 public:
  virtual bool isTarget(int) const {
    return false;
  }

  virtual ~TPathTarget(){};
};

class findFairFight : public TPathTarget {
 private:
  TBeing *myself;

 public:
  findFairFight(TBeing *);
  bool isTarget(int) const;
};


// findRoom(int vnum) = find room specified by vnum
class findRoom : public TPathTarget {
 private:
  int dest;

 public:
  findRoom(int);
  bool isTarget(int) const;
};

// findClutter(myself) = find closest room with stuff for a janitor to pick up
class findClutter : public TPathTarget {
 private:
  TBeing *myself;

 public:
  findClutter(TBeing *);
  bool isTarget(int) const;

};

// findClutter(myself) = find closest room with stuff for a janitor to pick up
class findClutterPrison : public TPathTarget {
 private:
  TBeing *myself;

 public:
  findClutterPrison(TBeing *);
  bool isTarget(int) const;

};

// findClutter(myself) = find closest room with stuff for a janitor to pick up
class findClutterAmber : public TPathTarget {
 private:
  TBeing *myself;

 public:
  findClutterAmber(TBeing *);
  bool isTarget(int) const;

};

// findClutter(myself) = find closest room with stuff for a janitor to pick up
class findClutterBrightmoon : public TPathTarget {
 private:
  TBeing *myself;

 public:
  findClutterBrightmoon(TBeing *);
  bool isTarget(int) const;

};




// findPolice() = find closest police (guard or bouncer)
class findPolice : public TPathTarget {
 public:
  findPolice();
  bool isTarget(int) const;
};

// findOutdoors() = find closest outdoors room
class findOutdoors : public TPathTarget {
 public:
  findOutdoors();
  bool isTarget(int) const;
};

// findCorpse() = find the closest corpse in Grimhaven
class findCorpse : public TPathTarget {
 public:
  findCorpse();
  bool isTarget(int) const;
};

// findFire() = find closest fire in grimhaven
class findFire : public TPathTarget {
 public:
  findFire();
  bool isTarget(int) const;
};

// findBeing(sstring) = find closest being specified by name
class findBeing : public TPathTarget {
 private:
  sstring name;
 public:
  findBeing(sstring);
  bool isTarget(int) const;
};

// findWater() = find closest source of water
class findWater : public TPathTarget {
 public:
  findWater();
  bool isTarget(int) const;
};

// findLeper() = find a leper
class findLeper : public TPathTarget {
 public:
  findLeper();
  bool isTarget(int) const;
};

// findEquipment() = finds eq once belonging to a specific being
class findEquipment : public TPathTarget {
 private:
  TPerson *owner;
  std::vector<TThing *> foundlist;
  bool findInStuff(StuffList stuff) const;
  bool findInThing(TThing *) const;
  bool contains(TThing *t) const;
  bool checkOwner(TObj *o) const;

 public:
  findEquipment(TPerson *o) { owner = o; }
  bool isTarget(int) const;
  int getCount() const { return (int)foundlist.size(); }
  TThing *getFound(int i) const { if ((int)foundlist.size() <= i) return NULL; return foundlist[i]; }
};


class pathData {
  public:
    int room;
    dirTypeT direct;
    int source;
    bool checked;
    int distance;
    pathData() : 
      room(0), direct(DIR_NONE), source(0), checked(false), distance(0) {}
    pathData(int r, dirTypeT d, int s, bool c, int dist) : 
      room(r), direct(d), source(s), checked(c), distance(dist) {}
    pathData(pathData *pd) :
      room(pd->room), direct(pd->direct), source(pd->source),
      checked(pd->checked), distance(pd->distance) {}
};

typedef boost::shared_ptr<pathData> pathDataPtr;
typedef std::deque <pathDataPtr> pathDataList;

class TPathFinder {
 private:
  TPathTarget pt;
  bool thru_doors;
  bool use_portals;
  int range;
  bool stay_zone;
  bool no_mob;
  bool ship_only;
  bool use_cached;

  int dest;
  int dist;

 public:
  pathDataList path;

  void setRange(int);
  void setThruDoors(bool t){ thru_doors=t; }
  void setStayZone(bool t){ stay_zone=t; }
  void setUsePortals(bool t){ use_portals=t; }
  void setNoMob(bool t){ no_mob=t; }
  void setShipOnly(bool t){ ship_only=t; }
  void setUseCached(bool t){ use_cached=t; }

  int getDest(){ return dest; }
  int getDist(){ return dist; }

  dirTypeT findPath(int, const TPathTarget &);

  TPathFinder();
  TPathFinder(int depth);

  ~TPathFinder();
};

// based on TPathFinder, it combines up a path to the target(s)
class TPathBuilder : public TPathFinder {


};



extern int go_ok(roomDirData *exitp);
extern int go_ok_smarter(roomDirData *exitp);



#endif
