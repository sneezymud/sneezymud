//////////////////////////////////////////////////////////////////////////
//
// SneezyMUD - All rights reserved, SneezyMUD Coding Team
//
//
//////////////////////////////////////////////////////////////////////////

#ifndef __OBJ_GAS_H
#define __OBJ_GAS_H

#include "obj.h"
#include "obj_mergeable.h"
#include "handler.h"

class TPathFinder;

// All gasses operate the same way somewhat.  They behavior is
// differentiated by their gasTypeT.  All sorts of data can be stored
// on a TGas mostly withour worrying about serialization, since the
// TGas object is never stored in the database.

class TGas : public TMergeable {
  public:    
    virtual void assignFourValues(int, int, int, int);
    virtual void getFourValues(int *, int *, int *, int *) const;
    virtual sstring statObjInfo() const;
    virtual void decayMe();
    virtual bool isPluralItem() const;
    virtual itemTypeT itemType() const { return ITEM_GAS; }
    virtual void setVolume(int);
    virtual void addToVolume(int);
    const char * getName() const;
    const char * getDesc() const;
    const char * getShortName() const;
    void doSpecials();
    void updateDesc();
    void doDrift();

    virtual bool willMerge(TMergeable *);
    virtual void doMerge(TMergeable *);

    TGas(gasTypeT gasType = GAS_UNDEFINED);
    TGas(const TGas &a);
    ~TGas();

    void setType(gasTypeT newType) { type = newType; }
    gasTypeT getType() const { return type; }
    void addCreator(const char *s) { creator += s; creator += ","; }
    bool hasCreator(const sstring s) const { return creator.find(s) != sstring::npos; }
    TBeing * getCreatedBy() const { return get_pc_world(NULL, creator.substr(0,creator.find(",")), EXACT_YES, INFRA_NO, FALSE); }

protected:
    gasTypeT type;
    sstring creator;

 private:
    TPathFinder *driftPath;
};



#endif
