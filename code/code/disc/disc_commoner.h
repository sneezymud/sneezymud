//////////////////////////////////////////////////////////////////////////
//
// SneezyMUD - All rights reserved, SneezyMUD Coding Team
//
//
//////////////////////////////////////////////////////////////////////////


#pragma once

#include "discipline.h"

class CDCommoner : public CDiscipline
{
public:
    CDCommoner()
      : CDiscipline(){
    }
    CDCommoner(const CDCommoner &a)
      : CDiscipline(a){
    }
    CDCommoner & operator=(const CDCommoner &a) {
      if (this == &a) return *this;
      CDiscipline::operator=(a);
      return *this;
    }
    virtual ~CDCommoner() {}
    virtual CDCommoner * cloneMe() { return new CDCommoner(*this); }

    bool isBasic(){ return true; }

private:
};
