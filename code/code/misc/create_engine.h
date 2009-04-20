//////////////////////////////////////////////////////////////////////////
//
// SneezyMUD - All rights reserved, SneezyMUD Coding Team
//
// $Log: create_engine.h,v $
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


#ifndef __CREATE_ENGINE_H
#define __CREATE_ENGINE_H

const int MAX_APPLIED_SUBSTANCES = 10;

enum CreateEngineMustHaveAS {
  CEMH_AS_BALM,   // carved wood canister, cooking grease, fire
    CEMH_AS_SALVE,  // animal hoof         , bacon grease  , fire           , oilded cloth
    CEMH_AS_POWDER, // animal horn         , sifted flour  , fire           , cork
    CEMH_AS_OIL,    // animal bladder      , crushed nuts  , fire           , silken cord
    CEMH_AS_ICHOR   // carved stone tube   , plant nectar  , piece of rubber
};

enum CreateEngineMethods {
  CEM_APPLIED,
  CEM_BREW,
  CEM_ALCHEMIST
};

struct appliedCreate_struct
{
  TThing *tObjList[10];
  int    totMessages;
};

class CreateEngineData
{
  public:
    unsigned short   eClass;
        int   ceType;
     short   skCEMin;
    short skCEMax;
    short skCERounds;
    spellNumT skCESkillNum;
    char     *ceName,
             *ceStartMsg,
             *ceEndMsg,
             *ceMessages[5];
    long int  CompList[10],
              CompReward;

    int (*ceStartup)(int, TBeing *, TObj *);
    int (*cePerMesg)(int, TBeing *, TObj *);
    int (*ceEnding )(int, TBeing *, TObj *);

    CreateEngineData(unsigned short, int, short, short, short, spellNumT, const char *, const char *, const char *, 
                     const char *, const char *, const char *, const char *, const char *, long, long, long, long, long,
                     long, long, long, long, long, long, int (*)(int, TBeing *, TObj *),
                     int (*)(int, TBeing *, TObj *), int (*)(int, TBeing *, TObj *));
    ~CreateEngineData();
};

extern class CreateEngineData *AppliedCreate[MAX_APPLIED_SUBSTANCES];

#endif
