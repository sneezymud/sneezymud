//////////////////////////////////////////////////////////////////////////
//
// SneezyMUD - All rights reserved, SneezyMUD Coding Team
//
//
//////////////////////////////////////////////////////////////////////////

#ifndef __OBJ_EXPANDABLE_CONTAINER_H
#define __OBJ_EXPANDABLE_CONTAINER_H

#include "obj_open_container.h"

// an openable container that changes in volume based on contents
class TExpandableContainer : public TOpenContainer {
  private:
  public:
    virtual void assignFourValues(int, int, int, int);
    virtual void getFourValues(int *, int *, int *, int *) const;
    virtual sstring statObjInfo() const;

    virtual int getTotalVolume() const;
    virtual void addToCarriedVolume(int num);

  protected:
    TExpandableContainer();
  public:
    TExpandableContainer(const TExpandableContainer &a);
    TExpandableContainer & operator=(const TExpandableContainer &a);
    virtual ~TExpandableContainer();
    virtual TThing & operator+= (TThing & t);
};



#endif
