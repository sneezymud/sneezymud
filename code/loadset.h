//////////////////////////////////////////////////////////////////////////
//
// SneezyMUD - All rights reserved, SneezyMUD Coding Team
//
// $Log: loadset.h,v $
// Revision 5.1.1.1  1999/10/16 04:32:20  batopr
// new branch
//
// Revision 5.1  1999/10/16 04:31:17  batopr
// new branch
//
// Revision 1.1  1999/09/12 17:24:04  sneezy
// Initial revision
//
//
//////////////////////////////////////////////////////////////////////////


#ifndef __LOADSET_H
#define __LOADSET_H

enum loadSetTypeT {
  LST_ALL  = -1,
  LST_HELM = 0,
  LST_COLLAR,
  LST_JACKET,
  LST_SLEEVE,
  LST_GLOVE,
  LST_BELT,
  LST_BRACELET,
  LST_LEGGING,
  LST_BOOT,
  LST_RING,
  LST_SHIELD,
  LST_CLOAK,
  LST_MAX
};

class loadSetStruct {
  public:
    const          char   *name;
                   int     equipment[LST_MAX];
                   race_t  suitRace;
                   double  suitLevel;
    unsigned short int     suitClass[LST_MAX],
                           suitClassTotal,
                           suitClassPossible;

    loadSetStruct & operator=(const loadSetStruct &);

    loadSetStruct();
    ~loadSetStruct();
};

class loadSetClass {
  public:
    map<unsigned short int, loadSetStruct>suits;

    bool suitLoad(const char *, TBeing *, loadSetTypeT, int, int);
    void SetupLoadSetSuits();
    void suitAdd(const char *, int, int, int, int, int, int,
                 int, int, int, int, int, int, race_t);

    loadSetClass() {}
    ~loadSetClass() {}
};

extern loadSetClass suitSets;

#endif
