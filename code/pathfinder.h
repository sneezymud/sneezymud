#ifndef __PATHFINDER_H
#define __PATHFINDER_H

class TPathTarget {
 public:
  virtual bool isTarget(int) const {
    return false;
  }
};


// findTargetRoom(int vnum) = find room specified by vnum
class findTargetRoom : public TPathTarget {
 private:
  int dest;

 public:
  findTargetRoom(int);
  bool isTarget(int) const;
};



class TPathFinder {
 private:
  TPathTarget pt;
  bool thru_doors;
  bool use_portals;
  int depth;
  bool stay_zone;
  bool no_mob;

 public:
  dirTypeT findPath(int, const TPathTarget &);

  TPathFinder();

};


class pathData {
  public:
    dirTypeT direct;
    int source;
    bool checked;
    pathData() : direct(DIR_NONE), source(0), checked(false) {}
};



extern int go_ok(roomDirData *exitp);
extern int go_ok_smarter(roomDirData *exitp);



#endif
