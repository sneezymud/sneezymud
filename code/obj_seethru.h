//////////////////////////////////////////////////////////////////////////
//
// SneezyMUD - All rights reserved, SneezyMUD Coding Team
//
//
//////////////////////////////////////////////////////////////////////////

#ifndef __OBJ_SEETHRU_H
#define __OBJ_SEETHRU_H

#include "obj.h"

class TSeeThru : public TObj {
  private:
    int target_room;
  public:
    virtual void assignFourValues(int, int, int, int);
    virtual void getFourValues(int *, int *, int *, int *) const;
    virtual sstring statObjInfo() const = 0;

    virtual void purgeMe(TBeing *);
    virtual bool canSeeMe(const TBeing *, infraTypeT) const;
    virtual int riverFlow(int);

    bool givesOutsideLight() const;
    int getLightFromOutside() const;

    int getTarget(int * = NULL) const;
    void setTarget(int);

  protected:
    TSeeThru();
  public:
    TSeeThru(const TSeeThru &a);
    TSeeThru & operator=(const TSeeThru &a);
    virtual ~TSeeThru();
};



#endif
