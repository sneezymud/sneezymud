#ifndef __OBJ_MERGEABLE_H
#define __OBJ_MERGEABLE_H

// this is a base class for object types are that "mergeable"
// ex, components, commodities, talens, etc


class TMergeable : public TObj {
 private:

 public:
  virtual bool willMerge(TMergeable *);
  virtual void doMerge(TMergeable *);

  TMergeable();
  TMergeable(const TMergeable &a);
  TMergeable & operator=(const TMergeable &a);
  virtual ~TMergeable();

};


#endif
