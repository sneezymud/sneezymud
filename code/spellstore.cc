//////////////////////////////////////////////////////////////////////////
//
// SneezyMUD - All rights reserved, SneezyMUD Coding Team
//
//////////////////////////////////////////////////////////////////////////

#include "stdsneezy.h"

#if 0
doCast -> doDiscipline -> flamingSword -> start_cast

spelltask.cc 1439: check for flag and store as needed

add storage class to TBeing
use it to pass the info needed for the spelltask.cc check

spellTaskData

#endif



int TBeing::doTrigger(const char *argument){
  char arg[256];
  spellNumT which;
  TBeing *ch=NULL;
  TObj *o=NULL;
  TThing *t=NULL;

  return FALSE;

  if(!preCastCheck())
    return FALSE;
  
  strcpy(arg, argument);

  if((which=parseSpellNum(arg))==TYPE_UNDEFINED)
    return FALSE;

  if (!discArray[which]) {
    vlogf(LOG_BUG, "doDiscipline called with null discArray[] (%d) (%s)", which, getName());
    return FALSE;
  }

  if (which <= TYPE_UNDEFINED) 
    return FALSE;

  if(!preDiscCheck(which))
    return FALSE;

  if(!parseTarget(which, arg, &t))
    return FALSE;

  ch=dynamic_cast<TBeing *>(t);
  o=dynamic_cast<TObj *>(t);

  doSpellCast(this, ch, o, roomp, which, getSpellType(discArray[which]->typ));

  return TRUE;
}

int TBeing::doStore(const char *argument){
  return FALSE;
}
