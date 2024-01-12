//////////////////////////////////////////////////////////////////////////
//
// SneezyMUD - All rights reserved, SneezyMUD Coding Team
//
//
//////////////////////////////////////////////////////////////////////////

#pragma once

#include "obj.h"
#include "sstring.h"

class TAudio : public TObj {
  private:
    int freq;

  public:
    virtual void assignFourValues(int, int, int, int);
    virtual void getFourValues(int*, int*, int*, int*) const;
    virtual sstring statObjInfo() const;
    virtual itemTypeT itemType() const { return ITEM_AUDIO; }

    virtual void audioCheck(int) const;

    int getFreq() const;
    void setFreq(int n);
    TAudio();
    TAudio(const TAudio& a);
    TAudio& operator=(const TAudio& a);
    virtual ~TAudio();
};
