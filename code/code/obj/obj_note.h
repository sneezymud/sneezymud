//////////////////////////////////////////////////////////////////////////
//
// SneezyMUD - All rights reserved, SneezyMUD Coding Team
//
//
//////////////////////////////////////////////////////////////////////////

#pragma once

#include "obj.h"

class TNote : public TObj {
  private:
    int repairman;
    int time_adjust;
    int obj_v;

  public:
    virtual void assignFourValues(int, int, int, int);
    virtual void getFourValues(int*, int*, int*, int*) const;
    virtual sstring statObjInfo() const;
    virtual itemTypeT itemType() const { return ITEM_NOTE; }
    virtual void showMe(TBeing*) const;

    virtual void postMe(TBeing*, const char*, TBoard*);
    virtual int personalizedCheck(TBeing*) { return false; }
    virtual void describeMe(TBeing*) const {}
    virtual void writeMeNote(TBeing*, TPen*);
    virtual int objectSell(TBeing*, TMonster*);
    virtual bool isPersonalized() {
      return false;
    }  // action_desc is not personalization
    virtual void giveToRepairNote(TMonster*, TBeing* ch, int*);
    virtual void giveToRepair(TMonster*, TBeing* ch, int*);
    virtual void junkMe(TBeing*);
    virtual void noteMe(TMonster*, TBeing*, TObj*, time_t, int);

    int getRepairman() const;
    void setRepairman(int n);
    int getTimeAdj() const;
    void setTimeAdj(int n);
    int getObjV() const;
    void setObjV(int n);

    TNote();
    TNote(const TNote& a);
    TNote& operator=(const TNote& a);
    virtual ~TNote();
};
