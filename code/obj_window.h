//////////////////////////////////////////////////////////////////////////
//
// SneezyMUD - All rights reserved, SneezyMUD Coding Team
//
//
//////////////////////////////////////////////////////////////////////////

#ifndef __OBJ_WINDOW_H
#define __OBJ_WINDOW_H

#include "obj_seethru.h"

class TWindow : public TSeeThru {
  private:
  public:
    virtual void assignFourValues(int, int, int, int);
    virtual void getFourValues(int *, int *, int *, int *) const;
    virtual sstring statObjInfo() const;
    virtual itemTypeT itemType() const { return ITEM_WINDOW; }

    virtual void listMe(TBeing *, unsigned int) const;
    virtual void listMeExcessive(TBeing *) const;
    virtual void showMe(TBeing *) const;

    virtual void lookObj(TBeing *, int) const;
    virtual void describeMe(TBeing *) const {}
    virtual bool listThingRoomMe(const TBeing *) const;
    virtual void show_me_mult_to_char(TBeing *, showModeT, unsigned int) const;
    virtual void show_me_to_char(TBeing *, showModeT) const;

    TWindow();
    TWindow(const TWindow &a);
    TWindow & operator=(const TWindow &a);
    virtual ~TWindow();
};



#endif
