//////////////////////////////////////////////////////////////////////////
//
// SneezyMUD - All rights reserved, SneezyMUD Coding Team
//
//////////////////////////////////////////////////////////////////////////

#include "being.h"

#if 0

doCast -> doDiscipline -> flamingSword -> start_cast

--
doStore:
go through normal cast, stop at spelltask.cc:1439 and check if we are storing
a spell, if so, store it and return

doTrigger:
go through cast checks etc.  once number is parsed, search for that spell in
the stored list, if its there, move to spelltask and call start_cast
--

need to make it a list so we can store more than one somehow

spellTaskData

class spellStoreData {
public:
  spellTaskData *spelltask;
  bool storing;
};


#endif



int TBeing::doTrigger(const char *argument){
  char arg[256];
  spellNumT which;
  TBeing *ch=NULL;
  TObj *o=NULL;
  TThing *t=NULL;
  int rc;

  if(!preCastCheck())
    return FALSE;
  
  strcpy(arg, argument);

  if((which=parseSpellNum(arg))==TYPE_UNDEFINED)
    return FALSE;

  if (!discArray[which]) {
    vlogf(LOG_BUG, format("doTrigger called with null discArray[] (%d) (%s)") %  which % getName());
    return FALSE;
  }

  if (which <= TYPE_UNDEFINED || !preDiscCheck(which) || 
      !parseTarget(which, arg, &t))
    return FALSE;

  ch=dynamic_cast<TBeing *>(t);
  o=dynamic_cast<TObj *>(t);

  spelltask=spellstore.spelltask;

  rc=doSpellCast(this, ch, o, roomp, which, getSpellType(discArray[which]->typ));
  spellstore.spelltask=NULL;
  spellstore.storing=false;

  return rc;
}

int TBeing::doStore(const char *argument)
{
  int rc;

  if(spellstore.storing || spelltask){
    sendTo("You are already casting a spell.\n\r");
    return FALSE;
  }
  if(spellstore.spelltask){
    sendTo("You already have a spell stored.\n\r");
    return FALSE;
  }

  spellstore.storing=TRUE;
  rc=doCast(argument);

  if(rc==FALSE){
    act("Your spell has not been stored.",
	TRUE,this, NULL, NULL, TO_CHAR, ANSI_RED);
    spellstore.storing=FALSE;
    delete spellstore.spelltask;
  }

  return rc;
}


