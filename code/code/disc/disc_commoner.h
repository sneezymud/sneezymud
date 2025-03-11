#pragma once

#include "discipline.h"

class CDCommoner : public CDiscipline {
  public:

    virtual CDCommoner* cloneMe() { return new CDCommoner(*this); }

    bool isBasic() { return true; }

  private:
};
