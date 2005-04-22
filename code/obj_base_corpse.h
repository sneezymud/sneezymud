//////////////////////////////////////////////////////////////////////////
//
// SneezyMUD - All rights reserved, SneezyMUD Coding Team
//
//
//////////////////////////////////////////////////////////////////////////

#ifndef __OBJ_BASE_CORPSE_H
#define __OBJ_BASE_CORPSE_H

#include "obj_base_container.h"

// an abstract corpse
class TBaseCorpse : public TBaseContainer {
  private:
    unsigned int corpse_flags;
    race_t corpse_race;
    unsigned int corpse_level;
    int corpse_vnum;
  public:
    dissectInfo *tDissections;

    virtual void assignFourValues(int, int, int, int);
    virtual void getFourValues(int *, int *, int *, int *) const;
    virtual sstring statObjInfo() const = 0;

    virtual int chiMe(TBeing *);
    virtual void peeOnMe(const TBeing *);
    virtual int dissectMe(TBeing *);
    virtual void update(int);
    virtual void lookObj(TBeing *, int) const;
    virtual int scavengeMe(TBeing *, TObj **);
    virtual void decayMe();
    virtual int objectDecay();
    virtual int putSomethingInto(TBeing *, TThing *);
    virtual int putMeInto(TBeing *, TOpenContainer *);
    virtual void describeObjectSpecifics(const TBeing *) const {}
    void getObjFromMeText(TBeing *, TThing *, getTypeT, bool);

    void setCorpseFlags(unsigned int);
    unsigned int getCorpseFlags() const;
    void addCorpseFlag(unsigned int);
    void remCorpseFlag(unsigned int);
    bool isCorpseFlag(unsigned int) const;
    void setCorpseRace(race_t);
    race_t getCorpseRace() const;
    void setCorpseLevel(unsigned int);
    unsigned int getCorpseLevel() const;
    void setCorpseVnum(int);
    int getCorpseVnum() const;

    void setupDissectionObjects();

  protected:
    TBaseCorpse();
  public:
    TBaseCorpse(const TBaseCorpse &a);
    TBaseCorpse & operator=(const TBaseCorpse &a);
    virtual ~TBaseCorpse();
};



#endif
