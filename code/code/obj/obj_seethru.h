//////////////////////////////////////////////////////////////////////////
//
// SneezyMUD - All rights reserved, SneezyMUD Coding Team
//
//
//////////////////////////////////////////////////////////////////////////

#pragma once

#include "enum.h"
#include "obj.h"
#include "sstring.h"

class TBeing;

class TSeeThru : public TObj {
  private:
    int target_room;

  public:
    virtual void assignFourValues(int, int, int, int);
    virtual void getFourValues(int*, int*, int*, int*) const;
    virtual sstring statObjInfo() const = 0;

    virtual void purgeMe(TBeing*);
    virtual bool canSeeMe(const TBeing*, infraTypeT) const;
    virtual int riverFlow(int);

    bool givesOutsideLight() const;
    int getLightFromOutside() const;

    int getTarget(int* = nullptr) const;
    void setTarget(int);

  protected:
    TSeeThru();

  public:
    TSeeThru(const TSeeThru& a);
    TSeeThru& operator=(const TSeeThru& a);
    virtual ~TSeeThru();
};
