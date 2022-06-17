#pragma once

#include "discipline.h"
#include "skills.h"

class CDRitualism : public CDiscipline
{
public:
    CSkill skRitualism;

    CDRitualism() :
      CDiscipline(),
      skRitualism()
    {
    }
    CDRitualism(const CDRitualism &a) :
      CDiscipline(a),
      skRitualism(a.skRitualism)
    {
    }
    CDRitualism & operator=(const CDRitualism &a) {
      if (this == &a) return *this;
      CDiscipline::operator=(a);
      skRitualism = a.skRitualism;
      return *this;
    }
    virtual ~CDRitualism() {}
    virtual CDRitualism * cloneMe() { return new CDRitualism(*this); }

    bool isAutomatic(){ return true; }
private:
};
