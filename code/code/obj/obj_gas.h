//////////////////////////////////////////////////////////////////////////
//
// SneezyMUD - All rights reserved, SneezyMUD Coding Team
//
//
//////////////////////////////////////////////////////////////////////////

#pragma once

#include <string>

#include "enum.h"
#include "handler.h"
#include "obj.h"
#include "obj_mergeable.h"
#include "sstring.h"

class TBeing;

class TPathFinder;

// All gasses operate the same way somewhat.  They behavior is
// differentiated by their gasTypeT.  All sorts of data can be stored
// on a TGas mostly withour worrying about serialization, since the
// TGas object is never stored in the database.

class TGas : public TMergeable {
  public:
    virtual void assignFourValues(int, int, int, int);
    virtual void getFourValues(int*, int*, int*, int*) const;
    virtual sstring statObjInfo() const;
    virtual void decayMe();
    virtual bool isPluralItem() const;
    virtual itemTypeT itemType() const { return ITEM_GAS; }
    virtual void setVolume(int);
    virtual void addToVolume(int);
    const sstring& getName() const;
    const sstring& getDesc() const;
    const sstring& getShortName() const;
    void doSpecials();
    void updateDesc();
    void doDrift();

    virtual bool willMerge(TMergeable*);
    virtual void doMerge(TMergeable*);

    TGas(gasTypeT gasType = GAS_UNDEFINED);
    TGas(const TGas& a);
    ~TGas();

    void setType(gasTypeT newType) { type = newType; }
    gasTypeT getType() const { return type; }
    void addCreator(const char* s) {
      creator += s;
      creator += ",";
    }
    bool hasCreator(const sstring s) const {
      return creator.find(s) != sstring::npos;
    }
    TBeing* getCreatedBy() const {
      return get_pc_world(nullptr, creator.substr(0, creator.find(",")), EXACT_YES,
        INFRA_NO, false);
    }

  protected:
    gasTypeT type;
    sstring creator;

  private:
    TPathFinder* driftPath;
};
